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

void print(struct timespec time_loop_start, t_info *info, int msg_count)
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
   
    printf("%d bytes from ", PING_PKT_S);
    printf("%s", info->ip);
    printf(": icmp_seq=%d ttl=%d time=%.3f ms\n", msg_count - 1, info->return_ttl, rtt_msec);
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
    msg_count -= 1;
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
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms", info->min, average, info->max, dev);
}

void print_header(t_info *info)
{
    if (strlen(info->domain))
        printf("PING %s (%s)", info->domain, info->ip);
    else
        printf("PING %s", info->ip);
    printf(": 56 data bytes\n");
}