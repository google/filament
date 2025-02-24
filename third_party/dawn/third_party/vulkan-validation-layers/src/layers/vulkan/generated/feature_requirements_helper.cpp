// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See feature_requirements.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/

// NOLINTBEGIN

#include "generated/feature_requirements_helper.h"

#include "generated/pnext_chain_extraction.h"

#include <vulkan/utility/vk_struct_helper.hpp>

namespace vkt {
FeatureAndName AddFeature(APIVersion api_version, vkt::Feature feature, void **inout_pnext_chain) {
    switch (feature) {
        case Feature::storageBuffer16BitAccess:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageBuffer16BitAccess, "VkPhysicalDeviceVulkan11Features::storageBuffer16BitAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice16BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice16BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice16BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageBuffer16BitAccess, "VkPhysicalDevice16BitStorageFeatures::storageBuffer16BitAccess"};
            }
        case Feature::storageInputOutput16:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageInputOutput16, "VkPhysicalDeviceVulkan11Features::storageInputOutput16"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice16BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice16BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice16BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageInputOutput16, "VkPhysicalDevice16BitStorageFeatures::storageInputOutput16"};
            }
        case Feature::storagePushConstant16:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storagePushConstant16, "VkPhysicalDeviceVulkan11Features::storagePushConstant16"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice16BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice16BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice16BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storagePushConstant16, "VkPhysicalDevice16BitStorageFeatures::storagePushConstant16"};
            }
        case Feature::uniformAndStorageBuffer16BitAccess:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformAndStorageBuffer16BitAccess,
                        "VkPhysicalDeviceVulkan11Features::uniformAndStorageBuffer16BitAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice16BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice16BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice16BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformAndStorageBuffer16BitAccess,
                        "VkPhysicalDevice16BitStorageFeatures::uniformAndStorageBuffer16BitAccess"};
            }
        case Feature::formatA4B4G4R4: {
            auto vk_struct = const_cast<VkPhysicalDevice4444FormatsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevice4444FormatsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevice4444FormatsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->formatA4B4G4R4, "VkPhysicalDevice4444FormatsFeaturesEXT::formatA4B4G4R4"};
        }

        case Feature::formatA4R4G4B4: {
            auto vk_struct = const_cast<VkPhysicalDevice4444FormatsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevice4444FormatsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevice4444FormatsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->formatA4R4G4B4, "VkPhysicalDevice4444FormatsFeaturesEXT::formatA4R4G4B4"};
        }

        case Feature::storageBuffer8BitAccess:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageBuffer8BitAccess, "VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice8BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storageBuffer8BitAccess, "VkPhysicalDevice8BitStorageFeatures::storageBuffer8BitAccess"};
            }
        case Feature::storagePushConstant8:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storagePushConstant8, "VkPhysicalDeviceVulkan12Features::storagePushConstant8"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice8BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->storagePushConstant8, "VkPhysicalDevice8BitStorageFeatures::storagePushConstant8"};
            }
        case Feature::uniformAndStorageBuffer8BitAccess:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformAndStorageBuffer8BitAccess,
                        "VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevice8BitStorageFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformAndStorageBuffer8BitAccess,
                        "VkPhysicalDevice8BitStorageFeatures::uniformAndStorageBuffer8BitAccess"};
            }
        case Feature::decodeModeSharedExponent: {
            auto vk_struct = const_cast<VkPhysicalDeviceASTCDecodeFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceASTCDecodeFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceASTCDecodeFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->decodeModeSharedExponent, "VkPhysicalDeviceASTCDecodeFeaturesEXT::decodeModeSharedExponent"};
        }

        case Feature::accelerationStructure: {
            auto vk_struct = const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAccelerationStructureFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->accelerationStructure, "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructure"};
        }

        case Feature::accelerationStructureCaptureReplay: {
            auto vk_struct = const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAccelerationStructureFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->accelerationStructureCaptureReplay,
                    "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureCaptureReplay"};
        }

        case Feature::accelerationStructureHostCommands: {
            auto vk_struct = const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAccelerationStructureFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->accelerationStructureHostCommands,
                    "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureHostCommands"};
        }

        case Feature::accelerationStructureIndirectBuild: {
            auto vk_struct = const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAccelerationStructureFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->accelerationStructureIndirectBuild,
                    "VkPhysicalDeviceAccelerationStructureFeaturesKHR::accelerationStructureIndirectBuild"};
        }

        case Feature::descriptorBindingAccelerationStructureUpdateAfterBind: {
            auto vk_struct = const_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAccelerationStructureFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAccelerationStructureFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorBindingAccelerationStructureUpdateAfterBind,
                    "VkPhysicalDeviceAccelerationStructureFeaturesKHR::descriptorBindingAccelerationStructureUpdateAfterBind"};
        }

        case Feature::reportAddressBinding: {
            auto vk_struct = const_cast<VkPhysicalDeviceAddressBindingReportFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAddressBindingReportFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAddressBindingReportFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->reportAddressBinding, "VkPhysicalDeviceAddressBindingReportFeaturesEXT::reportAddressBinding"};
        }

        case Feature::amigoProfiling: {
            auto vk_struct = const_cast<VkPhysicalDeviceAmigoProfilingFeaturesSEC *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAmigoProfilingFeaturesSEC>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAmigoProfilingFeaturesSEC;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->amigoProfiling, "VkPhysicalDeviceAmigoProfilingFeaturesSEC::amigoProfiling"};
        }

        case Feature::antiLag: {
            auto vk_struct = const_cast<VkPhysicalDeviceAntiLagFeaturesAMD *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAntiLagFeaturesAMD>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAntiLagFeaturesAMD;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->antiLag, "VkPhysicalDeviceAntiLagFeaturesAMD::antiLag"};
        }

        case Feature::attachmentFeedbackLoopDynamicState: {
            auto vk_struct = const_cast<VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->attachmentFeedbackLoopDynamicState,
                    "VkPhysicalDeviceAttachmentFeedbackLoopDynamicStateFeaturesEXT::attachmentFeedbackLoopDynamicState"};
        }

        case Feature::attachmentFeedbackLoopLayout: {
            auto vk_struct = const_cast<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->attachmentFeedbackLoopLayout,
                    "VkPhysicalDeviceAttachmentFeedbackLoopLayoutFeaturesEXT::attachmentFeedbackLoopLayout"};
        }

        case Feature::advancedBlendCoherentOperations: {
            auto vk_struct = const_cast<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->advancedBlendCoherentOperations,
                    "VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT::advancedBlendCoherentOperations"};
        }

        case Feature::borderColorSwizzle: {
            auto vk_struct = const_cast<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceBorderColorSwizzleFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->borderColorSwizzle, "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::borderColorSwizzle"};
        }

        case Feature::borderColorSwizzleFromImage: {
            auto vk_struct = const_cast<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceBorderColorSwizzleFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceBorderColorSwizzleFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->borderColorSwizzleFromImage,
                    "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT::borderColorSwizzleFromImage"};
        }

        case Feature::bufferDeviceAddress:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddress, "VkPhysicalDeviceVulkan12Features::bufferDeviceAddress"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceBufferDeviceAddressFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddress, "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress"};
            }
        case Feature::bufferDeviceAddressCaptureReplay:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddressCaptureReplay,
                        "VkPhysicalDeviceVulkan12Features::bufferDeviceAddressCaptureReplay"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceBufferDeviceAddressFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddressCaptureReplay,
                        "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddressCaptureReplay"};
            }
        case Feature::bufferDeviceAddressMultiDevice:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddressMultiDevice,
                        "VkPhysicalDeviceVulkan12Features::bufferDeviceAddressMultiDevice"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceBufferDeviceAddressFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bufferDeviceAddressMultiDevice,
                        "VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddressMultiDevice"};
            }
        case Feature::clusterAccelerationStructure: {
            auto vk_struct = const_cast<VkPhysicalDeviceClusterAccelerationStructureFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceClusterAccelerationStructureFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceClusterAccelerationStructureFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->clusterAccelerationStructure,
                    "VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::clusterAccelerationStructure"};
        }

        case Feature::clustercullingShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->clustercullingShader, "VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::clustercullingShader"};
        }

        case Feature::multiviewClusterCullingShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multiviewClusterCullingShader,
                    "VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::multiviewClusterCullingShader"};
        }

        case Feature::deviceCoherentMemory: {
            auto vk_struct = const_cast<VkPhysicalDeviceCoherentMemoryFeaturesAMD *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCoherentMemoryFeaturesAMD>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCoherentMemoryFeaturesAMD;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceCoherentMemory, "VkPhysicalDeviceCoherentMemoryFeaturesAMD::deviceCoherentMemory"};
        }

        case Feature::colorWriteEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceColorWriteEnableFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceColorWriteEnableFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceColorWriteEnableFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->colorWriteEnable, "VkPhysicalDeviceColorWriteEnableFeaturesEXT::colorWriteEnable"};
        }

        case Feature::commandBufferInheritance: {
            auto vk_struct = const_cast<VkPhysicalDeviceCommandBufferInheritanceFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCommandBufferInheritanceFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCommandBufferInheritanceFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->commandBufferInheritance,
                    "VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::commandBufferInheritance"};
        }

        case Feature::computeDerivativeGroupLinear: {
            auto vk_struct = const_cast<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->computeDerivativeGroupLinear,
                    "VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupLinear"};
        }

        case Feature::computeDerivativeGroupQuads: {
            auto vk_struct = const_cast<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->computeDerivativeGroupQuads,
                    "VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR::computeDerivativeGroupQuads"};
        }

        case Feature::conditionalRendering: {
            auto vk_struct = const_cast<VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceConditionalRenderingFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->conditionalRendering, "VkPhysicalDeviceConditionalRenderingFeaturesEXT::conditionalRendering"};
        }

        case Feature::inheritedConditionalRendering: {
            auto vk_struct = const_cast<VkPhysicalDeviceConditionalRenderingFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceConditionalRenderingFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceConditionalRenderingFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->inheritedConditionalRendering,
                    "VkPhysicalDeviceConditionalRenderingFeaturesEXT::inheritedConditionalRendering"};
        }

        case Feature::cooperativeMatrixBlockLoads: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixBlockLoads,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixBlockLoads"};
        }

        case Feature::cooperativeMatrixConversions: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixConversions,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixConversions"};
        }

        case Feature::cooperativeMatrixFlexibleDimensions: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixFlexibleDimensions,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixFlexibleDimensions"};
        }

        case Feature::cooperativeMatrixPerElementOperations: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixPerElementOperations,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixPerElementOperations"};
        }

        case Feature::cooperativeMatrixReductions: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixReductions,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixReductions"};
        }

        case Feature::cooperativeMatrixTensorAddressing: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixTensorAddressing,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixTensorAddressing"};
        }

        case Feature::cooperativeMatrixWorkgroupScope: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrix2FeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrix2FeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrix2FeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixWorkgroupScope,
                    "VkPhysicalDeviceCooperativeMatrix2FeaturesNV::cooperativeMatrixWorkgroupScope"};
        }

        case Feature::cooperativeMatrix: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrixFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrix, "VkPhysicalDeviceCooperativeMatrixFeaturesKHR::cooperativeMatrix"};
        }

        case Feature::cooperativeMatrixRobustBufferAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeMatrixFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeMatrixFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeMatrixFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeMatrixRobustBufferAccess,
                    "VkPhysicalDeviceCooperativeMatrixFeaturesKHR::cooperativeMatrixRobustBufferAccess"};
        }

        case Feature::cooperativeVector: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeVectorFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeVectorFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeVectorFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeVector, "VkPhysicalDeviceCooperativeVectorFeaturesNV::cooperativeVector"};
        }

        case Feature::cooperativeVectorTraining: {
            auto vk_struct = const_cast<VkPhysicalDeviceCooperativeVectorFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCooperativeVectorFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCooperativeVectorFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cooperativeVectorTraining,
                    "VkPhysicalDeviceCooperativeVectorFeaturesNV::cooperativeVectorTraining"};
        }

        case Feature::indirectCopy: {
            auto vk_struct = const_cast<VkPhysicalDeviceCopyMemoryIndirectFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCopyMemoryIndirectFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCopyMemoryIndirectFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->indirectCopy, "VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::indirectCopy"};
        }

        case Feature::cornerSampledImage: {
            auto vk_struct = const_cast<VkPhysicalDeviceCornerSampledImageFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCornerSampledImageFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCornerSampledImageFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cornerSampledImage, "VkPhysicalDeviceCornerSampledImageFeaturesNV::cornerSampledImage"};
        }

        case Feature::coverageReductionMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceCoverageReductionModeFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCoverageReductionModeFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCoverageReductionModeFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->coverageReductionMode, "VkPhysicalDeviceCoverageReductionModeFeaturesNV::coverageReductionMode"};
        }

        case Feature::cubicRangeClamp: {
            auto vk_struct = const_cast<VkPhysicalDeviceCubicClampFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCubicClampFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCubicClampFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cubicRangeClamp, "VkPhysicalDeviceCubicClampFeaturesQCOM::cubicRangeClamp"};
        }

        case Feature::selectableCubicWeights: {
            auto vk_struct = const_cast<VkPhysicalDeviceCubicWeightsFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCubicWeightsFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCubicWeightsFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->selectableCubicWeights, "VkPhysicalDeviceCubicWeightsFeaturesQCOM::selectableCubicWeights"};
        }

        case Feature::cudaKernelLaunchFeatures: {
            auto vk_struct = const_cast<VkPhysicalDeviceCudaKernelLaunchFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCudaKernelLaunchFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCudaKernelLaunchFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->cudaKernelLaunchFeatures, "VkPhysicalDeviceCudaKernelLaunchFeaturesNV::cudaKernelLaunchFeatures"};
        }

        case Feature::customBorderColorWithoutFormat: {
            auto vk_struct = const_cast<VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCustomBorderColorFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCustomBorderColorFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->customBorderColorWithoutFormat,
                    "VkPhysicalDeviceCustomBorderColorFeaturesEXT::customBorderColorWithoutFormat"};
        }

        case Feature::customBorderColors: {
            auto vk_struct = const_cast<VkPhysicalDeviceCustomBorderColorFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceCustomBorderColorFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceCustomBorderColorFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->customBorderColors, "VkPhysicalDeviceCustomBorderColorFeaturesEXT::customBorderColors"};
        }

        case Feature::dedicatedAllocationImageAliasing: {
            auto vk_struct = const_cast<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->dedicatedAllocationImageAliasing,
                    "VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::dedicatedAllocationImageAliasing"};
        }

        case Feature::depthBiasControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthBiasControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthBiasControl, "VkPhysicalDeviceDepthBiasControlFeaturesEXT::depthBiasControl"};
        }

        case Feature::depthBiasExact: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthBiasControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthBiasExact, "VkPhysicalDeviceDepthBiasControlFeaturesEXT::depthBiasExact"};
        }

        case Feature::floatRepresentation: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthBiasControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->floatRepresentation, "VkPhysicalDeviceDepthBiasControlFeaturesEXT::floatRepresentation"};
        }

        case Feature::leastRepresentableValueForceUnormRepresentation: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthBiasControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthBiasControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthBiasControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->leastRepresentableValueForceUnormRepresentation,
                    "VkPhysicalDeviceDepthBiasControlFeaturesEXT::leastRepresentableValueForceUnormRepresentation"};
        }

        case Feature::depthClampControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthClampControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthClampControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthClampControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthClampControl, "VkPhysicalDeviceDepthClampControlFeaturesEXT::depthClampControl"};
        }

        case Feature::depthClampZeroOne: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthClampZeroOneFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthClampZeroOneFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthClampZeroOneFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthClampZeroOne, "VkPhysicalDeviceDepthClampZeroOneFeaturesKHR::depthClampZeroOne"};
        }

        case Feature::depthClipControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthClipControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthClipControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthClipControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthClipControl, "VkPhysicalDeviceDepthClipControlFeaturesEXT::depthClipControl"};
        }

        case Feature::depthClipEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceDepthClipEnableFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDepthClipEnableFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDepthClipEnableFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->depthClipEnable, "VkPhysicalDeviceDepthClipEnableFeaturesEXT::depthClipEnable"};
        }

        case Feature::descriptorBuffer: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorBuffer, "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBuffer"};
        }

        case Feature::descriptorBufferCaptureReplay: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorBufferCaptureReplay,
                    "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferCaptureReplay"};
        }

        case Feature::descriptorBufferImageLayoutIgnored: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorBufferImageLayoutIgnored,
                    "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferImageLayoutIgnored"};
        }

        case Feature::descriptorBufferPushDescriptors: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorBufferPushDescriptors,
                    "VkPhysicalDeviceDescriptorBufferFeaturesEXT::descriptorBufferPushDescriptors"};
        }

        case Feature::descriptorBindingPartiallyBound:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingPartiallyBound,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingPartiallyBound"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingPartiallyBound,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingPartiallyBound"};
            }
        case Feature::descriptorBindingSampledImageUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingSampledImageUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingSampledImageUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingSampledImageUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingSampledImageUpdateAfterBind"};
            }
        case Feature::descriptorBindingStorageBufferUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageBufferUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageBufferUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageBufferUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageBufferUpdateAfterBind"};
            }
        case Feature::descriptorBindingStorageImageUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageImageUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageImageUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageImageUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageImageUpdateAfterBind"};
            }
        case Feature::descriptorBindingStorageTexelBufferUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageTexelBufferUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingStorageTexelBufferUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingStorageTexelBufferUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageTexelBufferUpdateAfterBind"};
            }
        case Feature::descriptorBindingUniformBufferUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUniformBufferUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingUniformBufferUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUniformBufferUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformBufferUpdateAfterBind"};
            }
        case Feature::descriptorBindingUniformTexelBufferUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUniformTexelBufferUpdateAfterBind,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingUniformTexelBufferUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUniformTexelBufferUpdateAfterBind,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformTexelBufferUpdateAfterBind"};
            }
        case Feature::descriptorBindingUpdateUnusedWhilePending:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUpdateUnusedWhilePending,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingUpdateUnusedWhilePending"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingUpdateUnusedWhilePending,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUpdateUnusedWhilePending"};
            }
        case Feature::descriptorBindingVariableDescriptorCount:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingVariableDescriptorCount,
                        "VkPhysicalDeviceVulkan12Features::descriptorBindingVariableDescriptorCount"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingVariableDescriptorCount,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingVariableDescriptorCount"};
            }
        case Feature::runtimeDescriptorArray:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->runtimeDescriptorArray, "VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->runtimeDescriptorArray, "VkPhysicalDeviceDescriptorIndexingFeatures::runtimeDescriptorArray"};
            }
        case Feature::shaderInputAttachmentArrayDynamicIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInputAttachmentArrayDynamicIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInputAttachmentArrayDynamicIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayDynamicIndexing"};
            }
        case Feature::shaderInputAttachmentArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInputAttachmentArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInputAttachmentArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayNonUniformIndexing"};
            }
        case Feature::shaderSampledImageArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSampledImageArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSampledImageArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderSampledImageArrayNonUniformIndexing"};
            }
        case Feature::shaderStorageBufferArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageBufferArrayNonUniformIndexing"};
            }
        case Feature::shaderStorageImageArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageImageArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageImageArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageImageArrayNonUniformIndexing"};
            }
        case Feature::shaderStorageTexelBufferArrayDynamicIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageTexelBufferArrayDynamicIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageTexelBufferArrayDynamicIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayDynamicIndexing"};
            }
        case Feature::shaderStorageTexelBufferArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageTexelBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderStorageTexelBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayNonUniformIndexing"};
            }
        case Feature::shaderUniformBufferArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformBufferArrayNonUniformIndexing"};
            }
        case Feature::shaderUniformTexelBufferArrayDynamicIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformTexelBufferArrayDynamicIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformTexelBufferArrayDynamicIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayDynamicIndexing"};
            }
        case Feature::shaderUniformTexelBufferArrayNonUniformIndexing:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformTexelBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayNonUniformIndexing"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDescriptorIndexingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorIndexingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDescriptorIndexingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderUniformTexelBufferArrayNonUniformIndexing,
                        "VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayNonUniformIndexing"};
            }
        case Feature::descriptorPoolOverallocation: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorPoolOverallocation,
                    "VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::descriptorPoolOverallocation"};
        }

        case Feature::descriptorSetHostMapping: {
            auto vk_struct = const_cast<VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorSetHostMapping,
                    "VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::descriptorSetHostMapping"};
        }

        case Feature::deviceGeneratedCompute: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceGeneratedCompute,
                    "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedCompute"};
        }

        case Feature::deviceGeneratedComputeCaptureReplay: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceGeneratedComputeCaptureReplay,
                    "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedComputeCaptureReplay"};
        }

        case Feature::deviceGeneratedComputePipelines: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceGeneratedComputePipelines,
                    "VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::deviceGeneratedComputePipelines"};
        }

        case Feature::deviceGeneratedCommands: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceGeneratedCommands,
                    "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::deviceGeneratedCommands"};
        }

        case Feature::dynamicGeneratedPipelineLayout: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->dynamicGeneratedPipelineLayout,
                    "VkPhysicalDeviceDeviceGeneratedCommandsFeaturesEXT::dynamicGeneratedPipelineLayout"};
        }

        case Feature::deviceMemoryReport: {
            auto vk_struct = const_cast<VkPhysicalDeviceDeviceMemoryReportFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDeviceMemoryReportFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDeviceMemoryReportFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceMemoryReport, "VkPhysicalDeviceDeviceMemoryReportFeaturesEXT::deviceMemoryReport"};
        }

        case Feature::diagnosticsConfig: {
            auto vk_struct = const_cast<VkPhysicalDeviceDiagnosticsConfigFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDiagnosticsConfigFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDiagnosticsConfigFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->diagnosticsConfig, "VkPhysicalDeviceDiagnosticsConfigFeaturesNV::diagnosticsConfig"};
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::displacementMicromap: {
            auto vk_struct = const_cast<VkPhysicalDeviceDisplacementMicromapFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDisplacementMicromapFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDisplacementMicromapFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->displacementMicromap, "VkPhysicalDeviceDisplacementMicromapFeaturesNV::displacementMicromap"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS

        case Feature::dynamicRendering:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->dynamicRendering, "VkPhysicalDeviceVulkan13Features::dynamicRendering"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDynamicRenderingFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDynamicRenderingFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDynamicRenderingFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->dynamicRendering, "VkPhysicalDeviceDynamicRenderingFeatures::dynamicRendering"};
            }
        case Feature::dynamicRenderingLocalRead:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->dynamicRenderingLocalRead, "VkPhysicalDeviceVulkan14Features::dynamicRenderingLocalRead"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceDynamicRenderingLocalReadFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceDynamicRenderingLocalReadFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceDynamicRenderingLocalReadFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->dynamicRenderingLocalRead,
                        "VkPhysicalDeviceDynamicRenderingLocalReadFeatures::dynamicRenderingLocalRead"};
            }
        case Feature::dynamicRenderingUnusedAttachments: {
            auto vk_struct = const_cast<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->dynamicRenderingUnusedAttachments,
                    "VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT::dynamicRenderingUnusedAttachments"};
        }

        case Feature::exclusiveScissor: {
            auto vk_struct = const_cast<VkPhysicalDeviceExclusiveScissorFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExclusiveScissorFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExclusiveScissorFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->exclusiveScissor, "VkPhysicalDeviceExclusiveScissorFeaturesNV::exclusiveScissor"};
        }

        case Feature::extendedDynamicState2: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState2, "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2"};
        }

        case Feature::extendedDynamicState2LogicOp: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState2LogicOp,
                    "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2LogicOp"};
        }

        case Feature::extendedDynamicState2PatchControlPoints: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState2PatchControlPoints,
                    "VkPhysicalDeviceExtendedDynamicState2FeaturesEXT::extendedDynamicState2PatchControlPoints"};
        }

        case Feature::extendedDynamicState3AlphaToCoverageEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3AlphaToCoverageEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3AlphaToCoverageEnable"};
        }

        case Feature::extendedDynamicState3AlphaToOneEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3AlphaToOneEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3AlphaToOneEnable"};
        }

        case Feature::extendedDynamicState3ColorBlendAdvanced: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ColorBlendAdvanced,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendAdvanced"};
        }

        case Feature::extendedDynamicState3ColorBlendEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ColorBlendEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendEnable"};
        }

        case Feature::extendedDynamicState3ColorBlendEquation: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ColorBlendEquation,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorBlendEquation"};
        }

        case Feature::extendedDynamicState3ColorWriteMask: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ColorWriteMask,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ColorWriteMask"};
        }

        case Feature::extendedDynamicState3ConservativeRasterizationMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ConservativeRasterizationMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ConservativeRasterizationMode"};
        }

        case Feature::extendedDynamicState3CoverageModulationMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageModulationMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationMode"};
        }

        case Feature::extendedDynamicState3CoverageModulationTable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageModulationTable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationTable"};
        }

        case Feature::extendedDynamicState3CoverageModulationTableEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageModulationTableEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageModulationTableEnable"};
        }

        case Feature::extendedDynamicState3CoverageReductionMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageReductionMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageReductionMode"};
        }

        case Feature::extendedDynamicState3CoverageToColorEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageToColorEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageToColorEnable"};
        }

        case Feature::extendedDynamicState3CoverageToColorLocation: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3CoverageToColorLocation,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3CoverageToColorLocation"};
        }

        case Feature::extendedDynamicState3DepthClampEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3DepthClampEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClampEnable"};
        }

        case Feature::extendedDynamicState3DepthClipEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3DepthClipEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClipEnable"};
        }

        case Feature::extendedDynamicState3DepthClipNegativeOneToOne: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3DepthClipNegativeOneToOne,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3DepthClipNegativeOneToOne"};
        }

        case Feature::extendedDynamicState3ExtraPrimitiveOverestimationSize: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ExtraPrimitiveOverestimationSize,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ExtraPrimitiveOverestimationSize"};
        }

        case Feature::extendedDynamicState3LineRasterizationMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3LineRasterizationMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LineRasterizationMode"};
        }

        case Feature::extendedDynamicState3LineStippleEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3LineStippleEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LineStippleEnable"};
        }

        case Feature::extendedDynamicState3LogicOpEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3LogicOpEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3LogicOpEnable"};
        }

        case Feature::extendedDynamicState3PolygonMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3PolygonMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3PolygonMode"};
        }

        case Feature::extendedDynamicState3ProvokingVertexMode: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ProvokingVertexMode,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ProvokingVertexMode"};
        }

        case Feature::extendedDynamicState3RasterizationSamples: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3RasterizationSamples,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RasterizationSamples"};
        }

        case Feature::extendedDynamicState3RasterizationStream: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3RasterizationStream,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RasterizationStream"};
        }

        case Feature::extendedDynamicState3RepresentativeFragmentTestEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3RepresentativeFragmentTestEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3RepresentativeFragmentTestEnable"};
        }

        case Feature::extendedDynamicState3SampleLocationsEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3SampleLocationsEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3SampleLocationsEnable"};
        }

        case Feature::extendedDynamicState3SampleMask: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3SampleMask,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3SampleMask"};
        }

        case Feature::extendedDynamicState3ShadingRateImageEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ShadingRateImageEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ShadingRateImageEnable"};
        }

        case Feature::extendedDynamicState3TessellationDomainOrigin: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3TessellationDomainOrigin,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3TessellationDomainOrigin"};
        }

        case Feature::extendedDynamicState3ViewportSwizzle: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ViewportSwizzle,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ViewportSwizzle"};
        }

        case Feature::extendedDynamicState3ViewportWScalingEnable: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicState3FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicState3FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState3ViewportWScalingEnable,
                    "VkPhysicalDeviceExtendedDynamicState3FeaturesEXT::extendedDynamicState3ViewportWScalingEnable"};
        }

        case Feature::extendedDynamicState: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedDynamicStateFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedDynamicState, "VkPhysicalDeviceExtendedDynamicStateFeaturesEXT::extendedDynamicState"};
        }

        case Feature::extendedSparseAddressSpace: {
            auto vk_struct = const_cast<VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->extendedSparseAddressSpace,
                    "VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::extendedSparseAddressSpace"};
        }
