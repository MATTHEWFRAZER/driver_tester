#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "../common/driver_test_ioctl.h"

static const char gtargetDriverName[4] = "hwkm";
static const char gtargetRoutineName[14] = "proc_file_read";

#define ARGUMENT_OFFSETS_SIZE 4

ssize_t userland_proc_file_read(struct file *file, char __user *ubuf,
                                size_t count, loff_t *ppos)
{
}

ARGUMENT_OFFSETS *malloc_argument_offsets()
{
    int i;
    ARGUMENT_OFFSETS* argumentOffsets = (ARGUMENT_OFFSETS*)malloc(sizeof(ARGUMENT_OFFSETS) * ARGUMENT_OFFSETS)
    for(i = 0; i < ARGUMENT_OFFSETS_SIZE; ++i )
    {
        // wondering if we have to worry about padding
        argmentOffsets[i].
    }
}


int main(int argc, char* argv[])
{
    DT_PATCH_REQUEST patchRequest;
    int fd = open("/dev/driver_tester");
    if (fd == -1)
    {
        printf("could not get desciptor on driver");
        exit(EXIT_FAILURE);
    }

    patchRequest.bytesRequired = 6;
    patchRequest.targetDriverName = gtargetDriverName;
    patchRequest.targetRoutineName = gtargetRoutineName;
    patchRequest.userlandRoutineAddress = (unsigned long)userland_proc_file_read;
    if (ioctl(fd, DT_PATCH, &patchRequest) < 0)
    {
        printf("failed send patch ioctl");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
