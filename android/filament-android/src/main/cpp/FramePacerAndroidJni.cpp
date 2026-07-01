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

#include <filament/Engine.h>
#include <filament/FramePacer.h>
#include <filament/Renderer.h>

#include <common/JniUtils.h>
#include <jni.h>

using namespace filament;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_android_FramePacer_nCreateBuilder(JNIEnv*, jclass) {
    return (jlong) new FramePacer::Builder();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nDestroyBuilder(JNIEnv*, jclass, jlong nativeBuilder) {
    auto builder = (FramePacer::Builder*) nativeBuilder;
    delete builder;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nBuilderTargetFrameRate(JNIEnv*, jclass, jlong nativeBuilder, jfloat fps) {
    auto builder = (FramePacer::Builder*) nativeBuilder;
    builder->targetFrameRate(fps);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nBuilderLatencyFrames(JNIEnv*, jclass, jlong nativeBuilder, jint frames) {
    auto builder = (FramePacer::Builder*) nativeBuilder;
    builder->latencyFrames((uint32_t) frames);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_android_FramePacer_nBuilderBuild(JNIEnv* env, jclass, jlong nativeBuilder, jlong nativeEngine) {
    auto builder = (FramePacer::Builder*) nativeBuilder;
    auto engine = (Engine*) nativeEngine;
    return filament::android::wrapJni<jlong>(env, [=]() {
        return (jlong) builder->build(*engine);
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_android_FramePacer_nSetupFrame(JNIEnv* env, jclass,
        jlong nativeObject, jlong baseTimeNanos, jlong vsyncPeriodNanos,
        jlongArray hardwareTimelines, jint timelineCount) {
    auto pacer = (FramePacer*) nativeObject;
    return filament::android::wrapJni<jboolean>(env, [=]() {
        FramePacer::VsyncTick tick;
        tick.baseTime    = FramePacer::time_point_t(std::chrono::nanoseconds(baseTimeNanos));
        tick.vsyncPeriod = FramePacer::duration_t(std::chrono::nanoseconds(vsyncPeriodNanos));

        if (hardwareTimelines != nullptr && timelineCount > 0) {
            jlong* elements = env->GetLongArrayElements(hardwareTimelines, nullptr);
            auto* timelineStructs = reinterpret_cast<FramePacer::HardwareTimeline*>(elements);

            tick.timelines = { timelineStructs, (size_t) timelineCount };
            jboolean result = (jboolean) pacer->setupFrame(tick);

            env->ReleaseLongArrayElements(hardwareTimelines, elements, JNI_ABORT);
            return result;
        }

        return (jboolean) pacer->setupFrame(tick);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_android_FramePacer_nGetExpectedPresentationTime(JNIEnv* env, jclass, jlong nativeObject) {
    auto pacer = (FramePacer*) nativeObject;
    return filament::android::wrapJni<jlong>(env, [=]() {
        return (jlong) pacer->getExpectedPresentationTime().time_since_epoch().count();
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_android_FramePacer_nGetRenderingDeadline(JNIEnv* env, jclass, jlong nativeObject) {
    auto pacer = (FramePacer*) nativeObject;
    return filament::android::wrapJni<jlong>(env, [=]() {
        return (jlong) pacer->getRenderingDeadline().time_since_epoch().count();
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_android_FramePacer_nHasGpuFallenBehind(JNIEnv* env, jclass, jlong nativeObject, jlong nativeRenderer) {
    auto pacer = (FramePacer*) nativeObject;
    auto renderer = (Renderer*) nativeRenderer;
    return filament::android::wrapJni<jboolean>(env, [=]() {
        return (jboolean) pacer->hasGpuFallenBehind(renderer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nApplyPresentationTime(JNIEnv* env, jclass, jlong nativeObject, jlong nativeRenderer) {
    auto pacer = (FramePacer*) nativeObject;
    auto renderer = (Renderer*) nativeRenderer;
    filament::android::wrapJni(env, [=]() {
        pacer->applyPresentationTime(renderer);
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nConfigure(JNIEnv* env, jclass, jlong nativeObject, jfloat targetFrameRate, jint latencyFrames) {
    auto pacer = (FramePacer*) nativeObject;
    filament::android::wrapJni(env, [=]() {
        FramePacer::Configuration config;
        config.targetFrameRate = (float) targetFrameRate;
        config.latencyFrames   = (uint32_t) latencyFrames;
        pacer->configure(config);
    });
}

extern "C" JNIEXPORT jfloat JNICALL
Java_com_google_android_filament_android_FramePacer_nGetSelectedFrameRate(JNIEnv* env, jclass, jlong nativeObject) {
    auto pacer = (FramePacer*) nativeObject;
    return filament::android::wrapJni<jfloat>(env, [=]() {
        return (jfloat) pacer->getSelectedFrameRate();
    });
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_android_FramePacer_nIsExactFrameRateAchieved(JNIEnv* env, jclass, jlong nativeObject) {
    auto pacer = (FramePacer*) nativeObject;
    return filament::android::wrapJni<jboolean>(env, [=]() {
        return (jboolean) pacer->isExactFrameRateAchieved();
    });
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_android_FramePacer_nDestroy(JNIEnv* env, jclass, jlong nativeEngine, jlong nativeObject) {
    auto engine = (Engine*) nativeEngine;
    auto pacer = (FramePacer*) nativeObject;
    filament::android::wrapJni(env, [=]() {
        engine->destroy(pacer);
    });
}
