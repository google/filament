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

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

# Add parent directory to path to import generate_android
APIGEN_DIR = Path(__file__).parent.parent.resolve()
sys.path.append(str(APIGEN_DIR))

import generate_android

class TestGenerateAndroid(unittest.TestCase):
    def test_reorganize_headers_tool_path(self):
        """Ensure that REORGANIZE_HEADERS is configured and points to run.py."""
        self.assertTrue(generate_android.REORGANIZE_HEADERS.exists(),
                        f"Expected {generate_android.REORGANIZE_HEADERS} to exist")
        self.assertEqual(generate_android.REORGANIZE_HEADERS.name, "run.py")

    def test_reorganize_cpp_file(self):
        """Verify that reorganize_cpp_file reorganizes include headers in a C++ file."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            test_cpp = Path(tmp_dir) / "Test.cpp"
            # Write unorganized includes (<jni.h> at top before <filament/Box.h>)
            test_cpp.write_text("""/* Header */
#include <jni.h>
#include <common/JniUtils.h>
#include <filament/Box.h>
#include <math/mat4.h>

void foo() {}
""")
            ok, err = generate_android.reorganize_cpp_file(test_cpp)
            self.assertTrue(ok, f"Reorganize failed: {err}")

            content = test_cpp.read_text()
            # filament header must precede common/JniUtils and jni.h
            idx_box = content.find("#include <filament/Box.h>")
            idx_jni_utils = content.find("#include <common/JniUtils.h>")
            idx_jni = content.find("#include <jni.h>")

            self.assertNotEqual(idx_box, -1)
            self.assertNotEqual(idx_jni_utils, -1)
            self.assertNotEqual(idx_jni, -1)
            self.assertLess(idx_box, idx_jni_utils)
            self.assertLess(idx_box, idx_jni)

    def test_generate_target_invokes_reorganize_headers(self):
        """Verify that generate_target reorganizes headers in the generated .cpp file."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            java_out = Path(tmp_dir) / "java"
            cpp_out = Path(tmp_dir) / "cpp"
            java_out.mkdir()
            cpp_out.mkdir()

            test_ir = {
                "classes": [
                    {
                        "name": "SampleTarget",
                        "qualified_name": "filament::SampleTarget",
                        "methods": [
                            {
                                "name": "dummyMethod",
                                "return_type": {"cpp_name": "void", "category": "void"},
                                "arguments": []
                            }
                        ]
                    }
                ]
            }
            json_file = Path(tmp_dir) / "SampleTarget.json"
            json_file.write_text(json.dumps(test_ir))

            ok, name, err = generate_android.generate_target(
                ("SampleTarget", json_file), java_out, cpp_out
            )
            self.assertTrue(ok, f"generate_target failed: {err}")
            self.assertEqual(name, "SampleTarget")

            generated_cpp = cpp_out / "SampleTarget.cpp"
            self.assertTrue(generated_cpp.exists())
            content = generated_cpp.read_text()

            # Ensure headers in generated file are reorganized
            if "#include <filament/SampleTarget.h>" in content:
                idx_filament = content.find("#include <filament/SampleTarget.h>")
                idx_jni = content.find("#include <jni.h>")
                if idx_jni != -1:
                    self.assertLess(idx_filament, idx_jni)

if __name__ == "__main__":
    unittest.main()
