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

#include <algorithm>
#include <array>
#include <sstream>
#include <iomanip>

#include "dawn/common/GPUInfo_autogen.h"

#include "dawn/common/Assert.h"

namespace dawn::gpu_info {

namespace {

enum class Architecture {
    Unknown,
    {% for vendor in vendors %}
        {% for architecture_name in vendor.architecture_names %}
            {{vendor.name.CamelCase()}}_{{architecture_name.CamelCase()}},
        {% endfor %}
    {% endfor %}
};

Architecture GetArchitecture(PCIVendorID vendorId, PCIDeviceID deviceId) {
    switch(vendorId) {
        {% for vendor in vendors %}
            {% if len(vendor.device_sets) %}
                case kVendorID_{{vendor.name.CamelCase()}}: {
                    {% for device_set in vendor.device_sets %}
                        {% if not device_set.internal %}
                            switch (deviceId{{device_set.maskDeviceId()}}) {
                                {% for architecture in device_set.architectures %}
                                    {% for device in architecture.devices %}
                                        case {{device}}:
                                    {% endfor %}
                                        return Architecture::{{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}};
                                {% endfor %}
                            }
                        {% endif %}
                    {% endfor %}
                } break;
            {% endif %}
        {% endfor %}
    }

    return Architecture::Unknown;
}

{% if has_internal %}

    enum class InternalArchitecture {
        Unknown,
        {% for vendor in vendors %}
            {% for architecture_name in vendor.internal_architecture_names %}
                {{vendor.name.CamelCase()}}_{{architecture_name.CamelCase()}},
            {% endfor %}
        {% endfor %}
    };

    InternalArchitecture GetInternalArchitecture(PCIVendorID vendorId, PCIDeviceID deviceId) {
        switch(vendorId) {
            {% for vendor in vendors %}
                {% if len(vendor.device_sets) %}
                    case kVendorID_{{vendor.name.CamelCase()}}: {
                        {% for device_set in vendor.device_sets %}
                            {% if device_set.internal %}
                                switch (deviceId{{device_set.maskDeviceId()}}) {
                                    {% for architecture in device_set.architectures %}
                                        {% for device in architecture.devices %}
                                            case {{device}}:
                                        {% endfor %}
                                            return InternalArchitecture::{{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}};
                                    {% endfor %}
                                }
                            {% endif %}
                        {% endfor %}
                    } break;
                {% endif %}
            {% endfor %}
        }

        return InternalArchitecture::Unknown;
    }

{% endif %}

}  // namespace

// Vendor checks
{% for vendor in vendors %}
    bool Is{{vendor.name.CamelCase()}}(PCIVendorID vendorId) {
        return vendorId == kVendorID_{{vendor.name.CamelCase()}};
    }
{% endfor %}

// Architecture checks

{% for vendor in vendors %}
    {% if len(vendor.architecture_names) %}
        // {{vendor.name.get()}} architectures
        {% for architecture_name in vendor.architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId) {
                return GetArchitecture(vendorId, deviceId) == Architecture::{{vendor.name.CamelCase()}}_{{architecture_name.CamelCase()}};
            }
        {% endfor %}
        {% for architecture_name in vendor.internal_architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId) {
                return GetInternalArchitecture(vendorId, deviceId) == InternalArchitecture::{{vendor.name.CamelCase()}}_{{architecture_name.CamelCase()}};
            }
        {% endfor %}
    {% endif %}
{% endfor %}

// GPUAdapterInfo fields
std::string GetVendorName(PCIVendorID vendorId) {
    switch(vendorId) {
        {% for vendor in vendors %}
            case kVendorID_{{vendor.name.CamelCase()}}:
            {% if vendor.name_override %}
                    return "{{vendor.name_override.js_enum_case()}}";
            {% else %}
                    return "{{vendor.name.js_enum_case()}}";
            {% endif %}
        {% endfor %}
    }

    return "";
}

std::string GetArchitectureName(PCIVendorID vendorId, PCIDeviceID deviceId) {
    Architecture arch = GetArchitecture(vendorId, deviceId);
    switch(arch) {
        case Architecture::Unknown:
            return "";
        {% for vendor in vendors %}
            {% for architecture_name in vendor.architecture_names %}
                case Architecture::{{vendor.name.CamelCase()}}_{{architecture_name.CamelCase()}}:
                    return "{{architecture_name.js_enum_case()}}";
            {% endfor %}
        {% endfor %}
    }

    return "";
}

}  // namespace dawn::gpu_info
