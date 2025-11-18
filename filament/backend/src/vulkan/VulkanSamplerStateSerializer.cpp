#include "VulkanSamplerStateSerializer.h"
#include <fstream>
#include <sstream>
#include <utils/Hash.h>

namespace filament::backend {
VulkanYcbcrConversionSerializer::VulkanYcbcrConversionSerializer(
        const VulkanYcbcrConversionCache::Params& params) {
    using ConversionHashFn = utils::hash::MurmurHashFn<VulkanYcbcrConversionCache::Params>;
    ConversionHashFn hashFn;
    uint32_t key = hashFn(params);
    std::stringstream filename;
    filename << "ycbcr_conv_" << key << ".json";

    std::ofstream file(filename.str());
    if (file.is_open()) {
        std::stringstream buffer;
        auto const& chroma = params.conversion;
        TextureSwizzle const swizzleArray[] = { chroma.r, chroma.g, chroma.b, chroma.a };
        buffer << "{" << std::endl;
        buffer << "\"format\":" << params.format << "," << std::endl;
        buffer << "\"external_format\":" << params.externalFormat << "," << std::endl;
        buffer << "\"ycbcr_model\":" << uint8_t(chroma.ycbcrModel) << "," << std::endl;
        buffer << "\"ycbcr_range\":" << uint8_t(chroma.ycbcrRange) << "," << std::endl;
        buffer << "\"swizzle_array\":[" << uint8_t(chroma.r) << "," << uint8_t(chroma.g) << ","
               << uint8_t(chroma.b) << "," << uint8_t(chroma.a) << "]," << std::endl;
        buffer << "\"x_chroma_offset\":" << uint8_t(chroma.xChromaOffset) << "," << std::endl;
        buffer << "\"y_chroma_offset\":" << uint8_t(chroma.yChromaOffset) << "," << std::endl;
        buffer << "\"chroma_filter\":" << uint8_t(chroma.chromaFilter) << std::endl;
        buffer << "}" << std::endl;

        file << buffer.str();
    }
    file.close();
}

VulkanSamplerStateSerializer::VulkanSamplerStateSerializer(const VulkanSamplerCache::Params& params, uint32_t ycbcr_conv_key) {
    using ConversionHashFn = utils::hash::MurmurHashFn<VulkanSamplerCache::Params>;
    ConversionHashFn hashFn;
    uint32_t key = hashFn(params);
    std::stringstream filename;
    filename << "sampler_" << key << ".json";

    std::ofstream file(filename.str());
    if (file.is_open()) {
        std::stringstream buffer;
        auto const& samplerParams = params.sampler;
        buffer << "{" << std::endl;
        buffer << "\"filter_mag\":" << uint32_t(samplerParams.filterMag) << "," << std::endl;
        buffer << "\"filter_min\":" << uint32_t(samplerParams.filterMin) << "," << std::endl;
        buffer << "\"wrap_s\":" << uint32_t(samplerParams.wrapS) << "," << std::endl;
        buffer << "\"wrap_t\":" << uint32_t(samplerParams.wrapT) << "," << std::endl;
        buffer << "\"wrap_r\":" << uint32_t(samplerParams.wrapR) << "," << std::endl;
        buffer << "\"anisotropy_log2\":" << uint32_t(samplerParams.anisotropyLog2) << "," << std::endl;
        buffer << "\"compare_mode\":" << uint32_t(samplerParams.compareMode) << "," << std::endl;
        buffer << "\"compare_func\":" << uint32_t(samplerParams.compareFunc) << "," << std::endl;
        buffer << "\"filter_min\":" << uint32_t(samplerParams.filterMin) << "," << std::endl;
        buffer << "\"ycbcr_conv_key\":" << ycbcr_conv_key << std::endl;
        buffer << "}" << std::endl;

        file << buffer.str();
    }
    file.close();
}

} // namespace filament::backend
