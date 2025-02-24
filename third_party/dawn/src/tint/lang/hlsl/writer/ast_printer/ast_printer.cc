/// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/ast_printer/ast_printer.h"

#include <cmath>
#include <functional>
#include <iomanip>
#include <utility>
#include <vector>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/hlsl/writer/ast_raise/calculate_array_length.h"
#include "src/tint/lang/hlsl/writer/ast_raise/decompose_memory_access.h"
#include "src/tint/lang/hlsl/writer/ast_raise/localize_struct_array_assignment.h"
#include "src/tint/lang/hlsl/writer/ast_raise/num_workgroups_from_uniform.h"
#include "src/tint/lang/hlsl/writer/ast_raise/pixel_local.h"
#include "src/tint/lang/hlsl/writer/ast_raise/truncate_interstage_variables.h"
#include "src/tint/lang/hlsl/writer/common/option_helpers.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/array_length_from_uniform.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"
#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"
#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/fold_constants.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/promote_initializers_to_let.h"
#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"
#include "src/tint/lang/wgsl/ast/transform/remove_continue_in_switch.h"
#include "src/tint/lang/wgsl/ast/transform/remove_phonies.h"
#include "src/tint/lang/wgsl/ast/transform/robustness.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h"
#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/helpers/append_vector.h"
#include "src/tint/lang/wgsl/helpers/check_supported_extensions.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

namespace tint::hlsl::writer {
namespace {

const char kTempNamePrefix[] = "tint_tmp";

const char* ImageFormatToRWtextureType(core::TexelFormat image_format) {
    switch (image_format) {
        case core::TexelFormat::kR8Unorm:
        case core::TexelFormat::kBgra8Unorm:
        case core::TexelFormat::kRgba8Unorm:
        case core::TexelFormat::kRgba8Snorm:
        case core::TexelFormat::kRgba16Float:
        case core::TexelFormat::kR32Float:
        case core::TexelFormat::kRg32Float:
        case core::TexelFormat::kRgba32Float:
            return "float4";
        case core::TexelFormat::kRgba8Uint:
        case core::TexelFormat::kRgba16Uint:
        case core::TexelFormat::kR32Uint:
        case core::TexelFormat::kRg32Uint:
        case core::TexelFormat::kRgba32Uint:
            return "uint4";
        case core::TexelFormat::kRgba8Sint:
        case core::TexelFormat::kRgba16Sint:
        case core::TexelFormat::kR32Sint:
        case core::TexelFormat::kRg32Sint:
        case core::TexelFormat::kRgba32Sint:
            return "int4";
        default:
            return nullptr;
    }
}

void PrintF32(StringStream& out, float value) {
    if (std::isinf(value)) {
        out << "0.0f " << (value >= 0 ? "/* inf */" : "/* -inf */");
    } else if (std::isnan(value)) {
        out << "0.0f /* nan */";
    } else {
        out << tint::strconv::FloatToString(value) << "f";
    }
}

void PrintF16(StringStream& out, float value) {
    if (std::isinf(value)) {
        out << "0.0h " << (value >= 0 ? "/* inf */" : "/* -inf */");
    } else if (std::isnan(value)) {
        out << "0.0h /* nan */";
    } else {
        out << tint::strconv::FloatToString(value) << "h";
    }
}

// Helper for writing " : register(RX, spaceY)", where R is the register, X is
// the binding point binding value, and Y is the binding point group value.
struct RegisterAndSpace {
    RegisterAndSpace(char r, BindingPoint bp) : reg(r), binding_point(bp) {}

    const char reg;
    BindingPoint const binding_point;
};

StringStream& operator<<(StringStream& s, const RegisterAndSpace& rs) {
    s << " : register(" << rs.reg << rs.binding_point.binding;
    // Omit the space if it's 0, as it's the default.
    // SM 5.0 doesn't support spaces, so we don't emit them if group is 0 for better compatibility.
    if (rs.binding_point.group == 0) {
        s << ")";
    } else {
        s << ", space" << rs.binding_point.group << ")";
    }
    return s;
}

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program& in, const Options& options) {
    ast::transform::Manager manager;
    ast::transform::DataMap data;

    manager.Add<ast::transform::FoldConstants>();

    manager.Add<ast::transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<ast::transform::ExpandCompoundAssignment>();

    manager.Add<ast::transform::Unshadow>();  // Must come before DirectVariableAccess

    // LocalizeStructArrayAssignment must come after:
    // * SimplifyPointers, because it assumes assignment to arrays in structs are
    // done directly, not indirectly.
    // TODO(crbug.com/tint/1340): See if we can get rid of the duplicate
    // SimplifyPointers transform. Can't do it right now because
    // LocalizeStructArrayAssignment introduces pointers.
    manager.Add<ast::transform::SimplifyPointers>();
    manager.Add<LocalizeStructArrayAssignment>();

    manager.Add<ast::transform::PromoteSideEffectsToDecl>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        manager.Add<ast::transform::Robustness>();

        ast::transform::Robustness::Config config = {};
        config.bindings_ignored = std::unordered_set<BindingPoint>(
            options.bindings.ignored_by_robustness_transform.cbegin(),
            options.bindings.ignored_by_robustness_transform.cend());

        // Direct3D guarantees to return zero for any resource that is accessed out of bounds, and
        // according to the description of the assembly store_uav_typed, out of bounds addressing
        // means nothing gets written to memory.
        config.texture_action = ast::transform::Robustness::Action::kIgnore;

        data.Add<ast::transform::Robustness::Config>(config);
    }

    tint::transform::multiplanar::BindingsMap multiplanar_map{};
    RemapperData remapper_data{};
    ArrayLengthFromUniformOptions array_length_from_uniform_options{};
    PopulateBindingRelatedOptions(options, remapper_data, multiplanar_map,
                                  array_length_from_uniform_options);

    manager.Add<ast::transform::BindingRemapper>();
    // D3D11 and 12 registers like `t3` and `c3` have the same bindingOffset number in
    // the remapping but should not be considered a collision because they have
    // different types.
    data.Add<ast::transform::BindingRemapper::Remappings>(
        remapper_data, options.bindings.access_controls, /* allow_collisions */ true);

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    // MultiplanarExternalTexture must come after BindingRemapper
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(multiplanar_map,
                                                                           /* may_collide */ true);
    manager.Add<ast::transform::MultiplanarExternalTexture>();

    {  // Builtin polyfills
        ast::transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = ast::transform::BuiltinPolyfill::Level::kFull;
        polyfills.asinh = true;
        polyfills.atanh = ast::transform::BuiltinPolyfill::Level::kFull;
        polyfills.bitshift_modulo = true;
        polyfills.clamp_int = true;
        // TODO(crbug.com/tint/1449): Some of these can map to HLSL's `firstbitlow`
        // and `firstbithigh`.
        polyfills.conv_f32_to_iu32 = true;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = ast::transform::BuiltinPolyfill::Level::kFull;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.fwidth_fine = true;
        polyfills.insert_bits = ast::transform::BuiltinPolyfill::Level::kFull;
        polyfills.int_div_mod = !options.disable_polyfill_integer_div_mod;
        polyfills.precise_float_mod = true;
        polyfills.reflect_vec2_f32 = options.polyfill_reflect_vec2_f32;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.workgroup_uniform_load = true;
        polyfills.dot_4x8_packed = options.polyfill_dot_4x8_packed;
        polyfills.pack_unpack_4x8 = options.polyfill_pack_unpack_4x8;
        // Currently Pack4xU8Clamp() must be polyfilled because on latest DXC pack_clamp_u8()
        // receives an int32_t4 as its input.
        // See https://github.com/microsoft/DirectXShaderCompiler/issues/5091 for more details.
        polyfills.pack_4xu8_clamp = true;
        data.Add<ast::transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<ast::transform::BuiltinPolyfill>();  // Must come before DirectVariableAccess
    }

    manager.Add<ast::transform::DirectVariableAccess>();

    if (!options.disable_workgroup_init) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<ast::transform::ZeroInitWorkgroupMemory>();
    }

    {
        PixelLocal::Config cfg;
        for (auto it : options.pixel_local.attachments) {
            uint32_t member_index = it.first;
            auto& attachment = it.second;

            cfg.pls_member_to_rov_reg.Add(member_index, attachment.index);

            core::TexelFormat format = core::TexelFormat::kUndefined;
            switch (attachment.format) {
                case PixelLocalAttachment::TexelFormat::kR32Sint:
                    format = core::TexelFormat::kR32Sint;
                    break;
                case PixelLocalAttachment::TexelFormat::kR32Uint:
                    format = core::TexelFormat::kR32Uint;
                    break;
                case PixelLocalAttachment::TexelFormat::kR32Float:
                    format = core::TexelFormat::kR32Float;
                    break;
                default:
                    TINT_ICE() << "missing texel format for pixel local storage attachment";
            }
            cfg.pls_member_to_rov_format.Add(member_index, format);
        }
        cfg.rov_group_index = options.pixel_local.group_index;
        data.Add<PixelLocal::Config>(cfg);
        manager.Add<PixelLocal>();
    }

    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<ast::transform::CanonicalizeEntryPointIO>();

    if (options.truncate_interstage_variables) {
        // When interstage_locations is empty, it means there's no user-defined interstage variables
        // being used in the next stage. Still, HLSL compiler register mismatch could happen, if
        // there's builtin inputs used in the next stage. So we still run
        // TruncateInterstageVariables transform.

        // TruncateInterstageVariables itself will skip when interstage_locations matches exactly
        // with the current stage output.

        // Build the config for internal TruncateInterstageVariables transform.
        TruncateInterstageVariables::Config truncate_interstage_variables_cfg;
        truncate_interstage_variables_cfg.interstage_locations =
            std::move(options.interstage_locations);
        manager.Add<TruncateInterstageVariables>();
        data.Add<TruncateInterstageVariables::Config>(std::move(truncate_interstage_variables_cfg));
    }

    // NumWorkgroupsFromUniform must come after CanonicalizeEntryPointIO, as it
    // assumes that num_workgroups builtins only appear as struct members and are
    // only accessed directly via member accessors.
    manager.Add<NumWorkgroupsFromUniform>();
    manager.Add<ast::transform::VectorizeScalarMatrixInitializers>();
    manager.Add<ast::transform::SimplifyPointers>();
    manager.Add<ast::transform::RemovePhonies>();

    // DemoteToHelper must come after CanonicalizeEntryPointIO, PromoteSideEffectsToDecl, and
    // ExpandCompoundAssignment.
    // TODO(crbug.com/42250787): This is only necessary when FXC is being used.
    if (options.compiler == tint::hlsl::writer::Options::Compiler::kFXC) {
        manager.Add<ast::transform::DemoteToHelper>();
    }

    // ArrayLengthFromUniform must come after SimplifyPointers as it assumes that the form of the
    // array length argument is &var.array.
    manager.Add<ast::transform::ArrayLengthFromUniform>();
    // Build the config for the internal ArrayLengthFromUniform transform.
    ast::transform::ArrayLengthFromUniform::Config array_length_from_uniform_cfg(
        BindingPoint{array_length_from_uniform_options.ubo_binding.group,
                     array_length_from_uniform_options.ubo_binding.binding});
    array_length_from_uniform_cfg.bindpoint_to_size_index =
        std::move(array_length_from_uniform_options.bindpoint_to_size_index);
    data.Add<ast::transform::ArrayLengthFromUniform::Config>(
        std::move(array_length_from_uniform_cfg));
    // DecomposeMemoryAccess must come after:
    // * SimplifyPointers, as we cannot take the address of calls to
    //   DecomposeMemoryAccess::Intrinsic and we need to fold away the address-of and dereferences
    //   of `*(&(intrinsic_load()))` expressions.
    // * RemovePhonies, as phonies can be assigned a pointer to a
    //   non-constructible buffer, or dynamic array, which DMA cannot cope with.
    manager.Add<DecomposeMemoryAccess>();
    // CalculateArrayLength must come after DecomposeMemoryAccess, as
    // DecomposeMemoryAccess special-cases the arrayLength() intrinsic, which
    // will be transformed by CalculateArrayLength
    manager.Add<CalculateArrayLength>();
    manager.Add<ast::transform::PromoteInitializersToLet>();

    manager.Add<ast::transform::RemoveContinueInSwitch>();

    manager.Add<ast::transform::AddEmptyEntryPoint>();

    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(
        ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    data.Add<NumWorkgroupsFromUniform::Config>(options.root_constant_binding_point);

    SanitizedResult result;
    ast::transform::DataMap outputs;
    result.program = manager.Run(in, data, outputs);
    if (auto* res = outputs.Get<ast::transform::ArrayLengthFromUniform::Result>()) {
        result.used_array_length_from_uniform_indices = std::move(res->used_size_indices);
    }
    return result;
}

ASTPrinter::ASTPrinter(const Program& program) : builder_(ProgramBuilder::Wrap(program)) {}

ASTPrinter::~ASTPrinter() = default;

bool ASTPrinter::Generate() {
    if (!tint::wgsl::CheckSupportedExtensions(
            "HLSL", builder_.AST(), diagnostics_,
            Vector{
                wgsl::Extension::kChromiumDisableUniformityAnalysis,
                wgsl::Extension::kChromiumExperimentalPixelLocal,
                wgsl::Extension::kChromiumExperimentalPushConstant,
                wgsl::Extension::kChromiumInternalGraphite,
                wgsl::Extension::kClipDistances,
                wgsl::Extension::kF16,
                wgsl::Extension::kDualSourceBlending,
                wgsl::Extension::kSubgroups,
                wgsl::Extension::kSubgroupsF16,
            })) {
        return false;
    }

    const tint::TypeInfo* last_kind = nullptr;
    size_t last_padding_line = 0;

    auto* mod = builder_.Sem().Module();
    for (auto* decl : mod->DependencyOrderedDeclarations()) {
        if (decl->IsAnyOf<ast::Alias, ast::DiagnosticDirective, ast::Enable, ast::Requires,
                          ast::ConstAssert>()) {
            continue;  // These are not emitted.
        }

        // Emit a new line between declarations if the type of declaration has
        // changed, or we're about to emit a function
        auto* kind = &decl->TypeInfo();
        if (current_buffer_->lines.size() != last_padding_line) {
            if (last_kind && (last_kind != kind || decl->Is<ast::Function>())) {
                Line();
                last_padding_line = current_buffer_->lines.size();
            }
        }
        last_kind = kind;

        global_insertion_point_ = current_buffer_->lines.size();

        bool ok = Switch(
            decl,
            [&](const ast::Variable* global) {  //
                return EmitGlobalVariable(global);
            },
            [&](const ast::Struct* str) {
                auto* ty = builder_.Sem().Get(str);
                auto address_space_uses = ty->AddressSpaceUsage();
                if (address_space_uses.Count() !=
                    ((address_space_uses.Contains(core::AddressSpace::kStorage) ? 1u : 0u) +
                     (address_space_uses.Contains(core::AddressSpace::kUniform) ? 1u : 0u))) {
                    // The structure is used as something other than a storage buffer or
                    // uniform buffer, so it needs to be emitted.
                    // Storage buffer are read and written to via a ByteAddressBuffer
                    // instead of true structure.
                    // Structures used as uniform buffer are read from an array of
                    // vectors instead of true structure.
                    return EmitStructType(current_buffer_, ty, str->members);
                }
                return true;
            },
            [&](const ast::Function* func) {
                if (func->IsEntryPoint()) {
                    return EmitEntryPointFunction(func);
                }
                return EmitFunction(func);
            },  //
            TINT_ICE_ON_NO_MATCH);

        if (!ok) {
            return false;
        }
    }

    if (!helpers_.lines.empty()) {
        current_buffer_->Insert(helpers_, 0, 0);
    }

    return true;
}

