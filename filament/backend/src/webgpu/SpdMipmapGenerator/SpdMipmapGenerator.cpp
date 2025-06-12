#include "SpdMipmapGenerator.h"
#include <sstream>
#include <stdexcept>
// C++ port of https://github.com/JolifantoBambla/webgpu-spd for early experiments
namespace spd {

// Helper to convert enums to strings for map keys or shader code
const char* to_string(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::RGBA8Unorm:
            return "rgba8unorm";
        case wgpu::TextureFormat::BGRA8Unorm:
            return "bgra8unorm";
        case wgpu::TextureFormat::R32Float:
            return "r32float";
        case wgpu::TextureFormat::RG32Float:
            return "rg32float";
        case wgpu::TextureFormat::RGBA32Float:
            return "rgba32float";
        case wgpu::TextureFormat::R16Float:
            return "r16float";
        case wgpu::TextureFormat::RG16Float:
            return "rg16float";
        case wgpu::TextureFormat::RGBA16Float:
            return "rgba16float";
        // Add other formats as needed
        default:
            return "rgba8unorm";
    }
}


std::string MipmapGenerator::GetFilterCode(SPDFilter filter) {
    switch (filter) {
        case SPDFilter::Min:
            return R"(
    fn spd_reduce_4(v0: vec4<SPDScalar>, v1: vec4<SPDScalar>, v2: vec4<SPDScalar>, v3: vec4<SPDScalar>) -> vec4<SPDScalar> {
        return min(min(v0, v1), min(v2, v3));
    }
    )";
        case SPDFilter::Max:
            return R"(
    fn spd_reduce_4(v0: vec4<SPDScalar>, v1: vec4<SPDScalar>, v2: vec4<SPDScalar>, v3: vec4<SPDScalar>) -> vec4<SPDScalar> {
        return max(max(v0, v1), max(v2, v3));
    }
    )";
        case SPDFilter::MinMax:
            return R"(
    fn spd_reduce_4(v0: vec4<SPDScalar>, v1: vec4<SPDScalar>, v2: vec4<SPDScalar>, v3: vec4<SPDScalar>) -> vec4<SPDScalar> {
        let max4 = max(max(v0.xy, v1.xy), max(v2.xy, v3.xy));
        return vec4<SPDScalar>(min(min(v0.x, v1.x), min(v2.x, v3.x)), max(max4.x, max4.y), 0.0, 0.0);
    }
    )";
        case SPDFilter::Average:
        default:
            return R"(
    fn spd_reduce_4(v0: vec4<SPDScalar>, v1: vec4<SPDScalar>, v2: vec4<SPDScalar>, v3: vec4<SPDScalar>) -> vec4<SPDScalar> {
        return (v0 + v1 + v2 + v3) * 0.25;
    }
    )";
    }
}


MipmapGenerator::MipmapGenerator(const wgpu::Device& device)
    : m_device(device) {
    wgpu::BindGroupLayoutEntry bglEntry{};
    bglEntry.binding = 0;
    bglEntry.visibility = wgpu::ShaderStage::Compute;
    bglEntry.buffer.type = wgpu::BufferBindingType::Uniform;
    bglEntry.buffer.minBindingSize = 16;

    wgpu::BindGroupLayoutDescriptor bglDesc{};
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;
    m_internalResourcesBindGroupLayout =
            m_device.CreateBindGroupLayout(&bglDesc); // Basic BGL for <= 6 mips

    wgpu::Limits deviceLimits;
    m_device.GetLimits(&deviceLimits);
    // Max number of *output* mips we can generate in a single pass.
    // This is device.maxStorageTexturesPerShaderStage - 1 (for the source texture binding),
    // further capped at 11 output mips (which means 12 total texture bindings: 1 source + 11
    // outputs). WebGPU minspec for maxStorageTexturesPerShaderStage is 4, so
    // deviceLimits.maxStorageTexturesPerShaderStage >= 4. Thus, m_maxMipsPerPass will be at least
    // std::min(4u - 1, 11u) = 3.
    m_maxMipsPerPass =
            std::min(static_cast<uint32_t>(deviceLimits.maxStorageTexturesPerShaderStage) - 1, 11u);
    
    // If we can generate more than 6 mips in a pass, we need an advanced BGL
    // that includes atomic counter and mid-mip buffer for levels > 5.
    if (m_maxMipsPerPass > 6) {
        std::vector<wgpu::BindGroupLayoutEntry> entriesAdvanced(3);
        // Entry 0: Uniform buffer (DownsamplePassMeta)
        entriesAdvanced[0].binding = 0;
        entriesAdvanced[0].visibility = wgpu::ShaderStage::Compute;
        entriesAdvanced[0].buffer.type = wgpu::BufferBindingType::Uniform;
        entriesAdvanced[0].buffer.minBindingSize = 16; // sizeof(DownsamplePassMeta) effectively

        // Entry 1: Storage buffer (spd_global_counter)
        entriesAdvanced[1].binding = 1;
        entriesAdvanced[1].visibility = wgpu::ShaderStage::Compute;
        entriesAdvanced[1].buffer.type = wgpu::BufferBindingType::Storage;
        entriesAdvanced[1].buffer.minBindingSize = 4; // sizeof(atomic<u32>)

        // Entry 2: Storage buffer (mip_dst_6_buffer)
        entriesAdvanced[2].binding = 2;
        entriesAdvanced[2].visibility = wgpu::ShaderStage::Compute;
        entriesAdvanced[2].buffer.type = wgpu::BufferBindingType::Storage;
        // Size for array<array<array<vec4<f32>, 64>, 64>> which is 16 (bytes for vec4f32) * 64 * 64
        // * arrayLayers. The shader defines it as [slice][uv.y][uv.x], so 64x64 per slice.
        // minBindingSize here is for one slice as a baseline. Actual buffer will be larger for
        // multiple array layers.
        entriesAdvanced[2].buffer.minBindingSize = 16 * 64 * 64;

        wgpu::BindGroupLayoutDescriptor bglDescAdvanced{};
        bglDescAdvanced.label = "SPD Internal BGL (Advanced)";
        bglDescAdvanced.entryCount = static_cast<uint32_t>(entriesAdvanced.size());
        bglDescAdvanced.entries = entriesAdvanced.data();
        m_internalResourcesBindGroupLayout_Advanced =
                m_device.CreateBindGroupLayout(&bglDescAdvanced);
    }
}

