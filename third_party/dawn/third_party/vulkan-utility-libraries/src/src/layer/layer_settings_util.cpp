// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#include "layer_settings_util.hpp"

#include <sstream>
#include <regex>
#include <cstdlib>
#include <cassert>
#include <cstdint>

namespace vl {

std::vector<std::string> Split(const std::string &pValues, char delimiter) {
    std::vector<std::string> result;

    std::string parse = pValues;

    std::size_t start = 0;
    std::size_t end = parse.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(parse.substr(start, end - start));
        start = end + 1;
        end = parse.find(delimiter, start);
    }

    const std::string last = parse.substr(start, end);
    if (!last.empty()) {
        result.push_back(last);
    }

    return result;
}

std::string GetFileSettingName(const char *pLayerName, const char *pSettingName) {
    assert(pLayerName != nullptr);
    assert(pSettingName != nullptr);

    std::stringstream settingName;
    settingName << vl::ToLower(TrimPrefix(pLayerName)) << "." << pSettingName;

    return settingName.str();
}

static const char *GetDefaultPrefix() {
#ifdef __ANDROID__
    return "vulkan";
#else
    return "";
#endif
}

std::string GetEnvSettingName(const char *layer_key, const char *requested_prefix, const char *setting_key, TrimMode trim_mode) {
    std::stringstream result;
    const std::string prefix = (requested_prefix == nullptr || trim_mode != TRIM_NAMESPACE) ? GetDefaultPrefix() : requested_prefix;

#if defined(__ANDROID__)
    const std::string full_prefix = std::string("debug.") + prefix + ".";
    switch (trim_mode) {
        default:
        case TRIM_NONE: {
            result << full_prefix << GetFileSettingName(layer_key, setting_key);
            break;
        }
        case TRIM_VENDOR: {
            result << full_prefix << GetFileSettingName(TrimVendor(layer_key).c_str(), setting_key);
            break;
        }
        case TRIM_NAMESPACE: {
            result << full_prefix << setting_key;
            break;
        }
    }
#else
    const std::string full_prefix = std::string("VK_") + (prefix.empty() ? "" : prefix + "_");
    switch (trim_mode) {
        default:
        case TRIM_NONE: {
            result << full_prefix << vl::ToUpper(TrimPrefix(layer_key)) << "_" << vl::ToUpper(setting_key);
            break;
        }
        case TRIM_VENDOR: {
            result << full_prefix << vl::ToUpper(TrimVendor(layer_key)) << "_" << vl::ToUpper(setting_key);
            break;
        }
        case TRIM_NAMESPACE: {
            result << full_prefix << vl::ToUpper(setting_key);
            break;
        }
    }

#endif
    return result.str();
}

char GetEnvDelimiter() {
#ifdef WIN32  // a define is necessary because ':' is used for disk drives on Windows path
    return ';';
#else
    return ':';
#endif
}

char FindDelimiter(const std::string &s) {
    if (s.find(',') != std::string::npos) {
        return ',';
    } else if (s.find(GetEnvDelimiter()) != std::string::npos) {
        return GetEnvDelimiter();
    } else {
        return ',';
    }
}

std::string TrimWhitespace(const std::string &s) {
    const char *whitespace = " \t\f\v\n\r";

    const auto trimmed_beg = s.find_first_not_of(whitespace);
    if (trimmed_beg == std::string::npos) return "";

    const auto trimmed_end = s.find_last_not_of(whitespace);
    assert(trimmed_end != std::string::npos && trimmed_beg <= trimmed_end);

    return s.substr(trimmed_beg, trimmed_end - trimmed_beg + 1);
}

std::string TrimPrefix(const std::string &layer_key) {
    std::string key{};
    if (layer_key.find("VK_LAYER_") == 0) {
        std::size_t prefix = std::strlen("VK_LAYER_");
        key = layer_key.substr(prefix, layer_key.size() - prefix);
    } else {
        key = layer_key;
    }
    return key;
}

