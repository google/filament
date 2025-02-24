#!/usr/bin/env bash
. demo_repo.sh

silent git branch no_upstream HEAD~

run git map-branches
run git map-branches -v

