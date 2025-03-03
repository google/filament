//===--- SemaObjCProperty.cpp - Semantic Analysis for ObjC @property ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for Objective C @property and
//  @synthesize declarations.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Initialization.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallString.h"

using namespace clang;

// HLSL Change Starts
// No ObjC parse/sema support, so simply skip all of this compilation.
// Here are enough stubs to link the current targets.
#if 1

Decl *Sema::ActOnProperty(Scope *S, SourceLocation AtLoc,
  SourceLocation LParenLoc,
  FieldDeclarator &FD,
  ObjCDeclSpec &ODS,
  Selector GetterSel,
  Selector SetterSel,
  bool *isOverridingProperty,
  tok::ObjCKeywordKind MethodImplKind,
  DeclContext *lexicalDC) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}


ObjCPropertyDecl *
Sema::HandlePropertyInClassExtension(Scope *S,
  SourceLocation AtLoc,
  SourceLocation LParenLoc,
  FieldDeclarator &FD,
  Selector GetterSel, Selector SetterSel,
  const bool isAssign,
  const bool isReadWrite,
  const unsigned Attributes,
  const unsigned AttributesAsWritten,
  bool *isOverridingProperty,
  QualType T,
  TypeSourceInfo *TSI,
  tok::ObjCKeywordKind MethodImplKind) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

ObjCPropertyDecl *Sema::CreatePropertyDecl(Scope *S,
  ObjCContainerDecl *CDecl,
  SourceLocation AtLoc,
  SourceLocation LParenLoc,
  FieldDeclarator &FD,
  Selector GetterSel,
  Selector SetterSel,
  const bool isAssign,
  const bool isReadWrite,
  const unsigned Attributes,
  const unsigned AttributesAsWritten,
  QualType T,
  TypeSourceInfo *TInfo,
  tok::ObjCKeywordKind MethodImplKind,
  DeclContext *lexicalDC) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

