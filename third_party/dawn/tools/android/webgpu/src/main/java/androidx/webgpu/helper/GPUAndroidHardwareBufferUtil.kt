/*
 * Copyright 2026 The Android Open Source Project
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

package androidx.webgpu.helper

import android.hardware.HardwareBuffer
import android.os.Build
import androidx.annotation.RequiresApi
import androidx.webgpu.GPUDevice
import androidx.webgpu.GPUExternalTextureDescriptor
import androidx.webgpu.GPUHardwareBufferExternalTexture
import androidx.webgpu.ExperimentalWebGpuApi
import androidx.webgpu.GPUHardwareBufferTexture

@RequiresApi(Build.VERSION_CODES.O)
@OptIn(ExperimentalWebGpuApi::class)
public object GPUAndroidHardwareBufferUtil {

  public fun createTexture(
    device: GPUDevice, ahb: HardwareBuffer,
    @androidx.webgpu.TextureUsage.Type usage: Int,
  ): GPUHardwareBufferTexture {
    return createTextureNative(device, ahb, usage)
      ?: throw IllegalStateException("Dawn failed to import RGB HardwareBuffer")
  }

  @JvmStatic
  private external fun createTextureNative(
    device: GPUDevice,
    ahb: HardwareBuffer,
    usage: Int,
  ): GPUHardwareBufferTexture?

  public fun createExternalTexture(
    device: GPUDevice,
    ahb: HardwareBuffer,
    descriptor: GPUExternalTextureDescriptor,
  ): GPUHardwareBufferExternalTexture {
    return createExternalTextureNative(device, ahb, descriptor)
      ?: throw IllegalStateException("Dawn failed to import YUV HardwareBuffer")
  }

  @JvmStatic
  private external fun createExternalTextureNative(
    device: GPUDevice,
    ahb: HardwareBuffer,
    descriptor: GPUExternalTextureDescriptor,
  ): GPUHardwareBufferExternalTexture?
}