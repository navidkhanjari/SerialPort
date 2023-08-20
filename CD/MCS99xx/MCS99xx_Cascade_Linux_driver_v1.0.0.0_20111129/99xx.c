/*
 *  linux/drivers/serial/99xx.c
 *
 *  Based on drivers/serial/8250.c by Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code is modified to support moschip 99xx series serial devices by ravikanthg@moschip.com
 */

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,15)
#include <linux/config.h>
#endif

#if defined(CONFIG_SERIAL_99xx_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/mca.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/nmi.h>
#include <linux/bitops.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include "99xx.h"

#define NORMAL_MODE 1
#define CASCADE_MODE 2
#define UART99xx_NR  16

//All transactions are with memory mapped registers
#define MEM_AXS 1

/*
 * Definitions for PCI support.
 */
#define FL_BASE_MASK		0x0007
#define FL_BASE0			0x0000
#define FL_BASE1			0x0001
#define FL_BASE2			0x0002
#define FL_BASE3			0x0003
#define FL_BASE4			0x0004
#define FL_GET_BASE(x)		(x & FL_BASE_MASK)

#if 0
#define DEBUG(fmt...)	DEBUG(fmt)
#else
#define DEBUG(fmt...)	do { } while (0)
#endif
//choose mode either as NORMAL_MODE or CASCADE_MODE
int mode = CASCADE_MODE;//NORMAL_MODE  


struct uart_99xx_port {
	struct uart_port	port;
	spinlock_t		lock_99xx;			//Per port lock
	int			tx_dma;				//Variable to serialise the start_tx calls in dma mode
	unsigned int 		dma_tx;				//TX DMA enable or not
	unsigned int 		dma_rx;				//RX DMA enable or not
	u8			ier; 				//Interrupt Enable Register
	u8 			lcr;				//Line Control Register
	u8			mcr;				//Modem Control Register 
	u8 			acr;				//Additional Control Register
	unsigned int		capabilities;			//port capabilities
	int			rxfifotrigger;		
	int 			txfifotrigger;
	u32			dma_tx_cnt;			//Amount of data to be DMA in TX
	u32			dma_rx_cnt;			//Amount of data to be DMA in RX
	char 	*		dma_tx_buf_v;			//Virtual Address of DMA Buffer for TX
	dma_addr_t 		dma_tx_buf_p;			//Physical Address of DMA Buffer for TX
	char	*		dma_rx_buf_v;			//Virtual Address of DMA Buffer for RX
	dma_addr_t		dma_rx_buf_p;			//Physical Address of DMA Buffer for TX
	u32			part_done_recv_cnt;		//RX DMA CIRC buffer Read index
	int 			rx_dma_done_cnt;	
	int			uart_mode;			//SERIAL TYPE
	int			flow_control;			//Flow control is enabled or not
	int			flow_ctrl_type;			//Type of Flow control
	u8 			x_on;				//X-ON Character
	u8 			x_off;				//X-OFF Character
	u32 			ser_dcr_din_reg;		//Device control register
	u32 			ser_ven_reg;			//Vendor register
	struct uart_99xx_port   *next_port;
	struct uart_99xx_port   *prev_port; 
};

static struct uart_99xx_port serial99xx_ports[UART99xx_NR];

static int test_mode=0;// Make it default to 0 after testing
static int nr_funs = 4;	

struct uart_99xx_contxt{
	int rx_dma_en;		
				//0-I/O mode of RX
				//1 -DMA mode of RX
	int tx_dma_en;		
				//0-I/O mode of TX 
				//1 -DMA mode of TX
	int  uart_mode;		
				//MCS99XX_RS232_MODE
				//MCS99XX_RS422_MODE
				//MCS99XX_RS485_HALF_DUPLEX
				//MCS99XX_RS485_FULL_DUPLEX
	int en_flow_control;  
				//0 - No H/W Flow Control	 
				//1 - H/W Flow Control
	int  flow_ctrl_type;	
				//MCS99XX_DTR_DSR_HW_FLOWCONTROL
				//MCS99XX_XON_XOFF_HW_FLOWCONTROL
				//MCS99XX_RTS_CTS_HW_FLOWCONTROL
	int rxfifotrigger;	//0-127
	int txfifotrigger;	//0-127
	int x_on;
	int x_off;
};

static struct uart_99xx_contxt uart_99xx_contxts[] = {
	//Port 0
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,//MCS99XX_RTS_CTS_HW_FLOWCONTROL,
		.rxfifotrigger	= 64,
		.txfifotrigger	= 64,		
		.x_on	= SERIAL_DEF_XON,
		.x_off	= SERIAL_DEF_XOFF,
	},
	//Port 1
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,//MCS99XX_RTS_CTS_HW_FLOWCONTROL,
		.rxfifotrigger  = 64,
		.txfifotrigger  = 64,		
		.x_on=  SERIAL_DEF_XON,
		.x_off= SERIAL_DEF_XOFF,
	},
	//Port 2
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,//MCS99XX_RTS_CTS_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 3
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,//MCS99XX_RTS_CTS_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 4
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,//MCS99XX_RTS_CTS_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 5
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_DTR_DSR_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 6
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 7
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 8
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 9
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 10
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 11
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 12
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 13
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 14
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
	//Port 15
	{
		.rx_dma_en	= 0,
		.tx_dma_en	= 0,
		.uart_mode	= MCS99XX_RS232_MODE,
		.en_flow_control= 0,
		.flow_ctrl_type = MCS99XX_XON_XOFF_HW_FLOWCONTROL,
		.rxfifotrigger=64,
		.txfifotrigger=64,		
		.x_on=SERIAL_DEF_XON,
		.x_off=SERIAL_DEF_XOFF,
	},
};

/*
 * Here we define the default xmit fifo size used for each type of UART.
 */
static const struct serial99xx_config uart_config[] = {
	[PORT_UNKNOWN] = {
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16450] = {
		.fifo_size	= 1,
		.tx_loadsz	= 1,
	},
	[PORT_16550] = {
		.fifo_size	= 16,
		.tx_loadsz	= 14,
	},
	[PORT_16550A] = {
		.fifo_size	= 256,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_01,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_16650] = {
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16750] = {
		.fifo_size	= 64,
		.tx_loadsz	= 64,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_10 |UART_FCR7_64BYTE,
		.flags		= UART_CAP_FIFO | UART_CAP_SLEEP | UART_CAP_AFE,
	},
	[PORT_16850] = {
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO,
		.flags		= UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP,
	},
	[PORT_16C950] = {
		.fifo_size	= 128,
		.tx_loadsz	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO,
		.flags		= UART_CAP_FIFO,
	},
	[PORT_ENHANC]= {
		.fifo_size	= 256,
		.tx_loadsz 	= 128,
		.fcr		= UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_R_TRIG_01,
		.flags		= UART_CAP_FIFO,
	},
};


//helper function for IO type read
static _INLINE_ u8 serial_in(struct uart_99xx_port *up, int offset)
{
#if MEM_AXS
        u8 tmp1;
        tmp1=readl(up->port.membase+0x280+(offset*4));
        return tmp1;
#else
	return inb(up->port.iobase + offset);
#endif
}

//helper function for IO type write
static _INLINE_ void serial_out(struct uart_99xx_port *up, int offset, int value)
{
#if MEM_AXS
        writel(value, up->port.membase+0x280+(offset*4));
#else
	outb(value, up->port.iobase + offset);
#endif
}

//Helper function to write to index control register
static void serial_icr_write(struct uart_99xx_port *up, int offset, int value)
{
	DEBUG("UART_LCR=0x%x\n",serial_in(up,UART_LCR));
        serial_out(up, UART_SCR, offset);
        serial_out(up, UART_ICR, value);
}

//Helper function to read from index control register
static unsigned int serial_icr_read(struct uart_99xx_port *up, int offset)
{
	unsigned int value;
	// DEBUG("iobase is 0x%x\n",up->port.iobase);
	serial_icr_write(up, UART_ACR, up->acr | UART_ACR_ICRRD);
	serial_out(up, UART_SCR, offset);
	value = inb(up->port.iobase+UART_ICR);
	serial_icr_write(up, UART_ACR, up->acr);
	return value;
}

//Helper function to set the 950 mode
void setserial_ENHANC_mode(struct uart_99xx_port *up)
{
	u8 lcr,efr;
	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);
//	DEBUG("BEFORE LCR\n");
	lcr=serial_in(up,UART_LCR);
	serial_out(up, UART_LCR, 0xBF);
	efr=serial_in(up,UART_EFR);
	efr |= UART_EFR_ECB;
	serial_out(up, UART_EFR,efr);
	serial_out(up, UART_LCR, lcr);	

	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
}

