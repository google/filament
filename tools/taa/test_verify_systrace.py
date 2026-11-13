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

"""Unit and integration tests for Systrace/Perfetto ground-truth verification of taa_harness."""

from __future__ import annotations

import copy
import json
from pathlib import Path
import tempfile
import unittest

from run_android_benchmark import (
    find_trace_processor,
    print_verification_report,
    verify_benchmark_against_systrace,
)


def build_systrace_text(
    scenarios: list[dict],
    drop_last_frame_for: str | None = None,
) -> str:
    """Builds a valid Linux ftrace/systrace text file with CPU and GPU tracks for trace_processor."""
    lines = [
        "# tracer: nop",
        "#",
        "# entries-in-buffer/entries-written: 1000/1000   #P:8",
        "#",
        "#                                _-----=> irqs-off",
        "#                               / _----=> need-resched",
        "#                              | / _---=> hardirq/softirq",
        "#                              || / _--=> preempt-depth",
        "#                              ||| /     delay",
        "#           TASK-PID     CPU#  ||||   TIMESTAMP  FUNCTION",
        "#              | |         |   ||||      |         |",
    ]

    cur_ts_us = 1_000_000.0  # start at 1000.0 s in microseconds
    pid = 4242
    gpu_pid = 9001

    def fmt_ts(ts_us: float) -> str:
        return f"{ts_us / 1_000_000.0:12.6f}"

    for idx, sc in enumerate(scenarios):
        regime = sc["regime"]
        upscaling = sc["upscaling"]
        win_name = f"taa_harness::timed_window::{regime}_x{upscaling:g}"
        cookie = idx + 1
        frame_gpu_ms_list: list[float] = sc["frame_gpu_ms"]
        frame_wall_ms: float = sc["window_wall_frame_ms"]
        n_frames = len(frame_gpu_ms_list)

        win_start_us = cur_ts_us
        lines.append(
            f"     taa_harness-{pid}  (   {pid}) [000] .... {fmt_ts(win_start_us)}: "
            f"tracing_mark_write: S|{pid}|{win_name}|{cookie}"
        )

        emit_frames = n_frames - 1 if regime == drop_last_frame_for else n_frames
        for k in range(emit_frames):
            f_start_us = win_start_us + k * frame_wall_ms * 1000.0 + 10.0
            f_end_us = f_start_us + 200.0
            lines.append(
                f"     taa_harness-{pid}  (   {pid}) [000] .... {fmt_ts(f_start_us)}: "
                f"tracing_mark_write: B|{pid}|taa_harness::timed_frame"
            )
            lines.append(
                f"     taa_harness-{pid}  (   {pid}) [000] .... {fmt_ts(f_end_us)}: "
                f"tracing_mark_write: E|{pid}"
            )

            gpu_dur_us = frame_gpu_ms_list[k] * 1000.0
            g_start_us = f_start_us + 20.0
            g_end_us = g_start_us + gpu_dur_us
            lines.append(
                f"      gpu_worker-{gpu_pid}  (  {gpu_pid}) [001] .... {fmt_ts(g_start_us)}: "
                f"tracing_mark_write: B|{gpu_pid}|taa_harness::gpu_frame"
            )
            lines.append(
                f"      gpu_worker-{gpu_pid}  (  {gpu_pid}) [001] .... {fmt_ts(g_end_us)}: "
                f"tracing_mark_write: E|{gpu_pid}"
            )

        win_end_us = win_start_us + n_frames * frame_wall_ms * 1000.0
        lines.append(
            f"     taa_harness-{pid}  (   {pid}) [000] .... {fmt_ts(win_end_us)}: "
            f"tracing_mark_write: F|{pid}|{win_name}|{cookie}"
        )
        cur_ts_us = win_end_us + 50_000.0

    return "\n".join(lines) + "\n"


