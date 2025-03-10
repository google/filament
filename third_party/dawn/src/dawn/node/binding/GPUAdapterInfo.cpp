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

#include "src/dawn/node/binding/GPUAdapterInfo.h"

#include <iomanip>
#include <sstream>

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUAdapterInfo
////////////////////////////////////////////////////////////////////////////////

GPUAdapterInfo::GPUAdapterInfo(const wgpu::AdapterInfo& info)
    : vendor_(info.vendor),
      architecture_(info.architecture),
      device_(info.device),
      description_(info.description),
      subgroup_min_size_(info.subgroupMinSize),
      subgroup_max_size_(info.subgroupMaxSize) {}

std::string GPUAdapterInfo::getVendor(Napi::Env) {
    return vendor_;
}

std::string GPUAdapterInfo::getArchitecture(Napi::Env) {
    return architecture_;
}

std::string GPUAdapterInfo::getDevice(Napi::Env) {
    return device_;
}

std::string GPUAdapterInfo::getDescription(Napi::Env) {
    return description_;
}

uint32_t GPUAdapterInfo::getSubgroupMinSize(Napi::Env) {
    return subgroup_min_size_;
}

uint32_t GPUAdapterInfo::getSubgroupMaxSize(Napi::Env) {
    return subgroup_max_size_;
}

}  // namespace wgpu::binding
