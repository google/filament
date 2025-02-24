//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// serial_utils:
//   Utilities for generating unique IDs for resources in ANGLE.
//

#ifndef LIBANGLE_RENDERER_SERIAL_UTILS_H_
#define LIBANGLE_RENDERER_SERIAL_UTILS_H_

#include <array>
#include <atomic>
#include <limits>

#include "common/angleutils.h"
#include "common/debug.h"

namespace rx
{
class ResourceSerial
{
  public:
    constexpr ResourceSerial() : mValue(kDirty) {}
    explicit constexpr ResourceSerial(uintptr_t value) : mValue(value) {}
    constexpr bool operator==(ResourceSerial other) const { return mValue == other.mValue; }
    constexpr bool operator!=(ResourceSerial other) const { return mValue != other.mValue; }

    void dirty() { mValue = kDirty; }
    void clear() { mValue = kEmpty; }

    constexpr bool valid() const { return mValue != kEmpty && mValue != kDirty; }
    constexpr bool empty() const { return mValue == kEmpty; }

  private:
    constexpr static uintptr_t kDirty = std::numeric_limits<uintptr_t>::max();
    constexpr static uintptr_t kEmpty = 0;

    uintptr_t mValue;
};

// Class UniqueSerial defines unique serial number for object identification. It has only
// equal/unequal comparison but no greater/smaller comparison. The default constructor creates an
// invalid value.
class UniqueSerial final
{
  public:
    constexpr UniqueSerial() : mValue(kInvalid) {}
    constexpr UniqueSerial(const UniqueSerial &other)  = default;
    UniqueSerial &operator=(const UniqueSerial &other) = default;

    constexpr bool operator==(const UniqueSerial &other) const
    {
        return mValue != kInvalid && mValue == other.mValue;
    }
    constexpr bool operator!=(const UniqueSerial &other) const
    {
        return mValue == kInvalid || mValue != other.mValue;
    }

    // Useful for serialization.
    constexpr uint64_t getValue() const { return mValue; }
    constexpr bool valid() const { return mValue != kInvalid; }

  private:
    friend class UniqueSerialFactory;
    constexpr explicit UniqueSerial(uint64_t value) : mValue(value) {}
    uint64_t mValue;
    static constexpr uint64_t kInvalid = 0;
};

class UniqueSerialFactory final : angle::NonCopyable
{
  public:
    UniqueSerialFactory() : mSerial(1) {}

    UniqueSerial generate()
    {
        uint64_t current = mSerial++;
        ASSERT(mSerial > current);  // Integer overflow
        return UniqueSerial(current);
    }

  private:
    uint64_t mSerial;
};

// Class Serial defines a monotonically increasing serial number that indicates the timeline of
// execution.
class Serial final
{
  public:
    constexpr Serial() : mValue(0) {}
    constexpr Serial(const Serial &other)  = default;
    Serial &operator=(const Serial &other) = default;

    static constexpr Serial Infinite() { return Serial(std::numeric_limits<uint64_t>::max()); }

    constexpr bool operator==(const Serial &other) const { return mValue == other.mValue; }
    constexpr bool operator!=(const Serial &other) const { return mValue != other.mValue; }
    constexpr bool operator>(const Serial &other) const { return mValue > other.mValue; }
    constexpr bool operator>=(const Serial &other) const { return mValue >= other.mValue; }
    constexpr bool operator<(const Serial &other) const { return mValue < other.mValue; }
    constexpr bool operator<=(const Serial &other) const { return mValue <= other.mValue; }

    // Useful for serialization.
    constexpr uint64_t getValue() const { return mValue; }

  private:
    friend class AtomicSerialFactory;
    friend class RangedSerialFactory;
    friend class AtomicQueueSerial;
    constexpr explicit Serial(uint64_t value) : mValue(value) {}
    uint64_t mValue;
};

// Defines class to track the queue serial that can be load/store from multiple threads atomically.
class alignas(8) AtomicQueueSerial final
{
  public:
    AtomicQueueSerial &operator=(const Serial &other)
    {
        mValue.store(other.mValue, std::memory_order_release);
        return *this;
    }
    Serial getSerial() const { return Serial(mValue.load(std::memory_order_consume)); }

  private:
    static constexpr uint64_t kInvalid = 0;
    std::atomic<uint64_t> mValue       = kInvalid;
};

// Used as default/initial serial
static constexpr Serial kZeroSerial = Serial();

// The factory to generate a serial number within the range [mSerial, mSerial+mCount}
class RangedSerialFactory final : angle::NonCopyable
{
  public:
    RangedSerialFactory() : mSerial(0), mCount(0) {}

    void reset() { mCount = 0; }
    bool empty() const { return mCount == 0; }
    bool generate(Serial *serialOut)
    {
        if (mCount > 0)
        {
            uint64_t current = mSerial++;
            ASSERT(mSerial > current);  // Integer overflow
            *serialOut = Serial(current);
            mCount--;
            return true;
        }
        return false;
    }

  private:
    friend class AtomicSerialFactory;
    void initialize(uint64_t initialSerial, size_t count)
    {
        mSerial = initialSerial;
        mCount  = count;
    }
    uint64_t mSerial;
    size_t mCount;
};

class AtomicSerialFactory final : angle::NonCopyable
{
  public:
    AtomicSerialFactory() : mSerial(1) {}

