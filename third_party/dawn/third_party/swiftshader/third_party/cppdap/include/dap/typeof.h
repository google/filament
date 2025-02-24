// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef dap_typeof_h
#define dap_typeof_h

#include "typeinfo.h"
#include "types.h"

#include "serialization.h"

namespace dap {

// BasicTypeInfo is an implementation of the TypeInfo interface for the simple
// template type T.
template <typename T>
struct BasicTypeInfo : public TypeInfo {
  constexpr BasicTypeInfo(std::string&& name) : name_(std::move(name)) {}

  // TypeInfo compliance
  inline std::string name() const override { return name_; }
  inline size_t size() const override { return sizeof(T); }
  inline size_t alignment() const override { return alignof(T); }
  inline void construct(void* ptr) const override { new (ptr) T(); }
  inline void copyConstruct(void* dst, const void* src) const override {
    new (dst) T(*reinterpret_cast<const T*>(src));
  }
  inline void destruct(void* ptr) const override {
    reinterpret_cast<T*>(ptr)->~T();
  }
  inline bool deserialize(const Deserializer* d, void* ptr) const override {
    return d->deserialize(reinterpret_cast<T*>(ptr));
  }
  inline bool serialize(Serializer* s, const void* ptr) const override {
    return s->serialize(*reinterpret_cast<const T*>(ptr));
  }

