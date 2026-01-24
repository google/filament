/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <viewer/AutomationEngine.h>

using namespace filament;
using namespace filament::viewer;
using namespace utils;

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nCreateAutomationEngine(JNIEnv* env, jclass,
        jstring spec_) {
    const char* spec = env->GetStringUTFChars(spec_, 0);
    jlong result = (jlong) AutomationEngine::createFromJSON(spec, strlen(spec));
    env->ReleaseStringUTFChars(spec_, spec);
    return result;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nCreateDefaultAutomationEngine(JNIEnv* env,
        jclass klass) {
    return (jlong) AutomationEngine::createDefault();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nSetOptions(JNIEnv* env, jclass klass,
        jlong nativeAutomation, jfloat sleepDuration, jint minFrameCount, jboolean verbose) {
    AutomationEngine* automation = (AutomationEngine*) nativeAutomation;
    AutomationEngine::Options options = {
        .sleepDuration = sleepDuration,
        .minFrameCount = minFrameCount,
        .verbose = (bool) verbose,

        // Since they write to the filesystem, we do not currently support exporting screenshots
        // and JSON files on Android.
        .exportScreenshots = false,
        .exportSettings = false,
    };
    automation->setOptions(options);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nStartRunning(JNIEnv* env, jclass klass,
        jlong nativeAutomation) {
    AutomationEngine* automation = (AutomationEngine*) nativeAutomation;
    automation->startRunning();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nStartBatchMode(JNIEnv* env, jclass klass,
        jlong nativeAutomation) {
    AutomationEngine* automation = (AutomationEngine*) nativeAutomation;
    automation->startBatchMode();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nTick(JNIEnv* env, jclass klass,
        jlong nativeAutomation, jlong nativeEngine,
        jlong view, jlongArray materials, jlong renderer, jfloat deltaTime) {
    using MaterialPointer = MaterialInstance*;
    jsize materialCount = 0;
    jlong* longMaterials = nullptr;
    MaterialPointer* ptrMaterials = nullptr;
    if (materials) {
        materialCount = env->GetArrayLength(materials);
        ptrMaterials = new MaterialPointer[materialCount];
        longMaterials = env->GetLongArrayElements(materials, nullptr);
        for (jsize i = 0; i < materialCount; i++) {
            ptrMaterials[i] = (MaterialPointer) longMaterials[i];
        }
    }
    AutomationEngine* automation = (AutomationEngine*) nativeAutomation;
    AutomationEngine::ViewerContent content = {
        .view = (View*) view,
        .renderer = (Renderer*) renderer,
        .materials = ptrMaterials,
        .materialCount = (size_t) materialCount,
    };
    Engine* engine = (Engine*)nativeEngine;
    automation->tick(engine, content, deltaTime);
    if (longMaterials) {
        env->ReleaseLongArrayElements(materials, longMaterials, 0);
        delete[] ptrMaterials;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nApplySettings(JNIEnv* env, jclass klass,
        jlong nativeAutomation, jlong nativeEngine,
        jstring json, jlong view, jlongArray materials, jlong nativeIbl,
        jint sunlightEntity, jintArray assetLights, jlong nativeLm, jlong scene, jlong renderer) {
    using MaterialPointer = MaterialInstance*;

    jsize materialCount = 0;
    jlong* longMaterials = nullptr;
    MaterialPointer* ptrMaterials = nullptr;
    if (materials) {
        materialCount = env->GetArrayLength(materials);
        ptrMaterials = new MaterialPointer[materialCount];
        longMaterials = env->GetLongArrayElements(materials, nullptr);
        for (jsize i = 0; i < materialCount; i++) {
            ptrMaterials[i] = (MaterialPointer) longMaterials[i];
        }
    }

    jsize lightCount = 0;
    jint* intLights = nullptr;
    if (assetLights) {
        lightCount = env->GetArrayLength(assetLights);
        intLights = env->GetIntArrayElements(assetLights, nullptr);
    }

    static_assert(sizeof(jint) == sizeof(Entity));

    AutomationEngine* automation = (AutomationEngine*) nativeAutomation;
    const char* nativeJson = env->GetStringUTFChars(json, 0);
    size_t jsonLength = env->GetStringUTFLength(json);

    AutomationEngine::ViewerContent content = {
        .view = (View*) view,
        .renderer = (Renderer*) renderer,
        .materials = ptrMaterials,
        .materialCount = (size_t) materialCount,
        .lightManager = (LightManager*) nativeLm,
        .scene = (Scene*) scene,
        .indirectLight = (IndirectLight*) nativeIbl,
        .sunlight = (Entity&) sunlightEntity,
        .assetLights = (Entity*) intLights,
        .assetLightCount = (size_t) lightCount,
    };
    Engine* engine = (Engine*)nativeEngine;
    automation->applySettings(engine, nativeJson, jsonLength, content);
    env->ReleaseStringUTFChars(json, nativeJson);
    if (longMaterials) {
        env->ReleaseLongArrayElements(materials, longMaterials, 0);
        delete[] ptrMaterials;
    }
    if (intLights) {
        env->ReleaseIntArrayElements(assetLights, intLights, 0);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nGetViewerOptions(JNIEnv* env, jclass,
        jlong nativeObject, jobject result) {
    AutomationEngine* automation = (AutomationEngine*) nativeObject;
    auto options = automation->getViewerOptions();

    const jclass klass = env->GetObjectClass(result);

    const jfieldID cameraAperture = env->GetFieldID(klass, "cameraAperture", "F");
    const jfieldID cameraSpeed = env->GetFieldID(klass, "cameraSpeed", "F");
    const jfieldID cameraISO = env->GetFieldID(klass, "cameraISO", "F");
    const jfieldID cameraNear = env->GetFieldID(klass, "cameraNear", "F");
    const jfieldID cameraFar = env->GetFieldID(klass, "cameraFar", "F");
    const jfieldID groundShadowStrength = env->GetFieldID(klass, "groundShadowStrength", "F");
    const jfieldID groundPlaneEnabled = env->GetFieldID(klass, "groundPlaneEnabled", "Z");
    const jfieldID skyboxEnabled = env->GetFieldID(klass, "skyboxEnabled", "Z");
    const jfieldID cameraFocalLength = env->GetFieldID(klass, "cameraFocalLength", "F");
    const jfieldID cameraFocusDistance = env->GetFieldID(klass, "cameraFocusDistance", "F");
    const jfieldID autoScaleEnabled = env->GetFieldID(klass, "autoScaleEnabled", "Z");
    const jfieldID autoInstancingEnabled = env->GetFieldID(klass, "autoInstancingEnabled", "Z");

    env->SetFloatField(result, cameraAperture, options.cameraAperture);
    env->SetFloatField(result, cameraSpeed, options.cameraSpeed);
    env->SetFloatField(result, cameraISO, options.cameraISO);
    env->SetFloatField(result, cameraNear, options.cameraNear);
    env->SetFloatField(result, cameraFar, options.cameraFar);
    env->SetFloatField(result, groundShadowStrength, options.groundShadowStrength);
    env->SetBooleanField(result, groundPlaneEnabled, options.groundPlaneEnabled);
    env->SetBooleanField(result, skyboxEnabled, options.skyboxEnabled);
    env->SetFloatField(result, cameraFocalLength, options.cameraFocalLength);
    env->SetFloatField(result, cameraFocusDistance, options.cameraFocusDistance);
    env->SetBooleanField(result, autoScaleEnabled, options.autoScaleEnabled);
    env->SetBooleanField(result, autoInstancingEnabled, options.autoInstancingEnabled);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nGetColorGrading(JNIEnv*, jclass,
        jlong nativeObject, jlong nativeEngine) {
    AutomationEngine* automation = (AutomationEngine*) nativeObject;
    return (jlong) automation->getColorGrading((Engine*) nativeEngine);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nSignalBatchMode(JNIEnv*, jclass, jlong native) {
    AutomationEngine* automation = (AutomationEngine*) native;
    automation->signalBatchMode();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nStopRunning(JNIEnv*, jclass, jlong native) {
    AutomationEngine* automation = (AutomationEngine*) native;
    automation->stopRunning();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nShouldClose(JNIEnv*, jclass, jlong native) {
    AutomationEngine* automation = (AutomationEngine*) native;
    return automation->shouldClose();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nDestroy(JNIEnv*, jclass, jlong native) {
    AutomationEngine* automation = (AutomationEngine*) native;
    delete automation;
}
