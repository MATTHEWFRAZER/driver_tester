#include <stdio.h>
#include <sys/ioctl.h>

#include "../common/driver_test_ioctl.h"

int main(int argc, char* argv[])
{
    DT_PATCH_REQUEST patchRequest;
    int fd = open("/dev/driver_tester");
    if (fd == -1)
    {
        printf("could not get desciptor on driver");
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, DT_PATCH, &patchRequest) < 0)
    {
        printf("failed send patch ioctl");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
