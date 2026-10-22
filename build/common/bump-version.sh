#!/usr/bin/env bash

set -e

case "$(uname -s)" in
    Darwin*) IS_DARWIN=1;;
    *) ;;
esac

function print_help {
    local SELF_NAME
    SELF_NAME=$(basename "$0")
    echo "$SELF_NAME. Update the Filament version number."
    echo ""
    echo "Usage:"
    echo "    $SELF_NAME <new version>"
    echo ""
    echo "<new version> should be a 3 part semantic version, such as 1.9.3"
    echo ""
    echo "This script does not interface with git. It is up to the user to commit the change."
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

VERSION_REGEX="[[:digit:]]+.[[:digit:]]+.[[:digit:]]+"
NEW_VERSION="$1"

function replace {
    FIND_STR="${1//\{\{VERSION\}\}/${VERSION_REGEX}}"
    REPLACE_STR="${1//\{\{VERSION\}\}/${NEW_VERSION}}"
    local FILE_NAME="$2"
    if [ $IS_DARWIN == 1 ]; then
        sed -i '' -E "s/${FIND_STR}/${REPLACE_STR}/" "${FILE_NAME}"
    else
        sed -i -E "s/${FIND_STR}/${REPLACE_STR}/" "${FILE_NAME}"
    fi
}

# The following are the canonical locations where the Filament version number is referenced.

replace \
    "implementation 'com.google.android.filament:filament-android:{{VERSION}}'" \
    README.md

replace \
    "pod 'Filament', '~> {{VERSION}}'" \
    README.md

replace \
    "VERSION_NAME={{VERSION}}" \
    android/gradle.properties

replace \
    "spec.version = \"{{VERSION}}\"" \
    ios/CocoaPods/Filament.podspec

replace \
    ":http => \"https:\/\/github.com\/google\/filament\/releases\/download\/v{{VERSION}}\/filament-v{{VERSION}}-ios.tgz\" }" \
    ios/CocoaPods/Filament.podspec

replace \
    "\"version\": \"{{VERSION}}\"" \
    web/filament-js/package.json
