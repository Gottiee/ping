#include "../inc/ping.h"

int nbr_loop = 1;

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

bool dns_lookup(char *domain, char *ip, struct sockaddr_in *addr_con)
{
    int value;

    printf("\nResolving DNS...\n");
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((value = getaddrinfo(domain, NULL, &hints, &res)) != 0)
    {
        freeaddrinfo(res);
        free(ip);
        return false;
    }
    addr_con->sin_family = AF_INET;
    addr_con->sin_port = htons(0);
    addr_con->sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    strcpy(ip, inet_ntoa(addr_con->sin_addr));
    freeaddrinfo(res);
    return true;
}

char *fill_sockaddr_in(struct sockaddr_in *addr_con, char *input) 
{
    int addrlen = sizeof(addr_con);
    (void)addrlen;

    char *ip = (char *)malloc(sizeof(char) * 1025);
    if (!ip)
        fatal_perror("Error: malloc");

    memset(addr_con, 0, sizeof(struct sockaddr_in));

    // tcheck si c'est une address ipv4
    if (inet_pton(AF_INET, input, &(addr_con->sin_addr)) == 1) {
        addr_con->sin_family = AF_INET;
        addr_con->sin_port = htons(0);
        strncpy(ip, input, 1024);
        return ip;
    }
    if (!dns_lookup(input, ip, addr_con))
        fatal_error("Host name not knows");
    return ip;
}

int socket_creation()
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd == -1)
        return -1;
    return sockfd;
}

void setup_socket(int sockfd, t_info info)
{
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    
    if (setsockopt(sockfd, SOL_IP, IP_TTL, &info.ttl, sizeof(info.ttl)) != 0)
        fatal_perror("Error: setup ttl to socket");
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out)) != 0)
        fatal_perror("Error: setup timeout socket");
}

void send_ping(int sockfd, t_ping_pkt *pckt, struct sockaddr_in *ping_addr)
{
    if (sendto(sockfd, pckt, sizeof(t_ping_pkt), 0, (struct sockaddr *)ping_addr, sizeof(*ping_addr)) <= 0 )
        fatal_perror("Packet Sending Failed");
}

bool receive_ping(int sockfd)
{
    char rbuffer[128];

    if (recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, NULL, NULL) <= 0 /*&& *msg_count > 1 */)
        printf("packet receive failed\n");
    else
    {
        struct icmphdr *recv_hdr = (struct icmphdr *)(rbuffer + sizeof(struct iphdr));
        if (!(recv_hdr->type == 0 && recv_hdr->code == 0))
        {
            printf("Error... Packet received with ICMP type %d code %d\n", recv_hdr->type, recv_hdr->code);
            return false;
        }
        return true;
    }
    return false;
}

void fill_icmp(t_ping_pkt *pckt, int *msg_count)
{
    long unsigned int i;

    bzero(pckt, sizeof(t_ping_pkt));
    pckt->hdr.type = ICMP_ECHO;
    pckt->hdr.un.echo.id = getpid();
    for (i = 0; i < sizeof(pckt->msg) - 1; i++)
        pckt->msg[i] = i + '0';
    pckt->msg[i] = 0;
    pckt->hdr.un.echo.sequence = htons(*msg_count);
    *msg_count = *msg_count + 1;
    pckt->hdr.checksum = checksum(pckt,sizeof((*pckt)));
}

void ping_loop(int sockfd, t_info info, struct sockaddr_in *ping_addr)
{
    t_ping_pkt pckt;
    int msg_count = 1;
    struct timespec time_loop_start, time_loop_end, time_start, time_end;

    (void)time_loop_end;
    (void)time_loop_start;
    (void)time_end;

    // long double rtt_msec = 0, total_msec = 0;

    setup_socket(sockfd, info);
    clock_gettime(CLOCK_MONOTONIC, &time_start);

    while (nbr_loop)
    {
        fill_icmp(&pckt, &msg_count);
        usleep(info.sleep_rate);
        send_ping(sockfd, &pckt, ping_addr);
        if (receive_ping(sockfd))
            print();
    }
    printf("fin de la boucle\n");
}

int main(int argc, char **argv)
{
    struct sockaddr_in addr_con;
    char *ip_addr = NULL;

    (void)argc;
    int sockfd = socket_creation();
    if (sockfd == -1)
        fatal_perror("Error: socket creation");

    //modifier, je ne dois pas prendre argv1 normalement
    ip_addr = fill_sockaddr_in(&addr_con, argv[1]);

    t_info info = manage_args(argv);
    signal(SIGINT, loop_handler);
    ping_loop(sockfd, info, &addr_con);
    printf("ip : %s\n", ip_addr);
    free(ip_addr);
}