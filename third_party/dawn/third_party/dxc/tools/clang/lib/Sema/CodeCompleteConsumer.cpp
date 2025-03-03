//===--- CodeCompleteConsumer.cpp - Code Completion Interface ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the CodeCompleteConsumer class.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/CodeCompleteConsumer.h"
#include "clang-c/Index.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstring>
#include <functional>
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
using namespace clang;

//===----------------------------------------------------------------------===//
// Code completion context implementation
//===----------------------------------------------------------------------===//

bool CodeCompletionContext::wantConstructorResults() const {
  switch (Kind) {
  case CCC_Recovery:
  case CCC_Statement:
  case CCC_Expression:
  case CCC_ObjCMessageReceiver:
  case CCC_ParenthesizedExpression:
    return true;
    
  case CCC_TopLevel:
  case CCC_ObjCInterface:
  case CCC_ObjCImplementation:
  case CCC_ObjCIvarList:
  case CCC_ClassStructUnion:
  case CCC_DotMemberAccess:
  case CCC_ArrowMemberAccess:
  case CCC_ObjCPropertyAccess:
  case CCC_EnumTag:
  case CCC_UnionTag:
  case CCC_ClassOrStructTag:
  case CCC_ObjCProtocolName:
  case CCC_Namespace:
  case CCC_Type:
  case CCC_Name:
  case CCC_PotentiallyQualifiedName:
  case CCC_MacroName:
  case CCC_MacroNameUse:
  case CCC_PreprocessorExpression:
  case CCC_PreprocessorDirective:
  case CCC_NaturalLanguage:
  case CCC_SelectorName:
  case CCC_TypeQualifiers:
  case CCC_Other:
  case CCC_OtherWithMacros:
  case CCC_ObjCInstanceMessage:
  case CCC_ObjCClassMessage:
  case CCC_ObjCInterfaceName:
  case CCC_ObjCCategoryName:
    return false;
  }

  llvm_unreachable("Invalid CodeCompletionContext::Kind!");
}

//===----------------------------------------------------------------------===//
// Code completion string implementation
//===----------------------------------------------------------------------===//
CodeCompletionString::Chunk::Chunk(ChunkKind Kind, const char *Text) 
  : Kind(Kind), Text("")
{
  switch (Kind) {
  case CK_TypedText:
  case CK_Text:
  case CK_Placeholder:
  case CK_Informative:
  case CK_ResultType:
  case CK_CurrentParameter:
    this->Text = Text;
    break;

  case CK_Optional:
    llvm_unreachable("Optional strings cannot be created from text");
      
  case CK_LeftParen:
    this->Text = "(";
    break;

  case CK_RightParen:
    this->Text = ")";
    break;

  case CK_LeftBracket:
    this->Text = "[";
    break;
    
  case CK_RightBracket:
    this->Text = "]";
    break;
    
  case CK_LeftBrace:
    this->Text = "{";
    break;

  case CK_RightBrace:
    this->Text = "}";
    break;

  case CK_LeftAngle:
    this->Text = "<";
    break;
    
  case CK_RightAngle:
    this->Text = ">";
    break;
      
  case CK_Comma:
    this->Text = ", ";
    break;

  case CK_Colon:
    this->Text = ":";
    break;

  case CK_SemiColon:
    this->Text = ";";
    break;

  case CK_Equal:
    this->Text = " = ";
    break;

  case CK_HorizontalSpace:
    this->Text = " ";
    break;

  case CK_VerticalSpace:
    this->Text = "\n";
    break;
  }
}

CodeCompletionString::Chunk
CodeCompletionString::Chunk::CreateText(const char *Text) {
  return Chunk(CK_Text, Text);
}

CodeCompletionString::Chunk 
CodeCompletionString::Chunk::CreateOptional(CodeCompletionString *Optional) {
  Chunk Result;
  Result.Kind = CK_Optional;
  Result.Optional = Optional;
  return Result;
}

CodeCompletionString::Chunk 
CodeCompletionString::Chunk::CreatePlaceholder(const char *Placeholder) {
  return Chunk(CK_Placeholder, Placeholder);
}

