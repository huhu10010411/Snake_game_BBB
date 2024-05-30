#include <linux/spi/spi.h>

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/gpio/consumer.h> /* For GPIO Descriptor */
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/uaccess.h>

#define LCD_WIDTH 		84
#define LCD_HEIGHT 		48

#define LCD_CMD			1
#define LCD_DATA		0

#define KERNEL_DRIVER_BUF_SIZE    100

#define LCD_CMD_CLR             "clr"
#define LCD_CMD_SET_CURSOR      "curs"

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__

static const unsigned short ASCII[][5] =
{
	{0x00, 0x00, 0x00, 0x00, 0x00}, // 20
	{0x00, 0x00, 0x5f, 0x00, 0x00}, // 21 !
	{0x00, 0x07, 0x00, 0x07, 0x00}, // 22 "
	{0x14, 0x7f, 0x14, 0x7f, 0x14}, // 23 #
	{0x24, 0x2a, 0x7f, 0x2a, 0x12}, // 24 $
	{0x23, 0x13, 0x08, 0x64, 0x62}, // 25 %
	{0x36, 0x49, 0x55, 0x22, 0x50}, // 26 &
	{0x00, 0x05, 0x03, 0x00, 0x00}, // 27 '
	{0x00, 0x1c, 0x22, 0x41, 0x00}, // 28 (
	{0x00, 0x41, 0x22, 0x1c, 0x00}, // 29 )
	{0x14, 0x08, 0x3e, 0x08, 0x14}, // 2a *
	{0x08, 0x08, 0x3e, 0x08, 0x08}, // 2b +
	{0x00, 0x50, 0x30, 0x00, 0x00}, // 2c ,
	{0x08, 0x08, 0x08, 0x08, 0x08}, // 2d -
	{0x00, 0x60, 0x60, 0x00, 0x00}, // 2e .
	{0x20, 0x10, 0x08, 0x04, 0x02}, // 2f /
	{0x3e, 0x51, 0x49, 0x45, 0x3e}, // 30 0
	{0x00, 0x42, 0x7f, 0x40, 0x00}, // 31 1
	{0x42, 0x61, 0x51, 0x49, 0x46}, // 32 2
	{0x21, 0x41, 0x45, 0x4b, 0x31}, // 33 3
	{0x18, 0x14, 0x12, 0x7f, 0x10}, // 34 4
	{0x27, 0x45, 0x45, 0x45, 0x39}, // 35 5
	{0x3c, 0x4a, 0x49, 0x49, 0x30}, // 36 6
	{0x01, 0x71, 0x09, 0x05, 0x03}, // 37 7
	{0x36, 0x49, 0x49, 0x49, 0x36}, // 38 8
	{0x06, 0x49, 0x49, 0x29, 0x1e}, // 39 9
	{0x00, 0x36, 0x36, 0x00, 0x00}, // 3a :
	{0x00, 0x56, 0x36, 0x00, 0x00}, // 3b ;
	{0x08, 0x14, 0x22, 0x41, 0x00}, // 3c <
	{0x14, 0x14, 0x14, 0x14, 0x14}, // 3d =
	{0x00, 0x41, 0x22, 0x14, 0x08}, // 3e >
	{0x02, 0x01, 0x51, 0x09, 0x06}, // 3f ?
	{0x32, 0x49, 0x79, 0x41, 0x3e}, // 40 @
	{0x7e, 0x11, 0x11, 0x11, 0x7e}, // 41 A
	{0x7f, 0x49, 0x49, 0x49, 0x36}, // 42 B
	{0x3e, 0x41, 0x41, 0x41, 0x22}, // 43 C
	{0x7f, 0x41, 0x41, 0x22, 0x1c}, // 44 D
	{0x7f, 0x49, 0x49, 0x49, 0x41}, // 45 E
	{0x7f, 0x09, 0x09, 0x09, 0x01}, // 46 F
	{0x3e, 0x41, 0x49, 0x49, 0x7a}, // 47 G
	{0x7f, 0x08, 0x08, 0x08, 0x7f}, // 48 H
	{0x00, 0x41, 0x7f, 0x41, 0x00}, // 49 I
	{0x20, 0x40, 0x41, 0x3f, 0x01}, // 4a J
	{0x7f, 0x08, 0x14, 0x22, 0x41}, // 4b K
	{0x7f, 0x40, 0x40, 0x40, 0x40}, // 4c L
	{0x7f, 0x02, 0x0c, 0x02, 0x7f}, // 4d M
	{0x7f, 0x04, 0x08, 0x10, 0x7f}, // 4e N
	{0x3e, 0x41, 0x41, 0x41, 0x3e}, // 4f O
	{0x7f, 0x09, 0x09, 0x09, 0x06}, // 50 P
	{0x3e, 0x41, 0x51, 0x21, 0x5e}, // 51 Q
	{0x7f, 0x09, 0x19, 0x29, 0x46}, // 52 R
	{0x46, 0x49, 0x49, 0x49, 0x31}, // 53 S
	{0x01, 0x01, 0x7f, 0x01, 0x01}, // 54 T
	{0x3f, 0x40, 0x40, 0x40, 0x3f}, // 55 U
	{0x1f, 0x20, 0x40, 0x20, 0x1f}, // 56 V
	{0x3f, 0x40, 0x38, 0x40, 0x3f}, // 57 W
	{0x63, 0x14, 0x08, 0x14, 0x63}, // 58 X
	{0x07, 0x08, 0x70, 0x08, 0x07}, // 59 Y
	{0x61, 0x51, 0x49, 0x45, 0x43}, // 5a Z
	{0x00, 0x7f, 0x41, 0x41, 0x00}, // 5b [
	{0x02, 0x04, 0x08, 0x10, 0x20}, // 5c �
	{0x00, 0x41, 0x41, 0x7f, 0x00}, // 5d ]
	{0x04, 0x02, 0x01, 0x02, 0x04}, // 5e ^
	{0x40, 0x40, 0x40, 0x40, 0x40}, // 5f _
	{0x00, 0x01, 0x02, 0x04, 0x00}, // 60 `
	{0x20, 0x54, 0x54, 0x54, 0x78}, // 61 a
	{0x7f, 0x48, 0x44, 0x44, 0x38}, // 62 b
	{0x38, 0x44, 0x44, 0x44, 0x20}, // 63 c
	{0x38, 0x44, 0x44, 0x48, 0x7f}, // 64 d
	{0x38, 0x54, 0x54, 0x54, 0x18}, // 65 e
	{0x08, 0x7e, 0x09, 0x01, 0x02}, // 66 f
	{0x0c, 0x52, 0x52, 0x52, 0x3e}, // 67 g
	{0x7f, 0x08, 0x04, 0x04, 0x78}, // 68 h
	{0x00, 0x44, 0x7d, 0x40, 0x00}, // 69 i
	{0x20, 0x40, 0x44, 0x3d, 0x00}, // 6a j
	{0x7f, 0x10, 0x28, 0x44, 0x00}, // 6b k
	{0x00, 0x41, 0x7f, 0x40, 0x00}, // 6c l
	{0x7c, 0x04, 0x18, 0x04, 0x78}, // 6d m
	{0x7c, 0x08, 0x04, 0x04, 0x78}, // 6e n
	{0x38, 0x44, 0x44, 0x44, 0x38}, // 6f o
	{0x7c, 0x14, 0x14, 0x14, 0x08}, // 70 p
	{0x08, 0x14, 0x14, 0x18, 0x7c}, // 71 q
	{0x7c, 0x08, 0x04, 0x04, 0x08}, // 72 r
	{0x48, 0x54, 0x54, 0x54, 0x20}, // 73 s
	{0x04, 0x3f, 0x44, 0x40, 0x20}, // 74 t
	{0x3c, 0x40, 0x40, 0x20, 0x7c}, // 75 u
	{0x1c, 0x20, 0x40, 0x20, 0x1c}, // 76 v
	{0x3c, 0x40, 0x30, 0x40, 0x3c}, // 77 w
	{0x44, 0x28, 0x10, 0x28, 0x44}, // 78 x
	{0x0c, 0x50, 0x50, 0x50, 0x3c}, // 79 y
	{0x44, 0x64, 0x54, 0x4c, 0x44}, // 7a z
	{0x00, 0x08, 0x36, 0x41, 0x00}, // 7b {
	{0x00, 0x00, 0x7f, 0x00, 0x00}, // 7c |
	{0x00, 0x41, 0x36, 0x08, 0x00}, // 7d },
	{0x10, 0x08, 0x08, 0x10, 0x08}, // 7e .
	{0x00, 0x06, 0x09, 0x09, 0x06}  // 7f .
};

