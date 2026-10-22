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

#ifndef SRC_TINT_LANG_SPIRV_TYPE_IMAGE_H_
#define SRC_TINT_LANG_SPIRV_TYPE_IMAGE_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::spirv::type {

enum class Dim : uint8_t {
    kD1 = 0,
    kD2 = 1,
    kD3 = 2,
    kCube = 3,
    // Not used in WGSL
    // Rect = 4,
    kBuffer = 5,
    kSubpassData = 6,
};

enum class Depth : uint8_t {
    kNotDepth = 0,
    kDepth = 1,
    kUnknown = 2,
};

enum class Arrayed : uint8_t {
    kNonArrayed = 0,
    kArrayed = 1,
};

enum class Multisampled : uint8_t {
    kSingleSampled = 0,
    kMultisampled = 1,
};

enum class Sampled : uint8_t {
    // KnownAtRuntime is not allowed in Vulkan environment, so ignore it
    kSamplingCompatible = 1,
    kReadWriteOpCompatible = 2,
};

/// Image represents an OpTypeImage in SPIR-V.
class Image final : public Castable<Image, core::type::Type> {
  public:
    /// Constructor
    /// @param sampled_type the type of the components that result from sampling or reading
    /// @param dim the image dimensionality
    /// @param depth image depth information
    /// @param arrayed image arrayed information
    /// @param ms the image multisampled information
    /// @param sampled the image sampled information
    /// @param fmt the image format
    /// @param access the image access qualifier
    Image(const core::type::Type* sampled_type,
          Dim dim,
          Depth depth,
          Arrayed arrayed,
          Multisampled ms,
          Sampled sampled,
          core::TexelFormat fmt,
          core::Access access);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    const core::type::Type* GetSampledType() const { return sampled_type_; }
    Dim GetDim() const { return dim_; }
    Depth GetDepth() const { return depth_; }
    Arrayed GetArrayed() const { return arrayed_; }
    Multisampled GetMultisampled() const { return ms_; }
    Sampled GetSampled() const { return sampled_; }
    core::TexelFormat GetTexelFormat() const { return fmt_; }
    core::Access GetAccess() const { return access_; }

    /// @returns the friendly name for this type
    std::string FriendlyName() const override;

    bool IsHandle() const override { return true; }

    /// @param ctx the clone context
    /// @returns a clone of this type
    Image* Clone(core::type::CloneContext& ctx) const override;

  private:
    const core::type::Type* sampled_type_;
    Dim dim_;
    Depth depth_;
    Arrayed arrayed_;
    Multisampled ms_;
    Sampled sampled_;
    core::TexelFormat fmt_;
    core::Access access_;
};

}  // namespace tint::spirv::type

#endif  // SRC_TINT_LANG_SPIRV_TYPE_IMAGE_H_
