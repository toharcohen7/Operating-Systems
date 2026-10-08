//  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{

    // Check the args number
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>", argv[0]);
        return 1;
    }

    const int times = atoi(argv[4]);
    char *child1Msg = argv[2], *child2Msg = argv[3], *parentMsg = argv[1];

    // Open an output file
    FILE *file = fopen("output.txt", "w+");
    if(file == NULL)
    {
        perror("Can't open a file");
        return 1;
    }

    // Fork first child and check if succeded
    pid_t child1 = fork();
    if (child1 < 0) {
        perror("Fork failed");
        fclose(file);
        return 1;
    }

    // Print Child1 massage for the requsted times
    if ( child1 == 0){
        for (int i = 0; i < times; i++)
        {
            fprintf(file, "%s\n", child1Msg);
        }
        exit(0);
    }

    // Fork sec child and check if succeded
    pid_t child2 = fork();
    if (child2 < 0) {
        perror("Fork failed");
        fclose(file);
        return 1;
    }

    // Print Child2 massage for the requsted times   
    if (child2 == 0)
    {
        if (sleep(5) != 0) {
        perror("Sleep failed");
        fclose(file);
        return 1;
        }

        for (int i = 0; i < times; i++)
        {
            fprintf(file, "%s\n", child2Msg);
        }
        exit(0);
    }

    //Wait for the childrens
    if (wait(NULL) < 0) {
        perror("Wait failed");
        fclose(file);
        return 1;
    } 
    if (wait(NULL) < 0) {
        perror("Wait failed");
        fclose(file);
        return 1;
    } 

    // Print for parent proccess
    for(int i = 0; i < times; i++)
    {
        fprintf(file, "%s\n", parentMsg);
    }

    // Close the output file
    fclose(file);

    return 0;

}

