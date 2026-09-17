/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <backend/PixelBufferDescriptor.h>

#include <common/JniUtils.h>
#include <jni.h>

using namespace filament;
using namespace filament::backend;
using namespace utils;

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_PixelBufferDescriptor_nComputeDataSize(JNIEnv*, jclass,
        jint format, jint type, jint stride, jint height, jint alignment) {
    return (jint) backend::PixelBufferDescriptor::computeDataSize(
            (backend::PixelDataFormat) format,
            (backend::PixelDataType) type,
            (size_t) stride, (size_t) height, (size_t) alignment);
}
