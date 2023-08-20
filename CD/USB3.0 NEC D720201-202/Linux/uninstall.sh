#!/bin/sh

### Define variables

## Please change the xHCI driver module name defined in MODULE_NAME and MODULE_EXISTS 
## corresponding to your system. Default module is assumed "xhci".

#MODULE_NAME="xhci"
MODULE_NAME="xhci_hcd"
LINUX_VER=`uname -a | cut -d' ' -f 3`
MODULE_EXISTS=`cat /lib/modules/$LINUX_VER/modules.alias | grep $MODULE_NAME`
DEVICE_NAME="xhc201"
DRIVER_NAME="xhc201.ko"
DIVICE_MODE="666"
DEV_MINOR="0"
DEV_TYPE="c"


#Unload driver
/sbin/rmmod $DRIVER_NAME

#Delete special file
rm -f $DEVICE_NAME

#Reload xhci-hcd driver
if [ -z "$MODULE_EXISTS" ]; then
echo $MODULE_NAME not found.
else
modprobe $MODULE_NAME
fi
