//===--- TemplateName.h - C++ Template Name Representation-------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TemplateName interface and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_TEMPLATENAME_H
#define LLVM_CLANG_AST_TEMPLATENAME_H

#include "clang/Basic/LLVM.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/PointerUnion.h"

namespace clang {
  
class ASTContext;
class DependentTemplateName;
class DiagnosticBuilder;
class IdentifierInfo;
class NamedDecl;
class NestedNameSpecifier;
enum OverloadedOperatorKind : int;
class OverloadedTemplateStorage;
struct PrintingPolicy;
class QualifiedTemplateName;
class SubstTemplateTemplateParmPackStorage;
class SubstTemplateTemplateParmStorage;
class TemplateArgument;
class TemplateDecl;
class TemplateTemplateParmDecl;
  
/// \brief Implementation class used to describe either a set of overloaded
/// template names or an already-substituted template template parameter pack.
class UncommonTemplateNameStorage {
protected:
  enum Kind {
    Overloaded,
    SubstTemplateTemplateParm,
    SubstTemplateTemplateParmPack
  };

  struct BitsTag {
    /// \brief A Kind.
    unsigned Kind : 2;
    
    /// \brief The number of stored templates or template arguments,
    /// depending on which subclass we have.
    unsigned Size : 30;
  };

  union {
    struct BitsTag Bits;
    void *PointerAlignment;
  };
  
  UncommonTemplateNameStorage(Kind kind, unsigned size) {
    Bits.Kind = kind;
    Bits.Size = size;
  }
  
public:
  unsigned size() const { return Bits.Size; }
  
  OverloadedTemplateStorage *getAsOverloadedStorage()  {
    return Bits.Kind == Overloaded
             ? reinterpret_cast<OverloadedTemplateStorage *>(this) 
             : nullptr;
  }
  
  SubstTemplateTemplateParmStorage *getAsSubstTemplateTemplateParm() {
    return Bits.Kind == SubstTemplateTemplateParm
             ? reinterpret_cast<SubstTemplateTemplateParmStorage *>(this)
             : nullptr;
  }

  SubstTemplateTemplateParmPackStorage *getAsSubstTemplateTemplateParmPack() {
    return Bits.Kind == SubstTemplateTemplateParmPack
             ? reinterpret_cast<SubstTemplateTemplateParmPackStorage *>(this)
             : nullptr;
  }
};
  
/// \brief A structure for storing the information associated with an
/// overloaded template name.
class OverloadedTemplateStorage : public UncommonTemplateNameStorage {
  friend class ASTContext;

  OverloadedTemplateStorage(unsigned size) 
    : UncommonTemplateNameStorage(Overloaded, size) { }

  NamedDecl **getStorage() {
    return reinterpret_cast<NamedDecl **>(this + 1);
  }
  NamedDecl * const *getStorage() const {
    return reinterpret_cast<NamedDecl *const *>(this + 1);
  }

public:
  typedef NamedDecl *const *iterator;

