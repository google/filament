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

#include "src/tint/lang/core/intrinsic/table.h"

#include <algorithm>
#include <limits>
#include <ostream>
#include <utility>

#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/invalid.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::core::intrinsic {

const Number Number::any{Number::kAny};
const Number Number::invalid{Number::kInvalid};

Any::Any() : Base(0u, core::type::Flags{}) {}
Any::~Any() = default;

bool Any::Equals(const core::type::UniqueNode&) const {
    return false;
}

std::string Any::FriendlyName() const {
    return "<any>";
}

core::type::Type* Any::Clone(core::type::CloneContext&) const {
    return nullptr;
}

namespace {

/// The Vector `N` template argument value for arrays of parameters.
constexpr const size_t kNumFixedParameters = Overload::kNumFixedParameters;

/// The Vector `N` template argument value for arrays of overload candidates.
constexpr const size_t kNumFixedCandidates = 8;

/// A list of candidates
using Candidates = Vector<Candidate, kNumFixedCandidates>;

/// Callback function when no overloads match.
using OnNoMatch = std::function<StyledText(VectorRef<Candidate>)>;

/// Sorts the candidates based on their score, with the lowest (best-ranking) scores first.
static inline void SortCandidates(Candidates& candidates) {
    std::stable_sort(candidates.begin(), candidates.end(),
                     [&](const Candidate& a, const Candidate& b) { return a.score < b.score; });
}

/// Prints a list of types.
static void PrintTypeList(StyledText& ss, VectorRef<const core::type::Type*> types) {
    bool first = true;
    for (auto* arg : types) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << style::Type((arg != nullptr) ? arg->FriendlyName() : "undef");
    }
}

/// Attempts to find a single intrinsic overload that matches the provided argument types.
/// @param context the intrinsic context
/// @param intrinsic the intrinsic being called
/// @param intrinsic_name the name of the intrinsic
/// @param template_args the template argument types
/// @param args the argument types
/// @param earliest_eval_stage the earliest evaluation stage that the call can be made
/// @param member_function `true` if the builtin should be a member function
/// @param on_no_match an error callback when no intrinsic overloads matched the provided
///                    arguments.
/// @returns the matched intrinsic
Result<Overload, StyledText> MatchIntrinsic(Context& context,
                                            const IntrinsicInfo& intrinsic,
                                            std::string_view intrinsic_name,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage,
                                            bool member_function,
                                            const OnNoMatch& on_no_match);

/// The scoring mode for ScoreOverload()
enum class ScoreMode {
    /// If the overload doesn't match, then the returned Candidate will simply have a score of 1.
    /// No other fields will be populated.
    kEarlyReject,
    /// A more expensive score calculations will be made for the overload, which can be used
    /// to rank potential overloads
    kFull
};

/// Evaluates the single overload for the provided argument types.
/// @param context the intrinsic context
/// @param overload the overload being considered
/// @param template_args the template argument types
/// @param args the argument types
/// @tparam MODE the scoring mode to use. Passed as a template argument to ensure that the
/// extremely-hot function is specialized without scoring logic for the common code path.
/// @returns the evaluated Candidate information.
template <ScoreMode MODE>
Candidate ScoreOverload(Context& context,
                        const OverloadInfo& overload,
                        VectorRef<const core::type::Type*> template_args,
                        VectorRef<const core::type::Type*> args,
                        EvaluationStage earliest_eval_stage);

/// Performs overload resolution given the list of candidates, by ranking the conversions of
/// arguments to the each of the candidate's parameter types.
/// @param context the intrinsic context
/// @param candidates the list of candidate overloads
/// @param intrinsic_name the name of the intrinsic
/// @param template_args the template argument types
/// @param args the argument types
/// @see https://www.w3.org/TR/WGSL/#overload-resolution-section
/// @returns the resolved Candidate.
Result<Candidate, StyledText> ResolveCandidate(Context& context,
                                               Candidates&& candidates,
                                               std::string_view intrinsic_name,
                                               VectorRef<const core::type::Type*> template_args,
                                               VectorRef<const core::type::Type*> args);

