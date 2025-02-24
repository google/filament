//* Copyright 2019 The Dawn & Tint Authors
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

#ifndef DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_
#define DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_

#include "dawn/wire/ObjectType_autogen.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

    template<typename T>
    inline constexpr ObjectType ObjectTypeToTypeEnum = static_cast<ObjectType>(-1);

    {% for type in by_category["object"] %}
        {% set Type = type.name.CamelCase() %}
        {% if type.name.CamelCase() in client_special_objects %}
            class {{Type}};
        {% else %}
            struct {{type.name.CamelCase()}} final : ObjectBase {
                using ObjectBase::ObjectBase;

                ObjectType GetObjectType() const override {
                    return ObjectType::{{type.name.CamelCase()}};
                }
            };
        {% endif %}

        inline {{Type}}* FromAPI(WGPU{{Type}} obj) {
            return reinterpret_cast<{{Type}}*>(obj);
        }
        inline WGPU{{Type}} ToAPI({{Type}}* obj) {
            return reinterpret_cast<WGPU{{Type}}>(obj);
        }

        template <>
        inline constexpr ObjectType ObjectTypeToTypeEnum<{{Type}}> = ObjectType::{{Type}};

    {% endfor %}
}  // namespace dawn::wire::client

#endif  // DAWNWIRE_CLIENT_APIOBJECTS_AUTOGEN_H_