  iterator begin() const { return getStorage(); }
  iterator end() const { return getStorage() + size(); }
};

/// \brief A structure for storing an already-substituted template template
/// parameter pack.
///
/// This kind of template names occurs when the parameter pack has been 
/// provided with a template template argument pack in a context where its
/// enclosing pack expansion could not be fully expanded.
class SubstTemplateTemplateParmPackStorage
  : public UncommonTemplateNameStorage, public llvm::FoldingSetNode
{
  TemplateTemplateParmDecl *Parameter;
  const TemplateArgument *Arguments;
  
public:
  SubstTemplateTemplateParmPackStorage(TemplateTemplateParmDecl *Parameter,
                                       unsigned Size, 
                                       const TemplateArgument *Arguments)
    : UncommonTemplateNameStorage(SubstTemplateTemplateParmPack, Size),
      Parameter(Parameter), Arguments(Arguments) { }
  
  /// \brief Retrieve the template template parameter pack being substituted.
  TemplateTemplateParmDecl *getParameterPack() const {
    return Parameter;
  }
  
  /// \brief Retrieve the template template argument pack with which this
  /// parameter was substituted.
  TemplateArgument getArgumentPack() const;
  
  void Profile(llvm::FoldingSetNodeID &ID, ASTContext &Context);
  
  static void Profile(llvm::FoldingSetNodeID &ID,
                      ASTContext &Context,
                      TemplateTemplateParmDecl *Parameter,
                      const TemplateArgument &ArgPack);
};

/// \brief Represents a C++ template name within the type system.
///
/// A C++ template name refers to a template within the C++ type
/// system. In most cases, a template name is simply a reference to a
/// class template, e.g.
///
/// \code
/// template<typename T> class X { };
///
/// X<int> xi;
/// \endcode
///
/// Here, the 'X' in \c X<int> is a template name that refers to the
/// declaration of the class template X, above. Template names can
/// also refer to function templates, C++0x template aliases, etc.
///
/// Some template names are dependent. For example, consider:
///
/// \code
/// template<typename MetaFun, typename T1, typename T2> struct apply2 {
///   typedef typename MetaFun::template apply<T1, T2>::type type;
/// };
/// \endcode
///
/// Here, "apply" is treated as a template name within the typename
/// specifier in the typedef. "apply" is a nested template, and can
/// only be understood in the context of
class TemplateName {
  typedef llvm::PointerUnion4<TemplateDecl *,
                              UncommonTemplateNameStorage *,
                              QualifiedTemplateName *,
                              DependentTemplateName *> StorageType;

  StorageType Storage;

  explicit TemplateName(void *Ptr) {
    Storage = StorageType::getFromOpaqueValue(Ptr);
  }

public:
  // \brief Kind of name that is actually stored.
  enum NameKind {
    /// \brief A single template declaration.
    Template,
    /// \brief A set of overloaded template declarations.
    OverloadedTemplate,
    /// \brief A qualified template name, where the qualification is kept 
    /// to describe the source code as written.
    QualifiedTemplate,
    /// \brief A dependent template name that has not been resolved to a 
    /// template (or set of templates).
    DependentTemplate,
    /// \brief A template template parameter that has been substituted
    /// for some other template name.
    SubstTemplateTemplateParm,
    /// \brief A template template parameter pack that has been substituted for 
    /// a template template argument pack, but has not yet been expanded into
    /// individual arguments.
    SubstTemplateTemplateParmPack
  };

  TemplateName() : Storage() { }
  explicit TemplateName(TemplateDecl *Template) : Storage(Template) { }
  explicit TemplateName(OverloadedTemplateStorage *Storage)
    : Storage(Storage) { }
  explicit TemplateName(SubstTemplateTemplateParmStorage *Storage);
  explicit TemplateName(SubstTemplateTemplateParmPackStorage *Storage)
    : Storage(Storage) { }
  explicit TemplateName(QualifiedTemplateName *Qual) : Storage(Qual) { }
  explicit TemplateName(DependentTemplateName *Dep) : Storage(Dep) { }

  /// \brief Determine whether this template name is NULL.
  bool isNull() const { return Storage.isNull(); }
  
  // \brief Get the kind of name that is actually stored.
  NameKind getKind() const;

  /// \brief Retrieve the underlying template declaration that
  /// this template name refers to, if known.
  ///
  /// \returns The template declaration that this template name refers
  /// to, if any. If the template name does not refer to a specific
  /// declaration because it is a dependent name, or if it refers to a
  /// set of function templates, returns NULL.
  TemplateDecl *getAsTemplateDecl() const;