// Helper function to clear the FIFO
static inline void serial99xx_clear_fifos(struct uart_99xx_port *p)
{
	DEBUG("serial99xx_clear_fifos\n");
	DEBUG("serial port no is %d\n",p->port.line);
	if (p->capabilities & UART_CAP_FIFO) {
		serial_out(p, UART_FCR, UART_FCR_ENABLE_FIFO);
		serial_out(p, UART_FCR, UART_FCR_ENABLE_FIFO |
			       UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
		serial_out(p, UART_FCR, 0);
	}
	DEBUG("END OF serial99xx_clear_fifos\n");
}

//Helper function to set the the UART to sleep mode
static inline void serial99xx_set_sleep(struct uart_99xx_port *p, int sleep)
{
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	if (p->capabilities & UART_CAP_SLEEP) {
		if (p->capabilities & UART_CAP_EFR) {
			serial_out(p, UART_LCR, 0xBF);
			serial_out(p, UART_EFR, UART_EFR_ECB);
			serial_out(p, UART_LCR, 0);
		}
		serial_out(p, UART_IER, sleep ? UART_IERX_SLEEP : 0);
		if (p->capabilities & UART_CAP_EFR) {
			serial_out(p, UART_LCR, 0xBF);
			serial_out(p, UART_EFR, 0);
			serial_out(p, UART_LCR, 0);
		}
	}
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//Member function of the port operations to stop the data transfer
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
static void serial99xx_stop_tx(struct uart_port *port, unsigned int tty_stop)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
static void serial99xx_stop_tx(struct uart_port *port)
#endif
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	DEBUG("port->line is %d\n",port->line);
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("device structure id for port%d is %x\n",up->port.line,up);
	if(up->dma_tx){
		up->tx_dma=0;
	}else{	//IO mode
		if (up->ier & UART_IER_THRI) {
			up->ier &= ~UART_IER_THRI;
			serial_out(up, UART_IER, up->ier);
		}
	}

	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//Member function of the port operations to start the data transfer
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
static void serial99xx_start_tx(struct uart_port *port, unsigned int tty_start)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
static void serial99xx_start_tx(struct uart_port *port)
#endif
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	struct circ_buf *xmit = &up->port.info->xmit;
	u32	length=0,len2end;
	int tail,head;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("device structure id for port%d is %x\n",port->line,up);

	if(up->dma_tx && (up->tx_dma == 0)){
		DEBUG(" I WAS IN DMA OF START_TX\n");

		//CALCULATING THE AMOUNT OF DATA AVAILABLE FOR THE NEXT TRANSFER
        //AND COPYING THE DATA TO THE DMA BUFFER
        length = uart_circ_chars_pending(xmit);
        head=xmit->head;
		tail=xmit->tail;
		len2end = CIRC_CNT_TO_END(head, tail, UART_XMIT_SIZE);
        DEBUG("In %s -------------------xmit->tail=%d, xmit->head=%d,length=%d,length2end=%d\n",__FUNCTION__,tail,head,length,len2end);
        if(tail < head){
			if(length <= DMA_TX_BUFFER_SZ){
				DEBUG("In %s normal circ buffer\n",__FUNCTION__);
				memcpy(up->dma_tx_buf_v,&xmit->buf[tail],length);  //xmit->buf + xmit->tail
				up->dma_tx_cnt = length;
			}else{
				memcpy(up->dma_tx_buf_v,&xmit->buf[tail],DMA_TX_BUFFER_SZ);
				up->dma_tx_cnt = DMA_TX_BUFFER_SZ;
			}
		}else{
			if(length <= DMA_TX_BUFFER_SZ){
				DEBUG("In %s 2 mode circ buffer\n",__FUNCTION__);
				memcpy(up->dma_tx_buf_v, &xmit->buf[tail], len2end);
				memcpy(up->dma_tx_buf_v+len2end, xmit->buf, head);
				up->dma_tx_cnt = length;
			}else{
				if(len2end <= DMA_TX_BUFFER_SZ){
					memcpy(up->dma_tx_buf_v,&xmit->buf[tail],len2end);
					memcpy(up->dma_tx_buf_v+len2end, xmit->buf, DMA_TX_BUFFER_SZ-len2end);
					up->dma_tx_cnt = len2end;
				}else{
					memcpy(up->dma_tx_buf_v,&xmit->buf[tail],DMA_TX_BUFFER_SZ);
					up->dma_tx_cnt = DMA_TX_BUFFER_SZ;
				}
			}
		}

		up->tx_dma++;	//variable to serialise the DMA tx calls
		DEBUG("In %s up->dma_tx_cnt=%d\n",__FUNCTION__,up->dma_tx_cnt);
		DEBUG("IN %s length=%d and data in dma->buf is: %s\n",__FUNCTION__,length,up->dma_tx_buf_v);

		spin_lock(&up->lock_99xx);
		writel(up->dma_tx_buf_p,up->port.membase + REG_TX_DMA_START_ADDRESS_LOW);
		writel(0,up->port.membase + REG_TX_DMA_START_ADDRESS_HIGH);
		writel(up->dma_tx_cnt,up->port.membase+REG_TX_DMA_LENGTH);
		writel(TX_DMA_START_BIT, up->port.membase + REG_TX_DMA_START);
		spin_unlock(&up->lock_99xx);
	
		DEBUG("In %s programmed registers\n",__FUNCTION__);
	}else{
		if (!(up->ier & UART_IER_THRI)) {
			up->ier |= UART_IER_THRI;
			serial_out(up, UART_IER, up->ier);
		}
	}
	 
 	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//Member function of the port operations to stop receiving the data
static void serial99xx_stop_rx(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	u32	value=0;
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("device structure id for port%d is %x\n",up->port.line,up);
	if(up->dma_rx){
		value |= RX_DMA_STOP_BIT;
		writel(value, up->port.membase + REG_RX_DMA_STOP);		
	}else{
		up->ier &= ~UART_IER_RLSI;
		up->port.read_status_mask &= ~UART_LSR_DR;
		serial_out(up, UART_IER, up->ier);
	}	
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}


//Member function of the port operations to enable modem status change interrupt
static void serial99xx_enable_ms(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];

	DEBUG("In %s --------------------------------------- START\n",__FUNCTION__);
	up->ier |= UART_IER_MSI;
	serial_out(up, UART_IER, up->ier);
}

//Function to check modem statuss
static _INLINE_ void check_modem_status(struct uart_99xx_port *up)
{
	u8 status;	

	DEBUG("In %s -------------------- START\n",__FUNCTION__);
	status = serial_in(up, UART_MSR);

	if ((status & UART_MSR_ANY_DELTA) == 0)
		return;

	if (status & UART_MSR_TERI)
		up->port.icount.rng++;
	if (status & UART_MSR_DDSR)
		up->port.icount.dsr++;
	if (status & UART_MSR_DDCD)
		uart_handle_dcd_change(&up->port, status & UART_MSR_DCD);
	if (status & UART_MSR_DCTS)
		uart_handle_cts_change(&up->port, status & UART_MSR_CTS);

	wake_up_interruptible(&up->port.info->delta_msr_wait);
	DEBUG("In %s -------------------- END\n",__FUNCTION__);
}

//Helper function used in ISR to receive the the charecters from the UART
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
static _INLINE_ void receive_chars(struct uart_99xx_port *up, u8 *status)
#else
static _INLINE_ void receive_chars(struct uart_99xx_port *up, u8 *status, struct pt_regs *regs)
#endif
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
	struct tty_struct *tty = up->port.info->tty;
#else
        struct tty_struct *tty = up->port.info->port.tty;
#endif

	u8 ch,lsr = *status;
	int max_count = 256;
	unsigned int flag;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	do {
		/* The following is not allowed by the tty layer and
		   unsafe. It should be fixed ASAP */
		#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
		if (unlikely(tty->flip.count >= TTY_FLIPBUF_SIZE)) {
			if (tty->low_latency) {
				spin_unlock(&up->port.lock);
				tty_flip_buffer_push(tty);
				spin_lock(&up->port.lock);
			}
			/*
			 * If this failed then we will throw away the
			 * bytes but must do so to clear interrupts
			 */
		}
		#endif
		ch = serial_in(up, UART_RX);
		flag = TTY_NORMAL;
		up->port.icount.rx++;

		if (unlikely(lsr & (UART_LSR_BI | UART_LSR_PE |UART_LSR_FE | UART_LSR_OE))) {
			/*
			 * For statistics only
			 */
			if (lsr & UART_LSR_BI) {
				lsr &= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port))
					goto ignore_char;
			} else if (lsr & UART_LSR_PE)
				up->port.icount.parity++;
			else if (lsr & UART_LSR_FE)
				up->port.icount.frame++;
			if (lsr & UART_LSR_OE)
				up->port.icount.overrun++;

			/*
			 * Mask off conditions which should be ignored.
			 */
			lsr &= up->port.read_status_mask;

			if (lsr & UART_LSR_BI) {
				DEBUG("handling break....");
				flag = TTY_BREAK;
			}else if (lsr & UART_LSR_PE)
				flag = TTY_PARITY;
			else if (lsr & UART_LSR_FE)
				flag = TTY_FRAME;
		}
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
			if (uart_handle_sysrq_char(&up->port, ch, regs))
				goto ignore_char;
		#else
			if (uart_handle_sysrq_char(&up->port, ch))
				goto ignore_char;
		#endif

		uart_insert_char(&up->port, lsr, UART_LSR_OE, ch, flag);

ignore_char:

		lsr = serial_in(up, UART_LSR);
	} while ((lsr & UART_LSR_DR) && (max_count-- > 0));
	spin_unlock(&up->port.lock);
	tty_flip_buffer_push(tty);
	spin_lock(&up->port.lock);
	*status = lsr;
	DEBUG("In %s -------------------------------------END\n",__FUNCTION__);
}


