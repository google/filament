#!/usr/bin/env bash
. demo_repo.sh

add deleted_file
add unstaged_deleted_file
add modified_file
c 'I committed this and am proud of it.

Cr-Commit-Position: refs/heads/main@{#292272}
Tech-Debt-Introduced: 17 microMSOffices
Tech-Debt-Introduced: -4 microMSOffices'

run git footers HEAD
run git footers --key Tech-Debt-Introduced HEAD
run git footers --position HEAD
run git footers --position-num HEAD
run git footers --position-ref HEAD
