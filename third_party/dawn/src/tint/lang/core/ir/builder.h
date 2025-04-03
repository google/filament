// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_BUILDER_H_
#define SRC_TINT_LANG_CORE_IR_BUILDER_H_

#include <utility>

#include "src/tint/lang/core/constant/scalar.h"  // IWYU pragma: export
#include "src/tint/lang/core/constant/splat.h"   // IWYU pragma: export
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/value.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"  // IWYU pragma: export
#include "src/tint/lang/core/type/bool.h"   // IWYU pragma: export
#include "src/tint/lang/core/type/f16.h"    // IWYU pragma: export
#include "src/tint/lang/core/type/f32.h"    // IWYU pragma: export
#include "src/tint/lang/core/type/i32.h"    // IWYU pragma: export
#include "src/tint/lang/core/type/i8.h"     // IWYU pragma: export
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"  // IWYU pragma: export
#include "src/tint/lang/core/type/type.h"     // IWYU pragma: export
#include "src/tint/lang/core/type/u32.h"      // IWYU pragma: export
#include "src/tint/lang/core/type/u64.h"      // IWYU pragma: export
#include "src/tint/lang/core/type/u8.h"       // IWYU pragma: export
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"  // IWYU pragma: export
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {

/// Builds an ir::Module
class Builder {
    /// Evaluates to true if T is a non-reference instruction pointer.
    template <typename T>
    static constexpr bool IsNonRefInstPtr =
        std::is_pointer_v<T> && std::is_base_of_v<ir::Instruction, std::remove_pointer_t<T>>;

    /// static_assert()s that ARGS contains no more than one non-reference instruction pointer.
    /// This is used to detect patterns where C++ non-deterministic evaluation order may cause
    /// instruction ordering bugs.
    template <typename... ARGS>
    static constexpr void CheckForNonDeterministicEvaluation() {
        constexpr bool possibly_non_deterministic_eval =
            ((IsNonRefInstPtr<ARGS> ? 1 : 0) + ...) > 1;
        static_assert(!possibly_non_deterministic_eval,
                      "Detected possible non-deterministic ordering of instructions. "
                      "Consider hoisting Builder call arguments to separate statements.");
    }

    /// A helper used to enable overloads if the first type in `TYPES` is a Vector or
    /// VectorRef.
    template <typename... TYPES>
    using EnableIfVectorLike = tint::traits::EnableIf<
        tint::IsVectorLike<tint::traits::Decay<tint::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is a Vector or
    /// VectorRef.
    template <typename... TYPES>
    using DisableIfVectorLike = tint::traits::EnableIf<
        !tint::IsVectorLike<tint::traits::Decay<tint::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A namespace for the various instruction insertion method
    struct InsertionPoints {
        /// Insertion point method that does no insertion
        struct NoInsertion {
            /// The insertion point function
            void operator()(ir::Instruction*) {}
        };
        /// Insertion point method that inserts the instruction to the end of #block
        struct AppendToBlock {
            /// The block to insert new instructions to the end of
            ir::Block* block = nullptr;
            /// The insertion point function
            /// @param i the instruction to insert
            void operator()(ir::Instruction* i) { block->Append(i); }
        };
        /// Insertion point method that inserts the instruction after #after and updates the
        /// insertion point to be after the inserted instruction.
        struct InsertAfter {
            /// The instruction to insert new instructions after
            ir::Instruction* after = nullptr;
            /// The insertion point function
            /// @param i the instruction to insert
            void operator()(ir::Instruction* i) {
                i->InsertAfter(after);
                after = i;
            }
        };
        /// Insertion point method that inserts the instruction before #before
        struct InsertBefore {
            /// The instruction to insert new instructions before
            ir::Instruction* before = nullptr;
            /// The insertion point function
            /// @param i the instruction to insert
            void operator()(ir::Instruction* i) { i->InsertBefore(before); }
        };
    };

    /// A variant of different instruction insertion methods
    using InsertionPoint = std::variant<InsertionPoints::NoInsertion,
                                        InsertionPoints::AppendToBlock,
                                        InsertionPoints::InsertAfter,
                                        InsertionPoints::InsertBefore>;

    /// The insertion method used for new instructions.
    InsertionPoint insertion_point_{InsertionPoints::NoInsertion{}};

  public:
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    explicit Builder(Module& mod);
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    /// @param block the block to append to
    Builder(Module& mod, ir::Block* block);
    /// Destructor
    ~Builder();

    /// Creates a new builder that will append to the given block
    /// @param b the block to append new instructions to
    /// @returns the builder
    Builder Append(ir::Block* b) { return Builder(ir, b); }

    /// Calls @p cb with the builder appending to block @p b
    /// @param b the block to set as the block to append to
    /// @param cb the function to call with the builder appending to block @p b
    template <typename FUNCTION>
    void Append(ir::Block* b, FUNCTION&& cb) {
        TINT_SCOPED_ASSIGNMENT(insertion_point_, InsertionPoints::AppendToBlock{b});
        cb();
    }

    /// Calls @p cb with the builder inserting after @p ip
    /// @param ip the insertion point for new instructions
    /// @param cb the function to call with the builder inserting new instructions after @p ip
    template <typename FUNCTION>
    void InsertAfter(ir::Instruction* ip, FUNCTION&& cb) {
        TINT_SCOPED_ASSIGNMENT(insertion_point_, InsertionPoints::InsertAfter{ip});
        cb();
    }

    /// Calls @p cb with the builder inserting before @p ip
    /// @param ip the insertion point for new instructions
    /// @param cb the function to call with the builder inserting new instructions before @p ip
    template <typename FUNCTION>
    void InsertBefore(ir::Instruction* ip, FUNCTION&& cb) {
        TINT_SCOPED_ASSIGNMENT(insertion_point_, InsertionPoints::InsertBefore{ip});
        cb();
    }

    /// Calls @p cb with the builder inserting at the first block position after @p val. This means
    /// if a `FunctionParam` or `BlockParam` are provided, the callback will insert into the _next_
    /// block seen after the parameters.
    /// @param val the value used to determine which block to insert into
    /// @param cb the function to call with the builder inserting new instructions in the first
    /// block position after @p val
    template <typename FUNCTION>
    void InsertInBlockAfter(ir::Value* val, FUNCTION&& cb) {
        tint::Switch(
            val,
            [&](core::ir::InstructionResult* result) {
                const TINT_SCOPED_ASSIGNMENT(insertion_point_,
                                             InsertionPoints::InsertAfter{result->Instruction()});
                cb();
            },
            [&](core::ir::FunctionParam* param) {
                auto* body = param->Function()->Block();
                if (body->IsEmpty()) {
                    Append(body, cb);
                } else {
                    InsertBefore(body->Front(), cb);
                }
            },
            [&](core::ir::BlockParam* param) {
                auto* block = param->Block();
                if (block->IsEmpty()) {
                    Append(block, cb);
                } else {
                    InsertBefore(block->Front(), cb);
                }
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Adds and returns the instruction @p instruction to the current insertion point. If there
    /// is no current insertion point set, then @p instruction is just returned.
    /// @param instruction the instruction to append
    /// @returns the instruction
    template <typename T>
    T* Append(T* instruction) {
        std::visit([instruction](auto&& mode) { mode(instruction); }, insertion_point_);
        return instruction;
    }

    /// @returns a new block
    ir::Block* Block();

    /// @returns a new multi-in block
    ir::MultiInBlock* MultiInBlock();

    /// Creates an unnamed function
    /// @param return_type the function return type
    /// @param stage the function stage
    /// @returns the function
    ir::Function* Function(const core::type::Type* return_type,
                           Function::PipelineStage stage = Function::PipelineStage::kUndefined);

    /// Creates a function
    /// @param name the function name
    /// @param return_type the function return type
    /// @param stage the function stage
    /// @returns the function
    ir::Function* Function(std::string_view name,
                           const core::type::Type* return_type,
                           Function::PipelineStage stage = Function::PipelineStage::kUndefined);

    /// Creates a compute function
    /// @param name the function name
    /// @returns the function
    ir::Function* ComputeFunction(std::string_view name) {
        return ComputeFunction(name, u32(1), u32(1), u32(1));
    }

    /// Creates an unnamed compute function
    /// @param name the function name
    /// @param x the x dimension
    /// @param y the y dimension
    /// @param z the z dimension
    template <typename X,
              typename Y,
              typename Z,
              typename = std::enable_if_t<!std::is_integral_v<X> && !std::is_integral_v<Y> &&
                                          !std::is_integral_v<Z>>>
    ir::Function* ComputeFunction(std::string_view name, X&& x, Y&& y, Z&& z) {
        CheckForNonDeterministicEvaluation<X, Y, Z>();
        auto* x_val = Value(std::forward<X>(x));
        auto* y_val = Value(std::forward<Y>(y));
        auto* z_val = Value(std::forward<Z>(z));

        auto* ir_func = Function(name, ir.Types().void_(), Function::PipelineStage::kCompute);
        ir_func->SetWorkgroupSize({x_val, y_val, z_val});
        return ir_func;
    }

    /// Creates an if instruction
    /// @param condition the if condition
    /// @returns the instruction
    template <typename T>
    ir::If* If(T&& condition) {
        auto* cond_val = Value(std::forward<T>(condition));
        return Append(ir.CreateInstruction<ir::If>(cond_val, Block(), Block()));
    }

    /// Creates an const expression if instruction
    /// @param condition the const expression if condition
    /// @returns the instruction
    template <typename T>
    ir::ConstExprIf* ConstExprIf(T&& condition) {
        auto* cond_val = Value(std::forward<T>(condition));
        return Append(ir.CreateInstruction<ir::ConstExprIf>(cond_val, Block(), Block()));
    }

    /// Creates a loop instruction
    /// @returns the instruction
    ir::Loop* Loop();

    /// Creates a switch instruction
    /// @param condition the switch condition
    /// @returns the instruction
    template <typename T>
    ir::Switch* Switch(T&& condition) {
        auto* cond_val = Value(std::forward<T>(condition));
        return Append(ir.CreateInstruction<ir::Switch>(cond_val));
    }

    /// Creates a default case for the switch @p s
    /// @param s the switch to create the case into
    /// @returns the start block for the case instruction
    ir::Block* DefaultCase(ir::Switch* s);

    /// Creates a case for the switch @p s with the given selectors
    /// @param s the switch to create the case into
    /// @param values the case selector values for the case statement
    /// @returns the start block for the case instruction
    ir::Block* Case(ir::Switch* s, VectorRef<ir::Constant*> values);

    /// Creates a case for the switch @p s with the given selectors
    /// @param s the switch to create the case into
    /// @param values the case selector values for the case statement
    /// @returns the start block for the case instruction
    ir::Block* Case(ir::Switch* s, std::initializer_list<ir::Constant*> values);

    /// Creates a new ir::Constant
    /// @param val the constant value
    /// @returns the new constant
    ir::Constant* Constant(const core::constant::Value* val) {
        return ir.constants.GetOrAdd(val, [&] { return ir.CreateValue<ir::Constant>(val); });
    }

    /// Creates a ir::Constant for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::i32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for an i8 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::i8 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::u32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a u64 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::u64 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a u8 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::u8 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::f32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(core::f16 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    template <typename BOOL, typename = std::enable_if_t<std::is_same_v<BOOL, bool>>>
    ir::Constant* Constant(BOOL v) {
        return Constant(ConstantValue(v));
    }

    /// Creates a new invalid ir::Constant
    /// @returns the new constant
    ir::Constant* InvalidConstant() { return Constant(ir.constant_values.Invalid()); }

    /// Retrieves the inner constant from an ir::Constant
    /// @param constant the ir constant
    /// @returns the core::constant::Value inside the constant
    const core::constant::Value* ConstantValue(ir::Constant* constant) { return constant->Value(); }

    /// Creates a core::constant::Value for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::i32 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for an i8 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::i8 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::u32 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a u64 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::u64 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a u8 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::u8 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::f32 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    const core::constant::Value* ConstantValue(core::f16 v) { return ir.constant_values.Get(v); }

    /// Creates a core::constant::Value for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    template <typename BOOL, typename = std::enable_if_t<std::is_same_v<BOOL, bool>>>
    const core::constant::Value* ConstantValue(BOOL v) {
        return ir.constant_values.Get(v);
    }

    /// Return a constant that has the same number of vector components as `match`, each with
    /// the `value`. If `match` is scalar just return `value` as a constant.
    /// @param value the value
    /// @param match the type to match
    /// @returns the new constant
    template <typename ARG>
    ir::Constant* MatchWidth(ARG&& value, const core::type::Type* match) {
        auto* element = Constant(std::forward<ARG>(value));
        if (match->Is<core::type::Vector>()) {
            return Splat(ir.Types().MatchWidth(element->Type(), match), element);
        }
        return element;
    }

    /// Creates a new ir::Constant
    /// @param ty the splat type
    /// @param value the splat value
    /// @returns the new constant
    template <typename ARG>
    ir::Constant* Splat(const core::type::Type* ty, ARG&& value) {
        return Constant(ir.constant_values.Splat(ty, ConstantValue(std::forward<ARG>(value))));
    }

    /// Creates a new ir::Constant
    /// @tparam TYPE the splat type
    /// @param value the splat value
    /// @returns the new constant
    template <typename TYPE, typename ARG>
    ir::Constant* Splat(ARG&& value) {
        auto* type = ir.Types().Get<TYPE>();
        return Splat(type, std::forward<ARG>(value));
    }

    /// Creates a new ir::Constant
    /// @param ty the constant type
    /// @param values the composite values
    /// @returns the new constant
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    ir::Constant* Composite(const core::type::Type* ty, ARGS&&... values) {
        return Constant(
            ir.constant_values.Composite(ty, Vector{ConstantValue(std::forward<ARGS>(values))...}));
    }

    /// Creates a new ir::Constant
    /// @tparam TYPE the constant type
    /// @param values the composite values
    /// @returns the new constant
    template <typename TYPE, typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    ir::Constant* Composite(ARGS&&... values) {
        auto* type = ir.Types().Get<TYPE>();
        return Composite(type, std::forward<ARGS>(values)...);
    }

    /// Creates a new zero-value ir::Constant
    /// @tparam TYPE the constant type
    /// @returns the new constant
    template <typename TYPE>
    ir::Constant* Zero() {
        return Constant(ir.constant_values.Zero(ir.Types().Get<TYPE>()));
    }

    /// Creates a new zero-value ir::Constant
    /// @param ty the constant type
    /// @returns the new constant
    ir::Constant* Zero(const core::type::Type* ty) { return Constant(ir.constant_values.Zero(ty)); }

    /// @param in the input value. One of: nullptr, ir::Value*, ir::Instruction* or a numeric
    /// value.
    /// @returns an ir::Value* from the given argument.
    template <typename T>
    ir::Value* Value(T&& in) {
        using D = std::decay_t<T>;
        constexpr bool is_null = std::is_same_v<T, std::nullptr_t>;
        constexpr bool is_ptr = std::is_pointer_v<D>;
        constexpr bool is_numeric = core::IsNumeric<D>;
        static_assert(is_null || is_ptr || is_numeric, "invalid argument type for Value()");

        if constexpr (is_null) {
            return nullptr;
        } else if constexpr (is_ptr) {
            using P = std::remove_pointer_t<D>;
            constexpr bool is_value = std::is_base_of_v<ir::Value, P>;
            constexpr bool is_instruction = std::is_base_of_v<ir::Instruction, P>;
            static_assert(is_value || is_instruction, "invalid pointer type for Value()");

            if constexpr (is_value) {
                return in;  /// Pass-through
            } else if constexpr (is_instruction) {
                /// Extract the first result from the instruction
                auto results = in->Results();
                TINT_ASSERT(results.Length() == 1);
                return results[0];
            }
        } else if constexpr (is_numeric) {
            /// Creates a value from the given number
            return Constant(in);
        }
    }

    /// Pass-through overload for Values() with vector-like argument
    /// @param vec the vector of ir::Value*
    /// @return @p vec
    template <typename VEC, typename = EnableIfVectorLike<tint::traits::Decay<VEC>>>
    auto Values(VEC&& vec) {
        return std::forward<VEC>(vec);
    }

    template <typename T>
    auto Values(tint::Slice<T>&&) {
        static_assert(sizeof(T) != sizeof(T),  // Condition must be type-dependent
                      "Cannot construct a Vector from a Slice as the size is not known at "
                      "compile-time. Use ToVector<N>(Slice&&) instead.");
    }

    /// Overload for Values() with tint::Empty argument
    /// @return tint::Empty
    tint::EmptyType Values(tint::EmptyType) { return tint::Empty; }

    /// Overload for Values() with no arguments
    /// @return tint::Empty
    tint::EmptyType Values() { return tint::Empty; }

    /// @param args the arguments to pass to Value()
    /// @returns a vector of ir::Value* built from transforming the arguments with Value()
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    auto Values(ARGS&&... args) {
        CheckForNonDeterministicEvaluation<ARGS...>();
        return Vector{Value(std::forward<ARGS>(args))...};
    }

    /// Creates an op for `lhs kind rhs`
    /// @param op the binary operator
    /// @param type the result type of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Binary(BinaryOp op, const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return BinaryWithResult(InstructionResult(type), op, std::forward<LHS>(lhs),
                                std::forward<RHS>(rhs));
    }

    /// Creates an op for `lhs kind rhs`
    /// @param op the binary operator
    /// @param result the result of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* BinaryWithResult(ir::InstructionResult* result,
                                     BinaryOp op,
                                     LHS&& lhs,
                                     RHS&& rhs) {
        CheckForNonDeterministicEvaluation<LHS, RHS>();
        auto* lhs_val = Value(std::forward<LHS>(lhs));
        auto* rhs_val = Value(std::forward<RHS>(rhs));
        return Append(ir.CreateInstruction<ir::CoreBinary>(result, op, lhs_val, rhs_val));
    }

    /// Creates an op for `lhs kind rhs`
    /// @param op the binary operator
    /// @param type the result type of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    template <typename KLASS, typename LHS, typename RHS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::Binary>, KLASS*>
    Binary(BinaryOp op, const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        CheckForNonDeterministicEvaluation<LHS, RHS>();
        auto* lhs_val = Value(std::forward<LHS>(lhs));
        auto* rhs_val = Value(std::forward<RHS>(rhs));
        return Append(ir.CreateInstruction<KLASS>(InstructionResult(type), op, lhs_val, rhs_val));
    }

    /// Creates an And operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* And(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kAnd, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an And operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* And(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return And(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Or operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Or(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kOr, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Or operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Or(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Or(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Xor operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Xor(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kXor, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Xor operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Xor(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Xor(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Equal operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Equal(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kEqual, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Equal operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Equal(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Equal(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an NotEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* NotEqual(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kNotEqual, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an NotEqual operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* NotEqual(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return NotEqual(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an LessThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* LessThan(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kLessThan, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an LessThan operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* LessThan(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return LessThan(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* GreaterThan(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kGreaterThan, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThan operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* GreaterThan(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return GreaterThan(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an LessThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* LessThanEqual(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kLessThanEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an LessThanEqual operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* LessThanEqual(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return LessThanEqual(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* GreaterThanEqual(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kGreaterThanEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThanEqual operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* GreaterThanEqual(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return GreaterThanEqual(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an ShiftLeft operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* ShiftLeft(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kShiftLeft, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an ShiftLeft operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* ShiftLeft(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return ShiftLeft(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an ShiftRight operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* ShiftRight(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kShiftRight, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an ShiftRight operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* ShiftRight(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return ShiftRight(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Add operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Add(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kAdd, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Add operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Add(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Add(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Add operation
    /// @param result the result
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* AddWithResult(ir::InstructionResult* result, LHS&& lhs, RHS&& rhs) {
        return BinaryWithResult(result, BinaryOp::kAdd, std::forward<LHS>(lhs),
                                std::forward<RHS>(rhs));
    }

    /// Creates an Subtract operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Subtract(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kSubtract, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Subtract operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Subtract(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Subtract(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Multiply operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Multiply(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kMultiply, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Multiply operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Multiply(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Multiply(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Divide operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Divide(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kDivide, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Divide operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Divide(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Divide(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Modulo operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::CoreBinary* Modulo(const core::type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(BinaryOp::kModulo, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Modulo operation
    /// @tparam TYPE the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename TYPE, typename LHS, typename RHS>
    ir::CoreBinary* Modulo(LHS&& lhs, RHS&& rhs) {
        auto* type = ir.Types().Get<TYPE>();
        return Modulo(type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an op for `op val`
    /// @param op the unary operator
    /// @param type the result type of the binary expression
    /// @param val the value of the operation
    /// @returns the operation
    template <typename VAL>
    ir::CoreUnary* Unary(UnaryOp op, const core::type::Type* type, VAL&& val) {
        auto* value = Value(std::forward<VAL>(val));
        return Append(ir.CreateInstruction<ir::CoreUnary>(InstructionResult(type), op, value));
    }

    /// Creates an op for `op val`
    /// @param op the unary operator
    /// @tparam TYPE the result type of the binary expression
    /// @param val the value of the operation
    /// @returns the operation
    template <typename TYPE, typename VAL>
    ir::CoreUnary* Unary(UnaryOp op, VAL&& val) {
        auto* type = ir.Types().Get<TYPE>();
        return Unary(op, type, std::forward<VAL>(val));
    }

    /// Creates a Complement operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::CoreUnary* Complement(const core::type::Type* type, VAL&& val) {
        return Unary(UnaryOp::kComplement, type, std::forward<VAL>(val));
    }

    /// Creates a Complement operation
    /// @tparam TYPE the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename TYPE, typename VAL>
    ir::CoreUnary* Complement(VAL&& val) {
        auto* type = ir.Types().Get<TYPE>();
        return Complement(type, std::forward<VAL>(val));
    }

    /// Creates a Negation operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::CoreUnary* Negation(const core::type::Type* type, VAL&& val) {
        return Unary(UnaryOp::kNegation, type, std::forward<VAL>(val));
    }

    /// Creates a Negation operation
    /// @tparam TYPE the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename TYPE, typename VAL>
    ir::CoreUnary* Negation(VAL&& val) {
        auto* type = ir.Types().Get<TYPE>();
        return Negation(type, std::forward<VAL>(val));
    }

    /// Creates a Not operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::CoreUnary* Not(const core::type::Type* type, VAL&& val) {
        return Unary(UnaryOp::kNot, type, std::forward<VAL>(val));
    }

    /// Creates a Not operation
    /// @tparam TYPE the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename TYPE, typename VAL>
    ir::CoreUnary* Not(VAL&& val) {
        auto* type = ir.Types().Get<TYPE>();
        return Not(type, std::forward<VAL>(val));
    }

    /// Creates a bitcast instruction
    /// @param type the result type of the bitcast
    /// @param val the value being bitcast
    /// @returns the instruction
    template <typename VAL>
    ir::Bitcast* Bitcast(const core::type::Type* type, VAL&& val) {
        auto* value = Value(std::forward<VAL>(val));
        return Append(ir.CreateInstruction<ir::Bitcast>(InstructionResult(type), value));
    }

    /// Creates a bitcast instruction
    /// @tparam TYPE the result type of the bitcast
    /// @param val the value being bitcast
    /// @returns the instruction
    template <typename TYPE, typename VAL>
    ir::Bitcast* Bitcast(VAL&& val) {
        auto* type = ir.Types().Get<TYPE>();
        auto* value = Value(std::forward<VAL>(val));
        return Bitcast(type, value);
    }

    /// Creates a bitcast instruction
    /// @param result the result
    /// @param val the value being bitcast
    /// @returns the instruction
    template <typename VAL>
    ir::Bitcast* BitcastWithResult(ir::InstructionResult* result, VAL&& val) {
        return Append(ir.CreateInstruction<ir::Bitcast>(result, val));
    }

    /// Creates a discard instruction
    /// @returns the instruction
    ir::Discard* Discard();

    /// Creates a user function call instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param func the function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::UserCall* CallWithResult(ir::InstructionResult* result,
                                 ir::Function* func,
                                 ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::UserCall>(result, func, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a user function call instruction
    /// @param func the function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::UserCall* Call(ir::Function* func, ARGS&&... args) {
        return Call(func->ReturnType(), func, std::forward<ARGS>(args)...);
    }

    /// Creates a user function call instruction
    /// @param type the return type of the call
    /// @param func the function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::UserCall* Call(const core::type::Type* type, ir::Function* func, ARGS&&... args) {
        return CallWithResult(InstructionResult(type), func, Values(std::forward<ARGS>(args)...));
    }

    /// Creates a user function call instruction
    /// @tparam TYPE the return type of the call
    /// @param func the function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename TYPE, typename... ARGS>
    ir::UserCall* Call(ir::Function* func, ARGS&&... args) {
        auto* type = ir.Types().Get<TYPE>();
        return CallWithResult(InstructionResult(type), func, Values(std::forward<ARGS>(args)...));
    }

    /// Creates a core builtin call instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::CoreBuiltinCall* CallWithResult(core::ir::InstructionResult* result,
                                        core::BuiltinFn func,
                                        ARGS&&... args) {
        return Append(ir.CreateInstruction<ir::CoreBuiltinCall>(
            result, func, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a core builtin call instruction
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::CoreBuiltinCall* Call(const core::type::Type* type, core::BuiltinFn func, ARGS&&... args) {
        return CallWithResult(InstructionResult(type), func, Values(std::forward<ARGS>(args)...));
    }

    /// Creates a core builtin call instruction
    /// @tparam TYPE the return type of the call
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename TYPE, typename... ARGS>
    ir::CoreBuiltinCall* Call(core::BuiltinFn func, ARGS&&... args) {
        auto* type = ir.Types().Get<TYPE>();
        return CallWithResult(InstructionResult(type), func, Values(std::forward<ARGS>(args)...));
    }

    /// Creates a builtin call instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param func the builtin function to call
    /// @param explicit_params the explicit params
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::BuiltinCall>, KLASS*>
    CallExplicitWithResult(ir::InstructionResult* result,
                           FUNC func,
                           VectorRef<const core::type::Type*> explicit_params,
                           ARGS&&... args) {
        auto* inst = ir.CreateInstruction<KLASS>(result, func, Values(std::forward<ARGS>(args)...));
        inst->SetExplicitTemplateParams(explicit_params);
        return Append(inst);
    }

    /// Creates a builtin call instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::BuiltinCall>, KLASS*>
    CallWithResult(ir::InstructionResult* result, FUNC func, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<KLASS>(result, func, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a builtin call instruction
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param explicit_params the explicit parameters
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::BuiltinCall>, KLASS*>
    CallExplicit(const core::type::Type* type,
                 FUNC func,
                 VectorRef<const core::type::Type*> explicit_params,
                 ARGS&&... args) {
        return CallExplicitWithResult<KLASS>(InstructionResult(type), func, explicit_params,
                                             Values(std::forward<ARGS>(args)...));
    }

    /// Creates a core builtin call instruction with explicit parameters
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param explicit_params the explicit parameters
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::CoreBuiltinCall* CallExplicit(const core::type::Type* type,
                                      core::BuiltinFn func,
                                      VectorRef<const core::type::Type*> explicit_params,
                                      ARGS&&... args) {
        return CallExplicitWithResult<core::ir::CoreBuiltinCall>(
            InstructionResult(type), func, explicit_params, Values(std::forward<ARGS>(args)...));
    }

    /// Creates a builtin call instruction
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::BuiltinCall>, KLASS*>
    Call(const core::type::Type* type, FUNC func, ARGS&&... args) {
        return CallWithResult<KLASS>(InstructionResult(type), func,
                                     Values(std::forward<ARGS>(args)...));
    }

    /// Creates a member builtin call instruction with an existing instruction result.
    /// @param result the instruction result to use
    /// @param func the builtin function to call
    /// @param obj the object
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename OBJ, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::MemberBuiltinCall>, KLASS*>
    MemberCallWithResult(ir::InstructionResult* result, FUNC func, OBJ&& obj, ARGS&&... args) {
        return Append(ir.CreateInstruction<KLASS>(result, func, Value(std::forward<OBJ>(obj)),
                                                  Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a member builtin call instruction.
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param obj the object
    /// @param args the call arguments
    /// @returns the instruction
    template <typename KLASS, typename FUNC, typename OBJ, typename... ARGS>
    tint::traits::EnableIf<tint::traits::IsTypeOrDerived<KLASS, ir::MemberBuiltinCall>, KLASS*>
    MemberCall(const core::type::Type* type, FUNC func, OBJ&& obj, ARGS&&... args) {
        return MemberCallWithResult<KLASS>(InstructionResult(type), func,
                                           Value(std::forward<OBJ>(obj)),
                                           Values(std::forward<ARGS>(args)...));
    }

    /// Creates a value conversion instruction with an existing instruction result.
    /// @param result the instruction result to use
    /// @param val the value to be converted
    /// @returns the instruction
    template <typename VAL>
    ir::Convert* ConvertWithResult(ir::InstructionResult* result, VAL&& val) {
        return Append(ir.CreateInstruction<ir::Convert>(result, Value(std::forward<VAL>(val))));
    }

    /// Creates a value conversion instruction to the template type T
    /// @param val the value to be converted
    /// @returns the instruction
    template <typename T, typename VAL>
    ir::Convert* Convert(VAL&& val) {
        auto* type = ir.Types().Get<T>();
        return Convert(type, std::forward<VAL>(val));
    }

    /// Creates a value conversion instruction
    /// @param to the type converted to
    /// @param val the value to be converted
    /// @returns the instruction
    template <typename VAL>
    ir::Convert* Convert(const core::type::Type* to, VAL&& val) {
        return ConvertWithResult(InstructionResult(to), Value(std::forward<VAL>(val)));
    }

    /// Adds a call to convert if destination type is different then the value's type
    /// @param to the type converted to
    /// @param val the value to be converted
    /// @returns either result of the conversion or original value
    ir::Value* InsertConvertIfNeeded(const core::type::Type* to, ir::Value* val) {
        return val->Type()->Equals(*to) ? val : Convert(to, val)->Result();
    }

    /// Creates a value constructor instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param args the arguments to the constructor
    /// @returns the instruction
    template <typename... ARGS>
    ir::Construct* ConstructWithResult(ir::InstructionResult* result, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::Construct>(result, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a value constructor instruction to the template type T
    /// @param args the arguments to the constructor
    /// @returns the instruction
    template <typename T, typename... ARGS>
    ir::Construct* Construct(ARGS&&... args) {
        auto* type = ir.Types().Get<T>();
        return Construct(type, std::forward<ARGS>(args)...);
    }

    /// Creates a value constructor instruction
    /// @param type the type to constructed
    /// @param args the arguments to the constructor
    /// @returns the instruction
    template <typename... ARGS>
    ir::Construct* Construct(const core::type::Type* type, ARGS&&... args) {
        return ConstructWithResult(InstructionResult(type), Values(std::forward<ARGS>(args)...));
    }

    /// Creates a load instruction with an existing result
    /// @param result the instruction result to use
    /// @param from the expression being loaded from
    /// @returns the instruction
    template <typename VAL>
    ir::Load* LoadWithResult(ir::InstructionResult* result, VAL&& from) {
        auto* value = Value(std::forward<VAL>(from));
        return Append(ir.CreateInstruction<ir::Load>(result, value));
    }

    /// Creates a load instruction
    /// @param from the expression being loaded from
    /// @returns the instruction
    template <typename VAL>
    ir::Load* Load(VAL&& from) {
        auto* value = Value(std::forward<VAL>(from));
        return LoadWithResult(InstructionResult(value->Type()->UnwrapPtrOrRef()), value);
    }

    /// Creates a store instruction
    /// @param to the expression being stored too
    /// @param from the expression being stored
    /// @returns the instruction
    template <typename TO, typename FROM>
    ir::Store* Store(TO&& to, FROM&& from) {
        CheckForNonDeterministicEvaluation<TO, FROM>();
        auto* to_val = Value(std::forward<TO>(to));
        auto* from_val = Value(std::forward<FROM>(from));
        return Append(ir.CreateInstruction<ir::Store>(to_val, from_val));
    }

    /// Creates a store vector element instruction
    /// @param to the vector pointer expression being stored too
    /// @param index the new vector element index
    /// @param value the new vector element expression
    /// @returns the instruction
    template <typename TO, typename INDEX, typename VALUE>
    ir::StoreVectorElement* StoreVectorElement(TO&& to, INDEX&& index, VALUE&& value) {
        CheckForNonDeterministicEvaluation<TO, INDEX, VALUE>();
        auto* to_val = Value(std::forward<TO>(to));
        auto* index_val = Value(std::forward<INDEX>(index));
        auto* value_val = Value(std::forward<VALUE>(value));
        return Append(ir.CreateInstruction<ir::StoreVectorElement>(to_val, index_val, value_val));
    }

    /// Creates a load vector element instruction with an existing instruction result
    /// @param result the instruction result to use
    /// @param from the vector pointer expression being loaded from
    /// @param index the new vector element index
    /// @returns the instruction
    template <typename FROM, typename INDEX>
    ir::LoadVectorElement* LoadVectorElementWithResult(ir::InstructionResult* result,
                                                       FROM&& from,
                                                       INDEX&& index) {
        CheckForNonDeterministicEvaluation<FROM, INDEX>();
        auto* from_val = Value(std::forward<FROM>(from));
        auto* index_val = Value(std::forward<INDEX>(index));
        return Append(ir.CreateInstruction<ir::LoadVectorElement>(result, from_val, index_val));
    }

    /// Creates a load vector element instruction
    /// @param from the vector pointer expression being loaded from
    /// @param index the new vector element index
    /// @returns the instruction
    template <typename FROM, typename INDEX>
    ir::LoadVectorElement* LoadVectorElement(FROM&& from, INDEX&& index) {
        CheckForNonDeterministicEvaluation<FROM, INDEX>();
        auto* from_val = Value(std::forward<FROM>(from));
        auto* index_val = Value(std::forward<INDEX>(index));
        auto* res = InstructionResult(VectorPtrElementType(from_val->Type()));
        return LoadVectorElementWithResult(res, from_val, index_val);
    }

    /// Creates a new `var` declaration
    /// @param type the var type
    /// @returns the instruction
    ir::Var* Var(const core::type::MemoryView* type);

    /// Creates a new `var` declaration with a name
    /// @param name the var name
    /// @param type the var type
    /// @returns the instruction
    ir::Var* Var(std::string_view name, const core::type::MemoryView* type);

    /// Creates a new `var` declaration with a name and initializer value
    /// @tparam SPACE the var's address space
    /// @tparam ACCESS the var's access mode
    /// @param name the var name
    /// @param init the var initializer
    /// @returns the instruction
    template <
        core::AddressSpace SPACE = core::AddressSpace::kFunction,
        core::Access ACCESS = core::Access::kReadWrite,
        typename VALUE = void,
        typename = std::enable_if_t<
            !traits::IsTypeOrDerived<std::remove_pointer_t<std::decay_t<VALUE>>, core::type::Type>>>
    ir::Var* Var(std::string_view name, VALUE&& init) {
        auto* val = Value(std::forward<VALUE>(init));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* var = Var(name, ir.Types().ptr(SPACE, val->Type(), ACCESS));
        var->SetInitializer(val);
        ir.SetName(var->Result(), name);
        return var;
    }

    /// Creates a new `var` declaration
    /// @tparam SPACE the var's address space
    /// @tparam T the storage pointer's element type
    /// @tparam ACCESS the var's access mode
    /// @returns the instruction
    template <core::AddressSpace SPACE,
              typename T,
              core::Access ACCESS = core::type::DefaultAccessFor(SPACE)>
    ir::Var* Var() {
        return Var(ir.Types().ptr<SPACE, T, ACCESS>());
    }

    /// Creates a new `var` declaration with a name
    /// @tparam SPACE the var's address space
    /// @tparam T the storage pointer's element type
    /// @tparam ACCESS the var's access mode
    /// @param name the var name
    /// @returns the instruction
    template <core::AddressSpace SPACE,
              typename T,
              core::Access ACCESS = core::type::DefaultAccessFor(SPACE)>
    ir::Var* Var(std::string_view name) {
        return Var(name, ir.Types().ptr<SPACE, T, ACCESS>());
    }

    /// Creates a new `var` declaration with a name
    /// @param name the var name
    /// @param space the var's address space
    /// @param subtype the storage pointer's element type
    /// @param access the var's access mode
    /// @returns the instruction
    ir::Var* Var(std::string_view name,
                 core::AddressSpace space,
                 const core::type::Type* subtype,
                 core::Access access = core::Access::kUndefined) {
        return Var(name, ir.Types().ptr(space, subtype, access));
    }

    /// Creates a new `let` declaration
    /// @param name the let name
    /// @param value the let value
    /// @returns the instruction
    template <typename VALUE>
    ir::Let* Let(std::string_view name, VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* let = Append(ir.CreateInstruction<ir::Let>(InstructionResult(val->Type()), val));
        ir.SetName(let->Result(), name);
        return let;
    }

    /// Creates a new `let` declaration, with an unassigned value
    /// @param type the let type
    /// @returns the instruction
    ir::Let* Let(const core::type::Type* type) {
        auto* let = ir.CreateInstruction<ir::Let>(InstructionResult(type), nullptr);
        Append(let);
        return let;
    }

    /// Creates a new `let` declaration
    /// @param value the value
    /// @returns the instruction
    template <
        typename VALUE,
        typename = std::enable_if_t<
            !traits::IsTypeOrDerived<std::remove_pointer_t<std::decay_t<VALUE>>, core::type::Type>>>
    ir::Let* Let(VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* let = ir.CreateInstruction<ir::Let>(InstructionResult(val->Type()), val);
        Append(let);
        return let;
    }

    /// Creates a return instruction
    /// @param func the function being returned
    /// @returns the instruction
    ir::Return* Return(ir::Function* func) {
        return Append(ir.CreateInstruction<ir::Return>(func));
    }

    /// Creates a return instruction
    /// @param func the function being returned
    /// @param value the return value
    /// @returns the instruction
    template <typename ARG>
    ir::Return* Return(ir::Function* func, ARG&& value) {
        if constexpr (std::is_same_v<std::decay_t<ARG>, ir::Value*>) {
            if (value == nullptr) {
                return Append(ir.CreateInstruction<ir::Return>(func));
            }
        }
        auto* val = Value(std::forward<ARG>(value));
        return Append(ir.CreateInstruction<ir::Return>(func, val));
    }

    /// Creates a loop next iteration instruction
    /// @param loop the loop being iterated
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::NextIteration* NextIteration(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::NextIteration>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a loop break-if instruction
    /// @param condition the break condition
    /// @param loop the loop being iterated
    /// @returns the instruction
    template <typename CONDITION>
    ir::BreakIf* BreakIf(ir::Loop* loop, CONDITION&& condition) {
        CheckForNonDeterministicEvaluation<CONDITION>();
        auto* cond_val = Value(std::forward<CONDITION>(condition));
        return Append(ir.CreateInstruction<ir::BreakIf>(cond_val, loop));
    }

    /// Creates a loop break-if instruction
    /// @param condition the break condition
    /// @param loop the loop being iterated
    /// @param next_iter_values the arguments passed to the loop body MultiInBlock, if the break
    /// condition evaluates to `false`.
    /// @param exit_values the values returned by the loop, if the break condition evaluates to
    /// `true`.
    /// @returns the instruction
    template <typename CONDITION, typename NEXT_ITER_VALUES, typename EXIT_VALUES>
    ir::BreakIf* BreakIf(ir::Loop* loop,
                         CONDITION&& condition,
                         NEXT_ITER_VALUES&& next_iter_values,
                         EXIT_VALUES&& exit_values) {
        CheckForNonDeterministicEvaluation<CONDITION, NEXT_ITER_VALUES, EXIT_VALUES>();
        auto* cond_val = Value(std::forward<CONDITION>(condition));
        return Append(ir.CreateInstruction<ir::BreakIf>(
            cond_val, loop, Values(std::forward<NEXT_ITER_VALUES>(next_iter_values)),
            Values(std::forward<EXIT_VALUES>(exit_values))));
    }

    /// Creates a continue instruction
    /// @param loop the loop being continued
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::Continue* Continue(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::Continue>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit switch instruction
    /// @param sw the switch being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitSwitch* ExitSwitch(ir::Switch* sw, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::ExitSwitch>(sw, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit loop instruction
    /// @param loop the loop being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitLoop* ExitLoop(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.CreateInstruction<ir::ExitLoop>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit if instruction
    /// @param i the if being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitIf* ExitIf(ir::If* i, ARGS&&... args) {
        return Append(ir.CreateInstruction<ir::ExitIf>(i, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit instruction for the given control instruction
    /// @param inst the control instruction being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the exit instruction, or nullptr if the control instruction is not supported.
    template <typename... ARGS>
    ir::Exit* Exit(ir::ControlInstruction* inst, ARGS&&... args) {
        return tint::Switch(
            inst,  //
            [&](ir::If* i) { return ExitIf(i, std::forward<ARGS>(args)...); },
            [&](ir::Loop* i) { return ExitLoop(i, std::forward<ARGS>(args)...); },
            [&](ir::Switch* i) { return ExitSwitch(i, std::forward<ARGS>(args)...); });
    }

    /// Creates a new `BlockParam`
    /// @param type the parameter type
    /// @returns the value
    ir::BlockParam* BlockParam(const core::type::Type* type);

    /// Creates a new `BlockParam` with a name.
    /// @param name the parameter name
    /// @param type the parameter type
    /// @returns the value
    ir::BlockParam* BlockParam(std::string_view name, const core::type::Type* type);

    /// Creates a new `BlockParam` with a name.
    /// @tparam TYPE the parameter type
    /// @param name the parameter name
    /// @returns the value
    template <typename TYPE>
    ir::BlockParam* BlockParam(std::string_view name) {
        auto* type = ir.Types().Get<TYPE>();
        return BlockParam(name, type);
    }

    /// Creates a new `BlockParam`
    /// @tparam TYPE the parameter type
    /// @returns the value
    template <typename TYPE>
    ir::BlockParam* BlockParam() {
        auto* type = ir.Types().Get<TYPE>();
        return BlockParam(type);
    }

    /// Creates a new `FunctionParam`
    /// @param type the parameter type
    /// @returns the value
    ir::FunctionParam* FunctionParam(const core::type::Type* type);

    /// Creates a new `FunctionParam` with a name.
    /// @param name the parameter name
    /// @param type the parameter type
    /// @returns the value
    ir::FunctionParam* FunctionParam(std::string_view name, const core::type::Type* type);

    /// Creates a new `FunctionParam` with a name.
    /// @tparam TYPE the parameter type
    /// @param name the parameter name
    /// @returns the value
    template <typename TYPE>
    ir::FunctionParam* FunctionParam(std::string_view name) {
        auto* type = ir.Types().Get<TYPE>();
        return FunctionParam(name, type);
    }

    /// Creates a new `FunctionParam`
    /// @tparam TYPE the parameter type
    /// @returns the value
    template <typename TYPE>
    ir::FunctionParam* FunctionParam() {
        auto* type = ir.Types().Get<TYPE>();
        return FunctionParam(type);
    }

    /// Creates a new `Access` with an existing instruction result
    /// @param result the instruction result to use
    /// @param object the object being accessed
    /// @param indices the access indices
    /// @returns the instruction
    template <typename OBJ, typename... ARGS>
    ir::Access* AccessWithResult(ir::InstructionResult* result, OBJ&& object, ARGS&&... indices) {
        CheckForNonDeterministicEvaluation<OBJ, ARGS...>();
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.CreateInstruction<ir::Access>(result, obj_val,
                                                       Values(std::forward<ARGS>(indices)...)));
    }

    /// Creates a new `Access`
    /// @param type the return type
    /// @param object the object being accessed
    /// @param indices the access indices
    /// @returns the instruction
    template <typename OBJ, typename... ARGS>
    ir::Access* Access(const core::type::Type* type, OBJ&& object, ARGS&&... indices) {
        return AccessWithResult(InstructionResult(type), std::forward<OBJ>(object),
                                Values(std::forward<ARGS>(indices)...));
    }

    /// Creates a new `Access`
    /// @tparam TYPE the return type
    /// @param object the object being accessed
    /// @param indices the access indices
    /// @returns the instruction
    template <typename TYPE, typename OBJ, typename... ARGS>
    ir::Access* Access(OBJ&& object, ARGS&&... indices) {
        auto* type = ir.Types().Get<TYPE>();
        return Access(type, std::forward<OBJ>(object), std::forward<ARGS>(indices)...);
    }

    /// Creates a new `Swizzle`
    /// @param type the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename OBJ>
    ir::Swizzle* Swizzle(const core::type::Type* type, OBJ&& object, VectorRef<uint32_t> indices) {
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.CreateInstruction<ir::Swizzle>(InstructionResult(type), obj_val,
                                                        std::move(indices)));
    }

    /// Creates a new `Swizzle`
    /// @tparam TYPE the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename TYPE, typename OBJ>
    ir::Swizzle* Swizzle(OBJ&& object, VectorRef<uint32_t> indices) {
        auto* type = ir.Types().Get<TYPE>();
        return Swizzle(type, std::forward<OBJ>(object), std::move(indices));
    }

    /// Creates a new `Swizzle`
    /// @param type the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename OBJ>
    ir::Swizzle* Swizzle(const core::type::Type* type,
                         OBJ&& object,
                         std::initializer_list<uint32_t> indices) {
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.CreateInstruction<ir::Swizzle>(InstructionResult(type), obj_val,
                                                        Vector<uint32_t, 4>(indices)));
    }

    /// Name names the value or instruction with @p name
    /// @param name the new name for the value or instruction
    /// @param object the value or instruction
    /// @return @p object
    template <typename OBJECT>
    OBJECT* Name(std::string_view name, OBJECT* object) {
        ir.SetName(object, name);
        return object;
    }

    /// Creates a terminate invocation instruction
    /// @returns the instruction
    ir::TerminateInvocation* TerminateInvocation();

    /// Creates an unreachable instruction
    /// @returns the instruction
    ir::Unreachable* Unreachable();

    /// Creates an unused instruction
    /// @returns the instruction
    ir::Unused* Unused();

    /// Creates a new phony assignment declaration
    /// @param value the assignment value
    /// @returns the instruction
    template <typename VALUE>
    ir::Phony* Phony(VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        return Append(ir.CreateInstruction<ir::Phony>(val));
    }

    /// Creates a new runtime value
    /// @param type the return type
    /// @returns the value
    ir::InstructionResult* InstructionResult(const core::type::Type* type) {
        return ir.CreateValue<ir::InstructionResult>(type);
    }

    /// Creates a new runtime value
    /// @tparam TYPE the return type
    /// @returns the value
    template <typename TYPE>
    ir::InstructionResult* InstructionResult() {
        auto* type = ir.Types().Get<TYPE>();
        return InstructionResult(type);
    }

    /// Create a ranged loop with a callback to build the loop body.
    /// @param ty the type manager to use for new types
    /// @param start the first loop index
    /// @param end one past the last loop index
    /// @param step the loop index step amount
    /// @param cb the callback to call for the loop body
    template <typename START, typename END, typename STEP, typename FUNCTION>
    void LoopRange(core::type::Manager& ty, START&& start, END&& end, STEP&& step, FUNCTION&& cb) {
        auto* start_value = Value(std::forward<START>(start));
        auto* end_value = Value(std::forward<END>(end));
        auto* step_value = Value(std::forward<STEP>(step));

        auto* loop = Loop();
        auto* idx = BlockParam("idx", start_value->Type());
        loop->Body()->SetParams({idx});
        Append(loop->Initializer(), [&] {
            // Start the loop with `idx = start`.
            NextIteration(loop, start_value);
        });
        Append(loop->Body(), [&] {
            // Loop until `idx == end`.
            auto* breakif = If(GreaterThanEqual(ty.bool_(), idx, end_value));
            Append(breakif->True(), [&] {  //
                ExitLoop(loop);
            });

            cb(idx);

            Continue(loop);
        });
        Append(loop->Continuing(), [&] {
            // Update the index with `idx += step` and go to the next iteration.
            auto* new_idx = Add(idx->Type(), idx, step_value);
            NextIteration(loop, new_idx);
        });
    }

    /// Creates a new `override` declaration
    /// @param name the override name
    /// @param value the override value
    /// @returns the instruction
    template <
        typename VALUE,
        typename = std::enable_if_t<
            !traits::IsTypeOrDerived<std::remove_pointer_t<std::decay_t<VALUE>>, core::type::Type>>>
    ir::Override* Override(std::string_view name, VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* override = Append(ir.CreateInstruction<ir::Override>(InstructionResult(val->Type())));
        override->SetInitializer(val);
        ir.SetName(override->Result(), name);
        return override;
    }

    /// Creates a new `override` declaration
    /// @param src the source
    /// @param name the override name
    /// @param value the override value
    /// @returns the instruction
    template <
        typename VALUE,
        typename = std::enable_if_t<
            !traits::IsTypeOrDerived<std::remove_pointer_t<std::decay_t<VALUE>>, core::type::Type>>>
    ir::Override* Override(Source src, std::string_view name, VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (DAWN_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* override = Append(ir.CreateInstruction<ir::Override>(InstructionResult(val->Type())));
        override->SetInitializer(val);
        ir.SetName(override->Result(), name);
        ir.SetSource(override, src);
        return override;
    }

    /// Creates a new `override` declaration, with an unassigned value
    /// @param name the override name
    /// @param type the override type
    /// @returns the instruction
    ir::Override* Override(std::string_view name, const core::type::Type* type) {
        return Override(Source{}, name, type);
    }

    /// Creates a new `override` declaration, with an unassigned value
    /// @param name the override name
    /// @param type the override type
    /// @returns the instruction
    ir::Override* Override(Source src, std::string_view name, const core::type::Type* type) {
        auto* override = ir.CreateInstruction<ir::Override>(InstructionResult(type));
        ir.SetName(override->Result(), name);
        ir.SetSource(override, src);
        Append(override);
        return override;
    }

    /// Creates a new `override` declaration, with an unassigned value
    /// @param type the override type
    /// @returns the instruction
    ir::Override* Override(const core::type::Type* type) {
        auto* override = ir.CreateInstruction<ir::Override>(InstructionResult(type));
        Append(override);
        return override;
    }

    /// The IR module.
    Module& ir;

  private:
    /// @returns the element type of the vector-pointer type
    /// Asserts and return i32 if @p type is not a pointer to a vector
    const core::type::Type* VectorPtrElementType(const core::type::Type* type);
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BUILDER_H_