//Helper function used in ISR to send the data to the UART
static _INLINE_ void transmit_chars(struct uart_99xx_port *up)
{
	struct circ_buf *xmit = &up->port.info->xmit;
	int count;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("up is %x\n",up);
	if (up->port.x_char) {
		serial_out(up, UART_TX, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char = 0;
		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&up->port)) {
		DEBUG("circ buffer empty\n");
		#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
		serial99xx_stop_tx(&up->port, 0);
		#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
		serial99xx_stop_tx(&up->port);
		#endif		
		return;
	}

	count = uart_config[up->port.type].tx_loadsz;
	DEBUG("In %s-----------up->port.type=%d,tx_loadsz=%d\n",__FUNCTION__,up->port.type,count);
	do {
		serial_out(up, UART_TX, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		up->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);

	if (uart_circ_empty(xmit)){
		DEBUG("circ buffer empty\n");
		#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
		serial99xx_stop_tx(&up->port, 0);
		#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
		serial99xx_stop_tx(&up->port);
		#endif
	}
	DEBUG("In %s --------------------------------------2END\n",__FUNCTION__);
}

//Helper function to stop the characters transmission in DMA mode
static void transmit_chars_dma_stop_done(struct uart_99xx_port * up)
{
		struct circ_buf *xmit = &up->port.info->xmit;
		long int transferred;
		DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
		//UPDATING THE TRANSMIT FIFO WITH THE AMOUNT OF DATA TRANSFERRED
		transferred=readl(up->port.membase+REG_TX_BYTES_TRANSFERRED);
		xmit->tail=((xmit->tail)+transferred) & (UART_XMIT_SIZE-1);
		up->port.icount.tx += transferred;
		up->tx_dma=0;
		
		memset(up->dma_tx_buf_v,0,DMA_TX_BUFFER_SZ);
		DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//Helper function to do the necessary action upon the successful completion of data transfer in DMA mode
static int transmit_chars_dma_done(struct uart_99xx_port * up)
{
	struct circ_buf *xmit = &(up->port.info->xmit);
	int length,tobe_transferred,transferred,len2end;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	
	//UPDATING THE xmit FIFO WITH THE AMOUNT OF DATA TRANSFERRED
	tobe_transferred=readl(up->port.membase+REG_TX_BYTES_TRANSFERRED);
	transferred = (up->dma_tx_cnt - tobe_transferred);
	DEBUG("In %s -------------transferred=%d--------------------------START\n",__FUNCTION__,transferred);
	xmit->tail = ((xmit->tail) + transferred) & (UART_XMIT_SIZE-1);
	
	up->port.icount.tx += transferred;			
	length = uart_circ_chars_pending(xmit); 
	DEBUG("In %s circ_buf lenght=%d after\n",__FUNCTION__,length); 
	memset(up->dma_tx_buf_v,0,DMA_TX_BUFFER_SZ);
	
	DEBUG("In %s up->dma_tx_buf_v=0x%x ---------------------------------------START\n",__FUNCTION__,(unsigned int)up->dma_tx_buf_v);
		
	if (uart_circ_empty(xmit) ||uart_tx_stopped(&up->port)){
		up->tx_dma=0;	
		if (length < WAKEUP_CHARS)
			uart_write_wakeup(&up->port);
		return 0;		
	}

	//CALCULATING THE AMOUNT OF DATA AVAILABLE FOR THE NEXT TRANSFER 
	//AND COPYING THE DATA TO THE DMA BUFFER
	
	length = uart_circ_chars_pending(xmit);
	len2end = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE); 
	DEBUG("In %s -------------------xmit->tail=%d, xmit->head=%d,length=%d,length2end=%d\n",__FUNCTION__,xmit->tail,xmit->head,length,len2end);

	if(xmit->tail < xmit->head){	
		if(length <= DMA_TX_BUFFER_SZ){
			memcpy(up->dma_tx_buf_v,&xmit->buf[xmit->tail],length);  //xmit->buf + xmit->tail
			up->dma_tx_cnt = length;
			DEBUG("In %s Normal mode\n",__FUNCTION__); 
		}else{
			memcpy(up->dma_tx_buf_v,&xmit->buf[xmit->tail],DMA_TX_BUFFER_SZ);
			up->dma_tx_cnt = DMA_TX_BUFFER_SZ;
		}	
	}else{
		if(length <= DMA_TX_BUFFER_SZ){
			DEBUG("In %s 2nd mode\n",__FUNCTION__); 
			memcpy(up->dma_tx_buf_v,&xmit->buf[xmit->tail],len2end);
			memcpy(up->dma_tx_buf_v+len2end,xmit->buf,xmit->head);
			up->dma_tx_cnt = length;
		}else{
			if(len2end <= DMA_TX_BUFFER_SZ){
				memcpy(up->dma_tx_buf_v,&xmit->buf[xmit->tail],len2end);
				memcpy(up->dma_tx_buf_v+len2end, xmit->buf, DMA_TX_BUFFER_SZ-len2end);
				up->dma_tx_cnt = len2end;
			}else{
				memcpy(up->dma_tx_buf_v,&xmit->buf[xmit->tail],DMA_TX_BUFFER_SZ);
				up->dma_tx_cnt = DMA_TX_BUFFER_SZ;
			}	
		}
	}


	DEBUG("In %s length=%d\n",__FUNCTION__,length);
		
	//INITIATING THE NEXT TRANSFER
		
	//Writing the source address to the TX DMA 
	writel(up->dma_tx_buf_p,up->port.membase+REG_TX_DMA_START_ADDRESS_LOW);
	//Writing the source address to the TX DMA 
	writel(0,up->port.membase+REG_TX_DMA_START_ADDRESS_HIGH);			
	//Writing the length of data to the TX DMA Length register
	writel(length,up->port.membase+REG_TX_DMA_LENGTH);			
	//Start the DMA data transfer
	writel(TX_DMA_START_BIT,up->port.membase+REG_TX_DMA_START);
		
	// Requesting more data to send out from the TTY layer to the driver
	if (length < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	return 0;
}

//Helper function to do the necessary action upon the successful completion of data receive in DMA mode
static void receive_chars_dma_done(struct uart_99xx_port * up, int iirg)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
	struct tty_struct *tty = up->port.info->tty;
#else
        struct tty_struct *tty = up->port.info->port.tty;
#endif

	int i,rxdma_done=0;
	u16 received_bytes;
	u32 need2recv;
	u8 status = serial_in(up, UART_LSR);
	
	DEBUG("In %s ---------iirg=0x%x------------------------------START\n",__FUNCTION__,iirg);
	//checking for the flip buffer size and asking to clear it upon some threshold
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
	if (unlikely(tty->flip.count >= TTY_FLIPBUF_SIZE)) {
		if (tty->low_latency) {
			spin_unlock(&up->port.lock);
			tty_flip_buffer_push(tty);
			spin_lock(&up->port.lock);
		}
	}
#endif
	if ((iirg & SPINTR_RXDMA_PARTDONE) || (iirg & SPINTR_RXDMA_DONE) || (iirg & SPINTR_RXDMA_ABORT_DONE)){
		//checking for the number of bytes received 
		need2recv=readl(up->port.membase + REG_RX_BYTES_NEED_TO_RECV);
		DEBUG("In %s --------Receive DMA Part Done need2recv=%d\n",__FUNCTION__,need2recv);
	
	
		if (iirg & SPINTR_RXDMA_ABORT_DONE){
			DEBUG("In %s --------Receive DMA ABORT Done\n",__FUNCTION__);
			DEBUG("up->rx_dma_done_cnt=%d\n",up->rx_dma_done_cnt);
			//Reinitialise the DMA for the rest of the characters in the previous instruction
			writel((up->dma_rx_buf_p+(DMA_RX_SZ * (up->rx_dma_done_cnt-1))),up->port.membase+REG_RX_DMA_START_ADDRESS_LOW);
			writel(0,up->port.membase+REG_RX_DMA_START_ADDRESS_HIGH);
			writel(need2recv,up->port.membase+REG_RX_DMA_LENGTH);
			writel(RX_DMA_START_BIT,up->port.membase+REG_RX_DMA_START);	
			DEBUG("REG_RX_DMA_START_ADDRESS_LOW:0x%x\n",readl(up->port.membase+REG_RX_DMA_START_ADDRESS_LOW));
			DEBUG("REG_RX_DMA_START_ADDRESS_HIGH:0x%x\n",readl(up->port.membase+REG_RX_DMA_START_ADDRESS_HIGH));
			DEBUG("REG_RX_DMA_LENGTH:0x%x\n",readl(up->port.membase+REG_RX_DMA_LENGTH));
		}

		if (iirg & SPINTR_RXDMA_DONE){
			DEBUG("In %s --------Receive DMA Done\n",__FUNCTION__);
			DEBUG("In %s --------up->rx_dma_done_cnt=%d\n",__FUNCTION__,up->rx_dma_done_cnt);
			//Reinitialise the DMA
			writel((up->dma_rx_buf_p+(DMA_RX_SZ * up->rx_dma_done_cnt)),up->port.membase+REG_RX_DMA_START_ADDRESS_LOW);
			writel(0,up->port.membase+REG_RX_DMA_START_ADDRESS_HIGH);
			writel(DMA_RX_SZ,up->port.membase+REG_RX_DMA_LENGTH);
			writel(RX_DMA_START_BIT,up->port.membase+REG_RX_DMA_START);	
			DEBUG("REG_RX_DMA_START_ADDRESS_LOW:0x%x\n",readl(up->port.membase+REG_RX_DMA_START_ADDRESS_LOW));
			DEBUG("REG_RX_DMA_START_ADDRESS_HIGH:0x%x\n",readl(up->port.membase+REG_RX_DMA_START_ADDRESS_HIGH));
			DEBUG("REG_RX_DMA_LENGTH:0x%x\n",readl(up->port.membase+REG_RX_DMA_LENGTH));
			rxdma_done=1;
		}
	
		received_bytes=((DMA_RX_SZ * (up->rx_dma_done_cnt))-(need2recv + up->part_done_recv_cnt));
	
		if(rxdma_done){
			up->rx_dma_done_cnt++;
			rxdma_done=0;
		}
		if (up->rx_dma_done_cnt == (DMA_RX_BUFFER_SZ/DMA_RX_SZ))
			up->rx_dma_done_cnt=0;
				
		DEBUG("In %s --------Receive DMA Part Done received_bytes=%d\n part_recv_cnt=%d\n",__FUNCTION__,received_bytes,up->part_done_recv_cnt);
		//copiying the recived bytes to the TTY layers flip buffer
		
		if (tty){
			DEBUG("received_bytes=%d and up->part_done_recv_cnt=%d\n",received_bytes,up->part_done_recv_cnt);
			for (i = 1; i <= received_bytes; i++){
				/* if we insert more than TTY_FLIPBUF_SIZE characters, tty layer will drop them. */
#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
				if(tty->flip.count >= TTY_FLIPBUF_SIZE){
					tty_flip_buffer_push(tty);
				}
#endif
				/* this doesn't actually push the data through unless tty->low_latency is set */
				uart_insert_char(&up->port, status, UART_LSR_OE, up->dma_rx_buf_v[ up->part_done_recv_cnt], TTY_NORMAL);
				up->part_done_recv_cnt++;
				DEBUG("char=%c \n",up->dma_rx_buf_v[up->part_done_recv_cnt]);
				if(up->part_done_recv_cnt == DMA_RX_BUFFER_SZ)
					up->part_done_recv_cnt = 0;
	            }
				tty_flip_buffer_push(tty);
		}
		
		up->port.icount.rx += received_bytes;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,11)
		if (unlikely(tty->flip.count >= TTY_FLIPBUF_SIZE)) {
			if (tty->low_latency) {
				spin_unlock(&up->port.lock);
				tty_flip_buffer_push(tty);
				spin_lock(&up->port.lock);
			}
		}
#endif
	}
}



