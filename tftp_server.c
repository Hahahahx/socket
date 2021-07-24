#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h>

#if __linux__

#include <sys/types.h>
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // htons inet_addr
#include <sys/stat.h>
#include <unistd.h>  // close
#include <fcntl.h>   // close
#include <ifaddrs.h> // ifaddr
#include <netdb.h>   // gethostname

#elif _WIN32
#include <WinSock2.h>
#include <Windows.h>
#define close closesocket
#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR 0x0002
#define O_APPEND 0x0008
#define O_CREAT 0x0100
#define O_EXCL 0x0400
#define O_TRUNC 0x0200

// 编译时应该加上-lwsock32
#endif
#define MAX_PATH 260

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

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int sockfd;
struct sockaddr_in clientaddr;

int main(int argc, char const *argv[])
{
    printf("listen running...\n");
#if _WIN32
    WSADATA ws;
    //增加动态库引用，并加载进来（一定要在socket函数前调用，需要释放）。
    WSAStartup(MAKEWORD(2, 2), &ws);
#endif

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("fail to socket");
        exit(1);
    }

    char ip[MAX_PATH] = "";
    getip(&ip);

    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(9090);

    // 将网络信息结构体与套接字绑定
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("fail to bind ");
        exit(1);
    }
    printf("launch at %s:9090\n", ip);

#if __linux__
    socklen_t addrlen = sizeof(clientaddr);
#elif _WIN32
    int addrlen = sizeof(clientaddr);
#endif

    ssize_t bytes;

    // 客户端传过来的数据
    unsigned char text[513] = "";
    // 返回给客户端数据
    unsigned char buf[1024] = "";
    int buf_len;

    FILE *fd = NULL;
    short ack = 0;

    while (1)
    {

        // 接收到信息
        if ((recvfrom(sockfd, text, 512, 0, (struct sockaddr *)&clientaddr, &addrlen)))
        {
            printf("recv msg...\n");

            puts(text);

            unsigned int one;
            unsigned int two ;
            unsigned int three;
            unsigned int four ;
            unsigned char filename[1024] = "";
            unsigned char mode[1024] = "";

            printf("\n%c\n",text[0]);
            printf("\n%c\n",text[1]);


            sscanf(text, "%d%s[0]%s%d",&one, filename, mode, &three);
            printf("______________________________");
            printf( "[[[[[[%d%s%d%s%d",one, filename, mode, three);

            // unsigned short i;
            // unsigned char value[1024] = "";

            // // sscanf(text, "%d%d%s%c%s%c", &i,&i, value);
            // puts(text);

            // 判断客户端传过来的数据是什么
            if ( text[0] == 1) // 建立连接，传输文件
            {
                printf("open file.. %d", text[1]);
                if ((fd = open(filename, O_RDONLY)) < 0)
                {
                    buf_len = sprintf(buf, "%c%c%s%c%s%c", 0, 5, 404, "not found file", 0);
                    sendto(sockfd, buf, buf_len, 0, (struct sockaddr *)&clientaddr, addrlen);
                    continue;
                }
                fseek(fd, 0, SEEK_SET);
                read(fd, text, sizeof(text));
                ack++;
                buf_len = sprintf(buf, "%c%c%u%s%c%s", 0, 3, ack, text);
                sendto(sockfd, buf, buf_len, 0, (struct sockaddr *)&clientaddr, addrlen);
                printf("send to block for %d", ack);
                continue;
            }

            if (text[1] == 4) // 继续传输
            {
                fseek(fd, ack * 512, SEEK_SET);
                read(fd, text, sizeof(text));
                ack++;
                buf_len = sprintf(buf, "%c%c%u%s%c%s", 0, 3, ack, text);
                sendto(sockfd, buf, buf_len, 0, (struct sockaddr *)&clientaddr, addrlen);
                printf("send to block for %d", ack);
                continue;
            }
        }
    }

    return 0;
}