bool ASTPrinter::EmitDynamicVectorAssignment(const ast::AssignmentStatement* stmt,
                                             const core::type::Vector* vec) {
    auto name = tint::GetOrAdd(dynamic_vector_write_, vec, [&]() -> std::string {
        std::string fn = UniqueIdentifier("set_vector_element");
        {
            auto out = Line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, vec, core::AddressSpace::kUndefined, core::Access::kUndefined,
                                 "vec")) {
                return "";
            }
            out << ", int idx, ";
            if (!EmitTypeAndName(out, vec->Type(), core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            auto out = Line(&helpers_);
            switch (vec->Width()) {
                case 2:
                    out << "vec = (idx.xx == int2(0, 1)) ? val.xx : vec;";
                    break;
                case 3:
                    out << "vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;";
                    break;
                case 4:
                    out << "vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;";
                    break;
                default:
                    TINT_UNREACHABLE() << "invalid vector size " << vec->Width();
            }
        }
        Line(&helpers_) << "}";
        Line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

    auto out = Line();
    out << name << "(";
    if (!EmitExpression(out, ast_access_expr->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, ast_access_expr->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool ASTPrinter::EmitDynamicMatrixVectorAssignment(const ast::AssignmentStatement* stmt,
                                                   const core::type::Matrix* mat) {
    auto name = tint::GetOrAdd(dynamic_matrix_vector_write_, mat, [&]() -> std::string {
        std::string fn = UniqueIdentifier("set_matrix_column");
        {
            auto out = Line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, mat, core::AddressSpace::kUndefined, core::Access::kUndefined,
                                 "mat")) {
                return "";
            }
            out << ", int col, ";
            if (!EmitTypeAndName(out, mat->ColumnType(), core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            Line(&helpers_) << "switch (col) {";
            {
                ScopedIndent si2(&helpers_);
                for (uint32_t i = 0; i < mat->Columns(); ++i) {
                    Line(&helpers_) << "case " << i << ": mat[" << i << "] = val; break;";
                }
            }
            Line(&helpers_) << "}";
        }
        Line(&helpers_) << "}";
        Line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

    auto out = Line();
    out << name << "(";
    if (!EmitExpression(out, ast_access_expr->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, ast_access_expr->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool ASTPrinter::EmitDynamicMatrixScalarAssignment(const ast::AssignmentStatement* stmt,
                                                   const core::type::Matrix* mat) {
    auto* lhs_row_access = stmt->lhs->As<ast::IndexAccessorExpression>();
    auto* lhs_col_access = lhs_row_access->object->As<ast::IndexAccessorExpression>();

    auto name = tint::GetOrAdd(dynamic_matrix_scalar_write_, mat, [&]() -> std::string {
        std::string fn = UniqueIdentifier("set_matrix_scalar");
        {
            auto out = Line(&helpers_);
            out << "void " << fn << "(inout ";
            if (!EmitTypeAndName(out, mat, core::AddressSpace::kUndefined, core::Access::kUndefined,
                                 "mat")) {
                return "";
            }
            out << ", int col, int row, ";
            if (!EmitTypeAndName(out, mat->Type(), core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, "val")) {
                return "";
            }
            out << ") {";
        }
        {
            ScopedIndent si(&helpers_);
            Line(&helpers_) << "switch (col) {";
            {
                ScopedIndent si2(&helpers_);
                for (uint32_t i = 0; i < mat->Columns(); ++i) {
                    Line(&helpers_) << "case " << i << ":";
                    {
                        auto vec_name = "mat[" + std::to_string(i) + "]";
                        ScopedIndent si3(&helpers_);
                        {
                            auto out = Line(&helpers_);
                            switch (mat->Rows()) {
                                case 2:
                                    out << vec_name
                                        << " = (row.xx == int2(0, 1)) ? val.xx : " << vec_name
                                        << ";";
                                    break;
                                case 3:
                                    out << vec_name
                                        << " = (row.xxx == int3(0, 1, 2)) ? val.xxx : " << vec_name
                                        << ";";
                                    break;
                                case 4:
                                    out << vec_name
                                        << " = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : "
                                        << vec_name << ";";
                                    break;
                                default: {
                                    auto* vec = TypeOf(lhs_row_access->object)
                                                    ->UnwrapPtrOrRef()
                                                    ->As<core::type::Vector>();
                                    TINT_UNREACHABLE() << "invalid vector size " << vec->Width();
                                }
                            }
                        }
                        Line(&helpers_) << "break;";
                    }
                }
            }
            Line(&helpers_) << "}";
        }
        Line(&helpers_) << "}";
        Line(&helpers_);
        return fn;
    });

    if (name.empty()) {
        return false;
    }

    auto out = Line();
    out << name << "(";
    if (!EmitExpression(out, lhs_col_access->object)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, lhs_col_access->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, lhs_row_access->index)) {
        return false;
    }
    out << ", ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ");";

    return true;
}

bool ASTPrinter::EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr) {
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    out << "[";

    if (!EmitExpression(out, expr->index)) {
        return false;
    }
    out << "]";

    return true;
}

bool ASTPrinter::EmitBitcastCall(StringStream& out, const ast::CallExpression* call) {
    auto* arg = call->args[0];
    auto* src_type = TypeOf(arg)->UnwrapRef();
    auto* dst_type = TypeOf(call);

    auto* src_el_type = src_type->DeepestElement();
    auto* dst_el_type = dst_type->DeepestElement();

    if (!dst_el_type->IsIntegerScalar() && !dst_el_type->IsFloatScalar()) {
        diagnostics_.AddError(Source{})
            << "Unable to do bitcast to type " << dst_el_type->FriendlyName();
        return false;
    }

    // Handle identity bitcast.
    if (src_type == dst_type) {
        return EmitExpression(out, arg);
    }

    // Handle the f16 types using polyfill functions
    if (src_el_type->Is<core::type::F16>() || dst_el_type->Is<core::type::F16>()) {
        auto f16_bitcast_polyfill = [&]() {
            if (src_el_type->Is<core::type::F16>()) {
                // Source type must be vec2<f16> or vec4<f16>, since type f16 and vec3<f16> can only
                // have identity bitcast.
                auto* src_vec = src_type->As<core::type::Vector>();
                TINT_ASSERT(src_vec);
                TINT_ASSERT(((src_vec->Width() == 2u) || (src_vec->Width() == 4u)));

                // Bitcast f16 types to others by converting the given f16 value to f32 and call
                // f32tof16 to get the bits. This should be safe, because the convertion is precise
                // for finite and infinite f16 value as they are exactly representable by f32, and
                // WGSL spec allow any result if f16 value is NaN.
                return tint::GetOrAdd(
                    bitcast_funcs_, BinaryType{{src_type, dst_type}}, [&]() -> std::string {
                        TextBuffer b;
                        TINT_DEFER(helpers_.Append(b));

                        auto fn_name = UniqueIdentifier(std::string("tint_bitcast_from_f16"));
                        {
                            auto decl = Line(&b);
                            if (!EmitTypeAndName(decl, dst_type, core::AddressSpace::kUndefined,
                                                 core::Access::kUndefined, fn_name)) {
                                return "";
                            }
                            {
                                ScopedParen sp(decl);
                                if (!EmitTypeAndName(decl, src_type, core::AddressSpace::kUndefined,
                                                     core::Access::kUndefined, "src")) {
                                    return "";
                                }
                            }
                            decl << " {";
                        }
                        {
                            ScopedIndent si(&b);
                            {
                                Line(&b) << "uint" << src_vec->Width() << " r = f32tof16(float"
                                         << src_vec->Width() << "(src));";

                                {
                                    auto s = Line(&b);
                                    s << "return as";
                                    if (!EmitType(s, dst_el_type, core::AddressSpace::kUndefined,
                                                  core::Access::kReadWrite, "")) {
                                        return "";
                                    }
                                    s << "(";
                                    switch (src_vec->Width()) {
                                        case 2: {
                                            s << "uint((r.x & 0xffff) | ((r.y & 0xffff) << 16))";
                                            break;
                                        }
                                        case 4: {
                                            s << "uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), "
                                                 "(r.z & 0xffff) | ((r.w & 0xffff) << 16))";
                                            break;
                                        }
                                    }
                                    s << ");";
                                }
                            }
                        }
                        Line(&b) << "}";
                        Line(&b);
                        return fn_name;
                    });
            } else {
                // Destination type must be vec2<f16> or vec4<f16>.
                auto* dst_vec = dst_type->As<core::type::Vector>();
                TINT_ASSERT((dst_vec && ((dst_vec->Width() == 2u) || (dst_vec->Width() == 4u)) &&
                             dst_el_type->Is<core::type::F16>()));
                // Source type must be f32/i32/u32 or vec2<f32/i32/u32>.
                auto* src_vec = src_type->As<core::type::Vector>();
                TINT_ASSERT(
                    (src_type->IsAnyOf<core::type::I32, core::type::U32, core::type::F32>() ||
                     (src_vec && src_vec->Width() == 2u &&
                      src_el_type->IsAnyOf<core::type::I32, core::type::U32, core::type::F32>())));
                std::string src_type_suffix = (src_vec ? "2" : "");

                // Bitcast other types to f16 types by reinterpreting their bits as f16 using
                // f16tof32, and convert the result f32 to f16. This should be safe, because the
                // convertion is precise for finite and infinite f16 result value as they are
                // exactly representable by f32, and WGSL spec allow any result if f16 result value
                // would be NaN.
                return tint::GetOrAdd(
                    bitcast_funcs_, BinaryType{{src_type, dst_type}}, [&]() -> std::string {
                        TextBuffer b;
                        TINT_DEFER(helpers_.Append(b));

                        auto fn_name = UniqueIdentifier(std::string("tint_bitcast_to_f16"));
                        {
                            auto decl = Line(&b);
                            if (!EmitTypeAndName(decl, dst_type, core::AddressSpace::kUndefined,
                                                 core::Access::kUndefined, fn_name)) {
                                return "";
                            }
                            {
                                ScopedParen sp(decl);
                                if (!EmitTypeAndName(decl, src_type, core::AddressSpace::kUndefined,
                                                     core::Access::kUndefined, "src")) {
                                    return "";
                                }
                            }
                            decl << " {";
                        }
                        {
                            ScopedIndent si(&b);
                            {
                                // Convert the source to uint for f16tof32.
                                Line(&b) << "uint" << src_type_suffix << " v = asuint(src);";
                                // Reinterprete the low 16 bits and high 16 bits
                                Line(&b) << "float" << src_type_suffix
                                         << " t_low = f16tof32(v & 0xffff);";
                                Line(&b) << "float" << src_type_suffix
                                         << " t_high = f16tof32((v >> 16) & 0xffff);";
                                // Construct the result f16 vector
                                {
                                    auto s = Line(&b);
                                    s << "return ";
                                    if (!EmitType(s, dst_type, core::AddressSpace::kUndefined,
                                                  core::Access::kReadWrite, "")) {
                                        return "";
                                    }
                                    s << "(";
                                    switch (dst_vec->Width()) {
                                        case 2: {
                                            s << "t_low.x, t_high.x";
                                            break;
                                        }
                                        case 4: {
                                            s << "t_low.x, t_high.x, t_low.y, t_high.y";
                                            break;
                                        }
                                    }
                                    s << ");";
                                }
                            }
                        }
                        Line(&b) << "}";
                        Line(&b);
                        return fn_name;
                    });
            }
        };

        // Get or create the polyfill
        auto fn = f16_bitcast_polyfill();
        if (fn.empty()) {
            return false;
        }
        // Call the polyfill
        out << fn;
        {
            ScopedParen sp(out);
            if (!EmitExpression(out, arg)) {
                return false;
            }
        }

        return true;
    }

    // Otherwise, bitcasting between non-f16 types.
    TINT_ASSERT((!src_el_type->Is<core::type::F16>() && !dst_el_type->Is<core::type::F16>()));
    out << "as";
    if (!EmitType(out, dst_el_type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "")) {
        return false;
    }
    out << "(";
    if (!EmitExpression(out, arg)) {
        return false;
    }
    out << ")";
    return true;
}

bool ASTPrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    if (auto* lhs_access = stmt->lhs->As<ast::IndexAccessorExpression>()) {
        auto validate_obj_not_pointer = [&](const core::type::Type* object_ty) {
            if (DAWN_UNLIKELY(object_ty->Is<core::type::Pointer>())) {
                TINT_ICE() << "lhs of index accessor should not be a pointer. These should have "
                              "been removed by transforms such as SimplifyPointers, "
                              "DecomposeMemoryAccess, and DirectVariableAccess";
            }
            return true;
        };

        // BUG(crbug.com/tint/1333): work around assignment of scalar to matrices
        // with at least one dynamic index
        if (auto* lhs_sub_access = lhs_access->object->As<ast::IndexAccessorExpression>()) {
            const auto* lhs_sub_access_type = TypeOf(lhs_sub_access->object);
            if (!validate_obj_not_pointer(lhs_sub_access_type)) {
                return false;
            }
            if (auto* mat = lhs_sub_access_type->UnwrapRef()->As<core::type::Matrix>()) {
                auto* rhs_row_idx_sem = builder_.Sem().GetVal(lhs_access->index);
                auto* rhs_col_idx_sem = builder_.Sem().GetVal(lhs_sub_access->index);
                if (!rhs_row_idx_sem->ConstantValue() || !rhs_col_idx_sem->ConstantValue()) {
                    return EmitDynamicMatrixScalarAssignment(stmt, mat);
                }
            }
        }
        // BUG(crbug.com/tint/1333): work around assignment of vector to matrices
        // with dynamic indices
        const auto* lhs_access_type = TypeOf(lhs_access->object);
        if (!validate_obj_not_pointer(lhs_access_type)) {
            return false;
        }
        if (auto* mat = lhs_access_type->UnwrapRef()->As<core::type::Matrix>()) {
            auto* lhs_index_sem = builder_.Sem().GetVal(lhs_access->index);
            if (!lhs_index_sem->ConstantValue()) {
                return EmitDynamicMatrixVectorAssignment(stmt, mat);
            }
        }
        // BUG(crbug.com/tint/534): work around assignment to vectors with dynamic
        // indices
        if (auto* vec = lhs_access_type->UnwrapRef()->As<core::type::Vector>()) {
            auto* rhs_sem = builder_.Sem().GetVal(lhs_access->index);
            if (!rhs_sem->ConstantValue()) {
                return EmitDynamicVectorAssignment(stmt, vec);
            }
        }
    }

    auto out = Line();
    if (!EmitExpression(out, stmt->lhs)) {
        return false;
    }
    out << " = ";
    if (!EmitExpression(out, stmt->rhs)) {
        return false;
    }
    out << ";";
    return true;
}

bool ASTPrinter::EmitBinary(StringStream& out, const ast::BinaryExpression* expr) {
    if (expr->op == core::BinaryOp::kLogicalAnd || expr->op == core::BinaryOp::kLogicalOr) {
        auto name = UniqueIdentifier(kTempNamePrefix);

        {
            auto pre = Line();
            pre << "bool " << name << " = ";
            if (!EmitExpression(pre, expr->lhs)) {
                return false;
            }
            pre << ";";
        }

        if (expr->op == core::BinaryOp::kLogicalOr) {
            Line() << "if (!" << name << ") {";
        } else {
            Line() << "if (" << name << ") {";
        }

        {
            ScopedIndent si(this);
            auto pre = Line();
            pre << name << " = ";
            if (!EmitExpression(pre, expr->rhs)) {
                return false;
            }
            pre << ";";
        }

        Line() << "}";

        out << "(" << name << ")";
        return true;
    }

    auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();
    // Multiplying by a matrix requires the use of `mul` in order to get the
    // type of multiply we desire.
    if (expr->op == core::BinaryOp::kMultiply &&
        ((lhs_type->Is<core::type::Vector>() && rhs_type->Is<core::type::Matrix>()) ||
         (lhs_type->Is<core::type::Matrix>() && rhs_type->Is<core::type::Vector>()) ||
         (lhs_type->Is<core::type::Matrix>() && rhs_type->Is<core::type::Matrix>()))) {
        // Matrices are transposed, so swap LHS and RHS.
        out << "mul(";
        if (!EmitExpression(out, expr->rhs)) {
            return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->lhs)) {
            return false;
        }
        out << ")";

        return true;
    }

    ScopedParen sp(out);

    if (!EmitExpression(out, expr->lhs)) {
        return false;
    }
    out << " ";

    switch (expr->op) {
        case core::BinaryOp::kAnd:
            out << "&";
            break;
        case core::BinaryOp::kOr:
            out << "|";
            break;
        case core::BinaryOp::kXor:
            out << "^";
            break;
        case core::BinaryOp::kLogicalAnd:
        case core::BinaryOp::kLogicalOr: {
            // These are both handled above.
            TINT_UNREACHABLE();
        }
        case core::BinaryOp::kEqual:
            out << "==";
            break;
        case core::BinaryOp::kNotEqual:
            out << "!=";
            break;
        case core::BinaryOp::kLessThan:
            out << "<";
            break;
        case core::BinaryOp::kGreaterThan:
            out << ">";
            break;
        case core::BinaryOp::kLessThanEqual:
            out << "<=";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            out << ">=";
            break;
        case core::BinaryOp::kShiftLeft:
            out << "<<";
            break;
        case core::BinaryOp::kShiftRight:
            // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
            // implementation-defined behaviour for negative LHS.  We may have to
            // generate extra code to implement WGSL-specified behaviour for negative
            // LHS.
            out << R"(>>)";
            break;

        case core::BinaryOp::kAdd:
            out << "+";
            break;
        case core::BinaryOp::kSubtract:
            out << "-";
            break;
        case core::BinaryOp::kMultiply:
            out << "*";
            break;
        case core::BinaryOp::kDivide:
            out << "/";
            break;
        case core::BinaryOp::kModulo:
            out << "%";
            break;
    }
    out << " ";

    if (!EmitExpression(out, expr->rhs)) {
        return false;
    }

    return true;
}

bool ASTPrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }
    }
    return true;
}

bool ASTPrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    return EmitStatements(stmts);
}

bool ASTPrinter::EmitBlock(const ast::BlockStatement* stmt) {
    Line() << "{";
    if (!EmitStatementsWithIndent(stmt->statements)) {
        return false;
    }
    Line() << "}";
    return true;
}

bool ASTPrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "break;";
    return true;
}

bool ASTPrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = Line();
    out << "if (";
    if (!EmitExpression(out, b->condition)) {
        return false;
    }
    out << ") { break; }";
    return true;
}

bool ASTPrinter::EmitCall(StringStream& out, const ast::CallExpression* expr) {
    auto* call = builder_.Sem().Get<sem::Call>(expr);
    auto* target = call->Target();
    return Switch(
        target,  //
        [&](const sem::Function* func) { return EmitFunctionCall(out, call, func); },
        [&](const sem::BuiltinFn* builtin) { return EmitBuiltinCall(out, call, builtin); },
        [&](const sem::ValueConversion* conv) { return EmitValueConversion(out, call, conv); },
        [&](const sem::ValueConstructor* ctor) { return EmitValueConstructor(out, call, ctor); },
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitFunctionCall(StringStream& out,
                                  const sem::Call* call,
                                  const sem::Function* func) {
    auto* expr = call->Declaration();

    if (ast::HasAttribute<CalculateArrayLength::BufferSizeIntrinsic>(
            func->Declaration()->attributes)) {
        // Special function generated by the CalculateArrayLength transform for
        // calling X.GetDimensions(Y)
        if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
            return false;
        }
        out << ".GetDimensions(";
        if (!EmitExpression(out, call->Arguments()[1]->Declaration())) {
            return false;
        }
        out << ")";
        return true;
    }

    if (auto* intrinsic =
            ast::GetAttribute<DecomposeMemoryAccess::Intrinsic>(func->Declaration()->attributes)) {
        switch (intrinsic->address_space) {
            case core::AddressSpace::kUniform:
                return EmitUniformBufferAccess(out, expr, intrinsic);
            case core::AddressSpace::kStorage:
                if (!intrinsic->IsAtomic()) {
                    return EmitStorageBufferAccess(out, expr, intrinsic);
                }
                break;
            default:
                TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic address space:"
                                   << intrinsic->address_space;
        }
    }

    if (auto* wave_intrinsic =
            ast::GetAttribute<ast::transform::CanonicalizeEntryPointIO::HLSLWaveIntrinsic>(
                func->Declaration()->attributes)) {
        switch (wave_intrinsic->op) {
            case ast::transform::CanonicalizeEntryPointIO::HLSLWaveIntrinsic::Op::kWaveGetLaneCount:
                out << "WaveGetLaneCount()";
                return true;
            case ast::transform::CanonicalizeEntryPointIO::HLSLWaveIntrinsic::Op::kWaveGetLaneIndex:
                out << "WaveGetLaneIndex()";
                return true;
        }
    }

    out << func->Declaration()->name->symbol.Name() << "(";

    bool first = true;
    for (auto* arg : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg->Declaration())) {
            return false;
        }
    }

    out << ")";
    return true;
}

