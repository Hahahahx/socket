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
#endif

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
static int getip(char *ip)
{
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
