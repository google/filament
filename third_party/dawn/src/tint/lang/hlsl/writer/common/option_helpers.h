/// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTION_HELPERS_H_
#define SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTION_HELPERS_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/common/multiplanar_options.h"
#include "src/tint/lang/hlsl/writer/common/options.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::hlsl::writer {

/// The remapper data
using RemapperData = std::unordered_map<BindingPoint, BindingPoint>;

/// @param options the options
/// @returns success or failure
diag::Result<SuccessType> ValidateBindingOptions(const Options& options);

/// Populates binding-related option from the writer options
/// @param options the writer options
/// @param remapper_data where to put the remapper data
/// @param multiplanar_map where to store the multiplanar texture options
/// @param array_length_from_uniform_options where to store the ArrayLengthFromUniform options
/// Note, these are populated together because there are dependencies between the two types of data.
void PopulateBindingRelatedOptions(
    const Options& options,
    RemapperData& remapper_data,
    tint::transform::multiplanar::BindingsMap& multiplanar_map,
    ArrayLengthFromUniformOptions& array_length_from_uniform_options);

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_COMMON_OPTION_HELPERS_H_
