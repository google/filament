/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
//stype-check off

#include <cstdarg>
#include <cstdint>
#include "../framework/layer_validation_tests.h"

class NegativeLayerSettings : public VkLayerTest {};

TEST_F(NegativeLayerSettings, CustomStypeStructString) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a list (string)");

    // Create a custom structure
    typedef struct CustomStruct {
        VkStructureType sType;
        const void *pNext;
        uint32_t custom_data;
    } CustomStruct;

    uint32_t custom_stype = 3000300000;
    CustomStruct custom_struct;
    custom_struct.pNext = nullptr;
    custom_struct.sType = static_cast<VkStructureType>(custom_stype);
    custom_struct.custom_data = 44;

    // Communicate list of structinfo pairs to layers
    const char *id[] = {"3000300000", "24"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_STRING_EXT,
                                       static_cast<uint32_t>(std::size(id)), &id};
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

static std::string format(const char *message, ...) {
    std::size_t const STRING_BUFFER(4096);

    assert(message != nullptr);
    assert(strlen(message) < STRING_BUFFER);

    char buffer[STRING_BUFFER];
    va_list list;

    va_start(list, message);
    vsnprintf(buffer, STRING_BUFFER, message, list);
    va_end(list);

    return buffer;
}

TEST_F(NegativeLayerSettings, CustomStypeStructStringArray) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a vector of strings");

    // Create a custom structure
    typedef struct CustomStruct {
        VkStructureType sType;
        const void *pNext;
        uint32_t custom_data;
    } CustomStruct;

    const uint32_t custom_stype_a = 3000300000;
    CustomStruct custom_struct_a;
    custom_struct_a.pNext = nullptr;
    custom_struct_a.sType = static_cast<VkStructureType>(custom_stype_a);
    custom_struct_a.custom_data = 44;

    const uint32_t custom_stype_b = 3000300001;
    CustomStruct custom_struct_b;
    custom_struct_b.pNext = &custom_struct_a;
    custom_struct_b.sType = static_cast<VkStructureType>(custom_stype_b);
    custom_struct_b.custom_data = 88;

    // Communicate list of structinfo pairs to layers, including a duplicate which should get filtered out
    const std::string string_stype_a = format("%u", custom_stype_a);
    const std::string string_stype_b = format("%u", custom_stype_b);
    const std::string sizeof_struct = format("%d", sizeof(CustomStruct));

    const char *ids[] = {
        string_stype_a.c_str(), sizeof_struct.c_str(),
        string_stype_b.c_str(), sizeof_struct.c_str(),
        string_stype_a.c_str(), sizeof_struct.c_str(),
    };
    const VkLayerSettingEXT setting = {
        OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_STRING_EXT, static_cast<uint32_t>(std::size(ids)), &ids};
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {
        VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

TEST_F(NegativeLayerSettings, CustomStypeStructIntegerArray) {
    TEST_DESCRIPTION("Positive Test for ability to specify custom pNext structs using a vector of integers");

    // Create a custom structure
    typedef struct CustomStruct {
        VkStructureType sType;
        const void *pNext;
        uint32_t custom_data;
    } CustomStruct;

    const uint32_t custom_stype_a = 3000300000;
    CustomStruct custom_struct_a;
    custom_struct_a.pNext = nullptr;
    custom_struct_a.sType = static_cast<VkStructureType>(custom_stype_a);
    custom_struct_a.custom_data = 44;

    const uint32_t custom_stype_b = 3000300001;
    CustomStruct custom_struct_b;
    custom_struct_b.pNext = &custom_struct_a;
    custom_struct_b.sType = static_cast<VkStructureType>(custom_stype_b);
    custom_struct_b.custom_data = 88;

    // Communicate list of structinfo pairs to layers, including a duplicate which should get filtered out
    const uint32_t ids[] = {
        custom_stype_a, sizeof(CustomStruct),
        custom_stype_b, sizeof(CustomStruct),
        custom_stype_a, sizeof(CustomStruct)
    };

    const VkLayerSettingEXT setting[] = {
        {OBJECT_LAYER_NAME, "custom_stype_list", VK_LAYER_SETTING_TYPE_UINT32_EXT, static_cast<uint32_t>(std::size(ids)), ids}
    };
    VkLayerSettingsCreateInfoEXT layer_setting_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, setting};

    RETURN_IF_SKIP(InitFramework(&layer_setting_create_info));
    RETURN_IF_SKIP(InitState());

    vkt::Buffer buffer(*m_device, 1024, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT);
    VkBufferViewCreateInfo bvci = vku::InitStructHelper(&custom_struct_b);  // Add custom struct through pNext
    bvci.buffer = buffer.handle();
    bvci.format = VK_FORMAT_R32_SFLOAT;
    bvci.range = VK_WHOLE_SIZE;
    vkt::BufferView buffer_view(*m_device, bvci);
}

