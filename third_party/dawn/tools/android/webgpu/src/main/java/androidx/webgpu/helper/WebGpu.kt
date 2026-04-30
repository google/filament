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
@file:JvmName("WebGpuUtils")

package androidx.webgpu.helper

import android.os.Handler
import android.os.Looper
import android.view.Surface
import androidx.webgpu.GPUAdapter
import androidx.webgpu.BackendType
import androidx.webgpu.GPUDevice
import androidx.webgpu.GPUDeviceDescriptor
import androidx.webgpu.DeviceLostCallback
import androidx.webgpu.DeviceLostException
import androidx.webgpu.DeviceLostReason
import androidx.webgpu.ErrorType
import androidx.webgpu.GPUInstance
import androidx.webgpu.GPUInstanceDescriptor
import androidx.webgpu.InternalException
import androidx.webgpu.OutOfMemoryException
import androidx.webgpu.GPURequestAdapterOptions
import androidx.webgpu.RequestAdapterStatus
import androidx.webgpu.GPUSurface
import androidx.webgpu.RequestDeviceStatus
import androidx.webgpu.GPUSurfaceDescriptor
import androidx.webgpu.GPUSurfaceSourceAndroidNativeWindow
import androidx.webgpu.UncapturedErrorCallback
import androidx.webgpu.UnknownException
import androidx.webgpu.ValidationException
import androidx.webgpu.GPU.createInstance
import androidx.webgpu.WebGpuRuntimeException
import androidx.webgpu.helper.Util.windowFromSurface
import java.util.concurrent.Executor

private const val POLLING_DELAY_MS = 100L

public abstract class WebGpu : AutoCloseable {
    public abstract val instance: GPUInstance
    public abstract val webgpuSurface: GPUSurface
    public abstract val device: GPUDevice
}

public suspend fun createWebGpu(
    surface: Surface? = null,
    instanceDescriptor: GPUInstanceDescriptor = GPUInstanceDescriptor(),
    requestAdapterOptions: GPURequestAdapterOptions = GPURequestAdapterOptions(),
    deviceDescriptor: GPUDeviceDescriptor = GPUDeviceDescriptor(
        deviceLostCallback = defaultDeviceLostCallback,
        deviceLostCallbackExecutor = Executor(Runnable::run),
        uncapturedErrorCallback = defaultUncapturedErrorCallback,
        uncapturedErrorCallbackExecutor = Executor(Runnable::run)
    ),
): WebGpu {
    initLibrary()

    val instance = createInstance(instanceDescriptor)
    val webgpuSurface =
        surface?.let {
            instance.createSurface(
                GPUSurfaceDescriptor(
                    surfaceSourceAndroidNativeWindow =
                        GPUSurfaceSourceAndroidNativeWindow(windowFromSurface(it))
                )
            )
        }

    val adapter = requestAdapter(instance, requestAdapterOptions)
    val device = requestDevice(adapter, deviceDescriptor)

    var isClosing = false
    // Long-running event poller for async methods. Can be removed when
    // https://issues.chromium.org/issues/323983633 is fixed.
    val handler = Handler(Looper.getMainLooper())
    fun nextProcess() {
        handler.postDelayed({
            if (isClosing) {
                return@postDelayed
            }
            instance.processEvents()
            nextProcess()
        }, POLLING_DELAY_MS)
    }
    nextProcess()

    return object : WebGpu() {
        override val instance = instance
        override val webgpuSurface
            get() = checkNotNull(webgpuSurface)
        override val device = device

        override fun close() {
            isClosing = true
            //device.close() // TODO(b/428866400): Uncomment when fixed.
            webgpuSurface?.close()
            instance.close()
            adapter.close()
        }
    }
}

private suspend fun requestAdapter(
    instance: GPUInstance,
    options: GPURequestAdapterOptions = GPURequestAdapterOptions(backendType = BackendType.Vulkan),
): GPUAdapter {
    return instance.requestAdapter(options)
}

private suspend inline fun requestDevice(
    adapter: GPUAdapter,
    deviceDescriptor: GPUDeviceDescriptor,
): GPUDevice {
    if (deviceDescriptor.deviceLostCallback == null) {
        deviceDescriptor.deviceLostCallback = defaultDeviceLostCallback
    }

    if (deviceDescriptor.uncapturedErrorCallback == null) {
        deviceDescriptor.uncapturedErrorCallback = defaultUncapturedErrorCallback
    }
    return adapter.requestDevice(deviceDescriptor)
}

private val defaultUncapturedErrorCallback
    get(): UncapturedErrorCallback {
        return UncapturedErrorCallback { _, type, message ->
            throw WebGpuRuntimeException.create(type, message)
        }
    }

private val defaultDeviceLostCallback get(): DeviceLostCallback {
    return DeviceLostCallback { device, reason, message ->
        throw DeviceLostException(device, reason, message)
    }
}

/** Initializes the native library. This method should be called before making and WebGPU calls. */
public fun initLibrary() {
    System.loadLibrary("webgpu_c_bundled")
}