CodeCompletionString::Chunk 
CodeCompletionString::Chunk::CreateInformative(const char *Informative) {
  return Chunk(CK_Informative, Informative);
}

CodeCompletionString::Chunk 
CodeCompletionString::Chunk::CreateResultType(const char *ResultType) {
  return Chunk(CK_ResultType, ResultType);
}

CodeCompletionString::Chunk 
CodeCompletionString::Chunk::CreateCurrentParameter(
                                                const char *CurrentParameter) {
  return Chunk(CK_CurrentParameter, CurrentParameter);
}

CodeCompletionString::CodeCompletionString(const Chunk *Chunks, 
                                           unsigned NumChunks,
                                           unsigned Priority, 
                                           CXAvailabilityKind Availability,
                                           const char **Annotations,
                                           unsigned NumAnnotations,
                                           StringRef ParentName,
                                           const char *BriefComment)
  : NumChunks(NumChunks), NumAnnotations(NumAnnotations),
    Priority(Priority), Availability(Availability),
    ParentName(ParentName), BriefComment(BriefComment)
{ 
  assert(NumChunks <= 0xffff);
  assert(NumAnnotations <= 0xffff);

  Chunk *StoredChunks = reinterpret_cast<Chunk *>(this + 1);
  for (unsigned I = 0; I != NumChunks; ++I)
    StoredChunks[I] = Chunks[I];

  const char **StoredAnnotations = reinterpret_cast<const char **>(StoredChunks + NumChunks);
  for (unsigned I = 0; I != NumAnnotations; ++I)
    StoredAnnotations[I] = Annotations[I];
}

unsigned CodeCompletionString::getAnnotationCount() const {
  return NumAnnotations;
}

const char *CodeCompletionString::getAnnotation(unsigned AnnotationNr) const {
  if (AnnotationNr < NumAnnotations)
    return reinterpret_cast<const char * const*>(end())[AnnotationNr];
  else
    return nullptr;
}


std::string CodeCompletionString::getAsString() const {
  std::string Result;
  llvm::raw_string_ostream OS(Result);
                          
  for (iterator C = begin(), CEnd = end(); C != CEnd; ++C) {
    switch (C->Kind) {
    case CK_Optional: OS << "{#" << C->Optional->getAsString() << "#}"; break;
    case CK_Placeholder: OS << "<#" << C->Text << "#>"; break;
        
    case CK_Informative: 
    case CK_ResultType:
      OS << "[#" << C->Text << "#]"; 
      break;
        
    case CK_CurrentParameter: OS << "<#" << C->Text << "#>"; break;
    default: OS << C->Text; break;
    }
  }
  return OS.str();
}

const char *CodeCompletionString::getTypedText() const {
  for (iterator C = begin(), CEnd = end(); C != CEnd; ++C)
    if (C->Kind == CK_TypedText)
      return C->Text;

  return nullptr;
}

const char *CodeCompletionAllocator::CopyString(const Twine &String) {
  SmallString<128> Data;
  StringRef Ref = String.toStringRef(Data);
  // FIXME: It would be more efficient to teach Twine to tell us its size and
  // then add a routine there to fill in an allocated char* with the contents
  // of the string.
  char *Mem = (char *)Allocate(Ref.size() + 1, 1);
  std::copy(Ref.begin(), Ref.end(), Mem);
  Mem[Ref.size()] = 0;
  return Mem;
}

