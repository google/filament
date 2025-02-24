//===- subzero/src/IceConverter.cpp - Converts LLVM to Ice  ---------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the LLVM to ICE converter.
///
//===----------------------------------------------------------------------===//

#include "IceConverter.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceGlobalContext.h"
#include "IceGlobalInits.h"
#include "IceInst.h"
#include "IceMangling.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"
#include "IceTypeConverter.h"
#include "IceTypes.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif // __clang__

#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

// TODO(kschimpf): Remove two namespaces being visible at once.
using namespace llvm;

namespace {

// Debugging helper
template <typename T> static std::string LLVMObjectAsString(const T *O) {
  std::string Dump;
  raw_string_ostream Stream(Dump);
  O->print(Stream);
  return Stream.str();
}

// Base class for converting LLVM to ICE.
// TODO(stichnot): Redesign Converter, LLVM2ICEConverter,
// LLVM2ICEFunctionConverter, and LLVM2ICEGlobalsConverter with respect to
// Translator.  In particular, the unique_ptr ownership rules in
// LLVM2ICEFunctionConverter.
class LLVM2ICEConverter {
  LLVM2ICEConverter() = delete;
  LLVM2ICEConverter(const LLVM2ICEConverter &) = delete;
  LLVM2ICEConverter &operator=(const LLVM2ICEConverter &) = delete;

public:
  explicit LLVM2ICEConverter(Ice::Converter &Converter)
      : Converter(Converter), Ctx(Converter.getContext()),
        TypeConverter(Converter.getModule()->getContext()) {}

  Ice::Converter &getConverter() const { return Converter; }

protected:
  Ice::Converter &Converter;
  Ice::GlobalContext *Ctx;
  const Ice::TypeConverter TypeConverter;
};

// Converter from LLVM functions to ICE. The entry point is the convertFunction
// method.
//
// Note: this currently assumes that the given IR was verified to be valid
// PNaCl bitcode. Otherwise, the behavior is undefined.
class LLVM2ICEFunctionConverter : LLVM2ICEConverter {
  LLVM2ICEFunctionConverter() = delete;
  LLVM2ICEFunctionConverter(const LLVM2ICEFunctionConverter &) = delete;
  LLVM2ICEFunctionConverter &
  operator=(const LLVM2ICEFunctionConverter &) = delete;

public:
  explicit LLVM2ICEFunctionConverter(Ice::Converter &Converter)
      : LLVM2ICEConverter(Converter), Func(nullptr) {}

  void convertFunction(const Function *F) {
    Func = Ice::Cfg::create(Ctx, Converter.getNextSequenceNumber());
    {
      Ice::CfgLocalAllocatorScope _(Func.get());

      VarMap.clear();
      NodeMap.clear();
      Func->setFunctionName(
          Ctx->getGlobalString(Ice::mangleName(F->getName())));
      Func->setReturnType(convertToIceType(F->getReturnType()));
      Func->setInternal(F->hasInternalLinkage());
      Ice::TimerMarker T(Ice::TimerStack::TT_llvmConvert, Func.get());

      // The initial definition/use of each arg is the entry node.
      for (auto ArgI = F->arg_begin(), ArgE = F->arg_end(); ArgI != ArgE;
           ++ArgI) {
        Func->addArg(mapValueToIceVar(&*ArgI));
      }

      // Make an initial pass through the block list just to resolve the blocks
      // in the original linearized order. Otherwise the ICE linearized order
      // will be affected by branch targets in terminator instructions.
      for (const BasicBlock &BBI : *F)
        mapBasicBlockToNode(&BBI);
      for (const BasicBlock &BBI : *F)
        convertBasicBlock(&BBI);
      Func->setEntryNode(mapBasicBlockToNode(&F->getEntryBlock()));
      Func->computeInOutEdges();
    }
    Converter.translateFcn(std::move(Func));
  }

