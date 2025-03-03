//===--- Decl.cpp - Declaration AST Node Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl subclasses.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTLambda.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/Support/ErrorHandling.h"
#include <algorithm>

using namespace clang;

Decl *clang::getPrimaryMergedDecl(Decl *D) {
  return D->getASTContext().getPrimaryMergedDecl(D);
}

// Defined here so that it can be inlined into its direct callers.
bool Decl::isOutOfLine() const {
  return !getLexicalDeclContext()->Equals(getDeclContext());
}

TranslationUnitDecl::TranslationUnitDecl(ASTContext &ctx)
    : Decl(TranslationUnit, nullptr, SourceLocation()),
      DeclContext(TranslationUnit), Ctx(ctx), AnonymousNamespace(nullptr) {
  Hidden = Ctx.getLangOpts().ModulesLocalVisibility;
}

//===----------------------------------------------------------------------===//
// NamedDecl Implementation
//===----------------------------------------------------------------------===//

// Visibility rules aren't rigorously externally specified, but here
// are the basic principles behind what we implement:
//
// 1. An explicit visibility attribute is generally a direct expression
// of the user's intent and should be honored.  Only the innermost
// visibility attribute applies.  If no visibility attribute applies,
// global visibility settings are considered.
//
// 2. There is one caveat to the above: on or in a template pattern,
// an explicit visibility attribute is just a default rule, and
// visibility can be decreased by the visibility of template
// arguments.  But this, too, has an exception: an attribute on an
// explicit specialization or instantiation causes all the visibility
// restrictions of the template arguments to be ignored.
//
// 3. A variable that does not otherwise have explicit visibility can
// be restricted by the visibility of its type.
//
// 4. A visibility restriction is explicit if it comes from an
// attribute (or something like it), not a global visibility setting.
// When emitting a reference to an external symbol, visibility
// restrictions are ignored unless they are explicit.
//
// 5. When computing the visibility of a non-type, including a
// non-type member of a class, only non-type visibility restrictions
// are considered: the 'visibility' attribute, global value-visibility
// settings, and a few special cases like __private_extern.
//
// 6. When computing the visibility of a type, including a type member
// of a class, only type visibility restrictions are considered:
// the 'type_visibility' attribute and global type-visibility settings.
// However, a 'visibility' attribute counts as a 'type_visibility'
// attribute on any declaration that only has the former.
//
// The visibility of a "secondary" entity, like a template argument,
// is computed using the kind of that entity, not the kind of the
// primary entity for which we are computing visibility.  For example,
// the visibility of a specialization of either of these templates:
//   template <class T, bool (&compare)(T, X)> bool has_match(list<T>, X);
//   template <class T, bool (&compare)(T, X)> class matcher;
// is restricted according to the type visibility of the argument 'T',
// the type visibility of 'bool(&)(T,X)', and the value visibility of
// the argument function 'compare'.  That 'has_match' is a value
// and 'matcher' is a type only matters when looking for attributes
// and settings from the immediate context.

const unsigned IgnoreExplicitVisibilityBit = 2;
const unsigned IgnoreAllVisibilityBit = 4;

/// Kinds of LV computation.  The linkage side of the computation is
/// always the same, but different things can change how visibility is
/// computed.
enum LVComputationKind {
  /// Do an LV computation for, ultimately, a type.
  /// Visibility may be restricted by type visibility settings and
  /// the visibility of template arguments.
  LVForType = NamedDecl::VisibilityForType,

  /// Do an LV computation for, ultimately, a non-type declaration.
  /// Visibility may be restricted by value visibility settings and
  /// the visibility of template arguments.
  LVForValue = NamedDecl::VisibilityForValue,

  /// Do an LV computation for, ultimately, a type that already has
  /// some sort of explicit visibility.  Visibility may only be
  /// restricted by the visibility of template arguments.
  LVForExplicitType = (LVForType | IgnoreExplicitVisibilityBit),

  /// Do an LV computation for, ultimately, a non-type declaration
  /// that already has some sort of explicit visibility.  Visibility
  /// may only be restricted by the visibility of template arguments.
  LVForExplicitValue = (LVForValue | IgnoreExplicitVisibilityBit),

  /// Do an LV computation when we only care about the linkage.
  LVForLinkageOnly =
      LVForValue | IgnoreExplicitVisibilityBit | IgnoreAllVisibilityBit
};

/// Does this computation kind permit us to consider additional
/// visibility settings from attributes and the like?
static bool hasExplicitVisibilityAlready(LVComputationKind computation) {
  return ((unsigned(computation) & IgnoreExplicitVisibilityBit) != 0);
}

/// Given an LVComputationKind, return one of the same type/value sort
/// that records that it already has explicit visibility.
static LVComputationKind
withExplicitVisibilityAlready(LVComputationKind oldKind) {
  LVComputationKind newKind =
    static_cast<LVComputationKind>(unsigned(oldKind) |
                                   IgnoreExplicitVisibilityBit);
  assert(oldKind != LVForType          || newKind == LVForExplicitType);
  assert(oldKind != LVForValue         || newKind == LVForExplicitValue);
  assert(oldKind != LVForExplicitType  || newKind == LVForExplicitType);
  assert(oldKind != LVForExplicitValue || newKind == LVForExplicitValue);
  return newKind;
}

static Optional<Visibility> getExplicitVisibility(const NamedDecl *D,
                                                  LVComputationKind kind) {
  assert(!hasExplicitVisibilityAlready(kind) &&
         "asking for explicit visibility when we shouldn't be");
  return D->getExplicitVisibility((NamedDecl::ExplicitVisibilityKind) kind);
}

/// Is the given declaration a "type" or a "value" for the purposes of
/// visibility computation?
static bool usesTypeVisibility(const NamedDecl *D) {
  return isa<TypeDecl>(D) ||
         isa<ClassTemplateDecl>(D) ||
         isa<ObjCInterfaceDecl>(D);
}

/// Does the given declaration have member specialization information,
/// and if so, is it an explicit specialization?
template <class T> static typename
std::enable_if<!std::is_base_of<RedeclarableTemplateDecl, T>::value, bool>::type
isExplicitMemberSpecialization(const T *D) {
  if (const MemberSpecializationInfo *member =
        D->getMemberSpecializationInfo()) {
    return member->isExplicitSpecialization();
  }
  return false;
}

/// For templates, this question is easier: a member template can't be
/// explicitly instantiated, so there's a single bit indicating whether
/// or not this is an explicit member specialization.
static bool isExplicitMemberSpecialization(const RedeclarableTemplateDecl *D) {
  return D->isMemberSpecialization();
}

/// Given a visibility attribute, return the explicit visibility
/// associated with it.
template <class T>
static Visibility getVisibilityFromAttr(const T *attr) {
  switch (attr->getVisibility()) {
  case T::Default:
    return DefaultVisibility;
  case T::Hidden:
    return HiddenVisibility;
  case T::Protected:
    return ProtectedVisibility;
  }
  llvm_unreachable("bad visibility kind");
}

/// Return the explicit visibility of the given declaration.
static Optional<Visibility> getVisibilityOf(const NamedDecl *D,
                                    NamedDecl::ExplicitVisibilityKind kind) {
  // If we're ultimately computing the visibility of a type, look for
  // a 'type_visibility' attribute before looking for 'visibility'.
  if (kind == NamedDecl::VisibilityForType) {
    if (const TypeVisibilityAttr *A = D->getAttr<TypeVisibilityAttr>()) {
      return getVisibilityFromAttr(A);
    }
  }

  // If this declaration has an explicit visibility attribute, use it.
  if (const VisibilityAttr *A = D->getAttr<VisibilityAttr>()) {
    return getVisibilityFromAttr(A);
  }

  // If we're on Mac OS X, an 'availability' for Mac OS X attribute
  // implies visibility(default).
  if (D->getASTContext().getTargetInfo().getTriple().isOSDarwin()) {
    for (const auto *A : D->specific_attrs<AvailabilityAttr>())
      if (A->getPlatform()->getName().equals("macosx"))
        return DefaultVisibility;
  }

  return None;
}

static LinkageInfo
getLVForType(const Type &T, LVComputationKind computation) {
  if (computation == LVForLinkageOnly)
    return LinkageInfo(T.getLinkage(), DefaultVisibility, true);
  return T.getLinkageAndVisibility();
}

/// \brief Get the most restrictive linkage for the types in the given
/// template parameter list.  For visibility purposes, template
/// parameters are part of the signature of a template.
static LinkageInfo
getLVForTemplateParameterList(const TemplateParameterList *Params,
                              LVComputationKind computation) {
  LinkageInfo LV;
  for (const NamedDecl *P : *Params) {
    // Template type parameters are the most common and never
    // contribute to visibility, pack or not.
    if (isa<TemplateTypeParmDecl>(P))
      continue;

    // Non-type template parameters can be restricted by the value type, e.g.
    //   template <enum X> class A { ... };
    // We have to be careful here, though, because we can be dealing with
    // dependent types.
    if (const NonTypeTemplateParmDecl *NTTP =
            dyn_cast<NonTypeTemplateParmDecl>(P)) {
      // Handle the non-pack case first.
      if (!NTTP->isExpandedParameterPack()) {
        if (!NTTP->getType()->isDependentType()) {
          LV.merge(getLVForType(*NTTP->getType(), computation));
        }
        continue;
      }

      // Look at all the types in an expanded pack.
      for (unsigned i = 0, n = NTTP->getNumExpansionTypes(); i != n; ++i) {
        QualType type = NTTP->getExpansionType(i);
        if (!type->isDependentType())
          LV.merge(type->getLinkageAndVisibility());
      }
      continue;
    }

    // Template template parameters can be restricted by their
    // template parameters, recursively.
    const TemplateTemplateParmDecl *TTP = cast<TemplateTemplateParmDecl>(P);

    // Handle the non-pack case first.
    if (!TTP->isExpandedParameterPack()) {
      LV.merge(getLVForTemplateParameterList(TTP->getTemplateParameters(),
                                             computation));
      continue;
    }

    // Look at all expansions in an expanded pack.
    for (unsigned i = 0, n = TTP->getNumExpansionTemplateParameters();
           i != n; ++i) {
      LV.merge(getLVForTemplateParameterList(
          TTP->getExpansionTemplateParameters(i), computation));
    }
  }

  return LV;
}

/// getLVForDecl - Get the linkage and visibility for the given declaration.
static LinkageInfo getLVForDecl(const NamedDecl *D,
                                LVComputationKind computation);

static const Decl *getOutermostFuncOrBlockContext(const Decl *D) {
  const Decl *Ret = nullptr;
  const DeclContext *DC = D->getDeclContext();
  while (DC->getDeclKind() != Decl::TranslationUnit) {
    if (isa<FunctionDecl>(DC) || isa<BlockDecl>(DC))
      Ret = cast<Decl>(DC);
    DC = DC->getParent();
  }
  return Ret;
}

/// \brief Get the most restrictive linkage for the types and
/// declarations in the given template argument list.
///
/// Note that we don't take an LVComputationKind because we always
/// want to honor the visibility of template arguments in the same way.
static LinkageInfo getLVForTemplateArgumentList(ArrayRef<TemplateArgument> Args,
                                                LVComputationKind computation) {
  LinkageInfo LV;

  for (const TemplateArgument &Arg : Args) {
    switch (Arg.getKind()) {
    case TemplateArgument::Null:
    case TemplateArgument::Integral:
    case TemplateArgument::Expression:
      continue;

    case TemplateArgument::Type:
      LV.merge(getLVForType(*Arg.getAsType(), computation));
      continue;

    case TemplateArgument::Declaration:
      if (NamedDecl *ND = dyn_cast<NamedDecl>(Arg.getAsDecl())) {
        assert(!usesTypeVisibility(ND));
        LV.merge(getLVForDecl(ND, computation));
      }
      continue;

    case TemplateArgument::NullPtr:
      LV.merge(Arg.getNullPtrType()->getLinkageAndVisibility());
      continue;

    case TemplateArgument::Template:
    case TemplateArgument::TemplateExpansion:
      if (TemplateDecl *Template =
              Arg.getAsTemplateOrTemplatePattern().getAsTemplateDecl())
        LV.merge(getLVForDecl(Template, computation));
      continue;

    case TemplateArgument::Pack:
      LV.merge(getLVForTemplateArgumentList(Arg.getPackAsArray(), computation));
      continue;
    }
    llvm_unreachable("bad template argument kind");
  }

  return LV;
}

static LinkageInfo
getLVForTemplateArgumentList(const TemplateArgumentList &TArgs,
                             LVComputationKind computation) {
  return getLVForTemplateArgumentList(TArgs.asArray(), computation);
}

static bool shouldConsiderTemplateVisibility(const FunctionDecl *fn,
                        const FunctionTemplateSpecializationInfo *specInfo) {
  // Include visibility from the template parameters and arguments
  // only if this is not an explicit instantiation or specialization
  // with direct explicit visibility.  (Implicit instantiations won't
  // have a direct attribute.)
  if (!specInfo->isExplicitInstantiationOrSpecialization())
    return true;

  return !fn->hasAttr<VisibilityAttr>();
}

/// Merge in template-related linkage and visibility for the given
/// function template specialization.
///
/// We don't need a computation kind here because we can assume
/// LVForValue.
///
/// \param[out] LV the computation to use for the parent
static void
mergeTemplateLV(LinkageInfo &LV, const FunctionDecl *fn,
                const FunctionTemplateSpecializationInfo *specInfo,
                LVComputationKind computation) {
  bool considerVisibility =
    shouldConsiderTemplateVisibility(fn, specInfo);

  // Merge information from the template parameters.
  FunctionTemplateDecl *temp = specInfo->getTemplate();
  LinkageInfo tempLV =
    getLVForTemplateParameterList(temp->getTemplateParameters(), computation);
  LV.mergeMaybeWithVisibility(tempLV, considerVisibility);

  // Merge information from the template arguments.
  const TemplateArgumentList &templateArgs = *specInfo->TemplateArguments;
  LinkageInfo argsLV = getLVForTemplateArgumentList(templateArgs, computation);
  LV.mergeMaybeWithVisibility(argsLV, considerVisibility);
}

/// Does the given declaration have a direct visibility attribute
/// that would match the given rules?
static bool hasDirectVisibilityAttribute(const NamedDecl *D,
                                         LVComputationKind computation) {
  switch (computation) {
  case LVForType:
  case LVForExplicitType:
    if (D->hasAttr<TypeVisibilityAttr>())
      return true;
    LLVM_FALLTHROUGH; // HLSL Change
  case LVForValue:
  case LVForExplicitValue:
    if (D->hasAttr<VisibilityAttr>())
      return true;
    return false;
  case LVForLinkageOnly:
    return false;
  }
  llvm_unreachable("bad visibility computation kind");
}

/// Should we consider visibility associated with the template
/// arguments and parameters of the given class template specialization?
static bool shouldConsiderTemplateVisibility(
                                 const ClassTemplateSpecializationDecl *spec,
                                 LVComputationKind computation) {
  // Include visibility from the template parameters and arguments
  // only if this is not an explicit instantiation or specialization
  // with direct explicit visibility (and note that implicit
  // instantiations won't have a direct attribute).
  //
  // Furthermore, we want to ignore template parameters and arguments
  // for an explicit specialization when computing the visibility of a
  // member thereof with explicit visibility.
  //
  // This is a bit complex; let's unpack it.
  //
  // An explicit class specialization is an independent, top-level
  // declaration.  As such, if it or any of its members has an
  // explicit visibility attribute, that must directly express the
  // user's intent, and we should honor it.  The same logic applies to
  // an explicit instantiation of a member of such a thing.

  // Fast path: if this is not an explicit instantiation or
  // specialization, we always want to consider template-related
  // visibility restrictions.
  if (!spec->isExplicitInstantiationOrSpecialization())
    return true;

  // This is the 'member thereof' check.
  if (spec->isExplicitSpecialization() &&
      hasExplicitVisibilityAlready(computation))
    return false;

  return !hasDirectVisibilityAttribute(spec, computation);
}

/// Merge in template-related linkage and visibility for the given
/// class template specialization.
static void mergeTemplateLV(LinkageInfo &LV,
                            const ClassTemplateSpecializationDecl *spec,
                            LVComputationKind computation) {
  bool considerVisibility = shouldConsiderTemplateVisibility(spec, computation);

  // Merge information from the template parameters, but ignore
  // visibility if we're only considering template arguments.

  ClassTemplateDecl *temp = spec->getSpecializedTemplate();
  LinkageInfo tempLV =
    getLVForTemplateParameterList(temp->getTemplateParameters(), computation);
  LV.mergeMaybeWithVisibility(tempLV,
           considerVisibility && !hasExplicitVisibilityAlready(computation));

  // Merge information from the template arguments.  We ignore
  // template-argument visibility if we've got an explicit
  // instantiation with a visibility attribute.
  const TemplateArgumentList &templateArgs = spec->getTemplateArgs();
  LinkageInfo argsLV = getLVForTemplateArgumentList(templateArgs, computation);
  if (considerVisibility)
    LV.mergeVisibility(argsLV);
  LV.mergeExternalVisibility(argsLV);
}

/// Should we consider visibility associated with the template
/// arguments and parameters of the given variable template
/// specialization? As usual, follow class template specialization
/// logic up to initialization.
static bool shouldConsiderTemplateVisibility(
                                 const VarTemplateSpecializationDecl *spec,
                                 LVComputationKind computation) {
  // Include visibility from the template parameters and arguments
  // only if this is not an explicit instantiation or specialization
  // with direct explicit visibility (and note that implicit
  // instantiations won't have a direct attribute).
  if (!spec->isExplicitInstantiationOrSpecialization())
    return true;

  // An explicit variable specialization is an independent, top-level
  // declaration.  As such, if it has an explicit visibility attribute,
  // that must directly express the user's intent, and we should honor
  // it.
  if (spec->isExplicitSpecialization() &&
      hasExplicitVisibilityAlready(computation))
    return false;

  return !hasDirectVisibilityAttribute(spec, computation);
}

/// Merge in template-related linkage and visibility for the given
/// variable template specialization. As usual, follow class template
/// specialization logic up to initialization.
static void mergeTemplateLV(LinkageInfo &LV,
                            const VarTemplateSpecializationDecl *spec,
                            LVComputationKind computation) {
  bool considerVisibility = shouldConsiderTemplateVisibility(spec, computation);

  // Merge information from the template parameters, but ignore
  // visibility if we're only considering template arguments.

  VarTemplateDecl *temp = spec->getSpecializedTemplate();
  LinkageInfo tempLV =
    getLVForTemplateParameterList(temp->getTemplateParameters(), computation);
  LV.mergeMaybeWithVisibility(tempLV,
           considerVisibility && !hasExplicitVisibilityAlready(computation));

  // Merge information from the template arguments.  We ignore
  // template-argument visibility if we've got an explicit
  // instantiation with a visibility attribute.
  const TemplateArgumentList &templateArgs = spec->getTemplateArgs();
  LinkageInfo argsLV = getLVForTemplateArgumentList(templateArgs, computation);
  if (considerVisibility)
    LV.mergeVisibility(argsLV);
  LV.mergeExternalVisibility(argsLV);
}