Decl *Sema::ActOnPropertyImplDecl(Scope *S,
  SourceLocation AtLoc,
  SourceLocation PropertyLoc,
  bool Synthesize,
  IdentifierInfo *PropertyId,
  IdentifierInfo *PropertyIvar,
  SourceLocation PropertyIvarLoc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void
Sema::DiagnosePropertyMismatch(ObjCPropertyDecl *Property,
  ObjCPropertyDecl *SuperProperty,
  const IdentifierInfo *inheritedName,
  bool OverridingProtocolProperty) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool Sema::DiagnosePropertyAccessorMismatch(ObjCPropertyDecl *property,
  ObjCMethodDecl *GetterMethod,
  SourceLocation Loc) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

bool
Sema::IvarBacksCurrentMethodAccessor(ObjCInterfaceDecl *IFace,
  ObjCMethodDecl *Method, ObjCIvarDecl *IV) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::DefaultSynthesizeProperties(Scope *S, ObjCImplDecl* IMPDecl,
  ObjCInterfaceDecl *IDecl) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::DefaultSynthesizeProperties(Scope *S, Decl *D) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::DiagnoseUnimplementedProperties(Scope *S, ObjCImplDecl* IMPDecl,
  ObjCContainerDecl *CDecl,
  bool SynthesizeProperties) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::diagnoseNullResettableSynthesizedSetters(const ObjCImplDecl *impDecl) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void
Sema::AtomicPropertySetterGetterRules(ObjCImplDecl* IMPDecl,
  ObjCContainerDecl* IDecl) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::DiagnoseOwningPropertyGetterSynthesis(const ObjCImplementationDecl *D) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::DiagnoseMissingDesignatedInitOverrides(
  const ObjCImplementationDecl *ImplD,
  const ObjCInterfaceDecl *IFD) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::ProcessPropertyDecl(ObjCPropertyDecl *property,
  ObjCContainerDecl *CD,
  ObjCPropertyDecl *redeclaredProperty,
  ObjCContainerDecl *lexicalDC) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

void Sema::CheckObjCPropertyAttributes(Decl *PDecl,
  SourceLocation Loc,
  unsigned &Attributes,
  bool propertyInPrimaryClass) {
  llvm_unreachable("HLSL does not support ObjC constructs");
}

// HLSL Change Ends
#else
// HLSL Change Ends

//===----------------------------------------------------------------------===//
// Grammar actions.
//===----------------------------------------------------------------------===//

/// getImpliedARCOwnership - Given a set of property attributes and a
/// type, infer an expected lifetime.  The type's ownership qualification
/// is not considered.
///
/// Returns OCL_None if the attributes as stated do not imply an ownership.
/// Never returns OCL_Autoreleasing.
static Qualifiers::ObjCLifetime getImpliedARCOwnership(
                               ObjCPropertyDecl::PropertyAttributeKind attrs,
                                                QualType type) {
  // retain, strong, copy, weak, and unsafe_unretained are only legal
  // on properties of retainable pointer type.
  if (attrs & (ObjCPropertyDecl::OBJC_PR_retain |
               ObjCPropertyDecl::OBJC_PR_strong |
               ObjCPropertyDecl::OBJC_PR_copy)) {
    return Qualifiers::OCL_Strong;
  } else if (attrs & ObjCPropertyDecl::OBJC_PR_weak) {
    return Qualifiers::OCL_Weak;
  } else if (attrs & ObjCPropertyDecl::OBJC_PR_unsafe_unretained) {
    return Qualifiers::OCL_ExplicitNone;
  }

  // assign can appear on other types, so we have to check the
  // property type.
  if (attrs & ObjCPropertyDecl::OBJC_PR_assign &&
      type->isObjCRetainableType()) {
    return Qualifiers::OCL_ExplicitNone;
  }

  return Qualifiers::OCL_None;
}

/// Check the internal consistency of a property declaration.
static void checkARCPropertyDecl(Sema &S, ObjCPropertyDecl *property) {
  if (property->isInvalidDecl()) return;

  ObjCPropertyDecl::PropertyAttributeKind propertyKind
    = property->getPropertyAttributes();
  Qualifiers::ObjCLifetime propertyLifetime
    = property->getType().getObjCLifetime();

  // Nothing to do if we don't have a lifetime.
  if (propertyLifetime == Qualifiers::OCL_None) return;

  Qualifiers::ObjCLifetime expectedLifetime
    = getImpliedARCOwnership(propertyKind, property->getType());
  if (!expectedLifetime) {
    // We have a lifetime qualifier but no dominating property
    // attribute.  That's okay, but restore reasonable invariants by
    // setting the property attribute according to the lifetime
    // qualifier.
    ObjCPropertyDecl::PropertyAttributeKind attr;
    if (propertyLifetime == Qualifiers::OCL_Strong) {
      attr = ObjCPropertyDecl::OBJC_PR_strong;
    } else if (propertyLifetime == Qualifiers::OCL_Weak) {
      attr = ObjCPropertyDecl::OBJC_PR_weak;
    } else {
      assert(propertyLifetime == Qualifiers::OCL_ExplicitNone);
      attr = ObjCPropertyDecl::OBJC_PR_unsafe_unretained;
    }
    property->setPropertyAttributes(attr);
    return;
  }

  if (propertyLifetime == expectedLifetime) return;

  property->setInvalidDecl();
  S.Diag(property->getLocation(),
         diag::err_arc_inconsistent_property_ownership)
    << property->getDeclName()
    << expectedLifetime
    << propertyLifetime;
}

/// \brief Check this Objective-C property against a property declared in the
/// given protocol.
static void
CheckPropertyAgainstProtocol(Sema &S, ObjCPropertyDecl *Prop,
                             ObjCProtocolDecl *Proto,
                             llvm::SmallPtrSetImpl<ObjCProtocolDecl *> &Known) {
  // Have we seen this protocol before?
  if (!Known.insert(Proto).second)
    return;

  // Look for a property with the same name.
  DeclContext::lookup_result R = Proto->lookup(Prop->getDeclName());
  for (unsigned I = 0, N = R.size(); I != N; ++I) {
    if (ObjCPropertyDecl *ProtoProp = dyn_cast<ObjCPropertyDecl>(R[I])) {
      S.DiagnosePropertyMismatch(Prop, ProtoProp, Proto->getIdentifier(), true);
      return;
    }
  }

  // Check this property against any protocols we inherit.
  for (auto *P : Proto->protocols())
    CheckPropertyAgainstProtocol(S, Prop, P, Known);
}

Decl *Sema::ActOnProperty(Scope *S, SourceLocation AtLoc,
                          SourceLocation LParenLoc,
                          FieldDeclarator &FD,
                          ObjCDeclSpec &ODS,
                          Selector GetterSel,
                          Selector SetterSel,
                          bool *isOverridingProperty,
                          tok::ObjCKeywordKind MethodImplKind,
                          DeclContext *lexicalDC) {
  unsigned Attributes = ODS.getPropertyAttributes();
  FD.D.setObjCWeakProperty((Attributes & ObjCDeclSpec::DQ_PR_weak) != 0);
  TypeSourceInfo *TSI = GetTypeForDeclarator(FD.D, S);
  QualType T = TSI->getType();
  Attributes |= deduceWeakPropertyFromType(T);
  bool isReadWrite = ((Attributes & ObjCDeclSpec::DQ_PR_readwrite) ||
                      // default is readwrite!
                      !(Attributes & ObjCDeclSpec::DQ_PR_readonly));
  // property is defaulted to 'assign' if it is readwrite and is
  // not retain or copy
  bool isAssign = ((Attributes & ObjCDeclSpec::DQ_PR_assign) ||
                   (isReadWrite &&
                    !(Attributes & ObjCDeclSpec::DQ_PR_retain) &&
                    !(Attributes & ObjCDeclSpec::DQ_PR_strong) &&
                    !(Attributes & ObjCDeclSpec::DQ_PR_copy) &&
                    !(Attributes & ObjCDeclSpec::DQ_PR_unsafe_unretained) &&
                    !(Attributes & ObjCDeclSpec::DQ_PR_weak)));

  // Proceed with constructing the ObjCPropertyDecls.
  ObjCContainerDecl *ClassDecl = cast<ObjCContainerDecl>(CurContext);
  ObjCPropertyDecl *Res = nullptr;
  if (ObjCCategoryDecl *CDecl = dyn_cast<ObjCCategoryDecl>(ClassDecl)) {
    if (CDecl->IsClassExtension()) {
      Res = HandlePropertyInClassExtension(S, AtLoc, LParenLoc,
                                           FD, GetterSel, SetterSel,
                                           isAssign, isReadWrite,
                                           Attributes,
                                           ODS.getPropertyAttributes(),
                                           isOverridingProperty, T, TSI,
                                           MethodImplKind);
      if (!Res)
        return nullptr;
    }
  }

  if (!Res) {
    Res = CreatePropertyDecl(S, ClassDecl, AtLoc, LParenLoc, FD,
                             GetterSel, SetterSel, isAssign, isReadWrite,
                             Attributes, ODS.getPropertyAttributes(),
                             T, TSI, MethodImplKind);
    if (lexicalDC)
      Res->setLexicalDeclContext(lexicalDC);
  }

  // Validate the attributes on the @property.
  CheckObjCPropertyAttributes(Res, AtLoc, Attributes, 
                              (isa<ObjCInterfaceDecl>(ClassDecl) ||
                               isa<ObjCProtocolDecl>(ClassDecl)));

  if (getLangOpts().ObjCAutoRefCount)
    checkARCPropertyDecl(*this, Res);

  llvm::SmallPtrSet<ObjCProtocolDecl *, 16> KnownProtos;
  if (ObjCInterfaceDecl *IFace = dyn_cast<ObjCInterfaceDecl>(ClassDecl)) {
    // For a class, compare the property against a property in our superclass.
    bool FoundInSuper = false;
    ObjCInterfaceDecl *CurrentInterfaceDecl = IFace;
    while (ObjCInterfaceDecl *Super = CurrentInterfaceDecl->getSuperClass()) {
      DeclContext::lookup_result R = Super->lookup(Res->getDeclName());
      for (unsigned I = 0, N = R.size(); I != N; ++I) {
        if (ObjCPropertyDecl *SuperProp = dyn_cast<ObjCPropertyDecl>(R[I])) {
          DiagnosePropertyMismatch(Res, SuperProp, Super->getIdentifier(), false);
          FoundInSuper = true;
          break;
        }
      }
      if (FoundInSuper)
        break;
      else
        CurrentInterfaceDecl = Super;
    }

    if (FoundInSuper) {
      // Also compare the property against a property in our protocols.
      for (auto *P : CurrentInterfaceDecl->protocols()) {
        CheckPropertyAgainstProtocol(*this, Res, P, KnownProtos);
      }
    } else {
      // Slower path: look in all protocols we referenced.
      for (auto *P : IFace->all_referenced_protocols()) {
        CheckPropertyAgainstProtocol(*this, Res, P, KnownProtos);
      }
    }
  } else if (ObjCCategoryDecl *Cat = dyn_cast<ObjCCategoryDecl>(ClassDecl)) {
    for (auto *P : Cat->protocols())
      CheckPropertyAgainstProtocol(*this, Res, P, KnownProtos);
  } else {
    ObjCProtocolDecl *Proto = cast<ObjCProtocolDecl>(ClassDecl);
    for (auto *P : Proto->protocols())
      CheckPropertyAgainstProtocol(*this, Res, P, KnownProtos);
  }

  ActOnDocumentableDecl(Res);
  return Res;
}

static ObjCPropertyDecl::PropertyAttributeKind
makePropertyAttributesAsWritten(unsigned Attributes) {
  unsigned attributesAsWritten = 0;
  if (Attributes & ObjCDeclSpec::DQ_PR_readonly)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_readonly;
  if (Attributes & ObjCDeclSpec::DQ_PR_readwrite)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_readwrite;
  if (Attributes & ObjCDeclSpec::DQ_PR_getter)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_getter;
  if (Attributes & ObjCDeclSpec::DQ_PR_setter)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_setter;
  if (Attributes & ObjCDeclSpec::DQ_PR_assign)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_assign;
  if (Attributes & ObjCDeclSpec::DQ_PR_retain)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_retain;
  if (Attributes & ObjCDeclSpec::DQ_PR_strong)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_strong;
  if (Attributes & ObjCDeclSpec::DQ_PR_weak)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_weak;
  if (Attributes & ObjCDeclSpec::DQ_PR_copy)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_copy;
  if (Attributes & ObjCDeclSpec::DQ_PR_unsafe_unretained)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_unsafe_unretained;
  if (Attributes & ObjCDeclSpec::DQ_PR_nonatomic)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_nonatomic;
  if (Attributes & ObjCDeclSpec::DQ_PR_atomic)
    attributesAsWritten |= ObjCPropertyDecl::OBJC_PR_atomic;
  
  return (ObjCPropertyDecl::PropertyAttributeKind)attributesAsWritten;
}

static bool LocPropertyAttribute( ASTContext &Context, const char *attrName, 
                                 SourceLocation LParenLoc, SourceLocation &Loc) {
  if (LParenLoc.isMacroID())
    return false;
  
  SourceManager &SM = Context.getSourceManager();
  std::pair<FileID, unsigned> locInfo = SM.getDecomposedLoc(LParenLoc);
  // Try to load the file buffer.
  bool invalidTemp = false;
  StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
  if (invalidTemp)
    return false;
  const char *tokenBegin = file.data() + locInfo.second;
  
  // Lex from the start of the given location.
  Lexer lexer(SM.getLocForStartOfFile(locInfo.first),
              Context.getLangOpts(),
              file.begin(), tokenBegin, file.end());
  Token Tok;
  do {
    lexer.LexFromRawLexer(Tok);
    if (Tok.is(tok::raw_identifier) && Tok.getRawIdentifier() == attrName) {
      Loc = Tok.getLocation();
      return true;
    }
  } while (Tok.isNot(tok::r_paren));
  return false;
  
}

static unsigned getOwnershipRule(unsigned attr) {
  return attr & (ObjCPropertyDecl::OBJC_PR_assign |
                 ObjCPropertyDecl::OBJC_PR_retain |
                 ObjCPropertyDecl::OBJC_PR_copy   |
                 ObjCPropertyDecl::OBJC_PR_weak   |
                 ObjCPropertyDecl::OBJC_PR_strong |
                 ObjCPropertyDecl::OBJC_PR_unsafe_unretained);
}

ObjCPropertyDecl *
Sema::HandlePropertyInClassExtension(Scope *S,
                                     SourceLocation AtLoc,
                                     SourceLocation LParenLoc,
                                     FieldDeclarator &FD,
                                     Selector GetterSel, Selector SetterSel,
                                     const bool isAssign,
                                     const bool isReadWrite,
                                     const unsigned Attributes,
                                     const unsigned AttributesAsWritten,
                                     bool *isOverridingProperty,
                                     QualType T,
                                     TypeSourceInfo *TSI,
                                     tok::ObjCKeywordKind MethodImplKind) {
  ObjCCategoryDecl *CDecl = cast<ObjCCategoryDecl>(CurContext);
  // Diagnose if this property is already in continuation class.
  DeclContext *DC = CurContext;
  IdentifierInfo *PropertyId = FD.D.getIdentifier();
  ObjCInterfaceDecl *CCPrimary = CDecl->getClassInterface();
  
  if (CCPrimary) {
    // Check for duplicate declaration of this property in current and
    // other class extensions.
    for (const auto *Ext : CCPrimary->known_extensions()) {
      if (ObjCPropertyDecl *prevDecl
            = ObjCPropertyDecl::findPropertyDecl(Ext, PropertyId)) {
        Diag(AtLoc, diag::err_duplicate_property);
        Diag(prevDecl->getLocation(), diag::note_property_declare);
        return nullptr;
      }
    }
  }

  // Create a new ObjCPropertyDecl with the DeclContext being
  // the class extension.
  // FIXME. We should really be using CreatePropertyDecl for this.
  ObjCPropertyDecl *PDecl =
    ObjCPropertyDecl::Create(Context, DC, FD.D.getIdentifierLoc(),
                             PropertyId, AtLoc, LParenLoc, T, TSI);
  PDecl->setPropertyAttributesAsWritten(
                          makePropertyAttributesAsWritten(AttributesAsWritten));
  if (Attributes & ObjCDeclSpec::DQ_PR_readonly)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_readonly);
  if (Attributes & ObjCDeclSpec::DQ_PR_readwrite)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_readwrite);
  if (Attributes & ObjCDeclSpec::DQ_PR_nonatomic)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_nonatomic);
  if (Attributes & ObjCDeclSpec::DQ_PR_atomic)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_atomic);
  if (Attributes & ObjCDeclSpec::DQ_PR_nullability)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_nullability);
  if (Attributes & ObjCDeclSpec::DQ_PR_null_resettable)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_null_resettable);

  // Set setter/getter selector name. Needed later.
  PDecl->setGetterName(GetterSel);
  PDecl->setSetterName(SetterSel);
  ProcessDeclAttributes(S, PDecl, FD.D);
  DC->addDecl(PDecl);

  // We need to look in the @interface to see if the @property was
  // already declared.
  if (!CCPrimary) {
    Diag(CDecl->getLocation(), diag::err_continuation_class);
    *isOverridingProperty = true;
    return nullptr;
  }

  // Find the property in continuation class's primary class only.
  ObjCPropertyDecl *PIDecl =
    CCPrimary->FindPropertyVisibleInPrimaryClass(PropertyId);

  if (!PIDecl) {
    // No matching property found in the primary class. Just fall thru
    // and add property to continuation class's primary class.
    ObjCPropertyDecl *PrimaryPDecl =
      CreatePropertyDecl(S, CCPrimary, AtLoc, LParenLoc,
                         FD, GetterSel, SetterSel, isAssign, isReadWrite,
                         Attributes,AttributesAsWritten, T, TSI, MethodImplKind, 
                         DC);

    // A case of continuation class adding a new property in the class. This
    // is not what it was meant for. However, gcc supports it and so should we.
    // Make sure setter/getters are declared here.
    ProcessPropertyDecl(PrimaryPDecl, CCPrimary,
                        /* redeclaredProperty = */ nullptr,
                        /* lexicalDC = */ CDecl);
    PDecl->setGetterMethodDecl(PrimaryPDecl->getGetterMethodDecl());
    PDecl->setSetterMethodDecl(PrimaryPDecl->getSetterMethodDecl());
    if (ASTMutationListener *L = Context.getASTMutationListener())
      L->AddedObjCPropertyInClassExtension(PrimaryPDecl, /*OrigProp=*/nullptr,
                                           CDecl);
    return PrimaryPDecl;
  }
  if (!Context.hasSameType(PIDecl->getType(), PDecl->getType())) {
    bool IncompatibleObjC = false;
    QualType ConvertedType;
    // Relax the strict type matching for property type in continuation class.
    // Allow property object type of continuation class to be different as long
    // as it narrows the object type in its primary class property. Note that
    // this conversion is safe only because the wider type is for a 'readonly'
    // property in primary class and 'narrowed' type for a 'readwrite' property
    // in continuation class.
    QualType PrimaryClassPropertyT = Context.getCanonicalType(PIDecl->getType());
    QualType ClassExtPropertyT = Context.getCanonicalType(PDecl->getType());
    if (!isa<ObjCObjectPointerType>(PrimaryClassPropertyT) ||
        !isa<ObjCObjectPointerType>(ClassExtPropertyT) ||
        (!isObjCPointerConversion(ClassExtPropertyT, PrimaryClassPropertyT,
                                  ConvertedType, IncompatibleObjC))
        || IncompatibleObjC) {
      Diag(AtLoc, 
          diag::err_type_mismatch_continuation_class) << PDecl->getType();
      Diag(PIDecl->getLocation(), diag::note_property_declare);
      return nullptr;
    }
  }
    
  // The property 'PIDecl's readonly attribute will be over-ridden
  // with continuation class's readwrite property attribute!
  unsigned PIkind = PIDecl->getPropertyAttributesAsWritten();
  if (isReadWrite && (PIkind & ObjCPropertyDecl::OBJC_PR_readonly)) {
    PIkind &= ~ObjCPropertyDecl::OBJC_PR_readonly;
    PIkind |= ObjCPropertyDecl::OBJC_PR_readwrite;
    PIkind |= deduceWeakPropertyFromType(PIDecl->getType());
    unsigned ClassExtensionMemoryModel = getOwnershipRule(Attributes);
    unsigned PrimaryClassMemoryModel = getOwnershipRule(PIkind);
    if (PrimaryClassMemoryModel && ClassExtensionMemoryModel &&
        (PrimaryClassMemoryModel != ClassExtensionMemoryModel)) {
      Diag(AtLoc, diag::warn_property_attr_mismatch);
      Diag(PIDecl->getLocation(), diag::note_property_declare);
    }
    else if (getLangOpts().ObjCAutoRefCount) {
      QualType PrimaryPropertyQT =
        Context.getCanonicalType(PIDecl->getType()).getUnqualifiedType();
      if (isa<ObjCObjectPointerType>(PrimaryPropertyQT)) {
        bool PropertyIsWeak = ((PIkind & ObjCPropertyDecl::OBJC_PR_weak) != 0);
        Qualifiers::ObjCLifetime PrimaryPropertyLifeTime =
          PrimaryPropertyQT.getObjCLifetime();
        if (PrimaryPropertyLifeTime == Qualifiers::OCL_None &&
            (Attributes & ObjCDeclSpec::DQ_PR_weak) &&
            !PropertyIsWeak) {
              Diag(AtLoc, diag::warn_property_implicitly_mismatched);
              Diag(PIDecl->getLocation(), diag::note_property_declare);
            }
        }
    }
    
    DeclContext *DC = cast<DeclContext>(CCPrimary);
    if (!ObjCPropertyDecl::findPropertyDecl(DC,
                                 PIDecl->getDeclName().getAsIdentifierInfo())) {
      // In mrr mode, 'readwrite' property must have an explicit
      // memory attribute. If none specified, select the default (assign).
      if (!getLangOpts().ObjCAutoRefCount) {
        if (!(PIkind & (ObjCDeclSpec::DQ_PR_assign |
                        ObjCDeclSpec::DQ_PR_retain |
                        ObjCDeclSpec::DQ_PR_strong |
                        ObjCDeclSpec::DQ_PR_copy |
                        ObjCDeclSpec::DQ_PR_unsafe_unretained |
                        ObjCDeclSpec::DQ_PR_weak)))
          PIkind |= ObjCPropertyDecl::OBJC_PR_assign;
      }
      
      // Protocol is not in the primary class. Must build one for it.
      ObjCDeclSpec ProtocolPropertyODS;
      // FIXME. Assuming that ObjCDeclSpec::ObjCPropertyAttributeKind
      // and ObjCPropertyDecl::PropertyAttributeKind have identical
      // values.  Should consolidate both into one enum type.
      ProtocolPropertyODS.
      setPropertyAttributes((ObjCDeclSpec::ObjCPropertyAttributeKind)
                            PIkind);
      // Must re-establish the context from class extension to primary
      // class context.
      ContextRAII SavedContext(*this, CCPrimary);
      
      Decl *ProtocolPtrTy =
        ActOnProperty(S, AtLoc, LParenLoc, FD, ProtocolPropertyODS,
                      PIDecl->getGetterName(),
                      PIDecl->getSetterName(),
                      isOverridingProperty,
                      MethodImplKind,
                      /* lexicalDC = */ CDecl);
      PIDecl = cast<ObjCPropertyDecl>(ProtocolPtrTy);
    }
    PIDecl->makeitReadWriteAttribute();
    if (Attributes & ObjCDeclSpec::DQ_PR_retain)
      PIDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_retain);
    if (Attributes & ObjCDeclSpec::DQ_PR_strong)
      PIDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_strong);
    if (Attributes & ObjCDeclSpec::DQ_PR_copy)
      PIDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_copy);
    PIDecl->setSetterName(SetterSel);
  } else {
    // Tailor the diagnostics for the common case where a readwrite
    // property is declared both in the @interface and the continuation.
    // This is a common error where the user often intended the original
    // declaration to be readonly.
    unsigned diag =
      (Attributes & ObjCDeclSpec::DQ_PR_readwrite) &&
      (PIkind & ObjCPropertyDecl::OBJC_PR_readwrite)
      ? diag::err_use_continuation_class_redeclaration_readwrite
      : diag::err_use_continuation_class;
    Diag(AtLoc, diag)
      << CCPrimary->getDeclName();
    Diag(PIDecl->getLocation(), diag::note_property_declare);
    return nullptr;
  }
  *isOverridingProperty = true;
  // Make sure setter decl is synthesized, and added to primary class's list.
  ProcessPropertyDecl(PIDecl, CCPrimary, PDecl, CDecl);
  PDecl->setGetterMethodDecl(PIDecl->getGetterMethodDecl());
  PDecl->setSetterMethodDecl(PIDecl->getSetterMethodDecl());
  if (ASTMutationListener *L = Context.getASTMutationListener())
    L->AddedObjCPropertyInClassExtension(PDecl, PIDecl, CDecl);
  return PDecl;
}

