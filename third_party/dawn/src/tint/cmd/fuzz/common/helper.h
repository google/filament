// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_CMD_FUZZ_COMMON_HELPER_H_
#define SRC_TINT_CMD_FUZZ_COMMON_HELPER_H_

#include <string>
#include <string_view>

namespace tint::fuzz::common {

/// ReplaceAll replaces all occurrences of @p substr with @p replacement in @p str
/// @param str the string to perform the replacement on
/// @param substr the substring to replace
/// @param replacement the replacement string
/// @returns the string with all occurrences of @p substr replaced with @p replacement
std::string ReplaceAll(std::string str, std::string_view substr, std::string_view replacement);

/// @returns the default Vulkan ICD path for the current executable
/// @param argv the command line arguments
std::string GetDefaultVkICDPath(char*** argv);

/// Prints the Vulkan ICD path if it is not empty
/// @param vk_icd_path the path to the Vulkan ICD
void PrintVkICDPathFound(const std::string& vk_icd_path);

/// @returns the default DXC path for the current executable
/// @param argv the command line arguments
std::string GetDefaultDxcPath(char*** argv);

/// Prints the DXC path if it is found
/// @param dxc_path the path to the DXC library
void PrintDxcPathFound(const std::string& dxc_path);

}  // namespace tint::fuzz::common

#endif  // SRC_TINT_CMD_FUZZ_COMMON_HELPER_H_
