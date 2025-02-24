#!/bin/bash

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SRC_DIR}/.." >/dev/null 2>&1 && pwd )"
TESTS_DIR="$( cd "${ROOT_DIR}/tests" >/dev/null 2>&1 && pwd )"

CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
${CLANG_FORMAT} --version

show_help()
{
# Tells cat to stop reading file when EOF is detected
cat << EOF
Usage: ./clang-format-all.sh [-ah]
Format files in the SwiftShader repository
-h, --help      Display this message and exit
-a, --all       Format all files (default is to format only files active in a git CL)
EOF
# cat finishes printing
}

while [[ $# -gt 0 ]]
do
    case $1 in
        -a|--all)
            all=1
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
    esac
done

if [[ $all -eq 1 ]]
then
    for DIR in "${SRC_DIR}/Device" "${SRC_DIR}/Pipeline" "${SRC_DIR}/Reactor" "${SRC_DIR}/System" "${SRC_DIR}/Vulkan" "${SRC_DIR}/WSI" "${TESTS_DIR}"
    do
        # Double clang-format, as it seems that one pass isn't always enough
        find ${DIR} -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs ${CLANG_FORMAT} -i -style=file
        find ${DIR} -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs ${CLANG_FORMAT} -i -style=file
    done
else
    BASEDIR=$(git rev-parse --show-toplevel)
    FILES=$(git diff --name-only --diff-filter=ACM | grep '\.cpp$\|\.hpp\|\.c$\|\.h$')
    for FILE in $FILES
    do
        ${CLANG_FORMAT} -i -style=file "$BASEDIR/$FILE"
    done
fi
