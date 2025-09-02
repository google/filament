// Copyright 2022 The Dawn & Tint Authors
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

#ifndef DAWN_WIRE_OBJECTHANDLE_H_
#define DAWN_WIRE_OBJECTHANDLE_H_

#include <cstddef>
#include <cstdint>

#include "dawn/common/HashUtils.h"
#include "dawn/wire/Wire.h"

namespace dawn::wire {

using ObjectId = uint32_t;
using ObjectGeneration = uint32_t;

// ObjectHandle identifies some WebGPU object in the wire.
// An ObjectHandle will never be reused, so can be used to uniquely identify an object forever.
struct ObjectHandle : public Handle {
    ObjectHandle();
    ObjectHandle(ObjectId objId, ObjectGeneration objGeneration);

    explicit ObjectHandle(const volatile ObjectHandle& rhs);
    ObjectHandle& operator=(const volatile ObjectHandle& rhs);

    ObjectHandle(const ObjectHandle& rhs);
    ObjectHandle& operator=(const ObjectHandle& rhs);

    // Allow direct conversion from the base Handle type.
    // NOLINTNEXTLINE(runtime/explicit)
    ObjectHandle(const Handle& rhs);

    // MSVC has a bug where it thinks the volatile copy assignment is a duplicate.
    // Workaround this by forwarding to a different function AssignFrom.
    template <typename T>
    ObjectHandle& operator=(const T& rhs) {
        return AssignFrom(rhs);
    }
    ObjectHandle& AssignFrom(const ObjectHandle& rhs);
    ObjectHandle& AssignFrom(const volatile ObjectHandle& rhs);

    bool IsValid() const;
};

}  // namespace dawn::wire

template <>
struct std::hash<dawn::wire::ObjectHandle> {
    size_t operator()(const dawn::wire::ObjectHandle& value) const {
        size_t hash = dawn::Hash(value.id);
        dawn::HashCombine(&hash, value.generation);
        return hash;
    }
};

#endif  // DAWN_WIRE_OBJECTHANDLE_H_
