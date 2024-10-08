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
    bool verbose;
    ssize_t size_recv;
    bool quiet;
    int count;
    int recv_timeout;
    int interval;
} t_info;

typedef struct s_global_send
{
    int sockfd;
    t_ping_pkt *pckt;
    struct sockaddr_in *ping_addr;
    struct timespec *time_loop_start;
    t_info *info;
    int *msg_count;
    struct iphdr *error_ip;
    struct icmphdr *error_icmp;
} t_global_send;

extern int nbr_loop;
extern t_global_send send_data;

/* signal.c */
void loop_handler(int signal);

/* error.c */
void fatal_perror(char *error);
void fatal_error(char *error);
void fatal_error_parsing(char *error, char c, char *str);
void fatal_error_parsing_no_arg(char *error, char *option);

/* parameters.c */
void init_info();
void handle_args(char **argv);
void free_list(t_to_ping *start);

/* print.c */
void print(struct timespec time_loop_start, t_info *info);
void print_end_loop(t_info *info, int msg_count);
void print_header();
void print_usage();
void print_error_code(int type, int code);
void print_error_usage(char c);
void print_option();

/* ping.c */
void send_ping();

#endif