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

"""Unit and security tests for tools/taa/taa_viewer.py."""

from __future__ import annotations

import http.client
import json
from pathlib import Path
import re
import struct
import sys
import tempfile
import threading
import unittest
import zlib

sys.path.insert(0, str(Path(__file__).resolve().parent))

import taa_viewer  # noqa: E402


def _make_tiny_png(r: int = 64, g: int = 128, b: int = 200) -> bytes:
    """Builds a minimal valid 2x2 RGBA PNG in memory using stdlib struct + zlib."""
    width, height = 2, 2
    row = b"\x00" + bytes([r, g, b, 255]) * width
    raw = row * height

    def chunk(tag: bytes, data: bytes) -> bytes:
        crc = zlib.crc32(tag + data) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", crc)

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    return (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", ihdr)
        + chunk(b"IDAT", zlib.compress(raw))
        + chunk(b"IEND", b"")
    )


def _write_mock_run(
    root: Path,
    name: str,
    *,
    label: str,
    edge_frames: list[float],
    spatial_db: float = 31.0,
    hotspot_top10: float = 0.080,
    qual_sig: str = "sig_v2_1",
) -> tuple[Path, Path]:
    dump_dir = root / f"dump_{name}"
    sc_dir = dump_dir / "helmet_obj_spin_x2"
    sc_dir.mkdir(parents=True, exist_ok=True)
    png_bytes = _make_tiny_png()
    for k in range(len(edge_frames)):
        (sc_dir / f"capture_{k:02d}.png").write_bytes(png_bytes)
        (sc_dir / f"reference_{k:02d}.png").write_bytes(png_bytes)
        (sc_dir / f"diff_{k:02d}.png").write_bytes(png_bytes)
    (sc_dir / "mask.png").write_bytes(png_bytes)

    records = [
        {"regime": "helmet_obj_spin", "upscaling": 2, "metric": "edge_rmse_mean", "value": sum(edge_frames) / len(edge_frames)},
        {"regime": "helmet_obj_spin", "upscaling": 2, "metric": "gmsd_mean", "value": 0.042},
        {"regime": "perf_helmet_obj_spin", "upscaling": 2, "metric": "upscaler_gpu_ms_p50", "value": 0.78},
    ]
    for k, val in enumerate(edge_frames):
        records.append(
            {"regime": "helmet_obj_spin", "upscaling": 2, "metric": f"edge_rmse_frame_{k}", "value": val}
        )

    payload = {
        "protocol_version": "2.1",
        "schedule_signature": qual_sig,
        "quality_schedule_signature": qual_sig,
        "label": label,
        "dump_directory": f"dump_{name}",
        "benchmark": True,
        "perf": True,
        "size": 512,
        "capture_frames": len(edge_frames),
        "reference_filter": "box",
        "benchmark_summary": [
            {
                "upscaling": 2,
                "spatial_quality_db": spatial_db,
                "spatial_hotspot_top10_rmse": hotspot_top10,
                "temporal_stability_db": 33.5,
                "motion_robustness_db": 28.4,
                "motion_hotspot_top10_rmse": 0.095,
                "perf_upscaler_gpu_ms": 0.78,
                "cts_valid": True,
            }
        ],
        "records": records,
    }
    json_path = root / f"taa_{name}.json"
    json_path.write_text(json.dumps(payload), encoding="utf-8")
    res_payload = dict(payload)
    res_payload["dump_directory"] = "."
    (dump_dir / "results.json").write_text(json.dumps(res_payload), encoding="utf-8")
    return json_path, dump_dir


