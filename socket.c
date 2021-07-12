#include <stdio.h>

#if _WIN32
#include <WinSock2.h>
#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")
#elif __linux__
#include <sys/socket.h>
#endif


/**
 * 网络通信要解决的是不同主机进程间的通信
 * 
 * 1、首要问题是网络将进程表示问题
 * 2、以及多重协议的识别问题
 * 20世纪80年代初，加州大学Berkeley分校在BSD（一个UNIX OS版本）系统内实现了TCP/IP协议，其网络程序编程开发接口为socket
 * 随着UNIX以及类UNIX操作系统的广泛应用，socket成为最流行的网络程序开发接口
 */


/**
 * socket 套接字
 * 是一种文件描述符，代表了一个通信管道的一个端点
 * 类似文件的操作（linux一切皆文件），可以使用read、write、close等函数对socket套接字进行网络数据的收取和发送等操作
 * 得到socket套接字（描述符）的方法调用socket() 
 * 
 * 分类
 * SOCK_STREAM，流式套接字，用于TCP
 * SOCK_DGRAM，数据报套接字，用于UDP
 * SOCK_RAW，原始套接字，对于其他层次的协议操作时需要使用这个类型
 * 
 * 服务端需要绑定ip，端口等信息
 * 服务端在recvfrom中阻塞等待客户端的sendto，随后应答请求sendto给客户端的recvfrom，
 * 进入循环，直到客户端close
 * udp服务端：socket() → bind() → recvfrom() → sendto()
 * udp客户端：socket() → sendto() → recvfrom() → close()
 * 
 * 
 * AF_UNIX 本地通信
 * AF_INET ipv4
 * AF_INET6 ipv6
 * AF_PACKET 底层接口
 * 
 * 
 * @param fmaily 协议族（AF_UNIX、AF_INET、AF_INET6、PF_PACKET等）
 * @param type 套接字类（SOCK_STREAM、SOCK_DGRAM、SOCK_RAW等）
 * @param protocol 协议类别（0、IPPROTO_TCP、IPPROTO_UDP等）
 */
int socket(int family, int type, int protocol);

int main()
{
    int socketfd;
    // 使用socket函数创建套接字
    // 创建一个用于UDP网络编程的套接字
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("fail to socket");
        exit(1);
    }

    printf("socketfd = %d", socketfd);

        return 0;
}