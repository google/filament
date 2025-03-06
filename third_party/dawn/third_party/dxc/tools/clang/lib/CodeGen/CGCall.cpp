//===--- CGCall.cpp - Encapsulate calling convention details --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// These classes wrap the information about a call or function
// definition used to handle ABI compliancy.
//
//===----------------------------------------------------------------------===//

#include "CGCall.h"
#include "ABIInfo.h"
#include "CGCXXABI.h"
#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "CGHLSLRuntime.h"    // HLSL Change
#include "TargetInfo.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Utils/Local.h"
using namespace clang;
using namespace CodeGen;

/***/

static unsigned ClangCallConvToLLVMCallConv(CallingConv CC) {
  switch (CC) {
  default: return llvm::CallingConv::C;
  case CC_X86StdCall: return llvm::CallingConv::X86_StdCall;
  case CC_X86FastCall: return llvm::CallingConv::X86_FastCall;
  case CC_X86ThisCall: return llvm::CallingConv::X86_ThisCall;
  case CC_X86_64Win64: return llvm::CallingConv::X86_64_Win64;
  case CC_X86_64SysV: return llvm::CallingConv::X86_64_SysV;
  case CC_AAPCS: return llvm::CallingConv::ARM_AAPCS;
  case CC_AAPCS_VFP: return llvm::CallingConv::ARM_AAPCS_VFP;
  case CC_IntelOclBicc: return llvm::CallingConv::Intel_OCL_BI;
  // TODO: Add support for __pascal to LLVM.
  case CC_X86Pascal: return llvm::CallingConv::C;
  // TODO: Add support for __vectorcall to LLVM.
  case CC_X86VectorCall: return llvm::CallingConv::X86_VectorCall;
  case CC_SpirFunction: return llvm::CallingConv::SPIR_FUNC;
  case CC_SpirKernel: return llvm::CallingConv::SPIR_KERNEL;
  }
}

/// Derives the 'this' type for codegen purposes, i.e. ignoring method
/// qualification.
/// FIXME: address space qualification?
static CanQualType GetThisType(ASTContext &Context, const CXXRecordDecl *RD) {
  QualType RecTy = Context.getTagDeclType(RD)->getCanonicalTypeInternal();
  return Context.getPointerType(CanQualType::CreateUnsafe(RecTy));
}

/// Returns the canonical formal type of the given C++ method.
static CanQual<FunctionProtoType> GetFormalType(const CXXMethodDecl *MD) {
  return MD->getType()->getCanonicalTypeUnqualified()
           .getAs<FunctionProtoType>();
}

/// Returns the "extra-canonicalized" return type, which discards
/// qualifiers on the return type.  Codegen doesn't care about them,
/// and it makes ABI code a little easier to be able to assume that
/// all parameter and return types are top-level unqualified.
static CanQualType GetReturnType(QualType RetTy) {
  return RetTy->getCanonicalTypeUnqualified().getUnqualifiedType();
}

/// Arrange the argument and result information for a value of the given
/// unprototyped freestanding function type.
const CGFunctionInfo &
CodeGenTypes::arrangeFreeFunctionType(CanQual<FunctionNoProtoType> FTNP) {
  // When translating an unprototyped function type, always use a
  // variadic type.
  return arrangeLLVMFunctionInfo(FTNP->getReturnType().getUnqualifiedType(),
                                 /*instanceMethod=*/false,
                                 /*chainCall=*/false, None,
                                 FTNP->getExtInfo(), RequiredArgs(0));
}

/// Arrange the LLVM function layout for a value of the given function
/// type, on top of any implicit parameters already stored.
static const CGFunctionInfo &
arrangeLLVMFunctionInfo(CodeGenTypes &CGT, bool instanceMethod,
                        SmallVectorImpl<CanQualType> &prefix,
                        CanQual<FunctionProtoType> FTP) {
  RequiredArgs required = RequiredArgs::forPrototypePlus(FTP, prefix.size());
  // FIXME: Kill copy.
  prefix.append(FTP->param_type_begin(), FTP->param_type_end());
  CanQualType resultType = FTP->getReturnType().getUnqualifiedType();
  return CGT.arrangeLLVMFunctionInfo(resultType, instanceMethod,
                                     /*chainCall=*/false, prefix,
                                     FTP->getExtInfo(), required);
}

/// Arrange the argument and result information for a value of the
/// given freestanding function type.
const CGFunctionInfo &
CodeGenTypes::arrangeFreeFunctionType(CanQual<FunctionProtoType> FTP) {
  SmallVector<CanQualType, 16> argTypes;
  return ::arrangeLLVMFunctionInfo(*this, /*instanceMethod=*/false, argTypes,
                                   FTP);
}

static CallingConv getCallingConventionForDecl(const Decl *D, bool IsWindows) {
  // Set the appropriate calling convention for the Function.
  if (D->hasAttr<StdCallAttr>())
    return CC_X86StdCall;

  if (D->hasAttr<FastCallAttr>())
    return CC_X86FastCall;

  if (D->hasAttr<ThisCallAttr>())
    return CC_X86ThisCall;

  if (D->hasAttr<VectorCallAttr>())
    return CC_X86VectorCall;

  if (D->hasAttr<PascalAttr>())
    return CC_X86Pascal;

  if (PcsAttr *PCS = D->getAttr<PcsAttr>())
    return (PCS->getPCS() == PcsAttr::AAPCS ? CC_AAPCS : CC_AAPCS_VFP);

  if (D->hasAttr<IntelOclBiccAttr>())
    return CC_IntelOclBicc;

  if (D->hasAttr<MSABIAttr>())
    return IsWindows ? CC_C : CC_X86_64Win64;

  if (D->hasAttr<SysVABIAttr>())
    return IsWindows ? CC_X86_64SysV : CC_C;

  return CC_C;
}

/// Arrange the argument and result information for a call to an
/// unknown C++ non-static member function of the given abstract type.
/// (Zero value of RD means we don't have any meaningful "this" argument type,
///  so fall back to a generic pointer type).
/// The member function must be an ordinary function, i.e. not a
/// constructor or destructor.
const CGFunctionInfo &
CodeGenTypes::arrangeCXXMethodType(const CXXRecordDecl *RD,
                                   const FunctionProtoType *FTP) {
  SmallVector<CanQualType, 16> argTypes;

  // Add the 'this' pointer.
  if (RD)
    argTypes.push_back(GetThisType(Context, RD));
  else
    argTypes.push_back(Context.VoidPtrTy);

  return ::arrangeLLVMFunctionInfo(
      *this, true, argTypes,
      FTP->getCanonicalTypeUnqualified().getAs<FunctionProtoType>());
}

/// Arrange the argument and result information for a declaration or
/// definition of the given C++ non-static member function.  The
/// member function must be an ordinary function, i.e. not a
/// constructor or destructor.
const CGFunctionInfo &
CodeGenTypes::arrangeCXXMethodDeclaration(const CXXMethodDecl *MD) {
  assert(!isa<CXXConstructorDecl>(MD) && "wrong method for constructors!");
  assert(!isa<CXXDestructorDecl>(MD) && "wrong method for destructors!");

  CanQual<FunctionProtoType> prototype = GetFormalType(MD);

  if (MD->isInstance()) {
    // The abstract case is perfectly fine.
    const CXXRecordDecl *ThisType = TheCXXABI.getThisArgumentTypeForMethod(MD);
    return arrangeCXXMethodType(ThisType, prototype.getTypePtr());
  }

  return arrangeFreeFunctionType(prototype);
}

const CGFunctionInfo &
CodeGenTypes::arrangeCXXStructorDeclaration(const CXXMethodDecl *MD,
                                            StructorType Type) {

  SmallVector<CanQualType, 16> argTypes;
  argTypes.push_back(GetThisType(Context, MD->getParent()));

  GlobalDecl GD;
  if (auto *CD = dyn_cast<CXXConstructorDecl>(MD)) {
    GD = GlobalDecl(CD, toCXXCtorType(Type));
  } else {
    auto *DD = dyn_cast<CXXDestructorDecl>(MD);
    GD = GlobalDecl(DD, toCXXDtorType(Type));
  }

  CanQual<FunctionProtoType> FTP = GetFormalType(MD);

  // Add the formal parameters.
  argTypes.append(FTP->param_type_begin(), FTP->param_type_end());

  TheCXXABI.buildStructorSignature(MD, Type, argTypes);

  RequiredArgs required =
      (MD->isVariadic() ? RequiredArgs(argTypes.size()) : RequiredArgs::All);

  FunctionType::ExtInfo extInfo = FTP->getExtInfo();
  CanQualType resultType = TheCXXABI.HasThisReturn(GD)
                               ? argTypes.front()
                               : TheCXXABI.hasMostDerivedReturn(GD)
                                     ? CGM.getContext().VoidPtrTy
                                     : Context.VoidTy;
  return arrangeLLVMFunctionInfo(resultType, /*instanceMethod=*/true,
                                 /*chainCall=*/false, argTypes, extInfo,
                                 required);
}

/// Arrange a call to a C++ method, passing the given arguments.
const CGFunctionInfo &
CodeGenTypes::arrangeCXXConstructorCall(const CallArgList &args,
                                        const CXXConstructorDecl *D,
                                        CXXCtorType CtorKind,
                                        unsigned ExtraArgs) {
  // FIXME: Kill copy.
  SmallVector<CanQualType, 16> ArgTypes;
  for (const auto &Arg : args)
    ArgTypes.push_back(Context.getCanonicalParamType(Arg.Ty));

  CanQual<FunctionProtoType> FPT = GetFormalType(D);
  RequiredArgs Required = RequiredArgs::forPrototypePlus(FPT, 1 + ExtraArgs);
  GlobalDecl GD(D, CtorKind);
  CanQualType ResultType = TheCXXABI.HasThisReturn(GD)
                               ? ArgTypes.front()
                               : TheCXXABI.hasMostDerivedReturn(GD)
                                     ? CGM.getContext().VoidPtrTy
                                     : Context.VoidTy;

  FunctionType::ExtInfo Info = FPT->getExtInfo();
  return arrangeLLVMFunctionInfo(ResultType, /*instanceMethod=*/true,
                                 /*chainCall=*/false, ArgTypes, Info,
                                 Required);
}

/// Arrange the argument and result information for the declaration or
/// definition of the given function.
const CGFunctionInfo &
CodeGenTypes::arrangeFunctionDeclaration(const FunctionDecl *FD) {
  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD))
    if (MD->isInstance())
      return arrangeCXXMethodDeclaration(MD);

  CanQualType FTy = FD->getType()->getCanonicalTypeUnqualified();

  assert(isa<FunctionType>(FTy));

  // When declaring a function without a prototype, always use a
  // non-variadic type.
  if (isa<FunctionNoProtoType>(FTy)) {
    CanQual<FunctionNoProtoType> noProto = FTy.getAs<FunctionNoProtoType>();
    return arrangeLLVMFunctionInfo(
        noProto->getReturnType(), /*instanceMethod=*/false,
        /*chainCall=*/false, None, noProto->getExtInfo(), RequiredArgs::All);
  }

  assert(isa<FunctionProtoType>(FTy));
  return arrangeFreeFunctionType(FTy.getAs<FunctionProtoType>());
}

/// Arrange the argument and result information for the declaration or
/// definition of an Objective-C method.
const CGFunctionInfo &
CodeGenTypes::arrangeObjCMethodDeclaration(const ObjCMethodDecl *MD) {
  // It happens that this is the same as a call with no optional
  // arguments, except also using the formal 'self' type.
  return arrangeObjCMessageSendSignature(MD, MD->getSelfDecl()->getType());
}

/// Arrange the argument and result information for the function type
/// through which to perform a send to the given Objective-C method,
/// using the given receiver type.  The receiver type is not always
/// the 'self' type of the method or even an Objective-C pointer type.
/// This is *not* the right method for actually performing such a
/// message send, due to the possibility of optional arguments.
const CGFunctionInfo &
CodeGenTypes::arrangeObjCMessageSendSignature(const ObjCMethodDecl *MD,
                                              QualType receiverType) {
  SmallVector<CanQualType, 16> argTys;
  argTys.push_back(Context.getCanonicalParamType(receiverType));
  argTys.push_back(Context.getCanonicalParamType(Context.getObjCSelType()));
  // FIXME: Kill copy?
  for (const auto *I : MD->params()) {
    argTys.push_back(Context.getCanonicalParamType(I->getType()));
  }

  FunctionType::ExtInfo einfo;
  bool IsWindows = getContext().getTargetInfo().getTriple().isOSWindows();
  einfo = einfo.withCallingConv(getCallingConventionForDecl(MD, IsWindows));

  if (getContext().getLangOpts().ObjCAutoRefCount &&
      MD->hasAttr<NSReturnsRetainedAttr>())
    einfo = einfo.withProducesResult(true);

  RequiredArgs required =
    (MD->isVariadic() ? RequiredArgs(argTys.size()) : RequiredArgs::All);

  return arrangeLLVMFunctionInfo(
      GetReturnType(MD->getReturnType()), /*instanceMethod=*/false,
      /*chainCall=*/false, argTys, einfo, required);
}

const CGFunctionInfo &
CodeGenTypes::arrangeGlobalDeclaration(GlobalDecl GD) {
  // FIXME: Do we need to handle ObjCMethodDecl?
  const FunctionDecl *FD = cast<FunctionDecl>(GD.getDecl());

  if (const CXXConstructorDecl *CD = dyn_cast<CXXConstructorDecl>(FD))
    return arrangeCXXStructorDeclaration(CD, getFromCtorType(GD.getCtorType()));

  if (const CXXDestructorDecl *DD = dyn_cast<CXXDestructorDecl>(FD))
    return arrangeCXXStructorDeclaration(DD, getFromDtorType(GD.getDtorType()));

  return arrangeFunctionDeclaration(FD);
}

/// Arrange a thunk that takes 'this' as the first parameter followed by
/// varargs.  Return a void pointer, regardless of the actual return type.
/// The body of the thunk will end in a musttail call to a function of the
/// correct type, and the caller will bitcast the function to the correct
/// prototype.
const CGFunctionInfo &
CodeGenTypes::arrangeMSMemberPointerThunk(const CXXMethodDecl *MD) {
  assert(MD->isVirtual() && "only virtual memptrs have thunks");
  CanQual<FunctionProtoType> FTP = GetFormalType(MD);
  CanQualType ArgTys[] = { GetThisType(Context, MD->getParent()) };
  return arrangeLLVMFunctionInfo(Context.VoidTy, /*instanceMethod=*/false,
                                 /*chainCall=*/false, ArgTys,
                                 FTP->getExtInfo(), RequiredArgs(1));
}

const CGFunctionInfo &
CodeGenTypes::arrangeMSCtorClosure(const CXXConstructorDecl *CD,
                                   CXXCtorType CT) {
  assert(CT == Ctor_CopyingClosure || CT == Ctor_DefaultClosure);

  CanQual<FunctionProtoType> FTP = GetFormalType(CD);
  SmallVector<CanQualType, 2> ArgTys;
  const CXXRecordDecl *RD = CD->getParent();
  ArgTys.push_back(GetThisType(Context, RD));
  if (CT == Ctor_CopyingClosure)
    ArgTys.push_back(*FTP->param_type_begin());
  if (RD->getNumVBases() > 0)
    ArgTys.push_back(Context.IntTy);
  CallingConv CC = Context.getDefaultCallingConvention(
      /*IsVariadic=*/false, /*IsCXXMethod=*/true);
  return arrangeLLVMFunctionInfo(Context.VoidTy, /*instanceMethod=*/true,
                                 /*chainCall=*/false, ArgTys,
                                 FunctionType::ExtInfo(CC), RequiredArgs::All);
}

/// Arrange a call as unto a free function, except possibly with an
/// additional number of formal parameters considered required.
static const CGFunctionInfo &
arrangeFreeFunctionLikeCall(CodeGenTypes &CGT,
                            CodeGenModule &CGM,
                            const CallArgList &args,
                            const FunctionType *fnType,
                            unsigned numExtraRequiredArgs,
                            bool chainCall) {
  assert(args.size() >= numExtraRequiredArgs);

  // In most cases, there are no optional arguments.
  RequiredArgs required = RequiredArgs::All;

  // If we have a variadic prototype, the required arguments are the
  // extra prefix plus the arguments in the prototype.
  if (const FunctionProtoType *proto = dyn_cast<FunctionProtoType>(fnType)) {
    if (proto->isVariadic())
      required = RequiredArgs(proto->getNumParams() + numExtraRequiredArgs);

  // If we don't have a prototype at all, but we're supposed to
  // explicitly use the variadic convention for unprototyped calls,
  // treat all of the arguments as required but preserve the nominal
  // possibility of variadics.
  } else if (CGM.getTargetCodeGenInfo()
                .isNoProtoCallVariadic(args,
                                       cast<FunctionNoProtoType>(fnType))) {
    required = RequiredArgs(args.size());
  }

  // FIXME: Kill copy.
  SmallVector<CanQualType, 16> argTypes;
  for (const auto &arg : args)
    argTypes.push_back(CGT.getContext().getCanonicalParamType(arg.Ty));
  return CGT.arrangeLLVMFunctionInfo(GetReturnType(fnType->getReturnType()),
                                     /*instanceMethod=*/false, chainCall,
                                     argTypes, fnType->getExtInfo(), required);
}

/// Figure out the rules for calling a function with the given formal
/// type using the given arguments.  The arguments are necessary
/// because the function might be unprototyped, in which case it's
/// target-dependent in crazy ways.
const CGFunctionInfo &
CodeGenTypes::arrangeFreeFunctionCall(const CallArgList &args,
                                      const FunctionType *fnType,
                                      bool chainCall) {
  return arrangeFreeFunctionLikeCall(*this, CGM, args, fnType,
                                     chainCall ? 1 : 0, chainCall);
}

/// A block function call is essentially a free-function call with an
/// extra implicit argument.
const CGFunctionInfo &
CodeGenTypes::arrangeBlockFunctionCall(const CallArgList &args,
                                       const FunctionType *fnType) {
  return arrangeFreeFunctionLikeCall(*this, CGM, args, fnType, 1,
                                     /*chainCall=*/false);
}

const CGFunctionInfo &
CodeGenTypes::arrangeFreeFunctionCall(QualType resultType,
                                      const CallArgList &args,
                                      FunctionType::ExtInfo info,
                                      RequiredArgs required) {
  // FIXME: Kill copy.
  SmallVector<CanQualType, 16> argTypes;
  for (const auto &Arg : args)
    argTypes.push_back(Context.getCanonicalParamType(Arg.Ty));
  return arrangeLLVMFunctionInfo(
      GetReturnType(resultType), /*instanceMethod=*/false,
      /*chainCall=*/false, argTypes, info, required);
}

/// Arrange a call to a C++ method, passing the given arguments.
const CGFunctionInfo &
CodeGenTypes::arrangeCXXMethodCall(const CallArgList &args,
                                   const FunctionProtoType *FPT,
                                   RequiredArgs required) {
  // FIXME: Kill copy.
  SmallVector<CanQualType, 16> argTypes;
  for (const auto &Arg : args)
    argTypes.push_back(Context.getCanonicalParamType(Arg.Ty));

  FunctionType::ExtInfo info = FPT->getExtInfo();
  return arrangeLLVMFunctionInfo(
      GetReturnType(FPT->getReturnType()), /*instanceMethod=*/true,
      /*chainCall=*/false, argTypes, info, required);
}

const CGFunctionInfo &CodeGenTypes::arrangeFreeFunctionDeclaration(
    QualType resultType, const FunctionArgList &args,
    const FunctionType::ExtInfo &info, bool isVariadic) {
  // FIXME: Kill copy.
  SmallVector<CanQualType, 16> argTypes;
  for (auto Arg : args)
    argTypes.push_back(Context.getCanonicalParamType(Arg->getType()));

  RequiredArgs required =
    (isVariadic ? RequiredArgs(args.size()) : RequiredArgs::All);
  return arrangeLLVMFunctionInfo(
      GetReturnType(resultType), /*instanceMethod=*/false,
      /*chainCall=*/false, argTypes, info, required);
}

const CGFunctionInfo &CodeGenTypes::arrangeNullaryFunction() {
  return arrangeLLVMFunctionInfo(
      getContext().VoidTy, /*instanceMethod=*/false, /*chainCall=*/false,
      None, FunctionType::ExtInfo(), RequiredArgs::All);
}

