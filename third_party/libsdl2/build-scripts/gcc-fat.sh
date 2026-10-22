#!/bin/sh
#
# Build Universal binaries on Mac OS X, thanks Ryan!
#
# Usage: ./configure CC="sh gcc-fat.sh" && make && rm -rf x86 x64

DEVELOPER="`xcode-select -print-path`/Platforms/MacOSX.platform/Developer"

# Intel 32-bit compiler flags (10.6 runtime compatibility)
GCC_COMPILE_X86="gcc -arch i386 -mmacosx-version-min=10.6 \
-I/usr/local/include"

GCC_LINK_X86="-mmacosx-version-min=10.6"

# Intel 64-bit compiler flags (10.6 runtime compatibility)
GCC_COMPILE_X64="gcc -arch x86_64 -mmacosx-version-min=10.6 \
-DMAC_OS_X_VERSION_MIN_REQUIRED=1060 \
-I/usr/local/include"

GCC_LINK_X64="-mmacosx-version-min=10.6"

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
        -E) GCC_COMPILE_X86="$GCC_COMPILE_X86 -E"
            GCC_COMPILE_X64="$GCC_COMPILE_X64 -E"
            compile=no; link=no;;
        -c) link=no;;
        -o) output=$2;;
        *.c|*.cc|*.cpp|*.S|*.m|*.mm) source=$1;;
    esac
    shift
done
if test x$link = xyes; then
    GCC_COMPILE_X86="$GCC_COMPILE_X86 $GCC_LINK_X86"
    GCC_COMPILE_X64="$GCC_COMPILE_X64 $GCC_LINK_X64"
fi
if test x"$output" = x; then
    if test x$link = xyes; then
        output=a.out
    elif test x$compile = xyes; then
        output=`echo $source | sed -e 's|.*/||' -e 's|\(.*\)\.[^\.]*|\1|'`.o
    fi
fi

# Compile X86 32-bit
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

# Compile X86 32-bit
if test x"$output" != x; then
    dir=x64/`dirname $output`
    if test -d $dir; then
        :
    else
        mkdir -p $dir
    fi
fi
set -- $args
while test x$1 != x; do
    if test -f "x64/$1" && test "$1" != "$output"; then
        x64_args="$x64_args x64/$1"
    else
        x64_args="$x64_args $1"
    fi
    shift
done
$GCC_COMPILE_X64 $x64_args || exit $?
if test x"$output" != x; then
    cp $output x64/$output
fi

if test x"$output" != x; then
    lipo -create -o $output x86/$output x64/$output
fi
