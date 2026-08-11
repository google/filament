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

#include "helloxr_features.h"

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#endif

#include <fstream>
#include <iterator>

using namespace filament::math;

namespace helloxr {
namespace {

#if defined(__ANDROID__)
AAssetManager* gAssetManager = nullptr;
#endif

} // anonymous namespace

Feature::~Feature() = default;

mat4 poseToMat4(XrPosef const& pose) {
    quat const q{ pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z };
    return mat4{ mat3{ q }, double3{ pose.position.x, pose.position.y, pose.position.z } };
}

void setAssetManager(void* assetManager) {
#if defined(__ANDROID__)
    gAssetManager = static_cast<AAssetManager*>(assetManager);
#else
    (void) assetManager;
#endif
}

bool readFile(std::string const& path, std::vector<uint8_t>* out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    *out = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
    return !out->empty();
}

bool readAsset(std::string const& name, std::vector<uint8_t>* out) {
#if defined(__ANDROID__)
    AAsset* asset = AAssetManager_open(gAssetManager, name.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        return false;
    }
    out->resize(size_t(AAsset_getLength(asset)));
    int const read = AAsset_read(asset, out->data(), out->size());
    AAsset_close(asset);
    return read == int(out->size());
#else
    return readFile(name, out);
#endif
}

} // namespace helloxr
