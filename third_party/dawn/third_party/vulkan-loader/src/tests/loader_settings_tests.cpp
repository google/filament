/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
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

#include <fstream>

std::string get_settings_location_log_message([[maybe_unused]] FrameworkEnvironment const& env,
                                              [[maybe_unused]] bool use_secure = false) {
    std::string s = "Using layer configurations found in loader settings from ";
#if defined(WIN32)
    return s + (env.get_folder(ManifestLocation::settings_location).location() / "vk_loader_settings.json").string();
#elif COMMON_UNIX_PLATFORMS
    if (use_secure)
        return s + "/etc/vulkan/loader_settings.d/vk_loader_settings.json";
    else
        return s + "/home/fake_home/.local/share/vulkan/loader_settings.d/vk_loader_settings.json";
#endif
}
enum class LayerType {
    exp,
    imp,
    imp_with_enable_env,
};
const char* add_layer_and_settings(FrameworkEnvironment& env, const char* layer_name, LayerType layer_type, const char* control) {
    if (layer_type == LayerType::imp) {
        env.add_implicit_layer(
            ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                          .set_name(layer_name)
                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                          .set_disable_environment("BADGER" + std::to_string(env.layers.size()))),
            std::string(layer_name) + std::to_string(env.layers.size()) + ".json");
    } else if (layer_type == LayerType::imp_with_enable_env) {
        env.add_implicit_layer(
            ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                          .set_name(layer_name)
                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                          .set_disable_environment("BADGER" + std::to_string(env.layers.size()))
                                          .set_enable_environment("MUSHROOM" + std::to_string(env.layers.size()))),
            std::string(layer_name) + std::to_string(env.layers.size()) + ".json");
    } else if (layer_type == LayerType::exp) {
        env.add_explicit_layer(TestLayerDetails{
            ManifestLayer{}.add_layer(
                ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
            std::string(layer_name) + std::to_string(env.layers.size()) + ".json"});
    } else {
        abort();
    }
    env.loader_settings.app_specific_settings.back().add_layer_configuration(
        LoaderSettingsLayerConfiguration{}
            .set_name(layer_name)
            .set_control(control)
            .set_treat_as_implicit_manifest(layer_type != LayerType::exp)
            .set_path(env.get_shimmed_layer_manifest_path(env.layers.size() - 1)));
    return layer_name;
}

// Make sure settings layer is found and that a layer defined in it is loaded
TEST(SettingsFile, FileExist) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all"));
    const char* regular_layer_name = add_layer_and_settings(env, "VK_LAYER_TestLayer_0", LayerType::exp, "on");
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, regular_layer_name));
    }
}

// Make sure that if the settings file is in a user local path, that it isn't used when running with elevated privileges
TEST(SettingsFile, SettingsInUnsecuredLocation) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* regular_layer_name = "VK_LAYER_TestLayer_0";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                       .set_name(regular_layer_name)
                                                                                       .set_path(env.get_layer_manifest_path())
                                                                                       .set_control("on"))));
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        env.debug_log.clear();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, regular_layer_name));
    }
    env.platform_shim->set_elevated_privilege(true);
    {
        ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

TEST(SettingsFile, SettingsInSecuredLocation) {
    FrameworkEnvironment env{FrameworkSettings{}.set_secure_loader_settings(true)};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    const char* regular_layer_name = "VK_LAYER_TestLayer_0";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                       .set_name(regular_layer_name)
                                                                                       .set_path(env.get_layer_manifest_path())
                                                                                       .set_control("on"))));
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env, true)));
        env.debug_log.clear();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, regular_layer_name));
    }
    env.platform_shim->set_elevated_privilege(true);
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env, true)));
        env.debug_log.clear();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, regular_layer_name));
    }
}

// Make sure settings file can have multiple sets of settings
TEST(SettingsFile, SupportsMultipleSettingsSimultaneously) {
    FrameworkEnvironment env{};
    const char* app_specific_layer_name = "VK_LAYER_TestLayer_0";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(app_specific_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "VK_LAYER_app_specific.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    const char* global_layer_name = "VK_LAYER_TestLayer_1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(global_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "VK_LAYER_global.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    env.update_loader_settings(
        env.loader_settings
            // configuration that matches the current executable path - but dont set the app-key just yet
            .add_app_specific_setting(AppSpecificSettings{}
                                          .add_stderr_log_filter("all")
                                          .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                       .set_name(app_specific_layer_name)
                                                                       .set_path(env.get_layer_manifest_path(0))
                                                                       .set_control("on"))
                                          .add_app_key("key0"))
            // configuration that should never be used
            .add_app_specific_setting(
                AppSpecificSettings{}
                    .add_stderr_log_filter("all")
                    .add_layer_configuration(
                        LoaderSettingsLayerConfiguration{}.set_name("VK_LAYER_haha").set_path("/made/up/path").set_control("auto"))
                    .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                 .set_name("VK_LAYER_haha2")
                                                 .set_path("/made/up/path2")
                                                 .set_control("auto"))
                    .add_app_key("key1")
                    .add_app_key("key2"))
            // Add a global configuration
            .add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
                LoaderSettingsLayerConfiguration{}
                    .set_name(global_layer_name)
                    .set_path(env.get_layer_manifest_path(1))
                    .set_control("on"))));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, global_layer_name));

        // Check that the global config is used
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, global_layer_name));
    }
    env.debug_log.clear();
    // Set one set to contain the current executable path
    env.loader_settings.app_specific_settings.at(0).add_app_key(test_platform_executable_path());
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, app_specific_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, app_specific_layer_name));
    }
}

// Make sure layers found through the settings file are enableable by environment variables
TEST(SettingsFile, LayerAutoEnabledByEnvVars) {
    FrameworkEnvironment env{};
    env.loader_settings.set_file_format_version({1, 0, 0});
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* layer_name = "VK_LAYER_automatic";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "layer_name.json"}
            .set_discovery_type(ManifestDiscoveryType::override_folder));

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}.set_name(layer_name).set_path(env.get_layer_manifest_path(0)).set_control("auto"))));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    {
        EnvVarWrapper instance_layers{"VK_INSTANCE_LAYERS", layer_name};
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
    env.debug_log.clear();

    {
        EnvVarWrapper loader_layers_enable{"VK_LOADER_LAYERS_ENABLE", layer_name};
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, layer_name));
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_name));
    }
}

// Make sure layers are disallowed from loading if the settings file says so
TEST(SettingsFile, LayerDisablesImplicitLayer) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* implicit_layer_name = "VK_LAYER_Implicit_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("oof")),
                           "implicit_test_layer.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("off")
                .set_treat_as_implicit_manifest(true))));
    {
        ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
}

// Implicit layers should be reordered by the settings file
TEST(SettingsFile, ImplicitLayersDontInterfere) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("oof")),
                           "implicit_test_layer1.json");
    const char* implicit_layer_name2 = "VK_LAYER_Implicit_TestLayer2";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("oof")),
                           "implicit_test_layer2.json");
    // validate order when settings file is not present
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name2));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name2));
    }
    // Now setup the settings file to contain a specific order
    env.update_loader_settings(
        LoaderSettings{}.add_app_specific_setting(AppSpecificSettings{}
                                                      .add_stderr_log_filter("all")
                                                      .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                   .set_name(implicit_layer_name1)
                                                                                   .set_path(env.get_shimmed_layer_manifest_path(0))
                                                                                   .set_control("auto")
                                                                                   .set_treat_as_implicit_manifest(true))
                                                      .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                   .set_name(implicit_layer_name2)
                                                                                   .set_path(env.get_shimmed_layer_manifest_path(1))
                                                                                   .set_control("auto")
                                                                                   .set_treat_as_implicit_manifest(true))));
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name2));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name2));
    }

    // Flip the order and store the settings in the env for later use in the test
    env.loader_settings =
        LoaderSettings{}.add_app_specific_setting(AppSpecificSettings{}
                                                      .add_stderr_log_filter("all")
                                                      .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                   .set_name(implicit_layer_name2)
                                                                                   .set_path(env.get_shimmed_layer_manifest_path(1))
                                                                                   .set_control("auto")
                                                                                   .set_treat_as_implicit_manifest(true))
                                                      .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                   .set_name(implicit_layer_name1)
                                                                                   .set_path(env.get_shimmed_layer_manifest_path(0))
                                                                                   .set_control("auto")
                                                                                   .set_treat_as_implicit_manifest(true)));
    env.update_loader_settings(env.loader_settings);

    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name1));
    }

    // Now add an explicit layer into the middle and verify that is in the correct location
    const char* explicit_layer_name3 = "VK_LAYER_Explicit_TestLayer3";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name3).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer3.json");
    env.loader_settings.app_specific_settings.at(0).layer_configurations.insert(
        env.loader_settings.app_specific_settings.at(0).layer_configurations.begin() + 1,
        LoaderSettingsLayerConfiguration{}
            .set_name(explicit_layer_name3)
            .set_path(env.get_shimmed_layer_manifest_path(2))
            .set_control("on"));
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, implicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name3));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, implicit_layer_name1));
    }
}