static bool useInlineVisibilityHidden(const NamedDecl *D) {
#if 1 // HLSL Change Starts
  return false;
#else
  // FIXME: we should warn if -fvisibility-inlines-hidden is used with c.
  const LangOptions &Opts = D->getASTContext().getLangOpts();
  if (!Opts.CPlusPlus || !Opts.InlineVisibilityHidden)
    return false;

  const FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
  if (!FD)
    return false;

  TemplateSpecializationKind TSK = TSK_Undeclared;
  if (FunctionTemplateSpecializationInfo *spec
      = FD->getTemplateSpecializationInfo()) {
    TSK = spec->getTemplateSpecializationKind();
  } else if (MemberSpecializationInfo *MSI =
             FD->getMemberSpecializationInfo()) {
    TSK = MSI->getTemplateSpecializationKind();
  }

  const FunctionDecl *Def = nullptr;
  // InlineVisibilityHidden only applies to definitions, and
  // isInlined() only gives meaningful answers on definitions
  // anyway.
  return TSK != TSK_ExplicitInstantiationDeclaration &&
    TSK != TSK_ExplicitInstantiationDefinition &&
    FD->hasBody(Def) && Def->isInlined() && !Def->hasAttr<GNUInlineAttr>();
#endif // HLSL Change Ends
}

template <typename T> static bool isFirstInExternCContext(T *D) {
  const T *First = D->getFirstDecl();
  return First->isInExternCContext();
}

static bool isSingleLineLanguageLinkage(const Decl &D) {
  if (const LinkageSpecDecl *SD = dyn_cast<LinkageSpecDecl>(D.getDeclContext()))
    if (!SD->hasBraces())
      return true;
  return false;
}

static LinkageInfo getLVForNamespaceScopeDecl(const NamedDecl *D,
                                              LVComputationKind computation) {
  assert(D->getDeclContext()->getRedeclContext()->isFileContext() &&
         "Not a name having namespace scope");
  ASTContext &Context = D->getASTContext();

  // C++ [basic.link]p3:
  //   A name having namespace scope (3.3.6) has internal linkage if it
  //   is the name of
  //     - an object, reference, function or function template that is
  //       explicitly declared static; or,
  // (This bullet corresponds to C99 6.2.2p3.)
  if (const VarDecl *Var = dyn_cast<VarDecl>(D)) {
    // Explicitly declared static.
    if (Var->getStorageClass() == SC_Static)
      return LinkageInfo::internal();

    // - a non-volatile object or reference that is explicitly declared const
    //   or constexpr and neither explicitly declared extern nor previously
    //   declared to have external linkage; or (there is no equivalent in C99)
    if (Context.getLangOpts().CPlusPlus &&
        Var->getType().isConstQualified() && 
        !Context.getLangOpts().HLSL && // HLSL Change -Initializer used on a global 'const' variable will be ignored for hlsl.
        !Var->getType().isVolatileQualified()) {
      const VarDecl *PrevVar = Var->getPreviousDecl();
      if (PrevVar)
        return getLVForDecl(PrevVar, computation);

      if (Var->getStorageClass() != SC_Extern &&
          Var->getStorageClass() != SC_PrivateExtern &&
          !isSingleLineLanguageLinkage(*Var))
        return LinkageInfo::internal();
    }

    for (const VarDecl *PrevVar = Var->getPreviousDecl(); PrevVar;
         PrevVar = PrevVar->getPreviousDecl()) {
      if (PrevVar->getStorageClass() == SC_PrivateExtern &&
          Var->getStorageClass() == SC_None)
        return PrevVar->getLinkageAndVisibility();
      // Explicitly declared static.
      if (PrevVar->getStorageClass() == SC_Static)
        return LinkageInfo::internal();
    }
  } else if (const FunctionDecl *Function = D->getAsFunction()) {
    // C++ [temp]p4:
    //   A non-member function template can have internal linkage; any
    //   other template name shall have external linkage.

    // Explicitly declared static.
    if (Function->getCanonicalDecl()->getStorageClass() == SC_Static)
      return LinkageInfo(InternalLinkage, DefaultVisibility, false);
  } else if (const auto *IFD = dyn_cast<IndirectFieldDecl>(D)) {
    //   - a data member of an anonymous union.
    const VarDecl *VD = IFD->getVarDecl();
    assert(VD && "Expected a VarDecl in this IndirectFieldDecl!");
    return getLVForNamespaceScopeDecl(VD, computation);
  }
  assert(!isa<FieldDecl>(D) && "Didn't expect a FieldDecl!");

  if (D->isInAnonymousNamespace()) {
    const VarDecl *Var = dyn_cast<VarDecl>(D);
    const FunctionDecl *Func = dyn_cast<FunctionDecl>(D);
    if ((!Var || !isFirstInExternCContext(Var)) &&
        (!Func || !isFirstInExternCContext(Func)))
      return LinkageInfo::uniqueExternal();
  }

  // Set up the defaults.

  // C99 6.2.2p5:
  //   If the declaration of an identifier for an object has file
  //   scope and no storage-class specifier, its linkage is
  //   external.
  LinkageInfo LV;

  if (!hasExplicitVisibilityAlready(computation)) {
    if (Optional<Visibility> Vis = getExplicitVisibility(D, computation)) {
      LV.mergeVisibility(*Vis, true);
    } else {
      // If we're declared in a namespace with a visibility attribute,
      // use that namespace's visibility, and it still counts as explicit.
      for (const DeclContext *DC = D->getDeclContext();
           !isa<TranslationUnitDecl>(DC);
           DC = DC->getParent()) {
        const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(DC);
        if (!ND) continue;
        if (Optional<Visibility> Vis = getExplicitVisibility(ND, computation)) {
          LV.mergeVisibility(*Vis, true);
          break;
        }
      }
    }

    // Add in global settings if the above didn't give us direct visibility.
    if (!LV.isVisibilityExplicit()) {
      // Use global type/value visibility as appropriate.
      Visibility globalVisibility;
      if (computation == LVForValue) {
        globalVisibility = Context.getLangOpts().getValueVisibilityMode();
      } else {
        assert(computation == LVForType);
        globalVisibility = Context.getLangOpts().getTypeVisibilityMode();
      }
      LV.mergeVisibility(globalVisibility, /*explicit*/ false);

      // If we're paying attention to global visibility, apply
      // -finline-visibility-hidden if this is an inline method.
      if (useInlineVisibilityHidden(D))
        LV.mergeVisibility(HiddenVisibility, true);
    }
  }

  // C++ [basic.link]p4:

  //   A name having namespace scope has external linkage if it is the
  //   name of
  //
  //     - an object or reference, unless it has internal linkage; or
  if (const VarDecl *Var = dyn_cast<VarDecl>(D)) {
    // GCC applies the following optimization to variables and static
    // data members, but not to functions:
    //
    // Modify the variable's LV by the LV of its type unless this is
    // C or extern "C".  This follows from [basic.link]p9:
    //   A type without linkage shall not be used as the type of a
    //   variable or function with external linkage unless
    //    - the entity has C language linkage, or
    //    - the entity is declared within an unnamed namespace, or
    //    - the entity is not used or is defined in the same
    //      translation unit.
    // and [basic.link]p10:
    //   ...the types specified by all declarations referring to a
    //   given variable or function shall be identical...
    // C does not have an equivalent rule.
    //
    // Ignore this if we've got an explicit attribute;  the user
    // probably knows what they're doing.
    //
    // Note that we don't want to make the variable non-external
    // because of this, but unique-external linkage suits us.
    if (Context.getLangOpts().CPlusPlus && !isFirstInExternCContext(Var)) {
      LinkageInfo TypeLV = getLVForType(*Var->getType(), computation);
      if (TypeLV.getLinkage() != ExternalLinkage)
        return LinkageInfo::uniqueExternal();
      if (!LV.isVisibilityExplicit())
        LV.mergeVisibility(TypeLV);
    }

    if (Var->getStorageClass() == SC_PrivateExtern)
      LV.mergeVisibility(HiddenVisibility, true);

    // Note that Sema::MergeVarDecl already takes care of implementing
    // C99 6.2.2p4 and propagating the visibility attribute, so we don't have
    // to do it here.

    // As per function and class template specializations (below),
    // consider LV for the template and template arguments.  We're at file
    // scope, so we do not need to worry about nested specializations.
    if (const VarTemplateSpecializationDecl *spec
              = dyn_cast<VarTemplateSpecializationDecl>(Var)) {
      mergeTemplateLV(LV, spec, computation);
    }

  //     - a function, unless it has internal linkage; or
  } else if (const FunctionDecl *Function = dyn_cast<FunctionDecl>(D)) {
    // In theory, we can modify the function's LV by the LV of its
    // type unless it has C linkage (see comment above about variables
    // for justification).  In practice, GCC doesn't do this, so it's
    // just too painful to make work.

    if (Function->getStorageClass() == SC_PrivateExtern)
      LV.mergeVisibility(HiddenVisibility, true);

    // Note that Sema::MergeCompatibleFunctionDecls already takes care of
    // merging storage classes and visibility attributes, so we don't have to
    // look at previous decls in here.

    // In C++, then if the type of the function uses a type with
    // unique-external linkage, it's not legally usable from outside
    // this translation unit.  However, we should use the C linkage
    // rules instead for extern "C" declarations.
    if (Context.getLangOpts().CPlusPlus &&
        !Function->isInExternCContext()) {
      // Only look at the type-as-written. If this function has an auto-deduced
      // return type, we can't compute the linkage of that type because it could
      // require looking at the linkage of this function, and we don't need this
      // for correctness because the type is not part of the function's
      // signature.
      // FIXME: This is a hack. We should be able to solve this circularity and 
      // the one in getLVForClassMember for Functions some other way.
      QualType TypeAsWritten = Function->getType();
      if (TypeSourceInfo *TSI = Function->getTypeSourceInfo())
        TypeAsWritten = TSI->getType();
      if (TypeAsWritten->getLinkage() == UniqueExternalLinkage)
        return LinkageInfo::uniqueExternal();
    }

    // Consider LV from the template and the template arguments.
    // We're at file scope, so we do not need to worry about nested
    // specializations.
    if (FunctionTemplateSpecializationInfo *specInfo
                               = Function->getTemplateSpecializationInfo()) {
      mergeTemplateLV(LV, Function, specInfo, computation);
    }

  //     - a named class (Clause 9), or an unnamed class defined in a
  //       typedef declaration in which the class has the typedef name
  //       for linkage purposes (7.1.3); or
  //     - a named enumeration (7.2), or an unnamed enumeration
  //       defined in a typedef declaration in which the enumeration
  //       has the typedef name for linkage purposes (7.1.3); or
  } else if (const TagDecl *Tag = dyn_cast<TagDecl>(D)) {
    // Unnamed tags have no linkage.
    if (!Tag->hasNameForLinkage())
      return LinkageInfo::none();

    // If this is a class template specialization, consider the
    // linkage of the template and template arguments.  We're at file
    // scope, so we do not need to worry about nested specializations.
    if (const ClassTemplateSpecializationDecl *spec
          = dyn_cast<ClassTemplateSpecializationDecl>(Tag)) {
      mergeTemplateLV(LV, spec, computation);
    }

  //     - an enumerator belonging to an enumeration with external linkage;
  } else if (isa<EnumConstantDecl>(D)) {
    LinkageInfo EnumLV = getLVForDecl(cast<NamedDecl>(D->getDeclContext()),
                                      computation);
    if (!isExternalFormalLinkage(EnumLV.getLinkage()))
      return LinkageInfo::none();
    LV.merge(EnumLV);

  //     - a template, unless it is a function template that has
  //       internal linkage (Clause 14);
  } else if (const TemplateDecl *temp = dyn_cast<TemplateDecl>(D)) {
    bool considerVisibility = !hasExplicitVisibilityAlready(computation);
    LinkageInfo tempLV =
      getLVForTemplateParameterList(temp->getTemplateParameters(), computation);
    LV.mergeMaybeWithVisibility(tempLV, considerVisibility);

  //     - a namespace (7.3), unless it is declared within an unnamed
  //       namespace.
  } else if (isa<NamespaceDecl>(D) && !D->isInAnonymousNamespace()) {
    return LV;

  // By extension, we assign external linkage to Objective-C
  // interfaces.
  } else if (isa<ObjCInterfaceDecl>(D)) {
    // fallout

  // Everything not covered here has no linkage.
  } else {
    // FIXME: A typedef declaration has linkage if it gives a type a name for
    // linkage purposes.
    return LinkageInfo::none();
  }

  // If we ended up with non-external linkage, visibility should
  // always be default.
  if (LV.getLinkage() != ExternalLinkage)
    return LinkageInfo(LV.getLinkage(), DefaultVisibility, false);

  return LV;
}

static LinkageInfo getLVForClassMember(const NamedDecl *D,
                                       LVComputationKind computation) {
  // Only certain class members have linkage.  Note that fields don't
  // really have linkage, but it's convenient to say they do for the
  // purposes of calculating linkage of pointer-to-data-member
  // template arguments.
  //
  // Templates also don't officially have linkage, but since we ignore
  // the C++ standard and look at template arguments when determining
  // linkage and visibility of a template specialization, we might hit
  // a template template argument that way. If we do, we need to
  // consider its linkage.
  if (!(isa<CXXMethodDecl>(D) ||
        isa<VarDecl>(D) ||
        isa<FieldDecl>(D) ||
        isa<IndirectFieldDecl>(D) ||
        isa<TagDecl>(D) ||
        isa<TemplateDecl>(D)))
    return LinkageInfo::none();

  LinkageInfo LV;

  // If we have an explicit visibility attribute, merge that in.
  if (!hasExplicitVisibilityAlready(computation)) {
    if (Optional<Visibility> Vis = getExplicitVisibility(D, computation))
      LV.mergeVisibility(*Vis, true);
    // If we're paying attention to global visibility, apply
    // -finline-visibility-hidden if this is an inline method.
    //
    // Note that we do this before merging information about
    // the class visibility.
    if (!LV.isVisibilityExplicit() && useInlineVisibilityHidden(D))
      LV.mergeVisibility(HiddenVisibility, true);
  }

  // If this class member has an explicit visibility attribute, the only
  // thing that can change its visibility is the template arguments, so
  // only look for them when processing the class.
  LVComputationKind classComputation = computation;
  if (LV.isVisibilityExplicit())
    classComputation = withExplicitVisibilityAlready(computation);

  LinkageInfo classLV =
    getLVForDecl(cast<RecordDecl>(D->getDeclContext()), classComputation);
  // If the class already has unique-external linkage, we can't improve.
  if (classLV.getLinkage() == UniqueExternalLinkage)
    return LinkageInfo::uniqueExternal();

  if (!isExternallyVisible(classLV.getLinkage()))
    return LinkageInfo::none();


  // Otherwise, don't merge in classLV yet, because in certain cases
  // we need to completely ignore the visibility from it.

  // Specifically, if this decl exists and has an explicit attribute.
  const NamedDecl *explicitSpecSuppressor = nullptr;

  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(D)) {
    // If the type of the function uses a type with unique-external
    // linkage, it's not legally usable from outside this translation unit.
    // But only look at the type-as-written. If this function has an
    // auto-deduced return type, we can't compute the linkage of that type
    // because it could require looking at the linkage of this function, and we
    // don't need this for correctness because the type is not part of the
    // function's signature.
    // FIXME: This is a hack. We should be able to solve this circularity and
    // the one in getLVForNamespaceScopeDecl for Functions some other way.
    {
      QualType TypeAsWritten = MD->getType();
      if (TypeSourceInfo *TSI = MD->getTypeSourceInfo())
        TypeAsWritten = TSI->getType();
      if (TypeAsWritten->getLinkage() == UniqueExternalLinkage)
        return LinkageInfo::uniqueExternal();
    }
    // If this is a method template specialization, use the linkage for
    // the template parameters and arguments.
    if (FunctionTemplateSpecializationInfo *spec
           = MD->getTemplateSpecializationInfo()) {
      mergeTemplateLV(LV, MD, spec, computation);
      if (spec->isExplicitSpecialization()) {
        explicitSpecSuppressor = MD;
      } else if (isExplicitMemberSpecialization(spec->getTemplate())) {
        explicitSpecSuppressor = spec->getTemplate()->getTemplatedDecl();
      }
    } else if (isExplicitMemberSpecialization(MD)) {
      explicitSpecSuppressor = MD;
    }

  } else if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(D)) {
    if (const ClassTemplateSpecializationDecl *spec
        = dyn_cast<ClassTemplateSpecializationDecl>(RD)) {
      mergeTemplateLV(LV, spec, computation);
      if (spec->isExplicitSpecialization()) {
        explicitSpecSuppressor = spec;
      } else {
        const ClassTemplateDecl *temp = spec->getSpecializedTemplate();
        if (isExplicitMemberSpecialization(temp)) {
          explicitSpecSuppressor = temp->getTemplatedDecl();
        }
      }
    } else if (isExplicitMemberSpecialization(RD)) {
      explicitSpecSuppressor = RD;
    }

  // Static data members.
  } else if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
    // HLSL Change: Make static data member internal linkage. This is to avoid confusion between global constant buffer
    if (VD->getStorageClass() == StorageClass::SC_Static)
      LV.setLinkage(Linkage::InternalLinkage);

    if (const VarTemplateSpecializationDecl *spec
      = dyn_cast<VarTemplateSpecializationDecl>(VD))
    mergeTemplateLV(LV, spec, computation);

    // Modify the variable's linkage by its type, but ignore the
    // type's visibility unless it's a definition.
    LinkageInfo typeLV = getLVForType(*VD->getType(), computation);
    if (!LV.isVisibilityExplicit() && !classLV.isVisibilityExplicit())
      LV.mergeVisibility(typeLV);
    LV.mergeExternalVisibility(typeLV);

    if (isExplicitMemberSpecialization(VD)) {
      explicitSpecSuppressor = VD;
    }

  // Template members.
  } else if (const TemplateDecl *temp = dyn_cast<TemplateDecl>(D)) {
    bool considerVisibility =
      (!LV.isVisibilityExplicit() &&
       !classLV.isVisibilityExplicit() &&
       !hasExplicitVisibilityAlready(computation));
    LinkageInfo tempLV =
      getLVForTemplateParameterList(temp->getTemplateParameters(), computation);
    LV.mergeMaybeWithVisibility(tempLV, considerVisibility);

    if (const RedeclarableTemplateDecl *redeclTemp =
          dyn_cast<RedeclarableTemplateDecl>(temp)) {
      if (isExplicitMemberSpecialization(redeclTemp)) {
        explicitSpecSuppressor = temp->getTemplatedDecl();
      }
    }
  }

  // We should never be looking for an attribute directly on a template.
  assert(!explicitSpecSuppressor || !isa<TemplateDecl>(explicitSpecSuppressor));

  // If this member is an explicit member specialization, and it has
  // an explicit attribute, ignore visibility from the parent.
  bool considerClassVisibility = true;
  if (explicitSpecSuppressor &&
      // optimization: hasDVA() is true only with explicit visibility.
      LV.isVisibilityExplicit() &&
      classLV.getVisibility() != DefaultVisibility &&
      hasDirectVisibilityAttribute(explicitSpecSuppressor, computation)) {
    considerClassVisibility = false;
  }

  // Finally, merge in information from the class.
  LV.mergeMaybeWithVisibility(classLV, considerClassVisibility);
  return LV;
}

