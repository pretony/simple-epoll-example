#include "clientServer.h"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("use: ./main-test 9999\n");
        return 0;
    }
    ClientServer clientServer;
    clientServer.tcpServer(atoi(argv[1]));
    return 0;
}