/// Arrange the argument and result information for an abstract value
/// of a given function type.  This is the method which all of the
/// above functions ultimately defer to.
const CGFunctionInfo &
CodeGenTypes::arrangeLLVMFunctionInfo(CanQualType resultType,
                                      bool instanceMethod,
                                      bool chainCall,
                                      ArrayRef<CanQualType> argTypes,
                                      FunctionType::ExtInfo info,
                                      RequiredArgs required) {
  // HLSL Change Starts
  ASTContext &context = getContext();
  auto isCanonicalAsParam = [&context](const CanQualType &Ty) {
    return Ty.isCanonicalAsParam() ||
           (context.getLangOpts().HLSL && Ty->isArrayType());
  };
  // HLSL Change Ends
  assert(std::all_of(argTypes.begin(), argTypes.end(),
                     isCanonicalAsParam)); // HLSL Change - skip array when
                                           // check isCanonicalAsParam
  (void)isCanonicalAsParam;

  unsigned CC = ClangCallConvToLLVMCallConv(info.getCC());

  // Lookup or create unique function info.
  llvm::FoldingSetNodeID ID;
  CGFunctionInfo::Profile(ID, instanceMethod, chainCall, info, required,
                          resultType, argTypes);

  void *insertPos = nullptr;
  CGFunctionInfo *FI = FunctionInfos.FindNodeOrInsertPos(ID, insertPos);
  if (FI)
    return *FI;

  // Construct the function info.  We co-allocate the ArgInfos.
  FI = CGFunctionInfo::create(CC, instanceMethod, chainCall, info,
                              resultType, argTypes, required);
  FunctionInfos.InsertNode(FI, insertPos);

  bool inserted = FunctionsBeingProcessed.insert(FI).second;
  (void)inserted;
  assert(inserted && "Recursively being processed?");
  
  // Compute ABI information.
  getABIInfo().computeInfo(*FI);

  // Loop over all of the computed argument and return value info.  If any of
  // them are direct or extend without a specified coerce type, specify the
  // default now.
  ABIArgInfo &retInfo = FI->getReturnInfo();
  if (retInfo.canHaveCoerceToType() && retInfo.getCoerceToType() == nullptr)
    retInfo.setCoerceToType(ConvertType(FI->getReturnType()));

  for (auto &I : FI->arguments())
    if (I.info.canHaveCoerceToType() && I.info.getCoerceToType() == nullptr)
      I.info.setCoerceToType(ConvertType(I.type));

  bool erased = FunctionsBeingProcessed.erase(FI); (void)erased;
  assert(erased && "Not in set?");
  
  return *FI;
}

CGFunctionInfo *CGFunctionInfo::create(unsigned llvmCC,
                                       bool instanceMethod,
                                       bool chainCall,
                                       const FunctionType::ExtInfo &info,
                                       CanQualType resultType,
                                       ArrayRef<CanQualType> argTypes,
                                       RequiredArgs required) {
  void *buffer = operator new(sizeof(CGFunctionInfo) +
                              sizeof(ArgInfo) * (argTypes.size() + 1));
  CGFunctionInfo *FI = new(buffer) CGFunctionInfo();
  FI->CallingConvention = llvmCC;
  FI->EffectiveCallingConvention = llvmCC;
  FI->ASTCallingConvention = info.getCC();
  FI->InstanceMethod = instanceMethod;
  FI->ChainCall = chainCall;
  FI->NoReturn = info.getNoReturn();
  FI->ReturnsRetained = info.getProducesResult();
  FI->Required = required;
  FI->HasRegParm = info.getHasRegParm();
  FI->RegParm = info.getRegParm();
  FI->ArgStruct = nullptr;
  FI->NumArgs = argTypes.size();
  FI->getArgsBuffer()[0].type = resultType;
  for (unsigned i = 0, e = argTypes.size(); i != e; ++i)
    FI->getArgsBuffer()[i + 1].type = argTypes[i];
  return FI;
}

/***/

namespace {
// ABIArgInfo::Expand implementation.

// Specifies the way QualType passed as ABIArgInfo::Expand is expanded.
struct TypeExpansion {
  enum TypeExpansionKind {
    // Elements of constant arrays are expanded recursively.
    TEK_ConstantArray,
    // Record fields are expanded recursively (but if record is a union, only
    // the field with the largest size is expanded).
    TEK_Record,
    // For complex types, real and imaginary parts are expanded recursively.
    TEK_Complex,
    // All other types are not expandable.
    TEK_None
  };

  const TypeExpansionKind Kind;

  TypeExpansion(TypeExpansionKind K) : Kind(K) {}
  virtual ~TypeExpansion() {}
};

struct ConstantArrayExpansion : TypeExpansion {
  QualType EltTy;
  uint64_t NumElts;

  ConstantArrayExpansion(QualType EltTy, uint64_t NumElts)
      : TypeExpansion(TEK_ConstantArray), EltTy(EltTy), NumElts(NumElts) {}
  static bool classof(const TypeExpansion *TE) {
    return TE->Kind == TEK_ConstantArray;
  }
};

struct RecordExpansion : TypeExpansion {
  SmallVector<const CXXBaseSpecifier *, 1> Bases;

  SmallVector<const FieldDecl *, 1> Fields;

  RecordExpansion(SmallVector<const CXXBaseSpecifier *, 1> &&Bases,
                  SmallVector<const FieldDecl *, 1> &&Fields)
      : TypeExpansion(TEK_Record), Bases(Bases), Fields(Fields) {}
  static bool classof(const TypeExpansion *TE) {
    return TE->Kind == TEK_Record;
  }
};

struct ComplexExpansion : TypeExpansion {
  QualType EltTy;

  ComplexExpansion(QualType EltTy) : TypeExpansion(TEK_Complex), EltTy(EltTy) {}
  static bool classof(const TypeExpansion *TE) {
    return TE->Kind == TEK_Complex;
  }
};

struct NoExpansion : TypeExpansion {
  NoExpansion() : TypeExpansion(TEK_None) {}
  static bool classof(const TypeExpansion *TE) {
    return TE->Kind == TEK_None;
  }
};
}  // namespace

static std::unique_ptr<TypeExpansion>
getTypeExpansion(QualType Ty, const ASTContext &Context) {
  if (const ConstantArrayType *AT = Context.getAsConstantArrayType(Ty)) {
    return llvm::make_unique<ConstantArrayExpansion>(
        AT->getElementType(), AT->getSize().getZExtValue());
  }
  if (const RecordType *RT = Ty->getAs<RecordType>()) {
    SmallVector<const CXXBaseSpecifier *, 1> Bases;
    SmallVector<const FieldDecl *, 1> Fields;
    const RecordDecl *RD = RT->getDecl();
    assert(!RD->hasFlexibleArrayMember() &&
           "Cannot expand structure with flexible array.");
    if (RD->isUnion()) {
      // Unions can be here only in degenerative cases - all the fields are same
      // after flattening. Thus we have to use the "largest" field.
      const FieldDecl *LargestFD = nullptr;
      CharUnits UnionSize = CharUnits::Zero();

      for (const auto *FD : RD->fields()) {
        // Skip zero length bitfields.
        if (FD->isBitField() && FD->getBitWidthValue(Context) == 0)
          continue;
        assert(!FD->isBitField() &&
               "Cannot expand structure with bit-field members.");
        CharUnits FieldSize = Context.getTypeSizeInChars(FD->getType());
        if (UnionSize < FieldSize) {
          UnionSize = FieldSize;
          LargestFD = FD;
        }
      }
      if (LargestFD)
        Fields.push_back(LargestFD);
    } else {
      if (const auto *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
        assert(!CXXRD->isDynamicClass() &&
               "cannot expand vtable pointers in dynamic classes");
        for (const CXXBaseSpecifier &BS : CXXRD->bases())
          Bases.push_back(&BS);
      }

      for (const auto *FD : RD->fields()) {
        // Skip zero length bitfields.
        if (FD->isBitField() && FD->getBitWidthValue(Context) == 0)
          continue;
        assert(!FD->isBitField() &&
               "Cannot expand structure with bit-field members.");
        Fields.push_back(FD);
      }
    }
    return llvm::make_unique<RecordExpansion>(std::move(Bases),
                                              std::move(Fields));
  }
  if (const ComplexType *CT = Ty->getAs<ComplexType>()) {
    return llvm::make_unique<ComplexExpansion>(CT->getElementType());
  }
  return llvm::make_unique<NoExpansion>();
}

static int getExpansionSize(QualType Ty, const ASTContext &Context) {
  auto Exp = getTypeExpansion(Ty, Context);
  if (auto CAExp = dyn_cast<ConstantArrayExpansion>(Exp.get())) {
    return CAExp->NumElts * getExpansionSize(CAExp->EltTy, Context);
  }
  if (auto RExp = dyn_cast<RecordExpansion>(Exp.get())) {
    int Res = 0;
    for (auto BS : RExp->Bases)
      Res += getExpansionSize(BS->getType(), Context);
    for (auto FD : RExp->Fields)
      Res += getExpansionSize(FD->getType(), Context);
    return Res;
  }
  if (isa<ComplexExpansion>(Exp.get()))
    return 2;
  assert(isa<NoExpansion>(Exp.get()));
  return 1;
}

void
CodeGenTypes::getExpandedTypes(QualType Ty,
                               SmallVectorImpl<llvm::Type *>::iterator &TI) {
  auto Exp = getTypeExpansion(Ty, Context);
  if (auto CAExp = dyn_cast<ConstantArrayExpansion>(Exp.get())) {
    for (int i = 0, n = CAExp->NumElts; i < n; i++) {
      getExpandedTypes(CAExp->EltTy, TI);
    }
  } else if (auto RExp = dyn_cast<RecordExpansion>(Exp.get())) {
    for (auto BS : RExp->Bases)
      getExpandedTypes(BS->getType(), TI);
    for (auto FD : RExp->Fields)
      getExpandedTypes(FD->getType(), TI);
  } else if (auto CExp = dyn_cast<ComplexExpansion>(Exp.get())) {
    llvm::Type *EltTy = ConvertType(CExp->EltTy);
    *TI++ = EltTy;
    *TI++ = EltTy;
  } else {
    assert(isa<NoExpansion>(Exp.get()));
    *TI++ = ConvertType(Ty);
  }
}

void CodeGenFunction::ExpandTypeFromArgs(
    QualType Ty, LValue LV, SmallVectorImpl<llvm::Argument *>::iterator &AI) {
  assert(LV.isSimple() &&
         "Unexpected non-simple lvalue during struct expansion.");

  auto Exp = getTypeExpansion(Ty, getContext());
  if (auto CAExp = dyn_cast<ConstantArrayExpansion>(Exp.get())) {
    for (int i = 0, n = CAExp->NumElts; i < n; i++) {
      llvm::Value *EltAddr =
          Builder.CreateConstGEP2_32(nullptr, LV.getAddress(), 0, i);
      LValue LV = MakeAddrLValue(EltAddr, CAExp->EltTy);
      ExpandTypeFromArgs(CAExp->EltTy, LV, AI);
    }
  } else if (auto RExp = dyn_cast<RecordExpansion>(Exp.get())) {
    llvm::Value *This = LV.getAddress();
    for (const CXXBaseSpecifier *BS : RExp->Bases) {
      // Perform a single step derived-to-base conversion.
      llvm::Value *Base =
          GetAddressOfBaseClass(This, Ty->getAsCXXRecordDecl(), &BS, &BS + 1,
                                /*NullCheckValue=*/false, SourceLocation());
      LValue SubLV = MakeAddrLValue(Base, BS->getType());

      // Recurse onto bases.
      ExpandTypeFromArgs(BS->getType(), SubLV, AI);
    }
    for (auto FD : RExp->Fields) {
      // FIXME: What are the right qualifiers here?
      LValue SubLV = EmitLValueForField(LV, FD);
      ExpandTypeFromArgs(FD->getType(), SubLV, AI);
    }
  } else if (auto CExp = dyn_cast<ComplexExpansion>(Exp.get())) {
    llvm::Value *RealAddr =
        Builder.CreateStructGEP(nullptr, LV.getAddress(), 0, "real");
    EmitStoreThroughLValue(RValue::get(*AI++),
                           MakeAddrLValue(RealAddr, CExp->EltTy));
    llvm::Value *ImagAddr =
        Builder.CreateStructGEP(nullptr, LV.getAddress(), 1, "imag");
    EmitStoreThroughLValue(RValue::get(*AI++),
                           MakeAddrLValue(ImagAddr, CExp->EltTy));
  } else {
    assert(isa<NoExpansion>(Exp.get()));
    EmitStoreThroughLValue(RValue::get(*AI++), LV);
  }
}

void CodeGenFunction::ExpandTypeToArgs(
    QualType Ty, RValue RV, llvm::FunctionType *IRFuncTy,
    SmallVectorImpl<llvm::Value *> &IRCallArgs, unsigned &IRCallArgPos) {
  auto Exp = getTypeExpansion(Ty, getContext());
  if (auto CAExp = dyn_cast<ConstantArrayExpansion>(Exp.get())) {
    llvm::Value *Addr = RV.getAggregateAddr();
    for (int i = 0, n = CAExp->NumElts; i < n; i++) {
      llvm::Value *EltAddr = Builder.CreateConstGEP2_32(nullptr, Addr, 0, i);
      RValue EltRV =
          convertTempToRValue(EltAddr, CAExp->EltTy, SourceLocation());
      ExpandTypeToArgs(CAExp->EltTy, EltRV, IRFuncTy, IRCallArgs, IRCallArgPos);
    }
  } else if (auto RExp = dyn_cast<RecordExpansion>(Exp.get())) {
    llvm::Value *This = RV.getAggregateAddr();
    for (const CXXBaseSpecifier *BS : RExp->Bases) {
      // Perform a single step derived-to-base conversion.
      llvm::Value *Base =
          GetAddressOfBaseClass(This, Ty->getAsCXXRecordDecl(), &BS, &BS + 1,
                                /*NullCheckValue=*/false, SourceLocation());
      RValue BaseRV = RValue::getAggregate(Base);

      // Recurse onto bases.
      ExpandTypeToArgs(BS->getType(), BaseRV, IRFuncTy, IRCallArgs,
                       IRCallArgPos);
    }

    LValue LV = MakeAddrLValue(This, Ty);
    for (auto FD : RExp->Fields) {
      RValue FldRV = EmitRValueForField(LV, FD, SourceLocation());
      ExpandTypeToArgs(FD->getType(), FldRV, IRFuncTy, IRCallArgs,
                       IRCallArgPos);
    }
  } else if (isa<ComplexExpansion>(Exp.get())) {
    ComplexPairTy CV = RV.getComplexVal();
    IRCallArgs[IRCallArgPos++] = CV.first;
    IRCallArgs[IRCallArgPos++] = CV.second;
  } else {
    assert(isa<NoExpansion>(Exp.get()));
    assert(RV.isScalar() &&
           "Unexpected non-scalar rvalue during struct expansion.");

    // Insert a bitcast as needed.
    llvm::Value *V = RV.getScalarVal();
    if (IRCallArgPos < IRFuncTy->getNumParams() &&
        V->getType() != IRFuncTy->getParamType(IRCallArgPos))
      V = Builder.CreateBitCast(V, IRFuncTy->getParamType(IRCallArgPos));

    IRCallArgs[IRCallArgPos++] = V;
  }
}

/// EnterStructPointerForCoercedAccess - Given a struct pointer that we are
/// accessing some number of bytes out of it, try to gep into the struct to get
/// at its inner goodness.  Dive as deep as possible without entering an element
/// with an in-memory size smaller than DstSize.
static llvm::Value *
EnterStructPointerForCoercedAccess(llvm::Value *SrcPtr,
                                   llvm::StructType *SrcSTy,
                                   uint64_t DstSize, CodeGenFunction &CGF) {
  // We can't dive into a zero-element struct.
  if (SrcSTy->getNumElements() == 0) return SrcPtr;

  llvm::Type *FirstElt = SrcSTy->getElementType(0);

  // If the first elt is at least as large as what we're looking for, or if the
  // first element is the same size as the whole struct, we can enter it. The
  // comparison must be made on the store size and not the alloca size. Using
  // the alloca size may overstate the size of the load.
  uint64_t FirstEltSize =
    CGF.CGM.getDataLayout().getTypeStoreSize(FirstElt);
  if (FirstEltSize < DstSize &&
      FirstEltSize < CGF.CGM.getDataLayout().getTypeStoreSize(SrcSTy))
    return SrcPtr;

  // GEP into the first element.
  SrcPtr = CGF.Builder.CreateConstGEP2_32(SrcSTy, SrcPtr, 0, 0, "coerce.dive");

  // If the first element is a struct, recurse.
  llvm::Type *SrcTy =
    cast<llvm::PointerType>(SrcPtr->getType())->getElementType();
  if (llvm::StructType *SrcSTy = dyn_cast<llvm::StructType>(SrcTy))
    return EnterStructPointerForCoercedAccess(SrcPtr, SrcSTy, DstSize, CGF);

  return SrcPtr;
}

/// CoerceIntOrPtrToIntOrPtr - Convert a value Val to the specific Ty where both
/// are either integers or pointers.  This does a truncation of the value if it
/// is too large or a zero extension if it is too small.
///
/// This behaves as if the value were coerced through memory, so on big-endian
/// targets the high bits are preserved in a truncation, while little-endian
/// targets preserve the low bits.
static llvm::Value *CoerceIntOrPtrToIntOrPtr(llvm::Value *Val,
                                             llvm::Type *Ty,
                                             CodeGenFunction &CGF) {
  if (Val->getType() == Ty)
    return Val;

  if (isa<llvm::PointerType>(Val->getType())) {
    // If this is Pointer->Pointer avoid conversion to and from int.
    if (isa<llvm::PointerType>(Ty))
      return CGF.Builder.CreateBitCast(Val, Ty, "coerce.val");

    // Convert the pointer to an integer so we can play with its width.
    Val = CGF.Builder.CreatePtrToInt(Val, CGF.IntPtrTy, "coerce.val.pi");
  }

  llvm::Type *DestIntTy = Ty;
  if (isa<llvm::PointerType>(DestIntTy))
    DestIntTy = CGF.IntPtrTy;

  if (Val->getType() != DestIntTy) {
    const llvm::DataLayout &DL = CGF.CGM.getDataLayout();
    if (DL.isBigEndian()) {
      // Preserve the high bits on big-endian targets.
      // That is what memory coercion does.
      uint64_t SrcSize = DL.getTypeSizeInBits(Val->getType());
      uint64_t DstSize = DL.getTypeSizeInBits(DestIntTy);

      if (SrcSize > DstSize) {
        Val = CGF.Builder.CreateLShr(Val, SrcSize - DstSize, "coerce.highbits");
        Val = CGF.Builder.CreateTrunc(Val, DestIntTy, "coerce.val.ii");
      } else {
        Val = CGF.Builder.CreateZExt(Val, DestIntTy, "coerce.val.ii");
        Val = CGF.Builder.CreateShl(Val, DstSize - SrcSize, "coerce.highbits");
      }
    } else {
      // Little-endian targets preserve the low bits. No shifts required.
      Val = CGF.Builder.CreateIntCast(Val, DestIntTy, false, "coerce.val.ii");
    }
  }

  if (isa<llvm::PointerType>(Ty))
    Val = CGF.Builder.CreateIntToPtr(Val, Ty, "coerce.val.ip");
  return Val;
}



