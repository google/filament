#!/usr/bin/env bash

. common_demo_functions.sh

TDIR=$(mktemp --tmpdir -d demo_repo.XXXXXXXXXX)
trap "rm -rf $TDIR" EXIT

cd $TDIR
silent git clone "$REMOTE" .
silent git reset --hard stage_1
silent git update-ref refs/remotes/origin/main stage_1
silent git tag -d $(git tag -l 'stage_*')
silent git checkout origin/main
silent git branch -d main
silent git config color.ui always

if [[ ! "$BLANK_DEMO"  ]]
then
  silent git new-branch cool_feature

  c "Add widget"
  c "Refactor spleen"
  silent git tag spleen_tag

  c "another improvement"

  silent git new-branch --upstream_current subfeature
  c "slick commenting action"
  c "integrate with CoolService"

  silent git checkout cool_feature
  c "Respond to CL comments"

  silent git new-branch fixit
  c "Epic README update"
  c "Add neat feature"

  silent git new-branch --upstream_current frozen_branch
  c "a deleted file"
  c "modfile"
  c "FREEZE.unindexed"
fi