// Make sure layers that are disabled can't be enabled by the application
TEST(SettingsFile, ApplicationEnablesIgnored) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(explicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("off"))));
    {
        ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(explicit_layer_name);
        ASSERT_NO_FATAL_FAILURE(inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT));
    }
}

TEST(SettingsFile, LayerListIsEmpty) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* implicit_layer_name = "VK_LAYER_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("HeeHee")),
                           "regular_test_layer.json");

    JsonWriter writer{};
    writer.StartObject();
    writer.AddKeyedString("file_format_version", "1.0.0");
    writer.StartKeyedObject("settings");
    writer.StartKeyedObject("layers");
    writer.EndObject();
    writer.EndObject();
    writer.EndObject();
    env.write_settings_file(writer.output);

    ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
}

// If a settings file exists but contains no valid settings - don't consider it
TEST(SettingsFile, InvalidSettingsFile) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* explicit_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json");
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("foobarbaz")),
                           "implicit_test_layer.json");
    auto check_integrity = [&env, explicit_layer_name, implicit_layer_name]() {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name));
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name));
    };

    {
        std::fstream fuzzer_output_json_file{FUZZER_OUTPUT_JSON_FILE, std::ios_base::in};
        ASSERT_TRUE(fuzzer_output_json_file.is_open());
        std::stringstream fuzzer_output_json;
        fuzzer_output_json << fuzzer_output_json_file.rdbuf();
        env.write_settings_file(fuzzer_output_json.str());

        check_integrity();
    }

    // No actual settings
    {
        JsonWriter writer{};
        writer.StartObject();
        writer.AddKeyedString("file_format_version", "0.0.0");
        writer.EndObject();
        env.write_settings_file(writer.output);

        check_integrity();
    }

    {
        JsonWriter writer{};
        writer.StartObject();
        writer.AddKeyedString("file_format_version", "0.0.0");
        writer.StartKeyedArray("settings_array");
        writer.EndArray();
        writer.StartKeyedObject("settings");
        writer.EndObject();
        writer.EndObject();
        env.write_settings_file(writer.output);

        check_integrity();
    }

    {
        JsonWriter writer{};
        writer.StartObject();
        for (uint32_t i = 0; i < 3; i++) {
            writer.StartKeyedArray("settings_array");
            writer.EndArray();
            writer.StartKeyedObject("boogabooga");
            writer.EndObject();
            writer.StartKeyedObject("settings");
            writer.EndObject();
        }
        writer.EndObject();
        env.write_settings_file(writer.output);

        check_integrity();
    }
}

// Unknown layers are put in the correct location
TEST(SettingsFile, UnknownLayersInRightPlace) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* explicit_layer_name1 = "VK_LAYER_TestLayer1";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer1.json");
    const char* implicit_layer_name1 = "VK_LAYER_ImplicitTestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("foobarbaz")),
                           "implicit_test_layer1.json");
    const char* explicit_layer_name2 = "VK_LAYER_TestLayer2";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer2.json");
    const char* implicit_layer_name2 = "VK_LAYER_ImplicitTestLayer2";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("foobarbaz")),
                           "implicit_test_layer2.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name2)
                                         .set_path(env.get_shimmed_layer_manifest_path(2))
                                         .set_control("on"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(implicit_layer_name2)
                                         .set_path(env.get_shimmed_layer_manifest_path(3))
                                         .set_control("on")
                                         .set_treat_as_implicit_manifest(true))));

    auto layer_props = env.GetLayerProperties(4);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name2));
    ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name1));
    ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));
    ASSERT_TRUE(string_eq(layer_props.at(3).layerName, implicit_layer_name2));
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.create_info.add_layer(explicit_layer_name1);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name2));
    ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name1));
    ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));
    ASSERT_TRUE(string_eq(layer_props.at(3).layerName, implicit_layer_name2));
}

// Settings file allows loading multiple layers with the same name - as long as the path is different
TEST(SettingsFile, MultipleLayersWithSameName) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name)
                                                         .set_description("0000")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                           "regular_test_layer1.json");

    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(explicit_layer_name)
                                                         .set_description("1111")
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                           "regular_test_layer2.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("on"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(1))
                                         .set_control("on"))));
    auto layer_props = env.GetLayerProperties(2);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name));
    ASSERT_TRUE(string_eq(layer_props.at(0).description, "0000"));
    ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name));
    ASSERT_TRUE(string_eq(layer_props.at(1).description, "1111"));
    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
    ASSERT_TRUE(string_eq(layers.at(0).description, "0000"));
    ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name));
    ASSERT_TRUE(string_eq(layers.at(1).description, "1111"));
}

// Settings file shouldn't be able to cause the same layer from the same path twice
TEST(SettingsFile, MultipleLayersWithSamePath) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer1.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("on"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("on"))));

    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
}

// Settings contains a layer whose name doesn't match the one found in the layer manifest - make sure the layer from the settings
// file is removed
TEST(SettingsFile, MismatchedLayerNameAndManifestPath) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* manifest_explicit_layer_name = "VK_LAYER_MANIFEST_TestLayer";
    const char* settings_explicit_layer_name = "VK_LAYER_Settings_TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(manifest_explicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                           "regular_test_layer1.json");

    const char* implicit_layer_name = "VK_LAYER_Implicit_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("oof")),
                           "implicit_test_layer.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(settings_explicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("on"))));

    ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
}

// Settings file should take precedence over the meta layer, if present
TEST(SettingsFile, ImplicitLayerWithEnableEnvironment) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_1 = "VK_LAYER_Regular_TestLayer";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json");

    const char* implicit_layer_1 = "VK_LAYER_RegularImplicit_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("AndISaidHey")
                                                         .set_enable_environment("WhatsGoingOn")),
                           "implicit_layer1.json");

    const char* explicit_layer_2 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer2.json"));

    const char* implicit_layer_2 = "VK_LAYER_RegularImplicit_TestLayer2";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(implicit_layer_2)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("HeyHeyHeyyaya")
                                                                          .set_enable_environment("HeyHeyHeyhey")),
                                            "implicit_layer2.json"));
    const char* explicit_layer_3 = "VK_LAYER_Regular_TestLayer3";
    env.add_explicit_layer(TestLayerDetails(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_3).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer3.json"));
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_1)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(false))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(implicit_layer_1)
                                         .set_path(env.get_shimmed_layer_manifest_path(1))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(true))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_2)
                                         .set_path(env.get_shimmed_layer_manifest_path(2))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(false))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(implicit_layer_2)
                                         .set_path(env.get_shimmed_layer_manifest_path(3))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(true))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_3)
                                         .set_path(env.get_shimmed_layer_manifest_path(4))
                                         .set_control("on")
                                         .set_treat_as_implicit_manifest(false))));
    {
        auto layer_props = env.GetLayerProperties(5);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(3).layerName, implicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(4).layerName, explicit_layer_3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_3));
    }
    {
        EnvVarWrapper enable_meta_layer{"WhatsGoingOn", "1"};
        auto layer_props = env.GetLayerProperties(5);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(3).layerName, implicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(4).layerName, explicit_layer_3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_3));
    }
    {
        EnvVarWrapper enable_meta_layer{"HeyHeyHeyhey", "1"};
        auto layer_props = env.GetLayerProperties(5);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(3).layerName, implicit_layer_2));
        ASSERT_TRUE(string_eq(layer_props.at(4).layerName, explicit_layer_3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_2));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_3));
    }
}

