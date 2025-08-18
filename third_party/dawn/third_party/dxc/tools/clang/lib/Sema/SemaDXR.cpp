//===------ SemaDXR.cpp - Semantic Analysis for DXR shader -----*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SemaDXR.cpp                                                               //
// Copyright (C) Nvidia Corporation. All rights reserved.                    //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file defines the semantic support for DXR.                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Sema/SemaHLSL.h"

#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/Analyses/ReachableCode.h"
#include "clang/Analysis/CFG.h"

#include "llvm/ADT/BitVector.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/HlslIntrinsicOp.h"

using namespace clang;
using namespace sema;
using namespace hlsl;

namespace {

struct PayloadUse {
  PayloadUse() = default;
  PayloadUse(const Stmt *S, const CFGBlock *Parent)
      : S(S), Parent(Parent), Member(nullptr) {}
  PayloadUse(const Stmt *S, const CFGBlock *Parent, const MemberExpr *Member)
      : S(S), Parent(Parent), Member(Member) {}

  bool operator<(const PayloadUse &Other) const { return S < Other.S; }

  const Stmt *S = nullptr;
  const CFGBlock *Parent = nullptr;
  const MemberExpr *Member = nullptr;
};

struct PayloadBuiltinCall {
  PayloadBuiltinCall() = default;
  PayloadBuiltinCall(const CallExpr *Call, const CFGBlock *Parent)
      : Call(Call), Parent(Parent) {}
  const CallExpr *Call = nullptr;
  const CFGBlock *Parent = nullptr;
};

struct PayloadAccessInfo {
  PayloadAccessInfo() = default;
  PayloadAccessInfo(const MemberExpr *Member, const CallExpr *Call,
                    bool IsLValue)
      : Member(Member), Call(Call), IsLValue(IsLValue) {}
  const MemberExpr *Member = nullptr;
  const CallExpr *Call = nullptr;
  bool IsLValue = false;
};

struct DxrShaderDiagnoseInfo {
  const FunctionDecl *funcDecl;
  const VarDecl *Payload;
  DXIL::PayloadAccessShaderStage Stage;
  std::vector<PayloadBuiltinCall> PayloadBuiltinCalls;
  std::map<const FieldDecl *, std::vector<PayloadUse>> WritesPerField;
  std::map<const FieldDecl *, std::vector<PayloadUse>> ReadsPerField;
  std::vector<PayloadUse> PayloadAsCallArg;
};

std::vector<const FieldDecl *>
DiagnosePayloadAccess(Sema &S, DxrShaderDiagnoseInfo &Info,
                      const std::set<const FieldDecl *> &FieldsToIgnoreRead,
                      const std::set<const FieldDecl *> &FieldsToIgnoreWrite,
                      std::set<const FunctionDecl *> VisitedFunctions);

const Stmt *IgnoreParensAndDecay(const Stmt *S);

// Transform the shader stage to string to be used in diagnostics
StringRef GetStringForShaderStage(DXIL::PayloadAccessShaderStage Stage) {
  StringRef StageNames[] = {"caller", "closesthit", "miss", "anyhit"};
  if (Stage != DXIL::PayloadAccessShaderStage::Invalid)
    return StageNames[static_cast<unsigned>(Stage)];
  return "";
}

// Returns the Qualifier for a Field and a given shader stage.
DXIL::PayloadAccessQualifier
GetPayloadQualifierForStage(FieldDecl *Field,
                            DXIL::PayloadAccessShaderStage Stage) {
  bool hasRead = false;
  bool hasWrite = false;
  for (UnusualAnnotation *annotation : Field->getUnusualAnnotations()) {
    if (auto *payloadAnnotation =
            dyn_cast<hlsl::PayloadAccessAnnotation>(annotation)) {
      for (auto &ShaderStage : payloadAnnotation->ShaderStages) {
        if (ShaderStage != Stage)
          continue;
        hasRead |=
            payloadAnnotation->qualifier == DXIL::PayloadAccessQualifier::Read;
        hasWrite |=
            payloadAnnotation->qualifier == DXIL::PayloadAccessQualifier::Write;
      }
    }
  }
  if (hasRead && hasWrite)
    return DXIL::PayloadAccessQualifier::ReadWrite;
  if (hasRead)
    return DXIL::PayloadAccessQualifier::Read;
  if (hasWrite)
    return DXIL::PayloadAccessQualifier::Write;
  return DXIL::PayloadAccessQualifier::NoAccess;
}

static int GetPayloadParamIdxForIntrinsic(const FunctionDecl *FD) {
  HLSLIntrinsicAttr *IntrinAttr = FD->getAttr<HLSLIntrinsicAttr>();
  if (!IntrinAttr)
    return -1;
  switch ((IntrinsicOp)IntrinAttr->getOpcode()) {
  default:
    return -1;
  case IntrinsicOp::IOP_TraceRay:
  case IntrinsicOp::MOP_DxHitObject_TraceRay:
  case IntrinsicOp::MOP_DxHitObject_Invoke:
    return FD->getNumParams() - 1;
  }
}

static bool IsBuiltinWithPayload(const FunctionDecl *FD) {
  return GetPayloadParamIdxForIntrinsic(FD) >= 0;
}

// Returns the declaration of the payload used in a call to TraceRay,
// HitObject::TraceRay or HitObject::Invoke.
const VarDecl *GetPayloadParameterForBuiltinCall(const CallExpr *Call) {
  const Decl *Callee = Call->getCalleeDecl();
  if (!Callee)
    return nullptr;

  if (!isa<FunctionDecl>(Callee))
    return nullptr;

  int PldParamIdx = GetPayloadParamIdxForIntrinsic(cast<FunctionDecl>(Callee));
  if (PldParamIdx < 0)
    return nullptr;

  const Stmt *Param = IgnoreParensAndDecay(Call->getArg(PldParamIdx));
  if (const DeclRefExpr *ParamRef = dyn_cast<DeclRefExpr>(Param))
    if (const VarDecl *Decl = dyn_cast<VarDecl>(ParamRef->getDecl()))
      return Decl;
  return nullptr;
}

// Recursively extracts accesses to a payload struct from a Stmt
void GetPayloadAccesses(const Stmt *S, const DxrShaderDiagnoseInfo &Info,
                        std::vector<PayloadAccessInfo> &Accesses, bool IsLValue,
                        const MemberExpr *Member, const CallExpr *Call) {
  for (auto C : S->children()) {
    if (!C)
      continue;
    if (const DeclRefExpr *Ref = dyn_cast<DeclRefExpr>(C)) {
      if (Ref->getDecl() == Info.Payload) {
        Accesses.push_back(PayloadAccessInfo{Member, Call, IsLValue});
      }
    }
    if (const ImplicitCastExpr *Cast = dyn_cast<ImplicitCastExpr>(C)) {
      if (Cast->getCastKind() == CK_LValueToRValue) {
        IsLValue = false;
      }
    }

    GetPayloadAccesses(C, Info, Accesses, IsLValue,
                       Member ? Member : dyn_cast<MemberExpr>(C),
                       Call ? Call : dyn_cast<CallExpr>(C));
  }
}

// Collects all reads, writes and calls with participation of the payload.
void CollectReadsWritesAndCallsForPayload(const Stmt *S,
                                          DxrShaderDiagnoseInfo &Info,
                                          const CFGBlock *Block) {
  std::vector<PayloadAccessInfo> PayloadAccesses;
  GetPayloadAccesses(S, Info, PayloadAccesses, true, dyn_cast<MemberExpr>(S),
                     dyn_cast<CallExpr>(S));
  for (auto &Access : PayloadAccesses) {
    // An access to a payload member was found.
    if (Access.Member) {
      FieldDecl *Field = cast<FieldDecl>(Access.Member->getMemberDecl());
      if (Access.IsLValue) {
        Info.WritesPerField[Field].push_back(
            PayloadUse{S, Block, Access.Member});
      } else {
        Info.ReadsPerField[Field].push_back(
            PayloadUse{S, Block, Access.Member});
      }
    } else if (Access.Call) {
      Info.PayloadAsCallArg.push_back(PayloadUse{S, Block});
    }
  }
}

// Collects all calls to TraceRay, HitObject::TraceRay and HitObject::Invoke.
void CollectBuiltinCallsWithPayload(const Stmt *S, DxrShaderDiagnoseInfo &Info,
                                    const CFGBlock *Block) {
  if (const CallExpr *Call = dyn_cast<CallExpr>(S)) {

    const Decl *Callee = Call->getCalleeDecl();
    if (!Callee || !isa<FunctionDecl>(Callee))
      return;

    const FunctionDecl *CalledFunction = cast<FunctionDecl>(Callee);

    if (IsBuiltinWithPayload(CalledFunction))
      Info.PayloadBuiltinCalls.push_back({Call, Block});
  }
}

// Find the last write to the payload field in the given block.
PayloadUse GetLastWriteInBlock(CFGBlock &Block,
                               ArrayRef<PayloadUse> PayloadWrites) {
  PayloadUse LastWrite;
  for (auto &Element : Block) { // TODO: reverse iterate?
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      auto It = std::find_if(
          PayloadWrites.begin(), PayloadWrites.end(),
          [&](const PayloadUse &V) { return V.S == S->getStmt(); });
      if (It != std::end(PayloadWrites)) {
        LastWrite = *It;
        LastWrite.Parent = &Block;
      }
    }
  }
  return LastWrite;
}

