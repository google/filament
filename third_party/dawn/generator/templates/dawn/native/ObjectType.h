//* Copyright 2020 The Dawn & Tint Authors
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

{% set namespace_name = Name(metadata.native_namespace) %}
{% set DIR = namespace_name.concatcase().upper() %}
#ifndef {{DIR}}_OBJECTTPYE_AUTOGEN_H_
#define {{DIR}}_OBJECTTPYE_AUTOGEN_H_

#include "dawn/common/ityp_array.h"

#include <cstdint>

{% set native_namespace = namespace_name.namespace_case() %}
namespace {{native_namespace}} {

    enum class ObjectType : uint32_t {
        {% for type in by_category["object"] %}
            {{type.name.CamelCase()}},
        {% endfor %}
    };

    template <typename T>
    using PerObjectType = ityp::array<ObjectType, T, {{len(by_category["object"])}}>;

    const char* ObjectTypeAsString(ObjectType type);

} // namespace {{native_namespace}}


#endif  // {{DIR}}_OBJECTTPYE_AUTOGEN_H_
