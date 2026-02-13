// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_

#include <stdint.h>
#include <limits>
#include <string>

#include "src/tint/lang/core/constant/eval.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

/// Forward declaration
namespace tint::core::intrinsic {
struct TableData;
}  // namespace tint::core::intrinsic

namespace tint::core::intrinsic {

/// An enumerator of index namespaces.
enum class TableIndexNamespace : uint8_t {
    kTemplate,
    kMatcher,
    kMatcherIndices,
    kTypeMatcher,
    kNumberMatcher,
    kParameter,
    kOverload,
    kConstEvalFunction,
};

/// A wrapper around an integer type, used to index intrinsic table data
/// @tparam T the index data type
/// @tparam N the index namespace
template <TableIndexNamespace N, typename T>
struct TableIndex {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                  "T must be an unsigned integer type");

    /// The value used to represent an invalid index
    static constexpr T kInvalid = std::numeric_limits<T>::max();

    /// Constructor for invalid index
    constexpr TableIndex() : value(kInvalid) {}

    /// Constructor
    /// @param index the index value
    constexpr explicit TableIndex(T index) : value(index) {}

    /// @returns true if this index is not invalid
    bool IsValid() const { return value != kInvalid; }

    /// Equality operator
    /// @param other the index to compare against
    /// @returns true if this index is equal to @p other
    bool operator==(const TableIndex& other) { return value == other.value; }

    /// Inequality operator
    /// @param other the index to compare against
    /// @returns true if this index is equal to @p other
    bool operator!=(const TableIndex& other) { return value != other.value; }

    /// @param offset the offset to apply to this index
    /// @returns a new index offset by @p offset
    template <typename U>
    auto operator+(U offset) const {
        static_assert(std::is_integral_v<U> && std::is_unsigned_v<U>,
                      "T must be an unsigned integer type");
        using C = std::conditional_t<(sizeof(U) > sizeof(T)), U, T>;
        C new_value = static_cast<C>(value) + static_cast<C>(offset);
        return TableIndex<N, decltype(new_value)>(new_value);
    }

    /// @param arr a C-style array
    /// @returns true if the integer type `T` has enough bits to index all the
    /// elements in the array @p arr.
    template <typename U, size_t COUNT>
    static constexpr bool CanIndex(U (&arr)[COUNT]) {
        (void)arr;  // The array isn't actually used
        /// kInvalid is the largest value representable by `T`. It is not a valid index.
        return COUNT < kInvalid;
    }

    /// The index value
    const T value = kInvalid;
};

/// Index type used to index TableData::template_types
using TemplateIndex = TableIndex<TableIndexNamespace::kTemplate, uint8_t>;

/// Index type used to index TableData::type_matchers or TableData::number_matchers
using MatcherIndex = TableIndex<TableIndexNamespace::kMatcher, uint8_t>;

/// Index type used to index TableData::matcher_indices
using MatcherIndicesIndex = TableIndex<TableIndexNamespace::kMatcherIndices, uint16_t>;

/// Index type used to index TableData::type_matchers
using TypeMatcherIndex = TableIndex<TableIndexNamespace::kTypeMatcher, uint8_t>;

/// Index type used to index TableData::number_matchers
using NumberMatcherIndex = TableIndex<TableIndexNamespace::kNumberMatcher, uint8_t>;

/// Index type used to index TableData::parameters
using ParameterIndex = TableIndex<TableIndexNamespace::kParameter, uint16_t>;

/// Index type used to index TableData::overloads
using OverloadIndex = TableIndex<TableIndexNamespace::kOverload, uint16_t>;

/// Index type used to index TableData::const_eval_functions
using ConstEvalFunctionIndex = TableIndex<TableIndexNamespace::kConstEvalFunction, uint8_t>;

/// Unique flag bits for overloads
enum class OverloadFlag : uint8_t {
    kIsBuiltin,                 // The overload is a builtin ('fn')
    kIsOperator,                // The overload is an operator ('op')
    kIsConstructor,             // The overload is a value constructor ('ctor')
    kIsConverter,               // The overload is a value converter ('conv')
    kSupportsVertexPipeline,    // The overload can be used in vertex shaders
    kSupportsFragmentPipeline,  // The overload can be used in fragment shaders
    kSupportsComputePipeline,   // The overload can be used in compute shaders
    kMustUse,                   // The overload cannot be called as a statement
    kMemberFunction,            // The overload is a member function
    kIsDeprecated,              // The overload is deprecated
};

