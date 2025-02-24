//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TargetLoweringMIPS32 class, which consists almost
/// entirely of the lowering sequence for each high-level instruction.
///
//===----------------------------------------------------------------------===//

#include "IceTargetLoweringMIPS32.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceELFObjectWriter.h"
#include "IceGlobalInits.h"
#include "IceInstMIPS32.h"
#include "IceInstVarIter.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IcePhiLoweringImpl.h"
#include "IceRegistersMIPS32.h"
#include "IceTargetLoweringMIPS32.def"
#include "IceUtils.h"
#include "llvm/Support/MathExtras.h"

namespace MIPS32 {
std::unique_ptr<::Ice::TargetLowering> createTargetLowering(::Ice::Cfg *Func) {
  return ::Ice::MIPS32::TargetMIPS32::create(Func);
}

std::unique_ptr<::Ice::TargetDataLowering>
createTargetDataLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::MIPS32::TargetDataMIPS32::create(Ctx);
}

std::unique_ptr<::Ice::TargetHeaderLowering>
createTargetHeaderLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::MIPS32::TargetHeaderMIPS32::create(Ctx);
}

void staticInit(::Ice::GlobalContext *Ctx) {
  ::Ice::MIPS32::TargetMIPS32::staticInit(Ctx);
}

bool shouldBePooled(const ::Ice::Constant *C) {
  return ::Ice::MIPS32::TargetMIPS32::shouldBePooled(C);
}

::Ice::Type getPointerType() {
  return ::Ice::MIPS32::TargetMIPS32::getPointerType();
}

} // end of namespace MIPS32

namespace Ice {
namespace MIPS32 {

using llvm::isInt;

namespace {

// The maximum number of arguments to pass in GPR registers.
constexpr uint32_t MIPS32_MAX_GPR_ARG = 4;

std::array<RegNumT, MIPS32_MAX_GPR_ARG> GPRArgInitializer;
std::array<RegNumT, MIPS32_MAX_GPR_ARG / 2> I64ArgInitializer;

constexpr uint32_t MIPS32_MAX_FP_ARG = 2;

std::array<RegNumT, MIPS32_MAX_FP_ARG> FP32ArgInitializer;
std::array<RegNumT, MIPS32_MAX_FP_ARG> FP64ArgInitializer;

const char *getRegClassName(RegClass C) {
  auto ClassNum = static_cast<RegClassMIPS32>(C);
  assert(ClassNum < RCMIPS32_NUM);
  switch (ClassNum) {
  default:
    assert(C < RC_Target);
    return regClassString(C);
    // Add handling of new register classes below.
  }
}

// Stack alignment
constexpr uint32_t MIPS32_STACK_ALIGNMENT_BYTES = 16;

// Value is in bytes. Return Value adjusted to the next highest multiple of the
// stack alignment required for the given type.
uint32_t applyStackAlignmentTy(uint32_t Value, Type Ty) {
  size_t typeAlignInBytes = typeWidthInBytes(Ty);
  // Vectors are stored on stack with the same alignment as that of int type
  if (isVectorType(Ty))
    typeAlignInBytes = typeWidthInBytes(IceType_i64);
  return Utils::applyAlignment(Value, typeAlignInBytes);
}

// Value is in bytes. Return Value adjusted to the next highest multiple of the
// stack alignment.
uint32_t applyStackAlignment(uint32_t Value) {
  return Utils::applyAlignment(Value, MIPS32_STACK_ALIGNMENT_BYTES);
}

} // end of anonymous namespace

TargetMIPS32::TargetMIPS32(Cfg *Func) : TargetLowering(Func) {}

void TargetMIPS32::assignVarStackSlots(VarList &SortedSpilledVariables,
                                       size_t SpillAreaPaddingBytes,
                                       size_t SpillAreaSizeBytes,
                                       size_t GlobalsAndSubsequentPaddingSize) {
  const VariablesMetadata *VMetadata = Func->getVMetadata();
  size_t GlobalsSpaceUsed = SpillAreaPaddingBytes;
  size_t NextStackOffset = SpillAreaPaddingBytes;
  CfgVector<size_t> LocalsSize(Func->getNumNodes());
  const bool SimpleCoalescing = !callsReturnsTwice();
  for (Variable *Var : SortedSpilledVariables) {
    size_t Increment = typeWidthInBytesOnStack(Var->getType());
    if (SimpleCoalescing && VMetadata->isTracked(Var)) {
      if (VMetadata->isMultiBlock(Var)) {
        GlobalsSpaceUsed += Increment;
        NextStackOffset = GlobalsSpaceUsed;
      } else {
        SizeT NodeIndex = VMetadata->getLocalUseNode(Var)->getIndex();
        LocalsSize[NodeIndex] += Increment;
        NextStackOffset = SpillAreaPaddingBytes +
                          GlobalsAndSubsequentPaddingSize +
                          LocalsSize[NodeIndex];
      }
    } else {
      NextStackOffset += Increment;
    }
    Var->setStackOffset(SpillAreaSizeBytes - NextStackOffset);
  }
}

void TargetMIPS32::staticInit(GlobalContext *Ctx) {
  (void)Ctx;
  RegNumT::setLimit(RegMIPS32::Reg_NUM);
  SmallBitVector IntegerRegisters(RegMIPS32::Reg_NUM);
  SmallBitVector I64PairRegisters(RegMIPS32::Reg_NUM);
  SmallBitVector Float32Registers(RegMIPS32::Reg_NUM);
  SmallBitVector Float64Registers(RegMIPS32::Reg_NUM);
  SmallBitVector VectorRegisters(RegMIPS32::Reg_NUM);
  SmallBitVector InvalidRegisters(RegMIPS32::Reg_NUM);
#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
  IntegerRegisters[RegMIPS32::val] = isInt;                                    \
  I64PairRegisters[RegMIPS32::val] = isI64Pair;                                \
  Float32Registers[RegMIPS32::val] = isFP32;                                   \
  Float64Registers[RegMIPS32::val] = isFP64;                                   \
  VectorRegisters[RegMIPS32::val] = isVec128;                                  \
  RegisterAliases[RegMIPS32::val].resize(RegMIPS32::Reg_NUM);                  \
  for (SizeT RegAlias : alias_init) {                                          \
    assert(!RegisterAliases[RegMIPS32::val][RegAlias] &&                       \
           "Duplicate alias for " #val);                                       \
    RegisterAliases[RegMIPS32::val].set(RegAlias);                             \
  }                                                                            \
  RegisterAliases[RegMIPS32::val].resize(RegMIPS32::Reg_NUM);                  \
  assert(RegisterAliases[RegMIPS32::val][RegMIPS32::val]);
  REGMIPS32_TABLE;
#undef X

  // TODO(mohit.bhakkad): Change these inits once we provide argument related
  // field in register tables
  for (size_t i = 0; i < MIPS32_MAX_GPR_ARG; i++)
    GPRArgInitializer[i] = RegNumT::fixme(RegMIPS32::Reg_A0 + i);

  for (size_t i = 0; i < MIPS32_MAX_GPR_ARG / 2; i++)
    I64ArgInitializer[i] = RegNumT::fixme(RegMIPS32::Reg_A0A1 + i);

  for (size_t i = 0; i < MIPS32_MAX_FP_ARG; i++) {
    FP32ArgInitializer[i] = RegNumT::fixme(RegMIPS32::Reg_F12 + i * 2);
    FP64ArgInitializer[i] = RegNumT::fixme(RegMIPS32::Reg_F12F13 + i);
  }

  TypeToRegisterSet[IceType_void] = InvalidRegisters;
  TypeToRegisterSet[IceType_i1] = IntegerRegisters;
  TypeToRegisterSet[IceType_i8] = IntegerRegisters;
  TypeToRegisterSet[IceType_i16] = IntegerRegisters;
  TypeToRegisterSet[IceType_i32] = IntegerRegisters;
  TypeToRegisterSet[IceType_i64] = IntegerRegisters;
  TypeToRegisterSet[IceType_f32] = Float32Registers;
  TypeToRegisterSet[IceType_f64] = Float64Registers;
  TypeToRegisterSet[IceType_v4i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v8i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v16i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v16i8] = VectorRegisters;
  TypeToRegisterSet[IceType_v8i16] = VectorRegisters;
  TypeToRegisterSet[IceType_v4i32] = VectorRegisters;
  TypeToRegisterSet[IceType_v4f32] = VectorRegisters;

  for (size_t i = 0; i < llvm::array_lengthof(TypeToRegisterSet); ++i)
    TypeToRegisterSetUnfiltered[i] = TypeToRegisterSet[i];

  filterTypeToRegisterSet(Ctx, RegMIPS32::Reg_NUM, TypeToRegisterSet,
                          llvm::array_lengthof(TypeToRegisterSet),
                          RegMIPS32::getRegName, getRegClassName);
}

void TargetMIPS32::unsetIfNonLeafFunc() {
  for (CfgNode *Node : Func->getNodes()) {
    for (Inst &Instr : Node->getInsts()) {
      if (llvm::isa<InstCall>(&Instr)) {
        // Unset MaybeLeafFunc if call instruction exists.
        MaybeLeafFunc = false;
        return;
      }
    }
  }
}

uint32_t TargetMIPS32::getStackAlignment() const {
  return MIPS32_STACK_ALIGNMENT_BYTES;
}

uint32_t TargetMIPS32::getCallStackArgumentsSizeBytes(const InstCall *Call) {
  TargetMIPS32::CallingConv CC;
  size_t OutArgsSizeBytes = 0;
  Variable *Dest = Call->getDest();
  bool PartialOnStack = false;
  if (Dest != nullptr && isVectorFloatingType(Dest->getType())) {
    CC.discardReg(RegMIPS32::Reg_A0);
    // Next vector is partially on stack
    PartialOnStack = true;
  }
  for (SizeT i = 0, NumArgs = Call->getNumArgs(); i < NumArgs; ++i) {
    Operand *Arg = legalizeUndef(Call->getArg(i));
    const Type Ty = Arg->getType();
    RegNumT RegNum;
    if (CC.argInReg(Ty, i, &RegNum)) {
      // If PartialOnStack is true and if this is a vector type then last two
      // elements are on stack
      if (PartialOnStack && isVectorType(Ty)) {
        OutArgsSizeBytes = applyStackAlignmentTy(OutArgsSizeBytes, IceType_i64);
        OutArgsSizeBytes += typeWidthInBytesOnStack(IceType_i32) * 2;
      }
      continue;
    }
    OutArgsSizeBytes = applyStackAlignmentTy(OutArgsSizeBytes, Ty);
    OutArgsSizeBytes += typeWidthInBytesOnStack(Ty);
  }
  // Add size of argument save area
  constexpr int BytesPerStackArg = 4;
  OutArgsSizeBytes += MIPS32_MAX_GPR_ARG * BytesPerStackArg;
  return applyStackAlignment(OutArgsSizeBytes);
}

namespace {
inline uint64_t getConstantMemoryOrder(Operand *Opnd) {
  if (auto *Integer = llvm::dyn_cast<ConstantInteger32>(Opnd))
    return Integer->getValue();
  return Intrinsics::MemoryOrderInvalid;
}
} // namespace

void TargetMIPS32::genTargetHelperCallFor(Inst *Instr) {
  constexpr bool NoTailCall = false;
  constexpr bool IsTargetHelperCall = true;
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest ? Dest->getType() : IceType_void;

  switch (Instr->getKind()) {
  default:
    return;
  case Inst::Select: {
    if (isVectorType(DestTy)) {
      Operand *SrcT = llvm::cast<InstSelect>(Instr)->getTrueOperand();
      Operand *SrcF = llvm::cast<InstSelect>(Instr)->getFalseOperand();
      Operand *Cond = llvm::cast<InstSelect>(Instr)->getCondition();
      Variable *T = Func->makeVariable(DestTy);
      auto *Undef = ConstantUndef::create(Ctx, DestTy);
      Context.insert<InstAssign>(T, Undef);
      auto *VarVecOn32 = llvm::cast<VariableVecOn32>(T);
      VarVecOn32->initVecElement(Func);
      for (SizeT I = 0; I < typeNumElements(DestTy); ++I) {
        auto *Index = Ctx->getConstantInt32(I);
        auto *OpC = Func->makeVariable(typeElementType(Cond->getType()));
        Context.insert<InstExtractElement>(OpC, Cond, Index);
        auto *OpT = Func->makeVariable(typeElementType(DestTy));
        Context.insert<InstExtractElement>(OpT, SrcT, Index);
        auto *OpF = Func->makeVariable(typeElementType(DestTy));
        Context.insert<InstExtractElement>(OpF, SrcF, Index);
        auto *Dst = Func->makeVariable(typeElementType(DestTy));
        Variable *DestT = Func->makeVariable(DestTy);
        Context.insert<InstSelect>(Dst, OpC, OpT, OpF);
        Context.insert<InstInsertElement>(DestT, T, Dst, Index);
        T = DestT;
      }
      Context.insert<InstAssign>(Dest, T);
      Instr->setDeleted();
    }
    return;
  }
  case Inst::Fcmp: {
    if (isVectorType(DestTy)) {
      InstFcmp::FCond Cond = llvm::cast<InstFcmp>(Instr)->getCondition();
      Operand *Src0 = Instr->getSrc(0);
      Operand *Src1 = Instr->getSrc(1);
      Variable *T = Func->makeVariable(IceType_v4f32);
      auto *Undef = ConstantUndef::create(Ctx, IceType_v4f32);
      Context.insert<InstAssign>(T, Undef);
      auto *VarVecOn32 = llvm::cast<VariableVecOn32>(T);
      VarVecOn32->initVecElement(Func);
      for (SizeT I = 0; I < typeNumElements(IceType_v4f32); ++I) {
        auto *Index = Ctx->getConstantInt32(I);
        auto *Op0 = Func->makeVariable(IceType_f32);
        Context.insert<InstExtractElement>(Op0, Src0, Index);
        auto *Op1 = Func->makeVariable(IceType_f32);
        Context.insert<InstExtractElement>(Op1, Src1, Index);
        auto *Dst = Func->makeVariable(IceType_f32);
        Variable *DestT = Func->makeVariable(IceType_v4f32);
        Context.insert<InstFcmp>(Cond, Dst, Op0, Op1);
        Context.insert<InstInsertElement>(DestT, T, Dst, Index);
        T = DestT;
      }
      Context.insert<InstAssign>(Dest, T);
      Instr->setDeleted();
    }
    return;
  }
  case Inst::Icmp: {
    if (isVectorType(DestTy)) {
      InstIcmp::ICond Cond = llvm::cast<InstIcmp>(Instr)->getCondition();
      Operand *Src0 = Instr->getSrc(0);
      Operand *Src1 = Instr->getSrc(1);
      const Type SrcType = Src0->getType();
      Variable *T = Func->makeVariable(DestTy);
      auto *Undef = ConstantUndef::create(Ctx, DestTy);
      Context.insert<InstAssign>(T, Undef);
      auto *VarVecOn32 = llvm::cast<VariableVecOn32>(T);
      VarVecOn32->initVecElement(Func);
      for (SizeT I = 0; I < typeNumElements(SrcType); ++I) {
        auto *Index = Ctx->getConstantInt32(I);
        auto *Op0 = Func->makeVariable(typeElementType(SrcType));
        Context.insert<InstExtractElement>(Op0, Src0, Index);
        auto *Op1 = Func->makeVariable(typeElementType(SrcType));
        Context.insert<InstExtractElement>(Op1, Src1, Index);
        auto *Dst = Func->makeVariable(typeElementType(DestTy));
        Variable *DestT = Func->makeVariable(DestTy);
        Context.insert<InstIcmp>(Cond, Dst, Op0, Op1);
        Context.insert<InstInsertElement>(DestT, T, Dst, Index);
        T = DestT;
      }
      Context.insert<InstAssign>(Dest, T);
      Instr->setDeleted();
    }
    return;
  }
  case Inst::Arithmetic: {
    const InstArithmetic::OpKind Op =
        llvm::cast<InstArithmetic>(Instr)->getOp();
    if (isVectorType(DestTy)) {
      scalarizeArithmetic(Op, Dest, Instr->getSrc(0), Instr->getSrc(1));
      Instr->setDeleted();
      return;
    }
    switch (DestTy) {
    default:
      return;
    case IceType_i64: {
      RuntimeHelper HelperID = RuntimeHelper::H_Num;
      switch (Op) {
      default:
        return;
      case InstArithmetic::Udiv:
        HelperID = RuntimeHelper::H_udiv_i64;
        break;
      case InstArithmetic::Sdiv:
        HelperID = RuntimeHelper::H_sdiv_i64;
        break;
      case InstArithmetic::Urem:
        HelperID = RuntimeHelper::H_urem_i64;
        break;
      case InstArithmetic::Srem:
        HelperID = RuntimeHelper::H_srem_i64;
        break;
      }

      if (HelperID == RuntimeHelper::H_Num) {
        return;
      }

      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(HelperID);
      constexpr SizeT MaxArgs = 2;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Instr->getSrc(0));
      Call->addArg(Instr->getSrc(1));
      Instr->setDeleted();
      return;
    }
    case IceType_f32:
    case IceType_f64: {
      if (Op != InstArithmetic::Frem) {
        return;
      }
      constexpr SizeT MaxArgs = 2;
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(
          DestTy == IceType_f32 ? RuntimeHelper::H_frem_f32
                                : RuntimeHelper::H_frem_f64);
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Instr->getSrc(0));
      Call->addArg(Instr->getSrc(1));
      Instr->setDeleted();
      return;
    }
    }
    llvm::report_fatal_error("Control flow should never have reached here.");
  }
  case Inst::Cast: {
    Operand *Src0 = Instr->getSrc(0);
    const Type SrcTy = Src0->getType();
    auto *CastInstr = llvm::cast<InstCast>(Instr);
    const InstCast::OpKind CastKind = CastInstr->getCastKind();

    if (isVectorType(DestTy)) {
      Variable *T = Func->makeVariable(DestTy);
      auto *VarVecOn32 = llvm::cast<VariableVecOn32>(T);
      VarVecOn32->initVecElement(Func);
      auto *Undef = ConstantUndef::create(Ctx, DestTy);
      Context.insert<InstAssign>(T, Undef);
      for (SizeT I = 0; I < typeNumElements(DestTy); ++I) {
        auto *Index = Ctx->getConstantInt32(I);
        auto *Op = Func->makeVariable(typeElementType(SrcTy));
        Context.insert<InstExtractElement>(Op, Src0, Index);
        auto *Dst = Func->makeVariable(typeElementType(DestTy));
        Variable *DestT = Func->makeVariable(DestTy);
        Context.insert<InstCast>(CastKind, Dst, Op);
        Context.insert<InstInsertElement>(DestT, T, Dst, Index);
        T = DestT;
      }
      Context.insert<InstAssign>(Dest, T);
      Instr->setDeleted();
      return;
    }

    switch (CastKind) {
    default:
      return;
    case InstCast::Fptosi:
    case InstCast::Fptoui: {
      if ((DestTy != IceType_i32) && (DestTy != IceType_i64)) {
        return;
      }
      const bool DestIs32 = DestTy == IceType_i32;
      const bool DestIsSigned = CastKind == InstCast::Fptosi;
      const bool Src0IsF32 = isFloat32Asserting32Or64(SrcTy);
      RuntimeHelper RTHFunc = RuntimeHelper::H_Num;
      if (DestIsSigned) {
        if (DestIs32) {
          return;
        }
        RTHFunc = Src0IsF32 ? RuntimeHelper::H_fptosi_f32_i64
                            : RuntimeHelper::H_fptosi_f64_i64;
      } else {
        RTHFunc = Src0IsF32 ? (DestIs32 ? RuntimeHelper::H_fptoui_f32_i32
                                        : RuntimeHelper::H_fptoui_f32_i64)
                            : (DestIs32 ? RuntimeHelper::H_fptoui_f64_i32
                                        : RuntimeHelper::H_fptoui_f64_i64);
      }
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(RTHFunc);
      static constexpr SizeT MaxArgs = 1;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Src0);
      Instr->setDeleted();
      return;
    }
    case InstCast::Sitofp:
    case InstCast::Uitofp: {
      if ((SrcTy != IceType_i32) && (SrcTy != IceType_i64)) {
        return;
      }
      const bool SourceIs32 = SrcTy == IceType_i32;
      const bool SourceIsSigned = CastKind == InstCast::Sitofp;
      const bool DestIsF32 = isFloat32Asserting32Or64(DestTy);
      RuntimeHelper RTHFunc = RuntimeHelper::H_Num;
      if (SourceIsSigned) {
        if (SourceIs32) {
          return;
        }
        RTHFunc = DestIsF32 ? RuntimeHelper::H_sitofp_i64_f32
                            : RuntimeHelper::H_sitofp_i64_f64;
      } else {
        RTHFunc = DestIsF32 ? (SourceIs32 ? RuntimeHelper::H_uitofp_i32_f32
                                          : RuntimeHelper::H_uitofp_i64_f32)
                            : (SourceIs32 ? RuntimeHelper::H_uitofp_i32_f64
                                          : RuntimeHelper::H_uitofp_i64_f64);
      }
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(RTHFunc);
      static constexpr SizeT MaxArgs = 1;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Src0);
      Instr->setDeleted();
      return;
    }
    case InstCast::Bitcast: {
      if (DestTy == SrcTy) {
        return;
      }
      Variable *CallDest = Dest;
      RuntimeHelper HelperID = RuntimeHelper::H_Num;
      switch (DestTy) {
      default:
        return;
      case IceType_i8:
        assert(SrcTy == IceType_v8i1);
        HelperID = RuntimeHelper::H_bitcast_8xi1_i8;
        CallDest = Func->makeVariable(IceType_i32);
        break;
      case IceType_i16:
        assert(SrcTy == IceType_v16i1);
        HelperID = RuntimeHelper::H_bitcast_16xi1_i16;
        CallDest = Func->makeVariable(IceType_i32);
        break;
      case IceType_v8i1: {
        assert(SrcTy == IceType_i8);
        HelperID = RuntimeHelper::H_bitcast_i8_8xi1;
        Variable *Src0AsI32 = Func->makeVariable(stackSlotType());
        // Arguments to functions are required to be at least 32 bits wide.
        Context.insert<InstCast>(InstCast::Zext, Src0AsI32, Src0);
        Src0 = Src0AsI32;
      } break;
      case IceType_v16i1: {
        assert(SrcTy == IceType_i16);
        HelperID = RuntimeHelper::H_bitcast_i16_16xi1;
        Variable *Src0AsI32 = Func->makeVariable(stackSlotType());
        // Arguments to functions are required to be at least 32 bits wide.
        Context.insert<InstCast>(InstCast::Zext, Src0AsI32, Src0);
        Src0 = Src0AsI32;
      } break;
      }
      constexpr SizeT MaxSrcs = 1;
      InstCall *Call = makeHelperCall(HelperID, CallDest, MaxSrcs);
      Call->addArg(Src0);
      Context.insert(Call);
      // The PNaCl ABI disallows i8/i16 return types, so truncate the helper
      // call result to the appropriate type as necessary.
      if (CallDest->getType() != DestTy)
        Context.insert<InstCast>(InstCast::Trunc, Dest, CallDest);
      Instr->setDeleted();
      return;
    }
    case InstCast::Trunc: {
      if (DestTy == SrcTy) {
        return;
      }
      if (!isVectorType(SrcTy)) {
        return;
      }
      assert(typeNumElements(DestTy) == typeNumElements(SrcTy));
      assert(typeElementType(DestTy) == IceType_i1);
      assert(isVectorIntegerType(SrcTy));
      return;
    }
    case InstCast::Sext:
    case InstCast::Zext: {
      if (DestTy == SrcTy) {
        return;
      }
      if (!isVectorType(DestTy)) {
        return;
      }
      assert(typeNumElements(DestTy) == typeNumElements(SrcTy));
      assert(typeElementType(SrcTy) == IceType_i1);
      assert(isVectorIntegerType(DestTy));
      return;
    }
    }
    llvm::report_fatal_error("Control flow should never have reached here.");
  }
  case Inst::Intrinsic: {
    auto *Intrinsic = llvm::cast<InstIntrinsic>(Instr);
    Intrinsics::IntrinsicID ID = Intrinsic->getIntrinsicID();
    if (isVectorType(DestTy) && ID == Intrinsics::Fabs) {
      Operand *Src0 = Intrinsic->getArg(0);
      Intrinsics::IntrinsicInfo Info = Intrinsic->getIntrinsicInfo();

      Variable *T = Func->makeVariable(IceType_v4f32);
      auto *Undef = ConstantUndef::create(Ctx, IceType_v4f32);
      Context.insert<InstAssign>(T, Undef);
      auto *VarVecOn32 = llvm::cast<VariableVecOn32>(T);
      VarVecOn32->initVecElement(Func);

      for (SizeT i = 0; i < typeNumElements(IceType_v4f32); ++i) {
        auto *Index = Ctx->getConstantInt32(i);
        auto *Op = Func->makeVariable(IceType_f32);
        Context.insert<InstExtractElement>(Op, Src0, Index);
        auto *Res = Func->makeVariable(IceType_f32);
        Variable *DestT = Func->makeVariable(IceType_v4f32);
        auto *Intrinsic = Context.insert<InstIntrinsic>(1, Res, Info);
        Intrinsic->addArg(Op);
        Context.insert<InstInsertElement>(DestT, T, Res, Index);
        T = DestT;
      }

      Context.insert<InstAssign>(Dest, T);

      Instr->setDeleted();
      return;
    }
    switch (ID) {
    default:
      return;
    case Intrinsics::AtomicLoad: {
      if (DestTy != IceType_i64)
        return;
      if (!Intrinsics::isMemoryOrderValid(
              ID, getConstantMemoryOrder(Intrinsic->getArg(1)))) {
        Func->setError("Unexpected memory ordering for AtomicLoad");
        return;
      }
      Operand *Addr = Intrinsic->getArg(0);
      Operand *TargetHelper = Ctx->getConstantExternSym(
          Ctx->getGlobalString("__sync_val_compare_and_swap_8"));
      static constexpr SizeT MaxArgs = 3;
      auto *_0 = Ctx->getConstantZero(IceType_i64);
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Addr);
      Call->addArg(_0);
      Call->addArg(_0);
      Context.insert<InstMIPS32Sync>();
      Instr->setDeleted();
      return;
    }
    case Intrinsics::AtomicStore: {
      Operand *Val = Intrinsic->getArg(0);
      if (Val->getType() != IceType_i64)
        return;
      if (!Intrinsics::isMemoryOrderValid(
              ID, getConstantMemoryOrder(Intrinsic->getArg(2)))) {
        Func->setError("Unexpected memory ordering for AtomicStore");
        return;
      }
      Operand *Addr = Intrinsic->getArg(1);
      Variable *NoDest = nullptr;
      Operand *TargetHelper = Ctx->getConstantExternSym(
          Ctx->getGlobalString("__sync_lock_test_and_set_8"));
      Context.insert<InstMIPS32Sync>();
      static constexpr SizeT MaxArgs = 2;
      auto *Call = Context.insert<InstCall>(MaxArgs, NoDest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Addr);
      Call->addArg(Val);
      Context.insert<InstMIPS32Sync>();
      Instr->setDeleted();
      return;
    }
    case Intrinsics::AtomicCmpxchg: {
      if (DestTy != IceType_i64)
        return;
      if (!Intrinsics::isMemoryOrderValid(
              ID, getConstantMemoryOrder(Intrinsic->getArg(3)),
              getConstantMemoryOrder(Intrinsic->getArg(4)))) {
        Func->setError("Unexpected memory ordering for AtomicCmpxchg");
        return;
      }
      Operand *Addr = Intrinsic->getArg(0);
      Operand *Oldval = Intrinsic->getArg(1);
      Operand *Newval = Intrinsic->getArg(2);
      Operand *TargetHelper = Ctx->getConstantExternSym(
          Ctx->getGlobalString("__sync_val_compare_and_swap_8"));
      Context.insert<InstMIPS32Sync>();
      static constexpr SizeT MaxArgs = 3;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Addr);
      Call->addArg(Oldval);
      Call->addArg(Newval);
      Context.insert<InstMIPS32Sync>();
      Instr->setDeleted();
      return;
    }
    case Intrinsics::AtomicRMW: {
      if (DestTy != IceType_i64)
        return;
      if (!Intrinsics::isMemoryOrderValid(
              ID, getConstantMemoryOrder(Intrinsic->getArg(3)))) {
        Func->setError("Unexpected memory ordering for AtomicRMW");
        return;
      }
      auto Operation = static_cast<Intrinsics::AtomicRMWOperation>(
          llvm::cast<ConstantInteger32>(Intrinsic->getArg(0))->getValue());
      auto *Addr = Intrinsic->getArg(1);
      auto *Newval = Intrinsic->getArg(2);
      Operand *TargetHelper;
      switch (Operation) {
      case Intrinsics::AtomicAdd:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_fetch_and_add_8"));
        break;
      case Intrinsics::AtomicSub:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_fetch_and_sub_8"));
        break;
      case Intrinsics::AtomicOr:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_fetch_and_or_8"));
        break;
      case Intrinsics::AtomicAnd:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_fetch_and_and_8"));
        break;
      case Intrinsics::AtomicXor:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_fetch_and_xor_8"));
        break;
      case Intrinsics::AtomicExchange:
        TargetHelper = Ctx->getConstantExternSym(
            Ctx->getGlobalString("__sync_lock_test_and_set_8"));
        break;
      default:
        llvm::report_fatal_error("Unknown AtomicRMW operation");
        return;
      }
      Context.insert<InstMIPS32Sync>();
      static constexpr SizeT MaxArgs = 2;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Addr);
      Call->addArg(Newval);
      Context.insert<InstMIPS32Sync>();
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Ctpop: {
      Operand *Src0 = Intrinsic->getArg(0);
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(isInt32Asserting32Or64(Src0->getType())
                                        ? RuntimeHelper::H_call_ctpop_i32
                                        : RuntimeHelper::H_call_ctpop_i64);
      static constexpr SizeT MaxArgs = 1;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Src0);
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Longjmp: {
      static constexpr SizeT MaxArgs = 2;
      static constexpr Variable *NoDest = nullptr;
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(RuntimeHelper::H_call_longjmp);
      auto *Call = Context.insert<InstCall>(MaxArgs, NoDest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Intrinsic->getArg(0));
      Call->addArg(Intrinsic->getArg(1));
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Memcpy: {
      static constexpr SizeT MaxArgs = 3;
      static constexpr Variable *NoDest = nullptr;
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(RuntimeHelper::H_call_memcpy);
      auto *Call = Context.insert<InstCall>(MaxArgs, NoDest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Intrinsic->getArg(0));
      Call->addArg(Intrinsic->getArg(1));
      Call->addArg(Intrinsic->getArg(2));
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Memmove: {
      static constexpr SizeT MaxArgs = 3;
      static constexpr Variable *NoDest = nullptr;
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(RuntimeHelper::H_call_memmove);
      auto *Call = Context.insert<InstCall>(MaxArgs, NoDest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Intrinsic->getArg(0));
      Call->addArg(Intrinsic->getArg(1));
      Call->addArg(Intrinsic->getArg(2));
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Memset: {
      Operand *ValOp = Intrinsic->getArg(1);
      assert(ValOp->getType() == IceType_i8);
      Variable *ValExt = Func->makeVariable(stackSlotType());
      Context.insert<InstCast>(InstCast::Zext, ValExt, ValOp);

      static constexpr SizeT MaxArgs = 3;
      static constexpr Variable *NoDest = nullptr;
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(RuntimeHelper::H_call_memset);
      auto *Call = Context.insert<InstCall>(MaxArgs, NoDest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Intrinsic->getArg(0));
      Call->addArg(ValExt);
      Call->addArg(Intrinsic->getArg(2));
      Instr->setDeleted();
      return;
    }
    case Intrinsics::Setjmp: {
      static constexpr SizeT MaxArgs = 1;
      Operand *TargetHelper =
          Ctx->getRuntimeHelperFunc(RuntimeHelper::H_call_setjmp);
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Intrinsic->getArg(0));
      Instr->setDeleted();
      return;
    }
    }
    llvm::report_fatal_error("Control flow should never have reached here.");
  }
  }
}

