dnl Process this file with autoconf to produce a configure script

dnl ----------------------------------------------------------
dnl Initialization and Versioning
dnl ----------------------------------------------------------

AC_INIT([dvdbackup], [0.2-svn], [dvdbackup-bugs@lists.sourceforge.net])

# Where to generate output; srcdir location.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])

dnl Must come before AM_INIT_AUTOMAKE.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2 -Wall -Werror])

AM_GNU_GETTEXT_VERSION([0.16.1])
AM_GNU_GETTEXT([external])

AM_MAINTAINER_MODE

dnl ----------------------------------------------------------
dnl Checks for programs
dnl ----------------------------------------------------------

AC_PROG_CC
AC_PROG_LN_S

dnl ----------------------------------------------------------
dnl Set build flags based on environment
dnl ----------------------------------------------------------

# use -Wall -Wextra when possible
#AX_CFLAGS_GCC_OPTION(-pedantic)
AX_CFLAGS_GCC_OPTION(-Wall)
AX_CFLAGS_GCC_OPTION(-Wextra)

dnl ----------------------------------------------------------
dnl Checks for libraries
dnl ----------------------------------------------------------

AC_CHECK_LIB(dvdread, DVDOpen, , AC_MSG_ERROR([You need libdvdread]))

dnl ----------------------------------------------------------
dnl Checks for header files
dnl ----------------------------------------------------------

AC_CHECK_HEADERS(dvdread/dvd_reader.h, , AC_MSG_ERROR([You need libdvdread (dvd_reader.h)]))
AC_CHECK_HEADERS([fcntl.h libintl.h limits.h locale.h])

dnl ----------------------------------------------------------
dnl Checks for types, structures and compilier characteristics
dnl ----------------------------------------------------------

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl ----------------------------------------------------------
dnl Checks for library functions
dnl ----------------------------------------------------------

AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([mkdir setlocale strstr])

dnl ----------------------------------------------------------
dnl Checks for system services
dnl ----------------------------------------------------------

AC_SYS_LARGEFILE
LFS_CFLAGS=$(getconf LFS_CFLAGS)
CFLAGS="${CFLAGS} ${LFS_CFLAGS}"

AC_CONFIG_FILES([
	Makefile
	man/Makefile
	po/Makefile.in
	src/Makefile
])
AC_OUTPUT
