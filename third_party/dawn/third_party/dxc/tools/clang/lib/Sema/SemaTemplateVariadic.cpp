//===------- SemaTemplateVariadic.cpp - C++ Variadic Templates ------------===/
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===/
//
//  This file implements semantic analysis for C++0x variadic templates.
//===----------------------------------------------------------------------===/

#include "clang/Sema/Sema.h"
#include "TypeLocBuilder.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/SemaInternal.h"
#include "clang/Sema/Template.h"

using namespace clang;

//----------------------------------------------------------------------------
// Visitor that collects unexpanded parameter packs
//----------------------------------------------------------------------------

namespace {
  /// \brief A class that collects unexpanded parameter packs.
  class CollectUnexpandedParameterPacksVisitor :
    public RecursiveASTVisitor<CollectUnexpandedParameterPacksVisitor> 
  {
    typedef RecursiveASTVisitor<CollectUnexpandedParameterPacksVisitor>
      inherited;

    SmallVectorImpl<UnexpandedParameterPack> &Unexpanded;

    bool InLambda;

  public:
    explicit CollectUnexpandedParameterPacksVisitor(
                  SmallVectorImpl<UnexpandedParameterPack> &Unexpanded)
      : Unexpanded(Unexpanded), InLambda(false) { }

    bool shouldWalkTypesOfTypeLocs() const { return false; }
    
    //------------------------------------------------------------------------
    // Recording occurrences of (unexpanded) parameter packs.
    //------------------------------------------------------------------------

    /// \brief Record occurrences of template type parameter packs.
    bool VisitTemplateTypeParmTypeLoc(TemplateTypeParmTypeLoc TL) {
      if (TL.getTypePtr()->isParameterPack())
        Unexpanded.push_back(std::make_pair(TL.getTypePtr(), TL.getNameLoc()));
      return true;
    }

    /// \brief Record occurrences of template type parameter packs
    /// when we don't have proper source-location information for
    /// them.
    ///
    /// Ideally, this routine would never be used.
    bool VisitTemplateTypeParmType(TemplateTypeParmType *T) {
      if (T->isParameterPack())
        Unexpanded.push_back(std::make_pair(T, SourceLocation()));

      return true;
    }

    /// \brief Record occurrences of function and non-type template
    /// parameter packs in an expression.
    bool VisitDeclRefExpr(DeclRefExpr *E) {
      if (E->getDecl()->isParameterPack())
        Unexpanded.push_back(std::make_pair(E->getDecl(), E->getLocation()));
      
      return true;
    }
    
    /// \brief Record occurrences of template template parameter packs.
    bool TraverseTemplateName(TemplateName Template) {
      if (TemplateTemplateParmDecl *TTP 
            = dyn_cast_or_null<TemplateTemplateParmDecl>(
                                                  Template.getAsTemplateDecl()))
        if (TTP->isParameterPack())
          Unexpanded.push_back(std::make_pair(TTP, SourceLocation()));
      
      return inherited::TraverseTemplateName(Template);
    }

    /// \brief Suppress traversal into Objective-C container literal
    /// elements that are pack expansions.
    bool TraverseObjCDictionaryLiteral(ObjCDictionaryLiteral *E) {
      if (!E->containsUnexpandedParameterPack())
        return true;

      for (unsigned I = 0, N = E->getNumElements(); I != N; ++I) {
        ObjCDictionaryElement Element = E->getKeyValueElement(I);
        if (Element.isPackExpansion())
          continue;

        TraverseStmt(Element.Key);
        TraverseStmt(Element.Value);
      }
      return true;
    }
    //------------------------------------------------------------------------
    // Pruning the search for unexpanded parameter packs.
    //------------------------------------------------------------------------

    /// \brief Suppress traversal into statements and expressions that
    /// do not contain unexpanded parameter packs.
    bool TraverseStmt(Stmt *S) { 
      Expr *E = dyn_cast_or_null<Expr>(S);
      if ((E && E->containsUnexpandedParameterPack()) || InLambda)
        return inherited::TraverseStmt(S);

      return true;
    }

    /// \brief Suppress traversal into types that do not contain
    /// unexpanded parameter packs.
    bool TraverseType(QualType T) {
      if ((!T.isNull() && T->containsUnexpandedParameterPack()) || InLambda)
        return inherited::TraverseType(T);

      return true;
    }

    /// \brief Suppress traversel into types with location information
    /// that do not contain unexpanded parameter packs.
    bool TraverseTypeLoc(TypeLoc TL) {
      if ((!TL.getType().isNull() && 
           TL.getType()->containsUnexpandedParameterPack()) ||
          InLambda)
        return inherited::TraverseTypeLoc(TL);

      return true;
    }

    /// \brief Suppress traversal of non-parameter declarations, since
    /// they cannot contain unexpanded parameter packs.
    bool TraverseDecl(Decl *D) { 
      if ((D && isa<ParmVarDecl>(D)) || InLambda)
        return inherited::TraverseDecl(D);

      return true;
    }

    /// \brief Suppress traversal of template argument pack expansions.
    bool TraverseTemplateArgument(const TemplateArgument &Arg) {
      if (Arg.isPackExpansion())
        return true;

      return inherited::TraverseTemplateArgument(Arg);
    }

    /// \brief Suppress traversal of template argument pack expansions.
    bool TraverseTemplateArgumentLoc(const TemplateArgumentLoc &ArgLoc) {
      if (ArgLoc.getArgument().isPackExpansion())
        return true;
      
      return inherited::TraverseTemplateArgumentLoc(ArgLoc);
    }

    /// \brief Note whether we're traversing a lambda containing an unexpanded
    /// parameter pack. In this case, the unexpanded pack can occur anywhere,
    /// including all the places where we normally wouldn't look. Within a
    /// lambda, we don't propagate the 'contains unexpanded parameter pack' bit
    /// outside an expression.
    bool TraverseLambdaExpr(LambdaExpr *Lambda) {
      // The ContainsUnexpandedParameterPack bit on a lambda is always correct,
      // even if it's contained within another lambda.
      if (!Lambda->containsUnexpandedParameterPack())
        return true;

      bool WasInLambda = InLambda;
      InLambda = true;

      // If any capture names a function parameter pack, that pack is expanded
      // when the lambda is expanded.
      for (LambdaExpr::capture_iterator I = Lambda->capture_begin(),
                                        E = Lambda->capture_end();
           I != E; ++I) {
        if (I->capturesVariable()) {
          VarDecl *VD = I->getCapturedVar();
          if (VD->isParameterPack())
            Unexpanded.push_back(std::make_pair(VD, I->getLocation()));
        }
      }

      inherited::TraverseLambdaExpr(Lambda);

      InLambda = WasInLambda;
      return true;
    }
  };
}

