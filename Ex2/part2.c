//  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

void write_message(const char *message, int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

int main(int argc, char *argv[])
{
    // Check the args number
    if (argc <= 4)
    {
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>", argv[0]);
        return 1;
    }

    int times = atoi(argv[argc - 1]), childsCount = argc - 2;
    pid_t pidsArr[childsCount];
    char *childsMsg[childsCount];

    // Saving the childs massages in an array
    for (int i = 0; i < childsCount; i++)
    {
        childsMsg[i] = argv[i+1];
    }

    // Loop for fork all the childrens
    for (int i = 0; i < childsCount; i++)
    {
        // Fork i child and check if succeded
        pidsArr[i] = fork();

        if (pidsArr[i] < 0)
        {
            perror("Fork failed");
            return 1;
        }

        // Print Child i massage for the requsted times
        if (pidsArr[i] == 0)
        {
            int lockFile;

            // Lock with the file
            while((lockFile = open("lockfile.lock", O_CREAT | O_EXCL | O_WRONLY, 0644)) < 0)
            {
                usleep(4444);
            }

            close(lockFile);
            write_message(childsMsg[i], times);

            // delete the file
            if (remove("lockfile.lock") < 0) {
                perror("Error removing lock file");
            }

            exit(0);
        }
    }

    // Making the parent wait
    for (int i = 0; i < childsCount; i++)
    {
        wait(NULL);
    }

    return 0;

}