// Travers the CFG until every path has reached a write or the ENTRY.
void TraverseCFGUntilWrite(CFGBlock &Current, std::vector<PayloadUse> &Writes,
                           ArrayRef<PayloadUse> PayloadWrites,
                           std::set<const CFGBlock *> &Visited) {

  if (Visited.count(&Current))
    return;
  Visited.insert(&Current);

  for (auto I = Current.pred_begin(), E = Current.pred_end(); I != E; ++I) {
    CFGBlock *Pred = *I;
    if (!Pred)
      continue;
    PayloadUse WriteInPred = GetLastWriteInBlock(*Pred, PayloadWrites);
    if (!WriteInPred.S)
      TraverseCFGUntilWrite(*Pred, Writes, PayloadWrites, Visited);
    else
      Writes.push_back(WriteInPred);
  }
}

// Traverse the CFG from the EXIT backwards and stop as soon as a block has a
// write to the payload field.
std::vector<PayloadUse>
GetAllWritesReachingExit(CFG &ShaderCFG, ArrayRef<PayloadUse> PayloadWrites) {

  std::vector<PayloadUse> Writes;
  CFGBlock &Exit = ShaderCFG.getExit();

  std::set<const CFGBlock *> Visited;
  TraverseCFGUntilWrite(Exit, Writes, PayloadWrites, Visited);

  return Writes;
}

// Find the first read to the payload field in the given block.
PayloadUse GetFirstReadInBlock(CFGBlock &Block,
                               ArrayRef<PayloadUse> PayloadReads) {
  PayloadUse FirstRead;
  for (auto &Element : Block) {
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      auto It = std::find_if(
          PayloadReads.begin(), PayloadReads.end(),
          [&](const PayloadUse &V) { return V.S == S->getStmt(); });
      if (It != std::end(PayloadReads)) {
        FirstRead = *It;
        FirstRead.Parent = &Block;
        break; // We found the first read and are done with this block.
      }
    }
  }
  return FirstRead;
}

// Travers the CFG until every path has reached a read or the EXIT.
void TraverseCFGUntilRead(CFGBlock &Current, std::vector<PayloadUse> &Reads,
                          ArrayRef<PayloadUse> PayloadWrites,
                          std::set<const CFGBlock *> &Visited) {

  if (Visited.count(&Current))
    return;
  Visited.insert(&Current);

  for (auto I = Current.succ_begin(), E = Current.succ_end(); I != E; ++I) {
    CFGBlock *Succ = *I;
    if (!Succ)
      continue;
    PayloadUse ReadInSucc = GetFirstReadInBlock(*Succ, PayloadWrites);
    if (!ReadInSucc.S)
      TraverseCFGUntilRead(*Succ, Reads, PayloadWrites, Visited);
    else
      Reads.push_back(ReadInSucc);
  }
}

// Traverse the CFG from the ENTRY down and stop as soon as a block has a read
// to the payload field.
std::vector<PayloadUse>
GetAllReadsReachedFromEntry(CFG &ShaderCFG, ArrayRef<PayloadUse> PayloadReads) {
  std::vector<PayloadUse> Reads;
  CFGBlock &Entry = ShaderCFG.getEntry();

  std::set<const CFGBlock *> Visited;
  TraverseCFGUntilRead(Entry, Reads, PayloadReads, Visited);

  return Reads;
}

// Returns the record type of a payload declaration.
CXXRecordDecl *GetPayloadType(const VarDecl *Payload) {
  auto PayloadType = Payload->getType();
  if (PayloadType->isStructureOrClassType()) {
    return PayloadType->getAsCXXRecordDecl();
  }
  return nullptr;
}

