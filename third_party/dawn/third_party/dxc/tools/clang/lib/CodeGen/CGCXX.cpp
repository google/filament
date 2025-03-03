//===--- CGCXX.cpp - Emit LLVM Code for declarations ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with C++ code generation.
//
//===----------------------------------------------------------------------===//

#include "CodeGenModule.h"
#include "CGCXXABI.h"
#include "CodeGenFunction.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/StringExtras.h"
using namespace clang;
using namespace CodeGen;

/// Try to emit a base destructor as an alias to its primary
/// base-class destructor.
bool CodeGenModule::TryEmitBaseDestructorAsAlias(const CXXDestructorDecl *D) {
  if (!getCodeGenOpts().CXXCtorDtorAliases)
    return true;

  // Producing an alias to a base class ctor/dtor can degrade debug quality
  // as the debugger cannot tell them apart.
  if (getCodeGenOpts().OptimizationLevel == 0)
    return true;

  // If the destructor doesn't have a trivial body, we have to emit it
  // separately.
  if (!D->hasTrivialBody())
    return true;

  const CXXRecordDecl *Class = D->getParent();

  // We are going to instrument this destructor, so give up even if it is
  // currently empty.
  if (Class->mayInsertExtraPadding())
    return true;

  // If we need to manipulate a VTT parameter, give up.
  if (Class->getNumVBases()) {
    // Extra Credit:  passing extra parameters is perfectly safe
    // in many calling conventions, so only bail out if the ctor's
    // calling convention is nonstandard.
    return true;
  }

  // If any field has a non-trivial destructor, we have to emit the
  // destructor separately.
  for (const auto *I : Class->fields())
    if (I->getType().isDestructedType())
      return true;

  // Try to find a unique base class with a non-trivial destructor.
  const CXXRecordDecl *UniqueBase = nullptr;
  for (const auto &I : Class->bases()) {

    // We're in the base destructor, so skip virtual bases.
    if (I.isVirtual()) continue;

    // Skip base classes with trivial destructors.
    const auto *Base =
        cast<CXXRecordDecl>(I.getType()->getAs<RecordType>()->getDecl());
    if (Base->hasTrivialDestructor()) continue;

    // If we've already found a base class with a non-trivial
    // destructor, give up.
    if (UniqueBase) return true;
    UniqueBase = Base;
  }

  // If we didn't find any bases with a non-trivial destructor, then
  // the base destructor is actually effectively trivial, which can
  // happen if it was needlessly user-defined or if there are virtual
  // bases with non-trivial destructors.
  if (!UniqueBase)
    return true;

  // If the base is at a non-zero offset, give up.
  const ASTRecordLayout &ClassLayout = Context.getASTRecordLayout(Class);
  if (!ClassLayout.getBaseClassOffset(UniqueBase).isZero())
    return true;

  // Give up if the calling conventions don't match. We could update the call,
  // but it is probably not worth it.
  const CXXDestructorDecl *BaseD = UniqueBase->getDestructor();
  if (BaseD->getType()->getAs<FunctionType>()->getCallConv() !=
      D->getType()->getAs<FunctionType>()->getCallConv())
    return true;

  return TryEmitDefinitionAsAlias(GlobalDecl(D, Dtor_Base),
                                  GlobalDecl(BaseD, Dtor_Base),
                                  false);
}