/// An enum set of OverloadFlag, used by OperatorInfo
using OverloadFlags = tint::EnumSet<OverloadFlag>;

/// ParameterInfo describes a parameter
struct ParameterInfo {
    /// The parameter usage (parameter name in definition file)
    const ParameterUsage usage;

    /// Index of the matcher indices that are used to match the parameter types.
    /// These indices are consumed by the matchers themselves.
    const MatcherIndicesIndex matcher_indices;
};

/// TemplateInfo describes an template
struct TemplateInfo {
    /// An enumerator of template kind
    enum class Kind : uint8_t { kType, kNumber };

    /// Name of the template type (e.g. 'T')
    const char* name;

    /// Index of the type matcher indices that are used to match the template types.
    /// These indices are consumed by the matchers themselves.
    const MatcherIndicesIndex matcher_indices;

    /// The template kind
    const Kind kind;
};

/// OverloadInfo describes a single function overload
struct OverloadInfo {
    /// The flags for the overload
    const OverloadFlags flags;
    /// Total number of parameters for the overload
    const uint8_t num_parameters;
    /// Total number of explicit templates for the overload
    const uint8_t num_explicit_templates;
    /// Total number of implicit and explicit templates for the overload
    const uint8_t num_templates;
    /// Index of the first template in TableData::templates
    /// This is a list of explicit template types followed by the implicit template types.
    const TemplateIndex templates;
    /// Index of the first parameter in TableData::parameters
    const ParameterIndex parameters;
    /// Index of a list of type matcher indices that are used to build the return type.
    const MatcherIndicesIndex return_matcher_indices;
    /// The function used to evaluate the overload at shader-creation time.
    const ConstEvalFunctionIndex const_eval_fn;
};

/// IntrinsicInfo describes a builtin function or operator overload
struct IntrinsicInfo {
    /// Number of overloads of the intrinsic
    const uint8_t num_overloads;
    /// Index of the first overload for the function
    const OverloadIndex overloads;
};

/// A IntrinsicInfo with no overloads
static constexpr IntrinsicInfo kNoOverloads{0, OverloadIndex(OverloadIndex::kInvalid)};

/// Number is an 32 bit unsigned integer, which can be in one of three states:
/// * Invalid - Number has not been assigned a value
/// * Valid   - a fixed integer value
/// * Any     - matches any other non-invalid number
class Number {
    enum State : uint8_t {
        kInvalid,
        kValid,
        kAny,
    };

    constexpr explicit Number(State state) : state_(state) {}

  public:
    /// A special number representing any number
    static const Number any;
    /// An invalid number
    static const Number invalid;

    /// Constructed as a valid number with the value v
    /// @param v the value for the number
    explicit constexpr Number(uint32_t v) : value_(v), state_(kValid) {}

    /// @returns the value of the number
    inline uint32_t Value() const { return value_; }

    /// @returns the true if the number is valid
    inline bool IsValid() const { return state_ == kValid; }

    /// @returns the true if the number is any
    inline bool IsAny() const { return state_ == kAny; }

    /// Assignment operator.
    /// The number becomes valid, with the value n
    /// @param n the new value for the number
    /// @returns this so calls can be chained
    inline Number& operator=(uint32_t n) {
        value_ = n;
        state_ = kValid;
        return *this;
    }

  private:
    uint32_t value_ = 0;
    State state_ = kInvalid;
};

/// A special type that matches all TypeMatchers
class Any final : public Castable<Any, core::type::Type> {
  public:
    Any();
    ~Any() override;

    /// @copydoc core::type::UniqueNode::Equals
    bool Equals(const core::type::UniqueNode& other) const override;
    /// @copydoc core::type::Type::FriendlyName
    std::string FriendlyName() const override;
    /// @copydoc core::type::Type::Clone
    core::type::Type* Clone(core::type::CloneContext& ctx) const override;
};

/// TemplateState holds the state of the template numbers and types.
/// Used by the MatchState.
class TemplateState {
  public:
    /// If the template type with index @p idx is undefined, then it is defined with the @p ty
    /// and Type() returns @p ty. If the template type is defined, and @p ty can be converted to
    /// the template type then the template type is returned. If the template type is defined,
    /// and the template type can be converted to @p ty, then the template type is replaced with
    /// @p ty, and @p ty is returned. If none of the above applies, then @p ty is a type
    /// mismatch for the template type, and nullptr is returned.
    /// @param idx the index of the template type
    /// @param ty the type
    /// @returns true on match or newly defined
    const core::type::Type* Type(size_t idx, const core::type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        auto& t = types_[idx];
        if (t == nullptr) {
            t = ty;
            return ty;
        }
        ty = core::type::Type::Common(Vector{t, ty});
        if (ty) {
            t = ty;
        }
        return ty;
    }

