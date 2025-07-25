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

#ifndef TNT_FILAMENT_POSTPROCESSMANAGER_H
#define TNT_FILAMENT_POSTPROCESSMANAGER_H

#include "backend/DriverApiForward.h"

#include "FrameHistory.h"
#include "MaterialInstanceManager.h"

#include "ds/PostProcessDescriptorSet.h"
#include "ds/SsrPassDescriptorSet.h"
#include "ds/StructureDescriptorSet.h"
#include "ds/TypedUniformBuffer.h"

#include "materials/StaticMaterialInfo.h"

#include <fg/FrameGraphId.h>
#include <fg/FrameGraphResources.h>
#include <fg/FrameGraphTexture.h>

#include <filament/Options.h>
#include <filament/Viewport.h>

#include <private/filament/EngineEnums.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <utils/Slice.h>

#include <tsl/robin_map.h>

#include <array>
#include <random>
#include <string_view>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FColorGrading;
class FEngine;
class FMaterial;
class FMaterialInstance;
class FrameGraph;
class RenderPass;
class RenderPassBuilder;
struct CameraInfo;

class PostProcessManager {
public:

    using StaticMaterialInfo = filament::StaticMaterialInfo;

    struct ColorGradingConfig {
        bool asSubpass{};
        bool customResolve{};
        bool translucent{};
        bool outputLuminance{}; // Whether to output luminance in the alpha channel. Ignored by the TRANSLUCENT variant.
        bool dithering{};
        backend::TextureFormat ldrFormat{};
    };

    struct StructurePassConfig {
        float scale = 0.5f;
        bool picking{};
    };

    explicit PostProcessManager(FEngine& engine) noexcept;
    ~PostProcessManager() noexcept;

    void init() noexcept;
    void terminate(backend::DriverApi& driver) noexcept;

    void configureTemporalAntiAliasingMaterial(
            TemporalAntiAliasingOptions const& taaOptions) noexcept;

    // methods below are ordered relative to their position in the pipeline (as much as possible)

    // structure (depth) pass
    struct StructurePassOutput {
        FrameGraphId<FrameGraphTexture> structure;
        FrameGraphId<FrameGraphTexture> picking;
    };
    StructurePassOutput structure(FrameGraph& fg,
            RenderPassBuilder const& passBuilder, uint8_t structureRenderFlags,
            uint32_t width, uint32_t height, StructurePassConfig const& config) noexcept;

    FrameGraphId<FrameGraphTexture> transparentPicking(FrameGraph& fg,
            RenderPassBuilder const& passBuilder, uint8_t structureRenderFlags,
            uint32_t width, uint32_t height, float scale) noexcept;

    // reflections pass
    FrameGraphId<FrameGraphTexture> ssr(FrameGraph& fg,
            RenderPassBuilder const& passBuilder,
            FrameHistory const& frameHistory,
            CameraInfo const& cameraInfo,
            FrameGraphId<FrameGraphTexture> structure,
            ScreenSpaceReflectionsOptions const& options,
            FrameGraphTexture::Descriptor const& desc) noexcept;

    // SSAO
    FrameGraphId<FrameGraphTexture> screenSpaceAmbientOcclusion(FrameGraph& fg,
            Viewport const& svp, const CameraInfo& cameraInfo,
            FrameGraphId<FrameGraphTexture> depth,
            AmbientOcclusionOptions const& options) noexcept;

    // Gaussian mipmap
    FrameGraphId<FrameGraphTexture> generateGaussianMipmap(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, size_t levels, bool reinhard,
            size_t kernelWidth, float sigma) noexcept;

    struct ScreenSpaceRefConfig {
        // The SSR texture (i.e. the 2d Array)
        FrameGraphId<FrameGraphTexture> ssr;
        // handle to subresource to receive the refraction
        FrameGraphId<FrameGraphTexture> refraction;
        // handle to subresource to receive the reflections
        FrameGraphId<FrameGraphTexture> reflection;
        float lodOffset;            // LOD offset
        uint8_t roughnessLodCount;  // LOD count
        uint8_t kernelSize;         // Kernel size
        float sigma0;               // sigma0
    };

