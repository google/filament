#!/usr/bin/env bash
set -e

# ci_legacy.sh
# Continuously integrate libpng using the legacy makefiles.
#
# Copyright (c) 2019-2022 Cosmin Truta.
#
# This software is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h.

CI_SCRIPTNAME="$(basename "$0")"
CI_SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
CI_SRCDIR="$(dirname "$CI_SCRIPTDIR")"
CI_BUILDDIR="$CI_SRCDIR"

function ci_info {
    printf >&2 "%s: %s\\n" "$CI_SCRIPTNAME" "$*"
}

function ci_err {
    printf >&2 "%s: error: %s\\n" "$CI_SCRIPTNAME" "$*"
    exit 2
}

function ci_spawn {
    printf >&2 "%s: executing:" "$CI_SCRIPTNAME"
    printf >&2 " %q" "$@"
    printf >&2 "\\n"
    "$@"
}

function ci_init_legacy {
    CI_SYSTEM_NAME="$(uname -s)"
    CI_MACHINE_NAME="$(uname -m)"
    CI_MAKE="${CI_MAKE:-make}"
    case "$CI_SYSTEM_NAME" in
    ( Darwin | *BSD | DragonFly )
        [[ -x $(command -v clang) ]] && CI_CC="${CI_CC:-clang}" ;;
    ( * )
        [[ -x $(command -v gcc) ]] && CI_CC="${CI_CC:-gcc}" ;;
    esac
    CI_CC="${CI_CC:-cc}"
    case "$CI_CC" in
    ( *clang* )
        CI_LEGACY_MAKEFILES="${CI_LEGACY_MAKEFILES:-"scripts/makefile.clang"}" ;;
    ( *gcc* )
        CI_LEGACY_MAKEFILES="${CI_LEGACY_MAKEFILES:-"scripts/makefile.gcc"}" ;;
    ( cc | c89 | c99 )
        CI_LEGACY_MAKEFILES="${CI_LEGACY_MAKEFILES:-"scripts/makefile.std"}" ;;
    esac
    CI_LD="${CI_LD:-"$CI_CC"}"
    CI_LIBS="${CI_LIBS:-"-lz -lm"}"
}

function ci_trace_legacy {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "system name: $CI_SYSTEM_NAME"
    ci_info "machine hardware name: $CI_MACHINE_NAME"
    ci_info "source directory: $CI_SRCDIR"
    ci_info "build directory: $CI_BUILDDIR"
    ci_info "environment option: \$CI_LEGACY_MAKEFILES: '$CI_LEGACY_MAKEFILES'"
    ci_info "environment option: \$CI_MAKE: '$CI_MAKE'"
    ci_info "environment option: \$CI_MAKE_FLAGS: '$CI_MAKE_FLAGS'"
    ci_info "environment option: \$CI_MAKE_VARS: '$CI_MAKE_VARS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_CPP: '$CI_CPP'"
    ci_info "environment option: \$CI_CPP_FLAGS: '$CI_CPP_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_LD: '$CI_LD'"
    ci_info "environment option: \$CI_LD_FLAGS: '$CI_LD_FLAGS'"
    ci_info "environment option: \$CI_LIBS: '$CI_LIBS'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
    ci_info "environment option: \$CI_NO_CLEAN: '$CI_NO_CLEAN'"
    ci_info "executable: \$CI_MAKE: $(command -V "$CI_MAKE")"
    [[ $CI_CC ]] &&
        ci_info "executable: \$CI_CC: $(command -V "$CI_CC")"
    [[ $CI_CPP ]] &&
        ci_info "executable: \$CI_CPP: $(command -V "$CI_CPP")"
    [[ $CI_AR ]] &&
        ci_info "executable: \$CI_AR: $(command -V "$CI_AR")"
    [[ $CI_RANLIB ]] &&
        ci_info "executable: \$CI_RANLIB: $(command -V "$CI_RANLIB")"
    [[ $CI_LD ]] &&
        ci_info "executable: \$CI_LD: $(command -V "$CI_LD")"
    ci_info "## END OF CONFIGURATION ##"
}

function ci_build_legacy {
    ci_info "## START OF BUILD ##"
    # Initialize ALL_CC_FLAGS and ALL_LD_FLAGS as strings.
    local ALL_CC_FLAGS="$CI_CC_FLAGS"
    local ALL_LD_FLAGS="$CI_LD_FLAGS"
    [[ $CI_SANITIZERS ]] && {
        ALL_CC_FLAGS="-fsanitize=$CI_SANITIZERS ${ALL_CC_FLAGS:-"-O2"}"
        ALL_LD_FLAGS="-fsanitize=$CI_SANITIZERS $ALL_LD_FLAGS"
    }
    # Initialize ALL_MAKE_FLAGS and ALL_MAKE_VARS as arrays.
    local -a ALL_MAKE_FLAGS=($CI_MAKE_FLAGS)
    local -a ALL_MAKE_VARS=()
    [[ $CI_CC ]] && ALL_MAKE_VARS+=(CC="$CI_CC")
    [[ $ALL_CC_FLAGS ]] && ALL_MAKE_VARS+=(CFLAGS="$ALL_CC_FLAGS")
    [[ $CI_CPP ]] && ALL_MAKE_VARS+=(CPP="$CI_CPP")
    [[ $CI_CPP_FLAGS ]] && ALL_MAKE_VARS+=(CPPFLAGS="$CI_CPP_FLAGS")
    [[ $CI_AR ]] && ALL_MAKE_VARS+=(
        AR="${CI_AR:-ar}"
        AR_RC="${CI_AR:-ar} rc"
    )
    [[ $CI_RANLIB ]] && ALL_MAKE_VARS+=(RANLIB="$CI_RANLIB")
    [[ $CI_LD ]] && ALL_MAKE_VARS+=(LD="$CI_LD")
    [[ $ALL_LD_FLAGS ]] && ALL_MAKE_VARS+=(LDFLAGS="$ALL_LD_FLAGS")
    ALL_MAKE_VARS+=(LIBS="$CI_LIBS")
    ALL_MAKE_VARS+=($CI_MAKE_VARS)
    # Build!
    ci_spawn cd "$CI_SRCDIR"
    local MY_MAKEFILE
    for MY_MAKEFILE in $CI_LEGACY_MAKEFILES
    do
        ci_info "using makefile: $MY_MAKEFILE"
        ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                            "${ALL_MAKE_FLAGS[@]}" \
                            "${ALL_MAKE_VARS[@]}"
        [[ $CI_NO_TEST ]] ||
            ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                                "${ALL_MAKE_FLAGS[@]}" \
                                "${ALL_MAKE_VARS[@]}" \
                                test
        [[ $CI_NO_CLEAN ]] ||
            ci_spawn "$CI_MAKE" -f "$MY_MAKEFILE" \
                                "${ALL_MAKE_FLAGS[@]}" \
                                "${ALL_MAKE_VARS[@]}" \
                                clean
    done
    ci_info "## END OF BUILD ##"
}

ci_init_legacy
ci_trace_legacy
[[ $# -eq 0 ]] || {
    ci_info "note: this program accepts environment options only"
    ci_err "unexpected command arguments: '$*'"
}
ci_build_legacy
