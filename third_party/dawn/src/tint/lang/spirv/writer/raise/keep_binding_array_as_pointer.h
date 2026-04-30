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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_RAISE_KEEP_BINDING_ARRAY_AS_POINTER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_RAISE_KEEP_BINDING_ARRAY_AS_POINTER_H_

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::spirv::writer::raise {

// The capabilities that the transform can support.
const core::ir::Capabilities kKeepBindingArrayAsPointerCapabilities{
    core::ir::Capability::kAllowDuplicateBindings,
    core::ir::Capability::kAllowNonCoreTypes,
};

/// KeepBindingArrayAsPointer is a transform that ensures that binding_arrays are never stored by
/// value but only used via a pointer to them. This is used to produce SPIR-V that's more similar to
/// what drivers typically ingest where OpTypeArray<OpTypeImage> is always kept as a pointer.
///
/// Note that it doesn't handle function parameters so DirectVariableAccess (DVA) for handles must
/// have run prior to this transform.
///
/// This mismatch between Tint IR and SPIR-V at the time of writing is because Tint IR disallows
/// handle address space pointers as function arguments, while "idiomatic" SPIR-V that drivers are
/// used to use pointers to pass handle types as function arguments.
///
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> KeepBindingArrayAsPointer(core::ir::Module& module);

}  // namespace tint::spirv::writer::raise

#endif  // SRC_TINT_LANG_SPIRV_WRITER_RAISE_KEEP_BINDING_ARRAY_AS_POINTER_H_