//This handles the interrupt from a port in IO mode.
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
static inline void serial99xx_handle_port(struct uart_99xx_port *up)
#else
static inline void serial99xx_handle_port(struct uart_99xx_port *up, struct pt_regs *regs)
#endif
{
	u8 status = serial_in(up, UART_LSR);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
	struct tty_struct *tty=up->port.info->tty;
#else
        struct tty_struct *tty = up->port.info->port.tty;
#endif	

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("device structure id for port%d is %u\n",up->port.line,up);
	DEBUG("UART_LSR = %x...", status);

	if((status & UART_LSR_DR) && !up->dma_rx){
		DEBUG("RECEIVE_CHARS\n");
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
		receive_chars(up, &status);
	#else
		receive_chars(up, &status, regs);
	#endif
	
	}

	check_modem_status(up);
	
	if ((status & UART_LSR_THRE) && !up->dma_tx){
		DEBUG("TRANSMIT_CHARS\n");
		transmit_chars(up);
	}

	if(up->dma_rx){
		if (status & (UART_LSR_BI | UART_LSR_PE |UART_LSR_FE | UART_LSR_OE)){
			//For statistics only
			if (status & UART_LSR_BI) {
				status &= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port))
					return;
			}else if (status & UART_LSR_PE)
				up->port.icount.parity++;
			else if (status & UART_LSR_FE)
				up->port.icount.frame++;
			if (status & UART_LSR_OE)
				up->port.icount.overrun++;
	
				//Mask off conditions which should be ignored.
				status &= up->port.read_status_mask;		
			}
		if (status & ~up->port.ignore_status_mask & UART_LSR_OE)
			tty_insert_flip_char(tty, 0, TTY_OVERRUN);	
	}
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);	
}


// This is the 99xx type serial driver's interrupt routine.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
static irqreturn_t serial99xx_interrupt(int irq, void *dev_id)
#else
static irqreturn_t serial99xx_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	struct uart_99xx_port *up = dev_id;
	u32 iirg=0;
	u8 iir;
	int handled=0;
	DEBUG("In serial99xx_interrupt function\n");
	DEBUG("line number is %d\n",up->port.line);
	DEBUG("up =%x up->prev_port :%x and up->next_port :%x\n" ,up,up->prev_port,up->next_port);
	iirg= readl(up->port.membase+ REG_GLBL_ISR);
	//writel(0xFFFFFFFE,up->port.membase+REG_GLBL_ICLR);
	//printk("iirg is %x\n",iirg);
	if(!(iirg & 0xFE))
	{
		up = up->next_port;
		iirg= readl(up->port.membase+ REG_GLBL_ISR);
	//	printk("iirg of next_port is %d \n",iirg);
	}
	//writel(0xFFFFFFFE,up->port.membase+REG_GLBL_ICLR);
	if(iirg & 0xFE){
		
		DEBUG("In %s ---GLobal Interrupt iirg=0x%x--------------------------START\n",__FUNCTION__,iirg);
		
		spin_lock(&up->port.lock);
		writel(0xFFFFFFFE,up->port.membase+REG_GLBL_ICLR);
		
		if(iirg & SPINTR_TXDMA_DONE){
			transmit_chars_dma_done(up);
			handled=1;
		}
//		if((iirg & SPINTR_TXDMA_STOP_DONE) ||(iirg & SPINTR_TXDMA_ABORT_DONE)){
//			handled=1;
//			transmit_chars_dma_stop_done(up);
		if(iirg & (SPINTR_RXDMA_DONE | SPINTR_RXDMA_PARTDONE)){
			receive_chars_dma_done(up,iirg);
			handled=1;
		}
		if(iirg & SPINTR_RXDMA_STOP_DONE){
			DEBUG("In %s SPINTR_RXDMA_STOP_DONE is Handled\n",__FUNCTION__);
			handled=1;
		}
		if(iirg & SPINTR_RXDMA_ABORT_DONE){
			DEBUG("In %s SPINTR_RXDMA_ABORT_DONE is Handled\n",__FUNCTION__);
			handled=1;
		}
		spin_unlock(&up->port.lock);
	}
	if(up->prev_port == NULL)
	{	
		DEBUG("control is in normal port\n");
		iir = serial_in(up, UART_IIR);
		DEBUG("device structure id for port%d is %u\n",up->port.line,up);
		DEBUG("IIR value is 0x%x\n",iir);
		if(iir & UART_IIR_NO_INT){
			if(up->next_port!=NULL)
			{
				up = up->next_port;
				DEBUG("device structure id of next_port for port%d is %u\n",up->port.line,up);
				iir = serial_in(up, UART_IIR);
				DEBUG("IIR value of next_port is 0x%x\n",iir);
			}
		}
	}
	else
        {
		DEBUG("control is in cascade port\n");
                iir = serial_in(up, UART_IIR);
                DEBUG("device structure id for port%d is %u\n",up->port.line,up);
                DEBUG("IIR value is 0x%x\n",iir);
                if(iir & UART_IIR_NO_INT){
                        if(up->prev_port!=NULL)
                        {
                                up = up->prev_port;
                                DEBUG("device structure id of prev_port for port%d is %u\n",up->port.line,up);
                                iir = serial_in(up, UART_IIR);
                                DEBUG("IIR value of prev_port is 0x%x\n",iir);
                        }
        	}
	//	DEBUG("In %s up->port.line= %d\n",__FUNCTION__,up->port.line);
	}	
	DEBUG("In %s up->port.line= %d\n",__FUNCTION__,up->port.line);

	if (!(iir & UART_IIR_NO_INT)) {
		DEBUG("In %s up->port.line= %d\n",__FUNCTION__,up->port.line);
		DEBUG("In %s IIR=0x%x UART_SCR=0x%x\n",__FUNCTION__,iir,serial_in(up,UART_SCR));
		spin_lock(&up->port.lock);
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
		serial99xx_handle_port(up);
	#else
		serial99xx_handle_port(up, regs);
	#endif	
		spin_unlock(&up->port.lock);
		handled = 1;
	} 
	return IRQ_RETVAL(handled);
}

//This is a port ops helper function to verify wether the transmitter is empty of not
static unsigned int serial99xx_tx_empty(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	unsigned long flags;
	unsigned int ret;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	spin_lock_irqsave(&up->lock_99xx, flags);
	ret = serial_in(up, UART_LSR) & UART_LSR_TEMT ? TIOCSER_TEMT : 0;
	spin_unlock_irqrestore(&up->lock_99xx, flags);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	
	return ret;
}

//This is a port ops helper function to find the current state of the modem control
static unsigned int serial99xx_get_mctrl(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	unsigned long flags;
	u8 status;
	unsigned int ret;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	spin_lock_irqsave(&up->lock_99xx, flags);
	status = serial_in(up, UART_MSR);
	spin_unlock_irqrestore(&up->lock_99xx, flags);

	ret = 0;
	if (status & UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & UART_MSR_DSR)
		ret |= TIOCM_DSR;
	if (status & UART_MSR_CTS)
		ret |= TIOCM_CTS;
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	
	return ret;
}

