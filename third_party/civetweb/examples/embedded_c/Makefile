# 
# Copyright (c) 2013 No Face Press, LLC
# License http://opensource.org/licenses/mit-license.php MIT License
#

#This makefile is used to test the other Makefiles


PROG = embedded_c
SRC = embedded_c.c

TOP = ../..
CIVETWEB_LIB = libcivetweb.a

CFLAGS = -I$(TOP)/include $(COPT) -DUSE_WEBSOCKET -DUSE_IPV6
LIBS = -lpthread

include $(TOP)/resources/Makefile.in-os

ifeq ($(TARGET_OS),LINUX) 
	LIBS += -ldl
endif

all: $(PROG)

$(PROG): $(CIVETWEB_LIB) $(SRC)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(SRC) $(CIVETWEB_LIB) $(LIBS) -lcrypto -lssl -DUSE_SSL_DH=1

$(CIVETWEB_LIB):
	$(MAKE) -C $(TOP) WITH_IPV6=1 WITH_WEBSOCKET=1 COPT='-DNO_SSL_DL=1' clean lib
	cp $(TOP)/$(CIVETWEB_LIB) .

clean:
	rm -f $(CIVETWEB_LIB) $(PROG)

.PHONY: all clean

