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

#ifndef SRC_TINT_LANG_GLSL_WRITER_COMMON_VERSION_H_
#define SRC_TINT_LANG_GLSL_WRITER_COMMON_VERSION_H_

#include <cstdint>

#include "src/tint/utils/reflection/reflection.h"

namespace tint::glsl::writer {

/// A structure representing the version of GLSL to be generated.
struct Version {
    /// Is this version desktop GLSL, or GLSL ES?
    enum class Standard {
        kDesktop,
        kES,
    };

    /// Constructor
    /// @param standard_ Desktop or ES
    /// @param major_ the major version
    /// @param minor_ the minor version
    Version(Standard standard_, uint32_t major_, uint32_t minor_)
        : standard(standard_), major_version(major_), minor_version(minor_) {}

    /// Default constructor (see default values below)
    Version() = default;

    /// @returns true if this version is GLSL ES
    bool IsES() const { return standard == Standard::kES; }

    /// @returns true if this version is Desktop GLSL
    bool IsDesktop() const { return standard == Standard::kDesktop; }

    /// Desktop or ES
    Standard standard = Standard::kES;

    /// Major GLSL version
    uint32_t major_version = 3;

    /// Minor GLSL version
    uint32_t minor_version = 1;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Version, standard, major_version, minor_version);
};

}  // namespace tint::glsl::writer

namespace tint {

/// Relect enum information for Version
TINT_REFLECT_ENUM_RANGE(glsl::writer::Version::Standard, kDesktop, kES);

}  // namespace tint

#endif  // SRC_TINT_LANG_GLSL_WRITER_COMMON_VERSION_H_
