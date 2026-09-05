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

#include "src/tint/lang/core/ir/validator.h"

#if TINT_ENABLE_IR_DUMPING
#include <iostream>
#endif

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/functional_validator.h"
#include "src/tint/lang/core/ir/structural_validator.h"
#include "src/tint/utils/text/styled_text_printer.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {

namespace {

/// Prints out the current IR state, iff ir.dump_ir_when_validating is set.
void DumpIRIfEnabled([[maybe_unused]] const Module& ir,
                     [[maybe_unused]] const std::string_view msg) {
#if TINT_ENABLE_IR_DUMPING
    if (ir.dump_ir_when_validating) {
        auto printer = StyledTextPrinter::Create(stdout);
        std::cout << "=========================================================\n";
        std::cout << "== IR dump " << msg << ":\n";
        std::cout << "=========================================================\n";
        printer->Print(Disassembler(ir).Text());
    }
#endif
}

/// The core IR validator.
class Validator {
  public:
    /// Create a core validator
    /// @param mod the module to be validated
    explicit Validator(const Module& mod) : mod_(mod) {}

    /// Destructor
    ~Validator() = default;

    /// Runs the validator over the module provided during construction
    /// @returns success or failure

    Result<SuccessType> Run() {
        validator::Structural s(mod_, diagnostics_);
        s.Validate();

        // Only run the functional validation if we are structurally valid
        if (!diagnostics_.ContainsErrors()) {
            validator::Functional f(mod_, diagnostics_, validator::Functional::ErrorSource::kIr);
            f.Validate();
        }

        if (diagnostics_.ContainsErrors()) {
            const StyledText disassembly = ir::Disassembler(mod_).Text();
            diagnostics_.AddNote(Source{}) << "# Disassembly\n" << disassembly;
            return Failure{diagnostics_.Str()};
        }
        return Success;
    }

  private:
    const Module& mod_;
    diag::List diagnostics_;
};

}  // namespace

Result<SuccessType> Validate(const Module& mod, std::string_view msg) {
    DumpIRIfEnabled(mod, msg);
    Validator v(mod);
    return v.Run();
}

void AssertValid(const Module& mod, std::string_view msg) {
    DumpIRIfEnabled(mod, msg);

#if TINT_ENABLE_IR_VALIDATION_ASSERTS
    if (mod.enable_validation_asserts) {
        Validator v(mod);
        auto result = v.Run();
        if (result != Success) {
            TINT_ICE() << "\n========================================================="
                       << "\n== IR validation failed " << msg << ":"
                       << "\n=========================================================\n"
                       << result.Failure().reason;
        }
    }
#endif
}

void AssertNoUnsupportedProperties(const Module& mod, Properties unsupported_properties) {
    auto check = mod.properties & unsupported_properties;
    TINT_IR_ASSERT(mod, check.Empty()) << "unsupported property '" << *check.begin() << "'";
}

}  // namespace tint::core::ir
