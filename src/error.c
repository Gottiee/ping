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

void fatal_error_parsing_no_arg(char *error, char *option)
{
    fprintf(stderr, error, option);
    free_list(send_data.info->start);
    exit(5);
}

void fatal_error_parsing(char *error, char c, char *str)
{
    fprintf(stderr, error, str, c);
    free_list(send_data.info->start);
    exit(5);
}