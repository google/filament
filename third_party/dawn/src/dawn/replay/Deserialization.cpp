// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/replay/Deserialization.h"

#include <cstdint>
#include <span>
#include <string>

#include "dawn/replay/ReadHead.h"

namespace dawn::replay {

MaybeError ReadBytes(ReadHead& s, void* data, size_t size) {
    return s.ReadBytes(std::span<uint8_t>(static_cast<uint8_t*>(data), size));
}

MaybeError Deserialize(ReadHead& s, int32_t* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, uint8_t* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, uint16_t* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, uint32_t* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, uint64_t* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, float* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, double* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, bool* v) {
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

MaybeError Deserialize(ReadHead& s, std::string* v) {
    size_t size = 0;
    DAWN_TRY(Deserialize(s, &size));
    v->resize(size);
    return ReadBytes(s, v->data(), size);
}

}  // namespace dawn::replay
