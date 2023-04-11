# Makefile for PngMinus (png2pnm and pnm2png)
# Linux / Unix

#CC = cc
CC = gcc
LD = $(CC)

RM = rm -f

PNGINC = -I../..
PNGLIB_SHARED = -L../.. -lpng
PNGLIB_STATIC = ../../libpng.a

# Uncomment the following if you have a custom zlib build at ../../../zlib
#ZINC = -I../../../zlib
#ZLIB_SHARED = -L../../../zlib -lz
#ZLIB_STATIC = ../../../zlib/libz.a

# Use the system zlib otherwise
ZLIB_SHARED = -lz
ZLIB_STATIC = -lz

CPPFLAGS = $(PNGINC) $(ZINC)
CFLAGS =
LDFLAGS =
LIBS_SHARED = $(PNGLIB_SHARED) $(ZLIB_SHARED)
LIBS_STATIC = $(PNGLIB_STATIC) $(ZLIB_STATIC)

EXEEXT =
#EXEEXT = .exe

# dependencies

all: png2pnm$(EXEEXT) pnm2png$(EXEEXT) png2pnm-static$(EXEEXT) pnm2png-static$(EXEEXT)

png2pnm.o: png2pnm.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) png2pnm.c

pnm2png.o: pnm2png.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) pnm2png.c

png2pnm$(EXEEXT): png2pnm.o
	$(LD) $(LDFLAGS) -o png2pnm$(EXEEXT) png2pnm.o $(LIBS_SHARED) -lm

pnm2png$(EXEEXT): pnm2png.o
	$(LD) $(LDFLAGS) -o pnm2png$(EXEEXT) pnm2png.o $(LIBS_SHARED) -lm

png2pnm-static$(EXEEXT): png2pnm.o
	$(LD) $(LDFLAGS) -o png2pnm-static$(EXEEXT) png2pnm.o $(LIBS_STATIC) -lm

pnm2png-static$(EXEEXT): pnm2png.o
	$(LD) $(LDFLAGS) -o pnm2png-static$(EXEEXT) pnm2png.o $(LIBS_STATIC) -lm

clean:
	$(RM) png2pnm.o
	$(RM) pnm2png.o
	$(RM) png2pnm$(EXEEXT)
	$(RM) pnm2png$(EXEEXT)
	$(RM) png2pnm-static$(EXEEXT)
	$(RM) pnm2png-static$(EXEEXT)

# End of makefile for png2pnm / pnm2png
