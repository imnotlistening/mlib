AUTOMAKE_OPTIONS = foreign

CFLAGS  += -O0 -Wall -Werror -ggdb
LDFLAGS +=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I@top_srcdir@/include ${libcurl_CFLAGS}

# The MLib shared library; modules can link against this.
lib_LTLIBRARIES	= libmlib.la
libmlib_la_SOURCES = module.c library.c core.c command.c playlist.c engine.c \
			bucket.c util.c
libmlib_la_LDFLAGS = ${libcurl_LIBS}

# The MLib program itself.
bin_PROGRAMS	= mlib
mlib_SOURCES	= mlib.c mlib_shell.c mlib_io.c mlib_lexxer.l
mlib_LDADD	= libmlib.la -lreadline

# A program for building libraries from pre-existing directories.
bin_PROGRAMS	+= mlib-genlib
mlib_genlib_SOURCES	= mlib_genlib.c
mlib_genlib_LDADD	= libmlib.la

# Libtool nicity. 
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
