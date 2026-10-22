// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_STRUCTURAL_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_STRUCTURAL_VALIDATOR_H_

#include <cstdint>
#include <functional>
#include <string>

#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/io_attribute_validator.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/swizzle_view.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {

/// State for validating blend_src attributes shared across multiple passes within the same entry
/// point.
struct BlendSrcContext {
    Function::PipelineStage stage;
    Hashmap<uint32_t, const CastableBase*, 4> locations;
    Hashset<uint32_t, 2> blend_srcs;
    const core::type::Type* blend_src_type = nullptr;
    IODirection dir;
};

using SupportedStages = tint::EnumSet<Function::PipelineStage>;

class Structural {
  public:
    Structural(const Module& ir, diag::List& diagnostics);
    ~Structural();

    void Validate();

    Structural(const Structural&) = delete;
    Structural(Structural&&) = delete;
    Structural& operator=(const Structural&) = delete;
    Structural& operator=(Structural&&) = delete;

  private:
    /// Runs validation to confirm the structural soundness of the module.
    /// Also runs any validation that is not dependent on the entire module being
    /// sound and sets up data structures for later checks.
    void RunStructuralSoundnessChecks();

    /// Checks that there is no direct or indirect recursion.
    /// Depends on CheckStructuralSoundness() having previously been run.
    void CheckForRecursion();

    /// Checks that there are no orphaned instructions
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForOrphanedInstructions();

    /// Checks that entry points do not use instructions that are not supported by their stage.
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckStageRestrictedInstructions();

    /// @returns the IR disassembly, performing a disassemble if this is the first call.
    ir::Disassembler& Disassemble();

    /// Adds an error for the @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst);

