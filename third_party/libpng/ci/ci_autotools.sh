#!/usr/bin/env bash
set -e

# ci_autotools.sh
# Continuously integrate libpng using the GNU Autotools.
#
# Copyright (c) 2019-2022 Cosmin Truta.
#
# This software is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h.

CI_SCRIPTNAME="$(basename "$0")"
CI_SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
CI_SRCDIR="$(dirname "$CI_SCRIPTDIR")"
CI_BUILDDIR="$CI_SRCDIR/out/autotools.build"
CI_INSTALLDIR="$CI_SRCDIR/out/autotools.install"

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

function ci_init_autotools {
    CI_SYSTEM_NAME="$(uname -s)"
    CI_MACHINE_NAME="$(uname -m)"
    CI_MAKE="${CI_MAKE:-make}"
    # Set CI_CC to cc by default, if the cc command is available.
    # The configure script defaults CC to gcc, which is not always a good idea.
    [[ -x $(command -v cc) ]] && CI_CC="${CI_CC:-cc}"
    # Ensure that the CI_ variables that cannot be customized reliably are not initialized.
    [[ ! $CI_CONFIGURE_VARS ]] || ci_err "unexpected: \$CI_CONFIGURE_VARS='$CI_CONFIGURE_VARS'"
    [[ ! $CI_MAKE_VARS ]] || ci_err "unexpected: \$CI_MAKE_VARS='$CI_MAKE_VARS'"
}

function ci_trace_autotools {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "system name: $CI_SYSTEM_NAME"
    ci_info "machine hardware name: $CI_MACHINE_NAME"
    ci_info "source directory: $CI_SRCDIR"
    ci_info "build directory: $CI_BUILDDIR"
    ci_info "install directory: $CI_INSTALLDIR"
    ci_info "environment option: \$CI_CONFIGURE_FLAGS: '$CI_CONFIGURE_FLAGS'"
    ci_info "environment option: \$CI_MAKE: '$CI_MAKE'"
    ci_info "environment option: \$CI_MAKE_FLAGS: '$CI_MAKE_FLAGS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_CPP: '$CI_CPP'"
    ci_info "environment option: \$CI_CPP_FLAGS: '$CI_CPP_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_LD: '$CI_LD'"
    ci_info "environment option: \$CI_LD_FLAGS: '$CI_LD_FLAGS'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
    ci_info "environment option: \$CI_NO_INSTALL: '$CI_NO_INSTALL'"
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

function ci_build_autotools {
    ci_info "## START OF BUILD ##"
    # Export the configure build environment.
    [[ $CI_CC ]] && ci_spawn export CC="$CI_CC"
    [[ $CI_CC_FLAGS ]] && ci_spawn export CFLAGS="$CI_CC_FLAGS"
    [[ $CI_CPP ]] && ci_spawn export CPP="$CI_CPP"
    [[ $CI_CPP_FLAGS ]] && ci_spawn export CPPFLAGS="$CI_CPP_FLAGS"
    [[ $CI_AR ]] && ci_spawn export AR="$CI_AR"
    [[ $CI_RANLIB ]] && ci_spawn export RANLIB="$CI_RANLIB"
    [[ $CI_LD ]] && ci_spawn export CPP="$CI_LD"
    [[ $CI_LD_FLAGS ]] && ci_spawn export LDFLAGS="$CI_LD_FLAGS"
    [[ $CI_SANITIZERS ]] && {
        ci_spawn export CFLAGS="-fsanitize=$CI_SANITIZERS ${CFLAGS:-"-O2"}"
        ci_spawn export LDFLAGS="-fsanitize=$CI_SANITIZERS $LDFLAGS"
    }
    # Build and install.
    ci_spawn rm -fr "$CI_BUILDDIR" "$CI_INSTALLDIR"
    ci_spawn mkdir -p "$CI_BUILDDIR"
    ci_spawn cd "$CI_BUILDDIR"
    ci_spawn "$CI_SRCDIR/configure" --prefix="$CI_INSTALLDIR" $CI_CONFIGURE_FLAGS
    ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS
    [[ $CI_NO_TEST ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS test
    [[ $CI_NO_INSTALL ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS install
    [[ $CI_NO_CLEAN ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS clean
    [[ $CI_NO_CLEAN ]] || ci_spawn "$CI_MAKE" $CI_MAKE_FLAGS distclean
    ci_info "## END OF BUILD ##"
}

ci_init_autotools
ci_trace_autotools
[[ $# -eq 0 ]] || {
    ci_info "note: this program accepts environment options only"
    ci_err "unexpected command arguments: '$*'"
}
ci_build_autotools
