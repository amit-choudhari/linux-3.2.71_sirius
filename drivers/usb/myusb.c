#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/usb.h>

MODULE_LICENSE("GPL");

struct usb_device_id my_id_table[] = {
	{USB_DEVICE(0x8564,0x1000)},
	{}

};

MODULE_DEVICE_TABLE(usb, my_id_table);

int my_probe(struct usb_device *usb_dev, const struct usb_device_id *id)
{
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
