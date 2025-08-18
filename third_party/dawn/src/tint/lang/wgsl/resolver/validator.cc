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

#include "src/tint/lang/wgsl/resolver/validator.h"

#include <algorithm>
#include <bitset>
#include <limits>
#include <string_view>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/blend_src_attribute.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/break_if_statement.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/loop_statement.h"
#include "src/tint/lang/wgsl/sem/materialize.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::resolver {
namespace {

constexpr size_t kMaxFunctionParameters = 255;
constexpr size_t kMaxSwitchCaseSelectors = 16383;
constexpr size_t kMaxClipDistancesSize = 8;

bool IsValidStorageTextureDimension(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::k1d:
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::k2dArray:
        case core::type::TextureDimension::k3d:
            return true;
        default:
            return false;
    }
}

bool IsValidStorageTextureTexelFormat(core::TexelFormat format) {
    switch (format) {
        case core::TexelFormat::kR8Unorm:
        case core::TexelFormat::kR8Snorm:
        case core::TexelFormat::kR8Uint:
        case core::TexelFormat::kR8Sint:
        case core::TexelFormat::kRg8Unorm:
        case core::TexelFormat::kRg8Snorm:
        case core::TexelFormat::kRg8Uint:
        case core::TexelFormat::kRg8Sint:
        case core::TexelFormat::kR16Unorm:
        case core::TexelFormat::kR16Snorm:
        case core::TexelFormat::kR16Uint:
        case core::TexelFormat::kR16Sint:
        case core::TexelFormat::kR16Float:
        case core::TexelFormat::kRg16Unorm:
        case core::TexelFormat::kRg16Snorm:
        case core::TexelFormat::kRg16Uint:
        case core::TexelFormat::kRg16Sint:
        case core::TexelFormat::kRg16Float:
        case core::TexelFormat::kBgra8Unorm:
        case core::TexelFormat::kR32Float:
        case core::TexelFormat::kR32Sint:
        case core::TexelFormat::kR32Uint:
        case core::TexelFormat::kRg32Float:
        case core::TexelFormat::kRg32Sint:
        case core::TexelFormat::kRg32Uint:
        case core::TexelFormat::kRgba16Unorm:
        case core::TexelFormat::kRgba16Snorm:
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
        case core::TexelFormat::kRgb10A2Uint:
        case core::TexelFormat::kRgb10A2Unorm:
        case core::TexelFormat::kRg11B10Ufloat:
            return true;
        default:
            return false;
    }
}

template <typename CALLBACK>
void TraverseCallChain(const sem::Function* from, const sem::Function* to, CALLBACK&& callback) {
    for (auto* f : from->TransitivelyCalledFunctions()) {
        if (f == to) {
            callback(f);
            return;
        }
        if (f->TransitivelyCalledFunctions().Contains(to)) {
            TraverseCallChain(f, to, callback);
            callback(f);
            return;
        }
    }
    TINT_ICE() << "TraverseCallChain() 'from' does not transitively call 'to'";
}

}  // namespace

Validator::Validator(
    ProgramBuilder* builder,
    SemHelper& sem,
    const wgsl::Extensions& enabled_extensions,
    const wgsl::AllowedFeatures& allowed_features,
    const Hashmap<const core::type::Type*, const Source*, 8>& atomic_composite_info,
    Hashset<TypeAndAddressSpace, 8>& valid_type_storage_layouts)
    : symbols_(builder->Symbols()),
      diagnostics_(builder->Diagnostics()),
      sem_(sem),
      enabled_extensions_(enabled_extensions),
      allowed_features_(allowed_features),
      atomic_composite_info_(atomic_composite_info),
      valid_type_storage_layouts_(valid_type_storage_layouts) {
    // Set default severities for filterable diagnostic rules.
    diagnostic_filters_.Set(wgsl::CoreDiagnosticRule::kDerivativeUniformity,
                            wgsl::DiagnosticSeverity::kError);
    diagnostic_filters_.Set(wgsl::CoreDiagnosticRule::kSubgroupUniformity,
                            wgsl::DiagnosticSeverity::kError);
    diagnostic_filters_.Set(wgsl::ChromiumDiagnosticRule::kSubgroupMatrixUniformity,
                            wgsl::DiagnosticSeverity::kError);
    diagnostic_filters_.Set(wgsl::ChromiumDiagnosticRule::kUnreachableCode,
                            wgsl::DiagnosticSeverity::kWarning);
}

Validator::~Validator() = default;

diag::Diagnostic& Validator::AddError(const Source& source) const {
    return diagnostics_.AddError(source);
}

diag::Diagnostic& Validator::AddWarning(const Source& source) const {
    return diagnostics_.AddWarning(source);
}

diag::Diagnostic& Validator::AddNote(const Source& source) const {
    return diagnostics_.AddNote(source);
}

diag::Diagnostic* Validator::MaybeAddDiagnostic(wgsl::DiagnosticRule rule,
                                                const Source& source) const {
    auto severity = diagnostic_filters_.Get(rule);
    if (severity != wgsl::DiagnosticSeverity::kOff) {
        diag::Diagnostic d{};
        d.severity = ToSeverity(severity);
        d.source = source;
        return &diagnostics_.Add(std::move(d));
    }
    return nullptr;
}

// https://gpuweb.github.io/gpuweb/wgsl/#plain-types-section
bool Validator::IsPlain(const core::type::Type* type) const {
    return type->IsAnyOf<core::type::Scalar, core::type::Atomic, core::type::Vector,
                         core::type::Matrix, sem::Array, core::type::Struct,
                         core::type::SubgroupMatrix>() &&
           !type->IsAnyOf<core::type::I8, core::type::U8>();
}

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Validator::IsStorable(const core::type::Type* type) const {
    return IsPlain(type) ||
           type->IsAnyOf<core::type::BindingArray, core::type::Texture, core::type::Sampler>();
}

const ast::Statement* Validator::ClosestContinuing(bool stop_at_loop,
                                                   bool stop_at_switch,
                                                   sem::Statement* current_statement) const {
    for (const auto* s = current_statement; s != nullptr; s = s->Parent()) {
        if (stop_at_loop && s->Is<sem::LoopStatement>()) {
            break;
        }
        if (stop_at_switch && s->Is<sem::SwitchStatement>()) {
            break;
        }
        if (s->Is<sem::LoopContinuingBlockStatement>()) {
            return s->Declaration();
        }
        if (auto* f = As<sem::ForLoopStatement>(s->Parent())) {
            if (f->Declaration()->continuing == s->Declaration()) {
                return s->Declaration();
            }
            if (stop_at_loop) {
                break;
            }
        }
        if (Is<sem::WhileStatement>(s->Parent())) {
            if (stop_at_loop) {
                break;
            }
        }
    }
    return nullptr;
}

bool Validator::Enables(VectorRef<const ast::Enable*> enables) const {
    auto source_of = [&](wgsl::Extension ext) {
        for (auto* enable : enables) {
            for (auto* extension : enable->extensions) {
                if (extension->name == ext) {
                    return extension->source;
                }
            }
        }
        return Source{};
    };

    // List of extensions that cannot be used together.
    std::pair<wgsl::Extension, wgsl::Extension> incompatible[] = {
        {
            wgsl::Extension::kChromiumExperimentalPixelLocal,
            wgsl::Extension::kChromiumExperimentalFramebufferFetch,
        },
    };

    for (auto pair : incompatible) {
        if (enabled_extensions_.Contains(pair.first) && enabled_extensions_.Contains(pair.second)) {
            AddError(source_of(pair.first))
                << "extension " << style::Code(pair.first) << " cannot be used with extension "
                << style::Code(pair.second);
            AddNote(source_of(pair.second)) << style::Code(pair.second) << " enabled here";
            return false;
        }
    }

    return true;
}

bool Validator::Atomic(const ast::TemplatedIdentifier* a, const core::type::Atomic* s) const {
    // https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
    // T must be either u32 or i32.
    if (!s->Type()->IsAnyOf<core::type::U32, core::type::I32>()) {
        AddError(a->arguments[0]->source)
            << style::Type("atomic") << " only supports " << style::Type("i32") << " or "
            << style::Type("u32") << " types";
        return false;
    }
    return true;
}

bool Validator::Pointer(const ast::TemplatedIdentifier* a, const core::type::Pointer* s) const {
    if (s->AddressSpace() == core::AddressSpace::kUndefined) {
        AddError(a->source) << "ptr missing address space";
        return false;
    }

    if (s->AddressSpace() != core::AddressSpace::kHandle) {
        if (s->StoreType()->IsHandle()) {
            AddError(a->source) << "pointer can not be formed to handle type "
                                << sem_.TypeNameOf(s->StoreType());
            return false;
        }
    }

    if (a->arguments.Length() > 2) {  // ptr<address-space, type [, access]>
        // https://www.w3.org/TR/WGSL/#access-mode-defaults
        // When writing a variable declaration or a pointer type in WGSL source:
        // * For the storage address space, the access mode is optional, and defaults to read.
        // * For other address spaces, the access mode must not be written.
        if (s->AddressSpace() != core::AddressSpace::kStorage) {
            AddError(a->source)
                << "only pointers in <storage> address space may specify an access mode";
            return false;
        }
    }

    if (auto* store_ty = s->StoreType(); !IsStorable(store_ty)) {
        AddError(a->arguments[1]->source)
            << sem_.TypeNameOf(store_ty) + " cannot be used as the store type of a pointer";
        return false;
    }

    return CheckTypeAccessAddressSpace(s->StoreType(), s->Access(), s->AddressSpace(), a->source);
}

bool Validator::StorageTexture(const core::type::StorageTexture* t, const Source& source) const {
    switch (t->Access()) {
        case core::Access::kRead:
            if (allowed_features_.features.count(
                    wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures) == 0u) {
                AddError(source) << "read-only storage textures require the "
                                    "readonly_and_readwrite_storage_textures language feature, "
                                    "which is not allowed in the current environment";
                return false;
            }
            break;
        case core::Access::kReadWrite:
            if (allowed_features_.features.count(
                    wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures) == 0u) {
                AddError(source) << "read-write storage textures require the "
                                    "readonly_and_readwrite_storage_textures language feature, "
                                    "which is not allowed in the current environment";
                return false;
            }
            break;
        case core::Access::kWrite:
            break;
        case core::Access::kUndefined:
            AddError(source) << "storage texture missing access control";
            return false;
    }

    if (!IsValidStorageTextureDimension(t->Dim())) {
        AddError(source) << "cube dimensions for storage textures are not supported";
        return false;
    }

    if (!IsValidStorageTextureTexelFormat(t->TexelFormat())) {
        AddError(source) << "image format must be one of the texel formats specified for storage "
                            "textures in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats";
        return false;
    }

    return true;
}

bool Validator::SampledTexture(const core::type::SampledTexture* t, const Source& source) const {
    if (!t->Type()->UnwrapRef()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        AddError(source) << "texture_2d<type>: type must be f32, i32 or u32";
        return false;
    }

    return true;
}

bool Validator::MultisampledTexture(const core::type::MultisampledTexture* t,
                                    const Source& source) const {
    if (t->Dim() != core::type::TextureDimension::k2d) {
        AddError(source) << "only 2d multisampled textures are supported";
        return false;
    }

    if (!t->Type()->UnwrapRef()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        AddError(source) << "texture_multisampled_2d<type>: type must be f32, i32 or u32";
        return false;
    }

    return true;
}

bool Validator::InputAttachment(const core::type::InputAttachment* t, const Source& source) const {
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumInternalInputAttachments)) {
        AddError(source) << "use of " << style::Type("input_attachment")
                         << " requires enabling extension "
                         << style::Code("chromium_internal_input_attachments");
        return false;
    }
    if (!t->Type()->UnwrapRef()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        AddError(source) << "input_attachment<type>: type must be f32, i32 or u32";
        return false;
    }

    return true;
}

bool Validator::InputAttachmentIndexAttribute(const ast::InputAttachmentIndexAttribute* attr,
                                              const core::type::Type* type,
                                              const Source& source) const {
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumInternalInputAttachments)) {
        AddError(source) << "use of " << style::Attribute("@input_attachment_index")
                         << " requires enabling extension "
                         << style::Code("chromium_internal_input_attachments");
        return false;
    }

    if (!type->Is<core::type::InputAttachment>()) {
        std::string invalid_type = sem_.TypeNameOf(type);
        AddError(source) << "cannot apply " << style::Attribute("@input_attachment_index")
                         << " to declaration of type " << style::Type(invalid_type);
        AddNote(attr->source) << style::Attribute("@input_attachment_index")
                              << " must only be applied to declarations of "
                              << style::Type("input_attachment") << " type";
        return false;
    }

    return true;
}

bool Validator::BindingArray(const core::type::BindingArray* t, const Source& source) const {
    if (allowed_features_.features.count(wgsl::LanguageFeature::kSizedBindingArray) == 0) {
        AddError(source) << "use of " << style::Type("binding_array") << " requires the "
                         << style::Code("sized_binding_array")
                         << "language feature, which is not allowed in the current environment";
        return false;
    }
    if (!t->Count()->Is<core::type::ConstantArrayCount>()) {
        AddError(source) << "binding_array count must be a constant expression";
        return false;
    }

    if (!t->ElemType()->Is<core::type::SampledTexture>()) {
        AddError(source) << "binding_array element type must be a sampled texture type";
        return false;
    }

    return true;
}

bool Validator::SubgroupMatrix(const core::type::SubgroupMatrix* t, const Source& source) const {
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalSubgroupMatrix)) {
        AddError(source) << "use of " << style::Type("subgroup_matrix_*")
                         << " requires enabling extension "
                         << style::Code("chromium_experimental_subgroup_matrix");
        return false;
    }
    if (!t->Type()
             ->IsAnyOf<core::type::F32, core::type::F16, core::type::I32, core::type::U32,
                       core::type::I8, core::type::U8>()) {
        AddError(source) << "subgroup_matrix element type must be f32, f16, i32, u32, i8 or u8";
        return false;
    }

    return true;
}

