#!/usr/bin/env bash
set -e

function print_help {
    local SELF_NAME=`basename $0`
    echo "$SELF_NAME. Combine multiple static libraries using an archiver tool."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME <path-to-ar> <output-archive> <archives>..."
    echo ""
    echo "Notes:"
    echo "    <archives> must be a list of absolute paths to static library archives."
    echo "    This script creates a temporary working directory inside the current directory."
}

if [[ "$#" -lt 3 ]]; then
    print_help
    exit 1
fi

AR_TOOL="$1"
shift
OUTPUT_PATH="$1"
shift
ARCHIVES="$@"

# Create a temporary working directory named after the destination library.
TEMP_DIR="${OUTPUT_PATH##*/}"   # remove leading path, if any
TEMP_DIR="_${TEMP_DIR%.a}"      # remove '.a' extension
mkdir -p "${TEMP_DIR}"

pushd "${TEMP_DIR}" >/dev/null

for a in ${ARCHIVES}; do
    # Create a separate directory for each archive to combine. This is necessary to avoid overriding
    # object files from seprate archives that have the same name.
    DIR_NAME="${a##*/}"         # remove leading path, if any
    DIR_NAME="${DIR_NAME%.a}"   # remove '.a' extension
    mkdir -p "${DIR_NAME}"

    pushd "${DIR_NAME}" >/dev/null

    # Extract object files from each archive.
    "${AR_TOOL}" -x "$a"

    # Prepend the library name to the object file to ensure each object file has a unique name.
    for o in *.o; do
        mv "$o" "${DIR_NAME}_${o}"
    done

    popd >/dev/null
done

popd >/dev/null

# Combine the library files into a single static library archive.
rm -f "${OUTPUT_PATH}"
"${AR_TOOL}" -qc "${OUTPUT_PATH}" $(find "${TEMP_DIR}" -iname '*.o')

# Clean up. Delete objects in the temporary working directory. In theory we could leave the
# directory as a "cache" of extracted object files, but it complicates the logic to handle cases
# where object files are removed from a library.
find "${TEMP_DIR}" -iname "*.o" -delete

