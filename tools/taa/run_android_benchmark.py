#!/usr/bin/env python3
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

"""Android benchmark runner and Systrace/Perfetto ground-truth verifier for taa_harness.

Features:
1. Configures Android ADPF Fixed Performance Mode (`cmd power set-fixed-performance-mode-enabled`)
   with guaranteed `finally:` restoration so the device never stays in fixed-clocks mode.
2. Checks thermal headroom (`dumpsys thermalservice`) before and after each benchmark run.
3. Installs `sample-taa-harness-debug.apk` (which packages `libtaa_harness.so`, IBL assets, and
   the 5 benchmark glTF models) and invokes
   `com.google.android.filament.taaharness/.TaaHarnessInstrumentation` via `am instrument -w`.
4. Benchmarks Filament's built-in TAA (`default`) or optional `-s` settings presets, pulls the
   resulting JSON files, and prints the side-by-side `--compare` scorecard when comparing multiple
   presets.
5. Supports `--verify-systrace` (and standalone `--verify-only <trace> <json>`) to cross-verify
   `taa_harness`'s self-reported GPU metrics against Perfetto/Systrace ground truth via
   `trace_processor`.
"""

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass, field
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time
from typing import Any


APK_PACKAGE = "com.google.android.filament.taaharness"
APK_INSTRUMENTATION = f"{APK_PACKAGE}/.TaaHarnessInstrumentation"
APP_FILES_DIR = f"/sdcard/Android/data/{APK_PACKAGE}/files"
TRACE_TMP_DIR = "/data/local/tmp/taa_harness_trace"
DEFAULT_APK_REL_PATH = (
    "android/samples/sample-taa-harness/build/outputs/apk/debug/sample-taa-harness-debug.apk"
)


@dataclass
class ScenarioVerification:
    regime: str
    upscaling: float
    expected_frames: int
    systrace_frame_count: int
    json_window_wall_ms: float
    systrace_window_wall_ms: float
    wall_diff_ms: float
    has_gpu_slices: bool = False
    json_gpu_ms: float = 0.0
    systrace_gpu_ms: float = 0.0
    gpu_diff_ms: float = 0.0
    json_upscaler_ms: float | None = None
    systrace_upscaler_ms: float | None = None
    upscaler_diff_ms: float | None = None
    passed: bool = True
    errors: list[str] = field(default_factory=list)


@dataclass
class VerificationResult:
    passed: bool
    scenarios: list[ScenarioVerification] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)


def find_trace_processor(explicit_path: str | None = None) -> str:
    """Locates the Perfetto `trace_processor` / `trace_processor_shell` executable."""
    if explicit_path:
        p = Path(explicit_path)
        if p.is_file() and os.access(p, os.X_OK):
            return str(p)
        raise FileNotFoundError(f"Specified --trace-processor not found or not executable: {p}")

    for name in ("trace_processor", "trace_processor_shell"):
        found = shutil.which(name)
        if found:
            return found

    repo_root = Path(__file__).resolve().parents[2]
    home = Path.home()
    candidates = [
        Path.cwd() / "trace_processor",
        repo_root / "trace_processor",
        home / ".local/share/perfetto/prebuilts/trace_processor_shell",
        home / ".local/share/perfetto/trace_processor",
        home / "bin/trace_processor",
        home / "bin/trace_processor_shell",
    ]
    for cand in candidates:
        if cand.is_file() and os.access(cand, os.X_OK):
            return str(cand)

    # Automatically download the official Perfetto trace_processor wrapper if not present
    wrapper_dest = home / ".local/share/perfetto/trace_processor"
    wrapper_dest.parent.mkdir(parents=True, exist_ok=True)
    proc = subprocess.run(
        ["curl", "-fsSL", "https://get.perfetto.dev/trace_processor", "-o", str(wrapper_dest)],
        check=False,
    )
    if proc.returncode == 0 and wrapper_dest.is_file():
        wrapper_dest.chmod(0o755)
        return str(wrapper_dest)

    raise FileNotFoundError(
        "Could not find `trace_processor` or `trace_processor_shell` in PATH or "
        "~/.local/share/perfetto/prebuilts/"
    )


