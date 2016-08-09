#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/pci.h>

#define PCI_VENDOR_ID_SRS PCI_VENDOR_ID_REALTEK
#define PCI_DEVICE_ID_SRS 0x8136
/*
struct priv_data{
	__iomem reg_base;
};
*/
void print_config(struct pci_dev *dev)
{
	uint8_t buff8,i;
	uint16_t buff16;
	uint32_t buff32;

	pci_read_config_word(dev,0, &buff16);
	printk(KERN_INFO "VID %x",buff16);
	pci_read_config_word(dev,2, &buff16);
	printk(KERN_INFO "DID %x",buff16);

	for(i=0; i<6; i++)
	{
		printk(KERN_INFO "\nBAR %d: 0x%llX - 0x%llX len- %d bytes Flags- 0x%X - %s",
			i,
			(unsigned long long)pci_resource_start(dev,i),
			(unsigned long long)pci_resource_end(dev,i),
			pci_resource_len(dev,i), pci_resource_flags(dev,i),
			(pci_resource_flags(dev,i) & 0x1)!=0?"IOMEM":"MEM");
	}

}

int srs001_probe(struct pci_dev *dev, struct pci_device_id *device_id)
{
	int retval;
	retval = pci_enable_device(dev);	
	if(retval<0){
		printk(KERN_INFO "srs001 driver not enabled..Shit happens!");
	}else
		printk(KERN_INFO "srs001 dirver enabled..not bad");

	print_config(dev);

	if(!pci_request_regions(dev, "srs001_region"))
		printk(KERN_INFO "got the regions too huh..");
	else
		printk(KERN_ERR "request region Failed..not your fault");

	pci_set_master(dev);

	return retval;
}

int srs001_remove(struct pci_dev *dev)
{

}

struct pci_device_id srs001_id[] ={
	{PCI_DEVICE(PCI_VENDOR_ID_SRS, PCI_DEVICE_ID_SRS)},
	{}
};

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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amit");

module_init(mypci_init)
module_exit(mypci_exit)
