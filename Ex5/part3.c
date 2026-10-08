#include "copytree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

// Helper function to check if a directory is empty (returns 1 if empty, 0 otherwise)
int is_dir_empty(const char *path) {
    DIR *d = opendir(path);
    if (!d) return 0; 

    struct dirent *entry;
    int has_files = 0;

    while ((entry = readdir(d)) != NULL) {
        // Skip the self (.) and parent (..) entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // If we found any other entry -> it is NOT empty
        has_files = 1;
        break;
    }
    
    closedir(d);
    return !has_files; 
}

void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s [-l] [-p] <source_directory> <destination_directory>", prog_name);
    fprintf(stderr, "  -l: Copy symbolic links as links");
    fprintf(stderr, "  -p: Copy file permissions");
}

int main(int argc, char *argv[])
{
    int opt;
    int copy_symlinks = 0;
    int copy_permissions = 0;

    while ((opt = getopt(argc, argv, "lp")) != -1)
    {
        switch (opt)
        {
        case 'l':
            copy_symlinks = 1;
            break;
        case 'p':
            copy_permissions = 1;
            break;
        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (optind + 2 != argc)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *src_dir = argv[optind];
    const char *dest_dir = argv[optind + 1];
    struct stat st;
    
    // Validate the source, must exist and be a directory
    if (stat(src_dir, &st) < 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "incorrect usage, expecting a directory\n");
        return EXIT_FAILURE;
    }

    // Validate the destination, must not exist or be an empty directory
    if (stat(dest_dir, &st) == 0) {
        
        // Check if it is not a directory
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: Destination exists and is not a directory.\n");
            return EXIT_FAILURE;
        }

        // Check if it is not empty
        if (!is_dir_empty(dest_dir)) {
            fprintf(stderr, "Make sure the destination directory does not already exist or is empty to avoid conflicts.\n");
            return EXIT_FAILURE;
        }
    }

    copy_directory(src_dir, dest_dir, copy_symlinks, copy_permissions);

    return 0;
}