dnl Process this file with autoconf to produce a configure script.

dnl ----------------------------------------------------------
dnl Initialization and Versioning
dnl ----------------------------------------------------------

AC_INIT([dvdbackup], [0.4.2])

# Where to generate output; srcdir location.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])

dnl Must come before AM_INIT_AUTOMAKE.
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-xz -Wall -Werror])

AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

AM_MAINTAINER_MODE

dnl ----------------------------------------------------------
dnl Checks for programs
dnl ----------------------------------------------------------

AC_PROG_CC_C99
AC_PROG_LN_S

AM_PROG_CC_C_O

dnl ----------------------------------------------------------
dnl Set build flags based on environment
dnl ----------------------------------------------------------

# enable extra warnings for gcc if they are not already enabled
if test -n "$GCC"; then
	case "$CFLAGS" in
	*-pedantic*) ;;
	*) CFLAGS="$CFLAGS -pedantic" ;;
	esac

	case "$CFLAGS" in
	*-Wall*) ;;
	*) CFLAGS="$CFLAGS -Wall" ;;
	esac

	case "$CFLAGS" in
	*-Wextra*) ;;
	*) CFLAGS="$CFLAGS -Wextra" ;;
	esac
fi

PKG_CHECK_MODULES(DEPS, glib-2.0)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

dnl ----------------------------------------------------------
dnl Checks for libraries
dnl ----------------------------------------------------------

AC_CHECK_LIB(dvdread, DVDOpen, , AC_MSG_ERROR([You need libdvdread]))
AC_CHECK_LIB(dvdread, DVDFileStat, [HAVE_DVDFileStat=yes], AC_MSG_ERROR([You have installed an incompatible version of libdvdread.
Have a look at http://dvdbackup.sourceforge.net for more details.]))

dnl ----------------------------------------------------------
dnl Checks for header files
dnl ----------------------------------------------------------

AC_CHECK_HEADERS(dvdread/dvd_reader.h, , AC_MSG_ERROR([You need libdvdread (dvd_reader.h)]))
AC_CHECK_HEADERS([fcntl.h libintl.h limits.h locale.h stdint.h stdlib.h string.h unistd.h])

dnl ----------------------------------------------------------
dnl Checks for types, structures and compilier characteristics
dnl ----------------------------------------------------------

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl check for dvdnav/dvd_types.h DVDDomain_t type in dvdnav/dvd_types.h
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[[#include <dvdnav/dvd_types.h>
DVDDomain_t t = DVD_DOMAIN_FirstPlay;]]
)], AC_DEFINE(HAVE_DVDNAV_DVDDOMAIN_TYPE, [1], [have dvdnav DVDDomain_t]))

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
