// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_
#define SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_

#include <cstdint>
#include <string>

using PCIVendorID = uint32_t;
using PCIDeviceID = uint32_t;

namespace dawn::gpu_info {

// Vendor IDs
{% for vendor in vendors %}
    static constexpr PCIVendorID kVendorID_{{vendor.name.CamelCase()}} = {{vendor.id}};
{% endfor %}

// Vendor checks
{% for vendor in vendors %}
    bool Is{{vendor.name.CamelCase()}}(PCIVendorID vendorId);
{% endfor %}

// Architecture checks
{% for vendor in vendors %}
    {% if len(vendor.architecture_names) %}

        // {{vendor.name.get()}} architectures
        {% for architecture_name in vendor.architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId);
        {% endfor %}
        {% for architecture_name in vendor.internal_architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId);
        {% endfor %}
    {% endif %}
{% endfor %}

// GPUAdapterInfo fields
std::string GetVendorName(PCIVendorID vendorId);
std::string GetArchitectureName(PCIVendorID vendorId, PCIDeviceID deviceId);

} // namespace dawn::gpu_info
#endif  // SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_
