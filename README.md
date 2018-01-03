# simple-epoll-example
<p>一个epoll LT的例子，每个链接有各自的read buf，还没有做write buf。目前只做单线程，buf使用开源的ring buf，消息采用json格式，前面加一个校验头。</p>
<p>编译：g++ -o main-test main-test.cpp clientServer.cpp jsoncpp.cpp ringbuf.cpp business.cpp -I. -std=c++11</p>
<p>客户端测试程序：client-test.cpp</p>