  // convertConstant() does not use Func or require it to be a valid Ice::Cfg
  // pointer. As such, it's suitable for e.g. constructing global initializers.
  Ice::Constant *convertConstant(const Constant *Const) {
    if (const auto GV = dyn_cast<GlobalValue>(Const)) {
      Ice::GlobalDeclaration *Decl = getConverter().getGlobalDeclaration(GV);
      bool IsUndefined = false;
      if (const auto *Func = llvm::dyn_cast<Ice::FunctionDeclaration>(Decl))
        IsUndefined = Func->isProto();
      else if (const auto *Var = llvm::dyn_cast<Ice::VariableDeclaration>(Decl))
        IsUndefined = !Var->hasInitializer();
      else
        report_fatal_error("Unhandled GlobalDeclaration type");
      if (IsUndefined)
        return Ctx->getConstantExternSym(Decl->getName());
      else {
        const Ice::RelocOffsetT Offset = 0;
        return Ctx->getConstantSym(
            Offset, Ctx->getGlobalString(Decl->getName().toString()));
      }
    } else if (const auto CI = dyn_cast<ConstantInt>(Const)) {
      Ice::Type Ty = convertToIceType(CI->getType());
      return Ctx->getConstantInt(Ty, CI->getSExtValue());
    } else if (const auto CFP = dyn_cast<ConstantFP>(Const)) {
      Ice::Type Type = convertToIceType(CFP->getType());
      if (Type == Ice::IceType_f32)
        return Ctx->getConstantFloat(CFP->getValueAPF().convertToFloat());
      else if (Type == Ice::IceType_f64)
        return Ctx->getConstantDouble(CFP->getValueAPF().convertToDouble());
      llvm_unreachable("Unexpected floating point type");
      return nullptr;
    } else if (const auto CU = dyn_cast<UndefValue>(Const)) {
      return Ctx->getConstantUndef(convertToIceType(CU->getType()));
    } else {
      llvm_unreachable("Unhandled constant type");
      return nullptr;
    }
  }

private:
  // LLVM values (instructions, etc.) are mapped directly to ICE variables.
  // mapValueToIceVar has a version that forces an ICE type on the variable,
  // and a version that just uses convertToIceType on V.
  Ice::Variable *mapValueToIceVar(const Value *V, Ice::Type IceTy) {
    if (IceTy == Ice::IceType_void)
      return nullptr;
    if (VarMap.find(V) == VarMap.end()) {
      VarMap[V] = Func->makeVariable(IceTy);
      if (Ice::BuildDefs::dump())
        VarMap[V]->setName(Func.get(), V->getName());
    }
    return VarMap[V];
  }

  Ice::Variable *mapValueToIceVar(const Value *V) {
    return mapValueToIceVar(V, convertToIceType(V->getType()));
  }

  Ice::CfgNode *mapBasicBlockToNode(const BasicBlock *BB) {
    if (NodeMap.find(BB) == NodeMap.end()) {
      NodeMap[BB] = Func->makeNode();
      if (Ice::BuildDefs::dump())
        NodeMap[BB]->setName(BB->getName());
    }
    return NodeMap[BB];
  }

  Ice::Type convertToIceType(Type *LLVMTy) const {
    Ice::Type IceTy = TypeConverter.convertToIceType(LLVMTy);
    if (IceTy == Ice::IceType_NUM)
      report_fatal_error(std::string("Invalid PNaCl type ") +
                         LLVMObjectAsString(LLVMTy));
    return IceTy;
  }

  // Given an LLVM instruction and an operand number, produce the Ice::Operand
  // this refers to. If there's no such operand, return nullptr.
  Ice::Operand *convertOperand(const Instruction *Instr, unsigned OpNum) {
    if (OpNum >= Instr->getNumOperands()) {
      return nullptr;
    }
    const Value *Op = Instr->getOperand(OpNum);
    return convertValue(Op);
  }

  Ice::Operand *convertValue(const Value *Op) {
    if (const auto Const = dyn_cast<Constant>(Op)) {
      return convertConstant(Const);
    } else {
      return mapValueToIceVar(Op);
    }
  }

