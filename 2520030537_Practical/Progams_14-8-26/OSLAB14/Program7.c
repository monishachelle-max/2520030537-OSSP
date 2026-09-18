#include <stdio.h>
#include <unistd.h>

int main()
{
    fork();

    for(int i = 1; i <= 5; i++)
    {
        printf("PID: %d Count: %d\n", getpid(), i);
        sleep(1);
    }

    return 0;
}
