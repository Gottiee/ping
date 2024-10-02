#include "../inc/ping.h"

void print_option()
{
    printf("Usage: iping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf("Options:\n");
    printf("  -v                 verbose output\n");
    printf("  -?                 give this help list\n");
    free_list(send_data.info->start);
    exit(3);
}

void add_ping_time(double rtt, t_info *info) {
    if (info->min == 0 || info->min > rtt)
        info->min = rtt;
    if (info->max == 0 || info->max < rtt) 
        info->max = rtt;
    if (info->msg_receive < MAX_PINGS) {
        info->times[info->msg_receive - 1] = rtt;
    }
}

void print_seq()
{
    printf("%ld bytes from ", send_data.info->size_recv);
    printf("%s: ", send_data.info->ip);
}

void print(struct timespec time_loop_start, t_info *info)
{
    struct timespec time_loop_end;
    double rtt_msec = 0;

    if (!nbr_loop)
        return;
    clock_gettime(CLOCK_MONOTONIC, &time_loop_end);

    long sec_diff = time_loop_end.tv_sec - time_loop_start.tv_sec;
    long nsec_diff = time_loop_end.tv_nsec - time_loop_start.tv_nsec;

    if (nsec_diff < 0) {
        sec_diff -= 1;
        nsec_diff += 1000000000;
    }
    rtt_msec = sec_diff * 1000.0 + (nsec_diff / 1000000.0);
   
    print_seq();
    printf("icmp_seq=%d ", send_data.info->sequence);
    printf("ttl=%d time=%.3f ms\n", info->return_ttl, rtt_msec);
    add_ping_time(rtt_msec, info);
}

double calculate_mean(t_info *info) {
    double sum = 0.0;
    for (unsigned int i = 0; i < info->msg_receive; i++)
        sum += info->times[i];
    return sum / info->msg_receive;
}

double calculate_std_deviation(double mean, t_info *info)
{
    double sum_deviation = 0.0;
    
    for (unsigned int i = 0; i < info->msg_receive; i++) {
        sum_deviation += pow(info->times[i] - mean, 2);
    }
    
    return sqrt(sum_deviation / info->msg_receive);
}

void print_end_loop(struct timespec *time_start, t_info *info, int msg_count)
{
    (void)time_start;
    // struct timespec time_end;
    // double rtt_msec = 0;

    // clock_gettime(CLOCK_MONOTONIC, &time_end);

    // long sec_diff = time_end.tv_sec - time_start->tv_sec;
    // long nsec_diff = time_end.tv_nsec - time_start->tv_nsec;

    // if (nsec_diff < 0) {
    //     sec_diff -= 1;
    //     nsec_diff += 1000000000;
    // }
    // rtt_msec = sec_diff * 1000.0 + (nsec_diff / 1000000.0);

    if (strlen(info->domain))
        printf("--- %s ping statistics ---\n", info->domain);
    else
        printf("--- %s ping statistics ---\n", info->ip);

    float packet_loss = ((msg_count - info->msg_receive) / (double)msg_count) * 100;
    printf("%d packets transmitted, %d packets received, %.0f%% packet loss\n", msg_count, info->msg_receive, packet_loss);
    if (!send_data.info->msg_receive)
        return ;
    double average = calculate_mean(info);
    double dev = calculate_std_deviation(average, info);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", info->min, average, info->max, dev);
}

void print_header()
{
    if (strlen(send_data.info->domain))
        printf("PING %s (%s)", send_data.info->domain, send_data.info->ip);
    else
        printf("PING %s", send_data.info->ip);
    printf(": %d data bytes", PING_PKT_S - 8);
    if (send_data.info->verbose)
        printf(", id 0x%1$x = %1$d", send_data.info->id);
    printf("\n");
}

void print_usage()
{
    printf("ping: missing host operand\n");
    printf("Try 'ping --help' or 'ping --usage' for more information.\n");
    free_list(send_data.info->start);
    exit(1);
}

void print_unreachable(int code)
{
    switch(code) 
    {
        case ICMP_NET_UNREACH:
            printf("Destination Net Unreachable\n");
            break;
        case ICMP_HOST_UNREACH:
            printf("Destination Host Unreachable\n");
            break;
        case ICMP_PROT_UNREACH:
            printf("Destination Protocol Unreachable\n");
            break;
        case ICMP_PORT_UNREACH:
            printf("Destination Port Unreachable\n");
            break;
        case ICMP_SR_FAILED:
            printf("Source Route Failed\n");
            break;
        case ICMP_NET_UNKNOWN:
            printf("Destination Net Unknown\n");
            break;
        case ICMP_HOST_UNKNOWN:
            printf("Destination Host Unknown\n");
            break;
        case ICMP_HOST_ISOLATED:
            printf("Source Host Isolated\n");
            break;
        case ICMP_NET_ANO:
            printf("Destination Net Prohibited\n");
            break;
        case ICMP_HOST_ANO:
            printf("Destination Host Prohibited\n");
            break;
        case ICMP_NET_UNR_TOS:
            printf("Destination Net Unreachable for Type of Service\n");
            break;
        case ICMP_HOST_UNR_TOS:
            printf("Destination Host Unreachable for Type of Service\n");
            break;
        case ICMP_PKT_FILTERED:
            printf("Packet filtered\n");
            break;
        case ICMP_PREC_VIOLATION:
            printf("Precedence Violation\n");
            break;
        case ICMP_PREC_CUTOFF:
            printf("Precedence Cutoff\n");
            break;
        default:
            printf("Dest Unreachable, Bad Code: %d\n", code);
            break;
    }
}

void print_redirect(int code)
{
    switch(code)
    {
		case ICMP_REDIR_NET:
			printf("Redirect Network");
			break;
		case ICMP_REDIR_HOST:
			printf("Redirect Host");
			break;
		case ICMP_REDIR_NETTOS:
			printf("Redirect Type of Service and Network");
			break;
		case ICMP_REDIR_HOSTTOS:
			printf("Redirect Type of Service and Host");
			break;
		default:
			printf("Redirect, Bad Code: %d", code);
			break;
    }
}       

void print_time_exceeded(int code)
{
    switch(code)
    {
		case ICMP_EXC_TTL:
			printf("Time to live exceeded\n");
			break;
		case ICMP_EXC_FRAGTIME:
			printf("Frag reassembly time exceeded\n");
			break;
		default:
			printf("Time exceeded, Bad Code: %d\n", code);
			break;
    }
}

void print_error_verbose()
{
    char *tmp = (char *)send_data.error_ip;
    int size = send_data.error_ip->ihl * 4;
    int i = 0;
    struct in_addr addr;

    printf("IP Hdr Dump:\n");
    printf(" ");
    while (i < size)
    {
        if (i != 0)
            printf(" ");
        printf("%.02hhx%.02hhx", tmp[i], tmp[i+1]);
        i +=2;
    }
    printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src	Dst	Data\n");
    printf(" %hhx  %hhx  %.02hhx", send_data.error_ip->version, send_data.error_ip->ihl, send_data.error_ip->tos);
    printf(" %.04hx %.04hx", send_data.error_ip->tot_len, htons(send_data.error_ip->id));
    printf("   0  %.04hx %.02hhx  %.02hhx",htons(send_data.error_ip->frag_off), send_data.error_ip->ttl, send_data.error_ip->protocol);
    printf(" %.04hx", htons(send_data.error_ip->check));
    addr.s_addr = send_data.error_ip->saddr;
    printf(" %s", inet_ntoa(addr));
    addr.s_addr = send_data.error_ip->daddr;
    printf("  %s\n", inet_ntoa(addr));
    printf("ICMP: type %d, code %d, size 21484", send_data.error_icmp->type, send_data.error_icmp->code);
    printf(", id 0x%.04hx, seq 0x%.04hx\n", htons(send_data.error_icmp->un.echo.id), htons(send_data.error_icmp->un.echo.sequence));
}

void print_error_code(int type, int code)
{
    print_seq();
    if (type == ICMP_DEST_UNREACH)
        print_unreachable(code);
    else if (type == ICMP_SOURCE_QUENCH)
        printf("Source Quench\n");
    else if (type == ICMP_REDIRECT)
        print_redirect(code);
    else if (type == ICMP_TIME_EXCEEDED)
        print_time_exceeded(code);
    else if (type == ICMP_PARAMETERPROB)
        printf("Parameter Problem\n");
    else if (type == ICMP_TIMESTAMP)
        printf("Timestamp\n");
    else if (type == ICMP_TIMESTAMPREPLY)
        printf("Timestamp Reply\n");
    else if (type == ICMP_INFO_REQUEST)
        printf("Information Request\n");
    else if (type == ICMP_INFO_REPLY)
        printf("Information Reply\n");
    else if (type == ICMP_ADDRESS)
        printf("Address Mask Request\n");
    else if (type == ICMP_ADDRESSREPLY)
        printf("Address Mask Reply\n");
    else
        printf("BAD ICMP type: %d\n", type);
    if (send_data.info->verbose)
        print_error_verbose();
}

void print_error_usage(char c)
{
    printf("ping: invalid option -- '%c'\n\n", c);
    printf("Try 'iping --help' or 'iping --usage' for more information.\n");
    free_list(send_data.info->start);
    exit(3);
}