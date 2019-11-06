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

struct {
#ifdef ANDROID
    jclass handlerClass;
    jmethodID post;
#endif
    jclass executorClass;
    jmethodID execute;
} gCallbackUtils;

JniBufferCallback* JniBufferCallback::make(filament::Engine* engine,
        JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer) {
    void* that = engine->streamAlloc(sizeof(JniBufferCallback), alignof(JniBufferCallback));
    return new (that) JniBufferCallback(env, handler, callback, std::move(buffer));
}

JniBufferCallback::JniBufferCallback(JNIEnv* env, jobject handler, jobject callback,
        AutoBuffer&& buffer)
        : mEnv(env)
        , mHandler(env->NewGlobalRef(handler))
        , mCallback(env->NewGlobalRef(callback))
        , mBuffer(std::move(buffer)){
}

JniBufferCallback::~JniBufferCallback() {
    if (mHandler && mCallback) {
#ifdef ANDROID
        if (mEnv->IsInstanceOf(mHandler, gCallbackUtils.handlerClass)) {
            mEnv->CallBooleanMethod(mHandler, gCallbackUtils.post, mCallback);
        }
#endif
        if (mEnv->IsInstanceOf(mHandler, gCallbackUtils.executorClass)) {
            mEnv->CallVoidMethod(mHandler, gCallbackUtils.execute, mCallback);
        }
    }
    mEnv->DeleteGlobalRef(mHandler);
    mEnv->DeleteGlobalRef(mCallback);
}

void JniBufferCallback::invoke(void*, size_t, void* user) {
    JniBufferCallback* data = reinterpret_cast<JniBufferCallback*>(user);
    // don't call delete here, because we don't own the storage
    data->~JniBufferCallback();
}

JniImageCallback* JniImageCallback::make(filament::Engine* engine,
        JNIEnv* env, jobject handler, jobject callback, long image) {
    void* that = engine->streamAlloc(sizeof(JniImageCallback), alignof(JniImageCallback));
    return new (that) JniImageCallback(env, handler, callback, image);
}

JniImageCallback::JniImageCallback(JNIEnv* env, jobject handler, jobject callback, long image)
        : mEnv(env)
        , mHandler(env->NewGlobalRef(handler))
        , mCallback(env->NewGlobalRef(callback))
        , mImage(image) { }

JniImageCallback::~JniImageCallback() {
    if (mHandler && mCallback) {
#ifdef ANDROID
        if (mEnv->IsInstanceOf(mHandler, gCallbackUtils.handlerClass)) {
            mEnv->CallBooleanMethod(mHandler, gCallbackUtils.post, mCallback);
        }
#endif
        if (mEnv->IsInstanceOf(mHandler, gCallbackUtils.executorClass)) {
            mEnv->CallVoidMethod(mHandler, gCallbackUtils.execute, mCallback);
        }
    }
    mEnv->DeleteGlobalRef(mHandler);
    mEnv->DeleteGlobalRef(mCallback);
}

void JniImageCallback::invoke(void* image, void* user) {
    reinterpret_cast<JniImageCallback*>(user)->~JniImageCallback();
}

void registerCallbackUtils(JNIEnv *env) {
#ifdef ANDROID
    gCallbackUtils.handlerClass = env->FindClass("android/os/Handler");
    gCallbackUtils.handlerClass = (jclass) env->NewGlobalRef(gCallbackUtils.handlerClass);
    gCallbackUtils.post = env->GetMethodID(gCallbackUtils.handlerClass,
            "post", "(Ljava/lang/Runnable;)Z");
#endif

    gCallbackUtils.executorClass = env->FindClass("java/util/concurrent/Executor");
    gCallbackUtils.executorClass = (jclass) env->NewGlobalRef(gCallbackUtils.executorClass);
    gCallbackUtils.execute = env->GetMethodID(gCallbackUtils.executorClass,
            "execute", "(Ljava/lang/Runnable;)V");
}
