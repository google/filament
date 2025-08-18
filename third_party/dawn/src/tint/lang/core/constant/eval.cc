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

#include "src/tint/lang/core/constant/eval.h"

#include <algorithm>
#include <limits>
#include <numbers>
#include <string>
#include <type_traits>
#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/memory/bitcast.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::constant {
namespace {

/// Returns the first element of a parameter pack
template <typename T>
T First(T&& first, ...) {
    return std::forward<T>(first);
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_iu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::U32*) { return f(cs->template ValueAs<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_ia_iu32(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::U32*) { return f(cs->template ValueAs<u32>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_ia_iu32_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const core::type::Bool*) { return f(cs->template ValueAs<bool>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fi32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const core::type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const core::type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fiu32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const core::type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const core::type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const core::type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fia_fiu32_f16_bool(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractInt*) { return f(cs->template ValueAs<AInt>()...); },
        [&](const core::type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const core::type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const core::type::I32*) { return f(cs->template ValueAs<i32>()...); },
        [&](const core::type::U32*) { return f(cs->template ValueAs<u32>()...); },
        [&](const core::type::F16*) { return f(cs->template ValueAs<f16>()...); },
        [&](const core::type::Bool*) { return f(cs->template ValueAs<bool>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_fa_f32_f16(F&& f, CONSTANTS&&... cs) {
    return Switch(
        First(cs...)->Type(),  //
        [&](const core::type::AbstractFloat*) { return f(cs->template ValueAs<AFloat>()...); },
        [&](const core::type::F32*) { return f(cs->template ValueAs<f32>()...); },
        [&](const core::type::F16*) { return f(cs->template ValueAs<f16>()...); });
}

/// Helper that calls `f` passing in the value of all `cs`.
/// Calls `f` with all constants cast to the type of the first `cs` argument.
template <typename F, typename... CONSTANTS>
auto Dispatch_bool(F&& f, CONSTANTS&&... cs) {
    return f(cs->template ValueAs<bool>()...);
}

/// ZeroTypeDispatch is a helper for calling the function `f`, passing a single zero-value argument
/// of the C++ type that corresponds to the core::type::Type `type`. For example, calling
/// `ZeroTypeDispatch()` with a type of `type::I32*` will call the function f with a single argument
/// of `i32(0)`.
/// @returns the value returned by calling `f`.
/// @note `type` must be a scalar or abstract numeric type. Other types will not call `f`, and will
/// return the zero-initialized value of the return type for `f`.
template <typename F>
auto ZeroTypeDispatch(const core::type::Type* type, F&& f) {
    return Switch(
        type,                                                            //
        [&](const core::type::AbstractInt*) { return f(AInt(0)); },      //
        [&](const core::type::AbstractFloat*) { return f(AFloat(0)); },  //
        [&](const core::type::I32*) { return f(i32(0)); },               //
        [&](const core::type::U32*) { return f(u32(0)); },               //
        [&](const core::type::F32*) { return f(f32(0)); },               //
        [&](const core::type::F16*) { return f(f16(0)); },               //
        [&](const core::type::Bool*) { return f(static_cast<bool>(0)); });
}

template <typename NumberT>
std::string OverflowErrorMessage(NumberT lhs, const char* op, NumberT rhs) {
    StringStream ss;
    ss << "'" << lhs.value << " " << op << " " << rhs.value << "' cannot be represented as '"
       << FriendlyName<NumberT>() << "'";
    return ss.str();
}

template <typename VALUE_TY>
std::string OverflowErrorMessage(VALUE_TY value, std::string_view target_ty) {
    StringStream ss;
    ss << "value " << value << " cannot be represented as " << "'" << target_ty << "'";
    return ss.str();
}

template <typename NumberT>
std::string OverflowExpErrorMessage(std::string_view base, NumberT exp) {
    StringStream ss;
    ss << base << "^" << exp << " cannot be represented as " << "'" << FriendlyName<NumberT>()
       << "'";
    return ss.str();
}

/// @returns the number of consecutive leading bits in `@p e` set to `@p bit_value_to_count`.
template <typename T>
std::make_unsigned_t<T> CountLeadingBits(T e, T bit_value_to_count) {
    using UT = std::make_unsigned_t<T>;
    constexpr UT kNumBits = sizeof(UT) * 8;
    constexpr UT kLeftMost = UT{1} << (kNumBits - 1);
    const UT b = bit_value_to_count == 0 ? UT{0} : kLeftMost;

    auto v = static_cast<UT>(e);
    auto count = UT{0};
    while ((count < kNumBits) && ((v & kLeftMost) == b)) {
        ++count;
        v <<= 1;
    }
    return count;
}

/// @returns the number of consecutive trailing bits set to `@p bit_value_to_count` in `@p e`
template <typename T>
std::make_unsigned_t<T> CountTrailingBits(T e, T bit_value_to_count) {
    using UT = std::make_unsigned_t<T>;
    constexpr UT kNumBits = sizeof(UT) * 8;
    constexpr UT kRightMost = UT{1};
    const UT b = static_cast<UT>(bit_value_to_count);

    auto v = static_cast<UT>(e);
    auto count = UT{0};
    while ((count < kNumBits) && ((v & kRightMost) == b)) {
        ++count;
        v >>= 1;
    }
    return count;
}

/// Common data for constant conversion.
struct ConvertContext {
    Manager& mgr;
    diag::List& diags;
    const Source& source;
    bool use_runtime_semantics;
};

/// Converts the constant scalar value to the target type.
/// @returns the converted scalar, or nullptr on error.
template <typename T>
const ScalarBase* ScalarConvert(const Scalar<T>* scalar,
                                const core::type::Type* target_ty,
                                ConvertContext& ctx) {
    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
    if (target_ty == scalar->type) {
        // If the types are identical, then no conversion is needed.
        return scalar;
    }
    return ZeroTypeDispatch(target_ty, [&](auto zero_to) -> const ScalarBase* {
        // `value` is the source value.
        // `FROM` is the source type.
        // `TO` is the target type.
        using TO = std::decay_t<decltype(zero_to)>;
        using FROM = T;
        if constexpr (std::is_same_v<TO, bool>) {
            // [x -> bool]
            return ctx.mgr.Get<Scalar<TO>>(target_ty, !scalar->IsZero());
        } else if constexpr (std::is_same_v<FROM, bool>) {
            // [bool -> x]
            return ctx.mgr.Get<Scalar<TO>>(target_ty, TO(scalar->value ? 1 : 0));
        } else if (auto conv = CheckedConvert<TO>(scalar->value); conv == Success) {
            // Conversion success
            return ctx.mgr.Get<Scalar<TO>>(target_ty, conv.Get());
            // --- Below this point are the failure cases ---
        } else if constexpr (IsAbstract<FROM>) {
            // [abstract-numeric -> x] - materialization failure
            auto msg = OverflowErrorMessage(scalar->value, target_ty->FriendlyName());
            if (ctx.use_runtime_semantics) {
                ctx.diags.AddWarning(ctx.source) << msg;
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Highest());
                }
            } else {
                ctx.diags.AddError(ctx.source) << msg;
                return nullptr;
            }
        } else if constexpr (IsFloatingPoint<TO>) {
            // [x -> floating-point] - number not exactly representable
            // https://www.w3.org/TR/WGSL/#floating-point-conversion
            auto msg = OverflowErrorMessage(scalar->value, target_ty->FriendlyName());
            if (ctx.use_runtime_semantics) {
                ctx.diags.AddWarning(ctx.source) << msg;
                switch (conv.Failure()) {
                    case ConversionFailure::kExceedsNegativeLimit:
                        return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Lowest());
                    case ConversionFailure::kExceedsPositiveLimit:
                        return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Highest());
                }
            } else {
                ctx.diags.AddError(ctx.source) << msg;
                return nullptr;
            }
        } else if constexpr (IsFloatingPoint<FROM>) {
            // [floating-point -> integer] - number not exactly representable
            // https://www.w3.org/TR/WGSL/#floating-point-conversion
            switch (conv.Failure()) {
                case ConversionFailure::kExceedsNegativeLimit:
                    return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Lowest());
                case ConversionFailure::kExceedsPositiveLimit:
                    return ctx.mgr.Get<Scalar<TO>>(target_ty, TO::Highest());
            }
        } else if constexpr (IsIntegral<FROM>) {
            // [integer -> integer] - number not exactly representable
            // Static cast
            return ctx.mgr.Get<Scalar<TO>>(target_ty, static_cast<TO>(scalar->value));
        }
        TINT_UNREACHABLE() << "Expression is not constant";
        return nullptr;
    });
    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
}

/// Converts the constant value to the target type.
/// @returns the converted value, or nullptr on error.
const Value* ConvertInternal(const Value* root_value,
                             const core::type::Type* root_target_ty,
                             ConvertContext& ctx) {
    struct ActionConvert {
        const Value* value = nullptr;
        const core::type::Type* target_ty = nullptr;
    };
    struct ActionBuildSplat {
        size_t count = 0;
        const core::type::Type* type = nullptr;
    };
    struct ActionBuildComposite {
        size_t count = 0;
        const core::type::Type* type = nullptr;
    };
    using Action = std::variant<ActionConvert, ActionBuildSplat, ActionBuildComposite>;

    Vector<Action, 8> pending{
        ActionConvert{root_value, root_target_ty},
    };

    Vector<const Value*, 32> value_stack;

    while (!pending.IsEmpty()) {
        auto next = pending.Pop();

        if (auto* build = std::get_if<ActionBuildSplat>(&next)) {
            TINT_ASSERT(value_stack.Length() >= 1);
            auto* el = value_stack.Pop();
            value_stack.Push(ctx.mgr.Splat(build->type, el));
            continue;
        }

        if (auto* build = std::get_if<ActionBuildComposite>(&next)) {
            TINT_ASSERT(value_stack.Length() >= build->count);
            // Take build->count elements off the top of value_stack
            // Note: The values are ordered with the first composite value at the top of the stack.
            Vector<const Value*, 32> elements;
            elements.Reserve(build->count);
            for (size_t i = 0; i < build->count; i++) {
                elements.Push(value_stack.Pop());
            }
            // Build the composite
            value_stack.Push(ctx.mgr.Composite(build->type, std::move(elements)));
            continue;
        }

        auto* convert = std::get_if<ActionConvert>(&next);

        bool ok = Switch(
            convert->value,
            [&](const ScalarBase* scalar) {
                auto* converted = Switch(
                    scalar,
                    [&](const Scalar<AFloat>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<AInt>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<u32>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<i32>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<f32>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<f16>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    },
                    [&](const Scalar<bool>* val) {
                        return ScalarConvert(val, convert->target_ty, ctx);
                    });
                if (!converted) {
                    return false;
                }
                value_stack.Push(converted);
                return true;
            },
            [&](const Splat* splat) {
                const core::type::Type* target_el_ty = nullptr;
                if (auto* str = convert->target_ty->As<core::type::Struct>()) {
                    // Structure conversion.
                    auto members = str->Members();
                    target_el_ty = members[0]->Type();

                    // Structures can only be converted during materialization. The user cannot
                    // declare the target structure type, so each member type must be the same
                    // default materialization type.
                    for (size_t i = 1; i < members.Length(); i++) {
                        if (members[i]->Type() != target_el_ty) {
                            TINT_ICE()
                                << "inconsistent target struct member types for SplatConvert";
                        }
                    }
                } else {
                    target_el_ty = convert->target_ty->Elements(convert->target_ty).type;
                }

                // Convert the single splatted element type.
                pending.Push(ActionBuildSplat{splat->count, convert->target_ty});
                pending.Push(ActionConvert{splat->el, target_el_ty});
                return true;
            },
            [&](const Composite* composite) {
                const size_t el_count = composite->NumElements();

                // Build the new composite from the converted element types.
                pending.Push(ActionBuildComposite{el_count, convert->target_ty});

                if (auto* str = convert->target_ty->As<core::type::Struct>()) {
                    if (DAWN_UNLIKELY(str->Members().Length() != el_count)) {
                        TINT_ICE()
                            << "const-eval conversion of structure has mismatched element counts";
                    }
                    // Struct composites can have different types for each member.
                    auto members = str->Members();
                    for (size_t i = 0; i < el_count; i++) {
                        pending.Push(ActionConvert{composite->Index(i), members[i]->Type()});
                    }
                } else {
                    // Non-struct composites have the same type for all elements.
                    auto* el_ty = convert->target_ty->Elements(convert->target_ty).type;
                    for (size_t i = 0; i < el_count; i++) {
                        auto* el = composite->Index(i);
                        pending.Push(ActionConvert{el, el_ty});
                    }
                }

                return true;
            });
        if (!ok) {
            return nullptr;
        }
    }

    TINT_ASSERT(value_stack.Length() == 1);
    return value_stack.Pop();
}

/// TransformElements constructs a new constant of type `composite_ty` by applying the
/// transformation function `f` on each of the most deeply nested elements of 'cs'. Assumes that all
/// input constants `cs` are of the same arity (all scalars or all vectors of the same size).
/// If `f`'s last argument is a `size_t`, then the index of the most deeply nested element inside
/// the most deeply nested aggregate type will be passed in.
template <typename F, typename... CONSTANTS>
std::enable_if_t<tint::traits::IsType<size_t, tint::traits::LastParameterType<F>>, Eval::Result>
TransformElements(Manager& mgr,
                  const core::type::Type* composite_ty,
                  const F& f,
                  size_t index,
                  CONSTANTS&&... cs) {
    auto [el_ty, n] = First(cs...)->Type()->Elements();
    if (!el_ty) {
        return f(cs..., index);
    }

    auto* composite_el_ty = composite_ty->Elements(composite_ty).type;

    Vector<const Value*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = TransformElements(mgr, composite_el_ty, f, index + i, cs->Index(i)...);
            el == Success) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return mgr.Composite(composite_ty, std::move(els));
}

/// Signature of a unary transformation callback
using UnaryTransform = std::function<Eval::Result(const Value*)>;

/// TransformUnaryElements constructs a new constant of type `composite_ty` by applying the
/// transformation function 'f' on each of the most deeply nested elements of `c0`.
Eval::Result TransformUnaryElements(Manager& mgr,
                                    const core::type::Type* composite_ty,
                                    const UnaryTransform& f,
                                    const Value* c0) {
    auto [el_ty, n] = c0->Type()->Elements();
    if (!el_ty) {
        return f(c0);
    }

    auto* composite_el_ty = composite_ty->Elements(composite_ty).type;

    Vector<const Value*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = TransformUnaryElements(mgr, composite_el_ty, f, c0->Index(i));
            el == Success) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return mgr.Composite(composite_ty, std::move(els));
}

/// Signature of a binary transformation callback.
using BinaryTransform = std::function<Eval::Result(const Value*, const Value*)>;

/// TransformBinaryElements constructs a new constant of type `composite_ty` by applying the
/// transformation function 'f' on each of the most deeply nested elements of both `c0` and `c1`.
Eval::Result TransformBinaryElements(Manager& mgr,
                                     const core::type::Type* composite_ty,
                                     const BinaryTransform& f,
                                     const Value* c0,
                                     const Value* c1) {
    auto [el_ty, n] = c0->Type()->Elements();
    if (!el_ty) {
        return f(c0, c1);
    }

    TINT_ASSERT(n == c1->Type()->Elements().count);

    auto* composite_el_ty = composite_ty->Elements(composite_ty).type;

    Vector<const Value*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = TransformBinaryElements(mgr, composite_el_ty, f, c0->Index(i), c1->Index(i));
            el == Success) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return mgr.Composite(composite_ty, std::move(els));
}

