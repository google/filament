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

#include "SpecializationConstantProcessor.h"

#include <utils/CString.h>
#include <utils/Panic.h>

#include <string_view>
#include <variant>

namespace filament::backend::webgpuutils {

namespace {

struct SpecializationConstantFormatter {
    utils::CString operator()(int32_t const value) const { return utils::to_string(value); }
    utils::CString operator()(float const value) const { return utils::to_string(value); }
    utils::CString operator()(bool const value) const {
        return utils::CString(value ? "true" : "false");
    }
};

} // namespace

/**
 * Specializes WGSL shader source by baking specialization constant values into the defaults of
 * WGSL `override` declarations.
 *
 * Example:
 *   Given:
 *     specializationConstants[0] = true;
 *     specializationConstants[1] = -7;
 *
 *   Before specialization:
 *     @id(0) override dynamic_lighting : bool = false;
 *     @id(1) override light_count : i32;
 *
 *   After specialization:
 *     @id(0) override dynamic_lighting : bool = true;
 *     @id(1) override light_count : i32 = -7;
 */
utils::CString specializeShaderSource(std::string_view const source,
        Program::SpecializationConstantsInfo const& specializationConstants) {
    utils::CString result(source);
    for (uint32_t id = 0; id < specializationConstants.size(); ++id) {
        // Build the WGSL override attribute to search for, e.g. "@id(0)".
        utils::CString const idAttribute = "@id(" + utils::to_string(id) + ")";
        size_t declarationBegin = 0;

        // Find every occurrence of "@id(N)" in the WGSL shader source.
        while ((declarationBegin = std::string_view(result).find(idAttribute.c_str_safe(),
                        declarationBegin)) != std::string_view::npos) {
            size_t const declarationEnd =
                    std::string_view(result).find(';', declarationBegin + idAttribute.size());
            assert_invariant(declarationEnd != std::string_view::npos);

            // Ensure the "override" keyword is present in this declaration statement.
            size_t const overridePosition =
                    std::string_view(result).find("override", declarationBegin);
            if (overridePosition == std::string_view::npos || overridePosition > declarationEnd) {
                declarationBegin += idAttribute.size();
                continue;
            }

            // Format the constant value (bool, int32_t, or float) to its WGSL string representation.
            utils::CString const value =
                    std::visit(SpecializationConstantFormatter{}, specializationConstants[id]);

            // Check if an existing initializer ('=') is present before ';'.
            size_t const initializer = std::string_view(result).find('=', overridePosition);
            if (initializer == std::string_view::npos || initializer > declarationEnd) {
                // Case A: No existing initializer (e.g. "@id(0) override enabled : bool;").
                // Insert " = <value>" directly before the terminating semicolon.
                result.insert(declarationEnd, " = " + value);
                declarationBegin = declarationEnd + value.size() + 3;
                continue;
            }

            // Case B: Existing initializer (e.g. "@id(0) override enabled : bool = false;").
            // Replace the old default value between '=' and ';' with " <value>".
            result.replace(initializer + 1, declarationEnd - initializer - 1, " " + value);
            declarationBegin = initializer + value.size() + 2;
        }
    }
    return result;
}

} // namespace filament::backend::webgpuutils
