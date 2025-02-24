dnl
dnl Check whether _Unwind_GetIPInfo is available without doing a link
dnl test so we can use this with libstdc++-v3 and libjava.  Need to
dnl use $target to set defaults because automatic checking is not possible
dnl without a link test (and maybe even with a link test).
dnl

AC_DEFUN([GCC_CHECK_UNWIND_GETIPINFO], [
  AC_ARG_WITH(system-libunwind,
  [  --with-system-libunwind use installed libunwind])
  # If system-libunwind was not specifically set, pick a default setting.
  if test x$with_system_libunwind = x; then
    case ${target} in
      ia64-*-hpux*) with_system_libunwind=yes ;;
      *) with_system_libunwind=no ;;
    esac
  fi
  # Based on system-libunwind and target, do we have ipinfo?
  if  test x$with_system_libunwind = xyes; then
    case ${target} in
      ia64-*-*) have_unwind_getipinfo=no ;;
      *) have_unwind_getipinfo=yes ;;
    esac
  else
    # Darwin before version 9 does not have _Unwind_GetIPInfo.
    changequote(,)
    case ${target} in
      *-*-darwin[3-8]|*-*-darwin[3-8].*) have_unwind_getipinfo=no ;;
      *) have_unwind_getipinfo=yes ;;
    esac
    changequote([,])
  fi

  if test x$have_unwind_getipinfo = xyes; then
    AC_DEFINE(HAVE_GETIPINFO, 1, [Define if _Unwind_GetIPInfo is available.])
  fi
])

# ACX_PROG_CC_WARNING_OPTS(WARNINGS, [VARIABLE = WARN_CFLAGS])
#   Sets @VARIABLE@ to the subset of the given options which the
#   compiler accepts.
AC_DEFUN([ACX_PROG_CC_WARNING_OPTS],
[AC_REQUIRE([AC_PROG_CC])dnl
AC_LANG_PUSH(C)
m4_pushdef([acx_Var], [m4_default([$2], [WARN_CFLAGS])])dnl
AC_SUBST(acx_Var)dnl
m4_expand_once([acx_Var=
],m4_quote(acx_Var=))dnl
save_CFLAGS="$CFLAGS"
for real_option in $1; do
  # Do the check with the no- prefix removed since gcc silently
  # accepts any -Wno-* option on purpose
  case $real_option in
    -Wno-*) option=-W`expr x$real_option : 'x-Wno-\(.*\)'` ;;
    *) option=$real_option ;;
  esac
  AS_VAR_PUSHDEF([acx_Woption], [acx_cv_prog_cc_warning_$option])
  AC_CACHE_CHECK([whether $CC supports $option], acx_Woption,
    [CFLAGS="$option"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
      [AS_VAR_SET(acx_Woption, yes)],
      [AS_VAR_SET(acx_Woption, no)])
  ])
  AS_IF([test AS_VAR_GET(acx_Woption) = yes],
        [acx_Var="$acx_Var${acx_Var:+ }$real_option"])
  AS_VAR_POPDEF([acx_Woption])dnl
done
CFLAGS="$save_CFLAGS"
m4_popdef([acx_Var])dnl
AC_LANG_POP(C)
])# ACX_PROG_CC_WARNING_OPTS

