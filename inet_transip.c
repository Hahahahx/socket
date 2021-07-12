#include <stdio.h>
#if _WIN32
#include <windows.h>
#pragma comment(lib, "wsock32.lib")
#elif __linux__
#include <arpa/inet.h>
#endif
int main()
{
    char ip_str[16] = "192.168.226.11";
    unsigned int ip_int = 0;
    unsigned char *ip_p = NULL;

    // 将点分十进制ip地址转化为32位无符号整型数据
    inet_pton(AF_INET, ip_str, &ip_int);
    printf("in_uint = %d\n", ip_int);

    ip_p = (char *)&ip_int;
    printf("in_uint=%d,%d,%d,%d", *ip_p, *(ip_p + 1), *(ip_p + 2), *(ip_p + 3));

    // 将数组转换成ip字符串
    unsigned char ip_int_arr[] = {192, 168, 226, 11};
    char ip_str[16] = "";

    inet_ntop(AF_INET, &ip_int_arr, ip_str, 16);
    printf("ip_s = %s\n", ip_str);


    /**
     * 上面的函数可以用于ipv6，而以下两个功能相同，但只能用于ipv4
     * 
     * ipv4字符串转换成有效地址
     * in_addr_t inet_addr(const char *cp);
     * 
     * ipv4地址转换为字符串
     * char *inet_ntoa(struct in_addr in);
     */

    return 0;
}