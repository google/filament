#!/usr/bin/env bash

set -e

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Merge the release candidate (rc) branch into release and cut a new rc branch."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME [options] <current rc branch new> <new rc branch name>"
    echo "    $SELF_NAME -c <current rc branch new>"
    echo ""
    echo "current rc branch name should be the name of an existing rc/* branch"
    echo "new rc branch name should be the new rc branch name and should not exist"
    echo ""
    echo "Example:"
    echo "    $SELF_NAME rc/1.9.3 rc/1.9.4"
    echo ""
    echo "Options:"
    echo "    -c"
    echo "       List the commits that make up a release and exit."
    echo "    -h"
    echo "       Print this help message and exit."
    echo "    -p"
    echo "       Automatically push changes to origin (dangerous)."
}

function list_commits {
    git log --oneline release.."$1"
}

PUSH_CHANGES=false
LIST_COMMITS=false
while getopts "hpc" opt; do
    case ${opt} in
        h)
            print_help
            exit 0
            ;;
        p)
            PUSH_CHANGES=true
            ;;
        c)
            LIST_COMMITS=true
            ;;
        *)
            print_help
            exit 1
            ;;
    esac
done
shift $((OPTIND - 1))

if [[ "${LIST_COMMITS}" == "true" ]]; then
    if [[ "$#" -ne 1 ]]; then
        print_help
        exit 1
    fi

    RC_BRANCH=$1

    list_commits "${RC_BRANCH}"
    exit 0
fi

if [[ "$#" -ne 2 ]]; then
    print_help
    exit 1
fi

RC_BRANCH=$1
NEW_RC_BRANCH=$2

####################################################################################################
# Preamble
####################################################################################################

# Check that the RC_BRANCH is a real branch.
git show-branch "${RC_BRANCH}" >/dev/null 2>&1 || {
    echo "Branch ${RC_BRANCH} does not exist (try running git fetch first)."
    exit 1
}

# Ensure no staged or working changes
if [ ! -z "$(git status --untracked-files=no --porcelain)" ]; then
  echo "You have uncommited changes. Please stash or commit before running."
  exit 1
fi

# Make sure release, main, and the release candidate branch are up-to-date.
git checkout main
git pull origin main --rebase

git checkout "${RC_BRANCH}"
git pull origin "${RC_BRANCH}" --rebase

git checkout release
git pull origin release --rebase

####################################################################################################
# Merge rc branch into release
####################################################################################################

# We need to merge rc into release in a somewhat roundabout way because we want the merge to always
# favor the changes on the rc branch. Any commits *only* on the release branch should be lost. These
# abandonded commits should only ever be temporary hotfixes.
# See the last part of https://stackoverflow.com/a/27338013/2671441 for more details.
# The benefit of this strategy is that we'll never get any merge conflicts. All of these commands
# can run without human intervention.

git checkout release

# Do a merge commit. The content of this commit does not matter, so we use a strategy that never
# fails. This advances release.
git merge --no-edit -s ours "${RC_BRANCH}"

# Change working tree and index to desired content.
# --detach ensures rc branch will not move when doing the reset in the next step.
git checkout --detach "${RC_BRANCH}"

# Move HEAD to release without changing contents of working tree and index.
git reset --soft release

# 'attach' HEAD to release.
# This ensures release will move when doing 'commit --amend'.
git checkout release

# Change content of merge commit to current index (i.e. content of rc branch).
git commit --amend -C HEAD

####################################################################################################
# Cut a new release candidate branch
####################################################################################################

git checkout main
git branch "${NEW_RC_BRANCH}"

####################################################################################################
# Delete the old release candidate branch
####################################################################################################

git branch -D "${RC_BRANCH}"

####################################################################################################
# Push changes
####################################################################################################

if [[ "${PUSH_CHANGES}" == "true" ]]; then
    # Push the release branch
    git push origin release

    # Delete the old rc branch remotely
    git push origin --delete "${RC_BRANCH}"

    # Push the new rc branch
    git push origin -u "${NEW_RC_BRANCH}"
else
    echo ""
    echo "Done."
    echo ""
    echo "Changes have not been pushed."
    echo "If everything looks good locally, run the following:"
    echo "---------------------------------------------------------"
    echo "git push origin release"
    echo "git push origin --delete ${RC_BRANCH}"
    echo "git push origin -u ${NEW_RC_BRANCH}"
    echo "---------------------------------------------------------"
    echo "Or, to undo everything:"
    echo "---------------------------------------------------------"
    echo "git checkout release && git reset --hard origin/release"
    echo "git branch -D ${NEW_RC_BRANCH}"
    echo "git checkout --track origin/rc/${RC_BRANCH}"
    echo "---------------------------------------------------------"
fi