bool Validator::TexelBuffer(const core::type::TexelBuffer* t, const Source& source) const {
    if (!allowed_features_.features.count(wgsl::LanguageFeature::kTexelBuffers)) {
        AddError(source) << "use of " << style::Type("texel_buffer")
                         << " requires the texel_buffer language feature, which is not allowed "
                            "in the current environment";
        return false;
    }

    if (!IsValidStorageTextureTexelFormat(t->TexelFormat())) {
        AddError(source) << "texel_buffer format must be one of the texel formats specified for "
                            "texel buffers in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats";
        return false;
    }

    if (!t->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        AddError(source) << "texel_buffer<format, access>: storage subtype must be f32, i32 or u32";
        return false;
    }

    if (t->Dim() != core::type::TextureDimension::k1d) {
        AddError(source) << "texel_buffer must be a 1d texture";
        return false;
    }

    if (t->Access() != core::Access::kRead && t->Access() != core::Access::kReadWrite) {
        AddError(source) << "texel_buffer access must be read or read_write";
        return false;
    }

    return true;
}

bool Validator::Materialize(const core::type::Type* to,
                            const core::type::Type* from,
                            const Source& source) const {
    if (core::type::Type::ConversionRank(from, to) == core::type::Type::kNoConversion) {
        AddError(source) << "cannot convert value of type " << style::Type(sem_.TypeNameOf(from))
                         << " to type " << style::Type(sem_.TypeNameOf(to));
        return false;
    }
    return true;
}

bool Validator::VariableInitializer(const ast::Variable* v,
                                    const core::type::Type* storage_ty,
                                    const sem::ValueExpression* initializer) const {
    auto* initializer_ty = initializer->Type();
    auto* value_type = initializer_ty->UnwrapRef();  // Implicit load of RHS

    // Value type has to match storage type
    if (storage_ty != value_type) {
        AddError(v->source) << "cannot initialize " << style::Keyword(v->Kind()) << " of type "
                            << style::Type(sem_.TypeNameOf(storage_ty)) << " with value of type "
                            << style::Type(sem_.TypeNameOf(initializer_ty));
        return false;
    }

    return true;
}

bool Validator::AddressSpaceLayout(const core::type::Type* store_ty,
                                   core::AddressSpace address_space,
                                   Source source) const {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class-layout-constraints

    auto is_uniform_struct_or_array = [address_space](const core::type::Type* ty) {
        return address_space == core::AddressSpace::kUniform &&
               ty->IsAnyOf<sem::Array, core::type::Struct>();
    };

    auto is_uniform_struct = [address_space](const core::type::Type* ty) {
        return address_space == core::AddressSpace::kUniform && ty->Is<core::type::Struct>();
    };

    auto required_alignment_of = [&](const core::type::Type* ty) {
        uint32_t actual_align = ty->Align();
        uint32_t required_align = actual_align;
        if (is_uniform_struct_or_array(ty)) {
            required_align = tint::RoundUp(16u, actual_align);
        }
        return required_align;
    };

    auto member_name_of = [](const core::type::StructMember* sm) { return sm->Name().Name(); };

    // Only validate the [type + address space] once
    if (!valid_type_storage_layouts_.Add(TypeAndAddressSpace{store_ty, address_space})) {
        return true;
    }

    auto note_usage = [&] {
        AddNote(source) << style::Type(store_ty->FriendlyName()) << " used in address space "
                        << style::Enum(address_space) << " here";
    };

    // Among three host-shareable address spaces, f16 is supported in "uniform" and
    // "storage" address space, but not "immediate" address space yet.
    if (Is<core::type::F16>(store_ty->DeepestElement()) &&
        address_space == core::AddressSpace::kImmediate) {
        AddError(source) << "using " << style::Type("f16") << " in " << style::Enum("immediate")
                         << " address space is not implemented yet";
        return false;
    }

    if (auto* str = store_ty->As<sem::Struct>()) {
        auto& str_source = str->Declaration()->name->source;
        for (size_t i = 0; i < str->Members().Length(); ++i) {
            auto* const m = str->Members()[i];
            uint32_t required_align = required_alignment_of(m->Type());

            // Recurse into the member type.
            if (!AddressSpaceLayout(m->Type(), address_space, m->Declaration()->type->source)) {
                AddNote(str_source) << "see layout of struct:\n" << str->Layout();
                note_usage();
                return false;
            }

            // Validate that member is at a valid byte offset
            if (m->Offset() % required_align != 0) {
                AddError(m->Declaration()->source)
                    << "the offset of a struct member of type "
                    << style::Type(m->Type()->UnwrapRef()->FriendlyName()) << " in address space "
                    << style::Enum(address_space) << " must be a multiple of " << required_align
                    << " bytes, but " << style::Variable(member_name_of(m))
                    << " is currently at offset " << m->Offset() << ". Consider setting "
                    << style::Attribute("@align") << style::Code("(", required_align, ")")
                    << " on this member";

                AddNote(str_source) << "see layout of struct:\n" << str->Layout();

                if (auto* member_str = m->Type()->As<sem::Struct>()) {
                    AddNote(member_str->Declaration()->name->source)
                        << "and layout of struct member:\n"
                        << member_str->Layout();
                }

                note_usage();
                return false;
            }

            // For uniform buffers, validate that the number of bytes between the previous member of
            // type struct and the current is a multiple of 16 bytes.
            auto* const prev_member = (i == 0) ? nullptr : str->Members()[i - 1];
            if (prev_member && is_uniform_struct(prev_member->Type())) {
                const uint32_t prev_to_curr_offset = m->Offset() - prev_member->Offset();
                if (prev_to_curr_offset % 16 != 0) {
                    AddError(m->Declaration()->source)
                        << style::Enum("uniform")
                        << " storage requires that the number of bytes between the start of the "
                           "previous member of type struct and the current member be a "
                           "multiple of 16 bytes, but there are currently "
                        << prev_to_curr_offset << " bytes between "
                        << style::Variable(member_name_of(prev_member)) << " and "
                        << style::Variable(member_name_of(m)) << ". Consider setting "
                        << style::Attribute("@align") << style::Code("(16)") << " on this member";

                    AddNote(str_source) << "see layout of struct:\n" << str->Layout();

                    auto* prev_member_str = prev_member->Type()->As<sem::Struct>();
                    AddNote(prev_member_str->Declaration()->name->source)
                        << "and layout of previous member struct:\n"
                        << prev_member_str->Layout();
                    note_usage();
                    return false;
                }
            }

            // If an alignment was explicitly specified, we need to validate that it satisfies the
            // alignment requirement of the address space.
            auto* align_attr =
                ast::GetAttribute<ast::StructMemberAlignAttribute>(m->Declaration()->attributes);
            if (align_attr != nullptr) {
                auto align = sem_.GetVal(align_attr->expr)->ConstantValue()->ValueAs<uint32_t>();
                if (align % required_align != 0) {
                    AddError(align_attr->expr->source)
                        << "alignment must be a multiple of " << style::Literal(required_align)
                        << " bytes for the " << style::Enum(address_space) << " address space";
                    note_usage();
                    return false;
                }
            }
        }
    }

    // For uniform buffer array members, validate that array elements are aligned to 16 bytes
    if (auto* arr = store_ty->As<sem::Array>()) {
        // Recurse into the element type.
        // TODO(crbug.com/tint/1388): Ideally we'd pass the source for nested element type here, but
        // we can't easily get that from the semantic node. We should consider recursing through the
        // AST type nodes instead.
        if (!AddressSpaceLayout(arr->ElemType(), address_space, source)) {
            return false;
        }

        if (address_space == core::AddressSpace::kUniform) {
            // We already validated that this array member is itself aligned to 16 bytes above, so
            // we only need to validate that stride is a multiple of 16 bytes.
            if (arr->Stride() % 16 != 0) {
                // Since WGSL has no stride attribute, try to provide a useful hint for how the
                // shader author can resolve the issue.
                StyledText hint;
                if (arr->ElemType()->Is<core::type::Scalar>()) {
                    hint << "Consider using a vector or struct as the element type instead.";
                } else if (auto* vec = arr->ElemType()->As<core::type::Vector>();
                           vec && vec->Type()->Size() == 4) {
                    hint << "Consider using a vec4 instead.";
                } else if (arr->ElemType()->Is<sem::Struct>()) {
                    hint << "Consider using the " << style::Attribute("@size")
                         << " attribute on the last struct member.";
                } else {
                    hint << "Consider wrapping the element type in a struct and using the "
                         << style::Attribute("@size") << " attribute.";
                }
                AddError(source) << style::Enum("uniform")
                                 << " storage requires that array elements are aligned to 16 "
                                    "bytes, but array element of type "
                                 << style::Type(arr->ElemType()->FriendlyName())
                                 << " has a stride of " << arr->Stride() << " bytes. " << hint;
                return false;
            }
        }
    }

    return true;
}

bool Validator::LocalVariable(const sem::Variable* local) const {
    auto* decl = local->Declaration();
    if (IsArrayWithOverrideCount(local->Type())) {
        RaiseArrayWithOverrideCountError(decl->type ? decl->type->source
                                                    : decl->initializer->source);
        return false;
    }
    return Switch(
        decl,  //
        [&](const ast::Var* var) {
            if (!local->Type()->UnwrapRef()->IsConstructible()) {
                AddError(var->type ? var->type->source : var->source)
                    << "function-scope " << style::Keyword("var")
                    << " must have a constructible type";
                return false;
            }
            return Var(local);
        },                                            //
        [&](const ast::Let*) { return Let(local); },  //
        [&](const ast::Const*) { return true; },      //
        TINT_ICE_ON_NO_MATCH);
}

bool Validator::GlobalVariable(
    const sem::GlobalVariable* global,
    const Hashmap<OverrideId, const sem::Variable*, 8>& override_ids) const {
    auto* decl = global->Declaration();
    if (global->AddressSpace() != core::AddressSpace::kWorkgroup &&
        IsArrayWithOverrideCount(global->Type())) {
        RaiseArrayWithOverrideCountError(decl->type ? decl->type->source
                                                    : decl->initializer->source);
        return false;
    }
    bool ok = Switch(
        decl,  //
        [&](const ast::Var* var) {
            if (auto* init = global->Initializer();
                init && init->Stage() > core::EvaluationStage::kOverride) {
                AddError(init->Declaration()->source)
                    << "module-scope " << style::Keyword("var")
                    << " initializer must be a constant or override-expression";
                return false;
            }

            if (!var->declared_address_space && !global->Type()->UnwrapRef()->IsHandle()) {
                AddError(decl->source) << "module-scope " << style::Keyword("var")
                                       << " declarations that are not of texture or sampler types "
                                          "must provide an address space";
                return false;
            }

            return Var(global);
        },
        [&](const ast::Override*) { return Override(global, override_ids); },
        [&](const ast::Const*) { return Const(global); },  //
        TINT_ICE_ON_NO_MATCH);

    if (!ok) {
        return false;
    }

    if (global->AddressSpace() == core::AddressSpace::kFunction) {
        AddError(decl->source) << "module-scope " << style::Keyword("var")
                               << " must not use address space " << style::Enum("function");
        return false;
    }

    auto* input_attachment_index_attr =
        ast::GetAttribute<ast::InputAttachmentIndexAttribute>(decl->attributes);
    if (input_attachment_index_attr &&
        !InputAttachmentIndexAttribute(input_attachment_index_attr, global->Type()->UnwrapRef(),
                                       decl->source)) {
        return false;
    }
    switch (global->AddressSpace()) {
        case core::AddressSpace::kUniform:
        case core::AddressSpace::kStorage:
        case core::AddressSpace::kHandle: {
            // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
            // Each resource variable must be declared with both group and binding attributes.
            if (!decl->HasBindingPoint()) {
                AddError(decl->source)
                    << "resource variables require " << style::Attribute("@group") << " and "
                    << style::Attribute("@binding") << " attributes";
                return false;
            }
            if (global->Type()->UnwrapRef()->Is<core::type::InputAttachment>() &&
                !input_attachment_index_attr) {
                AddError(decl->source)
                    << style::Type("input_attachment") << " variables require "
                    << style::Attribute("@input_attachment_index") << " attribute";
                return false;
            }
            break;
        }
        default: {
            auto* binding_attr = ast::GetAttribute<ast::BindingAttribute>(decl->attributes);
            auto* group_attr = ast::GetAttribute<ast::GroupAttribute>(decl->attributes);
            if (binding_attr || group_attr) {
                // https://gpuweb.github.io/gpuweb/wgsl/#attribute-binding
                // Must only be applied to a resource variable
                AddError(decl->source)
                    << "non-resource variables must not have " << style::Attribute("@group")
                    << " or " << style::Attribute("@binding") << " attributes";
                return false;
            }
        }
    }

    return true;
}

bool Validator::Var(const sem::Variable* v) const {
    auto* var = v->Declaration()->As<ast::Var>();
    auto* store_ty = v->Type()->UnwrapRef();

    if (!IsStorable(store_ty)) {
        AddError(var->source) << sem_.TypeNameOf(store_ty)
                              << " cannot be used as the type of a var";
        return false;
    }

    if (store_ty->IsHandle() && var->declared_address_space) {
        // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
        // If the store type is a texture type or a sampler type, then the variable declaration must
        // not have a address space attribute. The address space will always be handle.
        AddError(var->source) << "variables of type " << style::Type(sem_.TypeNameOf(store_ty))
                              << " must not specify an address space";
        return false;
    }

    if (var->declared_access) {
        // https://www.w3.org/TR/WGSL/#access-mode-defaults
        // When writing a variable declaration or a pointer type in WGSL source:
        // * For the storage address space, the access mode is optional, and defaults to read.
        // * For other address spaces, the access mode must not be written.
        if (v->AddressSpace() != core::AddressSpace::kStorage) {
            AddError(var->source)
                << "only variables in <storage> address space may specify an access mode";
            return false;
        }
    }

    if (var->initializer) {
        switch (v->AddressSpace()) {
            case core::AddressSpace::kPrivate:
            case core::AddressSpace::kFunction:
                break;  // Allowed an initializer
            default:
                // https://gpuweb.github.io/gpuweb/wgsl/#var-and-let
                // Optionally has an initializer expression, if the variable is in the private or
                // function address spaces.
                AddError(var->source)
                    << "var of address space " << style::Enum(v->AddressSpace())
                    << " cannot have an initializer. var initializers are only supported for the "
                       "address spaces "
                    << style::Enum("private") << " and " << style::Enum("function");
                return false;
        }
    }

    if (!CheckTypeAccessAddressSpace(v->Type()->UnwrapRef(), v->Access(), v->AddressSpace(),
                                     var->source)) {
        return false;
    }

    if ((v->AddressSpace() == core::AddressSpace::kIn ||
         v->AddressSpace() == core::AddressSpace::kOut)) {
        AddError(var->source) << "invalid use of input/output address space";
        return false;
    }
    return true;
}