  /// \brief Retrieve the underlying, overloaded function template
  // declarations that this template name refers to, if known.
  ///
  /// \returns The set of overloaded function templates that this template
  /// name refers to, if known. If the template name does not refer to a
  /// specific set of function templates because it is a dependent name or
  /// refers to a single template, returns NULL.
  OverloadedTemplateStorage *getAsOverloadedTemplate() const {
    if (UncommonTemplateNameStorage *Uncommon = 
                              Storage.dyn_cast<UncommonTemplateNameStorage *>())
      return Uncommon->getAsOverloadedStorage();
    
    return nullptr;
  }

  /// \brief Retrieve the substituted template template parameter, if 
  /// known.
  ///
  /// \returns The storage for the substituted template template parameter,
  /// if known. Otherwise, returns NULL.
  SubstTemplateTemplateParmStorage *getAsSubstTemplateTemplateParm() const {
    if (UncommonTemplateNameStorage *uncommon = 
          Storage.dyn_cast<UncommonTemplateNameStorage *>())
      return uncommon->getAsSubstTemplateTemplateParm();
    
    return nullptr;
  }

  /// \brief Retrieve the substituted template template parameter pack, if 
  /// known.
  ///
  /// \returns The storage for the substituted template template parameter pack,
  /// if known. Otherwise, returns NULL.
  SubstTemplateTemplateParmPackStorage *
  getAsSubstTemplateTemplateParmPack() const {
    if (UncommonTemplateNameStorage *Uncommon = 
        Storage.dyn_cast<UncommonTemplateNameStorage *>())
      return Uncommon->getAsSubstTemplateTemplateParmPack();
    
    return nullptr;
  }

  /// \brief Retrieve the underlying qualified template name
  /// structure, if any.
  QualifiedTemplateName *getAsQualifiedTemplateName() const {
    return Storage.dyn_cast<QualifiedTemplateName *>();
  }

  /// \brief Retrieve the underlying dependent template name
  /// structure, if any.
  DependentTemplateName *getAsDependentTemplateName() const {
    return Storage.dyn_cast<DependentTemplateName *>();
  }

  TemplateName getUnderlying() const;

  /// \brief Determines whether this is a dependent template name.
  bool isDependent() const;

  /// \brief Determines whether this is a template name that somehow
  /// depends on a template parameter.
  bool isInstantiationDependent() const;

  /// \brief Determines whether this template name contains an
  /// unexpanded parameter pack (for C++0x variadic templates).
  bool containsUnexpandedParameterPack() const;

  /// \brief Print the template name.
  ///
  /// \param OS the output stream to which the template name will be
  /// printed.
  ///
  /// \param SuppressNNS if true, don't print the
  /// nested-name-specifier that precedes the template name (if it has
  /// one).
  void print(raw_ostream &OS, const PrintingPolicy &Policy,
             bool SuppressNNS = false) const;

  /// \brief Debugging aid that dumps the template name.
  void dump(raw_ostream &OS) const;

  /// \brief Debugging aid that dumps the template name to standard
  /// error.
  void dump() const;

  void Profile(llvm::FoldingSetNodeID &ID) {
    ID.AddPointer(Storage.getOpaqueValue());
  }

  /// \brief Retrieve the template name as a void pointer.
  void *getAsVoidPointer() const { return Storage.getOpaqueValue(); }

  /// \brief Build a template name from a void pointer.
  static TemplateName getFromVoidPointer(void *Ptr) {
    return TemplateName(Ptr);
  }
};

/// Insertion operator for diagnostics.  This allows sending TemplateName's
/// into a diagnostic with <<.
const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                    TemplateName N);

/// \brief A structure for storing the information associated with a
/// substituted template template parameter.
class SubstTemplateTemplateParmStorage
  : public UncommonTemplateNameStorage, public llvm::FoldingSetNode {
  friend class ASTContext;

  TemplateTemplateParmDecl *Parameter;
  TemplateName Replacement;

  SubstTemplateTemplateParmStorage(TemplateTemplateParmDecl *parameter,
                                   TemplateName replacement)
    : UncommonTemplateNameStorage(SubstTemplateTemplateParm, 0),
      Parameter(parameter), Replacement(replacement) {}

public:
  TemplateTemplateParmDecl *getParameter() const { return Parameter; }
  TemplateName getReplacement() const { return Replacement; }

  void Profile(llvm::FoldingSetNodeID &ID);
  
  static void Profile(llvm::FoldingSetNodeID &ID,
                      TemplateTemplateParmDecl *parameter,
                      TemplateName replacement);
};

