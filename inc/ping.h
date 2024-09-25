#ifndef PING
#define PING

#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

extern int pingloop;

#define PING_PKT_S 64
#define RECV_TIMEOUT 1 // timeout for receving packet (in seconds)

typedef struct s_ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
}t_ping_pkt;


typedef struct s_info
{
    int ttl;
    int sleep_rate;
} t_info;

/* error.c */
void fatal_perror(char *error);
void fatal_error(char *error);

/* parameters.c */
t_info manage_args(char **args);

#endif