void NamedDecl::anchor() { }

static LinkageInfo computeLVForDecl(const NamedDecl *D,
                                    LVComputationKind computation);

bool NamedDecl::isLinkageValid() const {
  if (!hasCachedLinkage())
    return true;

  return computeLVForDecl(this, LVForLinkageOnly).getLinkage() ==
         getCachedLinkage();
}

ObjCStringFormatFamily NamedDecl::getObjCFStringFormattingFamily() const {
  StringRef name = getName();
  if (name.empty()) return SFF_None;
  
  if (name.front() == 'C')
    if (name == "CFStringCreateWithFormat" ||
        name == "CFStringCreateWithFormatAndArguments" ||
        name == "CFStringAppendFormat" ||
        name == "CFStringAppendFormatAndArguments")
      return SFF_CFString;
  return SFF_None;
}

Linkage NamedDecl::getLinkageInternal() const {
  // We don't care about visibility here, so ask for the cheapest
  // possible visibility analysis.
  return getLVForDecl(this, LVForLinkageOnly).getLinkage();
}

LinkageInfo NamedDecl::getLinkageAndVisibility() const {
  LVComputationKind computation =
    (usesTypeVisibility(this) ? LVForType : LVForValue);
  return getLVForDecl(this, computation);
}

static Optional<Visibility>
getExplicitVisibilityAux(const NamedDecl *ND,
                         NamedDecl::ExplicitVisibilityKind kind,
                         bool IsMostRecent) {
  assert(!IsMostRecent || ND == ND->getMostRecentDecl());

  // Check the declaration itself first.
  if (Optional<Visibility> V = getVisibilityOf(ND, kind))
    return V;

  // If this is a member class of a specialization of a class template
  // and the corresponding decl has explicit visibility, use that.
  if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(ND)) {
    CXXRecordDecl *InstantiatedFrom = RD->getInstantiatedFromMemberClass();
    if (InstantiatedFrom)
      return getVisibilityOf(InstantiatedFrom, kind);
  }

  // If there wasn't explicit visibility there, and this is a
  // specialization of a class template, check for visibility
  // on the pattern.
  if (const ClassTemplateSpecializationDecl *spec
        = dyn_cast<ClassTemplateSpecializationDecl>(ND))
    return getVisibilityOf(spec->getSpecializedTemplate()->getTemplatedDecl(),
                           kind);

  // Use the most recent declaration.
  if (!IsMostRecent && !isa<NamespaceDecl>(ND)) {
    const NamedDecl *MostRecent = ND->getMostRecentDecl();
    if (MostRecent != ND)
      return getExplicitVisibilityAux(MostRecent, kind, true);
  }

  if (const VarDecl *Var = dyn_cast<VarDecl>(ND)) {
    if (Var->isStaticDataMember()) {
      VarDecl *InstantiatedFrom = Var->getInstantiatedFromStaticDataMember();
      if (InstantiatedFrom)
        return getVisibilityOf(InstantiatedFrom, kind);
    }

    if (const auto *VTSD = dyn_cast<VarTemplateSpecializationDecl>(Var))
      return getVisibilityOf(VTSD->getSpecializedTemplate()->getTemplatedDecl(),
                             kind);

    return None;
  }
  // Also handle function template specializations.
  if (const FunctionDecl *fn = dyn_cast<FunctionDecl>(ND)) {
    // If the function is a specialization of a template with an
    // explicit visibility attribute, use that.
    if (FunctionTemplateSpecializationInfo *templateInfo
          = fn->getTemplateSpecializationInfo())
      return getVisibilityOf(templateInfo->getTemplate()->getTemplatedDecl(),
                             kind);

    // If the function is a member of a specialization of a class template
    // and the corresponding decl has explicit visibility, use that.
    FunctionDecl *InstantiatedFrom = fn->getInstantiatedFromMemberFunction();
    if (InstantiatedFrom)
      return getVisibilityOf(InstantiatedFrom, kind);

    return None;
  }

  // The visibility of a template is stored in the templated decl.
  if (const TemplateDecl *TD = dyn_cast<TemplateDecl>(ND))
    return getVisibilityOf(TD->getTemplatedDecl(), kind);

  return None;
}

Optional<Visibility>
NamedDecl::getExplicitVisibility(ExplicitVisibilityKind kind) const {
  return getExplicitVisibilityAux(this, kind, false);
}

static LinkageInfo getLVForClosure(const DeclContext *DC, Decl *ContextDecl,
                                   LVComputationKind computation) {
  // This lambda has its linkage/visibility determined by its owner.
  if (ContextDecl) {
    if (isa<ParmVarDecl>(ContextDecl))
      DC = ContextDecl->getDeclContext()->getRedeclContext();
    else
      return getLVForDecl(cast<NamedDecl>(ContextDecl), computation);
  }

  if (const NamedDecl *ND = dyn_cast<NamedDecl>(DC))
    return getLVForDecl(ND, computation);

  return LinkageInfo::external();
}

static LinkageInfo getLVForLocalDecl(const NamedDecl *D,
                                     LVComputationKind computation) {
  if (const FunctionDecl *Function = dyn_cast<FunctionDecl>(D)) {
    if (Function->isInAnonymousNamespace() &&
        !Function->isInExternCContext())
      return LinkageInfo::uniqueExternal();

    // This is a "void f();" which got merged with a file static.
    if (Function->getCanonicalDecl()->getStorageClass() == SC_Static)
      return LinkageInfo::internal();

    LinkageInfo LV;
    if (!hasExplicitVisibilityAlready(computation)) {
      if (Optional<Visibility> Vis =
              getExplicitVisibility(Function, computation))
        LV.mergeVisibility(*Vis, true);
    }

    // Note that Sema::MergeCompatibleFunctionDecls already takes care of
    // merging storage classes and visibility attributes, so we don't have to
    // look at previous decls in here.

    return LV;
  }

  if (const VarDecl *Var = dyn_cast<VarDecl>(D)) {
    if (Var->hasExternalStorage()) {
      if (Var->isInAnonymousNamespace() && !Var->isInExternCContext())
        return LinkageInfo::uniqueExternal();

      LinkageInfo LV;
      if (Var->getStorageClass() == SC_PrivateExtern)
        LV.mergeVisibility(HiddenVisibility, true);
      else if (!hasExplicitVisibilityAlready(computation)) {
        if (Optional<Visibility> Vis = getExplicitVisibility(Var, computation))
          LV.mergeVisibility(*Vis, true);
      }

      if (const VarDecl *Prev = Var->getPreviousDecl()) {
        LinkageInfo PrevLV = getLVForDecl(Prev, computation);
        if (PrevLV.getLinkage())
          LV.setLinkage(PrevLV.getLinkage());
        LV.mergeVisibility(PrevLV);
      }

      return LV;
    }

    if (!Var->isStaticLocal())
      return LinkageInfo::none();
  }

  ASTContext &Context = D->getASTContext();
  if (!Context.getLangOpts().CPlusPlus)
    return LinkageInfo::none();

  const Decl *OuterD = getOutermostFuncOrBlockContext(D);
  if (!OuterD)
    return LinkageInfo::none();

  LinkageInfo LV;
  if (const BlockDecl *BD = dyn_cast<BlockDecl>(OuterD)) {
    if (!BD->getBlockManglingNumber())
      return LinkageInfo::none();

    LV = getLVForClosure(BD->getDeclContext()->getRedeclContext(),
                         BD->getBlockManglingContextDecl(), computation);
  } else {
    const FunctionDecl *FD = cast<FunctionDecl>(OuterD);
    if (!FD->isInlined() &&
        !isTemplateInstantiation(FD->getTemplateSpecializationKind()))
      return LinkageInfo::none();

    LV = getLVForDecl(FD, computation);
  }
  if (!isExternallyVisible(LV.getLinkage()))
    return LinkageInfo::none();
  return LinkageInfo(VisibleNoLinkage, LV.getVisibility(),
                     LV.isVisibilityExplicit());
}

static inline const CXXRecordDecl*
getOutermostEnclosingLambda(const CXXRecordDecl *Record) {
  const CXXRecordDecl *Ret = Record;
  while (Record && Record->isLambda()) {
    Ret = Record;
    if (!Record->getParent()) break;
    // Get the Containing Class of this Lambda Class
    Record = dyn_cast_or_null<CXXRecordDecl>(
      Record->getParent()->getParent());
  }
  return Ret;
}

static LinkageInfo computeLVForDecl(const NamedDecl *D,
                                    LVComputationKind computation) {
  // Objective-C: treat all Objective-C declarations as having external
  // linkage.
  switch (D->getKind()) {
    default:
      break;
    case Decl::ParmVar:
      return LinkageInfo::none();
    case Decl::TemplateTemplateParm: // count these as external
    case Decl::NonTypeTemplateParm:
    case Decl::ObjCAtDefsField:
    case Decl::ObjCCategory:
    case Decl::ObjCCategoryImpl:
    case Decl::ObjCCompatibleAlias:
    case Decl::ObjCImplementation:
    case Decl::ObjCMethod:
    case Decl::ObjCProperty:
    case Decl::ObjCPropertyImpl:
    case Decl::ObjCProtocol:
      return LinkageInfo::external();
      
    case Decl::CXXRecord: {
      const CXXRecordDecl *Record = cast<CXXRecordDecl>(D);
      if (Record->isLambda()) {
        if (!Record->getLambdaManglingNumber()) {
          // This lambda has no mangling number, so it's internal.
          return LinkageInfo::internal();
        }

        // This lambda has its linkage/visibility determined:
        //  - either by the outermost lambda if that lambda has no mangling 
        //    number. 
        //  - or by the parent of the outer most lambda
        // This prevents infinite recursion in settings such as nested lambdas 
        // used in NSDMI's, for e.g. 
        //  struct L {
        //    int t{};
        //    int t2 = ([](int a) { return [](int b) { return b; };})(t)(t);    
        //  };
        const CXXRecordDecl *OuterMostLambda = 
            getOutermostEnclosingLambda(Record);
        if (!OuterMostLambda->getLambdaManglingNumber())
          return LinkageInfo::internal();
        
        return getLVForClosure(
                  OuterMostLambda->getDeclContext()->getRedeclContext(),
                  OuterMostLambda->getLambdaContextDecl(), computation);
      }
      
      break;
    }
  }

  // Handle linkage for namespace-scope names.
  if (D->getDeclContext()->getRedeclContext()->isFileContext())
    return getLVForNamespaceScopeDecl(D, computation);
  
  // C++ [basic.link]p5:
  //   In addition, a member function, static data member, a named
  //   class or enumeration of class scope, or an unnamed class or
  //   enumeration defined in a class-scope typedef declaration such
  //   that the class or enumeration has the typedef name for linkage
  //   purposes (7.1.3), has external linkage if the name of the class
  //   has external linkage.
  if (D->getDeclContext()->isRecord())
    return getLVForClassMember(D, computation);

  // C++ [basic.link]p6:
  //   The name of a function declared in block scope and the name of
  //   an object declared by a block scope extern declaration have
  //   linkage. If there is a visible declaration of an entity with
  //   linkage having the same name and type, ignoring entities
  //   declared outside the innermost enclosing namespace scope, the
  //   block scope declaration declares that same entity and receives
  //   the linkage of the previous declaration. If there is more than
  //   one such matching entity, the program is ill-formed. Otherwise,
  //   if no matching entity is found, the block scope entity receives
  //   external linkage.
  if (D->getDeclContext()->isFunctionOrMethod())
    return getLVForLocalDecl(D, computation);

  // C++ [basic.link]p6:
  //   Names not covered by these rules have no linkage.
  return LinkageInfo::none();
}

namespace clang {
class LinkageComputer {
public:
  static LinkageInfo getLVForDecl(const NamedDecl *D,
                                  LVComputationKind computation) {
    if (computation == LVForLinkageOnly && D->hasCachedLinkage())
      return LinkageInfo(D->getCachedLinkage(), DefaultVisibility, false);

    LinkageInfo LV = computeLVForDecl(D, computation);
    if (D->hasCachedLinkage())
      assert(D->getCachedLinkage() == LV.getLinkage());

    D->setCachedLinkage(LV.getLinkage());

#ifndef NDEBUG
    // In C (because of gnu inline) and in c++ with microsoft extensions an
    // static can follow an extern, so we can have two decls with different
    // linkages.
    const LangOptions &Opts = D->getASTContext().getLangOpts();
    (void)(Opts); // HLSL Change - this only has static consts referenced
    if (!Opts.CPlusPlus || Opts.MicrosoftExt)
      return LV;

    // We have just computed the linkage for this decl. By induction we know
    // that all other computed linkages match, check that the one we just
    // computed also does.
    NamedDecl *Old = nullptr;
    for (auto I : D->redecls()) {
      NamedDecl *T = cast<NamedDecl>(I);
      if (T == D)
        continue;
      if (!T->isInvalidDecl() && T->hasCachedLinkage()) {
        Old = T;
        break;
      }
    }
    assert(!Old || Old->getCachedLinkage() == D->getCachedLinkage());
#endif

    return LV;
  }
};
}

static LinkageInfo getLVForDecl(const NamedDecl *D,
                                LVComputationKind computation) {
  return clang::LinkageComputer::getLVForDecl(D, computation);
}

std::string NamedDecl::getQualifiedNameAsString() const {
  std::string QualName;
  llvm::raw_string_ostream OS(QualName);
  printQualifiedName(OS, getASTContext().getPrintingPolicy());
  return OS.str();
}

void NamedDecl::printQualifiedName(raw_ostream &OS) const {
  printQualifiedName(OS, getASTContext().getPrintingPolicy());
}

void NamedDecl::printQualifiedName(raw_ostream &OS,
                                   const PrintingPolicy &P) const {
  const DeclContext *Ctx = getDeclContext();

  if (Ctx->isFunctionOrMethod()) {
    printName(OS);
    return;
  }

  typedef SmallVector<const DeclContext *, 8> ContextsTy;
  ContextsTy Contexts;

  // Collect contexts.
  while (Ctx && isa<NamedDecl>(Ctx)) {
    Contexts.push_back(Ctx);
    Ctx = Ctx->getParent();
  }

  for (ContextsTy::reverse_iterator I = Contexts.rbegin(), E = Contexts.rend();
       I != E; ++I) {
    if (const ClassTemplateSpecializationDecl *Spec
          = dyn_cast<ClassTemplateSpecializationDecl>(*I)) {
      OS << Spec->getName();
      const TemplateArgumentList &TemplateArgs = Spec->getTemplateArgs();
      TemplateSpecializationType::PrintTemplateArgumentList(OS,
                                                            TemplateArgs.data(),
                                                            TemplateArgs.size(),
                                                            P);
    } else if (const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(*I)) {
      if (P.SuppressUnwrittenScope &&
          (ND->isAnonymousNamespace() || ND->isInline()))
        continue;
      if (ND->isAnonymousNamespace())
        OS << "(anonymous namespace)";
      else
        OS << *ND;
    // HLSL Change Begin - not add cbuffer name to qualified name.
    } else if (isa<HLSLBufferDecl>(*I)) {
      continue;
    // HLSL Change End.
    } else if (const RecordDecl *RD = dyn_cast<RecordDecl>(*I)) {
      if (!RD->getIdentifier())
        OS << "(anonymous " << RD->getKindName() << ')';
      else
        OS << *RD;
    } else if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(*I)) {
      const FunctionProtoType *FT = nullptr;
      if (FD->hasWrittenPrototype())
        FT = dyn_cast<FunctionProtoType>(FD->getType()->castAs<FunctionType>());

      OS << *FD << '(';
      if (FT) {
        unsigned NumParams = FD->getNumParams();
        for (unsigned i = 0; i < NumParams; ++i) {
          if (i)
            OS << ", ";
          OS << FD->getParamDecl(i)->getType().stream(P);
        }

        if (FT->isVariadic()) {
          if (NumParams > 0)
            OS << ", ";
          OS << "...";
        }
      }
      OS << ')';
    } else {
      OS << *cast<NamedDecl>(*I);
    }
    OS << "::";
  }

  if (getDeclName())
    OS << *this;
  else
    OS << "(anonymous)";
}

void NamedDecl::getNameForDiagnostic(raw_ostream &OS,
                                     const PrintingPolicy &Policy,
                                     bool Qualified) const {
  if (Qualified)
    printQualifiedName(OS, Policy);
  else
    printName(OS);
}

static bool isKindReplaceableBy(Decl::Kind OldK, Decl::Kind NewK) {
  // For method declarations, we never replace.
  if (ObjCMethodDecl::classofKind(NewK))
    return false;

  if (OldK == NewK)
    return true;

  // A compatibility alias for a class can be replaced by an interface.
  if (ObjCCompatibleAliasDecl::classofKind(OldK) &&
      ObjCInterfaceDecl::classofKind(NewK))
    return true;

  // A typedef-declaration, alias-declaration, or Objective-C class declaration
  // can replace another declaration of the same type. Semantic analysis checks
  // that we have matching types.
  if ((TypedefNameDecl::classofKind(OldK) ||
       ObjCInterfaceDecl::classofKind(OldK)) &&
      (TypedefNameDecl::classofKind(NewK) ||
       ObjCInterfaceDecl::classofKind(NewK)))
    return true;

  // Otherwise, a kind mismatch implies that the declaration is not replaced.
  return false;
}