StringRef CodeCompletionTUInfo::getParentName(const DeclContext *DC) {
  const NamedDecl *ND = dyn_cast<NamedDecl>(DC);
  if (!ND)
    return StringRef();
  
  // Check whether we've already cached the parent name.
  StringRef &CachedParentName = ParentNames[DC];
  if (!CachedParentName.empty())
    return CachedParentName;

  // If we already processed this DeclContext and assigned empty to it, the
  // data pointer will be non-null.
  if (CachedParentName.data() != nullptr)
    return StringRef();

  // Find the interesting names.
  SmallVector<const DeclContext *, 2> Contexts;
  while (DC && !DC->isFunctionOrMethod()) {
    if (const NamedDecl *ND = dyn_cast<NamedDecl>(DC)) {
      if (ND->getIdentifier())
        Contexts.push_back(DC);
    }
    
    DC = DC->getParent();
  }

  {
    SmallString<128> S;
    llvm::raw_svector_ostream OS(S);
    bool First = true;
    for (unsigned I = Contexts.size(); I != 0; --I) {
      if (First)
        First = false;
      else {
        OS << "::";
      }
      
      const DeclContext *CurDC = Contexts[I-1];
      if (const ObjCCategoryImplDecl *CatImpl = dyn_cast<ObjCCategoryImplDecl>(CurDC))
        CurDC = CatImpl->getCategoryDecl();
      
      if (const ObjCCategoryDecl *Cat = dyn_cast<ObjCCategoryDecl>(CurDC)) {
        const ObjCInterfaceDecl *Interface = Cat->getClassInterface();
        if (!Interface) {
          // Assign an empty StringRef but with non-null data to distinguish
          // between empty because we didn't process the DeclContext yet.
          CachedParentName = StringRef((const char *)~(uintptr_t)0U, 0); // HLSL Change
          return StringRef();
        }
        
        OS << Interface->getName() << '(' << Cat->getName() << ')';
      } else {
        OS << cast<NamedDecl>(CurDC)->getName();
      }
    }
    
    CachedParentName = AllocatorRef->CopyString(OS.str());
  }

  return CachedParentName;
}

CodeCompletionString *CodeCompletionBuilder::TakeString() {
  void *Mem = getAllocator().Allocate(
                  sizeof(CodeCompletionString) + sizeof(Chunk) * Chunks.size()
                                    + sizeof(const char *) * Annotations.size(),
                                 llvm::alignOf<CodeCompletionString>());
  CodeCompletionString *Result 
    = new (Mem) CodeCompletionString(Chunks.data(), Chunks.size(),
                                     Priority, Availability,
                                     Annotations.data(), Annotations.size(),
                                     ParentName, BriefComment);
  Chunks.clear();
  return Result;
}

void CodeCompletionBuilder::AddTypedTextChunk(const char *Text) {
  Chunks.push_back(Chunk(CodeCompletionString::CK_TypedText, Text));
}

void CodeCompletionBuilder::AddTextChunk(const char *Text) {
  Chunks.push_back(Chunk::CreateText(Text));
}

void CodeCompletionBuilder::AddOptionalChunk(CodeCompletionString *Optional) {
  Chunks.push_back(Chunk::CreateOptional(Optional));
}

void CodeCompletionBuilder::AddPlaceholderChunk(const char *Placeholder) {
  Chunks.push_back(Chunk::CreatePlaceholder(Placeholder));
}

void CodeCompletionBuilder::AddInformativeChunk(const char *Text) {
  Chunks.push_back(Chunk::CreateInformative(Text));
}

void CodeCompletionBuilder::AddResultTypeChunk(const char *ResultType) {
  Chunks.push_back(Chunk::CreateResultType(ResultType));
}

void
CodeCompletionBuilder::AddCurrentParameterChunk(const char *CurrentParameter) {
  Chunks.push_back(Chunk::CreateCurrentParameter(CurrentParameter));
}

void CodeCompletionBuilder::AddChunk(CodeCompletionString::ChunkKind CK,
                                     const char *Text) {
  Chunks.push_back(Chunk(CK, Text));
}

void CodeCompletionBuilder::addParentContext(const DeclContext *DC) {
  if (DC->isTranslationUnit()) {
    return;
  }
  
  if (DC->isFunctionOrMethod())
    return;
  
  const NamedDecl *ND = dyn_cast<NamedDecl>(DC);
  if (!ND)
    return;
  
  ParentName = getCodeCompletionTUInfo().getParentName(DC);
}

void CodeCompletionBuilder::addBriefComment(StringRef Comment) {
  BriefComment = Allocator.CopyString(Comment);
}