class TaaViewerTest(unittest.TestCase):
    def test_discovery_and_manifest_diagnostics(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            base_frames = [0.050] * 8
            cand_frames = [0.050, 0.050, 0.072, 0.080, 0.091, 0.075, 0.050, 0.050]
            _write_mock_run(
                root,
                "default",
                label="Filament TAA",
                edge_frames=base_frames,
                spatial_db=31.0,
                hotspot_top10=0.080,
            )
            _write_mock_run(
                root,
                "candidate",
                label="Candidate Upscaler",
                edge_frames=cand_frames,
                spatial_db=32.1,  # +1.1 dB global gain
                hotspot_top10=0.095,  # +18.75% hotspot regression -> rank reversal!
            )

            # Discovering from parent folder deduplicates taa_*.json and dump_*/results.json
            runs = taa_viewer.discover_runs([root])
            self.assertEqual(len(runs), 2)
            self.assertEqual(runs[0].label, "Filament TAA")
            self.assertEqual(runs[1].label, "Candidate Upscaler")

            manifest = taa_viewer.build_manifest(runs)
            self.assertEqual(len(manifest["scenarios"]), 1)
            self.assertEqual(manifest["scenarios"][0]["key"], "helmet_obj_spin_x2")

            # Paired perf_helmet_obj_spin metric folded into helmet_obj_spin_x2
            sc0 = manifest["runs"][0]["scenarios"]["helmet_obj_spin_x2"]
            self.assertAlmostEqual(sc0["scalars"]["upscaler_gpu_ms_p50"], 0.78)
            self.assertEqual(len(sc0["assets"]["frame_indices"]), 8)

            # 4-frame worst-window localization and hotspot rank reversal detected
            loc = manifest["localizations"]["run_1"]["helmet_obj_spin_x2"]["regression"]
            self.assertEqual((loc["t_start"], loc["t_end"]), (2, 5))
            self.assertEqual(loc["peak_frame"], 4)
            self.assertTrue(
                any("HOTSPOT_RANK_REVERSAL" in w for w in manifest["hotspot_warnings"]["run_1"])
            )

    def test_legacy_raw_dump_dir_discovery(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            dump = Path(tmp) / "raw_dump"
            sc = dump / "static_x1"
            sc.mkdir(parents=True)
            png = _make_tiny_png()
            (sc / "capture_00.png").write_bytes(png)
            (sc / "reference_00.png").write_bytes(png)

            runs = taa_viewer.discover_runs([dump])
            self.assertEqual(len(runs), 1)
            self.assertIn("static_x1", runs[0].scenarios)

    def test_non_loopback_host_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            jpath, _ = _write_mock_run(Path(tmp), "a", label="A", edge_frames=[0.04, 0.04])
            runs = taa_viewer.discover_runs([jpath])
            with self.assertRaises(ValueError):
                taa_viewer.create_server(runs, host="0.0.0.0", port=0)

    def test_http_security_headers_methods_and_traversal_defense(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            jpath, dump_dir = _write_mock_run(
                root, "a", label="Engine A", edge_frames=[0.04, 0.05]
            )
            secret_file = root / "outside.png"
            secret_file.write_bytes(_make_tiny_png(255, 0, 0))
            fake_png = dump_dir / "helmet_obj_spin_x2" / "not_a_png.png"
            fake_png.write_text("not png magic bytes", encoding="utf-8")

            runs = taa_viewer.discover_runs([jpath])
            server = taa_viewer.create_server(runs, host="127.0.0.1", port=0)
            host, port = server.server_address[:2]
            thread = threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            try:
                conn = http.client.HTTPConnection(host, port, timeout=5)

                # 1. GET / serves index.html with strict CSP and anti-framing headers
                conn.request("GET", "/")
                resp = conn.getresponse()
                body = resp.read()
                self.assertEqual(resp.status, 200)
                self.assertIn(b"taa_harness", body)
                self.assertIn("script-src 'self'", resp.getheader("Content-Security-Policy", ""))
                self.assertEqual(resp.getheader("X-Content-Type-Options"), "nosniff")
                self.assertEqual(resp.getheader("X-Frame-Options"), "DENY")

                # 2. Disallowed HTTP methods return 405
                for method in ("POST", "PUT", "DELETE", "PATCH"):
                    conn.request(method, "/api/manifest")
                    r_m = conn.getresponse()
                    r_m.read()
                    self.assertEqual(r_m.status, 405)

                # 3. Valid PNG route succeeds
                conn.request("GET", "/api/runs/run_0/image/helmet_obj_spin_x2/capture_00.png")
                r_img = conn.getresponse()
                img_bytes = r_img.read()
                self.assertEqual(r_img.status, 200)
                self.assertTrue(img_bytes.startswith(b"\x89PNG"))

                # 4. Path traversal & non-PNG magic byte attempts are rejected
                bad_paths = [
                    "/api/runs/run_0/image/..%2f/outside.png",
                    "/api/runs/run_0/image/helmet_obj_spin_x2/..%2f..%2foutside.png",
                    "/api/runs/run_0/image/helmet_obj_spin_x2/results.json",
                    "/api/runs/run_0/image/helmet_obj_spin_x2/not_a_png.png",
                    "/../../etc/passwd",
                ]
                for bp in bad_paths:
                    conn.request("GET", bp)
                    r_bad = conn.getresponse()
                    r_bad.read()
                    self.assertIn(r_bad.status, (400, 403, 404), f"Unexpected {r_bad.status} for {bp}")

                conn.close()
            finally:
                server.shutdown()
                server.server_close()

    def test_frontend_avoids_unsafe_dom_sinks(self) -> None:
        js_path = Path(__file__).resolve().parent / "viewer" / "app.js"
        js_code = js_path.read_text(encoding="utf-8")
        # Strip comments before checking for unsafe sinks
        stripped = re.sub(r"//.*", "", js_code)
        stripped = re.sub(r"/\*[\s\S]*?\*/", "", stripped)
        forbidden = [
            "innerHTML",
            "outerHTML",
            "insertAdjacentHTML",
            "document.write",
            "eval(",
            "alert(",
            "confirm(",
            "prompt(",
        ]
        for token in forbidden:
            self.assertNotIn(token, stripped, f"Forbidden DOM/JS sink '{token}' found in app.js")


if __name__ == "__main__":
    unittest.main()
