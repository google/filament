#!/usr/bin/env bash

set -e

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Combine multiple platform-specific libraries into XCFramework bundles."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME [options] <path>..."
    echo ""
    echo "Options:"
    echo "    -h"
    echo "       Print this help message."
    echo "    -o"
    echo "       Output directory to store the XCFrameworks."
    echo ""
    echo "Example:"
    echo "    $SELF_NAME -o xcframeworks/ iphoneos/ iphonesimulator/"
    echo ""
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

mkdir -p "${OUTPUT_DIR}"

# Use the first path as the leader to find all .a files
LEADER_PATH="${PATHS[0]}"

for FILE in "${LEADER_PATH}"/*.a; do
    [ -f "${FILE}" ] || continue

    LIBRARY_NAME="${FILE##*/}"
    FRAMEWORK_NAME="${LIBRARY_NAME%.a}.xcframework"
    
    echo "Creating XCFramework for: ${LIBRARY_NAME}"
    
    CMD="xcodebuild -create-xcframework"
    
    for PLATFORM_PATH in "${PATHS[@]}"; do
        LIB_PATH="${PLATFORM_PATH}/${LIBRARY_NAME}"
        if [[ -f "${LIB_PATH}" ]]; then
            CMD="${CMD} -library ${LIB_PATH}"
        else
            echo "Warning: ${LIB_PATH} does not exist, skipping this platform for ${LIBRARY_NAME}"
        fi
    done
    
    CMD="${CMD} -output ${OUTPUT_DIR}/${FRAMEWORK_NAME}"
    
    # Remove existing framework if it exists
    rm -rf "${OUTPUT_DIR}/${FRAMEWORK_NAME}"
    
    eval "${CMD}"
done
