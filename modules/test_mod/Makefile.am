AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -Werror -ggdb
LDFLAGS=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I@top_srcdir@/include

# The test module.
lib_LTLIBRARIES		= libtest_mod.la
libtest_mod_la_SOURCES	= test_mod.c
libtest_mod_la_LIBADD	= $(top_builddir)/src/libmlib.la
