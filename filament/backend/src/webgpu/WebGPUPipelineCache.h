/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUPIPELINECACHE_H
#define TNT_FILAMENT_BACKEND_WEBGPUPIPELINECACHE_H

#include "WebGPUVertexBufferInfo.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <utils/CString.h>
#include <utils/Hash.h>

#include <tsl/robin_map.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <type_traits>
#include <vector>

namespace filament::backend {

class WebGPUPipelineCache final {
public:
    struct RenderPipelineRequest final {
        utils::CString const& label;
        wgpu::ShaderModule const& vertexShaderModule;
        wgpu::ShaderModule const& fragmentShaderModule;
        std::vector<WebGPUVertexBufferInfo::WebGPUSlotBindingInfo> const& vertexBufferSlots;
        wgpu::VertexBufferLayout const* vertexBufferLayouts;
        wgpu::PipelineLayout const& pipelineLayout;
        const PrimitiveType primitiveType;
        RasterState const& rasterState;
        StencilState const& stencilState;
        PolygonOffset const& polygonOffset;
        const TargetBufferFlags targetRenderFlags;
        const uint8_t multisampleCount;
        const wgpu::TextureFormat depthStencilFormat;
        const uint8_t colorFormatCount;
        wgpu::TextureFormat const* colorFormats;
    };

    explicit WebGPUPipelineCache(wgpu::Device const&);
    WebGPUPipelineCache(WebGPUPipelineCache const&) = delete;
    WebGPUPipelineCache(WebGPUPipelineCache const&&) = delete;
    WebGPUPipelineCache& operator=(WebGPUPipelineCache const&) = delete;
    WebGPUPipelineCache& operator=(WebGPUPipelineCache const&&) = delete;

    [[nodiscard]] wgpu::RenderPipeline const& getOrCreateRenderPipeline(
            RenderPipelineRequest const&);

