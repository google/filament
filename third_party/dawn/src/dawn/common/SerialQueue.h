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

#ifndef SRC_DAWN_COMMON_SERIALQUEUE_H_
#define SRC_DAWN_COMMON_SERIALQUEUE_H_

#include <utility>
#include <vector>

#include "dawn/common/SerialStorage.h"

namespace dawn {

template <typename Serial, typename Value>
class SerialQueue;

template <typename SerialT, typename ValueT>
struct SerialStorageTraits<SerialQueue<SerialT, ValueT>> {
    using Serial = SerialT;
    using Value = ValueT;
    using SerialPair = std::pair<Serial, std::vector<Value>>;
    using Storage = std::vector<SerialPair>;
    using StorageIterator = typename Storage::iterator;
    using ConstStorageIterator = typename Storage::const_iterator;
};

// SerialQueue stores an associative list mapping a Serial to Value.
// It enforces that the Serials enqueued are strictly non-decreasing.
// This makes it very efficient iterate or clear all items added up
// to some Serial value because they are stored contiguously in memory.
template <typename Serial, typename Value>
class SerialQueue : public SerialStorage<SerialQueue<Serial, Value>> {
  public:
    // The serial must be given in (not strictly) increasing order.
    void Enqueue(const Value& value, Serial serial);
    void Enqueue(Value&& value, Serial serial);
    void Enqueue(const std::vector<Value>& values, Serial serial);
    void Enqueue(std::vector<Value>&& values, Serial serial);
};

// SerialQueue

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(const Value& value, Serial serial) {
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);

    if (this->Empty() || this->mStorage.back().first < serial) {
        this->mStorage.emplace_back(serial, std::vector<Value>{});
    }
    this->mStorage.back().second.push_back(value);
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(Value&& value, Serial serial) {
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);

    if (this->Empty() || this->mStorage.back().first < serial) {
        this->mStorage.emplace_back(serial, std::vector<Value>{});
    }
    this->mStorage.back().second.push_back(std::move(value));
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(const std::vector<Value>& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);
    this->mStorage.emplace_back(serial, values);
}

template <typename Serial, typename Value>
void SerialQueue<Serial, Value>::Enqueue(std::vector<Value>&& values, Serial serial) {
    DAWN_ASSERT(values.size() > 0);
    DAWN_ASSERT(this->Empty() || this->mStorage.back().first <= serial);
    this->mStorage.emplace_back(serial, std::move(values));
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_SERIALQUEUE_H_