template<typename T> static bool isRedeclarableImpl(Redeclarable<T> *) {
  return true;
}
static bool isRedeclarableImpl(...) { return false; }
static bool isRedeclarable(Decl::Kind K) {
  switch (K) {
#define DECL(Type, Base) \
  case Decl::Type: \
    return isRedeclarableImpl((Type##Decl *)nullptr);
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"
  }
  llvm_unreachable("unknown decl kind");
}

bool NamedDecl::declarationReplaces(NamedDecl *OldD, bool IsKnownNewer) const {
  assert(getDeclName() == OldD->getDeclName() && "Declaration name mismatch");

  // Never replace one imported declaration with another; we need both results
  // when re-exporting.
  if (OldD->isFromASTFile() && isFromASTFile())
    return false;

  if (!isKindReplaceableBy(OldD->getKind(), getKind()))
    return false;

  // Inline namespaces can give us two declarations with the same
  // name and kind in the same scope but different contexts; we should
  // keep both declarations in this case.
  if (!this->getDeclContext()->getRedeclContext()->Equals(
          OldD->getDeclContext()->getRedeclContext()))
    return false;

  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(this))
    // For function declarations, we keep track of redeclarations.
    // FIXME: This returns false for functions that should in fact be replaced.
    // Instead, perform some kind of type check?
    if (FD->getPreviousDecl() != OldD)
      return false;

  // For function templates, the underlying function declarations are linked.
  if (const FunctionTemplateDecl *FunctionTemplate =
          dyn_cast<FunctionTemplateDecl>(this))
    return FunctionTemplate->getTemplatedDecl()->declarationReplaces(
        cast<FunctionTemplateDecl>(OldD)->getTemplatedDecl());

  // Using shadow declarations can be overloaded on their target declarations
  // if they introduce functions.
  // FIXME: If our target replaces the old target, can we replace the old
  //        shadow declaration?
  if (auto *USD = dyn_cast<UsingShadowDecl>(this))
    if (USD->getTargetDecl() != cast<UsingShadowDecl>(OldD)->getTargetDecl())
      return false;

  // Using declarations can be overloaded if they introduce functions.
  if (auto *UD = dyn_cast<UsingDecl>(this)) {
    ASTContext &Context = getASTContext();
    return Context.getCanonicalNestedNameSpecifier(UD->getQualifier()) ==
           Context.getCanonicalNestedNameSpecifier(
               cast<UsingDecl>(OldD)->getQualifier());
  }
  if (auto *UUVD = dyn_cast<UnresolvedUsingValueDecl>(this)) {
    ASTContext &Context = getASTContext();
    return Context.getCanonicalNestedNameSpecifier(UUVD->getQualifier()) ==
           Context.getCanonicalNestedNameSpecifier(
                        cast<UnresolvedUsingValueDecl>(OldD)->getQualifier());
  }

  // UsingDirectiveDecl's are not really NamedDecl's, and all have same name.
  // We want to keep it, unless it nominates same namespace.
  if (auto *UD = dyn_cast<UsingDirectiveDecl>(this))
    return UD->getNominatedNamespace()->getOriginalNamespace() ==
           cast<UsingDirectiveDecl>(OldD)->getNominatedNamespace()
               ->getOriginalNamespace();

  if (!IsKnownNewer && isRedeclarable(getKind())) {
    // Check whether this is actually newer than OldD. We want to keep the
    // newer declaration. This loop will usually only iterate once, because
    // OldD is usually the previous declaration.
    for (auto D : redecls()) {
      if (D == OldD)
        break;

      // If we reach the canonical declaration, then OldD is not actually older
      // than this one.
      //
      // FIXME: In this case, we should not add this decl to the lookup table.
      if (D->isCanonicalDecl())
        return false;
    }
  }

  // It's a newer declaration of the same kind of declaration in the same scope,
  // and not an overload: we want this decl instead of the existing one.
  return true;
}

bool NamedDecl::hasLinkage() const {
  return getFormalLinkage() != NoLinkage;
}

NamedDecl *NamedDecl::getUnderlyingDeclImpl() {
  NamedDecl *ND = this;
  while (UsingShadowDecl *UD = dyn_cast<UsingShadowDecl>(ND))
    ND = UD->getTargetDecl();

  if (ObjCCompatibleAliasDecl *AD = dyn_cast<ObjCCompatibleAliasDecl>(ND))
    return AD->getClassInterface();

  return ND;
}

bool NamedDecl::isCXXInstanceMember() const {
  if (!isCXXClassMember())
    return false;
  
  const NamedDecl *D = this;
  if (isa<UsingShadowDecl>(D))
    D = cast<UsingShadowDecl>(D)->getTargetDecl();

  if (isa<FieldDecl>(D) || isa<IndirectFieldDecl>(D) || isa<MSPropertyDecl>(D))
    return true;
  if (const CXXMethodDecl *MD =
          dyn_cast_or_null<CXXMethodDecl>(D->getAsFunction()))
    return MD->isInstance();
  return false;
}

//===----------------------------------------------------------------------===//
// DeclaratorDecl Implementation
//===----------------------------------------------------------------------===//

template <typename DeclT>
static SourceLocation getTemplateOrInnerLocStart(const DeclT *decl) {
  if (decl->getNumTemplateParameterLists() > 0)
    return decl->getTemplateParameterList(0)->getTemplateLoc();
  else
    return decl->getInnerLocStart();
}

SourceLocation DeclaratorDecl::getTypeSpecStartLoc() const {
  TypeSourceInfo *TSI = getTypeSourceInfo();
  if (TSI) return TSI->getTypeLoc().getBeginLoc();
  return SourceLocation();
}

void DeclaratorDecl::setQualifierInfo(NestedNameSpecifierLoc QualifierLoc) {
  if (QualifierLoc) {
    // Make sure the extended decl info is allocated.
    if (!hasExtInfo()) {
      // Save (non-extended) type source info pointer.
      TypeSourceInfo *savedTInfo = DeclInfo.get<TypeSourceInfo*>();
      // Allocate external info struct.
      DeclInfo = new (getASTContext()) ExtInfo;
      // Restore savedTInfo into (extended) decl info.
      getExtInfo()->TInfo = savedTInfo;
    }
    // Set qualifier info.
    getExtInfo()->QualifierLoc = QualifierLoc;
  } else {
    // Here Qualifier == 0, i.e., we are removing the qualifier (if any).
    if (hasExtInfo()) {
      if (getExtInfo()->NumTemplParamLists == 0) {
        // Save type source info pointer.
        TypeSourceInfo *savedTInfo = getExtInfo()->TInfo;
        // Deallocate the extended decl info.
        getASTContext().Deallocate(getExtInfo());
        // Restore savedTInfo into (non-extended) decl info.
        DeclInfo = savedTInfo;
      }
      else
        getExtInfo()->QualifierLoc = QualifierLoc;
    }
  }
}

void
DeclaratorDecl::setTemplateParameterListsInfo(ASTContext &Context,
                                              unsigned NumTPLists,
                                              TemplateParameterList **TPLists) {
  assert(NumTPLists > 0);
  // Make sure the extended decl info is allocated.
  if (!hasExtInfo()) {
    // Save (non-extended) type source info pointer.
    TypeSourceInfo *savedTInfo = DeclInfo.get<TypeSourceInfo*>();
    // Allocate external info struct.
    DeclInfo = new (getASTContext()) ExtInfo;
    // Restore savedTInfo into (extended) decl info.
    getExtInfo()->TInfo = savedTInfo;
  }
  // Set the template parameter lists info.
  getExtInfo()->setTemplateParameterListsInfo(Context, NumTPLists, TPLists);
}

SourceLocation DeclaratorDecl::getOuterLocStart() const {
  return getTemplateOrInnerLocStart(this);
}

namespace {

// Helper function: returns true if QT is or contains a type
// having a postfix component.
bool typeIsPostfix(clang::QualType QT) {
  while (true) {
    const Type* T = QT.getTypePtr();
    switch (T->getTypeClass()) {
    default:
      return false;
    case Type::Pointer:
      QT = cast<PointerType>(T)->getPointeeType();
      break;
    case Type::BlockPointer:
      QT = cast<BlockPointerType>(T)->getPointeeType();
      break;
    case Type::MemberPointer:
      QT = cast<MemberPointerType>(T)->getPointeeType();
      break;
    case Type::LValueReference:
    case Type::RValueReference:
      QT = cast<ReferenceType>(T)->getPointeeType();
      break;
    case Type::PackExpansion:
      QT = cast<PackExpansionType>(T)->getPattern();
      break;
    case Type::Paren:
    case Type::ConstantArray:
    case Type::DependentSizedArray:
    case Type::IncompleteArray:
    case Type::VariableArray:
    case Type::FunctionProto:
    case Type::FunctionNoProto:
      return true;
    }
  }
}

} // namespace

SourceRange DeclaratorDecl::getSourceRange() const {
  SourceLocation RangeEnd = getLocation();
  if (TypeSourceInfo *TInfo = getTypeSourceInfo()) {
    // If the declaration has no name or the type extends past the name take the
    // end location of the type.
    if (!getDeclName() || typeIsPostfix(TInfo->getType()))
      RangeEnd = TInfo->getTypeLoc().getSourceRange().getEnd();
  }
  return SourceRange(getOuterLocStart(), RangeEnd);
}

void
QualifierInfo::setTemplateParameterListsInfo(ASTContext &Context,
                                             unsigned NumTPLists,
                                             TemplateParameterList **TPLists) {
  assert((NumTPLists == 0 || TPLists != nullptr) &&
         "Empty array of template parameters with positive size!");

  // Free previous template parameters (if any).
  if (NumTemplParamLists > 0) {
    Context.Deallocate(TemplParamLists);
    TemplParamLists = nullptr;
    NumTemplParamLists = 0;
  }
  // Set info on matched template parameter lists (if any).
  if (NumTPLists > 0) {
    TemplParamLists = new (Context) TemplateParameterList*[NumTPLists];
    NumTemplParamLists = NumTPLists;
    std::copy(TPLists, TPLists + NumTPLists, TemplParamLists);
  }
}

//===----------------------------------------------------------------------===//
// VarDecl Implementation
//===----------------------------------------------------------------------===//

const char *VarDecl::getStorageClassSpecifierString(StorageClass SC) {
  switch (SC) {
  case SC_None:                 break;
  case SC_Auto:                 return "auto";
  case SC_Extern:               return "extern";
  case SC_OpenCLWorkGroupLocal: return "<<work-group-local>>";
  case SC_PrivateExtern:        return "__private_extern__";
  case SC_Register:             return "register";
  case SC_Static:               return "static";
  }

  llvm_unreachable("Invalid storage class");
}

VarDecl::VarDecl(Kind DK, ASTContext &C, DeclContext *DC,
                 SourceLocation StartLoc, SourceLocation IdLoc,
                 IdentifierInfo *Id, QualType T, TypeSourceInfo *TInfo,
                 StorageClass SC)
    : DeclaratorDecl(DK, DC, IdLoc, Id, T, TInfo, StartLoc),
      redeclarable_base(C), Init() {
  static_assert(sizeof(VarDeclBitfields) <= sizeof(unsigned),
                "VarDeclBitfields too large!");
  static_assert(sizeof(ParmVarDeclBitfields) <= sizeof(unsigned),
                "ParmVarDeclBitfields too large!");
  static_assert(sizeof(NonParmVarDeclBitfields) <= sizeof(unsigned),
                "NonParmVarDeclBitfields too large!");
  AllBits = 0;
  VarDeclBits.SClass = SC;
  // Everything else is implicitly initialized to false.
}

VarDecl *VarDecl::Create(ASTContext &C, DeclContext *DC,
                         SourceLocation StartL, SourceLocation IdL,
                         IdentifierInfo *Id, QualType T, TypeSourceInfo *TInfo,
                         StorageClass S) {
  return new (C, DC) VarDecl(Var, C, DC, StartL, IdL, Id, T, TInfo, S);
}

VarDecl *VarDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID)
      VarDecl(Var, C, nullptr, SourceLocation(), SourceLocation(), nullptr,
              QualType(), nullptr, SC_None);
}

void VarDecl::setStorageClass(StorageClass SC) {
  assert(isLegalForVariable(SC));
  VarDeclBits.SClass = SC;
}

VarDecl::TLSKind VarDecl::getTLSKind() const {
  switch (VarDeclBits.TSCSpec) {
  case TSCS_unspecified:
    if (!hasAttr<ThreadAttr>() &&
        !(getASTContext().getLangOpts().OpenMPUseTLS &&
          getASTContext().getTargetInfo().isTLSSupported() &&
          hasAttr<OMPThreadPrivateDeclAttr>()))
      return TLS_None;
    return ((getASTContext().getLangOpts().isCompatibleWithMSVC(
                LangOptions::MSVC2015)) ||
            hasAttr<OMPThreadPrivateDeclAttr>())
               ? TLS_Dynamic
               : TLS_Static;
  case TSCS___thread: // Fall through.
  case TSCS__Thread_local:
    return TLS_Static;
  case TSCS_thread_local:
    return TLS_Dynamic;
  }
  llvm_unreachable("Unknown thread storage class specifier!");
}

SourceRange VarDecl::getSourceRange() const {
  if (const Expr *Init = getInit()) {
    SourceLocation InitEnd = Init->getLocEnd();
    // If Init is implicit, ignore its source range and fallback on 
    // DeclaratorDecl::getSourceRange() to handle postfix elements.
    if (InitEnd.isValid() && InitEnd != getLocation())
      return SourceRange(getOuterLocStart(), InitEnd);
  }
  return DeclaratorDecl::getSourceRange();
}

template<typename T>
static LanguageLinkage getDeclLanguageLinkage(const T &D) {
  // C++ [dcl.link]p1: All function types, function names with external linkage,
  // and variable names with external linkage have a language linkage.
  if (!D.hasExternalFormalLinkage())
    return NoLanguageLinkage;

  // Language linkage is a C++ concept, but saying that everything else in C has
  // C language linkage fits the implementation nicely.
  ASTContext &Context = D.getASTContext();
  if (!Context.getLangOpts().CPlusPlus)
    return CLanguageLinkage;

  // C++ [dcl.link]p4: A C language linkage is ignored in determining the
  // language linkage of the names of class members and the function type of
  // class member functions.
  const DeclContext *DC = D.getDeclContext();
  if (DC->isRecord())
    return CXXLanguageLinkage;

  // If the first decl is in an extern "C" context, any other redeclaration
  // will have C language linkage. If the first one is not in an extern "C"
  // context, we would have reported an error for any other decl being in one.
  if (isFirstInExternCContext(&D))
    return CLanguageLinkage;
  return CXXLanguageLinkage;
}

template<typename T>
static bool isDeclExternC(const T &D) {
  // Since the context is ignored for class members, they can only have C++
  // language linkage or no language linkage.
  const DeclContext *DC = D.getDeclContext();
  if (DC->isRecord()) {
    assert(D.getASTContext().getLangOpts().CPlusPlus);
    return false;
  }

  return D.getLanguageLinkage() == CLanguageLinkage;
}

LanguageLinkage VarDecl::getLanguageLinkage() const {
  return getDeclLanguageLinkage(*this);
}

bool VarDecl::isExternC() const {
  return isDeclExternC(*this);
}

bool VarDecl::isInExternCContext() const {
  return getLexicalDeclContext()->isExternCContext();
}

bool VarDecl::isInExternCXXContext() const {
  return getLexicalDeclContext()->isExternCXXContext();
}

VarDecl *VarDecl::getCanonicalDecl() { return getFirstDecl(); }

VarDecl::DefinitionKind
VarDecl::isThisDeclarationADefinition(ASTContext &C) const {
  // C++ [basic.def]p2:
  //   A declaration is a definition unless [...] it contains the 'extern'
  //   specifier or a linkage-specification and neither an initializer [...],
  //   it declares a static data member in a class declaration [...].
  // C++1y [temp.expl.spec]p15:
  //   An explicit specialization of a static data member or an explicit
  //   specialization of a static data member template is a definition if the
  //   declaration includes an initializer; otherwise, it is a declaration.
  //
  // FIXME: How do you declare (but not define) a partial specialization of
  // a static data member template outside the containing class?
  if (isStaticDataMember()) {
    if (isOutOfLine() &&
        (hasInit() ||
         // If the first declaration is out-of-line, this may be an
         // instantiation of an out-of-line partial specialization of a variable
         // template for which we have not yet instantiated the initializer.
         (getFirstDecl()->isOutOfLine()
              ? getTemplateSpecializationKind() == TSK_Undeclared
              : getTemplateSpecializationKind() !=
                    TSK_ExplicitSpecialization) ||
         isa<VarTemplatePartialSpecializationDecl>(this)))
      return Definition;
    else
      return DeclarationOnly;
  }
  // C99 6.7p5:
  //   A definition of an identifier is a declaration for that identifier that
  //   [...] causes storage to be reserved for that object.
  // Note: that applies for all non-file-scope objects.
  // C99 6.9.2p1:
  //   If the declaration of an identifier for an object has file scope and an
  //   initializer, the declaration is an external definition for the identifier
  if (hasInit())
    return Definition;

  if (hasAttr<AliasAttr>())
    return Definition;

  if (const auto *SAA = getAttr<SelectAnyAttr>())
    if (!SAA->isInherited())
      return Definition;

  // A variable template specialization (other than a static data member
  // template or an explicit specialization) is a declaration until we
  // instantiate its initializer.
  if (isa<VarTemplateSpecializationDecl>(this) &&
      getTemplateSpecializationKind() != TSK_ExplicitSpecialization)
    return DeclarationOnly;

  if (!getASTContext().getLangOpts().HLSL) // HLSL Change - take extern as define to match fxc.
  if (hasExternalStorage())
    return DeclarationOnly;

  // [dcl.link] p7:
  //   A declaration directly contained in a linkage-specification is treated
  //   as if it contains the extern specifier for the purpose of determining
  //   the linkage of the declared name and whether it is a definition.
  if (isSingleLineLanguageLinkage(*this))
    return DeclarationOnly;

  // C99 6.9.2p2:
  //   A declaration of an object that has file scope without an initializer,
  //   and without a storage class specifier or the scs 'static', constitutes
  //   a tentative definition.
  // No such thing in C++.
  if (!C.getLangOpts().CPlusPlus && isFileVarDecl())
    return TentativeDefinition;

  // What's left is (in C, block-scope) declarations without initializers or
  // external storage. These are definitions.
  return Definition;
}

VarDecl *VarDecl::getActingDefinition() {
  DefinitionKind Kind = isThisDeclarationADefinition();
  if (Kind != TentativeDefinition)
    return nullptr;

  VarDecl *LastTentative = nullptr;
  VarDecl *First = getFirstDecl();
  for (auto I : First->redecls()) {
    Kind = I->isThisDeclarationADefinition();
    if (Kind == Definition)
      return nullptr;
    else if (Kind == TentativeDefinition)
      LastTentative = I;
  }
  return LastTentative;
}

VarDecl *VarDecl::getDefinition(ASTContext &C) {
  VarDecl *First = getFirstDecl();
  for (auto I : First->redecls()) {
    if (I->isThisDeclarationADefinition(C) == Definition)
      return I;
  }
  return nullptr;
}

VarDecl::DefinitionKind VarDecl::hasDefinition(ASTContext &C) const {
  DefinitionKind Kind = DeclarationOnly;
  
  const VarDecl *First = getFirstDecl();
  for (auto I : First->redecls()) {
    Kind = std::max(Kind, I->isThisDeclarationADefinition(C));
    if (Kind == Definition)
      break;
  }

  return Kind;
}

const Expr *VarDecl::getAnyInitializer(const VarDecl *&D) const {
  for (auto I : redecls()) {
    if (auto Expr = I->getInit()) {
      D = I;
      return Expr;
    }
  }
  return nullptr;
}

bool VarDecl::isOutOfLine() const {
  if (Decl::isOutOfLine())
    return true;

  if (!isStaticDataMember())
    return false;

  // If this static data member was instantiated from a static data member of
  // a class template, check whether that static data member was defined 
  // out-of-line.
  if (VarDecl *VD = getInstantiatedFromStaticDataMember())
    return VD->isOutOfLine();
  
  return false;
}

VarDecl *VarDecl::getOutOfLineDefinition() {
  if (!isStaticDataMember())
    return nullptr;

  for (auto RD : redecls()) {
    if (RD->getLexicalDeclContext()->isFileContext())
      return RD;
  }

  return nullptr;
}

void VarDecl::setInit(Expr *I) {
  if (EvaluatedStmt *Eval = Init.dyn_cast<EvaluatedStmt *>()) {
    Eval->~EvaluatedStmt();
    getASTContext().Deallocate(Eval);
  }

  Init = I;
}

