#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/pci.h>

#define PCI_VENDOR_ID_SRS PCI_VENDOR_ID_REALTEK
#define PCI_PRODUCT_ID_SRS 0x8136

struct priv_data{
	__iomem reg_base;

}


int srs001_probe(struct pci_dev *dev, pci_device_id *device_id)
{
	int retval;
	retval = pci_enable_dev(dev);	
	if(retval<0){
		printk(KERN_INFO "srs001 driver not enabled..Shit happens!");
	}else
		printk(KERN_INFO "srs001 dirver enabled..not bad");

	
}

int srs001_remove(struct pci_dev *dev)
{

}

struct pci_device_id srs001_id[] ={
	{PCI_DEVICE(PCI_VENDOR_ID_SRS, PCI_DEVICE_ID_SRS)},
	{}
}

struct pci_driver srs001_driver = {
	.name = "srs001_driver",
	.id_table = srs001_id,
	.probe = &srs001_probe,
	.remove = &srs001_remove,
};

static void mypci_init(void)
{
	pci_register_driver(&srs001_driver);
}

static void mypci_exit(void)
{
	pci_unregister_driver(&srs001_driver);
}

module_init(mypci_init)
module_exit(mypci_exit)
