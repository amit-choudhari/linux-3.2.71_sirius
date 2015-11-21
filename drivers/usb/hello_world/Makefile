
obj-m   := hello_world.o
CFLAGS_hello_world.o	:= -DDEBUG
KERNELDIR ?=/lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
