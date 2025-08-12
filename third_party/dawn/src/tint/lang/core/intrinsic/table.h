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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_

#include <memory>
#include <string>
#include <utility>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/intrinsic/ctor_conv.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/unary_op.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"

// Forward declarations
namespace tint::diag {
class List;
}  // namespace tint::diag

namespace tint::core::intrinsic {

/// Overload describes a fully matched builtin function overload
struct Overload {
    static constexpr size_t kNumFixedParameters = 8;

    /// Parameter describes a single parameter
    struct Parameter {
        /// Parameter type
        const core::type::Type* const type;
        /// Parameter usage
        core::ParameterUsage const usage = core::ParameterUsage::kNone;

        /// Equality operator
        /// @param other the parameter to compare against
        /// @returns true if this parameter and @p other are the same
        bool operator==(const Parameter& other) const {
            return type == other.type && usage == other.usage;
        }

        /// Inequality operator
        /// @param other the parameter to compare against
        /// @returns false if this parameter and @p other are the same
        bool operator!=(const Parameter& other) const { return !(*this == other); }
    };

    /// The overload information
    const OverloadInfo* info = nullptr;

    /// The resolved overload return type
    core::type::Type const* return_type = nullptr;

    /// The resolved overload parameters
    Vector<Parameter, kNumFixedParameters> parameters;

    /// The constant evaluation function
    constant::Eval::Function const_eval_fn = nullptr;

    /// Equality operator
    /// @param other the overload to compare against
    /// @returns true if this overload and @p other are the same
    bool operator==(const Overload& other) const {
        return info == other.info && return_type == other.return_type &&
               parameters == other.parameters;
    }

    /// Inequality operator
    /// @param other the overload to compare against
    /// @returns false if this overload and @p other are the same
    bool operator!=(const Overload& other) const { return !(*this == other); }
};

/// The context data used to lookup intrinsic information
struct Context {
    /// The table table
    const TableData& data;
    /// The type manager
    core::type::Manager& types;
    /// The symbol table
    SymbolTable& symbols;

