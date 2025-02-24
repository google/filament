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

#include "src/tint/lang/hlsl/writer/ast_raise/decompose_memory_access.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/unary_op.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::DecomposeMemoryAccess);
TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::DecomposeMemoryAccess::Intrinsic);

namespace tint::hlsl::writer {

namespace {

bool ShouldRun(const Program& program) {
    for (auto* decl : program.AST().GlobalDeclarations()) {
        if (auto* var = program.Sem().Get<sem::Variable>(decl)) {
            if (var->AddressSpace() == core::AddressSpace::kStorage ||
                var->AddressSpace() == core::AddressSpace::kUniform) {
                return true;
            }
        }
    }
    return false;
}

/// Offset is a simple Expression builder interface, used to build byte
/// offsets for storage and uniform buffer accesses.
struct Offset : Castable<Offset> {
    /// @returns builds and returns the Expression in `ctx.dst`
    virtual const ast::Expression* Build(program::CloneContext& ctx) const = 0;
};

/// OffsetExpr is an implementation of Offset that clones and casts the given
/// expression to `u32`.
struct OffsetExpr : Offset {
    const ast::Expression* const expr = nullptr;

    explicit OffsetExpr(const ast::Expression* e) : expr(e) {}

    const ast::Expression* Build(program::CloneContext& ctx) const override {
        auto* type = ctx.src->Sem().GetVal(expr)->Type()->UnwrapRef();
        auto* res = ctx.Clone(expr);
        if (!type->Is<core::type::U32>()) {
            res = ctx.dst->Call<u32>(res);
        }
        return res;
    }
};

/// OffsetLiteral is an implementation of Offset that constructs a u32 literal
/// value.
struct OffsetLiteral final : Castable<OffsetLiteral, Offset> {
    uint32_t const literal = 0;

    explicit OffsetLiteral(uint32_t lit) : literal(lit) {}

    const ast::Expression* Build(program::CloneContext& ctx) const override {
        return ctx.dst->Expr(u32(literal));
    }
};

/// OffsetBinOp is an implementation of Offset that constructs a binary-op of
/// two Offsets.
struct OffsetBinOp : Offset {
    core::BinaryOp op;
    Offset const* lhs = nullptr;
    Offset const* rhs = nullptr;

    const ast::Expression* Build(program::CloneContext& ctx) const override {
        return ctx.dst->create<ast::BinaryExpression>(op, lhs->Build(ctx), rhs->Build(ctx));
    }
};

/// LoadStoreKey is the unordered map key to a load or store intrinsic.
struct LoadStoreKey {
    core::type::Type const* el_ty = nullptr;  // element type
    Symbol const buffer;                      // buffer name
    bool operator==(const LoadStoreKey& rhs) const {
        return el_ty == rhs.el_ty && buffer == rhs.buffer;
    }
    struct Hasher {
        inline std::size_t operator()(const LoadStoreKey& u) const {
            return Hash(u.el_ty, u.buffer);
        }
    };
};

/// AtomicKey is the unordered map key to an atomic intrinsic.
struct AtomicKey {
    core::type::Type const* el_ty = nullptr;  // element type
    wgsl::BuiltinFn const op;                 // atomic op
    Symbol const buffer;                      // buffer name
    bool operator==(const AtomicKey& rhs) const {
        return el_ty == rhs.el_ty && op == rhs.op && buffer == rhs.buffer;
    }
    struct Hasher {
        inline std::size_t operator()(const AtomicKey& u) const {
            return Hash(u.el_ty, u.op, u.buffer);
        }
    };
};

bool IntrinsicDataTypeFor(const core::type::Type* ty,
                          DecomposeMemoryAccess::Intrinsic::DataType& out) {
    if (ty->Is<core::type::I32>()) {
        out = DecomposeMemoryAccess::Intrinsic::DataType::kI32;
        return true;
    }
    if (ty->Is<core::type::U32>()) {
        out = DecomposeMemoryAccess::Intrinsic::DataType::kU32;
        return true;
    }
    if (ty->Is<core::type::F32>()) {
        out = DecomposeMemoryAccess::Intrinsic::DataType::kF32;
        return true;
    }
    if (ty->Is<core::type::F16>()) {
        out = DecomposeMemoryAccess::Intrinsic::DataType::kF16;
        return true;
    }
    if (auto* vec = ty->As<core::type::Vector>()) {
        switch (vec->Width()) {
            case 2:
                if (vec->Type()->Is<core::type::I32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2I32;
                    return true;
                }
                if (vec->Type()->Is<core::type::U32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2U32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2F32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F16>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec2F16;
                    return true;
                }
                break;
            case 3:
                if (vec->Type()->Is<core::type::I32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3I32;
                    return true;
                }
                if (vec->Type()->Is<core::type::U32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3U32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3F32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F16>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec3F16;
                    return true;
                }
                break;
            case 4:
                if (vec->Type()->Is<core::type::I32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4I32;
                    return true;
                }
                if (vec->Type()->Is<core::type::U32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4U32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F32>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4F32;
                    return true;
                }
                if (vec->Type()->Is<core::type::F16>()) {
                    out = DecomposeMemoryAccess::Intrinsic::DataType::kVec4F16;
                    return true;
                }
                break;
        }
        return false;
    }

    return false;
}

/// @returns a DecomposeMemoryAccess::Intrinsic attribute that can be applied to a stub function to
/// load the type @p ty from the uniform or storage buffer with name @p buffer.
DecomposeMemoryAccess::Intrinsic* IntrinsicLoadFor(ast::Builder* builder,
                                                   const core::type::Type* ty,
                                                   core::AddressSpace address_space,
                                                   const Symbol& buffer) {
    DecomposeMemoryAccess::Intrinsic::DataType type;
    if (!IntrinsicDataTypeFor(ty, type)) {
        return nullptr;
    }
    return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
        builder->ID(), builder->AllocateNodeID(), DecomposeMemoryAccess::Intrinsic::Op::kLoad, type,
        address_space, builder->Expr(buffer));
}

/// @returns a DecomposeMemoryAccess::Intrinsic attribute that can be applied to a stub function to
/// store the type @p ty to the storage buffer with name @p buffer.
DecomposeMemoryAccess::Intrinsic* IntrinsicStoreFor(ast::Builder* builder,
                                                    const core::type::Type* ty,
                                                    const Symbol& buffer) {
    DecomposeMemoryAccess::Intrinsic::DataType type;
    if (!IntrinsicDataTypeFor(ty, type)) {
        return nullptr;
    }
    return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
        builder->ID(), builder->AllocateNodeID(), DecomposeMemoryAccess::Intrinsic::Op::kStore,
        type, core::AddressSpace::kStorage, builder->Expr(buffer));
}

