/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#pragma once

#include <string>
#include <vector>

#include "test_defines.h"

// Environment variable list separator - not for filesystem paths
#if defined(_WIN32)
static const char OS_ENV_VAR_LIST_SEPARATOR = ';';
#elif TESTING_COMMON_UNIX_PLATFORMS
static const char OS_ENV_VAR_LIST_SEPARATOR = ':';
#else
#error "Unhandled platform in env_var_wrapper.h!"
#endif

// get_env_var() - returns the contents of env-var `name` as a std::string
// If report_failure is true, then it will log to stderr that it didn't find the env-var
std::string get_env_var(std::string const& name, bool report_failure = true);

// split_env_var_as_list() - split env_var_contents into separate elements using the platform
// specific env-var list separator.
std::vector<std::string> split_env_var_as_list(std::string const& env_var_contents);

/*
 * Wrapper around Environment Variables with common operations
 * Since Environment Variables leak between tests, there needs to be RAII code to remove them during test cleanup

 */

// Wrapper to set & remove env-vars automatically
struct EnvVarWrapper {
    // Constructor which unsets the env-var
    EnvVarWrapper(std::string const& name) noexcept;
    // Constructor which set the env-var to the specified value
    EnvVarWrapper(std::string const& name, std::string const& value) noexcept;
    ~EnvVarWrapper() noexcept;

    // delete copy operators
    EnvVarWrapper(const EnvVarWrapper&) = delete;
    EnvVarWrapper& operator=(const EnvVarWrapper&) = delete;

    void set_new_value(std::string const& value);
    void add_to_list(std::string const& list_item);
#if defined(_WIN32)
    void add_to_list(std::wstring const& list_item);
#endif
    void remove_value() const;
    const char* get() const;
    const char* value() const;

   private:
    std::string name;
    std::string cur_value;
    std::string initial_value;

    void set_env_var() const;
    void remove_env_var() const;
};
