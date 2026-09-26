#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <termios.h>

typedef enum { EMPTY, RUNNING, STOPPED, COMPLETED } State;

typedef struct {
    int id;
    pid_t pid;
    pid_t pgid;
    State state;
} Job;

static Job job = {0};
static pid_t shell_pgid;
static struct termios shell_termios;

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
    if (job.state != RUNNING && job.state != STOPPED)
        return;

    int status;
    pid_t result;

    do {
        result = waitpid(job.pid, &status,
                         WNOHANG | WUNTRACED | WCONTINUED);

        if (result == job.pid) {
            if (WIFSTOPPED(status))
                job.state = STOPPED;
            else if (WIFCONTINUED(status))
                job.state = RUNNING;
            else if (WIFEXITED(status) || WIFSIGNALED(status))
                job.state = COMPLETED;
        }
    } while (result == job.pid &&
             job.state != COMPLETED);
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

static void restore_terminal(void)
{
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) == -1)
        perror("restore tcsetpgrp");

    if (tcsetattr(STDIN_FILENO, TCSADRAIN,
                  &shell_termios) == -1)
        perror("restore terminal settings");
}

static void foreground_job(int resume)
{
    int status;
    pid_t result;

    if (tcsetpgrp(STDIN_FILENO, job.pgid) == -1) {
        perror("tcsetpgrp");
        return;
    }

    printf("Terminal transferred to PGID %ld.\n",
           (long)job.pgid);
    fflush(stdout);

    if (resume) {
        if (kill(-job.pgid, SIGCONT) == -1) {
            perror("SIGCONT");
            restore_terminal();
            return;
        }

        job.state = RUNNING;
        printf("Foreground job resumed.\n");
        fflush(stdout);
    }

    do {
        result = waitpid(job.pid, &status, WUNTRACED);
    } while (result == -1 && errno == EINTR);

    restore_terminal();

    printf("\nTerminal restored to shell PGID %ld.\n",
           (long)shell_pgid);

    if (result == -1) {
        perror("waitpid");
        return;
    }

    if (WIFSTOPPED(status)) {
        job.state = STOPPED;
        printf("Job [%d] stopped by signal %d.\n",
               job.id, WSTOPSIG(status));
    }
    else if (WIFSIGNALED(status)) {
        job.state = COMPLETED;
        printf("Job [%d] terminated by signal %d.\n",
               job.id, WTERMSIG(status));
    }
    else if (WIFEXITED(status)) {
        job.state = COMPLETED;
        printf("Job [%d] completed with exit status %d.\n",
               job.id, WEXITSTATUS(status));
    }
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
        if (setpgid(0, 0) == -1) {
            perror("setpgid");
            _exit(1);
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

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

    printf("\nCreated job [%d].\n", job.id);
    printf("Child PID: %ld\n", (long)job.pid);
    printf("Child PGID: %ld\n", (long)job.pgid);
    printf("Shell PGID: %ld\n", (long)shell_pgid);
    printf("Press Ctrl+Z to suspend or Ctrl+C to interrupt.\n");
    fflush(stdout);

    foreground_job(0);
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
    printf("Terminal remains under shell control.\n");
}

int main(void)
{
    char input[100];
    int number;
    char extra;

    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Use an interactive terminal.\n");
        return 1;
    }

    shell_pgid = getpgrp();

    if (tcgetpgrp(STDIN_FILENO) != shell_pgid) {
        fprintf(stderr,
                "Start this program as a foreground terminal job.\n");
        return 1;
    }

    if (tcgetattr(STDIN_FILENO, &shell_termios) == -1) {
        perror("tcgetattr");
        return 1;
    }

    /*
     * The shell must be able to reclaim the terminal
     * while it is temporarily in the background.
     */
    signal(SIGTTOU, SIG_IGN);

    printf("Q20 - Process Groups and Terminal Control\n");
    printf("Shell PID: %ld\n", (long)getpid());
    printf("Shell PGID: %ld\n", (long)shell_pgid);
    printf("Commands: run NUMBER, jobs, fg ID, bg ID, exit\n");

    while (1) {
        update_job();

        printf("\npgshell> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;
        else if (strcmp(input, "jobs") == 0)
            show_job();
        else if (sscanf(input, "run %d %c",
                        &number, &extra) == 1 && number >= 0)
            start_job(number);
        else if (sscanf(input, "fg %d %c",
                        &number, &extra) == 1) {
            update_job();

            if (job.id == number &&
                (job.state == RUNNING || job.state == STOPPED))
                foreground_job(job.state == STOPPED);
            else
                printf("Active job not found.\n");
        }
        else if (sscanf(input, "bg %d %c",
                        &number, &extra) == 1)
            resume_background(number);
        else
            printf("Invalid command.\n");
    }

    restore_terminal();

    if (job.state == STOPPED)
        kill(-job.pgid, SIGCONT);

    if (job.state == RUNNING || job.state == STOPPED)
        waitpid(job.pid, NULL, 0);

    printf("Process group program exited.\n");
    return 0;
}