bool Validator::Let(const sem::Variable* v) const {
    auto* decl = v->Declaration();
    auto* storage_ty = v->Type()->UnwrapRef();

    if (!(storage_ty->IsConstructible() || storage_ty->Is<core::type::Pointer>())) {
        AddError(decl->source) << sem_.TypeNameOf(storage_ty) << " cannot be used as the type of a "
                               << style::Keyword("let");
        return false;
    }
    return true;
}

bool Validator::Override(const sem::GlobalVariable* v,
                         const Hashmap<OverrideId, const sem::Variable*, 8>& override_ids) const {
    auto* decl = v->Declaration();
    auto* storage_ty = v->Type()->UnwrapRef();

    if (auto* init = v->Initializer(); init && init->Stage() > core::EvaluationStage::kOverride) {
        AddError(init->Declaration()->source)
            << style::Keyword("override") << " initializer must be an override-expression";
        return false;
    }

    if (auto id = v->Attributes().override_id) {
        if (auto var = override_ids.Get(*id); var && *var != v) {
            auto* attr = ast::GetAttribute<ast::IdAttribute>(v->Declaration()->attributes);
            AddError(attr->source) << style::Attribute("@id") << " values must be unique";
            AddNote(ast::GetAttribute<ast::IdAttribute>((*var)->Declaration()->attributes)->source)
                << "a override with an ID of " << id->value << " was previously declared here";
            return false;
        }
    }

    if (!storage_ty->Is<core::type::Scalar>()) {
        AddError(decl->source) << sem_.TypeNameOf(storage_ty) << " cannot be used as the type of a "
                               << style::Keyword("override");
        return false;
    }

    return true;
}

bool Validator::Const(const sem::Variable*) const {
    return true;
}

bool Validator::Parameter(const sem::Variable* var) const {
    auto* decl = var->Declaration();

    if (auto* ref = var->Type()->As<core::type::Pointer>()) {
        bool ok = false;

        auto sc = ref->AddressSpace();
        switch (sc) {
            case core::AddressSpace::kFunction:
            case core::AddressSpace::kPrivate:
                ok = true;
                break;
            case core::AddressSpace::kStorage:
            case core::AddressSpace::kUniform:
            case core::AddressSpace::kWorkgroup:
                ok = allowed_features_.features.count(
                         wgsl::LanguageFeature::kUnrestrictedPointerParameters) != 0;
                break;
            default:
                break;
        }
        if (!ok) {
            AddError(decl->source) << "function parameter of pointer type cannot be in "
                                   << style::Enum(sc) << " address space";
            return false;
        }
    }

    if (IsPlain(var->Type())) {
        if (!var->Type()->IsConstructible()) {
            AddError(decl->type->source) << "type of function parameter must be constructible";
            return false;
        }
    } else if (!var->Type()->Is<core::type::Pointer>() && !var->Type()->IsHandle()) {
        AddError(decl->source) << "type of function parameter cannot be "
                               << sem_.TypeNameOf(var->Type());
        return false;
    }

    return true;
}

bool Validator::BuiltinAttribute(const ast::BuiltinAttribute* attr,
                                 const core::type::Type* storage_ty,
                                 ast::PipelineStage stage,
                                 const bool is_input) const {
    auto* type = storage_ty->UnwrapRef();
    bool is_stage_mismatch = false;
    bool is_output = !is_input;
    auto builtin = attr->builtin;

    auto err_builtin_type = [&](std::string_view required) {
        AddError(attr->source) << "store type of " << style::Attribute("@builtin")
                               << style::Code("(", style::Enum(builtin), ")") << " must be "
                               << style::Type(required);
    };

    switch (builtin) {
        case core::BuiltinValue::kPosition: {
            if (stage != ast::PipelineStage::kNone &&
                !((is_input && stage == ast::PipelineStage::kFragment) ||
                  (is_output && stage == ast::PipelineStage::kVertex))) {
                is_stage_mismatch = true;
            }
            auto* vec = type->As<core::type::Vector>();
            if (!(vec && vec->Width() == 4 && vec->Type()->Is<core::type::F32>())) {
                err_builtin_type("vec4<f32>");
                return false;
            }
            break;
        }
        case core::BuiltinValue::kGlobalInvocationId:
        case core::BuiltinValue::kLocalInvocationId:
        case core::BuiltinValue::kNumWorkgroups:
        case core::BuiltinValue::kWorkgroupId:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kCompute && is_input)) {
                is_stage_mismatch = true;
            }
            if (!(type->IsUnsignedIntegerVector() &&
                  type->As<core::type::Vector>()->Width() == 3)) {
                err_builtin_type("vec3<u32>");
                return false;
            }
            break;
        case core::BuiltinValue::kFragDepth:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && !is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::F32>()) {
                err_builtin_type("f32");
                return false;
            }
            break;
        case core::BuiltinValue::kFrontFacing:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::Bool>()) {
                err_builtin_type("bool");
                return false;
            }
            break;
        case core::BuiltinValue::kLocalInvocationIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kCompute && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            break;
        case core::BuiltinValue::kVertexIndex:
        case core::BuiltinValue::kInstanceIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kVertex && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            break;
        case core::BuiltinValue::kSampleMask:
            if (stage != ast::PipelineStage::kNone && !(stage == ast::PipelineStage::kFragment)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            break;
        case core::BuiltinValue::kSampleIndex:
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            break;
        case core::BuiltinValue::kSubgroupId:
            // TODO(crbug.com/416555787): Move this to the `subgroups` extension.
            if (!enabled_extensions_.Contains(
                    wgsl::Extension::kChromiumExperimentalSubgroupMatrix)) {
                AddError(attr->source) << "use of " << style::Attribute("@builtin")
                                       << style::Code("(", style::Enum(builtin), ")")
                                       << " attribute requires enabling extension "
                                       << style::Code("chromium_experimental_subgroup_matrix");
                return false;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kCompute && is_input)) {
                AddError(attr->source)
                    << style::Attribute("@builtin") << style::Code("(", style::Enum(builtin), ")")
                    << " is only valid as a compute shader input";
                return false;
            }
            break;
        case core::BuiltinValue::kSubgroupInvocationId:
        case core::BuiltinValue::kSubgroupSize:
            if (!enabled_extensions_.Contains(wgsl::Extension::kSubgroups)) {
                AddError(attr->source)
                    << "use of " << style::Attribute("@builtin")
                    << style::Code("(", style::Enum(builtin), ")")
                    << " attribute requires enabling extension " << style::Code("subgroups");
                return false;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            if (stage != ast::PipelineStage::kNone && !((stage == ast::PipelineStage::kCompute ||
                                                         stage == ast::PipelineStage::kFragment) &&
                                                        is_input)) {
                AddError(attr->source)
                    << style::Attribute("@builtin") << style::Code("(", style::Enum(builtin), ")")
                    << " is only valid as a compute or fragment shader input";
                return false;
            }
            break;
        case core::BuiltinValue::kClipDistances: {
            if (!enabled_extensions_.Contains(wgsl::Extension::kClipDistances)) {
                AddError(attr->source)
                    << "use of " << style::Attribute("@builtin")
                    << style::Code("(", style::Enum(builtin), ")")
                    << " requires enabling extension " << style::Code("clip_distances");
                return false;
            }
            auto* arr = type->As<sem::Array>();
            if (!((arr != nullptr) && arr->ElemType()->Is<core::type::F32>() &&
                  arr->ConstantCount().has_value() &&
                  *arr->ConstantCount() <= kMaxClipDistancesSize)) {
                AddError(attr->source)
                    << "store type of " << style::Attribute("@builtin")
                    << style::Code("(", style::Enum(builtin), ")") << " must be "
                    << style::Type("array<f32, N>") << " (N <= " << kMaxClipDistancesSize << ")";
                return false;
            }
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kVertex && !is_input)) {
                is_stage_mismatch = true;
            }
            break;
        }
        case core::BuiltinValue::kPrimitiveId: {
            if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalPrimitiveId)) {
                AddError(attr->source) << "use of " << style::Attribute("@builtin")
                                       << style::Code("(", style::Enum(builtin), ")")
                                       << " requires enabling extension "
                                       << style::Code("chromium_experimental_primitive_id");
                return false;
            }
            if (!type->Is<core::type::U32>()) {
                err_builtin_type("u32");
                return false;
            }
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            break;
        }
        case core::BuiltinValue::kBarycentricCoord: {
            if (!enabled_extensions_.Contains(
                    wgsl::Extension::kChromiumExperimentalBarycentricCoord)) {
                AddError(attr->source) << "use of " << style::Attribute("@builtin")
                                       << style::Code("(", style::Enum(builtin), ")")
                                       << " requires enabling extension "
                                       << style::Code("chromium_experimental_barycentric_coord");
                return false;
            }
            if (!(type->IsFloatVector() && type->As<core::type::Vector>()->Width() == 3)) {
                err_builtin_type("vec3<f32>");
                return false;
            }
            if (stage != ast::PipelineStage::kNone &&
                !(stage == ast::PipelineStage::kFragment && is_input)) {
                is_stage_mismatch = true;
            }
            break;
        }
        default:
            break;
    }

    if (is_stage_mismatch) {
        AddError(attr->source) << style::Attribute("@builtin")
                               << style::Code("(", style::Enum(builtin), ")")
                               << " cannot be used for " << stage << " shader "
                               << (is_input ? "input" : "output");
        return false;
    }

    return true;
}

bool Validator::InterpolateAttribute(const ast::InterpolateAttribute* attr,
                                     const core::type::Type* storage_ty,
                                     const ast::PipelineStage stage) const {
    if (stage == ast::PipelineStage::kCompute) {
        AddError(attr->source) << style::Attribute("@", attr->Name())
                               << " cannot be used by compute shaders";
        return false;
    }

    auto* type = storage_ty->UnwrapRef();

    auto i_type = attr->interpolation.type;
    auto i_sampling = attr->interpolation.sampling;
    if (DAWN_UNLIKELY(i_type == core::InterpolationType::kUndefined)) {
        return false;
    }

    if (type->IsIntegerScalarOrVector() && i_type != core::InterpolationType::kFlat) {
        AddError(attr->source) << "interpolation type must be " << style::Enum("flat")
                               << " for integral user-defined IO types";
        return false;
    }

    if (i_sampling != core::InterpolationSampling::kUndefined) {
        bool is_first_or_either = i_sampling == core::InterpolationSampling::kFirst ||
                                  i_sampling == core::InterpolationSampling::kEither;

        if (i_type == core::InterpolationType::kFlat) {
            if (!is_first_or_either) {
                AddError(attr->source)
                    << "flat interpolation can only use 'first' and 'either' sampling parameters";
                return false;
            }
        } else {
            if (is_first_or_either) {
                AddError(attr->source) << "'first' and 'either' sampling parameters can only be "
                                          "used with flat interpolation";
                return false;
            }
        }
    }

    return true;
}

bool Validator::InvariantAttribute(const ast::InvariantAttribute* attr,
                                   const ast::PipelineStage stage) const {
    if (stage == ast::PipelineStage::kCompute) {
        AddError(attr->source) << style::Attribute("@", attr->Name())
                               << " cannot be used by compute shaders";
        return false;
    }
    return true;
}

bool Validator::Function(const sem::Function* func, ast::PipelineStage stage) const {
    auto* decl = func->Declaration();

    for (auto* attr : decl->attributes) {
        bool ok = Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute*) {
                if (decl->PipelineStage() != ast::PipelineStage::kCompute) {
                    AddError(attr->source) << style::Attribute("@workgroup_size")
                                           << " is only valid for compute stages";
                    return false;
                }
                return true;
            },
            [&](const ast::MustUseAttribute*) {
                if (func->ReturnType()->Is<core::type::Void>()) {
                    AddError(attr->source)
                        << style::Attribute("@must_use")
                        << " can only be applied to functions that return a value";
                    return false;
                }
                return true;
            },
            [&](Default) { return true; });
        if (!ok) {
            return false;
        }
    }

    if (decl->params.Length() > kMaxFunctionParameters) {
        AddError(decl->name->source) << "function declares " << decl->params.Length()
                                     << " parameters, maximum is " << kMaxFunctionParameters;
        return false;
    }

    if (!func->ReturnType()->Is<core::type::Void>()) {
        if (!func->ReturnType()->IsConstructible()) {
            AddError(decl->return_type->source)
                << "function return type must be a constructible type";
            return false;
        }

        if (decl->body) {
            auto behaviors = sem_.Get(decl->body)->Behaviors();
            if (behaviors.Contains(sem::Behavior::kNext)) {
                auto end_source = decl->body->source.End();
                end_source.range.begin.column--;
                AddError(end_source) << "missing return at end of function";
                return false;
            }
        } else if (DAWN_UNLIKELY(IsValidationEnabled(
                       decl->attributes, ast::DisabledValidation::kFunctionHasNoBody))) {
            TINT_ICE() << "function " << decl->name->symbol.NameView() << " has no body";
        }
    }

    if (decl->IsEntryPoint()) {
        if (!EntryPoint(func, stage)) {
            return false;
        }
    }

    // https://www.w3.org/TR/WGSL/#behaviors-rules
    // a function behavior is always one of {}, or {Next}.
    if (DAWN_UNLIKELY(func->Behaviors() != sem::Behaviors{} &&
                      func->Behaviors() != sem::Behavior::kNext)) {
        auto name = decl->name->symbol.NameView();
        TINT_ICE() << "function '" << name << "' behaviors are: " << func->Behaviors();
    }

    return true;
}

