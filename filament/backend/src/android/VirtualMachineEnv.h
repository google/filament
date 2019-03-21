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
#include <utils/ThreadLocal.h>

#include <jni.h>

#include <assert.h>

namespace filament {

class VirtualMachineEnv {
public:
    static jint JNI_OnLoad(JavaVM* vm) noexcept;

    static VirtualMachineEnv& get() noexcept {
        // declaring this thread local, will ensure it's destroyed with the calling thread
        static UTILS_DECLARE_TLS(VirtualMachineEnv)
        instance;
        return instance;
    }

    static JNIEnv* getThreadEnvironment() noexcept {
        JNIEnv* env;
        assert(sVirtualMachine);
        if (sVirtualMachine->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return nullptr; // this should not happen
        }
        return env;
    }

    VirtualMachineEnv() noexcept : mVirtualMachine(sVirtualMachine) {
        // We're not initializing the JVM here -- but we could -- because most of the time
        // we don't need the jvm. Instead we do the initialization on first use. This means we could get
        // a nasty slow down the very first time, but we'll live with it for now.
    }

    ~VirtualMachineEnv() {
        if (mVirtualMachine) {
            mVirtualMachine->DetachCurrentThread();
        }
    }

    inline JNIEnv* getEnvironment() noexcept {
        assert(mVirtualMachine);
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    static void handleException(JNIEnv* env) noexcept;

private:
    JNIEnv* getEnvironmentSlow() noexcept;
    static JavaVM* sVirtualMachine;
    JNIEnv* mJniEnv = nullptr;
    JavaVM* mVirtualMachine = nullptr;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_ANDROID_VIRTUAL_MACHINE_ENV_H
