#!/bin/sh
#
# Regenerate configuration files
cp acinclude.m4 aclocal.m4
found=false
for autoconf in autoconf autoconf259 autoconf-2.59
do if which $autoconf >/dev/null 2>&1; then $autoconf && found=true; break; fi
done
if test x$found = xfalse; then
    echo "Couldn't find autoconf, aborting"
    exit 1
fi