bool Validator::EntryPoint(const sem::Function* func, ast::PipelineStage stage) const {
    auto* decl = func->Declaration();

    // Use a lambda to validate the entry point attributes for a type.
    // Persistent state is used to track which builtins and locations have already been seen, in
    // order to catch conflicts.
    // TODO(jrprice): This state could be stored in sem::Function instead, and then passed to
    // sem::Function since it would be useful there too.
    Hashset<core::BuiltinValue, 4> builtins;
    Hashset<std::pair<uint32_t, uint32_t>, 8> locations_and_blend_srcs;
    Hashset<uint32_t, 4> colors;
    enum class ParamOrRetType {
        kParameter,
        kReturnType,
    };

    // Inner lambda that is applied to a type and all of its members.
    auto validate_entry_point_attributes_inner = [&](VectorRef<const ast::Attribute*> attrs,
                                                     const core::type::Type* ty, Source source,
                                                     ParamOrRetType param_or_ret,
                                                     bool is_struct_member,
                                                     std::optional<uint32_t> location,
                                                     std::optional<uint32_t> blend_src,
                                                     std::optional<uint32_t> color) {
        // Scan attributes for pipeline IO attributes.
        // Check for overlap with attributes that have been seen previously.
        const ast::Attribute* pipeline_io_attribute = nullptr;
        const ast::LocationAttribute* location_attribute = nullptr;
        const ast::ColorAttribute* color_attribute = nullptr;
        const ast::BlendSrcAttribute* blend_src_attribute = nullptr;
        const ast::InterpolateAttribute* interpolate_attribute = nullptr;
        const ast::InvariantAttribute* invariant_attribute = nullptr;
        for (auto* attr : attrs) {
            bool ok = Switch(
                attr,  //
                [&](const ast::BuiltinAttribute* builtin_attr) {
                    auto builtin = builtin_attr->builtin;

                    if (pipeline_io_attribute) {
                        AddError(attr->source) << "multiple entry point IO attributes";
                        AddNote(pipeline_io_attribute->source)
                            << "previously consumed "
                            << style::Attribute("@", pipeline_io_attribute->Name());
                        return false;
                    }
                    pipeline_io_attribute = attr;

                    if (builtins.Contains(builtin)) {
                        AddError(decl->source)
                            << style::Attribute("@builtin")
                            << style::Code("(", style::Enum(builtin), ")")
                            << " appears multiple times as pipeline "
                            << (param_or_ret == ParamOrRetType::kParameter ? "input" : "output");
                        return false;
                    }

                    if (!BuiltinAttribute(
                            builtin_attr, ty, stage,
                            /* is_input */ param_or_ret == ParamOrRetType::kParameter)) {
                        return false;
                    }
                    builtins.Add(builtin);
                    return true;
                },
                [&](const ast::LocationAttribute* loc_attr) {
                    location_attribute = loc_attr;
                    if (pipeline_io_attribute) {
                        AddError(attr->source) << "multiple entry point IO attributes";
                        AddNote(pipeline_io_attribute->source)
                            << "previously consumed "
                            << style::Attribute("@", pipeline_io_attribute->Name());
                        return false;
                    }
                    pipeline_io_attribute = attr;

                    if (DAWN_UNLIKELY(!location.has_value())) {
                        TINT_ICE() << "@location has no value";
                    }

                    return LocationAttribute(loc_attr, ty, stage, source);
                },
                [&](const ast::BlendSrcAttribute* blend_src_attr) {
                    blend_src_attribute = blend_src_attr;

                    if (DAWN_UNLIKELY(!blend_src.has_value())) {
                        TINT_ICE() << "@blend_src has no value";
                    }

                    bool is_input = param_or_ret == ParamOrRetType::kParameter;
                    return BlendSrcAttribute(blend_src_attr, stage, is_input);
                },
                [&](const ast::ColorAttribute* col_attr) {
                    color_attribute = col_attr;
                    if (pipeline_io_attribute) {
                        AddError(attr->source) << "multiple entry point IO attributes";
                        AddNote(pipeline_io_attribute->source)
                            << "previously consumed "
                            << style::Attribute("@", pipeline_io_attribute->Name());
                        return false;
                    }
                    pipeline_io_attribute = attr;

                    bool is_input = param_or_ret == ParamOrRetType::kParameter;

                    if (DAWN_UNLIKELY(!color.has_value())) {
                        TINT_ICE() << "@color has no value";
                    }

                    return ColorAttribute(col_attr, ty, stage, source, is_input);
                },
                [&](const ast::InterpolateAttribute* interpolate) {
                    interpolate_attribute = interpolate;
                    return InterpolateAttribute(interpolate, ty, stage);
                },
                [&](const ast::InvariantAttribute* invariant) {
                    invariant_attribute = invariant;
                    return InvariantAttribute(invariant, stage);
                },
                [&](const ast::InternalAttribute* internal) {
                    pipeline_io_attribute = internal;
                    return true;
                },
                [&](Default) { return true; });

            if (!ok) {
                return false;
            }
        }

        if (is_struct_member && ty->Is<core::type::Struct>()) {
            AddError(source) << "nested structures cannot be used for entry point IO";
            return false;
        }

        if (!ty->Is<core::type::Struct>() && !pipeline_io_attribute) {
            auto& err = AddError(source) << "missing entry point IO attribute";
            if (!is_struct_member) {
                err << (param_or_ret == ParamOrRetType::kParameter ? " on parameter"
                                                                   : " on return type");
            }
            return false;
        }

        if (pipeline_io_attribute && pipeline_io_attribute->Is<ast::LocationAttribute>()) {
            if (ty->IsIntegerScalarOrVector() && !interpolate_attribute) {
                if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
                    param_or_ret == ParamOrRetType::kReturnType) {
                    AddError(source) << "integral user-defined vertex outputs must have a "
                                     << style::Attribute("@interpolate")
                                     << style::Code("(", style::Enum("flat"), ")") << " attribute";
                    return false;
                }
                if (decl->PipelineStage() == ast::PipelineStage::kFragment &&
                    param_or_ret == ParamOrRetType::kParameter) {
                    AddError(source) << "integral user-defined fragment inputs must have a "
                                     << style::Attribute("@interpolate")
                                     << style::Code("(", style::Enum("flat"), ")") << " attribute";
                    return false;
                }
            }
        }

        if (location_attribute) {
            std::pair<uint32_t, uint32_t> location_and_blend_src(location.value(),
                                                                 blend_src.value_or(0));
            if (!locations_and_blend_srcs.Add(location_and_blend_src)) {
                auto& err = AddError(location_attribute->source)
                            << style::Attribute("@location")
                            << style::Code("(", style::Literal(location.value()), ")");
                if (blend_src_attribute) {
                    err << style::Attribute(" @blend_src")
                        << style::Code("(", style::Literal(blend_src.value()), ")");
                }
                err << " appears multiple times";
                return false;
            }
        }

        if (color_attribute && !colors.Add(color.value())) {
            AddError(color_attribute->source)
                << style::Attribute("@color")
                << style::Code("(", style::Literal(color.value()), ")")
                << " appears multiple times";
            return false;
        }

        if (interpolate_attribute) {
            if (!pipeline_io_attribute || !pipeline_io_attribute->Is<ast::LocationAttribute>()) {
                AddError(interpolate_attribute->source)
                    << style::Attribute("@interpolate") << " can only be used with "
                    << style::Attribute("@location");
                return false;
            }
        }

        if (invariant_attribute) {
            bool has_position = false;
            if (pipeline_io_attribute) {
                if (auto* builtin_attr = pipeline_io_attribute->As<ast::BuiltinAttribute>()) {
                    has_position = (builtin_attr->builtin == core::BuiltinValue::kPosition);
                }
            }
            if (!has_position) {
                AddError(invariant_attribute->source)
                    << style::Attribute("@invariant") << " must be applied to a "
                    << style::Attribute("@builtin")
                    << style::Code("(", style::Enum("position"), ")");
                return false;
            }
        }

        return true;
    };

    // Outer lambda for validating the entry point attributes for a type.
    auto validate_entry_point_attributes =
        [&](VectorRef<const ast::Attribute*> attrs, const core::type::Type* ty, Source source,
            ParamOrRetType param_or_ret, std::optional<uint32_t> location,
            std::optional<uint32_t> blend_src, std::optional<uint32_t> color) {
            if (!validate_entry_point_attributes_inner(attrs, ty, source, param_or_ret,
                                                       /*is_struct_member*/ false, location,
                                                       blend_src, color)) {
                return false;
            }

            if (auto* str = ty->As<sem::Struct>()) {
                for (auto* member : str->Members()) {
                    if (!validate_entry_point_attributes_inner(
                            member->Declaration()->attributes, member->Type(),
                            member->Declaration()->source, param_or_ret,
                            /*is_struct_member*/ true, member->Attributes().location,
                            member->Attributes().blend_src, member->Attributes().color)) {
                        AddNote(decl->source) << "while analyzing entry point "
                                              << style::Function(decl->name->symbol.NameView());
                        return false;
                    }
                }
            }

            return true;
        };

    for (auto* param : func->Parameters()) {
        auto* param_decl = param->Declaration();
        auto& attrs = param->Attributes();
        if (!validate_entry_point_attributes(param_decl->attributes, param->Type(),
                                             param_decl->source, ParamOrRetType::kParameter,
                                             attrs.location, attrs.blend_src, attrs.color)) {
            return false;
        }
    }

    // Clear IO sets after parameter validation. Builtin and location attributes in return types
    // should be validated independently from those used in parameters.
    builtins.Clear();
    locations_and_blend_srcs.Clear();

    if (!func->ReturnType()->Is<core::type::Void>()) {
        if (!validate_entry_point_attributes(decl->return_type_attributes, func->ReturnType(),
                                             decl->source, ParamOrRetType::kReturnType,
                                             func->ReturnLocation(), /* blend_src */ std::nullopt,
                                             /* color */ std::nullopt)) {
            return false;
        }
    }

    if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
        !builtins.Contains(core::BuiltinValue::kPosition)) {
        AddError(decl->source) << "a vertex shader must include the " << style::Enum("position")
                               << " builtin in its return type";
        return false;
    }

    if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
        if (!ast::HasAttribute<ast::WorkgroupAttribute>(decl->attributes)) {
            AddError(decl->source) << "a compute shader must include "
                                   << style::Attribute("@workgroup_size") << " in its attributes";
            return false;
        }
    }

    // Validate there are no resource variable binding collisions
    Hashmap<BindingPoint, const ast::Variable*, 8> binding_points;
    for (auto* global : func->TransitivelyReferencedGlobals()) {
        auto* var_decl = global->Declaration()->As<ast::Var>();
        if (!var_decl) {
            continue;
        }
        auto bp = global->Attributes().binding_point;
        if (!bp) {
            continue;
        }
        auto added = binding_points.Add(*bp, var_decl);
        if (!added) {
            // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
            // Bindings must not alias within a shader stage: two different variables in the
            // resource interface of a given shader must not have the same group and binding values,
            // when considered as a pair of values.
            auto func_name = decl->name->symbol.NameView();
            AddError(var_decl->source)
                << "entry point " << style::Function(func_name)
                << " references multiple variables that use the same resource binding "
                << style::Attribute("@group") << style::Code("(", style::Literal(bp->group), ")")
                << ", " << style::Attribute("@binding")
                << style::Code("(", style::Literal(bp->binding), ")");
            AddNote(added.value->source) << "first resource binding usage declared here";
            return false;
        }
    }

    return true;
}

bool Validator::EvaluationStage(const sem::ValueExpression* expr,
                                core::EvaluationStage latest_stage,
                                std::string_view constraint) const {
    if (expr->Stage() == core::EvaluationStage::kNotEvaluated) {
        return true;
    }
    if (expr->Stage() > latest_stage) {
        auto stage_name = [](core::EvaluationStage stage) -> std::string {
            switch (stage) {
                case core::EvaluationStage::kRuntime:
                    return "a runtime-expression";
                case core::EvaluationStage::kOverride:
                    return "an override-expression";
                case core::EvaluationStage::kConstant:
                    return "a const-expression";
                case core::EvaluationStage::kNotEvaluated:
                    return "an unevaluated expression";
            }
            return "<unknown>";
        };

        AddError(expr->Declaration()->source)
            << constraint << " requires " << stage_name(latest_stage) << ", but expression is "
            << stage_name(expr->Stage());

        if (auto* stmt = expr->Stmt()) {
            if (auto* decl = As<ast::VariableDeclStatement>(stmt->Declaration())) {
                if (decl->variable->Is<ast::Const>()) {
                    AddNote(decl->source) << "consider changing " << style::Keyword("const")
                                          << " to " << style::Keyword("let");
                }
            }
        }
        return false;
    }
    return true;
}

bool Validator::Statements(VectorRef<const ast::Statement*> stmts) const {
    for (auto* stmt : stmts) {
        if (!sem_.Get(stmt)->IsReachable()) {
            if (auto* d = MaybeAddDiagnostic(wgsl::ChromiumDiagnosticRule::kUnreachableCode,
                                             stmt->source)) {
                *d << "code is unreachable";
                if (d->severity >= diag::Severity::Error) {
                    return false;
                }
            }
            break;
        }
    }
    return true;
}

bool Validator::BinaryExpression(const ast::Node* node,
                                 const core::BinaryOp op,
                                 const tint::sem::ValueExpression* lhs,
                                 const tint::sem::ValueExpression* rhs) const {
    switch (op) {
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight: {
            // If lhs value is a concrete type, and rhs is a const-expression greater than or equal
            // to the bit width of lhs, then it is a shader-creation error.
            const auto* elem_type = lhs->Type()->UnwrapRef()->DeepestElement();
            if (!elem_type->IsAbstract() && rhs->Stage() == core::EvaluationStage::kConstant) {
                const uint32_t bit_width = elem_type->Size() * 8;
                auto* rhs_val = rhs->ConstantValue();
                for (size_t i = 0, n = rhs_val->NumElements(); i < n; i++) {
                    auto* shift_val = n == 1 ? rhs_val : rhs_val->Index(i);
                    if (shift_val->ValueAs<u32>() >= bit_width) {
                        AddError(node->source)
                            << "shift " << (op == core::BinaryOp::kShiftLeft ? "left" : "right")
                            << " value must be less than the bit width of the lhs, which is "
                            << bit_width;
                        return false;
                    }
                }
            }
            return true;
        }
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo: {
            // Integer division by zero should be checked for the partial evaluation case (only rhs
            // is const). FP division by zero is only invalid when the whole expression is
            // constant-evaluated.
            if (rhs->Type()->IsIntegerScalarOrVector() &&
                rhs->Stage() == core::EvaluationStage::kConstant) {
                if (rhs->ConstantValue()->AnyZero()) {
                    AddError(node->source) << "integer division by zero is invalid";
                    return false;
                }
            }
            return true;
        }
        default: {
            return true;
        }
    }
}