def build_benchmark_json(scenarios: list[dict], perf_frames: int = 8) -> dict:
    records = []
    by_regime = {sc["regime"]: sc for sc in scenarios}
    for sc in scenarios:
        regime = sc["regime"]
        upscaling = sc["upscaling"]
        gpu_list = sorted(sc["frame_gpu_ms"])
        n = len(gpu_list)
        p50 = 0.5 * (gpu_list[n // 2 - 1] + gpu_list[n // 2]) if n % 2 == 0 else gpu_list[n // 2]
        mean = sum(gpu_list) / n
        wall_ms = sc["window_wall_frame_ms"]

        records.append({"regime": regime, "upscaling": upscaling, "metric": "timed_frames_count", "value": n})
        records.append({"regime": regime, "upscaling": upscaling, "metric": "frame_gpu_ms_p50", "value": p50})
        records.append({"regime": regime, "upscaling": upscaling, "metric": "frame_gpu_ms_mean", "value": mean})
        records.append({"regime": regime, "upscaling": upscaling, "metric": "window_wall_frame_ms", "value": wall_ms})

        if not regime.endswith("_baseline"):
            base_sc = by_regime[f"{regime}_baseline"]
            base_gpu = sorted(base_sc["frame_gpu_ms"])
            base_p50 = (
                0.5 * (base_gpu[n // 2 - 1] + base_gpu[n // 2])
                if n % 2 == 0
                else base_gpu[n // 2]
            )
            records.append(
                {
                    "regime": regime,
                    "upscaling": upscaling,
                    "metric": "baseline_gpu_ms_p50",
                    "value": base_p50,
                }
            )
            records.append(
                {
                    "regime": regime,
                    "upscaling": upscaling,
                    "metric": "upscaler_gpu_ms_p50",
                    "value": p50 - base_p50,
                }
            )

    return {
        "label": "Test_SR_Engine",
        "benchmark": False,
        "perf": True,
        "perf_width": 1920,
        "perf_height": 1080,
        "perf_warmup_frames": 16,
        "perf_frames": perf_frames,
        "fixed_performance_mode": True,
        "thermal_status_start": "NONE",
        "thermal_status_end": "NONE",
        "records": records,
    }


class VerifySystraceTest(unittest.TestCase):
    def setUp(self) -> None:
        self.tp_bin = find_trace_processor()
        self.scenarios = [
            {
                "regime": "perf_fox_skinned_run_baseline",
                "upscaling": 2.0,
                "frame_gpu_ms": [1.50] * 8,
                "window_wall_frame_ms": 1.65,
            },
            {
                "regime": "perf_fox_skinned_run",
                "upscaling": 2.0,
                "frame_gpu_ms": [2.92] * 8,
                "window_wall_frame_ms": 3.10,
            },
            {
                "regime": "perf_helmet_cam_dolly_baseline",
                "upscaling": 2.0,
                "frame_gpu_ms": [1.80] * 8,
                "window_wall_frame_ms": 1.95,
            },
            {
                "regime": "perf_helmet_cam_dolly",
                "upscaling": 2.0,
                "frame_gpu_ms": [3.15] * 8,
                "window_wall_frame_ms": 3.30,
            },
        ]

    def test_matching_systrace_and_json_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            trace_path = tmp_dir / "trace.systrace"
            json_path = tmp_dir / "metrics.json"

            trace_path.write_text(build_systrace_text(self.scenarios), encoding="utf-8")
            json_path.write_text(
                json.dumps(build_benchmark_json(self.scenarios), indent=2),
                encoding="utf-8",
            )

            res = verify_benchmark_against_systrace(
                trace_path, json_path, trace_processor_bin=self.tp_bin
            )
            print_verification_report(res, trace_path, json_path)
            self.assertTrue(res.passed, f"Expected verification to pass, got errors: {res.errors}")
            self.assertEqual(len(res.scenarios), 4)
            for sv in res.scenarios:
                self.assertTrue(sv.has_gpu_slices)
                self.assertAlmostEqual(sv.gpu_diff_ms, 0.0, places=3)

    def test_spoofed_upscaler_gpu_ms_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            trace_path = tmp_dir / "trace.systrace"
            json_path = tmp_dir / "metrics_spoofed.json"

            trace_path.write_text(build_systrace_text(self.scenarios), encoding="utf-8")
            data = build_benchmark_json(self.scenarios)
            # Spoof upscaler_gpu_ms_p50 on perf_fox_skinned_run from 1.42 ms down to 0.15 ms
            for rec in data["records"]:
                if rec["regime"] == "perf_fox_skinned_run" and rec["metric"] == "upscaler_gpu_ms_p50":
                    rec["value"] = 0.15

            json_path.write_text(json.dumps(data, indent=2), encoding="utf-8")
            res = verify_benchmark_against_systrace(
                trace_path, json_path, trace_processor_bin=self.tp_bin
            )
            self.assertFalse(res.passed)
            self.assertTrue(
                any("upscaler_gpu_ms_p50 mismatch" in err for err in res.errors),
                f"Expected upscaler_gpu_ms_p50 mismatch error, got: {res.errors}",
            )

    def test_spoofed_frame_gpu_ms_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            trace_path = tmp_dir / "trace.systrace"
            json_path = tmp_dir / "metrics_spoofed_frame.json"

            trace_path.write_text(build_systrace_text(self.scenarios), encoding="utf-8")
            data = build_benchmark_json(self.scenarios)
            for rec in data["records"]:
                if rec["regime"] == "perf_helmet_cam_dolly" and rec["metric"] == "frame_gpu_ms_p50":
                    rec["value"] = 1.90  # actual in trace is 3.15 ms

            json_path.write_text(json.dumps(data, indent=2), encoding="utf-8")
            res = verify_benchmark_against_systrace(
                trace_path, json_path, trace_processor_bin=self.tp_bin
            )
            self.assertFalse(res.passed)
            self.assertTrue(
                any("frame_gpu_ms_p50 mismatch" in err for err in res.errors),
                f"Expected frame_gpu_ms_p50 mismatch error, got: {res.errors}",
            )

    def test_dropped_frame_in_trace_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            trace_path = tmp_dir / "trace_dropped_frame.systrace"
            json_path = tmp_dir / "metrics.json"

            trace_path.write_text(
                build_systrace_text(self.scenarios, drop_last_frame_for="perf_fox_skinned_run"),
                encoding="utf-8",
            )
            json_path.write_text(
                json.dumps(build_benchmark_json(self.scenarios), indent=2),
                encoding="utf-8",
            )

            res = verify_benchmark_against_systrace(
                trace_path, json_path, trace_processor_bin=self.tp_bin
            )
            self.assertFalse(res.passed)
            self.assertTrue(
                any("timed_frame slices, expected 8" in err for err in res.errors),
                f"Expected frame count mismatch error, got: {res.errors}",
            )

    def test_spoofed_window_wall_time_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            trace_path = tmp_dir / "trace.systrace"
            json_path = tmp_dir / "metrics_spoofed_wall.json"

            trace_path.write_text(build_systrace_text(self.scenarios), encoding="utf-8")
            data = build_benchmark_json(self.scenarios)
            for rec in data["records"]:
                if rec["regime"] == "perf_fox_skinned_run" and rec["metric"] == "window_wall_frame_ms":
                    rec["value"] = 0.80  # actual in trace is 3.10 ms

            json_path.write_text(json.dumps(data, indent=2), encoding="utf-8")
            res = verify_benchmark_against_systrace(
                trace_path, json_path, trace_processor_bin=self.tp_bin
            )
            self.assertFalse(res.passed)
            self.assertTrue(
                any("window_wall_frame_ms mismatch" in err for err in res.errors),
                f"Expected window_wall_frame_ms mismatch error, got: {res.errors}",
            )

    def test_real_taa_harness_json_roundtrip(self) -> None:
        """Verifies a real JSON produced by taa_harness --perf against its corresponding trace."""
        repo_root = Path(__file__).resolve().parents[2]
        harness_bin = repo_root / "out/cmake-debug/tools/taa/taa_harness"
        if not harness_bin.is_file():
            self.skipTest(f"{harness_bin} not built")

        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            real_json = tmp_dir / "live_perf.json"
            import subprocess
            proc = subprocess.run(
                [
                    str(harness_bin),
                    "-a",
                    "vulkan",
                    "--perf",
                    "--perf-warmup",
                    "8",
                    "--perf-frames",
                    "16",
                    "--perf-resolution",
                    "1280x720",
                    "-o",
                    str(real_json),
                    "-q",
                ],
                cwd=str(repo_root),
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(proc.returncode, 0, f"taa_harness failed: {proc.stderr}")

            data = json.loads(real_json.read_text(encoding="utf-8"))
            by_regime: dict[tuple[str, float], dict] = {}
            for rec in data["records"]:
                reg = str(rec["regime"])
                if not reg.startswith("perf_"):
                    continue
                up = float(rec["upscaling"])
                entry = by_regime.setdefault(
                    (reg, up),
                    {"regime": reg, "upscaling": up, "frame_gpu_ms_map": {}, "window_wall_frame_ms": 0.0},
                )
                metric = str(rec["metric"])
                if metric.startswith("frame_gpu_ms_") and metric[len("frame_gpu_ms_"):].isdigit():
                    k = int(metric[len("frame_gpu_ms_"):])
                    entry["frame_gpu_ms_map"][k] = float(rec["value"])
                elif metric == "window_wall_frame_ms":
                    entry["window_wall_frame_ms"] = float(rec["value"])

            scenarios = []
            for (_, _), entry in sorted(by_regime.items()):
                fmap = entry["frame_gpu_ms_map"]
                frames = [fmap[k] for k in sorted(fmap.keys())]
                scenarios.append(
                    {
                        "regime": entry["regime"],
                        "upscaling": entry["upscaling"],
                        "frame_gpu_ms": frames,
                        "window_wall_frame_ms": entry["window_wall_frame_ms"],
                    }
                )

            trace_path = tmp_dir / "real_harness.systrace"
            trace_path.write_text(build_systrace_text(scenarios), encoding="utf-8")

            res = verify_benchmark_against_systrace(
                trace_path, real_json, trace_processor_bin=self.tp_bin
            )
            print_verification_report(res, trace_path, real_json)
            self.assertTrue(res.passed, f"Expected real harness JSON verification to pass: {res.errors}")
            self.assertEqual(len(res.scenarios), 10)


def _make_quality_compare_json(
    label: str,
    *,
    protocol_version: str = "2.1",
    quality_desc: str = (
        "protocol=2.1;mode=benchmark_suite;"
        "scenarios=fox_static,fox_drift,fox_obj_translate,fox_skinned_run,helmet_static,"
        "helmet_cam_rotate,helmet_cam_dolly,helmet_obj_spin,star_static,ssr_reflection,"
        "buster_drone_anim;size=512;warmup_frames=64;capture_frames=16;"
        "reference_filter=box;reference_samples=32;jitter_pattern=halton_23_x16;"
        "edge_threshold=0.0800;upscalings=2.0000"
    ),
    spatial_db: float = 31.0,
    spatial_gmsd: float = 0.028,
    spatial_hotspot_top10: float = 0.080,
    spatial_hotspot_peak: float = 0.110,
    motion_db: float = 24.5,
    motion_wake_rms: float = 0.032,
    motion_hotspot_top10: float = 0.140,
    motion_hotspot_peak: float = 0.210,
    fox_skinned_frames: list[float] | None = None,
    star_static_frames: list[float] | None = None,
) -> dict:
    import hashlib

    q_sig = hashlib.sha256(quality_desc.encode("utf-8")).hexdigest()
    records = [
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_quality_db", "value": spatial_db},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_rmse", "value": 0.028},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_gmsd", "value": spatial_gmsd},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_detail_transfer_pct", "value": 72.0},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_spurious_hf_pct", "value": 55.0},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_hotspot_top10_rmse", "value": spatial_hotspot_top10},
        {"regime": "benchmark", "upscaling": 2, "metric": "spatial_hotspot_peak_rmse", "value": spatial_hotspot_peak},
        {"regime": "benchmark", "upscaling": 2, "metric": "temporal_stability_db", "value": 30.5},
        {"regime": "benchmark", "upscaling": 2, "metric": "temporal_flicker_rms", "value": 0.030},
        {"regime": "benchmark", "upscaling": 2, "metric": "temporal_subpixel_flicker_rms", "value": 0.040},
        {"regime": "benchmark", "upscaling": 2, "metric": "temporal_drift_flicker_rms", "value": 0.035},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_robustness_db", "value": motion_db},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_robustness_rmse", "value": 0.059},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_wake_ghosting_rms", "value": motion_wake_rms},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_hotspot_top10_rmse", "value": motion_hotspot_top10},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_hotspot_peak_rmse", "value": motion_hotspot_peak},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_tracked_db", "value": 24.8},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_tracked_rmse", "value": 0.057},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_unmodeled_db", "value": 24.1},
        {"regime": "benchmark", "upscaling": 2, "metric": "motion_unmodeled_rmse", "value": 0.062},
    ]
    if fox_skinned_frames is not None:
        for k, v in enumerate(fox_skinned_frames):
            records.append(
                {"regime": "fox_skinned_run", "upscaling": 2, "metric": f"edge_rmse_frame_{k}", "value": v}
            )
    if star_static_frames is not None:
        for k, v in enumerate(star_static_frames):
            records.append(
                {"regime": "star_static", "upscaling": 2, "metric": f"edge_rmse_frame_{k}", "value": v}
            )
    return {
        "protocol_version": protocol_version,
        "schedule_signature": q_sig,
        "schedule_descriptor": quality_desc,
        "quality_schedule_signature": q_sig,
        "quality_schedule_descriptor": quality_desc,
        "label": label,
        "benchmark": True,
        "perf": False,
        "reference_filter": "box",
        "records": records,
    }


