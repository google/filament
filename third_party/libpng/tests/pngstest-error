#!/bin/sh
code=77 # skipped
for t in "${srcdir}/contrib/pngsuite/"x*".png"
do
   if test "$t" != "${srcdir}/contrib/pngsuite/x*.png"
   then
      # not skipped, test it
      if ./pngstest --strict --tmpfile "error" --log "$@" "$t"
      then
         code=0 # oops, success: should not happen!
      fi
   fi
done
exit $code
