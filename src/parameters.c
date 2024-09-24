#include "../inc/ping.h"

t_info manage_args(char **args)
{
    (void)args;
    t_info info;

    info.ttl = 64;
    info.sleep_rate = 1000000; // microsec
    return info;
}