//===----------------------------------------------------------------------===//
// Code completion overload candidate implementation
//===----------------------------------------------------------------------===//
FunctionDecl *
CodeCompleteConsumer::OverloadCandidate::getFunction() const {
  if (getKind() == CK_Function)
    return Function;
  else if (getKind() == CK_FunctionTemplate)
    return FunctionTemplate->getTemplatedDecl();
  else
    return nullptr;
}

const FunctionType *
CodeCompleteConsumer::OverloadCandidate::getFunctionType() const {
  switch (Kind) {
  case CK_Function:
    return Function->getType()->getAs<FunctionType>();
      
  case CK_FunctionTemplate:
    return FunctionTemplate->getTemplatedDecl()->getType()
             ->getAs<FunctionType>();
      
  case CK_FunctionType:
    return Type;
  }

  llvm_unreachable("Invalid CandidateKind!");
}

//===----------------------------------------------------------------------===//
// Code completion consumer implementation
//===----------------------------------------------------------------------===//

CodeCompleteConsumer::~CodeCompleteConsumer() { }

void 
PrintingCodeCompleteConsumer::ProcessCodeCompleteResults(Sema &SemaRef,
                                                 CodeCompletionContext Context,
                                                 CodeCompletionResult *Results,
                                                         unsigned NumResults) {
  std::stable_sort(Results, Results + NumResults);
  
  // Print the results.
  for (unsigned I = 0; I != NumResults; ++I) {
    OS << "COMPLETION: ";
    switch (Results[I].Kind) {
    case CodeCompletionResult::RK_Declaration:
      OS << *Results[I].Declaration;
      if (Results[I].Hidden)
        OS << " (Hidden)";
      if (CodeCompletionString *CCS 
            = Results[I].CreateCodeCompletionString(SemaRef, Context,
                                                    getAllocator(),
                                                    CCTUInfo,
                                                    includeBriefComments())) {
        OS << " : " << CCS->getAsString();
        if (const char *BriefComment = CCS->getBriefComment())
          OS << " : " << BriefComment;
      }
        
      OS << '\n';
      break;
      
    case CodeCompletionResult::RK_Keyword:
      OS << Results[I].Keyword << '\n';
      break;
        
    case CodeCompletionResult::RK_Macro: {
      OS << Results[I].Macro->getName();
      if (CodeCompletionString *CCS 
            = Results[I].CreateCodeCompletionString(SemaRef, Context,
                                                    getAllocator(),
                                                    CCTUInfo,
                                                    includeBriefComments())) {
        OS << " : " << CCS->getAsString();
      }
      OS << '\n';
      break;
    }
        
    case CodeCompletionResult::RK_Pattern: {
      OS << "Pattern : " 
         << Results[I].Pattern->getAsString() << '\n';
      break;
    }
    }
  }
}

// This function is used solely to preserve the former presentation of overloads
// by "clang -cc1 -code-completion-at", since CodeCompletionString::getAsString
// needs to be improved for printing the newer and more detailed overload
// chunks.
static std::string getOverloadAsString(const CodeCompletionString &CCS) {
  std::string Result;
  llvm::raw_string_ostream OS(Result);

  for (auto &C : CCS) {
    switch (C.Kind) {
    case CodeCompletionString::CK_Informative:
    case CodeCompletionString::CK_ResultType:
      OS << "[#" << C.Text << "#]";
      break;

    case CodeCompletionString::CK_CurrentParameter:
      OS << "<#" << C.Text << "#>";
      break;

    default: OS << C.Text; break;
    }
  }
  return OS.str();
}

void 
PrintingCodeCompleteConsumer::ProcessOverloadCandidates(Sema &SemaRef,
                                                        unsigned CurrentArg,
                                              OverloadCandidate *Candidates,
                                                     unsigned NumCandidates) {
  for (unsigned I = 0; I != NumCandidates; ++I) {
    if (CodeCompletionString *CCS
          = Candidates[I].CreateSignatureString(CurrentArg, SemaRef,
                                                getAllocator(), CCTUInfo,
                                                includeBriefComments())) {
      OS << "OVERLOAD: " << getOverloadAsString(*CCS) << "\n";
    }
  }
}

