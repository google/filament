#pragma once

#include <optional>
#include <string>
#include <tsl/robin_map.h>
#include <vector>
#include <webgpu/webgpu_cpp.h>
// C++ port of https://github.com/JolifantoBambla/webgpu-spd for early experiments
namespace spd {

// Enum for selecting the downsampling filter.
enum class SPDFilter { Average, Min, Max, MinMax };

// Enum for shader scalar types.
enum class SPDScalarType { F32, F16, I32, U32 };

// Configuration for a single mipmap generation pass.
struct SPDPassConfig {
    SPDFilter filter = SPDFilter::Average;
    wgpu::Texture targetTexture = nullptr;
    uint32_t numMips = 0; // For the public API, this is total desired mips. Internally, mips for
                          // the current pass.
    bool halfPrecision = false;
    uint32_t sourceMipLevel = 0;
};

// Holds a pipeline and its corresponding bind group layout.
struct SPDPipeline {
    wgpu::BindGroupLayout mipsBindGroupLayout = nullptr;
    wgpu::ComputePipeline pipeline = nullptr;
};

// Manages pipeline creation, caching, and execution for mipmap generation.
class MipmapGenerator {
public:
    MipmapGenerator(const wgpu::Device& device);

    // Pre-creates pipelines for specified formats and filters.
    void PreparePipelines(wgpu::TextureFormat format, SPDFilter filter, bool halfPrecision = false);

    // Generates a compute pass for creating mipmaps.
    void Generate(wgpu::CommandEncoder& commandEncoder, wgpu::Texture srcTexture,
            const SPDPassConfig& config);

private:
    void generatePass(wgpu::CommandEncoder& commandEncoder, wgpu::Texture srcTexture,
            const SPDPassConfig&
                    passConfig); // Internal pass config might differ slightly if needed

    wgpu::Device m_device;
    wgpu::BindGroupLayout m_internalResourcesBindGroupLayout;
    wgpu::BindGroupLayout m_internalResourcesBindGroupLayout_Advanced = nullptr;

    // Maximum number of mips that can be generated in a single pass.
    uint32_t m_maxMipsPerPass;

    // Key for the pipeline cache.
    struct PipelineCacheKey {
        wgpu::TextureFormat format;
        SPDScalarType scalarType;
        SPDFilter filter;
        uint32_t numMips;

        bool operator==(const PipelineCacheKey& other) const {
            return format == other.format && scalarType == other.scalarType &&
                   filter == other.filter && numMips == other.numMips;
        }
    };

    // Hash function for PipelineCacheKey.
    struct PipelineCacheKeyHash {
        std::size_t operator()(const PipelineCacheKey& key) const {
            std::size_t h1 = std::hash<wgpu::TextureFormat>()(key.format);
            std::size_t h2 = std::hash<SPDScalarType>()(key.scalarType);
            std::size_t h3 = std::hash<SPDFilter>()(key.filter);
            std::size_t h4 = std::hash<uint32_t>()(key.numMips);
            // A simple way to combine hashes.
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    // Cached pipelines.
    tsl::robin_map<PipelineCacheKey, SPDPipeline, PipelineCacheKeyHash> m_pipelines;

    // Helper methods
    SPDPipeline& GetOrCreatePipeline(const PipelineCacheKey& key);
    SPDScalarType SanitizeScalarType(wgpu::TextureFormat format, bool halfPrecision);
    std::string GetFilterCode(SPDFilter filter);
};
// Assuming SPD_FILTER_AVERAGE is a string constant
const std::string SPD_FILTER_AVERAGE = "value * 0.25"; // Original filter operation

// Generates the WGSL shader code dynamically.
std::string MakeShaderCode(wgpu::TextureFormat outputFormat,
        const std::string& filterOp = SPD_FILTER_AVERAGE, uint32_t numMips = 0,
        SPDScalarType scalarType = SPDScalarType::F32);

} // namespace spd
