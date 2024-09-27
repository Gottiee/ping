#include "../inc/ping.h"

void free_list(t_to_ping *start)
{
    t_to_ping *tmp;

    while(start)
    {
        tmp = start;
        start = start->next;
        free(tmp);
    }
}

void add_to_ping_list(char *arg, t_to_ping *start)
{
    if (!start->target)
    {
        start->target = arg;
        return;
    }
    t_to_ping *next = malloc(sizeof(t_to_ping));
    while(start->next)
        start = start->next;
    start->next = next; 
    next->next = NULL;
    next->target = arg;
}

void handle_options(char *arg, t_info *info)
{
    (void)arg;
    (void)info;
}

void sort_arg(char *arg, t_to_ping *start, t_info *info)
{
    if (arg && arg[0] != '-')
        add_to_ping_list(arg, start);
    else if (arg)
        handle_options(arg, info);
    else
        fatal_error("Error arg is empty while parsing arguments\n", info->start);
}

void handle_args(t_to_ping *start, char **argv, t_info *info)
{
    int i = -1;

    set_info(info);
    info->start = start;
    start->next = NULL;
    start->target = NULL;
    while (argv[++i])
    {
        sort_arg(argv[i], start, info);
    }
}

void set_info(t_info *info)
{
    info->ttl = 64;
    info->sleep_rate = 1000000; // microsec
    info->msg_receive = 0;
    info->min = 0;
    info->max = 0;
}