struct my_device{
    dev_t device_number;
    struct cdev cdev;
    struct class *class;
    struct device *dev;
    struct spi_device *spi_dev;
    struct gpio_desc *dc;               /* CMD : 0 , data: 1 */
    struct gpio_desc *rst;
};

struct my_device my_nokia5110;

unsigned char kernel_buf[KERNEL_DRIVER_BUF_SIZE];


void nokia5110_init(void);
void nokia5110_clear_screen(void);
void nokia5110_send_byte(bool, unsigned char);
void nokia5110_write_char(char);
void nokia5110_write_string(char *);

void nokia5110_init()
{
    gpiod_set_value(my_nokia5110.rst, 0);
    udelay(2);
    gpiod_set_value(my_nokia5110.rst, 1);

	// init LCD
	nokia5110_send_byte(LCD_CMD, 0x21);	// LCD Extended Commands
	nokia5110_send_byte(LCD_CMD, 0xb1);	// Set LCD Cop (Contrast).	//0xb1
	nokia5110_send_byte(LCD_CMD, 0x04);	// Set Temp coefficent.		//0x04
	nokia5110_send_byte(LCD_CMD, 0x14);	// LCD bias mode 1:48. 		//0x13
	nokia5110_send_byte(LCD_CMD, 0x0c);	// LCD in normal mode. 0x0d inverse mode
	nokia5110_send_byte(LCD_CMD, 0x20);
	nokia5110_send_byte(LCD_CMD, 0x0c);

	nokia5110_clear_screen();
}

