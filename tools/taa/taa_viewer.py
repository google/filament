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

"""Zero-dependency localhost web viewer for inspecting and comparing taa_harness runs.

Usage examples:
  # Compare multiple benchmark or single-model runs with frame dumps:
  python3 tools/taa/taa_viewer.py /tmp/taa.json /tmp/fsr.json

  # Open dump directories directly (or a parent folder from run_android_benchmark.py --dump):
  python3 tools/taa/taa_viewer.py out/android_taa_benchmark
"""

from __future__ import annotations

import argparse
from collections import deque
from dataclasses import dataclass, field
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import math
import os
from pathlib import Path
import re
import socket
import sys
import threading
import time
from typing import Any, Sequence
import urllib.parse
import webbrowser


VIEWER_STATIC_DIR = (Path(__file__).resolve().parent / "viewer").resolve()
ALLOWED_STATIC_FILES: dict[str, tuple[str, str]] = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/index.html": ("index.html", "text/html; charset=utf-8"),
    "/styles.css": ("styles.css", "text/css; charset=utf-8"),
    "/app.js": ("app.js", "application/javascript; charset=utf-8"),
}

SAFE_SEGMENT_RE = re.compile(r"^[A-Za-z0-9_.-]+$")
SAFE_RUN_ID_RE = re.compile(r"^run_[0-9]+$")
PNG_MAGIC = b"\x89PNG\r\n\x1a\n"

FRAME_SERIES_PREFIXES: dict[str, str] = {
    "rmse_frame_": "rmse",
    "edge_rmse_frame_": "edge_rmse",
    "gmsd_frame_": "gmsd",
    "hotspot_top10_rmse_frame_": "hotspot_top10_rmse",
    "sharpness_frame_": "sharpness",
    "edge_sharpness_frame_": "edge_sharpness",
    "wake_ghosting_frame_": "wake_ghosting",
    "frame_gpu_ms_": "frame_gpu_ms",
}

BENCHMARK_SCENARIO_ORDER: list[str] = [
    "fox_static",
    "helmet_static",
    "star_static",
    "fox_drift",
    "helmet_cam_rotate",
    "helmet_cam_dolly",
    "fox_obj_translate",
    "helmet_obj_spin",
    "buster_drone_anim",
    "fox_skinned_run",
    "ssr_reflection",
    "static",
    "camera",
    "object",
]

SCENARIO_CATEGORIES: dict[str, str] = {
    "fox_static": "Static",
    "helmet_static": "Static",
    "star_static": "Static",
    "fox_drift": "Drift",
    "helmet_cam_rotate": "Tracked Motion",
    "helmet_cam_dolly": "Tracked Motion",
    "fox_obj_translate": "Tracked Motion",
    "helmet_obj_spin": "Tracked Motion",
    "buster_drone_anim": "Tracked Motion",
    "fox_skinned_run": "Unmodeled Motion",
    "ssr_reflection": "Unmodeled Motion",
    "static": "Single Model",
    "camera": "Single Model",
    "object": "Single Model",
}

# Security posture notes for mandatory-secure-web-skills:
# - Localhost-only: Server strictly binds to loopback (127.0.0.1 / ::1) and rejects non-loopback
#   bindings.
# - Read-only stateless design: Only GET and HEAD are allowed; there is no database, no file upload
#   endpoint, and no user account/credential storage.
# - TODO(security): OAuth2/OIDC, MFA, password hashing, cookie session expiration, CSRF tokens,
#   file-upload malware scanning, and SQL/mTLS database controls are intentionally omitted because
#   this developer tool is strictly read-only, stateless, loopback-bound (127.0.0.1), accepts zero
#   state-mutating requests or uploads, and connects to no database.

CSP_POLICY = (
    "default-src 'none'; "
    "script-src 'self'; "
    "style-src 'self'; "
    "img-src 'self' blob: data:; "
    "connect-src 'self'; "
    "font-src 'self'; "
    "frame-ancestors 'none'; "
    "base-uri 'none'; "
    "form-action 'none'"
)


def format_upscaling_tag(upscaling: float) -> str:
    """Formats upscaling factor identically to C++ `snprintf(..., \"%g\", upscaling)`."""
    return f"{upscaling:g}"


def make_scenario_key(regime: str, upscaling: float) -> str:
    return f"{regime}_x{format_upscaling_tag(upscaling)}"


def classify_scenario(regime: str) -> str:
    if regime in SCENARIO_CATEGORIES:
        return SCENARIO_CATEGORIES[regime]
    if regime.startswith("perf_"):
        return "Perf"
    return "Custom"


