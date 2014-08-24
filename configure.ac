#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sispmctl, 3.1, mondrian.nuessle@googlemail.com)
AC_CONFIG_AUX_DIR(admin)
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

dnl Archiver support
AM_PROG_AR

dnl Enhanced libtool support
LT_INIT

dnl Compiler check
AC_PROG_CC

dnl Configure macros
AC_CONFIG_MACRO_DIR([m4])

# check configure options
AC_ARG_ENABLE(webless,
AC_HELP_STRING([--enable-webless],
   [compile version of sispmctl without internal webserver]),
[WEBLESS=true])
AC_SUBST(WEBLESS)
AM_CONDITIONAL(WEBLESSCOND, test x$WEBLESS = xtrue)

AC_ARG_WITH(webdir,[  --with-webdir=DIR use DIR to look for html files for the web-interface], [WEBDIR="$withval"])
AC_SUBST(WEBDIR)

AC_ARG_WITH(bindaddr,[  --with-bindaddr=IP bind listening socket to IP], [BINDADDR="$withval"])
AC_SUBST(BINDADDR)

dnl check for libusb
PKG_CHECK_MODULES(LIBUSB, libusb >= 0.1.11)
CFLAGS="$CFLAGS $LIBUSB_CFLAGS"
LIBS="$LIBS $LIBUSB_LIBS"

dnl Check for recent pkg-config which supports Requires.private
case `$PKG_CONFIG --version` in
0.?|0.1[0-7]) PKGCONFIG_REQUIRES="Requires"; ;;
*) PKGCONFIG_REQUIRES="Requires.private"; ;;
esac
AC_SUBST(PKGCONFIG_REQUIRES)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h net/ethernet.h sys/ethernet.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strchr strdup strerror strncasecmp strrchr])

AC_CHECK_DECL(MSG_NOSIGNAL)
AC_CHECK_FUNC(nanosleep, [true], [AC_CHECK_LIB(rt, nanosleep)])
AC_CHECK_FUNC(inet_pton, [true], [AC_CHECK_LIB(nsl, inet_pton)])
AC_CHECK_FUNC(socket, [true], [AC_CHECK_LIB(socket, socket)])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AM_CONFIG_HEADER(config.h)
AC_OUTPUT
echo ""
echo "Good - your configure finished. Start make now"
echo ""
