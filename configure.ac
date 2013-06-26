AC_INIT(Nemo, 0.16.4)
AC_PROG_CC
AC_LANG(C)
AC_CHECK_HEADERS([stdbool.h], [AC_DEFINE([HAVE_STDBOOL_H])])
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(readline, readline)
AC_CHECK_LIB(m, sqrt)
# Set the default library path to /usr/lib
libdir=/usr/lib
# define the libdir
AC_DEFINE_UNQUOTED([LIBDIR], "${libdir}", [Path where to install the libraries])
# Create the Makefile
AC_OUTPUT(Makefile)
# Create the lib/Makefile
AC_OUTPUT(lib/Makefile)
# Create the config.h from confdefs.h
cp confdefs.h config.h