// Prints the list of candidates for emitting diagnostics
void PrintCandidates(StyledText& err,
                     Context& context,
                     VectorRef<Candidate> candidates,
                     std::string_view intrinsic_name,
                     VectorRef<const core::type::Type*> template_args,
                     VectorRef<const core::type::Type*> args);

/// Raises an ICE when no overload is a clear winner of overload resolution
StyledText ErrAmbiguousOverload(Context& context,
                                std::string_view intrinsic_name,
                                VectorRef<const core::type::Type*> template_args,
                                VectorRef<const core::type::Type*> args,
                                VectorRef<Candidate> candidates);

/// @return a string representing a call to a builtin with the given argument types.
StyledText CallSignature(std::string_view intrinsic_name,
                         VectorRef<const core::type::Type*> template_args,
                         VectorRef<const core::type::Type*> args) {
    StyledText out;
    out << style::Code << style::Function(intrinsic_name);
    if (!template_args.IsEmpty()) {
        out << "<";
        PrintTypeList(out, template_args);
        out << ">";
    }
    out << "(";
    PrintTypeList(out, args);
    out << ")";

    return out;
}

Result<Overload, StyledText> MatchIntrinsic(Context& context,
                                            const IntrinsicInfo& intrinsic,
                                            std::string_view intrinsic_name,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage,
                                            bool member_function,
                                            const OnNoMatch& on_no_match) {
    const size_t num_overloads = static_cast<size_t>(intrinsic.num_overloads);
    size_t num_matched = 0;
    size_t match_idx = 0;
    Vector<Candidate, kNumFixedCandidates> candidates;
    candidates.Reserve(intrinsic.num_overloads);
    for (size_t overload_idx = 0; overload_idx < num_overloads; overload_idx++) {
        auto& overload = context.data[intrinsic.overloads + overload_idx];

        // Check that the overload is a member function iff we expect one.
        if (overload.flags.Contains(OverloadFlag::kMemberFunction) != member_function) {
            continue;
        }

        auto candidate = ScoreOverload<ScoreMode::kEarlyReject>(context, overload, template_args,
                                                                args, earliest_eval_stage);
        if (candidate.score == 0) {
            match_idx = candidates.Length();
            num_matched++;
        }
        candidates.Push(std::move(candidate));
    }

    // How many candidates matched?
    if (DAWN_UNLIKELY(num_matched == 0)) {
        // Perform the full scoring of each overload
        for (size_t overload_idx = 0; overload_idx < num_overloads; overload_idx++) {
            auto& overload = context.data[intrinsic.overloads + overload_idx];

            // Check that the overload is a member function iff we expect one.
            if (overload.flags.Contains(OverloadFlag::kMemberFunction) != member_function) {
                continue;
            }

            candidates[overload_idx] = ScoreOverload<ScoreMode::kFull>(
                context, overload, template_args, args, earliest_eval_stage);
        }
        // Sort the candidates with the most promising first
        SortCandidates(candidates);
        return on_no_match(std::move(candidates));
    }

    Candidate match;

    if (num_matched == 1) {
        match = std::move(candidates[match_idx]);
    } else {
        auto result =
            ResolveCandidate(context, std::move(candidates), intrinsic_name, template_args, args);
        if (DAWN_UNLIKELY(result != Success)) {
            return result.Failure();
        }
        match = result.Get();
    }

    // Build the return type
    const core::type::Type* return_type = nullptr;
    if (auto* matcher_indices = context.data[match.overload->return_matcher_indices]) {
        Any any;
        return_type =
            context.Match(match.templates, *match.overload, matcher_indices, earliest_eval_stage)
                .Type(&any);
        if (DAWN_UNLIKELY(!return_type)) {
            StyledText err;
            err << "MatchState.MatchState() returned null";
            TINT_ICE() << err.Plain();
        }
    } else {
        return_type = context.types.void_();
    }

    return Overload{match.overload, return_type, std::move(match.parameters),
                    context.data[match.overload->const_eval_fn]};
}