// Settings file should take precedence over the meta layer, if present
TEST(SettingsFile, MetaLayerAlsoActivates) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* settings_explicit_layer_name = "VK_LAYER_Regular_TestLayer";
    env.add_explicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(settings_explicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                           "explicit_test_layer.json");

    const char* settings_implicit_layer_name = "VK_LAYER_RegularImplicit_TestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(settings_implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("AndISaidHey")
                                                         .set_enable_environment("WhatsGoingOn")),
                           "implicit_layer.json");

    const char* component_explicit_layer_name1 = "VK_LAYER_Component_TestLayer1";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(component_explicit_layer_name1)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "component_test_layer1.json"));

    const char* component_explicit_layer_name2 = "VK_LAYER_Component_TestLayer2";
    env.add_explicit_layer(TestLayerDetails(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(component_explicit_layer_name2)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "component_test_layer2.json"));

    const char* meta_layer_name1 = "VK_LAYER_meta_layer1";
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(meta_layer_name1)
                                                                         .add_component_layer(component_explicit_layer_name2)
                                                                         .add_component_layer(component_explicit_layer_name1)
                                                                         .set_disable_environment("NotGonnaWork")),
        "meta_test_layer.json");

    const char* meta_layer_name2 = "VK_LAYER_meta_layer2";
    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(ManifestLayer::LayerDescription{}
                                                                         .set_name(meta_layer_name2)
                                                                         .add_component_layer(component_explicit_layer_name1)
                                                                         .set_disable_environment("ILikeTrains")
                                                                         .set_enable_environment("BakedBeans")),
        "not_automatic_meta_test_layer.json");

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(settings_explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("on")
                                         .set_treat_as_implicit_manifest(false))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(settings_implicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path(1))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(true))));
    {
        EnvVarWrapper enable_meta_layer{"WhatsGoingOn", "1"};
        auto layer_props = env.GetLayerProperties(6);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, settings_explicit_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, meta_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, meta_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(3).layerName, component_explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(4).layerName, component_explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(5).layerName, settings_implicit_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 5);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, settings_explicit_layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, component_explicit_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, component_explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(3).layerName, meta_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(4).layerName, settings_implicit_layer_name));
    }
    {
        EnvVarWrapper enable_meta_layer{"BakedBeans", "1"};
        auto layer_props = env.GetLayerProperties(6);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, settings_explicit_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, meta_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, meta_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(3).layerName, component_explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(4).layerName, component_explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(5).layerName, settings_implicit_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 5);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, settings_explicit_layer_name));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, component_explicit_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, component_explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(3).layerName, meta_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(4).layerName, meta_layer_name2));
    }
}

// Layers are correctly ordered by settings file.
TEST(SettingsFile, LayerOrdering) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json");

    const char* explicit_layer_name2 = "VK_LAYER_Regular_TestLayer2";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer2.json");

    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Domierigato")),
                           "implicit_layer1.json");

    const char* implicit_layer_name2 = "VK_LAYER_Implicit_TestLayer2";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name2)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Mistehrobato")),
                           "implicit_layer2.json");

    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all"));

    std::vector<LoaderSettingsLayerConfiguration> layer_configs{4};
    layer_configs.at(0)
        .set_name(explicit_layer_name1)
        .set_path(env.get_shimmed_layer_manifest_path(0))
        .set_control("on")
        .set_treat_as_implicit_manifest(false);
    layer_configs.at(1)
        .set_name(explicit_layer_name2)
        .set_path(env.get_shimmed_layer_manifest_path(1))
        .set_control("on")
        .set_treat_as_implicit_manifest(false);
    layer_configs.at(2)
        .set_name(implicit_layer_name1)
        .set_path(env.get_shimmed_layer_manifest_path(2))
        .set_control("on")
        .set_treat_as_implicit_manifest(true);
    layer_configs.at(3)
        .set_name(implicit_layer_name2)
        .set_path(env.get_shimmed_layer_manifest_path(3))
        .set_control("on")
        .set_treat_as_implicit_manifest(true);

    std::sort(layer_configs.begin(), layer_configs.end());
    uint32_t permutation_count = 0;
    do {
        env.loader_settings.app_specific_settings.at(0).layer_configurations.clear();
        env.loader_settings.app_specific_settings.at(0).add_layer_configurations(layer_configs);
        env.update_loader_settings(env.loader_settings);

        auto layer_props = env.GetLayerProperties(4);
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_TRUE(layer_configs.at(i).name == layer_props.at(i).layerName);
        }

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto active_layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        for (uint32_t i = 0; i < 4; i++) {
            ASSERT_TRUE(layer_configs.at(i).name == active_layers.at(i).layerName);
        }
        env.debug_log.clear();
        permutation_count++;
    } while (std::next_permutation(layer_configs.begin(), layer_configs.end()));
    ASSERT_EQ(permutation_count, 24U);  // should be this many orderings
}

TEST(SettingsFile, EnvVarsWork_VK_LAYER_PATH) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"}
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Domierigato")),
                           "implicit_layer1.json");
    const char* non_env_var_layer_name2 = "VK_LAYER_Regular_TestLayer2";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(non_env_var_layer_name2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer2.json"});

    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name1));
    }
    env.update_loader_settings(env.loader_settings.add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(non_env_var_layer_name2)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(2)))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"))));
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, explicit_layer_name1));
    }
}

TEST(SettingsFile, EnvVarsWork_VK_ADD_LAYER_PATH) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Domierigato")),
                           "implicit_layer1.json");
    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"}
                               .set_discovery_type(ManifestDiscoveryType::add_env_var));
    const char* non_env_var_layer_name2 = "VK_LAYER_Regular_TestLayer2";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(non_env_var_layer_name2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer2.json"});

    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, non_env_var_layer_name2));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name1));
    }

    env.update_loader_settings(env.loader_settings.add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name1)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(1)))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(non_env_var_layer_name2)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(2)))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(implicit_layer_name1)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_treat_as_implicit_manifest(true))));
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, implicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, non_env_var_layer_name2));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, implicit_layer_name1));
    }
}

TEST(SettingsFile, EnvVarsWork_VK_IMPLICIT_LAYER_PATH) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"}
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Domierigato")),
                           "implicit_layer1.json");
    const char* settings_layer_path = "VK_LAYER_Regular_TestLayer2";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(settings_layer_path)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("Domierigato")),
                                            "implicit_test_layer2.json"});

    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, settings_layer_path));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_path));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_path));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, explicit_layer_name1));
    }
    env.update_loader_settings(env.loader_settings.add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(settings_layer_path)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(2))
                                         .set_treat_as_implicit_manifest(true))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"))));
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, settings_layer_path));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, settings_layer_path));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, settings_layer_path));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, explicit_layer_name1));
    }
}

TEST(SettingsFile, EnvVarsWork_VK_ADD_IMPLICIT_LAYER_PATH) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* implicit_layer_name1 = "VK_LAYER_Implicit_TestLayer1";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name1)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("Domierigato")),
                           "implicit_layer1.json");
    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"}
                               .set_discovery_type(ManifestDiscoveryType::add_env_var));
    const char* settings_layer_name = "VK_LAYER_Regular_TestLayer2";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(settings_layer_name)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                                          .set_disable_environment("gozaimasu")),
                                            "implicit_test_layer2.json"});

    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, settings_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_name));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_name));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, explicit_layer_name1));
    }

    env.update_loader_settings(env.loader_settings.add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name1)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(1)))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(settings_layer_name)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(2))
                                         .set_treat_as_implicit_manifest(true))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(implicit_layer_name1)
                                         .set_control("on")
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_treat_as_implicit_manifest(true))));
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, settings_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, implicit_layer_name1));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_name));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, implicit_layer_name1));
    }
    {
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.create_info.add_layer(explicit_layer_name1);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, settings_layer_name));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, implicit_layer_name1));
    }
}

