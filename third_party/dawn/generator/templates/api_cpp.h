// Copyright 2017 The Dawn & Tint Authors
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
{% from 'dawn/cpp_macros.tmpl' import wgpu_string_members with context %}

{% set API = metadata.api.upper() %}
{% set api = API.lower() %}
{% set CAPI = metadata.c_prefix %}

{% if 'dawn' in enabled_tags %}
    #ifdef __EMSCRIPTEN__
    #error "Do not include this header. Use the headers provided by Emdawnwebgpu instead."
    #endif
{% endif %}

{% set PREFIX = "" if not c_namespace else c_namespace.SNAKE_CASE() + "_" %}
#ifndef {{PREFIX}}{{API}}_CPP_H_
#define {{PREFIX}}{{API}}_CPP_H_

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <functional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

#include "{{c_header}}"
#include "{{api}}/{{api}}_cpp_chained_struct.h"
#include "{{api}}/{{api}}_enum_class_bitmasks.h"  // IWYU pragma: export

namespace {{metadata.namespace}} {

{% set c_prefix = metadata.c_prefix %}
{% for constant in by_category["constant"] %}
    {% set type = as_cppType(constant.type.name) %}
    {% if constant.cpp_value %}
        inline constexpr {{type}} k{{constant.name.CamelCase()}} = {{ constant.cpp_value }};
    {% else %}
        {% set value = c_prefix + "_" +  constant.name.SNAKE_CASE() %}
        inline constexpr {{type}} k{{constant.name.CamelCase()}} = {{ value }};
    {% endif %}
{% endfor %}

{%- macro render_c_actual_arg(arg) -%}
    {%- if arg.annotation == "value" -%}
        {%- if arg.type.category == "object" -%}
            {{as_varName(arg.name)}}.Get()
        {%- elif arg.type.category == "enum" or arg.type.category == "bitmask" -%}
            static_cast<{{as_cType(arg.type.name)}}>({{as_varName(arg.name)}})
        {%- elif arg.type.category == "structure" -%}
            *reinterpret_cast<{{as_cType(arg.type.name)}} const*>(&{{as_varName(arg.name)}})
        {%- elif arg.type.category in ["function pointer", "native"] -%}
            {{as_varName(arg.name)}}
        {%- else -%}
            UNHANDLED
        {%- endif -%}
    {%- else -%}
        reinterpret_cast<{{decorate(as_cType(arg.type.name), arg)}}>({{as_varName(arg.name)}})
    {%- endif -%}
{%- endmacro -%}

{%- macro render_cpp_to_c_method_call(type, method) -%}
    {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get()
        {%- for arg in method.arguments -%},{{" "}}{{render_c_actual_arg(arg)}}
        {%- endfor -%}
    )
{%- endmacro %}

//* Although 'optional bool' is defined as an enum value, in C++, we manually implement it to
//* provide conversion utilities.
{% for type in by_category["enum"] if type.name.get() != "optional bool" %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    enum class {{CppType}} : uint32_t {
        {% for value in type.values %}
            {{as_cppEnum(value.name)}} = {{as_cEnum(type.name, value.name)}},
        {% endfor %}
    };
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

{% for type in by_category["bitmask"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    enum class {{CppType}} : uint64_t {
        {% for value in type.values %}
            {{as_cppEnum(value.name)}} = {{as_cEnum(type.name, value.name)}},
        {% endfor %}
    };
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

// TODO(crbug.com/42241461): Update these to not be using the C callback types, and instead be
// defined using C++ types instead. Note that when we remove these, the C++ callback info types
// should also all be removed as they will no longer be necessary given the C++ templated
// functions calls and setter utilities.
{% for type in by_category["function pointer"] %}
    using {{as_cppType(type.name)}} = {{as_cType(type.name)}};
{% endfor %}

// Special class for booleans in order to allow implicit conversions.
{% set BoolCppType = as_cppType(types["bool"].name) %}
{% set BoolCType = as_cType(types["bool"].name) %}
class {{BoolCppType}} {
  public:
    constexpr {{BoolCppType}}() = default;
    explicit(false) constexpr {{BoolCppType}}(bool value) : mValue(static_cast<{{BoolCType}}>(value)) {}
    explicit(false) {{BoolCppType}}({{BoolCType}} value): mValue(value) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr operator bool() const { return static_cast<bool>(mValue); }

  private:
    friend struct std::hash<{{BoolCppType}}>;
    // Default to false.
    {{BoolCType}} mValue = static_cast<{{BoolCType}}>(false);
};

// Special class for optional booleans in order to allow conversions.
{% set OptionalBool = types["optional bool"] %}
{% set OptionalBoolCppType = as_cppType(OptionalBool.name) %}
{% set OptionalBoolCType = as_cType(OptionalBool.name) %}
{% set OptionalBoolUndefined = as_cEnum(OptionalBool.name, find_by_name(OptionalBool.values, "undefined").name) %}
class {{OptionalBoolCppType}} {
  public:
    constexpr {{OptionalBoolCppType}}() = default;
    explicit(false) constexpr {{OptionalBoolCppType}}(bool value) : mValue(static_cast<{{OptionalBoolCType}}>(value)) {}
    explicit(false) constexpr {{OptionalBoolCppType}}(std::optional<bool> value) :
        mValue(value ? static_cast<{{OptionalBoolCType}}>(*value) : {{OptionalBoolUndefined}}) {}
    explicit(false) constexpr {{OptionalBoolCppType}}({{OptionalBoolCType}} value): mValue(value) {}

    // Define the values that are equivalent to the enums.
    {% for value in OptionalBool.values %}
        static const {{OptionalBoolCppType}} {{as_cppEnum(value.name)}};
    {% endfor %}

    // Assignment operators.
    {{OptionalBoolCppType}}& operator=(const bool& value) {
        mValue = static_cast<{{OptionalBoolCType}}>(value);
        return *this;
    }
    {{OptionalBoolCppType}}& operator=(const std::optional<bool>& value) {
        mValue = value ? static_cast<{{OptionalBoolCType}}>(*value) : {{OptionalBoolUndefined}};
        return *this;
    }
    {{OptionalBoolCppType}}& operator=(const {{OptionalBoolCType}}& value) {
        mValue = value;
        return *this;
    }

    // Conversion functions.
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator {{OptionalBoolCType}}() const { return mValue; }
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator std::optional<bool>() const {
        if (mValue == {{OptionalBoolUndefined}}) {
            return std::nullopt;
        }
        return static_cast<bool>(mValue);
    }

    // Comparison functions.
    friend bool operator==(const {{OptionalBoolCppType}}& lhs, const {{OptionalBoolCppType}}& rhs) {
        return lhs.mValue == rhs.mValue;
    }
    friend bool operator!=(const {{OptionalBoolCppType}}& lhs, const {{OptionalBoolCppType}}& rhs) {
        return lhs.mValue != rhs.mValue;
    }

  private:
    friend struct std::hash<{{OptionalBoolCppType}}>;
    // Default to undefined.
    {{OptionalBoolCType}} mValue = {{OptionalBoolUndefined}};
};
{% for value in OptionalBool.values %}
    inline const {{OptionalBoolCppType}} {{OptionalBoolCppType}}::{{as_cppEnum(value.name)}} = {{OptionalBoolCppType}}({{as_cEnum(OptionalBool.name, value.name)}});
{% endfor %}

template<typename Derived, typename CType>
class ObjectBase {
  public:
    ObjectBase() = default;
    explicit(false) ObjectBase(CType handle): mHandle(handle) {
        if (mHandle) Derived::{{c_prefix}}AddRef(mHandle);
    }
    ~ObjectBase() {
        if (mHandle) Derived::{{c_prefix}}Release(mHandle);
    }

    ObjectBase(ObjectBase const& other)
        : ObjectBase(other.Get()) {
    }
    Derived& operator=(ObjectBase const& other) {
        if (&other != this) {
            if (mHandle) Derived::{{c_prefix}}Release(mHandle);
            mHandle = other.mHandle;
            if (mHandle) Derived::{{c_prefix}}AddRef(mHandle);
        }

        return static_cast<Derived&>(*this);
    }

    ObjectBase(ObjectBase&& other) {
        mHandle = other.mHandle;
        other.mHandle = nullptr;
    }
    Derived& operator=(ObjectBase&& other) {
        if (&other != this) {
            if (mHandle) Derived::{{c_prefix}}Release(mHandle);
            mHandle = other.mHandle;
            other.mHandle = nullptr;
        }

        return static_cast<Derived&>(*this);
    }

    explicit(false) ObjectBase(std::nullptr_t) {}
    Derived& operator=(std::nullptr_t) {
        if (mHandle != nullptr) {
            Derived::{{c_prefix}}Release(mHandle);
            mHandle = nullptr;
        }
        return static_cast<Derived&>(*this);
    }

    bool operator==(std::nullptr_t) const {
        return mHandle == nullptr;
    }
    bool operator!=(std::nullptr_t) const {
        return mHandle != nullptr;
    }

    explicit operator bool() const {
        return mHandle != nullptr;
    }
    CType Get() const {
        return mHandle;
    }
    CType MoveToCHandle() {
        CType result = mHandle;
        mHandle = nullptr;
        return result;
    }
    static Derived Acquire(CType handle) {
        Derived result;
        result.mHandle = handle;
        return result;
    }

  protected:
    CType mHandle = nullptr;
};

{%- macro render_cpp_default_value(member, is_struct, force_default=False, forced_default_value="") -%}
    {%- if forced_default_value -%}
        {{" "}}= {{forced_default_value}}
    {%- elif member.json_data.get("no_default", false) -%}
    {%- elif member.annotation in ["*", "const*", "const*const*"] and (is_struct or member.optional or member.default_value == "nullptr") -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "object" and (is_struct or member.optional) -%}
        {{" "}}= nullptr
    {%- elif member.type.category == "enum" -%}
        //* For enums that have an undefined value, instead of using the
        //* default, just put undefined because they should be the same.
        {%- if member.type.hasUndefined and is_struct -%}
            {{" "}}= {{as_cppType(member.type.name)}}::{{as_cppEnum(Name("undefined"))}}
        {%- elif member.default_value != None -%}
            {{" "}}= {{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
        {%- elif is_struct -%}
            {{" "}}= {}
        {%- endif -%}
    {%- elif member.type.category == "bitmask" -%}
        {%- if is_struct or member.optional -%}
            {%- if member.default_value != None -%}
                {{" "}}= {{as_cppType(member.type.name)}}::{{as_cppEnum(Name(member.default_value))}}
            {%- else -%}
                //* Bitmask types should currently always default to "none" if not
                //* explicitly set.
                {{" "}}= {{as_cppType(member.type.name)}}::{{as_cppEnum(Name("none"))}}
            {%- endif -%}
        {%- endif -%}
    {%- elif member.type.category == "native" and member.default_value != None -%}
        //* Check to see if the default value is a known constant.
        {%- set constant = find_by_name(by_category["constant"], member.default_value) -%}
        {%- if constant -%}
            {{" "}}= k{{constant.name.CamelCase()}}
        {%- else -%}
            {{" "}}= {{member.default_value}}
        {%- endif -%}
    {%- elif member.default_value != None -%}
        {{" "}}= {{member.default_value}}
    {%- elif member.type.category == "structure" and member.annotation == "value" and is_struct -%}
        {{" "}}= {}
    {%- else -%}
        {{assert(member.default_value == None)}}
        {%- if force_default -%}
            {{" "}}= {}
        {%- endif -%}
    {%- endif -%}
{%- endmacro %}

{%- macro render_member_declaration(member, make_const_member, forced_default_value="") -%}
    {{- as_annotated_cppType(member, make_const_member) }}
    {{- render_cpp_default_value(member, True, make_const_member, forced_default_value) }}
{%- endmacro %}

//* This rendering macro should ONLY be used for callback info type functions.
{%- macro render_cpp_callback_info_method_declaration(type, method, typed, const, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set MethodName = method.name.CamelCase() %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {% set CallbackInfoType = (method.arguments|last).type %}
    {% set Const = " const" if const else "" %}
    {% if typed %}
        template <typename F, typename T>
    {% else %}
        template <typename F>
    {% endif %}
    {{as_annotated_cppType(method.returns)}} {{MethodName}}(
        {%- for arg in method.arguments if arg.type.category != "callback info" -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}{{ ", "}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}{{ ", "}}
            {%- endif -%}
        {%- endfor -%}
        {%- if find_by_name(CallbackInfoType.members, "mode") -%}
            {{as_cppType(types["callback mode"].name)}} callbackMode{{ ", "}}
        {%- endif -%}
    {%- if typed -%}
        F callback, T userdata){{ Const }}
    {%- else -%}
        F callback){{ Const }}
    {%- endif -%}
{%- endmacro %}

//* This rendering macro should NOT be used for callback info type functions.
{%- macro render_cpp_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set MethodName = method.name.CamelCase() %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {{"Status" if method.returns and method.returns.type.name.get() == "status" else as_annotated_cppType(method.returns)}} {{MethodName}}(
        {%- for arg in method.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}
            {%- endif -%}
            {% if not dfn %}{{render_cpp_default_value(arg, False)}}{% endif %}
        {%- endfor -%}
    ) const
{%- endmacro %}

{%- macro render_function_call(function) -%}
    {{as_cMethodNamespaced(None, function.name, c_namespace)}}(
        {%- for arg in function.arguments -%}
            {% if not loop.first %}, {% endif %}{{render_c_actual_arg(arg)}}
        {%- endfor -%}
    )
{%- endmacro -%}

{%- if metadata.namespace != 'wgpu' %}
    // The operators of webgpu_enum_class_bitmasks.h are in the wgpu:: namespace,
    // and need to be imported into this namespace for Argument Dependent Lookup.
    WGPU_IMPORT_BITMASK_OPERATORS
{% endif %}

{% if c_namespace %}
    namespace {{c_namespace.namespace_case()}} {
{% endif %}

{% for type in by_category["object"] %}
    class {{as_cppType(type.name)}};
{% endfor %}

{% for type in by_category["structure"] %}
    struct {{as_cppType(type.name)}};
{% endfor %}

struct StringView {
    char const * data = nullptr;
    size_t length = WGPU_STRLEN;

    {{wgpu_string_members("StringView") | indent(4)}}
};

namespace detail {
constexpr size_t ConstexprMax(size_t a, size_t b) {
    return a > b ? a : b;
}

template <typename T>
inline T& AsNonConstReference(const T& value) {
    return const_cast<T&>(value);
}
}  // namespace detail

namespace detail {
// For callbacks, we support two modes:
//   1) No userdata where we allow a std::function type that can include argument captures.
//   2) Explicit typed userdata where we only allow non-capturing lambdas or function pointers.
template <typename R, typename... Args>
struct CallbackTypeBase;
template <typename R, typename... Args>
struct CallbackTypeBase<R, std::tuple<Args...>> {
    using Callback = std::function<R(Args...)>;
};
template <typename R, typename... Args>
struct CallbackTypeBase<R, std::tuple<Args...>, void> {
    using Callback = R (Args...);
};
template <typename R, typename... Args, typename T>
struct CallbackTypeBase<R, std::tuple<Args...>, T> {
    using Callback = R (Args..., T);
};

// Noexcept specializations of CallbackTypeBase.
template <typename R, typename... Args>
struct CallbackTypeBase<R, std::tuple<Args...>, std::true_type> {
    using Callback = R (Args...) noexcept;
};
template <typename R, typename... Args>
struct CallbackTypeBase<R, std::tuple<Args...>, void, std::true_type> {
    using Callback = R (Args...) noexcept;
};
template <typename R, typename... Args, typename T>
struct CallbackTypeBase<R, std::tuple<Args...>, T, std::true_type> {
    using Callback = R (Args..., T) noexcept;
};
}  // namespace detail

//* Special callbacks that require some custom code generation.
{%- set SpecialCallbacks = ["device lost callback", "uncaptured error callback"] %}

{% for type in by_category["callback function"] if type.name.get() not in SpecialCallbacks %}
    template <typename... T>
    using {{as_cppType(type.name)}} = detail::CallbackTypeBase<
        {{as_cppType(type.returns.type.name) if type.returns else "void"}},
        std::tuple<
        {%- for arg in type.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {{decorate(as_cppType(arg.type.name), arg)}}
        {%- endfor -%}
    >, T...>::Callback;
{% endfor %}
template <typename... T>
using DeviceLostCallback = detail::CallbackTypeBase<void, std::tuple<const Device&, DeviceLostReason, StringView>, T...>::Callback;
template <typename... T>
using UncapturedErrorCallback = detail::CallbackTypeBase<void, std::tuple<const Device&, ErrorType, StringView>, T...>::Callback;

{%- macro render_cpp_callback_info_method_impl(type, method, typed, const) %}
    {{render_cpp_callback_info_method_declaration(type, method, typed=typed, const=const, dfn=True)}} {
        {% set CallbackInfoType = (method.arguments|last).type %}
        {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
        {% if typed %}
            auto callbackInfo = detail::CallbackInfoHelper<{{as_cType(CallbackInfoType.name)}}, F>::Create(std::move(callback), userdata);
        {% else %}
            auto callbackInfo = detail::CallbackInfoHelper<{{as_cType(CallbackInfoType.name)}}, F>::Create(std::move(callback));
        {% endif %}
        {% if find_by_name(CallbackInfoType.members, "mode") %}
            callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
        {% endif %}
        {% if method.returns and method.returns.type.name.get() == "future" %}
            auto result = {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
                {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                    {{render_c_actual_arg(arg)}}{{", "}}
                {%- endfor -%}
            callbackInfo);
            return {{convert_cType_to_cppType(method.returns.type, 'value', 'result') | indent(4)}};
        {% else %}
            return {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
                {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                    {{render_c_actual_arg(arg)}}{{", "}}
                {%- endfor -%}
            callbackInfo);
        {% endif %}
    }
{%- endmacro %}

{% macro render_cpp_method_impl(type, method) %}
    {{render_cpp_method_declaration(type, method, dfn=True)}} {
        {% for arg in method.arguments if arg.type.has_free_members_function and arg.annotation == '*' %}
            *{{as_varName(arg.name)}} = {{as_cppType(arg.type.name)}}();
        {% endfor %}
        {% if not method.returns %}
            {{render_cpp_to_c_method_call(type, method)}};
        {% else %}
            auto result = {{render_cpp_to_c_method_call(type, method)}};
            return {{convert_cType_to_cppType(method.returns.type, 'value', 'result') | indent(8)}};
        {% endif %}
    }
{%- endmacro %}

{% for type in by_category["object"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    class {{CppType}} : public ObjectBase<{{CppType}}, {{CType}}> {
      public:
        using ObjectBase::ObjectBase;
        using ObjectBase::operator=;

        {% for method in type.methods %}
            {% if has_callbackInfoStruct(method.arguments) %}
                {{render_cpp_callback_info_method_declaration(type, method, typed=True, const=True)|indent}};
                {{render_cpp_callback_info_method_declaration(type, method, typed=False, const=True)|indent}};
            {% else %}
                inline {{render_cpp_method_declaration(type, method)}};
            {% endif %}
        {% endfor %}

        {% if CppType == "Instance" %}
            inline WaitStatus WaitAny(Future f, uint64_t timeout) const;
        {% endif %}

      private:
        friend ObjectBase<{{CppType}}, {{CType}}>;
        static inline void {{c_prefix}}AddRef({{CType}} handle);
        static inline void {{c_prefix}}Release({{CType}} handle);
    };

{% endfor %}

// ChainedStruct
{% set c_prefix = metadata.c_prefix %}
static_assert(sizeof(ChainedStruct) == sizeof({{c_prefix}}ChainedStruct),
    "sizeof mismatch for ChainedStruct");
static_assert(alignof(ChainedStruct) == alignof({{c_prefix}}ChainedStruct),
    "alignof mismatch for ChainedStruct");
static_assert(offsetof(ChainedStruct, nextInChain) == offsetof({{c_prefix}}ChainedStruct, next),
    "offsetof mismatch for ChainedStruct::nextInChain");
static_assert(offsetof(ChainedStruct, sType) == offsetof({{c_prefix}}ChainedStruct, sType),
    "offsetof mismatch for ChainedStruct::sType");

//* Renders the actual members for a struct type.
{%- macro render_cpp_struct_members(type) %}
    {% set Out = "Out" if type.output else "" %}
    {% set Const = "const" if not type.output else "" %}
    {% if type.extensible %}
        ChainedStruct{{Out}} {{Const}} * nextInChain = nullptr;
    {% endif %}
    {% for member in type.members %}
        //* We need special case handling for BindGroupLayoutEntry and it's members because in the
        //* spec we decided that the default values of the member structs will be different than
        //* the default value of those structs outside of the BindGroupLayoutEntry, i.e. the
        //* default initialized value for a BufferBindingLayout is:
        //*     {nullptr, BufferBindingType::Undefined, false, 0}
        //* but the default value of a BufferBindingLayout within the BindGroupLayoutEntry is:
        //*     {nullptr, BufferBindingType::BindingNotUsed, false, 0}
        {% if type.name.get() == "bind group layout entry" %}
            {% if member.name.canonical_case() == "buffer" %}
                {% set forced_default_value = "{ nullptr, BufferBindingType::BindingNotUsed, false, 0 }" %}
            {% elif member.name.canonical_case() == "sampler" %}
                {% set forced_default_value = "{ nullptr, SamplerBindingType::BindingNotUsed }" %}
            {% elif member.name.canonical_case() == "texture" %}
                {% set forced_default_value = "{ nullptr, TextureSampleType::BindingNotUsed, TextureViewDimension::e2D, false }" %}
            {% elif member.name.canonical_case() == "storage texture" %}
                {% set forced_default_value = "{ nullptr, StorageTextureAccess::BindingNotUsed, TextureFormat::Undefined, TextureViewDimension::e2D }" %}
            {% endif %}
        {% endif %}
        //* Special handling for callback info types where we default it to the C init macro.
        {% if member.type.category == "callback info" %}
            {% set member_declaration = as_annotated_cType(member) + " = " + CAPI + "_" + member.name.SNAKE_CASE() + "_INIT" %}
        {% else %}
            {% set member_declaration = render_member_declaration(member, type.has_free_members_function, forced_default_value) %}
        {% endif %}
        {% if type.chained and loop.first %}
            //* Align the first member after ChainedStruct to match the C struct layout.
            //* It has to be aligned both to its natural and ChainedStruct's alignment.
            static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct{{Out}}), alignof({{decorate(as_cppType(member.type.name), member)}}));
            alignas(kFirstMemberAlignment) {{member_declaration}};
        {% else %}
            {{member_declaration}};
        {% endif %}
    {%- endfor -%}
{%- endmacro %}

//* Renders aliases for the members for a struct type. This is used when dealing with callback info.
{%- macro render_cpp_struct_aliases(type) %}
    {% set CppType = as_cppType(type.name) %}
    {% if type.chained %}
        using detail::{{CppType}}::kFirstMemberAlignment;
    {% endif %}
    {% if type.extensible %}
        using detail::{{CppType}}::nextInChain;
    {% endif %}
    {% for member in type.members %}
        //* Special handling for callback info types where we default it to the C init macro.
        {% if member.type.category != "callback info" %}
            using detail::{{CppType}}::{{as_varName(member.name)}};
        {% endif %}
    {% endfor %}
{%- endmacro %}

//* Special structures that require some custom code generation.
{% set SpecialStructures = ["string view"] %}

// NOLINTBEGIN(bugprone-invalid-enum-default-initialization)

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set CppType = as_cppType(type.name) %}
    {% set Out = "Out" if type.output else "" %}
    {% set HasCallbackInfo = has_callbackInfoStruct(type.members) %}
    {% set Parents = [] %}
    //* If the struct has callback info members, create a backing struct for the members.
    {% if HasCallbackInfo %}
        namespace detail {
        struct {{CppType}} {
            {{render_cpp_struct_members(type)|indent -}}
        };
        }  // namespace detail
        {% set Parents = Parents + ["protected detail::" + CppType] %}
    {% endif %}
    {% if type.chained %}
        {% set Parents = ["ChainedStruct" + Out] + Parents %}
        {% for root in type.chain_roots %}
            // Can be chained in {{as_cppType(root.name)}}
        {% endfor %}
    {% endif %}
    {% set Inherits = (" : " + Parents | join(", ")) if Parents else "" %}
    struct {{CppType}}{{Inherits}} {
        //* We provide a default constructor for the following cases:
        //*   1) If we are a chained type, the default constructor sets the SType appropriately,
        //*      and an Init constructor version will be provided for designated initialization of
        //*      the other members.
        //*   2) If we have callback info members, we provide an Init constructor which hides the
        //*      callback info members. Because we provide an Init constructor, we also need to
        //*      explicitly provide a default constructor for general use-case.
        //*   3) If we have a free member function, we define a default constructor so that users
        //*      cannot initialize the implicit "out" type struct.
        {% if type.chained or HasCallbackInfo or type.has_free_members_function%}
            inline {{CppType}}();
        {% endif %}
        //* For chained types or types with hidden members, i.e. callback infos, we provide an
        //* Init struct for designated initializers. For chained types, this sets the sType.
        {% if type.chained or HasCallbackInfo %}
            struct Init;
            explicit(false) inline {{CppType}}(Init&& init);
        {% endif %}
        {% if type.has_free_members_function %}
            inline ~{{CppType}}();
            {{CppType}}(const {{CppType}}&) = delete;
            {{CppType}}& operator=(const {{CppType}}&) = delete;
            inline {{CppType}}({{CppType}}&&);
            inline {{CppType}}& operator=({{CppType}}&&);
        {% endif %}
        //* Provide a conversion operator to the underlying C struct type.
        // NOLINTNEXTLINE(google-explicit-constructor)
        inline operator const {{as_cType(type.name)}}&() const noexcept;

        {% if HasCallbackInfo %}
            {{ render_cpp_struct_aliases(type)|indent }}
            //* For callback info members, we need to provide setters instead of direct access
            //* to the members.
            {% for method in cpp_methods(type) %}
                {{render_cpp_callback_info_method_declaration(type, method, typed=True, const=False, dfn=False)|indent }};
                {{render_cpp_callback_info_method_declaration(type, method, typed=False, const=False, dfn=False)|indent }};
            {% endfor %}
        {% else %}
            {{ render_cpp_struct_members(type)|indent -}}
        {%- endif -%}

        {%- if type.has_free_members_function %}

          private:
            inline void FreeMembers();
            static inline void Reset({{as_cppType(type.name)}}& value);
        {% endif %}
    };

{% endfor %}
// NOLINTEND(bugprone-invalid-enum-default-initialization)

// Callback info handling is generated and/or custom implemented here to convert the types between C and C++.
namespace detail {
struct Untyped {};

// The current interface of CppFTraits exposes the following values:
//   - CppFTraits::capturing: true if the callback is a capturing callback (i.e. a lambda that
//     captures its environment).
//   - CppFTraits::PtrT: The C++ callback function pointer type.
//   - CppFTraits::ReturnT: The return type of the C++ callback function pointer.
//   - CppFTraits::BaseArgsTuple: A tuple of the C++ arguments minus the user specified typed
//     parameter.
//
// Implementation notes:
//   - The |Untyped| struct is only used to differentiate whether |T| is a user specified type
//     in which case we need to strip the type from the callback arguments for speecialization
//     deduction.
template <typename CppFT, typename CppFPtr, typename T>
struct CppFTraitsImpl;
// Specialization for raw function pointers.
template <typename CppFT, typename R, typename... CppArgs, typename T>
struct CppFTraitsImpl<CppFT, R(*)(CppArgs...), T> {
    using PtrT = R(*)(CppArgs...);
    using ReturnT = R;
    static constexpr bool capturing = false;

    static constexpr size_t NumCppArgs = sizeof...(CppArgs);
    using BaseArgsTuple = decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::type_identity<std::tuple<std::tuple_element_t<Is, std::tuple<CppArgs...>>...>>{};
    }(std::make_index_sequence<std::is_same_v<T, Untyped> ? NumCppArgs : NumCppArgs - 1>{})
    )::type;
};
// Specialization for noexcept raw function pointers.
template <typename CppFT, typename R, typename... CppArgs, typename T>
struct CppFTraitsImpl<CppFT, R(*)(CppArgs...) noexcept, T> {
    using PtrT = R(*)(CppArgs...) noexcept;
    using ReturnT = R;
    static constexpr bool capturing = false;

    static constexpr size_t NumCppArgs = sizeof...(CppArgs);
    using BaseArgsTuple = decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::type_identity<std::tuple<std::tuple_element_t<Is, std::tuple<CppArgs...>>...>>{};
    }(std::make_index_sequence<std::is_same_v<T, Untyped> ? NumCppArgs : NumCppArgs - 1>{})
    )::type;
};
// Specialization for member function pointers (lambdas);
template <typename CppFT, typename R, typename C, typename... CppArgs, typename T>
struct CppFTraitsImpl<CppFT, R(C::*)(CppArgs...) const, T> {
    using PtrT = R(*)(CppArgs...);
    using ReturnT = R;
    static constexpr bool capturing = !std::is_convertible_v<CppFT, PtrT>;

    static constexpr size_t NumCppArgs = sizeof...(CppArgs);
    using BaseArgsTuple = decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::type_identity<std::tuple<std::tuple_element_t<Is, std::tuple<CppArgs...>>...>>{};
    }(std::make_index_sequence<std::is_same_v<T, Untyped> ? NumCppArgs : NumCppArgs - 1>{})
    )::type;
};
// Specialization for noexcept member function pointers (noexcept lambdas);
template <typename CppFT, typename R, typename C, typename... CppArgs, typename T>
struct CppFTraitsImpl<CppFT, R(C::*)(CppArgs...) const noexcept, T> {
    using PtrT = R(*)(CppArgs...) noexcept;
    using ReturnT = R;
    static constexpr bool capturing = !std::is_convertible_v<CppFT, PtrT>;

    static constexpr size_t NumCppArgs = sizeof...(CppArgs);
    using BaseArgsTuple = decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::type_identity<std::tuple<std::tuple_element_t<Is, std::tuple<CppArgs...>>...>>{};
    }(std::make_index_sequence<std::is_same_v<T, Untyped> ? NumCppArgs : NumCppArgs - 1>{})
    )::type;
};
template <typename CppFT, typename T = Untyped>
struct CppFTraits : CppFTraitsImpl<CppFT, decltype(&CppFT::operator()), T> {};
template <typename R, typename... CppArgs, typename T>
struct CppFTraits<R(*)(CppArgs...), T> :
    CppFTraitsImpl<R(*)(CppArgs...), R(*)(CppArgs...), T> {};
template <typename R, typename... CppArgs, typename T>
struct CppFTraits<R(*)(CppArgs...) noexcept, T> :
    CppFTraitsImpl<R(*)(CppArgs...) noexcept, R(*)(CppArgs...) noexcept, T> {};

// CArgConverter are specialization structs that specialize a conversion from a C CallbackInfo's
// callback argument types to a set of valid C++ types. These specializations provide us a way to
// support additional C++ callback types, i.e. converting raw pointers to more C++ friendly
// structures when applicable.
template <typename CInfoT, typename CppArgs>
struct CArgConverter;
{% set SpecialCallbackInfos = [
           "device lost callback info", "uncaptured error callback info",
           "dawn load cache data callback info", "dawn store cache data callback info" ] %}
{% for type in by_category["callback info"] if type.name.get() not in SpecialCallbackInfos %}
    {% set CallbackType = find_by_name(type.members, "callback").type %}
    template <>
    struct CArgConverter<{{as_cType(type.name)}}, std::tuple<
        {%- for arg in CallbackType.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {{decorate(as_cppType(arg.type.name), arg)}}
        {%- endfor -%}
    >> {
        using Result = std::tuple<
            {%- for arg in CallbackType.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{decorate(as_cppType(arg.type.name), arg)}}
            {%- endfor -%}
        >;
        static Result Convert(
            {%- for arg in CallbackType.arguments -%}
                {%- if not loop.first %}, {% endif -%}
                {{as_annotated_cType(arg)}}
            {%- endfor -%}) {
            return std::make_tuple(
                {%- for arg in CallbackType.arguments -%}
                    {%- if not loop.first %}, {% endif -%}
                    {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}
                {%- endfor -%}
            );
        }
    };
{% endfor %}
//* Implement the custom special converters.
template <>
struct CArgConverter<WGPUDeviceLostCallbackInfo,
                     std::tuple<const Device&, DeviceLostReason, StringView>> {
    using Result = std::tuple<const Device&, DeviceLostReason, StringView>;
    static Result Convert(WGPUDevice const* device,
                          WGPUDeviceLostReason reason,
                          WGPUStringView message) {
        return std::make_tuple(std::cref(*reinterpret_cast<const Device*>(device)),
                               static_cast<DeviceLostReason>(reason), message);
    }
};
template <>
struct CArgConverter<WGPUUncapturedErrorCallbackInfo,
                     std::tuple<const Device&, ErrorType, StringView>> {
    using Result = std::tuple<const Device&, ErrorType, StringView>;
    static Result Convert(WGPUDevice const* device, WGPUErrorType type, WGPUStringView message) {
        return std::make_tuple(std::cref(*reinterpret_cast<const Device*>(device)),
                               static_cast<ErrorType>(type), message);
    }
};
{% if find_by_name(by_category["callback info"], "dawn load cache data callback info") %}
    template <>
    struct CArgConverter<WGPUDawnLoadCacheDataCallbackInfo,
                         std::tuple<std::span<const std::byte>, std::span<std::byte>>> {
        using Result = std::tuple<std::span<const std::byte>, std::span<std::byte>>;
        static Result Convert(size_t keySize, uint8_t const* key, size_t valueSize, uint8_t* value) {
            return std::make_tuple(
                std::span<const std::byte>(reinterpret_cast<const std::byte*>(key), keySize),
                std::span<std::byte>(reinterpret_cast<std::byte*>(value), valueSize)
            );
        }
    };
{% endif %}
{% if find_by_name(by_category["callback info"], "dawn store cache data callback info") %}
    template <>
    struct CArgConverter<WGPUDawnStoreCacheDataCallbackInfo,
                         std::tuple<std::span<const std::byte>, std::span<const std::byte>>> {
        using Result = std::tuple<std::span<const std::byte>, std::span<const std::byte>>;
        static Result Convert(size_t keySize, uint8_t const* key, size_t valueSize, uint8_t const* value) {
            return std::make_tuple(
                std::span<const std::byte>(reinterpret_cast<const std::byte*>(key), keySize),
                std::span<const std::byte>(reinterpret_cast<const std::byte*>(value), valueSize)
            );
        }
    };
{% endif %}

// The CallbackHelper struct implements the static functions needed to convert the base C callbacks
// into the user provided C++ callbacks. More than anything, it handles converting the real C
// callback arguments (i.e. not the userdata pointers we use internally) to C++ arguments, and
// casts the userdata pointers appropriately to ensure that everything is typed.
template <typename R, typename CInfoT, typename CppF, typename CArgsTuple, typename Indices>
struct CallbackHelperImpl;
template <typename R, typename CInfoT, typename CppF, typename... CArgs, std::size_t... Is>
struct CallbackHelperImpl<R, CInfoT, CppF, std::tuple<CArgs...>, std::index_sequence<Is...>> {
    // Implementation for callbacks without an additional typed argument. We support capturing
    // lambdas if users can specify a callback mode in this case and make an allocation.
    static R Call(std::tuple_element_t<Is, std::tuple<CArgs...>>... cArgs,
                     void* callbackParam, void*) {
        using CppFTraits = CppFTraits<CppF>;
        using Converter = CArgConverter<CInfoT, typename CppFTraits::BaseArgsTuple>;

        if constexpr (CppFTraits::capturing) {
            std::unique_ptr<CppF> callback(reinterpret_cast<CppF*>(callbackParam));
            return std::apply(*callback, Converter::Convert(cArgs...));
        } else {
            auto callback = reinterpret_cast<CppFTraits::PtrT>(callbackParam);
            return std::apply(callback, Converter::Convert(cArgs...));
        }
    }

    // Implementation for callbacks where the user specifies an additional typed argument. We do
    // not support capturing lambdas in this case since the user is already providing a typed
    // argument and would make more sense for any other state to be capturing by that argument
    // instead.
    template <typename T>
    static R Call(std::tuple_element_t<Is, std::tuple<CArgs...>>... cArgs,
                     void* callbackParam, void* userdataParam) {
        using CppFTraits = CppFTraits<CppF, T>;
        using Converter = CArgConverter<CInfoT, typename CppFTraits::BaseArgsTuple>;

        auto callback = reinterpret_cast<CppFTraits::PtrT>(callbackParam);
        auto param = std::make_tuple(static_cast<T>(userdataParam));
        return std::apply(callback, std::tuple_cat(Converter::Convert(cArgs...), param));
    }
};
template <typename CInfoT, typename F, typename CbT>
struct CallbackHelper;
template <typename CInfoT, typename F, typename R, typename... CArgs>
struct CallbackHelper<CInfoT, F, R(*)(CArgs...)> {
    static constexpr size_t NumCArgs = sizeof...(CArgs);
    using CArgsTuple = std::tuple<CArgs...>;
    static_assert(NumCArgs >= 2, "C Function pointers must have two void* trailing arguments.");
    static_assert(
        std::is_same_v<std::tuple_element_t<NumCArgs - 1, CArgsTuple>, void*>,
        "C Function pointer's last argument must be void*."
    );
    static_assert(
        std::is_same_v<std::tuple_element_t<NumCArgs - 2, CArgsTuple>, void*>,
        "C Function pointer's second to last argument must be void*."
    );

    using Impl = CallbackHelperImpl<R, CInfoT, F, CArgsTuple, std::make_index_sequence<NumCArgs - 2>>;
    static R Call(CArgs... args) {
        return Impl::Call(std::forward<CArgs>(args)...);
    }
    template <typename T>
    static R Call(CArgs... args) {
        return Impl::template Call<T>(std::forward<CArgs>(args)...);
    }
};

// The CallbackInfoHelper provides a utility to create a C CallbackInfo struct type from a C++
// callback. This allows us to de-duplicates the logic used for WebGPU structs that have
// CallbackInfo members and WebGPU API functions that return a Future.
template <typename CInfoT, typename F>
struct CallbackInfoHelper {
    static CInfoT Create(F lambda) {
        CInfoT info = {};
        if constexpr (CppFTraits<F>::capturing) {
            if constexpr (requires(CInfoT x) { x.mode; }) {
                // If we have a mode argument, then the callback is guaranteed to be called only
                // once so we can support it by moving it and making an allocation for it.
                std::unique_ptr<F> alloc = std::make_unique<F>(std::move(lambda));
                info.userdata1 = reinterpret_cast<void*>(alloc.release());
            } else {
                // MSVC <=19.39 doesn't support static_assert(false) and this was fixed in later C++
                // standards https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2593r1.html
                constexpr bool kAlwaysFalse = sizeof(F) == 0;
                static_assert(
                    kAlwaysFalse, "capturing lambdas aren't supported for repeatable callbacks"
                );
            }
        } else {
            // Otherwise, if the lambda is not capturing, that means we can refer to it as a
            // pointer directly.
            info.userdata1 = reinterpret_cast<void*>(+lambda);
        }
        info.callback = CallbackHelper<CInfoT, F, decltype(CInfoT::callback)>::Call;
        info.userdata2 = nullptr;
        return info;
    }

    template <typename T>
    requires (!CppFTraits<F>::capturing)
    static CInfoT Create(F lambda, T userdata) {
        CInfoT info = {};
        info.callback = CallbackHelper<CInfoT, F, decltype(CInfoT::callback)>::template Call<T>;
        info.userdata1 = reinterpret_cast<void*>(+lambda);
        info.userdata2 = userdata;
        return info;
    }
};

}  // namespace detail

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
// error: 'offsetof' within non-standard-layout type '{{metadata.namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
// NOLINTBEGIN(bugprone-invalid-enum-default-initialization)

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    {% set HasCallbackInfo = has_callbackInfoStruct(type.members) %}
    // {{CppType}} implementation
    {% if type.chained or has_callbackInfoStruct(type.members) %}
        {% set Out = "Out" if type.output else "" %}
        {% set Const = "const" if not type.output else "" %}
        {% if type.chained %}
            //* Default constructor for chained types sets the SType appropriately.
            {{CppType}}::{{CppType}}()
            : ChainedStruct{{Out}} { nullptr, SType::{{type.name.CamelCase()}} } {}
        {% else %}
            //* Otherwise, we need to provide a default constructor on top of the Init one.
            {{CppType}}::{{CppType}}() = default;
        {% endif %}
        //* Init struct allows for designated initializers and constructors.
        struct {{CppType}}::Init {
            {% if type.extensible or type.chained %}
                ChainedStruct{{Out}} * {{Const}} nextInChain;
            {% endif %}
            {% for member in type.members if member.type.category != "callback info" %}
                {{render_member_declaration(member, type.has_free_members_function)}};
            {% endfor %}
        };
        //* There's three cases to handle here, |type.chained|, |HasCallbackInfo|, and both. It is
        //* a bit easier to handle them individually even though it duplicates some templating a
        //* bit because of the differences in the constructor.
        {{CppType}}::{{CppType}}({{CppType}}::Init&& init) :
        {% if type.chained and HasCallbackInfo %}
                ChainedStruct{{Out}} { init.nextInChain, SType::{{type.name.CamelCase()}} },
                detail::{{CppType}} {
                    {% for member in type.members if member.type.category != "callback info" %}
                        std::move(init.{{as_varName(member.name)}}),
                    {% endfor %}
                } {}
        {% elif type.chained %}
                ChainedStruct{{Out}} { init.nextInChain, SType::{{type.name.CamelCase()}} }
                {%- for member in type.members -%}
                    ,
                    {{as_varName(member.name)}}(std::move(init.{{as_varName(member.name)}}))
                {%- endfor -%}
                {{" "}}{}
        {% elif HasCallbackInfo %}
                detail::{{CppType}} {
                    {% if type.extensible %}
                        init.nextInChain,
                    {% endif %}
                    {% for member in type.members if member.type.category != "callback info" %}
                        std::move(init.{{as_varName(member.name)}}),
                    {% endfor %}
                } {}
        {% endif %}
    {% elif type.has_free_members_function %}
        //* If the type has free member function, we also want to make sure to add a default
        //* constructor to ensure that users don't initialize the values themselves.
        {{CppType}}::{{CppType}}() = default;
    {% endif %}
    //* Add the implementation for setters for callback infos.
    {% if has_callbackInfoStruct(type.members) %}
        {% for method in cpp_methods(type) %}
            {% set memberName = as_varName((method.arguments | first).name) %}
            {% set CallbackInfoType = (method.arguments | first).type %}
            {{render_cpp_callback_info_method_declaration(type, method, typed=True, const=False, dfn=True) }} {
                static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                              "offsetof mismatch for {{CppType}}::{{memberName}}");

                assert({{memberName}}.callback == nullptr);
                {{memberName}} = detail::CallbackInfoHelper<{{as_cType(CallbackInfoType.name)}}, F>::Create(std::move(callback), userdata);
                {% if find_by_name(CallbackInfoType.members, "mode") %}
                    {{memberName}}.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
                {% endif %}
            }
            {{render_cpp_callback_info_method_declaration(type, method, typed=False, const=False, dfn=True) }} {
                assert({{memberName}}.callback == nullptr);
                {{memberName}} = detail::CallbackInfoHelper<{{as_cType(CallbackInfoType.name)}}, F>::Create(std::move(callback));
                {% if find_by_name(CallbackInfoType.members, "mode") %}
                    {{memberName}}.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
                {% endif %}
            }
        {% endfor %}
    {%- endif -%}
    {% if type.has_free_members_function %}
        {{CppType}}::~{{CppType}}() {
            FreeMembers();
        }

        {{CppType}}::{{CppType}}({{CppType}}&& rhs)
            : {% for member in type.members %}
            {%- set memberName = member.name.camelCase() -%}
            {{memberName}}(rhs.{{memberName}}){% if not loop.last %},{{"\n            "}}{% endif %}
        {% endfor -%}
        {
            Reset(rhs);
        }

        {{CppType}}& {{CppType}}::operator=({{CppType}}&& rhs) {
            if (&rhs == this) {
                return *this;
            }
            FreeMembers();
            {% for member in type.members %}
                detail::AsNonConstReference(this->{{member.name.camelCase()}}) = std::move(rhs.{{member.name.camelCase()}});
            {% endfor %}
            Reset(rhs);
            return *this;
        }

        {% if type.has_free_members_function %}
            void {{CppType}}::FreeMembers() {
                bool needsFreeing = false;
                {%- for member in type.members if member.annotation != 'value' %}
                    if (this->{{member.name.camelCase()}} != nullptr) { needsFreeing = true; }
                {%- endfor -%}
                {%- for member in type.members if member.type.name.canonical_case() == 'string view' %}
                    if (this->{{member.name.camelCase()}}.data != nullptr) { needsFreeing = true; }
                {%- endfor -%}
                if (needsFreeing) {
                    {{as_cMethodNamespaced(type.name, Name("free members"), c_namespace)}}(
                        *reinterpret_cast<{{CType}}*>(this));
                }
            }
        {% endif %}

        // static
        void {{CppType}}::Reset({{CppType}}& value) {
            {{CppType}} defaultValue{};
            {% for member in type.members %}
                detail::AsNonConstReference(value.{{member.name.camelCase()}}) = defaultValue.{{member.name.camelCase()}};
            {% endfor %}
        }
    {% endif %}

    {{CppType}}::operator const {{CType}}&() const noexcept {
        return *reinterpret_cast<const {{CType}}*>(this);
    }

    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");
    {% if type.extensible %}
        static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                "offsetof mismatch for {{CppType}}::nextInChain");
    {% endif %}
    {% for member in type.members if member.type.category != "callback info" %}
        {% set memberName = member.name.camelCase() %}
        static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                "offsetof mismatch for {{CppType}}::{{memberName}}");
    {% endfor %}

{% endfor %}
// NOLINTEND(bugprone-invalid-enum-default-initialization)
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

{% for type in by_category["object"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    // {{CppType}} implementation

    {% for method in type.methods %}
        {% if has_callbackInfoStruct(method.arguments) %}
            {{render_cpp_callback_info_method_impl(type, method, typed=True, const=True)}}
            {{render_cpp_callback_info_method_impl(type, method, typed=False, const=True)}}
        {% else %}
            {{render_cpp_method_impl(type, method)}}
        {% endif %}
    {% endfor %}

    {% if CppType == "Instance" %}
        WaitStatus Instance::WaitAny(Future f, uint64_t timeout) const {
            FutureWaitInfo waitInfo { f };
            return WaitAny(1, &waitInfo, timeout);
        }
    {% endif %}

    void {{CppType}}::{{c_prefix}}AddRef({{CType}} handle) {
        if (handle != nullptr) {
            {{as_cMethodNamespaced(type.name, Name("add ref"), c_namespace)}}(handle);
        }
    }
    void {{CppType}}::{{c_prefix}}Release({{CType}} handle) {
        if (handle != nullptr) {
            {{as_cMethodNamespaced(type.name, Name("release"), c_namespace)}}(handle);
        }
    }
    static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
    static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

{% endfor %}

{% if c_namespace %}
    }  // namespace {{c_namespace.namespace_case()}}

    {% for type in by_category["object"] %}
        using {{as_cppType(type.name)}} = {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] %}
        using {{as_cppType(type.name)}} = {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}};
    {% endfor %}
    {% for type in by_category["callback function"] %}
        template <typename... T>
        using {{as_cppType(type.name)}} = {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}}<T...>;
    {% endfor %}
{% endif %}

