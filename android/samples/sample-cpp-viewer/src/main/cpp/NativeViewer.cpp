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

#include "AndroidDisplayManager.h"

#include <filamentapp/FilamentApp2.h>

#include <utils/Log.h>

#include <android/native_window_jni.h>
#include <jni.h>

using namespace filament::app;

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nOnSurfaceCreated(JNIEnv* env, jobject thiz,
        jlong nativeApp, jobject surface) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
    if (!app) {
        return;
    }
    // In Plan Z, the SurfaceView is passed directly to AndroidDisplayManager during its creation.
    // We just tell the app to evaluate the surface state.
    app->onSurfaceCreated();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nOnSurfaceChanged(JNIEnv* env, jobject thiz,
        jlong nativeApp, jint width, jint height) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
    if (app) {
        app->onSurfaceChanged(width, height);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nOnSurfaceDestroyed(JNIEnv* env, jobject thiz,
        jlong nativeApp) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
    if (app) {
        app->onSurfaceDestroyed();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nOnTouchEvent(JNIEnv* env, jobject thiz,
        jlong nativeApp, jint action, jfloat x, jfloat y) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
    if (app) {
        auto* dm = static_cast<AndroidDisplayManager*>(app->getDisplayManager());
        if (dm) {
            dm->pushTouchEvent(action, x, y);
        }
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nDoFrame(JNIEnv* env, jobject thiz,
        jlong nativeApp) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
//    utils::slog.e <<"app=" << app << utils::io::endl;
    return (app && app->doFrame()) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_cppviewer_NativeViewer_nDestroy(JNIEnv* env, jobject thiz,
        jlong nativeApp) {
    auto* app = reinterpret_cast<FilamentApp2*>(nativeApp);
    if (app) {
        // This will call shutdown() and gracefully destroy the engine and all Filament resources.
        delete app;
    }
}
