// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_SERIALMAP_H_
#define SRC_DAWN_COMMON_SERIALMAP_H_

#include <map>
#include <optional>
#include <utility>
#include <vector>

#include "dawn/common/SerialStorage.h"

namespace dawn {

template <typename Serial, typename Value>
class SerialMap;

template <typename SerialT, typename ValueT>
struct SerialStorageTraits<SerialMap<SerialT, ValueT>> {
    using Serial = SerialT;
    using Value = ValueT;
    using Storage = std::map<Serial, std::vector<Value>>;
    using StorageIterator = typename Storage::iterator;
    using ConstStorageIterator = typename Storage::const_iterator;
};

// SerialMap stores a map from Serial to Value.
// Unlike SerialQueue, items may be enqueued with Serials in any
// arbitrary order. SerialMap provides useful iterators for iterating
// through Value items in order of increasing Serial.
template <typename Serial, typename Value>
class SerialMap : public SerialStorage<SerialMap<Serial, Value>> {
  public:
    void Enqueue(const Value& value, Serial serial);
    void Enqueue(Value&& value, Serial serial);
    void Enqueue(const std::vector<Value>& values, Serial serial);
    void Enqueue(std::vector<Value>&& values, Serial serial);

    std::optional<Value> TakeOne(Serial serial);
};

// SerialMap

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(const Value& value, Serial serial) {
    this->mStorage[serial].emplace_back(value);
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(Value&& value, Serial serial) {
    this->mStorage[serial].emplace_back(std::move(value));
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(const std::vector<Value>& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    for (const Value& value : values) {
        Enqueue(value, serial);
    }
}

template <typename Serial, typename Value>
void SerialMap<Serial, Value>::Enqueue(std::vector<Value>&& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    for (Value& value : values) {
        Enqueue(std::move(value), serial);
    }
}

template <typename Serial, typename Value>
std::optional<Value> SerialMap<Serial, Value>::TakeOne(Serial serial) {
    auto it = this->mStorage.find(serial);
    if (it == this->mStorage.end()) {
        return std::nullopt;
    }
    auto& vec = it->second;
    if (vec.empty()) {
        return std::nullopt;
    }
    Value value = std::move(vec.back());
    vec.pop_back();
    if (vec.empty()) {
        this->mStorage.erase(it);
    }
    return value;
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_SERIALMAP_H_
