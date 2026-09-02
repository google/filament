///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilValidationUttils.cpp                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file provides utils for validating DXIL.                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxilValidationUtils.h"

#include <limits>
#include <optional>

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilEntryProps.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilResourceBinding.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/Support/Global.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"

namespace hlsl {
EntryStatus::EntryStatus(DxilEntryProps &entryProps)
    : m_bCoverageIn(false), m_bInnerCoverageIn(false), hasViewID(false) {
  for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
    hasOutputPosition[i] = false;
    OutputPositionMask[i] = 0;
  }

  outputCols.resize(entryProps.sig.OutputSignature.GetElements().size(), 0);
  patchConstOrPrimCols.resize(
      entryProps.sig.PatchConstOrPrimSignature.GetElements().size(), 0);
}

static void
TryAddLinAlgTargetType(MDTuple *MDT,
                       std::unordered_map<Type *, LinAlgTargetType> &Map) {
  if (!MDT || MDT->getNumOperands() != 6)
    return;

  ConstantAsMetadata *ConstMD0 =
      dyn_cast<ConstantAsMetadata>(MDT->getOperand(0).get());
  if (!ConstMD0)
    return;

  Type *Ty = ConstMD0->getValue()->getType();
  uint64_t Ints[5];

  for (size_t I = 0; I < 5; ++I) {
    ConstantAsMetadata *ConstMDI =
        dyn_cast<ConstantAsMetadata>(MDT->getOperand(I + 1).get());
    if (!ConstMDI)
      return;
    ConstantInt *CI = dyn_cast<ConstantInt>(ConstMDI->getValue());
    if (!CI)
      return;
    Ints[I] = CI->getLimitedValue(std::numeric_limits<unsigned>::max());
  }

  Map.try_emplace(Ty,
                  LinAlgTargetType{static_cast<DXIL::ComponentType>(Ints[0]),
                                   static_cast<unsigned>(Ints[1]),
                                   static_cast<unsigned>(Ints[2]),
                                   static_cast<DXIL::MatrixUse>(Ints[3]),
                                   static_cast<DXIL::MatrixScope>(Ints[4])});
}

ValidationContext::ValidationContext(Module &llvmModule, Module *DebugModule,
                                     DxilModule &dxilModule)
    : M(llvmModule), pDebugModule(DebugModule), DxilMod(dxilModule),
      DL(llvmModule.getDataLayout()), LastRuleEmit((ValidationRule)-1),
      kDxilControlFlowHintMDKind(llvmModule.getContext().getMDKindID(
          DxilMDHelper::kDxilControlFlowHintMDName)),
      kDxilPreciseMDKind(llvmModule.getContext().getMDKindID(
          DxilMDHelper::kDxilPreciseAttributeMDName)),
      kDxilNonUniformMDKind(llvmModule.getContext().getMDKindID(
          DxilMDHelper::kDxilNonUniformAttributeMDName)),
      kLLVMLoopMDKind(llvmModule.getContext().getMDKindID("llvm.loop")),
      slotTracker(&llvmModule, true) {
  DxilMod.GetDxilVersion(m_DxilMajor, m_DxilMinor);
  HandleTy = DxilMod.GetOP()->GetHandleType();

  for (Function &F : llvmModule.functions()) {
    if (DxilMod.HasDxilEntryProps(&F)) {
      DxilEntryProps &entryProps = DxilMod.GetDxilEntryProps(&F);
      entryStatusMap[&F] = llvm::make_unique<EntryStatus>(entryProps);
    }
  }

  isLibProfile = dxilModule.GetShaderModel()->IsLib();
  BuildResMap();
  // Collect patch constant map.
  if (isLibProfile) {
    for (Function &F : dxilModule.GetModule()->functions()) {
      if (dxilModule.HasDxilEntryProps(&F)) {
        DxilEntryProps &entryProps = dxilModule.GetDxilEntryProps(&F);
        DxilFunctionProps &props = entryProps.props;
        if (props.IsHS()) {
          PatchConstantFuncMap[props.ShaderProps.HS.patchConstantFunc]
              .emplace_back(&F);
        }
      }
    }
  } else {
    Function *Entry = dxilModule.GetEntryFunction();
    if (!dxilModule.HasDxilEntryProps(Entry)) {
      // must have props.
      EmitFnError(Entry, ValidationRule::MetaNoEntryPropsForEntry);
      return;
    }
    DxilEntryProps &entryProps = dxilModule.GetDxilEntryProps(Entry);
    DxilFunctionProps &props = entryProps.props;
    if (props.IsHS()) {
      PatchConstantFuncMap[props.ShaderProps.HS.patchConstantFunc].emplace_back(
          Entry);
    }
  }

  // Capture the TargetTypes metadata in the validation context
  // if it is present and valid.
  NamedMDNode *NMD = M.getNamedMetadata("dx.targetTypes");
  if (NMD) {
    for (llvm::MDNode *MDN : NMD->operands()) {
      MDTuple *MDT = dyn_cast<MDTuple>(MDN);
      TryAddLinAlgTargetType(MDT, LinAlgTargetTypeMap);
    }
  }
}

