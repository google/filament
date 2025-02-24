// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include <gtest/gtest.h>

#include "vulkan/layer/vk_layer_settings.hpp"
#include <iterator>
#include <vector>

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Bool) {
    const VkBool32 value_data{VK_TRUE};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    bool pValues = true;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(true, pValues);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Bool) {
    const VkBool32 values_data[] = {VK_TRUE, VK_FALSE};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_BOOL32_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<bool> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(true, values[0]);
    EXPECT_EQ(false, values[1]);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Int32) {
    const std::int32_t value_data{76};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::int32_t pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(76, pValues);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Int32) {
    const std::int32_t values_data[] = {76, 82};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT32_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::int32_t> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(82, values[1]);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Uint32) {
    const std::uint32_t value_data{76};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::uint32_t pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(76u, pValues);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Uint32) {
    const std::uint32_t values_data[] = {76, 82};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::uint32_t> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(76u, values[0]);
    EXPECT_EQ(82u, values[1]);
    EXPECT_EQ(2u, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Int64) {
    const std::int64_t value_data{76};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT64_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::int64_t pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(76, pValues);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Int64) {
    const std::int64_t values_data[] = {76, 82};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_INT64_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::int64_t> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(82, values[1]);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Uint64) {
    const std::uint64_t value_data{76};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT64_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::uint64_t pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(76, pValues);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Uint64) {
    const std::uint64_t values_data[] = {76, 82};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT64_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::uint64_t> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(76, values[0]);
    EXPECT_EQ(82, values[1]);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Float) {
    const float value_data{-82.5f};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT32_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    float pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_TRUE(std::abs(pValues - -82.5f) <= 0.0001f);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Float) {
    const float values_data[] = {76.1f, -82.5f};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT32_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<float> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_TRUE(std::abs(values[0] - 76.1f) <= 0.0001f);
    EXPECT_TRUE(std::abs(values[1] - -82.5f) <= 0.0001f);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Double) {
    const double value_data{-82.5};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT64_EXT, 1, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    double pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_TRUE(std::abs(pValues - -82.5) <= 0.0001);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Double) {
    const double values_data[] = {76.1, -82.5};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_FLOAT64_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<double> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_TRUE(std::abs(values[0] - 76.1) <= 0.0001);
    EXPECT_TRUE(std::abs(values[1] - -82.5) <= 0.0001);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_String) {
    const char* value_data[] = {"VALUE_A"};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    std::string pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);
    EXPECT_STREQ("VALUE_A", pValues.c_str());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Strings) {
    const char* values_data[] = {"VALUE_A", "VALUE_B"};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    std::string pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);
    EXPECT_STREQ("VALUE_A,VALUE_B", pValues.c_str());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_String) {
    const char* values_data[] = {"VALUE_A", "VALUE_B"};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<std::string> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);
    EXPECT_STREQ("VALUE_A", values[0].c_str());
    EXPECT_STREQ("VALUE_B", values[1].c_str());
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValue_Frameset) {
    const VkuFrameset value_data{76, 100, 10};

    const VkLayerSettingEXT setting{"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT, 3, &value_data};

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1,
                                                                  &setting};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    VkuFrameset pValues;
    vkuGetLayerSettingValue(layerSettingSet, "my_setting", pValues);

    EXPECT_EQ(76u, pValues.first);
    EXPECT_EQ(100u, pValues.count);
    EXPECT_EQ(10u, pValues.step);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_Frameset) {
    const VkuFrameset values_data[] = {{76, 100, 10}, {1, 100, 1}};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data) * (sizeof(VkuFrameset) / sizeof(VkuFrameset::count)));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_UINT32_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, settings};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<VkuFrameset> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);

    EXPECT_EQ(76u, values[0].first);
    EXPECT_EQ(100u, values[0].count);
    EXPECT_EQ(10u, values[0].step);
    EXPECT_EQ(1u, values[1].first);
    EXPECT_EQ(100u, values[1].count);
    EXPECT_EQ(1u, values[1].step);
    EXPECT_EQ(2, values.size());

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetLayerSettingValues_VkuCustomSTypeInfo) {
    const char* values_data[] = {"0x76", "0X82", "76", "82"};
    const uint32_t value_count = static_cast<uint32_t>(std::size(values_data));

    const VkLayerSettingEXT settings[] = {
        {"VK_LAYER_LUNARG_test", "my_setting", VK_LAYER_SETTING_TYPE_STRING_EXT, value_count, values_data}};
    const uint32_t settings_size = static_cast<uint32_t>(std::size(settings));

    const VkLayerSettingsCreateInfoEXT layer_settings_create_info{VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr,
                                                                  settings_size, &settings[0]};

    VkuLayerSettingSet layerSettingSet = VK_NULL_HANDLE;
    vkuCreateLayerSettingSet("VK_LAYER_LUNARG_test", &layer_settings_create_info, nullptr, nullptr, &layerSettingSet);

    EXPECT_TRUE(vkuHasLayerSetting(layerSettingSet, "my_setting"));

    std::vector<VkuCustomSTypeInfo> values;
    vkuGetLayerSettingValues(layerSettingSet, "my_setting", values);
    EXPECT_EQ(0x76u, values[0].first);
    EXPECT_EQ(0x82u, values[0].second);
    EXPECT_EQ(76u, values[1].first);
    EXPECT_EQ(82u, values[1].second);

    vkuDestroyLayerSettingSet(layerSettingSet, nullptr);
}

TEST(test_layer_setting_cpp, vkuGetUnknownSettings) {
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

    const char* setting_names[] = {"int32_value", "int64_value", "uint32_value", "uint64_value", "float_value", "double_value"};
    const std::uint32_t setting_name_count = static_cast<std::uint32_t>(std::size(setting_names));

    std::vector<const char*> unknown_settings;
    vkuGetUnknownSettings(&layer_settings_create_info, setting_name_count, setting_names, unknown_settings);
    EXPECT_EQ(2, unknown_settings.size());

    EXPECT_STREQ("bool_value", unknown_settings[0]);
    EXPECT_STREQ("frameset_value", unknown_settings[1]);
}