/// Try to emit a definition as a global alias for another definition.
/// If \p InEveryTU is true, we know that an equivalent alias can be produced
/// in every translation unit.
bool CodeGenModule::TryEmitDefinitionAsAlias(GlobalDecl AliasDecl,
                                             GlobalDecl TargetDecl,
                                             bool InEveryTU) {
  if (!getCodeGenOpts().CXXCtorDtorAliases)
    return true;

  // The alias will use the linkage of the referent.  If we can't
  // support aliases with that linkage, fail.
  llvm::GlobalValue::LinkageTypes Linkage = getFunctionLinkage(AliasDecl);

  // We can't use an alias if the linkage is not valid for one.
  if (!llvm::GlobalAlias::isValidLinkage(Linkage))
    return true;

  // Don't create a weak alias for a dllexport'd symbol.
  if (AliasDecl.getDecl()->hasAttr<DLLExportAttr>() &&
      llvm::GlobalValue::isWeakForLinker(Linkage))
    return true;

  llvm::GlobalValue::LinkageTypes TargetLinkage =
      getFunctionLinkage(TargetDecl);

  // Check if we have it already.
  StringRef MangledName = getMangledName(AliasDecl);
  llvm::GlobalValue *Entry = GetGlobalValue(MangledName);
  if (Entry && !Entry->isDeclaration())
    return false;
  if (Replacements.count(MangledName))
    return false;

  // Derive the type for the alias.
  llvm::PointerType *AliasType
    = getTypes().GetFunctionType(AliasDecl)->getPointerTo();

  // Find the referent.  Some aliases might require a bitcast, in
  // which case the caller is responsible for ensuring the soundness
  // of these semantics.
  auto *Ref = cast<llvm::GlobalValue>(GetAddrOfGlobal(TargetDecl));
  llvm::Constant *Aliasee = Ref;
  if (Ref->getType() != AliasType)
    Aliasee = llvm::ConstantExpr::getBitCast(Ref, AliasType);

  // Instead of creating as alias to a linkonce_odr, replace all of the uses
  // of the aliasee.
  if (llvm::GlobalValue::isDiscardableIfUnused(Linkage) &&
     (TargetLinkage != llvm::GlobalValue::AvailableExternallyLinkage ||
      !TargetDecl.getDecl()->hasAttr<AlwaysInlineAttr>())) {
    // FIXME: An extern template instantiation will create functions with
    // linkage "AvailableExternally". In libc++, some classes also define
    // members with attribute "AlwaysInline" and expect no reference to
    // be generated. It is desirable to reenable this optimisation after
    // corresponding LLVM changes.
    Replacements[MangledName] = Aliasee;
    return false;
  }

  if (!InEveryTU) {
    // If we don't have a definition for the destructor yet, don't
    // emit.  We can't emit aliases to declarations; that's just not
    // how aliases work.
    if (Ref->isDeclaration())
      return true;
  }

  // Don't create an alias to a linker weak symbol. This avoids producing
  // different COMDATs in different TUs. Another option would be to
  // output the alias both for weak_odr and linkonce_odr, but that
  // requires explicit comdat support in the IL.
  if (llvm::GlobalValue::isWeakForLinker(TargetLinkage))
    return true;

  // Create the alias with no name.
  auto *Alias =
      llvm::GlobalAlias::create(AliasType, Linkage, "", Aliasee, &getModule());

  // Switch any previous uses to the alias.
  if (Entry) {
    assert(Entry->getType() == AliasType &&
           "declaration exists with different type");
    Alias->takeName(Entry);
    Entry->replaceAllUsesWith(Alias);
    Entry->eraseFromParent();
  } else {
    Alias->setName(MangledName);
  }

  // Finally, set up the alias with its proper name and attributes.
  setAliasAttributes(cast<NamedDecl>(AliasDecl.getDecl()), Alias);

  return false;
}

llvm::Function *CodeGenModule::codegenCXXStructor(const CXXMethodDecl *MD,
                                                  StructorType Type) {
  const CGFunctionInfo &FnInfo =
      getTypes().arrangeCXXStructorDeclaration(MD, Type);
  auto *Fn = cast<llvm::Function>(
      getAddrOfCXXStructor(MD, Type, &FnInfo, nullptr, true));

  GlobalDecl GD;
  if (const auto *DD = dyn_cast<CXXDestructorDecl>(MD)) {
    GD = GlobalDecl(DD, toCXXDtorType(Type));
  } else {
    const auto *CD = cast<CXXConstructorDecl>(MD);
    GD = GlobalDecl(CD, toCXXCtorType(Type));
  }

  setFunctionLinkage(GD, Fn);
  setFunctionDLLStorageClass(GD, Fn);

  CodeGenFunction(*this).GenerateCode(GD, Fn, FnInfo);
  setFunctionDefinitionAttributes(MD, Fn);
  SetLLVMFunctionAttributesForDefinition(MD, Fn);
  return Fn;
}

