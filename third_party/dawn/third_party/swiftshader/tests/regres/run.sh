#!/bin/bash

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"

pushd $ROOT_DIR
go run ./cmd/regres/main.go $@ 2>&1 | tee regres-log.txt
popd