    /// If the number with index @p idx is undefined, then it is defined with the number
    /// `number` and Num() returns true. If the number is defined, then `Num()` returns true iff
    /// it is equal to @p ty.
    /// @param idx the index of the template number
    /// @param number the number
    /// @returns true on match or newly defined
    bool Num(size_t idx, Number number) {
        if (idx >= numbers_.Length()) {
            numbers_.Resize(idx + 1, Number::any);
        }
        auto& n = numbers_[idx];
        if (n.IsAny()) {
            n = number.Value();
            return true;
        }
        return n.Value() == number.Value();
    }

    /// @param idx the index of the template type
    /// @returns the template type with index @p idx, or nullptr if the type was not
    /// defined.
    const core::type::Type* Type(size_t idx) const {
        if (idx >= types_.Length()) {
            return nullptr;
        }
        return types_[idx];
    }

    /// SetType replaces the template type with index @p idx with type @p ty.
    /// @param idx the index of the template type
    /// @param ty the new type for the template
    void SetType(size_t idx, const core::type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        types_[idx] = ty;
    }

    /// @returns the number type with index @p idx.
    /// @param idx the index of the template number
    Number Num(size_t idx) const {
        if (idx >= numbers_.Length()) {
            return Number::invalid;
        }
        return numbers_[idx];
    }

    /// SetNum replaces the template number with index @p idx with number @p num.
    /// @param idx the index of the template number
    /// @param num the new number for the template
    void SetNum(size_t idx, Number num) {
        if (idx >= numbers_.Length()) {
            numbers_.Resize(idx + 1, Number::any);
        }
        numbers_[idx] = num;
    }

    /// @return the total number of type and number templates
    size_t Count() const { return types_.Length() + numbers_.Length(); }

  private:
    Vector<const core::type::Type*, 4> types_;
    Vector<Number, 2> numbers_;
};

/// The current overload match state
/// MatchState holds the state used to match an overload.
class MatchState {
  public:
    /// Constructor
    /// @param ty_mgr the type manager
    /// @param syms the symbol table
    /// @param t the template state
    /// @param d the table data
    /// @param o the current overload
    /// @param matcher_indices the remaining matcher indices
    /// @param s the required evaluation stage of the overload
    MatchState(core::type::Manager& ty_mgr,
               SymbolTable& syms,
               TemplateState& t,
               const TableData& d,
               const OverloadInfo& o,
               const MatcherIndex* matcher_indices,
               EvaluationStage s)
        : types(ty_mgr),
          symbols(syms),
          templates(t),
          data(d),
          overload(o),
          earliest_eval_stage(s),
          matcher_indices_(matcher_indices) {}

    /// The type manager
    core::type::Manager& types;

    /// The symbol manager
    SymbolTable& symbols;

    /// The template types and numbers
    TemplateState& templates;

    /// The table data
    const TableData& data;

    /// The current overload being evaluated
    const OverloadInfo& overload;

    /// The earliest evaluation stage of the builtin call
    EvaluationStage earliest_eval_stage;

    /// Type uses the next TypeMatcher from the matcher indices to match the type @p ty.
    /// @param ty the type to try matching
    /// @returns the canonical expected type if the type matches, otherwise nullptr.
    /// @note: The matcher indices are progressed on calling.
    inline const core::type::Type* Type(const core::type::Type* ty);

    /// Num uses the next NumMatcher from the matcher indices to match @p number.
    /// @param number the number to try matching
    /// @returns the canonical expected number if the number matches, otherwise an invalid
    /// number.
    /// @note: The matcher indices are progressed on calling.
    inline Number Num(Number number);

    /// Prints the type matcher representation to @p out
    /// @note: The matcher indices are progressed on calling.
    inline void PrintType(StyledText& out);

    /// Prints the number matcher representation to @p out
    /// @note: The matcher indices are progressed on calling.
    inline void PrintNum(StyledText& out);

  private:
    const MatcherIndex* matcher_indices_ = nullptr;
};

/// A TypeMatcher is the interface used to match an type used as part of an
/// overload's parameter or return type.
struct TypeMatcher {
    /// Checks whether the given type matches the matcher rules, and returns the
    /// expected, canonicalized type on success.
    /// Match may define and refine the template types and numbers in state.
    /// The parameter `type` is the type to match
    /// Returns the canonicalized type on match, otherwise nullptr
    using MatchFn = const core::type::Type*(MatchState& state, const core::type::Type* type);