//This is a port ops helper function to set the modem control lines
static void serial99xx_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	u8 mcr = 0;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	if (mctrl & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (mctrl & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (mctrl & TIOCM_OUT1)
		mcr |= UART_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= UART_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;
	
	mcr |= up->mcr;
	serial_out(up, UART_MCR, mcr);

	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//This is a port ops helper function to control the transmission of a break signal
static void serial99xx_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	unsigned long flags;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	spin_lock_irqsave(&up->lock_99xx, flags);
	if (break_state == -1)
		up->lcr |= UART_LCR_SBC;
	else
		up->lcr &= ~UART_LCR_SBC;
	serial_out(up, UART_LCR, up->lcr);
	spin_unlock_irqrestore(&up->lock_99xx, flags);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
}

//This is a port ops helper function to enable the port for reception
static int serial99xx_startup(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	unsigned long flags;
	u8 fcr=0,efr=0,efr1=0,lcr=0,cks=0,acr=0,mcr=0;
	u32 tmp,value,ser_dcr_din_val=0,ser_ven_val=0;
	int uart_mode=0;
	

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("Device structure address id %x\n",up);
	DEBUG(" In startup port->line is %d\n",up->port.line);
	DEBUG("membase is 0x%x\n",up->port.membase);
	DEBUG("mapbase is 0x%x\n",up->port.mapbase);
	DEBUG("iobase is 0x%x\n",up->port.iobase);
	DEBUG("port.type is %d\n",up->port.type);
	DEBUG("fifo size is %d \n",uart_config[up->port.type].fifo_size);	
	up->capabilities = uart_config[up->port.type].flags;
	up->mcr = 0;
	up->part_done_recv_cnt = 0;	
	up->rx_dma_done_cnt = 1;
	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reeanbled in set_termios())
	 */
	serial99xx_clear_fifos(up);

	/*
	 * Clear the interrupt registers.
	 */
	if(up->dma_tx || up->dma_rx)
		(void) readl(up->port.membase+REG_GLBL_ISR);
		
	(void) serial_in(up, UART_LSR);
	(void) serial_in(up, UART_RX);
	(void) serial_in(up, UART_IIR);
	(void) serial_in(up, UART_MSR);
	/*
	 * Now, initialize the UART
	 */
	serial_out(up, UART_LCR, UART_LCR_WLEN8);

	spin_lock_irqsave(&up->lock_99xx, flags);

	up->port.mctrl |= TIOCM_OUT2;

	serial99xx_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->lock_99xx, flags);


	//Rx data transfer Interrupts
	up->ier = UART_IER_RLSI | UART_IER_RDI;
	serial_out(up, UART_IER, up->ier);
	DEBUG("up->port.type %d\n",up->port.type);
	if(up->port.type == PORT_16550A){
		DEBUG("In %s 550EX mode\n",__FUNCTION__);
		ser_dcr_din_val=readl(up->port.membase+SER_DCR_DIN_REG);
		ser_dcr_din_val |= COM_550EX_MODE_EN;
		writel(ser_dcr_din_val,up->port.membase+SER_DCR_DIN_REG);
		DEBUG("In %s 550EX mode SER_DCR_DIN_REG=0x%x\n",__FUNCTION__,readl(up->port.membase+SER_DCR_DIN_REG));
	
		if(up->flow_control){
			DEBUG("Enabled the Auto Hardware Flowcontrol\n");
			mcr = serial_in(up,UART_MCR);
			DEBUG("In %s mcr=0x%x and up->port.mctrl=0x%x\n",__FUNCTION__,mcr,up->port.mctrl);
			
			up->mcr |= UART_MCR_AFE;
			serial99xx_set_mctrl(&up->port,up->port.mctrl);	
			mcr = serial_in(up,UART_MCR);
			DEBUG("In %s mcr=0x%x and up->port.mctrl=0x%x\n",__FUNCTION__,mcr,up->port.mctrl);
		}	

		if (up->capabilities & UART_CAP_FIFO && uart_config[port->type].fifo_size > 1) {
				fcr = uart_config[up->port.type].fcr;
				serial_out(up,UART_FCR,fcr);
		}
	}

	if(up->port.type == PORT_ENHANC){
		//Setting the Enhanced Mode Features
		DEBUG("In ENHANCED MODE\n");
		setserial_ENHANC_mode(up);	

		if (up->capabilities & UART_CAP_FIFO && uart_config[port->type].fifo_size > 1) {
				DEBUG("In assigning fcr\n");
				fcr = uart_config[up->port.type].fcr;
				serial_out(up,UART_FCR,fcr);
		}

		
		switch(up->uart_mode){

			case MCS99XX_RS485_HALF_DUPLEX:
				//Commset Registers Offset 0
				//0x0008 0000  -19thBit -1 SwRS 485 enable
				//0x0000 0000  -17thBit -0 swFD enable RS485
				//0x0000 2000  -13thBit -1 RS485 RTS enable
 				//0x0000 0000  -14thBit -0 SwRTS enable

				DEBUG("TranceiverMode MCS99XX_RS485_MODE - RS485_HALF_DUPLEX\n");
				uart_mode=1;	
				ser_dcr_din_val = readl(up->port.membase+SER_DCR_DIN_REG);
				ser_dcr_din_val &= 0xfff00fff;
				ser_dcr_din_val |= 0x00080000;

				//Commset Registers Offset 1
				//0xff00 0000  -[31-24] ff
				ser_ven_val = readl(up->port.membase+SER_VEN_REG);
				ser_ven_val &= 0x00ffffff;
				ser_ven_val |= 0x00000000;

				//CKS - 0x00
				//serial_icr_write(up,UART_CKS,0x00);
				cks=serial_icr_read(up,UART_CKS);
				cks |= 0x00;

				//ACR - [4:3]-10 -> 0x10, 
				acr = 0x10;
				break;
				
			case MCS99XX_RS485_HALF_DUPLEX_ECHO:
				//Commset Registers Offset 0
				//0x0008 0000  -19thBit -1 SwRS 485 enable
				//0x0004 0000  -18thBit -1 SwEcho Rs485 enable
				//0x0000 0000  -17thBit -0 swFD enable RS485
				//0x0000 2000  -13thBit -1 RS485 RTS enable
				//0x0000 4000  -14thBit -1 SwRTS enable
				DEBUG("TranceiverMode MCS99XX_RS485_MODE - RS485_HALF_DUPLEX_ECHO\n");
				uart_mode=1;
				ser_dcr_din_val = readl(up->port.membase+SER_DCR_DIN_REG);
				ser_dcr_din_val &= 0xfff00fff;
				ser_dcr_din_val |= 0x000C6000;

				//Commset Registers Offset 1
				//0xff00 0000  -[31-24] ff
				ser_ven_val = readl(up->port.membase+SER_VEN_REG);
				ser_ven_val &= 0x00ffffff;
				ser_ven_val |= 0xff000000;

				//CKS - 0x00
				cks = serial_icr_read(up,UART_CKS);
				cks |= 0x00;

				//ACR - [4:3]-11 -> 0x18, 
				acr = 0x18;
				
				break;

			case MCS99XX_RS485_FULL_DUPLEX:
			case MCS99XX_RS422_MODE:

				//Commset Registers Offset 0
				//0x0008 0000  -19thBit - SwRS 485 enable
				//0x0002 0000  -17thBit - SwFD enable RS485
				//0x0000 0000  -13thBit - RS485 RTS enable(should be 0)
				//0x0000 4000  -14thBit - SwRTS enable
				DEBUG("TranceiverMode MCS99XX_RS485_MODE - RS485_FULL_DUPLEX\n");
				uart_mode=1;
				ser_dcr_din_val = readl(up->port.membase+SER_DCR_DIN_REG);
				ser_dcr_din_val &= 0xfff00fff;
				ser_dcr_din_val |= 0x000A4000;

				//Commset Registers Offset 1
				//0xff00 0000  -[31-24] ff
				ser_ven_val = readl(up->port.membase+SER_VEN_REG);
				ser_ven_val &= 0x00ffffff;
				ser_ven_val |= 0xff000000;

				//CKS - 0x00
				cks = serial_icr_read(up,UART_CKS);
				cks |= 0x00;

				//ACR - 0x10, 
				acr = 0x10;
				break;
			default:
				DEBUG("Tranceiver RS_232 mode\n");
				break;
		}

		if(uart_mode){	
			up->acr = up->acr|acr;

			writel(ser_dcr_din_val,up->port.membase+SER_DCR_DIN_REG);
			writel(ser_ven_val,up->port.membase+SER_VEN_REG);
			serial_icr_write(up,UART_CKS,cks);
			serial_icr_write(up,UART_ACR,up->acr);
			
			DEBUG("SER_DCR_DIN_REG=0x%x   SER_VEN_REG=0x%x   UART_CKS=0x%x   UART_ACR=0x%x\n",readl(up->port.membase+SER_DCR_DIN_REG),readl(up->port.membase+SER_VEN_REG),cks,up->acr);
			uart_mode = 0;
		}

		//Setting the trigger Levels
		serial_icr_write(up,UART_TTL,up->txfifotrigger);
		serial_icr_write(up,UART_RTL,up->rxfifotrigger);
		up->acr |= UART_ACR_TLENB;
		serial_icr_write(up,UART_ACR,up->acr);

		//If Hardware Flow Control is to be enabled. The RTS/CTS, DTR/DTS is possible only in 232 mode. 
		if(up->flow_control){
			
			//Setting the auto hardware flow control trigger levels
			serial_icr_write(up,UART_FCL,16);
			serial_icr_write(up,UART_FCH,240);

			//Setting the hw Flow control
			switch(up->flow_ctrl_type){
				case MCS99XX_DTR_DSR_HW_FLOWCONTROL:
					if(up->uart_mode == MCS99XX_RS232_MODE){
						DEBUG("H/W Flow Control MCS99XX_DTR_DSR_HW_FLOWCONTROL enabled\n");
						DEBUG("UART_ACR=0x%x and up->acr=0x%x\n",serial_icr_read(up,UART_ACR),up->acr);
						up->acr |= 0x0C;
						serial_icr_write(up, UART_ACR, up->acr);
						
						break;
					}else{
						DEBUG("No flow control enabled\n");
						break;
					}			
						
					
				case MCS99XX_XON_XOFF_HW_FLOWCONTROL:
					DEBUG("Enabled HwFlowControl MCS99XX_XON_XOFF_HW_FLOWCONTROL\n");
					lcr = serial_in(up,UART_LCR);

					serial_out(up,UART_LCR,0xBF);

					efr=serial_in(up,UART_EFR);
					efr1 = efr;
					efr |= 0x1A;
					serial_out(up,UART_EFR,efr);
					serial_out(up,UART_XON1,up->x_on);
					serial_out(up,UART_XOFF1,up->x_off);											
					serial_out(up,UART_XON2,up->x_on);
					serial_out(up,UART_XOFF2,up->x_off);
					serial_out(up,UART_EFR,efr1);
					serial_out(up,UART_LCR,lcr);
					break;

				case MCS99XX_RTS_CTS_HW_FLOWCONTROL:
				default:
					if(up->uart_mode == MCS99XX_RS232_MODE){
						DEBUG("H/W Flow Control MCS99XX_RTS_CTS_HW_FLOWCONTROL enabled\n");
						lcr = serial_in(up,UART_LCR);
						serial_out(up,UART_LCR,0xBF);	
						efr=serial_in(up,UART_EFR);
						efr |= 0xD0;
						serial_out(up,UART_EFR,efr);									
						serial_out(up,UART_LCR,lcr);	
						break;	
					}else{
						DEBUG("No H/W flow control enabled\n");
					}

			}
		}
	}
	
	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	if(up->dma_rx || up->dma_tx){
		//Set the comset DMA register to enable DMA
		DEBUG("DMA bit in DCR register was set ");
		ser_dcr_din_val=readl(up->port.membase+SER_DCR_DIN_REG);
		ser_dcr_din_val |= COM_DMA_MODE_EN;
		writel(ser_dcr_din_val,up->port.membase+SER_DCR_DIN_REG);
		DEBUG("SER_DCR_DIN_REG=0x%x\n",readl(up->port.membase+SER_DCR_DIN_REG));	
	}

	if(up->dma_rx){
		//clear the rx DMA registers
		value=readl(up->port.membase+REG_RX_BYTES_NEED_TO_RECV);	
		DEBUG("REG_RX_BYTES_NEED_TO_RECV=0x%x \n",value);
	}

	if(up->dma_rx || up->dma_tx){	
		writel(0xFFFFFFFF,up->port.membase+REG_GLBL_IER);
		tmp=readl(up->port.membase+REG_GLBL_IER);
		DEBUG("REG_GLBL_IER=0x%x, up->port.membase=0x%x\n",tmp,(unsigned int)up->port.membase+REG_GLBL_IER);	
	}

	if(up->dma_rx){
		DEBUG("RX_DMA engine started\n");
		writel(up->dma_rx_buf_p,up->port.membase+REG_RX_DMA_START_ADDRESS_LOW);
		writel(0,up->port.membase+REG_RX_DMA_START_ADDRESS_HIGH);
		writel(DMA_RX_SZ,up->port.membase+REG_RX_DMA_LENGTH);
		writel(RX_DMA_START_BIT,up->port.membase+REG_RX_DMA_START);	
	}	

	/*
	 * And clear the interrupt generating registers again for luck.
	 */
	(void) serial_in(up, UART_LSR);
	(void) serial_in(up, UART_RX);
	(void) serial_in(up, UART_IIR);
	(void) serial_in(up, UART_MSR);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);

	return 0;
}