bool VarDecl::isUsableInConstantExpressions(ASTContext &C) const {
  const LangOptions &Lang = C.getLangOpts();
  (void)(Lang); // HLSL Change - this object is only accessed through static consts

  // HLSL Change: non-static globals are constant buffer variables that look
  // like const globals, but cannot be used in constant expressions.
  if (Lang.HLSL && hasGlobalStorage() && getStorageClass() != SC_Static)
    return false;

  if (!Lang.CPlusPlus)
    return false;

  // In C++11, any variable of reference type can be used in a constant
  // expression if it is initialized by a constant expression.
  if (Lang.CPlusPlus11 && getType()->isReferenceType())
    return true;

  // Only const objects can be used in constant expressions in C++. C++98 does
  // not require the variable to be non-volatile, but we consider this to be a
  // defect.
  if (!getType().isConstQualified() || getType().isVolatileQualified())
    return false;

  // In C++, const, non-volatile variables of integral or enumeration types
  // can be used in constant expressions.
  if (getType()->isIntegralOrEnumerationType())
    return true;

  // Additionally, in C++11, non-volatile constexpr variables can be used in
  // constant expressions.
  return Lang.CPlusPlus11 && isConstexpr();
}

/// Convert the initializer for this declaration to the elaborated EvaluatedStmt
/// form, which contains extra information on the evaluated value of the
/// initializer.
EvaluatedStmt *VarDecl::ensureEvaluatedStmt() const {
  EvaluatedStmt *Eval = Init.dyn_cast<EvaluatedStmt *>();
  if (!Eval) {
    Stmt *S = Init.get<Stmt *>();
    // Note: EvaluatedStmt contains an APValue, which usually holds
    // resources not allocated from the ASTContext.  We need to do some
    // work to avoid leaking those, but we do so in VarDecl::evaluateValue
    // where we can detect whether there's anything to clean up or not.
    Eval = new (getASTContext()) EvaluatedStmt;
    Eval->Value = S;
    Init = Eval;
  }
  return Eval;
}

APValue *VarDecl::evaluateValue() const {
  SmallVector<PartialDiagnosticAt, 8> Notes;
  return evaluateValue(Notes);
}

namespace {
// Destroy an APValue that was allocated in an ASTContext.
void DestroyAPValue(void* UntypedValue) {
  static_cast<APValue*>(UntypedValue)->~APValue();
}
} // namespace

APValue *VarDecl::evaluateValue(
    SmallVectorImpl<PartialDiagnosticAt> &Notes) const {
  EvaluatedStmt *Eval = ensureEvaluatedStmt();

  // We only produce notes indicating why an initializer is non-constant the
  // first time it is evaluated. FIXME: The notes won't always be emitted the
  // first time we try evaluation, so might not be produced at all.
  if (Eval->WasEvaluated)
    return Eval->Evaluated.isUninit() ? nullptr : &Eval->Evaluated;

  const Expr *Init = cast<Expr>(Eval->Value);
  assert(!Init->isValueDependent());

  if (Eval->IsEvaluating) {
    // FIXME: Produce a diagnostic for self-initialization.
    Eval->CheckedICE = true;
    Eval->IsICE = false;
    return nullptr;
  }

  Eval->IsEvaluating = true;

  bool Result = Init->EvaluateAsInitializer(Eval->Evaluated, getASTContext(),
                                            this, Notes);

  // Ensure the computed APValue is cleaned up later if evaluation succeeded,
  // or that it's empty (so that there's nothing to clean up) if evaluation
  // failed.
  if (!Result)
    Eval->Evaluated = APValue();
  else if (Eval->Evaluated.needsCleanup())
    getASTContext().AddDeallocation(DestroyAPValue, &Eval->Evaluated);

  Eval->IsEvaluating = false;
  Eval->WasEvaluated = true;

  // In C++11, we have determined whether the initializer was a constant
  // expression as a side-effect.
  if (getASTContext().getLangOpts().CPlusPlus11 && !Eval->CheckedICE) {
    Eval->CheckedICE = true;
    Eval->IsICE = Result && Notes.empty();
  }

  return Result ? &Eval->Evaluated : nullptr;
}

bool VarDecl::checkInitIsICE() const {
  // Initializers of weak variables are never ICEs.
  if (isWeak())
    return false;

  EvaluatedStmt *Eval = ensureEvaluatedStmt();
  if (Eval->CheckedICE)
    // We have already checked whether this subexpression is an
    // integral constant expression.
    return Eval->IsICE;

  const Expr *Init = cast<Expr>(Eval->Value);
  assert(!Init->isValueDependent());

  // In C++11, evaluate the initializer to check whether it's a constant
  // expression.
  if (getASTContext().getLangOpts().CPlusPlus11) {
    SmallVector<PartialDiagnosticAt, 8> Notes;
    evaluateValue(Notes);
    return Eval->IsICE;
  }

  // It's an ICE whether or not the definition we found is
  // out-of-line.  See DR 721 and the discussion in Clang PR
  // 6206 for details.

  if (Eval->CheckingICE)
    return false;
  Eval->CheckingICE = true;

  Eval->IsICE = Init->isIntegerConstantExpr(getASTContext());
  Eval->CheckingICE = false;
  Eval->CheckedICE = true;
  return Eval->IsICE;
}

VarDecl *VarDecl::getInstantiatedFromStaticDataMember() const {
  if (MemberSpecializationInfo *MSI = getMemberSpecializationInfo())
    return cast<VarDecl>(MSI->getInstantiatedFrom());

  return nullptr;
}

TemplateSpecializationKind VarDecl::getTemplateSpecializationKind() const {
  if (const VarTemplateSpecializationDecl *Spec =
          dyn_cast<VarTemplateSpecializationDecl>(this))
    return Spec->getSpecializationKind();

  if (MemberSpecializationInfo *MSI = getMemberSpecializationInfo())
    return MSI->getTemplateSpecializationKind();

  return TSK_Undeclared;
}

SourceLocation VarDecl::getPointOfInstantiation() const {
  if (const VarTemplateSpecializationDecl *Spec =
          dyn_cast<VarTemplateSpecializationDecl>(this))
    return Spec->getPointOfInstantiation();

  if (MemberSpecializationInfo *MSI = getMemberSpecializationInfo())
    return MSI->getPointOfInstantiation();

  return SourceLocation();
}

VarTemplateDecl *VarDecl::getDescribedVarTemplate() const {
  return getASTContext().getTemplateOrSpecializationInfo(this)
      .dyn_cast<VarTemplateDecl *>();
}

void VarDecl::setDescribedVarTemplate(VarTemplateDecl *Template) {
  getASTContext().setTemplateOrSpecializationInfo(this, Template);
}

MemberSpecializationInfo *VarDecl::getMemberSpecializationInfo() const {
  if (isStaticDataMember())
    // FIXME: Remove ?
    // return getASTContext().getInstantiatedFromStaticDataMember(this);
    return getASTContext().getTemplateOrSpecializationInfo(this)
        .dyn_cast<MemberSpecializationInfo *>();
  return nullptr;
}

void VarDecl::setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                                         SourceLocation PointOfInstantiation) {
  assert((isa<VarTemplateSpecializationDecl>(this) ||
          getMemberSpecializationInfo()) &&
         "not a variable or static data member template specialization");

  if (VarTemplateSpecializationDecl *Spec =
          dyn_cast<VarTemplateSpecializationDecl>(this)) {
    Spec->setSpecializationKind(TSK);
    if (TSK != TSK_ExplicitSpecialization && PointOfInstantiation.isValid() &&
        Spec->getPointOfInstantiation().isInvalid())
      Spec->setPointOfInstantiation(PointOfInstantiation);
  }

  if (MemberSpecializationInfo *MSI = getMemberSpecializationInfo()) {
    MSI->setTemplateSpecializationKind(TSK);
    if (TSK != TSK_ExplicitSpecialization && PointOfInstantiation.isValid() &&
        MSI->getPointOfInstantiation().isInvalid())
      MSI->setPointOfInstantiation(PointOfInstantiation);
  }
}

void
VarDecl::setInstantiationOfStaticDataMember(VarDecl *VD,
                                            TemplateSpecializationKind TSK) {
  assert(getASTContext().getTemplateOrSpecializationInfo(this).isNull() &&
         "Previous template or instantiation?");
  getASTContext().setInstantiatedFromStaticDataMember(this, VD, TSK);
}

//===----------------------------------------------------------------------===//
// ParmVarDecl Implementation
//===----------------------------------------------------------------------===//

ParmVarDecl *ParmVarDecl::Create(ASTContext &C, DeclContext *DC,
                                 SourceLocation StartLoc,
                                 SourceLocation IdLoc, IdentifierInfo *Id,
                                 QualType T, TypeSourceInfo *TInfo,
                                 StorageClass S, Expr *DefArg,
                                 hlsl::ParameterModifier ParamMod) {
  return new (C, DC) ParmVarDecl(ParmVar, C, DC, StartLoc, IdLoc, Id, T, TInfo,
                                 S, DefArg, ParamMod);
}

QualType ParmVarDecl::getOriginalType() const {
  TypeSourceInfo *TSI = getTypeSourceInfo();
  QualType T = TSI ? TSI->getType() : getType();
  if (const DecayedType *DT = dyn_cast<DecayedType>(T))
    return DT->getOriginalType();
  return T;
}

ParmVarDecl *ParmVarDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID)
      ParmVarDecl(ParmVar, C, nullptr, SourceLocation(), SourceLocation(),
                  nullptr, QualType(), nullptr, SC_None, nullptr, hlsl::ParameterModifier());
}

SourceRange ParmVarDecl::getSourceRange() const {
  if (!hasInheritedDefaultArg()) {
    SourceRange ArgRange = getDefaultArgRange();
    if (ArgRange.isValid())
      return SourceRange(getOuterLocStart(), ArgRange.getEnd());
  }

  // DeclaratorDecl considers the range of postfix types as overlapping with the
  // declaration name, but this is not the case with parameters in ObjC methods.
  if (isa<ObjCMethodDecl>(getDeclContext()))
    return SourceRange(DeclaratorDecl::getLocStart(), getLocation());

  return DeclaratorDecl::getSourceRange();
}

Expr *ParmVarDecl::getDefaultArg() {
  assert(!hasUnparsedDefaultArg() && "Default argument is not yet parsed!");
  assert(!hasUninstantiatedDefaultArg() &&
         "Default argument is not yet instantiated!");
  
  Expr *Arg = getInit();
  if (ExprWithCleanups *E = dyn_cast_or_null<ExprWithCleanups>(Arg))
    return E->getSubExpr();

  return Arg;
}

SourceRange ParmVarDecl::getDefaultArgRange() const {
  if (const Expr *E = getInit())
    return E->getSourceRange();

  if (hasUninstantiatedDefaultArg())
    return getUninstantiatedDefaultArg()->getSourceRange();

  return SourceRange();
}

bool ParmVarDecl::isParameterPack() const {
  return isa<PackExpansionType>(getType());
}

void ParmVarDecl::setParameterIndexLarge(unsigned parameterIndex) {
  getASTContext().setParameterIndex(this, parameterIndex);
  ParmVarDeclBits.ParameterIndex = ParameterIndexSentinel;
}

unsigned ParmVarDecl::getParameterIndexLarge() const {
  return getASTContext().getParameterIndex(this);
}

// HLSL Change Begins
void ParmVarDecl::updateOutParamToRefType(ASTContext &C) {
  // Aggregate type will be indirect param convert to pointer type.
  // So don't update to ReferenceType.
  if ((!getType()->isArrayType() && !getType()->isRecordType()) ||
      hlsl::IsHLSLVecMatType(getType()))
    setType(C.getLValueReferenceType(getType(), false));
  // Add restrict to out param.
  QualType QT = getType();
  QT.addRestrict();
  setType(QT);
}
// HLSL Change Ends

//===----------------------------------------------------------------------===//
// FunctionDecl Implementation
//===----------------------------------------------------------------------===//

void FunctionDecl::getNameForDiagnostic(
    raw_ostream &OS, const PrintingPolicy &Policy, bool Qualified) const {
  NamedDecl::getNameForDiagnostic(OS, Policy, Qualified);
  const TemplateArgumentList *TemplateArgs = getTemplateSpecializationArgs();
  if (TemplateArgs)
    TemplateSpecializationType::PrintTemplateArgumentList(
        OS, TemplateArgs->data(), TemplateArgs->size(), Policy);
}

bool FunctionDecl::isVariadic() const {
  if (const FunctionProtoType *FT = getType()->getAs<FunctionProtoType>())
    return FT->isVariadic();
  return false;
}

bool FunctionDecl::hasBody(const FunctionDecl *&Definition) const {
  for (auto I : redecls()) {
    if (I->Body || I->IsLateTemplateParsed) {
      Definition = I;
      return true;
    }
  }

  return false;
}

bool FunctionDecl::hasTrivialBody() const
{
  Stmt *S = getBody();
  if (!S) {
    // Since we don't have a body for this function, we don't know if it's
    // trivial or not.
    return false;
  }

  if (isa<CompoundStmt>(S) && cast<CompoundStmt>(S)->body_empty())
    return true;
  return false;
}

bool FunctionDecl::isDefined(const FunctionDecl *&Definition) const {
  for (auto I : redecls()) {
    if (I->IsDeleted || I->IsDefaulted || I->Body || I->IsLateTemplateParsed ||
        I->hasAttr<AliasAttr>()) {
      Definition = I->IsDeleted ? I->getCanonicalDecl() : I;
      return true;
    }
  }

  return false;
}

Stmt *FunctionDecl::getBody(const FunctionDecl *&Definition) const {
  if (!hasBody(Definition))
    return nullptr;

  if (Definition->Body)
    return Definition->Body.get(getASTContext().getExternalSource());

  return nullptr;
}

void FunctionDecl::setBody(Stmt *B) {
  Body = B;
  if (B)
    EndRangeLoc = B->getLocEnd();
}

void FunctionDecl::setPure(bool P) {
  IsPure = P;
  if (P)
    if (CXXRecordDecl *Parent = dyn_cast<CXXRecordDecl>(getDeclContext()))
      Parent->markedVirtualFunctionPure();
}

template<std::size_t Len>
static bool isNamed(const NamedDecl *ND, const char (&Str)[Len]) {
  IdentifierInfo *II = ND->getIdentifier();
  return II && II->isStr(Str);
}

bool FunctionDecl::isMain() const {
  const TranslationUnitDecl *tunit =
    dyn_cast<TranslationUnitDecl>(getDeclContext()->getRedeclContext());
  return tunit &&
         !tunit->getASTContext().getLangOpts().Freestanding &&
         isNamed(this, "main");
}

bool FunctionDecl::isMSVCRTEntryPoint() const {
  const TranslationUnitDecl *TUnit =
      dyn_cast<TranslationUnitDecl>(getDeclContext()->getRedeclContext());
  if (!TUnit)
    return false;

  // Even though we aren't really targeting MSVCRT if we are freestanding,
  // semantic analysis for these functions remains the same.

  // MSVCRT entry points only exist on MSVCRT targets.
  if (!TUnit->getASTContext().getTargetInfo().getTriple().isOSMSVCRT())
    return false;

  // Nameless functions like constructors cannot be entry points.
  if (!getIdentifier())
    return false;

  return llvm::StringSwitch<bool>(getName())
      .Cases("main",     // an ANSI console app
             "wmain",    // a Unicode console App
             "WinMain",  // an ANSI GUI app
             "wWinMain", // a Unicode GUI app
             "DllMain",  // a DLL
             true)
      .Default(false);
}

bool FunctionDecl::isReservedGlobalPlacementOperator() const {
  assert(getDeclName().getNameKind() == DeclarationName::CXXOperatorName);
  assert(getDeclName().getCXXOverloadedOperator() == OO_New ||
         getDeclName().getCXXOverloadedOperator() == OO_Delete ||
         getDeclName().getCXXOverloadedOperator() == OO_Array_New ||
         getDeclName().getCXXOverloadedOperator() == OO_Array_Delete);

  if (!getDeclContext()->getRedeclContext()->isTranslationUnit())
    return false;

  const FunctionProtoType *proto = getType()->castAs<FunctionProtoType>();
  if (proto->getNumParams() != 2 || proto->isVariadic())
    return false;

  ASTContext &Context =
    cast<TranslationUnitDecl>(getDeclContext()->getRedeclContext())
      ->getASTContext();

  // The result type and first argument type are constant across all
  // these operators.  The second argument must be exactly void*.
  return (proto->getParamType(1).getCanonicalType() == Context.VoidPtrTy);
}

bool FunctionDecl::isReplaceableGlobalAllocationFunction() const {
  if (getDeclName().getNameKind() != DeclarationName::CXXOperatorName)
    return false;
  if (getDeclName().getCXXOverloadedOperator() != OO_New &&
      getDeclName().getCXXOverloadedOperator() != OO_Delete &&
      getDeclName().getCXXOverloadedOperator() != OO_Array_New &&
      getDeclName().getCXXOverloadedOperator() != OO_Array_Delete)
    return false;

  if (isa<CXXRecordDecl>(getDeclContext()))
    return false;

  // This can only fail for an invalid 'operator new' declaration.
  if (!getDeclContext()->getRedeclContext()->isTranslationUnit())
    return false;

  const FunctionProtoType *FPT = getType()->castAs<FunctionProtoType>();
  if (FPT->getNumParams() == 0 || FPT->getNumParams() > 2 || FPT->isVariadic())
    return false;

  // If this is a single-parameter function, it must be a replaceable global
  // allocation or deallocation function.
  if (FPT->getNumParams() == 1)
    return true;

  // Otherwise, we're looking for a second parameter whose type is
  // 'const std::nothrow_t &', or, in C++1y, 'std::size_t'.
  QualType Ty = FPT->getParamType(1);
  ASTContext &Ctx = getASTContext();
  if (Ctx.getLangOpts().SizedDeallocation &&
      Ctx.hasSameType(Ty, Ctx.getSizeType()))
    return true;
  if (!Ty->isReferenceType())
    return false;
  Ty = Ty->getPointeeType();
  if (Ty.getCVRQualifiers() != Qualifiers::Const)
    return false;
  const CXXRecordDecl *RD = Ty->getAsCXXRecordDecl();
  return RD && isNamed(RD, "nothrow_t") && RD->isInStdNamespace();
}

LanguageLinkage FunctionDecl::getLanguageLinkage() const {
  return getDeclLanguageLinkage(*this);
}

bool FunctionDecl::isExternC() const {
  return isDeclExternC(*this);
}

bool FunctionDecl::isInExternCContext() const {
  return getLexicalDeclContext()->isExternCContext();
}

bool FunctionDecl::isInExternCXXContext() const {
  return getLexicalDeclContext()->isExternCXXContext();
}

bool FunctionDecl::isGlobal() const {
  if (const CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(this))
    return Method->isStatic();

  if (getCanonicalDecl()->getStorageClass() == SC_Static)
    return false;

  for (const DeclContext *DC = getDeclContext();
       DC->isNamespace();
       DC = DC->getParent()) {
    if (const NamespaceDecl *Namespace = cast<NamespaceDecl>(DC)) {
      if (!Namespace->getDeclName())
        return false;
      break;
    }
  }

  return true;
}

bool FunctionDecl::isNoReturn() const {
  return hasAttr<NoReturnAttr>() || hasAttr<CXX11NoReturnAttr>() ||
         hasAttr<C11NoReturnAttr>() ||
         getType()->getAs<FunctionType>()->getNoReturnAttr();
}

