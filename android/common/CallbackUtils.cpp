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

#include <functional>

#include "CallbackUtils.h"

static void initCallbackJni(JNIEnv* env, CallbackJni& callbackUtils) {
#ifdef ANDROID
    callbackUtils.handlerClass = env->FindClass("android/os/Handler");
    callbackUtils.handlerClass = (jclass) env->NewGlobalRef(callbackUtils.handlerClass);
    callbackUtils.post = env->GetMethodID(callbackUtils.handlerClass,
            "post", "(Ljava/lang/Runnable;)Z");
#endif

    callbackUtils.executorClass = env->FindClass("java/util/concurrent/Executor");
    callbackUtils.executorClass = (jclass) env->NewGlobalRef(callbackUtils.executorClass);
    callbackUtils.execute = env->GetMethodID(callbackUtils.executorClass,
                                              "execute", "(Ljava/lang/Runnable;)V");
}

JniBufferCallback* JniBufferCallback::make(filament::Engine* engine,
        JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer) {
    return new JniBufferCallback(env, handler, callback, std::move(buffer));
}

JniBufferCallback::JniBufferCallback(JNIEnv* env, jobject handler, jobject callback,
        AutoBuffer&& buffer)
        : mEnv(env)
        , mHandler(env->NewGlobalRef(handler))
        , mCallback(env->NewGlobalRef(callback))
        , mBuffer(std::move(buffer)) {
    initCallbackJni(env, mCallbackUtils);
}

JniBufferCallback::~JniBufferCallback() {
    if (mHandler && mCallback) {
#ifdef ANDROID
        if (mEnv->IsInstanceOf(mHandler, mCallbackUtils.handlerClass)) {
            mEnv->CallBooleanMethod(mHandler, mCallbackUtils.post, mCallback);
        }
#endif
        if (mEnv->IsInstanceOf(mHandler, mCallbackUtils.executorClass)) {
            mEnv->CallVoidMethod(mHandler, mCallbackUtils.execute, mCallback);
        }
    }
    mEnv->DeleteGlobalRef(mHandler);
    mEnv->DeleteGlobalRef(mCallback);
#ifdef ANDROID
    mEnv->DeleteGlobalRef(mCallbackUtils.handlerClass);
#endif
    mEnv->DeleteGlobalRef(mCallbackUtils.executorClass);
}

void JniBufferCallback::invoke(void*, size_t, void* user) {
    JniBufferCallback* data = reinterpret_cast<JniBufferCallback*>(user);
    delete data;
}

// -----------------------------------------------------------------------------------------------

JniImageCallback* JniImageCallback::make(filament::Engine* engine,
        JNIEnv* env, jobject handler, jobject callback, long image) {
    return new JniImageCallback(env, handler, callback, image);
}

JniImageCallback::JniImageCallback(JNIEnv* env, jobject handler, jobject callback, long image)
        : mEnv(env)
        , mHandler(env->NewGlobalRef(handler))
        , mCallback(env->NewGlobalRef(callback))
        , mImage(image) {
    initCallbackJni(env, mCallbackUtils);
}

JniImageCallback::~JniImageCallback() {
    if (mHandler && mCallback) {
#ifdef ANDROID
        if (mEnv->IsInstanceOf(mHandler, mCallbackUtils.handlerClass)) {
            mEnv->CallBooleanMethod(mHandler, mCallbackUtils.post, mCallback);
        }
#endif
        if (mEnv->IsInstanceOf(mHandler, mCallbackUtils.executorClass)) {
            mEnv->CallVoidMethod(mHandler, mCallbackUtils.execute, mCallback);
        }
    }
    mEnv->DeleteGlobalRef(mHandler);
    mEnv->DeleteGlobalRef(mCallback);
#ifdef ANDROID
    mEnv->DeleteGlobalRef(mCallbackUtils.handlerClass);
#endif
    mEnv->DeleteGlobalRef(mCallbackUtils.executorClass);
}

void JniImageCallback::invoke(void*, void* user) {
    JniImageCallback* data = reinterpret_cast<JniImageCallback*>(user);
    delete data;
}
