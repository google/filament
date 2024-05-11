#!/bin/bash

# This is a script used by some Buildbot buildslaves to push the project
#  through Clang's static analyzer and prepare the output to be uploaded
#  back to the buildmaster. You might find it useful too.

# Install Clang (you already have it on Mac OS X, apt-get install clang
#  on Ubuntu, etc),
# or download checker at http://clang-analyzer.llvm.org/ and unpack it in
#  /usr/local ... update CHECKERDIR as appropriate.

FINALDIR="$1"

CHECKERDIR="/usr/local/checker-279"
if [ ! -d "$CHECKERDIR" ]; then
    echo "$CHECKERDIR not found. Trying /usr/share/clang ..." 1>&2
    CHECKERDIR="/usr/share/clang/scan-build"
fi

if [ ! -d "$CHECKERDIR" ]; then
    echo "$CHECKERDIR not found. Giving up." 1>&2
    exit 1
fi

if [ -z "$MAKE" ]; then
    OSTYPE=`uname -s`
    if [ "$OSTYPE" == "Linux" ]; then
        NCPU=`cat /proc/cpuinfo |grep vendor_id |wc -l`
        let NCPU=$NCPU+1
    elif [ "$OSTYPE" = "Darwin" ]; then
        NCPU=`sysctl -n hw.ncpu`
    elif [ "$OSTYPE" = "SunOS" ]; then
        NCPU=`/usr/sbin/psrinfo |wc -l |sed -e 's/^ *//g;s/ *$//g'`
    else
        NCPU=1
    fi

    if [ -z "$NCPU" ]; then
        NCPU=1
    elif [ "$NCPU" = "0" ]; then
        NCPU=1
    fi

    MAKE="make -j$NCPU"
fi

echo "\$MAKE is '$MAKE'"

# Unset $MAKE so submakes don't use it.
MAKECOMMAND="$MAKE"
unset MAKE

set -x
set -e

cd `dirname "$0"`
cd ..

rm -rf checker-buildbot analysis
if [ ! -z "$FINALDIR" ]; then
    rm -rf "$FINALDIR"
fi

mkdir checker-buildbot
cd checker-buildbot

# We turn off deprecated declarations, because we don't care about these warnings during static analysis.
# The -Wno-liblto is new since our checker-279 upgrade, I think; checker otherwise warns "libLTO.dylib relative to clang installed dir not found"

# You might want to do this for CMake-backed builds instead...
PATH="$CHECKERDIR/bin:$PATH" scan-build -o analysis cmake -Wno-dev -DSDL_STATIC=OFF -DCMAKE_BUILD_TYPE=Debug -DASSERTIONS=enabled -DCMAKE_C_FLAGS="-Wno-deprecated-declarations" -DCMAKE_SHARED_LINKER_FLAGS="-Wno-liblto" ..

# ...or run configure without the scan-build wrapper...
#CC="$CHECKERDIR/libexec/ccc-analyzer" CFLAGS="-O0 -Wno-deprecated-declarations" LDFLAGS="-Wno-liblto" ../configure --enable-assertions=enabled

rm -rf analysis
PATH="$CHECKERDIR/bin:$PATH" scan-build -o analysis $MAKECOMMAND

if [ `ls -A analysis |wc -l` == 0 ] ; then
    mkdir analysis/zarro
    echo '<html><head><title>Zarro boogs</title></head><body>Static analysis: no issues to report.</body></html>' >analysis/zarro/index.html
fi

mv analysis/* ../analysis
rmdir analysis   # Make sure this is empty.
cd ..
chmod -R a+r analysis
chmod -R go-w analysis
find analysis -type d -exec chmod a+x {} \;
if [ -x /usr/bin/xattr ]; then find analysis -exec /usr/bin/xattr -d com.apple.quarantine {} \; 2>/dev/null ; fi

if [ ! -z "$FINALDIR" ]; then
    mv analysis "$FINALDIR"
else
    FINALDIR=analysis
fi

rm -rf checker-buildbot

echo "Done. Final output is in '$FINALDIR' ..."

# end of checker-buildbot.sh ...