std::vector<FieldDecl *> GetAllPayloadFields(RecordDecl *PayloadType) {
  std::vector<FieldDecl *> PayloadFields;

  for (FieldDecl *Field : PayloadType->fields()) {
    QualType FieldType = Field->getType();
    if (RecordDecl *FieldRecordDecl = FieldType->getAsCXXRecordDecl()) {
      // Skip nested payload types.
      if (FieldRecordDecl->hasAttr<HLSLRayPayloadAttr>()) {
        auto SubTypeFields = GetAllPayloadFields(FieldRecordDecl);
        PayloadFields.insert(PayloadFields.end(), SubTypeFields.begin(),
                             SubTypeFields.end());
        continue;
      }
    }
    PayloadFields.push_back(Field);
  }

  return PayloadFields;
}

// Returns true if the field is writeable in an earlier shader stage.
bool IsFieldWriteableInEarlierStage(FieldDecl *Field,
                                    DXIL::PayloadAccessShaderStage ThisStage) {
  bool isWriteableInEarlierStage = false;
  switch (ThisStage) {
  case DXIL::PayloadAccessShaderStage::Anyhit:
  case DXIL::PayloadAccessShaderStage::Closesthit:
  case DXIL::PayloadAccessShaderStage::Miss: {
    auto Qualifier = GetPayloadQualifierForStage(
        Field, DXIL::PayloadAccessShaderStage::Caller);
    isWriteableInEarlierStage =
        Qualifier == DXIL::PayloadAccessQualifier::Write ||
        Qualifier == DXIL::PayloadAccessQualifier::ReadWrite;
    Qualifier = GetPayloadQualifierForStage(
        Field, DXIL::PayloadAccessShaderStage::Anyhit);
    isWriteableInEarlierStage |=
        Qualifier == DXIL::PayloadAccessQualifier::Write ||
        Qualifier == DXIL::PayloadAccessQualifier::ReadWrite;
  } break;
  default:
    break;
  }
  return isWriteableInEarlierStage;
}

// Emit warnings on payload writes.
void DiagnosePayloadWrites(Sema &S, CFG &ShaderCFG, DominatorTree &DT,
                           const DxrShaderDiagnoseInfo &Info,
                           ArrayRef<FieldDecl *> NonWriteableFields,
                           RecordDecl *PayloadType) {
  for (FieldDecl *Field : NonWriteableFields) {
    auto WritesToField = Info.WritesPerField.find(Field);
    if (WritesToField == Info.WritesPerField.end())
      continue;

    const auto &WritesToDiagnose =
        GetAllWritesReachingExit(ShaderCFG, WritesToField->second);
    for (auto &Write : WritesToDiagnose) {
      FieldDecl *MemField = cast<FieldDecl>(Write.Member->getMemberDecl());
      auto Qualifier = GetPayloadQualifierForStage(MemField, Info.Stage);
      if (Qualifier != DXIL::PayloadAccessQualifier::Write &&
          Qualifier != DXIL::PayloadAccessQualifier::ReadWrite) {
        S.Diag(Write.Member->getExprLoc(),
               diag::warn_hlsl_payload_access_write_loss)
            << Field->getName() << GetStringForShaderStage(Info.Stage);
      }
    }
  }

  // Check if a field is not unconditionally written and a write form an earlier
  // stage will be lost.
  auto PayloadFields = GetAllPayloadFields(PayloadType);
  for (FieldDecl *Field : PayloadFields) {
    auto Qualifier = GetPayloadQualifierForStage(Field, Info.Stage);
    if (IsFieldWriteableInEarlierStage(Field, Info.Stage) &&
        Qualifier == DXIL::PayloadAccessQualifier::Write) {

      // The field is writeable in an earlier stage and pure write in this
      // stage. Check if we find a write that dominates the exit of the
      // function.
      bool fieldHasDominatingWrite = false;
      auto It = Info.WritesPerField.find(Field);
      if (It != Info.WritesPerField.end()) {
        for (auto &Write : It->second) {
          fieldHasDominatingWrite =
              DT.dominates(Write.Parent, &ShaderCFG.getExit());
          if (fieldHasDominatingWrite)
            break;
        }
      }

      if (!fieldHasDominatingWrite) {
        S.Diag(Info.Payload->getLocation(),
               diag::warn_hlsl_payload_access_data_loss)
            << Field->getName() << GetStringForShaderStage(Info.Stage);
      }
    }
  }
}

// Returns true if A is earlier than B in Parent
bool IsEarlierStatementAs(const Stmt *A, const Stmt *B,
                          const CFGBlock &Parent) {
  for (auto Element : Parent) {
    if (auto S = Element.getAs<CFGStmt>()) {
      if (S->getStmt() == A)
        return true;
      if (S->getStmt() == B)
        return false;
    }
  }
  return true;
}

// Returns true if the write dominates payload use.
template <typename T>
bool WriteDominatesUse(const PayloadUse &Write, const T &Use,
                       DominatorTree &DT) {
  if (Use.Parent == Write.Parent) {
    // Use and write are in the same Block.
    return IsEarlierStatementAs(Write.S, Use.S, *Use.Parent);
  }

  return DT.dominates(Write.Parent, Use.Parent);
}

// Emit warnings for payload reads.
void DiagnosePayloadReads(Sema &S, CFG &ShaderCFG, DominatorTree &DT,
                          const DxrShaderDiagnoseInfo &Info,
                          ArrayRef<FieldDecl *> NonReadableFields) {
  for (FieldDecl *Field : NonReadableFields) {
    auto ReadsFromField = Info.ReadsPerField.find(Field);
    if (ReadsFromField == Info.ReadsPerField.end())
      continue;

    auto WritesToField = Info.WritesPerField.find(Field);
    bool FieldHasWrites = WritesToField != Info.WritesPerField.end();

    const auto &ReadsToDiagnose =
        GetAllReadsReachedFromEntry(ShaderCFG, ReadsFromField->second);

    for (auto &Read : ReadsToDiagnose) {
      bool ReadIsDominatedByWrite = false;
      if (FieldHasWrites) {
        // We found a read to a field that needs diagnose.
        // We do not want to warn about fields that read but are dominated by a
        // write. Find writes that dominate the read. If we found one, ignore
        // the read.
        for (auto Write : WritesToField->second) {
          ReadIsDominatedByWrite = WriteDominatesUse(Write, Read, DT);
          if (ReadIsDominatedByWrite)
            break;
        }
      }

      if (ReadIsDominatedByWrite)
        continue;

      FieldDecl *MemField = cast<FieldDecl>(Read.Member->getMemberDecl());

      auto Qualifier = GetPayloadQualifierForStage(MemField, Info.Stage);
      if (Qualifier != DXIL::PayloadAccessQualifier::Read &&
          Qualifier != DXIL::PayloadAccessQualifier::ReadWrite) {
        S.Diag(Read.Member->getExprLoc(),
               diag::warn_hlsl_payload_access_undef_read)
            << Field->getName() << GetStringForShaderStage(Info.Stage);
      }
    }
  }
}

