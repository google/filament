# Copyright (c) 2023 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# This adds completion to bash shells for git commands. It is
# meant for developers and not needed for inclusion by any automated
# processes that will, of course, specify the full command, not rely
# on or benefit from tab-completion.
#
# Requires:
#   Installed git bash completion.
#
# Usage:
#  Add this to your .bashrc:
#
#        # The next lines enable bash completion for git commands from
#        # depot_tools.
#        if [ -f "$HOME/bin/depot_tools/git_completion.sh" ]; then
#          . "$HOME/bin/depot_tools/git_completion.sh"
#        fi


_git_new_branch ()
{
  case "$cur" in
  -*)
    __gitcomp_nl_append "--upstream_current"
    __gitcomp_nl_append "--upstream"
    __gitcomp_nl_append "--lkgr"
    __gitcomp_nl_append "--inject_current"
    ;;
  *)
    case "$prev,$cur" in
      --upstream,o*)
        # By default (only local branch heads are shown after --upstream, see
        # the case below. If, however, the user types "--upstream o", also
        # remote branches (origin/*) are shown.
        __git_complete_refs --cur="$cur"
        ;;
      --upstream,*)
        __gitcomp_nl "$(__git_heads '' $cur)"
        ;;
    esac
  esac
}

_git_reparent_branch ()
{
  case "$cur" in
  -*)
    __gitcomp_nl_append "--lkgr"
    __gitcomp_nl_append "--root"
    ;;
  o*)
    # By default (only local branch heads are shown after --upstream, see the
    # case below. If, however, the user types "--upstream o", also remote
    # branches (origin/*) are shown.
    __git_complete_refs --cur="$cur"
    ;;
  *)
    __gitcomp_nl "$(__git_heads '' $cur)"
    ;;
  esac
}
