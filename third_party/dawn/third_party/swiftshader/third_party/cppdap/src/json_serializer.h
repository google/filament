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

#ifndef dap_json_serializer_h
#define dap_json_serializer_h

#include "dap/protocol.h"
#include "dap/serialization.h"
#include "dap/types.h"

#include <nlohmann/json_fwd.hpp>

namespace dap {
namespace json {

struct Deserializer : public dap::Deserializer {
  explicit Deserializer(const std::string&);
  ~Deserializer();

  // dap::Deserializer compliance
  bool deserialize(boolean* v) const override;
  bool deserialize(integer* v) const override;
  bool deserialize(number* v) const override;
  bool deserialize(string* v) const override;
  bool deserialize(object* v) const override;
  bool deserialize(any* v) const override;
  size_t count() const override;
  bool array(const std::function<bool(dap::Deserializer*)>&) const override;
  bool field(const std::string& name,
             const std::function<bool(dap::Deserializer*)>&) const override;

  // Unhide base overloads
  template <typename T>
  inline bool field(const std::string& name, T* v) {
    return dap::Deserializer::field(name, v);
  }

  template <typename T,
            typename = std::enable_if<TypeOf<T>::has_custom_serialization>>
  inline bool deserialize(T* v) const {
    return dap::Deserializer::deserialize(v);
  }

  template <typename T>
  inline bool deserialize(dap::array<T>* v) const {
    return dap::Deserializer::deserialize(v);
  }

  template <typename T>
  inline bool deserialize(dap::optional<T>* v) const {
    return dap::Deserializer::deserialize(v);
  }

  template <typename T0, typename... Types>
  inline bool deserialize(dap::variant<T0, Types...>* v) const {
    return dap::Deserializer::deserialize(v);
  }

  template <typename T>
  inline bool field(const std::string& name, T* v) const {
    return dap::Deserializer::deserialize(name, v);
  }

 private:
  Deserializer(const nlohmann::json*);
  const nlohmann::json* const json;
  const bool ownsJson;
};

struct Serializer : public dap::Serializer {
  Serializer();
  ~Serializer();

  std::string dump() const;

  // dap::Serializer compliance
  bool serialize(boolean v) override;
  bool serialize(integer v) override;
  bool serialize(number v) override;
  bool serialize(const string& v) override;
  bool serialize(const dap::object& v) override;
  bool serialize(const any& v) override;
  bool array(size_t count,
             const std::function<bool(dap::Serializer*)>&) override;
  bool object(const std::function<bool(dap::FieldSerializer*)>&) override;
  void remove() override;

  // Unhide base overloads
  template <typename T,
            typename = std::enable_if<TypeOf<T>::has_custom_serialization>>
  inline bool serialize(const T& v) {
    return dap::Serializer::serialize(v);
  }

  template <typename T>
  inline bool serialize(const dap::array<T>& v) {
    return dap::Serializer::serialize(v);
  }

  template <typename T>
  inline bool serialize(const dap::optional<T>& v) {
    return dap::Serializer::serialize(v);
  }

  template <typename T0, typename... Types>
  inline bool serialize(const dap::variant<T0, Types...>& v) {
    return dap::Serializer::serialize(v);
  }

  inline bool serialize(const char* v) { return dap::Serializer::serialize(v); }

 private:
  Serializer(nlohmann::json*);
  nlohmann::json* const json;
  const bool ownsJson;
  bool removed = false;
};

}  // namespace json
}  // namespace dap

#endif  // dap_json_serializer_h