llvm::GlobalValue *CodeGenModule::getAddrOfCXXStructor(
    const CXXMethodDecl *MD, StructorType Type, const CGFunctionInfo *FnInfo,
    llvm::FunctionType *FnType, bool DontDefer) {
  GlobalDecl GD;
  if (auto *CD = dyn_cast<CXXConstructorDecl>(MD)) {
    GD = GlobalDecl(CD, toCXXCtorType(Type));
  } else {
    GD = GlobalDecl(cast<CXXDestructorDecl>(MD), toCXXDtorType(Type));
  }

  StringRef Name = getMangledName(GD);
  if (llvm::GlobalValue *Existing = GetGlobalValue(Name))
    return Existing;

  if (!FnType) {
    if (!FnInfo)
      FnInfo = &getTypes().arrangeCXXStructorDeclaration(MD, Type);
    FnType = getTypes().GetFunctionType(*FnInfo);
  }

  return cast<llvm::Function>(GetOrCreateLLVMFunction(Name, FnType, GD,
                                                      /*ForVTable=*/false,
                                                      DontDefer));
}

static llvm::Value *BuildAppleKextVirtualCall(CodeGenFunction &CGF,
                                              GlobalDecl GD,
                                              llvm::Type *Ty,
                                              const CXXRecordDecl *RD) {
  assert(!CGF.CGM.getTarget().getCXXABI().isMicrosoft() &&
         "No kext in Microsoft ABI");
  GD = GD.getCanonicalDecl();
  CodeGenModule &CGM = CGF.CGM;
  llvm::Value *VTable = CGM.getCXXABI().getAddrOfVTable(RD, CharUnits());
  Ty = Ty->getPointerTo()->getPointerTo();
  VTable = CGF.Builder.CreateBitCast(VTable, Ty);
  assert(VTable && "BuildVirtualCall = kext vtbl pointer is null");
  uint64_t VTableIndex = CGM.getItaniumVTableContext().getMethodVTableIndex(GD);
  uint64_t AddressPoint =
    CGM.getItaniumVTableContext().getVTableLayout(RD)
       .getAddressPoint(BaseSubobject(RD, CharUnits::Zero()));
  VTableIndex += AddressPoint;
  llvm::Value *VFuncPtr =
    CGF.Builder.CreateConstInBoundsGEP1_64(VTable, VTableIndex, "vfnkxt");
  return CGF.Builder.CreateLoad(VFuncPtr);
}

/// BuildAppleKextVirtualCall - This routine is to support gcc's kext ABI making
/// indirect call to virtual functions. It makes the call through indexing
/// into the vtable.
llvm::Value *
CodeGenFunction::BuildAppleKextVirtualCall(const CXXMethodDecl *MD, 
                                  NestedNameSpecifier *Qual,
                                  llvm::Type *Ty) {
  assert((Qual->getKind() == NestedNameSpecifier::TypeSpec) &&
         "BuildAppleKextVirtualCall - bad Qual kind");
  
  const Type *QTy = Qual->getAsType();
  QualType T = QualType(QTy, 0);
  const RecordType *RT = T->getAs<RecordType>();
  assert(RT && "BuildAppleKextVirtualCall - Qual type must be record");
  const auto *RD = cast<CXXRecordDecl>(RT->getDecl());

  if (const auto *DD = dyn_cast<CXXDestructorDecl>(MD))
    return BuildAppleKextVirtualDestructorCall(DD, Dtor_Complete, RD);

  return ::BuildAppleKextVirtualCall(*this, MD, Ty, RD);
}

/// BuildVirtualCall - This routine makes indirect vtable call for
/// call to virtual destructors. It returns 0 if it could not do it.
llvm::Value *
CodeGenFunction::BuildAppleKextVirtualDestructorCall(
                                            const CXXDestructorDecl *DD,
                                            CXXDtorType Type,
                                            const CXXRecordDecl *RD) {
  const auto *MD = cast<CXXMethodDecl>(DD);
  // FIXME. Dtor_Base dtor is always direct!!
  // It need be somehow inline expanded into the caller.
  // -O does that. But need to support -O0 as well.
  if (MD->isVirtual() && Type != Dtor_Base) {
    // Compute the function type we're calling.
    const CGFunctionInfo &FInfo = CGM.getTypes().arrangeCXXStructorDeclaration(
        DD, StructorType::Complete);
    llvm::Type *Ty = CGM.getTypes().GetFunctionType(FInfo);
    return ::BuildAppleKextVirtualCall(*this, GlobalDecl(DD, Type), Ty, RD);
  }
  return nullptr;
}