bool ASTPrinter::EmitBuiltinCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    const auto type = builtin->Fn();

    auto* expr = call->Declaration();
    if (builtin->IsTexture()) {
        return EmitTextureCall(out, call, builtin);
    }
    if (type == wgsl::BuiltinFn::kBitcast) {
        return EmitBitcastCall(out, expr);
    }
    if (type == wgsl::BuiltinFn::kSelect) {
        return EmitSelectCall(out, expr);
    }
    if (type == wgsl::BuiltinFn::kModf) {
        return EmitModfCall(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kFrexp) {
        return EmitFrexpCall(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kDegrees) {
        return EmitDegreesCall(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kRadians) {
        return EmitRadiansCall(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kSign) {
        return EmitSignCall(out, call, builtin);
    }
    if (type == wgsl::BuiltinFn::kQuantizeToF16) {
        return EmitQuantizeToF16Call(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kTrunc) {
        return EmitTruncCall(out, expr, builtin);
    }
    if (builtin->IsDataPacking()) {
        return EmitDataPackingCall(out, expr, builtin);
    }
    if (builtin->IsDataUnpacking()) {
        return EmitDataUnpackingCall(out, expr, builtin);
    }
    if (builtin->IsBarrier()) {
        return EmitBarrierCall(out, builtin);
    }
    if (builtin->IsAtomic()) {
        return EmitWorkgroupAtomicCall(out, expr, builtin);
    }
    if (builtin->IsPacked4x8IntegerDotProductBuiltin()) {
        return EmitPacked4x8IntegerDotProductBuiltinCall(out, expr, builtin);
    }
    if (type == wgsl::BuiltinFn::kSubgroupShuffleXor ||
        type == wgsl::BuiltinFn::kSubgroupShuffleUp ||
        type == wgsl::BuiltinFn::kSubgroupShuffleDown) {
        return EmitSubgroupShuffleBuiltinCall(out, expr, builtin);
    }

    if (type == wgsl::BuiltinFn::kSubgroupInclusiveAdd ||
        type == wgsl::BuiltinFn::kSubgroupInclusiveMul) {
        return EmitSubgroupInclusiveBuiltinCall(out, expr, builtin);
    }
    auto name = generate_builtin_name(builtin);
    if (name.empty()) {
        return false;
    }

    // Handle single argument builtins that only accept and return uint (not int overload).
    // For count bits and reverse bits, we need to explicitly cast the return value (we also cast
    // the arg for good measure). See crbug.com/tint/1550.
    // For the following subgroup builtins, the lack of support may be a bug in DXC. See
    // github.com/microsoft/DirectXShaderCompiler/issues/6850.
    if (type == wgsl::BuiltinFn::kCountOneBits || type == wgsl::BuiltinFn::kReverseBits ||
        type == wgsl::BuiltinFn::kSubgroupAnd || type == wgsl::BuiltinFn::kSubgroupOr ||
        type == wgsl::BuiltinFn::kSubgroupXor) {
        auto* arg = call->Arguments()[0];
        auto* argType = arg->Type()->UnwrapRef();
        if (argType->IsSignedIntegerScalarOrVector()) {
            // Bitcast of literal int vectors fails in DXC so extract arg to a var. See
            // github.com/microsoft/DirectXShaderCompiler/issues/6851.
            if (argType->IsSignedIntegerVector() &&
                arg->Stage() == core::EvaluationStage::kConstant) {
                auto varName = UniqueIdentifier(kTempNamePrefix);
                auto pre = Line();
                if (!EmitTypeAndName(pre, argType, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, varName)) {
                    return false;
                }
                pre << " = ";
                if (!EmitExpression(pre, arg->Declaration())) {
                    return false;
                }
                pre << ";";
                out << "asint(" << name << "(asuint(" << varName << ")))";
                return true;
            }

            out << "asint(" << name << "(asuint(";
            if (!EmitExpression(out, arg->Declaration())) {
                return false;
            }
            out << ")))";
            return true;
        }
    }

    out << name << "(";

    bool first = true;
    for (auto* arg : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, arg->Declaration())) {
            return false;
        }
    }

    out << ")";

    return true;
}

bool ASTPrinter::EmitValueConversion(StringStream& out,
                                     const sem::Call* call,
                                     const sem::ValueConversion* conv) {
    if (!EmitType(out, conv->Target(), core::AddressSpace::kUndefined, core::Access::kReadWrite,
                  "")) {
        return false;
    }
    out << "(";

    if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
        return false;
    }

    out << ")";
    return true;
}

bool ASTPrinter::EmitValueConstructor(StringStream& out,
                                      const sem::Call* call,
                                      const sem::ValueConstructor* ctor) {
    auto* type = call->Type();

    // If the value constructor arguments are empty then we need to construct with the zero value
    // for all components.
    if (call->Arguments().IsEmpty()) {
        return EmitZeroValue(out, type);
    }

    // Single parameter matrix initializers must be identity initializer.
    // It could also be conversions between f16 and f32 matrix when f16 is properly supported.
    if (type->Is<core::type::Matrix>() && call->Arguments().Length() == 1) {
        if (!ctor->Parameters()[0]->Type()->UnwrapRef()->IsFloatMatrix()) {
            TINT_UNREACHABLE()
                << "found a single-parameter matrix initializer that is not identity initializer";
        }
    }

    bool brackets = type->IsAnyOf<core::type::Array, core::type::Struct>();

    // For single-value vector initializers, swizzle the scalar to the right
    // vector dimension using .x
    const bool is_single_value_vector_init =
        type->IsScalarVector() && call->Arguments().Length() == 1 &&
        ctor->Parameters()[0]->Type()->Is<core::type::Scalar>();

    if (brackets) {
        out << "{";
    } else {
        if (!EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite, "")) {
            return false;
        }
        out << "(";
    }

    if (is_single_value_vector_init) {
        out << "(";
    }

    bool first = true;
    for (auto* e : call->Arguments()) {
        if (!first) {
            out << ", ";
        }
        first = false;

        if (!EmitExpression(out, e->Declaration())) {
            return false;
        }
    }

    if (is_single_value_vector_init) {
        out << ")." << std::string(type->As<core::type::Vector>()->Width(), 'x');
    }

    out << (brackets ? "}" : ")");
    return true;
}