void MipmapGenerator::PreparePipelines(wgpu::TextureFormat format, SPDFilter filter,
        bool halfPrecision) {
    SPDScalarType scalarType = SanitizeScalarType(format, halfPrecision);
    // Prepare for a reasonable number of mips, up to m_maxMipsPerPass
    // The loop should go from 1 up to and including m_maxMipsPerPass.
    // If m_maxMipsPerPass is 0 (e.g. maxStorageTexturesPerShaderStage <=1 ), this loop won't run.
    for (uint32_t i = 1; i <= m_maxMipsPerPass; ++i) {
        GetOrCreatePipeline({ format, scalarType, filter, i });
    }
}

SPDScalarType MipmapGenerator::SanitizeScalarType(wgpu::TextureFormat format, bool halfPrecision) {
    std::string formatStr = to_string(format);
    std::transform(formatStr.begin(), formatStr.end(), formatStr.begin(), ::tolower);

    SPDScalarType texelType = SPDScalarType::F32; // Default to F32
    if (formatStr.find("sint") != std::string::npos) {
        texelType = SPDScalarType::I32;
    } else if (formatStr.find("uint") != std::string::npos) {
        texelType = SPDScalarType::U32;
    }

    if (halfPrecision && texelType == SPDScalarType::F32) {
        if (m_device.HasFeature(wgpu::FeatureName::ShaderF16)) {
            return SPDScalarType::F16;
        } else {
            // Optional: Log a warning here that halfPrecision was requested for a float format
            // but the device does not support ShaderF16, so F32 will be used.
        }
    } else if (halfPrecision && texelType != SPDScalarType::F32) {
        // Optional: Log a warning here that halfPrecision was requested for a non-float format.
    }
    return texelType;
}

SPDPipeline& MipmapGenerator::GetOrCreatePipeline(const PipelineCacheKey& key) {
    uint32_t numMipsForPipeline = std::min(key.numMips, m_maxMipsPerPass);
    if (m_pipelines.find(key) == m_pipelines.end()) {
        // Create the pipeline
        SPDPipeline spdPipeline;

        std::vector<wgpu::BindGroupLayoutEntry> mipsBglEntries;
        // Bindings are 0 for source, 1 to numMipsForPipeline for destinations
        for (uint32_t i = 0; i <= numMipsForPipeline; ++i) {
            wgpu::BindGroupLayoutEntry entry{};
            entry.binding = i;
            entry.visibility = wgpu::ShaderStage::Compute;
            if (i == 0) {
                entry.texture.sampleType = (key.scalarType == SPDScalarType::I32)
                                                   ? wgpu::TextureSampleType::Sint
                                           : (key.scalarType == SPDScalarType::U32)
                                                   ? wgpu::TextureSampleType::Uint
                                                   : wgpu::TextureSampleType::UnfilterableFloat;
                entry.texture.viewDimension = wgpu::TextureViewDimension::e2DArray;
            } else {
                entry.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                entry.storageTexture.format = key.format;
                entry.storageTexture.viewDimension = wgpu::TextureViewDimension::e2DArray;
            }
            mipsBglEntries.push_back(entry);
        }

        wgpu::BindGroupLayoutDescriptor mipsBglDesc{};
        mipsBglDesc.entryCount = mipsBglEntries.size();
        mipsBglDesc.entries = mipsBglEntries.data();
        spdPipeline.mipsBindGroupLayout = m_device.CreateBindGroupLayout(&mipsBglDesc);

        std::string shaderCode = MakeShaderCode(key.format, GetFilterCode(key.filter),
                numMipsForPipeline, key.scalarType);
        wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
        wgslDesc.code = shaderCode.c_str();

        wgpu::ShaderModuleDescriptor shaderModuleDesc{};
        shaderModuleDesc.nextInChain = &wgslDesc;
        wgpu::ShaderModule shaderModule = m_device.CreateShaderModule(&shaderModuleDesc);

        wgpu::BindGroupLayout chosenInternalBGL =
                (numMipsForPipeline > 6 && m_internalResourcesBindGroupLayout_Advanced)
                        ? m_internalResourcesBindGroupLayout_Advanced
                        : m_internalResourcesBindGroupLayout;

        wgpu::BindGroupLayout bgls[] = { spdPipeline.mipsBindGroupLayout, chosenInternalBGL };
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = 2;
        layoutDesc.bindGroupLayouts = bgls;

        wgpu::ComputePipelineDescriptor pipelineDesc{};
        pipelineDesc.layout = m_device.CreatePipelineLayout(&layoutDesc);
        pipelineDesc.compute.module = shaderModule;
        pipelineDesc.compute.entryPoint = "downsample";

        spdPipeline.pipeline = m_device.CreateComputePipeline(&pipelineDesc);
        m_pipelines[key] = std::move(spdPipeline);
    }
    return m_pipelines[key];
}


