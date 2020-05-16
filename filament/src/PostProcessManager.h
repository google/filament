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

class FMaterial;
class FMaterialInstance;
class FEngine;
class FView;
class RenderPass;
struct CameraInfo;

class PostProcessManager {
public:
    explicit PostProcessManager(FEngine& engine) noexcept;

    void init() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;

    FrameGraphId <FrameGraphTexture> toneMapping(FrameGraph& fg,
            FrameGraphId <FrameGraphTexture> input,
            backend::TextureFormat outFormat, bool translucent, bool fxaa, math::float2 scale,
            View::BloomOptions bloomOptions, bool dithering) noexcept;

    FrameGraphId<FrameGraphTexture> fxaa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            bool translucent) noexcept;

    FrameGraphId<FrameGraphTexture> dof(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            const View::DepthOfFieldOptions& dofOptions,
            const CameraInfo& cameraInfo) noexcept;

    FrameGraphId<FrameGraphTexture> opaqueBlit(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> blendBlit(
            FrameGraph& fg, bool translucent, View::QualityLevel quality,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> resolve(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept;

    FrameGraphId<FrameGraphTexture> structure(FrameGraph& fg, RenderPass const& pass,
            uint32_t width, uint32_t height, float scale) noexcept;

    FrameGraphId<FrameGraphTexture> screenSpaceAmbientOclusion(FrameGraph& fg,
            RenderPass& pass, filament::Viewport const& svp,
            CameraInfo const& cameraInfo,
            View::AmbientOcclusionOptions const& options) noexcept;

    FrameGraphId<FrameGraphTexture> generateGaussianMipmap(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t roughnessLodCount, bool reinhard,
            size_t kernelWidth, float sigmaRatio = 6.0f) noexcept;

    FrameGraphId<FrameGraphTexture> gaussianBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t srcLevel,
            FrameGraphId<FrameGraphTexture> output, uint8_t dstLevel,
            bool reinhard, size_t kernelWidth, float sigma = 6.0f) noexcept;

    backend::Handle<backend::HwTexture> getOneTexture() const { return mDummyOneTexture; }
    backend::Handle<backend::HwTexture> getZeroTexture() const { return mDummyZeroTexture; }

private:
    FEngine& mEngine;

    FrameGraphId<FrameGraphTexture> mipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t level) noexcept;

    FrameGraphId<FrameGraphTexture> bilateralBlurPass(
            FrameGraph& fg, FrameGraphId<FrameGraphTexture> input, math::int2 axis, float zf,
            backend::TextureFormat format) noexcept;

    FrameGraphId<FrameGraphTexture> bloomPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            View::BloomOptions& bloomOptions, math::float2 scale) noexcept;


    class PostProcessMaterial {
    public:
        PostProcessMaterial() noexcept = default;
        PostProcessMaterial(FEngine& engine, uint8_t const* data, size_t size) noexcept;

        PostProcessMaterial(PostProcessMaterial const& rhs) = delete;
        PostProcessMaterial& operator=(PostProcessMaterial const& rhs) = delete;

        PostProcessMaterial(PostProcessMaterial&& rhs) noexcept;
        PostProcessMaterial& operator=(PostProcessMaterial&& rhs) noexcept;

        ~PostProcessMaterial();

        void terminate(FEngine& engine) noexcept;

        FMaterial* getMaterial() const { return mMaterial; }
        FMaterialInstance* getMaterialInstance() const { return mMaterialInstance; }

        backend::PipelineState getPipelineState(uint8_t variant) const noexcept;
        backend::PipelineState getPipelineState() const noexcept;

    private:
        FMaterial* mMaterial = nullptr;
        FMaterialInstance* mMaterialInstance = nullptr;
        backend::Handle<backend::HwProgram> mProgram;
    };

    PostProcessMaterial mSSAO;
    PostProcessMaterial mMipmapDepth;
    PostProcessMaterial mBilateralBlur;
    PostProcessMaterial mSeparableGaussianBlur;
    PostProcessMaterial mBloomDownsample;
    PostProcessMaterial mBloomUpsample;
    PostProcessMaterial mBlit[3];
    PostProcessMaterial mTonemapping;
    PostProcessMaterial mFxaa;
    PostProcessMaterial mDoFBlur;
    PostProcessMaterial mDoF;

    backend::Handle<backend::HwTexture> mDummyOneTexture;
    backend::Handle<backend::HwTexture> mDummyZeroTexture;

    size_t mSeparableGaussianBlurKernelStorageSize = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
