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

module;
#include "{{cpp_header}}"
export module wgpu;

export namespace {{metadata.namespace}} {
// constants
{% for constant in by_category["constant"] %}
    using {{metadata.namespace}}::k{{constant.name.CamelCase()}};
{% endfor %}

// enums
{% for type in by_category["enum"] if type.name.get() != "optional bool" %}
    using {{metadata.namespace}}::{{as_cppType(type.name)}};
{% endfor %}

// bitmasks
{% for type in by_category["bitmask"] %}
    using {{metadata.namespace}}::{{as_cppType(type.name)}};
{% endfor %}

WGPU_IMPORT_BITMASK_OPERATORS

using {{metadata.namespace}}::IsWGPUBitmask;
using {{metadata.namespace}}::LowerBitmask;
using {{metadata.namespace}}::BoolConvertible;

// callbacks
{% for type in by_category["callback function"] %}
    using {{metadata.namespace}}::{{as_cppType(type.name)}};
{% endfor %}

// classes
{% for type in by_category["object"] %}
    using {{metadata.namespace}}::{{as_cppType(type.name)}};
{% endfor %}

// structs
{% for type in by_category["structure"] %}
    using {{metadata.namespace}}::{{as_cppType(type.name)}};
{% endfor %}

// Free Functions
{% for function in by_category["function"] if not function.no_cpp %}
    using {{metadata.namespace}}::{{as_cppType(function.name)}};
{% endfor %}

}