void
FunctionDecl::setPreviousDeclaration(FunctionDecl *PrevDecl) {
  redeclarable_base::setPreviousDecl(PrevDecl);

  if (FunctionTemplateDecl *FunTmpl = getDescribedFunctionTemplate()) {
    FunctionTemplateDecl *PrevFunTmpl
      = PrevDecl? PrevDecl->getDescribedFunctionTemplate() : nullptr;
    assert((!PrevDecl || PrevFunTmpl) && "Function/function template mismatch");
    FunTmpl->setPreviousDecl(PrevFunTmpl);
  }
  
  if (PrevDecl && PrevDecl->IsInline)
    IsInline = true;
}

FunctionDecl *FunctionDecl::getCanonicalDecl() { return getFirstDecl(); }

/// \brief Returns a value indicating whether this function
/// corresponds to a builtin function.
///
/// The function corresponds to a built-in function if it is
/// declared at translation scope or within an extern "C" block and
/// its name matches with the name of a builtin. The returned value
/// will be 0 for functions that do not correspond to a builtin, a
/// value of type \c Builtin::ID if in the target-independent range
/// \c [1,Builtin::First), or a target-specific builtin value.
unsigned FunctionDecl::getBuiltinID() const {
  if (!getIdentifier())
    return 0;

  unsigned BuiltinID = getIdentifier()->getBuiltinID();
  if (!BuiltinID)
    return 0;

  ASTContext &Context = getASTContext();
  if (Context.getLangOpts().CPlusPlus) {
    const LinkageSpecDecl *LinkageDecl = dyn_cast<LinkageSpecDecl>(
        getFirstDecl()->getDeclContext());
    // In C++, the first declaration of a builtin is always inside an implicit
    // extern "C".
    // FIXME: A recognised library function may not be directly in an extern "C"
    // declaration, for instance "extern "C" { namespace std { decl } }".
    if (!LinkageDecl) {
      if (BuiltinID == Builtin::BI__GetExceptionInfo &&
          Context.getTargetInfo().getCXXABI().isMicrosoft() &&
          isInStdNamespace())
        return Builtin::BI__GetExceptionInfo;
      return 0;
    }
    if (LinkageDecl->getLanguage() != LinkageSpecDecl::lang_c)
      return 0;
  }

  // If the function is marked "overloadable", it has a different mangled name
  // and is not the C library function.
  if (hasAttr<OverloadableAttr>())
    return 0;

  if (!Context.BuiltinInfo.isPredefinedLibFunction(BuiltinID))
    return BuiltinID;

  // This function has the name of a known C library
  // function. Determine whether it actually refers to the C library
  // function or whether it just has the same name.

  // If this is a static function, it's not a builtin.
  if (getStorageClass() == SC_Static)
    return 0;

  return BuiltinID;
}


/// getNumParams - Return the number of parameters this function must have
/// based on its FunctionType.  This is the length of the ParamInfo array
/// after it has been created.
unsigned FunctionDecl::getNumParams() const {
  const FunctionProtoType *FPT = getType()->getAs<FunctionProtoType>();
  return FPT ? FPT->getNumParams() : 0;
}

void FunctionDecl::setParams(ASTContext &C,
                             ArrayRef<ParmVarDecl *> NewParamInfo) {
  assert(!ParamInfo && "Already has param info!");
  assert(NewParamInfo.size() == getNumParams() && "Parameter count mismatch!");

  // Zero params -> null pointer.
  if (!NewParamInfo.empty()) {
    ParamInfo = new (C) ParmVarDecl*[NewParamInfo.size()];
    std::copy(NewParamInfo.begin(), NewParamInfo.end(), ParamInfo);
  }
}

void FunctionDecl::setDeclsInPrototypeScope(ArrayRef<NamedDecl *> NewDecls) {
  assert(DeclsInPrototypeScope.empty() && "Already has prototype decls!");

  if (!NewDecls.empty()) {
    NamedDecl **A = new (getASTContext()) NamedDecl*[NewDecls.size()];
    std::copy(NewDecls.begin(), NewDecls.end(), A);
    DeclsInPrototypeScope = llvm::makeArrayRef(A, NewDecls.size());
    // Move declarations introduced in prototype to the function context.
    for (auto I : NewDecls) {
      DeclContext *DC = I->getDeclContext();
      // Forward-declared reference to an enumeration is not added to
      // declaration scope, so skip declaration that is absent from its
      // declaration contexts.
      if (DC->containsDecl(I)) {
          DC->removeDecl(I);
          I->setDeclContext(this);
          addDecl(I);
      }
    }
  }
}

/// getMinRequiredArguments - Returns the minimum number of arguments
/// needed to call this function. This may be fewer than the number of
/// function parameters, if some of the parameters have default
/// arguments (in C++) or are parameter packs (C++11).
unsigned FunctionDecl::getMinRequiredArguments() const {
  if (!getASTContext().getLangOpts().CPlusPlus)
    return getNumParams();

  unsigned NumRequiredArgs = 0;
  for (auto *Param : params())
    if (!Param->isParameterPack() && !Param->hasDefaultArg())
      ++NumRequiredArgs;
  return NumRequiredArgs;
}

/// \brief The combination of the extern and inline keywords under MSVC forces
/// the function to be required.
///
/// Note: This function assumes that we will only get called when isInlined()
/// would return true for this FunctionDecl.
bool FunctionDecl::isMSExternInline() const {
  assert(isInlined() && "expected to get called on an inlined function!");

  const ASTContext &Context = getASTContext();
  if (!Context.getLangOpts().MSVCCompat && !hasAttr<DLLExportAttr>())
    return false;

  for (const FunctionDecl *FD = getMostRecentDecl(); FD;
       FD = FD->getPreviousDecl())
    if (!FD->isImplicit() && FD->getStorageClass() == SC_Extern)
      return true;

  return false;
}

static bool redeclForcesDefMSVC(const FunctionDecl *Redecl) {
  if (Redecl->getStorageClass() != SC_Extern)
    return false;

  for (const FunctionDecl *FD = Redecl->getPreviousDecl(); FD;
       FD = FD->getPreviousDecl())
    if (!FD->isImplicit() && FD->getStorageClass() == SC_Extern)
      return false;

  return true;
}

static bool RedeclForcesDefC99(const FunctionDecl *Redecl) {
  // Only consider file-scope declarations in this test.
  if (!Redecl->getLexicalDeclContext()->isTranslationUnit())
    return false;

  // Only consider explicit declarations; the presence of a builtin for a
  // libcall shouldn't affect whether a definition is externally visible.
  if (Redecl->isImplicit())
    return false;

  if (!Redecl->isInlineSpecified() || Redecl->getStorageClass() == SC_Extern) 
    return true; // Not an inline definition

  return false;
}

/// \brief For a function declaration in C or C++, determine whether this
/// declaration causes the definition to be externally visible.
///
/// For instance, this determines if adding the current declaration to the set
/// of redeclarations of the given functions causes
/// isInlineDefinitionExternallyVisible to change from false to true.
bool FunctionDecl::doesDeclarationForceExternallyVisibleDefinition() const {
  assert(!doesThisDeclarationHaveABody() &&
         "Must have a declaration without a body.");

  ASTContext &Context = getASTContext();

  if (Context.getLangOpts().MSVCCompat) {
    const FunctionDecl *Definition;
    if (hasBody(Definition) && Definition->isInlined() &&
        redeclForcesDefMSVC(this))
      return true;
  }

  if (Context.getLangOpts().GNUInline || hasAttr<GNUInlineAttr>()) {
    // With GNU inlining, a declaration with 'inline' but not 'extern', forces
    // an externally visible definition.
    //
    // FIXME: What happens if gnu_inline gets added on after the first
    // declaration?
    if (!isInlineSpecified() || getStorageClass() == SC_Extern)
      return false;

    const FunctionDecl *Prev = this;
    bool FoundBody = false;
    while ((Prev = Prev->getPreviousDecl())) {
      FoundBody |= Prev->Body.isValid();

      if (Prev->Body) {
        // If it's not the case that both 'inline' and 'extern' are
        // specified on the definition, then it is always externally visible.
        if (!Prev->isInlineSpecified() ||
            Prev->getStorageClass() != SC_Extern)
          return false;
      } else if (Prev->isInlineSpecified() && 
                 Prev->getStorageClass() != SC_Extern) {
        return false;
      }
    }
    return FoundBody;
  }

  if (Context.getLangOpts().CPlusPlus)
    return false;

  // C99 6.7.4p6:
  //   [...] If all of the file scope declarations for a function in a 
  //   translation unit include the inline function specifier without extern, 
  //   then the definition in that translation unit is an inline definition.
  if (isInlineSpecified() && getStorageClass() != SC_Extern)
    return false;
  const FunctionDecl *Prev = this;
  bool FoundBody = false;
  while ((Prev = Prev->getPreviousDecl())) {
    FoundBody |= Prev->Body.isValid();
    if (RedeclForcesDefC99(Prev))
      return false;
  }
  return FoundBody;
}

SourceRange FunctionDecl::getReturnTypeSourceRange() const {
  const TypeSourceInfo *TSI = getTypeSourceInfo();
  if (!TSI)
    return SourceRange();
  FunctionTypeLoc FTL =
      TSI->getTypeLoc().IgnoreParens().getAs<FunctionTypeLoc>();
  if (!FTL)
    return SourceRange();

  // Skip self-referential return types.
  const SourceManager &SM = getASTContext().getSourceManager();
  SourceRange RTRange = FTL.getReturnLoc().getSourceRange();
  SourceLocation Boundary = getNameInfo().getLocStart();
  if (RTRange.isInvalid() || Boundary.isInvalid() ||
      !SM.isBeforeInTranslationUnit(RTRange.getEnd(), Boundary))
    return SourceRange();

  return RTRange;
}

bool FunctionDecl::hasUnusedResultAttr() const {
  QualType RetType = getReturnType();
  if (RetType->isRecordType()) {
    const CXXRecordDecl *Ret = RetType->getAsCXXRecordDecl();
    const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(this);
    if (Ret && Ret->hasAttr<WarnUnusedResultAttr>() &&
        !(MD && MD->getCorrespondingMethodInClass(Ret, true)))
      return true;
  }
  return hasAttr<WarnUnusedResultAttr>();
}

/// \brief For an inline function definition in C, or for a gnu_inline function
/// in C++, determine whether the definition will be externally visible.
///
/// Inline function definitions are always available for inlining optimizations.
/// However, depending on the language dialect, declaration specifiers, and
/// attributes, the definition of an inline function may or may not be
/// "externally" visible to other translation units in the program.
///
/// In C99, inline definitions are not externally visible by default. However,
/// if even one of the global-scope declarations is marked "extern inline", the
/// inline definition becomes externally visible (C99 6.7.4p6).
///
/// In GNU89 mode, or if the gnu_inline attribute is attached to the function
/// definition, we use the GNU semantics for inline, which are nearly the 
/// opposite of C99 semantics. In particular, "inline" by itself will create 
/// an externally visible symbol, but "extern inline" will not create an 
/// externally visible symbol.
bool FunctionDecl::isInlineDefinitionExternallyVisible() const {
  assert(doesThisDeclarationHaveABody() && "Must have the function definition");
  assert(isInlined() && "Function must be inline");
  ASTContext &Context = getASTContext();
  
  if (Context.getLangOpts().GNUInline || hasAttr<GNUInlineAttr>()) {
    // Note: If you change the logic here, please change
    // doesDeclarationForceExternallyVisibleDefinition as well.
    //
    // If it's not the case that both 'inline' and 'extern' are
    // specified on the definition, then this inline definition is
    // externally visible.
    if (!(isInlineSpecified() && getStorageClass() == SC_Extern))
      return true;
    
    // If any declaration is 'inline' but not 'extern', then this definition
    // is externally visible.
    for (auto Redecl : redecls()) {
      if (Redecl->isInlineSpecified() && 
          Redecl->getStorageClass() != SC_Extern)
        return true;
    }    
    
    return false;
  }

  // The rest of this function is C-only.
  assert(!Context.getLangOpts().CPlusPlus &&
         "should not use C inline rules in C++");

  // C99 6.7.4p6:
  //   [...] If all of the file scope declarations for a function in a 
  //   translation unit include the inline function specifier without extern, 
  //   then the definition in that translation unit is an inline definition.
  for (auto Redecl : redecls()) {
    if (RedeclForcesDefC99(Redecl))
      return true;
  }
  
  // C99 6.7.4p6:
  //   An inline definition does not provide an external definition for the 
  //   function, and does not forbid an external definition in another 
  //   translation unit.
  return false;
}

/// getOverloadedOperator - Which C++ overloaded operator this
/// function represents, if any.
OverloadedOperatorKind FunctionDecl::getOverloadedOperator() const {
  if (getDeclName().getNameKind() == DeclarationName::CXXOperatorName)
    return getDeclName().getCXXOverloadedOperator();
  else
    return OO_None;
}

/// getLiteralIdentifier - The literal suffix identifier this function
/// represents, if any.
const IdentifierInfo *FunctionDecl::getLiteralIdentifier() const {
  if (getDeclName().getNameKind() == DeclarationName::CXXLiteralOperatorName)
    return getDeclName().getCXXLiteralIdentifier();
  else
    return nullptr;
}

FunctionDecl::TemplatedKind FunctionDecl::getTemplatedKind() const {
  if (TemplateOrSpecialization.isNull())
    return TK_NonTemplate;
  if (TemplateOrSpecialization.is<FunctionTemplateDecl *>())
    return TK_FunctionTemplate;
  if (TemplateOrSpecialization.is<MemberSpecializationInfo *>())
    return TK_MemberSpecialization;
  if (TemplateOrSpecialization.is<FunctionTemplateSpecializationInfo *>())
    return TK_FunctionTemplateSpecialization;
  if (TemplateOrSpecialization.is
                               <DependentFunctionTemplateSpecializationInfo*>())
    return TK_DependentFunctionTemplateSpecialization;

  llvm_unreachable("Did we miss a TemplateOrSpecialization type?");
}

FunctionDecl *FunctionDecl::getInstantiatedFromMemberFunction() const {
  if (MemberSpecializationInfo *Info = getMemberSpecializationInfo())
    return cast<FunctionDecl>(Info->getInstantiatedFrom());

  return nullptr;
}

void 
FunctionDecl::setInstantiationOfMemberFunction(ASTContext &C,
                                               FunctionDecl *FD,
                                               TemplateSpecializationKind TSK) {
  assert(TemplateOrSpecialization.isNull() && 
         "Member function is already a specialization");
  MemberSpecializationInfo *Info 
    = new (C) MemberSpecializationInfo(FD, TSK);
  TemplateOrSpecialization = Info;
}

bool FunctionDecl::isImplicitlyInstantiable() const {
  // If the function is invalid, it can't be implicitly instantiated.
  if (isInvalidDecl())
    return false;
  
  switch (getTemplateSpecializationKind()) {
  case TSK_Undeclared:
  case TSK_ExplicitInstantiationDefinition:
    return false;
      
  case TSK_ImplicitInstantiation:
    return true;

  // It is possible to instantiate TSK_ExplicitSpecialization kind
  // if the FunctionDecl has a class scope specialization pattern.
  case TSK_ExplicitSpecialization:
    return getClassScopeSpecializationPattern() != nullptr;

  case TSK_ExplicitInstantiationDeclaration:
    // Handled below.
    break;
  }

  // Find the actual template from which we will instantiate.
  const FunctionDecl *PatternDecl = getTemplateInstantiationPattern();
  bool HasPattern = false;
  if (PatternDecl)
    HasPattern = PatternDecl->hasBody(PatternDecl);
  
  // C++0x [temp.explicit]p9:
  //   Except for inline functions, other explicit instantiation declarations
  //   have the effect of suppressing the implicit instantiation of the entity
  //   to which they refer. 
  if (!HasPattern || !PatternDecl) 
    return true;

  return PatternDecl->isInlined();
}

bool FunctionDecl::isTemplateInstantiation() const {
  switch (getTemplateSpecializationKind()) {
    case TSK_Undeclared:
    case TSK_ExplicitSpecialization:
      return false;      
    case TSK_ImplicitInstantiation:
    case TSK_ExplicitInstantiationDeclaration:
    case TSK_ExplicitInstantiationDefinition:
      return true;
  }
  llvm_unreachable("All TSK values handled.");
}
   
FunctionDecl *FunctionDecl::getTemplateInstantiationPattern() const {
  // Handle class scope explicit specialization special case.
  if (getTemplateSpecializationKind() == TSK_ExplicitSpecialization)
    return getClassScopeSpecializationPattern();
  
  // If this is a generic lambda call operator specialization, its 
  // instantiation pattern is always its primary template's pattern
  // even if its primary template was instantiated from another 
  // member template (which happens with nested generic lambdas).
  // Since a lambda's call operator's body is transformed eagerly, 
  // we don't have to go hunting for a prototype definition template 
  // (i.e. instantiated-from-member-template) to use as an instantiation 
  // pattern.

  if (isGenericLambdaCallOperatorSpecialization(
          dyn_cast<CXXMethodDecl>(this))) {
    assert(getPrimaryTemplate() && "A generic lambda specialization must be "
                                   "generated from a primary call operator "
                                   "template");
    assert(getPrimaryTemplate()->getTemplatedDecl()->getBody() &&
           "A generic lambda call operator template must always have a body - "
           "even if instantiated from a prototype (i.e. as written) member "
           "template");
    return getPrimaryTemplate()->getTemplatedDecl();
  }
  
  if (FunctionTemplateDecl *Primary = getPrimaryTemplate()) {
    while (Primary->getInstantiatedFromMemberTemplate()) {
      // If we have hit a point where the user provided a specialization of
      // this template, we're done looking.
      if (Primary->isMemberSpecialization())
        break;
      Primary = Primary->getInstantiatedFromMemberTemplate();
    }
    
    return Primary->getTemplatedDecl();
  } 
    
  return getInstantiatedFromMemberFunction();
}

FunctionTemplateDecl *FunctionDecl::getPrimaryTemplate() const {
  if (FunctionTemplateSpecializationInfo *Info
        = TemplateOrSpecialization
            .dyn_cast<FunctionTemplateSpecializationInfo*>()) {
    return Info->Template.getPointer();
  }
  return nullptr;
}

FunctionDecl *FunctionDecl::getClassScopeSpecializationPattern() const {
    return getASTContext().getClassScopeSpecializationPattern(this);
}

const TemplateArgumentList *
FunctionDecl::getTemplateSpecializationArgs() const {
  if (FunctionTemplateSpecializationInfo *Info
        = TemplateOrSpecialization
            .dyn_cast<FunctionTemplateSpecializationInfo*>()) {
    return Info->TemplateArguments;
  }
  return nullptr;
}

const ASTTemplateArgumentListInfo *
FunctionDecl::getTemplateSpecializationArgsAsWritten() const {
  if (FunctionTemplateSpecializationInfo *Info
        = TemplateOrSpecialization
            .dyn_cast<FunctionTemplateSpecializationInfo*>()) {
    return Info->TemplateArgumentsAsWritten;
  }
  return nullptr;
}

