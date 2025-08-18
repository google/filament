// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/api/tint.h"

////////////////////////////////////////////////////////////////////////////////
// The following includes are used by './tools/run gen' to add an implicit
// dependency from 'tint/api' to the libraries used to make up the Tint API.
////////////////////////////////////////////////////////////////////////////////
// IWYU pragma: begin_keep
#include "src/tint/api/common/override_id.h"

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/reader.h"  // nogncheck
#endif

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/inspector/inspector.h"  // nogncheck
#include "src/tint/lang/wgsl/reader/reader.h"        // nogncheck
#endif

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"  // nogncheck
#endif

// IWYU pragma: end_keep

namespace tint {

/// Initialize initializes the Tint library. Call before using the Tint API.
void Initialize() {
#if TINT_BUILD_WGSL_WRITER
    // Register the Program printer. This is used for debugging purposes.
    tint::Program::printer = [](const tint::Program& program) {
        auto result = wgsl::writer::Generate(program);
        if (result != Success) {
            return result.Failure().reason;
        }
        return result->wgsl;
    };
#endif
}

/// Shutdown uninitializes the Tint library. Call after using the Tint API.
void Shutdown() {
    // Currently no-op, but may release tint resources in the future.
}

Result<std::string> SpirvToWgsl([[maybe_unused]] const std::vector<uint32_t>& spirv,
                                [[maybe_unused]] const wgsl::writer::ProgramOptions& wgsl_options) {
#if !TINT_BUILD_SPV_READER
    return Failure{"Tint SPIR-V reader is not enabled"};
#elif !TINT_BUILD_WGSL_WRITER
    return Failure{"Tint WGSL writer is not enabled"};
#else
    // Convert the SPIR-V program to an IR module.
    auto ir_from_spirv = tint::spirv::reader::ReadIR(spirv);
    if (ir_from_spirv != Success) {
        return ir_from_spirv.Failure();
    }

    // Convert the IR module to WGSL.
    auto wgsl_from_ir = tint::wgsl::writer::WgslFromIR(ir_from_spirv.Get(), wgsl_options);
    if (wgsl_from_ir != Success) {
        return wgsl_from_ir.Failure();
    }

    return wgsl_from_ir->wgsl;
#endif  // TINT_BUILD_SPV_READER && TINT_BUILD_WGSL_WRITER
}

}  // namespace tint