void ValidationContext::PropagateResMap(Value *V, DxilResourceBase *Res) {
  auto it = ResPropMap.find(V);
  if (it != ResPropMap.end()) {
    DxilResourceProperties RP = resource_helper::loadPropsFromResourceBase(Res);
    DxilResourceProperties itRP = it->second;
    if (itRP != RP) {
      EmitResourceError(Res, ValidationRule::InstrResourceMapToSingleEntry);
    }
  } else {
    DxilResourceProperties RP = resource_helper::loadPropsFromResourceBase(Res);
    ResPropMap[V] = RP;
    for (User *U : V->users()) {
      if (isa<GEPOperator>(U)) {
        PropagateResMap(U, Res);
      } else if (CallInst *CI = dyn_cast<CallInst>(U)) {
        // Stop propagate on function call.
        DxilInst_CreateHandleForLib hdl(CI);
        if (hdl) {
          DxilResourceProperties RP =
              resource_helper::loadPropsFromResourceBase(Res);
          ResPropMap[CI] = RP;
        }
      } else if (isa<LoadInst>(U)) {
        PropagateResMap(U, Res);
      } else if (isa<BitCastOperator>(U) && U->user_empty()) {
        // For hlsl type.
        continue;
      } else {
        EmitResourceError(Res, ValidationRule::InstrResourceUser);
      }
    }
  }
}