/// CreateCoercedLoad - Create a load from \arg SrcPtr interpreted as
/// a pointer to an object of type \arg Ty, known to be aligned to
/// \arg SrcAlign bytes.
///
/// This safely handles the case when the src type is smaller than the
/// destination type; in this situation the values of bits which not
/// present in the src are undefined.
static llvm::Value *CreateCoercedLoad(llvm::Value *SrcPtr,
                                      llvm::Type *Ty, CharUnits SrcAlign,
                                      CodeGenFunction &CGF) {
  llvm::Type *SrcTy =
    cast<llvm::PointerType>(SrcPtr->getType())->getElementType();

  // If SrcTy and Ty are the same, just do a load.
  if (SrcTy == Ty)
    return CGF.Builder.CreateAlignedLoad(SrcPtr, SrcAlign.getQuantity());

  uint64_t DstSize = CGF.CGM.getDataLayout().getTypeAllocSize(Ty);

  if (llvm::StructType *SrcSTy = dyn_cast<llvm::StructType>(SrcTy)) {
    SrcPtr = EnterStructPointerForCoercedAccess(SrcPtr, SrcSTy, DstSize, CGF);
    SrcTy = cast<llvm::PointerType>(SrcPtr->getType())->getElementType();
  }

  uint64_t SrcSize = CGF.CGM.getDataLayout().getTypeAllocSize(SrcTy);

  // If the source and destination are integer or pointer types, just do an
  // extension or truncation to the desired type.
  if ((isa<llvm::IntegerType>(Ty) || isa<llvm::PointerType>(Ty)) &&
      (isa<llvm::IntegerType>(SrcTy) || isa<llvm::PointerType>(SrcTy))) {
    llvm::LoadInst *Load =
      CGF.Builder.CreateAlignedLoad(SrcPtr, SrcAlign.getQuantity());
    return CoerceIntOrPtrToIntOrPtr(Load, Ty, CGF);
  }

  // If load is legal, just bitcast the src pointer.
  if (SrcSize >= DstSize) {
    // Generally SrcSize is never greater than DstSize, since this means we are
    // losing bits. However, this can happen in cases where the structure has
    // additional padding, for example due to a user specified alignment.
    //
    // FIXME: Assert that we aren't truncating non-padding bits when have access
    // to that information.
    llvm::Value *Casted =
      CGF.Builder.CreateBitCast(SrcPtr, llvm::PointerType::getUnqual(Ty));
    return CGF.Builder.CreateAlignedLoad(Casted, SrcAlign.getQuantity());
  }

  // Otherwise do coercion through memory. This is stupid, but
  // simple.
  llvm::AllocaInst *Tmp = CGF.CreateTempAlloca(Ty);
  Tmp->setAlignment(SrcAlign.getQuantity());
  llvm::Type *I8PtrTy = CGF.Builder.getInt8PtrTy();
  llvm::Value *Casted = CGF.Builder.CreateBitCast(Tmp, I8PtrTy);
  llvm::Value *SrcCasted = CGF.Builder.CreateBitCast(SrcPtr, I8PtrTy);
  CGF.Builder.CreateMemCpy(Casted, SrcCasted,
      llvm::ConstantInt::get(CGF.IntPtrTy, SrcSize),
      SrcAlign.getQuantity(), false);
  return CGF.Builder.CreateAlignedLoad(Tmp, SrcAlign.getQuantity());
}

// Function to store a first-class aggregate into memory.  We prefer to
// store the elements rather than the aggregate to be more friendly to
// fast-isel.
// FIXME: Do we need to recurse here?
static void BuildAggStore(CodeGenFunction &CGF, llvm::Value *Val,
                          llvm::Value *DestPtr, bool DestIsVolatile,
                          CharUnits DestAlign) {
  // Prefer scalar stores to first-class aggregate stores.
  if (llvm::StructType *STy =
        dyn_cast<llvm::StructType>(Val->getType())) {
    // HLSL Change Begins
    assert(!CGF.getLangOpts().HLSL &&
           "HLSL uses SRet so this should not be possible to reach.");
    // HLSL Change Ends
    const llvm::StructLayout *Layout =
      CGF.CGM.getDataLayout().getStructLayout(STy);

    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      llvm::Value *EltPtr = CGF.Builder.CreateConstGEP2_32(STy, DestPtr, 0, i);
      llvm::Value *Elt = CGF.Builder.CreateExtractValue(Val, i);
      uint64_t EltOffset = Layout->getElementOffset(i);
      CharUnits EltAlign =
        DestAlign.alignmentAtOffset(CharUnits::fromQuantity(EltOffset));
      CGF.Builder.CreateAlignedStore(Elt, EltPtr, EltAlign.getQuantity(),
                                     DestIsVolatile);
    }
  } else {
    CGF.Builder.CreateAlignedStore(Val, DestPtr, DestAlign.getQuantity(),
                                   DestIsVolatile);
  }
}

/// CreateCoercedStore - Create a store to \arg DstPtr from \arg Src,
/// where the source and destination may have different types.  The
/// destination is known to be aligned to \arg DstAlign bytes.
///
/// This safely handles the case when the src type is larger than the
/// destination type; the upper bits of the src will be lost.
static void CreateCoercedStore(llvm::Value *Src,
                               llvm::Value *DstPtr,
                               bool DstIsVolatile,
                               CharUnits DstAlign,
                               CodeGenFunction &CGF) {
  llvm::Type *SrcTy = Src->getType();
  llvm::Type *DstTy =
    cast<llvm::PointerType>(DstPtr->getType())->getElementType();
  if (SrcTy == DstTy) {
    CGF.Builder.CreateAlignedStore(Src, DstPtr, DstAlign.getQuantity(),
                                   DstIsVolatile);
    return;
  }

  uint64_t SrcSize = CGF.CGM.getDataLayout().getTypeAllocSize(SrcTy);

  if (llvm::StructType *DstSTy = dyn_cast<llvm::StructType>(DstTy)) {
    DstPtr = EnterStructPointerForCoercedAccess(DstPtr, DstSTy, SrcSize, CGF);
    DstTy = cast<llvm::PointerType>(DstPtr->getType())->getElementType();
  }

  // If the source and destination are integer or pointer types, just do an
  // extension or truncation to the desired type.
  if ((isa<llvm::IntegerType>(SrcTy) || isa<llvm::PointerType>(SrcTy)) &&
      (isa<llvm::IntegerType>(DstTy) || isa<llvm::PointerType>(DstTy))) {
    Src = CoerceIntOrPtrToIntOrPtr(Src, DstTy, CGF);
    CGF.Builder.CreateAlignedStore(Src, DstPtr, DstAlign.getQuantity(),
                                   DstIsVolatile);
    return;
  }

  uint64_t DstSize = CGF.CGM.getDataLayout().getTypeAllocSize(DstTy);

  // If store is legal, just bitcast the src pointer.
  if (SrcSize <= DstSize) {
    llvm::Value *Casted =
      CGF.Builder.CreateBitCast(DstPtr, llvm::PointerType::getUnqual(SrcTy));
    BuildAggStore(CGF, Src, Casted, DstIsVolatile, DstAlign);
  } else {
    // Otherwise do coercion through memory. This is stupid, but
    // simple.

    // Generally SrcSize is never greater than DstSize, since this means we are
    // losing bits. However, this can happen in cases where the structure has
    // additional padding, for example due to a user specified alignment.
    //
    // FIXME: Assert that we aren't truncating non-padding bits when have access
    // to that information.
    llvm::AllocaInst *Tmp = CGF.CreateTempAlloca(SrcTy);
    Tmp->setAlignment(DstAlign.getQuantity());
    CGF.Builder.CreateAlignedStore(Src, Tmp, DstAlign.getQuantity());
    llvm::Type *I8PtrTy = CGF.Builder.getInt8PtrTy();
    llvm::Value *Casted = CGF.Builder.CreateBitCast(Tmp, I8PtrTy);
    llvm::Value *DstCasted = CGF.Builder.CreateBitCast(DstPtr, I8PtrTy);
    CGF.Builder.CreateMemCpy(DstCasted, Casted,
        llvm::ConstantInt::get(CGF.IntPtrTy, DstSize),
        DstAlign.getQuantity(), false);
  }
}

namespace {

/// Encapsulates information about the way function arguments from
/// CGFunctionInfo should be passed to actual LLVM IR function.
class ClangToLLVMArgMapping {
  static const unsigned InvalidIndex = ~0U;
  unsigned InallocaArgNo;
  unsigned SRetArgNo;
  unsigned TotalIRArgs;

  /// Arguments of LLVM IR function corresponding to single Clang argument.
  struct IRArgs {
    unsigned PaddingArgIndex;
    // Argument is expanded to IR arguments at positions
    // [FirstArgIndex, FirstArgIndex + NumberOfArgs).
    unsigned FirstArgIndex;
    unsigned NumberOfArgs;

    IRArgs()
        : PaddingArgIndex(InvalidIndex), FirstArgIndex(InvalidIndex),
          NumberOfArgs(0) {}
  };

  SmallVector<IRArgs, 8> ArgInfo;

public:
  ClangToLLVMArgMapping(const ASTContext &Context, const CGFunctionInfo &FI,
                        bool OnlyRequiredArgs = false)
      : InallocaArgNo(InvalidIndex), SRetArgNo(InvalidIndex), TotalIRArgs(0),
        ArgInfo(OnlyRequiredArgs ? FI.getNumRequiredArgs() : FI.arg_size()) {
    construct(Context, FI, OnlyRequiredArgs);
  }

  bool hasInallocaArg() const { return InallocaArgNo != InvalidIndex; }
  unsigned getInallocaArgNo() const {
    assert(hasInallocaArg());
    return InallocaArgNo;
  }

  bool hasSRetArg() const { return SRetArgNo != InvalidIndex; }
  unsigned getSRetArgNo() const {
    assert(hasSRetArg());
    return SRetArgNo;
  }

  unsigned totalIRArgs() const { return TotalIRArgs; }

  bool hasPaddingArg(unsigned ArgNo) const {
    assert(ArgNo < ArgInfo.size());
    return ArgInfo[ArgNo].PaddingArgIndex != InvalidIndex;
  }
  unsigned getPaddingArgNo(unsigned ArgNo) const {
    assert(hasPaddingArg(ArgNo));
    return ArgInfo[ArgNo].PaddingArgIndex;
  }

  /// Returns index of first IR argument corresponding to ArgNo, and their
  /// quantity.
  std::pair<unsigned, unsigned> getIRArgs(unsigned ArgNo) const {
    assert(ArgNo < ArgInfo.size());
    return std::make_pair(ArgInfo[ArgNo].FirstArgIndex,
                          ArgInfo[ArgNo].NumberOfArgs);
  }

private:
  void construct(const ASTContext &Context, const CGFunctionInfo &FI,
                 bool OnlyRequiredArgs);
};

void ClangToLLVMArgMapping::construct(const ASTContext &Context,
                                      const CGFunctionInfo &FI,
                                      bool OnlyRequiredArgs) {
  unsigned IRArgNo = 0;
  bool SwapThisWithSRet = false;
  const ABIArgInfo &RetAI = FI.getReturnInfo();

  if (RetAI.getKind() == ABIArgInfo::Indirect) {
    SwapThisWithSRet = RetAI.isSRetAfterThis();
    SRetArgNo = SwapThisWithSRet ? 1 : IRArgNo++;
  }

  unsigned ArgNo = 0;
  unsigned NumArgs = OnlyRequiredArgs ? FI.getNumRequiredArgs() : FI.arg_size();
  for (CGFunctionInfo::const_arg_iterator I = FI.arg_begin(); ArgNo < NumArgs;
       ++I, ++ArgNo) {
    assert(I != FI.arg_end());
    QualType ArgType = I->type;
    const ABIArgInfo &AI = I->info;
    // Collect data about IR arguments corresponding to Clang argument ArgNo.
    auto &IRArgs = ArgInfo[ArgNo];

    if (AI.getPaddingType())
      IRArgs.PaddingArgIndex = IRArgNo++;

    switch (AI.getKind()) {
    case ABIArgInfo::Extend:
    case ABIArgInfo::Direct: {
      // FIXME: handle sseregparm someday...
      llvm::StructType *STy = dyn_cast<llvm::StructType>(AI.getCoerceToType());
      if (AI.isDirect() && AI.getCanBeFlattened() && STy) {
        IRArgs.NumberOfArgs = STy->getNumElements();
      } else {
        IRArgs.NumberOfArgs = 1;
      }
      break;
    }
    case ABIArgInfo::Indirect:
      IRArgs.NumberOfArgs = 1;
      break;
    case ABIArgInfo::Ignore:
    case ABIArgInfo::InAlloca:
      // ignore and inalloca doesn't have matching LLVM parameters.
      IRArgs.NumberOfArgs = 0;
      break;
    case ABIArgInfo::Expand: {
      IRArgs.NumberOfArgs = getExpansionSize(ArgType, Context);
      break;
    }
    }

    if (IRArgs.NumberOfArgs > 0) {
      IRArgs.FirstArgIndex = IRArgNo;
      IRArgNo += IRArgs.NumberOfArgs;
    }

    // Skip over the sret parameter when it comes second.  We already handled it
    // above.
    if (IRArgNo == 1 && SwapThisWithSRet)
      IRArgNo++;
  }
  assert(ArgNo == ArgInfo.size());

  if (FI.usesInAlloca())
    InallocaArgNo = IRArgNo++;

  TotalIRArgs = IRArgNo;
}
}  // namespace

/***/

bool CodeGenModule::ReturnTypeUsesSRet(const CGFunctionInfo &FI) {
  return FI.getReturnInfo().isIndirect();
}

bool CodeGenModule::ReturnSlotInterferesWithArgs(const CGFunctionInfo &FI) {
  return ReturnTypeUsesSRet(FI) &&
         getTargetCodeGenInfo().doesReturnSlotInterfereWithArgs();
}

bool CodeGenModule::ReturnTypeUsesFPRet(QualType ResultType) {
  if (const BuiltinType *BT = ResultType->getAs<BuiltinType>()) {
    switch (BT->getKind()) {
    default:
      return false;
    case BuiltinType::Float:
      return getTarget().useObjCFPRetForRealType(TargetInfo::Float);
    case BuiltinType::Double:
      return getTarget().useObjCFPRetForRealType(TargetInfo::Double);
    case BuiltinType::LongDouble:
      return getTarget().useObjCFPRetForRealType(TargetInfo::LongDouble);
    }
  }

  return false;
}

bool CodeGenModule::ReturnTypeUsesFP2Ret(QualType ResultType) {
  if (const ComplexType *CT = ResultType->getAs<ComplexType>()) {
    if (const BuiltinType *BT = CT->getElementType()->getAs<BuiltinType>()) {
      if (BT->getKind() == BuiltinType::LongDouble)
        return getTarget().useObjCFP2RetForComplexLongDouble();
    }
  }

  return false;
}

llvm::FunctionType *CodeGenTypes::GetFunctionType(GlobalDecl GD) {
  const CGFunctionInfo &FI = arrangeGlobalDeclaration(GD);
  return GetFunctionType(FI);
}

llvm::FunctionType *
CodeGenTypes::GetFunctionType(const CGFunctionInfo &FI) {

  bool Inserted = FunctionsBeingProcessed.insert(&FI).second;
  (void)Inserted;
  assert(Inserted && "Recursively being processed?");

  llvm::Type *resultType = nullptr;
  const ABIArgInfo &retAI = FI.getReturnInfo();
  switch (retAI.getKind()) {
  case ABIArgInfo::Expand:
    llvm_unreachable("Invalid ABI kind for return argument");

  case ABIArgInfo::Extend:
  case ABIArgInfo::Direct:
    resultType = retAI.getCoerceToType();
    break;

  case ABIArgInfo::InAlloca:
    if (retAI.getInAllocaSRet()) {
      // sret things on win32 aren't void, they return the sret pointer.
      QualType ret = FI.getReturnType();
      llvm::Type *ty = ConvertType(ret);
      unsigned addressSpace = Context.getTargetAddressSpace(ret);
      resultType = llvm::PointerType::get(ty, addressSpace);
    } else {
      resultType = llvm::Type::getVoidTy(getLLVMContext());
    }
    break;

  case ABIArgInfo::Indirect: {
    assert(!retAI.getIndirectAlign() && "Align unused on indirect return.");
    resultType = llvm::Type::getVoidTy(getLLVMContext());
    break;
  }

  case ABIArgInfo::Ignore:
    resultType = llvm::Type::getVoidTy(getLLVMContext());
    break;
  }

  ClangToLLVMArgMapping IRFunctionArgs(getContext(), FI, true);
  SmallVector<llvm::Type*, 8> ArgTypes(IRFunctionArgs.totalIRArgs());

  // Add type for sret argument.
  if (IRFunctionArgs.hasSRetArg()) {
    QualType Ret = FI.getReturnType();
    llvm::Type *Ty = ConvertType(Ret);
    unsigned AddressSpace = Context.getTargetAddressSpace(Ret);
    ArgTypes[IRFunctionArgs.getSRetArgNo()] =
        llvm::PointerType::get(Ty, AddressSpace);
  }

  // Add type for inalloca argument.
  if (IRFunctionArgs.hasInallocaArg()) {
    auto ArgStruct = FI.getArgStruct();
    assert(ArgStruct);
    ArgTypes[IRFunctionArgs.getInallocaArgNo()] = ArgStruct->getPointerTo();
  }

  // Add in all of the required arguments.
  unsigned ArgNo = 0;
  CGFunctionInfo::const_arg_iterator it = FI.arg_begin(),
                                     ie = it + FI.getNumRequiredArgs();
  for (; it != ie; ++it, ++ArgNo) {
    const ABIArgInfo &ArgInfo = it->info;

    // Insert a padding type to ensure proper alignment.
    if (IRFunctionArgs.hasPaddingArg(ArgNo))
      ArgTypes[IRFunctionArgs.getPaddingArgNo(ArgNo)] =
          ArgInfo.getPaddingType();

    unsigned FirstIRArg, NumIRArgs;
    std::tie(FirstIRArg, NumIRArgs) = IRFunctionArgs.getIRArgs(ArgNo);

    switch (ArgInfo.getKind()) {
    case ABIArgInfo::Ignore:
    case ABIArgInfo::InAlloca:
      assert(NumIRArgs == 0);
      break;

    case ABIArgInfo::Indirect: {
      assert(NumIRArgs == 1);
      // indirect arguments are always on the stack, which is addr space #0.
      llvm::Type *LTy = ConvertTypeForMem(it->type);
      ArgTypes[FirstIRArg] = LTy->getPointerTo();
      break;
    }

    case ABIArgInfo::Extend:
    case ABIArgInfo::Direct: {
      // Fast-isel and the optimizer generally like scalar values better than
      // FCAs, so we flatten them if this is safe to do for this argument.
      llvm::Type *argType = ArgInfo.getCoerceToType();
      llvm::StructType *st = dyn_cast<llvm::StructType>(argType);
      if (st && ArgInfo.isDirect() && ArgInfo.getCanBeFlattened()) {
        assert(NumIRArgs == st->getNumElements());
        for (unsigned i = 0, e = st->getNumElements(); i != e; ++i)
          ArgTypes[FirstIRArg + i] = st->getElementType(i);
      } else {
        assert(NumIRArgs == 1);
        ArgTypes[FirstIRArg] = argType;
      }
      break;
    }

    case ABIArgInfo::Expand:
      auto ArgTypesIter = ArgTypes.begin() + FirstIRArg;
      getExpandedTypes(it->type, ArgTypesIter);
      assert(ArgTypesIter == ArgTypes.begin() + FirstIRArg + NumIRArgs);
      break;
    }
  }

  bool Erased = FunctionsBeingProcessed.erase(&FI); (void)Erased;
  assert(Erased && "Not in set?");

  return llvm::FunctionType::get(resultType, ArgTypes, FI.isVariadic());
}

llvm::Type *CodeGenTypes::GetFunctionTypeForVTable(GlobalDecl GD) {
  const CXXMethodDecl *MD = cast<CXXMethodDecl>(GD.getDecl());
  const FunctionProtoType *FPT = MD->getType()->getAs<FunctionProtoType>();

  if (!isFuncTypeConvertible(FPT))
    return llvm::StructType::get(getLLVMContext());
    
  const CGFunctionInfo *Info;
  if (isa<CXXDestructorDecl>(MD))
    Info =
        &arrangeCXXStructorDeclaration(MD, getFromDtorType(GD.getDtorType()));
  else
    Info = &arrangeCXXMethodDeclaration(MD);
  return GetFunctionType(*Info);
}

