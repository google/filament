// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_
#define SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_

#include <cstdint>
#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::msl::writer {

/// A pair of byte size and alignment `uint32_t`s.
struct SizeAndAlign {
    /// The size
    uint32_t size;
    /// The alignment
    uint32_t align;
};

/// @param ty the type to generate size and align for
/// @returns the MSL packed type size and alignment in bytes for the given type.
SizeAndAlign MslPackedTypeSizeAndAlign(const core::type::Type* ty);

/// Converts a builtin to an attribute name
/// @param builtin the builtin to convert
/// @returns the string name of the builtin or blank on error
std::string BuiltinToAttribute(core::BuiltinValue builtin);

/// Converts interpolation attributes to an MSL attribute
/// @param type the interpolation type
/// @param sampling the interpolation sampling
/// @returns the string name of the attribute or blank on error
std::string InterpolationToAttribute(core::InterpolationType type,
                                     core::InterpolationSampling sampling);

/// Prints a float32 to the output stream
/// @param out the stream to write too
/// @param value the float32 value
void PrintF32(StringStream& out, float value);

/// Prints a float16 to the output stream
/// @param out the stream to write too
/// @param value the float16 value
void PrintF16(StringStream& out, float value);

/// Prints an int32 to the output stream
/// @param out the stream to write too
/// @param value the int32 value
void PrintI32(StringStream& out, int32_t value);

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_COMMON_PRINTER_SUPPORT_H_
