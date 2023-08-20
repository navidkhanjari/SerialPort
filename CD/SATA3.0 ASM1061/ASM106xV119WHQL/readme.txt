************************************************************
************************************************************
*  Installation Readme for Asmedia 106x SATA Host Controller Drive.
*
*  Refer to the system requirements for the operating systems
*  supported by Asmedia 106x SATA Host Controller Drive.
*
************************************************************
************************************************************

************************************************************
*  CONTENTS OF THIS DOCUMENT
************************************************************

This document contains the following sections:

1.  Overview
2.  System Requirements
3.  Language Support
4.  Determining the System Mode
5.  Installing the Software
6.  Verifying Installation of the Software
7.  Advanced Installation Instructions
8.  Identifying the Software Version Number
9.  Uninstalling the Software

************************************************************
* 1.  OVERVIEW
************************************************************

The Asmedia 106x SATA Host Controller Drive is designed to provide 
functionality for the following Storage Controllers:

	- Asmedia 106x SATA Controller
        - Asmedia 106x SATA Controller

Software installation is flexible and fully automated.

************************************************************
* 2.  SYSTEM REQUIREMENTS
************************************************************

1.  The system must contain one of the following ASMedia products:

	- Asmedia 106x SATA Controller	 

2.  The system must be running on one of the following
    operating systems:
    - Microsoft* Windows* XP Home Edition 
    - Microsoft* Windows* XP Professional
    - Microsoft* Windows* XP x64 Edition  
    - Microsoft* Windows* Server 2003 
    - Microsoft* Windows* Server 2003 x64 Edition  

	  - Microsoft* Windows* Vista
	  - Microsoft* Windows* Vista x64 Edition
	  - Microsoft* Windows* Server 2008
	  - Microsoft* Windows* Server 2008 x64 Edition
	  - Microsoft* Windows* Win7
	  - Microsoft* Windows* Win7 x64 Edition	  	  

3. The following operating systems are not supported:

    Any version of the following Microsoft operating systems:
    - MS-DOS
    - Windows 3.1
    - Windows NT 3.51
    - Windows 95
    - Windows 98
    - Windows Millennium Edition (Me)
    - Windows NT 4.0
    - Windows 2000 Datacenter Server

    Any version of the following operating systems:
    - Linux
    - UNIX
    - BeOS
    - MacOS
    - OS/2

4.  The system should contain at least the minimum system 
    memory required by the operating system.

************************************************************
* 3.  LANGUAGE SUPPORT
************************************************************

Below is a list of the languages (and their abbreviations)
for which the Asmedia 106x SATA Host Controller Drive software has 
been localized. The language code is listed in parentheses 
after each language.

enu -> English (United States) (0409)
chs -> Chinese (Simplified)    (0804)
cht -> Chinese (Traditional)   (0404)


************************************************************
* 4.  DETERMINING THE SYSTEM MODE
************************************************************

To use this readme effectively, you will need to know what 
mode your system is in. The easiest way to determine the 
mode is to identify how the Serial ATA controller is 
presented within the Device Manager. The following 
procedure will guide you through determining the mode.

1.  On the Start menu, select Control Panel.

2.  Open on the 'System' applet (you may first 
    have to select 'Switch to Classic View').

3.  Select the 'Hardware' tab. 

4.  Select the 'Device Manager' button.

5.  From the Device Manager, look for an entry named 
    'SCSI/RAID Controllers'.
    If this entry is present, expand it and look for one of
    the following controllers:

    - 'Asmedia 106x SATA Controller'
      If the Controller above is present, no further action
      is required.

     - If neither of the controllers above are shown, then
    the system is not running in RAID mode and you should 
    continue with step 6 below.

6.  From the Device Manager, look for an entry named 
    'IDE ATA/ATAPI controllers'.
    If this entry is present, expand it and look for one of
    the following controllers:

    - 'Asmedia 106x SATA Controller'
      If the Controller above is present, no further action
      is required.
   
    - If this controller is not present, then your system
    is not in AHCI mode. No other modes are supported by 
    the Asmedia 106x SATA Host Controller Drive software and you
    should continue with step 7 below.

