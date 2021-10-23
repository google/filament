#!/bin/sh

cp acinclude.m4 aclocal.m4

if test "$AUTOCONF"x = x; then
  AUTOCONF=autoconf
fi

$AUTOCONF || exit 1
rm aclocal.m4
rm -rf autom4te.cache
