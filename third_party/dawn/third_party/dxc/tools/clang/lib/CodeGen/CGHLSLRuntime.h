//===----- CGMSHLSLRuntime.h - Interface to HLSL Runtime ----------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// CGHLSLRuntime.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This provides a class for HLSL code generation.                          //
//
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <functional>
#include <llvm/ADT/SmallVector.h> // HLSL Change

namespace CGHLSLMSHelper {
struct Scope;
}

namespace llvm {
class Function;
class Value;
class Constant;
class TerminatorInst;
class GlobalVariable;
class Type;
class BasicBlock;
class BranchInst;
class SwitchInst;
template <typename T> class ArrayRef;
} // namespace llvm

namespace clang {
class Decl;
class QualType;
class ExtVectorType;
class ASTContext;
class FunctionDecl;
class CallExpr;
class InitListExpr;
class Expr;
class Stmt;
class ReturnStmt;
class Attr;
class VarDecl;
class HLSLRootSignatureAttr;

namespace CodeGen {
class CodeGenModule;
class ReturnValueSlot;
class CodeGenFunction;

class RValue;
class LValue;

class CGHLSLRuntime {
protected:
  CodeGenModule &CGM;
  llvm::SmallVector<llvm::BranchInst *, 16> m_DxBreaks;

public:
  CGHLSLRuntime(CodeGenModule &CGM) : CGM(CGM) {}
  virtual ~CGHLSLRuntime();

  virtual void addResource(Decl *D) = 0;
  virtual void addSubobject(Decl *D) = 0;
  virtual void FinishCodeGen() = 0;
  virtual RValue EmitHLSLBuiltinCallExpr(CodeGenFunction &CGF,
                                         const FunctionDecl *FD,
                                         const CallExpr *E,
                                         ReturnValueSlot ReturnValue) = 0;
  // Is E is a c++ init list not a hlsl init list which only match size.
  virtual bool IsTrivalInitListExpr(CodeGenFunction &CGF, InitListExpr *E) = 0;
  virtual llvm::Value *
  EmitHLSLInitListExpr(CodeGenFunction &CGF, InitListExpr *E,
                       // The destPtr when emiting aggregate init, for normal
                       // case, it will be null.
                       llvm::Value *DestPtr) = 0;
  virtual llvm::Constant *EmitHLSLConstInitListExpr(CodeGenModule &CGM,
                                                    InitListExpr *E) = 0;

  virtual void EmitHLSLOutParamConversionInit(
      CodeGenFunction &CGF, const FunctionDecl *FD, const CallExpr *E,
      llvm::SmallVector<LValue, 8> &castArgList,
      llvm::SmallVector<const Stmt *, 8> &argList,
      llvm::SmallVector<LValue, 8> &lifetimeCleanupList,
      const std::function<void(const VarDecl *, llvm::Value *)> &TmpArgMap) = 0;
  virtual void EmitHLSLOutParamConversionCopyBack(
      CodeGenFunction &CGF, llvm::SmallVector<LValue, 8> &castArgList,
      llvm::SmallVector<LValue, 8> &lifetimeCleanupList) = 0;
  virtual void MarkPotentialResourceTemp(CodeGenFunction &CGF, llvm::Value *V,
                                         clang::QualType QaulTy) = 0;
  virtual llvm::Value *
  EmitHLSLMatrixOperationCall(CodeGenFunction &CGF, const clang::Expr *E,
                              llvm::Type *RetType,
                              llvm::ArrayRef<llvm::Value *> paramList) = 0;
  virtual void EmitHLSLDiscard(CodeGenFunction &CGF) = 0;
  virtual llvm::BranchInst *EmitHLSLCondBreak(CodeGenFunction &CGF,
                                              llvm::Function *F,
                                              llvm::BasicBlock *DestBB,
                                              llvm::BasicBlock *AltBB) = 0;

