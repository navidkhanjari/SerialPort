SRCS=mcs9865_p.c ppc.c ppc_puc.c
KMOD=mcs9865_p
SRCS+= device_if.h bus_if.h pci_if.h usbdevs.h opt_uplcom.h opt_usb.h ppbus_if.h opt_ppc.h isa_if.h

#makeoptions	DEBUG=-g

.include <bsd.kmod.mk>


ld:
	kldload -v ./mcs9865_p.ko

uld:
	kldunload  mcs9865_p

lsmod:
	kldstat

fullclean:
	rm -f @ machine

