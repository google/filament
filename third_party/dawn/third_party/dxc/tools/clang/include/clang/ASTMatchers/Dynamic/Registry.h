//===--- Registry.h - Matcher registry -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Registry of all known matchers.
///
/// The registry provides a generic interface to construct any matcher by name.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ASTMATCHERS_DYNAMIC_REGISTRY_H
#define LLVM_CLANG_ASTMATCHERS_DYNAMIC_REGISTRY_H

#include "clang/ASTMatchers/Dynamic/Diagnostics.h"
#include "clang/ASTMatchers/Dynamic/VariantValue.h"
#include "clang/Basic/LLVM.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace ast_matchers {
namespace dynamic {

namespace internal {
class MatcherDescriptor;
}

typedef const internal::MatcherDescriptor *MatcherCtor;

struct MatcherCompletion {
  MatcherCompletion() {}
  MatcherCompletion(StringRef TypedText, StringRef MatcherDecl,
                    unsigned Specificity)
      : TypedText(TypedText), MatcherDecl(MatcherDecl),
        Specificity(Specificity) {}

  /// \brief The text to type to select this matcher.
  std::string TypedText;

  /// \brief The "declaration" of the matcher, with type information.
  std::string MatcherDecl;

  /// \brief Value corresponding to the "specificity" of the converted matcher.
  ///
  /// Zero specificity indicates that this conversion would produce a trivial
  /// matcher that will either always or never match.
  /// Such matchers are excluded from code completion results.
  unsigned Specificity;

  bool operator==(const MatcherCompletion &Other) const {
    return TypedText == Other.TypedText && MatcherDecl == Other.MatcherDecl;
  }
};

class Registry {
public:
  /// \brief Look up a matcher in the registry by name,
  ///
  /// \return An opaque value which may be used to refer to the matcher
  /// constructor, or Optional<MatcherCtor>() if not found.
  static llvm::Optional<MatcherCtor> lookupMatcherCtor(StringRef MatcherName);

  /// \brief Compute the list of completion types for \p Context.
  ///
  /// Each element of \p Context represents a matcher invocation, going from
  /// outermost to innermost. Elements are pairs consisting of a reference to
  /// the matcher constructor and the index of the next element in the
  /// argument list of that matcher (or for the last element, the index of
  /// the completion point in the argument list). An empty list requests
  /// completion for the root matcher.
  static std::vector<ArgKind> getAcceptedCompletionTypes(
      llvm::ArrayRef<std::pair<MatcherCtor, unsigned>> Context);

  /// \brief Compute the list of completions that match any of
  /// \p AcceptedTypes.
  ///
  /// \param AcceptedTypes All types accepted for this completion.
  ///
  /// \return All completions for the specified types.
  /// Completions should be valid when used in \c lookupMatcherCtor().
  /// The matcher constructed from the return of \c lookupMatcherCtor()
  /// should be convertible to some type in \p AcceptedTypes.
  static std::vector<MatcherCompletion>
  getMatcherCompletions(ArrayRef<ArgKind> AcceptedTypes);

  /// \brief Construct a matcher from the registry.
  ///
  /// \param Ctor The matcher constructor to instantiate.
  ///
  /// \param NameRange The location of the name in the matcher source.
  ///   Useful for error reporting.
  ///
  /// \param Args The argument list for the matcher. The number and types of the
  ///   values must be valid for the matcher requested. Otherwise, the function
  ///   will return an error.
  ///
  /// \return The matcher object constructed if no error was found.
  ///   A null matcher if the number of arguments or argument types do not match
  ///   the signature.  In that case \c Error will contain the description of
  ///   the error.
  static VariantMatcher constructMatcher(MatcherCtor Ctor,
                                         const SourceRange &NameRange,
                                         ArrayRef<ParserValue> Args,
                                         Diagnostics *Error);

  /// \brief Construct a matcher from the registry and bind it.
  ///
  /// Similar the \c constructMatcher() above, but it then tries to bind the
  /// matcher to the specified \c BindID.
  /// If the matcher is not bindable, it sets an error in \c Error and returns
  /// a null matcher.
  static VariantMatcher constructBoundMatcher(MatcherCtor Ctor,
                                              const SourceRange &NameRange,
                                              StringRef BindID,
                                              ArrayRef<ParserValue> Args,
                                              Diagnostics *Error);

private:
  Registry() = delete;
};

}  // namespace dynamic
}  // namespace ast_matchers
}  // namespace clang

#endif  // LLVM_CLANG_AST_MATCHERS_DYNAMIC_REGISTRY_H