    /// @returns a MatchState from the context and arguments.
    /// @param templates the template state used for matcher evaluation
    /// @param overload the overload being evaluated
    /// @param matcher_indices pointer to a list of matcher indices
    MatchState Match(TemplateState& templates,
                     const OverloadInfo& overload,
                     const MatcherIndex* matcher_indices,
                     EvaluationStage earliest_eval_stage) {
        return MatchState(types, symbols, templates, data, overload, matcher_indices,
                          earliest_eval_stage);
    }
};

/// Candidate holds information about an overload evaluated for resolution.
struct Candidate {
    /// The match-score of the candidate overload.
    /// A score of zero indicates an exact match.
    /// Non-zero scores are used for diagnostics when no overload matches.
    /// Lower scores are displayed first (top-most).
    size_t score = 0;
    /// The candidate overload
    const OverloadInfo* overload = nullptr;
    /// The template types and numbers
    TemplateState templates{};
    /// The parameter types for the candidate overload
    Vector<Overload::Parameter, Overload::kNumFixedParameters> parameters{};
};

// Prints the candidate overload for emitting diagnostics
void PrintCandidate(StyledText& ss,
                    Context& context,
                    const Candidate& candidate,
                    std::string_view intrinsic_name,
                    VectorRef<const core::type::Type*> template_args,
                    VectorRef<const core::type::Type*> args);

/// Lookup looks for the builtin overload with the given signature, raising an error diagnostic
/// if the builtin was not found.
/// @param context the intrinsic context
/// @param function_name the name of the function
/// @param function_id the function identifier
/// @param template_args the optional template arguments
/// @param args the argument types passed to the builtin function
/// @param earliest_eval_stage the earliest evaluation stage that a call to
///        the builtin can be made. This can alter the overloads considered.
///        For example, if the earliest evaluation stage is `EvaluationStage::kRuntime`, then
///        only overloads with concrete argument types will be considered, as all
///        abstract-numerics will have been materialized after shader creation time
///        (EvaluationStage::kConstant).
/// @return the resolved builtin function overload
Result<Overload, StyledText> LookupFn(Context& context,
                                      std::string_view function_name,
                                      size_t function_id,
                                      VectorRef<const core::type::Type*> template_args,
                                      VectorRef<const core::type::Type*> args,
                                      EvaluationStage earliest_eval_stage);

/// Lookup looks for the member builtin overload with the given signature, raising an error
/// diagnostic if the builtin was not found.
/// @param context the intrinsic context
/// @param function_name the name of the function
/// @param function_id the function identifier
/// @param template_args the optional template arguments
/// @param args the argument types passed to the builtin function
/// @param earliest_eval_stage the earliest evaluation stage that a call to
///        the builtin can be made. This can alter the overloads considered.
///        For example, if the earliest evaluation stage is `EvaluationStage::kRuntime`, then
///        only overloads with concrete argument types will be considered, as all
///        abstract-numerics will have been materialized after shader creation time
///        (EvaluationStage::kConstant).
/// @return the resolved builtin function overload
Result<Overload, StyledText> LookupMemberFn(Context& context,
                                            std::string_view function_name,
                                            size_t function_id,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage);

/// Lookup looks for the unary op overload with the given signature, raising an error
/// diagnostic if the operator was not found.
/// @param context the intrinsic context
/// @param op the unary operator
/// @param arg the type of the expression passed to the operator
/// @param earliest_eval_stage the earliest evaluation stage that a call to
///        the unary operator can be made. This can alter the overloads considered.
///        For example, if the earliest evaluation stage is
///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
///        will be considered, as all abstract-numerics will have been materialized
///        after shader creation time (EvaluationStage::kConstant).
/// @return the resolved unary operator overload
Result<Overload, StyledText> LookupUnary(Context& context,
                                         core::UnaryOp op,
                                         const core::type::Type* arg,
                                         EvaluationStage earliest_eval_stage);

/// Lookup looks for the binary op overload with the given signature, raising an error
/// diagnostic if the operator was not found.
/// @param context the intrinsic context
/// @param op the binary operator
/// @param lhs the LHS value type passed to the operator
/// @param rhs the RHS value type passed to the operator
/// @param earliest_eval_stage the earliest evaluation stage that a call to
///        the binary operator can be made. This can alter the overloads considered.
///        For example, if the earliest evaluation stage is
///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
///        will be considered, as all abstract-numerics will have been materialized
///        after shader creation time (EvaluationStage::kConstant).
/// @param is_compound true if the binary operator is being used as a compound assignment
/// @return the resolved binary operator overload
Result<Overload, StyledText> LookupBinary(Context& context,
                                          core::BinaryOp op,
                                          const core::type::Type* lhs,
                                          const core::type::Type* rhs,
                                          EvaluationStage earliest_eval_stage,
                                          bool is_compound);

/// Lookup looks for the value constructor or conversion overload for the given CtorConv.
/// @param context the intrinsic context
/// @param type_name the name of the type being constructed or converted
/// @param type_id the type identifier
/// @param template_args the optional template arguments
/// @param args the argument types passed to the constructor / conversion call
/// @param earliest_eval_stage the earliest evaluation stage that a call to
///        the constructor or conversion can be made. This can alter the overloads considered.
///        For example, if the earliest evaluation stage is
///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
///        will be considered, as all abstract-numerics will have been materialized
///        after shader creation time (EvaluationStage::kConstant).
/// @return the resolved type constructor or conversion function overload
Result<Overload, StyledText> LookupCtorConv(Context& context,
                                            std::string_view type_name,
                                            size_t type_id,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage);

/// Table is a wrapper around a dialect to provide type-safe interface to the intrinsic table.
template <typename DIALECT>
struct Table {
    /// Alias to DIALECT::BuiltinFn
    using BuiltinFn = typename DIALECT::BuiltinFn;

    /// Alias to DIALECT::CtorConv
    using CtorConv = typename DIALECT::CtorConv;

    static_assert(std::is_enum_v<BuiltinFn>);
    static_assert(std::is_enum_v<CtorConv>);

    /// @param types The type manager
    /// @param symbols The symbol table
    Table(core::type::Manager& types, SymbolTable& symbols)
        : context{DIALECT::kData, types, symbols} {}

    /// Lookup looks for the builtin overload with the given signature, raising an error diagnostic
    /// if the builtin was not found.
    /// @param builtin_fn the builtin function
    /// @param template_args the optional template arguments
    /// @param args the argument types passed to the builtin function
    /// @param earliest_eval_stage the earliest evaluation stage that a call to
    ///        the builtin can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is `EvaluationStage::kRuntime`, then
    ///        only overloads with concrete argument types will be considered, as all
    ///        abstract-numerics will have been materialized after shader creation time
    ///        (EvaluationStage::kConstant).
    /// @return the resolved builtin function overload
    Result<Overload, StyledText> Lookup(BuiltinFn builtin_fn,
                                        VectorRef<const core::type::Type*> template_args,
                                        VectorRef<const core::type::Type*> args,
                                        EvaluationStage earliest_eval_stage) {
        std::string_view name = DIALECT::ToString(builtin_fn);
        size_t id = static_cast<size_t>(builtin_fn);
        return LookupFn(context, name, id, std::move(template_args), std::move(args),
                        earliest_eval_stage);
    }

