AUTOMAKE_OPTIONS = foreign

CFLAGS  += -Wall -Werror -ggdb
LDFLAGS +=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I@top_srcdir@/include

# The test module.
lib_LTLIBRARIES		= libmplayer_engine.la
libmplayer_engine_la_SOURCES	= mplayer_engine.c
libmplayer_engine_la_LIBADD	= $(top_builddir)/src/libmlib.la
