#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define MAX_HISTORY 20
#define MAX_LENGTH 100

char history[MAX_HISTORY][MAX_LENGTH];
int history_count = 0;

void disable_raw_mode(struct termios *old)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, old);
}

void enable_raw_mode(struct termios *old)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, old);

    raw = *old;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void show_command(const char *command)
{
    printf("\r\033[Kmyshell> %s", command);
    fflush(stdout);
}

int main()
{
    char buffer[MAX_LENGTH];
    int length;
    int history_index;
    char ch;

    struct termios old;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        length = 0;

        history_index = history_count;

        printf("myshell> ");
        fflush(stdout);

        enable_raw_mode(&old);

        while (1)
        {
            read(STDIN_FILENO, &ch, 1);

            /* Enter key */
            if (ch == '\n' || ch == '\r')
            {
                buffer[length] = '\0';

                printf("\n");

                break;
            }

            /* Backspace */
            if (ch == 127 || ch == '\b')
            {
                if (length > 0)
                {
                    length--;

                    buffer[length] = '\0';

                    printf("\b \b");

                    fflush(stdout);
                }

                continue;
            }

            /* Escape sequence */
              if(ch == 27)
               {
                  char seq[2];
                  if(read(STDIN_FILENO,& seq[0],1)== 1 &&
                     read(STDIN_FILENO, &seq[1],1)== 1)
                 {
                /* Up Arrow */
                if (seq[0] == '[' && seq[1] == 'A')
                {
                    if (history_index > 0)
                    {
                        history_index--;

                        strcpy(buffer, history[history_index]);

                        length = strlen(buffer);

                        show_command(buffer);
                    }
                }

                /* Down Arrow */
                else if (seq[0] == '[' && seq[1] == 'B')
                {
                    if (history_index < history_count - 1)
                    {
                        history_index++;

                        strcpy(buffer, history[history_index]);

                        length = strlen(buffer);

                        show_command(buffer);
                    }
                    else
                    {
                        history_index = history_count;

                        buffer[0] = '\0';

                        length = 0;

                        show_command(buffer);
                    }
                }
              }

                continue;
            }

            /* Store normal characters */
            if (length < MAX_LENGTH - 1)
            {
                buffer[length++] = ch;

                buffer[length] = '\0';

                putchar(ch);

                fflush(stdout);
            }
        }

        disable_raw_mode(&old);

        /* Exit condition */
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Exiting shell...\n");

            break;
        }

        /* Ignore empty command */
        if (length == 0)
        {
            continue;
        }

        /* Store command in history */
        if (history_count < MAX_HISTORY)
        {
            strcpy(history[history_count], buffer);

            history_count++;
        }

        printf("Command entered: %s\n", buffer);
    }

    return 0;
}

