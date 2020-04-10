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

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

using namespace filament;
using namespace backend;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Renderer_nBeginFrame(JNIEnv *, jclass, jlong nativeRenderer,
        jlong nativeSwapChain, jlong frameTimeNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *swapChain = (SwapChain *) nativeSwapChain;
    return (jboolean) renderer->beginFrame(swapChain, uint64_t(frameTimeNanos));
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
Java_com_google_android_filament_Renderer_nCopyFrame(JNIEnv *, jclass, jlong nativeRenderer,
        jlong nativeDstSwapChain,
        jint dstLeft, jint dstBottom, jint dstWidth, jint dstHeight,
        jint srcLeft, jint srcBottom, jint srcWidth, jint srcHeight,
        jint flags) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *dstSwapChain = (SwapChain *) nativeDstSwapChain;
    const filament::Viewport dstViewport {dstLeft, dstBottom, (uint32_t) dstWidth, (uint32_t) dstHeight};
    const filament::Viewport srcViewport {srcLeft, srcBottom, (uint32_t) srcWidth, (uint32_t) srcHeight};
    renderer->copyFrame(dstSwapChain, dstViewport, srcViewport, (uint32_t) flags);
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

extern "C" JNIEXPORT jint JNICALL
Java_com_google_android_filament_Renderer_nReadPixelsEx(JNIEnv *env, jclass,
        jlong nativeRenderer, jlong nativeEngine, jlong nativeRenderTarget,
        jint xoffset, jint yoffset, jint width, jint height,
        jobject storage, jint remaining,
        jint left, jint top, jint type, jint alignment, jint stride, jint format,
        jobject handler, jobject runnable) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    Engine *engine = (Engine *) nativeEngine;
    RenderTarget *renderTarget = (RenderTarget *) nativeRenderTarget;

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

    renderer->readPixels(renderTarget,
            uint32_t(xoffset), uint32_t(yoffset), uint32_t(width), uint32_t(height),
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

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetDisplayInfo(JNIEnv*, jclass, jlong nativeRenderer,
        jfloat refreshRate, jlong presentationDeadlineNanos, jlong vsyncOffsetNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->setDisplayInfo({ .refreshRate = refreshRate,
                               .presentationDeadlineNanos = (uint64_t)presentationDeadlineNanos,
                               .vsyncOffsetNanos = (uint64_t)vsyncOffsetNanos });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetFrameRateOptions(JNIEnv*, jclass,
    jlong nativeRenderer, jfloat interval, jfloat headRoomRatio, jfloat scaleRate, jint history) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->setFrameRateOptions({ .headRoomRatio = headRoomRatio,
                                     .scaleRate = scaleRate,
                                     .history = (uint8_t)history,
                                     .interval = (uint8_t)interval });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetClearOptions(JNIEnv *, jclass ,
        jlong nativeRenderer, jfloat r, jfloat g, jfloat b, jfloat a,
        jboolean clear, jboolean discard) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->setClearOptions({ .clearColor = {r, g, b, a},
                                .clear = (bool) clear,
                                .discard = (bool) discard});
}
