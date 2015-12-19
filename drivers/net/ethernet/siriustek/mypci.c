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
	printk("\n		Printing Config Space");
	pci_read_config_word(dev, 0, &myword);
	printk("\n VENDOR_ID%x",myword);

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
	printk(KERN_INFO "\ndevice enabled");

	print_config_space(dev);

	rc = pci_request_regions(dev, "mypci");
	if(rc)
	{
	printk("\nrequest region FAILED");
	pci_disable_device(dev);
	return rc;
	}
	else
	printk(KERN_INFO "\ndevice region acquired");

        if ((dpv->reg_base = ioremap(pci_resource_start(dev, 2), pci_resource_len(dev, 2))) == NULL)
        {
                printk(KERN_ERR "Unable to map registers of this PCI device\n");
                pci_release_regions(dev);
                pci_disable_device(dev);
                return -ENODEV;
        }
        printk(KERN_INFO "Register Base: %p\n", dpv->reg_base);

        printk(KERN_INFO "IRQ: %u\n", dev->irq);

        pci_set_drvdata(dev, dpv);
	
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
