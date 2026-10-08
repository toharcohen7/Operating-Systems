#include "copytree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <dirent.h>

// --- Helper function to create directories recursively ---
void recursive_mkdir(const char *path, mode_t mode)
{
    // Create a temporary copy of the path for dirname (extracting parent directory)
    char temp_path[PATH_MAX];

    strncpy(temp_path, path, sizeof(temp_path));
    temp_path[sizeof(temp_path) - 1] = '\0';
    char *parent_dir = dirname(temp_path);

    if (strcmp(parent_dir, ".") != 0 && strcmp(parent_dir, "/") != 0)
    {
        // Recursively create parent directories
        recursive_mkdir(parent_dir, mode);
    }

    // Create the target directory
    if (mkdir(path, mode) == -1)
    {
        if (errno == EEXIST)
        {
            struct stat st;
            // If it already exists, ensure it's a directory
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
            {
                return;
            }
        }

        perror("mkdir failed");
        return;
    }
}

// --- Function to copy a file ---
void copy_file(const char *src, const char *dest, int copy_symlinks, int copy_permissions)
{

    struct stat check;

    // Check if source exists
    if (lstat(src, &check) == -1)
    {
        perror("lstat failed");
        return;
    }

    // Handle symlinks
    if (S_ISLNK(check.st_mode) && copy_symlinks)
    {

        char link_target[PATH_MAX];

        // Read the target of the symlink
        ssize_t len = readlink(src, link_target, sizeof(link_target) - 1);
        if (len == -1)
        {
            perror("readlink failed");
            return;
        }

        // Null terminate the link target string
        link_target[len] = '\0';

        // Create the symlink at the destination
        if (symlink(link_target, dest) == -1)
        {
            perror("symlink failed");
            return;
        }
        return;
    }

    // Open source file for reading
    int source_fd = open(src, O_RDONLY);
    if (source_fd == -1)
    {
        perror("open failed");
        return;
    }

    // Create destination file for writing
    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (dest_fd == -1)
    {
        perror("open failed");
        close(source_fd);
        return;
    }

    // Create a buffer for copying
    char buffer[4096];
    ssize_t bytes_read, bytes_written;

    // Copy data from source to destination
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0)
    {
        char *ptr = buffer;
        ssize_t bytes_left = bytes_read;

        // Write until all bytes are written
        while (bytes_left > 0)
        {
            bytes_written = write(dest_fd, ptr, bytes_left);
            if (bytes_written == -1)
            {
                perror("write failed");
                close(source_fd);
                close(dest_fd);
                return;
            }
            bytes_left -= bytes_written;
            ptr += bytes_written;
        }
    }
    if (bytes_read == -1)
    {
        perror("read failed");
        close(source_fd);
        close(dest_fd);
        return;
    }

    // Copy permissions if required
    if (copy_permissions)
    {
        if (S_ISLNK(check.st_mode))
        {
            if (stat(src, &check) == -1)
            {
                perror("stat failed");
                close(source_fd);
                close(dest_fd);
                return;
            }
        }

        if (fchmod(dest_fd, check.st_mode & 0777) == -1)
        {
            perror("fchmod failed");
        }
    }

    // Close file descriptors
    close(source_fd);
    close(dest_fd);
}

// --- Function to copy a directory ---
void copy_directory(const char *src, const char *dest, int copy_symlinks, int copy_permissions)
{

    // Create the destination directory
    recursive_mkdir(dest, 0755);

    // Open the source directory
    DIR *dir = opendir(src);
    if (dir == NULL)
    {
        perror("opendir failed");
        return;
    }

    struct dirent *entry;
    char src_path[PATH_MAX];
    char dest_path[PATH_MAX];

    // Iterate over directory entries
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // We need the full paths for source and destination
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat entry_stat;
        if (lstat(src_path, &entry_stat) == -1)
        {
            perror("lstat failed");
            closedir(dir);
            return;
        }
        // Recursively copy subdirectory
        if (S_ISDIR(entry_stat.st_mode))
        {
            copy_directory(src_path, dest_path, copy_symlinks, copy_permissions);
        }
        // Copy file or symlink
        else
        {
            copy_file(src_path, dest_path, copy_symlinks, copy_permissions);
        }
    }

    // Close the source directory
    closedir(dir);

    // Copy directory permissions if required
    if (copy_permissions)
    {
        struct stat dir_stat;
        if (stat(src, &dir_stat) == -1)
        {
            perror("stat failed");
            return;
        }
        if (chmod(dest, dir_stat.st_mode & 0777) == -1)
        {
            perror("chmod failed");
            return;
        }
    }
}