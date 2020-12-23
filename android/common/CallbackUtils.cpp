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

#include "CallbackUtils.h"

void acquireCallbackJni(JNIEnv* env, CallbackJni& callbackUtils) {
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

void releaseCallbackJni(JNIEnv* env, CallbackJni callbackUtils, jobject handler, jobject callback) {
    if (handler && callback) {
#ifdef ANDROID
        if (env->IsInstanceOf(handler, callbackUtils.handlerClass)) {
            env->CallBooleanMethod(handler, callbackUtils.post, callback);
        }
#endif
        if (env->IsInstanceOf(handler, callbackUtils.executorClass)) {
            env->CallVoidMethod(handler, callbackUtils.execute, callback);
        }
    }
    env->DeleteGlobalRef(handler);
    env->DeleteGlobalRef(callback);
#ifdef ANDROID
    env->DeleteGlobalRef(callbackUtils.handlerClass);
#endif
    env->DeleteGlobalRef(callbackUtils.executorClass);
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
    acquireCallbackJni(env, mCallbackUtils);
}

JniBufferCallback::~JniBufferCallback() {
    releaseCallbackJni(mEnv, mCallbackUtils, mHandler, mCallback);
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
    acquireCallbackJni(env, mCallbackUtils);
}

JniImageCallback::~JniImageCallback() {
    releaseCallbackJni(mEnv, mCallbackUtils, mHandler, mCallback);
}

void JniImageCallback::invoke(void*, void* user) {
    JniImageCallback* data = reinterpret_cast<JniImageCallback*>(user);
    delete data;
}

// -----------------------------------------------------------------------------------------------

JniCallback* JniCallback::make(JNIEnv* env, jobject handler, jobject callback) {
    return new JniCallback(env, handler, callback);
}

JniCallback::JniCallback(JNIEnv* env, jobject handler, jobject callback)
        : mEnv(env)
        , mHandler(env->NewGlobalRef(handler))
        , mCallback(env->NewGlobalRef(callback)) {
    acquireCallbackJni(env, mCallbackUtils);
}

JniCallback::~JniCallback() {
    releaseCallbackJni(mEnv, mCallbackUtils, mHandler, mCallback);
}

void JniCallback::invoke(void* user) {
    JniCallback* data = reinterpret_cast<JniCallback*>(user);
    delete data;
}