    /// Adds an error for the @p inst operand at @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p inst result at @p idx and highlgihts the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns the diagnostic
    diag::Diagnostic& AddResultError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p block and highlights the block header in the disassembly
    /// @param blk the block
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Block* blk);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const BlockParam* param);

    /// Adds an error for the @p func and highlights the function in the disassembly
    /// @param func the function
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Function* func);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const FunctionParam* param);

    /// Adds an error for the castable base @p base and highlights it in the disassembly
    /// @param base the declaration to add an error for
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const CastableBase* base);

    /// Adds an error the @p block and highlights the block header in the disassembly
    /// @param src the source lines to highlight
    /// @returns the diagnostic
    diag::Diagnostic& AddError(Source src);

    /// Adds a note to @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    diag::Diagnostic& AddNote(const Instruction* inst);

    /// Adds a note to @p func and highlights the function in the disassembly
    /// @param func the function
    diag::Diagnostic& AddNote(const Function* func);

    /// Adds a note to @p inst for operand @p idx and highlights the operand in the disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    diag::Diagnostic& AddOperandNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p inst for result @p idx and highlights the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    diag::Diagnostic& AddResultNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p blk and highlights the block in the disassembly
    /// @param blk the block
    diag::Diagnostic& AddNote(const Block* blk);

    /// Adds a note to the diagnostics
    /// @param src the source lines to highlight
    diag::Diagnostic& AddNote(Source src = {});

    /// Adds a note to the diagnostics highlighting where the value instruction or block is
    /// declared, if it has a source location.
    /// @param decl the value instruction or block
    void AddDeclarationNote(const CastableBase* decl);

    /// Adds a note to the diagnostics highlighting where the block is declared, if it has a source
    /// location.
    /// @param block the block
    void AddDeclarationNote(const Block* block);

    /// Adds a note to the diagnostics highlighting where the block parameter is declared, if it
    /// has a source location.
    /// @param param the block parameter
    void AddDeclarationNote(const BlockParam* param);

    /// Adds a note to the diagnostics highlighting where the function is declared, if it has a
    /// source location.
    /// @param fn the function
    void AddDeclarationNote(const Function* fn);

    /// Adds a note to the diagnostics highlighting where the function parameter is declared, if it
    /// has a source location.
    /// @param param the function parameter
    void AddDeclarationNote(const FunctionParam* param);

    /// Adds a note to the diagnostics highlighting where the instruction is declared, if it has a
    /// source location.
    /// @param inst the inst
    void AddDeclarationNote(const Instruction* inst);

    /// Adds a note to the diagnostics highlighting where instruction result was declared, if it has
    /// a source location.
    /// @param res the res
    void AddDeclarationNote(const InstructionResult* res);

    /// @param decl the type, value, instruction or block to get the name for
    /// @returns the styled name for the given value, instruction or block
    StyledText NameOf(const CastableBase* decl);

    // @param ty the type to get the name for
    /// @returns the styled name for the given type
    StyledText NameOf(const core::type::Type* ty);

    /// @param v the value to get the name for
    /// @returns the styled name for the given value
    StyledText NameOf(const Value* v);

    /// @param inst the instruction to get the name for
    /// @returns the styled  name for the given instruction
    StyledText NameOf(const Instruction* inst);

    /// @param block the block to get the name for
    /// @returns the styled  name for the given block
    StyledText NameOf(const Block* block);

    /// Checks the given result is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns true if the result is not null
    bool CheckResult(const Instruction* inst, size_t idx);

    /// Checks the results (and their types) for @p inst are not null. If count is specified then
    /// number of results is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of results to check
    /// @returns true if the results count is as expected and none are null
    bool CheckResults(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the given operand is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns true if the operand is not null
    bool CheckOperand(const Instruction* inst, size_t idx);

    /// Checks the number of operands provided to @p inst and that none of them are null. Also
    /// checks that the types for the operands are not null
    /// @param inst the instruction
    /// @param min_count the minimum number of operands to expect
    /// @param max_count the maximum number of operands to expect, if not set, than only the minimum
    /// number is checked.
    /// @returns true if the number of operands is in the expected range and none are null
    bool CheckOperands(const ir::Instruction* inst,
                       size_t min_count,
                       std::optional<size_t> max_count);

    /// Checks the operands (and their types) for @p inst are not null. If count is specified then
    /// number of operands is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of operands to check
    /// @returns true if the operands count is as expected and none are null
    bool CheckOperands(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the number of results for @p inst are exactly equal to @p num_results and the number
    /// of operands is correctly. Both results and operands are confirmed to be non-null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param min_operands the minimum number of operands to expect
    /// @param max_operands the maximum number of operands to expect, if not set, than only the
    /// minimum number is checked.
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperandRange(const ir::Instruction* inst,
                                     size_t num_results,
                                     size_t min_operands,
                                     std::optional<size_t> max_operands);

    /// Checks the number of results and operands for @p inst are exactly equal to num_results
    /// and num_operands, respectively, and that none of them are null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param num_operands expected number of operands for the instruction
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperands(const ir::Instruction* inst,
                                 size_t num_results,
                                 size_t num_operands);

    /// Checks that @p type is allowed by the spec, and does not use any types that are prohibited
    /// by the target properties.
    /// NOTE: Expects to be called on a 'root' type, i.e. the type of a variable declaration or a
    ///       function param, not in the middle a walk of elements of a composite.
    /// @param type the type
    /// @param diag a function that creates an error diagnostic for the source of the type
    void CheckType(const core::type::Type* type, std::function<diag::Diagnostic&()> diag);

    /// Check that @p type and its children are not nested beyond the depth limit
    /// NOTE: Expects to be called by CheckType, i.e. on a 'root' type, not in the middle a walk of
    ///       elements of a composite..
    /// @param type the type
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckNestDepth(const core::type::Type* type, std::function<diag::Diagnostic&()> diag);

    /// Checks that `str` is a valid structure.
    /// @param str the struct to validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckStruct(const core::type::Struct* str, std::function<diag::Diagnostic&()>& diag);

    /// Checks that `ref` is a valid reference type
    /// @param ref the type to check
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param root the root type of the ref type, maybe the ref itself.
    bool CheckRef(const core::type::Reference* ref,
                  std::function<diag::Diagnostic&()>& diag,
                  const core::type::Type* root);

    /// Checks that `arr` is a valid array type
    /// @param arr the array the validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckArray(const core::type::Array* arr, std::function<diag::Diagnostic&()>& diag);
    /// Checks that `vec` is a valid vector type
    /// @param vec the vector the validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckVector(const core::type::Vector* vec, std::function<diag::Diagnostic&()>& diag);
    /// Checks that `mat` is a valid matrix type
    /// @param mat the matrix the validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckMatrix(const core::type::Matrix* mat, std::function<diag::Diagnostic&()>& diag);

    /// Checks that `atom` is a valid atomic type
    /// @param atom the atomic to check
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckAtomic(const core::type::Atomic* atom, std::function<diag::Diagnostic&()>& diag);

    /// Checks that `s` is a valid sampled texture
    /// @param s the sampled texture to validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckSampledTexture(const core::type::SampledTexture* s,
                             std::function<diag::Diagnostic&()>& diag);
    /// Checks that `ms` is a valid multi-sampled texture
    /// @param ms the multi-sampled texture to validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckMultisampledTexture(const core::type::MultisampledTexture* ms,
                                  std::function<diag::Diagnostic&()>& diag);
    /// Checks that `storage` is a valid storage texture
    /// @param storage the storage texture
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckStorageTexture(const core::type::StorageTexture* storage,
                             std::function<diag::Diagnostic&()>& diag);

    /// Checks that `ia` is a valid input attachment
    /// @param ia the input attachment
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckInputAttachment(const core::type::InputAttachment* ia,
                              std::function<diag::Diagnostic&()>& diag);

    /// Checks that `m` is a valid subgroup matrix
    /// @param m the subgroup matrix
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param addrspace the address space of the root type
    bool CheckSubgroupMatrix(const core::type::SubgroupMatrix* m,
                             std::function<diag::Diagnostic&()>& diag,
                             core::AddressSpace addrspace);
    /// Checks that `ba` is a valid binding array
    /// @param ba the binding array
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param addrspace the address space of the root type
    bool CheckBindingArray(const core::type::BindingArray* ba,
                           std::function<diag::Diagnostic&()>& diag,
                           core::AddressSpace addrspace);

    /// Checks that buffers are available
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckBuffer(const core::type::Buffer* buf, std::function<diag::Diagnostic&()>& diag);

    /// Checks that `sv` is a valid swizzle view type
    /// @param sv the swizzle view to validate
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckSwizzleView(const core::type::SwizzleView* sv,
                          std::function<diag::Diagnostic&()>& diag);

    /// Checks that 8-bit integer types are permitted
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param parent the parent type for the 8-bit type
    bool Check8BitInteger(std::function<diag::Diagnostic&()>& diag, const core::type::Type* parent);
    /// Checks that 16-bit integer types are permitted
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool Check16BitInteger(std::function<diag::Diagnostic&()>& diag);
    /// Checks that 64-bit integer types are permitted
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool Check64BitInteger(std::function<diag::Diagnostic&()>& diag);

    /// Checks that 16-bit floats are allowed.
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool Check16BitFloat(std::function<diag::Diagnostic&()>& diag);

    /// Checks that `ptr` is a valid pointer type
    /// @param ptr the pointer to check
    /// @param diag a function that creates an error diagnostic for the source of the type
    bool CheckPtr(const core::type::Pointer* ptr, std::function<diag::Diagnostic&()>& diag);

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(const Block* blk);

    /// Validates the given instruction is only used in the root block.
    /// @param inst the instruction
    void CheckOnlyUsedInRootBlock(const Instruction* inst);

    /// Validates the given function
    /// @param func the function to validate
    void CheckFunction(const Function* func);
    /// Validates a function parameter
    /// @param param the parameter
    /// @returns true if validation should be continued
    bool CheckFunctionParam(const Function* func,
                            const FunctionParam* param,
                            Hashset<const FunctionParam*, 4>& param_set);
    /// Checks for entry point validation errors. Returns if `func` is not an entrypoint.
    void CheckEntryPoint(const Function* func);

    /// Validates the workgroup_size attribute for a given function
    /// @param func the function to validate
    void CheckWorkgroupSize(const Function* func);

    /// Validates the subgroup_size attribute for a given function
    /// @param func the function to validate
    void CheckSubgroupSize(const Function* func);

    /// Validates the specific function as a vertex entry point
    /// @param ep the function to validate
    void CheckPositionPresentForVertexOutput(const Function* ep);

    /// Validates the spec rules for IO attribute usage for a function.
    /// @param func the function to validate
    void ValidateIOAttributes(const Function* func);

    /// Implementation for validating the spec rules for IO attribute usage.
    /// @param ctx context object shared between the multiple invocations of this per entry point
    /// @param msg_anchor the object to anchor the error message to
    /// @param ty the data type being decorated by the attributes
    /// @param attr the attributes to test
    /// @param stage the shader stage the builtin is being used
    /// @param dir is value being used as an input or an output
    /// @param io_kind is the type of shader IO object the attribute is attached to
    void ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                  const CastableBase* msg_anchor,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr,
                                  Function::PipelineStage stage,
                                  IODirection dir,
                                  ShaderIOKind io_kind);

    /// Validates that a type is a bool only if it is decorated with @builtin(front_facing).
    /// @param msg_anchor where to attach errors to
    /// @param attr the IO attributes
    /// @param ty the type
    /// @param err error message to log when check fails
    void CheckFrontFacingIfBool(const CastableBase* msg_anchor,
                                const IOAttributes& attr,
                                const core::type::Type* ty,
                                const std::string& err);

    /// Validates that a type is not a bool.
    /// @param msg_anchor where to attach errors to
    /// @param ty the type
    /// @param err error message to log when check fails
    void CheckNotBool(const CastableBase* msg_anchor,
                      const core::type::Type* ty,
                      const std::string& err);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given override
    /// @param o the override to validate
    void CheckOverride(const Override* o);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates annotations related to shader IO
    /// @param msg_anchor where to attach errors to
    /// @param ty type of the value under test
    /// @param binding_point the binding information associated with the value
    /// @param attr IO attributes associated with the values
    /// @param kind the kind Shader IO being performed
    void ValidateShaderIOAnnotations(const CastableBase* msg_anchor,
                                     const core::type::Type* ty,
                                     const std::optional<BindingPoint>& binding_point,
                                     const IOAttributes& attr,
                                     ShaderIOKind kind);

    /// Validates the attributes of a struct member.
    /// @param member the struct member
    /// @param diag a function that creates an error diagnostic
    /// @returns true if the attributes are valid
    bool CheckStructMemberAttributes(const core::type::StructMember* member,
                                     std::function<diag::Diagnostic&()> make_diag);

    /// Validates the blend_src attribute for a given type, responsible for traversal of inner types
    /// and checking rules that span across a multiple attribute instances.
    /// @param ctx the blend_src context.
    /// @param target the object that has the struct ty.
    /// @param ty the ty to validate.
    /// @param attr the IO attributes for the object.
    void CheckBlendSrc(BlendSrcContext& ctx,
                       const CastableBase* target,
                       const core::type::Type* ty,
                       const IOAttributes& attr);

    /// Validates the details of a single attribute instance.
    /// @param ctx the blend_src context.
    /// @param target the object that has the struct type.
    /// @param ty the type to validate.
    /// @param attr the IO attributes for the object.
    void CheckBlendSrcImpl(BlendSrcContext& ctx,
                           const CastableBase* target,
                           const core::type::Type* ty,
                           const IOAttributes& attr);

    /// Validates location attributes on entry point IO.
    /// @param locations the map of locations used so far for the current IO direction.
    /// @param target the object that has the location attribute.
    /// @param attr the IO attributes for the object.
    /// @param stage the pipeline stage of the entry point.
    /// @param type the type of the IO object.
    /// @param dir the IO direction (input or output).
    void CheckLocation(Hashmap<uint32_t, const CastableBase*, 4>& locations,
                       const CastableBase* target,
                       const IOAttributes& attr,
                       Function::PipelineStage stage,
                       const core::type::Type* type,
                       IODirection dir);

    /// Validates interpolation attributes on entry point IO.
    /// @param anchor where to attach error messages to.
    /// @param ty the type of the IO object
    /// @param attr the IO attributes of the object.
    /// @param stage the shader stage
    /// @param dir the direction of the IO usage
    void CheckInterpolation(const CastableBase* anchor,
                            const core::type::Type* ty,
                            const IOAttributes& attr,
                            Function::PipelineStage stage,
                            IODirection dir);

    /// Validates binding_point attributes on entry point IO.
    /// @param anchor where to attach error messages to.
    /// @param ty the type of the IO object
    /// @param attr the IO attributes of the object
    /// @param io_kind the type of shader IO object binding point is attached to
    void CheckBindingPoint(const CastableBase* anchor,
                           const core::type::Type* ty,
                           const IOAttributes& attr,
                           const ShaderIOKind& io_kind);

    /// Validates the given let
    /// @param l the let to validate
    void CheckLet(const Let* l);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates a core builtin call
    /// @param call the call to validate
    /// @param overload the call intrinsic overload
    void CheckCoreBuiltinCall(const CoreBuiltinCall* call);

    /// Validates the given member builtin call
    /// @param call the member call to validate
    void CheckMemberBuiltinCall(const MemberBuiltinCall* call);

    /// Validates the given construct
    /// @param construct the construct to validate
    void CheckConstruct(const Construct* construct);

    /// Validates the given convert
    /// @param convert the convert to validate
    void CheckConvert(const Convert* convert);

    /// Validates the given discard
    /// @note Does not validate that the discard is in a fragment shader, that
    /// needs to be handled later in the validation.
    /// @param discard the discard to validate
    void CheckDiscard(const Discard* discard);

    /// Validates the given user call
    /// @param call the call to validate
    void CheckUserCall(const UserCall* call);

    /// Validates the given access
    /// @param a the access to validate
    void CheckAccess(const Access* a);

    /// Validates the given binary
    /// @param b the binary to validate
    void CheckBinary(const Binary* b);

    /// Validates the given unary
    /// @param u the unary to validate
    void CheckUnary(const Unary* u);

    /// Validates the given if
    /// @param if_ the if to validate
    void CheckIf(const If* if_);

    /// Validates the given loop
    /// @param l the loop to validate
    void CheckLoop(const Loop* l);

    /// Validates the loop body block
    /// @param l the loop to validate
    void CheckLoopBody(const Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(const Switch* s);

    /// Validates the given swizzle
    /// @param s the swizzle to validate
    void CheckSwizzle(const Swizzle* s);

    /// Validates the given terminator
    /// @param b the terminator to validate
    void CheckTerminator(const Terminator* b);

    /// Validates the break if instruction
    /// @param b the break if to validate
    void CheckBreakIf(const BreakIf* b);

    /// Validates the continue instruction
    /// @param c the continue to validate
    void CheckContinue(const Continue* c);

    /// Validates the given exit
    /// @param e the exit to validate
    void CheckExit(const Exit* e);

    /// Validates the next iteration instruction
    /// @param n the next iteration to validate
    void CheckNextIteration(const NextIteration* n);

    /// Validates the given exit if
    /// @param e the exit if to validate
    void CheckExitIf(const ExitIf* e);

    /// Validates the given return
    /// @param r the return to validate
    void CheckReturn(const Return* r);

    /// Validates the given unreachable
    /// @param u the unreachable to validate
    void CheckUnreachable(const Unreachable* u);

    /// Validates the @p exit targets a valid @p control instruction where the instruction may jump
    /// over if control instructions.
    /// @param exit the exit to validate
    /// @param control the control instruction targeted
    void CheckControlsAllowingIf(const Exit* exit, const Instruction* control);

    /// Validates the given exit switch
    /// @param s the exit switch to validate
    void CheckExitSwitch(const ExitSwitch* s);

    /// Validates the given exit loop
    /// @param l the exit loop to validate
    void CheckExitLoop(const ExitLoop* l);

    /// Validates the given load
    /// @param l the load to validate
    void CheckLoad(const Load* l);

    /// Validates the given store
    /// @param s the store to validate
    void CheckStore(const Store* s);

    /// Validates the given load vector element
    /// @param l the load vector element to validate
    void CheckLoadVectorElement(const LoadVectorElement* l);

    /// Validates the given store vector element
    /// @param s the store vector element to validate
    void CheckStoreVectorElement(const StoreVectorElement* s);

    /// Validates the given phony assignment
    /// @param p the phony assignment to validate
    void CheckPhony(const Phony* p);

    /// Validates that the number and types of the source instruction operands match the target's
    /// values.
    /// @param source_inst the source instruction
    /// @param source_operand_offset the index of the first operand of the source instruction
    /// @param source_operand_count the number of operands of the source instruction
    /// @param target the receiver of the operand values
    /// @param target_values the receiver of the operand values
    void CheckOperandsMatchTarget(const Instruction* source_inst,
                                  size_t source_operand_offset,
                                  size_t source_operand_count,
                                  const CastableBase* target,
                                  VectorRef<const Value*> target_values);

    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the vector pointer type for the given instruction operand
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);

    /// @returns true if @p ty and its elements can be loaded
    bool CanLoad(const core::type::Type* ty);

    /// Executes all the pending tasks
    void ProcessTasks();

    /// Queues the block to be validated with ProcessTasks()
    /// @param blk the block to validate
    void QueueBlock(const Block* blk);

    /// Queues the list of instructions starting with @p inst to be validated
    /// @param inst the first instruction
    void QueueInstructions(const Instruction* inst);

    /// Begins validation of the block @p blk, and its instructions.
    /// BeginBlock() pushes a new scope for values.
    /// Must be paired with a call to EndBlock().
    void BeginBlock(const Block* blk);

    /// Ends validation of the block opened with BeginBlock() and closes the block's scope for
    /// values.
    void EndBlock();

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    const ir::Function* ContainingFunction(const ir::Instruction* inst);

    /// Get any endpoints that call a function.
    /// @param f the function
    /// @returns all end points that call the function
    Hashset<const ir::Function*, 4> ContainingEndPoints(const ir::Function* f);

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    const Module& ir_;
    diag::List& diag_;
    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()

    SymbolTable symbols_ = SymbolTable::Wrap(ir_.symbols);
    core::type::Manager type_mgr_ = core::type::Manager::Wrap(ir_.Types());
    core::ir::ReferencedModuleVars<const Module> referenced_module_vars_;

    Vector<const ControlInstruction*, 8> control_stack_;
    Vector<const Block*, 8> block_stack_;
    ScopeStack scope_stack_;

    Vector<std::function<void()>, 16> tasks_;

    Hashset<const Function*, 4> all_functions_;
    Hashset<const Instruction*, 4> visited_instructions_;
    Hashmap<const ir::Block*, const ir::Function*, 64> block_to_function_{};
    Hashmap<const ir::Function*, Hashset<const ir::UserCall*, 4>, 4> user_func_calls_;
    Hashmap<const ir::Instruction*, SupportedStages, 4> stage_restricted_instructions_;
    Hashset<const core::type::Type*, 16> validated_types_{};
    Hashmap<const core::type::Type*, uint64_t, 16> max_nest_depth_{};
};

}  // namespace tint::core::ir::validator

#endif  // SRC_TINT_LANG_CORE_IR_STRUCTURAL_VALIDATOR_H_