//This is a port ops helper function to disable the port, disable any break condition that may be in
//effect, and free any interrupt resources.
static void serial99xx_shutdown(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	unsigned long flags;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	DEBUG("up= %x up->prev_port :%x and up->next_port :%x\n" ,up,up->prev_port,up->next_port);
	DEBUG("Device structure address for port%d id %u\n",port->line,up);
	DEBUG("membase is 0x%x\n",up->port.membase);
        DEBUG("mapbase is 0x%x\n",up->port.mapbase);
        DEBUG("iobase is 0x%x\n",up->port.iobase);

	DEBUG("No of Errors In ttyF%d brake=%d frame=%d parity=%d overrun=%d \n", \
		port->line, port->icount.brk, port->icount.frame, port->icount.parity,\
		port->icount.overrun);
	/*
	 * Disable interrupts from this port
	 */
	up->ier = 0;
	serial_out(up, UART_IER, 0);

	spin_lock_irqsave(&up->lock_99xx, flags);
	up->port.mctrl &= ~TIOCM_OUT2;

	serial99xx_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->lock_99xx, flags);

	/*
	 * Disable break condition and FIFOs
	 */
	serial_out(up, UART_LCR, serial_in(up, UART_LCR) & ~UART_LCR_SBC);
	serial99xx_clear_fifos(up);

	/*
	 * Read data port to reset things
	 */
	(void) serial_in(up, UART_LSR);
       // (void) serial_in(up, UART_RX);
        (void) serial_in(up, UART_IIR);
        (void) serial_in(up, UART_MSR);
	(void) serial_in(up, UART_RX);

	up->lcr = 0;
	up->mcr = 0;
	up->ser_dcr_din_reg = 0;
	up->ser_ven_reg = 0;
	DEBUG("up->port.type is %d\n",up->port.type);
	//Reset the UART upon port close
	if(up->port.type == PORT_ENHANC){
		
		DEBUG("In ENHANCED MODE\n");
		DEBUG("up= %x up->prev_port :%x and up->next_port :%x\n" ,up,up->prev_port,up->next_port);
		DEBUG("membase is 0x%x\n",up->port.membase);
	        DEBUG("mapbase is 0x%x\n",up->port.mapbase);
        	DEBUG("iobase is 0x%x\n",up->port.iobase);
		up->acr = 0x00;
	
		// ENHANC Mode reset
		serial_icr_write(up, UART_CSR, 0x00);
//		serial_icr_write(up, UART_CSR, 0xFF);
		
		// Serial soft reset
		writel(0x01,up->port.membase+SER_SOFT_RESET_REG);
		DEBUG("UART_ACR=0x%x and up->acr=0x%x\n",serial_icr_read(up,UART_ACR),up->acr);
		
	
	}else{
		// Serial soft reset
		writel(0x01,up->port.membase+SER_SOFT_RESET_REG);
	}

	DEBUG("In %s --------------------------------------END\n",__FUNCTION__);
}

//This is a port ops helper function to return the divsor (baud_base / baud) for the selected baud rate 
//	specified by termios.
static unsigned int serial99xx_get_divisor(struct uart_port *port, unsigned int baud)
{
	unsigned int quot;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);

	quot = uart_get_divisor(port, baud);

	DEBUG("In %s quot=%u----baud=%u-----------------------------END\n",__FUNCTION__,quot,baud);
	return quot;	
}

//This is a port ops function to set the terminal settings.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
static void serial99xx_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
#else
static void serial99xx_set_termios(struct uart_port *port, struct termios *termios, struct termios *old)
#endif

{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	u8 cval,fcr=0;
	unsigned long flags;
	unsigned int baud, quot;
	u32 clk_sel_val=0,ser_ven_val=0;

	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = 0x00;
		break;
	case CS6:
		cval = 0x01;
		break;
	case CS7:
		cval = 0x02;
		break;
	default:
	case CS8:
		cval = 0x03;
		break;
	}

	if (termios->c_cflag & CSTOPB)
		cval |= UART_LCR_STOP;
	if (termios->c_cflag & PARENB)
		cval |= UART_LCR_PARITY;
	if (!(termios->c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
#ifdef CMSPAR
	if (termios->c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;
#endif

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk); 
	DEBUG("In %s -------------------baud=%u\n",__FUNCTION__,baud);

	if(test_mode){
		switch (baud){
	
			case 300:
				baud = 460800;
				break;	
			case 1200:
				baud = 921600;
				break;
			case 2400:
				baud = 1500000;
				break;
			case 4800:
				baud = 3000000;
				break;
			default:
				break;
		}
		DEBUG("In %s -----baud=%u\n",__FUNCTION__,baud);
	}

	if(baud > 115200)
		quot = 1;
	else
		quot = serial99xx_get_divisor(port, baud);
	
	if((baud == 230400)){
		clk_sel_val=0x10;
	}else if(baud == 403200){
        	clk_sel_val=0x20;
	}else if(baud == 460800){
        	clk_sel_val=0x30;
	}else if(baud == 806400){
		clk_sel_val=0x40;
	}else if(baud == 921600){
	        clk_sel_val=0x50;
	}else if(baud == 1500000){
		clk_sel_val=0x60;
	}else if(baud == 3000000){
		clk_sel_val=0x70;
	}

	if (up->capabilities & UART_CAP_FIFO && uart_config[port->type].fifo_size > 1) {
		if (baud < 2400)
			fcr = UART_FCR_ENABLE_FIFO | UART_FCR_TRIGGER_1;
		else
			fcr = uart_config[up->port.type].fcr;
	}
	
	/*
	 * MCR-based auto flow control.  When AFE is enabled, RTS will be
	 * deasserted when the receive FIFO contains more characters than
	 * the trigger, or the MCR RTS bit is cleared.  In the case where
	 * the remote UART is not using CTS auto flow control, we must
	 * have sufficient FIFO entries for the latency of the remote
	 * UART to respond.  IOW, at least 32 bytes of FIFO.
	 */
	if (up->capabilities & UART_CAP_AFE && uart_config[port->type].fifo_size >= 32) {
		up->mcr &= ~UART_MCR_AFE;
		if (termios->c_cflag & CRTSCTS)
			up->mcr |= UART_MCR_AFE;
	}

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	spin_lock_irqsave(&up->lock_99xx, flags);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characteres to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		up->port.ignore_status_mask |= UART_LSR_DR;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier &= ~UART_IER_MSI;
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;
	serial_out(up, UART_IER, up->ier);
	if (up->capabilities & UART_CAP_EFR) {
		unsigned char efr = 0;
		/*
		 * TI16C752/Startech hardware flow control.  FIXME:
		 * - TI16C752 requires control thresholds to be set.
		 * - UART_MCR_RTS is ineffective if auto-RTS mode is enabled.
		 */
		if (termios->c_cflag & CRTSCTS)
			efr |= UART_EFR_CTS;

		serial_out(up, UART_LCR, 0xBF);
		serial_out(up, UART_EFR, efr);
	}


	//reset clk sel val 
	ser_ven_val = readl(up->port.membase+SER_VEN_REG);
	ser_ven_val = 0;
	writel(ser_ven_val, up->port.membase + SER_VEN_REG);
	DEBUG("In %s SER_VEN_REG=0x%x\n",__FUNCTION__,readl(up->port.membase+SER_VEN_REG));

	//for baud rates of 2x to 3Mbps
	ser_ven_val = readl(up->port.membase+SER_VEN_REG);
	ser_ven_val |= clk_sel_val;
	writel(ser_ven_val, up->port.membase + SER_VEN_REG);
	DEBUG("In %s SER_VEN_REG=0x%x\n",__FUNCTION__,readl(up->port.membase+SER_VEN_REG));

	//clock prescaling is used for higher baudrates than 3mbps
	if(baud > 3145728){
		writel(0, up->port.membase+SP_CLK_SELECT_REG); 
		DEBUG("In %s SP_CLK_SELECT_REG=0x%x\n",__FUNCTION__,readl(up->port.membase+SP_CLK_SELECT_REG));
		DEBUG("In %s SP_CLK_SELECT_REG=0x%x\n",__FUNCTION__,readl(up->port.membase+SP_CLK_SELECT_REG));
	}

	serial_out(up, UART_LCR, cval | UART_LCR_DLAB);/* set DLAB */

	serial_out(up, UART_DLL, quot & 0xff);		/* LS of divisor */
	serial_out(up, UART_DLM, quot >> 8);		/* MS of divisor */

	serial_out(up, UART_LCR, cval);		/* reset DLAB */

	up->lcr = cval;				/* Save LCR */

	if (up->port.type != PORT_16750) {
		if (fcr & UART_FCR_ENABLE_FIFO) {
			/* emulated UARTs (Lucent Venus 167x) need two steps */
			serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO);
			
		}
	serial_out(up,UART_FCR,fcr);		/* set fcr */
	DEBUG("In %s UART_FCR is written with fcr=0x%x\n",__FUNCTION__,fcr);	
	}
	serial99xx_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->lock_99xx, flags);
	DEBUG("In %s ------------------------------END\n",__FUNCTION__);
}

static void serial99xx_pm(struct uart_port *port, unsigned int state, unsigned int oldstate)
{
	struct uart_99xx_port *p = &serial99xx_ports[port->line];
	serial99xx_set_sleep(p, state != 0);
}

