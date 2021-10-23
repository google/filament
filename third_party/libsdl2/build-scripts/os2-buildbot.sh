#!/bin/bash

# This is the script buildbot.libsdl.org uses to cross-compile SDL2 from
#  x86 Linux to OS/2, using OpenWatcom.

# The final zipfile can be unpacked on any machine that supports OpenWatcom
#  (Windows, Linux, OS/2, etc). Point the compiler at the include directory
#  and link against the SDL2.lib file. Ship the SDL2.dll with your app.

if [ -z "$WATCOM" ]; then
    echo "This script expects \$WATCOM to be set to the OpenWatcom install dir." 1>&2
    echo "This is often something like '/usr/local/share/watcom'" 1>&2
    exit 1
fi

export PATH="$WATCOM/binl:$PATH"

ZIPFILE="$1"
if [ -z $1 ]; then
    ZIPFILE=sdl-os2.zip
fi
ZIPDIR=buildbot/SDL

set -e
set -x

cd `dirname "$0"`
cd ..

rm -f $ZIPFILE
wmake -f Makefile.os2
rm -rf $ZIPDIR
mkdir -p $ZIPDIR
chmod a+r SDL2.lib SDL2.dll
mv SDL2.lib SDL2.dll $ZIPDIR/
cp -R include $ZIPDIR/
zip -9r "buildbot/$ZIPFILE" $ZIPDIR

wmake -f Makefile.os2 distclean

set +x
echo "All done. Final installable is in $ZIPFILE ...";
