//===--- VariantValue.h - Polymorphic value type -*- C++ -*-===/
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Polymorphic value type.
///
/// Supports all the types required for dynamic Matcher construction.
///  Used by the registry to construct matchers in a generic way.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ASTMATCHERS_DYNAMIC_VARIANTVALUE_H
#define LLVM_CLANG_ASTMATCHERS_DYNAMIC_VARIANTVALUE_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchersInternal.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/Twine.h"
#include <memory>
#include <vector>

namespace clang {
namespace ast_matchers {
namespace dynamic {

/// \brief Kind identifier.
///
/// It supports all types that VariantValue can contain.
class ArgKind {
 public:
  enum Kind {
    AK_Matcher,
    AK_Unsigned,
    AK_String
  };
  /// \brief Constructor for non-matcher types.
  ArgKind(Kind K) : K(K) { assert(K != AK_Matcher); }

  /// \brief Constructor for matcher types.
  ArgKind(ast_type_traits::ASTNodeKind MatcherKind)
      : K(AK_Matcher), MatcherKind(MatcherKind) {}

  Kind getArgKind() const { return K; }
  ast_type_traits::ASTNodeKind getMatcherKind() const {
    assert(K == AK_Matcher);
    return MatcherKind;
  }

  /// \brief Determines if this type can be converted to \p To.
  ///
  /// \param To the requested destination type.
  ///
  /// \param Specificity value corresponding to the "specificity" of the
  ///   convertion.
  bool isConvertibleTo(ArgKind To, unsigned *Specificity) const;

  bool operator<(const ArgKind &Other) const {
    if (K == AK_Matcher && Other.K == AK_Matcher)
      return MatcherKind < Other.MatcherKind;
    return K < Other.K;
  }

  /// \brief String representation of the type.
  std::string asString() const;

private:
  Kind K;
  ast_type_traits::ASTNodeKind MatcherKind;
};

using ast_matchers::internal::DynTypedMatcher;

/// \brief A variant matcher object.
///
/// The purpose of this object is to abstract simple and polymorphic matchers
/// into a single object type.
/// Polymorphic matchers might be implemented as a list of all the possible
/// overloads of the matcher. \c VariantMatcher knows how to select the
/// appropriate overload when needed.
/// To get a real matcher object out of a \c VariantMatcher you can do:
///  - getSingleMatcher() which returns a matcher, only if it is not ambiguous
///    to decide which matcher to return. Eg. it contains only a single
///    matcher, or a polymorphic one with only one overload.
///  - hasTypedMatcher<T>()/getTypedMatcher<T>(): These calls will determine if
///    the underlying matcher(s) can unambiguously return a Matcher<T>.
class VariantMatcher {
  /// \brief Methods that depend on T from hasTypedMatcher/getTypedMatcher.
  class MatcherOps {
  public:
    MatcherOps(ast_type_traits::ASTNodeKind NodeKind) : NodeKind(NodeKind) {}

    bool canConstructFrom(const DynTypedMatcher &Matcher,
                          bool &IsExactMatch) const;

    /// \brief Convert \p Matcher the destination type and return it as a new
    /// DynTypedMatcher.
    virtual DynTypedMatcher
    convertMatcher(const DynTypedMatcher &Matcher) const = 0;

    /// \brief Constructs a variadic typed matcher from \p InnerMatchers.
    /// Will try to convert each inner matcher to the destination type and
    /// return llvm::None if it fails to do so.
    llvm::Optional<DynTypedMatcher>
    constructVariadicOperator(DynTypedMatcher::VariadicOperator Op,
                              ArrayRef<VariantMatcher> InnerMatchers) const;

  protected:
    ~MatcherOps() = default;

  private:
    ast_type_traits::ASTNodeKind NodeKind;
  };

  /// \brief Payload interface to be specialized by each matcher type.
  ///
  /// It follows a similar interface as VariantMatcher itself.
  class Payload : public RefCountedBaseVPTR {
  public:
    ~Payload() override;
    virtual llvm::Optional<DynTypedMatcher> getSingleMatcher() const = 0;
    virtual std::string getTypeAsString() const = 0;
    virtual llvm::Optional<DynTypedMatcher>
    getTypedMatcher(const MatcherOps &Ops) const = 0;
    virtual bool isConvertibleTo(ast_type_traits::ASTNodeKind Kind,
                                 unsigned *Specificity) const = 0;
  };

public:
  /// \brief A null matcher.
  VariantMatcher();

  /// \brief Clones the provided matcher.
  static VariantMatcher SingleMatcher(const DynTypedMatcher &Matcher);

  /// \brief Clones the provided matchers.
  ///
  /// They should be the result of a polymorphic matcher.
  static VariantMatcher
  PolymorphicMatcher(std::vector<DynTypedMatcher> Matchers);

  /// \brief Creates a 'variadic' operator matcher.
  ///
  /// It will bind to the appropriate type on getTypedMatcher<T>().
  static VariantMatcher
  VariadicOperatorMatcher(DynTypedMatcher::VariadicOperator Op,
                          std::vector<VariantMatcher> Args);

  /// \brief Makes the matcher the "null" matcher.
  void reset();

  /// \brief Whether the matcher is null.
  bool isNull() const { return !Value; }

