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
@file:JvmName("RoundingUtils")

package androidx.webgpu.helper

/**
 * Helpers for image dimension management.  Each adds a method to round to the next higher
 * or lower multiple of a value, e.g., "round -1,234 down to the nearest multiple of 1,000"
 * returns -2000.
 */
public fun Long.roundDownToNearestMultipleOf(boundary: Int): Long {
  val padding = if (this < 0 && (this % boundary != 0L)) boundary else 0
  return ((this - padding) / boundary) * boundary
}

public fun Int.roundDownToNearestMultipleOf(boundary: Int): Int {
  val padding = if (this < 0 && (this % boundary != 0)) boundary else 0
  return ((this - padding) / boundary) * boundary
}

public fun Long.roundUpToNearestMultipleOf(boundary: Int): Long {
  val padding = if (this > 0 && (this % boundary != 0L)) boundary else 0
  return ((this + padding) / boundary) * boundary
}

public fun Int.roundUpToNearestMultipleOf(boundary: Int): Int {
  val padding = if (this > 0 && (this % boundary != 0)) boundary else 0
  return ((this + padding) / boundary) * boundary
}
