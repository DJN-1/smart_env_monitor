#ifndef OLED_IOCTL_H
#define OLED_IOCTL_H

#include <linux/ioctl.h>

#define OLED_IOC_MAGIC 'o'

// ioctl 명령어 정의
#define OLED_IOC_INIT       _IO(OLED_IOC_MAGIC, 1)
#define OLED_IOC_CLEAR      _IO(OLED_IOC_MAGIC, 2)
#define OLED_IOC_ON         _IO(OLED_IOC_MAGIC, 3)
#define OLED_IOC_OFF        _IO(OLED_IOC_MAGIC, 4)
#define OLED_IOC_CONTRAST   _IOW(OLED_IOC_MAGIC, 5, int)

#define OLED_IOC_MAXNR 5

#endif
