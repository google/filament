# Copyright 2013-2026 The Khronos Group Inc.
# SPDX-License-Identifier: Apache-2.0

# Generator scripts and options
# GENOPTS can be e.g. '-noprotect'

PYFILES = genheaders.py reg.py
GENOPTS =
GENHEADERS = ./genheaders.py $(GENOPTS)

# Generate all headers for GL / GLES / WGL / GLX
# Different headers depend on different XML registry files
# The $(GENHEADERS) script has targets for the additional headers
#   $(API)/GL/gl.h
#   $(API)/GL/glx.h
#   $(API)/GL/wgl.h
# but these are experimental and not supported by Khronos. OpenGL
# drivers and SDKs provided by OS vendors and IHVs include their own
# platform-specific versions of these headers, where appropriate.

API	   = ../api
GLHEADERS  = $(API)/GL/glext.h \
	     $(API)/GL/glcorearb.h \
	     $(API)/GLES/gl.h \
	     $(API)/GLES/glext.h \
	     $(API)/GLES2/gl2.h \
	     $(API)/GLES2/gl2ext.h \
	     $(API)/GLES3/gl3.h \
	     $(API)/GLES3/gl31.h \
	     $(API)/GLES3/gl32.h \
	     $(API)/GLSC2/glsc2.h \
	     $(API)/GLSC2/glsc2ext.h

GLXHEADERS = $(API)/GL/glxext.h
WGLHEADERS = $(API)/GL/wglext.h \
	     $(API)/GL/wgl.h
ALLHEADERS = $(GLHEADERS) $(GLXHEADERS) $(WGLHEADERS)

# Generated files to clean up
TEXDIRT = readme.aux readme.idx readme.ilg readme.ind readme.log readme.toc readme.out
PYDIRT	= diag.txt dumpReg.txt errwarn.txt reg.pyc

default: $(ALLHEADERS)

$(GLHEADERS): gl.xml $(PYFILES)
	$(GENHEADERS) $@

$(GLXHEADERS): glx.xml $(PYFILES)
	$(GENHEADERS) $@ -registry glx.xml

$(WGLHEADERS): wgl.xml $(PYFILES)
	$(GENHEADERS) $@ -registry wgl.xml

# Generate Relax NG XML schema from Compact schema

registry.rng: registry.rnc
	trang registry.rnc registry.rng

# Verify all registry XML files against the schema

validate:
	jing -c registry.rnc gl.xml
	jing -c registry.rnc glx.xml
	jing -c registry.rnc wgl.xml

# Documentation target

readme.pdf: readme.tex
	touch readme.ind
	pdflatex readme.tex
	pdflatex readme.tex
	makeindex readme.idx
	pdflatex readme.tex

clean:
	-rm -f $(PYDIRT) $(TEXDIRT)