#ifdef VK_USE_PLATFORM_ANDROID_KHR

        case Feature::externalFormatResolve: {
            auto vk_struct = const_cast<VkPhysicalDeviceExternalFormatResolveFeaturesANDROID *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExternalFormatResolveFeaturesANDROID>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExternalFormatResolveFeaturesANDROID;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->externalFormatResolve,
                    "VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::externalFormatResolve"};
        }
#endif  // VK_USE_PLATFORM_ANDROID_KHR

        case Feature::externalMemoryRDMA: {
            auto vk_struct = const_cast<VkPhysicalDeviceExternalMemoryRDMAFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExternalMemoryRDMAFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExternalMemoryRDMAFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->externalMemoryRDMA, "VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::externalMemoryRDMA"};
        }
#ifdef VK_USE_PLATFORM_SCREEN_QNX

        case Feature::screenBufferImport: {
            auto vk_struct = const_cast<VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->screenBufferImport, "VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::screenBufferImport"};
        }
#endif  // VK_USE_PLATFORM_SCREEN_QNX

        case Feature::deviceFault: {
            auto vk_struct = const_cast<VkPhysicalDeviceFaultFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFaultFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFaultFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceFault, "VkPhysicalDeviceFaultFeaturesEXT::deviceFault"};
        }

        case Feature::deviceFaultVendorBinary: {
            auto vk_struct = const_cast<VkPhysicalDeviceFaultFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFaultFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFaultFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->deviceFaultVendorBinary, "VkPhysicalDeviceFaultFeaturesEXT::deviceFaultVendorBinary"};
        }

        case Feature::fragmentDensityMapDeferred: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMap2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentDensityMap2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentDensityMapDeferred,
                    "VkPhysicalDeviceFragmentDensityMap2FeaturesEXT::fragmentDensityMapDeferred"};
        }

        case Feature::fragmentDensityMap: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentDensityMapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentDensityMap, "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMap"};
        }

        case Feature::fragmentDensityMapDynamic: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentDensityMapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentDensityMapDynamic,
                    "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMapDynamic"};
        }

        case Feature::fragmentDensityMapNonSubsampledImages: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentDensityMapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentDensityMapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentDensityMapNonSubsampledImages,
                    "VkPhysicalDeviceFragmentDensityMapFeaturesEXT::fragmentDensityMapNonSubsampledImages"};
        }

        case Feature::fragmentDensityMapOffset: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentDensityMapOffset,
                    "VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::fragmentDensityMapOffset"};
        }

        case Feature::fragmentShaderBarycentric: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentShaderBarycentric,
                    "VkPhysicalDeviceFragmentShaderBarycentricFeaturesKHR::fragmentShaderBarycentric"};
        }

        case Feature::fragmentShaderPixelInterlock: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentShaderPixelInterlock,
                    "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderPixelInterlock"};
        }

        case Feature::fragmentShaderSampleInterlock: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentShaderSampleInterlock,
                    "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderSampleInterlock"};
        }

        case Feature::fragmentShaderShadingRateInterlock: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentShaderShadingRateInterlock,
                    "VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT::fragmentShaderShadingRateInterlock"};
        }

        case Feature::fragmentShadingRateEnums: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->fragmentShadingRateEnums,
                    "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::fragmentShadingRateEnums"};
        }

        case Feature::noInvocationFragmentShadingRates: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->noInvocationFragmentShadingRates,
                    "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::noInvocationFragmentShadingRates"};
        }

        case Feature::supersampleFragmentShadingRates: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->supersampleFragmentShadingRates,
                    "VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::supersampleFragmentShadingRates"};
        }

        case Feature::attachmentFragmentShadingRate: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->attachmentFragmentShadingRate,
                    "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::attachmentFragmentShadingRate"};
        }

        case Feature::pipelineFragmentShadingRate: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelineFragmentShadingRate,
                    "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::pipelineFragmentShadingRate"};
        }

        case Feature::primitiveFragmentShadingRate: {
            auto vk_struct = const_cast<VkPhysicalDeviceFragmentShadingRateFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFragmentShadingRateFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFragmentShadingRateFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitiveFragmentShadingRate,
                    "VkPhysicalDeviceFragmentShadingRateFeaturesKHR::primitiveFragmentShadingRate"};
        }

        case Feature::frameBoundary: {
            auto vk_struct = const_cast<VkPhysicalDeviceFrameBoundaryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceFrameBoundaryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceFrameBoundaryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->frameBoundary, "VkPhysicalDeviceFrameBoundaryFeaturesEXT::frameBoundary"};
        }

        case Feature::globalPriorityQuery:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->globalPriorityQuery, "VkPhysicalDeviceVulkan14Features::globalPriorityQuery"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceGlobalPriorityQueryFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceGlobalPriorityQueryFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceGlobalPriorityQueryFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->globalPriorityQuery, "VkPhysicalDeviceGlobalPriorityQueryFeatures::globalPriorityQuery"};
            }
        case Feature::graphicsPipelineLibrary: {
            auto vk_struct = const_cast<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->graphicsPipelineLibrary,
                    "VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT::graphicsPipelineLibrary"};
        }

        case Feature::hdrVivid: {
            auto vk_struct = const_cast<VkPhysicalDeviceHdrVividFeaturesHUAWEI *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceHdrVividFeaturesHUAWEI>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceHdrVividFeaturesHUAWEI;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->hdrVivid, "VkPhysicalDeviceHdrVividFeaturesHUAWEI::hdrVivid"};
        }

        case Feature::hostImageCopy:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->hostImageCopy, "VkPhysicalDeviceVulkan14Features::hostImageCopy"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceHostImageCopyFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceHostImageCopyFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceHostImageCopyFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->hostImageCopy, "VkPhysicalDeviceHostImageCopyFeatures::hostImageCopy"};
            }
        case Feature::hostQueryReset:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->hostQueryReset, "VkPhysicalDeviceVulkan12Features::hostQueryReset"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceHostQueryResetFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceHostQueryResetFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceHostQueryResetFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->hostQueryReset, "VkPhysicalDeviceHostQueryResetFeatures::hostQueryReset"};
            }
        case Feature::image2DViewOf3D: {
            auto vk_struct = const_cast<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImage2DViewOf3DFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->image2DViewOf3D, "VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::image2DViewOf3D"};
        }

        case Feature::sampler2DViewOf3D: {
            auto vk_struct = const_cast<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImage2DViewOf3DFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImage2DViewOf3DFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->sampler2DViewOf3D, "VkPhysicalDeviceImage2DViewOf3DFeaturesEXT::sampler2DViewOf3D"};
        }

        case Feature::imageAlignmentControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageAlignmentControlFeaturesMESA *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageAlignmentControlFeaturesMESA>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageAlignmentControlFeaturesMESA;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageAlignmentControl, "VkPhysicalDeviceImageAlignmentControlFeaturesMESA::imageAlignmentControl"};
        }

        case Feature::imageCompressionControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageCompressionControlFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageCompressionControlFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageCompressionControlFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageCompressionControl,
                    "VkPhysicalDeviceImageCompressionControlFeaturesEXT::imageCompressionControl"};
        }

        case Feature::imageCompressionControlSwapchain: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageCompressionControlSwapchain,
                    "VkPhysicalDeviceImageCompressionControlSwapchainFeaturesEXT::imageCompressionControlSwapchain"};
        }

        case Feature::textureBlockMatch2: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageProcessing2FeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageProcessing2FeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageProcessing2FeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->textureBlockMatch2, "VkPhysicalDeviceImageProcessing2FeaturesQCOM::textureBlockMatch2"};
        }

        case Feature::textureBlockMatch: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageProcessingFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageProcessingFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageProcessingFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->textureBlockMatch, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBlockMatch"};
        }

        case Feature::textureBoxFilter: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageProcessingFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageProcessingFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageProcessingFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->textureBoxFilter, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureBoxFilter"};
        }

        case Feature::textureSampleWeighted: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageProcessingFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageProcessingFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageProcessingFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->textureSampleWeighted, "VkPhysicalDeviceImageProcessingFeaturesQCOM::textureSampleWeighted"};
        }

        case Feature::robustImageAccess:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->robustImageAccess, "VkPhysicalDeviceVulkan13Features::robustImageAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceImageRobustnessFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceImageRobustnessFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceImageRobustnessFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->robustImageAccess, "VkPhysicalDeviceImageRobustnessFeatures::robustImageAccess"};
            }
        case Feature::imageSlicedViewOf3D: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageSlicedViewOf3D, "VkPhysicalDeviceImageSlicedViewOf3DFeaturesEXT::imageSlicedViewOf3D"};
        }

        case Feature::minLod: {
            auto vk_struct = const_cast<VkPhysicalDeviceImageViewMinLodFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceImageViewMinLodFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceImageViewMinLodFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->minLod, "VkPhysicalDeviceImageViewMinLodFeaturesEXT::minLod"};
        }

        case Feature::imagelessFramebuffer:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->imagelessFramebuffer, "VkPhysicalDeviceVulkan12Features::imagelessFramebuffer"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceImagelessFramebufferFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceImagelessFramebufferFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceImagelessFramebufferFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->imagelessFramebuffer, "VkPhysicalDeviceImagelessFramebufferFeatures::imagelessFramebuffer"};
            }
        case Feature::indexTypeUint8:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->indexTypeUint8, "VkPhysicalDeviceVulkan14Features::indexTypeUint8"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceIndexTypeUint8Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceIndexTypeUint8Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceIndexTypeUint8Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->indexTypeUint8, "VkPhysicalDeviceIndexTypeUint8Features::indexTypeUint8"};
            }
        case Feature::inheritedViewportScissor2D: {
            auto vk_struct = const_cast<VkPhysicalDeviceInheritedViewportScissorFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceInheritedViewportScissorFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceInheritedViewportScissorFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->inheritedViewportScissor2D,
                    "VkPhysicalDeviceInheritedViewportScissorFeaturesNV::inheritedViewportScissor2D"};
        }

        case Feature::descriptorBindingInlineUniformBlockUpdateAfterBind:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingInlineUniformBlockUpdateAfterBind,
                        "VkPhysicalDeviceVulkan13Features::descriptorBindingInlineUniformBlockUpdateAfterBind"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceInlineUniformBlockFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceInlineUniformBlockFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceInlineUniformBlockFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->descriptorBindingInlineUniformBlockUpdateAfterBind,
                        "VkPhysicalDeviceInlineUniformBlockFeatures::descriptorBindingInlineUniformBlockUpdateAfterBind"};
            }
        case Feature::inlineUniformBlock:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->inlineUniformBlock, "VkPhysicalDeviceVulkan13Features::inlineUniformBlock"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceInlineUniformBlockFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceInlineUniformBlockFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceInlineUniformBlockFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->inlineUniformBlock, "VkPhysicalDeviceInlineUniformBlockFeatures::inlineUniformBlock"};
            }
        case Feature::invocationMask: {
            auto vk_struct = const_cast<VkPhysicalDeviceInvocationMaskFeaturesHUAWEI *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceInvocationMaskFeaturesHUAWEI>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceInvocationMaskFeaturesHUAWEI;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->invocationMask, "VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::invocationMask"};
        }

        case Feature::legacyDithering: {
            auto vk_struct = const_cast<VkPhysicalDeviceLegacyDitheringFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceLegacyDitheringFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceLegacyDitheringFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->legacyDithering, "VkPhysicalDeviceLegacyDitheringFeaturesEXT::legacyDithering"};
        }

        case Feature::legacyVertexAttributes: {
            auto vk_struct = const_cast<VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->legacyVertexAttributes,
                    "VkPhysicalDeviceLegacyVertexAttributesFeaturesEXT::legacyVertexAttributes"};
        }

        case Feature::bresenhamLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bresenhamLines, "VkPhysicalDeviceVulkan14Features::bresenhamLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->bresenhamLines, "VkPhysicalDeviceLineRasterizationFeatures::bresenhamLines"};
            }
        case Feature::rectangularLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->rectangularLines, "VkPhysicalDeviceVulkan14Features::rectangularLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->rectangularLines, "VkPhysicalDeviceLineRasterizationFeatures::rectangularLines"};
            }
        case Feature::smoothLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->smoothLines, "VkPhysicalDeviceVulkan14Features::smoothLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->smoothLines, "VkPhysicalDeviceLineRasterizationFeatures::smoothLines"};
            }
        case Feature::stippledBresenhamLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledBresenhamLines, "VkPhysicalDeviceVulkan14Features::stippledBresenhamLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledBresenhamLines, "VkPhysicalDeviceLineRasterizationFeatures::stippledBresenhamLines"};
            }
        case Feature::stippledRectangularLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledRectangularLines, "VkPhysicalDeviceVulkan14Features::stippledRectangularLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledRectangularLines,
                        "VkPhysicalDeviceLineRasterizationFeatures::stippledRectangularLines"};
            }
        case Feature::stippledSmoothLines:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledSmoothLines, "VkPhysicalDeviceVulkan14Features::stippledSmoothLines"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceLineRasterizationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceLineRasterizationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceLineRasterizationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->stippledSmoothLines, "VkPhysicalDeviceLineRasterizationFeatures::stippledSmoothLines"};
            }
        case Feature::linearColorAttachment: {
            auto vk_struct = const_cast<VkPhysicalDeviceLinearColorAttachmentFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceLinearColorAttachmentFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceLinearColorAttachmentFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->linearColorAttachment, "VkPhysicalDeviceLinearColorAttachmentFeaturesNV::linearColorAttachment"};
        }

        case Feature::maintenance4:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance4, "VkPhysicalDeviceVulkan13Features::maintenance4"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMaintenance4Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance4Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMaintenance4Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance4, "VkPhysicalDeviceMaintenance4Features::maintenance4"};
            }
        case Feature::maintenance5:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance5, "VkPhysicalDeviceVulkan14Features::maintenance5"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMaintenance5Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance5Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMaintenance5Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance5, "VkPhysicalDeviceMaintenance5Features::maintenance5"};
            }
        case Feature::maintenance6:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance6, "VkPhysicalDeviceVulkan14Features::maintenance6"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMaintenance6Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance6Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMaintenance6Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->maintenance6, "VkPhysicalDeviceMaintenance6Features::maintenance6"};
            }
        case Feature::maintenance7: {
            auto vk_struct = const_cast<VkPhysicalDeviceMaintenance7FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance7FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMaintenance7FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->maintenance7, "VkPhysicalDeviceMaintenance7FeaturesKHR::maintenance7"};
        }

        case Feature::maintenance8: {
            auto vk_struct = const_cast<VkPhysicalDeviceMaintenance8FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMaintenance8FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMaintenance8FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->maintenance8, "VkPhysicalDeviceMaintenance8FeaturesKHR::maintenance8"};
        }

        case Feature::memoryMapPlaced: {
            auto vk_struct = const_cast<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMapMemoryPlacedFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->memoryMapPlaced, "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryMapPlaced"};
        }

        case Feature::memoryMapRangePlaced: {
            auto vk_struct = const_cast<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMapMemoryPlacedFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->memoryMapRangePlaced, "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryMapRangePlaced"};
        }

        case Feature::memoryUnmapReserve: {
            auto vk_struct = const_cast<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMapMemoryPlacedFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMapMemoryPlacedFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->memoryUnmapReserve, "VkPhysicalDeviceMapMemoryPlacedFeaturesEXT::memoryUnmapReserve"};
        }

        case Feature::memoryDecompression: {
            auto vk_struct = const_cast<VkPhysicalDeviceMemoryDecompressionFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMemoryDecompressionFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMemoryDecompressionFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->memoryDecompression, "VkPhysicalDeviceMemoryDecompressionFeaturesNV::memoryDecompression"};
        }

        case Feature::memoryPriority: {
            auto vk_struct = const_cast<VkPhysicalDeviceMemoryPriorityFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMemoryPriorityFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMemoryPriorityFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->memoryPriority, "VkPhysicalDeviceMemoryPriorityFeaturesEXT::memoryPriority"};
        }

        case Feature::meshShaderQueries: {
            auto vk_struct = const_cast<VkPhysicalDeviceMeshShaderFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMeshShaderFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->meshShaderQueries, "VkPhysicalDeviceMeshShaderFeaturesEXT::meshShaderQueries"};
        }

        case Feature::multiviewMeshShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceMeshShaderFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMeshShaderFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multiviewMeshShader, "VkPhysicalDeviceMeshShaderFeaturesEXT::multiviewMeshShader"};
        }

        case Feature::primitiveFragmentShadingRateMeshShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceMeshShaderFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMeshShaderFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitiveFragmentShadingRateMeshShader,
                    "VkPhysicalDeviceMeshShaderFeaturesEXT::primitiveFragmentShadingRateMeshShader"};
        }

        case Feature::meshShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceMeshShaderFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMeshShaderFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->meshShader, "VkPhysicalDeviceMeshShaderFeaturesEXT::meshShader"};
        }

        case Feature::taskShader: {
            auto vk_struct = const_cast<VkPhysicalDeviceMeshShaderFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMeshShaderFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMeshShaderFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->taskShader, "VkPhysicalDeviceMeshShaderFeaturesEXT::taskShader"};
        }

        case Feature::multiDraw: {
            auto vk_struct = const_cast<VkPhysicalDeviceMultiDrawFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMultiDrawFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMultiDrawFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multiDraw, "VkPhysicalDeviceMultiDrawFeaturesEXT::multiDraw"};
        }

        case Feature::multisampledRenderToSingleSampled: {
            auto vk_struct = const_cast<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multisampledRenderToSingleSampled,
                    "VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT::multisampledRenderToSingleSampled"};
        }

        case Feature::multiview:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiview, "VkPhysicalDeviceVulkan11Features::multiview"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMultiviewFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMultiviewFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiview, "VkPhysicalDeviceMultiviewFeatures::multiview"};
            }
        case Feature::multiviewGeometryShader:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiviewGeometryShader, "VkPhysicalDeviceVulkan11Features::multiviewGeometryShader"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMultiviewFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMultiviewFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiviewGeometryShader, "VkPhysicalDeviceMultiviewFeatures::multiviewGeometryShader"};
            }
        case Feature::multiviewTessellationShader:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiviewTessellationShader, "VkPhysicalDeviceVulkan11Features::multiviewTessellationShader"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceMultiviewFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceMultiviewFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->multiviewTessellationShader, "VkPhysicalDeviceMultiviewFeatures::multiviewTessellationShader"};
            }
        case Feature::multiviewPerViewRenderAreas: {
            auto vk_struct = const_cast<VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multiviewPerViewRenderAreas,
                    "VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::multiviewPerViewRenderAreas"};
        }

        case Feature::multiviewPerViewViewports: {
            auto vk_struct = const_cast<VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multiviewPerViewViewports,
                    "VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::multiviewPerViewViewports"};
        }

        case Feature::mutableDescriptorType: {
            auto vk_struct = const_cast<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->mutableDescriptorType, "VkPhysicalDeviceMutableDescriptorTypeFeaturesEXT::mutableDescriptorType"};
        }

        case Feature::nestedCommandBuffer: {
            auto vk_struct = const_cast<VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceNestedCommandBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceNestedCommandBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->nestedCommandBuffer, "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBuffer"};
        }

        case Feature::nestedCommandBufferRendering: {
            auto vk_struct = const_cast<VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceNestedCommandBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceNestedCommandBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->nestedCommandBufferRendering,
                    "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBufferRendering"};
        }

        case Feature::nestedCommandBufferSimultaneousUse: {
            auto vk_struct = const_cast<VkPhysicalDeviceNestedCommandBufferFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceNestedCommandBufferFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceNestedCommandBufferFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->nestedCommandBufferSimultaneousUse,
                    "VkPhysicalDeviceNestedCommandBufferFeaturesEXT::nestedCommandBufferSimultaneousUse"};
        }

        case Feature::nonSeamlessCubeMap: {
            auto vk_struct = const_cast<VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->nonSeamlessCubeMap, "VkPhysicalDeviceNonSeamlessCubeMapFeaturesEXT::nonSeamlessCubeMap"};
        }

        case Feature::micromap: {
            auto vk_struct = const_cast<VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceOpacityMicromapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceOpacityMicromapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->micromap, "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromap"};
        }

        case Feature::micromapCaptureReplay: {
            auto vk_struct = const_cast<VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceOpacityMicromapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceOpacityMicromapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->micromapCaptureReplay, "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromapCaptureReplay"};
        }

        case Feature::micromapHostCommands: {
            auto vk_struct = const_cast<VkPhysicalDeviceOpacityMicromapFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceOpacityMicromapFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceOpacityMicromapFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->micromapHostCommands, "VkPhysicalDeviceOpacityMicromapFeaturesEXT::micromapHostCommands"};
        }

        case Feature::opticalFlow: {
            auto vk_struct = const_cast<VkPhysicalDeviceOpticalFlowFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceOpticalFlowFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceOpticalFlowFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->opticalFlow, "VkPhysicalDeviceOpticalFlowFeaturesNV::opticalFlow"};
        }

        case Feature::pageableDeviceLocalMemory: {
            auto vk_struct = const_cast<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pageableDeviceLocalMemory,
                    "VkPhysicalDevicePageableDeviceLocalMemoryFeaturesEXT::pageableDeviceLocalMemory"};
        }

        case Feature::partitionedAccelerationStructure: {
            auto vk_struct = const_cast<VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->partitionedAccelerationStructure,
                    "VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::partitionedAccelerationStructure"};
        }

        case Feature::dynamicPipelineLayout: {
            auto vk_struct = const_cast<VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePerStageDescriptorSetFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePerStageDescriptorSetFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->dynamicPipelineLayout, "VkPhysicalDevicePerStageDescriptorSetFeaturesNV::dynamicPipelineLayout"};
        }

        case Feature::perStageDescriptorSet: {
            auto vk_struct = const_cast<VkPhysicalDevicePerStageDescriptorSetFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePerStageDescriptorSetFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePerStageDescriptorSetFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->perStageDescriptorSet, "VkPhysicalDevicePerStageDescriptorSetFeaturesNV::perStageDescriptorSet"};
        }

        case Feature::performanceCounterMultipleQueryPools: {
            auto vk_struct = const_cast<VkPhysicalDevicePerformanceQueryFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePerformanceQueryFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePerformanceQueryFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->performanceCounterMultipleQueryPools,
                    "VkPhysicalDevicePerformanceQueryFeaturesKHR::performanceCounterMultipleQueryPools"};
        }

        case Feature::performanceCounterQueryPools: {
            auto vk_struct = const_cast<VkPhysicalDevicePerformanceQueryFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePerformanceQueryFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePerformanceQueryFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->performanceCounterQueryPools,
                    "VkPhysicalDevicePerformanceQueryFeaturesKHR::performanceCounterQueryPools"};
        }

        case Feature::pipelineBinaries: {
            auto vk_struct = const_cast<VkPhysicalDevicePipelineBinaryFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePipelineBinaryFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePipelineBinaryFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelineBinaries, "VkPhysicalDevicePipelineBinaryFeaturesKHR::pipelineBinaries"};
        }

        case Feature::pipelineCreationCacheControl:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineCreationCacheControl, "VkPhysicalDeviceVulkan13Features::pipelineCreationCacheControl"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevicePipelineCreationCacheControlFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevicePipelineCreationCacheControlFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevicePipelineCreationCacheControlFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineCreationCacheControl,
                        "VkPhysicalDevicePipelineCreationCacheControlFeatures::pipelineCreationCacheControl"};
            }
        case Feature::pipelineExecutableInfo: {
            auto vk_struct = const_cast<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelineExecutableInfo,
                    "VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR::pipelineExecutableInfo"};
        }

        case Feature::pipelineLibraryGroupHandles: {
            auto vk_struct = const_cast<VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelineLibraryGroupHandles,
                    "VkPhysicalDevicePipelineLibraryGroupHandlesFeaturesEXT::pipelineLibraryGroupHandles"};
        }

        case Feature::pipelineOpacityMicromap: {
            auto vk_struct = const_cast<VkPhysicalDevicePipelineOpacityMicromapFeaturesARM *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePipelineOpacityMicromapFeaturesARM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePipelineOpacityMicromapFeaturesARM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelineOpacityMicromap,
                    "VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::pipelineOpacityMicromap"};
        }

        case Feature::pipelinePropertiesIdentifier: {
            auto vk_struct = const_cast<VkPhysicalDevicePipelinePropertiesFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePipelinePropertiesFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePipelinePropertiesFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pipelinePropertiesIdentifier,
                    "VkPhysicalDevicePipelinePropertiesFeaturesEXT::pipelinePropertiesIdentifier"};
        }

        case Feature::pipelineProtectedAccess:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineProtectedAccess, "VkPhysicalDeviceVulkan14Features::pipelineProtectedAccess"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevicePipelineProtectedAccessFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevicePipelineProtectedAccessFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevicePipelineProtectedAccessFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineProtectedAccess,
                        "VkPhysicalDevicePipelineProtectedAccessFeatures::pipelineProtectedAccess"};
            }
        case Feature::pipelineRobustness:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineRobustness, "VkPhysicalDeviceVulkan14Features::pipelineRobustness"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevicePipelineRobustnessFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevicePipelineRobustnessFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevicePipelineRobustnessFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->pipelineRobustness, "VkPhysicalDevicePipelineRobustnessFeatures::pipelineRobustness"};
            }