ObjCPropertyDecl *Sema::CreatePropertyDecl(Scope *S,
                                           ObjCContainerDecl *CDecl,
                                           SourceLocation AtLoc,
                                           SourceLocation LParenLoc,
                                           FieldDeclarator &FD,
                                           Selector GetterSel,
                                           Selector SetterSel,
                                           const bool isAssign,
                                           const bool isReadWrite,
                                           const unsigned Attributes,
                                           const unsigned AttributesAsWritten,
                                           QualType T,
                                           TypeSourceInfo *TInfo,
                                           tok::ObjCKeywordKind MethodImplKind,
                                           DeclContext *lexicalDC){
  IdentifierInfo *PropertyId = FD.D.getIdentifier();

  // Issue a warning if property is 'assign' as default and its object, which is
  // gc'able conforms to NSCopying protocol
  if (getLangOpts().getGC() != LangOptions::NonGC &&
      isAssign && !(Attributes & ObjCDeclSpec::DQ_PR_assign))
    if (const ObjCObjectPointerType *ObjPtrTy =
          T->getAs<ObjCObjectPointerType>()) {
      ObjCInterfaceDecl *IDecl = ObjPtrTy->getObjectType()->getInterface();
      if (IDecl)
        if (ObjCProtocolDecl* PNSCopying =
            LookupProtocol(&Context.Idents.get("NSCopying"), AtLoc))
          if (IDecl->ClassImplementsProtocol(PNSCopying, true))
            Diag(AtLoc, diag::warn_implements_nscopying) << PropertyId;
    }

  if (T->isObjCObjectType()) {
    SourceLocation StarLoc = TInfo->getTypeLoc().getLocEnd();
    StarLoc = getLocForEndOfToken(StarLoc);
    Diag(FD.D.getIdentifierLoc(), diag::err_statically_allocated_object)
      << FixItHint::CreateInsertion(StarLoc, "*");
    T = Context.getObjCObjectPointerType(T);
    SourceLocation TLoc = TInfo->getTypeLoc().getLocStart();
    TInfo = Context.getTrivialTypeSourceInfo(T, TLoc);
  }

  DeclContext *DC = cast<DeclContext>(CDecl);
  ObjCPropertyDecl *PDecl = ObjCPropertyDecl::Create(Context, DC,
                                                     FD.D.getIdentifierLoc(),
                                                     PropertyId, AtLoc, 
                                                     LParenLoc, T, TInfo);

  if (ObjCPropertyDecl *prevDecl =
        ObjCPropertyDecl::findPropertyDecl(DC, PropertyId)) {
    Diag(PDecl->getLocation(), diag::err_duplicate_property);
    Diag(prevDecl->getLocation(), diag::note_property_declare);
    PDecl->setInvalidDecl();
  }
  else {
    DC->addDecl(PDecl);
    if (lexicalDC)
      PDecl->setLexicalDeclContext(lexicalDC);
  }

  if (T->isArrayType() || T->isFunctionType()) {
    Diag(AtLoc, diag::err_property_type) << T;
    PDecl->setInvalidDecl();
  }

  ProcessDeclAttributes(S, PDecl, FD.D);

  // Regardless of setter/getter attribute, we save the default getter/setter
  // selector names in anticipation of declaration of setter/getter methods.
  PDecl->setGetterName(GetterSel);
  PDecl->setSetterName(SetterSel);
  PDecl->setPropertyAttributesAsWritten(
                          makePropertyAttributesAsWritten(AttributesAsWritten));

  if (Attributes & ObjCDeclSpec::DQ_PR_readonly)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_readonly);

  if (Attributes & ObjCDeclSpec::DQ_PR_getter)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_getter);

  if (Attributes & ObjCDeclSpec::DQ_PR_setter)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_setter);

  if (isReadWrite)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_readwrite);

  if (Attributes & ObjCDeclSpec::DQ_PR_retain)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_retain);

  if (Attributes & ObjCDeclSpec::DQ_PR_strong)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_strong);

  if (Attributes & ObjCDeclSpec::DQ_PR_weak)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_weak);

  if (Attributes & ObjCDeclSpec::DQ_PR_copy)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_copy);

  if (Attributes & ObjCDeclSpec::DQ_PR_unsafe_unretained)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_unsafe_unretained);

  if (isAssign)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_assign);

  // In the semantic attributes, one of nonatomic or atomic is always set.
  if (Attributes & ObjCDeclSpec::DQ_PR_nonatomic)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_nonatomic);
  else
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_atomic);

  // 'unsafe_unretained' is alias for 'assign'.
  if (Attributes & ObjCDeclSpec::DQ_PR_unsafe_unretained)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_assign);
  if (isAssign)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_unsafe_unretained);

  if (MethodImplKind == tok::objc_required)
    PDecl->setPropertyImplementation(ObjCPropertyDecl::Required);
  else if (MethodImplKind == tok::objc_optional)
    PDecl->setPropertyImplementation(ObjCPropertyDecl::Optional);

  if (Attributes & ObjCDeclSpec::DQ_PR_nullability)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_nullability);

  if (Attributes & ObjCDeclSpec::DQ_PR_null_resettable)
    PDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_null_resettable);

  return PDecl;
}

static void checkARCPropertyImpl(Sema &S, SourceLocation propertyImplLoc,
                                 ObjCPropertyDecl *property,
                                 ObjCIvarDecl *ivar) {
  if (property->isInvalidDecl() || ivar->isInvalidDecl()) return;

  QualType ivarType = ivar->getType();
  Qualifiers::ObjCLifetime ivarLifetime = ivarType.getObjCLifetime();

  // The lifetime implied by the property's attributes.
  Qualifiers::ObjCLifetime propertyLifetime =
    getImpliedARCOwnership(property->getPropertyAttributes(),
                           property->getType());

  // We're fine if they match.
  if (propertyLifetime == ivarLifetime) return;

  // These aren't valid lifetimes for object ivars;  don't diagnose twice.
  if (ivarLifetime == Qualifiers::OCL_None ||
      ivarLifetime == Qualifiers::OCL_Autoreleasing)
    return;

  // If the ivar is private, and it's implicitly __unsafe_unretained
  // becaues of its type, then pretend it was actually implicitly
  // __strong.  This is only sound because we're processing the
  // property implementation before parsing any method bodies.
  if (ivarLifetime == Qualifiers::OCL_ExplicitNone &&
      propertyLifetime == Qualifiers::OCL_Strong &&
      ivar->getAccessControl() == ObjCIvarDecl::Private) {
    SplitQualType split = ivarType.split();
    if (split.Quals.hasObjCLifetime()) {
      assert(ivarType->isObjCARCImplicitlyUnretainedType());
      split.Quals.setObjCLifetime(Qualifiers::OCL_Strong);
      ivarType = S.Context.getQualifiedType(split);
      ivar->setType(ivarType);
      return;
    }
  }

  switch (propertyLifetime) {
  case Qualifiers::OCL_Strong:
    S.Diag(ivar->getLocation(), diag::err_arc_strong_property_ownership)
      << property->getDeclName()
      << ivar->getDeclName()
      << ivarLifetime;
    break;

  case Qualifiers::OCL_Weak:
    S.Diag(ivar->getLocation(), diag::error_weak_property)
      << property->getDeclName()
      << ivar->getDeclName();
    break;

  case Qualifiers::OCL_ExplicitNone:
    S.Diag(ivar->getLocation(), diag::err_arc_assign_property_ownership)
      << property->getDeclName()
      << ivar->getDeclName()
      << ((property->getPropertyAttributesAsWritten() 
           & ObjCPropertyDecl::OBJC_PR_assign) != 0);
    break;

  case Qualifiers::OCL_Autoreleasing:
    llvm_unreachable("properties cannot be autoreleasing");

  case Qualifiers::OCL_None:
    // Any other property should be ignored.
    return;
  }

  S.Diag(property->getLocation(), diag::note_property_declare);
  if (propertyImplLoc.isValid())
    S.Diag(propertyImplLoc, diag::note_property_synthesize);
}

/// setImpliedPropertyAttributeForReadOnlyProperty -
/// This routine evaludates life-time attributes for a 'readonly'
/// property with no known lifetime of its own, using backing
/// 'ivar's attribute, if any. If no backing 'ivar', property's
/// life-time is assumed 'strong'.
static void setImpliedPropertyAttributeForReadOnlyProperty(
              ObjCPropertyDecl *property, ObjCIvarDecl *ivar) {
  Qualifiers::ObjCLifetime propertyLifetime = 
    getImpliedARCOwnership(property->getPropertyAttributes(),
                           property->getType());
  if (propertyLifetime != Qualifiers::OCL_None)
    return;
  
  if (!ivar) {
    // if no backing ivar, make property 'strong'.
    property->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_strong);
    return;
  }
  // property assumes owenership of backing ivar.
  QualType ivarType = ivar->getType();
  Qualifiers::ObjCLifetime ivarLifetime = ivarType.getObjCLifetime();
  if (ivarLifetime == Qualifiers::OCL_Strong)
    property->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_strong);
  else if (ivarLifetime == Qualifiers::OCL_Weak)
    property->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_weak);
  return;
}

/// DiagnosePropertyMismatchDeclInProtocols - diagnose properties declared
/// in inherited protocols with mismatched types. Since any of them can
/// be candidate for synthesis.
static void
DiagnosePropertyMismatchDeclInProtocols(Sema &S, SourceLocation AtLoc,
                                        ObjCInterfaceDecl *ClassDecl,
                                        ObjCPropertyDecl *Property) {
  ObjCInterfaceDecl::ProtocolPropertyMap PropMap;
  for (const auto *PI : ClassDecl->all_referenced_protocols()) {
    if (const ObjCProtocolDecl *PDecl = PI->getDefinition())
      PDecl->collectInheritedProtocolProperties(Property, PropMap);
  }
  if (ObjCInterfaceDecl *SDecl = ClassDecl->getSuperClass())
    while (SDecl) {
      for (const auto *PI : SDecl->all_referenced_protocols()) {
        if (const ObjCProtocolDecl *PDecl = PI->getDefinition())
          PDecl->collectInheritedProtocolProperties(Property, PropMap);
      }
      SDecl = SDecl->getSuperClass();
    }
  
  if (PropMap.empty())
    return;
  
  QualType RHSType = S.Context.getCanonicalType(Property->getType());
  bool FirsTime = true;
  for (ObjCInterfaceDecl::ProtocolPropertyMap::iterator
       I = PropMap.begin(), E = PropMap.end(); I != E; I++) {
    ObjCPropertyDecl *Prop = I->second;
    QualType LHSType = S.Context.getCanonicalType(Prop->getType());
    if (!S.Context.propertyTypesAreCompatible(LHSType, RHSType)) {
      bool IncompatibleObjC = false;
      QualType ConvertedType;
      if (!S.isObjCPointerConversion(RHSType, LHSType, ConvertedType, IncompatibleObjC)
          || IncompatibleObjC) {
        if (FirsTime) {
          S.Diag(Property->getLocation(), diag::warn_protocol_property_mismatch)
            << Property->getType();
          FirsTime = false;
        }
        S.Diag(Prop->getLocation(), diag::note_protocol_property_declare)
          << Prop->getType();
      }
    }
  }
  if (!FirsTime && AtLoc.isValid())
    S.Diag(AtLoc, diag::note_property_synthesize);
}