TEST(SettingsFile, EnvVarsWork_VK_INSTANCE_LAYERS) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* filler_layer_name = "VK_LAYER_filler";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(filler_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "filler_layer.json"});

    const char* explicit_layer_name = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});

    {
        EnvVarWrapper vk_instance_layers{"VK_INSTANCE_LAYERS", explicit_layer_name};
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, filler_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layer = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layer.at(0).layerName, explicit_layer_name));
    }
    env.update_loader_settings(env.loader_settings.add_app_specific_setting(
        AppSpecificSettings{}
            .add_stderr_log_filter("all")
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(filler_layer_name)
                                         .set_control("auto")
                                         .set_path(env.get_shimmed_layer_manifest_path(0)))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_control("off")
                                         .set_path(env.get_shimmed_layer_manifest_path(1)))));
    {
        auto layer_props = env.GetLayerProperties(1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, filler_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    EnvVarWrapper vk_instance_layers{"VK_INSTANCE_LAYERS", explicit_layer_name};
    {
        auto layer_props = env.GetLayerProperties(1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, filler_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).control = "auto";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, filler_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).control = "on";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, filler_layer_name));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name));

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
    }
}

TEST(SettingsFile, EnvVarsWork_VK_INSTANCE_LAYERS_multiple_layers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name1 = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});

    const char* explicit_layer_name2 = "VK_LAYER_Regular_TestLayer2";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer2.json"});

    const char* explicit_layer_name3 = "VK_LAYER_Regular_TestLayer3";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name3).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer3.json"});

    EnvVarWrapper vk_instance_layers{"VK_INSTANCE_LAYERS"};
    vk_instance_layers.add_to_list(explicit_layer_name2);
    vk_instance_layers.add_to_list(explicit_layer_name1);
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_FALSE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layer = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layer.at(0).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer.at(1).layerName, explicit_layer_name1));
    }
    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configurations(
        {LoaderSettingsLayerConfiguration{}
             .set_name(explicit_layer_name1)
             .set_control("off")
             .set_path(env.get_shimmed_layer_manifest_path(0)),
         LoaderSettingsLayerConfiguration{}
             .set_name(explicit_layer_name2)
             .set_control("off")
             .set_path(env.get_shimmed_layer_manifest_path(1)),
         LoaderSettingsLayerConfiguration{}
             .set_name(explicit_layer_name3)
             .set_control("auto")
             .set_path(env.get_shimmed_layer_manifest_path(2))}));
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(1);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).control = "auto";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).control = "on";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).control = "off";
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).control = "auto";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();
        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name2));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).control = "on";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name2));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).control = "auto";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name2));
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).control = "on";
    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(3);
        ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layer_props.at(1).layerName, explicit_layer_name2));
        ASSERT_TRUE(string_eq(layer_props.at(2).layerName, explicit_layer_name3));

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name1));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, explicit_layer_name2));
    }
}

// Make sure that layers disabled by settings file aren't enabled by VK_LOADER_LAYERS_ENABLE
TEST(SettingsFile, EnvVarsWork_VK_LOADER_LAYERS_ENABLE) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all"));
    const char* explicit_layer_name = add_layer_and_settings(env, "VK_LAYER_Regular_TestLayer1", LayerType::exp, "off");
    env.update_loader_settings(env.loader_settings);

    EnvVarWrapper vk_instance_layers{"VK_LOADER_LAYERS_ENABLE", explicit_layer_name};
    ASSERT_NO_FATAL_FAILURE(env.GetLayerProperties(0));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
}

TEST(SettingsFile, settings_disable_layer_enabled_by_env_vars) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all"));
    std::vector<const char*> layer_names;
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_alpha", LayerType::imp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_bravo", LayerType::imp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_charlie", LayerType::imp, "auto"));
    env.loader_settings.app_specific_settings.back().add_layer_configuration(
        LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_delta", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_echo", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_foxtrot", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_gamma", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_indigo", LayerType::exp, "auto"));
    auto disable_layer_name = add_layer_and_settings(env, "VK_LAYER_juniper", LayerType::exp, "off");
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_kangaroo", LayerType::exp, "on"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_lima", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_mango", LayerType::exp, "auto"));

    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper vk_instance_layers{"VK_INSTANCE_LAYERS", disable_layer_name};
        auto layer_props = env.GetLayerProperties(11);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        ASSERT_TRUE(string_eq(layer_names.at(0), layers.at(0).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(1), layers.at(1).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(2), layers.at(2).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(8), layers.at(3).layerName));
    }
    {
        EnvVarWrapper vk_instance_layers{"VK_LOADER_LAYERS_ENABLE", disable_layer_name};
        auto layer_props = env.GetLayerProperties(11);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        ASSERT_TRUE(string_eq(layer_names.at(0), layers.at(0).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(1), layers.at(1).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(2), layers.at(2).layerName));
        ASSERT_TRUE(string_eq(layer_names.at(8), layers.at(3).layerName));
    }
    {
        auto layer_props = env.GetLayerProperties(11);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(disable_layer_name);
        inst.CheckCreate(VK_ERROR_LAYER_NOT_PRESENT);
    }
}

// Make sure that layers enabled by settings file aren't disabled by VK_LOADER_LAYERS_ENABLE
TEST(SettingsFile, EnvVarsWork_VK_LOADER_LAYERS_DISABLE) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name = "VK_LAYER_Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(explicit_layer_name)
                .set_control("on")
                .set_path(env.get_shimmed_layer_manifest_path(0)))));

    EnvVarWrapper vk_instance_layers{"VK_LOADER_LAYERS_DISABLE", explicit_layer_name};
    auto layer_props = env.GetLayerProperties(1);
    ASSERT_TRUE(string_eq(layer_props.at(0).layerName, explicit_layer_name));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();
    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, explicit_layer_name));
}

#if defined(WIN32)
TEST(SettingsFile, MultipleKeysInRegistryInUnsecureLocation) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.platform_shim->add_unsecured_manifest(ManifestCategory::settings, "jank_path");
    env.platform_shim->add_unsecured_manifest(ManifestCategory::settings, "jank_path2");

    const char* regular_layer_name = "VK_LAYER_TestLayer_0";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                       .set_name(regular_layer_name)
                                                                                       .set_path(env.get_layer_manifest_path())
                                                                                       .set_control("on"))));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    auto layer_props = env.GetLayerProperties(1);
    EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

    InstWrapper inst{env.vulkan_functions};
    FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
    inst.CheckCreate();

    ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
    auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, regular_layer_name));
}

TEST(SettingsFile, MultipleKeysInRegistryInSecureLocation) {
    FrameworkEnvironment env{FrameworkSettings{}.set_secure_loader_settings(true)};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.platform_shim->add_manifest(ManifestCategory::settings, "jank_path");
    env.platform_shim->add_manifest(ManifestCategory::settings, "jank_path2");

    const char* regular_layer_name = "VK_LAYER_TestLayer_0";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_test_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::override_folder));
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                       .set_name(regular_layer_name)
                                                                                       .set_path(env.get_layer_manifest_path())
                                                                                       .set_control("on"))));
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));

    // Make sure it works if the settings file is in the HKEY_LOCAL_MACHINE
    env.platform_shim->set_elevated_privilege(true);
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2));
    {
        auto layer_props = env.GetLayerProperties(1);
        EXPECT_TRUE(string_eq(layer_props.at(0).layerName, regular_layer_name));

        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, regular_layer_name));
    }
}
#endif

