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

char *dns_lookup(char *domain, char *ip, struct sockaddr_in *addr_con)
{
    printf("\nResolving DNS...\n");
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((getaddrinfo(domain, NULL, &hints, &res)) != 0)
    {
        freeaddrinfo(res);
        return NULL;
    }
    addr_con->sin_family = AF_INET;
    addr_con->sin_port = htons(0);
    addr_con->sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    strcpy(ip, inet_ntoa(addr_con->sin_addr));
    freeaddrinfo(res);
    return ip;
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
        addr_con->sin_port = htons(0);  // port ?
        strncpy(ip, input, 1024);
        return ip;
    }
    dns_lookup(input, ip, addr_con);
    if (!ip)
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

void ping_loop(int sockfd, t_info info, struct sockaddr_in *ping_addr)
{
    int flag, msg_count = 0;
    socklen_t addr_len;
    long unsigned i;
    char rbuffer[500];
    t_ping_pkt pckt;

    (void)flag;

    struct sockaddr_in r_addr;

    // struct timespec time_start, time_end, tfs, tfe;
    // long double rtt_msec = 0, total_msec = 0;

    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
    
    if (setsockopt(sockfd, SOL_IP, IP_TTL, &info.ttl, sizeof(info.ttl)) != 0)
        fatal_perror("Error: setup ttl to socket");
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out)) != 0)
        fatal_perror("Error: setup timeout socket");
    
    while (nbr_loop)
    {
        flag = 1;
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();
        for (i = 0; i < sizeof(pckt.msg) - 1; i++)
            pckt.msg[i] = i + '0';
        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt,sizeof(pckt));

        usleep(info.sleep_rate);

        //send packet
        if (sendto(sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr *)ping_addr, sizeof(*ping_addr)) <= 0 )
        {
            printf("\nPacket Sending Failed\n");
            flag = 0;
        }

        // receive packet
        addr_len = sizeof(r_addr);
        if (recvfrom(sockfd, rbuffer, sizeof(rbuffer), 0, (struct sockaddr *)&r_addr, &addr_len) <= 0 && msg_count > 1)
            printf("packet receive failed\n");
        else
        {
            printf("packet receive\n");
        }
    }
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
    ping_loop(sockfd, info, &addr_con);
    printf("ip : %s\n", ip_addr);
    free(ip_addr);
}