/// ActOnPropertyImplDecl - This routine performs semantic checks and
/// builds the AST node for a property implementation declaration; declared
/// as \@synthesize or \@dynamic.
///
Decl *Sema::ActOnPropertyImplDecl(Scope *S,
                                  SourceLocation AtLoc,
                                  SourceLocation PropertyLoc,
                                  bool Synthesize,
                                  IdentifierInfo *PropertyId,
                                  IdentifierInfo *PropertyIvar,
                                  SourceLocation PropertyIvarLoc) {
  ObjCContainerDecl *ClassImpDecl =
    dyn_cast<ObjCContainerDecl>(CurContext);
  // Make sure we have a context for the property implementation declaration.
  if (!ClassImpDecl) {
    Diag(AtLoc, diag::error_missing_property_context);
    return nullptr;
  }
  if (PropertyIvarLoc.isInvalid())
    PropertyIvarLoc = PropertyLoc;
  SourceLocation PropertyDiagLoc = PropertyLoc;
  if (PropertyDiagLoc.isInvalid())
    PropertyDiagLoc = ClassImpDecl->getLocStart();
  ObjCPropertyDecl *property = nullptr;
  ObjCInterfaceDecl *IDecl = nullptr;
  // Find the class or category class where this property must have
  // a declaration.
  ObjCImplementationDecl *IC = nullptr;
  ObjCCategoryImplDecl *CatImplClass = nullptr;
  if ((IC = dyn_cast<ObjCImplementationDecl>(ClassImpDecl))) {
    IDecl = IC->getClassInterface();
    // We always synthesize an interface for an implementation
    // without an interface decl. So, IDecl is always non-zero.
    assert(IDecl &&
           "ActOnPropertyImplDecl - @implementation without @interface");

    // Look for this property declaration in the @implementation's @interface
    property = IDecl->FindPropertyDeclaration(PropertyId);
    if (!property) {
      Diag(PropertyLoc, diag::error_bad_property_decl) << IDecl->getDeclName();
      return nullptr;
    }
    unsigned PIkind = property->getPropertyAttributesAsWritten();
    if ((PIkind & (ObjCPropertyDecl::OBJC_PR_atomic |
                   ObjCPropertyDecl::OBJC_PR_nonatomic) ) == 0) {
      if (AtLoc.isValid())
        Diag(AtLoc, diag::warn_implicit_atomic_property);
      else
        Diag(IC->getLocation(), diag::warn_auto_implicit_atomic_property);
      Diag(property->getLocation(), diag::note_property_declare);
    }
    
    if (const ObjCCategoryDecl *CD =
        dyn_cast<ObjCCategoryDecl>(property->getDeclContext())) {
      if (!CD->IsClassExtension()) {
        Diag(PropertyLoc, diag::error_category_property) << CD->getDeclName();
        Diag(property->getLocation(), diag::note_property_declare);
        return nullptr;
      }
    }
    if (Synthesize&&
        (PIkind & ObjCPropertyDecl::OBJC_PR_readonly) &&
        property->hasAttr<IBOutletAttr>() &&
        !AtLoc.isValid()) {
      bool ReadWriteProperty = false;
      // Search into the class extensions and see if 'readonly property is
      // redeclared 'readwrite', then no warning is to be issued.
      for (auto *Ext : IDecl->known_extensions()) {
        DeclContext::lookup_result R = Ext->lookup(property->getDeclName());
        if (!R.empty())
          if (ObjCPropertyDecl *ExtProp = dyn_cast<ObjCPropertyDecl>(R[0])) {
            PIkind = ExtProp->getPropertyAttributesAsWritten();
            if (PIkind & ObjCPropertyDecl::OBJC_PR_readwrite) {
              ReadWriteProperty = true;
              break;
            }
          }
      }
      
      if (!ReadWriteProperty) {
        Diag(property->getLocation(), diag::warn_auto_readonly_iboutlet_property)
            << property;
        SourceLocation readonlyLoc;
        if (LocPropertyAttribute(Context, "readonly", 
                                 property->getLParenLoc(), readonlyLoc)) {
          SourceLocation endLoc = 
            readonlyLoc.getLocWithOffset(strlen("readonly")-1);
          SourceRange ReadonlySourceRange(readonlyLoc, endLoc);
          Diag(property->getLocation(), 
               diag::note_auto_readonly_iboutlet_fixup_suggest) <<
          FixItHint::CreateReplacement(ReadonlySourceRange, "readwrite");
        }
      }
    }
    if (Synthesize && isa<ObjCProtocolDecl>(property->getDeclContext()))
      DiagnosePropertyMismatchDeclInProtocols(*this, AtLoc, IDecl, property);
        
  } else if ((CatImplClass = dyn_cast<ObjCCategoryImplDecl>(ClassImpDecl))) {
    if (Synthesize) {
      Diag(AtLoc, diag::error_synthesize_category_decl);
      return nullptr;
    }
    IDecl = CatImplClass->getClassInterface();
    if (!IDecl) {
      Diag(AtLoc, diag::error_missing_property_interface);
      return nullptr;
    }
    ObjCCategoryDecl *Category =
    IDecl->FindCategoryDeclaration(CatImplClass->getIdentifier());

    // If category for this implementation not found, it is an error which
    // has already been reported eralier.
    if (!Category)
      return nullptr;
    // Look for this property declaration in @implementation's category
    property = Category->FindPropertyDeclaration(PropertyId);
    if (!property) {
      Diag(PropertyLoc, diag::error_bad_category_property_decl)
      << Category->getDeclName();
      return nullptr;
    }
  } else {
    Diag(AtLoc, diag::error_bad_property_context);
    return nullptr;
  }
  ObjCIvarDecl *Ivar = nullptr;
  bool CompleteTypeErr = false;
  bool compat = true;
  // Check that we have a valid, previously declared ivar for @synthesize
  if (Synthesize) {
    // @synthesize
    if (!PropertyIvar)
      PropertyIvar = PropertyId;
    // Check that this is a previously declared 'ivar' in 'IDecl' interface
    ObjCInterfaceDecl *ClassDeclared;
    Ivar = IDecl->lookupInstanceVariable(PropertyIvar, ClassDeclared);
    QualType PropType = property->getType();
    QualType PropertyIvarType = PropType.getNonReferenceType();

    if (RequireCompleteType(PropertyDiagLoc, PropertyIvarType,
                            diag::err_incomplete_synthesized_property,
                            property->getDeclName())) {
      Diag(property->getLocation(), diag::note_property_declare);
      CompleteTypeErr = true;
    }

    if (getLangOpts().ObjCAutoRefCount &&
        (property->getPropertyAttributesAsWritten() &
         ObjCPropertyDecl::OBJC_PR_readonly) &&
        PropertyIvarType->isObjCRetainableType()) {
      setImpliedPropertyAttributeForReadOnlyProperty(property, Ivar);    
    }
    
    ObjCPropertyDecl::PropertyAttributeKind kind 
      = property->getPropertyAttributes();

    // Add GC __weak to the ivar type if the property is weak.
    if ((kind & ObjCPropertyDecl::OBJC_PR_weak) && 
        getLangOpts().getGC() != LangOptions::NonGC) {
      assert(!getLangOpts().ObjCAutoRefCount);
      if (PropertyIvarType.isObjCGCStrong()) {
        Diag(PropertyDiagLoc, diag::err_gc_weak_property_strong_type);
        Diag(property->getLocation(), diag::note_property_declare);
      } else {
        PropertyIvarType =
          Context.getObjCGCQualType(PropertyIvarType, Qualifiers::Weak);
      }
    }
    if (AtLoc.isInvalid()) {
      // Check when default synthesizing a property that there is 
      // an ivar matching property name and issue warning; since this
      // is the most common case of not using an ivar used for backing
      // property in non-default synthesis case.
      ObjCInterfaceDecl *ClassDeclared=nullptr;
      ObjCIvarDecl *originalIvar = 
      IDecl->lookupInstanceVariable(property->getIdentifier(), 
                                    ClassDeclared);
      if (originalIvar) {
        Diag(PropertyDiagLoc, 
             diag::warn_autosynthesis_property_ivar_match)
        << PropertyId << (Ivar == nullptr) << PropertyIvar
        << originalIvar->getIdentifier();
        Diag(property->getLocation(), diag::note_property_declare);
        Diag(originalIvar->getLocation(), diag::note_ivar_decl);
      }
    }
    
    if (!Ivar) {
      // In ARC, give the ivar a lifetime qualifier based on the
      // property attributes.
      if (getLangOpts().ObjCAutoRefCount &&
          !PropertyIvarType.getObjCLifetime() &&
          PropertyIvarType->isObjCRetainableType()) {

        // It's an error if we have to do this and the user didn't
        // explicitly write an ownership attribute on the property.
        if (!property->hasWrittenStorageAttribute() &&
            !(kind & ObjCPropertyDecl::OBJC_PR_strong)) {
          Diag(PropertyDiagLoc,
               diag::err_arc_objc_property_default_assign_on_object);
          Diag(property->getLocation(), diag::note_property_declare);
        } else {
          Qualifiers::ObjCLifetime lifetime =
            getImpliedARCOwnership(kind, PropertyIvarType);
          assert(lifetime && "no lifetime for property?");
          if (lifetime == Qualifiers::OCL_Weak) {
            bool err = false;
            if (const ObjCObjectPointerType *ObjT =
                PropertyIvarType->getAs<ObjCObjectPointerType>()) {
              const ObjCInterfaceDecl *ObjI = ObjT->getInterfaceDecl();
              if (ObjI && ObjI->isArcWeakrefUnavailable()) {
                Diag(property->getLocation(),
                     diag::err_arc_weak_unavailable_property) << PropertyIvarType;
                Diag(ClassImpDecl->getLocation(), diag::note_implemented_by_class)
                  << ClassImpDecl->getName();
                err = true;
              }
            }
            if (!err && !getLangOpts().ObjCARCWeak) {
              Diag(PropertyDiagLoc, diag::err_arc_weak_no_runtime);
              Diag(property->getLocation(), diag::note_property_declare);
            }
          }
          
          Qualifiers qs;
          qs.addObjCLifetime(lifetime);
          PropertyIvarType = Context.getQualifiedType(PropertyIvarType, qs);   
        }
      }

      if (kind & ObjCPropertyDecl::OBJC_PR_weak &&
          !getLangOpts().ObjCAutoRefCount &&
          getLangOpts().getGC() == LangOptions::NonGC) {
        Diag(PropertyDiagLoc, diag::error_synthesize_weak_non_arc_or_gc);
        Diag(property->getLocation(), diag::note_property_declare);
      }

      Ivar = ObjCIvarDecl::Create(Context, ClassImpDecl,
                                  PropertyIvarLoc,PropertyIvarLoc, PropertyIvar,
                                  PropertyIvarType, /*Dinfo=*/nullptr,
                                  ObjCIvarDecl::Private,
                                  (Expr *)nullptr, true);
      if (RequireNonAbstractType(PropertyIvarLoc,
                                 PropertyIvarType,
                                 diag::err_abstract_type_in_decl,
                                 AbstractSynthesizedIvarType)) {
        Diag(property->getLocation(), diag::note_property_declare);
        Ivar->setInvalidDecl();
      } else if (CompleteTypeErr)
          Ivar->setInvalidDecl();
      ClassImpDecl->addDecl(Ivar);
      IDecl->makeDeclVisibleInContext(Ivar);

      if (getLangOpts().ObjCRuntime.isFragile())
        Diag(PropertyDiagLoc, diag::error_missing_property_ivar_decl)
            << PropertyId;
      // Note! I deliberately want it to fall thru so, we have a
      // a property implementation and to avoid future warnings.
    } else if (getLangOpts().ObjCRuntime.isNonFragile() &&
               !declaresSameEntity(ClassDeclared, IDecl)) {
      Diag(PropertyDiagLoc, diag::error_ivar_in_superclass_use)
      << property->getDeclName() << Ivar->getDeclName()
      << ClassDeclared->getDeclName();
      Diag(Ivar->getLocation(), diag::note_previous_access_declaration)
      << Ivar << Ivar->getName();
      // Note! I deliberately want it to fall thru so more errors are caught.
    }
    property->setPropertyIvarDecl(Ivar);

    QualType IvarType = Context.getCanonicalType(Ivar->getType());

    // Check that type of property and its ivar are type compatible.
    if (!Context.hasSameType(PropertyIvarType, IvarType)) {
      if (isa<ObjCObjectPointerType>(PropertyIvarType) 
          && isa<ObjCObjectPointerType>(IvarType))
        compat =
          Context.canAssignObjCInterfaces(
                                  PropertyIvarType->getAs<ObjCObjectPointerType>(),
                                  IvarType->getAs<ObjCObjectPointerType>());
      else {
        compat = (CheckAssignmentConstraints(PropertyIvarLoc, PropertyIvarType,
                                             IvarType)
                    == Compatible);
      }
      if (!compat) {
        Diag(PropertyDiagLoc, diag::error_property_ivar_type)
          << property->getDeclName() << PropType
          << Ivar->getDeclName() << IvarType;
        Diag(Ivar->getLocation(), diag::note_ivar_decl);
        // Note! I deliberately want it to fall thru so, we have a
        // a property implementation and to avoid future warnings.
      }
      else {
        // FIXME! Rules for properties are somewhat different that those
        // for assignments. Use a new routine to consolidate all cases;
        // specifically for property redeclarations as well as for ivars.
        QualType lhsType =Context.getCanonicalType(PropertyIvarType).getUnqualifiedType();
        QualType rhsType =Context.getCanonicalType(IvarType).getUnqualifiedType();
        if (lhsType != rhsType &&
            lhsType->isArithmeticType()) {
          Diag(PropertyDiagLoc, diag::error_property_ivar_type)
            << property->getDeclName() << PropType
            << Ivar->getDeclName() << IvarType;
          Diag(Ivar->getLocation(), diag::note_ivar_decl);
          // Fall thru - see previous comment
        }
      }
      // __weak is explicit. So it works on Canonical type.
      if ((PropType.isObjCGCWeak() && !IvarType.isObjCGCWeak() &&
           getLangOpts().getGC() != LangOptions::NonGC)) {
        Diag(PropertyDiagLoc, diag::error_weak_property)
        << property->getDeclName() << Ivar->getDeclName();
        Diag(Ivar->getLocation(), diag::note_ivar_decl);
        // Fall thru - see previous comment
      }
      // Fall thru - see previous comment
      if ((property->getType()->isObjCObjectPointerType() ||
           PropType.isObjCGCStrong()) && IvarType.isObjCGCWeak() &&
          getLangOpts().getGC() != LangOptions::NonGC) {
        Diag(PropertyDiagLoc, diag::error_strong_property)
        << property->getDeclName() << Ivar->getDeclName();
        // Fall thru - see previous comment
      }
    }
    if (getLangOpts().ObjCAutoRefCount)
      checkARCPropertyImpl(*this, PropertyLoc, property, Ivar);
  } else if (PropertyIvar)
    // @dynamic
    Diag(PropertyDiagLoc, diag::error_dynamic_property_ivar_decl);
    
  assert (property && "ActOnPropertyImplDecl - property declaration missing");
  ObjCPropertyImplDecl *PIDecl =
  ObjCPropertyImplDecl::Create(Context, CurContext, AtLoc, PropertyLoc,
                               property,
                               (Synthesize ?
                                ObjCPropertyImplDecl::Synthesize
                                : ObjCPropertyImplDecl::Dynamic),
                               Ivar, PropertyIvarLoc);

  if (CompleteTypeErr || !compat)
    PIDecl->setInvalidDecl();

  if (ObjCMethodDecl *getterMethod = property->getGetterMethodDecl()) {
    getterMethod->createImplicitParams(Context, IDecl);
    if (getLangOpts().CPlusPlus && Synthesize && !CompleteTypeErr &&
        Ivar->getType()->isRecordType()) {
      // For Objective-C++, need to synthesize the AST for the IVAR object to be
      // returned by the getter as it must conform to C++'s copy-return rules.
      // FIXME. Eventually we want to do this for Objective-C as well.
      SynthesizedFunctionScope Scope(*this, getterMethod);
      ImplicitParamDecl *SelfDecl = getterMethod->getSelfDecl();
      DeclRefExpr *SelfExpr = 
        new (Context) DeclRefExpr(SelfDecl, false, SelfDecl->getType(),
                                  VK_LValue, PropertyDiagLoc);
      MarkDeclRefReferenced(SelfExpr);
      Expr *LoadSelfExpr =
        ImplicitCastExpr::Create(Context, SelfDecl->getType(),
                                 CK_LValueToRValue, SelfExpr, nullptr,
                                 VK_RValue);
      Expr *IvarRefExpr =
        new (Context) ObjCIvarRefExpr(Ivar,
                                      Ivar->getUsageType(SelfDecl->getType()),
                                      PropertyDiagLoc,
                                      Ivar->getLocation(),
                                      LoadSelfExpr, true, true);
      ExprResult Res = PerformCopyInitialization(
          InitializedEntity::InitializeResult(PropertyDiagLoc,
                                              getterMethod->getReturnType(),
                                              /*NRVO=*/false),
          PropertyDiagLoc, IvarRefExpr);
      if (!Res.isInvalid()) {
        Expr *ResExpr = Res.getAs<Expr>();
        if (ResExpr)
          ResExpr = MaybeCreateExprWithCleanups(ResExpr);
        PIDecl->setGetterCXXConstructor(ResExpr);
      }
    }
    if (property->hasAttr<NSReturnsNotRetainedAttr>() &&
        !getterMethod->hasAttr<NSReturnsNotRetainedAttr>()) {
      Diag(getterMethod->getLocation(), 
           diag::warn_property_getter_owning_mismatch);
      Diag(property->getLocation(), diag::note_property_declare);
    }
    if (getLangOpts().ObjCAutoRefCount && Synthesize)
      switch (getterMethod->getMethodFamily()) {
        case OMF_retain:
        case OMF_retainCount:
        case OMF_release:
        case OMF_autorelease:
          Diag(getterMethod->getLocation(), diag::err_arc_illegal_method_def)
            << 1 << getterMethod->getSelector();
          break;
        default:
          break;
      }
  }
  if (ObjCMethodDecl *setterMethod = property->getSetterMethodDecl()) {
    setterMethod->createImplicitParams(Context, IDecl);
    if (getLangOpts().CPlusPlus && Synthesize && !CompleteTypeErr &&
        Ivar->getType()->isRecordType()) {
      // FIXME. Eventually we want to do this for Objective-C as well.
      SynthesizedFunctionScope Scope(*this, setterMethod);
      ImplicitParamDecl *SelfDecl = setterMethod->getSelfDecl();
      DeclRefExpr *SelfExpr = 
        new (Context) DeclRefExpr(SelfDecl, false, SelfDecl->getType(),
                                  VK_LValue, PropertyDiagLoc);
      MarkDeclRefReferenced(SelfExpr);
      Expr *LoadSelfExpr =
        ImplicitCastExpr::Create(Context, SelfDecl->getType(),
                                 CK_LValueToRValue, SelfExpr, nullptr,
                                 VK_RValue);
      Expr *lhs =
        new (Context) ObjCIvarRefExpr(Ivar,
                                      Ivar->getUsageType(SelfDecl->getType()),
                                      PropertyDiagLoc,
                                      Ivar->getLocation(),
                                      LoadSelfExpr, true, true);
      ObjCMethodDecl::param_iterator P = setterMethod->param_begin();
      ParmVarDecl *Param = (*P);
      QualType T = Param->getType().getNonReferenceType();
      DeclRefExpr *rhs = new (Context) DeclRefExpr(Param, false, T,
                                                   VK_LValue, PropertyDiagLoc);
      MarkDeclRefReferenced(rhs);
      ExprResult Res = BuildBinOp(S, PropertyDiagLoc, 
                                  BO_Assign, lhs, rhs);
      if (property->getPropertyAttributes() & 
          ObjCPropertyDecl::OBJC_PR_atomic) {
        Expr *callExpr = Res.getAs<Expr>();
        if (const CXXOperatorCallExpr *CXXCE = 
              dyn_cast_or_null<CXXOperatorCallExpr>(callExpr))
          if (const FunctionDecl *FuncDecl = CXXCE->getDirectCallee())
            if (!FuncDecl->isTrivial())
              if (property->getType()->isReferenceType()) {
                Diag(PropertyDiagLoc, 
                     diag::err_atomic_property_nontrivial_assign_op)
                    << property->getType();
                Diag(FuncDecl->getLocStart(), 
                     diag::note_callee_decl) << FuncDecl;
              }
      }
      PIDecl->setSetterCXXAssignment(Res.getAs<Expr>());
    }
  }
  
  if (IC) {
    if (Synthesize)
      if (ObjCPropertyImplDecl *PPIDecl =
          IC->FindPropertyImplIvarDecl(PropertyIvar)) {
        Diag(PropertyLoc, diag::error_duplicate_ivar_use)
        << PropertyId << PPIDecl->getPropertyDecl()->getIdentifier()
        << PropertyIvar;
        Diag(PPIDecl->getLocation(), diag::note_previous_use);
      }

    if (ObjCPropertyImplDecl *PPIDecl
        = IC->FindPropertyImplDecl(PropertyId)) {
      Diag(PropertyLoc, diag::error_property_implemented) << PropertyId;
      Diag(PPIDecl->getLocation(), diag::note_previous_declaration);
      return nullptr;
    }
    IC->addPropertyImplementation(PIDecl);
    if (getLangOpts().ObjCDefaultSynthProperties &&
        getLangOpts().ObjCRuntime.isNonFragile() &&
        !IDecl->isObjCRequiresPropertyDefs()) {
      // Diagnose if an ivar was lazily synthesdized due to a previous
      // use and if 1) property is @dynamic or 2) property is synthesized
      // but it requires an ivar of different name.
      ObjCInterfaceDecl *ClassDeclared=nullptr;
      ObjCIvarDecl *Ivar = nullptr;
      if (!Synthesize)
        Ivar = IDecl->lookupInstanceVariable(PropertyId, ClassDeclared);
      else {
        if (PropertyIvar && PropertyIvar != PropertyId)
          Ivar = IDecl->lookupInstanceVariable(PropertyId, ClassDeclared);
      }
      // Issue diagnostics only if Ivar belongs to current class.
      if (Ivar && Ivar->getSynthesize() && 
          declaresSameEntity(IC->getClassInterface(), ClassDeclared)) {
        Diag(Ivar->getLocation(), diag::err_undeclared_var_use) 
        << PropertyId;
        Ivar->setInvalidDecl();
      }
    }
  } else {
    if (Synthesize)
      if (ObjCPropertyImplDecl *PPIDecl =
          CatImplClass->FindPropertyImplIvarDecl(PropertyIvar)) {
        Diag(PropertyDiagLoc, diag::error_duplicate_ivar_use)
        << PropertyId << PPIDecl->getPropertyDecl()->getIdentifier()
        << PropertyIvar;
        Diag(PPIDecl->getLocation(), diag::note_previous_use);
      }

    if (ObjCPropertyImplDecl *PPIDecl =
        CatImplClass->FindPropertyImplDecl(PropertyId)) {
      Diag(PropertyDiagLoc, diag::error_property_implemented) << PropertyId;
      Diag(PPIDecl->getLocation(), diag::note_previous_declaration);
      return nullptr;
    }
    CatImplClass->addPropertyImplementation(PIDecl);
  }

  return PIDecl;
}

