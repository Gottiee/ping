#include "../inc/ping.h"

void fatal_perror(char *error, t_to_ping *start)
{
    perror(error);
    free_list(start);
    exit(1);
}

void fatal_error(char *error, t_to_ping *start)
{
    fprintf(stderr, "%s", error);
    free_list(start);
    exit(2);
}