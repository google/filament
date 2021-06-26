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
        jlong nativeAutomation, jlong view, jlongArray materials, jlong renderer, jfloat deltaTime) {
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
    automation->tick((View*) view, ptrMaterials, materialCount, (Renderer*) renderer, deltaTime);
    if (longMaterials) {
        env->ReleaseLongArrayElements(materials, longMaterials, 0);
        delete[] ptrMaterials;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AutomationEngine_nApplySettings(JNIEnv* env, jclass klass,
        jlong nativeAutomation, jstring json, jlong view, jlongArray materials, jlong nativeIbl,
        jint lightEntity, jlong nativeLm, jlong scene, jlong renderer) {
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
    const char* nativeJson = env->GetStringUTFChars(json, 0);
    size_t jsonLength = env->GetStringUTFLength(json);
    automation->applySettings(nativeJson, jsonLength, (View*) view, ptrMaterials, materialCount,
            (IndirectLight*) nativeIbl, (Entity&) lightEntity, (LightManager*) nativeLm,
            (Scene*) scene, (Renderer*) renderer);
    env->ReleaseStringUTFChars(json, nativeJson);
    if (longMaterials) {
        env->ReleaseLongArrayElements(materials, longMaterials, 0);
        delete[] ptrMaterials;
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
    const jfieldID groundShadowStrength = env->GetFieldID(klass, "groundShadowStrength", "F");
    const jfieldID groundPlaneEnabled = env->GetFieldID(klass, "groundPlaneEnabled", "Z");
    const jfieldID skyboxEnabled = env->GetFieldID(klass, "skyboxEnabled", "Z");
    const jfieldID cameraFocalLength = env->GetFieldID(klass, "cameraFocalLength", "F");
    const jfieldID cameraFocusDistance = env->GetFieldID(klass, "cameraFocusDistance", "F");

    env->SetFloatField(result, cameraAperture, options.cameraAperture);
    env->SetFloatField(result, cameraSpeed, options.cameraSpeed);
    env->SetFloatField(result, cameraISO, options.cameraISO);
    env->SetFloatField(result, groundShadowStrength, options.groundShadowStrength);
    env->SetBooleanField(result, groundPlaneEnabled, options.groundPlaneEnabled);
    env->SetBooleanField(result, skyboxEnabled, options.skyboxEnabled);
    env->SetFloatField(result, cameraFocalLength, options.cameraFocalLength);
    env->SetFloatField(result, cameraFocusDistance, options.cameraFocusDistance);
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
