#include <linux/ioctl.h>
#include <linux/errno.h>
//#include <asm/uaccess.h>

#include "../detour_patching/driver_tester_detour_patching.h"
#include "../../common/driver_tester_ioctl.h"

long dt_ioctl_handle_ioctl(struct file *pfile, unsigned int ioctl, unsigned long argument)
{
    if(_IOC_TYPE(ioctl) != DT_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    if(_IOC_NR(ioctl) > DT_IOC_MAXNR)
    {
        return -ENOTTY;
    }

    if (_IOC_NR(ioctl) == DT_PATCH_IOCTL && _IOC_DIR(ioctl) & _IOC_WRITE)
    {
        //void __user *userPointer = (void __user *)argument;
        //int size = _IOC_SIZE(ioctl);
        //if(!access_ok((userPointer), (size)))
        //{
        //    return -ENOTTY;
        //}

        if(!dt_detour_patching_patch((DT_PATCH_REQUEST *)argument))
        {
            return -ENOTTY;
        }
    }
    return 0;
}
