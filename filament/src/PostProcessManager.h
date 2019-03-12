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

#ifndef TNT_FILAMENT_POSTPROCESS_MANAGER_H
#define TNT_FILAMENT_POSTPROCESS_MANAGER_H

#include "UniformBuffer.h"

#include "driver/DriverApiForward.h"

#include "fg/FrameGraphResource.h"

#include <filament/driver/DriverEnums.h>

namespace filament {

namespace details {
class FEngine;
class FView;
} // namespace details

class PostProcessManager {
public:
    void init(details::FEngine& engine) noexcept;
    void terminate(driver::DriverApi& driver) noexcept;
    void setSource(uint32_t viewportWidth, uint32_t viewportHeight, Handle<HwTexture> texture,
            uint32_t textureWidth, uint32_t textureHeight) const noexcept;

    void setDithering(bool dithering) noexcept { mDithering = dithering; }

    FrameGraphResource toneMapping(
            FrameGraph& fg, FrameGraphResource input, driver::TextureFormat outFormat,
            bool translucent) noexcept;

    FrameGraphResource fxaa(
            FrameGraph& fg, FrameGraphResource input, driver::TextureFormat outFormat,
            bool translucent) noexcept;

    FrameGraphResource dynamicScaling(
            FrameGraph& fg, FrameGraphResource input, driver::TextureFormat outFormat) noexcept;

private:
    details::FEngine* mEngine = nullptr;

    // we need only one of these
    mutable UniformBuffer mPostProcessUb;
    Handle<HwSamplerGroup> mPostProcessSbh;
    Handle<HwUniformBuffer> mPostProcessUbh;
    bool mDithering = true;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