  // Note: this currently assumes a 1x1 mapping between LLVM IR and Ice
  // instructions.
  Ice::Inst *convertInstruction(const Instruction *Instr) {
    switch (Instr->getOpcode()) {
    case Instruction::PHI:
      return convertPHINodeInstruction(cast<PHINode>(Instr));
    case Instruction::Br:
      return convertBrInstruction(cast<BranchInst>(Instr));
    case Instruction::Ret:
      return convertRetInstruction(cast<ReturnInst>(Instr));
    case Instruction::IntToPtr:
      return convertIntToPtrInstruction(cast<IntToPtrInst>(Instr));
    case Instruction::PtrToInt:
      return convertPtrToIntInstruction(cast<PtrToIntInst>(Instr));
    case Instruction::ICmp:
      return convertICmpInstruction(cast<ICmpInst>(Instr));
    case Instruction::FCmp:
      return convertFCmpInstruction(cast<FCmpInst>(Instr));
    case Instruction::Select:
      return convertSelectInstruction(cast<SelectInst>(Instr));
    case Instruction::Switch:
      return convertSwitchInstruction(cast<SwitchInst>(Instr));
    case Instruction::Load:
      return convertLoadInstruction(cast<LoadInst>(Instr));
    case Instruction::Store:
      return convertStoreInstruction(cast<StoreInst>(Instr));
    case Instruction::ZExt:
      return convertCastInstruction(cast<ZExtInst>(Instr), Ice::InstCast::Zext);
    case Instruction::SExt:
      return convertCastInstruction(cast<SExtInst>(Instr), Ice::InstCast::Sext);
    case Instruction::Trunc:
      return convertCastInstruction(cast<TruncInst>(Instr),
                                    Ice::InstCast::Trunc);
    case Instruction::FPTrunc:
      return convertCastInstruction(cast<FPTruncInst>(Instr),
                                    Ice::InstCast::Fptrunc);
    case Instruction::FPExt:
      return convertCastInstruction(cast<FPExtInst>(Instr),
                                    Ice::InstCast::Fpext);
    case Instruction::FPToSI:
      return convertCastInstruction(cast<FPToSIInst>(Instr),
                                    Ice::InstCast::Fptosi);
    case Instruction::FPToUI:
      return convertCastInstruction(cast<FPToUIInst>(Instr),
                                    Ice::InstCast::Fptoui);
    case Instruction::SIToFP:
      return convertCastInstruction(cast<SIToFPInst>(Instr),
                                    Ice::InstCast::Sitofp);
    case Instruction::UIToFP:
      return convertCastInstruction(cast<UIToFPInst>(Instr),
                                    Ice::InstCast::Uitofp);
    case Instruction::BitCast:
      return convertCastInstruction(cast<BitCastInst>(Instr),
                                    Ice::InstCast::Bitcast);
    case Instruction::Add:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Add);
    case Instruction::Sub:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Sub);
    case Instruction::Mul:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Mul);
    case Instruction::UDiv:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Udiv);
    case Instruction::SDiv:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Sdiv);
    case Instruction::URem:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Urem);
    case Instruction::SRem:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Srem);
    case Instruction::Shl:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Shl);
    case Instruction::LShr:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Lshr);
    case Instruction::AShr:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Ashr);
    case Instruction::FAdd:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Fadd);
    case Instruction::FSub:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Fsub);
    case Instruction::FMul:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Fmul);
    case Instruction::FDiv:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Fdiv);
    case Instruction::FRem:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Frem);
    case Instruction::And:
      return convertArithInstruction(Instr, Ice::InstArithmetic::And);
    case Instruction::Or:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Or);
    case Instruction::Xor:
      return convertArithInstruction(Instr, Ice::InstArithmetic::Xor);
    case Instruction::ExtractElement:
      return convertExtractElementInstruction(cast<ExtractElementInst>(Instr));
    case Instruction::InsertElement:
      return convertInsertElementInstruction(cast<InsertElementInst>(Instr));
    case Instruction::Call:
      return convertCallInstruction(cast<CallInst>(Instr));
    case Instruction::Alloca:
      return convertAllocaInstruction(cast<AllocaInst>(Instr));
    case Instruction::Unreachable:
      return convertUnreachableInstruction(cast<UnreachableInst>(Instr));
    default:
      report_fatal_error(std::string("Invalid PNaCl instruction: ") +
                         LLVMObjectAsString(Instr));
    }

    llvm_unreachable("convertInstruction");
    return nullptr;
  }

  Ice::Inst *convertLoadInstruction(const LoadInst *Instr) {
    Ice::Operand *Src = convertOperand(Instr, 0);
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    return Ice::InstLoad::create(Func.get(), Dest, Src);
  }

  Ice::Inst *convertStoreInstruction(const StoreInst *Instr) {
    Ice::Operand *Addr = convertOperand(Instr, 1);
    Ice::Operand *Val = convertOperand(Instr, 0);
    return Ice::InstStore::create(Func.get(), Val, Addr);
  }

  Ice::Inst *convertArithInstruction(const Instruction *Instr,
                                     Ice::InstArithmetic::OpKind Opcode) {
    const auto BinOp = cast<BinaryOperator>(Instr);
    Ice::Operand *Src0 = convertOperand(Instr, 0);
    Ice::Operand *Src1 = convertOperand(Instr, 1);
    Ice::Variable *Dest = mapValueToIceVar(BinOp);
    return Ice::InstArithmetic::create(Func.get(), Opcode, Dest, Src0, Src1);
  }

  Ice::Inst *convertPHINodeInstruction(const PHINode *Instr) {
    unsigned NumValues = Instr->getNumIncomingValues();
    Ice::InstPhi *IcePhi =
        Ice::InstPhi::create(Func.get(), NumValues, mapValueToIceVar(Instr));
    for (unsigned N = 0, E = NumValues; N != E; ++N) {
      IcePhi->addArgument(convertOperand(Instr, N),
                          mapBasicBlockToNode(Instr->getIncomingBlock(N)));
    }
    return IcePhi;
  }

  Ice::Inst *convertBrInstruction(const BranchInst *Instr) {
    if (Instr->isConditional()) {
      Ice::Operand *Src = convertOperand(Instr, 0);
      BasicBlock *BBThen = Instr->getSuccessor(0);
      BasicBlock *BBElse = Instr->getSuccessor(1);
      Ice::CfgNode *NodeThen = mapBasicBlockToNode(BBThen);
      Ice::CfgNode *NodeElse = mapBasicBlockToNode(BBElse);
      return Ice::InstBr::create(Func.get(), Src, NodeThen, NodeElse);
    } else {
      BasicBlock *BBSucc = Instr->getSuccessor(0);
      return Ice::InstBr::create(Func.get(), mapBasicBlockToNode(BBSucc));
    }
  }

  Ice::Inst *convertIntToPtrInstruction(const IntToPtrInst *Instr) {
    Ice::Operand *Src = convertOperand(Instr, 0);
    Ice::Variable *Dest = mapValueToIceVar(Instr, Ice::getPointerType());
    return Ice::InstAssign::create(Func.get(), Dest, Src);
  }

  Ice::Inst *convertPtrToIntInstruction(const PtrToIntInst *Instr) {
    Ice::Operand *Src = convertOperand(Instr, 0);
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    return Ice::InstAssign::create(Func.get(), Dest, Src);
  }

  Ice::Inst *convertRetInstruction(const ReturnInst *Instr) {
    Ice::Operand *RetOperand = convertOperand(Instr, 0);
    if (RetOperand) {
      return Ice::InstRet::create(Func.get(), RetOperand);
    } else {
      return Ice::InstRet::create(Func.get());
    }
  }

  Ice::Inst *convertCastInstruction(const Instruction *Instr,
                                    Ice::InstCast::OpKind CastKind) {
    Ice::Operand *Src = convertOperand(Instr, 0);
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    return Ice::InstCast::create(Func.get(), CastKind, Dest, Src);
  }

  Ice::Inst *convertICmpInstruction(const ICmpInst *Instr) {
    Ice::Operand *Src0 = convertOperand(Instr, 0);
    Ice::Operand *Src1 = convertOperand(Instr, 1);
    Ice::Variable *Dest = mapValueToIceVar(Instr);

    Ice::InstIcmp::ICond Cond;
    switch (Instr->getPredicate()) {
    default:
      llvm_unreachable("ICmpInst predicate");
    case CmpInst::ICMP_EQ:
      Cond = Ice::InstIcmp::Eq;
      break;
    case CmpInst::ICMP_NE:
      Cond = Ice::InstIcmp::Ne;
      break;
    case CmpInst::ICMP_UGT:
      Cond = Ice::InstIcmp::Ugt;
      break;
    case CmpInst::ICMP_UGE:
      Cond = Ice::InstIcmp::Uge;
      break;
    case CmpInst::ICMP_ULT:
      Cond = Ice::InstIcmp::Ult;
      break;
    case CmpInst::ICMP_ULE:
      Cond = Ice::InstIcmp::Ule;
      break;
    case CmpInst::ICMP_SGT:
      Cond = Ice::InstIcmp::Sgt;
      break;
    case CmpInst::ICMP_SGE:
      Cond = Ice::InstIcmp::Sge;
      break;
    case CmpInst::ICMP_SLT:
      Cond = Ice::InstIcmp::Slt;
      break;
    case CmpInst::ICMP_SLE:
      Cond = Ice::InstIcmp::Sle;
      break;
    }

    return Ice::InstIcmp::create(Func.get(), Cond, Dest, Src0, Src1);
  }

  Ice::Inst *convertFCmpInstruction(const FCmpInst *Instr) {
    Ice::Operand *Src0 = convertOperand(Instr, 0);
    Ice::Operand *Src1 = convertOperand(Instr, 1);
    Ice::Variable *Dest = mapValueToIceVar(Instr);

    Ice::InstFcmp::FCond Cond;
    switch (Instr->getPredicate()) {

    default:
      llvm_unreachable("FCmpInst predicate");

    case CmpInst::FCMP_FALSE:
      Cond = Ice::InstFcmp::False;
      break;
    case CmpInst::FCMP_OEQ:
      Cond = Ice::InstFcmp::Oeq;
      break;
    case CmpInst::FCMP_OGT:
      Cond = Ice::InstFcmp::Ogt;
      break;
    case CmpInst::FCMP_OGE:
      Cond = Ice::InstFcmp::Oge;
      break;
    case CmpInst::FCMP_OLT:
      Cond = Ice::InstFcmp::Olt;
      break;
    case CmpInst::FCMP_OLE:
      Cond = Ice::InstFcmp::Ole;
      break;
    case CmpInst::FCMP_ONE:
      Cond = Ice::InstFcmp::One;
      break;
    case CmpInst::FCMP_ORD:
      Cond = Ice::InstFcmp::Ord;
      break;
    case CmpInst::FCMP_UEQ:
      Cond = Ice::InstFcmp::Ueq;
      break;
    case CmpInst::FCMP_UGT:
      Cond = Ice::InstFcmp::Ugt;
      break;
    case CmpInst::FCMP_UGE:
      Cond = Ice::InstFcmp::Uge;
      break;
    case CmpInst::FCMP_ULT:
      Cond = Ice::InstFcmp::Ult;
      break;
    case CmpInst::FCMP_ULE:
      Cond = Ice::InstFcmp::Ule;
      break;
    case CmpInst::FCMP_UNE:
      Cond = Ice::InstFcmp::Une;
      break;
    case CmpInst::FCMP_UNO:
      Cond = Ice::InstFcmp::Uno;
      break;
    case CmpInst::FCMP_TRUE:
      Cond = Ice::InstFcmp::True;
      break;
    }

    return Ice::InstFcmp::create(Func.get(), Cond, Dest, Src0, Src1);
  }

  Ice::Inst *convertExtractElementInstruction(const ExtractElementInst *Instr) {
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    Ice::Operand *Source1 = convertValue(Instr->getOperand(0));
    Ice::Operand *Source2 = convertValue(Instr->getOperand(1));
    return Ice::InstExtractElement::create(Func.get(), Dest, Source1, Source2);
  }

  Ice::Inst *convertInsertElementInstruction(const InsertElementInst *Instr) {
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    Ice::Operand *Source1 = convertValue(Instr->getOperand(0));
    Ice::Operand *Source2 = convertValue(Instr->getOperand(1));
    Ice::Operand *Source3 = convertValue(Instr->getOperand(2));
    return Ice::InstInsertElement::create(Func.get(), Dest, Source1, Source2,
                                          Source3);
  }

  Ice::Inst *convertSelectInstruction(const SelectInst *Instr) {
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    Ice::Operand *Cond = convertValue(Instr->getCondition());
    Ice::Operand *Source1 = convertValue(Instr->getTrueValue());
    Ice::Operand *Source2 = convertValue(Instr->getFalseValue());
    return Ice::InstSelect::create(Func.get(), Dest, Cond, Source1, Source2);
  }

  Ice::Inst *convertSwitchInstruction(const SwitchInst *Instr) {
    Ice::Operand *Source = convertValue(Instr->getCondition());
    Ice::CfgNode *LabelDefault = mapBasicBlockToNode(Instr->getDefaultDest());
    unsigned NumCases = Instr->getNumCases();
    Ice::InstSwitch *Switch =
        Ice::InstSwitch::create(Func.get(), NumCases, Source, LabelDefault);
    unsigned CurrentCase = 0;
    for (SwitchInst::ConstCaseIt I = Instr->case_begin(), E = Instr->case_end();
         I != E; ++I, ++CurrentCase) {
      uint64_t CaseValue = I.getCaseValue()->getSExtValue();
      Ice::CfgNode *CaseSuccessor = mapBasicBlockToNode(I.getCaseSuccessor());
      Switch->addBranch(CurrentCase, CaseValue, CaseSuccessor);
    }
    return Switch;
  }

  Ice::Inst *convertCallInstruction(const CallInst *Instr) {
    Ice::Variable *Dest = mapValueToIceVar(Instr);
    Ice::Operand *CallTarget = convertValue(Instr->getCalledValue());
    unsigned NumArgs = Instr->getNumArgOperands();

    if (const auto Target = dyn_cast<Ice::ConstantRelocatable>(CallTarget)) {
      // Check if this direct call is to an Intrinsic (starts with "llvm.")
      bool BadIntrinsic;
      const Ice::Intrinsics::FullIntrinsicInfo *Info =
          Ctx->getIntrinsicsInfo().find(Target->getName(), BadIntrinsic);
      if (BadIntrinsic) {
        report_fatal_error(std::string("Invalid PNaCl intrinsic call: ") +
                           LLVMObjectAsString(Instr));
      }
      if (Info) {
        Ice::InstIntrinsic *Intrinsic = Ice::InstIntrinsic::create(
            Func.get(), NumArgs, Dest, CallTarget, Info->Info);
        for (unsigned i = 0; i < NumArgs; ++i) {
          Intrinsic->addArg(convertOperand(Instr, i));
        }
        validateIntrinsic(Intrinsic, Info);

        return Intrinsic;
      }
    }

    // Not an intrinsic.
    // Note: Subzero doesn't (yet) do anything special with the Tail flag in
    // the bitcode, i.e. CallInst::isTailCall().
    Ice::InstCall *Call = Ice::InstCall::create(
        Func.get(), NumArgs, Dest, CallTarget, Instr->isTailCall());
    for (unsigned i = 0; i < NumArgs; ++i) {
      Intrinsic->addArg(convertOperand(Instr, i));
    }

    return Call;
  }

  Ice::Inst *convertAllocaInstruction(const AllocaInst *Instr) {
    // PNaCl bitcode only contains allocas of byte-granular objects.
    Ice::Operand *ByteCount = convertValue(Instr->getArraySize());
    uint32_t Align = Instr->getAlignment();
    Ice::Variable *Dest = mapValueToIceVar(Instr, Ice::getPointerType());

    return Ice::InstAlloca::create(Func.get(), Dest, ByteCount, Align);
  }

  Ice::Inst *convertUnreachableInstruction(const UnreachableInst * /*Instr*/) {
    return Ice::InstUnreachable::create(Func.get());
  }

  Ice::CfgNode *convertBasicBlock(const BasicBlock *BB) {
    Ice::CfgNode *Node = mapBasicBlockToNode(BB);
    for (const Instruction &II : *BB) {
      Ice::Inst *Instr = convertInstruction(&II);
      Node->appendInst(Instr);
    }
    return Node;
  }

  void validateIntrinsic(const Ice::InstIntrinsic *Intrinsic,
                         const Ice::Intrinsics::FullIntrinsicInfo *I) {
    Ice::SizeT ArgIndex = 0;
    switch (I->validateCall(Intrinsic, ArgIndex)) {
    case Ice::Intrinsics::IsValidCall:
      break;
    case Ice::Intrinsics::BadReturnType: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Intrinsic call expects return type " << I->getReturnType()
             << ". Found: " << Intrinsic->getReturnType();
      report_fatal_error(StrBuf.str());
      break;
    }
    case Ice::Intrinsics::WrongNumOfArgs: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Intrinsic call expects " << I->getNumArgs()
             << ". Found: " << Intrinsic->getNumArgs();
      report_fatal_error(StrBuf.str());
      break;
    }
    case Ice::Intrinsics::WrongCallArgType: {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Intrinsic call argument " << ArgIndex << " expects type "
             << I->getArgType(ArgIndex)
             << ". Found: " << Intrinsic->getArg(ArgIndex)->getType();
      report_fatal_error(StrBuf.str());
      break;
    }
    }
  }

