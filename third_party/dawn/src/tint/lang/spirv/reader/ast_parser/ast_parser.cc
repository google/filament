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

#include "src/tint/lang/spirv/reader/ast_parser/ast_parser.h"

#include <algorithm>
#include <limits>
#include <string_view>
#include <utility>

#include "source/opt/build_module.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/row_major_attribute.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::spirv::reader::ast_parser {
namespace {

// Input SPIR-V needs only to conform to Vulkan 1.1 requirements.
// The combination of the SPIR-V reader and the semantics of WGSL
// tighten up the code so that the output of the SPIR-V *writer*
// will satisfy SPV_ENV_WEBGPU_0 validation.
const spv_target_env kInputEnv = SPV_ENV_VULKAN_1_1;

/// @param inst a SPIR-V instruction
/// @returns Returns the opcode for an instruciton
inline spv::Op opcode(const spvtools::opt::Instruction& inst) {
    return inst.opcode();
}
/// @param inst a SPIR-V instruction pointer
/// @returns Returns the opcode for an instruciton
inline spv::Op opcode(const spvtools::opt::Instruction* inst) {
    return inst->opcode();
}

// A FunctionTraverser is used to compute an ordering of functions in the
// module such that callees precede callers.
class FunctionTraverser {
  public:
    explicit FunctionTraverser(const spvtools::opt::Module& module) : module_(module) {}

    // @returns the functions in the modules such that callees precede callers.
    std::vector<const spvtools::opt::Function*> TopologicallyOrderedFunctions() {
        visited_.clear();
        ordered_.clear();
        id_to_func_.clear();
        for (const auto& f : module_) {
            id_to_func_[f.result_id()] = &f;
        }
        for (const auto& f : module_) {
            Visit(f);
        }
        return ordered_;
    }

  private:
    void Visit(const spvtools::opt::Function& f) {
        if (visited_.count(&f)) {
            return;
        }
        visited_.insert(&f);
        for (const auto& bb : f) {
            for (const auto& inst : bb) {
                if (opcode(inst) != spv::Op::OpFunctionCall) {
                    continue;
                }
                const auto* callee = id_to_func_[inst.GetSingleWordInOperand(0)];
                if (callee) {
                    Visit(*callee);
                }
            }
        }
        ordered_.push_back(&f);
    }

