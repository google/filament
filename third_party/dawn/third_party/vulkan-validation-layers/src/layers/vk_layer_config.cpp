/**************************************************************************
 *
 * Copyright 2014-2024 Valve Software
 * Copyright 2015-2024 Google Inc.
 * Copyright 2019-2024 LunarG, Inc.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************/
#include "vk_layer_config.h"

#include <cstring>
#include <string>
#include <cstdlib>
#include <sys/stat.h>

#include <vulkan/vk_layer.h>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "error_message/logging.h"
#include <charconv>
#include <sys/system_properties.h>
#include <unistd.h>
#include "utils/android_ndk_types.h"
#define GetCurrentDir getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#if defined(__ANDROID__)
static void PropCallback(void *cookie, [[maybe_unused]] const char *name, const char *value, [[maybe_unused]] uint32_t serial) {
    std::string *property = static_cast<std::string *>(cookie);
    *property = value;
}
#endif

std::string GetEnvironment(const char *variable) {
#if !defined(__ANDROID__) && !defined(_WIN32)
    const char *output = getenv(variable);
    return output == NULL ? "" : output;
#elif defined(_WIN32)
    int size = GetEnvironmentVariable(variable, NULL, 0);
    if (size == 0) {
        return "";
    }
    char *buffer = new char[size];
    GetEnvironmentVariable(variable, buffer, size);
    std::string output = buffer;
    delete[] buffer;
    return output;
#elif defined(__ANDROID__)
    std::string var = variable;

    if (std::string_view{variable} != kForceDefaultCallbackKey) {
        // kForceDefaultCallbackKey is a special key that needs to be recognized for backwards compatibilty.
        // For all other strings, prefix the requested variable with "debug.vvl." so that desktop environment settings can be used
        // on Android.
        var = "debug.vvl." + var;
    }

    const prop_info *prop_info = __system_property_find(var.data());

    if (prop_info) {
        std::string property;
        __system_property_read_callback(prop_info, PropCallback, &property);
        return property;
    } else {
        return "";
    }
#else
    return "";
#endif
}

void SetEnvironment(const char *variable, const char *value) {
#if !defined(__ANDROID__) && !defined(_WIN32)
    setenv(variable, value, 1);
#elif defined(_WIN32)
    SetEnvironmentVariable(variable, value);
#elif defined(__ANDROID__)
    (void)variable;
    (void)value;
    assert(false && "Not supported on android");
#endif
}

#if defined(WIN32)
// Check for admin rights
static inline bool IsHighIntegrity() {
    HANDLE process_token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_QUERY_SOURCE, &process_token)) {
        // Maximum possible size of SID_AND_ATTRIBUTES is maximum size of a SID + size of attributes DWORD.
        uint8_t mandatory_label_buffer[SECURITY_MAX_SID_SIZE + sizeof(DWORD)];
        DWORD buffer_size;
        if (GetTokenInformation(process_token, TokenIntegrityLevel, mandatory_label_buffer, sizeof(mandatory_label_buffer),
                                &buffer_size) != 0) {
            const TOKEN_MANDATORY_LABEL *mandatory_label = (const TOKEN_MANDATORY_LABEL *)mandatory_label_buffer;
            const DWORD sub_authority_count = *GetSidSubAuthorityCount(mandatory_label->Label.Sid);
            const DWORD integrity_level = *GetSidSubAuthority(mandatory_label->Label.Sid, sub_authority_count - 1);

            CloseHandle(process_token);
            return integrity_level > SECURITY_MANDATORY_MEDIUM_RID;
        }

        CloseHandle(process_token);
    }

    return false;
}
#endif

// Ensure we are properly setting VK_USE_PLATFORM_METAL_EXT, VK_USE_PLATFORM_IOS_MVK, and VK_USE_PLATFORM_MACOS_MVK.
#if __APPLE__

#ifndef VK_USE_PLATFORM_METAL_EXT
#error "VK_USE_PLATFORM_METAL_EXT not defined!"
#endif

#include <TargetConditionals.h>

#if TARGET_OS_IOS

#ifndef VK_USE_PLATFORM_IOS_MVK
#error "VK_USE_PLATFORM_IOS_MVK not defined!"
#endif

#endif  //  TARGET_OS_IOS

#if TARGET_OS_OSX

#ifndef VK_USE_PLATFORM_MACOS_MVK
#error "VK_USE_PLATFORM_MACOS_MVK not defined!"
#endif

#endif  // TARGET_OS_OSX

#endif  // __APPLE__

#ifdef VK_USE_PLATFORM_ANDROID_KHR

// Require at least NDK 25 to build Validation Layers. Makes everything simpler to just have people building the layers to use a
// recent version of the NDK.
//
// This avoids issues with older NDKs which complicate correct CMake builds:
// Example:
//
// The NDK toolchain file in r23 contains a bug which means CMAKE_ANDROID_EXCEPTIONS might not be set correctly in some
// circumstances, if not set directly by the developer.
#if __NDK_MAJOR__ < 25
#error "Validation Layers require at least NDK r25 or greater to build"
#endif

// This catches before dlopen fails if the default Android-26 layers are being used and attempted to be ran on Android 25 or below
void __attribute__((constructor)) CheckAndroidVersion() {
    const std::string version = GetEnvironment("ro.build.version.sdk");

    if (version.empty()) {
        return;
    }

    constexpr uint32_t target_android_api = 26;
    constexpr uint32_t android_api = __ANDROID_API__;

    static_assert(android_api >= target_android_api, "Vulkan-ValidationLayers is not supported on Android 25 and below");

    uint32_t queried_version{};

    if (std::from_chars(version.data(), version.data() + version.size(), queried_version).ec != std::errc()) {
        return;
    }

    if (queried_version < target_android_api) {
        __android_log_print(ANDROID_LOG_FATAL, "VALIDATION", "ERROR - Android version is %d and needs to be 26 or above.",
                            queried_version);
    }
}

#endif
