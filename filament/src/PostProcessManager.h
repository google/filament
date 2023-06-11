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

#include <fg/FrameGraphId.h>
#include <fg/FrameGraphResources.h>

#include <filament/Options.h>

#include <backend/DriverEnums.h>
#include <backend/PipelineState.h>

#include <private/filament/Variant.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <tsl/robin_map.h>

#include <random>
#include <string_view>
#include <variant>

namespace filament {

class FColorGrading;
class FEngine;
class FMaterial;
class FMaterialInstance;
class FrameGraph;
class PerViewUniforms;
class RenderPass;
struct CameraInfo;

class PostProcessManager {
public:

    struct ConstantInfo {
        std::string_view name;
        std::variant<int32_t, float, bool> value;
    };

    struct MaterialInfo {
        std::string_view name;
        uint8_t const* data;
        int size;
        utils::FixedCapacityVector<ConstantInfo> constants = {};
    };

    struct ColorGradingConfig {
        bool asSubpass{};
        bool customResolve{};
        bool translucent{};
        bool fxaa{};
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

    // methods below are ordered relative to their position in the pipeline (as much as possible)

    // structure (depth) pass
    struct StructurePassOutput {
        FrameGraphId<FrameGraphTexture> structure;
        FrameGraphId<FrameGraphTexture> picking;
    };
    StructurePassOutput structure(FrameGraph& fg,
            RenderPass const& pass, uint8_t structureRenderFlags,
            uint32_t width, uint32_t height, StructurePassConfig const& config) noexcept;

    // reflections pass
    FrameGraphId<FrameGraphTexture> ssr(FrameGraph& fg,
            RenderPass const& pass,
            FrameHistory const& frameHistory,
            CameraInfo const& cameraInfo,
            PerViewUniforms& uniforms,
            FrameGraphId<FrameGraphTexture> structure,
            ScreenSpaceReflectionsOptions const& options,
            FrameGraphTexture::Descriptor const& desc) noexcept;

    // SSAO
    FrameGraphId<FrameGraphTexture> screenSpaceAmbientOcclusion(FrameGraph& fg,
            filament::Viewport const& svp, const CameraInfo& cameraInfo,
            FrameGraphId<FrameGraphTexture> structure,
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
            float bokehAspectRatio,
            const DepthOfFieldOptions& dofOptions) noexcept;

    // Bloom
    struct BloomPassOutput {
        FrameGraphId<FrameGraphTexture> bloom;
        FrameGraphId<FrameGraphTexture> flare;
    };
    BloomPassOutput bloom(FrameGraph& fg, FrameGraphId<FrameGraphTexture> input,
            BloomOptions& inoutBloomOptions, backend::TextureFormat outFormat,
            math::float2 scale) noexcept;

    FrameGraphId<FrameGraphTexture> flarePass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            uint32_t width, uint32_t height,
            backend::TextureFormat outFormat,
            BloomOptions const& bloomOptions) noexcept;

        // Color grading, tone mapping, dithering and bloom
    FrameGraphId<FrameGraphTexture> colorGrading(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
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
            FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
            backend::TextureFormat outFormat, bool translucent) noexcept;

    // Temporal Anti-aliasing
    void prepareTaa(FrameGraph& fg, filament::Viewport const& svp,
            FrameHistory& frameHistory,
            FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
            CameraInfo* inoutCameraInfo,
            PerViewUniforms& uniforms) const noexcept;

    FrameGraphId<FrameGraphTexture> taa(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth,
            FrameHistory& frameHistory,
            FrameHistoryEntry::TemporalAA FrameHistoryEntry::*pTaa,
            TemporalAntiAliasingOptions const& taaOptions,
            ColorGradingConfig const& colorGradingConfig) noexcept;

    // Blit/rescaling/resolves
    FrameGraphId<FrameGraphTexture> opaqueBlit(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
            FrameGraphTexture::Descriptor const& outDesc,
            backend::SamplerMagFilter filter = backend::SamplerMagFilter::LINEAR) noexcept;

    FrameGraphId<FrameGraphTexture> upscale(FrameGraph& fg, bool translucent,
            DynamicResolutionOptions dsrOptions, FrameGraphId<FrameGraphTexture> input,
            filament::Viewport const& vp, FrameGraphTexture::Descriptor const& outDesc,
            backend::SamplerMagFilter filter = backend::SamplerMagFilter::LINEAR) noexcept;

