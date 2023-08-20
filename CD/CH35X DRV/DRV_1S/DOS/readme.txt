中文：
------------------------------------------------------------------------------------------
通常DOS环境通过BIOS服务能操作任何串口(INT 14H)，但是DOS环境里只支持4个串口
(3F8h, 2F8h, 3E8h, 2E8h)。

I.文件说明:
1.CH35SDOS.EXE		-- DOS驱动安装工具,支持1S卡
2.ReadMe.txt		-- DOS驱动安装工具说明文档

II.选项:
-a			-- 添加CH35X端口
-r			-- CH35X端口重映射到相应的系统保留的端口
-d			-- 删除添加的CH35X端口
-s			-- 隐藏安装，不提示安装信息
-h			-- 提示安装选项
III.安装步骤:
1. -- 把CH35SDOS通过软驱拷贝到DOS系统. 
2. -- 输入"CH35SDOS"确定是否检测到设备. 
   -- 如果检测到卡会显示: 
  Find CH352 1 Serial Card
  Serial Port1 at 9000 (IRQ9)
3. -- 输入"CH35SDOS -a" ,按回车,把CH35SDOS端口安装到系统,显示如下:
  ===========================
  COM1 exists at 3FB
  ===========================
4. -- 输入"CH35SDOS -a -r" ,按回车,把CH35SDOS端口安装到系统保留的端口,显示如下:
   ===========================
  COM1 exists at 3FB
  ===========================
5. -- 如果想每次重启时安装，请在AUTOEXEC.BAT中添加一行:
  CH35SDOS -a -s(或者CH35SDOS -a -r -s)

==========================================================================================

ENGLISH:
------------------------------------------------------------------------------------------
DOS OS operation any serial port (INT 14H) and parallel (INT 17H) by BIOS service commonly,
BUT DOS OS can only support 4 serial port (3F8h, 2F8h, 3E8h, 2E8h) and 3 parallel port
(3BCh/378h/278h).

I. File explain:
1.CH35SDOS.EXE		-- The tool of install the driver support 1S card for DOS 
2.ReadMe.txt		-- The document to explain how to install the driver 

II. Option:
-a			-- Add port of CH35X
-r			-- The por of CH35X remapped to legacy port of DOS OS
-d			-- Delete the add port of CH35X
-s			-- Hidden install, show no install information
-h			-- Clue on install option

III. Install approach:
1. -- Copy the CH35SDOS to DOS OS.
2. -- Input "CH35SDOS" to check the device.
   -- If the card has found, it will display:
  Find CH352 1 Serial Card
  Serial Port1 at 9000 (IRQ9)
3. -- Input "CH35SDOS -a", then press "Enter", install the port of CH35SDOS to DOS OS, 
      it will display:
  ===========================
  COM1 exists at 3FB
  ===========================
4. -- Input "CH35SDOS -a -r", then press "Enter", install the port of CH35SDOS to legacy
      port of DOS OS, it will display:
  ===========================
  COM1 exists at 3FB
  ===========================
5. -- If you want install the driver at Reboot every time, please add this line into AUTOEXEC.BAT:
   CH35SDOS -a -s ( or CH35SDOS -a -r -s)
