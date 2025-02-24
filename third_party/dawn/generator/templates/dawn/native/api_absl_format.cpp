//* Copyright 2021 The Dawn & Tint Authors
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
{% set api = metadata.api.lower() %}
#include "{{native_dir}}/{{api}}_absl_format_autogen.h"

#include "{{native_dir}}/ChainUtils.h"
#include "{{native_dir}}/ObjectType_autogen.h"
#include "{{native_dir}}/webgpu_absl_format.h"

namespace {{native_namespace}} {

    //
    // Descriptors
    //

    {% for type in by_category["structure"] %}
        {% for member in type.members %}
            {% if member.name.canonical_case() == "label" %}
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                AbslFormatConvert(const {{as_cppType(type.name)}}* value,
                                  const absl::FormatConversionSpec& spec,
                                  absl::FormatSink* s) {
                    if (value == nullptr) {
                        s->Append("[null]");
                        return {true};
                    }
                    s->Append("[{{as_cppType(type.name)}}");
                    if (value->label.data != nullptr) {
                        s->Append(absl::StrFormat(" \"%s\"", value->label));
                    }
                    s->Append("]");
                    return {true};
                }
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                AbslFormatConvert(const UnpackedPtr<{{as_cppType(type.name)}}>& value,
                                  const absl::FormatConversionSpec& spec,
                                  absl::FormatSink* s) {
                    return AbslFormatConvert(*value, spec, s);
                }
            {% endif %}
        {% endfor %}
    {% endfor %}

}  // namespace {{native_namespace}}

{% set namespace = metadata.namespace %}
namespace {{namespace}} {

    //
    // Enums
    //

    {% for type in by_category["enum"] %}
    absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
    AbslFormatConvert({{as_cppType(type.name)}} value,
                      const absl::FormatConversionSpec& spec,
                      absl::FormatSink* s) {
        if (spec.conversion_char() == absl::FormatConversionChar::s) {
            s->Append("{{as_cppType(type.name)}}::");
            switch ({{as_cType(type.name)}}(value)) {
            {% for value in type.values if not is_enum_value_proxy(value) %}
                case {{as_cEnum(type.name, value.name)}}:
                    s->Append("{{as_cppEnum(value.name)}}");
                    return {true};
            {% endfor %}
            default:
                break;
            }
        }
        s->Append(absl::StrFormat("%u", static_cast<{{as_cType(type.name)}}>(value)));
        return {true};
    }
    {% endfor %}

    //
    // Bitmasks
    //

    {% for type in by_category["bitmask"] %}
    absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
    AbslFormatConvert({{as_cppType(type.name)}} value,
                      const absl::FormatConversionSpec& spec,
                      absl::FormatSink* s) {
        if (spec.conversion_char() == absl::FormatConversionChar::s) {
            s->Append("{{as_cppType(type.name)}}::");
            if (!static_cast<bool>(value)) {
                {% for value in type.values if value.value == 0 %}
                    // 0 is often explicitly declared as None.
                    s->Append("{{as_cppEnum(value.name)}}");
                {% else %}
                    s->Append(absl::StrFormat("{{as_cppType(type.name)}}::%x", 0));
                {% endfor %}
                return {true};
            }

            bool moreThanOneBit = !HasZeroOrOneBits(value);
            if (moreThanOneBit) {
                s->Append("(");
            }

            bool first = true;
            {% for value in type.values if value.value != 0 %}
                if (value & {{as_cppType(type.name)}}::{{as_cppEnum(value.name)}}) {
                    if (!first) {
                        s->Append("|");
                    }
                    first = false;
                    s->Append("{{as_cppEnum(value.name)}}");
                    value &= ~{{as_cppType(type.name)}}::{{as_cppEnum(value.name)}};
                }
            {% endfor %}

            if (static_cast<bool>(value)) {
                if (!first) {
                    s->Append("|");
                }
                s->Append(absl::StrFormat("{{as_cppType(type.name)}}::%x", static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value)));
            }

            if (moreThanOneBit) {
                s->Append(")");
            }
        } else {
            s->Append(absl::StrFormat("%u", static_cast<typename std::underlying_type<{{as_cppType(type.name)}}>::type>(value)));
        }
        return {true};
    }
    {% endfor %}

}  // namespace {{namespace}}
