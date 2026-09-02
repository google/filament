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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
{% set namespace = metadata.namespace %}
#ifndef {{DIR}}_{{namespace.upper()}}_STRUCTS_DEFAULTS_H_
#define {{DIR}}_{{namespace.upper()}}_STRUCTS_DEFAULTS_H_

{% set include_dir = namespace_name.Dirs() %}
{% set native_namespace = namespace_name.namespace_case() %}
#include "{{include_dir}}/{{namespace}}_structs_autogen.h"

namespace {{native_namespace}} {

// These functions make a copy of the struct, then, for any enum members with trivial defaulting
// (where something like "Undefined" is replaced with a default), applies all of the defaults for
// the struct, and recursively its by-value substructs (but NOT by-pointer substructs since they
// are const*). It must be called in an appropriate place in Dawn.
{% for type in by_category["structure"] %}
    {% if type.any_member_requires_struct_defaulting %}
        {% set CppType = as_cppType(type.name) %}
        [[nodiscard]] {{CppType}} WithTrivialFrontendDefaults(const {{CppType}}& in);
    {% endif %}
{% endfor %}

} // namespace {{native_namespace}}

#endif  // {{DIR}}_{{namespace.upper()}}_STRUCTS_DEFAULTS_H_