void MipmapGenerator::Generate(wgpu::CommandEncoder& commandEncoder, wgpu::Texture srcTexture,
        const SPDPassConfig& config) {
    // config.numMips now represents the total number of desired mip levels for the texture.
    // config.sourceMipLevel is the starting level (e.g., 0 for a full chain).
    if (config.numMips <= 1 || config.numMips <= config.sourceMipLevel + 1) {
        return; // Not enough mips to generate, or target is already met/exceeded.
    }

    if (m_maxMipsPerPass == 0) {
        // This indicates an issue, e.g., deviceLimits.maxStorageTexturesPerShaderStage <= 1.
        // Log an error or handle appropriately.
        return;
    }

    // Prepare pipelines once based on the overall configuration for filter and precision.
    // The actual number of mips per pipeline will be handled by GetOrCreatePipeline.
    PreparePipelines(srcTexture.GetFormat(), config.filter, config.halfPrecision);

    uint32_t mipsToGenerateCount = config.numMips - (config.sourceMipLevel + 1);
    uint32_t currentSourceMipLevel = config.sourceMipLevel;

    while (mipsToGenerateCount > 0) {
        uint32_t mipsThisPass = std::min(mipsToGenerateCount, m_maxMipsPerPass);

        SPDPassConfig passSpecificConfig = {}; // Create a new config for this specific pass
        passSpecificConfig.filter = config.filter;
        passSpecificConfig.halfPrecision = config.halfPrecision;
        passSpecificConfig.targetTexture = config.targetTexture ? config.targetTexture : srcTexture;
        passSpecificConfig.numMips = mipsThisPass; // numMips for this specific pass
        passSpecificConfig.sourceMipLevel = currentSourceMipLevel;

        generatePass(commandEncoder, srcTexture, passSpecificConfig);

        mipsToGenerateCount -= mipsThisPass;
        currentSourceMipLevel += mipsThisPass;
    }
}

