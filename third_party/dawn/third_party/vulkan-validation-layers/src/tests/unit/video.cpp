/*
 * Copyright (c) 2022-2025 The Khronos Group Inc.
 * Copyright (c) 2022-2025 RasterGrid Kft.
 * Modifications Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/video_objects.h"
#include "utils/vk_layer_utils.h"

class NegativeVideo : public VkVideoLayerTest {};

TEST_F(NegativeVideo, CodingScope) {
    TEST_DESCRIPTION("Tests calling functions inside/outside video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    // Video coding block must be ended before command buffer
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkEndCommandBuffer-None-06991");
    vk::EndCommandBuffer(cb.handle());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // vkCmdEndVideoCoding not allowed outside video coding block
    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-videocoding");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();

    cb.End();

    // vkCmdBeginVideoCoding not allowed inside video coding block
    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-videocoding");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();

    // vkCmdControlVideoCoding not allowed outside video coding block
    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-videocoding");
    cb.ControlVideoCoding(context.Control().Reset());
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, ProfileInvalidLumaChromaSubsampling) {
    TEST_DESCRIPTION("Test single bit set in VkVideoProfileInfoKHR chromaSubsampling, lumaBitDepth, and chromaBitDepth");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoProfileInfoKHR profile;

    // Multiple bits in chromaSubsampling
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07013");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR | VK_VIDEO_CHROMA_SUBSAMPLING_422_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in lumaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-lumaBitDepth-07014");
    profile = *config.Profile();
    profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();

    // Multiple bits in chromaBitDepth
    m_errorMonitor->SetDesiredError("VUID-VkVideoProfileInfoKHR-chromaSubsampling-07015");
    profile = *config.Profile();
    profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR | VK_VIDEO_COMPONENT_BIT_DEPTH_10_BIT_KHR;
    vk::GetPhysicalDeviceVideoCapabilitiesKHR(Gpu(), &profile, config.Caps());
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, VideoFormatQueryMissingProfile) {
    TEST_DESCRIPTION("vkGetPhysicalDeviceVideoFormatPropertiesKHR - missing profile info");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    auto format_info = vku::InitStruct<VkPhysicalDeviceVideoFormatInfoKHR>();
    format_info.imageUsage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    uint32_t format_count = 0;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk::GetPhysicalDeviceVideoFormatPropertiesKHR(Gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();

    auto profile_list = vku::InitStruct<VkVideoProfileListInfoKHR>();
    format_info.pNext = &profile_list;

    m_errorMonitor->SetDesiredError("VUID-vkGetPhysicalDeviceVideoFormatPropertiesKHR-pNext-06812");
    vk::GetPhysicalDeviceVideoFormatPropertiesKHR(Gpu(), &format_info, &format_count, nullptr);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, InUseDestroyed) {
    TEST_DESCRIPTION("Test destroying objects while they are still in use");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        config = GetConfig();
    }
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(context.Control().Reset());
    cb.EndVideoCoding(context.End());
    cb.End();

    context.Queue().Submit(cb);

    m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionKHR-videoSession-07192");
    vk::DestroyVideoSessionKHR(device(), context.Session(), nullptr);
    m_errorMonitor->VerifyFound();

    if (config.NeedsSessionParams()) {
        m_errorMonitor->SetDesiredError("VUID-vkDestroyVideoSessionParametersKHR-videoSessionParameters-07212");
        vk::DestroyVideoSessionParametersKHR(device(), context.SessionParams(), nullptr);
        m_errorMonitor->VerifyFound();
    }

    m_device->Wait();
}

TEST_F(NegativeVideo, CreateSessionVideoMaintenance1NotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot use inline queries without videoMaintenance1");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-08371");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionVideoMaintenance2NotEnabled) {
    TEST_DESCRIPTION("VkVideoSessionCreateInfoKHR - cannot use inline session parameters without videoMaintenance2");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigDecode();
    if (!config) {
        GTEST_SKIP() << "Test requires decode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10398");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInlineSessionParamsRequiresDecodeProfile) {
    TEST_DESCRIPTION("VkVideoSessionCreateInfoKHR - cannot use inline session parameters with encode");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_2_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance2);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires encode support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_INLINE_SESSION_PARAMETERS_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-flags-10399");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedMemoryNotEnabled) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content without protected memory");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionProtectedContentUnsupported) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - cannot enable protected content if not supported");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithoutProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with no protected content support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.flags = VK_VIDEO_SESSION_CREATE_PROTECTED_CONTENT_BIT_KHR;
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-protectedMemory-07189");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionUnsupportedProfile) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - unsupported profile");

    RETURN_IF_SKIP(Init());

    auto config = GetConfigInvalid();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    VkVideoSessionKHR session;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pVideoProfile-04845");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidReferencePictureCounts) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid reference picture slot and active counts");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();
    create_info.maxDpbSlots = config.Caps()->maxDpbSlots;
    create_info.maxActiveReferencePictures = config.Caps()->maxActiveReferencePictures;

    // maxDpbSlots too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04847");
    create_info.maxDpbSlots++;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxDpbSlots--;
    m_errorMonitor->VerifyFound();

    // maxActiveReferencePictures too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxActiveReferencePictures-04849");
    create_info.maxActiveReferencePictures++;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    create_info.maxActiveReferencePictures--;
    m_errorMonitor->VerifyFound();

    config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (config) {
        // maxDpbSlots is 0, but maxActiveReferencePictures is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 0;
        create_info.maxActiveReferencePictures = 1;
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();

        // maxActiveReferencePictures is 0, but maxDpbSlots is not
        m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxDpbSlots-04850");
        create_info.maxDpbSlots = 1;
        create_info.maxActiveReferencePictures = 0;
        vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateSessionInvalidMaxCodedExtent) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - maxCodedExtent outside of supported range");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = config.StdVersion();

    // maxCodedExtent.width too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.width;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too small
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->minCodedExtent;
    --create_info.maxCodedExtent.height;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.width too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.width;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();

    // maxCodedExtent.height too big
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-maxCodedExtent-04851");
    create_info.maxCodedExtent = config.Caps()->maxCodedExtent;
    ++create_info.maxCodedExtent.height;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateSessionInvalidStdHeaderVersion) {
    TEST_DESCRIPTION("vkCreateVideoSessionKHR - invalid Video Std header version");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkVideoSessionKHR session;
    VkVideoSessionCreateInfoKHR create_info = *config.SessionCreateInfo();
    VkExtensionProperties std_version = *config.StdVersion();
    create_info.pVideoProfile = config.Profile();
    create_info.pStdHeaderVersion = &std_version;

    // Video Std header version not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07191");
    ++std_version.specVersion;
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    --std_version.specVersion;
    m_errorMonitor->VerifyFound();

    // Video Std header name not supported
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionCreateInfoKHR-pStdHeaderVersion-07190");
    strcpy(std_version.extensionName, "invalid_std_header_name");
    vk::CreateVideoSessionKHR(device(), &create_info, nullptr, &session);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - memory binding related invalid usages");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    uint32_t mem_req_count;
    ASSERT_EQ(VK_SUCCESS, vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr));
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test can only run if video session needs memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count, vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    ASSERT_EQ(VK_SUCCESS, vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data()));

    std::vector<VkDeviceMemory> session_memory;
    std::vector<VkBindVideoSessionMemoryInfoKHR> bind_info(mem_req_count, vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>());
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        ASSERT_TRUE(m_device->Physical().SetMemoryType(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0));
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size * 2;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        bind_info[i].memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info[i].memory = memory;
        bind_info[i].memoryOffset = 0;
        bind_info[i].memorySize = mem_reqs[i].memoryRequirements.size;
    }

    // Duplicate memoryBindIndex
    if (mem_req_count > 1) {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-memoryBindIndex-07196");
        auto& duplicate = bind_info[mem_req_count / 2];
        auto backup = duplicate;
        duplicate = bind_info[0];
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        duplicate = backup;
        m_errorMonitor->VerifyFound();
    }

    // Invalid memoryBindIndex
    uint32_t invalid_bind_index = vvl::kU32Max;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        if (mem_reqs[i].memoryBindIndex < vvl::kU32Max) {
            invalid_bind_index = mem_reqs[i].memoryBindIndex + 1;
        }
    }
    if (invalid_bind_index != vvl::kU32Max) {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07197");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryBindIndex = invalid_bind_index;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incompatible memory type
    uint32_t invalid_mem_type_index = vvl::kU32Max;
    uint32_t invalid_mem_type_req_index = vvl::kU32Max;
    auto mem_props = m_device->Physical().memory_properties_;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        uint32_t mem_type_bits = mem_reqs[i].memoryRequirements.memoryTypeBits;
        for (uint32_t mem_type_index = 0; mem_type_index < mem_props.memoryTypeCount; ++mem_type_index) {
            if ((mem_type_bits & (1 << mem_type_index)) == 0) {
                invalid_mem_type_index = mem_type_index;
                invalid_mem_type_req_index = i;
                break;
            }
        }
        if (invalid_mem_type_index != vvl::kU32Max) break;
    }
    if (invalid_mem_type_index != vvl::kU32Max) {
        auto& mem_req = mem_reqs[invalid_mem_type_req_index].memoryRequirements;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        alloc_info.memoryTypeIndex = invalid_mem_type_index;
        alloc_info.allocationSize = mem_req.size * 2;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07198");
        auto& invalid = bind_info[invalid_mem_type_req_index];
        auto backup = invalid;
        invalid.memory = memory;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();

        vk::FreeMemory(device(), memory, nullptr);
    }

    // Incorrectly aligned memoryOffset
    uint32_t invalid_offset_align_index = vvl::kU32Max;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        if (mem_reqs[i].memoryRequirements.alignment > 1) {
            invalid_offset_align_index = i;
            break;
        }
    }
    if (invalid_offset_align_index != vvl::kU32Max) {
        auto& mem_req = mem_reqs[invalid_offset_align_index].memoryRequirements;

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[invalid_offset_align_index];
        auto backup = invalid;
        invalid.memoryOffset = mem_req.alignment / 2;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Incorrect memorySize
    {
        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07200");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memorySize += 16;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset
    {
        m_errorMonitor->SetDesiredError("VUID-VkBindVideoSessionMemoryInfoKHR-memoryOffset-07201");
        auto& invalid = bind_info[mem_req_count / 2];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize * 2;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Out-of-bounds memoryOffset + memorySize
    {
        uint32_t index = mem_req_count / 2;

        m_errorMonitor->SetDesiredError("VUID-VkBindVideoSessionMemoryInfoKHR-memorySize-07202");
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkBindVideoSessionMemoryKHR-pBindSessionMemoryInfos-07199");
        auto& invalid = bind_info[index];
        auto backup = invalid;
        invalid.memoryOffset = invalid.memorySize + 1;
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), mem_req_count, bind_info.data());
        invalid = backup;
        m_errorMonitor->VerifyFound();
    }

    // Already bound
    {
        uint32_t first_bind_count = mem_req_count / 2;
        if (first_bind_count == 0) {
            first_bind_count = 1;
        }

        vk::BindVideoSessionMemoryKHR(device(), context.Session(), first_bind_count, bind_info.data());

        m_errorMonitor->SetDesiredError("VUID-vkBindVideoSessionMemoryKHR-videoSession-07195");
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info[first_bind_count - 1]);
        m_errorMonitor->VerifyFound();
    }

    for (auto memory : session_memory) {
        vk::FreeMemory(device(), memory, nullptr);
    }
}

TEST_F(NegativeVideo, CreateSessionParamsIncompatibleTemplate) {
    TEST_DESCRIPTION("vkCreateVideoSessionParametersKHR - incompatible template");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(m_device, config);
    VideoContext context2(m_device, config);

    VkVideoSessionParametersCreateInfoKHR create_info = *config.SessionParamsCreateInfo();
    create_info.videoSessionParametersTemplate = context1.SessionParams();
    create_info.videoSession = context2.Session();

    VkVideoSessionParametersKHR params;
    m_errorMonitor->SetDesiredError("VUID-VkVideoSessionParametersCreateInfoKHR-videoSessionParametersTemplate-04855");
    vk::CreateVideoSessionParametersKHR(device(), &create_info, nullptr, &params);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, UpdateSessionParamsIncorrectSequenceCount) {
    TEST_DESCRIPTION("vkUpdateVideoSessionParametersKHR - invalid update sequence count");

    RETURN_IF_SKIP(Init());
    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context(m_device, config);

    auto update_info = vku::InitStruct<VkVideoSessionParametersUpdateInfoKHR>();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 2;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 1;
    ASSERT_EQ(VK_SUCCESS, vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 1;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkUpdateVideoSessionParametersKHR-pUpdateInfo-07215");
    update_info.updateSequenceCount = 3;
    vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info);
    m_errorMonitor->VerifyFound();

    update_info.updateSequenceCount = 2;
    ASSERT_EQ(VK_SUCCESS, vk::UpdateVideoSessionParametersKHR(device(), context.SessionParams(), &update_info));
}

TEST_F(NegativeVideo, BeginCodingUnsupportedCodecOp) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - unsupported video codec operation");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if ((QueueFamilyFlags(qfi) & (VK_QUEUE_VIDEO_DECODE_BIT_KHR | VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) &&
            ((QueueFamilyVideoCodecOps(qfi) & config.Profile()->videoCodecOperation) == 0)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family that supports video but not the specific codec op";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::CommandPool pool(*m_device, queue_family_index);
    vkt::CommandBuffer cb(*m_device, pool);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07231");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-None-07232");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSession) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for video session");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content support";
    }

    const bool use_protected = true;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources();

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, use_protected /* DPB */, use_protected /* src/dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07233");
    unprotected_cb.BeginVideoCoding(protected_context.Begin());
    m_errorMonitor->VerifyFound();
    unprotected_cb.End();

    protected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07234");
    protected_cb.BeginVideoCoding(unprotected_context.Begin());
    m_errorMonitor->VerifyFound();
    protected_cb.End();
}

TEST_F(NegativeVideo, BeginCodingProtectedNoFaultSlots) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - protectedNoFault tests for reference slots");

    AddRequiredFeature(vkt::Feature::protectedMemory);
    RETURN_IF_SKIP(Init());
    if (IsProtectedNoFaultSupported()) {
        GTEST_SKIP() << "Test requires protectedMemory support without protectedNoFault support";
    }

    VideoConfig config = GetConfigWithProtectedContent(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with protected content and reference picture support";
    }

    const bool use_protected = true;

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext unprotected_context(m_device, config);
    unprotected_context.CreateAndBindSessionMemory();
    unprotected_context.CreateResources(false /* bitstream */, use_protected /* DPB */, false /* src/dst image */);

    vkt::CommandBuffer& unprotected_cb = unprotected_context.CmdBuffer();

    VideoContext protected_context(m_device, config, use_protected);
    protected_context.CreateAndBindSessionMemory();
    protected_context.CreateResources(use_protected /* bitstream */, false /* DPB */, use_protected /* src/dst image */);

    vkt::CommandBuffer& protected_cb = protected_context.CmdBuffer();

    unprotected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07235");
    unprotected_cb.BeginVideoCoding(unprotected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    unprotected_cb.End();

    protected_cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginVideoCodingKHR-commandBuffer-07236");
    protected_cb.BeginVideoCoding(protected_context.Begin().AddResource(-1, 0));
    m_errorMonitor->VerifyFound();
    protected_cb.End();
}

