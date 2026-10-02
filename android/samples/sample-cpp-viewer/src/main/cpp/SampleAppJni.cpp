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

#include "AndroidAssetLoader.h"
#include "AndroidDisplayManager.h"

#include <filamentapp/FilamentApp2.h>

#include <private/backend/VirtualMachineEnv.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <samples/SampleConfig.h>
#include <samples/SampleDispatcher.h>

#include <memory>

using filament::app::AndroidDisplayManager;

namespace {

// A running sample and the display manager it renders into. Members are destroyed in reverse
// order of declaration, so the app is destroyed first. This matters because
// FilamentApp2::shutdown() still calls into the display manager.
struct NativeSample {
    AndroidDisplayManager displayManager;
    std::unique_ptr<FilamentApp2> app;
};

NativeSample* toSample(jlong handle) {
    return reinterpret_cast<NativeSample*>(handle);
}

} // namespace

// This is called automatically when the shared library is loaded by the JVM.
// It is necessary because Filament's Android OpenGL/GLES platform (PlatformEGLAndroid)
// and driver threads rely on VirtualMachineEnv to obtain JNIEnv/JavaVM references
// for EGL surface and lifecycle management. Without this call, VirtualMachineEnv::get()
// will fail an assertion ('sVirtualMachine') on startup.
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);

    return JNI_VERSION_1_6;
}

// ---- SampleAppDispatcher (any thread). ----

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_google_android_filament_cppviewer_SampleAppDispatcher_getSampleNames(JNIEnv* env,
        jclass clazz) {
    utils::FixedCapacityVector<utils::CString> names = samples::getSampleNames();
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray array = env->NewObjectArray(names.size(), stringClass, nullptr);
    for (size_t i = 0; i < names.size(); i++) {
        jstring str = env->NewStringUTF(names[i].c_str());
        env->SetObjectArrayElement(array, i, str);
        env->DeleteLocalRef(str);
    }
    return array;
}

// ---- SampleRenderThread. ----
//
// nAcquireNativeWindow() and nReleaseNativeWindow() may be called from any thread. Every other
// entry point must be called on the render thread that created the sample, because a Filament
// Engine must be used and destroyed on the thread that created it.

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nAcquireNativeWindow(JNIEnv* env,
        jobject thiz, jobject surface) {
    // Returns a new reference, which the caller must release with nReleaseNativeWindow().
    return reinterpret_cast<jlong>(ANativeWindow_fromSurface(env, surface));
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nReleaseNativeWindow(JNIEnv* env,
        jobject thiz, jlong nativeWindow) {
    if (nativeWindow) {
        ANativeWindow_release(reinterpret_cast<ANativeWindow*>(nativeWindow));
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nCreateAssetLoader(JNIEnv* env,
        jobject thiz, jobject assetManager) {
    AAssetManager* nativeAssetManager = AAssetManager_fromJava(env, assetManager);
    return reinterpret_cast<jlong>(new AndroidAssetLoader(nativeAssetManager));
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nDestroyAssetLoader(JNIEnv* env,
        jobject thiz, jlong nativeLoader) {
    delete reinterpret_cast<AndroidAssetLoader*>(nativeLoader);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nCreateSample(JNIEnv* env,
        jobject thiz, jstring sampleName, jlong nativeLoader) {
    const char* nameStr = env->GetStringUTFChars(sampleName, nullptr);
    utils::CString name(nameStr);
    env->ReleaseStringUTFChars(sampleName, nameStr);

    SampleConfig config;
    config.title = utils::CString(name.c_str());

    auto sample = std::make_unique<NativeSample>();
    sample->app = samples::dispatchSample(name, config, &sample->displayManager,
            reinterpret_cast<AndroidAssetLoader*>(nativeLoader));
    if (!sample->app) {
        return 0;
    }
    // init() creates the Engine. This must run on the render thread, because the Engine must
    // also be used and destroyed on the thread that created it.
    sample->app->init();
    return reinterpret_cast<jlong>(sample.release());
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nDestroySample(JNIEnv* env,
        jobject thiz, jlong nativeSample) {
    // Destroying the app shuts down its Engine and all of its Filament resources.
    delete toSample(nativeSample);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nOnSurfaceCreated(JNIEnv* env,
        jobject thiz, jlong nativeSample, jlong nativeWindow) {
    NativeSample* sample = toSample(nativeSample);
    sample->displayManager.setNativeWindow(reinterpret_cast<ANativeWindow*>(nativeWindow));
    sample->app->onSurfaceCreated();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nOnSurfaceChanged(JNIEnv* env,
        jobject thiz, jlong nativeSample, jint width, jint height) {
    toSample(nativeSample)->app->onSurfaceChanged(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nOnSurfaceDestroyed(JNIEnv* env,
        jobject thiz, jlong nativeSample) {
    NativeSample* sample = toSample(nativeSample);
    // Destroys the swapchain and waits for the backend to stop using the window.
    sample->app->onSurfaceDestroyed();
    sample->displayManager.setNativeWindow(nullptr);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nOnTouchEvent(JNIEnv* env,
        jobject thiz, jlong nativeSample, jint action, jfloat x, jfloat y) {
    toSample(nativeSample)->displayManager.pushTouchEvent(action, x, y);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_cppviewer_SampleRenderThread_nDoFrame(JNIEnv* env,
        jobject thiz, jlong nativeSample) {
    return toSample(nativeSample)->app->doFrame() ? JNI_TRUE : JNI_FALSE;
}
