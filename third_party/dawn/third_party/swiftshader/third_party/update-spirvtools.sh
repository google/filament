#!/bin/bash

# update-spirvtools merges the latest changes from
# the github.com/KhronosGroup/SPIRV-Tools into third_party/SPIRV-Tools.
# This script copies the change descriptions from the squash change into the
# top merge change, along with a standardized description.
REASON=$1

if [ ! -z "$REASON" ]; then
  REASON="\n$REASON\n"
fi

THIRD_PARTY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"

GIT_RESULT=`git subtree pull --prefix third_party/SPIRV-Tools https://github.com/KhronosGroup/SPIRV-Tools main --squash -m "Update SPIR-V Tools"`
if [[ $GIT_RESULT == *"CONFLICT"* ]]; then
  echo "subtree pull resulted in conflicts. Atempting to automatically resolve..."
  # CONFLICT is very likely due to Android.mk being deleted in our third_party.
  # Delete it, and try to continue.
  git rm ${THIRD_PARTY_DIR}/SPIRV-Tools/Android.mk
  git rm ${THIRD_PARTY_DIR}/SPIRV-Tools/android_test/Android.mk
  git rm ${THIRD_PARTY_DIR}/SPIRV-Tools/android_test/jni/Application.mk
  git -c core.editor=true merge --continue # '-c core.editor=true' prevents the editor from showing
  if [ $? -ne 0 ]; then
    echo "Could not automatically resolve conflicts."
    exit 1
  fi
fi


ALL_CHANGES=`git log -n 1 HEAD^2 | egrep '^(\s{4}[0-9a-f]{9}\s*.*)$'`
HEAD_CHANGE=`echo "$ALL_CHANGES" | egrep '[0-9a-f]{9}' -o -m 1`
LOG_MSG=`echo -e "Update SPIR-V Tools to $HEAD_CHANGE\n${REASON}\nChanges:\n$ALL_CHANGES\n\nCommands:\n    ./third_party/update-spirvtools.sh\n\nBug: b/123642959"`
git commit --no-verify --amend -m "$LOG_MSG"

# Use filter-branch to apply the Gerrit commit hook to both CLs
GIT_DIR=$(readlink -f "$(git rev-parse --git-dir)")
TMP_MSG="${GIT_DIR}/COMMIT_MSG_REWRITE"
FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch -f --msg-filter \
  "cat > ${TMP_MSG} && \"${GIT_DIR}/hooks/commit-msg\" ${TMP_MSG} && cat \"${TMP_MSG}\"" HEAD...HEAD~1
rm -rf "${TMP_MSG}"