/// \brief Determine whether it's possible for an unexpanded parameter pack to
/// be valid in this location. This only happens when we're in a declaration
/// that is nested within an expression that could be expanded, such as a
/// lambda-expression within a function call.
///
/// This is conservatively correct, but may claim that some unexpanded packs are
/// permitted when they are not.
bool Sema::isUnexpandedParameterPackPermitted() {
  for (auto *SI : FunctionScopes)
    if (isa<sema::LambdaScopeInfo>(SI))
      return true;
  return false;
}

/// \brief Diagnose all of the unexpanded parameter packs in the given
/// vector.
bool
Sema::DiagnoseUnexpandedParameterPacks(SourceLocation Loc,
                                       UnexpandedParameterPackContext UPPC,
                                 ArrayRef<UnexpandedParameterPack> Unexpanded) {
  if (Unexpanded.empty())
    return false;

  // If we are within a lambda expression, that lambda contains an unexpanded
  // parameter pack, and we are done.
  // FIXME: Store 'Unexpanded' on the lambda so we don't need to recompute it
  // later.
  for (unsigned N = FunctionScopes.size(); N; --N) {
    if (sema::LambdaScopeInfo *LSI =
          dyn_cast<sema::LambdaScopeInfo>(FunctionScopes[N-1])) {
      LSI->ContainsUnexpandedParameterPack = true;
      return false;
    }
  }
  
  SmallVector<SourceLocation, 4> Locations;
  SmallVector<IdentifierInfo *, 4> Names;
  llvm::SmallPtrSet<IdentifierInfo *, 4> NamesKnown;

  for (unsigned I = 0, N = Unexpanded.size(); I != N; ++I) {
    IdentifierInfo *Name = nullptr;
    if (const TemplateTypeParmType *TTP
          = Unexpanded[I].first.dyn_cast<const TemplateTypeParmType *>())
      Name = TTP->getIdentifier();
    else
      Name = Unexpanded[I].first.get<NamedDecl *>()->getIdentifier();

    if (Name && NamesKnown.insert(Name).second)
      Names.push_back(Name);

    if (Unexpanded[I].second.isValid())
      Locations.push_back(Unexpanded[I].second);
  }

  DiagnosticBuilder DB = Diag(Loc, diag::err_unexpanded_parameter_pack)
                         << (int)UPPC << (int)Names.size();
  for (size_t I = 0, E = std::min(Names.size(), (size_t)2); I != E; ++I)
    DB << Names[I];

  for (unsigned I = 0, N = Locations.size(); I != N; ++I)
    DB << SourceRange(Locations[I]);
  return true;
}

bool Sema::DiagnoseUnexpandedParameterPack(SourceLocation Loc, 
                                           TypeSourceInfo *T,
                                         UnexpandedParameterPackContext UPPC) {
  // C++0x [temp.variadic]p5:
  //   An appearance of a name of a parameter pack that is not expanded is 
  //   ill-formed.
  if (!T->getType()->containsUnexpandedParameterPack())
    return false;

  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded).TraverseTypeLoc(
                                                              T->getTypeLoc());
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(Loc, UPPC, Unexpanded);
}

