#include<stdio.h>
#include<unistd.h>
int main()
{
printf("Before exec()\n");
execlp("ls","ls",NULL);
printf("After exec()\n");
return 0;
}
