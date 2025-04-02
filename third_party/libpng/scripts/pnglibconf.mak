#!/usr/bin/make -f
# pnglibconf.mak - standard make lines for pnglibconf.h
#
# These lines are copied from Makefile.am, they illustrate
# how to automate the build of pnglibconf.h from scripts/pnglibconf.dfa
# given just 'awk', a C preprocessor and standard command line utilities

# Override as appropriate, these definitions can be overridden on
# the make command line (AWK='nawk' for example).
AWK = gawk
AWK = mawk
AWK = nawk
AWK = one-true-awk
AWK = awk      # This fails on SunOS 5.10; use 'nawk'
CPP = $(CC) -E # If this fails on SunOS 5.10, use '/lib/cpp'

MOVE = mv -f
DELETE = rm -f

DFA_XTRA = # Put your configuration file here, see scripts/pnglibconf.dfa.  Eg:
# DFA_XTRA = pngusr.dfa

# CPPFLAGS should contain the options to control the result,
# but DEFS and CFLAGS are also supported here, override
# as appropriate
DFNFLAGS = $(DEFS) $(CPPFLAGS) $(CFLAGS)

# srcdir is a de-facto standard for the location of the source
srcdir = .

# The standard pnglibconf.h exists as scripts/pnglibconf.h.prebuilt,
# copy this if the following doesn't work.
pnglibconf.h: pnglibconf.dfn
	$(DELETE) $@ pnglibconf.c pnglibconf.out pnglibconf.tmp
	echo '#include "pnglibconf.dfn"' >pnglibconf.c
	@echo "## If '$(CC) -E' fails, try /lib/cpp (e.g. CPP='/lib/cpp')" >&2
	$(CPP) $(DFNFLAGS) pnglibconf.c >pnglibconf.out
	$(AWK) -f $(srcdir)/scripts/dfn.awk out=pnglibconf.tmp pnglibconf.out >&2
	$(MOVE) pnglibconf.tmp $@

pnglibconf.dfn: $(srcdir)/scripts/pnglibconf.dfa $(srcdir)/scripts/options.awk $(srcdir)/pngconf.h $(srcdir)/pngusr.dfa $(DFA_XTRA)
	$(DELETE) $@ pnglibconf.pre pnglibconf.tmp
	@echo "## Calling $(AWK) from scripts/pnglibconf.mak" >&2
	@echo "## If 'awk' fails, try a better awk (e.g. AWK='nawk')" >&2
	$(AWK) -f $(srcdir)/scripts/options.awk out=pnglibconf.pre\
	    version=search $(srcdir)/pngconf.h $(srcdir)/scripts/pnglibconf.dfa\
	    $(srcdir)/pngusr.dfa $(DFA_XTRA) >&2
	$(AWK) -f $(srcdir)/scripts/options.awk out=pnglibconf.tmp pnglibconf.pre >&2
	$(MOVE) pnglibconf.tmp $@

clean-pnglibconf:
	$(DELETE) pnglibconf.h pnglibconf.c pnglibconf.out pnglibconf.pre \
	pnglibconf.dfn

clean: clean-pnglibconf
