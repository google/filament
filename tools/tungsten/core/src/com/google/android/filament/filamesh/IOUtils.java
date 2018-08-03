/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.google.android.filament.filamesh;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class IOUtils {
    private IOUtils() {
    }

    static void safelyClose(InputStream in) {
        try {
            if (in != null) {
                in.close();
            }
        } catch (IOException e) {
            // Ignore
        }
    }

    static int readUnsignedShortBE(InputStream in) throws IOException {
        return (in.read() & 0xff) << 8 | (in.read() & 0xff);
    }

    static int readIntLE(InputStream in) throws IOException {
        return (in.read() & 0xff)       |
               (in.read() & 0xff) <<  8 |
               (in.read() & 0xff) << 16 |
               (in.read() & 0xff) << 24;
    }

    static float readFloat32LE(InputStream in) throws IOException {
        byte[] bytes = new byte[4];
        in.read(bytes, 0, 4);
        return ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN).getFloat();
    }

    static long readUnsignedIntLE(InputStream in) throws IOException {
        return readIntLE(in) & 0xFFFFFFFFL;
    }
}
