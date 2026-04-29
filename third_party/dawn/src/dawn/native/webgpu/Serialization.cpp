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

#include "dawn/native/webgpu/Serialization.h"

#include <string>

#include "dawn/native/webgpu/CaptureContext.h"

namespace dawn::native::webgpu {

void WriteBytes(CaptureContext& context, const void* data, size_t size) {
    context.WriteCommandBytes(data, size);
}

void Serialize(CaptureContext& context, int32_t v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, uint8_t v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, uint16_t v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, uint32_t v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, uint64_t v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, float v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, double v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, bool v) {
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

void Serialize(CaptureContext& context, const std::string& v) {
    Serialize(context, v.size());
    WriteBytes(context, v.data(), v.size());
}

}  // namespace dawn::native::webgpu
