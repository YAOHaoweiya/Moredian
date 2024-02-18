#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>


#define WIEGAND_MAJOR 243
#define TIMER_DELAY HZ/4
#define DEVICE_NAME "wiegand"

bool TIMEER_FLAG = false;

bool READ_FLAG = false;
static struct class *cdev_class;

static DECLARE_WAIT_QUEUE_HEAD( wiegand_wait);// 定义等待队列

struct wiegand_dev
{
    char wiegand[34];  //Wiegand26-34
    int hid;
    int pid;
    int count;  //Global Counter
    struct cdev cdev;
    struct timer_list   wiegand_timer;
    struct work_struct  pen_event_work;
    struct workqueue_struct *ts_workqueue;
    int gpio_d0;//gpio no
    int gpio_d1;
    int d0_irq;// irq no
    int d1_irq;
};

static struct wiegand_dev *rf_card;

static char convert_data26(void)
{
    int i,even,odd;
    int cardno ;

   printk("%s: begin\n", __func__);

   //偶校验
    even = 0;
    for(i = 1; i < 13; i++)
    {
        if(rf_card->wiegand[i] == 1)
        {
            even = (~even) & 0x01;
        }
    }
    if(even != rf_card->wiegand[0])
    {
        rf_card->count = 0;
        printk("%s %d\n", __func__, __LINE__);
        goto error;
    }
   //奇校验
    odd = 1;
    for(i = 13; i< 25; i++)
    {
        if(rf_card->wiegand[i] == 1)
        {
            odd = (~odd)& 0x01;
        }
    }
    if(odd != rf_card->wiegand[25])
    {
        rf_card->count = 0;
        printk("%s %d\n", __func__, __LINE__);
        goto error;
    }

   //奇偶校验通过
    rf_card->hid = 0;
    for(i = 1 ; i<=8; i++) //hid转换
    {
        rf_card->hid = rf_card->hid << 1 | rf_card->wiegand[i];
    }

    rf_card->pid = 0;
    for(i = 9 ; i<=24; i++) //pid转换
    {
        rf_card->pid = rf_card->pid << 1 | rf_card->wiegand[i];
    }

    cardno = rf_card->hid << 16 | rf_card->pid;
    rf_card->count = 0;

    printk("%s cardno=(0x%x, %d) end\n", __func__, cardno, cardno);
    return 0;

error:
    printk("Parity Efficacy Error!\n");
    return -1;
}

static char convert_data34(void)
{
    int i,even,odd;
    int cardno;

   printk("%s: begin\n", __func__);


   //偶校验
    even = 0;
    for(i = 1; i < 17; i++)
    {
        if(rf_card->wiegand[i] == 1)
        {
            even = (~even) & 0x01;
        }
    }
    if(even != rf_card->wiegand[0])
    {
        rf_card->count = 0;
        printk("%s %d\n", __func__, __LINE__);
        goto error;
    }
   //奇校验
    odd = 1;
    for(i = 17; i < 34; i++)
    {
        if(rf_card->wiegand[i] == 1)
        {
            odd = (~odd)& 0x01;
        }
    }
    if(odd != rf_card->wiegand[33])
    {
        rf_card->count = 0;
        printk("%s %d\n", __func__, __LINE__);
        goto error;
    }

   //奇偶校验通过
    rf_card->hid = 0;
    for(i = 1 ; i<=16; i++) //hid转换
    {
        rf_card->hid = rf_card->hid << 1 | rf_card->wiegand[i];
    }

    rf_card->pid = 0;
    for(i = 17 ; i<=32; i++) //pid转换
    {
        rf_card->pid = rf_card->pid << 1 | rf_card->wiegand[i];
    }

    cardno = rf_card->hid << 16 | rf_card->pid;
    rf_card->count = 0;

    printk("wiegandr cardno=(0x%x, %d)\n", cardno, cardno);
    return 0;

error:
    printk("Parity Efficacy Error!\n");
    return -1;
}

//static void wiegand_pen_irq_work(struct work_struct *work)

static void wiegand_do_timer(unsigned long arg)
{

    //printk("rf_card->count=%d arg=%lu\n",rf_card->count, arg);
    printk("%s  card count: %d\n", __func__, rf_card->count );
    disable_irq(rf_card->d0_irq);
    disable_irq(rf_card->d1_irq);//防止wieg_data在转换期间发生变化

   if(rf_card->count == 26)
       convert_data26();
   else if(rf_card->count == 34)
      convert_data34();


    READ_FLAG = true;
   wake_up_interruptible(&wiegand_wait);//唤醒等待队列中的所有的进程

    rf_card->count =0 ;


    enable_irq(rf_card->d0_irq);
    enable_irq(rf_card->d1_irq);
    TIMEER_FLAG = false;

    printk("%s end\n", __func__);

}



