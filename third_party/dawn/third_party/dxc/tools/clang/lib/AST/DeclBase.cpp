//===--- DeclBase.cpp - Declaration AST Node Implementation ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl and DeclContext classes.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/DeclBase.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclContextInternals.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclOpenMP.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/DependentDiagnostic.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/Type.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

#include "llvm/Support/OacrIgnoreCond.h" // HLSL Change - options change visibility rules

using namespace clang;

//===----------------------------------------------------------------------===//
//  Statistics
//===----------------------------------------------------------------------===//

#define DECL(DERIVED, BASE) static int n##DERIVED##s = 0;
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"

void Decl::updateOutOfDate(IdentifierInfo &II) const {
  getASTContext().getExternalSource()->updateOutOfDateIdentifier(II);
}

void *Decl::operator new(std::size_t Size, const ASTContext &Context,
                         unsigned ID, std::size_t Extra) {
  // Allocate an extra 8 bytes worth of storage, which ensures that the
  // resulting pointer will still be 8-byte aligned. 
  void *Start = Context.Allocate(Size + Extra + 8);
  void *Result = (char*)Start + 8;

  unsigned *PrefixPtr = (unsigned *)Result - 2;

  // Zero out the first 4 bytes; this is used to store the owning module ID.
  PrefixPtr[0] = 0;

  // Store the global declaration ID in the second 4 bytes.
  PrefixPtr[1] = ID;

  return Result;
}

void *Decl::operator new(std::size_t Size, const ASTContext &Ctx,
                         DeclContext *Parent, std::size_t Extra) {
  assert(!Parent || &Parent->getParentASTContext() == &Ctx);
  // With local visibility enabled, we track the owning module even for local
  // declarations.
  if (Ctx.getLangOpts().ModulesLocalVisibility) {
    void *Buffer = ::operator new(sizeof(Module *) + Size + Extra, Ctx);
    return new (Buffer) Module*(nullptr) + 1;
  }
  return ::operator new(Size + Extra, Ctx);
}

Module *Decl::getOwningModuleSlow() const {
  assert(isFromASTFile() && "Not from AST file?");
  return getASTContext().getExternalSource()->getModule(getOwningModuleID());
}

bool Decl::hasLocalOwningModuleStorage() const {
  return getASTContext().getLangOpts().ModulesLocalVisibility;
}

const char *Decl::getDeclKindName() const {
  switch (DeclKind) {
  default: llvm_unreachable("Declaration not in DeclNodes.inc!");
#define DECL(DERIVED, BASE) case DERIVED: return #DERIVED;
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"
  }
}

void Decl::setInvalidDecl(bool Invalid) {
  InvalidDecl = Invalid;
  assert(!isa<TagDecl>(this) || !cast<TagDecl>(this)->isCompleteDefinition());
  if (Invalid && !isa<ParmVarDecl>(this)) {
    // Defensive maneuver for ill-formed code: we're likely not to make it to
    // a point where we set the access specifier, so default it to "public"
    // to avoid triggering asserts elsewhere in the front end. 
    setAccess(AS_public);
  }
}

const char *DeclContext::getDeclKindName() const {
  switch (DeclKind) {
  default: llvm_unreachable("Declaration context not in DeclNodes.inc!");
#define DECL(DERIVED, BASE) case Decl::DERIVED: return #DERIVED;
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"
  }
}

bool Decl::StatisticsEnabled = false;
void Decl::EnableStatistics() {
  StatisticsEnabled = true;
}

