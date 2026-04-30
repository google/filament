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
Builder::LetOptions::~LetOptions() = default;
Builder::ConstOptions::~ConstOptions() = default;
Builder::OverrideOptions::~OverrideOptions() = default;

Builder::Builder() : ast_(ast_nodes_.Create<ast::Module>(AllocateNodeID(), Source{})) {}

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

const ast::Statement* Builder::WrapInStatement(const ast::Expression* expr) {
    // Create a temporary variable of inferred type from expr.
    return Decl(Let(symbols_.New(), expr));
}

const ast::VariableDeclStatement* Builder::WrapInStatement(const ast::Variable* v) {
    return create<ast::VariableDeclStatement>(v);
}

const ast::Statement* Builder::WrapInStatement(const ast::Statement* stmt) {
    return stmt;
}

const ast::Function* Builder::WrapInFunction(VectorRef<const ast::Statement*> stmts) {
    return Func("test_function", {}, ty.void_(), std::move(stmts),
                Vector{
                    create<ast::StageAttribute>(ast::PipelineStage::kCompute),
                    WorkgroupSize(1_u, 1_u, 1_u),
                });
}

Builder::TypesBuilder::TypesBuilder(Builder* pb) : builder(pb) {}

ast::Type Builder::TypesBuilder::AsType(Symbol sym) const {
    return AsType(builder->source_, sym);
}

ast::Type Builder::TypesBuilder::AsType(const Source& source, Symbol sym) const {
    return {builder->Expr(builder->Ident(source, sym))};
}

ast::Type Builder::TypesBuilder::AsType(std::string_view name) const {
    return AsType(builder->source_, name);
}

ast::Type Builder::TypesBuilder::AsType(const Source& source, std::string_view name) const {
    return {builder->Expr(builder->Ident(source, name))};
}

ast::Type Builder::TypesBuilder::AsType(const ast::IdentifierExpression* ident) const {
    return {ident};
}

ast::Type Builder::TypesBuilder::void_() const {
    return ast::Type{};
}

ast::Type Builder::TypesBuilder::bool_() const {
    return AsType("bool");
}

ast::Type Builder::TypesBuilder::bool_(const Source& source) const {
    return AsType(source, "bool");
}

ast::Type Builder::TypesBuilder::f16() const {
    return AsType("f16");
}

ast::Type Builder::TypesBuilder::f16(const Source& source) const {
    return AsType(source, "f16");
}

ast::Type Builder::TypesBuilder::f32() const {
    return AsType("f32");
}

ast::Type Builder::TypesBuilder::f32(const Source& source) const {
    return AsType(source, "f32");
}

ast::Type Builder::TypesBuilder::i32() const {
    return AsType("i32");
}

ast::Type Builder::TypesBuilder::i32(const Source& source) const {
    return AsType(source, "i32");
}

ast::Type Builder::TypesBuilder::u32() const {
    return AsType("u32");
}

ast::Type Builder::TypesBuilder::u32(const Source& source) const {
    return AsType(source, "u32");
}

ast::Type Builder::TypesBuilder::i8() const {
    return AsType("i8");
}

ast::Type Builder::TypesBuilder::i8(const Source& source) const {
    return AsType(source, "i8");
}

ast::Type Builder::TypesBuilder::u8() const {
    return AsType("u8");
}

ast::Type Builder::TypesBuilder::u8(const Source& source) const {
    return AsType(source, "u8");
}

ast::Type Builder::TypesBuilder::vec(ast::Type type, uint32_t n) const {
    return vec(builder->source_, type, n);
}

