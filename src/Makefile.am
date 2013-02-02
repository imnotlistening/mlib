AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall -Werror -ggdb
LDFLAGS=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I../include

# The MLib shared library; modules can link against this.
lib_LTLIBRARIES	= libmlib.la
libmlib_la_SOURCES = module.c library.c core.c command.c playlist.c
libmlib_la_LDFLAGS = -lcurl

# The MLib program itself.
bin_PROGRAMS	= mlib
mlib_SOURCES	= mlib.c mlib_shell.c mlib_io.c mlib_lexxer.l bucket.c
mlib_LDADD	= libmlib.la -lreadline

# Libtool nicity. 
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