inline TemplateName::TemplateName(SubstTemplateTemplateParmStorage *Storage)
  : Storage(Storage) { }

inline TemplateName TemplateName::getUnderlying() const {
  if (SubstTemplateTemplateParmStorage *subst
        = getAsSubstTemplateTemplateParm())
    return subst->getReplacement().getUnderlying();
  return *this;
}

/// \brief Represents a template name that was expressed as a
/// qualified name.
///
/// This kind of template name refers to a template name that was
/// preceded by a nested name specifier, e.g., \c std::vector. Here,
/// the nested name specifier is "std::" and the template name is the
/// declaration for "vector". The QualifiedTemplateName class is only
/// used to provide "sugar" for template names that were expressed
/// with a qualified name, and has no semantic meaning. In this
/// manner, it is to TemplateName what ElaboratedType is to Type,
/// providing extra syntactic sugar for downstream clients.
class QualifiedTemplateName : public llvm::FoldingSetNode {
  /// \brief The nested name specifier that qualifies the template name.
  ///
  /// The bit is used to indicate whether the "template" keyword was
  /// present before the template name itself. Note that the
  /// "template" keyword is always redundant in this case (otherwise,
  /// the template name would be a dependent name and we would express
  /// this name with DependentTemplateName).
  llvm::PointerIntPair<NestedNameSpecifier *, 1> Qualifier;

  /// \brief The template declaration or set of overloaded function templates
  /// that this qualified name refers to.
  TemplateDecl *Template;

  friend class ASTContext;

  QualifiedTemplateName(NestedNameSpecifier *NNS, bool TemplateKeyword,
                        TemplateDecl *Template)
    : Qualifier(NNS, TemplateKeyword? 1 : 0),
      Template(Template) { }

public:
  /// \brief Return the nested name specifier that qualifies this name.
  NestedNameSpecifier *getQualifier() const { return Qualifier.getPointer(); }

  /// \brief Whether the template name was prefixed by the "template"
  /// keyword.
  bool hasTemplateKeyword() const { return Qualifier.getInt(); }

  /// \brief The template declaration that this qualified name refers
  /// to.
  TemplateDecl *getDecl() const { return Template; }

  /// \brief The template declaration to which this qualified name
  /// refers.
  TemplateDecl *getTemplateDecl() const { return Template; }

  void Profile(llvm::FoldingSetNodeID &ID) {
    Profile(ID, getQualifier(), hasTemplateKeyword(), getTemplateDecl());
  }

  static void Profile(llvm::FoldingSetNodeID &ID, NestedNameSpecifier *NNS,
                      bool TemplateKeyword, TemplateDecl *Template) {
    ID.AddPointer(NNS);
    ID.AddBoolean(TemplateKeyword);
    ID.AddPointer(Template);
  }
};

/// \brief Represents a dependent template name that cannot be
/// resolved prior to template instantiation.
///
/// This kind of template name refers to a dependent template name,
/// including its nested name specifier (if any). For example,
/// DependentTemplateName can refer to "MetaFun::template apply",
/// where "MetaFun::" is the nested name specifier and "apply" is the
/// template name referenced. The "template" keyword is implied.
class DependentTemplateName : public llvm::FoldingSetNode {
  /// \brief The nested name specifier that qualifies the template
  /// name.
  ///
  /// The bit stored in this qualifier describes whether the \c Name field
  /// is interpreted as an IdentifierInfo pointer (when clear) or as an
  /// overloaded operator kind (when set).
  llvm::PointerIntPair<NestedNameSpecifier *, 1, bool> Qualifier;

