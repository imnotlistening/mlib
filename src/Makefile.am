AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall 
LDFLAGS=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I../include

# The MLib shared library; modules can link against this.
lib_LTLIBRARIES	= libmlib.la
libmlib_la_SOURCES = module.c library.c

# The MLib program itself.
bin_PROGRAMS	= mlib
mlib_SOURCES	= mlib.c
mlib_LDADD	= libmlib.la

# Libtool nicity. 
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