bool Sema::DiagnoseUnexpandedParameterPack(Expr *E,
                                        UnexpandedParameterPackContext UPPC) {
  // C++0x [temp.variadic]p5:
  //   An appearance of a name of a parameter pack that is not expanded is 
  //   ill-formed.
  if (!E->containsUnexpandedParameterPack())
    return false;

  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded).TraverseStmt(E);
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(E->getLocStart(), UPPC, Unexpanded);
}

bool Sema::DiagnoseUnexpandedParameterPack(const CXXScopeSpec &SS,
                                        UnexpandedParameterPackContext UPPC) {
  // C++0x [temp.variadic]p5:
  //   An appearance of a name of a parameter pack that is not expanded is 
  //   ill-formed.
  if (!SS.getScopeRep() || 
      !SS.getScopeRep()->containsUnexpandedParameterPack())
    return false;

  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseNestedNameSpecifier(SS.getScopeRep());
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(SS.getRange().getBegin(),
                                          UPPC, Unexpanded);
}

bool Sema::DiagnoseUnexpandedParameterPack(const DeclarationNameInfo &NameInfo,
                                         UnexpandedParameterPackContext UPPC) {
  // C++0x [temp.variadic]p5:
  //   An appearance of a name of a parameter pack that is not expanded is 
  //   ill-formed.
  switch (NameInfo.getName().getNameKind()) {
  case DeclarationName::Identifier:
  case DeclarationName::ObjCZeroArgSelector:
  case DeclarationName::ObjCOneArgSelector:
  case DeclarationName::ObjCMultiArgSelector:
  case DeclarationName::CXXOperatorName:
  case DeclarationName::CXXLiteralOperatorName:
  case DeclarationName::CXXUsingDirective:
    return false;

  case DeclarationName::CXXConstructorName:
  case DeclarationName::CXXDestructorName:
  case DeclarationName::CXXConversionFunctionName:
    // FIXME: We shouldn't need this null check!
    if (TypeSourceInfo *TSInfo = NameInfo.getNamedTypeInfo())
      return DiagnoseUnexpandedParameterPack(NameInfo.getLoc(), TSInfo, UPPC);

    if (!NameInfo.getName().getCXXNameType()->containsUnexpandedParameterPack())
      return false;

    break;
  }

  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseType(NameInfo.getName().getCXXNameType());
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(NameInfo.getLoc(), UPPC, Unexpanded);
}

bool Sema::DiagnoseUnexpandedParameterPack(SourceLocation Loc,
                                           TemplateName Template,
                                       UnexpandedParameterPackContext UPPC) {
  
  if (Template.isNull() || !Template.containsUnexpandedParameterPack())
    return false;

  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseTemplateName(Template);
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(Loc, UPPC, Unexpanded);
}

bool Sema::DiagnoseUnexpandedParameterPack(TemplateArgumentLoc Arg,
                                         UnexpandedParameterPackContext UPPC) {
  if (Arg.getArgument().isNull() || 
      !Arg.getArgument().containsUnexpandedParameterPack())
    return false;
  
  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseTemplateArgumentLoc(Arg);
  assert(!Unexpanded.empty() && "Unable to find unexpanded parameter packs");
  return DiagnoseUnexpandedParameterPacks(Arg.getLocation(), UPPC, Unexpanded);
}

void Sema::collectUnexpandedParameterPacks(TemplateArgument Arg,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseTemplateArgument(Arg);
}

void Sema::collectUnexpandedParameterPacks(TemplateArgumentLoc Arg,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseTemplateArgumentLoc(Arg);
}

void Sema::collectUnexpandedParameterPacks(QualType T,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  CollectUnexpandedParameterPacksVisitor(Unexpanded).TraverseType(T);  
}  

void Sema::collectUnexpandedParameterPacks(TypeLoc TL,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  CollectUnexpandedParameterPacksVisitor(Unexpanded).TraverseTypeLoc(TL);  
}  

void Sema::collectUnexpandedParameterPacks(CXXScopeSpec &SS,
                                           SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  NestedNameSpecifier *Qualifier = SS.getScopeRep();
  if (!Qualifier)
    return;
  
  NestedNameSpecifierLoc QualifierLoc(Qualifier, SS.location_data());
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseNestedNameSpecifierLoc(QualifierLoc);
}

void Sema::collectUnexpandedParameterPacks(const DeclarationNameInfo &NameInfo,
                         SmallVectorImpl<UnexpandedParameterPack> &Unexpanded) {
  CollectUnexpandedParameterPacksVisitor(Unexpanded)
    .TraverseDeclarationNameInfo(NameInfo);
}


