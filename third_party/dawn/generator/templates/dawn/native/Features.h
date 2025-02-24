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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_FEATURES_AUTOGEN_H_
#define {{DIR}}_FEATURES_AUTOGEN_H_

#include "dawn/common/ityp_array.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class Feature {
  {% for enum in types["feature name"].values if (enum.valid and not is_enum_value_proxy(enum)) %}
    {{as_cppEnum(enum.name)}},
  {% endfor %}
  InvalidEnum,
};

template<>
struct EnumCount<Feature> {
    {% set counter = namespace(value = 0) %}
    {% for enum in types["feature name"].values if (enum.valid and not is_enum_value_proxy(enum)) -%}
        {% set counter.value = counter.value + 1 %}
    {% endfor %}
    static constexpr uint32_t value = {{counter.value}};
};

}  // namespace dawn::native

#endif  // {{DIR}}_FEATURES_AUTOGEN_H_
