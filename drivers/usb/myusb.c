#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/usb.h>

#define MIN(a,b) ((a <= b )?(a):(b))
#define BULK_EP_IN 0x02
#define BULK_EP_OUT 0x81

MODULE_LICENSE("GPL");

struct usb_class_driver class;
struct usb_device *device;
char bulk_buff[512];
struct usb_device_id my_id_table[] = {

	{USB_DEVICE(0x8564,0x1000)},
	{}

};

MODULE_DEVICE_TABLE(usb, my_id_table);

int usb_open(struct inode *inode, struct file *file)
{
	return 0;
}

int usb_close(struct inode *inode,struct file *file)
{
	return 0;
}

ssize_t usb_read(struct file *file, char __user *buff, size_t len, loff_t *off)
{
	int rc, read_cnt;

	rc = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN), bulk_buff, 512, &read_cnt,5000);

	printk("inside read");
	copy_to_user(buff, bulk_buff, MIN(len, read_cnt));
	return MIN(len, read_cnt);
}

ssize_t usb_write(struct file *file, char __user *buff, size_t len, loff_t *off)
{
	int wrote_cnt;
	copy_from_user(bulk_buff, buff, MIN(len,512));
	usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buff, MIN(len, 512), &wrote_cnt, 5000);

	return wrote_cnt;
}

static struct file_operations fops={
	.open = usb_open,
	.release = usb_close,
	.read = usb_read,
	.write = usb_write
};

int my_probe(struct usb_interface *usb_if, const struct usb_device_id *id)
{
	int i; 
	device = interface_to_usbdev(usb_if);
	class.name="sirius",
	class.fops=&fops;
	struct usb_interface_descriptor if_desc = usb_if->cur_altsetting->desc;
	struct usb_endpoint_descriptor ep_desc;

	usb_register_dev(usb_if,&class);
	printk("\nminor number %d\n",usb_if->minor);

	printk("current interface number %d \ncount endpoints %d\nclass %d\n subclass %d\n protocol %d\n",if_desc.bInterfaceNumber,if_desc.bNumEndpoints,if_desc.bInterfaceClass,if_desc.bInterfaceSubClass,if_desc.bInterfaceProtocol);
	
	for(i=0; i<if_desc.bNumEndpoints; i++)
	{
		ep_desc=usb_if->cur_altsetting->endpoint[i].desc;
		printk("\nendpoint address %d\npacketsize %d\n",ep_desc.bEndpointAddress,ep_desc.wMaxPacketSize);
	}
	printk("usb inserted inside probe\n");
	return 0;
}

void my_dc(struct usb_interface *usb_if)
{
	printk("usb is removed inside disconnect");
}


struct usb_driver myusb_driver = {
.name = "myusb_drv",
.probe = my_probe,
.disconnect = my_dc,
.id_table = my_id_table

};

static int myusb_init()
{

	return usb_register(&myusb_driver);
}

static void myusb_exit()
{
	usb_deregister(&myusb_driver);
}

module_init(myusb_init);
module_exit(myusb_exit);