TEST_F(NegativeLayerSettings, DuplicateMessageLimit) {
    TEST_DESCRIPTION("Use the duplicate_message_limit setting and verify correct operation");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    uint32_t value = 3;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &value};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    VkPhysicalDeviceProperties2KHR properties2 = vku::InitStructHelper(&bogus_struct);

    // Should get the first three errors just fine
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
    m_errorMonitor->VerifyFound();
    m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
    vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
    m_errorMonitor->VerifyFound();

    // Limit should prevent the message from coming through a fourth time
    vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
}

TEST_F(NegativeLayerSettings, DuplicateMessageLimitZero) {
    TEST_DESCRIPTION("Use the duplicate_message_limit setting with zero explicitly");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    uint32_t value = 0;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &value};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    VkPhysicalDeviceProperties2KHR properties2 = vku::InitStructHelper(&bogus_struct);

    const uint32_t default_value = 10; // what we set in vkconfig
    for (uint32_t i = 0; i < default_value + 1; i++) {
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
        vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeLayerSettings, DuplicateMessageLimitNone) {
    TEST_DESCRIPTION("Don't use duplicate_message_limit setting it with zero implicitly");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    VkPhysicalDeviceProperties2KHR properties2 = vku::InitStructHelper(&bogus_struct);

    const uint32_t default_value = 10; // what we set in vkconfig
    for (uint32_t i = 0; i < default_value; i++) {
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
        vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
        m_errorMonitor->VerifyFound();
    }

    // Limit should prevent the message from coming through now
    vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
}

TEST_F(NegativeLayerSettings, DuplicateMessageLimitDisable) {
    TEST_DESCRIPTION("use enable_message_limit explicitly");
    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkBool32 enable_message_limit = VK_FALSE;
    uint32_t value = 3;
    const VkLayerSettingEXT settings[2] = {
        {OBJECT_LAYER_NAME, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &enable_message_limit},
        {OBJECT_LAYER_NAME, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &value}};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, settings};

    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());

    // Create an invalid pNext structure to trigger the stateless validation warning
    VkBaseOutStructure bogus_struct{};
    bogus_struct.sType = static_cast<VkStructureType>(0x33333333);
    VkPhysicalDeviceProperties2KHR properties2 = vku::InitStructHelper(&bogus_struct);

    const uint32_t default_value = 10;  // what we set in vkconfig
    const uint32_t count = default_value + 5;
    for (uint32_t i = 0; i < count; i++) {
        m_errorMonitor->SetDesiredError("VUID-VkPhysicalDeviceProperties2-pNext-pNext");
        vk::GetPhysicalDeviceProperties2KHR(Gpu(), &properties2);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeLayerSettings, VuidIdFilterString) {
    TEST_DESCRIPTION("Validate that message id string filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.

    const char *ids[] = {"VUID-VkRenderPassCreateInfo-pNext-01963"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));

    RETURN_IF_SKIP(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

TEST_F(NegativeLayerSettings, VuidFilterHexInt) {
    TEST_DESCRIPTION("Validate that message id hex int filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.

    const char *ids[] = {"0xa19880e3"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));

    RETURN_IF_SKIP(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

TEST_F(NegativeLayerSettings, VuidFilterInt) {
    TEST_DESCRIPTION("Validate that message id decimal int filtering is working");

    AddRequiredExtensions(VK_KHR_MAINTENANCE_2_EXTENSION_NAME);
    // This test would normally produce an unexpected error or two.  Use the message filter instead of
    // the error_monitor's SetUnexpectedError to test the filtering.

    const char *ids[] = {"2711126243"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "message_id_filter", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, ids};
    VkLayerSettingsCreateInfoEXT layer_settings_create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};

    RETURN_IF_SKIP(InitFramework(&layer_settings_create_info));
    RETURN_IF_SKIP(InitState());
    VkAttachmentDescription attach = {0,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                      VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference ref = {0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkSubpassDescription subpass = {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &ref, 0, nullptr, nullptr, nullptr, 0, nullptr};
    VkInputAttachmentAspectReference iaar = {0, 0, VK_IMAGE_ASPECT_METADATA_BIT};
    VkRenderPassInputAttachmentAspectCreateInfo rpiaaci = {VK_STRUCTURE_TYPE_RENDER_PASS_INPUT_ATTACHMENT_ASPECT_CREATE_INFO,
                                                           nullptr, 1, &iaar};
    VkRenderPassCreateInfo rpci = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, &rpiaaci, 0, 1, &attach, 1, &subpass, 0, nullptr};
    m_errorMonitor->SetUnexpectedError("VUID-VkRenderPassCreateInfo2-attachment-02525");
    TestRenderPassCreate(m_errorMonitor, *m_device, rpci, false, "VUID-VkInputAttachmentAspectReference-aspectMask-01964", nullptr);
}

TEST_F(NegativeLayerSettings, DebugAction) {
    const char *action = "VK_DBG_LAYER_ACTION_NOT_A_REAL_THING";
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &action};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_DEBUG_ACTION");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, DebugAction2) {
    const char *actions[2] = {"VK_DBG_LAYER_ACTION_IGNORE,VK_DBG_LAYER_ACTION_CALLBACK"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, actions};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_DEBUG_ACTION");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, DebugAction3) {
    const char *actions[2] = {"VK_DBG_LAYER_ACTION_DEFAULT", "VK_DBG_LAYER_ACTION_NOT_A_REAL_THING"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 2, actions};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_DEBUG_ACTION");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, ReportFlags) {
    const char *report_flag = "fake";
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &report_flag};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_REPORT_FLAGS");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, ReportFlags2) {
    const char *report_flag = "warn,fake,info";
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &report_flag};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_REPORT_FLAGS");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, ReportFlags3) {
    const char *report_flag = "error,warn,info,verbose";
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, &report_flag};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("was not a valid option for VK_LAYER_REPORT_FLAGS");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

#ifndef WIN32
TEST_F(NegativeLayerSettings, LogFilename) {
    const char *path[] = {"/fake/path"};
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "log_filename", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, path};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().SetDesiredWarning("(/fake/path) could not be opened, falling back to stdout instead");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}
#endif