def run_trace_processor_sql(trace_processor_bin: str, trace_path: Path, sql: str) -> list[dict[str, str]]:
    """Executes a SQL query against `trace_path` using `trace_processor` and returns rows as dicts."""
    cmd = [trace_processor_bin, "-Q", sql, str(trace_path)]
    proc = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if proc.returncode != 0:
        raise RuntimeError(
            f"trace_processor failed (exit {proc.returncode}):\n{proc.stderr.strip()}"
        )
    lines = [line for line in proc.stdout.splitlines() if line.strip()]
    if not lines:
        return []
    reader = csv.DictReader(io.StringIO("\n".join(lines)))
    return list(reader)


def median_sorted(values: list[float]) -> float:
    if not values:
        return 0.0
    s = sorted(values)
    n = len(s)
    if n == 1:
        return s[0]
    pos = 0.5 * (n - 1)
    lo = int(pos)
    hi = min(n - 1, lo + 1)
    frac = pos - lo
    return s[lo] * (1.0 - frac) + s[hi] * frac


def within_tolerance(a: float, b: float, abs_tol_ms: float, rel_tol: float) -> bool:
    limit = max(abs_tol_ms, rel_tol * max(abs(a), abs(b)))
    return abs(a - b) <= limit


def verify_benchmark_against_systrace(
    trace_path: Path | str,
    json_path: Path | str,
    trace_processor_bin: str | None = None,
    tolerance_ms: float = 0.15,
    tolerance_pct: float = 0.05,
) -> VerificationResult:
    """Verifies that a `taa_harness` benchmark JSON matches a Perfetto/Systrace trace.

    Checks:
    1. Every `perf_*` regime in the JSON has a matching `taa_harness::timed_window::<regime>_x<u>`
       slice in the trace.
    2. Each timed window contains the exact expected number of `taa_harness::timed_frame` slices.
    3. Each timed window's wall-clock duration per frame in the trace matches the JSON's
       `window_wall_frame_ms` within `max(tolerance_ms, tolerance_pct * systrace_wall_ms)`.
    4. When GPU slices are present in the trace inside the timed windows, each scenario's GPU frame
       duration (`frame_gpu_ms_p50`) and net upscaler delta (`upscaler_gpu_ms_p50`) match the
       Systrace GPU slice durations within `max(tolerance_ms, tolerance_pct * systrace_gpu_ms)`.
    """
    trace_file = Path(trace_path)
    json_file = Path(json_path)
    tp = find_trace_processor(trace_processor_bin)

    with open(json_file, "r", encoding="utf-8") as f:
        data: dict[str, Any] = json.load(f)

    expected_perf_frames = int(data.get("perf_frames", 128))
    records = data.get("records", [])

    # Group JSON metrics by (regime, upscaling)
    metrics_by_run: dict[tuple[str, float], dict[str, float]] = {}
    for rec in records:
        regime = str(rec["regime"])
        if not regime.startswith("perf_"):
            continue
        upscaling = float(rec["upscaling"])
        key = (regime, upscaling)
        metrics_by_run.setdefault(key, {})[str(rec["metric"])] = float(rec["value"])

    result = VerificationResult(passed=True)
    if not metrics_by_run:
        result.passed = False
        result.errors.append(f"No `perf_*` records found in {json_file}")
        return result

    # Query all timed_window slices and their enclosed timed_frame counts from the trace.
    window_sql = """
    SELECT
        w.id AS window_id,
        w.name AS window_name,
        w.ts AS start_ts,
        w.ts + w.dur AS end_ts,
        w.dur / 1e6 AS window_dur_ms,
        COUNT(f.id) AS frame_count
    FROM slice w
    LEFT JOIN slice f
      ON f.name = 'taa_harness::timed_frame'
     AND f.ts >= w.ts
     AND f.ts < (w.ts + w.dur)
    WHERE w.name GLOB 'taa_harness::timed_window::*'
      AND w.dur > 0
    GROUP BY w.id, w.name, w.ts, w.dur
    ORDER BY w.ts;
    """
    window_rows = run_trace_processor_sql(tp, trace_file, window_sql)
    windows_by_name: dict[str, dict[str, Any]] = {}
    for row in window_rows:
        windows_by_name[row["window_name"]] = {
            "window_id": int(row["window_id"]),
            "start_ts": int(row["start_ts"]),
            "end_ts": int(row["end_ts"]),
            "window_dur_ms": float(row["window_dur_ms"]),
            "frame_count": int(row["frame_count"]),
        }

    # Query GPU slices inside each timed_window. Matches either hardware GPU tracks (`gpu_track` /
    # `gpu_render_stage` / `android_gpu_work_period` / track name containing `GPU`) or explicit
    # `taa_harness::gpu_frame` slices in synthetic traces.
    gpu_sql = """
    SELECT
        w.name AS window_name,
        t.type AS track_type,
        s.dur / 1e6 AS gpu_dur_ms
    FROM slice w
    JOIN slice s
      ON s.ts >= w.ts
     AND s.ts < (w.ts + w.dur)
     AND s.dur > 0
    JOIN track t
      ON s.track_id = t.id
    WHERE w.name GLOB 'taa_harness::timed_window::*'
      AND w.dur > 0
      AND s.id != w.id
      AND s.name != 'taa_harness::timed_frame'
      AND (
          t.type GLOB '*gpu*'
          OR t.name GLOB '*GPU*'
          OR t.name GLOB '*gpu*'
          OR s.category GLOB '*gpu*'
          OR s.name GLOB '*gpu_frame*'
      )
    ORDER BY w.ts, s.ts;
    """
    gpu_rows = run_trace_processor_sql(tp, trace_file, gpu_sql)
    gpu_durs_by_window: dict[str, list[float]] = {}
    gpu_track_types_by_window: dict[str, set[str]] = {}
    for row in gpu_rows:
        wname = row["window_name"]
        gpu_durs_by_window.setdefault(wname, []).append(float(row["gpu_dur_ms"]))
        gpu_track_types_by_window.setdefault(wname, set()).add(str(row.get("track_type", "")))

    systrace_gpu_p50_by_key: dict[tuple[str, float], float] = {}

    # First pass: verify each run's window existence, frame count, wall duration, and GPU slices.
    for (regime, upscaling), run_metrics in sorted(metrics_by_run.items()):
        up_str = f"{upscaling:g}"
        win_name = f"taa_harness::timed_window::{regime}_x{up_str}"
        expected_frames = int(run_metrics.get("timed_frames_count", expected_perf_frames))
        json_wall_ms = run_metrics.get("window_wall_frame_ms", 0.0)
        json_gpu_ms = run_metrics.get("frame_gpu_ms_p50", 0.0)

        if win_name not in windows_by_name:
            sv = ScenarioVerification(
                regime=regime,
                upscaling=upscaling,
                expected_frames=expected_frames,
                systrace_frame_count=0,
                json_window_wall_ms=json_wall_ms,
                systrace_window_wall_ms=0.0,
                wall_diff_ms=json_wall_ms,
                passed=False,
                errors=[f"Missing Systrace slice '{win_name}' in {trace_file.name}"],
            )
            result.passed = False
            result.errors.extend(sv.errors)
            result.scenarios.append(sv)
            continue

        win = windows_by_name[win_name]
        st_frames = win["frame_count"]
        st_wall_ms = win["window_dur_ms"] / max(1, expected_frames)
        wall_diff = abs(json_wall_ms - st_wall_ms)

        sv = ScenarioVerification(
            regime=regime,
            upscaling=upscaling,
            expected_frames=expected_frames,
            systrace_frame_count=st_frames,
            json_window_wall_ms=json_wall_ms,
            systrace_window_wall_ms=st_wall_ms,
            wall_diff_ms=wall_diff,
        )

        if st_frames != expected_frames:
            sv.passed = False
            sv.errors.append(
                f"{regime} (x{up_str}): Systrace recorded {st_frames} timed_frame slices, "
                f"expected {expected_frames}"
            )

        if not within_tolerance(json_wall_ms, st_wall_ms, tolerance_ms, tolerance_pct):
            sv.passed = False
            sv.errors.append(
                f"{regime} (x{up_str}): window_wall_frame_ms mismatch: "
                f"JSON={json_wall_ms:.4f} ms vs Systrace={st_wall_ms:.4f} ms "
                f"(diff={wall_diff:.4f} ms)"
            )

        gpu_durs = gpu_durs_by_window.get(win_name, [])
        if gpu_durs:
            sv.has_gpu_slices = True
            sv.json_gpu_ms = json_gpu_ms
            # If there is one GPU slice per frame, take the p50 across frames; if there are
            # multiple render-stage slices per frame, sum per-window GPU time / expected_frames.
            if len(gpu_durs) == expected_frames:
                st_gpu_ms = median_sorted(gpu_durs)
            else:
                st_gpu_ms = sum(gpu_durs) / max(1, expected_frames)
            sv.systrace_gpu_ms = st_gpu_ms
            sv.gpu_diff_ms = abs(json_gpu_ms - st_gpu_ms)
            systrace_gpu_p50_by_key[(regime, upscaling)] = st_gpu_ms

            # Kernel `android_gpu_work_period` tracepoints include CSF MCU wakeup/idle transitions
            # (~0.2-0.4 ms when the GPU sleeps between short baseline frames) on top of Vulkan's
            # in-command-buffer TOP_OF_PIPE..BOTTOM_OF_PIPE timestamps.
            track_types = gpu_track_types_by_window.get(win_name, set())
            eff_gpu_tol_ms = (
                max(tolerance_ms, 0.45)
                if track_types == {"android_gpu_work_period"} and tolerance_ms == 0.15
                else tolerance_ms
            )

            if not within_tolerance(json_gpu_ms, st_gpu_ms, eff_gpu_tol_ms, tolerance_pct):
                sv.passed = False
                sv.errors.append(
                    f"{regime} (x{up_str}): frame_gpu_ms_p50 mismatch: "
                    f"JSON={json_gpu_ms:.4f} ms vs Systrace GPU={st_gpu_ms:.4f} ms "
                    f"(diff={sv.gpu_diff_ms:.4f} ms)"
                )

        if not sv.passed:
            result.passed = False
            result.errors.extend(sv.errors)
        result.scenarios.append(sv)

    # Second pass: verify net upscaler GPU cost (`upscaler_gpu_ms_p50`) against
    # (Systrace upscaler GPU - Systrace baseline GPU) when GPU slices are present.
    for sv in result.scenarios:
        if sv.regime.endswith("_baseline") or not sv.has_gpu_slices:
            continue
        base_key = (f"{sv.regime}_baseline", sv.upscaling)
        up_key = (sv.regime, sv.upscaling)
        if base_key in systrace_gpu_p50_by_key and up_key in systrace_gpu_p50_by_key:
            run_metrics = metrics_by_run[up_key]
            if "upscaler_gpu_ms_p50" in run_metrics:
                json_up_ms = run_metrics["upscaler_gpu_ms_p50"]
                st_up_ms = systrace_gpu_p50_by_key[up_key] - systrace_gpu_p50_by_key[base_key]
                sv.json_upscaler_ms = json_up_ms
                sv.systrace_upscaler_ms = st_up_ms
                sv.upscaler_diff_ms = abs(json_up_ms - st_up_ms)
                win_name = f"taa_harness::timed_window::{sv.regime}_x{sv.upscaling:g}"
                track_types = gpu_track_types_by_window.get(win_name, set())
                eff_up_tol_ms = (
                    max(tolerance_ms, 0.45)
                    if track_types == {"android_gpu_work_period"} and tolerance_ms == 0.15
                    else tolerance_ms
                )
                if not within_tolerance(json_up_ms, st_up_ms, eff_up_tol_ms, tolerance_pct):
                    sv.passed = False
                    err = (
                        f"{sv.regime} (x{sv.upscaling:g}): upscaler_gpu_ms_p50 mismatch: "
                        f"JSON={json_up_ms:.4f} ms vs Systrace delta={st_up_ms:.4f} ms "
                        f"(diff={sv.upscaler_diff_ms:.4f} ms)"
                    )
                    sv.errors.append(err)
                    result.passed = False
                    result.errors.append(err)

    return result


