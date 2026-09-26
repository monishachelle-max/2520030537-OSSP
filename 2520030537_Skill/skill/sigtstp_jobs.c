#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

typedef enum { EMPTY, RUNNING, STOPPED, COMPLETED } State;

typedef struct {
    int id;
    pid_t pid;
    pid_t pgid;
    State state;
} Job;

static Job job = {0};
static volatile sig_atomic_t foreground_pgid = 0;

static void handle_sigtstp(int signo)
{
    (void)signo;

    if (foreground_pgid > 0)
        kill(-(pid_t)foreground_pgid, SIGTSTP);
}

static const char *state_name(State state)
{
    switch (state) {
        case RUNNING: return "RUNNING";
        case STOPPED: return "STOPPED";
        case COMPLETED: return "COMPLETED";
        default: return "EMPTY";
    }
}

static void update_job(void)
{
    if (job.state != RUNNING)
        return;

    int status;
    pid_t result = waitpid(job.pid, &status,
                           WNOHANG | WUNTRACED);

    if (result == job.pid) {
        if (WIFSTOPPED(status))
            job.state = STOPPED;
        else if (WIFEXITED(status) || WIFSIGNALED(status))
            job.state = COMPLETED;
    }
}

static void show_job(void)
{
    update_job();

    printf("\nJOB TABLE\n");
    printf("ID\tPID\tPGID\tSTATE\n");

    if (job.state == EMPTY) {
        printf("No job records.\n");
        return;
    }

    printf("%d\t%ld\t%ld\t%s\n",
           job.id, (long)job.pid, (long)job.pgid,
           state_name(job.state));
}

static void wait_for_foreground(void)
{
    int status;
    pid_t result;

    foreground_pgid = (sig_atomic_t)job.pgid;

    do {
        result = waitpid(job.pid, &status, WUNTRACED);
    } while (result == -1 && errno == EINTR);

    foreground_pgid = 0;

    if (result == -1) {
        perror("waitpid");
        return;
    }

    if (WIFSTOPPED(status)) {
        job.state = STOPPED;
        printf("\nJob [%d] suspended.\n", job.id);
    } else {
        job.state = COMPLETED;
        printf("\nJob [%d] completed.\n", job.id);
    }

    printf("Parent shell is still running.\n");
}

static void start_job(int seconds)
{
    update_job();

    if (job.state == RUNNING || job.state == STOPPED) {
        printf("Finish the existing job first.\n");
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        setpgid(0, 0);

        struct sigaction sa = {0};
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGTSTP, &sa, NULL);

        char duration[20];
        snprintf(duration, sizeof(duration), "%d", seconds);

        execlp("sleep", "sleep", duration, (char *)NULL);
        perror("execlp");
        _exit(127);
    }

    if (setpgid(pid, pid) == -1 && errno != EACCES)
        perror("setpgid");

    job.id++;
    job.pid = pid;
    job.pgid = pid;
    job.state = RUNNING;

    printf("Started foreground job [%d], PID=%ld\n",
           job.id, (long)pid);
    printf("Press Ctrl+Z to suspend it.\n");
    fflush(stdout);

    wait_for_foreground();
}

static void resume_background(int id)
{
    update_job();

    if (job.id != id || job.state != STOPPED) {
        printf("Stopped job not found.\n");
        return;
    }

    if (kill(-job.pgid, SIGCONT) == -1) {
        perror("SIGCONT");
        return;
    }

    job.state = RUNNING;
    printf("Job [%d] resumed in background.\n", id);
}

static void resume_foreground(int id)
{
    update_job();

    if (job.id != id || job.state != STOPPED) {
        printf("Stopped job not found.\n");
        return;
    }

    if (kill(-job.pgid, SIGCONT) == -1) {
        perror("SIGCONT");
        return;
    }

    job.state = RUNNING;
    printf("Job [%d] resumed in foreground.\n", id);
    fflush(stdout);

    wait_for_foreground();
}

int main(void)
{
    struct sigaction sa = {0};
    char input[100];
    int number;
    char extra;

    sa.sa_handler = handle_sigtstp;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Q20 - SIGTSTP and Job Suspension\n");
    printf("Commands: run NUMBER, jobs, bg ID, fg ID, exit\n");

    while (1) {
        update_job();

        printf("\nsuspend> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            if (ferror(stdin) && errno == EINTR) {
                clearerr(stdin);
                continue;
            }
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;
        else if (strcmp(input, "jobs") == 0)
            show_job();
        else if (sscanf(input, "run %d %c",
                        &number, &extra) == 1 && number >= 0)
            start_job(number);
        else if (sscanf(input, "bg %d %c",
                        &number, &extra) == 1)
            resume_background(number);
        else if (sscanf(input, "fg %d %c",
                        &number, &extra) == 1)
            resume_foreground(number);
        else
            printf("Invalid command.\n");
    }

    if (job.state == STOPPED)
        kill(-job.pgid, SIGCONT);

    if (job.state == RUNNING || job.state == STOPPED)
        waitpid(job.pid, NULL, 0);

    printf("Suspension program exited.\n");
    return 0;
}
