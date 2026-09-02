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

#include <vector>

// Macro to ease the definition of variables with builder member functions
// type = type of the variable
// name = name of the variable
// default_value = value to default initialize, use {} if nothing else makes sense
#define BUILDER_VALUE_WITH_DEFAULT(type, name, default_value) \
    type name = default_value;                                \
    auto set_##name(type const& name)->decltype(*this) {      \
        this->name = name;                                    \
        return *this;                                         \
    }

#define BUILDER_VALUE(type, name) BUILDER_VALUE_WITH_DEFAULT(type, name, {})

// Macro to ease the definition of vectors with builder member functions
// type = type of the variable
// name = name of the variable
// singular_name = used for the `add_singular_name` member function
#define BUILDER_VECTOR(type, name, singular_name)                                          \
    std::vector<type> name;                                                                \
    auto add_##singular_name(type const& singular_name)->decltype(*this) {                 \
        this->name.push_back(singular_name);                                               \
        return *this;                                                                      \
    }                                                                                      \
    auto add_##singular_name##s(std::vector<type> const& singular_name)->decltype(*this) { \
        for (auto& elem : singular_name) this->name.push_back(elem);                       \
        return *this;                                                                      \
    }
// Like BUILDER_VECTOR but for move only types - where passing in means giving up ownership
#define BUILDER_VECTOR_MOVE_ONLY(type, name, singular_name)           \
    std::vector<type> name;                                           \
    auto add_##singular_name(type&& singular_name)->decltype(*this) { \
        this->name.push_back(std::move(singular_name));               \
        return *this;                                                 \
    }