private:
  // Data
  std::unique_ptr<Ice::Cfg> Func;
  std::map<const Value *, Ice::Variable *> VarMap;
  std::map<const BasicBlock *, Ice::CfgNode *> NodeMap;
};

// Converter from LLVM global variables to ICE. The entry point is the
// convertGlobalsToIce method.
//
// Note: this currently assumes that the given IR was verified to be valid
// PNaCl bitcode. Otherwise, the behavior is undefined.
class LLVM2ICEGlobalsConverter : public LLVM2ICEConverter {
  LLVM2ICEGlobalsConverter() = delete;
  LLVM2ICEGlobalsConverter(const LLVM2ICEGlobalsConverter &) = delete;
  LLVM2ICEGlobalsConverter &
  operator=(const LLVM2ICEGlobalsConverter &) = delete;

public:
  explicit LLVM2ICEGlobalsConverter(Ice::Converter &Converter,
                                    Ice::VariableDeclarationList *G)
      : LLVM2ICEConverter(Converter), GlobalPool(G) {}

  /// Converts global variables, and their initializers into ICE global variable
  /// declarations, for module Mod. Returns the set of converted declarations.
  void convertGlobalsToIce(Module *Mod);

private:
  // Adds the Initializer to the list of initializers for the Global variable
  // declaration.
  void addGlobalInitializer(Ice::VariableDeclaration &Global,
                            const Constant *Initializer) {
    constexpr bool HasOffset = false;
    constexpr Ice::RelocOffsetT Offset = 0;
    addGlobalInitializer(Global, Initializer, HasOffset, Offset);
  }

