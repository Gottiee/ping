#include "../inc/ping.h"

int nbr_loop = 1;
t_global_send send_data;

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

char *reverse_dns_lookup(char *ip_addr)
{
    struct sockaddr_in temp_addr;
    socklen_t len;
    char buf[500], *ret_buf;

    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);

    if (getnameinfo((struct sockaddr *)&temp_addr, len, buf, sizeof(buf), NULL, 0, NI_NAMEREQD)) {
        perror("Error in reverse dns");
        return NULL;
    }

    ret_buf = (char *)malloc((strlen(buf) + 1) * sizeof(char));
    strcpy(ret_buf, buf);
    return ret_buf; 
}

bool dns_lookup(char *input_domain, char *ip, struct sockaddr_in *addr_con)
{
    int value;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((value = getaddrinfo(input_domain, NULL, &hints, &res)) != 0)
    {
        return false;
    }
    addr_con->sin_family = AF_INET;
    addr_con->sin_port = htons(0);
    addr_con->sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    strcpy(ip, inet_ntoa(addr_con->sin_addr));
    freeaddrinfo(res);
    return true;
}

bool fill_sockaddr_in(struct sockaddr_in *addr_con, char *target) 
{
    memset(addr_con, 0, sizeof(struct sockaddr_in));
    send_data.info->domain[0] = '\0';
    send_data.info->ip[0] = '\0' ;

    // tcheck si c'est une address ipv4
    if (inet_pton(AF_INET, target, &(addr_con->sin_addr)) == 1) {
        addr_con->sin_family = AF_INET;
        addr_con->sin_port = htons(0);
        strncpy(send_data.info->ip, target, 1024);
        return true;
    }
    if (!dns_lookup(target, send_data.info->ip, addr_con))
        fatal_error("ping: unknown host\n");
    strncpy(send_data.info->domain, target, 499);
    char *tmp = reverse_dns_lookup(send_data.info->ip);
    if (!tmp)
        return false;
    strncpy(send_data.info->reverse_domain, tmp, 499);
    free(tmp);
    return true;
}

int socket_creation()
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1)
        return -1;
    return sockfd;
}

bool setup_socket(t_info *info)
{
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    
    if (setsockopt(send_data.sockfd, SOL_IP, IP_TTL, &info->ttl, sizeof(info->ttl)) != 0)
    {
        perror("Error: setup ttl to socket");
        return false;
    }
    if (setsockopt(send_data.sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out)) != 0)
    {
        perror("Error: setup timeout socket");
        return false;
    }
    return true;
}

void fill_icmp(t_ping_pkt *pckt, int *msg_count)
{
    long unsigned int i;

    send_data.info->id = rand();
    bzero(pckt, sizeof(t_ping_pkt));
    pckt->hdr.type = ICMP_ECHO;
    pckt->hdr.un.echo.id = send_data.info->id;
    for (i = 0; i < sizeof(pckt->msg) - 1; i++)
        pckt->msg[i] = i + '0';
    pckt->msg[i] = 0;
    pckt->hdr.un.echo.sequence = htons(*msg_count);
    pckt->hdr.checksum = checksum(pckt,sizeof((*pckt)));
}

void send_ping()
{
    fill_icmp(send_data.pckt, send_data.msg_count);
    clock_gettime(CLOCK_MONOTONIC, send_data.time_loop_start);
    if (sendto(send_data.sockfd, send_data.pckt, sizeof(t_ping_pkt), 0, (struct sockaddr *)send_data.ping_addr, sizeof(*send_data.ping_addr)) <= 0 )
        fatal_perror("Packet Sending Failed");
    (*send_data.msg_count) ++;
    alarm(1);
}

bool receive_ping()
{
    char rbuffer[128];

    if (recvfrom(send_data.sockfd, rbuffer, sizeof(rbuffer), 0, NULL, NULL) <= 0)
        return false;
    struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + sizeof(struct iphdr));
    if (recv_hdr->un.echo.id != send_data.info->id || recv_hdr->type == 8)
        return false;
    send_data.info->sequence = htons(recv_hdr->un.echo.sequence);
    if (!(recv_hdr->type == 0 && recv_hdr->code == 0))
    {
        print_error_code(recv_hdr->type, recv_hdr->code);
        return false;
    }
    struct iphdr *recv_ip = (struct iphdr *)rbuffer;
    send_data.info->return_ttl = recv_ip->ttl;
    send_data.info->msg_receive++;
    return true;
}

void reset_info()
{
    send_data.info->msg_receive = 0;
    send_data.info->min = 0;
    send_data.info->max = 0;
    memset(send_data.info->times, MAX_PINGS, sizeof(double));
}

void fill_global_send(t_ping_pkt *pckt, struct timespec *time_loop_start, int *msg_count)
{
    send_data.pckt = pckt;
    send_data.time_loop_start = time_loop_start;
    send_data.msg_count = msg_count;
}

void ping_loop()
{
    t_ping_pkt pckt;
    int msg_count = 0;
    struct timespec time_loop_start,  time_start;
    reset_info();
    if (!setup_socket(send_data.info))
        return;
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    fill_global_send(&pckt, &time_loop_start, &msg_count);
    send_ping();
    while (nbr_loop)
    {
        if (receive_ping())
            print(time_loop_start, send_data.info);
    }
    print_end_loop(&time_start, send_data.info, msg_count);
}

void do_all_ping(struct sockaddr_in *addr_con, char *target)
{
    if (!fill_sockaddr_in(addr_con, target))
        return;
    send_data.ping_addr = addr_con;
    print_header();
    ping_loop();
}

void loop_for_all_domain(t_to_ping *tmp)
{
    struct sockaddr_in addr_con;

    while (tmp)
    {
        do_all_ping(&addr_con, tmp->target);
        tmp = tmp->next;
        nbr_loop = 1;
    }
}

int main(int argc, char **argv)
{
    t_info info;
    send_data.info = &info;

    if (argc == 1)
        print_usage();
    init_info();
    send_data.sockfd = socket_creation();
    if (send_data.sockfd == -1)
        fatal_error("ping: Lacking privilege for icmp socket.\n");
    send_data.info->start = malloc(sizeof(t_to_ping));
    if (!send_data.info->start)
        fatal_perror("Error in malloc");
    handle_args(&argv[1]);
    srand(time(NULL));
    signal(SIGINT, loop_handler);
    signal(SIGALRM, loop_handler);
    loop_for_all_domain(send_data.info->start);
    free_list(send_data.info->start);
    close(send_data.sockfd);
}