void Decl::PrintStats() {
  llvm::errs() << "\n*** Decl Stats:\n";

  int totalDecls = 0;
#define DECL(DERIVED, BASE) totalDecls += n##DERIVED##s;
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"
  llvm::errs() << "  " << totalDecls << " decls total.\n";

  int totalBytes = 0;
#define DECL(DERIVED, BASE)                                             \
  if (n##DERIVED##s > 0) {                                              \
    totalBytes += (int)(n##DERIVED##s * sizeof(DERIVED##Decl));         \
    llvm::errs() << "    " << n##DERIVED##s << " " #DERIVED " decls, "  \
                 << sizeof(DERIVED##Decl) << " each ("                  \
                 << n##DERIVED##s * sizeof(DERIVED##Decl)               \
                 << " bytes)\n";                                        \
  }
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"

  llvm::errs() << "Total bytes = " << totalBytes << "\n";
}

void Decl::add(Kind k) {
  switch (k) {
#define DECL(DERIVED, BASE) case DERIVED: ++n##DERIVED##s; break;
#define ABSTRACT_DECL(DECL)
#include "clang/AST/DeclNodes.inc"
  }
}

bool Decl::isTemplateParameterPack() const {
  if (const TemplateTypeParmDecl *TTP = dyn_cast<TemplateTypeParmDecl>(this))
    return TTP->isParameterPack();
  if (const NonTypeTemplateParmDecl *NTTP
                                = dyn_cast<NonTypeTemplateParmDecl>(this))
    return NTTP->isParameterPack();
  if (const TemplateTemplateParmDecl *TTP
                                    = dyn_cast<TemplateTemplateParmDecl>(this))
    return TTP->isParameterPack();
  return false;
}

bool Decl::isParameterPack() const {
  if (const ParmVarDecl *Parm = dyn_cast<ParmVarDecl>(this))
    return Parm->isParameterPack();
  
  return isTemplateParameterPack();
}

FunctionDecl *Decl::getAsFunction() {
  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(this))
    return FD;
  if (const FunctionTemplateDecl *FTD = dyn_cast<FunctionTemplateDecl>(this))
    return FTD->getTemplatedDecl();
  return nullptr;
}

bool Decl::isTemplateDecl() const {
  return isa<TemplateDecl>(this);
}

const DeclContext *Decl::getParentFunctionOrMethod() const {
  for (const DeclContext *DC = getDeclContext();
       DC && !DC->isTranslationUnit() && !DC->isNamespace(); 
       DC = DC->getParent())
    if (DC->isFunctionOrMethod())
      return DC;

  return nullptr;
}


//===----------------------------------------------------------------------===//
// PrettyStackTraceDecl Implementation
//===----------------------------------------------------------------------===//

void PrettyStackTraceDecl::print(raw_ostream &OS) const {
  SourceLocation TheLoc = Loc;
  if (TheLoc.isInvalid() && TheDecl)
    TheLoc = TheDecl->getLocation();

  if (TheLoc.isValid()) {
    TheLoc.print(OS, SM);
    OS << ": ";
  }

  OS << Message;

  if (const NamedDecl *DN = dyn_cast_or_null<NamedDecl>(TheDecl)) {
    OS << " '";
    DN->printQualifiedName(OS);
    OS << '\'';
  }
  OS << '\n';
}

//===----------------------------------------------------------------------===//
// Decl Implementation
//===----------------------------------------------------------------------===//

// Out-of-line virtual method providing a home for Decl.
Decl::~Decl() { }

void Decl::setDeclContext(DeclContext *DC) {
  DeclCtx = DC;
}

void Decl::setLexicalDeclContext(DeclContext *DC) {
  if (DC == getLexicalDeclContext())
    return;

  if (isInSemaDC()) {
    setDeclContextsImpl(getDeclContext(), DC, getASTContext());
  } else {
    getMultipleDC()->LexicalDC = DC;
  }
  Hidden = cast<Decl>(DC)->Hidden;
}

void Decl::setDeclContextsImpl(DeclContext *SemaDC, DeclContext *LexicalDC,
                               ASTContext &Ctx) {
  if (SemaDC == LexicalDC) {
    DeclCtx = SemaDC;
  } else {
    Decl::MultipleDC *MDC = new (Ctx) Decl::MultipleDC();
    MDC->SemanticDC = SemaDC;
    MDC->LexicalDC = LexicalDC;
    DeclCtx = MDC;
  }
}

/// HLSL Change Begin - back port from llvm-project/73c6a2448f24 & f721e0582b15.
bool Decl::isInLocalScopeForInstantiation() const {
  const DeclContext *LDC = getLexicalDeclContext();
  if (!LDC->isDependentContext())
    return false;
  while (true) {
    if (LDC->isFunctionOrMethod())
      return true;
    if (!isa<TagDecl>(LDC))
      return false;
    if (const auto *CRD = dyn_cast<CXXRecordDecl>(LDC))
      if (CRD->isLambda())
        return true;
    LDC = LDC->getLexicalParent();
  }
  return false;
}
/// HLSL Change End - back port from llvm-project/73c6a2448f24 & f721e0582b15.

bool Decl::isInAnonymousNamespace() const {
  const DeclContext *DC = getDeclContext();
  do {
    if (const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(DC))
      if (ND->isAnonymousNamespace())
        return true;
  } while ((DC = DC->getParent()));

  return false;
}

bool Decl::isInStdNamespace() const {
  return getDeclContext()->isStdNamespace();
}

TranslationUnitDecl *Decl::getTranslationUnitDecl() {
  if (TranslationUnitDecl *TUD = dyn_cast<TranslationUnitDecl>(this))
    return TUD;

  DeclContext *DC = getDeclContext();
  assert(DC && "This decl is not contained in a translation unit!");

  while (!DC->isTranslationUnit()) {
    DC = DC->getParent();
    assert(DC && "This decl is not contained in a translation unit!");
  }

  return cast<TranslationUnitDecl>(DC);
}

ASTContext &Decl::getASTContext() const {
  return getTranslationUnitDecl()->getASTContext();
}

ASTMutationListener *Decl::getASTMutationListener() const {
  return getASTContext().getASTMutationListener();
}

unsigned Decl::getMaxAlignment() const {
  if (!hasAttrs())
    return 0;

  unsigned Align = 0;
  const AttrVec &V = getAttrs();
  ASTContext &Ctx = getASTContext();
  specific_attr_iterator<AlignedAttr> I(V.begin()), E(V.end());
  for (; I != E; ++I)
    Align = std::max(Align, I->getAlignment(Ctx));
  return Align;
}

bool Decl::isUsed(bool CheckUsedAttr) const { 
  if (Used)
    return true;
  
  // Check for used attribute.
  if (CheckUsedAttr && hasAttr<UsedAttr>())
    return true;

  return false; 
}

void Decl::markUsed(ASTContext &C) {
  if (Used)
    return;

  if (C.getASTMutationListener())
    C.getASTMutationListener()->DeclarationMarkedUsed(this);

  Used = true;
}

bool Decl::isReferenced() const { 
  if (Referenced)
    return true;

  // Check redeclarations.
  for (auto I : redecls())
    if (I->Referenced)
      return true;

  return false; 
}

/// \brief Determine the availability of the given declaration based on
/// the target platform.
///
/// When it returns an availability result other than \c AR_Available,
/// if the \p Message parameter is non-NULL, it will be set to a
/// string describing why the entity is unavailable.
///
/// FIXME: Make these strings localizable, since they end up in
/// diagnostics.
static AvailabilityResult CheckAvailability(ASTContext &Context,
                                            const AvailabilityAttr *A,
                                            std::string *Message) {
  VersionTuple TargetMinVersion =
    Context.getTargetInfo().getPlatformMinVersion();

  if (TargetMinVersion.empty())
    return AR_Available;

  // Check if this is an App Extension "platform", and if so chop off
  // the suffix for matching with the actual platform.
  StringRef ActualPlatform = A->getPlatform()->getName();
  StringRef RealizedPlatform = ActualPlatform;
  if (Context.getLangOpts().AppExt) {
    size_t suffix = RealizedPlatform.rfind("_app_extension");
    if (suffix != StringRef::npos)
      RealizedPlatform = RealizedPlatform.slice(0, suffix);
  }

  StringRef TargetPlatform = Context.getTargetInfo().getPlatformName();

  // Match the platform name.
  if (RealizedPlatform != TargetPlatform)
    return AR_Available;

  StringRef PrettyPlatformName
    = AvailabilityAttr::getPrettyPlatformName(ActualPlatform);

  if (PrettyPlatformName.empty())
    PrettyPlatformName = ActualPlatform;

  std::string HintMessage;
  if (!A->getMessage().empty()) {
    HintMessage = " - ";
    HintMessage += A->getMessage();
  }
  
  // Make sure that this declaration has not been marked 'unavailable'.
  if (A->getUnavailable()) {
    if (Message) {
      Message->clear();
      llvm::raw_string_ostream Out(*Message);
      Out << "not available on " << PrettyPlatformName 
          << HintMessage;
    }

    return AR_Unavailable;
  }

  // Make sure that this declaration has already been introduced.
  if (!A->getIntroduced().empty() && 
      TargetMinVersion < A->getIntroduced()) {
    if (Message) {
      Message->clear();
      llvm::raw_string_ostream Out(*Message);
      VersionTuple VTI(A->getIntroduced());
      VTI.UseDotAsSeparator();
      Out << "introduced in " << PrettyPlatformName << ' ' 
          << VTI << HintMessage;
    }

    return AR_NotYetIntroduced;
  }

  // Make sure that this declaration hasn't been obsoleted.
  if (!A->getObsoleted().empty() && TargetMinVersion >= A->getObsoleted()) {
    if (Message) {
      Message->clear();
      llvm::raw_string_ostream Out(*Message);
      VersionTuple VTO(A->getObsoleted());
      VTO.UseDotAsSeparator();
      Out << "obsoleted in " << PrettyPlatformName << ' ' 
          << VTO << HintMessage;
    }
    
    return AR_Unavailable;
  }

  // Make sure that this declaration hasn't been deprecated.
  if (!A->getDeprecated().empty() && TargetMinVersion >= A->getDeprecated()) {
    if (Message) {
      Message->clear();
      llvm::raw_string_ostream Out(*Message);
      VersionTuple VTD(A->getDeprecated());
      VTD.UseDotAsSeparator();
      Out << "first deprecated in " << PrettyPlatformName << ' '
          << VTD << HintMessage;
    }
    
    return AR_Deprecated;
  }

  return AR_Available;
}

AvailabilityResult Decl::getAvailability(std::string *Message) const {
  AvailabilityResult Result = AR_Available;
  std::string ResultMessage;

  for (const auto *A : attrs()) {
    if (const auto *Deprecated = dyn_cast<DeprecatedAttr>(A)) {
      if (Result >= AR_Deprecated)
        continue;

      if (Message)
        ResultMessage = Deprecated->getMessage();

      Result = AR_Deprecated;
      continue;
    }

    if (const auto *Unavailable = dyn_cast<UnavailableAttr>(A)) {
      if (Message)
        *Message = Unavailable->getMessage();
      return AR_Unavailable;
    }

    if (const auto *Availability = dyn_cast<AvailabilityAttr>(A)) {
      AvailabilityResult AR = CheckAvailability(getASTContext(), Availability,
                                                Message);

      if (AR == AR_Unavailable)
        return AR_Unavailable;

      if (AR > Result) {
        Result = AR;
        if (Message)
          ResultMessage.swap(*Message);
      }
      continue;
    }
  }

  if (Message)
    Message->swap(ResultMessage);
  return Result;
}

bool Decl::canBeWeakImported(bool &IsDefinition) const {
  IsDefinition = false;

  // Variables, if they aren't definitions.
  if (const VarDecl *Var = dyn_cast<VarDecl>(this)) {
    if (Var->isThisDeclarationADefinition()) {
      IsDefinition = true;
      return false;
    }
    return true;

  // Functions, if they aren't definitions.
  } else if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(this)) {
    if (FD->hasBody()) {
      IsDefinition = true;
      return false;
    }
    return true;

  // Objective-C classes, if this is the non-fragile runtime.
  } else if (isa<ObjCInterfaceDecl>(this) &&
             getASTContext().getLangOpts().ObjCRuntime.hasWeakClassImport()) {
    return true;

  // Nothing else.
  } else {
    return false;
  }
}

