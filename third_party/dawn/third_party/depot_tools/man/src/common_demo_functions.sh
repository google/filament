#!/usr/bin/env bash

REMOTE=$(pwd)/demo_repo

unset GIT_DIR

# Helper functions
set_user() {
  export GIT_AUTHOR_EMAIL="$1@chromium.org"
  export GIT_AUTHOR_NAME="$1"
  export GIT_COMMITTER_EMAIL="$1@chromium.org"
  export GIT_COMMITTER_NAME="$1"
}
set_user 'local'


# increment time by X seconds
TIME=1397119976
tick() {
  TIME=$[$TIME + $1]
  export GIT_COMMITTER_DATE="$TIME +0000"
  export GIT_AUTHOR_DATE="$TIME +0000"
}
tick 0

# a commit
c() {
  silent git commit --allow-empty -m "$1"
  tick 10
}

praw() {
  echo -e "\x1B[37;1m$ $@\x1B[0m"
}

# print a visible command (but don't run it)
pcommand() {
  praw "$(python3 -c 'import sys, pipes; print(" ".join(map(pipes.quote, sys.argv[1:])))' "$@")"
}

# run a visible command
run() {
  pcommand "$@"
  "$@"
  # Some commands may not reset style, so issue reset command
  echo -e "\x1B[0m"
}

comment() {
  echo "###COMMENT### $@"
}

# run a command and print its output without printing the command itself
output() {
  "$@"
}

# run a silent command
silent() {
  if [[ $DEBUG ]]
  then
    "$@"
  else
    "$@" > /dev/null 2> /dev/null
  fi
}

# add a file with optionally content
add() {
  local CONTENT=$2
  if [[ ! $CONTENT ]]
  then
    CONTENT=$(python3 -c 'import random, string; print("".join(random.sample(string.ascii_lowercase, 16)))')
  fi
  echo "$CONTENT" > $1
  silent git add $1
}

# Add a special callout marker at the given line offset to indicate to
# filter_demo_output.py to add a callout at that offset.
callout() {
  echo -e "\x1b[${1}c"
}
