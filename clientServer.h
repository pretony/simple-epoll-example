#ifndef _CLIENTSERVER_H
#define _CLIENTSERVER_H
#include <map>
#include "ringbuf.h"

#define RINGBUF_SIZE 4096

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

struct TcpClinetConn
{
    ringbuf_t rb1;
    TcpClinetConn()
    {
        rb1 = ringbuf_new(RINGBUF_SIZE - 1);
    }
    ~TcpClinetConn()
    {
        ringbuf_free(&rb1);
    }
    size_t usedSpace()
    {
        return ringbuf_bytes_used(rb1);
    }
    size_t freeSpace()
    {
        return ringbuf_bytes_free(rb1);
    }
    bool isEmpty()
    {
        return (ringbuf_is_empty(rb1) == 1)?true:false;
    }
    bool isFull()
    {
        return (ringbuf_is_full(rb1) == 1)?true:false;
    }
    void clear()
    {
        ringbuf_reset(rb1);
    }
    void push(const void *data, size_t len)
    {
        ringbuf_memcpy_into(rb1, data, len);
    }
    void pull(void *data, size_t len)
    {
        ringbuf_memcpy_from(data, rb1, len);
    }
    size_t copyData(void *data, size_t len)
    {
        return ringbuf_copy_to_data(data, rb1, len);
    }
};

class ClientServer
{
public:
    bool bExit;
    ClientServer():bExit(false)
    {
    }
    ~ClientServer()
    {
    }
    int tcpServer(int port);

private:
    std::map<int, TcpClinetConn> tcpClinetConnMap;
    void handle_accept(int sfd, int efd);
    void handle_read(int fd);
    void process_business(int fd);
};

#endif
