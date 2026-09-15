/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "webgpu/utils/SpecializationConstantProcessor.h"

#include <backend/Program.h>

#include <utils/CString.h>

#include <gtest/gtest.h>

#include <string_view>

namespace filament::backend::webgpuutils {

TEST(WebGPUSpecializationConstantProcessor, ReplacesConstantsById) {
    Program::SpecializationConstantsInfo constants =
            Program::SpecializationConstantsInfo::with_capacity(3);
    constants.push_back(false);
    constants.push_back(int32_t{ -7 });
    constants.push_back(2.5f);

    constexpr std::string_view source = R"(
        @id(0) override dynamic_lighting : bool = true;
        @id(1) override light_count : i32 = 4;
        @id(2) override exposure : f32 = 1.0;
    )";
    utils::CString const specialized = specializeShaderSource(source, constants);
    std::string_view const specializedView{ specialized };

    EXPECT_NE(specializedView.find("@id(0) override dynamic_lighting : bool = false;"),
            std::string_view::npos);
    EXPECT_NE(specializedView.find("@id(1) override light_count : i32 = -7;"),
            std::string_view::npos);
    EXPECT_NE(specializedView.find("@id(2) override exposure : f32 = 2.500000;"),
            std::string_view::npos);
}

TEST(WebGPUSpecializationConstantProcessor, AddsMissingInitializer) {
    Program::SpecializationConstantsInfo constants =
            Program::SpecializationConstantsInfo::with_capacity(1);
    constants.push_back(true);

    EXPECT_EQ(specializeShaderSource("@id(0) override enabled : bool;", constants),
            "@id(0) override enabled : bool = true;");
}

TEST(WebGPUSpecializationConstantProcessor, IgnoresUnusedConstantsAndOtherAttributes) {
    Program::SpecializationConstantsInfo constants =
            Program::SpecializationConstantsInfo::with_capacity(2);
    constants.push_back(false);
    constants.push_back(true);

    constexpr std::string_view source = "@id(1) @diagnostic(off, derivative_uniformity) "
                                        "override enabled : bool = false;";
    EXPECT_EQ(specializeShaderSource(source, constants),
            "@id(1) @diagnostic(off, derivative_uniformity) override enabled : bool = true;");
}

TEST(WebGPUSpecializationConstantProcessor, MatchesMultiDigitIdsExactly) {
    Program::SpecializationConstantsInfo constants =
            Program::SpecializationConstantsInfo::with_capacity(19);
    for (size_t i = 0; i < 19; ++i) {
        constants.push_back(false);
    }
    constants[18] = true;

    constexpr std::string_view source = "@id(1) override first : bool = true;\n"
                                        "@id(18) override directional : bool = false;";
    EXPECT_EQ(specializeShaderSource(source, constants),
            "@id(1) override first : bool = false;\n"
            "@id(18) override directional : bool = true;");
}

} // namespace filament::backend::webgpuutils