TEST_F(NegativeVideo, BeginCodingSessionMemoryNotBound) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - session memory not bound");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    uint32_t mem_req_count;
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);
    if (mem_req_count == 0) {
        GTEST_SKIP() << "Test requires video session to need memory bindings";
    }

    std::vector<VkVideoSessionMemoryRequirementsKHR> mem_reqs(mem_req_count,
                                                              vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>());
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, mem_reqs.data());

    std::vector<VkDeviceMemory> session_memory;
    for (uint32_t i = 0; i < mem_req_count; ++i) {
        // Skip binding one of the memory bindings
        if (i == mem_req_count / 2) continue;

        VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
        m_device->Physical().SetMemoryType(mem_reqs[i].memoryRequirements.memoryTypeBits, &alloc_info, 0);
        alloc_info.allocationSize = mem_reqs[i].memoryRequirements.size;

        VkDeviceMemory memory = VK_NULL_HANDLE;
        ASSERT_EQ(VK_SUCCESS, vk::AllocateMemory(device(), &alloc_info, nullptr, &memory));
        session_memory.push_back(memory);

        VkBindVideoSessionMemoryInfoKHR bind_info = vku::InitStructHelper();
        bind_info.memoryBindIndex = mem_reqs[i].memoryBindIndex;
        bind_info.memory = memory;
        bind_info.memoryOffset = 0;
        bind_info.memorySize = mem_reqs[i].memoryRequirements.size;

        ASSERT_EQ(VK_SUCCESS, vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info));
    }

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSession-07237");
    cb.BeginVideoCoding(context.Begin());
    m_errorMonitor->VerifyFound();

    cb.End();

    for (auto memory : session_memory) {
        vk::FreeMemory(device(), memory, nullptr);
    }
}