    /*
     * Create the 2D array that will receive the reflection and refraction buffers
     */
    static ScreenSpaceRefConfig prepareMipmapSSR(FrameGraph& fg,
            uint32_t width, uint32_t height, backend::TextureFormat format,
            float verticalFieldOfView, math::float2 scale) noexcept;

    /*
     * Helper to generate gaussian mipmaps for SSR (refraction and reflections).
     * This performs the following tasks:
     *  - resolves input if needed
     *  - optionally duplicates the input
     *  - rescale input, so it has a homogenous scale
     *  - generate a new texture with gaussian mips
     */
    static FrameGraphId<FrameGraphTexture> generateMipmapSSR(
            PostProcessManager& ppm, FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> output,
            bool needInputDuplication, ScreenSpaceRefConfig const& config) noexcept;

    // Depth-of-field
    FrameGraphId<FrameGraphTexture> dof(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth,
            const CameraInfo& cameraInfo,
            bool translucent,
            math::float2 bokehScale,
            const DepthOfFieldOptions& dofOptions) noexcept;

    // Bloom
    struct BloomPassOutput {
        FrameGraphId<FrameGraphTexture> bloom;
        FrameGraphId<FrameGraphTexture> flare;
    };
    BloomPassOutput bloom(FrameGraph& fg, FrameGraphId<FrameGraphTexture> input,
            backend::TextureFormat outFormat,
            BloomOptions& inoutBloomOptions,
            TemporalAntiAliasingOptions const& taaOptions,
            math::float2 scale) noexcept;