void
FunctionDecl::setFunctionTemplateSpecialization(ASTContext &C,
                                                FunctionTemplateDecl *Template,
                                     const TemplateArgumentList *TemplateArgs,
                                                void *InsertPos,
                                                TemplateSpecializationKind TSK,
                        const TemplateArgumentListInfo *TemplateArgsAsWritten,
                                          SourceLocation PointOfInstantiation) {
  assert(TSK != TSK_Undeclared && 
         "Must specify the type of function template specialization");
  FunctionTemplateSpecializationInfo *Info
    = TemplateOrSpecialization.dyn_cast<FunctionTemplateSpecializationInfo*>();
  if (!Info)
    Info = FunctionTemplateSpecializationInfo::Create(C, this, Template, TSK,
                                                      TemplateArgs,
                                                      TemplateArgsAsWritten,
                                                      PointOfInstantiation);
  TemplateOrSpecialization = Info;
  Template->addSpecialization(Info, InsertPos);
}

void
FunctionDecl::setDependentTemplateSpecialization(ASTContext &Context,
                                    const UnresolvedSetImpl &Templates,
                             const TemplateArgumentListInfo &TemplateArgs) {
  assert(TemplateOrSpecialization.isNull());
  size_t Size = sizeof(DependentFunctionTemplateSpecializationInfo);
  Size += Templates.size() * sizeof(FunctionTemplateDecl*);
  Size += TemplateArgs.size() * sizeof(TemplateArgumentLoc);
  void *Buffer = Context.Allocate(Size);
  DependentFunctionTemplateSpecializationInfo *Info =
    new (Buffer) DependentFunctionTemplateSpecializationInfo(Templates,
                                                             TemplateArgs);
  TemplateOrSpecialization = Info;
}

DependentFunctionTemplateSpecializationInfo::
DependentFunctionTemplateSpecializationInfo(const UnresolvedSetImpl &Ts,
                                      const TemplateArgumentListInfo &TArgs)
  : AngleLocs(TArgs.getLAngleLoc(), TArgs.getRAngleLoc()) {
  static_assert(sizeof(*this) % llvm::AlignOf<void *>::Alignment == 0,
                "Trailing data is unaligned!");

  d.NumTemplates = Ts.size();
  d.NumArgs = TArgs.size();

  FunctionTemplateDecl **TsArray =
    const_cast<FunctionTemplateDecl**>(getTemplates());
  for (unsigned I = 0, E = Ts.size(); I != E; ++I)
    TsArray[I] = cast<FunctionTemplateDecl>(Ts[I]->getUnderlyingDecl());

  TemplateArgumentLoc *ArgsArray =
    const_cast<TemplateArgumentLoc*>(getTemplateArgs());
  for (unsigned I = 0, E = TArgs.size(); I != E; ++I)
    new (&ArgsArray[I]) TemplateArgumentLoc(TArgs[I]);
}

TemplateSpecializationKind FunctionDecl::getTemplateSpecializationKind() const {
  // For a function template specialization, query the specialization
  // information object.
  FunctionTemplateSpecializationInfo *FTSInfo
    = TemplateOrSpecialization.dyn_cast<FunctionTemplateSpecializationInfo*>();
  if (FTSInfo)
    return FTSInfo->getTemplateSpecializationKind();

  MemberSpecializationInfo *MSInfo
    = TemplateOrSpecialization.dyn_cast<MemberSpecializationInfo*>();
  if (MSInfo)
    return MSInfo->getTemplateSpecializationKind();
  
  return TSK_Undeclared;
}

void
FunctionDecl::setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                                          SourceLocation PointOfInstantiation) {
  if (FunctionTemplateSpecializationInfo *FTSInfo
        = TemplateOrSpecialization.dyn_cast<
                                    FunctionTemplateSpecializationInfo*>()) {
    FTSInfo->setTemplateSpecializationKind(TSK);
    if (TSK != TSK_ExplicitSpecialization &&
        PointOfInstantiation.isValid() &&
        FTSInfo->getPointOfInstantiation().isInvalid())
      FTSInfo->setPointOfInstantiation(PointOfInstantiation);
  } else if (MemberSpecializationInfo *MSInfo
             = TemplateOrSpecialization.dyn_cast<MemberSpecializationInfo*>()) {
    MSInfo->setTemplateSpecializationKind(TSK);
    if (TSK != TSK_ExplicitSpecialization &&
        PointOfInstantiation.isValid() &&
        MSInfo->getPointOfInstantiation().isInvalid())
      MSInfo->setPointOfInstantiation(PointOfInstantiation);
  } else
    llvm_unreachable("Function cannot have a template specialization kind");
}

SourceLocation FunctionDecl::getPointOfInstantiation() const {
  if (FunctionTemplateSpecializationInfo *FTSInfo
        = TemplateOrSpecialization.dyn_cast<
                                        FunctionTemplateSpecializationInfo*>())
    return FTSInfo->getPointOfInstantiation();
  else if (MemberSpecializationInfo *MSInfo
             = TemplateOrSpecialization.dyn_cast<MemberSpecializationInfo*>())
    return MSInfo->getPointOfInstantiation();
  
  return SourceLocation();
}

bool FunctionDecl::isOutOfLine() const {
  if (Decl::isOutOfLine())
    return true;
  
  // If this function was instantiated from a member function of a 
  // class template, check whether that member function was defined out-of-line.
  if (FunctionDecl *FD = getInstantiatedFromMemberFunction()) {
    const FunctionDecl *Definition;
    if (FD->hasBody(Definition))
      return Definition->isOutOfLine();
  }
  
  // If this function was instantiated from a function template,
  // check whether that function template was defined out-of-line.
  if (FunctionTemplateDecl *FunTmpl = getPrimaryTemplate()) {
    const FunctionDecl *Definition;
    if (FunTmpl->getTemplatedDecl()->hasBody(Definition))
      return Definition->isOutOfLine();
  }
  
  return false;
}

SourceRange FunctionDecl::getSourceRange() const {
  return SourceRange(getOuterLocStart(), EndRangeLoc);
}

unsigned FunctionDecl::getMemoryFunctionKind() const {
  IdentifierInfo *FnInfo = getIdentifier();

  if (!FnInfo)
    return 0;
    
  // Builtin handling.
  switch (getBuiltinID()) {
  case Builtin::BI__builtin_memset:
  case Builtin::BI__builtin___memset_chk:
  case Builtin::BImemset:
    return Builtin::BImemset;

  case Builtin::BI__builtin_memcpy:
  case Builtin::BI__builtin___memcpy_chk:
  case Builtin::BImemcpy:
    return Builtin::BImemcpy;

  case Builtin::BI__builtin_memmove:
  case Builtin::BI__builtin___memmove_chk:
  case Builtin::BImemmove:
    return Builtin::BImemmove;

  case Builtin::BIstrlcpy:
  case Builtin::BI__builtin___strlcpy_chk:
    return Builtin::BIstrlcpy;

  case Builtin::BIstrlcat:
  case Builtin::BI__builtin___strlcat_chk:
    return Builtin::BIstrlcat;

  case Builtin::BI__builtin_memcmp:
  case Builtin::BImemcmp:
    return Builtin::BImemcmp;

  case Builtin::BI__builtin_strncpy:
  case Builtin::BI__builtin___strncpy_chk:
  case Builtin::BIstrncpy:
    return Builtin::BIstrncpy;

  case Builtin::BI__builtin_strncmp:
  case Builtin::BIstrncmp:
    return Builtin::BIstrncmp;

  case Builtin::BI__builtin_strncasecmp:
  case Builtin::BIstrncasecmp:
    return Builtin::BIstrncasecmp;

  case Builtin::BI__builtin_strncat:
  case Builtin::BI__builtin___strncat_chk:
  case Builtin::BIstrncat:
    return Builtin::BIstrncat;

  case Builtin::BI__builtin_strndup:
  case Builtin::BIstrndup:
    return Builtin::BIstrndup;

  case Builtin::BI__builtin_strlen:
  case Builtin::BIstrlen:
    return Builtin::BIstrlen;

  default:
    if (isExternC()) {
      if (FnInfo->isStr("memset"))
        return Builtin::BImemset;
      else if (FnInfo->isStr("memcpy"))
        return Builtin::BImemcpy;
      else if (FnInfo->isStr("memmove"))
        return Builtin::BImemmove;
      else if (FnInfo->isStr("memcmp"))
        return Builtin::BImemcmp;
      else if (FnInfo->isStr("strncpy"))
        return Builtin::BIstrncpy;
      else if (FnInfo->isStr("strncmp"))
        return Builtin::BIstrncmp;
      else if (FnInfo->isStr("strncasecmp"))
        return Builtin::BIstrncasecmp;
      else if (FnInfo->isStr("strncat"))
        return Builtin::BIstrncat;
      else if (FnInfo->isStr("strndup"))
        return Builtin::BIstrndup;
      else if (FnInfo->isStr("strlen"))
        return Builtin::BIstrlen;
    }
    break;
  }
  return 0;
}

//===----------------------------------------------------------------------===//
// FieldDecl Implementation
//===----------------------------------------------------------------------===//

FieldDecl *FieldDecl::Create(const ASTContext &C, DeclContext *DC,
                             SourceLocation StartLoc, SourceLocation IdLoc,
                             IdentifierInfo *Id, QualType T,
                             TypeSourceInfo *TInfo, Expr *BW, bool Mutable,
                             InClassInitStyle InitStyle) {
  return new (C, DC) FieldDecl(Decl::Field, DC, StartLoc, IdLoc, Id, T, TInfo,
                               BW, Mutable, InitStyle);
}

FieldDecl *FieldDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) FieldDecl(Field, nullptr, SourceLocation(),
                               SourceLocation(), nullptr, QualType(), nullptr,
                               nullptr, false, ICIS_NoInit);
}

bool FieldDecl::isAnonymousStructOrUnion() const {
  if (!isImplicit() || getDeclName())
    return false;

  if (const RecordType *Record = getType()->getAs<RecordType>())
    return Record->getDecl()->isAnonymousStructOrUnion();

  return false;
}

unsigned FieldDecl::getBitWidthValue(const ASTContext &Ctx) const {
  assert(isBitField() && "not a bitfield");
  Expr *BitWidth = static_cast<Expr *>(InitStorage.getPointer());
  return BitWidth->EvaluateKnownConstInt(Ctx).getZExtValue();
}

unsigned FieldDecl::getFieldIndex() const {
  const FieldDecl *Canonical = getCanonicalDecl();
  if (Canonical != this)
    return Canonical->getFieldIndex();

  if (CachedFieldIndex) return CachedFieldIndex - 1;

  unsigned Index = 0;
  const RecordDecl *RD = getParent();

  for (auto *Field : RD->fields()) {
    Field->getCanonicalDecl()->CachedFieldIndex = Index + 1;
    ++Index;
  }

  assert(CachedFieldIndex && "failed to find field in parent");
  return CachedFieldIndex - 1;
}

SourceRange FieldDecl::getSourceRange() const {
  switch (InitStorage.getInt()) {
  // All three of these cases store an optional Expr*.
  case ISK_BitWidthOrNothing:
  case ISK_InClassCopyInit:
  case ISK_InClassListInit:
    if (const Expr *E = static_cast<const Expr *>(InitStorage.getPointer()))
      return SourceRange(getInnerLocStart(), E->getLocEnd());
    LLVM_FALLTHROUGH; // HLSL Change

  case ISK_CapturedVLAType:
    return DeclaratorDecl::getSourceRange();
  }
  llvm_unreachable("bad init storage kind");
}

void FieldDecl::setCapturedVLAType(const VariableArrayType *VLAType) {
  assert((getParent()->isLambda() || getParent()->isCapturedRecord()) &&
         "capturing type in non-lambda or captured record.");
  assert(InitStorage.getInt() == ISK_BitWidthOrNothing &&
         InitStorage.getPointer() == nullptr &&
         "bit width, initializer or captured type already set");
  InitStorage.setPointerAndInt(const_cast<VariableArrayType *>(VLAType),
                               ISK_CapturedVLAType);
}

//===----------------------------------------------------------------------===//
// TagDecl Implementation
//===----------------------------------------------------------------------===//

SourceLocation TagDecl::getOuterLocStart() const {
  return getTemplateOrInnerLocStart(this);
}

SourceRange TagDecl::getSourceRange() const {
  SourceLocation E = RBraceLoc.isValid() ? RBraceLoc : getLocation();
  return SourceRange(getOuterLocStart(), E);
}

TagDecl *TagDecl::getCanonicalDecl() { return getFirstDecl(); }

void TagDecl::setTypedefNameForAnonDecl(TypedefNameDecl *TDD) {
  NamedDeclOrQualifier = TDD;
  if (const Type *T = getTypeForDecl()) {
    (void)T;
    assert(T->isLinkageValid());
  }
  assert(isLinkageValid());
}

void TagDecl::startDefinition() {
  IsBeingDefined = true;

  if (CXXRecordDecl *D = dyn_cast<CXXRecordDecl>(this)) {
    struct CXXRecordDecl::DefinitionData *Data =
      new (getASTContext()) struct CXXRecordDecl::DefinitionData(D);
    for (auto I : redecls())
      cast<CXXRecordDecl>(I)->DefinitionData = Data;
  }
}

void TagDecl::completeDefinition() {
  assert((!isa<CXXRecordDecl>(this) ||
          cast<CXXRecordDecl>(this)->hasDefinition()) &&
         "definition completed but not started");

  IsCompleteDefinition = true;
  IsBeingDefined = false;

  if (ASTMutationListener *L = getASTMutationListener())
    L->CompletedTagDefinition(this);
}

TagDecl *TagDecl::getDefinition() const {
  if (isCompleteDefinition())
    return const_cast<TagDecl *>(this);

  // If it's possible for us to have an out-of-date definition, check now.
  if (MayHaveOutOfDateDef) {
    if (IdentifierInfo *II = getIdentifier()) {
      if (II->isOutOfDate()) {
        updateOutOfDate(*II);
      }
    }
  }

  if (const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(this))
    return CXXRD->getDefinition();

  for (auto R : redecls())
    if (R->isCompleteDefinition())
      return R;

  return nullptr;
}

void TagDecl::setQualifierInfo(NestedNameSpecifierLoc QualifierLoc) {
  if (QualifierLoc) {
    // Make sure the extended qualifier info is allocated.
    if (!hasExtInfo())
      NamedDeclOrQualifier = new (getASTContext()) ExtInfo;
    // Set qualifier info.
    getExtInfo()->QualifierLoc = QualifierLoc;
  } else {
    // Here Qualifier == 0, i.e., we are removing the qualifier (if any).
    if (hasExtInfo()) {
      if (getExtInfo()->NumTemplParamLists == 0) {
        getASTContext().Deallocate(getExtInfo());
        NamedDeclOrQualifier = (TypedefNameDecl*)nullptr;
      }
      else
        getExtInfo()->QualifierLoc = QualifierLoc;
    }
  }
}

void TagDecl::setTemplateParameterListsInfo(ASTContext &Context,
                                            unsigned NumTPLists,
                                            TemplateParameterList **TPLists) {
  assert(NumTPLists > 0);
  // Make sure the extended decl info is allocated.
  if (!hasExtInfo())
    // Allocate external info struct.
    NamedDeclOrQualifier = new (getASTContext()) ExtInfo;
  // Set the template parameter lists info.
  getExtInfo()->setTemplateParameterListsInfo(Context, NumTPLists, TPLists);
}

//===----------------------------------------------------------------------===//
// EnumDecl Implementation
//===----------------------------------------------------------------------===//

void EnumDecl::anchor() { }

EnumDecl *EnumDecl::Create(ASTContext &C, DeclContext *DC,
                           SourceLocation StartLoc, SourceLocation IdLoc,
                           IdentifierInfo *Id,
                           EnumDecl *PrevDecl, bool IsScoped,
                           bool IsScopedUsingClassTag, bool IsFixed) {
  EnumDecl *Enum = new (C, DC) EnumDecl(C, DC, StartLoc, IdLoc, Id, PrevDecl,
                                        IsScoped, IsScopedUsingClassTag,
                                        IsFixed);
  Enum->MayHaveOutOfDateDef = C.getLangOpts().Modules;
  C.getTypeDeclType(Enum, PrevDecl);
  return Enum;
}

EnumDecl *EnumDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  EnumDecl *Enum =
      new (C, ID) EnumDecl(C, nullptr, SourceLocation(), SourceLocation(),
                           nullptr, nullptr, false, false, false);
  Enum->MayHaveOutOfDateDef = C.getLangOpts().Modules;
  return Enum;
}

SourceRange EnumDecl::getIntegerTypeRange() const {
  if (const TypeSourceInfo *TI = getIntegerTypeSourceInfo())
    return TI->getTypeLoc().getSourceRange();
  return SourceRange();
}

void EnumDecl::completeDefinition(QualType NewType,
                                  QualType NewPromotionType,
                                  unsigned NumPositiveBits,
                                  unsigned NumNegativeBits) {
  assert(!isCompleteDefinition() && "Cannot redefine enums!");
  if (!IntegerType)
    IntegerType = NewType.getTypePtr();
  PromotionType = NewPromotionType;
  setNumPositiveBits(NumPositiveBits);
  setNumNegativeBits(NumNegativeBits);
  TagDecl::completeDefinition();
}

TemplateSpecializationKind EnumDecl::getTemplateSpecializationKind() const {
  if (MemberSpecializationInfo *MSI = getMemberSpecializationInfo())
    return MSI->getTemplateSpecializationKind();

  return TSK_Undeclared;
}

void EnumDecl::setTemplateSpecializationKind(TemplateSpecializationKind TSK,
                                         SourceLocation PointOfInstantiation) {
  MemberSpecializationInfo *MSI = getMemberSpecializationInfo();
  assert(MSI && "Not an instantiated member enumeration?");
  MSI->setTemplateSpecializationKind(TSK);
  if (TSK != TSK_ExplicitSpecialization &&
      PointOfInstantiation.isValid() &&
      MSI->getPointOfInstantiation().isInvalid())
    MSI->setPointOfInstantiation(PointOfInstantiation);
}

EnumDecl *EnumDecl::getInstantiatedFromMemberEnum() const {
  if (SpecializationInfo)
    return cast<EnumDecl>(SpecializationInfo->getInstantiatedFrom());

  return nullptr;
}

void EnumDecl::setInstantiationOfMemberEnum(ASTContext &C, EnumDecl *ED,
                                            TemplateSpecializationKind TSK) {
  assert(!SpecializationInfo && "Member enum is already a specialization");
  SpecializationInfo = new (C) MemberSpecializationInfo(ED, TSK);
}

//===----------------------------------------------------------------------===//
// RecordDecl Implementation
//===----------------------------------------------------------------------===//

RecordDecl::RecordDecl(Kind DK, TagKind TK, const ASTContext &C,
                       DeclContext *DC, SourceLocation StartLoc,
                       SourceLocation IdLoc, IdentifierInfo *Id,
                       RecordDecl *PrevDecl)
    : TagDecl(DK, TK, C, DC, IdLoc, Id, PrevDecl, StartLoc) {
  HasFlexibleArrayMember = false;
  AnonymousStructOrUnion = false;
  HasObjectMember = false;
  HasVolatileMember = false;
  LoadedFieldsFromExternalStorage = false;
  assert(classof(static_cast<Decl*>(this)) && "Invalid Kind!");
}

RecordDecl *RecordDecl::Create(const ASTContext &C, TagKind TK, DeclContext *DC,
                               SourceLocation StartLoc, SourceLocation IdLoc,
                               IdentifierInfo *Id, RecordDecl* PrevDecl) {
  RecordDecl *R = new (C, DC) RecordDecl(Record, TK, C, DC,
                                         StartLoc, IdLoc, Id, PrevDecl);
  R->MayHaveOutOfDateDef = C.getLangOpts().Modules;

  C.getTypeDeclType(R, PrevDecl);
  return R;
}

