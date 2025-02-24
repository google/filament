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

#include "dawn/native/d3d/BlobD3D.h"

namespace dawn::native {

Blob CreateBlob(ComPtr<ID3DBlob> blob) {
    // Detach so the deleter callback can "own" the reference
    ID3DBlob* ptr = blob.Detach();
    return Blob::UnsafeCreateWithDeleter(reinterpret_cast<uint8_t*>(ptr->GetBufferPointer()),
                                         ptr->GetBufferSize(), [=] {
                                             // Reattach and drop to delete it.
                                             ComPtr<ID3DBlob> b;
                                             b.Attach(ptr);
                                             b = nullptr;
                                         });
}

Blob CreateBlob(ComPtr<IDxcBlob> blob) {
    // Detach so the deleter callback can "own" the reference
    IDxcBlob* ptr = blob.Detach();
    return Blob::UnsafeCreateWithDeleter(reinterpret_cast<uint8_t*>(ptr->GetBufferPointer()),
                                         ptr->GetBufferSize(), [=] {
                                             // Reattach and drop to delete it.
                                             ComPtr<IDxcBlob> b;
                                             b.Attach(ptr);
                                             b = nullptr;
                                         });
}

}  // namespace dawn::native
