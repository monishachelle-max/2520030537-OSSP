#include <stdio.h>
#include <unistd.h>

int main()
{
    int x = 100;

    if (fork() == 0)
    {
        printf("Child Before=%d\n", x);
        x = 500;
        printf("Child After=%d\n", x);
    }
    else
    {
        sleep(2);
        printf("Parent=%d\n", x);
    }

    return 0;
}
