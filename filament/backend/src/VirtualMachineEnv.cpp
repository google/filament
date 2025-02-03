/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "private/backend/VirtualMachineEnv.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <jni.h>

#include <mutex>

namespace filament {

using namespace utils;

// This Mutex shouldn't be subject to the Static Initialization Order Fiasco because its initial state is
// a single int initialized to 0.
/* static*/ Mutex VirtualMachineEnv::sLock;
/* static*/ JavaVM* VirtualMachineEnv::sVirtualMachine = nullptr;

UTILS_NOINLINE
JavaVM* VirtualMachineEnv::getVirtualMachine() {
    std::lock_guard const lock(sLock);
    assert_invariant(sVirtualMachine);
    return sVirtualMachine;
}

/*
 * This is typically called by filament_jni.so when it is loaded. If filament_jni.so is not used,
 * then this must be called manually -- however, this is a problem because VirtualMachineEnv.h
 * is currently private and part of backend.
 * For now, we authorize this usage, but we will need to fix it; by making a proper public
 * API for this.
 */
UTILS_PUBLIC
UTILS_NOINLINE
jint VirtualMachineEnv::JNI_OnLoad(JavaVM* vm) noexcept {
    std::lock_guard const lock(sLock);
    if (sVirtualMachine) {
        // It doesn't make sense for JNI_OnLoad() to be called more than once
        return JNI_VERSION_1_6;
    }

    // Here we check this VM at least has JNI_VERSION_1_6
    JNIEnv* env = nullptr;
    jint const result = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

    FILAMENT_CHECK_POSTCONDITION(result == JNI_OK)
            << "Couldn't get JniEnv* from the VM, error = " << result;

    sVirtualMachine = vm;
    return JNI_VERSION_1_6;
}

UTILS_NOINLINE
VirtualMachineEnv& VirtualMachineEnv::get() noexcept {
    JavaVM* const vm = getVirtualMachine();
    // declaring this thread local, will ensure it's destroyed with the calling thread
    thread_local VirtualMachineEnv instance{ vm };
    return instance;
}

UTILS_NOINLINE
JNIEnv* VirtualMachineEnv::getThreadEnvironment() noexcept {
    JavaVM* const vm = getVirtualMachine();
    JNIEnv* env = nullptr;
    jint const result = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);

    FILAMENT_CHECK_POSTCONDITION(result == JNI_OK)
            << "Couldn't get JniEnv* from the VM, error = " << result;

    return env;
}

VirtualMachineEnv::VirtualMachineEnv(JavaVM* vm) noexcept : mVirtualMachine(vm) {
    // We're not initializing the JVM here -- but we could -- because most of the time
    // we don't need the jvm. Instead, we do the initialization on first use. This means we could get
    // a nasty slow down the very first time, but we'll live with it for now.
}

VirtualMachineEnv::~VirtualMachineEnv() noexcept {
    if (mVirtualMachine) {
        mVirtualMachine->DetachCurrentThread();
    }
}

UTILS_NOINLINE
JNIEnv* VirtualMachineEnv::getEnvironmentSlow() noexcept {
    FILAMENT_CHECK_PRECONDITION(mVirtualMachine)
            << "JNI_OnLoad() has not been called";

#if defined(__ANDROID__)
    jint const result = mVirtualMachine->AttachCurrentThread(&mJniEnv, nullptr);
#else
    jint const result = mVirtualMachine->AttachCurrentThread(reinterpret_cast<void**>(&mJniEnv), nullptr);
#endif

    FILAMENT_CHECK_POSTCONDITION(result == JNI_OK)
            << "JavaVM::AttachCurrentThread failed with error " << result;

    FILAMENT_CHECK_POSTCONDITION(mJniEnv)
            << "JavaVM::AttachCurrentThread returned a null mJniEnv";

    return mJniEnv;
}

UTILS_NOINLINE
void VirtualMachineEnv::handleException(JNIEnv* const env) noexcept {
    if (UTILS_UNLIKELY(env->ExceptionCheck())) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

} // namespace filament

