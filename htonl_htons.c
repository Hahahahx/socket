#include <stdio.h>
#if _WIN32
#include <windows.h>
#pragma comment(lib, "wsock32.lib")
#elif __linux__
#include <arpa/inet.h>
#endif

int main()
{

    int a = 0x12345678;
    short b = 0x1234;

    printf("%#x\n", htonl(a));
    printf("%#x\n", htons(b));

    return 0;
}