ParsedTemplateArgument 
Sema::ActOnPackExpansion(const ParsedTemplateArgument &Arg,
                         SourceLocation EllipsisLoc) {
  if (Arg.isInvalid())
    return Arg;

  switch (Arg.getKind()) {
  case ParsedTemplateArgument::Type: {
    TypeResult Result = ActOnPackExpansion(Arg.getAsType(), EllipsisLoc);
    if (Result.isInvalid())
      return ParsedTemplateArgument();

    return ParsedTemplateArgument(Arg.getKind(), Result.get().getAsOpaquePtr(), 
                                  Arg.getLocation());
  }

  case ParsedTemplateArgument::NonType: {
    ExprResult Result = ActOnPackExpansion(Arg.getAsExpr(), EllipsisLoc);
    if (Result.isInvalid())
      return ParsedTemplateArgument();
    
    return ParsedTemplateArgument(Arg.getKind(), Result.get(), 
                                  Arg.getLocation());
  }
    
  case ParsedTemplateArgument::Template:
    if (!Arg.getAsTemplate().get().containsUnexpandedParameterPack()) {
      SourceRange R(Arg.getLocation());
      if (Arg.getScopeSpec().isValid())
        R.setBegin(Arg.getScopeSpec().getBeginLoc());
      Diag(EllipsisLoc, diag::err_pack_expansion_without_parameter_packs)
        << R;
      return ParsedTemplateArgument();
    }
      
    return Arg.getTemplatePackExpansion(EllipsisLoc);
  }
  llvm_unreachable("Unhandled template argument kind?");
}

TypeResult Sema::ActOnPackExpansion(ParsedType Type, 
                                    SourceLocation EllipsisLoc) {
  TypeSourceInfo *TSInfo;
  GetTypeFromParser(Type, &TSInfo);
  if (!TSInfo)
    return true;

  TypeSourceInfo *TSResult = CheckPackExpansion(TSInfo, EllipsisLoc, None);
  if (!TSResult)
    return true;
  
  return CreateParsedType(TSResult->getType(), TSResult);
}

TypeSourceInfo *
Sema::CheckPackExpansion(TypeSourceInfo *Pattern, SourceLocation EllipsisLoc,
                         Optional<unsigned> NumExpansions) {
  // Create the pack expansion type and source-location information.
  QualType Result = CheckPackExpansion(Pattern->getType(), 
                                       Pattern->getTypeLoc().getSourceRange(),
                                       EllipsisLoc, NumExpansions);
  if (Result.isNull())
    return nullptr;

  TypeLocBuilder TLB;
  TLB.pushFullCopy(Pattern->getTypeLoc());
  PackExpansionTypeLoc TL = TLB.push<PackExpansionTypeLoc>(Result);
  TL.setEllipsisLoc(EllipsisLoc);

  return TLB.getTypeSourceInfo(Context, Result);
}

QualType Sema::CheckPackExpansion(QualType Pattern, SourceRange PatternRange,
                                  SourceLocation EllipsisLoc,
                                  Optional<unsigned> NumExpansions) {
  // C++0x [temp.variadic]p5:
  //   The pattern of a pack expansion shall name one or more
  //   parameter packs that are not expanded by a nested pack
  //   expansion.
  if (!Pattern->containsUnexpandedParameterPack()) {
    Diag(EllipsisLoc, diag::err_pack_expansion_without_parameter_packs)
      << PatternRange;
    return QualType();
  }

  return Context.getPackExpansionType(Pattern, NumExpansions);
}

ExprResult Sema::ActOnPackExpansion(Expr *Pattern, SourceLocation EllipsisLoc) {
  return CheckPackExpansion(Pattern, EllipsisLoc, None);
}

ExprResult Sema::CheckPackExpansion(Expr *Pattern, SourceLocation EllipsisLoc,
                                    Optional<unsigned> NumExpansions) {
  if (!Pattern)
    return ExprError();
  
  // C++0x [temp.variadic]p5:
  //   The pattern of a pack expansion shall name one or more
  //   parameter packs that are not expanded by a nested pack
  //   expansion.
  if (!Pattern->containsUnexpandedParameterPack()) {
    Diag(EllipsisLoc, diag::err_pack_expansion_without_parameter_packs)
    << Pattern->getSourceRange();
    return ExprError();
  }
  
  // Create the pack expansion expression and source-location information.
  return new (Context)
    PackExpansionExpr(Context.DependentTy, Pattern, EllipsisLoc, NumExpansions);
}

/// \brief Retrieve the depth and index of a parameter pack.
static std::pair<unsigned, unsigned> 
getDepthAndIndex(NamedDecl *ND) {
  if (TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(ND))
    return std::make_pair(TTP->getDepth(), TTP->getIndex());
  
  if (NonTypeTemplateParmDecl *NTTP = dyn_cast<NonTypeTemplateParmDecl>(ND))
    return std::make_pair(NTTP->getDepth(), NTTP->getIndex());
  
  TemplateTemplateParmDecl *TTP = cast<TemplateTemplateParmDecl>(ND);
  return std::make_pair(TTP->getDepth(), TTP->getIndex());
}

