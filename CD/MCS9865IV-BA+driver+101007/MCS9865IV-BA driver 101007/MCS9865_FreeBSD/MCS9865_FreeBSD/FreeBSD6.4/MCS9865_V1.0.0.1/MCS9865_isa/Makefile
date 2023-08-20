SRCS=mcs9865_isa.c
KMOD=mcs9865_isa
SRCS+= device_if.h bus_if.h pci_if.h ppbus_if.h 

#makeoptions	DEBUG=-g

.include <bsd.kmod.mk>


ld:
	kldload -v ./mcs9865_isa.ko

uld:
	kldunload  mcs9865_isa

lsmod:
	kldstat

fullclean:
	rm -f @ machine

