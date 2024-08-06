#!/bin/bash
set -e

function print_help {
    local self_name=$(basename "$0")
    echo "Usage:"
    echo "    $self_name [options] <build_type1> [<build_type2> ...]"
    echo ""
    echo "Options:"
    echo "    -h"
    echo "        Print this help message."
    echo "    -c"
    echo "        Clean build directories."
    echo "    -f"
    echo "        Always invoke CMake before incremental builds."
    echo ""
    echo "Build types:"
    echo "    release"
    echo "        Release build only"
    echo "    debug"
    echo "        Debug build only"
    echo ""
    echo "Examples:"
    echo "    Desktop release build:"
    echo "        \$ ./$self_name release"
    echo ""
    echo "    Desktop debug and release builds:"
    echo "        \$ ./$self_name debug release"
    echo ""
 }

# Requirements
CMAKE_MAJOR=3
CMAKE_MINOR=19

# Internal variables
ISSUE_CLEAN=false

ISSUE_DEBUG_BUILD=false
ISSUE_RELEASE_BUILD=false

ISSUE_CMAKE_ALWAYS=false

INSTALL_COMMAND=install

BUILD_GENERATOR=Ninja
BUILD_COMMAND=ninja

# Functions

function build_clean {
    echo "Cleaning build directories..."
    rm -Rf cmake-*
    rm -Rf dist
}

function build_desktop_target {
    local lc_target=$(echo "$1" | tr '[:upper:]' '[:lower:]')

    echo "Building ${lc_target} in cmake-${lc_target}..."
    mkdir -p "cmake-${lc_target}"

    pushd "cmake-${lc_target}" > /dev/null

    if [[ ! -d "CMakeFiles" ]] || [[ "${ISSUE_CMAKE_ALWAYS}" == "true" ]]; then
        CC=/usr/bin/clang CXX=/usr/bin/clang++ CXXFLAGS=-stdlib=libc++ cmake \
            -G "${BUILD_GENERATOR}" \
            -DCMAKE_BUILD_TYPE="$1" \
            -DCMAKE_INSTALL_PREFIX="../dist" \
            ..
    fi
    ${BUILD_COMMAND}

    if [[ "${INSTALL_COMMAND}" ]]; then
        echo "Installing ${lc_target} in ../dist..."
        ${BUILD_COMMAND} ${INSTALL_COMMAND}
    fi

    popd > /dev/null
}

function build_desktop {
    if [[ "${ISSUE_DEBUG_BUILD}" == "true" ]]; then
        build_desktop_target "Debug" "$1"
    fi

    if [[ "${ISSUE_RELEASE_BUILD}" == "true" ]]; then
        build_desktop_target "Release" "$1"
    fi
}

function validate_build_command {
    set +e
    # Make sure CMake is installed
    local cmake_binary=$(command -v cmake)
    if [[ ! "${cmake_binary}" ]]; then
        echo "Error: could not find cmake, exiting"
        exit 1
    fi

    # Make sure Ninja is installed
    if [[ "${BUILD_COMMAND}" == "ninja" ]]; then
        local ninja_binary=$(command -v ninja)
        if [[ ! "${ninja_binary}" ]]; then
            echo "Warning: could not find ninja, using make instead"
            BUILD_GENERATOR="Unix Makefiles"
            BUILD_COMMAND="make"
        fi
    fi

    set -e
}

function check_debug_release_build {
    if [[ "${ISSUE_DEBUG_BUILD}" == "true" || \
          "${ISSUE_RELEASE_BUILD}" == "true" || \
          "${ISSUE_CLEAN}" == "true" ]]; then
        "$@";
    else
        echo "You must declare a debug or release target for $@ builds."
        echo ""
        exit 1
    fi
}

# Beginning of the script

pushd "$(dirname "$0")" > /dev/null

while getopts ":hcf" opt; do
    case ${opt} in
        h)
            print_help
            exit 0
            ;;
        c)
            ISSUE_CLEAN=true
            ;;
        f)
            ISSUE_CMAKE_ALWAYS=true
            ;;
        \?)
            echo "Invalid option: -${OPTARG}" >&2
            echo ""
            print_help
            exit 1
            ;;
        :)
            echo "Option -${OPTARG} requires an argument." >&2
            echo ""
            print_help
            exit 1
            ;;
    esac
done

if [[ "$#" == "0" ]]; then
    print_help
    exit 1
fi

shift $((OPTIND - 1))

for arg; do
    if [[ $(echo "${arg}" | tr '[:upper:]' '[:lower:]') == "release" ]]; then
        ISSUE_RELEASE_BUILD=true
    elif [[ $(echo "${arg}" | tr '[:upper:]' '[:lower:]') == "debug" ]]; then
        ISSUE_DEBUG_BUILD=true
    else
        BUILD_CUSTOM_TARGETS="${BUILD_CUSTOM_TARGETS} ${arg}"
    fi
done

validate_build_command

if [[ "${ISSUE_CLEAN}" == "true" ]]; then
    build_clean
fi

check_debug_release_build build_desktop