  // For [] on matrix
  virtual llvm::Value *EmitHLSLMatrixSubscript(CodeGenFunction &CGF,
                                               llvm::Type *RetType,
                                               llvm::Value *Ptr,
                                               llvm::Value *Idx,
                                               clang::QualType Ty) = 0;
  // For ._m on matrix
  virtual llvm::Value *
  EmitHLSLMatrixElement(CodeGenFunction &CGF, llvm::Type *RetType,
                        llvm::ArrayRef<llvm::Value *> paramList,
                        clang::QualType Ty) = 0;

  virtual llvm::Value *EmitHLSLMatrixLoad(CodeGenFunction &CGF,
                                          llvm::Value *Ptr,
                                          clang::QualType Ty) = 0;
  virtual void EmitHLSLMatrixStore(CodeGenFunction &CGF, llvm::Value *Val,
                                   llvm::Value *DestPtr,
                                   clang::QualType Ty) = 0;
  virtual void EmitHLSLAggregateCopy(CodeGenFunction &CGF, llvm::Value *SrcPtr,
                                     llvm::Value *DestPtr,
                                     clang::QualType Ty) = 0;
  virtual void EmitHLSLFlatConversion(CodeGenFunction &CGF, llvm::Value *Val,
                                      llvm::Value *DestPtr, clang::QualType Ty,
                                      clang::QualType SrcTy) = 0;
  virtual void EmitHLSLFlatConversionAggregateCopy(CodeGenFunction &CGF,
                                                   llvm::Value *SrcPtr,
                                                   clang::QualType SrcTy,
                                                   llvm::Value *DestPtr,
                                                   clang::QualType DestTy) = 0;
  virtual llvm::Value *EmitHLSLLiteralCast(CodeGenFunction &CGF,
                                           llvm::Value *Src,
                                           clang::QualType SrcType,
                                           clang::QualType DstType) = 0;

  virtual void AddHLSLFunctionInfo(llvm::Function *,
                                   const FunctionDecl *FD) = 0;
  virtual void EmitHLSLFunctionProlog(llvm::Function *,
                                      const FunctionDecl *FD) = 0;

  virtual void AddControlFlowHint(CodeGenFunction &CGF, const Stmt &S,
                                  llvm::TerminatorInst *TI,
                                  llvm::ArrayRef<const Attr *> Attrs) = 0;

  virtual void FinishAutoVar(CodeGenFunction &CGF, const VarDecl &D,
                             llvm::Value *V) = 0;
  virtual const clang::Expr *CheckReturnStmtCoherenceMismatch(
      CodeGenFunction &CGF, const clang::Expr *RV, const clang::ReturnStmt &S,
      clang::QualType FnRetTy,
      const std::function<void(const VarDecl *, llvm::Value *)> &TmpArgMap) = 0;
  virtual void MarkIfStmt(CodeGenFunction &CGF, llvm::BasicBlock *endIfBB) = 0;
  virtual void MarkCleanupBlock(CodeGenFunction &CGF,
                                llvm::BasicBlock *cleanupBB) = 0;
  virtual void MarkSwitchStmt(CodeGenFunction &CGF,
                              llvm::SwitchInst *switchInst,
                              llvm::BasicBlock *endSwitch) = 0;
  virtual void MarkReturnStmt(CodeGenFunction &CGF,
                              llvm::BasicBlock *bbWithRet) = 0;
  virtual void MarkLoopStmt(CodeGenFunction &CGF,
                            llvm::BasicBlock *loopContinue,
                            llvm::BasicBlock *loopExit) = 0;

  virtual CGHLSLMSHelper::Scope *MarkScopeEnd(CodeGenFunction &CGF) = 0;

  virtual bool NeedHLSLMartrixCastForStoreOp(
      const clang::Decl *TD,
      llvm::SmallVector<llvm::Value *, 16> &IRCallArgs) = 0;

  virtual void EmitHLSLMartrixCastForStoreOp(
      CodeGenFunction &CGF, llvm::SmallVector<llvm::Value *, 16> &IRCallArgs,
      llvm::SmallVector<clang::QualType, 16> &ArgTys) = 0;
};

/// Create an instance of a HLSL runtime class.
CGHLSLRuntime *CreateMSHLSLRuntime(CodeGenModule &CGM);
} // namespace CodeGen
} // namespace clang
