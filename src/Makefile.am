AM_CFLAGS = -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = dvdbackup
dvdbackup_SOURCES = main.c \
	dvdbackup.c dvdbackup.h \
	gettext.h

dvdbackup_LDADD = $(LIBINTL)
