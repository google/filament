/*
 * Copyright (c) 2021 The Khronos Group Inc.
 * Copyright (c) 2021 Valve Corporation
 * Copyright (c) 2021 LunarG, Inc.
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

#include "test_util.h"

struct LayerDefinition {
    BUILDER_VALUE(std::string, layerName)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, specVersion, VK_API_VERSION_1_0)
    BUILDER_VALUE_WITH_DEFAULT(uint32_t, implementationVersion, VK_API_VERSION_1_0)
    BUILDER_VALUE(std::string, description)
    BUILDER_VECTOR(Extension, extensions, extension)

    VkLayerProperties get() const noexcept {
        VkLayerProperties props{};
        copy_string_to_char_array(layerName, &props.layerName[0], VK_MAX_EXTENSION_NAME_SIZE);
        props.specVersion = specVersion;
        props.implementationVersion = implementationVersion;
        copy_string_to_char_array(description, &props.description[0], VK_MAX_DESCRIPTION_SIZE);
        return props;
    }
};
