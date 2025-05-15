#ifndef _PCHAR_IOCTL_H
#define _PCHAR_IOCTL_H

#include<linux/ioctl.h>

typedef struct{
    short size;
    short len;
    short avail;
}fifo_info_t;

#define FIFO_CLEAR _IO('x',1)
#define FIFO_GET_INFO _IOR('x',2,fifo_info_t)
#define FIFO_RESIZE _IOW('x',3,long)

#endif