void TargetMIPS32::findMaxStackOutArgsSize() {
  // MinNeededOutArgsBytes should be updated if the Target ever creates a
  // high-level InstCall that requires more stack bytes.
  size_t MinNeededOutArgsBytes = 0;
  if (!MaybeLeafFunc)
    MinNeededOutArgsBytes = MIPS32_MAX_GPR_ARG * 4;
  MaxOutArgsSizeBytes = MinNeededOutArgsBytes;
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    while (!Context.atEnd()) {
      PostIncrLoweringContext PostIncrement(Context);
      Inst *CurInstr = iteratorToInst(Context.getCur());
      if (auto *Call = llvm::dyn_cast<InstCall>(CurInstr)) {
        SizeT OutArgsSizeBytes = getCallStackArgumentsSizeBytes(Call);
        MaxOutArgsSizeBytes = std::max(MaxOutArgsSizeBytes, OutArgsSizeBytes);
      }
    }
  }
  CurrentAllocaOffset = MaxOutArgsSizeBytes;
}

void TargetMIPS32::translateO2() {
  TimerMarker T(TimerStack::TT_O2, Func);

  // TODO(stichnot): share passes with X86?
  // https://code.google.com/p/nativeclient/issues/detail?id=4094
  genTargetHelperCalls();

  unsetIfNonLeafFunc();

  findMaxStackOutArgsSize();

  // Merge Alloca instructions, and lay out the stack.
  static constexpr bool SortAndCombineAllocas = true;
  Func->processAllocas(SortAndCombineAllocas);
  Func->dump("After Alloca processing");

  if (!getFlags().getEnablePhiEdgeSplit()) {
    // Lower Phi instructions.
    Func->placePhiLoads();
    if (Func->hasError())
      return;
    Func->placePhiStores();
    if (Func->hasError())
      return;
    Func->deletePhis();
    if (Func->hasError())
      return;
    Func->dump("After Phi lowering");
  }

  // Address mode optimization.
  Func->getVMetadata()->init(VMK_SingleDefs);
  Func->doAddressOpt();

  // Argument lowering
  Func->doArgLowering();

  // Target lowering. This requires liveness analysis for some parts of the
  // lowering decisions, such as compare/branch fusing. If non-lightweight
  // liveness analysis is used, the instructions need to be renumbered first.
  // TODO: This renumbering should only be necessary if we're actually
  // calculating live intervals, which we only do for register allocation.
  Func->renumberInstructions();
  if (Func->hasError())
    return;

  // TODO: It should be sufficient to use the fastest liveness calculation,
  // i.e. livenessLightweight(). However, for some reason that slows down the
  // rest of the translation. Investigate.
  Func->liveness(Liveness_Basic);
  if (Func->hasError())
    return;
  Func->dump("After MIPS32 address mode opt");

  Func->genCode();
  if (Func->hasError())
    return;
  Func->dump("After MIPS32 codegen");

  // Register allocation. This requires instruction renumbering and full
  // liveness analysis.
  Func->renumberInstructions();
  if (Func->hasError())
    return;
  Func->liveness(Liveness_Intervals);
  if (Func->hasError())
    return;
  // The post-codegen dump is done here, after liveness analysis and associated
  // cleanup, to make the dump cleaner and more useful.
  Func->dump("After initial MIPS32 codegen");
  // Validate the live range computations. The expensive validation call is
  // deliberately only made when assertions are enabled.
  assert(Func->validateLiveness());
  Func->getVMetadata()->init(VMK_All);
  regAlloc(RAK_Global);
  if (Func->hasError())
    return;
  Func->dump("After linear scan regalloc");

  if (getFlags().getEnablePhiEdgeSplit()) {
    Func->advancedPhiLowering();
    Func->dump("After advanced Phi lowering");
  }

  // Stack frame mapping.
  Func->genFrame();
  if (Func->hasError())
    return;
  Func->dump("After stack frame mapping");

  postLowerLegalization();
  if (Func->hasError())
    return;
  Func->dump("After postLowerLegalization");

  Func->contractEmptyNodes();
  Func->reorderNodes();

  // Branch optimization. This needs to be done just before code emission. In
  // particular, no transformations that insert or reorder CfgNodes should be
  // done after branch optimization. We go ahead and do it before nop insertion
  // to reduce the amount of work needed for searching for opportunities.
  Func->doBranchOpt();
  Func->dump("After branch optimization");
}

void TargetMIPS32::translateOm1() {
  TimerMarker T(TimerStack::TT_Om1, Func);

  // TODO: share passes with X86?
  genTargetHelperCalls();

  unsetIfNonLeafFunc();

  findMaxStackOutArgsSize();

  // Do not merge Alloca instructions, and lay out the stack.
  static constexpr bool SortAndCombineAllocas = false;
  Func->processAllocas(SortAndCombineAllocas);
  Func->dump("After Alloca processing");

  Func->placePhiLoads();
  if (Func->hasError())
    return;
  Func->placePhiStores();
  if (Func->hasError())
    return;
  Func->deletePhis();
  if (Func->hasError())
    return;
  Func->dump("After Phi lowering");

  Func->doArgLowering();

  Func->genCode();
  if (Func->hasError())
    return;
  Func->dump("After initial MIPS32 codegen");

  regAlloc(RAK_InfOnly);
  if (Func->hasError())
    return;
  Func->dump("After regalloc of infinite-weight variables");

  Func->genFrame();
  if (Func->hasError())
    return;
  Func->dump("After stack frame mapping");

  postLowerLegalization();
  if (Func->hasError())
    return;
  Func->dump("After postLowerLegalization");
}

bool TargetMIPS32::doBranchOpt(Inst *Instr, const CfgNode *NextNode) {
  if (auto *Br = llvm::dyn_cast<InstMIPS32Br>(Instr)) {
    return Br->optimizeBranch(NextNode);
  }
  return false;
}

namespace {

const char *RegNames[RegMIPS32::Reg_NUM] = {
#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
  name,
    REGMIPS32_TABLE
#undef X
};

} // end of anonymous namespace

const char *RegMIPS32::getRegName(RegNumT RegNum) {
  RegNum.assertIsValid();
  return RegNames[RegNum];
}

const char *TargetMIPS32::getRegName(RegNumT RegNum, Type Ty) const {
  (void)Ty;
  return RegMIPS32::getRegName(RegNum);
}

Variable *TargetMIPS32::getPhysicalRegister(RegNumT RegNum, Type Ty) {
  if (Ty == IceType_void)
    Ty = IceType_i32;
  if (PhysicalRegisters[Ty].empty())
    PhysicalRegisters[Ty].resize(RegMIPS32::Reg_NUM);
  RegNum.assertIsValid();
  Variable *Reg = PhysicalRegisters[Ty][RegNum];
  if (Reg == nullptr) {
    Reg = Func->makeVariable(Ty);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[Ty][RegNum] = Reg;
    // Specially mark a named physical register as an "argument" so that it is
    // considered live upon function entry.  Otherwise it's possible to get
    // liveness validation errors for saving callee-save registers.
    Func->addImplicitArg(Reg);
    // Don't bother tracking the live range of a named physical register.
    Reg->setIgnoreLiveness();
  }
  return Reg;
}

void TargetMIPS32::emitJumpTable(const Cfg *Func,
                                 const InstJumpTable *JumpTable) const {
  (void)Func;
  (void)JumpTable;
  UnimplementedError(getFlags());
}

/// Provide a trivial wrapper to legalize() for this common usage.
Variable *TargetMIPS32::legalizeToReg(Operand *From, RegNumT RegNum) {
  return llvm::cast<Variable>(legalize(From, Legal_Reg, RegNum));
}

/// Legalize undef values to concrete values.
Operand *TargetMIPS32::legalizeUndef(Operand *From, RegNumT RegNum) {
  (void)RegNum;
  Type Ty = From->getType();
  if (llvm::isa<ConstantUndef>(From)) {
    // Lower undefs to zero.  Another option is to lower undefs to an
    // uninitialized register; however, using an uninitialized register
    // results in less predictable code.
    //
    // If in the future the implementation is changed to lower undef
    // values to uninitialized registers, a FakeDef will be needed:
    //     Context.insert(InstFakeDef::create(Func, Reg));
    // This is in order to ensure that the live range of Reg is not
    // overestimated.  If the constant being lowered is a 64 bit value,
    // then the result should be split and the lo and hi components will
    // need to go in uninitialized registers.
    if (isVectorType(Ty)) {
      Variable *Var = makeReg(Ty, RegNum);
      auto *Reg = llvm::cast<VariableVecOn32>(Var);
      Reg->initVecElement(Func);
      auto *Zero = getZero();
      for (Variable *Var : Reg->getContainers()) {
        _mov(Var, Zero);
      }
      return Reg;
    }
    return Ctx->getConstantZero(Ty);
  }
  return From;
}

Variable *TargetMIPS32::makeReg(Type Type, RegNumT RegNum) {
  // There aren't any 64-bit integer registers for Mips32.
  assert(Type != IceType_i64);
  Variable *Reg = Func->makeVariable(Type);
  if (RegNum.hasValue())
    Reg->setRegNum(RegNum);
  else
    Reg->setMustHaveReg();
  return Reg;
}

OperandMIPS32Mem *TargetMIPS32::formMemoryOperand(Operand *Operand, Type Ty) {
  // It may be the case that address mode optimization already creates an
  // OperandMIPS32Mem, so in that case it wouldn't need another level of
  // transformation.
  if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(Operand)) {
    return llvm::cast<OperandMIPS32Mem>(legalize(Mem));
  }

  // If we didn't do address mode optimization, then we only have a base/offset
  // to work with. MIPS always requires a base register, so just use that to
  // hold the operand.
  auto *Base = llvm::cast<Variable>(
      legalize(Operand, Legal_Reg | Legal_Rematerializable));
  const int32_t Offset = Base->hasStackOffset() ? Base->getStackOffset() : 0;
  return OperandMIPS32Mem::create(
      Func, Ty, Base,
      llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(Offset)));
}

void TargetMIPS32::emitVariable(const Variable *Var) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  const Type FrameSPTy = IceType_i32;
  if (Var->hasReg()) {
    Str << '$' << getRegName(Var->getRegNum(), Var->getType());
    return;
  }
  if (Var->mustHaveReg()) {
    llvm::report_fatal_error("Infinite-weight Variable (" + Var->getName() +
                             ") has no register assigned - function " +
                             Func->getFunctionName());
  }
  const int32_t Offset = Var->getStackOffset();
  Str << Offset;
  Str << "($" << getRegName(getFrameOrStackReg(), FrameSPTy);
  Str << ")";
}

TargetMIPS32::CallingConv::CallingConv()
    : GPRegsUsed(RegMIPS32::Reg_NUM),
      GPRArgs(GPRArgInitializer.rbegin(), GPRArgInitializer.rend()),
      I64Args(I64ArgInitializer.rbegin(), I64ArgInitializer.rend()),
      VFPRegsUsed(RegMIPS32::Reg_NUM),
      FP32Args(FP32ArgInitializer.rbegin(), FP32ArgInitializer.rend()),
      FP64Args(FP64ArgInitializer.rbegin(), FP64ArgInitializer.rend()) {}

// In MIPS O32 abi FP argument registers can be used only if first argument is
// of type float/double. UseFPRegs flag is used to care of that. Also FP arg
// registers can be used only for first 2 arguments, so we require argument
// number to make register allocation decisions.
bool TargetMIPS32::CallingConv::argInReg(Type Ty, uint32_t ArgNo,
                                         RegNumT *Reg) {
  if (isScalarIntegerType(Ty) || isVectorType(Ty))
    return argInGPR(Ty, Reg);
  if (isScalarFloatingType(Ty)) {
    if (ArgNo == 0) {
      UseFPRegs = true;
      return argInVFP(Ty, Reg);
    }
    if (UseFPRegs && ArgNo == 1) {
      UseFPRegs = false;
      return argInVFP(Ty, Reg);
    }
    return argInGPR(Ty, Reg);
  }
  llvm::report_fatal_error("argInReg: Invalid type.");
  return false;
}

bool TargetMIPS32::CallingConv::argInGPR(Type Ty, RegNumT *Reg) {
  CfgVector<RegNumT> *Source;

  switch (Ty) {
  default: {
    llvm::report_fatal_error("argInGPR: Invalid type.");
    return false;
  } break;
  case IceType_v4i1:
  case IceType_v8i1:
  case IceType_v16i1:
  case IceType_v16i8:
  case IceType_v8i16:
  case IceType_v4i32:
  case IceType_v4f32:
  case IceType_i32:
  case IceType_f32: {
    Source = &GPRArgs;
  } break;
  case IceType_i64:
  case IceType_f64: {
    Source = &I64Args;
  } break;
  }

  discardUnavailableGPRsAndTheirAliases(Source);

  // If $4 is used for any scalar type (or returining v4f32) then the next
  // vector type if passed in $6:$7:stack:stack
  if (isVectorType(Ty)) {
    alignGPR(Source);
  }

  if (Source->empty()) {
    GPRegsUsed.set();
    return false;
  }

  *Reg = Source->back();
  // Note that we don't Source->pop_back() here. This is intentional. Notice how
  // we mark all of Reg's aliases as Used. So, for the next argument,
  // Source->back() is marked as unavailable, and it is thus implicitly popped
  // from the stack.
  GPRegsUsed |= RegisterAliases[*Reg];

  // All vector arguments irrespective of their base type are passed in GP
  // registers. First vector argument is passed in $4:$5:$6:$7 and 2nd
  // is passed in $6:$7:stack:stack. If it is 1st argument then discard
  // $4:$5:$6:$7 otherwise discard $6:$7 only.
  if (isVectorType(Ty)) {
    if (((unsigned)*Reg) == RegMIPS32::Reg_A0) {
      GPRegsUsed |= RegisterAliases[RegMIPS32::Reg_A1];
      GPRegsUsed |= RegisterAliases[RegMIPS32::Reg_A2];
      GPRegsUsed |= RegisterAliases[RegMIPS32::Reg_A3];
    } else {
      GPRegsUsed |= RegisterAliases[RegMIPS32::Reg_A3];
    }
  }

  return true;
}

inline void TargetMIPS32::CallingConv::discardNextGPRAndItsAliases(
    CfgVector<RegNumT> *Regs) {
  GPRegsUsed |= RegisterAliases[Regs->back()];
  Regs->pop_back();
}

inline void TargetMIPS32::CallingConv::alignGPR(CfgVector<RegNumT> *Regs) {
  if (Regs->back() == RegMIPS32::Reg_A1 || Regs->back() == RegMIPS32::Reg_A3)
    discardNextGPRAndItsAliases(Regs);
}

// GPR are not packed when passing parameters. Thus, a function foo(i32, i64,
// i32) will have the first argument in a0, the second in a2-a3, and the third
// on the stack. To model this behavior, whenever we pop a register from Regs,
// we remove all of its aliases from the pool of available GPRs. This has the
// effect of computing the "closure" on the GPR registers.
void TargetMIPS32::CallingConv::discardUnavailableGPRsAndTheirAliases(
    CfgVector<RegNumT> *Regs) {
  while (!Regs->empty() && GPRegsUsed[Regs->back()]) {
    discardNextGPRAndItsAliases(Regs);
  }
}

bool TargetMIPS32::CallingConv::argInVFP(Type Ty, RegNumT *Reg) {
  CfgVector<RegNumT> *Source;

  switch (Ty) {
  default: {
    llvm::report_fatal_error("argInVFP: Invalid type.");
    return false;
  } break;
  case IceType_f32: {
    Source = &FP32Args;
  } break;
  case IceType_f64: {
    Source = &FP64Args;
  } break;
  }

  discardUnavailableVFPRegsAndTheirAliases(Source);

  if (Source->empty()) {
    VFPRegsUsed.set();
    return false;
  }

  *Reg = Source->back();
  VFPRegsUsed |= RegisterAliases[*Reg];

  // In MIPS O32 abi if fun arguments are (f32, i32) then one can not use reg_a0
  // for second argument even though it's free. f32 arg goes in reg_f12, i32 arg
  // goes in reg_a1. Similarly if arguments are (f64, i32) second argument goes
  // in reg_a3 and a0, a1 are not used.
  Source = &GPRArgs;
  // Discard one GPR reg for f32(4 bytes), two for f64(4 + 4 bytes)
  if (Ty == IceType_f64) {
    // In MIPS o32 abi, when we use GPR argument pairs to store F64 values, pair
    // must be aligned at even register. Similarly when we discard GPR registers
    // when some arguments from starting 16 bytes goes in FPR, we must take care
    // of alignment. For example if fun args are (f32, f64, f32), for first f32
    // we discard a0, now for f64 argument, which will go in F14F15, we must
    // first align GPR vector to even register by discarding a1, then discard
    // two GPRs a2 and a3. Now last f32 argument will go on stack.
    alignGPR(Source);
    discardNextGPRAndItsAliases(Source);
  }
  discardNextGPRAndItsAliases(Source);
  return true;
}

void TargetMIPS32::CallingConv::discardUnavailableVFPRegsAndTheirAliases(
    CfgVector<RegNumT> *Regs) {
  while (!Regs->empty() && VFPRegsUsed[Regs->back()]) {
    Regs->pop_back();
  }
}

void TargetMIPS32::lowerArguments() {
  VarList &Args = Func->getArgs();
  TargetMIPS32::CallingConv CC;

  // For each register argument, replace Arg in the argument list with the home
  // register. Then generate an instruction in the prolog to copy the home
  // register to the assigned location of Arg.
  Context.init(Func->getEntryNode());
  Context.setInsertPoint(Context.getCur());

  // v4f32 is returned through stack. $4 is setup by the caller and passed as
  // first argument implicitly. Callee then copies the return vector at $4.
  Variable *ImplicitRetVec = nullptr;
  if (isVectorFloatingType(Func->getReturnType())) {
    ImplicitRetVec = Func->makeVariable(IceType_i32);
    ImplicitRetVec->setName(Func, "ImplicitRet_v4f32");
    ImplicitRetVec->setIsArg();
    Args.insert(Args.begin(), ImplicitRetVec);
    setImplicitRet(ImplicitRetVec);
  }

  for (SizeT i = 0, E = Args.size(); i < E; ++i) {
    Variable *Arg = Args[i];
    Type Ty = Arg->getType();
    RegNumT RegNum;
    if (!CC.argInReg(Ty, i, &RegNum)) {
      continue;
    }
    Variable *RegisterArg = Func->makeVariable(Ty);
    if (BuildDefs::dump()) {
      RegisterArg->setName(Func, "home_reg:" + Arg->getName());
    }
    RegisterArg->setIsArg();
    Arg->setIsArg(false);
    Args[i] = RegisterArg;

    if (isVectorType(Ty)) {
      auto *RegisterArgVec = llvm::cast<VariableVecOn32>(RegisterArg);
      RegisterArgVec->initVecElement(Func);
      RegisterArgVec->getContainers()[0]->setRegNum(
          RegNumT::fixme((unsigned)RegNum + 0));
      RegisterArgVec->getContainers()[1]->setRegNum(
          RegNumT::fixme((unsigned)RegNum + 1));
      // First two elements of second vector argument are passed
      // in $6:$7 and remaining two on stack. Do not assign register
      // to this is second vector argument.
      if (i == 0) {
        RegisterArgVec->getContainers()[2]->setRegNum(
            RegNumT::fixme((unsigned)RegNum + 2));
        RegisterArgVec->getContainers()[3]->setRegNum(
            RegNumT::fixme((unsigned)RegNum + 3));
      } else {
        RegisterArgVec->getContainers()[2]->setRegNum(
            RegNumT::fixme(RegNumT()));
        RegisterArgVec->getContainers()[3]->setRegNum(
            RegNumT::fixme(RegNumT()));
      }
    } else {
      switch (Ty) {
      default: {
        RegisterArg->setRegNum(RegNum);
      } break;
      case IceType_i64: {
        auto *RegisterArg64 = llvm::cast<Variable64On32>(RegisterArg);
        RegisterArg64->initHiLo(Func);
        RegisterArg64->getLo()->setRegNum(
            RegNumT::fixme(RegMIPS32::get64PairFirstRegNum(RegNum)));
        RegisterArg64->getHi()->setRegNum(
            RegNumT::fixme(RegMIPS32::get64PairSecondRegNum(RegNum)));
      } break;
      }
    }
    Context.insert<InstAssign>(Arg, RegisterArg);
  }

  // Insert fake use of ImplicitRet_v4f32 to keep it live
  if (ImplicitRetVec) {
    for (CfgNode *Node : Func->getNodes()) {
      for (Inst &Instr : Node->getInsts()) {
        if (llvm::isa<InstRet>(&Instr)) {
          Context.setInsertPoint(instToIterator(&Instr));
          Context.insert<InstFakeUse>(ImplicitRetVec);
          break;
        }
      }
    }
  }
}

Type TargetMIPS32::stackSlotType() { return IceType_i32; }

// Helper function for addProlog().
//
// This assumes Arg is an argument passed on the stack. This sets the frame
// offset for Arg and updates InArgsSizeBytes according to Arg's width. For an
// I64 arg that has been split into Lo and Hi components, it calls itself
// recursively on the components, taking care to handle Lo first because of the
// little-endian architecture. Lastly, this function generates an instruction
// to copy Arg into its assigned register if applicable.
void TargetMIPS32::finishArgumentLowering(Variable *Arg, bool PartialOnStack,
                                          Variable *FramePtr,
                                          size_t BasicFrameOffset,
                                          size_t *InArgsSizeBytes) {
  const Type Ty = Arg->getType();
  *InArgsSizeBytes = applyStackAlignmentTy(*InArgsSizeBytes, Ty);

  // If $4 is used for any scalar type (or returining v4f32) then the next
  // vector type if passed in $6:$7:stack:stack. Load 3nd and 4th element
  // from agument stack.
  if (auto *ArgVecOn32 = llvm::dyn_cast<VariableVecOn32>(Arg)) {
    if (PartialOnStack == false) {
      auto *Elem0 = ArgVecOn32->getContainers()[0];
      auto *Elem1 = ArgVecOn32->getContainers()[1];
      finishArgumentLowering(Elem0, PartialOnStack, FramePtr, BasicFrameOffset,
                             InArgsSizeBytes);
      finishArgumentLowering(Elem1, PartialOnStack, FramePtr, BasicFrameOffset,
                             InArgsSizeBytes);
    }
    auto *Elem2 = ArgVecOn32->getContainers()[2];
    auto *Elem3 = ArgVecOn32->getContainers()[3];
    finishArgumentLowering(Elem2, PartialOnStack, FramePtr, BasicFrameOffset,
                           InArgsSizeBytes);
    finishArgumentLowering(Elem3, PartialOnStack, FramePtr, BasicFrameOffset,
                           InArgsSizeBytes);
    return;
  }

  if (auto *Arg64On32 = llvm::dyn_cast<Variable64On32>(Arg)) {
    Variable *const Lo = Arg64On32->getLo();
    Variable *const Hi = Arg64On32->getHi();
    finishArgumentLowering(Lo, PartialOnStack, FramePtr, BasicFrameOffset,
                           InArgsSizeBytes);
    finishArgumentLowering(Hi, PartialOnStack, FramePtr, BasicFrameOffset,
                           InArgsSizeBytes);
    return;
  }

  assert(Ty != IceType_i64);
  assert(!isVectorType(Ty));

  const int32_t ArgStackOffset = BasicFrameOffset + *InArgsSizeBytes;
  *InArgsSizeBytes += typeWidthInBytesOnStack(Ty);

  if (!Arg->hasReg()) {
    Arg->setStackOffset(ArgStackOffset);
    return;
  }

  // If the argument variable has been assigned a register, we need to copy the
  // value from the stack slot.
  Variable *Parameter = Func->makeVariable(Ty);
  Parameter->setMustNotHaveReg();
  Parameter->setStackOffset(ArgStackOffset);
  _mov(Arg, Parameter);
}

