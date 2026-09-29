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

    path = getenv("PATH");

    if (path == NULL)
    {
        printf("PATH variable not found.\n");
        return 1;
    }

    printf("PATH variable:\n%s\n\n", path);

    path_copy = strdup(path);

    if (path_copy == NULL)
    {
        perror("strdup");
        return 1;
    }

    printf("Search directories:\n");

    directory = strtok(path_copy, ":");

    while (directory != NULL)
    {
        printf("%s\n", directory);

        directory = strtok(NULL, ":");
    }

    free(path_copy);

    printf("\nExecutable resolution:\n");

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
                 "%s/%s", directory, "ls");

        if (stat(full_path, &file_info) == 0)
        {
            if (access(full_path, X_OK) == 0)
            {
                printf("Found executable: %s\n", full_path);
                break;
            }
        }

        directory = strtok(NULL, ":");
    }

    free(path_copy);

    if (directory == NULL)
    {
        printf("Executable not found.\n");
    }

    return 0;
}
