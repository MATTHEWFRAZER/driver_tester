#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "../common/driver_test_ioctl.h"

static const char g_target_driver_name[4] = "hwkm";
static const char g_target_routine_name[14] = "proc_file_read";

#define ARGUMENT_OFFSETS_SIZE 4

ssize_t userland_proc_file_read(struct file *file, char __user *ubuf,
                                size_t count, loff_t *ppos)
{
}

void free_argument_offsets(ARGUMENT_OFFSETS *argument_offsets)
{
    if (argument_offset_size != NULL)
    {
       free(argument_offsets);
    }
}

int main(int argc, char* argv[])
{
    DT_PATCH_REQUEST patch_request;
    int fd = open("/dev/driver_tester");
    if (fd == -1)
    {
        printf("could not get desciptor on driver");
        exit(EXIT_FAILURE);
    }

    patch_request.bytes_required = 6;
    patch_request.target_driver_name = g_target_driver_name;
    patch_request.target_routine_name = g_target_routine_name;
    patch_request.target_driver_routine_offset = 26;
    patch_request.userland_routine_address = (unsigned long)userland_proc_file_read;
    patch_request.use_displaced_operands = 0;
    if (ioctl(fd, DT_PATCH, &patch_request) < 0)
    {
        printf("failed send patch ioctl");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
