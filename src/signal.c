#include "../inc/ping.h"

void loop_handler(int signal)
{
    if (signal == SIGINT)
        nbr_loop = 0;
    else if (signal == SIGALRM)
        send_ping();
}