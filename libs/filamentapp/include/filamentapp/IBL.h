/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SAMPLE_IBL_H
#define TNT_FILAMENT_SAMPLE_IBL_H

#include <filament/Texture.h>

#include <math/vec3.h>

#include <string>

namespace filament {
class Engine;
class IndexBuffer;
class IndirectLight;
class Material;
class MaterialInstance;
class Renderable;
class Texture;
class Skybox;
}

namespace utils {
    class Path;
}

class IBL {
public:
    explicit IBL(filament::Engine& engine);
    ~IBL();

    bool loadFromEquirect(const utils::Path& path);
    bool loadFromDirectory(const utils::Path& path);
    bool loadFromKtx(const std::string& prefix);

    filament::IndirectLight* getIndirectLight() const noexcept {
        return mIndirectLight;
    }

    filament::Skybox* getSkybox() const noexcept {
        return mSkybox;
    }

    filament::Texture* getFogTexture() const noexcept {
        return mFogTexture;
    }

    bool hasSphericalHarmonics() const { return mHasSphericalHarmonics; }
    filament::math::float3 const* getSphericalHarmonics() const { return mBands; }

private:
    bool loadCubemapLevel(filament::Texture** texture, const utils::Path& path,
            size_t level = 0, std::string const& levelPrefix = "") const;


    bool loadCubemapLevel(filament::Texture** texture,
            filament::Texture::PixelBufferDescriptor* outBuffer,
            uint32_t* dim,
            const utils::Path& path,
            size_t level = 0, std::string const& levelPrefix = "") const;

    filament::Engine& mEngine;

    filament::math::float3 mBands[9] = {};
    bool mHasSphericalHarmonics = false;

    filament::Texture* mTexture = nullptr;
    filament::IndirectLight* mIndirectLight = nullptr;
    filament::Texture* mSkyboxTexture = nullptr;
    filament::Texture* mFogTexture = nullptr;
    filament::Skybox* mSkybox = nullptr;
};

#endif // TNT_FILAMENT_SAMPLE_IBL_H
