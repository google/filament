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

#include "json_serializer.h"

// Disable JSON exceptions. We should be guarding against any exceptions being
// fired in this file.
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>

namespace {

struct NullDeserializer : public dap::Deserializer {
  static NullDeserializer instance;

  bool deserialize(dap::boolean*) const override { return false; }
  bool deserialize(dap::integer*) const override { return false; }
  bool deserialize(dap::number*) const override { return false; }
  bool deserialize(dap::string*) const override { return false; }
  bool deserialize(dap::object*) const override { return false; }
  bool deserialize(dap::any*) const override { return false; }
  size_t count() const override { return 0; }
  bool array(const std::function<bool(dap::Deserializer*)>&) const override {
    return false;
  }
  bool field(const std::string&,
             const std::function<bool(dap::Deserializer*)>&) const override {
    return false;
  }
};

NullDeserializer NullDeserializer::instance;

}  // anonymous namespace

namespace dap {
namespace json {

Deserializer::Deserializer(const std::string& str)
    : json(new nlohmann::json(nlohmann::json::parse(str, nullptr, false))),
      ownsJson(true) {}

Deserializer::Deserializer(const nlohmann::json* json)
    : json(json), ownsJson(false) {}

Deserializer::~Deserializer() {
  if (ownsJson) {
    delete json;
  }
}

bool Deserializer::deserialize(dap::boolean* v) const {
  if (!json->is_boolean()) {
    return false;
  }
  *v = json->get<bool>();
  return true;
}

bool Deserializer::deserialize(dap::integer* v) const {
  if (!json->is_number_integer()) {
    return false;
  }
  *v = json->get<int>();
  return true;
}

bool Deserializer::deserialize(dap::number* v) const {
  if (!json->is_number()) {
    return false;
  }
  *v = json->get<double>();
  return true;
}

bool Deserializer::deserialize(dap::string* v) const {
  if (!json->is_string()) {
    return false;
  }
  *v = json->get<std::string>();
  return true;
}

bool Deserializer::deserialize(dap::object* v) const {
  v->reserve(json->size());
  for (auto& el : json->items()) {
    Deserializer d(&el.value());
    dap::any val;
    if (!d.deserialize(&val)) {
      return false;
    }
    (*v)[el.key()] = val;
  }
  return true;
}

bool Deserializer::deserialize(dap::any* v) const {
  if (json->is_boolean()) {
    *v = dap::boolean(json->get<bool>());
  } else if (json->is_number_float()) {
    *v = dap::number(json->get<double>());
  } else if (json->is_number_integer()) {
    *v = dap::integer(json->get<int>());
  } else if (json->is_string()) {
    *v = json->get<std::string>();
  } else if (json->is_null()) {
    *v = null();
  } else {
    return false;
  }
  return true;
}

size_t Deserializer::count() const {
  return json->size();
}

bool Deserializer::array(
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->is_array()) {
    return false;
  }
  for (size_t i = 0; i < json->size(); i++) {
    Deserializer d(&(*json)[i]);
    if (!cb(&d)) {
      return false;
    }
  }
  return true;
}

bool Deserializer::field(
    const std::string& name,
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->is_structured()) {
    return false;
  }
  auto it = json->find(name);
  if (it == json->end()) {
    return cb(&NullDeserializer::instance);
  }
  auto obj = *it;
  Deserializer d(&obj);
  return cb(&d);
}

Serializer::Serializer() : json(new nlohmann::json()), ownsJson(true) {}

Serializer::Serializer(nlohmann::json* json) : json(json), ownsJson(false) {}

Serializer::~Serializer() {
  if (ownsJson) {
    delete json;
  }
}

std::string Serializer::dump() const {
  return json->dump();
}

bool Serializer::serialize(dap::boolean v) {
  *json = (bool)v;
  return true;
}

bool Serializer::serialize(dap::integer v) {
  *json = (int)v;
  return true;
}

bool Serializer::serialize(dap::number v) {
  *json = (double)v;
  return true;
}

bool Serializer::serialize(const dap::string& v) {
  *json = v;
  return true;
}

bool Serializer::serialize(const dap::object& v) {
  for (auto& it : v) {
    Serializer s(&(*json)[it.first]);
    if (!s.serialize(it.second)) {
      return false;
    }
  }
  return true;
}

bool Serializer::serialize(const dap::any& v) {
  if (v.is<dap::boolean>()) {
    *json = (bool)v.get<dap::boolean>();
  } else if (v.is<dap::integer>()) {
    *json = (int)v.get<dap::integer>();
  } else if (v.is<dap::number>()) {
    *json = (double)v.get<dap::number>();
  } else if (v.is<dap::string>()) {
    *json = v.get<dap::string>();
  } else if (v.is<dap::null>()) {
  } else {
    return false;
  }

  return true;
}

bool Serializer::array(size_t count,
                       const std::function<bool(dap::Serializer*)>& cb) {
  *json = std::vector<int>();
  for (size_t i = 0; i < count; i++) {
    Serializer s(&(*json)[i]);
    if (!cb(&s)) {
      return false;
    }
  }
  return true;
}

bool Serializer::object(const std::function<bool(dap::FieldSerializer*)>& cb) {
  struct FS : public FieldSerializer {
    nlohmann::json* const json;

    FS(nlohmann::json* json) : json(json) {}
    bool field(const std::string& name, const SerializeFunc& cb) override {
      Serializer s(&(*json)[name]);
      auto res = cb(&s);
      if (s.removed) {
        json->erase(name);
      }
      return res;
    }
  };

  *json = nlohmann::json({}, false, nlohmann::json::value_t::object);
  FS fs{json};
  return cb(&fs);
}

void Serializer::remove() {
  removed = true;
}

}  // namespace json
}  // namespace dap
