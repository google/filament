/* Copyright (c) 2021-2024 The Khronos Group Inc.
 * Copyright (c) 2021-2024 Valve Corporation
 * Copyright (c) 2021-2024 LunarG, Inc.
 * Copyright (C) 2021-2022 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cstdint>
#include <string>

#include "generated/error_location_helper.h"
#include "logging.h"
#include "containers/custom_containers.h"
#include "chassis/chassis_handle_data.h"
#include "utils/hash_util.h"

// Holds the 'Location' of where the code is inside a function/struct/etc
// see docs/error_object.md for more details
struct Location {
    static const uint32_t kNoIndex = vvl::kU32Max;

    // name of the vulkan function we're checking
    const vvl::Func function{};

    const vvl::Struct structure{};
    const vvl::Field field{};
    const uint32_t index{};  // optional index if checking an array.
    const bool isPNext{};    // will print the struct is from a 'pNext` chain
    const Location* prev{};
    mutable const std::string* debug_region{};

    Location(vvl::Func func, vvl::Struct s, vvl::Field f = vvl::Field::Empty, uint32_t i = kNoIndex)
        : function(func), structure(s), field(f), index(i), isPNext(false), prev(nullptr) {}
    Location(vvl::Func func, vvl::Field f = vvl::Field::Empty, uint32_t i = kNoIndex)
        : function(func), structure(vvl::Struct::Empty), field(f), index(i), isPNext(false), prev(nullptr) {}
    Location(const Location& prev_loc, vvl::Struct s, vvl::Field f, uint32_t i, bool p)
        : function(prev_loc.function), structure(s), field(f), index(i), isPNext(p), prev(&prev_loc) {}
    Location(const Location& loc, std::string& debug_region)
        : function(loc.function),
          structure(loc.structure),
          field(loc.field),
          index(loc.index),
          isPNext(loc.isPNext),
          prev(loc.prev),
          debug_region(&debug_region) {}

    void AppendFields(std::ostream &out) const;
    std::string Fields() const;
    std::string Message() const;

    // the dot() method is for walking down into a structure that is being validated
    // eg:  loc.dot(Field::pMemoryBarriers, 5).dot(Field::srcStagemask)
    Location dot(vvl::Struct s, vvl::Field sub_field, uint32_t sub_index = kNoIndex) const {
        Location result(*this, s, sub_field, sub_index, false);
        return result;
    }
    Location dot(vvl::Field sub_field, uint32_t sub_index = kNoIndex) const {
        Location result(*this, this->structure, sub_field, sub_index, false);
        return result;
    }
    Location dot(uint32_t sub_index) const {
        Location result(*this, this->structure, this->field, sub_index, false);
        return result;
    }

    // same as dot() but will mark these were part of a pNext struct
    Location pNext(vvl::Struct s, vvl::Field sub_field = vvl::Field::Empty, uint32_t sub_index = kNoIndex) const {
        Location result(*this, s, sub_field, sub_index, true);
        return result;
    }

    const char* StringFunc() const { return vvl::String(function); }
    const char* StringStruct() const { return vvl::String(structure); }
    const char* StringField() const { return vvl::String(field); }
};

// Contains the base information needed for errors to be logged out
// Created for each function as a starting point to build off of
struct ErrorObject {
    const Location location;   // starting location (Always the function entrypoint)
    const VulkanTypedHandle handle;  // dispatchable handle is always first parameter of the function call
    const LogObjectList objlist;
    const chassis::HandleData* handle_data;

    ErrorObject(vvl::Func command_, VulkanTypedHandle handle_)
        : location(Location(command_)), handle(handle_), objlist(handle), handle_data(nullptr) {}
    ErrorObject(vvl::Func command_, VulkanTypedHandle handle_, const chassis::HandleData* handle_data_)
        : location(Location(command_)), handle(handle_), objlist(handle), handle_data(handle_data_) {}
};

namespace vvl {

template <typename VuidFunctor>
struct LocationVuidAdapter {
    const Location loc;
    VuidFunctor vuid_functor;
    const char* FuncName() const {
        // the returned reference from loc must be valid for lifespan of loc, at least.
        return loc.StringFunc();
    }
    const char* Vuid() const {
        // the returned reference from functor must be valid for lifespan of vuid_functor, at least.
        const std::string& vuid = vuid_functor(loc);
        return vuid.c_str();
    }
    template <typename... Args>
    LocationVuidAdapter(const Location& loc_, const Args&... args) : loc(loc_), vuid_functor(args...) {}
};

struct LocationCapture {
    LocationCapture(const Location& loc);
    LocationCapture(const LocationCapture &other);
    LocationCapture(LocationCapture &&other);

    const Location& Get() const { return capture.back(); }

  protected:
    // TODO: Optimize this for "new" minimization
    using CaptureStore = small_vector<Location, 2>;
    const Location* Capture(const Location& loc, CaptureStore::size_type depth);
    CaptureStore capture;
};

// Key for use in tables of VUIDs.
//
// Fuzzy match rules:
//  key.function OR key.structure may be Empty
//  loc.structure may be Empty
//  key.field may be Empty
//  if key.recurse_field is true, key.field can match loc.field or any fields in loc.prev
//
struct Key {
    // If a new member is added, update operator<
    Func function;
    Struct structure;
    Field field;
    bool recurse_field;
    Key(Struct r, Field f = Field::Empty, bool recurse = false)
        : function(Func::Empty), structure(r), field(f), recurse_field(recurse) {}
    Key(Func fn, Field f = Field::Empty, bool recurse = false)
        : function(fn), structure(Struct::Empty), field(f), recurse_field(recurse) {}
    Key(Func fn, Struct r, Field f = Field::Empty, bool recurse = false)
        : function(fn), structure(r), field(f), recurse_field(recurse) {}

    struct hash {
      public:
        std::size_t operator()(const Key& key) const {
            hash_util::HashCombiner hc;
            hc << static_cast<uint32_t>(key.function) << static_cast<uint32_t>(key.structure) << static_cast<uint32_t>(key.field)
               << key.recurse_field;
            return hc.Value();
        }
    };
};

bool operator<(const Key& lhs, const Key& rhs);
bool operator==(const Key& lhs, const Key& rhs);
bool operator==(const Key& key, const Location& loc);

// Entry in a VUID lookup table
struct Entry {
    Key k;
    std::string v;
};

// look for a matching VUID in a vector or array-ish table
template <typename Table>
static const std::string& FindVUID(const Location& loc, const Table& table) {
    // For anything function promoted, we will get the alias here
    // While annoying to have to make copy, better then making Location non-const
    const Location core_loc(FindAlias(loc.function), loc.structure, loc.field, loc.index);

    static const std::string empty;
    auto predicate = [&core_loc](const Entry& entry) { return entry.k == core_loc; };

    // consistency check: there should never be more than 1 match in a table
    assert(std::count_if(table.begin(), table.end(), predicate) <= 1);

    const auto pos = std::find_if(table.begin(), table.end(), predicate);
    return (pos != table.end()) ? pos->v : empty;
}

// 2-level look up where the outer container is a map where we need to find
// different VUIDs for different values of an enum or bitfield
template <typename OuterKey, typename Table>
static const std::string& FindVUID(OuterKey key, const Location& loc, const Table& table) {
    // For anything function promoted, we will get the alias here
    // While annoying to have to make copy, better then making Location non-const
    const Location core_loc(FindAlias(loc.function), loc.structure, loc.field, loc.index);

    static const std::string empty;
    const auto entry = table.find(key);
    if (entry != table.end()) {
        return FindVUID(core_loc, entry->second);
    }
    return empty;
}

}  // namespace vvl