void CodeGenModule::ConstructAttributeList(const CGFunctionInfo &FI,
                                           const Decl *TargetDecl,
                                           AttributeListType &PAL,
                                           unsigned &CallingConv,
                                           bool AttrOnCallSite) {
  llvm::AttrBuilder FuncAttrs;
  llvm::AttrBuilder RetAttrs;
  bool HasOptnone = false;

  CallingConv = FI.getEffectiveCallingConvention();

  if (FI.isNoReturn())
    FuncAttrs.addAttribute(llvm::Attribute::NoReturn);

  // FIXME: handle sseregparm someday...
  if (TargetDecl) {
    if (TargetDecl->hasAttr<ReturnsTwiceAttr>())
      FuncAttrs.addAttribute(llvm::Attribute::ReturnsTwice);
    if (TargetDecl->hasAttr<NoThrowAttr>())
      FuncAttrs.addAttribute(llvm::Attribute::NoUnwind);
    if (TargetDecl->hasAttr<NoReturnAttr>())
      FuncAttrs.addAttribute(llvm::Attribute::NoReturn);
    if (TargetDecl->hasAttr<NoDuplicateAttr>())
      FuncAttrs.addAttribute(llvm::Attribute::NoDuplicate);

    if (const FunctionDecl *Fn = dyn_cast<FunctionDecl>(TargetDecl)) {
      const FunctionProtoType *FPT = Fn->getType()->getAs<FunctionProtoType>();
      if (FPT && FPT->isNothrow(getContext()))
        FuncAttrs.addAttribute(llvm::Attribute::NoUnwind);
      // Don't use [[noreturn]] or _Noreturn for a call to a virtual function.
      // These attributes are not inherited by overloads.
      const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(Fn);
      if (Fn->isNoReturn() && !(AttrOnCallSite && MD && MD->isVirtual()))
        FuncAttrs.addAttribute(llvm::Attribute::NoReturn);
    }

    // 'const' and 'pure' attribute functions are also nounwind.
    if (TargetDecl->hasAttr<ConstAttr>()) {
      FuncAttrs.addAttribute(llvm::Attribute::ReadNone);
      FuncAttrs.addAttribute(llvm::Attribute::NoUnwind);
    } else if (TargetDecl->hasAttr<PureAttr>()) {
      FuncAttrs.addAttribute(llvm::Attribute::ReadOnly);
      FuncAttrs.addAttribute(llvm::Attribute::NoUnwind);
    }
    if (TargetDecl->hasAttr<RestrictAttr>())
      RetAttrs.addAttribute(llvm::Attribute::NoAlias);
    if (TargetDecl->hasAttr<ReturnsNonNullAttr>())
      RetAttrs.addAttribute(llvm::Attribute::NonNull);

    HasOptnone = TargetDecl->hasAttr<OptimizeNoneAttr>();
  }

  // OptimizeNoneAttr takes precedence over -Os or -Oz. No warning needed.
  if (!HasOptnone) {
    if (CodeGenOpts.OptimizeSize)
      FuncAttrs.addAttribute(llvm::Attribute::OptimizeForSize);
    if (CodeGenOpts.OptimizeSize == 2)
      FuncAttrs.addAttribute(llvm::Attribute::MinSize);
  }

  if (CodeGenOpts.DisableRedZone)
    FuncAttrs.addAttribute(llvm::Attribute::NoRedZone);
  if (CodeGenOpts.NoImplicitFloat)
    FuncAttrs.addAttribute(llvm::Attribute::NoImplicitFloat);
  if (CodeGenOpts.EnableSegmentedStacks &&
      !(TargetDecl && TargetDecl->hasAttr<NoSplitStackAttr>()))
    FuncAttrs.addAttribute("split-stack");

  if (AttrOnCallSite) {
    // Attributes that should go on the call site only.
    if (!CodeGenOpts.SimplifyLibCalls)
      FuncAttrs.addAttribute(llvm::Attribute::NoBuiltin);
    if (!CodeGenOpts.TrapFuncName.empty())
      FuncAttrs.addAttribute("trap-func-name", CodeGenOpts.TrapFuncName);
  } else if (!getLangOpts().HLSL) {
    // Attributes that should go on the function, but not the call site.
    if (!CodeGenOpts.DisableFPElim) {
      FuncAttrs.addAttribute("no-frame-pointer-elim", "false");
    } else if (CodeGenOpts.OmitLeafFramePointer) {
      FuncAttrs.addAttribute("no-frame-pointer-elim", "false");
      FuncAttrs.addAttribute("no-frame-pointer-elim-non-leaf");
    } else {
      FuncAttrs.addAttribute("no-frame-pointer-elim", "true");
      FuncAttrs.addAttribute("no-frame-pointer-elim-non-leaf");
    }

    FuncAttrs.addAttribute("disable-tail-calls",
                           llvm::toStringRef(CodeGenOpts.DisableTailCalls));
    FuncAttrs.addAttribute("less-precise-fpmad",
                           llvm::toStringRef(CodeGenOpts.LessPreciseFPMAD));
    FuncAttrs.addAttribute("no-infs-fp-math",
                           llvm::toStringRef(CodeGenOpts.NoInfsFPMath));
    FuncAttrs.addAttribute("no-nans-fp-math",
                           llvm::toStringRef(CodeGenOpts.NoNaNsFPMath));
    FuncAttrs.addAttribute("unsafe-fp-math",
                           llvm::toStringRef(CodeGenOpts.UnsafeFPMath));
    FuncAttrs.addAttribute("use-soft-float",
                           llvm::toStringRef(CodeGenOpts.SoftFloat));
    FuncAttrs.addAttribute("stack-protector-buffer-size",
                           llvm::utostr(CodeGenOpts.SSPBufferSize));

    if (!CodeGenOpts.StackRealignment)
      FuncAttrs.addAttribute("no-realign-stack");

    // Add target-cpu and target-features attributes to functions. If
    // we have a decl for the function and it has a target attribute then
    // parse that and add it to the feature set.
    StringRef TargetCPU = getTarget().getTargetOpts().CPU;

    // TODO: Features gets us the features on the command line including
    // feature dependencies. For canonicalization purposes we might want to
    // avoid putting features in the target-features set if we know it'll be
    // one of the default features in the backend, e.g. corei7-avx and +avx or
    // figure out non-explicit dependencies.
    // Canonicalize the existing features in a new feature map.
    // TODO: Migrate the existing backends to keep the map around rather than
    // the vector.
    llvm::StringMap<bool> FeatureMap;
    for (auto F : getTarget().getTargetOpts().Features) {
      const char *Name = F.c_str();
      bool Enabled = Name[0] == '+';
      getTarget().setFeatureEnabled(FeatureMap, Name + 1, Enabled);
    }

    const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(TargetDecl);
    if (FD) {
      if (const auto *TD = FD->getAttr<TargetAttr>()) {
        StringRef FeaturesStr = TD->getFeatures();
        SmallVector<StringRef, 1> AttrFeatures;
        FeaturesStr.split(AttrFeatures, ",");

        // Grab the various features and prepend a "+" to turn on the feature to
        // the backend and add them to our existing set of features.
        for (auto &Feature : AttrFeatures) {
	  // Go ahead and trim whitespace rather than either erroring or
	  // accepting it weirdly.
	  Feature = Feature.trim();

          // While we're here iterating check for a different target cpu.
          if (Feature.startswith("arch="))
            TargetCPU = Feature.split("=").second.trim();
	  else if (Feature.startswith("tune="))
	    // We don't support cpu tuning this way currently.
	    ;
	  else if (Feature.startswith("fpmath="))
	    // TODO: Support the fpmath option this way. It will require checking
	    // overall feature validity for the function with the rest of the
	    // attributes on the function.
	    ;
	  else if (Feature.startswith("mno-"))
            getTarget().setFeatureEnabled(FeatureMap, Feature.split("-").second,
                                          false);
          else
            getTarget().setFeatureEnabled(FeatureMap, Feature, true);
        }
      }
    }

    // Produce the canonical string for this set of features.
    std::vector<std::string> Features;
    for (llvm::StringMap<bool>::const_iterator it = FeatureMap.begin(),
                                               ie = FeatureMap.end();
         it != ie; ++it)
      Features.push_back((it->second ? "+" : "-") + it->first().str());

    // Now add the target-cpu and target-features to the function.
    if (TargetCPU != "")
      FuncAttrs.addAttribute("target-cpu", TargetCPU);
    if (!Features.empty()) {
      std::sort(Features.begin(), Features.end());
      FuncAttrs.addAttribute("target-features",
                             llvm::join(Features.begin(), Features.end(), ","));
    }
  }

  ClangToLLVMArgMapping IRFunctionArgs(getContext(), FI);

  QualType RetTy = FI.getReturnType();
  const ABIArgInfo &RetAI = FI.getReturnInfo();
  switch (RetAI.getKind()) {
  case ABIArgInfo::Extend:
    if (RetTy->hasSignedIntegerRepresentation())
      RetAttrs.addAttribute(llvm::Attribute::SExt);
    else if (RetTy->hasUnsignedIntegerRepresentation())
      RetAttrs.addAttribute(llvm::Attribute::ZExt);
    LLVM_FALLTHROUGH; // HLSL Change
  case ABIArgInfo::Direct:
    if (RetAI.getInReg())
      RetAttrs.addAttribute(llvm::Attribute::InReg);
    break;
  case ABIArgInfo::Ignore:
    break;

  case ABIArgInfo::InAlloca:
  case ABIArgInfo::Indirect: {
    // inalloca and sret disable readnone and readonly
    FuncAttrs.removeAttribute(llvm::Attribute::ReadOnly)
      .removeAttribute(llvm::Attribute::ReadNone);
    break;
  }

  case ABIArgInfo::Expand:
    llvm_unreachable("Invalid ABI kind for return argument");
  }

  if (const auto *RefTy = RetTy->getAs<ReferenceType>()) {
    QualType PTy = RefTy->getPointeeType();
    if (!PTy->isIncompleteType() && PTy->isConstantSizeType())
      RetAttrs.addDereferenceableAttr(getContext().getTypeSizeInChars(PTy)
                                        .getQuantity());
    else if (getContext().getTargetAddressSpace(PTy) == 0)
      RetAttrs.addAttribute(llvm::Attribute::NonNull);
  }

  // Attach return attributes.
  if (RetAttrs.hasAttributes()) {
    PAL.push_back(llvm::AttributeSet::get(
        getLLVMContext(), llvm::AttributeSet::ReturnIndex, RetAttrs));
  }

  // Attach attributes to sret.
  if (IRFunctionArgs.hasSRetArg()) {
    llvm::AttrBuilder SRETAttrs;
    SRETAttrs.addAttribute(llvm::Attribute::StructRet);
    if (RetAI.getInReg())
      SRETAttrs.addAttribute(llvm::Attribute::InReg);
    PAL.push_back(llvm::AttributeSet::get(
        getLLVMContext(), IRFunctionArgs.getSRetArgNo() + 1, SRETAttrs));
  }

  // Attach attributes to inalloca argument.
  if (IRFunctionArgs.hasInallocaArg()) {
    llvm::AttrBuilder Attrs;
    Attrs.addAttribute(llvm::Attribute::InAlloca);
    PAL.push_back(llvm::AttributeSet::get(
        getLLVMContext(), IRFunctionArgs.getInallocaArgNo() + 1, Attrs));
  }

  unsigned ArgNo = 0;
  for (CGFunctionInfo::const_arg_iterator I = FI.arg_begin(),
                                          E = FI.arg_end();
       I != E; ++I, ++ArgNo) {
    QualType ParamType = I->type;
    const ABIArgInfo &AI = I->info;
    llvm::AttrBuilder Attrs;

    // Add attribute for padding argument, if necessary.
    if (IRFunctionArgs.hasPaddingArg(ArgNo)) {
      if (AI.getPaddingInReg())
        PAL.push_back(llvm::AttributeSet::get(
            getLLVMContext(), IRFunctionArgs.getPaddingArgNo(ArgNo) + 1,
            llvm::Attribute::InReg));
    }

    // 'restrict' -> 'noalias' is done in EmitFunctionProlog when we
    // have the corresponding parameter variable.  It doesn't make
    // sense to do it here because parameters are so messed up.
    switch (AI.getKind()) {
    case ABIArgInfo::Extend:
      if (ParamType->isSignedIntegerOrEnumerationType())
        Attrs.addAttribute(llvm::Attribute::SExt);
      else if (ParamType->isUnsignedIntegerOrEnumerationType()) {
        if (getTypes().getABIInfo().shouldSignExtUnsignedType(ParamType))
          Attrs.addAttribute(llvm::Attribute::SExt);
        else
          Attrs.addAttribute(llvm::Attribute::ZExt);
      }
      LLVM_FALLTHROUGH; // HLSL Change
    case ABIArgInfo::Direct:
      if (ArgNo == 0 && FI.isChainCall())
        Attrs.addAttribute(llvm::Attribute::Nest);
      else if (AI.getInReg())
        Attrs.addAttribute(llvm::Attribute::InReg);
      break;

    case ABIArgInfo::Indirect:
      if (AI.getInReg())
        Attrs.addAttribute(llvm::Attribute::InReg);

      if (AI.getIndirectByVal())
        Attrs.addAttribute(llvm::Attribute::ByVal);

      Attrs.addAlignmentAttr(AI.getIndirectAlign());

      // byval disables readnone and readonly.
      FuncAttrs.removeAttribute(llvm::Attribute::ReadOnly)
        .removeAttribute(llvm::Attribute::ReadNone);
      break;

    case ABIArgInfo::Ignore:
    case ABIArgInfo::Expand:
      continue;

    case ABIArgInfo::InAlloca:
      // inalloca disables readnone and readonly.
      FuncAttrs.removeAttribute(llvm::Attribute::ReadOnly)
          .removeAttribute(llvm::Attribute::ReadNone);
      continue;
    }

    if (const auto *RefTy = ParamType->getAs<ReferenceType>()) {
      QualType PTy = RefTy->getPointeeType();
      if (!PTy->isIncompleteType() && PTy->isConstantSizeType())
        Attrs.addDereferenceableAttr(getContext().getTypeSizeInChars(PTy)
                                       .getQuantity());
      else if (getContext().getTargetAddressSpace(PTy) == 0)
        Attrs.addAttribute(llvm::Attribute::NonNull);
    }

    if (Attrs.hasAttributes()) {
      unsigned FirstIRArg, NumIRArgs;
      std::tie(FirstIRArg, NumIRArgs) = IRFunctionArgs.getIRArgs(ArgNo);
      for (unsigned i = 0; i < NumIRArgs; i++)
        PAL.push_back(llvm::AttributeSet::get(getLLVMContext(),
                                              FirstIRArg + i + 1, Attrs));
    }
  }
  assert(ArgNo == FI.arg_size());

  if (FuncAttrs.hasAttributes())
    PAL.push_back(llvm::
                  AttributeSet::get(getLLVMContext(),
                                    llvm::AttributeSet::FunctionIndex,
                                    FuncAttrs));
}

/// An argument came in as a promoted argument; demote it back to its
/// declared type.
static llvm::Value *emitArgumentDemotion(CodeGenFunction &CGF,
                                         const VarDecl *var,
                                         llvm::Value *value) {
  llvm::Type *varType = CGF.ConvertType(var->getType());

  // This can happen with promotions that actually don't change the
  // underlying type, like the enum promotions.
  if (value->getType() == varType) return value;

  assert((varType->isIntegerTy() || varType->isFloatingPointTy())
         && "unexpected promotion type");

  if (isa<llvm::IntegerType>(varType))
    return CGF.Builder.CreateTrunc(value, varType, "arg.unpromote");

  return CGF.Builder.CreateFPCast(value, varType, "arg.unpromote");
}

/// Returns the attribute (either parameter attribute, or function
/// attribute), which declares argument ArgNo to be non-null.
static const NonNullAttr *getNonNullAttr(const Decl *FD, const ParmVarDecl *PVD,
                                         QualType ArgType, unsigned ArgNo) {
  // FIXME: __attribute__((nonnull)) can also be applied to:
  //   - references to pointers, where the pointee is known to be
  //     nonnull (apparently a Clang extension)
  //   - transparent unions containing pointers
  // In the former case, LLVM IR cannot represent the constraint. In
  // the latter case, we have no guarantee that the transparent union
  // is in fact passed as a pointer.
  if (!ArgType->isAnyPointerType() && !ArgType->isBlockPointerType())
    return nullptr;
  // First, check attribute on parameter itself.
  if (PVD) {
    if (auto ParmNNAttr = PVD->getAttr<NonNullAttr>())
      return ParmNNAttr;
  }
  // Check function attributes.
  if (!FD)
    return nullptr;
  for (const auto *NNAttr : FD->specific_attrs<NonNullAttr>()) {
    if (NNAttr->isNonNull(ArgNo))
      return NNAttr;
  }
  return nullptr;
}