7.  Your system does not appear to be running in RAID or 
    AHCI mode. If you feel that your system is running in 
    RAID or AHCI mode and you do not see any of the 
    controllers listed above, you may choose to contact 
    your system manufacturer or place of purchase for 
    assistance.


************************************************************
* 5.  INSTALLING THE SOFTWARE
************************************************************

5.1 General Installation Notes

1.  If you are installing the operating system on a system
    configured for RAID or AHCI mode, you must pre-install 
    the Asmedia 106x SATA Host Controller Drive driver using the 
    F6 installation method described in section 5.3; 
    
    If you are installing this driver on a system configured 
    for IDE mode, you need do the following steps to finish
    the installing:
    
    a) double-click on the self-extracting and self-installing
    	 setup file and answer all prompts presented
    	 
   	b) enter the following path: 
   			
   			<bootdriver>\Program Files\ASM106xSATA\Driver
   		 
   		 you can find a directory named "PREPARE",  open it
   			
   	c) double-click the script "install.cmd"
   	
   	d) reboot system, and switch to AHCI mode, operating system
   		 will automatic installed this drivr.   	

2.  To install the Asmedia 106x SATA Host Controller Drive, 
    double-click on the self-extracting and self-installing 
    setup file and answer all prompts presented.

3.  By default, all installed files (.inf, .sys, .cat) 
    are copied to the following path: 
    
    <bootdrive>\Program Files\ASM106xSATA\Driver

		Note: after install complete, you need doing follow steps 
					to keep it work on WIN7 x64 Edition or Win Server 2008
					x64 Edition:
					
					3.1 reboot the system
					
					3.2 press F8 on bootup menu
					
					3.3 select "Disable Driver Signature Enforcement",
							enter ENTER and go on
	
5.2 InstallShield* Installation from Hard Drive or CD-ROM
    
Note: This method is applicable to systems configured for 
      RAID or AHCI mode.

1.  Download the Asmedia 106x SATA Host Controller Drive setup file
		(Setup.exe)and double-click to self-extract and to begin
		the setup process.

2.  The 'Choose Language' window appears. Choose the language
		and Click on the Next button to continue.

3.  If the InstallShield* Wizard Complete window is shown 
    without a prompt to restart the system, click the Finish
    button and proceed to step 4. If it is shown with a 
    prompt to restart the system, click 'Yes, I want to 
    restart my computer now.' (the default selection) and 
    click the Finish button.  Once the system has restarted,
    proceed to step 4.

4.  To verify that the driver was loaded correctly, refer 
    to Section 6.
    
5.3 Pre-Installation Using the F6 Method. 

Note: These methods are applicable to systems configured for
      RAID or AHCI mode.

1.  Extract all driver files from the installation package.
    See section 7.2 for instructions on extracting the 
    files.

2.  Create a floppy* containing the following files in the root 
    directory:
    for XP, Server2003:
    		asahxp32.inf, asahxp32.cat, asahxp32.sys, txtSetup.oem
    
    for XP x64 edition, Server2003 x64 edition:
    		asahxp64.inf, asahxp64.cat, asahxp64.sys, txtSetup.oem
    
    for Vista, Server2008, Win7:
    		asahci32.inf, asahci32.cat, asahci32.sys
    
    for Vista x64 edition, Server2008 x64 edition, Win7 x64 edition:
    		asahci64.inf, asahci64.cat, asahci64.sys

*Note: For Windows Vista you can use Floppy, CD/DVD or USB.

3.  For Windows XP or later operating systems:

    -At the beginning of the operating system installation,
     press F6 to install a third party SCSI or RAID driver.

    -When prompted, select 'S' to Specify Additional Device.

    -continue to step 5.

4.  For Windows Vista:

    -During the Operating system installation, after selecting the 
     location to install Vista click on "load Driver" button to 
     install a third party SCSI or RAID driver.

    -Continue to step 5.

5.  When prompted, insert the floppy disk or media (Floppy, CD/DVD
    or USB) you created in step 2 and press Enter.

6.  At this point you should be presented with a selection
    for one of the following depending on your hardware
    version and configuration:

     - Asmedia 106x SATA Controller
 

7.  Highlight the selection that is appropriate for the 
    hardware in your system and press Enter.

8.  Press Enter again to continue. Leave the floppy disk in 
    the system until the next reboot as the software will 
    need to be copied from the floppy disk again when setup
    is copying files.

