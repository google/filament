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

#include "common/CallbackUtils.h"
#include "common/NioUtils.h"

#include <filament/Engine.h>

struct JniBufferCallback {
    static JniBufferCallback* make(filament::Engine* engine,
            JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer);

    static void invoke(void* buffer, size_t n, void* user);

private:
    JniBufferCallback(JNIEnv* env, jobject handler, jobject callback, AutoBuffer&& buffer);
    ~JniBufferCallback();

    JNIEnv* mEnv;
    jobject mHandler;
    jobject mCallback;
    AutoBuffer mBuffer;
};

struct JniImageCallback {
    static JniImageCallback* make(filament::Engine* engine, JNIEnv* env, jobject handler,
            jobject runnable, long image);

    static void invoke(void* image, void* user);

private:
    JniImageCallback(JNIEnv* env, jobject handler, jobject runnable, long image);
    ~JniImageCallback();

    JNIEnv* mEnv;
    jobject mHandler;
    jobject mCallback;
    long mImage;
};