{% for typeDef in by_category["typedef"] %}
    // {{as_cppType(typeDef.name)}} is deprecated.
    // Use {{as_cppType(typeDef.type.name)}} instead.
    using {{as_cppType(typeDef.name)}} = {{as_cppType(typeDef.type.name)}};
{% endfor %}

// Free Functions
{% for function in by_category["function"] if not function.no_cpp %}
    {% set FunctionName = as_cppType(function.name) %}
    inline {{as_annotated_cppType(function.returns)}} {{FunctionName}}(
        {%- for arg in function.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {{as_annotated_cppType(arg)}}{{render_cpp_default_value(arg, False)}}
        {%- endfor -%}
    ) {
        {% if not function.returns %}
            {{render_function_call(function)}};
        {% else %}
            auto result = {{render_function_call(function)}};
            return {{convert_cType_to_cppType(function.returns.type, 'value', 'result')}};
        {% endif %}
    }
{% endfor %}

}  // namespace {{metadata.namespace}}

namespace wgpu {

{% for type in by_category["bitmask"] %}
    template<>
    struct IsWGPUBitmask<{{metadata.namespace}}::{{as_cppType(type.name)}}> {
        static constexpr bool enable = true;
    };

{% endfor %}

{% if "texture component swizzle" in types %}
    inline bool operator==(const TextureComponentSwizzle& s1, const TextureComponentSwizzle& s2) {
        return s1.r == s2.r && s1.g == s2.g && s1.b == s2.b && s1.a == s2.a;
    }
    inline bool operator!=(const TextureComponentSwizzle& s1, const TextureComponentSwizzle& s2) {
        return !(s1 == s2);
    }
{% endif %}

} // namespace wgpu

namespace std {
// Custom boolean class needs corresponding hash function so that it appears as a transparent bool.
template <>
struct hash<{{metadata.namespace}}::{{BoolCppType}}> {
  public:
    size_t operator()(const {{metadata.namespace}}::{{BoolCppType}} &v) const {
        return hash<bool>()(v);
    }
};
template <>
struct hash<{{metadata.namespace}}::{{OptionalBoolCppType}}> {
  public:
    size_t operator()(const {{metadata.namespace}}::{{OptionalBoolCppType}} &v) const {
        return hash<{{OptionalBoolCType}}>()(v.mValue);
    }
};
}  // namespace std

#endif // {{PREFIX}}{{API}}_CPP_H_