TEST_F(NegativeLayerSettings, NotRealSetting) {
    int32_t enable = 1;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "not_a_real_setting", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &enable};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    Monitor().SetDesiredWarning(
        "The setting not_a_real_setting in VkLayerSettingsCreateInfoEXT was not recognize by the Validation Layers.");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, WrongSettingType) {
    // Actually needs a VK_LAYER_SETTING_TYPE_BOOL32_EXT
    int32_t enable = 1;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "enable_message_limit", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &enable};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    Monitor().SetDesiredWarning(
        " The setting enable_message_limit in VkLayerSettingsCreateInfoEXT was set to type VK_LAYER_SETTING_TYPE_UINT32_EXT but "
        "requires type VK_LAYER_SETTING_TYPE_BOOL32_EXT and the value may be parsed incorrectly.");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, WrongSettingType2) {
    // Actually needs a VK_LAYER_SETTING_TYPE_BOOL32_EXT
    int32_t enable = 1;
    const VkLayerSettingEXT setting = {OBJECT_LAYER_NAME, "thread_safety", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &enable};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &setting};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    Monitor().SetDesiredWarning(
        " The setting thread_safety in VkLayerSettingsCreateInfoEXT was set to type VK_LAYER_SETTING_TYPE_UINT32_EXT but requires "
        "type VK_LAYER_SETTING_TYPE_BOOL32_EXT and the value may be parsed incorrectly.");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}

TEST_F(NegativeLayerSettings, DuplicateSettings) {
    VkBool32 enable = VK_TRUE;
    VkBool32 disable = VK_FALSE;
    const VkLayerSettingEXT settings[2] = {
        {OBJECT_LAYER_NAME, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &enable},
        {OBJECT_LAYER_NAME, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &disable}};
    VkLayerSettingsCreateInfoEXT create_info = {VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 2, settings};
    Monitor().ExpectSuccess(kErrorBit | kWarningBit);
    Monitor().SetDesiredWarning(
        "The setting enable_message_limit in VkLayerSettingsCreateInfoEXT was listed twice and only the first one listed will be "
        "recognized");
    RETURN_IF_SKIP(InitFramework(&create_info));
    RETURN_IF_SKIP(InitState());
    Monitor().VerifyFound();
}