template <ScoreMode MODE>
Candidate ScoreOverload(Context& context,
                        const OverloadInfo& overload,
                        VectorRef<const core::type::Type*> template_args,
                        VectorRef<const core::type::Type*> args,
                        EvaluationStage earliest_eval_stage) {
#define MATCH_FAILURE(PENALTY)                           \
    do {                                                 \
        if constexpr (MODE == ScoreMode::kEarlyReject) { \
            return Candidate{1};                         \
        } else {                                         \
            score += PENALTY;                            \
        }                                                \
    } while (false)

    // Penalty weights for overload mismatching.
    // This scoring is used to order the suggested overloads in diagnostic on overload mismatch, and
    // has no impact for a correct program.
    // The overloads with the lowest score will be displayed first (top-most).
    constexpr int kMismatchedExplicitTemplateCountPenalty = 10;
    constexpr int kMismatchedParamCountPenalty = 3;
    constexpr int kMismatchedParamTypePenalty = 2;
    constexpr int kMismatchedExplicitTemplateTypePenalty = 1;
    constexpr int kMismatchedImplicitTemplateTypePenalty = 1;
    constexpr int kMismatchedImplicitTemplateNumberPenalty = 1;

    const size_t num_parameters = static_cast<size_t>(overload.num_parameters);
    const size_t num_arguments = static_cast<size_t>(args.Length());

    size_t score = 0;

    if (num_parameters != num_arguments) {
        MATCH_FAILURE(kMismatchedParamCountPenalty * (std::max(num_parameters, num_arguments) -
                                                      std::min(num_parameters, num_arguments)));
    }

    // Check that all of the template arguments provided are actually expected by the overload.
    const size_t expected_templates = overload.num_explicit_templates;
    const size_t provided_templates = template_args.Length();
    if (provided_templates != expected_templates) {
        MATCH_FAILURE(kMismatchedExplicitTemplateCountPenalty *
                      (std::max(expected_templates, provided_templates) -
                       std::min(expected_templates, provided_templates)));
    }

    TemplateState templates;

    // Check that the explicit template arguments match the constraint if specified, otherwise
    // just set the template type.
    auto num_tmpl_args = std::min<size_t>(overload.num_explicit_templates, template_args.Length());
    for (size_t i = 0; i < num_tmpl_args; ++i) {
        auto& tmpl = context.data[overload.templates + i];
        auto* type = template_args[i];
        if (auto* matcher_indices = context.data[tmpl.matcher_indices]) {
            // Ensure type matches the template's matcher.
            type =
                context.Match(templates, overload, matcher_indices, earliest_eval_stage).Type(type);
            if (!type) {
                MATCH_FAILURE(kMismatchedExplicitTemplateTypePenalty);
                continue;
            }
        }
        templates.SetType(i, type);
    }

    // Invoke the matchers for each parameter <-> argument pair.
    // If any arguments cannot be matched, then `score` will be increased.
    // If the overload has any template types or numbers then these will be set based on the
    // argument types. Template types may be refined by constraining with later argument types. For
    // example calling `F<T>(T, T)` with the argument types (abstract-int, i32) will first set T to
    // abstract-int when matching the first argument, and then constrained down to i32 when matching
    // the second argument.
    // Note that inferred template types are not tested against their matchers at this point.
    auto num_params = std::min(num_parameters, num_arguments);
    for (size_t p = 0; p < num_params; p++) {
        auto& parameter = context.data[overload.parameters + p];
        auto* matcher_indices = context.data[parameter.matcher_indices];
        if (!context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                 .Type(args[p])) {
            MATCH_FAILURE(kMismatchedParamTypePenalty);
        }
    }

    // Check each of the inferred types and numbers for the implicit templates match their
    // respective matcher.
    for (size_t i = overload.num_explicit_templates; i < overload.num_templates; i++) {
        auto& tmpl = context.data[overload.templates + i];
        auto* matcher_indices = context.data[tmpl.matcher_indices];
        if (!matcher_indices) {
            continue;
        }

        auto matcher = context.Match(templates, overload, matcher_indices, earliest_eval_stage);

        switch (tmpl.kind) {
            case TemplateInfo::Kind::kType: {
                // Check all constrained template types matched their constraint matchers.
                // If the template type *does not* match any of the types in the constraint
                // matcher, then `score` is incremented and the template is assigned an invalid
                // type. If the template type *does* match a type, then the template type is
                // replaced with the first matching type. The order of types in the template matcher
                // is important here, which can be controlled with the [[precedence(N)]] decorations
                // on the types in the def file.
                if (auto* type = templates.Type(i)) {
                    if (auto* ty = matcher.Type(type)) {
                        // Template type matched one of the types in the template type's
                        // matcher. Replace the template type with this type.
                        templates.SetType(i, ty);
                        continue;
                    }
                }
                templates.SetType(i, context.types.invalid());
                MATCH_FAILURE(kMismatchedImplicitTemplateTypePenalty);
                break;
            }

            case TemplateInfo::Kind::kNumber: {
                // Checking that the inferred number matches the constraints on the
                // template. Increments `score` and assigns the template an invalid number if the
                // template numbers do not match their constraint matchers.
                auto number = templates.Num(i);
                if (!number.IsValid() || !matcher.Num(number).IsValid()) {
                    templates.SetNum(i, Number::invalid);
                    MATCH_FAILURE(kMismatchedImplicitTemplateNumberPenalty);
                }
            }
        }
    }

    // Now that all the template types have been finalized, we can construct the parameters.
    Vector<Overload::Parameter, kNumFixedParameters> parameters;
    parameters.Reserve(num_params);
    for (size_t p = 0; p < num_params; p++) {
        auto& parameter = context.data[overload.parameters + p];
        auto* matcher_indices = context.data[parameter.matcher_indices];
        auto* ty =
            context.Match(templates, overload, matcher_indices, earliest_eval_stage).Type(args[p]);
        parameters.Emplace(ty, parameter.usage);
    }

    return Candidate{score, &overload, templates, parameters};
#undef MATCH_FAILURE
}