bool Validator::BreakStatement(const sem::Statement* stmt,
                               sem::Statement* current_statement) const {
    if (!stmt->FindFirstParent<sem::LoopBlockStatement, sem::CaseStatement>()) {
        AddError(stmt->Declaration()->source) << "break statement must be in a loop or switch case";
        return false;
    }
    if (ClosestContinuing(/*stop_at_loop*/ true, /* stop_at_switch */ true, current_statement) !=
        nullptr) {
        AddError(stmt->Declaration()->source)
            << style::Keyword("break") << " must not be used to exit from a continuing block. Use "
            << style::Keyword("break if") << " instead.";
        return false;
    }
    return true;
}

bool Validator::ContinueStatement(const sem::Statement* stmt,
                                  sem::Statement* current_statement) const {
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ true, /* stop_at_switch */ false,
                                             current_statement)) {
        AddError(stmt->Declaration()->source)
            << "continuing blocks must not contain a continue statement";
        if (continuing != stmt->Declaration() && continuing != stmt->Parent()->Declaration()) {
            AddNote(continuing->source) << "see continuing block here";
        }
        return false;
    }

    if (!stmt->FindFirstParent<sem::LoopBlockStatement>()) {
        AddError(stmt->Declaration()->source) << "continue statement must be in a loop";
        return false;
    }

    return true;
}

bool Validator::Call(const sem::Call* call, sem::Statement* current_statement) const {
    if (!call->Target()->MustUse()) {
        return true;
    }

    auto* expr = call->Declaration();
    bool is_call_stmt = (current_statement != nullptr) &&
                        Is<ast::CallStatement>(current_statement->Declaration(),
                                               [&](auto* stmt) { return stmt->expr == expr; });
    if (is_call_stmt) {
        // Call target is annotated with @must_use, but was used as a call statement.
        Switch(
            call->Target(),  //
            [&](const sem::Function* fn) {
                AddError(call->Declaration()->source)
                    << "ignoring return value of function "
                    << style::Function(fn->Declaration()->name->symbol.NameView())
                    << " annotated with " << style::Attribute("@must_use");
                sem_.NoteDeclarationSource(fn->Declaration());
            },
            [&](const sem::BuiltinFn* b) {
                AddError(call->Declaration()->source)
                    << "ignoring return value of builtin " << style::Function(b->Fn());
            },
            [&](const sem::ValueConversion*) {
                AddError(call->Declaration()->source) << "value conversion evaluated but not used";
            },
            [&](const sem::ValueConstructor*) {
                AddError(call->Declaration()->source) << "value constructor evaluated but not used";
            },
            [&](Default) {
                AddError(call->Declaration()->source) << "return value of call not used";
            });
        return false;
    }

    return true;
}

bool Validator::LoopStatement(const sem::LoopStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError(stmt->Declaration()->source.Begin()) << "loop does not exit";
        return false;
    }
    return true;
}

bool Validator::ForLoopStatement(const sem::ForLoopStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError(stmt->Declaration()->source.Begin()) << "for-loop does not exit";
        return false;
    }
    if (auto* cond = stmt->Condition()) {
        auto* cond_ty = cond->Type()->UnwrapRef();
        if (!cond_ty->Is<core::type::Bool>()) {
            AddError(stmt->Condition()->Declaration()->source)
                << "for-loop condition must be bool, got " << sem_.TypeNameOf(cond_ty);
            return false;
        }
    }
    return true;
}

bool Validator::WhileStatement(const sem::WhileStatement* stmt) const {
    if (stmt->Behaviors().Empty()) {
        AddError(stmt->Declaration()->source.Begin()) << "while does not exit";
        return false;
    }
    if (auto* cond = stmt->Condition()) {
        auto* cond_ty = cond->Type()->UnwrapRef();
        if (!cond_ty->Is<core::type::Bool>()) {
            AddError(stmt->Condition()->Declaration()->source)
                << "while condition must be bool, got " << sem_.TypeNameOf(cond_ty);
            return false;
        }
    }
    return true;
}

bool Validator::BreakIfStatement(const sem::BreakIfStatement* stmt,
                                 sem::Statement* current_statement) const {
    auto* cond_ty = stmt->Condition()->Type()->UnwrapRef();
    if (!cond_ty->Is<core::type::Bool>()) {
        AddError(stmt->Condition()->Declaration()->source)
            << "break-if statement condition must be bool, got " << sem_.TypeNameOf(cond_ty);
        return false;
    }

    for (const auto* s = current_statement; s != nullptr; s = s->Parent()) {
        if (s->Is<sem::LoopStatement>()) {
            break;
        }
        if (auto* continuing = s->As<sem::LoopContinuingBlockStatement>()) {
            if (continuing->Declaration()->statements.Back() != stmt->Declaration()) {
                AddError(stmt->Declaration()->source)
                    << "break-if must be the last statement in a continuing block";
                AddNote(s->Declaration()->source) << "see continuing block here";
                return false;
            }
            return true;
        }
    }

    AddError(stmt->Declaration()->source) << "break-if must be in a continuing block";
    return false;
}

bool Validator::IfStatement(const sem::IfStatement* stmt) const {
    auto* cond_ty = stmt->Condition()->Type()->UnwrapRef();
    if (!cond_ty->Is<core::type::Bool>()) {
        AddError(stmt->Condition()->Declaration()->source)
            << "if statement condition must be bool, got " << sem_.TypeNameOf(cond_ty);
        return false;
    }
    return true;
}

bool Validator::BuiltinCall(const sem::Call* call) const {
    if (call->Type()->Is<core::type::Void>()) {
        bool is_call_statement = false;
        // Some built-in call are not owned by a statement, e.g. a built-in called in global
        // variable declaration. Calling no-return-value built-in in these context is invalid as
        // well.
        if (auto* call_stmt = call->Stmt()) {
            if (auto* call_stmt_ast = As<ast::CallStatement>(call_stmt->Declaration())) {
                if (call_stmt_ast->expr == call->Declaration()) {
                    is_call_statement = true;
                }
            }
        }
        if (!is_call_statement) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
            // If the called function does not return a value, a function call statement should be
            // used instead.
            auto* builtin = call->Target()->As<sem::BuiltinFn>();
            AddError(call->Declaration()->source)
                << "builtin function " << style::Function(builtin->Fn())
                << " does not return a value";
            return false;
        }
    }

    // The `print()` builtin requires the chromium_print language feature to be available.
    if (auto* fn = call->Target()->As<sem::BuiltinFn>()) {
        if (fn->Fn() == wgsl::BuiltinFn::kPrint) {
            if (!allowed_features_.features.count(wgsl::LanguageFeature::kChromiumPrint)) {
                AddError(call->Declaration()->source) << "the 'chromium_print' language feature is "
                                                         "not allowed in the current environment";
                return false;
            }
        }
    }

    return true;
}

bool Validator::SubgroupShuffleFunction(wgsl::BuiltinFn fn, const sem::Call* call) const {
    auto* builtin = call->Target()->As<sem::BuiltinFn>();
    if (!builtin) {
        return false;
    }

    TINT_ASSERT(call->Arguments().Length() == 2);
    auto* id = call->Arguments()[1];
    auto* constant_value = id->ConstantValue();

    if (!constant_value) {
        // Non const values are allowed as parameters.
        return true;
    }

    // User friendly param name.
    std::string paramName = "sourceLaneIndex";
    switch (fn) {
        case wgsl::BuiltinFn::kSubgroupShuffleXor:
            paramName = "mask";
            break;
        case wgsl::BuiltinFn::kSubgroupShuffleUp:
        case wgsl::BuiltinFn::kSubgroupShuffleDown:
            paramName = "delta";
            break;
        default:
            break;
    }

    if (id->Type()->IsSignedIntegerScalar() && constant_value->ValueAs<i32>() < 0) {
        AddError(id->Declaration()->source)
            << "the " << paramName << " argument of " << builtin->str()
            << " must be greater than or equal to zero";
        return false;
    }

    if (id->Type()->IsSignedIntegerScalar() &&
        constant_value->ValueAs<i32>() >= tint::internal_limits::kMaxSubgroupSize) {
        AddError(id->Declaration()->source)
            << "the " << paramName << " argument of " << builtin->str() << " must be less than "
            << tint::internal_limits::kMaxSubgroupSize;
        return false;
    }

    if (id->Type()->IsUnsignedIntegerScalar() &&
        constant_value->ValueAs<u32>() >= tint::internal_limits::kMaxSubgroupSize) {
        AddError(id->Declaration()->source)
            << "the " << paramName << " argument of " << builtin->str() << " must be less than "
            << tint::internal_limits::kMaxSubgroupSize;
        return false;
    }

    return true;
}

bool Validator::TextureBuiltinFn(const sem::Call* call) const {
    auto* builtin = call->Target()->As<sem::BuiltinFn>();
    if (!builtin) {
        return false;
    }

    std::string func_name = builtin->str();
    auto& signature = builtin->Signature();

    auto check_arg_is_constexpr = [&](core::ParameterUsage usage, int min, int max) {
        auto signed_index = signature.IndexOf(usage);
        if (signed_index < 0) {
            return true;
        }
        auto index = static_cast<size_t>(signed_index);
        auto* arg = call->Arguments()[index];
        if (auto values = arg->ConstantValue()) {
            if (auto* vector = values->Type()->As<core::type::Vector>()) {
                for (size_t i = 0; i < vector->Width(); i++) {
                    auto value = values->Index(i)->ValueAs<AInt>();
                    if (value < min || value > max) {
                        AddError(arg->Declaration()->source)
                            << "each component of the " << usage << " argument must be at least "
                            << min << " and at most " << max << ". " << usage << " component " << i
                            << " is " << value;
                        return false;
                    }
                }
            } else {
                auto value = values->ValueAs<AInt>();
                if (value < min || value > max) {
                    AddError(arg->Declaration()->source)
                        << "the " << usage << " argument must be at least " << min
                        << " and at most " << max << ". " << usage << " is " << value;
                    return false;
                }
            }
            return true;
        }
        AddError(arg->Declaration()->source)
            << "the " << usage << " argument must be a const-expression";
        return false;
    };

    return check_arg_is_constexpr(core::ParameterUsage::kOffset, -8, 7) &&
           check_arg_is_constexpr(core::ParameterUsage::kComponent, 0, 3);
}

bool Validator::SubgroupBroadcast(const sem::Call* call) const {
    auto* builtin = call->Target()->As<sem::BuiltinFn>();
    if (!builtin) {
        return false;
    }

    TINT_ASSERT(call->Arguments().Length() == 2);
    auto* id = call->Arguments()[1];
    auto* constant_value = id->ConstantValue();

    if (!constant_value) {
        AddError(id->Declaration()->source)
            << "the sourceLaneIndex argument of subgroupBroadcast must be a const-expression";
        return false;
    }

    if (id->Type()->IsSignedIntegerScalar()) {
        if (constant_value->ValueAs<i32>() < 0) {
            AddError(id->Declaration()->source)
                << "the sourceLaneIndex argument of subgroupBroadcast "
                   "must be greater than or equal to zero";
            return false;
        }
        if (constant_value->ValueAs<i32>() >= tint::internal_limits::kMaxSubgroupSize) {
            AddError(id->Declaration()->source)
                << "the sourceLaneIndex argument of subgroupBroadcast "
                   "must be less than "
                << tint::internal_limits::kMaxSubgroupSize;
            return false;
        }
    }

    if (id->Type()->IsUnsignedIntegerScalar() &&
        constant_value->ValueAs<u32>() >= tint::internal_limits::kMaxSubgroupSize) {
        AddError(id->Declaration()->source) << "the sourceLaneIndex argument of subgroupBroadcast "
                                               "must be less than "
                                            << tint::internal_limits::kMaxSubgroupSize;
        return false;
    }

    return true;
}

bool Validator::QuadBroadcast(const sem::Call* call) const {
    auto* builtin = call->Target()->As<sem::BuiltinFn>();
    if (!builtin) {
        return false;
    }

    TINT_ASSERT(call->Arguments().Length() == 2);
    auto* id = call->Arguments()[1];
    auto* constant_value = id->ConstantValue();

    if (!constant_value) {
        AddError(id->Declaration()->source)
            << "the id argument of quadBroadcast must be a const-expression";
        return false;
    }

    if (id->Type()->IsSignedIntegerScalar()) {
        if (constant_value->ValueAs<i32>() < 0) {
            AddError(id->Declaration()->source)
                << "the id argument of quadBroadcast must be greater than or equal to zero";
            return false;
        }
        if (constant_value->ValueAs<i32>() >= tint::internal_limits::kQuadSize) {
            AddError(id->Declaration()->source)
                << "the id argument of quadBroadcast must be less than "
                << tint::internal_limits::kQuadSize;
            return false;
        }
    }

    if (id->Type()->IsUnsignedIntegerScalar() &&
        constant_value->ValueAs<u32>() >= tint::internal_limits::kQuadSize) {
        AddError(id->Declaration()->source) << "the id argument of quadBroadcast must be less than "
                                            << tint::internal_limits::kQuadSize;
        return false;
    }

    return true;
}

bool Validator::RequiredFeaturesForBuiltinFn(const sem::Call* call) const {
    const auto* builtin = call->Target()->As<sem::BuiltinFn>();
    if (!builtin) {
        return true;
    }

    if (builtin->IsSubgroup()) {
        if (!enabled_extensions_.Contains(wgsl::Extension::kSubgroups)) {
            AddError(call->Declaration()->source)
                << "cannot call built-in function " << style::Function(builtin->Fn())
                << " without extension "
                << style::Code(wgsl::ToString(wgsl::Extension::kSubgroups));
            return false;
        }
    }

    const auto feature = builtin->RequiredLanguageFeature();
    if (feature != wgsl::LanguageFeature::kUndefined) {
        if (!allowed_features_.features.count(feature)) {
            AddError(call->Declaration()->source)
                << "built-in function " << style::Function(builtin->Fn()) << " requires the "
                << style::Code(wgsl::ToString(feature))
                << " language feature, which is not allowed in the current environment";
            return false;
        }
    }

    return true;
}

