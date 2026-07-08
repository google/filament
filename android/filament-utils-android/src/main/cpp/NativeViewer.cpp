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

#include <filamentapp/FilamentApp.h>

#include <android/native_window_jni.h>
#include <jni.h>

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_NativeViewer_onSurfaceCreated(JNIEnv* env, jobject thiz, jobject surface) {
    ANativeWindow* window = nullptr;
    if (surface != nullptr) {
        window = ANativeWindow_fromSurface(env, surface);
    }
    FilamentApp::get().onSurfaceCreated(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_NativeViewer_onSurfaceChanged(JNIEnv* env, jobject thiz, jint width, jint height) {
    FilamentApp::get().onSurfaceChanged(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_NativeViewer_onSurfaceDestroyed(JNIEnv* env, jobject thiz) {
    FilamentApp::get().onSurfaceDestroyed();
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_NativeViewer_onTouchEvent(JNIEnv* env, jobject thiz, jint action, jfloat x, jfloat y) {
    FilamentApp::get().onTouchEvent(action, x, y);
}
