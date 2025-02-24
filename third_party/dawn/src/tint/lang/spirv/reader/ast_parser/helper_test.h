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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_HELPER_TEST_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/utils/macros/compiler.h"

TINT_BEGIN_DISABLE_WARNING(NEWLINE_EOF);
TINT_BEGIN_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_BEGIN_DISABLE_WARNING(SIGN_CONVERSION);
TINT_BEGIN_DISABLE_WARNING(WEAK_VTABLES);
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
#include "source/opt/ir_context.h"
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
TINT_END_DISABLE_WARNING(WEAK_VTABLES);
TINT_END_DISABLE_WARNING(SIGN_CONVERSION);
TINT_END_DISABLE_WARNING(OLD_STYLE_CAST);
TINT_END_DISABLE_WARNING(NEWLINE_EOF);

#include "gtest/gtest.h"
#include "src/tint/lang/spirv/reader/ast_parser/ast_parser.h"
#include "src/tint/lang/spirv/reader/ast_parser/fail_stream.h"
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/namer.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/usage.h"

namespace tint::spirv::reader::ast_parser::test {

/// A test class that wraps ParseImpl
class ASTParserWrapperForTest {
  public:
    /// Constructor
    /// @param input the input data to parse
    explicit ASTParserWrapperForTest(const std::vector<uint32_t>& input);
    /// Dumps SPIR-V if the conversion succeeded, then destroys the wrapper.
    ~ASTParserWrapperForTest();

    /// Sets global state to force dumping of the assembly text of succesfully
    /// SPIR-V.
    static void DumpSuccessfullyConvertedSpirv() { dump_successfully_converted_spirv_ = true; }
    /// Marks the test has having deliberately invalid SPIR-V
    void DeliberatelyInvalidSpirv() { skip_dumping_spirv_ = true; }
    /// Marks the test's SPIR-V as not being suitable for dumping, for a stated
    /// reason.
    void SkipDumpingPending(std::string) { skip_dumping_spirv_ = true; }

    /// @returns a new function emitter for the given function ID.
    /// Assumes ASTParser::BuildInternalRepresentation has been run and
    /// succeeded.
    /// @param function_id the SPIR-V identifier of the function
    FunctionEmitter function_emitter(uint32_t function_id) {
        auto* spirv_function = impl_.ir_context()->GetFunction(function_id);
        return FunctionEmitter(&impl_, *spirv_function);
    }

    /// Run the parser
    /// @returns true if the parse was successful, false otherwise.
    bool Parse() { return impl_.Parse(); }

    /// @returns the program. The program builder in the parser will be reset
    /// after this.
    Program program() { return impl_.Program(resolve_); }

    /// @returns the namer object
    Namer& namer() { return impl_.namer(); }

    /// @returns a reference to the internal builder, without building the
    /// program. To be used only for testing.
    ProgramBuilder& builder() { return impl_.builder(); }

    /// @returns the accumulated error string
    const std::string error() { return impl_.error(); }

    /// @return true if failure has not yet occurred
    bool success() { return impl_.success(); }

    /// Logs failure, ands return a failure stream to accumulate diagnostic
    /// messages. By convention, a failure should only be logged along with
    /// a non-empty string diagnostic.
    /// @returns the failure stream
    FailStream& Fail() { return impl_.Fail(); }

    /// @returns a borrowed pointer to the internal representation of the module.
    /// This is null until BuildInternalModule has been called.
    spvtools::opt::IRContext* ir_context() { return impl_.ir_context(); }

    /// Builds the internal representation of the SPIR-V module.
    /// Assumes the module is somewhat well-formed.  Normally you
    /// would want to validate the SPIR-V module before attempting
    /// to build this internal representation. Also computes a topological
    /// ordering of the functions.
    /// This is a no-op if the parser has already failed.
    /// @returns true if the parser is still successful.
    bool BuildInternalModule() { return impl_.BuildInternalModule(); }

    /// Builds an internal representation of the SPIR-V binary,
    /// and parses the module, except functions, into a Tint AST module.
    /// Diagnostics are emitted to the error stream.
    /// @returns true if it was successful.
    bool BuildAndParseInternalModuleExceptFunctions() {
        return impl_.BuildAndParseInternalModuleExceptFunctions();
    }

