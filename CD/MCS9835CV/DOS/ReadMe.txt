MosChip DOS installer version 1.00.025 dated 19-NOV-2006.
*************************************************
Usage : NmDosIn [<options>]
By default, NmDosIn displays COM/LPT ports.

Options :
*******
   -a[dd]                             = Add Moschip Ports
   -c[onfig]:<parameters>    = Configure Moschip Ports
   -d[elete]                          = Delete Moschip Ports
   -r[emap]                          = Remap Moschip ports to legacy ports
   -s[ilent]                            = Silent mode
   -t[est]                              = Test ports (requires external Loop-Back plugs)
   -u[nused]                         = remap to all Unused legacy port i/o base, 
                                              without consideration of COM/LPT number
   -f                                     = remapping to customize I/O addresses.

Procedure for Installing and Re-mapping the Moschip ports:
*********************************************
Step 1: Type "nmdosin -a" to add moschip ports.
Step 2; For remapping moschip ports to legacy ports type :nmdosin -a -f"

Steps to remap the customized I/O address:
*********************************

Step 1: Copy "NMCONFIG.CFG and NMDOSIN.EXE" files to Hard disk.
Step 2: Type "edit nmconfig.cfg" and change I/O address as per your requirement. Save the file and close it.
Step 3: Use "nmdosin -a -f" to configure port.

