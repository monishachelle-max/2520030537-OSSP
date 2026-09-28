#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t sigint_received = 0;
volatile sig_atomic_t sigterm_received = 0;
volatile sig_atomic_t sigusr1_received = 0;

void signal_handler(int signo)
{
    if (signo == SIGINT)
        sigint_received = 1;
    else if (signo == SIGTERM)
        sigterm_received = 1;
    else if (signo == SIGUSR1)
        sigusr1_received = 1;
}

int main(void)
{
    struct sigaction sa = {0};

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGTERM, &sa, NULL) == -1 ||
        sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Signal handling program started.\n");
    printf("Process PID: %ld\n", (long)getpid());
    printf("Press Ctrl+C or send SIGUSR1/SIGTERM.\n");
    fflush(stdout);

    while (!sigterm_received) {
        pause();

        if (sigint_received) {
            sigint_received = 0;
            printf("SIGINT received: Ctrl+C detected.\n");
        }

        if (sigusr1_received) {
            sigusr1_received = 0;
            printf("SIGUSR1 received: User-defined signal.\n");
        }

        fflush(stdout);
    }

    printf("SIGTERM received: Terminating gracefully.\n");
    printf("Program exited successfully.\n");

    return 0;
}