  // Adds Initializer to the list of initializers for Global variable
  // declaration. HasOffset is true only if Initializer is a relocation
  // initializer and Offset should be added to the relocation.
  void addGlobalInitializer(Ice::VariableDeclaration &Global,
                            const Constant *Initializer, bool HasOffset,
                            Ice::RelocOffsetT Offset);

  // Converts the given constant C to the corresponding integer literal it
  // contains.
  Ice::RelocOffsetT getIntegerLiteralConstant(const Value *C) {
    const auto CI = dyn_cast<ConstantInt>(C);
    if (CI && CI->getType()->isIntegerTy(32))
      return CI->getSExtValue();

    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Constant not i32 literal: " << *C;
    report_fatal_error(StrBuf.str());
    return 0;
  }

  Ice::VariableDeclarationList *GlobalPool;
};

void LLVM2ICEGlobalsConverter::convertGlobalsToIce(Module *Mod) {
  for (Module::const_global_iterator I = Mod->global_begin(),
                                     E = Mod->global_end();
       I != E; ++I) {

    const GlobalVariable *GV = &*I;

    Ice::GlobalDeclaration *Var = getConverter().getGlobalDeclaration(GV);
    auto *VarDecl = cast<Ice::VariableDeclaration>(Var);
    GlobalPool->push_back(VarDecl);

    if (!GV->hasInternalLinkage() && GV->hasInitializer()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Can't define external global declaration: " << GV->getName();
      report_fatal_error(StrBuf.str());
    }

    if (!GV->hasInitializer()) {
      if (Ice::getFlags().getAllowUninitializedGlobals())
        continue;
      else {
        std::string Buffer;
        raw_string_ostream StrBuf(Buffer);
        StrBuf << "Global declaration missing initializer: " << GV->getName();
        report_fatal_error(StrBuf.str());
      }
    }

    const Constant *Initializer = GV->getInitializer();
    if (const auto CompoundInit = dyn_cast<ConstantStruct>(Initializer)) {
      for (ConstantStruct::const_op_iterator I = CompoundInit->op_begin(),
                                             E = CompoundInit->op_end();
           I != E; ++I) {
        if (const auto Init = dyn_cast<Constant>(I)) {
          addGlobalInitializer(*VarDecl, Init);
        }
      }
    } else {
      addGlobalInitializer(*VarDecl, Initializer);
    }
  }
}