void nokia5110_send_byte(bool is_cmd, unsigned char data)
{
    unsigned char buf[1];
    buf[0] = data;
	if(is_cmd)
    {
        gpiod_set_value(my_nokia5110.dc, 0);
    }
	else
    {
        gpiod_set_value(my_nokia5110.dc, 1);
    }

	spi_write(my_nokia5110.spi_dev, buf, sizeof(buf));
}

void nokia5110_clear_screen()
{
    int i = 0;
    int stop = LCD_WIDTH * LCD_HEIGHT / 8;
	while(i < stop)
    {
        nokia5110_send_byte(LCD_DATA, 0x00);
        i++;
    }

	nokia5110_send_byte(LCD_CMD, 0x80 | 0); // set x coordinate to 0
	nokia5110_send_byte(LCD_CMD, 0x40 | 0); // set y coordinate to 0
}

void nokia5110_write_char(char data)
{
    int i;
	nokia5110_send_byte(LCD_DATA, 0x00);
	for( i = 0; i < 5; i++)
		nokia5110_send_byte(LCD_DATA, ASCII[data-0x20][i]);
}

void nokia5110_write_string(char *data)
{
	while(*data)
		nokia5110_write_char(*data++);
}

void lcd_goto_XY(uint8_t X, uint8_t Y)
{
	nokia5110_send_byte(LCD_CMD, 0x80 | X*6); // set x
	nokia5110_send_byte(LCD_CMD, 0x40 | Y); // set y
}

/* file operations function */

