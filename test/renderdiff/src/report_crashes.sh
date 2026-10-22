# Copyright (C) 2026 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/bash

# Collects a backtrace for every render that died on a signal. A crashed render otherwise reports
# only its exit code and whatever it managed to print, and because these crashes are intermittent,
# the run that produces one is the only chance to collect the evidence. Run unconditionally after
# the render; costs nothing when nothing crashed.
#
# The two platforms leave different evidence:
#
#   Linux writes a core, which gdb reads. Where it lands is kernel.core_pattern, which only root
#   can change, so the CI points it at CORE_DUMP_DIR; otherwise the core is usually named "core" in
#   the process's working directory, so the per-test directories are searched too.
#
#   macOS writes no core for these binaries, with the core limit raised or not and whether or not
#   they carry get-task-allow, but ReportCrash leaves a symbolicated .ips report. format_ips.py
#   renders those as ordinary backtraces.
#
# Reports land in CRASH_OUTPUT_DIR, which every renderdiff caller already uploads, and are echoed
# to stdout as well. Cores are deleted once read, unless RENDERDIFF_KEEP_CORES is set, because a
# debug core runs to hundreds of megabytes.

source `dirname $0`/preamble.sh

SRC_DIR=`dirname $0`

if ! mkdir -p "${CRASH_OUTPUT_DIR}" 2> /dev/null; then
    # The backtrace is still echoed below, so fall back rather than lose it to a failed redirect.
    CRASH_OUTPUT_DIR="$(mktemp -d)"
    echo "Could not write to the renderdiff output tree; using ${CRASH_OUTPUT_DIR} instead."
fi

os_name=$(uname -s)
found_any=false

# Prints a report to the log as well as leaving it in the artifacts.
function echo_report_() {
    echo "===== $1 ====="
    cat "$2"
    echo "===== end $1 ====="
}

# Only the reports this run produced are of interest. generate.sh drops the marker just before
# rendering; without one, fall back to a generous age cutoff.
if [[ -f "${RENDER_START_MARKER}" ]]; then
    recent_args=(-newer "${RENDER_START_MARKER}")
else
    recent_args=(-mmin -120)
fi

# ---------------------------------------------------------------------------------------------
# Core files.
# ---------------------------------------------------------------------------------------------

# Roots to search, as "directory:maxdepth".
core_search_roots=("${CORE_DUMP_DIR}:1" "/tmp/renderdiff:6")

# Names the binary a core came from, which is what makes the backtrace symbolic. gdb does not
# derive it from the core on its own, so all three of these are worth trying.
function executable_for_core_() {
    local core_name
    core_name=$(basename "$1")

    # The CI pattern is core.%E.<pid>.<time>, where %E is the executable's full path with the
    # slashes replaced by exclamation marks. Not %e, which is the comm name truncated to 15
    # characters, and so would not survive helloskinningbuffer_morebones.
    if [[ "${core_name}" == *'!'* ]]; then
        local encoded="${core_name#core.}"
        encoded="${encoded%.*}"  # drop the timestamp
        encoded="${encoded%.*}"  # drop the pid
        local decoded="${encoded//!//}"
        if [[ -x "${decoded}" ]]; then
            echo "${decoded}"
            return
        fi
    fi

    # A core written by a pattern we did not set records the executable anyway, and `file` reads it
    # back out of the note section.
    if command -v file > /dev/null 2>&1; then
        local execfn
        execfn=$(file "$1" 2> /dev/null | grep -o "execfn: '[^']*'" | head -1 | cut -d"'" -f2)
        if [[ -n "${execfn}" ]] && [[ -x "${execfn}" ]]; then
            echo "${execfn}"
            return
        fi
    fi

    # Last resort, for a core named after the short form of a sample.
    local exe_name
    exe_name=$(echo "${core_name}" | cut -d. -f2)
    local candidate="$(pwd)/out/cmake-debug/samples/${exe_name}"
    if [[ -x "${candidate}" ]]; then
        echo "${candidate}"
    fi
}

for root_spec in "${core_search_roots[@]}"; do
    root="${root_spec%:*}"
    depth="${root_spec##*:}"
    [[ -d "${root}" ]] || continue
    while IFS= read -r core; do
        [[ -s "${core}" ]] || continue
        found_any=true

        core_name=$(basename "${core}")
        report="${CRASH_OUTPUT_DIR}/${core_name}.backtrace.txt"
        exe=$(executable_for_core_ "${core}")

        echo "Extracting a backtrace from ${core} (executable: ${exe:-unknown})"

        if ! command -v gdb > /dev/null 2>&1; then
            echo "gdb is not installed, so ${core} cannot be read." > "${report}"
        else
            # The core goes through -c. As a bare positional argument gdb would take it for an
            # executable, and report only that the file has no symbols.
            #
            # `thread apply all bt` first, because with a software rasterizer the interesting
            # thread is rarely the one the debugger stops on. `bt full` then repeats the crashing
            # thread with its locals.
            gdb --batch \
                -ex "set pagination off" \
                -ex "set print pretty on" \
                -ex "thread apply all bt" \
                -ex "echo \n==== crashing thread, with locals ====\n" \
                -ex "bt full" \
                -ex "quit" \
                -c "${core}" ${exe:+"${exe}"} > "${report}" 2>&1 || true
        fi

        echo_report_ "${core_name}" "${report}"

        if [[ -z "${RENDERDIFF_KEEP_CORES}" ]]; then
            rm -f "${core}"
        fi
    done < <(find "${root}" -maxdepth "${depth}" -type f -name 'core*' "${recent_args[@]}" \
                 2> /dev/null)
done

# ---------------------------------------------------------------------------------------------
# macOS crash reports.
# ---------------------------------------------------------------------------------------------

if [[ "$os_name" == "Darwin" ]]; then
    for dir in "${HOME}/Library/Logs/DiagnosticReports" "/Library/Logs/DiagnosticReports"; do
        [[ -d "${dir}" ]] || continue
        while IFS= read -r ips; do
            found_any=true

            ips_name=$(basename "${ips}")
            report="${CRASH_OUTPUT_DIR}/${ips_name}.backtrace.txt"

            echo "Reading the crash report ${ips}"
            # The raw report is kept alongside the rendered one: it carries register state and
            # binary images that the readable form leaves out.
            cp "${ips}" "${CRASH_OUTPUT_DIR}/${ips_name}" 2> /dev/null || true
            python3 "${SRC_DIR}/format_ips.py" "${ips}" > "${report}" 2>&1 || true

            echo_report_ "${ips_name}" "${report}"
        done < <(find "${dir}" -maxdepth 1 -type f -name '*.ips' "${recent_args[@]}" 2> /dev/null)
    done
fi

if [[ "${found_any}" == "false" ]]; then
    echo "No crash evidence found; nothing to report."
fi

# Never fails the job: an unreadable core is a lost diagnostic, not a test result.
exit 0