// Preinstance functions respect the settings file
TEST(SettingsFile, PreInstanceFunctions) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment("DISABLE_ME")
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceVersion")
                                               .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
        "implicit_test_layer.json");

    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
        LoaderSettingsLayerConfiguration{}
            .set_name(implicit_layer_name)
            .set_control("on")
            .set_path(env.get_shimmed_layer_manifest_path(0))
            .set_treat_as_implicit_manifest(true)));
    env.update_loader_settings(env.loader_settings);
    {
        auto& layer = env.get_test_layer(0);
        // Check layer props
        uint32_t layer_props = 43;
        layer.set_reported_layer_props(layer_props);

        uint32_t count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
        ASSERT_EQ(count, layer_props);

        // check extension props
        uint32_t ext_props = 52;
        layer.set_reported_extension_props(ext_props);
        count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        ASSERT_EQ(count, ext_props);

        // check version
        uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
        layer.set_reported_instance_version(layer_version);

        uint32_t version = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
        ASSERT_EQ(version, layer_version);
    }
    // control is set to off
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("off");
    env.update_loader_settings(env.loader_settings);

    {
        auto& layer = env.get_test_layer(0);
        // Check layer props
        uint32_t layer_props = 43;
        layer.set_reported_layer_props(layer_props);

        uint32_t count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
        ASSERT_EQ(count, 0U);  // dont use the intercepted count

        // check extension props
        uint32_t ext_props = 52;
        layer.set_reported_extension_props(ext_props);
        count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        ASSERT_EQ(count, 4U);  // dont use the intercepted count - use default count

        // check version
        uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
        layer.set_reported_instance_version(layer_version);

        uint32_t version = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
        ASSERT_EQ(version, VK_HEADER_VERSION_COMPLETE);
    }

    // control is set to auto
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(0).set_control("auto");
    env.update_loader_settings(env.loader_settings);

    {
        auto& layer = env.get_test_layer(0);
        // Check layer props
        uint32_t layer_props = 43;
        layer.set_reported_layer_props(layer_props);

        uint32_t count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
        ASSERT_EQ(count, layer_props);

        // check extension props
        uint32_t ext_props = 52;
        layer.set_reported_extension_props(ext_props);
        count = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        ASSERT_EQ(count, ext_props);

        // check version
        uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
        layer.set_reported_instance_version(layer_version);

        uint32_t version = 0;
        ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
        ASSERT_EQ(version, layer_version);
    }
}

// If an implicit layer's disable environment variable is set, but the settings file says to turn the layer on, the layer should be
// activated.
TEST(SettingsFile, ImplicitLayerDisableEnvironmentVariableOverriden) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA)).add_physical_device({});

    const char* filler_layer_name = "VK_LAYER_filler";
    env.add_explicit_layer(
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(filler_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer.json");

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";
    env.add_implicit_layer(ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                         .set_name(implicit_layer_name)
                                                         .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                                         .set_disable_environment("DISABLE_ME")
                                                         .set_enable_environment("ENABLE_ME")),
                           "implicit_test_layer.json");
    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configurations(
        {LoaderSettingsLayerConfiguration{}
             .set_name(filler_layer_name)
             .set_path(env.get_shimmed_layer_manifest_path(0))
             .set_control("auto")
             .set_treat_as_implicit_manifest(false),
         LoaderSettingsLayerConfiguration{}
             .set_name(implicit_layer_name)
             .set_path(env.get_shimmed_layer_manifest_path(1))
             .set_control("auto")
             .set_treat_as_implicit_manifest(true)}));

    auto check_log_for_insert_instance_layer_string = [&env, implicit_layer_name](bool check_for_enable) {
        {
            InstWrapper inst{env.vulkan_functions};
            FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
            inst.CheckCreate();
            if (check_for_enable) {
                ASSERT_TRUE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
                auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
                ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name));
            } else {
                ASSERT_FALSE(env.debug_log.find(std::string("Insert instance layer \"") + implicit_layer_name));
                ASSERT_NO_FATAL_FAILURE(inst.GetActiveLayers(inst.GetPhysDev(), 0));
            }
        }
        env.debug_log.clear();
    };

    // control is set to on
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).set_control("on");
    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper enable_env_var{"ENABLE_ME"};
        EnvVarWrapper disable_env_var{"DISABLE_ME"};

        auto layers = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layers[0].layerName, filler_layer_name));
        ASSERT_TRUE(string_eq(layers[1].layerName, implicit_layer_name));

        check_log_for_insert_instance_layer_string(true);

        enable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(true);

        enable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(true);

        enable_env_var.remove_value();

        disable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(true);

        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(true);

        enable_env_var.set_new_value("1");
        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(true);
    }

    // control is set to off
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).set_control("off");
    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper enable_env_var{"ENABLE_ME"};
        EnvVarWrapper disable_env_var{"DISABLE_ME"};

        auto layers = env.GetLayerProperties(1);
        ASSERT_TRUE(string_eq(layers[0].layerName, filler_layer_name));

        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(false);

        enable_env_var.remove_value();

        disable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(false);

        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("1");
        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(false);
    }

    // control is set to auto
    env.loader_settings.app_specific_settings.at(0).layer_configurations.at(1).set_control("auto");
    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper enable_env_var{"ENABLE_ME"};
        EnvVarWrapper disable_env_var{"DISABLE_ME"};

        auto layers = env.GetLayerProperties(2);
        ASSERT_TRUE(string_eq(layers[0].layerName, filler_layer_name));
        ASSERT_TRUE(string_eq(layers[1].layerName, implicit_layer_name));

        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(true);

        enable_env_var.remove_value();

        disable_env_var.set_new_value("0");
        check_log_for_insert_instance_layer_string(false);

        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(false);

        enable_env_var.set_new_value("1");
        disable_env_var.set_new_value("1");
        check_log_for_insert_instance_layer_string(false);
    }
}

TEST(SettingsFile, ImplicitLayersNotAccidentallyEnabled) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all"));
    std::vector<const char*> layer_names;

    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_alpha", LayerType::imp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_bravo", LayerType::imp_with_enable_env, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_charlie", LayerType::imp_with_enable_env, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_delta", LayerType::imp_with_enable_env, "auto"));

    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_echo", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_foxtrot", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_gamma", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_indigo", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_juniper", LayerType::exp, "auto"));

    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_kangaroo", LayerType::exp, "on"));

    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_lima", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_mango", LayerType::exp, "auto"));
    layer_names.push_back(add_layer_and_settings(env, "VK_LAYER_niagara", LayerType::exp, "auto"));

    env.update_loader_settings(env.loader_settings);
    {
        auto layer_props = env.GetLayerProperties(13);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_names.at(0)));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, layer_names.at(9)));
    }

    {
        EnvVarWrapper env_var{"MUSHROOM1", "1"};
        auto layer_props = env.GetLayerProperties(13);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_names.at(0)));
        ASSERT_TRUE(string_eq(layers.at(1).layerName, layer_names.at(1)));
        ASSERT_TRUE(string_eq(layers.at(2).layerName, layer_names.at(9)));
    }
    {
        EnvVarWrapper env_var{"BADGER0", "1"};
        auto layer_props = env.GetLayerProperties(13);
        for (size_t i = 0; i < layer_props.size(); i++) {
            ASSERT_TRUE(string_eq(layer_names.at(i), layer_props.at(i).layerName));
        }
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        ASSERT_TRUE(string_eq(layers.at(0).layerName, layer_names.at(9)));
    }
}

TEST(SettingsFile, ImplicitLayersPreInstanceEnumInstLayerProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment("DISABLE_ME")
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))),
        "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t layer_props = 43;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(layer_props);

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, layer_props);
}

TEST(SettingsFile, EnableEnvironmentImplicitLayersPreInstanceEnumInstLayerProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment("DISABLE_ME")
                .set_enable_environment("ENABLE_ME")
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceLayerProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceLayerProperties"))),
        "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t layer_props = 43;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_layer_props(layer_props);

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, nullptr));
    ASSERT_EQ(count, 1U);
    std::array<VkLayerProperties, 1> layers{};
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceLayerProperties(&count, layers.data()));
    ASSERT_EQ(count, 1U);
    ASSERT_TRUE(string_eq(layers.at(0).layerName, implicit_layer_name));
}

