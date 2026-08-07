#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main()
{
char cmd[100];
printf("Enter Linux command: ");
scanf("%s", cmd);
pid_t pid = fork();
if(pid == 0)
{
printf("Child PID : %d\n", getpid());
execlp(cmd, cmd, NULL);
perror("Execution Failed");
}
else if(pid > 0)
{
printf("Parent PID : %d\n", getpid());
wait(NULL);
printf("Child process completed.\n");
}
else
{
printf("Fork failed\n");
}
return 0;
}

