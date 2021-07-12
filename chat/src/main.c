#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main(int argc, char const *argv[])
{
    int opt;
    char *opt_sht = "i::m::";
    int opt_index = 0;

    static struct option opt_long[] = {
        {"ip", required_argument, NULL, 'i'},
        {"msg", required_argument, NULL, 'm'},
    };

    while ((opt = getopt_long(argc, argv, opt_sht, opt_long, &opt_index)) != -1)
    {
        switch (opt)
        {
        case 'i':
            
            break;
        case 'm':
            break;
        default:
            break;
        }
    }

    return 0;
}
