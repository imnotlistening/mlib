AUTOMAKE_OPTIONS = foreign

CFLAGS=-Wall 
LDFLAGS=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS = -I../include

lib_LTLIBRARIES = libmlib.la
libmlib_la_SOURCES = module.c library.c

# Libtool nicity. 
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