bool ASTPrinter::EmitUniformBufferAccess(StringStream& out,
                                         const ast::CallExpression* expr,
                                         const DecomposeMemoryAccess::Intrinsic* intrinsic) {
    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();
    auto* const offset = expr->args[0];

    // offset in bytes
    uint32_t scalar_offset_bytes = 0;
    // offset in uint (4 bytes)
    uint32_t scalar_offset_index = 0;
    // expression to calculate offset in bytes
    std::string scalar_offset_bytes_expr;
    // expression to calculate offset in uint, by dividing scalar_offset_bytes_expr by 4
    std::string scalar_offset_index_expr;
    // expression to calculate offset in uint, independently
    std::string scalar_offset_index_unified_expr;

    // If true, use scalar_offset_index, otherwise use scalar_offset_index_expr
    bool scalar_offset_constant = false;

    if (auto* val = builder_.Sem().GetVal(offset)->ConstantValue()) {
        TINT_ASSERT(val->Type()->Is<core::type::U32>());
        scalar_offset_bytes = static_cast<uint32_t>(val->ValueAs<AInt>());
        scalar_offset_index = scalar_offset_bytes / 4;  // bytes -> scalar index
        scalar_offset_constant = true;
    }

    // If true, scalar_offset_bytes or scalar_offset_bytes_expr should be used, otherwise only use
    // scalar_offset_index or scalar_offset_index_unified_expr. Currently only loading f16 scalar
    // require using offset in bytes.
    const bool need_offset_in_bytes =
        intrinsic->type == DecomposeMemoryAccess::Intrinsic::DataType::kF16;

    if (!scalar_offset_constant) {
        // UBO offset not compile-time known.
        // Calculate the scalar offset into a temporary.
        if (need_offset_in_bytes) {
            scalar_offset_bytes_expr = UniqueIdentifier("scalar_offset_bytes");
            scalar_offset_index_expr = UniqueIdentifier("scalar_offset_index");
            {
                auto pre = Line();
                pre << "const uint " << scalar_offset_bytes_expr << " = (";
                if (!EmitExpression(pre, offset)) {
                    return false;
                }
                pre << ");";
            }
            Line() << "const uint " << scalar_offset_index_expr << " = " << scalar_offset_bytes_expr
                   << " / 4;";
        } else {
            scalar_offset_index_unified_expr = UniqueIdentifier("scalar_offset");
            auto pre = Line();
            pre << "const uint " << scalar_offset_index_unified_expr << " = (";
            if (!EmitExpression(pre, offset)) {
                return false;
            }
            pre << ") / 4;";
        }
    }

    const char swizzle[] = {'x', 'y', 'z', 'w'};

    using Op = DecomposeMemoryAccess::Intrinsic::Op;
    using DataType = DecomposeMemoryAccess::Intrinsic::DataType;
    switch (intrinsic->op) {
        case Op::kLoad: {
            auto cast = [&](const char* to, auto&& load) {
                out << to << "(";
                auto result = load();
                out << ")";
                return result;
            };
            auto load_u32_to = [&](StringStream& target) {
                target << buffer;
                if (scalar_offset_constant) {
                    target << "[" << (scalar_offset_index / 4) << "]."
                           << swizzle[scalar_offset_index & 3];
                } else {
                    target << "[" << scalar_offset_index_unified_expr << " / 4]["
                           << scalar_offset_index_unified_expr << " % 4]";
                }
                return true;
            };
            auto load_u32 = [&] { return load_u32_to(out); };
            // Has a minimum alignment of 8 bytes, so is either .xy or .zw
            auto load_vec2_u32_to = [&](StringStream& target) {
                if (scalar_offset_constant) {
                    target << buffer << "[" << (scalar_offset_index / 4) << "]"
                           << ((scalar_offset_index & 2) == 0 ? ".xy" : ".zw");
                } else {
                    std::string ubo_load = UniqueIdentifier("ubo_load");
                    {
                        auto pre = Line();
                        pre << "uint4 " << ubo_load << " = " << buffer << "["
                            << scalar_offset_index_unified_expr << " / 4];";
                    }
                    target << "((" << scalar_offset_index_unified_expr << " & 2) ? " << ubo_load
                           << ".zw : " << ubo_load << ".xy)";
                }
                return true;
            };
            auto load_vec2_u32 = [&] { return load_vec2_u32_to(out); };
            // vec4 has a minimum alignment of 16 bytes, easiest case
            auto load_vec4_u32 = [&] {
                out << buffer;
                if (scalar_offset_constant) {
                    out << "[" << (scalar_offset_index / 4) << "]";
                } else {
                    out << "[" << scalar_offset_index_unified_expr << " / 4]";
                }
                return true;
            };
            // vec3 has a minimum alignment of 16 bytes, so is just a .xyz swizzle
            auto load_vec3_u32 = [&] {
                if (!load_vec4_u32()) {
                    return false;
                }
                out << ".xyz";
                return true;
            };
            auto load_scalar_f16 = [&] {
                // offset bytes = 4k,   ((buffer[index].x) & 0xFFFF)
                // offset bytes = 4k+2, ((buffer[index].x >> 16) & 0xFFFF)
                out << "float16_t(f16tof32(((" << buffer;
                if (scalar_offset_constant) {
                    out << "[" << (scalar_offset_index / 4) << "]."
                        << swizzle[scalar_offset_index & 3];
                    // WGSL spec ensure little endian memory layout.
                    if (scalar_offset_bytes % 4 == 0) {
                        out << ") & 0xFFFF)";
                    } else {
                        out << " >> 16) & 0xFFFF)";
                    }
                } else {
                    out << "[" << scalar_offset_index_expr << " / 4][" << scalar_offset_index_expr
                        << " % 4] >> (" << scalar_offset_bytes_expr
                        << " % 4 == 0 ? 0 : 16)) & 0xFFFF)";
                }
                out << "))";
                return true;
            };
            auto load_vec2_f16 = [&] {
                // vec2<f16> is aligned to 4 bytes
                // Preclude code load the vec2<f16> data as a uint:
                //     uint ubo_load = buffer[id0][id1];
                // Loading code convert it to vec2<f16>:
                //     vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)),
                //     float16_t(f16tof32(ubo_load >> 16)))
                std::string ubo_load = UniqueIdentifier("ubo_load");
                {
                    auto pre = Line();
                    // Load the 4 bytes f16 vector as an uint
                    pre << "uint " << ubo_load << " = ";
                    if (!load_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                out << "vector<float16_t, 2>(float16_t(f16tof32(" << ubo_load
                    << " & 0xFFFF)), float16_t(f16tof32(" << ubo_load << " >> 16)))";
                return true;
            };
            auto load_vec3_f16 = [&] {
                // vec3<f16> is aligned to 8 bytes
                // Preclude code load the vec3<f16> data as uint2 and convert its elements to
                // float16_t:
                //     uint2 ubo_load = buffer[id0].xy;
                //     /* The low 8 bits of two uint are the x and z elements of vec3<f16> */
                //     vector<float16_t> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load &
                //     0xFFFF));
                //     /* The high 8 bits of first uint is the y element of vec3<f16> */
                //     float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
                // Loading code convert it to vec3<f16>:
                //     vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1])
                std::string ubo_load = UniqueIdentifier("ubo_load");
                std::string ubo_load_xz = UniqueIdentifier(ubo_load + "_xz");
                std::string ubo_load_y = UniqueIdentifier(ubo_load + "_y");
                {
                    auto pre = Line();
                    // Load the 8 bytes uint2 with the f16 vector at lower 6 bytes
                    pre << "uint2 " << ubo_load << " = ";
                    if (!load_vec2_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                {
                    auto pre = Line();
                    pre << "vector<float16_t, 2> " << ubo_load_xz
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " & 0xFFFF));";
                }
                {
                    auto pre = Line();
                    pre << "float16_t " << ubo_load_y << " = f16tof32(" << ubo_load
                        << "[0] >> 16);";
                }
                out << "vector<float16_t, 3>(" << ubo_load_xz << "[0], " << ubo_load_y << ", "
                    << ubo_load_xz << "[1])";
                return true;
            };
            auto load_vec4_f16 = [&] {
                // vec4<f16> is aligned to 8 bytes
                // Preclude code load the vec4<f16> data as uint2 and convert its elements to
                // float16_t:
                //     uint2 ubo_load = buffer[id0].xy;
                //     /* The low 8 bits of two uint are the x and z elements of vec4<f16> */
                //     vector<float16_t> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load &
                //     0xFFFF));
                //     /* The high 8 bits of two uint are the y and w elements of vec4<f16> */
                //     vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >>
                //     16));
                // Loading code convert it to vec4<f16>:
                //     vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1],
                //     ubo_load_yw[1])
                std::string ubo_load = UniqueIdentifier("ubo_load");
                std::string ubo_load_xz = UniqueIdentifier(ubo_load + "_xz");
                std::string ubo_load_yw = UniqueIdentifier(ubo_load + "_yw");
                {
                    auto pre = Line();
                    // Load the 8 bytes f16 vector as an uint2
                    pre << "uint2 " << ubo_load << " = ";
                    if (!load_vec2_u32_to(pre)) {
                        return false;
                    }
                    pre << ";";
                }
                {
                    auto pre = Line();
                    pre << "vector<float16_t, 2> " << ubo_load_xz
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " & 0xFFFF));";
                }
                {
                    auto pre = Line();
                    pre << "vector<float16_t, 2> " << ubo_load_yw
                        << " = vector<float16_t, 2>(f16tof32(" << ubo_load << " >> 16));";
                }
                out << "vector<float16_t, 4>(" << ubo_load_xz << "[0], " << ubo_load_yw << "[0], "
                    << ubo_load_xz << "[1], " << ubo_load_yw << "[1])";
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return load_u32();
                case DataType::kF32:
                    return cast("asfloat", load_u32);
                case DataType::kI32:
                    return cast("asint", load_u32);
                case DataType::kF16:
                    return load_scalar_f16();
                case DataType::kVec2U32:
                    return load_vec2_u32();
                case DataType::kVec2F32:
                    return cast("asfloat", load_vec2_u32);
                case DataType::kVec2I32:
                    return cast("asint", load_vec2_u32);
                case DataType::kVec2F16:
                    return load_vec2_f16();
                case DataType::kVec3U32:
                    return load_vec3_u32();
                case DataType::kVec3F32:
                    return cast("asfloat", load_vec3_u32);
                case DataType::kVec3I32:
                    return cast("asint", load_vec3_u32);
                case DataType::kVec3F16:
                    return load_vec3_f16();
                case DataType::kVec4U32:
                    return load_vec4_u32();
                case DataType::kVec4F32:
                    return cast("asfloat", load_vec4_u32);
                case DataType::kVec4I32:
                    return cast("asint", load_vec4_u32);
                case DataType::kVec4F16:
                    return load_vec4_f16();
            }
            TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                               << static_cast<int>(intrinsic->type);
        }
        default:
            break;
    }
    TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
                       << static_cast<int>(intrinsic->op);
}

bool ASTPrinter::EmitStorageBufferAccess(StringStream& out,
                                         const ast::CallExpression* expr,
                                         const DecomposeMemoryAccess::Intrinsic* intrinsic) {
    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();
    auto* const offset = expr->args[0];
    auto* const value = expr->args.Length() > 1 ? expr->args[1] : nullptr;

    using Op = DecomposeMemoryAccess::Intrinsic::Op;
    using DataType = DecomposeMemoryAccess::Intrinsic::DataType;
    switch (intrinsic->op) {
        case Op::kLoad: {
            auto load = [&](const char* cast, int n) {
                if (cast) {
                    out << cast << "(";
                }
                out << buffer << ".Load";
                if (n > 1) {
                    out << n;
                }
                ScopedParen sp(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                if (cast) {
                    out << ")";
                }
                return true;
            };
            // Templated load used for f16 types, requires SM6.2 or higher and DXC
            // Used by loading f16 types, e.g. for f16 type, set type parameter to "float16_t"
            // to emit `buffer.Load<float16_t>(offset)`.
            auto templated_load = [&](const char* type) {
                out << buffer << ".Load<" << type << ">";  // templated load
                ScopedParen sp(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return load(nullptr, 1);
                case DataType::kF32:
                    return load("asfloat", 1);
                case DataType::kI32:
                    return load("asint", 1);
                case DataType::kF16:
                    return templated_load("float16_t");
                case DataType::kVec2U32:
                    return load(nullptr, 2);
                case DataType::kVec2F32:
                    return load("asfloat", 2);
                case DataType::kVec2I32:
                    return load("asint", 2);
                case DataType::kVec2F16:
                    return templated_load("vector<float16_t, 2> ");
                case DataType::kVec3U32:
                    return load(nullptr, 3);
                case DataType::kVec3F32:
                    return load("asfloat", 3);
                case DataType::kVec3I32:
                    return load("asint", 3);
                case DataType::kVec3F16:
                    return templated_load("vector<float16_t, 3> ");
                case DataType::kVec4U32:
                    return load(nullptr, 4);
                case DataType::kVec4F32:
                    return load("asfloat", 4);
                case DataType::kVec4I32:
                    return load("asint", 4);
                case DataType::kVec4F16:
                    return templated_load("vector<float16_t, 4> ");
            }
            TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                               << static_cast<int>(intrinsic->type);
        }

        case Op::kStore: {
            auto store = [&](int n) {
                out << buffer << ".Store";
                if (n > 1) {
                    out << n;
                }
                ScopedParen sp1(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                out << ", asuint";
                ScopedParen sp2(out);
                if (!EmitTextureOrStorageBufferCallArgExpression(out, value)) {
                    return false;
                }
                return true;
            };
            // Templated stored used for f16 types, requires SM6.2 or higher and DXC
            // Used by storing f16 types, e.g. for f16 type, set type parameter to "float16_t"
            // to emit `buffer.Store<float16_t>(offset)`.
            auto templated_store = [&](const char* type) {
                out << buffer << ".Store<" << type << ">";  // templated store
                ScopedParen sp1(out);
                if (!EmitExpression(out, offset)) {
                    return false;
                }
                out << ", ";
                if (!EmitExpression(out, value)) {
                    return false;
                }
                return true;
            };
            switch (intrinsic->type) {
                case DataType::kU32:
                    return store(1);
                case DataType::kF32:
                    return store(1);
                case DataType::kI32:
                    return store(1);
                case DataType::kF16:
                    return templated_store("float16_t");
                case DataType::kVec2U32:
                    return store(2);
                case DataType::kVec2F32:
                    return store(2);
                case DataType::kVec2I32:
                    return store(2);
                case DataType::kVec2F16:
                    return templated_store("vector<float16_t, 2> ");
                case DataType::kVec3U32:
                    return store(3);
                case DataType::kVec3F32:
                    return store(3);
                case DataType::kVec3I32:
                    return store(3);
                case DataType::kVec3F16:
                    return templated_store("vector<float16_t, 3> ");
                case DataType::kVec4U32:
                    return store(4);
                case DataType::kVec4F32:
                    return store(4);
                case DataType::kVec4I32:
                    return store(4);
                case DataType::kVec4F16:
                    return templated_store("vector<float16_t, 4> ");
            }
            TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
                               << static_cast<int>(intrinsic->type);
        }
        default:
            // Break out to error case below
            // Note that atomic intrinsics are generated as functions.
            break;
    }

    TINT_UNREACHABLE() << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
                       << static_cast<int>(intrinsic->op);
}

bool ASTPrinter::EmitStorageAtomicIntrinsic(const ast::Function* func,
                                            const DecomposeMemoryAccess::Intrinsic* intrinsic) {
    using Op = DecomposeMemoryAccess::Intrinsic::Op;

    const sem::Function* sem_func = builder_.Sem().Get(func);
    auto* result_ty = sem_func->ReturnType();
    const auto name = func->name->symbol.Name();
    auto& buf = *current_buffer_;

    auto const buffer = intrinsic->Buffer()->identifier->symbol.Name();

    auto rmw = [&](const char* hlsl) -> bool {
        {
            auto fn = Line(&buf);
            if (!EmitTypeAndName(fn, result_ty, core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, name)) {
                return false;
            }
            fn << "(uint offset, ";
            if (!EmitTypeAndName(fn, result_ty, core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, "value")) {
                return false;
            }
            fn << ") {";
        }

        buf.IncrementIndent();
        TINT_DEFER({
            buf.DecrementIndent();
            Line(&buf) << "}";
            Line(&buf);
        });

        {
            auto l = Line(&buf);
            if (!EmitTypeAndName(l, result_ty, core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, "original_value")) {
                return false;
            }
            l << " = 0;";
        }
        {
            auto l = Line(&buf);
            l << buffer << "." << hlsl << "(offset, ";
            if (intrinsic->op == Op::kAtomicSub) {
                l << "-";
            }
            l << "value, original_value);";
        }
        Line(&buf) << "return original_value;";
        return true;
    };

    switch (intrinsic->op) {
        case Op::kAtomicAdd:
            return rmw("InterlockedAdd");

        case Op::kAtomicSub:
            // Use add with the operand negated.
            return rmw("InterlockedAdd");

        case Op::kAtomicMax:
            return rmw("InterlockedMax");

        case Op::kAtomicMin:
            return rmw("InterlockedMin");

        case Op::kAtomicAnd:
            return rmw("InterlockedAnd");

        case Op::kAtomicOr:
            return rmw("InterlockedOr");

        case Op::kAtomicXor:
            return rmw("InterlockedXor");

        case Op::kAtomicExchange:
            return rmw("InterlockedExchange");

        case Op::kAtomicLoad: {
            // HLSL does not have an InterlockedLoad, so we emulate it with
            // InterlockedOr using 0 as the OR value
            {
                auto fn = Line(&buf);
                if (!EmitTypeAndName(fn, result_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, name)) {
                    return false;
                }
                fn << "(uint offset) {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                Line(&buf) << "}";
                Line(&buf);
            });

            {
                auto l = Line(&buf);
                if (!EmitTypeAndName(l, result_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "value")) {
                    return false;
                }
                l << " = 0;";
            }

            Line(&buf) << buffer << ".InterlockedOr(offset, 0, value);";
            Line(&buf) << "return value;";
            return true;
        }
        case Op::kAtomicStore: {
            auto* const value_ty = sem_func->Parameters()[1]->Type()->UnwrapRef();
            // HLSL does not have an InterlockedStore, so we emulate it with
            // InterlockedExchange and discard the returned value
            {
                auto fn = Line(&buf);
                fn << "void " << name << "(uint offset, ";
                if (!EmitTypeAndName(fn, value_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "value")) {
                    return false;
                }
                fn << ") {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                Line(&buf) << "}";
                Line(&buf);
            });

            {
                auto l = Line(&buf);
                if (!EmitTypeAndName(l, value_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "ignored")) {
                    return false;
                }
                l << ";";
            }
            Line(&buf) << buffer << ".InterlockedExchange(offset, value, ignored);";
            return true;
        }
        case Op::kAtomicCompareExchangeWeak: {
            if (!EmitStructType(&helpers_, result_ty->As<core::type::Struct>())) {
                return false;
            }

            auto* const value_ty = sem_func->Parameters()[1]->Type()->UnwrapRef();
            // NOTE: We don't need to emit the return type struct here as DecomposeMemoryAccess
            // already added it to the AST, and it should have already been emitted by now.
            {
                auto fn = Line(&buf);
                if (!EmitTypeAndName(fn, result_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, name)) {
                    return false;
                }
                fn << "(uint offset, ";
                if (!EmitTypeAndName(fn, value_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "compare")) {
                    return false;
                }
                fn << ", ";
                if (!EmitTypeAndName(fn, value_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "value")) {
                    return false;
                }
                fn << ") {";
            }

            buf.IncrementIndent();
            TINT_DEFER({
                buf.DecrementIndent();
                Line(&buf) << "}";
                Line(&buf);
            });

            {  // T result = {0};
                auto l = Line(&buf);
                if (!EmitTypeAndName(l, result_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, "result")) {
                    return false;
                }
                l << "=";
                if (!EmitZeroValue(l, result_ty)) {
                    return false;
                }
                l << ";";
            }

            Line(&buf) << buffer
                       << ".InterlockedCompareExchange(offset, compare, value, result.old_value);";
            Line(&buf) << "result.exchanged = result.old_value == compare;";
            Line(&buf) << "return result;";

            return true;
        }
        default:
            break;
    }

    TINT_UNREACHABLE() << "unsupported atomic DecomposeMemoryAccess::Intrinsic::Op: "
                       << static_cast<int>(intrinsic->op);
}

bool ASTPrinter::EmitWorkgroupAtomicCall(StringStream& out,
                                         const ast::CallExpression* expr,
                                         const sem::BuiltinFn* builtin) {
    std::string result = UniqueIdentifier("atomic_result");

    if (!builtin->ReturnType()->Is<core::type::Void>()) {
        auto pre = Line();
        if (!EmitTypeAndName(pre, builtin->ReturnType(), core::AddressSpace::kUndefined,
                             core::Access::kUndefined, result)) {
            return false;
        }
        pre << " = ";
        if (!EmitZeroValue(pre, builtin->ReturnType())) {
            return false;
        }
        pre << ";";
    }

    auto call = [&](const char* name) {
        auto pre = Line();
        pre << name;

        {
            ScopedParen sp(pre);
            for (size_t i = 0; i < expr->args.Length(); i++) {
                auto* arg = expr->args[i];
                if (i > 0) {
                    pre << ", ";
                }
                if (!EmitExpression(pre, arg)) {
                    return false;
                }
            }

            pre << ", " << result;
        }

        pre << ";";

        out << result;
        return true;
    };

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAtomicLoad: {
            // HLSL does not have an InterlockedLoad, so we emulate it with
            // InterlockedOr using 0 as the OR value
            auto pre = Line();
            pre << "InterlockedOr";
            {
                ScopedParen sp(pre);
                if (!EmitExpression(pre, expr->args[0])) {
                    return false;
                }
                pre << ", 0, " << result;
            }
            pre << ";";

            out << result;
            return true;
        }
        case wgsl::BuiltinFn::kAtomicStore: {
            // HLSL does not have an InterlockedStore, so we emulate it with
            // InterlockedExchange and discard the returned value
            {  // T result = 0;
                auto pre = Line();
                auto* value_ty = builtin->Parameters()[1]->Type()->UnwrapRef();
                if (!EmitTypeAndName(pre, value_ty, core::AddressSpace::kUndefined,
                                     core::Access::kUndefined, result)) {
                    return false;
                }
                pre << " = ";
                if (!EmitZeroValue(pre, value_ty)) {
                    return false;
                }
                pre << ";";
            }

            out << "InterlockedExchange";
            {
                ScopedParen sp(out);
                if (!EmitExpression(out, expr->args[0])) {
                    return false;
                }
                out << ", ";
                if (!EmitExpression(out, expr->args[1])) {
                    return false;
                }
                out << ", " << result;
            }
            return true;
        }
        case wgsl::BuiltinFn::kAtomicCompareExchangeWeak: {
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>())) {
                return false;
            }

            auto* dest = expr->args[0];
            auto* compare_value = expr->args[1];
            auto* value = expr->args[2];

            std::string compare = UniqueIdentifier("atomic_compare_value");

            {  // T compare_value = <compare_value>;
                auto pre = Line();
                if (!EmitTypeAndName(pre, TypeOf(compare_value)->UnwrapRef(),
                                     core::AddressSpace::kUndefined, core::Access::kUndefined,
                                     compare)) {
                    return false;
                }
                pre << " = ";
                if (!EmitExpression(pre, compare_value)) {
                    return false;
                }
                pre << ";";
            }

            {  // InterlockedCompareExchange(dst, compare, value, result.old_value);
                auto pre = Line();
                pre << "InterlockedCompareExchange";
                {
                    ScopedParen sp(pre);
                    if (!EmitExpression(pre, dest)) {
                        return false;
                    }
                    pre << ", " << compare << ", ";
                    if (!EmitExpression(pre, value)) {
                        return false;
                    }
                    pre << ", " << result << ".old_value";
                }
                pre << ";";
            }

            // result.exchanged = result.old_value == compare;
            Line() << result << ".exchanged = " << result << ".old_value == " << compare << ";";

            out << result;
            return true;
        }

        case wgsl::BuiltinFn::kAtomicAdd:
            return call("InterlockedAdd");

        case wgsl::BuiltinFn::kAtomicSub: {
            auto pre = Line();
            // Sub uses InterlockedAdd with the operand negated.
            pre << "InterlockedAdd";
            {
                ScopedParen sp(pre);
                TINT_ASSERT(expr->args.Length() == 2);

                if (!EmitExpression(pre, expr->args[0])) {
                    return false;
                }
                pre << ", -";
                {
                    ScopedParen argSP(pre);
                    if (!EmitExpression(pre, expr->args[1])) {
                        return false;
                    }
                }

                pre << ", " << result;
            }

            pre << ";";

            out << result;
        }
            return true;

        case wgsl::BuiltinFn::kAtomicMax:
            return call("InterlockedMax");

        case wgsl::BuiltinFn::kAtomicMin:
            return call("InterlockedMin");

        case wgsl::BuiltinFn::kAtomicAnd:
            return call("InterlockedAnd");

        case wgsl::BuiltinFn::kAtomicOr:
            return call("InterlockedOr");

        case wgsl::BuiltinFn::kAtomicXor:
            return call("InterlockedXor");

        case wgsl::BuiltinFn::kAtomicExchange:
            return call("InterlockedExchange");

        default:
            break;
    }

    TINT_UNREACHABLE() << "unsupported atomic builtin: " << builtin->Fn();
}

bool ASTPrinter::EmitSelectCall(StringStream& out, const ast::CallExpression* expr) {
    auto* expr_false = expr->args[0];
    auto* expr_true = expr->args[1];
    auto* expr_cond = expr->args[2];
    ScopedParen paren(out);
    if (!EmitExpression(out, expr_cond)) {
        return false;
    }

    out << " ? ";

    if (!EmitExpression(out, expr_true)) {
        return false;
    }

    out << " : ";

    if (!EmitExpression(out, expr_false)) {
        return false;
    }

    return true;
}

bool ASTPrinter::EmitModfCall(StringStream& out,
                              const ast::CallExpression* expr,
                              const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<core::type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>())) {
                return false;
            }

            {
                auto l = Line(b);
                if (!EmitType(l, builtin->ReturnType(), core::AddressSpace::kUndefined,
                              core::Access::kUndefined, "")) {
                    return false;
                }
                l << " result;";
            }
            Line(b) << "result.fract = modf(" << params[0] << ", result.whole);";
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitFrexpCall(StringStream& out,
                               const ast::CallExpression* expr,
                               const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            auto* ty = builtin->Parameters()[0]->Type();
            auto in = params[0];

            std::string width;
            if (auto* vec = ty->As<core::type::Vector>()) {
                width = std::to_string(vec->Width());
            }

            // Emit the builtin return type unique to this overload. This does not
            // exist in the AST, so it will not be generated in Generate().
            if (!EmitStructType(&helpers_, builtin->ReturnType()->As<core::type::Struct>())) {
                return false;
            }

            std::string member_type;
            if (Is<core::type::F16>(ty->DeepestElement())) {
                member_type = width.empty() ? "float16_t" : ("vector<float16_t, " + width + ">");
            } else {
                member_type = "float" + width;
            }

            Line(b) << member_type << " exp;";
            Line(b) << member_type << " fract = sign(" << in << ") * frexp(" << in << ", exp);";
            {
                auto l = Line(b);
                if (!EmitType(l, builtin->ReturnType(), core::AddressSpace::kUndefined,
                              core::Access::kUndefined, "")) {
                    return false;
                }
                l << " result = {fract, int" << width << "(exp)};";
            }
            Line(b) << "return result;";
            return true;
        });
}

bool ASTPrinter::EmitDegreesCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kRadToDeg << ";";
                                 return true;
                             });
}

bool ASTPrinter::EmitRadiansCall(StringStream& out,
                                 const ast::CallExpression* expr,
                                 const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 Line(b) << "return " << params[0] << " * " << std::setprecision(20)
                                         << sem::kDegToRad << ";";
                                 return true;
                             });
}

// The HLSL `sign` method always returns an `int` result (scalar or vector). In WGSL the result is
// expected to be the same type as the argument. This injects a cast to the expected WGSL result
// type after the call to `sign`.
bool ASTPrinter::EmitSignCall(StringStream& out, const sem::Call* call, const sem::BuiltinFn*) {
    auto* arg = call->Arguments()[0];
    if (!EmitType(out, arg->Type(), core::AddressSpace::kUndefined, core::Access::kReadWrite, "")) {
        return false;
    }
    out << "(sign(";
    if (!EmitExpression(out, arg->Declaration())) {
        return false;
    }
    out << "))";
    return true;
}

bool ASTPrinter::EmitQuantizeToF16Call(StringStream& out,
                                       const ast::CallExpression* expr,
                                       const sem::BuiltinFn* builtin) {
    // Cast to f16 and back
    std::string width;
    if (auto* vec = builtin->ReturnType()->As<core::type::Vector>()) {
        width = std::to_string(vec->Width());
    }
    out << "f16tof32(f32tof16(";
    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }
    out << "))";
    return true;
}

bool ASTPrinter::EmitTruncCall(StringStream& out,
                               const ast::CallExpression* expr,
                               const sem::BuiltinFn* builtin) {
    // HLSL's trunc is broken for very large/small float values.
    // See crbug.com/tint/1883
    return CallBuiltinHelper(  //
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            // value < 0 ? ceil(value) : floor(value)
            Line(b) << "return " << params[0] << " < 0 ? ceil(" << params[0] << ") : floor("
                    << params[0] << ");";
            return true;
        });
}