Result<Candidate, StyledText> ResolveCandidate(Context& context,
                                               Candidates&& candidates,
                                               std::string_view intrinsic_name,
                                               VectorRef<const core::type::Type*> template_args,
                                               VectorRef<const core::type::Type*> args) {
    Vector<uint32_t, kNumFixedParameters> best_ranks;
    best_ranks.Resize(args.Length(), 0xffffffff);
    size_t num_matched = 0;
    Candidate* best = nullptr;
    for (auto& candidate : candidates) {
        if (candidate.score > 0) {
            continue;  // Candidate has already been ruled out.
        }
        bool some_won = false;   // An argument ranked less than the 'best' overload's argument
        bool some_lost = false;  // An argument ranked more than the 'best' overload's argument
        for (size_t i = 0; i < args.Length(); i++) {
            auto rank = core::type::Type::ConversionRank(args[i], candidate.parameters[i].type);
            if (best_ranks[i] > rank) {
                best_ranks[i] = rank;
                some_won = true;
            } else if (best_ranks[i] < rank) {
                some_lost = true;
            }
        }
        // If no arguments of this candidate ranked worse than the previous best candidate, then
        // this candidate becomes the new best candidate.
        // If no arguments of this candidate ranked better than the previous best candidate, then
        // this candidate is removed from the list of matches.
        // If neither of the above apply, then we have two candidates with no clear winner, which
        // results in an ambiguous overload error. In this situation the loop ends with
        // `num_matched > 1`.
        if (some_won) {
            // One or more arguments of this candidate ranked better than the previous best
            // candidate's argument(s).
            num_matched++;
            if (!some_lost) {
                // All arguments were at as-good or better than the previous best.
                if (best) {
                    // Mark the previous best candidate as no longer being in the running, by
                    // setting its score to a non-zero value. We pick 1 as this is the closest to 0
                    // (match) as we can get.
                    best->score = 1;
                    num_matched--;
                }
                // This candidate is the new best.
                best = &candidate;
            }
        } else {
            // No arguments ranked better than the current best.
            // Change the score of this candidate to a non-zero value, so that it's not considered a
            // match.
            candidate.score = 1;
        }
    }

    if (num_matched > 1) {
        // Re-sort the candidates with the most promising first
        SortCandidates(candidates);
        // Raise an error
        return ErrAmbiguousOverload(context, intrinsic_name, template_args, args, candidates);
    }

    return std::move(*best);
}

