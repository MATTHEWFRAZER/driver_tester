#include <linux/ioctl.h>
#include <linux/errno.h>
//#include <asm/uaccess.h>

#include "../detour_patching/driver_tester_detour_patching.h"

#define DT_IOC_MAGIC 'd'
#define DT_IOC_MAXNR 1

#define DT_PATCH _IOW(DT_IOC_MAGIC, 1, char*)

long int dt_ioctl_handle_ioctl(struct file *pfile, unsigned int ioctl, unsigned long argument)
{
    if(_IOC_TYPE(ioctl) != DT_IOC_MAGIC)
    {
        return -ENOTTY;
    }

    if(_IOC_NR(ioctl) > DT_IOC_MAXNR)
    {
        return -ENOTTY;
    }

    if (_IOC_NR(ioctl) == DT_PATCH && _IOC_DIR(ioctl) & _IOC_WRITE)
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
