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
static int Device_Open = 0; /* Used to prevent multiple access to device */
static char msg[BUF_LEN];   /* The msg the device will give when asked */
static char *msg_Ptr;
static char message[256] = {0}; /*Memory for the string that is passed from userspace*/
static short size_of_message;   /*Used to remember the size of the string stored*/
static struct class *cls;

/* Assessed Coursework Requirement: devi_open, devi_read, devi_write, devi_release. */
static struct file_operations chardev_fops = {
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
        // Check for error if failed registering major
        pr_alert("Registering char device failed with %d\n", Major);
        return Major;
    }
    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);
    printk(KERN_NOTICE "Chardev driver created on /dev/%s\n", DEVICE_NAME);
    printk(KERN_NOTICE "Chardev driver is ready to be read/write. \n");
    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */

void cleanup_module(void)
{
    printk(KERN_INFO "Status: %s \n", msg);
    printk(KERN_NOTICE "Chardev driver is closed \n");
    // Remove driver and major from os.
    device_destroy(cls, MKDEV(Major, 0));
    class_destroy(cls);
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
    static int counter = 1;
    if (Device_Open)
        return -EBUSY;
    Device_Open++;
    sprintf(msg, "Chardev driver had been opened %d times\n", counter++);
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */

static int device_release(struct inode *inode, struct file *file)
{
    // Decrement Device_Open to close the device
    Device_Open--;
    module_put(THIS_MODULE);
    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    static int counter = 1;
    int callback = 0;
    /* Assessed Coursework Requirement: print the message “Device has been read by %d times” into the kernel space. */
    printk(KERN_INFO "Chardev driver had been read %d times\n", counter++);
    
    /* Assessed Coursework Requirement: return the received sentences/messages from the device driver to the user space application. */
    callback = copy_to_user(buffer, message, size_of_message);
    memset(message, '\0', 256);
    if (callback == 0){
        printk(KERN_INFO "%d characters send from the kernel space\n", size_of_message);
        return (size_of_message = 0); /* clear the position to the start and return 0 */
    }
    else{
        printk(KERN_INFO "Failed to send %d characters to the user\n", callback);
        return -EFAULT;
    }
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    static int counter = 1;
    /* Assessed Coursework Requirement: print the message “Device has been written by %d times” into the kernel space. */
    printk(KERN_INFO "Chardev driver has been written by %d times\n", counter++);
    sprintf(message, "%s (%d letters)", buff, len); /* appending received string with its length */
    size_of_message = strlen(message);              /*store the length of the stored message */
    /* Assessed Coursework Requirement: print into the kernel space the message of how many characters received from the user space application. */
    printk(KERN_INFO "%d characters received from the user space application\n", len);
    return len;
}

MODULE_LICENSE("GPL");                      // The license type
MODULE_DESCRIPTION("Read Char Dev Module"); // The description of the module
