#!/bin/bash

# llvm now lives in a mono-repo along with clang, libc, lldb and a whole bunch
# of other projects (~90k files at time of writing).
# SwiftShader only requires the llvm project from this repo, and does not wish
# to pull in everything else.
# This script performs the following:
# * The llvm16-clean branch is fetched and checked out.
# * A sparse checkout of the llvm project is made to a temporary directory.
# * The third_party/llvm-16.0/llvm is replaced with the latest LLVM version.
# * This is committed and pushed.
# * The original branch is checked out again, and a merge from llvm16-clean to
#   the original branch is made.

THIRD_PARTY_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
STAGING_DIR="/tmp/llvm-16-update"
CLEAN_BRANCH="llvm16-clean"
SOURCE_DIR="${STAGING_DIR}/llvm"
TARGET_DIR="${THIRD_PARTY_DIR}/llvm-16.0/llvm"
LLVM_REPO_BRANCH="release/16.x"
BUG_NUMBER="b/272710814"

SWIFTSHADER_HEAD=`git rev-parse HEAD`

# Check there are no local changes
if ! git diff --quiet HEAD; then
  echo "Git workspace not clean"
  exit 1
fi

# Clone or update the staging directory
if [[ -d "$STAGING_DIR" ]]; then
  pushd "$STAGING_DIR"
else
  mkdir "$STAGING_DIR"
  pushd "$STAGING_DIR"
  git init
  git remote add origin https://github.com/llvm/llvm-project.git
  git config core.sparsecheckout true
  echo "/llvm/lib" >> .git/info/sparse-checkout
  echo "/llvm/include" >> .git/info/sparse-checkout
fi
git pull origin $LLVM_REPO_BRANCH
LLVM_HEAD=`git log HEAD -n 1 --pretty=format:'%h'`
popd

if [[ -d "$TARGET_DIR" ]]; then
  # Look for the last update change.
  LAST_TARGET_UPDATE=`git log --grep="^llvm-16-update: [0-9a-f]\{9\}$" -n 1 --pretty=format:'%h' ${TARGET_DIR}`
  if [[ ! -z "$LAST_TARGET_UPDATE" ]]; then
    # Get the LLVM commit hash from the update change.
    LAST_SOURCE_UPDATE=`git log $LAST_TARGET_UPDATE -n 1 | grep -oP "llvm-16-update: \K([0-9a-f]{9})"`
    if [ $LLVM_HEAD == $LAST_SOURCE_UPDATE ]; then
      echo "No new LLVM changes to apply"
      exit 0
    fi

    # Gather list of changes since last update
    pushd "$STAGING_DIR"
    LLVM_CHANGE_LOG=`git log $LAST_SOURCE_UPDATE..$LLVM_HEAD --pretty=format:'  %h %s'`
    LLVM_CHANGE_LOG="Changes:\n${LLVM_CHANGE_LOG}\n\n"
    popd
  fi
fi

COMMIT_MSG=`echo -e "Update LLVM 16 to ${LLVM_HEAD}\n\n${LLVM_CHANGE_LOG}Commands:\n  third_party/update-llvm-16.sh\n\nllvm-16-update: ${LLVM_HEAD}\nBug: ${BUG_NUMBER}"`

# Switch to the llvm-16-clean branch.
git fetch "https://swiftshader.googlesource.com/SwiftShader" $CLEAN_BRANCH
git checkout FETCH_HEAD

# Delete the target directory. We're going to re-populate it.
rm -fr "$TARGET_DIR"

# Update SwiftShader's $TARGET_DIR with a clean copy of latest LLVM
mkdir -p "$TARGET_DIR"
cp -r "$SOURCE_DIR/." "$TARGET_DIR"
git add "$TARGET_DIR"
git commit -m "$COMMIT_MSG"
MERGE_SOURCE=`git log HEAD -n 1 --pretty=format:'%h'`

# Push llvm-16-clean branch.
git push "https://swiftshader.googlesource.com/SwiftShader" $CLEAN_BRANCH

# Switch to the branch in use when calling the script.
git checkout $SWIFTSHADER_HEAD

# Update SwiftShader's $TARGET_DIR with a clean copy of latest LLVM
git merge -m "$COMMIT_MSG" "$MERGE_SOURCE"

# We're now done with the staging dir. Delete it.
rm -fr "$STAGING_DIR"
