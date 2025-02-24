#!/bin/bash

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")"/.. >/dev/null 2>&1 && pwd )"
SRC_DIR=${ROOT_DIR}/src
TESTS_DIR=${ROOT_DIR}/tests

# Presubmit Checks Script.
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}
GOFMT=${GOFMT:-gofmt}

git config --global --add safe.directory '*'

if test -t 1; then
  ncolors=$(tput colors)
  if test -n "$ncolors" && test $ncolors -ge 8; then
    normal="$(tput sgr0)"
    red="$(tput setaf 1)"
    green="$(tput setaf 2)"
  fi
fi

function check() {
  local name=$1; shift
  echo -n "Running check $name... "

  if ! "$@"; then
    echo "${red}FAILED${normal}"
    echo "  Error executing: $@";
    exit 1
  fi

  if ! git diff --quiet HEAD; then
    echo "${red}FAILED${normal}"
    echo "  Git workspace not clean:"
    git --no-pager diff -p HEAD
    echo "${red}Check $name failed.${normal}"
    exit 1
  fi

  echo "${green}OK${normal}"
}

function run_copyright_headers() {
  tmpfile=`mktemp`
  for suffix in "cpp" "hpp" "go" "h"; do
    # Grep flag '-L' print files that DO NOT match the copyright regex
    # Grep seems to match "(standard input)", filter this out in the for loop output
    find ${SRC_DIR} -type f -name "*.${suffix}" | xargs grep -L "Copyright .* The SwiftShader Authors\|Microsoft Visual C++ generated\|GNU Bison"
  done | grep -v "(standard input)" > ${tmpfile}
  if test -s ${tmpfile}; then
    # tempfile is NOT empty
    echo "${red}Copyright issue in these files:"
    cat ${tmpfile}
    rm ${tmpfile}
    echo "${normal}"
    return 1
  else
    rm ${tmpfile}
    return 0
  fi
}

function run_clang_format() {
  ${SRC_DIR}/clang-format-all.sh
}

function run_gofmt() {
  find ${SRC_DIR} ${TESTS_DIR} -name "*.go" | xargs $GOFMT -w
}

function run_check_build_files() {
  go run ${TESTS_DIR}/check_build_files/main.go --root="${ROOT_DIR}"
}

function run_scan_sources() {
  python3 ${TESTS_DIR}/scan_sources/main.py ${SRC_DIR}
}

# Ensure we are clean to start out with.
check "git workspace must be clean" true

# Check copyright headers
check copyright-headers run_copyright_headers

# Check clang-format.
check clang-format run_clang_format

# Check gofmt.
check gofmt run_gofmt

# Check build files.
check "build files don't reference non-existent files" run_check_build_files

# Check source files.
check scan_sources run_scan_sources

echo
echo "${green}All check completed successfully.${normal}"
