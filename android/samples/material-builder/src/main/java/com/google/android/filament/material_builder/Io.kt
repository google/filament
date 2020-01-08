/*
 * Copyright (C) 2018 The Android Open Source Project
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

package com.google.android.filament.material_builder

import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

fun readIntLE(input: InputStream): Int {
    return input.read() and 0xff or (
            input.read() and 0xff shl 8) or (
            input.read() and 0xff shl 16) or (
            input.read() and 0xff shl 24)
}

fun readFloat32LE(input: InputStream): Float {
    val bytes = ByteArray(4)
    input.read(bytes, 0, 4)
    return ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN).float
}

fun readUIntLE(input: InputStream): Long {
    return readIntLE(input).toLong() and 0xFFFFFFFFL
}
