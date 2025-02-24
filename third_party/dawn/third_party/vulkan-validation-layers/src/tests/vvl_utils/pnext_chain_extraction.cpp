/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/test_common.h"

#include "generated/pnext_chain_extraction.h"

// Return true iff all sTypes are found in chain, in order
bool FindSTypes(void* chain, const std::vector<VkStructureType>& sTypes_list) {
    size_t sTypes_list_i = 0;
    while (chain) {
        const auto* vk_struct = reinterpret_cast<const VkBaseOutStructure*>(chain);
        if (vk_struct->sType != sTypes_list[sTypes_list_i++]) {
            return false;
        }
        chain = vk_struct->pNext;
    }
    return sTypes_list_i == sTypes_list.size();
}

// Extract all structs from a pNext chain
TEST(PnextChainExtract, Extract1) {
    // Those structs extend VkPhysicalDeviceImageFormatInfo2
    VkImageCompressionControlEXT s1 = vku::InitStructHelper();
    VkImageFormatListCreateInfo s2 = vku::InitStructHelper(&s1);
    VkImageStencilUsageCreateInfo s3 = vku::InitStructHelper(&s2);
    VkOpticalFlowImageFormatInfoNV s4 = vku::InitStructHelper(&s3);

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 extracted_chain{};
    void* chain_begin = vvl::PnextChainExtract(&s4, extracted_chain);

    const std::vector<VkStructureType> expected_sTypes = {s1.sType, s2.sType, s3.sType, s4.sType};
    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
}

// Extract all structs mentioned in tuple vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 and found in input pNext chain
TEST(PnextChainExtract, Extract2) {
    // Those structs extend VkPhysicalDeviceImageFormatInfo2
    VkImageCompressionControlEXT s1 = vku::InitStructHelper();
    VkImageFormatListCreateInfo s2 = vku::InitStructHelper(&s1);
    VkImageStencilUsageCreateInfo s3 = vku::InitStructHelper(&s2);
    VkOpticalFlowImageFormatInfoNV s4 = vku::InitStructHelper(&s3);

    // Those do not
    VkExternalMemoryImageCreateInfo wrong1 = vku::InitStructHelper(&s4);
    VkImageDrmFormatModifierListCreateInfoEXT wrong2 = vku::InitStructHelper(&wrong1);

    // And this one does
    VkVideoProfileListInfoKHR s5 = vku::InitStructHelper(&wrong2);

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 extracted_chain{};
    void* chain_begin = vvl::PnextChainExtract(&s5, extracted_chain);

    const std::vector<VkStructureType> expected_sTypes = {s1.sType, s2.sType, s3.sType, s4.sType, s5.sType};
    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
}

// Test that no struct is extracted when no struct from a pNext chain extends the reference struct, here
// VkPhysicalDeviceImageFormatInfo2
TEST(PnextChainExtract, Extract3) {
    // Those structs do not extend VkPhysicalDeviceImageFormatInfo2
    VkExternalMemoryImageCreateInfo wrong1 = vku::InitStructHelper();
    VkImageDrmFormatModifierListCreateInfoEXT wrong2 = vku::InitStructHelper(&wrong1);

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 extracted_chain{};
    void* chain_begin = vvl::PnextChainExtract(&wrong2, extracted_chain);

    const std::vector<VkStructureType> expected_sTypes = {};
    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
}

// Extract all structs from a pNext chain, add a new element, then remove it
TEST(PnextChainExtract, ExtractAddRemove1) {
    // Those structs extend VkPhysicalDeviceImageFormatInfo2
    VkImageCompressionControlEXT s1 = vku::InitStructHelper();
    VkImageFormatListCreateInfo s2 = vku::InitStructHelper(&s1);
    VkImageStencilUsageCreateInfo s3 = vku::InitStructHelper(&s2);
    VkOpticalFlowImageFormatInfoNV s4 = vku::InitStructHelper(&s3);

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 extracted_chain{};
    void* chain_begin = vvl::PnextChainExtract(&s4, extracted_chain);

    std::vector<VkStructureType> expected_sTypes = {s1.sType, s2.sType, s3.sType, s4.sType};
    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));

    {
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT s5 = vku::InitStructHelper();
        vvl::PnextChainScopedAdd scoped_add_s5(chain_begin, &s5);
        expected_sTypes.emplace_back(s5.sType);
        ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
        expected_sTypes.resize(expected_sTypes.size() - 1);
    }

    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
}

// Extract all structs from a pNext chain, add two new elements, in a nested fashion, then remove them
TEST(PnextChainExtract, ExtractAddRemove2) {
    // Those structs extend VkPhysicalDeviceImageFormatInfo2
    VkImageCompressionControlEXT s1 = vku::InitStructHelper();
    VkImageFormatListCreateInfo s2 = vku::InitStructHelper(&s1);
    VkImageStencilUsageCreateInfo s3 = vku::InitStructHelper(&s2);
    VkOpticalFlowImageFormatInfoNV s4 = vku::InitStructHelper(&s3);

    vvl::PnextChainVkPhysicalDeviceImageFormatInfo2 extracted_chain{};
    void* chain_begin = vvl::PnextChainExtract(&s4, extracted_chain);

    std::vector<VkStructureType> expected_sTypes = {s1.sType, s2.sType, s3.sType, s4.sType};
    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));

    {
        VkPhysicalDeviceImageDrmFormatModifierInfoEXT s5 = vku::InitStructHelper();
        vvl::PnextChainScopedAdd scoped_add_s5(chain_begin, &s5);
        expected_sTypes.emplace_back(s5.sType);
        ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));

        {
            VkPhysicalDeviceImageViewImageFormatInfoEXT s6 = vku::InitStructHelper();
            vvl::PnextChainScopedAdd scoped_add_s6(chain_begin, &s6);
            expected_sTypes.emplace_back(s6.sType);
            ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
            expected_sTypes.resize(expected_sTypes.size() - 1);
        }

        expected_sTypes.resize(expected_sTypes.size() - 1);
    }

    ASSERT_TRUE(FindSTypes(chain_begin, expected_sTypes));
}