void MipmapGenerator::generatePass(wgpu::CommandEncoder& commandEncoder, wgpu::Texture srcTexture,
        const SPDPassConfig& passConfig) { // Renamed param to passConfig for clarity
    uint32_t width = srcTexture.GetWidth() >> passConfig.sourceMipLevel;
    uint32_t height = srcTexture.GetHeight() >> passConfig.sourceMipLevel;
    uint32_t arrayLayerCount = srcTexture.GetDepthOrArrayLayers();

    wgpu::Texture target = passConfig.targetTexture ? passConfig.targetTexture : srcTexture;
    uint32_t numMipsThisPass = passConfig.numMips; // This is now specific to this pass

    if (numMipsThisPass == 0) return;

    SPDScalarType scalarType = SanitizeScalarType(srcTexture.GetFormat(), passConfig.halfPrecision);
    SPDPipeline& spdPipeline = GetOrCreatePipeline(
            { target.GetFormat(), scalarType, passConfig.filter, numMipsThisPass });

    // --- Create Bind Group 0 (Mips) ---
    std::vector<wgpu::BindGroupEntry> mipEntries;

    wgpu::TextureViewDescriptor srcViewDesc{};
    srcViewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
    srcViewDesc.baseMipLevel = passConfig.sourceMipLevel;
    srcViewDesc.mipLevelCount = 1;
    srcViewDesc.baseArrayLayer = 0;
    srcViewDesc.arrayLayerCount = arrayLayerCount;

    wgpu::BindGroupEntry srcEntry{};
    srcEntry.binding = 0;
    srcEntry.textureView = srcTexture.CreateView(&srcViewDesc);
    mipEntries.push_back(srcEntry);

    for (uint32_t i = 0; i < numMipsThisPass; ++i) {
        wgpu::TextureViewDescriptor dstViewDesc{};
        dstViewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
        dstViewDesc.baseMipLevel = passConfig.sourceMipLevel + i + 1;
        dstViewDesc.mipLevelCount = 1;
        dstViewDesc.baseArrayLayer = 0;
        dstViewDesc.arrayLayerCount = arrayLayerCount;

        wgpu::BindGroupEntry dstEntry{};
        dstEntry.binding = i + 1;
        dstEntry.textureView = target.CreateView(&dstViewDesc);
        mipEntries.push_back(dstEntry);
    }

    wgpu::BindGroupDescriptor mipBindGroupDesc{};
    mipBindGroupDesc.layout = spdPipeline.mipsBindGroupLayout;
    mipBindGroupDesc.entryCount = mipEntries.size();
    mipBindGroupDesc.entries = mipEntries.data();
    wgpu::BindGroup mipsBindGroup = m_device.CreateBindGroup(&mipBindGroupDesc);

    // --- Create Bind Group 1 (Internal Resources) ---
    uint32_t numWorkGroupsX = (width + 63) / 64;
    uint32_t numWorkGroupsY = (height + 63) / 64;
    uint32_t numWorkGroups = numWorkGroupsX * numWorkGroupsY;

    struct DownsamplePassMeta {
        uint32_t work_group_offset[2] = { 0, 0 };
        uint32_t num_work_groups;
        uint32_t mips;
        uint32_t padding[12]; // Ensure size is multiple of 16
    } meta;
    meta.num_work_groups = numWorkGroups;
    meta.mips = numMipsThisPass;

    wgpu::BufferDescriptor metaBufferDesc{};
    metaBufferDesc.size = sizeof(DownsamplePassMeta);
    metaBufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer metaBuffer = m_device.CreateBuffer(&metaBufferDesc);
    m_device.GetQueue().WriteBuffer(metaBuffer, 0, &meta, sizeof(meta));

    wgpu::BindGroup internalBindGroup;
    wgpu::BindGroupLayout chosenInternalBGL =
            (numMipsThisPass > 6 && m_internalResourcesBindGroupLayout_Advanced)
                    ? m_internalResourcesBindGroupLayout_Advanced
                    : m_internalResourcesBindGroupLayout;

    // If using the advanced BGL for >6 mips, we must create and bind the additional resources.
    if (numMipsThisPass > 6 && m_internalResourcesBindGroupLayout_Advanced) {
        // Create the atomic counter buffer used to elect the "last workgroup".
        wgpu::BufferDescriptor atomicCounterBufferDesc{};
        atomicCounterBufferDesc.size = sizeof(uint32_t) * arrayLayerCount; // One counter per slice
        atomicCounterBufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer atomicCounterBuffer = m_device.CreateBuffer(&atomicCounterBufferDesc);

        // IMPORTANT: The atomic counter buffer must be cleared to 0 before dispatch.
        commandEncoder.ClearBuffer(atomicCounterBuffer, 0, WGPU_WHOLE_SIZE);

        // Create the intermediate buffer to hold Mip 6 results for the final passes.
        wgpu::BufferDescriptor midMipBufferDesc{};
        midMipBufferDesc.size = 64 * 64 * 16 * arrayLayerCount; // 64x64 * vec4<f32> * slices
        midMipBufferDesc.usage = wgpu::BufferUsage::Storage;
        wgpu::Buffer midMipBuffer = m_device.CreateBuffer(&midMipBufferDesc);

        // Create bind group entries for all three resources.
        std::vector<wgpu::BindGroupEntry> internalEntries(3);
        internalEntries[0].binding = 0;
        internalEntries[0].buffer = metaBuffer;
        internalEntries[0].size = sizeof(DownsamplePassMeta);

        internalEntries[1].binding = 1;
        internalEntries[1].buffer = atomicCounterBuffer;
        internalEntries[1].size = atomicCounterBuffer.GetSize();

        internalEntries[2].binding = 2;
        internalEntries[2].buffer = midMipBuffer;
        internalEntries[2].size = midMipBuffer.GetSize();

        wgpu::BindGroupDescriptor internalBindGroupDesc{};
        internalBindGroupDesc.layout = chosenInternalBGL;
        internalBindGroupDesc.entryCount = static_cast<uint32_t>(internalEntries.size());
        internalBindGroupDesc.entries = internalEntries.data();
        internalBindGroup = m_device.CreateBindGroup(&internalBindGroupDesc);
    } else {
        // Original path for <= 6 mips (only needs the meta uniform).
        wgpu::BindGroupEntry metaEntry{};
        metaEntry.binding = 0;
        metaEntry.buffer = metaBuffer;
        metaEntry.size = sizeof(DownsamplePassMeta);

        wgpu::BindGroupDescriptor internalBindGroupDesc{};
        internalBindGroupDesc.layout = chosenInternalBGL;
        internalBindGroupDesc.entryCount = 1;
        internalBindGroupDesc.entries = &metaEntry;
        internalBindGroup = m_device.CreateBindGroup(&internalBindGroupDesc);
    }

    // --- Dispatch ---
    wgpu::ComputePassEncoder pass = commandEncoder.BeginComputePass();
    pass.SetPipeline(spdPipeline.pipeline);
    pass.SetBindGroup(0, mipsBindGroup);
    pass.SetBindGroup(1, internalBindGroup);
    pass.DispatchWorkgroups(numWorkGroupsX, numWorkGroupsY, arrayLayerCount);
    pass.End();
}

// Main shader generation logic

// Helper function to check if a string is in a vector of strings
bool includes(const std::vector<std::string>& vec, const std::string& str) {
    for (const auto& s: vec) {
        if (s == str) {
            return true;
        }
    }
    return false;
}

