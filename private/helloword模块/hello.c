#include <linux/module.h> // 所有模块都需要
#include <linux/kernel.h> // printk和KERN_INFO等等
#include <linux/init.h>   // __init、__exit的定义


// 作者信息
MODULE_AUTHOR("Fw[a]rd");
// 模块描述
MODULE_DESCRIPTION("A test module");
// 版本号
MODULE_VERSION("1:1.0");
// 以上都可以通过modinfo查看


static int __init hello_init(void)
{
    
    printk(KERN_INFO "Hello: Hello, World!\n");
   
    return 0;
}


static void __exit hello_exit(void)
{   
    printk(KERN_INFO "Hello: Goodbye, crazy world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL"); 