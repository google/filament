#!/usr/bin/env bash
set -e

# ci_cmake.sh
# Continuously integrate libpng using CMake.
#
# Copyright (c) 2019-2022 Cosmin Truta.
#
# This software is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h.

CI_SCRIPTNAME="$(basename "$0")"
CI_SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
CI_SRCDIR="$(dirname "$CI_SCRIPTDIR")"
CI_BUILDDIR="$CI_SRCDIR/out/cmake.build"
CI_INSTALLDIR="$CI_SRCDIR/out/cmake.install"

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

function ci_init_cmake {
    CI_SYSTEM_NAME="$(uname -s)"
    CI_MACHINE_NAME="$(uname -m)"
    CI_CMAKE="${CI_CMAKE:-cmake}"
    CI_CTEST="${CI_CTEST:-ctest}"
    CI_CMAKE_BUILD_TYPE="${CI_CMAKE_BUILD_TYPE:-Release}"
    [[ -x $(command -v ninja) ]] && CI_CMAKE_GENERATOR="${CI_CMAKE_GENERATOR:-Ninja}"
    if [[ $CI_CMAKE_GENERATOR == "Visual Studio"* ]]
    then
        # Initialize the CI_...DIR_NATIVE variables, for the benefit of
        # the native Windows build tools. The regular CI_...DIR variables
        # can only be used inside Bash-on-Windows.
        mkdir -p "$CI_BUILDDIR"
        mkdir -p "$CI_INSTALLDIR"
        if [[ -x $CYGPATH ]]
        then
            CI_SRCDIR_NATIVE="$("$CYGPATH" -w "$CI_SRCDIR")"
            CI_BUILDDIR_NATIVE="$("$CYGPATH" -w "$CI_BUILDDIR")"
            CI_INSTALLDIR_NATIVE="$("$CYGPATH" -w "$CI_INSTALLDIR")"
        else
            CI_SRCDIR_NATIVE="$(cd "$CI_SRCDIR" ; pwd -W || pwd -P)"
            CI_BUILDDIR_NATIVE="$(cd "$CI_BUILDDIR" ; pwd -W || pwd -P)"
            CI_INSTALLDIR_NATIVE="$(cd "$CI_INSTALLDIR" ; pwd -W || pwd -P)"
        fi
        # Clean up incidental mixtures of Windows and Bash-on-Windows
        # environment variables, to avoid confusing MSBuild.
        [[ $TEMP && ( $Temp || $temp ) ]] && unset TEMP
        [[ $TMP && ( $Tmp || $tmp ) ]] && unset TMP
        # Ensure that CI_CMAKE_GENERATOR_PLATFORM is initialized for this generator.
        [[ $CI_CMAKE_GENERATOR_PLATFORM ]] || ci_err "missing: \$CI_CMAKE_GENERATOR_PLATFORM"
    fi
}

function ci_trace_cmake {
    ci_info "## START OF CONFIGURATION ##"
    ci_info "system name: $CI_SYSTEM_NAME"
    ci_info "machine hardware name: $CI_MACHINE_NAME"
    ci_info "source directory: $CI_SRCDIR"
    [[ $CI_SRCDIR_NATIVE ]] &&
        ci_info "source directory (native): $CI_SRCDIR_NATIVE"
    ci_info "build directory: $CI_BUILDDIR"
    [[ $CI_BUILDDIR_NATIVE ]] &&
        ci_info "build directory (native): $CI_BUILDDIR_NATIVE"
    ci_info "install directory: $CI_INSTALLDIR"
    [[ $CI_INSTALLDIR_NATIVE ]] &&
        ci_info "install directory (native): $CI_INSTALLDIR_NATIVE"
    ci_info "environment option: \$CI_CMAKE: '$CI_CMAKE'"
    ci_info "environment option: \$CI_CMAKE_GENERATOR: '$CI_CMAKE_GENERATOR'"
    ci_info "environment option: \$CI_CMAKE_GENERATOR_PLATFORM: '$CI_CMAKE_GENERATOR_PLATFORM'"
    ci_info "environment option: \$CI_CMAKE_BUILD_TYPE: '$CI_CMAKE_BUILD_TYPE'"
    ci_info "environment option: \$CI_CMAKE_BUILD_FLAGS: '$CI_CMAKE_BUILD_FLAGS'"
    ci_info "environment option: \$CI_CMAKE_VARS: '$CI_CMAKE_VARS'"
    ci_info "environment option: \$CI_CTEST: '$CI_CTEST'"
    ci_info "environment option: \$CI_CTEST_FLAGS: '$CI_CTEST_FLAGS'"
    ci_info "environment option: \$CI_CC: '$CI_CC'"
    ci_info "environment option: \$CI_CC_FLAGS: '$CI_CC_FLAGS'"
    ci_info "environment option: \$CI_AR: '$CI_AR'"
    ci_info "environment option: \$CI_RANLIB: '$CI_RANLIB'"
    ci_info "environment option: \$CI_SANITIZERS: '$CI_SANITIZERS'"
    ci_info "environment option: \$CI_NO_TEST: '$CI_NO_TEST'"
    ci_info "environment option: \$CI_NO_INSTALL: '$CI_NO_INSTALL'"
    ci_info "environment option: \$CI_NO_CLEAN: '$CI_NO_CLEAN'"
    ci_info "executable: \$CI_CMAKE: $(command -V "$CI_CMAKE")"
    ci_info "executable: \$CI_CTEST: $(command -V "$CI_CTEST")"
    [[ $CI_CMAKE_GENERATOR == *"Ninja"* ]] &&
        ci_info "executable: $(command -V ninja)"
    [[ $CI_CC ]] &&
        ci_info "executable: \$CI_CC: $(command -V "$CI_CC")"
    [[ $CI_AR ]] &&
        ci_info "executable: \$CI_AR: $(command -V "$CI_AR")"
    [[ $CI_RANLIB ]] &&
        ci_info "executable: \$CI_RANLIB: $(command -V "$CI_RANLIB")"
    ci_info "## END OF CONFIGURATION ##"
}

