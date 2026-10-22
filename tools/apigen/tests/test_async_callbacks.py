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
#

import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

# Add tools/apigen directory to module search path
REPO_ROOT = Path(__file__).parent.parent.parent.parent.resolve()
APIGEN_DIR = Path(__file__).parent.parent.resolve()
sys.path.append(str(APIGEN_DIR))

import javagen
from javagen.context import ClassContext
from javagen.jni_emitter import JniEmitter
from javagen.java_emitter import JavaEmitter


class TestAsyncCallbacks(unittest.TestCase):
    """Independent regression suite validating generic asynchronous callback mechanics.

    Proves:
        1. Exact positional argument sequence preservation in C++ calls (leading, trailing, mixed).
        2. Generic variadic lambda trampoline `[jniCallback](auto&&...)` decoupling callback arguments.
        3. Exception safety via `wrapJni<void>` for throwing methods and zero-overhead direct calls for `noexcept`.
        4. Physical C++17 compilation of generated bindings against Filament headers.
    """

    @classmethod
    def setUpClass(cls):
        cls.fixture_dir = Path(__file__).parent / "java"
        cls.fixture_header = cls.fixture_dir / "26_async_callbacks.h"
        cls.fixture_json = cls.fixture_dir / "26_async_callbacks.json"
        assert cls.fixture_header.exists(), f"Missing fixture header: {cls.fixture_header}"
        assert cls.fixture_json.exists(), f"Missing fixture json: {cls.fixture_json}"

    def test_ir_classification(self):
        """Verify that all fixture methods are classified as async callback methods."""
        with open(self.fixture_json) as f:
            api_ir = json.load(f)

        cls_ir = api_ir["classes"][0]
        ctx = ClassContext(cls_ir, api_ir, str(self.fixture_header))

        methods = {m["name"]: m for m in cls_ir.get("methods", [])}
        self.assertIn("setFrameScheduledCallback", methods)
        self.assertIn("setFrameCompletedCallback", methods)
        self.assertIn("registerChannelCallback", methods)
        self.assertIn("dispatchCustom", methods)

        for name, m in methods.items():
            self.assertTrue(ctx.is_async_callback_method(m), f"Expected {name} to be async callback method")

    def test_jni_positional_argument_sequence(self):
        """Verify exact positional sequence of arguments in generated C++ JNI calls."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            res = subprocess.run(
                [
                    sys.executable,
                    str(APIGEN_DIR / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(self.fixture_json),
                ],
                capture_output=True,
                text=True,
            )
            self.assertEqual(res.returncode, 0, f"javagen failed: {res.stderr}")

            cpp_files = list(Path(tmp_dir).glob("*.cpp"))
            self.assertEqual(len(cpp_files), 1)
            cpp_content = cpp_files[0].read_text()

            # Case 1: Trailing auxiliary parameter (flags follows handler and callback)
            # Signature: setFrameScheduledCallback(CallbackHandler* handler, FrameScheduledCallback&& callback, uint64_t flags)
            self.assertIn(
                "that->setFrameScheduledCallback(jniCallback->getHandler(), [jniCallback](auto&&...) { JniCallback::postToJavaAndDestroy(jniCallback); }, (uint64_t)flags);",
                cpp_content,
                "Trailing parameter ordering violated in setFrameScheduledCallback"
            )

            # Case 2: Standard 2-param noexcept method (handler, callback)
            self.assertIn(
                "that->setFrameCompletedCallback(jniCallback->getHandler(), [jniCallback](auto&&...) { JniCallback::postToJavaAndDestroy(jniCallback); });",
                cpp_content,
                "Standard 2-parameter callback ordering violated in setFrameCompletedCallback"
            )

            # Case 3: Leading auxiliary parameter (channel precedes handler and callback)
            # Signature: registerChannelCallback(uint32_t channel, CallbackHandler* handler, SimpleCallback&& callback)
            self.assertIn(
                "that->registerChannelCallback((uint32_t)channel, jniCallback->getHandler(), [jniCallback](auto&&...) { JniCallback::postToJavaAndDestroy(jniCallback); });",
                cpp_content,
                "Leading parameter ordering violated in registerChannelCallback"
            )

            # Case 4: Mixed leading and trailing auxiliary parameters
            # Signature: dispatchCustom(uint32_t channel, CallbackHandler* handler, SimpleCallback&& callback, float timeout, uint32_t flags)
            self.assertIn(
                "that->dispatchCustom((uint32_t)channel, jniCallback->getHandler(), [jniCallback](auto&&...) { JniCallback::postToJavaAndDestroy(jniCallback); }, (float)timeout, (uint32_t)flags);",
                cpp_content,
                "Mixed parameter ordering violated in dispatchCustom"
            )

    def test_exception_safety_wrapping(self):
        """Verify wrapJni<void> wrapping for non-noexcept methods and direct call for noexcept."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            subprocess.run(
                [
                    sys.executable,
                    str(APIGEN_DIR / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(self.fixture_json),
                ],
                capture_output=True,
                check=True,
            )
            cpp_content = list(Path(tmp_dir).glob("*.cpp"))[0].read_text()

            # setFrameCompletedCallback is marked `noexcept` -> must NOT be wrapped in wrapJni
            self.assertNotIn("wrapJni", cpp_content.split("nSetFrameCompletedCallback")[1].split("}")[0])

            # Non-noexcept methods must be wrapped in filament::android::wrapJni<void>
            scheduled_body = cpp_content.split("nSetFrameScheduledCallback")[1].split("}")[0]
            self.assertIn("filament::android::wrapJni<void>", scheduled_body)

            channel_body = cpp_content.split("nRegisterChannelCallback")[1].split("}")[0]
            self.assertIn("filament::android::wrapJni<void>", channel_body)

            custom_body = cpp_content.split("nDispatchCustom")[1].split("}")[0]
            self.assertIn("filament::android::wrapJni<void>", custom_body)

    def test_java_method_and_native_declarations(self):
        """Verify generated Java public methods and private native declarations."""
        with tempfile.TemporaryDirectory() as tmp_dir:
            subprocess.run(
                [
                    sys.executable,
                    str(APIGEN_DIR / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(self.fixture_json),
                ],
                capture_output=True,
                check=True,
            )
            java_content = list(Path(tmp_dir).glob("*.java"))[0].read_text()

            # Method declarations in Java
            self.assertIn("public void setFrameScheduledCallback(@Nullable Object handler, @Nullable Runnable callback, @IntRange(from = 0) long flags)", java_content)
            self.assertIn("public void setFrameCompletedCallback(@Nullable Object handler, @Nullable Runnable callback)", java_content)
            self.assertIn("public void registerChannelCallback(@IntRange(from = 0) int channel, @Nullable Object handler, @Nullable Runnable callback)", java_content)
            self.assertIn("public void dispatchCustom(@IntRange(from = 0) int channel, @Nullable Object handler, @Nullable Runnable callback, float timeout, @IntRange(from = 0) int flags)", java_content)

            # Native declarations in Java
            self.assertIn("private static native void nSetFrameScheduledCallback(long nativeAsyncCallbacksTest, Object handler, Runnable callback, long flags);", java_content)
            self.assertIn("private static native void nSetFrameCompletedCallback(long nativeAsyncCallbacksTest, Object handler, Runnable callback);", java_content)
            self.assertIn("private static native void nRegisterChannelCallback(long nativeAsyncCallbacksTest, int channel, Object handler, Runnable callback);", java_content)
            self.assertIn("private static native void nDispatchCustom(long nativeAsyncCallbacksTest, int channel, Object handler, Runnable callback, float timeout, int flags);", java_content)

            # Functional interfaces must NOT be generated for async callbacks
            self.assertNotIn("@FunctionalInterface", java_content)

    def test_physical_cpp_compilation(self):
        """Verify that generated C++ JNI bridge compiles without errors under Clang."""
        clang = shutil.which("clang++")
        if not clang:
            self.skipTest("clang++ compiler not found in PATH")

        # Find JDK jni.h
        jni_include_candidates = glob.glob("/Library/Java/JavaVirtualMachines/*/Contents/Home/include")
        if not jni_include_candidates:
            self.skipTest("JDK include path containing jni.h not found")
        jdk_include = jni_include_candidates[0]

        with tempfile.TemporaryDirectory() as tmp_dir:
            subprocess.run(
                [
                    sys.executable,
                    str(APIGEN_DIR / "javagen.py"),
                    "--java-dir", tmp_dir,
                    "--jni-dir", tmp_dir,
                    str(self.fixture_json),
                ],
                capture_output=True,
                check=True,
            )
            gen_cpp = list(Path(tmp_dir).glob("*.cpp"))[0]
            obj_file = Path(tmp_dir) / "output.o"

            compile_cmd = [
                clang,
                "-c", str(gen_cpp),
                "-std=c++17",
                "-I", str(REPO_ROOT),
                "-I", str(REPO_ROOT / "android"),
                "-I", str(REPO_ROOT / "filament/include"),
                "-I", str(REPO_ROOT / "libs/utils/include"),
                "-I", str(REPO_ROOT / "libs/math/include"),
                "-I", str(REPO_ROOT / "filament/backend/include"),
                "-I", str(REPO_ROOT / "android/filament-android/src/main/cpp"),
                "-I", jdk_include,
                "-I", f"{jdk_include}/darwin",
                "-o", str(obj_file),
            ]
            compile_res = subprocess.run(compile_cmd, capture_output=True, text=True)
            self.assertEqual(
                compile_res.returncode, 0,
                f"Generated C++ JNI compilation failed:\n{compile_res.stderr}"
            )
            self.assertTrue(obj_file.exists(), "Expected compiled object file to exist")


if __name__ == "__main__":
    unittest.main()