bool Sema::CheckParameterPacksForExpansion(
    SourceLocation EllipsisLoc, SourceRange PatternRange,
    ArrayRef<UnexpandedParameterPack> Unexpanded,
    const MultiLevelTemplateArgumentList &TemplateArgs, bool &ShouldExpand,
    bool &RetainExpansion, Optional<unsigned> &NumExpansions) {
  ShouldExpand = true;
  RetainExpansion = false;
  std::pair<IdentifierInfo *, SourceLocation> FirstPack;
  bool HaveFirstPack = false;

  if (getLangOpts().HLSL) {
    Diag(EllipsisLoc, diag::err_hlsl_variadic_templates);
    return true;
  }

  for (ArrayRef<UnexpandedParameterPack>::iterator i = Unexpanded.begin(),
                                                 end = Unexpanded.end();
                                                  i != end; ++i) {
    // Compute the depth and index for this parameter pack.
    unsigned Depth = 0, Index = 0;
    IdentifierInfo *Name;
    bool IsFunctionParameterPack = false;
    
    if (const TemplateTypeParmType *TTP
        = i->first.dyn_cast<const TemplateTypeParmType *>()) {
      Depth = TTP->getDepth();
      Index = TTP->getIndex();
      Name = TTP->getIdentifier();
    } else {
      NamedDecl *ND = i->first.get<NamedDecl *>();
      if (isa<ParmVarDecl>(ND))
        IsFunctionParameterPack = true;
      else
        std::tie(Depth, Index) = getDepthAndIndex(ND);

      Name = ND->getIdentifier();
    }
    
    // Determine the size of this argument pack.
    unsigned NewPackSize;    
    if (IsFunctionParameterPack) {
      // Figure out whether we're instantiating to an argument pack or not.
      typedef LocalInstantiationScope::DeclArgumentPack DeclArgumentPack;
      
      llvm::PointerUnion<Decl *, DeclArgumentPack *> *Instantiation
        = CurrentInstantiationScope->findInstantiationOf(
                                        i->first.get<NamedDecl *>());
      if (Instantiation->is<DeclArgumentPack *>()) {
        // We could expand this function parameter pack.
        NewPackSize = Instantiation->get<DeclArgumentPack *>()->size();
      } else {
        // We can't expand this function parameter pack, so we can't expand
        // the pack expansion.
        ShouldExpand = false;
        continue;
      }
    } else {
      // If we don't have a template argument at this depth/index, then we 
      // cannot expand the pack expansion. Make a note of this, but we still 
      // want to check any parameter packs we *do* have arguments for.
      if (Depth >= TemplateArgs.getNumLevels() ||
          !TemplateArgs.hasTemplateArgument(Depth, Index)) {
        ShouldExpand = false;
        continue;
      }
      
      // Determine the size of the argument pack.
      NewPackSize = TemplateArgs(Depth, Index).pack_size();
    }
    
    // C++0x [temp.arg.explicit]p9:
    //   Template argument deduction can extend the sequence of template 
    //   arguments corresponding to a template parameter pack, even when the
    //   sequence contains explicitly specified template arguments.
    if (!IsFunctionParameterPack) {
      if (NamedDecl *PartialPack 
                    = CurrentInstantiationScope->getPartiallySubstitutedPack()){
        unsigned PartialDepth, PartialIndex;
        std::tie(PartialDepth, PartialIndex) = getDepthAndIndex(PartialPack);
        if (PartialDepth == Depth && PartialIndex == Index)
          RetainExpansion = true;
      }
    }
    
    if (!NumExpansions) {
      // The is the first pack we've seen for which we have an argument. 
      // Record it.
      NumExpansions = NewPackSize;
      FirstPack.first = Name;
      FirstPack.second = i->second;
      HaveFirstPack = true;
      continue;
    }
    
    if (NewPackSize != *NumExpansions) {
      // C++0x [temp.variadic]p5:
      //   All of the parameter packs expanded by a pack expansion shall have 
      //   the same number of arguments specified.
      if (HaveFirstPack)
        Diag(EllipsisLoc, diag::err_pack_expansion_length_conflict)
          << FirstPack.first << Name << *NumExpansions << NewPackSize
          << SourceRange(FirstPack.second) << SourceRange(i->second);
      else
        Diag(EllipsisLoc, diag::err_pack_expansion_length_conflict_multilevel)
          << Name << *NumExpansions << NewPackSize
          << SourceRange(i->second);
      return true;
    }
  }
  
  return false;
}

