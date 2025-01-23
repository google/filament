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

#ifndef TNT_FILAMENT_DRIVER_ANDROID_VIRTUAL_MACHINE_ENV_H
#define TNT_FILAMENT_DRIVER_ANDROID_VIRTUAL_MACHINE_ENV_H

#include <utils/compiler.h>
#include <utils/Mutex.h>

#include <jni.h>

namespace filament {

class VirtualMachineEnv {
public:
    // must be called before VirtualMachineEnv::get() from a thread that is attached to the JavaVM
    static jint JNI_OnLoad(JavaVM* vm) noexcept;

    // must be called on backend thread
    static VirtualMachineEnv& get() noexcept;

    // can be called from any thread that already has a JniEnv
    static JNIEnv* getThreadEnvironment() noexcept;

    // must be called from the backend thread
    JNIEnv* getEnvironment() noexcept {
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    static void handleException(JNIEnv* env) noexcept;

private:
    explicit VirtualMachineEnv(JavaVM* vm) noexcept;
    ~VirtualMachineEnv() noexcept;
    JNIEnv* getEnvironmentSlow() noexcept;

    static utils::Mutex sLock;
    static JavaVM* sVirtualMachine;
    static JavaVM* getVirtualMachine();

    JNIEnv* mJniEnv = nullptr;
    JavaVM* mVirtualMachine = nullptr;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_ANDROID_VIRTUAL_MACHINE_ENV_H
