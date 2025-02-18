AC_INIT
AC_PROG_CC
AC_PATH_X
if test "x$no_x" != "xyes"; then
  savecpp="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS -I$x_includes"
  AC_CHECK_HEADER($x_includes/X11/Xlib.h,,\
	 [AC_MSG_ERROR(Could not find X installation.)])
  CPPFLAGS="$savecpp"
  MDRIINC="-I$x_includes"
  LIBS="-L$x_libraries $LIBS"
else
  AC_MSG_ERROR(Could not find X installation. Aborting.)
fi
AC_ARG_WITH(libdrm,
            AC_HELP_STRING([--with-libdrm=DIR],
                           [Installation prefix of libdrm [[default=/usr]]]),
            [libdrmpref="$withval"],
            [libdrmpref="/usr"])
savecpp="$CPPFLAGS"
MDRIINC="-I$libdrmpref/include -I$libdrmpref/include/drm -I$x_includes"
CPPFLAGS="$CPPFLAGS $MDRIINC"
AC_CHECK_HEADER(xf86drm.h,,\
		 [AC_MSG_ERROR(Could not find libdrm installation. Use --with-libdrm=<libdrm_installation_prefix>)])
AC_CHECK_HEADER(drm.h,,\
		 [AC_MSG_ERROR(Could not find libdrm installation. Use --with-libdrm=<libdrm_installation_prefix>)])
CPPFLAGS="$savecpp"
LIBS="-L$libdrmpref/lib64 -L$libdrmpref/lib $LIBS"
AC_SUBST(MDRIINC)
AC_SYS_LARGEFILE
AM_INIT_AUTOMAKE(minidri,0.1.0)
AM_CONFIG_HEADER(config.h)
AC_OUTPUT([Makefile src/Makefile])
