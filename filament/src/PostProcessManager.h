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

#include "private/backend/DriverApiForward.h"

#include "fg/FrameGraphResource.h"

#include <backend/DriverEnums.h>
#include <filament/View.h>

namespace filament {

namespace details {
class FMaterial;
class FMaterialInstance;
class FEngine;
class FView;
class RenderPass;
struct CameraInfo;
} // namespace details

class PostProcessManager {
public:
    explicit PostProcessManager(details::FEngine& engine) noexcept;

    void init() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;
    void setSource(uint32_t viewportWidth, uint32_t viewportHeight,
            backend::Handle<backend::HwTexture> color,
            backend::Handle<backend::HwTexture> depth,
            uint32_t textureWidth, uint32_t textureHeight) const noexcept;

    FrameGraphResource toneMapping(FrameGraph& fg, FrameGraphResource input,
            backend::TextureFormat outFormat, bool dithering, bool translucent) noexcept;

    FrameGraphResource fxaa(
            FrameGraph& fg, FrameGraphResource input, backend::TextureFormat outFormat,
            bool translucent) noexcept;

    FrameGraphResource dynamicScaling(
            FrameGraph& fg, FrameGraphResource input, backend::TextureFormat outFormat) noexcept;

    FrameGraphResource resolve(
            FrameGraph& fg, FrameGraphResource input) noexcept;


    FrameGraphResource ssao(FrameGraph& fg, details::RenderPass& pass,
            filament::Viewport const& svp,
            details::CameraInfo const& cameraInfo,
            View::AmbientOcclusionOptions const& options) noexcept;

    backend::Handle<backend::HwTexture> getNoSSAOTexture() const {
        return mNoSSAOTexture;
    }

private:
    details::FEngine& mEngine;

    FrameGraphResource depthPass(FrameGraph& fg, details::RenderPass& pass,
            uint32_t width, uint32_t height, View::AmbientOcclusionOptions const& options) noexcept;

    FrameGraphResource mipmapPass(FrameGraph& fg, FrameGraphResource input, size_t level) noexcept;

    FrameGraphResource blurPass(FrameGraph& fg,
            FrameGraphResource input, FrameGraphResource depth, math::int2 axis) noexcept;

    // we need only one of these
    mutable UniformBuffer mPostProcessUb;
    backend::Handle<backend::HwSamplerGroup> mPostProcessSbh;
    backend::Handle<backend::HwUniformBuffer> mPostProcessUbh;

    class PostProcessMaterial {
    public:
        PostProcessMaterial() noexcept = default;
        PostProcessMaterial(details::FEngine& engine, uint8_t const* data, size_t size) noexcept;

        PostProcessMaterial(PostProcessMaterial const& rhs) = delete;
        PostProcessMaterial& operator=(PostProcessMaterial const& rhs) = delete;

        PostProcessMaterial(PostProcessMaterial&& rhs) noexcept;
        PostProcessMaterial& operator=(PostProcessMaterial&& rhs) noexcept;

        ~PostProcessMaterial();

        void terminate(details::FEngine& engine) noexcept;

        details::FMaterial* getMaterial() const { return mMaterial; }
        details::FMaterialInstance* getMaterialInstance() const { return mMaterialInstance; }
        backend::Handle<backend::HwProgram> const& getProgram() const { return mProgram; }

    private:
        details::FMaterial* mMaterial = nullptr;
        details::FMaterialInstance* mMaterialInstance = nullptr;
        backend::Handle<backend::HwProgram> mProgram;
    };

    PostProcessMaterial mSSAO;
    PostProcessMaterial mMipmapDepth;
    PostProcessMaterial mBlur;

    backend::Handle<backend::HwTexture> mNoSSAOTexture;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
