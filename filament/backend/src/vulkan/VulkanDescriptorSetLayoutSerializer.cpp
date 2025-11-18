#include "VulkanDescriptorSetLayoutSerializer.h"
#include <fstream>
#include <sstream>
#include <utils/Hash.h>

namespace filament::backend {
VulkanDescriptorSetLayoutSerializer::VulkanDescriptorSetLayoutSerializer(
        const VkDescriptorSetLayoutCreateInfo& info,
        utils::FixedCapacityVector<uint32_t> immutableSamplers, uint32_t hash) {
    std::stringstream filename;
    filename << "descriptor_set_layout_" << hash << ".json";

    std::ofstream file(filename.str());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << "[" << std::endl;
        for (uint32_t i = 0; i < info.bindingCount; ++i) {
            buffer << "{" << std::endl;
            buffer << "\"binding\":" << info.pBindings[i].binding << "," << std::endl;
            buffer << "\"descriptor_type\":" << info.pBindings[i].descriptorType << "," << std::endl;
            buffer << "\"stage_flags\":" << info.pBindings[i].stageFlags << "," << std::endl;
            buffer << "\"immutable_samplers\":[" << std::endl;
            for (uint32_t i = 0; i < immutableSamplers.size(); ++i) {
                buffer << immutableSamplers[i];
                if (i < immutableSamplers.size() - 1) {
                    buffer << ",";
                }
            }
            buffer << "]" << std::endl;
            buffer << "}";
            if (i < info.bindingCount - 1) {
                buffer << ",";
            }
            buffer << std::endl;
        }
        buffer << "]" << std::endl;

        file << buffer.str();
    }
    file.close();
}

VulkanPipelineLayoutSerializer::VulkanPipelineLayoutSerializer(
        const VkPipelineLayoutCreateInfo& info, VulkanDescriptorSetLayoutCache* cache,
        uint32_t hash) {
    std::stringstream filename;
    filename << "pipeline_layout_" << hash << ".json";
    std::ofstream file(filename.str());
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << "{" << std::endl;
        buffer << "\"flags\":" << info.flags << "," << std::endl;

        buffer << "\"layouts\":[" << std::endl;
        for (uint32_t i = 0; i < info.setLayoutCount; ++i) {
            buffer << cache->getKey(info.pSetLayouts[i]);
            if (i < info.setLayoutCount - 1) {
                buffer << ",";
            }
        }
        buffer << "]," << std::endl;

        buffer << "\"push_constants\":[" << std::endl;
        for (uint32_t i = 0; i < info.pushConstantRangeCount; ++i) {
            buffer << "{" << std::endl;
            buffer << "\"stage_flags\":" << info.pPushConstantRanges[i].stageFlags << "," << std::endl;
            buffer << "\"offset\":" << info.pPushConstantRanges[i].offset << ","
                   << std::endl;
            buffer << "\"size\":" << info.pPushConstantRanges[i].size
                   << std::endl;
            buffer << "}" << std::endl;
            if (i < info.pushConstantRangeCount - 1) {
                buffer << ",";
            }
        }
        buffer << "]" << std::endl;

        buffer << "}" << std::endl;
        file << buffer.str();
    }
    file.close();
 }
} // namespace filament::backend
