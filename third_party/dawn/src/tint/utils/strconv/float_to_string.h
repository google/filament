// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_STRCONV_FLOAT_TO_STRING_H_
#define SRC_TINT_UTILS_STRCONV_FLOAT_TO_STRING_H_

#include <string>

namespace tint::strconv {

/// Converts the float `f` to a string using fixed-point notation (not
/// scientific). The float will be printed with the full precision required to
/// describe the float. All trailing `0`s will be omitted after the last
/// non-zero fractional number, unless the fractional is zero, in which case the
/// number will end with `.0`.
/// @return the float f formatted to a string
std::string FloatToString(float f);

/// Converts the double `f` to a string using fixed-point notation (not
/// scientific). The double will be printed with the full precision required to
/// describe the double. All trailing `0`s will be omitted after the last
/// non-zero fractional number, unless the fractional is zero, in which case the
/// number will end with `.0`.
/// @return the double f formatted to a string
std::string DoubleToString(double f);

/// Converts the float `f` to a string, using hex float notation for infinities,
/// NaNs, or subnormal numbers. Otherwise behaves as FloatToString.
/// @return the float f formatted to a string
std::string FloatToBitPreservingString(float f);

/// Converts the double `f` to a string, using hex double notation for infinities,
/// NaNs, or subnormal numbers. Otherwise behaves as FloatToString.
/// @return the double f formatted to a string
std::string DoubleToBitPreservingString(double f);

}  // namespace tint::strconv

#endif  // SRC_TINT_UTILS_STRCONV_FLOAT_TO_STRING_H_