#ifdef VK_ENABLE_BETA_EXTENSIONS

            case Feature::constantAlphaColorBlendFactors : {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->constantAlphaColorBlendFactors,
                    "VkPhysicalDevicePortabilitySubsetFeaturesKHR::constantAlphaColorBlendFactors"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::events: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->events, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::events"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::imageView2DOn3DImage: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageView2DOn3DImage, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageView2DOn3DImage"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::imageViewFormatReinterpretation: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageViewFormatReinterpretation,
                    "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageViewFormatReinterpretation"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::imageViewFormatSwizzle: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageViewFormatSwizzle, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::imageViewFormatSwizzle"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::multisampleArrayImage: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->multisampleArrayImage, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::multisampleArrayImage"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::mutableComparisonSamplers: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->mutableComparisonSamplers,
                    "VkPhysicalDevicePortabilitySubsetFeaturesKHR::mutableComparisonSamplers"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::pointPolygons: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pointPolygons, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::pointPolygons"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::samplerMipLodBias: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->samplerMipLodBias, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::samplerMipLodBias"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::separateStencilMaskRef: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->separateStencilMaskRef, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::separateStencilMaskRef"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::shaderSampleRateInterpolationFunctions: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSampleRateInterpolationFunctions,
                    "VkPhysicalDevicePortabilitySubsetFeaturesKHR::shaderSampleRateInterpolationFunctions"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::tessellationIsolines: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->tessellationIsolines, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::tessellationIsolines"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::tessellationPointMode: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->tessellationPointMode, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::tessellationPointMode"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::triangleFans: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->triangleFans, "VkPhysicalDevicePortabilitySubsetFeaturesKHR::triangleFans"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::vertexAttributeAccessBeyondStride: {
            auto vk_struct = const_cast<VkPhysicalDevicePortabilitySubsetFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePortabilitySubsetFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePortabilitySubsetFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->vertexAttributeAccessBeyondStride,
                    "VkPhysicalDevicePortabilitySubsetFeaturesKHR::vertexAttributeAccessBeyondStride"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS

        case Feature::presentBarrier: {
            auto vk_struct = const_cast<VkPhysicalDevicePresentBarrierFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePresentBarrierFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePresentBarrierFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->presentBarrier, "VkPhysicalDevicePresentBarrierFeaturesNV::presentBarrier"};
        }

        case Feature::presentId: {
            auto vk_struct = const_cast<VkPhysicalDevicePresentIdFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePresentIdFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePresentIdFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->presentId, "VkPhysicalDevicePresentIdFeaturesKHR::presentId"};
        }

        case Feature::presentModeFifoLatestReady: {
            auto vk_struct = const_cast<VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->presentModeFifoLatestReady,
                    "VkPhysicalDevicePresentModeFifoLatestReadyFeaturesEXT::presentModeFifoLatestReady"};
        }

        case Feature::presentWait: {
            auto vk_struct = const_cast<VkPhysicalDevicePresentWaitFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePresentWaitFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePresentWaitFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->presentWait, "VkPhysicalDevicePresentWaitFeaturesKHR::presentWait"};
        }

        case Feature::primitiveTopologyListRestart: {
            auto vk_struct = const_cast<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitiveTopologyListRestart,
                    "VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::primitiveTopologyListRestart"};
        }

        case Feature::primitiveTopologyPatchListRestart: {
            auto vk_struct = const_cast<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitiveTopologyPatchListRestart,
                    "VkPhysicalDevicePrimitiveTopologyListRestartFeaturesEXT::primitiveTopologyPatchListRestart"};
        }

        case Feature::primitivesGeneratedQuery: {
            auto vk_struct = const_cast<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitivesGeneratedQuery,
                    "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQuery"};
        }

        case Feature::primitivesGeneratedQueryWithNonZeroStreams: {
            auto vk_struct = const_cast<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitivesGeneratedQueryWithNonZeroStreams,
                    "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQueryWithNonZeroStreams"};
        }

        case Feature::primitivesGeneratedQueryWithRasterizerDiscard: {
            auto vk_struct = const_cast<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->primitivesGeneratedQueryWithRasterizerDiscard,
                    "VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT::primitivesGeneratedQueryWithRasterizerDiscard"};
        }

        case Feature::privateData:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->privateData, "VkPhysicalDeviceVulkan13Features::privateData"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDevicePrivateDataFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevicePrivateDataFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDevicePrivateDataFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->privateData, "VkPhysicalDevicePrivateDataFeatures::privateData"};
            }
        case Feature::protectedMemory:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->protectedMemory, "VkPhysicalDeviceVulkan11Features::protectedMemory"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceProtectedMemoryFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceProtectedMemoryFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceProtectedMemoryFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->protectedMemory, "VkPhysicalDeviceProtectedMemoryFeatures::protectedMemory"};
            }
        case Feature::provokingVertexLast: {
            auto vk_struct = const_cast<VkPhysicalDeviceProvokingVertexFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceProvokingVertexFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceProvokingVertexFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->provokingVertexLast, "VkPhysicalDeviceProvokingVertexFeaturesEXT::provokingVertexLast"};
        }

        case Feature::transformFeedbackPreservesProvokingVertex: {
            auto vk_struct = const_cast<VkPhysicalDeviceProvokingVertexFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceProvokingVertexFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceProvokingVertexFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->transformFeedbackPreservesProvokingVertex,
                    "VkPhysicalDeviceProvokingVertexFeaturesEXT::transformFeedbackPreservesProvokingVertex"};
        }

        case Feature::formatRgba10x6WithoutYCbCrSampler: {
            auto vk_struct = const_cast<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->formatRgba10x6WithoutYCbCrSampler,
                    "VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT::formatRgba10x6WithoutYCbCrSampler"};
        }

        case Feature::rasterizationOrderColorAttachmentAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rasterizationOrderColorAttachmentAccess,
                    "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::rasterizationOrderColorAttachmentAccess"};
        }

        case Feature::rasterizationOrderDepthAttachmentAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rasterizationOrderDepthAttachmentAccess,
                    "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::rasterizationOrderDepthAttachmentAccess"};
        }

        case Feature::rasterizationOrderStencilAttachmentAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rasterizationOrderStencilAttachmentAccess,
                    "VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT::rasterizationOrderStencilAttachmentAccess"};
        }

        case Feature::shaderRawAccessChains: {
            auto vk_struct = const_cast<VkPhysicalDeviceRawAccessChainsFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRawAccessChainsFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRawAccessChainsFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderRawAccessChains, "VkPhysicalDeviceRawAccessChainsFeaturesNV::shaderRawAccessChains"};
        }

        case Feature::rayQuery: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayQueryFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayQueryFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayQueryFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayQuery, "VkPhysicalDeviceRayQueryFeaturesKHR::rayQuery"};
        }

        case Feature::rayTracingInvocationReorder: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingInvocationReorder,
                    "VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::rayTracingInvocationReorder"};
        }

        case Feature::linearSweptSpheres: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->linearSweptSpheres, "VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::linearSweptSpheres"};
        }

        case Feature::spheres: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->spheres, "VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::spheres"};
        }

        case Feature::rayTracingMaintenance1: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingMaintenance1,
                    "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingMaintenance1"};
        }

        case Feature::rayTracingPipelineTraceRaysIndirect2: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPipelineTraceRaysIndirect2,
                    "VkPhysicalDeviceRayTracingMaintenance1FeaturesKHR::rayTracingPipelineTraceRaysIndirect2"};
        }

        case Feature::rayTracingMotionBlur: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingMotionBlurFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingMotionBlur, "VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlur"};
        }

        case Feature::rayTracingMotionBlurPipelineTraceRaysIndirect: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingMotionBlurFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingMotionBlurFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingMotionBlurPipelineTraceRaysIndirect,
                    "VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::rayTracingMotionBlurPipelineTraceRaysIndirect"};
        }

        case Feature::rayTracingPipeline: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPipelineFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPipeline, "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipeline"};
        }

        case Feature::rayTracingPipelineShaderGroupHandleCaptureReplay: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPipelineFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPipelineShaderGroupHandleCaptureReplay,
                    "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineShaderGroupHandleCaptureReplay"};
        }

        case Feature::rayTracingPipelineShaderGroupHandleCaptureReplayMixed: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPipelineFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPipelineShaderGroupHandleCaptureReplayMixed,
                    "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineShaderGroupHandleCaptureReplayMixed"};
        }

        case Feature::rayTracingPipelineTraceRaysIndirect: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPipelineFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPipelineTraceRaysIndirect,
                    "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTracingPipelineTraceRaysIndirect"};
        }

        case Feature::rayTraversalPrimitiveCulling: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPipelineFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTraversalPrimitiveCulling,
                    "VkPhysicalDeviceRayTracingPipelineFeaturesKHR::rayTraversalPrimitiveCulling"};
        }

        case Feature::rayTracingPositionFetch: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingPositionFetch,
                    "VkPhysicalDeviceRayTracingPositionFetchFeaturesKHR::rayTracingPositionFetch"};
        }

        case Feature::rayTracingValidation: {
            auto vk_struct = const_cast<VkPhysicalDeviceRayTracingValidationFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRayTracingValidationFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRayTracingValidationFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->rayTracingValidation, "VkPhysicalDeviceRayTracingValidationFeaturesNV::rayTracingValidation"};
        }

        case Feature::relaxedLineRasterization: {
            auto vk_struct = const_cast<VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->relaxedLineRasterization,
                    "VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::relaxedLineRasterization"};
        }

        case Feature::renderPassStriped: {
            auto vk_struct = const_cast<VkPhysicalDeviceRenderPassStripedFeaturesARM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRenderPassStripedFeaturesARM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRenderPassStripedFeaturesARM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->renderPassStriped, "VkPhysicalDeviceRenderPassStripedFeaturesARM::renderPassStriped"};
        }

        case Feature::representativeFragmentTest: {
            auto vk_struct = const_cast<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->representativeFragmentTest,
                    "VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::representativeFragmentTest"};
        }

        case Feature::nullDescriptor: {
            auto vk_struct = const_cast<VkPhysicalDeviceRobustness2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRobustness2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->nullDescriptor, "VkPhysicalDeviceRobustness2FeaturesEXT::nullDescriptor"};
        }

        case Feature::robustBufferAccess2: {
            auto vk_struct = const_cast<VkPhysicalDeviceRobustness2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRobustness2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->robustBufferAccess2, "VkPhysicalDeviceRobustness2FeaturesEXT::robustBufferAccess2"};
        }

        case Feature::robustImageAccess2: {
            auto vk_struct = const_cast<VkPhysicalDeviceRobustness2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceRobustness2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->robustImageAccess2, "VkPhysicalDeviceRobustness2FeaturesEXT::robustImageAccess2"};
        }

        case Feature::samplerYcbcrConversion:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->samplerYcbcrConversion, "VkPhysicalDeviceVulkan11Features::samplerYcbcrConversion"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceSamplerYcbcrConversionFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceSamplerYcbcrConversionFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->samplerYcbcrConversion,
                        "VkPhysicalDeviceSamplerYcbcrConversionFeatures::samplerYcbcrConversion"};
            }
        case Feature::scalarBlockLayout:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->scalarBlockLayout, "VkPhysicalDeviceVulkan12Features::scalarBlockLayout"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceScalarBlockLayoutFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceScalarBlockLayoutFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceScalarBlockLayoutFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->scalarBlockLayout, "VkPhysicalDeviceScalarBlockLayoutFeatures::scalarBlockLayout"};
            }
        case Feature::schedulingControls: {
            auto vk_struct = const_cast<VkPhysicalDeviceSchedulingControlsFeaturesARM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceSchedulingControlsFeaturesARM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceSchedulingControlsFeaturesARM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->schedulingControls, "VkPhysicalDeviceSchedulingControlsFeaturesARM::schedulingControls"};
        }

        case Feature::separateDepthStencilLayouts:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->separateDepthStencilLayouts, "VkPhysicalDeviceVulkan12Features::separateDepthStencilLayouts"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->separateDepthStencilLayouts,
                        "VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures::separateDepthStencilLayouts"};
            }
        case Feature::shaderFloat16VectorAtomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderFloat16VectorAtomics,
                    "VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::shaderFloat16VectorAtomics"};
        }

        case Feature::shaderBufferFloat16AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat16AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicAdd"};
        }

        case Feature::shaderBufferFloat16AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat16AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16AtomicMinMax"};
        }

        case Feature::shaderBufferFloat16Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat16Atomics,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat16Atomics"};
        }

        case Feature::shaderBufferFloat32AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat32AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat32AtomicMinMax"};
        }

        case Feature::shaderBufferFloat64AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat64AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderBufferFloat64AtomicMinMax"};
        }

        case Feature::shaderImageFloat32AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderImageFloat32AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderImageFloat32AtomicMinMax"};
        }

        case Feature::shaderSharedFloat16AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat16AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicAdd"};
        }

        case Feature::shaderSharedFloat16AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat16AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16AtomicMinMax"};
        }

        case Feature::shaderSharedFloat16Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat16Atomics,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat16Atomics"};
        }

        case Feature::shaderSharedFloat32AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat32AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat32AtomicMinMax"};
        }

        case Feature::shaderSharedFloat64AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat64AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::shaderSharedFloat64AtomicMinMax"};
        }

        case Feature::sparseImageFloat32AtomicMinMax: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->sparseImageFloat32AtomicMinMax,
                    "VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT::sparseImageFloat32AtomicMinMax"};
        }

        case Feature::shaderBufferFloat32AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat32AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32AtomicAdd"};
        }

        case Feature::shaderBufferFloat32Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat32Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat32Atomics"};
        }

        case Feature::shaderBufferFloat64AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat64AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64AtomicAdd"};
        }

        case Feature::shaderBufferFloat64Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderBufferFloat64Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderBufferFloat64Atomics"};
        }

        case Feature::shaderImageFloat32AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderImageFloat32AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32AtomicAdd"};
        }

        case Feature::shaderImageFloat32Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderImageFloat32Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderImageFloat32Atomics"};
        }

        case Feature::shaderSharedFloat32AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat32AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32AtomicAdd"};
        }

        case Feature::shaderSharedFloat32Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat32Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat32Atomics"};
        }

        case Feature::shaderSharedFloat64AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat64AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64AtomicAdd"};
        }

        case Feature::shaderSharedFloat64Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSharedFloat64Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::shaderSharedFloat64Atomics"};
        }

        case Feature::sparseImageFloat32AtomicAdd: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->sparseImageFloat32AtomicAdd,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::sparseImageFloat32AtomicAdd"};
        }

        case Feature::sparseImageFloat32Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicFloatFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderAtomicFloatFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->sparseImageFloat32Atomics,
                    "VkPhysicalDeviceShaderAtomicFloatFeaturesEXT::sparseImageFloat32Atomics"};
        }

        case Feature::shaderBufferInt64Atomics:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderBufferInt64Atomics, "VkPhysicalDeviceVulkan12Features::shaderBufferInt64Atomics"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicInt64Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicInt64Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderAtomicInt64Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderBufferInt64Atomics,
                        "VkPhysicalDeviceShaderAtomicInt64Features::shaderBufferInt64Atomics"};
            }
        case Feature::shaderSharedInt64Atomics:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSharedInt64Atomics, "VkPhysicalDeviceVulkan12Features::shaderSharedInt64Atomics"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderAtomicInt64Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderAtomicInt64Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderAtomicInt64Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSharedInt64Atomics,
                        "VkPhysicalDeviceShaderAtomicInt64Features::shaderSharedInt64Atomics"};
            }
        case Feature::shaderDeviceClock: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderClockFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderClockFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderClockFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderDeviceClock, "VkPhysicalDeviceShaderClockFeaturesKHR::shaderDeviceClock"};
        }

        case Feature::shaderSubgroupClock: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderClockFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderClockFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderClockFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSubgroupClock, "VkPhysicalDeviceShaderClockFeaturesKHR::shaderSubgroupClock"};
        }

        case Feature::shaderCoreBuiltins: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderCoreBuiltins, "VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::shaderCoreBuiltins"};
        }

        case Feature::shaderDemoteToHelperInvocation:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderDemoteToHelperInvocation,
                        "VkPhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderDemoteToHelperInvocation,
                        "VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures::shaderDemoteToHelperInvocation"};
            }
        case Feature::shaderDrawParameters:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderDrawParameters, "VkPhysicalDeviceVulkan11Features::shaderDrawParameters"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderDrawParametersFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderDrawParametersFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderDrawParametersFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderDrawParameters, "VkPhysicalDeviceShaderDrawParametersFeatures::shaderDrawParameters"};
            }
        case Feature::shaderEarlyAndLateFragmentTests: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderEarlyAndLateFragmentTests,
                    "VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::shaderEarlyAndLateFragmentTests"};
        }
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::shaderEnqueue: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderEnqueueFeaturesAMDX>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderEnqueueFeaturesAMDX;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderEnqueue, "VkPhysicalDeviceShaderEnqueueFeaturesAMDX::shaderEnqueue"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_ENABLE_BETA_EXTENSIONS

        case Feature::shaderMeshEnqueue: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderEnqueueFeaturesAMDX *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderEnqueueFeaturesAMDX>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderEnqueueFeaturesAMDX;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderMeshEnqueue, "VkPhysicalDeviceShaderEnqueueFeaturesAMDX::shaderMeshEnqueue"};
        }