/// @returns a DecomposeMemoryAccess::Intrinsic attribute that can be applied to a stub function for
/// the atomic op and the type @p ty.
DecomposeMemoryAccess::Intrinsic* IntrinsicAtomicFor(ast::Builder* builder,
                                                     wgsl::BuiltinFn ity,
                                                     const core::type::Type* ty,
                                                     const Symbol& buffer) {
    auto op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicLoad;
    switch (ity) {
        case wgsl::BuiltinFn::kAtomicLoad:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicLoad;
            break;
        case wgsl::BuiltinFn::kAtomicStore:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicStore;
            break;
        case wgsl::BuiltinFn::kAtomicAdd:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicAdd;
            break;
        case wgsl::BuiltinFn::kAtomicSub:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicSub;
            break;
        case wgsl::BuiltinFn::kAtomicMax:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicMax;
            break;
        case wgsl::BuiltinFn::kAtomicMin:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicMin;
            break;
        case wgsl::BuiltinFn::kAtomicAnd:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicAnd;
            break;
        case wgsl::BuiltinFn::kAtomicOr:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicOr;
            break;
        case wgsl::BuiltinFn::kAtomicXor:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicXor;
            break;
        case wgsl::BuiltinFn::kAtomicExchange:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicExchange;
            break;
        case wgsl::BuiltinFn::kAtomicCompareExchangeWeak:
            op = DecomposeMemoryAccess::Intrinsic::Op::kAtomicCompareExchangeWeak;
            break;
        default:
            TINT_ICE() << "invalid IntrinsicType for DecomposeMemoryAccess::Intrinsic: "
                       << ty->TypeInfo().name;
    }

    DecomposeMemoryAccess::Intrinsic::DataType type;
    if (!IntrinsicDataTypeFor(ty, type)) {
        return nullptr;
    }
    return builder->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
        builder->ID(), builder->AllocateNodeID(), op, type, core::AddressSpace::kStorage,
        builder->Expr(buffer));
}

/// BufferAccess describes a single storage or uniform buffer access
struct BufferAccess {
    sem::GlobalVariable const* var = nullptr;       // Storage or uniform buffer variable
    Offset const* offset = nullptr;                 // The byte offset on var
    core::type::Type const* type = nullptr;         // The type of the access
    explicit operator bool() const { return var; }  // Returns true if valid
};

/// Store describes a single storage or uniform buffer write
struct Store {
    const ast::AssignmentStatement* assignment;  // The AST assignment statement
    BufferAccess target;                         // The target for the write
};

}  // namespace

