dnl Process this file with autoconf to produce a configure script

dnl ------------------------------------------------
dnl Initialization and Versioning
dnl ------------------------------------------------

AC_INIT([dvdbackup], [0.1.1-svn])

# Where to generate output; srcdir location.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])

dnl Must come before AM_INIT_AUTOMAKE.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2 -Wall])

dnl ------------------------------------------------
dnl Information on the package
dnl ------------------------------------------------

dnl none

dnl ------------------------------------------------
dnl Checks for programs
dnl ------------------------------------------------

AC_PROG_CC

dnl ------------------------------------------------
dnl Set build flags based on environment
dnl ------------------------------------------------

# use -pedantic -Wall -Wextra when possible
AX_CFLAGS_GCC_OPTION(-pedantic)
AX_CFLAGS_GCC_OPTION(-Wall)
AX_CFLAGS_GCC_OPTION(-Wextra)

dnl ------------------------------------------------
dnl Checks for libraries
dnl ------------------------------------------------

AC_CHECK_LIB(dvdread, DVDOpen, , AC_MSG_ERROR([You need libdvdread]))

dnl ------------------------------------------------
dnl Checks for header files
dnl ------------------------------------------------

AC_CHECK_HEADER(dvdread/dvd_reader.h, , AC_MSG_ERROR([You need libdvdread (dvd_reader.h)]))

dnl ------------------------------------------------
dnl Checks for types
dnl ------------------------------------------------

dnl none

dnl ------------------------------------------------
dnl Checks for structures
dnl ------------------------------------------------

dnl none

dnl ------------------------------------------------
dnl Checks for compilier characteristics
dnl ------------------------------------------------

dnl none

dnl ------------------------------------------------
dnl Checks for library functions
dnl ------------------------------------------------

dnl none

dnl ------------------------------------------------
dnl Checks for system services
dnl ------------------------------------------------

AC_SYS_LARGEFILE
dnl TODO: check if -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 is used

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