/// \brief Retrieve the effective availability of the given declaration.
static AvailabilityResult getDeclAvailability(const Decl *D) {
  AvailabilityResult AR = D->getAvailability();
  if (isa<EnumConstantDecl>(D))
    AR = std::max(AR, cast<Decl>(D->getDeclContext())->getAvailability());
  return AR;
}

void CodeCompletionResult::computeCursorKindAndAvailability(bool Accessible) {
  switch (Kind) {
  case RK_Pattern:
    if (!Declaration) {
      // Do nothing: Patterns can come with cursor kinds!
      break;
    }
    LLVM_FALLTHROUGH; // HLSL Change
      
  case RK_Declaration: {
    // Set the availability based on attributes.
    switch (getDeclAvailability(Declaration)) {
    case AR_Available:
    case AR_NotYetIntroduced:
      Availability = CXAvailability_Available;      
      break;
      
    case AR_Deprecated:
      Availability = CXAvailability_Deprecated;
      break;
      
    case AR_Unavailable:
      Availability = CXAvailability_NotAvailable;
      break;
    }

    if (const FunctionDecl *Function = dyn_cast<FunctionDecl>(Declaration))
      if (Function->isDeleted())
        Availability = CXAvailability_NotAvailable;
      
    CursorKind = getCursorKindForDecl(Declaration);
    if (CursorKind == CXCursor_UnexposedDecl) {
      // FIXME: Forward declarations of Objective-C classes and protocols 
      // are not directly exposed, but we want code completion to treat them 
      // like a definition.
      if (isa<ObjCInterfaceDecl>(Declaration))
        CursorKind = CXCursor_ObjCInterfaceDecl;
      else if (isa<ObjCProtocolDecl>(Declaration))
        CursorKind = CXCursor_ObjCProtocolDecl;
      else
        CursorKind = CXCursor_NotImplemented;
    }
    break;
  }

  case RK_Macro:
  case RK_Keyword:
    llvm_unreachable("Macro and keyword kinds are handled by the constructors");
  }

  if (!Accessible)
    Availability = CXAvailability_NotAccessible;
}

/// \brief Retrieve the name that should be used to order a result.
///
/// If the name needs to be constructed as a string, that string will be
/// saved into Saved and the returned StringRef will refer to it.
static StringRef getOrderedName(const CodeCompletionResult &R,
                                    std::string &Saved) {
  switch (R.Kind) {
    case CodeCompletionResult::RK_Keyword:
      return R.Keyword;
      
    case CodeCompletionResult::RK_Pattern:
      return R.Pattern->getTypedText();
      
    case CodeCompletionResult::RK_Macro:
      return R.Macro->getName();
      
    case CodeCompletionResult::RK_Declaration:
      // Handle declarations below.
      break;
  }
  
  DeclarationName Name = R.Declaration->getDeclName();
  
  // If the name is a simple identifier (by far the common case), or a
  // zero-argument selector, just return a reference to that identifier.
  if (IdentifierInfo *Id = Name.getAsIdentifierInfo())
    return Id->getName();
  if (Name.isObjCZeroArgSelector())
    if (IdentifierInfo *Id
        = Name.getObjCSelector().getIdentifierInfoForSlot(0))
      return Id->getName();
  
  Saved = Name.getAsString();
  return Saved;
}
    
bool clang::operator<(const CodeCompletionResult &X, 
                      const CodeCompletionResult &Y) {
  std::string XSaved, YSaved;
  StringRef XStr = getOrderedName(X, XSaved);
  StringRef YStr = getOrderedName(Y, YSaved);
  int cmp = XStr.compare_lower(YStr);
  if (cmp)
    return cmp < 0;
  
  // If case-insensitive comparison fails, try case-sensitive comparison.
  cmp = XStr.compare(YStr);
  if (cmp)
    return cmp < 0;
  
  return false;
}