RecordDecl *RecordDecl::CreateDeserialized(const ASTContext &C, unsigned ID) {
  RecordDecl *R =
      new (C, ID) RecordDecl(Record, TTK_Struct, C, nullptr, SourceLocation(),
                             SourceLocation(), nullptr, nullptr);
  R->MayHaveOutOfDateDef = C.getLangOpts().Modules;
  return R;
}

bool RecordDecl::isInjectedClassName() const {
  return isImplicit() && getDeclName() && getDeclContext()->isRecord() &&
    cast<RecordDecl>(getDeclContext())->getDeclName() == getDeclName();
}

bool RecordDecl::isLambda() const {
  if (auto RD = dyn_cast<CXXRecordDecl>(this))
    return RD->isLambda();
  return false;
}

bool RecordDecl::isCapturedRecord() const {
  return hasAttr<CapturedRecordAttr>();
}

void RecordDecl::setCapturedRecord() {
  addAttr(CapturedRecordAttr::CreateImplicit(getASTContext()));
}

RecordDecl::field_iterator RecordDecl::field_begin() const {
  if (hasExternalLexicalStorage() && !LoadedFieldsFromExternalStorage)
    LoadFieldsFromExternalStorage();

  return field_iterator(decl_iterator(FirstDecl));
}

/// completeDefinition - Notes that the definition of this type is now
/// complete.
void RecordDecl::completeDefinition() {
  assert(!isCompleteDefinition() && "Cannot redefine record!");
  TagDecl::completeDefinition();
}

/// isMsStruct - Get whether or not this record uses ms_struct layout.
/// This which can be turned on with an attribute, pragma, or the
/// -mms-bitfields command-line option.
bool RecordDecl::isMsStruct(const ASTContext &C) const {
  return hasAttr<MSStructAttr>() || C.getLangOpts().MSBitfields == 1;
}

static bool isFieldOrIndirectField(Decl::Kind K) {
  return FieldDecl::classofKind(K) || IndirectFieldDecl::classofKind(K);
}

void RecordDecl::LoadFieldsFromExternalStorage() const {
  ExternalASTSource *Source = getASTContext().getExternalSource();
  assert(hasExternalLexicalStorage() && Source && "No external storage?");

  // Notify that we have a RecordDecl doing some initialization.
  ExternalASTSource::Deserializing TheFields(Source);

  SmallVector<Decl*, 64> Decls;
  LoadedFieldsFromExternalStorage = true;  
  switch (Source->FindExternalLexicalDecls(this, isFieldOrIndirectField,
                                           Decls)) {
  case ELR_Success:
    break;
    
  case ELR_AlreadyLoaded:
  case ELR_Failure:
    return;
  }

#ifndef NDEBUG
  // Check that all decls we got were FieldDecls.
  for (unsigned i=0, e=Decls.size(); i != e; ++i)
    assert(isa<FieldDecl>(Decls[i]) || isa<IndirectFieldDecl>(Decls[i]));
#endif

  if (Decls.empty())
    return;

  std::tie(FirstDecl, LastDecl) = BuildDeclChain(Decls,
                                                 /*FieldsAlreadyLoaded=*/false);
}

bool RecordDecl::mayInsertExtraPadding(bool EmitRemark) const {
  ASTContext &Context = getASTContext();
  if (!Context.getLangOpts().Sanitize.hasOneOf(
          SanitizerKind::Address | SanitizerKind::KernelAddress) ||
      !Context.getLangOpts().SanitizeAddressFieldPadding)
    return false;
  const auto &Blacklist = Context.getSanitizerBlacklist();
  const CXXRecordDecl *CXXRD = dyn_cast<CXXRecordDecl>(this);
  // We may be able to relax some of these requirements.
  int ReasonToReject = -1;
  if (!CXXRD || CXXRD->isExternCContext())
    ReasonToReject = 0;  // is not C++.
  else if (CXXRD->hasAttr<PackedAttr>())
    ReasonToReject = 1;  // is packed.
  else if (CXXRD->isUnion())
    ReasonToReject = 2;  // is a union.
  else if (CXXRD->isTriviallyCopyable())
    ReasonToReject = 3;  // is trivially copyable.
  else if (CXXRD->hasTrivialDestructor())
    ReasonToReject = 4;  // has trivial destructor.
  else if (CXXRD->isStandardLayout())
    ReasonToReject = 5;  // is standard layout.
  else if (Blacklist.isBlacklistedLocation(getLocation(), "field-padding"))
    ReasonToReject = 6;  // is in a blacklisted file.
  else if (Blacklist.isBlacklistedType(getQualifiedNameAsString(),
                                       "field-padding"))
    ReasonToReject = 7;  // is blacklisted.

  if (EmitRemark) {
    if (ReasonToReject >= 0)
      Context.getDiagnostics().Report(
          getLocation(),
          diag::remark_sanitize_address_insert_extra_padding_rejected)
          << getQualifiedNameAsString() << ReasonToReject;
    else
      Context.getDiagnostics().Report(
          getLocation(),
          diag::remark_sanitize_address_insert_extra_padding_accepted)
          << getQualifiedNameAsString();
  }
  return ReasonToReject < 0;
}

const FieldDecl *RecordDecl::findFirstNamedDataMember() const {
  for (const auto *I : fields()) {
    if (I->getIdentifier())
      return I;

    if (const RecordType *RT = I->getType()->getAs<RecordType>())
      if (const FieldDecl *NamedDataMember =
          RT->getDecl()->findFirstNamedDataMember())
        return NamedDataMember;
  }

  // We didn't find a named data member.
  return nullptr;
}


//===----------------------------------------------------------------------===//
// BlockDecl Implementation
//===----------------------------------------------------------------------===//

void BlockDecl::setParams(ArrayRef<ParmVarDecl *> NewParamInfo) {
  assert(!ParamInfo && "Already has param info!");

  // Zero params -> null pointer.
  if (!NewParamInfo.empty()) {
    NumParams = NewParamInfo.size();
    ParamInfo = new (getASTContext()) ParmVarDecl*[NewParamInfo.size()];
    std::copy(NewParamInfo.begin(), NewParamInfo.end(), ParamInfo);
  }
}

void BlockDecl::setCaptures(ASTContext &Context,
                            const Capture *begin,
                            const Capture *end,
                            bool capturesCXXThis) {
  CapturesCXXThis = capturesCXXThis;

  if (begin == end) {
    NumCaptures = 0;
    Captures = nullptr;
    return;
  }

  NumCaptures = end - begin;

  // Avoid new Capture[] because we don't want to provide a default
  // constructor.
  size_t allocationSize = NumCaptures * sizeof(Capture);
  void *buffer = Context.Allocate(allocationSize, /*alignment*/sizeof(void*));
  memcpy(buffer, begin, allocationSize);
  Captures = static_cast<Capture*>(buffer);
}

bool BlockDecl::capturesVariable(const VarDecl *variable) const {
  for (const auto &I : captures())
    // Only auto vars can be captured, so no redeclaration worries.
    if (I.getVariable() == variable)
      return true;

  return false;
}

SourceRange BlockDecl::getSourceRange() const {
  return SourceRange(getLocation(), Body? Body->getLocEnd() : getLocation());
}

//===----------------------------------------------------------------------===//
// Other Decl Allocation/Deallocation Method Implementations
//===----------------------------------------------------------------------===//

void TranslationUnitDecl::anchor() { }

TranslationUnitDecl *TranslationUnitDecl::Create(ASTContext &C) {
  return new (C, (DeclContext *)nullptr) TranslationUnitDecl(C);
}

void ExternCContextDecl::anchor() { }

ExternCContextDecl *ExternCContextDecl::Create(const ASTContext &C,
                                               TranslationUnitDecl *DC) {
  return new (C, DC) ExternCContextDecl(DC);
}

void LabelDecl::anchor() { }

LabelDecl *LabelDecl::Create(ASTContext &C, DeclContext *DC,
                             SourceLocation IdentL, IdentifierInfo *II) {
  return new (C, DC) LabelDecl(DC, IdentL, II, nullptr, IdentL);
}

LabelDecl *LabelDecl::Create(ASTContext &C, DeclContext *DC,
                             SourceLocation IdentL, IdentifierInfo *II,
                             SourceLocation GnuLabelL) {
  assert(GnuLabelL != IdentL && "Use this only for GNU local labels");
  return new (C, DC) LabelDecl(DC, IdentL, II, nullptr, GnuLabelL);
}

LabelDecl *LabelDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) LabelDecl(nullptr, SourceLocation(), nullptr, nullptr,
                               SourceLocation());
}

void LabelDecl::setMSAsmLabel(StringRef Name) {
  char *Buffer = new (getASTContext(), 1) char[Name.size() + 1];
  memcpy(Buffer, Name.data(), Name.size());
  Buffer[Name.size()] = '\0';
  MSAsmName = Buffer;
}

void ValueDecl::anchor() { }

bool ValueDecl::isWeak() const {
  for (const auto *I : attrs())
    if (isa<WeakAttr>(I) || isa<WeakRefAttr>(I))
      return true;

  return isWeakImported();
}

void ImplicitParamDecl::anchor() { }

ImplicitParamDecl *ImplicitParamDecl::Create(ASTContext &C, DeclContext *DC,
                                             SourceLocation IdLoc,
                                             IdentifierInfo *Id,
                                             QualType Type) {
  return new (C, DC) ImplicitParamDecl(C, DC, IdLoc, Id, Type);
}

ImplicitParamDecl *ImplicitParamDecl::CreateDeserialized(ASTContext &C,
                                                         unsigned ID) {
  return new (C, ID) ImplicitParamDecl(C, nullptr, SourceLocation(), nullptr,
                                       QualType());
}

FunctionDecl *FunctionDecl::Create(ASTContext &C, DeclContext *DC,
                                   SourceLocation StartLoc,
                                   const DeclarationNameInfo &NameInfo,
                                   QualType T, TypeSourceInfo *TInfo,
                                   StorageClass SC,
                                   bool isInlineSpecified,
                                   bool hasWrittenPrototype,
                                   bool isConstexprSpecified) {
  FunctionDecl *New =
      new (C, DC) FunctionDecl(Function, C, DC, StartLoc, NameInfo, T, TInfo,
                               SC, isInlineSpecified, isConstexprSpecified);
  New->HasWrittenPrototype = hasWrittenPrototype;
  return New;
}

FunctionDecl *FunctionDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) FunctionDecl(Function, C, nullptr, SourceLocation(),
                                  DeclarationNameInfo(), QualType(), nullptr,
                                  SC_None, false, false);
}

BlockDecl *BlockDecl::Create(ASTContext &C, DeclContext *DC, SourceLocation L) {
  return new (C, DC) BlockDecl(DC, L);
}

BlockDecl *BlockDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) BlockDecl(nullptr, SourceLocation());
}

CapturedDecl *CapturedDecl::Create(ASTContext &C, DeclContext *DC,
                                   unsigned NumParams) {
  return new (C, DC, NumParams * sizeof(ImplicitParamDecl *))
      CapturedDecl(DC, NumParams);
}

CapturedDecl *CapturedDecl::CreateDeserialized(ASTContext &C, unsigned ID,
                                               unsigned NumParams) {
  return new (C, ID, NumParams * sizeof(ImplicitParamDecl *))
      CapturedDecl(nullptr, NumParams);
}

EnumConstantDecl *EnumConstantDecl::Create(ASTContext &C, EnumDecl *CD,
                                           SourceLocation L,
                                           IdentifierInfo *Id, QualType T,
                                           Expr *E, const llvm::APSInt &V) {
  return new (C, CD) EnumConstantDecl(CD, L, Id, T, E, V);
}

EnumConstantDecl *
EnumConstantDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) EnumConstantDecl(nullptr, SourceLocation(), nullptr,
                                      QualType(), nullptr, llvm::APSInt());
}

void IndirectFieldDecl::anchor() { }

IndirectFieldDecl *
IndirectFieldDecl::Create(ASTContext &C, DeclContext *DC, SourceLocation L,
                          IdentifierInfo *Id, QualType T, NamedDecl **CH,
                          unsigned CHS) {
  return new (C, DC) IndirectFieldDecl(DC, L, Id, T, CH, CHS);
}

IndirectFieldDecl *IndirectFieldDecl::CreateDeserialized(ASTContext &C,
                                                         unsigned ID) {
  return new (C, ID) IndirectFieldDecl(nullptr, SourceLocation(),
                                       DeclarationName(), QualType(), nullptr,
                                       0);
}

SourceRange EnumConstantDecl::getSourceRange() const {
  SourceLocation End = getLocation();
  if (Init)
    End = Init->getLocEnd();
  return SourceRange(getLocation(), End);
}

void TypeDecl::anchor() { }

TypedefDecl *TypedefDecl::Create(ASTContext &C, DeclContext *DC,
                                 SourceLocation StartLoc, SourceLocation IdLoc,
                                 IdentifierInfo *Id, TypeSourceInfo *TInfo) {
  return new (C, DC) TypedefDecl(C, DC, StartLoc, IdLoc, Id, TInfo);
}

void TypedefNameDecl::anchor() { }

TagDecl *TypedefNameDecl::getAnonDeclWithTypedefName(bool AnyRedecl) const {
  if (auto *TT = getTypeSourceInfo()->getType()->getAs<TagType>()) {
    auto *OwningTypedef = TT->getDecl()->getTypedefNameForAnonDecl();
    auto *ThisTypedef = this;
    if (AnyRedecl && OwningTypedef) {
      OwningTypedef = OwningTypedef->getCanonicalDecl();
      ThisTypedef = ThisTypedef->getCanonicalDecl();
    }
    if (OwningTypedef == ThisTypedef)
      return TT->getDecl();
  }

  return nullptr;
}

TypedefDecl *TypedefDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) TypedefDecl(C, nullptr, SourceLocation(), SourceLocation(),
                                 nullptr, nullptr);
}

TypeAliasDecl *TypeAliasDecl::Create(ASTContext &C, DeclContext *DC,
                                     SourceLocation StartLoc,
                                     SourceLocation IdLoc, IdentifierInfo *Id,
                                     TypeSourceInfo *TInfo) {
  return new (C, DC) TypeAliasDecl(C, DC, StartLoc, IdLoc, Id, TInfo);
}

TypeAliasDecl *TypeAliasDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) TypeAliasDecl(C, nullptr, SourceLocation(),
                                   SourceLocation(), nullptr, nullptr);
}

SourceRange TypedefDecl::getSourceRange() const {
  SourceLocation RangeEnd = getLocation();
  if (TypeSourceInfo *TInfo = getTypeSourceInfo()) {
    if (typeIsPostfix(TInfo->getType()))
      RangeEnd = TInfo->getTypeLoc().getSourceRange().getEnd();
  }
  return SourceRange(getLocStart(), RangeEnd);
}

SourceRange TypeAliasDecl::getSourceRange() const {
  SourceLocation RangeEnd = getLocStart();
  if (TypeSourceInfo *TInfo = getTypeSourceInfo())
    RangeEnd = TInfo->getTypeLoc().getSourceRange().getEnd();
  return SourceRange(getLocStart(), RangeEnd);
}

void FileScopeAsmDecl::anchor() { }

FileScopeAsmDecl *FileScopeAsmDecl::Create(ASTContext &C, DeclContext *DC,
                                           StringLiteral *Str,
                                           SourceLocation AsmLoc,
                                           SourceLocation RParenLoc) {
  return new (C, DC) FileScopeAsmDecl(DC, Str, AsmLoc, RParenLoc);
}

FileScopeAsmDecl *FileScopeAsmDecl::CreateDeserialized(ASTContext &C,
                                                       unsigned ID) {
  return new (C, ID) FileScopeAsmDecl(nullptr, nullptr, SourceLocation(),
                                      SourceLocation());
}

void EmptyDecl::anchor() {}

EmptyDecl *EmptyDecl::Create(ASTContext &C, DeclContext *DC, SourceLocation L) {
  return new (C, DC) EmptyDecl(DC, L);
}

EmptyDecl *EmptyDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) EmptyDecl(nullptr, SourceLocation());
}

//===----------------------------------------------------------------------===//
// ImportDecl Implementation
//===----------------------------------------------------------------------===//

/// \brief Retrieve the number of module identifiers needed to name the given
/// module.
static unsigned getNumModuleIdentifiers(Module *Mod) {
  unsigned Result = 1;
  while (Mod->Parent) {
    Mod = Mod->Parent;
    ++Result;
  }
  return Result;
}

ImportDecl::ImportDecl(DeclContext *DC, SourceLocation StartLoc, 
                       Module *Imported,
                       ArrayRef<SourceLocation> IdentifierLocs)
  : Decl(Import, DC, StartLoc), ImportedAndComplete(Imported, true),
    NextLocalImport()
{
  assert(getNumModuleIdentifiers(Imported) == IdentifierLocs.size());
  SourceLocation *StoredLocs = reinterpret_cast<SourceLocation *>(this + 1);
  memcpy(StoredLocs, IdentifierLocs.data(), 
         IdentifierLocs.size() * sizeof(SourceLocation));
}

ImportDecl::ImportDecl(DeclContext *DC, SourceLocation StartLoc, 
                       Module *Imported, SourceLocation EndLoc)
  : Decl(Import, DC, StartLoc), ImportedAndComplete(Imported, false),
    NextLocalImport()
{
  *reinterpret_cast<SourceLocation *>(this + 1) = EndLoc;
}

ImportDecl *ImportDecl::Create(ASTContext &C, DeclContext *DC,
                               SourceLocation StartLoc, Module *Imported,
                               ArrayRef<SourceLocation> IdentifierLocs) {
  return new (C, DC, IdentifierLocs.size() * sizeof(SourceLocation))
      ImportDecl(DC, StartLoc, Imported, IdentifierLocs);
}

ImportDecl *ImportDecl::CreateImplicit(ASTContext &C, DeclContext *DC,
                                       SourceLocation StartLoc,
                                       Module *Imported,
                                       SourceLocation EndLoc) {
  ImportDecl *Import =
      new (C, DC, sizeof(SourceLocation)) ImportDecl(DC, StartLoc,
                                                     Imported, EndLoc);
  Import->setImplicit();
  return Import;
}

ImportDecl *ImportDecl::CreateDeserialized(ASTContext &C, unsigned ID,
                                           unsigned NumLocations) {
  return new (C, ID, NumLocations * sizeof(SourceLocation))
      ImportDecl(EmptyShell());
}

ArrayRef<SourceLocation> ImportDecl::getIdentifierLocs() const {
  if (!ImportedAndComplete.getInt())
    return None;

  const SourceLocation *StoredLocs
    = reinterpret_cast<const SourceLocation *>(this + 1);
  return llvm::makeArrayRef(StoredLocs,
                            getNumModuleIdentifiers(getImportedModule()));
}

SourceRange ImportDecl::getSourceRange() const {
  if (!ImportedAndComplete.getInt())
    return SourceRange(getLocation(), 
                       *reinterpret_cast<const SourceLocation *>(this + 1));
  
  return SourceRange(getLocation(), getIdentifierLocs().back());
}