    /// @see #MatchFn
    MatchFn* const match;

    /// Prints the representation of the matcher.
    /// Used for printing error messages when no overload is found.
    using PrintFn = void(MatchState* state, StyledText& out);

    /// @see #PrintFn
    PrintFn* const print;
};

/// A NumberMatcher is the interface used to match a number or enumerator used
/// as part of an overload's parameter or return type.
struct NumberMatcher {
    /// Checks whether the given number matches the matcher rules.
    /// Match may define template numbers in state.
    /// The parameter `number` is the number to match
    /// Returns true if the argument type is as expected.
    using MatchFn = Number(MatchState& state, Number number);

    /// @see #MatchFn
    MatchFn* const match;

    /// Prints the representation of the matcher.
    /// Used for printing error messages when no overload is found.
    using PrintFn = void(MatchState* state, StyledText& out);

    /// @see #PrintFn
    PrintFn* const print;
};

/// TableData holds the immutable data that holds the intrinsic data for a language.
struct TableData {
    /// @param idx the index of the TemplateInfo in the table data
    /// @returns the TemplateInfo with the given index
    template <typename T>
    const TemplateInfo& operator[](TableIndex<TableIndexNamespace::kTemplate, T> idx) const {
        return templates[idx.value];
    }

    /// @param idx the index of the MatcherIndices in the table data
    /// @returns the MatcherIndices with the given index
    template <typename T>
    const MatcherIndex* operator[](TableIndex<TableIndexNamespace::kMatcherIndices, T> idx) const {
        if (idx.IsValid()) {
            return &matcher_indices[idx.value];
        }
        return nullptr;
    }

    /// @param idx the index of the TypeMatcher in the table data
    /// @returns the TypeMatcher with the given index
    template <typename T>
    const TypeMatcher& operator[](TableIndex<TableIndexNamespace::kTypeMatcher, T> idx) const {
        return type_matchers[idx.value];
    }

    /// @param idx the index of the NumberMatcher in the table data
    /// @returns the NumberMatcher with the given index
    template <typename T>
    const NumberMatcher& operator[](TableIndex<TableIndexNamespace::kNumberMatcher, T> idx) const {
        return number_matchers[idx.value];
    }

    /// @param idx the index of the ParameterInfo in the table data
    /// @returns the ParameterInfo with the given index
    template <typename T>
    const ParameterInfo& operator[](TableIndex<TableIndexNamespace::kParameter, T> idx) const {
        return parameters[idx.value];
    }

    /// @param idx the index of the OverloadInfo in the table data
    /// @returns the OverloadInfo with the given index
    template <typename T>
    const OverloadInfo& operator[](TableIndex<TableIndexNamespace::kOverload, T> idx) const {
        return overloads[idx.value];
    }

    /// @param idx the index of the constant::Eval::Function in the table data
    /// @returns the constant::Eval::Function with the given index
    template <typename T>
    constant::Eval::Function operator[](
        TableIndex<TableIndexNamespace::kConstEvalFunction, T> idx) const {
        return idx.IsValid() ? const_eval_functions[idx.value] : nullptr;
    }

