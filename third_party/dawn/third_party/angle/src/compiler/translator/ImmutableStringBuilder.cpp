//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImmutableStringBuilder.cpp: Stringstream-like utility for building pool allocated strings where
// the maximum length is known in advance.
//

#include "compiler/translator/ImmutableStringBuilder.h"

#include <inttypes.h>
#include <stdio.h>

namespace sh
{

ImmutableStringBuilder &ImmutableStringBuilder::operator<<(const ImmutableString &str)
{
    ASSERT(mData != nullptr);
    ASSERT(mPos + str.length() <= mMaxLength);
    memcpy(mData + mPos, str.data(), str.length());
    mPos += str.length();
    return *this;
}

ImmutableStringBuilder &ImmutableStringBuilder::operator<<(char c)
{
    ASSERT(mData != nullptr);
    ASSERT(mPos + 1 <= mMaxLength);
    mData[mPos++] = c;
    return *this;
}

ImmutableStringBuilder &ImmutableStringBuilder::operator<<(uint64_t v)
{
    // + 1 is because snprintf writes at most bufsz - 1 and then \0.
    // Our bufsz is mMaxLength + 1.
    int numChars = snprintf(mData + mPos, mMaxLength - mPos + 1, "%" PRIu64, v);
    ASSERT(numChars >= 0);
    ASSERT(mPos + numChars <= mMaxLength);
    mPos += numChars;
    return *this;
}

ImmutableStringBuilder &ImmutableStringBuilder::operator<<(int64_t v)
{
    // + 1 is because snprintf writes at most bufsz - 1 and then \0.
    // Our bufsz is mMaxLength + 1.
    int numChars = snprintf(mData + mPos, mMaxLength - mPos + 1, "%" PRId64, v);
    ASSERT(numChars >= 0);
    ASSERT(mPos + numChars <= mMaxLength);
    mPos += numChars;
    return *this;
}

ImmutableStringBuilder::operator ImmutableString()
{
    mData[mPos] = '\0';
    ImmutableString str(mData, mPos);
#if defined(ANGLE_ENABLE_ASSERTS)
    // Make sure that nothing is added to the string after it is finalized.
    mData = nullptr;
#endif
    return str;
}

}  // namespace sh