bool Decl::isWeakImported() const {
  bool IsDefinition;
  if (!canBeWeakImported(IsDefinition))
    return false;

  for (const auto *A : attrs()) {
    if (isa<WeakImportAttr>(A))
      return true;

    if (const auto *Availability = dyn_cast<AvailabilityAttr>(A)) {
      if (CheckAvailability(getASTContext(), Availability,
                            nullptr) == AR_NotYetIntroduced)
        return true;
    }
  }

  return false;
}

unsigned Decl::getIdentifierNamespaceForKind(Kind DeclKind) {
  switch (DeclKind) {
    case Function:
    case CXXMethod:
    case CXXConstructor:
    case CXXDestructor:
    case CXXConversion:
    case EnumConstant:
    case Var:
    case ImplicitParam:
    case ParmVar:
    case NonTypeTemplateParm:
    case ObjCMethod:
    case ObjCProperty:
    case MSProperty:
      return IDNS_Ordinary;
    case Label:
      return IDNS_Label;
    case IndirectField:
      return IDNS_Ordinary | IDNS_Member;

    case ObjCCompatibleAlias:
    case ObjCInterface:
      return IDNS_Ordinary | IDNS_Type;

    case Typedef:
    case TypeAlias:
    case TypeAliasTemplate:
    case UnresolvedUsingTypename:
    case TemplateTypeParm:
    case ObjCTypeParam:
      return IDNS_Ordinary | IDNS_Type;

    case UsingShadow:
      return 0; // we'll actually overwrite this later

    case UnresolvedUsingValue:
      return IDNS_Ordinary | IDNS_Using;

    case Using:
      return IDNS_Using;

    case ObjCProtocol:
      return IDNS_ObjCProtocol;

    case Field:
    case ObjCAtDefsField:
    case ObjCIvar:
      return IDNS_Member;

    case Record:
    case CXXRecord:
    case Enum:
      return IDNS_Tag | IDNS_Type;

    case Namespace:
    case NamespaceAlias:
      return IDNS_Namespace;

    case FunctionTemplate:
    case VarTemplate:
      return IDNS_Ordinary;

    case ClassTemplate:
    case TemplateTemplateParm:
      return IDNS_Ordinary | IDNS_Tag | IDNS_Type;

    // Never have names.
    case Friend:
    case FriendTemplate:
    case AccessSpec:
    case LinkageSpec:
    case FileScopeAsm:
    case StaticAssert:
    case ObjCPropertyImpl:
    case Block:
    case Captured:
    case TranslationUnit:
    case ExternCContext:

    case UsingDirective:
    case ClassTemplateSpecialization:
    case ClassTemplatePartialSpecialization:
    case ClassScopeFunctionSpecialization:
    case VarTemplateSpecialization:
    case VarTemplatePartialSpecialization:
    case ObjCImplementation:
    case ObjCCategory:
    case ObjCCategoryImpl:
    case Import:
    case OMPThreadPrivate:
    case Empty:
    case HLSLBuffer: // HLSL Change
      // Never looked up by name.
      return 0;
  }

  llvm_unreachable("Invalid DeclKind!");
}

void Decl::setAttrsImpl(const AttrVec &attrs, ASTContext &Ctx) {
  assert(!HasAttrs && "Decl already contains attrs.");

  AttrVec &AttrBlank = Ctx.getDeclAttrs(this);
  assert(AttrBlank.empty() && "HasAttrs was wrong?");

  AttrBlank = attrs;
  HasAttrs = true;
}

void Decl::dropAttrs() {
  if (!HasAttrs) return;

  HasAttrs = false;
  getASTContext().eraseDeclAttrs(this);
}

const AttrVec &Decl::getAttrs() const {
  assert(HasAttrs && "No attrs to get!");
  return getASTContext().getDeclAttrs(this);
}

