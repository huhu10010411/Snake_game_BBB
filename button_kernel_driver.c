#include <linux/gpio/consumer.h>        /* The descriptor-base GPIO interface */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/fs.h>                   /* file system */
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/poll.h>

#define LEFT_CODE       1
#define RIGHT_CODE      2
#define UP_CODE         3
#define DOWN_CODE       4
#define OK_CODE         5


int kernel_buffer[5];
int btn_code_buff[6] = {0, 1, 2, 3, 4, 5};

struct my_device {
    dev_t device_number;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    struct gpio_desc *BTN_Left;
    struct gpio_desc *BTN_Right;
    struct gpio_desc *BTN_Up;
    struct gpio_desc *BTN_Down;
    int irq_BTN_Left;
    int irq_BTN_Right;
    int irq_BTN_Up;
    int irq_BTN_Down;
};

struct my_device my_button;

//Waitqueue
DECLARE_WAIT_QUEUE_HEAD(wait_queue_btn_data);
static bool canRead = false;

/***********************************Driver function***********************************/
static int button_open (struct inode *inode, struct file *filp);

static ssize_t button_read (struct file *filp, char __user *buff, size_t count, loff_t *f_pos);

static int button_release (struct inode *inode, struct file *filp);

static unsigned button_poll(struct file *filp, struct poll_table_struct *wait);

/** File operation struct */
static struct file_operations file_ops =
{
    .open = button_open,
    .read = button_read,
    .release = button_release,
    .poll = button_poll,
    .owner = THIS_MODULE
} ;
static irqreturn_t BTN_Left_pushed_irq_handler(int irq, void *dev_id)
{
    memcpy(kernel_buffer, &btn_code_buff[LEFT_CODE], 1);
    canRead = true;
    wake_up_interruptible(&wait_queue_btn_data);
    pr_info("BTN_Left interrupt\n");
    return IRQ_HANDLED;
}

static irqreturn_t BTN_Right_pushed_irq_handler(int irq, void *dev_id)
{
    memcpy(kernel_buffer, &btn_code_buff[RIGHT_CODE], 1);
    canRead = true;
    wake_up_interruptible(&wait_queue_btn_data);
    pr_info("BTN_Right interrupt\n");
    return IRQ_HANDLED;
}
static irqreturn_t BTN_Up_pushed_irq_handler(int irq, void *dev_id)
{
    memcpy(kernel_buffer, &btn_code_buff[UP_CODE], 1);
    canRead = true;
    wake_up_interruptible(&wait_queue_btn_data);
    pr_info("BTN_Up interrupt\n");
    return IRQ_HANDLED;
}
static irqreturn_t BTN_Down_pushed_irq_handler(int irq, void *dev_id)
{
    memcpy(kernel_buffer, &btn_code_buff[DOWN_CODE], 1);
    canRead = true;
    wake_up_interruptible(&wait_queue_btn_data);
    pr_info("BTN_Down interrupt\n");
    return IRQ_HANDLED;
}

static const struct of_device_id gpiod_dt_ids[] = {
    { .compatible = "packt,gpio-descriptor-sample", },
    { /* sentinel */ }
};