ssize_t nokia_write (struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    unsigned long n;
    int cursorX, cursorY;
    char foo_buf[10];
    /* Check count <= KERNEL_DRIVER_BUF_SIZE */
    if (count > KERNEL_DRIVER_BUF_SIZE )   return -EINVAL;

    /* Read data from user space save to kernel space */
    n = copy_from_user(kernel_buf, buff, count);
    pr_info("copy ret %ld", n);

    /* Process data */

    /* Check clear lcd command  */
    if (strncmp(kernel_buf, LCD_CMD_CLR, sizeof(LCD_CMD_CLR) - 1) == 0)
    {
        nokia5110_clear_screen();                                       /* Clear lcd */
        pr_info("clear lcd");
    }

    /* Check set lcd cursor command */
    else if (strncmp(kernel_buf, LCD_CMD_SET_CURSOR, sizeof(LCD_CMD_SET_CURSOR) - 1) == 0)
    {
        pr_info("Kernel_buff: %s", kernel_buf);
        /*command format: LCD_CMD_SET_CURSOR x y */
        sscanf(kernel_buf, "%s %d %d", foo_buf, &cursorX, &cursorY);    /* Get cursor position */

        lcd_goto_XY(cursorX, cursorY);                      /* Set cursor */

        pr_info("set cursor (%d, %d)", cursorX, cursorY);
    }
    else
    {
        nokia5110_write_string(kernel_buf);                 /* Print string to lcd */
        pr_info("Write string: %s",kernel_buf);

    }

    /* Clear kernel buffer */
    memset(kernel_buf, 0, KERNEL_DRIVER_BUF_SIZE);

    pr_info(" User app WRITE() system call handler");

	return 0;
}

int nokia_open (struct inode *inode, struct file *filp)
{
    pr_info(" User app OPEN() system call handler");
	return 0;
}

int nokia_release (struct inode *inode, struct file *filp)
{
    pr_info(" User app RELEASE() system call handler");
	return 0;
}

struct file_operations file_ops =
{
    .open = nokia_open,
    .write = nokia_write,
    .release = nokia_release,
    .owner = THIS_MODULE
} ;        /* file operations of the driver */


int nokia5110_probe(struct spi_device *spi)
{
    my_nokia5110.spi_dev = spi;

    struct device *dev;
    dev = &spi->dev;
    my_nokia5110.dc = gpiod_get(dev, "dc", GPIOD_OUT_HIGH);
    my_nokia5110.rst = gpiod_get(dev, "rst", GPIOD_OUT_HIGH);

    nokia5110_init();
    nokia5110_write_string("Helloo");
    pr_info("Probe successful\n");
    return 0;
}

int nokia5110_remove(struct spi_device *spi)
{
    gpiod_put(my_nokia5110.dc);
    gpiod_put(my_nokia5110.rst);
    pr_info("Remove successful\n");
    return 0;
}

struct of_device_id nokia5110_of_match[] = {
	{ .compatible = "nokia5110" },
	{}
};

MODULE_DEVICE_TABLE(of, nokia5110_of_match);

static struct spi_driver my_spi_driver = {
	.probe = nokia5110_probe,
	.remove = nokia5110_remove,
	.driver = {
		.name = "nokia5110",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nokia5110_of_match),
	},
};

static int __init func_init(void)
{
    /* - Dynamically allocate a device number */
	alloc_chrdev_region(&my_nokia5110.device_number, 0 , 1, "nokia5110");
    pr_info("Device number <major> : <minor> = %d:%d\n", MAJOR(my_nokia5110.device_number), MINOR(my_nokia5110.device_number));

    /* Initialize the cdev structure with fops */
	cdev_init(&my_nokia5110.cdev, &file_ops);

    /* Register a device (cdev structure) with VFS */
	my_nokia5110.cdev.owner = THIS_MODULE;
	cdev_add(&my_nokia5110.cdev, my_nokia5110.device_number, 1);

    /* Create device class under sys/class/ */
	my_nokia5110.class = class_create (THIS_MODULE, "nokia5110_class");

	/* Populate the sysfs with the device information */
	my_nokia5110.dev = device_create(my_nokia5110.class, NULL, my_nokia5110.device_number, NULL, "nokia5110f");

    pr_info("Nokia5110 init was successful\n");

	return spi_register_driver(&my_spi_driver);

        // return spi_register_driver(&nokia5110_driver);

}

static void __exit func_exit(void)
{
    spi_unregister_driver(&my_spi_driver);

    device_destroy(my_nokia5110.class, my_nokia5110.device_number);

	class_destroy(my_nokia5110.class);

	cdev_del(&my_nokia5110.cdev);

	unregister_chrdev_region(my_nokia5110.device_number, 1);

    pr_info("exit success");

}

module_init(func_init);
module_exit(func_exit);




/* Module description */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huyhung1001");
MODULE_DESCRIPTION(" Nokia5110_description ");
MODULE_INFO(board, "BBB");


