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

#ifndef SRC_DAWN_COMMON_PLACEMENTALLOCATED_H_
#define SRC_DAWN_COMMON_PLACEMENTALLOCATED_H_

#include <cstddef>

namespace dawn {

class PlacementAllocated {
  public:
    // Delete the default new operator so this can only be created with placement new.
    void* operator new(size_t) = delete;

    void* operator new(size_t size, void* ptr) {
        // Pass through the pointer of the allocation. This is essentially the default
        // placement-new implementation, but we must define it if we delete the default
        // new operator.
        return ptr;
    }

    void operator delete(void* ptr) {
        // Object is placement-allocated. Don't free the memory.
    }

    void operator delete(void*, void*) {
        // This is added to match new(size_t size, void* ptr)
        // Otherwise it triggers C4291 warning in MSVC
    }
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_PLACEMENTALLOCATED_H_