std::string MakeShaderCode(wgpu::TextureFormat outputFormat, const std::string& filterOp,
        unsigned int numMips,       // Assuming a default value for numMips
        SPDScalarType scalarType) { // Default scalarType
    std::stringstream ss;

    std::string texelType;
    if (scalarType == SPDScalarType::I32) {
        texelType = "i32";
    } else if (scalarType == SPDScalarType::U32) {
        texelType = "u32";
    } else {
        texelType = "f32";
    }

    bool useF16 = (scalarType == SPDScalarType::F16);

    std::string filterCode = filterOp;
    if (filterOp == SPD_FILTER_AVERAGE && !includes({ "f32", "f16" }, texelType)) {
        // Replace "* 0.25" with "/ 4"
        size_t pos = filterCode.find("* 0.25");
        if (pos != std::string::npos) {
            filterCode.replace(pos, std::string("* 0.25").length(), "/ 4");
        }
    }

    // Generate mipsBindings
    std::string mipsBindings;
    for (unsigned int i = 0; i < numMips; ++i) {
        mipsBindings += "@group(0) @binding(" + std::to_string(i + 1) + ") var dst_mip_" +
                        std::to_string(i + 1) + ": texture_storage_2d_array<" +
                        to_string(outputFormat) + ", write>;\n";
    }

    // Generate mipsAccessorBody
    std::string mipsAccessorBody;
    for (unsigned int i = 0; i < numMips; ++i) {
        if (i == 5 && numMips > 6) {
            mipsAccessorBody += " else if mip == 6 {\n";
            mipsAccessorBody += "                textureStore(dst_mip_6, uv, slice, " +
                                (useF16 ? "vec4<" + texelType + ">(value)" : "value") + ");\n";
            mipsAccessorBody += "                mip_dst_6_buffer[slice][uv.y][uv.x] = value;\n";
            mipsAccessorBody += "            }";
        } else {
            if (i != 0) {
                mipsAccessorBody += " else ";
            }
            mipsAccessorBody += "if mip == " + std::to_string(i + 1) + " {\n";
            mipsAccessorBody += "                textureStore(dst_mip_" + std::to_string(i + 1) +
                                ", uv, slice, " +
                                (useF16 ? "vec4<" + texelType + ">(value)" : "value") + ");\n";
            mipsAccessorBody += "            }";
        }
    }

    std::string mipsAccessor =
            "fn store_dst_mip(value: vec4<SPDScalar>, uv: vec2<u32>, slice: u32, mip: u32) {\n" +
            mipsAccessorBody + "\n}";
    std::string midMipAccessor = "return mip_dst_6_buffer[slice][uv.y][uv.x];";

    // Start building the final shader code string
    ss << R"(
// This file is part of the FidelityFX SDK.
//
// Copyright (C) 2023 Advanced Micro Devices, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the “Software”), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


// Definitions --------------------------------------------------------------------------------------------------------

)";
    if (useF16) {
        ss << "enable f16;\n";
    }
    ss << "alias SPDScalar = " << texelType
       << ";\n\n"; // Using texelType here, assuming SPDScalar maps to it.

    ss << R"(
// Helpers ------------------------------------------------------------------------------------------------------------

/**
 * A helper function performing a remap 64x1 to 8x8 remapping which is necessary for 2D wave reductions.
 * * The 64-wide lane indices to 8x8 remapping is performed as follows:
 * 00 01 08 09 10 11 18 19
 * 02 03 0a 0b 12 13 1a 1b
 * 04 05 0c 0d 14 15 1c 1d
 * 06 07 0e 0f 16 17 1e 1f
 * 20 21 28 29 30 31 38 39
 * 22 23 2a 2b 32 33 3a 3b
 * 24 25 2c 2d 34 35 3c 3d
 * 26 27 2e 2f 36 37 3e 3f
 * * @param a: The input 1D coordinate to remap.
 *
 * @returns The remapped 2D coordinates.
 */
fn remap_for_wave_reduction(a: u32) -> vec2<u32> {
    return vec2<u32>(
        insertBits(extractBits(a, 2u, 3u), a, 0u, 1u),
        insertBits(extractBits(a, 3u, 3u), extractBits(a, 1u, 2u), 0u, 2u)
    );
}

fn map_to_xy(local_invocation_index: u32) -> vec2<u32> {
    let sub_xy: vec2<u32> = remap_for_wave_reduction(local_invocation_index % 64);
    return vec2<u32>(
        sub_xy.x + 8 * ((local_invocation_index >> 6) % 2),
        sub_xy.y + 8 * ((local_invocation_index >> 7))
    );
}

/*
 * Compute a linear value from a SRGB value.
 * * @param value: The value to convert to linear from SRGB.
 * * @returns A value in SRGB space.
 */
/*
fn srgb_to_linear(value: SPDScalar) -> SPDScalar {
    let j = vec3<SPDScalar>(0.0031308 * 12.92, 12.92, 1.0 / 2.4);
    let k = vec2<SPDScalar>(1.055, -0.055);
    return clamp(j.x, value * j.y, pow(value, j.z) * k.x + k.y);
}
*/

// Resources & Accessors -----------------------------------------------------------------------------------------------
struct DownsamplePassMeta {
    work_group_offset: vec2<u32>,
    num_work_groups: u32,
    mips: u32,
}

// In the original version dst_mip_i is an image2Darray [SPD_MAX_MIP_LEVELS+1], i.e., 12+1, but WGSL doesn't support arrays of textures yet
// Also these are read_write because for mips 7-13, the workgroup reads from mip level 6 - since most formats don't support read_write access in WGSL yet, we use a single read_write buffer in such cases instead
@group(0) @binding(0) var src_mip_0: texture_2d_array<)"
       << texelType << R"(>;
)" << mipsBindings
       << R"(

