#!gmake

# XSLT processor - other possibilities like Saxon exist
XSLT	= xsltproc --nonet
SED	= sed

# Profiling options for xsltproc
XSLTOPTS =

# Location of locally customized stylesheet, which imports
# the Docbook modular stylesheets, and specifically the
# stylesheet to convert Docbook+MathML => XHTML+MathML
# DB2XHTML = opengl-man.xsl
DB2XHTML = egl-man.xsl

.SUFFIXES: .gl .xml .html .xhtml .ck.xhtml .tex .pdf .3G .tar .tar.gz
.PHONY: man html pdf tex

# We now generate the EGL man pages as .html instead of .xml
# because they don't include any MathML

%.html: ../%.xml $(DB2XHTML)
	$(XSLT) $(XSLTOPTS) --xinclude -o $@.tmp $(DB2XHTML) $<
	$(SED) 's/<?xml-stylesheet/<!-- saved from url=(0013)about:internet -->\n<?xml-stylesheet/g' $@.tmp | \
	    $(SED) 's#http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd#xhtml1-transitional.dtd#g' > $@
	$(RM) $@.tmp

# EGL man pages
EGLXML = \
    eglBindAPI.html \
    eglBindTexImage.html \
    eglChooseConfig.html \
    eglCopyBuffers.html \
    eglCreateContext.html \
    eglCreatePbufferSurface.html \
    eglCreatePbufferFromClientBuffer.html \
    eglCreatePixmapSurface.html \
    eglCreateWindowSurface.html \
    eglDestroyContext.html \
    eglDestroySurface.html \
    eglGetConfigAttrib.html \
    eglGetConfigs.html \
    eglGetCurrentContext.html \
    eglGetCurrentDisplay.html \
    eglGetCurrentSurface.html \
    eglGetDisplay.html \
    eglGetError.html \
    eglGetProcAddress.html \
    eglInitialize.html \
    eglIntro.html \
    eglMakeCurrent.html \
    eglQueryAPI.html \
    eglQueryContext.html \
    eglQueryString.html \
    eglQuerySurface.html \
    eglReleaseTexImage.html \
    eglReleaseThread.html \
    eglSurfaceAttrib.html \
    eglSwapBuffers.html \
    eglSwapInterval.html \
    eglTerminate.html \
    eglWaitClient.html \
    eglWaitGL.html \
    eglWaitNative.html

# XML man page source and XHTML targets
XML    = $(EGLXML)

default: $(XML)

clobber clean:
	$(RM) $(XML)