//===----------------------------------------------------------------------===//
// Helper methods.
//===----------------------------------------------------------------------===//

/// DiagnosePropertyMismatch - Compares two properties for their
/// attributes and types and warns on a variety of inconsistencies.
///
void
Sema::DiagnosePropertyMismatch(ObjCPropertyDecl *Property,
                               ObjCPropertyDecl *SuperProperty,
                               const IdentifierInfo *inheritedName,
                               bool OverridingProtocolProperty) {
  ObjCPropertyDecl::PropertyAttributeKind CAttr =
    Property->getPropertyAttributes();
  ObjCPropertyDecl::PropertyAttributeKind SAttr =
    SuperProperty->getPropertyAttributes();
  
  // We allow readonly properties without an explicit ownership
  // (assign/unsafe_unretained/weak/retain/strong/copy) in super class
  // to be overridden by a property with any explicit ownership in the subclass.
  if (!OverridingProtocolProperty &&
      !getOwnershipRule(SAttr) && getOwnershipRule(CAttr))
    ;
  else {
    if ((CAttr & ObjCPropertyDecl::OBJC_PR_readonly)
        && (SAttr & ObjCPropertyDecl::OBJC_PR_readwrite))
      Diag(Property->getLocation(), diag::warn_readonly_property)
        << Property->getDeclName() << inheritedName;
    if ((CAttr & ObjCPropertyDecl::OBJC_PR_copy)
        != (SAttr & ObjCPropertyDecl::OBJC_PR_copy))
      Diag(Property->getLocation(), diag::warn_property_attribute)
        << Property->getDeclName() << "copy" << inheritedName;
    else if (!(SAttr & ObjCPropertyDecl::OBJC_PR_readonly)){
      unsigned CAttrRetain =
        (CAttr &
         (ObjCPropertyDecl::OBJC_PR_retain | ObjCPropertyDecl::OBJC_PR_strong));
      unsigned SAttrRetain =
        (SAttr &
         (ObjCPropertyDecl::OBJC_PR_retain | ObjCPropertyDecl::OBJC_PR_strong));
      bool CStrong = (CAttrRetain != 0);
      bool SStrong = (SAttrRetain != 0);
      if (CStrong != SStrong)
        Diag(Property->getLocation(), diag::warn_property_attribute)
          << Property->getDeclName() << "retain (or strong)" << inheritedName;
    }
  }

  if ((CAttr & ObjCPropertyDecl::OBJC_PR_nonatomic)
      != (SAttr & ObjCPropertyDecl::OBJC_PR_nonatomic)) {
    Diag(Property->getLocation(), diag::warn_property_attribute)
      << Property->getDeclName() << "atomic" << inheritedName;
    Diag(SuperProperty->getLocation(), diag::note_property_declare);
  }
  if (Property->getSetterName() != SuperProperty->getSetterName()) {
    Diag(Property->getLocation(), diag::warn_property_attribute)
      << Property->getDeclName() << "setter" << inheritedName;
    Diag(SuperProperty->getLocation(), diag::note_property_declare);
  }
  if (Property->getGetterName() != SuperProperty->getGetterName()) {
    Diag(Property->getLocation(), diag::warn_property_attribute)
      << Property->getDeclName() << "getter" << inheritedName;
    Diag(SuperProperty->getLocation(), diag::note_property_declare);
  }

  QualType LHSType =
    Context.getCanonicalType(SuperProperty->getType());
  QualType RHSType =
    Context.getCanonicalType(Property->getType());

  if (!Context.propertyTypesAreCompatible(LHSType, RHSType)) {
    // Do cases not handled in above.
    // FIXME. For future support of covariant property types, revisit this.
    bool IncompatibleObjC = false;
    QualType ConvertedType;
    if (!isObjCPointerConversion(RHSType, LHSType, 
                                 ConvertedType, IncompatibleObjC) ||
        IncompatibleObjC) {
        Diag(Property->getLocation(), diag::warn_property_types_are_incompatible)
        << Property->getType() << SuperProperty->getType() << inheritedName;
      Diag(SuperProperty->getLocation(), diag::note_property_declare);
    }
  }
}

bool Sema::DiagnosePropertyAccessorMismatch(ObjCPropertyDecl *property,
                                            ObjCMethodDecl *GetterMethod,
                                            SourceLocation Loc) {
  if (!GetterMethod)
    return false;
  QualType GetterType = GetterMethod->getReturnType().getNonReferenceType();
  QualType PropertyIvarType = property->getType().getNonReferenceType();
  bool compat = Context.hasSameType(PropertyIvarType, GetterType);
  if (!compat) {
    if (isa<ObjCObjectPointerType>(PropertyIvarType) && 
        isa<ObjCObjectPointerType>(GetterType))
      compat =
        Context.canAssignObjCInterfaces(
                                      GetterType->getAs<ObjCObjectPointerType>(),
                                      PropertyIvarType->getAs<ObjCObjectPointerType>());
    else if (CheckAssignmentConstraints(Loc, GetterType, PropertyIvarType) 
              != Compatible) {
          Diag(Loc, diag::error_property_accessor_type)
            << property->getDeclName() << PropertyIvarType
            << GetterMethod->getSelector() << GetterType;
          Diag(GetterMethod->getLocation(), diag::note_declared_at);
          return true;
    } else {
      compat = true;
      QualType lhsType =Context.getCanonicalType(PropertyIvarType).getUnqualifiedType();
      QualType rhsType =Context.getCanonicalType(GetterType).getUnqualifiedType();
      if (lhsType != rhsType && lhsType->isArithmeticType())
        compat = false;
    }
  }
  
  if (!compat) {
    Diag(Loc, diag::warn_accessor_property_type_mismatch)
    << property->getDeclName()
    << GetterMethod->getSelector();
    Diag(GetterMethod->getLocation(), diag::note_declared_at);
    return true;
  }

  return false;
}

/// CollectImmediateProperties - This routine collects all properties in
/// the class and its conforming protocols; but not those in its super class.
static void CollectImmediateProperties(ObjCContainerDecl *CDecl,
                                       ObjCContainerDecl::PropertyMap &PropMap,
                                       ObjCContainerDecl::PropertyMap &SuperPropMap,
                                       bool IncludeProtocols = true) {

  if (ObjCInterfaceDecl *IDecl = dyn_cast<ObjCInterfaceDecl>(CDecl)) {
    for (auto *Prop : IDecl->properties())
      PropMap[Prop->getIdentifier()] = Prop;
    if (IncludeProtocols) {
      // Scan through class's protocols.
      for (auto *PI : IDecl->all_referenced_protocols())
        CollectImmediateProperties(PI, PropMap, SuperPropMap);
    }
  }
  if (ObjCCategoryDecl *CATDecl = dyn_cast<ObjCCategoryDecl>(CDecl)) {
    if (!CATDecl->IsClassExtension())
      for (auto *Prop : CATDecl->properties())
        PropMap[Prop->getIdentifier()] = Prop;
    if (IncludeProtocols) {
      // Scan through class's protocols.
      for (auto *PI : CATDecl->protocols())
        CollectImmediateProperties(PI, PropMap, SuperPropMap);
    }
  }
  else if (ObjCProtocolDecl *PDecl = dyn_cast<ObjCProtocolDecl>(CDecl)) {
    for (auto *Prop : PDecl->properties()) {
      ObjCPropertyDecl *PropertyFromSuper = SuperPropMap[Prop->getIdentifier()];
      // Exclude property for protocols which conform to class's super-class, 
      // as super-class has to implement the property.
      if (!PropertyFromSuper || 
          PropertyFromSuper->getIdentifier() != Prop->getIdentifier()) {
        ObjCPropertyDecl *&PropEntry = PropMap[Prop->getIdentifier()];
        if (!PropEntry)
          PropEntry = Prop;
      }
    }
    // scan through protocol's protocols.
    for (auto *PI : PDecl->protocols())
      CollectImmediateProperties(PI, PropMap, SuperPropMap);
  }
}