@group(1) @binding(0) var<uniform> downsample_pass_meta : DownsamplePassMeta;
@group(1) @binding(1) var<storage, read_write> spd_global_counter: array<atomic<u32>>;
@group(1) @binding(2) var<storage, read_write> mip_dst_6_buffer: array<array<array<vec4<f32>, 64>, 64>>;

fn get_mips() -> u32 {
    return downsample_pass_meta.mips;
}

fn get_num_work_groups() -> u32 {
    return downsample_pass_meta.num_work_groups;
}

fn get_work_group_offset() -> vec2<u32> {
    return downsample_pass_meta.work_group_offset;
}

fn load_src_image(uv: vec2<u32>, slice: u32) -> vec4<SPDScalar> {
    return vec4<SPDScalar>(textureLoad(src_mip_0, uv, slice, 0));
}

fn load_mid_mip_image(uv: vec2<u32>, slice: u32) -> vec4<SPDScalar> {
    )";
    if (numMips > 6) {
        ss << midMipAccessor;
    } else {
        ss << "return vec4<SPDScalar>();";
    }
    ss << R"(
}

)" << mipsAccessor
       << R"(

// Workgroup -----------------------------------------------------------------------------------------------------------

var<workgroup> spd_intermediate: array<array<vec4<SPDScalar>, 16>, 16>;
var<workgroup> spd_counter: atomic<u32>;

fn spd_increase_atomic_counter(slice: u32) {
    atomicStore(&spd_counter, atomicAdd(&spd_global_counter[slice], 1));
}

fn spd_get_atomic_counter() -> u32 {
    return atomicLoad(&spd_counter);
}

fn spd_reset_atomic_counter(slice: u32) {
    atomicStore(&spd_global_counter[slice], 0);
}

// Cotnrol flow --------------------------------------------------------------------------------------------------------

fn spd_barrier() {
    // in glsl this does: groupMemoryBarrier(); barrier();
    workgroupBarrier();
}

// Only last active workgroup should proceed
fn spd_exit_workgroup(num_work_groups: u32, local_invocation_index: u32, slice: u32) -> bool {
    // global atomic counter
    if (local_invocation_index == 0) {
        spd_increase_atomic_counter(slice);
    }
    spd_barrier();
    return spd_get_atomic_counter() != (num_work_groups - 1);
}

// Pixel access --------------------------------------------------------------------------------------------------------

)" << filterCode
       << R"(

fn spd_store(pix: vec2<u32>, out_value: vec4<SPDScalar>, mip: u32, slice: u32) {
    store_dst_mip(out_value, pix, slice, mip + 1);
}

fn spd_load_intermediate(x: u32, y: u32) -> vec4<SPDScalar> {
    return spd_intermediate[x][y];
}

fn spd_store_intermediate(x: u32, y: u32, value: vec4<SPDScalar>) {
    spd_intermediate[x][y] = value;
}

fn spd_reduce_intermediate(i0: vec2<u32>, i1: vec2<u32>, i2: vec2<u32>, i3: vec2<u32>) -> vec4<SPDScalar> {
    let v0 = spd_load_intermediate(i0.x, i0.y);
    let v1 = spd_load_intermediate(i1.x, i1.y);
    let v2 = spd_load_intermediate(i2.x, i2.y);
    let v3 = spd_load_intermediate(i3.x, i3.y);
    return spd_reduce_4(v0, v1, v2, v3);
}

fn spd_reduce_load_4(base: vec2<u32>, slice: u32) -> vec4<SPDScalar> {
    let v0 = load_src_image(base + vec2<u32>(0, 0), slice);
    let v1 = load_src_image(base + vec2<u32>(0, 1), slice);
    let v2 = load_src_image(base + vec2<u32>(1, 0), slice);
    let v3 = load_src_image(base + vec2<u32>(1, 1), slice);
    return spd_reduce_4(v0, v1, v2, v3);
}

fn spd_reduce_load_mid_mip_4(base: vec2<u32>, slice: u32) -> vec4<SPDScalar> {
    let v0 = load_mid_mip_image(base + vec2<u32>(0, 0), slice);
    let v1 = load_mid_mip_image(base + vec2<u32>(0, 1), slice);
    let v2 = load_mid_mip_image(base + vec2<u32>(1, 0), slice);
    let v3 = load_mid_mip_image(base + vec2<u32>(1, 1), slice);
    return spd_reduce_4(v0, v1, v2, v3);
}

// Main logic ---------------------------------------------------------------------------------------------------------

