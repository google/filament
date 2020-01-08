/*
 * Copyright (C) 2017 The Android Open Source Project
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

package com.google.android.filament;

import androidx.annotation.NonNull;

import com.google.android.filament.proguard.UsedByNative;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;

@UsedByNative("NioUtils.cpp")
final class NioUtils {

    enum BufferType {
        BYTE,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE
    }

    private NioUtils() {
    }

    @UsedByNative("NioUtils.cpp")
    static long getBasePointer(@NonNull Buffer b, long address, int sizeShift) {
        return address != 0 ? address + (b.position() << sizeShift) : 0;
    }

    @UsedByNative("NioUtils.cpp")
    static Object getBaseArray(@NonNull Buffer b) {
        return b.hasArray() ? b.array() : null;
    }

    @UsedByNative("NioUtils.cpp")
    static int getBaseArrayOffset(@NonNull Buffer b, int sizeShift) {
        return b.hasArray() ? ((b.arrayOffset() + b.position()) << sizeShift) : 0;
    }

    @UsedByNative("NioUtils.cpp")
    static int getBufferType(@NonNull Buffer b) {
        if (b instanceof ByteBuffer) {
            return BufferType.BYTE.ordinal();
        } else if (b instanceof CharBuffer) {
            return BufferType.CHAR.ordinal();
        } else if (b instanceof ShortBuffer) {
            return BufferType.SHORT.ordinal();
        } else if (b instanceof IntBuffer) {
            return BufferType.INT.ordinal();
        } else if (b instanceof LongBuffer) {
            return BufferType.LONG.ordinal();
        } else if (b instanceof FloatBuffer) {
            return BufferType.FLOAT.ordinal();
        }
        return BufferType.DOUBLE.ordinal();
    }
}
