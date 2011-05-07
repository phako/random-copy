#ifndef __COPY_TASK_H
#define __COPY_TASK_H

#include <unistd.h>

typedef struct _CopyTask CopyTask;

int
copy_task_copy_block (CopyTask *self,
                      size_t    index);

void
copy_task_generate_offset_list (CopyTask *self);

void
copy_task_free (CopyTask *self);

CopyTask *
copy_task_new (const char *source,
               const char *dest,
               size_t      block_size);
void
copy_task_run (CopyTask *self);
#endif