static irqreturn_t wiegand_handle_irq0(int irq, void *dev_id)
{
    disable_irq_nosync(rf_card->d0_irq);
    {
        rf_card->wiegand[rf_card->count] = 0;
        rf_card->count++;
    }

    enable_irq(rf_card->d0_irq);

    if(TIMEER_FLAG == false)
    {
        rf_card->wiegand_timer.expires = jiffies + TIMER_DELAY;
        add_timer(&rf_card->wiegand_timer);
        TIMEER_FLAG = true;
    }
    return IRQ_HANDLED;

}

static irqreturn_t wiegand_handle_irq1(int irq, void *dev_id)
{
    disable_irq_nosync(rf_card->d1_irq);
    {
        rf_card->wiegand[rf_card->count] = 1;
        rf_card->count ++;
    }

    enable_irq(rf_card->d1_irq);


    if(TIMEER_FLAG== false)
    {
        rf_card->wiegand_timer.expires = jiffies + TIMER_DELAY;
        add_timer(&rf_card->wiegand_timer);
        TIMEER_FLAG = true;
    }
    return IRQ_HANDLED;
}

static ssize_t wiegand_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
    printk("%s \n", __func__);
    int max_count;
    char cardno[12];
    memset( cardno , 0, sizeof(cardno));

    max_count = sizeof(cardno);

    if(size > max_count)
    {
        size = max_count;
    }
    //等待 队列头部的队列被唤醒，如果READ_FLAG 不为true 则继续等待。
    wait_event_interruptible( wiegand_wait, READ_FLAG);

    sprintf(cardno,"%d",rf_card->hid << 16 | rf_card->pid);
    printk("%s  cardno : %s\n", __func__, cardno );

    copy_to_user(buf, cardno, size);
    READ_FLAG = false;//重置等待队列的条件

    return size;
}

static ssize_t wiegand_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
    printk("%s \n", __func__);
    return 0;
}

static int wiegand_request_irqs(void)
{
    int ret;

    rf_card->d0_irq = gpio_to_irq(rf_card->gpio_d0);
    rf_card->d1_irq = gpio_to_irq(rf_card->gpio_d1);

    printk("%s:d0_irq=%d,d1_irq=%d\n",__func__, rf_card->d0_irq , rf_card->d1_irq);

    ret = request_irq(rf_card->d0_irq,wiegand_handle_irq0,IRQF_SHARED | IRQF_TRIGGER_FALLING,"wiegandr_data0",rf_card);

    if(ret)
    {
        printk("wiegandr %s:request rf_card->d0_irq):%d,ret:%d failed!\n",__func__,rf_card->d0_irq,ret);
        return -1;
    }
    ret = request_irq(rf_card->d1_irq,wiegand_handle_irq1,IRQF_SHARED | IRQF_TRIGGER_FALLING,"wiegandr_data1",rf_card);

    if(ret)
    {
        printk("wiegandr %s:request rf_card->d1_irq:%d,ret:%d failed!\n",__func__,rf_card->d1_irq,ret);
        return -1;
    }

    printk(KERN_INFO"%s:request irqs success!\n",__func__);
    return 0;
}

static int wiegand_open(struct inode *inode, struct file *filp)
{
    printk("%s \n", __func__);

    TIMEER_FLAG = false;

   //setup_timer(&rf_card->wiegand_timer,wiegand_do_timer,0);

    memset(rf_card->wiegand, 0x00, 26);
    rf_card->count = 0;

   //enable_irqs();
    printk("%s has been opened \n", __func__);
    return 0;
}

static void free_irqs(void)
{
    free_irq(rf_card->d0_irq,rf_card);
    free_irq(rf_card->d1_irq,rf_card);
}

int wiegand_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations wiegand_fops =
{
    .owner = THIS_MODULE,
    .read = wiegand_read,
    .write = wiegand_write,
    .open = wiegand_open,
    .release = wiegand_release,
};


