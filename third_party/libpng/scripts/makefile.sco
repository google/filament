# makefile for SCO OSr5  ELF and Unixware 7 with Native cc
# Contributed by Mike Hopkirk (hops at sco.com) modified from Makefile.lnx
#   force ELF build dynamic linking, SONAME setting in lib and RPATH in app
# Copyright (C) 2020-2022 Cosmin Truta
# Copyright (C) 2002, 2006, 2010-2014 Glenn Randers-Pehrson
# Copyright (C) 1998 Greg Roelofs
# Copyright (C) 1996, 1997 Andreas Dilger
#
# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

# Library name:
LIBNAME=libpng16
PNGMAJ=16

# Shared library names:
LIBSO=$(LIBNAME).so
LIBSOMAJ=$(LIBNAME).so.$(PNGMAJ)

# Utilities:
CC=cc
AR_RC=ar rc
RANLIB=echo
MKDIR_P=mkdir
LN_SF=ln -f -s
CP=cp
RM_F=/bin/rm -f

# Where the zlib library and include files are located
#ZLIBLIB=/usr/local/lib
#ZLIBINC=/usr/local/include
ZLIBLIB=../zlib
ZLIBINC=../zlib

CPPFLAGS=-I$(ZLIBINC)
CFLAGS= -dy -belf -O3
LDFLAGS=-L. -L$(ZLIBLIB) -lpng16 -lz -lm

# Pre-built configuration
# See scripts/pnglibconf.mak for more options
PNGLIBCONF_H_PREBUILT = scripts/pnglibconf.h.prebuilt

OBJS = png.o pngerror.o pngget.o pngmem.o pngpread.o \
       pngread.o pngrio.o pngrtran.o pngrutil.o pngset.o \
       pngtrans.o pngwio.o pngwrite.o pngwtran.o pngwutil.o

OBJSDLL = $(OBJS:.o=.pic.o)

.SUFFIXES:      .c .o .pic.o

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

.c.pic.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -KPIC -o $@ $*.c

all: libpng.a $(LIBSO) pngtest

pnglibconf.h: $(PNGLIBCONF_H_PREBUILT)
	$(CP) $(PNGLIBCONF_H_PREBUILT) $@

libpng.a: $(OBJS)
	$(AR_RC) $@ $(OBJS)
	$(RANLIB) $@

$(LIBSO): $(LIBSOMAJ)
	$(LN_SF) $(LIBSOMAJ) $(LIBSO)

$(LIBSOMAJ): $(OBJSDLL)
	$(CC) -G  -Wl,-h,$(LIBSOMAJ) -o $(LIBSOMAJ) \
	 $(OBJSDLL)

pngtest: pngtest.o $(LIBSO)
	LD_RUN_PATH=.:$(ZLIBLIB) $(CC) -o pngtest $(CFLAGS) pngtest.o $(LDFLAGS)

test: pngtest
	./pngtest

install:
	@echo "The $@ target is no longer supported by this makefile."
	@false

install-static:
	@echo "The $@ target is no longer supported by this makefile."
	@false

install-shared:
	@echo "The $@ target is no longer supported by this makefile."
	@false

clean:
	$(RM_F) *.o libpng.a pngtest pngout.png
	$(RM_F) $(LIBSO) $(LIBSOMAJ)* pngtest-static pnglibconf.h

# DO NOT DELETE THIS LINE -- make depend depends on it.

png.o      png.pic.o:      png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngerror.o pngerror.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngget.o   pngget.pic.o:   png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngmem.o   pngmem.pic.o:   png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngpread.o pngpread.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngread.o  pngread.pic.o:  png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngrio.o   pngrio.pic.o:   png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngrtran.o pngrtran.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngrutil.o pngrutil.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngset.o   pngset.pic.o:   png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngtrans.o pngtrans.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngwio.o   pngwio.pic.o:   png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngwrite.o pngwrite.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngwtran.o pngwtran.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h
pngwutil.o pngwutil.pic.o: png.h pngconf.h pnglibconf.h pngpriv.h pngstruct.h pnginfo.h pngdebug.h

pngtest.o: png.h pngconf.h pnglibconf.h
