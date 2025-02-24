#!/usr/bin/env bash
. git-hyper-blame.demo.common.sh
IGNORE=$(git rev-parse HEAD^)
run git hyper-blame -i ${IGNORE:0:8} ipsum.txt
