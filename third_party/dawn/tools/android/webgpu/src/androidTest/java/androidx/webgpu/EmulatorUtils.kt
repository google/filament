/*
 * Copyright 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package androidx.webgpu

import android.os.Build

object EmulatorUtils {
  val isEmulator: Boolean by lazy {
    checkIsEmulator()
  }

  /**
   * Checks if the current Android environment is running on a probable emulator.
   * This method is thread-safe and computes the result only once.
   *
   * @return True if the device is likely an emulator.
   */
  private fun checkIsEmulator(): Boolean {
    // Check ro.kernel.qemu system property.
    val qemuCheck = runCatching {
      val systemPropertyGet = Class.forName("android.os.SystemProperties")
        .getMethod("get", String::class.java, String::class.java)
      "1" == systemPropertyGet.invoke(null, "ro.kernel.qemu", "0")
    }.getOrDefault(false)

    // Hardware check (ranchu, goldfish, cutf_cvm)
    val hardwareCheck = Build.HARDWARE in listOf("ranchu", "goldfish", "cutf_cvm")
    return qemuCheck || hardwareCheck
  }
}
