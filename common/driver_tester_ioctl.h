#pragma once

#define DT_IOC_MAGIC 'd'
#define DT_IOC_MAXNR 1

#define DT_PATCH_IOCTL _IOW(DT_IOC_MAGIC, 1, char*)

int dt_ioctl_handle_ioctl(struct file *pfile, unsigned int ioctl, unsigned long argument);