void PrintCandidates(StyledText& ss,
                     Context& context,
                     VectorRef<Candidate> candidates,
                     std::string_view intrinsic_name,
                     VectorRef<const core::type::Type*> template_args,
                     VectorRef<const core::type::Type*> args) {
    for (auto& candidate : candidates) {
        ss << " • ";
        PrintCandidate(ss, context, candidate, intrinsic_name, template_args, args);
        ss << "\n";
    }
}

StyledText ErrAmbiguousOverload(Context& context,
                                std::string_view intrinsic_name,
                                VectorRef<const core::type::Type*> template_args,
                                VectorRef<const core::type::Type*> args,
                                VectorRef<Candidate> candidates) {
    StyledText err;
    err << "ambiguous overload while attempting to match "
        << CallSignature(intrinsic_name, template_args, args) << "\n";

    for (auto& candidate : candidates) {
        if (candidate.score == 0) {
            err << "  ";
            PrintCandidate(err, context, candidate, intrinsic_name, template_args, args);
            err << "\n";
        }
    }
    TINT_ICE() << err.Plain();
}

}  // namespace

void PrintCandidate(StyledText& ss,
                    Context& context,
                    const Candidate& candidate,
                    std::string_view intrinsic_name,
                    VectorRef<const core::type::Type*> template_args,
                    VectorRef<const core::type::Type*> args) {
    // Restore old style before returning.
    auto prev_style = ss.Style();
    TINT_DEFER(ss << prev_style);

    auto& overload = *candidate.overload;

    TemplateState templates = candidate.templates;

    // TODO(crbug.com/tint/1730): Use input evaluation stage to output only relevant overloads.
    auto earliest_eval_stage = EvaluationStage::kConstant;

    ss << style::Code << style::Function(intrinsic_name);

    if (overload.num_explicit_templates > 0) {
        ss << "<";
        for (size_t i = 0; i < overload.num_explicit_templates; i++) {
            const auto& tmpl = context.data[overload.templates + i];

            bool matched = false;
            if (i < template_args.Length()) {
                auto* matcher_indices = context.data[tmpl.matcher_indices];
                matched = !matcher_indices ||
                          context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                              .Type(template_args[i]);
            }

            if (i > 0) {
                ss << ", ";
            }
            ss << style::Type(tmpl.name) << " ";
            if (matched) {
                ss << (style::Code + style::Match)(" ✓ ");
            } else {
                ss << (style::Code + style::Mismatch)(" ✗ ");
            }
        }
        ss << ">";
    }

    bool all_params_match = true;
    ss << "(";
    for (size_t i = 0; i < overload.num_parameters; i++) {
        const auto& parameter = context.data[overload.parameters + i];
        auto* matcher_indices = context.data[parameter.matcher_indices];

        bool matched = false;
        if (i < args.Length()) {
            matched = context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                          .Type(args[i]);
        }
        all_params_match = all_params_match && matched;

        if (i > 0) {
            ss << ", ";
        }

        if (parameter.usage != ParameterUsage::kNone) {
            ss << style::Variable(parameter.usage, ": ");
        }
        context.Match(templates, overload, matcher_indices, earliest_eval_stage).PrintType(ss);

        ss << style::Code << " ";
        if (matched) {
            ss << (style::Code + style::Match)(" ✓ ");
        } else {
            ss << (style::Code + style::Mismatch)(" ✗ ");
        }
    }
    ss << ")";
    if (overload.return_matcher_indices.IsValid()) {
        ss << " -> ";
        auto* matcher_indices = context.data[overload.return_matcher_indices];
        context.Match(templates, overload, matcher_indices, earliest_eval_stage).PrintType(ss);
    }

    bool first = true;
    auto separator = [&] {
        ss << style::Plain(first ? " where:\n     " : "\n     ");
        first = false;
    };

    if (all_params_match && args.Length() > overload.num_parameters) {
        separator();
        ss << style::Mismatch(" ✗ ")
           << style::Plain(" overload expects ", static_cast<int>(overload.num_parameters),
                           " argument", overload.num_parameters != 1 ? "s" : "", ", call passed ",
                           args.Length(), " argument", args.Length() != 1 ? "s" : "");
    }
    if (all_params_match && template_args.Length() > overload.num_explicit_templates) {
        separator();
        ss << style::Mismatch(" ✗ ")
           << style::Plain(" overload expects ", static_cast<int>(overload.num_explicit_templates),
                           " template argument", overload.num_explicit_templates != 1 ? "s" : "",
                           ", call passed ", template_args.Length(), " argument",
                           template_args.Length() != 1 ? "s" : "");
    }

    for (size_t i = 0; i < overload.num_templates; i++) {
        auto& tmpl = context.data[overload.templates + i];
        if (auto* matcher_indices = context.data[tmpl.matcher_indices]) {
            separator();
            bool matched = false;
            if (tmpl.kind == TemplateInfo::Kind::kType) {
                if (auto* ty = templates.Type(i)) {
                    matched =
                        context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                            .Type(ty);
                }
            } else {
                matched = context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                              .Num(templates.Num(i))
                              .IsValid();
            }
            if (matched) {
                ss << style::Match(" ✓ ") << style::Plain(" ");
            } else {
                ss << style::Mismatch(" ✗ ") << style::Plain(" ");
            }

            ss << style::Type(tmpl.name) << style::Plain(" is ");
            if (tmpl.kind == TemplateInfo::Kind::kType) {
                context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                    .PrintType(ss);
            } else {
                context.Match(templates, overload, matcher_indices, earliest_eval_stage)
                    .PrintNum(ss);
            }
        }
    }
}