// Generic CFG traversal that performs PerElementAction on every Stmt in the
// CFG.
template <bool Backward, typename Action>
void TraverseCFG(const CFGBlock &Block, Action PerElementAction,
                 std::set<const CFGBlock *> &Visited) {
  if (Visited.count(&Block))
    return;
  Visited.insert(&Block);

  for (const auto &Element : Block) {
    PerElementAction(Block, Element);
  }

  if (!Backward) {
    for (auto I = Block.succ_begin(), E = Block.succ_end(); I != E; ++I) {
      CFGBlock *Succ = *I;
      if (!Succ)
        continue;
      TraverseCFG</*Backward=*/false>(*Succ, PerElementAction, Visited);
    }
  } else {
    for (auto I = Block.pred_begin(), E = Block.pred_end(); I != E; ++I) {
      CFGBlock *Pred = *I;
      if (!Pred)
        continue;
      TraverseCFG<Backward>(*Pred, PerElementAction, Visited);
    }
  }
}

// Forward traverse the CFG and collect calls to TraceRay, HitObject::TraceRay
// and HitObject::Invoke.
void ForwardTraverseCFGAndCollectBuiltinCallsWithPayload(
    const CFGBlock &Block, DxrShaderDiagnoseInfo &Info,
    std::set<const CFGBlock *> &Visited) {
  auto Action = [&Info](const CFGBlock &Block, const CFGElement &Element) {
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      CollectBuiltinCallsWithPayload(S->getStmt(), Info, &Block);
    }
  };

  TraverseCFG<false>(Block, Action, Visited);
}

// Foward traverse the CFG and collect all reads and writes to the payload.
void ForwardTraverseCFGAndCollectReadsWrites(
    const CFGBlock &StartBlock, DxrShaderDiagnoseInfo &Info,
    std::set<const CFGBlock *> &Visited) {
  auto Action = [&Info](const CFGBlock &Block, const CFGElement &Element) {
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      CollectReadsWritesAndCallsForPayload(S->getStmt(), Info, &Block);
    }
  };

  TraverseCFG<false>(StartBlock, Action, Visited);
}

// Backward traverse the CFG and collect all reads and writes to the payload.
void BackwardTraverseCFGAndCollectReadsWrites(
    const CFGBlock &StartBlock, DxrShaderDiagnoseInfo &Info,
    std::set<const CFGBlock *> &Visited) {
  auto Action = [&](const CFGBlock &Block, const CFGElement &Element) {
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      CollectReadsWritesAndCallsForPayload(S->getStmt(), Info, &Block);
    }
  };

  TraverseCFG<true>(StartBlock, Action, Visited);
}

// Returns true if the Stmt uses the Payload.
bool IsPayloadArg(const Stmt *S, const Decl *Payload) {
  if (const DeclRefExpr *Ref = dyn_cast<DeclRefExpr>(S)) {
    const Decl *Decl = Ref->getDecl();
    if (Decl == Payload)
      return true;
  }

  for (auto C : S->children()) {
    if (IsPayloadArg(C, Payload))
      return true;
  }
  return false;
}

bool DiagnoseCallExprForExternal(Sema &S, const FunctionDecl *FD,
                                 const CallExpr *CE,
                                 const ParmVarDecl *Payload);

// Collects all writes that dominate a PayloadUse in a CallExpr
// and returns a set of the Fields accessed.
std::set<const FieldDecl *>
CollectDominatingWritesForCall(PayloadUse &Use, DxrShaderDiagnoseInfo &Info,
                               DominatorTree &DT) {
  std::set<const FieldDecl *> FieldsToIgnore;

  for (auto P : Info.WritesPerField) {
    for (auto Write : P.second) {
      bool WriteDominatesCallSite = WriteDominatesUse(Write, Use, DT);
      if (WriteDominatesCallSite) {
        FieldsToIgnore.insert(P.first);
        break;
      }
    }
  }

  return FieldsToIgnore;
}

// Collects all reads that are reachable from a PayloadUse in a CallExpr
// and returns a set of the Fields accessed.
std::set<const FieldDecl *>
CollectReachableWritesForCall(PayloadUse &Use,
                              const DxrShaderDiagnoseInfo &Info) {
  std::set<const FieldDecl *> FieldsToIgnore;
  assert(Use.Parent);
  const CFGBlock *Current = Use.Parent;

  // Traverse the CFG beginning from the block of the call and collect all
  // fields written to after the call. These fields must not be diagnosed with
  // warnings about lost writes.
  DxrShaderDiagnoseInfo TempInfo;
  TempInfo.Payload = Info.Payload;
  bool foundCall = false;
  for (auto &Element : *Current) {
    // Search for the Call in the block
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      if (S->getStmt() == Use.S) {
        foundCall = true;
        continue;
      }
      if (foundCall)
        CollectReadsWritesAndCallsForPayload(S->getStmt(), TempInfo, Current);
    }
  }

  for (auto I = Current->succ_begin(); I != Current->succ_end(); ++I) {
    CFGBlock *Succ = *I;
    if (!Succ)
      continue;
    std::set<const CFGBlock *> Visited;
    ForwardTraverseCFGAndCollectReadsWrites(*Succ, TempInfo, Visited);
  }
  for (auto &p : TempInfo.WritesPerField)
    FieldsToIgnore.insert(p.first);

  return FieldsToIgnore;
}

