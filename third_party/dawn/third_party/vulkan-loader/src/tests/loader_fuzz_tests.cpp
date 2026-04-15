/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "framework/test_environment.h"

#include <fstream>

extern "C" {
#include "loader.h"
#include "loader_json.h"
#include "settings.h"
}

void execute_instance_enumerate_fuzzer(std::filesystem::path const& filename) {
    FrameworkEnvironment env{};
    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::implicit_layer, ManifestLocation::implicit_layer, "complex_layer.json");
    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::settings, ManifestLocation::settings_location, "vk_loader_settings.json");

    uint32_t pPropertyCount = 1;
    VkExtensionProperties pProperties = {0};

    env.vulkan_functions.vkEnumerateInstanceExtensionProperties("test_auto", &pPropertyCount, &pProperties);
}
// Common code for execute_instance_create_fuzzer and execute_instance_create_fuzzer_advanced
void execute_instance_create_fuzzer_logic(FrameworkEnvironment& env) {
    EnvVarWrapper enable_all_layers("VK_LOADER_LAYERS_ENABLE", "all");

    VkInstance inst = {0};
    const char* instance_layers[] = {"VK_LAYER_KHRONOS_validation", "VK_LAYER_test_layer_1", "VK_LAYER_test_layer_2"};
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pNext = NULL;
    app.pApplicationName = "TEST_APP";
    app.applicationVersion = 0;
    app.pEngineName = "TEST_ENGINE";
    app.engineVersion = 0;
    app.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo inst_info{};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.pApplicationInfo = &app;
    inst_info.enabledLayerCount = 1;
    inst_info.ppEnabledLayerNames = (const char* const*)instance_layers;
    inst_info.enabledExtensionCount = 0;
    inst_info.ppEnabledExtensionNames = NULL;

    VkResult err = env.vulkan_functions.vkCreateInstance(&inst_info, NULL, &inst);
    if (err != VK_SUCCESS) {
        return;
    }

    env.vulkan_functions.vkDestroyInstance(inst, NULL);
}

void execute_instance_create_fuzzer(std::filesystem::path const& filename) {
    FrameworkEnvironment env{};
    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::implicit_layer, ManifestLocation::unsecured_implicit_layer, "complex_layer.json");
    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::settings, ManifestLocation::unsecured_settings, "vk_loader_settings.json");
    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::icd, ManifestLocation::unsecured_driver, "icd_test.json");

    execute_instance_create_fuzzer_logic(env);
}

void execute_instance_create_advanced_fuzzer(std::filesystem::path const& filename) {
    FrameworkEnvironment env{};

    // The file actually contains three subfiles with their lengths specified in the first 12 bytes of the file.
    auto source_file = std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename;
    std::fstream file{source_file.string(), std::ios_base::in | std::ios_base::binary};
    ASSERT_TRUE(file.is_open());
    std::stringstream file_stream;
    file_stream << file.rdbuf();

    auto data_string = file_stream.str();
    auto data_cstring = data_string.c_str();
    std::array<uint64_t, 3> sections;
    memcpy(sections.data(), data_cstring, sizeof(uint64_t) * 3);
    sections[0] = sections[0] % 40000;
    sections[1] = sections[1] % 40000;
    sections[2] = sections[2] % 40000;

    uint64_t data_index = 3 * sizeof(uint64_t);
    std::string first = data_string.substr(data_index, sections[0]);
    data_index += sections[0];
    std::string second = data_string.substr(data_index, sections[1]);
    data_index += sections[1];
    std::string third = data_string.substr(data_index, sections[2]);
    data_index += sections[2];

    env.platform_shim->fuzz_data.resize(data_string.size() - data_index);
    memcpy(env.platform_shim->fuzz_data.data(), data_cstring + data_index, env.platform_shim->fuzz_data.size());

    env.write_file_from_string(first, ManifestCategory::implicit_layer, ManifestLocation::implicit_layer, "complex_layer.json");
    env.write_file_from_string(second, ManifestCategory::settings, ManifestLocation::settings_location, "vk_loader_settings.json");
    env.write_file_from_string(third, ManifestCategory::settings, ManifestLocation::settings_location, "icd_test.json");

    execute_instance_create_fuzzer_logic(env);
}
void execute_json_load_fuzzer(std::string const& filename) {
    FrameworkEnvironment env{};

    cJSON* json = NULL;
    loader_get_json(NULL, filename.c_str(), &json);

    if (json == NULL) {
        return;
    }
    bool out_of_mem = false;
    char* json_data = loader_cJSON_Print(json, &out_of_mem);

    if (json_data != NULL) {
        free(json_data);
    }

    if (json != NULL) {
        loader_cJSON_Delete(json);
    }
}
void execute_setting_fuzzer(std::filesystem::path const& filename) {
    FrameworkEnvironment env{};

    env.write_file_from_source((std::filesystem::path(CLUSTERFUZZ_TESTCASE_DIRECTORY) / filename).string().c_str(),
                               ManifestCategory::settings, ManifestLocation::settings_location, "vk_loader_settings.json");

    update_global_loader_settings();
    struct loader_layer_list settings_layers = {0};
    bool should_search_for_other_layers = true;
    get_settings_layers(NULL, &settings_layers, &should_search_for_other_layers);
    loader_delete_layer_list_and_properties(NULL, &settings_layers);
}

