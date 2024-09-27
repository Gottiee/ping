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
#include <math.h>


#define PING_PKT_S 64
#define RECV_TIMEOUT 1 // timeout for receving packet (in seconds)
#define MAX_PINGS 100000

typedef struct s_ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S - sizeof(struct icmphdr)];
}t_ping_pkt;

typedef struct s_to_ping
{
    char *target;
    struct s_to_ping *next;
} t_to_ping;

typedef struct s_info
{
    int ttl;
    int sleep_rate;
    char ip[1025];
    char domain[500];
    char reverse_domain[500];
    int return_ttl;
    unsigned int msg_receive;
    double min;
    double max;
    double times[MAX_PINGS];
    t_to_ping *start;
    u_int16_t sequence;
    u_int16_t id; 
} t_info;

typedef struct s_global_send
{
    int sockfd;
    t_ping_pkt *pckt;
    struct sockaddr_in *ping_addr;
    struct timespec *time_loop_start;
    t_info *info;
    int *msg_count;
} t_global_send;

extern int nbr_loop;
extern t_global_send send_data;

/* signal.c */
void loop_handler(int signal);

/* error.c */
void fatal_perror(char *error, t_to_ping *start);
void fatal_error(char *error, t_to_ping *start);

/* parameters.c */
void set_info(t_info *info);
void handle_args(t_to_ping *start, char **argv, t_info *info);
void free_list(t_to_ping *start);

/* print.c */
void print(struct timespec time_loop_start, t_info *info);
void print_end_loop(struct timespec *time_start, t_info *info, int msg_count);
void print_header(t_info *info);
void print_usage();

/* ping.c */
void send_ping();

#endif