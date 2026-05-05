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

#ifndef INCLUDE_TINT_TINT_H_
#define INCLUDE_TINT_TINT_H_

// Guard for accidental includes to private headers
#define CURRENTLY_IN_TINT_PUBLIC_HEADER

// TODO(tint:88): When implementing support for an install target, all of these
//                headers will need to be moved to include/tint/.

#include "src/tint/api/common/binding_point.h"                // IWYU pragma: export
#include "src/tint/api/common/resource_type.h"                // IWYU pragma: export
#include "src/tint/api/common/subgroup_matrix.h"              // IWYU pragma: export
#include "src/tint/api/common/substitute_overrides_config.h"  // IWYU pragma: export
#include "src/tint/api/common/vertex_pulling_config.h"        // IWYU pragma: export
#include "src/tint/api/common/workgroup_info.h"               // IWYU pragma: export
#include "src/tint/api/tint.h"                                // IWYU pragma: export
#include "src/tint/lang/core/type/manager.h"                  // IWYU pragma: export
#include "src/tint/lang/wgsl/enums.h"                         // IWYU pragma: export
#include "src/tint/lang/wgsl/feature_status.h"                // IWYU pragma: export
#include "src/tint/lang/wgsl/inspector/inspector.h"           // IWYU pragma: export
#include "src/tint/utils/diagnostic/formatter.h"              // IWYU pragma: export
#include "src/tint/utils/text/styled_text.h"                  // IWYU pragma: export

///////////////
// NOTE if adding a new guard include here, it must also appear in src/tint/api/tint.cc for the
// build to work correctly.
///////////////

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/reader.h"
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/reader.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/writer.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/writer/writer.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/writer/writer.h"
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/writer.h"
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_NULL_WRITER
#include "src/tint/lang/null/writer/writer.h"
#endif  // TINT_BUILD_NULL_WRITER

#undef CURRENTLY_IN_TINT_PUBLIC_HEADER

#endif  // INCLUDE_TINT_TINT_H_
