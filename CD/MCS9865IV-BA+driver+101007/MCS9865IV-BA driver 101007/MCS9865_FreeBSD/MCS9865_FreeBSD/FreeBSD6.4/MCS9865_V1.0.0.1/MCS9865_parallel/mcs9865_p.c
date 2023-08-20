/*-
 * Copyright (c) 2006 Marcel Moolenaar
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Modified for adding support for MOSCHIP MCS9865 parallel. Thanks to maintainers.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/ppc/ppc_pci.c,v 1.1.8.1 2009/04/15 03:14:26 kensmith Exp $");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <vm/vm.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <machine/vmparam.h>
#include <dev/pci/pcivar.h>

#include <dev/ppbus/ppbconf.h>
#include <dev/ppbus/ppb_msq.h>
#include "ppcvar.h"
#include <dev/ppc/ppcreg.h>
#include <sys/rman.h>

#define DEVTOSOFTC(dev)	((struct ppc_data *)device_get_softc(dev))
#include "ppbus_if.h"

int mos_ppc_probe(device_t dev,int rid);
static int mcs9865ppc_pci_probe(device_t dev);

#define BIOS_MAX_PPC	4
#define BIOS_PPC_PORTS	0x408
#define BIOS_PORTS	(short *)(KERNBASE+BIOS_PPC_PORTS)
#define IO_LPTSIZE_EXTENDED	8
#define IO_LPTSIZE_NORMAL	4
#define RF_ACTIVE	0x0002
#define RF_SHAREABLE	0x0004

static device_method_t ppc_pci_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		mcs9865ppc_pci_probe),
	DEVMETHOD(device_attach,	ppc_attach),
	DEVMETHOD(device_detach,	ppc_detach),

	/* bus interface */
	DEVMETHOD(bus_read_ivar,	ppc_read_ivar),
	DEVMETHOD(bus_setup_intr,	ppc_setup_intr),
	DEVMETHOD(bus_teardown_intr,	ppc_teardown_intr),
	DEVMETHOD(bus_alloc_resource,	bus_generic_alloc_resource),

	/* ppbus interface */
	DEVMETHOD(ppbus_io,		ppc_io),
	DEVMETHOD(ppbus_exec_microseq,	ppc_exec_microseq),
	DEVMETHOD(ppbus_reset_epp,	ppc_reset_epp),
	DEVMETHOD(ppbus_setmode,	ppc_setmode),
	DEVMETHOD(ppbus_ecp_sync,	ppc_ecp_sync),
	DEVMETHOD(ppbus_read,		ppc_read),
	DEVMETHOD(ppbus_write,		ppc_write),

	{ 0, 0 }
};

static driver_t mcs9865ppc_pci_driver = {
	"ppc",
	ppc_pci_methods,
	sizeof(struct ppc_data),
};

struct pci_id {
	uint32_t	type;
	const char	*desc;
	int		rid;
};

static struct pci_id pci_ids[] = {
	{ 0x98659710, "NetMos NM9865 1284 Printer port", 0x10 },
	{ 0xffff }
};

static int
mcs9865ppc_pci_probe(device_t dev)
{
	struct pci_id *id;
	uint32_t type;

	type = pci_get_devid(dev);


/*Code added for checking */
        uint16_t vendor, device;
        uint16_t subvendor, subdevice;

        vendor = pci_get_vendor(dev);
        device = pci_get_device(dev);
        subvendor = pci_get_subvendor(dev);
        subdevice = pci_get_subdevice(dev);

        if((vendor == 0x9710) &&(device==0x9865))
        {
                /* matching 9865 device*/

        //      printf("****************9865 found*********\n");
                //device_printf(dev,"vendor %x\n",vendor);
                //device_printf(dev,"device %x \n",device);
                //device_printf(dev,"subvendor %x\n",subvendor);
                //device_printf(dev,"subdevice %x\n",subdevice);

                if(subdevice == 0x1000)
                {
              //device_printf(dev," ***** ppc_pci.c  this is serial device returning Null***\n");
                return (ENXIO);
                }
        }

	
	id = pci_ids;
	while (id->type != 0xffff && id->type != type)
		id++;
	if (id->type == 0xffff)
		return (ENXIO);
	device_set_desc(dev, id->desc);
	return (ppc_probe(dev, id->rid));
	//return (ppc_probe(dev));
}
DRIVER_MODULE(mcs9865_ppc, pci, mcs9865ppc_pci_driver, ppc_devclass, 0, 0);