  /// \brief The dependent template name.
  union {
    /// \brief The identifier template name.
    ///
    /// Only valid when the bit on \c Qualifier is clear.
    const IdentifierInfo *Identifier;
    
    /// \brief The overloaded operator name.
    ///
    /// Only valid when the bit on \c Qualifier is set.
    OverloadedOperatorKind Operator;
  };

  /// \brief The canonical template name to which this dependent
  /// template name refers.
  ///
  /// The canonical template name for a dependent template name is
  /// another dependent template name whose nested name specifier is
  /// canonical.
  TemplateName CanonicalTemplateName;

  friend class ASTContext;

  DependentTemplateName(NestedNameSpecifier *Qualifier,
                        const IdentifierInfo *Identifier)
    : Qualifier(Qualifier, false), Identifier(Identifier), 
      CanonicalTemplateName(this) { }

  DependentTemplateName(NestedNameSpecifier *Qualifier,
                        const IdentifierInfo *Identifier,
                        TemplateName Canon)
    : Qualifier(Qualifier, false), Identifier(Identifier), 
      CanonicalTemplateName(Canon) { }

  DependentTemplateName(NestedNameSpecifier *Qualifier,
                        OverloadedOperatorKind Operator)
  : Qualifier(Qualifier, true), Operator(Operator), 
    CanonicalTemplateName(this) { }
  
  DependentTemplateName(NestedNameSpecifier *Qualifier,
                        OverloadedOperatorKind Operator,
                        TemplateName Canon)
  : Qualifier(Qualifier, true), Operator(Operator), 
    CanonicalTemplateName(Canon) { }
  
public:
  /// \brief Return the nested name specifier that qualifies this name.
  NestedNameSpecifier *getQualifier() const { return Qualifier.getPointer(); }

  /// \brief Determine whether this template name refers to an identifier.
  bool isIdentifier() const { return !Qualifier.getInt(); }

  /// \brief Returns the identifier to which this template name refers.
  const IdentifierInfo *getIdentifier() const { 
    assert(isIdentifier() && "Template name isn't an identifier?");
    return Identifier;
  }
  
  /// \brief Determine whether this template name refers to an overloaded
  /// operator.
  bool isOverloadedOperator() const { return Qualifier.getInt(); }
  
  /// \brief Return the overloaded operator to which this template name refers.
  OverloadedOperatorKind getOperator() const { 
    assert(isOverloadedOperator() &&
           "Template name isn't an overloaded operator?");
    return Operator; 
  }
  
  void Profile(llvm::FoldingSetNodeID &ID) {
    if (isIdentifier())
      Profile(ID, getQualifier(), getIdentifier());
    else
      Profile(ID, getQualifier(), getOperator());
  }

  static void Profile(llvm::FoldingSetNodeID &ID, NestedNameSpecifier *NNS,
                      const IdentifierInfo *Identifier) {
    ID.AddPointer(NNS);
    ID.AddBoolean(false);
    ID.AddPointer(Identifier);
  }

  static void Profile(llvm::FoldingSetNodeID &ID, NestedNameSpecifier *NNS,
                      OverloadedOperatorKind Operator) {
    ID.AddPointer(NNS);
    ID.AddBoolean(true);
    ID.AddInteger(Operator);
  }
};

} // end namespace clang.

namespace llvm {

/// \brief The clang::TemplateName class is effectively a pointer.
template<>
class PointerLikeTypeTraits<clang::TemplateName> {
public:
  static inline void *getAsVoidPointer(clang::TemplateName TN) {
    return TN.getAsVoidPointer();
  }

  static inline clang::TemplateName getFromVoidPointer(void *Ptr) {
    return clang::TemplateName::getFromVoidPointer(Ptr);
  }

  // No bits are available!
  enum { NumLowBitsAvailable = 0 };
};

} // end namespace llvm.

#endif
