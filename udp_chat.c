#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h>
#if _WIN32
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
#include <ifaddrs.h>    // ifaddr
#include <netdb.h>      // gethostname
#include <pthread.h>
#endif

pthread_mutex_t mut;

#define N 128
#if __linux__

static void getip_byhsot()
{
    char host_name[128];
    struct hostent *hent;
    int i;

    gethostname(host_name, sizeof(host_name));
    hent = (struct hostent *)gethostbyname(host_name);
    printf("hostname:%s\tip:", hent->h_name);
    for (i = 0; hent->h_addr_list[i]; i++)
    {
        printf("%s\t", inet_ntoa(*(struct in_addr *)(hent->h_addr_list[i])));
    }
}

static int getip(char *host)
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("fail to getifaddr");
        exit(1);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if (!strcmp(ifa->ifa_name, "lo"))
            continue;
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                return -1;
            }
            freeifaddrs(ifaddr);
            return 0;
        }
    }
    return -1;
}

#elif _WIN32

#define MAX_PATH 260

static int getip(char *ip)
{
    // char hostname[MAX_PATH] = {0};
    // gethostname(hostname, MAX_PATH);
    // struct hostent FAR *lpHostEnt = gethostbyname(hostname);
    // if (lpHostEnt == NULL)
    // {
    //     return -1;
    // }
    // printf("ip----------------------------------\n");

    // // 取得IP地址列表中的第一个为返回的IP(因为一台主机可能会绑定多个IP)
    // LPSTR lpAddr = lpHostEnt->h_addr_list[0];

    // // 将IP地址转化成字符串形式
    // struct in_addr inAddr;
    // memmove(&inAddr, lpAddr, 4);
    // printf("ip:%s\n", inet_ntoa(inAddr));
    // strcpy(ip, inet_ntoa(inAddr));

    WSADATA wsaData;
    char name[155];
    // char *ip;
    PHOSTENT hostinfo;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
    {
        if (gethostname(name, sizeof(name)) == 0)
        {
            if ((hostinfo = gethostbyname(name)) != NULL)
            {
                strcpy(ip, inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list));
            }
        }
        WSACleanup();
    }

    return 0;
}
#endif

struct sendparam
{
    int sockfd;
    struct sockaddr_in *sendtoaddr;
};

void sendfun(void *args)
{

    struct sendparam *temp;
    temp = (struct sendparam *)args;
    char sendbuf[N] = "";
    printf("sendfun-sockfd: %d\n", temp->sockfd);

    struct sockaddr_in sendtoaddr;
    sendtoaddr.sin_family = AF_INET;
    sendtoaddr.sin_addr.s_addr = inet_addr("192.168.50.219");
    sendtoaddr.sin_port = htons(9999);

    //pthread_mutex_lock(&mut);

    fgets(sendbuf, N, stdin);
    // 输入的字符含有无用的字符，比如\n, 将其转换为\0
    // sendbuf[strlen(sendbuf) - 1] = '\0';
    printf("向%s:%d发送了消息：%s\n", inet_ntoa(temp->sendtoaddr->sin_addr), ntohs(temp->sendtoaddr->sin_port), sendbuf);

    if (sendto(temp->sockfd, sendbuf, N, 0, (struct sockaddr *)&sendtoaddr, sizeof(sendtoaddr)) == -1)
    {
        perror("fail to sendto");
        exit(1);
    }
    // pthread_mutex_unlock(&mut);
}

void recvfun(int sockfd)
{

    char recvbuf[N] = "";
    struct sockaddr_in clientaddr;

#if __linux__
    socklen_t addrlen = sizeof(clientaddr);
#elif _WIN32
    int addrlen = sizeof(clientaddr);
#endif

    printf("recvfun-sockfd: %d\n", sockfd);
    //pthread_mutex_lock(&mut);
    if (recvfrom(sockfd, recvbuf, N, 0, (struct sockaddr *)&clientaddr, &addrlen) == -1)
    {
        perror("fail to recvfrom");
        exit(1);
    }
    // 打印ip和端口号
    printf("%s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    // 打印数据
    printf("%s\n", recvbuf);
    //pthread_mutex_unlock(&mut);
}

int main(int argc, char const *argv[])
{

    // ./udp_net_client.out 192.168.226.11 8080
    if (argc < 2)
    {
        fprintf(stderr, "Usage:%s <ip:!> \n", argv[0]);
        exit(1);
    }

#if _WIN32
    WSADATA ws;
    //增加动态库引用，并加载进来（一定要在socket函数前调用，需要释放）。
    WSAStartup(MAKEWORD(2, 2), &ws);

#endif
    // 1、创建套接字
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("fail to socket");
        exit(1);
    }

    // 2、将服务器的网络信息结构体绑定前进行填充

    char ip[16];
    if (getip(ip) == -1)
    {
        perror("fail to get local ip");
        exit(1);
    }
    printf("current ip:%s\n", ip);
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(atoi("9999"));

    // 3、将网络信息结构体与套接字绑定
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("fail to bind ");
        exit(1);
    }

    // 4、发送与接收数据
    char recvbuf[N] = "";
    char sendbuf[N] = "";

    struct sockaddr_in sendtoaddr;
    sendtoaddr.sin_family = AF_INET;
    sendtoaddr.sin_addr.s_addr = inet_addr(argv[1]);
    sendtoaddr.sin_port = htons(9999);

    // #if __linux__
    //     socklen_t addrlen = sizeof(clientaddr);
    // #elif _WIN32
    //     int addrlen = sizeof(clientaddr);
    // #endif
    struct sendparam *args;
    args = (struct sendparam *)malloc(sizeof(struct sendparam));
    args->sockfd = sockfd;
    args->sendtoaddr = &sendtoaddr;

    while (1)
    {
        pthread_mutex_init(&mut, NULL);
        pthread_t trecv;
        pthread_t tsend;

        pthread_create(&trecv, NULL, recvfun, sockfd);
        pthread_create(&tsend, NULL, sendfun, args);
        pthread_join(trecv, NULL);
        pthread_join(tsend, NULL);
        // if (recvfrom(sockfd, recvbuf, N, 0, (struct sockaddr *)&clientaddr, &addrlen) == -1)
        // {
        //     perror("fail to recvfrom");
        //     exit(1);
        // }
        // // 打印ip和端口号
        // printf("%s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        // // 打印数据
        // printf("%s\n", recvbuf);

        // fgets(sendbuf, N, stdin);
        // // 输入的字符含有无用的字符，比如\n, 将其转换为\0
        // // sendbuf[strlen(sendbuf) - 1] = '\0';

        // if (sendto(sockfd, sendbuf, N, 0, (struct sockaddr *)&sendtoaddr, sizeof(sendtoaddr)) == -1)
        // {
        //     perror("fail to sendto");
        //     exit(1);
        // }
    }

    close(sockfd);

    return 0;
}
