#!/usr/bin/env bash
BLANK_DEMO=1
. demo_repo.sh

trunc() {
  echo ... truncated output ...
}

trunc_command() {
  pcommand "$@"
  trunc
}

WS=build/whitespace_file.txt
add_ws() {
  praw cat '>>' $WS '<<EOF'
  echo -e "$1"
  echo EOF
  echo -e "$1" >> $WS
}

ed_ws() {
  echo -ne "\x1B[37;1m$ echo -e "
  echo -n "'$1'"
  echo -e " | ed $WS\x1B[0m"
  echo -e "$1" | ed $WS 2>&1
}

# needs an extra echo afterwards
map() {
  run git map
  echo
}

ED1='/Banana\ns/Banana/Kuun\nwq'

ADD1="
\"You recall what happened on Mulholland drive?\" The ceiling fan rotated slowly
overhead, barely disturbing the thick cigarette smoke. No doubt was left about
when the fan was last cleaned."

ED2='/Kuun\ns/Kuun/Kun\nwq'

ADD2="
There was an poignant pause."

ADD3="
CHAPTER 3:
Mr. Usagi felt that something wasn't right. Shortly after the Domo-Kun left he
began feeling sick."

trunc_command fetch chromium
pcommand cd src

comment "(only on linux)"
trunc_command ./build/install-build-deps.sh

comment "Pull in all dependencies for HEAD"
trunc_command gclient sync

comment "Let's fix something!"
run git new-branch fix_typo
ed_ws "$ED1"
run git commit -am 'Fix terrible typo.'
map
run git status
trunc_command git cl upload -r domo@chromium.org --send-mail

comment "While we wait for feedback, let's do something else."
run git new-branch chap2
run git map-branches
add_ws "$ADD1"
run git status

comment "Someone on the code review pointed out that our typo-fix has a typo :("
comment "We're still working on 'chap2' but we really want to land"
comment "'fix_typo', so let's switch over and fix it."
run git freeze
run git checkout fix_typo  2>&1
ed_ws "$ED2"
run git upstream-diff --wordwise
run git commit -am 'Fix typo for good!'
trunc_command git cl upload

comment "Since we got lgtm, let the CQ land it."
pcommand git cl set_commit
map

comment "Switch back to where we were using the nav* commands (for fun..."
comment "git checkout would work here too)"
run git map-branches
run git nav-upstream  2>&1
pcommand git nav-downstream
git nav-downstream --pick 0  2>&1
run git map-branches

comment "Now we can pick up on chapter2 where we left off."
run git thaw
run git diff
add_ws "$ADD2"
run git diff
run git commit -am 'Finish chapter 2'
map
trunc_command git cl upload -r domo@chromium.org --send-mail

comment "We poke a committer until they lgtm :)"
pcommand git cl set_commit

comment "While that runs through the CQ, let's get started on chapter 3."
comment "Since we know that chapter 3 depends on chapter 2, we'll track the"
comment "current chapter2 branch."
run git new-branch --upstream_current chap3
add_ws "$ADD3"
run git commit -am 'beginning of chapter 3'
map

comment "We haven't updated the code in a while, so let's do that now."
pcommand git rebase-update
echo Fetching origin
git fetch origin 2>&1 | grep -v 'stage' | sed 's+From.*+From https://upstream+'
silent git update-ref refs/remotes/origin/main stage_2
silent git tag -d $(git tag -l 'stage_*')
git rebase-update --no-fetch

comment "Well look at that. The CQ landed our typo and chapter2 branches "
comment "already and git rebase-update cleaned them up for us."
trunc_command gclient sync
map

comment "Someone on IRC mentions that they actually landed a chapter 3 already!"
comment "We should pull their changes before continuing. Brace for"
comment "a code conflict!"
pcommand git rebase-update
echo Fetching origin
git fetch origin 2>&1 | grep -v 'stage' | sed 's+From.*+From https://upstream+'
silent git tag -d $(git tag -l 'stage_*')
echo Rebasing: chap2
silent git rebase-update
echo ... lots of output, it\'s a conflict alright :\(...
run git diff

comment "Oh, well, that's not too bad. In fact... that's a terrible chapter 3!"
praw \$EDITOR "$WS"
echo "... /me deletes bad chapter 3 ..."
silent git checkout --theirs -- "$WS"
run git add "$WS"
run git diff --cached

comment "Much better"
run git rebase --continue
run git rebase-update
silent git tag -d $(git tag -l 'stage_*')
trunc_command gclient sync
map
trunc_command git cl upload
