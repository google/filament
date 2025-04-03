/*
 * Copyright (c) 2021-2023 The Khronos Group Inc.
 * Copyright (c) 2021-2023 Valve Corporation
 * Copyright (c) 2021-2023 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and/or associated documentation files (the "Materials"), to
 * deal in the Materials without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Materials, and to permit persons to whom the Materials are
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE
 * USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include "test_environment.h"

void CheckLogForLayerString(FrameworkEnvironment& env, const char* implicit_layer_name, bool check_for_enable) {
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        if (check_for_enable) {
            ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
        } else {
            ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
        }
    }
    env.debug_log.clear();
}

const char* lunarg_meta_layer_name = "VK_LAYER_LUNARG_override";

TEST(ImplicitLayers, WithEnableAndDisableEnvVar) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    EnvVarWrapper enable_env_var{"ENABLE_ME"};
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_env_var.get())
                                                         .set_enable_environment(enable_env_var.get())),
                           "implicit_test_layer.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // didn't set enable env-var, layer should not load
    CheckLogForLayerString(env, implicit_layer_name, false);

    // set enable env-var to 0, no layer should be found
    enable_env_var.set_new_value("0");
    CheckLogForLayerString(env, implicit_layer_name, false);

    // set enable env-var, layer should load
    enable_env_var.set_new_value("1");
    CheckLogForLayerString(env, implicit_layer_name, true);

    // remove enable env var, so we can check what happens when only disable is present
    enable_env_var.remove_value();

    // set disable env-var to 0, layer should not load
    disable_env_var.set_new_value("0");
    CheckLogForLayerString(env, implicit_layer_name, false);

    // set disable env-var to 1, layer should not load
    disable_env_var.set_new_value("1");
    CheckLogForLayerString(env, implicit_layer_name, false);

    // set both enable and disable env-var, layer should not load
    enable_env_var.set_new_value("1");
    disable_env_var.set_new_value("1");
    CheckLogForLayerString(env, implicit_layer_name, false);
}

TEST(ImplicitLayers, OnlyDisableEnvVar) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_env_var.get())),
                           "implicit_test_layer.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // don't set disable env-var, layer should load
    CheckLogForLayerString(env, implicit_layer_name, true);

    // set disable env-var to 0, layer should load
    disable_env_var.set_new_value("0");
    CheckLogForLayerString(env, implicit_layer_name, false);

    // set disable env-var to 1, layer should not load
    disable_env_var.set_new_value("1");
    CheckLogForLayerString(env, implicit_layer_name, false);

    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(implicit_layer_name);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
    }
}

TEST(ImplicitLayers, PreInstanceEnumInstLayerProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment(disable_env_var.get())
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))),
        "implicit_test_layer.json");

    uint32_t layer_props = 43;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(layer_props);

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, layer_props);

    // set disable env-var to 1, layer should not load
    disable_env_var.set_new_value("1");

    count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_NE(count, 0U);
    ASSERT_NE(count, layer_props);
}

TEST(ImplicitLayers, PreInstanceEnumInstExtProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment(disable_env_var.get())
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))),
        "implicit_test_layer.json");

    uint32_t ext_props = 52;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_extension_props(ext_props);

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_EQ(count, ext_props);

    // set disable env-var to 1, layer should not load
    disable_env_var.set_new_value("1");

    count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_NE(count, 0U);
    ASSERT_NE(count, ext_props);
}

TEST(ImplicitLayers, PreInstanceVersion) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3));

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(implicit_layer_name)
                                   .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3))
                                   .set_disable_environment(disable_env_var.get())
                                   .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                                                  .set_vk_func("vkEnumerateInstanceVersion")
                                                                  .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
                           "implicit_test_layer.json");

    uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
    auto& layer = env.get_test_layer(0);
    layer.set_reported_instance_version(layer_version);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, layer_version);

    // set disable env-var to 1, layer should not load
    disable_env_var.set_new_value("1");

    version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_NE(version, 0U);
    ASSERT_NE(version, layer_version);
}

// Run with a pre-Negotiate function version of the layer so that it has to query vkCreateInstance using the
// renamed vkGetInstanceProcAddr function which returns one that intentionally fails.  Then disable the
// layer and verify it works.  The non-override version of vkCreateInstance in the layer also works (and is
// tested through behavior above).
TEST(ImplicitLayers, OverrideGetInstanceProcAddr) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    EnvVarWrapper disable_env_var{"DISABLE_ME"};

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 0, 0}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(implicit_layer_name)
                                   .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_1)
                                   .set_disable_environment(disable_env_var.get())
                                   .add_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                                     .set_vk_func("vkGetInstanceProcAddr")
                                                     .set_override_name("test_override_vkGetInstanceProcAddr"))),
                           "implicit_test_layer.json");

    {
        InstWrapper inst1{env.vulkan_functions};
        inst1.CheckCreate(VK_ERROR_INVALID_SHADER_NV);
    }

    {
        // set disable env-var to 1, layer should not load
        disable_env_var.set_new_value("1");
        InstWrapper inst2{env.vulkan_functions};
        inst2.CheckCreate();
    }
}

// Force enable with filter env var
TEST(ImplicitLayers, EnableWithFilter) {
    FrameworkEnvironment env;

    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* implicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* implicit_json_name_1 = "First_layer.json";
    const char* disable_layer_name_1 = "DISABLE_FIRST";
    const char* enable_layer_name_1 = "ENABLE_FIRST";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_enable_environment(enable_layer_name_1)
                                                         .set_disable_environment(disable_layer_name_1)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_1);

    const char* implicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* implicit_json_name_2 = "Second_layer.json";
    const char* disable_layer_name_2 = "DISABLE_SECOND";
    const char* enable_layer_name_2 = "ENABLE_SECOND";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_enable_environment(enable_layer_name_2)
                                                         .set_disable_environment(disable_layer_name_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_2);

    const char* implicit_layer_name_3 = "VK_LAYER_LUNARG_Second_test_layer";
    const char* implicit_json_name_3 = "Second_test_layer.json";
    const char* disable_layer_name_3 = "DISABLE_THIRD";
    const char* enable_layer_name_3 = "ENABLE_THIRD";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_enable_environment(enable_layer_name_3)
                                                         .set_disable_environment(disable_layer_name_3)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_3);

    EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE"};
    EnvVarWrapper layer_1_enable_env_var{enable_layer_name_1};

    // First, test an instance/device without the layer forced on.
    InstWrapper inst1{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    inst1.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Now force on one layer with its full name
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value(implicit_layer_name_1);

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    inst2.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match prefix
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("VK_LAYER_LUNARG_*");

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    inst3.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match suffix
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*Second_layer");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    inst4.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match substring
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    inst5.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with star '*'
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*");

    InstWrapper inst6{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    inst6.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("~all~");

    InstWrapper inst7{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst7.create_info, env.debug_log);
    inst7.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match substring, but enable the other layer manually
    // ------------------------------------------
    env.debug_log.clear();
    layer_1_enable_env_var.set_new_value("1");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst8{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst8.create_info, env.debug_log);
    inst8.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Force disabled with new filter env var
TEST(ImplicitLayers, DisableWithFilter) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* implicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* implicit_json_name_1 = "First_layer.json";
    const char* disable_layer_name_1 = "DISABLE_FIRST";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_1)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_1);

    const char* implicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* implicit_json_name_2 = "Second_layer.json";
    const char* disable_layer_name_2 = "DISABLE_SECOND";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_2);

    const char* implicit_layer_name_3 = "VK_LAYER_LUNARG_Second_test_layer";
    const char* implicit_json_name_3 = "Second_test_layer.json";
    const char* disable_layer_name_3 = "DISABLE_THIRD";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_3)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_3);

    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};

    // First, test an instance/device
    InstWrapper inst1{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    inst1.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Now force off one layer with its full name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value(implicit_layer_name_1);

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    inst2.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match prefix
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("VK_LAYER_LUNARG_*");

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    inst3.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match suffix
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second_layer");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    inst4.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match substring
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    inst5.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with star '*'
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*");

    InstWrapper inst6{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    inst6.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~all~");

    InstWrapper inst7{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst7.create_info, env.debug_log);
    inst7.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Force disabled with new filter env var
TEST(ImplicitLayers, DisableWithFilterWhenLayersEnableEnvVarIsActive) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* implicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* implicit_json_name_1 = "First_layer.json";
    const char* disable_layer_name_1 = "DISABLE_FIRST";
    const char* enable_layer_name_1 = "ENABLE_FIRST";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_1)
                                                         .set_enable_environment(enable_layer_name_1)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_1);

    const char* implicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* implicit_json_name_2 = "Second_layer.json";
    const char* disable_layer_name_2 = "DISABLE_SECOND";
    const char* enable_layer_name_2 = "ENABLE_SECOND";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_2)
                                                         .set_enable_environment(enable_layer_name_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_2);

    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};
    EnvVarWrapper layer_1_enable_env_var{enable_layer_name_1};
    EnvVarWrapper layer_2_enable_env_var{enable_layer_name_2};

    // First, test an instance/device
    InstWrapper inst1{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    inst1.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));

    // Set the layers enable env-var
    // ------------------------------------------
    env.debug_log.clear();
    layer_1_enable_env_var.set_new_value("1");
    layer_2_enable_env_var.set_new_value("1");

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    inst2.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));

    // Now force off one layer with its full name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value(implicit_layer_name_1);

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    inst3.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));

    // Now force off both layers
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~implicit~");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    inst4.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
}

// Test interaction between both the enable and disable filter environment variables.  The enable should always
// override the disable.
TEST(ImplicitLayers, EnableAndDisableWithFilter) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* implicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* implicit_json_name_1 = "First_layer.json";
    const char* disable_layer_name_1 = "DISABLE_FIRST";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_1)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_1);

    const char* implicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* implicit_json_name_2 = "Second_layer.json";
    const char* disable_layer_name_2 = "DISABLE_SECOND";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_2);

    const char* implicit_layer_name_3 = "VK_LAYER_LUNARG_Second_test_layer";
    const char* implicit_json_name_3 = "Second_test_layer.json";
    const char* disable_layer_name_3 = "DISABLE_THIRD";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_layer_name_3)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           implicit_json_name_3);

    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};
    EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE"};

    // Disable 2 but enable 1
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second*");
    layers_enable_env_var.set_new_value("*test_layer");

    InstWrapper inst1{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    inst1.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable all but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    inst2.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable all but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~all~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    inst3.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable implicit but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~implicit~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    inst4.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable explicit but enable 2 (should still be everything)
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~explicit~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    inst5.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable implicit but enable all
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~implicit~");
    layers_enable_env_var.set_new_value("*");

    InstWrapper inst6{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    inst6.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", implicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", implicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(implicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Add 2 implicit layers with the same layer name and expect only one to be loaded.
// Expect the second layer to be found first, because it'll be in a path that is searched first.
TEST(ImplicitLayers, DuplicateLayers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* same_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("if_you_can")),
                                            "regular_layer_1.json"));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");
    layer1.set_make_spurious_log_in_create_instance("actually_layer_1");

    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("if_you_can")),
                                            "regular_layer_1.json")
                               // use override folder as just a folder and manually add it to the implicit layer search paths
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    layer2.set_make_spurious_log_in_create_instance("actually_layer_2");
#if defined(WIN32)
    env.platform_shim->add_manifest(ManifestCategory::implicit_layer, env.get_folder(ManifestLocation::override_layer).location());
#elif COMMON_UNIX_PLATFORMS
    env.platform_shim->redirect_path(std::filesystem::path(USER_LOCAL_SHARE_DIR "/vulkan/implicit_layer.d"),
                                     env.get_folder(ManifestLocation::override_layer).location());
#endif

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();

    auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
    ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
    ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
}

TEST(ImplicitLayers, VkImplicitLayerPathEnvVar) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layer loads successfully when setting VK_IMPLICIT_LAYER_PATH to a full filepath
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name_1)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Yikes")),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(false));

    InstWrapper inst(env.vulkan_functions);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name_1));
}

TEST(ImplicitLayers, VkImplicitLayerPathEnvVarContainsMultipleFilePaths) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layers load successfully when setting VK_IMPLICIT_LAYER_PATH to multiple full filepaths
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name_1)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Yikes")),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(false));

    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name_2)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Yikes")),
                                            "regular_layer_2.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(false));

    InstWrapper inst(env.vulkan_functions);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    EXPECT_TRUE(check_permutation({regular_layer_name_1, regular_layer_name_2}, layer_props));
}

TEST(ImplicitLayers, VkImplicitLayerPathEnvVarIsDirectory) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layers load successfully when setting VK_IMPLICIT_LAYER_PATH to a directory
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name_1)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Yikes")),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name_2)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Yikes")),
                                            "regular_layer_2.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    InstWrapper inst(env.vulkan_functions);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    EXPECT_TRUE(check_permutation({regular_layer_name_1, regular_layer_name_2}, layer_props));
}

// Test to make sure order layers are found in VK_IMPLICIT_LAYER_PATH is what decides which layer is loaded
TEST(ImplicitLayers, DuplicateLayersInVkImplicitLayerPath) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Boo!")),
                                            "layer.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");

    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Ah!")),
                                            "layer.json")
                               // putting it in a separate folder then manually adding the folder to VK_IMPLICIT_LAYER_PATH
                               .set_discovery_type(ManifestDiscoveryType::override_folder)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    env.env_var_vk_implicit_layer_paths.add_to_list(env.get_folder(ManifestLocation::override_layer).location().string());

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer_name, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));

    EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
    inst_layers_env_var.add_to_list(layer_name);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Expect the first layer added to be found
    auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layer_name, enabled_layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[0].description));
}

TEST(ImplicitLayers, DuplicateLayersInVK_ADD_IMPLICIT_LAYER_PATH) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* same_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Red")),
                                            "regular_layer_1.json")
                               // use override folder as just a folder and manually set the VK_ADD_IMPLICIT_LAYER_PATH env-var to it
                               .set_discovery_type(ManifestDiscoveryType::override_folder)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");
    layer1.set_make_spurious_log_in_create_instance("actually_layer_1");
    env.add_env_var_vk_implicit_layer_paths.add_to_list(env.get_folder(ManifestLocation::override_layer).location());

    env.add_implicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Blue")),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::add_env_var)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    layer2.set_make_spurious_log_in_create_instance("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
    ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
    ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
}

// Meta layer which contains component layers that do not exist.
TEST(MetaLayers, InvalidComponentLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* invalid_layer_name_1 = "VK_LAYER_InvalidLayer1";
    const char* invalid_layer_name_2 = "VK_LAYER_InvalidLayer2";
    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(meta_layer_name)
                                   .add_component_layers({invalid_layer_name_1, invalid_layer_name_2})
                                   .set_disable_environment("NotGonnaWork")
                                   .add_instance_extension({"NeverGonnaGiveYouUp"})
                                   .add_device_extension({"NeverGonnaLetYouDown"})),
                           "meta_test_layer.json");

    const char* regular_layer_name = "TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    // should find 1, the 'regular' layer
    auto layer_props = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(meta_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_TRUE(env.debug_log.find(std::string("verify_meta_layer_component_layers: Meta-layer ") + meta_layer_name +
                                   " can't find component layer " + invalid_layer_name_1 + " at index 0.  Skipping this layer."));
}

// Meta layer that is an explicit layer
TEST(MetaLayers, ExplicitMetaLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(meta_layer_name).add_component_layers({regular_layer_name})),
        "meta_test_layer.json");

    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    {  // global functions
       // should find 1, the 'regular' layer
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, meta_layer_name}, layer_props));

        auto extensions = env.GetInstanceExtensions(4);
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
        EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    }
    {  // don't enable the layer, shouldn't find any layers when calling vkEnumerateDeviceLayerProperties
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(meta_layer_name);
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, meta_layer_name}, layer_props));
    }
}

// Meta layer which adds itself in its list of component layers
TEST(MetaLayers, MetaLayerNameInComponentLayers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(meta_layer_name)
                                   .add_component_layers({meta_layer_name, regular_layer_name})
                                   .set_disable_environment("NotGonnaWork")
                                   .add_instance_extension({"NeverGonnaGiveYouUp"})
                                   .add_device_extension({"NeverGonnaLetYouDown"})),
                           "meta_test_layer.json");

    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    // should find 1, the 'regular' layer
    auto layer_props = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(meta_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_TRUE(env.debug_log.find(std::string("verify_meta_layer_component_layers: Meta-layer ") + meta_layer_name +
                                   " lists itself in its component layer " + "list at index 0.  Skipping this layer."));
}

// Meta layer which adds another meta layer as a component layer
TEST(MetaLayers, MetaLayerWhichAddsMetaLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* meta_meta_layer_name = "VK_LAYER_MetaMetaTestLayer";
    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(meta_layer_name).add_component_layers({regular_layer_name})),
        "meta_test_layer.json");
    env.add_explicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(meta_meta_layer_name)
                                   .add_component_layers({meta_layer_name, regular_layer_name})),
                           "meta_meta_test_layer.json");

    auto layer_props = env.GetLayerProperties(3);
    EXPECT_TRUE(check_permutation({regular_layer_name, meta_layer_name, meta_meta_layer_name}, layer_props));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(meta_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(std::string("verify_meta_layer_component_layers: Adding meta-layer ") + meta_meta_layer_name +
                                   " which also contains meta-layer " + meta_layer_name));
}

TEST(MetaLayers, InstanceExtensionInComponentLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* regular_layer_name = "VK_LAYER_TestLayer";
    const char* instance_ext_name = "VK_EXT_headless_surface";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .add_instance_extension({instance_ext_name})),
                           "regular_test_layer.json");
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(meta_layer_name).add_component_layers({regular_layer_name})),
        "meta_test_layer.json");

    auto extensions = env.GetInstanceExtensions(1, meta_layer_name);
    EXPECT_TRUE(string_eq(extensions[0].extensionName, instance_ext_name));
}

TEST(MetaLayers, DeviceExtensionInComponentLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* meta_layer_name = "VK_LAYER_MetaTestLayer";
    const char* regular_layer_name = "VK_LAYER_TestLayer";
    const char* device_ext_name = "VK_EXT_fake_dev_ext";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .add_device_extension({device_ext_name})),
                           "regular_test_layer.json");
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(meta_layer_name).add_component_layers({regular_layer_name})),
        "meta_test_layer.json");

    ASSERT_NO_FATAL_FAILURE(env.GetInstanceExtensions(0, meta_layer_name));

    {  // layer is not enabled
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Meta-layer ") + meta_layer_name + " component layer " + regular_layer_name +
                                       " adding device extension " + device_ext_name));

        auto extensions = inst.EnumerateLayerDeviceExtensions(inst.GetPhysDev(), meta_layer_name, 1);
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, device_ext_name));
    }
    {  // layer is enabled
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(meta_layer_name);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Meta-layer ") + meta_layer_name + " component layer " + regular_layer_name +
                                       " adding device extension " + device_ext_name));

        auto extensions = inst.EnumerateLayerDeviceExtensions(inst.GetPhysDev(), meta_layer_name, 1);
        EXPECT_TRUE(string_eq(extensions.at(0).extensionName, device_ext_name));
    }
}

// Override meta layer missing disable environment variable still enables the layer
TEST(OverrideMetaLayer, InvalidDisableEnvironment) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                         .add_device_extension({"NeverGonnaLetYouDown"})),
                           "regular_test_layer.json");

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layers({regular_layer_name})),
        "meta_test_layer.json");

    auto layer_props = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
}

// Override meta layer whose version is less than the api version of the instance
TEST(OverrideMetaLayer, OlderVersionThanInstance) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                         .add_device_extension({"NeverGonnaLetYouDown"})),
                           "regular_test_layer.json");

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_component_layers({regular_layer_name})),
        "meta_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    {  // 1.1 instance
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.api_version = VK_API_VERSION_1_1;
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
        ASSERT_TRUE(string_eq(layer_props[1].layerName, lunarg_meta_layer_name));
    }
    {  // 1.3 instance

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.api_version = VK_API_VERSION_1_3;
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);

        ASSERT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
        ASSERT_TRUE(string_eq(layer_props[1].layerName, lunarg_meta_layer_name));
    }
}

TEST(OverrideMetaLayer, OlderMetaLayerWithNewerInstanceVersion) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(regular_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
        "regular_test_layer.json");

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layers({regular_layer_name})
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    {
        // 1.1 instance
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
        ASSERT_TRUE(string_eq(layer_props[1].layerName, lunarg_meta_layer_name));
    }

    {
        // 1.3 instance
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 3, 0);
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
        ASSERT_TRUE(string_eq(layer_props[1].layerName, lunarg_meta_layer_name));
    }
}

TEST(OverrideMetaLayer, NewerComponentLayerInMetaLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(regular_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_API_VERSION_1_2)),
        "regular_test_layer.json");

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layers({regular_layer_name})
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");

    {  // global functions
        auto layer_props = env.GetLayerProperties(2);
        // Expect the explicit layer to still be found
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    {
        // 1.1 instance
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();
        // Newer component is allowed now
        EXPECT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    env.debug_log.clear();

    {
        // 1.3 instance
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.set_api_version(1, 3, 0);
        inst.CheckCreate();
        // Newer component is allowed now
        EXPECT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);

        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
}

TEST(OverrideMetaLayer, OlderComponentLayerInMetaLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(regular_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
        "regular_test_layer.json");

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layers({regular_layer_name})
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
    }
    {
        // 1.1 instance
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();
        EXPECT_TRUE(
            env.debug_log.find("verify_meta_layer_component_layers: Meta-layer uses API version 1.1, but component layer 0 has API "
                               "version 1.0 that is lower.  Skipping this layer."));
        env.debug_log.clear();
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }

    {
        // 1.2 instance
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.set_api_version(1, 2, 0);
        inst.CheckCreate();
        ASSERT_TRUE(
            env.debug_log.find("verify_meta_layer_component_layers: Meta-layer uses API version 1.1, but component layer 0 has API "
                               "version 1.0 that is lower.  Skipping this layer."));
        env.debug_log.clear();
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

TEST(OverrideMetaLayer, ApplicationEnabledLayerInBlacklist) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* automatic_regular_layer_name = "VK_LAYER_TestLayer_1";
    const char* manual_regular_layer_name = "VK_LAYER_TestLayer_2";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(automatic_regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                           "regular_test_layer_1.json");
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(manual_regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                           "regular_test_layer_2.json");
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(automatic_regular_layer_name)
                                                                         .add_blacklisted_layer(manual_regular_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");
    {  // Check that enumerating the layers returns only the non-blacklisted layers + override layer
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(check_permutation({automatic_regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    {
        // enable the layer in the blacklist
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(manual_regular_layer_name);
        inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
        ASSERT_TRUE(env.debug_log.find(std::string("loader_remove_layers_in_blacklist: Override layer is active and layer ") +
                                       manual_regular_layer_name +
                                       " is in the blacklist inside of it. Removing that layer from current layer list."));
        env.debug_log.clear();
    }
    {  // dont enable the layer in the blacklist
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("loader_remove_layers_in_blacklist: Override layer is active and layer ") +
                                       manual_regular_layer_name +
                                       " is in the blacklist inside of it. Removing that layer from current layer list."));
        env.debug_log.clear();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(check_permutation({automatic_regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
}

TEST(OverrideMetaLayer, BasicOverridePaths) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* regular_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(regular_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(regular_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_override_path(override_layer_folder.location())),
        "meta_test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
    env.layers.clear();
}

TEST(OverrideMetaLayer, BasicOverridePathsIgnoreOtherLayers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(regular_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
        "regular_test_layer.json");

    const char* special_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(special_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(special_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_override_path(override_layer_folder.location())),
        "meta_test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.create_info.add_layer(regular_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
    env.layers.clear();
}

TEST(OverrideMetaLayer, OverridePathsInteractionWithVK_LAYER_PATH) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // add explicit layer to VK_LAYER_PATH folder
    const char* env_var_layer_name = "VK_LAYER_env_var_set_path";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(env_var_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
        "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    // add layer to regular explicit layer folder
    const char* regular_layer_name = "VK_LAYER_regular_layer_path";
    env.add_explicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_layer_name)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                                            "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(lunarg_meta_layer_name)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                   .add_component_layer(regular_layer_name)
                                   .set_disable_environment("DisableMeIfYouCan")
                                   .add_override_path(env.get_folder(ManifestLocation::override_layer).location())),
                           "meta_test_layer.json");

    auto meta_layer_path = env.get_folder(ManifestLocation::override_layer).location();

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    inst.create_info.add_layer(env_var_layer_name);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + env_var_layer_name));
    ASSERT_TRUE(env.debug_log.find(
        std::string("Ignoring VK_LAYER_PATH. The Override layer is active and has override paths set, which takes priority. "
                    "VK_LAYER_PATH is set to ") +
        env.env_var_vk_layer_paths.value()));
    ASSERT_TRUE(env.debug_log.find("Override layer has override paths set to " + meta_layer_path.string()));

    env.layers.clear();
}

// Make sure that implicit layers not in the override paths aren't found by mistake
TEST(OverrideMetaLayer, OverridePathsEnableImplicitLayerInDefaultPaths) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* implicit_layer_name = "VK_LAYER_ImplicitLayer";
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(implicit_layer_name)
                                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
        "implicit_test_layer.json");

    const char* regular_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(regular_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(lunarg_meta_layer_name)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                   .add_component_layers({regular_layer_name, implicit_layer_name})
                                   .set_disable_environment("DisableMeIfYouCan")
                                   .add_override_path(override_layer_folder.location())),
                           "meta_test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.create_info.set_api_version(1, 1, 0);
    inst.CheckCreate();
    ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
    ASSERT_TRUE(
        env.debug_log.find("Removing meta-layer VK_LAYER_LUNARG_override from instance layer list since it appears invalid."));
    env.layers.clear();
}

TEST(OverrideMetaLayer, ManifestFileFormatVersionTooOld) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* regular_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(regular_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 0, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(regular_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_override_path(override_layer_folder.location())),
        "meta_test_layer.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(1, 1, 0);
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
    ASSERT_TRUE(env.debug_log.find(std::string("Layer \"") + lunarg_meta_layer_name +
                                   "\" contains meta-layer-specific override paths, but using older JSON file version."));
    env.layers.clear();
}

// app_key contains test executable name, should activate the override layer
TEST(OverrideMetaLayer, AppKeysDoesContainCurrentApplication) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    std::string cur_path = test_platform_executable_path();

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .add_component_layers({regular_layer_name})
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_app_key(cur_path)),
        "meta_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
    {
        // instance
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
    }
}

// app_key contains random strings, should not activate the override layer
TEST(OverrideMetaLayer, AppKeysDoesNotContainCurrentApplication) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(lunarg_meta_layer_name)
                                   .add_component_layers({regular_layer_name})
                                   .set_disable_environment("DisableMeIfYouCan")
                                   .add_app_keys({"/Hello", "Hi", "./../Uh-oh", "C:/Windows/Only"})),
                           "meta_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
    }
    {
        // instance
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

TEST(OverrideMetaLayer, RunningWithElevatedPrivilegesFromSecureLocation) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* regular_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(regular_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    auto override_folder_location = override_layer_folder.location().string();
    env.add_implicit_layer(TestLayerDetails{
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(regular_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_override_path(override_layer_folder.location())),
        "meta_test_layer.json"});

    {  // try with no elevated privileges
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));
        env.debug_log.clear();
    }

    env.platform_shim->set_elevated_privilege(true);

    {  // try with elevated privileges
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, active_layer_props));
    }
}

// Override layer should not be found and thus not loaded when running with elevated privileges
TEST(OverrideMetaLayer, RunningWithElevatedPrivilegesFromUnsecureLocation) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    auto& override_layer_folder = env.get_folder(ManifestLocation::override_layer);

    const char* regular_layer_name = "VK_LAYER_TestLayer_1";
    override_layer_folder.write_manifest("regular_test_layer.json",
                                         ManifestLayer{}
                                             .add_layer(ManifestLayer::LayerDescription{}
                                                            .set_name(regular_layer_name)
                                                            .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                            .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0)))
                                             .get_manifest_str());
    env.add_implicit_layer(TestLayerDetails{
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(regular_layer_name)
                                                                         .set_disable_environment("DisableMeIfYouCan")
                                                                         .add_override_path(override_layer_folder.location())),
        "meta_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::unsecured_generic));

    {  // try with no elevated privileges
        auto layer_props = env.GetLayerProperties(2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, layer_props));

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        env.debug_log.clear();
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        EXPECT_TRUE(check_permutation({regular_layer_name, lunarg_meta_layer_name}, active_layer_props));
    }

    env.platform_shim->set_elevated_privilege(true);

    {  // try with no elevated privileges
        ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

// Makes sure explicit layers can't override pre-instance functions even if enabled by the override layer
TEST(ExplicitLayers, OverridePreInstanceFunctions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_enabled_by_override";
    const char* disable_env_var = "DISABLE_ME";

    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceVersion")
                                               .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
        "explicit_test_layer.json");

    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(34);
    layer.set_reported_extension_props(22);
    layer.set_reported_instance_version(VK_MAKE_API_VERSION(1, 0, 0, 1));

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .add_component_layers({explicit_layer_name})
                                                                         .set_disable_environment(disable_env_var)),
        "override_meta_layer.json");

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, 2U);
    count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_EQ(count, 4U);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, VK_HEADER_VERSION_COMPLETE);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
    ASSERT_TRUE(string_eq(layers.at(1).layerName, lunarg_meta_layer_name));
}

TEST(ExplicitLayers, LayerSettingsPreInstanceFunctions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_enabled_by_override";

    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceVersion")
                                               .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
        "explicit_test_layer.json");

    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
        LoaderSettingsLayerConfiguration{}
            .set_name(explicit_layer_name)
            .set_control("on")
            .set_path(env.get_shimmed_layer_manifest_path(0))
            .set_treat_as_implicit_manifest(false)));
    env.update_loader_settings(env.loader_settings);

    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(34);
    layer.set_reported_extension_props(22);
    layer.set_reported_instance_version(VK_MAKE_API_VERSION(1, 0, 0, 1));

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, 1U);
    count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_EQ(count, 4U);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, VK_HEADER_VERSION_COMPLETE);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
}

TEST(ExplicitLayers, ContainsPreInstanceFunctions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_enabled_by_override";

    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceVersion")
                                               .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
        "explicit_test_layer.json");

    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(34);
    layer.set_reported_extension_props(22);
    layer.set_reported_instance_version(VK_MAKE_API_VERSION(1, 0, 0, 1));

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, 1U);
    count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_EQ(count, 4U);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, VK_HEADER_VERSION_COMPLETE);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
}

TEST(ExplicitLayers, CallsPreInstanceFunctionsInCreateInstance) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_enabled_by_override";

    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer.json");

    auto& layer = env.get_test_layer(0);
    layer.set_query_vkEnumerateInstanceLayerProperties(true);
    layer.set_query_vkEnumerateInstanceExtensionProperties(true);
    layer.set_query_vkEnumerateInstanceVersion(true);

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();

    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
}

// This test makes sure that any layer calling GetPhysicalDeviceProperties2 inside of CreateInstance
// succeeds and doesn't crash.
TEST(LayerCreateInstance, GetPhysicalDeviceProperties2) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .set_icd_api_version(VK_API_VERSION_1_1);

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    auto& layer_handle = env.get_test_layer(0);
    layer_handle.set_create_instance_callback([](TestLayer& layer) -> VkResult {
        uint32_t phys_dev_count = 0;
        VkResult res = layer.instance_dispatch_table.EnumeratePhysicalDevices(layer.instance_handle, &phys_dev_count, nullptr);
        if (res != VK_SUCCESS || phys_dev_count > 1) {
            return VK_ERROR_INITIALIZATION_FAILED;  // expecting only a single physical device.
        }
        VkPhysicalDevice phys_dev{};
        res = layer.instance_dispatch_table.EnumeratePhysicalDevices(layer.instance_handle, &phys_dev_count, &phys_dev);
        if (res != VK_SUCCESS) {
            return VK_ERROR_INITIALIZATION_FAILED;
        }
        VkPhysicalDeviceProperties2 props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        layer.instance_dispatch_table.GetPhysicalDeviceProperties2(phys_dev, &props2);

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        layer.instance_dispatch_table.GetPhysicalDeviceFeatures2(phys_dev, &features2);
        return VK_SUCCESS;
    });

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(regular_layer_name).set_api_version(1, 1, 0);
    inst.CheckCreate();
}

TEST(LayerCreateInstance, GetPhysicalDeviceProperties2KHR) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .add_instance_extension({"VK_KHR_get_physical_device_properties2", 0});

    const char* regular_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    auto& layer_handle = env.get_test_layer(0);
    layer_handle.set_create_instance_callback([](TestLayer& layer) -> VkResult {
        uint32_t phys_dev_count = 1;
        VkPhysicalDevice phys_dev{};
        layer.instance_dispatch_table.EnumeratePhysicalDevices(layer.instance_handle, &phys_dev_count, &phys_dev);

        VkPhysicalDeviceProperties2KHR props2{};
        props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        layer.instance_dispatch_table.GetPhysicalDeviceProperties2KHR(phys_dev, &props2);

        VkPhysicalDeviceFeatures2KHR features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
        layer.instance_dispatch_table.GetPhysicalDeviceFeatures2KHR(phys_dev, &features2);
        return VK_SUCCESS;
    });

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(regular_layer_name).add_extension("VK_KHR_get_physical_device_properties2");
    inst.CheckCreate();
}

TEST(ExplicitLayers, MultipleLayersInSingleManifest) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    const char* regular_layer_name_3 = "VK_LAYER_RegularLayer3";
    env.add_explicit_layer(TestLayerDetails(
        ManifestLayer{}
            .set_file_format_version({1, 0, 1})
            .add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2))
            .add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2))
            .add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_3).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "multi_layer_manifest.json"));

    auto layer_props = env.GetLayerProperties(3);
    ASSERT_TRUE(string_eq(regular_layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(regular_layer_name_2, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(regular_layer_name_3, layer_props[2].layerName));
}

TEST(ExplicitLayers, WrapObjects) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device("physical_device_0");

    const char* wrap_objects_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(ManifestLayer{}.add_layer(
                               ManifestLayer::LayerDescription{}.set_name(wrap_objects_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
                           "wrap_objects_layer.json");

    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_layer_1.json");

    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_layer_2.json");

    {  // just the wrap layer
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(wrap_objects_name);
        inst.CheckCreate();
        VkPhysicalDevice phys_dev = inst.GetPhysDev();

        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_dev);
    }
    {  // wrap layer first
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(wrap_objects_name).add_layer(regular_layer_name_1);
        inst.CheckCreate();
        VkPhysicalDevice phys_dev = inst.GetPhysDev();

        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_dev);
    }
    {  // wrap layer last
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(regular_layer_name_1).add_layer(wrap_objects_name);
        inst.CheckCreate();
        VkPhysicalDevice phys_dev = inst.GetPhysDev();

        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_dev);
    }
    {  // wrap layer last
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(regular_layer_name_1).add_layer(wrap_objects_name).add_layer(regular_layer_name_2);
        inst.CheckCreate();
        VkPhysicalDevice phys_dev = inst.GetPhysDev();

        DeviceWrapper dev{inst};
        dev.CheckCreate(phys_dev);
    }
}

TEST(ExplicitLayers, VkLayerPathEnvVar) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layer loads successfully when setting VK_LAYER_PATH to a full filepath
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(
        TestLayerDetails(
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            "regular_layer_1.json")
            .set_discovery_type(ManifestDiscoveryType::env_var)
            .set_is_dir(false));

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.add_layer(regular_layer_name_1);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name_1));
}

TEST(ExplicitLayers, VkLayerPathEnvVarContainsMultipleFilepaths) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layers load successfully when setting VK_LAYER_PATH to multiple full filepaths
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(
        TestLayerDetails(
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            "regular_layer_1.json")
            .set_discovery_type(ManifestDiscoveryType::env_var)
            .set_is_dir(false));

    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(
        TestLayerDetails(
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            "regular_layer_2.json")
            .set_discovery_type(ManifestDiscoveryType::env_var)
            .set_is_dir(false));

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.add_layer(regular_layer_name_1);
    inst.create_info.add_layer(regular_layer_name_2);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    EXPECT_TRUE(check_permutation({regular_layer_name_1, regular_layer_name_2}, layer_props));
}

TEST(ExplicitLayers, VkLayerPathEnvVarIsDirectory) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layers load successfully when setting VK_LAYER_PATH to a directory
    const char* regular_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(
        TestLayerDetails(
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            "regular_layer_1.json")
            .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* regular_layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(
        TestLayerDetails(
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(regular_layer_name_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            "regular_layer_2.json")
            .set_discovery_type(ManifestDiscoveryType::env_var));

    InstWrapper inst(env.vulkan_functions);
    inst.create_info.add_layer(regular_layer_name_1);
    inst.create_info.add_layer(regular_layer_name_2);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    EXPECT_TRUE(check_permutation({regular_layer_name_1, regular_layer_name_2}, layer_props));
}

TEST(ExplicitLayers, DuplicateLayersInVK_LAYER_PATH) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    // verify layer loads successfully when setting VK_LAYER_PATH to a full filepath
    const char* same_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json")
                               // use override folder as just a folder and manually set the VK_LAYER_PATH env-var to it
                               .set_discovery_type(ManifestDiscoveryType::override_folder)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");
    layer1.set_make_spurious_log_in_create_instance("actually_layer_1");
    env.env_var_vk_layer_paths.add_to_list(env.get_folder(ManifestLocation::override_layer).location());

    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    layer2.set_make_spurious_log_in_create_instance("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(same_layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();

    {
        EnvVarWrapper layers_enable_env_var{"VK_INSTANCE_LAYERS", same_layer_name_1};
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();

    {
        EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE", same_layer_name_1};
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();
}

TEST(ExplicitLayers, DuplicateLayersInVK_ADD_LAYER_PATH) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* same_layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json")
                               // use override folder as just a folder and manually set the VK_ADD_LAYER_PATH env-var to it
                               .set_discovery_type(ManifestDiscoveryType::override_folder)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");
    layer1.set_make_spurious_log_in_create_instance("actually_layer_1");
    env.add_env_var_vk_layer_paths.add_to_list(env.get_folder(ManifestLocation::override_layer).location());

    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(same_layer_name_1)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::add_env_var)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    layer2.set_make_spurious_log_in_create_instance("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(same_layer_name_1, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(same_layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();

    {
        EnvVarWrapper layers_enable_env_var{"VK_INSTANCE_LAYERS", same_layer_name_1};
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();

    {
        EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE", same_layer_name_1};
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(same_layer_name_1, enabled_layer_props.at(0).layerName));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props.at(0).description));
        ASSERT_TRUE(env.debug_log.find("actually_layer_1"));
        ASSERT_FALSE(env.debug_log.find("actually_layer_2"));
    }
    env.debug_log.clear();
}

TEST(ExplicitLayers, CorrectOrderOfEnvVarEnabledLayers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");

    const char* layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_2)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_2.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer_name_2, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));
    // 1 then 2
    {
        EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
        inst_layers_env_var.add_to_list(layer_name_1);
        inst_layers_env_var.add_to_list(layer_name_2);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        // Expect the env-var layer to be first
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[1].layerName));
    }
    // 2 then 1
    {
        EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
        inst_layers_env_var.add_to_list(layer_name_2);
        inst_layers_env_var.add_to_list(layer_name_1);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        // Expect the env-var layer to be first
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[1].layerName));
    }
}
// Test to make sure order layers are found in VK_LAYER_PATH is what decides which layer is loaded
TEST(ExplicitLayers, DuplicateLayersInVkLayerPath) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "layer.json")
                               .set_discovery_type(ManifestDiscoveryType::env_var)
                               .set_is_dir(true));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");

    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "layer.json")
                               // putting it in a separate folder then manually adding the folder to VK_LAYER_PATH
                               .set_discovery_type(ManifestDiscoveryType::override_folder)
                               .set_is_dir(true));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    env.env_var_vk_layer_paths.add_to_list(env.get_folder(ManifestLocation::override_layer).location().string());

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_name, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer_name, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));

    EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
    inst_layers_env_var.add_to_list(layer_name);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Expect the first layer added to be found
    auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layer_name, enabled_layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[0].description));
}

TEST(ExplicitLayers, CorrectOrderOfEnvVarEnabledLayersFromSystemLocations) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json"));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");

    const char* layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_2)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_2.json"));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer_name_2, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));
    // 1 then 2
    {
        EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
        inst_layers_env_var.add_to_list(layer_name_1);
        inst_layers_env_var.add_to_list(layer_name_2);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[1].layerName));
    }
    // 2 then 1
    {
        EnvVarWrapper inst_layers_env_var{"VK_INSTANCE_LAYERS"};
        inst_layers_env_var.add_to_list(layer_name_2);
        inst_layers_env_var.add_to_list(layer_name_1);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        // Expect the env-var layer to be first
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[1].layerName));
    }
}

TEST(ExplicitLayers, CorrectOrderOfApplicationEnabledLayers) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name_1 = "VK_LAYER_RegularLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_1)
                                                                          .set_description("actually_layer_1")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_1.json"));
    auto& layer1 = env.get_test_layer(0);
    layer1.set_description("actually_layer_1");
    layer1.set_make_spurious_log_in_create_instance("actually_layer_1");

    const char* layer_name_2 = "VK_LAYER_RegularLayer2";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name_2)
                                                                          .set_description("actually_layer_2")
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_layer_2.json"));
    auto& layer2 = env.get_test_layer(1);
    layer2.set_description("actually_layer_2");
    layer2.set_make_spurious_log_in_create_instance("actually_layer_2");

    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_name_1, layer_props[0].layerName));
    ASSERT_TRUE(string_eq(layer_name_2, layer_props[1].layerName));
    ASSERT_TRUE(string_eq(layer1.description.c_str(), layer_props[0].description));
    ASSERT_TRUE(string_eq(layer2.description.c_str(), layer_props[1].description));

    // 1 then 2
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name_1);
        inst.create_info.add_layer(layer_name_2);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        // Expect the env-var layer to be first
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[1].layerName));
    }
    // 2 then 1
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name_2);
        inst.create_info.add_layer(layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        // Expect the env-var layer to be first
        auto enabled_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer2.description.c_str(), enabled_layer_props[0].description));
        ASSERT_TRUE(string_eq(layer1.description.c_str(), enabled_layer_props[1].description));
        ASSERT_TRUE(string_eq(layer_name_2, enabled_layer_props[0].layerName));
        ASSERT_TRUE(string_eq(layer_name_1, enabled_layer_props[1].layerName));
    }
}

TEST(LayerExtensions, ImplicitNoAdditionalInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)),
                           "implicit_wrap_layer_no_ext.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    // Make sure the extensions that are implemented only in the test layers is not present.
    ASSERT_FALSE(contains(extensions, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME));
    ASSERT_FALSE(contains(extensions, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Make sure all the function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ImplicitDirDispModeInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_1)
                .set_disable_environment(disable_env_var)
                .set_enable_environment(enable_env_var)
                .add_instance_extension({VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, 1, {"vkReleaseDisplayEXT"}})),
        "implicit_wrap_layer_dir_disp_mode.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    auto extensions = env.GetInstanceExtensions(5);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME));

    // Make sure the extensions that are implemented only in the test layers is not present.
    ASSERT_FALSE(contains(extensions, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);
    inst.CheckCreate();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ImplicitDispSurfCountInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS_2)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)
                                                         .add_instance_extension({VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
                                                                                  1,
                                                                                  {"vkGetPhysicalDeviceSurfaceCapabilities2EXT"}})),
                           "implicit_wrap_layer_disp_surf_count.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    auto extensions = env.GetInstanceExtensions(5);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst.CheckCreate();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkReleaseDisplayEXT"));
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ImplicitBothInstanceExtensions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .set_disable_environment(disable_env_var)
                .set_enable_environment(enable_env_var)
                .add_instance_extension({VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, 1, {"vkReleaseDisplayEXT"}})
                .add_instance_extension(
                    {VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME, 1, {"vkGetPhysicalDeviceSurfaceCapabilities2EXT"}})),
        "implicit_wrap_layer_both_inst.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    auto extensions = env.GetInstanceExtensions(6);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(4).extensionName, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(5).extensionName, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)
        .add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst.CheckCreate();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkReleaseDisplayEXT"));
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ExplicitNoAdditionalInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
        "explicit_wrap_layer_no_ext.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    // Now query by layer name.
    ASSERT_NO_FATAL_FAILURE(env.GetInstanceExtensions(0, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    // Make sure all the function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ExplicitDirDispModeInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_1)
                .add_instance_extension({VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, 1, {"vkReleaseDisplayEXT"}})),
        "explicit_wrap_layer_dir_disp_mode.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    // Now query by layer name.
    auto layer_extensions = env.GetInstanceExtensions(1, explicit_layer_name);
    EXPECT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME));

    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);
    inst1.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));

    InstWrapper inst2{env.vulkan_functions};
    inst2.create_info.add_layer(explicit_layer_name).add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);
    inst2.CheckCreate();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ExplicitDispSurfCountInstanceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS_2)
                                                         .add_instance_extension({VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
                                                                                  1,
                                                                                  {"vkGetPhysicalDeviceSurfaceCapabilities2EXT"}})),
                           "explicit_wrap_layer_disp_surf_count.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    // Now query by layer name.
    auto layer_extensions = env.GetInstanceExtensions(1, explicit_layer_name);
    EXPECT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst1.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));

    InstWrapper inst2{env.vulkan_functions};
    inst2.create_info.add_layer(explicit_layer_name).add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst2.CheckCreate();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkReleaseDisplayEXT"));
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));
}

TEST(LayerExtensions, ExplicitBothInstanceExtensions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .add_instance_extension({VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, 1, {"vkReleaseDisplayEXT"}})
                .add_instance_extension(
                    {VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME, 1, {"vkGetPhysicalDeviceSurfaceCapabilities2EXT"}})),
        "explicit_wrap_layer_both_inst.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));

    // Make sure the extensions still aren't present in this layer
    auto layer_extensions = env.GetInstanceExtensions(2, explicit_layer_name);
    EXPECT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(layer_extensions.at(1).extensionName, VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME));

    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)
        .add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst1.CheckCreate(VK_ERROR_EXTENSION_NOT_PRESENT);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkReleaseDisplayEXT"));
    handle_assert_null(env.vulkan_functions.vkGetInstanceProcAddr(inst1.inst, "vkGetPhysicalDeviceSurfaceCapabilities2EXT"));

    InstWrapper inst2{env.vulkan_functions};
    inst2.create_info.add_layer(explicit_layer_name)
        .add_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)
        .add_extension(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
    inst2.CheckCreate();
    VkPhysicalDevice phys_dev = inst2.GetPhysDev();

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(env.vulkan_functions.vkGetInstanceProcAddr(inst2.inst, "vkReleaseDisplayEXT"));
    PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT pfnGetPhysicalDeviceSurfaceCapabilities2EXT =
        inst2.load("vkGetPhysicalDeviceSurfaceCapabilities2EXT");
    handle_assert_has_value(pfnGetPhysicalDeviceSurfaceCapabilities2EXT);

    VkSurfaceCapabilities2EXT surf_caps{VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT};

    // Call and then check a few things
    ASSERT_EQ(VK_SUCCESS, pfnGetPhysicalDeviceSurfaceCapabilities2EXT(phys_dev, VK_NULL_HANDLE, &surf_caps));
    ASSERT_EQ(7U, surf_caps.minImageCount);
    ASSERT_EQ(12U, surf_caps.maxImageCount);
    ASSERT_EQ(365U, surf_caps.maxImageArrayLayers);
}

TEST(LayerExtensions, ImplicitNoAdditionalDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)),
                           "implicit_wrap_layer_no_ext.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    ASSERT_NO_FATAL_FAILURE(inst.EnumerateDeviceExtensions(phys_dev, 0));

    // Device functions queried using vkGetInstanceProcAddr should be non-NULL since this could be available for any attached
    // physical device.
    PFN_vkTrimCommandPoolKHR pfn_vkTrimCommandPool = inst.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_vkGetSwapchainStatus = inst.load("vkGetSwapchainStatusKHR");
    PFN_vkSetDeviceMemoryPriorityEXT pfn_vkSetDeviceMemoryPriority = inst.load("vkSetDeviceMemoryPriorityEXT");
    handle_assert_has_value(pfn_vkTrimCommandPool);
    handle_assert_has_value(pfn_vkGetSwapchainStatus);
    handle_assert_has_value(pfn_vkSetDeviceMemoryPriority);

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_dev);

    // Query again after create device to make sure the value returned by vkGetInstanceProcAddr did not change
    PFN_vkTrimCommandPoolKHR pfn_vkTrimCommandPool2 = inst.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_vkGetSwapchainStatus2 = inst.load("vkGetSwapchainStatusKHR");
    PFN_vkSetDeviceMemoryPriorityEXT pfn_vkSetDeviceMemoryPriority2 = inst.load("vkSetDeviceMemoryPriorityEXT");
    ASSERT_EQ(pfn_vkTrimCommandPool, pfn_vkTrimCommandPool2);
    ASSERT_EQ(pfn_vkGetSwapchainStatus, pfn_vkGetSwapchainStatus2);
    ASSERT_EQ(pfn_vkSetDeviceMemoryPriority, pfn_vkSetDeviceMemoryPriority2);

    // Make sure all the function pointers returned by vkGetDeviceProcAddr for non-enabled extensions are NULL
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkSetDeviceMemoryPriorityEXT"));

    // Even though the instance functions returned are non-NULL.  They should not work if we haven't enabled the extensions.
    ASSERT_DEATH(pfn_vkTrimCommandPool(dev.dev, VK_NULL_HANDLE, 0), "");
    ASSERT_DEATH(pfn_vkGetSwapchainStatus(dev.dev, VK_NULL_HANDLE), "");
    ASSERT_DEATH(pfn_vkSetDeviceMemoryPriority(dev.dev, VK_NULL_HANDLE, 0.f), "");
}

TEST(LayerExtensions, ImplicitMaintenanceDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS_1)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)),
                           "implicit_wrap_layer_maint.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    auto device_extensions = inst.EnumerateDeviceExtensions(phys_dev, 1);
    ASSERT_TRUE(string_eq(device_extensions.at(0).extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
}

TEST(LayerExtensions, ImplicitPresentImageDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS_2)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)),
                           "implicit_wrap_layer_pres.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    auto device_extensions = inst.EnumerateDeviceExtensions(phys_dev, 1);
    ASSERT_TRUE(string_eq(device_extensions.at(0).extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
}

TEST(LayerExtensions, ImplicitBothDeviceExtensions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    const char* enable_env_var = "ENABLE_ME";
    const char* disable_env_var = "DISABLE_ME";

    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                                                         .set_disable_environment(disable_env_var)
                                                         .set_enable_environment(enable_env_var)),
                           "implicit_wrap_layer_both_dev.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, implicit_layer_name));

    // set enable env-var, layer should load
    EnvVarWrapper wrap_enable_env_var{enable_env_var, "1"};
    CheckLogForLayerString(env, implicit_layer_name, true);

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    auto device_extensions = inst.EnumerateDeviceExtensions(phys_dev, 2);
    ASSERT_TRUE(string_eq(device_extensions.at(0).extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME));
    ASSERT_TRUE(string_eq(device_extensions.at(1).extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME).add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
}

TEST(LayerExtensions, ExplicitNoAdditionalDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_WRAP_OBJECTS)),
        "explicit_wrap_layer_no_ext.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    ASSERT_NO_FATAL_FAILURE(inst.EnumerateDeviceExtensions(phys_dev, 0));
    ASSERT_NO_FATAL_FAILURE(inst.EnumerateLayerDeviceExtensions(phys_dev, explicit_layer_name, 0));

    DeviceWrapper dev{inst};
    dev.CheckCreate(phys_dev);

    // Make sure all the function pointers are NULL as well
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkSetDeviceMemoryPriorityEXT"));
}

TEST(LayerExtensions, ExplicitMaintenanceDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                      .set_name(explicit_layer_name)
                                      .set_lib_path(TEST_LAYER_WRAP_OBJECTS_1)
                                      .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                                      .add_device_extension({VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1, {"vkTrimCommandPoolKHR"}})),
        "explicit_wrap_layer_maint.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    ASSERT_NO_FATAL_FAILURE(inst.EnumerateDeviceExtensions(phys_dev, 0));
    auto layer_extensions = inst.EnumerateLayerDeviceExtensions(phys_dev, explicit_layer_name, 1);
    ASSERT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
}

TEST(LayerExtensions, ExplicitPresentImageDeviceExtension) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_2)
                .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                .add_device_extension({VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, 1, {"vkGetSwapchainStatusKHR"}})),
        "explicit_wrap_layer_pres.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    ASSERT_NO_FATAL_FAILURE(inst.EnumerateDeviceExtensions(phys_dev, 0));
    auto layer_extensions = inst.EnumerateLayerDeviceExtensions(phys_dev, explicit_layer_name, 1);
    ASSERT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_null(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkGetSwapchainStatusKHR"));
}

TEST(LayerExtensions, ExplicitBothDeviceExtensions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                .add_device_extension({VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1, {"vkTrimCommandPoolKHR"}})
                .add_device_extension({VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, 1, {"vkGetSwapchainStatusKHR"}})),
        "explicit_wrap_layer_both_dev.json");

    auto layers = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layers[0].layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();
    handle_assert_has_value(phys_dev);

    ASSERT_NO_FATAL_FAILURE(inst.EnumerateDeviceExtensions(phys_dev, 0));
    auto layer_extensions = inst.EnumerateLayerDeviceExtensions(phys_dev, explicit_layer_name, 2);
    ASSERT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME));
    ASSERT_TRUE(string_eq(layer_extensions.at(1).extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME));

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME).add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    dev.CheckCreate(phys_dev);

    // Make sure only the appropriate function pointers are NULL as well
    handle_assert_has_value(dev->vkGetDeviceProcAddr(dev.dev, "vkTrimCommandPoolKHR"));

    PFN_vkGetSwapchainStatusKHR gipa_pfnGetSwapchainStatusKHR = inst.load("vkGetSwapchainStatusKHR");
    PFN_vkGetSwapchainStatusKHR gdpa_pfnGetSwapchainStatusKHR = dev.load("vkGetSwapchainStatusKHR");
    handle_assert_has_value(gipa_pfnGetSwapchainStatusKHR);
    handle_assert_has_value(gdpa_pfnGetSwapchainStatusKHR);

    // Make sure both versions (from vkGetInstanceProcAddr and vkGetDeviceProcAddr) return the same value.
    ASSERT_EQ(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, gipa_pfnGetSwapchainStatusKHR(dev.dev, VK_NULL_HANDLE));
    ASSERT_EQ(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, gdpa_pfnGetSwapchainStatusKHR(dev.dev, VK_NULL_HANDLE));
}

TEST(TestLayers, ExplicitlyEnableImplicitLayer) {
    FrameworkEnvironment env;
    uint32_t api_version = VK_API_VERSION_1_2;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, api_version))
        .set_icd_api_version(api_version)
        .add_physical_device(PhysicalDevice{}.set_api_version(api_version).finish());

    const char* regular_layer_name = "VK_LAYER_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                         .set_disable_environment("DisableMeIfYouCan")),
                           "regular_test_layer.json");
    {  // 1.1 instance
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(regular_layer_name);
        inst.create_info.set_api_version(1, 1, 0);
        inst.CheckCreate();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
    {  // 1.2 instance
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(regular_layer_name);
        inst.create_info.set_api_version(1, 2, 0);
        inst.CheckCreate();
        inst.GetActiveLayers(inst.GetPhysDev(), 1);
    }
}

TEST(TestLayers, NewerInstanceVersionThanImplicitLayer) {
    FrameworkEnvironment env;
    uint32_t api_version = VK_API_VERSION_1_2;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, api_version))
        .set_icd_api_version(api_version)
        .add_physical_device(PhysicalDevice{}.set_api_version(api_version).finish());

    const char* regular_layer_name = "VK_LAYER_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                         .set_disable_environment("DisableMeIfYouCan")),
                           "regular_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
    }
    {  // 1.1 instance - should find the implicit layer
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        EXPECT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        env.debug_log.clear();
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
    {  // 1.2 instance -- instance layer should be found
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 2, 0);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        EXPECT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + regular_layer_name));
        env.debug_log.clear();

        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
}

TEST(TestLayers, ImplicitLayerPre10APIVersion) {
    FrameworkEnvironment env;
    uint32_t api_version = VK_API_VERSION_1_2;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, api_version))
        .set_icd_api_version(api_version)
        .add_physical_device(PhysicalDevice{}.set_api_version(api_version).finish());

    const char* regular_layer_name = "VK_LAYER_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(regular_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 0, 1, 0))
                                                         .set_disable_environment("DisableMeIfYouCan")),
                           "regular_test_layer.json");
    {  // global functions
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props[0].layerName, regular_layer_name));
    }
    {  // 1.0 instance
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 0, 0);
        FillDebugUtilsCreateDetails(inst.create_info, log);
        inst.CheckCreate();
        EXPECT_TRUE(log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
    {  // 1.1 instance
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 1, 0);
        FillDebugUtilsCreateDetails(inst.create_info, log);
        inst.CheckCreate();
        EXPECT_TRUE(log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
    {  // 1.2 instance
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(1, 2, 0);
        FillDebugUtilsCreateDetails(inst.create_info, log);
        inst.CheckCreate();
        EXPECT_TRUE(log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
    {  // application sets its API version to 0
        DebugUtilsLogger log;
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.set_api_version(0);
        FillDebugUtilsCreateDetails(inst.create_info, log);
        inst.CheckCreate();
        EXPECT_TRUE(log.find(std::string("Insert instance layer \"") + regular_layer_name));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(regular_layer_name, layer_props.at(0).layerName));
    }
}

// Verify that VK_INSTANCE_LAYERS work.  To test this, make sure that an explicit layer does not affect an instance until
// it is set with VK_INSTANCE_LAYERS
TEST(TestLayers, InstEnvironEnableExplicitLayer) {
    FrameworkEnvironment env;
    uint32_t api_version = VK_API_VERSION_1_2;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, api_version))
        .set_icd_api_version(api_version)
        .add_physical_device(PhysicalDevice{}.set_api_version(api_version).finish());

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                .add_device_extension({VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1, {"vkTrimCommandPoolKHR"}})
                .add_device_extension({VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, 1, {"vkGetSwapchainStatusKHR"}})),
        "explicit_wrap_layer_both_dev.json");

    // First, test an instance/device without the layer forced on.  The extensions shouldn't be present and
    // the function pointers should be NULL.
    InstWrapper inst1{env.vulkan_functions};
    inst1.CheckCreate();
    VkPhysicalDevice phys_dev1 = inst1.GetPhysDev();

    ASSERT_NO_FATAL_FAILURE(inst1.EnumerateDeviceExtensions(phys_dev1, 0));

    // Create a device and query the function pointers
    DeviceWrapper dev1{inst1};
    dev1.CheckCreate(phys_dev1);
    PFN_vkTrimCommandPoolKHR pfn_TrimCommandPoolBefore = dev1.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_GetSwapchainStatusBefore = dev1.load("vkGetSwapchainStatusKHR");
    handle_assert_null(pfn_TrimCommandPoolBefore);
    handle_assert_null(pfn_GetSwapchainStatusBefore);

    // Now setup the instance layer
    EnvVarWrapper instance_layers_env_var{"VK_INSTANCE_LAYERS", explicit_layer_name};

    // Now, test an instance/device with the layer forced on.  The extensions should be present and
    // the function pointers should be valid.
    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();
    VkPhysicalDevice phys_dev2 = inst2.GetPhysDev();

    auto layer_extensions = inst2.EnumerateLayerDeviceExtensions(phys_dev2, explicit_layer_name, 2);
    ASSERT_TRUE(string_eq(layer_extensions.at(0).extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME));
    ASSERT_TRUE(string_eq(layer_extensions.at(1).extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME));

    DeviceWrapper dev2{inst2};
    dev2.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
        .add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME);
    dev2.CheckCreate(phys_dev2);

    PFN_vkTrimCommandPoolKHR pfn_TrimCommandPoolAfter = dev2.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_GetSwapchainStatusAfter = dev2.load("vkGetSwapchainStatusKHR");
    handle_assert_has_value(pfn_TrimCommandPoolAfter);
    handle_assert_has_value(pfn_GetSwapchainStatusAfter);

    ASSERT_EQ(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, pfn_GetSwapchainStatusAfter(dev2.dev, VK_NULL_HANDLE));
}

// Verify that VK_LOADER_LAYERS_ENABLE work.  To test this, make sure that an explicit layer does not affect an instance until
// it is set with VK_LOADER_LAYERS_ENABLE
TEST(TestLayers, EnvironLayerEnableExplicitLayer) {
    FrameworkEnvironment env;
    uint32_t api_version = VK_API_VERSION_1_2;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, api_version))
        .set_icd_api_version(api_version)
        .add_physical_device(PhysicalDevice{});

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_1);

    const char* explicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* explicit_json_name_2 = "Second_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_2);

    const char* explicit_layer_name_3 = "VK_LAYER_LUNARG_second_test_layer";
    const char* explicit_json_name_3 = "second_test_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_3);

    EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE"};

    // First, test an instance/device without the layer forced on.
    InstWrapper inst1{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    inst1.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Now force on one layer with its full name
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value(explicit_layer_name_1);

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    inst2.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match prefix
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("VK_LAYER_LUNARG_*");

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    inst3.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match suffix
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*Second_layer");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    inst4.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match substring
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    inst5.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with star '*'
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("*");

    InstWrapper inst6{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    inst6.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value("~all~");

    InstWrapper inst7{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst7.create_info, env.debug_log);
    inst7.CheckCreate();

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Verify that VK_LOADER_LAYERS_DISABLE work.  To test this, make sure that an explicit layer does not affect an instance until
// it is set with VK_LOADER_LAYERS_DISABLE
TEST(TestLayers, EnvironLayerDisableExplicitLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2)
        .add_physical_device(PhysicalDevice{});

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_1);

    const char* explicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* explicit_json_name_2 = "Second_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_2);

    const char* explicit_layer_name_3 = "VK_LAYER_LUNARG_Second_test_layer";
    const char* explicit_json_name_3 = "Second_test_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_3);
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};

    // First, test an instance/device without the layer forced on.
    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst1.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Now force on one layer with its full name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value(explicit_layer_name_1);

    InstWrapper inst2{env.vulkan_functions};
    inst2.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst2.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match prefix
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("VK_LAYER_LUNARG_*");

    InstWrapper inst3{env.vulkan_functions};
    inst3.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst3.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match suffix
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second_layer");

    InstWrapper inst4{env.vulkan_functions};
    inst4.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst4.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match substring
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    inst5.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst5.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with star '*'
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*");

    InstWrapper inst6{env.vulkan_functions};
    inst6.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst6.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match all with special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~all~");

    InstWrapper inst7{env.vulkan_functions};
    inst7.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst7.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst7.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Match explicit special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~explicit~");

    InstWrapper inst8{env.vulkan_functions};
    inst8.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst8.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst8.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // No match implicit special name
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~implicit~");

    InstWrapper inst9{env.vulkan_functions};
    inst9.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst9.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst9.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Verify that VK_LOADER_LAYERS_ENABLE + VK_LOADER_LAYERS_DISABLE work.(results in the layer still being
// enabled)
TEST(TestLayers, EnvironLayerEnableDisableExplicitLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_1);

    const char* explicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* explicit_json_name_2 = "Second_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_2);

    const char* explicit_layer_name_3 = "VK_LAYER_LUNARG_Second_test_layer";
    const char* explicit_json_name_3 = "Second_test_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_3)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_3);

    EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE"};
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};

    // First, test an instance/device without the layer forced on.
    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst1.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable 2 but enable 1
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*Second*");
    layers_enable_env_var.set_new_value("*test_layer");

    InstWrapper inst2{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst2.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable all but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst3{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst3.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable all but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~all~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst4{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst4.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst4.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable explicit but enable 2
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~explicit~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst5{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst5.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst5.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable implicit but enable 2 (should still be everything)
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~implicit~");
    layers_enable_env_var.set_new_value("*Second*");

    InstWrapper inst6{env.vulkan_functions};
    inst6.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst6.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst6.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));

    // Disable explicit but enable all
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("~explicit~");
    layers_enable_env_var.set_new_value("*");

    InstWrapper inst7{env.vulkan_functions};
    inst7.create_info.add_layer(explicit_layer_name_1).add_layer(explicit_layer_name_2).add_layer(explicit_layer_name_3);
    FillDebugUtilsCreateDetails(inst7.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst7.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_3));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_3, "disabled because name matches filter of env var"));
}

// Verify that VK_INSTANCE_LAYERS + VK_LOADER_LAYERS_DISABLE work.(results in the layer still being
// enabled)
TEST(TestLayers, EnvironVkInstanceLayersAndDisableFilters) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_1);

    const char* explicit_layer_name_2 = "VK_LAYER_LUNARG_Second_layer";
    const char* explicit_json_name_2 = "Second_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_2);

    EnvVarWrapper layers_enable_env_var{"VK_INSTANCE_LAYERS"};
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};

    // First, test an instance/device without the layer forced on.
    InstWrapper inst1{env.vulkan_functions};
    inst1.create_info.add_layer(explicit_layer_name_1);
    FillDebugUtilsCreateDetails(inst1.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst1.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));

    // Enable the non-default enabled layer with VK_INSTANCE_LAYERS
    // ------------------------------------------
    env.debug_log.clear();
    layers_enable_env_var.set_new_value(explicit_layer_name_2);

    InstWrapper inst2{env.vulkan_functions};
    inst2.create_info.add_layer(explicit_layer_name_1);
    FillDebugUtilsCreateDetails(inst2.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst2.CheckCreate());

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));

    // Try to disable all
    // ------------------------------------------
    env.debug_log.clear();
    layers_disable_env_var.set_new_value("*");

    InstWrapper inst3{env.vulkan_functions};
    inst3.create_info.add_layer(explicit_layer_name_1);
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst3.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer", explicit_layer_name_2));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_2, "disabled because name matches filter of env var"));
}

// Verify that layers enabled through VK_INSTANCE_LAYERS which were not found get the proper error message
TEST(TestLayers, NonExistantLayerInVK_INSTANCE_LAYERS) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name = "VK_LAYER_test_layer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    EnvVarWrapper layers_enable_env_var{"VK_INSTANCE_LAYERS", "VK_LAYER_I_dont_exist"};
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(
            env.debug_log.find("Layer \"VK_LAYER_I_dont_exist\" was not found but was requested by env var VK_INSTANCE_LAYERS!"));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    // Make sure layers that do exist are loaded
    env.debug_log.clear();
    layers_enable_env_var.add_to_list(layer_name);
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(
            env.debug_log.find("Layer \"VK_LAYER_I_dont_exist\" was not found but was requested by env var VK_INSTANCE_LAYERS!"));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
    }
    // Make sure that if the layer appears twice in the env-var nothing bad happens
    env.debug_log.clear();
    layers_enable_env_var.add_to_list("VK_LAYER_I_dont_exist");
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(
            env.debug_log.find("Layer \"VK_LAYER_I_dont_exist\" was not found but was requested by env var VK_INSTANCE_LAYERS!"));
        auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
    }
}

// Verify that if the same layer appears twice in VK_INSTANCE_LAYERS nothing bad happens
TEST(TestLayers, DuplicatesInEnvironVK_INSTANCE_LAYERS) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* layer_name = "VK_LAYER_test_layer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer.json");

    EnvVarWrapper layers_enable_env_var{"VK_INSTANCE_LAYERS"};

    layers_enable_env_var.add_to_list(layer_name);
    layers_enable_env_var.add_to_list(layer_name);

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    auto layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
}

TEST(TestLayers, AppEnabledExplicitLayerFails) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))),
                           explicit_json_name_1);

    env.debug_log.clear();
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE", explicit_layer_name_1};

    ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

    InstWrapper inst3{env.vulkan_functions};
    inst3.create_info.add_layer(explicit_layer_name_1);
    FillDebugUtilsCreateDetails(inst3.create_info, env.debug_log);
    ASSERT_NO_FATAL_FAILURE(inst3.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));

    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
    ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
    ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
}

TEST(TestLayers, OverrideEnabledExplicitLayerWithDisableFilter) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                           explicit_json_name_1);
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(explicit_layer_name_1)
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");

    env.debug_log.clear();
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE", explicit_layer_name_1};

    auto layers = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(lunarg_meta_layer_name, layers[0].layerName));

    {  // both override layer and Disable env var
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_TRUE(
            env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    }
    env.debug_log.clear();
    {  // Now try to enable the explicit layer as well
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(explicit_layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_TRUE(
            env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    }
}

TEST(TestLayers, OverrideEnabledExplicitLayerWithDisableFilterForOverrideLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                           explicit_json_name_1);
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_component_layer(explicit_layer_name_1)
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");

    env.debug_log.clear();
    EnvVarWrapper layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE", lunarg_meta_layer_name};

    auto layers = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(explicit_layer_name_1, layers[0].layerName));

    {  // both override layer and Disable env var
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_TRUE(
            env.debug_log.find_prefix_then_postfix(lunarg_meta_layer_name, "disabled because name matches filter of env var"));
    }
    env.debug_log.clear();
    {  // Now try to enable the explicit layer as well
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(explicit_layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_FALSE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_TRUE(
            env.debug_log.find_prefix_then_postfix(lunarg_meta_layer_name, "disabled because name matches filter of env var"));
    }
}

TEST(TestLayers, OverrideBlacklistedLayerWithEnableFilter) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2);

    const char* explicit_layer_name_1 = "VK_LAYER_LUNARG_First_layer";
    const char* explicit_json_name_1 = "First_layer.json";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))),
                           explicit_json_name_1);
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(lunarg_meta_layer_name)
                                                                         .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                                                         .add_blacklisted_layer(explicit_layer_name_1)
                                                                         .set_disable_environment("DisableMeIfYouCan")),
        "meta_test_layer.json");

    env.debug_log.clear();
    EnvVarWrapper layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE", explicit_layer_name_1};

    auto layers = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(explicit_layer_name_1, layers[0].layerName));
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_FALSE(
            env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    }
    env.debug_log.clear();
    {  // Try to enable a blacklisted layer
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(explicit_layer_name_1);
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate());
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Insert instance layer ", explicit_layer_name_1));
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix("Found manifest file", explicit_json_name_1));
        ASSERT_TRUE(env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "forced enabled due to env var"));
        ASSERT_FALSE(
            env.debug_log.find_prefix_then_postfix(explicit_layer_name_1, "disabled because name matches filter of env var"));
    }
}

// Add a device layer, should not work
TEST(TestLayers, DoNotUseDeviceLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2)
        .add_physical_device(PhysicalDevice{}.set_api_version(VK_API_VERSION_1_2).finish());

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                .add_device_extension({VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1, {"vkTrimCommandPoolKHR"}})
                .add_device_extension({VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, 1, {"vkGetSwapchainStatusKHR"}})),
        "explicit_wrap_layer_both_dev.json");

    // First, test an instance/device without the layer forced on.  The extensions shouldn't be present and
    // the function pointers should be NULL.
    InstWrapper inst1{env.vulkan_functions};
    inst1.CheckCreate();
    VkPhysicalDevice phys_dev1 = inst1.GetPhysDev();

    // Make sure the extensions in the layer aren't present
    ASSERT_NO_FATAL_FAILURE(inst1.EnumerateDeviceExtensions(phys_dev1, 0));

    // Create a device and query the function pointers
    DeviceWrapper dev1{inst1};
    dev1.CheckCreate(phys_dev1);
    PFN_vkTrimCommandPoolKHR pfn_TrimCommandPoolBefore = dev1.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_GetSwapchainStatusBefore = dev1.load("vkGetSwapchainStatusKHR");
    handle_assert_null(pfn_TrimCommandPoolBefore);
    handle_assert_null(pfn_GetSwapchainStatusBefore);

    // Now, test an instance/device with the layer forced on.  The extensions should be present and
    // the function pointers should be valid.
    InstWrapper inst2{env.vulkan_functions};
    inst2.CheckCreate();
    VkPhysicalDevice phys_dev2 = inst2.GetPhysDev();

    // Make sure the extensions in the layer aren't present
    ASSERT_NO_FATAL_FAILURE(inst2.EnumerateDeviceExtensions(phys_dev2, 0));

    DeviceWrapper dev2{inst2};
    dev2.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
        .add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME)
        .add_layer(explicit_layer_name);
    dev2.CheckCreate(phys_dev2, VK_ERROR_EXTENSION_NOT_PRESENT);

    DeviceWrapper dev3{inst2};
    dev3.create_info.add_layer(explicit_layer_name);
    dev3.CheckCreate(phys_dev2);

    PFN_vkTrimCommandPoolKHR pfn_TrimCommandPoolAfter = dev3.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_GetSwapchainStatusAfter = dev3.load("vkGetSwapchainStatusKHR");
    handle_assert_null(pfn_TrimCommandPoolAfter);
    handle_assert_null(pfn_GetSwapchainStatusAfter);
}

// Make sure that a layer enabled as both an instance and device layer works properly.
TEST(TestLayers, InstanceAndDeviceLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2)
        .add_physical_device(PhysicalDevice{}.set_api_version(VK_API_VERSION_1_2).finish());

    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(explicit_layer_name)
                .set_lib_path(TEST_LAYER_WRAP_OBJECTS_3)
                .set_api_version(VK_MAKE_API_VERSION(0, 1, 0, 0))
                .add_device_extension({VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1, {"vkTrimCommandPoolKHR"}})
                .add_device_extension({VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, 1, {"vkGetSwapchainStatusKHR"}})),
        "explicit_wrap_layer_both_dev.json");

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.add_layer(explicit_layer_name);
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_extension(VK_KHR_MAINTENANCE1_EXTENSION_NAME)
        .add_extension(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME)
        .add_layer(explicit_layer_name);
    dev.CheckCreate(phys_dev);

    PFN_vkTrimCommandPoolKHR pfn_TrimCommandPoolAfter = dev.load("vkTrimCommandPoolKHR");
    PFN_vkGetSwapchainStatusKHR pfn_GetSwapchainStatusAfter = dev.load("vkGetSwapchainStatusKHR");
    handle_assert_has_value(pfn_TrimCommandPoolAfter);
    handle_assert_has_value(pfn_GetSwapchainStatusAfter);

    ASSERT_EQ(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, pfn_GetSwapchainStatusAfter(dev.dev, VK_NULL_HANDLE));
}

// Make sure loader does not throw an error for a device layer  that is not present
TEST(TestLayers, DeviceLayerNotPresent) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA, VK_API_VERSION_1_2))
        .set_icd_api_version(VK_API_VERSION_1_2)
        .add_physical_device(PhysicalDevice{}.set_api_version(VK_API_VERSION_1_2).finish());
    const char* explicit_layer_name = "VK_LAYER_LUNARG_wrap_objects";

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();
    VkPhysicalDevice phys_dev = inst.GetPhysDev();

    DeviceWrapper dev{inst};
    dev.create_info.add_layer(explicit_layer_name);
    dev.CheckCreate(phys_dev);
}

TEST(LayerPhysDeviceMod, AddPhysicalDevices) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_add_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_ADD_PHYS_DEV")),
                           "test_layer_add.json");

    auto& layer = env.get_test_layer(0);
    layer.set_add_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_devices = 6;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t dev_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &dev_count, nullptr));
    ASSERT_GT(dev_count, icd_devices);

    auto not_exp_physical_devices = std::vector<VkPhysicalDevice>(icd_devices);
    uint32_t returned_phys_dev_count = icd_devices;
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, not_exp_physical_devices.data()));
    ASSERT_EQ(icd_devices, returned_phys_dev_count);

    auto physical_devices = std::vector<VkPhysicalDevice>(dev_count);
    returned_phys_dev_count = dev_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
    ASSERT_EQ(dev_count, returned_phys_dev_count);

    uint32_t diff_count = dev_count - icd_devices;
    uint32_t found_incomplete = 0;
    uint32_t found_added_count = 0;
    for (uint32_t dev = 0; dev < dev_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        inst->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);
        if (string_eq(props.deviceName, "physdev_added_xx")) {
            found_added_count++;
        }
        for (uint32_t incomp = 0; incomp < icd_devices; ++incomp) {
            if (not_exp_physical_devices[incomp] == physical_devices[dev]) {
                found_incomplete++;
                break;
            }
        }
    }

    // We should have added the number of diff items, and the incomplete count should match the number of
    // original physical devices.
    ASSERT_EQ(found_added_count, diff_count);
    ASSERT_EQ(found_incomplete, icd_devices);
}

TEST(LayerPhysDeviceMod, RemovePhysicalDevices) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_remove_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_REMOVE_PHYS_DEV")),
                           "test_layer_remove.json");

    auto& layer = env.get_test_layer(0);
    layer.set_remove_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_devices = 6;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t dev_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &dev_count, nullptr));
    ASSERT_LT(dev_count, icd_devices);

    auto physical_devices = std::vector<VkPhysicalDevice>(dev_count);
    uint32_t returned_phys_dev_count = dev_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
    ASSERT_EQ(dev_count, returned_phys_dev_count);
}

TEST(LayerPhysDeviceMod, ReorderPhysicalDevices) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_reorder_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_REORDER_PHYS_DEV")),
                           "test_layer_reorder.json");

    auto& layer = env.get_test_layer(0);
    layer.set_reorder_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_devices = 6;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t dev_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &dev_count, nullptr));
    ASSERT_EQ(dev_count, icd_devices);

    auto physical_devices = std::vector<VkPhysicalDevice>(dev_count);
    uint32_t returned_phys_dev_count = dev_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
    ASSERT_EQ(dev_count, returned_phys_dev_count);
}

TEST(LayerPhysDeviceMod, AddRemoveAndReorderPhysicalDevices) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_all_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_ALL_PHYS_DEV")),
                           "test_layer_all.json");

    auto& layer = env.get_test_layer(0);
    layer.set_add_phys_devs(true).set_remove_phys_devs(true).set_reorder_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_devices = 6;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t dev_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &dev_count, nullptr));
    ASSERT_GT(dev_count, icd_devices);

    auto physical_devices = std::vector<VkPhysicalDevice>(dev_count);
    uint32_t returned_phys_dev_count = dev_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDevices(inst, &returned_phys_dev_count, physical_devices.data()));
    ASSERT_EQ(dev_count, returned_phys_dev_count);

    uint32_t found_added_count = 0;
    for (uint32_t dev = 0; dev < dev_count; ++dev) {
        VkPhysicalDeviceProperties props{};
        inst->vkGetPhysicalDeviceProperties(physical_devices[dev], &props);
        if (string_eq(props.deviceName, "physdev_added_xx")) {
            found_added_count++;
        }
    }

    // Should see 2 removed, but 3 added so a diff count of 1
    uint32_t diff_count = dev_count - icd_devices;
    ASSERT_EQ(1U, diff_count);
    ASSERT_EQ(found_added_count, 3U);
}

bool GroupsAreTheSame(VkPhysicalDeviceGroupProperties a, VkPhysicalDeviceGroupProperties b) {
    if (a.physicalDeviceCount != b.physicalDeviceCount) {
        return false;
    }
    for (uint32_t dev = 0; dev < a.physicalDeviceCount; ++dev) {
        if (a.physicalDevices[dev] != b.physicalDevices[dev]) {
            return false;
        }
    }
    return true;
}

TEST(LayerPhysDeviceMod, AddPhysicalDeviceGroups) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_add_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_ADD_PHYS_DEV")),
                           "test_layer_remove.json");

    auto& layer = env.get_test_layer(0);
    layer.set_add_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_groups = 4;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t grp_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &grp_count, nullptr));
    ASSERT_GT(grp_count, icd_groups);

    auto not_exp_phys_dev_groups =
        std::vector<VkPhysicalDeviceGroupProperties>(icd_groups, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});

    uint32_t returned_group_count = icd_groups;
    ASSERT_EQ(VK_INCOMPLETE, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, not_exp_phys_dev_groups.data()));
    ASSERT_EQ(icd_groups, returned_group_count);

    auto phys_dev_groups =
        std::vector<VkPhysicalDeviceGroupProperties>(grp_count, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});

    returned_group_count = grp_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, phys_dev_groups.data()));
    ASSERT_EQ(grp_count, returned_group_count);

    uint32_t diff_count = grp_count - icd_groups;
    uint32_t found_incomplete = 0;
    uint32_t found_added_count = 0;
    for (uint32_t grp = 0; grp < grp_count; ++grp) {
        // Shortcut, only groups with 1 device could be added in the newly added count
        if (1 == phys_dev_groups[grp].physicalDeviceCount) {
            for (uint32_t dev = 0; dev < phys_dev_groups[grp].physicalDeviceCount; ++dev) {
                VkPhysicalDeviceProperties props{};
                inst->vkGetPhysicalDeviceProperties(phys_dev_groups[grp].physicalDevices[dev], &props);
                if (string_eq(props.deviceName, "physdev_added_xx")) {
                    found_added_count++;
                }
            }
        }
        for (uint32_t incomp = 0; incomp < icd_groups; ++incomp) {
            if (GroupsAreTheSame(not_exp_phys_dev_groups[incomp], phys_dev_groups[grp])) {
                found_incomplete++;
                break;
            }
        }
    }

    // We should have added the number of diff items, and the incomplete count should match the number of
    // original physical devices.
    ASSERT_EQ(found_added_count, diff_count);
    ASSERT_EQ(found_incomplete, icd_groups);
}

TEST(LayerPhysDeviceMod, RemovePhysicalDeviceGroups) {
    FrameworkEnvironment env;
    EnvVarWrapper disable_linux_sort("VK_LOADER_DISABLE_SELECT", "1");
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_remove_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_REMOVE_PHYS_DEV")),
                           "test_layer_remove.json");

    auto& layer = env.get_test_layer(0);
    layer.set_remove_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_groups = 3;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t grp_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &grp_count, nullptr));
    ASSERT_EQ(grp_count, icd_groups);

    auto phys_dev_groups =
        std::vector<VkPhysicalDeviceGroupProperties>(grp_count, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});

    uint32_t returned_group_count = grp_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, phys_dev_groups.data()));
    ASSERT_EQ(grp_count, returned_group_count);
}

TEST(LayerPhysDeviceMod, ReorderPhysicalDeviceGroups) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_reorder_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_REORDER_PHYS_DEV")),
                           "test_layer_reorder.json");

    auto& layer = env.get_test_layer(0);
    layer.set_reorder_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_groups = 4;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t grp_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &grp_count, nullptr));
    ASSERT_EQ(grp_count, icd_groups);

    auto phys_dev_groups =
        std::vector<VkPhysicalDeviceGroupProperties>(grp_count, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});

    uint32_t returned_group_count = grp_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, phys_dev_groups.data()));
    ASSERT_EQ(grp_count, returned_group_count);
}

TEST(LayerPhysDeviceMod, AddRemoveAndReorderPhysicalDeviceGroups) {
    FrameworkEnvironment env;
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name("VK_LAYER_LunarG_all_phys_dev")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_api_version(VK_API_VERSION_1_1)
                                                         .set_disable_environment("TEST_DISABLE_ALL_PHYS_DEV")),
                           "test_layer_all.json");

    auto& layer = env.get_test_layer(0);
    layer.set_add_phys_devs(true).set_remove_phys_devs(true).set_reorder_phys_devs(true);

    for (uint32_t icd = 0; icd < 2; ++icd) {
        auto& cur_icd =
            env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2, VK_API_VERSION_1_2)).set_icd_api_version(VK_API_VERSION_1_2);
        VkPhysicalDeviceProperties properties{};
        properties.apiVersion = VK_API_VERSION_1_2;
        properties.vendorID = 0x11000000 + (icd << 6);
        for (uint32_t dev = 0; dev < 3; ++dev) {
            properties.deviceID = properties.vendorID + dev;
            properties.deviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
            auto dev_name = std::string("physdev_") + std::to_string(icd) + "_" + std::to_string(dev);
#if defined(_WIN32)
            strncpy_s(properties.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, dev_name.c_str(), dev_name.length() + 1);
#else
            strncpy(properties.deviceName, dev_name.c_str(), VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
#endif
            cur_icd.add_physical_device({});
            cur_icd.physical_devices.back().set_properties(properties);
        }
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[0]);
        cur_icd.physical_device_groups.back().use_physical_device(cur_icd.physical_devices[1]);
        cur_icd.physical_device_groups.emplace_back(cur_icd.physical_devices[2]);
    }
    const uint32_t icd_groups = 4;

    InstWrapper inst{env.vulkan_functions};
    inst.create_info.set_api_version(VK_API_VERSION_1_1);
    inst.CheckCreate();

    uint32_t grp_count = 0;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &grp_count, nullptr));
    ASSERT_GT(grp_count, icd_groups);

    auto phys_dev_groups =
        std::vector<VkPhysicalDeviceGroupProperties>(grp_count, {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES});

    uint32_t returned_group_count = grp_count;
    ASSERT_EQ(VK_SUCCESS, inst->vkEnumeratePhysicalDeviceGroups(inst, &returned_group_count, phys_dev_groups.data()));
    ASSERT_EQ(grp_count, returned_group_count);

    uint32_t diff_count = grp_count - icd_groups;
    uint32_t found_added_count = 0;
    for (uint32_t grp = 0; grp < grp_count; ++grp) {
        // Shortcut, only groups with 1 device could be added in the newly added count
        if (1 == phys_dev_groups[grp].physicalDeviceCount) {
            for (uint32_t dev = 0; dev < phys_dev_groups[grp].physicalDeviceCount; ++dev) {
                VkPhysicalDeviceProperties props{};
                inst->vkGetPhysicalDeviceProperties(phys_dev_groups[grp].physicalDevices[dev], &props);
                if (string_eq(props.deviceName, "physdev_added_xx")) {
                    found_added_count++;
                }
            }
        }
    }

    // Should see 2 devices removed which should result in 1 group removed and since 3
    // devices were added we should have 3 new groups.  So we should have a diff of 2
    // groups and 3 new groups
    ASSERT_EQ(2U, diff_count);
    ASSERT_EQ(found_added_count, 3U);
}

TEST(TestLayers, AllowFilterWithExplicitLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* layer_name = "VK_LAYER_test_layer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "test_layer_all.json");

    EnvVarWrapper allow{"VK_LOADER_LAYERS_ALLOW", layer_name};
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(false)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

TEST(TestLayers, AllowFilterWithImplicitLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* layer_name = "VK_LAYER_test_layer";
    const char* disable_env_var = "TEST_DISABLE_ENV_VAR";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment(disable_env_var)),
                           "test_layer_all.json");

    EnvVarWrapper allow{"VK_LOADER_LAYERS_ALLOW", layer_name};

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(true)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }

    env.remove_loader_settings();

    // Set the disable_environment variable
    EnvVarWrapper set_disable_env_var{disable_env_var, "1"};

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        // layer's disable_environment takes priority
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(true)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

TEST(TestLayers, AllowFilterWithConditionallyImlicitLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* layer_name = "VK_LAYER_test_layer";
    const char* enable_env_var = "TEST_ENABLE_ENV_VAR";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("TEST_DISABLE_ENV_VAR")
                                                         .set_enable_environment(enable_env_var)),
                           "test_layer_all.json");

    EnvVarWrapper allow{"VK_LOADER_LAYERS_ALLOW", layer_name};

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(true)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }

    env.remove_loader_settings();

    // Repeate the above tests but with the enable_environment variable set
    EnvVarWrapper set_enable_env_var{enable_env_var, "1"};

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(true)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
}

TEST(TestLayers, AllowFilterWithConditionallyImlicitLayerWithOverrideLayer) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* layer_name = "VK_LAYER_test_layer";
    const char* enable_env_var = "TEST_ENABLE_ENV_VAR";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(layer_name)
                                                                          .set_api_version(VK_API_VERSION_1_1)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("TEST_DISABLE_ENV_VAR")
                                                                          .set_enable_environment(enable_env_var)),
                                            "test_layer_all.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 2, 0}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(lunarg_meta_layer_name)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 1, 0))
                                   .add_component_layer(layer_name)
                                   .set_disable_environment("DisableMeIfYouCan")
                                   .add_override_path(env.get_folder(ManifestLocation::override_layer).location().string())),
                           "meta_test_layer.json");

    EnvVarWrapper allow{"VK_LOADER_LAYERS_ALLOW", layer_name};

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, lunarg_meta_layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, lunarg_meta_layer_name));
    }
    {
        EnvVarWrapper disable{"VK_LOADER_LAYERS_DISABLE", layer_name};
        EnvVarWrapper enable{"VK_LOADER_LAYERS_ENABLE", layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, lunarg_meta_layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(layer_name);
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, lunarg_meta_layer_name));
    }
    {
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_treat_as_implicit_manifest(true)));
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
        env.update_loader_settings(env.loader_settings);

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}