/// TransformBinaryDifferingArityElements constructs a new constant of type `composite_ty` by
/// applying the transformation function 'f' on each of the most deeply nested elements of both `c0`
/// and `c1`. Unlike TransformElements, this function handles the constants being of different
/// arity, e.g. vector-scalar, scalar-vector.
Eval::Result TransformBinaryDifferingArityElements(Manager& mgr,
                                                   const core::type::Type* composite_ty,
                                                   const BinaryTransform& f,
                                                   const Value* c0,
                                                   const Value* c1) {
    uint32_t n0 = c0->Type()->Elements(nullptr, 1).count;
    uint32_t n1 = c1->Type()->Elements(nullptr, 1).count;
    uint32_t max_n = std::max(n0, n1);
    // If arity of both constants is 1, invoke callback
    if (max_n == 1u) {
        return f(c0, c1);
    }

    const auto* element_ty = composite_ty->Elements(composite_ty).type;

    Vector<const Value*, 8> els;
    els.Reserve(max_n);
    for (uint32_t i = 0; i < max_n; i++) {
        auto nested_or_self = [&](auto* c, uint32_t num_elems) {
            return (num_elems == 1) ? c : c->Index(i);
        };
        if (auto el = TransformBinaryDifferingArityElements(
                mgr, element_ty, f, nested_or_self(c0, n0), nested_or_self(c1, n1));
            el == Success) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return mgr.Composite(composite_ty, std::move(els));
}

/// Signature of a ternary transformation callback
using TernaryTransform = std::function<Eval::Result(const Value*, const Value*, const Value*)>;

/// TransformTernaryElements constructs a new constant of type `composite_ty` by applying the
/// transformation function 'f' on each of the most deeply nested elements of both `c0`, `c1`, and
/// `c2`.
Eval::Result TransformTernaryElements(Manager& mgr,
                                      const core::type::Type* composite_ty,
                                      const TernaryTransform& f,
                                      const Value* c0,
                                      const Value* c1,
                                      const Value* c2) {
    auto [el_ty, n] = c0->Type()->Elements();
    if (!el_ty) {
        return f(c0, c1, c2);
    }

    auto* composite_el_ty = composite_ty->Elements(composite_ty).type;

    Vector<const Value*, 8> els;
    els.Reserve(n);
    for (uint32_t i = 0; i < n; i++) {
        if (auto el = TransformTernaryElements(mgr, composite_el_ty, f, c0->Index(i), c1->Index(i),
                                               c2->Index(i));
            el == Success) {
            els.Push(el.Get());
        } else {
            return el.Failure();
        }
    }
    return mgr.Composite(composite_ty, std::move(els));
}

/// @returns the nth byte of an integer value
template <typename I, typename R>
constexpr R GetNthByte(I num, size_t n) {
    return static_cast<R>((num >> (8 * n)) & 0xff);
}

constexpr int8_t (*GetNthSignedByte)(uint32_t, size_t) = &GetNthByte<uint32_t, int8_t>;
constexpr uint8_t (*GetNthUnsignedByte)(uint32_t, size_t) = &GetNthByte<uint32_t, uint8_t>;

}  // namespace

Eval::Eval(Manager& manager, diag::List& diagnostics, bool use_runtime_semantics /* = false */)
    : mgr(manager), diags(diagnostics), use_runtime_semantics_(use_runtime_semantics) {}