TEST(BadJsonInput, ClusterFuzzTestCase_5599244505186304) {
    // Doesn't crash with ASAN or UBSAN
    // Doesn't reproducibly crash - instance_create_fuzzer: Abrt in loader_cJSON_Delete
    execute_instance_create_fuzzer("clusterfuzz-testcase-instance_create_fuzzer-5599244505186304");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5126563864051712) {
    // Doesn't crash with ASAN or UBSAN
    // Doesn't reproducibly crash - instance_enumerate_fuzzer: Abrt in loader_cJSON_Delete
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-instance_enumerate_fuzzer-5126563864051712");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6308459683315712) {
    // Doesn't crash with ASAN or UBSAN
    // Doesn't reproducibly crash - instance_enumerate_fuzzer: Null-dereference READ in
    // combine_settings_layers_with_regular_layers
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-instance_enumerate_fuzzer-6308459683315712");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6583684169269248) {
    // Crashes ASAN
    // Nullptr dereference in loader_copy_to_new_str
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-minimized-instance_enumerate_fuzzer-6583684169269248");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6470575830925312) {
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-minimized-instance_enumerate_fuzzer-6470575830925312");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5258042868105216) {
    // Doesn't crash with ASAN or UBSAN
    // Doesn't reproducibly crash - json_load_fuzzer: Abrt in loader_cJSON_Delete
    execute_json_load_fuzzer("clusterfuzz-testcase-json_load_fuzzer-5258042868105216");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5487817455960064) {
    // Doesn't crash with ASAN or UBSAN
    // Doesn't reproducibly crash - json_load_fuzzer: Abrt in std::__Fuzzer::vector<std::__Fuzzer::pair<unsigned int, unsigned
    // short>, std::__
    execute_json_load_fuzzer("clusterfuzz-testcase-json_load_fuzzer-5487817455960064");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4558978302214144) {
    // Does crash with UBSAN and ASAN
    // loader.c:287: VkResult loader_copy_to_new_str(const struct loader_instance *, const char *, char **): Assertion
    // `source_str
    // && dest_str' failed.
    // instance_create_fuzzer: Null-dereference READ in loader_copy_to_new_str
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-4558978302214144");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4568454561071104) {
    // Does crash with UBSAN and ASAN
    // Causes hangs -  instance_create_fuzzer: Timeout in instance_create_fuzzer
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-4568454561071104");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4820577276723200) {
    // Does crash with UBSAN and ASAN
    // instance_create_fuzzer: Crash in printf_common
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-4820577276723200");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5177827962454016) {
    // Does crash with UBSAN and ASAN
    // free(): invalid next size (fast)
    // instance_create_fuzzer: Abrt in instance_create_fuzzer
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-5177827962454016");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5198773675425792) {
    // Does crash with UBSAN and ASAN
    // stack-overflow
    // instance_create_fuzzer: Stack-overflow with empty stacktrace
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-5198773675425792");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5416197367070720) {
    // Does crash with UBSAN and ASAN
    // free(): invalid next size (fast)
    // instance_create_fuzzer: Overwrites-const-input in instance_create_fuzzer
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-5416197367070720");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5494771615137792) {
    // Does crash with UBSAN and ASAN
    // stack-overflow
    // instance_create_fuzzer: Stack-overflow in verify_meta_layer_component_layers
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-5494771615137792");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5801855065915392) {
    // Does crash with ASAN
    // Doesn't crash with UBSAN
    // Causes a leak - instance_create_fuzzer: Direct-leak in print_string_ptr
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-5801855065915392");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6353004288081920) {
    // Does crash with ASAN and UBSAN
    // Stack overflow due to recursive meta layers
    execute_instance_create_fuzzer("clusterfuzz-testcase-minimized-instance_create_fuzzer-6353004288081920");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5817896795701248) {
    execute_instance_create_fuzzer("clusterfuzz-testcase-instance_create_fuzzer-5817896795701248");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6541440380895232) {
    execute_instance_create_fuzzer("clusterfuzz-testcase-instance_create_fuzzer-6541440380895232");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5612556809207808) {
    execute_instance_create_advanced_fuzzer("clusterfuzz-testcase-minimized-instance_create_advanced_fuzzer-5612556809207808");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4788849181261824) {
    execute_instance_create_advanced_fuzzer("clusterfuzz-testcase-minimized-instance_create_advanced_fuzzer-4788849181261824");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6465902356791296) {
    // Does crash with UBSAN
    // Doesn't crash with ASAN
    // Causes an integer overflow - instance_enumerate_fuzzer: Integer-overflow in parse_value
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-minimized-instance_enumerate_fuzzer-6465902356791296");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6740380288876544) {
    // Does crash with ASAN
    execute_instance_enumerate_fuzzer("clusterfuzz-testcase-minimized-instance_enumerate_fuzzer-6740380288876544");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4512865114259456) {
    // Does crash with UBSAN and ASAN
    // malloc(): invalid size (unsorted)
    // json_load_fuzzer: Heap-buffer-overflow in parse_string
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-4512865114259456");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4552015310880768) {
    // Does crash with UBSAN
    // Doesn't crash with ASAN
    // Causes an integer overflow
    // json_load_fuzzer: Integer-overflow in parse_value
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-4552015310880768");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5208693600747520) {
    // Does crash with UBSAN and ASAN
    // Stack overflow
    // json_load_fuzzer: Stack-overflow in print_value
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-5208693600747520");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5347670374612992) {
    // Doesn't crash with ASAN or UBSAN
    // No reported leaks in head, crashes in 1.3.269 & 1.3.250
    // Causes a leak -  json_load_fuzzer: Direct-leak in parse_array
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-5347670374612992");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5392928643547136) {
    // Does crash with UBSAN and ASAN
    // free(): corrupted unsorted chunks
    // json_load_fuzzer: Abrt in std::__Fuzzer::basic_filebuf<char, std::__Fuzzer::char_traits<char>>::~basic_fil
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-5392928643547136");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5636386303049728) {
    // Does crash with UBSAN and ASAN
    // terminate called after throwing an instance of 'std::bad_alloc' what():  std::bad_alloc
    // json_load_fuzzer: Abrt in json_load_fuzzer
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-5636386303049728");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6182254813249536) {
    // Doesn't crash with ASAN or UBSAN
    // No leaks reported in main, 1.3.269, nor 1.3.250
    // Causes a leak - json_load_fuzzer: Indirect-leak in parse_object
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-6182254813249536");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6265355951996928) {
    // Does crash with UBSAN and ASAN
    // json_load_fuzzer: Null-dereference READ in json_load_fuzzer
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-6265355951996928");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6363106126659584) {
    // Does crash with UBSAN and ASAN
    // json_load_fuzzer: Overwrites-const-input in json_load_fuzzer
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-6363106126659584");
}
TEST(BadJsonInput, ClusterFuzzTestCase_6482033715838976) {
    // Does crash with UBSAN and ASAN
    // json_load_fuzzer: Stack-overflow in parse_array
    execute_json_load_fuzzer("clusterfuzz-testcase-minimized-json_load_fuzzer-6482033715838976");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4857714377818112) {
    // Does crash with UBSAN and ASAN
    // settings_fuzzer: Abrt in settings_fuzzer
    execute_setting_fuzzer("clusterfuzz-testcase-minimized-settings_fuzzer-4857714377818112");
}
TEST(BadJsonInput, ClusterFuzzTestCase_5123849246867456) {
    // Doesn't crash with ASAN or UBSAN
    // No leaks reported in main, 1.3.269, nor 1.3.250
    // Causes a leak - settings_fuzzer: Direct-leak in loader_append_layer_property
    execute_setting_fuzzer("clusterfuzz-testcase-minimized-settings_fuzzer-5123849246867456");
}
TEST(BadJsonInput, ClusterFuzzTestCase_4626669072875520) {
    execute_setting_fuzzer("clusterfuzz-testcase-minimized-settings_fuzzer-4626669072875520");
}