/// PIMPL state for the transform
struct DecomposeMemoryAccess::State {
    /// The clone context
    program::CloneContext& ctx;
    /// Alias to `*ctx.dst`
    ast::Builder& b;
    /// Map of AST expression to storage or uniform buffer access
    /// This map has entries added when encountered, and removed when outer
    /// expressions chain the access.
    /// Subset of #expression_order, as expressions are not removed from
    /// #expression_order.
    std::unordered_map<const ast::Expression*, BufferAccess> accesses;
    /// The visited order of AST expressions (superset of #accesses)
    std::vector<const ast::Expression*> expression_order;
    /// [buffer-type, element-type] -> load function name
    std::unordered_map<LoadStoreKey, Symbol, LoadStoreKey::Hasher> load_funcs;
    /// [buffer-type, element-type] -> store function name
    std::unordered_map<LoadStoreKey, Symbol, LoadStoreKey::Hasher> store_funcs;
    /// [buffer-type, element-type, atomic-op] -> load function name
    std::unordered_map<AtomicKey, Symbol, AtomicKey::Hasher> atomic_funcs;
    /// List of storage or uniform buffer writes
    std::vector<Store> stores;
    /// Allocations for offsets
    BlockAllocator<Offset> offsets_;

    /// Constructor
    /// @param context the program::CloneContext
    explicit State(program::CloneContext& context) : ctx(context), b(*ctx.dst) {}

    /// @param offset the offset value to wrap in an Offset
    /// @returns an Offset for the given literal value
    const Offset* ToOffset(uint32_t offset) { return offsets_.Create<OffsetLiteral>(offset); }

    /// @param expr the expression to convert to an Offset
    /// @returns an Offset for the given Expression
    const Offset* ToOffset(const ast::Expression* expr) {
        if (auto* lit = expr->As<ast::IntLiteralExpression>()) {
            if (lit->value >= 0) {
                return offsets_.Create<OffsetLiteral>(static_cast<uint32_t>(lit->value));
            }
        }
        return offsets_.Create<OffsetExpr>(expr);
    }

    /// @param offset the Offset that is returned
    /// @returns the given offset (pass-through)
    const Offset* ToOffset(const Offset* offset) { return offset; }

    /// @param lhs_ the left-hand side of the add expression
    /// @param rhs_ the right-hand side of the add expression
    /// @return an Offset that is a sum of lhs and rhs, performing basic constant
    /// folding if possible
    template <typename LHS, typename RHS>
    const Offset* Add(LHS&& lhs_, RHS&& rhs_) {
        auto* lhs = ToOffset(std::forward<LHS>(lhs_));
        auto* rhs = ToOffset(std::forward<RHS>(rhs_));
        auto* lhs_lit = tint::As<OffsetLiteral>(lhs);
        auto* rhs_lit = tint::As<OffsetLiteral>(rhs);
        if (lhs_lit && lhs_lit->literal == 0) {
            return rhs;
        }
        if (rhs_lit && rhs_lit->literal == 0) {
            return lhs;
        }
        if (lhs_lit && rhs_lit) {
            if (static_cast<uint64_t>(lhs_lit->literal) + static_cast<uint64_t>(rhs_lit->literal) <=
                0xffffffff) {
                return offsets_.Create<OffsetLiteral>(lhs_lit->literal + rhs_lit->literal);
            }
        }
        auto* out = offsets_.Create<OffsetBinOp>();
        out->op = core::BinaryOp::kAdd;
        out->lhs = lhs;
        out->rhs = rhs;
        return out;
    }

    /// @param lhs_ the left-hand side of the multiply expression
    /// @param rhs_ the right-hand side of the multiply expression
    /// @return an Offset that is the multiplication of lhs and rhs, performing
    /// basic constant folding if possible
    template <typename LHS, typename RHS>
    const Offset* Mul(LHS&& lhs_, RHS&& rhs_) {
        auto* lhs = ToOffset(std::forward<LHS>(lhs_));
        auto* rhs = ToOffset(std::forward<RHS>(rhs_));
        auto* lhs_lit = tint::As<OffsetLiteral>(lhs);
        auto* rhs_lit = tint::As<OffsetLiteral>(rhs);
        if (lhs_lit && lhs_lit->literal == 0) {
            return offsets_.Create<OffsetLiteral>(0u);
        }
        if (rhs_lit && rhs_lit->literal == 0) {
            return offsets_.Create<OffsetLiteral>(0u);
        }
        if (lhs_lit && lhs_lit->literal == 1) {
            return rhs;
        }
        if (rhs_lit && rhs_lit->literal == 1) {
            return lhs;
        }
        if (lhs_lit && rhs_lit) {
            return offsets_.Create<OffsetLiteral>(lhs_lit->literal * rhs_lit->literal);
        }
        auto* out = offsets_.Create<OffsetBinOp>();
        out->op = core::BinaryOp::kMultiply;
        out->lhs = lhs;
        out->rhs = rhs;
        return out;
    }