def print_verification_report(res: VerificationResult, trace_path: Path, json_path: Path) -> None:
    print("\n" + "=" * 88)
    print(f"  Systrace / Perfetto Ground-Truth Verification  --  {'PASS' if res.passed else 'FAIL'}")
    print(f"  Trace : {trace_path}")
    print(f"  JSON  : {json_path}")
    print("=" * 88)
    print(
        f"  {'Scenario':<28} {'Frames':>8} {'JSON Wall':>11} {'Trace Wall':>11} "
        f"{'JSON GPU':>10} {'Trace GPU':>10} {'Status':>7}"
    )
    print("-" * 88)
    for sv in res.scenarios:
        frames_str = f"{sv.systrace_frame_count}/{sv.expected_frames}"
        json_gpu_str = f"{sv.json_gpu_ms:.3f} ms" if sv.has_gpu_slices else "-"
        trace_gpu_str = f"{sv.systrace_gpu_ms:.3f} ms" if sv.has_gpu_slices else "-"
        status_str = "PASS" if sv.passed else "FAIL"
        print(
            f"  {sv.regime:<28} {frames_str:>8} {sv.json_window_wall_ms:>8.3f} ms "
            f"{sv.systrace_window_wall_ms:>8.3f} ms {json_gpu_str:>10} {trace_gpu_str:>10} "
            f"{status_str:>7}"
        )
    print("=" * 88)
    if res.errors:
        for err in res.errors:
            print(f"  ERROR: {err}", file=sys.stderr)
    print()