  /// \brief Return a single matcher, if there is no ambiguity.
  ///
  /// \returns the matcher, if there is only one matcher. An empty Optional, if
  /// the underlying matcher is a polymorphic matcher with more than one
  /// representation.
  llvm::Optional<DynTypedMatcher> getSingleMatcher() const;

  /// \brief Determines if the contained matcher can be converted to
  ///   \c Matcher<T>.
  ///
  /// For the Single case, it returns true if it can be converted to
  /// \c Matcher<T>.
  /// For the Polymorphic case, it returns true if one, and only one, of the
  /// overloads can be converted to \c Matcher<T>. If there are more than one
  /// that can, the result would be ambiguous and false is returned.
  template <class T>
  bool hasTypedMatcher() const {
    if (!Value) return false;
    return Value->getTypedMatcher(TypedMatcherOps<T>()).hasValue();
  }

  /// \brief Determines if the contained matcher can be converted to \p Kind.
  ///
  /// \param Kind the requested destination type.
  ///
  /// \param Specificity value corresponding to the "specificity" of the
  ///   convertion.
  bool isConvertibleTo(ast_type_traits::ASTNodeKind Kind,
                       unsigned *Specificity) const {
    if (Value)
      return Value->isConvertibleTo(Kind, Specificity);
    return false;
  }

  /// \brief Return this matcher as a \c Matcher<T>.
  ///
  /// Handles the different types (Single, Polymorphic) accordingly.
  /// Asserts that \c hasTypedMatcher<T>() is true.
  template <class T>
  ast_matchers::internal::Matcher<T> getTypedMatcher() const {
    assert(hasTypedMatcher<T>() && "hasTypedMatcher<T>() == false");
    return Value->getTypedMatcher(TypedMatcherOps<T>())
        ->template convertTo<T>();
  }

  /// \brief String representation of the type of the value.
  ///
  /// If the underlying matcher is a polymorphic one, the string will show all
  /// the types.
  std::string getTypeAsString() const;

private:
  explicit VariantMatcher(Payload *Value) : Value(Value) {}

  template <typename T> struct TypedMatcherOps;

  class SinglePayload;
  class PolymorphicPayload;
  class VariadicOpPayload;

  IntrusiveRefCntPtr<const Payload> Value;
};

template <typename T>
struct VariantMatcher::TypedMatcherOps final : VariantMatcher::MatcherOps {
  TypedMatcherOps()
      : MatcherOps(ast_type_traits::ASTNodeKind::getFromNodeKind<T>()) {}
  typedef ast_matchers::internal::Matcher<T> MatcherT;

  DynTypedMatcher
  convertMatcher(const DynTypedMatcher &Matcher) const override {
    return DynTypedMatcher(Matcher.convertTo<T>());
  }
};

/// \brief Variant value class.
///
/// Basically, a tagged union with value type semantics.
/// It is used by the registry as the return value and argument type for the
/// matcher factory methods.
/// It can be constructed from any of the supported types. It supports
/// copy/assignment.
///
/// Supported types:
///  - \c unsigned
///  - \c llvm::StringRef
///  - \c VariantMatcher (\c DynTypedMatcher / \c Matcher<T>)
class VariantValue {
public:
  VariantValue() : Type(VT_Nothing) {}

  VariantValue(const VariantValue &Other);
  ~VariantValue();
  VariantValue &operator=(const VariantValue &Other);

  /// \brief Specific constructors for each supported type.
  VariantValue(unsigned Unsigned);
  VariantValue(StringRef String);
  VariantValue(const VariantMatcher &Matchers);

  /// \brief Returns true iff this is not an empty value.
  explicit operator bool() const { return hasValue(); }
  bool hasValue() const { return Type != VT_Nothing; }

  /// \brief Unsigned value functions.
  bool isUnsigned() const;
  unsigned getUnsigned() const;
  void setUnsigned(unsigned Unsigned);

  /// \brief String value functions.
  bool isString() const;
  const std::string &getString() const;
  void setString(StringRef String);

  /// \brief Matcher value functions.
  bool isMatcher() const;
  const VariantMatcher &getMatcher() const;
  void setMatcher(const VariantMatcher &Matcher);

  /// \brief Determines if the contained value can be converted to \p Kind.
  ///
  /// \param Kind the requested destination type.
  ///
  /// \param Specificity value corresponding to the "specificity" of the
  ///   convertion.
  bool isConvertibleTo(ArgKind Kind, unsigned* Specificity) const;

  /// \brief Determines if the contained value can be converted to any kind
  /// in \p Kinds.
  ///
  /// \param Kinds the requested destination types.
  ///
  /// \param Specificity value corresponding to the "specificity" of the
  ///   convertion. It is the maximum specificity of all the possible
  ///   conversions.
  bool isConvertibleTo(ArrayRef<ArgKind> Kinds, unsigned *Specificity) const;

  /// \brief String representation of the type of the value.
  std::string getTypeAsString() const;

private:
  void reset();

  /// \brief All supported value types.
  enum ValueType {
    VT_Nothing,
    VT_Unsigned,
    VT_String,
    VT_Matcher
  };

  /// \brief All supported value types.
  union AllValues {
    unsigned Unsigned;
    std::string *String;
    VariantMatcher *Matcher;
  };

  ValueType Type;
  AllValues Value;
};

} // end namespace dynamic
} // end namespace ast_matchers
} // end namespace clang

#endif  // LLVM_CLANG_AST_MATCHERS_DYNAMIC_VARIANT_VALUE_H
