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

#ifndef SRC_DAWN_NATIVE_BLOB_H_
#define SRC_DAWN_NATIVE_BLOB_H_

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// Blob represents a block of bytes. It may be constructed from
// various other container types and uses type erasure to take
// ownership of the container and release its memory on destruction.
class Blob {
  public:
    // This function is used to create Blob with actual data.
    // Make sure the creation and deleter handles the data ownership and lifetime correctly.
    static Blob UnsafeCreateWithDeleter(uint8_t* data, size_t size, std::function<void()> deleter);

    Blob();
    ~Blob();

    Blob(const Blob&) = delete;
    Blob& operator=(const Blob&) = delete;

    Blob(Blob&&);
    Blob& operator=(Blob&&);

    bool Empty() const;
    const uint8_t* Data() const;
    uint8_t* Data();
    size_t Size() const;

    bool operator==(const Blob& other) const;

  private:
    // The constructor should be responsible to take ownership of |data| and releases ownership by
    // calling |deleter|. The deleter function is called at ~Blob() and during std::move.
    explicit Blob(uint8_t* data, size_t size, std::function<void()> deleter);

    raw_ptr<uint8_t> mData;
    size_t mSize;
    std::function<void()> mDeleter;
};

Blob CreateBlob(size_t size);

template <typename T>
Blob CreateBlob(std::vector<T> vec)
    requires std::is_fundamental_v<T>
{
    uint8_t* data = reinterpret_cast<uint8_t*>(vec.data());
    size_t size = vec.size() * sizeof(T);
    // Move the vector into a new allocation so we can destruct it in the deleter.
    auto* wrapped_vec = new std::vector<T>(std::move(vec));
    return Blob::UnsafeCreateWithDeleter(data, size, [wrapped_vec] { delete wrapped_vec; });
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOB_H_
