#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>

MODULE_AUTHOR("Amit");
MODULE_DESCRIPTION("hello world");
MODULE_LICENSE("GPL");

static struct usb_device_id usb_tbl[] = {{
	USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
			   USB_INTERFACE_SUBCLASS_BOOT,
			   USB_INTERFACE_PROTOCOL_KEYBOARD)}, {}
};

MODULE_DEVICE_TABLE(usb, usb_tbl);

static int hello_init(void)
{
	pr_debug("hello world");
	return 0;
}

static void hello_exit(void)
{
	pr_debug("Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
