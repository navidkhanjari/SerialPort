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
#include <dev/sio/sioreg.h>

#define PUC_ENTRAILS 1

#include "pucvar.h"

#define  COM_FREQ   DEFAULT_RCLK

const struct puc_device_description puc_devices[] = {
	
	/* NetMos 2S1P PCI 16C650 : 2S, 1P */
	{   "NetMos NM9865 Dual UART and 1284 Printer port",
	    {	0x9710,	0x9865,	0xa000,	0x3012	},
	    {	0xffff,	0xffff,	0xffff,	0xffff	},
	    {
		{ PUC_PORT_TYPE_COM, 0x10, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_COM, 0x14, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_LPT, 0x18, 0x00, 0x00 },
	    },
	},

	/* NetMos 4S0P PCI: 4S, 0P */
	{   "NetMos NM9865 Quad UART",
	    {	0x9710,	0x9865,	0xa000,	0x3004	},
	    {	0xffff,	0xffff,	0xffff,	0xffff	},
	    {
		{ PUC_PORT_TYPE_COM, 0x10, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_COM, 0x14, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_COM, 0x18, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_COM, 0x1c, 0x00, COM_FREQ },
	    },
	},

	/* NetMos 3S0P PCI: 3S, 0P */
	{   "NetMos NM9865 Three UART",
	    	{   0x9710, 0x9865,	0xa000,	0x3003	},
		{   0xffff, 0xffff, 	0xffff, 0xffff  },
		{
			{ PUC_PORT_TYPE_COM, 0x10, 0x00, COM_FREQ },
			{ PUC_PORT_TYPE_COM, 0x14, 0x00, COM_FREQ },
			{ PUC_PORT_TYPE_COM, 0x18, 0x00, COM_FREQ },
		}, 
	},

	/* NetMos 0S2P PCI: 0S, 2P */
	{   "NetMos NM9865 1284 Dual Printer port",
	    {   0x9710, 0x9865,	0xa000,	0x3020	},
	    {   0xffff, 0xffff, 0xffff, 0xffff  },
	    {
		{ PUC_PORT_TYPE_LPT, 0x10, 0x00, 0x00 },
		{ PUC_PORT_TYPE_LPT, 0x14, 0x00, 0x00 },
	    },
	},
	/* NetMos 1S1P PCI 16C650 : 1S, 1P */
	{   "NetMos NM9865  UART and 1284 Printer port",
	    {	0x9710,	0x9865,	0xa000,	0x3011	},
	    {	0xffff,	0xffff,	0xffff,	0xffff	},
	    {
		{ PUC_PORT_TYPE_COM, 0x10, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_LPT, 0x14, 0x00, 0x00 },
	    },
	},
	/* NetMos 2S0P PCI 16C650 : 2S, 0P */
	{   "NetMos NM9865 Dual UART",
	    {	0x9710,	0x9865,	0xa000,	0x3002	},
	    {	0xffff,	0xffff,	0xffff,	0xffff	},
	    {
		{ PUC_PORT_TYPE_COM, 0x10, 0x00, COM_FREQ },
		{ PUC_PORT_TYPE_COM, 0x14, 0x00, COM_FREQ },
	    },
	},

	{ 0 }
};


/*
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
};*/
/*
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
			if (desc->subvendor == subvendor &&
		            desc->subdevice == subdevice)
			{

				return (desc);
			}
			if (desc->subvendor == 0xffff)
				return (desc);
		}
		desc++;
	}

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
	device_set_desc(dev,desc->name);
	return(BUS_PROBE_DEFAULT);
}


static int puc_bfe_attach(device_t dev)
{
	const struct puc_cfg *desc;
	desc = mcs9865_isa_puc_pci_match(dev, mcs9865_isa_puc_pci_devices);
	if (desc == NULL)
		return (ENXIO);
	return(puc_attach(dev,desc));

}

*/

static const struct puc_device_description *
puc_find_description(uint32_t vend, uint32_t prod, uint32_t svend, 
    uint32_t sprod)
{
	int i;

#define checkreg(val, index) \
    (((val) & puc_devices[i].rmask[(index)]) == puc_devices[i].rval[(index)])

	for (i = 0; puc_devices[i].name != NULL; i++) {
		if (checkreg(vend, PUC_REG_VEND) &&
		    checkreg(prod, PUC_REG_PROD) &&
		    checkreg(svend, PUC_REG_SVEND) &&
		    checkreg(sprod, PUC_REG_SPROD))
			return (&puc_devices[i]);
	}

#undef checkreg

	return (NULL);
}

