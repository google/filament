#!/bin/sh
#
# Generate a header file with the current source revision

outdir=`pwd`
cd `dirname $0`
srcdir=..
header=$outdir/include/SDL_revision.h

rev=`sh showrev.sh 2>/dev/null`
if [ "$rev" != "" ]; then
    echo "#define SDL_REVISION \"$rev\"" >"$header.new"
    echo "#define SDL_REVISION_NUMBER 0" >>"$header.new"
    if diff $header $header.new >/dev/null 2>&1; then
        rm "$header.new"
    else
        mv "$header.new" "$header"
    fi
fi