Decl *Decl::castFromDeclContext (const DeclContext *D) {
  Decl::Kind DK = D->getDeclKind();
  switch(DK) {
#define DECL(NAME, BASE)
#define DECL_CONTEXT(NAME) \
    case Decl::NAME:       \
      return static_cast<NAME##Decl*>(const_cast<DeclContext*>(D));
#define DECL_CONTEXT_BASE(NAME)
#include "clang/AST/DeclNodes.inc"
    default:
#define DECL(NAME, BASE)
#define DECL_CONTEXT_BASE(NAME)                  \
      if (DK >= first##NAME && DK <= last##NAME) \
        return static_cast<NAME##Decl*>(const_cast<DeclContext*>(D));
#include "clang/AST/DeclNodes.inc"
      llvm_unreachable("a decl that inherits DeclContext isn't handled");
  }
}

DeclContext *Decl::castToDeclContext(const Decl *D) {
  Decl::Kind DK = D->getKind();
  switch(DK) {
#define DECL(NAME, BASE)
#define DECL_CONTEXT(NAME) \
    case Decl::NAME:       \
      return static_cast<NAME##Decl*>(const_cast<Decl*>(D));
#define DECL_CONTEXT_BASE(NAME)
#include "clang/AST/DeclNodes.inc"
    default:
#define DECL(NAME, BASE)
#define DECL_CONTEXT_BASE(NAME)                                   \
      if (DK >= first##NAME && DK <= last##NAME)                  \
        return static_cast<NAME##Decl*>(const_cast<Decl*>(D));
#include "clang/AST/DeclNodes.inc"
      llvm_unreachable("a decl that inherits DeclContext isn't handled");
  }
}

SourceLocation Decl::getBodyRBrace() const {
  // Special handling of FunctionDecl to avoid de-serializing the body from PCH.
  // FunctionDecl stores EndRangeLoc for this purpose.
  if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(this)) {
    const FunctionDecl *Definition;
    if (FD->hasBody(Definition))
      return Definition->getSourceRange().getEnd();
    return SourceLocation();
  }

  if (Stmt *Body = getBody())
    return Body->getSourceRange().getEnd();

  return SourceLocation();
}

bool Decl::AccessDeclContextSanity() const {
#ifndef NDEBUG
  // Suppress this check if any of the following hold:
  // 1. this is the translation unit (and thus has no parent)
  // 2. this is a template parameter (and thus doesn't belong to its context)
  // 3. this is a non-type template parameter
  // 4. the context is not a record
  // 5. it's invalid
  // 6. it's a C++0x static_assert.
  if (isa<TranslationUnitDecl>(this) ||
      isa<TemplateTypeParmDecl>(this) ||
      isa<NonTypeTemplateParmDecl>(this) ||
      !isa<CXXRecordDecl>(getDeclContext()) ||
      isInvalidDecl() ||
      isa<StaticAssertDecl>(this) ||
      // FIXME: a ParmVarDecl can have ClassTemplateSpecialization
      // as DeclContext (?).
      isa<ParmVarDecl>(this) ||
      // FIXME: a ClassTemplateSpecialization or CXXRecordDecl can have
      // AS_none as access specifier.
      isa<CXXRecordDecl>(this) ||
      isa<ClassScopeFunctionSpecializationDecl>(this))
    return true;

  assert(Access != AS_none &&
         "Access specifier is AS_none inside a record decl");
#endif
  return true;
}

static Decl::Kind getKind(const Decl *D) { return D->getKind(); }
static Decl::Kind getKind(const DeclContext *DC) { return DC->getDeclKind(); }

const FunctionType *Decl::getFunctionType(bool BlocksToo) const {
  QualType Ty;
  if (const ValueDecl *D = dyn_cast<ValueDecl>(this))
    Ty = D->getType();
  else if (const TypedefNameDecl *D = dyn_cast<TypedefNameDecl>(this))
    Ty = D->getUnderlyingType();
  else
    return nullptr;

  if (Ty->isFunctionPointerType())
    Ty = Ty->getAs<PointerType>()->getPointeeType();
  else if (BlocksToo && Ty->isBlockPointerType())
    Ty = Ty->getAs<BlockPointerType>()->getPointeeType();

  return Ty->getAs<FunctionType>();
}


/// Starting at a given context (a Decl or DeclContext), look for a
/// code context that is not a closure (a lambda, block, etc.).
template <class T> static Decl *getNonClosureContext(T *D) {
  if (getKind(D) == Decl::CXXMethod) {
    CXXMethodDecl *MD = cast<CXXMethodDecl>(D);
    if (MD->getOverloadedOperator() == OO_Call &&
        MD->getParent()->isLambda())
      return getNonClosureContext(MD->getParent()->getParent());
    return MD;
  } else if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
    return FD;
  } else if (ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
    return MD;
  } else if (BlockDecl *BD = dyn_cast<BlockDecl>(D)) {
    return getNonClosureContext(BD->getParent());
  } else if (CapturedDecl *CD = dyn_cast<CapturedDecl>(D)) {
    return getNonClosureContext(CD->getParent());
  } else {
    return nullptr;
  }
}

Decl *Decl::getNonClosureContext() {
  return ::getNonClosureContext(this);
}

Decl *DeclContext::getNonClosureAncestor() {
  return ::getNonClosureContext(this);
}

//===----------------------------------------------------------------------===//
// DeclContext Implementation
//===----------------------------------------------------------------------===//

