#!/usr/bin/env bash
. demo_repo.sh

add deleted_file
add unstaged_deleted_file
add modified_file
c "demo changes"

add added_file_with_unstaged_changes
echo bob >> added_file_with_unstaged_changes

add added_file
echo bob >> modified_file
silent git rm deleted_file
rm unstaged_deleted_file
touch unadded_file

run git status --short
run git freeze
run git status --short
run git log -n 2 --stat
run git thaw
run git status --short
