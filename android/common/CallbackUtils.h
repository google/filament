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

#pragma once

#include <jni.h>

#include "common/NioUtils.h"

#include <backend/CallbackHandler.h>

#include <filament/Engine.h>

struct CallbackJni {
#ifdef __ANDROID__
    jclass handlerClass = nullptr;
    jmethodID post = nullptr;
#endif
    jclass executorClass = nullptr;
    jmethodID execute = nullptr;
};

void acquireCallbackJni(JNIEnv* env, CallbackJni& callbackUtils);
void releaseCallbackJni(JNIEnv* env, CallbackJni callbackUtils, jobject handler, jobject callback);

struct JniCallback : private filament::backend::CallbackHandler {
    JniCallback(JniCallback const &) = delete;
    JniCallback(JniCallback&&) = delete;
    JniCallback& operator=(JniCallback const &) = delete;
    JniCallback& operator=(JniCallback&&) = delete;

    // create a JniCallback
    static JniCallback* make(JNIEnv* env, jobject handler, jobject runnable);

    // execute the callback on the java thread and destroy ourselves
    static void postToJavaAndDestroy(JniCallback* callback);

    // CallbackHandler interface.
    void post(void* user, Callback callback) override;

    // Get the CallbackHandler interface
    filament::backend::CallbackHandler* getHandler() noexcept { return this; }

    jobject getCallbackObject() { return mCallback; }

protected:
    JniCallback(JNIEnv* env, jobject handler, jobject runnable);
    explicit JniCallback() = default; // this version does nothing
    virtual ~JniCallback();
    jobject mHandler{};
    jobject mCallback{};
    CallbackJni mCallbackUtils{};
};


struct JniBufferCallback : public JniCallback {
    // create a JniBufferCallback
    static JniBufferCallback* make(filament::Engine* engine,
            JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer);

    // execute the callback on the java thread and destroy ourselves
    static void postToJavaAndDestroy(void*, size_t, void* user);

private:
    JniBufferCallback(JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer);
    virtual ~JniBufferCallback();
    AutoBuffer mBuffer;
};

struct JniImageCallback : public JniCallback {
    // create a JniImageCallback
    static JniImageCallback* make(filament::Engine* engine, JNIEnv* env, jobject handler,
            jobject runnable, long image);

    // execute the callback on the java thread and destroy ourselves
    static void postToJavaAndDestroy(void*, void* user);

private:
    JniImageCallback(JNIEnv* env, jobject handler, jobject runnable, long image);
    virtual ~JniImageCallback();
    long mImage;
};
