#!/usr/bin/env bash

set -e

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Combine multiple single-architecture libraries into universal libraries."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME [options] <path>..."
    echo ""
    echo "Options:"
    echo "    -h"
    echo "       Print this help message."
    echo "    -o"
    echo "       Output directory to store the universal libraries."
    echo ""
    echo "Example:"
    echo "    Given the follow directories:"
    echo "    ├── arm64/"
    echo "    │   ├── libfoo.a"
    echo "    │   └── libbar.a"
    echo "    └── x86_64/"
    echo "        ├── libfoo.a"
    echo "        └── libbar.a"
    echo ""
    echo "    $SELF_NAME -o universal/ arm64/ x86_64/"
    echo ""
    echo "    Each library is combined into a corresponding universal library:"
    echo "    └── universal/"
    echo "        ├── libfoo.a"
    echo "        └── libbar.a"
    echo ""
    echo "Each <path> should contain one or more single-architecture static libraries."
    echo "All <path>s should contain the same number of libraries, with the same names."
}

OUTPUT_DIR=""
while getopts "ho:" opt; do
    case ${opt} in
        h)
            print_help
            exit 1
            ;;
        o)
            OUTPUT_DIR="${OPTARG}"
            ;;
        *)
            print_help
            exit 1
            ;;
    esac
done

shift $((OPTIND - 1))

PATHS=("$@")

if [[ ! "${PATHS[*]}" ]]; then
    echo "One or more paths required."
    print_help
    exit 1
fi

if [[ ! "${OUTPUT_DIR}" ]]; then
    echo "Output directory required."
    print_help
    exit 1
fi

# Create the output directory, if it doesn't exist already.
mkdir -p "${OUTPUT_DIR}"

# Use the first path as the "leader" path. All paths should contain the same number of files with
# the same names, so it doesn't matter which we chose.
LEADER_PATH="${PATHS[0]}"

# Loop through each file in the leader path. For each library we find, we'll collect additional
# architectures in the other paths and combine them all into a universal library.
for FILE in "${LEADER_PATH}"/*.a; do
    [ -f "${FILE}" ] || continue

    # The static library file name, like "libfilament.a"
    LIBRARY_NAME="${FILE##*/}"

    echo "Combining architectures for library: ${LIBRARY_NAME}"

    INPUT_FILES=("${LEADER_PATH}/${LIBRARY_NAME}")
    for ARCH_PATH in "${PATHS[@]:1}"; do
        THIS_FILE="${ARCH_PATH}/${LIBRARY_NAME}"
        if [[ -f "${THIS_FILE}" ]]; then
            INPUT_FILES+=("${THIS_FILE}")
        else
            echo "Error: ${THIS_FILE} does not exist."
            exit 1
        fi
    done

    lipo "${INPUT_FILES[@]}" -create -output "${OUTPUT_DIR}/${LIBRARY_NAME}"
done
