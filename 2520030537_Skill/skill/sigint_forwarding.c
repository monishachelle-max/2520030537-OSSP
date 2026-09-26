#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

static volatile sig_atomic_t interrupt_received = 0;
static volatile sig_atomic_t foreground_pgid = 0;

typedef enum {
    JOB_EMPTY,
    JOB_RUNNING,
    JOB_COMPLETED,
    JOB_INTERRUPTED
} JobState;

typedef struct {
    int id;
    pid_t pid;
    pid_t pgid;
    JobState state;
} Job;

static Job job = {0};
static int next_job_id = 1;

/* Capture Ctrl+C without terminating the parent shell. */
static void handle_sigint(int signo)
{
    (void)signo;

    interrupt_received = 1;

    /*
     * Forward SIGINT to the foreground job's process group.
     * kill() is async-signal-safe.
     */
    if (foreground_pgid > 0)
        kill(-(pid_t)foreground_pgid, SIGINT);
}

static const char *state_name(JobState state)
{
    switch (state) {
        case JOB_RUNNING:
            return "RUNNING";

        case JOB_COMPLETED:
            return "COMPLETED";

        case JOB_INTERRUPTED:
            return "INTERRUPTED";

        default:
            return "EMPTY";
    }
}

static void show_job(void)
{
    if (job.state == JOB_EMPTY) {
        printf("No job record found.\n");
        return;
    }

    printf("\nJOB TABLE\n");
    printf("ID\tPID\tPGID\tSTATE\n");

    printf("%d\t%ld\t%ld\t%s\n",
           job.id,
           (long)job.pid,
           (long)job.pgid,
           state_name(job.state));
}

static void start_foreground_job(int seconds)
{
    if (seconds < 0) {
        printf("Invalid duration.\n");
        return;
    }

    if (job.state == JOB_RUNNING) {
        printf("A foreground job is already running.\n");
        return;
    }

    /*
     * Block SIGINT while creating and registering the child.
     * This prevents a race before foreground_pgid is assigned.
     */
    sigset_t block_set;
    sigset_t old_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1) {
        perror("sigprocmask");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        sigprocmask(SIG_SETMASK, &old_set, NULL);
        return;
    }

    if (pid == 0) {
        /* Child belongs to its own process group. */
        if (setpgid(0, 0) == -1) {
            perror("setpgid");
            _exit(1);
        }

        /* Restore normal Ctrl+C behavior in the child. */
        struct sigaction default_action;

        memset(&default_action, 0, sizeof(default_action));
        default_action.sa_handler = SIG_DFL;
        sigemptyset(&default_action.sa_mask);

        sigaction(SIGINT, &default_action, NULL);

        /* Restore the original signal mask. */
        sigprocmask(SIG_SETMASK, &old_set, NULL);

        char duration[20];

        snprintf(duration, sizeof(duration),
                 "%d", seconds);

        execlp("sleep", "sleep",
               duration, (char *)NULL);

        perror("execlp");
        _exit(127);
    }

    /*
     * The parent also attempts setpgid to avoid
     * a process-group creation race.
     */
    if (setpgid(pid, pid) == -1 && errno != EACCES)
        perror("setpgid");

    job.id = next_job_id++;
    job.pid = pid;
    job.pgid = pid;
    job.state = JOB_RUNNING;

    foreground_pgid = (sig_atomic_t)pid;
    interrupt_received = 0;

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1)
        perror("sigprocmask");

    printf("\nForeground job [%d] started.\n", job.id);
    printf("PID: %ld\n", (long)job.pid);
    printf("PGID: %ld\n", (long)job.pgid);
    printf("Running sleep %d...\n", seconds);
    printf("Press Ctrl+C to interrupt the foreground job.\n");
    fflush(stdout);

    int status;
    pid_t result;

    do {
        result = waitpid(pid, &status, 0);
    } while (result == -1 && errno == EINTR);

    /*
     * Block SIGINT again while clearing the active job
     * to prevent forwarding a signal to an old PGID.
     */
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) == -1)
        perror("sigprocmask");

    foreground_pgid = 0;

    if (result == -1) {
        perror("waitpid");
        job.state = JOB_INTERRUPTED;
    }
    else if (WIFSIGNALED(status)) {
        job.state = JOB_INTERRUPTED;

        printf("\nForeground job terminated by signal %d.\n",
               WTERMSIG(status));
    }
    else if (WIFEXITED(status)) {
        job.state = JOB_COMPLETED;

        printf("\nForeground job completed.\n");
        printf("Exit status: %d\n",
               WEXITSTATUS(status));
    }

    if (sigprocmask(SIG_SETMASK, &old_set, NULL) == -1)
        perror("sigprocmask");

    if (interrupt_received)
        printf("SIGINT was captured by the shell.\n");

    printf("Shell process is still running.\n");

    show_job();
}

int main(void)
{
    struct sigaction sa;
    char input[100];
    int seconds;
    char extra;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Q19 - SIGINT Forwarding Program\n");
    printf("Shell PID: %ld\n", (long)getpid());
    printf("\nCommands:\n");
    printf("run NUMBER\n");
    printf("jobs\n");
    printf("exit\n");

    while (1) {
        printf("\nshell> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (ferror(stdin) && errno == EINTR) {
                clearerr(stdin);
                continue;
            }

            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        if (strcmp(input, "jobs") == 0) {
            show_job();
        }
        else if (sscanf(input, "run %d %c",
                        &seconds, &extra) == 1 &&
                 seconds >= 0) {
            start_foreground_job(seconds);
        }
        else {
            printf("Invalid command.\n");
        }
    }

    printf("Shell exited normally.\n");

    return 0;
}
