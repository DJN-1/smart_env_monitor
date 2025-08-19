#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "../include/oled_ioctl.h"
#include "../include/oled_ssd1306_commands.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Smart Environment Monitor Team");
MODULE_DESCRIPTION("OLED SSD1306 I2C Device Driver");
MODULE_VERSION("1.0");

#define DEVICE_NAME   "oled_display"
#define CLASS_NAME    "smart_env"

static dev_t             dev_number;
static struct class     *oled_class;
static struct device    *oled_device;
static struct cdev       oled_cdev;
static struct i2c_client *oled_client;

/* File operations prototypes */
static int      oled_open(struct inode *inode, struct file *file);
static int      oled_release(struct inode *inode, struct file *file);
static ssize_t  oled_write(struct file *file,
                           const char __user *buffer,
                           size_t len,
                           loff_t *offset);
static long     oled_ioctl(struct file *file,
                           unsigned int cmd,
                           unsigned long arg);

/* Character device operations */
static const struct file_operations oled_fops = {
    .owner          = THIS_MODULE,
    .open           = oled_open,
    .release        = oled_release,
    .write          = oled_write,
    .unlocked_ioctl = oled_ioctl,
};

/* I2C device ID table */
static const struct i2c_device_id oled_id[] = {
    { "ssd1306", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, oled_id);

/* Probe: called when the device is matched */
static int oled_probe(struct i2c_client *client)
{
    int ret;

    pr_info("smart_env: OLED I2C device at 0x%02x\n", client->addr);
    oled_client = client;

    ret = ssd1306_init_display(client);
    if (ret) {
        pr_err("smart_env: display init failed (%d)\n", ret);
        return ret;
    }

    pr_info("smart_env: OLED initialized\n");
    return 0;
}

/* Remove: called on driver detach */
static void oled_remove(struct i2c_client *client)
{
    pr_info("smart_env: OLED I2C device removed\n");
    oled_client = NULL;
}

/* I2C driver structure */
static struct i2c_driver oled_driver = {
    .driver = {
        .name  = "ssd1306",
        .owner = THIS_MODULE,
    },
    .probe    = oled_probe,
    .remove   = oled_remove,
    .id_table = oled_id,
};

/* open(): nothing special */
static int oled_open(struct inode *inode, struct file *file)
{
    pr_info("smart_env: OLED device opened\n");
    return 0;
}

/* release(): nothing special */
static int oled_release(struct inode *inode, struct file *file)
{
    pr_info("smart_env: OLED device closed\n");
    return 0;
}

/* write(): auto-clear + auto-wrap render */
static ssize_t oled_write(struct file *file,
                          const char __user *buffer,
                          size_t len,
                          loff_t *offset)
{
    char kernel_buffer[128];
    int ret;

    if (len >= sizeof(kernel_buffer))
        len = sizeof(kernel_buffer) - 1;

    if (copy_from_user(kernel_buffer, buffer, len))
        return -EFAULT;
    kernel_buffer[len] = '\0';

    if (!oled_client)
        return -ENODEV;

    ret = ssd1306_render_auto_wrapped(oled_client, kernel_buffer);
    if (ret < 0) {
        pr_err("smart_env: render failed (%d)\n", ret);
        return ret;
    }

    pr_info("smart_env: OLED text rendered\n");
    return len;
}

/* ioctl(): control commands */
static long oled_ioctl(struct file *file,
                       unsigned int cmd,
                       unsigned long arg)
{
    int ret = 0;

    if (!oled_client)
        return -ENODEV;
    if (_IOC_TYPE(cmd) != OLED_IOC_MAGIC ||
        _IOC_NR(cmd) > OLED_IOC_MAXNR)
        return -ENOTTY;

    switch (cmd) {
    case OLED_IOC_INIT:
        pr_info("smart_env: IOCTL INIT\n");
        ret = ssd1306_init_display(oled_client);
        break;

    case OLED_IOC_CLEAR:
        pr_info("smart_env: IOCTL CLEAR\n");
        ret = ssd1306_clear_display(oled_client);
        break;

    case OLED_IOC_ON:
        pr_info("smart_env: IOCTL ON\n");
        ret = ssd1306_display_on(oled_client);
        break;

    case OLED_IOC_OFF:
        pr_info("smart_env: IOCTL OFF\n");
        ret = ssd1306_display_off(oled_client);
        break;

    case OLED_IOC_CONTRAST:
        pr_info("smart_env: IOCTL CONTRAST %lu\n", arg);
        if (arg > 255)
            return -EINVAL;
        ret = ssd1306_set_contrast(oled_client, (u8)arg);
        break;

    default:
        return -ENOTTY;
    }

    return ret;
}

/* Module init: register char device and I2C driver */
static int __init oled_driver_init(void)
{
    int ret;

    pr_info("smart_env: init OLED driver\n");

    /* 1) allocate device number */
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("smart_env: alloc_chrdev_region failed\n");
        return ret;
    }

    /* 2) create device class */
    oled_class = class_create(CLASS_NAME);
    if (IS_ERR(oled_class)) {
        unregister_chrdev_region(dev_number, 1);
        pr_err("smart_env: class_create failed\n");
        return PTR_ERR(oled_class);
    }

    /* 3) create device node */
    oled_device = device_create(oled_class, NULL,
                               dev_number, NULL,
                               DEVICE_NAME);
    if (IS_ERR(oled_device)) {
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        pr_err("smart_env: device_create failed\n");
        return PTR_ERR(oled_device);
    }

    /* 4) initialize and add cdev */
    cdev_init(&oled_cdev, &oled_fops);
    oled_cdev.owner = THIS_MODULE;
    ret = cdev_add(&oled_cdev, dev_number, 1);
    if (ret) {
        device_destroy(oled_class, dev_number);
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        pr_err("smart_env: cdev_add failed\n");
        return ret;
    }

    /* 5) register I2C driver */
    ret = i2c_add_driver(&oled_driver);
    if (ret) {
        cdev_del(&oled_cdev);
        device_destroy(oled_class, dev_number);
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        pr_err("smart_env: i2c_add_driver failed\n");
        return ret;
    }

    pr_info("smart_env: OLED driver ready, /dev/%s\n", DEVICE_NAME);
    return 0;
}

/* Module exit: unregister I2C driver and char device */
static void __exit oled_driver_exit(void)
{
    i2c_del_driver(&oled_driver);
    cdev_del(&oled_cdev);
    device_destroy(oled_class, dev_number);
    class_destroy(oled_class);
    unregister_chrdev_region(dev_number, 1);
    pr_info("smart_env: OLED driver removed\n");
}

module_init(oled_driver_init);
module_exit(oled_driver_exit);
