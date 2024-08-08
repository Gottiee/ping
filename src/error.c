#include "../inc/ping.h"

void fatal_perror(char *error)
{
    perror(error);
    exit(1);
}