bool Validator::CheckF16Enabled(const Source& source) const {
    // Validate if f16 type is allowed.
    if (!enabled_extensions_.Contains(wgsl::Extension::kF16)) {
        AddError(source) << style::Type("f16") << " type used without " << style::Code("f16")
                         << " extension enabled";
        return false;
    }
    return true;
}

bool Validator::CheckI8Enabled(const Source& source) const {
    // Validate if i8 type is allowed.
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalSubgroupMatrix)) {
        AddError(source) << style::Type("i8") << " type used without "
                         << style::Code("chromium_experimental_subgroup_matrix")
                         << " extension enabled";
        return false;
    }
    return true;
}

bool Validator::CheckU8Enabled(const Source& source) const {
    // Validate if u8 type is allowed.
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalSubgroupMatrix)) {
        AddError(source) << style::Type("u8") << " type used without "
                         << style::Code("chromium_experimental_subgroup_matrix")
                         << " extension enabled";
        return false;
    }
    return true;
}

bool Validator::FunctionCall(const sem::Call* call, sem::Statement* current_statement) const {
    auto* decl = call->Declaration();
    auto* target = call->Target()->As<sem::Function>();
    auto sym = target->Declaration()->name->symbol;
    auto name = sym.Name();

    if (!current_statement) {  // Function call at module-scope.
        AddError(decl->source) << "user-declared functions cannot be called at module-scope";
        return false;
    }

    if (target->Declaration()->IsEntryPoint()) {
        // https://www.w3.org/TR/WGSL/#function-restriction
        // An entry point must never be the target of a function call.
        AddError(decl->source) << "entry point functions cannot be the target of a function call";
        return false;
    }

    if (decl->args.Length() != target->Parameters().Length()) {
        bool more = decl->args.Length() > target->Parameters().Length();
        AddError(decl->source) << "too "
                               << (more ? std::string("many") : std::string("few")) +
                                      " arguments in call to "
                               << style::Function(name) << ", expected "
                               << target->Parameters().Length() << ", got "
                               << call->Arguments().Length();
        return false;
    }

    for (size_t i = 0; i < call->Arguments().Length(); ++i) {
        const sem::Variable* param = target->Parameters()[i];
        const ast::Expression* arg_expr = decl->args[i];
        auto* param_type = param->Type();
        auto* arg_type = sem_.TypeOf(arg_expr)->UnwrapRef();

        if (param_type != arg_type) {
            AddError(arg_expr->source) << "type mismatch for argument " << (i + 1) << " in call to "
                                       << style::Function(name) << ", expected "
                                       << style::Type(sem_.TypeNameOf(param_type)) << ", got "
                                       << style::Type(sem_.TypeNameOf(arg_type));
            return false;
        }

        if (param_type->Is<core::type::Pointer>() &&
            (allowed_features_.features.count(
                 wgsl::LanguageFeature::kUnrestrictedPointerParameters) == 0u)) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
            // Each argument of pointer type to a user-defined function must have the same memory
            // view as its root identifier.
            // We can validate this by just comparing the store type of the argument with that of
            // its root identifier, as these will match iff the memory view is the same.
            auto* arg_store_type = arg_type->As<core::type::Pointer>()->StoreType();
            auto* root = call->Arguments()[i]->RootIdentifier();
            auto* root_ptr_ty = root->Type()->As<core::type::Pointer>();
            auto* root_ref_ty = root->Type()->As<core::type::Reference>();
            TINT_ASSERT(root_ptr_ty || root_ref_ty);
            const core::type::Type* root_store_type;
            if (root_ptr_ty) {
                root_store_type = root_ptr_ty->StoreType();
            } else {
                root_store_type = root_ref_ty->StoreType();
            }
            if (root_store_type != arg_store_type) {
                AddError(arg_expr->source) << "arguments of pointer type must not point to a "
                                              "subset of the originating variable";
                return false;
            }
        }
    }

    if (call->Type()->Is<core::type::Void>()) {
        bool is_call_statement = false;
        if (auto* call_stmt = As<ast::CallStatement>(call->Stmt()->Declaration())) {
            if (call_stmt->expr == call->Declaration()) {
                is_call_statement = true;
            }
        }
        if (!is_call_statement) {
            // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
            // If the called function does not return a value, a function call
            // statement should be used instead.
            AddError(decl->source)
                << "function " << style::Function(name) << " does not return a value";
            return false;
        }
    }

    return true;
}

bool Validator::StructureInitializer(const ast::CallExpression* ctor,
                                     const core::type::Struct* struct_type) const {
    if (!struct_type->IsConstructible()) {
        AddError(ctor->source) << "structure constructor has non-constructible type";
        return false;
    }

    if (ctor->args.Length() > 0) {
        if (ctor->args.Length() != struct_type->Members().Length()) {
            std::string fm = ctor->args.Length() < struct_type->Members().Length() ? "few" : "many";
            AddError(ctor->source)
                << "structure constructor has too " << fm << " inputs: expected "
                << struct_type->Members().Length() << ", found " << ctor->args.Length();
            return false;
        }
        for (auto* member : struct_type->Members()) {
            auto* value = ctor->args[member->Index()];
            auto* value_ty = sem_.TypeOf(value);
            if (member->Type() != value_ty->UnwrapRef()) {
                AddError(value->source)
                    << "type in structure constructor does not match struct member type: expected "
                    << style::Type(sem_.TypeNameOf(member->Type())) << ", found "
                    << style::Type(sem_.TypeNameOf(value_ty));
                return false;
            }
        }
    }
    return true;
}

bool Validator::ArrayConstructor(const ast::CallExpression* ctor,
                                 const sem::Array* array_type) const {
    auto& values = ctor->args;
    if (values.Length() > internal_limits::kMaxArrayConstructorElements) {
        AddError(ctor->target->source) << "array constructor has excessive number of elements (>"
                                       << internal_limits::kMaxArrayConstructorElements << ")";
        return false;
    }

    auto* elem_ty = array_type->ElemType();
    for (auto* value : values) {
        auto* value_ty = sem_.TypeOf(value)->UnwrapRef();
        if (core::type::Type::ConversionRank(value_ty, elem_ty) ==
            core::type::Type::kNoConversion) {
            AddError(value->source) << style::Type(sem_.TypeNameOf(value_ty))
                                    << " cannot be used to construct an array of "
                                    << style::Type(sem_.TypeNameOf(elem_ty));
            return false;
        }
    }

    auto* c = array_type->Count();
    if (c->Is<core::type::RuntimeArrayCount>()) {
        AddError(ctor->source) << "cannot construct a runtime-sized array";
        return false;
    }

    if (c->IsAnyOf<sem::NamedOverrideArrayCount, sem::UnnamedOverrideArrayCount>()) {
        AddError(ctor->source) << "cannot construct an array that has an override-expression count";
        return false;
    }

    if (!elem_ty->IsConstructible()) {
        AddError(ctor->source) << "array constructor has non-constructible element type";
        return false;
    }

    if (DAWN_UNLIKELY(!c->Is<core::type::ConstantArrayCount>())) {
        TINT_ICE() << "Invalid ArrayCount found";
    }

    const auto count = c->As<core::type::ConstantArrayCount>()->value;
    if (!values.IsEmpty() && (values.Length() != count)) {
        std::string fm = values.Length() < count ? "few" : "many";
        AddError(ctor->source) << "array constructor has too " << fm << " elements: expected "
                               << count << ", found " << values.Length();
        return false;
    }
    return true;
}

bool Validator::SubgroupMatrixConstructor(
    const ast::CallExpression* ctor,
    const core::type::SubgroupMatrix* subgroup_matrix_type) const {
    auto& values = ctor->args;
    if (values.Length() == 1) {
        auto* elem_ty = subgroup_matrix_type->Type();
        auto* value_ty = sem_.TypeOf(values[0])->UnwrapRef();
        if (core::type::Type::ConversionRank(value_ty, elem_ty) ==
            core::type::Type::kNoConversion) {
            AddError(values[0]->source) << style::Type(sem_.TypeNameOf(value_ty))
                                        << " cannot be used to construct a subgroup matrix of "
                                        << style::Type(sem_.TypeNameOf(elem_ty));
            return false;
        }
    } else if (values.Length() > 1) {
        AddError(ctor->target->source)
            << "subgroup_matrix constructor can only have zero or one elements";
        return false;
    }

    return true;
}

bool Validator::Vector(const core::type::Type* el_ty, const Source& source) const {
    if (!el_ty->Is<core::type::Scalar>() || el_ty->Is<core::type::I8>() ||
        el_ty->Is<core::type::U8>()) {
        AddError(source) << "vector element type must be " << style::Type("bool") << ", "
                         << style::Type("f32") << ", " << style::Type("f16") << ", "
                         << style::Type("i32") << " or " << style::Type("u32");
        return false;
    }
    return true;
}

bool Validator::Matrix(const core::type::Type* el_ty, const Source& source) const {
    if (!el_ty->IsFloatScalar()) {
        AddError(source) << "matrix element type must be " << style::Type("f32") << " or "
                         << style::Type("f16");
        return false;
    }
    return true;
}

bool Validator::PipelineStages(VectorRef<sem::Function*> entry_points) const {
    auto var_source = [&](const sem::Function* func, const sem::GlobalVariable* var) {
        for (auto* user : var->Users()) {
            if (user->Stmt() && func == user->Stmt()->Function()) {
                return user->Declaration()->source;
            }
        }
        return Source();
    };

    auto backtrace = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (func != entry_point) {
            TraverseCallChain(entry_point, func, [&](const sem::Function* f) {
                AddNote(f->Declaration()->source)
                    << "called by function "
                    << style::Function(f->Declaration()->name->symbol.NameView());
            });
            AddNote(entry_point->Declaration()->source)
                << "called by entry point "
                << style::Function(entry_point->Declaration()->name->symbol.NameView());
        }
    };

    auto check_var_uses = [&](const sem::Function* func, const sem::Function* entry_point) {
        auto var_decl_note = [&](const sem::GlobalVariable* var) {
            AddNote(var->Declaration()->source) << "variable is declared here";
            backtrace(func, entry_point);
        };

        auto stage = entry_point->Declaration()->PipelineStage();
        for (auto* var : func->DirectlyReferencedGlobals()) {
            Source source = var_source(func, var);
            if ((stage != ast::PipelineStage::kCompute &&
                 var->AddressSpace() == core::AddressSpace::kWorkgroup) ||
                (stage != ast::PipelineStage::kFragment &&
                 var->AddressSpace() == core::AddressSpace::kPixelLocal)) {
                AddError(source) << "var with " << style::Enum(var->AddressSpace())
                                 << " address space cannot be used by " << stage
                                 << " pipeline stage";
                var_decl_note(var);
                return false;
            }
            if (stage == ast::PipelineStage::kVertex &&
                var->AddressSpace() == core::AddressSpace::kStorage &&
                var->Access() != core::Access::kRead) {
                AddError(source) << "var with " << style::Enum(var->AddressSpace())
                                 << " address space and " << style::Enum(var->Access())
                                 << " access mode cannot be used by " << stage << " pipeline stage";
                var_decl_note(var);
                return false;
            }
            auto* storage = var->Type()->UnwrapRef()->As<core::type::StorageTexture>();
            if (stage == ast::PipelineStage::kVertex && storage &&
                storage->Access() != core::Access::kRead) {
                AddError(source) << "storage texture with " << style::Enum(storage->Access())
                                 << " access mode cannot be used by " << stage << " pipeline stage";
                var_decl_note(var);
                return false;
            }
        }
        return true;
    };

    auto check_builtin_calls = [&](const sem::Function* func, const sem::Function* entry_point) {
        auto stage = entry_point->Declaration()->PipelineStage();
        for (auto* builtin : func->DirectlyCalledBuiltins()) {
            if (!builtin->SupportedStages().Contains(stage)) {
                auto* call = func->FindDirectCallTo(builtin);
                AddError(call ? call->Declaration()->source : func->Declaration()->source)
                    << "built-in cannot be used by " << stage << " pipeline stage";
                backtrace(func, entry_point);
                return false;
            }
        }
        return true;
    };

    auto check_no_discards = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (auto* discard = func->DiscardStatement()) {
            auto stage = entry_point->Declaration()->PipelineStage();
            AddError(discard->Declaration()->source)
                << "discard statement cannot be used in " << stage << " pipeline stage";
            backtrace(func, entry_point);
            return false;
        }
        return true;
    };

    auto check_no_subgroup_matrix = [&](const sem::Function* func,
                                        const sem::Function* entry_point) {
        if (auto matrix_use = func->DirectlyUsedSubgroupMatrix()) {
            auto stage = entry_point->Declaration()->PipelineStage();
            AddError(*(matrix_use.value()))
                << "subgroup matrix type cannot be used in " << stage << " pipeline stage";
            backtrace(func, entry_point);
            return false;
        }
        return true;
    };

    auto check_func = [&](const sem::Function* func, const sem::Function* entry_point) {
        if (!check_var_uses(func, entry_point)) {
            return false;
        }
        if (!check_builtin_calls(func, entry_point)) {
            return false;
        }
        if (entry_point->Declaration()->PipelineStage() != ast::PipelineStage::kFragment) {
            if (!check_no_discards(func, entry_point)) {
                return false;
            }
        }
        if (entry_point->Declaration()->PipelineStage() != ast::PipelineStage::kCompute) {
            if (!check_no_subgroup_matrix(func, entry_point)) {
                return false;
            }
        }
        return true;
    };

    for (auto* entry_point : entry_points) {
        if (!check_func(entry_point, entry_point)) {
            return false;
        }
        for (auto* func : entry_point->TransitivelyCalledFunctions()) {
            if (!check_func(func, entry_point)) {
                return false;
            }
        }
    }

    return true;
}

