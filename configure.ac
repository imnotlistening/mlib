#
# Top level configure.
#

AC_INIT([MLib], [0.0], [imNotListening@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_LEX

AC_CONFIG_HEADERS([config.h])

AC_CHECK_HEADERS([sys/mman.h])

# Gonna need these.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T

# Check for libcurl.
PKG_CHECK_MODULES([libcurl], [libcurl >= 7.19.0])

# Standard libraries we need.
AC_SEARCH_LIBS([dlopen], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlopen() function])
])
AC_CHECK_LIB(pthread, pthread_create)

# Readline.
AC_CHECK_LIB(readline, readline, [], [
  AC_MSG_ERROR([Could not find readline])
], -lncurses)

# Gonna build some shared libraries.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC


AC_CONFIG_FILES([Makefile src/Makefile modules/Makefile regress/Makefile
			  modules/test_mod/Makefile
			  modules/mplayer_engine/Makefile])
AC_OUTPUT
