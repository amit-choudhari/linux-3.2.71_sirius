#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEVICE_NAME "temp_char"
dev_t dev;
struct cdev *my_cdev;

int my_open(struct inode *inode, struct file *fp)
{
	printk("opened my device\n");
	return 0;
}	

int my_release(struct inode *inode, struct file *fp)
{
	printk("close my device\n");
	return 0;
}	

ssize_t my_read(struct file *fp, char __user *buff,size_t count, loff_t *off )
{

	return 0;
}

ssize_t my_write(struct file *fp, char __user *buff,size_t count, loff_t *off )
{

	*off+=count;
	return count;
}

long my_ioctl (struct file *fp, unsigned int cmd, unsigned long arg)
{

	switch(cmd)
	{
	case 1:
		printk(KERN_DEBUG "\nInside ioctl cmd 1");
		break;
	case 2:
		printk(KERN_DEBUG "\nInside ioctl cmd 2");
		break;
	default:
		printk(KERN_DEBUG "\nSomething else");
	}
	return 1;
}

struct file_operations fops = {
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.unlocked_ioctl = my_ioctl,

};
static int __init hello_init (void)
{
	int rc, major, minor;

	printk("inside hello");
	if(!alloc_chrdev_region(&dev,0,1,DEVICE_NAME)){
	goto alloc_Fail;
	}	
	major=MAJOR(dev);
	minor=MINOR(dev);
	
	printk("device major = %d minor = %d",major,minor);
	cdev_add(my_cdev, dev, 1);
	cdev_init(my_cdev,&fops);
	return 0;

alloc_Fail:
	printk("char dev alloc failed");
	return 0;
}


static void __exit hello_exit (void)
{
	printk("exit hello");

}

module_init(hello_init);
module_exit(hello_exit);