TEST(SettingsFile, ImplicitLayersPreInstanceEnumInstExtProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment("DISABLE_ME")
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))),
        "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t ext_props = 52;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_extension_props(ext_props);

    uint32_t count = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    ASSERT_EQ(count, ext_props);
}

TEST(SettingsFile, EnableEnvironmentImplicitLayersPreInstanceEnumInstExtProps) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA));
    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(
        ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
            ManifestLayer::LayerDescription{}
                .set_name(implicit_layer_name)
                .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                .set_disable_environment("DISABLE_ME")
                .set_enable_environment("ENABLE_ME")
                .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                               .set_vk_func("vkEnumerateInstanceExtensionProperties")
                                               .set_override_name("test_preinst_vkEnumerateInstanceExtensionProperties"))),
        "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t ext_props = 52;
    auto& layer = env.get_test_layer(0);
    layer.set_reported_extension_props(ext_props);

    auto extensions = env.GetInstanceExtensions(4);
    EXPECT_TRUE(string_eq(extensions.at(0).extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(1).extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(2).extensionName, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME));
    EXPECT_TRUE(string_eq(extensions.at(3).extensionName, VK_LUNARG_DIRECT_DRIVER_LOADING_EXTENSION_NAME));
}

TEST(SettingsFile, ImplicitLayersPreInstanceVersion) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3));

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(implicit_layer_name)
                                   .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3))
                                   .set_disable_environment("DISABLE_ME")
                                   .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                                                  .set_vk_func("vkEnumerateInstanceVersion")
                                                                  .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
                           "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
    auto& layer = env.get_test_layer(0);
    layer.set_reported_instance_version(layer_version);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, layer_version);
}

TEST(SettingsFile, EnableEnvironmentImplicitLayersPreInstanceVersion) {
    FrameworkEnvironment env;
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2_EXPORT_ICD_GPDPA))
        .add_physical_device({})
        .set_icd_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3));

    const char* implicit_layer_name = "VK_LAYER_ImplicitTestLayer";

    env.add_implicit_layer(ManifestLayer{}.set_file_format_version({1, 1, 2}).add_layer(
                               ManifestLayer::LayerDescription{}
                                   .set_name(implicit_layer_name)
                                   .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)
                                   .set_api_version(VK_MAKE_API_VERSION(0, 1, 2, 3))
                                   .set_disable_environment("DISABLE_ME")
                                   .set_enable_environment("ENABLE_ME")
                                   .add_pre_instance_function(ManifestLayer::LayerDescription::FunctionOverride{}
                                                                  .set_vk_func("vkEnumerateInstanceVersion")
                                                                  .set_override_name("test_preinst_vkEnumerateInstanceVersion"))),
                           "implicit_test_layer.json");

    env.update_loader_settings(
        env.loader_settings.add_app_specific_setting(AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(implicit_layer_name)
                .set_path(env.get_shimmed_layer_manifest_path(0))
                .set_control("auto")
                .set_treat_as_implicit_manifest(true))));

    uint32_t layer_version = VK_MAKE_API_VERSION(1, 2, 3, 4);
    auto& layer = env.get_test_layer(0);
    layer.set_reported_instance_version(layer_version);

    uint32_t version = 0;
    ASSERT_EQ(VK_SUCCESS, env.vulkan_functions.vkEnumerateInstanceVersion(&version));
    ASSERT_EQ(version, VK_HEADER_VERSION_COMPLETE);
}

// Settings can say which filters to use - make sure those are propagated & treated correctly
TEST(SettingsFile, StderrLogFilters) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("")};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* explicit_layer_name = "Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path())
                                         .set_control("on"))
            .add_layer_configuration(
                LoaderSettingsLayerConfiguration{}.set_name("VK_LAYER_missing").set_path("/road/to/nowhere").set_control("on"))));

    std::string expected_output_verbose;
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Configurations count = 2\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---- Layer Configuration [0] ----\n";
    expected_output_verbose += std::string("[Vulkan Loader] DEBUG:          Name: ") + explicit_layer_name + "\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Path: " + env.get_shimmed_layer_manifest_path().string() + "\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Type: Explicit\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Control: on\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---- Layer Configuration [1] ----\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Name: VK_LAYER_missing\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Path: /road/to/nowhere\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Type: Explicit\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Control: on\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---------------------------------\n";

    std::string expected_output_info =
        std::string("[Vulkan Loader] INFO:           ") + get_settings_location_log_message(env) + "\n";

    std::string expected_output_warning = "[Vulkan Loader] WARNING:        Layer name " + std::string(explicit_layer_name) +
                                          " does not conform to naming standard (Policy #LLP_LAYER_3)\n";

    std::string expected_output_error =
        "[Vulkan Loader] ERROR:          loader_get_json: Failed to open JSON file /road/to/nowhere\n";

    env.loader_settings.app_specific_settings.at(0).stderr_log = {"all"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_TRUE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: ERROR | "
            "WARNING | INFO | DEBUG | PERF | DRIVER | LAYER\n"));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"error", "warn", "info", "debug"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_TRUE(
            env.platform_shim->find_in_log("[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard "
                                           "Error: ERROR | WARNING | INFO | DEBUG\n"));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"warn", "info", "debug"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_TRUE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: WARNING | INFO | DEBUG\n"));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"debug"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_TRUE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: DEBUG\n"));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"info"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: INFO\n"));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"warn"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: WARNING\n"));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {"error"};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error: ERROR\n"));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_TRUE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {""};  // Empty string shouldn't be misinterpreted
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error:"));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.platform_shim->clear_logs();
    env.loader_settings.app_specific_settings.at(0).stderr_log = {};  // No string in the log
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        ASSERT_FALSE(env.platform_shim->find_in_log(
            "[Vulkan Loader] DEBUG:          Loader Settings Filters for Logging to Standard Error:"));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_verbose));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_info));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_warning));
        ASSERT_FALSE(env.platform_shim->find_in_log(expected_output_error));
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
}

// Settings can say which filters to use - make sure the lack of this filter works correctly with VK_LOADER_DEBUG
TEST(SettingsFile, StderrLog_NoOutput) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("")};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* explicit_layer_name = "Regular_TestLayer1";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});
    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path())
                                         .set_control("auto"))
            .add_layer_configuration(
                LoaderSettingsLayerConfiguration{}.set_name("VK_LAYER_missing").set_path("/road/to/nowhere").set_control("auto"))));
    env.loader_settings.app_specific_settings.at(0).stderr_log = {""};
    env.update_loader_settings(env.loader_settings);

    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());

        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 0);
        EXPECT_TRUE(active_layer_props.size() == 0);
    }
    // Check if an empty string (vs lack of the stderr_log field) produces correct output
    env.loader_settings.app_specific_settings.at(0).stderr_log = {};
    env.update_loader_settings(env.loader_settings);
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 0);
        EXPECT_TRUE(active_layer_props.size() == 0);
    }

    env.loader_settings.app_specific_settings.at(0).stderr_log.clear();
    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper instance_layers{"VK_INSTANCE_LAYERS", explicit_layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());

        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }

    {
        EnvVarWrapper instance_layers{"VK_LOADER_LAYERS_ENABLE", explicit_layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
    env.loader_settings.app_specific_settings.at(0).stderr_log = {};
    env.update_loader_settings(env.loader_settings);
    {
        EnvVarWrapper instance_layers{"VK_INSTANCE_LAYERS", explicit_layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());

        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }

    {
        EnvVarWrapper instance_layers{"VK_LOADER_LAYERS_ENABLE", explicit_layer_name};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();

        EXPECT_TRUE(env.platform_shim->fputs_stderr_log.empty());
        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, explicit_layer_name));
    }
}

