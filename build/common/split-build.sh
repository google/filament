#!/bin/bash

# split-build.sh decides whether the separate "prebuilt tools" pass is worth running, and
# generates the following variable:
#     $SPLIT_BUILD_OPTION
#
# Source this after build-common.sh, which supplies $BUILD_DEBUG and $BUILD_RELEASE.
#
# Background: by default build.sh compiles the host tools (matc, resgen, cmgen, ...) in a separate
# Release-mode CMake tree before building Filament itself, and the main build then imports them
# rather than rebuilding them. This exists so that matc stays fast: it compiles every .mat file in
# the tree, so a matc built with -O0 (and especially with asan) would dominate a debug build.
#
# That separation is not free. The tools pull in the same shared libraries as Filament itself
# (utils, math, filabridge, filamat, image, ...), so the two passes compile roughly 500 identical
# translation units, about a quarter of a desktop build.
#
# When the only thing being built is a release target, the tools pass and the main pass use the
# same flags, so that duplicated work buys nothing and the separate pass can be skipped. Whenever
# a debug build is involved the duplication is the intended trade and the split build must stay.
#
# Note this covers every release-only invocation of the desktop wrappers, including the ones the
# release workflow uses to produce shipped archives. The tools are then built by the main tree
# instead of the dedicated pass, which is equivalent on desktop: the only flag the tools pass
# forces is -DFILAMENT_ENABLE_EXCEPTIONS=ON, and that is already the default off iOS.

SPLIT_BUILD_OPTION=

if [[ -z "$BUILD_DEBUG" && -n "$BUILD_RELEASE" ]]; then
    SPLIT_BUILD_OPTION="-y none"
fi