void CodeGenFunction::EmitFunctionProlog(const CGFunctionInfo &FI,
                                         llvm::Function *Fn,
                                         const FunctionArgList &Args) {
  if (CurCodeDecl && CurCodeDecl->hasAttr<NakedAttr>())
    // Naked functions don't have prologues.
    return;

  // If this is an implicit-return-zero function, go ahead and
  // initialize the return value.  TODO: it might be nice to have
  // a more general mechanism for this that didn't require synthesized
  // return statements.
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CurCodeDecl)) {
    if (FD->hasImplicitReturnZero()) {
      QualType RetTy = FD->getReturnType().getUnqualifiedType();
      llvm::Type* LLVMTy = CGM.getTypes().ConvertType(RetTy);
      llvm::Constant* Zero = llvm::Constant::getNullValue(LLVMTy);
      Builder.CreateStore(Zero, ReturnValue);
    }
  }

  // FIXME: We no longer need the types from FunctionArgList; lift up and
  // simplify.

  ClangToLLVMArgMapping IRFunctionArgs(CGM.getContext(), FI);
  // Flattened function arguments.
  SmallVector<llvm::Argument *, 16> FnArgs;
  FnArgs.reserve(IRFunctionArgs.totalIRArgs());
  for (auto &Arg : Fn->args()) {
    FnArgs.push_back(&Arg);
  }
  assert(FnArgs.size() == IRFunctionArgs.totalIRArgs());

  // If we're using inalloca, all the memory arguments are GEPs off of the last
  // parameter, which is a pointer to the complete memory area.
  llvm::Value *ArgStruct = nullptr;
  if (IRFunctionArgs.hasInallocaArg()) {
    ArgStruct = FnArgs[IRFunctionArgs.getInallocaArgNo()];
    assert(ArgStruct->getType() == FI.getArgStruct()->getPointerTo());
  }

  // Name the struct return parameter.
  if (IRFunctionArgs.hasSRetArg()) {
    auto AI = FnArgs[IRFunctionArgs.getSRetArgNo()];
    AI->setName("agg.result");
    AI->addAttr(llvm::AttributeSet::get(getLLVMContext(), AI->getArgNo() + 1,
                                        llvm::Attribute::NoAlias));
  }

  // Track if we received the parameter as a pointer (indirect, byval, or
  // inalloca).  If already have a pointer, EmitParmDecl doesn't need to copy it
  // into a local alloca for us.
  enum ValOrPointer { HaveValue = 0, HavePointer = 1 };
  typedef llvm::PointerIntPair<llvm::Value *, 1> ValueAndIsPtr;
  SmallVector<ValueAndIsPtr, 16> ArgVals;
  ArgVals.reserve(Args.size());

  // Create a pointer value for every parameter declaration.  This usually
  // entails copying one or more LLVM IR arguments into an alloca.  Don't push
  // any cleanups or do anything that might unwind.  We do that separately, so
  // we can push the cleanups in the correct order for the ABI.
  assert(FI.arg_size() == Args.size() &&
         "Mismatch between function signature & arguments.");
  unsigned ArgNo = 0;
  CGFunctionInfo::const_arg_iterator info_it = FI.arg_begin();
  for (FunctionArgList::const_iterator i = Args.begin(), e = Args.end();
       i != e; ++i, ++info_it, ++ArgNo) {
    const VarDecl *Arg = *i;
    QualType Ty = info_it->type;
    const ABIArgInfo &ArgI = info_it->info;

    bool isPromoted = !getLangOpts().HLSL && // HLSL Change - no knr promotion in HLSL
      isa<ParmVarDecl>(Arg) && cast<ParmVarDecl>(Arg)->isKNRPromoted();

    unsigned FirstIRArg, NumIRArgs;
    std::tie(FirstIRArg, NumIRArgs) = IRFunctionArgs.getIRArgs(ArgNo);

    switch (ArgI.getKind()) {
    case ABIArgInfo::InAlloca: {
      assert(NumIRArgs == 0);
      llvm::Value *V =
          Builder.CreateStructGEP(FI.getArgStruct(), ArgStruct,
                                  ArgI.getInAllocaFieldIndex(), Arg->getName());
      ArgVals.push_back(ValueAndIsPtr(V, HavePointer));
      break;
    }

    case ABIArgInfo::Indirect: {
      assert(NumIRArgs == 1);
      llvm::Value *V = FnArgs[FirstIRArg];

      if (!hasScalarEvaluationKind(Ty)) {
        // Aggregates and complex variables are accessed by reference.  All we
        // need to do is realign the value, if requested
        if (ArgI.getIndirectRealign()) {
          llvm::Value *AlignedTemp = CreateMemTemp(Ty, "coerce");

          // Copy from the incoming argument pointer to the temporary with the
          // appropriate alignment.
          //
          // FIXME: We should have a common utility for generating an aggregate
          // copy.
          llvm::Type *I8PtrTy = Builder.getInt8PtrTy();
          CharUnits Size = getContext().getTypeSizeInChars(Ty);
          llvm::Value *Dst = Builder.CreateBitCast(AlignedTemp, I8PtrTy);
          llvm::Value *Src = Builder.CreateBitCast(V, I8PtrTy);
          Builder.CreateMemCpy(Dst,
                               Src,
                               llvm::ConstantInt::get(IntPtrTy, 
                                                      Size.getQuantity()),
                               ArgI.getIndirectAlign(),
                               false);
          V = AlignedTemp;
        }
        ArgVals.push_back(ValueAndIsPtr(V, HavePointer));
      } else {
        // Load scalar value from indirect argument.
        V = EmitLoadOfScalar(V, false, ArgI.getIndirectAlign(), Ty,
                             Arg->getLocStart());

        if (isPromoted)
          V = emitArgumentDemotion(*this, Arg, V);
        ArgVals.push_back(ValueAndIsPtr(V, HaveValue));
      }
      break;
    }

    case ABIArgInfo::Extend:
    case ABIArgInfo::Direct: {
      // HLSL Change Begins
      if (hlsl::IsHLSLMatType(Ty)) {
        assert(NumIRArgs == 1);
        auto AI = FnArgs[FirstIRArg];
        llvm::Value *V = AI;
        ArgVals.push_back(ValueAndIsPtr(V, HaveValue));
        break;
      }
      // HLSL Change Ends
      // If we have the trivial case, handle it with no muss and fuss.
      if (!isa<llvm::StructType>(ArgI.getCoerceToType()) &&
          ArgI.getCoerceToType() == ConvertType(Ty) &&
          ArgI.getDirectOffset() == 0) {
        assert(NumIRArgs == 1);
        auto AI = FnArgs[FirstIRArg];
        llvm::Value *V = AI;

        if (const ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(Arg)) {
          if (getNonNullAttr(CurCodeDecl, PVD, PVD->getType(),
                             PVD->getFunctionScopeIndex()))
            AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                                AI->getArgNo() + 1,
                                                llvm::Attribute::NonNull));

          QualType OTy = PVD->getOriginalType();
          if (const auto *ArrTy =
              getContext().getAsConstantArrayType(OTy)) {
            // A C99 array parameter declaration with the static keyword also
            // indicates dereferenceability, and if the size is constant we can
            // use the dereferenceable attribute (which requires the size in
            // bytes).
            if (ArrTy->getSizeModifier() == ArrayType::Static) {
              QualType ETy = ArrTy->getElementType();
              uint64_t ArrSize = ArrTy->getSize().getZExtValue();
              if (!ETy->isIncompleteType() && ETy->isConstantSizeType() &&
                  ArrSize) {
                llvm::AttrBuilder Attrs;
                Attrs.addDereferenceableAttr(
                  getContext().getTypeSizeInChars(ETy).getQuantity()*ArrSize);
                AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                                    AI->getArgNo() + 1, Attrs));
              } else if (getContext().getTargetAddressSpace(ETy) == 0) {
                AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                                    AI->getArgNo() + 1,
                                                    llvm::Attribute::NonNull));
              }
            }
          } else if (const auto *ArrTy =
                     getContext().getAsVariableArrayType(OTy)) {
            // For C99 VLAs with the static keyword, we don't know the size so
            // we can't use the dereferenceable attribute, but in addrspace(0)
            // we know that it must be nonnull.
            if (ArrTy->getSizeModifier() == VariableArrayType::Static &&
                !getContext().getTargetAddressSpace(ArrTy->getElementType()))
              AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                                  AI->getArgNo() + 1,
                                                  llvm::Attribute::NonNull));
          }

          const auto *AVAttr = PVD->getAttr<AlignValueAttr>();
          if (!AVAttr)
            if (const auto *TOTy = dyn_cast<TypedefType>(OTy))
              AVAttr = TOTy->getDecl()->getAttr<AlignValueAttr>();
          if (AVAttr) {         
            llvm::Value *AlignmentValue =
              EmitScalarExpr(AVAttr->getAlignment());
            llvm::ConstantInt *AlignmentCI =
              cast<llvm::ConstantInt>(AlignmentValue);
            unsigned Alignment =
              std::min((unsigned) AlignmentCI->getZExtValue(),
                       +llvm::Value::MaximumAlignment);

            llvm::AttrBuilder Attrs;
            Attrs.addAlignmentAttr(Alignment);
            AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                                AI->getArgNo() + 1, Attrs));
          }
        }

        if (Arg->getType().isRestrictQualified())
          AI->addAttr(llvm::AttributeSet::get(getLLVMContext(),
                                              AI->getArgNo() + 1,
                                              llvm::Attribute::NoAlias));

        // Ensure the argument is the correct type.
        if (V->getType() != ArgI.getCoerceToType())
          V = Builder.CreateBitCast(V, ArgI.getCoerceToType());

        if (isPromoted)
          V = emitArgumentDemotion(*this, Arg, V);

        if (const CXXMethodDecl *MD =
            dyn_cast_or_null<CXXMethodDecl>(CurCodeDecl)) {
          if (MD->isVirtual() && Arg == CXXABIThisDecl)
            V = CGM.getCXXABI().
                adjustThisParameterInVirtualFunctionPrologue(*this, CurGD, V);
        }

        // Because of merging of function types from multiple decls it is
        // possible for the type of an argument to not match the corresponding
        // type in the function type. Since we are codegening the callee
        // in here, add a cast to the argument type.
        llvm::Type *LTy = ConvertType(Arg->getType());
        if (V->getType() != LTy)
          V = Builder.CreateBitCast(V, LTy);

        ArgVals.push_back(ValueAndIsPtr(V, HaveValue));
        break;
      }

      llvm::AllocaInst *Alloca = CreateMemTemp(Ty, Arg->getName());

      // The alignment we need to use is the max of the requested alignment for
      // the argument plus the alignment required by our access code below.
      unsigned AlignmentToUse =
        CGM.getDataLayout().getABITypeAlignment(ArgI.getCoerceToType());
      AlignmentToUse = std::max(AlignmentToUse,
                        (unsigned)getContext().getDeclAlign(Arg).getQuantity());

      Alloca->setAlignment(AlignmentToUse);
      llvm::Value *V = Alloca;
      llvm::Value *Ptr = V;    // Pointer to store into.
      CharUnits PtrAlign = CharUnits::fromQuantity(AlignmentToUse);

      // If the value is offset in memory, apply the offset now.
      if (unsigned Offs = ArgI.getDirectOffset()) {
        Ptr = Builder.CreateBitCast(Ptr, Builder.getInt8PtrTy());
        Ptr = Builder.CreateConstGEP1_32(Builder.getInt8Ty(), Ptr, Offs);
        Ptr = Builder.CreateBitCast(Ptr,
                          llvm::PointerType::getUnqual(ArgI.getCoerceToType()));
        PtrAlign = PtrAlign.alignmentAtOffset(CharUnits::fromQuantity(Offs));
      }

      // Fast-isel and the optimizer generally like scalar values better than
      // FCAs, so we flatten them if this is safe to do for this argument.
      llvm::StructType *STy = dyn_cast<llvm::StructType>(ArgI.getCoerceToType());
      if (ArgI.isDirect() && ArgI.getCanBeFlattened() && STy &&
          STy->getNumElements() > 1) {
        uint64_t SrcSize = CGM.getDataLayout().getTypeAllocSize(STy);
        llvm::Type *DstTy =
          cast<llvm::PointerType>(Ptr->getType())->getElementType();
        uint64_t DstSize = CGM.getDataLayout().getTypeAllocSize(DstTy);

        if (SrcSize <= DstSize) {
          Ptr = Builder.CreateBitCast(Ptr, llvm::PointerType::getUnqual(STy));

          assert(STy->getNumElements() == NumIRArgs);
          for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
            auto AI = FnArgs[FirstIRArg + i];
            AI->setName(Arg->getName() + ".coerce" + Twine(i));
            llvm::Value *EltPtr = Builder.CreateConstGEP2_32(STy, Ptr, 0, i);
            Builder.CreateStore(AI, EltPtr);
          }
        } else {
          llvm::AllocaInst *TempAlloca =
            CreateTempAlloca(ArgI.getCoerceToType(), "coerce");
          TempAlloca->setAlignment(AlignmentToUse);
          llvm::Value *TempV = TempAlloca;

          assert(STy->getNumElements() == NumIRArgs);
          for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
            auto AI = FnArgs[FirstIRArg + i];
            AI->setName(Arg->getName() + ".coerce" + Twine(i));
            llvm::Value *EltPtr =
                Builder.CreateConstGEP2_32(ArgI.getCoerceToType(), TempV, 0, i);
            Builder.CreateStore(AI, EltPtr);
          }

          Builder.CreateMemCpy(Ptr, TempV, DstSize, AlignmentToUse);
        }
      } else {
        // Simple case, just do a coerced store of the argument into the alloca.
        assert(NumIRArgs == 1);
        auto AI = FnArgs[FirstIRArg];
        AI->setName(Arg->getName() + ".coerce");
        CreateCoercedStore(AI, Ptr, /*DestIsVolatile=*/false, PtrAlign, *this);
      }


      // Match to what EmitParmDecl is expecting for this type.
      if (CodeGenFunction::hasScalarEvaluationKind(Ty)) {
        V = EmitLoadOfScalar(V, false, AlignmentToUse, Ty, Arg->getLocStart());
        if (isPromoted)
          V = emitArgumentDemotion(*this, Arg, V);
        ArgVals.push_back(ValueAndIsPtr(V, HaveValue));
      } else {
        ArgVals.push_back(ValueAndIsPtr(V, HavePointer));
      }
      break;
    }

    case ABIArgInfo::Expand: {
      // If this structure was expanded into multiple arguments then
      // we need to create a temporary and reconstruct it from the
      // arguments.
      llvm::AllocaInst *Alloca = CreateMemTemp(Ty);
      CharUnits Align = getContext().getDeclAlign(Arg);
      Alloca->setAlignment(Align.getQuantity());
      LValue LV = MakeAddrLValue(Alloca, Ty, Align);
      ArgVals.push_back(ValueAndIsPtr(Alloca, HavePointer));

      auto FnArgIter = FnArgs.begin() + FirstIRArg;
      ExpandTypeFromArgs(Ty, LV, FnArgIter);
      assert(FnArgIter == FnArgs.begin() + FirstIRArg + NumIRArgs);
      for (unsigned i = 0, e = NumIRArgs; i != e; ++i) {
        auto AI = FnArgs[FirstIRArg + i];
        AI->setName(Arg->getName() + "." + Twine(i));
      }
      break;
    }

    case ABIArgInfo::Ignore:
      assert(NumIRArgs == 0);
      // Initialize the local variable appropriately.
      if (!hasScalarEvaluationKind(Ty)) {
        ArgVals.push_back(ValueAndIsPtr(CreateMemTemp(Ty), HavePointer));
      } else {
        llvm::Value *U = llvm::UndefValue::get(ConvertType(Arg->getType()));
        ArgVals.push_back(ValueAndIsPtr(U, HaveValue));
      }
      break;
    }
  }

  if (getTarget().getCXXABI().areArgsDestroyedLeftToRightInCallee()) {
    for (int I = Args.size() - 1; I >= 0; --I)
      EmitParmDecl(*Args[I], ArgVals[I].getPointer(), ArgVals[I].getInt(),
                   I + 1);
  } else {
    for (unsigned I = 0, E = Args.size(); I != E; ++I)
      EmitParmDecl(*Args[I], ArgVals[I].getPointer(), ArgVals[I].getInt(),
                   I + 1);
  }

  // HLSL Change Begins.
  if (getLangOpts().HLSL) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CurCodeDecl)) {
      CGM.getHLSLRuntime().EmitHLSLFunctionProlog(Fn, FD);
    }
  }
  // HLSL Change Ends.
}

#if 0 // HLSL Change Start - no ObjC support
static void eraseUnusedBitCasts(llvm::Instruction *insn) {
  while (insn->use_empty()) {
    llvm::BitCastInst *bitcast = dyn_cast<llvm::BitCastInst>(insn);
    if (!bitcast) return;

    // This is "safe" because we would have used a ConstantExpr otherwise.
    insn = cast<llvm::Instruction>(bitcast->getOperand(0));
    bitcast->eraseFromParent();
  }
}

/// Try to emit a fused autorelease of a return result.
static llvm::Value *tryEmitFusedAutoreleaseOfResult(CodeGenFunction &CGF,
                                                    llvm::Value *result) {
  // We must be immediately followed the cast.
  llvm::BasicBlock *BB = CGF.Builder.GetInsertBlock();
  if (BB->empty()) return nullptr;
  if (&BB->back() != result) return nullptr;

  llvm::Type *resultType = result->getType();

  // result is in a BasicBlock and is therefore an Instruction.
  llvm::Instruction *generator = cast<llvm::Instruction>(result);

  SmallVector<llvm::Instruction*,4> insnsToKill;

  // Look for:
  //  %generator = bitcast %type1* %generator2 to %type2*
  while (llvm::BitCastInst *bitcast = dyn_cast<llvm::BitCastInst>(generator)) {
    // We would have emitted this as a constant if the operand weren't
    // an Instruction.
    generator = cast<llvm::Instruction>(bitcast->getOperand(0));

    // Require the generator to be immediately followed by the cast.
    if (generator->getNextNode() != bitcast)
      return nullptr;

    insnsToKill.push_back(bitcast);
  }

  // Look for:
  //   %generator = call i8* @objc_retain(i8* %originalResult)
  // or
  //   %generator = call i8* @objc_retainAutoreleasedReturnValue(i8* %originalResult)
  llvm::CallInst *call = dyn_cast<llvm::CallInst>(generator);
  if (!call) return nullptr;

  bool doRetainAutorelease;

  if (call->getCalledValue() == CGF.CGM.getARCEntrypoints().objc_retain) {
    doRetainAutorelease = true;
  } else if (call->getCalledValue() == CGF.CGM.getARCEntrypoints()
                                          .objc_retainAutoreleasedReturnValue) {
    doRetainAutorelease = false;

    // If we emitted an assembly marker for this call (and the
    // ARCEntrypoints field should have been set if so), go looking
    // for that call.  If we can't find it, we can't do this
    // optimization.  But it should always be the immediately previous
    // instruction, unless we needed bitcasts around the call.
    if (CGF.CGM.getARCEntrypoints().retainAutoreleasedReturnValueMarker) {
      llvm::Instruction *prev = call->getPrevNode();
      assert(prev);
      if (isa<llvm::BitCastInst>(prev)) {
        prev = prev->getPrevNode();
        assert(prev);
      }
      assert(isa<llvm::CallInst>(prev));
      assert(cast<llvm::CallInst>(prev)->getCalledValue() ==
               CGF.CGM.getARCEntrypoints().retainAutoreleasedReturnValueMarker);
      insnsToKill.push_back(prev);
    }
  } else {
    return nullptr;
  }

  result = call->getArgOperand(0);
  insnsToKill.push_back(call);

  // Keep killing bitcasts, for sanity.  Note that we no longer care
  // about precise ordering as long as there's exactly one use.
  while (llvm::BitCastInst *bitcast = dyn_cast<llvm::BitCastInst>(result)) {
    if (!bitcast->hasOneUse()) break;
    insnsToKill.push_back(bitcast);
    result = bitcast->getOperand(0);
  }

  // Delete all the unnecessary instructions, from latest to earliest.
  for (SmallVectorImpl<llvm::Instruction*>::iterator
         i = insnsToKill.begin(), e = insnsToKill.end(); i != e; ++i)
    (*i)->eraseFromParent();

  // Do the fused retain/autorelease if we were asked to.
  if (doRetainAutorelease)
    result = CGF.EmitARCRetainAutoreleaseReturnValue(result);

  // Cast back to the result type.
  return CGF.Builder.CreateBitCast(result, resultType);
}

/// If this is a +1 of the value of an immutable 'self', remove it.
static llvm::Value *tryRemoveRetainOfSelf(CodeGenFunction &CGF,
                                          llvm::Value *result) {
  // This is only applicable to a method with an immutable 'self'.
  const ObjCMethodDecl *method =
    dyn_cast_or_null<ObjCMethodDecl>(CGF.CurCodeDecl);
  if (!method) return nullptr;
  const VarDecl *self = method->getSelfDecl();
  if (!self->getType().isConstQualified()) return nullptr;

  // Look for a retain call.
  llvm::CallInst *retainCall =
    dyn_cast<llvm::CallInst>(result->stripPointerCasts());
  if (!retainCall ||
      retainCall->getCalledValue() != CGF.CGM.getARCEntrypoints().objc_retain)
    return nullptr;

  // Look for an ordinary load of 'self'.
  llvm::Value *retainedValue = retainCall->getArgOperand(0);
  llvm::LoadInst *load =
    dyn_cast<llvm::LoadInst>(retainedValue->stripPointerCasts());
  if (!load || load->isAtomic() || load->isVolatile() || 
      load->getPointerOperand() != CGF.GetAddrOfLocalVar(self))
    return nullptr;

  // Okay!  Burn it all down.  This relies for correctness on the
  // assumption that the retain is emitted as part of the return and
  // that thereafter everything is used "linearly".
  llvm::Type *resultType = result->getType();
  eraseUnusedBitCasts(cast<llvm::Instruction>(result));
  assert(retainCall->use_empty());
  retainCall->eraseFromParent();
  eraseUnusedBitCasts(cast<llvm::Instruction>(retainedValue));

  return CGF.Builder.CreateBitCast(load, resultType);
}

/// Emit an ARC autorelease of the result of a function.
///
/// \return the value to actually return from the function
static llvm::Value *emitAutoreleaseOfResult(CodeGenFunction &CGF,
                                            llvm::Value *result) {
  // If we're returning 'self', kill the initial retain.  This is a
  // heuristic attempt to "encourage correctness" in the really unfortunate
  // case where we have a return of self during a dealloc and we desperately
  // need to avoid the possible autorelease.
  if (llvm::Value *self = tryRemoveRetainOfSelf(CGF, result))
    return self;

  // At -O0, try to emit a fused retain/autorelease.
  if (CGF.shouldUseFusedARCCalls())
    if (llvm::Value *fused = tryEmitFusedAutoreleaseOfResult(CGF, result))
      return fused;

  return CGF.EmitARCAutoreleaseReturnValue(result);
}
#endif // HLSL Change Ends - no ObjC support

/// Heuristically search for a dominating store to the return-value slot.
static llvm::StoreInst *findDominatingStoreToReturnValue(CodeGenFunction &CGF) {
  // If there are multiple uses of the return-value slot, just check
  // for something immediately preceding the IP.  Sometimes this can
  // happen with how we generate implicit-returns; it can also happen
  // with noreturn cleanups.
  if (!CGF.ReturnValue->hasOneUse()) {
    llvm::BasicBlock *IP = CGF.Builder.GetInsertBlock();
    if (IP->empty()) return nullptr;
    llvm::Instruction *I = &IP->back();

    // Skip lifetime markers
    for (llvm::BasicBlock::reverse_iterator II = IP->rbegin(),
                                            IE = IP->rend();
         II != IE; ++II) {
      if (llvm::IntrinsicInst *Intrinsic =
              dyn_cast<llvm::IntrinsicInst>(&*II)) {
        if (Intrinsic->getIntrinsicID() == llvm::Intrinsic::lifetime_end) {
          const llvm::Value *CastAddr = Intrinsic->getArgOperand(1);
          ++II;
          if (II == IE)
            break;
          if (isa<llvm::BitCastInst>(&*II) && (CastAddr == &*II))
            continue;
        }
      }
      I = &*II;
      break;
    }

    llvm::StoreInst *store = dyn_cast<llvm::StoreInst>(I);
    if (!store) return nullptr;
    if (store->getPointerOperand() != CGF.ReturnValue) return nullptr;
    assert(!store->isAtomic() && !store->isVolatile()); // see below
    return store;
  }

  llvm::StoreInst *store =
    dyn_cast<llvm::StoreInst>(CGF.ReturnValue->user_back());
  if (!store) return nullptr;

  // These aren't actually possible for non-coerced returns, and we
  // only care about non-coerced returns on this code path.
  assert(!store->isAtomic() && !store->isVolatile());

  // Now do a first-and-dirty dominance check: just walk up the
  // single-predecessors chain from the current insertion point.
  llvm::BasicBlock *StoreBB = store->getParent();
  llvm::BasicBlock *IP = CGF.Builder.GetInsertBlock();
  while (IP != StoreBB) {
    if (!(IP = IP->getSinglePredecessor()))
      return nullptr;
  }

  // Okay, the store's basic block dominates the insertion point; we
  // can do our thing.
  return store;
}