// Settings can say which filters to use - make sure the lack of this filter works correctly with VK_LOADER_DEBUG
TEST(SettingsFile, NoStderr_log_but_VK_LOADER_DEBUG) {
    FrameworkEnvironment env{FrameworkSettings{}.set_log_filter("all")};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});

    const char* explicit_layer_name = "Regular_TestLayer1";

    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(explicit_layer_name).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "explicit_test_layer1.json"});

    env.update_loader_settings(env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(explicit_layer_name)
                                         .set_path(env.get_shimmed_layer_manifest_path())
                                         .set_control("auto"))
            .add_layer_configuration(
                LoaderSettingsLayerConfiguration{}.set_name("VK_LAYER_missing").set_path("/road/to/nowhere").set_control("auto"))));
    env.loader_settings.app_specific_settings.at(0).stderr_log = {};
    env.update_loader_settings(env.loader_settings);

    std::string expected_output_verbose;
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Configurations count = 2\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---- Layer Configuration [0] ----\n";
    expected_output_verbose += std::string("[Vulkan Loader] DEBUG:          Name: ") + explicit_layer_name + "\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Path: " + env.get_shimmed_layer_manifest_path().string() + "\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Type: Explicit\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Control: auto\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---- Layer Configuration [1] ----\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Name: VK_LAYER_missing\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Path: /road/to/nowhere\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Layer Type: Explicit\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          Control: auto\n";
    expected_output_verbose += "[Vulkan Loader] DEBUG:          ---------------------------------\n";

    std::string expected_output_info =
        std::string("[Vulkan Loader] INFO:           ") + get_settings_location_log_message(env) + "\n";

    std::string expected_output_warning =
        "[Vulkan Loader] WARNING:        Layer name Regular_TestLayer1 does not conform to naming standard (Policy #LLP_LAYER_3)\n";

    std::string expected_output_error =
        "[Vulkan Loader] ERROR:          loader_get_json: Failed to open JSON file /road/to/nowhere\n";

    env.platform_shim->clear_logs();
    {
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        EXPECT_TRUE(env.platform_shim->find_in_log(expected_output_verbose));
        EXPECT_TRUE(env.platform_shim->find_in_log(expected_output_info));
        EXPECT_TRUE(env.platform_shim->find_in_log(expected_output_warning));
        EXPECT_TRUE(env.platform_shim->find_in_log(expected_output_error));

        auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 0);
        EXPECT_TRUE(active_layer_props.size() == 0);
    }
}
TEST(SettingsFile, ManyLayersEnabledInManyWays) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    const char* layer1 = "VK_LAYER_layer1";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer1).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "explicit_layer1.json"});
    const char* layer2 = "VK_LAYER_layer2";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer2).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "explicit_layer2.json"});
    const char* layer3 = "VK_LAYER_layer3";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer3).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "explicit_layer3.json"});
    const char* layer4 = "VK_LAYER_layer4";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer4).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "explicit_layer4.json"});
    const char* layer5 = "VK_LAYER_layer5";
    env.add_explicit_layer(
        TestLayerDetails{ManifestLayer{}.add_layer(
                             ManifestLayer::LayerDescription{}.set_name(layer5).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                         "explicit_layer5.json"});

    env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all").add_layer_configurations(
            {LoaderSettingsLayerConfiguration{}.set_name(layer1).set_path(env.get_shimmed_layer_manifest_path(0)).set_control("on"),
             LoaderSettingsLayerConfiguration{}
                 .set_name(layer2)
                 .set_path(env.get_shimmed_layer_manifest_path(1))
                 .set_control("auto"),
             LoaderSettingsLayerConfiguration{}
                 .set_name(layer3)
                 .set_path(env.get_shimmed_layer_manifest_path(2))
                 .set_control("auto"),
             LoaderSettingsLayerConfiguration{}
                 .set_name(layer4)
                 .set_path(env.get_shimmed_layer_manifest_path(3))
                 .set_control("auto"),
             LoaderSettingsLayerConfiguration{}
                 .set_name(layer5)
                 .set_path(env.get_shimmed_layer_manifest_path(4))
                 .set_control("on")}));
    env.update_loader_settings(env.loader_settings);

    EnvVarWrapper vk_instance_layers{"VK_INSTANCE_LAYERS", layer2};
    EnvVarWrapper vk_loader_layers_enable{"VK_LOADER_LAYERS_ENABLE", layer1};

    auto layers = env.GetLayerProperties(5);
    ASSERT_TRUE(string_eq(layers[0].layerName, layer1));
    ASSERT_TRUE(string_eq(layers[1].layerName, layer2));
    ASSERT_TRUE(string_eq(layers[2].layerName, layer3));
    ASSERT_TRUE(string_eq(layers[3].layerName, layer4));
    ASSERT_TRUE(string_eq(layers[4].layerName, layer5));

    InstWrapper inst{env.vulkan_functions};
    inst.CheckCreate();

    auto active_layer_props = inst.GetActiveLayers(inst.GetPhysDev(), 3);
    EXPECT_TRUE(string_eq(active_layer_props.at(0).layerName, layer1));
    EXPECT_TRUE(string_eq(active_layer_props.at(1).layerName, layer2));
    EXPECT_TRUE(string_eq(active_layer_props.at(2).layerName, layer5));
}

// Enough layers exist that arrays need to be resized - make sure that works
TEST(SettingsFile, TooManyLayers) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device({});
    env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}.add_stderr_log_filter("all"));
    std::string layer_name = "VK_LAYER_regular_layer_name_";
    uint32_t layer_count = 40;
    for (uint32_t i = 0; i < layer_count; i++) {
        env.add_explicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                              .set_name(layer_name + std::to_string(i))
                                                                              .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                                layer_name + std::to_string(i) + ".json"}
                                   .set_discovery_type(ManifestDiscoveryType::override_folder));
        env.loader_settings.app_specific_settings.at(0).add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                                                                    .set_name(layer_name + std::to_string(i))
                                                                                    .set_path(env.get_layer_manifest_path(i))
                                                                                    .set_control("on"));
    }
    env.update_loader_settings(env.loader_settings);

    {
        auto layer_props = env.GetLayerProperties(40);
        for (uint32_t i = 0; i < layer_count; i++) {
            std::string expected_layer_name = layer_name + std::to_string(i);
            EXPECT_TRUE(string_eq(layer_props.at(i).layerName, expected_layer_name.c_str()));
        }
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        env.debug_log.clear();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 40);
        for (uint32_t i = 0; i < layer_count; i++) {
            std::string expected_layer_name = layer_name + std::to_string(i);
            EXPECT_TRUE(string_eq(layers.at(i).layerName, expected_layer_name.c_str()));
        }
    }
    env.loader_settings.app_specific_settings.at(0).layer_configurations.clear();

    // Now reverse the order to make sure adding the 'last' layer first works
    for (uint32_t i = 0; i < layer_count; i++) {
        env.loader_settings.app_specific_settings.at(0).add_layer_configuration(
            LoaderSettingsLayerConfiguration{}
                .set_name(layer_name + std::to_string(layer_count - i - 1))
                .set_path(env.get_layer_manifest_path(layer_count - i - 1))
                .set_control("on"));
    }
    env.update_loader_settings(env.loader_settings);

    {
        auto layer_props = env.GetLayerProperties(40);
        for (uint32_t i = 0; i < layer_count; i++) {
            std::string expected_layer_name = layer_name + std::to_string(layer_count - i - 1);
            EXPECT_TRUE(string_eq(layer_props.at(i).layerName, expected_layer_name.c_str()));
        }
        InstWrapper inst{env.vulkan_functions};
        FillDebugUtilsCreateDetails(inst.create_info, env.debug_log);
        inst.CheckCreate();

        ASSERT_TRUE(env.debug_log.find(get_settings_location_log_message(env)));
        env.debug_log.clear();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 40);
        for (uint32_t i = 0; i < layer_count; i++) {
            std::string expected_layer_name = layer_name + std::to_string(layer_count - i - 1);
            EXPECT_TRUE(string_eq(layers.at(i).layerName, expected_layer_name.c_str()));
        }
    }
}