// Emit diagnostics when the payload is used as an argument
// in a function call.
std::map<PayloadUse, std::vector<const FieldDecl *>>
DiagnosePayloadAsFunctionArg(
    Sema &S, DxrShaderDiagnoseInfo &Info, DominatorTree &DT,
    const std::set<const FieldDecl *> &ParentFieldsToIgnoreRead,
    const std::set<const FieldDecl *> &ParentFieldsToIgnoreWrite,
    std::set<const FunctionDecl *> VisitedFunctions) {
  std::map<PayloadUse, std::vector<const FieldDecl *>> WrittenFieldsInCalls;
  for (PayloadUse &Use : Info.PayloadAsCallArg) {
    if (const CallExpr *Call = dyn_cast<CallExpr>(Use.S)) {
      const Decl *Callee = Call->getCalleeDecl();
      if (!Callee || !isa<FunctionDecl>(Callee))
        continue;

      const FunctionDecl *CalledFunction = cast<FunctionDecl>(Callee);

      // Ignore trace calls here.
      if (IsBuiltinWithPayload(CalledFunction)) {
        Info.PayloadBuiltinCalls.push_back(
            PayloadBuiltinCall{Call, Use.Parent});
        continue;
      }

      // Handle external function calls
      if (!CalledFunction->hasBody()) {
        assert(isa<ParmVarDecl>(Info.Payload));
        DiagnoseCallExprForExternal(S, CalledFunction, Call,
                                    cast<ParmVarDecl>(Info.Payload));
        continue;
      }

      if (VisitedFunctions.count(CalledFunction))
        return WrittenFieldsInCalls;
      VisitedFunctions.insert(CalledFunction);

      DxrShaderDiagnoseInfo CalleeInfo;

      for (unsigned i = 0; i < Call->getNumArgs(); ++i) {
        const Expr *Arg = Call->getArg(i);
        if (IsPayloadArg(Arg, Info.Payload)) {
          CalleeInfo.Payload = CalledFunction->getParamDecl(i);
          break;
        }
      }

      if (CalleeInfo.Payload) {
        CalleeInfo.funcDecl = CalledFunction;
        CalleeInfo.Stage = Info.Stage;
        auto FieldsToIgnoreRead = CollectDominatingWritesForCall(Use, Info, DT);
        auto FieldsToIgnoreWrite = CollectReachableWritesForCall(Use, Info);
        FieldsToIgnoreRead.insert(ParentFieldsToIgnoreRead.begin(),
                                  ParentFieldsToIgnoreRead.end());
        FieldsToIgnoreWrite.insert(ParentFieldsToIgnoreWrite.begin(),
                                   ParentFieldsToIgnoreWrite.end());
        WrittenFieldsInCalls[Use] =
            DiagnosePayloadAccess(S, CalleeInfo, FieldsToIgnoreRead,
                                  FieldsToIgnoreWrite, VisitedFunctions);
      }
    }
  }
  return WrittenFieldsInCalls;
}

// Collect all fields that cannot be accessed for the given shader stage.
// This function recurses into nested payload types.
void CollectNonAccessableFields(
    RecordDecl *PayloadType, DXIL::PayloadAccessShaderStage Stage,
    const std::set<const FieldDecl *> &FieldsToIgnoreRead,
    const std::set<const FieldDecl *> &FieldsToIgnoreWrite,
    std::vector<FieldDecl *> &NonWriteableFields,
    std::vector<FieldDecl *> &NonReadableFields) {
  for (FieldDecl *Field : PayloadType->fields()) {
    QualType FieldType = Field->getType();
    if (RecordDecl *FieldRecordDecl = FieldType->getAsCXXRecordDecl()) {
      if (FieldRecordDecl->hasAttr<HLSLRayPayloadAttr>()) {
        CollectNonAccessableFields(FieldRecordDecl, Stage, FieldsToIgnoreRead,
                                   FieldsToIgnoreWrite, NonWriteableFields,
                                   NonReadableFields);
        continue;
      }
    }

    auto Qualifier = GetPayloadQualifierForStage(Field, Stage);
    // Diagnose writes only if they are not written heigher in the call-graph.
    if (!FieldsToIgnoreWrite.count(Field)) {
      if (Qualifier != DXIL::PayloadAccessQualifier::Write &&
          Qualifier != DXIL::PayloadAccessQualifier::ReadWrite)
        NonWriteableFields.push_back(Field);
    }
    // Diagnose reads only if they have no write heigher in the call-graph.
    if (!FieldsToIgnoreRead.count(Field)) {
      if (Qualifier != DXIL::PayloadAccessQualifier::Read &&
          Qualifier != DXIL::PayloadAccessQualifier::ReadWrite)
        NonReadableFields.push_back(Field);
    }
  }
}

void CollectAccessableFields(RecordDecl *PayloadType,
                             const std::vector<FieldDecl *> &NonWriteableFields,
                             const std::vector<FieldDecl *> &NonReadableFields,
                             std::vector<FieldDecl *> &WriteableFields,
                             std::vector<FieldDecl *> &ReadableFields) {
  for (FieldDecl *Field : PayloadType->fields()) {
    QualType FieldType = Field->getType();
    if (RecordDecl *FieldRecordDecl = FieldType->getAsCXXRecordDecl()) {
      // Skip nested payload types.
      if (FieldRecordDecl->hasAttr<HLSLRayPayloadAttr>()) {
        CollectAccessableFields(FieldRecordDecl, NonWriteableFields,
                                NonReadableFields, WriteableFields,
                                ReadableFields);
        continue;
      }
    }

    if (std::find(NonWriteableFields.begin(), NonWriteableFields.end(),
                  Field) == NonWriteableFields.end())
      WriteableFields.push_back(Field);

    if (std::find(NonReadableFields.begin(), NonReadableFields.end(), Field) ==
        NonReadableFields.end())
      ReadableFields.push_back(Field);
  }
}

void HandlePayloadInitializer(DxrShaderDiagnoseInfo &Info) {
  const VarDecl *Payload = Info.Payload;

  const Expr *Init = Payload->getInit();

  if (Init) {
    // If the payload has an initializer, then handle all fields as
    // written. Sema will check that the initializer is correct.
    // We can handle all fields as written.

    RecordDecl *PayloadType = GetPayloadType(Info.Payload);
    for (FieldDecl *Field : PayloadType->fields()) {
      Info.WritesPerField[Field].push_back(PayloadUse{Init, nullptr, nullptr});
    }
  }
}