/// CollectSuperClassPropertyImplementations - This routine collects list of
/// properties to be implemented in super class(s) and also coming from their
/// conforming protocols.
static void CollectSuperClassPropertyImplementations(ObjCInterfaceDecl *CDecl,
                                    ObjCInterfaceDecl::PropertyMap &PropMap) {
  if (ObjCInterfaceDecl *SDecl = CDecl->getSuperClass()) {
    ObjCInterfaceDecl::PropertyDeclOrder PO;
    while (SDecl) {
      SDecl->collectPropertiesToImplement(PropMap, PO);
      SDecl = SDecl->getSuperClass();
    }
  }
}

/// IvarBacksCurrentMethodAccessor - This routine returns 'true' if 'IV' is
/// an ivar synthesized for 'Method' and 'Method' is a property accessor
/// declared in class 'IFace'.
bool
Sema::IvarBacksCurrentMethodAccessor(ObjCInterfaceDecl *IFace,
                                     ObjCMethodDecl *Method, ObjCIvarDecl *IV) {
  if (!IV->getSynthesize())
    return false;
  ObjCMethodDecl *IMD = IFace->lookupMethod(Method->getSelector(),
                                            Method->isInstanceMethod());
  if (!IMD || !IMD->isPropertyAccessor())
    return false;
  
  // look up a property declaration whose one of its accessors is implemented
  // by this method.
  for (const auto *Property : IFace->properties()) {
    if ((Property->getGetterName() == IMD->getSelector() ||
         Property->getSetterName() == IMD->getSelector()) &&
        (Property->getPropertyIvarDecl() == IV))
      return true;
  }
  return false;
}

static bool SuperClassImplementsProperty(ObjCInterfaceDecl *IDecl,
                                         ObjCPropertyDecl *Prop) {
  bool SuperClassImplementsGetter = false;
  bool SuperClassImplementsSetter = false;
  if (Prop->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_readonly)
    SuperClassImplementsSetter = true;

  while (IDecl->getSuperClass()) {
    ObjCInterfaceDecl *SDecl = IDecl->getSuperClass();
    if (!SuperClassImplementsGetter && SDecl->getInstanceMethod(Prop->getGetterName()))
      SuperClassImplementsGetter = true;

    if (!SuperClassImplementsSetter && SDecl->getInstanceMethod(Prop->getSetterName()))
      SuperClassImplementsSetter = true;
    if (SuperClassImplementsGetter && SuperClassImplementsSetter)
      return true;
    IDecl = IDecl->getSuperClass();
  }
  return false;
}

/// \brief Default synthesizes all properties which must be synthesized
/// in class's \@implementation.
void Sema::DefaultSynthesizeProperties(Scope *S, ObjCImplDecl* IMPDecl,
                                       ObjCInterfaceDecl *IDecl) {
  
  ObjCInterfaceDecl::PropertyMap PropMap;
  ObjCInterfaceDecl::PropertyDeclOrder PropertyOrder;
  IDecl->collectPropertiesToImplement(PropMap, PropertyOrder);
  if (PropMap.empty())
    return;
  ObjCInterfaceDecl::PropertyMap SuperPropMap;
  CollectSuperClassPropertyImplementations(IDecl, SuperPropMap);
  
  for (unsigned i = 0, e = PropertyOrder.size(); i != e; i++) {
    ObjCPropertyDecl *Prop = PropertyOrder[i];
    // Is there a matching property synthesize/dynamic?
    if (Prop->isInvalidDecl() ||
        Prop->getPropertyImplementation() == ObjCPropertyDecl::Optional)
      continue;
    // Property may have been synthesized by user.
    if (IMPDecl->FindPropertyImplDecl(Prop->getIdentifier()))
      continue;
    if (IMPDecl->getInstanceMethod(Prop->getGetterName())) {
      if (Prop->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_readonly)
        continue;
      if (IMPDecl->getInstanceMethod(Prop->getSetterName()))
        continue;
    }
    if (ObjCPropertyImplDecl *PID =
        IMPDecl->FindPropertyImplIvarDecl(Prop->getIdentifier())) {
      Diag(Prop->getLocation(), diag::warn_no_autosynthesis_shared_ivar_property)
        << Prop->getIdentifier();
      if (!PID->getLocation().isInvalid())
        Diag(PID->getLocation(), diag::note_property_synthesize);
      continue;
    }
    ObjCPropertyDecl *PropInSuperClass = SuperPropMap[Prop->getIdentifier()];
    if (ObjCProtocolDecl *Proto =
          dyn_cast<ObjCProtocolDecl>(Prop->getDeclContext())) {
      // We won't auto-synthesize properties declared in protocols.
      // Suppress the warning if class's superclass implements property's
      // getter and implements property's setter (if readwrite property).
      // Or, if property is going to be implemented in its super class.
      if (!SuperClassImplementsProperty(IDecl, Prop) && !PropInSuperClass) {
        Diag(IMPDecl->getLocation(),
             diag::warn_auto_synthesizing_protocol_property)
          << Prop << Proto;
        Diag(Prop->getLocation(), diag::note_property_declare);
      }
      continue;
    }
    // If property to be implemented in the super class, ignore.
    if (PropInSuperClass) {
      if ((Prop->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_readwrite) &&
          (PropInSuperClass->getPropertyAttributes() &
           ObjCPropertyDecl::OBJC_PR_readonly) &&
          !IMPDecl->getInstanceMethod(Prop->getSetterName()) &&
          !IDecl->HasUserDeclaredSetterMethod(Prop)) {
        Diag(Prop->getLocation(), diag::warn_no_autosynthesis_property)
        << Prop->getIdentifier();
        Diag(PropInSuperClass->getLocation(), diag::note_property_declare);
      }
      else {
        Diag(Prop->getLocation(), diag::warn_autosynthesis_property_in_superclass)
        << Prop->getIdentifier();
        Diag(PropInSuperClass->getLocation(), diag::note_property_declare);
        Diag(IMPDecl->getLocation(), diag::note_while_in_implementation);
      }
      continue;
    }
    // We use invalid SourceLocations for the synthesized ivars since they
    // aren't really synthesized at a particular location; they just exist.
    // Saying that they are located at the @implementation isn't really going
    // to help users.
    ObjCPropertyImplDecl *PIDecl = dyn_cast_or_null<ObjCPropertyImplDecl>(
      ActOnPropertyImplDecl(S, SourceLocation(), SourceLocation(),
                            true,
                            /* property = */ Prop->getIdentifier(),
                            /* ivar = */ Prop->getDefaultSynthIvarName(Context),
                            Prop->getLocation()));
    if (PIDecl) {
      Diag(Prop->getLocation(), diag::warn_missing_explicit_synthesis);
      Diag(IMPDecl->getLocation(), diag::note_while_in_implementation);
    }
  }
}

void Sema::DefaultSynthesizeProperties(Scope *S, Decl *D) {
  if (!LangOpts.ObjCDefaultSynthProperties || LangOpts.ObjCRuntime.isFragile())
    return;
  ObjCImplementationDecl *IC=dyn_cast_or_null<ObjCImplementationDecl>(D);
  if (!IC)
    return;
  if (ObjCInterfaceDecl* IDecl = IC->getClassInterface())
    if (!IDecl->isObjCRequiresPropertyDefs())
      DefaultSynthesizeProperties(S, IC, IDecl);
}

static void DiagnoseUnimplementedAccessor(Sema &S,
                                          ObjCInterfaceDecl *PrimaryClass,
                                          Selector Method,
                                          ObjCImplDecl* IMPDecl,
                                          ObjCContainerDecl *CDecl,
                                          ObjCCategoryDecl *C,
                                          ObjCPropertyDecl *Prop,
                                          Sema::SelectorSet &SMap) {
  // When reporting on missing property setter/getter implementation in
  // categories, do not report when they are declared in primary class,
  // class's protocol, or one of it super classes. This is because,
  // the class is going to implement them.
  if (!SMap.count(Method) &&
      (PrimaryClass == nullptr ||
       !PrimaryClass->lookupPropertyAccessor(Method, C))) {
        S.Diag(IMPDecl->getLocation(),
               isa<ObjCCategoryDecl>(CDecl) ?
               diag::warn_setter_getter_impl_required_in_category :
               diag::warn_setter_getter_impl_required)
            << Prop->getDeclName() << Method;
        S.Diag(Prop->getLocation(),
             diag::note_property_declare);
        if (S.LangOpts.ObjCDefaultSynthProperties &&
            S.LangOpts.ObjCRuntime.isNonFragile())
          if (ObjCInterfaceDecl *ID = dyn_cast<ObjCInterfaceDecl>(CDecl))
            if (const ObjCInterfaceDecl *RID = ID->isObjCRequiresPropertyDefs())
            S.Diag(RID->getLocation(), diag::note_suppressed_class_declare);
      }
}

void Sema::DiagnoseUnimplementedProperties(Scope *S, ObjCImplDecl* IMPDecl,
                                           ObjCContainerDecl *CDecl,
                                           bool SynthesizeProperties) {
  ObjCContainerDecl::PropertyMap PropMap;
  ObjCInterfaceDecl *IDecl = dyn_cast<ObjCInterfaceDecl>(CDecl);

  if (!SynthesizeProperties) {
    ObjCContainerDecl::PropertyMap NoNeedToImplPropMap;
    // Gather properties which need not be implemented in this class
    // or category.
    if (!IDecl)
      if (ObjCCategoryDecl *C = dyn_cast<ObjCCategoryDecl>(CDecl)) {
        // For categories, no need to implement properties declared in
        // its primary class (and its super classes) if property is
        // declared in one of those containers.
        if ((IDecl = C->getClassInterface())) {
          ObjCInterfaceDecl::PropertyDeclOrder PO;
          IDecl->collectPropertiesToImplement(NoNeedToImplPropMap, PO);
        }
      }
    if (IDecl)
      CollectSuperClassPropertyImplementations(IDecl, NoNeedToImplPropMap);
    
    CollectImmediateProperties(CDecl, PropMap, NoNeedToImplPropMap);
  }

  // Scan the @interface to see if any of the protocols it adopts
  // require an explicit implementation, via attribute
  // 'objc_protocol_requires_explicit_implementation'.
  if (IDecl) {
    std::unique_ptr<ObjCContainerDecl::PropertyMap> LazyMap;

    for (auto *PDecl : IDecl->all_referenced_protocols()) {
      if (!PDecl->hasAttr<ObjCExplicitProtocolImplAttr>())
        continue;
      // Lazily construct a set of all the properties in the @interface
      // of the class, without looking at the superclass.  We cannot
      // use the call to CollectImmediateProperties() above as that
      // utilizes information from the super class's properties as well
      // as scans the adopted protocols.  This work only triggers for protocols
      // with the attribute, which is very rare, and only occurs when
      // analyzing the @implementation.
      if (!LazyMap) {
        ObjCContainerDecl::PropertyMap NoNeedToImplPropMap;
        LazyMap.reset(new ObjCContainerDecl::PropertyMap());
        CollectImmediateProperties(CDecl, *LazyMap, NoNeedToImplPropMap,
                                   /* IncludeProtocols */ false);
      }
      // Add the properties of 'PDecl' to the list of properties that
      // need to be implemented.
      for (auto *PropDecl : PDecl->properties()) {
        if ((*LazyMap)[PropDecl->getIdentifier()])
          continue;
        PropMap[PropDecl->getIdentifier()] = PropDecl;
      }
    }
  }

  if (PropMap.empty())
    return;

  llvm::DenseSet<ObjCPropertyDecl *> PropImplMap;
  for (const auto *I : IMPDecl->property_impls())
    PropImplMap.insert(I->getPropertyDecl());

  SelectorSet InsMap;
  // Collect property accessors implemented in current implementation.
  for (const auto *I : IMPDecl->instance_methods())
    InsMap.insert(I->getSelector());
  
  ObjCCategoryDecl *C = dyn_cast<ObjCCategoryDecl>(CDecl);
  ObjCInterfaceDecl *PrimaryClass = nullptr;
  if (C && !C->IsClassExtension())
    if ((PrimaryClass = C->getClassInterface()))
      // Report unimplemented properties in the category as well.
      if (ObjCImplDecl *IMP = PrimaryClass->getImplementation()) {
        // When reporting on missing setter/getters, do not report when
        // setter/getter is implemented in category's primary class
        // implementation.
        for (const auto *I : IMP->instance_methods())
          InsMap.insert(I->getSelector());
      }

  for (ObjCContainerDecl::PropertyMap::iterator
       P = PropMap.begin(), E = PropMap.end(); P != E; ++P) {
    ObjCPropertyDecl *Prop = P->second;
    // Is there a matching propery synthesize/dynamic?
    if (Prop->isInvalidDecl() ||
        Prop->getPropertyImplementation() == ObjCPropertyDecl::Optional ||
        PropImplMap.count(Prop) ||
        Prop->getAvailability() == AR_Unavailable)
      continue;

    // Diagnose unimplemented getters and setters.
    DiagnoseUnimplementedAccessor(*this,
          PrimaryClass, Prop->getGetterName(), IMPDecl, CDecl, C, Prop, InsMap);
    if (!Prop->isReadOnly())
      DiagnoseUnimplementedAccessor(*this,
                                    PrimaryClass, Prop->getSetterName(),
                                    IMPDecl, CDecl, C, Prop, InsMap);
  }
}

