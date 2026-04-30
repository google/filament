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

#ifndef SRC_TINT_LANG_NULL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_NULL_WRITER_COMMON_OPTIONS_H_

#include <string>

#include "src/tint/api/common/substitute_overrides_config.h"
#include "src/tint/utils/reflection.h"

namespace tint::null::writer {

/// Configuration options used for Null generator.
struct Options {
    /// Constructor
    Options();
    /// Destructor
    ~Options();
    /// Copy constructor
    Options(const Options&);
    /// Copy assignment
    /// @returns this Options
    Options& operator=(const Options&);

    /// The entry point name to emit
    std::string entry_point_name = {};

    // Substitute Overrides
    SubstituteOverridesConfig substitute_overrides_config = {};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Options, entry_point_name, substitute_overrides_config);
    TINT_REFLECT_HASH_CODE(Options);

    bool operator==(const Options&) const = default;
};

}  // namespace tint::null::writer

#endif  // SRC_TINT_LANG_NULL_WRITER_COMMON_OPTIONS_H_
