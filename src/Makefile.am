AM_CFLAGS = -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = dvdbackup
dvdbackup_SOURCES = main.c \
	find-sector.c find-sector.h \
	dvdbackup.c dvdbackup.h \
	gettext.h

dvdbackup_CFLAGS = -DFIND_UNUSED $(AM_CFLAGS) $(DEPS_CFLAGS)
dvdbackup_LDADD = $(LIBINTL) -ldvdvm $(DEPS_LIBS)