//Helper function to relase the kernel resources used by the port
static void serial99xx_release_port(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);
	
	iounmap(up->port.membase);
	
	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
}

//Helper function to get the necessary kernel resources for the port
static int serial99xx_request_port(struct uart_port *port)
{
	struct uart_99xx_port *up = &serial99xx_ports[port->line];
	int ret = 0,size=8,mem_size=4096;

	//TODO:the mem_size and the mem2_size are not yet known properly
	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);
	if(!request_region(up->port.iobase, size, "saturn-cascade-serial")){
		ret = -EBUSY;
		goto release1;
	}
	
	if(!request_mem_region(up->port.mapbase, mem_size, "saturn-cascade-serial")){
		ret = -EBUSY;
		goto release2;
	}
	
	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
	return ret;
	
release2:
	release_region(up->port.iobase,size);
release1:
	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
	return ret;
}

static const char *serial99xx_type(struct uart_port *port)
{
	return "saturn-cascade-serial";
}

static struct uart_ops serial99xx_pops = {
	.tx_empty	= serial99xx_tx_empty,
	.set_mctrl	= serial99xx_set_mctrl,
	.get_mctrl	= serial99xx_get_mctrl,
	.stop_tx	= serial99xx_stop_tx,
	.start_tx	= serial99xx_start_tx,
	.stop_rx	= serial99xx_stop_rx,
	.enable_ms	= serial99xx_enable_ms,
	.break_ctl	= serial99xx_break_ctl,
	.startup	= serial99xx_startup,
	.shutdown	= serial99xx_shutdown,
	.set_termios	= serial99xx_set_termios,
	.pm		= serial99xx_pm,
	.type		= serial99xx_type,
	.release_port	= serial99xx_release_port,
	.request_port	= serial99xx_request_port,

};

//Initialising the global per port context array to the default values
static void serial99xx_init_port(struct uart_99xx_port *up)
{
	spin_lock_init(&up->port.lock);
	spin_lock_init(&up->lock_99xx);
	up->port.ops 	= &serial99xx_pops;
	up->port.iotype = UPIO_PORT;
	up->port.type 	= PORT_16550A;
	up->port.flags |= UPF_SHARE_IRQ;
}

//Initialising the maximum allowed per port Structures with the default values
static void __init serial99xx_init_ports(void)
{
	int i;
	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);

	memset(serial99xx_ports,0,UART99xx_NR*sizeof(struct uart_99xx_port));
	for (i = 0; i < UART99xx_NR; i++) {
		serial99xx_init_port(&serial99xx_ports[i]);
		serial99xx_ports[i].port.line = i;
		
	}
	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
}

/*
 *	Are the two ports equivalent?
 */
int serial99xx_match_port(struct uart_port *port1, struct uart_port *port2)
{
	if (port1->iotype != port2->iotype)
		return 0;

	if ((port1->iobase == port2->iobase) && (port1->membase == port2->membase)){
		return 1;
	}
	else
		return 0;
}

static DECLARE_MUTEX(serial99xx_sem);

static struct uart_driver starex_serial_driver = {
        .owner                  = THIS_MODULE,
        .driver_name            = "saturn-cascade-serial",
        .dev_name               = "ttyF",//E",//D",
        .major                  = 200,
        .minor                  = 0,
        .nr                     = UART99xx_NR,
        .cons                   = NULL,
};

int serial99xx_find_match_or_unused(struct uart_port *port)
{
	int i;
	
	/*
	 * We didn't find a matching entry, so look for the first
	 * free entry.  We look for one which hasn't been previously
	 * used (indicated by zero iobase).
	 */
	for (i = 0; i < UART99xx_NR; i++){
		if (serial99xx_ports[i].port.iobase == 0){
			return i;
			}
	}	

	/*
	 * That also failed.  Last resort is to find any entry which
	 * doesn't have a real port associated with it.
	 */
	for (i = 0; i < UART99xx_NR; i++){
		if (serial99xx_ports[i].port.type == PORT_UNKNOWN){
			return i;
			}	
	}		
	return -1;
}

int serial99xx_register_port(struct uart_port *port,int index,int port_context,struct pci_dev *dev)
{
	//unsigned long base, len;
	int ret = -ENOSPC;
	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);
	if (port->uartclk == 0)
		return -EINVAL;

//	down(&serial99xx_sem);

//	index = serial99xx_find_match_or_unused(port);
	if (index >= 0) {

		serial99xx_ports[index].port.iobase   = port->iobase;
		serial99xx_ports[index].port.membase  = port->membase;
		serial99xx_ports[index].port.uartclk  = port->uartclk;
		serial99xx_ports[index].port.fifosize = port->fifosize;
		serial99xx_ports[index].port.regshift = port->regshift;
		serial99xx_ports[index].port.iotype   = port->iotype;
		serial99xx_ports[index].port.flags    = port->flags;
		serial99xx_ports[index].port.mapbase  = port->mapbase;
		serial99xx_ports[index].port.line = index;
		//This is the default value
		serial99xx_ports[index].port.type = PORT_16550A;
		if(port_context == 0)
			serial99xx_ports[index].port.irq = port->irq;
		if (port->dev)
			serial99xx_ports[index].port.dev = NULL;
		
/*		if(index==0 || index==1)
		{
			fun_no = index;
			serial99xx_ports[index].next_port = NULL;
		}
		else
		{
			serial99xx_ports[fun_no].next_port = &serial99xx_ports[index];
		}*/
		DEBUG("serial99xx_ports[index].port.type %d\n",serial99xx_ports[index].port.type);
		DEBUG("serial99xx_ports[index].port.iobase is 0x%x\n",serial99xx_ports[index].port.iobase);
		ret = uart_add_one_port(&starex_serial_driver,&serial99xx_ports[index].port);
		DEBUG("add one port ret value is %d\n",ret);
		if(ret<0)
			DEBUG("uart_add_one_port ----------failed\n");
		if (ret == 0)
			ret = serial99xx_ports[index].port.line;

		if(uart_99xx_contxts[index].tx_dma_en == 1){
			serial99xx_ports[index].dma_tx=1;
			serial99xx_ports[index].dma_tx_buf_v=(char *)pci_alloc_consistent(dev,DMA_TX_BUFFER_SZ,&serial99xx_ports[index].dma_tx_buf_p);
			serial99xx_ports[index].tx_dma=0;
			DEBUG("dma_tx_buf_v=0x%x\n dma_tx_buf_p=0x%x\n",(unsigned int)serial99xx_ports[index].dma_tx_buf_v,serial99xx_ports[index].dma_tx_buf_p);
		}else{
			serial99xx_ports[index].dma_tx=0;
			serial99xx_ports[index].dma_tx_buf_v=NULL;
		}
		
		if(uart_99xx_contxts[index].rx_dma_en == 1){
			serial99xx_ports[index].dma_rx=1;
			serial99xx_ports[index].dma_rx_buf_v=(char *)pci_alloc_consistent(dev,DMA_RX_BUFFER_SZ,&serial99xx_ports[index].dma_rx_buf_p);
			serial99xx_ports[index].part_done_recv_cnt=0;	
			serial99xx_ports[index].rx_dma_done_cnt=1;
			DEBUG("dma_rx_buf_v=0x%x\n dma_rx_buf_p=0x%x\n",(unsigned int)serial99xx_ports[index].dma_rx_buf_v,serial99xx_ports[index].dma_rx_buf_p);
		}else{
			serial99xx_ports[index].dma_rx=0;
			serial99xx_ports[index].dma_rx_buf_v=NULL;
		}	
		DEBUG("Befoer contxts index is %d\n",index);
		serial99xx_ports[index].uart_mode = uart_99xx_contxts[index].uart_mode;
		serial99xx_ports[index].flow_control = uart_99xx_contxts[index].en_flow_control;
		serial99xx_ports[index].flow_ctrl_type = uart_99xx_contxts[index].flow_ctrl_type;
		serial99xx_ports[index].x_on = uart_99xx_contxts[index].x_on;
		serial99xx_ports[index].x_off = uart_99xx_contxts[index].x_off;
		serial99xx_ports[index].ser_dcr_din_reg = 0;
		serial99xx_ports[index].ser_ven_reg = 0;
		serial99xx_ports[index].acr = 0;
		serial99xx_ports[index].lcr = 0;
		serial99xx_ports[index].mcr = 0;
		
		if(uart_99xx_contxts[index].uart_mode == MCS99XX_RS485_FULL_DUPLEX || \
  			uart_99xx_contxts[index].uart_mode == MCS99XX_RS485_HALF_DUPLEX || \
			uart_99xx_contxts[index].uart_mode == MCS99XX_RS485_HALF_DUPLEX_ECHO|| \
			uart_99xx_contxts[index].uart_mode == MCS99XX_RS422_MODE || \
			uart_99xx_contxts[index].uart_mode == MCS99XX_IRDA_MODE){
			
			serial99xx_ports[index].port.type = PORT_ENHANC;
			serial99xx_ports[index].rxfifotrigger = uart_99xx_contxts[index].rxfifotrigger;
			serial99xx_ports[index].txfifotrigger = uart_99xx_contxts[index].txfifotrigger;			
		}
		
		if(serial99xx_ports[index].flow_control){
			if (uart_99xx_contxts[index].flow_ctrl_type == MCS99XX_DTR_DSR_HW_FLOWCONTROL || \
				uart_99xx_contxts[index].flow_ctrl_type == MCS99XX_XON_XOFF_HW_FLOWCONTROL){
				serial99xx_ports[index].port.type = PORT_ENHANC;
				serial99xx_ports[index].rxfifotrigger = uart_99xx_contxts[index].rxfifotrigger;
				serial99xx_ports[index].txfifotrigger = uart_99xx_contxts[index].txfifotrigger;		
			}
		}

	}
//	up(&serial99xx_sem);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	return ret;
}

static struct pci_device_id serial99xx_pci_tbl[] = {
//	{PCI_VENDOR_ID_NETMOS, PCI_ANY_ID, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9900, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9950, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9904, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9922, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9912, PCI_SUBDEV_ID_MCS99XX, PCI_SUBVEN_ID_MCS99XX, 0, 0, 0},
	{0, },
};