void LLVM2ICEGlobalsConverter::addGlobalInitializer(
    Ice::VariableDeclaration &Global, const Constant *Initializer,
    bool HasOffset, Ice::RelocOffsetT Offset) {
  (void)HasOffset;
  assert(HasOffset || Offset == 0);

  if (const auto CDA = dyn_cast<ConstantDataArray>(Initializer)) {
    assert(!HasOffset && isa<IntegerType>(CDA->getElementType()) &&
           (cast<IntegerType>(CDA->getElementType())->getBitWidth() == 8));
    Global.addInitializer(Ice::VariableDeclaration::DataInitializer::create(
        GlobalPool, CDA->getRawDataValues().data(), CDA->getNumElements()));
    return;
  }

  if (isa<ConstantAggregateZero>(Initializer)) {
    if (const auto AT = dyn_cast<ArrayType>(Initializer->getType())) {
      assert(!HasOffset && isa<IntegerType>(AT->getElementType()) &&
             (cast<IntegerType>(AT->getElementType())->getBitWidth() == 8));
      Global.addInitializer(Ice::VariableDeclaration::ZeroInitializer::create(
          GlobalPool, AT->getNumElements()));
    } else {
      llvm_unreachable("Unhandled constant aggregate zero type");
    }
    return;
  }

  if (const auto Exp = dyn_cast<ConstantExpr>(Initializer)) {
    switch (Exp->getOpcode()) {
    case Instruction::Add:
      assert(!HasOffset);
      addGlobalInitializer(Global, Exp->getOperand(0), true,
                           getIntegerLiteralConstant(Exp->getOperand(1)));
      return;
    case Instruction::PtrToInt: {
      assert(TypeConverter.convertToIceType(Exp->getType()) ==
             Ice::getPointerType());
      const auto GV = dyn_cast<GlobalValue>(Exp->getOperand(0));
      assert(GV);
      const Ice::GlobalDeclaration *Addr =
          getConverter().getGlobalDeclaration(GV);
      Global.addInitializer(Ice::VariableDeclaration::RelocInitializer::create(
          GlobalPool, Addr, {Ice::RelocOffset::create(Ctx, Offset)}));
      return;
    }
    default:
      break;
    }
  }

  std::string Buffer;
  raw_string_ostream StrBuf(Buffer);
  StrBuf << "Unhandled global initializer: " << Initializer;
  report_fatal_error(StrBuf.str());
}

} // end of anonymous namespace

