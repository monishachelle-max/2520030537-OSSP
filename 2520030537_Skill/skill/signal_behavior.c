#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

volatile sig_atomic_t received_signal = 0;

void signal_handler(int signo)
{
    received_signal = signo;
}

int main(void)
{
    struct sigaction sa;
    sigset_t block_set;
    sigset_t old_set;
    sigset_t pending_set;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    printf("Q18 - Signal Behavior Program\n");
    printf("PID: %ld\n", (long)getpid());
    printf("Process Group ID: %ld\n", (long)getpgrp());
    printf("Press Ctrl+C to test SIGINT.\n");

    /* Block SIGUSR1 temporarily */
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1)
    {
        perror("sigprocmask");
        return 1;
    }

    printf("\nSIGUSR1 is now blocked.\n");
    printf("Sending SIGUSR1 to this process...\n");

    if (kill(getpid(), SIGUSR1) == -1)
    {
        perror("kill");
        return 1;
    }

    if (sigpending(&pending_set) == -1)
    {
        perror("sigpending");
        return 1;
    }

    if (sigismember(&pending_set, SIGUSR1))
        printf("SIGUSR1 is pending while blocked.\n");

    printf("Unblocking SIGUSR1...\n");

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1)
    {
        perror("sigprocmask");
        return 1;
    }

    if (received_signal == SIGUSR1)
        printf("SIGUSR1 delivered successfully.\n");

    printf("\nWaiting for terminal signals.\n");
    printf("Press Ctrl+C to send SIGINT.\n");
    printf("Type Ctrl+C once, then the program will exit.\n");
    fflush(stdout);

    while (received_signal != SIGINT)
        pause();

    printf("\nSIGINT received from terminal.\n");
    printf("Signal behavior test completed.\n");

    return 0;
}
