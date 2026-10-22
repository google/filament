dnl Process this file with autoconf to produce a configure script.
AC_INIT(README)

dnl Detect the canonical build and host environments
AC_CONFIG_AUX_DIRS($srcdir/../build-scripts)
AC_CANONICAL_HOST

dnl Check for tools

AC_PROG_CC

dnl Check for compiler environment

AC_C_CONST

dnl We only care about this for building testnative at the moment, so these
dnl  values shouldn't be considered absolute truth.
dnl  (Haiku, for example, sets none of these.)
ISUNIX="false"
ISWINDOWS="false"
ISMACOSX="false"

dnl Figure out which math library to use
case "$host" in
    *-*-cygwin* | *-*-mingw32*)
        ISWINDOWS="true"
        EXE=".exe"
        MATHLIB=""
        SYS_GL_LIBS="-lopengl32"
        ;;
    *-*-haiku*)
        EXE=""
        MATHLIB=""
        SYS_GL_LIBS="-lGL"
        ;;
    *-*-darwin* )
        ISMACOSX="true"
        EXE=""
        MATHLIB=""
        SYS_GL_LIBS="-Wl,-framework,OpenGL"
        ;;
    *-*-aix*)
        ISUNIX="true"
        EXE=""
        if test x$ac_cv_prog_gcc = xyes; then
            CFLAGS="-mthreads"
        fi
        SYS_GL_LIBS=""
        ;;
    *-*-mint*)
        EXE=""
        MATHLIB=""
        AC_PATH_PROG(OSMESA_CONFIG, osmesa-config, no)
        if test "x$OSMESA_CONFIG" = "xyes"; then
            OSMESA_CFLAGS=`$OSMESA_CONFIG --cflags`
            OSMESA_LIBS=`$OSMESA_CONFIG --libs`
            CFLAGS="$CFLAGS $OSMESA_CFLAGS"
            SYS_GL_LIBS="$OSMESA_LIBS"
        else
            SYS_GL_LIBS="-lOSMesa"
        fi
        ;;
    *-*-qnx*)
        EXE=""
        MATHLIB=""
        SYS_GL_LIBS="-lGLES_CM"
        ;;
    *-*-emscripten* )
        dnl This should really be .js, but we need to specify extra flags when compiling to js
        EXE=".bc"
        MATHLIB=""
        SYS_GL_LIBS=""
        ;;
    *)
        dnl Oh well, call it Unix...
        ISUNIX="true"
        EXE=""
        MATHLIB="-lm"
        SYS_GL_LIBS="-lGL"
        ;;
esac
AC_SUBST(EXE)
AC_SUBST(MATHLIB)
AC_SUBST(ISMACOSX)
AC_SUBST(ISWINDOWS)
AC_SUBST(ISUNIX)

dnl Check for SDL
SDL_VERSION=2.0.0
AM_PATH_SDL2($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS -lSDL2_test $SDL_LIBS"

dnl Check for X11 path, needed for OpenGL on some systems
AC_PATH_X
if test x$have_x = xyes; then
    if test x$ac_x_includes = xno || test "x$ac_x_includes" = xNone || test "x$ac_x_includes" = x; then
        :
    else
        CFLAGS="$CFLAGS -I$ac_x_includes"
    fi
    if test x$ac_x_libraries = xno || test "x$ac_x_libraries" = xNone; then
        :
    else
        if test "x$ac_x_libraries" = x; then
            XPATH=""
            XLIB="-lX11"
        else
            XPATH="-L$ac_x_libraries"
            XLIB="-L$ac_x_libraries -lX11"
        fi
    fi
fi

dnl Check for OpenGL
AC_MSG_CHECKING(for OpenGL support)
have_opengl=no
AC_TRY_COMPILE([
 #include "SDL_opengl.h"
],[
],[
have_opengl=yes
])
AC_MSG_RESULT($have_opengl)

dnl Check for OpenGL ES
AC_MSG_CHECKING(for OpenGL ES support)
have_opengles=no
AC_TRY_COMPILE([
 #if defined (__IPHONEOS__)
    #include <OpenGLES/ES1/gl.h>
 #else
    #include <GLES/gl.h>
 #endif /* __QNXNTO__ */
],[
],[
have_opengles=yes
])
AC_MSG_RESULT($have_opengles)

dnl Check for OpenGL ES2
AC_MSG_CHECKING(for OpenGL ES2 support)
have_opengles2=no
AC_TRY_COMPILE([
 #if defined (__IPHONEOS__)
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
 #else
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
 #endif
],[
],[
have_opengles2=yes
])
AC_MSG_RESULT($have_opengles2)

GLLIB=""
GLESLIB=""
GLES2LIB=""
if test x$have_opengles = xyes; then
    CFLAGS="$CFLAGS -DHAVE_OPENGLES"
    GLESLIB="$XPATH -lGLESv1_CM"
fi
if test x$have_opengles2 = xyes; then
    CFLAGS="$CFLAGS -DHAVE_OPENGLES2"
    #GLES2LIB="$XPATH -lGLESv2"
fi
if test x$have_opengl = xyes; then
    CFLAGS="$CFLAGS -DHAVE_OPENGL"
    GLLIB="$XPATH $SYS_GL_LIBS"
fi

AC_SUBST(GLLIB)
AC_SUBST(GLESLIB)
AC_SUBST(GLES2LIB)
AC_SUBST(XLIB)

dnl Check for SDL_ttf
AC_CHECK_LIB(SDL2_ttf, TTF_Init, have_SDL_ttf=yes)
if test x$have_SDL_ttf = xyes; then
    CFLAGS="$CFLAGS -DHAVE_SDL_TTF"
    SDL_TTF_LIB="-lSDL2_ttf"
fi
AC_SUBST(SDL_TTF_LIB)

dnl Finally create all the generated files
AC_OUTPUT([Makefile])
