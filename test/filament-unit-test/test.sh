#!/bin/bash
set -e

# Runs the unit test binaries listed in test_list.txt concurrently. Each gtest binary runs its
# tests on a single thread, so running the binaries one after another leaves most of a multi-core
# machine idle. Entries marked with shards=N are further split with gtest's own sharding
# (GTEST_TOTAL_SHARDS and GTEST_SHARD_INDEX), so that one slow binary does not set the pace.
# Entries marked exclusive run afterwards, one at a time; test_list.txt explains when to use it.
#
# This script has to run on the bash 3.2 that macOS ships, which rules out associative arrays and
# `wait -n`.

FULL_TEST=false
JOBS=""
JOB_TIMEOUT=600

function print_help {
    echo "Usage: $0 [-f] [-j jobs] [-t seconds]"
    echo "  -f    Run full test suite (ignores --gtest_filter arguments in test_list.txt)"
    echo "  -j    Number of test processes to run at once (default: number of CPUs)"
    echo "  -t    Seconds before a single test process is killed (default: ${JOB_TIMEOUT})."
    echo "        Only enforced when the timeout command (GNU coreutils) is installed."
}

while getopts "fj:t:h" opt; do
    case ${opt} in
        f)
            FULL_TEST=true
            ;;
        j)
            JOBS="${OPTARG}"
            ;;
        t)
            JOB_TIMEOUT="${OPTARG}"
            ;;
        h)
            print_help
            exit 0
            ;;
        \?)
            print_help
            exit 1
            ;;
    esac
done

if [[ -z "${JOBS}" ]]; then
    JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