 private:
  std::string name_;
};

// TypeOf has a template specialization for each DAP type, each declaring a
// const TypeInfo* type() static member function that describes type T.
template <typename T>
struct TypeOf {};

template <>
struct TypeOf<boolean> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<string> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<integer> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<number> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<object> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<any> {
  static const TypeInfo* type();
};

template <>
struct TypeOf<null> {
  static const TypeInfo* type();
};

template <typename T>
struct TypeOf<array<T>> {
  static inline const TypeInfo* type() {
    static auto typeinfo = TypeInfo::create<BasicTypeInfo<array<T>>>(
        "array<" + TypeOf<T>::type()->name() + ">");
    return typeinfo;
  }
};

template <typename T0, typename... Types>
struct TypeOf<variant<T0, Types...>> {
  static inline const TypeInfo* type() {
    static auto typeinfo =
        TypeInfo::create<BasicTypeInfo<variant<T0, Types...>>>("variant");
    return typeinfo;
  }
};

template <typename T>
struct TypeOf<optional<T>> {
  static inline const TypeInfo* type() {
    static auto typeinfo = TypeInfo::create<BasicTypeInfo<optional<T>>>(
        "optional<" + TypeOf<T>::type()->name() + ">");
    return typeinfo;
  }
};

// DAP_OFFSETOF() macro is a generalization of the offsetof() macro defined in
// <cstddef>. It evaluates to the offset of the given field, with fewer
// restrictions than offsetof(). We cast the address '32' and subtract it again,
// because null-dereference is undefined behavior.
#define DAP_OFFSETOF(s, m) \
  ((int)(size_t) & reinterpret_cast<const volatile char&>((((s*)32)->m)) - 32)

// internal functionality
namespace detail {
template <class T, class M>
M member_type(M T::*);
}  // namespace detail

// DAP_TYPEOF() returns the type of the struct (s) member (m).
#define DAP_TYPEOF(s, m) decltype(detail::member_type(&s::m))

// DAP_FIELD() declares a structure field for the DAP_IMPLEMENT_STRUCT_TYPEINFO
// macro.
// FIELD is the name of the struct field.
// NAME is the serialized name of the field, as described by the DAP
// specification.
#define DAP_FIELD(FIELD, NAME)                       \
  ::dap::Field {                                     \
    NAME, DAP_OFFSETOF(StructTy, FIELD),             \
        TypeOf<DAP_TYPEOF(StructTy, FIELD)>::type(), \
  }

// DAP_DECLARE_STRUCT_TYPEINFO() declares a TypeOf<> specialization for STRUCT.
// Must be used within the 'dap' namespace.
#define DAP_DECLARE_STRUCT_TYPEINFO(STRUCT)                         \
  template <>                                                       \
  struct TypeOf<STRUCT> {                                           \
    static constexpr bool has_custom_serialization = true;          \
    static const TypeInfo* type();                                  \
    static bool deserializeFields(const Deserializer*, void* obj);  \
    static bool serializeFields(FieldSerializer*, const void* obj); \
  }

// DAP_IMPLEMENT_STRUCT_FIELD_SERIALIZATION() implements the deserializeFields()
// and serializeFields() static methods of a TypeOf<> specialization. Used
// internally by DAP_IMPLEMENT_STRUCT_TYPEINFO() and
// DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT().
// You probably do not want to use this directly.
#define DAP_IMPLEMENT_STRUCT_FIELD_SERIALIZATION(STRUCT, NAME, ...)           \
  bool TypeOf<STRUCT>::deserializeFields(const Deserializer* d, void* obj) {  \
    using StructTy = STRUCT;                                                  \
    (void)sizeof(StructTy); /* avoid unused 'using' warning */                \
    for (auto field : std::initializer_list<Field>{__VA_ARGS__}) {            \
      if (!d->field(field.name, [&](Deserializer* d) {                        \
            auto ptr = reinterpret_cast<uint8_t*>(obj) + field.offset;        \
            return field.type->deserialize(d, ptr);                           \
          })) {                                                               \
        return false;                                                         \
      }                                                                       \
    }                                                                         \
    return true;                                                              \
  }                                                                           \
  bool TypeOf<STRUCT>::serializeFields(FieldSerializer* s, const void* obj) { \
    using StructTy = STRUCT;                                                  \
    (void)sizeof(StructTy); /* avoid unused 'using' warning */                \
    for (auto field : std::initializer_list<Field>{__VA_ARGS__}) {            \
      if (!s->field(field.name, [&](Serializer* s) {                          \
            auto ptr = reinterpret_cast<const uint8_t*>(obj) + field.offset;  \
            return field.type->serialize(s, ptr);                             \
          })) {                                                               \
        return false;                                                         \
      }                                                                       \
    }                                                                         \
    return true;                                                              \
  }

// DAP_IMPLEMENT_STRUCT_TYPEINFO() implements the type() member function for the
// TypeOf<> specialization for STRUCT.
// STRUCT is the structure typename.
// NAME is the serialized name of the structure, as described by the DAP
// specification. The variadic (...) parameters should be a repeated list of
// DAP_FIELD()s, one for each field of the struct.
// Must be used within the 'dap' namespace.
#define DAP_IMPLEMENT_STRUCT_TYPEINFO(STRUCT, NAME, ...)                    \
  DAP_IMPLEMENT_STRUCT_FIELD_SERIALIZATION(STRUCT, NAME, __VA_ARGS__)       \
  const ::dap::TypeInfo* TypeOf<STRUCT>::type() {                           \
    struct TI : BasicTypeInfo<STRUCT> {                                     \
      TI() : BasicTypeInfo<STRUCT>(NAME) {}                                 \
      bool deserialize(const Deserializer* d, void* obj) const override {   \
        return deserializeFields(d, obj);                                   \
      }                                                                     \
      bool serialize(Serializer* s, const void* obj) const override {       \
        return s->object(                                                   \
            [&](FieldSerializer* fs) { return serializeFields(fs, obj); }); \
      }                                                                     \
    };                                                                      \
    static TI typeinfo;                                                     \
    return &typeinfo;                                                       \
  }

// DAP_STRUCT_TYPEINFO() is a helper for declaring and implementing a TypeOf<>
// specialization for STRUCT in a single statement.
// Must be used within the 'dap' namespace.
#define DAP_STRUCT_TYPEINFO(STRUCT, NAME, ...) \
  DAP_DECLARE_STRUCT_TYPEINFO(STRUCT);         \
  DAP_IMPLEMENT_STRUCT_TYPEINFO(STRUCT, NAME, __VA_ARGS__)

// DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT() implements the type() member function for
// the TypeOf<> specialization for STRUCT that derives from BASE.
// STRUCT is the structure typename.
// BASE is the base structure typename.
// NAME is the serialized name of the structure, as described by the DAP
// specification. The variadic (...) parameters should be a repeated list of
// DAP_FIELD()s, one for each field of the struct.
// Must be used within the 'dap' namespace.
#define DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT(STRUCT, BASE, NAME, ...)        \
  static_assert(std::is_base_of<BASE, STRUCT>::value,                     \
                #STRUCT " does not derive from " #BASE);                  \
  DAP_IMPLEMENT_STRUCT_FIELD_SERIALIZATION(STRUCT, NAME, __VA_ARGS__)     \
  const ::dap::TypeInfo* TypeOf<STRUCT>::type() {                         \
    struct TI : BasicTypeInfo<STRUCT> {                                   \
      TI() : BasicTypeInfo<STRUCT>(NAME) {}                               \
      bool deserialize(const Deserializer* d, void* obj) const override { \
        auto derived = static_cast<STRUCT*>(obj);                         \
        auto base = static_cast<BASE*>(obj);                              \
        return TypeOf<BASE>::deserializeFields(d, base) &&                \
               deserializeFields(d, derived);                             \
      }                                                                   \
      bool serialize(Serializer* s, const void* obj) const override {     \
        return s->object([&](FieldSerializer* fs) {                       \
          auto derived = static_cast<const STRUCT*>(obj);                 \
          auto base = static_cast<const BASE*>(obj);                      \
          return TypeOf<BASE>::serializeFields(fs, base) &&               \
                 serializeFields(fs, derived);                            \
        });                                                               \
      }                                                                   \
    };                                                                    \
    static TI typeinfo;                                                   \
    return &typeinfo;                                                     \
  }

// DAP_STRUCT_TYPEINFO_EXT() is a helper for declaring and implementing a
// TypeOf<> specialization for STRUCT that derives from BASE in a single
// statement.
// Must be used within the 'dap' namespace.
#define DAP_STRUCT_TYPEINFO_EXT(STRUCT, BASE, NAME, ...) \
  DAP_DECLARE_STRUCT_TYPEINFO(STRUCT);                   \
  DAP_IMPLEMENT_STRUCT_TYPEINFO_EXT(STRUCT, BASE, NAME, __VA_ARGS__)

}  // namespace dap

#endif  // dap_typeof_h
