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

#include <jni.h>

#include <filament/Engine.h>

#include <backend/Platform.h>

#include <utils/CString.h>

#include <algorithm>
#include <array>

using namespace filament;

namespace {

constexpr std::array<backend::Platform::DeviceInfoType, 3> VULKAN_INFO = {
    backend::Platform::DeviceInfoType::VULKAN_DRIVER_NAME,
    backend::Platform::DeviceInfoType::VULKAN_DEVICE_NAME,
    backend::Platform::DeviceInfoType::VULKAN_DRIVER_INFO,
};

constexpr std::array<backend::Platform::DeviceInfoType, 3> GL_INFO = {
    backend::Platform::DeviceInfoType::OPENGL_VENDOR,
    backend::Platform::DeviceInfoType::OPENGL_RENDERER,
    backend::Platform::DeviceInfoType::OPENGL_VERSION,
};

} // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_google_android_filament_utils_DeviceUtils_nGetGpuDriverInfo(JNIEnv* env, jclass,
                jlong nativeEngine) {
    auto emptyStr = [env]() { return env->NewStringUTF(""); };
    Engine* engine = (Engine*) nativeEngine;
    if (!engine) {
        return emptyStr();
    }

    backend::Platform* platform = engine->getPlatform();
    if (!platform) {
        return emptyStr();
    }

    std::array<backend::Platform::DeviceInfoType, 3> infoTypes;
    switch (engine->getBackend()) {
        case backend::Backend::VULKAN:
            infoTypes = VULKAN_INFO;
            break;
        case backend::Backend::OPENGL:
            infoTypes = GL_INFO;
            break;
        default:
            return emptyStr();
    }

    backend::Driver* driver = const_cast<backend::Driver*>(engine->getDriver());
    utils::CString fullInfo;
    std::for_each(infoTypes.begin(), infoTypes.end(),
            [&](backend::Platform::DeviceInfoType infoType) {
                utils::CString const newInfo = platform->getDeviceInfo(infoType, driver);
                if (!newInfo.empty()) {
                    if (!fullInfo.empty()) {
                        fullInfo += " | ";
                    }
                    fullInfo += newInfo.c_str();
                }
            });
    return env->NewStringUTF(fullInfo.c_str());
}
