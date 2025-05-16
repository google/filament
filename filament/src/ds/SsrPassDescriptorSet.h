/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SSRPASSDESCRIPTORSET_H
#define TNT_FILAMENT_SSRPASSDESCRIPTORSET_H

#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"

#include "TypedUniformBuffer.h"

#include <private/filament/UibStructs.h>

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <math/mat4.h>

namespace filament {

class FEngine;

struct ScreenSpaceReflectionsOptions;

class SsrPassDescriptorSet {

    using TextureHandle = backend::Handle<backend::HwTexture>;

public:
    SsrPassDescriptorSet() noexcept;

    void init(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    void setFrameUniforms(FEngine const& engine, TypedUniformBuffer<PerViewUib>& uniforms) noexcept;

    void prepareStructure(FEngine const& engine, TextureHandle structure) noexcept;

    void prepareHistorySSR(FEngine const& engine, TextureHandle ssr,
            math::mat4f const& historyProjection,
            math::mat4f const& uvFromViewMatrix,
            ScreenSpaceReflectionsOptions const& ssrOptions) noexcept;


    // update local data into GPU UBO
    void commit(FEngine& engine) noexcept;

    // bind this descriptor set
    void bind(backend::DriverApi& driver) noexcept;

private:
    TypedUniformBuffer<PerViewUib>* mUniforms = nullptr;
    DescriptorSet mDescriptorSet;
    backend::BufferObjectHandle mShadowUbh;
};

} // namespace filament

#endif //TNT_FILAMENT_SSRPASSDESCRIPTORSET_H
