#!/bin/bash
set -e
pushd "$(dirname "$0")" > /dev/null
docker build --tag zbloat .
docker run --rm -it -v `pwd`/../..:/filament zbloat python3 tools/zbloat/zbloat.py $1 $2 $3 $4
