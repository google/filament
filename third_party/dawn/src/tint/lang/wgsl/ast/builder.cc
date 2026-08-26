// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/builder.h"

#include "src/tint/lang/core/enums.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {

Builder::VarOptions::~VarOptions() = default;

Builder::Builder() : ast_(ast_nodes_.Create<Module>(AllocateNodeID(), Source{})) {}

Builder::Builder(Builder&& rhs)
    : last_ast_node_id_(std::move(rhs.last_ast_node_id_)),
      ast_nodes_(std::move(rhs.ast_nodes_)),
      ast_(rhs.ast_),
      symbols_(std::move(rhs.symbols_)),
      diagnostics_(std::move(rhs.diagnostics_)) {
    rhs.MarkAsMoved();
}

Builder::~Builder() = default;

Builder& Builder::operator=(Builder&& rhs) {
    rhs.MarkAsMoved();
    AssertNotMoved();
    last_ast_node_id_ = std::move(rhs.last_ast_node_id_);
    ast_nodes_ = std::move(rhs.ast_nodes_);
    ast_ = std::move(rhs.ast_);
    symbols_ = std::move(rhs.symbols_);
    diagnostics_ = std::move(rhs.diagnostics_);

    return *this;
}

bool Builder::IsValid() const {
    return !diagnostics_.ContainsErrors();
}

void Builder::MarkAsMoved() {
    AssertNotMoved();
    moved_ = true;
}

void Builder::AssertNotMoved() const {
    TINT_ASSERT(!moved_) << "Attempting to use Builder after it has been moved";
}

const Statement* Builder::WrapInStatement(const Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), ast::Type{}, expr));
}

const VariableDeclStatement* Builder::WrapInStatement(const Variable* v) {
    return create<VariableDeclStatement>(v);
}

const Statement* Builder::WrapInStatement(const Statement* stmt) {
    return stmt;
}

const Function* Builder::WrapInFunction(VectorRef<const Statement*> stmts) {
    return Func("test_function", {}, ty.void_(), std::move(stmts),
                Vector{
                    create<StageAttribute>(PipelineStage::kCompute),
                    WorkgroupSize(1_u, 1_u, 1_u),
                });
}

Builder::TypesBuilder::TypesBuilder(Builder* pb) : builder(pb) {}

Type Builder::TypesBuilder::AsType(Symbol sym) const {
    return AsType(builder->source_, sym);
}

Type Builder::TypesBuilder::AsType(const Source& source, Symbol sym) const {
    return {builder->Expr(builder->Ident(source, sym))};
}

Type Builder::TypesBuilder::AsType(std::string_view name) const {
    return AsType(builder->source_, name);
}

Type Builder::TypesBuilder::AsType(const Source& source, std::string_view name) const {
    return {builder->Expr(builder->Ident(source, name))};
}

Type Builder::TypesBuilder::AsType(const IdentifierExpression* ident) const {
    return {ident};
}

Type Builder::TypesBuilder::void_() const {
    return Type{};
}

Type Builder::TypesBuilder::bool_() const {
    return AsType("bool");
}

Type Builder::TypesBuilder::bool_(const Source& source) const {
    return AsType(source, "bool");
}

Type Builder::TypesBuilder::f16() const {
    return AsType("f16");
}

Type Builder::TypesBuilder::f16(const Source& source) const {
    return AsType(source, "f16");
}

Type Builder::TypesBuilder::f32() const {
    return AsType("f32");
}

Type Builder::TypesBuilder::f32(const Source& source) const {
    return AsType(source, "f32");
}

Type Builder::TypesBuilder::i32() const {
    return AsType("i32");
}

Type Builder::TypesBuilder::i32(const Source& source) const {
    return AsType(source, "i32");
}

Type Builder::TypesBuilder::u32() const {
    return AsType("u32");
}

Type Builder::TypesBuilder::u32(const Source& source) const {
    return AsType(source, "u32");
}