// Emit diagnostics for this call to either TraceRay, HitObject::TraceRay or
// HitObject::Invoke.
void DiagnoseBuiltinCallWithPayload(Sema &S, const VarDecl *Payload,
                                    const PayloadBuiltinCall &PldCall,
                                    DominatorTree &DT) {
  // For each call check if write(caller) fields are written.
  const DXIL::PayloadAccessShaderStage CallerStage =
      DXIL::PayloadAccessShaderStage::Caller;

  std::vector<FieldDecl *> WriteableFields;
  std::vector<FieldDecl *> NonWriteableFields;
  std::vector<FieldDecl *> ReadableFields;
  std::vector<FieldDecl *> NonReadableFields;

  RecordDecl *PayloadType = GetPayloadType(Payload);

  // Check if the payload type used for this trace call is a payload type
  if (!PayloadType->hasAttr<HLSLRayPayloadAttr>()) {
    S.Diag(Payload->getLocation(), diag::err_payload_requires_attribute)
        << PayloadType->getName();
    return;
  }

  // Verify that the payload type is legal
  if (!hlsl::IsHLSLCopyableAnnotatableRecord(Payload->getType()))
    S.Diag(Payload->getLocation(), diag::err_payload_attrs_must_be_udt)
        << /*payload|attributes|callable*/ 0 << /*parameter %2|type*/ 0
        << Payload;

  // This will produce more details, but also catch disallowed long vectors
  const TypeDiagContext DiagContext = TypeDiagContext::PayloadParameters;
  if (DiagnoseTypeElements(S, Payload->getLocation(), Payload->getType(),
                           DiagContext, DiagContext))
    return;

  CollectNonAccessableFields(PayloadType, CallerStage, {}, {},
                             NonWriteableFields, NonReadableFields);

  CollectAccessableFields(PayloadType, NonWriteableFields, NonReadableFields,
                          WriteableFields, ReadableFields);

  // Find all writes to Payload that reaches the Trace
  DxrShaderDiagnoseInfo TraceInfo;
  TraceInfo.Payload = Payload;

  // Handle initializers for the payload struct if any is present.
  HandlePayloadInitializer(TraceInfo);

  std::set<const CFGBlock *> Visited;

  const CFGBlock *Parent = PldCall.Parent;
  Visited.insert(Parent);
  // Collect payload accesses in the same block until we reach the call
  for (auto Element : *Parent) {
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      if (S->getStmt() == PldCall.Call)
        break;
      CollectReadsWritesAndCallsForPayload(S->getStmt(), TraceInfo, Parent);
    }
  }

  for (auto I = Parent->pred_begin(); I != Parent->pred_end(); ++I) {
    CFGBlock *Pred = *I;
    if (!Pred)
      continue;
    BackwardTraverseCFGAndCollectReadsWrites(*Pred, TraceInfo, Visited);
  }

  int PldArgIdx = PldCall.Call->getNumArgs() - 1;

  // Warn if a writeable field has not been written.
  for (const FieldDecl *Field : WriteableFields) {
    if (!TraceInfo.WritesPerField.count(Field)) {
      S.Diag(PldCall.Call->getArg(PldArgIdx)->getExprLoc(),
             diag::warn_hlsl_payload_access_no_write_for_trace_payload)
          << Field->getName();
    }
  }
  // Warn if a written field is not write(caller)
  for (const FieldDecl *Field : NonWriteableFields) {
    if (TraceInfo.WritesPerField.count(Field)) {
      S.Diag(
          PldCall.Call->getArg(PldArgIdx)->getExprLoc(),
          diag::warn_hlsl_payload_access_write_but_no_write_for_trace_payload)
          << Field->getName();
    }
  }

  // After a trace call, collect all reads that are not dominated by another
  // write warn if a field is not read(caller) but the value is read (undef
  // read).

  // Discard reads/writes from backward traversal.
  TraceInfo.ReadsPerField.clear();
  TraceInfo.WritesPerField.clear();
  bool CallFound = false;
  for (auto Element : *Parent) { // TODO: reverse iterate?
    if (Optional<CFGStmt> S = Element.getAs<CFGStmt>()) {
      if (S->getStmt() == PldCall.Call) {
        CallFound = true;
        continue;
      }
      if (CallFound)
        CollectReadsWritesAndCallsForPayload(S->getStmt(), TraceInfo, Parent);
    }
  }
  for (auto I = Parent->succ_begin(); I != Parent->succ_end(); ++I) {
    CFGBlock *Pred = *I;
    if (!Pred)
      continue;
    ForwardTraverseCFGAndCollectReadsWrites(*Pred, TraceInfo, Visited);
  }

  for (const FieldDecl *Field : ReadableFields) {
    if (!TraceInfo.ReadsPerField.count(Field)) {
      S.Diag(PldCall.Call->getArg(PldArgIdx)->getExprLoc(),
             diag::warn_hlsl_payload_access_read_but_no_read_after_trace)
          << Field->getName();
    }
  }

  for (const FieldDecl *Field : NonReadableFields) {
    auto WritesToField = TraceInfo.WritesPerField.find(Field);
    bool FieldHasWrites = WritesToField != TraceInfo.WritesPerField.end();
    for (auto &Read : TraceInfo.ReadsPerField[Field]) {
      bool ReadIsDominatedByWrite = false;
      if (FieldHasWrites) {
        // We found a read to a field that needs diagnose.
        // We do not want to warn about fields that read but are dominated by
        // a write. Find writes that dominate the read. If we found one,
        // ignore the read.
        for (auto Write : WritesToField->second) {
          ReadIsDominatedByWrite = WriteDominatesUse(Write, Read, DT);
          if (ReadIsDominatedByWrite)
            break;
        }
      }

      if (ReadIsDominatedByWrite)
        continue;

      S.Diag(Read.Member->getExprLoc(),
             diag::warn_hlsl_payload_access_read_of_undef_after_trace)
          << Field->getName();
    }
  }
}

// Emit diagnostics for all calls to TraceRay, HitObject::TraceRay or
// HitObject::Invoke.
void DiagnoseBuiltinCallsWithPayload(Sema &S, CFG &ShaderCFG, DominatorTree &DT,
                                     DxrShaderDiagnoseInfo &Info) {
  // Collect calls with payload in the shader.
  std::set<const CFGBlock *> Visited;
  ForwardTraverseCFGAndCollectBuiltinCallsWithPayload(ShaderCFG.getEntry(),
                                                      Info, Visited);

  std::set<const CallExpr *> Diagnosed;

  for (const PayloadBuiltinCall &PldCall : Info.PayloadBuiltinCalls) {
    if (Diagnosed.count(PldCall.Call))
      continue;
    Diagnosed.insert(PldCall.Call);

    const VarDecl *Payload = GetPayloadParameterForBuiltinCall(PldCall.Call);
    DiagnoseBuiltinCallWithPayload(S, Payload, PldCall, DT);
  }
}

