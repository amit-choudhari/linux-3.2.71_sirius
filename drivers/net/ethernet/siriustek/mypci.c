#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/pci.h>
#include<linux/types.h>
#include<linux/io.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/slab.h>
#include<linux/uaccess.h>


#define EXPT_VENDOR_ID PCI_VENDOR_ID_REALTEK
#define EXPT_PRODUCT_ID 0x8136 /* Fast ethernet card on PCs */
#define DEVICE_NAME "siriuspci"
#define CLASS_NAME "sirius_class"
#define PAK_SIZE 256

#define CFG_REG 0x50
#define CFG_UNLOCK 0xc0
#define CFG_LOCK 0x00

#define TX_HIGH_DESC 0x24
#define TX_LOW_DESC 0x20
#define RX_HIGH_DESC 0xe8
#define RX_LOW_DESC 0xe4

enum statusBits{
	DevOwn = 1<<31;
	RingEnd = 1<<30;
	FirstFrag = 1<<29;
	LastFrag =1<<28 ;
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amit");

typedef struct Desc{
	uint32_t opts1;
	uint32_t opts2;
	uint64_t addr;

} Desc;

static struct dev_priv
{
	dev_t mydev;
	struct cdev mycdev;
	struct class *cl;
        void __iomem *reg_base;
	Desc *tx_desc, *rx_desc;
	dma_addr_t tx_phy_desc, rx_phy_desc;
	dma_addr_t tx_phy_pak, rx_phy_pak;
	atomic_t pkt_tx_busy,pkt_tx_avail;
} _pvt;


void setup_buffer(pci_dev *dev)
{
	struct *dev_priv = pci_get_drvdata(dev);
	dev_priv->tx_desc = dma_alloc_coherent(dev, (size_t) sizeof(Desc), &dev_priv->tx_phy_desc, GFP_ATOMIC);
	if(!tx_desc)
	{
		printk(KERN_INFO "\ntx dma_alloc_coherent failed");
	}

	dev_priv->rx_desc = dma_alloc_coherent(dev, (size_t) sizeof(Desc), &dev_priv->rx_phy_desc, GFP_ATOMIC);
	if(!dev_priv->rx_desc)
	{
		printk(KERN_INFO "\n rx dma_alloc_coherent failed");
	}

	dev_priv->tx_pak = kmalloc(PAK_SIZE, GFP_KERNEL | GFP_DMA);
	if(!dev_priv->tx_pak)
	{
		printf("\n unable to kmalloc tx_pak");
	}
	dev_priv->tx_phy_pak = dma_map_single(dev, dev_priv->tx_pak, PAK_SIZE, DMA_TO_DEVICE);

	dev_priv->rx_pak = kmalloc(PAK_SIZE, GFP_KERNEL | GFP_DMA);
	if(!dev_priv->rx_pak)
	{
		printf("\n unable to kmalloc rx_pak");
	}
	dev_priv->rx_phy_pak = dma_map_single(dev, dev_priv->rx_pak, PAK_SIZE, DMA_FROM_DEVICE);

	dev_priv->tx_desc->opts1 = cpu_to_le32(RingEnd | FirstFrag | LastFrag);
	dev_priv->tx_desc->opts2 = 0;
	dev_priv->tx_desc->addr = cpu_to_le64(dev_priv->tx_phy_pak);
	dev_priv->rx_desc->opts1 = cpu_to_le32(DevOwn | RingEnd | PAK_SIZE);
	dev_priv->rx_desc->opts2 = 0;
	dev_priv->rx_desc->addr = cpu_to_le64(dev_priv->rx_phy_pak);

	iowrite8(CFG_UNLOCK, dev_priv->reg_base + CFG_REG);
	iowrite32(dev_priv->tx_phy_desc & DMA_BIT_MASK(32),dev_priv->reg_base + TX_LOW_DESC)
	iowrite32((uint64_t)dev_priv->tx_phy_desc >>32,dev_priv->reg_base + TX_HIGH_DESC)
	iowrite32(dev_priv->tx_phy_desc,dev_priv->reg_base + RX_LOW_DESC)
	iowrite32((uint64_t)dev_priv->rx_phy_desc >>32,dev_priv->reg_base + RX_HIGH_DESC)
	iowrite8(CFG_LOCK, dev_priv->reg_base + CFG_REG);

	atomic_set(&dev_priv->pkt_tx_busy,0);
	atomic_set(&dev_priv->pkt_rx_avail,0);
		
}

ssize_t pci_read(struct file *fp, char __user *buff, size_t count, loff_t *off)
{
	int i,data,to_read;
	char mac[7];
	struct dev_priv *private = (struct dev_priv*) fp->private_data;
	printk(KERN_INFO "\nmac-addr count %d",count);

	to_read = min(count, 7- (size_t)(*off));
	for(i=0; i<6; i++){
		data = ioread8(private->reg_base +i);
		printk(KERN_INFO "%x:",data);
		mac[i]=data;
	}
	mac[i]='\0';
	if(copy_to_user(buff, mac, 7)){
		printk(KERN_INFO "copy to user failed");
	}
	*off += to_read;
	return to_read;
}
/*
ssize_t pci_write(struct file *fp, const char __user *buff, size_t count, loff_t *off)
{
	
	return 0;
}
*/
int pci_open(struct inode *inode, struct file *fp)
{
	fp->private_data = &_pvt;
	printk(KERN_INFO "\ninside open");
	return 0;
}

int pci_close(struct inode *inode, struct file *fp)
{
	printk(KERN_INFO "\ninside close");
	return 0;
}

struct file_operations pci_fops = {
	.open = pci_open,
	.release = pci_close,
	.read = pci_read,
//	.write = pci_write
};

int init_chardev(struct dev_priv *pdata)
{
	int major, minor;
	int ret;
	struct device *dev_ret;
	
	ret = alloc_chrdev_region(&(pdata->mydev), 0, 1, DEVICE_NAME);
	if(ret){
	printk(KERN_ERR "\nalloc_chrdev_region failed");
	goto ALLOC_FAILED;
	}
	major = MAJOR(pdata->mydev);
	minor = MINOR(pdata->mydev);
	printk(KERN_INFO "\nmajor = %d minor = %d", major, minor);

	cdev_init(&(pdata->mycdev), &pci_fops);

	if(cdev_add(&(pdata->mycdev), pdata->mydev, 1)){
	printk(KERN_ERR "\ncdev add Failed");
	goto CDEV_ADD_FAIL;
	}

	if(IS_ERR(pdata->cl = class_create(THIS_MODULE, CLASS_NAME))){
	printk(KERN_ERR "\nclass create failed");
	goto CLASS_CREATE_FAIL;
	}

	if(IS_ERR(dev_ret = device_create(pdata->cl, NULL, pdata->mydev, NULL, "sirius_pci%d",0))){
	printk(KERN_ERR "\ndevice create failed");
	goto DEVICE_CREATE_FAIL;
	}
	
	goto DONE;

DEVICE_CREATE_FAIL:
class_destroy(pdata->cl);
CLASS_CREATE_FAIL:
cdev_del(&(pdata->mycdev));

CDEV_ADD_FAIL:
unregister_chrdev_region(pdata->mydev, 1);

ALLOC_FAILED:
return -EFAULT;

DONE:
return 0;
}

struct pci_device_id mydeviceid[] = {
	{
//	PCI_DEVICE(EXPT_VENDOR_ID, EXPT_PRODUCT_ID)
	.vendor = 0x10ec,
	.device = 0x8136,
	.subvendor = PCI_ANY_ID,
	.subdevice = PCI_ANY_ID,
	},
	{}
};

void print_config_space(struct pci_dev *dev)
{
	uint8_t mybyte;
	uint16_t myword;
	uint32_t mydword;

	printk(KERN_DEBUG "\ninside probe for realtek driver \nvendor = %d\ndevice = %d\nfunction = %d\n",dev->vendor,dev->device,dev->devfn);	
	printk(KERN_DEBUG "\nsubsystem_vendor = %d\nsubsystem_device = %d\nfunction = %d\n",dev->subsystem_vendor,dev->subsystem_device,dev->devfn);	
	printk(KERN_INFO "\n-----------Printing Config Space-----------");
	pci_read_config_word(dev, 0, &myword);
	printk(KERN_INFO "\n VENDOR_ID = %x",myword);
	pci_read_config_word(dev, 2, &myword);
	printk(KERN_INFO "\n Device_ID = %x", myword);
	pci_read_config_word(dev, 4, &myword);
	printk(KERN_INFO "\n command register = %x", myword);
	pci_read_config_word(dev, 6, &myword);
	printk(KERN_INFO "\n status register = %x", myword);
	pci_read_config_byte(dev, 8, &mybyte);
	printk(KERN_INFO "\n Revesion ID = %x", mybyte);
	pci_read_config_byte(dev, 0x0B, &mybyte);
	printk(KERN_INFO "\n Base-class = %x", mybyte);
	pci_read_config_byte(dev, 0x0A, &mybyte);
	printk(KERN_INFO "   Sub-class = %x", mybyte);
	pci_read_config_byte(dev, 9, &mybyte);
	printk(KERN_INFO "   Prog I/F = %x", mybyte);
	pci_read_config_dword(dev, 0x0C, &mydword);
	printk(KERN_INFO "\n BIST Header-type Latency-timer Cache-line-size = %lx", mydword);
	pci_read_config_dword(dev, 0x10, &mydword);
	printk(KERN_INFO "\n Base Address 0 (IOAR) = %lx", mydword);
	pci_read_config_dword(dev, 0x14, &mydword);
	printk(KERN_INFO "\n Base Address 1 (MEMAR) = %lx", mydword);
	pci_read_config_dword(dev, 0x18, &mydword);
	printk(KERN_INFO "\n Base Address 2 (MEMAR) = %lx", mydword);
	printk(KERN_INFO "\n Base Address 3-5 (RESERVED)");
	pci_read_config_dword(dev, 0x28, &mydword);
	printk(KERN_INFO "\n CardBus CIS pointer = %lx", mydword);
	pci_read_config_word(dev, 0x2c, &myword);
	printk(KERN_INFO "\n Sub VENDOR_ID = %x",myword);
	pci_read_config_word(dev, 0x2E, &myword);
	printk(KERN_INFO "\n Sub Device_ID = %x", myword);
	pci_read_config_dword(dev, 0x30, &mydword);
	printk(KERN_INFO "\n Expansion ROM Base Address = %lx", mydword);

}


static int myPciProbe (struct pci_dev *dev,const struct pci_device_id *id)
{
	int rc;
        struct dev_priv *dpv = &_pvt;

	/*  Initialize device before it's used by a driver. Ask low-level code to enable I/O and memory. Wake up the device if it was suspended*/
	rc = pci_enable_device(dev);
	if(rc) {
	printk(KERN_ERR "\ndevice enabling FAILED");
	return rc;
	}
	else
	printk(KERN_INFO "\ndevice enabled\n");

	print_config_space(dev);
	
        if(pci_request_regions(dev, "siriustek_pci_driver")) {
	printk(KERN_ERR "pci_request_region failed\n");
	goto ALLOC_REGION_FAIL;
	}
	else {
	printk(KERN_INFO "pci_request_region pass\n");
	}

	if((dpv->reg_base = ioremap(pci_resource_start(dev,2), pci_resource_len(dev,2))) == NULL){
        printk("ioremap fail");
	goto IOREMAP_FAIL;	
	}
	printk(KERN_INFO "Register Base: %p\n", dpv->reg_base);
	printk(KERN_INFO "IRQ: %u\n", dev->irq);
	
	pci_set_drvdata(dev,dpv);

	printk("IRQ %d\n",dev->irq);

	printk("TESTING STRUCTS pci_device =%p pci_driver=%p device=%p device_driver=%p &(dev->pci_driver->device_driver)=%p",dev,dev->driver,&(dev->dev),dev->dev.driver,&(dev->driver->driver));
	
	if(pci_enable_msi(dev)<0)
	{
		printk("Failed to init MSI");
	}else{
		printk("IRQ with MSI %d\n",dev->irq);
		printk("MSI enabled");
	}
	
	setup_buffer(dev);
	
	init_chardev(dpv);
	printk(KERN_INFO "PCI DEVICE REGISTERED OUT OF PROBE");
	goto DONE;	

IOREMAP_FAIL:
	pci_release_regions(dev);
ALLOC_REGION_FAIL:
	pci_disable_device(dev);
DONE:
	return 0;
}

static void myPciRemove(struct pci_dev *dev)
{
        struct dev_priv *dpv = pci_get_drvdata(dev);

        pci_set_drvdata(dev, NULL);

        iounmap(dpv->reg_base);
        printk(KERN_INFO "PCI device memory unmapped\n");

        pci_release_regions(dev);
        printk(KERN_INFO "PCI device regions");

	pci_disable_device(dev);
}

struct pci_driver mypci_driver = {
	.name = "siriustek_pci_driver",
	.id_table = mydeviceid,
	.probe = myPciProbe,
	.remove = myPciRemove,

};

static int __init mypci_init(void)
{
	/* Register the pci horizontal with PCI core(then register it to pci bus) */
	pci_register_driver(&mypci_driver);
	printk(KERN_DEBUG "PCI driver registered");
	return 0;
}

static void __exit mypci_exit(void)
{
	pci_unregister_driver(&mypci_driver);
}

module_init(mypci_init);
module_exit(mypci_exit);
