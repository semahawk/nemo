AC_INIT(Nemo, `cat VERSION`)
AC_PROG_CC
AC_LANG(C)
AC_CHECK_HEADERS([stdbool.h], [AC_DEFINE([HAVE_STDBOOL_H])])
AC_CHECK_HEADERS([stdint.h], [], [AC_MSG_ERROR([Could not find stdint.h])])
AC_CHECK_LIB(dl, dlopen)
AC_CHECK_LIB(readline, readline)
AC_CHECK_LIB(m, sqrt)
# define the Nemo's version
version=`cat VERSION`
AC_DEFINE_UNQUOTED([VERSION], "${version}", [Version of Nemo])
# Set the default library path to /usr/lib
libdir=/usr/lib
# define the libdir
AC_DEFINE_UNQUOTED([LIBDIR], "${libdir}", [Path where to install the libraries])
# --enable-debug
debug=0
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Enable debugging]))

AS_IF([test "x$enable_debug" = "xyes"], [
  debug=1
])
AC_DEFINE_UNQUOTED([DEBUG], ${debug}, [Debugging enabled])
# Create the Makefile
AC_OUTPUT(Makefile)
# Create the lib/Makefile
AC_OUTPUT(lib/Makefile)
# Create the config.h from confdefs.h
cp confdefs.h config.h