void TargetMIPS32::addProlog(CfgNode *Node) {
  // Stack frame layout:
  //
  // +------------------------+
  // | 1. preserved registers |
  // +------------------------+
  // | 2. padding             |
  // +------------------------+
  // | 3. global spill area   |
  // +------------------------+
  // | 4. padding             |
  // +------------------------+
  // | 5. local spill area    |
  // +------------------------+
  // | 6. padding             |
  // +------------------------+
  // | 7. allocas             |
  // +------------------------+
  // | 8. padding             |
  // +------------------------+
  // | 9. out args            |
  // +------------------------+ <--- StackPointer
  //
  // The following variables record the size in bytes of the given areas:
  //  * PreservedRegsSizeBytes: area 1
  //  * SpillAreaPaddingBytes:  area 2
  //  * GlobalsSize:            area 3
  //  * GlobalsAndSubsequentPaddingSize: areas 3 - 4
  //  * LocalsSpillAreaSize:    area 5
  //  * SpillAreaSizeBytes:     areas 2 - 9
  //  * maxOutArgsSizeBytes():  area 9

  Context.init(Node);
  Context.setInsertPoint(Context.getCur());

  SmallBitVector CalleeSaves = getRegisterSet(RegSet_CalleeSave, RegSet_None);
  RegsUsed = SmallBitVector(CalleeSaves.size());

  VarList SortedSpilledVariables;

  size_t GlobalsSize = 0;
  // If there is a separate locals area, this represents that area. Otherwise
  // it counts any variable not counted by GlobalsSize.
  SpillAreaSizeBytes = 0;
  // If there is a separate locals area, this specifies the alignment for it.
  uint32_t LocalsSlotsAlignmentBytes = 0;
  // The entire spill locations area gets aligned to largest natural alignment
  // of the variables that have a spill slot.
  uint32_t SpillAreaAlignmentBytes = 0;
  // For now, we don't have target-specific variables that need special
  // treatment (no stack-slot-linked SpillVariable type).
  std::function<bool(Variable *)> TargetVarHook = [](Variable *Var) {
    static constexpr bool AssignStackSlot = false;
    static constexpr bool DontAssignStackSlot = !AssignStackSlot;
    if (llvm::isa<Variable64On32>(Var)) {
      return DontAssignStackSlot;
    }
    return AssignStackSlot;
  };

  // Compute the list of spilled variables and bounds for GlobalsSize, etc.
  getVarStackSlotParams(SortedSpilledVariables, RegsUsed, &GlobalsSize,
                        &SpillAreaSizeBytes, &SpillAreaAlignmentBytes,
                        &LocalsSlotsAlignmentBytes, TargetVarHook);
  uint32_t LocalsSpillAreaSize = SpillAreaSizeBytes;
  SpillAreaSizeBytes += GlobalsSize;

  PreservedGPRs.reserve(CalleeSaves.size());

  // Consider FP and RA as callee-save / used as needed.
  if (UsesFramePointer) {
    if (RegsUsed[RegMIPS32::Reg_FP]) {
      llvm::report_fatal_error("Frame pointer has been used.");
    }
    CalleeSaves[RegMIPS32::Reg_FP] = true;
    RegsUsed[RegMIPS32::Reg_FP] = true;
  }
  if (!MaybeLeafFunc) {
    CalleeSaves[RegMIPS32::Reg_RA] = true;
    RegsUsed[RegMIPS32::Reg_RA] = true;
  }

  // Make two passes over the used registers. The first pass records all the
  // used registers -- and their aliases. Then, we figure out which GPR
  // registers should be saved.
  SmallBitVector ToPreserve(RegMIPS32::Reg_NUM);
  for (SizeT i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      ToPreserve |= RegisterAliases[i];
    }
  }

  uint32_t NumCallee = 0;

  // RegClasses is a tuple of
  //
  // <First Register in Class, Last Register in Class, Vector of Save Registers>
  //
  // We use this tuple to figure out which register we should save/restore
  // during
  // prolog/epilog.
  using RegClassType = std::tuple<uint32_t, uint32_t, VarList *>;
  const RegClassType RegClass = RegClassType(
      RegMIPS32::Reg_GPR_First, RegMIPS32::Reg_FPR_Last, &PreservedGPRs);
  const uint32_t FirstRegInClass = std::get<0>(RegClass);
  const uint32_t LastRegInClass = std::get<1>(RegClass);
  VarList *const PreservedRegsInClass = std::get<2>(RegClass);
  for (uint32_t Reg = LastRegInClass; Reg > FirstRegInClass; Reg--) {
    if (!ToPreserve[Reg]) {
      continue;
    }
    ++NumCallee;
    Variable *PhysicalRegister = getPhysicalRegister(RegNumT::fromInt(Reg));
    PreservedRegsSizeBytes +=
        typeWidthInBytesOnStack(PhysicalRegister->getType());
    PreservedRegsInClass->push_back(PhysicalRegister);
  }

  Ctx->statsUpdateRegistersSaved(NumCallee);

  // Align the variables area. SpillAreaPaddingBytes is the size of the region
  // after the preserved registers and before the spill areas.
  // LocalsSlotsPaddingBytes is the amount of padding between the globals and
  // locals area if they are separate.
  assert(SpillAreaAlignmentBytes <= MIPS32_STACK_ALIGNMENT_BYTES);
  (void)MIPS32_STACK_ALIGNMENT_BYTES;
  assert(LocalsSlotsAlignmentBytes <= SpillAreaAlignmentBytes);
  uint32_t SpillAreaPaddingBytes = 0;
  uint32_t LocalsSlotsPaddingBytes = 0;
  alignStackSpillAreas(PreservedRegsSizeBytes, SpillAreaAlignmentBytes,
                       GlobalsSize, LocalsSlotsAlignmentBytes,
                       &SpillAreaPaddingBytes, &LocalsSlotsPaddingBytes);
  SpillAreaSizeBytes += SpillAreaPaddingBytes + LocalsSlotsPaddingBytes;
  uint32_t GlobalsAndSubsequentPaddingSize =
      GlobalsSize + LocalsSlotsPaddingBytes;

  // Adds the out args space to the stack, and align SP if necessary.
  if (!NeedsStackAlignment) {
    SpillAreaSizeBytes += MaxOutArgsSizeBytes * (VariableAllocaUsed ? 0 : 1);
  } else {
    SpillAreaSizeBytes = applyStackAlignment(
        SpillAreaSizeBytes +
        (VariableAllocaUsed ? VariableAllocaAlignBytes : MaxOutArgsSizeBytes));
  }

  // Combine fixed alloca with SpillAreaSize.
  SpillAreaSizeBytes += FixedAllocaSizeBytes;

  TotalStackSizeBytes =
      applyStackAlignment(PreservedRegsSizeBytes + SpillAreaSizeBytes);

  // Generate "addiu sp, sp, -TotalStackSizeBytes"
  if (TotalStackSizeBytes) {
    // Use the scratch register if needed to legalize the immediate.
    Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
    _addiu(SP, SP, -TotalStackSizeBytes);
  }

  Ctx->statsUpdateFrameBytes(TotalStackSizeBytes);

  if (!PreservedGPRs.empty()) {
    uint32_t StackOffset = TotalStackSizeBytes;
    for (Variable *Var : *PreservedRegsInClass) {
      Type RegType;
      if (RegMIPS32::isFPRReg(Var->getRegNum()))
        RegType = IceType_f32;
      else
        RegType = IceType_i32;
      auto *PhysicalRegister = makeReg(RegType, Var->getRegNum());
      StackOffset -= typeWidthInBytesOnStack(RegType);
      Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
      OperandMIPS32Mem *MemoryLocation = OperandMIPS32Mem::create(
          Func, RegType, SP,
          llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(StackOffset)));
      _sw(PhysicalRegister, MemoryLocation);
    }
  }

  Variable *FP = getPhysicalRegister(RegMIPS32::Reg_FP);

  // Generate "mov FP, SP" if needed.
  if (UsesFramePointer) {
    Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
    _mov(FP, SP);
    // Keep FP live for late-stage liveness analysis (e.g. asm-verbose mode).
    Context.insert<InstFakeUse>(FP);
  }

  // Fill in stack offsets for stack args, and copy args into registers for
  // those that were register-allocated. Args are pushed right to left, so
  // Arg[0] is closest to the stack/frame pointer.
  const VarList &Args = Func->getArgs();
  size_t InArgsSizeBytes = MIPS32_MAX_GPR_ARG * 4;
  TargetMIPS32::CallingConv CC;
  uint32_t ArgNo = 0;

  for (Variable *Arg : Args) {
    RegNumT DummyReg;
    const Type Ty = Arg->getType();
    bool PartialOnStack;
    // Skip arguments passed in registers.
    if (CC.argInReg(Ty, ArgNo, &DummyReg)) {
      // Load argument from stack:
      // 1. If this is first vector argument and return type is v4f32.
      //    In this case $4 is used to pass stack address implicitly.
      //    3rd and 4th element of vector argument is passed through stack.
      // 2. If this is second vector argument.
      if (ArgNo != 0 && isVectorType(Ty)) {
        PartialOnStack = true;
        finishArgumentLowering(Arg, PartialOnStack, FP, TotalStackSizeBytes,
                               &InArgsSizeBytes);
      }
    } else {
      PartialOnStack = false;
      finishArgumentLowering(Arg, PartialOnStack, FP, TotalStackSizeBytes,
                             &InArgsSizeBytes);
    }
    ++ArgNo;
  }

  // Fill in stack offsets for locals.
  assignVarStackSlots(SortedSpilledVariables, SpillAreaPaddingBytes,
                      SpillAreaSizeBytes, GlobalsAndSubsequentPaddingSize);
  this->HasComputedFrame = true;

  if (BuildDefs::dump() && Func->isVerbose(IceV_Frame)) {
    OstreamLocker _(Func->getContext());
    Ostream &Str = Func->getContext()->getStrDump();

    Str << "Stack layout:\n";
    uint32_t SPAdjustmentPaddingSize =
        SpillAreaSizeBytes - LocalsSpillAreaSize -
        GlobalsAndSubsequentPaddingSize - SpillAreaPaddingBytes -
        MaxOutArgsSizeBytes;
    Str << " in-args = " << InArgsSizeBytes << " bytes\n"
        << " preserved registers = " << PreservedRegsSizeBytes << " bytes\n"
        << " spill area padding = " << SpillAreaPaddingBytes << " bytes\n"
        << " globals spill area = " << GlobalsSize << " bytes\n"
        << " globals-locals spill areas intermediate padding = "
        << GlobalsAndSubsequentPaddingSize - GlobalsSize << " bytes\n"
        << " locals spill area = " << LocalsSpillAreaSize << " bytes\n"
        << " SP alignment padding = " << SPAdjustmentPaddingSize << " bytes\n";

    Str << "Stack details:\n"
        << " SP adjustment = " << SpillAreaSizeBytes << " bytes\n"
        << " spill area alignment = " << SpillAreaAlignmentBytes << " bytes\n"
        << " outgoing args size = " << MaxOutArgsSizeBytes << " bytes\n"
        << " locals spill area alignment = " << LocalsSlotsAlignmentBytes
        << " bytes\n"
        << " is FP based = " << 1 << "\n";
  }
  return;
}

void TargetMIPS32::addEpilog(CfgNode *Node) {
  InstList &Insts = Node->getInsts();
  InstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<InstMIPS32Ret>(*RI))
      break;
  }
  if (RI == E)
    return;

  // Convert the reverse_iterator position into its corresponding (forward)
  // iterator position.
  InstList::iterator InsertPoint = reverseToForwardIterator(RI);
  --InsertPoint;
  Context.init(Node);
  Context.setInsertPoint(InsertPoint);

  Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
  if (UsesFramePointer) {
    Variable *FP = getPhysicalRegister(RegMIPS32::Reg_FP);
    // For late-stage liveness analysis (e.g. asm-verbose mode), adding a fake
    // use of SP before the assignment of SP=FP keeps previous SP adjustments
    // from being dead-code eliminated.
    Context.insert<InstFakeUse>(SP);
    _mov(SP, FP);
  }

  VarList::reverse_iterator RIter, END;

  if (!PreservedGPRs.empty()) {
    uint32_t StackOffset = TotalStackSizeBytes - PreservedRegsSizeBytes;
    for (RIter = PreservedGPRs.rbegin(), END = PreservedGPRs.rend();
         RIter != END; ++RIter) {
      Type RegType;
      if (RegMIPS32::isFPRReg((*RIter)->getRegNum()))
        RegType = IceType_f32;
      else
        RegType = IceType_i32;
      auto *PhysicalRegister = makeReg(RegType, (*RIter)->getRegNum());
      Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
      OperandMIPS32Mem *MemoryLocation = OperandMIPS32Mem::create(
          Func, RegType, SP,
          llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(StackOffset)));
      _lw(PhysicalRegister, MemoryLocation);
      StackOffset += typeWidthInBytesOnStack(PhysicalRegister->getType());
    }
  }

  if (TotalStackSizeBytes) {
    _addiu(SP, SP, TotalStackSizeBytes);
  }
}

Variable *TargetMIPS32::PostLoweringLegalizer::newBaseRegister(
    Variable *Base, int32_t Offset, RegNumT ScratchRegNum) {
  // Legalize will likely need a lui/ori combination, but if the top bits are
  // all 0 from negating the offset and subtracting, we could use that instead.
  const bool ShouldSub = Offset != 0 && (-Offset & 0xFFFF0000) == 0;
  Variable *ScratchReg = Target->makeReg(IceType_i32, ScratchRegNum);
  if (ShouldSub) {
    Target->_addi(ScratchReg, Base, -Offset);
  } else {
    constexpr bool SignExt = true;
    if (!OperandMIPS32Mem::canHoldOffset(Base->getType(), SignExt, Offset)) {
      const uint32_t UpperBits = (Offset >> 16) & 0xFFFF;
      const uint32_t LowerBits = Offset & 0xFFFF;
      Target->_lui(ScratchReg, Target->Ctx->getConstantInt32(UpperBits));
      if (LowerBits)
        Target->_ori(ScratchReg, ScratchReg, LowerBits);
      Target->_addu(ScratchReg, ScratchReg, Base);
    } else {
      Target->_addiu(ScratchReg, Base, Offset);
    }
  }

  return ScratchReg;
}

void TargetMIPS32::PostLoweringLegalizer::legalizeMovFp(
    InstMIPS32MovFP64ToI64 *MovInstr) {
  Variable *Dest = MovInstr->getDest();
  Operand *Src = MovInstr->getSrc(0);
  const Type SrcTy = Src->getType();

  if (Dest != nullptr && SrcTy == IceType_f64) {
    int32_t Offset = Dest->getStackOffset();
    auto *Base = Target->getPhysicalRegister(Target->getFrameOrStackReg());
    OperandMIPS32Mem *TAddr = OperandMIPS32Mem::create(
        Target->Func, IceType_f32, Base,
        llvm::cast<ConstantInteger32>(Target->Ctx->getConstantInt32(Offset)));
    OperandMIPS32Mem *Addr = legalizeMemOperand(TAddr);
    auto *SrcV = llvm::cast<Variable>(Src);
    Variable *SrcR;
    if (MovInstr->getInt64Part() == Int64_Lo) {
      SrcR = Target->makeReg(
          IceType_f32, RegMIPS32::get64PairFirstRegNum(SrcV->getRegNum()));
    } else {
      SrcR = Target->makeReg(
          IceType_f32, RegMIPS32::get64PairSecondRegNum(SrcV->getRegNum()));
    }
    Target->_sw(SrcR, Addr);
    if (MovInstr->isDestRedefined()) {
      Target->_set_dest_redefined();
    }
    MovInstr->setDeleted();
    return;
  }

  llvm::report_fatal_error("legalizeMovFp: Invalid operands");
}

void TargetMIPS32::PostLoweringLegalizer::legalizeMov(InstMIPS32Mov *MovInstr) {
  Variable *Dest = MovInstr->getDest();
  assert(Dest != nullptr);
  const Type DestTy = Dest->getType();
  assert(DestTy != IceType_i64);

  Operand *Src = MovInstr->getSrc(0);
  const Type SrcTy = Src->getType();
  (void)SrcTy;
  assert(SrcTy != IceType_i64);

  bool Legalized = false;
  auto *SrcR = llvm::cast<Variable>(Src);
  if (Dest->hasReg() && SrcR->hasReg()) {
    // This might be a GP to/from FP move generated due to argument passing.
    // Use mtc1/mfc1 instead of mov.[s/d] if src and dst registers are of
    // different types.
    const bool IsDstGPR = RegMIPS32::isGPRReg(Dest->getRegNum());
    const bool IsSrcGPR = RegMIPS32::isGPRReg(SrcR->getRegNum());
    const RegNumT SRegNum = SrcR->getRegNum();
    const RegNumT DRegNum = Dest->getRegNum();
    if (IsDstGPR != IsSrcGPR) {
      if (IsDstGPR) {
        // Dest is GPR and SrcR is FPR. Use mfc1.
        int32_t TypeWidth = typeWidthInBytes(DestTy);
        if (MovInstr->getDestHi() != nullptr)
          TypeWidth += typeWidthInBytes(MovInstr->getDestHi()->getType());
        if (TypeWidth == 8) {
          // Split it into two mfc1 instructions
          Variable *SrcGPRHi = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairFirstRegNum(SRegNum));
          Variable *SrcGPRLo = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairSecondRegNum(SRegNum));
          Variable *DstFPRHi, *DstFPRLo;
          if (MovInstr->getDestHi() != nullptr && Dest != nullptr) {
            DstFPRHi = Target->makeReg(IceType_i32,
                                       MovInstr->getDestHi()->getRegNum());
            DstFPRLo = Target->makeReg(IceType_i32, Dest->getRegNum());
          } else {
            DstFPRHi = Target->makeReg(
                IceType_i32, RegMIPS32::get64PairFirstRegNum(DRegNum));
            DstFPRLo = Target->makeReg(
                IceType_i32, RegMIPS32::get64PairSecondRegNum(DRegNum));
          }
          Target->_mov(DstFPRHi, SrcGPRHi);
          Target->_mov(DstFPRLo, SrcGPRLo);
          Legalized = true;
        } else {
          Variable *SrcGPR = Target->makeReg(IceType_f32, SRegNum);
          Variable *DstFPR = Target->makeReg(IceType_i32, DRegNum);
          Target->_mov(DstFPR, SrcGPR);
          Legalized = true;
        }
      } else {
        // Dest is FPR and SrcR is GPR. Use mtc1.
        if (typeWidthInBytes(Dest->getType()) == 8) {
          Variable *SrcGPRHi, *SrcGPRLo;
          // SrcR could be $zero which is i32
          if (SRegNum == RegMIPS32::Reg_ZERO) {
            SrcGPRHi = Target->makeReg(IceType_i32, SRegNum);
            SrcGPRLo = SrcGPRHi;
          } else {
            // Split it into two mtc1 instructions
            if (MovInstr->getSrcSize() == 2) {
              const auto FirstReg =
                  (llvm::cast<Variable>(MovInstr->getSrc(0)))->getRegNum();
              const auto SecondReg =
                  (llvm::cast<Variable>(MovInstr->getSrc(1)))->getRegNum();
              SrcGPRHi = Target->makeReg(IceType_i32, FirstReg);
              SrcGPRLo = Target->makeReg(IceType_i32, SecondReg);
            } else {
              SrcGPRLo = Target->makeReg(
                  IceType_i32, RegMIPS32::get64PairFirstRegNum(SRegNum));
              SrcGPRHi = Target->makeReg(
                  IceType_i32, RegMIPS32::get64PairSecondRegNum(SRegNum));
            }
          }
          Variable *DstFPRHi = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairFirstRegNum(DRegNum));
          Variable *DstFPRLo = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairSecondRegNum(DRegNum));
          Target->_mov(DstFPRHi, SrcGPRLo);
          Target->_mov(DstFPRLo, SrcGPRHi);
          Legalized = true;
        } else {
          Variable *SrcGPR = Target->makeReg(IceType_i32, SRegNum);
          Variable *DstFPR = Target->makeReg(IceType_f32, DRegNum);
          Target->_mov(DstFPR, SrcGPR);
          Legalized = true;
        }
      }
    }
    if (Legalized) {
      if (MovInstr->isDestRedefined()) {
        Target->_set_dest_redefined();
      }
      MovInstr->setDeleted();
      return;
    }
  }

  if (!Dest->hasReg()) {
    auto *SrcR = llvm::cast<Variable>(Src);
    assert(SrcR->hasReg());
    assert(!SrcR->isRematerializable());
    int32_t Offset = Dest->getStackOffset();

    // This is a _mov(Mem(), Variable), i.e., a store.
    auto *Base = Target->getPhysicalRegister(Target->getFrameOrStackReg());

    OperandMIPS32Mem *TAddr = OperandMIPS32Mem::create(
        Target->Func, DestTy, Base,
        llvm::cast<ConstantInteger32>(Target->Ctx->getConstantInt32(Offset)));
    OperandMIPS32Mem *TAddrHi = OperandMIPS32Mem::create(
        Target->Func, DestTy, Base,
        llvm::cast<ConstantInteger32>(
            Target->Ctx->getConstantInt32(Offset + 4)));
    OperandMIPS32Mem *Addr = legalizeMemOperand(TAddr);

    // FP arguments are passed in GP reg if first argument is in GP. In this
    // case type of the SrcR is still FP thus we need to explicitly generate sw
    // instead of swc1.
    const RegNumT RegNum = SrcR->getRegNum();
    const bool IsSrcGPReg = RegMIPS32::isGPRReg(SrcR->getRegNum());
    if (SrcTy == IceType_f32 && IsSrcGPReg) {
      Variable *SrcGPR = Target->makeReg(IceType_i32, RegNum);
      Target->_sw(SrcGPR, Addr);
    } else if (SrcTy == IceType_f64 && IsSrcGPReg) {
      Variable *SrcGPRHi =
          Target->makeReg(IceType_i32, RegMIPS32::get64PairFirstRegNum(RegNum));
      Variable *SrcGPRLo = Target->makeReg(
          IceType_i32, RegMIPS32::get64PairSecondRegNum(RegNum));
      Target->_sw(SrcGPRHi, Addr);
      OperandMIPS32Mem *AddrHi = legalizeMemOperand(TAddrHi);
      Target->_sw(SrcGPRLo, AddrHi);
    } else if (DestTy == IceType_f64 && IsSrcGPReg) {
      const auto FirstReg =
          (llvm::cast<Variable>(MovInstr->getSrc(0)))->getRegNum();
      const auto SecondReg =
          (llvm::cast<Variable>(MovInstr->getSrc(1)))->getRegNum();
      Variable *SrcGPRHi = Target->makeReg(IceType_i32, FirstReg);
      Variable *SrcGPRLo = Target->makeReg(IceType_i32, SecondReg);
      Target->_sw(SrcGPRLo, Addr);
      OperandMIPS32Mem *AddrHi = legalizeMemOperand(TAddrHi);
      Target->_sw(SrcGPRHi, AddrHi);
    } else {
      Target->_sw(SrcR, Addr);
    }

    Target->Context.insert<InstFakeDef>(Dest);
    Legalized = true;
  } else if (auto *Var = llvm::dyn_cast<Variable>(Src)) {
    if (Var->isRematerializable()) {
      // This is equivalent to an x86 _lea(RematOffset(%esp/%ebp), Variable).

      // ExtraOffset is only needed for stack-pointer based frames as we have
      // to account for spill storage.
      const int32_t ExtraOffset =
          (Var->getRegNum() == Target->getFrameOrStackReg())
              ? Target->getFrameFixedAllocaOffset()
              : 0;

      const int32_t Offset = Var->getStackOffset() + ExtraOffset;
      Variable *Base = Target->getPhysicalRegister(Var->getRegNum());
      Variable *T = newBaseRegister(Base, Offset, Dest->getRegNum());
      Target->_mov(Dest, T);
      Legalized = true;
    } else {
      if (!Var->hasReg()) {
        // This is a _mov(Variable, Mem()), i.e., a load.
        const int32_t Offset = Var->getStackOffset();
        auto *Base = Target->getPhysicalRegister(Target->getFrameOrStackReg());
        const RegNumT RegNum = Dest->getRegNum();
        const bool IsDstGPReg = RegMIPS32::isGPRReg(Dest->getRegNum());
        // If we are moving i64 to a double using stack then the address may
        // not be aligned to 8-byte boundary as we split i64 into Hi-Lo parts
        // and store them individually with 4-byte alignment. Load the Hi-Lo
        // parts in TmpReg and move them to the dest using mtc1.
        if (DestTy == IceType_f64 && !Utils::IsAligned(Offset, 8) &&
            !IsDstGPReg) {
          auto *Reg = Target->makeReg(IceType_i32, Target->getReservedTmpReg());
          const RegNumT RegNum = Dest->getRegNum();
          Variable *DestLo = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairFirstRegNum(RegNum));
          Variable *DestHi = Target->makeReg(
              IceType_f32, RegMIPS32::get64PairSecondRegNum(RegNum));
          OperandMIPS32Mem *AddrLo = OperandMIPS32Mem::create(
              Target->Func, IceType_i32, Base,
              llvm::cast<ConstantInteger32>(
                  Target->Ctx->getConstantInt32(Offset)));
          OperandMIPS32Mem *AddrHi = OperandMIPS32Mem::create(
              Target->Func, IceType_i32, Base,
              llvm::cast<ConstantInteger32>(
                  Target->Ctx->getConstantInt32(Offset + 4)));
          Target->_lw(Reg, AddrLo);
          Target->_mov(DestLo, Reg);
          Target->_lw(Reg, AddrHi);
          Target->_mov(DestHi, Reg);
        } else {
          OperandMIPS32Mem *TAddr = OperandMIPS32Mem::create(
              Target->Func, DestTy, Base,
              llvm::cast<ConstantInteger32>(
                  Target->Ctx->getConstantInt32(Offset)));
          OperandMIPS32Mem *Addr = legalizeMemOperand(TAddr);
          OperandMIPS32Mem *TAddrHi = OperandMIPS32Mem::create(
              Target->Func, DestTy, Base,
              llvm::cast<ConstantInteger32>(
                  Target->Ctx->getConstantInt32(Offset + 4)));
          // FP arguments are passed in GP reg if first argument is in GP.
          // In this case type of the Dest is still FP thus we need to
          // explicitly generate lw instead of lwc1.
          if (DestTy == IceType_f32 && IsDstGPReg) {
            Variable *DstGPR = Target->makeReg(IceType_i32, RegNum);
            Target->_lw(DstGPR, Addr);
          } else if (DestTy == IceType_f64 && IsDstGPReg) {
            Variable *DstGPRHi = Target->makeReg(
                IceType_i32, RegMIPS32::get64PairFirstRegNum(RegNum));
            Variable *DstGPRLo = Target->makeReg(
                IceType_i32, RegMIPS32::get64PairSecondRegNum(RegNum));
            Target->_lw(DstGPRHi, Addr);
            OperandMIPS32Mem *AddrHi = legalizeMemOperand(TAddrHi);
            Target->_lw(DstGPRLo, AddrHi);
          } else if (DestTy == IceType_f64 && IsDstGPReg) {
            const auto FirstReg =
                (llvm::cast<Variable>(MovInstr->getSrc(0)))->getRegNum();
            const auto SecondReg =
                (llvm::cast<Variable>(MovInstr->getSrc(1)))->getRegNum();
            Variable *DstGPRHi = Target->makeReg(IceType_i32, FirstReg);
            Variable *DstGPRLo = Target->makeReg(IceType_i32, SecondReg);
            Target->_lw(DstGPRLo, Addr);
            OperandMIPS32Mem *AddrHi = legalizeMemOperand(TAddrHi);
            Target->_lw(DstGPRHi, AddrHi);
          } else {
            Target->_lw(Dest, Addr);
          }
        }
        Legalized = true;
      }
    }
  }

  if (Legalized) {
    if (MovInstr->isDestRedefined()) {
      Target->_set_dest_redefined();
    }
    MovInstr->setDeleted();
  }
}

