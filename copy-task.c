/*
 * Copyright (c) 2011, Jens Georg All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.  Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions and
 * the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "copy-task.h"

struct _CopyTask {
    size_t in_size;
    size_t *offset_list;
    size_t offset_list_length;
    size_t block_size;
    FILE *in;
    int in_fd;
    FILE *out;
    int out_fd;
    char *buffer;
};

int copy_task_copy_block (CopyTask *self,
                          size_t index)
{
    size_t bytes_read;
    struct timeval tv[2];

    memset (tv, 0, sizeof (struct timeval) * 2);
    tv[0].tv_sec = time (&tv[1].tv_sec);

    if (fseek (self->in, self->offset_list[index], SEEK_SET))
        return -1;

    if (fseek (self->out, self->offset_list[index], SEEK_SET))
        return -1;

    bytes_read = fread (self->buffer, 1, self->block_size, self->in);
    if (ferror (self->in))
        return -1;

    fwrite (self->buffer, 1, bytes_read, self->out);
    if (ferror (self->in))
        return -1;

    if (futimes (self->out_fd, tv))
        return -1;

    return 0;
}

void copy_task_generate_offset_list (CopyTask *self)
{
    size_t i;
    size_t j;

    self->offset_list_length = (self->in_size / self->block_size) + 2;
    self->offset_list = (size_t *) malloc (self->offset_list_length * sizeof (size_t));
    memset (self->offset_list, 0, self->offset_list_length * sizeof (size_t));

    /* Write beginning of file first */
    self->offset_list[0] = 0;

    /* Then write end of file */
    self->offset_list[1] = self->in_size - self->block_size;

    /* Use Knuth/Fisher-Yates shuffle to get a random block distribution */
    printf ("Generating offset list: %lu... ", self->offset_list_length);
    self->offset_list[2] = self->block_size;
    for (i = 3; i < self->offset_list_length; ++i)
    {
        j = random () % i;
        self->offset_list[i] = self->offset_list[j];
        self->offset_list[j] = (i - 1) * self->block_size;
    }
    printf ("Done.\n");
}

void copy_task_free (CopyTask *self)
{
    if (self->in)
        fclose (self->in);

    if (self->out)
        fclose (self->out);

    if (self->offset_list)
        free (self->offset_list);

    if (self->buffer)
        free (self->buffer);

    free (self);
}

CopyTask *copy_task_new (const char* source,
                         const char* dest,
                         size_t      block_size)
{
    CopyTask *self = NULL;
    struct stat info;

    self = (CopyTask *) malloc (sizeof (CopyTask));
    memset (self, 0, sizeof (CopyTask));
    self->buffer = malloc (block_size);

    self->in = fopen (source, "rb");
    if (!self->in)
        goto error;

    self->in_fd = fileno (self->in);

    self->out = fopen (dest, "wb");
    if (!self->out)
        goto error;

    self->out_fd = fileno (self->out);

    memset (&info, 0, sizeof (struct stat));
    if (fstat (self->in_fd, &info))
        goto error;

    self->in_size = info.st_size;
    self->block_size = block_size;

    return self;
error:
    copy_task_free (self);

    return NULL;
}

int copy_task_run (CopyTask *self)
{
    size_t index;

    printf ("Pre-allocating file... ");
    if (ftruncate (self->out_fd, self->in_size) != 0)
    {
        printf ("Failed: %s\n", strerror (errno));

        return -1;
    }
    printf ("Done.\n");

    copy_task_generate_offset_list (self);
    for (index = 0; index < self->offset_list_length; ++index)
    {
        printf("Copying chunk #%010lu/%010lu\r", index, self->offset_list_length);
        fflush (stdout);
        if (copy_task_copy_block (self, index))
            return -1;

        usleep (100000);
    }

    return 0;
}


