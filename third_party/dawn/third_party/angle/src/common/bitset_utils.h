//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// bitset_utils:
//   Bitset-related helper classes, such as a fast iterator to scan for set bits.
//

#ifndef COMMON_BITSETITERATOR_H_
#define COMMON_BITSETITERATOR_H_

#include <stdint.h>

#include <array>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"

namespace angle
{
// Given x, create 1 << x.
template <typename BitsT, typename ParamT>
constexpr BitsT Bit(ParamT x)
{
    // It's undefined behavior if the shift size is equal to or larger than the width of the type.
    ASSERT(static_cast<size_t>(x) < sizeof(BitsT) * 8);

    return (static_cast<BitsT>(1) << static_cast<size_t>(x));
}

// Given x, create (1 << x) - 1, i.e. a mask with x bits set.
template <typename BitsT, typename ParamT>
constexpr BitsT BitMask(ParamT x)
{
    if (static_cast<size_t>(x) == 0)
    {
        return 0;
    }
    return ((Bit<BitsT>(static_cast<ParamT>(static_cast<size_t>(x) - 1)) - 1) << 1) | 1;
}

template <size_t N, typename BitsT, typename ParamT = std::size_t>
class BitSetT final
{
  public:
    class Reference final
    {
      public:
        ~Reference() {}
        Reference &operator=(bool x)
        {
            mParent->set(mBit, x);
            return *this;
        }
        explicit operator bool() const { return mParent->test(mBit); }

      private:
        friend class BitSetT;

        Reference(BitSetT *parent, ParamT bit) : mParent(parent), mBit(bit) {}

        BitSetT *mParent;
        ParamT mBit;
    };

    class Iterator final
    {
      public:
        Iterator(const BitSetT &bits);
        Iterator &operator++();

        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        ParamT operator*() const;

        // These helper functions allow mutating an iterator in-flight.
        // They only operate on later bits to ensure we don't iterate the same bit twice.
        void resetLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBitsCopy.reset(index);
        }

        // bits could contain bit that earlier than mCurrentBit. Since mBitCopy can't have bits
        // earlier than mCurrentBit, the & operation will mask out earlier bits anyway.
        void resetLaterBits(const BitSetT &bits)
        {
            BitSetT maskedBits = ~Mask(mCurrentBit + 1);
            maskedBits &= bits;
            mBitsCopy &= ~maskedBits;
        }

        void setLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBitsCopy.set(index);
        }

        void setLaterBits(const BitSetT &bits)
        {
            ASSERT((BitSetT(bits) &= Mask(mCurrentBit + 1)).none());
            mBitsCopy |= bits;
        }

      private:
        std::size_t getNextBit();

