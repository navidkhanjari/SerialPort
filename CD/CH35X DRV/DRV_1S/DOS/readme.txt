���ģ�
------------------------------------------------------------------------------------------
ͨ��DOS����ͨ��BIOS�����ܲ����κδ���(INT 14H)������DOS������ֻ֧��4������
(3F8h, 2F8h, 3E8h, 2E8h)��

I.�ļ�˵��:
1.CH35SDOS.EXE		-- DOS������װ����,֧��1S��
2.ReadMe.txt		-- DOS������װ����˵���ĵ�

II.ѡ��:
-a			-- ���CH35X�˿�
-r			-- CH35X�˿���ӳ�䵽��Ӧ��ϵͳ�����Ķ˿�
-d			-- ɾ����ӵ�CH35X�˿�
-s			-- ���ذ�װ������ʾ��װ��Ϣ
-h			-- ��ʾ��װѡ��
III.��װ����:
1. -- ��CH35SDOSͨ������������DOSϵͳ. 
2. -- ����"CH35SDOS"ȷ���Ƿ��⵽�豸. 
   -- �����⵽������ʾ: 
  Find CH352 1 Serial Card
  Serial Port1 at 9000 (IRQ9)
3. -- ����"CH35SDOS -a" ,���س�,��CH35SDOS�˿ڰ�װ��ϵͳ,��ʾ����:
  ===========================
  COM1 exists at 3FB
  ===========================
4. -- ����"CH35SDOS -a -r" ,���س�,��CH35SDOS�˿ڰ�װ��ϵͳ�����Ķ˿�,��ʾ����:
   ===========================
  COM1 exists at 3FB
  ===========================
5. -- �����ÿ������ʱ��װ������AUTOEXEC.BAT�����һ��:
  CH35SDOS -a -s(����CH35SDOS -a -r -s)

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
