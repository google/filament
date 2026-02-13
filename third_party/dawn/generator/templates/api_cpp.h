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
    #error "Do not include this header. Emscripten already provides headers needed for {{metadata.api}}."
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
        static constexpr {{type}} k{{constant.name.CamelCase()}} = {{ constant.cpp_value }};
    {% else %}
        {% set value = c_prefix + "_" +  constant.name.SNAKE_CASE() %}
        static constexpr {{type}} k{{constant.name.CamelCase()}} = {{ value }};
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
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{BoolCppType}}(bool value) : mValue(static_cast<{{BoolCType}}>(value)) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    {{BoolCppType}}({{BoolCType}} value): mValue(value) {}

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
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}(bool value) : mValue(static_cast<{{OptionalBoolCType}}>(value)) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}(std::optional<bool> value) :
        mValue(value ? static_cast<{{OptionalBoolCType}}>(*value) : {{OptionalBoolUndefined}}) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr {{OptionalBoolCppType}}({{OptionalBoolCType}} value): mValue(value) {}

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
    operator {{OptionalBoolCType}}() const { return mValue; }
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

// Helper class to wrap Status which allows implicit conversion to bool.
// Used while callers switch to checking the Status enum instead of booleans.
// TODO(crbug.com/42241199): Remove when all callers check the enum.
struct ConvertibleStatus {
    // NOLINTNEXTLINE(runtime/explicit) allow implicit construction
    constexpr ConvertibleStatus(Status status) : status(status) {}
    // NOLINTNEXTLINE(runtime/explicit) allow implicit conversion
    constexpr operator bool() const {
        return status == Status::Success;
    }
    // NOLINTNEXTLINE(runtime/explicit) allow implicit conversion
    constexpr operator Status() const {
        return status;
    }
    Status status;
};

template<typename Derived, typename CType>
class ObjectBase {
  public:
    ObjectBase() = default;
    ObjectBase(CType handle): mHandle(handle) {
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
        other.mHandle = 0;
    }
    Derived& operator=(ObjectBase&& other) {
        if (&other != this) {
            if (mHandle) Derived::{{c_prefix}}Release(mHandle);
            mHandle = other.mHandle;
            other.mHandle = 0;
        }

        return static_cast<Derived&>(*this);
    }