fn spd_downsample_mips_0_1(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, mip: u32, slice: u32) {
    var v: array<vec4<SPDScalar>, 4>;

    let workgroup64 = workgroup_id.xy * 64;
    let workgroup32 = workgroup_id.xy * 32;
    let workgroup16 = workgroup_id.xy * 16;

    var tex = workgroup64 + vec2<u32>(x * 2, y * 2);
    var pix = workgroup32 + vec2<u32>(x, y);
    v[0] = spd_reduce_load_4(tex, slice);
    spd_store(pix, v[0], 0, slice);

    tex = workgroup64 + vec2<u32>(x * 2 + 32, y * 2);
    pix = workgroup32 + vec2<u32>(x + 16, y);
    v[1] = spd_reduce_load_4(tex, slice);
    spd_store(pix, v[1], 0, slice);

    tex = workgroup64 + vec2<u32>(x * 2, y * 2 + 32);
    pix = workgroup32 + vec2<u32>(x, y + 16);
    v[2] = spd_reduce_load_4(tex, slice);
    spd_store(pix, v[2], 0, slice);

    tex = workgroup64 + vec2<u32>(x * 2 + 32, y * 2 + 32);
    pix = workgroup32 + vec2<u32>(x + 16, y + 16);
    v[3] = spd_reduce_load_4(tex, slice);
    spd_store(pix, v[3], 0, slice);

    if mip <= 1 {
        return;
    }

    for (var i = 0u; i < 4u; i++) {
        spd_store_intermediate(x, y, v[i]);
        spd_barrier();
        if local_invocation_index < 64 {
            v[i] = spd_reduce_intermediate(
                vec2<u32>(x * 2 + 0, y * 2 + 0),
                vec2<u32>(x * 2 + 1, y * 2 + 0),
                vec2<u32>(x * 2 + 0, y * 2 + 1),
                vec2<u32>(x * 2 + 1, y * 2 + 1)
            );
            spd_store(workgroup16 + vec2<u32>(x + (i % 2) * 8, y + (i / 2) * 8), v[i], 1, slice);
        }
        spd_barrier();
    }

    if local_invocation_index < 64 {
        spd_store_intermediate(x + 0, y + 0, v[0]);
        spd_store_intermediate(x + 8, y + 0, v[1]);
        spd_store_intermediate(x + 0, y + 8, v[2]);
        spd_store_intermediate(x + 8, y + 8, v[3]);
    }
}

fn spd_downsample_mip_2(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, mip: u32, slice: u32) {
    if local_invocation_index < 64u {
        let v = spd_reduce_intermediate(
            vec2<u32>(x * 2 + 0, y * 2 + 0),
            vec2<u32>(x * 2 + 1, y * 2 + 0),
            vec2<u32>(x * 2 + 0, y * 2 + 1),
            vec2<u32>(x * 2 + 1, y * 2 + 1)
        );
        spd_store(workgroup_id.xy * 8 + vec2<u32>(x, y), v, mip, slice);
        // store to LDS, try to reduce bank conflicts
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0 x
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        // ...
        // x 0 x 0 x 0 x 0 x 0 x 0 x 0 x 0
        spd_store_intermediate(x * 2 + y % 2, y * 2, v);
    }
}

fn spd_downsample_mip_3(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, mip: u32, slice: u32) {
    if local_invocation_index < 16u {
        // x 0 x 0
        // 0 0 0 0
        // 0 x 0 x
        // 0 0 0 0
        let v = spd_reduce_intermediate(
            vec2<u32>(x * 4 + 0 + 0, y * 4 + 0),
            vec2<u32>(x * 4 + 2 + 0, y * 4 + 0),
            vec2<u32>(x * 4 + 0 + 1, y * 4 + 2),
            vec2<u32>(x * 4 + 2 + 1, y * 4 + 2)
        );
        spd_store(workgroup_id.xy * 4 + vec2<u32>(x, y), v, mip, slice);
        // store to LDS
        // x 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
        // 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0 0
        // ...
        // 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x 0
        // ...
        // 0 0 0 x 0 0 0 x 0 0 0 x 0 0 0 x
        // ...
        spd_store_intermediate(x * 4 + y, y * 4, v);
    }
}

fn spd_downsample_mip_4(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, mip: u32, slice: u32) {
    if local_invocation_index < 4u {
        // x 0 0 0 x 0 0 0
        // ...
        // 0 x 0 0 0 x 0 0
        let v = spd_reduce_intermediate(
            vec2<u32>(x * 8 + 0 + 0 + y * 2, y * 8 + 0),
            vec2<u32>(x * 8 + 4 + 0 + y * 2, y * 8 + 0),
            vec2<u32>(x * 8 + 0 + 1 + y * 2, y * 8 + 4),
            vec2<u32>(x * 8 + 4 + 1 + y * 2, y * 8 + 4)
        );
        spd_store(workgroup_id.xy * 2 + vec2<u32>(x, y), v, mip, slice);
        // store to LDS
        // x x x x 0 ...
        // 0 ...
        spd_store_intermediate(x + y * 2, 0, v);
    }
}

fn spd_downsample_mip_5(workgroup_id: vec2<u32>, local_invocation_index: u32, mip: u32, slice: u32) {
    if local_invocation_index < 1u {
        // x x x x 0 ...
        // 0 ...
        let v = spd_reduce_intermediate(vec2<u32>(0, 0), vec2<u32>(1, 0), vec2<u32>(2, 0), vec2<u32>(3, 0));
        spd_store(workgroup_id.xy, v, mip, slice);
    }
}

fn spd_downsample_next_four(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, base_mip: u32, mips: u32, slice: u32) {
    if mips <= base_mip {
        return;
    }
    spd_barrier();
    spd_downsample_mip_2(x, y, workgroup_id, local_invocation_index, base_mip, slice);

    if mips <= base_mip + 1 {
        return;
    }
    spd_barrier();
    spd_downsample_mip_3(x, y, workgroup_id, local_invocation_index, base_mip + 1, slice);

    if mips <= base_mip + 2 {
        return;
    }
    spd_barrier();
    spd_downsample_mip_4(x, y, workgroup_id, local_invocation_index, base_mip + 2, slice);

    if mips <= base_mip + 3 {
        return;
    }
    spd_barrier();
    spd_downsample_mip_5(workgroup_id, local_invocation_index, base_mip + 3, slice);
}

