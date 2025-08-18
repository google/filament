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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_USAGE_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_USAGE_H_

#include <string>

#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {

/// Records the properties of a sampler or texture based on how it's used
/// by image instructions inside function bodies.
///
/// For example:
///
///   If %X is the "Image" parameter of an OpImageWrite instruction then
///    - The memory object declaration underlying %X will gain
///      AddStorageWriteTexture usage
///
///   If %Y is the "Sampled Image" parameter of an OpImageSampleDrefExplicitLod
///   instruction, and %Y is composed from sampler %YSam and image %YIm, then:
///    - The memory object declaration underlying %YSam will gain
///      AddComparisonSampler usage
///    - The memory object declaration unederlying %YIm will gain
///      AddSampledTexture and AddDepthTexture usages
class Usage {
  public:
    /// Constructor
    Usage();
    /// Copy constructor
    /// @param other the Usage to clone
    Usage(const Usage& other);
    /// Destructor
    ~Usage();

    /// @returns true if this usage is internally consistent
    bool IsValid() const;
    /// @returns true if the usage fully determines a WebGPU binding type.
    bool IsComplete() const;

    /// @returns true if this usage is a sampler usage.
    bool IsSampler() const { return is_sampler_; }
    /// @returns true if this usage is a comparison sampler usage.
    bool IsComparisonSampler() const { return is_comparison_sampler_; }

    /// @returns true if this usage is a texture usage.
    bool IsTexture() const { return is_texture_; }
    /// @returns true if this usage is a sampled texture usage.
    bool IsSampledTexture() const { return is_sampled_; }
    /// @returns true if this usage is a multisampled texture usage.
    bool IsMultisampledTexture() const { return is_multisampled_; }
    /// @returns true if this usage is a dpeth texture usage.
    bool IsDepthTexture() const { return is_depth_; }
    /// @returns true if this usage is a read-only storage texture
    bool IsStorageReadOnlyTexture() const { return is_storage_read_ && !is_storage_write_; }
    /// @returns true if this usage is a read-write storage texture
    bool IsStorageReadWriteTexture() const { return is_storage_read_ && is_storage_write_; }
    /// @returns true if this usage is a write-only storage texture
    bool IsStorageWriteOnlyTexture() const { return is_storage_write_ && !is_storage_read_; }

    /// @returns true if this is a storage texture.
    bool IsStorageTexture() const { return is_storage_read_ || is_storage_write_; }

    /// Emits this usage to the given stream
    /// @param out the output stream.
    /// @returns the modified stream.
    StringStream& operator<<(StringStream& out) const;

    /// Equality operator
    /// @param other the RHS of the equality test.
    /// @returns true if `other` is identical to `*this`
    bool operator==(const Usage& other) const;

    /// Adds the usages from another usage object.
    /// @param other the other usage
    void Add(const Usage& other);

    /// Records usage as a sampler.
    void AddSampler();
    /// Records usage as a comparison sampler.
    void AddComparisonSampler();

    /// Records usage as a texture of some kind.
    void AddTexture();
    /// Records usage as a read-only storage texture.
    void AddStorageReadTexture();
    /// Records usage as a write-only storage texture.
    void AddStorageWriteTexture();
    /// Records usage as a sampled texture.
    void AddSampledTexture();
    /// Records usage as a multisampled texture.
    void AddMultisampledTexture();
    /// Records usage as a depth texture.
    void AddDepthTexture();

    /// @returns this usage object as a string.
    std::string to_str() const;

  private:
    // Sampler properties.
    bool is_sampler_ = false;
    // A comparison sampler is always a sampler:
    //    |is_comparison_sampler_| implies |is_sampler_|
    bool is_comparison_sampler_ = false;

    // Texture properties.
    // |is_texture_| is always implied by any of the others below.
    bool is_texture_ = false;
    bool is_sampled_ = false;
    bool is_multisampled_ = false;  // This implies it's sampled as well.
    bool is_depth_ = false;
    bool is_storage_read_ = false;
    bool is_storage_write_ = false;
};

/// Writes the Usage to the stream
/// @param out the stream
/// @param u the Usage
/// @returns the stream so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const Usage& u) {
    return u.operator<<(out);
}

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_USAGE_H_
