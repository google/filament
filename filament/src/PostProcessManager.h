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

#include "FrameHistory.h"

#include <fg/FrameGraphHandle.h>

#include <backend/DriverEnums.h>
#include <filament/View.h>

#include <utils/CString.h>

#include <tsl/robin_map.h>

#include <random>

namespace filament {

class FrameGraph;
class FColorGrading;
class FEngine;
class FMaterial;
class FMaterialInstance;
class FView;
class RenderPass;
struct CameraInfo;

class PostProcessManager {
public:
    struct ColorGradingConfig {
        bool asSubpass = false;
        bool translucent{};
        bool fxaa{};
        bool dithering{};
        backend::TextureFormat ldrFormat{};
    };

    explicit PostProcessManager(FEngine& engine) noexcept;

    void init() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;

    // methods below are ordered relative to their position in the pipeline (as much as possible)

    // structure (depth) pass
    FrameGraphId<FrameGraphTexture> structure(FrameGraph& fg, RenderPass const& pass,
            uint32_t width, uint32_t height, float scale) noexcept;

    // SSAO
    FrameGraphId<FrameGraphTexture> screenSpaceAmbientOcclusion(FrameGraph& fg,
            RenderPass& pass, filament::Viewport const& svp,
            CameraInfo const& cameraInfo,
            View::AmbientOcclusionOptions options) noexcept;

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

    // Color grading, tone mapping, etc.
    void colorGradingPrepareSubpass(backend::DriverApi& driver, const FColorGrading* colorGrading,
            View::VignetteOptions vignetteOptions, bool fxaa, bool dithering,
            uint32_t width, uint32_t height) noexcept;

    void colorGradingSubpass(backend::DriverApi& driver, bool translucent) noexcept;

    FrameGraphId<FrameGraphTexture> colorGrading(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, const FColorGrading* colorGrading,
            backend::TextureFormat outFormat, bool translucent, bool fxaa, math::float2 scale,
            View::BloomOptions bloomOptions, View::VignetteOptions vignetteOptions, bool dithering) noexcept;

    // Anti-aliasing
    FrameGraphId<FrameGraphTexture> fxaa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            bool translucent) noexcept;

    // Temporal Anti-aliasing
    void prepareTaa(FrameHistory& frameHistory,
            CameraInfo const& cameraInfo,
            View::TemporalAntiAliasingOptions const& taaOptions) const noexcept;

    FrameGraphId<FrameGraphTexture> taa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, FrameHistory& frameHistory,
            View::TemporalAntiAliasingOptions taaOptions,
            ColorGradingConfig colorGradingConfig) noexcept;

    // Blit/rescaling/resolves
    FrameGraphId<FrameGraphTexture> opaqueBlit(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc,
            backend::SamplerMagFilter filter = backend::SamplerMagFilter::LINEAR) noexcept;

    FrameGraphId<FrameGraphTexture> blendBlit(
            FrameGraph& fg, bool translucent, View::QualityLevel quality,
            FrameGraphId<FrameGraphTexture> input, FrameGraphTexture::Descriptor outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> resolve(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept;

    // VSM shadow mipmap pass
    FrameGraphId<FrameGraphTexture> vsmMipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t layer, size_t level) noexcept;

    backend::Handle<backend::HwTexture> getOneTexture() const { return mDummyOneTexture; }
    backend::Handle<backend::HwTexture> getZeroTexture() const { return mDummyZeroTexture; }
    backend::Handle<backend::HwTexture> getOneTextureArray() const { return mDummyOneTextureArray; }

    math::float2 halton(size_t index) const noexcept {
        return mHaltonSamples[index & 0xFu];
    }

private:
    FEngine& mEngine;
    class PostProcessMaterial;

    FrameGraphId<FrameGraphTexture> mipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t level) noexcept;

    struct BilateralPassConfig {
        uint8_t kernelSize = 11;
        float standardDeviation = 1.0f;
        float bilateralThreshold = 0.0625f;
        float scale = 1.0f;
    };

    FrameGraphId<FrameGraphTexture> bilateralBlurPass(
            FrameGraph& fg, FrameGraphId<FrameGraphTexture> input, math::int2 axis, float zf,
            backend::TextureFormat format, BilateralPassConfig config) noexcept;

    FrameGraphId<FrameGraphTexture> gaussianBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t srcLevel,
            FrameGraphId<FrameGraphTexture> output, uint8_t dstLevel,
            bool reinhard, size_t kernelWidth, float sigma = 6.0f) noexcept;

    FrameGraphId<FrameGraphTexture> bloomPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            View::BloomOptions& bloomOptions, math::float2 scale) noexcept;

    FrameGraphId<FrameGraphTexture> bloomPassPingPong(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            View::BloomOptions& bloomOptions, math::float2 scale) noexcept;

    void commitAndRender(FrameGraphRenderTarget const& out,
            PostProcessMaterial const& material, uint8_t variant,
            backend::DriverApi& driver) const noexcept;

    void commitAndRender(FrameGraphRenderTarget const& out,
            PostProcessMaterial const& material,
            backend::DriverApi& driver) const noexcept;

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

        backend::PipelineState getPipelineState(uint8_t variant = 0u) const noexcept;

    private:
        FMaterial* assertMaterial() const noexcept;
        FMaterial* loadMaterial() const noexcept;

        union {
            struct {
                mutable FMaterial* mMaterial;
            };
            struct {
                FEngine* mEngine;
                uint8_t const* mData;
            };
        };
        uint32_t mSize{};
        mutable bool mHasMaterial{};
    };

    tsl::robin_map<utils::StaticString, PostProcessMaterial> mMaterialRegistry;

    void registerPostProcessMaterial(utils::StaticString name, uint8_t const* data, int size);
    PostProcessMaterial& getPostProcessMaterial(utils::StaticString name) noexcept;

    backend::Handle<backend::HwTexture> mDummyOneTexture;
    backend::Handle<backend::HwTexture> mDummyOneTextureArray;
    backend::Handle<backend::HwTexture> mDummyZeroTexture;

    size_t mSeparableGaussianBlurKernelStorageSize = 0;

    std::uniform_real_distribution<float> mUniformDistribution{0.0f, 1.0f};

    const math::float2 mHaltonSamples[16];
    bool mDisableFeedbackLoops;
};


} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESS_MANAGER_H