        BitSetT mBitsCopy;
        std::size_t mCurrentBit;
    };

    using value_type = BitsT;
    using param_type = ParamT;

    constexpr BitSetT();
    constexpr explicit BitSetT(BitsT value);
    constexpr explicit BitSetT(std::initializer_list<ParamT> init);

    constexpr bool operator==(const BitSetT &other) const;
    constexpr bool operator!=(const BitSetT &other) const;

    constexpr bool operator[](ParamT pos) const;
    Reference operator[](ParamT pos) { return Reference(this, pos); }

    constexpr bool test(ParamT pos) const;

    constexpr bool all() const;
    constexpr bool any() const;
    constexpr bool none() const;
    constexpr std::size_t count() const;

    // Returns true iff there are unset bits prior
    // to the most significant bit set. For example:
    // 0b0000 - false
    // 0b0001 - false
    // 0b0011 - false
    // 0b0010 - true
    // 0b0101 - true
    constexpr bool hasGaps() const;

    constexpr static std::size_t size() { return N; }

    constexpr BitSetT &operator&=(const BitSetT &other);
    constexpr BitSetT &operator|=(const BitSetT &other);
    constexpr BitSetT &operator^=(const BitSetT &other);
    constexpr BitSetT operator~() const;

    constexpr BitSetT &operator&=(BitsT value);
    constexpr BitSetT &operator|=(BitsT value);
    constexpr BitSetT &operator^=(BitsT value);

    constexpr BitSetT operator<<(std::size_t pos) const;
    constexpr BitSetT &operator<<=(std::size_t pos);
    constexpr BitSetT operator>>(std::size_t pos) const;
    constexpr BitSetT &operator>>=(std::size_t pos);

    constexpr BitSetT &set();
    constexpr BitSetT &set(ParamT pos, bool value = true);

    constexpr BitSetT &reset();
    constexpr BitSetT &reset(ParamT pos);

    constexpr BitSetT &flip();
    constexpr BitSetT &flip(ParamT pos);

    constexpr unsigned long to_ulong() const { return static_cast<unsigned long>(mBits); }
    constexpr BitsT bits() const { return mBits; }

    Iterator begin() const { return Iterator(*this); }
    Iterator end() const { return Iterator(BitSetT()); }

    constexpr static BitSetT Zero() { return BitSetT(); }

    constexpr ParamT first() const;
    constexpr ParamT last() const;

    // Produces a mask of ones up to the "x"th bit.
    constexpr static BitSetT Mask(std::size_t x)
    {
        BitSetT result;
        result.mBits = BitMask<BitsT>(static_cast<ParamT>(x));
        return result;
    }

  private:
    BitsT mBits;
};

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT>::BitSetT() : mBits(0)
{
    static_assert(N > 0, "Bitset type cannot support zero bits.");
    static_assert(N <= sizeof(BitsT) * 8, "Bitset type cannot support a size this large.");
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT>::BitSetT(BitsT value) : mBits(value & Mask(N).bits())
{}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT>::BitSetT(std::initializer_list<ParamT> init) : mBits(0)
{
    for (ParamT element : init)
    {
        mBits |= Bit<BitsT>(element);
    }
    ASSERT(mBits == (mBits & Mask(N).bits()));
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::operator==(const BitSetT &other) const
{
    return mBits == other.mBits;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::operator!=(const BitSetT &other) const
{
    return mBits != other.mBits;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::operator[](ParamT pos) const
{
    return test(pos);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::test(ParamT pos) const
{
    return (mBits & Bit<BitsT>(pos)) != 0;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::all() const
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return mBits == Mask(N).bits();
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::any() const
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return (mBits != 0);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::none() const
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return (mBits == 0);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr std::size_t BitSetT<N, BitsT, ParamT>::count() const
{
    return gl::BitCount(mBits);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::hasGaps() const
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return (mBits != Mask(N).bits()) && ((mBits & (mBits + 1)) != 0);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator&=(const BitSetT &other)
{
    mBits &= other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator|=(const BitSetT &other)
{
    mBits |= other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator^=(const BitSetT &other)
{
    mBits = mBits ^ other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator~() const
{
    return BitSetT<N, BitsT, ParamT>(~mBits & Mask(N).bits());
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator&=(BitsT value)
{
    mBits &= value;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator|=(BitsT value)
{
    mBits |= value;
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator^=(BitsT value)
{
    mBits ^= value;
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator<<(std::size_t pos) const
{
    return BitSetT<N, BitsT, ParamT>((mBits << pos) & Mask(N).bits());
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator<<=(std::size_t pos)
{
    mBits = mBits << pos & Mask(N).bits();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator>>(std::size_t pos) const
{
    return BitSetT<N, BitsT, ParamT>(mBits >> pos);
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator>>=(std::size_t pos)
{
    mBits = (mBits >> pos) & Mask(N).bits();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::set()
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    mBits = Mask(N).bits();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::set(ParamT pos, bool value)
{
    ASSERT(static_cast<size_t>(pos) < N);
    if (value)
    {
        mBits |= Bit<BitsT>(pos);
    }
    else
    {
        reset(pos);
    }
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::reset()
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    mBits = 0;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::reset(ParamT pos)
{
    ASSERT(static_cast<size_t>(pos) < N);
    ASSERT(mBits == (mBits & Mask(N).bits()));
    mBits &= ~Bit<BitsT>(pos);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::flip()
{
    ASSERT(mBits == (mBits & Mask(N).bits()));
    mBits ^= Mask(N).bits();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::flip(ParamT pos)
{
    ASSERT(static_cast<size_t>(pos) < N);
    mBits ^= Bit<BitsT>(pos);
    ASSERT(mBits == (mBits & Mask(N).bits()));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr ParamT BitSetT<N, BitsT, ParamT>::first() const
{
    ASSERT(!none());
    return static_cast<ParamT>(gl::ScanForward(mBits));
}

template <size_t N, typename BitsT, typename ParamT>
constexpr ParamT BitSetT<N, BitsT, ParamT>::last() const
{
    ASSERT(!none());
    return static_cast<ParamT>(gl::ScanReverse(mBits));
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT>::Iterator::Iterator(const BitSetT &bits) : mBitsCopy(bits), mCurrentBit(0)
{
    if (bits.any())
    {
        mCurrentBit = getNextBit();
    }
}

template <size_t N, typename BitsT, typename ParamT>
ANGLE_INLINE typename BitSetT<N, BitsT, ParamT>::Iterator &
BitSetT<N, BitsT, ParamT>::Iterator::operator++()
{
    ASSERT(mBitsCopy.any());
    mBitsCopy.reset(static_cast<ParamT>(mCurrentBit));
    mCurrentBit = getNextBit();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::Iterator::operator==(const Iterator &other) const
{
    return mBitsCopy == other.mBitsCopy;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <size_t N, typename BitsT, typename ParamT>
ParamT BitSetT<N, BitsT, ParamT>::Iterator::operator*() const
{
    return static_cast<ParamT>(mCurrentBit);
}

template <size_t N, typename BitsT, typename ParamT>
std::size_t BitSetT<N, BitsT, ParamT>::Iterator::getNextBit()
{
    if (mBitsCopy.none())
    {
        return 0;
    }

    return gl::ScanForward(mBitsCopy.mBits);
}

template <size_t N>
using BitSet8 = BitSetT<N, uint8_t>;

template <size_t N>
using BitSet16 = BitSetT<N, uint16_t>;

template <size_t N>
using BitSet32 = BitSetT<N, uint32_t>;
static_assert(std::is_trivially_copyable<BitSet32<32>>(), "must be memcpy-able");

template <size_t N>
using BitSet64 = BitSetT<N, uint64_t>;

template <std::size_t N>
class BitSetArray;

namespace priv
{

template <size_t N, typename T>
using EnableIfBitsFit = typename std::enable_if<N <= sizeof(T) * 8>::type;

template <size_t N, typename Enable = void>
struct GetBitSet
{
    using Type = BitSetArray<N>;
};

// Prefer 64-bit bitsets on 64-bit CPUs. They seem faster than 32-bit.
#if defined(ANGLE_IS_64_BIT_CPU)
template <size_t N>
struct GetBitSet<N, EnableIfBitsFit<N, uint64_t>>
{
    using Type = BitSet64<N>;
};
constexpr std::size_t kDefaultBitSetSize = 64;
using BaseBitSetType                     = BitSet64<kDefaultBitSetSize>;
#else
template <size_t N>
struct GetBitSet<N, EnableIfBitsFit<N, uint32_t>>
{
    using Type = BitSet32<N>;
};
constexpr std::size_t kDefaultBitSetSize = 32;
using BaseBitSetType                     = BitSet32<kDefaultBitSetSize>;
#endif  // defined(ANGLE_IS_64_BIT_CPU)

}  // namespace priv

template <size_t N>
using BitSet = typename priv::GetBitSet<N>::Type;

template <std::size_t N>
class BitSetArray final
{
  public:
    using BaseBitSet = priv::BaseBitSetType;
    using value_type = BaseBitSet::value_type;
    using param_type = BaseBitSet::param_type;

    constexpr BitSetArray();
    constexpr explicit BitSetArray(uint64_t value);
    constexpr explicit BitSetArray(std::initializer_list<param_type> init);

    class Reference final
    {
      public:
        ~Reference() {}
        Reference &operator=(bool x)
        {
            mParent.set(mPosition, x);
            return *this;
        }
        explicit operator bool() const { return mParent.test(mPosition); }

      private:
        friend class BitSetArray;

        Reference(BitSetArray &parent, std::size_t pos) : mParent(parent), mPosition(pos) {}

        BitSetArray &mParent;
        std::size_t mPosition;
    };
    class Iterator final
    {
      public:
        Iterator(const BitSetArray<N> &bitSetArray, std::size_t index);
        Iterator &operator++();
        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        size_t operator*() const;

        // These helper functions allow mutating an iterator in-flight.
        // They only operate on later bits to ensure we don't iterate the same bit twice.
        void resetLaterBit(std::size_t pos)
        {
            ASSERT(pos > (mIndex * priv::kDefaultBitSetSize) + *mCurrentIterator);
            prepareCopy();
            mParentCopy.reset(pos);
            updateIteratorBit(pos, false);
        }

        void setLaterBit(std::size_t pos)
        {
            ASSERT(pos > (mIndex * priv::kDefaultBitSetSize) + *mCurrentIterator);
            prepareCopy();
            mParentCopy.set(pos);
            updateIteratorBit(pos, true);
        }

        void setLaterBits(const BitSetArray &bits)
        {
            prepareCopy();
            mParentCopy |= bits;
            updateIteratorBits(bits);
        }

      private:
        ANGLE_INLINE void prepareCopy()
        {
            ASSERT(mParent.mBaseBitSetArray[mIndex].end() ==
                   mParentCopy.mBaseBitSetArray[mIndex].end());
            if (mParentCopy.none())
            {
                mParentCopy    = mParent;
                mCurrentParent = &mParentCopy;
            }
        }

        ANGLE_INLINE void updateIteratorBit(std::size_t pos, bool setBit)
        {
            // Get the index and offset, update current interator if within range
            size_t index  = pos >> kShiftForDivision;
            size_t offset = pos & kDefaultBitSetSizeMinusOne;
            if (index == mIndex)
            {
                if (setBit)
                {
                    mCurrentIterator.setLaterBit(offset);
                }
                else
                {
                    mCurrentIterator.resetLaterBit(offset);
                }
            }
        }

        ANGLE_INLINE void updateIteratorBits(const BitSetArray &bits)
        {
            mCurrentIterator.setLaterBits(bits.mBaseBitSetArray[mIndex]);
        }

        // Problem -
        // We want to provide the fastest path possible for usecases that iterate though the bitset.
        //
        // Options -
        // 1) For non-mutating iterations the const ref <mParent> is set as mCurrentParent and only
        //    for usecases that need to mutate the bitset while iterating we perform a copy of
        //    <mParent> into <mParentCopy> and modify its bits accordingly.
        // 2) The alternate approach was to perform a copy all the time in the constructor
        //    irrespective of whether it was a mutating usecase or not.
        //
        // Experiment -
        // BitSetIteratorPerfTest was run on a Windows machine with Intel CPU and these were the
        // results -
        // 1) Copy only when necessary -
        //      RESULT BitSetIteratorPerf.wall_time:    run = 116.1067374961 ns
        //      RESULT BitSetIteratorPerf.trial_steps : run = 8416124 count
        //      RESULT BitSetIteratorPerf.total_steps : run = 16832251 count
        // 2) Copy always -
        //      RESULT BitSetIteratorPerf.wall_time:    run = 242.7446459439 ns
        //      RESULT BitSetIteratorPerf.trial_steps : run = 4171416 count
        //      RESULT BitSetIteratorPerf.total_steps : run = 8342834 count
        //
        // Resolution -
        // We settled on the copy only when necessary path.
        size_t mIndex;
        const BitSetArray &mParent;
        BitSetArray mParentCopy;
        const BitSetArray *mCurrentParent;
        typename BaseBitSet::Iterator mCurrentIterator;
    };

    constexpr static std::size_t size() { return N; }
    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, kArraySize); }
    constexpr unsigned long to_ulong() const
    {
        // TODO(anglebug.com/42264163): Handle serializing more than kDefaultBitSetSize
        for (std::size_t index = 1; index < kArraySize; index++)
        {
            ASSERT(mBaseBitSetArray[index].none());
        }
        return static_cast<unsigned long>(mBaseBitSetArray[0].to_ulong());
    }

    // Assignment operators
    constexpr BitSetArray &operator&=(const BitSetArray &other);
    constexpr BitSetArray &operator|=(const BitSetArray &other);
    constexpr BitSetArray &operator^=(const BitSetArray &other);

    // Bitwise operators
    constexpr BitSetArray<N> operator&(const angle::BitSetArray<N> &other) const;
    constexpr BitSetArray<N> operator|(const angle::BitSetArray<N> &other) const;
    constexpr BitSetArray<N> operator^(const angle::BitSetArray<N> &other) const;

    // Relational Operators
    constexpr bool operator==(const angle::BitSetArray<N> &other) const;
    constexpr bool operator!=(const angle::BitSetArray<N> &other) const;

    // Unary operators
    constexpr BitSetArray operator~() const;
    constexpr bool operator[](std::size_t pos) const;
    constexpr Reference operator[](std::size_t pos)
    {
        ASSERT(pos < size());
        return Reference(*this, pos);
    }

    // Setter, getters and other helper methods
    constexpr BitSetArray &set();
    constexpr BitSetArray &set(std::size_t pos, bool value = true);
    constexpr BitSetArray &reset();
    constexpr BitSetArray &reset(std::size_t pos);
    constexpr bool test(std::size_t pos) const;
    constexpr bool all() const;
    constexpr bool any() const;
    constexpr bool none() const;
    constexpr std::size_t count() const;
    constexpr bool intersects(const BitSetArray &other) const;
    constexpr BitSetArray<N> &flip();
    constexpr param_type first() const;
    constexpr param_type last() const;

    constexpr value_type bits(size_t index) const;

    // Produces a mask of ones up to the "x"th bit.
    constexpr static BitSetArray Mask(std::size_t x);

  private:
    static constexpr std::size_t kDefaultBitSetSizeMinusOne = priv::kDefaultBitSetSize - 1;
    static constexpr std::size_t kShiftForDivision =
        static_cast<std::size_t>(rx::Log2(static_cast<unsigned int>(priv::kDefaultBitSetSize)));
    static constexpr std::size_t kArraySize =
        ((N + kDefaultBitSetSizeMinusOne) >> kShiftForDivision);
    constexpr static std::size_t kLastElementCount = (N & kDefaultBitSetSizeMinusOne);
    constexpr static std::size_t kLastElementMask =
        priv::BaseBitSetType::Mask(kLastElementCount == 0 ? priv::kDefaultBitSetSize
                                                          : kLastElementCount)
            .bits();

    std::array<BaseBitSet, kArraySize> mBaseBitSetArray;
};
static_assert(std::is_trivially_copyable<BitSetArray<32>>(), "must be memcpy-able");

template <std::size_t N>
constexpr BitSetArray<N>::BitSetArray()
{
    static_assert(N > priv::kDefaultBitSetSize, "BitSetArray type can't support requested size.");
    reset();
}

template <std::size_t N>
constexpr BitSetArray<N>::BitSetArray(uint64_t value)
{
    reset();

    if (priv::kDefaultBitSetSize < 64)
    {
        size_t i = 0;
        for (; i < kArraySize - 1; ++i)
        {
            value_type elemValue =
                value & priv::BaseBitSetType::Mask(priv::kDefaultBitSetSize).bits();
            mBaseBitSetArray[i] = priv::BaseBitSetType(elemValue);
            value >>= priv::kDefaultBitSetSize;
        }
        value_type elemValue = value & kLastElementMask;
        mBaseBitSetArray[i]  = priv::BaseBitSetType(elemValue);
    }
    else
    {
        value_type elemValue = value & priv::BaseBitSetType::Mask(priv::kDefaultBitSetSize).bits();
        mBaseBitSetArray[0]  = priv::BaseBitSetType(elemValue);
    }
}

template <std::size_t N>
constexpr BitSetArray<N>::BitSetArray(std::initializer_list<param_type> init)
{
    reset();

    for (param_type element : init)
    {
        size_t index  = element >> kShiftForDivision;
        size_t offset = element & kDefaultBitSetSizeMinusOne;
        mBaseBitSetArray[index].set(offset, true);
    }
}

template <size_t N>
BitSetArray<N>::Iterator::Iterator(const BitSetArray<N> &bitSetArray, std::size_t index)
    : mIndex(index),
      mParent(bitSetArray),
      mCurrentParent(&mParent),
      mCurrentIterator(mParent.mBaseBitSetArray[0].begin())
{
    while (mIndex < mCurrentParent->kArraySize)
    {
        if (mCurrentParent->mBaseBitSetArray[mIndex].any())
        {
            break;
        }
        mIndex++;
    }

    if (mIndex < mCurrentParent->kArraySize)
    {
        mCurrentIterator = mCurrentParent->mBaseBitSetArray[mIndex].begin();
    }
    else
    {
        mCurrentIterator = mCurrentParent->mBaseBitSetArray[mCurrentParent->kArraySize - 1].end();
    }
}

template <std::size_t N>
typename BitSetArray<N>::Iterator &BitSetArray<N>::Iterator::operator++()
{
    ++mCurrentIterator;
    while (mCurrentIterator == mCurrentParent->mBaseBitSetArray[mIndex].end())
    {
        mIndex++;
        if (mIndex >= mCurrentParent->kArraySize)
        {
            break;
        }
        mCurrentIterator = mCurrentParent->mBaseBitSetArray[mIndex].begin();
    }
    return *this;
}

template <std::size_t N>
bool BitSetArray<N>::Iterator::operator==(const BitSetArray<N>::Iterator &other) const
{
    return mCurrentIterator == other.mCurrentIterator;
}

template <std::size_t N>
bool BitSetArray<N>::Iterator::operator!=(const BitSetArray<N>::Iterator &other) const
{
    return mCurrentIterator != other.mCurrentIterator;
}

template <std::size_t N>
std::size_t BitSetArray<N>::Iterator::operator*() const
{
    return (mIndex * priv::kDefaultBitSetSize) + *mCurrentIterator;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::operator&=(const BitSetArray<N> &other)
{
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        mBaseBitSetArray[index] &= other.mBaseBitSetArray[index];
    }
    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::operator|=(const BitSetArray<N> &other)
{
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        mBaseBitSetArray[index] |= other.mBaseBitSetArray[index];
    }
    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::operator^=(const BitSetArray<N> &other)
{
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        mBaseBitSetArray[index] ^= other.mBaseBitSetArray[index];
    }
    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> BitSetArray<N>::operator&(const angle::BitSetArray<N> &other) const
{
    angle::BitSetArray<N> result(other);
    result &= *this;
    return result;
}

template <std::size_t N>
constexpr BitSetArray<N> BitSetArray<N>::operator|(const angle::BitSetArray<N> &other) const
{
    angle::BitSetArray<N> result(other);
    result |= *this;
    return result;
}

template <std::size_t N>
constexpr BitSetArray<N> BitSetArray<N>::operator^(const angle::BitSetArray<N> &other) const
{
    angle::BitSetArray<N> result(other);
    result ^= *this;
    return result;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::operator==(const angle::BitSetArray<N> &other) const
{
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        if (mBaseBitSetArray[index] != other.mBaseBitSetArray[index])
        {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::operator!=(const angle::BitSetArray<N> &other) const
{
    return !(*this == other);
}

template <std::size_t N>
constexpr BitSetArray<N> BitSetArray<N>::operator~() const
{
    angle::BitSetArray<N> result;
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        result.mBaseBitSetArray[index] |= ~mBaseBitSetArray[index];
    }
    // The last element in result may need special handling
    result.mBaseBitSetArray[kArraySize - 1] &= kLastElementMask;

    return result;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::operator[](std::size_t pos) const
{
    ASSERT(pos < size());
    return test(pos);
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::set()
{
    for (BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        baseBitSet.set();
    }
    // The last element in mBaseBitSetArray may need special handling
    mBaseBitSetArray[kArraySize - 1] &= kLastElementMask;

    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::set(std::size_t pos, bool value)
{
    ASSERT(pos < size());
    // Get the index and offset, then set the bit
    size_t index  = pos >> kShiftForDivision;
    size_t offset = pos & kDefaultBitSetSizeMinusOne;
    mBaseBitSetArray[index].set(offset, value);
    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::reset()
{
    for (BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        baseBitSet.reset();
    }
    return *this;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::reset(std::size_t pos)
{
    ASSERT(pos < size());
    return set(pos, false);
}

template <std::size_t N>
constexpr bool BitSetArray<N>::test(std::size_t pos) const
{
    ASSERT(pos < size());
    // Get the index and offset, then test the bit
    size_t index  = pos >> kShiftForDivision;
    size_t offset = pos & kDefaultBitSetSizeMinusOne;
    return mBaseBitSetArray[index].test(offset);
}

template <std::size_t N>
constexpr bool BitSetArray<N>::all() const
{
    constexpr priv::BaseBitSetType kLastElementBitSet = priv::BaseBitSetType(kLastElementMask);

    for (std::size_t index = 0; index < kArraySize - 1; index++)
    {
        if (!mBaseBitSetArray[index].all())
        {
            return false;
        }
    }

    // The last element in mBaseBitSetArray may need special handling
    return mBaseBitSetArray[kArraySize - 1] == kLastElementBitSet;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::any() const
{
    for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        if (baseBitSet.any())
        {
            return true;
        }
    }
    return false;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::none() const
{
    for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        if (!baseBitSet.none())
        {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
constexpr std::size_t BitSetArray<N>::count() const
{
    size_t count = 0;
    for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        count += baseBitSet.count();
    }
    return count;
}

template <std::size_t N>
constexpr bool BitSetArray<N>::intersects(const BitSetArray<N> &other) const
{
    for (std::size_t index = 0; index < kArraySize; index++)
    {
        if ((mBaseBitSetArray[index].bits() & other.mBaseBitSetArray[index].bits()) != 0)
        {
            return true;
        }
    }
    return false;
}

template <std::size_t N>
constexpr BitSetArray<N> &BitSetArray<N>::flip()
{
    for (BaseBitSet &baseBitSet : mBaseBitSetArray)
    {
        baseBitSet.flip();
    }

    // The last element in mBaseBitSetArray may need special handling
    mBaseBitSetArray[kArraySize - 1] &= kLastElementMask;
    return *this;
}

template <std::size_t N>
constexpr typename BitSetArray<N>::param_type BitSetArray<N>::first() const
{
    ASSERT(any());
    for (size_t arrayIndex = 0; arrayIndex < kArraySize; ++arrayIndex)
    {
        const BaseBitSet &baseBitSet = mBaseBitSetArray[arrayIndex];
        if (baseBitSet.any())
        {
            return baseBitSet.first() + arrayIndex * priv::kDefaultBitSetSize;
        }
    }
    UNREACHABLE();
    return 0;
}

template <std::size_t N>
constexpr typename BitSetArray<N>::param_type BitSetArray<N>::last() const
{
    ASSERT(any());
    for (size_t arrayIndex = kArraySize; arrayIndex > 0; --arrayIndex)
    {
        const BaseBitSet &baseBitSet = mBaseBitSetArray[arrayIndex - 1];
        if (baseBitSet.any())
        {
            return baseBitSet.last() + (arrayIndex - 1) * priv::kDefaultBitSetSize;
        }
    }
    UNREACHABLE();
    return 0;
}

template <std::size_t N>
constexpr typename BitSetArray<N>::value_type BitSetArray<N>::bits(size_t index) const
{
    return mBaseBitSetArray[index].bits();
}

template <std::size_t N>
constexpr BitSetArray<N> BitSetArray<N>::Mask(std::size_t x)
{
    BitSetArray result;

    for (size_t arrayIndex = 0; arrayIndex < kArraySize; ++arrayIndex)
    {
        const size_t bitOffset = arrayIndex * priv::kDefaultBitSetSize;
        if (x <= bitOffset)
        {
            break;
        }
        const size_t bitsInThisIndex        = std::min(x - bitOffset, priv::kDefaultBitSetSize);
        result.mBaseBitSetArray[arrayIndex] = BaseBitSet::Mask(bitsInThisIndex);
    }

    return result;
}
}  // namespace angle

template <size_t N, typename BitsT, typename ParamT>
inline constexpr angle::BitSetT<N, BitsT, ParamT> operator&(
    const angle::BitSetT<N, BitsT, ParamT> &lhs,
    const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result &= rhs.bits();
    return result;
}

template <size_t N, typename BitsT, typename ParamT>
inline constexpr angle::BitSetT<N, BitsT, ParamT> operator|(
    const angle::BitSetT<N, BitsT, ParamT> &lhs,
    const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result |= rhs.bits();
    return result;
}

template <size_t N, typename BitsT, typename ParamT>
inline constexpr angle::BitSetT<N, BitsT, ParamT> operator^(
    const angle::BitSetT<N, BitsT, ParamT> &lhs,
    const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result ^= rhs.bits();
    return result;
}

template <size_t N, typename BitsT, typename ParamT>
inline bool operator==(angle::BitSetT<N, BitsT, ParamT> &lhs, angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    return lhs.bits() == rhs.bits();
}

template <size_t N, typename BitsT, typename ParamT>
inline bool operator!=(angle::BitSetT<N, BitsT, ParamT> &lhs, angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    return !(lhs == rhs);
}

#endif  // COMMON_BITSETITERATOR_H_
