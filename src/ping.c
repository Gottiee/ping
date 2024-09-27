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

bool fill_sockaddr_in(struct sockaddr_in *addr_con, char *target, t_info *info) 
{
    memset(addr_con, 0, sizeof(struct sockaddr_in));
    info->domain[0] = '\0';
    info->ip[0] = '\0' ;

    // tcheck si c'est une address ipv4
    if (inet_pton(AF_INET, target, &(addr_con->sin_addr)) == 1) {
        addr_con->sin_family = AF_INET;
        addr_con->sin_port = htons(0);
        strncpy(info->ip, target, 1024);
        return true;
    }
    if (!dns_lookup(target, info->ip, addr_con))
        fatal_error("ping: unknown host\n", info->start);
    strncpy(info->domain, target, 499);
    char *tmp = reverse_dns_lookup(info->ip);
    if (!tmp)
        return false;
    strncpy(info->reverse_domain, tmp, 499);
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

bool setup_socket(int sockfd, t_info *info)
{
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    
    if (setsockopt(sockfd, SOL_IP, IP_TTL, &info->ttl, sizeof(info->ttl)) != 0)
    {
        perror("Error: setup ttl to socket");
        return false;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out)) != 0)
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
        fatal_perror("Packet Sending Failed", send_data.info->start);
    (*send_data.msg_count) ++;
    alarm(1);
}

bool receive_ping(int sockfd, t_info *info)
{
    char rbuffer[128];

    if (recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, NULL, NULL) <= 0)
        return false;
    struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + sizeof(struct iphdr));
    if (recv_hdr->un.echo.id != send_data.info->id)
        return false;
    if (!(recv_hdr->type == 0 && recv_hdr->code == 0))
    {
        printf("Error... Packet received with ICMP type %d code %d\n", recv_hdr->type, recv_hdr->code);
        // print_error_code();
        return false;
    }
    struct iphdr *recv_ip = (struct iphdr *)rbuffer;
    info->return_ttl = recv_ip->ttl;
    info->msg_receive++;
    info->sequence = htons(recv_hdr->un.echo.sequence);
    return true;
}

void reset_info(t_info *info)
{
    info->msg_receive = 0;
    info->min = 0;
    info->max = 0;
    memset(info->times, MAX_PINGS, sizeof(double));
}

void fill_global_send(int sockfd, t_ping_pkt *pckt, struct sockaddr_in *ping_addr, struct timespec *time_loop_start, t_info *info, int *msg_count)
{
    send_data.sockfd = sockfd;
    send_data.pckt = pckt;
    send_data.ping_addr = ping_addr;
    send_data.time_loop_start = time_loop_start;
    send_data.info = info;
    send_data.msg_count = msg_count;
}

void ping_loop(int sockfd, t_info *info, struct sockaddr_in *ping_addr)
{
    t_ping_pkt pckt;
    int msg_count = 0;
    struct timespec time_loop_start,  time_start;
    reset_info(info);

    if (!setup_socket(sockfd, info))
        return;
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    fill_global_send(sockfd, &pckt, ping_addr, &time_loop_start, info, &msg_count);
    send_ping(sockfd, &pckt, ping_addr, &time_loop_start, info, &msg_count);
    while (nbr_loop)
    {
        if (receive_ping(sockfd, info))
            print(time_loop_start, info);
    }
    print_end_loop(&time_start, info, msg_count);
}

void do_all_ping(t_info *info, struct sockaddr_in *addr_con, char *target, int sockfd)
{
    if (!fill_sockaddr_in(addr_con, target, info))
        return;
    print_header(info);
    signal(SIGINT, loop_handler);
    signal(SIGALRM, loop_handler);
    ping_loop(sockfd, info, addr_con);
}

int main(int argc, char **argv)
{
    struct sockaddr_in addr_con;
    t_info info;
    t_to_ping *tmp;

    if (argc == 1)
        print_usage();
    int sockfd = socket_creation();
    if (sockfd == -1)
        fatal_error("ping: Lacking privilege for icmp socket.\n", NULL);
    t_to_ping *start = malloc(sizeof(t_to_ping));
    if (!start)
        fatal_perror("Error in malloc", info.start);
    handle_args(start, &argv[1], &info);
    srand(time(NULL));
    
    tmp = start;
    while (tmp)
    {
        do_all_ping(&info, &addr_con, tmp->target, sockfd);
        tmp = tmp->next;
        nbr_loop = 1;
    }

    free_list(start);
    close(sockfd);
}