    FrameGraphId<FrameGraphTexture> blit(FrameGraph& fg, bool translucent,
            FrameGraphId<FrameGraphTexture> input, filament::Viewport const& vp,
            FrameGraphTexture::Descriptor const& outDesc,
            backend::SamplerMagFilter filter = backend::SamplerMagFilter::LINEAR) noexcept;

    // resolve base level of input and outputs a 1-level texture
    FrameGraphId<FrameGraphTexture> resolveBaseLevel(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input) noexcept;

    // resolves base level of input and outputs a texture from outDesc. outDesc must
    // have the same dimensions and format as input, or this will fail.
    // outDesc can have mipmaps.
    FrameGraphId<FrameGraphTexture> resolveBaseLevelNoCheck(FrameGraph& fg,
            const char* outputBufferName, FrameGraphId<FrameGraphTexture> input,
            FrameGraphTexture::Descriptor const& outDesc) noexcept;

    // VSM shadow mipmap pass
    FrameGraphId<FrameGraphTexture> vsmMipmapPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, uint8_t layer, size_t level,
            math::float4 clearColor, bool finalize) noexcept;

    FrameGraphId<FrameGraphTexture> gaussianBlurPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> output,
            bool reinhard, size_t kernelWidth, float sigma) noexcept;

    FrameGraphId<FrameGraphTexture> debugShadowCascades(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input,
            FrameGraphId<FrameGraphTexture> depth) noexcept;

    backend::Handle<backend::HwTexture> getOneTexture() const;
    backend::Handle<backend::HwTexture> getZeroTexture() const;
    backend::Handle<backend::HwTexture> getOneTextureArray() const;
    backend::Handle<backend::HwTexture> getZeroTextureArray() const;

    math::float2 halton(size_t index) const noexcept {
        return mHaltonSamples[index & 0xFu];
    }

private:
    FEngine& mEngine;
    class PostProcessMaterial;

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

    BloomPassOutput bloomPass(FrameGraph& fg,
            FrameGraphId<FrameGraphTexture> input, backend::TextureFormat outFormat,
            BloomOptions& inoutBloomOptions, math::float2 scale) noexcept;

    void commitAndRender(FrameGraphResources::RenderPassInfo const& out,
            PostProcessMaterial const& material, uint8_t variant,
            backend::DriverApi& driver) const noexcept;

    void commitAndRender(FrameGraphResources::RenderPassInfo const& out,
            PostProcessMaterial const& material,
            backend::DriverApi& driver) const noexcept;

    void render(FrameGraphResources::RenderPassInfo const& out,
            backend::PipelineState const& pipeline,
            backend::DriverApi& driver) const noexcept;

    class PostProcessMaterial {
    public:
        PostProcessMaterial() noexcept;
        PostProcessMaterial(MaterialInfo const& info) noexcept;

        PostProcessMaterial(PostProcessMaterial const& rhs) = delete;
        PostProcessMaterial& operator=(PostProcessMaterial const& rhs) = delete;

        PostProcessMaterial(PostProcessMaterial&& rhs) noexcept;
        PostProcessMaterial& operator=(PostProcessMaterial&& rhs) noexcept;

        ~PostProcessMaterial();

        void terminate(FEngine& engine) noexcept;

        FMaterial* getMaterial(FEngine& engine) const noexcept;
        FMaterialInstance* getMaterialInstance(FEngine& engine) const noexcept;

        backend::PipelineState getPipelineState(FEngine& engine,
                Variant::type_t variantKey = 0u) const noexcept;

    private:
        void loadMaterial(FEngine& engine) const noexcept;

        union {
            mutable FMaterial* mMaterial;
            uint8_t const* mData;
        };
        uint32_t mSize{};
        mutable bool mHasMaterial{};
        utils::FixedCapacityVector<ConstantInfo> mConstants{};
    };

    using MaterialRegistryMap = tsl::robin_map<
            std::string_view,
            PostProcessMaterial>;

    MaterialRegistryMap mMaterialRegistry;

    void registerPostProcessMaterial(std::string_view name, MaterialInfo const& info);
    PostProcessMaterial& getPostProcessMaterial(std::string_view name) noexcept;

    backend::Handle<backend::HwTexture> mStarburstTexture;

    std::uniform_real_distribution<float> mUniformDistribution{0.0f, 1.0f};

    static const math::float2 sHaltonSamples[16];
    math::float2 const* mHaltonSamples = sHaltonSamples;

    bool mWorkaroundSplitEasu : 1;
    bool mWorkaroundAllowReadOnlyAncillaryFeedbackLoop : 1;
};

} // namespace filament

#endif // TNT_FILAMENT_POSTPROCESSMANAGER_H
