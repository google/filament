# 
# Copyright (c) 2013 No Face Press, LLC
# License http://opensource.org/licenses/mit-license.php MIT License
#

#This makefile is used to test the other Makefiles


PROG = chat
SRC = chat.c

TOP = ../..
CIVETWEB_LIB = libcivetweb.a
SSL_CERT = ssl_cert.pem

CFLAGS = -I$(TOP)/include $(COPT)
LIBS = -lpthread

include $(TOP)/resources/Makefile.in-os

ifeq ($(TARGET_OS),LINUX) 
	LIBS += -ldl
endif

all: $(PROG) $(SSL_CERT)

$(PROG): $(CIVETWEB_LIB) $(SRC)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(SRC) $(CIVETWEB_LIB) $(LIBS)

$(CIVETWEB_LIB):
	$(MAKE) -C $(TOP) clean lib
	cp $(TOP)/$(CIVETWEB_LIB) .

$(SSL_CERT):
	cp $(TOP)/resources/$(SSL_CERT) .

clean:
	rm -f $(CIVETWEB_LIB) $(PROG) $(SSL_CERT)

.PHONY: all clean
