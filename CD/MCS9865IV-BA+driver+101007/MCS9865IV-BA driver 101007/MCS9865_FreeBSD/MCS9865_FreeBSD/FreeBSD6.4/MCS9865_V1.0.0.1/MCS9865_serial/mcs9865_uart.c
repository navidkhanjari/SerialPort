/*-
 * Copyright (c) 2006 Marcel Moolenaar
 * Copyright (c) 2001 M. Warner Losh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	
 The original code is modified for supporting the MCS9865 MOSCHIP serial controller by Ravikanth G.
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/uart/uart_bus_pci.c,v 1.11.2.1.2.1 2009/04/15 03:14:26 kensmith Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/pci/pcivar.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

#include "uart_dev_mcs9865.h"

#define	DEFAULT_RCLK	1843200

static int mcs9865_uart_pci_probe(device_t dev);

static device_method_t mcs9865_uart_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		mcs9865_uart_pci_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_pci_driver = {
	"mcs9865_uart",
	mcs9865_uart_pci_methods,
	sizeof(struct uart_softc),
};

static	void	mcs9865_pci_kludge_unit(device_t dev);
struct pci_id {
	uint16_t	vendor;
	uint16_t	device;
	uint16_t	subven;
	uint16_t	subdev;
	const char	*desc;
	int		rid;
	int		rclk;
};

static struct pci_id pci_mcs9865_uart_ids[] = {
{ 0x9710, 0x9865, 0xa000, 0x1000, "NetMos NM9865 Serial Port", 0x10 },
{ 0xffff, 0, 0xffff, 0, NULL, 0, 0}
};

static struct pci_id *
uart_pci_match(device_t dev, struct pci_id *id)
{
	uint16_t device, subdev, subven, vendor;

	vendor = pci_get_vendor(dev);
	device = pci_get_device(dev);
	while (id->vendor != 0xffff &&
	    (id->vendor != vendor || id->device != device))
		id++;
	if (id->vendor == 0xffff)
		return (NULL);
	if (id->subven == 0xffff)
		return (id);
	subven = pci_get_subvendor(dev);
	subdev = pci_get_subdevice(dev);
	while (id->vendor == vendor && id->device == device &&
	    (id->subven != subven || id->subdev != subdev))
		id++;
	return ((id->vendor == vendor && id->device == device) ? id : NULL);
}

static int
mcs9865_uart_pci_probe(device_t dev)
{
	struct uart_softc *sc;
	struct pci_id *id;

	sc = device_get_softc(dev);

	id = uart_pci_match(dev, pci_mcs9865_uart_ids);
	if (id != NULL) {
		sc->sc_class = &uart_mcs9865_uart_class;
		goto match;
	}
	/* Add checks for non-mcs9865_uart IDs here. */
	return (ENXIO);

 match:
	if (id->desc)
		device_set_desc(dev, id->desc);
	mcs9865_pci_kludge_unit(dev);
	return (uart_bus_probe(dev, 0, id->rclk, id->rid, 0));
}

static void
mcs9865_pci_kludge_unit(dev)
	device_t dev;
{
	devclass_t	dc;
	int		err;
	int		start;
	int		unit;

	unit = 0;
	start = 0;
	while (resource_int_value("sio", unit, "port", &start) == 0 && 
	    start > 0)
		unit++;
	if (device_get_unit(dev) < unit) {
		dc = device_get_devclass(dev);
		while (devclass_get_device(dc, unit))
			unit++;
		//device_printf(dev, "moving to sio%d\n", unit);
		err = device_set_unit(dev, unit);	
		if (err)
			device_printf(dev, "error moving device %d\n", err);
	}
}

DRIVER_MODULE(mcs9865_uart, pci, uart_pci_driver, uart_devclass, 0, 0);

