#!/usr/bin/env bash
. demo_repo.sh

silent git checkout subfeature

run git map-branches
run git nav-upstream  2>&1
run git map-branches
callout 3
run git nav-upstream  2>&1
run git map-branches