namespace Ice {

void Converter::nameUnnamedGlobalVariables(Module *Mod) {
  const std::string GlobalPrefix = getFlags().getDefaultGlobalPrefix();
  if (GlobalPrefix.empty())
    return;
  uint32_t NameIndex = 0;
  for (auto V = Mod->global_begin(), E = Mod->global_end(); V != E; ++V) {
    if (!V->hasName()) {
      V->setName(createUnnamedName(GlobalPrefix, NameIndex));
      ++NameIndex;
    } else {
      checkIfUnnamedNameSafe(V->getName(), "global", GlobalPrefix);
    }
  }
}

void Converter::nameUnnamedFunctions(Module *Mod) {
  const std::string FunctionPrefix = getFlags().getDefaultFunctionPrefix();
  if (FunctionPrefix.empty())
    return;
  uint32_t NameIndex = 0;
  for (Function &F : *Mod) {
    if (!F.hasName()) {
      F.setName(createUnnamedName(FunctionPrefix, NameIndex));
      ++NameIndex;
    } else {
      checkIfUnnamedNameSafe(F.getName(), "function", FunctionPrefix);
    }
  }
}

void Converter::convertToIce() {
  TimerMarker T(TimerStack::TT_convertToIce, Ctx);
  nameUnnamedGlobalVariables(Mod);
  nameUnnamedFunctions(Mod);
  installGlobalDeclarations(Mod);
  convertGlobals(Mod);
  convertFunctions();
}

GlobalDeclaration *Converter::getGlobalDeclaration(const GlobalValue *V) {
  GlobalDeclarationMapType::const_iterator Pos = GlobalDeclarationMap.find(V);
  if (Pos == GlobalDeclarationMap.end()) {
    std::string Buffer;
    raw_string_ostream StrBuf(Buffer);
    StrBuf << "Can't find global declaration for: " << V->getName();
    report_fatal_error(StrBuf.str());
  }
  return Pos->second;
}

void Converter::installGlobalDeclarations(Module *Mod) {
  const TypeConverter Converter(Mod->getContext());
  // Install function declarations.
  for (const Function &Func : *Mod) {
    FuncSigType Signature;
    FunctionType *FuncType = Func.getFunctionType();
    Signature.setReturnType(
        Converter.convertToIceType(FuncType->getReturnType()));
    for (size_t I = 0; I < FuncType->getNumParams(); ++I) {
      Signature.appendArgType(
          Converter.convertToIceType(FuncType->getParamType(I)));
    }
    auto *IceFunc = FunctionDeclaration::create(
        Ctx, Signature, Func.getCallingConv(), Func.getLinkage(), Func.empty());
    IceFunc->setName(Ctx, Func.getName());
    if (!IceFunc->verifyLinkageCorrect(Ctx)) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Function " << IceFunc->getName()
             << " has incorrect linkage: " << IceFunc->getLinkageName();
      if (IceFunc->isExternal())
        StrBuf << "\n  Use flag -allow-externally-defined-symbols to override";
      report_fatal_error(StrBuf.str());
    }
    if (!IceFunc->validateTypeSignature(Ctx))
      report_fatal_error(IceFunc->getTypeSignatureError(Ctx));
    GlobalDeclarationMap[&Func] = IceFunc;
  }
  // Install global variable declarations.
  for (Module::const_global_iterator I = Mod->global_begin(),
                                     E = Mod->global_end();
       I != E; ++I) {
    const GlobalVariable *GV = &*I;
    constexpr bool NoSuppressMangling = false;
    auto *Var = VariableDeclaration::create(
        GlobalDeclarationsPool.get(), NoSuppressMangling, GV->getLinkage());
    Var->setAlignment(GV->getAlignment());
    Var->setIsConstant(GV->isConstant());
    Var->setName(Ctx, GV->getName());
    if (!Var->verifyLinkageCorrect()) {
      std::string Buffer;
      raw_string_ostream StrBuf(Buffer);
      StrBuf << "Global " << Var->getName()
             << " has incorrect linkage: " << Var->getLinkageName();
      if (Var->isExternal())
        StrBuf << "\n  Use flag -allow-externally-defined-symbols to override";
      report_fatal_error(StrBuf.str());
    }
    GlobalDeclarationMap[GV] = Var;
  }
}

void Converter::convertGlobals(Module *Mod) {
  LLVM2ICEGlobalsConverter(*this, GlobalDeclarationsPool.get())
      .convertGlobalsToIce(Mod);
  lowerGlobals(std::move(GlobalDeclarationsPool));
}

void Converter::convertFunctions() {
  for (const Function &I : *Mod) {
    if (I.empty())
      continue;
    TimerMarker _(Ctx, I.getName());
    LLVM2ICEFunctionConverter FunctionConverter(*this);
    FunctionConverter.convertFunction(&I);
  }
}

} // end of namespace Ice
