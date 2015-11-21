#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/version.h>
#include<linux/types.h>
#include<linux/kdev_t.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/io.h>
#include<linux/uaccess.h>

MODULE_LICENSE("GPL");

const int MYRAM_BASE=0x100000000, MYRAM_MAX=0x9c10dfff,MYRAM_SIZE=0x00000200;

static dev_t mydev;
const int BASE_MINOR=0;
static int major,minor;
static struct cdev mycdev;
static struct class *cl;
static void __iomem *myioram;

int myopen(struct inode *inode, struct file *fp)
{
	printk("file opened");
	return 0;
}

int myrelease(struct inode *inode, struct file *fp)
{
	printk("file released");
	return 0;
}

ssize_t myread (struct file *fp, char __user *buff, size_t count, loff_t *off)
{
	u8 byte;
	byte=ioread8((u8 *)myioram);
	copy_to_user(buff,&byte,1);	
	printk("\n read byte %d buff %d",byte,*buff);
	*off += sizeof(u8);
	return sizeof(u8);
}

ssize_t mywrite (struct file *fp, const char __user *buff, size_t count, loff_t *off)
{
	u8 byte;
	copy_from_user(&byte,buff,1);
	iowrite8(byte,(u8 *)myioram);
	printk("\n write byte %d buff %d",byte,*buff);
	*off +=count;
	return count;
}

struct file_operations fops = {
.owner=THIS_MODULE,
.open=myopen,
.release= myrelease,
.read= myread,
.write=mywrite
};

static int myinit(void)
{
	if((myioram=ioremap(MYRAM_BASE,MYRAM_SIZE))==NULL){
	printk("ioremap error");
	return -1;
	}

	if(alloc_chrdev_region(&mydev, BASE_MINOR, 1, "myioram")){
	printk("alloc error");
	return -1;
	}
	major=MAJOR(mydev);
	minor=MINOR(mydev);
	printk("\nmajor=%d minor=%d",major,minor);

	if((cl=class_create(THIS_MODULE,"myioclass"))==NULL){
	printk("class error");
	return -1;	
	}

	if(device_create(cl,NULL,mydev,NULL,"myioramdev")==NULL){
	printk("device error");	
	return -1;
	}
	cdev_init(&mycdev,&fops);
	cdev_add(&mycdev,mydev,1);
	return 0;

}

static void myexit(void)
{
	cdev_del(&mycdev);
	device_destroy(cl,mydev);
	class_destroy(cl);
	unregister_chrdev_region(mydev,1);

}

module_init(myinit);
module_exit(myexit);
