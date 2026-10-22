// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_BYTES_BUFFER_READER_H_
#define SRC_TINT_UTILS_BYTES_BUFFER_READER_H_

#include <cstddef>
#include <span>
#include <string_view>

#include "src/tint/utils/bytes/reader.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::bytes {

/// BufferReader is an implementation of the Reader interface backed by a buffer.
class BufferReader final : public Reader {
  public:
    /// Destructor
    ~BufferReader() override;

    // This constructor represents the boundary between unsafe and safe memory constructs, since a
    // raw pointer is being converted to a std::span. There is no way to avoid this warning, because
    // if the compiler could statically guarantee the raw pointer + size was valid here, there would
    // be no need for std::span to exist.
    TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE_IN_CONTAINER);
    /// Constructor
    /// @param data the data to read from
    /// @param size the number of bytes in the buffer
    BufferReader(const std::byte* data, size_t size) : data_(data, size) { TINT_ASSERT(data); }
    TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE_IN_CONTAINER);

    /// Constructor
    /// @param str the string to read from
    explicit BufferReader(std::string_view str) : data_(std::as_bytes(std::span{str})) {}

    /// Constructor
    /// @param span the byte span to read from
    explicit BufferReader(std::span<const std::byte> span) : data_(span) {
        TINT_ASSERT(span.data());
    }

    /// @copydoc Reader::Read
    size_t Read(std::byte* out, size_t count) override;

    /// @copydoc Reader::IsEOF
    bool IsEOF() const override;

  private:
    /// The data to read from
    std::span<const std::byte> data_;
};

}  // namespace tint::bytes

#endif  // SRC_TINT_UTILS_BYTES_BUFFER_READER_H_