    /// Lookup looks for the member builtin overload with the given signature, raising an error
    /// diagnostic if the builtin was not found.
    /// @param builtin_fn the builtin function
    /// @param object the object type
    /// @param template_args the optional template arguments
    /// @param args the argument types passed to the builtin function
    /// @param earliest_eval_stage the earliest evaluation stage that a call to
    ///        the builtin can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is `EvaluationStage::kRuntime`, then
    ///        only overloads with concrete argument types will be considered, as all
    ///        abstract-numerics will have been materialized after shader creation time
    ///        (EvaluationStage::kConstant).
    /// @return the resolved builtin function overload
    Result<Overload, StyledText> Lookup(BuiltinFn builtin_fn,
                                        const core::type::Type* object,
                                        VectorRef<const core::type::Type*> template_args,
                                        VectorRef<const core::type::Type*> args,
                                        EvaluationStage earliest_eval_stage) {
        // Push the object type into the argument list.
        auto full_args = Vector<const core::type::Type*, 8>({object});
        for (auto* arg : args) {
            full_args.Push(arg);
        }
        std::string_view name = DIALECT::ToString(builtin_fn);
        size_t id = static_cast<size_t>(builtin_fn);
        return LookupMemberFn(context, name, id, std::move(template_args), std::move(full_args),
                              earliest_eval_stage);
    }

    /// Lookup looks for the unary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the unary operator
    /// @param arg the type of the expression passed to the operator
    /// @param earliest_eval_stage the earliest evaluation stage that a call to
    ///        the unary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).

    /// @return the resolved unary operator overload
    Result<Overload, StyledText> Lookup(core::UnaryOp op,
                                        const core::type::Type* arg,
                                        EvaluationStage earliest_eval_stage) {
        return LookupUnary(context, op, arg, earliest_eval_stage);
    }

    /// Lookup looks for the binary op overload with the given signature, raising an error
    /// diagnostic if the operator was not found.
    /// @param op the binary operator
    /// @param lhs the LHS value type passed to the operator
    /// @param rhs the RHS value type passed to the operator
    /// @param earliest_eval_stage the earliest evaluation stage that a call to
    ///        the binary operator can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).

    /// @param is_compound true if the binary operator is being used as a compound assignment
    /// @return the resolved binary operator overload
    Result<Overload, StyledText> Lookup(core::BinaryOp op,
                                        const core::type::Type* lhs,
                                        const core::type::Type* rhs,
                                        EvaluationStage earliest_eval_stage,
                                        bool is_compound) {
        return LookupBinary(context, op, lhs, rhs, earliest_eval_stage, is_compound);
    }

    /// Lookup looks for the value constructor or conversion overload for the given CtorConv.
    /// @param type the type being constructed or converted
    /// @param template_args the optional template arguments
    /// @param args the argument types passed to the constructor / conversion call
    /// @param earliest_eval_stage the earliest evaluation stage that a call to
    ///        the constructor or conversion can be made. This can alter the overloads considered.
    ///        For example, if the earliest evaluation stage is
    ///        `EvaluationStage::kRuntime`, then only overloads with concrete argument types
    ///        will be considered, as all abstract-numerics will have been materialized
    ///        after shader creation time (EvaluationStage::kConstant).
    /// @return the resolved type constructor or conversion function overload
    Result<Overload, StyledText> Lookup(CtorConv type,
                                        VectorRef<const core::type::Type*> template_args,
                                        VectorRef<const core::type::Type*> args,
                                        EvaluationStage earliest_eval_stage) {
        std::string_view name = DIALECT::ToString(type);
        size_t id = static_cast<size_t>(type);
        return LookupCtorConv(context, name, id, std::move(template_args), std::move(args),
                              earliest_eval_stage);
    }

    /// The intrinsic context
    Context context;
};

}  // namespace tint::core::intrinsic

namespace tint {

/// Hasher specialization for core::intrinsic::Overload
template <>
struct Hasher<core::intrinsic::Overload> {
    /// @param i the core::intrinsic::Overload to create a hash for
    /// @return the hash value
    inline HashCode operator()(const core::intrinsic::Overload& i) const {
        HashCode hash = Hash(i.parameters.Length());
        for (auto& p : i.parameters) {
            hash = HashCombine(hash, p.type, p.usage);
        }
        return Hash(hash, i.info, i.return_type);
    }
};

}  // namespace tint

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TABLE_H_