    /// AddAccess() adds the `expr -> access` map item to #accesses, and `expr`
    /// to #expression_order.
    /// @param expr the expression that performs the access
    /// @param access the access
    void AddAccess(const ast::Expression* expr, const BufferAccess& access) {
        TINT_ASSERT(access.type);
        accesses.emplace(expr, access);
        expression_order.emplace_back(expr);
    }

    /// TakeAccess() removes the `node` item from #accesses (if it exists),
    /// returning the BufferAccess. If #accesses does not hold an item for
    /// `node`, an invalid BufferAccess is returned.
    /// @param node the expression that performed an access
    /// @return the BufferAccess for the given expression
    BufferAccess TakeAccess(const ast::Expression* node) {
        auto lhs_it = accesses.find(node);
        if (lhs_it == accesses.end()) {
            return {};
        }
        auto access = lhs_it->second;
        accesses.erase(node);
        return access;
    }

    /// LoadFunc() returns a symbol to an intrinsic function that loads an element of type @p el_ty
    /// from a storage or uniform buffer with name @p buffer.
    /// The emitted function has the signature:
    ///   `fn load(offset : u32) -> el_ty`
    /// @param el_ty the storage or uniform buffer element type
    /// @param address_space either kUniform or kStorage
    /// @param buffer the symbol of the storage or uniform buffer variable, owned by the target
    /// ProgramBuilder.
    /// @return the name of the function that performs the load
    Symbol LoadFunc(const core::type::Type* el_ty,
                    core::AddressSpace address_space,
                    const Symbol& buffer) {
        return tint::GetOrAdd(load_funcs, LoadStoreKey{el_ty, buffer}, [&] {
            Vector params{b.Param("offset", b.ty.u32())};

            auto name = b.Symbols().New(buffer.Name() + "_load");

            if (auto* intrinsic = IntrinsicLoadFor(ctx.dst, el_ty, address_space, buffer)) {
                auto el_ast_ty = CreateASTTypeFor(ctx, el_ty);
                b.Func(name, params, el_ast_ty, nullptr,
                       Vector{
                           intrinsic,
                           b.Disable(ast::DisabledValidation::kFunctionHasNoBody),
                       });
            } else if (auto* arr_ty = el_ty->As<core::type::Array>()) {
                // fn load_func(buffer : buf_ty, offset : u32) -> array<T, N> {
                //   var arr : array<T, N>;
                //   for (var i = 0u; i < array_count; i = i + 1) {
                //     arr[i] = el_load_func(buffer, offset + i * array_stride)
                //   }
                //   return arr;
                // }
                auto load = LoadFunc(arr_ty->ElemType()->UnwrapRef(), address_space, buffer);
                auto* arr = b.Var(b.Symbols().New("arr"), CreateASTTypeFor(ctx, arr_ty));
                auto* i = b.Var(b.Symbols().New("i"), b.Expr(0_u));
                auto* for_init = b.Decl(i);
                auto arr_cnt = arr_ty->ConstantCount();
                if (DAWN_UNLIKELY(!arr_cnt)) {
                    // Non-constant counts should not be possible:
                    // * Override-expression counts can only be applied to workgroup arrays, and
                    //   this method only handles storage and uniform.
                    // * Runtime-sized arrays are not loadable.
                    TINT_ICE() << "unexpected non-constant array count";
                }
                auto* for_cond = b.create<ast::BinaryExpression>(
                    core::BinaryOp::kLessThan, b.Expr(i), b.Expr(u32(arr_cnt.value())));
                auto* for_cont = b.Assign(i, b.Add(i, 1_u));
                auto* arr_el = b.IndexAccessor(arr, i);
                auto* el_offset = b.Add(b.Expr("offset"), b.Mul(i, u32(arr_ty->Stride())));
                auto* el_val = b.Call(load, el_offset);
                auto* for_loop =
                    b.For(for_init, for_cond, for_cont, b.Block(b.Assign(arr_el, el_val)));

                b.Func(name, params, CreateASTTypeFor(ctx, arr_ty),
                       Vector{
                           b.Decl(arr),
                           for_loop,
                           b.Return(arr),
                       });
            } else {
                Vector<const ast::Expression*, 8> values;
                if (auto* mat_ty = el_ty->As<core::type::Matrix>()) {
                    auto* vec_ty = mat_ty->ColumnType();
                    Symbol load = LoadFunc(vec_ty, address_space, buffer);
                    for (uint32_t i = 0; i < mat_ty->Columns(); i++) {
                        auto* offset = b.Add("offset", u32(i * mat_ty->ColumnStride()));
                        values.Push(b.Call(load, offset));
                    }
                } else if (auto* str = el_ty->As<core::type::Struct>()) {
                    for (auto* member : str->Members()) {
                        auto* offset = b.Add("offset", u32(member->Offset()));
                        Symbol load = LoadFunc(member->Type()->UnwrapRef(), address_space, buffer);
                        values.Push(b.Call(load, offset));
                    }
                }
                b.Func(name, params, CreateASTTypeFor(ctx, el_ty),
                       Vector{
                           b.Return(b.Call(CreateASTTypeFor(ctx, el_ty), values)),
                       });
            }
            return name;
        });
    }