    Serial generate()
    {
        uint64_t current = mSerial++;
        ASSERT(mSerial > current);  // Integer overflow
        return Serial(current);
    }

    void reserve(RangedSerialFactory *rangeFactory, size_t count)
    {
        uint64_t current = mSerial;
        mSerial += count;
        ASSERT(mSerial > current);  // Integer overflow
        rangeFactory->initialize(current, count);
    }

  private:
    std::atomic<uint64_t> mSerial;
};

// For backend that supports multiple queue serials, QueueSerial includes a Serial and an index.
using SerialIndex                                     = uint32_t;
static constexpr SerialIndex kInvalidQueueSerialIndex = SerialIndex(-1);

class QueueSerial;
// Because we release queue index when context becomes non-current, in order to use up all index
// count, you will need to have 256 threads each has a context current. This is not a reasonable
// usage case.
constexpr size_t kMaxQueueSerialIndexCount = 256;
// Fixed array of queue serials
class AtomicQueueSerialFixedArray final
{
  public:
    AtomicQueueSerialFixedArray()  = default;
    ~AtomicQueueSerialFixedArray() = default;

    void setQueueSerial(SerialIndex index, Serial serial);
    void setQueueSerial(const QueueSerial &queueSerial);
    void fill(Serial serial) { std::fill(mSerials.begin(), mSerials.end(), serial); }
    Serial operator[](SerialIndex index) const { return mSerials[index].getSerial(); }
    size_t size() const { return mSerials.size(); }

  private:
    std::array<AtomicQueueSerial, kMaxQueueSerialIndexCount> mSerials;
};
std::ostream &operator<<(std::ostream &os, const AtomicQueueSerialFixedArray &serials);

class QueueSerial final
{
  public:
    QueueSerial() : mIndex(kInvalidQueueSerialIndex) {}
    QueueSerial(SerialIndex index, Serial serial) : mIndex(index), mSerial(serial)
    {
        ASSERT(index != kInvalidQueueSerialIndex);
    }
    constexpr QueueSerial(const QueueSerial &other)  = default;
    QueueSerial &operator=(const QueueSerial &other) = default;

    constexpr bool operator==(const QueueSerial &other) const
    {
        return mIndex == other.mIndex && mSerial == other.mSerial;
    }
    constexpr bool operator!=(const QueueSerial &other) const
    {
        return mIndex != other.mIndex || mSerial != other.mSerial;
    }
    constexpr bool operator<(const QueueSerial &other) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        ASSERT(mIndex == other.mIndex);
        return mSerial < other.mSerial;
    }
    constexpr bool operator<=(const QueueSerial &other) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        ASSERT(mIndex == other.mIndex);
        return mSerial <= other.mSerial;
    }
    constexpr bool operator>(const QueueSerial &other) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        ASSERT(mIndex == other.mIndex);
        return mSerial > other.mSerial;
    }
    constexpr bool operator>=(const QueueSerial &other) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        ASSERT(mIndex == other.mIndex);
        return mSerial >= other.mSerial;
    }

    bool operator>(const AtomicQueueSerialFixedArray &serials) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        return mSerial > serials[mIndex];
    }
    bool operator<=(const AtomicQueueSerialFixedArray &serials) const
    {
        ASSERT(mIndex != kInvalidQueueSerialIndex);
        return mSerial <= serials[mIndex];
    }

    constexpr bool valid() const { return mIndex != kInvalidQueueSerialIndex; }

    SerialIndex getIndex() const { return mIndex; }
    Serial getSerial() const { return mSerial; }

  private:
    SerialIndex mIndex;
    Serial mSerial;
};
std::ostream &operator<<(std::ostream &os, const QueueSerial &queueSerial);

ANGLE_INLINE void AtomicQueueSerialFixedArray::setQueueSerial(SerialIndex index, Serial serial)
{
    ASSERT(index != kInvalidQueueSerialIndex);
    ASSERT(index < mSerials.size());
    // Serial can only increase
    ASSERT(serial > mSerials[index].getSerial());
    mSerials[index] = serial;
}

ANGLE_INLINE void AtomicQueueSerialFixedArray::setQueueSerial(const QueueSerial &queueSerial)
{
    setQueueSerial(queueSerial.getIndex(), queueSerial.getSerial());
}

ANGLE_INLINE std::ostream &operator<<(std::ostream &os, const AtomicQueueSerialFixedArray &serials)
{
    // Search for last non-zero index (or 0 if all zeros).
    SerialIndex lastIndex = serials.size() == 0 ? 0 : static_cast<SerialIndex>(serials.size() - 1);
    while (lastIndex > 0 && serials[lastIndex].getValue() == 0)
    {
        lastIndex--;
    }
    os << '{';
    for (SerialIndex i = 0; i < lastIndex; i++)
    {
        os << serials[i].getValue() << ',';
    }
    os << serials[lastIndex].getValue() << '}';
    return os;
}

ANGLE_INLINE std::ostream &operator<<(std::ostream &os, const QueueSerial &queueSerial)
{
    os << '{' << queueSerial.getIndex() << ':' << queueSerial.getSerial().getValue() << '}';
    return os;
}
}  // namespace rx

#endif  // LIBANGLE_RENDERER_SERIAL_UTILS_H_
