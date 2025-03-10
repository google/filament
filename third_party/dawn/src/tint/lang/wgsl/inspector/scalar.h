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

#ifndef SRC_TINT_LANG_WGSL_INSPECTOR_SCALAR_H_
#define SRC_TINT_LANG_WGSL_INSPECTOR_SCALAR_H_

#include <cstdint>

namespace tint::inspector {

/// Contains a literal scalar value
class Scalar {
  public:
    /// Null Constructor
    Scalar();
    /// @param val literal scalar value to contain
    explicit Scalar(bool val);
    /// @param val literal scalar value to contain
    explicit Scalar(uint32_t val);
    /// @param val literal scalar value to contain
    explicit Scalar(int32_t val);
    /// @param val literal scalar value to contain
    explicit Scalar(float val);

    /// @returns true if this is a null
    bool IsNull() const;
    /// @returns true if this is a bool
    bool IsBool() const;
    /// @returns true if this is a unsigned integer.
    bool IsU32() const;
    /// @returns true if this is a signed integer.
    bool IsI32() const;
    /// @returns true if this is a float.
    bool IsFloat() const;

    /// @returns scalar value if bool, otherwise undefined behaviour.
    bool AsBool() const;
    /// @returns scalar value if unsigned integer, otherwise undefined behaviour.
    uint32_t AsU32() const;
    /// @returns scalar value if signed integer, otherwise undefined behaviour.
    int32_t AsI32() const;
    /// @returns scalar value if float, otherwise undefined behaviour.
    float AsFloat() const;

  private:
    typedef enum {
        kNull,
        kBool,
        kU32,
        kI32,
        kFloat,
    } Type;

    typedef union {
        bool b;
        uint32_t u;
        int32_t i;
        float f;
    } Value;

    Type type_;
    Value value_;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_SCALAR_H_
