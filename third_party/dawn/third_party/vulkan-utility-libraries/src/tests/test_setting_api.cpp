// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include <gtest/gtest.h>

#include "vulkan/layer/vk_layer_settings.h"

TEST(test_layer_setting_api, vkuHasLayerSetting_NotFound) {
    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;

    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", nullptr, nullptr, nullptr, &layerSettingSet);

    EXPECT_FALSE(vkuHasLayerSetting(layerSettingSet, "setting_key"));

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuHasLayerSetting_Found_SingleCreateInfo) {
    std::int32_t pValues = 76;

    VkLayerSettingEXT my_setting;
    my_setting.pLayerName = "VK_LAYER_LUNARG_test";
    my_setting.pSettingName = "my_setting";
    my_setting.type = VK_LAYER_SETTING_TYPE_INT32_EXT;
    my_setting.valueCount = 1;
    my_setting.pValues = &pValues;

    std::vector<VkLayerSettingEXT> settings;
    settings.push_back(my_setting);

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuHasLayerSetting_Found_MultipleCreateInfo) {
    const std::int32_t valueA = 76;
    const std::int32_t valueC1 = 77;

    const VkLayerSettingEXT settingsA[] = {{"VK_LAYER_LUNARG_test", "my_settingA", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &valueA},
                                           {"VK_LAYER_LUNARG_test", "my_settingC", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &valueC1}};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_infoA{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                   static_cast<uint32_t>(std::size(settingsA)), settingsA};

    const std::int32_t valueB = 82;
    const std::int32_t valueC2 = 83;  // Override valueC1 value!

    const VkLayerSettingEXT settingsB[] = {{"VK_LAYER_LUNARG_test", "my_settingB", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &valueB},
                                           {"VK_LAYER_LUNARG_test", "my_settingC", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &valueC2}};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_infoB{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                                                                   &layer_settings_create_infoA,
                                                                   static_cast<uint32_t>(std::size(settingsB)), settingsB};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_infoB, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_settingA"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_settingB"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_settingC"));

    std::int32_t resulatA = 0;
    std::int32_t resulatB = 0;
    std::int32_t resulatC = 0;

    uint32_t value_count = 1;
    vkuGetLayerSettingValues(layerSettingSet, "my_settingA", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &resulatA);
    EXPECT_EQ(76, resulatA);
    vkuGetLayerSettingValues(layerSettingSet, "my_settingB", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &resulatB);
    EXPECT_EQ(82, resulatB);
    vkuGetLayerSettingValues(layerSettingSet, "my_settingC", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &resulatC);
    EXPECT_EQ(83, resulatC);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuHasLayerSetting) {
    // The expected application code side:
    std::vector<VkLayerSettingEXT> settings;

    VkBool32 value_bool = VK_TRUE;
    VkLayerSettingEXT setting_bool_value{};
    setting_bool_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_bool_value.pSettingName = "bool_value";
    setting_bool_value.type = VK_LAYER_SETTING_TYPE_BOOL32_EXT;
    setting_bool_value.pValues = &value_bool;
    setting_bool_value.valueCount = 1;
    settings.push_back(setting_bool_value);

    std::int32_t value_int32 = 76;
    VkLayerSettingEXT setting_int32_value{};
    setting_int32_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_int32_value.pSettingName = "int32_value";
    setting_int32_value.type = VK_LAYER_SETTING_TYPE_INT32_EXT;
    setting_int32_value.pValues = &value_int32;
    setting_int32_value.valueCount = 1;
    settings.push_back(setting_int32_value);

    std::int64_t value_int64 = static_cast<int64_t>(1) << static_cast<int64_t>(40);
    VkLayerSettingEXT setting_int64_value{};
    setting_int64_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_int64_value.pSettingName = "int64_value";
    setting_int64_value.type = VK_LAYER_SETTING_TYPE_INT64_EXT;
    setting_int64_value.pValues = &value_int64;
    setting_int64_value.valueCount = 1;
    settings.push_back(setting_int64_value);

    std::uint32_t value_uint32 = 76u;
    VkLayerSettingEXT setting_uint32_value{};
    setting_uint32_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_uint32_value.pSettingName = "uint32_value";
    setting_uint32_value.type = VK_LAYER_SETTING_TYPE_UINT32_EXT;
    setting_uint32_value.pValues = &value_uint32;
    setting_uint32_value.valueCount = 1;
    settings.push_back(setting_uint32_value);

    std::uint64_t value_uint64 = static_cast<uint64_t>(1) << static_cast<uint64_t>(40);
    VkLayerSettingEXT setting_uint64_value{};
    setting_uint64_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_uint64_value.pSettingName = "uint64_value";
    setting_uint64_value.type = VK_LAYER_SETTING_TYPE_UINT64_EXT;
    setting_uint64_value.pValues = &value_uint64;
    setting_uint64_value.valueCount = 1;
    settings.push_back(setting_uint64_value);

    float value_float = 76.1f;
    VkLayerSettingEXT setting_float_value{};
    setting_float_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_float_value.pSettingName = "float_value";
    setting_float_value.type = VK_LAYER_SETTING_TYPE_FLOAT32_EXT;
    setting_float_value.pValues = &value_float;
    setting_float_value.valueCount = 1;
    settings.push_back(setting_float_value);

    double value_double = 76.1;
    VkLayerSettingEXT setting_double_value{};
    setting_double_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_double_value.pSettingName = "double_value";
    setting_double_value.type = VK_LAYER_SETTING_TYPE_FLOAT64_EXT;
    setting_double_value.pValues = &value_double;
    setting_double_value.valueCount = 1;
    settings.push_back(setting_double_value);

    VkuFrameset value_frameset{76, 100, 10};
    VkLayerSettingEXT setting_frameset_value{};
    setting_frameset_value.pLayerName = "VK_LAYER_LUNARG_test";
    setting_frameset_value.pSettingName = "frameset_value";
    setting_frameset_value.type = VK_LAYER_SETTING_TYPE_UINT32_EXT;
    setting_frameset_value.pValues = &value_frameset;
    setting_frameset_value.valueCount = sizeof(VkuFrameset) / sizeof(VkuFrameset::count);
    settings.push_back(setting_frameset_value);

    VkLayerSettingsCreateInfoEXT layer_settings_create_info;
    layer_settings_create_info.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
    layer_settings_create_info.pNext = nullptr;
    layer_settings_create_info.settingCount = static_cast<uint32_t>(settings.size());
    layer_settings_create_info.pSettings = &settings[0];

    // The expected layer code side:
    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_FALSE(vkuHasLayerSetting(layerSettingSet, "setting0"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "bool_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "int32_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "int64_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "uint32_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "uint64_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "float_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "double_value"));
    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "frameset_value"));

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Bool) {
    std::vector<VkBool32> input_values{VK_TRUE, VK_FALSE};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_BOOL32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<VkBool32> values(static_cast<uint32_t>(value_count));

    value_count = 1u;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(VK_TRUE, values[0]);
    EXPECT_EQ(VK_FALSE, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2u;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(VK_TRUE, values[0]);
    EXPECT_EQ(VK_FALSE, values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Int32) {
    std::vector<std::int32_t> input_values{76, -82};

    std::vector<VkLayerSettingEXT> settings{VkLayerSettingEXT{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT32_EXT,
                                                              static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 2;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT32, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<std::int32_t> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(0, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(-82, values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Int64) {
    std::vector<std::int64_t> input_values{76, -82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 2;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT64, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<std::int64_t> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT64, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(0, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(-82, values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Uint32) {
    std::vector<std::uint32_t> input_values{76, 82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT32, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<std::uint32_t> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(76u, values[0]);
    EXPECT_EQ(0u, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76u, values[0]);
    EXPECT_EQ(82u, values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Uint64) {
    std::vector<std::uint64_t> input_values{76, 82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT64, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<std::uint64_t> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT64, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(0, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(82, values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Float) {
    std::vector<float> input_values{76.1f, -82.5f};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<float> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_TRUE(std::abs(values[0] - 76.1f) <= 0.0001f);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_TRUE(std::abs(values[0] - 76.1f) <= 0.0001f);
    EXPECT_TRUE(std::abs(values[1] - -82.5f) <= 0.0001f);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Double) {
    std::vector<double> input_values{76.1, -82.5};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<double> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_TRUE(std::abs(values[0] - 76.1) <= 0.0001);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_TRUE(std::abs(values[0] - 76.1) <= 0.0001);
    EXPECT_TRUE(std::abs(values[1] - -82.5) <= 0.0001);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_Frameset) {
    const VkuFrameset input_values[] = {{76, 100, 10}, {1, 100, 1}};
    const std::uint32_t frameset_count =
        static_cast<std::uint32_t>(std::size(input_values) * (sizeof(VkuFrameset) / sizeof(VkuFrameset::count)));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT, frameset_count, input_values}};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  static_cast<std::uint32_t>(std::size(settings)), settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FRAMESET, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<VkuFrameset> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FRAMESET, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_EQ(76u, values[0].first);
    EXPECT_EQ(100u, values[0].count);
    EXPECT_EQ(10u, values[0].step);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FRAMESET, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76u, values[0].first);
    EXPECT_EQ(100u, values[0].count);
    EXPECT_EQ(10u, values[0].step);
    EXPECT_EQ(1u, values[1].first);
    EXPECT_EQ(100u, values[1].count);
    EXPECT_EQ(1u, values[1].step);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_api, vkuGetLayerSettingValues_String) {
    std::vector<const char*> input_values{"VALUE_A", "VALUE_B"};
    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    uint32_t value_count = 0;
    VkResult result_count =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, nullptr);
    EXPECT_EQ(VK_SUCCESS, result_count);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> values(static_cast<uint32_t>(value_count));

    value_count = 1;
    VkResult result_incomplete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &values[0]);
    EXPECT_EQ(VK_INCOMPLETE, result_incomplete);
    EXPECT_STREQ("VALUE_A", values[0]);
    EXPECT_STREQ(nullptr, values[1]);
    EXPECT_EQ(1u, value_count);

    value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_STREQ("VALUE_A", values[0]);
    EXPECT_STREQ("VALUE_B", values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}
