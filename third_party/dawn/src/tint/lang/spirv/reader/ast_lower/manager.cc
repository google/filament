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

#include "src/tint/lang/spirv/reader/ast_lower/manager.h"
#include "src/tint/lang/spirv/reader/ast_lower/transform.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

/// If set to 1 then the transform::Manager will dump the WGSL of the program
/// before and after each transform. Helpful for debugging bad output.
#define TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM 0

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#include <iostream>
#define TINT_IF_PRINT_PROGRAM(x) x
#else  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
#define TINT_IF_PRINT_PROGRAM(x)
#endif  // TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM

namespace tint::ast::transform {

Manager::Manager() = default;
Manager::~Manager() = default;

Program Manager::Run(const Program& program_in, const DataMap& inputs, DataMap& outputs) const {
    const Program* program = &program_in;

#if TINT_PRINT_PROGRAM_FOR_EACH_TRANSFORM
    auto print_program = [&](const char* msg, const Transform* transform) {
        auto wgsl = Program::printer(*program);
        std::cout << "=========================================================\n";
        std::cout << "== " << msg << " "
                  << (transform ? transform->TypeInfo().name : "transform manager") << ":\n";
        std::cout << "=========================================================\n";
        std::cout << wgsl << "\n";
        if (!program->IsValid()) {
            std::cout << "-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n"
                      << program->Diagnostics() << "\n";
        }
        std::cout << "=========================================================\n\n";
    };
#endif

    std::optional<Program> output;

    TINT_IF_PRINT_PROGRAM(print_program("Input of", nullptr));

    for (const auto& transform : transforms_) {
        if (auto result = transform->Apply(*program, inputs, outputs)) {
            output.emplace(std::move(result.value()));
            program = &output.value();

            if (!program->IsValid()) {
                TINT_IF_PRINT_PROGRAM(print_program("Invalid output of", transform.get()));
                break;
            }

            TINT_IF_PRINT_PROGRAM(print_program("Output of", transform.get()));
        } else {
            TINT_IF_PRINT_PROGRAM(std::cout << "Skipped " << transform->TypeInfo().name << "\n");
        }
    }

    TINT_IF_PRINT_PROGRAM(print_program("Final output of", nullptr));

    if (!output) {
        ProgramBuilder b;
        program::CloneContext ctx{&b, program, /* auto_clone_symbols */ true};
        ctx.Clone();
        output = resolver::Resolve(b);
    }
    return std::move(output.value());
}

}  // namespace tint::ast::transform