void CodeGenFunction::EmitFunctionEpilog(const CGFunctionInfo &FI,
                                         bool EmitRetDbgLoc,
                                         SourceLocation EndLoc) {
  if (CurCodeDecl && CurCodeDecl->hasAttr<NakedAttr>()) {
    // Naked functions don't have epilogues.
    Builder.CreateUnreachable();
    return;
  }

  // Functions with no result always return void.
  if (!ReturnValue) {
    Builder.CreateRetVoid();
    return;
  }

  llvm::DebugLoc RetDbgLoc;
  llvm::Value *RV = nullptr;
  QualType RetTy = FI.getReturnType();
  const ABIArgInfo &RetAI = FI.getReturnInfo();

  switch (RetAI.getKind()) {
  case ABIArgInfo::InAlloca:
    // Aggregrates get evaluated directly into the destination.  Sometimes we
    // need to return the sret value in a register, though.
    assert(hasAggregateEvaluationKind(RetTy));
    if (RetAI.getInAllocaSRet()) {
      llvm::Function::arg_iterator EI = CurFn->arg_end();
      --EI;
      llvm::Value *ArgStruct = EI;
      llvm::Value *SRet = Builder.CreateStructGEP(
          nullptr, ArgStruct, RetAI.getInAllocaFieldIndex());
      RV = Builder.CreateLoad(SRet, "sret");
    }
    break;

  case ABIArgInfo::Indirect: {
    auto AI = CurFn->arg_begin();
    if (RetAI.isSRetAfterThis())
      ++AI;
    switch (getEvaluationKind(RetTy)) {
    case TEK_Complex: {
      ComplexPairTy RT =
        EmitLoadOfComplex(MakeNaturalAlignAddrLValue(ReturnValue, RetTy),
                          EndLoc);
      EmitStoreOfComplex(RT, MakeNaturalAlignAddrLValue(AI, RetTy),
                         /*isInit*/ true);
      break;
    }
    case TEK_Aggregate:
      // Do nothing; aggregrates get evaluated directly into the destination.
      break;
    case TEK_Scalar:
      EmitStoreOfScalar(Builder.CreateLoad(ReturnValue),
                        MakeNaturalAlignAddrLValue(AI, RetTy),
                        /*isInit*/ true);
      break;
    }
    break;
  }

  case ABIArgInfo::Extend:
  case ABIArgInfo::Direct:
    if (RetAI.getCoerceToType() == ConvertType(RetTy) &&
        RetAI.getDirectOffset() == 0) {
      // HLSL Change Begin.
      // If optimization is disabled, just load return value.
      if (CGM.getCodeGenOpts().DisableLLVMOpts) {
        // HLSL Change Begins
        if (hlsl::IsHLSLMatType(RetTy))
          RV = CGM.getHLSLRuntime().EmitHLSLMatrixLoad(*this, ReturnValue,
                                      FnRetTy);  // FnRetTy retains attributed type
        else
          // HLSL Change Ends
          RV = Builder.CreateLoad(ReturnValue);
      } else {
      // HLSL Change End.
      // The internal return value temp always will have pointer-to-return-type
      // type, just do a load.

      // If there is a dominating store to ReturnValue, we can elide
      // the load, zap the store, and usually zap the alloca.
      if (llvm::StoreInst *SI =
              findDominatingStoreToReturnValue(*this)) {
        // Reuse the debug location from the store unless there is
        // cleanup code to be emitted between the store and return
        // instruction.
        if (EmitRetDbgLoc && !AutoreleaseResult)
          RetDbgLoc = SI->getDebugLoc();
        // Get the stored value and nuke the now-dead store.
        RV = SI->getValueOperand();
        SI->eraseFromParent();

        // If that was the only use of the return value, nuke it as well now.
        if (ReturnValue->use_empty() && isa<llvm::AllocaInst>(ReturnValue)) {
          cast<llvm::AllocaInst>(ReturnValue)->eraseFromParent();
          ReturnValue = nullptr;
        }

      // Otherwise, we have to do a simple load.
      } else {
        // HLSL Change Begins
        if (hlsl::IsHLSLMatType(RetTy))
          RV = CGM.getHLSLRuntime().EmitHLSLMatrixLoad(*this, ReturnValue,
                                      FnRetTy);  // FnRetTy retains attributed type
        else
          // HLSL Change Ends
          RV = Builder.CreateLoad(ReturnValue);
      }
      } // HLSL Change
    } else {
      llvm::Value *V = ReturnValue;
      CharUnits Align = getContext().getTypeAlignInChars(RetTy);
      // If the value is offset in memory, apply the offset now.
      if (unsigned Offs = RetAI.getDirectOffset()) {
        V = Builder.CreateBitCast(V, Builder.getInt8PtrTy());
        V = Builder.CreateConstGEP1_32(Builder.getInt8Ty(), V, Offs);
        V = Builder.CreateBitCast(V,
                         llvm::PointerType::getUnqual(RetAI.getCoerceToType()));
        Align = Align.alignmentAtOffset(CharUnits::fromQuantity(Offs));
      }

      RV = CreateCoercedLoad(V, RetAI.getCoerceToType(), Align, *this);
    }

    // In ARC, end functions that return a retainable type with a call
    // to objc_autoreleaseReturnValue.
#if 0 // HLSL Change - no ObjC support
    if (AutoreleaseResult) {
      assert(getLangOpts().ObjCAutoRefCount &&
             !FI.isReturnsRetained() &&
             RetTy->isObjCRetainableType());
      RV = emitAutoreleaseOfResult(*this, RV);
    }
#else
    assert(!AutoreleaseResult && "autorelease not supported in HLSL");
#endif // HLSL Change - no ObjC support
    break;

  case ABIArgInfo::Ignore:
    break;

  case ABIArgInfo::Expand:
    llvm_unreachable("Invalid ABI kind for return argument");
  }

  llvm::Instruction *Ret;
  if (RV) {
    if (SanOpts.has(SanitizerKind::ReturnsNonnullAttribute)) {
      if (auto RetNNAttr = CurGD.getDecl()->getAttr<ReturnsNonNullAttr>()) {
        SanitizerScope SanScope(this);
        llvm::Value *Cond = Builder.CreateICmpNE(
            RV, llvm::Constant::getNullValue(RV->getType()));
        llvm::Constant *StaticData[] = {
            EmitCheckSourceLocation(EndLoc),
            EmitCheckSourceLocation(RetNNAttr->getLocation()),
        };
        EmitCheck(std::make_pair(Cond, SanitizerKind::ReturnsNonnullAttribute),
                  "nonnull_return", StaticData, None);
      }
    }
    Ret = Builder.CreateRet(RV);
  } else {
    Ret = Builder.CreateRetVoid();
  }

  if (RetDbgLoc)
    Ret->setDebugLoc(std::move(RetDbgLoc));
}

static bool isInAllocaArgument(CGCXXABI &ABI, QualType type) {
  const CXXRecordDecl *RD = type->getAsCXXRecordDecl();
  return RD && ABI.getRecordArgABI(RD) == CGCXXABI::RAA_DirectInMemory;
}

static AggValueSlot createPlaceholderSlot(CodeGenFunction &CGF, QualType Ty) {
  // FIXME: Generate IR in one pass, rather than going back and fixing up these
  // placeholders.
  llvm::Type *IRTy = CGF.ConvertTypeForMem(Ty);
  llvm::Value *Placeholder =
      llvm::UndefValue::get(IRTy->getPointerTo()->getPointerTo());
  Placeholder = CGF.Builder.CreateLoad(Placeholder);
  return AggValueSlot::forAddr(Placeholder, CharUnits::Zero(),
                               Ty.getQualifiers(),
                               AggValueSlot::IsNotDestructed,
                               AggValueSlot::DoesNotNeedGCBarriers,
                               AggValueSlot::IsNotAliased);
}

void CodeGenFunction::EmitDelegateCallArg(CallArgList &args,
                                          const VarDecl *param,
                                          SourceLocation loc) {
  // StartFunction converted the ABI-lowered parameter(s) into a
  // local alloca.  We need to turn that into an r-value suitable
  // for EmitCall.
  llvm::Value *local = GetAddrOfLocalVar(param);

  QualType type = param->getType();

  // For the most part, we just need to load the alloca, except:
  // 1) aggregate r-values are actually pointers to temporaries, and
  // 2) references to non-scalars are pointers directly to the aggregate.
  // I don't know why references to scalars are different here.
  if (const ReferenceType *ref = type->getAs<ReferenceType>()) {
    if (!hasScalarEvaluationKind(ref->getPointeeType()))
      return args.add(RValue::getAggregate(local), type);

    // Locals which are references to scalars are represented
    // with allocas holding the pointer.
    return args.add(RValue::get(Builder.CreateLoad(local)), type);
  }

  assert(!isInAllocaArgument(CGM.getCXXABI(), type) &&
         "cannot emit delegate call arguments for inalloca arguments!");

  args.add(convertTempToRValue(local, type, loc), type);
}

#if 0 // HLSL Change - no ObjC support
static bool isProvablyNull(llvm::Value *addr) {
  return isa<llvm::ConstantPointerNull>(addr);
}

static bool isProvablyNonNull(llvm::Value *addr) {
  return isa<llvm::AllocaInst>(addr);
}

/// Emit the actual writing-back of a writeback.
static void emitWriteback(CodeGenFunction &CGF,
                          const CallArgList::Writeback &writeback) {
  const LValue &srcLV = writeback.Source;
  llvm::Value *srcAddr = srcLV.getAddress();
  assert(!isProvablyNull(srcAddr) &&
         "shouldn't have writeback for provably null argument");

  llvm::BasicBlock *contBB = nullptr;

  // If the argument wasn't provably non-null, we need to null check
  // before doing the store.
  bool provablyNonNull = isProvablyNonNull(srcAddr);
  if (!provablyNonNull) {
    llvm::BasicBlock *writebackBB = CGF.createBasicBlock("icr.writeback");
    contBB = CGF.createBasicBlock("icr.done");

    llvm::Value *isNull = CGF.Builder.CreateIsNull(srcAddr, "icr.isnull");
    CGF.Builder.CreateCondBr(isNull, contBB, writebackBB);
    CGF.EmitBlock(writebackBB);
  }

  // Load the value to writeback.
  llvm::Value *value = CGF.Builder.CreateLoad(writeback.Temporary);

  // Cast it back, in case we're writing an id to a Foo* or something.
  value = CGF.Builder.CreateBitCast(value,
               cast<llvm::PointerType>(srcAddr->getType())->getElementType(),
                            "icr.writeback-cast");
  
  // Perform the writeback.

  // If we have a "to use" value, it's something we need to emit a use
  // of.  This has to be carefully threaded in: if it's done after the
  // release it's potentially undefined behavior (and the optimizer
  // will ignore it), and if it happens before the retain then the
  // optimizer could move the release there.
  if (writeback.ToUse) {
    assert(srcLV.getObjCLifetime() == Qualifiers::OCL_Strong);

    // Retain the new value.  No need to block-copy here:  the block's
    // being passed up the stack.
    value = CGF.EmitARCRetainNonBlock(value);

    // Emit the intrinsic use here.
    CGF.EmitARCIntrinsicUse(writeback.ToUse);

    // Load the old value (primitively).
    llvm::Value *oldValue = CGF.EmitLoadOfScalar(srcLV, SourceLocation());

    // Put the new value in place (primitively).
    CGF.EmitStoreOfScalar(value, srcLV, /*init*/ false);

    // Release the old value.
    CGF.EmitARCRelease(oldValue, srcLV.isARCPreciseLifetime());

  // Otherwise, we can just do a normal lvalue store.
  } else {
    CGF.EmitStoreThroughLValue(RValue::get(value), srcLV);
  }

  // Jump to the continuation block.
  if (!provablyNonNull)
    CGF.EmitBlock(contBB);
}

static void emitWritebacks(CodeGenFunction &CGF,
                           const CallArgList &args) {
  for (const auto &I : args.writebacks())
    emitWriteback(CGF, I);
}
#endif // HLSL Change - no ObjC support

static void deactivateArgCleanupsBeforeCall(CodeGenFunction &CGF,
                                            const CallArgList &CallArgs) {
  assert(CGF.getTarget().getCXXABI().areArgsDestroyedLeftToRightInCallee());
  ArrayRef<CallArgList::CallArgCleanup> Cleanups =
    CallArgs.getCleanupsToDeactivate();
  // Iterate in reverse to increase the likelihood of popping the cleanup.
  for (ArrayRef<CallArgList::CallArgCleanup>::reverse_iterator
         I = Cleanups.rbegin(), E = Cleanups.rend(); I != E; ++I) {
    CGF.DeactivateCleanupBlock(I->Cleanup, I->IsActiveIP);
    I->IsActiveIP->eraseFromParent();
  }
}

#if 0 // HLSL Change - no ObjC support
static const Expr *maybeGetUnaryAddrOfOperand(const Expr *E) {
  if (const UnaryOperator *uop = dyn_cast<UnaryOperator>(E->IgnoreParens()))
    if (uop->getOpcode() == UO_AddrOf)
      return uop->getSubExpr();
  return nullptr;
}


/// Emit an argument that's being passed call-by-writeback.  That is,
/// we are passing the address of 
static void emitWritebackArg(CodeGenFunction &CGF, CallArgList &args,
                             const ObjCIndirectCopyRestoreExpr *CRE) {
  LValue srcLV;

  // Make an optimistic effort to emit the address as an l-value.
  // This can fail if the argument expression is more complicated.
  if (const Expr *lvExpr = maybeGetUnaryAddrOfOperand(CRE->getSubExpr())) {
    srcLV = CGF.EmitLValue(lvExpr);

  // Otherwise, just emit it as a scalar.
  } else {
    llvm::Value *srcAddr = CGF.EmitScalarExpr(CRE->getSubExpr());

    QualType srcAddrType =
      CRE->getSubExpr()->getType()->castAs<PointerType>()->getPointeeType();
    srcLV = CGF.MakeNaturalAlignAddrLValue(srcAddr, srcAddrType);
  }
  llvm::Value *srcAddr = srcLV.getAddress();

  // The dest and src types don't necessarily match in LLVM terms
  // because of the crazy ObjC compatibility rules.

  llvm::PointerType *destType =
    cast<llvm::PointerType>(CGF.ConvertType(CRE->getType()));

  // If the address is a constant null, just pass the appropriate null.
  if (isProvablyNull(srcAddr)) {
    args.add(RValue::get(llvm::ConstantPointerNull::get(destType)),
             CRE->getType());
    return;
  }

  // Create the temporary.
  llvm::Value *temp = CGF.CreateTempAlloca(destType->getElementType(),
                                           "icr.temp");
  // Loading an l-value can introduce a cleanup if the l-value is __weak,
  // and that cleanup will be conditional if we can't prove that the l-value
  // isn't null, so we need to register a dominating point so that the cleanups
  // system will make valid IR.
  CodeGenFunction::ConditionalEvaluation condEval(CGF);
  
  // Zero-initialize it if we're not doing a copy-initialization.
  bool shouldCopy = CRE->shouldCopy();
  if (!shouldCopy) {
    llvm::Value *null =
      llvm::ConstantPointerNull::get(
        cast<llvm::PointerType>(destType->getElementType()));
    CGF.Builder.CreateStore(null, temp);
  }

  llvm::BasicBlock *contBB = nullptr;
  llvm::BasicBlock *originBB = nullptr;

  // If the address is *not* known to be non-null, we need to switch.
  llvm::Value *finalArgument;

  bool provablyNonNull = isProvablyNonNull(srcAddr);
  if (provablyNonNull) {
    finalArgument = temp;
  } else {
    llvm::Value *isNull = CGF.Builder.CreateIsNull(srcAddr, "icr.isnull");

    finalArgument = CGF.Builder.CreateSelect(isNull, 
                                   llvm::ConstantPointerNull::get(destType),
                                             temp, "icr.argument");

    // If we need to copy, then the load has to be conditional, which
    // means we need control flow.
    if (shouldCopy) {
      originBB = CGF.Builder.GetInsertBlock();
      contBB = CGF.createBasicBlock("icr.cont");
      llvm::BasicBlock *copyBB = CGF.createBasicBlock("icr.copy");
      CGF.Builder.CreateCondBr(isNull, contBB, copyBB);
      CGF.EmitBlock(copyBB);
      condEval.begin(CGF);
    }
  }

  llvm::Value *valueToUse = nullptr;

  // Perform a copy if necessary.
  if (shouldCopy) {
    RValue srcRV = CGF.EmitLoadOfLValue(srcLV, SourceLocation());
    assert(srcRV.isScalar());

    llvm::Value *src = srcRV.getScalarVal();
    src = CGF.Builder.CreateBitCast(src, destType->getElementType(),
                                    "icr.cast");

    // Use an ordinary store, not a store-to-lvalue.
    CGF.Builder.CreateStore(src, temp);

    // If optimization is enabled, and the value was held in a
    // __strong variable, we need to tell the optimizer that this
    // value has to stay alive until we're doing the store back.
    // This is because the temporary is effectively unretained,
    // and so otherwise we can violate the high-level semantics.
    if (CGF.CGM.getCodeGenOpts().OptimizationLevel != 0 &&
        srcLV.getObjCLifetime() == Qualifiers::OCL_Strong) {
      valueToUse = src;
    }
  }
  
  // Finish the control flow if we needed it.
  if (shouldCopy && !provablyNonNull) {
    llvm::BasicBlock *copyBB = CGF.Builder.GetInsertBlock();
    CGF.EmitBlock(contBB);

    // Make a phi for the value to intrinsically use.
    if (valueToUse) {
      llvm::PHINode *phiToUse = CGF.Builder.CreatePHI(valueToUse->getType(), 2,
                                                      "icr.to-use");
      phiToUse->addIncoming(valueToUse, copyBB);
      phiToUse->addIncoming(llvm::UndefValue::get(valueToUse->getType()),
                            originBB);
      valueToUse = phiToUse;
    }

    condEval.end(CGF);
  }

  args.addWriteback(srcLV, temp, valueToUse);
  args.add(RValue::get(finalArgument), CRE->getType());
}

#endif // HLSL Change - no ObjC support

void CallArgList::allocateArgumentMemory(CodeGenFunction &CGF) {
  assert(!StackBase && !StackCleanup.isValid());

  // Save the stack.
  llvm::Function *F = CGF.CGM.getIntrinsic(llvm::Intrinsic::stacksave);
  StackBase = CGF.Builder.CreateCall(F, {}, "inalloca.save");

  // Control gets really tied up in landing pads, so we have to spill the
  // stacksave to an alloca to avoid violating SSA form.
  // TODO: This is dead if we never emit the cleanup.  We should create the
  // alloca and store lazily on the first cleanup emission.
  StackBaseMem = CGF.CreateTempAlloca(CGF.Int8PtrTy, "inalloca.spmem");
  CGF.Builder.CreateStore(StackBase, StackBaseMem);
  CGF.pushStackRestore(EHCleanup, StackBaseMem);
  StackCleanup = CGF.EHStack.getInnermostEHScope();
  assert(StackCleanup.isValid());
}

void CallArgList::freeArgumentMemory(CodeGenFunction &CGF) const {
  if (StackBase) {
    CGF.DeactivateCleanupBlock(StackCleanup, StackBase);
    llvm::Value *F = CGF.CGM.getIntrinsic(llvm::Intrinsic::stackrestore);
    // We could load StackBase from StackBaseMem, but in the non-exceptional
    // case we can skip it.
    CGF.Builder.CreateCall(F, StackBase);
  }
}

void CodeGenFunction::EmitNonNullArgCheck(RValue RV, QualType ArgType,
                                          SourceLocation ArgLoc,
                                          const FunctionDecl *FD,
                                          unsigned ParmNum) {
  if (!SanOpts.has(SanitizerKind::NonnullAttribute) || !FD)
    return;
  auto PVD = ParmNum < FD->getNumParams() ? FD->getParamDecl(ParmNum) : nullptr;
  unsigned ArgNo = PVD ? PVD->getFunctionScopeIndex() : ParmNum;
  auto NNAttr = getNonNullAttr(FD, PVD, ArgType, ArgNo);
  if (!NNAttr)
    return;
  SanitizerScope SanScope(this);
  assert(RV.isScalar());
  llvm::Value *V = RV.getScalarVal();
  llvm::Value *Cond =
      Builder.CreateICmpNE(V, llvm::Constant::getNullValue(V->getType()));
  llvm::Constant *StaticData[] = {
      EmitCheckSourceLocation(ArgLoc),
      EmitCheckSourceLocation(NNAttr->getLocation()),
      llvm::ConstantInt::get(Int32Ty, ArgNo + 1),
  };
  EmitCheck(std::make_pair(Cond, SanitizerKind::NonnullAttribute),
                "nonnull_arg", StaticData, None);
}

