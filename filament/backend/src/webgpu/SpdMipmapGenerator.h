#pragma once

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
// C++ port of https://github.com/JolifantoBambla/webgpu-spd for early experiments- do not merge like this
namespace spd {

    // Enum for selecting the downsampling filter.
    enum class SPDFilter {
        Average,
        Min,
        Max,
        MinMax
    };

    // Enum for shader scalar types.
    enum class SPDScalarType {
        F32,
        F16,
        I32,
        U32
    };

    // Configuration for a single mipmap generation pass.
struct SPDPassConfig {
    SPDFilter filter = SPDFilter::Average;
    wgpu::Texture targetTexture = nullptr;
    uint32_t numMips = 0;
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
        void Generate(
            wgpu::CommandEncoder& commandEncoder,
            wgpu::Texture srcTexture,
            const SPDPassConfig& config
        );

    private:
        wgpu::Device m_device;
        wgpu::BindGroupLayout m_internalResourcesBindGroupLayout;

        // Cached pipelines: Map<TextureFormat, Map<SPDScalarType, Map<Filter, Map<NumMips, Pipeline>>>>
        std::unordered_map<wgpu::TextureFormat,
            std::unordered_map<SPDScalarType,
                std::unordered_map<SPDFilter,
                    std::unordered_map<uint32_t, SPDPipeline>>>> m_pipelines;
        
        // Helper methods
        SPDPipeline& GetOrCreatePipeline(wgpu::TextureFormat format, SPDFilter filter, uint32_t numMips, SPDScalarType scalarType);
        SPDScalarType SanitizeScalarType(wgpu::TextureFormat format, bool halfPrecision);
        std::string GetFilterCode(SPDFilter filter);
    };
    // Assuming SPD_FILTER_AVERAGE is a string constant
    const std::string SPD_FILTER_AVERAGE = "value * 0.25"; // Original filter operation

    // Generates the WGSL shader code dynamically.
    std::string MakeShaderCode(
        wgpu::TextureFormat outputFormat,
        const std::string& filterOp = SPD_FILTER_AVERAGE,
        uint32_t numMips = 0,
        SPDScalarType scalarType = SPDScalarType::F32
    );

} // namespace spd
