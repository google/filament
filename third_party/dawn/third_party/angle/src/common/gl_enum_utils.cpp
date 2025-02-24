//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl_enum_utils.cpp:
//   Utility functions for converting GLenums to string.

#include "common/gl_enum_utils.h"

#include "common/bitset_utils.h"
#include "common/string_utils.h"

#include <iomanip>
#include <sstream>

namespace gl
{
namespace
{

template <typename EnumType>
void OutputGLenumStringImpl(std::ostream &out, EnumType enumGroup, unsigned int value)
{
    const char *enumStr = GLenumToString(enumGroup, value);
    if (enumStr != kUnknownGLenumString)
    {
        out << enumStr;
        return;
    }

    if (enumGroup == EnumType::Boolean)
    {
        // If an unknown enum was submitted as GLboolean, just write out the value.
        if (enumStr == kUnknownGLenumString)
        {
            out << value;
        }
        else
        {
            out << enumStr;
        }

        return;
    }

    if (enumGroup != EnumType::AllEnums)
    {
        // Retry with the "Default" group
        enumStr = GLenumToString(EnumType::AllEnums, value);
        if (enumStr != kUnknownGLenumString)
        {
            out << enumStr;
            return;
        }
    }

    out << std::hex << "0x" << std::setfill('0') << std::setw(4) << value << std::dec;
}

template <typename EnumType>
std::string GLbitfieldToStringImpl(EnumType enumGroup, unsigned int value)
{
    std::stringstream st;

    if (value == 0)
    {
        return "0";
    }

    const angle::BitSet<32> bitSet(value);
    bool first = true;
    for (const auto index : bitSet)
    {
        if (!first)
        {
            st << " | ";
        }
        first = false;

        unsigned int mask = 1u << index;
        OutputGLenumString(st, enumGroup, mask);
    }

    return st.str();
}
}  // namespace

const char kUnknownGLenumString[] = "EnumUnknown";

void OutputGLenumString(std::ostream &out, GLESEnum enumGroup, unsigned int value)
{
    return OutputGLenumStringImpl(out, enumGroup, value);
}

void OutputGLenumString(std::ostream &out, BigGLEnum enumGroup, unsigned int value)
{
    return OutputGLenumStringImpl(out, enumGroup, value);
}

void OutputGLbitfieldString(std::ostream &out, GLESEnum enumGroup, unsigned int value)
{
    out << GLbitfieldToString(enumGroup, value);
}

const char *GLbooleanToString(unsigned int value)
{
    return GLenumToString(GLESEnum::Boolean, value);
}

std::string GLbitfieldToString(GLESEnum enumGroup, unsigned int value)
{
    return GLbitfieldToStringImpl(enumGroup, value);
}

std::string GLbitfieldToString(BigGLEnum enumGroup, unsigned int value)
{
    return GLbitfieldToStringImpl(enumGroup, value);
}

const char *GLinternalFormatToString(unsigned int format)
{
    return GLenumToString(gl::GLESEnum::InternalFormat, format);
}

unsigned int StringToGLbitfield(const char *str)
{
    unsigned int value = 0;
    std::vector<std::string> strings =
        angle::SplitString(str, " |", angle::WhitespaceHandling::TRIM_WHITESPACE,
                           angle::SplitResult::SPLIT_WANT_NONEMPTY);
    for (const std::string &enumString : strings)
    {
        value |= StringToGLenum(enumString.c_str());
    }
    return value;
}
}  // namespace gl
