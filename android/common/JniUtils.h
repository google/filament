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

#include <filament/TextureSampler.h>

#include <utils/algorithm.h>
#include <utils/compiler.h>
#include <utils/Panic.h>

#include <jni.h>

#include <cstring>
#include <exception>
#include <type_traits>

namespace filament::JniUtils {

inline jint to_int(TextureSampler const& sampler) noexcept {
    return jint(utils::bit_cast<uint32_t>(sampler.getSamplerParams()));
}

inline TextureSampler from_int(jint params) noexcept {
    return TextureSampler{
            utils::bit_cast<backend::SamplerParams>(
                    static_cast<uint32_t>(params))};
}

inline jlong to_long(TextureSampler const& sampler) noexcept {
    return jlong(to_int(sampler));
}

inline TextureSampler from_long(jlong params) noexcept {
    return from_int(static_cast<jint>(params));
}

template<typename T>
inline jshort to_short(T const& value) noexcept {
    static_assert(sizeof(T) <= sizeof(uint16_t));
    static_assert(std::is_trivially_copyable_v<T>);
    uint16_t bits = 0;
    std::memcpy(&bits, &value, sizeof(T));
    return static_cast<jshort>(bits);
}

template<typename T>
inline T from_short(jshort bits) noexcept {
    static_assert(sizeof(T) <= sizeof(uint16_t));
    static_assert(std::is_trivially_copyable_v<T>);
    T value{};
    auto ubits = static_cast<uint16_t>(bits);
    std::memcpy(&value, &ubits, sizeof(T));
    return value;
}

template<typename T>
inline jint to_int(T const& value) noexcept {
    static_assert(sizeof(T) <= sizeof(uint32_t));
    static_assert(std::is_trivially_copyable_v<T>);
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(T));
    return static_cast<jint>(bits);
}

template<typename T>
inline T from_int(jint bits) noexcept {
    static_assert(sizeof(T) <= sizeof(uint32_t));
    static_assert(std::is_trivially_copyable_v<T>);
    T value{};
    auto ubits = static_cast<uint32_t>(bits);
    std::memcpy(&value, &ubits, sizeof(T));
    return value;
}

template<>
inline TextureSampler from_int<TextureSampler>(jint params) noexcept {
    return from_int(params);
}

template<typename T>
inline jlong to_long(T const& value) noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t));
    static_assert(std::is_trivially_copyable_v<T>);
    uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(T));
    return static_cast<jlong>(bits);
}

template<typename T>
inline T from_long(jlong bits) noexcept {
    static_assert(sizeof(T) <= sizeof(uint64_t));
    static_assert(std::is_trivially_copyable_v<T>);
    T value{};
    auto ubits = static_cast<uint64_t>(bits);
    std::memcpy(&value, &ubits, sizeof(T));
    return value;
}

template<>
inline TextureSampler from_long<TextureSampler>(jlong params) noexcept {
    return from_long(params);
}

} // namespace filament::JniUtils

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
