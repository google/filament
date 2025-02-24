#!/usr/bin/env bash
. demo_repo.sh

run git map
echo

# since these are all empty commits, pretend there's something there
pcommand git squash-branch -m "cool squash demo"
git squash-branch -m "cool squash demo" > /dev/null 2> /dev/null
c "cool squash demo"

run git map
