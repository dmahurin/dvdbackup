CFLAGS =  -g -Isrc/inc -Iinc -Iinc/dvdvm
#-DTRACE

src/%.o: src/%.c
	gcc -c $(CFLAGS)  $< -o $@

OBJECTS = src/decoder.o  src/vm.o  src/vmcmd.o

all: src inc
	make lib/libdvdvm.a

src inc:
	tar -xjf libdvdnav-4.1.3.tar.bz2  libdvdnav-4.1.3/src/vm
	patch -p1 -d libdvdnav-4.1.3 < libdvdnav-4.1.3-vm.patch
	mkdir -p src
	mkdir -p inc/dvdvm
	mv libdvdnav-4.1.3/src/vm/*.c src
	mv libdvdnav-4.1.3/src/vm/*.h inc/dvdvm
	rm -rf libdvdnav-4.1.3
	cp -dpr stub-inc/*.h inc/dvdvm

lib/libdvdvm.a: $(OBJECTS)
	mkdir -p lib
	ar r $@ $(OBJECTS)

vmtest: vmtest.c
	gcc -g -Llib -Iinc vmtest.c -ldvdvm -ldvdread -o vmtest

clean:
	rm -f lib/libdvdvm.a vmtest $(OBJECTS)

distclean: clean
	rm -rf lib src inc
