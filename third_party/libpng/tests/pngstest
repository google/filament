#!/bin/sh
#
# Usage:
#
#  tests/pngstest pattern
#
# Runs pngstest on all the contrib/pngsuite/[^x]*${pattern}.png files
# NOTE: pattern is used to name the temporary files pngstest generates
#
pattern="$1"
shift
exec ./pngstest --strict --tmpfile "${pattern}" --log ${1+"$@"}\
   "${srcdir}/contrib/pngsuite/"[a-wyz]*${pattern}".png"
