#! /usr/bin/env bash

dir=`realpath $(dirname "$BASH_SOURCE:-0")`
githookdir=`git rev-parse --git-dir`/hooks

# Space separated list of hooks to install
declare -a HOOKS=("pre-commit" "commit-msg")

hookConfigExists="$(git hook list pre-commit 2>/dev/null >> /dev/null ; echo $?)"

for hook in ${HOOKS[@]}; do
    if [ ! -f "${dir}/${hook}" ]; then
        continue
    fi
    # Soft remove the old script in .git/hooks so that git doesn't try to run
    # multiple hooks.
    if [ -f "${githookdir}/${hook}" ]; then
        mv "${githookdir}/${hook}" "${githookdir}/${hook}.old"
    fi
    if [ "$hookConfigExists" -eq "0" ]; then
        git config --unset-all hook.${hook}.command
        git config --add hook.${hook}.command "${dir}/${hook}"
    else
        cp ${dir}/${hook} ${githookdir}/${hook}
    fi
done