bool DeclContext::classof(const Decl *D) {
  switch (D->getKind()) {
#define DECL(NAME, BASE)
#define DECL_CONTEXT(NAME) case Decl::NAME:
#define DECL_CONTEXT_BASE(NAME)
#include "clang/AST/DeclNodes.inc"
      return true;
    default:
#define DECL(NAME, BASE)
#define DECL_CONTEXT_BASE(NAME)                 \
      if (D->getKind() >= Decl::first##NAME &&  \
          D->getKind() <= Decl::last##NAME)     \
        return true;
#include "clang/AST/DeclNodes.inc"
      return false;
  }
}

DeclContext::~DeclContext() { }

/// \brief Find the parent context of this context that will be
/// used for unqualified name lookup.
///
/// Generally, the parent lookup context is the semantic context. However, for
/// a friend function the parent lookup context is the lexical context, which
/// is the class in which the friend is declared.
DeclContext *DeclContext::getLookupParent() {
  // FIXME: Find a better way to identify friends
  if (isa<FunctionDecl>(this))
    if (getParent()->getRedeclContext()->isFileContext() &&
        getLexicalParent()->getRedeclContext()->isRecord())
      return getLexicalParent();
  
  return getParent();
}

bool DeclContext::isInlineNamespace() const {
  return isNamespace() &&
         cast<NamespaceDecl>(this)->isInline();
}

bool DeclContext::isStdNamespace() const {
  if (!isNamespace())
    return false;

  const NamespaceDecl *ND = cast<NamespaceDecl>(this);
  if (ND->isInline()) {
    return ND->getParent()->isStdNamespace();
  }

  if (!getParent()->getRedeclContext()->isTranslationUnit())
    return false;

  const IdentifierInfo *II = ND->getIdentifier();
  return II && II->isStr("std");
}

bool DeclContext::isDependentContext() const {
  if (isFileContext())
    return false;

  if (isa<ClassTemplatePartialSpecializationDecl>(this))
    return true;

  if (const CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(this)) {
    if (Record->getDescribedClassTemplate())
      return true;
    
    if (Record->isDependentLambda())
      return true;
  }
  
  if (const FunctionDecl *Function = dyn_cast<FunctionDecl>(this)) {
    if (Function->getDescribedFunctionTemplate())
      return true;

    // Friend function declarations are dependent if their *lexical*
    // context is dependent.
    if (cast<Decl>(this)->getFriendObjectKind())
      return getLexicalParent()->isDependentContext();
  }

  // FIXME: A variable template is a dependent context, but is not a
  // DeclContext. A context within it (such as a lambda-expression)
  // should be considered dependent.

  return getParent() && getParent()->isDependentContext();
}

bool DeclContext::isTransparentContext() const {
  if (DeclKind == Decl::Enum)
    return !cast<EnumDecl>(this)->isScoped();
  else if (DeclKind == Decl::LinkageSpec)
    return true;

  // HLSL Change Starts
  if (DeclKind == Decl::HLSLBuffer)
    return true;
  // HLSL Change Ends

  return false;
}

static bool isLinkageSpecContext(const DeclContext *DC,
                                 LinkageSpecDecl::LanguageIDs ID) {
  while (DC->getDeclKind() != Decl::TranslationUnit) {
    if (DC->getDeclKind() == Decl::LinkageSpec)
      return cast<LinkageSpecDecl>(DC)->getLanguage() == ID;
    DC = DC->getLexicalParent();
  }
  return false;
}

bool DeclContext::isExternCContext() const {
  return isLinkageSpecContext(this, clang::LinkageSpecDecl::lang_c);
}

bool DeclContext::isExternCXXContext() const {
  return isLinkageSpecContext(this, clang::LinkageSpecDecl::lang_cxx);
}

bool DeclContext::Encloses(const DeclContext *DC) const {
  if (getPrimaryContext() != this)
    return getPrimaryContext()->Encloses(DC);

  for (; DC; DC = DC->getParent())
    if (DC->getPrimaryContext() == this)
      return true;
  return false;
}

DeclContext *DeclContext::getPrimaryContext() {
  switch (DeclKind) {
  case Decl::TranslationUnit:
  case Decl::ExternCContext:
  case Decl::LinkageSpec:
  case Decl::Block:
  case Decl::Captured:
    // There is only one DeclContext for these entities.
    return this;

  // HLSL Change Starts
  case Decl::HLSLBuffer:
    // Each buffer, even with the same name, is a distinct construct.
    return this;
  // HLSL Change Ends

  case Decl::Namespace:
    // The original namespace is our primary context.
    return static_cast<NamespaceDecl*>(this)->getOriginalNamespace();

  case Decl::ObjCMethod:
    return this;

  case Decl::ObjCInterface:
    if (ObjCInterfaceDecl *Def = cast<ObjCInterfaceDecl>(this)->getDefinition())
      return Def;
      
    return this;
      
  case Decl::ObjCProtocol:
    if (ObjCProtocolDecl *Def = cast<ObjCProtocolDecl>(this)->getDefinition())
      return Def;
    
    return this;
      
  case Decl::ObjCCategory:
    return this;

  case Decl::ObjCImplementation:
  case Decl::ObjCCategoryImpl:
    return this;

  default:
    if (DeclKind >= Decl::firstTag && DeclKind <= Decl::lastTag) {
      // If this is a tag type that has a definition or is currently
      // being defined, that definition is our primary context.
      TagDecl *Tag = cast<TagDecl>(this);

      if (TagDecl *Def = Tag->getDefinition())
        return Def;

      if (const TagType *TagTy = dyn_cast<TagType>(Tag->getTypeForDecl())) {
        // Note, TagType::getDecl returns the (partial) definition one exists.
        TagDecl *PossiblePartialDef = TagTy->getDecl();
        if (PossiblePartialDef->isBeingDefined())
          return PossiblePartialDef;
      } else {
        assert(isa<InjectedClassNameType>(Tag->getTypeForDecl()));
      }

      return Tag;
    }

    assert(DeclKind >= Decl::firstFunction && DeclKind <= Decl::lastFunction &&
          "Unknown DeclContext kind");
    return this;
  }
}

void 
DeclContext::collectAllContexts(SmallVectorImpl<DeclContext *> &Contexts){
  Contexts.clear();
  
  if (DeclKind != Decl::Namespace) {
    Contexts.push_back(this);
    return;
  }
  
  NamespaceDecl *Self = static_cast<NamespaceDecl *>(this);
  for (NamespaceDecl *N = Self->getMostRecentDecl(); N;
       N = N->getPreviousDecl())
    Contexts.push_back(N);
  
  std::reverse(Contexts.begin(), Contexts.end());
}

std::pair<Decl *, Decl *>
DeclContext::BuildDeclChain(ArrayRef<Decl*> Decls,
                            bool FieldsAlreadyLoaded) {
  // Build up a chain of declarations via the Decl::NextInContextAndBits field.
  Decl *FirstNewDecl = nullptr;
  Decl *PrevDecl = nullptr;
  for (unsigned I = 0, N = Decls.size(); I != N; ++I) {
    if (FieldsAlreadyLoaded && isa<FieldDecl>(Decls[I]))
      continue;

    Decl *D = Decls[I];
    if (PrevDecl)
      PrevDecl->NextInContextAndBits.setPointer(D);
    else
      FirstNewDecl = D;

    PrevDecl = D;
  }

  return std::make_pair(FirstNewDecl, PrevDecl);
}

/// \brief We have just acquired external visible storage, and we already have
/// built a lookup map. For every name in the map, pull in the new names from
/// the external storage.
void DeclContext::reconcileExternalVisibleStorage() const {
  assert(NeedToReconcileExternalVisibleStorage && LookupPtr);
  NeedToReconcileExternalVisibleStorage = false;

  for (auto &Lookup : *LookupPtr)
    Lookup.second.setHasExternalDecls();
}

/// \brief Load the declarations within this lexical storage from an
/// external source.
/// \return \c true if any declarations were added.
bool
DeclContext::LoadLexicalDeclsFromExternalStorage() const {
  ExternalASTSource *Source = getParentASTContext().getExternalSource();
  assert(hasExternalLexicalStorage() && Source && "No external storage?");

  // Notify that we have a DeclContext that is initializing.
  ExternalASTSource::Deserializing ADeclContext(Source);

  // Load the external declarations, if any.
  SmallVector<Decl*, 64> Decls;
  ExternalLexicalStorage = false;
  switch (Source->FindExternalLexicalDecls(this, Decls)) {
  case ELR_Success:
    break;
    
  case ELR_Failure:
  case ELR_AlreadyLoaded:
    return false;
  }

  if (Decls.empty())
    return false;

  // We may have already loaded just the fields of this record, in which case
  // we need to ignore them.
  bool FieldsAlreadyLoaded = false;
  if (const RecordDecl *RD = dyn_cast<RecordDecl>(this))
    FieldsAlreadyLoaded = RD->LoadedFieldsFromExternalStorage;
  
  // Splice the newly-read declarations into the beginning of the list
  // of declarations.
  Decl *ExternalFirst, *ExternalLast;
  std::tie(ExternalFirst, ExternalLast) =
      BuildDeclChain(Decls, FieldsAlreadyLoaded);
  ExternalLast->NextInContextAndBits.setPointer(FirstDecl);
  FirstDecl = ExternalFirst;
  if (!LastDecl)
    LastDecl = ExternalLast;
  return true;
}

DeclContext::lookup_result
ExternalASTSource::SetNoExternalVisibleDeclsForName(const DeclContext *DC,
                                                    DeclarationName Name) {
  ASTContext &Context = DC->getParentASTContext();
  StoredDeclsMap *Map;
  if (!(Map = DC->LookupPtr))
    Map = DC->CreateStoredDeclsMap(Context);
  if (DC->NeedToReconcileExternalVisibleStorage)
    DC->reconcileExternalVisibleStorage();

  (*Map)[Name].removeExternalDecls();

  return DeclContext::lookup_result();
}

DeclContext::lookup_result
ExternalASTSource::SetExternalVisibleDeclsForName(const DeclContext *DC,
                                                  DeclarationName Name,
                                                  ArrayRef<NamedDecl*> Decls) {
  ASTContext &Context = DC->getParentASTContext();
  StoredDeclsMap *Map;
  if (!(Map = DC->LookupPtr))
    Map = DC->CreateStoredDeclsMap(Context);
  if (DC->NeedToReconcileExternalVisibleStorage)
    DC->reconcileExternalVisibleStorage();

  StoredDeclsList &List = (*Map)[Name];

  // Clear out any old external visible declarations, to avoid quadratic
  // performance in the redeclaration checks below.
  List.removeExternalDecls();

  if (!List.isNull()) {
    // We have both existing declarations and new declarations for this name.
    // Some of the declarations may simply replace existing ones. Handle those
    // first.
    llvm::SmallVector<unsigned, 8> Skip;
    for (unsigned I = 0, N = Decls.size(); I != N; ++I)
      if (List.HandleRedeclaration(Decls[I], /*IsKnownNewer*/false))
        Skip.push_back(I);
    Skip.push_back(Decls.size());

    // Add in any new declarations.
    unsigned SkipPos = 0;
    for (unsigned I = 0, N = Decls.size(); I != N; ++I) {
      if (I == Skip[SkipPos])
        ++SkipPos;
      else
        List.AddSubsequentDecl(Decls[I]);
    }
  } else {
    // Convert the array to a StoredDeclsList.
    for (ArrayRef<NamedDecl*>::iterator
           I = Decls.begin(), E = Decls.end(); I != E; ++I) {
      if (List.isNull())
        List.setOnlyValue(*I);
      else
        List.AddSubsequentDecl(*I);
    }
  }

  return List.getLookupResult();
}

DeclContext::decl_iterator DeclContext::decls_begin() const {
  if (hasExternalLexicalStorage())
    LoadLexicalDeclsFromExternalStorage();
  return decl_iterator(FirstDecl);
}

bool DeclContext::decls_empty() const {
  if (hasExternalLexicalStorage())
    LoadLexicalDeclsFromExternalStorage();

  return !FirstDecl;
}

bool DeclContext::containsDecl(Decl *D) const {
  return (D->getLexicalDeclContext() == this &&
          (D->NextInContextAndBits.getPointer() || D == LastDecl));
}

void DeclContext::removeDecl(Decl *D) {
  assert(D->getLexicalDeclContext() == this &&
         "decl being removed from non-lexical context");
  assert((D->NextInContextAndBits.getPointer() || D == LastDecl) &&
         "decl is not in decls list");

  // Remove D from the decl chain.  This is O(n) but hopefully rare.
  if (D == FirstDecl) {
    if (D == LastDecl)
      FirstDecl = LastDecl = nullptr;
    else
      FirstDecl = D->NextInContextAndBits.getPointer();
  } else {
    for (Decl *I = FirstDecl; true; I = I->NextInContextAndBits.getPointer()) {
      assert(I && "decl not found in linked list");
      if (I->NextInContextAndBits.getPointer() == D) {
        I->NextInContextAndBits.setPointer(D->NextInContextAndBits.getPointer());
        if (D == LastDecl) LastDecl = I;
        break;
      }
    }
  }
  
  // Mark that D is no longer in the decl chain.
  D->NextInContextAndBits.setPointer(nullptr);

  // Remove D from the lookup table if necessary.
  if (isa<NamedDecl>(D)) {
    NamedDecl *ND = cast<NamedDecl>(D);

    // Remove only decls that have a name
    if (!ND->getDeclName()) return;

    StoredDeclsMap *Map = getPrimaryContext()->LookupPtr;
    if (!Map) return;

    StoredDeclsMap::iterator Pos = Map->find(ND->getDeclName());
    assert(Pos != Map->end() && "no lookup entry for decl");
    if (Pos->second.getAsVector() || Pos->second.getAsDecl() == ND)
      Pos->second.remove(ND);
  }
}

void DeclContext::addHiddenDecl(Decl *D) {
  assert(D->getLexicalDeclContext() == this &&
         "Decl inserted into wrong lexical context");
  assert(!D->getNextDeclInContext() && D != LastDecl &&
         "Decl already inserted into a DeclContext");

  if (FirstDecl) {
    LastDecl->NextInContextAndBits.setPointer(D);
    LastDecl = D;
  } else {
    FirstDecl = LastDecl = D;
  }

  // Notify a C++ record declaration that we've added a member, so it can
  // update it's class-specific state.
  if (CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(this))
    Record->addedMember(D);

  // If this is a newly-created (not de-serialized) import declaration, wire
  // it in to the list of local import declarations.
  if (!D->isFromASTFile()) {
    if (ImportDecl *Import = dyn_cast<ImportDecl>(D))
      D->getASTContext().addedLocalImportDecl(Import);
  }
}

void DeclContext::addDecl(Decl *D) {
  addHiddenDecl(D);

  if (NamedDecl *ND = dyn_cast<NamedDecl>(D))
    ND->getDeclContext()->getPrimaryContext()->
        makeDeclVisibleInContextWithFlags(ND, false, true);
}

void DeclContext::addDeclInternal(Decl *D) {
  addHiddenDecl(D);

  if (NamedDecl *ND = dyn_cast<NamedDecl>(D))
    ND->getDeclContext()->getPrimaryContext()->
        makeDeclVisibleInContextWithFlags(ND, true, true);
}

/// shouldBeHidden - Determine whether a declaration which was declared
/// within its semantic context should be invisible to qualified name lookup.
static bool shouldBeHidden(NamedDecl *D) {
  // Skip unnamed declarations.
  if (!D->getDeclName())
    return true;

  // Skip entities that can't be found by name lookup into a particular
  // context.
  if ((D->getIdentifierNamespace() == 0 && !isa<UsingDirectiveDecl>(D)) ||
      D->isTemplateParameter())
    return true;

  // Skip template specializations.
  // FIXME: This feels like a hack. Should DeclarationName support
  // template-ids, or is there a better way to keep specializations
  // from being visible?
  if (isa<ClassTemplateSpecializationDecl>(D))
    return true;
  if (FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
    if (FD->isFunctionTemplateSpecialization())
      return true;

  return false;
}

/// buildLookup - Build the lookup data structure with all of the
/// declarations in this DeclContext (and any other contexts linked
/// to it or transparent contexts nested within it) and return it.
///
/// Note that the produced map may miss out declarations from an
/// external source. If it does, those entries will be marked with
/// the 'hasExternalDecls' flag.
StoredDeclsMap *DeclContext::buildLookup() {
  assert(this == getPrimaryContext() && "buildLookup called on non-primary DC");

  if (!HasLazyLocalLexicalLookups && !HasLazyExternalLexicalLookups)
    return LookupPtr;

  SmallVector<DeclContext *, 2> Contexts;
  collectAllContexts(Contexts);

  if (HasLazyExternalLexicalLookups) {
    HasLazyExternalLexicalLookups = false;
    for (auto *DC : Contexts) {
      if (DC->hasExternalLexicalStorage())
        HasLazyLocalLexicalLookups |=
            DC->LoadLexicalDeclsFromExternalStorage();
    }

    if (!HasLazyLocalLexicalLookups)
      return LookupPtr;
  }

  for (auto *DC : Contexts)
    buildLookupImpl(DC, hasExternalVisibleStorage());

  // We no longer have any lazy decls.
  HasLazyLocalLexicalLookups = false;
  return LookupPtr;
}

/// buildLookupImpl - Build part of the lookup data structure for the
/// declarations contained within DCtx, which will either be this
/// DeclContext, a DeclContext linked to it, or a transparent context
/// nested within it.
void DeclContext::buildLookupImpl(DeclContext *DCtx, bool Internal) {
  for (Decl *D : DCtx->noload_decls()) {
    // Insert this declaration into the lookup structure, but only if
    // it's semantically within its decl context. Any other decls which
    // should be found in this context are added eagerly.
    //
    // If it's from an AST file, don't add it now. It'll get handled by
    // FindExternalVisibleDeclsByName if needed. Exception: if we're not
    // in C++, we do not track external visible decls for the TU, so in
    // that case we need to collect them all here.
    if (NamedDecl *ND = dyn_cast<NamedDecl>(D))
      if (ND->getDeclContext() == DCtx && !shouldBeHidden(ND) &&
          (!ND->isFromASTFile() ||
           (isTranslationUnit() &&
            !getParentASTContext().getLangOpts().CPlusPlus)))
        makeDeclVisibleInContextImpl(ND, Internal);

    // If this declaration is itself a transparent declaration context
    // or inline namespace, add the members of this declaration of that
    // context (recursively).
    if (DeclContext *InnerCtx = dyn_cast<DeclContext>(D))
      if (InnerCtx->isTransparentContext() || InnerCtx->isInlineNamespace())
        buildLookupImpl(InnerCtx, Internal);
  }
}

NamedDecl *const DeclContextLookupResult::SingleElementDummyList = nullptr;

DeclContext::lookup_result
DeclContext::lookup(DeclarationName Name) const {
  assert(DeclKind != Decl::LinkageSpec &&
         "Should not perform lookups into linkage specs!");

  const DeclContext *PrimaryContext = getPrimaryContext();
  if (PrimaryContext != this)
    return PrimaryContext->lookup(Name);

  // If we have an external source, ensure that any later redeclarations of this
  // context have been loaded, since they may add names to the result of this
  // lookup (or add external visible storage).
  ExternalASTSource *Source = getParentASTContext().getExternalSource();
  if (Source)
    (void)cast<Decl>(this)->getMostRecentDecl();

  if (hasExternalVisibleStorage()) {
    assert(Source && "external visible storage but no external source?");

    if (NeedToReconcileExternalVisibleStorage)
      reconcileExternalVisibleStorage();

    StoredDeclsMap *Map = LookupPtr;

    if (HasLazyLocalLexicalLookups || HasLazyExternalLexicalLookups)
      // FIXME: Make buildLookup const?
      Map = const_cast<DeclContext*>(this)->buildLookup();

    if (!Map)
      Map = CreateStoredDeclsMap(getParentASTContext());

    // If we have a lookup result with no external decls, we are done.
    std::pair<StoredDeclsMap::iterator, bool> R =
        Map->insert(std::make_pair(Name, StoredDeclsList()));
    if (!R.second && !R.first->second.hasExternalDecls())
      return R.first->second.getLookupResult();

    if (Source->FindExternalVisibleDeclsByName(this, Name) || !R.second) {
      if (StoredDeclsMap *Map = LookupPtr) {
        StoredDeclsMap::iterator I = Map->find(Name);
        if (I != Map->end())
          return I->second.getLookupResult();
      }
    }

    return lookup_result();
  }

  StoredDeclsMap *Map = LookupPtr;
  if (HasLazyLocalLexicalLookups || HasLazyExternalLexicalLookups)
    Map = const_cast<DeclContext*>(this)->buildLookup();

  if (!Map)
    return lookup_result();

  StoredDeclsMap::iterator I = Map->find(Name);
  if (I == Map->end())
    return lookup_result();

  return I->second.getLookupResult();
}

DeclContext::lookup_result
DeclContext::noload_lookup(DeclarationName Name) {
  assert(DeclKind != Decl::LinkageSpec &&
         "Should not perform lookups into linkage specs!");

  DeclContext *PrimaryContext = getPrimaryContext();
  if (PrimaryContext != this)
    return PrimaryContext->noload_lookup(Name);

  // If we have any lazy lexical declarations not in our lookup map, add them
  // now. Don't import any external declarations, not even if we know we have
  // some missing from the external visible lookups.
  if (HasLazyLocalLexicalLookups) {
    SmallVector<DeclContext *, 2> Contexts;
    collectAllContexts(Contexts);
    for (unsigned I = 0, N = Contexts.size(); I != N; ++I)
      buildLookupImpl(Contexts[I], hasExternalVisibleStorage());
    HasLazyLocalLexicalLookups = false;
  }

  StoredDeclsMap *Map = LookupPtr;
  if (!Map)
    return lookup_result();

  StoredDeclsMap::iterator I = Map->find(Name);
  return I != Map->end() ? I->second.getLookupResult()
                         : lookup_result();
}

void DeclContext::localUncachedLookup(DeclarationName Name,
                                      SmallVectorImpl<NamedDecl *> &Results) {
  Results.clear();
  
  // If there's no external storage, just perform a normal lookup and copy
  // the results.
  if (!hasExternalVisibleStorage() && !hasExternalLexicalStorage() && Name) {
    lookup_result LookupResults = lookup(Name);
    Results.insert(Results.end(), LookupResults.begin(), LookupResults.end());
    return;
  }

  // If we have a lookup table, check there first. Maybe we'll get lucky.
  // FIXME: Should we be checking these flags on the primary context?
  if (Name && !HasLazyLocalLexicalLookups && !HasLazyExternalLexicalLookups) {
    if (StoredDeclsMap *Map = LookupPtr) {
      StoredDeclsMap::iterator Pos = Map->find(Name);
      if (Pos != Map->end()) {
        Results.insert(Results.end(),
                       Pos->second.getLookupResult().begin(),
                       Pos->second.getLookupResult().end());
        return;
      }
    }
  }

  // Slow case: grovel through the declarations in our chain looking for 
  // matches.
  // FIXME: If we have lazy external declarations, this will not find them!
  // FIXME: Should we CollectAllContexts and walk them all here?
  for (Decl *D = FirstDecl; D; D = D->getNextDeclInContext()) {
    if (NamedDecl *ND = dyn_cast<NamedDecl>(D))
      if (ND->getDeclName() == Name)
        Results.push_back(ND);
  }
}

DeclContext *DeclContext::getRedeclContext() {
  DeclContext *Ctx = this;
  // Skip through transparent contexts.
  while (Ctx->isTransparentContext())
    Ctx = Ctx->getParent();
  return Ctx;
}

DeclContext *DeclContext::getEnclosingNamespaceContext() {
  DeclContext *Ctx = this;
  // Skip through non-namespace, non-translation-unit contexts.
  while (!Ctx->isFileContext())
    Ctx = Ctx->getParent();
  return Ctx->getPrimaryContext();
}

RecordDecl *DeclContext::getOuterLexicalRecordContext() {
  // Loop until we find a non-record context.
  RecordDecl *OutermostRD = nullptr;
  DeclContext *DC = this;
  while (DC->isRecord()) {
    OutermostRD = cast<RecordDecl>(DC);
    DC = DC->getLexicalParent();
  }
  return OutermostRD;
}

bool DeclContext::InEnclosingNamespaceSetOf(const DeclContext *O) const {
  // For non-file contexts, this is equivalent to Equals.
  if (!isFileContext())
    return O->Equals(this);

  do {
    if (O->Equals(this))
      return true;

    const NamespaceDecl *NS = dyn_cast<NamespaceDecl>(O);
    if (!NS || !NS->isInline())
      break;
    O = NS->getParent();
  } while (O);

  return false;
}

void DeclContext::makeDeclVisibleInContext(NamedDecl *D) {
  DeclContext *PrimaryDC = this->getPrimaryContext();
  DeclContext *DeclDC = D->getDeclContext()->getPrimaryContext();
  // If the decl is being added outside of its semantic decl context, we
  // need to ensure that we eagerly build the lookup information for it.
  PrimaryDC->makeDeclVisibleInContextWithFlags(D, false, PrimaryDC == DeclDC);
}

void DeclContext::makeDeclVisibleInContextWithFlags(NamedDecl *D, bool Internal,
                                                    bool Recoverable) {
  assert(this == getPrimaryContext() && "expected a primary DC");

  // Skip declarations within functions.
  if (isFunctionOrMethod())
    return;

  // Skip declarations which should be invisible to name lookup.
  if (shouldBeHidden(D))
    return;

  // If we already have a lookup data structure, perform the insertion into
  // it. If we might have externally-stored decls with this name, look them
  // up and perform the insertion. If this decl was declared outside its
  // semantic context, buildLookup won't add it, so add it now.
  //
  // FIXME: As a performance hack, don't add such decls into the translation
  // unit unless we're in C++, since qualified lookup into the TU is never
  // performed.
  if (LookupPtr || hasExternalVisibleStorage() ||
      ((!Recoverable || D->getDeclContext() != D->getLexicalDeclContext()) &&
       (getParentASTContext().getLangOpts().CPlusPlus ||
        !isTranslationUnit()))) {
    // If we have lazily omitted any decls, they might have the same name as
    // the decl which we are adding, so build a full lookup table before adding
    // this decl.
    buildLookup();
    makeDeclVisibleInContextImpl(D, Internal);
  } else {
    HasLazyLocalLexicalLookups = true;
  }

  // If we are a transparent context or inline namespace, insert into our
  // parent context, too. This operation is recursive.
  if (isTransparentContext() || isInlineNamespace())
    getParent()->getPrimaryContext()->
        makeDeclVisibleInContextWithFlags(D, Internal, Recoverable);

  Decl *DCAsDecl = cast<Decl>(this);
  // Notify that a decl was made visible unless we are a Tag being defined.
  if (!(isa<TagDecl>(DCAsDecl) && cast<TagDecl>(DCAsDecl)->isBeingDefined()))
    if (ASTMutationListener *L = DCAsDecl->getASTMutationListener())
      L->AddedVisibleDecl(this, D);
}

void DeclContext::makeDeclVisibleInContextImpl(NamedDecl *D, bool Internal) {
  // Find or create the stored declaration map.
  StoredDeclsMap *Map = LookupPtr;
  if (!Map) {
    ASTContext *C = &getParentASTContext();
    Map = CreateStoredDeclsMap(*C);
  }

  // If there is an external AST source, load any declarations it knows about
  // with this declaration's name.
  // If the lookup table contains an entry about this name it means that we
  // have already checked the external source.
  if (!Internal)
    if (ExternalASTSource *Source = getParentASTContext().getExternalSource())
      if (hasExternalVisibleStorage() &&
          Map->find(D->getDeclName()) == Map->end())
        Source->FindExternalVisibleDeclsByName(this, D->getDeclName());

  // Insert this declaration into the map.
  StoredDeclsList &DeclNameEntries = (*Map)[D->getDeclName()];

  if (Internal) {
    // If this is being added as part of loading an external declaration,
    // this may not be the only external declaration with this name.
    // In this case, we never try to replace an existing declaration; we'll
    // handle that when we finalize the list of declarations for this name.
    DeclNameEntries.setHasExternalDecls();
    DeclNameEntries.AddSubsequentDecl(D);
    return;
  }

  if (DeclNameEntries.isNull()) {
    DeclNameEntries.setOnlyValue(D);
    return;
  }

  if (DeclNameEntries.HandleRedeclaration(D, /*IsKnownNewer*/!Internal)) {
    // This declaration has replaced an existing one for which
    // declarationReplaces returns true.
    return;
  }

  // Put this declaration into the appropriate slot.
  DeclNameEntries.AddSubsequentDecl(D);
}

UsingDirectiveDecl *DeclContext::udir_iterator::operator*() const {
  return cast<UsingDirectiveDecl>(*I);
}

/// Returns iterator range [First, Last) of UsingDirectiveDecls stored within
/// this context.
DeclContext::udir_range DeclContext::using_directives() const {
  // FIXME: Use something more efficient than normal lookup for using
  // directives. In C++, using directives are looked up more than anything else.
  lookup_result Result = lookup(UsingDirectiveDecl::getName());
  return udir_range(Result.begin(), Result.end());
}

//===----------------------------------------------------------------------===//
// Creation and Destruction of StoredDeclsMaps.                               //
//===----------------------------------------------------------------------===//

StoredDeclsMap *DeclContext::CreateStoredDeclsMap(ASTContext &C) const {
  assert(!LookupPtr && "context already has a decls map");
  assert(getPrimaryContext() == this &&
         "creating decls map on non-primary context");

  StoredDeclsMap *M;
  bool Dependent = isDependentContext();
  if (Dependent)
    M = new DependentStoredDeclsMap();
  else
    M = new StoredDeclsMap();
  M->Previous = C.LastSDM;
  C.LastSDM = llvm::PointerIntPair<StoredDeclsMap*,1>(M, Dependent);
  LookupPtr = M;
  return M;
}

void ASTContext::ReleaseDeclContextMaps() {
  // It's okay to delete DependentStoredDeclsMaps via a StoredDeclsMap
  // pointer because the subclass doesn't add anything that needs to
  // be deleted.
  StoredDeclsMap::DestroyAll(LastSDM.getPointer(), LastSDM.getInt());
}

void StoredDeclsMap::DestroyAll(StoredDeclsMap *Map, bool Dependent) {
  while (Map) {
    // Advance the iteration before we invalidate memory.
    llvm::PointerIntPair<StoredDeclsMap*,1> Next = Map->Previous;

    if (Dependent)
      delete static_cast<DependentStoredDeclsMap*>(Map);
    else
      delete Map;

    Map = Next.getPointer();
    Dependent = Next.getInt();
  }
}

DependentDiagnostic *DependentDiagnostic::Create(ASTContext &C,
                                                 DeclContext *Parent,
                                           const PartialDiagnostic &PDiag) {
  assert(Parent->isDependentContext()
         && "cannot iterate dependent diagnostics of non-dependent context");
  Parent = Parent->getPrimaryContext();
  if (!Parent->LookupPtr)
    Parent->CreateStoredDeclsMap(C);

  DependentStoredDeclsMap *Map =
      static_cast<DependentStoredDeclsMap *>(Parent->LookupPtr);

  // Allocate the copy of the PartialDiagnostic via the ASTContext's
  // BumpPtrAllocator, rather than the ASTContext itself.
  PartialDiagnostic::Storage *DiagStorage = nullptr;
  if (PDiag.hasStorage())
    DiagStorage = new (C) PartialDiagnostic::Storage;
  
  DependentDiagnostic *DD = new (C) DependentDiagnostic(PDiag, DiagStorage);

  // TODO: Maybe we shouldn't reverse the order during insertion.
  DD->NextDiagnostic = Map->FirstDiagnostic;
  Map->FirstDiagnostic = DD;

  return DD;
}