@dataclass
class ScenarioAssets:
    scenario_dir: str
    captures: dict[int, str] = field(default_factory=dict)
    references: dict[int, str] = field(default_factory=dict)
    diffs: dict[int, str] = field(default_factory=dict)
    mask: str | None = None
    msaa_reference: str | None = None
    msaa_vs_reference_diff: str | None = None
    aliased_reference: str | None = None
    aliased_diff: str | None = None
    perf_capture: str | None = None
    perf_aliased_reference: str | None = None
    perf_aliased_diff: str | None = None


@dataclass
class ScenarioData:
    key: str
    regime: str
    upscaling: float
    category: str
    scalars: dict[str, float] = field(default_factory=dict)
    series: dict[str, list[float | None]] = field(default_factory=dict)
    assets: ScenarioAssets | None = None


@dataclass
class LoadedRun:
    run_id: str
    label: str
    source_path: Path
    dump_dir: Path | None
    header: dict[str, Any]
    benchmark_summaries: list[dict[str, Any]]
    scenarios: dict[str, ScenarioData]


def _scan_scenario_dir(run_id: str, subdir: Path) -> ScenarioAssets | None:
    if not subdir.is_dir():
        return None
    dir_name = subdir.name
    if not SAFE_SEGMENT_RE.match(dir_name):
        return None

    assets = ScenarioAssets(scenario_dir=dir_name)
    has_any = False

    def url_for(fname: str) -> str:
        return f"/api/runs/{run_id}/image/{dir_name}/{fname}"

    for entry in sorted(subdir.iterdir(), key=lambda p: p.name):
        if not entry.is_file() or entry.suffix.lower() != ".png":
            continue
        name = entry.name
        if not SAFE_SEGMENT_RE.match(name):
            continue
        m_cap = re.match(r"^capture_(\d+)\.png$", name)
        if m_cap:
            assets.captures[int(m_cap.group(1))] = url_for(name)
            has_any = True
            continue
        m_ref = re.match(r"^reference_(\d+)\.png$", name)
        if m_ref:
            assets.references[int(m_ref.group(1))] = url_for(name)
            has_any = True
            continue
        m_diff = re.match(r"^diff_(\d+)\.png$", name)
        if m_diff:
            assets.diffs[int(m_diff.group(1))] = url_for(name)
            has_any = True
            continue
        if name == "mask.png":
            assets.mask = url_for(name)
            has_any = True
        elif name == "msaa_reference.png":
            assets.msaa_reference = url_for(name)
            has_any = True
        elif name == "msaa_vs_reference_diff.png":
            assets.msaa_vs_reference_diff = url_for(name)
            has_any = True
        elif name == "aliased_reference.png":
            assets.aliased_reference = url_for(name)
            has_any = True
        elif name == "aliased_diff.png":
            assets.aliased_diff = url_for(name)
            has_any = True
        elif name == "perf_capture.png":
            assets.perf_capture = url_for(name)
            has_any = True
        elif name == "perf_aliased_reference.png":
            assets.perf_aliased_reference = url_for(name)
            has_any = True
        elif name == "perf_aliased_diff.png":
            assets.perf_aliased_diff = url_for(name)
            has_any = True

    return assets if has_any else None


def _looks_like_dump_dir(directory: Path) -> bool:
    if not directory.is_dir():
        return False
    if (directory / "results.json").is_file():
        return True
    for child in directory.iterdir():
        if child.is_dir() and "_x" in child.name:
            if (child / "capture_00.png").is_file() or (child / "perf_capture.png").is_file():
                return True
    return False


def _resolve_dump_dir_for_json(json_path: Path, raw_json: dict[str, Any]) -> Path | None:
    raw_dump = raw_json.get("dump_directory")
    candidates: list[Path] = []
    if isinstance(raw_dump, str) and raw_dump.strip():
        d_path = Path(raw_dump.strip())
        if d_path.is_absolute():
            candidates.append(d_path)
        else:
            candidates.append(json_path.parent / d_path)
            candidates.append(Path.cwd() / d_path)

    # Heuristic sibling fallbacks for JSONs generated before dump_directory metadata existed
    stem = json_path.stem
    if json_path.name == "results.json":
        candidates.append(json_path.parent)
    candidates.append(json_path.parent / stem)
    candidates.append(json_path.parent / f"{stem}_dump")
    if stem.startswith("taa_"):
        tag = stem[len("taa_") :]
        candidates.append(json_path.parent / f"dump_{tag}")

    for cand in candidates:
        try:
            resolved = cand.resolve()
        except OSError:
            continue
        if _looks_like_dump_dir(resolved):
            return resolved
    return None


def _set_series_value(series_map: dict[str, list[float | None]], key: str, idx: int, val: float) -> None:
    if idx < 0 or idx >= 4096:
        return
    arr = series_map.setdefault(key, [])
    if len(arr) <= idx:
        arr.extend([None] * (idx + 1 - len(arr)))
    arr[idx] = val


