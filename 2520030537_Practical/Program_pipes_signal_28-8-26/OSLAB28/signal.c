#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void myhandler(int signo)
{
printf("\nSignal Received\n");
printf("Signal Number = %d\n", signo);
exit(1);
}

int main()
{
signal(SIGSEGV, myhandler);

int x = 0;  

printf("Enter the number : ");  
scanf("%d", &x);  

printf("The entered number is %d\n", x);  

 
}