// Emit diagnostics for all access to the payload of a shader,
// and the input to TraceRay, HitObject::TraceRay or HitObject::Invoke calls.
std::vector<const FieldDecl *>
DiagnosePayloadAccess(Sema &S, DxrShaderDiagnoseInfo &Info,
                      const std::set<const FieldDecl *> &FieldsToIgnoreRead,
                      const std::set<const FieldDecl *> &FieldsToIgnoreWrite,
                      std::set<const FunctionDecl *> VisitedFunctions) {
  clang::DominatorTree DT;
  AnalysisDeclContextManager AnalysisManager;
  AnalysisDeclContext *AnalysisContext =
      AnalysisManager.getContext(Info.funcDecl);

  CFG &TheCFG = *AnalysisContext->getCFG();
  DT.buildDominatorTree(*AnalysisContext);

  // Collect all Fields that gets written to return it back up through the
  // recursion.
  std::vector<const FieldDecl *> WrittenFields;

  // Skip if we are in a RayGeneration shader without payload.
  if (Info.Payload) {
    std::vector<FieldDecl *> NonWriteableFields;
    std::vector<FieldDecl *> NonReadableFields;
    RecordDecl *PayloadType = GetPayloadType(Info.Payload);
    if (!PayloadType)
      return WrittenFields;

    CollectNonAccessableFields(PayloadType, Info.Stage, FieldsToIgnoreRead,
                               FieldsToIgnoreWrite, NonWriteableFields,
                               NonReadableFields);

    std::set<const CFGBlock *> Visited;
    ForwardTraverseCFGAndCollectReadsWrites(TheCFG.getEntry(), Info, Visited);

    if (Info.Payload->hasAttr<HLSLOutAttr>() ||
        Info.Payload->hasAttr<HLSLInOutAttr>()) {
      // If there is copy-out semantic on the payload field,
      // save the written fields and return it back to the caller for
      // better diagnostics in higher recursion levels.

      for (auto &p : Info.WritesPerField) {
        WrittenFields.push_back(p.first);
      }

      DiagnosePayloadWrites(S, TheCFG, DT, Info, NonWriteableFields,
                            PayloadType);
    }

    auto WrittenFieldsInCalls = DiagnosePayloadAsFunctionArg(
        S, Info, DT, FieldsToIgnoreRead, FieldsToIgnoreWrite, VisitedFunctions);

    // Add calls that write fields as writes to allow the diagnostics on reads
    // to check if a call that writes the field dominates the read.

    for (auto &P : WrittenFieldsInCalls) {
      for (const FieldDecl *Field : P.second) {
        Info.WritesPerField[Field].push_back(P.first);
      }
    }

    if (Info.Payload->hasAttr<HLSLInAttr>() ||
        Info.Payload->hasAttr<HLSLInOutAttr>())
      DiagnosePayloadReads(S, TheCFG, DT, Info, NonReadableFields);
  }

  DiagnoseBuiltinCallsWithPayload(S, TheCFG, DT, Info);

  return WrittenFields;
}

const Stmt *IgnoreParensAndDecay(const Stmt *S) {
  for (;;) {
    switch (S->getStmtClass()) {
    case Stmt::ParenExprClass:
      S = cast<ParenExpr>(S)->getSubExpr();
      break;
    case Stmt::ImplicitCastExprClass: {
      const ImplicitCastExpr *castExpr = cast<ImplicitCastExpr>(S);
      if (castExpr->getCastKind() != CK_ArrayToPointerDecay &&
          castExpr->getCastKind() != CK_NoOp &&
          castExpr->getCastKind() != CK_LValueToRValue) {
        return S;
      }
      S = castExpr->getSubExpr();
    } break;
    default:
      return S;
    }
  }
}

// Emit warnings for calls that pass the payload to extern functions.
bool DiagnoseCallExprForExternal(Sema &S, const FunctionDecl *FD,
                                 const CallExpr *CE,
                                 const ParmVarDecl *Payload) {
  // We check if we are passing the entire payload struct to an extern function.
  // Here ends what we can check, so we just issue a warning.
  if (!FD->hasBody()) {
    const DeclRefExpr *DRef = nullptr;
    const ParmVarDecl *PDecl = nullptr;
    for (unsigned i = 0; i < CE->getNumArgs(); ++i) {
      const Stmt *arg = IgnoreParensAndDecay(CE->getArg(i));
      if (const DeclRefExpr *ArgRef = dyn_cast<DeclRefExpr>(arg)) {
        if (ArgRef->getDecl() == Payload) {
          DRef = ArgRef;
          PDecl = FD->getParamDecl(i);
          break;
        }
      }
    }

    if (DRef) {
      S.Diag(CE->getExprLoc(),
             diag::warn_qualified_payload_passed_to_extern_function);
      return true;
    }
  }
  return false;
}

// Emits diagnostics for the Payload parameter of a DXR shader stage.
bool DiagnosePayloadParameter(Sema &S, ParmVarDecl *Payload, FunctionDecl *FD,
                              DXIL::PayloadAccessShaderStage stage) {
  if (!Payload) {
    // cought already during codgegen of the function
    return false;
  }
  if (!Payload->getAttr<HLSLInOutAttr>()) {
    // error: payload must be inout qualified
    return false;
  }

  CXXRecordDecl *Decl = Payload->getType()->getAsCXXRecordDecl();
  if (!Decl || Decl->isImplicit()) {
    // error: not a user defined type decl
    return false;
  }

  if (!Decl->hasAttr<HLSLRayPayloadAttr>()) {
    S.Diag(Payload->getLocation(), diag::err_payload_requires_attribute)
        << Decl->getName();
    return false;
  }

  return true;
}

class DXRShaderVisitor : public RecursiveASTVisitor<DXRShaderVisitor> {
public:
  DXRShaderVisitor(Sema &S) : S(S) {}

  void diagnose(TranslationUnitDecl *TU) { TraverseTranslationUnitDecl(TU); }

  bool VisitFunctionDecl(FunctionDecl *Decl) {
    auto attr = Decl->getAttr<HLSLShaderAttr>();
    if (!attr)
      return true;

    StringRef shaderStage = attr->getStage();
    if (StringRef("miss,closesthit,anyhit,raygeneration").count(shaderStage)) {
      ParmVarDecl *Payload = nullptr;
      if (shaderStage != "raygeneration")
        Payload = Decl->getParamDecl(0);

      DXIL::PayloadAccessShaderStage Stage =
          DXIL::PayloadAccessShaderStage::Invalid;
      if (shaderStage == "closesthit") {
        Stage = DXIL::PayloadAccessShaderStage::Closesthit;
      } else if (shaderStage == "miss") {
        Stage = DXIL::PayloadAccessShaderStage::Miss;
      } else if (shaderStage == "anyhit") {
        Stage = DXIL::PayloadAccessShaderStage::Anyhit;
      }
      // Diagnose the payload parameter.
      if (Payload) {
        DiagnosePayloadParameter(S, Payload, Decl, Stage);
      }
      DxrShaderDiagnoseInfo Info;
      Info.funcDecl = Decl;
      Info.Payload = Payload;
      Info.Stage = Stage;

      std::set<const FunctionDecl *> VisitedFunctions;
      DiagnosePayloadAccess(S, Info, {}, {}, VisitedFunctions);
    }
    return true;
  }

private:
  Sema &S;
};
} // namespace

