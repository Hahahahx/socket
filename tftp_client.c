#include <stdio.h>  // printf
#include <stdlib.h> // exit
#include <string.h>

#if __linux__

#include <sys/types.h>
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // htons inet_addr
#include <sys/stat.h>
#include <unistd.h> // close
#include <fcntl.h>  // close

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

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/**
 *              
 * 读写请求     |   操作码 1/2  |      文件名      | 0 |       模式        | 0 |   选项1   | 0 |   值1    | 0 |    值n     |...
 *                  2字节           n字节string     1B     n字节string 
 *
 * 数据包       |   操作码 3  |      块编号      |            数据            |
 *                  2字节            2字节                  512字节                                             
 * 
 * ACK          |   操作码 4  |      块编号       |
 *                  2字节            2字节
 * 
 * ERROR        |   操作码 5  |      差错码       |       差错信息        | 0 |
 *                  2字节            2字节              n字节string        1B
 * 
 * OACK         |   操作码 6  |       选项1       | 0 |   值1   | 0 |    值n    | 0 |
 *                  2字节          n字节string     1B  n字节string
 * 
 */

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server_ip!> <filename!>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serveraddr;

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

    // 填充服务器网络信息结构体
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(9090);

    do_download(sockfd, serveraddr, argv[2]);

    return 0;
}

int do_download(int sockfd, struct sockaddr_in serveraddr, char *filename)
{
    // char filename[128] = "";
    // printf("请输入要下载的文件名：");
    // scanf("%s", filename);

    // 给服务器发送消息，告知服务器执行下载操作
    unsigned char text[1024] = "";
    int text_len;
#if __linux__
    socklen_t addrlen = sizeof(struct sockaddr_in);
#elif _WIN32
    int addrlen = sizeof(struct sockaddr_in);
#endif

    int fd;
    int flags = 0;
    int num = 0;
    ssize_t bytes;

    // 构建读写请求，给服务器发送的tftp指令并发送给服务器,例如：01test.txt0octet0
    text_len = sprintf(text, "%d%s%d%s%d", 01, filename, 0, "octet", 0);

    // puts(text);
    // printf("send msg ");


    // printf("get text:%d%s",text[0], text);
    // puts(text);
    if (sendto(sockfd, text, text_len, 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
    {
        perror("fail to sendto ");
        exit(1);
    }

    while (1)
    {
        // 接收服务器发送过来的数据并处理
        if ((bytes = recvfrom(sockfd, text, sizeof(text), 0, (struct sockaddr *)&serveraddr, &bytes)) < 0)
        {
            perror("fail to recvform");
            exit(1);
        }

        printf("操作码：%d，块编号：%u\n", text[1], ntohs(*(unsigned short *)(text + 2)));
        // printf("数据：%s\n",text+4);

        // 判断操作码执行相应的处理
        if (text[1] == 5) //  5：ERROR
        {
            printf("error: %s\n", text + 4);
            return;
        }
        else if (text[1] == 3) // 3：数据包
        {
            if (flags == 0)
            {
                // 创建文件
                if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0064)) < 0)
                {
                    perror("fail to open");
                    exit(1);
                }
                flags = 1;
            }

            // 对比块编号和接收的数据大小并将文件写入文件
            if ((num + 1) == ntohs(*(unsigned short *)(text + 2))) // text+2 从块编号开始读取，short两个字节，也就是取到了块编号的值
            {
                if (likely(bytes == 516)) // 2（操作码）+ 2（快编号）+ 512（数据）
                {
                    printf("下载文件..");
                    num = ntohs(*(unsigned short *)(text + 2));
                    // issue：看不懂这个write用法
                    // +4 为了不写入前面两个块编号和操作码 -4 也是 516 - 4 = 512
                    if (write(fd, text + 4, bytes - 4) < 0)
                    {
                        perror("fail to write");
                        exit(1);
                    }

                    // 当一个数据包写完以后，给服务器发送ACK
                    text[1] = 4;
                    if (sendto(sockfd, text, 4, 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                    {
                        perror("fail to sendto");
                        exit(1);
                    }
                }
                // 当最后一个数据接收完毕后，写入文件后退出函数
                else if (bytes < 516)
                {
                    printf("最后一块");
                    if (write(fd, text + 4, bytes - 4) < 0)
                    {
                        perror("fail to write");
                        exit(1);
                    }

                    text[1] = 4;
                    if (sendto(sockfd, text, 4, 0, (struct sockaddr *)&serveraddr, addrlen) < 0)
                    {
                        perror("fail to sendto");
                        exit(1);
                    }

                    printf("文件下载完毕\n");
                    return;
                }
            }
        }
    }
}