************************************************************
* 6.  VERIFYING INSTALLATION OF THE SOFTWARE
************************************************************

6.1 Verifying Hard Disk, F6, or Unattended Installation: 
    depending on your system configuration, refer to the 
    appropriate sub-topic below: 

	  Note: Systems must Configured for AHCI Mode:

1.  On the Start menu, select 'Control Panel'

2.  Open on the 'System' applet (you may first 
    have to select 'Switch to Classic View')
    
3.  Select the 'Hardware' tab

4.  Select the 'Device Manager' button.

5.  For Vista, Server 2008, Win7
			Expand the 'IDE ATA/ATAPI Controllers' entry
		
		For XP, Server 2003
			Expand teh 'SCSI/RAID Controllers' entry

6.  Right-click on 'ASMedia 106x SATA Controller'

7.  Select 'Properties'

8.  Select the 'Driver' tab

9.  Select the 'Driver Details' button

10. follow file shoud displayed, then installation was 
		successful:
		
		-'asahxp32.sys' for XP, Server2003
		-'asahxp64.sys' for XP x64 edition, Server2003 x64 edition
		-'asahci32.sys' for Vista, Server2008, Win7
		-'asahci64.sys' for Vista x64 edition,  Server2008 x64 edition,
											  Win7 x64 edition  


************************************************************
* 7.  ADVANCED INSTALLATION INSTRUCTIONS
************************************************************

7.1 Available Setup Flags:

    /?             The installer presents a dialog showing
		   						 all the supported setup flags (shown here)
		   						 and their usage. 
    /L             specify the language ID
    /S             silent install (no user prompts)
    /V						 specify the windows msiexec.exe command line
    /X						 uninstall the driver

7.2 Silent install/uninstall example

	install:  	setup.exe /s /v/qn   
							or 
							setup.exe /s /v"/qn /forcerestart" 
							
	uninstall: 	setup.exe /s /x /v/qn 
							or 
							setup.exe /s /x /v"/qn /forcerestart"

Notes:  Flags and their parameters are not case-sensitive.  
        Flags may be supplied in any order, with the 
				exception of the -S and -L, which must be 
				supplied last.  

************************************************************
* 8.  IDENTIFYING THE SOFTWARE VERSION NUMBER
************************************************************

8.1 Use the following steps to identify the software version 
    number following a Have Disk, F6, or unattended 
    installation.

		Note: Systems must Configured for AHCI Mode:

1.  On the Start menu, select Control Panel.

2.  Open on the 'System' applet (you may first  have to select
    'Switch to Classic View').
    
3.  Select the 'Hardware' tab 

4.  Select the 'Device Manager' button

5.  For Vista, Server 2008, Win7
			Expand the 'IDE ATA/ATAPI Controllers' entry
		
		For XP, Server 2003
			Expand teh 'SCSI/RAID Controllers' entry

6.  Right-click on 'ASMedia 106x SATA Controller' 

7.  Select 'Properties'

8.  Select the 'Driver' tab

9.  The software version is displayed after 'Driver Version'

************************************************************
* 9.  UNINSTALLING THE SOFTWARE
************************************************************

9. UNINSTALLATION NON-DRIVER COMPONENTS
	Removal of this software from the system will render any 
	Serial ATA hard drives inaccessible by the operating system;
	therefore, uninstallation procedure will only uninstall 
	non-critical components of this software (user interface, 
	start menu links, etc.). 

Use the following procedure to uninstall the software:

1. Select 'Uninstall' from the following steps:
		- Go to "Start"  
		- Select "Control Panel" 
		- Select "Add or Remove Programs"
		- Find "Asmedia ASM106x SATA Host Controller Driver",
		  select it and click the "Remove" button.
	
	 you can also use the following procedure to unisntall the
	 software:
	 	- Go to "Start"
	 	- Select "Program"
	 	- Select "Asmedia Technology"
	 	- Select "ASM106x SATA Driver"
	 	- Click shortcut "uninstall"

2. The uninstall program will start. Click through the
   options for the uninstallation.

************************************************************

* Third-party brands and names may be claimed as the 
  property of others.

Copyright (c) ASMedia Technology, 2010
************************************************************