fn spd_downsample_last_four(x: u32, y: u32, workgroup_id: vec2<u32>, local_invocation_index: u32, base_mip: u32, mips: u32, slice: u32, exit: bool) {
    if mips <= base_mip {
        return;
    }
    spd_barrier();
    if !exit {
        spd_downsample_mip_2(x, y, workgroup_id, local_invocation_index, base_mip, slice);
    }

    if mips <= base_mip + 1 {
        return;
    }
    spd_barrier();
    if !exit {
        spd_downsample_mip_3(x, y, workgroup_id, local_invocation_index, base_mip + 1, slice);
    }

    if mips <= base_mip + 2 {
        return;
    }
    spd_barrier();
    if !exit {
        spd_downsample_mip_4(x, y, workgroup_id, local_invocation_index, base_mip + 2, slice);
    }

    if mips <= base_mip + 3 {
        return;
    }
    spd_barrier();
    if !exit {
        spd_downsample_mip_5(workgroup_id, local_invocation_index, base_mip + 3, slice);
    }
}

fn spd_downsample_mips_6_7(x: u32, y: u32, mips: u32, slice: u32) {
    var tex = vec2<u32>(x * 4 + 0, y * 4 + 0);
    var pix = vec2<u32>(x * 2 + 0, y * 2 + 0);
    let v0 = spd_reduce_load_mid_mip_4(tex, slice);
    spd_store(pix, v0, 6, slice);

    tex = vec2<u32>(x * 4 + 2, y * 4 + 0);
    pix = vec2<u32>(x * 2 + 1, y * 2 + 0);
    let v1 = spd_reduce_load_mid_mip_4(tex, slice);
    spd_store(pix, v1, 6, slice);

    tex = vec2<u32>(x * 4 + 0, y * 4 + 2);
    pix = vec2<u32>(x * 2 + 0, y * 2 + 1);
    let v2 = spd_reduce_load_mid_mip_4(tex, slice);
    spd_store(pix, v2, 6, slice);

    tex = vec2<u32>(x * 4 + 2, y * 4 + 2);
    pix = vec2<u32>(x * 2 + 1, y * 2 + 1);
    let v3 = spd_reduce_load_mid_mip_4(tex, slice);
    spd_store(pix, v3, 6, slice);

    if mips <= 7 {
        return;
    }
    // no barrier needed, working on values only from the same thread

    let v = spd_reduce_4(v0, v1, v2, v3);
    spd_store(vec2<u32>(x, y), v, 7, slice);
    spd_store_intermediate(x, y, v);
}

fn spd_downsample_last_6(x: u32, y: u32, local_invocation_index: u32, mips: u32, num_work_groups: u32, slice: u32) {
    if mips <= 6 {
        return;
    }

    // increase the global atomic counter for the given slice and check if it's the last remaining thread group:
    // terminate if not, continue if yes.
    let exit = spd_exit_workgroup(num_work_groups, local_invocation_index, slice);

    // can't exit directly because subsequent barrier calls break uniform control flow...
    if !exit {
        // reset the global atomic counter back to 0 for the next spd dispatch
        spd_reset_atomic_counter(slice);

        // After mip 5 there is only a single workgroup left that downsamples the remaining up to 64x64 texels.
        // compute MIP level 6 and 7
        spd_downsample_mips_6_7(x, y, mips, slice);
    }

    // compute MIP level 8, 9, 10, 11
    spd_downsample_last_four(x, y, vec2<u32>(0, 0), local_invocation_index, 8, mips, slice, exit);
}

/// Downsamples a 64x64 tile based on the work group id.
/// If after downsampling it's the last active thread group, computes the remaining MIP levels.
///
/// @param [in] workGroupID        index of the work group / thread group
/// @param [in] localInvocationIndex   index of the thread within the thread group in 1D
/// @param [in] mips             the number of total MIP levels to compute for the input texture
/// @param [in] numWorkGroups        the total number of dispatched work groups / thread groups for this slice
/// @param [in] slice             the slice of the input texture
fn spd_downsample(workgroup_id: vec2<u32>, local_invocation_index: u32, mips: u32, num_work_groups: u32, slice: u32) {
    let xy = map_to_xy(local_invocation_index);
    spd_downsample_mips_0_1(xy.x, xy.y, workgroup_id, local_invocation_index, mips, slice);
    spd_downsample_next_four(xy.x, xy.y, workgroup_id, local_invocation_index, 2, mips, slice);
)";
    if (numMips > 6) {
        ss << "    spd_downsample_last_6(xy.x, xy.y, local_invocation_index, mips, "
              "num_work_groups, slice);\n";
    }
    ss << R"(}

// Entry points -------------------------------------------------------------------------------------------------------

@compute
@workgroup_size(256, 1, 1)
fn downsample(@builtin(local_invocation_index) local_invocation_index: u32, @builtin(workgroup_id) workgroup_id: vec3<u32>) {
    spd_downsample(
        workgroup_id.xy + get_work_group_offset(),
        local_invocation_index,
        get_mips(),
        get_num_work_groups(),
        workgroup_id.z
    );
}
)";
    return ss.str();
}


} // namespace spd