class AdbDevice:
    def __init__(self, serial: str | None = None):
        self.serial = serial

    def _base_cmd(self) -> list[str]:
        cmd = ["adb"]
        if self.serial:
            cmd.extend(["-s", self.serial])
        return cmd

    def run(self, args: list[str], check: bool = True) -> subprocess.CompletedProcess[str]:
        proc = subprocess.run(self._base_cmd() + args, capture_output=True, text=True, check=False)
        if proc.returncode != 0 and "not found" in (proc.stderr or ""):
            subprocess.run(self._base_cmd() + ["wait-for-device"], check=False)
            proc = subprocess.run(
                self._base_cmd() + args, capture_output=True, text=True, check=check
            )
        elif check and proc.returncode != 0:
            raise subprocess.CalledProcessError(
                proc.returncode, proc.args, output=proc.stdout, stderr=proc.stderr
            )
        return proc

    def shell(self, command: str, check: bool = True) -> subprocess.CompletedProcess[str]:
        return self.run(["shell", command], check=check)

    def push(self, local: Path, remote: str) -> None:
        self.run(["push", str(local), remote], check=True)

    def pull(self, remote: str, local: Path) -> None:
        self.run(["pull", remote, str(local)], check=True)

    def install(self, apk: Path) -> None:
        self.run(["install", "-r", "-t", str(apk)], check=True)

    def instrument_cmd(self, harness_args: str, fixed_perf: bool = True) -> str:
        fp = "1" if fixed_perf else "0"
        return (
            f'am instrument -w -e fixed_perf "{fp}" '
            f'-e args "{harness_args}" {APK_INSTRUMENTATION}'
        )

    @staticmethod
    def parse_instrument_output(stdout: str) -> tuple[int, str]:
        m = re.search(r"\[taa_harness exit_code=(-?\d+)\b", stdout)
        if m:
            return int(m.group(1)), stdout
        exit_code = 0
        out_chunks: list[str] = []
        current_lines: list[str] = []
        in_stdout = False
        for line in stdout.splitlines():
            if line.startswith("INSTRUMENTATION_STATUS: stdout="):
                in_stdout = True
                current_lines = [line[len("INSTRUMENTATION_STATUS: stdout=") :]]
            elif line.startswith("INSTRUMENTATION_STATUS: exit_code="):
                if in_stdout:
                    out_chunks.append("\n".join(current_lines))
                    in_stdout = False
                try:
                    exit_code = int(line.split("=", 1)[1].strip())
                except ValueError:
                    exit_code = 1
            elif line.startswith("INSTRUMENTATION_"):
                if in_stdout:
                    out_chunks.append("\n".join(current_lines))
                    in_stdout = False
            elif in_stdout:
                current_lines.append(line)
        if in_stdout:
            out_chunks.append("\n".join(current_lines))
        return exit_code, "\n".join(out_chunks) if out_chunks else stdout

    def get_thermal_status(self) -> int:
        proc = self.shell("dumpsys thermalservice", check=False)
        for line in proc.stdout.splitlines():
            m = re.search(r"Thermal Status:\s*(\d+)", line)
            if m:
                return int(m.group(1))
        return 0

    def set_fixed_performance_mode(self, enabled: bool) -> bool:
        val = "true" if enabled else "false"
        proc = self.shell(f"cmd power set-fixed-performance-mode-enabled {val}", check=False)
        return proc.returncode == 0 and not (proc.stderr or "").strip()


