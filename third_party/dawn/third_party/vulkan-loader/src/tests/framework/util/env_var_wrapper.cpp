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

#include "env_var_wrapper.h"

#include <stdlib.h>

#include <iostream>

#include "wide_char_handling.h"

// NOTE: get_env_var() are only intended for test framework code, all test code MUST use
// EnvVarWrapper

#if defined(_WIN32)
#include <Windows.h>
#include <wchar.h>
#include <strsafe.h>

// Windows specific error handling logic
const long ERROR_SETENV_FAILED = 10543;           // chosen at random, attempts to not conflict
const long ERROR_REMOVEDIRECTORY_FAILED = 10544;  // chosen at random, attempts to not conflict
const char* win_api_error_str(LSTATUS status) {
    if (status == ERROR_INVALID_FUNCTION) return "ERROR_INVALID_FUNCTION";
    if (status == ERROR_FILE_NOT_FOUND) return "ERROR_FILE_NOT_FOUND";
    if (status == ERROR_PATH_NOT_FOUND) return "ERROR_PATH_NOT_FOUND";
    if (status == ERROR_TOO_MANY_OPEN_FILES) return "ERROR_TOO_MANY_OPEN_FILES";
    if (status == ERROR_ACCESS_DENIED) return "ERROR_ACCESS_DENIED";
    if (status == ERROR_INVALID_HANDLE) return "ERROR_INVALID_HANDLE";
    if (status == ERROR_ENVVAR_NOT_FOUND) return "ERROR_ENVVAR_NOT_FOUND";
    if (status == ERROR_SETENV_FAILED) return "ERROR_SETENV_FAILED";
    return "UNKNOWN ERROR";
}
std::string error_code_to_string(DWORD error_code) {
    LPSTR lpMsgBuf{};
    size_t MsgBufSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                      nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsgBuf, 0, nullptr);
    std::string code_str(lpMsgBuf, MsgBufSize);
    LocalFree(lpMsgBuf);
    return code_str;
}

void print_error_message(LSTATUS status, const char* function_name, std::string optional_message = "") {
    DWORD dw = GetLastError();

    std::cerr << function_name << " failed with " << win_api_error_str(status) << ": " << error_code_to_string(dw);
    if (optional_message != "") {
        std::cerr << " | " << optional_message;
    }
    std::cerr << "\n";
}

// get_env_var() - returns a std::string of `name`. if report_failure is true, then it will log to stderr that it didn't find the
//     env-var
std::string get_env_var(std::string const& name, bool report_failure) {
    std::wstring name_utf16 = widen(name);
    DWORD value_size = GetEnvironmentVariableW(name_utf16.c_str(), nullptr, 0);
    if (0 == value_size) {
        if (report_failure) print_error_message(ERROR_ENVVAR_NOT_FOUND, "GetEnvironmentVariableW");
        return {};
    }
    std::wstring value(value_size, L'\0');
    if (GetEnvironmentVariableW(name_utf16.c_str(), &value[0], value_size) != value_size - 1) {
        return {};
    }
    return narrow(value);
}
#elif TESTING_COMMON_UNIX_PLATFORMS

// get_env_var() - returns a std::string of `name`. if report_failure is true, then it will log to stderr that it didn't find the
//     env-var
std::string get_env_var(std::string const& name, bool report_failure) {
    char* ret = getenv(name.c_str());
    if (ret == nullptr) {
        if (report_failure) std::cerr << "Failed to get environment variable:" << name << "\n";
        return std::string();
    }
    return ret;
}
#endif

std::vector<std::string> split_env_var_as_list(std::string const& env_var_contents) {
    std::vector<std::string> items;
    size_t start = 0;
    size_t len = 0;
    for (size_t i = 0; i < env_var_contents.size(); i++) {
        if (env_var_contents[i] == OS_ENV_VAR_LIST_SEPARATOR) {
            if (len != 0) {
                // only push back non empty strings
                items.push_back(env_var_contents.substr(start, len));
            }
            start = i + 1;
            len = 0;
        } else {
            len++;
        }
    }
    items.push_back(env_var_contents.substr(start, len));
    return items;
}

/*
 * Wrapper around Environment Variables with common operations
 * Since Environment Variables leak between tests, there needs to be RAII code to remove them during test cleanup

 */

EnvVarWrapper::EnvVarWrapper(std::string const& name) noexcept : name(name) {
    initial_value = get_env_var(name, false);
    remove_env_var();
}
EnvVarWrapper::EnvVarWrapper(std::string const& name, std::string const& value) noexcept : name(name), cur_value(value) {
    initial_value = get_env_var(name, false);
    set_env_var();
}
EnvVarWrapper::~EnvVarWrapper() noexcept {
    remove_env_var();
    if (!initial_value.empty()) {
        set_new_value(initial_value);
    }
}

void EnvVarWrapper::set_new_value(std::string const& value) {
    cur_value = value;
    set_env_var();
}
void EnvVarWrapper::add_to_list(std::string const& list_item) {
    if (!cur_value.empty()) {
        cur_value += OS_ENV_VAR_LIST_SEPARATOR;
    }
    cur_value += list_item;
    set_env_var();
}
#if defined(_WIN32)
void EnvVarWrapper::add_to_list(std::wstring const& list_item) {
    if (!cur_value.empty()) {
        cur_value += OS_ENV_VAR_LIST_SEPARATOR;
    }
    cur_value += narrow(list_item);
    set_env_var();
}
#endif
void EnvVarWrapper::remove_value() const { remove_env_var(); }
const char* EnvVarWrapper::get() const { return name.c_str(); }
const char* EnvVarWrapper::value() const { return cur_value.c_str(); }

#if defined(_WIN32)

void EnvVarWrapper::set_env_var() const {
    BOOL ret = SetEnvironmentVariableW(widen(name).c_str(), widen(cur_value).c_str());
    if (ret == 0) {
        print_error_message(ERROR_SETENV_FAILED, "SetEnvironmentVariableW");
    }
}
void EnvVarWrapper::remove_env_var() const { SetEnvironmentVariableW(widen(name).c_str(), nullptr); }

#elif TESTING_COMMON_UNIX_PLATFORMS

void EnvVarWrapper::set_env_var() const { setenv(name.c_str(), cur_value.c_str(), 1); }
void EnvVarWrapper::remove_env_var() const { unsetenv(name.c_str()); }
#endif