static int
puc_pci_probe(device_t dev)
{
	uint32_t v1, v2, d1, d2;
	const struct puc_device_description *desc;

	if ((pci_read_config(dev, PCIR_HDRTYPE, 1) & PCIM_HDRTYPE) != 0)
		return (ENXIO);

	v1 = pci_read_config(dev, PCIR_VENDOR, 2);
	d1 = pci_read_config(dev, PCIR_DEVICE, 2);
	v2 = pci_read_config(dev, PCIR_SUBVEND_0, 2);
	d2 = pci_read_config(dev, PCIR_SUBDEV_0, 2);

	//device_printf(dev,"vendor:%x device:%x svendor:%x sdevice:%x\n",v1,d1,v2,d2);
	desc = puc_find_description(v1, d1, v2, d2);
	if (desc == NULL)
		return (ENXIO);
	//device_printf(dev,"desc->name:%s\n",desc->name);
	device_set_desc(dev, desc->name);
	return (BUS_PROBE_DEFAULT);
}

static int
puc_pci_attach(device_t dev)
{
	uint32_t v1, v2, d1, d2;

	v1 = pci_read_config(dev, PCIR_VENDOR, 2);
	d1 = pci_read_config(dev, PCIR_DEVICE, 2);
	v2 = pci_read_config(dev, PCIR_SUBVEND_0, 2);
	d2 = pci_read_config(dev, PCIR_SUBDEV_0, 2);
	device_printf(dev,"vendor:%x device:%x svendor:%x sdevice:%x\n",v1,d1,v2,d2);
	return (puc_attach(dev, puc_find_description(v1, d1, v2, d2)));
}

/*
static int
puc_pci_detach(device_t dev)
{
	struct puc_bar *bar;
	struct puc_port *port;
	struct puc_softc *sc;
	int error, idx;

	sc = device_get_softc(dev);

	// Detach our children. 
	error = 0;
	for (idx = 0; idx < sc->sc_nports; idx++) {
		port = &sc->sc_port[idx];
		if (port->p_dev == NULL)
			continue;
		if (device_detach(port->p_dev) == 0) {
			device_delete_child(dev, port->p_dev);
			if (port->p_rres != NULL)
				rman_release_resource(port->p_rres);
			if (port->p_ires != NULL)
				rman_release_resource(port->p_ires);
		} else
			error = ENXIO;
	}
	if (error)
		return (error);

	if (sc->sc_serdevs != 0UL)
		bus_teardown_intr(dev, sc->sc_ires, sc->sc_icookie);
	bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irid, sc->sc_ires);

	for (idx = 0; idx < PUC_PCI_BARS; idx++) {
		bar = &sc->sc_bar[idx];
		if (bar->b_res != NULL)
			bus_release_resource(sc->sc_dev, bar->b_type,
			    bar->b_rid, bar->b_res);
	}

	rman_fini(&sc->sc_irq);
	free(__DECONST(void *, sc->sc_irq.rm_descr), M_PUC);
	rman_fini(&sc->sc_iomem);
	free(__DECONST(void *, sc->sc_iomem.rm_descr), M_PUC);
	rman_fini(&sc->sc_ioport);
	free(__DECONST(void *, sc->sc_ioport.rm_descr), M_PUC);
	free(sc->sc_port, M_PUC);
	return (0);

}
*/


static device_method_t puc_pci_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		puc_pci_probe),
    DEVMETHOD(device_attach,		puc_pci_attach),
    //DEVMETHOD(device_detach,		puc_pci_detach),
    //DEVMETHOD(device_detach,		device_detach),

    DEVMETHOD(bus_alloc_resource,	puc_alloc_resource),
    DEVMETHOD(bus_release_resource,	puc_release_resource),
    DEVMETHOD(bus_get_resource,		puc_get_resource),
    DEVMETHOD(bus_read_ivar,		puc_read_ivar),
    DEVMETHOD(bus_setup_intr,		puc_setup_intr),
    DEVMETHOD(bus_teardown_intr,	puc_teardown_intr),
    DEVMETHOD(bus_print_child,		bus_generic_print_child),
    DEVMETHOD(bus_driver_added,		bus_generic_driver_added),
    { 0, 0 }
};

static driver_t mcs9865_isa_puc_pci_driver = {
	"puc",
	puc_pci_methods,
	sizeof(struct puc_softc),
};


DRIVER_MODULE(mcs9865_isa_puc, pci, mcs9865_isa_puc_pci_driver, puc_devclass, 0, 0);
