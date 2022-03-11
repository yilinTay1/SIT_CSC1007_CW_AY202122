#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/cdev.h>

/*
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80            /* Max length of the message from the device */

/*
 * Global variables are declared as static, so are global within the file.
 */

static int Major;           /* Major number assigned to our device driver */
static int Device_Open = 0; /* Is device open?
                             * Used to prevent multiple access to device */
static char msg[BUF_LEN];   /* The msg the device will give when asked */
static char *msg_Ptr;

static struct class *cls;

static struct file_operations chardev_fops =
    {
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .release = device_release};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &chardev_fops);

    if (Major < 0)
    {
        // pr_alert: printk with KERN_ALERT an alert-level message;
        pr_alert("Registering char device failed with %d\n", Major);
        return Major;
    }

    // pr_info: printk with KERN_INFO loglevel, an info-level message
    pr_info("I was assigned major number %d.\n", Major);
    printk(KERN_INFO "I was assigned major number %d.\n", Major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

    // pr_info: printk with KERN_INFO loglevel, an info-level message
    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    device_destroy(cls, MKDEV(Major, 0));
    class_destroy(cls);

    /*
     * Unregister the device
     */
    unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    if (Device_Open)
        return -EBUSY;

    Device_Open++;

    // sprintf: print message on the screen
    sprintf(msg, "I already told you %d times Hello OS!\n", counter++);
    //    printk(KERN_INFO "I already told you %d times Hello OS!\n", counter++);
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--; /* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
                           char *buffer,      /* buffer to fill with data */
                           size_t length,     /* length of the buffer     */
                           loff_t *offset)
{
    static int counter = 0;
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_read = 0;

    /*
     * If we're at the end of the message,
     * return 0 signifying end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /*
     * Actually put the data into the buffer
     */

    /* Assessed Coursework Requirement: print into the kernel space the message of how many characters send from the kernel space to user space application. */
    printk(KERN_INFO "%d characters send from the kernel space to user space application\n", length);

    while (length && *msg_Ptr)
    {

        /*
         * The buffer is in the user data segment, not the kernel
         * segment so "*" assignment won't work.  We have to use
         * put_user which copies data from the kernel data segment to
         * the user data segment.
         */
        put_user(*(msg_Ptr++), buffer++);

        length--;
        bytes_read++;
    }

    /* Assessed Coursework Requirement: print the message “Device has been read by %d times” into the kernel space. */
    printk(KERN_INFO "Device has been read by %d times\n", counter++);

    /*
     * Most read functions return the number of bytes put into the buffer
     */
    /* Assessed Coursework Requirement: return the received sentences/messages from the device driver to the user space application. */
    return buffer;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data = (struct my_device_data *)file->private_data;
    ssize_t len = min(my_data->size - *offset, size);

    if (len <= 0)
        return 0;

    /* read data from user buffer to my_data->buffer */
    if (copy_from_user(my_data->buffer + *offset, user_buffer, len))
        return -EFAULT;

    *offset += len;
    /* Assessed Coursework Requirement: print into the kernel space the message of how many characters received from the user space application. */
    printk(KERN_INFO "%d characters received from the user space application\n", len);

    /* Assessed Coursework Requirement: print the message “Device has been written by %d times” into the kernel space. */
    printk(KERN_INFO "Device has been written by %d times\n", len);
    return len;
}

MODULE_LICENSE("GPL");                      // The license type
MODULE_DESCRIPTION("Read Char Dev Module"); // The description of the module
MODULE_VERSION("0.1a");                     // The version of the module