Optional<unsigned> Sema::getNumArgumentsInExpansion(QualType T,
                          const MultiLevelTemplateArgumentList &TemplateArgs) {
  QualType Pattern = cast<PackExpansionType>(T)->getPattern();
  SmallVector<UnexpandedParameterPack, 2> Unexpanded;
  CollectUnexpandedParameterPacksVisitor(Unexpanded).TraverseType(Pattern);

  Optional<unsigned> Result;
  for (unsigned I = 0, N = Unexpanded.size(); I != N; ++I) {
    // Compute the depth and index for this parameter pack.
    unsigned Depth;
    unsigned Index;
    
    if (const TemplateTypeParmType *TTP
          = Unexpanded[I].first.dyn_cast<const TemplateTypeParmType *>()) {
      Depth = TTP->getDepth();
      Index = TTP->getIndex();
    } else {      
      NamedDecl *ND = Unexpanded[I].first.get<NamedDecl *>();
      if (isa<ParmVarDecl>(ND)) {
        // Function parameter pack.
        typedef LocalInstantiationScope::DeclArgumentPack DeclArgumentPack;
        
        llvm::PointerUnion<Decl *, DeclArgumentPack *> *Instantiation
          = CurrentInstantiationScope->findInstantiationOf(
                                        Unexpanded[I].first.get<NamedDecl *>());
        if (Instantiation->is<Decl*>())
          // The pattern refers to an unexpanded pack. We're not ready to expand
          // this pack yet.
          return None;

        unsigned Size = Instantiation->get<DeclArgumentPack *>()->size();
        assert((!Result || *Result == Size) && "inconsistent pack sizes");
        Result = Size;
        continue;
      }

      std::tie(Depth, Index) = getDepthAndIndex(ND);
    }
    if (Depth >= TemplateArgs.getNumLevels() ||
        !TemplateArgs.hasTemplateArgument(Depth, Index))
      // The pattern refers to an unknown template argument. We're not ready to
      // expand this pack yet.
      return None;
    
    // Determine the size of the argument pack.
    unsigned Size = TemplateArgs(Depth, Index).pack_size();
    assert((!Result || *Result == Size) && "inconsistent pack sizes");
    Result = Size;
  }
  
  return Result;
}

bool Sema::containsUnexpandedParameterPacks(Declarator &D) {
  const DeclSpec &DS = D.getDeclSpec();
  switch (DS.getTypeSpecType()) {
  case TST_typename:
  case TST_typeofType:
  case TST_underlyingType:
  case TST_atomic: {
    QualType T = DS.getRepAsType().get();
    if (!T.isNull() && T->containsUnexpandedParameterPack())
      return true;
    break;
  }
      
  case TST_typeofExpr:
  case TST_decltype:
    if (DS.getRepAsExpr() && 
        DS.getRepAsExpr()->containsUnexpandedParameterPack())
      return true;
    break;
      
  case TST_unspecified:
  case TST_void:
  case TST_char:
  case TST_wchar:
  case TST_char16:
  case TST_char32:
  case TST_int:
  case TST_int128:
  case TST_half:
  case TST_halffloat: // HLSL Change
  case TST_float:
  case TST_double:
  case TST_bool:
  case TST_decimal32:
  case TST_decimal64:
  case TST_decimal128:
  case TST_enum:
  case TST_union:
  case TST_struct:
  case TST_interface:
  case TST_class:
  case TST_auto:
  case TST_decltype_auto:
  case TST_unknown_anytype:
  case TST_error:
  // HLSL Change Start
  case TST_min16float:
  case TST_min16int:
  case TST_min16uint:
  case TST_min10float:
  case TST_min12int:
  case TST_int8_4packed:
  case TST_uint8_4packed:
  // HLSL Change End
    break;
  }

  for (unsigned I = 0, N = D.getNumTypeObjects(); I != N; ++I) {
    const DeclaratorChunk &Chunk = D.getTypeObject(I);
    switch (Chunk.Kind) {
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::Paren:
    case DeclaratorChunk::BlockPointer:
      // These declarator chunks cannot contain any parameter packs.
      break;
        
    case DeclaratorChunk::Array:
      if (Chunk.Arr.NumElts &&
          Chunk.Arr.NumElts->containsUnexpandedParameterPack())
        return true;
      break;
    case DeclaratorChunk::Function:
      for (unsigned i = 0, e = Chunk.Fun.NumParams; i != e; ++i) {
        ParmVarDecl *Param = cast<ParmVarDecl>(Chunk.Fun.Params[i].Param);
        QualType ParamTy = Param->getType();
        assert(!ParamTy.isNull() && "Couldn't parse type?");
        if (ParamTy->containsUnexpandedParameterPack()) return true;
      }

      if (Chunk.Fun.getExceptionSpecType() == EST_Dynamic) {
        for (unsigned i = 0; i != Chunk.Fun.NumExceptions; ++i) {
          if (Chunk.Fun.Exceptions[i]
                  .Ty.get()
                  ->containsUnexpandedParameterPack())
            return true;
        }
      } else if (Chunk.Fun.getExceptionSpecType() == EST_ComputedNoexcept &&
                 Chunk.Fun.NoexceptExpr->containsUnexpandedParameterPack())
        return true;

      if (Chunk.Fun.hasTrailingReturnType()) {
        QualType T = Chunk.Fun.getTrailingReturnType().get();
	if (!T.isNull() && T->containsUnexpandedParameterPack())
	  return true;
      }
      break;

    case DeclaratorChunk::MemberPointer:
      if (Chunk.Mem.Scope().getScopeRep() &&
          Chunk.Mem.Scope().getScopeRep()->containsUnexpandedParameterPack())
        return true;
      break;
    }
  }
  
  return false;
}

