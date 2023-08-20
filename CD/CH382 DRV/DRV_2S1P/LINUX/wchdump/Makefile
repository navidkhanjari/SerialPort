# Copyright 2010 WCH GROUP all right reserved 
#CC=gcc -fmessage-length=0 -Wall -D_FORTIFY_SOURCE=2 -fstack-protector -O2 -pipe -o
CC=gcc -Wall -O2 -pipe -o

all: wchdump

install: wchdump
	cp -p wchdump /usr/bin


wchdump: wchdump.c
	$(CC) wchdump wchdump.c	


clean:  
	rm -f wchdump
	rm -f *~ *.o

