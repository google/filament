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

import os
import sys
import tempfile
import unittest

import renderers
import test_config


class TestSchemaInvariants(unittest.TestCase):
    """Unit tests validating schema rules, constraints, and mutual exclusivity invariants."""

    def test_valid_gltf_test(self):
        data = {
            "name": "SuiteGltf",
            "renderers": ["desktop-opengl"],
            "presets": [
                {
                    "name": "BasePreset",
                    "tolerance": {"maxAbsDiff": 0.02, "maxFailingPixelsFraction": 0.001},
                    "gltf_test": {
                        "rendering": {"camera.focalLength": 35.0}
                    }
                }
            ],
            "tests": [
                {
                    "name": "Test1",
                    "apply_presets": ["BasePreset"],
                    "gltf_test": {
                        "models": ["Box"],
                        "rendering": {"lighting.iblIntensity": 1.0}
                    }
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        self.assertEqual(len(cfg.tests), 1)
        test = cfg.tests[0]
        self.assertTrue(test.is_gltf_test)
        self.assertFalse(test.is_sample_test)
        self.assertEqual(test.gltf_test.models, ["Box"])
        self.assertEqual(test.gltf_test.rendering["camera.focalLength"], 35.0)
        self.assertEqual(test.gltf_test.rendering["lighting.iblIntensity"], 1.0)
        self.assertIsNotNone(test.tolerance)

    def test_valid_sample_test(self):
        data = {
            "name": "SuiteSample",
            "renderers": ["desktop-vulkan"],
            "presets": [
                {
                    "name": "SamplePreset",
                    "sample_test": {
                        "warmup_frames": 15,
                        "fixed_timestep": 0.0333,
                        "args": ["--window-size", "512x512"]
                    }
                }
            ],
            "tests": [
                {
                    "name": "SampleTest1",
                    "apply_presets": ["SamplePreset"],
                    "sample_test": {
                        "executable": "hellopbr",
                        "args": ["--split-view"]
                    }
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        self.assertEqual(len(cfg.tests), 1)
        test = cfg.tests[0]
        self.assertFalse(test.is_gltf_test)
        self.assertTrue(test.is_sample_test)
        self.assertEqual(test.sample_test.executable, "hellopbr")
        self.assertEqual(test.sample_test.target, "hellopbr")
        self.assertEqual(test.sample_test.warmup_frames, 15)
        self.assertAlmostEqual(test.sample_test.fixed_timestep, 0.0333)
        self.assertEqual(test.sample_test.args, ["--window-size", "512x512", "--split-view"])

    def test_mutually_exclusive_test_blocks(self):
        # A test cannot have both gltf_test and sample_test
        data = {
            "name": "InvalidSuite",
            "renderers": ["desktop-opengl"],
            "tests": [
                {
                    "name": "ConflictingTest",
                    "gltf_test": {"rendering": {}},
                    "sample_test": {"executable": "hellopbr"}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

    def test_missing_test_blocks(self):
        # A test must have at least one test block
        data = {
            "name": "InvalidSuite",
            "renderers": ["desktop-opengl"],
            "tests": [
                {
                    "name": "EmptyTest"
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

    def test_cross_type_preset_conflict(self):
        # gltf_test cannot apply sample_test preset
        data_gltf_with_sample_preset = {
            "name": "ConflictSuite1",
            "renderers": ["desktop-opengl"],
            "presets": [
                {
                    "name": "SampleP",
                    "sample_test": {"args": ["--headless"]}
                }
            ],
            "tests": [
                {
                    "name": "GltfTest",
                    "apply_presets": ["SampleP"],
                    "gltf_test": {"rendering": {}}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data_gltf_with_sample_preset)

        # sample_test cannot apply gltf_test preset
        data_sample_with_gltf_preset = {
            "name": "ConflictSuite2",
            "renderers": ["desktop-opengl"],
            "presets": [
                {
                    "name": "GltfP",
                    "gltf_test": {"rendering": {"view.dithering": "NONE"}}
                }
            ],
            "tests": [
                {
                    "name": "SampleTest",
                    "apply_presets": ["GltfP"],
                    "sample_test": {"executable": "hellotriangle"}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data_sample_with_gltf_preset)

    def test_universal_tolerance_preset(self):
        # Pure tolerance preset can be applied to both gltf_test and sample_test
        data = {
            "name": "ToleranceSuite",
            "renderers": ["desktop-opengl"],
            "presets": [
                {
                    "name": "CommonTol",
                    "tolerance": {"maxAbsDiff": 0.0196, "maxFailingPixelsFraction": 0.001}
                }
            ],
            "tests": [
                {
                    "name": "GltfTest",
                    "apply_presets": ["CommonTol"],
                    "gltf_test": {"rendering": {}}
                },
                {
                    "name": "SampleTest",
                    "apply_presets": ["CommonTol"],
                    "sample_test": {"executable": "suzanne"}
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        self.assertEqual(len(cfg.tests), 2)
        self.assertIsNotNone(cfg.tests[0].tolerance)
        self.assertIsNotNone(cfg.tests[1].tolerance)

    def test_reject_legacy_flat_test(self):
        # Tests using legacy root-level 'rendering' or 'models' must be rejected
        data = {
            "name": "LegacySuite",
            "renderers": ["desktop-opengl"],
            "tests": [
                {
                    "name": "LegacyTest",
                    "models": ["Box"],
                    "rendering": {"camera.focalLength": 35.0}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

    def test_reject_legacy_flat_preset(self):
        # Presets using legacy root-level 'rendering' or 'models' must be rejected
        data = {
            "name": "LegacyPresetSuite",
            "renderers": ["desktop-opengl"],
            "presets": [
                {
                    "name": "LegacyPreset",
                    "models": ["Box"],
                    "rendering": {}
                }
            ],
            "tests": [
                {
                    "name": "Test1",
                    "gltf_test": {"rendering": {}}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

    def test_reject_legacy_root_model_search_paths(self):
        # Suites using legacy root-level 'model_search_paths' must be rejected
        data = {
            "name": "LegacyRootSearchPathsSuite",
            "renderers": ["desktop-opengl"],
            "model_search_paths": ["third_party/models"],
            "tests": [
                {
                    "name": "Test1",
                    "gltf_test": {"rendering": {}}
                }
            ]
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

    def test_render_test_case_polymorphism(self):
        renderer = renderers.DesktopRenderer("desktop", "opengl", "/bin/gltf_viewer")

        gltf_case = renderers.GltfRenderTestCase(
            test_name="PointLights",
            backend="opengl",
            output_dir="/tmp/out",
            model="lucy",
            model_path="/tmp/lucy.glb",
            test_json_path="/tmp/test.json"
        )
        self.assertEqual(gltf_case.target_name, "lucy")
        self.assertEqual(gltf_case.get_out_name(renderer), "PointLights.desktop-opengl.lucy")
        self.assertEqual(gltf_case.get_out_tif_name(renderer), "/tmp/out/PointLights.desktop-opengl.lucy.tif")
        self.assertEqual(gltf_case.get_working_dir(renderer), "/tmp/renderdiff/desktop-opengl/PointLights/lucy")

        sample_case = renderers.SampleRenderTestCase(
            test_name="HelloPBR",
            backend="vulkan",
            output_dir="/tmp/out",
            target="hellopbr_1024",
            executable="hellopbr",
            warmup_frames=15,
            fixed_timestep=0.033,
            extra_args=["--window-size", "1024x1024"]
        )
        self.assertEqual(sample_case.target_name, "hellopbr_1024")
        self.assertEqual(sample_case.get_out_name(renderer), "HelloPBR.desktop-vulkan.hellopbr_1024")
        self.assertEqual(sample_case.get_out_tif_name(renderer), "/tmp/out/HelloPBR.desktop-vulkan.hellopbr_1024.tif")
        self.assertEqual(sample_case.get_working_dir(renderer), "/tmp/renderdiff/desktop-vulkan/HelloPBR/hellopbr_1024")

    def test_desktop_renderer_create_test_cases(self):
        renderer = renderers.DesktopRenderer("desktop", "opengl", "/bin/gltf_viewer")
        data = {
            "name": "PolySuite",
            "renderers": ["desktop-opengl"],
            "tests": [
                {
                    "name": "GltfTest",
                    "gltf_test": {
                        "models": ["Box"],
                        "rendering": {}
                    }
                },
                {
                    "name": "SampleTest",
                    "sample_test": {
                        "executable": "hellotriangle",
                        "args": ["--headless"]
                    }
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        cfg.tests[0].gltf_test.models_map = {"Box": "/models/Box.glb"}
        with tempfile.TemporaryDirectory() as tmpdir:
            gltf_cases = renderer._create_test_cases(cfg.tests[0], tmpdir)
            self.assertEqual(len(gltf_cases), 1)
            self.assertIsInstance(gltf_cases[0], renderers.GltfRenderTestCase)
            self.assertEqual(gltf_cases[0].target_name, "Box")

            sample_cases = renderer._create_test_cases(cfg.tests[1], tmpdir)
            self.assertEqual(len(sample_cases), 1)
            self.assertIsInstance(sample_cases[0], renderers.SampleRenderTestCase)
            self.assertEqual(sample_cases[0].target_name, "hellotriangle")
            self.assertEqual(sample_cases[0].extra_args, ["--headless"])



def validate_config_file(file_path: str) -> bool:
    """Parses and validates a given JSON test configuration file."""
    if not os.path.exists(file_path):
        print(f"[FAIL] Configuration file does not exist: {file_path}", file=sys.stderr)
        return False

    try:
        cfg = test_config.parse_from_path(file_path)
        gltf_count = sum(1 for t in cfg.tests if t.is_gltf_test)
        sample_count = sum(1 for t in cfg.tests if t.is_sample_test)
        print(
            f"[PASS] {file_path}: suite='{cfg.name}', "
            f"renderers={cfg.renderers}, presets={len(cfg.presets)}, "
            f"total_tests={len(cfg.tests)} (gltf={gltf_count}, sample={sample_count})"
        )
        return True
    except Exception as e:
        print(f"[FAIL] Failed to parse {file_path}: {e}", file=sys.stderr)
        return False


def main():
    import argparse

    parser = argparse.ArgumentParser(description="Validate RenderDiff test configurations and schema invariants.")
    parser.add_argument("configs", nargs="*", help="Paths to configuration JSON files to validate.")
    args = parser.parse_args()

    print("=== Running Schema Invariant Unit Tests ===")
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSchemaInvariants)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    if not result.wasSuccessful():
        print("[FAIL] Schema invariant tests failed!", file=sys.stderr)
        sys.exit(1)

    if args.configs:
        print("\n=== Validating Specified Configuration Files ===")
        all_passed = True
        for config_path in args.configs:
            if not validate_config_file(config_path):
                all_passed = False

        if not all_passed:
            print("\n[FAIL] One or more configuration files failed schema validation!", file=sys.stderr)
            sys.exit(1)
        print("\n[SUCCESS] All specified configuration files are valid!")
    else:
        print("\nNotice: No configuration files passed via CLI arguments to validate.")

    sys.exit(0)


if __name__ == "__main__":
    main()