    /// StoreFunc() returns a symbol to an intrinsic function that stores an element of type @p
    /// el_ty to the storage buffer @p buffer. The function has the signature:
    ///   `fn store(offset : u32, value : el_ty)`
    /// @param el_ty the storage buffer element type
    /// @param buffer the symbol of the storage buffer variable, owned by the target ProgramBuilder.
    /// @return the name of the function that performs the store
    Symbol StoreFunc(const core::type::Type* el_ty, const Symbol& buffer) {
        return tint::GetOrAdd(store_funcs, LoadStoreKey{el_ty, buffer}, [&] {
            Vector params{
                b.Param("offset", b.ty.u32()),
                b.Param("value", CreateASTTypeFor(ctx, el_ty)),
            };

            auto name = b.Symbols().New(buffer.Name() + "_store");

            if (auto* intrinsic = IntrinsicStoreFor(ctx.dst, el_ty, buffer)) {
                b.Func(name, params, b.ty.void_(), nullptr,
                       Vector{
                           intrinsic,
                           b.Disable(ast::DisabledValidation::kFunctionHasNoBody),
                       });
            } else {
                auto body = Switch<Vector<const ast::Statement*, 8>>(
                    el_ty,  //
                    [&](const core::type::Array* arr_ty) {
                        // fn store_func(buffer : buf_ty, offset : u32, value : el_ty) {
                        //   var array = value; // No dynamic indexing on constant arrays
                        //   for (var i = 0u; i < array_count; i = i + 1) {
                        //     arr[i] = el_store_func(buffer, offset + i * array_stride,
                        //     value[i])
                        //   }
                        //   return arr;
                        // }
                        auto* array = b.Var(b.Symbols().New("array"), b.Expr("value"));
                        auto store = StoreFunc(arr_ty->ElemType()->UnwrapRef(), buffer);
                        auto* i = b.Var(b.Symbols().New("i"), b.Expr(0_u));
                        auto* for_init = b.Decl(i);
                        auto arr_cnt = arr_ty->ConstantCount();
                        if (DAWN_UNLIKELY(!arr_cnt)) {
                            // Non-constant counts should not be possible:
                            // * Override-expression counts can only be applied to workgroup
                            //   arrays, and this method only handles storage and uniform.
                            // * Runtime-sized arrays are not storable.
                            TINT_ICE() << "unexpected non-constant array count";
                        }
                        auto* for_cond = b.create<ast::BinaryExpression>(
                            core::BinaryOp::kLessThan, b.Expr(i), b.Expr(u32(arr_cnt.value())));
                        auto* for_cont = b.Assign(i, b.Add(i, 1_u));
                        auto* arr_el = b.IndexAccessor(array, i);
                        auto* el_offset = b.Add(b.Expr("offset"), b.Mul(i, u32(arr_ty->Stride())));
                        auto* store_stmt = b.CallStmt(b.Call(store, el_offset, arr_el));
                        auto* for_loop = b.For(for_init, for_cond, for_cont, b.Block(store_stmt));

                        return Vector{b.Decl(array), for_loop};
                    },
                    [&](const core::type::Matrix* mat_ty) {
                        auto* vec_ty = mat_ty->ColumnType();
                        Symbol store = StoreFunc(vec_ty, buffer);
                        Vector<const ast::Statement*, 4> stmts;
                        for (uint32_t i = 0; i < mat_ty->Columns(); i++) {
                            auto* offset = b.Add("offset", u32(i * mat_ty->ColumnStride()));
                            auto* element = b.IndexAccessor("value", u32(i));
                            auto* call = b.Call(store, offset, element);
                            stmts.Push(b.CallStmt(call));
                        }
                        return stmts;
                    },
                    [&](const core::type::Struct* str) {
                        Vector<const ast::Statement*, 8> stmts;
                        for (auto* member : str->Members()) {
                            auto* offset = b.Add("offset", u32(member->Offset()));
                            auto* element = b.MemberAccessor("value", ctx.Clone(member->Name()));
                            Symbol store = StoreFunc(member->Type()->UnwrapRef(), buffer);
                            auto* call = b.Call(store, offset, element);
                            stmts.Push(b.CallStmt(call));
                        }
                        return stmts;
                    });

                b.Func(name, params, b.ty.void_(), body);
            }

            return name;
        });
    }

