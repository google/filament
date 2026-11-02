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

import json
import os
import struct
import sys
import tempfile
import types
import unittest

import compare
import renderers
import results as results_mod
import test_config
import update_golden
import web_renderer


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

    def test_preset_renderers(self):
        # Renderers resolve as: test > last preset that sets them > root default.
        data = {
            "name": "RendererSuite",
            "renderers": ["desktop-opengl", "web-webgl"],
            "presets": [
                {"name": "Desktop", "renderers": ["desktop-opengl"]},
                {"name": "Vulkan", "renderers": ["desktop-vulkan"]},
                {"name": "NoRenderers", "sample_test": {"warmup_frames": 1}}
            ],
            "tests": [
                {
                    "name": "Default",
                    "sample_test": {"executable": "suzanne"}
                },
                {
                    "name": "FromPreset",
                    "apply_presets": ["Desktop", "NoRenderers"],
                    "sample_test": {"executable": "suzanne"}
                },
                {
                    "name": "LastPresetWins",
                    "apply_presets": ["Desktop", "Vulkan"],
                    "sample_test": {"executable": "suzanne"}
                },
                {
                    "name": "TestWins",
                    "apply_presets": ["Desktop"],
                    "renderers": ["desktop-webgpu"],
                    "sample_test": {"executable": "suzanne"}
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        got = {t.name: t.renderers for t in cfg.tests}
        self.assertEqual(got, {
            "Default": ["desktop-opengl", "web-webgl"],
            "FromPreset": ["desktop-opengl"],
            "LastPresetWins": ["desktop-vulkan"],
            "TestWins": ["desktop-webgpu"],
        })

    def test_reject_invalid_preset_renderers(self):
        data = {
            "name": "BadRenderers",
            "renderers": ["desktop-opengl"],
            "presets": [{"name": "Bad", "renderers": "desktop-opengl"}],
            "tests": []
        }
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(data)

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

    def test_desktop_gltf_case_carries_nested_settings(self):
        renderer = renderers.DesktopRenderer("desktop", "opengl", "/bin/gltf_viewer")
        data = {
            "name": "SettingsSuite",
            "renderers": ["desktop-opengl"],
            "tests": [
                {
                    "name": "Bloom",
                    "gltf_test": {
                        "models": ["Box"],
                        "rendering": {"view.bloom.enabled": True}
                    }
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        cfg.tests[0].gltf_test.models_map = {"Box": "/models/Box.glb"}
        with tempfile.TemporaryDirectory() as tmpdir:
            cases = renderer._create_test_cases(cfg.tests[0], tmpdir)
        self.assertEqual(json.loads(cases[0].settings_json), {"view": {"bloom": {"enabled": True}}})

    def test_expand_dotted_keys(self):
        rendering = {
            "camera.focalLength": 35.0,
            "view.bloom.enabled": True,
            "view.dithering": "NONE",
            "animation": {"enabled": True, "time": 0.5},
            "lighting.lights": [{"type": "POINT"}],
        }
        self.assertEqual(test_config.expand_dotted_keys(rendering), {
            "camera": {"focalLength": 35.0},
            "view": {"bloom": {"enabled": True}, "dithering": "NONE"},
            "animation": {"enabled": True, "time": 0.5},
            "lighting": {"lights": [{"type": "POINT"}]},
        })

        # AutomationSpec applies keys in order and only touches the fields each one names, so a
        # later key overrides an earlier one field by field.
        self.assertEqual(
            test_config.expand_dotted_keys(
                {"animation": {"enabled": True, "time": 0.5}, "animation.time": 1.0}),
            {"animation": {"enabled": True, "time": 1.0}})

        # The expansion must not alias the test's own rendering dictionary.
        source = {"animation": {"enabled": True}}
        test_config.expand_dotted_keys(source)["animation"]["enabled"] = False
        self.assertTrue(source["animation"]["enabled"])


class TestModelVariant(unittest.TestCase):
    """Tests the choice among glTF-Sample-Assets variants of a model."""

    def setUp(self):
        self._models = tempfile.TemporaryDirectory()
        self.addCleanup(self._models.cleanup)
        root = self._models.name
        # Duck has several variants; Box has no quantized variant.
        for rel in ("Duck/glTF-Binary/Duck.glb", "Duck/glTF/Duck.gltf",
                    "Duck/glTF-Quantized/Duck.gltf", "Duck/glTF-Draco/Duck.gltf",
                    "Box/glTF-Binary/Box.glb"):
            file_path = os.path.join(root, rel)
            os.makedirs(os.path.dirname(file_path), exist_ok=True)
            open(file_path, "w").close()
        test_config._MODEL_SCAN_CACHE.clear()

    def _suite(self, preset_gltf, test_gltf):
        return {
            "name": "VariantSuite",
            "renderers": ["desktop-webgpu"],
            "presets": [{"name": "base", "gltf_test": preset_gltf}],
            "tests": [{"name": "T", "apply_presets": ["base"], "gltf_test": test_gltf}],
        }

    def _model_path(self, rel):
        return os.path.join(self._models.name, rel)

    def test_default_prefers_binary(self):
        cfg = test_config.RenderTestConfig(self._suite(
            {"model_search_paths": [self._models.name]}, {"models": ["Duck"]}))
        self.assertEqual(cfg.tests[0].gltf_test.models_map["Duck"],
                         self._model_path("Duck/glTF-Binary/Duck.glb"))

    def test_explicit_variant(self):
        cfg = test_config.RenderTestConfig(self._suite(
            {"model_search_paths": [self._models.name]},
            {"models": ["Duck"], "model_variant": "glTF-Quantized"}))
        gltf = cfg.tests[0].gltf_test
        self.assertEqual(gltf.model_variant, "glTF-Quantized")
        self.assertEqual(gltf.models_map["Duck"],
                         self._model_path("Duck/glTF-Quantized/Duck.gltf"))

    def test_variant_inherited_from_preset(self):
        cfg = test_config.RenderTestConfig(self._suite(
            {"model_search_paths": [self._models.name], "model_variant": "glTF-Draco"},
            {"models": ["Duck"]}))
        self.assertEqual(cfg.tests[0].gltf_test.models_map["Duck"],
                         self._model_path("Duck/glTF-Draco/Duck.gltf"))

    def test_missing_variant_is_reported(self):
        with self.assertRaises(AssertionError) as raised:
            test_config.RenderTestConfig(self._suite(
                {"model_search_paths": [self._models.name]},
                {"models": ["Box"], "model_variant": "glTF-Quantized"}))
        self.assertIn("glTF-Quantized", str(raised.exception))

    def test_rejects_non_variant_name(self):
        with self.assertRaises(AssertionError):
            test_config.RenderTestConfig(self._suite(
                {"model_search_paths": [self._models.name]},
                {"models": ["Duck"], "model_variant": "Quantized"}))


def _make_tiff(width: int, height: int, pixels: bytes, samples: int = 4) -> bytes:
    """Builds a minimal little-endian, uncompressed TIFF, matching what Filament writes."""
    tags = [
        (256, 4, width),           # ImageWidth
        (257, 4, height),          # ImageLength
        (259, 3, 1),               # Compression: none
        (273, 4, 0),               # StripOffsets, patched below
        (277, 3, samples),         # SamplesPerPixel
        (279, 4, len(pixels)),     # StripByteCounts
    ]
    header = struct.pack('<2sHI', b'II', 42, 8)
    ifd_size = 2 + 12 * len(tags) + 4
    strip_offset = len(header) + ifd_size
    tags = [(tag, ftype, strip_offset if tag == 273 else value) for tag, ftype, value in tags]

    ifd = struct.pack('<H', len(tags))
    for tag, ftype, value in sorted(tags):
        payload = struct.pack('<H', value) + b'\0\0' if ftype == 3 else struct.pack('<I', value)
        ifd += struct.pack('<HHI', tag, ftype, 1) + payload
    ifd += struct.pack('<I', 0)
    return header + ifd + pixels


class TestWebRenderer(unittest.TestCase):
    """Unit tests for the browser-driven renderer's naming and image acceptance rules."""

    def _renderer(self, backend='webgl'):
        # WebRenderer validates that --executable points at a real WASM bundle, so fake one.
        self._bundle = tempfile.TemporaryDirectory()
        self.addCleanup(self._bundle.cleanup)
        for name in ('gltf_viewer.html', 'gltf_viewer.js', 'gltf_viewer.wasm'):
            open(os.path.join(self._bundle.name, name), 'w').close()
        return web_renderer.WebRenderer('web', backend, self._bundle.name)

    def _renderer_with_browser(self, version):
        renderer = self._renderer()
        renderer._browser = types.SimpleNamespace(version=version)
        return renderer

    def test_accepts_the_pinned_browser(self):
        renderer = self._renderer_with_browser(web_renderer.EXPECTED_BROWSER_VERSION)
        renderer._check_browser_version()  # must not raise

    def test_rejects_an_unpinned_browser(self):
        # Without this, an unexpected browser shows up as every web golden mismatching at
        # once, with nothing pointing at the real cause.
        renderer = self._renderer_with_browser('999.0.1.2')
        with self.assertRaises(web_renderer.RasterizerMismatch) as ctx:
            renderer._check_browser_version()
        message = str(ctx.exception)
        self.assertIn('999.0.1.2', message)
        self.assertIn(web_renderer.EXPECTED_BROWSER_VERSION, message)
        # The message has to say how to fix it, not just that it is broken.
        self.assertIn('playwright install chromium', message)

    def test_browser_pin_can_be_overridden(self):
        renderer = self._renderer_with_browser('999.0.1.2')
        os.environ['RDIFF_ALLOW_ANY_BROWSER'] = '1'
        self.addCleanup(os.environ.pop, 'RDIFF_ALLOW_ANY_BROWSER', None)
        renderer._check_browser_version()  # must not raise

    def test_goldens_are_namespaced_by_platform(self):
        # Web goldens must never collide with desktop goldens: different rasterizer, different
        # pixels.
        renderer = self._renderer()
        case = web_renderer.WebGltfRenderTestCase(
            test_name="PointLights",
            backend="webgl",
            output_dir="/tmp/out",
            model="lucy",
            model_path="/tmp/lucy.glb",
            test_json_path="/tmp/test.json"
        )
        self.assertEqual(case.get_out_name(renderer), "PointLights.web-webgl.lucy")
        self.assertEqual(case.get_out_tif_name(renderer), "/tmp/out/PointLights.web-webgl.lucy.tif")
        self.assertEqual(case.get_working_dir(renderer), "/tmp/renderdiff/web-webgl/PointLights/lucy")

        renderer_webgpu = self._renderer(backend='webgpu')
        case_webgpu = web_renderer.WebGltfRenderTestCase(
            test_name="PointLights",
            backend="webgpu",
            output_dir="/tmp/out",
            model="lucy",
            model_path="/tmp/lucy.glb",
            test_json_path="/tmp/test.json"
        )
        self.assertEqual(case_webgpu.get_out_name(renderer_webgpu), "PointLights.web-webgpu.lucy")
        self.assertEqual(case_webgpu.get_out_tif_name(renderer_webgpu), "/tmp/out/PointLights.web-webgpu.lucy.tif")
        self.assertEqual(case_webgpu.get_working_dir(renderer_webgpu), "/tmp/renderdiff/web-webgpu/PointLights/lucy")

    def test_rejects_non_web_backend(self):
        with self.assertRaises(ValueError):
            self._renderer(backend='vulkan')

    def test_rejects_missing_bundle(self):
        with tempfile.TemporaryDirectory() as empty:
            with self.assertRaises(FileNotFoundError):
                web_renderer.WebRenderer('web', 'webgl', empty)

    def test_sample_tests_are_not_silently_dropped(self):
        renderer = self._renderer()
        data = {
            "name": "Suite",
            "renderers": ["web-webgl"],
            "tests": [
                {
                    "name": "SampleTest",
                    "sample_test": {"executable": "hellotriangle", "args": []}
                }
            ]
        }
        cfg = test_config.RenderTestConfig(data)
        with tempfile.TemporaryDirectory() as tmpdir:
            with self.assertRaises(NotImplementedError):
                renderer._create_test_cases(cfg.tests[0], tmpdir)

    def test_accepts_a_well_formed_render(self):
        renderer = self._renderer()
        width, height = renderer.RENDER_WIDTH, renderer.RENDER_HEIGHT
        pixels = bytes((i * 7) % 251 for i in range(width * height * 4))
        self.assertIsNone(renderer._validate_tif(_make_tiff(width, height, pixels)))

    def test_rejects_wrong_size(self):
        # The image size depends on an empirically measured ImGui sidebar width, so it can drift
        # silently. If it ever does, every golden would be rewritten; refuse instead.
        renderer = self._renderer()
        width, height = renderer.RENDER_WIDTH + 1, renderer.RENDER_HEIGHT
        pixels = bytes((i * 7) % 251 for i in range(width * height * 4))
        problem = renderer._validate_tif(_make_tiff(width, height, pixels))
        self.assertIsNotNone(problem)
        self.assertIn('SIDEBAR_WIDTH', problem)

    def test_rejects_blank_render(self):
        # A correctly sized all-black image is the classic false pass.
        renderer = self._renderer()
        width, height = renderer.RENDER_WIDTH, renderer.RENDER_HEIGHT
        problem = renderer._validate_tif(_make_tiff(width, height, bytes(width * height * 4)))
        self.assertIsNotNone(problem)
        self.assertIn('trivial', problem)

    def test_rejects_non_tiff(self):
        renderer = self._renderer()
        self.assertIsNotNone(renderer._validate_tif(b'\x89PNG\r\n\x1a\n' + bytes(64)))



class TestGoldenDeletion(unittest.TestCase):
    """Deleting goldens is destructive and hard to notice, so the scoping rules are pinned here."""

    def setUp(self):
        # The real comparator shells out to the native diffimg tool; these tests only care about
        # which files are selected, not whether their pixels differ.
        self._real_same = update_golden._same_image_diffimg
        update_golden._same_image_diffimg = lambda diffimg, a, b: True
        self.addCleanup(lambda: setattr(update_golden, '_same_image_diffimg', self._real_same))

    def _tree(self, root, names):
        for name in names:
            path = os.path.join(root, name)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'w') as f:
                f.write(name)

    def _deletes(self, golden_names, new_names):
        golden = tempfile.TemporaryDirectory()
        update = tempfile.TemporaryDirectory()
        self.addCleanup(golden.cleanup)
        self.addCleanup(update.cleanup)
        self._tree(golden.name, golden_names)
        self._tree(update.name, new_names)
        deletes, _ = update_golden._get_deletes_updates(update.name, golden.name, 'diffimg')
        return set(deletes)

    def test_desktop_run_does_not_delete_web_goldens(self):
        # The regression this guards: web and desktop goldens are produced by separate CI jobs,
        # so a desktop render set must not be treated as authoritative for web.
        deletes = self._deletes(
            golden_names=[
                'presubmit/Bloom.desktop-opengl.DamagedHelmet.tif',
                'presubmit/Bloom.web-webgl.DamagedHelmet.tif',
                'presubmit/Removed.desktop-opengl.DamagedHelmet.tif',
            ],
            new_names=['presubmit/Bloom.desktop-opengl.DamagedHelmet.tif'])
        self.assertEqual(deletes, {'./presubmit/Removed.desktop-opengl.DamagedHelmet.tif'})

    def test_web_run_does_not_delete_desktop_goldens(self):
        deletes = self._deletes(
            golden_names=[
                'presubmit/Bloom.desktop-opengl.DamagedHelmet.tif',
                'presubmit/Removed.web-webgl.DamagedHelmet.tif',
                'presubmit/Bloom.web-webgl.DamagedHelmet.tif',
            ],
            new_names=['presubmit/Bloom.web-webgl.DamagedHelmet.tif'])
        self.assertEqual(deletes, {'./presubmit/Removed.web-webgl.DamagedHelmet.tif'})

    def test_empty_render_set_deletes_nothing(self):
        # A render job that produced nothing (crashed, or was skipped) must not be read as
        # "every golden is stale".
        deletes = self._deletes(
            golden_names=[
                'presubmit/Bloom.desktop-opengl.DamagedHelmet.tif',
                'presubmit/Bloom.web-webgl.DamagedHelmet.tif',
            ],
            new_names=[])
        self.assertEqual(deletes, set())

    def test_unnamespaced_files_are_never_deleted(self):
        # test.json / render_results_*.json do not encode a renderer; they are refreshed by the
        # update path, never pruned.
        deletes = self._deletes(
            golden_names=['presubmit/test.json', 'presubmit/Bloom.web-webgl.X.tif'],
            new_names=['presubmit/Bloom.web-webgl.X.tif'])
        self.assertEqual(deletes, set())

    def test_aggregated_run_prunes_both_platforms(self):
        # When both legs' renders are merged before updating, stale goldens on either platform
        # are still pruned.
        deletes = self._deletes(
            golden_names=[
                'presubmit/Gone.desktop-opengl.X.tif',
                'presubmit/Gone.web-webgl.X.tif',
                'presubmit/Kept.desktop-opengl.X.tif',
                'presubmit/Kept.web-webgl.X.tif',
            ],
            new_names=[
                'presubmit/Kept.desktop-opengl.X.tif',
                'presubmit/Kept.web-webgl.X.tif',
            ])
        self.assertEqual(deletes, {'./presubmit/Gone.desktop-opengl.X.tif',
                                   './presubmit/Gone.web-webgl.X.tif'})


class TestCompareScoping(unittest.TestCase):
    """Each CI matrix leg renders one platform, so compare.py must be able to scope to it."""

    def setUp(self):
        # compare.py shells out to the native diffimg tool. These tests are about which files
        # get selected, so stand in a comparator that always reports a match.
        self._real_diffimg = compare._run_diffimg
        compare._run_diffimg = lambda *args, **kwargs: (True, {})
        self.addCleanup(lambda: setattr(compare, '_run_diffimg', self._real_diffimg))

    def _tree(self, root, names):
        for name in names:
            path = os.path.join(root, name)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'wb') as f:
                f.write(b'tif')

    def _compare(self, golden_names, render_names, platform=None, out_dir=None):
        golden = tempfile.TemporaryDirectory()
        render = tempfile.TemporaryDirectory()
        self.addCleanup(golden.cleanup)
        self.addCleanup(render.cleanup)
        self._tree(golden.name, golden_names)
        self._tree(render.name, render_names)
        results = compare._compare_goldens(golden.name, render.name, 'diffimg',
                                           out_dir=out_dir, platform=platform)
        return {r['name']: r['result'] for r in results}

    GOLDENS = [
        'presubmit/Bloom.desktop-opengl.Helmet.tif',
        'presubmit/Bloom.web-webgl.Helmet.tif',
    ]

    def test_scoped_run_ignores_other_platform(self):
        # The regression this guards: a desktop-only CI leg walking the whole golden tree
        # reports every web golden as missing and fails the build.
        results = self._compare(
            golden_names=self.GOLDENS,
            render_names=['presubmit/Bloom.desktop-opengl.Helmet.tif'],
            platform='desktop')
        self.assertEqual(results, {'Bloom.desktop-opengl.Helmet.tif': results_mod.RESULT_OK})

    def test_scoped_web_run_ignores_desktop(self):
        results = self._compare(
            golden_names=self.GOLDENS,
            render_names=['presubmit/Bloom.web-webgl.Helmet.tif'],
            platform='web')
        self.assertEqual(results, {'Bloom.web-webgl.Helmet.tif': results_mod.RESULT_OK})

    def test_unscoped_run_still_reports_missing(self):
        # Without --platform the old behaviour must be preserved, otherwise a genuinely
        # missing render would be silently ignored.
        results = self._compare(
            golden_names=self.GOLDENS,
            render_names=['presubmit/Bloom.desktop-opengl.Helmet.tif'])
        self.assertEqual(results.get('Bloom.web-webgl.Helmet.tif'), results_mod.RESULT_MISSING)

    def test_scoped_run_ignores_other_platform_renders(self):
        # A render with no golden is normally GOLDEN_MISSING (a build failure). Another
        # platform's render must not trigger that in a scoped run.
        results = self._compare(
            golden_names=['presubmit/Bloom.desktop-opengl.Helmet.tif'],
            render_names=[
                'presubmit/Bloom.desktop-opengl.Helmet.tif',
                'presubmit/New.web-webgl.Helmet.tif',
            ],
            platform='desktop')
        self.assertEqual(results, {'Bloom.desktop-opengl.Helmet.tif': results_mod.RESULT_OK})

    def test_scoped_run_still_reports_its_own_missing_render(self):
        # Scoping must not weaken detection within the platform being compared.
        results = self._compare(
            golden_names=self.GOLDENS,
            render_names=[],
            platform='web')
        self.assertEqual(results, {'Bloom.web-webgl.Helmet.tif': results_mod.RESULT_MISSING})

    def test_results_filename_is_namespaced_only_when_scoped(self):
        # Matrix legs upload into a shared artifact tree; an unqualified filename would have
        # one leg overwrite the other's results.
        for platform, expected in (('web', 'compare_results_web.json'),
                                   (None, 'compare_results.json')):
            out = tempfile.TemporaryDirectory()
            self.addCleanup(out.cleanup)
            self._compare(golden_names=self.GOLDENS,
                          render_names=['presubmit/Bloom.web-webgl.Helmet.tif'],
                          platform=platform, out_dir=out.name)
            written = os.listdir(os.path.join(out.name, 'presubmit'))
            self.assertIn(expected, written)


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
    loader = unittest.TestLoader()
    suite = unittest.TestSuite(
        loader.loadTestsFromTestCase(cls)
        for cls in (TestSchemaInvariants, TestModelVariant, TestWebRenderer,
                    TestGoldenDeletion, TestCompareScoping))

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
