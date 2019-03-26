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


#include <jni.h>

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Viewport.h>
#include <backend/PixelBufferDescriptor.h>

#include "CallbackUtils.h"
#include "NioUtils.h"

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Renderer_nBeginFrame(JNIEnv *, jclass, jlong nativeRenderer,
        jlong nativeSwapChain) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *swapChain = (SwapChain *) nativeSwapChain;
    return (jboolean) renderer->beginFrame(swapChain);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nEndFrame(JNIEnv *, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->endFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nRender(JNIEnv *, jclass, jlong nativeRenderer,
        jlong nativeView) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    View *view = (View *) nativeView;
    renderer->render(view);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nMirrorFrame(JNIEnv *, jclass, jlong nativeRenderer,
        jlong nativeDstSwapChain,
        jint dstLeft, jint dstBottom, jint dstWidth, jint dstHeight,
        jint srcLeft, jint srcBottom, jint srcWidth, jint srcHeight,
        jint flags) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *dstSwapChain = (SwapChain *) nativeDstSwapChain;
    const filament::Viewport dstViewport {dstLeft, dstBottom, (uint32_t) dstWidth, (uint32_t) dstHeight};
    const filament::Viewport srcViewport {srcLeft, srcBottom, (uint32_t) srcWidth, (uint32_t) srcHeight};
    renderer->mirrorFrame(dstSwapChain, dstViewport, srcViewport, (uint32_t) flags);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Renderer_nReadPixels(JNIEnv *env, jclass,
        jlong nativeRenderer, jlong nativeEngine,
        jint xoffset, jint yoffset, jint width, jint height,
        jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment, jint stride, jint format,
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
    auto *callback = JniBufferCallback::make(engine, env, handler, runnable, std::move(nioBuffer));

    PixelBufferDescriptor desc(buffer, sizeInBytes, (backend::PixelDataFormat) format,
            (backend::PixelDataType) type, (uint8_t) alignment, (uint32_t) left, (uint32_t) top,
            (uint32_t) stride, &JniBufferCallback::invoke, callback);

    renderer->readPixels(uint32_t(xoffset), uint32_t(yoffset), uint32_t(width), uint32_t(height),
            std::move(desc));

    return 0;
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Renderer_nGetUserTime(JNIEnv*, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    return renderer->getUserTime();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nResetUserTime(JNIEnv*, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->resetUserTime();
}
