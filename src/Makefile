#!/usr/bin/make -f

COPTS = -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64

CFLAGS = -Wall $(COPTS)

LDFLAGS = -ldvdread

dvdbackup: dvdbackup.c
	$(CC) $(CFLAGS) $(COPTS) $(LDFLAGS) -o $@ $<

clean:
	rm -f dvdbackup

.PHONY: clean dvdbackup