void Sema::diagnoseNullResettableSynthesizedSetters(const ObjCImplDecl *impDecl) {
  for (const auto *propertyImpl : impDecl->property_impls()) {
    const auto *property = propertyImpl->getPropertyDecl();

    // Warn about null_resettable properties with synthesized setters,
    // because the setter won't properly handle nil.
    if (propertyImpl->getPropertyImplementation()
          == ObjCPropertyImplDecl::Synthesize &&
        (property->getPropertyAttributes() &
         ObjCPropertyDecl::OBJC_PR_null_resettable) &&
        property->getGetterMethodDecl() &&
        property->getSetterMethodDecl()) {
      auto *getterMethod = property->getGetterMethodDecl();
      auto *setterMethod = property->getSetterMethodDecl();
      if (!impDecl->getInstanceMethod(setterMethod->getSelector()) &&
          !impDecl->getInstanceMethod(getterMethod->getSelector())) {
        SourceLocation loc = propertyImpl->getLocation();
        if (loc.isInvalid())
          loc = impDecl->getLocStart();

        Diag(loc, diag::warn_null_resettable_setter)
          << setterMethod->getSelector() << property->getDeclName();
      }
    }
  }
}

void
Sema::AtomicPropertySetterGetterRules (ObjCImplDecl* IMPDecl,
                                       ObjCContainerDecl* IDecl) {
  // Rules apply in non-GC mode only
  if (getLangOpts().getGC() != LangOptions::NonGC)
    return;
  for (const auto *Property : IDecl->properties()) {
    ObjCMethodDecl *GetterMethod = nullptr;
    ObjCMethodDecl *SetterMethod = nullptr;
    bool LookedUpGetterSetter = false;

    unsigned Attributes = Property->getPropertyAttributes();
    unsigned AttributesAsWritten = Property->getPropertyAttributesAsWritten();

    if (!(AttributesAsWritten & ObjCPropertyDecl::OBJC_PR_atomic) &&
        !(AttributesAsWritten & ObjCPropertyDecl::OBJC_PR_nonatomic)) {
      GetterMethod = IMPDecl->getInstanceMethod(Property->getGetterName());
      SetterMethod = IMPDecl->getInstanceMethod(Property->getSetterName());
      LookedUpGetterSetter = true;
      if (GetterMethod) {
        Diag(GetterMethod->getLocation(),
             diag::warn_default_atomic_custom_getter_setter)
          << Property->getIdentifier() << 0;
        Diag(Property->getLocation(), diag::note_property_declare);
      }
      if (SetterMethod) {
        Diag(SetterMethod->getLocation(),
             diag::warn_default_atomic_custom_getter_setter)
          << Property->getIdentifier() << 1;
        Diag(Property->getLocation(), diag::note_property_declare);
      }
    }

    // We only care about readwrite atomic property.
    if ((Attributes & ObjCPropertyDecl::OBJC_PR_nonatomic) ||
        !(Attributes & ObjCPropertyDecl::OBJC_PR_readwrite))
      continue;
    if (const ObjCPropertyImplDecl *PIDecl
         = IMPDecl->FindPropertyImplDecl(Property->getIdentifier())) {
      if (PIDecl->getPropertyImplementation() == ObjCPropertyImplDecl::Dynamic)
        continue;
      if (!LookedUpGetterSetter) {
        GetterMethod = IMPDecl->getInstanceMethod(Property->getGetterName());
        SetterMethod = IMPDecl->getInstanceMethod(Property->getSetterName());
      }
      if ((GetterMethod && !SetterMethod) || (!GetterMethod && SetterMethod)) {
        SourceLocation MethodLoc =
          (GetterMethod ? GetterMethod->getLocation()
                        : SetterMethod->getLocation());
        Diag(MethodLoc, diag::warn_atomic_property_rule)
          << Property->getIdentifier() << (GetterMethod != nullptr)
          << (SetterMethod != nullptr);
        // fixit stuff.
        if (!AttributesAsWritten) {
          if (Property->getLParenLoc().isValid()) {
            // @property () ... case.
            SourceRange PropSourceRange(Property->getAtLoc(), 
                                        Property->getLParenLoc());
            Diag(Property->getLocation(), diag::note_atomic_property_fixup_suggest) <<
              FixItHint::CreateReplacement(PropSourceRange, "@property (nonatomic");
          }
          else {
            //@property id etc.
            SourceLocation endLoc = 
              Property->getTypeSourceInfo()->getTypeLoc().getBeginLoc();
            endLoc = endLoc.getLocWithOffset(-1);
            SourceRange PropSourceRange(Property->getAtLoc(), endLoc);
            Diag(Property->getLocation(), diag::note_atomic_property_fixup_suggest) <<
              FixItHint::CreateReplacement(PropSourceRange, "@property (nonatomic) ");
          }
        }
        else if (!(AttributesAsWritten & ObjCPropertyDecl::OBJC_PR_atomic)) {
          // @property () ... case.
          SourceLocation endLoc = Property->getLParenLoc();
          SourceRange PropSourceRange(Property->getAtLoc(), endLoc);
          Diag(Property->getLocation(), diag::note_atomic_property_fixup_suggest) <<
           FixItHint::CreateReplacement(PropSourceRange, "@property (nonatomic, ");
        }
        else
          Diag(MethodLoc, diag::note_atomic_property_fixup_suggest);
        Diag(Property->getLocation(), diag::note_property_declare);
      }
    }
  }
}

void Sema::DiagnoseOwningPropertyGetterSynthesis(const ObjCImplementationDecl *D) {
  if (getLangOpts().getGC() == LangOptions::GCOnly)
    return;

  for (const auto *PID : D->property_impls()) {
    const ObjCPropertyDecl *PD = PID->getPropertyDecl();
    if (PD && !PD->hasAttr<NSReturnsNotRetainedAttr>() &&
        !D->getInstanceMethod(PD->getGetterName())) {
      ObjCMethodDecl *method = PD->getGetterMethodDecl();
      if (!method)
        continue;
      ObjCMethodFamily family = method->getMethodFamily();
      if (family == OMF_alloc || family == OMF_copy ||
          family == OMF_mutableCopy || family == OMF_new) {
        if (getLangOpts().ObjCAutoRefCount)
          Diag(PD->getLocation(), diag::err_cocoa_naming_owned_rule);
        else
          Diag(PD->getLocation(), diag::warn_cocoa_naming_owned_rule);

        // Look for a getter explicitly declared alongside the property.
        // If we find one, use its location for the note.
        SourceLocation noteLoc = PD->getLocation();
        SourceLocation fixItLoc;
        for (auto *getterRedecl : method->redecls()) {
          if (getterRedecl->isImplicit())
            continue;
          if (getterRedecl->getDeclContext() != PD->getDeclContext())
            continue;
          noteLoc = getterRedecl->getLocation();
          fixItLoc = getterRedecl->getLocEnd();
        }

        Preprocessor &PP = getPreprocessor();
        TokenValue tokens[] = {
          tok::kw___attribute, tok::l_paren, tok::l_paren,
          PP.getIdentifierInfo("objc_method_family"), tok::l_paren,
          PP.getIdentifierInfo("none"), tok::r_paren,
          tok::r_paren, tok::r_paren
        };
        StringRef spelling = "__attribute__((objc_method_family(none)))";
        StringRef macroName = PP.getLastMacroWithSpelling(noteLoc, tokens);
        if (!macroName.empty())
          spelling = macroName;

        auto noteDiag = Diag(noteLoc, diag::note_cocoa_naming_declare_family)
            << method->getDeclName() << spelling;
        if (fixItLoc.isValid()) {
          SmallString<64> fixItText(" ");
          fixItText += spelling;
          noteDiag << FixItHint::CreateInsertion(fixItLoc, fixItText);
        }
      }
    }
  }
}

void Sema::DiagnoseMissingDesignatedInitOverrides(
                                            const ObjCImplementationDecl *ImplD,
                                            const ObjCInterfaceDecl *IFD) {
  assert(IFD->hasDesignatedInitializers());
  const ObjCInterfaceDecl *SuperD = IFD->getSuperClass();
  if (!SuperD)
    return;

  SelectorSet InitSelSet;
  for (const auto *I : ImplD->instance_methods())
    if (I->getMethodFamily() == OMF_init)
      InitSelSet.insert(I->getSelector());

  SmallVector<const ObjCMethodDecl *, 8> DesignatedInits;
  SuperD->getDesignatedInitializers(DesignatedInits);
  for (SmallVector<const ObjCMethodDecl *, 8>::iterator
         I = DesignatedInits.begin(), E = DesignatedInits.end(); I != E; ++I) {
    const ObjCMethodDecl *MD = *I;
    if (!InitSelSet.count(MD->getSelector())) {
      Diag(ImplD->getLocation(),
           diag::warn_objc_implementation_missing_designated_init_override)
        << MD->getSelector();
      Diag(MD->getLocation(), diag::note_objc_designated_init_marked_here);
    }
  }
}

/// AddPropertyAttrs - Propagates attributes from a property to the
/// implicitly-declared getter or setter for that property.
static void AddPropertyAttrs(Sema &S, ObjCMethodDecl *PropertyMethod,
                             ObjCPropertyDecl *Property) {
  // Should we just clone all attributes over?
  for (const auto *A : Property->attrs()) {
    if (isa<DeprecatedAttr>(A) || 
        isa<UnavailableAttr>(A) || 
        isa<AvailabilityAttr>(A))
      PropertyMethod->addAttr(A->clone(S.Context));
  }
}

