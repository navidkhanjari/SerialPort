SRCS=mcs9865_uart.c
KMOD=mcs9865_uart
SRCS+= device_if.h bus_if.h pci_if.h uart_if.h

#makeoptions	DEBUG=-g

.include <bsd.kmod.mk>


ld:
	kldload -v ./mcs9865_uart.ko

uld:
	kldunload  mcs9865_uart

lsmod:
	kldstat

fullclean:
	rm -f @ machine
