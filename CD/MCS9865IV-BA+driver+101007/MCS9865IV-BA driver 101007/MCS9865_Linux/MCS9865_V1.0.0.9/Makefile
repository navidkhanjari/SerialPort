KDIR:=/lib/modules/$(shell  uname -r)/build/

obj-m +=mcs9865.o
obj-m +=mcs9865-isa.o

default:
	$(RM) *.mod.c *.o *.ko .*.cmd *.symvers
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
load:
	insmod mcs9865.ko
unload:
	rmmod mcs9865

install:
	cp mcs9865.ko mcs9865-isa.ko /lib/modules/$(shell uname -r)/kernel/drivers/serial/
	depmod -A
	chmod +x mcs9865
	cp mcs9865 /etc/init.d/
	ln -s /etc/init.d/mcs9865 /etc/rc.d/rc3.d/Smcs9865 || true  	
	ln -s /etc/init.d/mcs9865 /etc/rc.d/rc5.d/Smcs9865 || true
	modprobe mcs9865
	modprobe mcs9865-isa	

uninstall:
	modprobe -r mcs9865
	modprobe -r mcs9865-isa
	rm /lib/modules/$(shell uname -r)/kernel/drivers/serial/mcs9865*
	depmod -A
	rm -f /etc/init.d/mcs9865
	rm -f /etc/rc.d/rc3.d/Smcs9865
	rm -f /etc/rc.d/rc5.d/Smcs9865

clean:
	$(RM) *.mod.c *.o *.ko .*.cmd *.symvers *.order *.markers
	$(RM) -r .tmp_versions
