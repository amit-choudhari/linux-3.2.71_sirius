#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define DEVICE_NAME "temp_char"
#define GET_VARIABLE 1
#define SET_VARIABLE 2
#define CLASS_NAME "class_Sirius"
dev_t dev;
struct cdev *my_cdev;
struct class *cl;

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

	return count;
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
	case GET_VARIABLE:
		printk(KERN_DEBUG "\nInside ioctl cmd GET_VARIABLE");
		break;
	case SET_VARIABLE:
		printk(KERN_DEBUG "\nInside ioctl cmd SET_VARIABLE");
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
	rc = alloc_chrdev_region(&dev,0,1,DEVICE_NAME);
	if(!rc)
	goto alloc_Fail;

	major=MAJOR(dev);
	minor=MINOR(dev);

	printk("device major = %d minor = %d",major,minor);
	rc = cdev_add(my_cdev, dev, 1);
	if(!rc)
	goto cdev_add_fail;

	rc = cdev_init(my_cdev,&fops);
	if(!rc)
	goto cdev_init_fail;

	cl = class_create(THIS_MODULE,CLASS_NAME);
	if(!rc)
	goto device_create_fail;

	return 0;

alloc_Fail:
	printk("char dev alloc failed");
	return -EFAULT;

cdev_add_fail:
	printk("cdev add failed");
	return -EFAULT;

device_create_fail:
	printk("device creation failed");
	class_destroy(cl);
	cdev_del(my_cdev);
	unregister_chrdev_region(&dev,1);
	return -EFAULT;
}


static void __exit hello_exit (void)
{
	printk("exit hello");
	class_destroy(cl);
	cdev_del(my_cdev);
	unregister_chardev_region(&dev,1);
}

module_init(hello_init);
module_exit(hello_exit);
