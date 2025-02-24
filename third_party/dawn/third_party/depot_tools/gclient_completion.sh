# gclient_completion.sh
#
# This adds completion to bash shells for gclient commands. It is
# meant for developers and not needed for inclusion by any automated
# processes that will, of course, specify the full command, not rely
# on or benefit from tab-completion.
#
# Requires:
#   bash-completion package for _get_comp_words_by_ref.
#   newer versions of sed for the improved regular expression handling.
#
# On Mac, this is accomplished by installing fink (www.finkproject.org)
# then doing sudo apt-get update ; sudo apt-get install sed
#
# Usage:
#   Put something like the following in your .bashrc:
#   . $PATH_TO_DEPOT_TOOLS/gclient_completion.sh
#


# Parses commands from gclient -h.
__gclient_commands () {
  gclient -h 2> /dev/null | sed -n 's/^\s*\x1b\[32m\(.*\)\x1b\[39m.*$/\1/p'
}

# Caches variables in __gclient_all_commands.
# Adds "update" command, which is not listed.
__gclient_compute_all_commands () {
  test -n "$__gclient_all_commands" ||
  __gclient_all_commands="$(__gclient_commands) update"
}

# Since gclient fetch is a passthrough to git, let the completions
# come from git's completion if it's defined.
if [[ -n _git_fetch ]]; then
    _gclient_fetch=_git_fetch
fi

# Completion callback for gclient cmdlines.
_gclient () {
  local cur prev words cword
  _get_comp_words_by_ref -n =: cur prev words cword

  # Find the command by ignoring flags.
  local i c=1 cword_adjust=0 command 
  while [ $c -lt $cword ]; do
    i="${words[$c]}"
    case "$i" in
      -*)
        ((cword_adjust++))
        : ignore options ;;
      *) command="$i"; break ;;
    esac
    ((c++))
  done

  # If there is a completion function for the command, use it and
  # return.
  local completion_func="_gclient_${command//-/_}"
  local -f $completion_func >/dev/null && $completion_func && return

  # If the command or hasn't been given, provide completions for all
  # commands. Also provide all commands as completion for the help
  # command.
  # echo "command=$command" >> /tmp/comp.log
  case "$command" in
    ""|help)
      if [[ "$command" != help || $((cword - cword_adjust)) -le 2 ]]; then
        __gclient_compute_all_commands
        COMPREPLY=($(compgen -W "$__gclient_all_commands" $cur))
      fi
      ;;
    *) : just use the default ;;
  esac
} &&
complete -F _gclient -o default gclient