Result<Overload, StyledText> LookupFn(Context& context,
                                      std::string_view intrinsic_name,
                                      size_t function_id,
                                      VectorRef<const core::type::Type*> template_args,
                                      VectorRef<const core::type::Type*> args,
                                      EvaluationStage earliest_eval_stage) {
    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](VectorRef<Candidate> candidates) {
        StyledText err;
        err << "no matching call to " << CallSignature(intrinsic_name, template_args, args) << "\n";
        if (!candidates.IsEmpty()) {
            err << "\n"
                << candidates.Length() << " candidate function"
                << (candidates.Length() > 1 ? "s:" : ":") << "\n";
            PrintCandidates(err, context, candidates, intrinsic_name, template_args, args);
        }
        return err;
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(context, context.data.builtins[function_id], intrinsic_name,
                          template_args, args, earliest_eval_stage, /* member_function */ false,
                          on_no_match);
}

Result<Overload, StyledText> LookupMemberFn(Context& context,
                                            std::string_view intrinsic_name,
                                            size_t function_id,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage) {
    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](VectorRef<Candidate> candidates) {
        StyledText err;
        err << "no matching call to " << CallSignature(intrinsic_name, template_args, args) << "\n";
        if (!candidates.IsEmpty()) {
            err << "\n"
                << candidates.Length() << " candidate function"
                << (candidates.Length() > 1 ? "s:" : ":") << "\n";
            PrintCandidates(err, context, candidates, intrinsic_name, template_args, args);
        }
        return err;
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(context, context.data.builtins[function_id], intrinsic_name,
                          template_args, args, earliest_eval_stage, /* member_function */ true,
                          on_no_match);
}