static int my_pdrv_probe (struct platform_device *pdev)
{
    int retval_Left, retval_Right, retval_Up, retval_Down;
    struct device *dev = &pdev->dev;

    /* allocate a GPIO descriptor */
    my_button.BTN_Left = gpiod_get(dev, "BTN_Left", GPIOD_IN);
    my_button.BTN_Right = gpiod_get(dev, "BTN_Right", GPIOD_IN);
    my_button.BTN_Up = gpiod_get(dev, "BTN_Up",  GPIOD_IN);
    my_button.BTN_Down = gpiod_get(dev, "BTN_Down", GPIOD_IN);

    /*  set the debounce timeout of a given GPIO */
    gpiod_set_debounce(my_button.BTN_Left, 150);
    gpiod_set_debounce(my_button.BTN_Right, 150);
    gpiod_set_debounce(my_button.BTN_Up, 150);
    gpiod_set_debounce(my_button.BTN_Down, 150);

    /*get the IRQ number that corresponds to the given GPIO descriptor*/
    my_button.irq_BTN_Left    = gpiod_to_irq(my_button.BTN_Left);
    my_button.irq_BTN_Right   = gpiod_to_irq(my_button.BTN_Right);
    my_button.irq_BTN_Up      = gpiod_to_irq(my_button.BTN_Up);
    my_button.irq_BTN_Down    = gpiod_to_irq(my_button.BTN_Down);

    retval_Left = request_threaded_irq(my_button.irq_BTN_Left, NULL,              \
                                       BTN_Left_pushed_irq_handler,     \
                                       IRQF_TRIGGER_FALLING | IRQF_ONESHOT, \
                                       "gpio-descriptor-sample", NULL);

    retval_Right = request_threaded_irq(my_button.irq_BTN_Right, NULL,              \
                                        BTN_Right_pushed_irq_handler,    \
                                        IRQF_TRIGGER_FALLING | IRQF_ONESHOT, \
                                        "gpio-descriptor-sample", NULL);

    retval_Up = request_threaded_irq(my_button.irq_BTN_Up, NULL,              \
                                     BTN_Up_pushed_irq_handler,       \
                                     IRQF_TRIGGER_FALLING | IRQF_ONESHOT, \
                                     "gpio-descriptor-sample", NULL);

    retval_Down = request_threaded_irq(my_button.irq_BTN_Down, NULL,              \
                                       BTN_Down_pushed_irq_handler,     \
                                       IRQF_TRIGGER_FALLING | IRQF_ONESHOT, \
                                       "gpio-descriptor-sample", NULL);

    pr_info("Hello! device probed!\n");
    return 0;
}


static int my_pdrv_remove(struct platform_device *pdev)
{
    free_irq(my_button.irq_BTN_Left, NULL);
    free_irq(my_button.irq_BTN_Right, NULL);
    free_irq(my_button.irq_BTN_Up, NULL);
    free_irq(my_button.irq_BTN_Down, NULL);

    gpiod_put(my_button.BTN_Left);
    gpiod_put(my_button.BTN_Right);
    gpiod_put(my_button.BTN_Up);
    gpiod_put(my_button.BTN_Down);
    pr_info("good bye reader!\n");
    return 0;
}

static struct platform_driver mypdrv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "gpio_descriptor_sample",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

static int button_open (struct inode *inode, struct file *filp)
{
    pr_info("button open ...");
    return 0;
}

static int button_release (struct inode *inode, struct file *filp)
{
    pr_info("button close ...");
    return 0;
}

static ssize_t button_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    if (canRead)
    {
        if (copy_to_user(buff, kernel_buffer, 1))
        {
            pr_err("ERROR: Not all the bytes have been copied to user\n");
            return -EFAULT;
        }
        else {
            canRead = false;
            return 1;
        }
    }
    return 0;
}

static unsigned int button_poll(struct file *filp, struct poll_table_struct *wait)
{
      __poll_t mask = 0;
        poll_wait(filp, &wait_queue_btn_data, wait);
        pr_info("Poll function, canRead = %d\n", canRead);

        if( canRead )
        {
            mask |= ( POLLIN | POLLRDNORM );
        }
        return mask;
}



static int __init func_init(void)
{
    /* - Dynamically allocate a device number */
	alloc_chrdev_region(&my_button.device_number, 0 , 1, "my_button");
    pr_info("Device number <major> : <minor> = %d:%d\n", MAJOR(my_button.device_number), MINOR(my_button.device_number));

    /* Initialize the cdev structure with fops */
	cdev_init(&my_button.cdev, &file_ops);

    /* Register a device (cdev structure) with VFS */
	my_button.cdev.owner = THIS_MODULE;
	cdev_add(&my_button.cdev, my_button.device_number, 1);

    /* Create device class under sys/class/ */
	my_button.class = class_create (THIS_MODULE, "my_button_class");

	/* Populate the sysfs with the device information */
	my_button.dev = device_create(my_button.class, NULL, my_button.device_number, NULL, "my_buttonf");

    pr_info("my_button init was successful\n");

    memset(kernel_buffer, 0, sizeof(kernel_buffer));

	return platform_driver_register(&mypdrv);

}

static void __exit func_exit(void)
{
    platform_driver_unregister(&mypdrv);

    device_destroy(my_button.class, my_button.device_number);

	class_destroy(my_button.class);

	cdev_del(&my_button.cdev);

	unregister_chrdev_region(my_button.device_number, 1);

    pr_info("exit success");

}
// module_platform_driver(mypdrv);

module_init(func_init);
module_exit(func_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huyhung1001");
MODULE_DESCRIPTION(" Button_description ");
MODULE_INFO(board, "BBB");
