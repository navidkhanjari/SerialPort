KDIR:=/lib/modules/$(shell  uname -r)/build/ 

obj-m +=99xx.o

default:
	$(RM) *.mod.c *.o *.ko .*.cmd *.symvers
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

install:
	mkdir  -p   /lib/modules/$(shell uname -r)/kernel/drivers/serial/
	cp 99xx.ko  /lib/modules/$(shell uname -r)/kernel/drivers/serial/
	depmod -A
	chmod +x mcs99xx
	cp mcs99xx /etc/init.d/
	ln -s /etc/init.d/mcs99xx /etc/rc.d/rc3.d/Smcs99xx || true  	
	ln -s /etc/init.d/mcs99xx /etc/rc.d/rc5.d/Smcs99xx || true
	modprobe 99xx nr_funs=4

uninstall:
	modprobe -r 99xx
	rm /lib/modules/$(shell uname -r)/kernel/drivers/serial/99xx.*
	depmod -A
	rm -f /etc/init.d/mcs99xx
	rm -f /etc/rc.d/rc3.d/Smcs99xx
	rm -f /etc/rc.d/rc5.d/Smcs99xx

clean:
	$(RM) *.mod.c *.o *.ko .*.cmd *.symvers
	rm -rf .tmp_version*