def _load_run_from_json(run_id: str, json_path: Path, explicit_dump_dir: Path | None = None) -> LoadedRun:
    raw = json.loads(json_path.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise ValueError(f"Expected JSON object in {json_path}")

    dump_dir = explicit_dump_dir.resolve() if explicit_dump_dir else _resolve_dump_dir_for_json(json_path, raw)

    label = str(raw.get("label") or "").strip()
    if not label:
        if json_path.name == "results.json" and dump_dir is not None:
            label = dump_dir.name
        else:
            label = json_path.stem

    header_keys = [
        "protocol_version",
        "schedule_signature",
        "schedule_descriptor",
        "quality_schedule_signature",
        "quality_schedule_descriptor",
        "perf_schedule_signature",
        "perf_schedule_descriptor",
        "benchmark",
        "perf",
        "perf_width",
        "perf_height",
        "perf_warmup_frames",
        "perf_frames",
        "fixed_performance_mode",
        "thermal_status_start",
        "thermal_status_end",
        "model",
        "backend",
        "size",
        "warmup_frames",
        "capture_frames",
        "reference_filter",
        "reference_samples",
        "msaa",
    ]
    header = {k: raw[k] for k in header_keys if k in raw}

    summaries_raw = raw.get("benchmark_summary")
    benchmark_summaries: list[dict[str, Any]] = (
        [s for s in summaries_raw if isinstance(s, dict)] if isinstance(summaries_raw, list) else []
    )

    scenarios: dict[str, ScenarioData] = {}
    perf_by_base_key: dict[str, ScenarioData] = {}

    records = raw.get("records", [])
    if isinstance(records, list):
        for rec in records:
            if not isinstance(rec, dict):
                continue
            regime = str(rec.get("regime") or "")
            if not regime or regime.endswith("_baseline"):
                continue
            try:
                upscaling = float(rec.get("upscaling", 1.0))
                value = float(rec.get("value", 0.0))
            except (TypeError, ValueError):
                continue
            if not math.isfinite(value):
                continue
            metric = str(rec.get("metric") or "")
            if not metric:
                continue

            s_key = make_scenario_key(regime, upscaling)
            sc = scenarios.get(s_key)
            if sc is None:
                sc = ScenarioData(
                    key=s_key,
                    regime=regime,
                    upscaling=upscaling,
                    category=classify_scenario(regime),
                )
                scenarios[s_key] = sc
                if regime.startswith("perf_"):
                    base_regime = regime[len("perf_") :]
                    perf_by_base_key[make_scenario_key(base_regime, upscaling)] = sc

            matched_series = False
            for prefix, series_name in FRAME_SERIES_PREFIXES.items():
                if metric.startswith(prefix):
                    tail = metric[len(prefix) :]
                    if tail.isdigit():
                        _set_series_value(sc.series, series_name, int(tail), value)
                        matched_series = True
                    break
            if not matched_series:
                sc.scalars[metric] = value

    # Fold paired perf_<scenario> GPU metrics into quality <scenario> when both exist so users see
    # quality + GPU timing together on one scenario entry instead of duplicate sidebar items.
    for base_key, perf_sc in list(perf_by_base_key.items()):
        if base_key in scenarios:
            target_sc = scenarios[base_key]
            for m_name, m_val in perf_sc.scalars.items():
                target_sc.scalars.setdefault(m_name, m_val)
            for s_name, s_vals in perf_sc.series.items():
                target_sc.series.setdefault(s_name, s_vals)
            if dump_dir is not None:
                perf_assets = _scan_scenario_dir(run_id, dump_dir / perf_sc.key)
                if perf_assets and perf_assets.perf_capture:
                    # Keep perf_sc only if quality scenario has no captures at all; otherwise fold.
                    pass
            del scenarios[perf_sc.key]

    if dump_dir is not None and dump_dir.is_dir():
        for child in sorted(dump_dir.iterdir(), key=lambda p: p.name):
            if not child.is_dir():
                continue
            assets = _scan_scenario_dir(run_id, child)
            if assets is None:
                continue
            if child.name in scenarios:
                scenarios[child.name].assets = assets
            elif child.name.startswith("perf_"):
                base_name = child.name[len("perf_") :]
                if base_name in scenarios and scenarios[base_name].assets is not None:
                    base_assets = scenarios[base_name].assets
                    assert base_assets is not None
                    base_assets.perf_capture = assets.perf_capture
                    base_assets.perf_aliased_reference = assets.perf_aliased_reference
                    base_assets.perf_aliased_diff = assets.perf_aliased_diff
                else:
                    m = re.match(r"^(.+)_x([0-9.]+)$", child.name)
                    reg = m.group(1) if m else child.name
                    ups = float(m.group(2)) if m else 1.0
                    scenarios[child.name] = ScenarioData(
                        key=child.name,
                        regime=reg,
                        upscaling=ups,
                        category=classify_scenario(reg),
                        assets=assets,
                    )
            else:
                m = re.match(r"^(.+)_x([0-9.]+)$", child.name)
                reg = m.group(1) if m else child.name
                ups = float(m.group(2)) if m else 1.0
                scenarios[child.name] = ScenarioData(
                    key=child.name,
                    regime=reg,
                    upscaling=ups,
                    category=classify_scenario(reg),
                    assets=assets,
                )

    return LoadedRun(
        run_id=run_id,
        label=label,
        source_path=json_path.resolve(),
        dump_dir=dump_dir,
        header=header,
        benchmark_summaries=benchmark_summaries,
        scenarios=scenarios,
    )


def _load_run_from_raw_dump_dir(run_id: str, dump_dir: Path) -> LoadedRun:
    resolved = dump_dir.resolve()
    scenarios: dict[str, ScenarioData] = {}
    max_frames = 0
    for child in sorted(resolved.iterdir(), key=lambda p: p.name):
        if not child.is_dir():
            continue
        assets = _scan_scenario_dir(run_id, child)
        if assets is None:
            continue
        m = re.match(r"^(.+)_x([0-9.]+)$", child.name)
        regime = m.group(1) if m else child.name
        upscaling = float(m.group(2)) if m else 1.0
        if assets.captures:
            max_frames = max(max_frames, max(assets.captures.keys()) + 1)
        scenarios[child.name] = ScenarioData(
            key=child.name,
            regime=regime,
            upscaling=upscaling,
            category=classify_scenario(regime),
            assets=assets,
        )

    return LoadedRun(
        run_id=run_id,
        label=resolved.name,
        source_path=resolved,
        dump_dir=resolved,
        header={"capture_frames": max_frames} if max_frames > 0 else {},
        benchmark_summaries=[],
        scenarios=scenarios,
    )


def discover_runs(input_paths: Sequence[Path]) -> list[LoadedRun]:
    """Discovers one or more taa_harness runs from JSON files, dump folders, or parent folders."""
    runs: list[LoadedRun] = []
    seen_sources: set[Path] = set()
    seen_dumps: set[Path] = set()

    def next_id() -> str:
        return f"run_{len(runs)}"

    def add_json(jpath: Path, explicit_dump: Path | None = None) -> None:
        resolved = jpath.resolve()
        if resolved in seen_sources:
            return
        run = _load_run_from_json(next_id(), resolved, explicit_dump)
        if not run.scenarios and not run.benchmark_summaries:
            return
        seen_sources.add(resolved)
        if run.dump_dir is not None:
            seen_dumps.add(run.dump_dir)
            res_json = (run.dump_dir / "results.json").resolve()
            seen_sources.add(res_json)
        runs.append(run)

    def add_raw_dump(dpath: Path) -> None:
        resolved = dpath.resolve()
        if resolved in seen_dumps:
            return
        res_json = resolved / "results.json"
        if res_json.is_file():
            add_json(res_json, explicit_dump=resolved)
            return
        run = _load_run_from_raw_dump_dir(next_id(), resolved)
        if run.scenarios:
            seen_dumps.add(resolved)
            runs.append(run)

    for raw_p in input_paths:
        p = raw_p.expanduser().resolve()
        if not p.exists():
            raise FileNotFoundError(f"Input path does not exist: {raw_p}")
        if p.is_file():
            if p.suffix.lower() != ".json":
                raise ValueError(f"Expected a .json file or directory, got: {raw_p}")
            add_json(p)
        elif p.is_dir():
            if _looks_like_dump_dir(p):
                add_raw_dump(p)
            else:
                # Parent container directory (e.g., out/android_taa_benchmark): load top-level JSONs
                # first (putting default/baseline first), then any unclaimed dump subdirectories.
                json_files = sorted(
                    [c for c in p.iterdir() if c.is_file() and c.suffix.lower() == ".json"],
                    key=lambda c: (0 if "default" in c.stem or "base" in c.stem else 1, c.name),
                )
                for jf in json_files:
                    try:
                        add_json(jf)
                    except (ValueError, json.JSONDecodeError):
                        continue
                for sub in sorted([c for c in p.iterdir() if c.is_dir()], key=lambda c: c.name):
                    if _looks_like_dump_dir(sub):
                        add_raw_dump(sub)

    # Disambiguate duplicate labels if user passed two runs with identical labels
    label_counts: dict[str, int] = {}
    for r in runs:
        label_counts[r.label] = label_counts.get(r.label, 0) + 1
    seen_labels: dict[str, int] = {}
    for r in runs:
        if label_counts[r.label] > 1:
            idx = seen_labels.get(r.label, 0) + 1
            seen_labels[r.label] = idx
            r.label = f"{r.label} ({r.source_path.stem if idx > 1 else '1'})" if idx == 1 else f"{r.label} ({idx})"

    return runs


def compute_window_localization(
    base_series: Sequence[float | None],
    cand_series: Sequence[float | None],
    window: int = 4,
) -> dict[str, Any] | None:
    """Finds the worst regression and best improvement 4-frame windows (matching --compare)."""
    n = min(len(base_series), len(cand_series))
    if n == 0:
        return None
    deltas: list[float] = []
    for k in range(n):
        b = base_series[k]
        c = cand_series[k]
        if b is None or c is None:
            return None
        deltas.append(c - b)

    w = min(window, n)
    worst_start = 0
    worst_mean = -math.inf
    best_start = 0
    best_mean = math.inf
    for start in range(0, n - w + 1):
        mean_d = sum(deltas[start : start + w]) / float(w)
        if mean_d > worst_mean:
            worst_mean = mean_d
            worst_start = start
        if mean_d < best_mean:
            best_mean = mean_d
            best_start = start

    worst_peak_t = max(range(worst_start, worst_start + w), key=lambda t: deltas[t])
    best_peak_t = min(range(best_start, best_start + w), key=lambda t: deltas[t])

    result: dict[str, Any] = {}
    if worst_mean > 1e-6:
        result["regression"] = {
            "t_start": worst_start,
            "t_end": worst_start + w - 1,
            "window_mean_delta": worst_mean,
            "peak_frame": worst_peak_t,
            "peak_delta": deltas[worst_peak_t],
        }
    if best_mean < -1e-6:
        result["improvement"] = {
            "t_start": best_start,
            "t_end": best_start + w - 1,
            "window_mean_delta": best_mean,
            "peak_frame": best_peak_t,
            "peak_delta": deltas[best_peak_t],
        }
    return result or None


def detect_hotspot_rank_reversals(
    base_summary: dict[str, Any], cand_summary: dict[str, Any]
) -> list[str]:
    warnings: list[str] = []
    checks = [
        ("Spatial Quality", "spatial_quality_db", "spatial_hotspot_top10_rmse"),
        ("Motion Robustness", "motion_robustness_db", "motion_hotspot_top10_rmse"),
    ]
    for axis_name, db_key, hs_key in checks:
        b_db = base_summary.get(db_key)
        c_db = cand_summary.get(db_key)
        b_hs = base_summary.get(hs_key)
        c_hs = cand_summary.get(hs_key)
        if all(isinstance(v, (int, float)) for v in (b_db, c_db, b_hs, c_hs)) and b_hs > 0:
            db_gain = float(c_db) - float(b_db)
            hs_delta = float(c_hs) - float(b_hs)
            hs_rel = hs_delta / float(b_hs)
            if db_gain >= 0.05 and hs_rel > 0.02 and hs_delta > 0.0005:
                warnings.append(
                    f"HOTSPOT_RANK_REVERSAL ({axis_name}): global +{db_gain:.2f} dB, "
                    f"but Hotspot Top-10 P95 RMSE worsened +{hs_rel * 100.0:.1f}%"
                )
    return warnings


def build_manifest(runs: Sequence[LoadedRun]) -> dict[str, Any]:
    all_scenario_keys: dict[str, tuple[str, float, str]] = {}
    for r in runs:
        for s_key, sc in r.scenarios.items():
            all_scenario_keys.setdefault(s_key, (sc.regime, sc.upscaling, sc.category))

    def scenario_sort_key(item: tuple[str, tuple[str, float, str]]) -> tuple[int, float, str]:
        _, (regime, upscaling, _) = item
        try:
            order_idx = BENCHMARK_SCENARIO_ORDER.index(regime)
        except ValueError:
            order_idx = len(BENCHMARK_SCENARIO_ORDER)
        return (order_idx, upscaling, regime)

    ordered_scenarios = [
        {"key": k, "regime": reg, "upscaling": ups, "category": cat}
        for k, (reg, ups, cat) in sorted(all_scenario_keys.items(), key=scenario_sort_key)
    ]

    # Schedule signature compatibility check across loaded runs
    schedule_warnings: list[str] = []
    if len(runs) >= 2:
        base_q = runs[0].header.get("quality_schedule_signature")
        base_p = runs[0].header.get("perf_schedule_signature")
        for cand in runs[1:]:
            cand_q = cand.header.get("quality_schedule_signature")
            if base_q and cand_q and base_q != cand_q:
                schedule_warnings.append(
                    f"Quality schedule mismatch between '{runs[0].label}' and '{cand.label}'"
                )
            cand_p = cand.header.get("perf_schedule_signature")
            if base_p and cand_p and base_p != cand_p:
                schedule_warnings.append(
                    f"Perf schedule mismatch between '{runs[0].label}' and '{cand.label}'"
                )

    # Pairwise localization and hotspot rank reversal diagnostics vs Run 0 (baseline)
    localizations: dict[str, dict[str, Any]] = {}
    hotspot_warnings: dict[str, list[str]] = {}
    if len(runs) >= 2:
        base = runs[0]
        base_sum_by_u = {
            format_upscaling_tag(float(s.get("upscaling", 2.0))): s
            for s in base.benchmark_summaries
        }
        for cand in runs[1:]:
            per_scenario_loc: dict[str, Any] = {}
            for s_key, base_sc in base.scenarios.items():
                cand_sc = cand.scenarios.get(s_key)
                if cand_sc is None:
                    continue
                b_edge = base_sc.series.get("edge_rmse")
                c_edge = cand_sc.series.get("edge_rmse")
                if b_edge and c_edge:
                    loc = compute_window_localization(b_edge, c_edge)
                    if loc:
                        per_scenario_loc[s_key] = loc
            if per_scenario_loc:
                localizations[cand.run_id] = per_scenario_loc

            cand_warns: list[str] = []
            for cs in cand.benchmark_summaries:
                u_tag = format_upscaling_tag(float(cs.get("upscaling", 2.0)))
                bs = base_sum_by_u.get(u_tag)
                if bs:
                    cand_warns.extend(detect_hotspot_rank_reversals(bs, cs))
            if cand_warns:
                hotspot_warnings[cand.run_id] = cand_warns

    runs_payload: list[dict[str, Any]] = []
    for r in runs:
        sc_payload: dict[str, Any] = {}
        for s_key, sc in r.scenarios.items():
            assets_obj: dict[str, Any] | None = None
            if sc.assets is not None:
                frame_indices = sorted(
                    set(sc.assets.captures.keys())
                    | set(sc.assets.references.keys())
                    | set(sc.assets.diffs.keys())
                )
                assets_obj = {
                    "frame_indices": frame_indices,
                    "captures": {str(k): v for k, v in sorted(sc.assets.captures.items())},
                    "references": {str(k): v for k, v in sorted(sc.assets.references.items())},
                    "diffs": {str(k): v for k, v in sorted(sc.assets.diffs.items())},
                    "mask": sc.assets.mask,
                    "msaa_reference": sc.assets.msaa_reference,
                    "msaa_vs_reference_diff": sc.assets.msaa_vs_reference_diff,
                    "aliased_reference": sc.assets.aliased_reference,
                    "aliased_diff": sc.assets.aliased_diff,
                    "perf_capture": sc.assets.perf_capture,
                    "perf_aliased_reference": sc.assets.perf_aliased_reference,
                    "perf_aliased_diff": sc.assets.perf_aliased_diff,
                }
            sc_payload[s_key] = {
                "regime": sc.regime,
                "upscaling": sc.upscaling,
                "category": sc.category,
                "scalars": sc.scalars,
                "series": sc.series,
                "assets": assets_obj,
            }

        runs_payload.append(
            {
                "id": r.run_id,
                "label": r.label,
                "has_dumps": r.dump_dir is not None,
                "header": r.header,
                "benchmark_summaries": r.benchmark_summaries,
                "scenarios": sc_payload,
            }
        )

    return {
        "scenarios": ordered_scenarios,
        "runs": runs_payload,
        "schedule_warnings": schedule_warnings,
        "localizations": localizations,
        "hotspot_warnings": hotspot_warnings,
    }


class SlidingWindowRateLimiter:
    """Thread-safe per-client sliding-window rate limiter for localhost API protection."""

    def __init__(self, max_requests: int = 3000, window_sec: float = 10.0) -> None:
        self.max_requests = max_requests
        self.window_sec = window_sec
        self._lock = threading.Lock()
        self._hits: dict[str, deque[float]] = {}

    def allow(self, client_key: str) -> bool:
        now = time.monotonic()
        cutoff = now - self.window_sec
        with self._lock:
            q = self._hits.setdefault(client_key, deque())
            while q and q[0] < cutoff:
                q.popleft()
            if len(q) >= self.max_requests:
                return False
            q.append(now)
            return True


class ViewerState:
    def __init__(self, runs: Sequence[LoadedRun], static_dir: Path = VIEWER_STATIC_DIR) -> None:
        self.runs = list(runs)
        self.runs_by_id: dict[str, LoadedRun] = {r.run_id: r for r in runs}
        self.static_dir = static_dir.resolve()
        self.manifest_bytes = json.dumps(build_manifest(self.runs), separators=(",", ":")).encode(
            "utf-8"
        )
        self.rate_limiter = SlidingWindowRateLimiter()


def create_request_handler(state: ViewerState) -> type[BaseHTTPRequestHandler]:
    class TaaViewerRequestHandler(BaseHTTPRequestHandler):
        server_version = "FilamentTaaViewer/1.0"
        sys_version = ""

        def log_message(self, fmt: str, *args: Any) -> None:
            # Quiet clean logging without leaking query parameters or headers
            pass

        def _send_security_headers(self, cache_control: str = "no-store") -> None:
            self.send_header("Content-Security-Policy", CSP_POLICY)
            self.send_header("X-Content-Type-Options", "nosniff")
            self.send_header("X-Frame-Options", "DENY")
            self.send_header("Referrer-Policy", "no-referrer")
            self.send_header("Permissions-Policy", "camera=(), microphone=(), geolocation=()")
            self.send_header("Cross-Origin-Resource-Policy", "same-origin")
            self.send_header("Cache-Control", cache_control)

        def _send_error_json(self, status: HTTPStatus, message: str) -> None:
            payload = json.dumps({"error": message}).encode("utf-8")
            self.send_response(int(status))
            if status == HTTPStatus.METHOD_NOT_ALLOWED:
                self.send_header("Allow", "GET, HEAD")
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.send_header("Content-Length", str(len(payload)))
            self._send_security_headers("no-store")
            self.end_headers()
            if self.command != "HEAD":
                self.wfile.write(payload)

        def _check_rate_limit(self) -> bool:
            client_ip = self.client_address[0] if self.client_address else "local"
            if not state.rate_limiter.allow(client_ip):
                self._send_error_json(HTTPStatus.TOO_MANY_REQUESTS, "Rate limit exceeded")
                return False
            return True

        def do_GET(self) -> None:
            if not self._check_rate_limit():
                return
            self._dispatch(send_body=True)

        def do_HEAD(self) -> None:
            if not self._check_rate_limit():
                return
            self._dispatch(send_body=False)

        def do_POST(self) -> None:
            self._send_error_json(HTTPStatus.METHOD_NOT_ALLOWED, "Method not allowed")

        def do_PUT(self) -> None:
            self._send_error_json(HTTPStatus.METHOD_NOT_ALLOWED, "Method not allowed")

        def do_DELETE(self) -> None:
            self._send_error_json(HTTPStatus.METHOD_NOT_ALLOWED, "Method not allowed")

        def do_PATCH(self) -> None:
            self._send_error_json(HTTPStatus.METHOD_NOT_ALLOWED, "Method not allowed")

        def do_OPTIONS(self) -> None:
            self._send_error_json(HTTPStatus.METHOD_NOT_ALLOWED, "Method not allowed")

        def _dispatch(self, send_body: bool) -> None:
            parsed = urllib.parse.urlsplit(self.path)
            raw_path = parsed.path

            if raw_path in ALLOWED_STATIC_FILES:
                rel_name, content_type = ALLOWED_STATIC_FILES[raw_path]
                safe_name = os.path.basename(rel_name)
                target = (state.static_dir / safe_name).resolve()
                if not str(target).startswith(str(state.static_dir) + os.sep) or not target.is_file():
                    self._send_error_json(HTTPStatus.NOT_FOUND, "Static asset not found")
                    return
                data = target.read_bytes()
                self.send_response(int(HTTPStatus.OK))
                self.send_header("Content-Type", content_type)
                self.send_header("Content-Length", str(len(data)))
                self._send_security_headers("no-store")
                self.end_headers()
                if send_body:
                    self.wfile.write(data)
                return

            if raw_path == "/api/manifest":
                data = state.manifest_bytes
                self.send_response(int(HTTPStatus.OK))
                self.send_header("Content-Type", "application/json; charset=utf-8")
                self.send_header("Content-Length", str(len(data)))
                self._send_security_headers("no-store")
                self.end_headers()
                if send_body:
                    self.wfile.write(data)
                return

            # Route: /api/runs/<run_id>/image/<scenario_dir>/<filename>
            parts = raw_path.split("/")
            if len(parts) == 7 and parts[0] == "" and parts[1] == "api" and parts[2] == "runs" and parts[4] == "image":
                run_id = urllib.parse.unquote(parts[3])
                raw_scenario = urllib.parse.unquote(parts[5])
                raw_filename = urllib.parse.unquote(parts[6])
                self._serve_run_image(run_id, raw_scenario, raw_filename, send_body=send_body)
                return

            self._send_error_json(HTTPStatus.NOT_FOUND, "Not found")

        def _serve_run_image(
            self, run_id: str, raw_scenario: str, raw_filename: str, *, send_body: bool
        ) -> None:
            if not SAFE_RUN_ID_RE.match(run_id):
                self._send_error_json(HTTPStatus.BAD_REQUEST, "Invalid run identifier")
                return

            run = state.runs_by_id.get(run_id)
            if run is None or run.dump_dir is None:
                self._send_error_json(HTTPStatus.NOT_FOUND, "Run or dump directory not found")
                return

            # Reject any traversal tokens or directory separators before and after basename()
            if (
                "/" in raw_scenario
                or "\\" in raw_scenario
                or ".." in raw_scenario
                or "/" in raw_filename
                or "\\" in raw_filename
                or ".." in raw_filename
            ):
                self._send_error_json(HTTPStatus.FORBIDDEN, "Invalid path segment")
                return

            scenario_seg = os.path.basename(raw_scenario)
            file_seg = os.path.basename(raw_filename)
            if (
                scenario_seg != raw_scenario
                or file_seg != raw_filename
                or not SAFE_SEGMENT_RE.match(scenario_seg)
                or not SAFE_SEGMENT_RE.match(file_seg)
                or not file_seg.lower().endswith(".png")
            ):
                self._send_error_json(HTTPStatus.FORBIDDEN, "Disallowed image filename")
                return

            dump_root = run.dump_dir.resolve()
            candidate = (dump_root / scenario_seg / file_seg).resolve()
            if not str(candidate).startswith(str(dump_root) + os.sep) or not candidate.is_file():
                self._send_error_json(HTTPStatus.NOT_FOUND, "Image not found")
                return

            try:
                data = candidate.read_bytes()
            except OSError:
                self._send_error_json(HTTPStatus.INTERNAL_SERVER_ERROR, "Unable to read image")
                return

            if len(data) < len(PNG_MAGIC) or data[: len(PNG_MAGIC)] != PNG_MAGIC:
                self._send_error_json(HTTPStatus.FORBIDDEN, "Invalid PNG file header")
                return

            self.send_response(int(HTTPStatus.OK))
            self.send_header("Content-Type", "image/png")
            self.send_header("Content-Length", str(len(data)))
            self._send_security_headers("private, max-age=300")
            self.end_headers()
            if send_body:
                self.wfile.write(data)

    return TaaViewerRequestHandler


def _validate_loopback_host(host: str) -> str:
    if host not in ("127.0.0.1", "localhost", "::1"):
        raise ValueError(
            f"Refusing to bind taa_viewer to non-loopback address '{host}'. "
            "Use 127.0.0.1 or localhost."
        )
    return "127.0.0.1" if host == "localhost" else host


def create_server(
    runs: Sequence[LoadedRun],
    host: str = "127.0.0.1",
    port: int = 8910,
    static_dir: Path = VIEWER_STATIC_DIR,
) -> ThreadingHTTPServer:
    bind_host = _validate_loopback_host(host)
    state = ViewerState(runs, static_dir=static_dir)
    handler_cls = create_request_handler(state)
    try:
        return ThreadingHTTPServer((bind_host, port), handler_cls)
    except OSError:
        if port != 0:
            # Fall back to an ephemeral OS-assigned loopback port if default port is occupied
            return ThreadingHTTPServer((bind_host, 0), handler_cls)
        raise


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Interactive browser viewer for inspecting and comparing taa_harness runs."
    )
    parser.add_argument(
        "inputs",
        nargs="+",
        help="Paths to taa_harness .json files, -d dump directories, or a parent directory of runs",
    )
    parser.add_argument(
        "--port",
        type=int,
        default=8910,
        help="Localhost TCP port (default: 8910; falls back to a free port if busy)",
    )
    parser.add_argument(
        "--no-browser",
        action="store_true",
        help="Do not automatically open the URL in the default web browser",
    )
    args = parser.parse_args(argv)

    try:
        runs = discover_runs([Path(p) for p in args.inputs])
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as exc:
        print(f"taa_viewer: error: {exc}", file=sys.stderr)
        return 1

    if not runs:
        print(
            "taa_viewer: no valid taa_harness JSON files or frame dump directories found in inputs.",
            file=sys.stderr,
        )
        return 1

    server = create_server(runs, host="127.0.0.1", port=args.port)
    bound_host, bound_port = server.server_address[:2]
    url = f"http://{bound_host}:{bound_port}/"

    print("========================================================================")
    print("  Filament taa_harness Interactive Result Viewer")
    print(f"  URL  : {url}")
    print(f"  Runs : {len(runs)}")
    for idx, r in enumerate(runs, start=1):
        dump_status = f"dumps: {r.dump_dir}" if r.dump_dir else "metrics-only (no -d dump dir)"
        print(f"    [{idx}] {r.label}  ({len(r.scenarios)} scenarios, {dump_status})")
    print("  Press Ctrl+C to stop.")
    print("========================================================================")

    if not args.no_browser:
        threading.Timer(0.15, lambda: webbrowser.open(url)).start()

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\ntaa_viewer: shutting down.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
