#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/pci.h>
#include<linux/types.h>
#include<linux/io.h>

#define EXPT_VENDOR_ID PCI_VENDOR_ID_REALTEK
#define EXPT_PRODUCT_ID 0x8136 /* Fast ethernet card on PCs */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amit");
static struct dev_priv
{
        void __iomem *reg_base;
} _pvt;


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

	printk(KERN_DEBUG "inside probe for realtek driver \nvendor = %d\ndevice = %d\nfunction = %d\n",dev->vendor,dev->device,dev->devfn);	
	printk(KERN_DEBUG "\nsubsystem_vendor = %d\nsubsystem_device = %d\nfunction = %d\n",dev->subsystem_vendor,dev->subsystem_device,dev->devfn);	
	printk("\n-----------Printing Config Space-----------");
	pci_read_config_word(dev, 0, &myword);
	printk("\n VENDOR_ID = %x",myword);
	pci_read_config_word(dev, 2, &myword);
	printk("\n Device_ID = %x", myword);
	pci_read_config_word(dev, 4, &myword);
	printk("\n command register = %x", myword);
	pci_read_config_word(dev, 6, &myword);
	printk("\n status register = %x", myword);
	pci_read_config_byte(dev, 8, &mybyte);
	printk("\n Revesion ID = %x", mybyte);
	pci_read_config_byte(dev, 0x0B, &mybyte);
	printk("\n Base-class = %x", mybyte);
	pci_read_config_byte(dev, 0x0A, &mybyte);
	printk("   Sub-class = %x", mybyte);
	pci_read_config_byte(dev, 9, &mybyte);
	printk("   Prog I/F = %x", mybyte);
	pci_read_config_dword(dev, 0x0C, &mydword);
	printk("\n BIST Header-type Latency-timer Cache-line-size = %lx", mydword);
	pci_read_config_dword(dev, 0x10, &mydword);
	printk("\n Base Address 0 (IOAR) = %lx", mydword);
	pci_read_config_dword(dev, 0x14, &mydword);
	printk("\n Base Address 1 (MEMAR) = %lx", mydword);
	printk("\n Base Address 2-5 (RESERVED)");
	pci_read_config_dword(dev, 0x28, &mydword);
	printk("\n CardBus CIS pointer = %lx", mydword);
	pci_read_config_word(dev, 0x2c, &myword);
	printk("\n Sub VENDOR_ID = %x",myword);
	pci_read_config_word(dev, 0x2E, &myword);
	printk("\n Sub Device_ID = %x", myword);
	pci_read_config_dword(dev, 0x30, &mydword);
	printk("\n Expansion ROM Base Address = %lx", mydword);

}


static int myPciProbe (struct pci_dev *dev,const struct pci_device_id *id)
{
	int rc;
        struct dev_priv *dpv = &_pvt;

	/*  Initialize device before it's used by a driver. Ask low-level code to enable I/O and memory. Wake up the device if it was suspended*/
	rc = pci_enable_device(dev);
	if(rc)
	{
	printk(KERN_ERR "\ndevice enabling FAILED");
	return rc;
	}
	else
	printk(KERN_INFO "\ndevice enabled\n");

	print_config_space(dev);
	
        if(pci_request_region(dev, 0, "mypci_mem") != 0)
	{
	printk(KERN_ERR "pci_request_region failed\n");
	goto ALLOC_REGION_FAIL;
	}
	else
	{
	printk(KERN_INFO "pci_request_region pass\n");
	}

	if((dpv->reg_base = ioremap(pci_resource_start(dev,1), pci_resource_len(dev,1))) == NULL){
        printk("ioremap fail");
	goto IOREMAP_FAIL;	
	}	
	printk(KERN_INFO "Register Base: %p\n", dpv->reg_base);
	printk(KERN_INFO "IRQ: %u\n", dev->irq);
	
	pci_set_drvdata(dev,dpv);
	printk(KERN_INFO "PCI DEVICE REGISTERED. OUT OF PROBE");
	goto DONE;	

IOREMAP_FAIL:

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
