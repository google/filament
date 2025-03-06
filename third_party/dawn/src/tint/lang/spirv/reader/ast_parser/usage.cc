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

#include "src/tint/lang/spirv/reader/ast_parser/usage.h"

#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {

Usage::Usage() {}
Usage::Usage(const Usage& other) = default;
Usage::~Usage() = default;

StringStream& Usage::operator<<(StringStream& out) const {
    out << "Usage(";
    if (IsSampler()) {
        out << "Sampler(";
        if (is_comparison_sampler_) {
            out << " comparison";
        }
        out << " )";
    }
    if (IsTexture()) {
        out << "Texture(";
        if (is_sampled_) {
            out << " is_sampled";
        }
        if (is_multisampled_) {
            out << " ms";
        }
        if (is_depth_) {
            out << " depth";
        }
        if (is_storage_read_) {
            out << " read";
        }
        if (is_storage_write_) {
            out << " write";
        }
        out << " )";
    }
    out << ")";
    return out;
}

bool Usage::IsValid() const {
    // Check sampler state internal consistency.
    if (is_comparison_sampler_ && !is_sampler_) {
        return false;
    }

    // Check texture state.
    // |is_texture_| is implied by any of the later texture-based properties.
    if ((IsStorageTexture() || is_sampled_ || is_multisampled_ || is_depth_) && !is_texture_) {
        return false;
    }
    if (is_texture_) {
        // Multisampled implies sampled.
        if (is_multisampled_) {
            if (!is_sampled_) {
                return false;
            }
        }
        // Depth implies sampled.
        if (is_depth_) {
            if (!is_sampled_) {
                return false;
            }
        }

        // Sampled can't be storage.
        if (is_sampled_) {
            if (IsStorageTexture()) {
                return false;
            }
        }

        // Storage can't be sampled.
        if (IsStorageTexture()) {
            if (is_sampled_) {
                return false;
            }
        }
        // Storage texture can't also be a sampler.
        if (IsStorageTexture()) {
            if (is_sampler_) {
                return false;
            }
        }
    }
    return true;
}

bool Usage::IsComplete() const {
    if (!IsValid()) {
        return false;
    }
    if (IsSampler()) {
        return true;
    }
    if (IsTexture()) {
        return is_sampled_ || IsStorageTexture();
    }
    return false;
}

bool Usage::operator==(const Usage& other) const {
    return is_sampler_ == other.is_sampler_ &&
           is_comparison_sampler_ == other.is_comparison_sampler_ &&
           is_texture_ == other.is_texture_ && is_sampled_ == other.is_sampled_ &&
           is_multisampled_ == other.is_multisampled_ && is_depth_ == other.is_depth_ &&
           is_storage_read_ == other.is_storage_read_ &&
           is_storage_write_ == other.is_storage_write_;
}

void Usage::Add(const Usage& other) {
    is_sampler_ = is_sampler_ || other.is_sampler_;
    is_comparison_sampler_ = is_comparison_sampler_ || other.is_comparison_sampler_;
    is_texture_ = is_texture_ || other.is_texture_;
    is_sampled_ = is_sampled_ || other.is_sampled_;
    is_multisampled_ = is_multisampled_ || other.is_multisampled_;
    is_depth_ = is_depth_ || other.is_depth_;
    is_storage_read_ = is_storage_read_ || other.is_storage_read_;
    is_storage_write_ = is_storage_write_ || other.is_storage_write_;
}

void Usage::AddSampler() {
    is_sampler_ = true;
}

void Usage::AddComparisonSampler() {
    AddSampler();
    is_comparison_sampler_ = true;
}

void Usage::AddTexture() {
    is_texture_ = true;
}

void Usage::AddStorageReadTexture() {
    AddTexture();
    is_storage_read_ = true;
}

void Usage::AddStorageWriteTexture() {
    AddTexture();
    is_storage_write_ = true;
}

void Usage::AddSampledTexture() {
    AddTexture();
    is_sampled_ = true;
}

void Usage::AddMultisampledTexture() {
    AddSampledTexture();
    is_multisampled_ = true;
}

void Usage::AddDepthTexture() {
    AddSampledTexture();
    is_depth_ = true;
}

std::string Usage::to_str() const {
    StringStream ss;
    ss << *this;
    return ss.str();
}

}  // namespace tint::spirv::reader::ast_parser