Type Builder::TypesBuilder::i8() const {
    return AsType("i8");
}

Type Builder::TypesBuilder::i8(const Source& source) const {
    return AsType(source, "i8");
}

Type Builder::TypesBuilder::u8() const {
    return AsType("u8");
}

Type Builder::TypesBuilder::u8(const Source& source) const {
    return AsType(source, "u8");
}

Type Builder::TypesBuilder::vec(Type type, uint32_t n) const {
    return vec(builder->source_, type, n);
}

Type Builder::TypesBuilder::vec(const Source& source, Type type, uint32_t n) const {
    switch (n) {
        case 2:
            return vec2(source, type);
        case 3:
            return vec3(source, type);
        case 4:
            return vec4(source, type);
    }
    TINT_ICE() << "invalid vector width " << n;
}

Type Builder::TypesBuilder::vec2(Type type) const {
    return vec2(builder->source_, type);
}

Type Builder::TypesBuilder::vec2(const Source& source, Type type) const {
    return AsType(source, "vec2", type);
}

Type Builder::TypesBuilder::vec3(Type type) const {
    return vec3(builder->source_, type);
}

Type Builder::TypesBuilder::vec3(const Source& source, Type type) const {
    return AsType(source, "vec3", type);
}

Type Builder::TypesBuilder::vec4(Type type) const {
    return vec4(builder->source_, type);
}

Type Builder::TypesBuilder::vec4(const Source& source, Type type) const {
    return AsType(source, "vec4", type);
}

Type Builder::TypesBuilder::mat(Type type, uint32_t columns, uint32_t rows) const {
    return mat(builder->source_, type, columns, rows);
}

Type Builder::TypesBuilder::mat(const Source& source,
                                Type type,
                                uint32_t columns,
                                uint32_t rows) const {
    if (DAWN_LIKELY(columns >= 2 && columns <= 4 && rows >= 2 && rows <= 4)) {
        static constexpr std::array<const char*, 9> names = {
            "mat2x2", "mat2x3", "mat2x4",  //
            "mat3x2", "mat3x3", "mat3x4",  //
            "mat4x2", "mat4x3", "mat4x4",  //
        };
        auto i = (columns - 2) * 3 + (rows - 2);
        return AsType(source, names[i], type);
    }
    TINT_ICE() << "invalid matrix dimensions " << columns << "x" << rows;
}

Type Builder::TypesBuilder::mat2x2(Type type) const {
    return mat2x2(builder->source_, type);
}

Type Builder::TypesBuilder::mat2x3(Type type) const {
    return mat2x3(builder->source_, type);
}

Type Builder::TypesBuilder::mat2x4(Type type) const {
    return mat2x4(builder->source_, type);
}

Type Builder::TypesBuilder::mat3x2(Type type) const {
    return mat3x2(builder->source_, type);
}

Type Builder::TypesBuilder::mat3x3(Type type) const {
    return mat3x3(builder->source_, type);
}

Type Builder::TypesBuilder::mat3x4(Type type) const {
    return mat3x4(builder->source_, type);
}

Type Builder::TypesBuilder::mat4x2(Type type) const {
    return mat4x2(builder->source_, type);
}

Type Builder::TypesBuilder::mat4x3(Type type) const {
    return mat4x3(builder->source_, type);
}

Type Builder::TypesBuilder::mat4x4(Type type) const {
    return mat4x4(builder->source_, type);
}

Type Builder::TypesBuilder::mat2x2(const Source& source) const {
    return AsType(source, "mat2x2");
}

Type Builder::TypesBuilder::mat2x2(const Source& source, Type type) const {
    return AsType(source, "mat2x2", type);
}

Type Builder::TypesBuilder::mat2x3(const Source& source) const {
    return AsType(source, "mat2x3");
}

