#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

static volatile sig_atomic_t sigint_count = 0;
static volatile sig_atomic_t sigusr1_received = 0;

/* Signal handlers only update flags */
void handle_sigint(int signo)
{
    (void)signo;
    sigint_count++;
}

void handle_sigusr1(int signo)
{
    (void)signo;
    sigusr1_received = 1;
}

int main(void)
{
    struct sigaction sa_int;
    struct sigaction sa_usr1;

    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_usr1, 0, sizeof(sa_usr1));

    /* Register the SIGINT handler */
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) == -1)
    {
        perror("sigaction SIGINT");
        return 1;
    }

    /* Register the SIGUSR1 handler */
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1)
    {
        perror("sigaction SIGUSR1");
        return 1;
    }

    printf("Q19 - Signal Handlers Program\n");
    printf("PID: %ld\n", (long)getpid());
    printf("SIGINT and SIGUSR1 handlers registered.\n");

    /* Generate a test signal */
    printf("\nSending SIGUSR1 to this process...\n");

    if (kill(getpid(), SIGUSR1) == -1)
    {
        perror("kill");
        return 1;
    }

    if (sigusr1_received)
    {
        printf("SIGUSR1 handled successfully.\n");
        sigusr1_received = 0;
    }

    printf("\nPress Ctrl+C twice to test SIGINT.\n");
    printf("The program will remain active after the first Ctrl+C.\n");
    fflush(stdout);

    while (sigint_count < 2)
    {
        pause();

        if (sigint_count == 1)
        {
            printf("\nSIGINT received. Program is still running.\n");
            printf("Press Ctrl+C again to exit.\n");
            fflush(stdout);
        }
    }

    printf("\nSIGINT received twice.\n");
    printf("Signal handling test completed successfully.\n");

    return 0;
}