namespace hlsl {

void DiagnoseRaytracingPayloadAccess(clang::Sema &S,
                                     clang::TranslationUnitDecl *TU) {
  DXRShaderVisitor visitor(S);
  visitor.diagnose(TU);
}

void DiagnoseCallableEntry(Sema &S, FunctionDecl *FD,
                           llvm::StringRef StageName) {
  if (!FD->getReturnType()->isVoidType())
    S.Diag(FD->getLocation(), diag::err_shader_must_return_void) << StageName;

  if (FD->getNumParams() != 1)
    S.Diag(FD->getLocation(), diag::err_raytracing_entry_param_count)
        << StageName << FD->getNumParams()
        << /*Special message for callable.*/ 3;
  else {
    ParmVarDecl *Param = FD->getParamDecl(0);
    if (!(Param->getAttr<HLSLInOutAttr>() ||
          (Param->getAttr<HLSLOutAttr>() && Param->getAttr<HLSLInAttr>())))
      S.Diag(Param->getLocation(), diag::err_payload_requires_inout)
          << /*payload|callable*/ 1 << Param;
    QualType Ty = Param->getType().getNonReferenceType();

    // Don't diagnose incomplete type here. Function parameters are
    // checked in Sema::CheckParmsForFunctionDef.
    if (!S.RequireCompleteType(Param->getLocation(), Ty, 0) &&
        !(hlsl::IsHLSLCopyableAnnotatableRecord(Ty)))
      S.Diag(Param->getLocation(), diag::err_payload_attrs_must_be_udt)
          << /*payload|attributes|callable*/ 2 << /*parameter %2|type*/ 0
          << Param;
  }
  return;
}

void DiagnoseMissOrAnyHitEntry(Sema &S, FunctionDecl *FD,
                               llvm::StringRef StageName,
                               DXIL::ShaderKind Stage) {
  if (!FD->getReturnType()->isVoidType())
    S.Diag(FD->getLocation(), diag::err_shader_must_return_void) << StageName;

  unsigned ExpectedParams = Stage == DXIL::ShaderKind::Miss ? 1 : 2;
  if (ExpectedParams != FD->getNumParams()) {
    S.Diag(FD->getLocation(), diag::err_raytracing_entry_param_count)
        << StageName << FD->getNumParams() << ExpectedParams;
    return;
  }
  ParmVarDecl *Param = FD->getParamDecl(0);
  if (!(Param->getAttr<HLSLInOutAttr>() ||
        (Param->getAttr<HLSLOutAttr>() && Param->getAttr<HLSLInAttr>()))) {
    S.Diag(Param->getLocation(), diag::err_payload_requires_inout)
        << /*payload|callable*/ 0 << Param;
    return;
  }

  if (FD->getNumParams() > 1) {
    Param = FD->getParamDecl(1);
    if (Param->getAttr<HLSLInOutAttr>() || Param->getAttr<HLSLOutAttr>()) {
      S.Diag(Param->getLocation(), diag::err_attributes_requiers_in) << Param;
      return;
    }
  }

  for (unsigned Idx = 0; Idx < ExpectedParams && Idx < FD->getNumParams();
       ++Idx) {
    Param = FD->getParamDecl(Idx);

    QualType Ty = Param->getType().getNonReferenceType();

    // Don't diagnose here, just continue if this fails. Function parameters are
    // checked in Sema::CheckParmsForFunctionDef.
    if (S.RequireCompleteType(Param->getLocation(), Ty, 0))
      continue;

    if (!(hlsl::IsHLSLCopyableAnnotatableRecord(Ty))) {
      S.Diag(Param->getLocation(), diag::err_payload_attrs_must_be_udt)
          << /*payload|attributes|callable*/ Idx << /*parameter %2|type*/ 0
          << Param;
    }
  }
  return;
}

void DiagnoseRayGenerationOrIntersectionEntry(Sema &S, FunctionDecl *FD,
                                              llvm::StringRef StageName) {
  if (!FD->getReturnType()->isVoidType())
    S.Diag(FD->getLocation(), diag::err_shader_must_return_void) << StageName;
  unsigned ExpectedParams = 0;
  if (ExpectedParams != FD->getNumParams())
    S.Diag(FD->getLocation(), diag::err_raytracing_entry_param_count)
        << StageName << FD->getNumParams() << ExpectedParams;
  return;
}

void DiagnoseClosestHitEntry(Sema &S, FunctionDecl *FD,
                             llvm::StringRef StageName) {
  if (!FD->getReturnType()->isVoidType())
    S.Diag(FD->getLocation(), diag::err_shader_must_return_void) << StageName;
  unsigned ExpectedParams = 2;

  if (ExpectedParams != FD->getNumParams()) {
    S.Diag(FD->getLocation(), diag::err_raytracing_entry_param_count)
        << StageName << FD->getNumParams() << ExpectedParams;
  }

  if (FD->getNumParams() == 0)
    return;

  ParmVarDecl *Param = FD->getParamDecl(0);
  if (!(Param->getAttr<HLSLInOutAttr>() ||
        (Param->getAttr<HLSLOutAttr>() && Param->getAttr<HLSLInAttr>()))) {
    S.Diag(Param->getLocation(), diag::err_payload_requires_inout)
        << /*payload|callable*/ 0 << Param;
  }

  if (FD->getNumParams() > 1) {
    Param = FD->getParamDecl(1);
    if (Param->getAttr<HLSLInOutAttr>() || Param->getAttr<HLSLOutAttr>()) {
      S.Diag(Param->getLocation(), diag::err_attributes_requiers_in) << Param;
    }
  }

  for (unsigned Idx = 0; Idx < ExpectedParams && Idx < FD->getNumParams();
       ++Idx) {
    Param = FD->getParamDecl(Idx);

    QualType Ty = Param->getType().getNonReferenceType();

    // Don't diagnose here, just continue if this fails. Function parameters are
    // checked in Sema::CheckParmsForFunctionDef.
    if (S.RequireCompleteType(Param->getLocation(), Ty, 0))
      continue;

    if (!(hlsl::IsHLSLCopyableAnnotatableRecord(Ty))) {
      S.Diag(Param->getLocation(), diag::err_payload_attrs_must_be_udt)
          << /*payload|attributes|callable*/ Idx << /*parameter %2|type*/ 0
          << Param;
    }
  }
  return;
}
} // namespace hlsl