namespace {

// Callback to only accept typo corrections that refer to parameter packs.
class ParameterPackValidatorCCC : public CorrectionCandidateCallback {
 public:
  bool ValidateCandidate(const TypoCorrection &candidate) override {
    NamedDecl *ND = candidate.getCorrectionDecl();
    return ND && ND->isParameterPack();
  }
};

}

/// \brief Called when an expression computing the size of a parameter pack
/// is parsed.
///
/// \code
/// template<typename ...Types> struct count {
///   static const unsigned value = sizeof...(Types);
/// };
/// \endcode
///
//
/// \param OpLoc The location of the "sizeof" keyword.
/// \param Name The name of the parameter pack whose size will be determined.
/// \param NameLoc The source location of the name of the parameter pack.
/// \param RParenLoc The location of the closing parentheses.
ExprResult Sema::ActOnSizeofParameterPackExpr(Scope *S,
                                              SourceLocation OpLoc,
                                              IdentifierInfo &Name,
                                              SourceLocation NameLoc,
                                              SourceLocation RParenLoc) {
  // C++0x [expr.sizeof]p5:
  //   The identifier in a sizeof... expression shall name a parameter pack.
  LookupResult R(*this, &Name, NameLoc, LookupOrdinaryName);
  LookupName(R, S);

  NamedDecl *ParameterPack = nullptr;
  switch (R.getResultKind()) {
  case LookupResult::Found:
    ParameterPack = R.getFoundDecl();
    break;
    
  case LookupResult::NotFound:
  case LookupResult::NotFoundInCurrentInstantiation:
    if (TypoCorrection Corrected =
            CorrectTypo(R.getLookupNameInfo(), R.getLookupKind(), S, nullptr,
                        llvm::make_unique<ParameterPackValidatorCCC>(),
                        CTK_ErrorRecovery)) {
      diagnoseTypo(Corrected,
                   PDiag(diag::err_sizeof_pack_no_pack_name_suggest) << &Name,
                   PDiag(diag::note_parameter_pack_here));
      ParameterPack = Corrected.getCorrectionDecl();
    }
    break;
  case LookupResult::FoundOverloaded:
  case LookupResult::FoundUnresolvedValue:
    break;
    
  case LookupResult::Ambiguous:
    DiagnoseAmbiguousLookup(R);
    return ExprError();
  }
  
  if (!ParameterPack || !ParameterPack->isParameterPack()) {
    Diag(NameLoc, diag::err_sizeof_pack_no_pack_name)
      << &Name;
    return ExprError();
  }

  MarkAnyDeclReferenced(OpLoc, ParameterPack, true);

  return new (Context) SizeOfPackExpr(Context.getSizeType(), OpLoc, 
                                      ParameterPack, NameLoc, RParenLoc);
}

TemplateArgumentLoc
Sema::getTemplateArgumentPackExpansionPattern(
      TemplateArgumentLoc OrigLoc,
      SourceLocation &Ellipsis, Optional<unsigned> &NumExpansions) const {
  const TemplateArgument &Argument = OrigLoc.getArgument();
  assert(Argument.isPackExpansion());
  switch (Argument.getKind()) {
  case TemplateArgument::Type: {
    // FIXME: We shouldn't ever have to worry about missing
    // type-source info!
    TypeSourceInfo *ExpansionTSInfo = OrigLoc.getTypeSourceInfo();
    if (!ExpansionTSInfo)
      ExpansionTSInfo = Context.getTrivialTypeSourceInfo(Argument.getAsType(),
                                                         Ellipsis);
    PackExpansionTypeLoc Expansion =
        ExpansionTSInfo->getTypeLoc().castAs<PackExpansionTypeLoc>();
    Ellipsis = Expansion.getEllipsisLoc();

    TypeLoc Pattern = Expansion.getPatternLoc();
    NumExpansions = Expansion.getTypePtr()->getNumExpansions();

    // We need to copy the TypeLoc because TemplateArgumentLocs store a
    // TypeSourceInfo.
    // FIXME: Find some way to avoid the copy?
    TypeLocBuilder TLB;
    TLB.pushFullCopy(Pattern);
    TypeSourceInfo *PatternTSInfo =
        TLB.getTypeSourceInfo(Context, Pattern.getType());
    return TemplateArgumentLoc(TemplateArgument(Pattern.getType()),
                               PatternTSInfo);
  }

  case TemplateArgument::Expression: {
    PackExpansionExpr *Expansion
      = cast<PackExpansionExpr>(Argument.getAsExpr());
    Expr *Pattern = Expansion->getPattern();
    Ellipsis = Expansion->getEllipsisLoc();
    NumExpansions = Expansion->getNumExpansions();
    return TemplateArgumentLoc(Pattern, Pattern);
  }

  case TemplateArgument::TemplateExpansion:
    Ellipsis = OrigLoc.getTemplateEllipsisLoc();
    NumExpansions = Argument.getNumTemplateExpansions();
    return TemplateArgumentLoc(Argument.getPackExpansionPattern(),
                               OrigLoc.getTemplateQualifierLoc(),
                               OrigLoc.getTemplateNameLoc());

  case TemplateArgument::Declaration:
  case TemplateArgument::NullPtr:
  case TemplateArgument::Template:
  case TemplateArgument::Integral:
  case TemplateArgument::Pack:
  case TemplateArgument::Null:
    return TemplateArgumentLoc();
  }

  llvm_unreachable("Invalid TemplateArgument Kind!");
}

