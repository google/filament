#!/usr/bin/env bash

set -e

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Combine multiple single-architecture or universal libraries into xc-frameworks."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME [options] <path>..."
    echo ""
    echo "Options:"
    echo "    -h"
    echo "       Print this help message."
    echo "    -o"
    echo "       Output directory to store the xcframeworks libraries."
    echo ""
    echo "Example:"
    echo "    Given the follow directories:"
    echo "    ├── universal/"
    echo "    │   └── libfoo.a <- universal library - ensure they share the same platform (iphone/simulator)"
    echo "    └── arm64-iphoneos/"
    echo "        └── libfoo.a <- arm64 iphoneos platform"
    echo ""
    echo "    $SELF_NAME -o frameworks/ arm64-iphoneos/ universal/"
    echo ""
    echo "    Each library is combined into an xc-framework:"
    echo "    └── frameworks/"
    echo "        └── libfoo.xcframework"
    echo ""
    echo "Each <path> should contain one or more single or universal-architecture static libraries."
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

echo "Creating universal libraries from path: ${LEADER_PATH}..."

# Loop through each file in the leader path. For each library we find, we'll collect additional
# architectures in the other paths and combine them all into a universal library.
for FILE in "${LEADER_PATH}"/*.a; do
    [ -f "${FILE}" ] || continue

    # The static library file name, like "libfilament.a"
    LIBRARY_NAME="${FILE##*/}"

    INPUT_FILES=("-library ${LEADER_PATH}/${LIBRARY_NAME}")
    for ARCH_PATH in "${PATHS[@]:1}"; do
        THIS_FILE="${ARCH_PATH}/${LIBRARY_NAME}"
        if [[ -f "${THIS_FILE}" ]]; then
            INPUT_FILES+=("-library ${THIS_FILE}")
        else
            echo "Error: ${THIS_FILE} does not exist."
            exit 1
        fi
    done

    # Remove the .a extension
    LIBRARY_NAME="${LIBRARY_NAME%.a}"
    
    OUTPUT="${OUTPUT_DIR}/${LIBRARY_NAME}.xcframework"
    # Delete previous xcframework
    rm -rf $OUTPUT

    # Create the xcframework command and execute it
    CMD="xcodebuild -create-xcframework ${INPUT_FILES[@]} -output ${OUTPUT}"    
    eval $CMD
done