OperandMIPS32Mem *
TargetMIPS32::PostLoweringLegalizer::legalizeMemOperand(OperandMIPS32Mem *Mem) {
  if (llvm::isa<ConstantRelocatable>(Mem->getOffset())) {
    return nullptr;
  }
  Variable *Base = Mem->getBase();
  auto *Ci32 = llvm::cast<ConstantInteger32>(Mem->getOffset());
  int32_t Offset = Ci32->getValue();

  if (Base->isRematerializable()) {
    const int32_t ExtraOffset =
        (Base->getRegNum() == Target->getFrameOrStackReg())
            ? Target->getFrameFixedAllocaOffset()
            : 0;
    Offset += Base->getStackOffset() + ExtraOffset;
    Base = Target->getPhysicalRegister(Base->getRegNum());
  }

  constexpr bool SignExt = true;
  if (!OperandMIPS32Mem::canHoldOffset(Mem->getType(), SignExt, Offset)) {
    Base = newBaseRegister(Base, Offset, Target->getReservedTmpReg());
    Offset = 0;
  }

  return OperandMIPS32Mem::create(
      Target->Func, Mem->getType(), Base,
      llvm::cast<ConstantInteger32>(Target->Ctx->getConstantInt32(Offset)));
}

Variable *TargetMIPS32::PostLoweringLegalizer::legalizeImmediate(int32_t Imm) {
  Variable *Reg = nullptr;
  if (!((std::numeric_limits<int16_t>::min() <= Imm) &&
        (Imm <= std::numeric_limits<int16_t>::max()))) {
    const uint32_t UpperBits = (Imm >> 16) & 0xFFFF;
    const uint32_t LowerBits = Imm & 0xFFFF;
    Variable *TReg = Target->makeReg(IceType_i32, Target->getReservedTmpReg());
    Reg = Target->makeReg(IceType_i32, Target->getReservedTmpReg());
    if (LowerBits) {
      Target->_lui(TReg, Target->Ctx->getConstantInt32(UpperBits));
      Target->_ori(Reg, TReg, LowerBits);
    } else {
      Target->_lui(Reg, Target->Ctx->getConstantInt32(UpperBits));
    }
  }
  return Reg;
}

void TargetMIPS32::postLowerLegalization() {
  Func->dump("Before postLowerLegalization");
  assert(hasComputedFrame());
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    PostLoweringLegalizer Legalizer(this);
    while (!Context.atEnd()) {
      PostIncrLoweringContext PostIncrement(Context);
      Inst *CurInstr = iteratorToInst(Context.getCur());
      const SizeT NumSrcs = CurInstr->getSrcSize();
      Operand *Src0 = NumSrcs < 1 ? nullptr : CurInstr->getSrc(0);
      Operand *Src1 = NumSrcs < 2 ? nullptr : CurInstr->getSrc(1);
      auto *Src0V = llvm::dyn_cast_or_null<Variable>(Src0);
      auto *Src0M = llvm::dyn_cast_or_null<OperandMIPS32Mem>(Src0);
      auto *Src1M = llvm::dyn_cast_or_null<OperandMIPS32Mem>(Src1);
      Variable *Dst = CurInstr->getDest();
      if (auto *MovInstr = llvm::dyn_cast<InstMIPS32Mov>(CurInstr)) {
        Legalizer.legalizeMov(MovInstr);
        continue;
      }
      if (auto *MovInstr = llvm::dyn_cast<InstMIPS32MovFP64ToI64>(CurInstr)) {
        Legalizer.legalizeMovFp(MovInstr);
        continue;
      }
      if (llvm::isa<InstMIPS32Sw>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src1M)) {
          _sw(Src0V, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (llvm::isa<InstMIPS32Swc1>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src1M)) {
          _swc1(Src0V, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (llvm::isa<InstMIPS32Sdc1>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src1M)) {
          _sdc1(Src0V, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (llvm::isa<InstMIPS32Lw>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src0M)) {
          _lw(Dst, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (llvm::isa<InstMIPS32Lwc1>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src0M)) {
          _lwc1(Dst, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (llvm::isa<InstMIPS32Ldc1>(CurInstr)) {
        if (auto *LegalMem = Legalizer.legalizeMemOperand(Src0M)) {
          _ldc1(Dst, LegalMem);
          CurInstr->setDeleted();
        }
        continue;
      }
      if (auto *AddiuInstr = llvm::dyn_cast<InstMIPS32Addiu>(CurInstr)) {
        if (auto *LegalImm = Legalizer.legalizeImmediate(
                static_cast<int32_t>(AddiuInstr->getImmediateValue()))) {
          _addu(Dst, Src0V, LegalImm);
          CurInstr->setDeleted();
        }
        continue;
      }
    }
  }
}

Operand *TargetMIPS32::loOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getLo();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand)) {
    return Ctx->getConstantInt32(static_cast<uint32_t>(Const->getValue()));
  }
  if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(Operand)) {
    // Conservatively disallow memory operands with side-effects (pre/post
    // increment) in case of duplication.
    assert(Mem->getAddrMode() == OperandMIPS32Mem::Offset);
    return OperandMIPS32Mem::create(Func, IceType_i32, Mem->getBase(),
                                    Mem->getOffset(), Mem->getAddrMode());
  }
  llvm_unreachable("Unsupported operand type");
  return nullptr;
}

Operand *TargetMIPS32::getOperandAtIndex(Operand *Operand, Type BaseType,
                                         uint32_t Index) {
  if (!isVectorType(Operand->getType())) {
    llvm::report_fatal_error("getOperandAtIndex: Operand is not vector");
    return nullptr;
  }

  if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(Operand)) {
    assert(Mem->getAddrMode() == OperandMIPS32Mem::Offset);
    Variable *Base = Mem->getBase();
    auto *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
    assert(!Utils::WouldOverflowAdd(Offset->getValue(), 4));
    int32_t NextOffsetVal =
        Offset->getValue() + (Index * typeWidthInBytes(BaseType));
    constexpr bool NoSignExt = false;
    if (!OperandMIPS32Mem::canHoldOffset(BaseType, NoSignExt, NextOffsetVal)) {
      Constant *_4 = Ctx->getConstantInt32(4);
      Variable *NewBase = Func->makeVariable(Base->getType());
      lowerArithmetic(
          InstArithmetic::create(Func, InstArithmetic::Add, NewBase, Base, _4));
      Base = NewBase;
    } else {
      Offset =
          llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(NextOffsetVal));
    }
    return OperandMIPS32Mem::create(Func, BaseType, Base, Offset,
                                    Mem->getAddrMode());
  }

  if (auto *VarVecOn32 = llvm::dyn_cast<VariableVecOn32>(Operand))
    return VarVecOn32->getContainers()[Index];

  llvm_unreachable("Unsupported operand type");
  return nullptr;
}

Operand *TargetMIPS32::hiOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getHi();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand)) {
    return Ctx->getConstantInt32(
        static_cast<uint32_t>(Const->getValue() >> 32));
  }
  if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(Operand)) {
    // Conservatively disallow memory operands with side-effects
    // in case of duplication.
    assert(Mem->getAddrMode() == OperandMIPS32Mem::Offset);
    const Type SplitType = IceType_i32;
    Variable *Base = Mem->getBase();
    auto *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
    assert(!Utils::WouldOverflowAdd(Offset->getValue(), 4));
    int32_t NextOffsetVal = Offset->getValue() + 4;
    constexpr bool SignExt = false;
    if (!OperandMIPS32Mem::canHoldOffset(SplitType, SignExt, NextOffsetVal)) {
      // We have to make a temp variable and add 4 to either Base or Offset.
      // If we add 4 to Offset, this will convert a non-RegReg addressing
      // mode into a RegReg addressing mode. Since NaCl sandboxing disallows
      // RegReg addressing modes, prefer adding to base and replacing instead.
      // Thus we leave the old offset alone.
      Constant *Four = Ctx->getConstantInt32(4);
      Variable *NewBase = Func->makeVariable(Base->getType());
      lowerArithmetic(InstArithmetic::create(Func, InstArithmetic::Add, NewBase,
                                             Base, Four));
      Base = NewBase;
    } else {
      Offset =
          llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(NextOffsetVal));
    }
    return OperandMIPS32Mem::create(Func, SplitType, Base, Offset,
                                    Mem->getAddrMode());
  }
  llvm_unreachable("Unsupported operand type");
  return nullptr;
}

SmallBitVector TargetMIPS32::getRegisterSet(RegSetMask Include,
                                            RegSetMask Exclude) const {
  SmallBitVector Registers(RegMIPS32::Reg_NUM);

#define X(val, encode, name, scratch, preserved, stackptr, frameptr, isInt,    \
          isI64Pair, isFP32, isFP64, isVec128, alias_init)                     \
  if (scratch && (Include & RegSet_CallerSave))                                \
    Registers[RegMIPS32::val] = true;                                          \
  if (preserved && (Include & RegSet_CalleeSave))                              \
    Registers[RegMIPS32::val] = true;                                          \
  if (stackptr && (Include & RegSet_StackPointer))                             \
    Registers[RegMIPS32::val] = true;                                          \
  if (frameptr && (Include & RegSet_FramePointer))                             \
    Registers[RegMIPS32::val] = true;                                          \
  if (scratch && (Exclude & RegSet_CallerSave))                                \
    Registers[RegMIPS32::val] = false;                                         \
  if (preserved && (Exclude & RegSet_CalleeSave))                              \
    Registers[RegMIPS32::val] = false;                                         \
  if (stackptr && (Exclude & RegSet_StackPointer))                             \
    Registers[RegMIPS32::val] = false;                                         \
  if (frameptr && (Exclude & RegSet_FramePointer))                             \
    Registers[RegMIPS32::val] = false;

  REGMIPS32_TABLE

#undef X

  return Registers;
}

void TargetMIPS32::lowerAlloca(const InstAlloca *Instr) {
  // Conservatively require the stack to be aligned. Some stack adjustment
  // operations implemented below assume that the stack is aligned before the
  // alloca. All the alloca code ensures that the stack alignment is preserved
  // after the alloca. The stack alignment restriction can be relaxed in some
  // cases.
  NeedsStackAlignment = true;

  // For default align=0, set it to the real value 1, to avoid any
  // bit-manipulation problems below.
  const uint32_t AlignmentParam = std::max(1u, Instr->getAlignInBytes());

  // LLVM enforces power of 2 alignment.
  assert(llvm::isPowerOf2_32(AlignmentParam));
  assert(llvm::isPowerOf2_32(MIPS32_STACK_ALIGNMENT_BYTES));

  const uint32_t Alignment =
      std::max(AlignmentParam, MIPS32_STACK_ALIGNMENT_BYTES);
  const bool OverAligned = Alignment > MIPS32_STACK_ALIGNMENT_BYTES;
  const bool OptM1 = Func->getOptLevel() == Opt_m1;
  const bool AllocaWithKnownOffset = Instr->getKnownFrameOffset();
  const bool UseFramePointer =
      hasFramePointer() || OverAligned || !AllocaWithKnownOffset || OptM1;

  if (UseFramePointer)
    setHasFramePointer();

  Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);

  Variable *Dest = Instr->getDest();
  Operand *TotalSize = Instr->getSizeInBytes();

  if (const auto *ConstantTotalSize =
          llvm::dyn_cast<ConstantInteger32>(TotalSize)) {
    const uint32_t Value =
        Utils::applyAlignment(ConstantTotalSize->getValue(), Alignment);
    FixedAllocaSizeBytes += Value;
    // Constant size alloca.
    if (!UseFramePointer) {
      // If we don't need a Frame Pointer, this alloca has a known offset to the
      // stack pointer. We don't need adjust the stack pointer, nor assign any
      // value to Dest, as Dest is rematerializable.
      assert(Dest->isRematerializable());
      Context.insert<InstFakeDef>(Dest);
      return;
    }

    if (Alignment > MIPS32_STACK_ALIGNMENT_BYTES) {
      CurrentAllocaOffset =
          Utils::applyAlignment(CurrentAllocaOffset, Alignment);
    }
    auto *T = I32Reg();
    _addiu(T, SP, CurrentAllocaOffset);
    _mov(Dest, T);
    CurrentAllocaOffset += Value;
    return;

  } else {
    // Non-constant sizes need to be adjusted to the next highest multiple of
    // the required alignment at runtime.
    VariableAllocaUsed = true;
    VariableAllocaAlignBytes = AlignmentParam;
    Variable *AlignAmount;
    auto *TotalSizeR = legalizeToReg(TotalSize, Legal_Reg);
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    _addiu(T1, TotalSizeR, MIPS32_STACK_ALIGNMENT_BYTES - 1);
    _addiu(T2, getZero(), -MIPS32_STACK_ALIGNMENT_BYTES);
    _and(T3, T1, T2);
    _subu(T4, SP, T3);
    if (Instr->getAlignInBytes()) {
      AlignAmount =
          legalizeToReg(Ctx->getConstantInt32(-AlignmentParam), Legal_Reg);
      _and(T5, T4, AlignAmount);
      _mov(Dest, T5);
    } else {
      _mov(Dest, T4);
    }
    _mov(SP, Dest);
    return;
  }
}

void TargetMIPS32::lowerInt64Arithmetic(const InstArithmetic *Instr,
                                        Variable *Dest, Operand *Src0,
                                        Operand *Src1) {
  InstArithmetic::OpKind Op = Instr->getOp();
  auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
  auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
  Variable *Src0LoR = nullptr;
  Variable *Src1LoR = nullptr;
  Variable *Src0HiR = nullptr;
  Variable *Src1HiR = nullptr;

  switch (Op) {
  case InstArithmetic::_num:
    llvm::report_fatal_error("Unknown arithmetic operator");
    return;
  case InstArithmetic::Add: {
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Carry = I32Reg(), *T_Lo = I32Reg(), *T_Hi = I32Reg(),
         *T_Hi2 = I32Reg();
    _addu(T_Lo, Src0LoR, Src1LoR);
    _mov(DestLo, T_Lo);
    _sltu(T_Carry, T_Lo, Src0LoR);
    _addu(T_Hi, T_Carry, Src0HiR);
    _addu(T_Hi2, Src1HiR, T_Hi);
    _mov(DestHi, T_Hi2);
    return;
  }
  case InstArithmetic::And: {
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Lo = I32Reg(), *T_Hi = I32Reg();
    _and(T_Lo, Src0LoR, Src1LoR);
    _mov(DestLo, T_Lo);
    _and(T_Hi, Src0HiR, Src1HiR);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Sub: {
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Borrow = I32Reg(), *T_Lo = I32Reg(), *T_Hi = I32Reg(),
         *T_Hi2 = I32Reg();
    _subu(T_Lo, Src0LoR, Src1LoR);
    _mov(DestLo, T_Lo);
    _sltu(T_Borrow, Src0LoR, Src1LoR);
    _addu(T_Hi, T_Borrow, Src1HiR);
    _subu(T_Hi2, Src0HiR, T_Hi);
    _mov(DestHi, T_Hi2);
    return;
  }
  case InstArithmetic::Or: {
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Lo = I32Reg(), *T_Hi = I32Reg();
    _or(T_Lo, Src0LoR, Src1LoR);
    _mov(DestLo, T_Lo);
    _or(T_Hi, Src0HiR, Src1HiR);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Xor: {
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Lo = I32Reg(), *T_Hi = I32Reg();
    _xor(T_Lo, Src0LoR, Src1LoR);
    _mov(DestLo, T_Lo);
    _xor(T_Hi, Src0HiR, Src1HiR);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Mul: {
    // TODO(rkotler): Make sure that mul has the side effect of clobbering
    // LO, HI. Check for any other LO, HI quirkiness in this section.
    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));
    Src1HiR = legalizeToReg(hiOperand(Src1));
    auto *T_Lo = I32Reg(RegMIPS32::Reg_LO), *T_Hi = I32Reg(RegMIPS32::Reg_HI);
    auto *T1 = I32Reg(), *T2 = I32Reg();
    auto *TM1 = I32Reg(), *TM2 = I32Reg(), *TM3 = I32Reg(), *TM4 = I32Reg();
    _multu(T_Lo, Src0LoR, Src1LoR);
    Context.insert<InstFakeDef>(T_Hi, T_Lo);
    _mflo(T1, T_Lo);
    _mfhi(T2, T_Hi);
    _mov(DestLo, T1);
    _mul(TM1, Src0HiR, Src1LoR);
    _mul(TM2, Src0LoR, Src1HiR);
    _addu(TM3, TM1, T2);
    _addu(TM4, TM3, TM2);
    _mov(DestHi, TM4);
    return;
  }
  case InstArithmetic::Shl: {
    auto *T_Lo = I32Reg();
    auto *T_Hi = I32Reg();
    auto *T1_Lo = I32Reg();
    auto *T1_Hi = I32Reg();
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();

    if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Src1)) {
      Src0LoR = legalizeToReg(loOperand(Src0));
      int64_t ShiftAmount = Const->getValue();
      if (ShiftAmount == 1) {
        Src0HiR = legalizeToReg(hiOperand(Src0));
        _addu(T_Lo, Src0LoR, Src0LoR);
        _sltu(T1, T_Lo, Src0LoR);
        _addu(T2, T1, Src0HiR);
        _addu(T_Hi, Src0HiR, T2);
      } else if (ShiftAmount < INT32_BITS) {
        Src0HiR = legalizeToReg(hiOperand(Src0));
        _srl(T1, Src0LoR, INT32_BITS - ShiftAmount);
        _sll(T2, Src0HiR, ShiftAmount);
        _or(T_Hi, T1, T2);
        _sll(T_Lo, Src0LoR, ShiftAmount);
      } else if (ShiftAmount == INT32_BITS) {
        _addiu(T_Lo, getZero(), 0);
        _mov(T_Hi, Src0LoR);
      } else if (ShiftAmount > INT32_BITS && ShiftAmount < 64) {
        _sll(T_Hi, Src0LoR, ShiftAmount - INT32_BITS);
        _addiu(T_Lo, getZero(), 0);
      }
      _mov(DestLo, T_Lo);
      _mov(DestHi, T_Hi);
      return;
    }

    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));

    _sllv(T1, Src0HiR, Src1LoR);
    _not(T2, Src1LoR);
    _srl(T3, Src0LoR, 1);
    _srlv(T4, T3, T2);
    _or(T_Hi, T1, T4);
    _sllv(T_Lo, Src0LoR, Src1LoR);

    _mov(T1_Hi, T_Hi);
    _mov(T1_Lo, T_Lo);
    _andi(T5, Src1LoR, INT32_BITS);
    _movn(T1_Hi, T_Lo, T5);
    _movn(T1_Lo, getZero(), T5);
    _mov(DestHi, T1_Hi);
    _mov(DestLo, T1_Lo);
    return;
  }
  case InstArithmetic::Lshr: {

    auto *T_Lo = I32Reg();
    auto *T_Hi = I32Reg();
    auto *T1_Lo = I32Reg();
    auto *T1_Hi = I32Reg();
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();

    if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Src1)) {
      Src0HiR = legalizeToReg(hiOperand(Src0));
      int64_t ShiftAmount = Const->getValue();
      if (ShiftAmount < INT32_BITS) {
        Src0LoR = legalizeToReg(loOperand(Src0));
        _sll(T1, Src0HiR, INT32_BITS - ShiftAmount);
        _srl(T2, Src0LoR, ShiftAmount);
        _or(T_Lo, T1, T2);
        _srl(T_Hi, Src0HiR, ShiftAmount);
      } else if (ShiftAmount == INT32_BITS) {
        _mov(T_Lo, Src0HiR);
        _addiu(T_Hi, getZero(), 0);
      } else if (ShiftAmount > INT32_BITS && ShiftAmount < 64) {
        _srl(T_Lo, Src0HiR, ShiftAmount - INT32_BITS);
        _addiu(T_Hi, getZero(), 0);
      }
      _mov(DestLo, T_Lo);
      _mov(DestHi, T_Hi);
      return;
    }

    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));

    _srlv(T1, Src0LoR, Src1LoR);
    _not(T2, Src1LoR);
    _sll(T3, Src0HiR, 1);
    _sllv(T4, T3, T2);
    _or(T_Lo, T1, T4);
    _srlv(T_Hi, Src0HiR, Src1LoR);

    _mov(T1_Hi, T_Hi);
    _mov(T1_Lo, T_Lo);
    _andi(T5, Src1LoR, INT32_BITS);
    _movn(T1_Lo, T_Hi, T5);
    _movn(T1_Hi, getZero(), T5);
    _mov(DestHi, T1_Hi);
    _mov(DestLo, T1_Lo);
    return;
  }
  case InstArithmetic::Ashr: {

    auto *T_Lo = I32Reg();
    auto *T_Hi = I32Reg();
    auto *T1_Lo = I32Reg();
    auto *T1_Hi = I32Reg();
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    auto *T6 = I32Reg();

    if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Src1)) {
      Src0HiR = legalizeToReg(hiOperand(Src0));
      int64_t ShiftAmount = Const->getValue();
      if (ShiftAmount < INT32_BITS) {
        Src0LoR = legalizeToReg(loOperand(Src0));
        _sll(T1, Src0HiR, INT32_BITS - ShiftAmount);
        _srl(T2, Src0LoR, ShiftAmount);
        _or(T_Lo, T1, T2);
        _sra(T_Hi, Src0HiR, ShiftAmount);
      } else if (ShiftAmount == INT32_BITS) {
        _sra(T_Hi, Src0HiR, INT32_BITS - 1);
        _mov(T_Lo, Src0HiR);
      } else if (ShiftAmount > INT32_BITS && ShiftAmount < 64) {
        _sra(T_Lo, Src0HiR, ShiftAmount - INT32_BITS);
        _sra(T_Hi, Src0HiR, INT32_BITS - 1);
      }
      _mov(DestLo, T_Lo);
      _mov(DestHi, T_Hi);
      return;
    }

    Src0LoR = legalizeToReg(loOperand(Src0));
    Src1LoR = legalizeToReg(loOperand(Src1));
    Src0HiR = legalizeToReg(hiOperand(Src0));

    _srlv(T1, Src0LoR, Src1LoR);
    _not(T2, Src1LoR);
    _sll(T3, Src0HiR, 1);
    _sllv(T4, T3, T2);
    _or(T_Lo, T1, T4);
    _srav(T_Hi, Src0HiR, Src1LoR);

    _mov(T1_Hi, T_Hi);
    _mov(T1_Lo, T_Lo);
    _andi(T5, Src1LoR, INT32_BITS);
    _movn(T1_Lo, T_Hi, T5);
    _sra(T6, Src0HiR, INT32_BITS - 1);
    _movn(T1_Hi, T6, T5);
    _mov(DestHi, T1_Hi);
    _mov(DestLo, T1_Lo);
    return;
  }
  case InstArithmetic::Fadd:
  case InstArithmetic::Fsub:
  case InstArithmetic::Fmul:
  case InstArithmetic::Fdiv:
  case InstArithmetic::Frem:
    llvm::report_fatal_error("FP instruction with i64 type");
    return;
  case InstArithmetic::Udiv:
  case InstArithmetic::Sdiv:
  case InstArithmetic::Urem:
  case InstArithmetic::Srem:
    llvm::report_fatal_error("64-bit div and rem should have been prelowered");
    return;
  }
}