fi
if ! [[ "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: -j expects a positive integer, got '${JOBS}'."
    exit 1
fi
if ! [[ "${JOB_TIMEOUT}" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: -t expects a positive integer, got '${JOB_TIMEOUT}'."
    exit 1
fi

# Logs are only printed once every job finishes, so a single hung test would otherwise run until
# CI cancels the whole job, and no log would ever be shown. The timeout turns a hang into an
# ordinary failure whose log is printed with the others, and the last "[ RUN ]" line in that log
# names the hung test. Stock macOS has no timeout command, so there the jobs run unguarded.
TIMEOUT_CMD=()
if command -v timeout > /dev/null 2>&1; then
    # -k follows up with SIGKILL if the process ignores the initial SIGTERM.
    TIMEOUT_CMD=(timeout -k 30 "${JOB_TIMEOUT}")
fi

# Move to repository root
cd "$(dirname "$0")/../.."

TEST_LIST=test/filament-unit-test/test_list.txt
BUILD_DIR=out/cmake-debug
RESULTS_DIR=out/test-results

echo "Running filament unit tests with up to ${JOBS} processes..."

# One entry per job in each of these parallel arrays. A job is a whole binary, or one shard of it.
JOB_LABELS=()
JOB_BINARIES=()
JOB_ARGS=()
JOB_SHARD_INDICES=()
JOB_SHARD_TOTALS=()
JOB_OUT_DIRS=()
JOB_EXCLUSIVE=()

# The entries carry gtest filter patterns such as RenderingTest.*, which must reach the binary
# as written rather than be expanded against the files in the working directory.
set -f

MISSING=false
while read -r line || [[ -n "${line}" ]]; do
    # Skip blank lines and comments.
    if [[ -z "${line// /}" || "${line}" == \#* ]]; then
        continue
    fi

    # shellcheck disable=SC2086
    set -- ${line}
    shards=1
    exclusive=false
    while [[ "$1" == shards=* || "$1" == exclusive ]]; do
        if [[ "$1" == exclusive ]]; then
            exclusive=true
        else
            shards="${1#shards=}"
            if ! [[ "${shards}" =~ ^[1-9][0-9]*$ ]]; then
                echo "Error: invalid shard count in '${line}'."
                exit 1
            fi
        fi
        shift
    done
    binary="$1"
    shift

    args=()
    for arg in "$@"; do
        if [[ "${FULL_TEST}" == "true" && "${arg}" == --gtest_filter=* ]]; then
            continue
        fi
        args+=("${arg}")
    done

    if [[ ! -x "${BUILD_DIR}/${binary}" ]]; then
        echo "Error: Test binary ${BUILD_DIR}/${binary} not found or not executable."
        MISSING=true
        continue
    fi

    test_name=$(basename "${binary}")
    # Cleared once per binary rather than per job, so that results left by an earlier run with a
    # different shard count do not linger beside the new ones.
    rm -rf "${RESULTS_DIR:?}/${test_name}"
    for ((i = 0; i < shards; i++)); do
        if [[ ${shards} -gt 1 ]]; then
            JOB_LABELS+=("${test_name} (shard $((i + 1)) of ${shards})")
            JOB_OUT_DIRS+=("${RESULTS_DIR}/${test_name}/shard${i}")
        else
            JOB_LABELS+=("${test_name}")
            JOB_OUT_DIRS+=("${RESULTS_DIR}/${test_name}")
        fi
        JOB_BINARIES+=("${binary}")
        JOB_ARGS+=("${args[*]}")
        JOB_SHARD_INDICES+=("${i}")
        JOB_SHARD_TOTALS+=("${shards}")
        JOB_EXCLUSIVE+=("${exclusive}")
    done
done < "${TEST_LIST}"

set +f

if [[ "${MISSING}" == "true" ]]; then
    echo ""
    echo "Some test targets are missing. Please build Filament with the 'debug' target first."
    echo "For example: ./build.sh debug"
    exit 1
fi

# Runs one job with its output captured to a log file, so that concurrent jobs do not interleave.
# Prints a single status line when it finishes. Runs in a background subshell, so the exports only
# affect this job.
function run_job {
    local index=$1
    local label="${JOB_LABELS[${index}]}"
    local out_dir="${JOB_OUT_DIRS[${index}]}"
    local shard_total="${JOB_SHARD_TOTALS[${index}]}"
    local start=${SECONDS}
    local status=0
    local args

    mkdir -p "${out_dir}"

    if [[ ${shard_total} -gt 1 ]]; then
        export GTEST_TOTAL_SHARDS="${shard_total}"
        export GTEST_SHARD_INDEX="${JOB_SHARD_INDICES[${index}]}"
    else
        unset GTEST_TOTAL_SHARDS GTEST_SHARD_INDEX
    fi

    set -f
    # shellcheck disable=SC2206
    args=(${JOB_ARGS[${index}]})
    set +f

    ${TIMEOUT_CMD[@]+"${TIMEOUT_CMD[@]}"} \
        "./${BUILD_DIR}/${JOB_BINARIES[${index}]}" ${args[@]+"${args[@]}"} \
        --gtest_output="xml:${out_dir}/sponge_log.xml" > "${out_dir}/test.log" 2>&1 || status=$?

    if [[ ${status} -eq 0 ]]; then
        echo "[PASS] ${label} ($((SECONDS - start))s)"
    elif [[ ${#TIMEOUT_CMD[@]} -gt 0 && ${status} -eq 124 ]]; then
        echo "[TIMEOUT] ${label} (killed after ${JOB_TIMEOUT}s)"
    else
        echo "[FAIL] ${label} ($((SECONDS - start))s, exit code ${status})"
    fi
    return ${status}
}

# Without this handler, an interrupted run would leave its tests running and lose their logs.
# Background jobs start with SIGINT ignored, so Ctrl-C stops only this script. A CI cancellation,
# including the job's timeout-minutes, also kills the script before any log is printed. This is a
# fallback to the per-job timeout, because CI kills the script a few seconds after signalling it.
function on_signal {
    local code=$1
    local running=()
    local children
    local i
    trap - INT TERM
    # Stderr is discarded while jobs are killed and reaped, because bash otherwise prints a
    # "Terminated" notice for each of them in the middle of the logs below.
    {
        for i in ${PIDS[@]+"${!PIDS[@]}"}; do
            if kill -0 "${PIDS[${i}]}"; then
                running+=("${i}")
                # Look up the test process before killing the job's subshell, which would reparent
                # it. The subshell goes first so that it exits without reporting a failure.
                children=$(pgrep -P "${PIDS[${i}]}" || true)
                kill -TERM "${PIDS[${i}]}" || true
                # shellcheck disable=SC2086
                kill -TERM ${children} || true
            fi
        done
        for i in ${running[@]+"${running[@]}"}; do
            wait "${PIDS[${i}]}" || true
        done
    } 2> /dev/null
    for i in ${running[@]+"${running[@]}"}; do
        echo ""
        echo "===== ${JOB_LABELS[${i}]} (interrupted; last 100 lines) ====="
        tail -n 100 "${JOB_OUT_DIRS[${i}]}/test.log" 2> /dev/null || true
    done
    exit "${code}"
}

function is_failed {
    [[ " ${FAILED[*]} " == *" $1 "* ]]
}

# Indexed by job, and filled in as jobs start.
PIDS=()
FAILED=()
trap 'on_signal 130' INT
trap 'on_signal 143' TERM

# All jobs except the exclusive ones run concurrently, up to JOBS at a time.
for ((i = 0; i < ${#JOB_LABELS[@]}; i++)); do
    if [[ "${JOB_EXCLUSIVE[${i}]}" == "true" ]]; then
        continue
    fi
    # Wait for a free slot.
    while [[ $(jobs -rp | wc -l) -ge ${JOBS} ]]; do
        sleep 0.2
    done
    run_job "${i}" &
    PIDS[${i}]=$!
done
for i in ${PIDS[@]+"${!PIDS[@]}"}; do
    if ! wait "${PIDS[${i}]}"; then
        FAILED+=("${i}")
    fi
done

# Exclusive jobs run afterwards, one at a time, so that each has the machine to itself. They are
# for tests that rely on wall-clock budgets and fail when other processes compete for the CPU.
# Each job still runs in the background so that the signal handler can fire while it is waited on.
for ((i = 0; i < ${#JOB_LABELS[@]}; i++)); do
    if [[ "${JOB_EXCLUSIVE[${i}]}" != "true" ]]; then
        continue
    fi
    run_job "${i}" &
    PIDS[${i}]=$!
    if ! wait "${PIDS[${i}]}"; then
        FAILED+=("${i}")
    fi
done
trap - INT TERM

# Print the captured output in list order. On GitHub Actions the logs of passing jobs are folded
# away, and the logs of failing jobs are printed last and unfolded, so they sit next to the summary.
function print_log {
    local index=$1
    echo ""
    echo "===== ${JOB_LABELS[${index}]} ====="
    cat "${JOB_OUT_DIRS[${index}]}/test.log"
}

for ((i = 0; i < ${#JOB_LABELS[@]}; i++)); do
    if is_failed "${i}"; then
        continue
    fi
    if [[ -n "${GITHUB_ACTIONS}" ]]; then
        echo "::group::${JOB_LABELS[${i}]}"
    fi
    print_log "${i}"
    if [[ -n "${GITHUB_ACTIONS}" ]]; then
        echo "::endgroup::"
    fi
done

for ((i = 0; i < ${#JOB_LABELS[@]}; i++)); do
    if is_failed "${i}"; then
        print_log "${i}"
    fi
done

echo ""
if [[ ${#FAILED[@]} -gt 0 ]]; then
    echo "${#FAILED[@]} of ${#JOB_LABELS[@]} test jobs failed:"
    for ((i = 0; i < ${#JOB_LABELS[@]}; i++)); do
        if is_failed "${i}"; then
            echo "    ${JOB_LABELS[${i}]}"
        fi
    done
    exit 1
fi

echo "All tests passed."
