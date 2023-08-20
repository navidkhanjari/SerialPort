**********************************************
*                                            *
* Maxtor SATA150 TX Series Driver Diskette  *
*                                            *
**********************************************

This driver version is used on all Maxtor branded Serial ATA hosts.
Version 1.00.0050.3 offers support for Windows 98/Millennium,
NT 4.x, Win 2K, Win 2003 Server, and Windows XP operating systems. 
This driver supports digital signatures for Microsoft operating systems.

For the latest software, and driver updates please visit our web site
at www.maxtor.com

For the latest technical support documentation, please visit our 
knowledgebase at www.maxtorkb.com

      If you are installing drivers during a Windows NT4.x,Win 2K or XP
      fresh Operating System install, you will need to copy the Serial ATA 
      driver files from the MaxBlast 3 CD to a clean formatted floppy.

      From the MaxBlast 3 CD choose the create Maxtor Ultra ATA driver diskette
      option and follow the on screen prompts.

      On new build NT,2K, and XP systems without floppy drives, it will be 
      necessary to temporarily connect a 3.5 floppy drive to install the SATA 
      host drivers.This is because the Microsoft operating system doesn't allow 
      browsing to CD ROM drives during a new install.
     
      On systems that have NT, 2K, or XP already installed. Add the SATA host
      and driver to the system first before connecting the new SATA drive to the 
      system.      
  
      SATA controllers report as a SCSI device in Windows:
      During the Windows NT4.x,Win2K,or WinXP install you must specify additional 
      devices SCSI or RAID controllers, and choose other to install the driver 
      from the floppy when prompted by the operating system. 


====================
Contents on the disk
====================
README   TXT            This readme file
ULSATA                  Microsoft Windows miniport disk label file
TXTSETUP OEM            Microsoft Windows miniport text mode setup file

WINXP        <DIR>
  ULSATA   SYS          Microsoft WindowsXP miniport driver
  PTIPBM   DLL          Promise WindowsXP set Pbm dynamic link library
  ULSATA   INF          Microsoft Windows miniport setup file

WNET         <DIR>
  ULSATA   SYS          Microsoft Windows Server 2003 miniport driver
  PTIPBM   DLL          Promise Windows Server 2003 set Pbm dynamic link library
  ULSATA   INF          Microsoft Windows miniport setup file

WIN2000        <DIR>
  ULSATA   SYS          Microsoft Windows2000 miniport driver
  PTIPBM   DLL          Promise Windows2000 Set Pbm dynamic link library
  ULSATA   INF          Microsoft Windows miniport setup file

WIN98-ME       <DIR>
  ULSATA   MPD          Microsoft Windows98-ME miniport driver
  ULSATA   INF          Microsoft Windows miniport setup file
  PTISTP   DLL          Advanced INF file format support file
  PU66VSD  VXD          Promise power management driver
  SMARTVSD VXD          Promise S.M.A.R.T. driver

NT4            <DIR>
  ULSATA   SYS          Microsoft WindowsNT miniport driver file
  PTIPBM   EXE          Promise WindowsNT Set Pbm execution file
  ULSATA   INF          Microsoft Windows miniport setup file

================
Software Version
================
Microsoft Windows NT miniport driver     		1.00.0050.3
Microsoft Windows 95-98 miniport driver  		1.00.0050.3
Microsoft Windows 2000 miniport driver   		1.00.0050.3
Microsoft Windows XP miniport driver     		1.00.0050.3
Microsoft Windows Server 2003 miniport driver	1.00.0050.3
