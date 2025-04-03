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

// Makes any failed assertion throw, allowing for graceful cleanup of resources instead of hard aborts
class ThrowListener : public testing::EmptyTestEventListener {
    void OnTestPartResult(const testing::TestPartResult& result) override {
        if (result.type() == testing::TestPartResult::kFatalFailure) {
            // We need to make sure an exception wasn't already thrown so we dont throw another exception at the same time
            std::exception_ptr ex = std::current_exception();
            if (ex) {
                return;
            }
            throw testing::AssertionException(result);
        }
    }
};

int main(int argc, char** argv) {
#if defined(_WIN32)
    // Avoid "Abort, Retry, Ignore" dialog boxes
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
#endif

    // make sure the tests don't find these env-vars if they were set on the system
    EnvVarWrapper vk_icd_filenames_env_var{"VK_ICD_FILENAMES"};
    EnvVarWrapper vk_driver_files_env_var{"VK_DRIVER_FILES"};
    EnvVarWrapper vk_add_driver_files_env_var{"VK_ADD_DRIVER_FILES"};
    EnvVarWrapper vk_layer_path_env_var{"VK_LAYER_PATH"};
    EnvVarWrapper vk_add_layer_path_env_var{"VK_ADD_LAYER_PATH"};
    EnvVarWrapper vk_implicit_layer_path_env_var{"VK_IMPLICIT_LAYER_PATH"};
    EnvVarWrapper vk_add_implicit_layer_path_env_var{"VK_ADD_IMPLICIT_LAYER_PATH"};
    EnvVarWrapper vk_instance_layers_env_var{"VK_INSTANCE_LAYERS"};
    EnvVarWrapper vk_loader_drivers_select_env_var{"VK_LOADER_DRIVERS_SELECT"};
    EnvVarWrapper vk_loader_drivers_disable_env_var{"VK_LOADER_DRIVERS_DISABLE"};
    EnvVarWrapper vk_loader_layers_enable_env_var{"VK_LOADER_LAYERS_ENABLE"};
    EnvVarWrapper vk_loader_layers_disable_env_var{"VK_LOADER_LAYERS_DISABLE"};
    EnvVarWrapper vk_loader_debug_env_var{"VK_LOADER_DEBUG"};
    EnvVarWrapper vk_loader_disable_inst_ext_filter_env_var{"VK_LOADER_DISABLE_INST_EXT_FILTER"};

#if COMMON_UNIX_PLATFORMS
    // Set only one of the 4 XDG variables to /etc, let everything else be empty
    EnvVarWrapper xdg_config_home_env_var{"XDG_CONFIG_HOME", ETC_DIR};
    EnvVarWrapper xdg_config_dirs_env_var{"XDG_CONFIG_DIRS"};
    EnvVarWrapper xdg_data_home_env_var{"XDG_DATA_HOME"};
    EnvVarWrapper xdg_data_dirs_env_var{"XDG_DATA_DIRS"};
    EnvVarWrapper home_env_var{"HOME", HOME_DIR};
#endif
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::UnitTest::GetInstance()->listeners().Append(new ThrowListener);
    int result = RUN_ALL_TESTS();

    return result;
}