#endif  // VK_ENABLE_BETA_EXTENSIONS

        case Feature::shaderExpectAssume:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderExpectAssume, "VkPhysicalDeviceVulkan14Features::shaderExpectAssume"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderExpectAssumeFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderExpectAssumeFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderExpectAssumeFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderExpectAssume, "VkPhysicalDeviceShaderExpectAssumeFeatures::shaderExpectAssume"};
            }
        case Feature::shaderFloat16:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderFloat16, "VkPhysicalDeviceVulkan12Features::shaderFloat16"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderFloat16Int8Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderFloat16Int8Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderFloat16Int8Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderFloat16, "VkPhysicalDeviceShaderFloat16Int8Features::shaderFloat16"};
            }
        case Feature::shaderInt8:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInt8, "VkPhysicalDeviceVulkan12Features::shaderInt8"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderFloat16Int8Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderFloat16Int8Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderFloat16Int8Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderInt8, "VkPhysicalDeviceShaderFloat16Int8Features::shaderInt8"};
            }
        case Feature::shaderFloatControls2:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderFloatControls2, "VkPhysicalDeviceVulkan14Features::shaderFloatControls2"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderFloatControls2Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderFloatControls2Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderFloatControls2Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderFloatControls2, "VkPhysicalDeviceShaderFloatControls2Features::shaderFloatControls2"};
            }
        case Feature::shaderImageInt64Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderImageInt64Atomics,
                    "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::shaderImageInt64Atomics"};
        }

        case Feature::sparseImageInt64Atomics: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->sparseImageInt64Atomics,
                    "VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT::sparseImageInt64Atomics"};
        }

        case Feature::imageFootprint: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderImageFootprintFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderImageFootprintFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderImageFootprintFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->imageFootprint, "VkPhysicalDeviceShaderImageFootprintFeaturesNV::imageFootprint"};
        }

        case Feature::shaderIntegerDotProduct:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderIntegerDotProduct, "VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderIntegerDotProductFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderIntegerDotProductFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderIntegerDotProductFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderIntegerDotProduct,
                        "VkPhysicalDeviceShaderIntegerDotProductFeatures::shaderIntegerDotProduct"};
            }
        case Feature::shaderIntegerFunctions2: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderIntegerFunctions2,
                    "VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::shaderIntegerFunctions2"};
        }

        case Feature::shaderMaximalReconvergence: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderMaximalReconvergence,
                    "VkPhysicalDeviceShaderMaximalReconvergenceFeaturesKHR::shaderMaximalReconvergence"};
        }

        case Feature::shaderModuleIdentifier: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderModuleIdentifier,
                    "VkPhysicalDeviceShaderModuleIdentifierFeaturesEXT::shaderModuleIdentifier"};
        }

        case Feature::shaderObject: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderObjectFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderObjectFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderObjectFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderObject, "VkPhysicalDeviceShaderObjectFeaturesEXT::shaderObject"};
        }

        case Feature::shaderQuadControl: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderQuadControlFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderQuadControlFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderQuadControlFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderQuadControl, "VkPhysicalDeviceShaderQuadControlFeaturesKHR::shaderQuadControl"};
        }

        case Feature::shaderRelaxedExtendedInstruction: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderRelaxedExtendedInstruction,
                    "VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR::shaderRelaxedExtendedInstruction"};
        }

        case Feature::shaderReplicatedComposites: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderReplicatedComposites,
                    "VkPhysicalDeviceShaderReplicatedCompositesFeaturesEXT::shaderReplicatedComposites"};
        }

        case Feature::shaderSMBuiltins: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderSMBuiltinsFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderSMBuiltinsFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSMBuiltins, "VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::shaderSMBuiltins"};
        }

        case Feature::shaderSubgroupExtendedTypes:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupExtendedTypes, "VkPhysicalDeviceVulkan12Features::shaderSubgroupExtendedTypes"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupExtendedTypes,
                        "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures::shaderSubgroupExtendedTypes"};
            }
        case Feature::shaderSubgroupRotate:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupRotate, "VkPhysicalDeviceVulkan14Features::shaderSubgroupRotate"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderSubgroupRotateFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderSubgroupRotateFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderSubgroupRotateFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupRotate, "VkPhysicalDeviceShaderSubgroupRotateFeatures::shaderSubgroupRotate"};
            }
        case Feature::shaderSubgroupRotateClustered:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupRotateClustered,
                        "VkPhysicalDeviceVulkan14Features::shaderSubgroupRotateClustered"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderSubgroupRotateFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderSubgroupRotateFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderSubgroupRotateFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderSubgroupRotateClustered,
                        "VkPhysicalDeviceShaderSubgroupRotateFeatures::shaderSubgroupRotateClustered"};
            }
        case Feature::shaderSubgroupUniformControlFlow: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderSubgroupUniformControlFlow,
                    "VkPhysicalDeviceShaderSubgroupUniformControlFlowFeaturesKHR::shaderSubgroupUniformControlFlow"};
        }

        case Feature::shaderTerminateInvocation:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderTerminateInvocation, "VkPhysicalDeviceVulkan13Features::shaderTerminateInvocation"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceShaderTerminateInvocationFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceShaderTerminateInvocationFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceShaderTerminateInvocationFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderTerminateInvocation,
                        "VkPhysicalDeviceShaderTerminateInvocationFeatures::shaderTerminateInvocation"};
            }
        case Feature::shaderTileImageColorReadAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderTileImageFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderTileImageFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderTileImageFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderTileImageColorReadAccess,
                    "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageColorReadAccess"};
        }

        case Feature::shaderTileImageDepthReadAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderTileImageFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderTileImageFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderTileImageFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderTileImageDepthReadAccess,
                    "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageDepthReadAccess"};
        }

        case Feature::shaderTileImageStencilReadAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceShaderTileImageFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShaderTileImageFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShaderTileImageFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderTileImageStencilReadAccess,
                    "VkPhysicalDeviceShaderTileImageFeaturesEXT::shaderTileImageStencilReadAccess"};
        }

        case Feature::shadingRateCoarseSampleOrder: {
            auto vk_struct = const_cast<VkPhysicalDeviceShadingRateImageFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShadingRateImageFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shadingRateCoarseSampleOrder,
                    "VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateCoarseSampleOrder"};
        }

        case Feature::shadingRateImage: {
            auto vk_struct = const_cast<VkPhysicalDeviceShadingRateImageFeaturesNV *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceShadingRateImageFeaturesNV>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceShadingRateImageFeaturesNV;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shadingRateImage, "VkPhysicalDeviceShadingRateImageFeaturesNV::shadingRateImage"};
        }

        case Feature::computeFullSubgroups:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->computeFullSubgroups, "VkPhysicalDeviceVulkan13Features::computeFullSubgroups"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceSubgroupSizeControlFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceSubgroupSizeControlFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceSubgroupSizeControlFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->computeFullSubgroups, "VkPhysicalDeviceSubgroupSizeControlFeatures::computeFullSubgroups"};
            }
        case Feature::subgroupSizeControl:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->subgroupSizeControl, "VkPhysicalDeviceVulkan13Features::subgroupSizeControl"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceSubgroupSizeControlFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceSubgroupSizeControlFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceSubgroupSizeControlFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->subgroupSizeControl, "VkPhysicalDeviceSubgroupSizeControlFeatures::subgroupSizeControl"};
            }
        case Feature::subpassMergeFeedback: {
            auto vk_struct = const_cast<VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->subpassMergeFeedback, "VkPhysicalDeviceSubpassMergeFeedbackFeaturesEXT::subpassMergeFeedback"};
        }

        case Feature::subpassShading: {
            auto vk_struct = const_cast<VkPhysicalDeviceSubpassShadingFeaturesHUAWEI *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceSubpassShadingFeaturesHUAWEI>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceSubpassShadingFeaturesHUAWEI;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->subpassShading, "VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::subpassShading"};
        }

        case Feature::swapchainMaintenance1: {
            auto vk_struct = const_cast<VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->swapchainMaintenance1, "VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT::swapchainMaintenance1"};
        }

        case Feature::synchronization2:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->synchronization2, "VkPhysicalDeviceVulkan13Features::synchronization2"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceSynchronization2Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceSynchronization2Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceSynchronization2Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->synchronization2, "VkPhysicalDeviceSynchronization2Features::synchronization2"};
            }
        case Feature::texelBufferAlignment: {
            auto vk_struct = const_cast<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->texelBufferAlignment, "VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT::texelBufferAlignment"};
        }

        case Feature::textureCompressionASTC_HDR:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->textureCompressionASTC_HDR, "VkPhysicalDeviceVulkan13Features::textureCompressionASTC_HDR"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceTextureCompressionASTCHDRFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceTextureCompressionASTCHDRFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->textureCompressionASTC_HDR,
                        "VkPhysicalDeviceTextureCompressionASTCHDRFeatures::textureCompressionASTC_HDR"};
            }
        case Feature::tileProperties: {
            auto vk_struct = const_cast<VkPhysicalDeviceTilePropertiesFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTilePropertiesFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceTilePropertiesFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->tileProperties, "VkPhysicalDeviceTilePropertiesFeaturesQCOM::tileProperties"};
        }

        case Feature::timelineSemaphore:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->timelineSemaphore, "VkPhysicalDeviceVulkan12Features::timelineSemaphore"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceTimelineSemaphoreFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->timelineSemaphore, "VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore"};
            }
        case Feature::geometryStreams: {
            auto vk_struct = const_cast<VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceTransformFeedbackFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->geometryStreams, "VkPhysicalDeviceTransformFeedbackFeaturesEXT::geometryStreams"};
        }

        case Feature::transformFeedback: {
            auto vk_struct = const_cast<VkPhysicalDeviceTransformFeedbackFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceTransformFeedbackFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceTransformFeedbackFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->transformFeedback, "VkPhysicalDeviceTransformFeedbackFeaturesEXT::transformFeedback"};
        }

        case Feature::uniformBufferStandardLayout:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformBufferStandardLayout, "VkPhysicalDeviceVulkan12Features::uniformBufferStandardLayout"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceUniformBufferStandardLayoutFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceUniformBufferStandardLayoutFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->uniformBufferStandardLayout,
                        "VkPhysicalDeviceUniformBufferStandardLayoutFeatures::uniformBufferStandardLayout"};
            }
        case Feature::variablePointers:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->variablePointers, "VkPhysicalDeviceVulkan11Features::variablePointers"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVariablePointersFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVariablePointersFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVariablePointersFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->variablePointers, "VkPhysicalDeviceVariablePointersFeatures::variablePointers"};
            }
        case Feature::variablePointersStorageBuffer:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan11Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan11Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan11Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->variablePointersStorageBuffer,
                        "VkPhysicalDeviceVulkan11Features::variablePointersStorageBuffer"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVariablePointersFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVariablePointersFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVariablePointersFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->variablePointersStorageBuffer,
                        "VkPhysicalDeviceVariablePointersFeatures::variablePointersStorageBuffer"};
            }
        case Feature::vertexAttributeInstanceRateDivisor:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vertexAttributeInstanceRateDivisor,
                        "VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateDivisor"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVertexAttributeDivisorFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVertexAttributeDivisorFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVertexAttributeDivisorFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vertexAttributeInstanceRateDivisor,
                        "VkPhysicalDeviceVertexAttributeDivisorFeatures::vertexAttributeInstanceRateDivisor"};
            }
        case Feature::vertexAttributeInstanceRateZeroDivisor:
            if (api_version >= VK_API_VERSION_1_4) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan14Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vertexAttributeInstanceRateZeroDivisor,
                        "VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateZeroDivisor"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVertexAttributeDivisorFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVertexAttributeDivisorFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVertexAttributeDivisorFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vertexAttributeInstanceRateZeroDivisor,
                        "VkPhysicalDeviceVertexAttributeDivisorFeatures::vertexAttributeInstanceRateZeroDivisor"};
            }
        case Feature::vertexAttributeRobustness: {
            auto vk_struct = const_cast<VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->vertexAttributeRobustness,
                    "VkPhysicalDeviceVertexAttributeRobustnessFeaturesEXT::vertexAttributeRobustness"};
        }

        case Feature::vertexInputDynamicState: {
            auto vk_struct = const_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->vertexInputDynamicState,
                    "VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT::vertexInputDynamicState"};
        }

        case Feature::videoEncodeAV1: {
            auto vk_struct = const_cast<VkPhysicalDeviceVideoEncodeAV1FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVideoEncodeAV1FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVideoEncodeAV1FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->videoEncodeAV1, "VkPhysicalDeviceVideoEncodeAV1FeaturesKHR::videoEncodeAV1"};
        }

        case Feature::videoEncodeQuantizationMap: {
            auto vk_struct = const_cast<VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->videoEncodeQuantizationMap,
                    "VkPhysicalDeviceVideoEncodeQuantizationMapFeaturesKHR::videoEncodeQuantizationMap"};
        }

        case Feature::videoMaintenance1: {
            auto vk_struct = const_cast<VkPhysicalDeviceVideoMaintenance1FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVideoMaintenance1FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVideoMaintenance1FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->videoMaintenance1, "VkPhysicalDeviceVideoMaintenance1FeaturesKHR::videoMaintenance1"};
        }

        case Feature::videoMaintenance2: {
            auto vk_struct = const_cast<VkPhysicalDeviceVideoMaintenance2FeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVideoMaintenance2FeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVideoMaintenance2FeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->videoMaintenance2, "VkPhysicalDeviceVideoMaintenance2FeaturesKHR::videoMaintenance2"};
        }

        case Feature::descriptorIndexing: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->descriptorIndexing, "VkPhysicalDeviceVulkan12Features::descriptorIndexing"};
        }

        case Feature::drawIndirectCount: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->drawIndirectCount, "VkPhysicalDeviceVulkan12Features::drawIndirectCount"};
        }

        case Feature::samplerFilterMinmax: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->samplerFilterMinmax, "VkPhysicalDeviceVulkan12Features::samplerFilterMinmax"};
        }

        case Feature::samplerMirrorClampToEdge: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->samplerMirrorClampToEdge, "VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge"};
        }

        case Feature::shaderOutputLayer: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderOutputLayer, "VkPhysicalDeviceVulkan12Features::shaderOutputLayer"};
        }

        case Feature::shaderOutputViewportIndex: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->shaderOutputViewportIndex, "VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex"};
        }

        case Feature::subgroupBroadcastDynamicId: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan12Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->subgroupBroadcastDynamicId, "VkPhysicalDeviceVulkan12Features::subgroupBroadcastDynamicId"};
        }

        case Feature::vulkanMemoryModel:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModel, "VkPhysicalDeviceVulkan12Features::vulkanMemoryModel"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkanMemoryModelFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModel, "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModel"};
            }
        case Feature::vulkanMemoryModelAvailabilityVisibilityChains:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModelAvailabilityVisibilityChains,
                        "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelAvailabilityVisibilityChains"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkanMemoryModelFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModelAvailabilityVisibilityChains,
                        "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelAvailabilityVisibilityChains"};
            }
        case Feature::vulkanMemoryModelDeviceScope:
            if (api_version >= VK_API_VERSION_1_2) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan12Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModelDeviceScope, "VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkanMemoryModelFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->vulkanMemoryModelDeviceScope,
                        "VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelDeviceScope"};
            }
        case Feature::shaderZeroInitializeWorkgroupMemory:
            if (api_version >= VK_API_VERSION_1_3) {
                auto vk_struct = const_cast<VkPhysicalDeviceVulkan13Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan13Features>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceVulkan13Features;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderZeroInitializeWorkgroupMemory,
                        "VkPhysicalDeviceVulkan13Features::shaderZeroInitializeWorkgroupMemory"};
            } else {
                auto vk_struct = const_cast<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures>(*inout_pnext_chain));
                if (!vk_struct) {
                    vk_struct = new VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures;
                    *vk_struct = vku::InitStructHelper();
                    if (*inout_pnext_chain) {
                        vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                    } else {
                        *inout_pnext_chain = vk_struct;
                    }
                }
                return {&vk_struct->shaderZeroInitializeWorkgroupMemory,
                        "VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures::shaderZeroInitializeWorkgroupMemory"};
            }
        case Feature::pushDescriptor: {
            auto vk_struct = const_cast<VkPhysicalDeviceVulkan14Features *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceVulkan14Features>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceVulkan14Features;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->pushDescriptor, "VkPhysicalDeviceVulkan14Features::pushDescriptor"};
        }

        case Feature::workgroupMemoryExplicitLayout: {
            auto vk_struct = const_cast<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->workgroupMemoryExplicitLayout,
                    "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout"};
        }

        case Feature::workgroupMemoryExplicitLayout16BitAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->workgroupMemoryExplicitLayout16BitAccess,
                    "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout16BitAccess"};
        }

        case Feature::workgroupMemoryExplicitLayout8BitAccess: {
            auto vk_struct = const_cast<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->workgroupMemoryExplicitLayout8BitAccess,
                    "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayout8BitAccess"};
        }

        case Feature::workgroupMemoryExplicitLayoutScalarBlockLayout: {
            auto vk_struct = const_cast<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->workgroupMemoryExplicitLayoutScalarBlockLayout,
                    "VkPhysicalDeviceWorkgroupMemoryExplicitLayoutFeaturesKHR::workgroupMemoryExplicitLayoutScalarBlockLayout"};
        }

        case Feature::ycbcr2plane444Formats: {
            auto vk_struct = const_cast<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->ycbcr2plane444Formats, "VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT::ycbcr2plane444Formats"};
        }

        case Feature::ycbcrDegamma: {
            auto vk_struct = const_cast<VkPhysicalDeviceYcbcrDegammaFeaturesQCOM *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceYcbcrDegammaFeaturesQCOM>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceYcbcrDegammaFeaturesQCOM;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->ycbcrDegamma, "VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::ycbcrDegamma"};
        }

        case Feature::ycbcrImageArrays: {
            auto vk_struct = const_cast<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT *>(
                vku::FindStructInPNextChain<VkPhysicalDeviceYcbcrImageArraysFeaturesEXT>(*inout_pnext_chain));
            if (!vk_struct) {
                vk_struct = new VkPhysicalDeviceYcbcrImageArraysFeaturesEXT;
                *vk_struct = vku::InitStructHelper();
                if (*inout_pnext_chain) {
                    vvl::PnextChainAdd(*inout_pnext_chain, vk_struct);
                } else {
                    *inout_pnext_chain = vk_struct;
                }
            }
            return {&vk_struct->ycbcrImageArrays, "VkPhysicalDeviceYcbcrImageArraysFeaturesEXT::ycbcrImageArrays"};
        }
        default:
            assert(false);
            return {nullptr, ""};
    }
}
}  // namespace vkt
// NOLINTEND
