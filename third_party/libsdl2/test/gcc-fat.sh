#!/bin/sh
#
# Build Universal binaries on Mac OS X, thanks Ryan!
#
# Usage: ./configure CC="sh gcc-fat.sh" && make && rm -rf ppc x86

# PowerPC compiler flags (10.2 runtime compatibility)
GCC_COMPILE_PPC="gcc-3.3 -arch ppc \
-DMAC_OS_X_VERSION_MIN_REQUIRED=1020 \
-nostdinc \
-F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks \
-I/Developer/SDKs/MacOSX10.2.8.sdk/usr/include/gcc/darwin/3.3 \
-isystem /Developer/SDKs/MacOSX10.2.8.sdk/usr/include"

GCC_LINK_PPC="\
-L/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.3 \
-F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks \
-Wl,-syslibroot,/Developer/SDKs/MacOSX10.2.8.sdk"

# Intel compiler flags (10.4 runtime compatibility)
GCC_COMPILE_X86="gcc-4.0 -arch i386 -mmacosx-version-min=10.4 \
-DMAC_OS_X_VERSION_MIN_REQUIRED=1040 \
-nostdinc \
-F/Developer/SDKs/MacOSX10.4u.sdk/System/Library/Frameworks \
-I/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin8/4.0.1/include \
-isystem /Developer/SDKs/MacOSX10.4u.sdk/usr/include"

GCC_LINK_X86="\
-L/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin8/4.0.0 \
-Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk"

# Output both PowerPC and Intel object files
args="$*"
compile=yes
link=yes
while test x$1 != x; do
    case $1 in
        --version) exec gcc $1;;
        -v) exec gcc $1;;
        -V) exec gcc $1;;
        -print-prog-name=*) exec gcc $1;;
        -print-search-dirs) exec gcc $1;;
        -E) GCC_COMPILE_PPC="$GCC_COMPILE_PPC -E"
            GCC_COMPILE_X86="$GCC_COMPILE_X86 -E"
            compile=no; link=no;;
        -c) link=no;;
        -o) output=$2;;
        *.c|*.cc|*.cpp|*.S) source=$1;;
    esac
    shift
done
if test x$link = xyes; then
    GCC_COMPILE_PPC="$GCC_COMPILE_PPC $GCC_LINK_PPC"
    GCC_COMPILE_X86="$GCC_COMPILE_X86 $GCC_LINK_X86"
fi
if test x"$output" = x; then
    if test x$link = xyes; then
        output=a.out
    elif test x$compile = xyes; then
        output=`echo $source | sed -e 's|.*/||' -e 's|\(.*\)\.[^\.]*|\1|'`.o
    fi
fi

if test x"$output" != x; then
    dir=ppc/`dirname $output`
    if test -d $dir; then
        :
    else
        mkdir -p $dir
    fi
fi
set -- $args
while test x$1 != x; do
    if test -f "ppc/$1" && test "$1" != "$output"; then
        ppc_args="$ppc_args ppc/$1"
    else
        ppc_args="$ppc_args $1"
    fi
    shift
done
$GCC_COMPILE_PPC $ppc_args || exit $?
if test x"$output" != x; then
    cp $output ppc/$output
fi

if test x"$output" != x; then
    dir=x86/`dirname $output`
    if test -d $dir; then
        :
    else
        mkdir -p $dir
    fi
fi
set -- $args
while test x$1 != x; do
    if test -f "x86/$1" && test "$1" != "$output"; then
        x86_args="$x86_args x86/$1"
    else
        x86_args="$x86_args $1"
    fi
    shift
done
$GCC_COMPILE_X86 $x86_args || exit $?
if test x"$output" != x; then
    cp $output x86/$output
fi

if test x"$output" != x; then
    lipo -create -o $output ppc/$output x86/$output
fi
