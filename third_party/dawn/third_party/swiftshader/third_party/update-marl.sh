#!/bin/bash

# update-marl merges the latest changes from the github.com/google/marl into
# third_party/marl. This script copies the change descriptions from the squash
# change into the top merge change, along with a standardized description. It
# should be run from the top of the swiftshader directory, followed by
# `git cl upload`.
#
# IMPORTANT NOTE: that git subtree doesn't play nicely with gerrit, so this script will
# result in two separate CLs, the first of which will have a merge conflict.
# You should force-submit the second (child) CL, ignoring the merge conflict on
# the parent. Make sure you're up-to-date beforehand, to avoid the possibility
# of a real merge conflict!
REASON=$1

if [ ! -z "$REASON" ]; then
  REASON="\n$REASON\n"
fi

git subtree pull --prefix third_party/marl https://github.com/google/marl main --squash -m "Update marl"

ALL_CHANGES=`git log -n 1 HEAD^2 | egrep '^(\s{4}[0-9a-f]{9}\s*.*)$'`
HEAD_CHANGE=`echo "$ALL_CHANGES" | egrep '[0-9a-f]{9}' -o -m 1`
LOG_MSG=`echo -e "Update Marl to $HEAD_CHANGE\n${REASON}\nChanges:\n$ALL_CHANGES\n\nCommands:\n    ./third_party/update-marl.sh\n\nBug: b/140546382"`
git commit --amend -m "$LOG_MSG"

# Use filter-branch to apply the Gerrit commit hook to both CLs
GIT_DIR=$(readlink -f "$(git rev-parse --git-dir)")
TMP_MSG="${GIT_DIR}/COMMIT_MSG_REWRITE"
FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch -f --msg-filter \
  "cat > ${TMP_MSG} && \"${GIT_DIR}/hooks/commit-msg\" ${TMP_MSG} && cat \"${TMP_MSG}\"" HEAD...HEAD~1
rm -rf "${TMP_MSG}"