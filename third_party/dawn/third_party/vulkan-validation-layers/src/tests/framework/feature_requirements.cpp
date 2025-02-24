/*
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/feature_requirements.h"

#include "generated/pnext_chain_extraction.h"

#include <vulkan/utility/vk_struct_helper.hpp>

namespace vkt {

FeatureRequirements::~FeatureRequirements() {
    vvl::PnextChainFree(feature_chain_);
    feature_chain_ = nullptr;
}

void FeatureRequirements::AddRequiredFeature(APIVersion api_version, vkt::Feature feature) {
    FeatureAndName f = SetFeature(api_version, feature, VK_TRUE);
    *f.feature = VK_TRUE;
    required_features_.emplace_back(f);
}

void FeatureRequirements::AddOptionalFeature(APIVersion api_version, vkt::Feature feature) {
    // By setting the feature here it will be queried and passed pack to device creation
    // with the resulting values, so this in effect allows enabling the feature when available
    FeatureAndName f = SetFeature(api_version, feature, VK_TRUE);
    *f.feature = VK_TRUE;
}

vkt::FeatureAndName FeatureRequirements::SetFeature(APIVersion api_version, vkt::Feature feature, VkBool32 value) {
    switch (feature) {
        case vkt::Feature::robustBufferAccess: {
            phys_dev_features_.features.robustBufferAccess = value;
            FeatureAndName f{&phys_dev_features_.features.robustBufferAccess, "VkPhysicalDeviceFeatures::robustBufferAccess"};
            return f;
        }
        case vkt::Feature::fullDrawIndexUint32: {
            phys_dev_features_.features.fullDrawIndexUint32 = value;
            FeatureAndName f{&phys_dev_features_.features.fullDrawIndexUint32, "VkPhysicalDeviceFeatures::fullDrawIndexUint32"};
            return f;
        }
        case vkt::Feature::imageCubeArray: {
            phys_dev_features_.features.imageCubeArray = value;
            FeatureAndName f{&phys_dev_features_.features.imageCubeArray, "VkPhysicalDeviceFeatures::imageCubeArray"};
            return f;
        }
        case vkt::Feature::independentBlend: {
            phys_dev_features_.features.independentBlend = value;
            FeatureAndName f{&phys_dev_features_.features.independentBlend, "VkPhysicalDeviceFeatures::independentBlend"};
            return f;
        }
        case vkt::Feature::geometryShader: {
            phys_dev_features_.features.geometryShader = value;
            FeatureAndName f{&phys_dev_features_.features.geometryShader, "VkPhysicalDeviceFeatures::geometryShader"};
            return f;
        }
        case vkt::Feature::tessellationShader: {
            phys_dev_features_.features.tessellationShader = value;
            FeatureAndName f{&phys_dev_features_.features.tessellationShader, "VkPhysicalDeviceFeatures::tessellationShader"};
            return f;
        }
        case vkt::Feature::sampleRateShading: {
            phys_dev_features_.features.sampleRateShading = value;
            FeatureAndName f{&phys_dev_features_.features.sampleRateShading, "VkPhysicalDeviceFeatures::sampleRateShading"};
            return f;
        }
        case vkt::Feature::dualSrcBlend: {
            phys_dev_features_.features.dualSrcBlend = value;
            FeatureAndName f{&phys_dev_features_.features.dualSrcBlend, "VkPhysicalDeviceFeatures::dualSrcBlend"};
            return f;
        }
        case vkt::Feature::logicOp: {
            phys_dev_features_.features.logicOp = value;
            FeatureAndName f{&phys_dev_features_.features.logicOp, "VkPhysicalDeviceFeatures::logicOp"};
            return f;
        }
        case vkt::Feature::multiDrawIndirect: {
            phys_dev_features_.features.multiDrawIndirect = value;
            FeatureAndName f{&phys_dev_features_.features.multiDrawIndirect, "VkPhysicalDeviceFeatures::multiDrawIndirect"};
            return f;
        }
        case vkt::Feature::drawIndirectFirstInstance: {
            phys_dev_features_.features.drawIndirectFirstInstance = value;
            FeatureAndName f{&phys_dev_features_.features.drawIndirectFirstInstance,
                             "VkPhysicalDeviceFeatures::drawIndirectFirstInstance"};
            return f;
        }
        case vkt::Feature::depthClamp: {
            phys_dev_features_.features.depthClamp = value;
            FeatureAndName f{&phys_dev_features_.features.depthClamp, "VkPhysicalDeviceFeatures::depthClamp"};
            return f;
        }
        case vkt::Feature::depthBiasClamp: {
            phys_dev_features_.features.depthBiasClamp = value;
            FeatureAndName f{&phys_dev_features_.features.depthBiasClamp, "VkPhysicalDeviceFeatures::depthBiasClamp"};
            return f;
        }
        case vkt::Feature::fillModeNonSolid: {
            phys_dev_features_.features.fillModeNonSolid = value;
            FeatureAndName f{&phys_dev_features_.features.fillModeNonSolid, "VkPhysicalDeviceFeatures::fillModeNonSolid"};
            return f;
        }
        case vkt::Feature::depthBounds: {
            phys_dev_features_.features.depthBounds = value;
            FeatureAndName f{&phys_dev_features_.features.depthBounds, "VkPhysicalDeviceFeatures::depthBounds"};
            return f;
        }
        case vkt::Feature::wideLines: {
            phys_dev_features_.features.wideLines = value;
            FeatureAndName f{&phys_dev_features_.features.wideLines, "VkPhysicalDeviceFeatures::wideLines"};
            return f;
        }
        case vkt::Feature::largePoints: {
            phys_dev_features_.features.largePoints = value;
            FeatureAndName f{&phys_dev_features_.features.largePoints, "VkPhysicalDeviceFeatures::largePoints"};
            return f;
        }
        case vkt::Feature::alphaToOne: {
            phys_dev_features_.features.alphaToOne = value;
            FeatureAndName f{&phys_dev_features_.features.alphaToOne, "VkPhysicalDeviceFeatures::alphaToOne"};
            return f;
        }
        case vkt::Feature::multiViewport: {
            phys_dev_features_.features.multiViewport = value;
            FeatureAndName f{&phys_dev_features_.features.multiViewport, "VkPhysicalDeviceFeatures::multiViewport"};
            return f;
        }
        case vkt::Feature::samplerAnisotropy: {
            phys_dev_features_.features.samplerAnisotropy = value;
            FeatureAndName f{&phys_dev_features_.features.samplerAnisotropy, "VkPhysicalDeviceFeatures::samplerAnisotropy"};
            return f;
        }
        case vkt::Feature::textureCompressionETC2: {
            phys_dev_features_.features.textureCompressionETC2 = value;
            FeatureAndName f{&phys_dev_features_.features.textureCompressionETC2,
                             "VkPhysicalDeviceFeatures::textureCompressionETC2"};
            return f;
        }
        case vkt::Feature::textureCompressionASTC_LDR: {
            phys_dev_features_.features.textureCompressionASTC_LDR = value;
            FeatureAndName f{&phys_dev_features_.features.textureCompressionASTC_LDR,
                             "VkPhysicalDeviceFeatures::textureCompressionASTC_LDR"};
            return f;
        }
        case vkt::Feature::textureCompressionBC: {
            phys_dev_features_.features.textureCompressionBC = value;
            FeatureAndName f{&phys_dev_features_.features.textureCompressionBC, "VkPhysicalDeviceFeatures::textureCompressionBC"};
            return f;
        }
        case vkt::Feature::occlusionQueryPrecise: {
            phys_dev_features_.features.occlusionQueryPrecise = value;
            FeatureAndName f{&phys_dev_features_.features.occlusionQueryPrecise, "VkPhysicalDeviceFeatures::occlusionQueryPrecise"};
            return f;
        }
        case vkt::Feature::pipelineStatisticsQuery: {
            phys_dev_features_.features.pipelineStatisticsQuery = value;
            FeatureAndName f{&phys_dev_features_.features.pipelineStatisticsQuery,
                             "VkPhysicalDeviceFeatures::pipelineStatisticsQuery"};
            return f;
        }
        case vkt::Feature::vertexPipelineStoresAndAtomics: {
            phys_dev_features_.features.vertexPipelineStoresAndAtomics = value;
            FeatureAndName f{&phys_dev_features_.features.vertexPipelineStoresAndAtomics,
                             "VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics"};
            return f;
        }
        case vkt::Feature::fragmentStoresAndAtomics: {
            phys_dev_features_.features.fragmentStoresAndAtomics = value;
            FeatureAndName f{&phys_dev_features_.features.fragmentStoresAndAtomics,
                             "VkPhysicalDeviceFeatures::fragmentStoresAndAtomics"};
            return f;
        }
        case vkt::Feature::shaderTessellationAndGeometryPointSize: {
            phys_dev_features_.features.shaderTessellationAndGeometryPointSize = value;
            FeatureAndName f{&phys_dev_features_.features.shaderTessellationAndGeometryPointSize,
                             "VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize"};
            return f;
        }
        case vkt::Feature::shaderImageGatherExtended: {
            phys_dev_features_.features.shaderImageGatherExtended = value;
            FeatureAndName f{&phys_dev_features_.features.shaderImageGatherExtended,
                             "VkPhysicalDeviceFeatures::shaderImageGatherExtended"};
            return f;
        }
        case vkt::Feature::shaderStorageImageExtendedFormats: {
            phys_dev_features_.features.shaderStorageImageExtendedFormats = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageImageExtendedFormats,
                             "VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats"};
            return f;
        }
        case vkt::Feature::shaderStorageImageMultisample: {
            phys_dev_features_.features.shaderStorageImageMultisample = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageImageMultisample,
                             "VkPhysicalDeviceFeatures::shaderStorageImageMultisample"};
            return f;
        }
        case vkt::Feature::shaderStorageImageReadWithoutFormat: {
            phys_dev_features_.features.shaderStorageImageReadWithoutFormat = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageImageReadWithoutFormat,
                             "VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat"};
            return f;
        }
        case vkt::Feature::shaderStorageImageWriteWithoutFormat: {
            phys_dev_features_.features.shaderStorageImageWriteWithoutFormat = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageImageWriteWithoutFormat,
                             "VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat"};
            return f;
        }
        case vkt::Feature::shaderUniformBufferArrayDynamicIndexing: {
            phys_dev_features_.features.shaderUniformBufferArrayDynamicIndexing = value;
            FeatureAndName f{&phys_dev_features_.features.shaderUniformBufferArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderSampledImageArrayDynamicIndexing: {
            phys_dev_features_.features.shaderSampledImageArrayDynamicIndexing = value;
            FeatureAndName f{&phys_dev_features_.features.shaderSampledImageArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderStorageBufferArrayDynamicIndexing: {
            phys_dev_features_.features.shaderStorageBufferArrayDynamicIndexing = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageBufferArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderStorageImageArrayDynamicIndexing: {
            phys_dev_features_.features.shaderStorageImageArrayDynamicIndexing = value;
            FeatureAndName f{&phys_dev_features_.features.shaderStorageImageArrayDynamicIndexing,
                             "VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing"};
            return f;
        }
        case vkt::Feature::shaderClipDistance: {
            phys_dev_features_.features.shaderClipDistance = value;
            FeatureAndName f{&phys_dev_features_.features.shaderClipDistance, "VkPhysicalDeviceFeatures::shaderClipDistance"};
            return f;
        }
        case vkt::Feature::shaderCullDistance: {
            phys_dev_features_.features.shaderCullDistance = value;
            FeatureAndName f{&phys_dev_features_.features.shaderCullDistance, "VkPhysicalDeviceFeatures::shaderCullDistance"};
            return f;
        }
        case vkt::Feature::shaderFloat64: {
            phys_dev_features_.features.shaderFloat64 = value;
            FeatureAndName f{&phys_dev_features_.features.shaderFloat64, "VkPhysicalDeviceFeatures::shaderFloat64"};
            return f;
        }
        case vkt::Feature::shaderInt64: {
            phys_dev_features_.features.shaderInt64 = value;
            FeatureAndName f{&phys_dev_features_.features.shaderInt64, "VkPhysicalDeviceFeatures::shaderInt64"};
            return f;
        }
        case vkt::Feature::shaderInt16: {
            phys_dev_features_.features.shaderInt16 = value;
            FeatureAndName f{&phys_dev_features_.features.shaderInt16, "VkPhysicalDeviceFeatures::shaderInt16"};
            return f;
        }
        case vkt::Feature::shaderResourceResidency: {
            phys_dev_features_.features.shaderResourceResidency = value;
            FeatureAndName f{&phys_dev_features_.features.shaderResourceResidency,
                             "VkPhysicalDeviceFeatures::shaderResourceResidency"};
            return f;
        }
        case vkt::Feature::shaderResourceMinLod: {
            phys_dev_features_.features.shaderResourceMinLod = value;
            FeatureAndName f{&phys_dev_features_.features.shaderResourceMinLod, "VkPhysicalDeviceFeatures::shaderResourceMinLod"};
            return f;
        }
        case vkt::Feature::sparseBinding: {
            phys_dev_features_.features.sparseBinding = value;
            FeatureAndName f{&phys_dev_features_.features.sparseBinding, "VkPhysicalDeviceFeatures::sparseBinding"};
            return f;
        }
        case vkt::Feature::sparseResidencyBuffer: {
            phys_dev_features_.features.sparseResidencyBuffer = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidencyBuffer, "VkPhysicalDeviceFeatures::sparseResidencyBuffer"};
            return f;
        }
        case vkt::Feature::sparseResidencyImage2D: {
            phys_dev_features_.features.sparseResidencyImage2D = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidencyImage2D,
                             "VkPhysicalDeviceFeatures::sparseResidencyImage2D"};
            return f;
        }
        case vkt::Feature::sparseResidencyImage3D: {
            phys_dev_features_.features.sparseResidencyImage3D = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidencyImage3D,
                             "VkPhysicalDeviceFeatures::sparseResidencyImage3D"};
            return f;
        }
        case vkt::Feature::sparseResidency2Samples: {
            phys_dev_features_.features.sparseResidency2Samples = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidency2Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency2Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency4Samples: {
            phys_dev_features_.features.sparseResidency4Samples = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidency4Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency4Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency8Samples: {
            phys_dev_features_.features.sparseResidency8Samples = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidency8Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency8Samples"};
            return f;
        }
        case vkt::Feature::sparseResidency16Samples: {
            phys_dev_features_.features.sparseResidency16Samples = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidency16Samples,
                             "VkPhysicalDeviceFeatures::sparseResidency16Samples"};
            return f;
        }
        case vkt::Feature::sparseResidencyAliased: {
            phys_dev_features_.features.sparseResidencyAliased = value;
            FeatureAndName f{&phys_dev_features_.features.sparseResidencyAliased,
                             "VkPhysicalDeviceFeatures::sparseResidencyAliased"};
            return f;
        }
        case vkt::Feature::variableMultisampleRate: {
            phys_dev_features_.features.variableMultisampleRate = value;
            FeatureAndName f{&phys_dev_features_.features.variableMultisampleRate,
                             "VkPhysicalDeviceFeatures::variableMultisampleRate"};
            return f;
        }
        case vkt::Feature::inheritedQueries: {
            phys_dev_features_.features.inheritedQueries = value;
            FeatureAndName f{&phys_dev_features_.features.inheritedQueries, "VkPhysicalDeviceFeatures::inheritedQueries"};
            return f;
        }
        default:
            FeatureAndName f = vkt::AddFeature(api_version, feature, &feature_chain_);
            return f;
    }
}

VkPhysicalDeviceFeatures2* FeatureRequirements::GetFeatures2() {
    phys_dev_features_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    phys_dev_features_.pNext = feature_chain_;
    return &phys_dev_features_;
}

const char* FeatureRequirements::AnyRequiredFeatureDisabled() const {
    for (const auto [feature, name] : required_features_) {
        if (*feature == VK_FALSE) {
            return name;
        }
    }
    return nullptr;
}

void FeatureRequirements::EnforceRequiredFeatures() {
    for (const auto [feature, name] : required_features_) {
        *feature = VK_TRUE;
    }
}

}  // namespace vkt