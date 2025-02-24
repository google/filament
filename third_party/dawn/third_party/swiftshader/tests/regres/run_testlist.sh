#!/bin/bash

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"

pushd $ROOT_DIR
go run ./cmd/run_testlist/main.go --test-list=$ROOT_DIR/testlists/vk-master.txt $@
popd
