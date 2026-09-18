#include <stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int main()
{
pid_t pid = fork();
if(pid == 0)
{
 printf("Child Process Executing...\n");
sleep(3);
printf("Child Finished\n");
}
else
{
wait(NULL);
printf("Parent Process Continues\n");
}
return 0;
}