Result<Overload, StyledText> LookupUnary(Context& context,
                                         core::UnaryOp op,
                                         const core::type::Type* arg,
                                         EvaluationStage earliest_eval_stage) {
    const IntrinsicInfo* intrinsic_info = nullptr;
    std::string_view intrinsic_name;
    switch (op) {
        case core::UnaryOp::kComplement:
            intrinsic_info = &context.data.unary_complement;
            intrinsic_name = "operator ~ ";
            break;
        case core::UnaryOp::kNegation:
            intrinsic_info = &context.data.unary_minus;
            intrinsic_name = "operator - ";
            break;
        case core::UnaryOp::kAddressOf:
            intrinsic_info = &context.data.unary_and;
            intrinsic_name = "operator & ";
            break;
        case core::UnaryOp::kIndirection:
            intrinsic_info = &context.data.unary_star;
            intrinsic_name = "operator * ";
            break;
        case core::UnaryOp::kNot:
            intrinsic_info = &context.data.unary_not;
            intrinsic_name = "operator ! ";
            break;
    }

    Vector args{arg};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](VectorRef<Candidate> candidates) {
        StyledText err;
        err << "no matching overload for " << CallSignature(name, Empty, args) << "\n";
        if (!candidates.IsEmpty()) {
            err << "\n"
                << candidates.Length() << " candidate operator"
                << (candidates.Length() > 1 ? "s:" : ":") << "\n";
            PrintCandidates(err, context, candidates, name, Empty, Vector{arg});
        }
        return err;
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(context, *intrinsic_info, intrinsic_name, Empty, args,
                          earliest_eval_stage, /* member_function */ false, on_no_match);
}

