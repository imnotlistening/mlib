AUTOMAKE_OPTIONS = foreign

CFLAGS  += -Wall -Werror -ggdb
LDFLAGS +=

# Include our include path in the preprocessor flags.
AM_CPPFLAGS	= -I@top_srcdir@/include -I@top_srcdir@/regress

# A regression program.
bin_PROGRAMS	= mlib-regress
mlib_regress_SOURCES	= regress.c basic.c
mlib_regress_LDADD	= $(top_builddir)/src/libmlib.la

# Libtool nicity. 
LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
