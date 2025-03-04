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

#include "src/tint/lang/wgsl/inspector/scalar.h"

namespace tint::inspector {

Scalar::Scalar() : type_(kNull) {}

Scalar::Scalar(bool val) : type_(kBool) {
    value_.b = val;
}

Scalar::Scalar(uint32_t val) : type_(kU32) {
    value_.u = val;
}

Scalar::Scalar(int32_t val) : type_(kI32) {
    value_.i = val;
}

Scalar::Scalar(float val) : type_(kFloat) {
    value_.f = val;
}

bool Scalar::IsNull() const {
    return type_ == kNull;
}

bool Scalar::IsBool() const {
    return type_ == kBool;
}

bool Scalar::IsU32() const {
    return type_ == kU32;
}

bool Scalar::IsI32() const {
    return type_ == kI32;
}

bool Scalar::IsFloat() const {
    return type_ == kFloat;
}

bool Scalar::AsBool() const {
    return value_.b;
}

uint32_t Scalar::AsU32() const {
    return value_.u;
}

int32_t Scalar::AsI32() const {
    return value_.i;
}

float Scalar::AsFloat() const {
    return value_.f;
}

}  // namespace tint::inspector