Result<Overload, StyledText> LookupBinary(Context& context,
                                          core::BinaryOp op,
                                          const core::type::Type* lhs,
                                          const core::type::Type* rhs,
                                          EvaluationStage earliest_eval_stage,
                                          bool is_compound) {
    const IntrinsicInfo* intrinsic_info = nullptr;
    std::string_view intrinsic_name;
    switch (op) {
        case core::BinaryOp::kAnd:
            intrinsic_info = &context.data.binary_and;
            intrinsic_name = is_compound ? "operator &= " : "operator & ";
            break;
        case core::BinaryOp::kOr:
            intrinsic_info = &context.data.binary_or;
            intrinsic_name = is_compound ? "operator |= " : "operator | ";
            break;
        case core::BinaryOp::kXor:
            intrinsic_info = &context.data.binary_xor;
            intrinsic_name = is_compound ? "operator ^= " : "operator ^ ";
            break;
        case core::BinaryOp::kLogicalAnd:
            intrinsic_info = &context.data.binary_logical_and;
            intrinsic_name = "operator && ";
            break;
        case core::BinaryOp::kLogicalOr:
            intrinsic_info = &context.data.binary_logical_or;
            intrinsic_name = "operator || ";
            break;
        case core::BinaryOp::kEqual:
            intrinsic_info = &context.data.binary_equal;
            intrinsic_name = "operator == ";
            break;
        case core::BinaryOp::kNotEqual:
            intrinsic_info = &context.data.binary_not_equal;
            intrinsic_name = "operator != ";
            break;
        case core::BinaryOp::kLessThan:
            intrinsic_info = &context.data.binary_less_than;
            intrinsic_name = "operator < ";
            break;
        case core::BinaryOp::kGreaterThan:
            intrinsic_info = &context.data.binary_greater_than;
            intrinsic_name = "operator > ";
            break;
        case core::BinaryOp::kLessThanEqual:
            intrinsic_info = &context.data.binary_less_than_equal;
            intrinsic_name = "operator <= ";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            intrinsic_info = &context.data.binary_greater_than_equal;
            intrinsic_name = "operator >= ";
            break;
        case core::BinaryOp::kShiftLeft:
            intrinsic_info = &context.data.binary_shift_left;
            intrinsic_name = is_compound ? "operator <<= " : "operator << ";
            break;
        case core::BinaryOp::kShiftRight:
            intrinsic_info = &context.data.binary_shift_right;
            intrinsic_name = is_compound ? "operator >>= " : "operator >> ";
            break;
        case core::BinaryOp::kAdd:
            intrinsic_info = &context.data.binary_plus;
            intrinsic_name = is_compound ? "operator += " : "operator + ";
            break;
        case core::BinaryOp::kSubtract:
            intrinsic_info = &context.data.binary_minus;
            intrinsic_name = is_compound ? "operator -= " : "operator - ";
            break;
        case core::BinaryOp::kMultiply:
            intrinsic_info = &context.data.binary_star;
            intrinsic_name = is_compound ? "operator *= " : "operator * ";
            break;
        case core::BinaryOp::kDivide:
            intrinsic_info = &context.data.binary_divide;
            intrinsic_name = is_compound ? "operator /= " : "operator / ";
            break;
        case core::BinaryOp::kModulo:
            intrinsic_info = &context.data.binary_modulo;
            intrinsic_name = is_compound ? "operator %= " : "operator % ";
            break;
    }

    Vector args{lhs, rhs};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](VectorRef<Candidate> candidates) {
        StyledText err;
        err << "no matching overload for " << CallSignature(name, Empty, args) << "\n";
        if (!candidates.IsEmpty()) {
            err << "\n"
                << candidates.Length() << " candidate operator"
                << (candidates.Length() > 1 ? "s:" : ":") << "\n";
            PrintCandidates(err, context, candidates, name, Empty, args);
        }
        return err;
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(context, *intrinsic_info, intrinsic_name, Empty, args,
                          earliest_eval_stage, /* member_function */ false, on_no_match);
}

Result<Overload, StyledText> LookupCtorConv(Context& context,
                                            std::string_view type_name,
                                            size_t type_id,
                                            VectorRef<const core::type::Type*> template_args,
                                            VectorRef<const core::type::Type*> args,
                                            EvaluationStage earliest_eval_stage) {
    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](VectorRef<Candidate> candidates) {
        StyledText err;
        err << "no matching constructor for " << CallSignature(type_name, template_args, args)
            << "\n";
        Candidates ctor, conv;
        for (auto candidate : candidates) {
            if (candidate.overload->flags.Contains(OverloadFlag::kIsConstructor)) {
                ctor.Push(candidate);
            } else {
                conv.Push(candidate);
            }
        }
        if (!ctor.IsEmpty()) {
            err << "\n"
                << ctor.Length() << " candidate constructor" << (ctor.Length() > 1 ? "s:" : ":")
                << "\n";
            PrintCandidates(err, context, ctor, type_name, template_args, args);
        }
        if (!conv.IsEmpty()) {
            err << "\n"
                << conv.Length() << " candidate conversion" << (conv.Length() > 1 ? "s:" : ":")
                << "\n";
            PrintCandidates(err, context, conv, type_name, template_args, args);
        }
        return err;
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(context, context.data.ctor_conv[type_id], type_name, template_args, args,
                          earliest_eval_stage, /* member_function */ false, on_no_match);
}

}  // namespace tint::core::intrinsic

/// TypeInfo for the Any type declared in the anonymous namespace above
TINT_INSTANTIATE_TYPEINFO(tint::core::intrinsic::Any);
