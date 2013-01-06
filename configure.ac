#
# Top level configure.
#

AC_INIT([MLib], [0.0], [imNotListening@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC

AC_CONFIG_HEADERS([config.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Check for libcurl.
LIBCURL_CHECK_CONFIG()

# Gonna build some shared libraries.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC


AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
