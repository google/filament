#!/usr/bin/env bash
export EDITOR=${EDITOR:=notepad}
WIN_BASE=`dirname $0`
UNIX_BASE=`cygpath "$WIN_BASE"`
export PATH="$PATH:$UNIX_BASE/${PYTHON3_BIN_RELDIR_UNIX}:$UNIX_BASE/${PYTHON3_BIN_RELDIR_UNIX}/Scripts"
export PYTHON_DIRECT=1
export PYTHONUNBUFFERED=1

WIN_GIT_PARENT=`dirname "${GIT_BIN_ABSDIR}"`
UNIX_GIT_PARENT=`cygpath "$WIN_GIT_PARENT"`
BASE_GIT=`basename "${GIT_BIN_ABSDIR}"`
UNIX_GIT="$UNIX_GIT_PARENT/$BASE_GIT"
if [[ $# > 0 ]]; then
  "$UNIX_GIT/bin/bash.exe" "$@"
else
  "$UNIX_GIT/git-bash.exe" &
fi
