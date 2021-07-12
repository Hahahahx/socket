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

// struct int_addr
// {
//     // 32位的整型数据
//     // 4字节，就是ip地址，就是通过s_addr来确定ip地址的
//     int_addr_t s_addr;
// };

// struct sockaddr_in
// {
//     // 2字节，协议族
//     sa_family_t sin_family;
//     // 2字节，端口号
//     int_port_t sin_port;
//     // 4字节，ip地址
//     struct int_addr sin_addr;
//     // 8字节
//     char sin_zero[8]
// };

/**
 * 与上面的区别而言，此结构体是通用的，也就是
 * 为了使不同格式地址能被传入套接字函数，地址必须要签字转换成通用的套接字地址结构
 */
// struct sockaddr
// {
//     // 2字节
//     sa_family_t sa_family;
//     // 14字节
//     char sa_data[14];
// };

/**
 * 在定义源地址和目的地址结构的时候，一般选用struct sockaddr_in
 * struct sockaddr_in my_addr;
 * 
 * 当调用编程接口函数，且该函数需要传入地址结构时需要用struct sockaddr进行强制转换
 * bind(socketfd,(struct sockaddr*)&my_addr,size_of(my_addr));
 * 
 */

/**
 * 发送数据
 * 向to结构体指针中指定的ip，发送udp数据
 * 
 * sockfd   套接字
 * buf      发送是数据缓冲区
 * nbytes   发送数据缓冲区大小
 * flags    一般为0
 * to       指向目的主机地址结构体的指针
 * addrlen  to所指向内容的长度
 * 
 * 
 * 通过 to 和 addrlen 来确定目的地址
 * 可以发送0长度的UDP数据包
 */
// ssize_t sendto(int sockfd, const void *buf, size_t nbytes, int flags, const struct sockaddr *to, socklen_t addrlen);

int main(int argc, char const *argv[])
{
    // ./udp_net_client.out 192.168.226.11 8080
    if (argc < 3)
    {
        fprintf(stderr, "Usage:%s <ip:!> <port:!> \n", argv[0]);
        exit(1);
    }

    // 1、创建socket

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

    // 2、填充服务器网络信息结构体，sockaddr_in
    struct sockaddr_in serveraddr;
#if __linux__
    socklen_t addrlen = sizeof(serveraddr);
#elif _WIN32
    int addrlen = sizeof(serveraddr);
#endif
    serveraddr.sin_family = AF_INET;                 // 协议族，AF_INET：ipv4网络协议
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]); // inet_addr("192.168.226.11"); // ip地址
    /**
     * 由于不确定主机字节序，所以同意进行转换成网络的大端字节序
     * 而客户端的端口也就是该应用程序的端口号，如果没有指定，则会随机分配一个
     */
    serveraddr.sin_port = htons(atoi(argv[2])); // htons(8080);

    // 3、发送数据
    char buf[N] = "";
    while (1)
    {
        fgets(buf, N, stdin);

        // 输入的字符含有无用的字符，比如\n, 将其转换为\0
        buf[strlen(buf) - 1] = '\0';

        if (sendto(sockfd, buf, N, 0, (struct sockaddr *)&serveraddr, addrlen) == -1)
        {
            perror("fail to sendto");
            exit(1);
        }
    }

    // 4、关闭套接字

    close(sockfd);

    return 0;
}