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
import org.junit.Assume
import org.junit.rules.TestRule
import org.junit.runner.Description
import org.junit.runners.model.Statement

class ApiLevelSkipRule : TestRule {
  override fun apply(base: Statement, description: Description): Statement {
    val annotation = getAnnotation(description) ?: return base

    val minApi = annotation.minApi
    if (Build.VERSION.SDK_INT >= minApi) {
      return base // API level is sufficient, do not skip.
    }

    // API level is less than minApi. Check if we should still run.
    if (annotation.onlySkipOnEmulator && !EmulatorUtils.isEmulator) {
      return base // Don't skip if onlySkipOnEmulator is true and we are not on an emulator.
    }
    // Otherwise, skip the test.
    return object : Statement() {
      override fun evaluate() {
        Assume.assumeTrue(
          "Skipping test due to API level requirement: " +
            "minApi=${minApi}, deviceApi=${Build.VERSION.SDK_INT}",
          false,
        )
      }
    }
  }

  private fun getAnnotation(description: Description): ApiRequirement? {
    return description.getAnnotation(ApiRequirement::class.java)
      ?: description.testClass?.getAnnotation(ApiRequirement::class.java)
  }
}