/// ProcessPropertyDecl - Make sure that any user-defined setter/getter methods
/// have the property type and issue diagnostics if they don't.
/// Also synthesize a getter/setter method if none exist (and update the
/// appropriate lookup tables. FIXME: Should reconsider if adding synthesized
/// methods is the "right" thing to do.
void Sema::ProcessPropertyDecl(ObjCPropertyDecl *property,
                               ObjCContainerDecl *CD,
                               ObjCPropertyDecl *redeclaredProperty,
                               ObjCContainerDecl *lexicalDC) {

  ObjCMethodDecl *GetterMethod, *SetterMethod;

  if (CD->isInvalidDecl())
    return;

  GetterMethod = CD->getInstanceMethod(property->getGetterName());
  SetterMethod = CD->getInstanceMethod(property->getSetterName());
  DiagnosePropertyAccessorMismatch(property, GetterMethod,
                                   property->getLocation());

  if (SetterMethod) {
    ObjCPropertyDecl::PropertyAttributeKind CAttr =
      property->getPropertyAttributes();
    if ((!(CAttr & ObjCPropertyDecl::OBJC_PR_readonly)) &&
        Context.getCanonicalType(SetterMethod->getReturnType()) !=
            Context.VoidTy)
      Diag(SetterMethod->getLocation(), diag::err_setter_type_void);
    if (SetterMethod->param_size() != 1 ||
        !Context.hasSameUnqualifiedType(
          (*SetterMethod->param_begin())->getType().getNonReferenceType(), 
          property->getType().getNonReferenceType())) {
      Diag(property->getLocation(),
           diag::warn_accessor_property_type_mismatch)
        << property->getDeclName()
        << SetterMethod->getSelector();
      Diag(SetterMethod->getLocation(), diag::note_declared_at);
    }
  }

  // Synthesize getter/setter methods if none exist.
  // Find the default getter and if one not found, add one.
  // FIXME: The synthesized property we set here is misleading. We almost always
  // synthesize these methods unless the user explicitly provided prototypes
  // (which is odd, but allowed). Sema should be typechecking that the
  // declarations jive in that situation (which it is not currently).
  if (!GetterMethod) {
    // No instance method of same name as property getter name was found.
    // Declare a getter method and add it to the list of methods
    // for this class.
    SourceLocation Loc = redeclaredProperty ? 
      redeclaredProperty->getLocation() :
      property->getLocation();

    // If the property is null_resettable, the getter returns nonnull.
    QualType resultTy = property->getType();
    if (property->getPropertyAttributes() &
        ObjCPropertyDecl::OBJC_PR_null_resettable) {
      QualType modifiedTy = resultTy;
      if (auto nullability = AttributedType::stripOuterNullability(modifiedTy)) {
        if (*nullability == NullabilityKind::Unspecified)
          resultTy = Context.getAttributedType(AttributedType::attr_nonnull,
                                               modifiedTy, modifiedTy);
      }
    }

    GetterMethod = ObjCMethodDecl::Create(Context, Loc, Loc,
                             property->getGetterName(),
                             resultTy, nullptr, CD,
                             /*isInstance=*/true, /*isVariadic=*/false,
                             /*isPropertyAccessor=*/true,
                             /*isImplicitlyDeclared=*/true, /*isDefined=*/false,
                             (property->getPropertyImplementation() ==
                              ObjCPropertyDecl::Optional) ?
                             ObjCMethodDecl::Optional :
                             ObjCMethodDecl::Required);
    CD->addDecl(GetterMethod);

    AddPropertyAttrs(*this, GetterMethod, property);

    // FIXME: Eventually this shouldn't be needed, as the lexical context
    // and the real context should be the same.
    if (lexicalDC)
      GetterMethod->setLexicalDeclContext(lexicalDC);
    if (property->hasAttr<NSReturnsNotRetainedAttr>())
      GetterMethod->addAttr(NSReturnsNotRetainedAttr::CreateImplicit(Context,
                                                                     Loc));
    
    if (property->hasAttr<ObjCReturnsInnerPointerAttr>())
      GetterMethod->addAttr(
        ObjCReturnsInnerPointerAttr::CreateImplicit(Context, Loc));
    
    if (const SectionAttr *SA = property->getAttr<SectionAttr>())
      GetterMethod->addAttr(
          SectionAttr::CreateImplicit(Context, SectionAttr::GNU_section,
                                      SA->getName(), Loc));

    if (getLangOpts().ObjCAutoRefCount)
      CheckARCMethodDecl(GetterMethod);
  } else
    // A user declared getter will be synthesize when @synthesize of
    // the property with the same name is seen in the @implementation
    GetterMethod->setPropertyAccessor(true);
  property->setGetterMethodDecl(GetterMethod);

  // Skip setter if property is read-only.
  if (!property->isReadOnly()) {
    // Find the default setter and if one not found, add one.
    if (!SetterMethod) {
      // No instance method of same name as property setter name was found.
      // Declare a setter method and add it to the list of methods
      // for this class.
      SourceLocation Loc = redeclaredProperty ? 
        redeclaredProperty->getLocation() :
        property->getLocation();

      SetterMethod =
        ObjCMethodDecl::Create(Context, Loc, Loc,
                               property->getSetterName(), Context.VoidTy,
                               nullptr, CD, /*isInstance=*/true,
                               /*isVariadic=*/false,
                               /*isPropertyAccessor=*/true,
                               /*isImplicitlyDeclared=*/true,
                               /*isDefined=*/false,
                               (property->getPropertyImplementation() ==
                                ObjCPropertyDecl::Optional) ?
                                ObjCMethodDecl::Optional :
                                ObjCMethodDecl::Required);

      // If the property is null_resettable, the setter accepts a
      // nullable value.
      QualType paramTy = property->getType().getUnqualifiedType();
      if (property->getPropertyAttributes() &
          ObjCPropertyDecl::OBJC_PR_null_resettable) {
        QualType modifiedTy = paramTy;
        if (auto nullability = AttributedType::stripOuterNullability(modifiedTy)){
          if (*nullability == NullabilityKind::Unspecified)
            paramTy = Context.getAttributedType(AttributedType::attr_nullable,
                                                modifiedTy, modifiedTy);
        }
      }

      // Invent the arguments for the setter. We don't bother making a
      // nice name for the argument.
      ParmVarDecl *Argument = ParmVarDecl::Create(Context, SetterMethod,
                                                  Loc, Loc,
                                                  property->getIdentifier(),
                                                  paramTy,
                                                  /*TInfo=*/nullptr,
                                                  SC_None,
                                                  nullptr);
      SetterMethod->setMethodParams(Context, Argument, None);

      AddPropertyAttrs(*this, SetterMethod, property);

      CD->addDecl(SetterMethod);
      // FIXME: Eventually this shouldn't be needed, as the lexical context
      // and the real context should be the same.
      if (lexicalDC)
        SetterMethod->setLexicalDeclContext(lexicalDC);
      if (const SectionAttr *SA = property->getAttr<SectionAttr>())
        SetterMethod->addAttr(
            SectionAttr::CreateImplicit(Context, SectionAttr::GNU_section,
                                        SA->getName(), Loc));
      // It's possible for the user to have set a very odd custom
      // setter selector that causes it to have a method family.
      if (getLangOpts().ObjCAutoRefCount)
        CheckARCMethodDecl(SetterMethod);
    } else
      // A user declared setter will be synthesize when @synthesize of
      // the property with the same name is seen in the @implementation
      SetterMethod->setPropertyAccessor(true);
    property->setSetterMethodDecl(SetterMethod);
  }
  // Add any synthesized methods to the global pool. This allows us to
  // handle the following, which is supported by GCC (and part of the design).
  //
  // @interface Foo
  // @property double bar;
  // @end
  //
  // void thisIsUnfortunate() {
  //   id foo;
  //   double bar = [foo bar];
  // }
  //
  if (GetterMethod)
    AddInstanceMethodToGlobalPool(GetterMethod);
  if (SetterMethod)
    AddInstanceMethodToGlobalPool(SetterMethod);

  ObjCInterfaceDecl *CurrentClass = dyn_cast<ObjCInterfaceDecl>(CD);
  if (!CurrentClass) {
    if (ObjCCategoryDecl *Cat = dyn_cast<ObjCCategoryDecl>(CD))
      CurrentClass = Cat->getClassInterface();
    else if (ObjCImplDecl *Impl = dyn_cast<ObjCImplDecl>(CD))
      CurrentClass = Impl->getClassInterface();
  }
  if (GetterMethod)
    CheckObjCMethodOverrides(GetterMethod, CurrentClass, Sema::RTC_Unknown);
  if (SetterMethod)
    CheckObjCMethodOverrides(SetterMethod, CurrentClass, Sema::RTC_Unknown);
}

void Sema::CheckObjCPropertyAttributes(Decl *PDecl,
                                       SourceLocation Loc,
                                       unsigned &Attributes,
                                       bool propertyInPrimaryClass) {
  // FIXME: Improve the reported location.
  if (!PDecl || PDecl->isInvalidDecl())
    return;
  
  if ((Attributes & ObjCDeclSpec::DQ_PR_readonly) &&
      (Attributes & ObjCDeclSpec::DQ_PR_readwrite))
    Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
    << "readonly" << "readwrite";
  
  ObjCPropertyDecl *PropertyDecl = cast<ObjCPropertyDecl>(PDecl);
  QualType PropertyTy = PropertyDecl->getType();
  unsigned PropertyOwnership = getOwnershipRule(Attributes);

  // 'readonly' property with no obvious lifetime.
  // its life time will be determined by its backing ivar.
  if (getLangOpts().ObjCAutoRefCount &&
      Attributes & ObjCDeclSpec::DQ_PR_readonly &&
      PropertyTy->isObjCRetainableType() &&
      !PropertyOwnership)
    return;

  // Check for copy or retain on non-object types.
  if ((Attributes & (ObjCDeclSpec::DQ_PR_weak | ObjCDeclSpec::DQ_PR_copy |
                    ObjCDeclSpec::DQ_PR_retain | ObjCDeclSpec::DQ_PR_strong)) &&
      !PropertyTy->isObjCRetainableType() &&
      !PropertyDecl->hasAttr<ObjCNSObjectAttr>()) {
    Diag(Loc, diag::err_objc_property_requires_object)
      << (Attributes & ObjCDeclSpec::DQ_PR_weak ? "weak" :
          Attributes & ObjCDeclSpec::DQ_PR_copy ? "copy" : "retain (or strong)");
    Attributes &= ~(ObjCDeclSpec::DQ_PR_weak   | ObjCDeclSpec::DQ_PR_copy |
                    ObjCDeclSpec::DQ_PR_retain | ObjCDeclSpec::DQ_PR_strong);
    PropertyDecl->setInvalidDecl();
  }

  // Check for more than one of { assign, copy, retain }.
  if (Attributes & ObjCDeclSpec::DQ_PR_assign) {
    if (Attributes & ObjCDeclSpec::DQ_PR_copy) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "assign" << "copy";
      Attributes &= ~ObjCDeclSpec::DQ_PR_copy;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_retain) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "assign" << "retain";
      Attributes &= ~ObjCDeclSpec::DQ_PR_retain;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_strong) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "assign" << "strong";
      Attributes &= ~ObjCDeclSpec::DQ_PR_strong;
    }
    if (getLangOpts().ObjCAutoRefCount  &&
        (Attributes & ObjCDeclSpec::DQ_PR_weak)) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "assign" << "weak";
      Attributes &= ~ObjCDeclSpec::DQ_PR_weak;
    }
    if (PropertyDecl->hasAttr<IBOutletCollectionAttr>())
      Diag(Loc, diag::warn_iboutletcollection_property_assign);
  } else if (Attributes & ObjCDeclSpec::DQ_PR_unsafe_unretained) {
    if (Attributes & ObjCDeclSpec::DQ_PR_copy) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "unsafe_unretained" << "copy";
      Attributes &= ~ObjCDeclSpec::DQ_PR_copy;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_retain) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "unsafe_unretained" << "retain";
      Attributes &= ~ObjCDeclSpec::DQ_PR_retain;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_strong) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "unsafe_unretained" << "strong";
      Attributes &= ~ObjCDeclSpec::DQ_PR_strong;
    }
    if (getLangOpts().ObjCAutoRefCount  &&
        (Attributes & ObjCDeclSpec::DQ_PR_weak)) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "unsafe_unretained" << "weak";
      Attributes &= ~ObjCDeclSpec::DQ_PR_weak;
    }
  } else if (Attributes & ObjCDeclSpec::DQ_PR_copy) {
    if (Attributes & ObjCDeclSpec::DQ_PR_retain) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "copy" << "retain";
      Attributes &= ~ObjCDeclSpec::DQ_PR_retain;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_strong) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "copy" << "strong";
      Attributes &= ~ObjCDeclSpec::DQ_PR_strong;
    }
    if (Attributes & ObjCDeclSpec::DQ_PR_weak) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "copy" << "weak";
      Attributes &= ~ObjCDeclSpec::DQ_PR_weak;
    }
  }
  else if ((Attributes & ObjCDeclSpec::DQ_PR_retain) &&
           (Attributes & ObjCDeclSpec::DQ_PR_weak)) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "retain" << "weak";
      Attributes &= ~ObjCDeclSpec::DQ_PR_retain;
  }
  else if ((Attributes & ObjCDeclSpec::DQ_PR_strong) &&
           (Attributes & ObjCDeclSpec::DQ_PR_weak)) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "strong" << "weak";
      Attributes &= ~ObjCDeclSpec::DQ_PR_weak;
  }

  if (Attributes & ObjCDeclSpec::DQ_PR_weak) {
    // 'weak' and 'nonnull' are mutually exclusive.
    if (auto nullability = PropertyTy->getNullability(Context)) {
      if (*nullability == NullabilityKind::NonNull)
        Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
          << "nonnull" << "weak";
    } else {
        PropertyTy =
          Context.getAttributedType(
            AttributedType::getNullabilityAttrKind(NullabilityKind::Nullable),
            PropertyTy, PropertyTy);
        TypeSourceInfo *TSInfo = PropertyDecl->getTypeSourceInfo();
        PropertyDecl->setType(PropertyTy, TSInfo);
    }
  }

  if ((Attributes & ObjCDeclSpec::DQ_PR_atomic) &&
      (Attributes & ObjCDeclSpec::DQ_PR_nonatomic)) {
      Diag(Loc, diag::err_objc_property_attr_mutually_exclusive)
        << "atomic" << "nonatomic";
      Attributes &= ~ObjCDeclSpec::DQ_PR_atomic;
  }

  // Warn if user supplied no assignment attribute, property is
  // readwrite, and this is an object type.
  if (!(Attributes & (ObjCDeclSpec::DQ_PR_assign | ObjCDeclSpec::DQ_PR_copy |
                      ObjCDeclSpec::DQ_PR_unsafe_unretained |
                      ObjCDeclSpec::DQ_PR_retain | ObjCDeclSpec::DQ_PR_strong |
                      ObjCDeclSpec::DQ_PR_weak)) &&
      PropertyTy->isObjCObjectPointerType()) {
      if (getLangOpts().ObjCAutoRefCount)
        // With arc,  @property definitions should default to (strong) when 
        // not specified; including when property is 'readonly'.
        PropertyDecl->setPropertyAttributes(ObjCPropertyDecl::OBJC_PR_strong);
      else if (!(Attributes & ObjCDeclSpec::DQ_PR_readonly)) {
        bool isAnyClassTy = 
          (PropertyTy->isObjCClassType() || 
           PropertyTy->isObjCQualifiedClassType());
        // In non-gc, non-arc mode, 'Class' is treated as a 'void *' no need to
        // issue any warning.
        if (isAnyClassTy && getLangOpts().getGC() == LangOptions::NonGC)
          ;
        else if (propertyInPrimaryClass) {
          // Don't issue warning on property with no life time in class 
          // extension as it is inherited from property in primary class.
          // Skip this warning in gc-only mode.
          if (getLangOpts().getGC() != LangOptions::GCOnly)
            Diag(Loc, diag::warn_objc_property_no_assignment_attribute);

          // If non-gc code warn that this is likely inappropriate.
          if (getLangOpts().getGC() == LangOptions::NonGC)
            Diag(Loc, diag::warn_objc_property_default_assign_on_object);
        }
      }

    // FIXME: Implement warning dependent on NSCopying being
    // implemented. See also:
    // <rdar://5168496&4855821&5607453&5096644&4947311&5698469&4947014&5168496>
    // (please trim this list while you are at it).
  }

  if (!(Attributes & ObjCDeclSpec::DQ_PR_copy)
      &&!(Attributes & ObjCDeclSpec::DQ_PR_readonly)
      && getLangOpts().getGC() == LangOptions::GCOnly
      && PropertyTy->isBlockPointerType())
    Diag(Loc, diag::warn_objc_property_copy_missing_on_block);
  else if ((Attributes & ObjCDeclSpec::DQ_PR_retain) &&
           !(Attributes & ObjCDeclSpec::DQ_PR_readonly) &&
           !(Attributes & ObjCDeclSpec::DQ_PR_strong) &&
           PropertyTy->isBlockPointerType())
      Diag(Loc, diag::warn_objc_property_retain_of_block);
  
  if ((Attributes & ObjCDeclSpec::DQ_PR_readonly) &&
      (Attributes & ObjCDeclSpec::DQ_PR_setter))
    Diag(Loc, diag::warn_objc_readonly_property_has_setter);
      
}

#endif // HLSL Change
