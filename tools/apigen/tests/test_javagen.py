#!/usr/bin/env python3
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

import difflib
import glob
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

# Add parent directory to path to import javagen
sys.path.append(str(Path(__file__).parent.parent))
import javagen

class TestJavaGen(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.test_dir = Path(__file__).parent / "java"

    def test_golden_java_and_cpp_generation(self):
        """Verify that all JSON IR test fixtures generate matching Java and JNI C++ code."""
        json_files = sorted(self.test_dir.glob("*.json"))
        self.assertTrue(len(json_files) > 0, "No JSON test files found")

        for jf in json_files:
            base = jf.stem
            expected_java = self.test_dir / f"{base}.java"
            expected_cpp = self.test_dir / f"{base}.cpp"

            if not expected_java.exists() and not expected_cpp.exists():
                continue

            with tempfile.TemporaryDirectory() as tmp_dir:
                res = subprocess.run(
                    [
                        sys.executable,
                        str(Path(__file__).parent.parent / "javagen.py"),
                        "--java-dir", tmp_dir,
                        "--jni-dir", tmp_dir,
                        str(jf),
                    ],
                    capture_output=True,
                    text=True,
                )
                self.assertEqual(res.returncode, 0, f"javagen failed for {base}: {res.stderr}")

                if expected_java.exists():
                    gen_java_files = list(Path(tmp_dir).glob("*.java"))
                    self.assertEqual(len(gen_java_files), 1, f"Expected 1 Java file for {base}, got {len(gen_java_files)}")
                    with open(expected_java) as f:
                        expected_content = f.read()
                    with open(gen_java_files[0]) as f:
                        actual_content = f.read()
                    
                    diff = "\n".join(
                        difflib.unified_diff(
                            expected_content.splitlines(),
                            actual_content.splitlines(),
                            fromfile=str(expected_java),
                            tofile=str(gen_java_files[0]),
                        )
                    )
                    self.assertEqual(actual_content, expected_content, f"Java output mismatch for {base}:\n{diff}")

                if expected_cpp.exists():
                    gen_cpp_files = list(Path(tmp_dir).glob("*.cpp"))
                    self.assertEqual(len(gen_cpp_files), 1, f"Expected 1 CPP file for {base}, got {len(gen_cpp_files)}")
                    with open(expected_cpp) as f:
                        expected_content = f.read()
                    with open(gen_cpp_files[0]) as f:
                        actual_content = f.read()
                    diff = "\n".join(
                        difflib.unified_diff(
                            expected_content.splitlines(),
                            actual_content.splitlines(),
                            fromfile=str(expected_cpp),
                            tofile=str(gen_cpp_files[0]),
                        )
                    )
                    self.assertEqual(actual_content, expected_content, f"CPP output mismatch for {base}:\n{diff}")

    def test_frustum_value_object_generation(self):
        """Verify that Frustum generates as a value object without native handle."""
        wip_frustum_json = Path(__file__).parent.parent / "wip" / "Frustum.json"
        if not wip_frustum_json.exists():
            return

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(wip_frustum_json),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed for Frustum: {res.stderr}")

            java_path = Path(tmp_dir) / "Frustum.java"
            cpp_path = Path(tmp_dir) / "Frustum.cpp"
            self.assertTrue(java_path.exists())
            self.assertTrue(cpp_path.exists())

            java_src = java_path.read_text()
            cpp_src = cpp_path.read_text()

            # Invariants
            self.assertIn("/* package */ final float[] mPlanes = new float[24];", java_src)
            self.assertNotIn("mNativeObject", java_src)
            self.assertNotIn("getNativeObject()", java_src)
            self.assertIn("public boolean intersects(@NonNull Box box)", java_src)
            self.assertIn("nIntersects(mPlanes, box.getCenterX(), box.getCenterY(), box.getCenterZ(), box.getHalfExtentX(), box.getHalfExtentY(), box.getHalfExtentZ())", java_src)

            self.assertIn("reinterpret_cast<Frustum*>(planes)", cpp_src)
            self.assertIn("reinterpret_cast<Frustum const *>(planes)", cpp_src)
            self.assertIn("Box box;", cpp_src)
            self.assertIn("box.center = { boxCenterX, boxCenterY, boxCenterZ };", cpp_src)
            self.assertIn("box.halfExtent = { boxHalfExtentX, boxHalfExtentY, boxHalfExtentZ };", cpp_src)

    def test_generic_aggregate_struct_box_generation(self):
        """Verify that Box generates as a 1-object aggregate struct with exploded primitive storage."""
        wip_box_json = Path(__file__).parent.parent / "wip" / "Box.json"
        if not wip_box_json.exists():
            return

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(wip_box_json),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed for Box: {res.stderr}")

            java_path = Path(tmp_dir) / "Box.java"
            cpp_path = Path(tmp_dir) / "Box.cpp"
            self.assertTrue(java_path.exists())
            self.assertTrue(cpp_path.exists())

            java_src = java_path.read_text()
            cpp_src = cpp_path.read_text()

            # Invariants
            self.assertNotIn("mNativeObject", java_src)
            self.assertNotIn("getNativeObject()", java_src)
            self.assertIn("private float mCenterX;", java_src)
            self.assertIn("private float mHalfExtentX;", java_src)
            self.assertIn("public void setCenter(float centerX, float centerY, float centerZ)", java_src)
            self.assertIn("public void setCenter(@NonNull @Size(min = 3) float[] center)", java_src)
            self.assertIn("public float[] getCenter(@Nullable @Size(min = 3) float[] out)", java_src)
            self.assertIn("public float getCenterX()", java_src)
            self.assertIn("public boolean isEmpty()", java_src)

    def test_texture_sampler_bitfield_archetype_generation(self):
        """Verify that TextureSampler generates as a packed bitfield value object without heap allocation."""
        test_ir = {
            "meta": {"generator_version": "1.0.0"},
            "classes": [
                {
                    "name": "TextureSampler",
                    "qualified_name": "filament::TextureSampler",
                    "category": "bitfield",
                    "archetype": "bitfield",
                    "bitfield_size": 4,
                    "bitfield_primitive": "int",
                    "attributes": [],
                    "doc": {"brief": "TextureSampler defines how a texture is accessed."},
                    "enums": [
                        {
                            "name": "WrapMode",
                            "qualified_name": "filament::TextureSampler::WrapMode",
                            "values": [{"name": "CLAMP_TO_EDGE", "value": "0"}, {"name": "REPEAT", "value": "1"}]
                        },
                        {
                            "name": "MinFilter",
                            "qualified_name": "filament::TextureSampler::MinFilter",
                            "values": [{"name": "NEAREST", "value": "0"}, {"name": "LINEAR", "value": "1"}]
                        },
                        {
                            "name": "MagFilter",
                            "qualified_name": "filament::TextureSampler::MagFilter",
                            "values": [{"name": "NEAREST", "value": "0"}, {"name": "LINEAR", "value": "1"}]
                        }
                    ],
                    "methods": [
                        {
                            "name": "TextureSampler",
                            "is_constructor": True,
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [],
                            "doc": {"brief": "Creates a default sampler."}
                        },
                        {
                            "name": "TextureSampler",
                            "is_constructor": True,
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "minMag", "type": {"cpp_name": "MagFilter", "qualified_name": "filament::TextureSampler::MagFilter", "category": "enum"}, "default_value": None},
                                {"name": "str", "type": {"cpp_name": "WrapMode", "qualified_name": "filament::TextureSampler::WrapMode", "category": "enum"}, "default_value": "WrapMode::CLAMP_TO_EDGE"}
                            ],
                            "doc": {"brief": "Creates a TextureSampler with wrap and filter."}
                        },
                        {
                            "name": "setMinFilter",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "v", "type": {"cpp_name": "MinFilter", "qualified_name": "filament::TextureSampler::MinFilter", "category": "enum"}, "default_value": None}
                            ],
                            "is_const": False,
                            "is_static": False,
                            "is_noexcept": True,
                            "doc": {"brief": "Sets the minification filter"}
                        },
                        {
                            "name": "getMinFilter",
                            "return_type": {"cpp_name": "MinFilter", "qualified_name": "filament::TextureSampler::MinFilter", "category": "enum"},
                            "arguments": [],
                            "is_const": True,
                            "is_static": False,
                            "is_noexcept": True,
                            "doc": {"brief": "returns the minification filter value"}
                        }
                    ],
                    "fields": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "TextureSampler.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed for TextureSampler: {res.stderr}")

            java_src = (Path(tmp_dir) / "TextureSampler.java").read_text()
            cpp_src = (Path(tmp_dir) / "TextureSampler.cpp").read_text()

            # Java Invariants
            self.assertIn("/* package */ int mSampler;", java_src)
            self.assertNotIn("mNativeObject", java_src)
            self.assertNotIn("getNativeObject()", java_src)
            self.assertIn("static final class EnumCache", java_src)
            self.assertIn("static final MinFilter[] sMinFilterValues = MinFilter.values();", java_src)
            self.assertIn("this(minMag, WrapMode.CLAMP_TO_EDGE);", java_src)
            self.assertIn("mSampler = nCreateTextureSamplerMagFilterWrapMode(minMag.toFilamentNative(), str.toFilamentNative());", java_src)
            self.assertIn("mSampler = nSetMinFilter(mSampler, v.toFilamentNative());", java_src)
            self.assertIn("return EnumCache.sMinFilterValues[nGetMinFilter(mSampler)];", java_src)

            # JNI Invariants
            self.assertIn("#include <filament/TextureSampler.h>", cpp_src)
            self.assertIn("#include <utils/algorithm.h>", cpp_src)
            self.assertIn("filament::JniUtils::to_int(TextureSampler{})", cpp_src)
            self.assertIn("TextureSampler that = filament::JniUtils::from_int(sampler_);", cpp_src)
            self.assertIn("that.setMinFilter((TextureSampler::MinFilter)v);", cpp_src)
            self.assertIn("return filament::JniUtils::to_int(that);", cpp_src)
            self.assertIn("return (jint)that.getMinFilter();", cpp_src)

    def test_phase1_string_types_generation(self):
        """Verify string_view, CString, StaticString, ImmutableCString marshaling."""
        test_ir = {
            "classes": [
                {
                    "name": "StringTest",
                    "methods": [
                        {
                            "name": "hasParameter",
                            "return_type": {"cpp_name": "bool", "qualified_name": "bool", "category": "primitive"},
                            "arguments": [
                                {"name": "name", "type": {"cpp_name": "std::string_view", "qualified_name": "std::basic_string_view<char>", "category": "string"}}
                            ]
                        },
                        {
                            "name": "setName",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "name", "type": {"cpp_name": "const utils::CString&", "qualified_name": "const utils::CString&", "category": "string"}}
                            ]
                        },
                        {
                            "name": "getName",
                            "return_type": {"cpp_name": "utils::ImmutableCString", "qualified_name": "utils::ImmutableCString", "category": "string"},
                            "arguments": []
                        }
                    ],
                    "fields": []
                }
            ],
            "enums": []
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "StringTest.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "StringTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "StringTest.cpp").read_text()

            # Java assertions
            self.assertIn("public boolean hasParameter(@NonNull String name)", java_src)
            self.assertIn("public void setName(@NonNull String name)", java_src)
            self.assertIn("public String getName()", java_src)

            # JNI assertions
            self.assertIn("#include <string_view>", cpp_src)
            self.assertIn("#include <utils/CString.h>", cpp_src)
            self.assertIn("#include <utils/ImmutableCString.h>", cpp_src)
            self.assertIn("char const * const name = env->GetStringUTFChars(name_, nullptr);", cpp_src)
            self.assertIn("that->hasParameter(std::string_view(name));", cpp_src)
            self.assertIn("env->ReleaseStringUTFChars(name_, name);", cpp_src)
            self.assertIn("return env->NewStringUTF((that->getName()).c_str());", cpp_src)

    def test_phase1_chrono_and_tribool_generation(self):
        """Verify std::chrono, utils::tribool, std::optional, and utils::bitset marshaling."""
        test_ir = {
            "classes": [
                {
                    "name": "ChronoTest",
                    "methods": [
                        {
                            "name": "setLatency",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "latency", "type": {"cpp_name": "std::chrono::nanoseconds", "qualified_name": "std::chrono::duration<long long, std::ratio>", "category": "chrono"}}
                            ]
                        },
                        {
                            "name": "getPresentationTime",
                            "return_type": {"cpp_name": "std::chrono::time_point", "qualified_name": "std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long long, std::ratio>>", "category": "chrono"},
                            "arguments": []
                        },
                        {
                            "name": "compile",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "mode", "type": {"cpp_name": "utils::tribool", "qualified_name": "utils::tribool", "category": "tribool"}}
                            ]
                        },
                        {
                            "name": "getFlags",
                            "return_type": {"cpp_name": "utils::bitset<uint32_t>", "qualified_name": "utils::bitset<uint32_t>", "category": "object"},
                            "arguments": []
                        }
                    ],
                    "fields": []
                }
            ],
            "enums": []
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "ChronoTest.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "ChronoTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "ChronoTest.cpp").read_text()

            # Java assertions
            self.assertIn("public void setLatency(@IntRange(from = 0) long latency)", java_src)
            self.assertIn("public long getPresentationTime()", java_src)
            self.assertIn("public void compile(boolean mode)", java_src)
            self.assertIn("public int getFlags()", java_src)

            # JNI assertions
            self.assertIn("#include <chrono>", cpp_src)
            self.assertIn("#include <utils/tribool.h>", cpp_src)
            self.assertIn("#include <utils/bitset.h>", cpp_src)
            self.assertIn("std::chrono::nanoseconds(latency)", cpp_src)
            self.assertIn("(jlong)that->getPresentationTime().time_since_epoch().count()", cpp_src)
            self.assertIn("utils::tribool((bool)mode)", cpp_src)
            self.assertIn("(jint)that->getFlags().getValue()", cpp_src)

    def test_constant_extraction_and_generation(self):
        """Verify that public static constexpr constants are extracted and generated into Java."""
        test_ir = {
            "classes": [
                {
                    "name": "ConstantsTest",
                    "type": {"category": "object", "cpp_name": "ConstantsTest"},
                    "methods": [
                        {
                            "name": "doWait",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "timeout",
                                    "type": {"cpp_name": "uint64_t", "qualified_name": "uint64_t", "category": "primitive"},
                                    "default_value": "WAIT_FOR_EVER"
                                }
                            ]
                        }
                    ],
                    "fields": [],
                    "constants": [
                        {
                            "name": "FENCE_WAIT_FOR_EVER",
                            "type": {"cpp_name": "const uint64_t", "qualified_name": "uint64_t", "category": "primitive"},
                            "value": "uint64_t(-1)",
                            "doc": {"brief": "Special timeout value."}
                        },
                        {
                            "name": "WAIT_FOR_EVER",
                            "type": {"cpp_name": "const uint64_t", "qualified_name": "uint64_t", "category": "primitive"},
                            "value": "FENCE_WAIT_FOR_EVER",
                            "doc": {"brief": "Alias for FENCE_WAIT_FOR_EVER."}
                        },
                        {
                            "name": "EFFICIENCY_LED",
                            "type": {"cpp_name": "const float", "qualified_name": "float", "category": "primitive"},
                            "value": "0.1171f",
                            "doc": {"brief": "LED efficiency."}
                        }
                    ]
                }
            ],
            "enums": []
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "ConstantsTest.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "ConstantsTest.java").read_text()
            self.assertIn("public static final long FENCE_WAIT_FOR_EVER = -1;", java_src)
            self.assertIn("public static final long WAIT_FOR_EVER = FENCE_WAIT_FOR_EVER;", java_src)
            self.assertIn("public static final float EFFICIENCY_LED = 0.1171f;", java_src)
            self.assertIn("doWait(WAIT_FOR_EVER);", java_src)

    def test_no_apigen_attribute(self):
        """Verify that methods, classes, fields, and enums annotated with no_apigen are omitted."""
        test_ir = {
            "classes": [
                {
                    "name": "SkippedClass",
                    "qualified_name": "SkippedClass",
                    "attributes": ["no_apigen"],
                    "methods": [],
                    "fields": [],
                    "constants": []
                },
                {
                    "name": "TargetClass",
                    "qualified_name": "TargetClass",
                    "attributes": [],
                    "methods": [
                        {
                            "name": "includedMethod",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "primitive"},
                            "arguments": [],
                            "is_const": False,
                            "is_static": False,
                            "is_noexcept": True,
                            "attributes": []
                        },
                        {
                            "name": "skippedMethod",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "primitive"},
                            "arguments": [],
                            "is_const": False,
                            "is_static": False,
                            "is_noexcept": True,
                            "attributes": ["no_apigen"]
                        }
                    ],
                    "fields": [],
                    "constants": [
                        {
                            "name": "SKIPPED_CONST",
                            "type": {"cpp_name": "const int", "qualified_name": "int", "category": "primitive"},
                            "value": "42",
                            "attributes": ["no_apigen"]
                        },
                        {
                            "name": "INCLUDED_CONST",
                            "type": {"cpp_name": "const int", "qualified_name": "int", "category": "primitive"},
                            "value": "100",
                            "attributes": []
                        }
                    ]
                }
            ],
            "enums": []
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "SkipTest.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            # SkippedClass should not be generated
            self.assertFalse((Path(tmp_dir) / "SkippedClass.java").exists())
            self.assertFalse((Path(tmp_dir) / "SkippedClass.cpp").exists())

            # TargetClass should include includedMethod and INCLUDED_CONST, but omit skippedMethod and SKIPPED_CONST
            java_src = (Path(tmp_dir) / "TargetClass.java").read_text()
            cpp_src = (Path(tmp_dir) / "TargetClass.cpp").read_text()

            self.assertIn("includedMethod", java_src)
            self.assertIn("includedMethod", cpp_src)
            self.assertNotIn("skippedMethod", java_src)
            self.assertNotIn("skippedMethod", cpp_src)

            self.assertIn("INCLUDED_CONST", java_src)
            self.assertNotIn("SKIPPED_CONST", java_src)

    def test_utility_class_generation(self):
        """Verify that utility classes / namespaces generate static methods and no native handle."""
        test_ir = {
            "meta": {"generator_version": "1.0.0"},
            "classes": [
                {
                    "name": "Exposure",
                    "qualified_name": "filament::Exposure",
                    "category": "utility",
                    "is_utility": True,
                    "is_namespace": True,
                    "location": {"file": "filament/Exposure.h", "line": 1},
                    "doc": {"brief": "Exposure utilities."},
                    "methods": [
                        {
                            "name": "ev100FromLuminance",
                            "return_type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive"},
                            "arguments": [
                                {
                                    "name": "luminance",
                                    "type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive"},
                                    "default_value": None
                                }
                            ],
                            "is_const": False,
                            "is_static": True,
                            "is_noexcept": True,
                            "attributes": []
                        }
                    ],
                    "fields": [],
                    "constants": [],
                    "enums": [],
                    "bases": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "Exposure.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "Exposure.java").read_text()
            cpp_src = (Path(tmp_dir) / "Exposure.cpp").read_text()

            # Java checks
            self.assertIn("private Exposure()", java_src)
            self.assertNotIn("mNativeObject", java_src)
            self.assertNotIn("getNativeObject()", java_src)
            self.assertIn("public static float ev100FromLuminance(float luminance)", java_src)
            self.assertIn("private static native float nEv100FromLuminance(float luminance);", java_src)

            # C++ checks
            self.assertNotIn("nativeExposure", cpp_src)
            self.assertNotIn("that->", cpp_src)
            self.assertNotIn("namespace filament::math {}", cpp_src)
            self.assertNotIn("using namespace filament::math;", cpp_src)
            self.assertIn("Exposure::ev100FromLuminance(luminance)", cpp_src)

    def test_color_and_colors_generation(self):
        """Verify Color class generation with name override, alias annotations, and math vector overloads."""
        test_ir = {
            "meta": {"generator_version": "1.0.0"},
            "aliases": [
                {"name": "LinearColor", "type": {"cpp_name": "LinearColor", "qualified_name": "filament::math::details::TVec3<float>"}}
            ],
            "enums": [
                {
                    "name": "RgbType",
                    "qualified_name": "filament::RgbType",
                    "values": [{"name": "sRGB", "value": "0"}, {"name": "LINEAR", "value": "1"}]
                }
            ],
            "classes": [
                {
                    "name": "Color",
                    "qualified_name": "filament::Color",
                    "category": "utility",
                    "is_utility": True,
                    "location": {"file": "filament/Color.h", "line": 1},
                    "doc": {"brief": "Utilities to manipulate and convert colors"},
                    "methods": [
                        {
                            "name": "toLinear",
                            "return_type": {"cpp_name": "LinearColor", "qualified_name": "filament::math::details::TVec3<float>"},
                            "arguments": [
                                {
                                    "name": "type",
                                    "type": {"cpp_name": "RgbType", "qualified_name": "filament::RgbType", "category": "enum"},
                                    "default_value": None
                                },
                                {
                                    "name": "color",
                                    "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::details::TVec3<float>"},
                                    "default_value": None
                                }
                            ],
                            "is_const": False,
                            "is_static": True,
                            "is_noexcept": True,
                            "attributes": []
                        }
                    ],
                    "fields": [],
                    "constants": [],
                    "enums": [],
                    "bases": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "Color.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    "--class-name", "Colors",
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "Colors.java").read_text()
            cpp_src = (Path(tmp_dir) / "Colors.cpp").read_text()

            # Java checks
            self.assertIn("public class Colors", java_src)
            self.assertIn("public @interface LinearColor", java_src)
            self.assertIn("public enum RgbType", java_src)
            self.assertIn("@NonNull @Size(min = 3) @LinearColor", java_src)
            self.assertIn("public static float[] toLinear(@NonNull RgbType type, @NonNull @Size(min = 3) float[] color, @Nullable @Size(min = 3) float[] out)", java_src)
            self.assertIn("public static float[] toLinear(@NonNull RgbType type, float colorx, float colory, float colorz, @Nullable @Size(min = 3) float[] out)", java_src)
            self.assertIn("nToLinear(type.toFilamentNative(), colorx, colory, colorz, out);", java_src)

            # C++ checks
            self.assertIn("Java_com_google_android_filament_Colors_nToLinear", cpp_src)
            self.assertIn("Color::toLinear((RgbType)type, float3{colorx, colory, colorz})", cpp_src)
            self.assertIn("jclass clazz", cpp_src)

    def test_enum_cache_generation(self):
        """Verify that EnumCache inner class is generated and used for zero-allocation enum return values."""
        test_ir = {
            "meta": {"generator_version": "1.0.0"},
            "classes": [
                {
                    "name": "StateHolder",
                    "qualified_name": "filament::StateHolder",
                    "category": "object",
                    "enums": [
                        {
                            "name": "State",
                            "qualified_name": "filament::StateHolder::State",
                            "values": [{"name": "OFF", "value": "0"}, {"name": "ON", "value": "1"}]
                        },
                        {
                            "name": "Mode",
                            "qualified_name": "filament::StateHolder::Mode",
                            "values": [{"name": "AUTO", "value": "0"}, {"name": "MANUAL", "value": "1"}]
                        }
                    ],
                    "methods": [
                        {
                            "name": "getState",
                            "return_type": {"cpp_name": "State", "qualified_name": "filament::StateHolder::State", "category": "enum"},
                            "arguments": [],
                            "is_const": True,
                            "is_static": False,
                            "is_noexcept": True
                        },
                        {
                            "name": "getMode",
                            "return_type": {"cpp_name": "Mode", "qualified_name": "filament::StateHolder::Mode", "category": "enum"},
                            "arguments": [],
                            "is_const": True,
                            "is_static": False,
                            "is_noexcept": True
                        }
                    ],
                    "fields": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "StateHolder.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "StateHolder.java").read_text()

            # Verify EnumCache exists and has cached arrays
            self.assertIn("static final class EnumCache {", java_src)
            self.assertIn("static final Mode[] sModeValues = Mode.values();", java_src)
            self.assertIn("static final State[] sStateValues = State.values();", java_src)

            # Verify getters use EnumCache instead of calling .values() in method body
            self.assertIn("return EnumCache.sStateValues[nGetState(getNativeObject())];", java_src)
            self.assertIn("return EnumCache.sModeValues[nGetMode(getNativeObject())];", java_src)

    def test_builder_pattern_java_and_jni_generation(self):
        """Verify that nested Builder generates nested Builder class in Java and correct JNI bridges."""
        test_ir = {
            "classes": [
                {
                    "name": "Texture",
                    "qualified_name": "filament::Texture",
                    "attributes": ["apigen:skip"],
                    "type": {"cpp_name": "Texture", "category": "object"},
                    "methods": [],
                    "fields": [],
                    "constants": []
                },
                {
                    "name": "Skybox",
                    "qualified_name": "filament::Skybox",
                    "location": {"file": "filament/Skybox.h", "line": 20},
                    "type": {"cpp_name": "Skybox", "category": "object"},
                    "methods": [
                        {
                            "name": "setColor",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [{"name": "color", "type": {"cpp_name": "float", "category": "primitive"}}],
                            "is_const": False,
                            "is_static": False,
                            "is_noexcept": True
                        }
                    ],
                    "fields": [],
                    "constants": []
                },
                {
                    "name": "Builder",
                    "qualified_name": "filament::Skybox::Builder",
                    "parent_class": "Skybox",
                    "is_nested": True,
                    "is_builder": True,
                    "archetype": "builder",
                    "category": "builder",
                    "location": {"file": "filament/Skybox.h", "line": 30},
                    "bases": ["BuilderBase<BuilderDetails>"],
                    "methods": [
                        {
                            "name": "Builder",
                            "is_constructor": True,
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [],
                            "is_noexcept": True
                        },
                        {
                            "name": "environment",
                            "return_type": {"cpp_name": "Builder &", "category": "object"},
                            "arguments": [
                                {
                                    "name": "cubemap",
                                    "type": {
                                        "cpp_name": "Texture",
                                        "qualified_name": "filament::Texture",
                                        "category": "object",
                                        "nullability": "nullable",
                                        "is_pointer": True,
                                        "is_const": True
                                    }
                                }
                            ],
                            "is_noexcept": True
                        },
                        {
                            "name": "build",
                            "return_type": {"cpp_name": "Skybox *", "category": "object"},
                            "arguments": [
                                {
                                    "name": "engine",
                                    "type": {
                                        "cpp_name": "Engine &",
                                        "qualified_name": "filament::Engine&",
                                        "category": "object"
                                    }
                                }
                            ],
                            "is_noexcept": False
                        }
                    ],
                    "fields": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "Skybox.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            # Verify only Skybox.java and Skybox.cpp are generated (no standalone Builder.java)
            java_files = [p.name for p in Path(tmp_dir).glob("*.java")]
            cpp_files = [p.name for p in Path(tmp_dir).glob("*.cpp")]
            self.assertEqual(java_files, ["Skybox.java"])
            self.assertEqual(cpp_files, ["Skybox.cpp"])

            java_src = (Path(tmp_dir) / "Skybox.java").read_text()
            cpp_src = (Path(tmp_dir) / "Skybox.cpp").read_text()

            # Verify Java nested Builder class and methods
            self.assertIn("public static class Builder {", java_src)
            self.assertIn("private final BuilderFinalizer mFinalizer;", java_src)
            self.assertIn("private final long mNativeBuilder;", java_src)
            self.assertIn("public Builder environment(@Nullable Texture cubemap) {", java_src)
            self.assertIn("nBuilderEnvironment(mNativeBuilder, cubemap != null ? cubemap.getNativeObject() : 0);", java_src)
            self.assertIn("return this;", java_src)
            self.assertIn("public Skybox build(@NonNull Engine engine) {", java_src)
            self.assertIn("long nativeSkybox = nBuilderBuild(mNativeBuilder, engine.getNativeObject());", java_src)
            self.assertIn("if (nativeSkybox == 0) throw new IllegalStateException(\"Couldn't create Skybox\");", java_src)
            self.assertIn("return new Skybox(nativeSkybox);", java_src)

            # Verify native declarations in Java
            self.assertIn("private static native long nCreateBuilder();", java_src)
            self.assertIn("private static native void nDestroyBuilder(long nativeBuilder);", java_src)
            self.assertIn("private static native void nBuilderEnvironment(long nativeBuilder, long cubemap);", java_src)
            self.assertIn("private static native long nBuilderBuild(long nativeBuilder, long nativeEngine);", java_src)

            # Verify JNI C++ functions
            self.assertIn("#include <filament/Engine.h>", cpp_src)
            self.assertIn("Java_com_google_android_filament_Skybox_nCreateBuilder", cpp_src)
            self.assertIn("Java_com_google_android_filament_Skybox_nDestroyBuilder", cpp_src)
            self.assertIn("Java_com_google_android_filament_Skybox_nBuilderEnvironment", cpp_src)
            self.assertIn("builder->environment((const Texture*)cubemap);", cpp_src)
            self.assertIn("Java_com_google_android_filament_Skybox_nBuilderBuild", cpp_src)
            self.assertIn("return (jlong) builder->build(*engine);", cpp_src)

    def test_nested_aggregate_struct_and_fixed_array_generation(self):
        """Verify nested aggregate structs (e.g. ShadowOptions, Vsm) and fixed-size array flattening."""
        test_ir = {
            "classes": [
                {
                    "name": "Vsm",
                    "qualified_name": "filament::LightManager::ShadowOptions::Vsm",
                    "parent_class": "ShadowOptions",
                    "is_nested": True,
                    "is_aggregate": True,
                    "fields": [
                        {"name": "elvsm", "type": {"cpp_name": "bool", "qualified_name": "bool", "category": "primitive"}, "default_value": "false"},
                        {"name": "blurWidth", "type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive"}, "default_value": "0.0f"}
                    ],
                    "methods": [],
                    "enums": [],
                    "constants": []
                },
                {
                    "name": "ShadowOptions",
                    "qualified_name": "filament::LightManager::ShadowOptions",
                    "parent_class": "LightManager",
                    "is_nested": True,
                    "is_aggregate": True,
                    "fields": [
                        {"name": "mapSize", "type": {"cpp_name": "uint32_t", "qualified_name": "uint32_t", "category": "primitive"}, "default_value": "1024"},
                        {"name": "cascadeSplitPositions", "type": {"cpp_name": "float[3]", "qualified_name": "float[3]", "category": "unknown"}, "default_value": "{ 0.125f, 0.25f, 0.50f }"},
                        {"name": "vsm", "type": {"cpp_name": "struct Vsm", "qualified_name": "filament::LightManager::ShadowOptions::Vsm", "category": "object"}},
                        {"name": "transform", "type": {"cpp_name": "math::quatf", "qualified_name": "filament::math::details::TQuaternion<float>", "category": "object"}}
                    ],
                    "methods": [],
                    "enums": [],
                    "constants": []
                },
                {
                    "name": "LightManager",
                    "qualified_name": "filament::LightManager",
                    "category": "manager",
                    "methods": [
                        {
                            "name": "setShadowOptions",
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "i",
                                    "type": {
                                        "cpp_name": "Instance",
                                        "qualified_name": "filament::utils::EntityInstance<filament::LightManager>",
                                        "category": "primitive"
                                    }
                                },
                                {
                                    "name": "options",
                                    "type": {
                                        "cpp_name": "const ShadowOptions &",
                                        "qualified_name": "const filament::LightManager::ShadowOptions&",
                                        "category": "object"
                                    }
                                }
                            ],
                            "is_const": False,
                            "is_noexcept": True
                        }
                    ],
                    "fields": [],
                    "enums": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "LightManager.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            # Verify only LightManager.java and LightManager.cpp are generated (no standalone ShadowOptions.java or Vsm.java)
            java_files = sorted([p.name for p in Path(tmp_dir).glob("*.java")])
            cpp_files = sorted([p.name for p in Path(tmp_dir).glob("*.cpp")])
            self.assertEqual(java_files, ["LightManager.java"])
            self.assertEqual(cpp_files, ["LightManager.cpp"])

            java_src = (Path(tmp_dir) / "LightManager.java").read_text()
            cpp_src = (Path(tmp_dir) / "LightManager.cpp").read_text()

            # Verify nested class structure in Java
            self.assertIn("public static class ShadowOptions {", java_src)
            self.assertIn("public static class Vsm {", java_src)
            self.assertIn("private float mCascadeSplitPositions0;", java_src)
            self.assertIn("private float mCascadeSplitPositions1;", java_src)
            self.assertIn("private float mCascadeSplitPositions2;", java_src)
            self.assertIn("public void setCascadeSplitPositions(@NonNull @Size(min = 3) float[] cascadeSplitPositions)", java_src)
            self.assertIn("public float[] getCascadeSplitPositions(@Nullable @Size(min = 3) float[] out)", java_src)
            self.assertIn("public void setTransform(@NonNull @Size(min = 4) float[] transform)", java_src)
            self.assertIn("public void setVsm(@NonNull Vsm vsm)", java_src)

            # Verify JNI C++ emission
            self.assertIn("LightManager::ShadowOptions options;", cpp_src)
            self.assertIn("options.mapSize = optionsMapSize;", cpp_src)
            self.assertIn("options.cascadeSplitPositions[0] = optionsCascadeSplitPositions0;", cpp_src)
            self.assertIn("options.cascadeSplitPositions[1] = optionsCascadeSplitPositions1;", cpp_src)
            self.assertIn("options.cascadeSplitPositions[2] = optionsCascadeSplitPositions2;", cpp_src)
            self.assertIn("options.vsm.elvsm = optionsVsmElvsm;", cpp_src)
            self.assertIn("options.vsm.blurWidth = optionsVsmBlurWidth;", cpp_src)
            self.assertIn("options.transform = { optionsTransformX, optionsTransformY, optionsTransformZ, optionsTransformW };", cpp_src)
            self.assertIn("that->setShadowOptions(EntityInstance<LightManager>(i), options);", cpp_src)

    def test_custom_negative_enums_and_deduplication(self):
        """Verify custom/negative enum generation and alias deduplication in from(int)."""
        test_ir = {
            "classes": [
                {
                    "name": "CustomEnumTest",
                    "qualified_name": "filament::CustomEnumTest",
                    "archetype": "utility",
                    "enums": [
                        {
                            "name": "FrameStatus",
                            "qualified_name": "filament::CustomEnumTest::FrameStatus",
                            "is_scoped": True,
                            "underlying_type": "int8_t",
                            "entries": [
                                {"name": "SKIPPED_SPURIOUS", "value": -2},
                                {"name": "SKIPPED_STALE", "value": -1},
                                {"name": "ACCEPTED", "value": 0}
                            ]
                        },
                        {
                            "name": "AttachmentPoint",
                            "qualified_name": "filament::CustomEnumTest::AttachmentPoint",
                            "is_scoped": True,
                            "underlying_type": "uint8_t",
                            "entries": [
                                {"name": "COLOR0", "value": 0},
                                {"name": "COLOR1", "value": 1},
                                {"name": "DEPTH", "value": 8},
                                {"name": "COLOR", "value": 0}
                            ]
                        }
                    ],
                    "methods": [],
                    "fields": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "CustomEnumTest.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "CustomEnumTest.java").read_text()

            # Verify FrameStatus has negative values, toFilamentNative, and from(int)
            self.assertIn("SKIPPED_SPURIOUS(-2)", java_src)
            self.assertIn("SKIPPED_STALE(-1)", java_src)
            self.assertIn("ACCEPTED(0)", java_src)
            self.assertIn("public int toFilamentNative() { return mValue; }", java_src)
            self.assertIn("case -2: return SKIPPED_SPURIOUS;", java_src)
            self.assertIn("case -1: return SKIPPED_STALE;", java_src)
            self.assertIn("case 0: return ACCEPTED;", java_src)

            # Verify AttachmentPoint deduplicates case 0 (only 1 case 0 in switch)
            self.assertEqual(java_src.count("case 0: return COLOR0;"), 1)
            self.assertEqual(java_src.count("case 0: return COLOR;"), 0)

    def test_struct_slices_and_cleanup_generation(self):
        """Verify struct slice unpacking, zero-copy long array passing, and JNI_ABORT cleanup."""
        test_ir = {
            "classes": [
                {
                    "name": "HardwareTimeline",
                    "qualified_name": "filament::Pacer::HardwareTimeline",
                    "parent_class": "Pacer",
                    "is_nested": True,
                    "category": "struct",
                    "fields": [
                        {"name": "expectedPresentationTime", "type": {"cpp_name": "std::chrono::nanoseconds", "qualified_name": "std::chrono::nanoseconds", "category": "primitive"}},
                        {"name": "deadline", "type": {"cpp_name": "std::chrono::nanoseconds", "qualified_name": "std::chrono::nanoseconds", "category": "primitive"}}
                    ],
                    "methods": [],
                    "enums": [],
                    "constants": []
                },
                {
                    "name": "VsyncTick",
                    "qualified_name": "filament::Pacer::VsyncTick",
                    "parent_class": "Pacer",
                    "is_nested": True,
                    "category": "struct",
                    "fields": [
                        {"name": "baseTime", "type": {"cpp_name": "std::chrono::nanoseconds", "qualified_name": "std::chrono::nanoseconds", "category": "primitive"}},
                        {"name": "timelines", "type": {"cpp_name": "utils::Slice<const HardwareTimeline>", "qualified_name": "utils::Slice<const filament::Pacer::HardwareTimeline>", "category": "object"}}
                    ],
                    "methods": [],
                    "enums": [],
                    "constants": []
                },
                {
                    "name": "Pacer",
                    "qualified_name": "filament::Pacer",
                    "category": "class",
                    "methods": [
                        {
                            "name": "setupFrame",
                            "return_type": {"cpp_name": "int", "qualified_name": "int", "category": "primitive"},
                            "arguments": [
                                {
                                    "name": "tick",
                                    "type": {"cpp_name": "const VsyncTick &", "qualified_name": "const filament::Pacer::VsyncTick&", "category": "object"}
                                }
                            ]
                        }
                    ],
                    "fields": [],
                    "enums": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "Pacer.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "Pacer.java").read_text()
            cpp_src = (Path(tmp_dir) / "Pacer.cpp").read_text()

            # Verify VsyncTick has HardwareTimeline[] field and getter/setter
            self.assertIn("private @Nullable HardwareTimeline[] mTimelines;", java_src)
            self.assertIn("public void setTimelines(@Nullable HardwareTimeline[] timelines)", java_src)
            self.assertIn("public HardwareTimeline[] getTimelines()", java_src)

            # Verify JNI C++ code unpacks and cleans up slice elements
            self.assertIn("jlong* tickTimelinesElements = nullptr;", cpp_src)
            self.assertIn("tickTimelinesElements = env->GetLongArrayElements(tickTimelines, nullptr);", cpp_src)
            self.assertIn("auto* structs = reinterpret_cast<Pacer::HardwareTimeline*>(tickTimelinesElements);", cpp_src)
            self.assertIn("tick.timelines = { structs, (size_t) tickTimelinesCount };", cpp_src)
            self.assertIn("env->ReleaseLongArrayElements(tickTimelines, tickTimelinesElements, JNI_ABORT);", cpp_src)

    def test_valued_enums_end_to_end(self):
        """Verify that valued enums (negative values, bitflags, and aliases) generate valid switch from(int) and deduplicate cases."""
        json_file = self.test_dir / "19_valued_enums.json"
        self.assertTrue(json_file.exists(), "19_valued_enums.json fixture not found")

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "ValuedEnumsTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "ValuedEnumsTest.cpp").read_text()

            # 1. Verify FrameStatus has negative values and switch cases
            self.assertIn("SKIPPED_SPURIOUS(-2),", java_src)
            self.assertIn("SKIPPED_STALE(-1),", java_src)
            self.assertIn("ACCEPTED(0);", java_src)
            self.assertIn("case -2: return SKIPPED_SPURIOUS;", java_src)
            self.assertIn("case -1: return SKIPPED_STALE;", java_src)
            self.assertIn("case 0: return ACCEPTED;", java_src)

            # 2. Verify BitFlags has sparse positive values and switch cases
            self.assertIn("FLAG_A(1),", java_src)
            self.assertIn("FLAG_B(2),", java_src)
            self.assertIn("FLAG_C(4);", java_src)
            self.assertIn("case 4: return FLAG_C;", java_src)

            # 3. Verify AttachmentPoint aliases are deduplicated in from(int)
            self.assertIn("COLOR(0),", java_src)
            self.assertIn("COLOR0(0),", java_src)
            self.assertEqual(java_src.count("case 0: return COLOR;"), 1)
            self.assertNotIn("case 0: return COLOR0;", java_src)

            # 4. Verify JNI C++ casts int to enum type
            self.assertIn("(ValuedEnumsTest::FrameStatus)status", cpp_src)
            self.assertIn("(ValuedEnumsTest::BitFlags)flags", cpp_src)

    def test_buffer_descriptor_methods(self):
        """Verify BufferDescriptor generates the 3 Java overloads, JNI AutoBuffer handling, and imports."""
        json_data = {
            "meta": {"source_file": "filament/include/filament/BufferTest.h"},
            "classes": [
                {
                    "name": "BufferTest",
                    "qualified_name": "filament::BufferTest",
                    "doc": {"brief": "Buffer test class", "details": "", "params": {}},
                    "archetype": "class",
                    "methods": [
                        {
                            "name": "setBuffer",
                            "doc": {"brief": "Sets the buffer data", "details": "", "params": {}},
                            "return_type": {"cpp_name": "void", "qualified_name": "void"},
                            "arguments": [
                                {
                                    "name": "engine",
                                    "type": {"cpp_name": "Engine &", "qualified_name": "filament::Engine &", "is_reference": True}
                                },
                                {
                                    "name": "buffer",
                                    "type": {"cpp_name": "BufferDescriptor &&", "qualified_name": "filament::backend::BufferDescriptor &&"}
                                },
                                {
                                    "name": "byteOffset",
                                    "type": {"cpp_name": "uint32_t", "qualified_name": "uint32_t"}
                                }
                            ]
                        }
                    ]
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "BufferTest.json"
            json_file.write_text(json.dumps(json_data))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "BufferTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "BufferTest.cpp").read_text()

            # Verify Java imports
            self.assertIn("import java.nio.Buffer;", java_src)
            self.assertIn("import java.nio.BufferOverflowException;", java_src)
            self.assertIn("import androidx.annotation.IntRange;", java_src)
            self.assertIn("import androidx.annotation.NonNull;", java_src)
            self.assertIn("import androidx.annotation.Nullable;", java_src)

            # Verify Java 3 overloads
            self.assertIn("public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer)", java_src)
            self.assertIn("public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,\n            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count)", java_src)
            self.assertIn("public void setBuffer(@NonNull Engine engine, @NonNull Buffer buffer,\n            @IntRange(from = 0) int destOffsetInBytes, @IntRange(from = 0) int count,\n            @Nullable Object handler, @Nullable Runnable callback)", java_src)
            self.assertIn("throw new BufferOverflowException();", java_src)

            # Verify native declaration
            self.assertIn("private static native int nSetBuffer(long nativeBufferTest, long nativeEngine, Buffer buffer, int remaining, int destOffsetInBytes, int count, Object handler, Runnable callback);", java_src)

            # Verify C++ headers and JNI implementation
            self.assertIn("#include <backend/BufferDescriptor.h>", cpp_src)
            self.assertIn('#include "common/CallbackUtils.h"', cpp_src)
            self.assertIn('#include "common/NioUtils.h"', cpp_src)
            self.assertIn("AutoBuffer nioBuffer(env, buffer, count);", cpp_src)
            self.assertIn("backend::BufferDescriptor desc(data, sizeInBytes,", cpp_src)
            self.assertIn("that->setBuffer(*engine, std::move(desc), (uint32_t) destOffsetInBytes);", cpp_src)

    def test_viewport_aggregate_struct_generation(self):
        """Verify generation of Viewport aggregate struct with public primitive fields, constructors, and methods."""
        test_ir = {
            "classes": [
                {
                    "name": "Viewport",
                    "qualified_name": "filament::Viewport",
                    "doc": {"brief": "Viewport describes a view port in pixel coordinates"},
                    "type": {"cpp_name": "Viewport", "category": "struct"},
                    "bases": ["backend::Viewport"],
                    "is_aggregate": True,
                    "fields": [
                        {"name": "left", "type": {"cpp_name": "int32_t", "category": "primitive"}},
                        {"name": "bottom", "type": {"cpp_name": "int32_t", "category": "primitive"}},
                        {"name": "width", "type": {"cpp_name": "uint32_t", "category": "primitive"}},
                        {"name": "height", "type": {"cpp_name": "uint32_t", "category": "primitive"}}
                    ],
                    "methods": [
                        {
                            "name": "Viewport",
                            "is_constructor": True,
                            "arguments": [],
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "is_static": False,
                            "is_noexcept": True
                        },
                        {
                            "name": "Viewport",
                            "is_constructor": True,
                            "arguments": [
                                {"name": "left", "type": {"cpp_name": "int32_t", "category": "primitive"}},
                                {"name": "bottom", "type": {"cpp_name": "int32_t", "category": "primitive"}},
                                {"name": "width", "type": {"cpp_name": "uint32_t", "category": "primitive"}},
                                {"name": "height", "type": {"cpp_name": "uint32_t", "category": "primitive"}}
                            ],
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "is_static": False,
                            "is_noexcept": True
                        },
                        {
                            "name": "empty",
                            "arguments": [],
                            "return_type": {"cpp_name": "bool", "category": "primitive"},
                            "is_static": False,
                            "is_const": True,
                            "is_noexcept": True
                        },
                        {
                            "name": "right",
                            "arguments": [],
                            "return_type": {"cpp_name": "int32_t", "category": "primitive"},
                            "is_static": False,
                            "is_const": True,
                            "is_noexcept": True
                        }
                    ]
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            ir_path = Path(tmp_dir) / "viewport_ir.json"
            ir_path.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    "--class-name", "Viewport",
                    str(ir_path),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "Viewport.java").read_text()
            cpp_src = (Path(tmp_dir) / "Viewport.cpp").read_text()

            # Verify public fields
            self.assertIn("public int left;", java_src)
            self.assertIn("public int bottom;", java_src)
            self.assertIn("@IntRange(from = 0)\n    public int width;", java_src)
            self.assertIn("@IntRange(from = 0)\n    public int height;", java_src)

            # Verify constructor
            self.assertIn("public Viewport() {", java_src)
            self.assertIn("public Viewport(int left, int bottom, @IntRange(from = 0) int width, @IntRange(from = 0) int height) {", java_src)
            self.assertIn("this.left = left;", java_src)
            self.assertIn("this.width = width;", java_src)

            # Verify methods
            self.assertIn("public boolean empty() {", java_src)
            self.assertIn("return nEmpty(left, bottom, width, height);", java_src)
            self.assertIn("public int right() {", java_src)
            self.assertIn("return nRight(left, bottom, width, height);", java_src)

            # Verify native declarations
            self.assertIn("private static native boolean nEmpty(int left, int bottom, int width, int height);", java_src)
            self.assertIn("private static native int nRight(int left, int bottom, int width, int height);", java_src)

            # Verify JNI C++
            self.assertIn("Java_com_google_android_filament_Viewport_nEmpty(JNIEnv *env, jclass clazz, jint left, jint bottom, jint width, jint height)", cpp_src)
            self.assertIn("Viewport that;", cpp_src)
            self.assertIn("that.left = left;", cpp_src)
            self.assertIn("that.width = width;", cpp_src)
            self.assertIn("return (jboolean)that.empty();", cpp_src)

    def test_vertexbuffer_generation(self):
        """Verify that VertexBuffer generates properly with enum aliases and buffer descriptor methods."""
        header_path = Path(__file__).parent.parent.parent.parent / "filament" / "include" / "filament" / "VertexBuffer.h"
        if not header_path.exists():
            self.skipTest("VertexBuffer.h not found")

        extractor_py = Path(__file__).parent.parent / "extractor.py"
        javagen_py = Path(__file__).parent.parent / "javagen.py"
        repo_root = Path(__file__).parent.parent.parent.parent

        includes = [
            "-I" + str(repo_root / "filament" / "include"),
            "-I" + str(repo_root / "libs" / "utils" / "include"),
            "-I" + str(repo_root / "libs" / "math" / "include"),
            "-I" + str(repo_root / "libs" / "filabackend" / "include"),
            "-I" + str(repo_root / "libs" / "filabridge" / "include"),
            "-I" + str(repo_root / "out" / "android-release" / "filament" / "include"),
        ]

        with tempfile.TemporaryDirectory() as tmp_dir:
            ir_res = subprocess.run(
                [sys.executable, str(extractor_py)] + includes + [str(header_path)],
                capture_output=True,
                text=True,
                cwd=str(repo_root),
            )
            self.assertEqual(ir_res.returncode, 0, f"extractor failed: {ir_res.stderr}")

            ir_path = Path(tmp_dir) / "VertexBuffer.json"
            ir_path.write_text(ir_res.stdout)

            res = subprocess.run(
                [
                    sys.executable,
                    str(javagen_py),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    "--class-name", "VertexBuffer",
                    str(ir_path),
                ],
                capture_output=True,
                text=True,
                cwd=str(repo_root),
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "VertexBuffer.java").read_text()
            cpp_src = (Path(tmp_dir) / "VertexBuffer.cpp").read_text()

            # Verify VertexAttribute enum
            self.assertIn("public enum VertexAttribute {", java_src)
            self.assertIn("POSITION(0),", java_src)
            self.assertIn("TANGENTS(1),", java_src)
            self.assertIn("COLOR(2),", java_src)

            # Verify AttributeType enum
            self.assertIn("public enum AttributeType {", java_src)
            self.assertIn("FLOAT3,", java_src)
            self.assertIn("UBYTE4,", java_src)

            # Verify Builder methods
            self.assertIn("public Builder bufferCount(@IntRange(from = 0) int bufferCount)", java_src)
            self.assertIn("public Builder attribute(@NonNull VertexAttribute attribute, @IntRange(from = 0) int bufferIndex, @NonNull AttributeType attributeType", java_src)
            self.assertIn("public Builder normalized(@NonNull VertexAttribute attribute)", java_src)

            # Verify setBufferAt overloads
            self.assertIn("public void setBufferAt(@NonNull Engine engine, @IntRange(from = 0) int bufferIndex, @NonNull Buffer buffer)", java_src)
            self.assertIn("public void setBufferAt(@NonNull Engine engine, @IntRange(from = 0) int bufferIndex, @NonNull Buffer buffer,", java_src)

            # Verify JNI C++
            self.assertIn("Java_com_google_android_filament_VertexBuffer_nSetBufferAt", cpp_src)
            self.assertIn("AutoBuffer nioBuffer(env, buffer, count);", cpp_src)
            self.assertIn("that->setBufferAt(*engine, (uint8_t)bufferIndex, std::move(desc), (uint32_t) destOffsetInBytes);", cpp_src)

    def test_pixel_buffer_descriptor_skipped(self):
        """Verify that PixelBufferDescriptor is annotated with UTILS_NOAPIGEN and skipped by APIGen."""
        repo_root = Path(__file__).parent.parent.parent.parent.resolve()
        header_path = repo_root / "filament" / "backend" / "include" / "backend" / "PixelBufferDescriptor.h"
        extractor_py = repo_root / "tools" / "apigen" / "extractor.py"
        javagen_py = repo_root / "tools" / "apigen" / "javagen.py"

        includes = [
            "-I" + str(repo_root),
            "-I" + str(repo_root / "filament" / "include"),
            "-I" + str(repo_root / "libs" / "utils" / "include"),
            "-I" + str(repo_root / "libs" / "math" / "include"),
            "-I" + str(repo_root / "filament" / "backend" / "include"),
            "-I" + str(repo_root / "libs" / "filabridge" / "include"),
        ]

        with tempfile.TemporaryDirectory() as tmp_dir:
            ir_res = subprocess.run(
                [sys.executable, str(extractor_py)] + includes + [str(header_path)],
                capture_output=True,
                text=True,
                cwd=str(repo_root),
            )
            self.assertEqual(ir_res.returncode, 0, f"extractor failed: {ir_res.stderr}")

            ir_data = json.loads(ir_res.stdout)
            cls_ir = next(c for c in ir_data["classes"] if c["name"] == "PixelBufferDescriptor")
            self.assertTrue(any(a in ("filament:apigen:skip", "apigen:skip") for a in cls_ir.get("attributes", [])))

            ir_path = Path(tmp_dir) / "PixelBufferDescriptor.json"
            ir_path.write_text(ir_res.stdout)

            res = subprocess.run(
                [
                    sys.executable,
                    str(javagen_py),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    "--class-name", "PixelBufferDescriptor",
                    str(ir_path),
                ],
                capture_output=True,
                text=True,
                cwd=str(repo_root),
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            # PixelBufferDescriptor is annotated with UTILS_NOAPIGEN and should not be generated by javagen
            self.assertFalse((Path(tmp_dir) / "PixelBufferDescriptor.java").exists())
            self.assertFalse((Path(tmp_dir) / "PixelBufferDescriptor.cpp").exists())

    def test_alternate_name_annotation(self):
        """Verify UTILS_APIGEN_ALTERNATE_NAME properly renames keywords in Java/JNI while preserving C++ calls."""
        from tools.apigen.javagen import get_effective_method_name, ClassContext

        # 1. Test get_effective_method_name helper
        m_normal = {"name": "setWidth", "attributes": []}
        self.assertEqual(get_effective_method_name(m_normal), "setWidth")

        m_keyword_annotated = {"name": "package", "attributes": ["apigen:alternate_name:payload"]}
        self.assertEqual(get_effective_method_name(m_keyword_annotated), "payload")

        m_keyword_unannotated = {"name": "package", "attributes": []}
        with self.assertRaises(ValueError) as ctx:
            get_effective_method_name(m_keyword_unannotated)
        self.assertIn("reserved keyword", str(ctx.exception))

        # 2. Test generation with builder methods (package -> payload, import -> importTexture)
        target_ir = {
            "name": "TargetClass",
            "qualified_name": "filament::TargetClass",
            "category": "class",
            "archetype": "object",
            "fields": [],
            "methods": [],
            "enums": []
        }
        builder_ir = {
            "name": "Builder",
            "qualified_name": "filament::TargetClass::Builder",
            "category": "builder",
            "archetype": "builder",
            "fields": [],
            "enums": [],
            "methods": [
                {
                    "name": "package",
                    "attributes": ["apigen:alternate_name:payload"],
                    "return_type": {
                        "cpp_name": "Builder &",
                        "qualified_name": "filament::TargetClass::Builder&",
                        "category": "object"
                    },
                    "arguments": [
                        {
                            "name": "size",
                            "type": {"cpp_name": "size_t", "category": "primitive"}
                        }
                    ]
                },
                {
                    "name": "import",
                    "attributes": ["apigen:alternate_name:importTexture"],
                    "return_type": {
                        "cpp_name": "Builder &",
                        "qualified_name": "filament::TargetClass::Builder&",
                        "category": "object"
                    },
                    "arguments": [
                        {
                            "name": "id",
                            "type": {"cpp_name": "intptr_t", "category": "primitive"}
                        }
                    ]
                }
            ]
        }

        generator = ClassContext(target_ir, "filament/TargetClass.h", builder_ir=builder_ir)
        java_src = generator.generate_java()
        jni_src = generator.generate_jni()

        # Verify Java method signatures use alternate names
        self.assertIn("public Builder payload(@IntRange(from = 0) int size)", java_src)
        self.assertIn("nBuilderPayload(mNativeBuilder, size);", java_src)
        self.assertIn("public Builder importTexture(long id)", java_src)
        self.assertIn("nBuilderImportTexture(mNativeBuilder, id);", java_src)
        self.assertNotIn("public Builder package(", java_src)
        self.assertNotIn("public Builder import(", java_src)

        # Verify JNI native decls use alternate names
        self.assertIn("private static native void nBuilderPayload(long nativeBuilder, int size);", java_src)
        self.assertIn("private static native void nBuilderImportTexture(long nativeBuilder, long id);", java_src)

        # Verify JNI C++ bridge uses alternate names for export but original C++ names for dispatch
        self.assertIn("nBuilderPayload", jni_src)
        self.assertIn("builder->package(", jni_src)
        self.assertIn("nBuilderImportTexture", jni_src)
        self.assertIn("builder->import(", jni_src)

    def test_alternate_name_headers_extraction(self):
        """Verify extractor extracts UTILS_APIGEN_ALTERNATE_NAME from Material.h and Texture.h."""
        repo_root = Path(__file__).parent.parent.parent.parent.resolve()
        extractor_py = repo_root / "tools" / "apigen" / "extractor.py"
        includes = [
            "-I" + str(repo_root),
            "-I" + str(repo_root / "filament" / "include"),
            "-I" + str(repo_root / "libs" / "utils" / "include"),
            "-I" + str(repo_root / "libs" / "math" / "include"),
            "-I" + str(repo_root / "filament" / "backend" / "include"),
            "-I" + str(repo_root / "libs" / "filabridge" / "include"),
        ]

        # Check Material.h package -> payload
        mat_res = subprocess.run(
            [sys.executable, str(extractor_py)] + includes + [str(repo_root / "filament" / "include" / "filament" / "Material.h")],
            capture_output=True,
            text=True,
            cwd=str(repo_root),
        )
        self.assertEqual(mat_res.returncode, 0, f"extractor failed on Material.h: {mat_res.stderr}")
        mat_ir = json.loads(mat_res.stdout)
        mat_builder = next(c for c in mat_ir["classes"] if c["name"] == "Builder")
        package_m = next(m for m in mat_builder["methods"] if m["name"] == "package")
        self.assertTrue(any(a in ("filament:apigen:alternate_name:payload", "apigen:alternate_name:payload") for a in package_m.get("attributes", [])))

        # Check Texture.h import -> importTexture
        tex_res = subprocess.run(
            [sys.executable, str(extractor_py)] + includes + [str(repo_root / "filament" / "include" / "filament" / "Texture.h")],
            capture_output=True,
            text=True,
            cwd=str(repo_root),
        )
        self.assertEqual(tex_res.returncode, 0, f"extractor failed on Texture.h: {tex_res.stderr}")
        tex_ir = json.loads(tex_res.stdout)
        tex_builder = next(c for c in tex_ir["classes"] if c["name"] == "Builder")
        import_m = next(m for m in tex_builder["methods"] if m["name"] == "import")
        self.assertTrue(any(a in ("filament:apigen:alternate_name:importTexture", "apigen:alternate_name:importTexture") for a in import_m.get("attributes", [])))

    def test_pixel_buffer_descriptor_methods(self):
        """Verify PixelBufferDescriptor unrolling in Java and JNI C++ bridge generation."""
        repo_root = Path(__file__).parent.parent.parent.parent.resolve()
        extractor_py = repo_root / "tools" / "apigen" / "extractor.py"
        javagen_py = repo_root / "tools" / "apigen" / "javagen.py"
        includes = [
            "-I" + str(repo_root),
            "-I" + str(repo_root / "filament" / "include"),
            "-I" + str(repo_root / "libs" / "utils" / "include"),
            "-I" + str(repo_root / "libs" / "math" / "include"),
            "-I" + str(repo_root / "filament" / "backend" / "include"),
            "-I" + str(repo_root / "libs" / "filabridge" / "include"),
        ]

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_path = Path(tmp_dir) / "Renderer.json"
            res = subprocess.run(
                [sys.executable, str(extractor_py)] + includes + [str(repo_root / "filament" / "include" / "filament" / "Renderer.h")],
                capture_output=True,
                text=True,
                cwd=str(repo_root),
            )
            self.assertEqual(res.returncode, 0, f"extractor failed on Renderer.h: {res.stderr}")
            json_path.write_text(res.stdout)

            gen_res = subprocess.run(
                [
                    sys.executable,
                    str(javagen_py),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_path),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(gen_res.returncode, 0, f"javagen failed on Renderer.json: {gen_res.stderr}")

            java_src = (Path(tmp_dir) / "Renderer.java").read_text()
            cpp_src = (Path(tmp_dir) / "Renderer.cpp").read_text()

            # 1. Java import assertions
            self.assertIn("import java.nio.Buffer;", java_src)
            self.assertIn("import java.nio.BufferOverflowException;", java_src)
            self.assertIn("import java.nio.ReadOnlyBufferException;", java_src)

            # 2. Java method assertions
            self.assertIn("public void readPixels(@IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset, @IntRange(from = 0) int width, @IntRange(from = 0) int height, @NonNull PixelBufferDescriptor buffer) {", java_src)
            self.assertIn("public void readPixels(@NonNull RenderTarget renderTarget, @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset, @IntRange(from = 0) int width, @IntRange(from = 0) int height, @NonNull PixelBufferDescriptor buffer) {", java_src)
            self.assertIn("if (buffer.storage.isReadOnly()) {", java_src)
            self.assertIn("throw new ReadOnlyBufferException();", java_src)
            self.assertIn("buffer.left, buffer.top, type, buffer.alignment,", java_src)
            self.assertIn("stride, format,", java_src)
            self.assertIn("buffer.handler, buffer.callback);", java_src)

            # 3. Native declarations in Java
            self.assertIn("private static native void nReadPixels(long nativeRenderer, int xoffset, int yoffset, int width, int height, Buffer storage, int remaining, int left, int top, int type, int alignment, int stride, int format, Object handler, Runnable callback);", java_src)
            self.assertIn("private static native void nReadPixels(long nativeRenderer, long nativeRenderTarget, int xoffset, int yoffset, int width, int height, Buffer storage, int remaining, int left, int top, int type, int alignment, int stride, int format, Object handler, Runnable callback);", java_src)

            # 4. JNI C++ header inclusions
            self.assertIn("#include <backend/PixelBufferDescriptor.h>", cpp_src)
            self.assertIn('#include "common/CallbackUtils.h"', cpp_src)
            self.assertIn('#include "common/NioUtils.h"', cpp_src)

            # 5. Overloaded JNI C++ function signatures with mangling
            mangled_1 = "Java_com_google_android_filament_Renderer_nReadPixels__JIIIILjava_nio_Buffer_2IIIIIIILjava_lang_Object_2Ljava_lang_Runnable_2"
            mangled_2 = "Java_com_google_android_filament_Renderer_nReadPixels__JJIIIILjava_nio_Buffer_2IIIIIIILjava_lang_Object_2Ljava_lang_Runnable_2"
            self.assertIn(mangled_1, cpp_src)
            self.assertIn(mangled_2, cpp_src)

            # 6. JNI C++ body validation
            self.assertIn("AutoBuffer nioBuffer(env, storage, remaining);", cpp_src)
            self.assertIn("auto* bufferCallback = JniBufferCallback::make(nullptr, env, handler, callback, std::move(nioBuffer));", cpp_src)
            self.assertIn("that->readPixels((uint32_t)xoffset, (uint32_t)yoffset, (uint32_t)width, (uint32_t)height, std::move(desc));", cpp_src)
            self.assertIn("that->readPixels(renderTarget, (uint32_t)xoffset, (uint32_t)yoffset, (uint32_t)width, (uint32_t)height, std::move(desc));", cpp_src)

    def test_pixel_buffer_descriptor_fixture_unrolling(self):
        """Verify complete PixelBufferDescriptor unrolling across read and write methods, compressed handling, and JNI reconstruction."""
        json_file = self.test_dir / "20_pixel_buffer_descriptor.json"
        self.assertTrue(json_file.exists(), "20_pixel_buffer_descriptor.json fixture not found")

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "PixelBufferDescriptorTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "PixelBufferDescriptorTest.cpp").read_text()

            # 1. Imports
            self.assertIn("import java.nio.Buffer;", java_src)
            self.assertIn("import java.nio.ReadOnlyBufferException;", java_src)

            # 2. Unrolling in read method (readPixels) - includes ReadOnlyBufferException check
            self.assertIn("public void readPixels(@IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset, @IntRange(from = 0) int width, @IntRange(from = 0) int height, @NonNull PixelBufferDescriptor buffer) {", java_src)
            self.assertIn("if (buffer.storage.isReadOnly()) {", java_src)
            self.assertIn("throw new ReadOnlyBufferException();", java_src)

            # 3. Dynamic compressed vs uncompressed unrolling logic
            self.assertIn("int type = (buffer.type != null) ? buffer.type.ordinal() : 0;", java_src)
            self.assertIn("int format = (buffer.type == Texture.Type.COMPRESSED) ?", java_src)
            self.assertIn("(buffer.compressedFormat != null ? buffer.compressedFormat.ordinal() : 0) :", java_src)
            self.assertIn("(buffer.format != null ? buffer.format.ordinal() : 0);", java_src)
            self.assertIn("int stride = (buffer.type == Texture.Type.COMPRESSED) ? buffer.compressedSizeInBytes : buffer.stride;", java_src)

            # 4. Unrolling in write method (setImage) - MUST NOT have ReadOnlyBufferException check
            self.assertIn("public void setImage(@NonNull Engine engine, @IntRange(from = 0) int level, @IntRange(from = 0) int xoffset, @IntRange(from = 0) int yoffset, @IntRange(from = 0) int width, @IntRange(from = 0) int height, @NonNull PixelBufferDescriptor buffer) {", java_src)
            # Find the body of setImage and assert read-only check is absent
            set_image_body = java_src.split("public void setImage(")[1].split("public long getNativeObject()")[0]
            self.assertNotIn("ReadOnlyBufferException", set_image_body)
            self.assertIn("nSetImage(getNativeObject(), engine.getNativeObject(), level, xoffset, yoffset, width, height, buffer.storage, buffer.storage.remaining(),", set_image_body)

            # 5. Native declarations
            self.assertIn("private static native void nReadPixels(long nativePixelBufferDescriptorTest, int xoffset, int yoffset, int width, int height, Buffer storage, int remaining, int left, int top, int type, int alignment, int stride, int format, Object handler, Runnable callback);", java_src)
            self.assertIn("private static native void nReadPixels(long nativePixelBufferDescriptorTest, long nativeRenderTarget, int xoffset, int yoffset, int width, int height, Buffer storage, int remaining, int left, int top, int type, int alignment, int stride, int format, Object handler, Runnable callback);", java_src)
            self.assertIn("private static native void nSetImage(long nativePixelBufferDescriptorTest, long nativeEngine, int level, int xoffset, int yoffset, int width, int height, Buffer storage, int remaining, int left, int top, int type, int alignment, int stride, int format, Object handler, Runnable callback);", java_src)

            # 6. JNI C++ signature mangling (overloaded nReadPixels mangled, non-overloaded nSetImage unmangled)
            mangled_1 = "Java_com_google_android_filament_PixelBufferDescriptorTest_nReadPixels__JIIIILjava_nio_Buffer_2IIIIIIILjava_lang_Object_2Ljava_lang_Runnable_2"
            mangled_2 = "Java_com_google_android_filament_PixelBufferDescriptorTest_nReadPixels__JJIIIILjava_nio_Buffer_2IIIIIIILjava_lang_Object_2Ljava_lang_Runnable_2"
            unmangled = "Java_com_google_android_filament_PixelBufferDescriptorTest_nSetImage("
            self.assertIn(mangled_1, cpp_src)
            self.assertIn(mangled_2, cpp_src)
            self.assertIn(unmangled, cpp_src)

            # 7. JNI C++ unrolling and reconstruction
            self.assertIn("AutoBuffer nioBuffer(env, storage, remaining);", cpp_src)
            self.assertIn("auto* bufferCallback = JniBufferCallback::make(nullptr, env, handler, callback, std::move(nioBuffer));", cpp_src)
            self.assertIn("backend::PixelBufferDescriptor desc = (type == (jint) backend::PixelDataType::COMPRESSED) ?", cpp_src)
            self.assertIn("that->readPixels((uint32_t)xoffset, (uint32_t)yoffset, (uint32_t)width, (uint32_t)height, std::move(desc));", cpp_src)
            self.assertIn("that->readPixels(renderTarget, (uint32_t)xoffset, (uint32_t)yoffset, (uint32_t)width, (uint32_t)height, std::move(desc));", cpp_src)
            self.assertIn("that->setImage(*engine, (uint32_t)level, (uint32_t)xoffset, (uint32_t)yoffset, (uint32_t)width, (uint32_t)height, std::move(desc));", cpp_src)

    def test_external_aggregate_struct_unrolling(self):
        """Verify external aggregate structs (Viewport, Box) unroll scalar coordinates instead of calling getNativeObject."""
        json_file = self.test_dir / "21_external_aggregates.json"
        self.assertTrue(json_file.exists(), "21_external_aggregates.json fixture not found")

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "ExternalAggregatesTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "ExternalAggregatesTest.cpp").read_text()

            # 1. Java copyFrame unrolls Viewport coordinates instead of calling getNativeObject
            self.assertIn("public void copyFrame(@NonNull SwapChain dstSwapChain, @NonNull Viewport dstViewport, @NonNull Viewport srcViewport, @IntRange(from = 0) int flags) {", java_src)
            self.assertIn("nCopyFrame(getNativeObject(), dstSwapChain.getNativeObject(), dstViewport.getLeft(), dstViewport.getBottom(), dstViewport.getWidth(), dstViewport.getHeight(), srcViewport.getLeft(), srcViewport.getBottom(), srcViewport.getWidth(), srcViewport.getHeight(), flags);", java_src)
            self.assertNotIn("dstViewport.getNativeObject()", java_src)
            self.assertNotIn("srcViewport.getNativeObject()", java_src)

            # 2. Convenience overload with default flags=0
            self.assertIn("public void copyFrame(@NonNull SwapChain dstSwapChain, @NonNull Viewport dstViewport, @NonNull Viewport srcViewport) {", java_src)
            self.assertIn("copyFrame(dstSwapChain, dstViewport, srcViewport, 0);", java_src)

            # 3. Java intersects unrolls Box coordinates
            self.assertIn("public boolean intersects(@NonNull Box box) {", java_src)
            self.assertIn("nIntersects(getNativeObject(), box.getCenterX(), box.getCenterY(), box.getCenterZ(), box.getHalfExtentX(), box.getHalfExtentY(), box.getHalfExtentZ());", java_src)
            self.assertNotIn("box.getNativeObject()", java_src)

            # 4. Native declarations unroll to scalar primitives
            self.assertIn("private static native void nCopyFrame(long nativeExternalAggregatesTest, long nativeDstSwapChain, int dstViewportLeft, int dstViewportBottom, int dstViewportWidth, int dstViewportHeight, int srcViewportLeft, int srcViewportBottom, int srcViewportWidth, int srcViewportHeight, @IntRange(from = 0) int flags);", java_src)
            self.assertIn("private static native boolean nIntersects(long nativeExternalAggregatesTest, float boxCenterX, float boxCenterY, float boxCenterZ, float boxHalfExtentX, float boxHalfExtentY, float boxHalfExtentZ);", java_src)

            # 5. Header inclusions in JNI C++
            self.assertIn("#include <filament/Viewport.h>", cpp_src)
            self.assertIn("#include <filament/Box.h>", cpp_src)

            # 6. JNI C++ reconstruction on stack
            self.assertIn("Viewport dstViewport;", cpp_src)
            self.assertIn("dstViewport.left = dstViewportLeft;", cpp_src)
            self.assertIn("dstViewport.bottom = dstViewportBottom;", cpp_src)
            self.assertIn("dstViewport.width = dstViewportWidth;", cpp_src)
            self.assertIn("dstViewport.height = dstViewportHeight;", cpp_src)
            self.assertIn("that->copyFrame(dstSwapChain, dstViewport, srcViewport, (uint32_t)flags);", cpp_src)

            self.assertIn("Box box;", cpp_src)
            self.assertIn("box.center = { boxCenterX, boxCenterY, boxCenterZ };", cpp_src)
            self.assertIn("box.halfExtent = { boxHalfExtentX, boxHalfExtentY, boxHalfExtentZ };", cpp_src)
            self.assertIn("that->intersects(box)", cpp_src)

            # 7. Java Box out-parameter and convenience overload
            self.assertIn("public Box getBoundingBox(@Nullable Box out) {", java_src)
            self.assertIn("if (out == null) {", java_src)
            self.assertIn("out = new Box();", java_src)
            self.assertIn("nGetBoundingBox(getNativeObject(), out);", java_src)
            self.assertIn("return out;", java_src)
            self.assertIn("public Box getBoundingBox() {", java_src)
            self.assertIn("return getBoundingBox(null);", java_src)
            self.assertIn("private static native void nGetBoundingBox(long nativeExternalAggregatesTest, @NonNull Box out);", java_src)

            # 8. JNI C++ Box out-parameter setting
            self.assertIn("Java_com_google_android_filament_ExternalAggregatesTest_nGetBoundingBox(JNIEnv *env, jclass clazz, jlong nativeExternalAggregatesTest, jobject out_)", cpp_src)
            self.assertIn("static const struct JniBoxState {", cpp_src)
            self.assertIn('jclass clazz = env->FindClass("com/google/android/filament/Box");', cpp_src)
            self.assertIn('center_X = env->GetFieldID(clazz, "mCenterX", "F");', cpp_src)
            self.assertIn('halfExtent_Z = env->GetFieldID(clazz, "mHalfExtentZ", "F");', cpp_src)
            self.assertIn("auto const& res = that->getBoundingBox();", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.center_X, (jfloat) res.center.x);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.halfExtent_Z, (jfloat) res.halfExtent.z);", cpp_src)

    def test_struct_pointer_array_not_unrolled(self):
        """Verify struct pointer array parameters (e.g. const Bone* transforms) are bound as long[] instead of unrolled."""
        json_data = {
            "meta": {"generator_version": "1.0.0", "source_file": "TestSkinning.h"},
            "includes": {"user": [], "system": ["<filament/Engine.h>", "<filament/RenderableManager.h>"]},
            "aliases": [],
            "enums": [],
            "classes": [
                {
                    "name": "TestSkinning",
                    "qualified_name": "filament::TestSkinning",
                    "location": {"file": "TestSkinning.h", "line": 10},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "TestSkinning", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [
                        {
                            "name": "setBones",
                            "qualified_name": "setBones(Engine &, const Bone *, size_t, size_t)",
                            "location": {"file": "TestSkinning.h", "line": 15},
                            "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                            "attributes": [],
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"},
                            "arguments": [
                                {"name": "engine", "type": {"cpp_name": "Engine &", "qualified_name": "filament::Engine&", "category": "object", "is_const": False, "is_pointer": False, "is_reference": True, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "transforms", "type": {"cpp_name": "const Bone *", "qualified_name": "filament::RenderableManager::Bone*", "category": "object", "is_const": True, "is_pointer": True, "is_reference": False, "is_move_reference": False, "nullability": "nonnull"}, "default_value": None, "attributes": []},
                                {"name": "count", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "offset", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0", "attributes": []}
                            ],
                            "is_static": False,
                            "is_const": False,
                            "is_noexcept": False,
                            "template_parameters": [],
                            "constraint": None,
                            "specializations": []
                        }
                    ],
                    "fields": [],
                    "is_nested": False,
                    "is_aggregate": False
                }
            ],
            "referenced_classes": [
                {
                    "name": "Bone",
                    "qualified_name": "filament::RenderableManager::Bone",
                    "location": {"file": "filament/RenderableManager.h", "line": 139},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "Bone", "category": "struct", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [],
                    "fields": [
                        {"name": "unitQuaternion", "qualified_name": "unitQuaternion", "location": {"file": "filament/RenderableManager.h", "line": 140}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::quatf", "qualified_name": "filament::math::details::TQuaternion<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 1.f , 0.f , 0.f , 0.f }"},
                        {"name": "translation", "qualified_name": "translation", "location": {"file": "filament/RenderableManager.h", "line": 141}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::details::TVec3<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 0.f , 0.f , 0.f }"},
                        {"name": "reserved", "qualified_name": "reserved", "location": {"file": "filament/RenderableManager.h", "line": 142}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0"}
                    ],
                    "aliases": [],
                    "constants": [],
                    "parent_class": "RenderableManager",
                    "is_nested": True,
                    "is_aggregate": True
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "TestSkinning.json"
            json_file.write_text(json.dumps(json_data))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "TestSkinning.java").read_text()
            cpp_src = (Path(tmp_dir) / "TestSkinning.cpp").read_text()

            # Verify Java takes float[] transforms with @Size(min = 8) rather than unrolling into 8 float params or unsafe long[]
            self.assertIn("public void setBones(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("public void setBones(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("private static native void nSetBones(long nativeTestSkinning, long nativeEngine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset);", java_src)

            # Verify JNI C++ casts transforms array
            self.assertIn("jfloat const * const transforms = env->GetFloatArrayElements(transforms_, nullptr);", cpp_src)
            self.assertIn("that->setBones(*engine, (RenderableManager::Bone const *)transforms, (size_t)count, (size_t)offset);", cpp_src)

    def test_format_see_tag(self):
        from tools.apigen.javagen import format_see_tag
        
        # Method in current class (unqualified, empty parens, or arguments)
        self.assertEqual(format_see_tag("foo()"), "#foo")
        self.assertEqual(format_see_tag("foo"), "#foo")
        self.assertEqual(format_see_tag("#foo"), "#foo")
        self.assertEqual(format_see_tag("setProjection"), "#setProjection")
        self.assertEqual(format_see_tag("getIndirectLight"), "#getIndirectLight")
        self.assertEqual(format_see_tag("hasComponent()"), "#hasComponent")
        self.assertEqual(format_see_tag("foo(SwapChain, Viewport, Viewport)"), "#foo(SwapChain, Viewport, Viewport)")
        self.assertEqual(format_see_tag("#foo(SwapChain, Viewport, Viewport)"), "#foo(SwapChain, Viewport, Viewport)")
        self.assertEqual(format_see_tag("create(utils::Entity, Instance, const math::mat4&);"), "#create(int, int, float[])")
        self.assertEqual(format_see_tag("inverseProjection(const math::mat4&)"), "#inverseProjection(float[])")
        self.assertEqual(format_see_tag("irradiance(uint8_t bands, math::float3 const* sh)"), "#irradiance(int, float[])")
        
        # Scoped references / inner classes
        self.assertEqual(format_see_tag("Builder.direction()"), "Builder#direction")
        self.assertEqual(format_see_tag("Builder::direction()"), "Builder#direction")
        self.assertEqual(format_see_tag("LightManager::Builder::direction()"), "LightManager.Builder#direction")
        self.assertEqual(format_see_tag("RenderableManager:Builder:boneIndicesAndWeights"), "RenderableManager.Builder#boneIndicesAndWeights")
        self.assertEqual(format_see_tag("Builder.intensity(float watts, float efficiency)"), "Builder#intensity(float, float)")
        self.assertEqual(format_see_tag("Builder::irradiance(uint8_t, math::float3 const*)"), "Builder#irradiance(int, float[])")
        self.assertEqual(format_see_tag("Texture::isTextureSwizzleSupported()"), "Texture#isTextureSwizzleSupported")
        self.assertEqual(format_see_tag("Engine::Config::stereoscopicEyeCount"), "Engine.Config#stereoscopicEyeCount")
        
        # Top-level class / interface names
        self.assertEqual(format_see_tag("Scene"), "Scene")
        self.assertEqual(format_see_tag("Fov."), "Fov")
        
        # HTML links and strings
        self.assertEqual(format_see_tag('<a href="https://example.com">doc</a>'), '<a href="https://example.com">doc</a>')
        self.assertEqual(format_see_tag('"Specification Doc"'), '"Specification Doc"')

    def test_box_struct_output_parameter(self):
        """Verify methods returning Box struct generate zero-allocation out-param in Java and field-setting JNI C++."""
        json_data = {
            "meta": {"generator_version": "1.0.0", "source_file": "filament/RenderableManager.h"},
            "includes": {"user": [], "system": ["<filament/Engine.h>", "<filament/Box.h>"]},
            "aliases": [
                {"name": "Instance", "qualified_name": "filament::RenderableManager::Instance", "type": {"cpp_name": "utils::EntityInstance<RenderableManager>", "qualified_name": "filament::utils::EntityInstance<filament::RenderableManager>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False}}
            ],
            "enums": [],
            "classes": [
                {
                    "name": "RenderableManager",
                    "qualified_name": "filament::RenderableManager",
                    "location": {"file": "filament/RenderableManager.h", "line": 40},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "RenderableManager", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [
                        {
                            "name": "getAxisAlignedBoundingBox",
                            "qualified_name": "getAxisAlignedBoundingBox(Instance) const",
                            "location": {"file": "filament/RenderableManager.h", "line": 50},
                            "doc": {"brief": "Gets the axis-aligned bounding box.", "details": "", "params": {"i": "Instance of the renderable"}, "returns": "The bounding box", "meta": {}},
                            "attributes": [],
                            "return_type": {"cpp_name": "Box", "qualified_name": "filament::Box", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"},
                            "arguments": [
                                {"name": "i", "type": {"cpp_name": "Instance", "qualified_name": "filament::RenderableManager::Instance", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []}
                            ],
                            "is_static": False,
                            "is_const": True,
                            "is_noexcept": True,
                            "template_parameters": [],
                            "constraint": None,
                            "specializations": []
                        }
                    ],
                    "fields": [],
                    "is_nested": False,
                    "is_aggregate": False
                }
            ],
            "referenced_classes": [
                {
                    "name": "Box",
                    "qualified_name": "filament::Box",
                    "location": {"file": "filament/Box.h", "line": 30},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "Box", "category": "struct", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [],
                    "fields": [
                        {"name": "center", "qualified_name": "center", "location": {"file": "filament/Box.h", "line": 32}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::details::TVec3<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 0.f , 0.f , 0.f }"},
                        {"name": "halfExtent", "qualified_name": "halfExtent", "location": {"file": "filament/Box.h", "line": 33}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::details::TVec3<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 0.f , 0.f , 0.f }"}
                    ],
                    "aliases": [],
                    "constants": [],
                    "is_nested": False,
                    "is_aggregate": True
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "RenderableManager.json"
            json_file.write_text(json.dumps(json_data))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "RenderableManager.java").read_text()
            cpp_src = (Path(tmp_dir) / "RenderableManager.cpp").read_text()

            # Verify Java out-param method and convenience overload
            self.assertIn("public Box getAxisAlignedBoundingBox(@EntityInstance int i, @Nullable Box out) {", java_src)
            self.assertIn("if (out == null) {", java_src)
            self.assertIn("out = new Box();", java_src)
            self.assertIn("nGetAxisAlignedBoundingBox(getNativeObject(), i, out);", java_src)
            self.assertIn("return out;", java_src)
            self.assertIn("public Box getAxisAlignedBoundingBox(@EntityInstance int i) {", java_src)
            self.assertIn("return getAxisAlignedBoundingBox(i, null);", java_src)
            self.assertIn("private static native void nGetAxisAlignedBoundingBox(long nativeRenderableManager, @EntityInstance int i, @NonNull Box out);", java_src)

            # Verify JNI C++ implementation
            self.assertIn("Java_com_google_android_filament_RenderableManager_nGetAxisAlignedBoundingBox(JNIEnv *env, jclass clazz, jlong nativeRenderableManager, jint i, jobject out_)", cpp_src)
            self.assertIn("static const struct JniBoxState {", cpp_src)
            self.assertIn('jclass clazz = env->FindClass("com/google/android/filament/Box");', cpp_src)
            self.assertIn('center_X = env->GetFieldID(clazz, "mCenterX", "F");', cpp_src)
            self.assertIn('center_Y = env->GetFieldID(clazz, "mCenterY", "F");', cpp_src)
            self.assertIn('center_Z = env->GetFieldID(clazz, "mCenterZ", "F");', cpp_src)
            self.assertIn('halfExtent_X = env->GetFieldID(clazz, "mHalfExtentX", "F");', cpp_src)
            self.assertIn('halfExtent_Y = env->GetFieldID(clazz, "mHalfExtentY", "F");', cpp_src)
            self.assertIn('halfExtent_Z = env->GetFieldID(clazz, "mHalfExtentZ", "F");', cpp_src)
            self.assertIn("auto const& res = that->getAxisAlignedBoundingBox(EntityInstance<RenderableManager>(i));", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.center_X, (jfloat) res.center.x);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.center_Y, (jfloat) res.center.y);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.center_Z, (jfloat) res.center.z);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.halfExtent_X, (jfloat) res.halfExtent.x);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.halfExtent_Y, (jfloat) res.halfExtent.y);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.halfExtent_Z, (jfloat) res.halfExtent.z);", cpp_src)

    def test_shadow_options_nested_struct_output_parameter(self):
        """Verify methods returning nested aggregate structs (e.g. LightManager::ShadowOptions) generate zero-allocation out-param."""
        json_data = {
            "meta": {"generator_version": "1.0.0", "source_file": "filament/LightManager.h"},
            "includes": {"user": [], "system": ["<filament/Engine.h>"]},
            "aliases": [
                {"name": "Instance", "qualified_name": "filament::LightManager::Instance", "type": {"cpp_name": "utils::EntityInstance<LightManager>", "qualified_name": "filament::utils::EntityInstance<filament::LightManager>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False}}
            ],
            "enums": [],
            "classes": [
                {
                    "name": "LightManager",
                    "qualified_name": "filament::LightManager",
                    "location": {"file": "filament/LightManager.h", "line": 40},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "LightManager", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [
                        {
                            "name": "getShadowOptions",
                            "qualified_name": "getShadowOptions(Instance) const",
                            "location": {"file": "filament/LightManager.h", "line": 60},
                            "doc": {"brief": "Gets shadow options", "details": "", "params": {"i": "Instance"}, "returns": "Shadow options", "meta": {}},
                            "attributes": [],
                            "return_type": {"cpp_name": "const ShadowOptions &", "qualified_name": "const filament::LightManager::ShadowOptions&", "category": "object", "is_const": True, "is_pointer": False, "is_reference": True, "is_move_reference": False, "nullability": "unspecified"},
                            "arguments": [
                                {"name": "i", "type": {"cpp_name": "Instance", "qualified_name": "filament::LightManager::Instance", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []}
                            ],
                            "is_static": False,
                            "is_const": True,
                            "is_noexcept": True,
                            "template_parameters": [],
                            "constraint": None,
                            "specializations": []
                        }
                    ],
                    "fields": [],
                    "is_nested": False,
                    "is_aggregate": False
                }
            ],
            "referenced_classes": [
                {
                    "name": "ShadowOptions",
                    "qualified_name": "filament::LightManager::ShadowOptions",
                    "location": {"file": "filament/LightManager.h", "line": 70},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "ShadowOptions", "category": "struct", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [],
                    "fields": [
                        {"name": "mapSize", "qualified_name": "mapSize", "location": {"file": "filament/LightManager.h", "line": 71}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "uint32_t", "qualified_name": "uint32_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "1024"},
                        {"name": "screenSpaceContactShadows", "qualified_name": "screenSpaceContactShadows", "location": {"file": "filament/LightManager.h", "line": 72}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "bool", "qualified_name": "bool", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "false"},
                        {"name": "shadowBulgeRadius", "qualified_name": "shadowBulgeRadius", "location": {"file": "filament/LightManager.h", "line": 73}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0.02f"}
                    ],
                    "aliases": [],
                    "constants": [],
                    "parent_class": "LightManager",
                    "is_nested": True,
                    "is_aggregate": True
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "LightManager.json"
            json_file.write_text(json.dumps(json_data))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "LightManager.java").read_text()
            cpp_src = (Path(tmp_dir) / "LightManager.cpp").read_text()

            # Verify Java out-param method and convenience overload
            self.assertIn("public ShadowOptions getShadowOptions(@EntityInstance int i, @Nullable ShadowOptions out) {", java_src)
            self.assertIn("if (out == null) {", java_src)
            self.assertIn("out = new ShadowOptions();", java_src)
            self.assertIn("nGetShadowOptions(getNativeObject(), i, out);", java_src)
            self.assertIn("return out;", java_src)
            self.assertIn("public ShadowOptions getShadowOptions(@EntityInstance int i) {", java_src)
            self.assertIn("return getShadowOptions(i, null);", java_src)

            # Verify JNI C++ nested class path and field assignment
            self.assertIn('jclass clazz = env->FindClass("com/google/android/filament/ShadowOptions");', cpp_src)
            self.assertIn('mapSize = env->GetFieldID(clazz, "mapSize", "I");', cpp_src)
            self.assertIn('screenSpaceContactShadows = env->GetFieldID(clazz, "screenSpaceContactShadows", "Z");', cpp_src)
            self.assertIn('shadowBulgeRadius = env->GetFieldID(clazz, "shadowBulgeRadius", "F");', cpp_src)
            self.assertIn("env->SetIntField(out_, jniState.mapSize, static_cast<jint>(res.mapSize));", cpp_src)
            self.assertIn("env->SetBooleanField(out_, jniState.screenSpaceContactShadows, (jboolean) res.screenSpaceContactShadows);", cpp_src)
            self.assertIn("env->SetFloatField(out_, jniState.shadowBulgeRadius, (jfloat) res.shadowBulgeRadius);", cpp_src)

    def test_skinning_buffer_bone_and_matrix_bindings(self):
        json_data = {
            "classes": [
                {
                    "name": "SkinningBuffer",
                    "qualified_name": "filament::SkinningBuffer",
                    "location": {"file": "filament/SkinningBuffer.h", "line": 40},
                    "doc": {"brief": "SkinningBuffer", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "SkinningBuffer", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [
                        {
                            "name": "setBones",
                            "qualified_name": "setBones(Engine &, const Bone *, size_t, size_t)",
                            "location": {"file": "filament/SkinningBuffer.h", "line": 80},
                            "doc": {"brief": "Updates bone transforms", "details": "", "params": {"transforms": "pointer to bones"}, "returns": "", "meta": {}},
                            "attributes": ["filament:apigen:alternate_name:setBonesAsQuaternions"],
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"},
                            "arguments": [
                                {"name": "engine", "type": {"cpp_name": "Engine &", "qualified_name": "filament::Engine&", "category": "object", "is_const": False, "is_pointer": False, "is_reference": True, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "transforms", "type": {"cpp_name": "const Bone *", "qualified_name": "filament::RenderableManager::Bone*", "category": "object", "is_const": True, "is_pointer": True, "is_reference": False, "is_move_reference": False, "nullability": "nonnull"}, "default_value": None, "attributes": ["filament:apigen:size_param:count"]},
                                {"name": "count", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "offset", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0", "attributes": []}
                            ],
                            "is_static": False,
                            "is_const": False,
                            "is_noexcept": False,
                            "template_parameters": [],
                            "constraint": None,
                            "specializations": []
                        },
                        {
                            "name": "setBones",
                            "qualified_name": "setBones(Engine &, const math::mat4f *, size_t, size_t)",
                            "location": {"file": "filament/SkinningBuffer.h", "line": 95},
                            "doc": {"brief": "Updates bone transforms", "details": "", "params": {"transforms": "pointer to mat4f"}, "returns": "", "meta": {}},
                            "attributes": ["filament:apigen:alternate_name:setBonesAsMatrices"],
                            "return_type": {"cpp_name": "void", "qualified_name": "void", "category": "void", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"},
                            "arguments": [
                                {"name": "engine", "type": {"cpp_name": "Engine &", "qualified_name": "filament::Engine&", "category": "object", "is_const": False, "is_pointer": False, "is_reference": True, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "transforms", "type": {"cpp_name": "const math::mat4f *", "qualified_name": "filament::math::mat4f*", "category": "object", "is_const": True, "is_pointer": True, "is_reference": False, "is_move_reference": False, "nullability": "nonnull"}, "default_value": None, "attributes": ["filament:apigen:size_param:count"]},
                                {"name": "count", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": None, "attributes": []},
                                {"name": "offset", "type": {"cpp_name": "size_t", "qualified_name": "size_t", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0", "attributes": []}
                            ],
                            "is_static": False,
                            "is_const": False,
                            "is_noexcept": False,
                            "template_parameters": [],
                            "constraint": None,
                            "specializations": []
                        }
                    ],
                    "fields": [],
                    "is_nested": False,
                    "is_aggregate": False
                }
            ],
            "referenced_classes": [
                {
                    "name": "Bone",
                    "qualified_name": "filament::RenderableManager::Bone",
                    "location": {"file": "filament/RenderableManager.h", "line": 139},
                    "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}},
                    "attributes": [],
                    "type": {"cpp_name": "Bone", "category": "struct", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False},
                    "bases": [],
                    "methods": [],
                    "fields": [
                        {"name": "unitQuaternion", "qualified_name": "unitQuaternion", "location": {"file": "filament/RenderableManager.h", "line": 140}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::quatf", "qualified_name": "filament::math::details::TQuaternion<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 1.f , 0.f , 0.f , 0.f }"},
                        {"name": "translation", "qualified_name": "translation", "location": {"file": "filament/RenderableManager.h", "line": 141}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::details::TVec3<float>", "category": "object", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "{ 0.f , 0.f , 0.f }"},
                        {"name": "reserved", "qualified_name": "reserved", "location": {"file": "filament/RenderableManager.h", "line": 142}, "doc": {"brief": "", "details": "", "params": {}, "returns": "", "meta": {}}, "attributes": [], "type": {"cpp_name": "float", "qualified_name": "float", "category": "primitive", "is_const": False, "is_pointer": False, "is_reference": False, "is_move_reference": False, "nullability": "unspecified"}, "default_value": "0"}
                    ],
                    "aliases": [],
                    "constants": [],
                    "parent_class": "RenderableManager",
                    "is_nested": True,
                    "is_aggregate": True
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "SkinningBuffer.json"
            json_file.write_text(json.dumps(json_data))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "SkinningBuffer.java").read_text()
            cpp_src = (Path(tmp_dir) / "SkinningBuffer.cpp").read_text()

            # Verify Java imports
            self.assertIn("import java.nio.Buffer;", java_src)
            self.assertIn("import java.nio.BufferOverflowException;", java_src)

            # Verify quaternions overloads: Buffer and float[]
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull Buffer transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull Buffer transforms, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int arrayOffset, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("public void setBonesAsQuaternions(@NonNull Engine engine, @NonNull @Size(min = 8) float[] transforms) {", java_src)

            # Verify matrices overloads: Buffer and float[]
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull Buffer transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull Buffer transforms, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull @Size(min = 16) float[] transforms, @IntRange(from = 0) int arrayOffset, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull @Size(min = 16) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull @Size(min = 16) float[] transforms, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("public void setBonesAsMatrices(@NonNull Engine engine, @NonNull @Size(min = 16) float[] transforms) {", java_src)

            # Verify native private static declarations
            self.assertIn("private static native int nSetBonesAsQuaternions(long nativeSkinningBuffer, long nativeEngine, @NonNull Buffer transforms, int remaining, @IntRange(from = 0) int count, @IntRange(from = 0) int offset);", java_src)
            self.assertIn("private static native void nSetBonesAsQuaternions(long nativeSkinningBuffer, long nativeEngine, @NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset, @IntRange(from = 0) int arrayOffset);", java_src)
            self.assertIn("private static native int nSetBonesAsMatrices(long nativeSkinningBuffer, long nativeEngine, @NonNull Buffer transforms, int remaining, @IntRange(from = 0) int count, @IntRange(from = 0) int offset);", java_src)
            self.assertIn("private static native void nSetBonesAsMatrices(long nativeSkinningBuffer, long nativeEngine, @NonNull @Size(min = 16) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset, @IntRange(from = 0) int arrayOffset);", java_src)

            # Verify JNI C++ includes
            self.assertIn('#include "common/NioUtils.h"', cpp_src)
            self.assertIn("#include <filament/RenderableManager.h>", cpp_src)
            self.assertIn("#include <math/mat4.h>", cpp_src)

            # Verify JNI overloaded mangled function names
            self.assertIn("Java_com_google_android_filament_SkinningBuffer_nSetBonesAsQuaternions__JJLjava_nio_Buffer_2III", cpp_src)
            self.assertIn("Java_com_google_android_filament_SkinningBuffer_nSetBonesAsQuaternions__JJ_3FIII", cpp_src)
            self.assertIn("Java_com_google_android_filament_SkinningBuffer_nSetBonesAsMatrices__JJLjava_nio_Buffer_2III", cpp_src)
            self.assertIn("Java_com_google_android_filament_SkinningBuffer_nSetBonesAsMatrices__JJ_3FIII", cpp_src)

            # Verify AutoBuffer sizing and casts
            self.assertIn("AutoBuffer nioBuffer(env, transforms_, count * 8);", cpp_src)
            self.assertIn("that->setBones(*engine, static_cast<RenderableManager::Bone const *>(data), (size_t)count, (size_t)offset);", cpp_src)
            self.assertIn("AutoBuffer nioBuffer(env, transforms_, count * 16);", cpp_src)
            self.assertIn("that->setBones(*engine, static_cast<math::mat4f const *>(data), (size_t)count, (size_t)offset);", cpp_src)

            # Verify float[] marshalling
            self.assertIn("reinterpret_cast<RenderableManager::Bone const *>(transforms) + arrayOffset", cpp_src)
            self.assertIn("reinterpret_cast<math::mat4f const *>(transforms) + arrayOffset", cpp_src)

    def test_bone_buffer_bindings_fixture(self):
        """Verify 23_bone_buffer_bindings.json generates BoneBufferBindingsTest.java and BoneBufferBindingsTest.cpp matching golden fixtures."""
        json_file = self.test_dir / "23_bone_buffer_bindings.json"
        self.assertTrue(json_file.exists(), "23_bone_buffer_bindings.json fixture not found")

        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "BoneBufferBindingsTest.java").read_text()
            cpp_src = (Path(tmp_dir) / "BoneBufferBindingsTest.cpp").read_text()

            expected_java = (self.test_dir / "23_bone_buffer_bindings.java").read_text()
            expected_cpp = (self.test_dir / "23_bone_buffer_bindings.cpp").read_text()

            self.assertEqual(java_src, expected_java)
            self.assertEqual(cpp_src, expected_cpp)

    def test_renderable_manager_generator_features(self):
        """Verify RenderableManager special features: AttributeBitset, setGeometryAt convenience overloads, and Builder skinning dual overloads."""
        test_ir = {
            "classes": [
                {
                    "name": "RenderableManager",
                    "qualified_name": "filament::RenderableManager",
                    "parent_class": None,
                    "is_nested": False,
                    "category": "manager",
                    "methods": [
                        {
                            "name": "getEnabledAttributesAt",
                            "return_type": {
                                "cpp_name": "AttributeBitset",
                                "qualified_name": "utils::bitset<uint32_t>",
                                "category": "object"
                            },
                            "arguments": [
                                {
                                    "name": "instance",
                                    "type": {
                                        "cpp_name": "Instance",
                                        "qualified_name": "filament::RenderableManager::Instance",
                                        "category": "primitive"
                                    }
                                },
                                {
                                    "name": "primitiveIndex",
                                    "type": {
                                        "cpp_name": "size_t",
                                        "qualified_name": "size_t",
                                        "category": "primitive"
                                    }
                                }
                            ]
                        },
                        {
                            "name": "setGeometryAt",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "instance",
                                    "type": {"cpp_name": "Instance", "category": "primitive"}
                                },
                                {
                                    "name": "primitiveIndex",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "type",
                                    "type": {"cpp_name": "PrimitiveType", "category": "enum"}
                                },
                                {
                                    "name": "vertices",
                                    "type": {
                                        "cpp_name": "VertexBuffer*",
                                        "qualified_name": "filament::VertexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                },
                                {
                                    "name": "indices",
                                    "type": {
                                        "cpp_name": "IndexBuffer*",
                                        "qualified_name": "filament::IndexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                },
                                {
                                    "name": "offset",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "count",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                }
                            ]
                        },
                        {
                            "name": "setGeometryAt",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "instance",
                                    "type": {"cpp_name": "Instance", "category": "primitive"}
                                },
                                {
                                    "name": "primitiveIndex",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "type",
                                    "type": {"cpp_name": "PrimitiveType", "category": "enum"}
                                },
                                {
                                    "name": "vertices",
                                    "type": {
                                        "cpp_name": "VertexBuffer*",
                                        "qualified_name": "filament::VertexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                },
                                {
                                    "name": "offset",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "count",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                }
                            ]
                        },
                        {
                            "name": "setGeometryAt",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "instance",
                                    "type": {"cpp_name": "Instance", "category": "primitive"}
                                },
                                {
                                    "name": "primitiveIndex",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "type",
                                    "type": {"cpp_name": "PrimitiveType", "category": "enum"}
                                },
                                {
                                    "name": "vertices",
                                    "type": {
                                        "cpp_name": "VertexBuffer*",
                                        "qualified_name": "filament::VertexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                },
                                {
                                    "name": "indices",
                                    "type": {
                                        "cpp_name": "IndexBuffer*",
                                        "qualified_name": "filament::IndexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                }
                            ]
                        },
                        {
                            "name": "setGeometryAt",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "instance",
                                    "type": {"cpp_name": "Instance", "category": "primitive"}
                                },
                                {
                                    "name": "primitiveIndex",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "type",
                                    "type": {"cpp_name": "PrimitiveType", "category": "enum"}
                                },
                                {
                                    "name": "vertices",
                                    "type": {
                                        "cpp_name": "VertexBuffer*",
                                        "qualified_name": "filament::VertexBuffer*",
                                        "category": "object",
                                        "is_pointer": True
                                    }
                                }
                            ]
                        }
                    ],
                    "fields": [],
                    "constants": []
                },
                {
                    "name": "Builder",
                    "qualified_name": "filament::RenderableManager::Builder",
                    "parent_class": "RenderableManager",
                    "is_nested": True,
                    "is_builder": True,
                    "category": "builder",
                    "bases": ["BuilderBase<BuilderDetails>"],
                    "methods": [
                        {
                            "name": "Builder",
                            "is_constructor": True,
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "count",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                }
                            ]
                        },
                        {
                            "name": "skinning",
                            "return_type": {"cpp_name": "Builder &", "category": "object"},
                            "arguments": [
                                {
                                    "name": "boneCount",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "bones",
                                    "type": {
                                        "cpp_name": "Bone const*",
                                        "category": "pointer",
                                        "is_pointer": True,
                                        "is_const": True,
                                        "size_param": "boneCount"
                                    }
                                }
                            ]
                        },
                        {
                            "name": "skinningAsMatrices",
                            "return_type": {"cpp_name": "Builder &", "category": "object"},
                            "arguments": [
                                {
                                    "name": "boneCount",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "transforms",
                                    "type": {
                                        "cpp_name": "math::mat4f const*",
                                        "category": "pointer",
                                        "is_pointer": True,
                                        "is_const": True,
                                        "size_param": "boneCount"
                                    }
                                }
                            ]
                        },
                        {
                            "name": "build",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "engine",
                                    "type": {"cpp_name": "Engine &", "category": "object"}
                                },
                                {
                                    "name": "entity",
                                    "type": {"cpp_name": "Entity", "category": "object"}
                                }
                            ]
                        }
                    ],
                    "fields": [],
                    "constants": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "RenderableManager.json"
            json_file.write_text(json.dumps(test_ir))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "RenderableManager.java").read_text()
            cpp_src = (Path(tmp_dir) / "RenderableManager.cpp").read_text()

            # AttributeBitset to Set<VertexBuffer.VertexAttribute>
            self.assertIn("import java.util.Collections;", java_src)
            self.assertIn("import java.util.EnumSet;", java_src)
            self.assertIn("import java.util.Set;", java_src)
            self.assertIn("Set<VertexBuffer.VertexAttribute> getEnabledAttributesAt", java_src)
            self.assertIn("private static Set<VertexBuffer.VertexAttribute> getAttributes(int bitSet) {", java_src)
            self.assertIn(".getValue()", cpp_src)

            # setGeometryAt overloads
            self.assertIn("public void setGeometryAt(Instance instance, @IntRange(from = 0) int primitiveIndex, @NonNull PrimitiveType type, VertexBuffer vertices, IndexBuffer indices) {", java_src)
            self.assertIn("public void setGeometryAt(Instance instance, @IntRange(from = 0) int primitiveIndex, @NonNull PrimitiveType type, VertexBuffer vertices) {", java_src)
            self.assertIn("that->setGeometryAt((Instance)instance, (size_t)primitiveIndex, (PrimitiveType)type, vertices, indices);", cpp_src)
            self.assertIn("that->setGeometryAt((Instance)instance, (size_t)primitiveIndex, (PrimitiveType)type, vertices);", cpp_src)

            # Builder skinning dual overloads
            self.assertIn("public Builder skinning(@IntRange(from = 0) int boneCount, @NonNull Buffer bones) {", java_src)
            self.assertIn("public Builder skinningAsMatrices(@IntRange(from = 0) int boneCount, @NonNull Buffer transforms) {", java_src)
            self.assertIn("nBuilderSkinning(mNativeBuilder, boneCount, bones, bones.remaining());", java_src)
            self.assertIn("nBuilderSkinningAsMatrices(mNativeBuilder, boneCount, transforms, transforms.remaining());", java_src)

            # JNI AutoBuffer calls
            self.assertIn("AutoBuffer nioBuffer(env, bones_, boneCount * 8);", cpp_src)
            self.assertIn("builder->skinning((size_t)boneCount, reinterpret_cast<const RenderableManager::Bone *>(bones));", cpp_src)
            self.assertIn("AutoBuffer nioBuffer(env, transforms_, boneCount * 16);", cpp_src)
            self.assertIn("builder->skinningAsMatrices((size_t)boneCount, reinterpret_cast<const math::mat4f *>(transforms));", cpp_src)

    def test_javadoc_meta_note_warning_deprecated(self):
        """Verify that note, warning, and deprecated tags from meta are rendered in Javadoc."""
        doc = {
            "brief": "Controls frustum culling, true by default.",
            "details": "",
            "params": {},
            "returns": "",
            "meta": {
                "note": "Do not confuse frustum culling with backface culling. The latter is controlled via\nthe material.",
                "warning": "Experimental feature.",
                "deprecated": "Use cullingOptions instead."
            }
        }
        res = javagen.generate_javadoc(doc, indent_spaces=8)
        self.assertIn("Do not confuse frustum culling with backface culling.", res)
        self.assertIn("<p>Experimental feature.</p>", res)
        self.assertIn("@deprecated Use cullingOptions instead.", res)

    def test_template_trait_specializations(self):
        """Verify that template methods with specializations expand into type-safe Java and JNI overloads."""
        ir = {
            "classes": [
                {
                    "name": "ParameterContainer",
                    "cpp_name": "filament::ParameterContainer",
                    "archetype": "handle",
                    "methods": [
                        {
                            "name": "setParameter",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "name",
                                    "type": {"cpp_name": "const char *", "qualified_name": "const char *", "is_pointer": True, "category": "string"}
                                },
                                {
                                    "name": "value",
                                    "type": {"cpp_name": "T", "qualified_name": "type-parameter-0-0", "is_template_param": True, "template_param_name": "T"}
                                }
                            ],
                            "specializations": [
                                {"T": "float"},
                                {"T": "int32_t"},
                                {"T": "bool"},
                                {"T": "math::float3"}
                            ]
                        },
                        {
                            "name": "getConstant",
                            "return_type": {"cpp_name": "T", "qualified_name": "type-parameter-0-0", "is_template_param": True, "template_param_name": "T"},
                            "arguments": [
                                {
                                    "name": "name",
                                    "type": {"cpp_name": "const char *", "qualified_name": "const char *", "is_pointer": True, "category": "string"}
                                }
                            ],
                            "specializations": [
                                {"T": "float"},
                                {"T": "int32_t"},
                                {"T": "bool"}
                            ]
                        }
                    ],
                    "fields": [],
                    "enums": []
                },
                {
                    "name": "Builder",
                    "parent_class": "ParameterContainer",
                    "is_builder": True,
                    "archetype": "builder",
                    "category": "builder",
                    "methods": [
                        {
                            "name": "constant",
                            "return_type": {"cpp_name": "Builder &", "is_reference": True, "category": "record"},
                            "arguments": [
                                {
                                    "name": "name",
                                    "type": {"cpp_name": "const char *", "qualified_name": "const char *", "is_pointer": True, "category": "string"}
                                },
                                {
                                    "name": "value",
                                    "type": {"cpp_name": "T", "qualified_name": "type-parameter-0-0", "is_template_param": True, "template_param_name": "T"}
                                }
                            ],
                            "specializations": [
                                {"T": "float"},
                                {"T": "int32_t"},
                                {"T": "bool"}
                            ]
                        },
                        {
                            "name": "build",
                            "return_type": {"cpp_name": "ParameterContainer *", "is_pointer": True, "category": "record"},
                            "arguments": [
                                {
                                    "name": "engine",
                                    "type": {"cpp_name": "Engine &", "qualified_name": "filament::Engine &", "is_reference": True, "category": "record"}
                                }
                            ]
                        }
                    ],
                    "fields": [],
                    "enums": []
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            ir_path = Path(tmp_dir) / "ParameterContainer.json"
            ir_path.write_text(json.dumps(ir))

            javagen.process_file(str(ir_path), tmp_dir, tmp_dir, class_name="ParameterContainer")

            java_path = Path(tmp_dir) / "ParameterContainer.java"
            cpp_path = Path(tmp_dir) / "ParameterContainer.cpp"
            self.assertTrue(java_path.exists())
            self.assertTrue(cpp_path.exists())

            java_src = java_path.read_text()
            cpp_src = cpp_path.read_text()

            # Verify Java setter overloads (scalars and vector unrolled + array)
            self.assertIn("public void setParameter(@NonNull String name, float value)", java_src)
            self.assertIn("public void setParameter(@NonNull String name, int value)", java_src)
            self.assertIn("public void setParameter(@NonNull String name, boolean value)", java_src)
            self.assertIn("public void setParameter(@NonNull String name, float valuex, float valuey, float valuez)", java_src)
            self.assertIn("public void setParameter(@NonNull String name, @NonNull @Size(min = 3) float[] value)", java_src)

            # Verify Java getter methods with unmangled return types
            self.assertIn("public float getConstantFloat(@NonNull String name)", java_src)
            self.assertIn("public int getConstantInt(@NonNull String name)", java_src)
            self.assertIn("public boolean getConstantBoolean(@NonNull String name)", java_src)

            # Verify Builder template methods in Java
            self.assertIn("public Builder constant(@NonNull String name, float value)", java_src)
            self.assertIn("public Builder constant(@NonNull String name, int value)", java_src)
            self.assertIn("public Builder constant(@NonNull String name, boolean value)", java_src)

            # Verify JNI C++ dispatch with explicit template syntax
            self.assertIn("that->setParameter<float>(name, value);", cpp_src)
            self.assertIn("that->setParameter<int32_t>(name, (int32_t)value);", cpp_src)
            self.assertIn("that->setParameter<bool>(name, (bool)value);", cpp_src)
            self.assertIn("that->setParameter<math::float3>(name, float3{valuex, valuey, valuez});", cpp_src)

            self.assertIn("return (jfloat)that->getConstant<float>(name);", cpp_src)
            self.assertIn("return (jint)that->getConstant<int32_t>(name);", cpp_src)
            self.assertIn("return (jboolean)that->getConstant<bool>(name);", cpp_src)

            # Verify Builder JNI C++ dispatch with explicit template syntax
            self.assertIn("builder->constant<float>(name, value);", cpp_src)
            self.assertIn("builder->constant<int32_t>(name, (int32_t)value);", cpp_src)
            self.assertIn("builder->constant<bool>(name, (bool)value);", cpp_src)

    def test_tagged_array_buffers(self):
        """Verify that tagged array buffer methods collapse into type-safe Java and JNI methods."""
        json_path = self.test_dir / "25_tagged_array_buffers.json"
        if not json_path.exists():
            return
        with open(json_path) as f:
            ir = json.load(f)

        cls_ir = ir["classes"][0]
        ctx = javagen.ClassContext(cls_ir, ir["meta"]["source_file"])
        java_src = ctx.generate_java()
        cpp_src = ctx.generate_jni()

        # 1. Verify synthesized Element enums and entries
        self.assertIn("public enum FloatElement {", java_src)
        self.assertIn("FLOAT,", java_src)
        self.assertIn("FLOAT2,", java_src)
        self.assertIn("FLOAT4,", java_src)
        self.assertIn("MAT4;", java_src)

        self.assertIn("public enum IntElement {", java_src)
        self.assertIn("INT,", java_src)
        self.assertIn("INT4;", java_src)

        self.assertIn("public enum BooleanElement {", java_src)
        self.assertIn("BOOL,", java_src)
        self.assertIn("BOOL4;", java_src)

        # 2. Verify Java primary and convenience methods
        self.assertIn("public void setBuffer(@NonNull String name, @NonNull FloatElement type, @NonNull float[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count)", java_src)
        self.assertIn("public void setBuffer(@NonNull String name, @NonNull FloatElement type, @NonNull float[] values, @IntRange(from = 1) int count)", java_src)
        self.assertIn("setBuffer(name, type, values, 0, count);", java_src)

        self.assertIn("public void setBuffer(@NonNull String name, @NonNull IntElement type, @NonNull int[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count)", java_src)
        self.assertIn("public void setBuffer(@NonNull String name, @NonNull IntElement type, @NonNull int[] values, @IntRange(from = 1) int count)", java_src)

        self.assertIn("public void setBuffer(@NonNull String name, @NonNull BooleanElement type, @NonNull boolean[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count)", java_src)
        self.assertIn("public void setBuffer(@NonNull String name, @NonNull BooleanElement type, @NonNull boolean[] values, @IntRange(from = 1) int count)", java_src)

        # 3. Verify native declarations
        self.assertIn("private static native void nSetBufferFloatArray(long nativeTaggedArrayBuffersTest, @NonNull String name, int type, @NonNull @Size(min = 1) float[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count);", java_src)
        self.assertIn("private static native void nSetBufferIntArray(long nativeTaggedArrayBuffersTest, @NonNull String name, int type, @NonNull @Size(min = 1) int[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count);", java_src)
        self.assertIn("private static native void nSetBufferBooleanArray(long nativeTaggedArrayBuffersTest, @NonNull String name, int type, @NonNull @Size(min = 1) boolean[] values, @IntRange(from = 0) int offset, @IntRange(from = 1) int count);", java_src)

        # 4. Verify JNI C++ methods and switch dispatch
        self.assertIn("Java_com_google_android_filament_TaggedArrayBuffersTest_nSetBufferFloatArray", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const float>(((const float*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const math::float2>(((const math::float2*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const math::float4>(((const math::float4*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const math::mat4f>(((const math::mat4f*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("env->ReleaseFloatArrayElements(values_, values, JNI_ABORT);", cpp_src)
        self.assertIn("env->ReleaseStringUTFChars(name_, name);", cpp_src)

        self.assertIn("Java_com_google_android_filament_TaggedArrayBuffersTest_nSetBufferIntArray", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const int32_t>(((const int32_t*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const math::int4>(((const math::int4*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("env->ReleaseIntArrayElements(values_, values, JNI_ABORT);", cpp_src)

        self.assertIn("Java_com_google_android_filament_TaggedArrayBuffersTest_nSetBufferBooleanArray", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const bool>(((const bool*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("that->setBuffer(name, utils::Slice<const math::bool4>(((const math::bool4*) values) + offset, (size_t)count));", cpp_src)
        self.assertIn("env->ReleaseBooleanArrayElements(values_, values, JNI_ABORT);", cpp_src)

    def test_material_generation(self):
        """Verify that Material generates expected builder, parameter inspection, and JNI wrapper code."""
        java_path = Path(__file__).parent.parent.parent.parent / "android" / "filament-android" / "src" / "main" / "java" / "com" / "google" / "android" / "filament" / "Material.java"
        cpp_path = Path(__file__).parent.parent.parent.parent / "android" / "filament-android" / "src" / "main" / "cpp" / "Material.cpp"
        if not java_path.exists() or not cpp_path.exists():
            return

        java_src = java_path.read_text()
        cpp_src = cpp_path.read_text()

        # 1. Verify Material constructor and default instance caching
        self.assertIn("private final MaterialInstance mDefaultInstance;", java_src)
        self.assertIn("mDefaultInstance = new MaterialInstance(nGetDefaultInstance(nativeObject), this);", java_src)
        self.assertIn("public MaterialInstance getDefaultInstance() {\n        return mDefaultInstance;\n    }", java_src)

        # 2. Verify createInstance associates instance with this Material
        self.assertIn("return new MaterialInstance(nativeMaterialInstance, this);", java_src)

        # 3. Verify Builder buffer retention and payload methods
        self.assertIn("private final java.util.List<Object> mRetainedBuffers = new java.util.ArrayList<>();", java_src)
        self.assertIn("public Builder payload(@NonNull Buffer payload, @IntRange(from = 0) int size) {", java_src)
        self.assertIn("mRetainedBuffers.add(payload);", java_src)
        self.assertIn("public Builder payload(@NonNull Buffer payload) {", java_src)
        self.assertIn("return payload(payload, payload.remaining());", java_src)

        # 4. Verify ParameterInfo class and getParameters
        self.assertIn("public static class ParameterInfo {", java_src)
        self.assertIn("public String name;", java_src)
        self.assertIn("public boolean isSampler;", java_src)
        self.assertIn("public boolean isSubpass;", java_src)
        self.assertIn("public ParameterType type;", java_src)
        self.assertIn("public SamplerType samplerType;", java_src)
        self.assertIn("public SubpassType subpassType;", java_src)
        self.assertIn("public ParameterInfo[] getParameters() {", java_src)
        self.assertIn("public int getParameters(@NonNull ParameterInfo[] parameters, @IntRange(from = 0) int count) {", java_src)

        # 5. Verify MaterialBuilderWrapper in C++ JNI
        self.assertIn("struct MaterialBuilderWrapper {", cpp_src)
        self.assertIn("Material::Builder builder;", cpp_src)
        self.assertIn("std::vector<std::unique_ptr<AutoBuffer>> retainedBuffers;", cpp_src)
        self.assertIn("Java_com_google_android_filament_Material_nCreateBuilder", cpp_src)
        self.assertIn("return (jlong) new MaterialBuilderWrapper{Material::Builder{}};", cpp_src)
        self.assertIn("Java_com_google_android_filament_Material_nDestroyBuilder", cpp_src)
        self.assertIn("delete wrapper;", cpp_src)
        self.assertIn("Java_com_google_android_filament_Material_nBuilderPayload", cpp_src)
        self.assertIn("wrapper->retainedBuffers.push_back(std::make_unique<AutoBuffer>(env, payload, size));", cpp_src)

        # 6. Verify JNI nGetParameters implementation
        self.assertIn("Java_com_google_android_filament_Material_nGetParameters", cpp_src)
        self.assertIn("env->FindClass(\"com/google/android/filament/Material$ParameterInfo\")", cpp_src)
        self.assertIn("env->SetObjectField(elem, samplerTypeField, sTypeObj);", cpp_src)
        self.assertIn("env->SetObjectField(elem, subpassTypeField, spTypeObj);", cpp_src)
        self.assertIn("env->SetObjectField(elem, typeField, pTypeObj);", cpp_src)

        # 7. Verify getters are properly exposed in Java
        self.assertIn("public boolean hasShadowMultiplier() {", java_src)
        self.assertIn("public boolean hasSpecularAntiAliasing() {", java_src)
        self.assertIn("public boolean isSampler(@NonNull String name) {", java_src)
        self.assertIn("public String getSource() {", java_src)
        self.assertIn("public int getSupportedVariants() {", java_src)
        self.assertIn("public MaterialDomain getMaterialDomain() {", java_src)

    def test_packed_buffer_windowed_array_overloads(self):
        """Verify systematic windowed array, full array, and direct Buffer overloads for packed buffer methods."""
        test_ir = {
            "classes": [
                {
                    "name": "PoseManager",
                    "qualified_name": "filament::PoseManager",
                    "parent_class": None,
                    "is_nested": False,
                    "category": "manager",
                    "methods": [
                        {
                            "name": "setBones",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "transforms",
                                    "type": {
                                        "cpp_name": "Bone const*",
                                        "category": "pointer",
                                        "is_pointer": True,
                                        "is_const": True,
                                        "size_param": "count"
                                    }
                                },
                                {
                                    "name": "count",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "offset",
                                    "type": {"cpp_name": "size_t", "category": "primitive"},
                                    "default_value": "0"
                                }
                            ]
                        },
                        {
                            "name": "applyPose",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {
                                    "name": "transforms",
                                    "type": {
                                        "cpp_name": "Bone const*",
                                        "category": "pointer",
                                        "is_pointer": True,
                                        "is_const": True,
                                        "size_param": "count"
                                    }
                                },
                                {
                                    "name": "count",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                }
                            ]
                        }
                    ]
                },
                {
                    "name": "Builder",
                    "qualified_name": "filament::PoseManager::Builder",
                    "parent_class": "PoseManager",
                    "is_nested": True,
                    "is_builder": True,
                    "category": "builder",
                    "methods": [
                        {
                            "name": "skinning",
                            "return_type": {"cpp_name": "Builder &", "category": "object"},
                            "arguments": [
                                {
                                    "name": "boneCount",
                                    "type": {"cpp_name": "size_t", "category": "primitive"}
                                },
                                {
                                    "name": "bones",
                                    "type": {
                                        "cpp_name": "Bone const*",
                                        "category": "pointer",
                                        "is_pointer": True,
                                        "is_const": True,
                                        "size_param": "boneCount"
                                    }
                                }
                            ]
                        }
                    ]
                }
            ]
        }

        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "PoseManager.json"
            json_file.write_text(json.dumps(test_ir))
            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "PoseManager.java").read_text()
            cpp_src = (Path(tmp_dir) / "PoseManager.cpp").read_text()

            # 1. Method with existing offset (setBones) -> source array offset named arrayOffset
            self.assertIn("public void setBones(@NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int arrayOffset, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("if (transforms.length < (arrayOffset + count) * 8) {", java_src)
            self.assertIn("throw new ArrayIndexOutOfBoundsException(", java_src)
            self.assertIn("nSetBones(getNativeObject(), transforms, count, offset, arrayOffset);", java_src)

            # Convenience overloads for setBones
            self.assertIn("public void setBones(@NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("setBones(transforms, 0, count, offset);", java_src)
            self.assertIn("public void setBones(@NonNull @Size(min = 8) float[] transforms) {", java_src)
            self.assertIn("setBones(transforms, 0, transforms.length / 8, 0);", java_src)

            # Direct Buffer overloads for setBones
            self.assertIn("public void setBones(@NonNull Buffer transforms, @IntRange(from = 0) int count, @IntRange(from = 0) int offset) {", java_src)
            self.assertIn("int result = nSetBones(getNativeObject(), transforms, transforms.remaining(), count, offset);", java_src)
            self.assertIn("if (result < 0) {", java_src)
            self.assertIn("throw new BufferOverflowException();", java_src)
            self.assertIn("public void setBones(@NonNull Buffer transforms, @IntRange(from = 0) int count) {", java_src)

            # Javadoc structured element count
            self.assertIn("@param arrayOffset offset in elements (structured element count) in <code>transforms</code> to skip", java_src)

            # JNI for setBones array
            self.assertIn("reinterpret_cast<RenderableManager::Bone const *>(transforms) + arrayOffset", cpp_src)

            # 2. Method without existing offset (applyPose) -> source array offset named offset
            self.assertIn("public void applyPose(@NonNull @Size(min = 8) float[] transforms, @IntRange(from = 0) int offset, @IntRange(from = 0) int count) {", java_src)
            self.assertIn("if (transforms.length < (offset + count) * 8) {", java_src)
            self.assertIn("nApplyPose(getNativeObject(), transforms, count, offset);", java_src)
            self.assertIn("public void applyPose(@NonNull @Size(min = 8) float[] transforms) {", java_src)
            self.assertIn("applyPose(transforms, 0, transforms.length / 8);", java_src)
            self.assertIn("public void applyPose(@IntRange(from = 0) int count, @NonNull Buffer transforms) {", java_src)
            self.assertIn("@param offset offset in elements (structured element count) in <code>transforms</code> to skip", java_src)
            self.assertIn("reinterpret_cast<RenderableManager::Bone const *>(transforms) + offset", cpp_src)

            # 3. Builder method (skinning)
            self.assertIn("public Builder skinning(@NonNull @Size(min = 8) float[] bones, @IntRange(from = 0) int offset, @IntRange(from = 0) int boneCount) {", java_src)
            self.assertIn("if (bones.length < (offset + boneCount) * 8) {", java_src)
            self.assertIn("public Builder skinning(@NonNull @Size(min = 8) float[] bones, @IntRange(from = 0) int boneCount) {", java_src)
            self.assertIn("return skinning(bones, 0, boneCount);", java_src)
            self.assertIn("public Builder skinning(@NonNull @Size(min = 8) float[] bones) {", java_src)
            self.assertIn("return skinning(bones, 0, bones.length / 8);", java_src)
            self.assertIn("public Builder skinning(@IntRange(from = 0) int boneCount, @NonNull Buffer bones) {", java_src)
            self.assertIn("builder->skinning((size_t)boneCount, reinterpret_cast<const RenderableManager::Bone *>(bones) + offset);", cpp_src)

    def test_builder_convenience_overloads_indentation(self):
        """Verify Builder convenience overloads have consistent 8-space indentation for Javadoc, annotations, signatures, and braces."""
        test_ir = {
            "classes": [
                {
                    "name": "FooManager",
                    "qualified_name": "filament::FooManager",
                    "parent_class": None,
                    "is_nested": False,
                    "category": "manager",
                    "methods": []
                },
                {
                    "name": "Builder",
                    "qualified_name": "filament::FooManager::Builder",
                    "parent_class": "FooManager",
                    "is_nested": True,
                    "is_builder": True,
                    "category": "builder",
                    "bases": ["BuilderBase<BuilderDetails>"],
                    "methods": [
                        {
                            "name": "Builder",
                            "is_constructor": True,
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": []
                        },
                        {
                            "name": "lightChannel",
                            "return_type": {
                                "cpp_name": "Builder &",
                                "qualified_name": "filament::FooManager::Builder&",
                                "category": "object",
                                "is_reference": True
                            },
                            "doc": {
                                "brief": "Enables or disables a light channel.",
                                "params": {
                                    "channel": "Light channel",
                                    "enable": "Whether to enable"
                                }
                            },
                            "arguments": [
                                {
                                    "name": "channel",
                                    "type": {"cpp_name": "int", "category": "primitive"}
                                },
                                {
                                    "name": "enable",
                                    "type": {"cpp_name": "bool", "category": "primitive"},
                                    "default_value": "true"
                                }
                            ]
                        },
                        {
                            "name": "build",
                            "return_type": {"cpp_name": "void", "category": "void"},
                            "arguments": [
                                {"name": "engine", "type": {"cpp_name": "Engine &", "category": "object"}},
                                {"name": "entity", "type": {"cpp_name": "Entity", "category": "primitive"}}
                            ]
                        }
                    ]
                }
            ]
        }
        with tempfile.TemporaryDirectory() as tmp_dir:
            json_file = Path(tmp_dir) / "FooManager.json"
            json_file.write_text(json.dumps(test_ir))

            res = subprocess.run(
                [
                    sys.executable,
                    str(Path(__file__).parent.parent / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(json_file),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            java_src = (Path(tmp_dir) / "FooManager.java").read_text()
            expected_snippet = (
                "        /**\n"
                "         * Enables or disables a light channel.\n"
                "         *\n"
                "         * @param channel Light channel\n"
                "         */\n"
                "        @NonNull\n"
                "        public Builder lightChannel(int channel) {\n"
                "            return lightChannel(channel, true);\n"
                "        }"
            )
            self.assertIn(expected_snippet, java_src)

    def test_retained_references_annotation(self):
        """Verify that methods annotated with apigen:retained generate retained fields, canonical constructors, and pure Java getters without JNI bridge."""
        from tools.apigen.javagen import JavaEmitter, JniEmitter, ClassContext

        # Test with a handle class having a retained Engine and a retained void* surface
        test_ir = {
            "name": "CustomView",
            "qualified_name": "filament::CustomView",
            "category": "class",
            "archetype": "handle",
            "fields": [],
            "enums": [],
            "methods": [
                {
                    "name": "getEngine",
                    "qualified_name": "getEngine()",
                    "attributes": ["filament:apigen:retained"],
                    "return_type": {
                        "cpp_name": "Engine *",
                        "qualified_name": "filament::Engine *",
                        "category": "pointer",
                        "is_pointer": True,
                        "nullability": "nonnull",
                    },
                    "arguments": [],
                    "doc": {"brief": "Returns associated Engine."}
                },
                {
                    "name": "getNativeWindow",
                    "qualified_name": "getNativeWindow()",
                    "attributes": ["apigen:retained"],
                    "return_type": {
                        "cpp_name": "void *",
                        "qualified_name": "void *",
                        "category": "pointer",
                        "is_pointer": True,
                        "nullability": "nonnull",
                    },
                    "arguments": [],
                    "doc": {"brief": "Returns native window."}
                },
                {
                    "name": "render",
                    "qualified_name": "render()",
                    "attributes": [],
                    "return_type": {
                        "cpp_name": "void",
                        "category": "void",
                    },
                    "arguments": [],
                    "doc": {"brief": "Renders view."}
                }
            ]
        }

        ctx = ClassContext(test_ir, "filament/CustomView.h")
        self.assertIn("getEngine", ctx.retained_references)
        self.assertIn("getNativeWindow", ctx.retained_references)
        self.assertEqual(ctx.retained_references["getEngine"]["java_type"], "Engine")
        self.assertEqual(ctx.retained_references["getNativeWindow"]["java_type"], "Object")
        self.assertEqual(ctx.retained_references["getNativeWindow"]["field_name"], "mNativeWindow")
        self.assertEqual(ctx.retained_references["getNativeWindow"]["param_name"], "nativeWindow")

        java_emitter = JavaEmitter(ctx)
        java_src = java_emitter.generate_java()

        # 1. Field emission
        self.assertIn("private final Engine mEngine;", java_src)
        self.assertIn("private final Object mNativeWindow;", java_src)

        # 2. Canonical constructor parameter ordering: (long nativeObject, <retained_args>...)
        self.assertIn("CustomView(long nativeObject, @NonNull Engine engine, @NonNull Object nativeWindow) {", java_src)
        self.assertIn("mNativeObject = nativeObject;", java_src)
        self.assertIn("mEngine = engine;", java_src)
        self.assertIn("mNativeWindow = nativeWindow;", java_src)

        # 3. Pure Java getters
        self.assertIn("@NonNull\n    public Engine getEngine() {\n        return mEngine;\n    }", java_src)
        self.assertIn("@NonNull\n    public Object getNativeWindow() {\n        return mNativeWindow;\n    }", java_src)

        # 4. Suppressed native method declarations
        self.assertNotIn("nGetEngine", java_src)
        self.assertNotIn("nGetNativeWindow", java_src)
        self.assertIn("private static native void nRender(long nativeCustomView);", java_src)

        # 5. Suppressed JNI C++ bridge functions
        jni_emitter = JniEmitter(ctx)
        jni_src = jni_emitter.generate_jni()
        self.assertNotIn("nGetEngine", jni_src)
        self.assertNotIn("nGetNativeWindow", jni_src)
        self.assertIn("Java_com_google_android_filament_CustomView_nRender", jni_src)

    def test_handle_wrap_factory_and_package_private_constructor(self):
        """Verify that handle classes generate package-private constructors and @RestrictTo wrap factories."""
        from tools.apigen.javagen import JavaEmitter, ClassContext
        handle_ir = {
            "name": "CustomHandle",
            "qualified_name": "filament::CustomHandle",
            "base_classes": [],
            "attributes": [],
            "methods": [],
            "fields": [],
            "constants": [],
            "enums": [],
        }
        ctx = ClassContext(handle_ir, "CustomHandle.h")
        java_src = JavaEmitter(ctx).generate_java()
        self.assertIn("    CustomHandle(long nativeObject) {", java_src)
        self.assertNotIn("public CustomHandle(long nativeObject)", java_src)
        self.assertIn("@NonNull", java_src)
        self.assertIn("@RestrictTo(RestrictTo.Scope.LIBRARY_GROUP)", java_src)
        self.assertIn("public static CustomHandle wrap(long nativeObject) {", java_src)
        self.assertIn("return new CustomHandle(nativeObject);", java_src)

    def test_unhardcoded_byte_buffer_and_linear_color_imports(self):
        """Verify that classes with PixelBufferDescriptor alias import ByteBuffer,
        and classes with LinearColor alias emit @interface LinearColor without
        relying on hardcoded 'Texture' or 'Colors' class names."""
        from tools.apigen.javagen import JavaEmitter, ClassContext
        
        # Non-Texture class with PixelBufferDescriptor alias
        pbd_class_ir = {
            "name": "CustomStream",
            "qualified_name": "filament::CustomStream",
            "base_classes": [],
            "attributes": [],
            "methods": [],
            "fields": [],
            "constants": [],
            "enums": [],
            "aliases": [
                {
                    "name": "PixelBufferDescriptor",
                    "type": {"cpp_name": "backend::PixelBufferDescriptor", "qualified_name": "filament::backend::PixelBufferDescriptor"}
                }
            ]
        }
        ctx_pbd = ClassContext(pbd_class_ir, "CustomStream.h")
        java_pbd = JavaEmitter(ctx_pbd).generate_java()
        self.assertIn("import java.nio.ByteBuffer;", java_pbd)
        self.assertIn("import java.nio.Buffer;", java_pbd)

        # Non-Colors class with LinearColor alias
        color_class_ir = {
            "name": "CustomPalette",
            "qualified_name": "filament::CustomPalette",
            "base_classes": [],
            "attributes": [],
            "methods": [],
            "fields": [],
            "constants": [],
            "enums": [],
            "aliases": [
                {
                    "name": "LinearColor",
                    "type": {"cpp_name": "math::float3", "qualified_name": "filament::math::float3"}
                }
            ]
        }
        ctx_color = ClassContext(color_class_ir, "CustomPalette.h")
        java_color = JavaEmitter(ctx_color).generate_java()
        self.assertIn("public @interface LinearColor {", java_color)
        self.assertIn("import java.lang.annotation.Retention;", java_color)

    def test_java_object_reserved_noarg_methods(self):
        """Verify that methods named 'wait' or other final Object methods with default parameters
        generate telescopic overloads for non-empty argument lists, but suppress zero-arg wait()."""
        from tools.apigen.javagen import JavaEmitter, ClassContext
        
        fence_ir = {
            "name": "CustomFence",
            "qualified_name": "filament::CustomFence",
            "base_classes": [],
            "attributes": [],
            "methods": [
                {
                    "name": "wait",
                    "return_type": {"cpp_name": "void", "java": "void"},
                    "arguments": [
                        {"name": "mode", "type": {"cpp_name": "int", "java": "int"}, "default_value": "0"},
                        {"name": "timeout", "type": {"cpp_name": "long", "java": "long"}, "default_value": "1000"}
                    ]
                }
            ],
            "fields": [],
            "constants": [],
            "enums": [],
        }
        ctx_fence = ClassContext(fence_ir, "CustomFence.h")
        java_fence = JavaEmitter(ctx_fence).generate_java()
        # Should have convenience overload with 1 arg: wait(int mode)
        self.assertIn("void wait(int mode) {", java_fence)
        # Should NOT have zero-arg wait() which collides with Object.wait()
        self.assertNotIn("void wait() {", java_fence)

    def test_process_file_with_class_name_filter(self):
        """Verify that process_file with class_name only generates the target class when IR has multiple classes."""
        ir = {
            "classes": [
                {
                    "name": "Alpha",
                    "qualified_name": "filament::Alpha",
                    "base_classes": [],
                    "attributes": [],
                    "methods": [],
                    "fields": [],
                    "enums": []
                },
                {
                    "name": "Beta",
                    "qualified_name": "filament::Beta",
                    "base_classes": [],
                    "attributes": [],
                    "methods": [],
                    "fields": [],
                    "enums": []
                }
            ]
        }
        with tempfile.TemporaryDirectory() as tmp_dir:
            ir_path = Path(tmp_dir) / "Multi.json"
            ir_path.write_text(json.dumps(ir))

            javagen.process_file(str(ir_path), tmp_dir, tmp_dir, class_name="Alpha")
            self.assertTrue((Path(tmp_dir) / "Alpha.java").exists())
            self.assertTrue((Path(tmp_dir) / "Alpha.cpp").exists())
            self.assertFalse((Path(tmp_dir) / "Beta.java").exists())
            self.assertFalse((Path(tmp_dir) / "Beta.cpp").exists())

    def test_markdown_to_javadoc_html_handling(self):
        """Verify that markdown_to_javadoc does not produce <p>p>, <p><p>, or wrap block elements in <p>."""
        from tools.apigen.javagen.doc import markdown_to_javadoc

        # Pre-existing <p>...</p> should not be double wrapped into <p><p>...</p></p>
        raw_p = "<p>This is already a paragraph.</p>"
        out = markdown_to_javadoc(raw_p)
        self.assertNotIn("<p><p>", out)
        self.assertNotIn("<p>p>", out)
        self.assertNotIn("</p></p>", out)
        self.assertEqual(out, "<p>This is already a paragraph.</p>")

        # HTML block elements (like <ul>, <table>) should not be wrapped in <p>
        raw_block = "<ul>\n<li>Item 1</li>\n<li>Item 2</li>\n</ul>"
        out_block = markdown_to_javadoc(raw_block)
        self.assertNotIn("<p><ul>", out_block)
        self.assertNotIn("</ul></p>", out_block)

        # Standard paragraph is properly wrapped in <p>...</p>
        raw_std = "This is regular text."
        out_std = markdown_to_javadoc(raw_std)
        self.assertEqual(out_std, "<p>This is regular text.</p>")

    def test_reserved_and_padding_fields_omitted(self):
        """Verify that fields prefixed with reserved, rfu, or padding are omitted from Java and JNI leaves."""
        from tools.apigen.javagen.utils import is_reserved_or_padding_field
        from tools.apigen.javagen import JavaEmitter, ClassContext

        # Test predicate directly
        self.assertTrue(is_reserved_or_padding_field("reserved"))
        self.assertTrue(is_reserved_or_padding_field("reserved1"))
        self.assertTrue(is_reserved_or_padding_field("reserved_pad"))
        self.assertTrue(is_reserved_or_padding_field("rfu"))
        self.assertTrue(is_reserved_or_padding_field("rfu0"))
        self.assertTrue(is_reserved_or_padding_field("padding"))
        self.assertTrue(is_reserved_or_padding_field("padding_bytes"))
        self.assertTrue(is_reserved_or_padding_field("_padding"))
        self.assertFalse(is_reserved_or_padding_field("renderable"))
        self.assertFalse(is_reserved_or_padding_field("radius"))
        self.assertFalse(is_reserved_or_padding_field("ref"))
        self.assertFalse(is_reserved_or_padding_field("color"))

        # Test Java code emission and leaves for a POJO struct
        struct_ir = {
            "name": "CustomOptions",
            "qualified_name": "filament::CustomOptions",
            "is_pojo_struct": True,
            "archetype": "pojo_struct",
            "base_classes": [],
            "attributes": [],
            "methods": [],
            "fields": [
                {"name": "scale", "type": {"cpp_name": "float", "java": "float"}},
                {"name": "reserved1", "type": {"cpp_name": "uint32_t", "java": "int"}},
                {"name": "rfu_mode", "type": {"cpp_name": "uint16_t", "java": "int"}},
                {"name": "padding0", "type": {"cpp_name": "uint8_t", "java": "int"}},
                {"name": "enabled", "type": {"cpp_name": "bool", "java": "boolean"}}
            ],
            "enums": []
        }
        ctx = ClassContext(struct_ir, "CustomOptions.h")
        self.assertEqual([f["name"] for f in ctx.fields], ["scale", "enabled"])

        java_out = JavaEmitter(ctx).generate_java()
        self.assertIn("public float scale;", java_out)
        self.assertIn("public boolean enabled;", java_out)
        self.assertNotIn("reserved1", java_out)
        self.assertNotIn("rfu_mode", java_out)
        self.assertNotIn("padding0", java_out)

        leaves = ctx.get_pojo_leaves(struct_ir)
        leaf_names = [leaf["name"] for leaf in leaves]
        self.assertEqual(leaf_names, ["scale", "enabled"])

    def test_view_npick_jni_generation(self):
        """Verify that View generates Java nPick and C++ JNI Java_com_google_android_filament_View_nPick."""
        from tools.apigen.javagen import ClassContext, JavaEmitter, JniEmitter
        view_ir = {
            "name": "View",
            "qualified_name": "filament::View",
            "base_classes": [],
            "attributes": [],
            "methods": [],
            "fields": [],
            "enums": []
        }
        ctx = ClassContext(view_ir, "View.h")
        java_code = JavaEmitter(ctx).generate_java()
        self.assertIn("void pick(int x, int y,", java_code)
        self.assertIn("nPick(getNativeObject(), x, y, handler, internalCallback);", java_code)
        self.assertIn("private static native void nPick(", java_code)

        jni_code = JniEmitter(ctx).generate_jni()
        self.assertIn("Java_com_google_android_filament_View_nPick", jni_code)
        self.assertIn("view->pick(x, y, [callback]", jni_code)

    def test_binding_validator_clean_on_filament_repo(self):
        """Verify that BindingValidator passes with zero errors on the Filament repo."""
        from tools.apigen.javagen.validator import BindingValidator
        repo_root = Path(__file__).parent.parent.parent.parent.resolve()
        java_dir = repo_root / "android/filament-android/src/main/java"
        cpp_dir = repo_root / "android/filament-android/src/main/cpp"
        validator = BindingValidator(java_dir, cpp_dir)
        res = validator.run()
        self.assertTrue(res.is_clean, f"Validator found errors:\n{res.format_report(verbose=True)}")

    def test_binding_validator_detects_mismatches(self):
        """Verify that BindingValidator accurately detects symbol, signature, and reflection defects."""
        import tempfile
        from tools.apigen.javagen.validator import BindingValidator
        with tempfile.TemporaryDirectory() as tmp_dir:
            tmp_path = Path(tmp_dir)
            j_dir = tmp_path / "java/com/google/android/filament"
            c_dir = tmp_path / "cpp"
            j_dir.mkdir(parents=True)
            c_dir.mkdir(parents=True)

            # Case 1: Symbol missing in C++
            (j_dir / "TestSymbol.java").write_text(
                "package com.google.android.filament;\n"
                "public class TestSymbol {\n"
                "    private static native void nMissingInCpp();\n"
                "}\n"
            )
            (c_dir / "TestSymbol.cpp").write_text(
                '#include <jni.h>\n'
                'extern "C" JNIEXPORT void JNICALL\n'
                'Java_com_google_android_filament_TestSymbol_nOther(JNIEnv*, jclass) {}\n'
            )
            val = BindingValidator(tmp_path / "java", c_dir)
            res = val.run()
            self.assertFalse(res.is_clean)
            err_cats = [e.category for e in res.errors]
            self.assertIn("symbol_parity", err_cats)

            # Case 2: Signature mismatch
            (j_dir / "TestSig.java").write_text(
                "package com.google.android.filament;\n"
                "public class TestSig {\n"
                "    private static native void nFoo(int a);\n"
                "}\n"
            )
            (c_dir / "TestSig.cpp").write_text(
                '#include <jni.h>\n'
                'extern "C" JNIEXPORT void JNICALL\n'
                'Java_com_google_android_filament_TestSig_nFoo(JNIEnv*, jclass, jlong a) {}\n'
            )
            val = BindingValidator(tmp_path / "java", c_dir)
            res = val.run()
            self.assertFalse(res.is_clean)
            err_cats = [e.category for e in res.errors]
            self.assertIn("signature", err_cats)

            # Case 3: Reflection GetFieldID mismatch
            (j_dir / "TestReflect.java").write_text(
                "package com.google.android.filament;\n"
                "public class TestReflect {\n"
                "    public int validField;\n"
                "}\n"
            )
            (c_dir / "TestReflect.cpp").write_text(
                '#include <jni.h>\n'
                'void test(JNIEnv* env) {\n'
                '    jclass clazz = env->FindClass("com/google/android/filament/TestReflect");\n'
                '    env->GetFieldID(clazz, "nonExistentField", "I");\n'
                '}\n'
            )
            val = BindingValidator(tmp_path / "java", c_dir)
            res = val.run()
            self.assertFalse(res.is_clean)
            err_cats = [e.category for e in res.errors]
            self.assertIn("reflection", err_cats)

    def test_generic_standalone_pojo_struct_getter(self):
        """Verify standalone zero-argument POJO struct getters generate cached fields, lazy getters, and skip JNI."""
        from tools.apigen.javagen import ClassContext, JavaEmitter, JniEmitter
        handle_ir = {
            "name": "CustomHandle",
            "qualified_name": "filament::CustomHandle",
            "archetype": "handle",
            "base_classes": [],
            "attributes": [],
            "methods": [
                {
                    "name": "getConfig",
                    "qualified_name": "getConfig()",
                    "return_type": {
                        "cpp_name": "const Config &",
                        "qualified_name": "const filament::CustomHandle::Config &",
                        "category": "struct",
                        "is_const": True,
                        "is_reference": True,
                        "is_pointer": False,
                        "is_pojo_struct": True,
                        "nullability": "nonnull",
                    },
                    "arguments": [],
                    "is_const": True,
                    "is_static": False,
                    "attributes": [],
                    "doc": {
                        "brief": "Returns configuration object.",
                        "return": "a Config object"
                    }
                }
            ],
            "fields": [],
            "enums": [],
            "nested_structs": [
                {
                    "name": "Config",
                    "qualified_name": "filament::CustomHandle::Config",
                    "is_pojo_struct": True,
                    "archetype": "pojo_struct",
                    "base_classes": [],
                    "attributes": [],
                    "methods": [],
                    "fields": [
                        {"name": "size", "type": {"cpp_name": "int", "java": "int"}},
                    ],
                    "enums": []
                }
            ]
        }
        ctx = ClassContext(handle_ir, "CustomHandle.h", nested_structs_map={"CustomHandle": handle_ir["nested_structs"]})
        self.assertIn("Config", ctx.cached_fields)
        self.assertEqual(ctx.cached_fields["Config"]["getter"], "getConfig")
        self.assertIsNone(ctx.cached_fields["Config"]["setter"])

        java_src = JavaEmitter(ctx).generate_java()
        self.assertIn("private @Nullable Config mConfig;", java_src)
        self.assertIn("public Config getConfig() {", java_src)
        self.assertIn("if (mConfig == null) {", java_src)
        self.assertIn("mConfig = new Config();", java_src)
        self.assertIn("Returns configuration object.", java_src)
        self.assertNotIn("nGetConfig", java_src)

        cpp_src = JniEmitter(ctx).generate_jni()
        self.assertNotIn("nGetConfig", cpp_src)

    def test_factory_method_retained_parent_and_nullability(self):
        """Test generic factory method emission with retained parent passing and nullability guards."""
        from tools.apigen.javagen import ClassContext, JavaEmitter, JniEmitter
        from tools.apigen.javagen.config import KNOWN_CLASSES, register_known_classes

        # Child 1: Retains ParentEngine
        child_renderer_ir = {
            "name": "ChildRenderer",
            "qualified_name": "filament::ChildRenderer",
            "archetype": "handle",
            "attributes": [],
            "methods": [
                {
                    "name": "getParent",
                    "qualified_name": "filament::ChildRenderer::getParent",
                    "return_type": {
                        "cpp_name": "ParentEngine *",
                        "qualified_name": "filament::ParentEngine*",
                        "category": "object",
                        "is_pointer": True,
                        "nullability": "nonnull"
                    },
                    "arguments": [],
                    "attributes": ["filament:apigen:retained"]
                }
            ]
        }

        # Child 2: Does not retain parent
        child_view_ir = {
            "name": "ChildView",
            "qualified_name": "filament::ChildView",
            "archetype": "handle",
            "attributes": [],
            "methods": []
        }

        register_known_classes([child_renderer_ir, child_view_ir])

        # Parent handle with factory methods
        parent_ir = {
            "name": "ParentEngine",
            "qualified_name": "filament::ParentEngine",
            "archetype": "handle",
            "attributes": [],
            "methods": [
                {
                    "name": "createRenderer",
                    "qualified_name": "filament::ParentEngine::createRenderer",
                    "return_type": {
                        "cpp_name": "ChildRenderer *",
                        "qualified_name": "filament::ChildRenderer*",
                        "category": "object",
                        "is_pointer": True,
                        "nullability": "nonnull"
                    },
                    "arguments": [],
                    "attributes": [],
                    "doc": {"brief": "Creates a renderer."}
                },
                {
                    "name": "createView",
                    "qualified_name": "filament::ParentEngine::createView",
                    "return_type": {
                        "cpp_name": "ChildView *",
                        "qualified_name": "filament::ChildView*",
                        "category": "object",
                        "is_pointer": True,
                        "nullability": "nonnull"
                    },
                    "arguments": [],
                    "attributes": [],
                    "doc": {"brief": "Creates a view."}
                },
                {
                    "name": "findView",
                    "qualified_name": "filament::ParentEngine::findView",
                    "return_type": {
                        "cpp_name": "ChildView *",
                        "qualified_name": "filament::ChildView*",
                        "category": "object",
                        "is_pointer": True,
                        "nullability": "nullable"
                    },
                    "arguments": [],
                    "attributes": [],
                    "doc": {"brief": "Finds a view or returns null."}
                }
            ],
            "fields": [],
            "enums": []
        }

        ctx = ClassContext(parent_ir, "ParentEngine.h")
        java_src = JavaEmitter(ctx).generate_java()

        # 1. createRenderer should check != 0 and pass 'this'
        self.assertIn("long nativeChildRenderer = nCreateRenderer(getNativeObject());", java_src)
        self.assertIn('if (nativeChildRenderer == 0) throw new IllegalStateException("Couldn\'t create ChildRenderer");', java_src)
        self.assertIn("return new ChildRenderer(nativeChildRenderer, this);", java_src)

        # 2. createView should check != 0 and NOT pass 'this'
        self.assertIn("long nativeChildView = nCreateView(getNativeObject());", java_src)
        self.assertIn('if (nativeChildView == 0) throw new IllegalStateException("Couldn\'t create ChildView");', java_src)
        self.assertIn("return new ChildView(nativeChildView);", java_src)

        # 3. findView is nullable: should return null on 0 without throwing
        self.assertIn("return nativeChildView == 0 ? null : new ChildView(nativeChildView);", java_src)

        # 4. JNI bridge verification
        cpp_src = JniEmitter(ctx).generate_jni()
        self.assertIn("nCreateRenderer", cpp_src)
        self.assertIn("that->createRenderer()", cpp_src)
        self.assertIn("nCreateView", cpp_src)
        self.assertIn("that->createView()", cpp_src)

    def test_generic_cached_handle_reference_getter(self):
        """Verify that zero-argument methods returning C++ references to handles (T&) are lazily cached."""
        from tools.apigen.javagen import ClassContext, JavaEmitter, JniEmitter
        from tools.apigen.javagen.config import KNOWN_CLASSES, register_known_classes

        manager_ir = {
            "name": "DummyManager",
            "qualified_name": "filament::DummyManager",
            "archetype": "handle",
            "attributes": [],
            "methods": []
        }
        parent_ir = {
            "name": "HostEngine",
            "qualified_name": "filament::HostEngine",
            "archetype": "handle",
            "attributes": [],
            "methods": [
                {
                    "name": "getDummyManager",
                    "qualified_name": "filament::HostEngine::getDummyManager",
                    "return_type": {
                        "cpp_name": "DummyManager &",
                        "qualified_name": "filament::DummyManager&",
                        "category": "object",
                        "is_pointer": False,
                        "is_reference": True,
                        "nullability": "unspecified"
                    },
                    "arguments": [],
                    "attributes": [],
                    "doc": {"brief": "Returns DummyManager reference."}
                }
            ],
            "fields": [],
            "enums": []
        }

        register_known_classes([manager_ir])

        ctx = ClassContext(parent_ir, "HostEngine.h")
        self.assertIn("DummyManager", ctx.cached_fields)
        self.assertTrue(ctx.cached_fields["DummyManager"].get("is_handle_reference"))

        java_src = JavaEmitter(ctx).generate_java()
        # 1. Private cached field
        self.assertIn("private @Nullable DummyManager mDummyManager;", java_src)

        # 2. Lazy cached getter with null check
        self.assertIn("@NonNull", java_src)
        self.assertIn("public DummyManager getDummyManager() {", java_src)
        self.assertIn("if (mDummyManager == null) {", java_src)
        self.assertIn("long nativeDummyManager = nGetDummyManager(getNativeObject());", java_src)
        self.assertIn('if (nativeDummyManager == 0) throw new IllegalStateException("Couldn\'t get DummyManager");', java_src)
        self.assertIn("mDummyManager = new DummyManager(nativeDummyManager);", java_src)
        self.assertIn("return mDummyManager;", java_src)

        # 3. Native declaration
        self.assertIn("private static native long nGetDummyManager(long nativeHostEngine);", java_src)

        # 4. JNI implementation
        cpp_src = JniEmitter(ctx).generate_jni()
        self.assertIn("nGetDummyManager", cpp_src)
        self.assertIn("(jlong)&(that->getDummyManager())", cpp_src)


if __name__ == "__main__":
    unittest.main()



