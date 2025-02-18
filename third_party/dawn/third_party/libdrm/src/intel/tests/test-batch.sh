#!/bin/sh

TEST_FILENAME=`echo "$0" | sed 's|\.sh$||'`
./test_decode $TEST_FILENAME

ret=$?

# pretty-print a diff showing what happened, and leave the dumped
# around for possibly moving over the ref.
if test $ret = 1; then
    REF_FILENAME="$TEST_FILENAME-ref.txt"
    NEW_FILENAME="$TEST_FILENAME-new.txt"
    ./test_decode $TEST_FILENAME -dump > $NEW_FILENAME
    if test $? = 0; then
	echo "Differences:"
	diff -u $REF_FILENAME $NEW_FILENAME
    fi
fi

exit $ret