    /// Builds an internal representation of the SPIR-V binary,
    /// and parses it into a Tint AST module.  Diagnostics are emitted
    /// to the error stream.
    /// @returns true if it was successful.
    bool BuildAndParseInternalModule() { return impl_.BuildAndParseInternalModule(); }

    /// Registers user names for SPIR-V objects, from OpName, and OpMemberName.
    /// Also synthesizes struct field names.  Ensures uniqueness for names for
    /// SPIR-V IDs, and uniqueness of names of fields within any single struct.
    /// This is a no-op if the parser has already failed.
    /// @returns true if parser is still successful.
    bool RegisterUserAndStructMemberNames() { return impl_.RegisterUserAndStructMemberNames(); }

    /// Register Tint AST types for SPIR-V types, including type aliases as
    /// needed.  This is a no-op if the parser has already failed.
    /// @returns true if parser is still successful.
    bool RegisterTypes() { return impl_.RegisterTypes(); }

    /// Register sampler and texture usage for memory object declarations.
    /// This must be called after we've registered line numbers for all
    /// instructions. This is a no-op if the parser has already failed.
    /// @returns true if parser is still successful.
    bool RegisterHandleUsage() { return impl_.RegisterHandleUsage(); }

    /// Emits module-scope variables.
    /// This is a no-op if the parser has already failed.
    /// @returns true if parser is still successful.
    bool EmitModuleScopeVariables() { return impl_.EmitModuleScopeVariables(); }

    /// @returns the set of SPIR-V IDs for imports of the "GLSL.std.450"
    /// extended instruction set.
    const std::unordered_set<uint32_t>& glsl_std_450_imports() const {
        return impl_.glsl_std_450_imports();
    }

    /// Converts a SPIR-V type to a Tint type, and saves it for fast lookup.
    /// If the type is only used for builtins, then register that specially,
    /// and return null.  If the type is a sampler, image, or sampled image, then
    /// return the Void type, because those opaque types are handled in a
    /// different way.
    /// On failure, logs an error and returns null.  This should only be called
    /// after the internal representation of the module has been built.
    /// @param id the SPIR-V ID of a type.
    /// @returns a Tint type, or nullptr
    const Type* ConvertType(uint32_t id) { return impl_.ConvertType(id); }

    /// Gets the list of decorations for a SPIR-V result ID.  Returns an empty
    /// vector if the ID is not a result ID, or if no decorations target that ID.
    /// The internal representation must have already been built.
    /// @param id SPIR-V ID
    /// @returns the list of decorations on the given ID
    DecorationList GetDecorationsFor(uint32_t id) const { return impl_.GetDecorationsFor(id); }

    /// Gets the list of decorations for the member of a struct.  Returns an empty
    /// list if the `id` is not the ID of a struct, or if the member index is out
    /// of range, or if the target member has no decorations.
    /// The internal representation must have already been built.
    /// @param id SPIR-V ID of a struct
    /// @param member_index the member within the struct
    /// @returns the list of decorations on the member
    DecorationList GetDecorationsForMember(uint32_t id, uint32_t member_index) const {
        return impl_.GetDecorationsForMember(id, member_index);
    }

    /// Converts a SPIR-V struct member decoration into a number of AST
    /// decorations. If the decoration is recognized but deliberately dropped,
    /// then returns an empty list without a diagnostic. On failure, emits a
    /// diagnostic and returns an empty list.
    /// @param struct_type_id the ID of the struct type
    /// @param member_index the index of the member
    /// @param member_ty the type of the member
    /// @param decoration an encoded SPIR-V Decoration
    /// @returns the AST decorations
    auto ConvertMemberDecoration(uint32_t struct_type_id,
                                 uint32_t member_index,
                                 const Type* member_ty,
                                 const Decoration& decoration) {
        return impl_.ConvertMemberDecoration(struct_type_id, member_index, member_ty, decoration);
    }

