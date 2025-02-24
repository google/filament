//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLBitField.h: A bit field class that encapsulates the cl_bitfield type.

#ifndef LIBANGLE_CLBITFIELD_H_
#define LIBANGLE_CLBITFIELD_H_

#include <angle_cl.h>

namespace cl
{

class BitField
{
  public:
    BitField() noexcept : mBits(0u) {}
    explicit BitField(cl_bitfield bits) noexcept : mBits(bits) {}

    BitField &operator=(cl_bitfield bits)
    {
        mBits = bits;
        return *this;
    }

    bool operator==(cl_bitfield bits) const { return mBits == bits; }
    bool operator!=(cl_bitfield bits) const { return mBits != bits; }
    bool operator==(const BitField &other) const { return mBits == other.mBits; }
    bool operator!=(const BitField &other) const { return mBits != other.mBits; }

    cl_bitfield get() const { return mBits; }

    bool intersects(cl_bitfield bits) const { return (mBits & bits) != 0u; }
    bool intersects(const BitField &other) const { return (mBits & other.mBits) != 0u; }
    bool excludes(cl_bitfield bits) const { return !intersects(bits); }
    bool excludes(const BitField &other) const { return !intersects(mBits); }

    bool hasOtherBitsThan(cl_bitfield bits) const { return (mBits & ~bits) != 0u; }
    bool hasOtherBitsThan(const BitField &other) const { return (mBits & ~other.mBits) != 0u; }

    bool areMutuallyExclusive(cl_bitfield bits1, cl_bitfield bits2) const
    {
        return (intersects(bits1) ? 1 : 0) + (intersects(bits2) ? 1 : 0) <= 1;
    }

    bool areMutuallyExclusive(cl_bitfield bits1, cl_bitfield bits2, cl_bitfield bits3) const
    {
        return (intersects(bits1) ? 1 : 0) + (intersects(bits2) ? 1 : 0) +
                   (intersects(bits3) ? 1 : 0) <=
               1;
    }

    BitField mask(cl_bitfield bits) const { return BitField(mBits & bits); }
    BitField mask(const BitField &other) const { return BitField(mBits & other.mBits); }

    void set(cl_bitfield bits) { mBits |= bits; }
    void set(const BitField &other) { mBits |= other.mBits; }
    void clear(cl_bitfield bits) { mBits &= ~bits; }
    void clear(const BitField &other) { mBits &= ~other.mBits; }

  private:
    cl_bitfield mBits;
};

static_assert(sizeof(BitField) == sizeof(cl_bitfield), "Type size mismatch");

using DeviceType                = BitField;
using DeviceFpConfig            = BitField;
using DeviceExecCapabilities    = BitField;
using DeviceSvmCapabilities     = BitField;
using CommandQueueProperties    = BitField;
using DeviceAffinityDomain      = BitField;
using MemFlags                  = BitField;
using SVM_MemFlags              = BitField;
using MemMigrationFlags         = BitField;
using MapFlags                  = BitField;
using KernelArgTypeQualifier    = BitField;
using DeviceAtomicCapabilities  = BitField;
using DeviceEnqueueCapabilities = BitField;

}  // namespace cl

#endif  // LIBANGLE_CLBITFIELD_H_
