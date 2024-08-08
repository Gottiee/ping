#include "../inc/ping.h"

int socket_creation()
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1)
        return -1;
    return sockfd;
}

struct icmp_header *create_struct(char *buffer)
{
    struct 
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    int sockfd = socket_creation();
    if (sockfd == -1)
        fatal_perror("Fatal error");
    char buffer[1024];
    create_struct((char *)buffer);
}