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
#include <vector>
#include <cstring>

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Bool) {
    std::vector<VkBool32> input_values{VK_TRUE, VK_FALSE};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_BOOL32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<VkBool32> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_BOOL32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_EQ(VK_TRUE, values[0]);
    EXPECT_EQ(VK_FALSE, values[1]);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("true", string_values[0]);
    EXPECT_STREQ("false", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Int32) {
    std::vector<std::int32_t> input_values{76, -82};

    std::vector<VkLayerSettingEXT> settings{VkLayerSettingEXT{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT32_EXT,
                                                              static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::int32_t> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(-82, values[1]);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("76", string_values[0]);
    EXPECT_STREQ("-82", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Int64) {
    std::vector<std::int64_t> input_values{76, -82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::int64_t> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_INT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(-82, values[1]);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("76", string_values[0]);
    EXPECT_STREQ("-82", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Uint32) {
    std::vector<std::uint32_t> input_values{76, 82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::uint32_t> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76u, values[0]);
    EXPECT_EQ(82u, values[1]);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("76", string_values[0]);
    EXPECT_STREQ("82", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Uint64) {
    std::vector<std::uint64_t> input_values{76, 82};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::uint64_t> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(82, values[1]);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("76", string_values[0]);
    EXPECT_STREQ("82", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Float) {
    std::vector<float> input_values{76.1f, -82.5f};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT32_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<float> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_TRUE(std::abs(values[0] - 76.1f) <= 0.0001f);
    EXPECT_TRUE(std::abs(values[1] - -82.5f) <= 0.0001f);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_TRUE(std::strstr(string_values[0], "76.") != nullptr);
    EXPECT_TRUE(std::strstr(string_values[1], "-82.5") != nullptr);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Double) {
    std::vector<double> input_values{76.1, -82.5};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT64_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<double> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FLOAT64, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_TRUE(std::abs(values[0] - 76.1) <= 0.0001);
    EXPECT_TRUE(std::abs(values[1] - -82.5) <= 0.0001);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> string_values(input_values.size());

    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_TRUE(std::strstr(string_values[0], "76.") != nullptr);
    EXPECT_TRUE(std::strstr(string_values[1], "-82.5") != nullptr);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_Frameset) {
    const std::size_t frameset_size = (sizeof(VkuFrameset) / sizeof(VkuFrameset::count));

    std::vector<VkuFrameset> input_values{{76, 100, 10}, {1, 100, 1}};

    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT,
                                             static_cast<uint32_t>(input_values.size() * frameset_size), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<VkuFrameset> values(input_values.size());

    uint32_t value_count = 2 * frameset_size;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_UINT32, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(76u, values[0].first);
    EXPECT_EQ(100u, values[0].count);
    EXPECT_EQ(10u, values[0].step);
    EXPECT_EQ(1u, values[1].first);
    EXPECT_EQ(100u, values[1].count);
    EXPECT_EQ(1u, values[1].step);
    EXPECT_EQ(6u, value_count);

    value_count = 0;
    result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FRAMESET_STRING, &value_count, nullptr);
    EXPECT_EQ(2u, value_count);

    std::vector<const char*> string_values(input_values.size());

    value_count = 2;
    result_complete = vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_FRAMESET_STRING, &value_count,
                                               &string_values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_EQ(2u, value_count);
    EXPECT_STREQ("76-100-10", string_values[0]);
    EXPECT_STREQ("1-100-1", string_values[1]);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cast, vkuGetLayerSettingValues_String) {
    std::vector<const char*> input_values{"VALUE_A", "VALUE_B"};
    std::vector<VkLayerSettingEXT> settings{{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT,
                                             static_cast<uint32_t>(input_values.size()), &input_values[0]}};

    VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                            static_cast<uint32_t>(settings.size()), &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<const char*> values(input_values.size());

    uint32_t value_count = 2;
    VkResult result_complete =
        vkuGetLayerSettingValues(layerSettingSet, "my_setting", VKU_LAYER_SETTING_TYPE_STRING, &value_count, &values[0]);
    EXPECT_EQ(VK_SUCCESS, result_complete);
    EXPECT_STREQ("VALUE_A", values[0]);
    EXPECT_STREQ("VALUE_B", values[1]);
    EXPECT_EQ(2u, value_count);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}
