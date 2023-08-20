# Copyright 2010 WCH GROUP all right reserved 
all: driver_make wchdump_make wchterm_make

install: driver_install wchdump_install wchterm_install wchmknod_install 

clean: driver_clean wchdump_clean wchterm_clean wchmknod_clean local_clean


driver_make:
	cd driver;\
	make

driver_install:
	cd driver;\
	make install

driver_clean:
	cd driver;\
	make clean
	
wchdump_make:
	cd wchdump;\
	make

wchdump_install:
	cd wchdump;\
	make install

wchdump_clean:
	cd wchdump;\
	make clean;

wchterm_make:
	cd wchterm;\
	make

wchterm_install:
	cd wchterm;\
	make install

wchterm_clean:
	cd wchterm;\
	make clean;

wchmknod_install:
	cd wchmknod;\
	./wchmknod

wchmknod_clean:
	cd wchmknod;\
	rm -f *~



local_clean:
	rm -f *~
