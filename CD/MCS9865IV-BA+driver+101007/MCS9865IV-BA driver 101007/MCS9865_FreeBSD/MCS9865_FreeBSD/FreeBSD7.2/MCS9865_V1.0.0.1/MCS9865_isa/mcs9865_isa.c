/*	$NetBSD: puc.c,v 1.7 2000/07/29 17:43:38 jlam Exp $	*/

/*-
 * Copyright (c) 2002 JF Hay.  All rights reserved.
 * Copyright (c) 2000 M. Warner Losh.  All rights reserved.
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
 *	
 The original code is modified for supporting the MCS9865 MOSCHIP serial controller.
 *
 */

/*-
 * Copyright (c) 1996, 1998, 1999
 *	Christopher G. Demetriou.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou
 *	for the NetBSD Project.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/puc/puc_pci.c,v 1.16.2.1.6.1 2009/04/15 03:14:26 kensmith Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/malloc.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

//#include <dev/puc/puc_cfg.h>
//#include <dev/puc/puc_bfe.h>
#include "mcs9865_isa_cfg.h"
#include "mcs9865_isa_bfe.h"

const struct puc_cfg mcs9865_isa_puc_pci_devices[] = {
    
        {   0x9710, 0x9865, 0xa000,0x3012,
            "NetMos NM9865 Dual UART and 1284 Printer port",
            DEFAULT_RCLK,
            PUC_PORT_2S1P, 0x10, 4, 0,
        },
        {   0x9710, 0x9865, 0xa000,0x3004,
            "NetMos NM9865 Quad UART",
            DEFAULT_RCLK,
            PUC_PORT_4S, 0x10, 4, 0,0
        },
        {   0x9710, 0x9865, 0xa000,0x3003,
            "NetMos NM9865 Three UART",
            DEFAULT_RCLK,
            PUC_PORT_3S, 0x10, 4, 0,
        },
        {   0x9710, 0x9865, 0xa000,0x3002,
            "NetMos NM9865 Dual UART",
            DEFAULT_RCLK,
            PUC_PORT_2S, 0x10, 4, 0,
        },
        /*{   0x9710, 0x9865, 0xa000,0x3001,
            "NetMos NM9865 Single UART",
            DEFAULT_RCLK,
            PUC_PORT_1S, 0x10, 4, 0,
        },*/    /* This above PUC_PORT_1S not supported in PUC driver*/
        {   0x9710, 0x9865, 0xa000,0x3011,
            "NetMos NM9865 Single UART and 1284 Printer port",
            DEFAULT_RCLK,
            PUC_PORT_1S1P, 0x10, 4, 0,
        },
        {   0x9710, 0x9865, 0xa000,0x3020,
            "NetMos NM9865 1284 Dual Printer port",
            DEFAULT_RCLK,
            PUC_PORT_2P, 0x10, 4, 0,
        },
        { 0xffff, 0, 0xffff, 0, NULL, 0 }
};

static const struct puc_cfg *
mcs9865_isa_puc_pci_match(device_t dev, const struct puc_cfg *desc)
{
	uint16_t vendor, device;
	uint16_t subvendor, subdevice;

	vendor = pci_get_vendor(dev);
	device = pci_get_device(dev);
	subvendor = pci_get_subvendor(dev);
	subdevice = pci_get_subdevice(dev);

	while (desc->vendor != 0xffff) {
		if (desc->vendor == vendor && desc->device == device) {
			/* exact match */
			if (desc->subvendor == subvendor &&
		            desc->subdevice == subdevice)
			{

				return (desc);
			}
			/* wildcard match */
			if (desc->subvendor == 0xffff)
				return (desc);
		}
		desc++;
	}

	/* no match */
	return (NULL);
}



static int
mcs9865_isa_puc_pci_probe(device_t dev)
{
	const struct puc_cfg *desc;

	if ((pci_read_config(dev, PCIR_HDRTYPE, 1) & PCIM_HDRTYPE) != 0)
		return (ENXIO);

	desc = mcs9865_isa_puc_pci_match(dev, mcs9865_isa_puc_pci_devices);
	if (desc == NULL)
		return (ENXIO);
	return (puc_bfe_probe(dev, desc));
}

static device_method_t puc_pci_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		mcs9865_isa_puc_pci_probe),
    DEVMETHOD(device_attach,		puc_bfe_attach),
    DEVMETHOD(device_detach,		puc_bfe_detach),

    DEVMETHOD(bus_alloc_resource,	puc_bus_alloc_resource),
    DEVMETHOD(bus_release_resource,	puc_bus_release_resource),
    DEVMETHOD(bus_get_resource,		puc_bus_get_resource),
    DEVMETHOD(bus_read_ivar,		puc_bus_read_ivar),
    DEVMETHOD(bus_setup_intr,		puc_bus_setup_intr),
    DEVMETHOD(bus_teardown_intr,	puc_bus_teardown_intr),
    DEVMETHOD(bus_print_child,		bus_generic_print_child),
    DEVMETHOD(bus_driver_added,		bus_generic_driver_added),
    { 0, 0 }
};

static driver_t mcs9865_isa_puc_pci_driver = {
	puc_driver_name,
	puc_pci_methods,
	sizeof(struct puc_softc),
};


DRIVER_MODULE(mcs9865_isa_puc, pci, mcs9865_isa_puc_pci_driver, puc_devclass, 0, 0);