    ObjectBase(std::nullptr_t) {}
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
        mHandle = 0;
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
{%- macro render_cpp_callback_info_template_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set MethodName = method.name.CamelCase() %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {% set CallbackInfoType = (method.arguments|last).type %}
    {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
    {% set SfinaeArg = " = std::enable_if_t<std::is_convertible_v<F, Cb*> || std::is_convertible_v<F, CbChar*>>" if not dfn else "" %}
    template <typename F, typename T,
              typename Cb
                {%- if not dfn -%}
                    {{" "}}= {{as_cppType(CallbackType.name)}}<T>
                {%- endif -%},
              //* The Callback fnptr with const char* instead of StringView.
              //* TODO(42241188): Remove once all clients use StringView versions of the callbacks
              typename CbChar
                {%- if not dfn -%}
                    {{" "}}= void (
                        {%- for arg in CallbackType.arguments -%}
                            {%- if arg.type.name.canonical_case() == "string view" -%}
                                const char* {{as_varName(arg.name)}}{{", "}}
                            {%- else -%}
                                {{as_annotated_cppType(arg)}}{{", "}}
                            {%- endif -%}
                        {%- endfor -%}
                    T userdata)
                {%- endif -%},
              typename{{SfinaeArg}}>
    {{as_annotated_cppType(method.returns)}} {{MethodName}}(
        {%- for arg in method.arguments if arg.type.category != "callback info" -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}{{ ", "}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}{{ ", "}}
            {%- endif -%}
        {%- endfor -%}
        {%- if find_by_name(CallbackInfoType.members, "mode") -%}
            {{as_cppType(types["callback mode"].name)}} callbackMode,
        {%- endif -%}
    F callback, T userdata) const
{%- endmacro %}

//* This rendering macro should ONLY be used for callback info type functions.
{%- macro render_cpp_callback_info_lambda_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set MethodName = method.name.CamelCase() %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {% set CallbackInfoType = (method.arguments|last).type %}
    {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
    {% set SfinaeArg = " = std::enable_if_t<std::is_convertible_v<L, Cb> || std::is_convertible_v<L, CbChar>>" if not dfn else "" %}
    template <typename L,
              typename Cb
                {%- if not dfn -%}
                    {{" "}}= {{as_cppType(CallbackType.name)}}<>
                {%- endif -%},
              //* The Callback fnptr with const char* instead of StringView.
              //* TODO(42241188): Remove once all clients use StringView versions of the callbacks
              typename CbChar
                {%- if not dfn -%}
                    {{" "}}= std::function<void(
                        {%- for arg in CallbackType.arguments -%}
                            {%- if not loop.first %}, {% endif -%}
                            {%- if arg.type.name.canonical_case() == "string view" -%}
                                const char* {{as_varName(arg.name)}}
                            {%- else -%}
                                {{as_annotated_cppType(arg)}}
                            {%- endif -%}
                        {%- endfor -%}
                    )>
                {%- endif -%},
              typename{{SfinaeArg}}>
    {{as_annotated_cppType(method.returns)}} {{MethodName}}(
        {%- for arg in method.arguments if arg.type.category != "callback info" -%}
            {%- if arg.type.category == "object" and arg.annotation == "value" -%}
                {{as_cppType(arg.type.name)}} const& {{as_varName(arg.name)}}{{ ", "}}
            {%- else -%}
                {{as_annotated_cppType(arg)}}{{ ", "}}
            {%- endif -%}
        {%- endfor -%}
    {%- if find_by_name(CallbackInfoType.members, "mode") -%}
            {{as_cppType(types["callback mode"].name)}} callbackMode,
        {%- endif -%}
    L callback) const
{%- endmacro %}

//* This rendering macro should NOT be used for callback info type functions.
{%- macro render_cpp_method_declaration(type, method, dfn=False) %}
    {% set CppType = as_cppType(type.name) %}
    {% set MethodName = method.name.CamelCase() %}
    {% set MethodName = CppType + "::" + MethodName if dfn else MethodName %}
    {{"ConvertibleStatus" if method.returns and method.returns.type.name.get() == "status" else as_annotated_cppType(method.returns)}} {{MethodName}}(
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

// TODO(42241188): Remove once all clients use StringView versions of the callbacks.
// To make MSVC happy we need a StringView constructor from the adapter, so we first need to
// forward declare StringViewAdapter here. Otherwise MSVC complains about an ambiguous conversion.
namespace detail {
    struct StringViewAdapter;
}  // namespace detail

struct StringView {
    char const * data = nullptr;
    size_t length = WGPU_STRLEN;

    {{wgpu_string_members("StringView") | indent(4)}}

    StringView(const detail::StringViewAdapter& s);
};

namespace detail {
constexpr size_t ConstexprMax(size_t a, size_t b) {
    return a > b ? a : b;
}

template <typename T>
static T& AsNonConstReference(const T& value) {
    return const_cast<T&>(value);
}

// A wrapper around StringView that can be implicitly converted to const char* with temporary
// storage that adds the \0 for output strings that are all explicitly-sized.
// TODO(42241188): Remove once all clients use StringView versions of the callbacks.
struct StringViewAdapter {
    WGPUStringView sv;
    char* nullTerminated = nullptr;

    StringViewAdapter(WGPUStringView sv) : sv(sv) {}
    ~StringViewAdapter() { delete[] nullTerminated; }
    operator ::WGPUStringView() { return sv; }
    operator StringView() { return {sv.data, sv.length}; }
    operator const char*() {
        assert(sv.length != WGPU_STRLEN);
        assert(nullTerminated == nullptr);
        nullTerminated = new char[sv.length + 1];
        for (size_t i = 0; i < sv.length; i++) {
            nullTerminated[i] = sv.data[i];
        }
        nullTerminated[sv.length] = 0;
        return nullTerminated;
    }
};
}  // namespace detail

inline StringView::StringView(const detail::StringViewAdapter& s): data(s.sv.data), length(s.sv.length) {}

namespace detail {
// For callbacks, we support two modes:
//   1) No userdata where we allow a std::function type that can include argument captures.
//   2) Explicit typed userdata where we only allow non-capturing lambdas or function pointers.
template <typename... Args>
struct CallbackTypeBase;
template <typename... Args>
struct CallbackTypeBase<std::tuple<Args...>> {
    using Callback = std::function<void(Args...)>;
};
template <typename... Args>
struct CallbackTypeBase<std::tuple<Args...>, void> {
    using Callback = void (Args...);
};
template <typename... Args, typename T>
struct CallbackTypeBase<std::tuple<Args...>, T> {
    using Callback = void (Args..., T);
};
}  // namespace detail

//* Special callbacks that require some custom code generation.
{% set SpecialCallbacks = ["device lost callback", "uncaptured error callback"] %}

{% for type in by_category["callback function"] if type.name.get() not in SpecialCallbacks %}
    template <typename... T>
    using {{as_cppType(type.name)}} = typename detail::CallbackTypeBase<std::tuple<
        {%- for arg in type.arguments -%}
            {%- if not loop.first %}, {% endif -%}
            {{decorate(as_cppType(arg.type.name), arg)}}
        {%- endfor -%}
    >, T...>::Callback;
{% endfor %}
template <typename... T>
using DeviceLostCallback = typename detail::CallbackTypeBase<std::tuple<const Device&, DeviceLostReason, StringView>, T...>::Callback;
template <typename... T>
using UncapturedErrorCallback = typename detail::CallbackTypeBase<std::tuple<const Device&, ErrorType, StringView>, T...>::Callback;

{% macro render_cpp_callback_info_template_method_impl(type, method) %}
    {{render_cpp_callback_info_template_method_declaration(type, method, dfn=True)}} {
        {% set CallbackInfoType = (method.arguments|last).type %}
        {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
        {{as_cType(CallbackInfoType.name)}} callbackInfo = {};
        {% if find_by_name(CallbackInfoType.members, "mode") %}
            callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
        {% endif %}
        if constexpr (std::is_convertible_v<F, Cb*>) {
            callbackInfo.callback = [](
                {%- for arg in CallbackType.arguments -%}
                    {{as_annotated_cType(arg)}}{{", "}}
                {%- endfor -%}
            void* callback_param, void* userdata_param) {
                auto cb = reinterpret_cast<Cb*>(callback_param);
                (*cb)(
                    {%- for arg in CallbackType.arguments -%}
                        {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}{{", "}}
                    {%- endfor -%}
                static_cast<T>(userdata_param));
            };
        } else {
            //* Handle functors that take in const char* instead of StringView.
            //* TODO(42241188): Remove once all clients use StringView versions of the callbacks
            //* and also remove CbChar at the same time.
            callbackInfo.callback = [](
                {%- for arg in CallbackType.arguments -%}
                    {{as_annotated_cType(arg)}}{{", "}}
                {%- endfor -%}
            void* callback_param, void* userdata_param) {
                auto cb = reinterpret_cast<CbChar*>(callback_param);
                (*cb)(
                    {%- for arg in CallbackType.arguments -%}
                        {%- if arg.type.name.canonical_case() == "string view" -%}
                            {detail::StringViewAdapter({{as_varName(arg.name)}})}{{", "}}
                        {%- else -%}
                            {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}{{", "}}
                        {%- endif -%}
                    {%- endfor -%}
                static_cast<T>(userdata_param));
            };
        }
        callbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
        callbackInfo.userdata2 = reinterpret_cast<void*>(userdata);
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

{% macro render_cpp_callback_info_lambda_method_impl(type, method) %}
    {{render_cpp_callback_info_lambda_method_declaration(type, method, dfn=True)}} {
        {% set CallbackInfoType = (method.arguments|last).type %}
        {% set CallbackType = find_by_name(CallbackInfoType.members, "callback").type %}
        using F = {{as_cppType(CallbackType.name)}}<void>;

        {{as_cType(CallbackInfoType.name)}} callbackInfo = {};
        {% if find_by_name(CallbackInfoType.members, "mode") %}
            callbackInfo.mode = static_cast<{{as_cType(types["callback mode"].name)}}>(callbackMode);
        {% endif %}
        if constexpr (std::is_convertible_v<L, F*>) {
            callbackInfo.callback = [](
            {%- for arg in CallbackType.arguments -%}
                {{as_annotated_cType(arg)}}{{", "}}
            {%- endfor -%}
            void* callback_param, void*) {
                auto cb = reinterpret_cast<F*>(callback_param);
                (*cb)(
                    {%- for arg in CallbackType.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}
                    {%- endfor -%});
            };
            callbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
            callbackInfo.userdata2 = nullptr;
        } else {
            auto* lambda = new L(std::move(callback));
            callbackInfo.callback = [](
                {%- for arg in CallbackType.arguments -%}
                    {{as_annotated_cType(arg)}}{{", "}}
                {%- endfor -%}
            void* callback_param, void*) {
                std::unique_ptr<L> the_lambda(reinterpret_cast<L*>(callback_param));
                (*the_lambda)(
                    {%- for arg in CallbackType.arguments -%}
                        {%- if not loop.first %}, {% endif -%}
                        //* Handle functors that take in const char* instead of StringView.
                        //* TODO(42241188): Remove once all clients use StringView versions of the callbacks
                        //* and also remove CbChar at the same time.
                        {%- if arg.type.name.canonical_case() == "string view" -%}
                            {detail::StringViewAdapter({{as_varName(arg.name)}})}
                        {%- else -%}
                            {{convert_cType_to_cppType(arg.type, arg.annotation, as_varName(arg.name))}}
                        {%- endif -%}
                    {%- endfor -%});
            };
            callbackInfo.userdata1 = reinterpret_cast<void*>(lambda);
            callbackInfo.userdata2 = nullptr;
        }
        {% if method.returns and method.returns.type.name.get() == "future" %}
            auto result = {{as_cMethodNamespaced(type.name, method.name, c_namespace)}}(Get(){{", "}}
            {%- for arg in method.arguments if arg.type.category != "callback info" -%}
                {{render_c_actual_arg(arg)}}{{", "}}
            {%- endfor -%}
            callbackInfo);
            return {{convert_cType_to_cppType(method.returns.type, 'value', 'result') | indent(8)}};
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
            {% if has_callbackInfoStruct(method) %}
                {{render_cpp_callback_info_template_method_declaration(type, method)|indent}};
                {{render_cpp_callback_info_lambda_method_declaration(type, method)|indent}};
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

//* Special structures that require some custom code generation.
{% set SpecialStructures = ["device descriptor", "string view"] %}

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set Out = "Out" if type.output else "" %}
    {% set const = "const" if not type.output else "" %}
    {% if type.chained %}
        {% for root in type.chain_roots %}
            // Can be chained in {{as_cppType(root.name)}}
        {% endfor %}
        struct {{as_cppType(type.name)}} : ChainedStruct{{Out}} {
            inline {{as_cppType(type.name)}}();

            struct Init;
            inline {{as_cppType(type.name)}}(Init&& init);
    {% else %}
        struct {{as_cppType(type.name)}} {
            {% if type.has_free_members_function %}
                inline {{as_cppType(type.name)}}();
            {% endif %}
    {% endif %}
        {% if type.has_free_members_function %}
            inline ~{{as_cppType(type.name)}}();
            {{as_cppType(type.name)}}(const {{as_cppType(type.name)}}&) = delete;
            {{as_cppType(type.name)}}& operator=(const {{as_cppType(type.name)}}&) = delete;
            inline {{as_cppType(type.name)}}({{as_cppType(type.name)}}&&);
            inline {{as_cppType(type.name)}}& operator=({{as_cppType(type.name)}}&&);
        {% endif %}
        inline operator const {{as_cType(type.name)}}&() const noexcept;

        {% if type.extensible %}
            ChainedStruct{{Out}} {{const}} * nextInChain = nullptr;
        {% endif %}
        {% for member in type.members %}
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
            {% set member_declaration = render_member_declaration(member, type.has_free_members_function, forced_default_value) %}
            {% if type.chained and loop.first %}
                //* Align the first member after ChainedStruct to match the C struct layout.
                //* It has to be aligned both to its natural and ChainedStruct's alignment.
                static constexpr size_t kFirstMemberAlignment = detail::ConstexprMax(alignof(ChainedStruct{{out}}), alignof({{decorate(as_cppType(member.type.name), member)}}));
                alignas(kFirstMemberAlignment) {{member_declaration}};
            {% else %}
                {{member_declaration}};
            {% endif %}
        {% endfor %}
        {% if type.has_free_members_function %}

          private:
            inline void FreeMembers();
            static inline void Reset({{as_cppType(type.name)}}& value);
        {% endif %}
    };

{% endfor %}

//* Device descriptor is specially implemented in C++ in order to hide callback info. Note that
//* this is placed at the end of the structs and works for the device descriptor because no other
//* structs include it as a member. In the future for these special structs, we may need to add
//* a way to order the definitions w.r.t the topology of the structs.
{% set type = types["device descriptor"] %}
{% set CppType = as_cppType(type.name) %}
namespace detail {
struct {{CppType}} {
    ChainedStruct const * nextInChain = nullptr;
    {% for member in type.members %}
        {% if member.type.category != "callback info" %}
            {{render_member_declaration(member, type.has_free_members_function)}};
        {% else %}
            {{as_annotated_cType(member)}} = {{CAPI}}_{{member.name.SNAKE_CASE()}}_INIT;
        {% endif %}
    {% endfor %}
};
}  // namespace detail
struct {{CppType}} : protected detail::{{CppType}} {
    inline operator const {{as_cType(type.name)}}&() const noexcept;

    using detail::{{CppType}}::nextInChain;
    {% for member in type.members %}
        {% if member.type.category != "callback info" %}
            using detail::{{CppType}}::{{as_varName(member.name)}};
        {% endif %}
    {% endfor %}

    inline {{CppType}}();
    struct Init;
    inline {{CppType}}(Init&& init);

    template <typename F, typename T,
              typename Cb = DeviceLostCallback<T>,
              typename = std::enable_if_t<std::is_convertible_v<F, Cb*>>>
    void SetDeviceLostCallback(CallbackMode callbackMode, F callback, T userdata);
    template <typename L,
              typename Cb = DeviceLostCallback<>,
              typename = std::enable_if_t<std::is_convertible_v<L, Cb>>>
    void SetDeviceLostCallback(CallbackMode callbackMode, L callback);

    template <typename F, typename T,
              typename Cb = UncapturedErrorCallback<T>,
              typename = std::enable_if_t<std::is_convertible_v<F, Cb*>>>
    void SetUncapturedErrorCallback(F callback, T userdata);
    template <typename L,
              typename Cb = UncapturedErrorCallback<>,
              typename = std::enable_if_t<std::is_convertible_v<L, Cb>>>
    void SetUncapturedErrorCallback(L callback);
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
// error: 'offsetof' within non-standard-layout type '{{metadata.namespace}}::XXX' is conditionally-supported
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

{% for type in by_category["structure"] if type.name.get() not in SpecialStructures %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    // {{CppType}} implementation
    {% if type.chained %}
        {% set Out = "Out" if type.output else "" %}
        {% set const = "const" if not type.output else "" %}
        {{CppType}}::{{CppType}}()
          : ChainedStruct{{Out}} { nullptr, SType::{{type.name.CamelCase()}} } {}
        struct {{CppType}}::Init {
            ChainedStruct{{Out}} * {{const}} nextInChain;
            {% for member in type.members %}
                {{render_member_declaration(member, type.has_free_members_function)}};
            {% endfor %}
        };
        {{CppType}}::{{CppType}}({{CppType}}::Init&& init)
          : ChainedStruct{{Out}} { init.nextInChain, SType::{{type.name.CamelCase()}} }
            {%- for member in type.members -%},{{" "}}
                {{as_varName(member.name)}}(std::move(init.{{as_varName(member.name)}}))
            {%- endfor -%}
            {}
    {% elif type.has_free_members_function %}
        {{CppType}}::{{CppType}}() = default;
    {% endif %}
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
    {% for member in type.members %}
        {% set memberName = member.name.camelCase() %}
        static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                "offsetof mismatch for {{CppType}}::{{memberName}}");
    {% endfor %}

{% endfor %}
//* Special implementation for device descriptor.
{% set type = types["device descriptor"] %}
{% set CppType = as_cppType(type.name) %}
{% set CType = as_cType(type.name) %}
// {{CppType}} implementation

{{CppType}}::operator const {{CType}}&() const noexcept {
    return *reinterpret_cast<const {{CType}}*>(this);
}

{{CppType}}::{{CppType}}() : detail::{{CppType}} {} {
    static_assert(offsetof({{CppType}}, nextInChain) == offsetof({{CType}}, nextInChain),
                "offsetof mismatch for {{CppType}}::nextInChain");
    {% for member in type.members %}
        {% set memberName = member.name.camelCase() %}
        static_assert(offsetof({{CppType}}, {{memberName}}) == offsetof({{CType}}, {{memberName}}),
                "offsetof mismatch for {{CppType}}::{{memberName}}");
    {% endfor %}
}

struct {{CppType}}::Init {
    ChainedStruct const * nextInChain;
    {% for member in type.members if member.type.category != "callback info" %}
        {{render_member_declaration(member, type.has_free_members_function)}};
    {% endfor %}
};

{{CppType}}::{{CppType}}({{CppType}}::Init&& init) : detail::{{CppType}} {
    init.nextInChain
    {%- for member in type.members if member.type.category != "callback info" -%},{{" "}}
        std::move(init.{{as_varName(member.name)}})
    {%- endfor -%}
} {}

static_assert(sizeof({{CppType}}) == sizeof({{CType}}), "sizeof mismatch for {{CppType}}");
static_assert(alignof({{CppType}}) == alignof({{CType}}), "alignof mismatch for {{CppType}}");

template <typename F, typename T, typename Cb, typename>
void {{CppType}}::SetDeviceLostCallback(CallbackMode callbackMode, F callback, T userdata) {
    assert(deviceLostCallbackInfo.callback == nullptr);

    deviceLostCallbackInfo.mode = static_cast<WGPUCallbackMode>(callbackMode);
    deviceLostCallbackInfo.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, void* callback_param, void* userdata_param) {
        auto cb = reinterpret_cast<Cb*>(callback_param);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<DeviceLostReason>(reason), message, static_cast<T>(userdata_param));
        apiDevice.MoveToCHandle();
    };
    deviceLostCallbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
    deviceLostCallbackInfo.userdata2 = reinterpret_cast<void*>(userdata);
}

template <typename L, typename Cb, typename>
void {{CppType}}::SetDeviceLostCallback(CallbackMode callbackMode, L callback) {
    assert(deviceLostCallbackInfo.callback == nullptr);
    using F = DeviceLostCallback<void>;

    deviceLostCallbackInfo.mode = static_cast<WGPUCallbackMode>(callbackMode);
    if constexpr (std::is_convertible_v<L, F*>) {
        deviceLostCallbackInfo.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, void* callback_param, void*) {
            auto cb = reinterpret_cast<F*>(callback_param);
            // We manually acquire and release the device to avoid changing any ref counts.
            auto apiDevice = Device::Acquire(*device);
            (*cb)(apiDevice, static_cast<DeviceLostReason>(reason), message);
            apiDevice.MoveToCHandle();
        };
        deviceLostCallbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
        deviceLostCallbackInfo.userdata2 = nullptr;
    } else {
        auto* lambda = new L(std::move(callback));
        deviceLostCallbackInfo.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, void* callback_param, void*) {
            std::unique_ptr<L> the_lambda(reinterpret_cast<L*>(callback_param));
            // We manually acquire and release the device to avoid changing any ref counts.
            auto apiDevice = Device::Acquire(*device);
            (*the_lambda)(apiDevice, static_cast<DeviceLostReason>(reason), message);
            apiDevice.MoveToCHandle();
        };
        deviceLostCallbackInfo.userdata1 = reinterpret_cast<void*>(lambda);
        deviceLostCallbackInfo.userdata2 = nullptr;
    }
}

template <typename F, typename T, typename Cb, typename>
void {{CppType}}::SetUncapturedErrorCallback(F callback, T userdata) {
    assert(uncapturedErrorCallbackInfo.callback == nullptr);

    uncapturedErrorCallbackInfo.callback = [](WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, void* callback_param, void* userdata_param) {
        auto cb = reinterpret_cast<Cb*>(callback_param);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<ErrorType>(type), message, static_cast<T>(userdata_param));
        apiDevice.MoveToCHandle();
    };
    uncapturedErrorCallbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
    uncapturedErrorCallbackInfo.userdata2 = reinterpret_cast<void*>(userdata);
}

template <typename L, typename Cb, typename>
void {{CppType}}::SetUncapturedErrorCallback(L callback) {
    assert(uncapturedErrorCallbackInfo.callback == nullptr);
    using F = UncapturedErrorCallback<void>;
    static_assert(std::is_convertible_v<L, F*>, "Uncaptured error callback cannot be a binding lambda");

    uncapturedErrorCallbackInfo.callback = [](WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, void* callback_param, void*) {
        auto cb = reinterpret_cast<F*>(callback_param);
        // We manually acquire and release the device to avoid changing any ref counts.
        auto apiDevice = Device::Acquire(*device);
        (*cb)(apiDevice, static_cast<ErrorType>(type), message);
        apiDevice.MoveToCHandle();
    };
    uncapturedErrorCallbackInfo.userdata1 = reinterpret_cast<void*>(+callback);
    uncapturedErrorCallbackInfo.userdata2 = nullptr;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

{% for type in by_category["object"] %}
    {% set CppType = as_cppType(type.name) %}
    {% set CType = as_cType(type.name) %}
    // {{CppType}} implementation

    {% for method in type.methods %}
        {% if has_callbackInfoStruct(method) %}
            {{render_cpp_callback_info_template_method_impl(type, method)}}
            {{render_cpp_callback_info_lambda_method_impl(type, method)}}
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
        using {{as_cppType(type.name)}} = typename {{c_namespace.namespace_case()}}::{{as_cppType(type.name)}}<T...>;
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
    static inline {{as_annotated_cppType(function.returns)}} {{FunctionName}}(
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