    const spvtools::opt::Module& module_;
    std::unordered_set<const spvtools::opt::Function*> visited_;
    std::unordered_map<uint32_t, const spvtools::opt::Function*> id_to_func_;
    std::vector<const spvtools::opt::Function*> ordered_;
};

// Returns true if the opcode operates as if its operands are signed integral.
bool AssumesSignedOperands(spv::Op opcode) {
    switch (opcode) {
        case spv::Op::OpSNegate:
        case spv::Op::OpSDiv:
        case spv::Op::OpSRem:
        case spv::Op::OpSMod:
        case spv::Op::OpSLessThan:
        case spv::Op::OpSLessThanEqual:
        case spv::Op::OpSGreaterThan:
        case spv::Op::OpSGreaterThanEqual:
        case spv::Op::OpConvertSToF:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the GLSL extended instruction expects operands to be signed.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if all operands must be signed integral type
bool AssumesSignedOperands(GLSLstd450 extended_opcode) {
    switch (extended_opcode) {
        case GLSLstd450SAbs:
        case GLSLstd450SSign:
        case GLSLstd450SMin:
        case GLSLstd450SMax:
        case GLSLstd450SClamp:
        case GLSLstd450FindSMsb:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the opcode operates as if its operands are unsigned integral.
bool AssumesUnsignedOperands(spv::Op opcode) {
    switch (opcode) {
        case spv::Op::OpUDiv:
        case spv::Op::OpUMod:
        case spv::Op::OpULessThan:
        case spv::Op::OpULessThanEqual:
        case spv::Op::OpUGreaterThan:
        case spv::Op::OpUGreaterThanEqual:
        case spv::Op::OpConvertUToF:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the GLSL extended instruction expects operands to be
// unsigned.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if all operands must be unsigned integral type
bool AssumesUnsignedOperands(GLSLstd450 extended_opcode) {
    switch (extended_opcode) {
        case GLSLstd450UMin:
        case GLSLstd450UMax:
        case GLSLstd450UClamp:
        case GLSLstd450FindUMsb:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the corresponding WGSL operation requires
// the signedness of the second operand to match the signedness of the
// first operand, and it's not one of the OpU* or OpS* instructions.
// (Those are handled via MakeOperand.)
bool AssumesSecondOperandSignednessMatchesFirstOperand(spv::Op opcode) {
    switch (opcode) {
        // All the OpI* integer binary operations.
        case spv::Op::OpIAdd:
        case spv::Op::OpISub:
        case spv::Op::OpIMul:
        case spv::Op::OpIEqual:
        case spv::Op::OpINotEqual:
        // All the bitwise integer binary operations.
        case spv::Op::OpBitwiseAnd:
        case spv::Op::OpBitwiseOr:
        case spv::Op::OpBitwiseXor:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the corresponding WGSL operation requires
// the signedness of the result to match the signedness of the first operand.
bool AssumesResultSignednessMatchesFirstOperand(spv::Op opcode) {
    switch (opcode) {
        case spv::Op::OpNot:
        case spv::Op::OpSNegate:
        case spv::Op::OpBitCount:
        case spv::Op::OpBitReverse:
        case spv::Op::OpSDiv:
        case spv::Op::OpSMod:
        case spv::Op::OpSRem:
        case spv::Op::OpIAdd:
        case spv::Op::OpISub:
        case spv::Op::OpIMul:
        case spv::Op::OpBitwiseAnd:
        case spv::Op::OpBitwiseOr:
        case spv::Op::OpBitwiseXor:
        case spv::Op::OpShiftLeftLogical:
        case spv::Op::OpShiftRightLogical:
        case spv::Op::OpShiftRightArithmetic:
            return true;
        default:
            break;
    }
    return false;
}

// Returns true if the extended instruction requires the signedness of the
// result to match the signedness of the first operand to the operation.
// @param extended_opcode GLSL.std.450 opcode
// @returns true if the result type must match the first operand type.
bool AssumesResultSignednessMatchesFirstOperand(GLSLstd450 extended_opcode) {
    switch (extended_opcode) {
        case GLSLstd450SAbs:
        case GLSLstd450SSign:
        case GLSLstd450SMin:
        case GLSLstd450SMax:
        case GLSLstd450SClamp:
        case GLSLstd450UMin:
        case GLSLstd450UMax:
        case GLSLstd450UClamp:
        case GLSLstd450FindILsb:
        case GLSLstd450FindSMsb:
        case GLSLstd450FindUMsb:
            return true;
        default:
            break;
    }
    return false;
}

// @param a SPIR-V decoration
// @return true when the given decoration is a pipeline decoration other than a
// bulitin variable.
bool IsPipelineDecoration(const Decoration& deco) {
    if (deco.size() < 1) {
        return false;
    }
    switch (static_cast<spv::Decoration>(deco[0])) {
        case spv::Decoration::Location:
        case spv::Decoration::Index:
        case spv::Decoration::Flat:
        case spv::Decoration::NoPerspective:
        case spv::Decoration::Centroid:
        case spv::Decoration::Sample:
            return true;
        default:
            break;
    }
    return false;
}

}  // namespace

TypedExpression::TypedExpression() = default;

TypedExpression::TypedExpression(const TypedExpression&) = default;

TypedExpression& TypedExpression::operator=(const TypedExpression&) = default;

TypedExpression::TypedExpression(const Type* type_in, const ast::Expression* expr_in)
    : type(type_in), expr(expr_in) {}

ASTParser::ASTParser(const std::vector<uint32_t>& spv_binary)
    : spv_binary_(spv_binary),
      fail_stream_(&success_, &errors_),
      namer_(fail_stream_),
      enum_converter_(fail_stream_),
      tools_context_(kInputEnv) {
    // Create a message consumer to propagate error messages from SPIRV-Tools
    // out as our own failures.
    message_consumer_ = [this](spv_message_level_t level, const char* /*source*/,
                               const spv_position_t& position, const char* message) {
        switch (level) {
            // Ignore info and warning message.
            case SPV_MSG_WARNING:
            case SPV_MSG_INFO:
                break;
            // Otherwise, propagate the error.
            default:
                // For binary validation errors, we only have the instruction
                // number.  It's not text, so there is no column number.
                this->Fail() << "line:" << position.index << ": " << message;
        }
    };
}

ASTParser::~ASTParser() = default;

bool ASTParser::Parse() {
    // Set up use of SPIRV-Tools utilities.
    spvtools::SpirvTools spv_tools(kInputEnv);

    // Error messages from SPIRV-Tools are forwarded as failures, including
    // setting |success_| to false.
    spv_tools.SetMessageConsumer(message_consumer_);

    if (!success_) {
        return false;
    }

    // Only consider modules valid for Vulkan 1.0.  On failure, the message
    // consumer will set the error status.
    if (!spv_tools.Validate(spv_binary_)) {
        success_ = false;
        return false;
    }
    if (!BuildInternalModule()) {
        return false;
    }
    if (!ParseInternalModule()) {
        return false;
    }

    return success_;
}

Program ASTParser::Program(bool resolve) {
    // TODO(dneto): Should we clear out spv_binary_ here, to reduce
    // memory usage?
    if (resolve) {
        return tint::resolver::Resolve(builder_);
    } else {
        return tint::Program(std::move(builder_));
    }
}

const Type* ASTParser::ConvertType(uint32_t type_id, PtrAs ptr_as) {
    if (!success_) {
        return nullptr;
    }

    if (type_mgr_ == nullptr) {
        Fail() << "ConvertType called when the internal module has not been built";
        return nullptr;
    }

    auto* spirv_type = type_mgr_->GetType(type_id);
    if (spirv_type == nullptr) {
        Fail() << "ID is not a SPIR-V type: " << type_id;
        return nullptr;
    }

    switch (spirv_type->kind()) {
        case spvtools::opt::analysis::Type::kVoid:
            return ty_.Void();
        case spvtools::opt::analysis::Type::kBool:
            return ty_.Bool();
        case spvtools::opt::analysis::Type::kInteger:
            return ConvertType(spirv_type->AsInteger());
        case spvtools::opt::analysis::Type::kFloat:
            return ConvertType(spirv_type->AsFloat());
        case spvtools::opt::analysis::Type::kVector:
            return ConvertType(spirv_type->AsVector());
        case spvtools::opt::analysis::Type::kMatrix:
            return ConvertType(spirv_type->AsMatrix());
        case spvtools::opt::analysis::Type::kRuntimeArray:
            return ConvertType(type_id, spirv_type->AsRuntimeArray());
        case spvtools::opt::analysis::Type::kArray:
            return ConvertType(type_id, spirv_type->AsArray());
        case spvtools::opt::analysis::Type::kStruct:
            return ConvertStructType(type_id);
        case spvtools::opt::analysis::Type::kPointer:
            return ConvertType(type_id, ptr_as, spirv_type->AsPointer());
        case spvtools::opt::analysis::Type::kFunction:
            // Tint doesn't have a Function type.
            // We need to convert the result type and parameter types.
            // But the SPIR-V defines those before defining the function
            // type.  No further work is required here.
            return nullptr;
        case spvtools::opt::analysis::Type::kSampler:
        case spvtools::opt::analysis::Type::kSampledImage:
        case spvtools::opt::analysis::Type::kImage:
            // Fake it for sampler and texture types.  These are handled in an
            // entirely different way.
            return ty_.Void();
        default:
            break;
    }

    Fail() << "unknown SPIR-V type with ID " << type_id << ": "
           << def_use_mgr_->GetDef(type_id)->PrettyPrint();
    return nullptr;
}

DecorationList ASTParser::GetDecorationsFor(uint32_t id) const {
    DecorationList result;
    const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
    std::unordered_set<uint32_t> visited;
    for (const auto* inst : decorations) {
        if (opcode(inst) != spv::Op::OpDecorate) {
            continue;
        }
        // Example: OpDecorate %struct_id Block
        // Example: OpDecorate %array_ty ArrayStride 16
        auto decoration_kind = inst->GetSingleWordInOperand(1);
        switch (static_cast<spv::Decoration>(decoration_kind)) {
            // Restrict and RestrictPointer have no effect in graphics APIs.
            case spv::Decoration::Restrict:
            case spv::Decoration::RestrictPointer:
                break;
            default:
                if (visited.emplace(decoration_kind).second) {
                    std::vector<uint32_t> inst_as_words;
                    inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
                    Decoration d(inst_as_words.begin() + 2, inst_as_words.end());
                    result.push_back(d);
                }
                break;
        }
    }
    return result;
}

DecorationList ASTParser::GetDecorationsForMember(uint32_t id, uint32_t member_index) const {
    DecorationList result;
    const auto& decorations = deco_mgr_->GetDecorationsFor(id, true);
    std::unordered_set<uint32_t> visited;
    for (const auto* inst : decorations) {
        // Example: OpMemberDecorate %struct_id 1 Offset 16
        if ((opcode(inst) != spv::Op::OpMemberDecorate) ||
            (inst->GetSingleWordInOperand(1) != member_index)) {
            continue;
        }
        auto decoration_kind = inst->GetSingleWordInOperand(2);
        switch (static_cast<spv::Decoration>(decoration_kind)) {
            // Restrict and RestrictPointer have no effect in graphics APIs.
            case spv::Decoration::Restrict:
            case spv::Decoration::RestrictPointer:
                break;
            default:
                if (visited.emplace(decoration_kind).second) {
                    std::vector<uint32_t> inst_as_words;
                    inst->ToBinaryWithoutAttachedDebugInsts(&inst_as_words);
                    Decoration d(inst_as_words.begin() + 3, inst_as_words.end());
                    result.push_back(d);
                }
        }
    }
    return result;
}

std::string ASTParser::ShowType(uint32_t type_id) {
    if (def_use_mgr_) {
        const auto* type_inst = def_use_mgr_->GetDef(type_id);
        if (type_inst) {
            return type_inst->PrettyPrint();
        }
    }
    return "SPIR-V type " + std::to_string(type_id);
}

Attributes ASTParser::ConvertMemberDecoration(uint32_t struct_type_id,
                                              uint32_t member_index,
                                              const Type* member_ty,
                                              const Decoration& decoration) {
    if (decoration.empty()) {
        Fail() << "malformed SPIR-V decoration: it's empty";
        return {};
    }
    Attributes out;
    switch (static_cast<spv::Decoration>(decoration[0])) {
        case spv::Decoration::Offset: {
            if (decoration.size() != 2) {
                Fail() << "malformed Offset decoration: expected 1 literal operand, has "
                       << decoration.size() - 1 << ": member " << member_index << " of "
                       << ShowType(struct_type_id);
                return {};
            }
            out.Add(builder_.MemberOffset(Source{}, AInt(decoration[1])));
            break;
        }
        case spv::Decoration::NonReadable:       // WGSL doesn't have a member decoration for this.
        case spv::Decoration::NonWritable:       // WGSL doesn't have a member decoration for this.
        case spv::Decoration::ColMajor:          // WGSL only supports column major matrices.
        case spv::Decoration::RelaxedPrecision:  // WGSL doesn't support relaxed precision.
            break;
        case spv::Decoration::RowMajor: {
            auto* ty = member_ty->UnwrapAlias();
            while (auto* arr = ty->As<Array>()) {
                ty = arr->type->UnwrapAlias();
            }
            auto* mat = ty->As<Matrix>();
            if (!mat) {
                Fail() << "MatrixStride cannot be applied to type " << ty->String();
                break;
            }
            out.Add(create<ast::RowMajorAttribute>(Source{}));
            break;
        }
        case spv::Decoration::MatrixStride: {
            if (decoration.size() != 2) {
                Fail() << "malformed MatrixStride decoration: expected 1 literal operand, has "
                       << decoration.size() - 1 << ": member " << member_index << " of "
                       << ShowType(struct_type_id);
                break;
            }
            auto* ty = member_ty->UnwrapAlias();
            while (auto* arr = ty->As<Array>()) {
                ty = arr->type->UnwrapAlias();
            }
            auto* mat = ty->As<Matrix>();
            if (!mat) {
                Fail() << "MatrixStride cannot be applied to type " << ty->String();
                break;
            }

            // Note: We do not know at this point whether the matrix is laid out as row-major or
            // column-major, and therefore do not know the "natural" stride. So we add the stride
            // attribute unconditionally, and let the DecomposeStridedMatrix transform determine if
            // anything needs to be done.

            out.Add(create<ast::StrideAttribute>(Source{}, decoration[1]));
            out.Add(builder_.ASTNodes().Create<ast::DisableValidationAttribute>(
                builder_.ID(), builder_.AllocateNodeID(),
                ast::DisabledValidation::kIgnoreStrideAttribute));
            break;
        }
        default: {
            // TODO(dneto): Support the remaining member decorations.
            Fail() << "unhandled member decoration: " << decoration[0] << " on member "
                   << member_index << " of " << ShowType(struct_type_id);
            break;
        }
    }
    return out;
}

bool ASTParser::BuildInternalModule() {
    if (!success_) {
        return false;
    }

    const spv_context& context = tools_context_.CContext();
    ir_context_ = spvtools::BuildModule(context->target_env, context->consumer, spv_binary_.data(),
                                        spv_binary_.size());
    if (!ir_context_) {
        return Fail() << "internal error: couldn't build the internal "
                         "representation of the module";
    }
    module_ = ir_context_->module();
    def_use_mgr_ = ir_context_->get_def_use_mgr();
    constant_mgr_ = ir_context_->get_constant_mgr();
    type_mgr_ = ir_context_->get_type_mgr();
    deco_mgr_ = ir_context_->get_decoration_mgr();

    topologically_ordered_functions_ = FunctionTraverser(*module_).TopologicallyOrderedFunctions();

    return success_;
}

void ASTParser::ResetInternalModule() {
    ir_context_.reset(nullptr);
    module_ = nullptr;
    def_use_mgr_ = nullptr;
    constant_mgr_ = nullptr;
    type_mgr_ = nullptr;
    deco_mgr_ = nullptr;

    glsl_std_450_imports_.clear();
    enabled_extensions_.Clear();
}

bool ASTParser::ParseInternalModule() {
    if (!success_) {
        return false;
    }
    RegisterLineNumbers();
    if (!ParseInternalModuleExceptFunctions()) {
        return false;
    }
    if (!EmitFunctions()) {
        return false;
    }
    return success_;
}

void ASTParser::RegisterLineNumbers() {
    Source::Location instruction_number{};

    // Has there been an OpLine since the last OpNoLine or start of the module?
    bool in_op_line_scope = false;
    // The source location provided by the most recent OpLine instruction.
    Source::Location op_line_source{};
    const bool run_on_debug_insts = true;
    module_->ForEachInst(
        [this, &in_op_line_scope, &op_line_source,
         &instruction_number](const spvtools::opt::Instruction* inst) {
            ++instruction_number.line;
            switch (opcode(inst)) {
                case spv::Op::OpLine:
                    in_op_line_scope = true;
                    // TODO(dneto): This ignores the File ID (operand 0), since the Tint
                    // Source concept doesn't represent that.
                    op_line_source.line = inst->GetSingleWordInOperand(1);
                    op_line_source.column = inst->GetSingleWordInOperand(2);
                    break;
                case spv::Op::OpNoLine:
                    in_op_line_scope = false;
                    break;
                default:
                    break;
            }
            this->inst_source_[inst] = in_op_line_scope ? op_line_source : instruction_number;
        },
        run_on_debug_insts);
}

Source ASTParser::GetSourceForResultIdForTest(uint32_t id) const {
    return GetSourceForInst(def_use_mgr_->GetDef(id));
}

Source ASTParser::GetSourceForInst(const spvtools::opt::Instruction* inst) const {
    auto where = inst_source_.find(inst);
    if (where == inst_source_.end()) {
        return {};
    }
    return Source{where->second};
}

bool ASTParser::ParseInternalModuleExceptFunctions() {
    if (!success_) {
        return false;
    }
    if (!RegisterExtendedInstructionImports()) {
        return false;
    }
    if (!RegisterUserAndStructMemberNames()) {
        return false;
    }
    if (!RegisterWorkgroupSizeBuiltin()) {
        return false;
    }
    if (!RegisterEntryPoints()) {
        return false;
    }
    if (!RegisterHandleUsage()) {
        return false;
    }
    if (!RegisterTypes()) {
        return false;
    }
    if (!RejectInvalidPointerRoots()) {
        return false;
    }
    if (!EmitScalarSpecConstants()) {
        return false;
    }
    if (!EmitModuleScopeVariables()) {
        return false;
    }
    return success_;
}

bool ASTParser::RegisterExtendedInstructionImports() {
    for (const spvtools::opt::Instruction& import : module_->ext_inst_imports()) {
        std::string name(reinterpret_cast<const char*>(import.GetInOperand(0).words.data()));
        // TODO(dneto): Handle other extended instruction sets when needed.
        if (name == "GLSL.std.450") {
            glsl_std_450_imports_.insert(import.result_id());
        } else if (name.find("NonSemantic.") == 0) {
            ignored_imports_.insert(import.result_id());
        } else {
            return Fail() << "Unrecognized extended instruction set: " << name;
        }
    }
    return true;
}

bool ASTParser::IsGlslExtendedInstruction(const spvtools::opt::Instruction& inst) const {
    return (opcode(inst) == spv::Op::OpExtInst) &&
           (glsl_std_450_imports_.count(inst.GetSingleWordInOperand(0)) > 0);
}

bool ASTParser::IsIgnoredExtendedInstruction(const spvtools::opt::Instruction& inst) const {
    return (opcode(inst) == spv::Op::OpExtInst) &&
           (ignored_imports_.count(inst.GetSingleWordInOperand(0)) > 0);
}

bool ASTParser::RegisterUserAndStructMemberNames() {
    if (!success_) {
        return false;
    }
    // Register entry point names. An entry point name is the point of contact
    // between the API and the shader. It has the highest priority for
    // preservation, so register it first.
    for (const spvtools::opt::Instruction& entry_point : module_->entry_points()) {
        const uint32_t function_id = entry_point.GetSingleWordInOperand(1);
        const std::string name = entry_point.GetInOperand(2).AsString();

        // This translator requires the entry point to be a valid WGSL identifier.
        // Allowing otherwise leads to difficulties in that the programmer needs
        // to get a mapping from their original entry point name to the WGSL name,
        // and we don't have a good mechanism for that.
        if (!IsValidIdentifier(name)) {
            return Fail() << "entry point name is not a valid WGSL identifier: " << name;
        }

        // SPIR-V allows a single function to be the implementation for more
        // than one entry point.  In the common case, it's one-to-one, and we should
        // try to name the function after the entry point.  Otherwise, give the
        // function a name automatically derived from the entry point name.
        namer_.SuggestSanitizedName(function_id, name);

        // There is another many-to-one relationship to take care of:  In SPIR-V
        // the same name can be used for multiple entry points, provided they are
        // for different shader stages. Take action now to ensure we can use the
        // entry point name later on, and not have it taken for another identifier
        // by an accidental collision with a derived name made for a different ID.
        if (!namer_.IsRegistered(name)) {
            // The entry point name is "unoccupied" becase an earlier entry point
            // grabbed the slot for the function that implements both entry points.
            // Register this new entry point's name, to avoid accidental collisions
            // with a future generated ID.
            if (!namer_.RegisterWithoutId(name)) {
                return false;
            }
        }
    }

    // Register names from OpName and OpMemberName
    for (const auto& inst : module_->debugs2()) {
        switch (opcode(inst)) {
            case spv::Op::OpName: {
                const auto name = inst.GetInOperand(1).AsString();
                if (!name.empty()) {
                    namer_.SuggestSanitizedName(inst.GetSingleWordInOperand(0), name);
                }
                break;
            }
            case spv::Op::OpMemberName: {
                const auto name = inst.GetInOperand(2).AsString();
                if (!name.empty()) {
                    namer_.SuggestSanitizedMemberName(inst.GetSingleWordInOperand(0),
                                                      inst.GetSingleWordInOperand(1), name);
                }
                break;
            }
            default:
                break;
        }
    }

    // Fill in struct member names, and disambiguate them.
    for (const auto* type_inst : module_->GetTypes()) {
        if (opcode(type_inst) == spv::Op::OpTypeStruct) {
            namer_.ResolveMemberNamesForStruct(type_inst->result_id(), type_inst->NumInOperands());
        }
    }

    return true;
}

bool ASTParser::IsValidIdentifier(std::string_view str) {
    if (str.empty()) {
        return false;
    }
    if (str[0] == '_') {
        if (str.length() == 1u || str[1] == '_') {
            // https://www.w3.org/TR/WGSL/#identifiers
            // must not be '_' (a single underscore)
            // must not start with two underscores
            return false;
        }
    }

    // Must begin with an XID_Source unicode character, or underscore
    {
        auto* utf8 = reinterpret_cast<const uint8_t*>(str.data());
        auto [code_point, n] = tint::utf8::Decode(utf8, str.size());
        if (code_point != tint::CodePoint('_') && !code_point.IsXIDStart()) {
            return false;
        }
        str = str.substr(n);
    }

    // Must continue with an XID_Continue unicode character
    while (!str.empty()) {
        auto* utf8 = reinterpret_cast<const uint8_t*>(str.data());
        auto [code_point, n] = tint::utf8::Decode(utf8, str.size());
        if (!code_point.IsXIDContinue()) {
            return false;
        }
        str = str.substr(n);
    }

    return true;
}

bool ASTParser::RegisterWorkgroupSizeBuiltin() {
    WorkgroupSizeInfo& info = workgroup_size_builtin_;
    for (const spvtools::opt::Instruction& inst : module_->annotations()) {
        if (opcode(inst) != spv::Op::OpDecorate) {
            continue;
        }
        if (inst.GetSingleWordInOperand(1) != uint32_t(spv::Decoration::BuiltIn)) {
            continue;
        }
        if (inst.GetSingleWordInOperand(2) != uint32_t(spv::BuiltIn::WorkgroupSize)) {
            continue;
        }
        info.id = inst.GetSingleWordInOperand(0);
    }
    if (info.id == 0) {
        return true;
    }
    // Gather the values.
    const spvtools::opt::Instruction* composite_def = def_use_mgr_->GetDef(info.id);
    if (!composite_def) {
        return Fail() << "Invalid WorkgroupSize builtin value";
    }
    // SPIR-V validation checks that the result is a 3-element vector of 32-bit
    // integer scalars (signed or unsigned).  Rely on validation to check the
    // type.  In theory the instruction could be OpConstantNull and still
    // pass validation, but that would be non-sensical.  Be a little more
    // stringent here and check for specific opcodes.  WGSL does not support
    // const-expr yet, so avoid supporting OpSpecConstantOp here.
    // TODO(dneto): See https://github.com/gpuweb/gpuweb/issues/1272 for WGSL
    // const_expr proposals.
    if ((opcode(composite_def) != spv::Op::OpSpecConstantComposite &&
         opcode(composite_def) != spv::Op::OpConstantComposite)) {
        return Fail() << "Invalid WorkgroupSize builtin.  Expected 3-element "
                         "OpSpecConstantComposite or OpConstantComposite:  "
                      << composite_def->PrettyPrint();
    }
    info.type_id = composite_def->type_id();
    // Extract the component type from the vector type.
    info.component_type_id = def_use_mgr_->GetDef(info.type_id)->GetSingleWordInOperand(0);

    /// Sets the ID and value of the index'th member of the composite constant.
    /// Returns false and emits a diagnostic on error.
    auto set_param = [this, composite_def](uint32_t* id_ptr, uint32_t* value_ptr,
                                           int index) -> bool {
        const auto id = composite_def->GetSingleWordInOperand(static_cast<uint32_t>(index));
        const auto* def = def_use_mgr_->GetDef(id);
        if (!def ||
            (opcode(def) != spv::Op::OpSpecConstant && opcode(def) != spv::Op::OpConstant) ||
            (def->NumInOperands() != 1)) {
            return Fail() << "invalid component " << index << " of workgroupsize "
                          << (def ? def->PrettyPrint() : std::string("no definition"));
        }
        *id_ptr = id;
        // Use the default value of a spec constant.
        *value_ptr = def->GetSingleWordInOperand(0);
        return true;
    };

    return set_param(&info.x_id, &info.x_value, 0) && set_param(&info.y_id, &info.y_value, 1) &&
           set_param(&info.z_id, &info.z_value, 2);
}

bool ASTParser::RegisterEntryPoints() {
    // Mapping from entry point ID to GridSize computed from LocalSize
    // decorations.
    std::unordered_map<uint32_t, GridSize> local_size;
    for (const spvtools::opt::Instruction& inst : module_->execution_modes()) {
        auto mode = static_cast<spv::ExecutionMode>(inst.GetSingleWordInOperand(1));
        if (mode == spv::ExecutionMode::LocalSize) {
            if (inst.NumInOperands() != 5) {
                // This won't even get past SPIR-V binary parsing.
                return Fail() << "invalid LocalSize execution mode: " << inst.PrettyPrint();
            }
            uint32_t function_id = inst.GetSingleWordInOperand(0);
            local_size[function_id] =
                GridSize{inst.GetSingleWordInOperand(2), inst.GetSingleWordInOperand(3),
                         inst.GetSingleWordInOperand(4)};
        }
    }

    for (const spvtools::opt::Instruction& entry_point : module_->entry_points()) {
        const auto stage = spv::ExecutionModel(entry_point.GetSingleWordInOperand(0));
        const uint32_t function_id = entry_point.GetSingleWordInOperand(1);

        const std::string ep_name = entry_point.GetOperand(2).AsString();
        if (!IsValidIdentifier(ep_name)) {
            return Fail() << "entry point name is not a valid WGSL identifier: " << ep_name;
        }

        bool owns_inner_implementation = false;
        std::string inner_implementation_name;

        auto where = function_to_ep_info_.find(function_id);
        if (where == function_to_ep_info_.end()) {
            // If this is the first entry point to have function_id as its
            // implementation, then this entry point is responsible for generating
            // the inner implementation.
            owns_inner_implementation = true;
            inner_implementation_name = namer_.MakeDerivedName(ep_name);
        } else {
            // Reuse the inner implementation owned by the first entry point.
            inner_implementation_name = where->second[0].inner_name;
        }
        TINT_ASSERT(!inner_implementation_name.empty());
        TINT_ASSERT(ep_name != inner_implementation_name);

        UniqueVector<uint32_t, 8> inputs;
        UniqueVector<uint32_t, 8> outputs;
        for (unsigned iarg = 3; iarg < entry_point.NumInOperands(); iarg++) {
            const uint32_t var_id = entry_point.GetSingleWordInOperand(iarg);
            if (const auto* var_inst = def_use_mgr_->GetDef(var_id)) {
                switch (spv::StorageClass(var_inst->GetSingleWordInOperand(0))) {
                    case spv::StorageClass::Input:
                        inputs.Add(var_id);
                        break;
                    case spv::StorageClass::Output:
                        outputs.Add(var_id);
                        break;
                    default:
                        break;
                }
            }
        }
        // Save the lists, in ID-sorted order.
        tint::Vector<uint32_t, 8> sorted_inputs(inputs);
        std::sort(sorted_inputs.begin(), sorted_inputs.end());
        tint::Vector<uint32_t, 8> sorted_outputs(outputs);
        std::sort(sorted_outputs.begin(), sorted_outputs.end());

        const auto ast_stage = enum_converter_.ToPipelineStage(stage);
        GridSize wgsize;
        if (ast_stage == ast::PipelineStage::kCompute) {
            if (workgroup_size_builtin_.id) {
                // Store the default values.
                // WGSL allows specializing these, but this code doesn't support that
                // yet. https://github.com/gpuweb/gpuweb/issues/1442
                wgsize = GridSize{workgroup_size_builtin_.x_value, workgroup_size_builtin_.y_value,
                                  workgroup_size_builtin_.z_value};
            } else {
                // Use the LocalSize execution mode.  This is the second choice.
                auto where_local_size = local_size.find(function_id);
                if (where_local_size != local_size.end()) {
                    wgsize = where_local_size->second;
                }
            }
        }
        function_to_ep_info_[function_id].emplace_back(
            ep_name, ast_stage, owns_inner_implementation, inner_implementation_name,
            std::move(sorted_inputs), std::move(sorted_outputs), wgsize);
    }

    // The enum conversion could have failed, so return the existing status value.
    return success_;
}

const Type* ASTParser::ConvertType(const spvtools::opt::analysis::Integer* int_ty) {
    if (int_ty->width() == 32) {
        return int_ty->IsSigned() ? static_cast<const Type*>(ty_.I32())
                                  : static_cast<const Type*>(ty_.U32());
    }
    Fail() << "unhandled integer width: " << int_ty->width();
    return nullptr;
}

const Type* ASTParser::ConvertType(const spvtools::opt::analysis::Float* float_ty) {
    if (float_ty->width() == 32) {
        return ty_.F32();
    }
    if (float_ty->width() == 16) {
        Enable(wgsl::Extension::kF16);
        return ty_.F16();
    }
    Fail() << "unhandled float width: " << float_ty->width();
    return nullptr;
}

const Type* ASTParser::ConvertType(const spvtools::opt::analysis::Vector* vec_ty) {
    const auto num_elem = vec_ty->element_count();
    auto* ast_elem_ty = ConvertType(type_mgr_->GetId(vec_ty->element_type()));
    if (ast_elem_ty == nullptr) {
        return ast_elem_ty;
    }
    return ty_.Vector(ast_elem_ty, num_elem);
}

const Type* ASTParser::ConvertType(const spvtools::opt::analysis::Matrix* mat_ty) {
    const auto* vec_ty = mat_ty->element_type()->AsVector();
    const auto* scalar_ty = vec_ty->element_type();
    const auto num_rows = vec_ty->element_count();
    const auto num_columns = mat_ty->element_count();
    auto* ast_scalar_ty = ConvertType(type_mgr_->GetId(scalar_ty));
    if (ast_scalar_ty == nullptr) {
        return nullptr;
    }
    return ty_.Matrix(ast_scalar_ty, num_columns, num_rows);
}

const Type* ASTParser::ConvertType(uint32_t type_id,
                                   const spvtools::opt::analysis::RuntimeArray* rtarr_ty) {
    auto* ast_elem_ty = ConvertType(type_mgr_->GetId(rtarr_ty->element_type()));
    if (ast_elem_ty == nullptr) {
        return nullptr;
    }
    uint32_t array_stride = 0;
    if (!ParseArrayDecorations(rtarr_ty, &array_stride)) {
        return nullptr;
    }
    const Type* result = ty_.Array(ast_elem_ty, 0, array_stride);
    return MaybeGenerateAlias(type_id, rtarr_ty, result);
}

const Type* ASTParser::ConvertType(uint32_t type_id, const spvtools::opt::analysis::Array* arr_ty) {
    // Get the element type. The SPIR-V optimizer's types representation
    // deduplicates array types that have the same parameterization.
    // We don't want that deduplication, so get the element type from
    // the SPIR-V type directly.
    const auto* inst = def_use_mgr_->GetDef(type_id);
    const auto elem_type_id = inst->GetSingleWordInOperand(0);
    auto* ast_elem_ty = ConvertType(elem_type_id);
    if (ast_elem_ty == nullptr) {
        return nullptr;
    }
    // Get the length.
    const auto& length_info = arr_ty->length_info();
    if (length_info.words.empty()) {
        // The internal representation is invalid. The discriminant vector
        // is mal-formed.
        Fail() << "internal error: Array length info is invalid";
        return nullptr;
    }
    if (length_info.words[0] != spvtools::opt::analysis::Array::LengthInfo::kConstant) {
        Fail() << "Array type " << type_mgr_->GetId(arr_ty)
               << " length is a specialization constant";
        return nullptr;
    }
    const auto* constant = constant_mgr_->FindDeclaredConstant(length_info.id);
    if (constant == nullptr) {
        Fail() << "Array type " << type_mgr_->GetId(arr_ty) << " length ID " << length_info.id
               << " does not name an OpConstant";
        return nullptr;
    }
    const uint64_t num_elem = constant->GetZeroExtendedValue();
    // For now, limit to only 32bits.
    if (num_elem > std::numeric_limits<uint32_t>::max()) {
        Fail() << "Array type " << type_mgr_->GetId(arr_ty)
               << " has too many elements (more than can fit in 32 bits): " << num_elem;
        return nullptr;
    }
    uint32_t array_stride = 0;
    if (!ParseArrayDecorations(arr_ty, &array_stride)) {
        return nullptr;
    }
    if (remap_buffer_block_type_.count(elem_type_id)) {
        remap_buffer_block_type_.insert(type_mgr_->GetId(arr_ty));
    }
    const Type* result = ty_.Array(ast_elem_ty, static_cast<uint32_t>(num_elem), array_stride);
    return MaybeGenerateAlias(type_id, arr_ty, result);
}

bool ASTParser::ParseArrayDecorations(const spvtools::opt::analysis::Type* spv_type,
                                      uint32_t* array_stride) {
    *array_stride = 0;  // Implicit stride case.
    const auto type_id = type_mgr_->GetId(spv_type);
    for (auto& decoration : this->GetDecorationsFor(type_id)) {
        if (decoration.size() == 2 && decoration[0] == uint32_t(spv::Decoration::ArrayStride)) {
            const auto stride = decoration[1];
            if (stride == 0) {
                return Fail() << "invalid array type ID " << type_id << ": ArrayStride can't be 0";
            }
            *array_stride = stride;
        } else {
            return Fail() << "invalid array type ID " << type_id << ": unknown decoration "
                          << (decoration.empty() ? "(empty)" : std::to_string(decoration[0]))
                          << " with " << decoration.size() << " total words";
        }
    }
    return true;
}

const Type* ASTParser::ConvertStructType(uint32_t type_id) {
    // Compute the struct decoration.
    auto struct_decorations = this->GetDecorationsFor(type_id);
    if (struct_decorations.size() == 1) {
        const auto decoration = struct_decorations[0][0];
        if (decoration == uint32_t(spv::Decoration::BufferBlock)) {
            remap_buffer_block_type_.insert(type_id);
        } else if (decoration != uint32_t(spv::Decoration::Block)) {
            Fail() << "struct with ID " << type_id
                   << " has unrecognized decoration: " << int(decoration);
        }
    } else if (struct_decorations.size() > 1) {
        Fail() << "can't handle a struct with more than one decoration: struct " << type_id
               << " has " << struct_decorations.size();
        return nullptr;
    }

    // The SPIR-V optimizer's types representation deduplicates types. We don't want that
    // deduplication, so get the member types from the SPIR-V instruction directly.
    const auto* inst = def_use_mgr_->GetDef(type_id);
    auto num_members = inst->NumOperands() - 1;
    if (num_members == 0) {
        Fail() << "WGSL does not support empty structures. can't convert type: "
               << def_use_mgr_->GetDef(type_id)->PrettyPrint();
        return nullptr;
    }

    // Compute members
    tint::Vector<const ast::StructMember*, 8> ast_members;
    TypeList ast_member_types;
    unsigned num_non_writable_members = 0;
    for (uint32_t member_index = 0; member_index < num_members; ++member_index) {
        const auto member_type_id = inst->GetOperand(member_index + 1).AsId();
        auto* ast_member_ty = ConvertType(member_type_id);
        if (ast_member_ty == nullptr) {
            // Already emitted diagnostics.
            return nullptr;
        }

        ast_member_types.emplace_back(ast_member_ty);

        // Scan member for built-in decorations. Some vertex built-ins are handled
        // specially, and should not generate a structure member.
        bool create_ast_member = true;
        for (auto& decoration : GetDecorationsForMember(type_id, member_index)) {
            if (decoration.empty()) {
                Fail() << "malformed SPIR-V decoration: it's empty";
                return nullptr;
            }
            if ((decoration[0] == uint32_t(spv::Decoration::BuiltIn)) && (decoration.size() > 1)) {
                switch (static_cast<spv::BuiltIn>(decoration[1])) {
                    case spv::BuiltIn::Position:
                        // Record this built-in variable specially.
                        builtin_position_.struct_type_id = type_id;
                        builtin_position_.position_member_index = member_index;
                        builtin_position_.position_member_type_id = member_type_id;
                        create_ast_member = false;  // Not part of the WGSL structure.
                        break;
                    case spv::BuiltIn::PointSize:  // not supported in WGSL, but ignore
                        builtin_position_.pointsize_member_index = member_index;
                        create_ast_member = false;  // Not part of the WGSL structure.
                        break;
                    case spv::BuiltIn::ClipDistance:
                    case spv::BuiltIn::CullDistance:  // not supported in WGSL
                        create_ast_member = false;    // Not part of the WGSL structure.
                        break;
                    default:
                        Fail() << "unrecognized builtin " << decoration[1];
                        return nullptr;
                }
            }
        }
        if (!create_ast_member) {
            // This member is decorated as a built-in, and is handled specially.
            continue;
        }

        bool is_non_writable = false;
        Attributes ast_member_decorations;
        for (auto& decoration : GetDecorationsForMember(type_id, member_index)) {
            if (IsPipelineDecoration(decoration)) {
                // IO decorations are handled when emitting the entry point.
                continue;
            } else if (decoration[0] == uint32_t(spv::Decoration::NonWritable)) {
                // WGSL doesn't represent individual members as non-writable. Instead,
                // apply the ReadOnly access control to the containing struct if all
                // the members are non-writable.
                is_non_writable = true;
            } else {
                auto attrs =
                    ConvertMemberDecoration(type_id, member_index, ast_member_ty, decoration);
                ast_member_decorations.Add(attrs);
                if (!success_) {
                    return nullptr;
                }
            }
        }

        if (is_non_writable) {
            // Count a member as non-writable only once, no matter how many
            // NonWritable decorations are applied to it.
            ++num_non_writable_members;
        }
        const auto member_name = namer_.GetMemberName(type_id, member_index);
        auto* ast_struct_member =
            builder_.Member(Source{}, member_name, ast_member_ty->Build(builder_),
                            std::move(ast_member_decorations.list));
        ast_members.Push(ast_struct_member);
    }

    if (ast_members.IsEmpty()) {
        // All members were likely built-ins. Don't generate an empty AST structure.
        return nullptr;
    }

    namer_.SuggestSanitizedName(type_id, "S");

    auto name = namer_.GetName(type_id);

    // Now make the struct.
    auto sym = builder_.Symbols().Register(name);
    auto* ast_struct =
        create<ast::Struct>(Source{}, builder_.Ident(sym), std::move(ast_members), tint::Empty);
    if (num_non_writable_members == num_members) {
        read_only_struct_types_.insert(ast_struct->name->symbol);
    }
    AddTypeDecl(sym, ast_struct);
    const auto* result = ty_.Struct(sym, std::move(ast_member_types));
    struct_id_for_symbol_[sym] = type_id;
    return result;
}

void ASTParser::AddTypeDecl(Symbol name, const ast::TypeDecl* decl) {
    auto iter = declared_types_.insert(name);
    if (iter.second) {
        builder_.AST().AddTypeDecl(decl);
    }
}

const Type* ASTParser::ConvertType(uint32_t type_id,
                                   PtrAs ptr_as,
                                   const spvtools::opt::analysis::Pointer*) {
    const auto* inst = def_use_mgr_->GetDef(type_id);
    const auto pointee_type_id = inst->GetSingleWordInOperand(1);
    const auto storage_class = spv::StorageClass(inst->GetSingleWordInOperand(0));

    if (pointee_type_id == builtin_position_.struct_type_id) {
        builtin_position_.pointer_type_id = type_id;
        // Pipeline IO builtins map to private variables.
        builtin_position_.storage_class = spv::StorageClass::Private;
        return nullptr;
    }
    auto* ast_elem_ty = ConvertType(pointee_type_id, PtrAs::Ptr);
    if (ast_elem_ty == nullptr) {
        Fail() << "SPIR-V pointer type with ID " << type_id << " has invalid pointee type "
               << pointee_type_id;
        return nullptr;
    }

    auto ast_address_space = enum_converter_.ToAddressSpace(storage_class);
    if (ast_address_space == core::AddressSpace::kUniform &&
        remap_buffer_block_type_.count(pointee_type_id)) {
        ast_address_space = core::AddressSpace::kStorage;
        remap_buffer_block_type_.insert(type_id);
    }

    // Pipeline input and output variables map to private variables.
    if (ast_address_space == core::AddressSpace::kIn ||
        ast_address_space == core::AddressSpace::kOut) {
        ast_address_space = core::AddressSpace::kPrivate;
    }
    switch (ptr_as) {
        case PtrAs::Ref:
            return ty_.Reference(ast_address_space, ast_elem_ty);
        case PtrAs::Ptr:
            return ty_.Pointer(ast_address_space, ast_elem_ty);
    }
    Fail() << "invalid value for ptr_as: " << static_cast<int>(ptr_as);
    return nullptr;
}

bool ASTParser::RegisterTypes() {
    if (!success_) {
        return false;
    }

    // First record the structure types that should have a `block` decoration
    // in WGSL. In particular, exclude user-defined pipeline IO in a
    // block-decorated struct.
    for (const auto& type_or_value : module_->types_values()) {
        if (opcode(type_or_value) != spv::Op::OpVariable) {
            continue;
        }
        const auto& var = type_or_value;
        const auto spirv_storage_class = spv::StorageClass(var.GetSingleWordInOperand(0));
        if ((spirv_storage_class != spv::StorageClass::StorageBuffer) &&
            (spirv_storage_class != spv::StorageClass::Uniform)) {
            continue;
        }
        const auto* ptr_type = def_use_mgr_->GetDef(var.type_id());
        if (opcode(ptr_type) != spv::Op::OpTypePointer) {
            return Fail() << "OpVariable type expected to be a pointer: " << var.PrettyPrint();
        }
        const auto* store_type = def_use_mgr_->GetDef(ptr_type->GetSingleWordInOperand(1));
        if (opcode(store_type) == spv::Op::OpTypeStruct) {
            struct_types_for_buffers_.insert(store_type->result_id());
        } else {
            Fail() << "WGSL does not support arrays of buffers: " << var.PrettyPrint();
        }
    }

    // Now convert each type.
    for (auto& type_or_const : module_->types_values()) {
        const auto* type = type_mgr_->GetType(type_or_const.result_id());
        if (type == nullptr) {
            continue;
        }
        ConvertType(type_or_const.result_id());
    }
    // Manufacture a type for the gl_Position variable if we have to.
    if ((builtin_position_.struct_type_id != 0) &&
        (builtin_position_.position_member_pointer_type_id == 0)) {
        builtin_position_.position_member_pointer_type_id = type_mgr_->FindPointerToType(
            builtin_position_.position_member_type_id, builtin_position_.storage_class);
        ConvertType(builtin_position_.position_member_pointer_type_id);
    }
    return success_;
}

bool ASTParser::RejectInvalidPointerRoots() {
    if (!success_) {
        return false;
    }
    for (auto& inst : module_->types_values()) {
        if (const auto* result_type = type_mgr_->GetType(inst.type_id())) {
            if (result_type->AsPointer()) {
                switch (opcode(inst)) {
                    case spv::Op::OpVariable:
                        // This is the only valid case.
                        break;
                    case spv::Op::OpUndef:
                        return Fail() << "undef pointer is not valid: " << inst.PrettyPrint();
                    case spv::Op::OpConstantNull:
                        return Fail() << "null pointer is not valid: " << inst.PrettyPrint();
                    default:
                        return Fail()
                               << "module-scope pointer is not valid: " << inst.PrettyPrint();
                }
            }
        }
    }
    return success();
}

bool ASTParser::EmitScalarSpecConstants() {
    if (!success_) {
        return false;
    }
    // Generate a module-scope const declaration for each instruction
    // that is OpSpecConstantTrue, OpSpecConstantFalse, or OpSpecConstant.
    for (auto& inst : module_->types_values()) {
        // These will be populated for a valid scalar spec constant.
        const Type* ast_type = nullptr;
        ast::LiteralExpression* ast_expr = nullptr;

        switch (opcode(inst)) {
            case spv::Op::OpSpecConstantTrue:
            case spv::Op::OpSpecConstantFalse: {
                ast_type = ConvertType(inst.type_id());
                ast_expr = create<ast::BoolLiteralExpression>(
                    Source{}, opcode(inst) == spv::Op::OpSpecConstantTrue);
                break;
            }
            case spv::Op::OpSpecConstant: {
                ast_type = ConvertType(inst.type_id());
                const uint32_t literal_value = inst.GetSingleWordInOperand(0);
                ast_expr = Switch(
                    ast_type,  //
                    [&](const I32*) {
                        return create<ast::IntLiteralExpression>(
                            Source{}, static_cast<int64_t>(literal_value),
                            ast::IntLiteralExpression::Suffix::kI);
                    },
                    [&](const U32*) {
                        return create<ast::IntLiteralExpression>(
                            Source{}, static_cast<int64_t>(literal_value),
                            ast::IntLiteralExpression::Suffix::kU);
                    },
                    [&](const F32*) {
                        float float_value;
                        // Copy the bits so we can read them as a float.
                        std::memcpy(&float_value, &literal_value, sizeof(float_value));
                        return create<ast::FloatLiteralExpression>(
                            Source{}, static_cast<double>(float_value),
                            ast::FloatLiteralExpression::Suffix::kF);
                    });
                if (ast_expr == nullptr) {
                    return Fail() << " invalid result type for OpSpecConstant "
                                  << inst.PrettyPrint();
                }
                break;
            }
            default:
                break;
        }
        if (ast_type && ast_expr) {
            Attributes spec_id_attrs;
            for (const auto& deco : GetDecorationsFor(inst.result_id())) {
                if ((deco.size() == 2) && (deco[0] == uint32_t(spv::Decoration::SpecId))) {
                    const uint32_t id = deco[1];
                    if (id > 65535) {
                        return Fail() << "SpecId too large. WGSL override IDs must be "
                                         "between 0 and 65535: ID %"
                                      << inst.result_id() << " has SpecId " << id;
                    }
                    auto* cid = builder_.Id(Source{}, AInt(id));
                    spec_id_attrs.Add(cid);
                    break;
                }
            }
            auto* ast_var =
                MakeOverride(inst.result_id(), ast_type, ast_expr, std::move(spec_id_attrs));
            if (ast_var) {
                scalar_spec_constants_.insert(inst.result_id());
            }
        }
    }
    return success_;
}

const Type* ASTParser::MaybeGenerateAlias(uint32_t type_id,
                                          const spvtools::opt::analysis::Type* type,
                                          const Type* ast_type) {
    if (!success_) {
        return nullptr;
    }

    // We only care about arrays, and runtime arrays.
    switch (type->kind()) {
        case spvtools::opt::analysis::Type::kRuntimeArray:
            // Runtime arrays are always decorated with ArrayStride so always get a
            // type alias.
            namer_.SuggestSanitizedName(type_id, "RTArr");
            break;
        case spvtools::opt::analysis::Type::kArray:
            // Only make a type aliase for arrays with decorations.
            if (GetDecorationsFor(type_id).empty()) {
                return ast_type;
            }
            namer_.SuggestSanitizedName(type_id, "Arr");
            break;
        default:
            // Ignore constants, and any other types.
            return ast_type;
    }
    auto* ast_underlying_type = ast_type;
    if (ast_underlying_type == nullptr) {
        Fail() << "internal error: no type registered for SPIR-V ID: " << type_id;
        return nullptr;
    }
    const auto name = namer_.GetName(type_id);
    const auto sym = builder_.Symbols().Register(name);
    auto* ast_alias_type = builder_.ty.alias(sym, ast_underlying_type->Build(builder_));

    // Record this new alias as the AST type for this SPIR-V ID.
    AddTypeDecl(sym, ast_alias_type);

    return ty_.Alias(sym, ast_underlying_type);
}

bool ASTParser::EmitModuleScopeVariables() {
    if (!success_) {
        return false;
    }
    for (const auto& type_or_value : module_->types_values()) {
        if (opcode(type_or_value) != spv::Op::OpVariable) {
            continue;
        }
        const auto& var = type_or_value;
        const auto spirv_storage_class = spv::StorageClass(var.GetSingleWordInOperand(0));

        uint32_t type_id = var.type_id();
        if ((type_id == builtin_position_.pointer_type_id) &&
            ((spirv_storage_class == spv::StorageClass::Input) ||
             (spirv_storage_class == spv::StorageClass::Output))) {
            // TODO(crbug.com/tint/103): Support modules that contain multiple Position built-ins.
            if (builtin_position_.per_vertex_var_id != 0) {
                return Fail()
                       << "unsupported: multiple Position built-in variables in the same module";
            }

            // Skip emitting gl_PerVertex.
            builtin_position_.per_vertex_var_id = var.result_id();
            builtin_position_.per_vertex_var_init_id =
                var.NumInOperands() > 1 ? var.GetSingleWordInOperand(1) : 0u;
            continue;
        }
        switch (enum_converter_.ToAddressSpace(spirv_storage_class)) {
            case core::AddressSpace::kUndefined:
            case core::AddressSpace::kIn:
            case core::AddressSpace::kOut:
            case core::AddressSpace::kUniform:
            case core::AddressSpace::kHandle:
            case core::AddressSpace::kStorage:
            case core::AddressSpace::kWorkgroup:
            case core::AddressSpace::kPrivate:
                break;
            default:
                return Fail() << "invalid SPIR-V storage class " << int(spirv_storage_class)
                              << " for module scope variable: " << var.PrettyPrint();
        }
        if (!success_) {
            return false;
        }
        const Type* ast_store_type = nullptr;
        core::AddressSpace ast_address_space = core::AddressSpace::kUndefined;
        if (spirv_storage_class == spv::StorageClass::UniformConstant) {
            // These are opaque handles: samplers or textures
            ast_store_type = GetHandleTypeForSpirvHandle(var);
            if (!ast_store_type) {
                return false;
            }
            // ast_storage_class should remain kNone because handle variables
            // are never declared with an explicit address space.
        } else {
            const Type* ast_type = ConvertType(type_id);
            if (ast_type == nullptr) {
                return Fail() << "internal error: failed to register Tint AST type for "
                                 "SPIR-V type with ID: "
                              << var.type_id();
            }
            if (auto* ast_ptr_type = ast_type->As<Pointer>()) {
                ast_store_type = ast_ptr_type->type;
                ast_address_space = ast_ptr_type->address_space;
            } else {
                return Fail() << "variable with ID " << var.result_id() << " has non-pointer type "
                              << var.type_id();
            }
        }
        TINT_ASSERT(ast_store_type != nullptr);

        const ast::Expression* ast_initializer = nullptr;
        if (var.NumInOperands() > 1) {
            // SPIR-V initializers are always constants.
            // (OpenCL also allows the ID of an OpVariable, but we don't handle that
            // here.)
            ast_initializer = MakeConstantExpression(var.GetSingleWordInOperand(1)).expr;
        }
        auto* ast_var = MakeVar(var.result_id(), ast_address_space, ast_store_type, ast_initializer,
                                Attributes{});
        // TODO(dneto): initializers (a.k.a. initializer expression)
        if (ast_var) {
            builder_.AST().AddGlobalVariable(ast_var);
            module_variable_.GetOrAdd(var.result_id(), [&] {
                auto access = VarAccess(var.result_id(), ast_store_type, ast_address_space);
                return ModuleVariable{ast_var, ast_address_space, access};
            });
        }
    }

    // Emit gl_Position instead of gl_PerVertex
    // TODO(chromium:358408571): handle gl_ClipDistance[] in gl_PerVertex
    if (builtin_position_.per_vertex_var_id) {
        // Make sure the variable has a name.
        namer_.SuggestSanitizedName(builtin_position_.per_vertex_var_id, "gl_Position");
        const ast::Expression* ast_initializer = nullptr;
        if (builtin_position_.per_vertex_var_init_id) {
            // The initializer is complex.
            const auto* init = def_use_mgr_->GetDef(builtin_position_.per_vertex_var_init_id);
            switch (opcode(init)) {
                case spv::Op::OpConstantComposite:
                case spv::Op::OpSpecConstantComposite:
                    ast_initializer =
                        MakeConstantExpression(
                            init->GetSingleWordInOperand(builtin_position_.position_member_index))
                            .expr;
                    break;
                default:
                    return Fail() << "gl_PerVertex initializer too complex. only "
                                     "OpCompositeConstruct and OpSpecConstantComposite "
                                     "are supported: "
                                  << init->PrettyPrint();
            }
        }
        auto storage_type = ConvertType(builtin_position_.position_member_type_id);
        auto ast_address_space = enum_converter_.ToAddressSpace(builtin_position_.storage_class);
        auto* ast_var = MakeVar(builtin_position_.per_vertex_var_id, ast_address_space,
                                storage_type, ast_initializer, {});

        builder_.AST().AddGlobalVariable(ast_var);
        module_variable_.GetOrAdd(builtin_position_.per_vertex_var_id, [&] {
            return ModuleVariable{ast_var, ast_address_space};
        });
    }
    return success_;
}

// @param var_id SPIR-V id of an OpVariable, assumed to be pointer
// to an array
// @returns the IntConstant for the size of the array, or nullptr
const spvtools::opt::analysis::IntConstant* ASTParser::GetArraySize(uint32_t var_id) {
    auto* var = def_use_mgr_->GetDef(var_id);
    if (!var || opcode(var) != spv::Op::OpVariable) {
        return nullptr;
    }
    auto* ptr_type = def_use_mgr_->GetDef(var->type_id());
    if (!ptr_type || opcode(ptr_type) != spv::Op::OpTypePointer) {
        return nullptr;
    }
    auto* array_type = def_use_mgr_->GetDef(ptr_type->GetSingleWordInOperand(1));
    if (!array_type || opcode(array_type) != spv::Op::OpTypeArray) {
        return nullptr;
    }
    auto* size = constant_mgr_->FindDeclaredConstant(array_type->GetSingleWordInOperand(1));
    if (!size) {
        return nullptr;
    }
    return size->AsIntConstant();
}

core::Access ASTParser::VarAccess(uint32_t var_id,
                                  const Type* storage_type,
                                  core::AddressSpace address_space) {
    if (address_space != core::AddressSpace::kStorage) {
        return core::Access::kUndefined;
    }

    bool read_only = read_only_vars_.count(var_id) > 0;
    if (auto* tn = storage_type->As<Named>()) {
        read_only = read_only || read_only_struct_types_.count(tn->name) > 0;
    }

    // Apply the access(read) or access(read_write) modifier.
    return read_only ? core::Access::kRead : core::Access::kReadWrite;
}

const ast::Var* ASTParser::MakeVar(uint32_t id,
                                   core::AddressSpace address_space,
                                   const Type* storage_type,
                                   const ast::Expression* initializer,
                                   Attributes attrs) {
    if (storage_type == nullptr) {
        Fail() << "internal error: can't make ast::Variable for null type";
        return nullptr;
    }

    // Handle variables (textures and samplers) are always in the handle
    // address space, so we don't mention the address space.
    if (address_space == core::AddressSpace::kHandle) {
        address_space = core::AddressSpace::kUndefined;
    }

    if (!ConvertDecorationsForVariable(id, &storage_type, attrs,
                                       address_space != core::AddressSpace::kPrivate)) {
        return nullptr;
    }

    const auto access = VarAccess(id, storage_type, address_space);

    // Use type inference if there is an initializer.
    auto sym = builder_.Symbols().Register(namer_.Name(id));
    return builder_.Var(Source{}, sym, initializer ? ast::Type{} : storage_type->Build(builder_),
                        address_space, access, initializer, std::move(attrs.list));
}

const ast::Let* ASTParser::MakeLet(uint32_t id, const ast::Expression* initializer) {
    auto sym = builder_.Symbols().Register(namer_.Name(id));
    return builder_.Let(Source{}, sym, initializer, tint::Empty);
}

const ast::Override* ASTParser::MakeOverride(uint32_t id,
                                             const Type* type,
                                             const ast::Expression* initializer,
                                             Attributes attrs) {
    if (!ConvertDecorationsForVariable(id, &type, attrs, false)) {
        return nullptr;
    }
    auto sym = builder_.Symbols().Register(namer_.Name(id));
    return builder_.Override(Source{}, sym, type->Build(builder_), initializer,
                             std::move(attrs.list));
}

const ast::Parameter* ASTParser::MakeParameter(uint32_t id, const Type* type, Attributes attrs) {
    if (!ConvertDecorationsForVariable(id, &type, attrs, false)) {
        return nullptr;
    }

    auto sym = builder_.Symbols().Register(namer_.Name(id));
    return builder_.Param(Source{}, sym, type->Build(builder_), attrs.list);
}

bool ASTParser::ConvertDecorationsForVariable(uint32_t id,
                                              const Type** store_type,
                                              Attributes& attrs,
                                              bool transfer_pipeline_io) {
    DecorationList non_builtin_pipeline_decorations;
    for (auto& deco : GetDecorationsFor(id)) {
        if (deco.empty()) {
            return Fail() << "malformed decoration on ID " << id << ": it is empty";
        }
        if (deco[0] == uint32_t(spv::Decoration::BuiltIn)) {
            if (deco.size() == 1) {
                return Fail() << "malformed BuiltIn decoration on ID " << id << ": has no operand";
            }
            const auto spv_builtin = static_cast<spv::BuiltIn>(deco[1]);
            switch (spv_builtin) {
                case spv::BuiltIn::PointSize:
                    special_builtins_[id] = spv_builtin;
                    return false;  // This is not an error
                case spv::BuiltIn::SampleId:
                case spv::BuiltIn::VertexIndex:
                case spv::BuiltIn::InstanceIndex:
                case spv::BuiltIn::LocalInvocationId:
                case spv::BuiltIn::LocalInvocationIndex:
                case spv::BuiltIn::GlobalInvocationId:
                case spv::BuiltIn::WorkgroupId:
                case spv::BuiltIn::NumWorkgroups:
                    // The SPIR-V variable may signed (because GLSL requires signed for
                    // some of these), but WGSL requires unsigned.  Handle specially
                    // so we always perform the conversion at load and store.
                    special_builtins_[id] = spv_builtin;
                    if (auto* forced_type = UnsignedTypeFor(*store_type)) {
                        // Requires conversion and special handling in code generation.
                        if (transfer_pipeline_io) {
                            *store_type = forced_type;
                        }
                    }
                    break;
                case spv::BuiltIn::SampleMask: {
                    // In SPIR-V this is used for both input and output variable.
                    // The SPIR-V variable has store type of array of integer scalar,
                    // either signed or unsigned.
                    // WGSL requires the store type to be u32.
                    auto* size = GetArraySize(id);
                    if (!size || size->GetZeroExtendedValue() != 1) {
                        Fail() << "WGSL supports a sample mask of at most 32 bits. "
                                  "SampleMask must be an array of 1 element.";
                    }
                    special_builtins_[id] = spv_builtin;
                    if (transfer_pipeline_io) {
                        *store_type = ty_.U32();
                    }
                    break;
                }
                case spv::BuiltIn::ClipDistance:
                    Enable(wgsl::Extension::kClipDistances);
                    break;
                default:
                    break;
            }
            auto ast_builtin = enum_converter_.ToBuiltin(spv_builtin);
            if (ast_builtin == core::BuiltinValue::kUndefined) {
                // A diagnostic has already been emitted.
                return false;
            }
            if (transfer_pipeline_io) {
                attrs.Add(builder_, Source{}, ast_builtin);
            }
        }
        if (transfer_pipeline_io && IsPipelineDecoration(deco)) {
            non_builtin_pipeline_decorations.push_back(deco);
        }
        if (deco[0] == uint32_t(spv::Decoration::DescriptorSet)) {
            if (deco.size() == 1) {
                return Fail() << "malformed DescriptorSet decoration on ID " << id
                              << ": has no operand";
            }
            attrs.Add(builder_.Group(Source{}, AInt(deco[1])));
        }
        if (deco[0] == uint32_t(spv::Decoration::Binding)) {
            if (deco.size() == 1) {
                return Fail() << "malformed Binding decoration on ID " << id << ": has no operand";
            }
            attrs.Add(builder_.Binding(Source{}, AInt(deco[1])));
        }
        if (deco[0] == uint32_t(spv::Decoration::NonWritable)) {
            read_only_vars_.insert(id);
        }
    }

    if (transfer_pipeline_io) {
        if (!ConvertPipelineDecorations(*store_type, non_builtin_pipeline_decorations, attrs)) {
            return false;
        }
    }

    return success();
}

DecorationList ASTParser::GetMemberPipelineDecorations(const Struct& struct_type,
                                                       int member_index) {
    // Yes, I could have used std::copy_if or std::copy_if.
    DecorationList result;
    for (const auto& deco : GetDecorationsForMember(struct_id_for_symbol_[struct_type.name],
                                                    static_cast<uint32_t>(member_index))) {
        if (IsPipelineDecoration(deco)) {
            result.emplace_back(deco);
        }
    }
    return result;
}

void ASTParser::SetLocation(Attributes& attributes, const ast::Attribute* replacement) {
    if (!replacement) {
        return;
    }
    for (auto*& attribute : attributes.list) {
        if (attribute->Is<ast::LocationAttribute>()) {
            // Replace this location attribute with the replacement.
            // The old one doesn't leak because it's kept in the builder's AST node
            // list.
            attribute = replacement;
            return;  // Assume there is only one such decoration.
        }
    }
    // The list didn't have a location. Add it.
    attributes.Add(replacement);
}

void ASTParser::SetBlendSrc(Attributes& attributes, const ast::Attribute* replacement) {
    if (!replacement) {
        return;
    }
    for (auto*& attribute : attributes.list) {
        if (attribute->Is<ast::BlendSrcAttribute>()) {
            // Replace this BlendSrc attribute with the replacement.
            // The old one doesn't leak because it's kept in the builder's AST node
            // list.
            attribute = replacement;
            return;  // Assume there is only one such decoration.
        }
    }
    // The list didn't have a BlendSrc. Add it.
    attributes.Add(replacement);
}

bool ASTParser::ConvertPipelineDecorations(const Type* store_type,
                                           const DecorationList& decorations,
                                           Attributes& attributes) {
    // Vulkan defaults to perspective-correct interpolation.
    core::InterpolationType type = core::InterpolationType::kPerspective;
    core::InterpolationSampling sampling = core::InterpolationSampling::kUndefined;

    for (const auto& deco : decorations) {
        TINT_ASSERT(deco.size() > 0);
        switch (static_cast<spv::Decoration>(deco[0])) {
            case spv::Decoration::Location:
                if (deco.size() != 2) {
                    return Fail()
                           << "malformed Location decoration on ID requires one literal operand";
                }
                SetLocation(attributes, builder_.Location(AInt(deco[1])));
                if (store_type->IsIntegerScalarOrVector()) {
                    // Default to flat interpolation for integral user-defined IO types.
                    type = core::InterpolationType::kFlat;
                }
                break;
            case spv::Decoration::Flat:
                type = core::InterpolationType::kFlat;
                break;
            case spv::Decoration::NoPerspective:
                if (store_type->IsIntegerScalarOrVector()) {
                    // This doesn't capture the array or struct case.
                    return Fail() << "NoPerspective is invalid on integral IO";
                }
                type = core::InterpolationType::kLinear;
                break;
            case spv::Decoration::Centroid:
                if (store_type->IsIntegerScalarOrVector()) {
                    // This doesn't capture the array or struct case.
                    return Fail() << "Centroid interpolation sampling is invalid on integral IO";
                }
                sampling = core::InterpolationSampling::kCentroid;
                break;
            case spv::Decoration::Sample:
                if (store_type->IsIntegerScalarOrVector()) {
                    // This doesn't capture the array or struct case.
                    return Fail() << "Sample interpolation sampling is invalid on integral IO";
                }
                sampling = core::InterpolationSampling::kSample;
                break;
            case spv::Decoration::Index:
                if (deco.size() != 2) {
                    return Fail()
                           << "malformed Index decoration on ID requires one literal operand";
                }
                Enable(wgsl::Extension::kDualSourceBlending);
                SetBlendSrc(attributes, builder_.BlendSrc(AInt(deco[1])));
                break;
            default:
                break;
        }
    }

    if (type == core::InterpolationType::kFlat && !attributes.Has<ast::LocationAttribute>()) {
        // WGSL requires that '@interpolate(flat)' needs to be paired with '@location', however
        // SPIR-V requires all fragment shader integer Inputs are 'flat'. If the decorations do not
        // contain a spv::Decoration::Location, then make this perspective.
        type = core::InterpolationType::kPerspective;
    }

    // Apply interpolation.
    if (type == core::InterpolationType::kPerspective &&
        sampling == core::InterpolationSampling::kUndefined) {
        // This is the default. Don't add a decoration.
    } else {
        attributes.Add(builder_.Interpolate(type, sampling));
    }

    return success();
}

bool ASTParser::CanMakeConstantExpression(uint32_t id) {
    if ((id == workgroup_size_builtin_.id) || (id == workgroup_size_builtin_.x_id) ||
        (id == workgroup_size_builtin_.y_id) || (id == workgroup_size_builtin_.z_id)) {
        return true;
    }
    const auto* inst = def_use_mgr_->GetDef(id);
    if (!inst) {
        return false;
    }
    if (opcode(inst) == spv::Op::OpUndef) {
        return true;
    }
    return nullptr != constant_mgr_->FindDeclaredConstant(id);
}

TypedExpression ASTParser::MakeConstantExpression(uint32_t id) {
    if (!success_) {
        return {};
    }

    // Handle the special cases for workgroup sizing.
    if (id == workgroup_size_builtin_.id) {
        auto x = MakeConstantExpression(workgroup_size_builtin_.x_id);
        auto y = MakeConstantExpression(workgroup_size_builtin_.y_id);
        auto z = MakeConstantExpression(workgroup_size_builtin_.z_id);
        auto* ast_type = ty_.Vector(x.type, 3);
        return {ast_type, builder_.Call(Source{}, ast_type->Build(builder_),
                                        tint::Vector{x.expr, y.expr, z.expr})};
    } else if (id == workgroup_size_builtin_.x_id) {
        return MakeConstantExpressionForScalarSpirvConstant(
            Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
            constant_mgr_->GetConstant(
                type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
                {workgroup_size_builtin_.x_value}));
    } else if (id == workgroup_size_builtin_.y_id) {
        return MakeConstantExpressionForScalarSpirvConstant(
            Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
            constant_mgr_->GetConstant(
                type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
                {workgroup_size_builtin_.y_value}));
    } else if (id == workgroup_size_builtin_.z_id) {
        return MakeConstantExpressionForScalarSpirvConstant(
            Source{}, ConvertType(workgroup_size_builtin_.component_type_id),
            constant_mgr_->GetConstant(
                type_mgr_->GetType(workgroup_size_builtin_.component_type_id),
                {workgroup_size_builtin_.z_value}));
    }

    // Handle the general case where a constant is already registered
    // with the SPIR-V optimizer's analysis framework.
    const auto* inst = def_use_mgr_->GetDef(id);
    if (inst == nullptr) {
        Fail() << "ID " << id << " is not a registered instruction";
        return {};
    }
    auto source = GetSourceForInst(inst);

    auto* original_ast_type = ConvertType(inst->type_id());
    if (original_ast_type == nullptr) {
        return {};
    }

    switch (opcode(inst)) {
        case spv::Op::OpUndef:  // Remap undef to null.
        case spv::Op::OpConstantNull:
            return {original_ast_type, MakeNullValue(original_ast_type)};
        case spv::Op::OpConstantTrue:
        case spv::Op::OpConstantFalse:
        case spv::Op::OpConstant: {
            const auto* spirv_const = constant_mgr_->FindDeclaredConstant(id);
            if (spirv_const == nullptr) {
                Fail() << "ID " << id << " is not a constant";
                return {};
            }
            return MakeConstantExpressionForScalarSpirvConstant(source, original_ast_type,
                                                                spirv_const);
        }
        case spv::Op::OpConstantComposite: {
            // Handle vector, matrix, array, and struct

            auto itr = declared_constant_composites_.find(id);
            if (itr != declared_constant_composites_.end()) {
                // We've already declared this constant value as a module-scope const, so just
                // reference that identifier.
                return {original_ast_type, builder_.Expr(itr->second)};
            }

            const auto* spirv_const = constant_mgr_->FindDeclaredConstant(id);
            if (spirv_const->IsZero()) {
                // All zeros, so just use a zero value constructor and always inline it.
                return {original_ast_type,
                        builder_.Call(source, original_ast_type->Build(builder_))};
            }

            // Generate a composite from explicit components.
            bool all_same = true;
            uint32_t first_id = 0u;
            ExpressionList ast_components;
            if (!inst->WhileEachInId([&](const uint32_t* id_ref) -> bool {
                    auto component = MakeConstantExpression(*id_ref);
                    if (!component) {
                        this->Fail() << "invalid constant with ID " << *id_ref;
                        return false;
                    }
                    ast_components.Push(component.expr);

                    // Check if this argument is different from the others.
                    if (first_id != 0u) {
                        if (*id_ref != first_id) {
                            all_same = false;
                        }
                    } else {
                        first_id = *id_ref;
                    }

                    return true;
                })) {
                // We've already emitted a diagnostic.
                return {};
            }

            const ast::Expression* expr = nullptr;
            if (all_same && original_ast_type->Is<Vector>()) {
                // We're constructing a vector and all the operands were the same, so use a splat.
                expr = builder_.Call(source, original_ast_type->Build(builder_), ast_components[0]);
            } else {
                expr = builder_.Call(source, original_ast_type->Build(builder_),
                                     std::move(ast_components));
            }

            if (def_use_mgr_->NumUses(id) == 1) {
                // The constant is only used once, so just inline its use.
                return {original_ast_type, expr};
            }

            // Create a module-scope const declaration for the constant.
            auto name = namer_.Name(id);
            auto* decl = builder_.GlobalConst(name, expr);
            declared_constant_composites_.insert({id, decl->name->symbol});
            return {original_ast_type, builder_.Expr(name)};
        }
        case spv::Op::OpSpecConstantComposite:
        case spv::Op::OpSpecConstantOp: {
            // TODO(crbug.com/tint/111): Handle OpSpecConstantOp and OpSpecConstantComposite here.
            Fail() << "unimplemented: OpSpecConstantOp and OpSpecConstantComposite";
            return {};
        }
        default:
            break;
    }
    Fail() << "unhandled constant instruction " << inst->PrettyPrint();
    return {};
}

TypedExpression ASTParser::MakeConstantExpressionForScalarSpirvConstant(
    Source source,
    const Type* original_ast_type,
    const spvtools::opt::analysis::Constant* spirv_const) {
    auto* ast_type = original_ast_type->UnwrapAlias();

    // TODO(dneto): Note: NullConstant for int, uint, float map to a regular 0.
    // So canonicalization should map that way too.
    // Currently "null<type>" is missing from the WGSL parser.
    // See https://bugs.chromium.org/p/tint/issues/detail?id=34
    return Switch(
        ast_type,
        [&](const I32*) -> TypedExpression {
            const auto value = spirv_const->GetS32();
            if (value == std::numeric_limits<int32_t>::min()) {
                // Avoid overflowing i-suffixed literal.
                return {ty_.I32(), builder_.Call(source, builder_.ty.i32(),
                                                 create<ast::IntLiteralExpression>(
                                                     source, value,
                                                     ast::IntLiteralExpression::Suffix::kNone))};
            } else {
                return {ty_.I32(),
                        create<ast::IntLiteralExpression>(source, static_cast<int64_t>(value),
                                                          ast::IntLiteralExpression::Suffix::kI)};
            }
        },
        [&](const U32*) {
            return TypedExpression{ty_.U32(),
                                   create<ast::IntLiteralExpression>(
                                       source, static_cast<int64_t>(spirv_const->GetU32()),
                                       ast::IntLiteralExpression::Suffix::kU)};
        },
        [&](const F32*) {
            if (auto f = core::CheckedConvert<f32>(AFloat(spirv_const->GetFloat())); f == Success) {
                return TypedExpression{ty_.F32(),
                                       create<ast::FloatLiteralExpression>(
                                           source, static_cast<double>(spirv_const->GetFloat()),
                                           ast::FloatLiteralExpression::Suffix::kF)};
            } else {
                Fail() << "value cannot be represented as 'f32': " << spirv_const->GetFloat();
                return TypedExpression{};
            }
        },
        [&](const F16*) {
            auto bits = spirv_const->AsScalarConstant()->GetU32BitValue();

            // Section 2.2.1 of the SPIR-V spec guarantees that all integer types
            // smaller than 32-bits are automatically zero or sign extended to 32-bits.
            auto val = f16::FromBits(static_cast<uint16_t>(bits));

            if (auto f = core::CheckedConvert<f16>(AFloat(val)); f == Success) {
                return TypedExpression{ty_.F16(), create<ast::FloatLiteralExpression>(
                                                      source, static_cast<double>(val.value),
                                                      ast::FloatLiteralExpression::Suffix::kH)};
            } else {
                Fail() << "value cannot be represented as 'f16': " << spirv_const->GetFloat();
                return TypedExpression{};
            }
        },
        [&](const Bool*) {
            const bool value =
                spirv_const->AsNullConstant() ? false : spirv_const->AsBoolConstant()->value();
            return TypedExpression{ty_.Bool(), create<ast::BoolLiteralExpression>(source, value)};
        },  //
        TINT_ICE_ON_NO_MATCH);
}

const ast::Expression* ASTParser::MakeNullValue(const Type* type) {
    // TODO(dneto): Use the no-operands initializer syntax when it becomes
    // available in Tint.
    // https://github.com/gpuweb/gpuweb/issues/685
    // https://bugs.chromium.org/p/tint/issues/detail?id=34

    if (!type) {
        Fail() << "trying to create null value for a null type";
        return nullptr;
    }

    auto* original_type = type;
    type = type->UnwrapAlias();

    return Switch(
        type,  //
        [&](const I32*) {
            return create<ast::IntLiteralExpression>(Source{}, 0,
                                                     ast::IntLiteralExpression::Suffix::kI);
        },
        [&](const U32*) {
            return create<ast::IntLiteralExpression>(Source{}, 0,
                                                     ast::IntLiteralExpression::Suffix::kU);
        },
        [&](const F32*) {
            return create<ast::FloatLiteralExpression>(Source{}, 0,
                                                       ast::FloatLiteralExpression::Suffix::kF);
        },
        [&](const Vector*) { return builder_.Call(Source{}, type->Build(builder_)); },
        [&](const Matrix*) { return builder_.Call(Source{}, type->Build(builder_)); },
        [&](const Array*) { return builder_.Call(Source{}, type->Build(builder_)); },
        [&](const Bool*) { return create<ast::BoolLiteralExpression>(Source{}, false); },
        [&](const Struct* struct_ty) {
            ExpressionList ast_components;
            for (auto* member : struct_ty->members) {
                ast_components.Push(MakeNullValue(member));
            }
            return builder_.Call(Source{}, original_type->Build(builder_),
                                 std::move(ast_components));
        },  //
        TINT_ICE_ON_NO_MATCH);
}

TypedExpression ASTParser::MakeNullExpression(const Type* type) {
    return {type, MakeNullValue(type)};
}

const Type* ASTParser::UnsignedTypeFor(const Type* type) {
    if (type->Is<I32>()) {
        return ty_.U32();
    }
    if (auto* v = type->As<Vector>()) {
        if (v->type->Is<I32>()) {
            return ty_.Vector(ty_.U32(), v->size);
        }
    }
    return {};
}

const Type* ASTParser::SignedTypeFor(const Type* type) {
    if (type->Is<U32>()) {
        return ty_.I32();
    }
    if (auto* v = type->As<Vector>()) {
        if (v->type->Is<U32>()) {
            return ty_.Vector(ty_.I32(), v->size);
        }
    }
    return {};
}

TypedExpression ASTParser::RectifyOperandSignedness(const spvtools::opt::Instruction& inst,
                                                    TypedExpression&& expr) {
    bool requires_signed = false;
    bool requires_unsigned = false;
    if (IsGlslExtendedInstruction(inst)) {
        const auto extended_opcode = static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1));
        requires_signed = AssumesSignedOperands(extended_opcode);
        requires_unsigned = AssumesUnsignedOperands(extended_opcode);
    } else {
        const auto op = opcode(inst);
        requires_signed = AssumesSignedOperands(op);
        requires_unsigned = AssumesUnsignedOperands(op);
    }
    if (!requires_signed && !requires_unsigned) {
        // No conversion is required, assuming our tables are complete.
        return std::move(expr);
    }
    if (!expr) {
        Fail() << "internal error: RectifyOperandSignedness given a null expr\n";
        return {};
    }
    // TODO(crbug.com/tint/1669) should this unpack aliases too?
    auto* type = expr.type->UnwrapRef();
    if (!type) {
        Fail() << "internal error: unmapped type for: " << expr.expr->TypeInfo().name << "\n";
        return {};
    }
    if (requires_unsigned) {
        if (auto* unsigned_ty = UnsignedTypeFor(type)) {
            // Conversion is required.
            return {unsigned_ty,
                    builder_.Bitcast(Source{}, unsigned_ty->Build(builder_), expr.expr)};
        }
    } else if (requires_signed) {
        if (auto* signed_ty = SignedTypeFor(type)) {
            // Conversion is required.
            return {signed_ty, builder_.Bitcast(Source{}, signed_ty->Build(builder_), expr.expr)};
        }
    }
    // We should not reach here.
    return std::move(expr);
}

TypedExpression ASTParser::RectifySecondOperandSignedness(const spvtools::opt::Instruction& inst,
                                                          const Type* first_operand_type,
                                                          TypedExpression&& second_operand_expr) {
    const Type* target_type = first_operand_type->UnwrapRef();
    if ((target_type != second_operand_expr.type->UnwrapRef()) &&
        AssumesSecondOperandSignednessMatchesFirstOperand(opcode(inst))) {
        // Conversion is required.
        return {target_type,
                builder_.Bitcast(Source{}, target_type->Build(builder_), second_operand_expr.expr)};
    }
    // No conversion necessary.
    return std::move(second_operand_expr);
}

const Type* ASTParser::ForcedResultType(const spvtools::opt::Instruction& inst,
                                        const Type* first_operand_type) {
    first_operand_type = first_operand_type->UnwrapRef();
    const auto op = opcode(inst);
    if (AssumesResultSignednessMatchesFirstOperand(op)) {
        return first_operand_type;
    }
    if (IsGlslExtendedInstruction(inst)) {
        const auto extended_opcode = static_cast<GLSLstd450>(inst.GetSingleWordInOperand(1));
        if (AssumesResultSignednessMatchesFirstOperand(extended_opcode)) {
            return first_operand_type;
        }
    }
    return nullptr;
}

const Type* ASTParser::GetSignedIntMatchingShape(const Type* other) {
    if (other == nullptr) {
        Fail() << "no type provided";
    }
    if (other->IsAnyOf<F32, U32, I32>()) {
        return ty_.I32();
    }
    if (auto* vec_ty = other->As<Vector>()) {
        return ty_.Vector(ty_.I32(), vec_ty->size);
    }
    Fail() << "required numeric scalar or vector, but got " << other->TypeInfo().name;
    return nullptr;
}

const Type* ASTParser::GetUnsignedIntMatchingShape(const Type* other) {
    if (other == nullptr) {
        Fail() << "no type provided";
        return nullptr;
    }
    if (other->IsAnyOf<F32, U32, I32>()) {
        return ty_.U32();
    }
    if (auto* vec_ty = other->As<Vector>()) {
        return ty_.Vector(ty_.U32(), vec_ty->size);
    }
    Fail() << "required numeric scalar or vector, but got " << other->TypeInfo().name;
    return nullptr;
}

TypedExpression ASTParser::RectifyForcedResultType(TypedExpression expr,
                                                   const spvtools::opt::Instruction& inst,
                                                   const Type* first_operand_type) {
    auto* forced_result_ty = ForcedResultType(inst, first_operand_type);
    if ((!forced_result_ty) || (forced_result_ty == expr.type)) {
        return expr;
    }
    return {expr.type, builder_.Bitcast(Source{}, expr.type->Build(builder_), expr.expr)};
}

TypedExpression ASTParser::AsUnsigned(TypedExpression expr) {
    if (expr.type && expr.type->IsSignedScalarOrVector()) {
        auto* new_type = GetUnsignedIntMatchingShape(expr.type);
        return {new_type, builder_.Bitcast(Source{}, new_type->Build(builder_), expr.expr)};
    }
    return expr;
}

TypedExpression ASTParser::AsSigned(TypedExpression expr) {
    if (expr.type && expr.type->IsUnsignedScalarOrVector()) {
        auto* new_type = GetSignedIntMatchingShape(expr.type);
        return {new_type, builder_.Bitcast(Source{}, new_type->Build(builder_), expr.expr)};
    }
    return expr;
}

bool ASTParser::EmitFunctions() {
    if (!success_) {
        return false;
    }
    for (const auto* f : topologically_ordered_functions_) {
        if (!success_) {
            return false;
        }

        auto id = f->result_id();
        auto it = function_to_ep_info_.find(id);
        if (it == function_to_ep_info_.end()) {
            FunctionEmitter emitter(this, *f, nullptr);
            success_ = emitter.Emit();
        } else {
            for (const auto& ep : it->second) {
                FunctionEmitter emitter(this, *f, &ep);
                success_ = emitter.Emit();
                if (!success_) {
                    return false;
                }
            }
        }
    }
    return success_;
}

const spvtools::opt::Instruction* ASTParser::GetMemoryObjectDeclarationForHandle(
    uint32_t id,
    bool follow_image) {
    auto saved_id = id;
    auto local_fail = [this, saved_id, id, follow_image]() -> const spvtools::opt::Instruction* {
        const auto* inst = def_use_mgr_->GetDef(id);
        Fail() << "Could not find memory object declaration for the "
               << (follow_image ? "image" : "sampler") << " underlying id " << id
               << " (from original id " << saved_id << ") "
               << (inst ? inst->PrettyPrint() : std::string());
        return nullptr;
    };

    auto& memo_table = (follow_image ? mem_obj_decl_image_ : mem_obj_decl_sampler_);

    // Use a visited set to defend against bad input which might have long
    // chains or even loops.
    std::unordered_set<uint32_t> visited;

    // Trace backward in the SSA data flow until we hit a memory object
    // declaration.
    while (true) {
        auto where = memo_table.find(id);
        if (where != memo_table.end()) {
            return where->second;
        }
        // Protect against loops.
        auto visited_iter = visited.find(id);
        if (visited_iter != visited.end()) {
            // We've hit a loop. Mark all the visited nodes
            // as dead ends.
            for (auto iter : visited) {
                memo_table[iter] = nullptr;
            }
            return nullptr;
        }
        visited.insert(id);

        const auto* inst = def_use_mgr_->GetDef(id);
        if (inst == nullptr) {
            return local_fail();
        }
        switch (opcode(inst)) {
            case spv::Op::OpFunctionParameter:
            case spv::Op::OpVariable:
                // We found the memory object declaration.
                // Remember it as the answer for the whole path.
                for (auto iter : visited) {
                    memo_table[iter] = inst;
                }
                return inst;
            case spv::Op::OpLoad:
                // Follow the pointer being loaded
                id = inst->GetSingleWordInOperand(0);
                break;
            case spv::Op::OpCopyObject:
                // Follow the object being copied.
                id = inst->GetSingleWordInOperand(0);
                break;
            case spv::Op::OpAccessChain:
            case spv::Op::OpInBoundsAccessChain:
            case spv::Op::OpPtrAccessChain:
            case spv::Op::OpInBoundsPtrAccessChain:
                // Follow the base pointer.
                id = inst->GetSingleWordInOperand(0);
                break;
            case spv::Op::OpSampledImage:
                // Follow the image or the sampler, depending on the follow_image
                // parameter.
                id = inst->GetSingleWordInOperand(follow_image ? 0 : 1);
                break;
            case spv::Op::OpImage:
                // Follow the sampled image
                id = inst->GetSingleWordInOperand(0);
                break;
            default:
                // Can't trace further.
                // Remember it as the answer for the whole path.
                for (auto iter : visited) {
                    memo_table[iter] = nullptr;
                }
                return nullptr;
        }
    }
}

const spvtools::opt::Instruction* ASTParser::GetSpirvTypeForHandleOrHandleMemoryObjectDeclaration(
    const spvtools::opt::Instruction& obj) {
    if (!success()) {
        return nullptr;
    }
    // The WGSL handle type is determined by looking at information from
    // several sources:
    //    - the usage of the handle by image access instructions
    //    - the SPIR-V type declaration
    // Each source does not have enough information to completely determine
    // the result.

    // Messages are phrased in terms of images and samplers because those
    // are the only SPIR-V handles supported by WGSL.

    // Get the SPIR-V handle type.
    const auto* type = def_use_mgr_->GetDef(obj.type_id());
    if (!type) {
        Fail() << "Invalid type for image, sampler, variable or function parameter to image or "
                  "sampler "
               << obj.PrettyPrint();
        return nullptr;
    }
    switch (opcode(type)) {
        case spv::Op::OpTypeSampler:
        case spv::Op::OpTypeImage:
            return type;
        case spv::Op::OpTypePointer:
            // The remaining cases.
            break;
        default:
            Fail() << "Invalid type for image, sampler, variable or function parameter to image or "
                      "sampler "
                   << obj.PrettyPrint();
            return nullptr;
    }

    // Look at the pointee type instead.
    const auto* raw_handle_type = def_use_mgr_->GetDef(type->GetSingleWordInOperand(1));
    if (!raw_handle_type) {
        Fail() << "Invalid pointer type for variable or function parameter " << obj.PrettyPrint();
        return nullptr;
    }
    switch (opcode(raw_handle_type)) {
        case spv::Op::OpTypeSampler:
        case spv::Op::OpTypeImage:
            // The expected cases.
            break;
        case spv::Op::OpTypeArray:
        case spv::Op::OpTypeRuntimeArray:
            Fail() << "arrays of textures or samplers are not supported in WGSL; can't "
                      "translate variable or function parameter: "
                   << obj.PrettyPrint();
            return nullptr;
        case spv::Op::OpTypeSampledImage:
            Fail() << "WGSL does not support combined image-samplers: " << obj.PrettyPrint();
            return nullptr;
        default:
            Fail() << "invalid type for image or sampler variable or function "
                      "parameter: "
                   << obj.PrettyPrint();
            return nullptr;
    }
    return raw_handle_type;
}

const Type* ASTParser::GetHandleTypeForSpirvHandle(const spvtools::opt::Instruction& obj) {
    auto where = handle_type_.find(&obj);
    if (where != handle_type_.end()) {
        return where->second;
    }

    const spvtools::opt::Instruction* raw_handle_type =
        GetSpirvTypeForHandleOrHandleMemoryObjectDeclaration(obj);
    if (!raw_handle_type) {
        return nullptr;
    }

    // The memory object declaration could be a sampler or image.
    // Where possible, determine which one it is from the usage inferred
    // for the variable.
    Usage usage = handle_usage_[&obj];
    if (!usage.IsValid()) {
        Fail() << "Invalid sampler or texture usage for variable or function parameter "
               << obj.PrettyPrint() << "\n"
               << usage;
        return nullptr;
    }
    // Infer a handle type, if usage didn't already tell us.
    if (!usage.IsComplete()) {
        // In SPIR-V you could statically reference a texture or sampler without
        // using it in a way that gives us a clue on how to declare it.  Look inside
        // the store type to infer a usage.
        if (opcode(raw_handle_type) == spv::Op::OpTypeSampler) {
            usage.AddSampler();
        } else {
            // It's a texture.
            if (raw_handle_type->NumInOperands() != 7) {
                Fail() << "invalid SPIR-V image type: expected 7 operands: "
                       << raw_handle_type->PrettyPrint();
                return nullptr;
            }
            const auto sampled_param = raw_handle_type->GetSingleWordInOperand(5);
            const auto format_param = raw_handle_type->GetSingleWordInOperand(6);
            // Only storage images have a format.
            if ((format_param != uint32_t(spv::ImageFormat::Unknown)) ||
                sampled_param == 2 /* without sampler */) {
                // Get NonWritable and NonReadable attributes of the variable.
                bool is_nonwritable = false;
                bool is_nonreadable = false;
                for (const auto& deco : GetDecorationsFor(obj.result_id())) {
                    if (deco.size() != 1) {
                        continue;
                    }
                    if (deco[0] == uint32_t(spv::Decoration::NonWritable)) {
                        is_nonwritable = true;
                    }
                    if (deco[0] == uint32_t(spv::Decoration::NonReadable)) {
                        is_nonreadable = true;
                    }
                }
                if (is_nonwritable && is_nonreadable) {
                    Fail() << "storage image variable is both NonWritable and NonReadable"
                           << obj.PrettyPrint();
                }
                if (!is_nonwritable && !is_nonreadable) {
                    Fail() << "storage image variable is neither NonWritable nor NonReadable"
                           << obj.PrettyPrint();
                }
                // Let's make it one of the storage textures.
                if (is_nonwritable) {
                    usage.AddStorageReadTexture();
                } else {
                    usage.AddStorageWriteTexture();
                }
            } else {
                usage.AddSampledTexture();
            }
        }
        if (!usage.IsComplete()) {
            Fail() << "internal error: should have inferred a complete handle type. got "
                   << usage.to_str();
            return nullptr;
        }
    }

    // Construct the Tint handle type.
    const Type* ast_handle_type = nullptr;
    if (usage.IsSampler()) {
        ast_handle_type =
            ty_.Sampler(usage.IsComparisonSampler() ? core::type::SamplerKind::kComparisonSampler
                                                    : core::type::SamplerKind::kSampler);
    } else if (usage.IsTexture()) {
        const spvtools::opt::analysis::Image* image_type =
            type_mgr_->GetType(raw_handle_type->result_id())->AsImage();
        if (!image_type) {
            Fail() << "internal error: Couldn't look up image type"
                   << raw_handle_type->PrettyPrint();
            return nullptr;
        }

        if (image_type->is_arrayed()) {
            // Give a nicer error message here, where we have the offending variable
            // in hand, rather than inside the enum converter.
            switch (static_cast<spv::Dim>(image_type->dim())) {
                case spv::Dim::Dim2D:
                case spv::Dim::Cube:
                    break;
                default:
                    Fail() << "WGSL arrayed textures must be 2d_array or cube_array: "
                              "invalid multisampled texture variable or function parameter "
                           << namer_.Name(obj.result_id()) << ": " << obj.PrettyPrint();
                    return nullptr;
            }
        }

        const core::type::TextureDimension dim =
            enum_converter_.ToDim(image_type->dim(), image_type->is_arrayed());
        if (dim == core::type::TextureDimension::kNone) {
            return nullptr;
        }

        // WGSL storage textures are always formatted.  Unformatted textures are always sampled.
        if (usage.IsSampledTexture() || usage.IsStorageReadOnlyTexture() ||
            (uint32_t(image_type->format()) == uint32_t(spv::ImageFormat::Unknown))) {
            // Make a sampled texture type.
            auto* ast_sampled_component_type =
                ConvertType(raw_handle_type->GetSingleWordInOperand(0));

            // Vulkan ignores the depth parameter on OpImage, so pay attention to the
            // usage as well.  That is, it's valid for a Vulkan shader to use an
            // OpImage variable with an OpImage*Dref* instruction.  In WGSL we must
            // treat that as a depth texture.
            if (image_type->depth() == 1 || usage.IsDepthTexture()) {
                if (image_type->is_multisampled()) {
                    ast_handle_type = ty_.DepthMultisampledTexture(dim);
                } else {
                    ast_handle_type = ty_.DepthTexture(dim);
                }
            } else if (image_type->is_multisampled()) {
                if (dim != core::type::TextureDimension::k2d) {
                    Fail() << "WGSL multisampled textures must be 2d and non-arrayed: "
                              "invalid multisampled texture variable or function parameter "
                           << namer_.Name(obj.result_id()) << ": " << obj.PrettyPrint();
                }
                // Multisampled textures are never depth textures.
                ast_handle_type = ty_.MultisampledTexture(dim, ast_sampled_component_type);
            } else {
                ast_handle_type = ty_.SampledTexture(dim, ast_sampled_component_type);
            }
        } else {
            const auto access =
                usage.IsStorageReadWriteTexture() ? core::Access::kReadWrite : core::Access::kWrite;
            if (access == core::Access::kReadWrite) {
                Require(wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);
            }
            const auto format = enum_converter_.ToTexelFormat(image_type->format());
            if (format == core::TexelFormat::kUndefined) {
                return nullptr;
            }
            ast_handle_type = ty_.StorageTexture(dim, format, access);
        }
    } else {
        Fail() << "unsupported: UniformConstant variable or function parameter is not a recognized "
                  "sampler or texture "
               << obj.PrettyPrint();
        return nullptr;
    }

    // Remember it for later.
    handle_type_[&obj] = ast_handle_type;
    return ast_handle_type;
}

const Type* ASTParser::GetComponentTypeForFormat(core::TexelFormat format) {
    switch (format) {
        case core::TexelFormat::kR32Uint:
        case core::TexelFormat::kRgba8Uint:
        case core::TexelFormat::kRg32Uint:
        case core::TexelFormat::kRgba16Uint:
        case core::TexelFormat::kRgba32Uint:
            return ty_.U32();

        case core::TexelFormat::kR32Sint:
        case core::TexelFormat::kRgba8Sint:
        case core::TexelFormat::kRg32Sint:
        case core::TexelFormat::kRgba16Sint:
        case core::TexelFormat::kRgba32Sint:
            return ty_.I32();

        case core::TexelFormat::kRgba8Unorm:
        case core::TexelFormat::kRgba8Snorm:
        case core::TexelFormat::kR32Float:
        case core::TexelFormat::kRg32Float:
        case core::TexelFormat::kRgba16Float:
        case core::TexelFormat::kRgba32Float:
            return ty_.F32();
        default:
            break;
    }
    Fail() << "unknown format " << int(format);
    return nullptr;
}

unsigned ASTParser::GetChannelCountForFormat(core::TexelFormat format) {
    switch (format) {
        case core::TexelFormat::kR32Float:
        case core::TexelFormat::kR32Sint:
        case core::TexelFormat::kR32Uint:
            // One channel
            return 1;

        case core::TexelFormat::kRg32Float:
        case core::TexelFormat::kRg32Sint:
        case core::TexelFormat::kRg32Uint:
            // Two channels
            return 2;

        case core::TexelFormat::kRgba16Float:
        case core::TexelFormat::kRgba16Sint:
        case core::TexelFormat::kRgba16Uint:
        case core::TexelFormat::kRgba32Float:
        case core::TexelFormat::kRgba32Sint:
        case core::TexelFormat::kRgba32Uint:
        case core::TexelFormat::kRgba8Sint:
        case core::TexelFormat::kRgba8Snorm:
        case core::TexelFormat::kRgba8Uint:
        case core::TexelFormat::kRgba8Unorm:
            // Four channels
            return 4;

        default:
            break;
    }
    Fail() << "unknown format " << int(format);
    return 0;
}

const Type* ASTParser::GetTexelTypeForFormat(core::TexelFormat format) {
    const auto* component_type = GetComponentTypeForFormat(format);
    if (!component_type) {
        return nullptr;
    }
    return ty_.Vector(component_type, 4);
}

bool ASTParser::RegisterHandleUsage() {
    if (!success_) {
        return false;
    }

    // Map a function ID to the list of its function parameter instructions, in
    // order.
    std::unordered_map<uint32_t, std::vector<const spvtools::opt::Instruction*>> function_params;
    for (const auto* f : topologically_ordered_functions_) {
        // Record the instructions defining this function's parameters.
        auto& params = function_params[f->result_id()];
        f->ForEachParam(
            [&params](const spvtools::opt::Instruction* param) { params.push_back(param); });
    }

    // Returns the memory object declaration for an image underlying the first
    // operand of the given image instruction.
    auto get_image = [this](const spvtools::opt::Instruction& image_inst) {
        return this->GetMemoryObjectDeclarationForHandle(image_inst.GetSingleWordInOperand(0),
                                                         true);
    };
    // Returns the memory object declaration for a sampler underlying the first
    // operand of the given image instruction.
    auto get_sampler = [this](const spvtools::opt::Instruction& image_inst) {
        return this->GetMemoryObjectDeclarationForHandle(image_inst.GetSingleWordInOperand(0),
                                                         false);
    };

    // Scan the bodies of functions for image operations, recording their implied
    // usage properties on the memory object declarations (i.e. variables or
    // function parameters).  We scan the functions in an order so that callees
    // precede callers. That way the usage on a function parameter is already
    // computed before we see the call to that function.  So when we reach
    // a function call, we can add the usage from the callee formal parameters.
    for (const auto* f : topologically_ordered_functions_) {
        for (const auto& bb : *f) {
            for (const auto& inst : bb) {
                switch (opcode(inst)) {
                        // Single texel reads and writes

                    case spv::Op::OpImageRead:
                        handle_usage_[get_image(inst)].AddStorageReadTexture();
                        break;
                    case spv::Op::OpImageWrite:
                        handle_usage_[get_image(inst)].AddStorageWriteTexture();
                        break;
                    case spv::Op::OpImageFetch:
                        handle_usage_[get_image(inst)].AddSampledTexture();
                        break;

                        // Sampling and gathering from a sampled image.

                    case spv::Op::OpImageSampleImplicitLod:
                    case spv::Op::OpImageSampleExplicitLod:
                    case spv::Op::OpImageSampleProjImplicitLod:
                    case spv::Op::OpImageSampleProjExplicitLod:
                    case spv::Op::OpImageGather:
                        handle_usage_[get_image(inst)].AddSampledTexture();
                        handle_usage_[get_sampler(inst)].AddSampler();
                        break;
                    case spv::Op::OpImageSampleDrefImplicitLod:
                    case spv::Op::OpImageSampleDrefExplicitLod:
                    case spv::Op::OpImageSampleProjDrefImplicitLod:
                    case spv::Op::OpImageSampleProjDrefExplicitLod:
                    case spv::Op::OpImageDrefGather:
                        // Depth reference access implies usage as a depth texture, which
                        // in turn is a sampled texture.
                        handle_usage_[get_image(inst)].AddDepthTexture();
                        handle_usage_[get_sampler(inst)].AddComparisonSampler();
                        break;

                        // Image queries

                    case spv::Op::OpImageQuerySizeLod:
                        // Vulkan requires Sampled=1 for this. SPIR-V already requires MS=0.
                        handle_usage_[get_image(inst)].AddSampledTexture();
                        break;
                    case spv::Op::OpImageQuerySize:
                        // Applies to either MS=1 or Sampled=0 or 2.
                        // So we can't force it to be multisampled, or storage image.
                        break;
                    case spv::Op::OpImageQueryLod:
                        handle_usage_[get_image(inst)].AddSampledTexture();
                        handle_usage_[get_sampler(inst)].AddSampler();
                        break;
                    case spv::Op::OpImageQueryLevels:
                        // We can't tell anything more than that it's an image.
                        handle_usage_[get_image(inst)].AddTexture();
                        break;
                    case spv::Op::OpImageQuerySamples:
                        handle_usage_[get_image(inst)].AddMultisampledTexture();
                        break;

                        // Function calls

                    case spv::Op::OpFunctionCall: {
                        // Propagate handle usages from callee function formal parameters to
                        // the matching caller parameters.  This is where we rely on the
                        // fact that callees have been processed earlier in the flow.
                        const auto num_in_operands = inst.NumInOperands();
                        // The first operand of the call is the function ID.
                        // The remaining operands are the operands to the function.
                        if (num_in_operands < 1) {
                            return Fail() << "Call instruction must have at least one operand"
                                          << inst.PrettyPrint();
                        }
                        const auto function_id = inst.GetSingleWordInOperand(0);
                        const auto& formal_params = function_params[function_id];
                        if (formal_params.size() != (num_in_operands - 1)) {
                            return Fail()
                                   << "Called function has " << formal_params.size()
                                   << " parameters, but function call has " << (num_in_operands - 1)
                                   << " parameters" << inst.PrettyPrint();
                        }
                        for (uint32_t i = 1; i < num_in_operands; ++i) {
                            auto where = handle_usage_.find(formal_params[i - 1]);
                            if (where == handle_usage_.end()) {
                                // We haven't recorded any handle usage on the formal parameter.
                                continue;
                            }
                            const Usage& formal_param_usage = where->second;
                            const auto operand_id = inst.GetSingleWordInOperand(i);
                            const auto* operand_as_sampler =
                                GetMemoryObjectDeclarationForHandle(operand_id, false);
                            const auto* operand_as_image =
                                GetMemoryObjectDeclarationForHandle(operand_id, true);
                            if (operand_as_sampler) {
                                handle_usage_[operand_as_sampler].Add(formal_param_usage);
                            }
                            if (operand_as_image && (operand_as_image != operand_as_sampler)) {
                                handle_usage_[operand_as_image].Add(formal_param_usage);
                            }
                        }
                        break;
                    }

                    default:
                        break;
                }
            }
        }
    }
    return success_;
}

Usage ASTParser::GetHandleUsage(uint32_t id) const {
    const auto where = handle_usage_.find(def_use_mgr_->GetDef(id));
    if (where != handle_usage_.end()) {
        return where->second;
    }
    return Usage();
}

const spvtools::opt::Instruction* ASTParser::GetInstructionForTest(uint32_t id) const {
    return def_use_mgr_ ? def_use_mgr_->GetDef(id) : nullptr;
}

std::string ASTParser::GetMemberName(const Struct& struct_type, int member_index) {
    auto where = struct_id_for_symbol_.find(struct_type.name);
    if (where == struct_id_for_symbol_.end()) {
        Fail() << "no structure type registered for symbol";
        return "";
    }
    return namer_.GetMemberName(where->second, static_cast<uint32_t>(member_index));
}

WorkgroupSizeInfo::WorkgroupSizeInfo() = default;

WorkgroupSizeInfo::~WorkgroupSizeInfo() = default;

}  // namespace tint::spirv::reader::ast_parser