bool Validator::ModuleScopeVarUsages(VectorRef<sem::Function*> entry_points) const {
    for (auto* entry_point : entry_points) {
        if (!CheckNoMultipleModuleScopeVarsOfAddressSpace(entry_point,
                                                          core::AddressSpace::kImmediate)) {
            return false;
        }
        if (!CheckNoMultipleModuleScopeVarsOfAddressSpace(entry_point,
                                                          core::AddressSpace::kPixelLocal)) {
            return false;
        }
    }

    return true;
}

bool Validator::Array(const sem::Array* arr, const Source& el_source) const {
    auto* el_ty = arr->ElemType();

    if (!IsPlain(el_ty)) {
        AddError(el_source) << sem_.TypeNameOf(el_ty)
                            << " cannot be used as an element type of an array";
        return false;
    }

    if (!el_ty->HasFixedFootprint()) {
        AddError(el_source) << "an array element type cannot contain a runtime-sized array";
        return false;
    }

    if (IsArrayWithOverrideCount(el_ty)) {
        RaiseArrayWithOverrideCountError(el_source);
        return false;
    }

    return true;
}

bool Validator::ArrayStrideAttribute(const ast::StrideAttribute* attr,
                                     uint32_t el_size,
                                     uint32_t el_align) const {
    auto stride = attr->stride;
    bool is_valid_stride = (stride >= el_size) && (stride >= el_align) && (stride % el_align == 0);
    if (!is_valid_stride) {
        // https://gpuweb.github.io/gpuweb/wgsl/#array-layout-rules
        // Arrays decorated with the stride attribute must have a stride that is
        // at least the size of the element type, and be a multiple of the
        // element type's alignment value.
        AddError(attr->source)
            << "arrays decorated with the stride attribute must have a stride that is at least the "
               "size of the element type, and be a multiple of the element type's alignment value";
        return false;
    }
    return true;
}

bool Validator::Alias(const ast::Alias*) const {
    return true;
}

bool Validator::Structure(const sem::Struct* str, ast::PipelineStage stage) const {
    if (str->Members().IsEmpty()) {
        AddError(str->Declaration()->name->source) << "structures must have at least one member";
        return false;
    }

    std::bitset<2> blend_src_appearance_mask;
    const ast::BlendSrcAttribute* first_blend_src = nullptr;
    const core::type::Type* first_blend_src_type = nullptr;
    const ast::LocationAttribute* first_location_without_blend_src = nullptr;
    Hashset<std::pair<uint32_t, std::optional<uint32_t>>, 8> locations_and_blend_srcs;
    Hashset<uint32_t, 4> colors;
    for (auto* member : str->Members()) {
        if (auto* r = member->Type()->As<sem::Array>()) {
            if (r->Count()->Is<core::type::RuntimeArrayCount>()) {
                if (member != str->Members().Back()) {
                    AddError(member->Declaration()->source)
                        << "runtime arrays may only appear as the last member of a struct";
                    return false;
                }
            }

            if (IsArrayWithOverrideCount(member->Type())) {
                RaiseArrayWithOverrideCountError(member->Declaration()->type->source);
                return false;
            }
        } else if (!member->Type()->HasFixedFootprint()) {
            AddError(member->Declaration()->source)
                << "a struct that contains a runtime array cannot be nested inside another struct";
            return false;
        }

        auto has_position = false;
        const ast::BlendSrcAttribute* blend_src_attribute = nullptr;
        const ast::LocationAttribute* location_attribute = nullptr;
        const ast::ColorAttribute* color_attribute = nullptr;
        const ast::InvariantAttribute* invariant_attribute = nullptr;
        const ast::InterpolateAttribute* interpolate_attribute = nullptr;
        for (auto* attr : member->Declaration()->attributes) {
            bool ok = Switch(
                attr,  //
                [&](const ast::InvariantAttribute* invariant) {
                    invariant_attribute = invariant;
                    return InvariantAttribute(invariant, stage);
                },
                [&](const ast::LocationAttribute* location) {
                    location_attribute = location;
                    TINT_ASSERT(member->Attributes().location.has_value());
                    return LocationAttribute(location, member->Type(), stage,
                                             member->Declaration()->source);
                },
                [&](const ast::BlendSrcAttribute* blend_src) {
                    blend_src_attribute = blend_src;
                    return BlendSrcAttribute(blend_src, stage);
                },
                [&](const ast::ColorAttribute* color) {
                    color_attribute = color;
                    return ColorAttribute(color, member->Type(), stage,
                                          member->Declaration()->source);
                },
                [&](const ast::BuiltinAttribute* builtin_attr) {
                    if (!BuiltinAttribute(builtin_attr, member->Type(), stage,
                                          /* is_input */ false)) {
                        return false;
                    }
                    if (builtin_attr->builtin == core::BuiltinValue::kPosition) {
                        has_position = true;
                    }
                    return true;
                },
                [&](const ast::InterpolateAttribute* interpolate) {
                    interpolate_attribute = interpolate;
                    return InterpolateAttribute(interpolate, member->Type(), stage);
                },
                [&](const ast::StructMemberSizeAttribute*) {
                    if (!member->Type()->HasCreationFixedFootprint()) {
                        AddError(attr->source)
                            << style::Attribute("@size")
                            << " can only be applied to members where the member's type size can "
                               "be fully determined at shader creation time";
                        return false;
                    }
                    return true;
                },
                [&](Default) { return true; });
            if (!ok) {
                return false;
            }
        }

        if (invariant_attribute && !has_position) {
            AddError(invariant_attribute->source)
                << style::Attribute("@invariant") << " must be applied to a position builtin";
            return false;
        }

        if (blend_src_attribute) {
            if (member->Attributes().location.value_or(1) != 0) {
                AddError(blend_src_attribute->source)
                    << style::Attribute("@blend_src") << " can only be used with "
                    << style::Attribute("@location") << style::Code("(", style::Literal("0"), ")");
                return false;
            }

            if (first_blend_src == nullptr) {
                first_blend_src = blend_src_attribute;
                first_blend_src_type = member->Type();
            } else if (!first_blend_src_type->Equals(*member->Type())) {
                AddError(blend_src_attribute->source)
                    << "All the outputs with " << style::Attribute("@blend_src")
                    << " must have same type";
                return false;
            }

            TINT_ASSERT(member->Attributes().blend_src.has_value() &&
                        *member->Attributes().blend_src <= 1u);
            uint32_t blend_src = *member->Attributes().blend_src;
            blend_src_appearance_mask.set(blend_src);
        } else if (location_attribute) {
            first_location_without_blend_src = location_attribute;
        }

        if (first_blend_src && first_location_without_blend_src) {
            AddError(member->Declaration()->source)
                << style::Attribute("@blend_src") << " and " << style::Attribute("@location")
                << " are used on one member while another member with "
                << style::Attribute("@location") << " doesn't use "
                << style::Attribute("@blend_src");
            return false;
        }

        if (interpolate_attribute && !location_attribute) {
            AddError(interpolate_attribute->source)
                << style::Attribute("@interpolate") << " can only be used with "
                << style::Attribute("@location");
            return false;
        }

        // Ensure all locations and optional blend_src pairs are unique
        if (location_attribute) {
            uint32_t location = member->Attributes().location.value();
            std::optional<uint32_t> blend_src = member->Attributes().blend_src;

            if (!locations_and_blend_srcs.Add(std::make_pair(location, blend_src))) {
                auto& err = AddError(location_attribute->source)
                            << style::Attribute("@location")
                            << style::Code("(", style::Literal(location), ")");
                if (blend_src) {
                    err << style::Attribute(" @blend_src")
                        << style::Code("(", style::Literal(blend_src.value()), ")");
                }
                err << " appears multiple times";
                return false;
            }
        }

        if (color_attribute) {
            uint32_t color = member->Attributes().color.value();
            if (!colors.Add(color)) {
                AddError(color_attribute->source)
                    << style::Attribute("@color") << style::Code("(", style::Literal(color), ")")
                    << " appears multiple times";
                return false;
            }
        }
    }

    if (first_blend_src != nullptr && !blend_src_appearance_mask.all()) {
        for (uint32_t i = 0; i < blend_src_appearance_mask.size(); ++i) {
            if (!blend_src_appearance_mask.test(i)) {
                AddError(first_blend_src->source)
                    << style::Attribute("@blend_src") << style::Code("(", style::Literal(i), ")")
                    << " is missing when " << style::Attribute("@blend_src") << " is used";
            }
        }
        return false;
    }

    return true;
}

bool Validator::LocationAttribute(const ast::LocationAttribute* attr,
                                  const core::type::Type* type,
                                  ast::PipelineStage stage,
                                  const Source& source) const {
    if (stage == ast::PipelineStage::kCompute) {
        AddError(attr->source) << style::Attribute("@", attr->Name())
                               << " cannot be used by compute shaders";
        return false;
    }

    if (!type->IsNumericScalarOrVector()) {
        std::string invalid_type = sem_.TypeNameOf(type);
        AddError(source) << "cannot apply " << style::Attribute("@location")
                         << " to declaration of type " << style::Type(invalid_type);
        AddNote(attr->source)
            << style::Attribute("@location")
            << " must only be applied to declarations of numeric scalar or numeric vector type";
        return false;
    }

    return true;
}

bool Validator::ColorAttribute(const ast::ColorAttribute* attr,
                               const core::type::Type* type,
                               ast::PipelineStage stage,
                               const Source& source,
                               const std::optional<bool> is_input) const {
    if (!enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalFramebufferFetch)) {
        AddError(attr->source) << "use of " << style::Attribute("@color")
                               << " requires enabling extension "
                               << style::Code("chromium_experimental_framebuffer_fetch");
        return false;
    }

    bool is_stage_non_fragment =
        stage != ast::PipelineStage::kNone && stage != ast::PipelineStage::kFragment;
    bool is_output = !is_input.value_or(true);
    if (is_stage_non_fragment || is_output) {
        AddError(attr->source) << style::Attribute("@color")
                               << " can only be used for fragment shader input";
        return false;
    }

    if (!type->IsNumericScalarOrVector()) {
        std::string invalid_type = sem_.TypeNameOf(type);
        AddError(source) << "cannot apply " << style::Attribute("@color")
                         << " to declaration of type " << style::Type(invalid_type);
        AddNote(attr->source)
            << style::Attribute("@color")
            << " must only be applied to declarations of numeric scalar or numeric vector type";
        return false;
    }

    return true;
}

bool Validator::BlendSrcAttribute(const ast::BlendSrcAttribute* attr,
                                  ast::PipelineStage stage,
                                  const std::optional<bool> is_input) const {
    if (!enabled_extensions_.Contains(wgsl::Extension::kDualSourceBlending)) {
        AddError(attr->source) << "use of " << style::Attribute("@blend_src")
                               << " requires enabling extension "
                               << style::Code("dual_source_blending");
        return false;
    }

    bool is_stage_non_fragment =
        stage != ast::PipelineStage::kNone && stage != ast::PipelineStage::kFragment;
    bool is_input_param = is_input.value_or(false);
    if (is_stage_non_fragment || is_input_param) {
        AddError(attr->source) << style::Attribute("@", attr->Name())
                               << " can only be used for fragment shader output";
        return false;
    }

    return true;
}

bool Validator::Return(const ast::ReturnStatement* ret,
                       const core::type::Type* func_type,
                       const core::type::Type* ret_type,
                       sem::Statement* current_statement) const {
    if (func_type->UnwrapRef() != ret_type) {
        AddError(ret->source)
            << "return statement type must match its function return type, returned "
            << style::Type(sem_.TypeNameOf(ret_type)) << ", expected "
            << style::Type(sem_.TypeNameOf(func_type));
        return false;
    }

    auto* sem = sem_.Get(ret);
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ false, /* stop_at_switch */ false,
                                             current_statement)) {
        AddError(ret->source) << "continuing blocks must not contain a return statement";
        if (continuing != sem->Declaration() && continuing != sem->Parent()->Declaration()) {
            AddNote(continuing->source) << "see continuing block here";
        }
        return false;
    }

    return true;
}

bool Validator::SwitchStatement(const ast::SwitchStatement* s) {
    if (s->body.Length() > kMaxSwitchCaseSelectors) {
        AddError(s->source) << "switch statement has " << s->body.Length()
                            << " case selectors, max is " << kMaxSwitchCaseSelectors;
        return false;
    }

    auto* cond_ty = sem_.TypeOf(s->condition);
    if (!cond_ty->IsIntegerScalar()) {
        AddError(s->condition->source)
            << "switch statement selector expression must be of a scalar integer type";
        return false;
    }

    const sem::CaseSelector* default_selector = nullptr;
    Hashmap<int64_t, Source, 4> selectors;

    for (auto* case_stmt : s->body) {
        auto* case_sem = sem_.Get<sem::CaseStatement>(case_stmt);
        for (auto* selector : case_sem->Selectors()) {
            if (selector->IsDefault()) {
                if (default_selector != nullptr) {
                    // More than one default clause
                    AddError(selector->Declaration()->source)
                        << "switch statement must have exactly one default clause";

                    AddNote(default_selector->Declaration()->source) << "previous default case";
                    return false;
                }
                default_selector = selector;
                continue;
            }

            auto* decl_ty = selector->Value()->Type();
            if (cond_ty != decl_ty) {
                AddError(selector->Declaration()->source)
                    << "the case selector values must have the same type as the "
                       "selector expression.";
                return false;
            }

            auto value = selector->Value()->ValueAs<u32>();
            if (auto added = selectors.Add(value, selector->Declaration()->source); !added) {
                auto& err = AddError(selector->Declaration()->source)
                            << "duplicate switch case " << style::Literal;
                if (decl_ty->IsAnyOf<core::type::I32, core::type::AbstractNumeric>()) {
                    err << i32(value);
                } else {
                    err << value;
                }
                AddNote(added.value) << "previous case declared here";
                return false;
            }
        }
    }

    if (default_selector == nullptr) {
        // No default clause
        AddError(s->source) << "switch statement must have a default clause";
        return false;
    }

    return true;
}

