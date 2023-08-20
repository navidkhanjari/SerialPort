/*
 *                                                                                      	
 *                      Port Information Dump Program                     
 *                                                                                      
 *				Copyright 2010 WCH GROUP all right reserved
 *                                                                                        
 *                                                              Version: 1.0.0.0           	
 *                                                              Date: 2010/09/11        
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>


#define WCH_SER_TOTAL_MAX	32


#define WCH_IOCTL  0x900
#define WCH_SER_DUMP_PORT_INFO  (WCH_IOCTL + 50)
#define WCH_SER_DUMP_PORT_PERF  (WCH_IOCTL + 51)
#define WCH_SER_DUMP_DRIVER_VER (WCH_IOCTL + 52)


#define WCH_BOARDNAME_LENGTH  		15
#define WCH_DRIVERVERSION_LENGTH 	15

    
struct wch_ser_port_info 
{
 	char  			board_name_info[WCH_BOARDNAME_LENGTH];
 	unsigned int  	bus_number_info;
 	unsigned int  	dev_number_info;
 	unsigned int  	port_info;
 	unsigned int 	base_info;		
  	unsigned int  	irq_info;	
};

struct wch_info
{
 	struct wch_ser_port_info 	ser_port_info[WCH_SER_TOTAL_MAX];
	
	char 						driver_ver[WCH_DRIVERVERSION_LENGTH];
	
	int 						ser_count;
	
	int 						ser_status1;
	int							ser_status2;
};


static int get_ser_port_info(struct wch_info *infop)
{
	char ser_name[14] = "/dev/ttyWCH32";
	int ser_fd;
	
	ser_fd = open(ser_name, O_RDWR | O_NOCTTY);

	if (ser_fd > 0)
	{
		infop->ser_status1 = ioctl(ser_fd, WCH_SER_DUMP_PORT_INFO, &infop->ser_port_info);
		infop->ser_status2 = ioctl(ser_fd, WCH_SER_DUMP_DRIVER_VER, &infop->driver_ver);
		close(ser_fd);
		return 0;
	}
	
	return -ENXIO;
}

static void print_info(struct wch_info *infop)
{
	int i;
	
	system("clear");
	
	for (i = 0; i < WCH_SER_TOTAL_MAX; i++)
	{
		if (infop->ser_port_info[i].base_info)
		{
			infop->ser_count++;
		}
	}    
    
    
	if (infop->ser_count > 0)
	{
	    printf("\n================ Found %2d WCH port , list informations ====================\n", infop->ser_count);
	    if (infop->ser_status2 == 0)
		    printf("                                             WCH driver ver -- %s\n\n", infop->driver_ver);
	    else
		    printf("\n");

	    for (i = 0; i < WCH_SER_TOTAL_MAX; i++)
	    {
		    if (!(infop->ser_port_info[i].base_info))
			    continue;
	
		    printf("ttyWCH%d --\n", infop->ser_port_info[i].port_info); 
		    printf("WCH %s Series (bus:%d device:%2d) , base address = %4x,    irq = %2d\n\n",
				    infop->ser_port_info[i].board_name_info,
				    infop->ser_port_info[i].bus_number_info,
				    infop->ser_port_info[i].dev_number_info,
				    infop->ser_port_info[i].base_info,
				    infop->ser_port_info[i].irq_info
				    );
	    }
	}
	else
	{
        printf("\n============ No any port been found, type 'lsmod' to check driver ===========\n\n");
	}

	printf("=============================================================================\n");
}


int main(void)
{
	int status = 0;
	struct wch_info info;
	
	memset(&info, 0, (sizeof(struct wch_info)));
	
	status = get_ser_port_info(&info);
	if (status < 0)
	{
	}

	print_info(&info);
	
 	return 0;
}

