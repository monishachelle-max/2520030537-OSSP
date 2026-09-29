#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_PATH 4096

int main()
{
    char *path;
    char *path_copy;
    char *directory;
    char full_path[MAX_PATH];
    struct stat file_info;
    int found = 0;

    path = getenv("PATH");

    if (path == NULL)
    {
        printf("PATH variable not found.\n");
        return 1;
    }

    path_copy = strdup(path);

    if (path_copy == NULL)
    {
        perror("strdup");
        return 1;
    }

    directory = strtok(path_copy, ":");

    while (directory != NULL)
    {
        snprintf(full_path, sizeof(full_path),
                 "%s/%s", directory, "ossp_missing_command");

        if (stat(full_path, &file_info) == 0 &&
            access(full_path, X_OK) == 0)
        {
            printf("Found executable: %s\n", full_path);
            found = 1;
            break;
        }

        directory = strtok(NULL, ":");
    }

    free(path_copy);

    if (!found)
    {
        printf("Command not found in PATH.\n");
    }

    return 0;
}
