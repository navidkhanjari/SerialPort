���ģ�
------------------------------------------------------------------------------------------
ͨ��DOS����ͨ��BIOS�����ܲ����κδ���(INT 14H)�Ͳ���(INT 17H)������DOS������ֻ֧��4������
(3F8h, 2F8h, 3E8h, 2E8h)��3������(3BCh/378h/278h)��

I.�ļ�˵��:
1.CH35XDOS.EXE		-- DOS������װ����,֧��2S,2S1P,1S1P�� 
2.ReadMe.txt		-- DOS������װ����˵���ĵ�

II.ѡ��:
-a			-- ����CH35X�˿�
-r			-- CH35X�˿���ӳ�䵽��Ӧ��ϵͳ�����Ķ˿�
-d			-- ɾ�����ӵ�CH35X�˿�
-s			-- ���ذ�װ������ʾ��װ��Ϣ
-h			-- ��ʾ��װѡ��
III.��װ����:
1. -- ��CH35XDOSͨ������������DOSϵͳ. 
2. -- ����"CH35XDOS"ȷ���Ƿ��⵽�豸. 
   -- �����⵽������ʾ: 
  Find CH353 2 Serial 1 Parallel Card
  Serial Port1 at 9000 (IRQ9)
  Serial Port2 at 9400 (IRQ9)
  PrinterPort1 at 9800 (IRQ9)
3. -- ����"CH35XDOS -a" ,���س�,��CH35XDOS�˿ڰ�װ��ϵͳ,��ʾ����:
  ===========================
  COM1 exists at 3FB
  COM2 exists at 2FB
  COM3 added  at 9000 (IRQ9)
  COM4 added  at 9400 (IRQ9)
  LPT1 exists at 378
  LPT2 added  at 9800 (IRQ9)
  ===========================
4. -- ����"CH35XDOS -a -r" ,���س�,��CH35XDOS�˿ڰ�װ��ϵͳ�����Ķ˿�,��ʾ����:
   ===========================
  COM1 exists at 3FB
  COM2 exists at 2FB
  COM3 added  at 3EB (IRQ9)
  COM4 added  at 2EB (IRQ9)
  LPT1 exists at 378
  LPT2 added  at 278 (IRQ9)
  ===========================
5. -- �����ÿ������ʱ��װ������AUTOEXEC.BAT������һ��:
  CH35XDOS -a -s(����CH35XDOS -a -r -s)

==========================================================================================

ENGLISH:
------------------------------------------------------------------------------------------
DOS OS operation any serial port (INT 14H) and parallel (INT 17H) by BIOS service commonly,
BUT DOS OS can only support 4 serial port (3F8h, 2F8h, 3E8h, 2E8h) and 3 parallel port
(3BCh/378h/278h).

I. File explain:
1.CH35XDOS.EXE		-- The tool of install the driver support 2S/2S1P/1S1P card for DOS
2.ReadMe.txt		-- The document to explain how to install the driver 

II. Option:
-a			-- Add port of CH35X
-r			-- The por of CH35X remapped to legacy port of DOS OS
-d			-- Delete the add port of CH35X
-s			-- Hidden install, show no install information
-h			-- Clue on install option

III. Install approach:
1. -- Copy the CH35XDOS to DOS OS.
2. -- Input "CH35XDOS" to check the device.
   -- If the card has found, it will display:
  Find CH353 2 Serial 1 Parallel Card
  Serial Port1 at 9000 (IRQ9)
  Serial Port2 at 9400 (IRQ9)
  PrinterPort1 at 9800 (IRQ9)
3. -- Input "CH35XDOS -a", then press "Enter", install the port of CH35XDOS to DOS OS, 
      it will display:
  ===========================
  COM1 exists at 3FB
  COM2 exists at 2FB
  COM3 added  at 9000 (IRQ9)
  COM4 added  at 9400 (IRQ9)
  LPT1 exists at 378
  LPT2 added  at 9800 (IRQ9)
  ===========================
4. -- Input "CH35XDOS -a -r", then press "Enter", install the port of CH35XDOS to legacy
      port of DOS OS, it will display:
  ===========================
  COM1 exists at 3FB
  COM2 exists at 2FB
  COM3 added  at 3EB (IRQ9)
  COM4 added  at 2EB (IRQ9)
  LPT1 exists at 378
  LPT2 added  at 278 (IRQ9)
  ===========================
5. -- If you want install the driver at Reboot every time, please add this line into AUTOEXEC.BAT:
   CH35XDOS -a -s ( or CH35XDOS -a -r -s)