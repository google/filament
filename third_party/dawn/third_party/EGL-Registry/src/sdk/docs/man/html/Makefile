#!gmake

# XSLT processor - other possibilities like Saxon exist
XSLT	= xsltproc --nonet

# Location of locally customized stylesheet, which imports
# the Docbook modular stylesheets, and specifically the
# stylesheet to convert Docbook 5 + MathML => HTML5
DB2XHTML = khronos-man.xsl

.SUFFIXES: .xml .html .xhtml

# EGL man pages

EGLXML = \
    eglBindAPI.xhtml \
    eglBindTexImage.xhtml \
    eglChooseConfig.xhtml \
    eglClientWaitSync.xhtml \
    eglCopyBuffers.xhtml \
    eglCreateContext.xhtml \
    eglCreateImage.xhtml \
    eglCreatePbufferSurface.xhtml \
    eglCreatePbufferFromClientBuffer.xhtml \
    eglCreatePixmapSurface.xhtml \
    eglCreatePlatformPixmapSurface.xhtml \
    eglCreatePlatformWindowSurface.xhtml \
    eglCreateSync.xhtml \
    eglCreateWindowSurface.xhtml \
    eglDestroyContext.xhtml \
    eglDestroyImage.xhtml \
    eglDestroySurface.xhtml \
    eglDestroySync.xhtml \
    eglGetConfigAttrib.xhtml \
    eglGetConfigs.xhtml \
    eglGetCurrentContext.xhtml \
    eglGetCurrentDisplay.xhtml \
    eglGetCurrentSurface.xhtml \
    eglGetDisplay.xhtml \
    eglGetError.xhtml \
    eglGetPlatformDisplay.xhtml \
    eglGetProcAddress.xhtml \
    eglGetSyncAttrib.xhtml \
    eglInitialize.xhtml \
    eglIntro.xhtml \
    eglMakeCurrent.xhtml \
    eglQueryAPI.xhtml \
    eglQueryContext.xhtml \
    eglQueryString.xhtml \
    eglQuerySurface.xhtml \
    eglReleaseTexImage.xhtml \
    eglReleaseThread.xhtml \
    eglSurfaceAttrib.xhtml \
    eglSwapBuffers.xhtml \
    eglSwapInterval.xhtml \
    eglTerminate.xhtml \
    eglWaitClient.xhtml \
    eglWaitGL.xhtml \
    eglWaitNative.xhtml \
    eglWaitSync.xhtml

XML    = $(EGLXML)

# Generate just a few files, for testing
#XML = glViewport.xhtml glTexImage2D.xhtml glPixelStore.xhtml

default: $(XML) indices

# Dependencies on XIncluded files - none yet.
#
#glTexImage2D.xhtml glTexImage3D.xhtml \
#    : ../internalformattable.xml ../unsizedformattable.xml
#
## These are very conservative dependencies
#$(GLSLXML): $(GLSLXIFILES)

$(XML): %.xhtml: ../%.xml $(DB2XHTML)
	$(XSLT) --xinclude --noout $(DB2XHTML) $<

indices: Makefile makeindex.py $(XML)
	./makeindex.py . .. index.php indexflat.php

clean:
	$(RM) $(XML) index.php indexflat.php

clobber: clean