static int wiegand_probe(struct platform_device *pdev)
{
    dump_stack();
    printk("%s begin \n", __func__);
    const char *str = NULL;
    of_property_read_string(pdev->dev.of_node, "wiegand_name", &str);
    printk("%s wiegand_name %s \n", __func__, str);


    int err,result;
    dev_t devno = MKDEV(WIEGAND_MAJOR, 1);


   //if(WIEGAND_MAJOR)
    if(0)
    {
        result = register_chrdev_region(devno, 1, DEVICE_NAME);
    }
    else
    {
        result = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
    }

    if(result < 0)
    {
        printk("%s:register_chrdev_region error\n", __func__);
        return result;
    }

    rf_card = kmalloc(sizeof(struct wiegand_dev), GFP_KERNEL);
    if(!rf_card)
    {
        result = -ENOMEM;
        goto fail_malloc;
    }

    memset(rf_card, 0, sizeof(struct wiegand_dev));

    rf_card->count = 0;

    cdev_init(&(rf_card->cdev), &wiegand_fops);

    rf_card->cdev.owner = THIS_MODULE;

    err = cdev_add(&rf_card->cdev, devno, 1);

    if(err)
    {
        unregister_chrdev_region(devno,1);
        kfree(rf_card);
        free_irqs();
        return err;
    }

    cdev_class = class_create(THIS_MODULE, DEVICE_NAME);//动态创建设备结点
    if(IS_ERR(cdev_class))
    {
        printk("ERR:cannot create a cdev_class\n");
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    device_create(cdev_class, NULL, devno, 0, DEVICE_NAME);

 


    rf_card->gpio_d0 = of_get_named_gpio(pdev->dev.of_node, "wiegn0-gpios", 0);
    rf_card->gpio_d1 = of_get_named_gpio(pdev->dev.of_node, "wiegn1-gpios", 0);
    if (!gpio_is_valid(rf_card->gpio_d0)  || !gpio_is_valid(rf_card->gpio_d1) ) {
        printk("%s -gpio: %d is invalid  -gpio %d is invalid\n",__func__, rf_card->gpio_d0, rf_card->gpio_d1);
    }


    printk("%s gpio_d0 %d, gpio_d1 %d\n", __func__, rf_card->gpio_d0, rf_card->gpio_d1);
    result = devm_gpio_request_one(&pdev->dev,
                        rf_card->gpio_d0,
                        GPIOF_IN,
                        "INTR0");
   if (result) {
       printk("%s request gpio  error  %d\n", __func__, rf_card->gpio_d0);

   }
    result = devm_gpio_request_one(&pdev->dev,
                        rf_card->gpio_d1,
                        GPIOF_IN,
                        "INTR1");
   if (result) {
       printk("%s request gpio  error  %d\n", __func__, rf_card->gpio_d1);

   }

    result = wiegand_request_irqs();

    if(result < 0)
    {
        printk("%s: error\n",__func__);
        return result;
    }

    setup_timer(&rf_card->wiegand_timer, wiegand_do_timer, 0);


    printk("%s  end!\n",__func__);
    return 0;

fail_malloc:
    unregister_chrdev_region(devno,1);

    return result;
   return 0;
}

static void wiegand_shutdown(struct platform_device *pdev)
{
    printk("%s \n", __func__);
}

/* match dts */
static const struct of_device_id of_wiegand_match[] = {
   { .compatible = "wiegand", },
   {},
};

static struct platform_driver  wiegand_driver = {
   .probe    =  wiegand_probe,
   .shutdown  =  wiegand_shutdown,
   .driver       = {
      .name  = "wiegand",
      .of_match_table = of_wiegand_match,
   },
};

static int __init wiegand_init(void)
{
    dump_stack();
    printk("%s\n",__func__);
    int ret = 0;
    ret =platform_driver_register(&wiegand_driver);
    if (ret) {
            printk( " %s failed!\n", __func__);
            return ret;
    }
    printk("%s ok!\n", __func__);
    return ret;
 }

static void __exit wiegand_exit(void)
{
    printk("%s\n",__func__);
    platform_driver_unregister(&wiegand_driver);

    cdev_del(&rf_card->cdev);
    free_irqs();
    kfree(rf_card);
    unregister_chrdev_region(MKDEV(WIEGAND_MAJOR,0),1);
    printk(KERN_INFO"%s removed\n",DEVICE_NAME);
}
module_init( wiegand_init );
module_exit( wiegand_exit );

MODULE_AUTHOR("xiangshaoxiong");
MODULE_DESCRIPTION(" wiegand driver");
MODULE_LICENSE("GPL");
