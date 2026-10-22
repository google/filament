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
package {{ kotlin_package }}

import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlinx.coroutines.suspendCancellableCoroutine

/**
 * A generic callback interface for asynchronous GPU requests.
 * @param T The type of the successful result object.
 */
public interface GPURequestCallback<T> {
  public fun onResult(result: T)
  public fun onError(exception: Exception)
}

public suspend fun <T> awaitGPURequest(
  block: (callback: GPURequestCallback<T>) -> Unit,
): T = suspendCancellableCoroutine { continuation ->
  block(object : GPURequestCallback<T> {
    override fun onResult(result: T) {
      if (continuation.isActive) continuation.resume(result)
    }
    override fun onError(exception: Exception) {
      if (continuation.isActive) continuation.resumeWithException(exception)
    }
  })
}

/**
 * Common Base Class for all JNI Runnables.
 *
 * This handles the "Transport Layer" errors. If the native side reports a non-Success
 * status (like DeviceLost or Unknown), this class handles it immediately.
 *
 * @param T The type expected by the callback.
 */
internal abstract class BaseGPURequestRunnable<T>(
  protected val callback: GPURequestCallback<T>,
  protected val status: Int,
  protected val message: String
) : Runnable {

  override fun run() {
    if (status != Status.Success) {
      callback.onError(WebGpuException(status = status, reason = message))
      return
    }

    handleSuccess()
  }

  /**
   * Called only if status == Status.Success.
   * Implementations should check their specific 'result' payload here.
   */
  abstract fun handleSuccess()
}

/**
 * Handles cases where a Result Object (T) is returned.
 */
internal class GPURequestCallbackRunnable<T>(
  callback: GPURequestCallback<T>,
  status: Int,
  message: String,
  private val result: T?,
) : BaseGPURequestRunnable<T>(callback, status, message) {

  override fun handleSuccess() {
    if (result == null) {
      callback.onError(WebGpuException(status = status, reason = "Null value returned"))
    } else {
      callback.onResult(result)
    }
  }
}

/**
 * Handles cases where the native function returns void (Kotlin Unit).
 */
internal class GPURequestCallbackVoidRunnable(
  callback: GPURequestCallback<Unit>,
  status: Int,
  message: String,
) : BaseGPURequestRunnable<Unit>(callback, status, message) {

  override fun handleSuccess() {
    callback.onResult(Unit)
  }
}

/**
 * Handles cases where the "Success" payload is actually an Error Code integer.
 */
internal class GPURequestCallbackErrorTypeRunnable(
  callback: GPURequestCallback<@ErrorType.Type Int>,
  status: Int,
  private val type: @ErrorType.Type Int,
  message: String,
) : BaseGPURequestRunnable<Int>(callback, status, message) {

  override fun handleSuccess() {
    if (type != ErrorType.NoError) {
      callback.onError(WebGpuRuntimeException.create(type, message))
    } else {
      callback.onResult(type)
    }
  }
}