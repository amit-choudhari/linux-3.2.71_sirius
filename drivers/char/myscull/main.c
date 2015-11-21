#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>        /* copy_*_user */
#include <linux/semaphore.h>

int scull_minor=0;
int scull_major;
int scull_nr_devices=1;

struct mydevice{
 char data[100];
 struct semaphore sem;
};

struct cdev *mcdev; 
struct mydevice device_ptr;
dev_t dev;
int ret;

int device_open(struct inode *inode,struct file *file)
{
	printk(KERN_DEBUG "inside device open");
	//   struct mydevice *dev;
	//   dev=container_of(inode->i_cdev,struct mydevice,cdev);
/*	   if(down_interruptible(&device_ptr.sem))
		return -ERESTARTSYS;
*/	return 0;
}

int device_release(struct inode *inode,struct file *file)
{
	printk("inside device release");
/*	up(&device_ptr.sem);
*/	return 0;

}
 
static ssize_t device_read(struct file* filep, char* bufStoreData,size_t bufCount, loff_t* curOffset){

	printk("inside device read");
	copy_to_user(bufStoreData,device_ptr.data,bufCount);
	return 0;
}

static ssize_t device_write(struct file* filep, char* bufSourceData,size_t bufCount,loff_t* curoffset){

printk("inside device write");
copy_from_user(device_ptr.data,bufSourceData,bufCount);
return 0;
}

struct file_operations scull_fops = {
	.owner=THIS_MODULE,
	.open =device_open,
	.release=device_release,
	.write=device_write,
	.read=device_read,
};

void scull_setup_cdev(void){
	//int err;
	// cdev_init(&device_ptr->cdev,&scull_fops);
	// device_ptr->cdev.owner=THIS_MODULE;
	// device_ptr->cdev.ops=&scull_fops;
	// err=cdev_add(&device_ptr->cdev,devno,1);

}

int myscull_init_module(void){
	printk(KERN_DEBUG "started myscull init");
	ret=alloc_chrdev_region(&dev,scull_minor,1,"myscull");
	if(ret < 0) {
		printk(KERN_ALERT "failed to allocate major number");
		return ret;
	}
	scull_major=MAJOR(dev);
	printk(KERN_DEBUG "major= %d",scull_major);
	//    device_ptr=kmalloc(1*sizeof(struct mydevice), GFP_KERNEL);
	//    memset(device_ptr,0, 1*sizeof(struct mydevice));
        mcdev=cdev_alloc();
        mcdev->ops=&scull_fops;
        mcdev->owner=THIS_MODULE;
        ret=cdev_add(mcdev,dev,1);
        if(ret<0)
        printk(KERN_DEBUG "unable to add cdev");    
//	scull_setup_cdev();
//	sema_init(&device_ptr.sem,1);
//	dev=MKDEV(scull_major,scull_minor);
	printk("end myscull init");
	return 0;
  
}

int myscull_fini_module(void){

cdev_del(mcdev);
unregister_chrdev_region(dev,1);
return 0;
}

module_init(myscull_init_module);
module_exit(myscull_fini_module);
