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

#include "dap/typeof.h"

#include <atomic>
#include <memory>
#include <vector>

namespace {

// TypeInfos owns all the dap::TypeInfo instances.
struct TypeInfos {
  // get() returns the TypeInfos singleton pointer.
  // TypeInfos is constructed with an internal reference count of 1.
  static TypeInfos* get();

  // reference() increments the TypeInfos reference count.
  inline void reference() {
    assert(refcount.load() > 0);
    refcount++;
  }

  // release() decrements the TypeInfos reference count.
  // If the reference count becomes 0, then the TypeInfos is destructed.
  inline void release() {
    if (--refcount == 0) {
      this->~TypeInfos();
    }
  }

  struct NullTI : public dap::TypeInfo {
    using null = dap::null;
    inline std::string name() const override { return "null"; }
    inline size_t size() const override { return sizeof(null); }
    inline size_t alignment() const override { return alignof(null); }
    inline void construct(void* ptr) const override { new (ptr) null(); }
    inline void copyConstruct(void* dst, const void* src) const override {
      new (dst) null(*reinterpret_cast<const null*>(src));
    }
    inline void destruct(void* ptr) const override {
      reinterpret_cast<null*>(ptr)->~null();
    }
    inline bool deserialize(const dap::Deserializer*, void*) const override {
      return true;
    }
    inline bool serialize(dap::Serializer*, const void*) const override {
      return true;
    }
  };

  dap::BasicTypeInfo<dap::boolean> boolean = {"boolean"};
  dap::BasicTypeInfo<dap::string> string = {"string"};
  dap::BasicTypeInfo<dap::integer> integer = {"integer"};
  dap::BasicTypeInfo<dap::number> number = {"number"};
  dap::BasicTypeInfo<dap::object> object = {"object"};
  dap::BasicTypeInfo<dap::any> any = {"any"};
  NullTI null;
  std::vector<std::unique_ptr<dap::TypeInfo>> types;

 private:
  TypeInfos() = default;
  ~TypeInfos() = default;
  std::atomic<uint64_t> refcount = {1};
};

// aligned_storage() is a replacement for std::aligned_storage that isn't busted
// on older versions of MSVC.
template <size_t SIZE, size_t ALIGNMENT>
struct aligned_storage {
  struct alignas(ALIGNMENT) type {
    unsigned char data[SIZE];
  };
};

TypeInfos* TypeInfos::get() {
  static aligned_storage<sizeof(TypeInfos), alignof(TypeInfos)>::type memory;

  struct Instance {
    TypeInfos* ptr() { return reinterpret_cast<TypeInfos*>(memory.data); }
    Instance() { new (ptr()) TypeInfos(); }
    ~Instance() { ptr()->release(); }
  };

  static Instance instance;
  return instance.ptr();
}

}  // namespace

namespace dap {

const TypeInfo* TypeOf<boolean>::type() {
  return &TypeInfos::get()->boolean;
}

const TypeInfo* TypeOf<string>::type() {
  return &TypeInfos::get()->string;
}

const TypeInfo* TypeOf<integer>::type() {
  return &TypeInfos::get()->integer;
}

const TypeInfo* TypeOf<number>::type() {
  return &TypeInfos::get()->number;
}

const TypeInfo* TypeOf<object>::type() {
  return &TypeInfos::get()->object;
}

const TypeInfo* TypeOf<any>::type() {
  return &TypeInfos::get()->any;
}

const TypeInfo* TypeOf<null>::type() {
  return &TypeInfos::get()->null;
}

void TypeInfo::deleteOnExit(TypeInfo* ti) {
  return TypeInfos::get()->types.emplace_back(std::unique_ptr<TypeInfo>(ti));
}

void initialize() {
  TypeInfos::get()->reference();
}

void terminate() {
  TypeInfos::get()->release();
}

}  // namespace dap
