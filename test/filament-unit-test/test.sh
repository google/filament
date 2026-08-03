#!/bin/bash
set -e

FULL_TEST=false

while getopts "f" opt; do
    case ${opt} in
        f)
            FULL_TEST=true
            ;;
        \?)
            echo "Usage: $0 [-f]"
            echo "  -f    Run full test suite (ignores --gtest_filter arguments in test_list.txt)"
            exit 1
            ;;
    esac
done

# Move to repository root
cd "$(dirname "$0")/../.."

echo "Running filament unit tests..."

# Check that the test binaries exist
MISSING=false
while read -r test; do
    # test might contain arguments, extract just the binary name
    # shellcheck disable=SC2086
    set -- ${test}
    if [[ ! -x "out/cmake-debug/${1}" ]]; then
        echo "Error: Test binary out/cmake-debug/${1} not found or not executable."
        MISSING=true
    fi
done < test/filament-unit-test/test_list.txt

if [[ "${MISSING}" == "true" ]]; then
    echo ""
    echo "Some test targets are missing. Please build Filament with the 'debug' target first."
    echo "For example: ./build.sh debug"
    exit 1
fi

# Run the tests
while read -r test; do
    # shellcheck disable=SC2086
    set -- ${test}
    test_name=$(basename "$1")
    echo "Running ${test_name}..."
    if [[ "${FULL_TEST}" == "true" ]]; then
        test=$(echo "$test" | sed 's/--gtest_filter=[^ ]*//g')
    fi
    # shellcheck disable=SC2086
    ./out/cmake-debug/${test} --gtest_output="xml:out/test-results/${test_name}/sponge_log.xml"
done < test/filament-unit-test/test_list.txt

echo "All tests passed."
