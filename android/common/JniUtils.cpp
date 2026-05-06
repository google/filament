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

#include "JniUtils.h"

namespace filament {
namespace android {

#ifdef __EXCEPTIONS

UTILS_NOINLINE void wrapJniHelper(JNIEnv* env, void (*invoker)(void*), void* userData) {
    try {
        invoker(userData);
    } catch (const utils::PreconditionPanic& e) {
        jclass exClass = env->FindClass("java/lang/IllegalArgumentException");
        env->ThrowNew(exClass, e.what());
    } catch (const utils::PostconditionPanic& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, e.what());
    } catch (const std::exception& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, e.what());
    }
}

UTILS_NOINLINE void wrapJniBackendHelper(JNIEnv* env, void (*invoker)(void*), void* userData) {
    try {
        invoker(userData);
    } catch (const std::exception& e) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, e.what());
    }
}

#endif

} // namespace android
} // namespace filament
