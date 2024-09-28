#include "../inc/ping.h"

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
    printf("%d bytes from ", PING_PKT_S);
    printf("%s", send_data.info->ip);
    printf(": icmp_seq=%d ", send_data.info->sequence);
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
   
    print_seq(info);
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
    printf(": 56 data bytes\n");
}

void print_usage()
{
    printf("ping: missing host operand\n");
    printf("Try 'ping --help' or 'ping --usage' for more information.\n");
    exit(1);
}

void print_unreachable(int code)
{
    if (code == ICMP_NET_UNREACH)
        printf("Network Unreachable");
    else if (code == ICMP_HOST_UNREACH)
        printf("Host Unreachable");
    else if (code == ICMP_PROT_UNREACH)
        printf("Protocol Unreachable");
    else if (code == ICMP_PORT_UNREACH)
        printf("Port Unreachable");
    else if (code == ICMP_FRAG_NEEDED)
        printf("Fragmentation Needed/DF set");
    else if (code == ICMP_SR_FAILED)
        printf("Source Route failed");
    else if (code == ICMP_PKT_FILTERED)
        printf("Packet filtered");
    else if (code == ICMP_PREC_VIOLATION)
        printf("Precedence violation");
    else if (code == ICMP_PREC_CUTOFF)
        printf("Precedence cut off");
    else if (code == NR_ICMP_UNREACH)
        printf("instead of hardcoding immediate value");
    else   
        printf("Unknow Error For ICMP_DEST_UNREACH code %d", code);
    printf("\n");
}

void print_redirect(int code)
{
    if (code == ICMP_REDIR_NET)
        printf("Redirect Net");
    else if (code == ICMP_REDIR_HOST)
        printf("Redirect Host");
    else if (code == ICMP_REDIR_NETTOS)
        printf("Redirect Net for TOS");
    else if (code == ICMP_REDIR_HOSTTOS)
        printf("Redirect Host for TOS");
    else
        printf("Unknow Error For ICMP_REDIRECT code %d", code);
    printf("\n");
}       

void print_time_exceeded(int code)
{
    if (code == ICMP_EXC_TTL)
        printf("TTL count exceeded");
    else if (code == ICMP_EXC_FRAGTIME)
        printf("Fragment Reass time exceeded");
    else
        printf("Unknow Error For ICMP_TIME_EXCEEDED code %d", code);
    printf("\n");
}

void print_error_code(int type, int code)
{
    print_seq();
    if (type == ICMP_DEST_UNREACH)
        print_unreachable(code);
    if (type == ICMP_SOURCE_QUENCH)
        printf("Source Quench\n");
    if (type == ICMP_REDIRECT)
        print_redirect(code);
    if (type == ICMP_TIME_EXCEEDED)
        print_time_exceeded(code);
    if (type == ICMP_PARAMETERPROB)
        printf("Parameter Problem\n");
    if (type == ICMP_TIMESTAMP)
        printf("Timestamp Request\n");
    if (type == ICMP_TIMESTAMPREPLY)
        printf("Timestamp Reply\n");
    if (type == ICMP_INFO_REQUEST)
        printf("Information Request\n");
    if (type == ICMP_INFO_REPLY)
        printf("Information Reply\n");
    if (type == ICMP_ADDRESS)
        printf("Address Mask Request\n");
    if (type == ICMP_ADDRESSREPLY)
        printf("Address Mask Reply\n");
    else
        printf("Unknow error for type %d\n", type);
}
