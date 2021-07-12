#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h>
#if _WIN32
// #include <wininet.h>
#include <WinSock2.h>
#include <Windows.h>
#define close closesocket 

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")
// 编译时应该加上-lwsock32
#elif __linux__
#include <netinet/in.h> // socketaddr_in
#include <sys/types.h>  //
#include <sys/socket.h> // socket
#include <arpa/inet.h>  // htons inet_addr
#include <unistd.h>     // close
#endif

#define N 128

/**
 * UDP接收数据需要确定的IP地址和port
 */
/**
 * 使用bind函数，来完成地址结构与socket套接字的绑定，这样ip、port就固定了
 * 发送端在sendto函数中指定接收端的ip、port就可以发送数据了
 * 
 * 由于服务器是被动的，客户端是主动的，所以一般先运行服务器，后运行客户端，
 * 所以服务器需要固定自己的信息（ip、port）
 * 这样客户端才可以找到服务器并与之通信，但是客户端一般不需要bind绑定
 * 因为系统会自动的给客户端分配ip地址和端口号
 * 当然如果一定要bind也是可以的
 */

// int bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);

/**
 * 接收UDP数据，并将源地址信息保存在from指向的结构中
 * from和addrlen可以为NULL，表示不保存数据来源
 * 
 * @param sockfd 套接字
 * @param buf 接收数据缓冲区
 * @param nbytes 接收数据缓冲区大小
 * @param flags 套接字标志
 * @param from 源地址结构体指针，用来保存数据的来源
 * @param addrlen from所指内容的长度
 */
// ssize_t recvfrom(int sockfd, void *buf, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen);

int main(int argc, char const *argv[])
{
    // ./udp_net_client.out 192.168.226.11 8080
    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <ip:!> <port:!> \n", argv[0]);
        exit(1);
    }

    // 1、创建套接字

#if _WIN32
    WSADATA ws;
    //增加动态库引用，并加载进来（一定要在socket函数前调用，需要释放）。
    WSAStartup(MAKEWORD(2, 2), &ws);

#endif

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("fail to socket");
        exit(1);
    }

    printf("sockfd = %d\n", sockfd);

    // 2、将服务器的网络信息结构体绑定前进行填充
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    // 3、将网络信息结构体与套接字绑定
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("fail to bind ");
        exit(1);
    }

    // 4、接收数据
    char buf[N] = "";
    struct sockaddr_in clientaddr;

#if __linux__
    socklen_t addrlen = sizeof(clientaddr);
#elif _WIN32
    int addrlen = sizeof(clientaddr);
#endif

    while (1)
    {
        printf("-");

        if (recvfrom(sockfd, buf, N, 0, (struct sockaddr *)&clientaddr, &addrlen) == -1)
        {
            perror("fail to recvfrom");
            exit(1);
        }

        // 打印ip和端口号
        printf("\n%s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        // 打印数据
        printf("%s\n", buf);
    }

    close(sockfd);

    return 0;
}
