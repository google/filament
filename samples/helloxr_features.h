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

#ifndef HELLOXR_FEATURES_H
#define HELLOXR_FEATURES_H

// Shared preamble for helloxr and its optional feature modules. The include order below matters:
// BlueVK pulls in vulkan.h with VK_NO_PROTOTYPES and, on Windows, windows.h; openxr_platform.h
// expects both to already be present, plus jni.h on Android.

#if defined(_WIN32)
#define XR_USE_PLATFORM_WIN32
#elif defined(__ANDROID__)
#define XR_USE_PLATFORM_ANDROID
#endif
#define XR_USE_GRAPHICS_API_VULKAN

// Must precede everything else: Filament's math headers use std::sqrt and friends without including
// <cmath>, and they are reachable transitively from BlueVK.
#include <cmath>

#include <bluevk/BlueVK.h>

#if defined(__ANDROID__)
#include <android/log.h>
#include <jni.h>
#endif

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec3.h>

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#if defined(__ANDROID__)
#define XRLOG(...) __android_log_print(ANDROID_LOG_INFO, "helloxr", __VA_ARGS__)
#else
#define XRLOG(...) do { printf("[helloxr] " __VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
#endif

namespace filament {
class Engine;
class Material;
class Scene;
} // namespace filament

namespace helloxr {

// Everything a feature module needs from the host application.
struct FeatureContext {
    XrInstance instance = XR_NULL_HANDLE;
    XrSession session = XR_NULL_HANDLE;
    XrSpace appSpace = XR_NULL_HANDLE;
    filament::Engine* engine = nullptr;
    filament::Scene* scene = nullptr;
    // The sample's lit material, shared so a module does not have to carry its own copy.
    filament::Material* material = nullptr;
    // When set, modules may write debug artifacts using this as a file name prefix.
    std::string dumpPrefix;
};

// An optional capability that renders extra content into the scene. Features are constructed before
// the OpenXR instance exists so they can declare the extensions they need, then initialized once
// there is a session, and torn down before the Engine goes away.
class Feature {
public:
    virtual ~Feature();

    virtual char const* name() const = 0;

    // Requested only when the runtime advertises them; a feature whose extensions are missing is
    // never initialized.
    virtual std::vector<char const*> requiredExtensions() const = 0;

    virtual bool initialize(FeatureContext const& context) = 0;

    virtual void update(XrTime displayTime) = 0;

    virtual void terminate() = 0;
};

std::unique_ptr<Feature> createRenderModels();
std::unique_ptr<Feature> createHandMeshes();
std::unique_ptr<Feature> createVertexStreaming();

filament::math::mat4 poseToMat4(XrPosef const& pose);

// Android routes asset reads through the APK, so the host app hands over the manager up front.
void setAssetManager(void* assetManager);

// Reads an APK asset on Android, a plain file elsewhere.
bool readAsset(std::string const& name, std::vector<uint8_t>* out);

// Always reads the filesystem, on every platform.
bool readFile(std::string const& path, std::vector<uint8_t>* out);

} // namespace helloxr

#endif // HELLOXR_FEATURES_H
