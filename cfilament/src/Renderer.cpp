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

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/driver/PixelBufferDescriptor.h>

#include "API.h"

using namespace filament;
using namespace driver;

FBool Filament_Renderer_BeginFrame(Renderer *renderer,
                                  SwapChain *swapChain) {
  return renderer->beginFrame(swapChain);
}

void Filament_Renderer_EndFrame(Renderer *renderer) {
  renderer->endFrame();
}

void Filament_Renderer_Render(Renderer *renderer, View *view) {
  renderer->render(view);
}

/*

jint
Filament_Renderer_nReadPixels(JNIEnv *env, jclass,
        Renderer *renderer, Engine *engine,
        jint xoffset, jint yoffset, jint width, jint height,
        jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment, jint stride, jint
format,
        jobject handler, jobject runnable) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    Engine *engine = (Engine *) nativeEngine;

    stride = stride ? stride : width;
    size_t sizeInBytes = PixelBufferDescriptor::computeDataSize(
            (PixelDataFormat) format, (PixelDataType) type,
            (size_t) stride, (size_t) (height + top), (size_t) alignment);

    AutoBuffer nioBuffer(env, storage, 0);
    if (sizeInBytes > (remaining << nioBuffer.getShift())) {
        // BufferOverflowException
        return -1;
    }

    void *buffer = nioBuffer.getData();
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable,
std::move(nioBuffer));

    PixelBufferDescriptor desc(buffer, sizeInBytes, (driver::PixelDataFormat)
format,
            (driver::PixelDataType) type, (uint8_t) alignment, (uint32_t) left,
(uint32_t) top,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    renderer->readPixels(uint32_t(xoffset), uint32_t(yoffset), uint32_t(width),
uint32_t(height),
            std::move(desc));

    return 0;
}

*/