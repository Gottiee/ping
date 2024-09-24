#include "../inc/ping.h"

void fatal_perror(char *error)
{
    perror(error);
    exit(1);
}

void fatal_error(char *error)
{
    fprintf(stderr, "%s", error);
    exit(2);
}