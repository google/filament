# Copyright (c) 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# This adds completion to bash shells for git cl commands. It is
# meant for developers and not needed for inclusion by any automated
# processes that will, of course, specify the full command, not rely
# on or benefit from tab-completion.
#
# Requires:
#   Installed git bash completion.
#
# Usage:
#   Put something like the following in your .bashrc:
#   . $PATH_TO_DEPOT_TOOLS/git cl_completion.sh
#


# Parses commands from git cl -h.
__git_cl_commands () {
  git cl -h 2> /dev/null | sed -n 's/^\s*\x1b\[32m\(\S\+\)\s*\x1b\[39m.*$/\1/p'
}

# Caches variables in __git_cl_all_commands.
__git_cl_compute_all_commands () {
  test -n "$__git_cl_all_commands" ||
  __git_cl_all_commands="$(__git_cl_commands)"
}

_git_cl () {
  __git_cl_compute_all_commands
  local subcommands=$(echo "$__git_cl_all_commands" | xargs)
  local subcommand=$(__git_find_on_cmdline "$subcommands")
  if [[ -z "$subcommand" ]]; then
      __gitcomp "$subcommands"
      return
  fi

  case "$subcommand,$cur" in
      upload,--*)
          __gitcomp_builtin cl_upload
          ;;
      "",*)
          __gitcomp_nl "${__git_cl_all_commands}"
          ;;
  esac
}
