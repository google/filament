//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BinaryStream.h: Provides binary serialization of simple types.

#ifndef COMMON_BINARYSTREAM_H_
#define COMMON_BINARYSTREAM_H_

#include <stdint.h>
#include <cstddef>
#include <string>
#include <vector>

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "common/mathutil.h"

namespace gl
{
template <typename IntT>
struct PromotedIntegerType
{
    using type = typename std::conditional<
        std::is_signed<IntT>::value,
        typename std::conditional<sizeof(IntT) <= 4, int32_t, int64_t>::type,
        typename std::conditional<sizeof(IntT) <= 4, uint32_t, uint64_t>::type>::type;
};

class BinaryInputStream : angle::NonCopyable
{
  public:
    BinaryInputStream(const void *data, size_t length)
    {
        mError  = false;
        mOffset = 0;
        mData   = static_cast<const uint8_t *>(data);
        mLength = length;
    }

    // readInt will generate an error for bool types
    template <class IntT>
    IntT readInt()
    {
        static_assert(!std::is_same<bool, std::remove_cv<IntT>()>(), "Use readBool");
        using PromotedIntT = typename PromotedIntegerType<IntT>::type;
        PromotedIntT value = 0;
        read(&value);
        ASSERT(angle::IsValueInRangeForNumericType<IntT>(value));
        return static_cast<IntT>(value);
    }

    template <class IntT>
    void readInt(IntT *outValue)
    {
        *outValue = readInt<IntT>();
    }

    template <class T>
    void readVector(std::vector<T> *param)
    {
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        ASSERT(param->empty());
        size_t size = readInt<size_t>();
        if (size > 0)
        {
            param->resize(size);
            readBytes(reinterpret_cast<uint8_t *>(param->data()), param->size() * sizeof(T));
        }
    }

    template <typename E, typename T>
    void readPackedEnumMap(angle::PackedEnumMap<E, T> *param)
    {
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        readBytes(reinterpret_cast<uint8_t *>(param->data()), param->size() * sizeof(T));
    }

    template <class T>
    void readStruct(T *param)
    {
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        readBytes(reinterpret_cast<uint8_t *>(param), sizeof(T));
    }

    template <class EnumT>
    EnumT readEnum()
    {
        using UnderlyingType = typename std::underlying_type<EnumT>::type;
        return static_cast<EnumT>(readInt<UnderlyingType>());
    }

    template <class EnumT>
    void readEnum(EnumT *outValue)
    {
        *outValue = readEnum<EnumT>();
    }

    bool readBool()
    {
        int value = 0;
        read(&value);
        return (value > 0);
    }

    void readBool(bool *outValue) { *outValue = readBool(); }

    void readBytes(unsigned char outArray[], size_t count) { read<unsigned char>(outArray, count); }
    const unsigned char *getBytes(size_t count) { return read<unsigned char>(nullptr, count); }

    std::string readString()
    {
        std::string outString;
        readString(&outString);
        return outString;
    }

    void readString(std::string *v)
    {
        size_t length;
        readInt(&length);

        if (mError)
        {
            return;
        }

        angle::CheckedNumeric<size_t> checkedOffset(mOffset);
        checkedOffset += length;

        if (!checkedOffset.IsValid() || mOffset + length > mLength)
        {
            mError = true;
            return;
        }

        v->assign(reinterpret_cast<const char *>(mData) + mOffset, length);
        mOffset = checkedOffset.ValueOrDie();
    }

    float readFloat()
    {
        float f;
        read(&f, 1);
        return f;
    }

    void skip(size_t length)
    {
        angle::CheckedNumeric<size_t> checkedOffset(mOffset);
        checkedOffset += length;

        if (!checkedOffset.IsValid() || mOffset + length > mLength)
        {
            mError = true;
            return;
        }

        mOffset = checkedOffset.ValueOrDie();
    }

    size_t offset() const { return mOffset; }
    size_t remainingSize() const
    {
        ASSERT(mLength >= mOffset);
        return mLength - mOffset;
    }

    bool error() const { return mError; }

    bool endOfStream() const { return mOffset == mLength; }

    const uint8_t *data() { return mData; }

  private:
    bool mError;
    size_t mOffset;
    const uint8_t *mData;
    size_t mLength;

