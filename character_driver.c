#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>  
#include <linux/cdev.h> 
#include <linux/semaphore.h> 
#include <asm/uaccess.h> 
#include <linux/uaccess.h>
#include <linux/random.h>
 
// Create a structure for our character device driver

struct character_device
{
    char data[100];
    struct semaphore char_sem;
}char_device;
 
// In order to register our device later, we need a cdev object and some other variables
 
struct cdev *mychr;  
int major_num;        //will store our major number-extracted from dev_t
int result;           // Used to store the result values of the functions
 
dev_t char_num;
 
#define DEVICE_NAME  "MyFirstCharDriver"
 
// This is called on device file open
//      inode reference to the file on disk
//      and contains information about that file
//      struct file represents an abstract open file
int char_open(struct inode* pinode, struct file *cfile)
{
    //only allow one process to open this device by using a semaphore as mutual exclusive lock-mutex
    if(down_interruptible(&char_device.char_sem)!=0)
    {
        printk(KERN_ALERT "Saitejasri Character driver: should not lock device during open");
        return -1;
    }
    printk(KERN_INFO "Saitejasri Character driver: opened device");
    return 0;
}

// called when user wants to get information from the device
ssize_t char_read(struct file* cfile, char* buffer,size_t length,loff_t* offset){
    //take data from kernel space to user space
     printk(KERN_INFO "Saitejasri: Reading from character device");
     result = copy_to_user(buffer,char_device.data,length);   //Copy to user space
     return result;
}
 
// called when user wants to send information to the device
ssize_t char_write(struct file* cfile,const char* bufSourceData,size_t length,loff_t* offset)
{
    //write data from user to kernel
  
    printk(KERN_INFO "Saitejasri character device driver: writing to character device");
    result = copy_from_user(char_device.data,bufSourceData, length);
    printk(KERN_INFO "Data written %ld from App to kernel",length);
    return result;
}
 
// called upon user close
int char_close(struct inode *pinode, struct file *cfile)
{
    //When we call up, the opposite of down for semaphore, we release the mutex we obtained at device open.
    up(&char_device.char_sem);         // This will other process to use the deviceup
    printk(KERN_INFO "Saitejasri character device driver: closed device");
    return 0;
}
 
 
// Defining file operations
struct file_operations char_file_operations={
    .owner = THIS_MODULE,   //prevent unloading of this module when operations are in use
    .open = char_open,      // calling the method to open the deivce
    .release = char_close,  //Calling the method to close the device
    .write = char_write,    //Calling the method to write to the device
    .read = char_read       //Calling the method to read from the device
};
 
 
 // Registering the device with system
static int char_drv_init(void)
{
    // Using dynamic allocation to assign the device
    
    result = alloc_chrdev_region(&char_num,0,1,DEVICE_NAME);
    if (result<0) 
    {
        printk(KERN_ALERT "Saitejasri char device driver: failed to allcate a major number");
        return result;
    }
    major_num= MAJOR(char_num); //extracts the major number and store in our variable
    printk(KERN_INFO "Saitejasri character device driver: major number is %d",major_num);
    printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file",DEVICE_NAME,major_num); //dmesg
    mychr=cdev_alloc(); //create our cdev structure and initialize our cdev
    mychr->ops=&char_file_operations; //struct file_operations
    mychr->owner=THIS_MODULE;
   // Adding cdev to the kernel
    
    result = cdev_add(mychr,char_num,1);
    if(result<0) 
    { 
        
        printk(KERN_ALERT "Saitejasri char device driver: unable to add cdev to kernel");   // Checks the error
        return result;
    }
    
    // Initializing our semaphore
    sema_init(&char_device.char_sem,1);  //Providing intial value one
 
    return 0;
}
 
static void char_drv_exit(void)
{
    // unregistering
   
    cdev_del(mychr);
    
    unregister_chrdev_region(char_num,1);
    printk(KERN_ALERT "saitejasri char device driver: unloaded module");
}

//inform the kernel where to start and stop with our driver

module_init(char_drv_init);

module_exit(char_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Saitejasri and Naveen");
