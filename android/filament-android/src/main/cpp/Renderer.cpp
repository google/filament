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


#include "common/CallbackUtils.h"
#include "common/JniUtils.h"
#include "common/NioUtils.h"

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>

#include <jni.h>

#include <exception>

using namespace filament;
using namespace backend;
using namespace filament::android;


extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSkipFrame(JNIEnv *env, jclass, jlong nativeRenderer,
        jlong vsyncSteadyClockTimeNano) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->skipFrame(uint64_t(vsyncSteadyClockTimeNano));
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Renderer_nShouldRenderFrame(JNIEnv *, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    return (jboolean) renderer->shouldRenderFrame();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Renderer_nBeginFrame(JNIEnv *env, jclass, jlong nativeRenderer,
        jlong nativeSwapChain, jlong frameTimeNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *swapChain = (SwapChain *) nativeSwapChain;
    return wrapJniBackend<jboolean>(env, [=]() {
        return (jboolean) renderer->beginFrame(swapChain, uint64_t(frameTimeNanos));
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nEndFrame(JNIEnv *env, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJniBackend(env, [=]() {
        renderer->endFrame();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nRender(JNIEnv *env, jclass, jlong nativeRenderer,
        jlong nativeView) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    View *view = (View *) nativeView;
    wrapJniBackend(env, [=]() {
        renderer->render(view);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nRenderStandaloneView(JNIEnv *env, jclass, jlong nativeRenderer,
        jlong nativeView) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    View *view = (View *) nativeView;
    wrapJni(env, [=]() {
        renderer->renderStandaloneView(view);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nCopyFrame(JNIEnv *env, jclass, jlong nativeRenderer,
        jlong nativeDstSwapChain,
        jint dstLeft, jint dstBottom, jint dstWidth, jint dstHeight,
        jint srcLeft, jint srcBottom, jint srcWidth, jint srcHeight,
        jint flags) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    SwapChain *dstSwapChain = (SwapChain *) nativeDstSwapChain;
    const filament::Viewport dstViewport {dstLeft, dstBottom, (uint32_t) dstWidth, (uint32_t) dstHeight};
    const filament::Viewport srcViewport {srcLeft, srcBottom, (uint32_t) srcWidth, (uint32_t) srcHeight};
    wrapJni(env, [=]() {
        renderer->copyFrame(dstSwapChain, dstViewport, srcViewport, (uint32_t) flags);
    });
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
            (uint32_t) stride,
            callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

    return wrapJni<jint>(env, [&]() {
        renderer->readPixels(uint32_t(xoffset), uint32_t(yoffset), uint32_t(width), uint32_t(height),
                std::move(desc));
        return 0;
    });
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
            (uint32_t) stride,
            callback->getHandler(), &JniBufferCallback::postToJavaAndDestroy, callback);

    return wrapJni<jint>(env, [&]() {
        renderer->readPixels(renderTarget,
                uint32_t(xoffset), uint32_t(yoffset), uint32_t(width), uint32_t(height),
                std::move(desc));
        return 0;
    });
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_google_android_filament_Renderer_nGetMaterialTime(JNIEnv *env, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    return wrapJni<jdouble>(env, [=]() {
        return renderer->getMaterialTime();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nResetUserTime(JNIEnv *env, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->resetUserTime();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetMaterialTimeEpoch(JNIEnv *env, jclass, jlong nativeRenderer, jlong monotonicClockNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setMaterialTimeEpoch(monotonicClockNanos);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetDisplayInfo(JNIEnv*, jclass, jlong nativeRenderer, jfloat refreshRate) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->setDisplayInfo({ .refreshRate = refreshRate });
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
Java_com_google_android_filament_Renderer_nSetClearOptions(
    JNIEnv* env, jclass, jlong nativeRenderer, jdouble r, jdouble g, jdouble b, jdouble a,
    jboolean clear, jboolean discard) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setClearOptions({ .clearColor = { r, g, b, a },
                                    .clear = (bool) clear,
                                    .discard = (bool) discard });
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetPresentationTime(JNIEnv *env, jclass ,
    jlong nativeRenderer, jlong monotonicClockNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setPresentationTime(monotonicClockNanos);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetDesiredPresentationTime(JNIEnv *env, jclass ,
    jlong nativeRenderer, jlong monotonicClockNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setDesiredPresentationTime(monotonicClockNanos);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetRenderingDeadline(JNIEnv *env, jclass ,
    jlong nativeRenderer, jlong monotonicClockNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setRenderingDeadline(std::chrono::steady_clock::time_point(std::chrono::nanoseconds(monotonicClockNanos)));
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetFrameScheduleTime(JNIEnv *env, jclass ,
    jlong nativeRenderer, jlong timeSteadyClockNano) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->setFrameScheduleTime(timeSteadyClockNano);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSetVsyncTime(JNIEnv *, jclass,
    jlong nativeRenderer, jlong steadyClockTimeNano) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->setVsyncTime(steadyClockTimeNano);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nSkipNextFrames(JNIEnv *, jclass ,
    jlong nativeRenderer, jint frameCount) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    renderer->skipNextFrames(frameCount);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Renderer_nGetFrameToSkipCount(JNIEnv *, jclass ,
    jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    return renderer->getFrameToSkipCount();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_Renderer_nHasGpuFallenBehind(JNIEnv *env, jclass, jlong nativeRenderer) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    return wrapJni<jboolean>(env, [=]() {
        return (jboolean) renderer->hasGpuFallenBehind();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Renderer_nPauseRenderThread(JNIEnv *env, jclass, jlong nativeRenderer, jlong durationNanos) {
    Renderer *renderer = (Renderer *) nativeRenderer;
    wrapJni(env, [=]() {
        renderer->pauseRenderThread(durationNanos);
    });
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Renderer_nGetMaxFrameHistorySize(JNIEnv *env, jclass ,
    jlong nativeRenderer) {
    Renderer const* renderer = (Renderer const*) nativeRenderer;
    return static_cast<jint>(renderer->getMaxFrameHistorySize());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_google_android_filament_Renderer_nGetFrameInfoHistory(JNIEnv *env, jclass ,
    jlong nativeRenderer, jobjectArray outHistory) {
    Renderer const* renderer = (Renderer const*) nativeRenderer;

    static const struct JniFrameInfoState {
        jfieldID frameId;
        jfieldID gpuFrameDuration;
        jfieldID denoisedGpuFrameDuration;
        jfieldID beginFrame;
        jfieldID endFrame;
        jfieldID backendBeginFrame;
        jfieldID backendEndFrame;
        jfieldID gpuFrameComplete;
        jfieldID vsync;
        jfieldID displayPresent;
        jfieldID presentDeadline;
        jfieldID displayPresentInterval;
        jfieldID compositionToPresentLatency;
        jfieldID expectedPresentLatency;
        jfieldID frameScheduleTime;
        explicit JniFrameInfoState(JNIEnv* env) noexcept {
            jclass frameInfoClass = env->FindClass("com/google/android/filament/Renderer$FrameInfo");
            frameId = env->GetFieldID(frameInfoClass, "frameId", "I");
            gpuFrameDuration = env->GetFieldID(frameInfoClass, "gpuFrameDuration", "J");
            denoisedGpuFrameDuration = env->GetFieldID(frameInfoClass, "denoisedGpuFrameDuration", "J");
            beginFrame = env->GetFieldID(frameInfoClass, "beginFrame", "J");
            endFrame = env->GetFieldID(frameInfoClass, "endFrame", "J");
            backendBeginFrame = env->GetFieldID(frameInfoClass, "backendBeginFrame", "J");
            backendEndFrame = env->GetFieldID(frameInfoClass, "backendEndFrame", "J");
            gpuFrameComplete = env->GetFieldID(frameInfoClass, "gpuFrameComplete", "J");
            vsync = env->GetFieldID(frameInfoClass, "vsync", "J");
            displayPresent = env->GetFieldID(frameInfoClass, "displayPresent", "J");
            presentDeadline = env->GetFieldID(frameInfoClass, "presentDeadline", "J");
            displayPresentInterval = env->GetFieldID(frameInfoClass, "displayPresentInterval", "J");
            compositionToPresentLatency = env->GetFieldID(frameInfoClass, "compositionToPresentLatency", "J");
            expectedPresentLatency = env->GetFieldID(frameInfoClass, "expectedPresentLatency", "J");
            frameScheduleTime = env->GetFieldID(frameInfoClass, "frameScheduleTime", "J");
        }
    } jniState(env);

    jsize const arrayLength = env->GetArrayLength(outHistory);
    if (arrayLength <= 0) {
        return 0;
    }

    auto const history = renderer->getFrameInfoHistory(static_cast<size_t>(arrayLength));
    jsize const count = static_cast<jsize>(history.size());

    for (jsize i = 0; i < count; ++i) {
        jobject obj = env->GetObjectArrayElement(outHistory, i);
        if (!obj) {
            continue;
        }

        auto const& info = history[i];
        env->SetIntField(obj,  jniState.frameId, static_cast<jint>(info.frameId));
        env->SetLongField(obj, jniState.gpuFrameDuration, info.gpuFrameDuration);
        env->SetLongField(obj, jniState.denoisedGpuFrameDuration, info.denoisedGpuFrameDuration);
        env->SetLongField(obj, jniState.beginFrame, info.beginFrame);
        env->SetLongField(obj, jniState.endFrame, info.endFrame);
        env->SetLongField(obj, jniState.backendBeginFrame, info.backendBeginFrame);
        env->SetLongField(obj, jniState.backendEndFrame, info.backendEndFrame);
        env->SetLongField(obj, jniState.gpuFrameComplete, info.gpuFrameComplete);
        env->SetLongField(obj, jniState.vsync, info.vsync);
        env->SetLongField(obj, jniState.displayPresent, info.displayPresent);
        env->SetLongField(obj, jniState.presentDeadline, info.presentDeadline);
        env->SetLongField(obj, jniState.displayPresentInterval, info.displayPresentInterval);
        env->SetLongField(obj, jniState.compositionToPresentLatency, info.compositionToPresentLatency);
        env->SetLongField(obj, jniState.expectedPresentLatency, info.expectedPresentLatency);
        env->SetLongField(obj, jniState.frameScheduleTime, info.frameScheduleTime);

        env->DeleteLocalRef(obj);
    }

    return count;
}
