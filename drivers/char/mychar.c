#include <include/linux.h>
#include <include/module.h>


static int __init hello_init (void)
{

	printk("inside hello");
	return 0;
}


static void __exit hello_exit (void)
{
	printk("exit hello");

}

module_init(hello_init);
module_exit(hello_exit);
