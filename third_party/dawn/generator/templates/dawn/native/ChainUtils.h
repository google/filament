// Copyright 2021 The Dawn & Tint Authors
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
#ifndef {{DIR}}_CHAIN_UTILS_H_
#define {{DIR}}_CHAIN_UTILS_H_

{% set impl_dir = metadata.impl_dir + "/" if metadata.impl_dir else "" %}
{% set namespace = metadata.namespace %}
{% set namespace_name = Name(metadata.native_namespace) %}
{% set native_namespace = namespace_name.namespace_case() %}
{% set native_dir = impl_dir + namespace_name.Dirs() %}
{% set prefix = metadata.proc_table_prefix.lower() %}
#include <tuple>

#include "absl/strings/str_format.h"
#include "{{native_dir}}/{{prefix}}_platform.h"
#include "{{native_dir}}/Error.h"
#include "{{native_dir}}/{{namespace}}_structs_autogen.h"

namespace {{native_namespace}} {
namespace detail {

// SType for implementation details. Kept inside the detail namespace for extensibility.
template <typename T>
constexpr inline {{namespace}}::SType STypeForImpl = {{namespace}}::SType(0u);

// Specialize STypeFor to map from native struct types to their SType.
{% for value in types["s type"].values %}
    {% if value.valid and not is_enum_value_proxy(value) and value.name.get() in types %}
        template <>
        constexpr inline {{namespace}}::SType STypeForImpl<{{as_cppEnum(value.name)}}> =
            {{namespace}}::SType::{{as_cppEnum(value.name)}};
    {% endif %}
{% endfor %}

template <typename Arg, typename... Rest>
std::string STypesToString() {
    if constexpr (sizeof...(Rest)) {
        return absl::StrFormat("%s, ", STypeForImpl<Arg>) + STypesToString<Rest...>();
    } else {
        return absl::StrFormat("%s", STypeForImpl<Arg>);
    }
}

// Typelist type used to further add extensions to chain roots when they are not in the json.
template <typename... Exts>
struct AdditionalExtensionsList;

// Root specializations for adding additional extensions.
template <typename Root>
struct AdditionalExtensions {
    using List = AdditionalExtensionsList<>;
};

// Template structs to get the typing for the unpacked chains.
template <typename...>
struct UnpackedPtrChain;
template <typename... Additionals, typename... Ts>
struct UnpackedPtrChain<AdditionalExtensionsList<Additionals...>, Ts...> {
    using Type = std::tuple<Ts..., Additionals...>;
};

}  // namespace detail

template <typename T>
constexpr inline wgpu::SType STypeFor = detail::STypeForImpl<T>;
template <typename T>
constexpr inline wgpu::SType STypeFor<const T*> = detail::STypeForImpl<T>;

}  // namespace {{native_namespace}}

// Include specializations before declaring types for ordering purposes.
#include "{{native_dir}}/ChainUtilsImpl.inl"

namespace {{native_namespace}} {
namespace detail {

// Template type to get the unpacked chain type from the root type.
template <typename Root>
struct UnpackedPtrTypeFor;

// Template for extensible structures typing.
enum class Extensibility { In, Out };
template <typename T>
inline Extensibility ExtensibilityFor;

{% for type in by_category["structure"] %}
    {% set T = as_cppType(type.name) %}
    {% if type.extensible == "in" %}
        template <>
        struct UnpackedPtrTypeFor<{{T}}> {
            using Type = UnpackedPtrChain<
                AdditionalExtensions<{{T}}>::List
                {% for extension in type.extensions %}
                    , const {{as_cppType(extension.name)}}*
                {% endfor %}
            >::Type;
        };
        template <>
        constexpr inline Extensibility ExtensibilityFor<{{T}}> = Extensibility::In;

    {% elif type.extensible == "out" %}
        template <>
        struct UnpackedPtrTypeFor<{{T}}> {
            using Type = UnpackedPtrChain<
                AdditionalExtensions<{{T}}>::List
                {% for extension in type.extensions %}
                    , {{as_cppType(extension.name)}}*
                {% endfor %}
            >::Type;
        };
        template <>
        constexpr inline Extensibility ExtensibilityFor<{{T}}> = Extensibility::Out;

    {% endif %}
{% endfor %}

}  // namespace detail

}  // namespace {{native_namespace}}

#endif  // {{DIR}}_CHAIN_UTILS_H_