class TestCompareProtocolAndDiagnostics(unittest.TestCase):
    def setUp(self) -> None:
        repo_root = Path(__file__).resolve().parents[2]
        self.harness_bin = repo_root / "out/cmake-debug/tools/taa/taa_harness"
        if not self.harness_bin.is_file():
            self.skipTest(f"{self.harness_bin} not built")

    def test_compare_rejects_mismatched_protocol_version(self) -> None:
        import subprocess

        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            a_path = tmp_dir / "a.json"
            b_path = tmp_dir / "b.json"
            a_path.write_text(json.dumps(_make_quality_compare_json("Baseline")), encoding="utf-8")
            b_path.write_text(
                json.dumps(_make_quality_compare_json("LegacyV20", protocol_version="2.0")),
                encoding="utf-8",
            )
            proc = subprocess.run(
                [str(self.harness_bin), "--compare", f"{a_path},{b_path}"],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("protocol_version mismatch", proc.stderr)

    def test_compare_rejects_mismatched_schedule_signature(self) -> None:
        import subprocess

        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            a_path = tmp_dir / "a.json"
            b_path = tmp_dir / "b.json"
            a_path.write_text(json.dumps(_make_quality_compare_json("Baseline")), encoding="utf-8")
            mismatched_desc = (
                "protocol=2.1;mode=benchmark_suite;"
                "scenarios=fox_static,fox_drift,fox_obj_translate,fox_skinned_run,helmet_static,"
                "helmet_cam_rotate,helmet_cam_dolly,helmet_obj_spin,star_static,ssr_reflection;"
                "size=512;warmup_frames=16;capture_frames=16;"
                "reference_filter=box;reference_samples=32;jitter_pattern=halton_23_x16;"
                "edge_threshold=0.0800;upscalings=2.0000"
            )
            b_path.write_text(
                json.dumps(_make_quality_compare_json("ShortWarmup", quality_desc=mismatched_desc)),
                encoding="utf-8",
            )
            proc = subprocess.run(
                [str(self.harness_bin), "--compare", f"{a_path},{b_path}"],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertNotEqual(proc.returncode, 0)
            self.assertIn("quality schedule_signature mismatch", proc.stderr)
            self.assertIn("parameter 'warmup_frames': '64' vs '16'", proc.stderr)
            self.assertIn("parameter 'scenarios':", proc.stderr)

    def test_compare_hotspot_rank_reversal_and_per_frame_localization(self) -> None:
        import subprocess

        with tempfile.TemporaryDirectory() as tmp:
            tmp_dir = Path(tmp)
            a_path = tmp_dir / "base.json"
            b_path = tmp_dir / "cand.json"

            base_fox = [0.050] * 16
            cand_fox = list(base_fox)
            cand_fox[4] += 0.020
            cand_fox[5] += 0.022
            cand_fox[6] += 0.030  # peak regression frame t*=6
            cand_fox[7] += 0.024

            base_star = [0.040] * 16
            cand_star = list(base_star)
            cand_star[0] -= 0.008
            cand_star[1] -= 0.009
            cand_star[2] -= 0.012  # peak improvement frame t*=2
            cand_star[3] -= 0.007

            a_path.write_text(
                json.dumps(
                    _make_quality_compare_json(
                        "Baseline",
                        spatial_db=31.00,
                        spatial_hotspot_top10=0.0800,
                        fox_skinned_frames=base_fox,
                        star_static_frames=base_star,
                    )
                ),
                encoding="utf-8",
            )
            b_path.write_text(
                json.dumps(
                    _make_quality_compare_json(
                        "Candidate",
                        spatial_db=32.20,  # +1.20 dB global gain
                        spatial_hotspot_top10=0.0960,  # +20% localized hotspot regression
                        fox_skinned_frames=cand_fox,
                        star_static_frames=cand_star,
                    )
                ),
                encoding="utf-8",
            )

            proc = subprocess.run(
                [str(self.harness_bin), "--compare", f"{a_path},{b_path}"],
                capture_output=True,
                text=True,
                check=False,
            )
            self.assertEqual(proc.returncode, 0, f"stderr: {proc.stderr}")
            self.assertIn("Interior Texture GMSD", proc.stdout)
            self.assertIn("Hotspot Top-10 P95 RMSE", proc.stdout)
            self.assertIn("Wake Ghosting RMS", proc.stdout)
            self.assertIn("WARNING [HOTSPOT_RANK_REVERSAL]", proc.stdout)
            self.assertIn("Per-Frame Edge RMSE Localization: Candidate vs Baseline", proc.stdout)
            self.assertIn("Regression  fox_skinned_run    window [ 4.. 7]", proc.stdout)
            self.assertIn("peak t*= 6 (+0.0300)", proc.stdout)
            self.assertIn("Improvement star_static        window [ 0.. 3]", proc.stdout)
            self.assertIn("peak t*= 2 (-0.0120)", proc.stdout)


if __name__ == "__main__":
    unittest.main()
