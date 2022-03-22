
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/uaccess.h>

/* Define these values to match your devices */

#define USB_VENDOR_ID	0x0781
#define USB_PRODUCT_ID	0x5597
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 	0x02
#define BULK_EP_IN 	0x81
#define MAX_PKT_SIZE 	1024

static int Usbpen_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void Usbpen_disconnect(struct usb_interface *interface);
static int Usbpen_open(struct inode *i, struct file *f);
static int Usbpen_close(struct inode *i, struct file *f);
static ssize_t Usbpen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off);
static ssize_t Usbpen_write(struct file *f, const char __user *buf, size_t cnt,loff_t *off);

//provides a list of different types of USB devices that this driver supports
static struct usb_device_id usb_ids[ ] = 
{
	{ USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
	{}
};

//MODULE_DEVICE_TABLE macro is necessary to allow user-space tools to figure out what devices this driver can control.
MODULE_DEVICE_TABLE (usb, usb_ids);

//identifies USB interface driver to usb core
static struct usb_driver Usbpen_driver =
{
	.name 		= "Usbpen_driver",
	.id_table 	= usb_ids,		
	.probe 	= Usbpen_probe,
	.disconnect 	= Usbpen_disconnect,
};

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];
static struct usb_interface *interface_1;

static struct file_operations fops =
{
	.owner 	= THIS_MODULE,
	.open 		= Usbpen_open,
	.release 	= Usbpen_close,
	.read 		= Usbpen_read,
	.write 	= Usbpen_write,
};
/**************************************************************************
function	: Usbpen_open	

description	: 

input param	: 

output param	: 

**************************************************************************/
static int Usbpen_open(struct inode *i, struct file *f)
{
	printk(KERN_ALERT "Inside pen open function\n");
	return 0;
}
/**************************************************************************
function	: Usbpen_close	

description	: 

input param	: 

output param	: 

**************************************************************************/
static int Usbpen_close(struct inode *i, struct file *f)
{
	printk(KERN_ALERT "Inside pen close function\n");
	return 0;
}
/**************************************************************************
function	: Usbpen_read	

description	: 

input param	: 

output param	: 

**************************************************************************/
static ssize_t Usbpen_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
	printk(KERN_ALERT "Inside pen read function\n");
	return 0;
}
/**************************************************************************
function	: Usbpen_write	

description	: 

input param	: 

output param	: 

**************************************************************************/
static ssize_t Usbpen_write(struct file *f, const char __user *buf, size_t cnt,loff_t *off)
{
	int retval;
	int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);

	printk(KERN_ALERT "\n********** WRITE OPERATION , HOST to USB  ********* \n\n");
	
	if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
	{       
	        printk(KERN_ERR "Copy from User space is Unsuccessful\n");
		return -EFAULT;
	}
	

	/* Write the data into the bulk endpoint */
	retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
	
	while (retval < 0)
	{      
		usb_queue_reset_device(interface_1);
		msleep(10);
		retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
		if(retval == 0) break;
	}
	
	printk(KERN_ALERT " Usbpen_write function completed with %d bytes\n",wrote_cnt);

	return wrote_cnt;
}

/**************************************************************************
function	: Usbpen_probe	

description	: 

input param	: 

output param	: 

**************************************************************************/

static int Usbpen_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	
	struct usb_host_interface *iface_desc;
	//struct usb_endpoint_descriptor *endpoint;
	//int i=0;
	int retval;
	
	printk(KERN_ALERT "\n********** PROBE THE USB SYSTEM , USB DEVICE CONNECTED ********* \n\n");
	
	iface_desc = interface->cur_altsetting;

	/*printk(KERN_ALERT "ID->bNumEndpoints: %02X\n", iface_desc->desc.bNumEndpoints);
	printk(KERN_ALERT "ID->bInterfaceClass: %02X\n",	iface_desc->desc.bInterfaceClass);

	for (i = 0; i < iface_desc->desc.bNumEndpoints; i++)
	{
		endpoint = &iface_desc->endpoint[i].desc;

		printk(KERN_ALERT "ED[%d]->bEndpointAddress: 0x%02X\n", i, endpoint->bEndpointAddress);
		printk(KERN_ALERT "ED[%d]->bmAttributes: 0x%02X\n", i, endpoint->bmAttributes);
		printk(KERN_ALERT "ED[%d]->wMaxPacketSize: 0x%04X (%d)\n", i, endpoint->wMaxPacketSize, endpoint->wMaxPacketSize);
	}*/

	device = interface_to_usbdev(interface);
	printk(KERN_ALERT "Pen drive (%X:%X) plugged\n", id->idVendor,id->idProduct);

	class.name = "usb/pen%d";
	class.fops = &fops;
	
	// Save the Instance
	interface_1 = interface;
	
	if ((retval = usb_register_dev(interface, &class)) < 0)
	{
		/* Something prevented us from registering this driver */
		printk(KERN_ERR "Not able to get a minor for this device.");
	}
	else
	{
		// printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
	}
	return retval;
}

/**************************************************************************
function	: Usbpen_disconnect	

description	: 

input param	: 

output param	: 

**************************************************************************/

static void Usbpen_disconnect(struct usb_interface *interface)
{      
        printk(KERN_ALERT "\n********** DISCONNECTED THE USB DEVICE ********* \n\n");      
	printk(KERN_ALERT "Pen  %d now disconnected\n", interface->cur_altsetting->desc.bInterfaceNumber);
	usb_deregister_dev(interface, &class);
}

/**************************************************************************
function	: Usbpen_init	

description	: 

input param	: 

output param	: 

**************************************************************************/

static int Usbpen_init(void)
{
	int result;
	//printk(KERN_ALERT "%s\n", __FUNCTION__);
	printk(KERN_ALERT "\n********** INIT THE USB SYSTEM , USB DRIVER MODULE INSERTED ********* \n\n");
	
	/* register this driver with the USB subsystem */
	result = usb_register(&Usbpen_driver);
	if(result)
	{
		//On faliure usb_register return: negative value
		printk(KERN_ALERT "usb_register failed. Error number %d", result);
		return result;
	} 
	
	else
		return 0; //On success usb_register return: zero
}
/**************************************************************************
function	: Usbpen_exit	

description	: 

input param	: 

output param	: 

**************************************************************************/

static void  Usbpen_exit(void)
{
	printk(KERN_ALERT "\n********** EXIT THE USB SYSTEM , USB DRIVER MODULE REMOVED  ********* \n\n");
	usb_deregister(&Usbpen_driver);
}


// Init the Module 
module_init(Usbpen_init);

// Exit the Module 
module_exit(Usbpen_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Naveen");