TEST(SettingsFile, EnvVarsWorkTogether) {
    FrameworkEnvironment env{};
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2)).add_physical_device(PhysicalDevice{}.set_deviceName("regular").finish());
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2).set_discovery_type(ManifestDiscoveryType::env_var))
        .add_physical_device(PhysicalDevice{}.set_deviceName("env_var").finish());
    env.add_icd(TestICDDetails(TEST_ICD_PATH_VERSION_2).set_discovery_type(ManifestDiscoveryType::add_env_var))
        .add_physical_device(PhysicalDevice{}.set_deviceName("add_env_var").finish());

    const char* regular_explicit_layer = "VK_LAYER_regular_explicit_layer";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(regular_explicit_layer).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "regular_explicit_layer.json"});
    const char* regular_explicit_layer_settings_file_set_on = "VK_LAYER_regular_explicit_layer_settings_file_set_on";
    env.add_explicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_name(regular_explicit_layer_settings_file_set_on)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_explicit_layer_settings_file_set_on.json"});

    const char* env_var_explicit_layer = "VK_LAYER_env_var_explicit_layer";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(env_var_explicit_layer).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "env_var_explicit_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* add_env_var_explicit_layer = "VK_LAYER_add_env_var_explicit_layer";
    env.add_explicit_layer(TestLayerDetails{
        ManifestLayer{}.add_layer(
            ManifestLayer::LayerDescription{}.set_name(add_env_var_explicit_layer).set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
        "add_env_var_explicit_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::add_env_var));

    const char* regular_implicit_layer = "VK_LAYER_regular_implicit_layer";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_disable_environment("A")
                                                                          .set_name(regular_implicit_layer)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "regular_implicit_layer.json"});

    const char* env_var_implicit_layer = "VK_LAYER_env_var_implicit_layer";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_disable_environment("B")
                                                                          .set_name(env_var_implicit_layer)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "env_var_implicit_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::env_var));

    const char* add_env_var_implicit_layer = "VK_LAYER_add_env_var_implicit_layer";
    env.add_implicit_layer(TestLayerDetails{ManifestLayer{}.add_layer(ManifestLayer::LayerDescription{}
                                                                          .set_disable_environment("C")
                                                                          .set_name(add_env_var_implicit_layer)
                                                                          .set_lib_path(TEST_LAYER_PATH_EXPORT_VERSION_2)),
                                            "add_env_var_implicit_layer.json"}
                               .set_discovery_type(ManifestDiscoveryType::add_env_var));

    // Settings file only contains the one layer it wants enabled
    env.loader_settings.set_file_format_version({1, 0, 0}).add_app_specific_setting(
        AppSpecificSettings{}
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(regular_implicit_layer)
                                         .set_path(env.get_shimmed_layer_manifest_path(4))
                                         .set_control("auto")
                                         .set_treat_as_implicit_manifest(true))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(regular_explicit_layer)
                                         .set_path(env.get_shimmed_layer_manifest_path(0))
                                         .set_control("auto"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}
                                         .set_name(regular_explicit_layer_settings_file_set_on)
                                         .set_path(env.get_shimmed_layer_manifest_path(1))
                                         .set_control("on"))
            .add_layer_configuration(LoaderSettingsLayerConfiguration{}.set_control("unordered_layer_location")));
    env.update_loader_settings(env.loader_settings);

    {  // VK_INSTANCE_LAYERS
        EnvVarWrapper instance_layers{"VK_INSTANCE_LAYERS", regular_explicit_layer};
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(3).layerName, env_var_implicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log(
            "env var 'VK_INSTANCE_LAYERS' defined and adding layers: VK_LAYER_regular_explicit_layer"));
    }
    env.platform_shim->clear_logs();
    {  // VK_LOADER_LAYERS_ENABLE
        EnvVarWrapper env_var_enable{"VK_LOADER_LAYERS_ENABLE", regular_explicit_layer};
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(3).layerName, env_var_implicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log(
            "Layer \"VK_LAYER_regular_explicit_layer\" forced enabled due to env var 'VK_LOADER_LAYERS_ENABLE'"));
    }
    env.platform_shim->clear_logs();
    {                                                                        // VK_LOADER_LAYERS_DISABLE
        EnvVarWrapper env_var_disable{"VK_LOADER_LAYERS_DISABLE", "~all~"};  // ignored by settings file
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 1);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(
            env.platform_shim->find_in_log("Layer \"VK_LAYER_env_var_implicit_layer\" forced disabled because name matches filter "
                                           "of env var 'VK_LOADER_LAYERS_DISABLE'"));
    }
    env.platform_shim->clear_logs();

    {  // VK_LOADER_LAYERS_ALLOW
        EnvVarWrapper env_var_allow{"VK_LOADER_LAYERS_ALLOW", regular_implicit_layer};
        // Allow only makes sense when the disable env-var is also set
        EnvVarWrapper env_var_disable{"VK_LOADER_LAYERS_DISABLE", "~implicit~"};

        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 2);
        // The regular_implicit_layer is set to "auto" so is affected by environment variables
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer_settings_file_set_on));

        EXPECT_TRUE(
            env.platform_shim->find_in_log("Layer \"VK_LAYER_env_var_implicit_layer\" forced disabled because name matches filter "
                                           "of env var 'VK_LOADER_LAYERS_DISABLE'"));
    }
    env.platform_shim->clear_logs();

    {  // VK_LAYER_PATH
        // VK_LAYER_PATH is set by add_explicit_layer()
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(env_var_explicit_layer);
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, env_var_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(3).layerName, env_var_explicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log("Insert instance layer \"VK_LAYER_env_var_explicit_layer\""));
    }
    env.platform_shim->clear_logs();
    {  // VK_IMPLICIT_LAYER_PATH
        // VK_IMPLICIT_LAYER_PATH is set by add_implicit_layer()
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, env_var_implicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log("Insert instance layer \"VK_LAYER_env_var_implicit_layer\""));
    }
    env.platform_shim->clear_logs();
    {  // VK_ADD_LAYER_PATH
        // VK_ADD_LAYER_PATH is set by add_explicit_layer(), but we need to disable VK_LAYER_PATH
        // since VK_LAYER_PATH overrides VK_ADD_LAYER_PATH
        EnvVarWrapper env_var_vk_layer_path{"VK_LAYER_PATH", ""};
        env_var_vk_layer_path.remove_value();
        InstWrapper inst{env.vulkan_functions};
        inst.create_info.add_layer(add_env_var_explicit_layer);
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 4);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, env_var_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(3).layerName, add_env_var_explicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log("Insert instance layer \"VK_LAYER_add_env_var_explicit_layer\""));
    }
    env.platform_shim->clear_logs();
    {  // VK_ADD_IMPLICIT_LAYER_PATH
        // VK_ADD_IMPLICIT_LAYER_PATH is set by add_explicit_layer(), but we need to disable VK_LAYER_PATH
        // since VK_IMPLICIT_LAYER_PATH overrides VK_ADD_IMPLICIT_LAYER_PATH
        EnvVarWrapper env_var_vk_layer_path{"VK_IMPLICIT_LAYER_PATH", ""};
        env_var_vk_layer_path.remove_value();
        InstWrapper inst{env.vulkan_functions};
        inst.CheckCreate();
        auto layers = inst.GetActiveLayers(inst.GetPhysDev(), 3);
        EXPECT_TRUE(string_eq(layers.at(0).layerName, regular_implicit_layer));
        EXPECT_TRUE(string_eq(layers.at(1).layerName, regular_explicit_layer_settings_file_set_on));
        EXPECT_TRUE(string_eq(layers.at(2).layerName, add_env_var_implicit_layer));
        EXPECT_TRUE(env.platform_shim->find_in_log("Insert instance layer \"VK_LAYER_add_env_var_implicit_layer\""));
    }
}
