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

#include "VirtualMachineEnv.h"


namespace filament {

JavaVM* VirtualMachineEnv::sVirtualMachine = nullptr;

// This is called when the library is loaded. We need this to get a reference to the global VM
UTILS_NOINLINE
jint VirtualMachineEnv::JNI_OnLoad(JavaVM* vm) noexcept {
    JNIEnv* env = nullptr;
    if (UTILS_UNLIKELY(vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)) {
        // this should not happen
        return -1;
    }
    sVirtualMachine = vm;
    return JNI_VERSION_1_6;
}

UTILS_NOINLINE
void VirtualMachineEnv::handleException(JNIEnv* const env) noexcept {
    if (UTILS_UNLIKELY(env->ExceptionCheck())) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

UTILS_NOINLINE
JNIEnv* VirtualMachineEnv::getEnvironmentSlow() noexcept {
    mVirtualMachine->AttachCurrentThread(&mJniEnv, nullptr);
    assert(mJniEnv);
    return mJniEnv;
}

} // namespace filament