void ValidationContext::BuildResMap() {
  hlsl::OP *hlslOP = DxilMod.GetOP();

  if (isLibProfile) {
    std::unordered_set<Value *> ResSet;
    // Start from all global variable in resTab.
    for (auto &Res : DxilMod.GetCBuffers())
      PropagateResMap(Res->GetGlobalSymbol(), Res.get());
    for (auto &Res : DxilMod.GetUAVs())
      PropagateResMap(Res->GetGlobalSymbol(), Res.get());
    for (auto &Res : DxilMod.GetSRVs())
      PropagateResMap(Res->GetGlobalSymbol(), Res.get());
    for (auto &Res : DxilMod.GetSamplers())
      PropagateResMap(Res->GetGlobalSymbol(), Res.get());
  } else {
    // Scan all createHandle.
    for (auto &it : hlslOP->GetOpFuncList(DXIL::OpCode::CreateHandle)) {
      Function *F = it.second;
      if (!F)
        continue;
      for (User *U : F->users()) {
        CallInst *CI = cast<CallInst>(U);
        DxilInst_CreateHandle hdl(CI);
        // Validate Class/RangeID/Index.
        Value *resClass = hdl.get_resourceClass();
        if (!isa<ConstantInt>(resClass)) {
          EmitInstrError(CI, ValidationRule::InstrOpConstRange);
          continue;
        }
        Value *rangeIndex = hdl.get_rangeId();
        if (!isa<ConstantInt>(rangeIndex)) {
          EmitInstrError(CI, ValidationRule::InstrOpConstRange);
          continue;
        }

        DxilResourceBase *Res = nullptr;
        unsigned rangeId = hdl.get_rangeId_val();
        switch (static_cast<DXIL::ResourceClass>(hdl.get_resourceClass_val())) {
        default:
          EmitInstrError(CI, ValidationRule::InstrOpConstRange);
          continue;
          break;
        case DXIL::ResourceClass::CBuffer:
          if (DxilMod.GetCBuffers().size() > rangeId) {
            Res = &DxilMod.GetCBuffer(rangeId);
          } else {
            // Emit Error.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
          break;
        case DXIL::ResourceClass::Sampler:
          if (DxilMod.GetSamplers().size() > rangeId) {
            Res = &DxilMod.GetSampler(rangeId);
          } else {
            // Emit Error.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
          break;
        case DXIL::ResourceClass::SRV:
          if (DxilMod.GetSRVs().size() > rangeId) {
            Res = &DxilMod.GetSRV(rangeId);
          } else {
            // Emit Error.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
          break;
        case DXIL::ResourceClass::UAV:
          if (DxilMod.GetUAVs().size() > rangeId) {
            Res = &DxilMod.GetUAV(rangeId);
          } else {
            // Emit Error.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
          break;
        }

        ConstantInt *cIndex = dyn_cast<ConstantInt>(hdl.get_index());
        if (!Res->GetHLSLType()->getPointerElementType()->isArrayTy()) {
          if (!cIndex) {
            // index must be 0 for none array resource.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
        }
        if (cIndex) {
          unsigned index = cIndex->getLimitedValue();
          if (index < Res->GetLowerBound() || index > Res->GetUpperBound()) {
            // index out of range.
            EmitInstrError(CI, ValidationRule::InstrOpConstRange);
            continue;
          }
        }
        HandleResIndexMap[CI] = rangeId;
        DxilResourceProperties RP =
            resource_helper::loadPropsFromResourceBase(Res);
        ResPropMap[CI] = RP;
      }
    }
  }
  const ShaderModel &SM = *DxilMod.GetShaderModel();

  // Scan all createHandleFromBinding for validation.
  for (auto &it :
       hlslOP->GetOpFuncList(DXIL::OpCode::CreateHandleFromBinding)) {
    Function *F = it.second;
    if (!F)
      continue;
    for (User *U : F->users()) {
      CallInst *CI = cast<CallInst>(U);
      DxilInst_CreateHandleFromBinding Hdl(CI);

      // Validate bind parameter is constant.
      Value *Bind = Hdl.get_bind();
      if (!isa<Constant>(Bind)) {
        EmitInstrError(CI, ValidationRule::InstrOpConstRange);
        continue;
      }

      DxilResourceBinding B =
          resource_helper::loadBindingFromCreateHandleFromBinding(
              Hdl, hlslOP->GetHandleType(), SM);

      // Validate resourceClass is valid.
      switch (static_cast<DXIL::ResourceClass>(B.resourceClass)) {
      case DXIL::ResourceClass::CBuffer:
      case DXIL::ResourceClass::Sampler:
      case DXIL::ResourceClass::SRV:
      case DXIL::ResourceClass::UAV:
        break;
      default:
        EmitInstrError(CI, ValidationRule::InstrOpConstRange);
        continue;
      }

      // Validate constant index is within binding range.
      ConstantInt *CIndex = dyn_cast<ConstantInt>(Hdl.get_index());
      if (CIndex) {
        unsigned Index = CIndex->getLimitedValue();
        if (Index < B.rangeLowerBound || Index > B.rangeUpperBound) {
          // Index out of range.
          EmitInstrError(CI, ValidationRule::InstrOpConstRange);
          continue;
        }
      }
    }
  }

  for (auto &it : hlslOP->GetOpFuncList(DXIL::OpCode::AnnotateHandle)) {
    Function *F = it.second;
    if (!F)
      continue;

    for (User *U : F->users()) {
      CallInst *CI = cast<CallInst>(U);
      DxilInst_AnnotateHandle hdl(CI);
      DxilResourceProperties RP =
          resource_helper::loadPropsFromAnnotateHandle(hdl, SM);
      if (RP.getResourceKind() == DXIL::ResourceKind::Invalid) {
        EmitInstrError(CI, ValidationRule::InstrOpConstRange);
        continue;
      }

      ResPropMap[CI] = RP;
    }
  }
}

bool ValidationContext::HasEntryStatus(Function *F) {
  return entryStatusMap.find(F) != entryStatusMap.end();
}

EntryStatus &ValidationContext::GetEntryStatus(Function *F) {
  return *entryStatusMap[F];
}

CallGraph &ValidationContext::GetCallGraph() {
  if (!pCallGraph)
    pCallGraph = llvm::make_unique<CallGraph>(M);
  return *pCallGraph.get();
}

void ValidationContext::EmitGlobalVariableFormatError(
    GlobalVariable *GV, ValidationRule rule, ArrayRef<StringRef> args) {
  std::string ruleText = GetValidationRuleText(rule);
  FormatRuleText(ruleText, args);
  if (pDebugModule)
    GV = pDebugModule->getGlobalVariable(GV->getName());
  dxilutil::EmitErrorOnGlobalVariable(M.getContext(), GV, ruleText);
  Failed = true;
}

// This is the least desirable mechanism, as it has no context.
void ValidationContext::EmitError(ValidationRule rule) {
  dxilutil::EmitErrorOnContext(M.getContext(), GetValidationRuleText(rule));
  Failed = true;
}

void ValidationContext::FormatRuleText(std::string &ruleText,
                                       ArrayRef<StringRef> args) {
  std::string escapedArg;
  // Consider changing const char * to StringRef
  for (unsigned i = 0; i < args.size(); i++) {
    std::string argIdx = "%" + std::to_string(i);
    StringRef pArg = args[i];
    if (pArg == "")
      pArg = "<null>";
    if (pArg[0] == 1) {
      escapedArg = "";
      raw_string_ostream os(escapedArg);
      dxilutil::PrintEscapedString(pArg, os);
      os.flush();
      pArg = escapedArg;
    }

    std::string::size_type offset = ruleText.find(argIdx);
    if (offset == std::string::npos)
      continue;

    unsigned size = argIdx.size();
    ruleText.replace(offset, size, pArg);
  }
}

void ValidationContext::EmitFormatError(ValidationRule rule,
                                        ArrayRef<StringRef> args) {
  std::string ruleText = GetValidationRuleText(rule);
  FormatRuleText(ruleText, args);
  dxilutil::EmitErrorOnContext(M.getContext(), ruleText);
  Failed = true;
}

void ValidationContext::EmitMetaError(Metadata *Meta, ValidationRule rule) {
  std::string O;
  raw_string_ostream OSS(O);
  Meta->print(OSS, &M);
  dxilutil::EmitErrorOnContext(M.getContext(), GetValidationRuleText(rule) + O);
  Failed = true;
}

// Use this instead of DxilResourceBase::GetGlobalName
std::string
ValidationContext::GetResourceName(const hlsl::DxilResourceBase *Res) {
  if (!Res)
    return "nullptr";
  std::string resName = Res->GetGlobalName();
  if (!resName.empty())
    return resName;
  if (pDebugModule) {
    DxilModule &DM = pDebugModule->GetOrCreateDxilModule();
    switch (Res->GetClass()) {
    case DXIL::ResourceClass::CBuffer:
      return DM.GetCBuffer(Res->GetID()).GetGlobalName();
    case DXIL::ResourceClass::Sampler:
      return DM.GetSampler(Res->GetID()).GetGlobalName();
    case DXIL::ResourceClass::SRV:
      return DM.GetSRV(Res->GetID()).GetGlobalName();
    case DXIL::ResourceClass::UAV:
      return DM.GetUAV(Res->GetID()).GetGlobalName();
    default:
      return "Invalid Resource";
    }
  }
  // When names have been stripped, use class and binding location to
  // identify the resource.  Format is roughly:
  // Allocated:   (CB|T|U|S)<ID>: <ResourceKind> ((cb|t|u|s)<LB>[<RangeSize>]
  // space<SpaceID>) Unallocated: (CB|T|U|S)<ID>: <ResourceKind> (no bind
  // location) Example: U0: TypedBuffer (u5[2] space1)
  // [<RangeSize>] and space<SpaceID> skipped if 1 and 0 respectively.
  return (Twine(Res->GetResIDPrefix()) + Twine(Res->GetID()) + ": " +
          Twine(Res->GetResKindName()) +
          (Res->IsAllocated() ? (" (" + Twine(Res->GetResBindPrefix()) +
                                 Twine(Res->GetLowerBound()) +
                                 (Res->IsUnbounded() ? Twine("[unbounded]")
                                  : (Res->GetRangeSize() != 1)
                                      ? "[" + Twine(Res->GetRangeSize()) + "]"
                                      : Twine()) +
                                 ((Res->GetSpaceID() != 0)
                                      ? " space" + Twine(Res->GetSpaceID())
                                      : Twine()) +
                                 ")")
                              : Twine(" (no bind location)")))
      .str();
}

void ValidationContext::EmitResourceError(const hlsl::DxilResourceBase *Res,
                                          ValidationRule rule) {
  std::string QuotedRes = " '" + GetResourceName(Res) + "'";
  dxilutil::EmitErrorOnContext(M.getContext(),
                               GetValidationRuleText(rule) + QuotedRes);
  Failed = true;
}

void ValidationContext::EmitResourceFormatError(
    const hlsl::DxilResourceBase *Res, ValidationRule rule,
    ArrayRef<StringRef> args) {
  std::string QuotedRes = " '" + GetResourceName(Res) + "'";
  std::string ruleText = GetValidationRuleText(rule);
  FormatRuleText(ruleText, args);
  dxilutil::EmitErrorOnContext(M.getContext(), ruleText + QuotedRes);
  Failed = true;
}

bool ValidationContext::IsDebugFunctionCall(Instruction *I) {
  return isa<DbgInfoIntrinsic>(I);
}

Instruction *ValidationContext::GetDebugInstr(Instruction *I) {
  DXASSERT_NOMSG(I);
  if (pDebugModule) {
    // Look up the matching instruction in the debug module.
    llvm::Function *Fn = I->getParent()->getParent();
    llvm::Function *DbgFn = pDebugModule->getFunction(Fn->getName());
    if (DbgFn) {
      // Linear lookup, but then again, failing validation is rare.
      inst_iterator it = inst_begin(Fn);
      inst_iterator dbg_it = inst_begin(DbgFn);
      while (IsDebugFunctionCall(&*dbg_it))
        ++dbg_it;
      while (&*it != I) {
        ++it;
        ++dbg_it;
        while (IsDebugFunctionCall(&*dbg_it))
          ++dbg_it;
      }
      return &*dbg_it;
    }
  }
  return I;
}

// Emit Error or note on instruction `I` with `Msg`.
// If `isError` is true, `Rule` may omit repeated errors
void ValidationContext::EmitInstrDiagMsg(Instruction *I, ValidationRule Rule,
                                         std::string Msg, bool isError) {
  BasicBlock *BB = I->getParent();
  Function *F = BB->getParent();

  Instruction *DbgI = GetDebugInstr(I);
  if (isError) {
    if (const DebugLoc L = DbgI->getDebugLoc()) {
      // Instructions that get scalarized will likely hit
      // this case. Avoid redundant diagnostic messages.
      if (Rule == LastRuleEmit && L == LastDebugLocEmit) {
        return;
      }
      LastRuleEmit = Rule;
      LastDebugLocEmit = L;
    }
    dxilutil::EmitErrorOnInstruction(DbgI, Msg);
  } else {
    dxilutil::EmitNoteOnContext(DbgI->getContext(), Msg);
  }

  // Add llvm information as a note to instruction string
  std::string InstrStr;
  raw_string_ostream InstrStream(InstrStr);
  I->print(InstrStream, slotTracker);
  InstrStream.flush();
  StringRef InstrStrRef = InstrStr;
  InstrStrRef = InstrStrRef.ltrim(); // Ignore indentation
  Msg = "at '" + InstrStrRef.str() + "'";

  // Print the parent block name
  Msg += " in block '";
  if (!BB->getName().empty()) {
    Msg += BB->getName();
  } else {
    unsigned idx = 0;
    for (auto i = F->getBasicBlockList().begin(),
              e = F->getBasicBlockList().end();
         i != e; ++i) {
      if (BB == &(*i)) {
        break;
      }
      idx++;
    }
    Msg += "#" + std::to_string(idx);
  }
  Msg += "'";

  // Print the function name
  Msg += " of function '" + F->getName().str() + "'.";

  dxilutil::EmitNoteOnContext(DbgI->getContext(), Msg);

  Failed = true;
}

void ValidationContext::EmitInstrError(Instruction *I, ValidationRule rule) {
  EmitInstrDiagMsg(I, rule, GetValidationRuleText(rule));
}

void ValidationContext::EmitInstrNote(Instruction *I, std::string Msg) {
  EmitInstrDiagMsg(I, LastRuleEmit, Msg, false);
}

void ValidationContext::EmitInstrFormatError(Instruction *I,
                                             ValidationRule rule,
                                             ArrayRef<StringRef> args) {
  std::string ruleText = GetValidationRuleText(rule);
  FormatRuleText(ruleText, args);
  EmitInstrDiagMsg(I, rule, ruleText);
}

void ValidationContext::EmitSignatureError(DxilSignatureElement *SE,
                                           ValidationRule rule) {
  EmitFormatError(rule, {SE->GetName()});
}

void ValidationContext::EmitTypeError(Type *Ty, ValidationRule rule) {
  std::string O;
  raw_string_ostream OSS(O);
  Ty->print(OSS);
  EmitFormatError(rule, {OSS.str()});
}

void ValidationContext::EmitFnError(Function *F, ValidationRule rule) {
  if (pDebugModule)
    if (Function *dbgF = pDebugModule->getFunction(F->getName()))
      F = dbgF;
  dxilutil::EmitErrorOnFunction(M.getContext(), F, GetValidationRuleText(rule));
  Failed = true;
}

void ValidationContext::EmitFnFormatError(Function *F, ValidationRule rule,
                                          ArrayRef<StringRef> args) {
  std::string ruleText = GetValidationRuleText(rule);
  FormatRuleText(ruleText, args);
  if (pDebugModule)
    if (Function *dbgF = pDebugModule->getFunction(F->getName()))
      F = dbgF;
  dxilutil::EmitErrorOnFunction(M.getContext(), F, ruleText);
  Failed = true;
}

void ValidationContext::EmitFnAttributeError(Function *F, StringRef Kind,
                                             StringRef Value) {
  EmitFnFormatError(F, ValidationRule::DeclFnAttribute,
                    {F->getName(), Kind, Value});
}

llvm::StringRef ComponentTypeToString(DXIL::ComponentType CT) {
  switch (CT) {
  case DXIL::ComponentType::Invalid:
    return "Invalid";
  case DXIL::ComponentType::I1:
    return "I1";
  case DXIL::ComponentType::I16:
    return "I16";
  case DXIL::ComponentType::U16:
    return "U16";
  case DXIL::ComponentType::I32:
    return "I32";
  case DXIL::ComponentType::U32:
    return "U32";
  case DXIL::ComponentType::I64:
    return "I64";
  case DXIL::ComponentType::U64:
    return "U64";
  case DXIL::ComponentType::F16:
    return "F16";
  case DXIL::ComponentType::F32:
    return "F32";
  case DXIL::ComponentType::F64:
    return "F64";
  case DXIL::ComponentType::SNormF16:
    return "SNormF16";
  case DXIL::ComponentType::UNormF16:
    return "UNormF16";
  case DXIL::ComponentType::SNormF32:
    return "SNormF32";
  case DXIL::ComponentType::UNormF32:
    return "UNormF32";
  case DXIL::ComponentType::SNormF64:
    return "SNormF64";
  case DXIL::ComponentType::UNormF64:
    return "UNormF64";
  case DXIL::ComponentType::PackedS8x32:
    return "PackedS8x32";
  case DXIL::ComponentType::PackedU8x32:
    return "PackedU8x32";
  case DXIL::ComponentType::I8:
    return "I8";
  case DXIL::ComponentType::U8:
    return "U8";
  case DXIL::ComponentType::F8_E4M3FN:
    return "F8_E4M3FN";
  case DXIL::ComponentType::F8_E5M2:
    return "F8_E5M2";
  case DXIL::ComponentType::BFloat16:
    return "BFloat16";
  default:
    return "Unknown ComponentType";
  }
}

llvm::StringRef MatrixScopeToString(DXIL::MatrixScope MS) {
  switch (MS) {
  case DXIL::MatrixScope::Thread:
    return "Thread";
  case DXIL::MatrixScope::Wave:
    return "Wave";
  case DXIL::MatrixScope::ThreadGroup:
    return "ThreadGroup";
  default:
    return "Unknown MatrixScope";
  }
}

llvm::StringRef MatrixUseToString(DXIL::MatrixUse MU) {
  switch (MU) {
  case DXIL::MatrixUse::A:
    return "A";
  case DXIL::MatrixUse::B:
    return "B";
  case DXIL::MatrixUse::Accumulator:
    return "Accumulator";
  default:
    return "Unknown MatrixUse";
  }
}

llvm::StringRef MatrixLayoutToString(DXIL::MatrixLayout ML) {
  switch (ML) {
  case DXIL::MatrixLayout::ColumnMajor:
    return "ColumnMajor";
  case DXIL::MatrixLayout::RowMajor:
    return "RowMajor";
  case DXIL::MatrixLayout::MulOptimal:
    return "MulOptimal";
  case DXIL::MatrixLayout::MulOptimalTranspose:
    return "MulOptimalTranspose";
  case DXIL::MatrixLayout::OuterProductOptimal:
    return "OuterProductOptimal";
  case DXIL::MatrixLayout::OuterProductOptimalTranspose:
    return "OuterProductOptimalTranspose";
  default:
    return "Unknown MatrixUse";
  }
}

std::string TypeToString(llvm::Type *Ty) {
  std::string S;
  llvm::raw_string_ostream OS(S);
  Ty->print(OS);
  return OS.str();
}

} // namespace hlsl
