#include <stdio.h>

#include "copy-task.h"

#define BLOCK_SIZE ((2 << 12) - 1)


int main(int argc, char *argv[])
{
    CopyTask *task = NULL;

    if (argc != 3)
    {
        printf ("Usage: %s <in-file> <out-file>\n",
                argv[0]);
        return 1;
    }

    task = copy_task_new (argv[1], argv[2], BLOCK_SIZE);
    if (!task)
    {
        printf ("Failed to create copy task\n");

        return 1;
    }

    copy_task_run (task);
    copy_task_free (task);

    return 0;
}
