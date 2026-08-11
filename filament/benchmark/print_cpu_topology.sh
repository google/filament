#!/usr/bin/env bash
#
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

set -euo pipefail

REMOTE_SCRIPT='
get_part_name() {
    case "$1" in
        0xd03) echo "Cortex-A53" ;;
        0xd04) echo "Cortex-A35" ;;
        0xd05) echo "Cortex-A55" ;;
        0xd07) echo "Cortex-A57" ;;
        0xd08) echo "Cortex-A72" ;;
        0xd09) echo "Cortex-A73" ;;
        0xd0a) echo "Cortex-A75" ;;
        0xd0b) echo "Cortex-A76" ;;
        0xd0c) echo "Neoverse-N1" ;;
        0xd0d) echo "Cortex-A77" ;;
        0xd0e) echo "Cortex-A76AE" ;;
        0xd41) echo "Cortex-A78" ;;
        0xd42) echo "Cortex-A78AE" ;;
        0xd44) echo "Cortex-X1" ;;
        0xd46) echo "Cortex-A510" ;;
        0xd47) echo "Cortex-A710" ;;
        0xd48) echo "Cortex-X2" ;;
        0xd4a) echo "Neoverse-V2" ;;
        0xd4b) echo "Cortex-A78C" ;;
        0xd4c) echo "Cortex-X1C" ;;
        0xd4d) echo "Cortex-A715" ;;
        0xd4e) echo "Cortex-X3/X4" ;;
        0xd80) echo "Cortex-A520" ;;
        0xd81) echo "Cortex-A720" ;;
        0xd82) echo "Cortex-X4" ;;
        0xd84) echo "Cortex-A725" ;;
        0xd85) echo "Cortex-X925" ;;
        0x800|0x801) echo "Kryo-260/280" ;;
        0x804) echo "Kryo-485-Gold" ;;
        0x805) echo "Kryo-485-Silver" ;;
        *) echo "Unknown ($1)" ;;
    esac
}

format_ranges() {
    echo "$1" | tr "," "\n" | awk "
        NR==1 { start=\$1; prev=\$1; next }
        \$1 == prev + 1 { prev=\$1; next }
        {
            if (start == prev) printf \"%s,\", start;
            else printf \"%s-%s,\", start, prev;
            start=\$1; prev=\$1;
        }
        END {
            if (start == prev) printf \"%s\", start;
            else printf \"%s-%s\", start, prev;
        }
    "
}

printf "\n%-6s %-16s %-12s %-10s %-12s\n" "CPU" "Core Model" "Max Clock" "Capacity" "Cluster"
printf "%-6s %-16s %-12s %-10s %-12s\n" "-----" "---------------" "-----------" "---------" "-----------"

# Read /proc/cpuinfo parts
while IFS= read -r line; do
    case "$line" in
        processor*)
            cur_proc=$(echo "$line" | awk -F: "{print \$2}" | tr -d " \t\r\n")
            ;;
        "CPU part"*)
            cur_part=$(echo "$line" | awk -F: "{print \$2}" | tr -d " \t\r\n")
            eval "part_${cur_proc}=\$cur_part"
            ;;
    esac
done < /proc/cpuinfo

little_cpus=""
mid_cpus=""
big_cpus=""

for cpu_dir in /sys/devices/system/cpu/cpu[0-9]*; do
    [ -d "$cpu_dir" ] || continue
    cpu_name=$(basename "$cpu_dir")
    cpu_id=${cpu_name#cpu}

    # Max frequency
    max_freq="N/A"
    if [ -f "$cpu_dir/cpufreq/cpuinfo_max_freq" ]; then
        khz=$(cat "$cpu_dir/cpufreq/cpuinfo_max_freq" 2>/dev/null)
        [ -n "$khz" ] && max_freq=$(awk "BEGIN {printf \"%.2f GHz\", $khz / 1000000}")
    elif [ -f "$cpu_dir/cpufreq/scaling_max_freq" ]; then
        khz=$(cat "$cpu_dir/cpufreq/scaling_max_freq" 2>/dev/null)
        [ -n "$khz" ] && max_freq=$(awk "BEGIN {printf \"%.2f GHz\", $khz / 1000000}")
    fi

    # Capacity
    cap="N/A"
    if [ -f "$cpu_dir/cpu_capacity" ]; then
        cap=$(cat "$cpu_dir/cpu_capacity" 2>/dev/null)
    fi

    # Part name
    eval "part_val=\${part_${cpu_id}:-}"
    model="Unknown"
    if [ -n "$part_val" ]; then
        model=$(get_part_name "$part_val")
    fi

    # Determine cluster type based on capacity or model
    cluster="Mid"
    if [ "$cap" != "N/A" ]; then
        if [ "$cap" -le 350 ]; then
            cluster="Little"
            little_cpus="${little_cpus:+$little_cpus,}$cpu_id"
        elif [ "$cap" -ge 900 ]; then
            cluster="Prime/Big"
            big_cpus="${big_cpus:+$big_cpus,}$cpu_id"
        else
            cluster="Mid"
            mid_cpus="${mid_cpus:+$mid_cpus,}$cpu_id"
        fi
    fi

    printf "%-6s %-16s %-12s %-10s %-12s\n" "$cpu_id" "$model" "$max_freq" "$cap" "$cluster"
done

echo ""
echo "Recommended taskset commands for benchmarking:"
if [ -n "$mid_cpus" ]; then
    mid_range=$(format_ranges "$mid_cpus")
    echo "  - Mid Cluster   (Recommended): taskset -c $mid_range"
fi
if [ -n "$little_cpus" ]; then
    little_range=$(format_ranges "$little_cpus")
    echo "  - Little Cluster (Efficiency):  taskset -c $little_range"
fi
if [ -n "$big_cpus" ]; then
    big_range=$(format_ranges "$big_cpus")
    echo "  - Prime/Big Core (Peak Speed):  taskset -c $big_range"
fi
echo ""
'

if command -v getprop >/dev/null 2>&1; then
    # Running directly inside Android device shell
    sh -c "$REMOTE_SCRIPT"
else
    # Running on host machine via adb
    ADB="${ADB:-adb}"
    if ! command -v "$ADB" >/dev/null 2>&1; then
        echo "Error: adb command not found in PATH." >&2
        exit 1
    fi
    "$ADB" "$@" shell "$REMOTE_SCRIPT"
fi