void CodeGenFunction::EmitCallArgs(CallArgList &Args,
                                   ArrayRef<QualType> ArgTypes,
                                   CallExpr::const_arg_iterator ArgBeg,
                                   CallExpr::const_arg_iterator ArgEnd,
                                   const FunctionDecl *CalleeDecl,
                                   unsigned ParamsToSkip) {
  // We *have* to evaluate arguments from right to left in the MS C++ ABI,
  // because arguments are destroyed left to right in the callee.
  if (CGM.getTarget().getCXXABI().areArgsDestroyedLeftToRightInCallee()) {
    // Insert a stack save if we're going to need any inalloca args.
    bool HasInAllocaArgs = false;
    for (ArrayRef<QualType>::iterator I = ArgTypes.begin(), E = ArgTypes.end();
         I != E && !HasInAllocaArgs; ++I)
      HasInAllocaArgs = isInAllocaArgument(CGM.getCXXABI(), *I);
    if (HasInAllocaArgs) {
      assert(getTarget().getTriple().getArch() == llvm::Triple::x86);
      Args.allocateArgumentMemory(*this);
    }

    // Evaluate each argument.
    size_t CallArgsStart = Args.size();
    for (int I = ArgTypes.size() - 1; I >= 0; --I) {
      CallExpr::const_arg_iterator Arg = ArgBeg + I;
      EmitCallArg(Args, *Arg, ArgTypes[I]);
      // HLSL Change begin.
      RValue CallArg = Args.back().RV;
      if (CallArg.isAggregate())
        CGM.getHLSLRuntime().MarkPotentialResourceTemp(
            *this, CallArg.getAggregateAddr(), ArgTypes[I]);
      // HLSL Change end.
      EmitNonNullArgCheck(Args.back().RV, ArgTypes[I], Arg->getExprLoc(),
                          CalleeDecl, ParamsToSkip + I);
    }

    // Un-reverse the arguments we just evaluated so they match up with the LLVM
    // IR function.
    std::reverse(Args.begin() + CallArgsStart, Args.end());
    return;
  }

  for (unsigned I = 0, E = ArgTypes.size(); I != E; ++I) {
    CallExpr::const_arg_iterator Arg = ArgBeg + I;
    assert(Arg != ArgEnd);
    EmitCallArg(Args, *Arg, ArgTypes[I]);
    EmitNonNullArgCheck(Args.back().RV, ArgTypes[I], Arg->getExprLoc(),
                        CalleeDecl, ParamsToSkip + I);
  }
}

namespace {

struct DestroyUnpassedArg : EHScopeStack::Cleanup {
  DestroyUnpassedArg(llvm::Value *Addr, QualType Ty)
      : Addr(Addr), Ty(Ty) {}

  llvm::Value *Addr;
  QualType Ty;

  void Emit(CodeGenFunction &CGF, Flags flags) override {
    const CXXDestructorDecl *Dtor = Ty->getAsCXXRecordDecl()->getDestructor();
    assert(!Dtor->isTrivial());
    CGF.EmitCXXDestructorCall(Dtor, Dtor_Complete, /*for vbase*/ false,
                              /*Delegating=*/false, Addr);
  }
};

}

struct DisableDebugLocationUpdates {
  CodeGenFunction &CGF;
  bool disabledDebugInfo;
  DisableDebugLocationUpdates(CodeGenFunction &CGF, const Expr *E) : CGF(CGF) {
    if ((disabledDebugInfo = isa<CXXDefaultArgExpr>(E) && CGF.getDebugInfo()))
      CGF.disableDebugInfo();
  }
  ~DisableDebugLocationUpdates() {
    if (disabledDebugInfo)
      CGF.enableDebugInfo();
  }
};

void CodeGenFunction::EmitCallArg(CallArgList &args, const Expr *E,
                                  QualType type) {
  DisableDebugLocationUpdates Dis(*this, E);
#if 0 // HLSL Change - no ObjC support
  if (const ObjCIndirectCopyRestoreExpr *CRE
        = dyn_cast<ObjCIndirectCopyRestoreExpr>(E)) {
    assert(getLangOpts().ObjCAutoRefCount);
    assert(getContext().hasSameType(E->getType(), type));
    return emitWritebackArg(*this, args, CRE);
  }
#endif // HLSL Change - no ObjC support

  assert(type->isReferenceType() == E->isGLValue() &&
         "reference binding to unmaterialized r-value!");

  if (E->isGLValue()) {
    // HLSL Change Begins.
    if (E->getObjectKind() == OK_VectorComponent) {
      if (const HLSLVectorElementExpr *VecElt = dyn_cast<HLSLVectorElementExpr>(E)) {
        LValue LV = EmitHLSLVectorElementExpr(VecElt);
        llvm::Value *Ptr = nullptr;
        if (LV.isSimple()) {
          // Handle the special case when the vector component access
          // is done on a scalar using .x or .r.
          //
          // Example 1:
          // groupshared uint g;
          // InterlockedAdd(g.x, 1);
          //
          // Example 2:
          // RWBuffer<uint> buf;
          // InterlockedAdd(buf[0].r, 1);
          llvm::Value *V = LV.getAddress();
          Ptr = Builder.CreateGEP(V, Builder.getInt32(0));
        } else {
          llvm::Value *V = LV.getExtVectorAddr();
          llvm::Constant *Elts = LV.getExtVectorElts();
          // Only support scalar for atomic operations.
          assert(Elts->getType()->getVectorNumElements() == 1);
          llvm::Value *ch = Builder.CreateExtractElement(Elts, (uint64_t)0);
          Ptr = Builder.CreateGEP(V, { Builder.getInt32(0), ch });
        }
        RValue RV = RValue::get(Ptr);
        return args.add(RV, type);
      } else {
        LValue LV = EmitExtMatrixElementExpr(cast<ExtMatrixElementExpr>(E));
        llvm::Value *Ptr = LV.getAddress();
        // Only support scalar for atomic operations.
        assert(Ptr->getType()->getPointerElementType() == Ptr->getType()->getPointerElementType()->getScalarType());

        RValue RV = RValue::get(Ptr);
        return args.add(RV, type);
      }
    }
    // HLSL Change Ends.
    assert(E->getObjectKind() == OK_Ordinary);
    return args.add(EmitReferenceBindingToExpr(E), type);
  }

  bool HasAggregateEvalKind = hasAggregateEvaluationKind(type);

  // In the Microsoft C++ ABI, aggregate arguments are destructed by the callee.
  // However, we still have to push an EH-only cleanup in case we unwind before
  // we make it to the call.
  if (HasAggregateEvalKind && 
      !LangOptions().HLSL && // HLSL Change : Do not generate agg.tmp for HLSL
      CGM.getTarget().getCXXABI().areArgsDestroyedLeftToRightInCallee()) {
    // If we're using inalloca, use the argument memory.  Otherwise, use a
    // temporary.
    AggValueSlot Slot;
    if (args.isUsingInAlloca())
      Slot = createPlaceholderSlot(*this, type);
    else
      Slot = CreateAggTemp(type, "agg.tmp");

    const CXXRecordDecl *RD = type->getAsCXXRecordDecl();
    bool DestroyedInCallee =
        RD && RD->hasNonTrivialDestructor() &&
        CGM.getCXXABI().getRecordArgABI(RD) != CGCXXABI::RAA_Default;
    if (DestroyedInCallee)
      Slot.setExternallyDestructed();

    EmitAggExpr(E, Slot);
    RValue RV = Slot.asRValue();
    args.add(RV, type);

    if (DestroyedInCallee) {
      // Create a no-op GEP between the placeholder and the cleanup so we can
      // RAUW it successfully.  It also serves as a marker of the first
      // instruction where the cleanup is active.
      pushFullExprCleanup<DestroyUnpassedArg>(EHCleanup, Slot.getAddr(), type);
      // This unreachable is a temporary marker which will be removed later.
      llvm::Instruction *IsActive = Builder.CreateUnreachable();
      args.addArgCleanupDeactivation(EHStack.getInnermostEHScope(), IsActive);
    }
    return;
  }

  if (HasAggregateEvalKind && isa<ImplicitCastExpr>(E) &&
      cast<CastExpr>(E)->getCastKind() == CK_LValueToRValue) {
    LValue L = EmitLValue(cast<CastExpr>(E)->getSubExpr());
    assert(L.isSimple());
    if (L.getAlignment() >= getContext().getTypeAlignInChars(type)) {
      // HLSL Change Begin - don't copy input arg.
      // Copy for out param is done at CGMSHLSLRuntime::EmitHLSLOutParamConversion*.
      args.add(L.asAggregateRValue(), type); // /*NeedsCopy*/true);
      // HLSL Change End
    } else {
      // We can't represent a misaligned lvalue in the CallArgList, so copy
      // to an aligned temporary now.
      llvm::Value *tmp = CreateMemTemp(type);
      EmitAggregateCopy(tmp, L.getAddress(), type, L.isVolatile(),
                        L.getAlignment());
      args.add(RValue::getAggregate(tmp), type);
    }
    return;
  }

  // HLSL Change Begins.
  // For DeclRefExpr of aggregate type, don't create temp.
  if (HasAggregateEvalKind && LangOptions().HLSL &&
      isa<DeclRefExpr>(E)) {
    LValue LV = EmitDeclRefLValue(cast<DeclRefExpr>(E));
    RValue RV = RValue::getAggregate(LV.getAddress());
    args.add(RV, type);
    return;
  }
  // HLSL Change Ends.
  args.add(EmitAnyExprToTemp(E), type);
}

QualType CodeGenFunction::getVarArgType(const Expr *Arg) {
  // System headers on Windows define NULL to 0 instead of 0LL on Win64. MSVC
  // implicitly widens null pointer constants that are arguments to varargs
  // functions to pointer-sized ints.
  if (!getTarget().getTriple().isOSWindows())
    return Arg->getType();

  if (Arg->getType()->isIntegerType() &&
      getContext().getTypeSize(Arg->getType()) <
          getContext().getTargetInfo().getPointerWidth(0) &&
      Arg->isNullPointerConstant(getContext(),
                                 Expr::NPC_ValueDependentIsNotNull)) {
    return getContext().getIntPtrType();
  }

  return Arg->getType();
}

// In ObjC ARC mode with no ObjC ARC exception safety, tell the ARC
// optimizer it can aggressively ignore unwind edges.
void
CodeGenFunction::AddObjCARCExceptionMetadata(llvm::Instruction *Inst) {
  if (CGM.getCodeGenOpts().OptimizationLevel != 0 &&
      !CGM.getCodeGenOpts().ObjCAutoRefCountExceptions)
    Inst->setMetadata("clang.arc.no_objc_arc_exceptions",
                      CGM.getNoObjCARCExceptionsMetadata());
}

/// Emits a call to the given no-arguments nounwind runtime function.
llvm::CallInst *
CodeGenFunction::EmitNounwindRuntimeCall(llvm::Value *callee,
                                         const llvm::Twine &name) {
  return EmitNounwindRuntimeCall(callee, None, name);
}

/// Emits a call to the given nounwind runtime function.
llvm::CallInst *
CodeGenFunction::EmitNounwindRuntimeCall(llvm::Value *callee,
                                         ArrayRef<llvm::Value*> args,
                                         const llvm::Twine &name) {
  llvm::CallInst *call = EmitRuntimeCall(callee, args, name);
  call->setDoesNotThrow();
  return call;
}

/// Emits a simple call (never an invoke) to the given no-arguments
/// runtime function.
llvm::CallInst *
CodeGenFunction::EmitRuntimeCall(llvm::Value *callee,
                                 const llvm::Twine &name) {
  return EmitRuntimeCall(callee, None, name);
}

/// Emits a simple call (never an invoke) to the given runtime
/// function.
llvm::CallInst *
CodeGenFunction::EmitRuntimeCall(llvm::Value *callee,
                                 ArrayRef<llvm::Value*> args,
                                 const llvm::Twine &name) {
  llvm::CallInst *call = Builder.CreateCall(callee, args, name);
  call->setCallingConv(getRuntimeCC());
  return call;
}

/// Emits a call or invoke to the given noreturn runtime function.
void CodeGenFunction::EmitNoreturnRuntimeCallOrInvoke(llvm::Value *callee,
                                               ArrayRef<llvm::Value*> args) {
  if (getInvokeDest()) {
    llvm::InvokeInst *invoke = 
      Builder.CreateInvoke(callee,
                           getUnreachableBlock(),
                           getInvokeDest(),
                           args);
    invoke->setDoesNotReturn();
    invoke->setCallingConv(getRuntimeCC());
  } else {
    llvm::CallInst *call = Builder.CreateCall(callee, args);
    call->setDoesNotReturn();
    call->setCallingConv(getRuntimeCC());
    Builder.CreateUnreachable();
  }
}

/// Emits a call or invoke instruction to the given nullary runtime
/// function.
llvm::CallSite
CodeGenFunction::EmitRuntimeCallOrInvoke(llvm::Value *callee,
                                         const Twine &name) {
  return EmitRuntimeCallOrInvoke(callee, None, name);
}

/// Emits a call or invoke instruction to the given runtime function.
llvm::CallSite
CodeGenFunction::EmitRuntimeCallOrInvoke(llvm::Value *callee,
                                         ArrayRef<llvm::Value*> args,
                                         const Twine &name) {
  llvm::CallSite callSite = EmitCallOrInvoke(callee, args, name);
  callSite.setCallingConv(getRuntimeCC());
  return callSite;
}

llvm::CallSite
CodeGenFunction::EmitCallOrInvoke(llvm::Value *Callee,
                                  const Twine &Name) {
  return EmitCallOrInvoke(Callee, None, Name);
}

/// Emits a call or invoke instruction to the given function, depending
/// on the current state of the EH stack.
llvm::CallSite
CodeGenFunction::EmitCallOrInvoke(llvm::Value *Callee,
                                  ArrayRef<llvm::Value *> Args,
                                  const Twine &Name) {
  llvm::BasicBlock *InvokeDest = getInvokeDest();

  llvm::Instruction *Inst;
  if (!InvokeDest)
    Inst = Builder.CreateCall(Callee, Args, Name);
  else {
    llvm::BasicBlock *ContBB = createBasicBlock("invoke.cont");
    Inst = Builder.CreateInvoke(Callee, ContBB, InvokeDest, Args, Name);
    EmitBlock(ContBB);
  }

  // In ObjC ARC mode with no ObjC ARC exception safety, tell the ARC
  // optimizer it can aggressively ignore unwind edges.
  if (CGM.getLangOpts().ObjCAutoRefCount)
    AddObjCARCExceptionMetadata(Inst);

  return llvm::CallSite(Inst);
}

/// \brief Store a non-aggregate value to an address to initialize it.  For
/// initialization, a non-atomic store will be used.
static void EmitInitStoreOfNonAggregate(CodeGenFunction &CGF, RValue Src,
                                        LValue Dst) {
  if (Src.isScalar())
    CGF.EmitStoreOfScalar(Src.getScalarVal(), Dst, /*init=*/true);
  else
    CGF.EmitStoreOfComplex(Src.getComplexVal(), Dst, /*init=*/true);
}

void CodeGenFunction::deferPlaceholderReplacement(llvm::Instruction *Old,
                                                  llvm::Value *New) {
  DeferredReplacements.push_back(std::make_pair(Old, New));
}

