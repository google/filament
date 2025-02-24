/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <type_traits>
#include <vector>
#include "containers/custom_containers.h"

// Hash and equality utilities for supporting hashing containers (e.g. unordered_set, unordered_map)
namespace hash_util {

// True iff both pointers are null or both are non-null
template <typename T>
bool SimilarForNullity(const T *const lhs, const T *const rhs) {
    return ((lhs != nullptr) && (rhs != nullptr)) || ((lhs == nullptr) && (rhs == nullptr));
}

// Wrap std hash to avoid manual casts for the holes in std::hash (in C++11)
template <typename Value>
size_t HashWithUnderlying(Value value, typename std::enable_if<!std::is_enum<Value>::value, void *>::type = nullptr) {
    return vvl::hash<Value>()(value);
}

template <typename Value>
size_t HashWithUnderlying(Value value, typename std::enable_if<std::is_enum<Value>::value, void *>::type = nullptr) {
    using Underlying = typename std::underlying_type<Value>::type;
    return vvl::hash<Underlying>()(static_cast<const Underlying &>(value));
}

class HashCombiner {
  public:
    using Key = size_t;

    template <typename Value>
    struct WrappedHash {
        size_t operator()(const Value &value) const { return HashWithUnderlying(value); }
    };

    HashCombiner(Key combined = 0) : combined_(combined) {}

    // If you need to override the default hash
    template <typename Value, typename Hasher = WrappedHash<Value>>
    HashCombiner &Combine(const Value &value) {
        // magic and combination algorithm based on boost::hash_combine
        // http://www.boost.org/doc/libs/1_43_0/doc/html/hash/reference.html#boost.hash_combine
        // Magic value is 2^size / ((1-sqrt(5)/2)
        constexpr Key kMagic = sizeof(Key) > 4 ? static_cast<Key>(0x9e3779b97f4a7c16UL) : static_cast<Key>(0x9e3779b9U);

        combined_ ^= Hasher()(value) + kMagic + (combined_ << 6) + (combined_ >> 2);
        return *this;
    }

    template <typename Iterator, typename Hasher = WrappedHash<typename std::iterator_traits<Iterator>::value_type>>
    HashCombiner &Combine(Iterator first, Iterator end) {
        using Value = typename std::iterator_traits<Iterator>::value_type;
        auto current = first;
        for (; current != end; ++current) {
            Combine<Value, Hasher>(*current);
        }
        return *this;
    }

    template <typename Value, typename Hasher = WrappedHash<Value>>
    HashCombiner &Combine(const std::vector<Value> &vector) {
        return Combine(vector.cbegin(), vector.cend());
    }

    template <typename Value>
    HashCombiner &operator<<(const Value &value) {
        return Combine(value);
    }

    Key Value() const { return combined_; }
    void Reset(Key combined = 0) { combined_ = combined; }

  private:
    Key combined_;
};

// A template to inherit std::hash overloads from when T::hash() is defined
template <typename T>
struct HasHashMember {
    size_t operator()(const T &value) const { return value.hash(); }
};

// A template to inherit std::hash overloads from when is an *ordered* constainer
template <typename T>
struct IsOrderedContainer {
    size_t operator()(const T &value) const { return HashCombiner().Combine(value.cbegin(), value.cend()).Value(); }
};

// The dictionary provides a way of referencing canonical/reference
// data by id, such that the id's are invariant with dictionary
// resize/insert and that no entries point to identical data.  This
// approach uses the address of the unique data and as the unique
//  ID  for a give value of T.
//
// Note: This ID is unique for a given application execution, neither
//       globally unique, invariant, nor repeatable from execution to
//       execution.
//
// The entries of the dictionary are shared_pointers (the contents of
// which are invariant with resize/insert), with the hash and equality
// template arguments wrapped in a shared pointer dereferencing
// function object
template <typename T, typename Hasher = vvl::hash<T>, typename KeyEqual = std::equal_to<T>>
class Dictionary {
  public:
    using Def = T;
    using Id = std::shared_ptr<const Def>;

    // Find the unique entry match the provided value, adding if needed
    // TODO: segregate lookup from insert, using reader/write locks to reduce contention -- if needed
    template <typename U = T>
    Id LookUp(U &&value) {
        // We create an Id from the value, which will either be retained by dict (if new) or deleted on return (if extant)
        Id from_input = std::make_shared<T>(std::forward<U>(value));

        // Insert takes care of the "unique" id part by rejecting the insert if a key matching by_value exists, but returning us
        // the Id of the extant shared_pointer(id->def) instead.
        // return the value of the Iterator from the <Iterator, bool> pair returned by insert
        Guard g(lock);  // Dict isn't thread safe, and use is presumed to be multi-threaded
        return *dict.insert(from_input).first;
    }

  private:
    struct HashKeyValue {
        size_t operator()(const Id &value) const { return Hasher()(*value); }
    };
    struct KeyValueEqual {
        bool operator()(const Id &lhs, const Id &rhs) const { return KeyEqual()(*lhs, *rhs); }
    };
    using Dict = vvl::unordered_set<Id, HashKeyValue, KeyValueEqual>;
    using Lock = std::mutex;
    using Guard = std::lock_guard<Lock>;
    Lock lock;
    Dict dict;
};

uint32_t VuidHash(std::string_view vuid);

uint32_t ShaderHash(const void *pCode, const size_t codeSize);

uint64_t DescriptorVariableHash(const void *info, const size_t info_size);

}  // namespace hash_util
