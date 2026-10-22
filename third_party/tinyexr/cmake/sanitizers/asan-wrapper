#!/bin/sh

# The MIT License (MIT)
#
# Copyright (c)
#   2013 Matthew Arsenault
#   2015-2016 RWTH Aachen University, Federal Republic of Germany
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# This script is a wrapper for AddressSanitizer. In some special cases you need
# to preload AddressSanitizer to avoid error messages - e.g. if you're
# preloading another library to your application. At the moment this script will
# only do something, if we're running on a Linux platform. OSX might not be
# affected.


# Exit immediately, if platform is not Linux.
if [ "$(uname)" != "Linux" ]
then
    exec $@
fi


# Get the used libasan of the application ($1). If a libasan was found, it will
# be prepended to LD_PRELOAD.
libasan=$(ldd $1 | grep libasan | sed "s/^[[:space:]]//" | cut -d' ' -f1)
if [ -n "$libasan" ]
then
    if [ -n "$LD_PRELOAD" ]
    then
        export LD_PRELOAD="$libasan:$LD_PRELOAD"
    else
        export LD_PRELOAD="$libasan"
    fi
fi

# Execute the application.
exec $@
