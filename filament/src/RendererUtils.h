/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_RENDERERUTILS_H
#define TNT_FILAMENT_DETAILS_RENDERERUTILS_H

#include "PostProcessManager.h"
#include "RenderPass.h"

#include "fg/FrameGraphId.h"

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <math/vec4.h>

#include <stdint.h>

#include <utility>

namespace filament {

class FRenderTarget;
class FrameGraph;
class FrameGraph;
class FView;

class RendererUtils {
public:

    struct ColorPassConfig {
        // Rendering viewport (e.g. scaled down viewport from dynamic resolution)
        Viewport physicalViewport;
        // Logical viewport (e.g. left-bottom non-zero when we have guard bands), origin
        // relative to physicalViewport
        Viewport logicalViewport;
        // dynamic resolution scale
        math::float2 scale;
        // HDR format
        backend::TextureFormat hdrFormat;
        // MSAA sample count
        uint8_t msaa;
        // Clear flags
        backend::TargetBufferFlags clearFlags;
        // Clear color
        math::float4 clearColor = {};
        // Clear stencil
        uint8_t clearStencil = 0u;
        // Lod offset for the SSR passes
        float ssrLodOffset;
        // Contact shadow enabled?
        bool hasContactShadows;
        // Screen space reflections enabled
        bool hasScreenSpaceReflectionsOrRefractions;
        // Use a depth format with a stencil component.
        bool enabledStencilBuffer;
    };

    static FrameGraphId<FrameGraphTexture> colorPass(
            FrameGraph& fg, const char* name, FEngine& engine, FView const& view,
            FrameGraphTexture::Descriptor const& colorBufferDesc,
            ColorPassConfig const& config,
            PostProcessManager::ColorGradingConfig colorGradingConfig,
            RenderPass::Executor const& passExecutor) noexcept;

    static std::pair<FrameGraphId<FrameGraphTexture>, bool> refractionPass(
            FrameGraph& fg, FEngine& engine, FView const& view,
            ColorPassConfig config,
            PostProcessManager::ScreenSpaceRefConfig const& ssrConfig,
            PostProcessManager::ColorGradingConfig colorGradingConfig,
            RenderPass const& pass) noexcept;

    static void readPixels(backend::DriverApi& driver,
            backend::Handle<backend::HwRenderTarget> renderTargetHandle,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            backend::PixelBufferDescriptor&& buffer);
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERERUTILS_H
