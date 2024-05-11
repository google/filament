#!/usr/bin/env bash

set -e

function print_help {
    local self_name=$(basename "$0")
    echo "This tool verifies that every Metal shader in the given Filament material file(s) successfully compiles."
    echo "Requires the matinfo tool and xcrun to be on the system PATH."
    echo ""
    echo "Usage:"
    echo "    $self_name [options] [<material> ...]"
    echo ""
    echo "Options:"
    echo "    -h"
    echo "        Print this help message."
    echo "    -w"
    echo "        Pass the -w flag to the Metal compiler, to disable warnings."
    echo ""
}

COMP_FLAGS=""
while getopts ":hw" opt; do
    case ${opt} in
        h)
            print_help
            exit 1
            ;;
        w)
            COMP_FLAGS="${COMP_FLAGS} -w"
            ;;
    esac
done

shift $((OPTIND - 1))

if [[ "$#" == "0" ]]; then
    print_help
    exit 1
fi

# Make sure matinfo and xcrun are available.
if [[ ! $(command -v matinfo) ]]; then
    echo "Error: matinfo not on PATH."
    exit 1
fi
if [[ ! $(command -v xcrun) ]]; then
    echo "Error: xcrun not on PATH."
    exit 1
fi

function check_material {
    local material="$1"

    tmpdir="$(mktemp -d /tmp/check_metal_shaders.XXXXX)"

    # First check that the material is valid.
    if [[ ! $(matinfo "${material}") ]]; then
        echo "Invalid material file: ${material}"
        exit 1
    fi

    echo "Checking that Metal shaders compile: ${material}"

    set +e
    local i=0
    while true; do
        # The file must have a .metal extension.
        metalFile="${tmpdir}/${i}.metal"

        # Extract shader i. matinfo will return a non-zero exit code if the shader doesn't exist,
        # which means we're finished with this material.
        matinfo --print-metal=${i} "${material}" > "${metalFile}" 2> /dev/null

        if [[ "$?" -ne 0 ]]; then
            break
        fi

        echo "Testing Metal shader ${i}"

        # Attempt to compile the Metal shader.
        xcrun -sdk macosx metal ${COMP_FLAGS} -c "${metalFile}" -o /dev/null

        if [[ "$?" -ne 0 ]]; then
            echo "Error compiling Metal shader: ${metalFile}"
            exit 1
        fi

        ((i++))
    done
    set -e

    rm -r "${tmpdir}"
}

for material in "$@"
do
    check_material "${material}"
done