static void CheckFoldOperand(Sema &S, Expr *E) {
  if (!E)
    return;

  E = E->IgnoreImpCasts();
  if (isa<BinaryOperator>(E) || isa<AbstractConditionalOperator>(E)) {
    S.Diag(E->getExprLoc(), diag::err_fold_expression_bad_operand)
        << E->getSourceRange()
        << FixItHint::CreateInsertion(E->getLocStart(), "(")
        << FixItHint::CreateInsertion(E->getLocEnd(), ")");
  }
}

ExprResult Sema::ActOnCXXFoldExpr(SourceLocation LParenLoc, Expr *LHS,
                                  tok::TokenKind Operator,
                                  SourceLocation EllipsisLoc, Expr *RHS,
                                  SourceLocation RParenLoc) {
  // LHS and RHS must be cast-expressions. We allow an arbitrary expression
  // in the parser and reduce down to just cast-expressions here.
  CheckFoldOperand(*this, LHS);
  CheckFoldOperand(*this, RHS);

  // [expr.prim.fold]p3:
  //   In a binary fold, op1 and op2 shall be the same fold-operator, and
  //   either e1 shall contain an unexpanded parameter pack or e2 shall contain
  //   an unexpanded parameter pack, but not both.
  if (LHS && RHS &&
      LHS->containsUnexpandedParameterPack() ==
          RHS->containsUnexpandedParameterPack()) {
    return Diag(EllipsisLoc,
                LHS->containsUnexpandedParameterPack()
                    ? diag::err_fold_expression_packs_both_sides
                    : diag::err_pack_expansion_without_parameter_packs)
        << LHS->getSourceRange() << RHS->getSourceRange();
  }

  // [expr.prim.fold]p2:
  //   In a unary fold, the cast-expression shall contain an unexpanded
  //   parameter pack.
  if (!LHS || !RHS) {
    Expr *Pack = LHS ? LHS : RHS;
    assert(Pack && "fold expression with neither LHS nor RHS");
    if (!Pack->containsUnexpandedParameterPack())
      return Diag(EllipsisLoc, diag::err_pack_expansion_without_parameter_packs)
             << Pack->getSourceRange();
  }

  BinaryOperatorKind Opc = ConvertTokenKindToBinaryOpcode(Operator);
  return BuildCXXFoldExpr(LParenLoc, LHS, Opc, EllipsisLoc, RHS, RParenLoc);
}

ExprResult Sema::BuildCXXFoldExpr(SourceLocation LParenLoc, Expr *LHS,
                                  BinaryOperatorKind Operator,
                                  SourceLocation EllipsisLoc, Expr *RHS,
                                  SourceLocation RParenLoc) {
  return new (Context) CXXFoldExpr(Context.DependentTy, LParenLoc, LHS,
                                   Operator, EllipsisLoc, RHS, RParenLoc);
}

ExprResult Sema::BuildEmptyCXXFoldExpr(SourceLocation EllipsisLoc,
                                       BinaryOperatorKind Operator) {
  // [temp.variadic]p9:
  //   If N is zero for a unary fold-expression, the value of the expression is
  //       *   ->  1
  //       +   ->  int()
  //       &   ->  -1
  //       |   ->  int()
  //       &&  ->  true
  //       ||  ->  false
  //       ,   ->  void()
  //   if the operator is not listed [above], the instantiation is ill-formed.
  //
  // Note that we need to use something like int() here, not merely 0, to
  // prevent the result from being a null pointer constant.
  QualType ScalarType;
  switch (Operator) {
  case BO_Add:
    ScalarType = Context.IntTy;
    break;
  case BO_Mul:
    return ActOnIntegerConstant(EllipsisLoc, 1);
  case BO_Or:
    ScalarType = Context.IntTy;
    break;
  case BO_And:
    return CreateBuiltinUnaryOp(EllipsisLoc, UO_Minus,
                                ActOnIntegerConstant(EllipsisLoc, 1).get());
  case BO_LOr:
    return ActOnCXXBoolLiteral(EllipsisLoc, tok::kw_false);
  case BO_LAnd:
    return ActOnCXXBoolLiteral(EllipsisLoc, tok::kw_true);
  case BO_Comma:
    ScalarType = Context.VoidTy;
    break;

  default:
    return Diag(EllipsisLoc, diag::err_fold_expression_empty)
        << BinaryOperator::getOpcodeStr(Operator);
  }

  return new (Context) CXXScalarValueInitExpr(
      ScalarType, Context.getTrivialTypeSourceInfo(ScalarType, EllipsisLoc),
      EllipsisLoc);
}
