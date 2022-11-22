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

#include "private/backend/VirtualMachineEnv.h"

void acquireCallbackJni(JNIEnv* env, CallbackJni& callbackUtils) {
#ifdef __ANDROID__
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
#ifdef __ANDROID__
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
#ifdef __ANDROID__
    env->DeleteGlobalRef(callbackUtils.handlerClass);
#endif
    env->DeleteGlobalRef(callbackUtils.executorClass);
}

// -----------------------------------------------------------------------------------------------

JniCallback* JniCallback::make(JNIEnv* env, jobject handler, jobject callback) {
    return new JniCallback(env, handler, callback);
}

JniCallback::JniCallback(JNIEnv* env, jobject handler, jobject callback)
        : mHandler(env->NewGlobalRef(handler)),
          mCallback(env->NewGlobalRef(callback)) {
    acquireCallbackJni(env, mCallbackUtils);
}

JniCallback::~JniCallback() = default;

void JniCallback::post(void* user, filament::backend::CallbackHandler::Callback callback) {
    callback(user);
}

void JniCallback::postToJavaAndDestroy(JniCallback* callback) {
    JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
    releaseCallbackJni(env, callback->mCallbackUtils, callback->mHandler, callback->mCallback);
    delete callback;
}

// -----------------------------------------------------------------------------------------------

JniBufferCallback* JniBufferCallback::make(filament::Engine*,
        JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer) {
    return new JniBufferCallback(env, handler, callback, std::move(buffer));
}

JniBufferCallback::JniBufferCallback(JNIEnv* env, jobject handler, jobject callback,
        AutoBuffer&& buffer)
        : JniCallback(env, handler, callback),
        mBuffer(std::move(buffer)) {
}

JniBufferCallback::~JniBufferCallback() = default;

void JniBufferCallback::postToJavaAndDestroy(void*, size_t, void* user) {
    JniBufferCallback* callback = (JniBufferCallback*)user;
    JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
    callback->mBuffer.attachToJniThread(env);
    releaseCallbackJni(env, callback->mCallbackUtils, callback->mHandler, callback->mCallback);
    delete callback;
}

// -----------------------------------------------------------------------------------------------

JniImageCallback* JniImageCallback::make(filament::Engine*,
        JNIEnv* env, jobject handler, jobject callback, long image) {
    return new JniImageCallback(env, handler, callback, image);
}

JniImageCallback::JniImageCallback(JNIEnv* env, jobject handler, jobject callback, long image)
        : JniCallback(env, handler, callback),
        mImage(image) {
}

JniImageCallback::~JniImageCallback() = default;

void JniImageCallback::postToJavaAndDestroy(void*, void* user) {
    JniImageCallback* callback = (JniImageCallback*)user;
    JNIEnv* env = filament::VirtualMachineEnv::get().getEnvironment();
    releaseCallbackJni(env, callback->mCallbackUtils, callback->mHandler, callback->mCallback);
    delete callback;
}