    FrameGraphId<FrameGraphTexture> flarePass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            uint32_t width, uint32_t height,
            backend::TextureFormat outFormat,
            BloomOptions const& bloomOptions) noexcept;

        // Color grading, tone mapping, dithering and bloom
    FrameGraphId<FrameGraphTexture> colorGrading(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, Viewport const& vp,
            FrameGraphId<FrameGraphTexture> bloom,
            FrameGraphId<FrameGraphTexture> flare,
            const FColorGrading* colorGrading,
            ColorGradingConfig const& colorGradingConfig,
            BloomOptions const& bloomOptions,
            VignetteOptions const& vignetteOptions) noexcept;

    void colorGradingPrepareSubpass(backend::DriverApi& driver, const FColorGrading* colorGrading,
            ColorGradingConfig const& colorGradingConfig,
            VignetteOptions const& vignetteOptions,
            uint32_t width, uint32_t height) noexcept;

    void colorGradingSubpass(backend::DriverApi& driver,
            ColorGradingConfig const& colorGradingConfig) noexcept;

    // custom MSAA resolve as subpass
    enum class CustomResolveOp { COMPRESS, UNCOMPRESS };
    void customResolvePrepareSubpass(backend::DriverApi& driver, CustomResolveOp op) noexcept;
    void customResolveSubpass(backend::DriverApi& driver) noexcept;
    FrameGraphId<FrameGraphTexture> customResolveUncompressPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> inout) noexcept;

    // Anti-aliasing
    FrameGraphId<FrameGraphTexture> fxaa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, Viewport const& vp,
            backend::TextureFormat outFormat, bool preserveAlphaChannel) noexcept;

    // Temporal Anti-aliasing
    void TaaJitterCamera(
            Viewport const& svp,
            TemporalAntiAliasingOptions const& taaOptions,
            FrameHistory& frameHistory,
            FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
            CameraInfo* inoutCameraInfo) const noexcept;

    FrameGraphId<FrameGraphTexture> taa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth,
            FrameHistory& frameHistory,
            FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
            TemporalAntiAliasingOptions const& taaOptions,
            ColorGradingConfig const& colorGradingConfig) noexcept;

    /*
     * Blit/rescaling/resolves
     */

    // high quality upscaler
    //  - when translucent, reverts to LINEAR
    //  - doesn't handle sub-resouces
    FrameGraphId<FrameGraphTexture> upscale(FrameGraph& fg, bool translucent,
            bool sourceHasLuminance, DynamicResolutionOptions dsrOptions,
            FrameGraphId<FrameGraphTexture> input, Viewport const& vp,
            FrameGraphTexture::Descriptor const& outDesc, backend::SamplerMagFilter filter) noexcept;

    FrameGraphId<FrameGraphTexture> upscaleBilinear(FrameGraph& fg,
            DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> input,
            Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
            backend::SamplerMagFilter filter) noexcept;

    FrameGraphId<FrameGraphTexture> upscaleFSR1(FrameGraph& fg,
            DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> input,
            filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> upscaleSGSR1(FrameGraph& fg, bool sourceHasLuminance,
            DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> input,
            filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc) noexcept;

    FrameGraphId<FrameGraphTexture> rcas(
            FrameGraph& fg,
            float sharpness,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphTexture::Descriptor const& outDesc,
            bool translucent);

    // color blitter using shaders
    FrameGraphId<FrameGraphTexture> blit(FrameGraph& fg, bool translucent,
            FrameGraphId<FrameGraphTexture> input,
            Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
            backend::SamplerMagFilter filterMag,
            backend::SamplerMinFilter filterMin) noexcept;

    // depth blitter using shaders
    FrameGraphId<FrameGraphTexture> blitDepth(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input) noexcept;

    // Resolves base level of input and outputs a texture from outDesc.
    // outDesc with, height, format and samples will be overridden.
    FrameGraphId<FrameGraphTexture> resolve(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input,
            FrameGraphTexture::Descriptor outDesc) noexcept;

    // Resolves base level of input and outputs a texture from outDesc.
    // outDesc with, height, format and samples will be overridden.
    FrameGraphId<FrameGraphTexture> resolveDepth(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input,
            FrameGraphTexture::Descriptor outDesc) noexcept;

    // VSM shadow mipmap pass
    FrameGraphId<FrameGraphTexture> vsmMipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t layer, size_t level,
            math::float4 clearColor) noexcept;

    FrameGraphId<FrameGraphTexture> gaussianBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> output,
            bool reinhard, size_t kernelWidth, float sigma) noexcept;

    FrameGraphId<FrameGraphTexture> debugShadowCascades(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth) noexcept;

    FrameGraphId<FrameGraphTexture> debugDisplayShadowTexture(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> shadowmap, float scale,
            uint8_t layer, uint8_t level, uint8_t channel, float power) noexcept;

    // Combine an array texture pointed to by `input` into a single image, then return it.
    // This is only useful to check the multiview rendered scene as a debugging purpose, thus this
    // is not expected to be used in normal cases.
    FrameGraphId<FrameGraphTexture> debugCombineArrayTexture(FrameGraph& fg, bool translucent,
        FrameGraphId<FrameGraphTexture> input,
        Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
        backend::SamplerMagFilter filterMag,
        backend::SamplerMinFilter filterMin) noexcept;

    backend::Handle<backend::HwTexture> getOneTexture() const;
    backend::Handle<backend::HwTexture> getZeroTexture() const;
    backend::Handle<backend::HwTexture> getOneTextureArray() const;
    backend::Handle<backend::HwTexture> getZeroTextureArray() const;

    class PostProcessMaterial {
    public:
        explicit PostProcessMaterial(StaticMaterialInfo const& info) noexcept;

        PostProcessMaterial(PostProcessMaterial const& rhs) = delete;
        PostProcessMaterial& operator=(PostProcessMaterial const& rhs) = delete;

        PostProcessMaterial(PostProcessMaterial&& rhs) noexcept;
        PostProcessMaterial& operator=(PostProcessMaterial&& rhs) noexcept;

        ~PostProcessMaterial() noexcept;

        void terminate(FEngine& engine) noexcept;

        FMaterial* getMaterial(FEngine& engine,
                PostProcessVariant variant = PostProcessVariant::OPAQUE) const noexcept;

    private:
        void loadMaterial(FEngine& engine) const noexcept;

        union {
            mutable FMaterial* mMaterial;
            uint8_t const* mData;
        };
        // mSize == 0 if mMaterial is valid, otherwise mSize > 0
        mutable uint32_t mSize{};
        // the objects' must outlive the Slice<>
        utils::Slice<StaticMaterialInfo::ConstantInfo> mConstants{};
    };

    void registerPostProcessMaterial(std::string_view name, StaticMaterialInfo const& info);

    PostProcessMaterial& getPostProcessMaterial(std::string_view name) noexcept;

    void setFrameUniforms(backend::DriverApi& driver,
            TypedUniformBuffer<PerViewUib>& uniforms) noexcept;

    void bindPostProcessDescriptorSet(backend::DriverApi& driver) const noexcept;

    backend::PipelineState getPipelineState(
            FMaterial const* ma,
            PostProcessVariant variant = PostProcessVariant::OPAQUE) const noexcept;

    void renderFullScreenQuad(FrameGraphResources::RenderPassInfo const& out,
            backend::PipelineState const& pipeline,
            backend::DriverApi& driver) const noexcept;

    void renderFullScreenQuadWithScissor(FrameGraphResources::RenderPassInfo const& out,
            backend::PipelineState const& pipeline,
            backend::Viewport scissor,
            backend::DriverApi& driver) const noexcept;

    // Helper for a common case. Don't use in a loop because retrieving the PipelineState
    // from FMaterialInstance is not trivial.
    void commitAndRenderFullScreenQuad(backend::DriverApi& driver,
            FrameGraphResources::RenderPassInfo const& out,
            FMaterialInstance const* mi,
            PostProcessVariant variant = PostProcessVariant::OPAQUE) const noexcept;

    // Sets the necessary spec constants and uniforms common to both colorGrading.mat and
    // colorGradingAsSubpass.mat.
    FMaterialInstance* configureColorGradingMaterial(
            PostProcessMaterial& material, FColorGrading const* colorGrading,
            ColorGradingConfig const& colorGradingConfig, VignetteOptions const& vignetteOptions,
            uint32_t width, uint32_t height) noexcept;

    StructureDescriptorSet& getStructureDescriptorSet() const noexcept { return mStructureDescriptorSet; }

    void resetForRender();