void TargetMIPS32::lowerArithmetic(const InstArithmetic *Instr) {
  Variable *Dest = Instr->getDest();

  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }

  // We need to signal all the UnimplementedLoweringError errors before any
  // legalization into new variables, otherwise Om1 register allocation may fail
  // when it sees variables that are defined but not used.
  Type DestTy = Dest->getType();
  Operand *Src0 = legalizeUndef(Instr->getSrc(0));
  Operand *Src1 = legalizeUndef(Instr->getSrc(1));
  if (DestTy == IceType_i64) {
    lowerInt64Arithmetic(Instr, Instr->getDest(), Src0, Src1);
    return;
  }
  if (isVectorType(Dest->getType())) {
    llvm::report_fatal_error("Arithmetic: Destination type is vector");
    return;
  }

  Variable *T = makeReg(Dest->getType());
  Variable *Src0R = legalizeToReg(Src0);
  Variable *Src1R = nullptr;
  uint32_t Value = 0;
  bool IsSrc1Imm16 = false;

  switch (Instr->getOp()) {
  case InstArithmetic::Add:
  case InstArithmetic::Sub: {
    auto *Const32 = llvm::dyn_cast<ConstantInteger32>(Src1);
    if (Const32 != nullptr && isInt<16>(int32_t(Const32->getValue()))) {
      IsSrc1Imm16 = true;
      Value = Const32->getValue();
    } else {
      Src1R = legalizeToReg(Src1);
    }
    break;
  }
  case InstArithmetic::And:
  case InstArithmetic::Or:
  case InstArithmetic::Xor:
  case InstArithmetic::Shl:
  case InstArithmetic::Lshr:
  case InstArithmetic::Ashr: {
    auto *Const32 = llvm::dyn_cast<ConstantInteger32>(Src1);
    if (Const32 != nullptr && llvm::isUInt<16>(uint32_t(Const32->getValue()))) {
      IsSrc1Imm16 = true;
      Value = Const32->getValue();
    } else {
      Src1R = legalizeToReg(Src1);
    }
    break;
  }
  default:
    Src1R = legalizeToReg(Src1);
    break;
  }
  constexpr uint32_t DivideByZeroTrapCode = 7;

  switch (Instr->getOp()) {
  case InstArithmetic::_num:
    break;
  case InstArithmetic::Add: {
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      if (!IsSrc1Imm16) {
        T1R = makeReg(IceType_i32);
        lowerCast(InstCast::create(Func, InstCast::Sext, T1R, Src1R));
      }
    }
    if (IsSrc1Imm16) {
      _addiu(T, T0R, Value);
    } else {
      _addu(T, T0R, T1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::And:
    if (IsSrc1Imm16) {
      _andi(T, Src0R, Value);
    } else {
      _and(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  case InstArithmetic::Or:
    if (IsSrc1Imm16) {
      _ori(T, Src0R, Value);
    } else {
      _or(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  case InstArithmetic::Xor:
    if (IsSrc1Imm16) {
      _xori(T, Src0R, Value);
    } else {
      _xor(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  case InstArithmetic::Sub: {
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      if (!IsSrc1Imm16) {
        T1R = makeReg(IceType_i32);
        lowerCast(InstCast::create(Func, InstCast::Sext, T1R, Src1R));
      }
    }
    if (IsSrc1Imm16) {
      _addiu(T, T0R, -Value);
    } else {
      _subu(T, T0R, T1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Mul: {
    _mul(T, Src0R, Src1R);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Shl: {
    if (IsSrc1Imm16) {
      _sll(T, Src0R, Value);
    } else {
      _sllv(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Lshr: {
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Zext, T0R, Src0R));
      if (!IsSrc1Imm16) {
        T1R = makeReg(IceType_i32);
        lowerCast(InstCast::create(Func, InstCast::Zext, T1R, Src1R));
      }
    }
    if (IsSrc1Imm16) {
      _srl(T, T0R, Value);
    } else {
      _srlv(T, T0R, T1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Ashr: {
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      if (!IsSrc1Imm16) {
        T1R = makeReg(IceType_i32);
        lowerCast(InstCast::create(Func, InstCast::Sext, T1R, Src1R));
      }
    }
    if (IsSrc1Imm16) {
      _sra(T, T0R, Value);
    } else {
      _srav(T, T0R, T1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Udiv: {
    auto *T_Zero = I32Reg(RegMIPS32::Reg_ZERO);
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Zext, T0R, Src0R));
      T1R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Zext, T1R, Src1R));
    }
    _divu(T_Zero, T0R, T1R);
    _teq(T1R, T_Zero, DivideByZeroTrapCode); // Trap if divide-by-zero
    _mflo(T, T_Zero);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Sdiv: {
    auto *T_Zero = I32Reg(RegMIPS32::Reg_ZERO);
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      T1R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T1R, Src1R));
    }
    _div(T_Zero, T0R, T1R);
    _teq(T1R, T_Zero, DivideByZeroTrapCode); // Trap if divide-by-zero
    _mflo(T, T_Zero);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Urem: {
    auto *T_Zero = I32Reg(RegMIPS32::Reg_ZERO);
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Zext, T0R, Src0R));
      T1R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Zext, T1R, Src1R));
    }
    _divu(T_Zero, T0R, T1R);
    _teq(T1R, T_Zero, DivideByZeroTrapCode); // Trap if divide-by-zero
    _mfhi(T, T_Zero);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Srem: {
    auto *T_Zero = I32Reg(RegMIPS32::Reg_ZERO);
    auto *T0R = Src0R;
    auto *T1R = Src1R;
    if (Dest->getType() != IceType_i32) {
      T0R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      T1R = makeReg(IceType_i32);
      lowerCast(InstCast::create(Func, InstCast::Sext, T1R, Src1R));
    }
    _div(T_Zero, T0R, T1R);
    _teq(T1R, T_Zero, DivideByZeroTrapCode); // Trap if divide-by-zero
    _mfhi(T, T_Zero);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Fadd: {
    if (DestTy == IceType_f32) {
      _add_s(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    if (DestTy == IceType_f64) {
      _add_d(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    break;
  }
  case InstArithmetic::Fsub:
    if (DestTy == IceType_f32) {
      _sub_s(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    if (DestTy == IceType_f64) {
      _sub_d(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    break;
  case InstArithmetic::Fmul:
    if (DestTy == IceType_f32) {
      _mul_s(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    if (DestTy == IceType_f64) {
      _mul_d(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    break;
  case InstArithmetic::Fdiv:
    if (DestTy == IceType_f32) {
      _div_s(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    if (DestTy == IceType_f64) {
      _div_d(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
    break;
  case InstArithmetic::Frem:
    llvm::report_fatal_error("frem should have been prelowered.");
    break;
  }
  llvm::report_fatal_error("Unknown arithmetic operator");
}

void TargetMIPS32::lowerAssign(const InstAssign *Instr) {
  Variable *Dest = Instr->getDest();

  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }

  // Source type may not be same as destination
  if (isVectorType(Dest->getType())) {
    Operand *Src0 = legalizeUndef(Instr->getSrc(0));
    auto *DstVec = llvm::dyn_cast<VariableVecOn32>(Dest);
    for (SizeT i = 0; i < DstVec->ContainersPerVector; ++i) {
      auto *DCont = DstVec->getContainers()[i];
      auto *SCont =
          legalize(getOperandAtIndex(Src0, IceType_i32, i), Legal_Reg);
      auto *TReg = makeReg(IceType_i32);
      _mov(TReg, SCont);
      _mov(DCont, TReg);
    }
    return;
  }
  Operand *Src0 = Instr->getSrc(0);
  assert(Dest->getType() == Src0->getType());
  if (Dest->getType() == IceType_i64) {
    Src0 = legalizeUndef(Src0);
    Operand *Src0Lo = legalize(loOperand(Src0), Legal_Reg);
    Operand *Src0Hi = legalize(hiOperand(Src0), Legal_Reg);
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    auto *T_Lo = I32Reg(), *T_Hi = I32Reg();
    _mov(T_Lo, Src0Lo);
    _mov(DestLo, T_Lo);
    _mov(T_Hi, Src0Hi);
    _mov(DestHi, T_Hi);
    return;
  }
  Operand *SrcR;
  if (Dest->hasReg()) {
    // If Dest already has a physical register, then legalize the Src operand
    // into a Variable with the same register assignment.  This especially
    // helps allow the use of Flex operands.
    SrcR = legalize(Src0, Legal_Reg, Dest->getRegNum());
  } else {
    // Dest could be a stack operand. Since we could potentially need
    // to do a Store (and store can only have Register operands),
    // legalize this to a register.
    SrcR = legalize(Src0, Legal_Reg);
  }
  _mov(Dest, SrcR);
}

void TargetMIPS32::lowerBr(const InstBr *Instr) {
  if (Instr->isUnconditional()) {
    _br(Instr->getTargetUnconditional());
    return;
  }
  CfgNode *TargetTrue = Instr->getTargetTrue();
  CfgNode *TargetFalse = Instr->getTargetFalse();
  Operand *Boolean = Instr->getCondition();
  const Inst *Producer = Computations.getProducerOf(Boolean);
  if (Producer == nullptr) {
    // Since we don't know the producer of this boolean we will assume its
    // producer will keep it in positive logic and just emit beqz with this
    // Boolean as an operand.
    auto *BooleanR = legalizeToReg(Boolean);
    _br(TargetTrue, TargetFalse, BooleanR, CondMIPS32::Cond::EQZ);
    return;
  }
  if (Producer->getKind() == Inst::Icmp) {
    const InstIcmp *CompareInst = llvm::cast<InstIcmp>(Producer);
    Operand *Src0 = CompareInst->getSrc(0);
    Operand *Src1 = CompareInst->getSrc(1);
    const Type Src0Ty = Src0->getType();
    assert(Src0Ty == Src1->getType());

    Variable *Src0R = nullptr;
    Variable *Src1R = nullptr;
    Variable *Src0HiR = nullptr;
    Variable *Src1HiR = nullptr;
    if (Src0Ty == IceType_i64) {
      Src0R = legalizeToReg(loOperand(Src0));
      Src1R = legalizeToReg(loOperand(Src1));
      Src0HiR = legalizeToReg(hiOperand(Src0));
      Src1HiR = legalizeToReg(hiOperand(Src1));
    } else {
      auto *Src0RT = legalizeToReg(Src0);
      auto *Src1RT = legalizeToReg(Src1);
      // Sign/Zero extend the source operands
      if (Src0Ty != IceType_i32) {
        InstCast::OpKind CastKind;
        switch (CompareInst->getCondition()) {
        case InstIcmp::Eq:
        case InstIcmp::Ne:
        case InstIcmp::Sgt:
        case InstIcmp::Sge:
        case InstIcmp::Slt:
        case InstIcmp::Sle:
          CastKind = InstCast::Sext;
          break;
        default:
          CastKind = InstCast::Zext;
          break;
        }
        Src0R = makeReg(IceType_i32);
        Src1R = makeReg(IceType_i32);
        lowerCast(InstCast::create(Func, CastKind, Src0R, Src0RT));
        lowerCast(InstCast::create(Func, CastKind, Src1R, Src1RT));
      } else {
        Src0R = Src0RT;
        Src1R = Src1RT;
      }
    }
    auto *DestT = makeReg(IceType_i32);

    switch (CompareInst->getCondition()) {
    default:
      llvm_unreachable("unexpected condition");
      return;
    case InstIcmp::Eq: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _xor(T2, Src0R, Src1R);
        _or(T3, T1, T2);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _br(TargetTrue, TargetFalse, Src0R, Src1R, CondMIPS32::Cond::NE);
      }
      return;
    }
    case InstIcmp::Ne: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _xor(T2, Src0R, Src1R);
        _or(T3, T1, T2);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::EQZ);
      } else {
        _br(TargetTrue, TargetFalse, Src0R, Src1R, CondMIPS32::Cond::EQ);
      }
      return;
    }
    case InstIcmp::Ugt: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        auto *T4 = I32Reg();
        auto *T5 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _sltu(T2, Src1HiR, Src0HiR);
        _xori(T3, T2, 1);
        _sltu(T4, Src1R, Src0R);
        _xori(T5, T4, 1);
        _movz(T3, T5, T1);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _sltu(DestT, Src1R, Src0R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::EQZ);
      }
      return;
    }
    case InstIcmp::Uge: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _sltu(T2, Src0HiR, Src1HiR);
        _sltu(T3, Src0R, Src1R);
        _movz(T2, T3, T1);
        _mov(DestT, T2);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _sltu(DestT, Src0R, Src1R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      }
      return;
    }
    case InstIcmp::Ult: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        auto *T4 = I32Reg();
        auto *T5 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _sltu(T2, Src0HiR, Src1HiR);
        _xori(T3, T2, 1);
        _sltu(T4, Src0R, Src1R);
        _xori(T5, T4, 1);
        _movz(T3, T5, T1);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _sltu(DestT, Src0R, Src1R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::EQZ);
      }
      return;
    }
    case InstIcmp::Ule: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _sltu(T2, Src1HiR, Src0HiR);
        _sltu(T3, Src1R, Src0R);
        _movz(T2, T3, T1);
        _mov(DestT, T2);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _sltu(DestT, Src1R, Src0R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      }
      return;
    }
    case InstIcmp::Sgt: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        auto *T4 = I32Reg();
        auto *T5 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _slt(T2, Src1HiR, Src0HiR);
        _xori(T3, T2, 1);
        _sltu(T4, Src1R, Src0R);
        _xori(T5, T4, 1);
        _movz(T3, T5, T1);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _slt(DestT, Src1R, Src0R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::EQZ);
      }
      return;
    }
    case InstIcmp::Sge: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _slt(T2, Src0HiR, Src1HiR);
        _sltu(T3, Src0R, Src1R);
        _movz(T2, T3, T1);
        _mov(DestT, T2);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _slt(DestT, Src0R, Src1R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      }
      return;
    }
    case InstIcmp::Slt: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        auto *T4 = I32Reg();
        auto *T5 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _slt(T2, Src0HiR, Src1HiR);
        _xori(T3, T2, 1);
        _sltu(T4, Src0R, Src1R);
        _xori(T5, T4, 1);
        _movz(T3, T5, T1);
        _mov(DestT, T3);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _slt(DestT, Src0R, Src1R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::EQZ);
      }
      return;
    }
    case InstIcmp::Sle: {
      if (Src0Ty == IceType_i64) {
        auto *T1 = I32Reg();
        auto *T2 = I32Reg();
        auto *T3 = I32Reg();
        _xor(T1, Src0HiR, Src1HiR);
        _slt(T2, Src1HiR, Src0HiR);
        _sltu(T3, Src1R, Src0R);
        _movz(T2, T3, T1);
        _mov(DestT, T2);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      } else {
        _slt(DestT, Src1R, Src0R);
        _br(TargetTrue, TargetFalse, DestT, CondMIPS32::Cond::NEZ);
      }
      return;
    }
    }
  }
}

void TargetMIPS32::lowerCall(const InstCall *Instr) {
  CfgVector<Variable *> RegArgs;
  NeedsStackAlignment = true;

  //  Assign arguments to registers and stack. Also reserve stack.
  TargetMIPS32::CallingConv CC;

  // Pair of Arg Operand -> GPR number assignments.
  llvm::SmallVector<std::pair<Operand *, RegNumT>, MIPS32_MAX_GPR_ARG> GPRArgs;
  llvm::SmallVector<std::pair<Operand *, RegNumT>, MIPS32_MAX_FP_ARG> FPArgs;
  // Pair of Arg Operand -> stack offset.
  llvm::SmallVector<std::pair<Operand *, int32_t>, 8> StackArgs;
  size_t ParameterAreaSizeBytes = 16;

  // Classify each argument operand according to the location where the
  // argument is passed.

  // v4f32 is returned through stack. $4 is setup by the caller and passed as
  // first argument implicitly. Callee then copies the return vector at $4.
  SizeT ArgNum = 0;
  Variable *Dest = Instr->getDest();
  Variable *RetVecFloat = nullptr;
  if (Dest && isVectorFloatingType(Dest->getType())) {
    ArgNum = 1;
    CC.discardReg(RegMIPS32::Reg_A0);
    RetVecFloat = Func->makeVariable(IceType_i32);
    auto *ByteCount = ConstantInteger32::create(Ctx, IceType_i32, 16);
    constexpr SizeT Alignment = 4;
    lowerAlloca(InstAlloca::create(Func, RetVecFloat, ByteCount, Alignment));
    RegArgs.emplace_back(
        legalizeToReg(RetVecFloat, RegNumT::fixme(RegMIPS32::Reg_A0)));
  }

  for (SizeT i = 0, NumArgs = Instr->getNumArgs(); i < NumArgs; ++i) {
    Operand *Arg = legalizeUndef(Instr->getArg(i));
    const Type Ty = Arg->getType();
    bool InReg = false;
    RegNumT Reg;

    InReg = CC.argInReg(Ty, i, &Reg);

    if (!InReg) {
      if (isVectorType(Ty)) {
        auto *ArgVec = llvm::cast<VariableVecOn32>(Arg);
        ParameterAreaSizeBytes =
            applyStackAlignmentTy(ParameterAreaSizeBytes, IceType_i64);
        for (Variable *Elem : ArgVec->getContainers()) {
          StackArgs.push_back(std::make_pair(Elem, ParameterAreaSizeBytes));
          ParameterAreaSizeBytes += typeWidthInBytesOnStack(IceType_i32);
        }
      } else {
        ParameterAreaSizeBytes =
            applyStackAlignmentTy(ParameterAreaSizeBytes, Ty);
        StackArgs.push_back(std::make_pair(Arg, ParameterAreaSizeBytes));
        ParameterAreaSizeBytes += typeWidthInBytesOnStack(Ty);
      }
      ++ArgNum;
      continue;
    }

    if (isVectorType(Ty)) {
      auto *ArgVec = llvm::cast<VariableVecOn32>(Arg);
      Operand *Elem0 = ArgVec->getContainers()[0];
      Operand *Elem1 = ArgVec->getContainers()[1];
      GPRArgs.push_back(
          std::make_pair(Elem0, RegNumT::fixme((unsigned)Reg + 0)));
      GPRArgs.push_back(
          std::make_pair(Elem1, RegNumT::fixme((unsigned)Reg + 1)));
      Operand *Elem2 = ArgVec->getContainers()[2];
      Operand *Elem3 = ArgVec->getContainers()[3];
      // First argument is passed in $4:$5:$6:$7
      // Second and rest arguments are passed in $6:$7:stack:stack
      if (ArgNum == 0) {
        GPRArgs.push_back(
            std::make_pair(Elem2, RegNumT::fixme((unsigned)Reg + 2)));
        GPRArgs.push_back(
            std::make_pair(Elem3, RegNumT::fixme((unsigned)Reg + 3)));
      } else {
        ParameterAreaSizeBytes =
            applyStackAlignmentTy(ParameterAreaSizeBytes, IceType_i64);
        StackArgs.push_back(std::make_pair(Elem2, ParameterAreaSizeBytes));
        ParameterAreaSizeBytes += typeWidthInBytesOnStack(IceType_i32);
        StackArgs.push_back(std::make_pair(Elem3, ParameterAreaSizeBytes));
        ParameterAreaSizeBytes += typeWidthInBytesOnStack(IceType_i32);
      }
    } else if (Ty == IceType_i64) {
      Operand *Lo = loOperand(Arg);
      Operand *Hi = hiOperand(Arg);
      GPRArgs.push_back(
          std::make_pair(Lo, RegMIPS32::get64PairFirstRegNum(Reg)));
      GPRArgs.push_back(
          std::make_pair(Hi, RegMIPS32::get64PairSecondRegNum(Reg)));
    } else if (isScalarIntegerType(Ty)) {
      GPRArgs.push_back(std::make_pair(Arg, Reg));
    } else {
      FPArgs.push_back(std::make_pair(Arg, Reg));
    }
    ++ArgNum;
  }

  // Adjust the parameter area so that the stack is aligned. It is assumed that
  // the stack is already aligned at the start of the calling sequence.
  ParameterAreaSizeBytes = applyStackAlignment(ParameterAreaSizeBytes);

  // Copy arguments that are passed on the stack to the appropriate stack
  // locations.
  Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
  for (auto &StackArg : StackArgs) {
    ConstantInteger32 *Loc =
        llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(StackArg.second));
    Type Ty = StackArg.first->getType();
    OperandMIPS32Mem *Addr;
    constexpr bool SignExt = false;
    if (OperandMIPS32Mem::canHoldOffset(Ty, SignExt, StackArg.second)) {
      Addr = OperandMIPS32Mem::create(Func, Ty, SP, Loc);
    } else {
      Variable *NewBase = Func->makeVariable(SP->getType());
      lowerArithmetic(
          InstArithmetic::create(Func, InstArithmetic::Add, NewBase, SP, Loc));
      Addr = formMemoryOperand(NewBase, Ty);
    }
    lowerStore(InstStore::create(Func, StackArg.first, Addr));
  }

  // Generate the call instruction.  Assign its result to a temporary with high
  // register allocation weight.

  // ReturnReg doubles as ReturnRegLo as necessary.
  Variable *ReturnReg = nullptr;
  Variable *ReturnRegHi = nullptr;
  if (Dest) {
    switch (Dest->getType()) {
    case IceType_NUM:
      llvm_unreachable("Invalid Call dest type");
      return;
    case IceType_void:
      break;
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32:
      ReturnReg = makeReg(Dest->getType(), RegMIPS32::Reg_V0);
      break;
    case IceType_i64:
      ReturnReg = I32Reg(RegMIPS32::Reg_V0);
      ReturnRegHi = I32Reg(RegMIPS32::Reg_V1);
      break;
    case IceType_f32:
      ReturnReg = makeReg(Dest->getType(), RegMIPS32::Reg_F0);
      break;
    case IceType_f64:
      ReturnReg = makeReg(IceType_f64, RegMIPS32::Reg_F0);
      break;
    case IceType_v4i1:
    case IceType_v8i1:
    case IceType_v16i1:
    case IceType_v16i8:
    case IceType_v8i16:
    case IceType_v4i32: {
      ReturnReg = makeReg(Dest->getType(), RegMIPS32::Reg_V0);
      auto *RetVec = llvm::dyn_cast<VariableVecOn32>(ReturnReg);
      RetVec->initVecElement(Func);
      for (SizeT i = 0; i < RetVec->ContainersPerVector; ++i) {
        auto *Var = RetVec->getContainers()[i];
        Var->setRegNum(RegNumT::fixme(RegMIPS32::Reg_V0 + i));
      }
      break;
    }
    case IceType_v4f32:
      ReturnReg = makeReg(IceType_i32, RegMIPS32::Reg_V0);
      break;
    }
  }
  Operand *CallTarget = Instr->getCallTarget();
  // Allow ConstantRelocatable to be left alone as a direct call,
  // but force other constants like ConstantInteger32 to be in
  // a register and make it an indirect call.
  if (!llvm::isa<ConstantRelocatable>(CallTarget)) {
    CallTarget = legalize(CallTarget, Legal_Reg);
  }

  // Copy arguments to be passed in registers to the appropriate registers.
  for (auto &FPArg : FPArgs) {
    RegArgs.emplace_back(legalizeToReg(FPArg.first, FPArg.second));
  }
  for (auto &GPRArg : GPRArgs) {
    RegArgs.emplace_back(legalizeToReg(GPRArg.first, GPRArg.second));
  }

  // Generate a FakeUse of register arguments so that they do not get dead code
  // eliminated as a result of the FakeKill of scratch registers after the call.
  // These fake-uses need to be placed here to avoid argument registers from
  // being used during the legalizeToReg() calls above.
  for (auto *RegArg : RegArgs) {
    Context.insert<InstFakeUse>(RegArg);
  }

  // If variable alloca is used the extra 16 bytes for argument build area
  // will be allocated on stack before a call.
  if (VariableAllocaUsed)
    _addiu(SP, SP, -MaxOutArgsSizeBytes);

  Inst *NewCall;

  // We don't need to define the return register if it is a vector.
  // We have inserted fake defs of it just after the call.
  if (ReturnReg && isVectorIntegerType(ReturnReg->getType())) {
    Variable *RetReg = nullptr;
    NewCall = InstMIPS32Call::create(Func, RetReg, CallTarget);
    Context.insert(NewCall);
  } else {
    NewCall = Context.insert<InstMIPS32Call>(ReturnReg, CallTarget);
  }

  if (VariableAllocaUsed)
    _addiu(SP, SP, MaxOutArgsSizeBytes);

  // Insert a fake use of stack pointer to avoid dead code elimination of addiu
  // instruction.
  Context.insert<InstFakeUse>(SP);

  if (ReturnRegHi)
    Context.insert(InstFakeDef::create(Func, ReturnRegHi));

  if (ReturnReg) {
    if (auto *RetVec = llvm::dyn_cast<VariableVecOn32>(ReturnReg)) {
      for (Variable *Var : RetVec->getContainers()) {
        Context.insert(InstFakeDef::create(Func, Var));
      }
    }
  }

  // Insert a register-kill pseudo instruction.
  Context.insert(InstFakeKill::create(Func, NewCall));

  // Generate a FakeUse to keep the call live if necessary.
  if (Instr->hasSideEffects() && ReturnReg) {
    if (auto *RetVec = llvm::dyn_cast<VariableVecOn32>(ReturnReg)) {
      for (Variable *Var : RetVec->getContainers()) {
        Context.insert<InstFakeUse>(Var);
      }
    } else {
      Context.insert<InstFakeUse>(ReturnReg);
    }
  }

  if (Dest == nullptr)
    return;

  // Assign the result of the call to Dest.
  if (ReturnReg) {
    if (RetVecFloat) {
      auto *DestVecOn32 = llvm::cast<VariableVecOn32>(Dest);
      auto *TBase = legalizeToReg(RetVecFloat);
      for (SizeT i = 0; i < DestVecOn32->ContainersPerVector; ++i) {
        auto *Var = DestVecOn32->getContainers()[i];
        auto *TVar = makeReg(IceType_i32);
        OperandMIPS32Mem *Mem = OperandMIPS32Mem::create(
            Func, IceType_i32, TBase,
            llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(i * 4)));
        _lw(TVar, Mem);
        _mov(Var, TVar);
      }
    } else if (auto *RetVec = llvm::dyn_cast<VariableVecOn32>(ReturnReg)) {
      auto *DestVecOn32 = llvm::cast<VariableVecOn32>(Dest);
      for (SizeT i = 0; i < DestVecOn32->ContainersPerVector; ++i) {
        _mov(DestVecOn32->getContainers()[i], RetVec->getContainers()[i]);
      }
    } else if (ReturnRegHi) {
      assert(Dest->getType() == IceType_i64);
      auto *Dest64On32 = llvm::cast<Variable64On32>(Dest);
      Variable *DestLo = Dest64On32->getLo();
      Variable *DestHi = Dest64On32->getHi();
      _mov(DestLo, ReturnReg);
      _mov(DestHi, ReturnRegHi);
    } else {
      assert(Dest->getType() == IceType_i32 || Dest->getType() == IceType_i16 ||
             Dest->getType() == IceType_i8 || Dest->getType() == IceType_i1 ||
             isScalarFloatingType(Dest->getType()) ||
             isVectorType(Dest->getType()));
      _mov(Dest, ReturnReg);
    }
  }
}

void TargetMIPS32::lowerCast(const InstCast *Instr) {
  InstCast::OpKind CastKind = Instr->getCastKind();
  Variable *Dest = Instr->getDest();
  Operand *Src0 = legalizeUndef(Instr->getSrc(0));
  const Type DestTy = Dest->getType();
  const Type Src0Ty = Src0->getType();
  const uint32_t ShiftAmount =
      (Src0Ty == IceType_i1
           ? INT32_BITS - 1
           : INT32_BITS - (CHAR_BITS * typeWidthInBytes(Src0Ty)));
  const uint32_t Mask =
      (Src0Ty == IceType_i1
           ? 1
           : (1 << (CHAR_BITS * typeWidthInBytes(Src0Ty))) - 1);

  if (isVectorType(DestTy)) {
    llvm::report_fatal_error("Cast: Destination type is vector");
    return;
  }
  switch (CastKind) {
  default:
    Func->setError("Cast type not supported");
    return;
  case InstCast::Sext: {
    if (DestTy == IceType_i64) {
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T1_Lo = I32Reg();
      Variable *T2_Lo = I32Reg();
      Variable *T_Hi = I32Reg();
      if (Src0Ty == IceType_i1) {
        _sll(T1_Lo, Src0R, INT32_BITS - 1);
        _sra(T2_Lo, T1_Lo, INT32_BITS - 1);
        _mov(DestHi, T2_Lo);
        _mov(DestLo, T2_Lo);
      } else if (Src0Ty == IceType_i8 || Src0Ty == IceType_i16) {
        _sll(T1_Lo, Src0R, ShiftAmount);
        _sra(T2_Lo, T1_Lo, ShiftAmount);
        _sra(T_Hi, T2_Lo, INT32_BITS - 1);
        _mov(DestHi, T_Hi);
        _mov(DestLo, T2_Lo);
      } else if (Src0Ty == IceType_i32) {
        _mov(T1_Lo, Src0R);
        _sra(T_Hi, T1_Lo, INT32_BITS - 1);
        _mov(DestHi, T_Hi);
        _mov(DestLo, T1_Lo);
      }
    } else {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T1 = makeReg(DestTy);
      Variable *T2 = makeReg(DestTy);
      if (Src0Ty == IceType_i1 || Src0Ty == IceType_i8 ||
          Src0Ty == IceType_i16) {
        _sll(T1, Src0R, ShiftAmount);
        _sra(T2, T1, ShiftAmount);
        _mov(Dest, T2);
      }
    }
    break;
  }
  case InstCast::Zext: {
    if (DestTy == IceType_i64) {
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T_Lo = I32Reg();
      Variable *T_Hi = I32Reg();

      if (Src0Ty == IceType_i1 || Src0Ty == IceType_i8 || Src0Ty == IceType_i16)
        _andi(T_Lo, Src0R, Mask);
      else if (Src0Ty == IceType_i32)
        _mov(T_Lo, Src0R);
      else
        assert(Src0Ty != IceType_i64);
      _mov(DestLo, T_Lo);

      auto *Zero = getZero();
      _addiu(T_Hi, Zero, 0);
      _mov(DestHi, T_Hi);
    } else {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T = makeReg(DestTy);
      if (Src0Ty == IceType_i1 || Src0Ty == IceType_i8 ||
          Src0Ty == IceType_i16) {
        _andi(T, Src0R, Mask);
        _mov(Dest, T);
      }
    }
    break;
  }
  case InstCast::Trunc: {
    if (Src0Ty == IceType_i64)
      Src0 = loOperand(Src0);
    Variable *Src0R = legalizeToReg(Src0);
    Variable *T = makeReg(DestTy);
    switch (DestTy) {
    case IceType_i1:
      _andi(T, Src0R, 0x1);
      break;
    case IceType_i8:
      _andi(T, Src0R, 0xff);
      break;
    case IceType_i16:
      _andi(T, Src0R, 0xffff);
      break;
    default:
      _mov(T, Src0R);
      break;
    }
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptrunc: {
    assert(Dest->getType() == IceType_f32);
    assert(Src0->getType() == IceType_f64);
    auto *DestR = legalizeToReg(Dest);
    auto *Src0R = legalizeToReg(Src0);
    _cvt_s_d(DestR, Src0R);
    _mov(Dest, DestR);
    break;
  }
  case InstCast::Fpext: {
    assert(Dest->getType() == IceType_f64);
    assert(Src0->getType() == IceType_f32);
    auto *DestR = legalizeToReg(Dest);
    auto *Src0R = legalizeToReg(Src0);
    _cvt_d_s(DestR, Src0R);
    _mov(Dest, DestR);
    break;
  }
  case InstCast::Fptosi:
  case InstCast::Fptoui: {
    if (llvm::isa<Variable64On32>(Dest)) {
      llvm::report_fatal_error("fp-to-i64 should have been prelowered.");
      return;
    }
    if (DestTy != IceType_i64) {
      if (Src0Ty == IceType_f32 && isScalarIntegerType(DestTy)) {
        Variable *Src0R = legalizeToReg(Src0);
        Variable *FTmp = makeReg(IceType_f32);
        _trunc_w_s(FTmp, Src0R);
        _mov(Dest, FTmp);
        return;
      }
      if (Src0Ty == IceType_f64 && isScalarIntegerType(DestTy)) {
        Variable *Src0R = legalizeToReg(Src0);
        Variable *FTmp = makeReg(IceType_f64);
        _trunc_w_d(FTmp, Src0R);
        _mov(Dest, FTmp);
        return;
      }
    }
    llvm::report_fatal_error("Destination is i64 in fp-to-i32");
    break;
  }
  case InstCast::Sitofp:
  case InstCast::Uitofp: {
    if (llvm::isa<Variable64On32>(Dest)) {
      llvm::report_fatal_error("i64-to-fp should have been prelowered.");
      return;
    }
    if (Src0Ty != IceType_i64) {
      Variable *Src0R = legalizeToReg(Src0);
      auto *T0R = Src0R;
      if (Src0Ty != IceType_i32) {
        T0R = makeReg(IceType_i32);
        if (CastKind == InstCast::Uitofp)
          lowerCast(InstCast::create(Func, InstCast::Zext, T0R, Src0R));
        else
          lowerCast(InstCast::create(Func, InstCast::Sext, T0R, Src0R));
      }
      if (isScalarIntegerType(Src0Ty) && DestTy == IceType_f32) {
        Variable *FTmp1 = makeReg(IceType_f32);
        Variable *FTmp2 = makeReg(IceType_f32);
        _mtc1(FTmp1, T0R);
        _cvt_s_w(FTmp2, FTmp1);
        _mov(Dest, FTmp2);
        return;
      }
      if (isScalarIntegerType(Src0Ty) && DestTy == IceType_f64) {
        Variable *FTmp1 = makeReg(IceType_f64);
        Variable *FTmp2 = makeReg(IceType_f64);
        _mtc1(FTmp1, T0R);
        _cvt_d_w(FTmp2, FTmp1);
        _mov(Dest, FTmp2);
        return;
      }
    }
    llvm::report_fatal_error("Source is i64 in i32-to-fp");
    break;
  }
  case InstCast::Bitcast: {
    Operand *Src0 = Instr->getSrc(0);
    if (DestTy == Src0->getType()) {
      auto *Assign = InstAssign::create(Func, Dest, Src0);
      lowerAssign(Assign);
      return;
    }
    if (isVectorType(DestTy) || isVectorType(Src0->getType())) {
      llvm::report_fatal_error(
          "Bitcast: vector type should have been prelowered.");
      return;
    }
    switch (DestTy) {
    case IceType_NUM:
    case IceType_void:
      llvm::report_fatal_error("Unexpected bitcast.");
    case IceType_i1:
      UnimplementedLoweringError(this, Instr);
      break;
    case IceType_i8:
      assert(Src0->getType() == IceType_v8i1);
      llvm::report_fatal_error(
          "i8 to v8i1 conversion should have been prelowered.");
      break;
    case IceType_i16:
      assert(Src0->getType() == IceType_v16i1);
      llvm::report_fatal_error(
          "i16 to v16i1 conversion should have been prelowered.");
      break;
    case IceType_i32:
    case IceType_f32: {
      Variable *Src0R = legalizeToReg(Src0);
      _mov(Dest, Src0R);
      break;
    }
    case IceType_i64: {
      assert(Src0->getType() == IceType_f64);
      Variable *Src0R = legalizeToReg(Src0);
      auto *T = llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
      T->initHiLo(Func);
      T->getHi()->setMustNotHaveReg();
      T->getLo()->setMustNotHaveReg();
      Context.insert<InstFakeDef>(T->getHi());
      Context.insert<InstFakeDef>(T->getLo());
      _mov_fp64_to_i64(T->getHi(), Src0R, Int64_Hi);
      _mov_fp64_to_i64(T->getLo(), Src0R, Int64_Lo);
      lowerAssign(InstAssign::create(Func, Dest, T));
      break;
    }
    case IceType_f64: {
      assert(Src0->getType() == IceType_i64);
      const uint32_t Mask = 0xFFFFFFFF;
      if (auto *C64 = llvm::dyn_cast<ConstantInteger64>(Src0)) {
        Variable *RegHi, *RegLo;
        const uint64_t Value = C64->getValue();
        uint64_t Upper32Bits = (Value >> INT32_BITS) & Mask;
        uint64_t Lower32Bits = Value & Mask;
        RegLo = legalizeToReg(Ctx->getConstantInt32(Lower32Bits));
        RegHi = legalizeToReg(Ctx->getConstantInt32(Upper32Bits));
        _mov(Dest, RegHi, RegLo);
      } else {
        auto *Var64On32 = llvm::cast<Variable64On32>(Src0);
        auto *RegLo = legalizeToReg(loOperand(Var64On32));
        auto *RegHi = legalizeToReg(hiOperand(Var64On32));
        _mov(Dest, RegHi, RegLo);
      }
      break;
    }
    default:
      llvm::report_fatal_error("Unexpected bitcast.");
    }
    break;
  }
  }
}

void TargetMIPS32::lowerExtractElement(const InstExtractElement *Instr) {
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();
  Operand *Src1 = Instr->getSrc(1);
  if (const auto *Imm = llvm::dyn_cast<ConstantInteger32>(Src1)) {
    const uint32_t Index = Imm->getValue();
    Variable *TDest = makeReg(DestTy);
    Variable *TReg = makeReg(DestTy);
    auto *Src0 = legalizeUndef(Instr->getSrc(0));
    auto *Src0R = llvm::dyn_cast<VariableVecOn32>(Src0);
    // Number of elements in each container
    uint32_t ElemPerCont =
        typeNumElements(Src0->getType()) / Src0R->ContainersPerVector;
    auto *Src = Src0R->getContainers()[Index / ElemPerCont];
    auto *SrcE = legalizeToReg(Src);
    // Position of the element in the container
    uint32_t PosInCont = Index % ElemPerCont;
    if (ElemPerCont == 1) {
      _mov(TDest, SrcE);
    } else if (ElemPerCont == 2) {
      switch (PosInCont) {
      case 0:
        _andi(TDest, SrcE, 0xffff);
        break;
      case 1:
        _srl(TDest, SrcE, 16);
        break;
      default:
        llvm::report_fatal_error("ExtractElement: Invalid PosInCont");
        break;
      }
    } else if (ElemPerCont == 4) {
      switch (PosInCont) {
      case 0:
        _andi(TDest, SrcE, 0xff);
        break;
      case 1:
        _srl(TReg, SrcE, 8);
        _andi(TDest, TReg, 0xff);
        break;
      case 2:
        _srl(TReg, SrcE, 16);
        _andi(TDest, TReg, 0xff);
        break;
      case 3:
        _srl(TDest, SrcE, 24);
        break;
      default:
        llvm::report_fatal_error("ExtractElement: Invalid PosInCont");
        break;
      }
    }
    if (typeElementType(Src0R->getType()) == IceType_i1) {
      Variable *TReg1 = makeReg(DestTy);
      _andi(TReg1, TDest, 0x1);
      _mov(Dest, TReg1);
    } else {
      _mov(Dest, TDest);
    }
    return;
  }
  llvm::report_fatal_error("ExtractElement requires a constant index");
}

void TargetMIPS32::lowerFcmp(const InstFcmp *Instr) {
  Variable *Dest = Instr->getDest();
  if (isVectorType(Dest->getType())) {
    llvm::report_fatal_error("Fcmp: Destination type is vector");
    return;
  }

  auto *Src0 = Instr->getSrc(0);
  auto *Src1 = Instr->getSrc(1);
  auto *Zero = getZero();

  InstFcmp::FCond Cond = Instr->getCondition();
  auto *DestR = makeReg(IceType_i32);
  auto *Src0R = legalizeToReg(Src0);
  auto *Src1R = legalizeToReg(Src1);
  const Type Src0Ty = Src0->getType();

  Operand *FCC0 = OperandMIPS32FCC::create(getFunc(), OperandMIPS32FCC::FCC0);

  switch (Cond) {
  default: {
    llvm::report_fatal_error("Unhandled fp comparison.");
    return;
  }
  case InstFcmp::False: {
    Context.insert<InstFakeUse>(Src0R);
    Context.insert<InstFakeUse>(Src1R);
    _addiu(DestR, Zero, 0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Oeq: {
    if (Src0Ty == IceType_f32) {
      _c_eq_s(Src0R, Src1R);
    } else {
      _c_eq_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ogt: {
    if (Src0Ty == IceType_f32) {
      _c_ule_s(Src0R, Src1R);
    } else {
      _c_ule_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Oge: {
    if (Src0Ty == IceType_f32) {
      _c_ult_s(Src0R, Src1R);
    } else {
      _c_ult_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Olt: {
    if (Src0Ty == IceType_f32) {
      _c_olt_s(Src0R, Src1R);
    } else {
      _c_olt_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ole: {
    if (Src0Ty == IceType_f32) {
      _c_ole_s(Src0R, Src1R);
    } else {
      _c_ole_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::One: {
    if (Src0Ty == IceType_f32) {
      _c_ueq_s(Src0R, Src1R);
    } else {
      _c_ueq_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ord: {
    if (Src0Ty == IceType_f32) {
      _c_un_s(Src0R, Src1R);
    } else {
      _c_un_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ueq: {
    if (Src0Ty == IceType_f32) {
      _c_ueq_s(Src0R, Src1R);
    } else {
      _c_ueq_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ugt: {
    if (Src0Ty == IceType_f32) {
      _c_ole_s(Src0R, Src1R);
    } else {
      _c_ole_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Uge: {
    if (Src0Ty == IceType_f32) {
      _c_olt_s(Src0R, Src1R);
    } else {
      _c_olt_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ult: {
    if (Src0Ty == IceType_f32) {
      _c_ult_s(Src0R, Src1R);
    } else {
      _c_ult_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Ule: {
    if (Src0Ty == IceType_f32) {
      _c_ule_s(Src0R, Src1R);
    } else {
      _c_ule_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Une: {
    if (Src0Ty == IceType_f32) {
      _c_eq_s(Src0R, Src1R);
    } else {
      _c_eq_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movt(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::Uno: {
    if (Src0Ty == IceType_f32) {
      _c_un_s(Src0R, Src1R);
    } else {
      _c_un_d(Src0R, Src1R);
    }
    _addiu(DestR, Zero, 1);
    _movf(DestR, Zero, FCC0);
    _mov(Dest, DestR);
    break;
  }
  case InstFcmp::True: {
    Context.insert<InstFakeUse>(Src0R);
    Context.insert<InstFakeUse>(Src1R);
    _addiu(DestR, Zero, 1);
    _mov(Dest, DestR);
    break;
  }
  }
}

void TargetMIPS32::lower64Icmp(const InstIcmp *Instr) {
  Operand *Src0 = legalize(Instr->getSrc(0));
  Operand *Src1 = legalize(Instr->getSrc(1));
  Variable *Dest = Instr->getDest();
  InstIcmp::ICond Condition = Instr->getCondition();

  Variable *Src0LoR = legalizeToReg(loOperand(Src0));
  Variable *Src0HiR = legalizeToReg(hiOperand(Src0));
  Variable *Src1LoR = legalizeToReg(loOperand(Src1));
  Variable *Src1HiR = legalizeToReg(hiOperand(Src1));

  switch (Condition) {
  default:
    llvm_unreachable("unexpected condition");
    return;
  case InstIcmp::Eq: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _xor(T2, Src0LoR, Src1LoR);
    _or(T3, T1, T2);
    _sltiu(T4, T3, 1);
    _mov(Dest, T4);
    return;
  }
  case InstIcmp::Ne: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _xor(T2, Src0LoR, Src1LoR);
    _or(T3, T1, T2);
    _sltu(T4, getZero(), T3);
    _mov(Dest, T4);
    return;
  }
  case InstIcmp::Sgt: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _slt(T2, Src1HiR, Src0HiR);
    _sltu(T3, Src1LoR, Src0LoR);
    _movz(T2, T3, T1);
    _mov(Dest, T2);
    return;
  }
  case InstIcmp::Ugt: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _sltu(T2, Src1HiR, Src0HiR);
    _sltu(T3, Src1LoR, Src0LoR);
    _movz(T2, T3, T1);
    _mov(Dest, T2);
    return;
  }
  case InstIcmp::Sge: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _slt(T2, Src0HiR, Src1HiR);
    _xori(T3, T2, 1);
    _sltu(T4, Src0LoR, Src1LoR);
    _xori(T5, T4, 1);
    _movz(T3, T5, T1);
    _mov(Dest, T3);
    return;
  }
  case InstIcmp::Uge: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _sltu(T2, Src0HiR, Src1HiR);
    _xori(T3, T2, 1);
    _sltu(T4, Src0LoR, Src1LoR);
    _xori(T5, T4, 1);
    _movz(T3, T5, T1);
    _mov(Dest, T3);
    return;
  }
  case InstIcmp::Slt: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _slt(T2, Src0HiR, Src1HiR);
    _sltu(T3, Src0LoR, Src1LoR);
    _movz(T2, T3, T1);
    _mov(Dest, T2);
    return;
  }
  case InstIcmp::Ult: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _sltu(T2, Src0HiR, Src1HiR);
    _sltu(T3, Src0LoR, Src1LoR);
    _movz(T2, T3, T1);
    _mov(Dest, T2);
    return;
  }
  case InstIcmp::Sle: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _slt(T2, Src1HiR, Src0HiR);
    _xori(T3, T2, 1);
    _sltu(T4, Src1LoR, Src0LoR);
    _xori(T5, T4, 1);
    _movz(T3, T5, T1);
    _mov(Dest, T3);
    return;
  }
  case InstIcmp::Ule: {
    auto *T1 = I32Reg();
    auto *T2 = I32Reg();
    auto *T3 = I32Reg();
    auto *T4 = I32Reg();
    auto *T5 = I32Reg();
    _xor(T1, Src0HiR, Src1HiR);
    _sltu(T2, Src1HiR, Src0HiR);
    _xori(T3, T2, 1);
    _sltu(T4, Src1LoR, Src0LoR);
    _xori(T5, T4, 1);
    _movz(T3, T5, T1);
    _mov(Dest, T3);
    return;
  }
  }
}

void TargetMIPS32::lowerIcmp(const InstIcmp *Instr) {
  auto *Src0 = Instr->getSrc(0);
  auto *Src1 = Instr->getSrc(1);
  if (Src0->getType() == IceType_i64) {
    lower64Icmp(Instr);
    return;
  }
  Variable *Dest = Instr->getDest();
  if (isVectorType(Dest->getType())) {
    llvm::report_fatal_error("Icmp: Destination type is vector");
    return;
  }
  InstIcmp::ICond Cond = Instr->getCondition();
  auto *Src0R = legalizeToReg(Src0);
  auto *Src1R = legalizeToReg(Src1);
  const Type Src0Ty = Src0R->getType();
  const uint32_t ShAmt = INT32_BITS - getScalarIntBitWidth(Src0->getType());
  Variable *Src0RT = I32Reg();
  Variable *Src1RT = I32Reg();

  if (Src0Ty != IceType_i32) {
    _sll(Src0RT, Src0R, ShAmt);
    _sll(Src1RT, Src1R, ShAmt);
  } else {
    _mov(Src0RT, Src0R);
    _mov(Src1RT, Src1R);
  }

  switch (Cond) {
  case InstIcmp::Eq: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    _xor(T, Src0RT, Src1RT);
    _sltiu(DestT, T, 1);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Ne: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    auto *Zero = getZero();
    _xor(T, Src0RT, Src1RT);
    _sltu(DestT, Zero, T);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Ugt: {
    auto *DestT = I32Reg();
    _sltu(DestT, Src1RT, Src0RT);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Uge: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    _sltu(T, Src0RT, Src1RT);
    _xori(DestT, T, 1);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Ult: {
    auto *DestT = I32Reg();
    _sltu(DestT, Src0RT, Src1RT);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Ule: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    _sltu(T, Src1RT, Src0RT);
    _xori(DestT, T, 1);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Sgt: {
    auto *DestT = I32Reg();
    _slt(DestT, Src1RT, Src0RT);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Sge: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    _slt(T, Src0RT, Src1RT);
    _xori(DestT, T, 1);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Slt: {
    auto *DestT = I32Reg();
    _slt(DestT, Src0RT, Src1RT);
    _mov(Dest, DestT);
    return;
  }
  case InstIcmp::Sle: {
    auto *DestT = I32Reg();
    auto *T = I32Reg();
    _slt(T, Src1RT, Src0RT);
    _xori(DestT, T, 1);
    _mov(Dest, DestT);
    return;
  }
  default:
    llvm_unreachable("Invalid ICmp operator");
    return;
  }
}

void TargetMIPS32::lowerInsertElement(const InstInsertElement *Instr) {
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();
  Operand *Src2 = Instr->getSrc(2);
  if (const auto *Imm = llvm::dyn_cast<ConstantInteger32>(Src2)) {
    const uint32_t Index = Imm->getValue();
    // Vector to insert in
    auto *Src0 = legalizeUndef(Instr->getSrc(0));
    auto *Src0R = llvm::dyn_cast<VariableVecOn32>(Src0);
    // Number of elements in each container
    uint32_t ElemPerCont =
        typeNumElements(Src0->getType()) / Src0R->ContainersPerVector;
    // Source Element
    auto *Src = Src0R->getContainers()[Index / ElemPerCont];
    auto *SrcE = Src;
    if (ElemPerCont > 1)
      SrcE = legalizeToReg(Src);
    // Dest is a vector
    auto *VDest = llvm::dyn_cast<VariableVecOn32>(Dest);
    VDest->initVecElement(Func);
    // Temp vector variable
    auto *TDest = makeReg(DestTy);
    auto *TVDest = llvm::dyn_cast<VariableVecOn32>(TDest);
    TVDest->initVecElement(Func);
    // Destination element
    auto *DstE = TVDest->getContainers()[Index / ElemPerCont];
    // Element to insert
    auto *Src1R = legalizeToReg(Instr->getSrc(1));
    auto *TReg1 = makeReg(IceType_i32);
    auto *TReg2 = makeReg(IceType_i32);
    auto *TReg3 = makeReg(IceType_i32);
    auto *TReg4 = makeReg(IceType_i32);
    auto *TReg5 = makeReg(IceType_i32);
    auto *TDReg = makeReg(IceType_i32);
    // Position of the element in the container
    uint32_t PosInCont = Index % ElemPerCont;
    // Load source vector in a temporary vector
    for (SizeT i = 0; i < TVDest->ContainersPerVector; ++i) {
      auto *DCont = TVDest->getContainers()[i];
      // Do not define DstE as we are going to redefine it
      if (DCont == DstE)
        continue;
      auto *SCont = Src0R->getContainers()[i];
      auto *TReg = makeReg(IceType_i32);
      _mov(TReg, SCont);
      _mov(DCont, TReg);
    }
    // Insert the element
    if (ElemPerCont == 1) {
      _mov(DstE, Src1R);
    } else if (ElemPerCont == 2) {
      switch (PosInCont) {
      case 0:
        _andi(TReg1, Src1R, 0xffff); // Clear upper 16-bits of source
        _srl(TReg2, SrcE, 16);
        _sll(TReg3, TReg2, 16); // Clear lower 16-bits of element
        _or(TDReg, TReg1, TReg3);
        _mov(DstE, TDReg);
        break;
      case 1:
        _sll(TReg1, Src1R, 16); // Clear lower 16-bits  of source
        _sll(TReg2, SrcE, 16);
        _srl(TReg3, TReg2, 16); // Clear upper 16-bits of element
        _or(TDReg, TReg1, TReg3);
        _mov(DstE, TDReg);
        break;
      default:
        llvm::report_fatal_error("InsertElement: Invalid PosInCont");
        break;
      }
    } else if (ElemPerCont == 4) {
      switch (PosInCont) {
      case 0:
        _andi(TReg1, Src1R, 0xff); // Clear bits[31:8] of source
        _srl(TReg2, SrcE, 8);
        _sll(TReg3, TReg2, 8); // Clear bits[7:0] of element
        _or(TDReg, TReg1, TReg3);
        _mov(DstE, TDReg);
        break;
      case 1:
        _andi(TReg1, Src1R, 0xff); // Clear bits[31:8] of source
        _sll(TReg5, TReg1, 8);     // Position in the destination
        _lui(TReg2, Ctx->getConstantInt32(0xffff));
        _ori(TReg3, TReg2, 0x00ff);
        _and(TReg4, SrcE, TReg3); // Clear bits[15:8] of element
        _or(TDReg, TReg5, TReg4);
        _mov(DstE, TDReg);
        break;
      case 2:
        _andi(TReg1, Src1R, 0xff); // Clear bits[31:8] of source
        _sll(TReg5, TReg1, 16);    // Position in the destination
        _lui(TReg2, Ctx->getConstantInt32(0xff00));
        _ori(TReg3, TReg2, 0xffff);
        _and(TReg4, SrcE, TReg3); // Clear bits[15:8] of element
        _or(TDReg, TReg5, TReg4);
        _mov(DstE, TDReg);
        break;
      case 3:
        _sll(TReg1, Src1R, 24); // Position in the destination
        _sll(TReg2, SrcE, 8);
        _srl(TReg3, TReg2, 8); // Clear bits[31:24] of element
        _or(TDReg, TReg1, TReg3);
        _mov(DstE, TDReg);
        break;
      default:
        llvm::report_fatal_error("InsertElement: Invalid PosInCont");
        break;
      }
    }
    // Write back temporary vector to the destination
    auto *Assign = InstAssign::create(Func, Dest, TDest);
    lowerAssign(Assign);
    return;
  }
  llvm::report_fatal_error("InsertElement requires a constant index");
}

void TargetMIPS32::createArithInst(Intrinsics::AtomicRMWOperation Operation,
                                   Variable *Dest, Variable *Src0,
                                   Variable *Src1) {
  switch (Operation) {
  default:
    llvm::report_fatal_error("Unknown AtomicRMW operation");
  case Intrinsics::AtomicExchange:
    llvm::report_fatal_error("Can't handle Atomic xchg operation");
  case Intrinsics::AtomicAdd:
    _addu(Dest, Src0, Src1);
    break;
  case Intrinsics::AtomicAnd:
    _and(Dest, Src0, Src1);
    break;
  case Intrinsics::AtomicSub:
    _subu(Dest, Src0, Src1);
    break;
  case Intrinsics::AtomicOr:
    _or(Dest, Src0, Src1);
    break;
  case Intrinsics::AtomicXor:
    _xor(Dest, Src0, Src1);
    break;
  }
}

void TargetMIPS32::lowerIntrinsic(const InstIntrinsic *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = (Dest == nullptr) ? IceType_void : Dest->getType();

  Intrinsics::IntrinsicID ID = Instr->getIntrinsicID();
  switch (ID) {
  case Intrinsics::AtomicLoad: {
    assert(isScalarIntegerType(DestTy));
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(1)))) {
      Func->setError("Unexpected memory ordering for AtomicLoad");
      return;
    }
    if (DestTy == IceType_i64) {
      llvm::report_fatal_error("AtomicLoad.i64 should have been prelowered.");
      return;
    } else if (DestTy == IceType_i32) {
      auto *T1 = makeReg(DestTy);
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      auto *Base = legalizeToReg(Instr->getArg(0));
      auto *Addr = formMemoryOperand(Base, DestTy);
      InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
      InstMIPS32Label *Exit = InstMIPS32Label::create(Func, this);
      constexpr CfgNode *NoTarget = nullptr;
      _sync();
      Context.insert(Retry);
      _ll(T1, Addr);
      _br(NoTarget, NoTarget, T1, getZero(), Exit, CondMIPS32::Cond::NE);
      _addiu(RegAt, getZero(), 0); // Loaded value is zero here, writeback zero
      _sc(RegAt, Addr);
      _br(NoTarget, NoTarget, RegAt, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert(Exit);
      _sync();
      _mov(Dest, T1);
      Context.insert<InstFakeUse>(T1);
    } else {
      const uint32_t Mask = (1 << (CHAR_BITS * typeWidthInBytes(DestTy))) - 1;
      auto *Base = legalizeToReg(Instr->getArg(0));
      auto *T1 = makeReg(IceType_i32);
      auto *T2 = makeReg(IceType_i32);
      auto *T3 = makeReg(IceType_i32);
      auto *T4 = makeReg(IceType_i32);
      auto *T5 = makeReg(IceType_i32);
      auto *T6 = makeReg(IceType_i32);
      auto *SrcMask = makeReg(IceType_i32);
      auto *Tdest = makeReg(IceType_i32);
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
      InstMIPS32Label *Exit = InstMIPS32Label::create(Func, this);
      constexpr CfgNode *NoTarget = nullptr;
      _sync();
      _addiu(T1, getZero(), -4); // Address mask 0xFFFFFFFC
      _andi(T2, Base, 3);        // Last two bits of the address
      _and(T3, Base, T1);        // Align the address
      _sll(T4, T2, 3);
      _ori(T5, getZero(), Mask);
      _sllv(SrcMask, T5, T4); // Source mask
      auto *Addr = formMemoryOperand(T3, IceType_i32);
      Context.insert(Retry);
      _ll(T6, Addr);
      _and(Tdest, T6, SrcMask);
      _br(NoTarget, NoTarget, T6, getZero(), Exit, CondMIPS32::Cond::NE);
      _addiu(RegAt, getZero(), 0); // Loaded value is zero here, writeback zero
      _sc(RegAt, Addr);
      _br(NoTarget, NoTarget, RegAt, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert(Exit);
      auto *T7 = makeReg(IceType_i32);
      auto *T8 = makeReg(IceType_i32);
      _srlv(T7, Tdest, T4);
      _andi(T8, T7, Mask);
      _sync();
      _mov(Dest, T8);
      Context.insert<InstFakeUse>(T6);
      Context.insert<InstFakeUse>(SrcMask);
    }
    return;
  }
  case Intrinsics::AtomicStore: {
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal stores are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(2)))) {
      Func->setError("Unexpected memory ordering for AtomicStore");
      return;
    }
    auto *Val = Instr->getArg(0);
    auto Ty = Val->getType();
    if (Ty == IceType_i64) {
      llvm::report_fatal_error("AtomicStore.i64 should have been prelowered.");
      return;
    } else if (Ty == IceType_i32) {
      auto *Val = legalizeToReg(Instr->getArg(0));
      auto *Base = legalizeToReg(Instr->getArg(1));
      auto *Addr = formMemoryOperand(Base, Ty);
      InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
      constexpr CfgNode *NoTarget = nullptr;
      auto *T1 = makeReg(IceType_i32);
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      _sync();
      Context.insert(Retry);
      _ll(T1, Addr);
      _mov(RegAt, Val);
      _sc(RegAt, Addr);
      _br(NoTarget, NoTarget, RegAt, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(T1); // To keep LL alive
      _sync();
    } else {
      auto *Val = legalizeToReg(Instr->getArg(0));
      auto *Base = legalizeToReg(Instr->getArg(1));
      InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
      constexpr CfgNode *NoTarget = nullptr;
      auto *T1 = makeReg(IceType_i32);
      auto *T2 = makeReg(IceType_i32);
      auto *T3 = makeReg(IceType_i32);
      auto *T4 = makeReg(IceType_i32);
      auto *T5 = makeReg(IceType_i32);
      auto *T6 = makeReg(IceType_i32);
      auto *T7 = makeReg(IceType_i32);
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      auto *SrcMask = makeReg(IceType_i32);
      auto *DstMask = makeReg(IceType_i32);
      const uint32_t Mask = (1 << (CHAR_BITS * typeWidthInBytes(Ty))) - 1;
      _sync();
      _addiu(T1, getZero(), -4);
      _and(T7, Base, T1);
      auto *Addr = formMemoryOperand(T7, Ty);
      _andi(T2, Base, 3);
      _sll(T3, T2, 3);
      _ori(T4, getZero(), Mask);
      _sllv(T5, T4, T3);
      _sllv(T6, Val, T3);
      _nor(SrcMask, getZero(), T5);
      _and(DstMask, T6, T5);
      Context.insert(Retry);
      _ll(RegAt, Addr);
      _and(RegAt, RegAt, SrcMask);
      _or(RegAt, RegAt, DstMask);
      _sc(RegAt, Addr);
      _br(NoTarget, NoTarget, RegAt, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(SrcMask);
      Context.insert<InstFakeUse>(DstMask);
      _sync();
    }
    return;
  }
  case Intrinsics::AtomicCmpxchg: {
    assert(isScalarIntegerType(DestTy));
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)),
            getConstantMemoryOrder(Instr->getArg(4)))) {
      Func->setError("Unexpected memory ordering for AtomicCmpxchg");
      return;
    }

    InstMIPS32Label *Exit = InstMIPS32Label::create(Func, this);
    InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
    constexpr CfgNode *NoTarget = nullptr;
    auto *New = Instr->getArg(2);
    auto *Expected = Instr->getArg(1);
    auto *ActualAddress = Instr->getArg(0);

    if (DestTy == IceType_i64) {
      llvm::report_fatal_error(
          "AtomicCmpxchg.i64 should have been prelowered.");
      return;
    } else if (DestTy == IceType_i8 || DestTy == IceType_i16) {
      auto *NewR = legalizeToReg(New);
      auto *ExpectedR = legalizeToReg(Expected);
      auto *ActualAddressR = legalizeToReg(ActualAddress);
      const uint32_t ShiftAmount =
          (INT32_BITS - CHAR_BITS * typeWidthInBytes(DestTy));
      const uint32_t Mask = (1 << (CHAR_BITS * typeWidthInBytes(DestTy))) - 1;
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *T5 = I32Reg();
      auto *T6 = I32Reg();
      auto *T7 = I32Reg();
      auto *T8 = I32Reg();
      auto *T9 = I32Reg();
      _addiu(RegAt, getZero(), -4);
      _and(T1, ActualAddressR, RegAt);
      auto *Addr = formMemoryOperand(T1, DestTy);
      _andi(RegAt, ActualAddressR, 3);
      _sll(T2, RegAt, 3);
      _ori(RegAt, getZero(), Mask);
      _sllv(T3, RegAt, T2);
      _nor(T4, getZero(), T3);
      _andi(RegAt, ExpectedR, Mask);
      _sllv(T5, RegAt, T2);
      _andi(RegAt, NewR, Mask);
      _sllv(T6, RegAt, T2);
      _sync();
      Context.insert(Retry);
      _ll(T7, Addr);
      _and(T8, T7, T3);
      _br(NoTarget, NoTarget, T8, T5, Exit, CondMIPS32::Cond::NE);
      _and(RegAt, T7, T4);
      _or(T9, RegAt, T6);
      _sc(T9, Addr);
      _br(NoTarget, NoTarget, getZero(), T9, Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(getZero());
      Context.insert(Exit);
      _srlv(RegAt, T8, T2);
      _sll(RegAt, RegAt, ShiftAmount);
      _sra(RegAt, RegAt, ShiftAmount);
      _mov(Dest, RegAt);
      _sync();
      Context.insert<InstFakeUse>(T3);
      Context.insert<InstFakeUse>(T4);
      Context.insert<InstFakeUse>(T5);
      Context.insert<InstFakeUse>(T6);
      Context.insert<InstFakeUse>(T8);
      Context.insert<InstFakeUse>(ExpectedR);
      Context.insert<InstFakeUse>(NewR);
    } else {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *NewR = legalizeToReg(New);
      auto *ExpectedR = legalizeToReg(Expected);
      auto *ActualAddressR = legalizeToReg(ActualAddress);
      _sync();
      Context.insert(Retry);
      _ll(T1, formMemoryOperand(ActualAddressR, DestTy));
      _br(NoTarget, NoTarget, T1, ExpectedR, Exit, CondMIPS32::Cond::NE);
      _mov(T2, NewR);
      _sc(T2, formMemoryOperand(ActualAddressR, DestTy));
      _br(NoTarget, NoTarget, T2, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(getZero());
      Context.insert(Exit);
      _mov(Dest, T1);
      _sync();
      Context.insert<InstFakeUse>(ExpectedR);
      Context.insert<InstFakeUse>(NewR);
    }
    return;
  }
  case Intrinsics::AtomicRMW: {
    assert(isScalarIntegerType(DestTy));
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)))) {
      Func->setError("Unexpected memory ordering for AtomicRMW");
      return;
    }

    constexpr CfgNode *NoTarget = nullptr;
    InstMIPS32Label *Retry = InstMIPS32Label::create(Func, this);
    auto Operation = static_cast<Intrinsics::AtomicRMWOperation>(
        llvm::cast<ConstantInteger32>(Instr->getArg(0))->getValue());
    auto *New = Instr->getArg(2);
    auto *ActualAddress = Instr->getArg(1);

    if (DestTy == IceType_i64) {
      llvm::report_fatal_error("AtomicRMW.i64 should have been prelowered.");
      return;
    } else if (DestTy == IceType_i8 || DestTy == IceType_i16) {
      const uint32_t ShiftAmount =
          INT32_BITS - (CHAR_BITS * typeWidthInBytes(DestTy));
      const uint32_t Mask = (1 << (CHAR_BITS * typeWidthInBytes(DestTy))) - 1;
      auto *NewR = legalizeToReg(New);
      auto *ActualAddressR = legalizeToReg(ActualAddress);
      auto *RegAt = getPhysicalRegister(RegMIPS32::Reg_AT);
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *T5 = I32Reg();
      auto *T6 = I32Reg();
      auto *T7 = I32Reg();
      _sync();
      _addiu(RegAt, getZero(), -4);
      _and(T1, ActualAddressR, RegAt);
      _andi(RegAt, ActualAddressR, 3);
      _sll(T2, RegAt, 3);
      _ori(RegAt, getZero(), Mask);
      _sllv(T3, RegAt, T2);
      _nor(T4, getZero(), T3);
      _sllv(T5, NewR, T2);
      Context.insert(Retry);
      _ll(T6, formMemoryOperand(T1, DestTy));
      if (Operation != Intrinsics::AtomicExchange) {
        createArithInst(Operation, RegAt, T6, T5);
        _and(RegAt, RegAt, T3);
      }
      _and(T7, T6, T4);
      if (Operation == Intrinsics::AtomicExchange) {
        _or(RegAt, T7, T5);
      } else {
        _or(RegAt, T7, RegAt);
      }
      _sc(RegAt, formMemoryOperand(T1, DestTy));
      _br(NoTarget, NoTarget, RegAt, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(getZero());
      _and(RegAt, T6, T3);
      _srlv(RegAt, RegAt, T2);
      _sll(RegAt, RegAt, ShiftAmount);
      _sra(RegAt, RegAt, ShiftAmount);
      _mov(Dest, RegAt);
      _sync();
      Context.insert<InstFakeUse>(NewR);
      Context.insert<InstFakeUse>(Dest);
    } else {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *NewR = legalizeToReg(New);
      auto *ActualAddressR = legalizeToReg(ActualAddress);
      _sync();
      Context.insert(Retry);
      _ll(T1, formMemoryOperand(ActualAddressR, DestTy));
      if (Operation == Intrinsics::AtomicExchange) {
        _mov(T2, NewR);
      } else {
        createArithInst(Operation, T2, T1, NewR);
      }
      _sc(T2, formMemoryOperand(ActualAddressR, DestTy));
      _br(NoTarget, NoTarget, T2, getZero(), Retry, CondMIPS32::Cond::EQ);
      Context.insert<InstFakeUse>(getZero());
      _mov(Dest, T1);
      _sync();
      Context.insert<InstFakeUse>(NewR);
      Context.insert<InstFakeUse>(Dest);
    }
    return;
  }
  case Intrinsics::AtomicFence:
  case Intrinsics::AtomicFenceAll:
    assert(Dest == nullptr);
    _sync();
    return;
  case Intrinsics::AtomicIsLockFree: {
    Operand *ByteSize = Instr->getArg(0);
    auto *CI = llvm::dyn_cast<ConstantInteger32>(ByteSize);
    auto *T = I32Reg();
    if (CI == nullptr) {
      // The PNaCl ABI requires the byte size to be a compile-time constant.
      Func->setError("AtomicIsLockFree byte size should be compile-time const");
      return;
    }
    static constexpr int32_t NotLockFree = 0;
    static constexpr int32_t LockFree = 1;
    int32_t Result = NotLockFree;
    switch (CI->getValue()) {
    case 1:
    case 2:
    case 4:
      Result = LockFree;
      break;
    }
    _addiu(T, getZero(), Result);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::Bswap: {
    auto *Src = Instr->getArg(0);
    const Type SrcTy = Src->getType();
    assert(SrcTy == IceType_i16 || SrcTy == IceType_i32 ||
           SrcTy == IceType_i64);
    switch (SrcTy) {
    case IceType_i16: {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *SrcR = legalizeToReg(Src);
      _sll(T1, SrcR, 8);
      _lui(T2, Ctx->getConstantInt32(255));
      _and(T1, T1, T2);
      _sll(T3, SrcR, 24);
      _or(T1, T3, T1);
      _srl(T4, T1, 16);
      _mov(Dest, T4);
      return;
    }
    case IceType_i32: {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *T5 = I32Reg();
      auto *SrcR = legalizeToReg(Src);
      _srl(T1, SrcR, 24);
      _srl(T2, SrcR, 8);
      _andi(T2, T2, 0xFF00);
      _or(T1, T2, T1);
      _sll(T4, SrcR, 8);
      _lui(T3, Ctx->getConstantInt32(255));
      _and(T4, T4, T3);
      _sll(T5, SrcR, 24);
      _or(T4, T5, T4);
      _or(T4, T4, T1);
      _mov(Dest, T4);
      return;
    }
    case IceType_i64: {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *T5 = I32Reg();
      auto *T6 = I32Reg();
      auto *T7 = I32Reg();
      auto *T8 = I32Reg();
      auto *T9 = I32Reg();
      auto *T10 = I32Reg();
      auto *T11 = I32Reg();
      auto *T12 = I32Reg();
      auto *T13 = I32Reg();
      auto *T14 = I32Reg();
      auto *T15 = I32Reg();
      auto *T16 = I32Reg();
      auto *T17 = I32Reg();
      auto *T18 = I32Reg();
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Src = legalizeUndef(Src);
      auto *SrcLoR = legalizeToReg(loOperand(Src));
      auto *SrcHiR = legalizeToReg(hiOperand(Src));
      _sll(T1, SrcHiR, 8);
      _srl(T2, SrcHiR, 24);
      _srl(T3, SrcHiR, 8);
      _andi(T3, T3, 0xFF00);
      _lui(T4, Ctx->getConstantInt32(255));
      _or(T5, T3, T2);
      _and(T6, T1, T4);
      _sll(T7, SrcHiR, 24);
      _or(T8, T7, T6);
      _srl(T9, SrcLoR, 24);
      _srl(T10, SrcLoR, 8);
      _andi(T11, T10, 0xFF00);
      _or(T12, T8, T5);
      _or(T13, T11, T9);
      _sll(T14, SrcLoR, 8);
      _and(T15, T14, T4);
      _sll(T16, SrcLoR, 24);
      _or(T17, T16, T15);
      _or(T18, T17, T13);
      _mov(DestLo, T12);
      _mov(DestHi, T18);
      return;
    }
    default:
      llvm::report_fatal_error("Control flow should never have reached here.");
    }
    return;
  }
  case Intrinsics::Ctpop: {
    llvm::report_fatal_error("Ctpop should have been prelowered.");
    return;
  }
  case Intrinsics::Ctlz: {
    auto *Src = Instr->getArg(0);
    const Type SrcTy = Src->getType();
    assert(SrcTy == IceType_i32 || SrcTy == IceType_i64);
    switch (SrcTy) {
    case IceType_i32: {
      auto *T = I32Reg();
      auto *SrcR = legalizeToReg(Src);
      _clz(T, SrcR);
      _mov(Dest, T);
      break;
    }
    case IceType_i64: {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *SrcHiR = legalizeToReg(hiOperand(Src));
      Variable *SrcLoR = legalizeToReg(loOperand(Src));
      _clz(T1, SrcHiR);
      _clz(T2, SrcLoR);
      _addiu(T3, T2, 32);
      _movn(T3, T1, SrcHiR);
      _addiu(T4, getZero(), 0);
      _mov(DestHi, T4);
      _mov(DestLo, T3);
      break;
    }
    default:
      llvm::report_fatal_error("Control flow should never have reached here.");
    }
    break;
  }
  case Intrinsics::Cttz: {
    auto *Src = Instr->getArg(0);
    const Type SrcTy = Src->getType();
    assert(SrcTy == IceType_i32 || SrcTy == IceType_i64);
    switch (SrcTy) {
    case IceType_i32: {
      auto *T1 = I32Reg();
      auto *T2 = I32Reg();
      auto *T3 = I32Reg();
      auto *T4 = I32Reg();
      auto *T5 = I32Reg();
      auto *T6 = I32Reg();
      auto *SrcR = legalizeToReg(Src);
      _addiu(T1, SrcR, -1);
      _not(T2, SrcR);
      _and(T3, T2, T1);
      _clz(T4, T3);
      _addiu(T5, getZero(), 32);
      _subu(T6, T5, T4);
      _mov(Dest, T6);
      break;
    }
    case IceType_i64: {
      auto *THi1 = I32Reg();
      auto *THi2 = I32Reg();
      auto *THi3 = I32Reg();
      auto *THi4 = I32Reg();
      auto *THi5 = I32Reg();
      auto *THi6 = I32Reg();
      auto *TLo1 = I32Reg();
      auto *TLo2 = I32Reg();
      auto *TLo3 = I32Reg();
      auto *TLo4 = I32Reg();
      auto *TLo5 = I32Reg();
      auto *TLo6 = I32Reg();
      auto *TResHi = I32Reg();
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *SrcHiR = legalizeToReg(hiOperand(Src));
      Variable *SrcLoR = legalizeToReg(loOperand(Src));
      _addiu(THi1, SrcHiR, -1);
      _not(THi2, SrcHiR);
      _and(THi3, THi2, THi1);
      _clz(THi4, THi3);
      _addiu(THi5, getZero(), 64);
      _subu(THi6, THi5, THi4);
      _addiu(TLo1, SrcLoR, -1);
      _not(TLo2, SrcLoR);
      _and(TLo3, TLo2, TLo1);
      _clz(TLo4, TLo3);
      _addiu(TLo5, getZero(), 32);
      _subu(TLo6, TLo5, TLo4);
      _movn(THi6, TLo6, SrcLoR);
      _addiu(TResHi, getZero(), 0);
      _mov(DestHi, TResHi);
      _mov(DestLo, THi6);
      break;
    }
    default:
      llvm::report_fatal_error("Control flow should never have reached here.");
    }
    return;
  }
  case Intrinsics::Fabs: {
    if (isScalarFloatingType(DestTy)) {
      Variable *T = makeReg(DestTy);
      if (DestTy == IceType_f32) {
        _abs_s(T, legalizeToReg(Instr->getArg(0)));
      } else {
        _abs_d(T, legalizeToReg(Instr->getArg(0)));
      }
      _mov(Dest, T);
    }
    return;
  }
  case Intrinsics::Longjmp: {
    llvm::report_fatal_error("longjmp should have been prelowered.");
    return;
  }
  case Intrinsics::Memcpy: {
    llvm::report_fatal_error("memcpy should have been prelowered.");
    return;
  }
  case Intrinsics::Memmove: {
    llvm::report_fatal_error("memmove should have been prelowered.");
    return;
  }
  case Intrinsics::Memset: {
    llvm::report_fatal_error("memset should have been prelowered.");
    return;
  }
  case Intrinsics::Setjmp: {
    llvm::report_fatal_error("setjmp should have been prelowered.");
    return;
  }
  case Intrinsics::Sqrt: {
    if (isScalarFloatingType(DestTy)) {
      Variable *T = makeReg(DestTy);
      if (DestTy == IceType_f32) {
        _sqrt_s(T, legalizeToReg(Instr->getArg(0)));
      } else {
        _sqrt_d(T, legalizeToReg(Instr->getArg(0)));
      }
      _mov(Dest, T);
    } else {
      UnimplementedLoweringError(this, Instr);
    }
    return;
  }
  case Intrinsics::Stacksave: {
    Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
    _mov(Dest, SP);
    return;
  }
  case Intrinsics::Stackrestore: {
    Variable *SP = getPhysicalRegister(RegMIPS32::Reg_SP);
    Variable *Val = legalizeToReg(Instr->getArg(0));
    _mov(SP, Val);
    return;
  }
  case Intrinsics::Trap: {
    const uint32_t TrapCodeZero = 0;
    _teq(getZero(), getZero(), TrapCodeZero);
    return;
  }
  case Intrinsics::LoadSubVector: {
    UnimplementedLoweringError(this, Instr);
    return;
  }
  case Intrinsics::StoreSubVector: {
    UnimplementedLoweringError(this, Instr);
    return;
  }
  default: // UnknownIntrinsic
    Func->setError("Unexpected intrinsic");
    return;
  }
  return;
}

void TargetMIPS32::lowerLoad(const InstLoad *Instr) {
  // A Load instruction can be treated the same as an Assign instruction, after
  // the source operand is transformed into an OperandMIPS32Mem operand.
  Type Ty = Instr->getDest()->getType();
  Operand *Src0 = formMemoryOperand(Instr->getLoadAddress(), Ty);
  Variable *DestLoad = Instr->getDest();
  auto *Assign = InstAssign::create(Func, DestLoad, Src0);
  lowerAssign(Assign);
}

namespace {
void dumpAddressOpt(const Cfg *Func, const Variable *Base, int32_t Offset,
                    const Inst *Reason) {
  if (!BuildDefs::dump())
    return;
  if (!Func->isVerbose(IceV_AddrOpt))
    return;
  OstreamLocker _(Func->getContext());
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "Instruction: ";
  Reason->dumpDecorated(Func);
  Str << "  results in Base=";
  if (Base)
    Base->dump(Func);
  else
    Str << "<null>";
  Str << ", Offset=" << Offset << "\n";
}

bool matchAssign(const VariablesMetadata *VMetadata, Variable **Var,
                 int32_t *Offset, const Inst **Reason) {
  // Var originates from Var=SrcVar ==> set Var:=SrcVar
  if (*Var == nullptr)
    return false;
  const Inst *VarAssign = VMetadata->getSingleDefinition(*Var);
  if (!VarAssign)
    return false;
  assert(!VMetadata->isMultiDef(*Var));
  if (!llvm::isa<InstAssign>(VarAssign))
    return false;

  Operand *SrcOp = VarAssign->getSrc(0);
  bool Optimized = false;
  if (auto *SrcVar = llvm::dyn_cast<Variable>(SrcOp)) {
    if (!VMetadata->isMultiDef(SrcVar) ||
        // TODO: ensure SrcVar stays single-BB
        false) {
      Optimized = true;
      *Var = SrcVar;
    } else if (auto *Const = llvm::dyn_cast<ConstantInteger32>(SrcOp)) {
      int32_t MoreOffset = Const->getValue();
      int32_t NewOffset = MoreOffset + *Offset;
      if (Utils::WouldOverflowAdd(*Offset, MoreOffset))
        return false;
      *Var = nullptr;
      *Offset += NewOffset;
      Optimized = true;
    }
  }

  if (Optimized) {
    *Reason = VarAssign;
  }

  return Optimized;
}

bool isAddOrSub(const Inst *Instr, InstArithmetic::OpKind *Kind) {
  if (const auto *Arith = llvm::dyn_cast<InstArithmetic>(Instr)) {
    switch (Arith->getOp()) {
    default:
      return false;
    case InstArithmetic::Add:
    case InstArithmetic::Sub:
      *Kind = Arith->getOp();
      return true;
    }
  }
  return false;
}

bool matchOffsetBase(const VariablesMetadata *VMetadata, Variable **Base,
                     int32_t *Offset, const Inst **Reason) {
  // Base is Base=Var+Const || Base is Base=Const+Var ==>
  //   set Base=Var, Offset+=Const
  // Base is Base=Var-Const ==>
  //   set Base=Var, Offset-=Const
  if (*Base == nullptr)
    return false;
  const Inst *BaseInst = VMetadata->getSingleDefinition(*Base);
  if (BaseInst == nullptr) {
    return false;
  }
  assert(!VMetadata->isMultiDef(*Base));

  auto *ArithInst = llvm::dyn_cast<const InstArithmetic>(BaseInst);
  if (ArithInst == nullptr)
    return false;
  InstArithmetic::OpKind Kind;
  if (!isAddOrSub(ArithInst, &Kind))
    return false;
  bool IsAdd = Kind == InstArithmetic::Add;
  Operand *Src0 = ArithInst->getSrc(0);
  Operand *Src1 = ArithInst->getSrc(1);
  auto *Var0 = llvm::dyn_cast<Variable>(Src0);
  auto *Var1 = llvm::dyn_cast<Variable>(Src1);
  auto *Const0 = llvm::dyn_cast<ConstantInteger32>(Src0);
  auto *Const1 = llvm::dyn_cast<ConstantInteger32>(Src1);
  Variable *NewBase = nullptr;
  int32_t NewOffset = *Offset;

  if (Var0 == nullptr && Const0 == nullptr) {
    assert(llvm::isa<ConstantRelocatable>(Src0));
    return false;
  }

  if (Var1 == nullptr && Const1 == nullptr) {
    assert(llvm::isa<ConstantRelocatable>(Src1));
    return false;
  }

  if (Var0 && Var1)
    // TODO(jpp): merge base/index splitting into here.
    return false;
  if (!IsAdd && Var1)
    return false;
  if (Var0)
    NewBase = Var0;
  else if (Var1)
    NewBase = Var1;
  // Compute the updated constant offset.
  if (Const0) {
    int32_t MoreOffset = IsAdd ? Const0->getValue() : -Const0->getValue();
    if (Utils::WouldOverflowAdd(NewOffset, MoreOffset))
      return false;
    NewOffset += MoreOffset;
  }
  if (Const1) {
    int32_t MoreOffset = IsAdd ? Const1->getValue() : -Const1->getValue();
    if (Utils::WouldOverflowAdd(NewOffset, MoreOffset))
      return false;
    NewOffset += MoreOffset;
  }

  // Update the computed address parameters once we are sure optimization
  // is valid.
  *Base = NewBase;
  *Offset = NewOffset;
  *Reason = BaseInst;
  return true;
}
} // end of anonymous namespace

OperandMIPS32Mem *TargetMIPS32::formAddressingMode(Type Ty, Cfg *Func,
                                                   const Inst *LdSt,
                                                   Operand *Base) {
  assert(Base != nullptr);
  int32_t OffsetImm = 0;

  Func->resetCurrentNode();
  if (Func->isVerbose(IceV_AddrOpt)) {
    OstreamLocker _(Func->getContext());
    Ostream &Str = Func->getContext()->getStrDump();
    Str << "\nAddress mode formation:\t";
    LdSt->dumpDecorated(Func);
  }

  if (isVectorType(Ty)) {
    return nullptr;
  }

  auto *BaseVar = llvm::dyn_cast<Variable>(Base);
  if (BaseVar == nullptr)
    return nullptr;

  const VariablesMetadata *VMetadata = Func->getVMetadata();
  const Inst *Reason = nullptr;

  do {
    if (Reason != nullptr) {
      dumpAddressOpt(Func, BaseVar, OffsetImm, Reason);
      Reason = nullptr;
    }

    if (matchAssign(VMetadata, &BaseVar, &OffsetImm, &Reason)) {
      continue;
    }

    if (matchOffsetBase(VMetadata, &BaseVar, &OffsetImm, &Reason)) {
      continue;
    }
  } while (Reason);

  if (BaseVar == nullptr) {
    // We need base register rather than just OffsetImm. Move the OffsetImm to
    // BaseVar and form 0(BaseVar) addressing.
    const Type PointerType = getPointerType();
    BaseVar = makeReg(PointerType);
    Context.insert<InstAssign>(BaseVar, Ctx->getConstantInt32(OffsetImm));
    OffsetImm = 0;
  } else if (OffsetImm != 0) {
    // If the OffsetImm is more than signed 16-bit value then add it in the
    // BaseVar and form 0(BaseVar) addressing.
    const int32_t PositiveOffset = OffsetImm > 0 ? OffsetImm : -OffsetImm;
    const InstArithmetic::OpKind Op =
        OffsetImm > 0 ? InstArithmetic::Add : InstArithmetic::Sub;
    constexpr bool ZeroExt = false;
    if (!OperandMIPS32Mem::canHoldOffset(Ty, ZeroExt, OffsetImm)) {
      const Type PointerType = getPointerType();
      Variable *T = makeReg(PointerType);
      Context.insert<InstArithmetic>(Op, T, BaseVar,
                                     Ctx->getConstantInt32(PositiveOffset));
      BaseVar = T;
      OffsetImm = 0;
    }
  }

  assert(BaseVar != nullptr);
  assert(OffsetImm < 0 ? (-OffsetImm & 0x0000ffff) == -OffsetImm
                       : (OffsetImm & 0x0000ffff) == OffsetImm);

  return OperandMIPS32Mem::create(
      Func, Ty, BaseVar,
      llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(OffsetImm)));
}

void TargetMIPS32::doAddressOptLoad() {
  Inst *Instr = iteratorToInst(Context.getCur());
  assert(llvm::isa<InstLoad>(Instr));
  Variable *Dest = Instr->getDest();
  Operand *Addr = Instr->getSrc(0);
  if (OperandMIPS32Mem *Mem =
          formAddressingMode(Dest->getType(), Func, Instr, Addr)) {
    Instr->setDeleted();
    Context.insert<InstLoad>(Dest, Mem);
  }
}

void TargetMIPS32::lowerPhi(const InstPhi * /*Instr*/) {
  Func->setError("Phi found in regular instruction list");
}

void TargetMIPS32::lowerRet(const InstRet *Instr) {
  Variable *Reg = nullptr;
  if (Instr->hasRetValue()) {
    Operand *Src0 = Instr->getRetValue();
    switch (Src0->getType()) {
    case IceType_f32: {
      Operand *Src0F = legalizeToReg(Src0);
      Reg = makeReg(Src0F->getType(), RegMIPS32::Reg_F0);
      _mov(Reg, Src0F);
      break;
    }
    case IceType_f64: {
      Operand *Src0F = legalizeToReg(Src0);
      Reg = makeReg(Src0F->getType(), RegMIPS32::Reg_F0F1);
      _mov(Reg, Src0F);
      break;
    }
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32: {
      Operand *Src0F = legalizeToReg(Src0);
      Reg = makeReg(Src0F->getType(), RegMIPS32::Reg_V0);
      _mov(Reg, Src0F);
      break;
    }
    case IceType_i64: {
      Src0 = legalizeUndef(Src0);
      Variable *R0 = legalizeToReg(loOperand(Src0), RegMIPS32::Reg_V0);
      Variable *R1 = legalizeToReg(hiOperand(Src0), RegMIPS32::Reg_V1);
      Reg = R0;
      Context.insert<InstFakeUse>(R1);
      break;
    }
    case IceType_v4i1:
    case IceType_v8i1:
    case IceType_v16i1:
    case IceType_v16i8:
    case IceType_v8i16:
    case IceType_v4i32: {
      auto *SrcVec = llvm::dyn_cast<VariableVecOn32>(legalizeUndef(Src0));
      Variable *V0 =
          legalizeToReg(SrcVec->getContainers()[0], RegMIPS32::Reg_V0);
      Variable *V1 =
          legalizeToReg(SrcVec->getContainers()[1], RegMIPS32::Reg_V1);
      Variable *A0 =
          legalizeToReg(SrcVec->getContainers()[2], RegMIPS32::Reg_A0);
      Variable *A1 =
          legalizeToReg(SrcVec->getContainers()[3], RegMIPS32::Reg_A1);
      Reg = V0;
      Context.insert<InstFakeUse>(V1);
      Context.insert<InstFakeUse>(A0);
      Context.insert<InstFakeUse>(A1);
      break;
    }
    case IceType_v4f32: {
      auto *SrcVec = llvm::dyn_cast<VariableVecOn32>(legalizeUndef(Src0));
      Reg = getImplicitRet();
      auto *RegT = legalizeToReg(Reg);
      // Return the vector through buffer in implicit argument a0
      for (SizeT i = 0; i < SrcVec->ContainersPerVector; ++i) {
        OperandMIPS32Mem *Mem = OperandMIPS32Mem::create(
            Func, IceType_f32, RegT,
            llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(i * 4)));
        Variable *Var = legalizeToReg(SrcVec->getContainers()[i]);
        _sw(Var, Mem);
      }
      Variable *V0 = makeReg(IceType_i32, RegMIPS32::Reg_V0);
      _mov(V0, Reg); // move v0,a0
      Context.insert<InstFakeUse>(Reg);
      Context.insert<InstFakeUse>(V0);
      break;
    }
    default:
      llvm::report_fatal_error("Ret: Invalid type.");
      break;
    }
  }
  _ret(getPhysicalRegister(RegMIPS32::Reg_RA), Reg);
}

void TargetMIPS32::lowerSelect(const InstSelect *Instr) {
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();

  if (isVectorType(DestTy)) {
    llvm::report_fatal_error("Select: Destination type is vector");
    return;
  }

  Variable *DestR = nullptr;
  Variable *DestHiR = nullptr;
  Variable *SrcTR = nullptr;
  Variable *SrcTHiR = nullptr;
  Variable *SrcFR = nullptr;
  Variable *SrcFHiR = nullptr;

  if (DestTy == IceType_i64) {
    DestR = llvm::cast<Variable>(loOperand(Dest));
    DestHiR = llvm::cast<Variable>(hiOperand(Dest));
    SrcTR = legalizeToReg(loOperand(legalizeUndef(Instr->getTrueOperand())));
    SrcTHiR = legalizeToReg(hiOperand(legalizeUndef(Instr->getTrueOperand())));
    SrcFR = legalizeToReg(loOperand(legalizeUndef(Instr->getFalseOperand())));
    SrcFHiR = legalizeToReg(hiOperand(legalizeUndef(Instr->getFalseOperand())));
  } else {
    SrcTR = legalizeToReg(legalizeUndef(Instr->getTrueOperand()));
    SrcFR = legalizeToReg(legalizeUndef(Instr->getFalseOperand()));
  }

  Variable *ConditionR = legalizeToReg(Instr->getCondition());

  assert(Instr->getCondition()->getType() == IceType_i1);

  switch (DestTy) {
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
    _movn(SrcFR, SrcTR, ConditionR);
    _mov(Dest, SrcFR);
    break;
  case IceType_i64:
    _movn(SrcFR, SrcTR, ConditionR);
    _movn(SrcFHiR, SrcTHiR, ConditionR);
    _mov(DestR, SrcFR);
    _mov(DestHiR, SrcFHiR);
    break;
  case IceType_f32:
    _movn_s(SrcFR, SrcTR, ConditionR);
    _mov(Dest, SrcFR);
    break;
  case IceType_f64:
    _movn_d(SrcFR, SrcTR, ConditionR);
    _mov(Dest, SrcFR);
    break;
  default:
    llvm::report_fatal_error("Select: Invalid type.");
  }
}

void TargetMIPS32::lowerShuffleVector(const InstShuffleVector *Instr) {
  UnimplementedLoweringError(this, Instr);
}

void TargetMIPS32::lowerStore(const InstStore *Instr) {
  Operand *Value = Instr->getData();
  Operand *Addr = Instr->getStoreAddress();
  OperandMIPS32Mem *NewAddr = formMemoryOperand(Addr, Value->getType());
  Type Ty = NewAddr->getType();

  if (Ty == IceType_i64) {
    Value = legalizeUndef(Value);
    Variable *ValueHi = legalizeToReg(hiOperand(Value));
    Variable *ValueLo = legalizeToReg(loOperand(Value));
    _sw(ValueHi, llvm::cast<OperandMIPS32Mem>(hiOperand(NewAddr)));
    _sw(ValueLo, llvm::cast<OperandMIPS32Mem>(loOperand(NewAddr)));
  } else if (isVectorType(Value->getType())) {
    auto *DataVec = llvm::dyn_cast<VariableVecOn32>(Value);
    for (SizeT i = 0; i < DataVec->ContainersPerVector; ++i) {
      auto *DCont = legalizeToReg(DataVec->getContainers()[i]);
      auto *MCont = llvm::cast<OperandMIPS32Mem>(
          getOperandAtIndex(NewAddr, IceType_i32, i));
      _sw(DCont, MCont);
    }
  } else {
    Variable *ValueR = legalizeToReg(Value);
    _sw(ValueR, NewAddr);
  }
}

void TargetMIPS32::doAddressOptStore() {
  Inst *Instr = iteratorToInst(Context.getCur());
  assert(llvm::isa<InstStore>(Instr));
  Operand *Src = Instr->getSrc(0);
  Operand *Addr = Instr->getSrc(1);
  if (OperandMIPS32Mem *Mem =
          formAddressingMode(Src->getType(), Func, Instr, Addr)) {
    Instr->setDeleted();
    Context.insert<InstStore>(Src, Mem);
  }
}

void TargetMIPS32::lowerSwitch(const InstSwitch *Instr) {
  Operand *Src = Instr->getComparison();
  SizeT NumCases = Instr->getNumCases();
  if (Src->getType() == IceType_i64) {
    Src = legalizeUndef(Src);
    Variable *Src0Lo = legalizeToReg(loOperand(Src));
    Variable *Src0Hi = legalizeToReg(hiOperand(Src));
    for (SizeT I = 0; I < NumCases; ++I) {
      Operand *ValueLo = Ctx->getConstantInt32(Instr->getValue(I));
      Operand *ValueHi = Ctx->getConstantInt32(Instr->getValue(I) >> 32);
      CfgNode *TargetTrue = Instr->getLabel(I);
      constexpr CfgNode *NoTarget = nullptr;
      ValueHi = legalizeToReg(ValueHi);
      InstMIPS32Label *IntraLabel = InstMIPS32Label::create(Func, this);
      _br(NoTarget, NoTarget, Src0Hi, ValueHi, IntraLabel,
          CondMIPS32::Cond::NE);
      ValueLo = legalizeToReg(ValueLo);
      _br(NoTarget, TargetTrue, Src0Lo, ValueLo, CondMIPS32::Cond::EQ);
      Context.insert(IntraLabel);
    }
    _br(Instr->getLabelDefault());
    return;
  }
  Variable *SrcVar = legalizeToReg(Src);
  assert(SrcVar->mustHaveReg());
  for (SizeT I = 0; I < NumCases; ++I) {
    Operand *Value = Ctx->getConstantInt32(Instr->getValue(I));
    CfgNode *TargetTrue = Instr->getLabel(I);
    constexpr CfgNode *NoTargetFalse = nullptr;
    Value = legalizeToReg(Value);
    _br(NoTargetFalse, TargetTrue, SrcVar, Value, CondMIPS32::Cond::EQ);
  }
  _br(Instr->getLabelDefault());
}

void TargetMIPS32::lowerBreakpoint(const InstBreakpoint *Instr) {
  UnimplementedLoweringError(this, Instr);
}

void TargetMIPS32::lowerUnreachable(const InstUnreachable *) {
  const uint32_t TrapCodeZero = 0;
  _teq(getZero(), getZero(), TrapCodeZero);
}

void TargetMIPS32::lowerOther(const Inst *Instr) {
  if (llvm::isa<InstMIPS32Sync>(Instr)) {
    _sync();
  } else {
    TargetLowering::lowerOther(Instr);
  }
}

// Turn an i64 Phi instruction into a pair of i32 Phi instructions, to preserve
// integrity of liveness analysis. Undef values are also turned into zeroes,
// since loOperand() and hiOperand() don't expect Undef input.
void TargetMIPS32::prelowerPhis() {
  PhiLowering::prelowerPhis32Bit<TargetMIPS32>(this, Context.getNode(), Func);
}

void TargetMIPS32::postLower() {
  if (Func->getOptLevel() == Opt_m1)
    return;
  markRedefinitions();
  Context.availabilityUpdate();
}

/* TODO(jvoung): avoid duplicate symbols with multiple targets.
void ConstantUndef::emitWithoutDollar(GlobalContext *) const {
  llvm_unreachable("Not expecting to emitWithoutDollar undef");
}

void ConstantUndef::emit(GlobalContext *) const {
  llvm_unreachable("undef value encountered by emitter.");
}
*/

TargetDataMIPS32::TargetDataMIPS32(GlobalContext *Ctx)
    : TargetDataLowering(Ctx) {}

// Generate .MIPS.abiflags section. This section contains a versioned data
// structure with essential information required for loader to determine the
// requirements of the application.
void TargetDataMIPS32::emitTargetRODataSections() {
  struct MipsABIFlagsSection Flags;
  ELFObjectWriter *Writer = Ctx->getObjectWriter();
  const std::string Name = ".MIPS.abiflags";
  const llvm::ELF::Elf64_Word ShType = llvm::ELF::SHT_MIPS_ABIFLAGS;
  const llvm::ELF::Elf64_Xword ShFlags = llvm::ELF::SHF_ALLOC;
  const llvm::ELF::Elf64_Xword ShAddralign = 8;
  const llvm::ELF::Elf64_Xword ShEntsize = sizeof(Flags);
  Writer->writeTargetRODataSection(
      Name, ShType, ShFlags, ShAddralign, ShEntsize,
      llvm::StringRef(reinterpret_cast<const char *>(&Flags), sizeof(Flags)));
}

void TargetDataMIPS32::lowerGlobals(const VariableDeclarationList &Vars,
                                    const std::string &SectionSuffix) {
  const bool IsPIC = false;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();
    Writer->writeDataSection(Vars, llvm::ELF::R_MIPS_32, SectionSuffix, IsPIC);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    OstreamLocker L(Ctx);
    for (const VariableDeclaration *Var : Vars) {
      if (getFlags().matchTranslateOnly(Var->getName(), 0)) {
        emitGlobal(*Var, SectionSuffix);
      }
    }
  } break;
  }
}

namespace {
template <typename T> struct ConstantPoolEmitterTraits;

static_assert(sizeof(uint64_t) == 8,
              "uint64_t is supposed to be 8 bytes wide.");

// TODO(jaydeep.patil): implement the following when implementing constant
// randomization:
//  * template <> struct ConstantPoolEmitterTraits<uint8_t>
//  * template <> struct ConstantPoolEmitterTraits<uint16_t>
//  * template <> struct ConstantPoolEmitterTraits<uint32_t>
template <> struct ConstantPoolEmitterTraits<float> {
  using ConstantType = ConstantFloat;
  static constexpr Type IceType = IceType_f32;
  // AsmTag and TypeName can't be constexpr because llvm::StringRef is unhappy
  // about them being constexpr.
  static const char AsmTag[];
  static const char TypeName[];
  static uint64_t bitcastToUint64(float Value) {
    static_assert(sizeof(Value) == sizeof(uint32_t),
                  "Float should be 4 bytes.");
    const uint32_t IntValue = Utils::bitCopy<uint32_t>(Value);
    return static_cast<uint64_t>(IntValue);
  }
};
const char ConstantPoolEmitterTraits<float>::AsmTag[] = ".word";
const char ConstantPoolEmitterTraits<float>::TypeName[] = "f32";

template <> struct ConstantPoolEmitterTraits<double> {
  using ConstantType = ConstantDouble;
  static constexpr Type IceType = IceType_f64;
  static const char AsmTag[];
  static const char TypeName[];
  static uint64_t bitcastToUint64(double Value) {
    static_assert(sizeof(double) == sizeof(uint64_t),
                  "Double should be 8 bytes.");
    return Utils::bitCopy<uint64_t>(Value);
  }
};
const char ConstantPoolEmitterTraits<double>::AsmTag[] = ".quad";
const char ConstantPoolEmitterTraits<double>::TypeName[] = "f64";

template <typename T>
void emitConstant(
    Ostream &Str,
    const typename ConstantPoolEmitterTraits<T>::ConstantType *Const) {
  if (!BuildDefs::dump())
    return;
  using Traits = ConstantPoolEmitterTraits<T>;
  Str << Const->getLabelName();
  T Value = Const->getValue();
  Str << ":\n\t" << Traits::AsmTag << "\t0x";
  Str.write_hex(Traits::bitcastToUint64(Value));
  Str << "\t/* " << Traits::TypeName << " " << Value << " */\n";
}

template <typename T> void emitConstantPool(GlobalContext *Ctx) {
  if (!BuildDefs::dump())
    return;
  using Traits = ConstantPoolEmitterTraits<T>;
  static constexpr size_t MinimumAlignment = 4;
  SizeT Align = std::max(MinimumAlignment, typeAlignInBytes(Traits::IceType));
  assert((Align % 4) == 0 && "Constants should be aligned");
  Ostream &Str = Ctx->getStrEmit();
  ConstantList Pool = Ctx->getConstantPool(Traits::IceType);
  Str << "\t.section\t.rodata.cst" << Align << ",\"aM\",%progbits," << Align
      << "\n"
      << "\t.align\t" << (Align == 4 ? 2 : 3) << "\n";
  for (Constant *C : Pool) {
    if (!C->getShouldBePooled()) {
      continue;
    }
    emitConstant<T>(Str, llvm::dyn_cast<typename Traits::ConstantType>(C));
  }
}
} // end of anonymous namespace

void TargetDataMIPS32::lowerConstants() {
  if (getFlags().getDisableTranslation())
    return;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();
    Writer->writeConstantPool<ConstantFloat>(IceType_f32);
    Writer->writeConstantPool<ConstantDouble>(IceType_f64);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    OstreamLocker _(Ctx);
    emitConstantPool<float>(Ctx);
    emitConstantPool<double>(Ctx);
    break;
  }
  }
}

void TargetDataMIPS32::lowerJumpTables() {
  if (getFlags().getDisableTranslation())
    return;
}

// Helper for legalize() to emit the right code to lower an operand to a
// register of the appropriate type.
Variable *TargetMIPS32::copyToReg(Operand *Src, RegNumT RegNum) {
  Type Ty = Src->getType();
  Variable *Reg = makeReg(Ty, RegNum);
  if (isVectorType(Ty)) {
    llvm::report_fatal_error("Invalid copy from vector type.");
  } else {
    if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(Src)) {
      _lw(Reg, Mem);
    } else {
      _mov(Reg, Src);
    }
  }
  return Reg;
}

Operand *TargetMIPS32::legalize(Operand *From, LegalMask Allowed,
                                RegNumT RegNum) {
  Type Ty = From->getType();
  // Assert that a physical register is allowed.  To date, all calls
  // to legalize() allow a physical register. Legal_Flex converts
  // registers to the right type OperandMIPS32FlexReg as needed.
  assert(Allowed & Legal_Reg);

  if (RegNum.hasNoValue()) {
    if (Variable *Subst = getContext().availabilityGet(From)) {
      // At this point we know there is a potential substitution available.
      if (!Subst->isRematerializable() && Subst->mustHaveReg() &&
          !Subst->hasReg()) {
        // At this point we know the substitution will have a register.
        if (From->getType() == Subst->getType()) {
          // At this point we know the substitution's register is compatible.
          return Subst;
        }
      }
    }
  }

  // Go through the various types of operands:
  // OperandMIPS32Mem, Constant, and Variable.
  // Given the above assertion, if type of operand is not legal
  // (e.g., OperandMIPS32Mem and !Legal_Mem), we can always copy
  // to a register.
  if (auto *Mem = llvm::dyn_cast<OperandMIPS32Mem>(From)) {
    // Base must be in a physical register.
    Variable *Base = Mem->getBase();
    ConstantInteger32 *Offset = llvm::cast<ConstantInteger32>(Mem->getOffset());
    Variable *RegBase = nullptr;
    assert(Base);

    RegBase = llvm::cast<Variable>(
        legalize(Base, Legal_Reg | Legal_Rematerializable));

    if (Offset != nullptr && Offset->getValue() != 0) {
      static constexpr bool ZeroExt = false;
      if (!OperandMIPS32Mem::canHoldOffset(Ty, ZeroExt, Offset->getValue())) {
        llvm::report_fatal_error("Invalid memory offset.");
      }
    }

    // Create a new operand if there was a change.
    if (Base != RegBase) {
      Mem = OperandMIPS32Mem::create(Func, Ty, RegBase, Offset,
                                     Mem->getAddrMode());
    }

    if (Allowed & Legal_Mem) {
      From = Mem;
    } else {
      Variable *Reg = makeReg(Ty, RegNum);
      _lw(Reg, Mem);
      From = Reg;
    }
    return From;
  }

  if (llvm::isa<Constant>(From)) {
    if (llvm::isa<ConstantUndef>(From)) {
      From = legalizeUndef(From, RegNum);
      if (isVectorType(Ty))
        return From;
    }
    if (auto *C = llvm::dyn_cast<ConstantRelocatable>(From)) {
      Variable *Reg = makeReg(Ty, RegNum);
      Variable *TReg = makeReg(Ty, RegNum);
      _lui(TReg, C, RO_Hi);
      _addiu(Reg, TReg, C, RO_Lo);
      return Reg;
    } else if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(From)) {
      const uint32_t Value = C32->getValue();
      // Use addiu if the immediate is a 16bit value. Otherwise load it
      // using a lui-ori instructions.
      Variable *Reg = makeReg(Ty, RegNum);
      if (isInt<16>(int32_t(Value))) {
        Variable *Zero = makeReg(Ty, RegMIPS32::Reg_ZERO);
        Context.insert<InstFakeDef>(Zero);
        _addiu(Reg, Zero, Value);
      } else {
        uint32_t UpperBits = (Value >> 16) & 0xFFFF;
        uint32_t LowerBits = Value & 0xFFFF;
        if (LowerBits) {
          Variable *TReg = makeReg(Ty, RegNum);
          _lui(TReg, Ctx->getConstantInt32(UpperBits));
          _ori(Reg, TReg, LowerBits);
        } else {
          _lui(Reg, Ctx->getConstantInt32(UpperBits));
        }
      }
      return Reg;
    } else if (isScalarFloatingType(Ty)) {
      auto *CFrom = llvm::cast<Constant>(From);
      Variable *TReg = makeReg(Ty);
      if (!CFrom->getShouldBePooled()) {
        // Float/Double constant 0 is not pooled.
        Context.insert<InstFakeDef>(TReg);
        _mov(TReg, getZero());
      } else {
        // Load floats/doubles from literal pool.
        Constant *Offset = Ctx->getConstantSym(0, CFrom->getLabelName());
        Variable *TReg1 = makeReg(getPointerType());
        _lui(TReg1, Offset, RO_Hi);
        OperandMIPS32Mem *Addr =
            OperandMIPS32Mem::create(Func, Ty, TReg1, Offset);
        if (Ty == IceType_f32)
          _lwc1(TReg, Addr, RO_Lo);
        else
          _ldc1(TReg, Addr, RO_Lo);
      }
      return copyToReg(TReg, RegNum);
    }
  }

  if (auto *Var = llvm::dyn_cast<Variable>(From)) {
    if (Var->isRematerializable()) {
      if (Allowed & Legal_Rematerializable) {
        return From;
      }

      Variable *T = makeReg(Var->getType(), RegNum);
      _mov(T, Var);
      return T;
    }
    // Check if the variable is guaranteed a physical register.  This
    // can happen either when the variable is pre-colored or when it is
    // assigned infinite weight.
    bool MustHaveRegister = (Var->hasReg() || Var->mustHaveReg());
    // We need a new physical register for the operand if:
    //   Mem is not allowed and Var isn't guaranteed a physical
    //   register, or
    //   RegNum is required and Var->getRegNum() doesn't match.
    if ((!(Allowed & Legal_Mem) && !MustHaveRegister) ||
        (RegNum.hasValue() && RegNum != Var->getRegNum())) {
      From = copyToReg(From, RegNum);
    }
    return From;
  }
  return From;
}

namespace BoolFolding {
// TODO(sagar.thakur): Add remaining instruction kinds to shouldTrackProducer()
// and isValidConsumer()
bool shouldTrackProducer(const Inst &Instr) {
  return Instr.getKind() == Inst::Icmp;
}

bool isValidConsumer(const Inst &Instr) { return Instr.getKind() == Inst::Br; }
} // end of namespace BoolFolding

void TargetMIPS32::ComputationTracker::recordProducers(CfgNode *Node) {
  for (Inst &Instr : Node->getInsts()) {
    if (Instr.isDeleted())
      continue;
    // Check whether Instr is a valid producer.
    Variable *Dest = Instr.getDest();
    if (Dest // only consider instructions with an actual dest var; and
        && Dest->getType() == IceType_i1 // only bool-type dest vars; and
        && BoolFolding::shouldTrackProducer(Instr)) { // white-listed instr.
      KnownComputations.emplace(Dest->getIndex(),
                                ComputationEntry(&Instr, IceType_i1));
    }
    // Check each src variable against the map.
    FOREACH_VAR_IN_INST(Var, Instr) {
      SizeT VarNum = Var->getIndex();
      auto ComputationIter = KnownComputations.find(VarNum);
      if (ComputationIter == KnownComputations.end()) {
        continue;
      }

      ++ComputationIter->second.NumUses;
      switch (ComputationIter->second.ComputationType) {
      default:
        KnownComputations.erase(VarNum);
        continue;
      case IceType_i1:
        if (!BoolFolding::isValidConsumer(Instr)) {
          KnownComputations.erase(VarNum);
          continue;
        }
        break;
      }

      if (Instr.isLastUse(Var)) {
        ComputationIter->second.IsLiveOut = false;
      }
    }
  }

  for (auto Iter = KnownComputations.begin(), End = KnownComputations.end();
       Iter != End;) {
    // Disable the folding if its dest may be live beyond this block.
    if (Iter->second.IsLiveOut || Iter->second.NumUses > 1) {
      Iter = KnownComputations.erase(Iter);
      continue;
    }

    // Mark as "dead" rather than outright deleting. This is so that other
    // peephole style optimizations during or before lowering have access to
    // this instruction in undeleted form. See for example
    // tryOptimizedCmpxchgCmpBr().
    Iter->second.Instr->setDead();
    ++Iter;
  }
}

TargetHeaderMIPS32::TargetHeaderMIPS32(GlobalContext *Ctx)
    : TargetHeaderLowering(Ctx) {}

void TargetHeaderMIPS32::lower() {
  if (!BuildDefs::dump())
    return;
  OstreamLocker L(Ctx);
  Ostream &Str = Ctx->getStrEmit();
  Str << "\t.set\t"
      << "nomicromips\n";
  Str << "\t.set\t"
      << "nomips16\n";
  Str << "\t.set\t"
      << "noat\n";
}

SmallBitVector TargetMIPS32::TypeToRegisterSet[RCMIPS32_NUM];
SmallBitVector TargetMIPS32::TypeToRegisterSetUnfiltered[RCMIPS32_NUM];
SmallBitVector TargetMIPS32::RegisterAliases[RegMIPS32::Reg_NUM];

} // end of namespace MIPS32
} // end of namespace Ice