bool ASTPrinter::EmitDataPackingCall(StringStream& out,
                                     const ast::CallExpression* expr,
                                     const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            uint32_t dims = 2;
            bool is_signed = false;
            uint32_t scale = 65535;
            if (builtin->Fn() == wgsl::BuiltinFn::kPack4X8Snorm ||
                builtin->Fn() == wgsl::BuiltinFn::kPack4X8Unorm) {
                dims = 4;
                scale = 255;
            }
            if (builtin->Fn() == wgsl::BuiltinFn::kPack4X8Snorm ||
                builtin->Fn() == wgsl::BuiltinFn::kPack2X16Snorm) {
                is_signed = true;
                scale = (scale - 1) / 2;
            }
            switch (builtin->Fn()) {
                case wgsl::BuiltinFn::kPack4X8Snorm:
                case wgsl::BuiltinFn::kPack4X8Unorm:
                case wgsl::BuiltinFn::kPack2X16Snorm:
                case wgsl::BuiltinFn::kPack2X16Unorm: {
                    {
                        auto l = Line(b);
                        l << (is_signed ? "" : "u") << "int" << dims
                          << " i = " << (is_signed ? "" : "u") << "int" << dims << "(round(clamp("
                          << params[0] << ", " << (is_signed ? "-1.0" : "0.0") << ", 1.0) * "
                          << scale << ".0))";
                        if (is_signed) {
                            l << " & " << (dims == 4 ? "0xff" : "0xffff");
                        }
                        l << ";";
                    }
                    {
                        auto l = Line(b);
                        l << "return ";
                        if (is_signed) {
                            l << "asuint";
                        }
                        l << "(i.x | i.y << " << (32 / dims);
                        if (dims == 4) {
                            l << " | i.z << 16 | i.w << 24";
                        }
                        l << ");";
                    }
                    break;
                }
                case wgsl::BuiltinFn::kPack2X16Float: {
                    Line(b) << "uint2 i = f32tof16(" << params[0] << ");";
                    Line(b) << "return i.x | (i.y << 16);";
                    break;
                }
                default:
                    TINT_ICE() << " unhandled data packing builtin";
            }

            return true;
        });
}

bool ASTPrinter::EmitDataUnpackingCall(StringStream& out,
                                       const ast::CallExpression* expr,
                                       const sem::BuiltinFn* builtin) {
    return CallBuiltinHelper(
        out, expr, builtin, [&](TextBuffer* b, const std::vector<std::string>& params) {
            uint32_t dims = 2;
            bool is_signed = false;
            uint32_t scale = 65535;
            if (builtin->Fn() == wgsl::BuiltinFn::kUnpack4X8Snorm ||
                builtin->Fn() == wgsl::BuiltinFn::kUnpack4X8Unorm) {
                dims = 4;
                scale = 255;
            }
            if (builtin->Fn() == wgsl::BuiltinFn::kUnpack4X8Snorm ||
                builtin->Fn() == wgsl::BuiltinFn::kUnpack2X16Snorm) {
                is_signed = true;
                scale = (scale - 1) / 2;
            }
            switch (builtin->Fn()) {
                case wgsl::BuiltinFn::kUnpack4X8Snorm:
                case wgsl::BuiltinFn::kUnpack2X16Snorm: {
                    Line(b) << "int j = int(" << params[0] << ");";
                    {  // Perform sign extension on the converted values.
                        auto l = Line(b);
                        l << "int" << dims << " i = int" << dims << "(";
                        if (dims == 2) {
                            l << "j << 16, j) >> 16";
                        } else {
                            l << "j << 24, j << 16, j << 8, j) >> 24";
                        }
                        l << ";";
                    }
                    Line(b) << "return clamp(float" << dims << "(i) / " << scale << ".0, "
                            << (is_signed ? "-1.0" : "0.0") << ", 1.0);";
                    break;
                }
                case wgsl::BuiltinFn::kUnpack4X8Unorm:
                case wgsl::BuiltinFn::kUnpack2X16Unorm: {
                    Line(b) << "uint j = " << params[0] << ";";
                    {
                        auto l = Line(b);
                        l << "uint" << dims << " i = uint" << dims << "(";
                        l << "j & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
                        if (dims == 4) {
                            l << "(j >> " << (32 / dims) << ") & 0xff, (j >> 16) & 0xff, j >> 24";
                        } else {
                            l << "j >> " << (32 / dims);
                        }
                        l << ");";
                    }
                    Line(b) << "return float" << dims << "(i) / " << scale << ".0;";
                    break;
                }
                case wgsl::BuiltinFn::kUnpack2X16Float:
                    Line(b) << "uint i = " << params[0] << ";";
                    Line(b) << "return f16tof32(uint2(i & 0xffff, i >> 16));";
                    break;
                default:
                    TINT_ICE() << "unhandled data packing builtin";
            }

            return true;
        });
}

bool ASTPrinter::EmitPacked4x8IntegerDotProductBuiltinCall(StringStream& out,
                                                           const ast::CallExpression* expr,
                                                           const sem::BuiltinFn* builtin) {
    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kDot4I8Packed:
        case wgsl::BuiltinFn::kDot4U8Packed:
            break;
        case wgsl::BuiltinFn::kPack4XI8: {
            out << "uint(pack_s8(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kPack4XU8: {
            out << "uint(pack_u8(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kPack4XI8Clamp: {
            out << "uint(pack_clamp_s8(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kUnpack4XI8: {
            out << "unpack_s8s32(int8_t4_packed(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kUnpack4XU8: {
            out << "unpack_u8u32(uint8_t4_packed(";
            if (!EmitExpression(out, expr->args[0])) {
                return false;
            }
            out << "))";
            return true;
        }
        case wgsl::BuiltinFn::kPack4XU8Clamp:
        default:
            TINT_UNIMPLEMENTED() << builtin->Fn();
    }

    return CallBuiltinHelper(out, expr, builtin,
                             [&](TextBuffer* b, const std::vector<std::string>& params) {
                                 std::string functionName;
                                 switch (builtin->Fn()) {
                                     case wgsl::BuiltinFn::kDot4I8Packed:
                                         Line(b) << "int accumulator = 0;";
                                         functionName = "dot4add_i8packed";
                                         break;
                                     case wgsl::BuiltinFn::kDot4U8Packed:
                                         Line(b) << "uint accumulator = 0u;";
                                         functionName = "dot4add_u8packed";
                                         break;
                                     default:
                                         TINT_ICE() << "Internal error: unhandled DP4a builtin";
                                 }
                                 Line(b) << "return " << functionName << "(" << params[0] << ", "
                                         << params[1] << ", accumulator);";

                                 return true;
                             });
}

bool ASTPrinter::EmitBarrierCall(StringStream& out, const sem::BuiltinFn* builtin) {
    // TODO(crbug.com/tint/661): Combine sequential barriers to a single
    // instruction.
    if (builtin->Fn() == wgsl::BuiltinFn::kWorkgroupBarrier) {
        out << "GroupMemoryBarrierWithGroupSync()";
    } else if (builtin->Fn() == wgsl::BuiltinFn::kStorageBarrier) {
        out << "DeviceMemoryBarrierWithGroupSync()";
    } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureBarrier) {
        out << "DeviceMemoryBarrierWithGroupSync()";
    } else {
        TINT_UNREACHABLE() << "unexpected barrier builtin type " << builtin->Fn();
    }
    return true;
}

bool ASTPrinter::EmitTextureOrStorageBufferCallArgExpression(StringStream& out,
                                                             const ast::Expression* expr) {
    // TODO(crbug.com/tint/1976): Workaround DXC bug that fails to compile texture/storage function
    // calls with signed integer splatted constants. DXC fails to convert the coord arg, for e.g.
    // `0.xxx`, from a vector of 64-bit ints to a vector of 32-bit ints to match the texture load
    // parameter type. We work around this for now by explicitly casting the splatted constant to
    // the right type, for e.g. `int3(0.xxx)`.
    bool emitted_cast = false;
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto* constant = sem->ConstantValue()) {
            if (auto* splat = constant->As<core::constant::Splat>()) {
                if (splat->Type()->IsSignedIntegerVector()) {
                    if (!EmitType(out, constant->Type(), core::AddressSpace::kUndefined,
                                  core::Access::kUndefined, "")) {
                        return false;
                    }
                    out << "(";
                    emitted_cast = true;
                }
            }
        }
    }
    if (!EmitExpression(out, expr)) {
        return false;
    }
    if (emitted_cast) {
        out << ")";
    }
    return true;
}

bool ASTPrinter::EmitTextureCall(StringStream& out,
                                 const sem::Call* call,
                                 const sem::BuiltinFn* builtin) {
    using Usage = core::ParameterUsage;

    auto& signature = builtin->Signature();
    auto* expr = call->Declaration();
    auto arguments = expr->args;

    // Returns the argument with the given usage
    auto arg = [&](Usage usage) {
        int idx = signature.IndexOf(usage);
        return (idx >= 0) ? arguments[static_cast<size_t>(idx)] : nullptr;
    };

    auto* texture = arg(Usage::kTexture);
    if (DAWN_UNLIKELY(!texture)) {
        TINT_ICE() << "missing texture argument";
    }

    auto* texture_type = TypeOf(texture)->UnwrapRef()->As<core::type::Texture>();

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureDimensions:
        case wgsl::BuiltinFn::kTextureNumLayers:
        case wgsl::BuiltinFn::kTextureNumLevels:
        case wgsl::BuiltinFn::kTextureNumSamples: {
            // All of these builtins use the GetDimensions() method on the texture
            bool is_ms = texture_type->IsAnyOf<core::type::MultisampledTexture,
                                               core::type::DepthMultisampledTexture>();
            int num_dimensions = 0;
            std::string swizzle;

            switch (builtin->Fn()) {
                case wgsl::BuiltinFn::kTextureDimensions:
                    switch (texture_type->Dim()) {
                        case core::type::TextureDimension::kNone:
                            TINT_ICE() << "texture dimension is kNone";
                        case core::type::TextureDimension::k1d:
                            num_dimensions = 1;
                            break;
                        case core::type::TextureDimension::k2d:
                            num_dimensions = is_ms ? 3 : 2;
                            swizzle = is_ms ? ".xy" : "";
                            break;
                        case core::type::TextureDimension::k2dArray:
                            num_dimensions = is_ms ? 4 : 3;
                            swizzle = ".xy";
                            break;
                        case core::type::TextureDimension::k3d:
                            num_dimensions = 3;
                            break;
                        case core::type::TextureDimension::kCube:
                            num_dimensions = 2;
                            break;
                        case core::type::TextureDimension::kCubeArray:
                            num_dimensions = 3;
                            swizzle = ".xy";
                            break;
                    }
                    break;
                case wgsl::BuiltinFn::kTextureNumLayers:
                    switch (texture_type->Dim()) {
                        default:
                            TINT_ICE() << "texture dimension is not arrayed";
                        case core::type::TextureDimension::k2dArray:
                            num_dimensions = is_ms ? 4 : 3;
                            swizzle = ".z";
                            break;
                        case core::type::TextureDimension::kCubeArray:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                    }
                    break;
                case wgsl::BuiltinFn::kTextureNumLevels:
                    switch (texture_type->Dim()) {
                        default:
                            TINT_ICE() << "texture dimension does not support mips";
                        case core::type::TextureDimension::k1d:
                            num_dimensions = 2;
                            swizzle = ".y";
                            break;
                        case core::type::TextureDimension::k2d:
                        case core::type::TextureDimension::kCube:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                        case core::type::TextureDimension::k2dArray:
                        case core::type::TextureDimension::k3d:
                        case core::type::TextureDimension::kCubeArray:
                            num_dimensions = 4;
                            swizzle = ".w";
                            break;
                    }
                    break;
                case wgsl::BuiltinFn::kTextureNumSamples:
                    switch (texture_type->Dim()) {
                        default:
                            TINT_ICE() << "texture dimension does not support multisampling";
                        case core::type::TextureDimension::k2d:
                            num_dimensions = 3;
                            swizzle = ".z";
                            break;
                        case core::type::TextureDimension::k2dArray:
                            num_dimensions = 4;
                            swizzle = ".w";
                            break;
                    }
                    break;
                default:
                    TINT_ICE() << "unexpected builtin";
            }

            auto* level_arg = arg(Usage::kLevel);

            if (level_arg) {
                // `NumberOfLevels` is a non-optional argument if `MipLevel` was passed.
                // Increment the number of dimensions for the temporary vector to
                // accommodate this.
                num_dimensions++;

                // If the swizzle was empty, the expression will evaluate to the whole
                // vector. As we've grown the vector by one element, we now need to
                // swizzle to keep the result expression equivalent.
                if (swizzle.empty()) {
                    static constexpr const char* swizzles[] = {"", ".x", ".xy", ".xyz"};
                    swizzle = swizzles[num_dimensions - 1];
                }
            }

            if (DAWN_UNLIKELY(num_dimensions > 4)) {
                TINT_ICE() << "Texture query builtin temporary vector has " << num_dimensions
                           << " dimensions";
            }

            // Declare a variable to hold the queried texture info
            auto dims = UniqueIdentifier(kTempNamePrefix);
            if (num_dimensions == 1) {
                Line() << "uint " << dims << ";";
            } else {
                Line() << "uint" << num_dimensions << " " << dims << ";";
            }

            {  // texture.GetDimensions(...)
                auto pre = Line();
                if (!EmitExpression(pre, texture)) {
                    return false;
                }
                pre << ".GetDimensions(";

                if (level_arg) {
                    if (!EmitExpression(pre, level_arg)) {
                        return false;
                    }
                    pre << ", ";
                } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureNumLevels) {
                    pre << "0, ";
                }

                if (num_dimensions == 1) {
                    pre << dims;
                } else {
                    static constexpr char xyzw[] = {'x', 'y', 'z', 'w'};
                    if (DAWN_UNLIKELY(num_dimensions < 0 || num_dimensions > 4)) {
                        TINT_ICE() << "vector dimensions are " << num_dimensions;
                    }
                    for (int i = 0; i < num_dimensions; i++) {
                        if (i > 0) {
                            pre << ", ";
                        }
                        pre << dims << "." << xyzw[i];
                    }
                }

                pre << ");";
            }

            // The out parameters of the GetDimensions() call is now in temporary
            // `dims` variable. This may be packed with other data, so the final
            // expression may require a swizzle.
            out << dims << swizzle;
            return true;
        }
        default:
            break;
    }

    if (!EmitExpression(out, texture)) {
        return false;
    }

    // If pack_level_in_coords is true, then the mip level will be appended as the
    // last value of the coordinates argument. If the WGSL builtin overload does
    // not have a level parameter and pack_level_in_coords is true, then a zero
    // mip level will be inserted.
    bool pack_level_in_coords = false;

    uint32_t hlsl_ret_width = 4u;

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kTextureSample:
            out << ".Sample(";
            break;
        case wgsl::BuiltinFn::kTextureSampleBias:
            out << ".SampleBias(";
            break;
        case wgsl::BuiltinFn::kTextureSampleLevel:
            out << ".SampleLevel(";
            break;
        case wgsl::BuiltinFn::kTextureSampleGrad:
            out << ".SampleGrad(";
            break;
        case wgsl::BuiltinFn::kTextureSampleCompare:
            out << ".SampleCmp(";
            hlsl_ret_width = 1;
            break;
        case wgsl::BuiltinFn::kTextureSampleCompareLevel:
            out << ".SampleCmpLevelZero(";
            hlsl_ret_width = 1;
            break;
        case wgsl::BuiltinFn::kTextureLoad:
            out << ".Load(";
            // Multisampled textures and read-write storage textures do not support mip-levels.
            if (texture_type->Is<core::type::MultisampledTexture>()) {
                break;
            }
            if (auto* storage_texture_type = texture_type->As<core::type::StorageTexture>()) {
                if (storage_texture_type->Access() == core::Access::kReadWrite) {
                    break;
                }
            }
            pack_level_in_coords = true;
            break;
        case wgsl::BuiltinFn::kTextureGather:
            out << ".Gather";
            if (builtin->Parameters()[0]->Usage() == core::ParameterUsage::kComponent) {
                switch (call->Arguments()[0]->ConstantValue()->ValueAs<AInt>()) {
                    case 0:
                        out << "Red";
                        break;
                    case 1:
                        out << "Green";
                        break;
                    case 2:
                        out << "Blue";
                        break;
                    case 3:
                        out << "Alpha";
                        break;
                }
            }
            out << "(";
            break;
        case wgsl::BuiltinFn::kTextureGatherCompare:
            out << ".GatherCmp(";
            break;
        case wgsl::BuiltinFn::kTextureStore:
            out << "[";
            break;
        default:
            TINT_ICE() << "Unhandled texture builtin '" << builtin << "'";
    }

    if (auto* sampler = arg(Usage::kSampler)) {
        if (!EmitExpression(out, sampler)) {
            return false;
        }
        out << ", ";
    }

    auto* param_coords = arg(Usage::kCoords);
    if (DAWN_UNLIKELY(!param_coords)) {
        TINT_ICE() << "missing coords argument";
    }

    auto emit_vector_appended_with_i32_zero = [&](const ast::Expression* vector) {
        auto* i32 = builder_.create<core::type::I32>();
        auto* zero = builder_.Expr(0_i);
        auto* stmt = builder_.Sem().Get(vector)->Stmt();
        builder_.Sem().Add(zero, builder_.create<sem::ValueExpression>(
                                     zero, i32, core::EvaluationStage::kRuntime, stmt,
                                     /* constant_value */ nullptr,
                                     /* has_side_effects */ false));
        auto* packed = tint::wgsl::AppendVector(&builder_, vector, zero);
        return EmitExpression(out, packed->Declaration());
    };

    auto emit_vector_appended_with_level = [&](const ast::Expression* vector) {
        if (auto* level = arg(Usage::kLevel)) {
            auto* packed = tint::wgsl::AppendVector(&builder_, vector, level);
            return EmitExpression(out, packed->Declaration());
        }
        return emit_vector_appended_with_i32_zero(vector);
    };

    if (auto* array_index = arg(Usage::kArrayIndex)) {
        // Array index needs to be appended to the coordinates.
        auto* packed = tint::wgsl::AppendVector(&builder_, param_coords, array_index);
        if (pack_level_in_coords) {
            // Then mip level needs to be appended to the coordinates.
            if (!emit_vector_appended_with_level(packed->Declaration())) {
                return false;
            }
        } else {
            if (!EmitExpression(out, packed->Declaration())) {
                return false;
            }
        }
    } else if (pack_level_in_coords) {
        // Mip level needs to be appended to the coordinates.
        if (!emit_vector_appended_with_level(param_coords)) {
            return false;
        }
    } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureStore) {
        // param_coords is an index expression, not a function arg
        if (!EmitExpression(out, param_coords)) {
            return false;
        }
    } else if (!EmitTextureOrStorageBufferCallArgExpression(out, param_coords)) {
        return false;
    }

    for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx, Usage::kDdy,
                       Usage::kSampleIndex, Usage::kOffset}) {
        if (usage == Usage::kLevel && pack_level_in_coords) {
            continue;  // mip level already packed in coordinates.
        }
        if (auto* e = arg(usage)) {
            out << ", ";
            if (usage == Usage::kBias) {
                out << "clamp(";
            }
            if (!EmitTextureOrStorageBufferCallArgExpression(out, e)) {
                return false;
            }
            if (usage == Usage::kBias) {
                out << ", -16.0f, 15.99f)";
            }
        }
    }

    if (builtin->Fn() == wgsl::BuiltinFn::kTextureStore) {
        out << "] = ";
        if (!EmitExpression(out, arg(Usage::kValue))) {
            return false;
        }
    } else {
        out << ")";

        // If the builtin return type does not match the number of elements of the
        // HLSL builtin, we need to swizzle the expression to generate the correct
        // number of components.
        uint32_t wgsl_ret_width = 1;
        if (auto* vec = builtin->ReturnType()->As<core::type::Vector>()) {
            wgsl_ret_width = vec->Width();
        }
        if (wgsl_ret_width < hlsl_ret_width) {
            out << ".";
            for (uint32_t i = 0; i < wgsl_ret_width; i++) {
                out << "xyz"[i];
            }
        }
        if (DAWN_UNLIKELY(wgsl_ret_width > hlsl_ret_width)) {
            TINT_ICE() << "WGSL return width (" << wgsl_ret_width
                       << ") is wider than HLSL return width (" << hlsl_ret_width << ") for "
                       << builtin->Fn();
        }
    }

    return true;
}