bool Validator::Assignment(const ast::Statement* a, const core::type::Type* rhs_ty) const {
    const ast::Expression* lhs;
    const ast::Expression* rhs;
    if (auto* assign = a->As<ast::AssignmentStatement>()) {
        lhs = assign->lhs;
        rhs = assign->rhs;
    } else if (auto* compound = a->As<ast::CompoundAssignmentStatement>()) {
        lhs = compound->lhs;
        rhs = compound->rhs;
        if (!BinaryExpression(a, compound->op, sem_.GetVal(lhs), sem_.GetVal(rhs))) {
            return false;
        }
    } else {
        TINT_ICE() << "invalid assignment statement";
    }

    if (lhs->Is<ast::PhonyExpression>()) {
        // https://www.w3.org/TR/WGSL/#phony-assignment-section
        auto* ty = rhs_ty->UnwrapRef();
        if (!ty->IsConstructible() &&
            !ty->IsAnyOf<core::type::Pointer, core::type::Texture, core::type::Sampler,
                         core::type::AbstractNumeric>()) {
            AddError(rhs->source)
                << "cannot assign " << style::Type(sem_.TypeNameOf(rhs_ty)) << " to "
                << style::Code("_") << ". " << style::Code("_")
                << " can only be assigned a constructible, pointer, texture or sampler type";
            return false;
        }
        return true;  // RHS can be anything.
    }

    // https://gpuweb.github.io/gpuweb/wgsl/#assignment-statement
    auto const* lhs_sem = sem_.GetVal(lhs);
    auto const* lhs_ty = lhs_sem->Type();

    auto* lhs_ref = lhs_ty->As<core::type::Reference>();
    if (!lhs_ref) {
        // LHS is not a reference, so it has no storage.
        AddError(lhs->source) << "cannot assign to " << sem_.Describe(lhs_sem);

        auto* expr = lhs;
        while (expr) {
            expr = Switch(
                expr,  //
                [&](const ast::AccessorExpression* e) { return e->object; },
                [&](const ast::IdentifierExpression* i) {
                    if (auto user = sem_.Get<sem::VariableUser>(i)) {
                        Switch(
                            user->Variable()->Declaration(),  //
                            [&](const ast::Let* v) {
                                AddNote(user->Declaration()->source)
                                    << style::Variable("let") << " variables are immutable";
                                sem_.NoteDeclarationSource(v);
                            },
                            [&](const ast::Const* v) {
                                AddNote(user->Declaration()->source)
                                    << style::Variable("const") << " variables are immutable";
                                sem_.NoteDeclarationSource(v);
                            },
                            [&](const ast::Override* v) {
                                AddNote(user->Declaration()->source)
                                    << style::Variable("override") << " variables are immutable";
                                sem_.NoteDeclarationSource(v);
                            },
                            [&](const ast::Parameter* v) {
                                AddNote(user->Declaration()->source) << "parameters are immutable";
                                sem_.NoteDeclarationSource(v);
                            });
                    }
                    return nullptr;
                });
        }

        return false;
    }

    auto* storage_ty = lhs_ref->StoreType();
    auto* value_type = rhs_ty->UnwrapRef();  // Implicit load of RHS

    // Value type has to match storage type
    if (storage_ty != value_type) {
        AddError(a->source) << "cannot assign " << style::Type(sem_.TypeNameOf(rhs_ty)) << " to "
                            << style::Type(sem_.TypeNameOf(lhs_ty));
        return false;
    }
    if (!storage_ty->IsConstructible()) {
        AddError(a->source) << "storage type of assignment must be constructible";
        return false;
    }
    if (lhs_ref->Access() == core::Access::kRead) {
        AddError(a->source) << "cannot store into a read-only type "
                            << style::Type(sem_.RawTypeNameOf(lhs_ty));
        return false;
    }
    return true;
}

bool Validator::IncrementDecrementStatement(const ast::IncrementDecrementStatement* inc) const {
    const ast::Expression* lhs = inc->lhs;

    // https://gpuweb.github.io/gpuweb/wgsl/#increment-decrement

    if (auto* var_user = sem_.Get<sem::VariableUser>(lhs)) {
        auto* v = var_user->Variable()->Declaration();
        bool errored = Switch(
            v,  //
            [&](const ast::Parameter*) {
                AddError(lhs->source) << "cannot modify function parameter";
                return true;
            },
            [&](const ast::Let*) {
                AddError(lhs->source) << "cannot modify " << style::Keyword("let");
                return true;
            },
            [&](const ast::Override*) {
                AddError(lhs->source) << "cannot modify " << style::Keyword("override");
                return true;
            });
        if (errored) {
            sem_.NoteDeclarationSource(v);
            return false;
        }
    }

    auto const* lhs_ty = sem_.TypeOf(lhs);
    auto* lhs_ref = lhs_ty->As<core::type::Reference>();
    if (!lhs_ref) {
        // LHS is not a reference, so it has no storage.
        AddError(lhs->source) << "cannot modify value of type "
                              << style::Type(sem_.TypeNameOf(lhs_ty));
        return false;
    }

    if (!lhs_ref->StoreType()->IsIntegerScalar()) {
        const std::string kind = inc->increment ? "increment" : "decrement";
        AddError(lhs->source) << kind << " statement can only be applied to an integer scalar";
        return false;
    }

    if (lhs_ref->Access() == core::Access::kRead) {
        AddError(inc->source) << "cannot modify read-only type "
                              << style::Type(sem_.RawTypeNameOf(lhs_ty));
        return false;
    }
    return true;
}

bool Validator::NoDuplicateAttributes(VectorRef<const ast::Attribute*> attributes) const {
    Hashmap<const tint::TypeInfo*, Source, 8> seen;
    tint::Vector<const ast::DiagnosticControl*, 8> diagnostic_controls;
    for (auto* d : attributes) {
        if (auto* diag = d->As<ast::DiagnosticAttribute>()) {
            // Allow duplicate diagnostic attributes, and check for conflicts later.
            diagnostic_controls.Push(&diag->control);
        } else {
            auto added = seen.Add(&d->TypeInfo(), d->source);
            if (!added && !d->Is<ast::InternalAttribute>()) {
                AddError(d->source) << "duplicate " << d->Name() << " attribute";
                AddNote(added.value) << "first attribute declared here";
                return false;
            }
        }
    }
    return DiagnosticControls(diagnostic_controls, "attribute", DiagnosticDuplicates::kDenied);
}

bool Validator::DiagnosticControls(VectorRef<const ast::DiagnosticControl*> controls,
                                   const char* use,
                                   DiagnosticDuplicates allow_duplicates) const {
    // Make sure that no two diagnostic controls conflict.
    // They conflict if the rule name is the same and the severity is different.
    Hashmap<std::pair<Symbol, Symbol>, const ast::DiagnosticControl*, 8> diagnostics;
    for (auto* dc : controls) {
        auto category = dc->rule_name->category ? dc->rule_name->category->symbol : Symbol();
        auto name = dc->rule_name->name->symbol;

        auto diag_added = diagnostics.Add(std::make_pair(category, name), dc);
        if (!diag_added) {
            if (diag_added.value->severity != dc->severity) {
                AddError(dc->rule_name->source) << "conflicting diagnostic " << use;
                AddNote(diag_added.value->rule_name->source)
                    << "severity of " << style::Code(dc->rule_name->String()) << " set to "
                    << style::Code(dc->severity) << " here";
                return false;
            } else if (allow_duplicates == DiagnosticDuplicates::kDenied) {
                AddError(dc->rule_name->source) << "duplicate diagnostic " << use;
                AddNote(diag_added.value->rule_name->source) << "first " << use << " declared here";
                return false;
            }
        }
    }
    return true;
}

bool Validator::IsValidationDisabled(VectorRef<const ast::Attribute*> attributes,
                                     ast::DisabledValidation validation) const {
    for (auto* attribute : attributes) {
        if (auto* dv = attribute->As<ast::DisableValidationAttribute>()) {
            if (dv->validation == validation) {
                return true;
            }
        }
    }
    return false;
}

bool Validator::IsValidationEnabled(VectorRef<const ast::Attribute*> attributes,
                                    ast::DisabledValidation validation) const {
    return !IsValidationDisabled(attributes, validation);
}

bool Validator::IsArrayWithOverrideCount(const core::type::Type* ty) const {
    if (auto* arr = ty->UnwrapRef()->As<sem::Array>()) {
        if (arr->Count()->IsAnyOf<sem::NamedOverrideArrayCount, sem::UnnamedOverrideArrayCount>()) {
            return true;
        }
    }
    return false;
}

void Validator::RaiseArrayWithOverrideCountError(const Source& source) const {
    AddError(source) << style::Type("array") << " with an " << style::Keyword("override")
                     << " element count can only be used as the store type of a "
                     << style::Keyword("var") << style::Code("<", style::Enum("workgroup"), ">");
}

std::string Validator::VectorPretty(uint32_t size, const core::type::Type* element_type) const {
    core::type::Vector vec_type(element_type, size);
    return vec_type.FriendlyName();
}

bool Validator::CheckTypeAccessAddressSpace(const core::type::Type* store_ty,
                                            core::Access access,
                                            core::AddressSpace address_space,
                                            const Source& source) const {
    if (!AddressSpaceLayout(store_ty, address_space, source)) {
        return false;
    }

    switch (address_space) {
        case core::AddressSpace::kPixelLocal:
            if (auto* str = store_ty->As<sem::Struct>()) {
                for (auto* member : str->Members()) {
                    using Allowed = std::tuple<core::type::I32, core::type::U32, core::type::F32>;
                    if (DAWN_UNLIKELY(!member->Type()->TypeInfo().IsAnyOfTuple<Allowed>())) {
                        AddError(member->Declaration()->source)
                            << style::Keyword("struct") << " members used in the "
                            << style::Enum("pixel_local")
                            << " address space can only be of the type " << style::Type("i32")
                            << ", " << style::Type("u32") << " or " << style::Type("f32");
                        AddNote(source) << style::Keyword("struct ")
                                        << style::Type(str->Name().Name()) << " used in the "
                                        << style::Enum("pixel_local") << " address space here";
                        return false;
                    }
                }
            } else if (DAWN_UNLIKELY(!store_ty->TypeInfo().Is<core::type::Struct>())) {
                AddError(source) << style::Enum("pixel_local")
                                 << " variable only support struct storage types";
                return false;
            }
            break;
        case core::AddressSpace::kImmediate:
            if (DAWN_UNLIKELY(!enabled_extensions_.Contains(
                    wgsl::Extension::kChromiumExperimentalImmediate))) {
                AddError(source) << "use of variable address space " << style::Enum("immediate")
                                 << " requires enabling extension "
                                 << style::Code("chromium_experimental_immediate");
                return false;
            }
            break;
        case core::AddressSpace::kStorage:
            if (DAWN_UNLIKELY(access == core::Access::kWrite)) {
                // The access mode for the storage address space can only be 'read' or 'read_write'.
                AddError(source) << "access mode " << style::Enum("write")
                                 << " is not valid for the " << style::Enum("storage")
                                 << " address space";
                return false;
            }
            break;
        default:
            break;
    }

    auto atomic_error = [&] {
        StyledText err;
        if (address_space != core::AddressSpace::kStorage &&
            address_space != core::AddressSpace::kWorkgroup) {
            AddError(source) << style::Type("atomic") << " variables must have "
                             << style::Enum("storage") << " or " << style::Enum("workgroup")
                             << " address space";
            return true;
        }
        if (address_space == core::AddressSpace::kStorage && access != core::Access::kReadWrite) {
            AddError(source) << "atomic variables in " << style::Enum("storage")
                             << " address space must have " << style::Enum("read_write")
                             << " access mode";
            return true;
        }
        return false;
    };

    auto check_sub_atomics = [&] {
        if (auto atomic_use = atomic_composite_info_.Get(store_ty)) {
            if (DAWN_UNLIKELY(atomic_error())) {
                AddNote(**atomic_use)
                    << "atomic sub-type of " << style::Type(sem_.TypeNameOf(store_ty))
                    << " is declared here";
                return false;
            }
        }
        return true;
    };

    return Switch(
        store_ty,  //
        [&](const core::type::Atomic*) {
            if (DAWN_UNLIKELY(atomic_error())) {
                return false;
            }
            return true;
        },
        [&](const core::type::Struct*) { return check_sub_atomics(); },  //
        [&](const sem::Array*) { return check_sub_atomics(); },          //
        [&](Default) { return true; });
}

bool Validator::CheckNoMultipleModuleScopeVarsOfAddressSpace(sem::Function* entry_point,
                                                             core::AddressSpace space) const {
    // State checked and modified by check() so that it remembers previously seen immediate
    // variables for an entry-point.
    const sem::Variable* seen_var = nullptr;
    const sem::Function* seen_func = nullptr;

    auto check = [&](const sem::Function* func, const sem::Function* ep) {
        for (auto* var : func->DirectlyReferencedGlobals()) {
            if (var->AddressSpace() != space || var == seen_var) {
                continue;
            }

            if (seen_var == nullptr) {
                seen_var = var;
                seen_func = func;
                continue;
            }

            AddError(ep->Declaration()->source)
                << "entry point " << style::Function(ep->Declaration()->name->symbol.NameView())
                << " uses two different " << style::Enum(space) << " variables.";
            AddNote(var->Declaration()->source)
                << "first " << style::Enum(space) << " variable declaration is here";
            if (func != ep) {
                TraverseCallChain(ep, func, [&](const sem::Function* f) {
                    AddNote(f->Declaration()->source)
                        << "called by function "
                        << style::Function(f->Declaration()->name->symbol.NameView());
                });
                AddNote(ep->Declaration()->source)
                    << "called by entry point "
                    << style::Function(ep->Declaration()->name->symbol.NameView());
            }
            AddNote(seen_var->Declaration()->source)
                << "second " << style::Enum(space) << " variable declaration is here";
            if (seen_func != ep) {
                TraverseCallChain(ep, seen_func, [&](const sem::Function* f) {
                    AddNote(f->Declaration()->source)
                        << "called by function "
                        << style::Function(f->Declaration()->name->symbol.NameView());
                });
                AddNote(ep->Declaration()->source)
                    << "called by entry point "
                    << style::Function(ep->Declaration()->name->symbol.NameView());
            }
            return false;
        }

        return true;
    };

    if (!check(entry_point, entry_point)) {
        return false;
    }
    for (auto* func : entry_point->TransitivelyCalledFunctions()) {
        if (!check(func, entry_point)) {
            return false;
        }
    }
    return true;
}

}  // namespace tint::resolver
