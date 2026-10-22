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
package androidx.webgpu

public suspend fun <T : Throwable> assertThrowsSuspend(
    expectedThrowable: Class<T>,
    block: suspend () -> Unit,
): T {
  try {
    block()
  } catch (t: Throwable) {
    if (expectedThrowable.isInstance(t)) {
      @Suppress("UNCHECKED_CAST")
      return t as T
    }
    throw AssertionError(
      "Expected exception of type ${expectedThrowable.name} but got ${t.javaClass.name}",
      t
    )
  }
  throw AssertionError("Expected exception of type ${expectedThrowable.name} but no exception was thrown")
}

public suspend fun <T : Throwable> assertThrowsSuspend(
    message: String,
    expectedThrowable: Class<T>,
    block: suspend () -> Unit,
): T {
  try {
    block()
  } catch (t: Throwable) {
    if (expectedThrowable.isInstance(t)) {
      @Suppress("UNCHECKED_CAST")
      return t as T
    }
    throw AssertionError(
      "$message: Expected exception of type ${expectedThrowable.name} but got ${t.javaClass.name}",
      t
    )
  }
  throw AssertionError("$message: Expected exception of type ${expectedThrowable.name} but no exception was thrown")
}
