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

#ifndef TNT_ANDROID_COMMON_JNIUTILS_H
#define TNT_ANDROID_COMMON_JNIUTILS_H

#include <jni.h>
#include <exception>
#include <type_traits>
#include <utils/Panic.h>
#include <utils/compiler.h>

namespace filament {
namespace android {

#ifdef __EXCEPTIONS

// Non-templated helpers implemented in JniUtils.cpp
void wrapJniHelper(JNIEnv* env, void (*invoker)(void*), void* userData);
void wrapJniBackendHelper(JNIEnv* env, void (*invoker)(void*), void* userData);

// For JNI methods that return a value
template<typename R, typename F>
R wrapJni(JNIEnv* env, F const& f) {
    if constexpr (std::is_void_v<R>) {
        auto invoker = [](void* data) {
            auto& f_ref = *reinterpret_cast<const F*>(data);
            f_ref();
        };
        wrapJniHelper(env, invoker, (void*)&f);
    } else {
        struct Context {
            const F& f;
            R result;
        };
        Context ctx{ f, R{} };
        auto invoker = [](void* data) {
            auto& context = *reinterpret_cast<Context*>(data);
            context.result = context.f();
        };
        wrapJniHelper(env, invoker, &ctx);
        return ctx.result;
    }
}

// Overload for JNI methods that return void
template<typename F>
inline void wrapJni(JNIEnv* env, F const& f) {
    wrapJni<void, F>(env, f);
}

// For JNI methods that can return backend errors (mapped to java.lang.Error)
template<typename R, typename F>
R wrapJniBackend(JNIEnv* env, F const& f) {
    if constexpr (std::is_void_v<R>) {
        auto invoker = [](void* data) {
            auto& f_ref = *reinterpret_cast<const F*>(data);
            f_ref();
        };
        wrapJniBackendHelper(env, invoker, (void*)&f);
    } else {
        struct Context {
            const F& f;
            R result;
        };
        Context ctx{ f, R{} };
        auto invoker = [](void* data) {
            auto& context = *reinterpret_cast<Context*>(data);
            context.result = context.f();
        };
        wrapJniBackendHelper(env, invoker, &ctx);
        return ctx.result;
    }
}

template<typename F>
inline void wrapJniBackend(JNIEnv* env, F const& f) {
    wrapJniBackend<void, F>(env, f);
}

#else

// For JNI methods that return a value
template<typename R, typename F>
R wrapJni(JNIEnv* env, F const& f) {
    return f();
}

// Overload for JNI methods that return void
template<typename F>
inline void wrapJni(JNIEnv* env, F const& f) {
    f();
}

template<typename R, typename F>
R wrapJniBackend(JNIEnv* env, F const& f) {
    return f();
}

template<typename F>
inline void wrapJniBackend(JNIEnv* env, F const& f) {
    f();
}

#endif

} // namespace android
} // namespace filament

#endif // TNT_ANDROID_COMMON_JNIUTILS_H