private:

    // Helper to get a MaterialInstance from a FMaterial
    // This currently just call FMaterial::getDefaultInstance().
    FMaterialInstance* getMaterialInstance(FMaterial const* ma) {
        return mMaterialInstanceManager.getMaterialInstance(ma);
    }

    // Helper to get a MaterialInstance from a PostProcessMaterial.
    FMaterialInstance* getMaterialInstance(FEngine& engine, PostProcessMaterial const& material,
            PostProcessVariant variant = PostProcessVariant::OPAQUE) {
        FMaterial const* ma = material.getMaterial(engine, variant);
        return getMaterialInstance(ma);
    }

    backend::RenderPrimitiveHandle mFullScreenQuadRph;
    backend::VertexBufferInfoHandle mFullScreenQuadVbih;
    backend::DescriptorSetLayoutHandle mPerRenderableDslh;

    FEngine& mEngine;

    mutable SsrPassDescriptorSet mSsrPassDescriptorSet;
    mutable PostProcessDescriptorSet mPostProcessDescriptorSet;
    mutable StructureDescriptorSet mStructureDescriptorSet;

    struct BilateralPassConfig {
        uint8_t kernelSize = 11;
        bool bentNormals = false;
        float standardDeviation = 1.0f;
        float bilateralThreshold = 0.0625f;
        float scale = 1.0f;
    };

    FrameGraphId<FrameGraphTexture> bilateralBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, FrameGraphId<FrameGraphTexture> depth,
            math::int2 axis, float zf, backend::TextureFormat format,
            BilateralPassConfig const& config) noexcept;

    FrameGraphId<FrameGraphTexture> downscalePass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphTexture::Descriptor const& outDesc,
            bool threshold, float highlight, bool fireflies) noexcept;

    using MaterialRegistryMap = tsl::robin_map<
            std::string_view,
            PostProcessMaterial>;

    MaterialRegistryMap mMaterialRegistry;

    MaterialInstanceManager mMaterialInstanceManager;

    backend::Handle<backend::HwTexture> mStarburstTexture;

    std::uniform_real_distribution<float> mUniformDistribution{0.0f, 1.0f};

    template<size_t SIZE>
    struct JitterSequence {
        math::float2 operator()(size_t const i) const noexcept { return positions[i % SIZE] - 0.5f; }
        const std::array<math::float2, SIZE> positions;
    };

    static const JitterSequence<4> sRGSS4;
    static const JitterSequence<4> sUniformHelix4;
    static const JitterSequence<32> sHaltonSamples;

    bool mWorkaroundSplitEasu : 1;
    bool mWorkaroundAllowReadOnlyAncillaryFeedbackLoop : 1;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESSMANAGER_H
