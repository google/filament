#!/usr/bin/env bash
. demo_repo.sh

run git map-branches -v
run git new-branch independent_cl
run git map-branches -v
praw vi foo \&\& git add -A \&\& git commit -m 'foo'
silent echo 'foo' >foo
silent git add -A
silent git commit -m 'foo'
run git map-branches -v
run git new-branch --upstream subfeature nested_cl
callout 3
run git map-branches -v
run git checkout cool_feature 2>&1
run git new-branch --upstream_current cl_depends_on_cool_feature
run git map-branches -v
run git checkout subfeature
run git new-branch --inject_current injected_cl
run git map-branches -v