ast::Type Builder::TypesBuilder::vec(const Source& source, ast::Type type, uint32_t n) const {
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

ast::Type Builder::TypesBuilder::vec2(ast::Type type) const {
    return vec2(builder->source_, type);
}

ast::Type Builder::TypesBuilder::vec2(const Source& source, ast::Type type) const {
    return AsType(source, "vec2", type);
}

ast::Type Builder::TypesBuilder::vec3(ast::Type type) const {
    return vec3(builder->source_, type);
}

ast::Type Builder::TypesBuilder::vec3(const Source& source, ast::Type type) const {
    return AsType(source, "vec3", type);
}

ast::Type Builder::TypesBuilder::vec4(ast::Type type) const {
    return vec4(builder->source_, type);
}

ast::Type Builder::TypesBuilder::vec4(const Source& source, ast::Type type) const {
    return AsType(source, "vec4", type);
}

ast::Type Builder::TypesBuilder::mat(ast::Type type, uint32_t columns, uint32_t rows) const {
    return mat(builder->source_, type, columns, rows);
}

ast::Type Builder::TypesBuilder::mat(const Source& source,
                                     ast::Type type,
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

ast::Type Builder::TypesBuilder::mat2x2(ast::Type type) const {
    return mat2x2(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat2x3(ast::Type type) const {
    return mat2x3(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat2x4(ast::Type type) const {
    return mat2x4(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat3x2(ast::Type type) const {
    return mat3x2(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat3x3(ast::Type type) const {
    return mat3x3(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat3x4(ast::Type type) const {
    return mat3x4(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat4x2(ast::Type type) const {
    return mat4x2(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat4x3(ast::Type type) const {
    return mat4x3(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat4x4(ast::Type type) const {
    return mat4x4(builder->source_, type);
}

ast::Type Builder::TypesBuilder::mat2x2(const Source& source) const {
    return AsType(source, "mat2x2");
}

ast::Type Builder::TypesBuilder::mat2x2(const Source& source, ast::Type type) const {
    return AsType(source, "mat2x2", type);
}

ast::Type Builder::TypesBuilder::mat2x3(const Source& source) const {
    return AsType(source, "mat2x3");
}

ast::Type Builder::TypesBuilder::mat2x3(const Source& source, ast::Type type) const {
    return AsType(source, "mat2x3", type);
}

ast::Type Builder::TypesBuilder::mat2x4(const Source& source) const {
    return AsType(source, "mat2x4");
}

ast::Type Builder::TypesBuilder::mat2x4(const Source& source, ast::Type type) const {
    return AsType(source, "mat2x4", type);
}

ast::Type Builder::TypesBuilder::mat3x2(const Source& source) const {
    return AsType(source, "mat3x2");
}

ast::Type Builder::TypesBuilder::mat3x2(const Source& source, ast::Type type) const {
    return AsType(source, "mat3x2", type);
}

ast::Type Builder::TypesBuilder::mat3x3(const Source& source) const {
    return AsType(source, "mat3x3");
}

ast::Type Builder::TypesBuilder::mat3x3(const Source& source, ast::Type type) const {
    return AsType(source, "mat3x3", type);
}

ast::Type Builder::TypesBuilder::mat3x4(const Source& source) const {
    return AsType(source, "mat3x4");
}

ast::Type Builder::TypesBuilder::mat3x4(const Source& source, ast::Type type) const {
    return AsType(source, "mat3x4", type);
}

ast::Type Builder::TypesBuilder::mat4x2(const Source& source) const {
    return AsType(source, "mat4x2");
}

ast::Type Builder::TypesBuilder::mat4x2(const Source& source, ast::Type type) const {
    return AsType(source, "mat4x2", type);
}

ast::Type Builder::TypesBuilder::mat4x3(const Source& source) const {
    return AsType(source, "mat4x3");
}

ast::Type Builder::TypesBuilder::mat4x3(const Source& source, ast::Type type) const {
    return AsType(source, "mat4x3", type);
}

ast::Type Builder::TypesBuilder::mat4x4(const Source& source) const {
    return AsType(source, "mat4x4");
}

ast::Type Builder::TypesBuilder::mat4x4(const Source& source, ast::Type type) const {
    return AsType(source, "mat4x4", type);
}

ast::Type Builder::TypesBuilder::array(const Source& source) const {
    return AsType(source, "array");
}

ast::Type Builder::TypesBuilder::array() const {
    return array(builder->source_);
}

ast::Type Builder::TypesBuilder::array(ast::Type subtype) const {
    return array(builder->source_, subtype);
}

ast::Type Builder::TypesBuilder::array(const Source& source, ast::Type subtype) const {
    return ast::Type{
        builder->Expr(builder->create<ast::TemplatedIdentifier>(source, builder->Sym("array"),
                                                                Vector{
                                                                    subtype.expr,
                                                                }))};
}

ast::Type Builder::TypesBuilder::array(ast::Type subtype, uint32_t n) const {
    return array(builder->source_, subtype, core::u32(n));
}

ast::Type Builder::TypesBuilder::array(ast::Type subtype, const ast::Const* expr) const {
    return array(builder->source_, subtype, expr);
}

ast::Type Builder::TypesBuilder::array(ast::Type subtype, const ast::Expression* expr) const {
    return array(builder->source_, subtype, expr);
}

ast::Type Builder::TypesBuilder::array(ast::Type subtype, const ast::Override* expr) const {
    return array(builder->source_, subtype, expr);
}

ast::Type Builder::TypesBuilder::array(const Source& source, ast::Type subtype, uint32_t n) const {
    if (n == 0) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, core::u32(n));
}

ast::Type Builder::TypesBuilder::array(const Source& source,
                                       ast::Type subtype,
                                       const ast::Const* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

ast::Type Builder::TypesBuilder::array(const Source& source,
                                       ast::Type subtype,
                                       const ast::Expression* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

ast::Type Builder::TypesBuilder::array(const Source& source,
                                       ast::Type subtype,
                                       const ast::Override* expr) const {
    if (expr == nullptr) {
        return AsType(source, "array", subtype);
    }
    return AsType(source, "array", subtype, expr);
}

const ast::Alias* Builder::TypesBuilder::alias(std::string_view name, ast::Type type) const {
    return alias(builder->source_, builder->Ident(name), type);
}

const ast::Alias* Builder::TypesBuilder::alias(Symbol name, ast::Type type) const {
    return alias(builder->source_, builder->Ident(name), type);
}

const ast::Alias* Builder::TypesBuilder::alias(const Source& source,
                                               const ast::Identifier* name,
                                               ast::Type type) const {
    return builder->create<ast::Alias>(source, builder->Ident(name), type);
}

ast::Type Builder::TypesBuilder::ptr(core::AddressSpace address_space,
                                     ast::Type type,
                                     core::Access access) const {
    return ptr(builder->source_, address_space, type, access);
}

ast::Type Builder::TypesBuilder::ptr(const Source& source,
                                     core::AddressSpace address_space,
                                     ast::Type type,
                                     core::Access access) const {
    if (access != core::Access::kUndefined) {
        return AsType(source, "ptr", address_space, type, access);
    }
    return AsType(source, "ptr", address_space, type);
}

ast::Type Builder::TypesBuilder::atomic(const Source& source, ast::Type type) const {
    return AsType(source, "atomic", type);
}

ast::Type Builder::TypesBuilder::atomic(ast::Type type) const {
    return AsType("atomic", type);
}

ast::Type Builder::TypesBuilder::sampler(core::type::SamplerKind kind) const {
    return sampler(builder->source_, kind);
}

ast::Type Builder::TypesBuilder::sampler(core::SamplerFiltering filtering) const {
    return sampler(builder->source_, filtering);
}

ast::Type Builder::TypesBuilder::sampler(const Source& source, core::type::SamplerKind kind) const {
    switch (kind) {
        case core::type::SamplerKind::kSampler:
            return AsType(source, "sampler");
        case core::type::SamplerKind::kComparisonSampler:
            return AsType(source, "sampler_comparison");
    }
    TINT_ICE() << "invalid sampler kind " << kind;
}

ast::Type Builder::TypesBuilder::sampler(const Source& source,
                                         core::SamplerFiltering filtering) const {
    TINT_ASSERT(filtering == core::SamplerFiltering::kFiltering ||
                filtering == core::SamplerFiltering::kNonFiltering);

    return AsType(source, "sampler", filtering);
}

ast::Type Builder::TypesBuilder::depth_texture(core::type::TextureDimension dims) const {
    return depth_texture(builder->source_, dims);
}

ast::Type Builder::TypesBuilder::depth_texture(const Source& source,
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

ast::Type Builder::TypesBuilder::depth_multisampled_texture(
    core::type::TextureDimension dims) const {
    return depth_multisampled_texture(builder->source_, dims);
}

ast::Type Builder::TypesBuilder::depth_multisampled_texture(
    const Source& source,
    core::type::TextureDimension dims) const {
    if (dims == core::type::TextureDimension::k2d) {
        return AsType(source, "texture_depth_multisampled_2d");
    }
    TINT_ICE() << "invalid depth_multisampled_texture dimensions: " << dims;
}

ast::Type Builder::TypesBuilder::sampled_texture(core::type::TextureDimension dims,
                                                 ast::Type subtype) const {
    return sampled_texture(builder->source_, dims, subtype);
}

ast::Type Builder::TypesBuilder::sampled_texture(const Source& source,
                                                 core::type::TextureDimension dims,
                                                 ast::Type subtype) const {
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

ast::Type Builder::TypesBuilder::sampled_texture(core::type::TextureDimension dims,
                                                 ast::Type subtype,
                                                 core::TextureFilterable filterable) const {
    return sampled_texture(builder->source_, dims, subtype, filterable);
}

ast::Type Builder::TypesBuilder::sampled_texture(const Source& source,
                                                 core::type::TextureDimension dims,
                                                 ast::Type subtype,
                                                 core::TextureFilterable filterable) const {
    TINT_ASSERT(filterable == core::TextureFilterable::kFilterable ||
                filterable == core::TextureFilterable::kUnfilterable);

    std::string_view name;
    switch (dims) {
        case core::type::TextureDimension::k1d:
            name = "texture_1d";
            break;
        case core::type::TextureDimension::k2d:
            name = "texture_2d";
            break;
        case core::type::TextureDimension::k3d:
            name = "texture_3d";
            break;
        case core::type::TextureDimension::k2dArray:
            name = "texture_2d_array";
            break;
        case core::type::TextureDimension::kCube:
            name = "texture_cube";
            break;
        case core::type::TextureDimension::kCubeArray:
            name = "texture_cube_array";
            break;
        default:
            TINT_ICE() << "invalid sampled_texture dimensions: " << dims;
    }

    return ast::Type{builder->Expr(
        builder->create<ast::TemplatedIdentifier>(source, builder->Sym(name),
                                                  Vector{
                                                      subtype.expr,
                                                      builder->Expr(builder->Ident(filterable)),
                                                  }))};
}

ast::Type Builder::TypesBuilder::multisampled_texture(core::type::TextureDimension dims,
                                                      ast::Type subtype) const {
    return multisampled_texture(builder->source_, dims, subtype);
}

ast::Type Builder::TypesBuilder::multisampled_texture(const Source& source,
                                                      core::type::TextureDimension dims,
                                                      ast::Type subtype) const {
    if (dims == core::type::TextureDimension::k2d) {
        return AsType(source, "texture_multisampled_2d", subtype);
    }
    TINT_ICE() << "invalid multisampled_texture dimensions: " << dims;
}

ast::Type Builder::TypesBuilder::storage_texture(core::type::TextureDimension dims,
                                                 core::TexelFormat format,
                                                 core::Access access) const {
    return storage_texture(builder->source_, dims, format, access);
}

ast::Type Builder::TypesBuilder::storage_texture(const Source& source,
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

ast::Type Builder::TypesBuilder::texel_buffer(core::TexelFormat format, core::Access access) const {
    return texel_buffer(builder->source_, format, access);
}

ast::Type Builder::TypesBuilder::texel_buffer(const Source& source,
                                              core::TexelFormat format,
                                              core::Access access) const {
    return AsType(source, "texel_buffer", format, access);
}

ast::Type Builder::TypesBuilder::input_attachment(ast::Type subtype) const {
    return AsType("input_attachment", subtype);
}

ast::Type Builder::TypesBuilder::external_texture(const Source& source) const {
    return AsType(source, "texture_external");
}

ast::Type Builder::TypesBuilder::external_texture() const {
    return external_texture(builder->source_);
}

ast::Type Builder::TypesBuilder::subgroup_matrix_result(ast::Type el,
                                                        uint32_t cols,
                                                        uint32_t rows) const {
    return subgroup_matrix_result(builder->source_, el, cols, rows);
}

ast::Type Builder::TypesBuilder::subgroup_matrix_result(const Source& source,
                                                        ast::Type el,
                                                        uint32_t cols,
                                                        uint32_t rows) const {
    return subgroup_matrix_result(source, el, core::AInt(cols), core::AInt(rows));
}

ast::Type Builder::TypesBuilder::subgroup_matrix_right(ast::Type el,
                                                       uint32_t cols,
                                                       uint32_t rows) const {
    return AsType("subgroup_matrix_right", el, core::AInt(cols), core::AInt(rows));
}

ast::Type Builder::TypesBuilder::subgroup_matrix_left(ast::Type el,
                                                      uint32_t cols,
                                                      uint32_t rows) const {
    return AsType("subgroup_matrix_left", el, core::AInt(cols), core::AInt(rows));
}

ast::Type Builder::TypesBuilder::subgroup_matrix(core::SubgroupMatrixKind kind,
                                                 ast::Type el,
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
}

ast::Type Builder::TypesBuilder::buffer(uint32_t size) const {
    if (size == 0) {
        return AsType("buffer");
    }
    return buffer(core::AInt(size));
}

ast::Type Builder::TypesBuilder::Of(const ast::TypeDecl* type) const {
    return AsType(type->name->symbol);
}

}  // namespace tint::ast
