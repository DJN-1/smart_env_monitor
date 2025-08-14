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

#define DEVICE_NAME "oled_display"
#define CLASS_NAME "smart_env"
#define OLED_I2C_ADDR 0x3C

// 전역 변수들
static dev_t dev_number;
static struct class *oled_class = NULL;
static struct device *oled_device = NULL;
static struct cdev oled_cdev;
static struct i2c_client *oled_client = NULL;

// 외부 함수 선언 (oled_ssd1306_commands.c에서 구현)
extern int ssd1306_init_display(struct i2c_client *client);
extern int ssd1306_clear_display(struct i2c_client *client);
extern int ssd1306_display_on(struct i2c_client *client);
extern int ssd1306_display_off(struct i2c_client *client);
extern int ssd1306_set_contrast(struct i2c_client *client, u8 contrast);

// 파일 연산 함수들
static int oled_open(struct inode *inode, struct file *file);
static int oled_release(struct inode *inode, struct file *file);
static ssize_t oled_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset);
static long oled_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// 파일 연산 구조체
static struct file_operations oled_fops = {
    .owner = THIS_MODULE,
    .open = oled_open,
    .release = oled_release,
    .write = oled_write,
    .unlocked_ioctl = oled_ioctl,
};

// I2C 디바이스 ID 테이블
static const struct i2c_device_id oled_id[] = {
    {"ssd1306", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, oled_id);

// I2C 프로브 함수
static int oled_probe(struct i2c_client *client)
{
    printk(KERN_INFO "smart_env: OLED I2C 디바이스가 감지되었습니다! (주소: 0x%02x)\n", client->addr);
    oled_client = client;
    
    // 자동으로 디스플레이 초기화
    if (ssd1306_init_display(client) == 0) {
        printk(KERN_INFO "smart_env: OLED 자동 초기화 완료\n");
    }
    
    return 0;
}

// I2C 제거 함수
static void oled_remove(struct i2c_client *client)
{
    printk(KERN_INFO "smart_env: OLED I2C 디바이스가 제거되었습니다!\n");
    oled_client = NULL;
}

// I2C 드라이버 구조체
static struct i2c_driver oled_driver = {
    .driver = {
        .name = "ssd1306",
        .owner = THIS_MODULE,
    },
    .probe = oled_probe,
    .remove = oled_remove,
    .id_table = oled_id,
};

// 파일 열기
static int oled_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "smart_env: OLED 디바이스 파일이 열렸습니다\n");
    return 0;
}

// 파일 닫기
static int oled_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "smart_env: OLED 디바이스 파일이 닫혔습니다\n");
    return 0;
}

// 데이터 쓰기
static ssize_t oled_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    char kernel_buffer[64];
    int ret;

    // 사용자 공간에서 문자열 복사
    if (len >= sizeof(kernel_buffer))
        len = sizeof(kernel_buffer) - 1;

    if (copy_from_user(kernel_buffer, buffer, len))
        return -EFAULT;

    kernel_buffer[len] = '\0';

    if (!oled_client)
        return -ENODEV;

    printk(KERN_INFO "smart_env: OLED 문자열 수신: %s\n", kernel_buffer);

    // OLED 화면 지우기
    ret = ssd1306_clear_display(oled_client);
    if (ret < 0) {
        printk(KERN_ERR "smart_env: 화면 지우기 실패\n");
        return ret;
    }

    // 텍스트 렌더링
    ret = ssd1306_render_text(oled_client, kernel_buffer, 0);  // 0번 페이지에 출력
    if (ret < 0) {
        printk(KERN_ERR "smart_env: 텍스트 렌더링 실패\n");
        return ret;
    }

    printk(KERN_INFO "smart_env: OLED 텍스트 출력 완료\n");
    return len;
}


// ioctl 핸들러
static long oled_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    
    if (!oled_client) {
        printk(KERN_ERR "smart_env: I2C 클라이언트가 없습니다\n");
        return -ENODEV;
    }
    
    if (_IOC_TYPE(cmd) != OLED_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > OLED_IOC_MAXNR) return -ENOTTY;
    
    switch (cmd) {
        case OLED_IOC_INIT:
            printk(KERN_INFO "smart_env: ioctl - OLED 초기화\n");
            ret = ssd1306_init_display(oled_client);
            break;
            
        case OLED_IOC_CLEAR:
            printk(KERN_INFO "smart_env: ioctl - OLED 화면 지우기\n");
            ret = ssd1306_clear_display(oled_client);
            break;
            
        case OLED_IOC_ON:
            printk(KERN_INFO "smart_env: ioctl - OLED 켜기\n");
            ret = ssd1306_display_on(oled_client);
            break;
            
        case OLED_IOC_OFF:
            printk(KERN_INFO "smart_env: ioctl - OLED 끄기\n");
            ret = ssd1306_display_off(oled_client);
            break;
            
        case OLED_IOC_CONTRAST:
            printk(KERN_INFO "smart_env: ioctl - 대비 설정: %lu\n", arg);
            if (arg > 255) return -EINVAL;
            ret = ssd1306_set_contrast(oled_client, (u8)arg);
            break;
            
        default:
            return -ENOTTY;
    }
    
    return ret;
}

// 모듈 초기화
static int __init oled_driver_init(void)
{
    int ret;
    
    printk(KERN_INFO "smart_env: OLED I2C 드라이버 초기화 중...\n");
    
    // 1. 디바이스 번호 할당
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "smart_env: 디바이스 번호 할당 실패\n");
        return ret;
    }
    
    // 2. 디바이스 클래스 생성
    oled_class = class_create(CLASS_NAME);
    if (IS_ERR(oled_class)) {
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "smart_env: 클래스 생성 실패\n");
        return PTR_ERR(oled_class);
    }
    
    // 3. 디바이스 파일 생성
    oled_device = device_create(oled_class, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(oled_device)) {
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "smart_env: 디바이스 생성 실패\n");
        return PTR_ERR(oled_device);
    }
    
    // 4. 문자 디바이스 초기화 및 등록
    cdev_init(&oled_cdev, &oled_fops);
    ret = cdev_add(&oled_cdev, dev_number, 1);
    if (ret < 0) {
        device_destroy(oled_class, dev_number);
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "smart_env: 문자 디바이스 추가 실패\n");
        return ret;
    }
    
    // 5. I2C 드라이버 등록
    ret = i2c_add_driver(&oled_driver);
    if (ret < 0) {
        cdev_del(&oled_cdev);
        device_destroy(oled_class, dev_number);
        class_destroy(oled_class);
        unregister_chrdev_region(dev_number, 1);
        printk(KERN_ERR "smart_env: I2C 드라이버 등록 실패\n");
        return ret;
    }
    
    printk(KERN_INFO "smart_env: OLED 드라이버 초기화 완료! /dev/%s 생성됨\n", DEVICE_NAME);
    return 0;
}

// 모듈 종료
static void __exit oled_driver_exit(void)
{
    i2c_del_driver(&oled_driver);
    cdev_del(&oled_cdev);
    device_destroy(oled_class, dev_number);
    class_destroy(oled_class);
    unregister_chrdev_region(dev_number, 1);
    printk(KERN_INFO "smart_env: OLED 드라이버가 제거되었습니다\n");
}

module_init(oled_driver_init);
module_exit(oled_driver_exit);