    /// AtomicFunc() returns a symbol to an builtin function that performs an  atomic operation on
    /// the storage buffer @p buffer. The function has the signature:
    // `fn atomic_op(offset : u32, ...) -> T`
    /// @param el_ty the storage buffer element type
    /// @param builtin the atomic builtin
    /// @param buffer the symbol of the storage buffer variable, owned by the target ProgramBuilder.
    /// @return the name of the function that performs the load
    Symbol AtomicFunc(const core::type::Type* el_ty,
                      const sem::BuiltinFn* builtin,
                      const Symbol& buffer) {
        auto fn = builtin->Fn();
        return tint::GetOrAdd(atomic_funcs, AtomicKey{el_ty, fn, buffer}, [&] {
            // The first parameter to all WGSL atomics is the expression to the
            // atomic. This is replaced with two parameters: the buffer and offset.
            Vector params{b.Param("offset", b.ty.u32())};

            // Other parameters are copied as-is:
            for (size_t i = 1; i < builtin->Parameters().Length(); i++) {
                auto* param = builtin->Parameters()[i];
                auto ty = CreateASTTypeFor(ctx, param->Type());
                params.Push(b.Param("param_" + std::to_string(i), ty));
            }

            auto* atomic = IntrinsicAtomicFor(ctx.dst, fn, el_ty, buffer);
            if (DAWN_UNLIKELY(!atomic)) {
                TINT_ICE() << "IntrinsicAtomicFor() returned nullptr for fn " << fn << " and type "
                           << el_ty->TypeInfo().name;
            }

            ast::Type ret_ty = CreateASTTypeFor(ctx, builtin->ReturnType());

            auto name = b.Symbols().New(buffer.Name() + builtin->str());
            b.Func(name, std::move(params), ret_ty, nullptr,
                   Vector{
                       atomic,
                       b.Disable(ast::DisabledValidation::kFunctionHasNoBody),
                   });
            return name;
        });
    }
};

DecomposeMemoryAccess::Intrinsic::Intrinsic(GenerationID pid,
                                            ast::NodeID nid,
                                            Op o,
                                            DataType ty,
                                            core::AddressSpace as,
                                            const ast::IdentifierExpression* buf)
    : Base(pid, nid, Vector{buf}), op(o), type(ty), address_space(as) {}
DecomposeMemoryAccess::Intrinsic::~Intrinsic() = default;
std::string DecomposeMemoryAccess::Intrinsic::InternalName() const {
    StringStream ss;
    switch (op) {
        case Op::kLoad:
            ss << "intrinsic_load_";
            break;
        case Op::kStore:
            ss << "intrinsic_store_";
            break;
        case Op::kAtomicLoad:
            ss << "intrinsic_atomic_load_";
            break;
        case Op::kAtomicStore:
            ss << "intrinsic_atomic_store_";
            break;
        case Op::kAtomicAdd:
            ss << "intrinsic_atomic_add_";
            break;
        case Op::kAtomicSub:
            ss << "intrinsic_atomic_sub_";
            break;
        case Op::kAtomicMax:
            ss << "intrinsic_atomic_max_";
            break;
        case Op::kAtomicMin:
            ss << "intrinsic_atomic_min_";
            break;
        case Op::kAtomicAnd:
            ss << "intrinsic_atomic_and_";
            break;
        case Op::kAtomicOr:
            ss << "intrinsic_atomic_or_";
            break;
        case Op::kAtomicXor:
            ss << "intrinsic_atomic_xor_";
            break;
        case Op::kAtomicExchange:
            ss << "intrinsic_atomic_exchange_";
            break;
        case Op::kAtomicCompareExchangeWeak:
            ss << "intrinsic_atomic_compare_exchange_weak_";
            break;
    }
    ss << address_space << "_";
    switch (type) {
        case DataType::kU32:
            ss << "u32";
            break;
        case DataType::kF32:
            ss << "f32";
            break;
        case DataType::kI32:
            ss << "i32";
            break;
        case DataType::kF16:
            ss << "f16";
            break;
        case DataType::kVec2U32:
            ss << "vec2_u32";
            break;
        case DataType::kVec2F32:
            ss << "vec2_f32";
            break;
        case DataType::kVec2I32:
            ss << "vec2_i32";
            break;
        case DataType::kVec2F16:
            ss << "vec2_f16";
            break;
        case DataType::kVec3U32:
            ss << "vec3_u32";
            break;
        case DataType::kVec3F32:
            ss << "vec3_f32";
            break;
        case DataType::kVec3I32:
            ss << "vec3_i32";
            break;
        case DataType::kVec3F16:
            ss << "vec3_f16";
            break;
        case DataType::kVec4U32:
            ss << "vec4_u32";
            break;
        case DataType::kVec4F32:
            ss << "vec4_f32";
            break;
        case DataType::kVec4I32:
            ss << "vec4_i32";
            break;
        case DataType::kVec4F16:
            ss << "vec4_f16";
            break;
    }
    return ss.str();
}

