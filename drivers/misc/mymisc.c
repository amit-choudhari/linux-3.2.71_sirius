#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/miscdevice.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

MODULE_LICENSE("GPL");

struct miscdevice dev;
dev_t eudy;
char id[] = "5024ed70faba";

int hello_open(struct inode *inode, struct file *file)
{

	pr_debug("inside open\n");
	return 0;
}

int hello_close(struct inode *inode, struct file *file)
{

	pr_debug("inside close\n");
	return 0;
}

ssize_t hello_read(struct file *file, char __user *buff,
		   size_t count, loff_t *off)
{

	pr_debug("inside read\n");
	count = strlen(id);
	if (*off == 0) {
		if (copy_to_user(buff, id, count) != 0)
			return -EFAULT;

		*off += count;
		return count;
	}
	return 0;

}

ssize_t hello_write(struct file *file, const char __user *buff,
		    size_t count, loff_t *off)
{
	if (!((strncmp(buff, id, strlen(id)) == 0) && (strlen(id) == count))) {
		pr_debug("FAIL");
		return -EINVAL;
	}
	pr_debug("SUCCESS");
	*off += count;
	return count;

}

static const struct file_operations file_oper = {
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_close,
	.read = hello_read,
	.write = hello_write,

};


static int hello_init(void)
{
	int rc;

	dev.minor = MISC_DYNAMIC_MINOR;
	dev.name = "eudyptula";
	dev.fops = &file_oper;

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

module_init(hello_init);
module_exit(hello_exit);
