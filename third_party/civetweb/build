#!/bin/sh
set -euo pipefail
IFS=$'\n\t'

stdout() {
  cat <<< "$@"
}

stderr() {
  cat <<< "$@" 1>&2
}

prereqs () {
  local E_BADARGS=65
  if [ $# -eq 0 ]; then
    stderr "Usage: $(basename $0) [prerequisite_program] [another_program...]"
    return $E_BADARGS
  fi
  for prog in $@; do
    hash $prog 2>&-
    if [ $? -ne 0 ]; then
      return 1
    fi
  done
}

usage() {
  if [ $# -ne 0 ]; then
    stdout $@
  fi
  stdout "Usage: $(basename $0) [options]"
  stdout
  stdout "A convenience script to quickly build the library with CMake."
  stdout
  stdout "Options:"
  stdout "  [--shared|(--static)]   Builds either a static or a shared library"
  stdout "  [--debug|(--release)]   Builds a certain variant of the library"
  stdout "  -g,--generator name     The CMake generator to use ('Unix Makefiles')"
  stdout "  -o,--output folder      The place to output the build files (./output)"
  stdout
  stdout "Examples:"
  stdout "  ./build"
  stdout "  ./build --shared --debug"
  stdout "  ./build --static --release -o ~/my-output-folder"
}

check() {
  local E_BADARGS=65
  if [ $# -ne 1 ]; then
    stderr "Usage: check prerequisite_program"
    return $E_BADARGS
  fi
  prereqs $1
  if [ $? -ne 0 ]; then
    stderr "Failed to find `$1` on the command line:"
    stderr "Please install it with your package manager"
    return 1
  fi
}

sanitize() {
  local E_BADARGS=65
  if [ $# -ne 1 ]; then
    stderr "Usage: sanitize string_to_clean"
    return $E_BADARGS
  fi
  echo $(echo "$1" | sed "s|[^A-Za-z]\+|-|g" | tr '[:upper:]' '[:lower:]')
  return 0
}

build () {
  # Get the build locations
  local src_dir=$(cd $(dirname $0); pwd -P)

  # Arguments
  local E_BADARGS=65
  local generator="Unix Makefiles"
  local shared=NO
  local build_type=Release
  local output_dir="${src_dir}/output"
  while (( "$#" )); do
    case "$1" in
      --debug) build_type=Debug;;
      --release) build_type=Release;;
      --shared) shared=YES;;
      --static) shared=NO;;
      --output) shift; out="$1";;
      -o) shift; output_dir="$1";;
      --generator) shift; generator="$1";;
      -g) shift; generator="$1";;
      --help) usage; return 0;;
      --) shift; break;;
      -*) usage "Bad argument $1"; return ${E_BADARGS};;
      *) break;;
    esac
    shift
  done

  # Update the build folder
  local build_dir=${output_dir}/build
  local install_dir=${output_dir}/install

  # Create the build folder
  mkdir -p ${build_dir}

  # Enter the build folder
  cd ${build_dir}
  trap 'cd ${src_dir}' INT TERM EXIT

  # Do the CMake configuration
  check cmake
  cmake -G ${generator} -DCMAKE_BUILD_TYPE=${build_type} -DBUILD_SHARED_LIBS:BOOL=${shared} ${src_dir}

  # Do the build
  if [ "${generator}" = "Unix Makefiles" ]; then
    check make
    make all test
  else
    stderr "Unknown build system for ${generator}, go to ${build_dir} and run the correct build program"
  fi

  # Do the install
  cmake -DCMAKE_INSTALL_PREFIX="${install_dir}" -P "${build_dir}/cmake_install.cmake"

  # Return to the correct folder
  trap - INT TERM EXIT
  cd ${src_dir}

  # Notify the user
  stdout "Built files are available at ${install_dir}"
}

# If the script was not sourced we need to run the function
case "$0" in
  *"build")
    build "$@"
    ;;
esac
