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

#include "json_writer.h"

#include "wide_char_handling.h"

void JsonWriter::StartObject() {
    CommaAndNewLine();
    Indent();
    output += "{";
    stack.push(false);
}
void JsonWriter::StartKeyedObject(std::string const& key) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": {";
    stack.push(false);
}
void JsonWriter::EndObject() {
    stack.pop();
    output += "\n";
    Indent();
    output += "}";
}
void JsonWriter::StartKeyedArray(std::string const& key) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": [";
    stack.push(false);
}
void JsonWriter::StartArray() {
    CommaAndNewLine();
    Indent();
    output += "[";
    stack.push(false);
}
void JsonWriter::EndArray() {
    stack.pop();
    output += "\n";
    Indent();
    output += "]";
}

void JsonWriter::AddKeyedString(std::string const& key, std::string const& value) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": \"" + escape(value) + "\"";
}
void JsonWriter::AddString(std::string const& value) {
    CommaAndNewLine();
    Indent();
    output += "\"" + escape(value) + "\"";
}
#if defined(_WIN32)
void JsonWriter::AddKeyedString(std::string const& key, std::wstring const& value) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": \"" + escape(narrow(value)) + "\"";
}
void JsonWriter::AddString(std::wstring const& value) {
    CommaAndNewLine();
    Indent();
    output += "\"" + escape(narrow(value)) + "\"";
}
#endif

void JsonWriter::AddKeyedBool(std::string const& key, bool value) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": " + (value ? "true" : "false");
}
void JsonWriter::AddBool(bool value) {
    CommaAndNewLine();
    Indent();
    output += std::string(value ? "true" : "false");
}

void JsonWriter::AddKeyedNumber(std::string const& key, double number) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": " + std::to_string(number);
}
void JsonWriter::AddNumber(double number) {
    CommaAndNewLine();
    Indent();
    output += std::to_string(number);
}

void JsonWriter::AddKeyedInteger(std::string const& key, int64_t number) {
    CommaAndNewLine();
    Indent();
    output += "\"" + key + "\": " + std::to_string(number);
}
void JsonWriter::AddInteger(int64_t number) {
    CommaAndNewLine();
    Indent();
    output += std::to_string(number);
}

// Json doesn't allow `\` in strings, it must be escaped. Thus we have to convert '\\' to '\\\\' in strings
std::string JsonWriter::escape(std::string const& in_path) {
    std::string out;
    for (auto& c : in_path) {
        if (c == '\\')
            out += "\\\\";
        else
            out += c;
    }
    return out;
}
std::string JsonWriter::escape(std::filesystem::path const& in_path) { return escape(narrow(in_path.native())); }

void JsonWriter::CommaAndNewLine() {
    if (stack.size() > 0) {
        if (stack.top() == false) {
            stack.top() = true;
        } else {
            output += ",";
        }
        output += "\n";
    }
}
void JsonWriter::Indent() {
    for (uint32_t i = 0; i < stack.size(); i++) {
        output += '\t';
    }
}