//PCI driver remove function. Rlease the resources used by the port
static void __devexit serial99xx_remove_one(struct pci_dev *dev)
{
	int i,port_context,ret;
	unsigned long base;
	struct uart_99xx_port *uart=NULL;
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
//	mode = CASCADE_MODE;
        //DEBUG("mode is %d\n",mode);
	DEBUG("mode is %d\n",mode);
        for(port_context = 0;port_context < mode; port_context++)
{
	base = pci_resource_start(dev, FL_BASE0 + (port_context * 0x0003));
	//DEBUG("base is 0x%x\n",base);

	for (i = 0; i < UART99xx_NR; i++){
		if(serial99xx_ports[i].port.iobase == base){
			DEBUG("port removing is %d\n",i);
			uart=&serial99xx_ports[i];
			break;
		}
	}

	if(uart){
		//DEBUG("Enter into resource remove loop\n");
		//Free the IRQ
		if(port_context == 0){
			//DEBUG("irq removing for port %d\n",i);
			free_irq(uart->port.irq,uart);
		}
		DEBUG("membase=0x%x\n mapbase=0x%x\n",(unsigned int)uart->port.membase,(unsigned int)uart->port.mapbase);
		 DEBUG("value at address 3FC is %x\n",readl(uart->port.membase + 0x3FC));
                writel(1,uart->port.membase + 0x3FC);
                DEBUG("value at address 3FC after configuring is %x\n",readl(uart->port.membase + 0x3FC));
		down(&serial99xx_sem);
		//DEBUG("Before remove function\n");
		ret = uart_remove_one_port(&starex_serial_driver, &uart->port);
		DEBUG("ret val of uart_remove_one_port is %d \n",ret);
		//DEBUG("After remove function\n");
		//uart->port.dev = NULL;	
		uart->next_port = NULL;
		uart->prev_port = NULL;	
		up(&serial99xx_sem);
		//DEBUG("Before pci_free_consistent\n");	
		pci_free_consistent(dev,DMA_TX_BUFFER_SZ,uart->dma_tx_buf_v,uart->dma_tx_buf_p);
		pci_free_consistent(dev,DMA_RX_BUFFER_SZ,uart->dma_rx_buf_v,uart->dma_rx_buf_p);
		//DEBUG("After pci_free_consistent\n");
/*		DEBUG("Before pci_disable_device\n");
		if(i==0 || i==1)
			pci_disable_device(dev);
		DEBUG("After pci_disable_device\n");
*/
		//Initialise the uart_99xx_port arrays port specific element to the default state
		//DEBUG("Before assigning to default state\n");
		serial99xx_init_port(&serial99xx_ports[uart->port.line]);
		DEBUG("dev->is_busmaster is %d \n",dev->is_busmaster);
	}
}
	DEBUG("Before pci_disable_device\n");
	pci_disable_device(dev);
	DEBUG("After pci_disable_device\n");
	DEBUG("dev->is_busmaster after disable device is %d \n",dev->is_busmaster);
	DEBUG("In %s---------------------------------------END\n",__FUNCTION__);
}


//PCI drivers probe function
static int __devinit serial99xx_probe(struct pci_dev *dev, const struct pci_device_id *ent)
{
	int retval;
	unsigned long base, len;
	struct uart_port serial_port;
	int index=0,port_context,fun_no=0;
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	retval = pci_enable_device(dev);
        if (retval) {
		dev_err(&dev->dev, "Device enable FAILED\n");
                return retval;
	}

	//To verify wether it is a serial communication hardware
/*	if ((((dev->class >> 8) != PCI_CLASS_COMMUNICATION_SERIAL) &&
		((dev->class >> 8) != PCI_CLASS_COMMUNICATION_MODEM)) ||
		(dev->class & 0xff) > 6){
		DEBUG("Not a serial communication hardware\n");
		retval = -ENODEV;
		goto disable;
	}*/

	//To verify wether it is a MCS99XX type BARs
	if(((pci_resource_flags(dev,FL_BASE0) & BAR_FMT) ^ BAR_IO) || ((pci_resource_flags(dev,FL_BASE1) & BAR_FMT) ^ BAR_MEM) || ((pci_resource_flags(dev,FL_BASE4) & BAR_FMT) ^ BAR_MEM)) {
		DEBUG("Not a MCS99XX type device\n");
		retval = -ENOMEM;
		goto disable;
	}

	pci_set_master(dev);	

	memset(&serial_port, 0, sizeof(struct uart_port));

	index = serial99xx_find_match_or_unused(&serial_port);
	DEBUG("index is %d\n",index);

	//choose mode either as NORMAL_MODE or CASCADE_MODE
//	mode = CASCADE_MODE;
	DEBUG("mode is %d\n",mode);
	DEBUG("No of functions are %d\n",nr_funs);
	
	for(port_context = 0;port_context < mode; port_context++)
        {	
		DEBUG("In for loop\n");
	        if(port_context == (CASCADE_MODE-1))
			index = index + nr_funs;
		DEBUG("index in for loop %d\n",index);	
		memset(&serial_port, 0, sizeof(struct uart_port));
		serial_port.flags = UPF_SHARE_IRQ |UPF_SKIP_TEST;
		serial_port.uartclk = DEFAULT99xx_BAUD * 16;
		serial_port.irq = dev->irq;
//		serial_port.dev = &dev->dev;
		len =  pci_resource_len(dev, (FL_BASE1 + (port_context * 0x0003)));
		base = pci_resource_start(dev, (FL_BASE1 + (port_context * 0x0003)));
		serial_port.mapbase = base;
		serial_port.membase = ioremap(base,len);
		DEBUG("membase=0x%x\n mapbase=0x%x\n",(unsigned int)serial_port.membase,(unsigned int)serial_port.mapbase);
		DEBUG("value at address 3FC is %x\n",readl(serial_port.membase + 0x3FC));
	        writel(0,serial_port.membase + 0x3FC);
        	DEBUG("value at address 3FC after configuring is %x\n",readl(serial_port.membase + 0x3FC));
		base = pci_resource_start(dev,(FL_BASE0 + (port_context * 0x0003)));
		serial_port.iobase = base;
		DEBUG("iobase=0x%x\n",(unsigned int)serial_port.iobase);
		retval = serial99xx_register_port(&serial_port,index,port_context,dev);
		if (retval < 0){
			DEBUG(KERN_WARNING "Couldn't register serial port %s, retval=%d: \n", pci_name(dev),retval);
			goto disable;	
		}
		DEBUG("serial99xx_ports[index] : %x\n",&serial99xx_ports[index]);	

		if(port_context == 0)
                {
                        fun_no = index;
			DEBUG("index in is %d\n",index);
                        serial99xx_ports[index].next_port = NULL;
			serial99xx_ports[index].prev_port = NULL;
                }
                else
                {
			DEBUG("fun_no and index are %d %d\n",fun_no,index);
			
			serial99xx_ports[index].next_port = NULL;
			serial99xx_ports[index].prev_port =  &serial99xx_ports[fun_no];
                        serial99xx_ports[fun_no].next_port = &serial99xx_ports[index];
			DEBUG("serial99xx_ports[index] :%x serial99xx_ports[index].prev_port :%x and serial99xx_ports[fun_no].next_port :%x\n" ,&serial99xx_ports[index],serial99xx_ports[index].prev_port,serial99xx_ports[fun_no].next_port);
		}
	
		DEBUG("-------------serial port no is %d----------------\n",retval);
		DEBUG("Device structure for port %d is %u\n",index,&serial99xx_ports[index]);
	}
		DEBUG("IRQ value is %d\n",dev->irq);
		DEBUG("fun no is %d\n",fun_no);
		// Register an ISR
		#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
		if ((retval = request_irq(dev->irq, serial99xx_interrupt,SA_SHIRQ,"saturn-cascade-serial",&serial99xx_ports[fun_no]))) 
			  goto disable;
		#else
		if ((retval = request_irq(dev->irq, serial99xx_interrupt,IRQF_SHARED,"saturn-cascade-serial",&serial99xx_ports[fun_no]))) 
			  goto disable;
		#endif
		DEBUG("After interrupt request retval is %d\n",retval);

	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	return 0;	
	 
disable:
	pci_disable_device(dev);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	return retval;
}

	
static struct pci_driver starex_pci_driver = {
	.name		= "saturn-cascade-serial",
	.probe		= serial99xx_probe,
	.remove		= __devexit_p(serial99xx_remove_one),
	.id_table	= serial99xx_pci_tbl,
};


//Drivers entry function. register with the pci core and the serial core
static int __init serial99xx_init(void)
{
	int ret;

	DEBUG("In %s---------------------------------------START\n",__FUNCTION__);
	serial99xx_init_ports();
	ret = uart_register_driver(&starex_serial_driver);
        DEBUG("uart_register_driver is %d\n",ret);	
	if (ret){
		DEBUG("In %s uart_register_driver FAILED\n",__FUNCTION__);
		return ret;
	}	

	ret = pci_register_driver(&starex_pci_driver);
	DEBUG("pci_register_driver is %d\n",ret);
	if (ret < 0){
		DEBUG("In %s pci_register_driver FAILED\n",__FUNCTION__);
		uart_unregister_driver(&starex_serial_driver);
	}	
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);
	return ret;
}

//Drivers exit function. Unregister with the PCI core as well as serial core
static void __exit serial99xx_exit(void)
{
	DEBUG("In %s ---------------------------------------START\n",__FUNCTION__);
	pci_unregister_driver(&starex_pci_driver);
	uart_unregister_driver(&starex_serial_driver);
	DEBUG("In %s ---------------------------------------END\n",__FUNCTION__);	
}

module_param(test_mode,bool,0);
module_param(nr_funs,int,0);
//module_param(mode,int,0);

module_init(serial99xx_init);
module_exit(serial99xx_exit);

MODULE_DESCRIPTION("moschip 99xx serial driver module");
MODULE_SUPPORTED_DEVICE("moschip serial 99xx");
MODULE_LICENSE("GPL");
