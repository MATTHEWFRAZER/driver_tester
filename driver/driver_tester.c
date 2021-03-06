#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include "../common/driver_tester_ioctl.h"
#include "detour_patching/driver_tester_detour_patching.h"

MODULE_DESCRIPTION("driver used to test other drivers");
MODULE_LICENSE("MIT");
MODULE_AUTHOR("Matthew Frazer");

static const struct file_operations driver_tester_file_operations =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = dt_ioctl_handle_ioctl
};

static const char device_name[] = "Driver Tester";
static int device_file_major_number = 0;
static int dt_init(void)
{
    int result = 0;

    printk(KERN_NOTICE "%s(): driver init", __FUNCTION__);

    result = register_chrdev(0, device_name, &driver_tester_file_operations);
    if (result < 0)
    {
        printk(KERN_WARNING "%s(): driver tester register failed", __FUNCTION__);
    }
    else
    {
        device_file_major_number = result;
    }
    dt_detour_patching_init();
    return result;
}

void dt_exit(void)
{
    printk(KERN_NOTICE "%s(): exit", __FUNCTION__);
    dt_detour_patching_exit();
    if (device_file_major_number != 0)
    {
        unregister_chrdev(device_file_major_number, device_name);
    }
    return;
}

module_init(dt_init);
module_exit(dt_exit);