Type Builder::TypesBuilder::mat2x3(const Source& source, Type type) const {
    return AsType(source, "mat2x3", type);
}

Type Builder::TypesBuilder::mat2x4(const Source& source) const {
    return AsType(source, "mat2x4");
}

Type Builder::TypesBuilder::mat2x4(const Source& source, Type type) const {
    return AsType(source, "mat2x4", type);
}

Type Builder::TypesBuilder::mat3x2(const Source& source) const {
    return AsType(source, "mat3x2");
}

Type Builder::TypesBuilder::mat3x2(const Source& source, Type type) const {
    return AsType(source, "mat3x2", type);
}

Type Builder::TypesBuilder::mat3x3(const Source& source) const {
    return AsType(source, "mat3x3");
}

Type Builder::TypesBuilder::mat3x3(const Source& source, Type type) const {
    return AsType(source, "mat3x3", type);
}

Type Builder::TypesBuilder::mat3x4(const Source& source) const {
    return AsType(source, "mat3x4");
}

Type Builder::TypesBuilder::mat3x4(const Source& source, Type type) const {
    return AsType(source, "mat3x4", type);
}

Type Builder::TypesBuilder::mat4x2(const Source& source) const {
    return AsType(source, "mat4x2");
}

Type Builder::TypesBuilder::mat4x2(const Source& source, Type type) const {
    return AsType(source, "mat4x2", type);
}

Type Builder::TypesBuilder::mat4x3(const Source& source) const {
    return AsType(source, "mat4x3");
}

Type Builder::TypesBuilder::mat4x3(const Source& source, Type type) const {
    return AsType(source, "mat4x3", type);
}

Type Builder::TypesBuilder::mat4x4(const Source& source) const {
    return AsType(source, "mat4x4");
}

Type Builder::TypesBuilder::mat4x4(const Source& source, Type type) const {
    return AsType(source, "mat4x4", type);
}

Type Builder::TypesBuilder::array(const Source& source) const {
    return AsType(source, "array");
}

Type Builder::TypesBuilder::array() const {
    return array(builder->source_);
}

Type Builder::TypesBuilder::array(Type subtype) const {
    return array(builder->source_, subtype);
}

Type Builder::TypesBuilder::array(const Source& source, Type subtype) const {
    return Type{builder->Expr(builder->create<TemplatedIdentifier>(source, builder->Sym("array"),
                                                                   Vector{
                                                                       subtype.expr,
                                                                   }))};
}

Type Builder::TypesBuilder::array(Type subtype, uint32_t n) const {
    return array(builder->source_, subtype, core::u32(n));
}

Type Builder::TypesBuilder::array(Type subtype, const ast::Const* expr) const {
    return array(builder->source_, subtype, expr);
}

Type Builder::TypesBuilder::array(Type subtype, const Expression* expr) const {
    return array(builder->source_, subtype, expr);
}

Type Builder::TypesBuilder::array(Type subtype, const ast::Override* expr) const {
    return array(builder->source_, subtype, expr);
}

Type Builder::TypesBuilder::array(const Source& source, Type subtype, uint32_t n) const {
    if (n == 0) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, core::u32(n));
}