// The following subgroup builtin functions are translated to HLSL as follows:
// +---------------------+----------------------------------------------------------------+
// |        WGSL         |                              HLSL                              |
// +---------------------+----------------------------------------------------------------+
// | subgroupShuffleXor  | WaveReadLaneAt with index equal subgroup_invocation_id ^ mask  |
// | subgroupShuffleUp   | WaveReadLaneAt with index equal subgroup_invocation_id - delta |
// | subgroupShuffleDown | WaveReadLaneAt with index equal subgroup_invocation_id + delta |
// +---------------------+----------------------------------------------------------------+
bool ASTPrinter::EmitSubgroupShuffleBuiltinCall(StringStream& out,
                                                const ast::CallExpression* expr,
                                                const sem::BuiltinFn* builtin) {
    out << "WaveReadLaneAt(";

    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }

    out << ", (WaveGetLaneIndex() ";

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kSubgroupShuffleXor:
            out << "^ ";
            break;
        case wgsl::BuiltinFn::kSubgroupShuffleUp:
            out << "- ";
            break;
        case wgsl::BuiltinFn::kSubgroupShuffleDown:
            out << "+ ";
            break;
        default:
            TINT_UNREACHABLE();
    }

    if (!EmitExpression(out, expr->args[1])) {
        return false;
    }
    out << "))";

    return true;
}

// The following subgroup builtin functions are translated to HLSL as follows:
// +-----------------------+----------------------+
// |        WGSL           |       HLSL           |
// +-----------------------+----------------------+
// | subgroupInclusiveAdd  | WavePrefixSum(x) + x |
// | subgroupInclusiveMul  | WavePrefixMul(x) * x |
// +-----------------------+----------------------+
bool ASTPrinter::EmitSubgroupInclusiveBuiltinCall(StringStream& out,
                                                  const ast::CallExpression* expr,
                                                  const sem::BuiltinFn* builtin) {
    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kSubgroupInclusiveAdd:
            out << "(WavePrefixSum(";
            break;
        case wgsl::BuiltinFn::kSubgroupInclusiveMul:
            out << "(WavePrefixProduct(";
            break;
        default:
            TINT_UNREACHABLE();
    }

    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }

    out << ") ";

    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kSubgroupInclusiveAdd:
            out << "+";
            break;
        case wgsl::BuiltinFn::kSubgroupInclusiveMul:
            out << "*";
            break;
        default:
            TINT_UNREACHABLE();
    }
    // Add a space after the operand to be more consistent with IR generated code.
    out << " ";
    if (!EmitExpression(out, expr->args[0])) {
        return false;
    }

    out << ")";

    return true;
}

std::string ASTPrinter::generate_builtin_name(const sem::BuiltinFn* builtin) {
    switch (builtin->Fn()) {
        case wgsl::BuiltinFn::kAbs:
        case wgsl::BuiltinFn::kAcos:
        case wgsl::BuiltinFn::kAll:
        case wgsl::BuiltinFn::kAny:
        case wgsl::BuiltinFn::kAsin:
        case wgsl::BuiltinFn::kAtan:
        case wgsl::BuiltinFn::kAtan2:
        case wgsl::BuiltinFn::kCeil:
        case wgsl::BuiltinFn::kClamp:
        case wgsl::BuiltinFn::kCos:
        case wgsl::BuiltinFn::kCosh:
        case wgsl::BuiltinFn::kCross:
        case wgsl::BuiltinFn::kDeterminant:
        case wgsl::BuiltinFn::kDistance:
        case wgsl::BuiltinFn::kDot:
        case wgsl::BuiltinFn::kExp:
        case wgsl::BuiltinFn::kExp2:
        case wgsl::BuiltinFn::kFloor:
        case wgsl::BuiltinFn::kFrexp:
        case wgsl::BuiltinFn::kLdexp:
        case wgsl::BuiltinFn::kLength:
        case wgsl::BuiltinFn::kLog:
        case wgsl::BuiltinFn::kLog2:
        case wgsl::BuiltinFn::kMax:
        case wgsl::BuiltinFn::kMin:
        case wgsl::BuiltinFn::kModf:
        case wgsl::BuiltinFn::kNormalize:
        case wgsl::BuiltinFn::kPow:
        case wgsl::BuiltinFn::kReflect:
        case wgsl::BuiltinFn::kRefract:
        case wgsl::BuiltinFn::kRound:
        case wgsl::BuiltinFn::kSaturate:
        case wgsl::BuiltinFn::kSin:
        case wgsl::BuiltinFn::kSinh:
        case wgsl::BuiltinFn::kSqrt:
        case wgsl::BuiltinFn::kStep:
        case wgsl::BuiltinFn::kTan:
        case wgsl::BuiltinFn::kTanh:
        case wgsl::BuiltinFn::kTranspose:
            return builtin->str();
        case wgsl::BuiltinFn::kCountOneBits:  // uint
            return "countbits";
        case wgsl::BuiltinFn::kDpdx:
            return "ddx";
        case wgsl::BuiltinFn::kDpdxCoarse:
            return "ddx_coarse";
        case wgsl::BuiltinFn::kDpdxFine:
            return "ddx_fine";
        case wgsl::BuiltinFn::kDpdy:
            return "ddy";
        case wgsl::BuiltinFn::kDpdyCoarse:
            return "ddy_coarse";
        case wgsl::BuiltinFn::kDpdyFine:
            return "ddy_fine";
        case wgsl::BuiltinFn::kFaceForward:
            return "faceforward";
        case wgsl::BuiltinFn::kFract:
            return "frac";
        case wgsl::BuiltinFn::kFma:
            return "mad";
        case wgsl::BuiltinFn::kFwidth:
        case wgsl::BuiltinFn::kFwidthCoarse:
        case wgsl::BuiltinFn::kFwidthFine:
            return "fwidth";
        case wgsl::BuiltinFn::kInverseSqrt:
            return "rsqrt";
        case wgsl::BuiltinFn::kMix:
            return "lerp";
        case wgsl::BuiltinFn::kReverseBits:  // uint
            return "reversebits";
        case wgsl::BuiltinFn::kSmoothstep:
            return "smoothstep";
        case wgsl::BuiltinFn::kSubgroupBallot:
            return "WaveActiveBallot";
        case wgsl::BuiltinFn::kSubgroupElect:
            return "WaveIsFirstLane";
        case wgsl::BuiltinFn::kSubgroupBroadcast:
            return "WaveReadLaneAt";
        case wgsl::BuiltinFn::kSubgroupBroadcastFirst:
            return "WaveReadLaneFirst";
        case wgsl::BuiltinFn::kSubgroupShuffle:
            return "WaveReadLaneAt";
        case wgsl::BuiltinFn::kSubgroupAdd:
            return "WaveActiveSum";
        case wgsl::BuiltinFn::kSubgroupExclusiveAdd:
            return "WavePrefixSum";
        case wgsl::BuiltinFn::kSubgroupMul:
            return "WaveActiveProduct";
        case wgsl::BuiltinFn::kSubgroupExclusiveMul:
            return "WavePrefixProduct";
        case wgsl::BuiltinFn::kSubgroupAnd:
            return "WaveActiveBitAnd";
        case wgsl::BuiltinFn::kSubgroupOr:
            return "WaveActiveBitOr";
        case wgsl::BuiltinFn::kSubgroupXor:
            return "WaveActiveBitXor";
        case wgsl::BuiltinFn::kSubgroupMin:
            return "WaveActiveMin";
        case wgsl::BuiltinFn::kSubgroupMax:
            return "WaveActiveMax";
        case wgsl::BuiltinFn::kSubgroupAll:
            return "WaveActiveAllTrue";
        case wgsl::BuiltinFn::kSubgroupAny:
            return "WaveActiveAnyTrue";
        case wgsl::BuiltinFn::kQuadBroadcast:
            return "QuadReadLaneAt";
        case wgsl::BuiltinFn::kQuadSwapX:
            return "QuadReadAcrossX";
        case wgsl::BuiltinFn::kQuadSwapY:
            return "QuadReadAcrossY";
        case wgsl::BuiltinFn::kQuadSwapDiagonal:
            return "QuadReadAcrossDiagonal";
        default:
            diagnostics_.AddError(Source{}) << "Unknown builtin method: " << builtin->str();
    }

    return "";
}

bool ASTPrinter::EmitCase(const ast::SwitchStatement* s, size_t case_idx) {
    auto* stmt = s->body[case_idx];
    auto* sem = builder_.Sem().Get<sem::CaseStatement>(stmt);
    for (auto* selector : sem->Selectors()) {
        auto out = Line();
        if (selector->IsDefault()) {
            out << "default";
        } else {
            out << "case ";
            if (!EmitConstant(out, selector->Value(), /* is_variable_initializer */ false)) {
                return false;
            }
        }
        out << ":";
        if (selector == sem->Selectors().back()) {
            out << " {";
        }
    }

    IncrementIndent();
    TINT_DEFER({
        DecrementIndent();
        Line() << "}";
    });

    // Emit the case statement
    if (!EmitStatements(stmt->body->statements)) {
        return false;
    }

    if (!tint::IsAnyOf<ast::BreakStatement>(stmt->body->Last())) {
        Line() << "break;";
    }

    return true;
}

bool ASTPrinter::EmitContinue(const ast::ContinueStatement*) {
    if (!emit_continuing_ || !emit_continuing_()) {
        return false;
    }
    Line() << "continue;";
    return true;
}

bool ASTPrinter::EmitDiscard(const ast::DiscardStatement*) {
    // TODO(dsinclair): Verify this is correct when the discard semantics are
    // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
    Line() << "discard;";
    return true;
}

bool ASTPrinter::EmitExpression(StringStream& out, const ast::Expression* expr) {
    if (auto* sem = builder_.Sem().GetVal(expr)) {
        if (auto* constant = sem->ConstantValue()) {
            bool is_variable_initializer = false;
            if (auto* stmt = sem->Stmt()) {
                if (auto* decl = As<ast::VariableDeclStatement>(stmt->Declaration())) {
                    is_variable_initializer = decl->variable->initializer == expr;
                }
            }
            return EmitConstant(out, constant, is_variable_initializer);
        }
    }
    return Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { return EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { return EmitBinary(out, b); },
        [&](const ast::CallExpression* c) { return EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { return EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { return EmitMemberAccessor(out, m); },
        [&](const ast::UnaryOpExpression* u) { return EmitUnaryOp(out, u); },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr) {
    out << expr->identifier->symbol.Name();
    return true;
}

bool ASTPrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = Line();
        out << "if (";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
    }

    if (stmt->else_statement) {
        Line() << "} else {";
        if (auto* block = stmt->else_statement->As<ast::BlockStatement>()) {
            if (!EmitStatementsWithIndent(block->statements)) {
                return false;
            }
        } else {
            if (!EmitStatementsWithIndent(Vector{stmt->else_statement})) {
                return false;
            }
        }
    }
    Line() << "}";

    return true;
}

bool ASTPrinter::EmitFunction(const ast::Function* func) {
    auto* sem = builder_.Sem().Get(func);

    // Emit storage atomic helpers
    if (auto* intrinsic = ast::GetAttribute<DecomposeMemoryAccess::Intrinsic>(func->attributes)) {
        if (intrinsic->address_space == core::AddressSpace::kStorage && intrinsic->IsAtomic()) {
            if (!EmitStorageAtomicIntrinsic(func, intrinsic)) {
                return false;
            }
        }
        return true;
    }

    if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
        // An internal function. Do not emit.
        return true;
    }

    {
        auto out = Line();
        auto name = func->name->symbol.Name();
        // If the function returns an array, then we need to declare a typedef for
        // this.
        if (sem->ReturnType()->Is<core::type::Array>()) {
            auto typedef_name = UniqueIdentifier(name + "_ret");
            auto pre = Line();
            pre << "typedef ";
            if (!EmitTypeAndName(pre, sem->ReturnType(), core::AddressSpace::kUndefined,
                                 core::Access::kReadWrite, typedef_name)) {
                return false;
            }
            pre << ";";
            out << typedef_name;
        } else {
            if (!EmitType(out, sem->ReturnType(), core::AddressSpace::kUndefined,
                          core::Access::kReadWrite, "")) {
                return false;
            }
        }

        out << " " << name << "(";

        bool first = true;

        for (auto* v : sem->Parameters()) {
            if (!first) {
                out << ", ";
            }
            first = false;

            auto const* type = v->Type();
            auto address_space = core::AddressSpace::kUndefined;
            auto access = core::Access::kUndefined;

            if (auto* ptr = type->As<core::type::Pointer>()) {
                type = ptr->StoreType();
                switch (ptr->AddressSpace()) {
                    case core::AddressSpace::kStorage:
                    case core::AddressSpace::kUniform:
                        // Not allowed by WGSL, but is used by certain transforms (e.g. DMA) to pass
                        // storage buffers and uniform buffers down into transform-generated
                        // functions. In this situation we want to generate the parameter without an
                        // 'inout', using the address space and access from the pointer.
                        address_space = ptr->AddressSpace();
                        access = ptr->Access();
                        break;
                    default:
                        // Transform regular WGSL pointer parameters in to `inout` parameters.
                        out << "inout ";
                }
            }

            // Note: WGSL only allows for AddressSpace::kUndefined on parameters, however
            // the sanitizer transforms generates load / store functions for storage
            // or uniform buffers. These functions have a buffer parameter with
            // AddressSpace::kStorage or AddressSpace::kUniform. This is required to
            // correctly translate the parameter to a [RW]ByteAddressBuffer for
            // storage buffers and a uint4[N] for uniform buffers.
            if (!EmitTypeAndName(out, type, address_space, access,
                                 v->Declaration()->name->symbol.Name())) {
                return false;
            }
        }
        out << ") {";
    }

    if (sem->DiscardStatement() && !sem->ReturnType()->Is<core::type::Void>()) {
        // BUG(crbug.com/tint/1081): work around non-void functions with discard
        // failing compilation sometimes
        if (!EmitFunctionBodyWithDiscard(func)) {
            return false;
        }
    } else {
        if (!EmitStatementsWithIndent(func->body->statements)) {
            return false;
        }
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitFunctionBodyWithDiscard(const ast::Function* func) {
    // FXC sometimes fails to compile functions that discard with 'Not all control
    // paths return a value'. We work around this by wrapping the function body
    // within an "if (true) { <body> } return <default return type obj>;" so that
    // there is always an (unused) return statement.

    auto* sem = builder_.Sem().Get(func);
    TINT_ASSERT(sem->DiscardStatement() && !sem->ReturnType()->Is<core::type::Void>());

    ScopedIndent si(this);
    Line() << "if (true) {";

    if (!EmitStatementsWithIndent(func->body->statements)) {
        return false;
    }

    Line() << "}";

    // Return an unused result that matches the type of the return value
    auto name = builder_.Symbols().New("unused").Name();
    {
        auto out = Line();
        if (!EmitTypeAndName(out, sem->ReturnType(), core::AddressSpace::kUndefined,
                             core::Access::kReadWrite, name)) {
            return false;
        }
        out << ";";
    }
    Line() << "return " << name << ";";

    return true;
}

bool ASTPrinter::EmitGlobalVariable(const ast::Variable* global) {
    return Switch(
        global,  //
        [&](const ast::Var* var) {
            auto* sem = builder_.Sem().Get(global);
            switch (sem->AddressSpace()) {
                case core::AddressSpace::kUniform:
                    return EmitUniformVariable(var, sem);
                case core::AddressSpace::kStorage:
                    return EmitStorageVariable(var, sem);
                case core::AddressSpace::kHandle:
                    return EmitHandleVariable(var, sem);
                case core::AddressSpace::kPrivate:
                    return EmitPrivateVariable(sem);
                case core::AddressSpace::kWorkgroup:
                    return EmitWorkgroupVariable(sem);
                case core::AddressSpace::kPushConstant:
                    diagnostics_.AddError(Source{})
                        << "unhandled address space " << sem->AddressSpace();
                    return false;
                default: {
                    TINT_ICE() << "unhandled address space " << sem->AddressSpace();
                }
            }
        },
        [&](const ast::Override*) {
            // Override is removed with SubstituteOverride
            diagnostics_.AddError(Source{})
                << "override-expressions should have been removed with the SubstituteOverride "
                   "transform";
            return false;
        },
        [&](const ast::Const*) {
            return true;  // Constants are embedded at their use
        },                //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitUniformVariable(const ast::Var* var, const sem::Variable* sem) {
    auto binding_point = *sem->As<sem::GlobalVariable>()->Attributes().binding_point;
    auto* type = sem->Type()->UnwrapRef();
    auto name = var->name->symbol.Name();
    Line() << "cbuffer cbuffer_" << name << RegisterAndSpace('b', binding_point) << " {";

    {
        ScopedIndent si(this);
        auto out = Line();
        if (!EmitTypeAndName(out, type, core::AddressSpace::kUniform, sem->Access(), name)) {
            return false;
        }
        out << ";";
    }

    Line() << "};";

    return true;
}

bool ASTPrinter::EmitStorageVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* type = sem->Type()->UnwrapRef();
    auto out = Line();
    if (!EmitTypeAndName(out, type, core::AddressSpace::kStorage, sem->Access(),
                         var->name->symbol.Name())) {
        return false;
    }

    auto* global_sem = sem->As<sem::GlobalVariable>();
    out << RegisterAndSpace(sem->Access() == core::Access::kRead ? 't' : 'u',
                            *global_sem->Attributes().binding_point)
        << ";";

    return true;
}

bool ASTPrinter::EmitHandleVariable(const ast::Var* var, const sem::Variable* sem) {
    auto* unwrapped_type = sem->Type()->UnwrapRef();
    auto out = Line();

    auto name = var->name->symbol.Name();
    auto* type = sem->Type()->UnwrapRef();
    if (ast::HasAttribute<PixelLocal::RasterizerOrderedView>(var->attributes)) {
        TINT_ASSERT(!type->Is<core::type::MultisampledTexture>());
        auto* storage = type->As<core::type::StorageTexture>();
        if (!storage) {
            TINT_ICE() << "Rasterizer Ordered View type isn't storage texture";
        }
        out << "RasterizerOrderedTexture2D";
        auto* component = ImageFormatToRWtextureType(storage->TexelFormat());
        if (DAWN_UNLIKELY(!component)) {
            TINT_ICE() << "Unsupported StorageTexture TexelFormat: "
                       << static_cast<int>(storage->TexelFormat());
        }
        out << "<" << component << "> " << name;
    } else if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), name)) {
        return false;
    }

    const char* register_space = nullptr;

    if (unwrapped_type->Is<core::type::Texture>()) {
        register_space = "t";
        if (auto* st = unwrapped_type->As<core::type::StorageTexture>();
            st && st->Access() != core::Access::kRead) {
            register_space = "u";
        }
    } else if (unwrapped_type->Is<core::type::Sampler>()) {
        register_space = "s";
    }

    if (register_space) {
        auto bp = sem->As<sem::GlobalVariable>()->Attributes().binding_point;
        out << " : register(" << register_space << bp->binding;
        // Omit the space if it's 0, as it's the default.
        // SM 5.0 doesn't support spaces, so we don't emit them if group is 0 for better
        // compatibility.
        if (bp->group == 0) {
            out << ")";
        } else {
            out << ", space" << bp->group << ")";
        }
    }

    out << ";";
    return true;
}