    template <typename T>
    const uint8_t *read(T *v, size_t num)
    {
        static_assert(std::is_fundamental<T>::value, "T must be a fundamental type.");

        angle::CheckedNumeric<size_t> checkedLength(num);
        checkedLength *= sizeof(T);
        if (!checkedLength.IsValid())
        {
            mError = true;
            return nullptr;
        }

        angle::CheckedNumeric<size_t> checkedOffset(mOffset);
        checkedOffset += checkedLength;

        if (!checkedOffset.IsValid() || checkedOffset.ValueOrDie() > mLength)
        {
            mError = true;
            return nullptr;
        }

        const uint8_t *srcBytes = mData + mOffset;
        if (v != nullptr)
        {
            memcpy(v, srcBytes, checkedLength.ValueOrDie());
        }
        mOffset = checkedOffset.ValueOrDie();

        return srcBytes;
    }

    template <typename T>
    void read(T *v)
    {
        read(v, 1);
    }
};

class BinaryOutputStream : angle::NonCopyable
{
  public:
    BinaryOutputStream();
    ~BinaryOutputStream();

    // writeInt also handles bool types
    template <class IntT>
    void writeInt(IntT param)
    {
        static_assert(std::is_integral<IntT>::value, "Not an integral type");
        static_assert(!std::is_same<bool, std::remove_cv<IntT>()>(), "Use writeBool");
        using PromotedIntT = typename PromotedIntegerType<IntT>::type;
        ASSERT(angle::IsValueInRangeForNumericType<PromotedIntT>(param));
        PromotedIntT intValue = static_cast<PromotedIntT>(param);
        write(&intValue, 1);
    }

    // Specialized writeInt for values that can also be exactly -1.
    template <class UintT>
    void writeIntOrNegOne(UintT param)
    {
        if (param == static_cast<UintT>(-1))
        {
            writeInt(-1);
        }
        else
        {
            writeInt(param);
        }
    }

    template <class T>
    void writeVector(const std::vector<T> &param)
    {
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        writeInt(param.size());
        if (param.size() > 0)
        {
            writeBytes(reinterpret_cast<const uint8_t *>(param.data()), param.size() * sizeof(T));
        }
    }

    template <typename E, typename T>
    void writePackedEnumMap(const angle::PackedEnumMap<E, T> &param)
    {
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        writeBytes(reinterpret_cast<const uint8_t *>(param.data()), param.size() * sizeof(T));
    }

    template <class T>
    void writeStruct(const T &param)
    {
        static_assert(!std::is_pointer<T>::value,
                      "Must pass in a struct, not the pointer to struct");
        static_assert(std::is_trivially_copyable<T>(), "must be memcpy-able");
        writeBytes(reinterpret_cast<const uint8_t *>(&param), sizeof(T));
    }

    template <class EnumT>
    void writeEnum(EnumT param)
    {
        using UnderlyingType = typename std::underlying_type<EnumT>::type;
        writeInt<UnderlyingType>(static_cast<UnderlyingType>(param));
    }

    void writeString(const std::string &v)
    {
        writeInt(v.length());
        write(v.c_str(), v.length());
    }

    void writeString(const char *v)
    {
        size_t len = strlen(v);
        writeInt(len);
        write(v, len);
    }

    void writeBytes(const unsigned char *bytes, size_t count) { write(bytes, count); }

    void writeBool(bool value)
    {
        int intValue = value ? 1 : 0;
        write(&intValue, 1);
    }

    void writeFloat(float value) { write(&value, 1); }

    size_t length() const { return mData.size(); }

    const void *data() const { return mData.size() ? &mData[0] : nullptr; }

    const std::vector<uint8_t> &getData() const { return mData; }

  private:
    template <typename T>
    void write(const T *v, size_t num)
    {
        static_assert(std::is_fundamental<T>::value, "T must be a fundamental type.");
        const char *asBytes = reinterpret_cast<const char *>(v);
        mData.insert(mData.end(), asBytes, asBytes + num * sizeof(T));
    }

    std::vector<uint8_t> mData;
};

inline BinaryOutputStream::BinaryOutputStream() {}

inline BinaryOutputStream::~BinaryOutputStream() = default;

}  // namespace gl

#endif  // COMMON_BINARYSTREAM_H_