const DecomposeMemoryAccess::Intrinsic* DecomposeMemoryAccess::Intrinsic::Clone(
    ast::CloneContext& ctx) const {
    auto buf = ctx.Clone(Buffer());
    return ctx.dst->ASTNodes().Create<DecomposeMemoryAccess::Intrinsic>(
        ctx.dst->ID(), ctx.dst->AllocateNodeID(), op, type, address_space, buf);
}

bool DecomposeMemoryAccess::Intrinsic::IsAtomic() const {
    return op != Op::kLoad && op != Op::kStore;
}

const ast::IdentifierExpression* DecomposeMemoryAccess::Intrinsic::Buffer() const {
    return dependencies[0];
}

DecomposeMemoryAccess::DecomposeMemoryAccess() = default;
DecomposeMemoryAccess::~DecomposeMemoryAccess() = default;

ast::transform::Transform::ApplyResult DecomposeMemoryAccess::Apply(
    const Program& src,
    const ast::transform::DataMap&,
    ast::transform::DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    auto& sem = src.Sem();
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};
    State state(ctx);

    // Scan the AST nodes for storage and uniform buffer accesses. Complex
    // expression chains (e.g. `storage_buffer.foo.bar[20].x`) are handled by
    // maintaining an offset chain via the `state.TakeAccess()`,
    // `state.AddAccess()` methods.
    //
    // Inner-most expression nodes are guaranteed to be visited first because AST
    // nodes are fully immutable and require their children to be constructed
    // first so their pointer can be passed to the parent's initializer.
    for (auto* node : src.ASTNodes().Objects()) {
        if (auto* ident = node->As<ast::IdentifierExpression>()) {
            // X
            if (auto* sem_ident = sem.GetVal(ident)) {
                if (auto* user = sem_ident->UnwrapLoad()->As<sem::VariableUser>()) {
                    if (auto* global = user->Variable()->As<sem::GlobalVariable>()) {
                        if (global->AddressSpace() == core::AddressSpace::kStorage ||
                            global->AddressSpace() == core::AddressSpace::kUniform) {
                            // Variable to a storage or uniform buffer
                            state.AddAccess(ident, {
                                                       global,
                                                       state.ToOffset(0u),
                                                       global->Type()->UnwrapRef(),
                                                   });
                        }
                    }
                }
            }
            continue;
        }

        if (auto* accessor = node->As<ast::MemberAccessorExpression>()) {
            // X.Y
            auto* accessor_sem = sem.Get(accessor)->UnwrapLoad();
            if (auto* swizzle = accessor_sem->As<sem::Swizzle>()) {
                if (swizzle->Indices().Length() == 1) {
                    if (auto access = state.TakeAccess(accessor->object)) {
                        auto* vec_ty = access.type->As<core::type::Vector>();
                        auto* offset = state.Mul(vec_ty->Type()->Size(), swizzle->Indices()[0u]);
                        state.AddAccess(accessor, {
                                                      access.var,
                                                      state.Add(access.offset, offset),
                                                      vec_ty->Type(),
                                                  });
                    }
                }
            } else {
                if (auto access = state.TakeAccess(accessor->object)) {
                    auto* str_ty = access.type->As<core::type::Struct>();
                    auto* member = str_ty->FindMember(accessor->member->symbol);
                    auto offset = member->Offset();
                    state.AddAccess(accessor, {
                                                  access.var,
                                                  state.Add(access.offset, offset),
                                                  member->Type(),
                                              });
                }
            }
            continue;
        }

        if (auto* accessor = node->As<ast::IndexAccessorExpression>()) {
            if (auto access = state.TakeAccess(accessor->object)) {
                // X[Y]
                if (auto* arr = access.type->As<core::type::Array>()) {
                    auto* offset = state.Mul(arr->Stride(), accessor->index);
                    state.AddAccess(accessor, {
                                                  access.var,
                                                  state.Add(access.offset, offset),
                                                  arr->ElemType(),
                                              });
                    continue;
                }
                if (auto* vec_ty = access.type->As<core::type::Vector>()) {
                    auto* offset = state.Mul(vec_ty->Type()->Size(), accessor->index);
                    state.AddAccess(accessor, {
                                                  access.var,
                                                  state.Add(access.offset, offset),
                                                  vec_ty->Type(),
                                              });
                    continue;
                }
                if (auto* mat_ty = access.type->As<core::type::Matrix>()) {
                    auto* offset = state.Mul(mat_ty->ColumnStride(), accessor->index);
                    state.AddAccess(accessor, {
                                                  access.var,
                                                  state.Add(access.offset, offset),
                                                  mat_ty->ColumnType(),
                                              });
                    continue;
                }
            }
        }

        if (auto* op = node->As<ast::UnaryOpExpression>()) {
            if (op->op == core::UnaryOp::kAddressOf) {
                // &X
                if (auto access = state.TakeAccess(op->expr)) {
                    // HLSL does not support pointers, so just take the access from the
                    // reference and place it on the pointer.
                    state.AddAccess(op, access);
                    continue;
                }
            }
        }

        if (auto* assign = node->As<ast::AssignmentStatement>()) {
            // X = Y
            // Move the LHS access to a store.
            if (auto lhs = state.TakeAccess(assign->lhs)) {
                state.stores.emplace_back(Store{assign, lhs});
            }
        }

        if (auto* call_expr = node->As<ast::CallExpression>()) {
            auto* call = sem.Get(call_expr)->UnwrapMaterialize()->As<sem::Call>();
            if (auto* builtin = call->Target()->As<sem::BuiltinFn>()) {
                if (builtin->Fn() == wgsl::BuiltinFn::kArrayLength) {
                    // arrayLength(X)
                    // Don't convert X into a load, this builtin actually requires the real pointer.
                    state.TakeAccess(call_expr->args[0]);
                    continue;
                }
                if (builtin->IsAtomic()) {
                    if (auto access = state.TakeAccess(call_expr->args[0])) {
                        // atomic___(X)
                        ctx.Replace(call_expr, [=, &ctx, &state] {
                            auto* offset = access.offset->Build(ctx);
                            auto* el_ty =
                                access.type->UnwrapRef()->As<core::type::Atomic>()->Type();
                            auto buffer = ctx.Clone(access.var->Declaration()->name->symbol);
                            Symbol func = state.AtomicFunc(el_ty, builtin, buffer);

                            Vector<const ast::Expression*, 8> args{offset};
                            for (size_t i = 1; i < call_expr->args.Length(); i++) {
                                auto* arg = call_expr->args[i];
                                args.Push(ctx.Clone(arg));
                            }
                            return ctx.dst->Call(func, args);
                        });
                    }
                }
            }
        }
    }

    // All remaining accesses are loads, transform these into calls to the
    // corresponding load function
    // TODO(crbug.com/tint/1784): Use `sem::Load`s instead of maintaining `state.expression_order`.
    for (auto* expr : state.expression_order) {
        auto access_it = state.accesses.find(expr);
        if (access_it == state.accesses.end()) {
            continue;
        }
        BufferAccess access = access_it->second;
        ctx.Replace(expr, [=, &ctx, &state] {
            auto* offset = access.offset->Build(ctx);
            auto* el_ty = access.type->UnwrapRef();
            auto buffer = ctx.Clone(access.var->Declaration()->name->symbol);
            Symbol func = state.LoadFunc(el_ty, access.var->AddressSpace(), buffer);
            return ctx.dst->Call(func, offset);
        });
    }

    // And replace all storage and uniform buffer assignments with stores
    for (auto store : state.stores) {
        ctx.Replace(store.assignment, [=, &ctx, &state] {
            auto* offset = store.target.offset->Build(ctx);
            auto* el_ty = store.target.type->UnwrapRef();
            auto* value = store.assignment->rhs;
            auto buffer = ctx.Clone(store.target.var->Declaration()->name->symbol);
            Symbol func = state.StoreFunc(el_ty, buffer);
            auto* call = ctx.dst->Call(func, offset, ctx.Clone(value));
            return ctx.dst->CallStmt(call);
        });
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::hlsl::writer

TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::Offset);
TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::OffsetLiteral);
