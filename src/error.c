#include "../inc/ping.h"

void fatal_perror(char *error)
{
    perror(error);
    free_list(send_data.info->start);
    exit(1);
}

void fatal_error(char *error)
{
    fprintf(stderr, "%s", error);
    free_list(send_data.info->start);
    exit(2);
}