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

#include "src/tint/lang/spirv/reader/lower/lower.h"

#include "src/tint/lang/core/ir/transform/remove_terminator_args.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/reader/lower/atomics.h"
#include "src/tint/lang/spirv/reader/lower/builtins.h"
#include "src/tint/lang/spirv/reader/lower/shader_io.h"
#include "src/tint/lang/spirv/reader/lower/vector_element_pointer.h"

namespace tint::spirv::reader {

Result<SuccessType> Lower(core::ir::Module& mod) {
#define RUN_TRANSFORM(name, ...)         \
    do {                                 \
        auto result = name(__VA_ARGS__); \
        if (result != Success) {         \
            return result;               \
        }                                \
    } while (false)

    RUN_TRANSFORM(lower::VectorElementPointer, mod);
    RUN_TRANSFORM(lower::ShaderIO, mod);
    RUN_TRANSFORM(lower::Builtins, mod);
    RUN_TRANSFORM(lower::Atomics, mod);

    // Remove the terminator args at this point. There are no logical short-circuiting operators in
    // SPIR-V that we will lose track of, all the terminators are for hoisted values. We don't do
    // this on WGSL raise because `RemoveTerminatorArgs` loses the ability to determine logical `&&`
    // and `||` operators based on the terminators. Moving the logical detection to a separate
    // transform is also complicated because of the way it currently detects multi element `&&` and
    // `||` statements.
    RUN_TRANSFORM(core::ir::transform::RemoveTerminatorArgs, mod);

    auto res = core::ir::ValidateAndDumpIfNeeded(mod, "spirv.Lower",
                                                 core::ir::Capabilities{
                                                     core::ir::Capability::kAllowOverrides,
                                                 });
    if (res != Success) {
        return res.Failure();
    }

    return Success;
}

}  // namespace tint::spirv::reader
