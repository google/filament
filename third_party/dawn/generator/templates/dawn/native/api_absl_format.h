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

{% set API = metadata.api.upper() %}
#ifndef {{API}}_ABSL_FORMAT_H_
#define {{API}}_ABSL_FORMAT_H_

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/Forward.h"

#include "absl/strings/str_format.h"

namespace {{native_namespace}} {

    //
    // Descriptors
    //

    // Only includes structures that have a 'label' member.
    {% for type in by_category["structure"] %}
        {% for member in type.members %}
            {% if member.name.canonical_case() == "label" %}
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                    AbslFormatConvert(const {{as_cppType(type.name)}}* value,
                                      const absl::FormatConversionSpec& spec,
                                      absl::FormatSink* s);
                absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
                    AbslFormatConvert(const UnpackedPtr<{{as_cppType(type.name)}}>& value,
                                      const absl::FormatConversionSpec& spec,
                                      absl::FormatSink* s);
            {% endif %}
        {% endfor %}
    {% endfor %}

} // namespace {{native_namespace}}

{% set namespace = metadata.namespace %}
namespace {{namespace}} {

    //
    // Enums
    //

    {% for type in by_category["enum"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                          const absl::FormatConversionSpec& spec,
                          absl::FormatSink* s);
    {% endfor %}

    //
    // Bitmasks
    //

    {% for type in by_category["bitmask"] %}
        absl::FormatConvertResult<absl::FormatConversionCharSet::kString|absl::FormatConversionCharSet::kIntegral>
        AbslFormatConvert({{as_cppType(type.name)}} value,
                          const absl::FormatConversionSpec& spec,
                          absl::FormatSink* s);
    {% endfor %}

}  // namespace {{namespace}}

#endif // {{API}}_ABSL_FORMAT_H_
