#!/bin/sh

### Define variables

## Please change the xHCI driver module name defined in MODULE_NAME and MODULE_EXISTS 
## corresponding to your system. Default module is assumed "xhci".

#MODULE_NAME="xhci"
MODULE_NAME="xhci_hcd"
MODULE_EXISTS=`cat /proc/modules | grep $MODULE_NAME`
DEVICE_NAME="xhc201"
DRIVER_NAME="xhc201.ko"
DEVICE_MODE="666"
DEV_MINOR="0"
DEV_TYPE="c"

#If the xhci-hcd driver is loaded, it stops
if [ -z "$MODULE_EXISTS" ]; then
echo $MODULE_NAME is not found.
else
rmmod $MODULE_NAME
fi

#Load the driver for the SROM tool
/sbin/insmod ./$DRIVER_NAME

#Get the major device number of the driver
DEV_MAJOR=`cat /proc/devices | grep $DEVICE_NAME | cut -c1-3`

#Make special file for driver
mknod -m $DEVICE_MODE $DEVICE_NAME $DEV_TYPE $DEV_MAJOR $DEV_MINOR

