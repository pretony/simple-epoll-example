#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <memory>
#include "clientServer.h"
#include "business.h"

using namespace std;
#define MAXEVENTS 64
#define COMM_HEADER_KEY 0x12344321
static int create_and_bind(int port);
static int make_socket_non_blocking(int sfd);

void ClientServer::process_business(int fd)
{
    while(tcpClinetConnMap[fd].usedSpace() > sizeof(header_t))
    {
        //先读取前8个字节，获取需要的字节数
        header_t header;
        size_t len = tcpClinetConnMap[fd].copyData(&header, sizeof(header_t));
        if (len != sizeof(header_t) || header.key != ntohl(COMM_HEADER_KEY))
        {
            //出错
            printf("[%d] read error, len=%ld, header.key=0x%08X\n", __LINE__, len, header.key);
            close(fd);
            tcpClinetConnMap.erase(fd);
            return;
        }
        header.data_size = ntohl(header.data_size);
        //有足够的数据
        if(header.data_size <= tcpClinetConnMap[fd].usedSpace())
        {
            size_t readSize = sizeof(header_t)+header.data_size;
            std::unique_ptr<uint8_t[]> processData(new uint8_t[readSize]);
            tcpClinetConnMap[fd].pull(processData.get(), readSize);
            const char *data = (const char *)(processData.get()+sizeof(header_t));
            size_t len = header.data_size;
            //printf("recv data:%s\n", data);
            process_business_real(this, data, len);
        }
    }
}

void ClientServer::handle_read(int fd)
{
    while (1)
    {
        ssize_t count;
        char buf[1024]={0};
        size_t freeSpace = tcpClinetConnMap[fd].freeSpace();
        size_t readSize = (freeSpace < sizeof(buf))?freeSpace:sizeof(buf);
        count = read(fd, buf, readSize);
        if (count <= 0)
        {
            if((count == 0) || (errno != EAGAIN && errno != EWOULDBLOCK))
            {
                //printf("[%d] read error, count=%ld, error=%d\n", __LINE__, count, errno);
                close(fd);
                tcpClinetConnMap.erase(fd);
            }
            break;
        }
        else
        {
            tcpClinetConnMap[fd].push(buf, count);
            //业务处理
            process_business(fd);
        }
    }
}

void ClientServer::handle_accept(int sfd, int efd)
{
    int s = 0;
    struct epoll_event event;
    while (1)
    {
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd;
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        in_len = sizeof in_addr;
        infd = accept(sfd, &in_addr, &in_len);
        if (infd == -1)
        {
            if(errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("accept");
            }
            break;
        }

        s = getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf,
                sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
        if (s == 0)
        {
            printf("Accepted connection on descriptor %d "
                    "(host=%s, port=%s)\n", infd, hbuf, sbuf);
        }

        s = make_socket_non_blocking(infd);
        if (s == -1)
            break;

        memset(&event, 0x00, sizeof(struct epoll_event));
        event.data.fd = infd;
        event.events = EPOLLIN;
        s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
        if (s == -1)
        {
            perror("epoll_ctl");
            break;
        }
    }
}

int ClientServer::tcpServer(int port)
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event events[MAXEVENTS];

    sfd = create_and_bind(port);
    if (sfd == -1)
        abort();

    s = make_socket_non_blocking(sfd);
    if (s == -1)
        abort();

    s = listen(sfd, 16);
    if (s == -1)
    {
        perror("listen");
        abort();
    }

    efd = epoll_create1(0);
    if (efd == -1)
    {
        perror("epoll_create");
        abort();
    }

    memset(&event, 0x00, sizeof(struct epoll_event));
    event.data.fd = sfd;
    event.events = EPOLLIN;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1)
    {
        perror("epoll_ctl");
        abort();
    }

    while (!bExit)
    {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            int events_ = events[i].events;
            if (events_ & (EPOLLIN))
            {
                if (fd == sfd)
                {
                    handle_accept(sfd, efd);
                }
                else
                {
                    handle_read(fd);
                }
            }
            else
            {
                printf("unknown event\n");
            }
        }
    }

    close(sfd);
}

int create_and_bind(int port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char portStr[10] = {0};
    snprintf(portStr, sizeof(portStr), "%d", port);
    s = getaddrinfo(NULL, portStr, &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0)
        {
            break;
        }

        close(sfd);
    }

    if (rp == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(result);

    return sfd;
}

int make_socket_non_blocking(int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror("fcntl");
        return -1;
    }

    return 0;
}
