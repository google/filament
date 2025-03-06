//===--- NestedNameSpecifier.cpp - C++ nested name specifiers -----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the NestedNameSpecifier class, which represents
//  a C++ nested-name-specifier.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLoc.h"
#include "llvm/Support/AlignOf.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
using namespace clang;

NestedNameSpecifier *
NestedNameSpecifier::FindOrInsert(const ASTContext &Context,
                                  const NestedNameSpecifier &Mockup) {
  llvm::FoldingSetNodeID ID;
  Mockup.Profile(ID);

  void *InsertPos = nullptr;
  NestedNameSpecifier *NNS
    = Context.NestedNameSpecifiers.FindNodeOrInsertPos(ID, InsertPos);
  if (!NNS) {
    NNS = new (Context, llvm::alignOf<NestedNameSpecifier>())
        NestedNameSpecifier(Mockup);
    Context.NestedNameSpecifiers.InsertNode(NNS, InsertPos);
  }

  return NNS;
}

NestedNameSpecifier *
NestedNameSpecifier::Create(const ASTContext &Context,
                            NestedNameSpecifier *Prefix, IdentifierInfo *II) {
  assert(II && "Identifier cannot be NULL");
  assert((!Prefix || Prefix->isDependent()) && "Prefix must be dependent");

  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(Prefix);
  Mockup.Prefix.setInt(StoredIdentifier);
  Mockup.Specifier = II;
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier *
NestedNameSpecifier::Create(const ASTContext &Context,
                            NestedNameSpecifier *Prefix,
                            const NamespaceDecl *NS) {
  assert(NS && "Namespace cannot be NULL");
  assert((!Prefix ||
          (Prefix->getAsType() == nullptr &&
           Prefix->getAsIdentifier() == nullptr)) &&
         "Broken nested name specifier");
  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(Prefix);
  Mockup.Prefix.setInt(StoredDecl);
  Mockup.Specifier = const_cast<NamespaceDecl *>(NS);
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier *
NestedNameSpecifier::Create(const ASTContext &Context,
                            NestedNameSpecifier *Prefix, 
                            NamespaceAliasDecl *Alias) {
  assert(Alias && "Namespace alias cannot be NULL");
  assert((!Prefix ||
          (Prefix->getAsType() == nullptr &&
           Prefix->getAsIdentifier() == nullptr)) &&
         "Broken nested name specifier");
  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(Prefix);
  Mockup.Prefix.setInt(StoredDecl);
  Mockup.Specifier = Alias;
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier *
NestedNameSpecifier::Create(const ASTContext &Context,
                            NestedNameSpecifier *Prefix,
                            bool Template, const Type *T) {
  assert(T && "Type cannot be NULL");
  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(Prefix);
  Mockup.Prefix.setInt(Template? StoredTypeSpecWithTemplate : StoredTypeSpec);
  Mockup.Specifier = const_cast<Type*>(T);
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier *
NestedNameSpecifier::Create(const ASTContext &Context, IdentifierInfo *II) {
  assert(II && "Identifier cannot be NULL");
  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(nullptr);
  Mockup.Prefix.setInt(StoredIdentifier);
  Mockup.Specifier = II;
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier *
NestedNameSpecifier::GlobalSpecifier(const ASTContext &Context) {
  if (!Context.GlobalNestedNameSpecifier)
    Context.GlobalNestedNameSpecifier =
        new (Context, llvm::alignOf<NestedNameSpecifier>())
            NestedNameSpecifier();
  return Context.GlobalNestedNameSpecifier;
}

NestedNameSpecifier *
NestedNameSpecifier::SuperSpecifier(const ASTContext &Context,
                                    CXXRecordDecl *RD) {
  NestedNameSpecifier Mockup;
  Mockup.Prefix.setPointer(nullptr);
  Mockup.Prefix.setInt(StoredDecl);
  Mockup.Specifier = RD;
  return FindOrInsert(Context, Mockup);
}

NestedNameSpecifier::SpecifierKind NestedNameSpecifier::getKind() const {
  if (!Specifier)
    return Global;

  switch (Prefix.getInt()) {
  case StoredIdentifier:
    return Identifier;

  case StoredDecl: {
    NamedDecl *ND = static_cast<NamedDecl *>(Specifier);
    if (isa<CXXRecordDecl>(ND))
      return Super;
    return isa<NamespaceDecl>(ND) ? Namespace : NamespaceAlias;
  }

  case StoredTypeSpec:
    return TypeSpec;

  case StoredTypeSpecWithTemplate:
    return TypeSpecWithTemplate;
  }

  llvm_unreachable("Invalid NNS Kind!");
}

/// \brief Retrieve the namespace stored in this nested name specifier.
NamespaceDecl *NestedNameSpecifier::getAsNamespace() const {
	if (Prefix.getInt() == StoredDecl)
    return dyn_cast<NamespaceDecl>(static_cast<NamedDecl *>(Specifier));

  return nullptr;
}

/// \brief Retrieve the namespace alias stored in this nested name specifier.
NamespaceAliasDecl *NestedNameSpecifier::getAsNamespaceAlias() const {
	if (Prefix.getInt() == StoredDecl)
    return dyn_cast<NamespaceAliasDecl>(static_cast<NamedDecl *>(Specifier));

  return nullptr;
}

/// \brief Retrieve the record declaration stored in this nested name specifier.
CXXRecordDecl *NestedNameSpecifier::getAsRecordDecl() const {
  if (Prefix.getInt() == StoredDecl)
    return dyn_cast<CXXRecordDecl>(static_cast<NamedDecl *>(Specifier));

  return nullptr;
}

/// \brief Whether this nested name specifier refers to a dependent
/// type or not.
bool NestedNameSpecifier::isDependent() const {
  switch (getKind()) {
  case Identifier:
    // Identifier specifiers always represent dependent types
    return true;

  case Namespace:
  case NamespaceAlias:
  case Global:
    return false;

  case Super: {
    CXXRecordDecl *RD = static_cast<CXXRecordDecl *>(Specifier);
    for (const auto &Base : RD->bases())
      if (Base.getType()->isDependentType())
        return true;

    return false;
  }

  case TypeSpec:
  case TypeSpecWithTemplate:
    return getAsType()->isDependentType();
  }

  llvm_unreachable("Invalid NNS Kind!");
}

/// \brief Whether this nested name specifier refers to a dependent
/// type or not.
bool NestedNameSpecifier::isInstantiationDependent() const {
  switch (getKind()) {
  case Identifier:
    // Identifier specifiers always represent dependent types
    return true;
    
  case Namespace:
  case NamespaceAlias:
  case Global:
  case Super:
    return false;

  case TypeSpec:
  case TypeSpecWithTemplate:
    return getAsType()->isInstantiationDependentType();
  }

  llvm_unreachable("Invalid NNS Kind!");
}

bool NestedNameSpecifier::containsUnexpandedParameterPack() const {
  switch (getKind()) {
  case Identifier:
    return getPrefix() && getPrefix()->containsUnexpandedParameterPack();

  case Namespace:
  case NamespaceAlias:
  case Global:
  case Super:
    return false;

  case TypeSpec:
  case TypeSpecWithTemplate:
    return getAsType()->containsUnexpandedParameterPack();
  }

  llvm_unreachable("Invalid NNS Kind!");
}

/// \brief Print this nested name specifier to the given output
/// stream.
void
NestedNameSpecifier::print(raw_ostream &OS,
                           const PrintingPolicy &Policy) const {
  if (getPrefix())
    getPrefix()->print(OS, Policy);

  switch (getKind()) {
  case Identifier:
    OS << getAsIdentifier()->getName();
    break;

  case Namespace:
    if (getAsNamespace()->isAnonymousNamespace())
      return;
      
    OS << getAsNamespace()->getName();
    break;

  case NamespaceAlias:
    OS << getAsNamespaceAlias()->getName();
    break;

  case Global:
    break;

  case Super:
    OS << "__super";
    break;

  case TypeSpecWithTemplate:
    OS << "template ";
    // Fall through to print the type.
    LLVM_FALLTHROUGH; // HLSL Change

  case TypeSpec: {
    const Type *T = getAsType();

    PrintingPolicy InnerPolicy(Policy);
    InnerPolicy.SuppressScope = true;

    // Nested-name-specifiers are intended to contain minimally-qualified
    // types. An actual ElaboratedType will not occur, since we'll store
    // just the type that is referred to in the nested-name-specifier (e.g.,
    // a TypedefType, TagType, etc.). However, when we are dealing with
    // dependent template-id types (e.g., Outer<T>::template Inner<U>),
    // the type requires its own nested-name-specifier for uniqueness, so we
    // suppress that nested-name-specifier during printing.
    assert(!isa<ElaboratedType>(T) &&
           "Elaborated type in nested-name-specifier");
    if (const TemplateSpecializationType *SpecType
          = dyn_cast<TemplateSpecializationType>(T)) {
      // Print the template name without its corresponding
      // nested-name-specifier.
      SpecType->getTemplateName().print(OS, InnerPolicy, true);

      // Print the template argument list.
      TemplateSpecializationType::PrintTemplateArgumentList(
          OS, SpecType->getArgs(), SpecType->getNumArgs(), InnerPolicy);
    } else {
      // Print the type normally
      QualType(T, 0).print(OS, InnerPolicy);
    }
    break;
  }
  }

  OS << "::";
}

void NestedNameSpecifier::dump(const LangOptions &LO) {
  print(llvm::errs(), PrintingPolicy(LO));
}

unsigned 
NestedNameSpecifierLoc::getLocalDataLength(NestedNameSpecifier *Qualifier) {
  assert(Qualifier && "Expected a non-NULL qualifier");

  // Location of the trailing '::'.
  unsigned Length = sizeof(unsigned);

  switch (Qualifier->getKind()) {
  case NestedNameSpecifier::Global:
    // Nothing more to add.
    break;

  case NestedNameSpecifier::Identifier:
  case NestedNameSpecifier::Namespace:
  case NestedNameSpecifier::NamespaceAlias:
  case NestedNameSpecifier::Super:
    // The location of the identifier or namespace name.
    Length += sizeof(unsigned);
    break;

  case NestedNameSpecifier::TypeSpecWithTemplate:
  case NestedNameSpecifier::TypeSpec:
    // The "void*" that points at the TypeLoc data.
    // Note: the 'template' keyword is part of the TypeLoc.
    Length += sizeof(void *);
    break;
  }

  return Length;
}

unsigned 
NestedNameSpecifierLoc::getDataLength(NestedNameSpecifier *Qualifier) {
  unsigned Length = 0;
  for (; Qualifier; Qualifier = Qualifier->getPrefix())
    Length += getLocalDataLength(Qualifier);
  return Length;
}

namespace {
  /// \brief Load a (possibly unaligned) source location from a given address
  /// and offset.
  SourceLocation LoadSourceLocation(void *Data, unsigned Offset) {
    unsigned Raw;
    memcpy(&Raw, static_cast<char *>(Data) + Offset, sizeof(unsigned));
    return SourceLocation::getFromRawEncoding(Raw);
  }
  
  /// \brief Load a (possibly unaligned) pointer from a given address and
  /// offset.
  void *LoadPointer(void *Data, unsigned Offset) {
    void *Result;
    memcpy(&Result, static_cast<char *>(Data) + Offset, sizeof(void*));
    return Result;
  }
}

SourceRange NestedNameSpecifierLoc::getSourceRange() const {
  if (!Qualifier)
    return SourceRange();
  
  NestedNameSpecifierLoc First = *this;
  while (NestedNameSpecifierLoc Prefix = First.getPrefix())
    First = Prefix;
  
  return SourceRange(First.getLocalSourceRange().getBegin(), 
                     getLocalSourceRange().getEnd());
}

SourceRange NestedNameSpecifierLoc::getLocalSourceRange() const {
  if (!Qualifier)
    return SourceRange();
  
  unsigned Offset = getDataLength(Qualifier->getPrefix());
  switch (Qualifier->getKind()) {
  case NestedNameSpecifier::Global:
    return LoadSourceLocation(Data, Offset);

  case NestedNameSpecifier::Identifier:
  case NestedNameSpecifier::Namespace:
  case NestedNameSpecifier::NamespaceAlias:
  case NestedNameSpecifier::Super:
    return SourceRange(LoadSourceLocation(Data, Offset),
                       LoadSourceLocation(Data, Offset + sizeof(unsigned)));

  case NestedNameSpecifier::TypeSpecWithTemplate:
  case NestedNameSpecifier::TypeSpec: {
    // The "void*" that points at the TypeLoc data.
    // Note: the 'template' keyword is part of the TypeLoc.
    void *TypeData = LoadPointer(Data, Offset);
    TypeLoc TL(Qualifier->getAsType(), TypeData);
    return SourceRange(TL.getBeginLoc(),
                       LoadSourceLocation(Data, Offset + sizeof(void*)));
  }
  }

  llvm_unreachable("Invalid NNS Kind!");
}

TypeLoc NestedNameSpecifierLoc::getTypeLoc() const {
  assert((Qualifier->getKind() == NestedNameSpecifier::TypeSpec ||
          Qualifier->getKind() == NestedNameSpecifier::TypeSpecWithTemplate) &&
         "Nested-name-specifier location is not a type");

  // The "void*" that points at the TypeLoc data.
  unsigned Offset = getDataLength(Qualifier->getPrefix());
  void *TypeData = LoadPointer(Data, Offset);
  return TypeLoc(Qualifier->getAsType(), TypeData);
}

namespace {
void Append(char *Start, char *End, char *&Buffer, unsigned &BufferSize,
            unsigned &BufferCapacity) {
  if (Start == End)
    return;

  if (BufferSize + (End - Start) > BufferCapacity) {
    // Reallocate the buffer.
    unsigned NewCapacity = std::max(
        (unsigned)(BufferCapacity ? BufferCapacity * 2 : sizeof(void *) * 2),
        (unsigned)(BufferSize + (End - Start)));
    char *NewBuffer =
        new char[NewCapacity]; // HLSL Change: Use overridable operator new
    if (BufferCapacity) {
      memcpy(NewBuffer, Buffer, BufferSize);
      delete[] Buffer; // HLSL Change: Use overridable operator delete
    }
    Buffer = NewBuffer;
    BufferCapacity = NewCapacity;
  }

  memcpy(Buffer + BufferSize, Start, End - Start);
  BufferSize += End - Start;
}

  /// \brief Save a source location to the given buffer.
void SaveSourceLocation(SourceLocation Loc, char *&Buffer, unsigned &BufferSize,
                        unsigned &BufferCapacity) {
  unsigned Raw = Loc.getRawEncoding();
  Append(reinterpret_cast<char *>(&Raw),
         reinterpret_cast<char *>(&Raw) + sizeof(unsigned), Buffer, BufferSize,
         BufferCapacity);
}

  /// \brief Save a pointer to the given buffer.
void SavePointer(void *Ptr, char *&Buffer, unsigned &BufferSize,
                 unsigned &BufferCapacity) {
  Append(reinterpret_cast<char *>(&Ptr),
         reinterpret_cast<char *>(&Ptr) + sizeof(void *), Buffer, BufferSize,
         BufferCapacity);
}
}

NestedNameSpecifierLocBuilder::
NestedNameSpecifierLocBuilder(const NestedNameSpecifierLocBuilder &Other) 
  : Representation(Other.Representation), Buffer(nullptr),
    BufferSize(0), BufferCapacity(0)
{
  if (!Other.Buffer)
    return;
  
  if (Other.BufferCapacity == 0) {
    // Shallow copy is okay.
    Buffer = Other.Buffer;
    BufferSize = Other.BufferSize;
    return;
  }
  
  // Deep copy
  Append(Other.Buffer, Other.Buffer + Other.BufferSize, Buffer, BufferSize,
         BufferCapacity);
}

NestedNameSpecifierLocBuilder &
NestedNameSpecifierLocBuilder::
operator=(const NestedNameSpecifierLocBuilder &Other) {
  Representation = Other.Representation;
  
  if (Buffer && Other.Buffer && BufferCapacity >= Other.BufferSize) {
    // Re-use our storage.
    BufferSize = Other.BufferSize;
    memcpy(Buffer, Other.Buffer, BufferSize);
    return *this;
  }
  
  // Free our storage, if we have any.
  if (BufferCapacity) {
    delete[] Buffer; // HLSL Change: Use overridable operator delete
    BufferCapacity = 0;
  }
  
  if (!Other.Buffer) {
    // Empty.
    Buffer = nullptr;
    BufferSize = 0;
    return *this;
  }
  
  if (Other.BufferCapacity == 0) {
    // Shallow copy is okay.
    Buffer = Other.Buffer;
    BufferSize = Other.BufferSize;
    return *this;
  }
  
  // Deep copy.
  Append(Other.Buffer, Other.Buffer + Other.BufferSize, Buffer, BufferSize,
         BufferCapacity);
  return *this;
}

void NestedNameSpecifierLocBuilder::Extend(ASTContext &Context, 
                                           SourceLocation TemplateKWLoc, 
                                           TypeLoc TL, 
                                           SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier::Create(Context, Representation, 
                                               TemplateKWLoc.isValid(), 
                                               TL.getTypePtr());
  
  // Push source-location info into the buffer.
  SavePointer(TL.getOpaqueData(), Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::Extend(ASTContext &Context, 
                                           IdentifierInfo *Identifier,
                                           SourceLocation IdentifierLoc, 
                                           SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier::Create(Context, Representation, 
                                               Identifier);
  
  // Push source-location info into the buffer.
  SaveSourceLocation(IdentifierLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::Extend(ASTContext &Context, 
                                           NamespaceDecl *Namespace,
                                           SourceLocation NamespaceLoc, 
                                           SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier::Create(Context, Representation, 
                                               Namespace);
  
  // Push source-location info into the buffer.
  SaveSourceLocation(NamespaceLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::Extend(ASTContext &Context,
                                           NamespaceAliasDecl *Alias,
                                           SourceLocation AliasLoc, 
                                           SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier::Create(Context, Representation, Alias);
  
  // Push source-location info into the buffer.
  SaveSourceLocation(AliasLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::MakeGlobal(ASTContext &Context, 
                                               SourceLocation ColonColonLoc) {
  assert(!Representation && "Already have a nested-name-specifier!?");
  Representation = NestedNameSpecifier::GlobalSpecifier(Context);
  
  // Push source-location info into the buffer.
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::MakeSuper(ASTContext &Context,
                                              CXXRecordDecl *RD,
                                              SourceLocation SuperLoc,
                                              SourceLocation ColonColonLoc) {
  Representation = NestedNameSpecifier::SuperSpecifier(Context, RD);

  // Push source-location info into the buffer.
  SaveSourceLocation(SuperLoc, Buffer, BufferSize, BufferCapacity);
  SaveSourceLocation(ColonColonLoc, Buffer, BufferSize, BufferCapacity);
}

void NestedNameSpecifierLocBuilder::MakeTrivial(ASTContext &Context, 
                                                NestedNameSpecifier *Qualifier, 
                                                SourceRange R) {
  Representation = Qualifier;
  
  // Construct bogus (but well-formed) source information for the 
  // nested-name-specifier.
  BufferSize = 0;
  SmallVector<NestedNameSpecifier *, 4> Stack;
  for (NestedNameSpecifier *NNS = Qualifier; NNS; NNS = NNS->getPrefix())
    Stack.push_back(NNS);
  while (!Stack.empty()) {
    NestedNameSpecifier *NNS = Stack.pop_back_val();
    switch (NNS->getKind()) {
      case NestedNameSpecifier::Identifier:
      case NestedNameSpecifier::Namespace:
      case NestedNameSpecifier::NamespaceAlias:
        SaveSourceLocation(R.getBegin(), Buffer, BufferSize, BufferCapacity);
        break;
        
      case NestedNameSpecifier::TypeSpec:
      case NestedNameSpecifier::TypeSpecWithTemplate: {
        TypeSourceInfo *TSInfo
        = Context.getTrivialTypeSourceInfo(QualType(NNS->getAsType(), 0),
                                           R.getBegin());
        SavePointer(TSInfo->getTypeLoc().getOpaqueData(), Buffer, BufferSize, 
                    BufferCapacity);
        break;
      }
        
      case NestedNameSpecifier::Global:
      case NestedNameSpecifier::Super:
        break;
    }
    
    // Save the location of the '::'.
    SaveSourceLocation(Stack.empty()? R.getEnd() : R.getBegin(), 
                       Buffer, BufferSize, BufferCapacity);
  }
}

void NestedNameSpecifierLocBuilder::Adopt(NestedNameSpecifierLoc Other) {
  if (BufferCapacity)
    delete[] Buffer; // HLSL Change: Use overridable operator delete

  if (!Other) {
    Representation = nullptr;
    BufferSize = 0;
    return;
  }
  
  // Rather than copying the data (which is wasteful), "adopt" the 
  // pointer (which points into the ASTContext) but set the capacity to zero to
  // indicate that we don't own it.
  Representation = Other.getNestedNameSpecifier();
  Buffer = static_cast<char *>(Other.getOpaqueData());
  BufferSize = Other.getDataLength();
  BufferCapacity = 0;
}

NestedNameSpecifierLoc 
NestedNameSpecifierLocBuilder::getWithLocInContext(ASTContext &Context) const {
  if (!Representation)
    return NestedNameSpecifierLoc();
  
  // If we adopted our data pointer from elsewhere in the AST context, there's
  // no need to copy the memory.
  if (BufferCapacity == 0)
    return NestedNameSpecifierLoc(Representation, Buffer);
  
  // FIXME: After copying the source-location information, should we free
  // our (temporary) buffer and adopt the ASTContext-allocated memory?
  // Doing so would optimize repeated calls to getWithLocInContext().
  void *Mem = Context.Allocate(BufferSize, llvm::alignOf<void *>());
  memcpy(Mem, Buffer, BufferSize);
  return NestedNameSpecifierLoc(Representation, Mem);
}
