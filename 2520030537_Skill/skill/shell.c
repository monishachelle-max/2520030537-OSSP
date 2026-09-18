#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define BUFFER_SIZE 100

int main()
{
    struct termios oldt, newt;

    char buffer[BUFFER_SIZE];

    int length = 0;

    /* Get current terminal settings */
    tcgetattr(STDIN_FILENO, &oldt);

    /* Copy the settings */
    newt = oldt;

    /* Disable canonical mode and echo */
    newt.c_lflag &= ~(ICANON | ECHO);

    /* Apply new terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("ossp> ");
    fflush(stdout);

    while (1)
    {
        char ch;

        /* Read one character at a time */
        read(STDIN_FILENO, &ch, 1);

        /* Enter key */
        if (ch == '\n' || ch == '\r')
        {
            buffer[length] = '\0';

            printf("\n");

            /* Exit condition */
            if (strcmp(buffer, "exit") == 0)
            {
                break;
            }

            printf("The command entered: %s\n", buffer);

            /* Clear input buffer */
            length = 0;

            printf("ossp> ");
            fflush(stdout);
        }

        /* Backspace key */
        else if (ch == 127 || ch == '\b')
        {
            if (length > 0)
            {
                length--;

                printf("\b \b");
                fflush(stdout);
            }
        }

        /* Normal keyboard character */
        else if (ch >= 32 && ch <= 126)
        {
            if (length < BUFFER_SIZE - 1)
            {
                buffer[length] = ch;

                length++;

                printf("%c", ch);
                fflush(stdout);
            }
        }
    }

    /* Restore original terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    printf("\nGoodbye!\n");

    return 0;
}
