#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/utsname.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Smart Environment Monitor Team");
MODULE_DESCRIPTION("Hello World Kernel Module");
MODULE_VERSION("1.0");

static int __init hello_init(void)
{
    printk(KERN_INFO "smart_env: Hello World 커널 모듈이 로드되었습니다!\n");
    printk(KERN_INFO "smart_env: 시스템 정보: %s %s\n", 
           utsname()->sysname, utsname()->release);
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "smart_env: Hello World 커널 모듈이 언로드되었습니다!\n");
}

module_init(hello_init);
module_exit(hello_exit);
