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

void verify_next_number(char *arg, int size_arg)
{
    int i = size_arg - 1;

    if (!arg[size_arg])
        fatal_error_parsing_no_arg("ping: option requires an argument -- '%s'\n", arg);

    while (arg[++i])
    {
        if (arg[i] < '0' || arg[i] > '9')
            fatal_error_parsing("ping: invalid value (`%s' near `%c')\n", arg[i], &arg[size_arg]);
    }
}

char numeric_option(char *arg, int size_arg, int *target, bool error_on_zero)
{
    verify_next_number(arg, size_arg);
    arg += size_arg;
    *target = atoi(arg);
    if (error_on_zero)
        if (!*target)
            fatal_error("iping option value too small: 0\n");
    return '\0';
}

char handle_options(char *arg)
{
    while(*arg)
    {
        if (*arg == 'v')
            send_data.info->verbose = true;
        else if (*arg == '?')
            print_option();
        else if (*arg == 'q')
            send_data.info->quiet = true;
        else if (*arg == 'c')
            return numeric_option(arg, 1, &send_data.info->count, false);
        else if (!strncmp("-ttl", arg, 4))
            return numeric_option(++arg, 3, &send_data.info->ttl, true);
        else if (*arg == 'W')
            return numeric_option(arg, 1, &send_data.info->recv_timeout, true);
        else if (*arg == 'i')
            return numeric_option(arg, 1, &send_data.info->interval, true);
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
    send_data.info->quiet = false;
    send_data.info->count = -1;
    send_data.info->recv_timeout = 1;
    send_data.info->interval = 1;
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