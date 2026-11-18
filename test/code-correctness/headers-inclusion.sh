#!/usr/bin/env bash

set -e

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Check that public headers compile independently."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME <public include directory>"
    echo ""
    echo "Example:"
    echo "    $SELF_NAME out/release/filament/include"
    echo ""
    echo "Dependencies: clang"
    echo ""
    echo "Options:"
    echo "    -h"
    echo "       Print this help message and exit."
}

while getopts "h" opt; do
    case ${opt} in
        h)
            print_help
            exit 0
            ;;
        *)
            print_help
            exit 1
            ;;
    esac
done
shift $((OPTIND - 1))

if [[ "$#" -ne 1 ]]; then
    print_help
    exit 1
fi

FILAMENT_HEADERS="$1"

includes=()
pushd "$FILAMENT_HEADERS" >/dev/null
for f in $(find . -name '*.h'); do
    # Ignore platform headers. These may contain platform-specific includes.
    if [[ "$f" == *backend/platforms/* ]]; then
        continue
    fi
    include_path="${f#./}"    # strip leading ./
    includes+=("${include_path}")
done
popd >/dev/null

if [[ "${#includes[@]}" -eq 0 ]]; then
    echo "Error: no public headers found under ${FILAMENT_HEADERS}" >&2
    exit 1
fi

if [[ -n "${GITHUB_CLANG_VERSION:-}" ]] && command -v "clang-${GITHUB_CLANG_VERSION}" >/dev/null 2>&1; then
    CLANG_BIN="clang-${GITHUB_CLANG_VERSION}"
else
    CLANG_BIN="clang"
fi

# Check if the system has getopt
HAS_SYSTEM_GETOPT=0
if echo "#include <getopt.h>" | "${CLANG_BIN}" -x c++ -std=c++17 -E - > /dev/null 2>&1; then
    HAS_SYSTEM_GETOPT=1
fi

JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
export FILAMENT_HEADERS HAS_SYSTEM_GETOPT CLANG_BIN

echo "Checking that ${#includes[@]} public headers compile independently using up to ${JOBS} workers..."
printf '%s\0' "${includes[@]}" | xargs -0 -n 8 -P "${JOBS}" bash -c '
    rc=0
    for include in "$@"; do
        src=""
        case "${include}" in
            "utils/Systrace.h")
                # A necessary define before we can include utils/Systrace.h
                src+="#define SYSTRACE_TAG SYSTRACE_TAG_DISABLED"$'\''\n'\''
                ;;
            "utils/getopt.h")
                if [[ "${HAS_SYSTEM_GETOPT}" -eq 1 ]]; then
                    src+="#define HAS_SYSTEM_GETOPT 1"$'\''\n'\''
                fi
                ;;
        esac
        src+="#include <${include}>"$'\''\n'\''
        # Filament is built internally with C++20, but we maintain C++17 compatibility (for the
        # time being) in our public headers to support projects on older toolchains.
        if ! err=$(printf "%s" "${src}" | "${CLANG_BIN}" -x c++ -std=c++17 -fsyntax-only -I "${FILAMENT_HEADERS}" - 2>&1); then
            printf "FAILED: %s\n%s\n" "${include}" "${err}" >&2
            rc=1
        fi
    done
    exit "${rc}"
' _
echo "Done!"