std::string TrimVendor(const std::string &layer_key) {
    static const char *separator = "_";

    const std::string &namespace_key = TrimPrefix(layer_key);

    const auto trimmed_beg = namespace_key.find_first_of(separator);
    if (trimmed_beg == std::string::npos) return namespace_key;

    assert(namespace_key.find_last_not_of(separator) != std::string::npos &&
           trimmed_beg <= namespace_key.find_last_not_of(separator));

    return namespace_key.substr(trimmed_beg + 1, namespace_key.size());
}

std::string ToLower(const std::string &s) {
    std::string result = s;
    for (auto &c : result) {
        c = (char)std::tolower(c);
    }
    return result;
}

std::string ToUpper(const std::string &s) {
    std::string result = s;
    for (auto &c : result) {
        c = (char)std::toupper(c);
    }
    return result;
}

uint32_t ToUint32(const std::string &token) {
    uint32_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint32_t>(std::strtoul(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

uint64_t ToUint64(const std::string &token) {
    uint64_t int_id = 0;
    if ((token.find("0x") == 0) || token.find("0X") == 0) {  // Handle hex format
        int_id = static_cast<uint64_t>(std::strtoull(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<uint64_t>(std::strtoull(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

int32_t ToInt32(const std::string &token) {
    int32_t int_id = 0;
    if (token.find("0x") == 0 || token.find("0X") == 0 || token.find("-0x") == 0 || token.find("-0X") == 0) {  // Handle hex format
        int_id = static_cast<int32_t>(std::strtol(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<int32_t>(std::strtol(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

int64_t ToInt64(const std::string &token) {
    int64_t int_id = 0;
    if (token.find("0x") == 0 || token.find("0X") == 0 || token.find("-0x") == 0 || token.find("-0X") == 0) {  // Handle hex format
        int_id = static_cast<int64_t>(std::strtoll(token.c_str(), nullptr, 16));
    } else {
        int_id = static_cast<int64_t>(std::strtoll(token.c_str(), nullptr, 10));  // Decimal format
    }
    return int_id;
}

VkuFrameset ToFrameSet(const std::string &s) {
    assert(IsFrameSets(s));

    VkuFrameset frameset{0, 1, 1};

    const std::vector<std::string> &frameset_split = vl::Split(s, '-');
    if (frameset_split.size() >= 1) {
        frameset.first = static_cast<std::uint32_t>(std::atoll(frameset_split[0].c_str()));
    }
    if (frameset_split.size() >= 2) {
        frameset.count = static_cast<std::uint32_t>(std::atoll(frameset_split[1].c_str()));
    }
    if (frameset_split.size() >= 3) {
        frameset.step = static_cast<std::uint32_t>(std::atoll(frameset_split[2].c_str()));
    }

    return frameset;
}

std::vector<VkuFrameset> ToFrameSets(const std::string &s) {
    std::vector<std::string> tokens = Split(s, FindDelimiter(s));

    std::vector<VkuFrameset> results;
    results.resize(tokens.size());
    for (std::size_t i = 0, n = tokens.size(); i < n; ++i) {
        results[i] = ToFrameSet(tokens[i]);
    }

    return results;
}

bool IsFrameSets(const std::string &s) {
    static const std::regex FRAME_REGEX("^([0-9]+([-][0-9]+){0,2})(,([0-9]+([-][0-9]+){0,2}))*$");

    return std::regex_search(s, FRAME_REGEX);
}

bool IsInteger(const std::string &s) {
    static const std::regex FRAME_REGEX("^-?([0-9]*|0x[0-9|a-z|A-Z]*)$");

    return std::regex_search(s, FRAME_REGEX);
}

bool IsFloat(const std::string &s) {
    static const std::regex FRAME_REGEX("^-?[0-9]*([.][0-9]*f?)?$");

    return std::regex_search(s, FRAME_REGEX);
}

std::string FormatString(const char *message, ...) {
    std::size_t const STRING_BUFFER(4096);

    assert(message != nullptr);
    assert(strlen(message) >= 1 && strlen(message) < STRING_BUFFER);

    char buffer[STRING_BUFFER];
    va_list list;

    va_start(list, message);
    vsnprintf(buffer, STRING_BUFFER, message, list);
    va_end(list);

    return buffer;
}

}  // namespace vl