bool ASTPrinter::EmitPrivateVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = Line();

    out << "static ";

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    if (!EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name)) {
        return false;
    }

    out << " = ";
    if (auto* initializer = decl->initializer) {
        if (!EmitExpression(out, initializer)) {
            return false;
        }
    } else {
        if (!EmitZeroValue(out, var->Type()->UnwrapRef())) {
            return false;
        }
    }

    out << ";";
    return true;
}

bool ASTPrinter::EmitWorkgroupVariable(const sem::Variable* var) {
    auto* decl = var->Declaration();
    auto out = Line();

    out << "groupshared ";

    auto name = decl->name->symbol.Name();
    auto* type = var->Type()->UnwrapRef();
    if (!EmitTypeAndName(out, type, var->AddressSpace(), var->Access(), name)) {
        return false;
    }

    if (auto* initializer = decl->initializer) {
        out << " = ";
        if (!EmitExpression(out, initializer)) {
            return false;
        }
    }

    out << ";";
    return true;
}

std::string ASTPrinter::builtin_to_attribute(core::BuiltinValue builtin) const {
    switch (builtin) {
        case core::BuiltinValue::kPosition:
            return "SV_Position";
        case core::BuiltinValue::kVertexIndex:
            return "SV_VertexID";
        case core::BuiltinValue::kInstanceIndex:
            return "SV_InstanceID";
        case core::BuiltinValue::kFrontFacing:
            return "SV_IsFrontFace";
        case core::BuiltinValue::kFragDepth:
            return "SV_Depth";
        case core::BuiltinValue::kLocalInvocationId:
            return "SV_GroupThreadID";
        case core::BuiltinValue::kLocalInvocationIndex:
            return "SV_GroupIndex";
        case core::BuiltinValue::kGlobalInvocationId:
            return "SV_DispatchThreadID";
        case core::BuiltinValue::kWorkgroupId:
            return "SV_GroupID";
        case core::BuiltinValue::kSampleIndex:
            return "SV_SampleIndex";
        case core::BuiltinValue::kSampleMask:
            return "SV_Coverage";
        case core::BuiltinValue::kClipDistances:
            return "SV_ClipDistance0";
        default:
            break;
    }
    return "";
}

std::string ASTPrinter::interpolation_to_modifiers(core::InterpolationType type,
                                                   core::InterpolationSampling sampling) const {
    std::string modifiers;
    switch (type) {
        case core::InterpolationType::kPerspective:
            modifiers += "linear ";
            break;
        case core::InterpolationType::kLinear:
            modifiers += "noperspective ";
            break;
        case core::InterpolationType::kFlat:
            modifiers += "nointerpolation ";
            break;
        case core::InterpolationType::kUndefined:
            break;
    }
    switch (sampling) {
        case core::InterpolationSampling::kCentroid:
            modifiers += "centroid ";
            break;
        case core::InterpolationSampling::kSample:
            modifiers += "sample ";
            break;
        case core::InterpolationSampling::kCenter:
        case core::InterpolationSampling::kFirst:
        case core::InterpolationSampling::kEither:
        case core::InterpolationSampling::kUndefined:
            break;
    }
    return modifiers;
}

