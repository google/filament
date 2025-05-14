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

rm -rf out/check-headers
mkdir -p out/check-headers

TMP_FILE=out/check-headers/temp.cpp
echo "Checking that public headers compile independently..."
for include in "${includes[@]}"; do
    rm -f ${TMP_FILE}
    echo "Checking ${include}"
    if [[ "${include}" == "utils/Systrace.h" ]]; then
        # A necessary define before we can include utils/Systrace.h
        echo "#define SYSTRACE_TAG SYSTRACE_TAG_DISABLED" >> ${TMP_FILE}
    fi
    echo "#include <${include}>" >> ${TMP_FILE}
    clang -std=c++17 -I "${FILAMENT_HEADERS}" ${TMP_FILE} -c -o /dev/null
done
echo "Done!"
