// Copyright 2023 The Dawn & Tint Authors
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

#include "src/dawn/node/binding/TogglesLoader.h"
#include "src/dawn/node/binding/Split.h"

namespace wgpu::binding {

TogglesLoader::TogglesLoader(const Flags& flags) {
    if (auto values = flags.Get("enable-dawn-features")) {
        enabledTogglesStrings_ = Split(*values, ',');
        for (auto& t : enabledTogglesStrings_) {
            enabledToggles_.emplace_back(t.c_str());
        }
    }
    if (auto values = flags.Get("disable-dawn-features")) {
        disabledTogglesStrings_ = Split(*values, ',');
        for (auto& t : disabledTogglesStrings_) {
            disabledToggles_.emplace_back(t.c_str());
        }
    }
}

DawnTogglesDescriptor TogglesLoader::GetDescriptor() {
    DawnTogglesDescriptor result;
    result.enabledToggleCount = enabledToggles_.size();
    result.enabledToggles = enabledToggles_.data();
    result.disabledToggleCount = disabledToggles_.size();
    result.disabledToggles = disabledToggles_.data();
    return result;
}
}  // namespace wgpu::binding
