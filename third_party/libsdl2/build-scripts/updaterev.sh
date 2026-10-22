#!/bin/sh
#
# Generate a header file with the current source revision

outdir=`pwd`
cd `dirname $0`
srcdir=..
header=$outdir/include/SDL_revision.h

rev=`sh showrev.sh 2>/dev/null`
if [ "$rev" != "" -a "$rev" != "hg-0:baadf00d" ]; then
    revnum=`echo $rev | sed 's,hg-\([0-9]*\).*,\1,'`
    echo "#define SDL_REVISION \"$rev\"" >"$header.new"
    echo "#define SDL_REVISION_NUMBER $revnum" >>"$header.new"
    if diff $header $header.new >/dev/null 2>&1; then
        rm "$header.new"
    else
        mv "$header.new" "$header"
    fi
fi
