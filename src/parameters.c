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

char handle_options(char *arg)
{
    while(*arg)
    {
        if (*arg == 'v')
            send_data.info->verbose = true;
        else if (*arg == '?')
            print_option();
        else if (*arg == 't')
            send_data.info->ttl = 3;
        else
            return *arg;
        arg ++;
    }
    return '\0';
}

void sort_arg(char *arg)
{
    if (arg && arg[0] != '-')
        add_to_ping_list(arg, send_data.info->start);
    else if (arg)
    {
        char c = handle_options(++arg);
        if (c)
            print_error_usage(c);

    }
    else
        fatal_error("Error arg is empty while parsing arguments\n");
}

void init_info()
{
    send_data.info->ttl = 64;
    send_data.info->sleep_rate = 1000000; // microsec
    send_data.info->msg_receive = 0;
    send_data.info->min = 0;
    send_data.info->max = 0;
    send_data.info->start = NULL;
    send_data.info->verbose = false;
}

void handle_args(char **argv)
{
    int i = -1;

    send_data.info->start->next = NULL;
    send_data.info->start->target = NULL;
    while (argv[++i])
    {
        sort_arg(argv[i]);
    }
}