function ci_build_cmake {
    ci_info "## START OF BUILD ##"
    ci_spawn "$(command -v "$CI_CMAKE")" --version
    ci_spawn "$(command -v "$CI_CTEST")" --version
    # Initialize ALL_CC_FLAGS as a string.
    local ALL_CC_FLAGS="$CI_CC_FLAGS"
    [[ $CI_SANITIZERS ]] && ALL_CC_FLAGS="-fsanitize=$CI_SANITIZERS $ALL_CC_FLAGS"
    # Initialize ALL_CMAKE_VARS, ALL_CMAKE_BUILD_FLAGS and ALL_CTEST_FLAGS as arrays.
    local -a ALL_CMAKE_VARS=()
    [[ $CI_CC ]] && ALL_CMAKE_VARS+=(-DCMAKE_C_COMPILER="$CI_CC")
    [[ $ALL_CC_FLAGS ]] && ALL_CMAKE_VARS+=(-DCMAKE_C_FLAGS="$ALL_CC_FLAGS")
    [[ $CI_AR ]] && ALL_CMAKE_VARS+=(-DCMAKE_AR="$CI_AR")
    [[ $CI_RANLIB ]] && ALL_CMAKE_VARS+=(-DCMAKE_RANLIB="$CI_RANLIB")
    ALL_CMAKE_VARS+=(-DCMAKE_BUILD_TYPE="$CI_CMAKE_BUILD_TYPE")
    ALL_CMAKE_VARS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
    [[ $CI_NO_TEST ]] && ALL_CMAKE_VARS+=(-DPNG_TESTS=OFF)
    ALL_CMAKE_VARS+=($CI_CMAKE_VARS)
    local -a ALL_CMAKE_BUILD_FLAGS=($CI_CMAKE_BUILD_FLAGS)
    local -a ALL_CTEST_FLAGS=($CI_CTEST_FLAGS)
    # Initialize SRCDIR_NATIVE and INSTALLDIR_NATIVE.
    local SRCDIR_NATIVE="${CI_SRCDIR_NATIVE:-"$CI_SRCDIR"}"
    local INSTALLDIR_NATIVE="${CI_INSTALLDIR_NATIVE:-"$CI_INSTALLDIR"}"
    # Export the CMake environment variables.
    [[ $CI_CMAKE_GENERATOR ]] &&
        ci_spawn export CMAKE_GENERATOR="$CI_CMAKE_GENERATOR"
    [[ $CI_CMAKE_GENERATOR_PLATFORM ]] &&
        ci_spawn export CMAKE_GENERATOR_PLATFORM="$CI_CMAKE_GENERATOR_PLATFORM"
    # Build and install.
    ci_spawn rm -fr "$CI_BUILDDIR" "$CI_INSTALLDIR"
    ci_spawn mkdir -p "$CI_BUILDDIR"
    ci_spawn cd "$CI_BUILDDIR"
    ci_spawn "$CI_CMAKE" "${ALL_CMAKE_VARS[@]}" \
                         -DCMAKE_INSTALL_PREFIX="$INSTALLDIR_NATIVE" \
                         "$SRCDIR_NATIVE"
    ci_spawn "$CI_CMAKE" --build . \
                         --config "$CI_CMAKE_BUILD_TYPE" \
                         "${ALL_CMAKE_BUILD_FLAGS[@]}"
    [[ $CI_NO_TEST ]] ||
        ci_spawn "$CI_CTEST" --build-config "$CI_CMAKE_BUILD_TYPE" \
                             "${ALL_CTEST_FLAGS[@]}"
    [[ $CI_NO_INSTALL ]] ||
        ci_spawn "$CI_CMAKE" --build . \
                             --config "$CI_CMAKE_BUILD_TYPE" \
                             --target install \
                             "${ALL_CMAKE_BUILD_FLAGS[@]}"
    [[ $CI_NO_CLEAN ]] ||
        ci_spawn "$CI_CMAKE" --build . \
                             --config "$CI_CMAKE_BUILD_TYPE" \
                             --target clean \
                             "${ALL_CMAKE_BUILD_FLAGS[@]}"
    ci_info "## END OF BUILD ##"
}

ci_init_cmake
ci_trace_cmake
[[ $# -eq 0 ]] || {
    ci_info "note: this program accepts environment options only"
    ci_err "unexpected command arguments: '$*'"
}
ci_build_cmake
