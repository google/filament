// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//
// Author(s):
// - Christophe Riccio <christophe@lunarg.com>
#pragma once

#include "vulkan/layer/vk_layer_settings.h"

#include <vector>
#include <string>
#include <cstring>
#include <cstdarg>

namespace vl {
const VkLayerSettingsCreateInfoEXT *FindSettingsInChain(const void *next);

std::vector<std::string> Split(const std::string &pValues, char delimiter);

enum TrimMode {
    TRIM_NONE,
    TRIM_VENDOR,
    TRIM_NAMESPACE,

    TRIM_FIRST = TRIM_NONE,
    TRIM_LAST = TRIM_NAMESPACE,
};

std::string GetEnvSettingName(const char *layer_key, const char *prefix, const char *setting_key, TrimMode trim_mode);

std::string GetFileSettingName(const char *layer_key, const char *setting_key);

// Find the delimiter (, ; :) in a string made of tokens. Return ',' by default
char FindDelimiter(const std::string &s);

// ';' on WIN32 and ':' on Unix
char GetEnvDelimiter();

// Remove whitespaces at the beginning of the end
std::string TrimWhitespace(const std::string &s);

std::string TrimPrefix(const std::string &layer_name);

std::string TrimVendor(const std::string &layer_name);

std::string ToLower(const std::string &s);

std::string ToUpper(const std::string &s);

uint32_t ToUint32(const std::string &token);

uint64_t ToUint64(const std::string &token);

int32_t ToInt32(const std::string &token);

int64_t ToInt64(const std::string &token);

bool IsFrameSets(const std::string &s);

VkuFrameset ToFrameSet(const std::string &s);

std::vector<VkuFrameset> ToFrameSets(const std::string &s);

bool IsInteger(const std::string &s);

bool IsFloat(const std::string &s);

std::string FormatString(const char *message, ...);
}  // namespace vl
