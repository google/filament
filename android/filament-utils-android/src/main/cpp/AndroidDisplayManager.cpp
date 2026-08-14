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

#include <filamentapp/AppEvent.h>

#include <utils/Mutex.h>

#include <android/native_window_jni.h>

#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>


namespace filament::app {

AndroidDisplayManager::AndroidDisplayManager(JavaVM* vm, jobject surfaceView)
        : mJavaVM(vm), mStartTime(std::chrono::steady_clock::now()) {
    JNIEnv* env;
    mJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    mSurfaceView = surfaceView ? env->NewGlobalRef(surfaceView) : nullptr;
}

AndroidDisplayManager::~AndroidDisplayManager() {
    terminate();
}

void AndroidDisplayManager::terminate() {
    if (mSurfaceView) {
        JNIEnv* env;
        mJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
        env->DeleteGlobalRef(mSurfaceView);
        mSurfaceView = nullptr;
    }
    utils::LockGuard<utils::Mutex> lock(mMutex);
    mEventQueue.clear();
}

WindowHandle AndroidDisplayManager::createWindow(const char* title, uint32_t w, uint32_t h,
        bool resizable, bool headless) {
    if (headless) {
        return nullptr;
    }
    return (WindowHandle) mSurfaceView;
}

void AndroidDisplayManager::destroyWindow(WindowHandle window) {
    // Handled in terminate
}


void* AndroidDisplayManager::getNativeWindow(WindowHandle window) const {
    if (!window) return nullptr;

    jobject surfaceView = static_cast<jobject>(window);

    JNIEnv* env;
    bool attached = false;
    if (mJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (mJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return nullptr;
        }
        attached = true;
    }

    jclass svClass = env->GetObjectClass(surfaceView);
    jmethodID getHolder = env->GetMethodID(svClass, "getHolder", "()Landroid/view/SurfaceHolder;");
    jobject holder = env->CallObjectMethod(surfaceView, getHolder);

    jclass holderClass = env->GetObjectClass(holder);
    jmethodID getSurface = env->GetMethodID(holderClass, "getSurface", "()Landroid/view/Surface;");
    jobject surface = env->CallObjectMethod(holder, getSurface);

    jclass surfaceClass = env->GetObjectClass(surface);
    jmethodID isValid = env->GetMethodID(surfaceClass, "isValid", "()Z");

    void* nativeWindow = nullptr;
    if (env->CallBooleanMethod(surface, isValid)) {
        nativeWindow = ANativeWindow_fromSurface(env, surface);
    }

    if (attached) {
        mJavaVM->DetachCurrentThread();
    }
    return nativeWindow;
}

void AndroidDisplayManager::setWindowTitle(WindowHandle window, const char* title) {}

void AndroidDisplayManager::getWindowSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    if (w) *w = mWidth;
    if (h) *h = mHeight;
}

void AndroidDisplayManager::getDrawableSize(WindowHandle window, uint32_t* w, uint32_t* h) const {
    if (w) *w = mWidth;
    if (h) *h = mHeight;
}

void AndroidDisplayManager::pollEvents(std::vector<AppEvent>& events) {
    utils::LockGuard<utils::Mutex> lock(mMutex);
    events.insert(events.end(), mEventQueue.begin(), mEventQueue.end());
    mEventQueue.clear();
}

uint32_t AndroidDisplayManager::getMouseState(int* x, int* y) const {
    return 0;
}

double AndroidDisplayManager::getTime() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = now - mStartTime;
    return diff.count();
}

void AndroidDisplayManager::onFrameFinished(WindowHandle window, filament::Engine* engine,
        filament::Renderer* renderer) {}



void AndroidDisplayManager::setWindowSize(uint32_t w, uint32_t h) {
    mWidth = w;
    mHeight = h;
}

void AndroidDisplayManager::pushEvent(const AppEvent& event) {
    utils::LockGuard<utils::Mutex> lock(mMutex);
    mEventQueue.push_back(event);
}

void AndroidDisplayManager::pushTouchEvent(int action, float x, float y) {
    AppEvent event;
    event.windowId = mSurfaceView;
    switch (action) {
        case 0: // ACTION_DOWN
            event.type = AppEvent::Type::MOUSE_BUTTON_DOWN;
            event.mouseButton.button = 1;
            event.mouseButton.x = (int32_t) x;
            event.mouseButton.y = (int32_t) y;
            pushEvent(event);
            break;
        case 1: // ACTION_UP
        case 3: // ACTION_CANCEL
            event.type = AppEvent::Type::MOUSE_BUTTON_UP;
            event.mouseButton.button = 1;
            event.mouseButton.x = (int32_t) x;
            event.mouseButton.y = (int32_t) y;
            pushEvent(event);
            break;
        case 2: // ACTION_MOVE
            event.type = AppEvent::Type::MOUSE_MOVE;
            event.mouseMove.x = (int32_t) x;
            event.mouseMove.y = (int32_t) y;
            pushEvent(event);
            break;
        default:
            break;
    }
}

} // namespace filament::app

extern "C" JNIEXPORT jlong JNICALL
Java_com_google_android_filament_utils_AndroidDisplayManager_nCreate(JNIEnv* env, jobject thiz,
        jobject surfaceView) {
    JavaVM* vm;
    env->GetJavaVM(&vm);
    auto* dm = new filament::app::AndroidDisplayManager(vm, surfaceView);
    return reinterpret_cast<jlong>(dm);
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_utils_AndroidDisplayManager_nDestroy(JNIEnv* env, jobject thiz,
        jlong nativeDm) {
    auto* dm = reinterpret_cast<filament::app::AndroidDisplayManager*>(nativeDm);
    delete dm;
}
