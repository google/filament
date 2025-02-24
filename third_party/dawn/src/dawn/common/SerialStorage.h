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

#ifndef SRC_DAWN_COMMON_SERIALSTORAGE_H_
#define SRC_DAWN_COMMON_SERIALSTORAGE_H_

#include <cstdint>
#include <utility>

#include "dawn/common/Assert.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {

template <typename T>
struct SerialStorageTraits {};

template <typename Derived>
class SerialStorage {
  protected:
    using Serial = typename SerialStorageTraits<Derived>::Serial;
    using Value = typename SerialStorageTraits<Derived>::Value;
    using Storage = typename SerialStorageTraits<Derived>::Storage;
    using StorageIterator = typename SerialStorageTraits<Derived>::StorageIterator;
    using ConstStorageIterator = typename SerialStorageTraits<Derived>::ConstStorageIterator;

  public:
    class Iterator {
      public:
        explicit Iterator(StorageIterator start);
        Iterator& operator++();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
        Value& operator*() const;

      private:
        StorageIterator mStorageIterator;
        // Special case the mSerialIterator when it should be equal to mStorageIterator.begin()
        // otherwise we could ask mStorageIterator.begin() when mStorageIterator is mStorage.end()
        // which is invalid. mStorageIterator.begin() is tagged with a nullptr.
        raw_ptr<Value, AllowPtrArithmetic> mSerialIterator;
    };

    class ConstIterator {
      public:
        explicit ConstIterator(ConstStorageIterator start);
        ConstIterator& operator++();

        bool operator==(const ConstIterator& other) const;
        bool operator!=(const ConstIterator& other) const;
        const Value& operator*() const;

      private:
        ConstStorageIterator mStorageIterator;
        raw_ptr<const Value, AllowPtrArithmetic> mSerialIterator;
    };

    class BeginEnd {
      public:
        BeginEnd(StorageIterator start, StorageIterator end);

        Iterator begin() const;
        Iterator end() const;

      private:
        StorageIterator mStartIt;
        StorageIterator mEndIt;
    };

    class ConstBeginEnd {
      public:
        ConstBeginEnd(ConstStorageIterator start, ConstStorageIterator end);

        ConstIterator begin() const;
        ConstIterator end() const;

      private:
        ConstStorageIterator mStartIt;
        ConstStorageIterator mEndIt;
    };

    // Derived classes may specialize constraits for elements stored
    // Ex.) SerialQueue enforces that the serial must be given in (not strictly)
    //      increasing order
    template <typename... Params>
    void Enqueue(Params&&... args, Serial serial) {
        Derived::Enqueue(std::forward<Params>(args)..., serial);
    }

    bool Empty() const;

    // The UpTo variants of Iterate and Clear affect all values associated to a serial
    // that is smaller OR EQUAL to the given serial. Iterating is done like so:
    //     for (const T& value : queue.IterateAll()) { stuff(T); }
    ConstBeginEnd IterateAll() const;
    ConstBeginEnd IterateUpTo(Serial serial) const;
    BeginEnd IterateAll();
    BeginEnd IterateUpTo(Serial serial);

    void Clear();
    void ClearUpTo(Serial serial);

    Serial FirstSerial() const;
    Serial LastSerial() const;

  protected:
    // Returns the first StorageIterator that a serial bigger than serial.
    ConstStorageIterator FindUpTo(Serial serial) const;
    StorageIterator FindUpTo(Serial serial);
    Storage mStorage;
};

// SerialStorage

template <typename Derived>
bool SerialStorage<Derived>::Empty() const {
    return mStorage.empty();
}

template <typename Derived>
typename SerialStorage<Derived>::ConstBeginEnd SerialStorage<Derived>::IterateAll() const {
    return {mStorage.begin(), mStorage.end()};
}

template <typename Derived>
typename SerialStorage<Derived>::ConstBeginEnd SerialStorage<Derived>::IterateUpTo(
    Serial serial) const {
    return {mStorage.begin(), FindUpTo(serial)};
}

template <typename Derived>
typename SerialStorage<Derived>::BeginEnd SerialStorage<Derived>::IterateAll() {
    return {mStorage.begin(), mStorage.end()};
}

template <typename Derived>
typename SerialStorage<Derived>::BeginEnd SerialStorage<Derived>::IterateUpTo(Serial serial) {
    return {mStorage.begin(), FindUpTo(serial)};
}

template <typename Derived>
void SerialStorage<Derived>::Clear() {
    mStorage.clear();
}

template <typename Derived>
void SerialStorage<Derived>::ClearUpTo(Serial serial) {
    mStorage.erase(mStorage.begin(), FindUpTo(serial));
}

template <typename Derived>
typename SerialStorage<Derived>::Serial SerialStorage<Derived>::FirstSerial() const {
    DAWN_ASSERT(!Empty());
    return mStorage.begin()->first;
}

template <typename Derived>
typename SerialStorage<Derived>::Serial SerialStorage<Derived>::LastSerial() const {
    DAWN_ASSERT(!Empty());
    return mStorage.back().first;
}

