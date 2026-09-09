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

#pragma once

#include <cstdint>
#include <stack>
#include <string>
#include <filesystem>

#include "wide_char_handling.h"

// Utility class to simplify the writing of JSON manifest files

struct JsonWriter {
    std::string output;

    // the bool represents whether an object has been written & a comma is needed
    std::stack<bool> stack;

    void StartObject();
    void StartKeyedObject(std::string const& key);
    void EndObject();
    void StartKeyedArray(std::string const& key);
    void StartArray();
    void EndArray();

    void AddKeyedString(std::string const& key, std::string const& value);
    void AddString(std::string const& value);
#if defined(_WIN32)
    void AddKeyedString(std::string const& key, std::wstring const& value);
    void AddString(std::wstring const& value);
#endif

    void AddKeyedBool(std::string const& key, bool value);
    void AddBool(bool value);

    void AddKeyedNumber(std::string const& key, double number);
    void AddNumber(double number);

    void AddKeyedInteger(std::string const& key, int64_t number);
    void AddInteger(int64_t number);

    // Json doesn't allow `\` in strings, it must be escaped. Thus we have to convert '\\' to '\\\\' in strings
    static std::string escape(std::string const& in_path);
    static std::string escape(std::filesystem::path const& in_path);

   private:
    void CommaAndNewLine();
    void Indent();
};
