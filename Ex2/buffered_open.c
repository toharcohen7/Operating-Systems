
#include <stdio.h>
#include <stdlib.h>
#include "buffered_open.h"
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

// Function to wrap the original open function
buffered_file_t *buffered_open(const char *pathname, int flags, ...)
{

    // Check if there is wrong flags toghether
    if ((flags & O_PREAPPEND) && (flags & O_ACCMODE) == O_WRONLY)
    {
        errno = EINVAL;
        return NULL;
    }

    // Memory allocations
    buffered_file_t *buff = (buffered_file_t *)malloc(sizeof(buffered_file_t));
    if (!buff)
    {
        errno = ENOMEM;
        return NULL;
    }

    buff->read_buffer = (char *)malloc(BUFFER_SIZE);
    buff->write_buffer = (char *)malloc(BUFFER_SIZE);
    if (!buff->read_buffer || !buff->write_buffer)
    {
        if (buff->read_buffer)
            free(buff->read_buffer);
        if (buff->write_buffer)
            free(buff->write_buffer);

        errno = ENOMEM;
        return NULL;
    }

    // Initializing
    buff->write_buffer_size = BUFFER_SIZE;
    buff->read_buffer_size = 0;
    buff->write_buffer_pos = 0;
    buff->read_buffer_pos = 0;
    buff->preappend = 0;

    // Check preappend flag, save 1 or 0 and turn of the bit if needed
    if (flags & O_PREAPPEND)
    {
        buff->preappend = 1;
        flags = flags & ~O_PREAPPEND;
    }
    buff->flags = flags;

    // Check for third variable
    mode_t mode = 0;
    if (flags & O_CREAT)
    {
        va_list p1;
        va_start(p1, flags);
        // I'm on mac and it shows a warning without casting (mode_t on mac is unsigned short and va_arg promotes it to int, I checked on the server works there as well)
        mode = (mode_t)va_arg(p1, int); 
        va_end(p1);
    }

    // Open the file and check for fail
    buff->fd = open(pathname, flags, mode);
    if (buff->fd < 0)
    {
        free(buff->read_buffer);
        free(buff->write_buffer);
        free(buff);

        return NULL;
    }

    return buff;
}

// Function to write to the buffered file
ssize_t buffered_write(buffered_file_t *bf, const void *buf, size_t count)
{
    // Check if valid
    if (bf == NULL)
    {
        errno = EBADF;
        return -1;
    }

    size_t totalWritten = 0;
    char *tempBuf = (char *)buf;

    while (count > 0)
    {
        size_t remainSize = bf->write_buffer_size - bf->write_buffer_pos;

        // Check is there any room in the buffer
        if (remainSize == 0)
        {
            if (buffered_flush(bf) < 0)
            {
                errno = ENOMEM;
                return -1;
            }
            remainSize = bf->write_buffer_size;
        }

        // Write min{count, remainSize}
        char *dest = bf->write_buffer + bf->write_buffer_pos;
        size_t remainCopy = (remainSize > count) ? count : remainSize;

        memcpy(dest, tempBuf, remainCopy);

        // Update all pointers and vars
        bf->write_buffer_pos += remainCopy;
        count -= remainCopy;
        tempBuf += remainCopy;
        totalWritten += remainCopy;
    }

    return totalWritten;
}

// Function to read from the buffered file
ssize_t buffered_read(buffered_file_t *bf, void *buf, size_t count)
{
    // Check if valid
    if (bf == NULL)
    {
        errno = EBADF;
        return -1;
    }

    size_t totalRead = 0;
    char *tempBuf = (char *)buf;

    // Check there isn't something to write
    if (buffered_flush(bf) < 0)
    {
        return -1;
    }

    while (count > 0)
    {
        if (bf->read_buffer_pos == bf->read_buffer_size)
        {
            // Check if the file has less bytes than count
            ssize_t byteReaded = read(bf->fd, bf->read_buffer, BUFFER_SIZE);
            if (byteReaded == 0)
            {
                break;
            }
            // Check for valid return value (from read)
            if (byteReaded < 0)
            {
                return -1;
            }

            bf->read_buffer_size = byteReaded;
            bf->read_buffer_pos = 0;
        }

        // Send the user the min between count he asked for and the available size
        size_t available = bf->read_buffer_size - bf->read_buffer_pos;
        size_t prog = (count < available) ? count : available;
        memcpy(tempBuf, bf->read_buffer + bf->read_buffer_pos, prog);

        // update pointers and vars
        bf->read_buffer_pos += prog;
        tempBuf += prog;
        count -= prog;
        totalRead += prog;
    }

    return totalRead;
}

// Function to flush the buffer to the file
int buffered_flush(buffered_file_t *bf)
{
    // Check if valid
    if (bf == NULL)
    {
        errno = EBADF;
        return -1;
    }

    // If pos is at start, do nothing
    if (bf->write_buffer_pos == 0)
        return 0;
    // Check for O_PREAPPEND
    if (bf->preappend == 1)
    {
        off_t curFileSize = lseek(bf->fd, 0, SEEK_END);
        if (curFileSize < 0)
            return -1;

        // Return to the start fd
        if (lseek(bf->fd, 0, SEEK_SET) < 0)
            return -1;

        char *tempBuf = (char *)malloc(curFileSize);
        if (tempBuf == NULL)
        {
            errno = ENOMEM;
            return -1;
        }

        // Copy the data from fd
        ssize_t readBytes = read(bf->fd, tempBuf, curFileSize);
        if (readBytes < 0)
        {
            free(tempBuf);
            return -1;
        }

        // Return to the start of the file
        if (lseek(bf->fd, 0, SEEK_SET) < 0)
        {
            free(tempBuf);
            return -1;
        }

        // Write the data from the buffer and then from the temp buffer
        if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) < 0)
        {
            free(tempBuf);
            return -1;
        }

        if (write(bf->fd, tempBuf, readBytes) < 0)
        {
            free(tempBuf);
            return -1;
        }

        free(tempBuf);
    }
    else
    {
        if (write(bf->fd, bf->write_buffer, bf->write_buffer_pos) < 0)
            return -1;
    }

    bf->write_buffer_pos = 0;
    return 0;
}

// Function to close the buffered file
int buffered_close(buffered_file_t *bf)
{
    // Check if valid
    if (bf == NULL)
    {
        errno = EBADF;
        return -1;
    }

    int status = 0;
    int statusReason = 0;

    // Empty the write buffer
    if (buffered_flush(bf) < 0)
    {
        status = -1;
        statusReason = errno;
    }

    // Closing the file
    if (close(bf->fd) < 0)
    {
        // I don't want to run over the errno if also the buffered_flush failed
        if (status == 0)
        {
            status = -1;
            statusReason = errno;
        }
    }

    // Free everything
    free(bf->read_buffer);
    free(bf->write_buffer);
    free(bf);

    // Check if needed to update errno
    if (status == -1) {
        errno = statusReason;
    }

    return status;
}
