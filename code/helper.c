#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include "ece454rpc_types.h"

void print_dirType(unsigned char d_type)
{
    switch( d_type )
    {
        case DT_UNKNOWN:
            printf("DT_UNKNOWN");
            break;
        case DT_REG:
            printf("DT_REG");
            break;
        case DT_DIR:
            printf("DT_DIR");
            break;
        case DT_FIFO:
            printf("DT_FIFO");
            break;
        case DT_CHR:
            printf("DT_CHR");
            break;
        case DT_LNK:
            printf("DT_LNK");
            break;
        default:
            printf("d_type=%u", d_type);
            break;
    }
}
