#!/usr/bin/make -f

dvdbackup: 
	cd src && make dvdbackup

clean:
	cd src && make clean

.PHONY: clean dvdbackup