    /// For a SPIR-V ID that might define a sampler, image, or sampled image
    /// value, return the SPIR-V instruction that represents the memory object
    /// declaration for the object.  If we encounter an OpSampledImage along the
    /// way, follow the image operand when follow_image is true; otherwise follow
    /// the sampler operand. Returns nullptr if we can't trace back to a memory
    /// object declaration.  Emits an error and returns nullptr when the scan
    /// fails due to a malformed module. This method can be used any time after
    /// BuildInternalModule has been invoked.
    /// @param id the SPIR-V ID of the sampler, image, or sampled image
    /// @param follow_image indicates whether to follow the image operand of
    /// OpSampledImage
    /// @returns the memory object declaration for the handle, or nullptr
    const spvtools::opt::Instruction* GetMemoryObjectDeclarationForHandle(uint32_t id,
                                                                          bool follow_image) {
        return impl_.GetMemoryObjectDeclarationForHandle(id, follow_image);
    }

    /// @param entry_point the SPIR-V ID of an entry point.
    /// @returns the entry point info for the given ID
    const std::vector<EntryPointInfo>& GetEntryPointInfo(uint32_t entry_point) {
        return impl_.GetEntryPointInfo(entry_point);
    }

    /// Returns the handle usage for a memory object declaration.
    /// @param id SPIR-V ID of a sampler or image OpVariable or
    /// OpFunctionParameter
    /// @returns the handle usage, or an empty usage object.
    Usage GetHandleUsage(uint32_t id) const { return impl_.GetHandleUsage(id); }

    /// Returns the SPIR-V instruction with the given ID, or nullptr.
    /// @param id the SPIR-V result ID
    /// @returns the instruction, or nullptr on error
    const spvtools::opt::Instruction* GetInstructionForTest(uint32_t id) const {
        return impl_.GetInstructionForTest(id);
    }

    /// @returns info about the gl_Position builtin variable.
    const ASTParser::BuiltInPositionInfo& GetBuiltInPositionInfo() {
        return impl_.GetBuiltInPositionInfo();
    }

    /// Returns the source record for the SPIR-V instruction with the given
    /// result ID.
    /// @param id the SPIR-V result id.
    /// @return the Source record, or a default one
    Source GetSourceForResultIdForTest(uint32_t id) const {
        return impl_.GetSourceForResultIdForTest(id);
    }

    /// @param resolve if true, the resolver should be run on the program when its build
    void SetResolveOnBuild(bool resolve) { resolve_ = resolve; }

  private:
    ASTParser impl_;
    /// When true, indicates the input SPIR-V module should not be emitted.
    /// It's either deliberately invalid, or not supported for some pending
    /// reason.
    bool skip_dumping_spirv_ = false;
    static bool dump_successfully_converted_spirv_;
    bool resolve_ = true;
};

// Sets global state to force dumping of the assembly text of succesfully
// SPIR-V.
inline void DumpSuccessfullyConvertedSpirv() {
    ASTParserWrapperForTest::DumpSuccessfullyConvertedSpirv();
}

/// Returns the WGSL printed string of a program.
/// @param program the Program
/// @returns the WGSL printed string the program.
std::string ToString(const Program& program);

/// Returns the WGSL printed string of a statement list.
/// @param program the Program
/// @param stmts the statement list
/// @returns the WGSL printed string of a statement list.
std::string ToString(const Program& program, VectorRef<const ast::Statement*> stmts);

/// Returns the WGSL printed string of an AST node.
/// @param program the Program
/// @param node the AST node
/// @returns the WGSL printed string of the AST node.
std::string ToString(const Program& program, const ast::Node* node);

}  // namespace tint::spirv::reader::ast_parser::test

namespace tint::spirv::reader::ast_parser {

/// SPIR-V Parser test class
template <typename T>
class SpirvASTParserTestBase : public T {
  public:
    SpirvASTParserTestBase() = default;
    ~SpirvASTParserTestBase() override = default;

    /// Retrieves the parser from the helper
    /// @param input the SPIR-V binary to parse
    /// @returns a parser for the given binary
    std::unique_ptr<test::ASTParserWrapperForTest> parser(const std::vector<uint32_t>& input) {
        auto parser = std::make_unique<test::ASTParserWrapperForTest>(input);
        // Don't run the Resolver when building the program.
        // We're not interested in type information with these tests.
        parser->SetResolveOnBuild(false);
        return parser;
    }
};

/// SpirvASTParserTest the the base class for SPIR-V reader unit tests.
/// Use this form when you don't need to template any further.
using SpirvASTParserTest = SpirvASTParserTestBase<::testing::Test>;

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_HELPER_TEST_H_