bool ASTPrinter::EmitEntryPointFunction(const ast::Function* func) {
    auto* func_sem = builder_.Sem().Get(func);

    {
        auto out = Line();
        if (func->PipelineStage() == ast::PipelineStage::kCompute) {
            // Emit the workgroup_size attribute.
            auto wgsize = func_sem->WorkgroupSize();
            out << "[numthreads(";
            for (size_t i = 0; i < 3; i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!wgsize[i].has_value()) {
                    diagnostics_.AddError(Source{})
                        << "override-expressions should have been removed with the "
                           "SubstituteOverride transform";
                    return false;
                }
                out << std::to_string(wgsize[i].value());
            }
            out << ")]\n";
        }

        if (!EmitTypeAndName(out, func_sem->ReturnType(), core::AddressSpace::kUndefined,
                             core::Access::kUndefined, func->name->symbol.Name())) {
            return false;
        }
        out << "(";

        bool first = true;

        // Emit entry point parameters.
        for (auto* var : func->params) {
            auto* sem = builder_.Sem().Get(var);
            auto* type = sem->Type();
            if (DAWN_UNLIKELY(!type->Is<core::type::Struct>())) {
                // ICE likely indicates that the CanonicalizeEntryPointIO transform was
                // not run, or a builtin parameter was added after it was run.
                TINT_ICE() << "Unsupported non-struct entry point parameter";
            }

            if (!first) {
                out << ", ";
            }
            first = false;

            if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(),
                                 var->name->symbol.Name())) {
                return false;
            }
        }

        out << ") {";
    }

    {
        ScopedIndent si(this);

        if (!EmitStatements(func->body->statements)) {
            return false;
        }

        if (!Is<ast::ReturnStatement>(func->body->Last())) {
            ast::ReturnStatement ret(GenerationID(), ast::NodeID{}, Source{});
            if (!EmitStatement(&ret)) {
                return false;
            }
        }
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitConstant(StringStream& out,
                              const core::constant::Value* constant,
                              bool is_variable_initializer) {
    return Switch(
        constant->Type(),  //
        [&](const core::type::Bool*) {
            out << (constant->ValueAs<AInt>() ? "true" : "false");
            return true;
        },
        [&](const core::type::F32*) {
            PrintF32(out, constant->ValueAs<f32>());
            return true;
        },
        [&](const core::type::F16*) {
            // emit a f16 scalar with explicit float16_t type declaration.
            out << "float16_t(";
            PrintF16(out, constant->ValueAs<f16>());
            out << ")";
            return true;
        },
        [&](const core::type::I32*) {
            out << constant->ValueAs<AInt>();
            return true;
        },
        [&](const core::type::U32*) {
            out << constant->ValueAs<AInt>() << "u";
            return true;
        },
        [&](const core::type::Vector* v) {
            if (auto* splat = constant->As<core::constant::Splat>()) {
                {
                    ScopedParen sp(out);
                    if (!EmitConstant(out, splat->el, is_variable_initializer)) {
                        return false;
                    }
                }
                out << ".";
                for (size_t i = 0; i < v->Width(); i++) {
                    out << "x";
                }
                return true;
            }

            if (!EmitType(out, v, core::AddressSpace::kUndefined, core::Access::kUndefined, "")) {
                return false;
            }

            ScopedParen sp(out);

            for (size_t i = 0; i < v->Width(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Matrix* m) {
            if (!EmitType(out, m, core::AddressSpace::kUndefined, core::Access::kUndefined, "")) {
                return false;
            }

            ScopedParen sp(out);

            for (size_t i = 0; i < m->Columns(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Array* a) {
            if (constant->AllZero()) {
                out << "(";
                if (!EmitType(out, a, core::AddressSpace::kUndefined, core::Access::kUndefined,
                              "")) {
                    return false;
                }
                out << ")0";
                return true;
            }

            out << "{";
            TINT_DEFER(out << "}");

            auto count = a->ConstantCount();
            if (!count) {
                diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                return false;
            }

            for (size_t i = 0; i < count; i++) {
                if (i > 0) {
                    out << ", ";
                }
                if (!EmitConstant(out, constant->Index(i), is_variable_initializer)) {
                    return false;
                }
            }

            return true;
        },
        [&](const core::type::Struct* s) {
            if (!EmitStructType(&helpers_, s)) {
                return false;
            }

            if (constant->AllZero()) {
                out << "(" << StructName(s) << ")0";
                return true;
            }

            auto emit_member_values = [&](StringStream& o) {
                o << "{";
                for (size_t i = 0; i < s->Members().Length(); i++) {
                    if (i > 0) {
                        o << ", ";
                    }
                    if (!EmitConstant(o, constant->Index(i), is_variable_initializer)) {
                        return false;
                    }
                }
                o << "}";
                return true;
            };

            if (is_variable_initializer) {
                if (!emit_member_values(out)) {
                    return false;
                }
            } else {
                // HLSL requires structure initializers to be assigned directly to a variable.
                // For these constants use 'static const' at global-scope. 'const' at global scope
                // creates a variable who's initializer is ignored, and the value is expected to be
                // provided in a cbuffer. 'static const' is a true value-embedded-in-the-shader-code
                // constant. We also emit these for function-local constant expressions for
                // consistency and to ensure that these are not computed at execution time.
                auto name = UniqueIdentifier("c");
                {
                    StringStream decl;
                    decl << "static const " << StructName(s) << " " << name << " = ";
                    if (!emit_member_values(decl)) {
                        return false;
                    }
                    decl << ";";
                    current_buffer_->Insert(decl.str(), global_insertion_point_++, 0);
                }
                out << name;
            }

            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitLiteral(StringStream& out, const ast::LiteralExpression* lit) {
    return Switch(
        lit,
        [&](const ast::BoolLiteralExpression* l) {
            out << (l->value ? "true" : "false");
            return true;
        },
        [&](const ast::FloatLiteralExpression* l) {
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kH) {
                // Emit f16 literal with explicit float16_t type declaration.
                out << "float16_t(";
                PrintF16(out, static_cast<float>(l->value));
                out << ")";
            }
            PrintF32(out, static_cast<float>(l->value));
            return true;
        },
        [&](const ast::IntLiteralExpression* i) {
            out << i->value;
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                case ast::IntLiteralExpression::Suffix::kI:
                    return true;
                case ast::IntLiteralExpression::Suffix::kU:
                    out << "u";
                    return true;
            }
            diagnostics_.AddError(Source{}) << "unknown integer literal suffix type";
            return false;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitValue(StringStream& out, const core::type::Type* type, int value) {
    return Switch(
        type,
        [&](const core::type::Bool*) {
            out << (value == 0 ? "false" : "true");
            return true;
        },
        [&](const core::type::F32*) {
            out << value << ".0f";
            return true;
        },
        [&](const core::type::F16*) {
            out << "float16_t(" << value << ".0h)";
            return true;
        },
        [&](const core::type::I32*) {
            out << value;
            return true;
        },
        [&](const core::type::U32*) {
            out << value << "u";
            return true;
        },
        [&](const core::type::Vector* vec) {
            if (!EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite,
                          "")) {
                return false;
            }
            ScopedParen sp(out);
            for (uint32_t i = 0; i < vec->Width(); i++) {
                if (i != 0) {
                    out << ", ";
                }
                if (!EmitValue(out, vec->Type(), value)) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Matrix* mat) {
            if (!EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kReadWrite,
                          "")) {
                return false;
            }
            ScopedParen sp(out);
            for (uint32_t i = 0; i < (mat->Rows() * mat->Columns()); i++) {
                if (i != 0) {
                    out << ", ";
                }
                if (!EmitValue(out, mat->Type(), value)) {
                    return false;
                }
            }
            return true;
        },
        [&](const core::type::Struct*) {
            out << "(";
            TINT_DEFER(out << ")" << value);
            return EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined,
                            "");
        },
        [&](const core::type::Array*) {
            out << "(";
            TINT_DEFER(out << ")" << value);
            return EmitType(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined,
                            "");
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitZeroValue(StringStream& out, const core::type::Type* type) {
    return EmitValue(out, type, 0);
}

bool ASTPrinter::EmitLoop(const ast::LoopStatement* stmt) {
    auto emit_continuing = [this, stmt] {
        if (stmt->continuing && !stmt->continuing->Empty()) {
            if (!EmitBlock(stmt->continuing)) {
                return false;
            }
        }
        return true;
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    Line() << "while (true) {";
    {
        ScopedIndent si(this);
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
        if (!emit_continuing_()) {
            return false;
        }
    }
    Line() << "}";

    return true;
}

bool ASTPrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
    // Nest a for loop with a new block. In HLSL the initializer scope is not
    // nested by the for-loop, so we may get variable redefinitions.
    Line() << "{";
    IncrementIndent();
    TINT_DEFER({
        DecrementIndent();
        Line() << "}";
    });

    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        if (!EmitStatement(init)) {
            return false;
        }
    }

    TextBuffer cond_pre;
    StringStream cond_buf;
    if (auto* cond = stmt->condition) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        if (!EmitExpression(cond_buf, cond)) {
            return false;
        }
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        if (!EmitStatement(cont)) {
            return false;
        }
    }

    // If the for-loop has a multi-statement conditional and / or continuing, then
    // we cannot emit this as a regular for-loop in HLSL. Instead we need to
    // generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1;

    // If the for-loop has multi-statement initializer, or is going to be emitted
    // as a `while(true)` loop, then declare the initializer statement(s) before
    // the loop.
    if (init_buf.lines.size() > 1 || (stmt->initializer && emit_as_loop)) {
        current_buffer_->Append(init_buf);
        init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
    }

    if (emit_as_loop) {
        auto emit_continuing = [&] {
            current_buffer_->Append(cont_buf);
            return true;
        };

        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
        Line() << "while (true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        if (stmt->condition) {
            current_buffer_->Append(cond_pre);
            Line() << "if (!(" << cond_buf.str() << ")) { break; }";
        }

        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }

        if (!emit_continuing_()) {
            return false;
        }
    } else {
        // For-loop can be generated.
        {
            auto out = Line();
            out << "for";
            {
                ScopedParen sp(out);

                if (!init_buf.lines.empty()) {
                    out << init_buf.lines[0].content << " ";
                } else {
                    out << "; ";
                }

                out << cond_buf.str() << "; ";

                if (!cont_buf.lines.empty()) {
                    out << tint::TrimSuffix(cont_buf.lines[0].content, ";");
                }
            }
            out << " {";
        }
        {
            auto emit_continuing = [] { return true; };
            TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
            if (!EmitStatementsWithIndent(stmt->body->statements)) {
                return false;
            }
        }
        Line() << "}";
    }

    return true;
}

bool ASTPrinter::EmitWhile(const ast::WhileStatement* stmt) {
    TextBuffer cond_pre;
    StringStream cond_buf;
    {
        auto* cond = stmt->condition;
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
        if (!EmitExpression(cond_buf, cond)) {
            return false;
        }
    }

    auto emit_continuing = [&] { return true; };
    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

    // If the while has a multi-statement conditional, then we cannot emit this
    // as a regular while in HLSL. Instead we need to generate a `while(true)` loop.
    bool emit_as_loop = cond_pre.lines.size() > 0;
    if (emit_as_loop) {
        Line() << "while (true) {";
        IncrementIndent();
        TINT_DEFER({
            DecrementIndent();
            Line() << "}";
        });

        current_buffer_->Append(cond_pre);
        Line() << "if (!(" << cond_buf.str() << ")) { break; }";
        if (!EmitStatements(stmt->body->statements)) {
            return false;
        }
    } else {
        // While can be generated.
        {
            auto out = Line();
            out << "while";
            {
                ScopedParen sp(out);
                out << cond_buf.str();
            }
            out << " {";
        }
        if (!EmitStatementsWithIndent(stmt->body->statements)) {
            return false;
        }
        Line() << "}";
    }

    return true;
}

bool ASTPrinter::EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr) {
    if (!EmitExpression(out, expr->object)) {
        return false;
    }
    out << ".";

    auto* sem = builder_.Sem().Get(expr)->UnwrapLoad();

    return Switch(
        sem,
        [&](const sem::Swizzle*) {
            // Swizzles output the name directly
            out << expr->member->symbol.Name();
            return true;
        },
        [&](const sem::StructMemberAccess* member_access) {
            out << member_access->Member()->Name().Name();
            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    if (stmt->value) {
        auto out = Line();
        out << "return ";
        if (!EmitExpression(out, stmt->value)) {
            return false;
        }
        out << ";";
    } else {
        Line() << "return;";
    }
    return true;
}

bool ASTPrinter::EmitStatement(const ast::Statement* stmt) {
    return Switch(
        stmt,
        [&](const ast::AssignmentStatement* a) {  //
            return EmitAssign(a);
        },
        [&](const ast::BlockStatement* b) {  //
            return EmitBlock(b);
        },
        [&](const ast::BreakStatement* b) {  //
            return EmitBreak(b);
        },
        [&](const ast::BreakIfStatement* b) {  //
            return EmitBreakIf(b);
        },
        [&](const ast::CallStatement* c) {  //
            auto out = Line();
            if (!EmitCall(out, c->expr)) {
                return false;
            }
            out << ";";
            return true;
        },
        [&](const ast::ContinueStatement* c) {  //
            return EmitContinue(c);
        },
        [&](const ast::DiscardStatement* d) {  //
            return EmitDiscard(d);
        },
        [&](const ast::IfStatement* i) {  //
            return EmitIf(i);
        },
        [&](const ast::LoopStatement* l) {  //
            return EmitLoop(l);
        },
        [&](const ast::ForLoopStatement* l) {  //
            return EmitForLoop(l);
        },
        [&](const ast::WhileStatement* l) {  //
            return EmitWhile(l);
        },
        [&](const ast::ReturnStatement* r) {  //
            return EmitReturn(r);
        },
        [&](const ast::SwitchStatement* s) {  //
            return EmitSwitch(s);
        },
        [&](const ast::VariableDeclStatement* v) {  //
            return Switch(
                v->variable,  //
                [&](const ast::Var* var) { return EmitVar(var); },
                [&](const ast::Let* let) { return EmitLet(let); },
                [&](const ast::Const*) {
                    return true;  // Constants are embedded at their use
                },                //
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },                //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitDefaultOnlySwitch(const ast::SwitchStatement* stmt) {
    TINT_ASSERT(stmt->body.Length() == 1 && stmt->body[0]->ContainsDefault());

    // FXC fails to compile a switch with just a default case, ignoring the
    // default case body. We work around this here by emitting the default case
    // without the switch.

    // Emit the switch condition as-is if it has side-effects (e.g.
    // function call). Note that we can ignore the result of the expression (if any).
    if (auto* sem_cond = builder_.Sem().GetVal(stmt->condition); sem_cond->HasSideEffects()) {
        auto out = Line();
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ";";
    }

    // Emit "do { <default case body> } while(false);". We use a 'do' loop so
    // that break statements work as expected, and make it 'while (false)' in
    // case there isn't a break statement.
    Line() << "do {";
    {
        ScopedIndent si(this);
        if (!EmitStatements(stmt->body[0]->body->statements)) {
            return false;
        }
    }
    Line() << "} while (false);";
    return true;
}

bool ASTPrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    // BUG(crbug.com/tint/1188): work around default-only switches
    if (stmt->body.Length() == 1 && stmt->body[0]->selectors.Length() == 1 &&
        stmt->body[0]->ContainsDefault()) {
        return EmitDefaultOnlySwitch(stmt);
    }

    {  // switch(expr) {
        auto out = Line();
        out << "switch(";
        if (!EmitExpression(out, stmt->condition)) {
            return false;
        }
        out << ") {";
    }

    {
        ScopedIndent si(this);
        for (size_t i = 0; i < stmt->body.Length(); i++) {
            if (!EmitCase(stmt, i)) {
                return false;
            }
        }
    }

    Line() << "}";

    return true;
}

bool ASTPrinter::EmitType(StringStream& out,
                          const core::type::Type* type,
                          core::AddressSpace address_space,
                          core::Access access,
                          const std::string& name,
                          bool* name_printed /* = nullptr */) {
    if (name_printed) {
        *name_printed = false;
    }
    switch (address_space) {
        case core::AddressSpace::kStorage:
            if (access != core::Access::kRead) {
                out << "RW";
            }
            out << "ByteAddressBuffer";
            return true;
        case core::AddressSpace::kUniform: {
            auto array_length = (type->Size() + 15) / 16;
            out << "uint4 " << name << "[" << array_length << "]";
            if (name_printed) {
                *name_printed = true;
            }
            return true;
        }
        default:
            break;
    }

    return Switch(
        type,
        [&](const core::type::Array* ary) {
            const core::type::Type* base_type = ary;
            std::vector<uint32_t> sizes;
            while (auto* arr = base_type->As<core::type::Array>()) {
                if (DAWN_UNLIKELY(arr->Count()->Is<core::type::RuntimeArrayCount>())) {
                    TINT_ICE()
                        << "runtime arrays may only exist in storage buffers, which should have "
                           "been transformed into a ByteAddressBuffer";
                }
                const auto count = arr->ConstantCount();
                if (!count) {
                    diagnostics_.AddError(Source{}) << core::type::Array::kErrExpectedConstantCount;
                    return false;
                }

                sizes.push_back(count.value());
                base_type = arr->ElemType();
            }
            if (!EmitType(out, base_type, address_space, access, "")) {
                return false;
            }
            if (!name.empty()) {
                out << " " << name;
                if (name_printed) {
                    *name_printed = true;
                }
            }
            for (uint32_t size : sizes) {
                out << "[" << size << "]";
            }
            return true;
        },
        [&](const core::type::Bool*) {
            out << "bool";
            return true;
        },
        [&](const core::type::F32*) {
            out << "float";
            return true;
        },
        [&](const core::type::F16*) {
            out << "float16_t";
            return true;
        },
        [&](const core::type::I32*) {
            out << "int";
            return true;
        },
        [&](const core::type::Matrix* mat) {
            if (mat->Type()->Is<core::type::F16>()) {
                // Use matrix<type, N, M> for f16 matrix
                out << "matrix<";
                if (!EmitType(out, mat->Type(), address_space, access, "")) {
                    return false;
                }
                out << ", " << mat->Columns() << ", " << mat->Rows() << ">";
                return true;
            }
            if (!EmitType(out, mat->Type(), address_space, access, "")) {
                return false;
            }
            // Note: HLSL's matrices are declared as <type>NxM, where N is the
            // number of rows and M is the number of columns. Despite HLSL's
            // matrices being column-major by default, the index operator and
            // initializers actually operate on row-vectors, where as WGSL operates
            // on column vectors. To simplify everything we use the transpose of the
            // matrices. See:
            // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
            out << mat->Columns() << "x" << mat->Rows();
            return true;
        },
        [&](const core::type::Pointer*) -> bool {
            TINT_ICE() << "Attempting to emit pointer type. These should have been removed with "
                          "the SimplifyPointers transform";
        },
        [&](const core::type::Sampler* sampler) {
            out << "Sampler";
            if (sampler->IsComparison()) {
                out << "Comparison";
            }
            out << "State";
            return true;
        },
        [&](const core::type::Struct* str) {
            out << StructName(str);
            return true;
        },
        [&](const core::type::Texture* tex) {
            if (DAWN_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
                TINT_ICE() << "Multiplanar external texture transform was not run.";
            }

            auto* storage = tex->As<core::type::StorageTexture>();
            auto* ms = tex->As<core::type::MultisampledTexture>();
            auto* depth_ms = tex->As<core::type::DepthMultisampledTexture>();
            auto* sampled = tex->As<core::type::SampledTexture>();

            if (storage && storage->Access() != core::Access::kRead) {
                out << "RW";
            }
            out << "Texture";

            switch (tex->Dim()) {
                case core::type::TextureDimension::k1d:
                    out << "1D";
                    break;
                case core::type::TextureDimension::k2d:
                    out << ((ms || depth_ms) ? "2DMS" : "2D");
                    break;
                case core::type::TextureDimension::k2dArray:
                    out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
                    break;
                case core::type::TextureDimension::k3d:
                    out << "3D";
                    break;
                case core::type::TextureDimension::kCube:
                    out << "Cube";
                    break;
                case core::type::TextureDimension::kCubeArray:
                    out << "CubeArray";
                    break;
                default:
                    TINT_UNREACHABLE() << "unexpected TextureDimension " << tex->Dim();
            }

            if (storage) {
                auto* component = ImageFormatToRWtextureType(storage->TexelFormat());
                if (DAWN_UNLIKELY(!component)) {
                    TINT_ICE() << "Unsupported StorageTexture TexelFormat: "
                               << static_cast<int>(storage->TexelFormat());
                }
                out << "<" << component << ">";
            } else if (depth_ms) {
                out << "<float4>";
            } else if (sampled || ms) {
                auto* subtype = sampled ? sampled->Type() : ms->Type();
                out << "<";
                if (subtype->Is<core::type::F32>()) {
                    out << "float4";
                } else if (subtype->Is<core::type::I32>()) {
                    out << "int4";
                } else if (DAWN_LIKELY(subtype->Is<core::type::U32>())) {
                    out << "uint4";
                } else {
                    TINT_ICE() << "Unsupported multisampled texture type";
                }
                out << ">";
            }
            return true;
        },
        [&](const core::type::U32*) {
            out << "uint";
            return true;
        },
        [&](const core::type::Vector* vec) {
            auto width = vec->Width();
            if (vec->Type()->Is<core::type::F32>() && width >= 1 && width <= 4) {
                out << "float" << width;
            } else if (vec->Type()->Is<core::type::I32>() && width >= 1 && width <= 4) {
                out << "int" << width;
            } else if (vec->Type()->Is<core::type::U32>() && width >= 1 && width <= 4) {
                out << "uint" << width;
            } else if (vec->Type()->Is<core::type::Bool>() && width >= 1 && width <= 4) {
                out << "bool" << width;
            } else {
                // For example, use "vector<float16_t, N>" for f16 vector.
                out << "vector<";
                if (!EmitType(out, vec->Type(), address_space, access, "")) {
                    return false;
                }
                out << ", " << width << ">";
            }
            return true;
        },
        [&](const core::type::Atomic* atomic) {
            return EmitType(out, atomic->Type(), address_space, access, name);
        },
        [&](const core::type::Void*) {
            out << "void";
            return true;
        },  //
        TINT_ICE_ON_NO_MATCH);
}

bool ASTPrinter::EmitTypeAndName(StringStream& out,
                                 const core::type::Type* type,
                                 core::AddressSpace address_space,
                                 core::Access access,
                                 const std::string& name) {
    bool name_printed = false;
    if (!EmitType(out, type, address_space, access, name, &name_printed)) {
        return false;
    }
    if (!name.empty() && !name_printed) {
        out << " " << name;
    }
    return true;
}

bool ASTPrinter::EmitStructType(TextBuffer* b,
                                const core::type::Struct* str,
                                VectorRef<const ast::StructMember*> ast_struct_members) {
    auto it = emitted_structs_.emplace(str);
    if (!it.second) {
        return true;
    }

    const auto struct_type_members = str->Members();
    size_t struct_type_member_length = struct_type_members.Length();
    TINT_ASSERT(ast_struct_members.IsEmpty() ||
                (struct_type_member_length == ast_struct_members.Length()));

    Line(b) << "struct " << StructName(str) << " {";
    {
        ScopedIndent si(b);
        for (size_t i = 0; i < struct_type_member_length; ++i) {
            auto* mem = struct_type_members[i];
            auto mem_name = mem->Name().Name();
            auto* ty = mem->Type();
            auto out = Line(b);
            std::string pre, post;

            auto& attributes = mem->Attributes();

            if (auto location = attributes.location) {
                auto& pipeline_stage_uses = str->PipelineStageUses();
                if (DAWN_UNLIKELY(pipeline_stage_uses.Count() != 1)) {
                    TINT_ICE() << "invalid entry point IO struct uses";
                }
                if (pipeline_stage_uses.Contains(core::type::PipelineStageUsage::kVertexInput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kVertexOutput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kFragmentInput)) {
                    post += " : TEXCOORD" + std::to_string(location.value());
                } else if (DAWN_LIKELY(pipeline_stage_uses.Contains(
                               core::type::PipelineStageUsage::kFragmentOutput))) {
                    if (auto blend_src = attributes.blend_src) {
                        post +=
                            " : SV_Target" + std::to_string(location.value() + blend_src.value());
                    } else {
                        post += " : SV_Target" + std::to_string(location.value());
                    }

                } else {
                    TINT_ICE() << "invalid use of location attribute";
                }
            }
            if (auto builtin = attributes.builtin) {
                auto name = builtin_to_attribute(builtin.value());
                if (name.empty()) {
                    diagnostics_.AddError(Source{}) << "unsupported builtin";
                    return false;
                }
                post += " : " + name;
            }
            if (auto interpolation = attributes.interpolation) {
                auto mod = interpolation_to_modifiers(interpolation->type, interpolation->sampling);
                if (mod.empty()) {
                    diagnostics_.AddError(Source{}) << "unsupported interpolation";
                    return false;
                }
                pre += mod;
            }
            if (attributes.invariant) {
                // Note: `precise` is not exactly the same as `invariant`, but is
                // stricter and therefore provides the necessary guarantees.
                // See discussion here: https://github.com/gpuweb/gpuweb/issues/893
                pre += "precise ";
            }
            if (!ast_struct_members.IsEmpty() &&
                ast::HasAttribute<ast::transform::CanonicalizeEntryPointIO::HLSLClipDistance1>(
                    ast_struct_members[i]->attributes)) {
                post += " : SV_ClipDistance1";
            }

            out << pre;
            if (!EmitTypeAndName(out, ty, core::AddressSpace::kUndefined, core::Access::kReadWrite,
                                 mem_name)) {
                return false;
            }
            out << post << ";";
        }
    }

    Line(b) << "};";
    return true;
}

bool ASTPrinter::EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case core::UnaryOp::kIndirection:
        case core::UnaryOp::kAddressOf:
            return EmitExpression(out, expr->expr);
        case core::UnaryOp::kComplement:
            out << "~";
            break;
        case core::UnaryOp::kNot:
            out << "!";
            break;
        case core::UnaryOp::kNegation:
            out << "-";
            break;
    }
    out << "(";

    if (!EmitExpression(out, expr->expr)) {
        return false;
    }

    out << ")";

    return true;
}

bool ASTPrinter::EmitVar(const ast::Var* var) {
    auto* sem = builder_.Sem().Get(var);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();
    if (!EmitTypeAndName(out, type, sem->AddressSpace(), sem->Access(), var->name->symbol.Name())) {
        return false;
    }

    out << " = ";

    if (var->initializer) {
        if (!EmitExpression(out, var->initializer)) {
            return false;
        }
    } else {
        if (!EmitZeroValue(out, type)) {
            return false;
        }
    }
    out << ";";

    return true;
}

bool ASTPrinter::EmitLet(const ast::Let* let) {
    auto* sem = builder_.Sem().Get(let);
    auto* type = sem->Type()->UnwrapRef();

    auto out = Line();
    if (!EmitTypeAndName(out, type, core::AddressSpace::kUndefined, core::Access::kUndefined,
                         let->name->symbol.Name())) {
        return false;
    }
    out << " = ";
    if (!EmitExpression(out, let->initializer)) {
        return false;
    }
    out << ";";

    return true;
}

template <typename F>
bool ASTPrinter::CallBuiltinHelper(StringStream& out,
                                   const ast::CallExpression* call,
                                   const sem::BuiltinFn* builtin,
                                   F&& build) {
    // Generate the helper function if it hasn't been created already
    auto fn = tint::GetOrAdd(builtins_, builtin, [&]() -> std::string {
        TextBuffer b;
        TINT_DEFER(helpers_.Append(b));

        auto fn_name = UniqueIdentifier(std::string("tint_") + wgsl::str(builtin->Fn()));
        std::vector<std::string> parameter_names;
        {
            auto decl = Line(&b);
            if (!EmitTypeAndName(decl, builtin->ReturnType(), core::AddressSpace::kUndefined,
                                 core::Access::kUndefined, fn_name)) {
                return "";
            }
            {
                ScopedParen sp(decl);
                for (auto* param : builtin->Parameters()) {
                    if (!parameter_names.empty()) {
                        decl << ", ";
                    }
                    auto param_name = "param_" + std::to_string(parameter_names.size());
                    const auto* ty = param->Type();
                    if (auto* ptr = ty->As<core::type::Pointer>()) {
                        decl << "inout ";
                        ty = ptr->StoreType();
                    }
                    if (!EmitTypeAndName(decl, ty, core::AddressSpace::kUndefined,
                                         core::Access::kUndefined, param_name)) {
                        return "";
                    }
                    parameter_names.emplace_back(std::move(param_name));
                }
            }
            decl << " {";
        }
        {
            ScopedIndent si(&b);
            if (!build(&b, parameter_names)) {
                return "";
            }
        }
        Line(&b) << "}";
        Line(&b);
        return fn_name;
    });

    if (fn.empty()) {
        return false;
    }

    // Call the helper
    out << fn;
    {
        ScopedParen sp(out);
        bool first = true;
        for (auto* arg : call->args) {
            if (!first) {
                out << ", ";
            }
            first = false;
            if (!EmitExpression(out, arg)) {
                return false;
            }
        }
    }
    return true;
}

std::string ASTPrinter::StructName(const core::type::Struct* s) {
    auto name = s->Name().Name();
    if (HasPrefix(name, "__")) {
        name = tint::GetOrAdd(builtin_struct_names_, s,
                              [&] { return UniqueIdentifier(name.substr(2)); });
    }
    return name;
}

std::string ASTPrinter::UniqueIdentifier(const std::string& prefix /* = "" */) {
    return builder_.Symbols().New(prefix).Name();
}

}  // namespace tint::hlsl::writer

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
