#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COMM_HEADER_KEY 0x12344321
struct header_t
{
    int   key;        //双方认证码
    int   data_size;  //传输的字节，不包括header的8个字节
    header_t():key(0),data_size(0){}
    void clear()
    {
        key = 0;
        data_size = 0;
    }
};

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("use: ./client-test 127.0.0.1 9999\n");
        return 0;
    }
    struct addrinfo hint, *result;
    int res, sfd;
    char buf[4096]={0};

    memset(&hint, 0, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    res = getaddrinfo(argv[1], argv[2], &hint, &result);
    if (res == -1) {
        perror("error : cannot get socket address!\n");
        exit(1);
    }

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1) {
        perror("error : cannot get socket file descriptor!\n");
        exit(1);
    }

    res = connect(sfd, result->ai_addr, result->ai_addrlen);
    if (res == -1) {
        perror("error : cannot connect the server!\n");
        exit(1);
    }

    for(int i=0; i<5; i++)
    {
        memset(buf,0,sizeof(buf));
        char sendData[] = "[{\"Cmd\":1}]";
        header_t header;
        header.key = htonl(COMM_HEADER_KEY);
        header.data_size = htonl(strlen(sendData));

        memcpy(buf, &header, sizeof(header_t));
        memcpy(buf+sizeof(header_t), sendData, strlen(sendData));
        size_t nbytes = write(sfd, buf, strlen(sendData)+sizeof(header_t));
        printf("write < %s > to server, nbytes=%ld\n", sendData, nbytes);
    }
    for(int i=0; i<5; i++)
    {
        memset(buf,0,sizeof(buf));
        char sendData[] = "[{\"Cmd\":2}]";
        header_t header;
        header.key = htonl(COMM_HEADER_KEY);
        header.data_size = htonl(strlen(sendData));

        memcpy(buf, &header, sizeof(header_t));
        memcpy(buf+sizeof(header_t), sendData, strlen(sendData));
        size_t nbytes = write(sfd, buf, strlen(sendData)+sizeof(header_t));
        printf("write < %s > to server, nbytes=%ld\n", sendData, nbytes);
    }
    {
        memset(buf,0,sizeof(buf));
        char sendData[] = "[{\"Cmd\":3}]";
        header_t header;
        header.key = htonl(COMM_HEADER_KEY);
        header.data_size = htonl(strlen(sendData));

        memcpy(buf, &header, sizeof(header_t));
        memcpy(buf+sizeof(header_t), sendData, strlen(sendData));
        size_t nbytes = write(sfd, buf, strlen(sendData)+sizeof(header_t));
        printf("write < %s > to server, nbytes=%ld\n", sendData, nbytes);
    }

    close(sfd);
    return 0;
}
