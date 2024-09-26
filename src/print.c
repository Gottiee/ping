#include "../inc/ping.h"

void print(struct timespec time_loop_start, t_info *info, int msg_count)
{
    struct timespec time_loop_end;
    long double rtt_msec = 0;

    clock_gettime(CLOCK_MONOTONIC, &time_loop_end);

    double timeElapsed = ((double)(time_loop_end.tv_nsec - time_loop_start.tv_nsec)) / 1000000.0;
    rtt_msec = (time_loop_end.tv_sec - time_loop_start.tv_sec) * 1000.0 + timeElapsed;
   
    printf("%d bytes from ", PING_PKT_S);
    if (strlen(info->domain))
        printf("%s (%s)", info->reverse_domain, info->ip);
    else
        printf("%s", info->ip);
    printf(": icmp_seq=%d ttl=%d time=%Lf ms\n", msg_count -1, info->return_ttl, rtt_msec);
}