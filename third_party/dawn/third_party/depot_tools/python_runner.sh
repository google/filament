# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

## This file is designed to be sourced from a bash script whose name takes the
## form 'command-name'. This script will then instead invoke
## '[depot_tools]/command_name.py' correctly under mingw as well
## as posix-ey systems, passing along all other command line flags.

## Example:
## echo ". python_runner.sh" > git-foo-command
## ./git-foo-command  #=> runs `python git_foo_command.py`

## Constants
PYTHONDONTWRITEBYTECODE=1; export PYTHONDONTWRITEBYTECODE

## "Input parameters".
# If set before the script is sourced, then we'll use the pre-set values.
#
# SCRIPT defaults to the basename of $0, with dashes replaced with underscores

# "$0" can have several different formats depending on how the script was called
# and the environment being used, including having different formats even in the
# same environment (e.g. in msys, 'git cl' causes $0 to have a Windows-style
# path, but calling 'git-cl' results in a POSIX-style path), so don't assume a
# particular format.
# First try to split it using Windows format ...
DEPOT_TOOLS="${0%\\*}"
if [[ "$DEPOT_TOOLS" = "$0" ]]; then
  # If that didn't work, try POSIX format ...
  DEPOT_TOOLS="${0%/*}"
  if [[ "$DEPOT_TOOLS" = "$0" ]]; then
    # Sometimes commands will run with no path (e.g. a git command run from
    # within the depot_tools dir itself). In that case, treat it as if run like:
    # "./command"
    DEPOT_TOOLS="."
    BASENAME="$0"
  else
    BASENAME="${0##*/}"
  fi
else
  BASENAME="${0##*\\}"
fi

SCRIPT="${SCRIPT-${BASENAME//-/_}.py}"

# Ensure that "depot_tools" is somewhere in PATH so this tool can be used
# standalone, but allow other PATH manipulations to take priority.
PATH=$PATH:$DEPOT_TOOLS

vpython3 "$DEPOT_TOOLS/$SCRIPT" "$@"