template <typename Derived>
typename SerialStorage<Derived>::ConstStorageIterator SerialStorage<Derived>::FindUpTo(
    Serial serial) const {
    auto it = mStorage.begin();
    while (it != mStorage.end() && it->first <= serial) {
        it++;
    }
    return it;
}

template <typename Derived>
typename SerialStorage<Derived>::StorageIterator SerialStorage<Derived>::FindUpTo(Serial serial) {
    auto it = mStorage.begin();
    while (it != mStorage.end() && it->first <= serial) {
        it++;
    }
    return it;
}

// SerialStorage::BeginEnd

template <typename Derived>
SerialStorage<Derived>::BeginEnd::BeginEnd(typename SerialStorage<Derived>::StorageIterator start,
                                           typename SerialStorage<Derived>::StorageIterator end)
    : mStartIt(start), mEndIt(end) {}

template <typename Derived>
typename SerialStorage<Derived>::Iterator SerialStorage<Derived>::BeginEnd::begin() const {
    return SerialStorage::Iterator(mStartIt);
}

template <typename Derived>
typename SerialStorage<Derived>::Iterator SerialStorage<Derived>::BeginEnd::end() const {
    return SerialStorage::Iterator(mEndIt);
}

// SerialStorage::Iterator

template <typename Derived>
SerialStorage<Derived>::Iterator::Iterator(typename SerialStorage<Derived>::StorageIterator start)
    : mStorageIterator(start), mSerialIterator(nullptr) {}

template <typename Derived>
typename SerialStorage<Derived>::Iterator& SerialStorage<Derived>::Iterator::operator++() {
    Value* vectorData = mStorageIterator->second.data();

    if (mSerialIterator == nullptr) {
        mSerialIterator = vectorData + 1;
    } else {
        mSerialIterator++;
    }

    if (mSerialIterator >= vectorData + mStorageIterator->second.size()) {
        mSerialIterator = nullptr;
        mStorageIterator++;
    }

    return *this;
}

template <typename Derived>
bool SerialStorage<Derived>::Iterator::operator==(
    const typename SerialStorage<Derived>::Iterator& other) const {
    return other.mStorageIterator == mStorageIterator && other.mSerialIterator == mSerialIterator;
}

template <typename Derived>
bool SerialStorage<Derived>::Iterator::operator!=(
    const typename SerialStorage<Derived>::Iterator& other) const {
    return !(*this == other);
}

template <typename Derived>
typename SerialStorage<Derived>::Value& SerialStorage<Derived>::Iterator::operator*() const {
    if (mSerialIterator == nullptr) {
        return *mStorageIterator->second.begin();
    }
    return *mSerialIterator;
}

// SerialStorage::ConstBeginEnd

template <typename Derived>
SerialStorage<Derived>::ConstBeginEnd::ConstBeginEnd(
    typename SerialStorage<Derived>::ConstStorageIterator start,
    typename SerialStorage<Derived>::ConstStorageIterator end)
    : mStartIt(start), mEndIt(end) {}

template <typename Derived>
typename SerialStorage<Derived>::ConstIterator SerialStorage<Derived>::ConstBeginEnd::begin()
    const {
    return SerialStorage::ConstIterator(mStartIt);
}

template <typename Derived>
typename SerialStorage<Derived>::ConstIterator SerialStorage<Derived>::ConstBeginEnd::end() const {
    return SerialStorage::ConstIterator(mEndIt);
}

// SerialStorage::ConstIterator

template <typename Derived>
SerialStorage<Derived>::ConstIterator::ConstIterator(
    typename SerialStorage<Derived>::ConstStorageIterator start)
    : mStorageIterator(start), mSerialIterator(nullptr) {}

template <typename Derived>
typename SerialStorage<Derived>::ConstIterator&
SerialStorage<Derived>::ConstIterator::operator++() {
    const Value* vectorData = mStorageIterator->second.data();

    if (mSerialIterator == nullptr) {
        mSerialIterator = vectorData + 1;
    } else {
        mSerialIterator++;
    }

    if (mSerialIterator >= vectorData + mStorageIterator->second.size()) {
        mSerialIterator = nullptr;
        mStorageIterator++;
    }

    return *this;
}

template <typename Derived>
bool SerialStorage<Derived>::ConstIterator::operator==(
    const typename SerialStorage<Derived>::ConstIterator& other) const {
    return other.mStorageIterator == mStorageIterator && other.mSerialIterator == mSerialIterator;
}

template <typename Derived>
bool SerialStorage<Derived>::ConstIterator::operator!=(
    const typename SerialStorage<Derived>::ConstIterator& other) const {
    return !(*this == other);
}

template <typename Derived>
const typename SerialStorage<Derived>::Value& SerialStorage<Derived>::ConstIterator::operator*()
    const {
    if (mSerialIterator == nullptr) {
        return *mStorageIterator->second.begin();
    }
    return *mSerialIterator;
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_SERIALSTORAGE_H_
