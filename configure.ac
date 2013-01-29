#
# Top level configure.
#

AC_INIT([MLib], [0.0], [imNotListening@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LEX

AC_CONFIG_HEADERS([config.h])

AC_CHECK_HEADERS([sys/mman.h readline/readline.h readline/history.h])

# Gonna need these.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Check for libcurl.
LIBCURL_CHECK_CONFIG()

# Standard libraries we need.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(dl, dlopen)

# Gonna build some shared libraries.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC


AC_CONFIG_FILES([Makefile src/Makefile modules/Makefile
			  modules/test_mod/Makefile])
AC_OUTPUT
