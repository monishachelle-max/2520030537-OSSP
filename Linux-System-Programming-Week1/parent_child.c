#include<stdio.h>
#include<unistd.h>
int main()
{
pid_t pid;
pid=fork();
if(pid==0)
{
printf("I am Child Process\n");
}
else
{
printf("I am Parent Process\n");
}
return 0;
}


