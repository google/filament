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

#ifndef ANDROID_ASSET_LOADER_H
#define ANDROID_ASSET_LOADER_H

#include <filamentapp/AssetLoader.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class AndroidAssetLoader : public filament::app::AssetLoader {
public:
    explicit AndroidAssetLoader(AAssetManager* assetManager) : mAssetManager(assetManager) {}
    ~AndroidAssetLoader() override = default;

    std::vector<uint8_t> load(utils::Path const& path) const override {
        std::vector<uint8_t> buffer;
        if (!mAssetManager) return buffer;

        // Strip "assets/" prefix if it exists, since AAssetManager implicitly starts inside the assets folder
        std::string pathStr = path.c_str();
        if (pathStr.rfind("assets/", 0) == 0) {
            pathStr = pathStr.substr(7);
        }

        AAsset* asset = AAssetManager_open(mAssetManager, pathStr.c_str(), AASSET_MODE_BUFFER);
        if (asset) {
            size_t length = AAsset_getLength(asset);
            buffer.resize(length);
            AAsset_read(asset, buffer.data(), length);
            AAsset_close(asset);
        }
        return buffer;
    }

private:
    AAssetManager* mAssetManager;
};

#endif // ANDROID_ASSET_LOADER_H