    /// The list of templates used by the intrinsic overloads
    const Slice<const TemplateInfo> templates;
    /// The list of type matcher indices
    const Slice<const MatcherIndex> matcher_indices;
    /// The list of type matchers used by the intrinsic overloads
    const Slice<const TypeMatcher> type_matchers;
    /// The list of number matchers used by the intrinsic overloads
    const Slice<const NumberMatcher> number_matchers;
    /// The list of parameters used by the intrinsic overloads
    const Slice<const ParameterInfo> parameters;
    /// The list of overloads used by the intrinsics
    const Slice<const OverloadInfo> overloads;
    /// The list of constant evaluation functions used by the intrinsics
    const Slice<const constant::Eval::Function> const_eval_functions;
    /// The type constructor and convertor intrinsics
    const Slice<const IntrinsicInfo> ctor_conv;
    /// The builtin function intrinsic
    const Slice<const IntrinsicInfo> builtins;
    /// The IntrinsicInfo for the binary operator 'plus'
    const IntrinsicInfo& binary_plus;
    /// The IntrinsicInfo for the binary operator 'minus'
    const IntrinsicInfo& binary_minus;
    /// The IntrinsicInfo for the binary operator 'star'
    const IntrinsicInfo& binary_star;
    /// The IntrinsicInfo for the binary operator 'divide'
    const IntrinsicInfo& binary_divide;
    /// The IntrinsicInfo for the binary operator 'modulo'
    const IntrinsicInfo& binary_modulo;
    /// The IntrinsicInfo for the binary operator 'xor'
    const IntrinsicInfo& binary_xor;
    /// The IntrinsicInfo for the binary operator 'and'
    const IntrinsicInfo& binary_and;
    /// The IntrinsicInfo for the binary operator 'or'
    const IntrinsicInfo& binary_or;
    /// The IntrinsicInfo for the binary operator 'logical_and'
    const IntrinsicInfo& binary_logical_and;
    /// The IntrinsicInfo for the binary operator 'logical_or'
    const IntrinsicInfo& binary_logical_or;
    /// The IntrinsicInfo for the binary operator 'equal'
    const IntrinsicInfo& binary_equal;
    /// The IntrinsicInfo for the binary operator 'not_equal'
    const IntrinsicInfo& binary_not_equal;
    /// The IntrinsicInfo for the binary operator 'less_than'
    const IntrinsicInfo& binary_less_than;
    /// The IntrinsicInfo for the binary operator 'greater_than'
    const IntrinsicInfo& binary_greater_than;
    /// The IntrinsicInfo for the binary operator 'less_than_equal'
    const IntrinsicInfo& binary_less_than_equal;
    /// The IntrinsicInfo for the binary operator 'greater_than_equal'
    const IntrinsicInfo& binary_greater_than_equal;
    /// The IntrinsicInfo for the binary operator 'shift_left'
    const IntrinsicInfo& binary_shift_left;
    /// The IntrinsicInfo for the binary operator 'shift_right'
    const IntrinsicInfo& binary_shift_right;
    /// The IntrinsicInfo for the unary operator 'not'
    const IntrinsicInfo& unary_not;
    /// The IntrinsicInfo for the unary operator 'complement'
    const IntrinsicInfo& unary_complement;
    /// The IntrinsicInfo for the unary operator 'minus'
    const IntrinsicInfo& unary_minus;
    /// The IntrinsicInfo for the unary operator 'star'
    const IntrinsicInfo& unary_star;
    /// The IntrinsicInfo for the unary operator 'and'
    const IntrinsicInfo& unary_and;
};

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
const core::type::Type* MatchState::Type(const core::type::Type* ty) {
    TypeMatcherIndex matcher_index{(*matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.match(*this, ty);
}

Number MatchState::Num(Number number) {
    NumberMatcherIndex matcher_index{(*matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.match(*this, number);
}

void MatchState::PrintType(StyledText& out) {
    TypeMatcherIndex matcher_index{(*matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    matcher.print(this, out);
}

void MatchState::PrintNum(StyledText& out) {
    NumberMatcherIndex matcher_index{(*matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    matcher.print(this, out);
}
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

/// TemplateTypeMatcher is a Matcher for a template type.
/// The TemplateTypeMatcher will initially match against any type, and then will only be further
/// constrained based on the conversion rules defined at
/// https://www.w3.org/TR/WGSL/#conversion-rank
template <size_t INDEX>
struct TemplateTypeMatcher {
    /// The TypeMatcher for the template type with the index `INDEX`
    static constexpr TypeMatcher matcher{
        /* match */
        [](MatchState& state, const core::type::Type* type) -> const core::type::Type* {
            if (type->Is<Any>()) {
                return state.templates.Type(INDEX);
            }
            if (auto* templates = state.templates.Type(INDEX, type)) {
                return templates;
            }
            return nullptr;
        },
        /* print */
        [](MatchState* state, StyledText& out) {
            out << style::Type(state->data[state->overload.templates + INDEX].name);
        },
    };
};

/// TemplateNumberMatcher is a Matcher for a template number.
/// The TemplateNumberMatcher will match against any number (so long as it is
/// consistent for all uses in the overload)
template <size_t INDEX>
struct TemplateNumberMatcher {
    /// The NumberMatcher for the template number with the index `INDEX`
    static constexpr NumberMatcher matcher{
        /* match */
        [](MatchState& state, Number number) -> Number {
            if (number.IsAny()) {
                return state.templates.Num(INDEX);
            }
            return state.templates.Num(INDEX, number) ? number : Number::invalid;
        },
        /* print */
        [](MatchState* state, StyledText& out) {
            out << style::Variable(state->data[state->overload.templates + INDEX].name);
        },
    };
};

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_
