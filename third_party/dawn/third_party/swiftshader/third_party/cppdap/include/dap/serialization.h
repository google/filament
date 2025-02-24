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

#ifndef dap_serialization_h
#define dap_serialization_h

#include "typeof.h"
#include "types.h"

#include <cstddef>  // ptrdiff_t
#include <type_traits>

namespace dap {

// Field describes a single field of a struct.
struct Field {
  std::string name;      // name of the field
  ptrdiff_t offset;      // offset of the field to the base of the struct
  const TypeInfo* type;  // type of the field
};

////////////////////////////////////////////////////////////////////////////////
// Deserializer
////////////////////////////////////////////////////////////////////////////////

// Deserializer is the interface used to decode data from structured storage.
// Methods that return a bool use this to indicate success.
class Deserializer {
 public:
  virtual ~Deserializer() = default;

  // deserialization methods for simple data types.
  // If the stored object is not of the correct type, then these function will
  // return false.
  virtual bool deserialize(boolean*) const = 0;
  virtual bool deserialize(integer*) const = 0;
  virtual bool deserialize(number*) const = 0;
  virtual bool deserialize(string*) const = 0;
  virtual bool deserialize(object*) const = 0;
  virtual bool deserialize(any*) const = 0;

  // count() returns the number of elements in the array object referenced by
  // this Deserializer.
  virtual size_t count() const = 0;

  // array() calls the provided std::function for deserializing each array
  // element in the array object referenced by this Deserializer.
  virtual bool array(const std::function<bool(Deserializer*)>&) const = 0;

  // field() calls the provided std::function for deserializing the field with
  // the given name from the struct object referenced by this Deserializer.
  virtual bool field(const std::string& name,
                     const std::function<bool(Deserializer*)>&) const = 0;

  // deserialize() delegates to TypeOf<T>::type()->deserialize().
  template <typename T,
            typename = std::enable_if<TypeOf<T>::has_custom_serialization>>
  inline bool deserialize(T*) const;

  // deserialize() decodes an array.
  template <typename T>
  inline bool deserialize(dap::array<T>*) const;

  // deserialize() decodes an optional.
  template <typename T>
  inline bool deserialize(dap::optional<T>*) const;

  // deserialize() decodes an variant.
  template <typename T0, typename... Types>
  inline bool deserialize(dap::variant<T0, Types...>*) const;

  // deserialize() decodes the struct field f with the given name.
  template <typename T>
  inline bool field(const std::string& name, T* f) const;
};

template <typename T, typename>
bool Deserializer::deserialize(T* ptr) const {
  return TypeOf<T>::type()->deserialize(this, ptr);
}

template <typename T>
bool Deserializer::deserialize(dap::array<T>* vec) const {
  auto n = count();
  vec->resize(n);
  size_t i = 0;
  if (!array([&](Deserializer* d) { return d->deserialize(&(*vec)[i++]); })) {
    return false;
  }
  return true;
}

template <typename T>
bool Deserializer::deserialize(dap::optional<T>* opt) const {
  T v;
  if (deserialize(&v)) {
    *opt = v;
  }
  return true;
}

template <typename T0, typename... Types>
bool Deserializer::deserialize(dap::variant<T0, Types...>* var) const {
  return deserialize(&var->value);
}

template <typename T>
bool Deserializer::field(const std::string& name, T* v) const {
  return this->field(name,
                     [&](const Deserializer* d) { return d->deserialize(v); });
}

////////////////////////////////////////////////////////////////////////////////
// Serializer
////////////////////////////////////////////////////////////////////////////////
class FieldSerializer;

// Serializer is the interface used to encode data to structured storage.
// A Serializer is associated with a single storage object, whos type and value
// is assigned by a call to serialize().
// If serialize() is called multiple times on the same Serializer instance,
// the last type and value is stored.
// Methods that return a bool use this to indicate success.
class Serializer {
 public:
  virtual ~Serializer() = default;

  // serialization methods for simple data types.
  virtual bool serialize(boolean) = 0;
  virtual bool serialize(integer) = 0;
  virtual bool serialize(number) = 0;
  virtual bool serialize(const string&) = 0;
  virtual bool serialize(const dap::object&) = 0;
  virtual bool serialize(const any&) = 0;

  // array() encodes count array elements to the array object referenced by this
  // Serializer. The std::function will be called count times, each time with a
  // Serializer that should be used to encode the n'th array element's data.
  virtual bool array(size_t count, const std::function<bool(Serializer*)>&) = 0;

  // object() begins encoding the object referenced by this Serializer.
  // The std::function will be called with a FieldSerializer to serialize the
  // object's fields.
  virtual bool object(const std::function<bool(dap::FieldSerializer*)>&) = 0;

  // remove() deletes the object referenced by this Serializer.
  // remove() can be used to serialize optionals with no value assigned.
  virtual void remove() = 0;

  // serialize() delegates to TypeOf<T>::type()->serialize().
  template <typename T,
            typename = std::enable_if<TypeOf<T>::has_custom_serialization>>
  inline bool serialize(const T&);

  // serialize() encodes the given array.
  template <typename T>
  inline bool serialize(const dap::array<T>&);

  // serialize() encodes the given optional.
  template <typename T>
  inline bool serialize(const dap::optional<T>& v);

  // serialize() encodes the given variant.
  template <typename T0, typename... Types>
  inline bool serialize(const dap::variant<T0, Types...>&);

  // deserialize() encodes the given string.
  inline bool serialize(const char* v);
};

template <typename T, typename>
bool Serializer::serialize(const T& object) {
  return TypeOf<T>::type()->serialize(this, &object);
}

template <typename T>
bool Serializer::serialize(const dap::array<T>& vec) {
  auto it = vec.begin();
  return array(vec.size(), [&](Serializer* s) { return s->serialize(*it++); });
}

template <typename T>
bool Serializer::serialize(const dap::optional<T>& opt) {
  if (!opt.has_value()) {
    remove();
    return true;
  }
  return serialize(opt.value());
}

template <typename T0, typename... Types>
bool Serializer::serialize(const dap::variant<T0, Types...>& var) {
  return serialize(var.value);
}

bool Serializer::serialize(const char* v) {
  return serialize(std::string(v));
}

////////////////////////////////////////////////////////////////////////////////
// FieldSerializer
////////////////////////////////////////////////////////////////////////////////

// FieldSerializer is the interface used to serialize fields of an object.
class FieldSerializer {
 public:
  using SerializeFunc = std::function<bool(Serializer*)>;
  template <typename T>
  using IsSerializeFunc = std::is_convertible<T, SerializeFunc>;

  virtual ~FieldSerializer() = default;

  // field() encodes a field to the struct object referenced by this Serializer.
  // The SerializeFunc will be called with a Serializer used to encode the
  // field's data.
  virtual bool field(const std::string& name, const SerializeFunc&) = 0;

  // field() encodes the field with the given name and value.
  template <
      typename T,
      typename = typename std::enable_if<!IsSerializeFunc<T>::value>::type>
  inline bool field(const std::string& name, const T& v);
};

template <typename T, typename>
bool FieldSerializer::field(const std::string& name, const T& v) {
  return this->field(name, [&](Serializer* s) { return s->serialize(v); });
}

}  // namespace dap

#endif  // dap_serialization_h