template <typename T>
Eval::Result Eval::CreateScalar(const Source& source, const core::type::Type* t, T v) {
    static_assert(IsNumber<T> || std::is_same_v<T, bool>, "T must be a Number or bool");
    TINT_ASSERT(t->Is<core::type::Scalar>());

    if constexpr (IsFloatingPoint<T>) {
        if (!std::isfinite(v.value)) {
            AddError(source) << OverflowErrorMessage(v, t->FriendlyName());
            if (use_runtime_semantics_) {
                return mgr.Zero(t);
            } else {
                return error;
            }
        }
    }
    return mgr.Get<Scalar<T>>(t, v);
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Add(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedAdd(a, b)) {
            result = r->value;
        } else {
            AddError(source) << OverflowErrorMessage(a, "+", b);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return error;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto add_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // Ensure no UB for signed overflow
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) + static_cast<UT>(rhs));
            } else {
                return lhs + rhs;
            }
        };
        result = add_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Sub(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedSub(a, b)) {
            result = r->value;
        } else {
            AddError(source) << OverflowErrorMessage(a, "-", b);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return error;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto sub_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // Ensure no UB for signed overflow
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) - static_cast<UT>(rhs));
            } else {
                return lhs - rhs;
            }
        };
        result = sub_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Mul(const Source& source, NumberT a, NumberT b) {
    using T = UnwrapNumber<NumberT>;
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedMul(a, b)) {
            result = r->value;
        } else {
            AddError(source) << OverflowErrorMessage(a, "*", b);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return error;
            }
        }
    } else {
        auto mul_values = [](T lhs, T rhs) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // For signed integrals, avoid C++ UB by multiplying as unsigned
                using UT = std::make_unsigned_t<T>;
                return static_cast<T>(static_cast<UT>(lhs) * static_cast<UT>(rhs));
            } else {
                return lhs * rhs;
            }
        };
        result = mul_values(a.value, b.value);
    }
    return result;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Div(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedDiv(a, b)) {
            result = r->value;
        } else {
            AddError(source) << OverflowErrorMessage(a, "/", b);
            if (use_runtime_semantics_) {
                return a;
            } else {
                return error;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto lhs = a.value;
        auto rhs = b.value;
        if (rhs == 0) {
            // For integers (as for floats), lhs / 0 is an error
            AddError(source) << OverflowErrorMessage(a, "/", b);
            if (use_runtime_semantics_) {
                return a;
            } else {
                return error;
            }
        }
        if constexpr (std::is_signed_v<T>) {
            // For signed integers, lhs / -1 where lhs is the
            // most negative value is an error
            if (rhs == -1 && lhs == std::numeric_limits<T>::min()) {
                AddError(source) << OverflowErrorMessage(a, "/", b);
                if (use_runtime_semantics_) {
                    return a;
                } else {
                    return error;
                }
            }
        }
        result = lhs / rhs;
    }
    return result;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Mod(const Source& source, NumberT a, NumberT b) {
    NumberT result;
    if constexpr (IsAbstract<NumberT> || IsFloatingPoint<NumberT>) {
        if (auto r = CheckedMod(a, b)) {
            result = r->value;
        } else {
            AddError(source) << OverflowErrorMessage(a, "%", b);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return error;
            }
        }
    } else {
        using T = UnwrapNumber<NumberT>;
        auto lhs = a.value;
        auto rhs = b.value;
        if (rhs == 0) {
            // lhs % 0 is an error
            AddError(source) << OverflowErrorMessage(a, "%", b);
            if (use_runtime_semantics_) {
                return NumberT{0};
            } else {
                return error;
            }
        }
        if constexpr (std::is_signed_v<T>) {
            // For signed integers, lhs % -1 where lhs is the
            // most negative value is an error
            if (rhs == -1 && lhs == std::numeric_limits<T>::min()) {
                AddError(source) << OverflowErrorMessage(a, "%", b);
                if (use_runtime_semantics_) {
                    return NumberT{0};
                } else {
                    return error;
                }
            }
        }
        result = lhs % rhs;
    }
    return result;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Dot2(const Source& source,
                                              NumberT a1,
                                              NumberT a2,
                                              NumberT b1,
                                              NumberT b2) {
    auto r1 = Mul(source, a1, b1);
    if (r1 != Success) {
        return error;
    }
    auto r2 = Mul(source, a2, b2);
    if (r2 != Success) {
        return error;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (r != Success) {
        return error;
    }
    return r;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Dot3(const Source& source,
                                              NumberT a1,
                                              NumberT a2,
                                              NumberT a3,
                                              NumberT b1,
                                              NumberT b2,
                                              NumberT b3) {
    auto r1 = Mul(source, a1, b1);
    if (r1 != Success) {
        return error;
    }
    auto r2 = Mul(source, a2, b2);
    if (r2 != Success) {
        return error;
    }
    auto r3 = Mul(source, a3, b3);
    if (r3 != Success) {
        return error;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (r != Success) {
        return error;
    }
    r = Add(source, r.Get(), r3.Get());
    if (r != Success) {
        return error;
    }
    return r;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Dot4(const Source& source,
                                              NumberT a1,
                                              NumberT a2,
                                              NumberT a3,
                                              NumberT a4,
                                              NumberT b1,
                                              NumberT b2,
                                              NumberT b3,
                                              NumberT b4) {
    auto r1 = Mul(source, a1, b1);
    if (r1 != Success) {
        return error;
    }
    auto r2 = Mul(source, a2, b2);
    if (r2 != Success) {
        return error;
    }
    auto r3 = Mul(source, a3, b3);
    if (r3 != Success) {
        return error;
    }
    auto r4 = Mul(source, a4, b4);
    if (r4 != Success) {
        return error;
    }
    auto r = Add(source, r1.Get(), r2.Get());
    if (r != Success) {
        return error;
    }
    r = Add(source, r.Get(), r3.Get());
    if (r != Success) {
        return error;
    }
    r = Add(source, r.Get(), r4.Get());
    if (r != Success) {
        return error;
    }
    return r;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Det2(const Source& source,
                                              NumberT a,
                                              NumberT b,
                                              NumberT c,
                                              NumberT d) {
    // | a c |
    // | b d |
    //
    // =
    //
    // a * d - c * b

    auto r1 = Mul(source, a, d);
    if (r1 != Success) {
        return error;
    }
    auto r2 = Mul(source, c, b);
    if (r2 != Success) {
        return error;
    }
    auto r = Sub(source, r1.Get(), r2.Get());
    if (r != Success) {
        return error;
    }
    return r;
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Det3(const Source& source,
                                              NumberT a,
                                              NumberT b,
                                              NumberT c,
                                              NumberT d,
                                              NumberT e,
                                              NumberT f,
                                              NumberT g,
                                              NumberT h,
                                              NumberT i) {
    // | a d g |
    // | b e h |
    // | c f i |
    //
    // =
    //
    // a | e h | - d | b h | + g | b e |
    //   | f i |     | c i |     | c f |

    auto det1 = Det2(source, e, f, h, i);
    if (det1 != Success) {
        return error;
    }
    auto a_det1 = Mul(source, a, det1.Get());
    if (a_det1 != Success) {
        return error;
    }
    auto det2 = Det2(source, b, c, h, i);
    if (det2 != Success) {
        return error;
    }
    auto d_det2 = Mul(source, d, det2.Get());
    if (d_det2 != Success) {
        return error;
    }
    auto det3 = Det2(source, b, c, e, f);
    if (det3 != Success) {
        return error;
    }
    auto g_det3 = Mul(source, g, det3.Get());
    if (g_det3 != Success) {
        return error;
    }
    auto r = Sub(source, a_det1.Get(), d_det2.Get());
    if (r != Success) {
        return error;
    }
    return Add(source, r.Get(), g_det3.Get());
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Det4(const Source& source,
                                              NumberT a,
                                              NumberT b,
                                              NumberT c,
                                              NumberT d,
                                              NumberT e,
                                              NumberT f,
                                              NumberT g,
                                              NumberT h,
                                              NumberT i,
                                              NumberT j,
                                              NumberT k,
                                              NumberT l,
                                              NumberT m,
                                              NumberT n,
                                              NumberT o,
                                              NumberT p) {
    // | a e i m |
    // | b f j n |
    // | c g k o |
    // | d h l p |
    //
    // =
    //
    // a | f j n | - e | b j n | + i | b f n | - m | b f j |
    //   | g k o |     | c k o |     | c g o |     | c g k |
    //   | h l p |     | d l p |     | d h p |     | d h l |

    auto det1 = Det3(source, f, g, h, j, k, l, n, o, p);
    if (det1 != Success) {
        return error;
    }
    auto a_det1 = Mul(source, a, det1.Get());
    if (a_det1 != Success) {
        return error;
    }
    auto det2 = Det3(source, b, c, d, j, k, l, n, o, p);
    if (det2 != Success) {
        return error;
    }
    auto e_det2 = Mul(source, e, det2.Get());
    if (e_det2 != Success) {
        return error;
    }
    auto det3 = Det3(source, b, c, d, f, g, h, n, o, p);
    if (det3 != Success) {
        return error;
    }
    auto i_det3 = Mul(source, i, det3.Get());
    if (i_det3 != Success) {
        return error;
    }
    auto det4 = Det3(source, b, c, d, f, g, h, j, k, l);
    if (det4 != Success) {
        return error;
    }
    auto m_det4 = Mul(source, m, det4.Get());
    if (m_det4 != Success) {
        return error;
    }
    auto r = Sub(source, a_det1.Get(), e_det2.Get());
    if (r != Success) {
        return error;
    }
    r = Add(source, r.Get(), i_det3.Get());
    if (r != Success) {
        return error;
    }
    return Sub(source, r.Get(), m_det4.Get());
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Sqrt(const Source& source, NumberT v) {
    if (v < NumberT(0)) {
        AddError(source) << "sqrt must be called with a value >= 0";
        if (use_runtime_semantics_) {
            return NumberT{0};
        } else {
            return error;
        }
    }
    return NumberT{std::sqrt(v)};
}

auto Eval::SqrtFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto v) -> Eval::Result {
        if (auto r = Sqrt(source, v); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

template <typename NumberT>
tint::Result<NumberT, Eval::Error> Eval::Clamp(const Source& source,
                                               NumberT e,
                                               NumberT low,
                                               NumberT high) {
    if (low > high) {
        AddError(source) << "clamp called with 'low' (" << low << ") greater than 'high' (" << high
                         << ")";
        if (!use_runtime_semantics_) {
            return error;
        }
    }
    return NumberT{std::min(std::max(e, low), high)};
}

auto Eval::ClampFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto e, auto low, auto high) -> Eval::Result {
        if (auto r = Clamp(source, e, low, high); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::AddFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2) -> Eval::Result {
        if (auto r = Add(source, a1, a2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::SubFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2) -> Eval::Result {
        if (auto r = Sub(source, a1, a2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::MulFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2) -> Eval::Result {
        if (auto r = Mul(source, a1, a2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::DivFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2) -> Eval::Result {
        if (auto r = Div(source, a1, a2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::ModFunc(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2) -> Eval::Result {
        if (auto r = Mod(source, a1, a2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::Dot2Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2, auto b1, auto b2) -> Eval::Result {
        if (auto r = Dot2(source, a1, a2, b1, b2); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::Dot3Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2, auto a3, auto b1, auto b2,
                                   auto b3) -> Eval::Result {
        if (auto r = Dot3(source, a1, a2, a3, b1, b2, b3); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::Dot4Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a1, auto a2, auto a3, auto a4, auto b1, auto b2, auto b3,
                                   auto b4) -> Eval::Result {
        if (auto r = Dot4(source, a1, a2, a3, a4, b1, b2, b3, b4); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

Eval::Result Eval::Dot(const Source& source, const Value* v1, const Value* v2) {
    auto* vec_ty = v1->Type()->As<core::type::Vector>();
    TINT_ASSERT(vec_ty);
    auto* elem_ty = vec_ty->Type();
    switch (vec_ty->Width()) {
        case 2:
            return Dispatch_fia_fiu32_f16(   //
                Dot2Func(source, elem_ty),   //
                v1->Index(0), v1->Index(1),  //
                v2->Index(0), v2->Index(1));
        case 3:
            return Dispatch_fia_fiu32_f16(                 //
                Dot3Func(source, elem_ty),                 //
                v1->Index(0), v1->Index(1), v1->Index(2),  //
                v2->Index(0), v2->Index(1), v2->Index(2));
        case 4:
            return Dispatch_fia_fiu32_f16(                               //
                Dot4Func(source, elem_ty),                               //
                v1->Index(0), v1->Index(1), v1->Index(2), v1->Index(3),  //
                v2->Index(0), v2->Index(1), v2->Index(2), v2->Index(3));
    }
    TINT_ICE() << "Expected vector";
}

Eval::Result Eval::Length(const Source& source, const core::type::Type* ty, const Value* c0) {
    auto* vec_ty = c0->Type()->As<core::type::Vector>();
    // Evaluates to the absolute value of e if T is scalar.
    if (vec_ty == nullptr) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            return CreateScalar(source, ty, NumberT{std::abs(e)});
        };
        return Dispatch_fa_f32_f16(create, c0);
    }

    // Evaluates to sqrt(e[0]^2 + e[1]^2 + ...) if T is a vector type.
    auto d = Dot(source, c0, c0);
    if (d != Success) {
        return error;
    }
    return Dispatch_fa_f32_f16(SqrtFunc(source, ty), d.Get());
}

Eval::Result Eval::Mul(const Source& source,
                       const core::type::Type* ty,
                       const Value* v1,
                       const Value* v2) {
    auto transform = [&](const Value* c0, const Value* c1) {
        return Dispatch_fia_fiu32_f16(MulFunc(source, c0->Type()), c0, c1);
    };
    return TransformBinaryDifferingArityElements(mgr, ty, transform, v1, v2);
}

Eval::Result Eval::Sub(const Source& source,
                       const core::type::Type* ty,
                       const Value* v1,
                       const Value* v2) {
    auto transform = [&](const Value* c0, const Value* c1) {
        return Dispatch_fia_fiu32_f16(SubFunc(source, c0->Type()), c0, c1);
    };
    return TransformBinaryDifferingArityElements(mgr, ty, transform, v1, v2);
}

auto Eval::Det2Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a, auto b, auto c, auto d) -> Eval::Result {
        if (auto r = Det2(source, a, b, c, d); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::Det3Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h,
                                   auto i) -> Eval::Result {
        if (auto r = Det3(source, a, b, c, d, e, f, g, h, i); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

auto Eval::Det4Func(const Source& source, const core::type::Type* elem_ty) {
    return [this, source, elem_ty](auto a, auto b, auto c, auto d, auto e, auto f, auto g, auto h,
                                   auto i, auto j, auto k, auto l, auto m, auto n, auto o,
                                   auto p) -> Eval::Result {
        if (auto r = Det4(source, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); r == Success) {
            return CreateScalar(source, elem_ty, r.Get());
        }
        return error;
    };
}

Eval::Result Eval::ArrayOrStructCtor(const core::type::Type* ty, VectorRef<const Value*> args) {
    if (args.IsEmpty()) {
        return mgr.Zero(ty);
    }

    if (args.Length() == 1 && args[0]->Type() == ty) {
        // Identity constructor.
        return args[0];
    }

    // Multiple arguments. Must be a value constructor.
    return mgr.Composite(ty, std::move(args));
}

Eval::Result Eval::Conv(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto* el_ty = ty->Elements(ty).type;
    if (!el_ty) {
        return nullptr;
    }

    if (!args[0]) {
        return nullptr;  // Single argument is not constant.
    }

    return Convert(ty, args[0], source);
}

Eval::Result Eval::Zero(const core::type::Type* ty, VectorRef<const Value*>, const Source&) {
    return mgr.Zero(ty);
}

Eval::Result Eval::Identity(const core::type::Type*, VectorRef<const Value*> args, const Source&) {
    return args[0];
}

Eval::Result Eval::VecSplat(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source&) {
    if (auto* arg = args[0]) {
        return mgr.Splat(ty, arg);
    }
    return nullptr;
}

Eval::Result Eval::VecInitS(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source&) {
    return mgr.Composite(ty, args);
}

Eval::Result Eval::VecInitM(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source&) {
    Vector<const Value*, 4> els;
    for (auto* arg : args) {
        auto* val = arg;
        if (!val) {
            return nullptr;
        }
        auto* arg_ty = arg->Type();
        if (auto* arg_vec = arg_ty->As<core::type::Vector>()) {
            // Extract out vector elements.
            for (uint32_t j = 0; j < arg_vec->Width(); j++) {
                auto* el = val->Index(j);
                if (!el) {
                    return nullptr;
                }
                els.Push(el);
            }
        } else {
            els.Push(val);
        }
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::MatInitS(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source&) {
    auto* m = static_cast<const core::type::Matrix*>(ty);

    Vector<const Value*, 4> els;
    for (uint32_t c = 0; c < m->Columns(); c++) {
        Vector<const Value*, 4> column;
        for (uint32_t r = 0; r < m->Rows(); r++) {
            auto i = r + c * m->Rows();
            column.Push(args[i]);
        }
        els.Push(mgr.Composite(m->ColumnType(), std::move(column)));
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::MatInitV(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source&) {
    return mgr.Composite(ty, args);
}

Eval::Result Eval::Index(const Value* obj_val,
                         const core::type::Type* obj_ty,
                         const Value* idx_val,
                         const Source& idx_source) {
    // We may not have a Value at this stage but we still want to const evaluate this access based
    // on the type of the access op. An example is here is a failure generated by a compile time
    // negative index array access.
    auto el = obj_val ? obj_val->Type()->Elements() : obj_ty->UnwrapPtrOrRef()->Elements();
    AInt idx = idx_val->ValueAs<AInt>();
    if (idx < 0 || (el.count > 0 && idx >= el.count)) {
        auto& err = AddError(idx_source) << "index " << idx << " out of bounds";
        if (el.count > 0) {
            err << " [0.." + std::to_string(el.count - 1) + "]";
        }

        if (use_runtime_semantics_) {
            return mgr.Zero(el.type);
        } else {
            return error;
        }
    }

    return obj_val ? obj_val->Index(static_cast<size_t>(idx)) : nullptr;
}

Eval::Result Eval::Swizzle(const core::type::Type* ty,
                           const Value* object,
                           VectorRef<uint32_t> indices) {
    if (indices.Length() == 1) {
        return object->Index(static_cast<size_t>(indices[0]));
    }
    auto values = tint::Transform<4>(
        indices, [&](uint32_t i) { return object->Index(static_cast<size_t>(i)); });
    return mgr.Composite(ty, std::move(values));
}

Eval::Result Eval::bitcast(const core::type::Type* ty,
                           VectorRef<const Value*> args,
                           const Source& source) {
    auto* value = args[0];
    bool is_abstract = value->Type()->IsAbstractScalarOrVector();

    // Target type
    auto dst_elements = ty->Elements(ty->DeepestElement(), 1u);
    auto dst_el_ty = dst_elements.type;
    auto dst_count = dst_elements.count;
    // Source type
    auto src_elements = value->Type()->Elements(value->Type()->DeepestElement(), 1u);
    auto src_el_ty = src_elements.type;
    auto src_count = src_elements.count;

    TINT_ASSERT(is_abstract || (dst_count * dst_el_ty->Size() == src_count * src_el_ty->Size()));
    uint32_t total_bitwidth = dst_count * dst_el_ty->Size();
    // Buffer holding the bits from source value, result value reinterpreted from it.
    Vector<std::byte, 16> buffer;
    buffer.Reserve(total_bitwidth);

    // Pushes bits from source value into the buffer.
    auto push_src_element_bits = [&](const Value* element) {
        auto push_32_bits = [&](uint32_t v) {
            buffer.Push(std::byte(v & 0xffu));
            buffer.Push(std::byte((v >> 8) & 0xffu));
            buffer.Push(std::byte((v >> 16) & 0xffu));
            buffer.Push(std::byte((v >> 24) & 0xffu));
            return Success;
        };
        auto push_16_bits = [&](uint16_t v) {
            buffer.Push(std::byte(v & 0xffu));
            buffer.Push(std::byte((v >> 8) & 0xffu));
            return Success;
        };
        return Switch(
            src_el_ty,
            [&](const core::type::AbstractInt*) -> tint::Result<SuccessType, Error> {
                if (element->ValueAs<AInt>() < 0) {
                    auto res = Conv(mgr.types.i32(), Vector{element}, source);
                    if (res != Success) {
                        return res.Failure();
                    }
                    return push_32_bits(tint::Bitcast<u32>(res.Get()->ValueAs<i32>()));
                } else {
                    auto res = Conv(mgr.types.u32(), Vector{element}, source);
                    if (res != Success) {
                        return res.Failure();
                    }
                    return push_32_bits(res.Get()->ValueAs<u32>());
                }
            },
            [&](const core::type::U32*) -> tint::Result<SuccessType, Error> {
                return push_32_bits(element->ValueAs<u32>());
            },
            [&](const core::type::I32*) -> tint::Result<SuccessType, Error> {
                return push_32_bits(tint::Bitcast<u32>(element->ValueAs<i32>()));
            },
            [&](const core::type::F32*) -> tint::Result<SuccessType, Error> {
                return push_32_bits(tint::Bitcast<u32>(element->ValueAs<f32>()));
            },
            [&](const core::type::F16*) -> tint::Result<SuccessType, Error> {
                return push_16_bits(element->ValueAs<f16>().BitsRepresentation());
            },
            TINT_ICE_ON_NO_MATCH);
    };
    if (src_count == 1) {
        if (auto res = push_src_element_bits(value); res != Success) {
            return res.Failure();
        }
    } else {
        for (size_t i = 0; i < src_count; i++) {
            if (auto res = push_src_element_bits(value->Index(i)); res != Success) {
                return res.Failure();
            }
        }
    }

    // Vector holding elements of return value
    Vector<const Value*, 4> els;

    // Reinterprets the buffer bits as destination element and push the result into the vector.
    // Return false if an error occurred, otherwise return true.
    auto push_dst_element = [&](size_t offset) -> bool {
        uint32_t v;
        if (dst_el_ty->Size() == 4) {
            v = (std::to_integer<uint32_t>(buffer[offset])) |
                (std::to_integer<uint32_t>(buffer[offset + 1]) << 8) |
                (std::to_integer<uint32_t>(buffer[offset + 2]) << 16) |
                (std::to_integer<uint32_t>(buffer[offset + 3]) << 24);
        } else {
            v = (std::to_integer<uint32_t>(buffer[offset])) |
                (std::to_integer<uint32_t>(buffer[offset + 1]) << 8);
        }

        return Switch(
            dst_el_ty,
            [&](const core::type::U32*) {  //
                auto r = CreateScalar(source, dst_el_ty, u32(v));
                if (r != Success) {
                    return false;
                }
                els.Push(r.Get());
                return true;
            },
            [&](const core::type::I32*) {  //
                auto r = CreateScalar(source, dst_el_ty, tint::Bitcast<i32>(v));
                if (r != Success) {
                    return false;
                }
                els.Push(r.Get());
                return true;
            },
            [&](const core::type::F32*) {  //
                auto r = CreateScalar(source, dst_el_ty, tint::Bitcast<f32>(v));
                if (r != Success) {
                    return false;
                }
                els.Push(r.Get());
                return true;
            },
            [&](const core::type::F16*) {  //
                auto r = CreateScalar(source, dst_el_ty, f16::FromBits(static_cast<uint16_t>(v)));
                if (r != Success) {
                    return false;
                }
                els.Push(r.Get());
                return true;
            },
            TINT_ICE_ON_NO_MATCH);
    };

    TINT_ASSERT((buffer.Length() == total_bitwidth));
    for (size_t i = 0; i < dst_count; i++) {
        if (!push_dst_element(i * dst_el_ty->Size())) {
            return error;
        }
    }

    if (dst_count == 1) {
        return std::move(els[0]);
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::Complement(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto transform = [&](const Value* c) {
        auto create = [&](auto i) {
            return CreateScalar(source, c->Type(), decltype(i)(~i.value));
        };
        return Dispatch_ia_iu32(create, c);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::UnaryMinus(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto transform = [&](const Value* c) {
        auto create = [&](auto i) {
            // For signed integrals, avoid C++ UB by not negating the
            // smallest negative number. In WGSL, this operation is well
            // defined to return the same value, see:
            // https://gpuweb.github.io/gpuweb/wgsl/#arithmetic-expr.
            using T = UnwrapNumber<decltype(i)>;
            if constexpr (std::is_integral_v<T>) {
                auto v = i.value;
                if (v != std::numeric_limits<T>::min()) {
                    v = -v;
                }
                return CreateScalar(source, c->Type(), decltype(i)(v));
            } else {
                return CreateScalar(source, c->Type(), decltype(i)(-i.value));
            }
        };
        return Dispatch_fia_fi32_f16(create, c);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::Not(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c) {
        auto create = [&](auto i) { return CreateScalar(source, c->Type(), decltype(i)(!i)); };
        return Dispatch_bool(create, c);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::Plus(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        return Dispatch_fia_fiu32_f16(AddFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryDifferingArityElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::Minus(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    return Sub(source, ty, args[0], args[1]);
}

Eval::Result Eval::Multiply(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    return Mul(source, ty, args[0], args[1]);
}

Eval::Result Eval::MultiplyMatVec(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto* mat_ty = args[0]->Type()->As<core::type::Matrix>();
    auto* vec_ty = args[1]->Type()->As<core::type::Vector>();
    auto* elem_ty = vec_ty->Type();

    auto dot = [&](const Value* m, size_t row, const Value* v) {
        Eval::Result result;
        switch (mat_ty->Columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             m->Index(2)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1), v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m->Index(0)->Index(row),    //
                                             m->Index(1)->Index(row),    //
                                             m->Index(2)->Index(row),    //
                                             m->Index(3)->Index(row),    //
                                             v->Index(0),                //
                                             v->Index(1),                //
                                             v->Index(2),                //
                                             v->Index(3));
                break;
        }
        return result;
    };

    Vector<const Value*, 4> result;
    for (size_t i = 0; i < mat_ty->Rows(); ++i) {
        auto r = dot(args[0], i, args[1]);  // matrix row i * vector
        if (r != Success) {
            return error;
        }
        result.Push(r.Get());
    }
    return mgr.Composite(ty, result);
}
Eval::Result Eval::MultiplyVecMat(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto* vec_ty = args[0]->Type()->As<core::type::Vector>();
    auto* mat_ty = args[1]->Type()->As<core::type::Matrix>();
    auto* elem_ty = vec_ty->Type();

    auto dot = [&](const Value* v, const Value* m, size_t col) {
        Eval::Result result;
        switch (mat_ty->Rows()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             v->Index(0),                //
                                             v->Index(1));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             m->Index(col)->Index(2),
                                             v->Index(0),  //
                                             v->Index(1),  //
                                             v->Index(2));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m->Index(col)->Index(0),    //
                                             m->Index(col)->Index(1),    //
                                             m->Index(col)->Index(2),    //
                                             m->Index(col)->Index(3),    //
                                             v->Index(0),                //
                                             v->Index(1),                //
                                             v->Index(2),                //
                                             v->Index(3));
        }
        return result;
    };

    Vector<const Value*, 4> result;
    for (size_t i = 0; i < mat_ty->Columns(); ++i) {
        auto r = dot(args[0], args[1], i);  // vector * matrix col i
        if (r != Success) {
            return error;
        }
        result.Push(r.Get());
    }
    return mgr.Composite(ty, result);
}

Eval::Result Eval::MultiplyMatMat(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto* mat1 = args[0];
    auto* mat2 = args[1];
    auto* mat1_ty = mat1->Type()->As<core::type::Matrix>();
    auto* mat2_ty = mat2->Type()->As<core::type::Matrix>();
    auto* elem_ty = mat1_ty->Type();

    auto dot = [&](const Value* m1, size_t row, const Value* m2, size_t col) {
        auto m1e = [&](size_t r, size_t c) { return m1->Index(c)->Index(r); };
        auto m2e = [&](size_t r, size_t c) { return m2->Index(c)->Index(r); };

        Eval::Result result;
        switch (mat1_ty->Columns()) {
            case 2:
                result = Dispatch_fa_f32_f16(Dot2Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m2e(0, col),                //
                                             m2e(1, col));
                break;
            case 3:
                result = Dispatch_fa_f32_f16(Dot3Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m1e(row, 2),                //
                                             m2e(0, col),                //
                                             m2e(1, col),                //
                                             m2e(2, col));
                break;
            case 4:
                result = Dispatch_fa_f32_f16(Dot4Func(source, elem_ty),  //
                                             m1e(row, 0),                //
                                             m1e(row, 1),                //
                                             m1e(row, 2),                //
                                             m1e(row, 3),                //
                                             m2e(0, col),                //
                                             m2e(1, col),                //
                                             m2e(2, col),                //
                                             m2e(3, col));
                break;
        }
        return result;
    };

    Vector<const Value*, 4> result_mat;
    for (size_t c = 0; c < mat2_ty->Columns(); ++c) {
        Vector<const Value*, 4> col_vec;
        for (size_t r = 0; r < mat1_ty->Rows(); ++r) {
            auto v = dot(mat1, r, mat2, c);  // mat1 row r * mat2 col c
            if (v != Success) {
                return error;
            }
            col_vec.Push(v.Get());  // mat1 row r * mat2 col c
        }

        // Add column vector to matrix
        auto* col_vec_ty = ty->As<core::type::Matrix>()->ColumnType();
        result_mat.Push(mgr.Composite(col_vec_ty, col_vec));
    }
    return mgr.Composite(ty, result_mat);
}

Eval::Result Eval::Divide(const core::type::Type* ty,
                          VectorRef<const Value*> args,
                          const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        return Dispatch_fia_fiu32_f16(DivFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryDifferingArityElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::Modulo(const core::type::Type* ty,
                          VectorRef<const Value*> args,
                          const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        return Dispatch_fia_fiu32_f16(ModFunc(source, c0->Type()), c0, c1);
    };

    return TransformBinaryDifferingArityElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::Equal(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i == j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::NotEqual(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i != j);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::LessThan(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i < j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::GreaterThan(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i > j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::LessThanEqual(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i <= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::GreaterThanEqual(const core::type::Type* ty,
                                    VectorRef<const Value*> args,
                                    const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), i >= j);
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::LogicalAnd(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    // Due to short-circuiting, this function is only called if lhs is true, so we only return the
    // value of the rhs.
    TINT_ASSERT(args[0]->ValueAs<bool>());
    return CreateScalar(source, ty, args[1]->ValueAs<bool>());
}

Eval::Result Eval::LogicalOr(const core::type::Type* ty,
                             VectorRef<const Value*> args,
                             const Source& source) {
    // Due to short-circuiting, this function is only called if lhs is false, so we only only return
    // the value of the rhs.
    TINT_ASSERT(!args[0]->ValueAs<bool>());
    return CreateScalar(source, ty, args[1]->ValueAs<bool>());
}

Eval::Result Eval::And(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i && j;
            } else {  // integral
                result = i & j;
            }
            return CreateScalar(source, ty->DeepestElement(), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::Or(const core::type::Type* ty,
                      VectorRef<const Value*> args,
                      const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            using T = decltype(i);
            T result;
            if constexpr (std::is_same_v<T, bool>) {
                result = i || j;
            } else {  // integral
                result = i | j;
            }
            return CreateScalar(source, ty->DeepestElement(), result);
        };
        return Dispatch_ia_iu32_bool(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::Xor(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), decltype(i){i ^ j});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::ShiftLeft(const core::type::Type* ty,
                             VectorRef<const Value*> args,
                             const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto e1, auto e2) -> Eval::Result {
            using NumberT = decltype(e1);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr size_t bit_width = BitWidth<NumberT>;
            UT e1u = static_cast<UT>(e1);
            UT e2u = static_cast<UT>(e2);

            if constexpr (IsAbstract<NumberT>) {
                // The e2 + 1 most significant bits of e1 must have the same bit value, otherwise
                // sign change (overflow) would occur.
                // Check sign change only if e2 is less than bit width of e1. If e1 is larger
                // than bit width, we check for non-representable value below.
                if (e2u < bit_width) {
                    UT must_match_msb = e2u + 1;
                    UT mask = ~UT{0} << (bit_width - must_match_msb);
                    if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                        AddError(source) << "shift left operation results in sign change";
                        if (!use_runtime_semantics_) {
                            return error;
                        }
                    }
                } else {
                    // If shift value >= bit_width, then any non-zero value would overflow
                    if (e1 != 0) {
                        AddError(source) << OverflowErrorMessage(e1, "<<", e2);
                        if (!use_runtime_semantics_) {
                            return error;
                        }
                    }

                    // It's UB in C++ to shift by greater or equal to the bit width (even if the lhs
                    // is 0), so we make sure to avoid this by setting the shift value to 0.
                    e2u = 0;
                }
            } else {
                if (static_cast<size_t>(e2) >= bit_width && use_runtime_semantics_) {
                    // At shader/pipeline-creation time, it is an error to shift by the bit width of
                    // the lhs or greater, which should have already been caught by the validator.
                    // At runtime, we shift by e2 % (bit width of e1).
                    AddError(source)
                        << "shift left value must be less than the bit width of the lhs, which is "
                        << bit_width;
                    e2u = e2u % bit_width;
                }

                if constexpr (std::is_signed_v<T>) {
                    // If T is a signed integer type, and the e2+1 most significant bits of e1 do
                    // not have the same bit value, then error.
                    size_t must_match_msb = e2u + 1;
                    UT mask = ~UT{0} << (bit_width - must_match_msb);
                    if ((e1u & mask) != 0 && (e1u & mask) != mask) {
                        AddError(source) << "shift left operation results in sign change";
                        if (!use_runtime_semantics_) {
                            return error;
                        }
                    }
                } else {
                    // If T is an unsigned integer type, and any of the e2 most significant bits of
                    // e1 are 1, then error.
                    if (e2u > 0) {
                        size_t must_be_zero_msb = e2u;
                        UT mask = ~UT{0} << (bit_width - must_be_zero_msb);
                        if ((e1u & mask) != 0) {
                            AddError(source) << OverflowErrorMessage(e1, "<<", e2);
                            if (!use_runtime_semantics_) {
                                return error;
                            }
                        }
                    }
                }
            }

            // Avoid UB by left shifting as unsigned value
            auto result = static_cast<T>(static_cast<UT>(e1) << e2u);
            return CreateScalar(source, ty->DeepestElement(), NumberT{result});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    if (DAWN_UNLIKELY(!args[1]->Type()->DeepestElement()->Is<core::type::U32>())) {
        TINT_ICE() << "Element type of rhs of ShiftLeft must be a u32";
    }

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::ShiftRight(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto e1, auto e2) -> Eval::Result {
            using NumberT = decltype(e1);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            const size_t bit_width = BitWidth<NumberT>;
            UT e1u = static_cast<UT>(e1);
            UT e2u = static_cast<UT>(e2);

            [[maybe_unused]] auto signed_shift_right = [&] {
                // In C++, right shift of a signed negative number is implementation-defined.
                // Although most implementations sign-extend, we do it manually to ensure it works
                // correctly on all implementations.
                const UT msb = UT{1} << (bit_width - 1);
                UT sign_ext = 0;
                if (e1u & msb) {
                    // Set e2 + 1 bits to 1
                    UT num_shift_bits_mask = ((UT{1} << e2u) - UT{1});
                    sign_ext = (num_shift_bits_mask << (bit_width - e2u - UT{1})) | msb;
                }
                return static_cast<T>((e1u >> e2u) | sign_ext);
            };

            T result = 0;
            if constexpr (IsAbstract<NumberT>) {
                if (static_cast<size_t>(e2) >= bit_width) {
                    // For an abstract shift right, if e1 is negative, each inserted bit is 1,
                    // resulting in the value -1 for all 1s. For a non-negative e1, each inserted
                    // bit is 0, resulting in 0.
                    result = e1 < 0 ? T{-1} : T{0};
                } else {
                    result = signed_shift_right();
                }
            } else {
                if (static_cast<size_t>(e2) >= bit_width && use_runtime_semantics_) {
                    // At shader/pipeline-creation time, it is an error to shift by the bit width of
                    // the lhs or greater, which should have already been caught by the validator.
                    // At runtime, we shift by e2 % (bit width of e1).
                    AddError(source)
                        << "shift right value must be less than the bit width of the lhs, which is "
                        << bit_width;
                    e2u = e2u % bit_width;
                }

                if constexpr (std::is_signed_v<T>) {
                    result = signed_shift_right();
                } else {
                    result = e1 >> e2u;
                }
            }
            return CreateScalar(source, ty->DeepestElement(), NumberT{result});
        };
        return Dispatch_ia_iu32(create, c0, c1);
    };

    if (DAWN_UNLIKELY(!args[1]->Type()->DeepestElement()->Is<core::type::U32>())) {
        TINT_ICE() << "Element type of rhs of ShiftLeft must be a u32";
    }

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::abs(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            NumberT result;
            if constexpr (IsUnsignedIntegral<NumberT>) {
                result = e;
            } else if constexpr (IsSignedIntegral<NumberT>) {
                if (e == NumberT::Lowest()) {
                    result = e;
                } else {
                    result = NumberT{std::abs(e)};
                }
            } else {
                result = NumberT{std::abs(e)};
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fia_fiu32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::acos(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform =
        [&](const Value* c0) {
            auto create = [&](auto i) -> Eval::Result {
                using NumberT = decltype(i);
                if (i < NumberT(-1.0) || i > NumberT(1.0)) {
                    AddError(source)
                        << "acos must be called with a value in the range [-1 .. 1] (inclusive)";
                    if (use_runtime_semantics_) {
                        return mgr.Zero(c0->Type());
                    } else {
                        return error;
                    }
                }
                return CreateScalar(source, c0->Type(), NumberT(std::acos(i.value)));
            };
            return Dispatch_fa_f32_f16(create, c0);
        };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::acosh(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            if (i < NumberT(1.0)) {
                AddError(source) << "acosh must be called with a value >= 1.0";
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::acosh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::all(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    return CreateScalar(source, ty, !args[0]->AnyZero());
}

Eval::Result Eval::any(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    return CreateScalar(source, ty, !args[0]->AllZero());
}

Eval::Result Eval::asin(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform =
        [&](const Value* c0) {
            auto create = [&](auto i) -> Eval::Result {
                using NumberT = decltype(i);
                if (i < NumberT(-1.0) || i > NumberT(1.0)) {
                    AddError(source)
                        << "asin must be called with a value in the range [-1 .. 1] (inclusive)";
                    if (use_runtime_semantics_) {
                        return mgr.Zero(c0->Type());
                    } else {
                        return error;
                    }
                }
                return CreateScalar(source, c0->Type(), NumberT(std::asin(i.value)));
            };
            return Dispatch_fa_f32_f16(create, c0);
        };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::asinh(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::asinh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::atan(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::atan(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::atanh(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform =
        [&](const Value* c0) {
            auto create = [&](auto i) -> Eval::Result {
                using NumberT = decltype(i);
                if (i <= NumberT(-1.0) || i >= NumberT(1.0)) {
                    AddError(source)
                        << "atanh must be called with a value in the range (-1 .. 1) (exclusive)";
                    if (use_runtime_semantics_) {
                        return mgr.Zero(c0->Type());
                    } else {
                        return error;
                    }
                }
                return CreateScalar(source, c0->Type(), NumberT(std::atanh(i.value)));
            };
            return Dispatch_fa_f32_f16(create, c0);
        };

    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::atan2(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto i, auto j) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::atan2(i.value, j.value)));
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::ceil(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            return CreateScalar(source, c0->Type(), decltype(e)(std::ceil(e)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::clamp(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1, const Value* c2) {
        return Dispatch_fia_fiu32_f16(ClampFunc(source, c0->Type()), c0, c1, c2);
    };
    return TransformTernaryElements(mgr, ty, transform, args[0], args[1], args[2]);
}

Eval::Result Eval::cos(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::cos(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::cosh(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::cosh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::countLeadingZeros(const core::type::Type* ty,
                                     VectorRef<const Value*> args,
                                     const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            auto count = CountLeadingBits(T{e}, T{0});
            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::countOneBits(const core::type::Type* ty,
                                VectorRef<const Value*> args,
                                const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kRightMost = UT{1};

            auto count = UT{0};
            for (auto v = static_cast<UT>(e); v != UT{0}; v >>= 1) {
                if ((v & kRightMost) == 1) {
                    ++count;
                }
            }

            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::countTrailingZeros(const core::type::Type* ty,
                                      VectorRef<const Value*> args,
                                      const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            auto count = CountTrailingBits(T{e}, T{0});
            return CreateScalar(source, c0->Type(), NumberT(count));
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::cross(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto* u = args[0];
    auto* v = args[1];
    auto* elem_ty = u->Type()->As<core::type::Vector>()->Type();

    // cross product of a v3 is the determinant of the 3x3 matrix:
    //
    // |i   j   k |
    // |u0  u1  u2|
    // |v0  v1  v2|
    //
    // |u1 u2|i  - |u0 u2|j + |u0 u1|k
    // |v1 v2|     |v0 v2|    |v0 v1|
    //
    // |u1 u2|i  + |v0 v2|j + |u0 u1|k
    // |v1 v2|     |u0 u2|    |v0 v1|

    auto* u0 = u->Index(0);
    auto* u1 = u->Index(1);
    auto* u2 = u->Index(2);
    auto* v0 = v->Index(0);
    auto* v1 = v->Index(1);
    auto* v2 = v->Index(2);

    auto x = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), u1, u2, v1, v2);
    if (x != Success) {
        return error;
    }
    auto y = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), v0, v2, u0, u2);
    if (y != Success) {
        return error;
    }
    auto z = Dispatch_fa_f32_f16(Det2Func(source, elem_ty), u0, u1, v0, v1);
    if (z != Success) {
        return error;
    }

    return mgr.Composite(ty, Vector<const Value*, 3>{x.Get(), y.Get(), z.Get()});
}

Eval::Result Eval::degrees(const core::type::Type* ty,
                           VectorRef<const Value*> args,
                           const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) -> Eval::Result {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto scale = Div(source, NumberT(180), NumberT(std::numbers::pi_v<T>));
            if (scale != Success) {
                AddNote(source) << "when calculating degrees";
                return error;
            }
            auto result = Mul(source, e, scale.Get());
            if (result != Success) {
                AddNote(source) << "when calculating degrees";
                return error;
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::determinant(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto calculate = [&]() -> Eval::Result {
        auto* m = args[0];
        auto* mat_ty = m->Type()->As<core::type::Matrix>();
        auto me = [&](size_t r, size_t c) { return m->Index(c)->Index(r); };
        switch (mat_ty->Rows()) {
            case 2:
                return Dispatch_fa_f32_f16(Det2Func(source, ty),  //
                                           me(0, 0), me(1, 0),    //
                                           me(0, 1), me(1, 1));

            case 3:
                return Dispatch_fa_f32_f16(Det3Func(source, ty),          //
                                           me(0, 0), me(1, 0), me(2, 0),  //
                                           me(0, 1), me(1, 1), me(2, 1),  //
                                           me(0, 2), me(1, 2), me(2, 2));

            case 4:
                return Dispatch_fa_f32_f16(Det4Func(source, ty),                    //
                                           me(0, 0), me(1, 0), me(2, 0), me(3, 0),  //
                                           me(0, 1), me(1, 1), me(2, 1), me(3, 1),  //
                                           me(0, 2), me(1, 2), me(2, 2), me(3, 2),  //
                                           me(0, 3), me(1, 3), me(2, 3), me(3, 3));
        }
        TINT_ICE() << "Unexpected number of matrix rows";
    };
    auto r = calculate();
    if (r != Success) {
        AddNote(source) << "when calculating determinant";
    }
    return r;
}

Eval::Result Eval::distance(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto err = [&]() -> Eval::Result {
        AddNote(source) << "when calculating distance";
        return error;
    };

    auto minus = Minus(args[0]->Type(), args, source);
    if (minus != Success) {
        return err();
    }

    auto len = Length(source, ty, minus.Get());
    if (len != Success) {
        return err();
    }
    return len;
}

Eval::Result Eval::dot(const core::type::Type*,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto r = Dot(source, args[0], args[1]);
    if (r != Success) {
        AddNote(source) << "when calculating dot";
    }
    return r;
}

Eval::Result Eval::dot4I8Packed(const core::type::Type* ty,
                                VectorRef<const Value*> args,
                                const Source& source) {
    uint32_t packed_int8_vec4_1 = args[0]->ValueAs<u32>();
    uint32_t packed_int8_vec4_2 = args[1]->ValueAs<u32>();

    int32_t result = 0;
    for (size_t i = 0; i < 4; i++) {
        result += GetNthSignedByte(packed_int8_vec4_1, i) * GetNthSignedByte(packed_int8_vec4_2, i);
    }

    return CreateScalar(source, ty, i32(result));
}

Eval::Result Eval::dot4U8Packed(const core::type::Type* ty,
                                VectorRef<const Value*> args,
                                const Source& source) {
    uint32_t packed_uint8_vec4_1 = args[0]->ValueAs<u32>();
    uint32_t packed_uint8_vec4_2 = args[1]->ValueAs<u32>();

    uint32_t result = 0;
    for (size_t i = 0; i < 4; i++) {
        result +=
            GetNthUnsignedByte(packed_uint8_vec4_1, i) * GetNthUnsignedByte(packed_uint8_vec4_2, i);
    }

    return CreateScalar(source, ty, u32(result));
}

Eval::Result Eval::exp(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e0) -> Eval::Result {
            using NumberT = decltype(e0);
            auto val = NumberT(std::exp(e0));
            if (!std::isfinite(val.value)) {
                AddError(source) << OverflowExpErrorMessage("e", e0);
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), val);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::exp2(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e0) -> Eval::Result {
            using NumberT = decltype(e0);
            auto val = NumberT(std::exp2(e0));
            if (!std::isfinite(val.value)) {
                AddError(source) << OverflowExpErrorMessage("2", e0);
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), val);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::extractBits(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto transform =
        [&](const Value* c0) {
            auto create = [&](auto in_e) -> Eval::Result {
                using NumberT = decltype(in_e);
                using T = UnwrapNumber<NumberT>;
                using UT = std::make_unsigned_t<T>;
                using NumberUT = Number<UT>;

                // Read args that are always scalar
                NumberUT in_offset = args[1]->ValueAs<NumberUT>();
                NumberUT in_count = args[2]->ValueAs<NumberUT>();

                // Cast all to unsigned
                UT e = static_cast<UT>(in_e);
                UT o = static_cast<UT>(in_offset);
                UT c = static_cast<UT>(in_count);

                constexpr UT w = sizeof(UT) * 8;
                if (o > w || c > w || (o + c) > w) {
                    AddError(source)
                        << "'offset' + 'count' must be less than or equal to the bit width of 'e'";
                    if (use_runtime_semantics_) {
                        o = std::min(o, w);
                        c = std::min(c, w - o);
                    } else {
                        return error;
                    }
                }

                NumberT result;
                if (c == UT{0}) {
                    // The result is 0 if c is 0
                    result = NumberT{0};
                } else if (c == w) {
                    // The result is e if c is w
                    result = NumberT{e};
                } else {
                    // Otherwise, bits 0..c - 1 of the result are copied from bits o..o + c - 1 of
                    // e.
                    UT src_mask = ((UT{1} << c) - UT{1}) << o;
                    UT r = (e & src_mask) >> o;
                    if constexpr (IsSignedIntegral<NumberT>) {
                        // Other bits of the result are the same as bit c - 1 of the result.
                        // Only need to set other bits if bit at c - 1 of result is 1
                        if ((r & (UT{1} << (c - UT{1}))) != UT{0}) {
                            UT dst_mask = src_mask >> o;
                            r |= (~UT{0} & ~dst_mask);
                        }
                    }

                    result = NumberT{r};
                }
                return CreateScalar(source, c0->Type(), result);
            };
            return Dispatch_iu32(create, c0);
        };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::faceForward(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    // Returns e1 if dot(e2, e3) is negative, and -e1 otherwise.
    auto* e1 = args[0];
    auto* e2 = args[1];
    auto* e3 = args[2];
    auto r = Dot(source, e2, e3);
    if (r != Success) {
        AddNote(source) << "when calculating faceForward";
        return error;
    }
    auto is_negative = [](auto v) { return v < 0; };
    if (Dispatch_fa_f32_f16(is_negative, r.Get())) {
        return e1;
    }
    return UnaryMinus(ty, Vector{e1}, source);
}

Eval::Result Eval::firstLeadingBit(const core::type::Type* ty,
                                   VectorRef<const Value*> args,
                                   const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kNumBits = sizeof(UT) * 8;

            NumberT result;
            if constexpr (IsUnsignedIntegral<T>) {
                if (e == T{0}) {
                    // T(-1) if e is zero.
                    result = NumberT(static_cast<T>(-1));
                } else {
                    // Otherwise the position of the most significant 1 bit in e.
                    static_assert(std::is_same_v<T, UT>);
                    UT count = CountLeadingBits(UT{e}, UT{0});
                    UT pos = kNumBits - count - 1;
                    result = NumberT(pos);
                }
            } else {
                if (e == T{0} || e == T{-1}) {
                    // -1 if e is 0 or -1.
                    result = NumberT(-1);
                } else {
                    // Otherwise the position of the most significant bit in e that is different
                    // from e's sign bit.
                    UT eu = static_cast<UT>(e);
                    UT sign_bit = eu >> (kNumBits - 1);
                    UT count = CountLeadingBits(eu, sign_bit);
                    UT pos = kNumBits - count - 1;
                    result = NumberT(pos);
                }
            }

            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::firstTrailingBit(const core::type::Type* ty,
                                    VectorRef<const Value*> args,
                                    const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;

            NumberT result;
            if (e == T{0}) {
                // T(-1) if e is zero.
                result = NumberT(static_cast<T>(-1));
            } else {
                // Otherwise the position of the least significant 1 bit in e.
                UT pos = CountTrailingBits(T{e}, T{0});
                result = NumberT(pos);
            }

            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::floor(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            return CreateScalar(source, c0->Type(), decltype(e)(std::floor(e)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::fma(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c1, const Value* c2, const Value* c3) {
        auto create = [&](auto e1, auto e2, auto e3) -> Eval::Result {
            auto err_msg = [&] {
                AddNote(source) << "when calculating fma";
                return error;
            };

            auto mul = Mul(source, e1, e2);
            if (mul != Success) {
                return err_msg();
            }

            auto val = Add(source, mul.Get(), e3);
            if (val != Success) {
                return err_msg();
            }
            return CreateScalar(source, c1->Type(), val.Get());
        };
        return Dispatch_fa_f32_f16(create, c1, c2, c3);
    };
    return TransformTernaryElements(mgr, ty, transform, args[0], args[1], args[2]);
}

Eval::Result Eval::fract(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c1) {
        auto create = [&](auto e) -> Eval::Result {
            using NumberT = decltype(e);
            auto r = e - std::floor(e);
            return CreateScalar(source, c1->Type(), NumberT{r});
        };
        return Dispatch_fa_f32_f16(create, c1);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::frexp(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto* arg = args[0];

    struct FractExp {
        Eval::Result fract;
        Eval::Result exp;
    };

    auto scalar = [&](const Value* s) {
        int exp = 0;
        double fract = std::frexp(s->ValueAs<AFloat>(), &exp);
        return Switch(
            s->Type(),
            [&](const core::type::F32*) {
                return FractExp{
                    CreateScalar(source, mgr.types.f32(), f32(fract)),
                    CreateScalar(source, mgr.types.i32(), i32(exp)),
                };
            },
            [&](const core::type::F16*) {
                return FractExp{
                    CreateScalar(source, mgr.types.f16(), f16(fract)),
                    CreateScalar(source, mgr.types.i32(), i32(exp)),
                };
            },
            [&](const core::type::AbstractFloat*) {
                return FractExp{
                    CreateScalar(source, mgr.types.AFloat(), AFloat(fract)),
                    CreateScalar(source, mgr.types.AInt(), AInt(exp)),
                };
            },
            TINT_ICE_ON_NO_MATCH);
    };

    if (auto* vec = arg->Type()->As<core::type::Vector>()) {
        Vector<const Value*, 4> fract_els;
        Vector<const Value*, 4> exp_els;
        for (uint32_t i = 0; i < vec->Width(); i++) {
            auto fe = scalar(arg->Index(i));
            if (fe.fract != Success || fe.exp != Success) {
                return error;
            }
            fract_els.Push(fe.fract.Get());
            exp_els.Push(fe.exp.Get());
        }
        auto fract_ty = mgr.types.vec(fract_els[0]->Type(), vec->Width());
        auto exp_ty = mgr.types.vec(exp_els[0]->Type(), vec->Width());
        return mgr.Composite(ty, Vector<const Value*, 2>{
                                     mgr.Composite(fract_ty, std::move(fract_els)),
                                     mgr.Composite(exp_ty, std::move(exp_els)),
                                 });
    } else {
        auto fe = scalar(arg);
        if (fe.fract != Success || fe.exp != Success) {
            return error;
        }
        return mgr.Composite(ty, Vector<const Value*, 2>{
                                     fe.fract.Get(),
                                     fe.exp.Get(),
                                 });
    }
}

Eval::Result Eval::insertBits(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto transform =
        [&](const Value* c0, const Value* c1) {
            auto create = [&](auto in_e, auto in_newbits) -> Eval::Result {
                using NumberT = decltype(in_e);
                using T = UnwrapNumber<NumberT>;
                using UT = std::make_unsigned_t<T>;
                using NumberUT = Number<UT>;

                // Read args that are always scalar
                NumberUT in_offset = args[2]->ValueAs<NumberUT>();
                NumberUT in_count = args[3]->ValueAs<NumberUT>();

                // Cast all to unsigned
                UT e = static_cast<UT>(in_e);
                UT newbits = static_cast<UT>(in_newbits);
                UT o = static_cast<UT>(in_offset);
                UT c = static_cast<UT>(in_count);

                constexpr UT w = sizeof(UT) * 8;
                if (o > w || c > w || (o + c) > w) {
                    AddError(source)
                        << "'offset' + 'count' must be less than or equal to the bit width of 'e'";
                    if (use_runtime_semantics_) {
                        o = std::min(o, w);
                        c = std::min(c, w - o);
                    } else {
                        return error;
                    }
                }

                NumberT result;
                if (c == UT{0}) {
                    // The result is e if c is 0
                    result = NumberT{e};
                } else if (c == w) {
                    // The result is newbits if c is w
                    result = NumberT{newbits};
                } else {
                    // Otherwise, bits o..o + c - 1 of the result are copied from bits 0..c - 1 of
                    // newbits. Other bits of the result are copied from e.
                    UT from = newbits << o;
                    UT mask = ((UT{1} << c) - UT{1}) << UT{o};
                    auto r = e;          // Start with 'e' as the result
                    r &= ~mask;          // Zero the bits in 'e' we're overwriting
                    r |= (from & mask);  // Overwrite from 'newbits' (shifted into position)
                    result = NumberT{r};
                }

                return CreateScalar(source, c0->Type(), result);
            };
            return Dispatch_iu32(create, c0, c1);
        };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::inverseSqrt(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) -> Eval::Result {
            using NumberT = decltype(e);

            if (e <= NumberT(0)) {
                AddError(source) << "inverseSqrt must be called with a value > 0";
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }

            auto err = [&] {
                AddNote(source) << "when calculating inverseSqrt";
                return error;
            };

            auto s = Sqrt(source, e);
            if (s != Success) {
                return err();
            }
            auto div = Div(source, NumberT(1), s.Get());
            if (div != Success) {
                return err();
            }

            return CreateScalar(source, c0->Type(), div.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };

    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::ldexp(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c1, size_t index) {
        auto create = [&](auto e1) -> Eval::Result {
            using E1Type = decltype(e1);
            // If e1 is AFloat, then e2 is AInt, otherwise it's i32
            using E2Type = std::conditional_t<std::is_same_v<E1Type, AFloat>, AInt, i32>;

            E2Type e2;
            auto* c2 = args[1];
            if (c2->Type()->Is<core::type::Vector>()) {
                e2 = c2->Index(index)->ValueAs<E2Type>();
            } else {
                e2 = c2->ValueAs<E2Type>();
            }

            E2Type bias;
            if constexpr (std::is_same_v<E1Type, f16>) {
                bias = 15;
            } else if constexpr (std::is_same_v<E1Type, f32>) {
                bias = 127;
            } else {
                bias = 1023;
            }

            if (e2 > bias + 1) {
                AddError(source) << "e2 must be less than or equal to " << (bias + 1);
                if (use_runtime_semantics_) {
                    return mgr.Zero(c1->Type());
                } else {
                    return error;
                }
            }

            auto target_ty = ty->DeepestElement();

            auto r = std::ldexp(e1, static_cast<int>(e2));
            return CreateScalar(source, target_ty, E1Type{r});
        };
        return Dispatch_fa_f32_f16(create, c1);
    };

    return TransformElements(mgr, ty, transform, 0, args[0]);
}

Eval::Result Eval::length(const core::type::Type* ty,
                          VectorRef<const Value*> args,
                          const Source& source) {
    auto r = Length(source, ty, args[0]);
    if (r != Success) {
        AddNote(source) << "when calculating length";
    }
    return r;
}

Eval::Result Eval::log(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto v) -> Eval::Result {
            using NumberT = decltype(v);
            if (v <= NumberT(0)) {
                AddError(source) << "log must be called with a value > 0";
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::log(v)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::log2(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto v) -> Eval::Result {
            using NumberT = decltype(v);
            if (v <= NumberT(0)) {
                AddError(source) << "log2 must be called with a value > 0";
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), NumberT(std::log2(v)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::max(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto e0, auto e1) {
            return CreateScalar(source, c0->Type(), decltype(e0)(std::max(e0, e1)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::min(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto e0, auto e1) {
            return CreateScalar(source, c0->Type(), decltype(e0)(std::min(e0, e1)));
        };
        return Dispatch_fia_fiu32_f16(create, c0, c1);
    };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::mix(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1, size_t index) {
        auto create = [&](auto e1, auto e2) -> Eval::Result {
            using NumberT = decltype(e1);
            // e3 is either a vector or a scalar
            NumberT e3;
            auto* c2 = args[2];
            if (c2->Type()->Is<core::type::Vector>()) {
                e3 = c2->Index(index)->ValueAs<NumberT>();
            } else {
                e3 = c2->ValueAs<NumberT>();
            }
            // Implement as `e1 * (1 - e3) + e2 * e3)` instead of as `e1 + e3 * (e2 - e1)` to avoid
            // float precision loss when e1 and e2 significantly differ in magnitude.
            auto one_sub_e3 = Sub(source, NumberT{1}, e3);
            if (one_sub_e3 != Success) {
                return error;
            }
            auto e1_mul_one_sub_e3 = Mul(source, e1, one_sub_e3.Get());
            if (e1_mul_one_sub_e3 != Success) {
                return error;
            }
            auto e2_mul_e3 = Mul(source, e2, e3);
            if (e2_mul_e3 != Success) {
                return error;
            }
            auto r = Add(source, e1_mul_one_sub_e3.Get(), e2_mul_e3.Get());
            if (r != Success) {
                return error;
            }
            return CreateScalar(source, c0->Type(), r.Get());
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    auto r = TransformElements(mgr, ty, transform, 0, args[0], args[1]);
    if (r != Success) {
        AddNote(source) << "when calculating mix";
    }
    return r;
}

Eval::Result Eval::modf(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform_fract = [&](const Value* c) {
        auto create = [&](auto e) {
            return CreateScalar(source, c->Type(), decltype(e)(e.value - std::trunc(e.value)));
        };
        return Dispatch_fa_f32_f16(create, c);
    };
    auto transform_whole = [&](const Value* c) {
        auto create = [&](auto e) {
            return CreateScalar(source, c->Type(), decltype(e)(std::trunc(e.value)));
        };
        return Dispatch_fa_f32_f16(create, c);
    };

    Vector<const Value*, 2> fields;

    if (auto fract = TransformUnaryElements(mgr, args[0]->Type(), transform_fract, args[0]);
        fract == Success) {
        fields.Push(fract.Get());
    } else {
        return error;
    }

    if (auto whole = TransformUnaryElements(mgr, args[0]->Type(), transform_whole, args[0]);
        whole == Success) {
        fields.Push(whole.Get());
    } else {
        return error;
    }

    return mgr.Composite(ty, std::move(fields));
}

Eval::Result Eval::normalize(const core::type::Type* ty,
                             VectorRef<const Value*> args,
                             const Source& source) {
    auto* len_ty = ty->DeepestElement();
    auto len = Length(source, len_ty, args[0]);
    if (len != Success) {
        AddNote(source) << "when calculating normalize";
        return error;
    }
    auto* v = len.Get();
    if (v->AllZero()) {
        AddError(source) << "zero length vector can not be normalized";
        if (use_runtime_semantics_) {
            return mgr.Zero(ty);
        } else {
            return error;
        }
    }
    return Divide(ty, Vector{args[0], v}, source);
}

Eval::Result Eval::pack2x16float(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto convert = [&](f32 val) -> tint::Result<uint32_t, Error> {
        auto conv = CheckedConvert<f16>(val);
        if (conv != Success) {
            AddError(source) << OverflowErrorMessage(val, "f16");
            if (use_runtime_semantics_) {
                return 0;
            } else {
                return error;
            }
        }
        uint16_t v = conv.Get().BitsRepresentation();
        return tint::Result<uint32_t, Error>{v};
    };

    auto* e = args[0];
    auto e0 = convert(e->Index(0)->ValueAs<f32>());
    if (e0 != Success) {
        return error;
    }

    auto e1 = convert(e->Index(1)->ValueAs<f32>());
    if (e1 != Success) {
        return error;
    }

    u32 ret = u32((e0.Get() & 0x0000'ffff) | (e1.Get() << 16));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack2x16snorm(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(-1.0f), f32(1.0f)).Get();
        return u32(
            tint::Bitcast<uint16_t>(static_cast<int16_t>(std::floor(0.5f + (32767.0f * clamped)))));
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());

    u32 ret = u32((e0 & 0x0000'ffff) | (e1 << 16));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack2x16unorm(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(0.0f), f32(1.0f)).Get();
        return u32{std::floor(0.5f + (65535.0f * clamped))};
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());

    u32 ret = u32((e0 & 0x0000'ffff) | (e1 << 16));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4x8snorm(const core::type::Type* ty,
                                VectorRef<const Value*> args,
                                const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(-1.0f), f32(1.0f)).Get();
        return u32(
            tint::Bitcast<uint8_t>(static_cast<int8_t>(std::floor(0.5f + (127.0f * clamped)))));
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());
    auto e2 = calc(e->Index(2)->ValueAs<f32>());
    auto e3 = calc(e->Index(3)->ValueAs<f32>());

    uint32_t mask = 0x0000'00ff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4x8unorm(const core::type::Type* ty,
                                VectorRef<const Value*> args,
                                const Source& source) {
    auto calc = [&](f32 val) -> u32 {
        auto clamped = Clamp(source, val, f32(0.0f), f32(1.0f)).Get();
        return u32{std::floor(0.5f + (255.0f * clamped))};
    };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<f32>());
    auto e1 = calc(e->Index(1)->ValueAs<f32>());
    auto e2 = calc(e->Index(2)->ValueAs<f32>());
    auto e3 = calc(e->Index(3)->ValueAs<f32>());

    uint32_t mask = 0x0000'00ff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4xI8(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto* e = args[0];
    auto e0 = e->Index(0)->ValueAs<i32>();
    auto e1 = e->Index(1)->ValueAs<i32>();
    auto e2 = e->Index(2)->ValueAs<i32>();
    auto e3 = e->Index(3)->ValueAs<i32>();

    int32_t mask = 0xff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4xU8(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto* e = args[0];
    auto e0 = e->Index(0)->ValueAs<u32>();
    auto e1 = e->Index(1)->ValueAs<u32>();
    auto e2 = e->Index(2)->ValueAs<u32>();
    auto e3 = e->Index(3)->ValueAs<u32>();

    uint32_t mask = 0xff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4xI8Clamp(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto calc = [&](i32 val) -> i32 { return Clamp(source, val, i32(-128), i32(127)).Get(); };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<i32>());
    auto e1 = calc(e->Index(1)->ValueAs<i32>());
    auto e2 = calc(e->Index(2)->ValueAs<i32>());
    auto e3 = calc(e->Index(3)->ValueAs<i32>());

    int32_t mask = 0xff;
    u32 ret = u32((e0 & mask) | ((e1 & mask) << 8) | ((e2 & mask) << 16) | ((e3 & mask) << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pack4xU8Clamp(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto calc = [&](u32 val) -> u32 { return Clamp(source, val, u32(0), u32(255)).Get(); };

    auto* e = args[0];
    auto e0 = calc(e->Index(0)->ValueAs<u32>());
    auto e1 = calc(e->Index(1)->ValueAs<u32>());
    auto e2 = calc(e->Index(2)->ValueAs<u32>());
    auto e3 = calc(e->Index(3)->ValueAs<u32>());

    u32 ret = u32(e0 | (e1 << 8) | (e2 << 16) | (e3 << 24));
    return CreateScalar(source, ty, ret);
}

Eval::Result Eval::pow(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto e1, auto e2) -> Eval::Result {
            auto r = CheckedPow(e1, e2);
            if (!r) {
                AddError(source) << OverflowErrorMessage(e1, "^", e2);
                if (use_runtime_semantics_) {
                    return mgr.Zero(c0->Type());
                } else {
                    return error;
                }
            }
            return CreateScalar(source, c0->Type(), *r);
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::radians(const core::type::Type* ty,
                           VectorRef<const Value*> args,
                           const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) -> Eval::Result {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto scale = Div(source, NumberT(std::numbers::pi_v<T>), NumberT(180));
            if (scale != Success) {
                AddNote(source) << "when calculating radians";
                return error;
            }
            auto result = Mul(source, e, scale.Get());
            if (result != Success) {
                AddNote(source) << "when calculating radians";
                return error;
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::reflect(const core::type::Type* ty,
                           VectorRef<const Value*> args,
                           const Source& source) {
    auto calculate = [&]() -> Eval::Result {
        // For the incident vector e1 and surface orientation e2, returns the reflection direction
        // e1 - 2 * dot(e2, e1) * e2.
        auto* e1 = args[0];
        auto* e2 = args[1];
        auto* vec_ty = ty->As<core::type::Vector>();
        auto* el_ty = vec_ty->Type();

        // dot(e2, e1)
        auto dot_e2_e1 = Dot(source, e2, e1);
        if (dot_e2_e1 != Success) {
            return error;
        }

        // 2 * dot(e2, e1)
        auto mul2 = [&](auto v) -> Eval::Result {
            using NumberT = decltype(v);
            return CreateScalar(source, el_ty, NumberT{NumberT{2} * v});
        };
        auto dot_e2_e1_2 = Dispatch_fa_f32_f16(mul2, dot_e2_e1.Get());
        if (dot_e2_e1_2 != Success) {
            return error;
        }

        // 2 * dot(e2, e1) * e2
        auto dot_e2_e1_2_e2 = Mul(source, ty, dot_e2_e1_2.Get(), e2);
        if (dot_e2_e1_2_e2 != Success) {
            return error;
        }

        // e1 - 2 * dot(e2, e1) * e2
        return Sub(source, ty, e1, dot_e2_e1_2_e2.Get());
    };
    auto r = calculate();
    if (r != Success) {
        AddNote(source) << "when calculating reflect";
    }
    return r;
}

Eval::Result Eval::refract(const core::type::Type* ty,
                           VectorRef<const Value*> args,
                           const Source& source) {
    auto* vec_ty = ty->As<core::type::Vector>();
    auto* el_ty = vec_ty->Type();

    auto compute_k = [&](auto e3, auto dot_e2_e1) -> Eval::Result {
        using NumberT = decltype(e3);
        // let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1))
        auto e3_squared = Mul(source, e3, e3);
        if (e3_squared != Success) {
            return error;
        }
        auto dot_e2_e1_squared = Mul(source, dot_e2_e1, dot_e2_e1);
        if (dot_e2_e1_squared != Success) {
            return error;
        }
        auto r = Sub(source, NumberT(1), dot_e2_e1_squared.Get());
        if (r != Success) {
            return error;
        }
        r = Mul(source, e3_squared.Get(), r.Get());
        if (r != Success) {
            return error;
        }
        r = Sub(source, NumberT(1), r.Get());
        if (r != Success) {
            return error;
        }
        return CreateScalar(source, el_ty, r.Get());
    };

    auto compute_e2_scale = [&](auto e3, auto dot_e2_e1, auto k) -> Eval::Result {
        // e3 * dot(e2, e1) + sqrt(k)
        auto sqrt_k = Sqrt(source, k);
        if (sqrt_k != Success) {
            return error;
        }
        auto r = Mul(source, e3, dot_e2_e1);
        if (r != Success) {
            return error;
        }
        r = Add(source, r.Get(), sqrt_k.Get());
        if (r != Success) {
            return error;
        }
        return CreateScalar(source, el_ty, r.Get());
    };

    auto calculate = [&]() -> Eval::Result {
        auto* e1 = args[0];
        auto* e2 = args[1];
        auto* e3 = args[2];

        // For the incident vector e1 and surface normal e2, and the ratio of indices of refraction
        // e3, let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1)). If k < 0.0, returns the
        // refraction vector 0.0, otherwise return the refraction vector e3 * e1 - (e3 * dot(e2, e1)
        // + sqrt(k)) * e2.

        // dot(e2, e1)
        auto dot_e2_e1 = Dot(source, e2, e1);
        if (dot_e2_e1 != Success) {
            return error;
        }

        // let k = 1.0 - e3 * e3 * (1.0 - dot(e2, e1) * dot(e2, e1))
        auto k = Dispatch_fa_f32_f16(compute_k, e3, dot_e2_e1.Get());
        if (k != Success) {
            return error;
        }

        // If k < 0.0, returns the refraction vector 0.0
        if (k.Get()->ValueAs<AFloat>() < 0) {
            return mgr.Zero(ty);
        }

        // Otherwise return the refraction vector e3 * e1 - (e3 * dot(e2, e1) + sqrt(k)) * e2
        auto e1_scaled = Mul(source, ty, e3, e1);
        if (e1_scaled != Success) {
            return error;
        }
        auto e2_scale = Dispatch_fa_f32_f16(compute_e2_scale, e3, dot_e2_e1.Get(), k.Get());
        if (e2_scale != Success) {
            return error;
        }
        auto e2_scaled = Mul(source, ty, e2_scale.Get(), e2);
        if (e2_scaled != Success) {
            return error;
        }
        return Sub(source, ty, e1_scaled.Get(), e2_scaled.Get());
    };
    auto r = calculate();
    if (r != Success) {
        AddNote(source) << "when calculating refract";
    }
    return r;
}

Eval::Result Eval::reverseBits(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto in_e) -> Eval::Result {
            using NumberT = decltype(in_e);
            using T = UnwrapNumber<NumberT>;
            using UT = std::make_unsigned_t<T>;
            constexpr UT kNumBits = sizeof(UT) * 8;

            UT e = static_cast<UT>(in_e);
            UT r = UT{0};
            for (size_t s = 0; s < kNumBits; ++s) {
                // Write source 's' bit to destination 'd' bit if 1
                if (e & (UT{1} << s)) {
                    size_t d = kNumBits - s - 1;
                    r |= (UT{1} << d);
                }
            }

            return CreateScalar(source, c0->Type(), NumberT{r});
        };
        return Dispatch_iu32(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::round(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            using T = UnwrapNumber<NumberT>;

            auto integral = NumberT(0);
            auto fract = std::abs(std::modf(e.value, &(integral.value)));
            // When e lies halfway between integers k and k + 1, the result is k when k is even,
            // and k + 1 when k is odd.
            NumberT result = NumberT(0.0);
            if (fract == NumberT(0.5)) {
                // If the integral value is negative, then we need to subtract one in order to move
                // to the correct `k`. The half way check is `k` and `k + 1` which in the positive
                // case is `x` and `x + 1` but in the negative case is `x - 1` and `x`.
                T integral_val = integral.value;
                if (std::signbit(integral_val)) {
                    integral_val = std::abs(integral_val - 1);
                }
                if (uint64_t(integral_val) % 2 == 0) {
                    result = NumberT(std::floor(e.value));
                } else {
                    result = NumberT(std::ceil(e.value));
                }
            } else {
                result = NumberT(std::round(e.value));
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::saturate(const core::type::Type* ty,
                            VectorRef<const Value*> args,
                            const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) {
            using NumberT = decltype(e);
            return CreateScalar(source, c0->Type(),
                                NumberT(std::min(std::max(e, NumberT(0.0)), NumberT(1.0))));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::select_bool(const core::type::Type* ty,
                               VectorRef<const Value*> args,
                               const Source& source) {
    auto cond = args[2]->ValueAs<bool>();
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto f, auto t) -> Eval::Result {
            return CreateScalar(source, ty->DeepestElement(), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::select_boolvec(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1, size_t index) {
        auto create = [&](auto f, auto t) -> Eval::Result {
            // Get corresponding bool value at the current vector value index
            auto cond = args[2]->Index(index)->ValueAs<bool>();
            return CreateScalar(source, ty->DeepestElement(), cond ? t : f);
        };
        return Dispatch_fia_fiu32_f16_bool(create, c0, c1);
    };

    return TransformElements(mgr, ty, transform, 0, args[0], args[1]);
}

Eval::Result Eval::sign(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto e) -> Eval::Result {
            using NumberT = decltype(e);
            NumberT result;
            NumberT zero{0.0};
            if (e.value < zero) {
                result = NumberT{-1.0};
            } else if (e.value > zero) {
                result = NumberT{1.0};
            } else {
                result = zero;
            }
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fia_fi32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::sin(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::sin(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::sinh(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::sinh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::smoothstep(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1, const Value* c2) {
        auto create = [&](auto low, auto high, auto x) -> Eval::Result {
            using NumberT = decltype(low);

            if (low == high) {
                AddError(source) << "smoothstep called with 'low' (" << low << ") equal to 'high' ("
                                 << high << ")";
                if (!use_runtime_semantics_) {
                    return error;
                }
            }

            auto err = [&] {
                AddNote(source) << "when calculating smoothstep";
                return error;
            };

            // t = clamp((x - low) / (high - low), 0.0, 1.0)
            auto x_minus_low = Sub(source, x, low);
            auto high_minus_low = Sub(source, high, low);
            if (x_minus_low != Success || high_minus_low != Success) {
                return err();
            }

            auto div = Div(source, x_minus_low.Get(), high_minus_low.Get());
            if (div != Success) {
                return err();
            }

            auto clamp = Clamp(source, div.Get(), NumberT(0), NumberT(1));
            auto t = clamp.Get();

            // result = t * t * (3.0 - 2.0 * t)
            auto t_times_t = Mul(source, t, t);
            auto t_times_2 = Mul(source, NumberT(2), t);
            if (t_times_t != Success || t_times_2 != Success) {
                return err();
            }

            auto three_minus_t_times_2 = Sub(source, NumberT(3), t_times_2.Get());
            if (three_minus_t_times_2 != Success) {
                return err();
            }

            auto result = Mul(source, t_times_t.Get(), three_minus_t_times_2.Get());
            if (result != Success) {
                return err();
            }
            return CreateScalar(source, c0->Type(), result.Get());
        };
        return Dispatch_fa_f32_f16(create, c0, c1, c2);
    };
    return TransformTernaryElements(mgr, ty, transform, args[0], args[1], args[2]);
}

Eval::Result Eval::step(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0, const Value* c1) {
        auto create = [&](auto edge, auto x) -> Eval::Result {
            using NumberT = decltype(edge);
            NumberT result = x.value < edge.value ? NumberT(0.0) : NumberT(1.0);
            return CreateScalar(source, c0->Type(), result);
        };
        return Dispatch_fa_f32_f16(create, c0, c1);
    };
    return TransformBinaryElements(mgr, ty, transform, args[0], args[1]);
}

Eval::Result Eval::sqrt(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        return Dispatch_fa_f32_f16(SqrtFunc(source, c0->Type()), c0);
    };

    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::tan(const core::type::Type* ty,
                       VectorRef<const Value*> args,
                       const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::tan(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::tanh(const core::type::Type* ty,
                        VectorRef<const Value*> args,
                        const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) -> Eval::Result {
            using NumberT = decltype(i);
            return CreateScalar(source, c0->Type(), NumberT(std::tanh(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::transpose(const core::type::Type* ty,
                             VectorRef<const Value*> args,
                             const Source&) {
    auto* m = args[0];
    auto* mat_ty = m->Type()->As<core::type::Matrix>();
    auto me = [&](size_t r, size_t c) { return m->Index(c)->Index(r); };
    auto* result_mat_ty = ty->As<core::type::Matrix>();

    // Produce column vectors from each row
    Vector<const Value*, 4> result_mat;
    for (size_t r = 0; r < mat_ty->Rows(); ++r) {
        Vector<const Value*, 4> new_col_vec;
        for (size_t c = 0; c < mat_ty->Columns(); ++c) {
            new_col_vec.Push(me(r, c));
        }
        result_mat.Push(mgr.Composite(result_mat_ty->ColumnType(), new_col_vec));
    }
    return mgr.Composite(ty, result_mat);
}

Eval::Result Eval::trunc(const core::type::Type* ty,
                         VectorRef<const Value*> args,
                         const Source& source) {
    auto transform = [&](const Value* c0) {
        auto create = [&](auto i) {
            return CreateScalar(source, c0->Type(), decltype(i)(std::trunc(i.value)));
        };
        return Dispatch_fa_f32_f16(create, c0);
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::unpack2x16float(const core::type::Type* ty,
                                   VectorRef<const Value*> args,
                                   const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto in = f16::FromBits(uint16_t((e >> (16 * i)) & 0x0000'ffff));
        auto val = CheckedConvert<f32>(in);
        if (val != Success) {
            AddError(source) << OverflowErrorMessage(in, "f32");
            if (use_runtime_semantics_) {
                val = f32(0.f);
            } else {
                return error;
            }
        }
        auto el = CreateScalar(source, inner_ty, val.Get());
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack2x16snorm(const core::type::Type* ty,
                                   VectorRef<const Value*> args,
                                   const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto val = f32(
            std::max(static_cast<float>(int16_t((e >> (16 * i)) & 0x0000'ffff)) / 32767.f, -1.f));
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack2x16unorm(const core::type::Type* ty,
                                   VectorRef<const Value*> args,
                                   const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 2> els;
    els.Reserve(2);
    for (size_t i = 0; i < 2; ++i) {
        auto val = f32(static_cast<float>(uint16_t((e >> (16 * i)) & 0x0000'ffff)) / 65535.f);
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack4x8snorm(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 4> els;
    els.Reserve(4);
    for (size_t i = 0; i < 4; ++i) {
        auto val =
            f32(std::max(static_cast<float>(int8_t((e >> (8 * i)) & 0x0000'00ff)) / 127.f, -1.f));
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack4x8unorm(const core::type::Type* ty,
                                  VectorRef<const Value*> args,
                                  const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 4> els;
    els.Reserve(4);
    for (size_t i = 0; i < 4; ++i) {
        auto val = f32(static_cast<float>(uint8_t((e >> (8 * i)) & 0x0000'00ff)) / 255.f);
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack4xI8(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 4> els;
    els.Reserve(4);

    for (size_t i = 0; i < 4; ++i) {
        uint8_t e_i = (e >> (8 * i)) & 0xff;
        auto val = i32(*reinterpret_cast<int8_t*>(&e_i));
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::unpack4xU8(const core::type::Type* ty,
                              VectorRef<const Value*> args,
                              const Source& source) {
    auto* inner_ty = ty->DeepestElement();
    auto e = args[0]->ValueAs<u32>().value;

    Vector<const Value*, 4> els;
    els.Reserve(4);

    for (size_t i = 0; i < 4; ++i) {
        auto val = u32((e >> (8 * i)) & 0xff);
        auto el = CreateScalar(source, inner_ty, val);
        if (el != Success) {
            return el;
        }
        els.Push(el.Get());
    }
    return mgr.Composite(ty, std::move(els));
}

Eval::Result Eval::quantizeToF16(const core::type::Type* ty,
                                 VectorRef<const Value*> args,
                                 const Source& source) {
    auto transform = [&](const Value* c) -> Eval::Result {
        auto value = c->ValueAs<f32>();
        auto conv = CheckedConvert<f32>(f16(value));
        if (conv != Success) {
            AddError(source) << OverflowErrorMessage(value, "f16");
            if (use_runtime_semantics_) {
                return mgr.Zero(c->Type());
            } else {
                return error;
            }
        }
        return CreateScalar(source, c->Type(), conv.Get());
    };
    return TransformUnaryElements(mgr, ty, transform, args[0]);
}

Eval::Result Eval::Convert(const core::type::Type* target_ty,
                           const Value* value,
                           const Source& source) {
    if (value->Type() == target_ty) {
        return value;
    }
    ConvertContext ctx{mgr, diags, source, use_runtime_semantics_};
    auto* converted = ConvertInternal(value, target_ty, ctx);
    return converted ? Result(converted) : Result(error);
}

diag::Diagnostic& Eval::AddError(const Source& source) const {
    if (use_runtime_semantics_) {
        return diags.AddWarning(source);
    } else {
        return diags.AddError(source);
    }
}

diag::Diagnostic& Eval::AddWarning(const Source& source) const {
    return diags.AddWarning(source);
}

diag::Diagnostic& Eval::AddNote(const Source& source) const {
    return diags.AddNote(source);
}

}  // namespace tint::core::constant