RValue CodeGenFunction::EmitCall(const CGFunctionInfo &CallInfo,
                                 llvm::Value *Callee,
                                 ReturnValueSlot ReturnValue,
                                 const CallArgList &CallArgs,
                                 const Decl *TargetDecl,
                                 llvm::Instruction **callOrInvoke) {
  // FIXME: We no longer need the types from CallArgs; lift up and simplify.

  // Handle struct-return functions by passing a pointer to the
  // location that we would like to return into.
  QualType RetTy = CallInfo.getReturnType();
  const ABIArgInfo &RetAI = CallInfo.getReturnInfo();

  llvm::FunctionType *IRFuncTy =
    cast<llvm::FunctionType>(
                  cast<llvm::PointerType>(Callee->getType())->getElementType());

  // If we're using inalloca, insert the allocation after the stack save.
  // FIXME: Do this earlier rather than hacking it in here!
  llvm::AllocaInst *ArgMemory = nullptr;
  if (llvm::StructType *ArgStruct = CallInfo.getArgStruct()) {
    llvm::Instruction *IP = CallArgs.getStackBase();
    llvm::AllocaInst *AI;
    if (IP) {
      IP = IP->getNextNode();
      AI = new llvm::AllocaInst(ArgStruct, "argmem", IP);
    } else {
      AI = CreateTempAlloca(ArgStruct, "argmem");
    }
    AI->setUsedWithInAlloca(true);
    assert(AI->isUsedWithInAlloca() && !AI->isStaticAlloca());
    ArgMemory = AI;
  }

  ClangToLLVMArgMapping IRFunctionArgs(CGM.getContext(), CallInfo);
  SmallVector<llvm::Value *, 16> IRCallArgs(IRFunctionArgs.totalIRArgs());

  // If the call returns a temporary with struct return, create a temporary
  // alloca to hold the result, unless one is given to us.
  llvm::Value *SRetPtr = nullptr;
  size_t UnusedReturnSize = 0;
  if (RetAI.isIndirect() || RetAI.isInAlloca()) {
    SRetPtr = ReturnValue.getValue();
    if (!SRetPtr) {
      SRetPtr = CreateMemTemp(RetTy);
      if (HaveInsertPoint() && ReturnValue.isUnused()) {
        uint64_t size =
            CGM.getDataLayout().getTypeAllocSize(ConvertTypeForMem(RetTy));
        if (EmitLifetimeStart(size, SRetPtr))
          UnusedReturnSize = size;
      }
    }
    // HLSL Change begin.
    CGM.getHLSLRuntime().MarkPotentialResourceTemp(*this, SRetPtr, RetTy);
    // HLSL Change end.
    if (IRFunctionArgs.hasSRetArg()) {
      IRCallArgs[IRFunctionArgs.getSRetArgNo()] = SRetPtr;
    } else {
      llvm::Value *Addr =
          Builder.CreateStructGEP(ArgMemory->getAllocatedType(), ArgMemory,
                                  RetAI.getInAllocaFieldIndex());
      Builder.CreateStore(SRetPtr, Addr);
    }
  }

  assert(CallInfo.arg_size() == CallArgs.size() &&
         "Mismatch between function signature & arguments.");
  unsigned ArgNo = 0;
  CGFunctionInfo::const_arg_iterator info_it = CallInfo.arg_begin();
  for (CallArgList::const_iterator I = CallArgs.begin(), E = CallArgs.end();
       I != E; ++I, ++info_it, ++ArgNo) {
    const ABIArgInfo &ArgInfo = info_it->info;
    RValue RV = I->RV;

    CharUnits TypeAlign = getContext().getTypeAlignInChars(I->Ty);

    // Insert a padding argument to ensure proper alignment.
    if (IRFunctionArgs.hasPaddingArg(ArgNo))
      IRCallArgs[IRFunctionArgs.getPaddingArgNo(ArgNo)] =
          llvm::UndefValue::get(ArgInfo.getPaddingType());

    unsigned FirstIRArg, NumIRArgs;
    std::tie(FirstIRArg, NumIRArgs) = IRFunctionArgs.getIRArgs(ArgNo);

    switch (ArgInfo.getKind()) {
    case ABIArgInfo::InAlloca: {
      assert(NumIRArgs == 0);
      assert(getTarget().getTriple().getArch() == llvm::Triple::x86);
      if (RV.isAggregate()) {
        // Replace the placeholder with the appropriate argument slot GEP.
        llvm::Instruction *Placeholder =
            cast<llvm::Instruction>(RV.getAggregateAddr());
        CGBuilderTy::InsertPoint IP = Builder.saveIP();
        Builder.SetInsertPoint(Placeholder);
        llvm::Value *Addr =
            Builder.CreateStructGEP(ArgMemory->getAllocatedType(), ArgMemory,
                                    ArgInfo.getInAllocaFieldIndex());
        Builder.restoreIP(IP);
        deferPlaceholderReplacement(Placeholder, Addr);
      } else {
        // Store the RValue into the argument struct.
        llvm::Value *Addr =
            Builder.CreateStructGEP(ArgMemory->getAllocatedType(), ArgMemory,
                                    ArgInfo.getInAllocaFieldIndex());
        unsigned AS = Addr->getType()->getPointerAddressSpace();
        llvm::Type *MemType = ConvertTypeForMem(I->Ty)->getPointerTo(AS);
        // There are some cases where a trivial bitcast is not avoidable.  The
        // definition of a type later in a translation unit may change it's type
        // from {}* to (%struct.foo*)*.
        if (Addr->getType() != MemType)
          Addr = Builder.CreateBitCast(Addr, MemType);
        LValue argLV = MakeAddrLValue(Addr, I->Ty, TypeAlign);
        EmitInitStoreOfNonAggregate(*this, RV, argLV);
      }
      break;
    }

    case ABIArgInfo::Indirect: {
      assert(NumIRArgs == 1);
      if (RV.isScalar() || RV.isComplex()) {
        // Make a temporary alloca to pass the argument.
        llvm::AllocaInst *AI = CreateMemTemp(I->Ty);
        if (ArgInfo.getIndirectAlign() > AI->getAlignment())
          AI->setAlignment(ArgInfo.getIndirectAlign());
        IRCallArgs[FirstIRArg] = AI;

        LValue argLV = MakeAddrLValue(AI, I->Ty, TypeAlign);
        EmitInitStoreOfNonAggregate(*this, RV, argLV);
      } else {
        // We want to avoid creating an unnecessary temporary+copy here;
        // however, we need one in three cases:
        // 1. If the argument is not byval, and we are required to copy the
        //    source.  (This case doesn't occur on any common architecture.)
        // 2. If the argument is byval, RV is not sufficiently aligned, and
        //    we cannot force it to be sufficiently aligned.
        // 3. If the argument is byval, but RV is located in an address space
        //    different than that of the argument (0).
        llvm::Value *Addr = RV.getAggregateAddr();
        unsigned Align = ArgInfo.getIndirectAlign();
        const llvm::DataLayout *TD = &CGM.getDataLayout();
        const unsigned RVAddrSpace = Addr->getType()->getPointerAddressSpace();
        const unsigned ArgAddrSpace =
            (FirstIRArg < IRFuncTy->getNumParams()
                 ? IRFuncTy->getParamType(FirstIRArg)->getPointerAddressSpace()
                 : 0);
        if ((!ArgInfo.getIndirectByVal() && I->NeedsCopy) ||
            (ArgInfo.getIndirectByVal() && TypeAlign.getQuantity() < Align &&
             llvm::getOrEnforceKnownAlignment(Addr, Align, *TD) < Align) ||
            (ArgInfo.getIndirectByVal() && (RVAddrSpace != ArgAddrSpace))) {
          // Create an aligned temporary, and copy to it.
          llvm::AllocaInst *AI = CreateMemTemp(I->Ty);
          if (Align > AI->getAlignment())
            AI->setAlignment(Align);
          IRCallArgs[FirstIRArg] = AI;
          EmitAggregateCopy(AI, Addr, I->Ty, RV.isVolatileQualified());
        } else {
          // Skip the extra memcpy call.
          // HLSL Change Starts
          // Generate AddrSpaceCast for shared memory.
          if (RVAddrSpace != ArgAddrSpace) {
            Addr = Builder.CreateAddrSpaceCast(
                Addr, IRFuncTy->getParamType(FirstIRArg));
          }
          // HLSL Change Ends
          IRCallArgs[FirstIRArg] = Addr;
        }
      }
      break;
    }

    case ABIArgInfo::Ignore:
      assert(NumIRArgs == 0);
      break;

    case ABIArgInfo::Extend:
    case ABIArgInfo::Direct: {
      if (!isa<llvm::StructType>(ArgInfo.getCoerceToType()) &&
          ArgInfo.getCoerceToType() == ConvertType(info_it->type) &&
          ArgInfo.getDirectOffset() == 0) {
        assert(NumIRArgs == 1);
        llvm::Value *V;
        if (RV.isScalar())
          V = RV.getScalarVal();
        else
          V = Builder.CreateLoad(RV.getAggregateAddr());

        // We might have to widen integers, but we should never truncate.
        if (ArgInfo.getCoerceToType() != V->getType() &&
            V->getType()->isIntegerTy())
          V = Builder.CreateZExt(V, ArgInfo.getCoerceToType());

        // If the argument doesn't match, perform a bitcast to coerce it.  This
        // can happen due to trivial type mismatches.
        if (FirstIRArg < IRFuncTy->getNumParams() &&
            V->getType() != IRFuncTy->getParamType(FirstIRArg)) {
          // HLSL Change Starts
          // Generate AddrSpaceCast for shared memory.
          if (V->getType()->isPointerTy())
            V = Builder.CreatePointerBitCastOrAddrSpaceCast(
                V, IRFuncTy->getParamType(FirstIRArg));
          else
            // HLSL Change Ends
            V = Builder.CreateBitCast(V, IRFuncTy->getParamType(FirstIRArg));
        }
        IRCallArgs[FirstIRArg] = V;
        break;
      }

      // HLSL Change Begins
      if (hlsl::IsHLSLMatType(I->Ty)) {
        // For matrix, just use the val directly
        IRCallArgs[FirstIRArg] = RV.getScalarVal();
        continue;
      }
      // HLSL Change Ends

      // FIXME: Avoid the conversion through memory if possible.
      llvm::Value *SrcPtr;
      CharUnits SrcAlign;
      if (RV.isScalar() || RV.isComplex()) {
        SrcPtr = CreateMemTemp(I->Ty, "coerce");
        SrcAlign = TypeAlign;
        LValue SrcLV = MakeAddrLValue(SrcPtr, I->Ty, TypeAlign);
        EmitInitStoreOfNonAggregate(*this, RV, SrcLV);
      } else {
        SrcPtr = RV.getAggregateAddr();
        // This alignment is guaranteed by EmitCallArg.
        SrcAlign = TypeAlign;
      }

      // If the value is offset in memory, apply the offset now.
      if (unsigned Offs = ArgInfo.getDirectOffset()) {
        SrcPtr = Builder.CreateBitCast(SrcPtr, Builder.getInt8PtrTy());
        SrcPtr = Builder.CreateConstGEP1_32(Builder.getInt8Ty(), SrcPtr, Offs);
        SrcPtr = Builder.CreateBitCast(SrcPtr,
                       llvm::PointerType::getUnqual(ArgInfo.getCoerceToType()));
        SrcAlign = SrcAlign.alignmentAtOffset(CharUnits::fromQuantity(Offs));
      }

      // Fast-isel and the optimizer generally like scalar values better than
      // FCAs, so we flatten them if this is safe to do for this argument.
      llvm::StructType *STy =
            dyn_cast<llvm::StructType>(ArgInfo.getCoerceToType());
      if (STy && ArgInfo.isDirect() && ArgInfo.getCanBeFlattened()) {
        llvm::Type *SrcTy =
          cast<llvm::PointerType>(SrcPtr->getType())->getElementType();
        uint64_t SrcSize = CGM.getDataLayout().getTypeAllocSize(SrcTy);
        uint64_t DstSize = CGM.getDataLayout().getTypeAllocSize(STy);

        // If the source type is smaller than the destination type of the
        // coerce-to logic, copy the source value into a temp alloca the size
        // of the destination type to allow loading all of it. The bits past
        // the source value are left undef.
        if (SrcSize < DstSize) {
          llvm::AllocaInst *TempAlloca
            = CreateTempAlloca(STy, SrcPtr->getName() + ".coerce");
          Builder.CreateMemCpy(TempAlloca, SrcPtr, SrcSize, 0);
          SrcPtr = TempAlloca;
        } else {
          SrcPtr = Builder.CreateBitCast(SrcPtr,
                                         llvm::PointerType::getUnqual(STy));
        }

        assert(NumIRArgs == STy->getNumElements());
        for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
          llvm::Value *EltPtr = Builder.CreateConstGEP2_32(STy, SrcPtr, 0, i);
          llvm::LoadInst *LI = Builder.CreateLoad(EltPtr);
          // We don't know what we're loading from.
          LI->setAlignment(1);
          IRCallArgs[FirstIRArg + i] = LI;
        }
      } else {
        // In the simple case, just pass the coerced loaded value.
        assert(NumIRArgs == 1);
        IRCallArgs[FirstIRArg] =
            CreateCoercedLoad(SrcPtr, ArgInfo.getCoerceToType(),
                              SrcAlign, *this);
      }

      break;
    }

    case ABIArgInfo::Expand:
      unsigned IRArgPos = FirstIRArg;
      ExpandTypeToArgs(I->Ty, RV, IRFuncTy, IRCallArgs, IRArgPos);
      assert(IRArgPos == FirstIRArg + NumIRArgs);
      break;
    }
  }

  if (ArgMemory) {
    llvm::Value *Arg = ArgMemory;
    if (CallInfo.isVariadic()) {
      // When passing non-POD arguments by value to variadic functions, we will
      // end up with a variadic prototype and an inalloca call site.  In such
      // cases, we can't do any parameter mismatch checks.  Give up and bitcast
      // the callee.
      unsigned CalleeAS =
          cast<llvm::PointerType>(Callee->getType())->getAddressSpace();
      Callee = Builder.CreateBitCast(
          Callee, getTypes().GetFunctionType(CallInfo)->getPointerTo(CalleeAS));
    } else {
      llvm::Type *LastParamTy =
          IRFuncTy->getParamType(IRFuncTy->getNumParams() - 1);
      if (Arg->getType() != LastParamTy) {
#ifndef NDEBUG
        // Assert that these structs have equivalent element types.
        llvm::StructType *FullTy = CallInfo.getArgStruct();
        llvm::StructType *DeclaredTy = cast<llvm::StructType>(
            cast<llvm::PointerType>(LastParamTy)->getElementType());
        assert(DeclaredTy->getNumElements() == FullTy->getNumElements());
        for (llvm::StructType::element_iterator DI = DeclaredTy->element_begin(),
                                                DE = DeclaredTy->element_end(),
                                                FI = FullTy->element_begin();
             DI != DE; ++DI, ++FI)
          assert(*DI == *FI);
#endif
        Arg = Builder.CreateBitCast(Arg, LastParamTy);
      }
    }
    assert(IRFunctionArgs.hasInallocaArg());
    IRCallArgs[IRFunctionArgs.getInallocaArgNo()] = Arg;
  }

  if (!CallArgs.getCleanupsToDeactivate().empty())
    deactivateArgCleanupsBeforeCall(*this, CallArgs);

  // If the callee is a bitcast of a function to a varargs pointer to function
  // type, check to see if we can remove the bitcast.  This handles some cases
  // with unprototyped functions.
  if (llvm::ConstantExpr *CE = dyn_cast<llvm::ConstantExpr>(Callee))
    if (llvm::Function *CalleeF = dyn_cast<llvm::Function>(CE->getOperand(0))) {
      llvm::PointerType *CurPT=cast<llvm::PointerType>(Callee->getType());
      llvm::FunctionType *CurFT =
        cast<llvm::FunctionType>(CurPT->getElementType());
      llvm::FunctionType *ActualFT = CalleeF->getFunctionType();

      if (CE->getOpcode() == llvm::Instruction::BitCast &&
          ActualFT->getReturnType() == CurFT->getReturnType() &&
          ActualFT->getNumParams() == CurFT->getNumParams() &&
          ActualFT->getNumParams() == IRCallArgs.size() &&
          (CurFT->isVarArg() || !ActualFT->isVarArg())) {
        bool ArgsMatch = true;
        for (unsigned i = 0, e = ActualFT->getNumParams(); i != e; ++i)
          if (ActualFT->getParamType(i) != CurFT->getParamType(i)) {
            ArgsMatch = false;
            break;
          }

        // Strip the cast if we can get away with it.  This is a nice cleanup,
        // but also allows us to inline the function at -O0 if it is marked
        // always_inline.
        if (ArgsMatch)
          Callee = CalleeF;
      }
    }

  assert(IRCallArgs.size() == IRFuncTy->getNumParams() || IRFuncTy->isVarArg());
  for (unsigned i = 0; i < IRCallArgs.size(); ++i) {
    // Inalloca argument can have different type.
    if (IRFunctionArgs.hasInallocaArg() &&
        i == IRFunctionArgs.getInallocaArgNo())
      continue;
    if (i < IRFuncTy->getNumParams())
      assert(IRCallArgs[i]->getType() == IRFuncTy->getParamType(i));
  }

  unsigned CallingConv;
  CodeGen::AttributeListType AttributeList;
  CGM.ConstructAttributeList(CallInfo, TargetDecl, AttributeList,
                             CallingConv, true);
  llvm::AttributeSet Attrs = llvm::AttributeSet::get(getLLVMContext(),
                                                     AttributeList);

  llvm::BasicBlock *InvokeDest = nullptr;
  if (!Attrs.hasAttribute(llvm::AttributeSet::FunctionIndex,
                          llvm::Attribute::NoUnwind) ||
      currentFunctionUsesSEHTry())
    InvokeDest = getInvokeDest();

  llvm::CallSite CS;
  if (!InvokeDest) {
    // HLSL changes begin
    // When storing a matrix to memory, make sure to change its orientation to match in-memory
    // orientation.
    if (getLangOpts().HLSL && CGM.getHLSLRuntime().NeedHLSLMartrixCastForStoreOp(TargetDecl, IRCallArgs)) {
      llvm::SmallVector<clang::QualType, 16> tyList;
      for (CallArgList::const_iterator I = CallArgs.begin(), E = CallArgs.end(); I != E; ++I) {
        tyList.emplace_back(I->Ty);
      }
      CGM.getHLSLRuntime().EmitHLSLMartrixCastForStoreOp(*this, IRCallArgs, tyList);
    }
    // HLSL changes end
    CS = Builder.CreateCall(Callee, IRCallArgs);
  } else {
    llvm::BasicBlock *Cont = createBasicBlock("invoke.cont");
    CS = Builder.CreateInvoke(Callee, Cont, InvokeDest, IRCallArgs);
    EmitBlock(Cont);
  }
  if (callOrInvoke)
    *callOrInvoke = CS.getInstruction();

  if (CurCodeDecl && CurCodeDecl->hasAttr<FlattenAttr>() &&
      !CS.hasFnAttr(llvm::Attribute::NoInline))
    Attrs =
        Attrs.addAttribute(getLLVMContext(), llvm::AttributeSet::FunctionIndex,
                           llvm::Attribute::AlwaysInline);

  // Disable inlining inside SEH __try blocks.
  if (isSEHTryScope())
    Attrs =
        Attrs.addAttribute(getLLVMContext(), llvm::AttributeSet::FunctionIndex,
                           llvm::Attribute::NoInline);

  CS.setAttributes(Attrs);
  CS.setCallingConv(static_cast<llvm::CallingConv::ID>(CallingConv));

  // In ObjC ARC mode with no ObjC ARC exception safety, tell the ARC
  // optimizer it can aggressively ignore unwind edges.
  if (CGM.getLangOpts().ObjCAutoRefCount)
    AddObjCARCExceptionMetadata(CS.getInstruction());

  // If the call doesn't return, finish the basic block and clear the
  // insertion point; this allows the rest of IRgen to discard
  // unreachable code.
  if (CS.doesNotReturn()) {
    if (UnusedReturnSize)
      EmitLifetimeEnd(llvm::ConstantInt::get(Int64Ty, UnusedReturnSize),
                      SRetPtr);

    Builder.CreateUnreachable();
    Builder.ClearInsertionPoint();

    // FIXME: For now, emit a dummy basic block because expr emitters in
    // generally are not ready to handle emitting expressions at unreachable
    // points.
    EnsureInsertPoint();

    // Return a reasonable RValue.
    return GetUndefRValue(RetTy);
  }

  llvm::Instruction *CI = CS.getInstruction();
  if (Builder.isNamePreserving() && !CI->getType()->isVoidTy())
    CI->setName("call");

#if 0 // HLSL Change - no ObjC support
  // Emit any writebacks immediately.  Arguably this should happen
  // after any return-value munging.
  if (CallArgs.hasWritebacks())
    emitWritebacks(*this, CallArgs);
#else
  assert(!CallArgs.hasWritebacks() && "writebacks are unavailable in HLSL");
#endif // HLSL Change - no ObjC support

  // The stack cleanup for inalloca arguments has to run out of the normal
  // lexical order, so deactivate it and run it manually here.
  CallArgs.freeArgumentMemory(*this);

  RValue Ret = [&] {
    switch (RetAI.getKind()) {
    case ABIArgInfo::InAlloca:
    case ABIArgInfo::Indirect: {
      RValue ret = convertTempToRValue(SRetPtr, RetTy, SourceLocation());
      if (UnusedReturnSize)
        EmitLifetimeEnd(llvm::ConstantInt::get(Int64Ty, UnusedReturnSize),
                        SRetPtr);
      return ret;
    }

    case ABIArgInfo::Ignore:
      // If we are ignoring an argument that had a result, make sure to
      // construct the appropriate return value for our caller.
      return GetUndefRValue(RetTy);

    case ABIArgInfo::Extend:
    case ABIArgInfo::Direct: {
      llvm::Type *RetIRTy = ConvertType(RetTy);
      if (RetAI.getCoerceToType() == RetIRTy && RetAI.getDirectOffset() == 0) {
        switch (getEvaluationKind(RetTy)) {
        case TEK_Complex: {
          llvm::Value *Real = Builder.CreateExtractValue(CI, 0);
          llvm::Value *Imag = Builder.CreateExtractValue(CI, 1);
          return RValue::getComplex(std::make_pair(Real, Imag));
        }
        case TEK_Aggregate: {
          llvm::Value *DestPtr = ReturnValue.getValue();
          bool DestIsVolatile = ReturnValue.isVolatile();
          CharUnits DestAlign = getContext().getTypeAlignInChars(RetTy);

          if (!DestPtr) {
            DestPtr = CreateMemTemp(RetTy, "agg.tmp");
            DestIsVolatile = false;
          }
          BuildAggStore(*this, CI, DestPtr, DestIsVolatile, DestAlign);
          return RValue::getAggregate(DestPtr);
        }
        case TEK_Scalar: {
          // If the argument doesn't match, perform a bitcast to coerce it.  This
          // can happen due to trivial type mismatches.
          llvm::Value *V = CI;
          if (V->getType() != RetIRTy)
            V = Builder.CreateBitCast(V, RetIRTy);
          return RValue::get(V);
        }
        }
        llvm_unreachable("bad evaluation kind");
      }

      llvm::Value *DestPtr = ReturnValue.getValue();
      bool DestIsVolatile = ReturnValue.isVolatile();
      CharUnits DestAlign = getContext().getTypeAlignInChars(RetTy);

      if (!DestPtr) {
        DestPtr = CreateMemTemp(RetTy, "coerce");
        DestIsVolatile = false;
      }

      // If the value is offset in memory, apply the offset now.
      llvm::Value *StorePtr = DestPtr;
      CharUnits StoreAlign = DestAlign;
      if (unsigned Offs = RetAI.getDirectOffset()) {
        StorePtr = Builder.CreateBitCast(StorePtr, Builder.getInt8PtrTy());
        StorePtr =
            Builder.CreateConstGEP1_32(Builder.getInt8Ty(), StorePtr, Offs);
        StorePtr = Builder.CreateBitCast(StorePtr,
                           llvm::PointerType::getUnqual(RetAI.getCoerceToType()));
        StoreAlign =
          StoreAlign.alignmentAtOffset(CharUnits::fromQuantity(Offs));
      }
      CreateCoercedStore(CI, StorePtr, DestIsVolatile, StoreAlign, *this);

      return convertTempToRValue(DestPtr, RetTy, SourceLocation());
    }

    case ABIArgInfo::Expand:
      llvm_unreachable("Invalid ABI kind for return argument");
    }

    llvm_unreachable("Unhandled ABIArgInfo::Kind");
  } ();

  if (Ret.isScalar() && TargetDecl) {
    if (const auto *AA = TargetDecl->getAttr<AssumeAlignedAttr>()) {
      llvm::Value *OffsetValue = nullptr;
      if (const auto *Offset = AA->getOffset())
        OffsetValue = EmitScalarExpr(Offset);

      llvm::Value *Alignment = EmitScalarExpr(AA->getAlignment());
      llvm::ConstantInt *AlignmentCI = cast<llvm::ConstantInt>(Alignment);
      EmitAlignmentAssumption(Ret.getScalarVal(), AlignmentCI->getZExtValue(),
                              OffsetValue);
    }
  }

  return Ret;
}

/* VarArg handling */

llvm::Value *CodeGenFunction::EmitVAArg(llvm::Value *VAListAddr, QualType Ty) {
  return CGM.getTypes().getABIInfo().EmitVAArg(VAListAddr, Ty, *this);
}
