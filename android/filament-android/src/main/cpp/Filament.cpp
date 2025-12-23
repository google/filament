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

#include <jni.h>

#include "private/backend/VirtualMachineEnv.h"
#include "../../../../common/JniExceptionBridge.h"
#include "../../../../common/ThreadExceptionBridge.h"
#include "../../../../common/PanicHandler.h"

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    // This must be called when the library is loaded. We need this to get a reference to the
    // global VM
    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);

    // Initialize main thread detection for exception handling
    ::filament::android::MainThreadDetector::initialize();

    // Register panic handler for worker thread recovery
    ::filament::android::PanicHandler::initialize();

    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_android_filament_Filament_nHealthCheck(JNIEnv* env, jclass) {
    ::filament::android::throwStoredExceptionIfAny(env);
}