TEST_F(NegativeVideo, BeginCodingInvalidSessionParams) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - invalid session parameters");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigWithParams(GetConfigs());
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with session parameters";
    }

    VideoContext context1(m_device, config);
    VideoContext context2(m_device, config);
    vkt::CommandBuffer& cb = context1.CmdBuffer();

    context1.CreateAndBindSessionMemory();

    VkVideoBeginCodingInfoKHR begin_info = context1.Begin();
    begin_info.videoSessionParameters = context2.SessionParams();

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-videoSessionParameters-04857");
    cb.BeginVideoCoding(begin_info);
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingIncompatRefPicProfile) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - reference picture must be compatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig configs[2] = {};
    const auto& all_configs = GetConfigs();
    for (uint32_t i = 0; i < all_configs.size(); ++i) {
        for (uint32_t j = i + 1; j < all_configs.size(); ++j) {
            const auto& coded_extent1 = all_configs[i].SessionCreateInfo()->maxCodedExtent;
            const auto& coded_extent2 = all_configs[j].SessionCreateInfo()->maxCodedExtent;
            const auto& dpb_format1 = *all_configs[i].DpbFormatProps();
            const auto& dpb_format2 = *all_configs[j].DpbFormatProps();
            if ((coded_extent1.width == coded_extent2.width) && (coded_extent1.height == coded_extent2.height) &&
                (dpb_format1.imageType == dpb_format2.imageType) && (dpb_format1.imageTiling == dpb_format2.imageTiling) &&
                (dpb_format1.format == dpb_format2.format) && (dpb_format1.imageUsageFlags == dpb_format2.imageUsageFlags)) {
                configs[0] = all_configs[i];
                configs[1] = all_configs[j];
            }
        }
    }
    if (!configs[0]) {
        GTEST_SKIP() << "Test requires two video profiles with matching DPB format/size";
    }

    for (uint32_t i = 0; i < 2; ++i) {
        configs[i].SessionCreateInfo()->maxDpbSlots = 1;
        configs[i].SessionCreateInfo()->maxActiveReferencePictures = 1;
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context1.CreateResources();
    context2.CreateAndBindSessionMemory();
    context2.CreateResources();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07240");
    cb.BeginVideoCoding(context1.Begin().AddResource(-1, context2.Dpb()->Picture(0)));
    m_errorMonitor->VerifyFound();
    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidResourceLayer) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - out-of-bounds layer index in VkVideoPictureResourceInfoKHR");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires video support with reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    // Invalid baseArrayLayer in VkVideoBeginCodingInfoKHR::pReferenceSlots
    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);
    res.baseArrayLayer = 5;

    m_errorMonitor->SetDesiredError("VUID-VkVideoPictureResourceInfoKHR-baseArrayLayer-07175");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidSlotIndex) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced DPB slot index is invalid");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 4));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 4 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 3;
    config.SessionCreateInfo()->maxActiveReferencePictures = 3;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-slotIndex-04856");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, 0).AddResource(3, 1).AddResource(-1, 2));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingResourcesNotUnique) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - referenced video picture resources are not unique");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for 2 reference pictures";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07238");
    cb.BeginVideoCoding(context.Begin().AddResource(0, 0).AddResource(1, 0));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingReferenceFormatMismatch) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - reference picture format mismatch");

    RETURN_IF_SKIP(Init());

    uint32_t alt_ref_format_index = 0;
    VideoConfig config =
        GetConfig(FilterConfigs(GetConfigsWithReferences(GetConfigs()), [&alt_ref_format_index](const VideoConfig& config) {
            const auto& format_props = config.SupportedDpbFormatProps();
            for (size_t i = 0; i < format_props.size(); ++i) {
                if (format_props[i].format != format_props[0].format && alt_ref_format_index == 0) {
                    alt_ref_format_index = i;
                    return true;
                }
            }
            return false;
        }));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with support for two reference picture formats";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoConfig config2 = config;
    config2.SetFormatProps(config.SupportedPictureFormatProps(), {config.SupportedDpbFormatProps()[alt_ref_format_index]});
    VideoDPB dpb(m_device, config2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07241");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, dpb.Picture(0)));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidCodedOffset) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded offset");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 5;
    res.codedOffset.y = 0;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07242");
    res.codedOffset.x = 0;
    res.codedOffset.y = 4;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidCodedExtent) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - invalid coded extent");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs()));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    config.SessionCreateInfo()->maxDpbSlots = 1;
    config.SessionCreateInfo()->maxActiveReferencePictures = 1;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    VkVideoPictureResourceInfoKHR res = context.Dpb()->Picture(0);

    cb.Begin();

    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width - 1;
    res.codedExtent.height = config.Caps()->minCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.Caps()->minCodedExtent.width;
    res.codedExtent.height = config.Caps()->minCodedExtent.height - 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    cb.EndVideoCoding(context.End());

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width + 1;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-pPictureResource-07243");
    res.codedExtent.width = config.SessionCreateInfo()->maxCodedExtent.width;
    res.codedExtent.height = config.SessionCreateInfo()->maxCodedExtent.height + 1;
    cb.BeginVideoCoding(context.Begin().AddResource(-1, res));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginCodingInvalidSeparateReferenceImages) {
    TEST_DESCRIPTION("vkCmdBeginCodingKHR - unsupported use of separate reference images");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig(GetConfigsWithReferences(GetConfigs(), 2));
    if (!config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    if (config.Caps()->flags & VK_VIDEO_CAPABILITY_SEPARATE_REFERENCE_IMAGES_BIT_KHR) {
        GTEST_SKIP() << "This test can only run on implementations with no support for separate reference images";
    }

    config.SessionCreateInfo()->maxDpbSlots = 2;
    config.SessionCreateInfo()->maxActiveReferencePictures = 2;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    VideoDPB separate_dpb(m_device, config);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-VkVideoBeginCodingInfoKHR-flags-07244");
    cb.BeginVideoCoding(context.Begin().AddResource(-1, context.Dpb()->Picture(0)).AddResource(-1, separate_dpb.Picture(1)));
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, EndCodingActiveQueriesNotAllowed) {
    TEST_DESCRIPTION("vkCmdBeginVideoCodingKHR - there must be no active query");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->SetDesiredError("VUID-vkCmdEndVideoCodingKHR-None-07251");
    cb.EndVideoCoding(context.End());
    m_errorMonitor->VerifyFound();
    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, ControlSessionUninitialized) {
    TEST_DESCRIPTION("vkCmdControlVideoCodingKHR - session uninitialized");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateResources();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    auto rc_info = vku::InitStruct<VkVideoEncodeRateControlInfoKHR>();
    auto control_info = vku::InitStruct<VkVideoCodingControlInfoKHR>(&rc_info);
    control_info.flags = VK_VIDEO_CODING_CONTROL_ENCODE_RATE_CONTROL_BIT_KHR;

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    cb.ControlVideoCoding(control_info);
    cb.EndVideoCoding(context.End());
    cb.End();

    m_errorMonitor->SetDesiredError("VUID-vkCmdControlVideoCodingKHR-flags-07017");
    context.Queue().Submit(cb);
    m_errorMonitor->VerifyFound();

    m_device->Wait();
}

TEST_F(NegativeVideo, CreateBufferInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateBuffer - invalid/missing profile list");

    AddOptionalExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddOptionalFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.size = 2048;

    if (decode_config) {
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        VkVideoProfileInfoKHR profiles[] = {*decode_config.Profile(), *decode_config.Profile()};
        video_profiles.profileCount = 2;
        video_profiles.pProfiles = profiles;
        buffer_ci.pNext = &video_profiles;

        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        video_profiles.profileCount = 1;
        video_profiles.pProfiles = decode_config.Profile();

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }

    if (decode_config && encode_config) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04814");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();
    }

    if (encode_config) {
        buffer_ci.pNext = nullptr;
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-usage-04814");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateBufferProfileIndependentNotSupported) {
    TEST_DESCRIPTION("vkCreateBuffer - profile independent buffer creation requires videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    buffer_ci.size = 2048;

    if (GetConfigDecode()) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
    }

    if (GetConfigEncode()) {
        buffer_ci.usage |= VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkBufferCreateInfo-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkBufferCreateInfo-flags-08325");
    vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateBufferProfileIndependent) {
    TEST_DESCRIPTION("vkCreateBuffer - profile independent buffer creation with invalid parameters");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    VkBuffer buffer = VK_NULL_HANDLE;
    VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
    VkBufferCreateInfo buffer_ci = vku::InitStructHelper();
    buffer_ci.flags = VK_BUFFER_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    buffer_ci.size = 2048;

    if (decode_config) {
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;

        // Creating profile independent buffers without a profile list is allowed
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);

        // An invalid profile list, however, should still cause a validation failure
        VkVideoProfileInfoKHR profiles[] = {*decode_config.Profile(), *decode_config.Profile()};
        video_profiles.profileCount = 2;
        video_profiles.pProfiles = profiles;
        buffer_ci.pNext = &video_profiles;

        m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        m_errorMonitor->VerifyFound();

        // But a valid profile list should not
        video_profiles.profileCount = 1;
        video_profiles.pProfiles = decode_config.Profile();

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }

    if (encode_config) {
        // Creating profile independent buffers without a profile list is allowed
        buffer_ci.pNext = nullptr;
        buffer_ci.usage = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;

        vk::CreateBuffer(device(), &buffer_ci, nullptr, &buffer);
        vk::DestroyBuffer(device(), buffer, nullptr);
    }
}

TEST_F(NegativeVideo, CreateImageInvalidProfileList) {
    TEST_DESCRIPTION("vkCreateImage - invalid/missing profile list");

    AddOptionalExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddOptionalFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        VkImage image = VK_NULL_HANDLE;
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.imageType = config.PictureFormatProps()->imageType;
        image_ci.format = config.PictureFormatProps()->format;
        image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = config.PictureFormatProps()->imageTiling;
        image_ci.usage = config.PictureFormatProps()->imageUsageFlags;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (config.IsDecode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-04815");
        }
        if (config.IsEncode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-usage-04816");
        }
        vk::CreateImage(device(), &image_ci, nullptr, &image);
        m_errorMonitor->VerifyFound();

        if (config.IsDecode()) {
            VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
            VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
            video_profiles.profileCount = 2;
            video_profiles.pProfiles = profiles;
            image_ci.pNext = &video_profiles;

            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-pNext-06811");
            m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeVideo, CreateImageProfileIndependentNotSupported) {
    TEST_DESCRIPTION("vkCreateImage - profile independent image creation requires videoMaintenance1");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VkImage image = VK_NULL_HANDLE;
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
    image_ci.imageType = config.PictureFormatProps()->imageType;
    image_ci.format = config.PictureFormatProps()->format;
    image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = config.PictureFormatProps()->imageTiling;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (GetConfigDecode()) {
        image_ci.usage |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
    }

    if (GetConfigEncode()) {
        image_ci.usage |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
    }

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-flags-parameter");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08328");
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImage - profile independent image creation with invalid parameters");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfigDecode();
    VideoConfig encode_config = GetConfigEncode();
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires video decode or encode support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        VkImage image = VK_NULL_HANDLE;
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR;
        image_ci.imageType = config.PictureFormatProps()->imageType;
        image_ci.format = config.PictureFormatProps()->format;
        image_ci.extent = {config.MaxCodedExtent().width, config.MaxCodedExtent().height, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = config.PictureFormatProps()->imageTiling;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (config.IsDecode()) {
            // Video profile independent DECODE_DPB usage is not allowed
            image_ci.usage = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08329");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();

            // Except when DECODE_DST usage is also requested
            image_ci.usage |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            vk::DestroyImage(device(), image, nullptr);

            // Profile list is still validated though
            VkVideoProfileListInfoKHR video_profiles = vku::InitStructHelper();
            VkVideoProfileInfoKHR profiles[] = {*config.Profile(), *config.Profile()};
            video_profiles.profileCount = 2;
            video_profiles.pProfiles = profiles;
            image_ci.pNext = &video_profiles;

            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-pNext-06811");
            m_errorMonitor->SetDesiredError("VUID-VkVideoProfileListInfoKHR-pProfiles-06813");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }

        if (config.IsEncode()) {
            // Video profile independent ENCODE_DPB usage is not allowed
            image_ci.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
            m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
            m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-flags-08331");
            vk::CreateImage(device(), &image_ci, nullptr, &image);
            m_errorMonitor->VerifyFound();
        }
    }
}

TEST_F(NegativeVideo, CreateImageIncompatibleProfile) {
    TEST_DESCRIPTION("vkCreateImage - image parameters are incompatible with the video profile");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
    profile_list.profileCount = 1;
    profile_list.pProfiles = config.Profile();

    const VkVideoFormatPropertiesKHR* format_props = config.DpbFormatProps();
    VkImageCreateInfo image_ci = vku::InitStructHelper();
    image_ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    image_ci.pNext = &profile_list;
    image_ci.imageType = format_props->imageType;
    image_ci.format = format_props->format;
    image_ci.extent = {1024, 1024, 1};
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 6;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.tiling = format_props->imageTiling;
    image_ci.usage = format_props->imageUsageFlags;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image = VK_NULL_HANDLE;

    m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageCreateInfo-imageCreateMaxMipLevels-02251");
    m_errorMonitor->SetDesiredError("VUID-VkImageCreateInfo-pNext-06811");
    image_ci.format = VK_FORMAT_D16_UNORM;
    vk::CreateImage(device(), &image_ci, nullptr, &image);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CreateImageViewInvalidViewType) {
    TEST_DESCRIPTION("vkCreateImageView - view type not compatible with video usage");

    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    VideoConfig encode_config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    for (auto config : {decode_config, encode_config}) {
        if (!config) continue;

        config.SessionCreateInfo()->maxDpbSlots = 1;
        config.SessionCreateInfo()->maxActiveReferencePictures = 1;

        VkVideoProfileListInfoKHR profile_list = vku::InitStructHelper();
        profile_list.profileCount = 1;
        profile_list.pProfiles = config.Profile();

        const VkVideoFormatPropertiesKHR* format_props = config.DpbFormatProps();
        VkImageCreateInfo image_ci = vku::InitStructHelper();
        image_ci.pNext = &profile_list;
        image_ci.imageType = format_props->imageType;
        image_ci.format = format_props->format;
        image_ci.extent = {1024, 1024, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 6;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = format_props->imageTiling;
        image_ci.usage = format_props->imageUsageFlags;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);

        VkImageViewCreateInfo image_view_ci = vku::InitStructHelper();
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        image_view_ci.format = image_ci.format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 6;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-image-01003");
        if (config.IsDecode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-04817");
        }
        if (config.IsEncode()) {
            m_errorMonitor->SetDesiredError("VUID-VkImageViewCreateInfo-image-04818");
        }
        VkImageView image_view = VK_NULL_HANDLE;
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, CreateImageViewProfileIndependent) {
    TEST_DESCRIPTION("vkCreateImageView - video usage not supported in view created from video profile independent image");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig decode_config = GetConfig(GetConfigsWithReferences(GetConfigsDecode()));
    VideoConfig encode_config = GetConfig(GetConfigsWithReferences(GetConfigsEncode()));
    if (!decode_config && !encode_config) {
        GTEST_SKIP() << "Test requires a video profile with reference picture support";
    }

    struct TestCase {
        VideoConfig& config;
        VkImageUsageFlags usage;
        const char* vuid;
    };

    std::vector<TestCase> test_cases = {
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08333"},
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08334"},
        {decode_config, VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08335"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08336"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08337"},
        {encode_config, VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR, "VUID-VkImageViewCreateInfo-image-08338"},
    };

    // We choose a format that is not expected to support video usage
    VkFormat format = VK_FORMAT_R8G8_SNORM;
    VkFormatFeatureFlags video_format_flags =
        VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR |
        VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR | VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR;
    VkFormatProperties format_props;
    vk::GetPhysicalDeviceFormatProperties(Gpu(), format, &format_props);
    if ((format_props.optimalTilingFeatures & video_format_flags) != 0) {
        GTEST_SKIP() << "Test expects R8G8_SNORM format to not support video usage";
    }

    for (const auto& test_case : test_cases) {
        if (!test_case.config) {
            continue;
        }

        auto image_ci = vku::InitStruct<VkImageCreateInfo>();
        image_ci.flags = VK_IMAGE_CREATE_VIDEO_PROFILE_INDEPENDENT_BIT_KHR | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.format = format;
        image_ci.extent = {1024, 1024, 1};
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = 1;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        vkt::Image image(*m_device, image_ci);

        auto image_usage_ci = vku::InitStruct<VkImageViewUsageCreateInfo>();
        image_usage_ci.usage = test_case.usage;
        auto image_view_ci = vku::InitStruct<VkImageViewCreateInfo>(&image_usage_ci);
        image_view_ci.image = image.handle();
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_ci.format = format;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.subresourceRange.layerCount = 1;

        VkImageView image_view = VK_NULL_HANDLE;

        m_errorMonitor->SetAllowedFailureMsg("VUID-VkImageViewCreateInfo-pNext-02662");
        m_errorMonitor->SetDesiredError(test_case.vuid);
        vk::CreateImageView(device(), &image_view_ci, nullptr, &image_view);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, BeginQueryIncompatibleQueueFamily) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status only queries require queue family support");

    RETURN_IF_SKIP(Init());

    uint32_t queue_family_index = VK_QUEUE_FAMILY_IGNORED;
    for (uint32_t qfi = 0; qfi < QueueFamilyCount(); ++qfi) {
        if (!QueueFamilySupportsResultStatusOnlyQueries(qfi)) {
            queue_family_index = qfi;
            break;
        }
    }

    if (queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        GTEST_SKIP() << "Test requires a queue family with no support for result status queries";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    vkt::CommandPool cmd_pool(*m_device, queue_family_index, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkt::CommandBuffer cb(*m_device, cmd_pool);

    cb.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07126");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.End();
}

TEST_F(NegativeVideo, BeginQueryVideoCodingScopeQueryAlreadyActive) {
    TEST_DESCRIPTION("vkCmdBeginQuery - there must be no active query in video scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool(2);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryPool-01922");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-07127");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 1, 0);
    m_errorMonitor->VerifyFound();

    vk::CmdEndQuery(cb.handle(), context.StatusQueryPool(), 0);
    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryResultStatusProfileMismatch) {
    TEST_DESCRIPTION("vkCmdBeginQuery - result status query must have been created with the same profile");

    RETURN_IF_SKIP(Init());

    auto configs = GetConfigs();
    if (configs.size() < 2) {
        GTEST_SKIP() << "Test requires support for at least two video profiles";
    }

    if (!QueueFamilySupportsResultStatusOnlyQueries(configs[0].QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    VideoContext context1(m_device, configs[0]);
    VideoContext context2(m_device, configs[1]);
    context1.CreateAndBindSessionMemory();
    context2.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context1.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context1.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07128");
    vk::CmdBeginQuery(cb.handle(), context2.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context1.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryVideoCodingScopeIncompatibleQueryType) {
    TEST_DESCRIPTION("vkCmdBeginQuery - incompatible query type used in video coding scope");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_OCCLUSION, 1);

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-00803");
    m_errorMonitor->SetAllowedFailureMsg("VUID-vkCmdBeginQuery-queryType-07128");
    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-queryType-07131");
    vk::CmdBeginQuery(cb.handle(), query_pool.handle(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, BeginQueryInlineQueries) {
    TEST_DESCRIPTION("vkCmdBeginQuery - bound video session was created with VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR");

    AddRequiredExtensions(VK_KHR_VIDEO_MAINTENANCE_1_EXTENSION_NAME);
    AddRequiredFeature(vkt::Feature::videoMaintenance1);
    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }
    if (!QueueFamilySupportsResultStatusOnlyQueries(config.QueueFamilyIndex())) {
        GTEST_SKIP() << "Test requires video queue to support result status queries";
    }

    config.SessionCreateInfo()->flags |= VK_VIDEO_SESSION_CREATE_INLINE_QUERIES_BIT_KHR;

    VideoContext context(m_device, config);
    context.CreateAndBindSessionMemory();
    context.CreateStatusQueryPool();

    vkt::CommandBuffer& cb = context.CmdBuffer();

    cb.Begin();
    cb.BeginVideoCoding(context.Begin());

    m_errorMonitor->SetDesiredError("VUID-vkCmdBeginQuery-None-08370");
    vk::CmdBeginQuery(cb.handle(), context.StatusQueryPool(), 0, 0);
    m_errorMonitor->VerifyFound();

    cb.EndVideoCoding(context.End());
    cb.End();
}

TEST_F(NegativeVideo, GetQueryPoolResultsVideoQueryDataSize) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test expected data size for video query results");
    SetInstancePNext(&kDisableMessageLimit);
    RETURN_IF_SKIP(Init());

    auto config = GetConfigEncode();
    if (!config) {
        GTEST_SKIP() << "Test requires video encode support";
    }

    auto feedback_flags = config.EncodeCaps()->supportedEncodeFeedbackFlags;
    auto feedback_flag_count = GetBitSetCount(feedback_flags);
    uint32_t total_query_count = 4;

    VideoContext context(m_device, config);
    context.CreateEncodeFeedbackQueryPool(total_query_count, feedback_flags);

    auto query_pool = context.EncodeFeedbackQueryPool();
    std::vector<uint64_t> results(feedback_flag_count + 1);

    for (uint32_t query_count = 1; query_count <= total_query_count; query_count++) {
        size_t total_feedback_count = feedback_flag_count * query_count;

        // Test 32-bit no availability/status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count - 1), results.data(),
                                sizeof(uint32_t) * (total_feedback_count - 1), 0);
        m_errorMonitor->VerifyFound();

        // Test 32-bit with availability
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();

        // Test 32-bit with status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint32_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
        m_errorMonitor->VerifyFound();

        // Test 64-bit no availability/status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count - 1), results.data(),
                                sizeof(uint64_t) * (total_feedback_count - 1), VK_QUERY_RESULT_64_BIT);
        m_errorMonitor->VerifyFound();

        // Test 64-bit with availability
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
        m_errorMonitor->VerifyFound();

        // Test 64-bit with status
        m_errorMonitor->SetAllowedFailureMsg("VUID-vkGetQueryPoolResults-None-09401");
        m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-dataSize-00817");
        vk::GetQueryPoolResults(device(), query_pool, 0, query_count, sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                results.data(), sizeof(uint64_t) * (total_feedback_count + query_count - 1),
                                VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_STATUS_BIT_KHR);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeVideo, GetQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkGetQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    uint32_t status;
    VkQueryResultFlags flags;

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-queryType-09442");
    flags = 0;
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkGetQueryPoolResults-flags-09443");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::GetQueryPoolResults(device(), query_pool.handle(), 0, 1, sizeof(status), &status, sizeof(status), flags);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideo, CopyQueryPoolResultsStatusBit) {
    TEST_DESCRIPTION("vkCmdCopyQueryPoolResults - test invalid use of VK_QUERY_RESULT_WITH_STATUS_BIT_KHR");

    RETURN_IF_SKIP(Init());

    if (!GetConfig()) {
        GTEST_SKIP() << "Test requires video support";
    }

    vkt::QueryPool query_pool(*m_device, VK_QUERY_TYPE_RESULT_STATUS_ONLY_KHR, 1);

    VkQueryResultFlags flags;

    vkt::Buffer buffer(*m_device, sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    m_command_buffer.Begin();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-queryType-09442");
    flags = 0;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_errorMonitor->SetDesiredError("VUID-vkCmdCopyQueryPoolResults-flags-09443");
    flags = VK_QUERY_RESULT_WITH_STATUS_BIT_KHR | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT;
    vk::CmdCopyQueryPoolResults(m_command_buffer.handle(), query_pool.handle(), 0, 1, buffer.handle(), 0, sizeof(uint32_t), flags);
    m_errorMonitor->VerifyFound();

    m_command_buffer.End();
}

TEST_F(NegativeVideoBestPractices, GetVideoSessionMemoryRequirements) {
    TEST_DESCRIPTION("vkGetVideoSessionMemoryRequirementsKHR - best practices");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    auto mem_req = vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>();
    uint32_t mem_req_count = 1;

    m_errorMonitor->SetDesiredWarning("BestPractices-vkGetVideoSessionMemoryRequirementsKHR-count-not-retrieved");
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeVideoBestPractices, BindVideoSessionMemory) {
    TEST_DESCRIPTION("vkBindVideoSessionMemoryKHR - best practices");

    RETURN_IF_SKIP(Init());

    VideoConfig config = GetConfig();
    if (!config) {
        GTEST_SKIP() << "Test requires video support";
    }

    VideoContext context(m_device, config);

    // Create a buffer to get non-video-related memory requirements
    VkBufferCreateInfo buffer_create_info =
        vku::InitStruct<VkBufferCreateInfo>(nullptr, static_cast<VkBufferCreateFlags>(0), static_cast<VkDeviceSize>(4096),
                                          static_cast<VkBufferUsageFlags>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT));
    vkt::Buffer buffer(*m_device, buffer_create_info);
    VkMemoryRequirements buf_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &buf_mem_reqs);

    // Create non-video-related DeviceMemory
    VkMemoryAllocateInfo alloc_info = vku::InitStructHelper();
    alloc_info.allocationSize = buf_mem_reqs.size;
    ASSERT_TRUE(m_device->Physical().SetMemoryType(buf_mem_reqs.memoryTypeBits, &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    vkt::DeviceMemory memory(*m_device, alloc_info);

    // Set VkBindVideoSessionMemoryInfoKHR::memory to an allocation created before GetVideoSessionMemoryRequirementsKHR was called
    auto bind_info = vku::InitStruct<VkBindVideoSessionMemoryInfoKHR>();
    bind_info.memory = memory;
    bind_info.memoryOffset = 0;
    bind_info.memorySize = alloc_info.allocationSize;

    m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-count-not-retrieved");
    vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
    m_errorMonitor->VerifyFound();

    uint32_t mem_req_count = 0;
    vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, nullptr);

    if (mem_req_count > 0) {
        m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
        vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
        m_errorMonitor->VerifyFound();

        if (mem_req_count > 1) {
            auto mem_req = vku::InitStruct<VkVideoSessionMemoryRequirementsKHR>();
            mem_req_count = 1;

            vk::GetVideoSessionMemoryRequirementsKHR(device(), context.Session(), &mem_req_count, &mem_req);

            m_errorMonitor->SetDesiredWarning("BestPractices-vkBindVideoSessionMemoryKHR-requirements-not-all-retrieved");
            vk::BindVideoSessionMemoryKHR(device(), context.Session(), 1, &bind_info);
            m_errorMonitor->VerifyFound();
        }
    }
}
