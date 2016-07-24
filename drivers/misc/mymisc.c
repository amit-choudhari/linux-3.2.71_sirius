#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/miscdevice.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define EUDYPTULA_ID_SIZE 13 
#define EUDYPTULA_ID "5024ed70faba"
#define DEVICE_NAME "eudyptula"


dev_t eudy;

ssize_t hello_read(struct file *file, char __user *buff,
		   size_t count, loff_t *off)
{

	pr_debug("inside read\n");
	return simple_read_from_buffer(buff, count, off, EUDYPTULA_ID, strlen(EUDYPTULA_ID));

}

ssize_t hello_write(struct file *file, const char __user *buff,
		    size_t count, loff_t *off)
{
	char tmp[EUDYPTULA_ID_SIZE];
	int result = 0;

	if(count == EUDYPTULA_ID_SIZE){
		result = simple_write_to_buffer(tmp, EUDYPTULA_ID_SIZE-1, off, buff, count) + 1;
		tmp[EUDYPTULA_ID_SIZE - 1] = '\0';
	} else
		result = -EINVAL;

	if((*off) == strlen(EUDYPTULA_ID))
		result = strncmp(tmp, EUDYPTULA_ID, strlen(EUDYPTULA_ID))? -EINVAL:result;

	return result;

}

static const struct file_operations file_oper = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write,
};

struct miscdevice dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &file_oper,
};

static int hello_init(void)
{
	int rc;

	rc = misc_register(&dev);
	if (rc) {
		pr_debug("error while registering\n");
		return -1;
	}
	pr_debug("dev registered minor number %d\n", dev.minor);
	return 0;
}

static void hello_exit(void)
{
	misc_deregister(&dev);
	pr_debug("exiting hello\n");

}

MODULE_LICENSE("GPL");

module_init(hello_init);
module_exit(hello_exit);
