//* Copyright 2018 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
#include "{{native_dir}}/ValidationUtils_autogen.h"

namespace {{native_namespace}} {

    {% set namespace = metadata.namespace %}
    {% for type in by_category["enum"] %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value) {
            switch ({{as_cType(type.name)}}(value)) {
                {% for value in type.values if (value.valid and not is_enum_value_proxy(value)) %}
                    case {{as_cEnum(type.name, value.name)}}:
                        return {};
                {% endfor %}
                default:
                    return DAWN_VALIDATION_ERROR("Value %i is invalid for {{as_cType(type.name)}}.", value);
            }
        }

    {% endfor %}

    {% for type in by_category["bitmask"] %}
        MaybeError Validate{{type.name.CamelCase()}}({{namespace}}::{{as_cppType(type.name)}} value) {
            if ((value & static_cast<{{namespace}}::{{as_cppType(type.name)}}>(~{{type.full_mask}})) == 0) {
                return {};
            }
            return DAWN_VALIDATION_ERROR("Value %i is invalid for {{as_cType(type.name)}}.", value);
        }

    {% endfor %}

} // namespace {{native_namespace}}