def build_harness_flags(
    *,
    api: str,
    json_filename: str,
    upscaling: str,
    perf_resolution: str,
    perf_warmup: int,
    perf_frames: int,
    run_perf: bool = True,
    run_quality: bool = False,
    dump_dirname: str | None = None,
    settings_arg: str | None = None,
) -> list[str]:
    flags = [f"-a {api}"]
    if run_perf:
        flags.extend(
            [
                "--perf",
                f"--perf-resolution {perf_resolution}",
                f"--perf-warmup {perf_warmup}",
                f"--perf-frames {perf_frames}",
            ]
        )
    if run_quality or dump_dirname:
        flags.append("-b")
    flags.extend([f"-u {upscaling}", f"-o {json_filename}"])
    if dump_dirname:
        flags.append(f"-d {dump_dirname}")
    if settings_arg:
        flags.append(f"-s {settings_arg}")
    flags.append("-q")
    return flags


def build_perfetto_config(duration_ms: int = 60000) -> str:
    return f"""
buffers: {{
    size_kb: 65536
    fill_policy: DISCARD
}}
data_sources: {{
    config {{
        name: "linux.ftrace"
        ftrace_config {{
            ftrace_events: "sched/sched_switch"
            ftrace_events: "power/cpu_frequency"
            ftrace_events: "power/gpu_frequency"
            atrace_categories: "gfx"
            atrace_categories: "view"
            atrace_apps: "*"
        }}
    }}
}}
data_sources: {{
    config {{
        name: "gpu.renderstages"
    }}
}}
data_sources: {{
    config {{
        name: "gpu.counters"
    }}
}}
duration_ms: {duration_ms}
"""


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Run taa_harness GPU performance benchmarks on Android via the "
        "sample-taa-harness APK with ADPF Fixed Performance Mode and optional "
        "Perfetto/Systrace ground-truth verification."
    )
    parser.add_argument("-s", "--serial", help="ADB device serial")
    parser.add_argument(
        "--apk",
        default=DEFAULT_APK_REL_PATH,
        help="Path to sample-taa-harness-debug.apk",
    )
    parser.add_argument(
        "--host-harness",
        help="Optional host taa_harness binary for running --compare locally",
    )
    parser.add_argument(
        "--output-dir",
        default="out/android_taa_benchmark",
        help="Directory on host to store pulled JSONs and Perfetto traces",
    )
    parser.add_argument(
        "-a",
        "--api",
        default="vulkan",
        choices=["vulkan", "opengl"],
        help="Graphics backend to benchmark on Android (default: vulkan)",
    )
    parser.add_argument(
        "-s",
        "--settings",
        default="",
        help="Optional comma-separated device/host settings JSON paths to benchmark alongside default",
    )
    parser.add_argument(
        "--quality",
        action="store_true",
        help="Also run the 11-scenario quality benchmark (-b) alongside --perf",
    )
    parser.add_argument(
        "--dump",
        action="store_true",
        help="Capture and adb-pull per-frame PNG sequences (implies --quality) for taa_viewer.py",
    )
    parser.add_argument(
        "--no-perf",
        action="store_true",
        help="Skip timed --perf windows (requires --quality or --dump)",
    )
    parser.add_argument("--perf-resolution", default="1920x1080")
    parser.add_argument("--perf-warmup", type=int, default=64)
    parser.add_argument("--perf-frames", type=int, default=128)
    parser.add_argument("-u", "--upscaling", default="2")
    parser.add_argument(
        "--verify-systrace",
        action="store_true",
        help="Record a Perfetto trace during each benchmark run and verify JSON metrics against "
        "Systrace ground truth via trace_processor",
    )
    parser.add_argument(
        "--verify-only",
        nargs=2,
        metavar=("TRACE_FILE", "JSON_FILE"),
        help="Offline mode: verify an existing Perfetto trace against a taa_harness JSON file",
    )
    parser.add_argument(
        "--trace-processor",
        help="Path to Perfetto trace_processor / trace_processor_shell binary",
    )
    parser.add_argument("--tolerance-ms", type=float, default=0.15)
    parser.add_argument("--tolerance-pct", type=float, default=0.05)
    args = parser.parse_args()

    if args.no_perf and not (args.quality or args.dump):
        print(
            "ERROR: --no-perf requires --quality or --dump so at least one schedule executes.",
            file=sys.stderr,
        )
        return 1
    if args.no_perf and args.verify_systrace:
        print(
            "ERROR: --verify-systrace verifies --perf GPU timing windows and cannot be combined "
            "with --no-perf.",
            file=sys.stderr,
        )
        return 1

    if args.verify_only:
        trace_file, json_file = Path(args.verify_only[0]), Path(args.verify_only[1])
        res = verify_benchmark_against_systrace(
            trace_file,
            json_file,
            trace_processor_bin=args.trace_processor,
            tolerance_ms=args.tolerance_ms,
            tolerance_pct=args.tolerance_pct,
        )
        print_verification_report(res, trace_file, json_file)
        return 0 if res.passed else 1

    repo_root = Path(__file__).resolve().parents[2]
    apk_path = Path(args.apk)
    if not apk_path.is_file() and (repo_root / args.apk).is_file():
        apk_path = repo_root / args.apk
    if not apk_path.is_file():
        print(
            f"ERROR: sample-taa-harness APK not found at {apk_path}.\n"
            f"Build it first with:\n"
            f"  ninja -C out/cmake-android-release-aarch64 taa_harness install\n"
            f"  cd android && ./gradlew "
            f'-Pcom.google.android.filament.dist-dir="$PWD/../out/android-release/filament" '
            f'-Pcom.google.android.filament.tools-dir="$PWD/../out/release/filament" '
            f"-Pcom.google.android.filament.abis=arm64-v8a "
            f":samples:sample-taa-harness:assembleDebug",
            file=sys.stderr,
        )
        return 1

    adb = AdbDevice(args.serial)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # 1. Thermal pre-check
    thermal = adb.get_thermal_status()
    if thermal > 1:
        print(
            f"ERROR: Device thermal status is {thermal} (> LIGHT). Wait for cooldown before "
            f"running CTS performance benchmarks.",
            file=sys.stderr,
        )
        return 1

    # 2. Install APK (bundles libtaa_harness.so, IBL, and models)
    print(f"Installing {apk_path.name}...")
    adb.install(apk_path)

    # 3. Enable ADPF Fixed Performance Mode with guaranteed teardown
    try:
        adb.shell("input keyevent KEYCODE_WAKEUP", check=False)
        adb.shell("wm dismiss-keyguard", check=False)
        fixed_ok = adb.set_fixed_performance_mode(True)
        if not fixed_ok:
            print(
                "WARNING: Could not verify mFixedPerformanceModeEnabled=true via dumpsys power",
                file=sys.stderr,
            )

        benchmark_runs: list[tuple[str, str | None]] = [("default", None)]
        for s_item in [s.strip() for s in args.settings.split(",") if s.strip()]:
            s_path = Path(s_item)
            tag = re.sub(r"[^a-zA-Z0-9_-]+", "_", s_path.stem or "custom")
            remote_settings = f"{APP_FILES_DIR}/{s_path.name}"
            if s_path.is_file():
                adb.push(s_path, remote_settings)
                benchmark_runs.append((tag, s_path.name))
            else:
                benchmark_runs.append((tag, s_item))

        pulled_jsons: list[Path] = []
        all_verified = True
        suite_t0 = time.monotonic()

        for tag, settings_arg in benchmark_runs:
            json_filename = f"taa_{tag}.json"
            remote_json = f"{APP_FILES_DIR}/{json_filename}"
            local_json = out_dir / json_filename
            dump_dirname = f"dump_{tag}" if args.dump else None
            remote_dump = f"{APP_FILES_DIR}/{dump_dirname}" if dump_dirname else None
            local_dump = (out_dir / dump_dirname) if dump_dirname else None

            flags = build_harness_flags(
                api=args.api,
                json_filename=json_filename,
                upscaling=args.upscaling,
                perf_resolution=args.perf_resolution,
                perf_warmup=args.perf_warmup,
                perf_frames=args.perf_frames,
                run_perf=not args.no_perf,
                run_quality=args.quality,
                dump_dirname=dump_dirname,
                settings_arg=settings_arg,
            )
            if remote_dump:
                adb.shell(f"rm -rf {remote_dump}", check=False)

            timeout_sec = 600 if (args.quality or args.dump) else 180
            remote_trace = f"/data/misc/perfetto-traces/taa_{tag}.perfetto-trace"
            local_trace = out_dir / f"taa_{tag}.perfetto-trace"
            if args.verify_systrace:
                cfg_text = build_perfetto_config(duration_ms=timeout_sec * 1000)
                cfg_local = out_dir / "perfetto_config.pbtxt"
                cfg_local.write_text(cfg_text, encoding="utf-8")
                adb.shell(f"mkdir -p {TRACE_TMP_DIR}", check=False)
                adb.push(cfg_local, f"{TRACE_TMP_DIR}/perfetto_config.pbtxt")
                adb.shell(f"rm -f {remote_trace}", check=False)

            print(f"\nRunning benchmark [{tag}]...")
            inst_cmd = adb.instrument_cmd(" ".join(flags), fixed_perf=fixed_ok)
            if args.verify_systrace:
                cmd_str = (
                    f"cat {TRACE_TMP_DIR}/perfetto_config.pbtxt | "
                    f"perfetto --txt -c - -o {remote_trace} & "
                    f"PERFETTO_PID=$!; sleep 1; "
                    f"timeout {timeout_sec} {inst_cmd}; "
                    f"HARNESS_RC=$?; sleep 1; kill -INT $PERFETTO_PID 2>/dev/null; "
                    f"wait $PERFETTO_PID 2>/dev/null; exit $HARNESS_RC"
                )
            else:
                cmd_str = f"timeout {timeout_sec} {inst_cmd}"

            run_t0 = time.monotonic()
            proc = adb.shell(cmd_str, check=False)
            run_elapsed = time.monotonic() - run_t0
            rc, parsed_stdout = adb.parse_instrument_output(proc.stdout)
            print(parsed_stdout)
            print(f"  [Elapsed wall time for {tag}: {run_elapsed:.2f}s]")
            if proc.stderr:
                print(proc.stderr, file=sys.stderr)
            if proc.returncode != 0 or rc != 0:
                err_code = rc if rc != 0 else proc.returncode
                print(f"ERROR: taa_harness failed with exit code {err_code}", file=sys.stderr)
                return err_code

            adb.pull(remote_json, local_json)
            pulled_jsons.append(local_json)
            if remote_dump and local_dump:
                shutil.rmtree(local_dump, ignore_errors=True)
                adb.pull(remote_dump, local_dump)

            if args.verify_systrace:
                adb.pull(remote_trace, local_trace)
                res = verify_benchmark_against_systrace(
                    local_trace,
                    local_json,
                    trace_processor_bin=args.trace_processor,
                    tolerance_ms=args.tolerance_ms,
                    tolerance_pct=args.tolerance_pct,
                )
                print_verification_report(res, local_trace, local_json)
                all_verified = all_verified and res.passed

        suite_elapsed = time.monotonic() - suite_t0
        print(f"\nTotal benchmark suite wall time: {suite_elapsed:.2f}s")
        if args.dump:
            print(f"\nInspect captured frames interactively:\n  python3 tools/taa/taa_viewer.py {out_dir}")

        if len(pulled_jsons) > 1 and args.host_harness and Path(args.host_harness).is_file():
            compare_arg = ",".join(str(p) for p in pulled_jsons)
            subprocess.run([args.host_harness, "--compare", compare_arg], check=False)

        return 0 if all_verified else 1
    finally:
        adb.set_fixed_performance_mode(False)


if __name__ == "__main__":
    sys.exit(main())
