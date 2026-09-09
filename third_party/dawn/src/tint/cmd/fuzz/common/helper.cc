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

#include "src/tint/cmd/fuzz/common/helper.h"

#include <filesystem>
#include <iostream>

#if TINT_BUILD_FUZZER_VULKAN_SUPPORT || TINT_BUILD_HLSL_WRITER
#include "src/tint/utils/command/command.h"
#endif

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/validate/validate.h"
#endif

namespace tint::fuzz::common {

std::string ReplaceAll(std::string str, std::string_view substr, std::string_view replacement) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string_view::npos) {
        str.replace(pos, substr.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

std::string GetDefaultVkICDPath([[maybe_unused]] char*** argv) {
    std::string default_vk_icd_path = "";
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    // Assume the Vulkan ICD JSON is in the same directory as this executable
    std::string exe_path = (*argv)[0];
    exe_path = ReplaceAll(exe_path, "\\", "/");
    auto pos = exe_path.rfind('/');
    if (pos != std::string::npos) {
        default_vk_icd_path = exe_path.substr(0, pos) + "/lvp_icd.json";
    } else {
        // argv[0] doesn't contain path to exe, try relative to cwd
        default_vk_icd_path = "lvp_icd.json";
    }

    if (!std::filesystem::exists(default_vk_icd_path)) {
        default_vk_icd_path = "";
    }
#endif
    return default_vk_icd_path;
}

void PrintVkICDPathFound([[maybe_unused]] const std::string& vk_icd_path) {
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    if (!vk_icd_path.empty()) {
        std::cout << "Vulkan ICD JSON found: " << vk_icd_path << "\n";
    } else {
        std::cout << "Vulkan ICD JSON not found\n";
    }
#endif
}

std::string GetDefaultDxcPath([[maybe_unused]] char*** argv) {
    std::string default_dxc_path = "";
#if TINT_BUILD_HLSL_WRITER
    // Assume the DXC library is in the same directory as this executable
    std::string exe_path = (*argv)[0];
    exe_path = ReplaceAll(exe_path, "\\", "/");
    auto pos = exe_path.rfind('/');
    if (pos != std::string::npos) {
        default_dxc_path = exe_path.substr(0, pos) + '/' + tint::hlsl::validate::kDxcDLLName;
    } else {
        // argv[0] doesn't contain path to exe, try relative to cwd
        default_dxc_path = tint::hlsl::validate::kDxcDLLName;
    }
#endif
    return default_dxc_path;
}

void PrintDxcPathFound([[maybe_unused]] const std::string& dxc_path) {
#if TINT_BUILD_HLSL_WRITER
    // Log whether the DXC library was found or not once at initialization.
    auto dxc = tint::Command::LookPath(dxc_path);
    if (dxc.Found()) {
        std::cout << "DXC library found: " << dxc.Path() << "\n";
    } else {
        std::cout << "DXC library not found: " << dxc_path << "\n";
    }
#endif
}

}  // namespace tint::fuzz::common
