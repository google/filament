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

import androidx.test.filters.SmallTest
import androidx.webgpu.helper.WebGpu
import androidx.webgpu.helper.createWebGpu
import kotlinx.coroutines.runBlocking
import org.junit.Test
import junit.framework.TestCase.assertEquals
import org.junit.After
import org.junit.Assert.assertThrows
import org.junit.Before

@Suppress("UNUSED_VARIABLE")
@SmallTest
class ShaderModuleTest {
  private lateinit var webGpu: WebGpu
  private lateinit var device: GPUDevice

  private val invalidShader = """
            @vertex fn main() -> @builtin(position) vec4<f32> {
              return unknown(0.0, 0.0, 0.0, 1.0);
            }
        """.trimIndent()

  @Before
  fun setup() = runBlocking {
    webGpu = createWebGpu()
    device = webGpu.device
  }

  @After
  fun teardown() {
    runCatching { device.destroy() }
    webGpu.close()
  }

  // The info request should still succeed even if compilation fails.
  private suspend fun getCompilationInfo(code: String): GPUCompilationInfo {
    val shaderModule =
      device.createShaderModule(GPUShaderModuleDescriptor(shaderSourceWGSL = GPUShaderSourceWGSL(code)))
    return shaderModule.getCompilationInfo()
  }


  @Test
  fun getCompilationInfoReturnsNoErrorsForValidAsciiShader() {
    val code = """
            @vertex fn main() -> @builtin(position) vec4<f32> {
              return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            }
        """.trimIndent()
    val info = runBlocking { getCompilationInfo(code) }
    val errorCount = info.messages.count { it.type == CompilationMessageType.Error }
    assertEquals(0, errorCount)
  }

  /**
   * Verifies that an invalid shader correctly produces a validation error
   * and a compilation error message.
   */
  @Test
  fun invalidShader_producesACompilationError() {
    device.pushErrorScope(ErrorFilter.Validation)
    val info = runBlocking { getCompilationInfo(invalidShader) }
    assertThrows("The operation should result in a validation error",
      ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }

    val errorCount = info.messages.count { it.type == CompilationMessageType.Error }
    assertEquals(1, errorCount)
  }

  /**
   * Verifies that the primary compilation error for the invalid shader is reported
   * on the correct line number.
   */
  @Test
  fun invalidShader_reportsCorrectLineNumber() {
    device.pushErrorScope(ErrorFilter.Validation)
    val info = runBlocking { getCompilationInfo(invalidShader) }
    val errorMessage = info.messages.first { it.type == CompilationMessageType.Error }

    val expectedErrorLine = 2L
    assertEquals(
      "The error should be reported on line $expectedErrorLine.",
      expectedErrorLine,
      errorMessage.lineNum
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }

  /**
   * Verifies that the absolute character offset reported in the error message is
   * mathematically consistent with its reported line number and line position (column).
   */
  @Test
  fun invalidShader_reportsConsistentOffsetAndLinePosition() {
    device.pushErrorScope(ErrorFilter.Validation)
    val info = runBlocking { getCompilationInfo(invalidShader) }
    val errorMessage = info.messages.first { it.type == CompilationMessageType.Error }

    // Pre-computation based on the error message's reported location
    val lines = invalidShader.split('\n')
    val lineIndex = (errorMessage.lineNum - 1).toInt()   // 1-based line number to 0-based index
    val positionOffset = (errorMessage.linePos - 1)     // 1-based column to 0-based offset

    // Calculate the expected offset from scratch
    val calculatedOffset = lines.take(lineIndex).sumOf { it.length + 1 } + positionOffset

    assertEquals(
      "Calculated offset from line/pos should match the message's absolute offset.",
      errorMessage.offset,
      calculatedOffset
    )
    assertThrows(ValidationException::class.java) {
      runBlocking { device.popErrorScope() }
    }
  }
}