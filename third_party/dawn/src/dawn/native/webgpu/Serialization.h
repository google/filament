// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_
#define SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "dawn/native/Error.h"

namespace dawn::native::webgpu {

class CaptureContext;

void WriteBytes(CaptureContext& context, const void* data, size_t size);

void Serialize(CaptureContext& context, int32_t v);
void Serialize(CaptureContext& context, uint8_t v);
void Serialize(CaptureContext& context, uint16_t v);
void Serialize(CaptureContext& context, uint32_t v);
void Serialize(CaptureContext& context, uint64_t v);
void Serialize(CaptureContext& context, float v);
void Serialize(CaptureContext& context, double v);
void Serialize(CaptureContext& context, bool v);
void Serialize(CaptureContext& context, const std::string& v);

template <typename T>
void Serialize(CaptureContext& context, T v)
    requires(std::is_same_v<T, size_t> && !std::is_same_v<T, uint64_t> &&
             !std::is_same_v<T, uint32_t>)
{
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

template <typename T>
void Serialize(CaptureContext& context, const std::vector<T>& v) {
    Serialize(context, v.size());
    for (const auto& elem : v) {
        Serialize(context, elem);
    }
}

template <typename T, size_t N>
void Serialize(CaptureContext& context, const std::array<T, N>& a) {
    for (const auto& elem : a) {
        Serialize(context, elem);
    }
}

// Serialize for enum types with uint32_t or uint64_t underlying type.
template <typename T>
void Serialize(CaptureContext& s,
               T value,
               typename std::enable_if<
                   std::is_enum<T>::value &&
                       (std::is_same<typename std::underlying_type<T>::type, uint32_t>::value ||
                        std::is_same<typename std::underlying_type<T>::type, uint64_t>::value),
                   void>::type* = nullptr) {
    return WriteBytes(s, &value, sizeof(T));
}

// Helper to call Serialize on a parameter pack.
template <typename T, typename... Ts>
constexpr auto Serialize(CaptureContext& s, const T& v, const Ts&... vs)
    -> std::enable_if_t<(sizeof...(Ts) > 0) || !std::is_enum_v<T>, void> {
    Serialize(s, v);
    Serialize(s, vs...);
}

// Helper to call Serialize on an empty parameter pack.
// Do nothing.
inline void Serialize(CaptureContext& context) {}

template <typename Derived>
class Serializable {
  public:
    friend void Serialize(CaptureContext& context, const Derived& in) {
        in.VisitAll([&](const auto&... members) { Serialize(context, members...); });
    }
};

// Helper for X macro to declare a visitable member.
#define DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(type, name, ...) \
    type name{__VA_OPT__(__VA_ARGS__)};

// Helper for X macro for visiting a visitable member.
#define DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_ARG(type, name, ...) , name

constexpr int kInternalVisitableUnusedForComma = 0;

// Helper X macro to declare members of a class or struct, along with VisitAll
// methods to call a functor on all members.
// Example usage:
//   #define MEMBERS(X)    \
//       X(int, a)         \
//       X(float, b, 42.0) \
//       X(Foo, foo, kFoo) \
//       X(Bar, bar)
//   struct MyStruct {
//    DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)
//   };
//   #undef MEMBERS
#define DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)                                                  \
    MEMBERS(DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL)                                         \
                                                                                                \
    template <typename V>                                                                       \
    constexpr auto VisitAll(V&& visit) const {                                                  \
        return [&](int, const auto&... ms) {                                                    \
            return visit(ms...);                                                                \
        }(kInternalVisitableUnusedForComma MEMBERS(DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_ARG)); \
    }

// Helper macro to define a struct or class along with VisitAll methods to call
// a functor on all members.
// Example usage:
//   #define MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   DAWN_REPLAY_SERIALIZABLE(struct, MyStruct, MEMBERS) {
//      void SomeAdditionalMethod();
//   };
//   #undef MEMBERS
#define DAWN_REPLAY_SERIALIZABLE(qualifier, Name, MEMBERS) \
    struct Name##__Contents {                              \
        DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)             \
    };                                                     \
    qualifier Name : Name##__Contents, public Serializable<Name>

// Makes a struct for a given bindgrouplayout entry type.
#define DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(VariantName, VARIANT_MEMBERS)                 \
    DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayoutEntryType##VariantName##Data,               \
                             VARIANT_MEMBERS){};                                                \
                                                                                                \
    struct BindGroupLayoutEntryType##VariantName##__Contents {                                  \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType,                    \
                                                   variantType,                                 \
                                                   BindGroupLayoutEntryType::VariantName)       \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutBinding, binding)             \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType##VariantName##Data, \
                                                   data)                                        \
                                                                                                \
        template <typename V>                                                                   \
        constexpr auto VisitAll(V&& visit) const {                                              \
            return [&](int, const auto&... ms) { return visit(ms...); }(                        \
                       kInternalVisitableUnusedForComma, variantType, binding, data);           \
        }                                                                                       \
    };                                                                                          \
    struct BindGroupLayoutEntryType##VariantName                                                \
        : BindGroupLayoutEntryType##VariantName##__Contents,                                    \
          public Serializable<BindGroupLayoutEntryType##VariantName>

// Makes a struct for a given bindgroup entry type.
#define DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(VariantName, VARIANT_MEMBERS)                        \
    DAWN_REPLAY_SERIALIZABLE(struct, BindGroupEntryType##VariantName##Data, VARIANT_MEMBERS){}; \
                                                                                                \
    struct BindGroupEntryType##VariantName##__Contents {                                        \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType,                    \
                                                   variantType,                                 \
                                                   BindGroupLayoutEntryType::VariantName)       \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(uint32_t, binding)                           \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupEntryType##VariantName##Data, data) \
                                                                                                \
        template <typename V>                                                                   \
        constexpr auto VisitAll(V&& visit) const {                                              \
            return [&](int, const auto&... ms) { return visit(ms...); }(                        \
                       kInternalVisitableUnusedForComma, variantType, binding, data);           \
        }                                                                                       \
    };                                                                                          \
    struct BindGroupEntryType##VariantName : BindGroupEntryType##VariantName##__Contents,       \
                                             public Serializable<BindGroupEntryType##VariantName>

// Makes both a CmdData and a Cmd struct for a given command name.
#define DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(CmdType, CmdName, CMD_MEMBERS)               \
    DAWN_REPLAY_SERIALIZABLE(struct, CmdType##CmdName##CmdData, CMD_MEMBERS){};        \
                                                                                       \
    struct CmdType##CmdName##Cmd##__Contents {                                         \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(CmdType, command, CmdType::CmdName) \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(CmdType##CmdName##CmdData, data)    \
                                                                                       \
        template <typename V>                                                          \
        constexpr auto VisitAll(V&& visit) const {                                     \
            return [&](int, const auto&... ms) { return visit(ms...); }(               \
                       kInternalVisitableUnusedForComma, command, data);               \
        }                                                                              \
    };                                                                                 \
    struct CmdType##CmdName##Cmd : CmdType##CmdName##Cmd##__Contents,                  \
                                   public Serializable<CmdType##CmdName##Cmd>

// Makes both a CmdData and a Cmd struct for a given command buffer command name.
#define DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(CommandBufferCommand, CmdName, CMD_MEMBERS)

// Makes both a CmdData and a Cmd struct for a given root command name.
#define DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(RootCommand, CmdName, CMD_MEMBERS)

#include "dawn/serialization/Schema.h"

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_
