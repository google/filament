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

class FColorGrading;
class FEngine;
class FMaterial;
class FMaterialInstance;
class FView;
class RenderPass;
struct CameraInfo;

class PostProcessManager {
public:
    explicit PostProcessManager(FEngine& engine) noexcept;

    void init() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;

    // methods below are ordered relative to their position in the pipeline (as much as possible)

    // structure (depth) pass
    FrameGraphId<FrameGraphTexture> structure(FrameGraph& fg, RenderPass const& pass,
            uint32_t width, uint32_t height, float scale) noexcept;

    // SSAO
    FrameGraphId<FrameGraphTexture> screenSpaceAmbientOclusion(FrameGraph& fg,
            RenderPass& pass, filament::Viewport const& svp,
            CameraInfo const& cameraInfo,
            View::AmbientOcclusionOptions const& options) noexcept;

    // Used in refraction pass
    FrameGraphId<FrameGraphTexture> generateGaussianMipmap(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t roughnessLodCount, bool reinhard,
            size_t kernelWidth, float sigmaRatio = 6.0f) noexcept;

    // Depth-of-field
    FrameGraphId<FrameGraphTexture> dof(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            const View::DepthOfFieldOptions& dofOptions,
            bool translucent,
            const CameraInfo& cameraInfo) noexcept;

    // Tone mapping
    void colorGradingSubpass(backend::DriverApi& driver, const FColorGrading* colorGrading,
            bool translucent, bool fxaa, bool dithering) noexcept;

    FrameGraphId<FrameGraphTexture> colorGrading(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, const FColorGrading* colorGrading,
            backend::TextureFormat outFormat, bool translucent, bool fxaa, math::float2 scale,
            View::BloomOptions bloomOptions, bool dithering) noexcept;

    // Anti-aliasing
    FrameGraphId<FrameGraphTexture> fxaa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            bool translucent) noexcept;

    // Blit/rescaling/resolves
    FrameGraphId<FrameGraphTexture> opaqueBlit(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc,
            backend::SamplerMagFilter filter = backend::SamplerMagFilter::LINEAR) noexcept;

    FrameGraphId<FrameGraphTexture> blendBlit(
            FrameGraph& fg, bool translucent, View::QualityLevel quality,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> resolve(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept;

    backend::Handle<backend::HwTexture> getOneTexture() const { return mDummyOneTexture; }
    backend::Handle<backend::HwTexture> getZeroTexture() const { return mDummyZeroTexture; }

private:
    FEngine& mEngine;

    FrameGraphId<FrameGraphTexture> mipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t level) noexcept;

    FrameGraphId<FrameGraphTexture> bilateralBlurPass(
            FrameGraph& fg, FrameGraphId<FrameGraphTexture> input, math::int2 axis, float zf,
            backend::TextureFormat format) noexcept;

    FrameGraphId<FrameGraphTexture> gaussianBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t srcLevel,
            FrameGraphId<FrameGraphTexture> output, uint8_t dstLevel,
            bool reinhard, size_t kernelWidth, float sigma = 6.0f) noexcept;

    FrameGraphId<FrameGraphTexture> bloomPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            View::BloomOptions& bloomOptions, math::float2 scale) noexcept;


    class PostProcessMaterial {
    public:
        PostProcessMaterial() noexcept;
        PostProcessMaterial(FEngine& engine, uint8_t const* data, int size) noexcept;

        PostProcessMaterial(PostProcessMaterial const& rhs) = delete;
        PostProcessMaterial& operator=(PostProcessMaterial const& rhs) = delete;

        PostProcessMaterial(PostProcessMaterial&& rhs) noexcept;
        PostProcessMaterial& operator=(PostProcessMaterial&& rhs) noexcept;

        ~PostProcessMaterial();

        void terminate(FEngine& engine) noexcept;

        FMaterial* getMaterial() const;
        FMaterialInstance* getMaterialInstance() const;

        backend::PipelineState getPipelineState(uint8_t variant) const noexcept;
        backend::PipelineState getPipelineState() const noexcept;

    private:
        void assertMaterial() const noexcept;

        union {
            struct {
                mutable FMaterial* mMaterial;
                mutable FMaterialInstance* mMaterialInstance;
            };
            struct {
                FEngine* mEngine;
                uint8_t const* mData;
            };
        };
        uint32_t mSize{};
        mutable bool mHasMaterial{};
    };

    PostProcessMaterial mMipmapDepth;

    PostProcessMaterial mSSAO;
    PostProcessMaterial mBilateralBlur;
    PostProcessMaterial mSeparableGaussianBlur;
    PostProcessMaterial mDoFDownsample;
    PostProcessMaterial mDoFMipmap;
    PostProcessMaterial mDoF;
    PostProcessMaterial mDoFTiles;
    PostProcessMaterial mDoFDilate;
    PostProcessMaterial mDoFCombine;
    PostProcessMaterial mBloomDownsample;
    PostProcessMaterial mBloomUpsample;
    PostProcessMaterial mColorGradingAsSubpass;
    PostProcessMaterial mColorGrading;
    PostProcessMaterial mFxaa;
    PostProcessMaterial mBlit[3];

    backend::Handle<backend::HwTexture> mDummyOneTexture;
    backend::Handle<backend::HwTexture> mDummyZeroTexture;

    size_t mSeparableGaussianBlurKernelStorageSize = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
