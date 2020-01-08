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

#include "fg/FrameGraphHandle.h"

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

    FrameGraphId <FrameGraphTexture> toneMapping(FrameGraph& fg,
            FrameGraphId <FrameGraphTexture> input,
            backend::TextureFormat outFormat, bool dithering, bool translucent, bool fxaa) noexcept;

    FrameGraphId<FrameGraphTexture> fxaa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            bool translucent) noexcept;

    FrameGraphId <FrameGraphTexture> dynamicScaling(
            FrameGraph& fg, uint8_t msaa, bool scaled, bool blend,
            FrameGraphId <FrameGraphTexture> input,
            backend::TextureFormat outFormat) noexcept;

    FrameGraphId<FrameGraphTexture> ssao(FrameGraph& fg, details::RenderPass& pass,
            filament::Viewport const& svp,
            details::CameraInfo const& cameraInfo,
            View::AmbientOcclusionOptions const& options) noexcept;

    backend::Handle<backend::HwTexture> getNoSSAOTexture() const {
        return mNoSSAOTexture;
    }

private:
    details::FEngine& mEngine;

    FrameGraphId<FrameGraphTexture> depthPass(FrameGraph& fg, details::RenderPass const& pass,
            uint32_t width, uint32_t height, View::AmbientOcclusionOptions const& options) noexcept;

    FrameGraphId<FrameGraphTexture> mipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t level) noexcept;

    FrameGraphId<FrameGraphTexture> blurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth, math::int2 axis) noexcept;

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
    PostProcessMaterial mBlit;
    PostProcessMaterial mTonemapping;
    PostProcessMaterial mFxaa;

    backend::Handle<backend::HwTexture> mNoSSAOTexture;
    backend::Handle<backend::HwTexture> mNoiseTexture;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
