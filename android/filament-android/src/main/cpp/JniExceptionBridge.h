#pragma once
#include <jni.h>
#include <string>
#include <type_traits>
#include <utils/Panic.h>   // Filament's panic base type
#include "PanicState.h"

namespace filament::android {

// Prefer a dedicated Java exception type if you add one.
// For now, use IllegalStateException.
inline void throwIllegalState(JNIEnv* env, const std::string& msg) {
    jclass cls = env->FindClass("java/lang/IllegalStateException");
    if (!cls) {
        env->ExceptionClear(); // avoid recursive exception states
        return;
    }
    env->ThrowNew(cls, msg.c_str());
}

inline void throwFromPanic(JNIEnv* env, const utils::Panic& p, const char* where) {
    const std::string msg = std::string("Filament panic @ ") + (where ? where : "?") + ": " + p.what();
    PanicState::report(where, p.what());   // poison / disable
    throwIllegalState(env, msg);
}

inline void throwFromStd(JNIEnv* env, const std::exception& e, const char* where) {
    const std::string msg = std::string("Native exception @ ") + (where ? where : "?") + ": " + e.what();
    PanicState::report(where, e.what());   // treat as fatal for Filament usage
    throwIllegalState(env, msg);
}

inline void throwUnknown(JNIEnv* env, const char* where) {
    const std::string msg = std::string("Unknown native exception @ ") + (where ? where : "?");
    PanicState::report(where, "unknown native exception");
    throwIllegalState(env, msg);
}

template <typename Ret, typename Fn>
inline Ret jniGuard(JNIEnv* env, const char* where, Ret defaultValue, Fn&& fn) {
    if (PanicState::isDisabled()) {
        throwIllegalState(env, "Filament is disabled due to a previous fatal error.");
        return defaultValue;
    }

    try {
        return fn();
    } catch (const utils::Panic& p) {
        throwFromPanic(env, p, where);
    } catch (const std::exception& e) {
        throwFromStd(env, e, where);
    } catch (...) {
        throwUnknown(env, where);
    }
    return defaultValue;
}

template <typename Fn>
inline void jniGuardVoid(JNIEnv* env, const char* where, Fn&& fn) {
    if (PanicState::isDisabled()) {
        throwIllegalState(env, "Filament is disabled due to a previous fatal error.");
        return;
    }

    try {
        fn();
    } catch (const utils::Panic& p) {
        throwFromPanic(env, p, where);
    } catch (const std::exception& e) {
        throwFromStd(env, e, where);
    } catch (...) {
        throwUnknown(env, where);
    }
}

} // namespace filament::android
