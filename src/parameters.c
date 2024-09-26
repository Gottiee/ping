#include "../inc/ping.h"

void manage_args(char **args, t_info *info)
{
    (void)args;

    info->ttl = 64;
    info->sleep_rate = 1000000; // microsec
    info->msg_receive = 0;
    info->min = 0;
    info->max = 0;
}