Type Builder::TypesBuilder::array(const Source& source,
                                  Type subtype,
                                  const ast::Const* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

Type Builder::TypesBuilder::array(const Source& source,
                                  Type subtype,
                                  const Expression* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

Type Builder::TypesBuilder::array(const Source& source,
                                  Type subtype,
                                  const ast::Override* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

const ast::Alias* Builder::TypesBuilder::alias(std::string_view name, Type type) const {
    return alias(builder->source_, builder->Ident(name), type);
}

const ast::Alias* Builder::TypesBuilder::alias(Symbol name, Type type) const {
    return alias(builder->source_, builder->Ident(name), type);
}

const ast::Alias* Builder::TypesBuilder::alias(const Source& source,
                                               const Identifier* name,
                                               Type type) const {
    return builder->create<ast::Alias>(source, builder->Ident(name), type);
}

Type Builder::TypesBuilder::ptr(core::AddressSpace address_space,
                                Type type,
                                core::Access access) const {
    return ptr(builder->source_, address_space, type, access);
}

Type Builder::TypesBuilder::ptr(const Source& source,
                                core::AddressSpace address_space,
                                Type type,
                                core::Access access) const {
    if (access != core::Access::kUndefined) {
        return AsType(source, "ptr", address_space, type, access);
    }
    return AsType(source, "ptr", address_space, type);
}

Type Builder::TypesBuilder::atomic(const Source& source, Type type) const {
    return AsType(source, "atomic", type);
}

Type Builder::TypesBuilder::atomic(Type type) const {
    return AsType("atomic", type);
}

Type Builder::TypesBuilder::sampler(core::type::SamplerKind kind) const {
    return sampler(builder->source_, kind);
}

Type Builder::TypesBuilder::sampler(const Source& source, core::type::SamplerKind kind) const {
    switch (kind) {
        case core::type::SamplerKind::kSampler:
            return AsType(source, "sampler");
        case core::type::SamplerKind::kComparisonSampler:
            return AsType(source, "sampler_comparison");
    }
    TINT_ICE() << "invalid sampler kind " << kind;
}

Type Builder::TypesBuilder::depth_texture(core::type::TextureDimension dims) const {
    return depth_texture(builder->source_, dims);
}

Type Builder::TypesBuilder::depth_texture(const Source& source,
                                          core::type::TextureDimension dims) const {
    switch (dims) {
        case core::type::TextureDimension::k2d:
            return AsType(source, "texture_depth_2d");
        case core::type::TextureDimension::k2dArray:
            return AsType(source, "texture_depth_2d_array");
        case core::type::TextureDimension::kCube:
            return AsType(source, "texture_depth_cube");
        case core::type::TextureDimension::kCubeArray:
            return AsType(source, "texture_depth_cube_array");
        default:
            break;
    }
    TINT_ICE() << "invalid depth_texture dimensions: " << dims;
}

Type Builder::TypesBuilder::depth_multisampled_texture(core::type::TextureDimension dims) const {
    return depth_multisampled_texture(builder->source_, dims);
}

Type Builder::TypesBuilder::depth_multisampled_texture(const Source& source,
                                                       core::type::TextureDimension dims) const {
    if (dims == core::type::TextureDimension::k2d) {
        return AsType(source, "texture_depth_multisampled_2d");
    }
    TINT_ICE() << "invalid depth_multisampled_texture dimensions: " << dims;
}

Type Builder::TypesBuilder::sampled_texture(core::type::TextureDimension dims, Type subtype) const {
    return sampled_texture(builder->source_, dims, subtype);
}

Type Builder::TypesBuilder::sampled_texture(const Source& source,
                                            core::type::TextureDimension dims,
                                            Type subtype) const {
    switch (dims) {
        case core::type::TextureDimension::k1d:
            return AsType(source, "texture_1d", subtype);
        case core::type::TextureDimension::k2d:
            return AsType(source, "texture_2d", subtype);
        case core::type::TextureDimension::k3d:
            return AsType(source, "texture_3d", subtype);
        case core::type::TextureDimension::k2dArray:
            return AsType(source, "texture_2d_array", subtype);
        case core::type::TextureDimension::kCube:
            return AsType(source, "texture_cube", subtype);
        case core::type::TextureDimension::kCubeArray:
            return AsType(source, "texture_cube_array", subtype);
        default:
            break;
    }
    TINT_ICE() << "invalid sampled_texture dimensions: " << dims;
}

Type Builder::TypesBuilder::multisampled_texture(core::type::TextureDimension dims,
                                                 Type subtype) const {
    return multisampled_texture(builder->source_, dims, subtype);
}

Type Builder::TypesBuilder::multisampled_texture(const Source& source,
                                                 core::type::TextureDimension dims,
                                                 Type subtype) const {
    if (dims == core::type::TextureDimension::k2d) {
        return AsType(source, "texture_multisampled_2d", subtype);
    }
    TINT_ICE() << "invalid multisampled_texture dimensions: " << dims;
}

Type Builder::TypesBuilder::storage_texture(core::type::TextureDimension dims,
                                            core::TexelFormat format,
                                            core::Access access) const {
    return storage_texture(builder->source_, dims, format, access);
}

Type Builder::TypesBuilder::storage_texture(const Source& source,
                                            core::type::TextureDimension dims,
                                            core::TexelFormat format,
                                            core::Access access) const {
    switch (dims) {
        case core::type::TextureDimension::k1d:
            return AsType(source, "texture_storage_1d", format, access);
        case core::type::TextureDimension::k2d:
            return AsType(source, "texture_storage_2d", format, access);
        case core::type::TextureDimension::k2dArray:
            return AsType(source, "texture_storage_2d_array", format, access);
        case core::type::TextureDimension::k3d:
            return AsType(source, "texture_storage_3d", format, access);
        default:
            break;
    }
    TINT_ICE() << "invalid storage_texture  dimensions: " << dims;
}

Type Builder::TypesBuilder::texel_buffer(core::TexelFormat format, core::Access access) const {
    return texel_buffer(builder->source_, format, access);
}

Type Builder::TypesBuilder::texel_buffer(const Source& source,
                                         core::TexelFormat format,
                                         core::Access access) const {
    return AsType(source, "texel_buffer", format, access);
}

Type Builder::TypesBuilder::input_attachment(Type subtype) const {
    return AsType("input_attachment", subtype);
}

Type Builder::TypesBuilder::external_texture(const Source& source) const {
    return AsType(source, "texture_external");
}

Type Builder::TypesBuilder::external_texture() const {
    return external_texture(builder->source_);
}

Type Builder::TypesBuilder::subgroup_matrix_result(Type el, uint32_t cols, uint32_t rows) const {
    return subgroup_matrix_result(builder->source_, el, cols, rows);
}

Type Builder::TypesBuilder::subgroup_matrix_result(const Source& source,
                                                   Type el,
                                                   uint32_t cols,
                                                   uint32_t rows) const {
    return subgroup_matrix_result(source, el, core::AInt(cols), core::AInt(rows));
}

Type Builder::TypesBuilder::subgroup_matrix_right(Type el, uint32_t cols, uint32_t rows) const {
    return AsType("subgroup_matrix_right", el, core::AInt(cols), core::AInt(rows));
}

Type Builder::TypesBuilder::subgroup_matrix_left(Type el, uint32_t cols, uint32_t rows) const {
    return AsType("subgroup_matrix_left", el, core::AInt(cols), core::AInt(rows));
}

Type Builder::TypesBuilder::subgroup_matrix(core::SubgroupMatrixKind kind,
                                            Type el,
                                            uint32_t cols,
                                            uint32_t rows) const {
    switch (kind) {
        case core::SubgroupMatrixKind::kLeft:
            return subgroup_matrix_left(el, cols, rows);
        case core::SubgroupMatrixKind::kRight:
            return subgroup_matrix_right(el, cols, rows);
        case core::SubgroupMatrixKind::kResult:
            return subgroup_matrix_result(el, cols, rows);
        case core::SubgroupMatrixKind::kUndefined:
            TINT_UNREACHABLE();
    }
    TINT_UNREACHABLE();
}

Type Builder::TypesBuilder::buffer(uint32_t size) const {
    if (size == 0) {
        return AsType("buffer");
    }
    return buffer(core::AInt(size));
}

Type Builder::TypesBuilder::Of(const TypeDecl* type) const {
    return AsType(type->name->symbol);
}

}  // namespace tint::ast
