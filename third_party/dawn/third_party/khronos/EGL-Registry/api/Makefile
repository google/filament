# Copyright 2013-2020 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# Generated headers
EGLHEADERS = EGL/egl.h EGL/eglext.h

# Generation tools
PYTHON = python3
PYFILES = genheaders.py reg.py
REGISTRY = egl.xml
GENOPTS =
GENHEADERS = $(PYTHON) -B genheaders.py $(GENOPTS) -registry $(REGISTRY)

all: $(EGLHEADERS)

EGL/egl.h: egl.xml $(PYFILES)
	$(GENHEADERS) EGL/egl.h

EGL/eglext.h: egl.xml $(PYFILES)
	$(GENHEADERS) EGL/eglext.h

# Simple test to make sure generated headers compile
KHR   = .
TESTS = Tests

tests: egltest.c $(EGLHEADERS)
	$(CC) -c -I$(KHR) egltest.c
	$(CXX) -c -I$(KHR) egltest.c
	-rm egltest.o

# Verify registries against the schema

validate:
	jing -c registry.rnc egl.xml

################################################

# Remove intermediate targets from 'make tests'
clean:
	rm -f *.[io] Tests/*.[io] diag.txt dumpReg.txt errwarn.txt

clobber: clean
	rm -f $(EGLHEADERS)
