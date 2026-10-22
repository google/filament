#!/bin/sh

prefix=@prefix@
exec_prefix=@exec_prefix@
exec_prefix_set=no
libdir=@libdir@

@ENABLE_STATIC_FALSE@usage="\
@ENABLE_STATIC_FALSE@Usage: $0 [--prefix[=DIR]] [--exec-prefix[=DIR]] [--version] [--cflags] [--libs]"
@ENABLE_STATIC_TRUE@usage="\
@ENABLE_STATIC_TRUE@Usage: $0 [--prefix[=DIR]] [--exec-prefix[=DIR]] [--version] [--cflags] [--libs] [--static-libs]"

if test $# -eq 0; then
      echo "${usage}" 1>&2
      exit 1
fi

while test $# -gt 0; do
  case "$1" in
  -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) optarg= ;;
  esac

  case $1 in
    --prefix=*)
      prefix=$optarg
      if test $exec_prefix_set = no ; then
        exec_prefix=$optarg
      fi
      ;;
    --prefix)
      echo $prefix
      ;;
    --exec-prefix=*)
      exec_prefix=$optarg
      exec_prefix_set=yes
      ;;
    --exec-prefix)
      echo $exec_prefix
      ;;
    --version)
      echo @SDL_VERSION@
      ;;
    --cflags)
      echo -I@includedir@/SDL2 @SDL_CFLAGS@
      ;;
@ENABLE_SHARED_TRUE@    --libs)
@ENABLE_SHARED_TRUE@      echo -L@libdir@ @SDL_RLD_FLAGS@ @SDL_LIBS@
@ENABLE_SHARED_TRUE@      ;;
@ENABLE_STATIC_TRUE@@ENABLE_SHARED_TRUE@    --static-libs)
@ENABLE_STATIC_TRUE@@ENABLE_SHARED_FALSE@    --libs|--static-libs)
@ENABLE_STATIC_TRUE@      echo -L@libdir@ @SDL_RLD_FLAGS@ @SDL_STATIC_LIBS@
@ENABLE_STATIC_TRUE@      ;;
    *)
      echo "${usage}" 1>&2
      exit 1
      ;;
  esac
  shift
done