    void onFrameEnd();

private:
    /**
     * Part of the pipeline key specifically about one of the vertex attributes
     */
    struct VertexAttribute final {       // size : offset (need multiples of 4 bytes for hashing)
        uint8_t bufferIndex{ 0 };        // 1    : 0
        // this is the webgpu offset,    //
        // bytes from the start of       //
        // the vertex data               //
        // (interleaved offset)          //
        uint8_t offset{ 0 };             // 1    : 1
        uint8_t shaderLocation{ 0 };     // 1    : 2
        uint8_t padding { 0 };           // 1    : 3
        wgpu::VertexFormat format{ 0 };  // 4    : 4
    };
    static_assert(sizeof(VertexAttribute) == 8, "VertexAttribute must not have implicit padding.");
    static_assert(std::is_trivially_copyable<VertexAttribute>::value,
            "VertexAttribute must be a trivially copyable POD for fast hashing.");
    /**
     * Part of the pipeline key specifically about one of the vertex buffers
     */
    struct VertexBuffer final {  // size : offset (need multiples of 4 bytes for hashing)
        uint8_t stride{ 0 };     // 1    : 0
        uint8_t padding[3]{ 0 }; // 3    : 1
        // offset in bytes from  //
        // the start of the      //
        // (physical) GPU buffer //
        // (not logical buffer   //
        //  partition)           //
        uint32_t offset{ 0 };    // 4    : 4
    };
    static_assert(sizeof(VertexBuffer) == 8, "VertexBuffer must not have implicit padding.");
    static_assert(std::is_trivially_copyable<VertexBuffer>::value,
            "VertexAttribute must be a trivially copyable POD for fast hashing.");
    /**
     * Key designed for efficient hashing and uniquely identifying all the parameters for
     * creating a render pipeline.
     * The efficient hashing requires a small memory footprint
     * (using the smallest representations of enums, just handle instances instead of wrapper class
     *  instances, single bytes for booleans etc.), trivial copying and comparison (byte by byte),
     *  and a word-aligned structure with a size in bytes as a multiple of 4 (for murmer hash).
     */
    struct RenderPipelineKey final {                                                        // size : offset (need multiples of 4 bytes for hashing)
        // shaders...                                                                       //
        WGPUShaderModule vertexShaderModuleHandle{ nullptr };                               // 8    : 0
        WGPUShaderModule fragmentShaderModuleHandle{ nullptr };                             // 8    : 8
        // vertex attributes...                                                             //
        VertexAttribute vertexAttributes[MAX_VERTEX_ATTRIBUTE_COUNT]{};                     // 128  : 16
        VertexBuffer vertexBuffers[MAX_VERTEX_BUFFER_COUNT]{};                              // 128  : 144
        // pipeline layout...                                                               //
        WGPUPipelineLayout pipelineLayoutHandle{ nullptr };                                 // 8    : 272
        // general settings...                                                              //
        int32_t depthBias{ 0 };                                                             // 4    : 280
        float depthBiasSlopeScale { 0.0f };                                                 // 4    : 284
        PrimitiveType primitiveType{ PrimitiveType::POINTS };                               // 1    : 288
        // stencil state...                                                                 //
        SamplerCompareFunc stencilFrontCompare{ SamplerCompareFunc::LE };                   // 1    : 289
        StencilOperation stencilFrontFailOperation{ StencilOperation::KEEP };               // 1    : 290
        StencilOperation stencilFrontDepthFailOperation{ StencilOperation::KEEP };          // 1    : 291
        StencilOperation stencilFrontPassOperation{ StencilOperation::KEEP };               // 1    : 292
        /* bool, 0 -> false, 1 -> true */ uint8_t stencilWrite{ 0 };                        // 1    : 293
        uint8_t stencilFrontReadMask{ 0 };                                                  // 1    : 294
        uint8_t stencilFrontWriteMask{ 0 };                                                 // 1    : 295
        SamplerCompareFunc stencilBackCompare{ SamplerCompareFunc::LE };                    // 1    : 296
        StencilOperation stencilBackFailOperation{ StencilOperation::KEEP };                // 1    : 297
        StencilOperation stencilBackDepthFailOperation{ StencilOperation::KEEP };           // 1    : 298
        StencilOperation stencilBackPassOperation{ StencilOperation::KEEP };                // 1    : 299
        // general rasterization settings...                                                //
        CullingMode cullingMode{ CullingMode::NONE };                                       // 1    : 300
        /* bool, 0 -> counter-clockwise, 1 -> clockwise  */ uint8_t inverseFrontFaces{ 0 }; // 1    : 301
        // rasterization depth state...                                                     //
        /* bool, 0 -> false, 1 -> true */ uint8_t depthWriteEnabled{ 0 };                   // 1    : 302
        SamplerCompareFunc depthCompare{ SamplerCompareFunc::LE };                          // 1    : 303
        /* bool, 0 -> false, 1 -> true */ uint8_t depthClamp{ 0 };                          // 1    : 304
        // more rasterization flags...                                                      //
        /* bool, 0 -> false, 1 -> true */ uint8_t colorWrite{ 0 };                          // 1    : 305
        /* bool, 0 -> false, 1 -> true */ uint8_t alphaToCoverageEnabled{ 0 };              // 1    : 306
        // color blending...                                                                //
        BlendEquation colorBlendOperation{ BlendEquation::ADD };                            // 1    : 307
        BlendFunction colorBlendSourceFactor{ BlendFunction::ZERO };                        // 1    : 308
        BlendFunction colorBlendDestinationFactor{ BlendFunction::ZERO };                   // 1    : 309
        // alpha blending...                                                                //
        BlendEquation alphaBlendOperation{ BlendEquation::ADD };                            // 1    : 310
        BlendFunction alphaBlendSourceFactor{ BlendFunction::ZERO };                        // 1    : 311
        BlendFunction alphaBlendDestinationFactor{ BlendFunction::ZERO };                   // 1    : 312
        // render targets...                                                                //
        uint8_t multisampleCount{ 0 };                                                      // 1    : 313
        uint8_t colorFormatCount{ 0 };                                                      // 1    : 314
        uint8_t padding[5]{ 0 };                                                            // 5    : 319
        TargetBufferFlags targetRenderFlags{ TargetBufferFlags::NONE };                     // 4    : 320
        wgpu::TextureFormat depthStencilFormat { wgpu::TextureFormat::Undefined };          // 4    : 324
        wgpu::TextureFormat colorFormats[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT]{           //
            wgpu::TextureFormat::Undefined                                              //
        };                                                                                  // 32   : 328
    };
    static_assert(sizeof(RenderPipelineKey) == 360,
            "RenderPipelineKey must not have implicit padding.");
    static_assert(std::is_trivially_copyable<RenderPipelineKey>::value,
            "RenderPipelineKey must be a trivially copyable POD for fast hashing.");

    struct RenderPipelineKeyEqual {
        bool operator()(RenderPipelineKey const&, RenderPipelineKey const&) const;
    };

    struct RenderPipelineCacheEntry final {
        wgpu::RenderPipeline pipeline{ nullptr };
        uint64_t lastUsedFrameCount{ 0 };
    };

    static void populateKey(RenderPipelineRequest const&, RenderPipelineKey& outKey);

    [[nodiscard]] wgpu::RenderPipeline createRenderPipeline(RenderPipelineRequest const&);

    void removeExpiredPipelines();

    wgpu::Device mDevice;
    tsl::robin_map<RenderPipelineKey, RenderPipelineCacheEntry,
            utils::hash::MurmurHashFn<RenderPipelineKey>, RenderPipelineKeyEqual>
            mRenderPipelines{};
    uint64_t mFrameCount{ 0 };
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUPIPELINECACHE_H
