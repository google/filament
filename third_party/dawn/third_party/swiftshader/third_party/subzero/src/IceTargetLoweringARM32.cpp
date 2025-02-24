//===- subzero/src/IceTargetLoweringARM32.cpp - ARM32 lowering ------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TargetLoweringARM32 class, which consists almost
/// entirely of the lowering sequence for each high-level instruction.
///
//===----------------------------------------------------------------------===//
#include "IceTargetLoweringARM32.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceELFObjectWriter.h"
#include "IceGlobalInits.h"
#include "IceInstARM32.def"
#include "IceInstARM32.h"
#include "IceInstVarIter.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IcePhiLoweringImpl.h"
#include "IceRegistersARM32.h"
#include "IceTargetLoweringARM32.def"
#include "IceUtils.h"
#include "llvm/Support/MathExtras.h"

#include <algorithm>
#include <array>
#include <utility>

namespace ARM32 {
std::unique_ptr<::Ice::TargetLowering> createTargetLowering(::Ice::Cfg *Func) {
  return ::Ice::ARM32::TargetARM32::create(Func);
}

std::unique_ptr<::Ice::TargetDataLowering>
createTargetDataLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::ARM32::TargetDataARM32::create(Ctx);
}

std::unique_ptr<::Ice::TargetHeaderLowering>
createTargetHeaderLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::ARM32::TargetHeaderARM32::create(Ctx);
}

void staticInit(::Ice::GlobalContext *Ctx) {
  ::Ice::ARM32::TargetARM32::staticInit(Ctx);
}

bool shouldBePooled(const ::Ice::Constant *C) {
  return ::Ice::ARM32::TargetARM32::shouldBePooled(C);
}

::Ice::Type getPointerType() {
  return ::Ice::ARM32::TargetARM32::getPointerType();
}

} // end of namespace ARM32

namespace Ice {
namespace ARM32 {

namespace {

/// SizeOf is used to obtain the size of an initializer list as a constexpr
/// expression. This is only needed until our C++ library is updated to
/// C++ 14 -- which defines constexpr members to std::initializer_list.
class SizeOf {
  SizeOf(const SizeOf &) = delete;
  SizeOf &operator=(const SizeOf &) = delete;

public:
  constexpr SizeOf() : Size(0) {}
  template <typename... T>
  explicit constexpr SizeOf(T...) : Size(__length<T...>::value) {}
  constexpr SizeT size() const { return Size; }

private:
  template <typename T, typename... U> struct __length {
    static constexpr std::size_t value = 1 + __length<U...>::value;
  };

  template <typename T> struct __length<T> {
    static constexpr std::size_t value = 1;
  };

  const std::size_t Size;
};

} // end of anonymous namespace

// Defines the RegARM32::Table table with register information.
RegARM32::RegTableType RegARM32::RegTable[RegARM32::Reg_NUM] = {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  {name,      encode,                                                          \
   cc_arg,    scratch,                                                         \
   preserved, stackptr,                                                        \
   frameptr,  isGPR,                                                           \
   isInt,     isI64Pair,                                                       \
   isFP32,    isFP64,                                                          \
   isVec128,  (SizeOf alias_init).size(),                                      \
   alias_init},
    REGARM32_TABLE
#undef X
};

namespace {

// The following table summarizes the logic for lowering the icmp instruction
// for i32 and narrower types. Each icmp condition has a clear mapping to an
// ARM32 conditional move instruction.

const struct TableIcmp32_ {
  CondARM32::Cond Mapping;
} TableIcmp32[] = {
#define X(val, is_signed, swapped64, C_32, C1_64, C2_64, C_V, INV_V, NEG_V)    \
  {CondARM32::C_32},
    ICMPARM32_TABLE
#undef X
};

// The following table summarizes the logic for lowering the icmp instruction
// for the i64 type. Two conditional moves are needed for setting to 1 or 0.
// The operands may need to be swapped, and there is a slight difference for
// signed vs unsigned (comparing hi vs lo first, and using cmp vs sbc).
const struct TableIcmp64_ {
  bool IsSigned;
  bool Swapped;
  CondARM32::Cond C1, C2;
} TableIcmp64[] = {
#define X(val, is_signed, swapped64, C_32, C1_64, C2_64, C_V, INV_V, NEG_V)    \
  {is_signed, swapped64, CondARM32::C1_64, CondARM32::C2_64},
    ICMPARM32_TABLE
#undef X
};

CondARM32::Cond getIcmp32Mapping(InstIcmp::ICond Cond) {
  assert(Cond < llvm::array_lengthof(TableIcmp32));
  return TableIcmp32[Cond].Mapping;
}

// In some cases, there are x-macros tables for both high-level and low-level
// instructions/operands that use the same enum key value. The tables are kept
// separate to maintain a proper separation between abstraction layers. There
// is a risk that the tables could get out of sync if enum values are reordered
// or if entries are added or deleted. The following anonymous namespaces use
// static_asserts to ensure everything is kept in sync.

// Validate the enum values in ICMPARM32_TABLE.
namespace {
// Define a temporary set of enum values based on low-level table entries.
enum _icmp_ll_enum {
#define X(val, is_signed, swapped64, C_32, C1_64, C2_64, C_V, INV_V, NEG_V)    \
  _icmp_ll_##val,
  ICMPARM32_TABLE
#undef X
      _num
};
// Define a set of constants based on high-level table entries.
#define X(tag, reverse, str)                                                   \
  static constexpr int _icmp_hl_##tag = InstIcmp::tag;
ICEINSTICMP_TABLE
#undef X
// Define a set of constants based on low-level table entries, and ensure the
// table entry keys are consistent.
#define X(val, is_signed, swapped64, C_32, C1_64, C2_64, C_V, INV_V, NEG_V)    \
  static_assert(                                                               \
      _icmp_ll_##val == _icmp_hl_##val,                                        \
      "Inconsistency between ICMPARM32_TABLE and ICEINSTICMP_TABLE: " #val);
ICMPARM32_TABLE
#undef X
// Repeat the static asserts with respect to the high-level table entries in
// case the high-level table has extra entries.
#define X(tag, reverse, str)                                                   \
  static_assert(                                                               \
      _icmp_hl_##tag == _icmp_ll_##tag,                                        \
      "Inconsistency between ICMPARM32_TABLE and ICEINSTICMP_TABLE: " #tag);
ICEINSTICMP_TABLE
#undef X
} // end of anonymous namespace

// Stack alignment
const uint32_t ARM32_STACK_ALIGNMENT_BYTES = 16;

// Value is in bytes. Return Value adjusted to the next highest multiple of the
// stack alignment.
uint32_t applyStackAlignment(uint32_t Value) {
  return Utils::applyAlignment(Value, ARM32_STACK_ALIGNMENT_BYTES);
}

// Value is in bytes. Return Value adjusted to the next highest multiple of the
// stack alignment required for the given type.
uint32_t applyStackAlignmentTy(uint32_t Value, Type Ty) {
  // Use natural alignment, except that normally (non-NaCl) ARM only aligns
  // vectors to 8 bytes.
  // TODO(jvoung): Check this ...
  size_t typeAlignInBytes = typeWidthInBytes(Ty);
  if (isVectorType(Ty))
    typeAlignInBytes = 8;
  return Utils::applyAlignment(Value, typeAlignInBytes);
}

// Conservatively check if at compile time we know that the operand is
// definitely a non-zero integer.
bool isGuaranteedNonzeroInt(const Operand *Op) {
  if (auto *Const = llvm::dyn_cast_or_null<ConstantInteger32>(Op)) {
    return Const->getValue() != 0;
  }
  return false;
}

} // end of anonymous namespace

TargetARM32Features::TargetARM32Features(const ClFlags &Flags) {
  static_assert(
      (ARM32InstructionSet::End - ARM32InstructionSet::Begin) ==
          (TargetInstructionSet::ARM32InstructionSet_End -
           TargetInstructionSet::ARM32InstructionSet_Begin),
      "ARM32InstructionSet range different from TargetInstructionSet");
  if (Flags.getTargetInstructionSet() !=
      TargetInstructionSet::BaseInstructionSet) {
    InstructionSet = static_cast<ARM32InstructionSet>(
        (Flags.getTargetInstructionSet() -
         TargetInstructionSet::ARM32InstructionSet_Begin) +
        ARM32InstructionSet::Begin);
  }
}

namespace {
constexpr SizeT NumGPRArgs =
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(((cc_arg) > 0) ? 1 : 0)
    REGARM32_GPR_TABLE
#undef X
    ;
std::array<RegNumT, NumGPRArgs> GPRArgInitializer;

constexpr SizeT NumI64Args =
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(((cc_arg) > 0) ? 1 : 0)
    REGARM32_I64PAIR_TABLE
#undef X
    ;
std::array<RegNumT, NumI64Args> I64ArgInitializer;

constexpr SizeT NumFP32Args =
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(((cc_arg) > 0) ? 1 : 0)
    REGARM32_FP32_TABLE
#undef X
    ;
std::array<RegNumT, NumFP32Args> FP32ArgInitializer;

constexpr SizeT NumFP64Args =
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(((cc_arg) > 0) ? 1 : 0)
    REGARM32_FP64_TABLE
#undef X
    ;
std::array<RegNumT, NumFP64Args> FP64ArgInitializer;

constexpr SizeT NumVec128Args =
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  +(((cc_arg > 0)) ? 1 : 0)
    REGARM32_VEC128_TABLE
#undef X
    ;
std::array<RegNumT, NumVec128Args> Vec128ArgInitializer;

const char *getRegClassName(RegClass C) {
  auto ClassNum = static_cast<RegARM32::RegClassARM32>(C);
  assert(ClassNum < RegARM32::RCARM32_NUM);
  switch (ClassNum) {
  default:
    assert(C < RC_Target);
    return regClassString(C);
  // Add handling of new register classes below.
  case RegARM32::RCARM32_QtoS:
    return "QtoS";
  }
}

} // end of anonymous namespace

TargetARM32::TargetARM32(Cfg *Func)
    : TargetLowering(Func), CPUFeatures(getFlags()) {}

void TargetARM32::staticInit(GlobalContext *Ctx) {
  RegNumT::setLimit(RegARM32::Reg_NUM);
  // Limit this size (or do all bitsets need to be the same width)???
  SmallBitVector IntegerRegisters(RegARM32::Reg_NUM);
  SmallBitVector I64PairRegisters(RegARM32::Reg_NUM);
  SmallBitVector Float32Registers(RegARM32::Reg_NUM);
  SmallBitVector Float64Registers(RegARM32::Reg_NUM);
  SmallBitVector VectorRegisters(RegARM32::Reg_NUM);
  SmallBitVector QtoSRegisters(RegARM32::Reg_NUM);
  SmallBitVector InvalidRegisters(RegARM32::Reg_NUM);
  const unsigned EncodedReg_q8 = RegARM32::RegTable[RegARM32::Reg_q8].Encoding;
  for (int i = 0; i < RegARM32::Reg_NUM; ++i) {
    const auto &Entry = RegARM32::RegTable[i];
    IntegerRegisters[i] = Entry.IsInt;
    I64PairRegisters[i] = Entry.IsI64Pair;
    Float32Registers[i] = Entry.IsFP32;
    Float64Registers[i] = Entry.IsFP64;
    VectorRegisters[i] = Entry.IsVec128;
    RegisterAliases[i].resize(RegARM32::Reg_NUM);
    // TODO(eholk): It would be better to store a QtoS flag in the
    // IceRegistersARM32 table than to compare their encodings here.
    QtoSRegisters[i] = Entry.IsVec128 && Entry.Encoding < EncodedReg_q8;
    for (int j = 0; j < Entry.NumAliases; ++j) {
      assert(i == j || !RegisterAliases[i][Entry.Aliases[j]]);
      RegisterAliases[i].set(Entry.Aliases[j]);
    }
    assert(RegisterAliases[i][i]);
    if (Entry.CCArg <= 0) {
      continue;
    }
    const auto RegNum = RegNumT::fromInt(i);
    if (Entry.IsGPR) {
      GPRArgInitializer[Entry.CCArg - 1] = RegNum;
    } else if (Entry.IsI64Pair) {
      I64ArgInitializer[Entry.CCArg - 1] = RegNum;
    } else if (Entry.IsFP32) {
      FP32ArgInitializer[Entry.CCArg - 1] = RegNum;
    } else if (Entry.IsFP64) {
      FP64ArgInitializer[Entry.CCArg - 1] = RegNum;
    } else if (Entry.IsVec128) {
      Vec128ArgInitializer[Entry.CCArg - 1] = RegNum;
    }
  }
  TypeToRegisterSet[IceType_void] = InvalidRegisters;
  TypeToRegisterSet[IceType_i1] = IntegerRegisters;
  TypeToRegisterSet[IceType_i8] = IntegerRegisters;
  TypeToRegisterSet[IceType_i16] = IntegerRegisters;
  TypeToRegisterSet[IceType_i32] = IntegerRegisters;
  TypeToRegisterSet[IceType_i64] = I64PairRegisters;
  TypeToRegisterSet[IceType_f32] = Float32Registers;
  TypeToRegisterSet[IceType_f64] = Float64Registers;
  TypeToRegisterSet[IceType_v4i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v8i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v16i1] = VectorRegisters;
  TypeToRegisterSet[IceType_v16i8] = VectorRegisters;
  TypeToRegisterSet[IceType_v8i16] = VectorRegisters;
  TypeToRegisterSet[IceType_v4i32] = VectorRegisters;
  TypeToRegisterSet[IceType_v4f32] = VectorRegisters;
  TypeToRegisterSet[RegARM32::RCARM32_QtoS] = QtoSRegisters;

  for (size_t i = 0; i < llvm::array_lengthof(TypeToRegisterSet); ++i)
    TypeToRegisterSetUnfiltered[i] = TypeToRegisterSet[i];

  filterTypeToRegisterSet(
      Ctx, RegARM32::Reg_NUM, TypeToRegisterSet,
      llvm::array_lengthof(TypeToRegisterSet),
      [](RegNumT RegNum) -> std::string {
        // This function simply removes ", " from the
        // register name.
        std::string Name = RegARM32::getRegName(RegNum);
        constexpr const char RegSeparator[] = ", ";
        constexpr size_t RegSeparatorWidth =
            llvm::array_lengthof(RegSeparator) - 1;
        for (size_t Pos = Name.find(RegSeparator); Pos != std::string::npos;
             Pos = Name.find(RegSeparator)) {
          Name.replace(Pos, RegSeparatorWidth, "");
        }
        return Name;
      },
      getRegClassName);
}

namespace {
void copyRegAllocFromInfWeightVariable64On32(const VarList &Vars) {
  for (Variable *Var : Vars) {
    auto *Var64 = llvm::dyn_cast<Variable64On32>(Var);
    if (!Var64) {
      // This is not the variable we are looking for.
      continue;
    }
    // only allow infinite-weight i64 temporaries to be register allocated.
    assert(!Var64->hasReg() || Var64->mustHaveReg());
    if (!Var64->hasReg()) {
      continue;
    }
    const auto FirstReg =
        RegNumT::fixme(RegARM32::getI64PairFirstGPRNum(Var->getRegNum()));
    // This assumes little endian.
    Variable *Lo = Var64->getLo();
    Variable *Hi = Var64->getHi();
    assert(Lo->hasReg() == Hi->hasReg());
    if (Lo->hasReg()) {
      continue;
    }
    Lo->setRegNum(FirstReg);
    Lo->setMustHaveReg();
    Hi->setRegNum(RegNumT::fixme(FirstReg + 1));
    Hi->setMustHaveReg();
  }
}
} // end of anonymous namespace

uint32_t TargetARM32::getCallStackArgumentsSizeBytes(const InstCall *Call) {
  TargetARM32::CallingConv CC;
  RegNumT DummyReg;
  size_t OutArgsSizeBytes = 0;
  for (SizeT i = 0, NumArgs = Call->getNumArgs(); i < NumArgs; ++i) {
    Operand *Arg = legalizeUndef(Call->getArg(i));
    const Type Ty = Arg->getType();
    if (isScalarIntegerType(Ty)) {
      if (CC.argInGPR(Ty, &DummyReg)) {
        continue;
      }
    } else {
      if (CC.argInVFP(Ty, &DummyReg)) {
        continue;
      }
    }

    OutArgsSizeBytes = applyStackAlignmentTy(OutArgsSizeBytes, Ty);
    OutArgsSizeBytes += typeWidthInBytesOnStack(Ty);
  }

  return applyStackAlignment(OutArgsSizeBytes);
}

void TargetARM32::genTargetHelperCallFor(Inst *Instr) {
  constexpr bool NoTailCall = false;
  constexpr bool IsTargetHelperCall = true;

  switch (Instr->getKind()) {
  default:
    return;
  case Inst::Arithmetic: {
    Variable *Dest = Instr->getDest();
    const Type DestTy = Dest->getType();
    const InstArithmetic::OpKind Op =
        llvm::cast<InstArithmetic>(Instr)->getOp();
    if (isVectorType(DestTy)) {
      switch (Op) {
      default:
        break;
      case InstArithmetic::Fdiv:
      case InstArithmetic::Frem:
      case InstArithmetic::Sdiv:
      case InstArithmetic::Srem:
      case InstArithmetic::Udiv:
      case InstArithmetic::Urem:
        scalarizeArithmetic(Op, Dest, Instr->getSrc(0), Instr->getSrc(1));
        Instr->setDeleted();
        return;
      }
    }
    switch (DestTy) {
    default:
      return;
    case IceType_i64: {
      // Technically, ARM has its own aeabi routines, but we can use the
      // non-aeabi routine as well. LLVM uses __aeabi_ldivmod for div, but uses
      // the more standard __moddi3 for rem.
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
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(HelperID);
      ARM32HelpersPreamble[TargetHelper] = &TargetARM32::preambleDivRem;
      constexpr SizeT MaxArgs = 2;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Instr->getSrc(0));
      Call->addArg(Instr->getSrc(1));
      Instr->setDeleted();
      return;
    }
    case IceType_i32:
    case IceType_i16:
    case IceType_i8: {
      const bool HasHWDiv = hasCPUFeature(TargetARM32Features::HWDivArm);
      InstCast::OpKind CastKind;
      RuntimeHelper HelperID = RuntimeHelper::H_Num;
      switch (Op) {
      default:
        return;
      case InstArithmetic::Udiv:
        HelperID = HasHWDiv ? RuntimeHelper::H_Num : RuntimeHelper::H_udiv_i32;
        CastKind = InstCast::Zext;
        break;
      case InstArithmetic::Sdiv:
        HelperID = HasHWDiv ? RuntimeHelper::H_Num : RuntimeHelper::H_sdiv_i32;
        CastKind = InstCast::Sext;
        break;
      case InstArithmetic::Urem:
        HelperID = HasHWDiv ? RuntimeHelper::H_Num : RuntimeHelper::H_urem_i32;
        CastKind = InstCast::Zext;
        break;
      case InstArithmetic::Srem:
        HelperID = HasHWDiv ? RuntimeHelper::H_Num : RuntimeHelper::H_srem_i32;
        CastKind = InstCast::Sext;
        break;
      }
      if (HelperID == RuntimeHelper::H_Num) {
        // HelperID should only ever be undefined when the processor does not
        // have a hardware divider. If any other helpers are ever introduced,
        // the following assert will have to be modified.
        assert(HasHWDiv);
        return;
      }
      Operand *Src0 = Instr->getSrc(0);
      Operand *Src1 = Instr->getSrc(1);
      if (DestTy != IceType_i32) {
        // Src0 and Src1 have to be zero-, or signed-extended to i32. For Src0,
        // we just insert a InstCast right before the call to the helper.
        Variable *Src0_32 = Func->makeVariable(IceType_i32);
        Context.insert<InstCast>(CastKind, Src0_32, Src0);
        Src0 = Src0_32;

        // For extending Src1, we will just insert an InstCast if Src1 is not a
        // Constant. If it is, then we extend it here, and not during program
        // runtime. This allows preambleDivRem to optimize-out the div-by-0
        // check.
        if (auto *C = llvm::dyn_cast<ConstantInteger32>(Src1)) {
          const int32_t ShAmt = (DestTy == IceType_i16) ? 16 : 24;
          int32_t NewC = C->getValue();
          if (CastKind == InstCast::Zext) {
            NewC &= ~(0x80000000l >> ShAmt);
          } else {
            NewC = (NewC << ShAmt) >> ShAmt;
          }
          Src1 = Ctx->getConstantInt32(NewC);
        } else {
          Variable *Src1_32 = Func->makeVariable(IceType_i32);
          Context.insert<InstCast>(CastKind, Src1_32, Src1);
          Src1 = Src1_32;
        }
      }
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(HelperID);
      ARM32HelpersPreamble[TargetHelper] = &TargetARM32::preambleDivRem;
      constexpr SizeT MaxArgs = 2;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      assert(Src0->getType() == IceType_i32);
      Call->addArg(Src0);
      assert(Src1->getType() == IceType_i32);
      Call->addArg(Src1);
      Instr->setDeleted();
      return;
    }
    case IceType_f64:
    case IceType_f32: {
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
    Variable *Dest = Instr->getDest();
    Operand *Src0 = Instr->getSrc(0);
    const Type DestTy = Dest->getType();
    const Type SrcTy = Src0->getType();
    auto *CastInstr = llvm::cast<InstCast>(Instr);
    const InstCast::OpKind CastKind = CastInstr->getCastKind();

    switch (CastKind) {
    default:
      return;
    case InstCast::Fptosi:
    case InstCast::Fptoui: {
      if (DestTy != IceType_i64) {
        return;
      }
      const bool DestIsSigned = CastKind == InstCast::Fptosi;
      const bool Src0IsF32 = isFloat32Asserting32Or64(SrcTy);
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(
          Src0IsF32 ? (DestIsSigned ? RuntimeHelper::H_fptosi_f32_i64
                                    : RuntimeHelper::H_fptoui_f32_i64)
                    : (DestIsSigned ? RuntimeHelper::H_fptosi_f64_i64
                                    : RuntimeHelper::H_fptoui_f64_i64));
      static constexpr SizeT MaxArgs = 1;
      auto *Call = Context.insert<InstCall>(MaxArgs, Dest, TargetHelper,
                                            NoTailCall, IsTargetHelperCall);
      Call->addArg(Src0);
      Instr->setDeleted();
      return;
    }
    case InstCast::Sitofp:
    case InstCast::Uitofp: {
      if (SrcTy != IceType_i64) {
        return;
      }
      const bool SourceIsSigned = CastKind == InstCast::Sitofp;
      const bool DestIsF32 = isFloat32Asserting32Or64(Dest->getType());
      Operand *TargetHelper = Ctx->getRuntimeHelperFunc(
          DestIsF32 ? (SourceIsSigned ? RuntimeHelper::H_sitofp_i64_f32
                                      : RuntimeHelper::H_uitofp_i64_f32)
                    : (SourceIsSigned ? RuntimeHelper::H_sitofp_i64_f64
                                      : RuntimeHelper::H_uitofp_i64_f64));
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
      if (CallDest->getType() != Dest->getType())
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
    Variable *Dest = Instr->getDest();
    auto *Intrinsic = llvm::cast<InstIntrinsic>(Instr);
    Intrinsics::IntrinsicID ID = Intrinsic->getIntrinsicID();
    switch (ID) {
    default:
      return;
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
      if (Src0->getType() == IceType_i64) {
        ARM32HelpersPostamble[TargetHelper] = &TargetARM32::postambleCtpop64;
      }
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
      // In the future, we could potentially emit an inline memcpy/memset, etc.
      // for intrinsic calls w/ a known length.
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
      // The value operand needs to be extended to a stack slot size because the
      // PNaCl ABI requires arguments to be at least 32 bits wide.
      Operand *ValOp = Intrinsic->getArg(1);
      assert(ValOp->getType() == IceType_i8);
      Variable *ValExt = Func->makeVariable(stackSlotType());
      Context.insert<InstCast>(InstCast::Zext, ValExt, ValOp);

      // Technically, ARM has its own __aeabi_memset, but we can use plain
      // memset too. The value and size argument need to be flipped if we ever
      // decide to use __aeabi_memset.
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

void TargetARM32::findMaxStackOutArgsSize() {
  // MinNeededOutArgsBytes should be updated if the Target ever creates a
  // high-level InstCall that requires more stack bytes.
  constexpr size_t MinNeededOutArgsBytes = 0;
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
}

GlobalString
TargetARM32::createGotoffRelocation(const ConstantRelocatable *CR) {
  GlobalString CRName = CR->getName();
  GlobalString CRGotoffName =
      Ctx->getGlobalString("GOTOFF$" + Func->getFunctionName() + "$" + CRName);
  if (KnownGotoffs.count(CRGotoffName) == 0) {
    constexpr bool SuppressMangling = true;
    auto *Global =
        VariableDeclaration::create(Func->getGlobalPool(), SuppressMangling);
    Global->setIsConstant(true);
    Global->setName(CRName);
    Func->getGlobalPool()->willNotBeEmitted(Global);

    auto *Gotoff =
        VariableDeclaration::create(Func->getGlobalPool(), SuppressMangling);
    constexpr auto GotFixup = R_ARM_GOTOFF32;
    Gotoff->setIsConstant(true);
    Gotoff->addInitializer(VariableDeclaration::RelocInitializer::create(
        Func->getGlobalPool(), Global, {RelocOffset::create(Ctx, 0)},
        GotFixup));
    Gotoff->setName(CRGotoffName);
    Func->addGlobal(Gotoff);
    KnownGotoffs.emplace(CRGotoffName);
  }
  return CRGotoffName;
}

void TargetARM32::translateO2() {
  TimerMarker T(TimerStack::TT_O2, Func);

  genTargetHelperCalls();
  findMaxStackOutArgsSize();

  // Do not merge Alloca instructions, and lay out the stack.
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
  Func->materializeVectorShuffles();

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
  Func->dump("After ARM32 address mode opt");

  Func->genCode();
  if (Func->hasError())
    return;
  Func->dump("After ARM32 codegen");

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
  Func->dump("After initial ARM32 codegen");
  // Validate the live range computations. The expensive validation call is
  // deliberately only made when assertions are enabled.
  assert(Func->validateLiveness());
  Func->getVMetadata()->init(VMK_All);
  regAlloc(RAK_Global);
  if (Func->hasError())
    return;

  copyRegAllocFromInfWeightVariable64On32(Func->getVariables());
  Func->dump("After linear scan regalloc");

  if (getFlags().getEnablePhiEdgeSplit()) {
    Func->advancedPhiLowering();
    Func->dump("After advanced Phi lowering");
  }

  ForbidTemporaryWithoutReg _(this);

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

void TargetARM32::translateOm1() {
  TimerMarker T(TimerStack::TT_Om1, Func);

  genTargetHelperCalls();
  findMaxStackOutArgsSize();

  // Do not merge Alloca instructions, and lay out the stack.
  static constexpr bool DontSortAndCombineAllocas = false;
  Func->processAllocas(DontSortAndCombineAllocas);
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
  Func->dump("After initial ARM32 codegen");

  regAlloc(RAK_InfOnly);
  if (Func->hasError())
    return;

  copyRegAllocFromInfWeightVariable64On32(Func->getVariables());
  Func->dump("After regalloc of infinite-weight variables");

  ForbidTemporaryWithoutReg _(this);

  Func->genFrame();
  if (Func->hasError())
    return;
  Func->dump("After stack frame mapping");

  postLowerLegalization();
  if (Func->hasError())
    return;
  Func->dump("After postLowerLegalization");
}

uint32_t TargetARM32::getStackAlignment() const {
  return ARM32_STACK_ALIGNMENT_BYTES;
}

bool TargetARM32::doBranchOpt(Inst *I, const CfgNode *NextNode) {
  if (auto *Br = llvm::dyn_cast<InstARM32Br>(I)) {
    return Br->optimizeBranch(NextNode);
  }
  return false;
}

const char *TargetARM32::getRegName(RegNumT RegNum, Type Ty) const {
  (void)Ty;
  return RegARM32::getRegName(RegNum);
}

Variable *TargetARM32::getPhysicalRegister(RegNumT RegNum, Type Ty) {
  static const Type DefaultType[] = {
#define X(val, encode, name, cc_arg, scratch, preserved, stackptr, frameptr,   \
          isGPR, isInt, isI64Pair, isFP32, isFP64, isVec128, alias_init)       \
  (isFP32)                                                                     \
      ? IceType_f32                                                            \
      : ((isFP64) ? IceType_f64 : ((isVec128 ? IceType_v4i32 : IceType_i32))),
      REGARM32_TABLE
#undef X
  };

  if (Ty == IceType_void) {
    assert(unsigned(RegNum) < llvm::array_lengthof(DefaultType));
    Ty = DefaultType[RegNum];
  }
  if (PhysicalRegisters[Ty].empty())
    PhysicalRegisters[Ty].resize(RegARM32::Reg_NUM);
  assert(unsigned(RegNum) < PhysicalRegisters[Ty].size());
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

void TargetARM32::emitJumpTable(const Cfg *Func,
                                const InstJumpTable *JumpTable) const {
  (void)Func;
  (void)JumpTable;
  UnimplementedError(getFlags());
}

void TargetARM32::emitVariable(const Variable *Var) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  if (Var->hasReg()) {
    Str << getRegName(Var->getRegNum(), Var->getType());
    return;
  }
  if (Var->mustHaveReg()) {
    llvm::report_fatal_error("Infinite-weight Variable (" + Var->getName() +
                             ") has no register assigned - function " +
                             Func->getFunctionName());
  }
  assert(!Var->isRematerializable());
  int32_t Offset = Var->getStackOffset();
  auto BaseRegNum = Var->getBaseRegNum();
  if (BaseRegNum.hasNoValue()) {
    BaseRegNum = getFrameOrStackReg();
  }
  const Type VarTy = Var->getType();
  Str << "[" << getRegName(BaseRegNum, VarTy);
  if (Offset != 0) {
    Str << ", #" << Offset;
  }
  Str << "]";
}

TargetARM32::CallingConv::CallingConv()
    : GPRegsUsed(RegARM32::Reg_NUM),
      GPRArgs(GPRArgInitializer.rbegin(), GPRArgInitializer.rend()),
      I64Args(I64ArgInitializer.rbegin(), I64ArgInitializer.rend()),
      VFPRegsUsed(RegARM32::Reg_NUM),
      FP32Args(FP32ArgInitializer.rbegin(), FP32ArgInitializer.rend()),
      FP64Args(FP64ArgInitializer.rbegin(), FP64ArgInitializer.rend()),
      Vec128Args(Vec128ArgInitializer.rbegin(), Vec128ArgInitializer.rend()) {}

bool TargetARM32::CallingConv::argInGPR(Type Ty, RegNumT *Reg) {
  CfgVector<RegNumT> *Source;

  switch (Ty) {
  default: {
    assert(isScalarIntegerType(Ty));
    Source = &GPRArgs;
  } break;
  case IceType_i64: {
    Source = &I64Args;
  } break;
  }

  discardUnavailableGPRsAndTheirAliases(Source);

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
  return true;
}

// GPR are not packed when passing parameters. Thus, a function foo(i32, i64,
// i32) will have the first argument in r0, the second in r1-r2, and the third
// on the stack. To model this behavior, whenever we pop a register from Regs,
// we remove all of its aliases from the pool of available GPRs. This has the
// effect of computing the "closure" on the GPR registers.
void TargetARM32::CallingConv::discardUnavailableGPRsAndTheirAliases(
    CfgVector<RegNumT> *Regs) {
  while (!Regs->empty() && GPRegsUsed[Regs->back()]) {
    GPRegsUsed |= RegisterAliases[Regs->back()];
    Regs->pop_back();
  }
}

bool TargetARM32::CallingConv::argInVFP(Type Ty, RegNumT *Reg) {
  CfgVector<RegNumT> *Source;

  switch (Ty) {
  default: {
    assert(isVectorType(Ty));
    Source = &Vec128Args;
  } break;
  case IceType_f32: {
    Source = &FP32Args;
  } break;
  case IceType_f64: {
    Source = &FP64Args;
  } break;
  }

  discardUnavailableVFPRegs(Source);

  if (Source->empty()) {
    VFPRegsUsed.set();
    return false;
  }

  *Reg = Source->back();
  VFPRegsUsed |= RegisterAliases[*Reg];
  return true;
}

// Arguments in VFP registers are not packed, so we don't mark the popped
// registers' aliases as unavailable.
void TargetARM32::CallingConv::discardUnavailableVFPRegs(
    CfgVector<RegNumT> *Regs) {
  while (!Regs->empty() && VFPRegsUsed[Regs->back()]) {
    Regs->pop_back();
  }
}

void TargetARM32::lowerArguments() {
  VarList &Args = Func->getArgs();
  TargetARM32::CallingConv CC;

  // For each register argument, replace Arg in the argument list with the home
  // register. Then generate an instruction in the prolog to copy the home
  // register to the assigned location of Arg.
  Context.init(Func->getEntryNode());
  Context.setInsertPoint(Context.getCur());

  for (SizeT I = 0, E = Args.size(); I < E; ++I) {
    Variable *Arg = Args[I];
    Type Ty = Arg->getType();
    RegNumT RegNum;
    if (isScalarIntegerType(Ty)) {
      if (!CC.argInGPR(Ty, &RegNum)) {
        continue;
      }
    } else {
      if (!CC.argInVFP(Ty, &RegNum)) {
        continue;
      }
    }

    Variable *RegisterArg = Func->makeVariable(Ty);
    if (BuildDefs::dump()) {
      RegisterArg->setName(Func, "home_reg:" + Arg->getName());
    }
    RegisterArg->setIsArg();
    Arg->setIsArg(false);
    Args[I] = RegisterArg;
    switch (Ty) {
    default: {
      RegisterArg->setRegNum(RegNum);
    } break;
    case IceType_i64: {
      auto *RegisterArg64 = llvm::cast<Variable64On32>(RegisterArg);
      RegisterArg64->initHiLo(Func);
      RegisterArg64->getLo()->setRegNum(
          RegNumT::fixme(RegARM32::getI64PairFirstGPRNum(RegNum)));
      RegisterArg64->getHi()->setRegNum(
          RegNumT::fixme(RegARM32::getI64PairSecondGPRNum(RegNum)));
    } break;
    }
    Context.insert<InstAssign>(Arg, RegisterArg);
  }
}

// Helper function for addProlog().
//
// This assumes Arg is an argument passed on the stack. This sets the frame
// offset for Arg and updates InArgsSizeBytes according to Arg's width. For an
// I64 arg that has been split into Lo and Hi components, it calls itself
// recursively on the components, taking care to handle Lo first because of the
// little-endian architecture. Lastly, this function generates an instruction
// to copy Arg into its assigned register if applicable.
void TargetARM32::finishArgumentLowering(Variable *Arg, Variable *FramePtr,
                                         size_t BasicFrameOffset,
                                         size_t *InArgsSizeBytes) {
  const Type Ty = Arg->getType();
  *InArgsSizeBytes = applyStackAlignmentTy(*InArgsSizeBytes, Ty);

  if (auto *Arg64On32 = llvm::dyn_cast<Variable64On32>(Arg)) {
    Variable *const Lo = Arg64On32->getLo();
    Variable *const Hi = Arg64On32->getHi();
    finishArgumentLowering(Lo, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    finishArgumentLowering(Hi, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    return;
  }
  assert(Ty != IceType_i64);

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

Type TargetARM32::stackSlotType() { return IceType_i32; }

void TargetARM32::addProlog(CfgNode *Node) {
  // Stack frame layout:
  //
  // +------------------------+
  // | 1. preserved registers |
  // +------------------------+
  // | 2. padding             |
  // +------------------------+ <--- FramePointer (if used)
  // | 3. global spill area   |
  // +------------------------+
  // | 4. padding             |
  // +------------------------+
  // | 5. local spill area    |
  // +------------------------+
  // | 6. padding             |
  // +------------------------+
  // | 7. allocas (variable)  |
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
  //  * SpillAreaSizeBytes:     areas 2 - 6, and 9
  //  * MaxOutArgsSizeBytes:    area 9
  //
  // Determine stack frame offsets for each Variable without a register
  // assignment.  This can be done as one variable per stack slot.  Or, do
  // coalescing by running the register allocator again with an infinite set of
  // registers (as a side effect, this gives variables a second chance at
  // physical register assignment).
  //
  // A middle ground approach is to leverage sparsity and allocate one block of
  // space on the frame for globals (variables with multi-block lifetime), and
  // one block to share for locals (single-block lifetime).

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

  // Add push instructions for preserved registers. On ARM, "push" can push a
  // whole list of GPRs via a bitmask (0-15). Unlike x86, ARM also has
  // callee-saved float/vector registers.
  //
  // The "vpush" instruction can handle a whole list of float/vector registers,
  // but it only handles contiguous sequences of registers by specifying the
  // start and the length.
  PreservedGPRs.reserve(CalleeSaves.size());
  PreservedSRegs.reserve(CalleeSaves.size());

  // Consider FP and LR as callee-save / used as needed.
  if (UsesFramePointer) {
    if (RegsUsed[RegARM32::Reg_fp]) {
      llvm::report_fatal_error("Frame pointer has been used.");
    }
    CalleeSaves[RegARM32::Reg_fp] = true;
    RegsUsed[RegARM32::Reg_fp] = true;
  }
  if (!MaybeLeafFunc) {
    CalleeSaves[RegARM32::Reg_lr] = true;
    RegsUsed[RegARM32::Reg_lr] = true;
  }

  // Make two passes over the used registers. The first pass records all the
  // used registers -- and their aliases. Then, we figure out which GPRs and
  // VFP S registers should be saved. We don't bother saving D/Q registers
  // because their uses are recorded as S regs uses.
  SmallBitVector ToPreserve(RegARM32::Reg_NUM);
  for (SizeT i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      ToPreserve |= RegisterAliases[i];
    }
  }

  uint32_t NumCallee = 0;
  size_t PreservedRegsSizeBytes = 0;

  // RegClasses is a tuple of
  //
  // <First Register in Class, Last Register in Class, Vector of Save Registers>
  //
  // We use this tuple to figure out which register we should push/pop during
  // prolog/epilog.
  using RegClassType = std::tuple<uint32_t, uint32_t, VarList *>;
  const RegClassType RegClasses[] = {
      RegClassType(RegARM32::Reg_GPR_First, RegARM32::Reg_GPR_Last,
                   &PreservedGPRs),
      RegClassType(RegARM32::Reg_SREG_First, RegARM32::Reg_SREG_Last,
                   &PreservedSRegs)};
  for (const auto &RegClass : RegClasses) {
    const uint32_t FirstRegInClass = std::get<0>(RegClass);
    const uint32_t LastRegInClass = std::get<1>(RegClass);
    VarList *const PreservedRegsInClass = std::get<2>(RegClass);
    for (uint32_t Reg = FirstRegInClass; Reg <= LastRegInClass; ++Reg) {
      if (!ToPreserve[Reg]) {
        continue;
      }
      ++NumCallee;
      Variable *PhysicalRegister = getPhysicalRegister(RegNumT::fromInt(Reg));
      PreservedRegsSizeBytes +=
          typeWidthInBytesOnStack(PhysicalRegister->getType());
      PreservedRegsInClass->push_back(PhysicalRegister);
    }
  }

  Ctx->statsUpdateRegistersSaved(NumCallee);
  if (!PreservedSRegs.empty())
    _push(PreservedSRegs);
  if (!PreservedGPRs.empty())
    _push(PreservedGPRs);

  // Generate "mov FP, SP" if needed.
  if (UsesFramePointer) {
    Variable *FP = getPhysicalRegister(RegARM32::Reg_fp);
    Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
    _mov(FP, SP);
    // Keep FP live for late-stage liveness analysis (e.g. asm-verbose mode).
    Context.insert<InstFakeUse>(FP);
  }

  // Align the variables area. SpillAreaPaddingBytes is the size of the region
  // after the preserved registers and before the spill areas.
  // LocalsSlotsPaddingBytes is the amount of padding between the globals and
  // locals area if they are separate.
  assert(SpillAreaAlignmentBytes <= ARM32_STACK_ALIGNMENT_BYTES);
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
    SpillAreaSizeBytes += MaxOutArgsSizeBytes;
  } else {
    uint32_t StackOffset = PreservedRegsSizeBytes;
    uint32_t StackSize = applyStackAlignment(StackOffset + SpillAreaSizeBytes);
    StackSize = applyStackAlignment(StackSize + MaxOutArgsSizeBytes);
    SpillAreaSizeBytes = StackSize - StackOffset;
  }

  // Combine fixed alloca with SpillAreaSize.
  SpillAreaSizeBytes += FixedAllocaSizeBytes;

  // Generate "sub sp, SpillAreaSizeBytes"
  if (SpillAreaSizeBytes) {
    Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
    // Use the scratch register if needed to legalize the immediate.
    Operand *SubAmount = legalize(Ctx->getConstantInt32(SpillAreaSizeBytes),
                                  Legal_Reg | Legal_Flex, getReservedTmpReg());
    _sub(SP, SP, SubAmount);
    if (FixedAllocaAlignBytes > ARM32_STACK_ALIGNMENT_BYTES) {
      alignRegisterPow2(SP, FixedAllocaAlignBytes);
    }
  }

  Ctx->statsUpdateFrameBytes(SpillAreaSizeBytes);

  // Fill in stack offsets for stack args, and copy args into registers for
  // those that were register-allocated. Args are pushed right to left, so
  // Arg[0] is closest to the stack/frame pointer.
  Variable *FramePtr = getPhysicalRegister(getFrameOrStackReg());
  size_t BasicFrameOffset = PreservedRegsSizeBytes;
  if (!UsesFramePointer)
    BasicFrameOffset += SpillAreaSizeBytes;

  const VarList &Args = Func->getArgs();
  size_t InArgsSizeBytes = 0;
  TargetARM32::CallingConv CC;
  for (Variable *Arg : Args) {
    RegNumT DummyReg;
    const Type Ty = Arg->getType();

    // Skip arguments passed in registers.
    if (isScalarIntegerType(Ty)) {
      if (CC.argInGPR(Ty, &DummyReg)) {
        continue;
      }
    } else {
      if (CC.argInVFP(Ty, &DummyReg)) {
        continue;
      }
    }
    finishArgumentLowering(Arg, FramePtr, BasicFrameOffset, &InArgsSizeBytes);
  }

  // Fill in stack offsets for locals.
  assignVarStackSlots(SortedSpilledVariables, SpillAreaPaddingBytes,
                      SpillAreaSizeBytes, GlobalsAndSubsequentPaddingSize,
                      UsesFramePointer);
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
        << " is FP based = " << UsesFramePointer << "\n";
  }
}

void TargetARM32::addEpilog(CfgNode *Node) {
  InstList &Insts = Node->getInsts();
  InstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<InstARM32Ret>(*RI))
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

  Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
  if (UsesFramePointer) {
    Variable *FP = getPhysicalRegister(RegARM32::Reg_fp);
    // For late-stage liveness analysis (e.g. asm-verbose mode), adding a fake
    // use of SP before the assignment of SP=FP keeps previous SP adjustments
    // from being dead-code eliminated.
    Context.insert<InstFakeUse>(SP);
    _mov_redefined(SP, FP);
  } else {
    // add SP, SpillAreaSizeBytes
    if (SpillAreaSizeBytes) {
      // Use the scratch register if needed to legalize the immediate.
      Operand *AddAmount =
          legalize(Ctx->getConstantInt32(SpillAreaSizeBytes),
                   Legal_Reg | Legal_Flex, getReservedTmpReg());
      _add(SP, SP, AddAmount);
    }
  }

  if (!PreservedGPRs.empty())
    _pop(PreservedGPRs);
  if (!PreservedSRegs.empty())
    _pop(PreservedSRegs);
}

bool TargetARM32::isLegalMemOffset(Type Ty, int32_t Offset) const {
  constexpr bool ZeroExt = false;
  return OperandARM32Mem::canHoldOffset(Ty, ZeroExt, Offset);
}

Variable *TargetARM32::PostLoweringLegalizer::newBaseRegister(
    Variable *Base, int32_t Offset, RegNumT ScratchRegNum) {
  // Legalize will likely need a movw/movt combination, but if the top bits are
  // all 0 from negating the offset and subtracting, we could use that instead.
  const bool ShouldSub = Offset != 0 && (-Offset & 0xFFFF0000) == 0;
  Variable *ScratchReg = Target->makeReg(IceType_i32, ScratchRegNum);
  if (ShouldSub) {
    Operand *OffsetVal =
        Target->legalize(Target->Ctx->getConstantInt32(-Offset),
                         Legal_Reg | Legal_Flex, ScratchRegNum);
    Target->_sub(ScratchReg, Base, OffsetVal);
  } else {
    Operand *OffsetVal =
        Target->legalize(Target->Ctx->getConstantInt32(Offset),
                         Legal_Reg | Legal_Flex, ScratchRegNum);
    Target->_add(ScratchReg, Base, OffsetVal);
  }

  if (ScratchRegNum == Target->getReservedTmpReg()) {
    const bool BaseIsStackOrFramePtr =
        Base->getRegNum() == Target->getFrameOrStackReg();
    // There is currently no code path that would trigger this assertion, so we
    // leave this assertion here in case it is ever violated. This is not a
    // fatal error (thus the use of assert() and not llvm::report_fatal_error)
    // as the program compiled by subzero will still work correctly.
    assert(BaseIsStackOrFramePtr);
    // Side-effect: updates TempBase to reflect the new Temporary.
    if (BaseIsStackOrFramePtr) {
      TempBaseReg = ScratchReg;
      TempBaseOffset = Offset;
    } else {
      TempBaseReg = nullptr;
      TempBaseOffset = 0;
    }
  }

  return ScratchReg;
}

OperandARM32Mem *TargetARM32::PostLoweringLegalizer::createMemOperand(
    Type Ty, Variable *Base, int32_t Offset, bool AllowOffsets) {
  assert(!Base->isRematerializable());
  if (Offset == 0 || (AllowOffsets && Target->isLegalMemOffset(Ty, Offset))) {
    return OperandARM32Mem::create(
        Target->Func, Ty, Base,
        llvm::cast<ConstantInteger32>(Target->Ctx->getConstantInt32(Offset)),
        OperandARM32Mem::Offset);
  }

  if (!AllowOffsets || TempBaseReg == nullptr) {
    newBaseRegister(Base, Offset, Target->getReservedTmpReg());
  }

  int32_t OffsetDiff = Offset - TempBaseOffset;
  assert(AllowOffsets || OffsetDiff == 0);

  if (!Target->isLegalMemOffset(Ty, OffsetDiff)) {
    newBaseRegister(Base, Offset, Target->getReservedTmpReg());
    OffsetDiff = 0;
  }

  assert(!TempBaseReg->isRematerializable());
  return OperandARM32Mem::create(
      Target->Func, Ty, TempBaseReg,
      llvm::cast<ConstantInteger32>(Target->Ctx->getConstantInt32(OffsetDiff)),
      OperandARM32Mem::Offset);
}

void TargetARM32::PostLoweringLegalizer::resetTempBaseIfClobberedBy(
    const Inst *Instr) {
  bool ClobbersTempBase = false;
  if (TempBaseReg != nullptr) {
    Variable *Dest = Instr->getDest();
    if (llvm::isa<InstARM32Call>(Instr)) {
      // The following assertion is an invariant, so we remove it from the if
      // test. If the invariant is ever broken/invalidated/changed, remember
      // to add it back to the if condition.
      assert(TempBaseReg->getRegNum() == Target->getReservedTmpReg());
      // The linker may need to clobber IP if the call is too far from PC. Thus,
      // we assume IP will be overwritten.
      ClobbersTempBase = true;
    } else if (Dest != nullptr &&
               Dest->getRegNum() == TempBaseReg->getRegNum()) {
      // Register redefinition.
      ClobbersTempBase = true;
    }
  }

  if (ClobbersTempBase) {
    TempBaseReg = nullptr;
    TempBaseOffset = 0;
  }
}

void TargetARM32::PostLoweringLegalizer::legalizeMov(InstARM32Mov *MovInstr) {
  Variable *Dest = MovInstr->getDest();
  assert(Dest != nullptr);
  Type DestTy = Dest->getType();
  assert(DestTy != IceType_i64);

  Operand *Src = MovInstr->getSrc(0);
  Type SrcTy = Src->getType();
  (void)SrcTy;
  assert(SrcTy != IceType_i64);

  if (MovInstr->isMultiDest() || MovInstr->isMultiSource())
    return;

  bool Legalized = false;
  if (!Dest->hasReg()) {
    auto *SrcR = llvm::cast<Variable>(Src);
    assert(SrcR->hasReg());
    assert(!SrcR->isRematerializable());
    const int32_t Offset = Dest->getStackOffset();
    // This is a _mov(Mem(), Variable), i.e., a store.
    Target->_str(SrcR, createMemOperand(DestTy, StackOrFrameReg, Offset),
                 MovInstr->getPredicate());
    // _str() does not have a Dest, so we add a fake-def(Dest).
    Target->Context.insert<InstFakeDef>(Dest);
    Legalized = true;
  } else if (auto *Var = llvm::dyn_cast<Variable>(Src)) {
    if (Var->isRematerializable()) {
      // This is equivalent to an x86 _lea(RematOffset(%esp/%ebp), Variable).

      // ExtraOffset is only needed for frame-pointer based frames as we have
      // to account for spill storage.
      const int32_t ExtraOffset = (Var->getRegNum() == Target->getFrameReg())
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
        Target->_ldr(Dest, createMemOperand(DestTy, StackOrFrameReg, Offset),
                     MovInstr->getPredicate());
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

// ARM32 address modes:
//  ld/st i[8|16|32]: [reg], [reg +/- imm12], [pc +/- imm12],
//                    [reg +/- reg << shamt5]
//  ld/st f[32|64]  : [reg], [reg +/- imm8] , [pc +/- imm8]
//  ld/st vectors   : [reg]
//
// For now, we don't handle address modes with Relocatables.
namespace {
// MemTraits contains per-type valid address mode information.
#define X(tag, elementty, int_width, fp_width, uvec_width, svec_width, sbits,  \
          ubits, rraddr, shaddr)                                               \
  static_assert(!(shaddr) || rraddr, "Check ICETYPEARM32_TABLE::" #tag);
ICETYPEARM32_TABLE
#undef X

static const struct {
  int32_t ValidImmMask;
  bool CanHaveImm;
  bool CanHaveIndex;
  bool CanHaveShiftedIndex;
} MemTraits[] = {
#define X(tag, elementty, int_width, fp_width, uvec_width, svec_width, sbits,  \
          ubits, rraddr, shaddr)                                               \
  {                                                                            \
      (1 << ubits) - 1,                                                        \
      (ubits) > 0,                                                             \
      rraddr,                                                                  \
      shaddr,                                                                  \
  },
    ICETYPEARM32_TABLE
#undef X
};
static constexpr SizeT MemTraitsSize = llvm::array_lengthof(MemTraits);
} // end of anonymous namespace

OperandARM32Mem *
TargetARM32::PostLoweringLegalizer::legalizeMemOperand(OperandARM32Mem *Mem,
                                                       bool AllowOffsets) {
  assert(!Mem->isRegReg() || !Mem->getIndex()->isRematerializable());
  assert(Mem->isRegReg() || Target->isLegalMemOffset(
                                Mem->getType(), Mem->getOffset()->getValue()));

  bool Legalized = false;
  Variable *Base = Mem->getBase();
  int32_t Offset = Mem->isRegReg() ? 0 : Mem->getOffset()->getValue();
  if (Base->isRematerializable()) {
    const int32_t ExtraOffset = (Base->getRegNum() == Target->getFrameReg())
                                    ? Target->getFrameFixedAllocaOffset()
                                    : 0;
    Offset += Base->getStackOffset() + ExtraOffset;
    Base = Target->getPhysicalRegister(Base->getRegNum());
    assert(!Base->isRematerializable());
    Legalized = true;
  }

  if (!Legalized) {
    return nullptr;
  }

  if (!Mem->isRegReg()) {
    return createMemOperand(Mem->getType(), Base, Offset, AllowOffsets);
  }

  assert(MemTraits[Mem->getType()].CanHaveIndex);

  if (Offset != 0) {
    if (TempBaseReg == nullptr) {
      Base = newBaseRegister(Base, Offset, Target->getReservedTmpReg());
    } else {
      uint32_t Imm8, Rotate;
      const int32_t OffsetDiff = Offset - TempBaseOffset;
      if (OffsetDiff == 0) {
        Base = TempBaseReg;
      } else if (OperandARM32FlexImm::canHoldImm(OffsetDiff, &Rotate, &Imm8)) {
        auto *OffsetDiffF = OperandARM32FlexImm::create(
            Target->Func, IceType_i32, Imm8, Rotate);
        Target->_add(TempBaseReg, TempBaseReg, OffsetDiffF);
        TempBaseOffset += OffsetDiff;
        Base = TempBaseReg;
      } else if (OperandARM32FlexImm::canHoldImm(-OffsetDiff, &Rotate, &Imm8)) {
        auto *OffsetDiffF = OperandARM32FlexImm::create(
            Target->Func, IceType_i32, Imm8, Rotate);
        Target->_sub(TempBaseReg, TempBaseReg, OffsetDiffF);
        TempBaseOffset += OffsetDiff;
        Base = TempBaseReg;
      } else {
        Base = newBaseRegister(Base, Offset, Target->getReservedTmpReg());
      }
    }
  }

  return OperandARM32Mem::create(Target->Func, Mem->getType(), Base,
                                 Mem->getIndex(), Mem->getShiftOp(),
                                 Mem->getShiftAmt(), Mem->getAddrMode());
}

void TargetARM32::postLowerLegalization() {
  // If a stack variable's frame offset doesn't fit, convert from:
  //   ldr X, OFF[SP]
  // to:
  //   movw/movt TMP, OFF_PART
  //   add TMP, TMP, SP
  //   ldr X, OFF_MORE[TMP]
  //
  // This is safe because we have reserved TMP, and add for ARM does not
  // clobber the flags register.
  Func->dump("Before postLowerLegalization");
  assert(hasComputedFrame());
  // Do a fairly naive greedy clustering for now. Pick the first stack slot
  // that's out of bounds and make a new base reg using the architecture's temp
  // register. If that works for the next slot, then great. Otherwise, create a
  // new base register, clobbering the previous base register. Never share a
  // base reg across different basic blocks. This isn't ideal if local and
  // multi-block variables are far apart and their references are interspersed.
  // It may help to be more coordinated about assign stack slot numbers and may
  // help to assign smaller offsets to higher-weight variables so that they
  // don't depend on this legalization.
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    // One legalizer per basic block, otherwise we would share the Temporary
    // Base Register between basic blocks.
    PostLoweringLegalizer Legalizer(this);
    while (!Context.atEnd()) {
      PostIncrLoweringContext PostIncrement(Context);
      Inst *CurInstr = iteratorToInst(Context.getCur());

      // Check if the previous TempBaseReg is clobbered, and reset if needed.
      Legalizer.resetTempBaseIfClobberedBy(CurInstr);

      if (auto *MovInstr = llvm::dyn_cast<InstARM32Mov>(CurInstr)) {
        Legalizer.legalizeMov(MovInstr);
      } else if (auto *LdrInstr = llvm::dyn_cast<InstARM32Ldr>(CurInstr)) {
        if (OperandARM32Mem *LegalMem = Legalizer.legalizeMemOperand(
                llvm::cast<OperandARM32Mem>(LdrInstr->getSrc(0)))) {
          _ldr(CurInstr->getDest(), LegalMem, LdrInstr->getPredicate());
          CurInstr->setDeleted();
        }
      } else if (auto *LdrexInstr = llvm::dyn_cast<InstARM32Ldrex>(CurInstr)) {
        constexpr bool DisallowOffsetsBecauseLdrex = false;
        if (OperandARM32Mem *LegalMem = Legalizer.legalizeMemOperand(
                llvm::cast<OperandARM32Mem>(LdrexInstr->getSrc(0)),
                DisallowOffsetsBecauseLdrex)) {
          _ldrex(CurInstr->getDest(), LegalMem, LdrexInstr->getPredicate());
          CurInstr->setDeleted();
        }
      } else if (auto *StrInstr = llvm::dyn_cast<InstARM32Str>(CurInstr)) {
        if (OperandARM32Mem *LegalMem = Legalizer.legalizeMemOperand(
                llvm::cast<OperandARM32Mem>(StrInstr->getSrc(1)))) {
          _str(llvm::cast<Variable>(CurInstr->getSrc(0)), LegalMem,
               StrInstr->getPredicate());
          CurInstr->setDeleted();
        }
      } else if (auto *StrexInstr = llvm::dyn_cast<InstARM32Strex>(CurInstr)) {
        constexpr bool DisallowOffsetsBecauseStrex = false;
        if (OperandARM32Mem *LegalMem = Legalizer.legalizeMemOperand(
                llvm::cast<OperandARM32Mem>(StrexInstr->getSrc(1)),
                DisallowOffsetsBecauseStrex)) {
          _strex(CurInstr->getDest(), llvm::cast<Variable>(CurInstr->getSrc(0)),
                 LegalMem, StrexInstr->getPredicate());
          CurInstr->setDeleted();
        }
      }

      // Sanity-check: the Legalizer will either have no Temp, or it will be
      // bound to IP.
      Legalizer.assertNoTempOrAssignedToIP();
    }
  }
}

Operand *TargetARM32::loOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getLo();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand))
    return Ctx->getConstantInt32(static_cast<uint32_t>(Const->getValue()));
  if (auto *Mem = llvm::dyn_cast<OperandARM32Mem>(Operand)) {
    // Conservatively disallow memory operands with side-effects (pre/post
    // increment) in case of duplication.
    assert(Mem->getAddrMode() == OperandARM32Mem::Offset ||
           Mem->getAddrMode() == OperandARM32Mem::NegOffset);
    if (Mem->isRegReg()) {
      Variable *IndexR = legalizeToReg(Mem->getIndex());
      return OperandARM32Mem::create(Func, IceType_i32, Mem->getBase(), IndexR,
                                     Mem->getShiftOp(), Mem->getShiftAmt(),
                                     Mem->getAddrMode());
    } else {
      return OperandARM32Mem::create(Func, IceType_i32, Mem->getBase(),
                                     Mem->getOffset(), Mem->getAddrMode());
    }
  }
  llvm::report_fatal_error("Unsupported operand type");
  return nullptr;
}

Operand *TargetARM32::hiOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getHi();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand)) {
    return Ctx->getConstantInt32(
        static_cast<uint32_t>(Const->getValue() >> 32));
  }
  if (auto *Mem = llvm::dyn_cast<OperandARM32Mem>(Operand)) {
    // Conservatively disallow memory operands with side-effects in case of
    // duplication.
    assert(Mem->getAddrMode() == OperandARM32Mem::Offset ||
           Mem->getAddrMode() == OperandARM32Mem::NegOffset);
    const Type SplitType = IceType_i32;
    if (Mem->isRegReg()) {
      // We have to make a temp variable T, and add 4 to either Base or Index.
      // The Index may be shifted, so adding 4 can mean something else. Thus,
      // prefer T := Base + 4, and use T as the new Base.
      Variable *Base = Mem->getBase();
      Constant *Four = Ctx->getConstantInt32(4);
      Variable *NewBase = Func->makeVariable(Base->getType());
      lowerArithmetic(InstArithmetic::create(Func, InstArithmetic::Add, NewBase,
                                             Base, Four));
      Variable *BaseR = legalizeToReg(NewBase);
      Variable *IndexR = legalizeToReg(Mem->getIndex());
      return OperandARM32Mem::create(Func, SplitType, BaseR, IndexR,
                                     Mem->getShiftOp(), Mem->getShiftAmt(),
                                     Mem->getAddrMode());
    } else {
      Variable *Base = Mem->getBase();
      ConstantInteger32 *Offset = Mem->getOffset();
      assert(!Utils::WouldOverflowAdd(Offset->getValue(), 4));
      int32_t NextOffsetVal = Offset->getValue() + 4;
      constexpr bool ZeroExt = false;
      if (!OperandARM32Mem::canHoldOffset(SplitType, ZeroExt, NextOffsetVal)) {
        // We have to make a temp variable and add 4 to either Base or Offset.
        // If we add 4 to Offset, this will convert a non-RegReg addressing
        // mode into a RegReg addressing mode. Since NaCl sandboxing disallows
        // RegReg addressing modes, prefer adding to base and replacing
        // instead. Thus we leave the old offset alone.
        Constant *_4 = Ctx->getConstantInt32(4);
        Variable *NewBase = Func->makeVariable(Base->getType());
        lowerArithmetic(InstArithmetic::create(Func, InstArithmetic::Add,
                                               NewBase, Base, _4));
        Base = NewBase;
      } else {
        Offset =
            llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(NextOffsetVal));
      }
      Variable *BaseR = legalizeToReg(Base);
      return OperandARM32Mem::create(Func, SplitType, BaseR, Offset,
                                     Mem->getAddrMode());
    }
  }
  llvm::report_fatal_error("Unsupported operand type");
  return nullptr;
}

SmallBitVector TargetARM32::getRegisterSet(RegSetMask Include,
                                           RegSetMask Exclude) const {
  SmallBitVector Registers(RegARM32::Reg_NUM);

  for (uint32_t i = 0; i < RegARM32::Reg_NUM; ++i) {
    const auto &Entry = RegARM32::RegTable[i];
    if (Entry.Scratch && (Include & RegSet_CallerSave))
      Registers[i] = true;
    if (Entry.Preserved && (Include & RegSet_CalleeSave))
      Registers[i] = true;
    if (Entry.StackPtr && (Include & RegSet_StackPointer))
      Registers[i] = true;
    if (Entry.FramePtr && (Include & RegSet_FramePointer))
      Registers[i] = true;
    if (Entry.Scratch && (Exclude & RegSet_CallerSave))
      Registers[i] = false;
    if (Entry.Preserved && (Exclude & RegSet_CalleeSave))
      Registers[i] = false;
    if (Entry.StackPtr && (Exclude & RegSet_StackPointer))
      Registers[i] = false;
    if (Entry.FramePtr && (Exclude & RegSet_FramePointer))
      Registers[i] = false;
  }

  return Registers;
}

void TargetARM32::lowerAlloca(const InstAlloca *Instr) {
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
  assert(llvm::isPowerOf2_32(ARM32_STACK_ALIGNMENT_BYTES));

  const uint32_t Alignment =
      std::max(AlignmentParam, ARM32_STACK_ALIGNMENT_BYTES);
  const bool OverAligned = Alignment > ARM32_STACK_ALIGNMENT_BYTES;
  const bool OptM1 = Func->getOptLevel() == Opt_m1;
  const bool AllocaWithKnownOffset = Instr->getKnownFrameOffset();
  const bool UseFramePointer =
      hasFramePointer() || OverAligned || !AllocaWithKnownOffset || OptM1;

  if (UseFramePointer)
    setHasFramePointer();

  Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
  if (OverAligned) {
    alignRegisterPow2(SP, Alignment);
  }

  Variable *Dest = Instr->getDest();
  Operand *TotalSize = Instr->getSizeInBytes();

  if (const auto *ConstantTotalSize =
          llvm::dyn_cast<ConstantInteger32>(TotalSize)) {
    const uint32_t Value =
        Utils::applyAlignment(ConstantTotalSize->getValue(), Alignment);
    // Constant size alloca.
    if (!UseFramePointer) {
      // If we don't need a Frame Pointer, this alloca has a known offset to the
      // stack pointer. We don't need adjust the stack pointer, nor assign any
      // value to Dest, as Dest is rematerializable.
      assert(Dest->isRematerializable());
      FixedAllocaSizeBytes += Value;
      Context.insert<InstFakeDef>(Dest);
      return;
    }

    // If a frame pointer is required, then we need to store the alloca'd result
    // in Dest.
    Operand *SubAmountRF =
        legalize(Ctx->getConstantInt32(Value), Legal_Reg | Legal_Flex);
    _sub(SP, SP, SubAmountRF);
  } else {
    // Non-constant sizes need to be adjusted to the next highest multiple of
    // the required alignment at runtime.
    TotalSize = legalize(TotalSize, Legal_Reg | Legal_Flex);
    Variable *T = makeReg(IceType_i32);
    _mov(T, TotalSize);
    Operand *AddAmount = legalize(Ctx->getConstantInt32(Alignment - 1));
    _add(T, T, AddAmount);
    alignRegisterPow2(T, Alignment);
    _sub(SP, SP, T);
  }

  // Adds back a few bytes to SP to account for the out args area.
  Variable *T = SP;
  if (MaxOutArgsSizeBytes != 0) {
    T = makeReg(getPointerType());
    Operand *OutArgsSizeRF = legalize(
        Ctx->getConstantInt32(MaxOutArgsSizeBytes), Legal_Reg | Legal_Flex);
    _add(T, SP, OutArgsSizeRF);
  }

  _mov(Dest, T);
}

void TargetARM32::div0Check(Type Ty, Operand *SrcLo, Operand *SrcHi) {
  if (isGuaranteedNonzeroInt(SrcLo) || isGuaranteedNonzeroInt(SrcHi))
    return;
  Variable *SrcLoReg = legalizeToReg(SrcLo);
  switch (Ty) {
  default:
    llvm_unreachable(
        ("Unexpected type in div0Check: " + typeStdString(Ty)).c_str());
  case IceType_i8:
  case IceType_i16: {
    Operand *ShAmtImm = shAmtImm(32 - getScalarIntBitWidth(Ty));
    Variable *T = makeReg(IceType_i32);
    _lsls(T, SrcLoReg, ShAmtImm);
    Context.insert<InstFakeUse>(T);
  } break;
  case IceType_i32: {
    _tst(SrcLoReg, SrcLoReg);
    break;
  }
  case IceType_i64: {
    Variable *T = makeReg(IceType_i32);
    _orrs(T, SrcLoReg, legalize(SrcHi, Legal_Reg | Legal_Flex));
    // T isn't going to be used, but we need the side-effect of setting flags
    // from this operation.
    Context.insert<InstFakeUse>(T);
  }
  }
  auto *Label = InstARM32Label::create(Func, this);
  _br(Label, CondARM32::NE);
  _trap();
  Context.insert(Label);
}

void TargetARM32::lowerIDivRem(Variable *Dest, Variable *T, Variable *Src0R,
                               Operand *Src1, ExtInstr ExtFunc,
                               DivInstr DivFunc, bool IsRemainder) {
  div0Check(Dest->getType(), Src1, nullptr);
  Variable *Src1R = legalizeToReg(Src1);
  Variable *T0R = Src0R;
  Variable *T1R = Src1R;
  if (Dest->getType() != IceType_i32) {
    T0R = makeReg(IceType_i32);
    (this->*ExtFunc)(T0R, Src0R, CondARM32::AL);
    T1R = makeReg(IceType_i32);
    (this->*ExtFunc)(T1R, Src1R, CondARM32::AL);
  }
  if (hasCPUFeature(TargetARM32Features::HWDivArm)) {
    (this->*DivFunc)(T, T0R, T1R, CondARM32::AL);
    if (IsRemainder) {
      Variable *T2 = makeReg(IceType_i32);
      _mls(T2, T, T1R, T0R);
      T = T2;
    }
    _mov(Dest, T);
  } else {
    llvm::report_fatal_error("div should have already been turned into a call");
  }
}

TargetARM32::SafeBoolChain
TargetARM32::lowerInt1Arithmetic(const InstArithmetic *Instr) {
  Variable *Dest = Instr->getDest();
  assert(Dest->getType() == IceType_i1);

  // So folding didn't work for Instr. Not a problem: We just need to
  // materialize the Sources, and perform the operation. We create regular
  // Variables (and not infinite-weight ones) because this call might recurse a
  // lot, and we might end up with tons of infinite weight temporaries.
  assert(Instr->getSrcSize() == 2);
  Variable *Src0 = Func->makeVariable(IceType_i1);
  SafeBoolChain Src0Safe = lowerInt1(Src0, Instr->getSrc(0));

  Operand *Src1 = Instr->getSrc(1);
  SafeBoolChain Src1Safe = SBC_Yes;

  if (!llvm::isa<Constant>(Src1)) {
    Variable *Src1V = Func->makeVariable(IceType_i1);
    Src1Safe = lowerInt1(Src1V, Src1);
    Src1 = Src1V;
  }

  Variable *T = makeReg(IceType_i1);
  Src0 = legalizeToReg(Src0);
  Operand *Src1RF = legalize(Src1, Legal_Reg | Legal_Flex);
  switch (Instr->getOp()) {
  default:
    // If this Unreachable is ever executed, add the offending operation to
    // the list of valid consumers.
    llvm::report_fatal_error("Unhandled i1 Op");
  case InstArithmetic::And:
    _and(T, Src0, Src1RF);
    break;
  case InstArithmetic::Or:
    _orr(T, Src0, Src1RF);
    break;
  case InstArithmetic::Xor:
    _eor(T, Src0, Src1RF);
    break;
  }
  _mov(Dest, T);
  return Src0Safe == SBC_Yes && Src1Safe == SBC_Yes ? SBC_Yes : SBC_No;
}

namespace {
// NumericOperands is used during arithmetic/icmp lowering for constant folding.
// It holds the two sources operands, and maintains some state as to whether one
// of them is a constant. If one of the operands is a constant, then it will be
// be stored as the operation's second source, with a bit indicating whether the
// operands were swapped.
//
// The class is split into a base class with operand type-independent methods,
// and a derived, templated class, for each type of operand we want to fold
// constants for:
//
// NumericOperandsBase --> NumericOperands<ConstantFloat>
//                     --> NumericOperands<ConstantDouble>
//                     --> NumericOperands<ConstantInt32>
//
// NumericOperands<ConstantInt32> also exposes helper methods for emitting
// inverted/negated immediates.
class NumericOperandsBase {
  NumericOperandsBase() = delete;
  NumericOperandsBase(const NumericOperandsBase &) = delete;
  NumericOperandsBase &operator=(const NumericOperandsBase &) = delete;

public:
  NumericOperandsBase(Operand *S0, Operand *S1)
      : Src0(NonConstOperand(S0, S1)), Src1(ConstOperand(S0, S1)),
        Swapped(Src0 == S1 && S0 != S1) {
    assert(Src0 != nullptr);
    assert(Src1 != nullptr);
    assert(Src0 != Src1 || S0 == S1);
  }

  bool hasConstOperand() const {
    return llvm::isa<Constant>(Src1) && !llvm::isa<ConstantRelocatable>(Src1);
  }

  bool swappedOperands() const { return Swapped; }

  Variable *src0R(TargetARM32 *Target) const {
    return legalizeToReg(Target, Src0);
  }

  Variable *unswappedSrc0R(TargetARM32 *Target) const {
    return legalizeToReg(Target, Swapped ? Src1 : Src0);
  }

  Operand *src1RF(TargetARM32 *Target) const {
    return legalizeToRegOrFlex(Target, Src1);
  }

  Variable *unswappedSrc1R(TargetARM32 *Target) const {
    return legalizeToReg(Target, Swapped ? Src0 : Src1);
  }

  Operand *src1() const { return Src1; }

protected:
  Operand *const Src0;
  Operand *const Src1;
  const bool Swapped;

  static Variable *legalizeToReg(TargetARM32 *Target, Operand *Src) {
    return Target->legalizeToReg(Src);
  }

  static Operand *legalizeToRegOrFlex(TargetARM32 *Target, Operand *Src) {
    return Target->legalize(Src,
                            TargetARM32::Legal_Reg | TargetARM32::Legal_Flex);
  }

private:
  static Operand *NonConstOperand(Operand *S0, Operand *S1) {
    if (!llvm::isa<Constant>(S0))
      return S0;
    if (!llvm::isa<Constant>(S1))
      return S1;
    if (llvm::isa<ConstantRelocatable>(S1) &&
        !llvm::isa<ConstantRelocatable>(S0))
      return S1;
    return S0;
  }

  static Operand *ConstOperand(Operand *S0, Operand *S1) {
    if (!llvm::isa<Constant>(S0))
      return S1;
    if (!llvm::isa<Constant>(S1))
      return S0;
    if (llvm::isa<ConstantRelocatable>(S1) &&
        !llvm::isa<ConstantRelocatable>(S0))
      return S0;
    return S1;
  }
};

template <typename C> class NumericOperands : public NumericOperandsBase {
  NumericOperands() = delete;
  NumericOperands(const NumericOperands &) = delete;
  NumericOperands &operator=(const NumericOperands &) = delete;

public:
  NumericOperands(Operand *S0, Operand *S1) : NumericOperandsBase(S0, S1) {
    assert(!hasConstOperand() || llvm::isa<C>(this->Src1));
  }

  typename C::PrimType getConstantValue() const {
    return llvm::cast<C>(Src1)->getValue();
  }
};

using FloatOperands = NumericOperands<ConstantFloat>;
using DoubleOperands = NumericOperands<ConstantDouble>;

class Int32Operands : public NumericOperands<ConstantInteger32> {
  Int32Operands() = delete;
  Int32Operands(const Int32Operands &) = delete;
  Int32Operands &operator=(const Int32Operands &) = delete;

public:
  Int32Operands(Operand *S0, Operand *S1) : NumericOperands(S0, S1) {}

  Operand *unswappedSrc1RShAmtImm(TargetARM32 *Target) const {
    if (!swappedOperands() && hasConstOperand()) {
      return Target->shAmtImm(getConstantValue() & 0x1F);
    }
    return legalizeToReg(Target, Swapped ? Src0 : Src1);
  }

  bool isSrc1ImmediateZero() const {
    if (!swappedOperands() && hasConstOperand()) {
      return getConstantValue() == 0;
    }
    return false;
  }

  bool immediateIsFlexEncodable() const {
    uint32_t Rotate, Imm8;
    return OperandARM32FlexImm::canHoldImm(getConstantValue(), &Rotate, &Imm8);
  }

  bool negatedImmediateIsFlexEncodable() const {
    uint32_t Rotate, Imm8;
    return OperandARM32FlexImm::canHoldImm(
        -static_cast<int32_t>(getConstantValue()), &Rotate, &Imm8);
  }

  Operand *negatedSrc1F(TargetARM32 *Target) const {
    return legalizeToRegOrFlex(Target,
                               Target->getCtx()->getConstantInt32(
                                   -static_cast<int32_t>(getConstantValue())));
  }

  bool invertedImmediateIsFlexEncodable() const {
    uint32_t Rotate, Imm8;
    return OperandARM32FlexImm::canHoldImm(
        ~static_cast<uint32_t>(getConstantValue()), &Rotate, &Imm8);
  }

  Operand *invertedSrc1F(TargetARM32 *Target) const {
    return legalizeToRegOrFlex(Target,
                               Target->getCtx()->getConstantInt32(
                                   ~static_cast<uint32_t>(getConstantValue())));
  }
};
} // end of anonymous namespace

void TargetARM32::preambleDivRem(const InstCall *Instr) {
  Operand *Src1 = Instr->getArg(1);

  switch (Src1->getType()) {
  default:
    llvm::report_fatal_error("Invalid type for idiv.");
  case IceType_i64: {
    if (auto *C = llvm::dyn_cast<ConstantInteger64>(Src1)) {
      if (C->getValue() == 0) {
        _trap();
        return;
      }
    }
    div0Check(IceType_i64, loOperand(Src1), hiOperand(Src1));
    return;
  }
  case IceType_i32: {
    // Src0 and Src1 have already been appropriately extended to an i32, so we
    // don't check for i8 and i16.
    if (auto *C = llvm::dyn_cast<ConstantInteger32>(Src1)) {
      if (C->getValue() == 0) {
        _trap();
        return;
      }
    }
    div0Check(IceType_i32, Src1, nullptr);
    return;
  }
  }
}

void TargetARM32::lowerInt64Arithmetic(InstArithmetic::OpKind Op,
                                       Variable *Dest, Operand *Src0,
                                       Operand *Src1) {
  Int32Operands SrcsLo(loOperand(Src0), loOperand(Src1));
  Int32Operands SrcsHi(hiOperand(Src0), hiOperand(Src1));
  assert(SrcsLo.swappedOperands() == SrcsHi.swappedOperands());
  assert(SrcsLo.hasConstOperand() == SrcsHi.hasConstOperand());

  auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
  auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
  Variable *T_Lo = makeReg(DestLo->getType());
  Variable *T_Hi = makeReg(DestHi->getType());

  switch (Op) {
  case InstArithmetic::_num:
    llvm::report_fatal_error("Unknown arithmetic operator");
    return;
  case InstArithmetic::Add: {
    Variable *Src0LoR = SrcsLo.src0R(this);
    Operand *Src1LoRF = SrcsLo.src1RF(this);
    Variable *Src0HiR = SrcsHi.src0R(this);
    Operand *Src1HiRF = SrcsHi.src1RF(this);
    _adds(T_Lo, Src0LoR, Src1LoRF);
    _mov(DestLo, T_Lo);
    _adc(T_Hi, Src0HiR, Src1HiRF);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::And: {
    Variable *Src0LoR = SrcsLo.src0R(this);
    Operand *Src1LoRF = SrcsLo.src1RF(this);
    Variable *Src0HiR = SrcsHi.src0R(this);
    Operand *Src1HiRF = SrcsHi.src1RF(this);
    _and(T_Lo, Src0LoR, Src1LoRF);
    _mov(DestLo, T_Lo);
    _and(T_Hi, Src0HiR, Src1HiRF);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Or: {
    Variable *Src0LoR = SrcsLo.src0R(this);
    Operand *Src1LoRF = SrcsLo.src1RF(this);
    Variable *Src0HiR = SrcsHi.src0R(this);
    Operand *Src1HiRF = SrcsHi.src1RF(this);
    _orr(T_Lo, Src0LoR, Src1LoRF);
    _mov(DestLo, T_Lo);
    _orr(T_Hi, Src0HiR, Src1HiRF);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Xor: {
    Variable *Src0LoR = SrcsLo.src0R(this);
    Operand *Src1LoRF = SrcsLo.src1RF(this);
    Variable *Src0HiR = SrcsHi.src0R(this);
    Operand *Src1HiRF = SrcsHi.src1RF(this);
    _eor(T_Lo, Src0LoR, Src1LoRF);
    _mov(DestLo, T_Lo);
    _eor(T_Hi, Src0HiR, Src1HiRF);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Sub: {
    Variable *Src0LoR = SrcsLo.src0R(this);
    Operand *Src1LoRF = SrcsLo.src1RF(this);
    Variable *Src0HiR = SrcsHi.src0R(this);
    Operand *Src1HiRF = SrcsHi.src1RF(this);
    if (SrcsLo.swappedOperands()) {
      _rsbs(T_Lo, Src0LoR, Src1LoRF);
      _mov(DestLo, T_Lo);
      _rsc(T_Hi, Src0HiR, Src1HiRF);
      _mov(DestHi, T_Hi);
    } else {
      _subs(T_Lo, Src0LoR, Src1LoRF);
      _mov(DestLo, T_Lo);
      _sbc(T_Hi, Src0HiR, Src1HiRF);
      _mov(DestHi, T_Hi);
    }
    return;
  }
  case InstArithmetic::Mul: {
    // GCC 4.8 does:
    // a=b*c ==>
    //   t_acc =(mul) (b.lo * c.hi)
    //   t_acc =(mla) (c.lo * b.hi) + t_acc
    //   t.hi,t.lo =(umull) b.lo * c.lo
    //   t.hi += t_acc
    //   a.lo = t.lo
    //   a.hi = t.hi
    //
    // LLVM does:
    //   t.hi,t.lo =(umull) b.lo * c.lo
    //   t.hi =(mla) (b.lo * c.hi) + t.hi
    //   t.hi =(mla) (b.hi * c.lo) + t.hi
    //   a.lo = t.lo
    //   a.hi = t.hi
    //
    // LLVM's lowering has fewer instructions, but more register pressure:
    // t.lo is live from beginning to end, while GCC delays the two-dest
    // instruction till the end, and kills c.hi immediately.
    Variable *T_Acc = makeReg(IceType_i32);
    Variable *T_Acc1 = makeReg(IceType_i32);
    Variable *T_Hi1 = makeReg(IceType_i32);
    Variable *Src0RLo = SrcsLo.unswappedSrc0R(this);
    Variable *Src0RHi = SrcsHi.unswappedSrc0R(this);
    Variable *Src1RLo = SrcsLo.unswappedSrc1R(this);
    Variable *Src1RHi = SrcsHi.unswappedSrc1R(this);
    _mul(T_Acc, Src0RLo, Src1RHi);
    _mla(T_Acc1, Src1RLo, Src0RHi, T_Acc);
    _umull(T_Lo, T_Hi1, Src0RLo, Src1RLo);
    _add(T_Hi, T_Hi1, T_Acc1);
    _mov(DestLo, T_Lo);
    _mov(DestHi, T_Hi);
    return;
  }
  case InstArithmetic::Shl: {
    if (!SrcsLo.swappedOperands() && SrcsLo.hasConstOperand()) {
      Variable *Src0RLo = SrcsLo.src0R(this);
      // Truncating the ShAmt to [0, 63] because that's what ARM does anyway.
      const int32_t ShAmtImm = SrcsLo.getConstantValue() & 0x3F;
      if (ShAmtImm == 0) {
        _mov(DestLo, Src0RLo);
        _mov(DestHi, SrcsHi.src0R(this));
        return;
      }

      if (ShAmtImm >= 32) {
        if (ShAmtImm == 32) {
          _mov(DestHi, Src0RLo);
        } else {
          Operand *ShAmtOp = shAmtImm(ShAmtImm - 32);
          _lsl(T_Hi, Src0RLo, ShAmtOp);
          _mov(DestHi, T_Hi);
        }

        Operand *_0 =
            legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
        _mov(T_Lo, _0);
        _mov(DestLo, T_Lo);
        return;
      }

      Variable *Src0RHi = SrcsHi.src0R(this);
      Operand *ShAmtOp = shAmtImm(ShAmtImm);
      Operand *ComplShAmtOp = shAmtImm(32 - ShAmtImm);
      _lsl(T_Hi, Src0RHi, ShAmtOp);
      _orr(T_Hi, T_Hi,
           OperandARM32FlexReg::create(Func, IceType_i32, Src0RLo,
                                       OperandARM32::LSR, ComplShAmtOp));
      _mov(DestHi, T_Hi);

      _lsl(T_Lo, Src0RLo, ShAmtOp);
      _mov(DestLo, T_Lo);
      return;
    }

    // a=b<<c ==>
    // pnacl-llc does:
    // mov     t_b.lo, b.lo
    // mov     t_b.hi, b.hi
    // mov     t_c.lo, c.lo
    // rsb     T0, t_c.lo, #32
    // lsr     T1, t_b.lo, T0
    // orr     t_a.hi, T1, t_b.hi, lsl t_c.lo
    // sub     T2, t_c.lo, #32
    // cmp     T2, #0
    // lslge   t_a.hi, t_b.lo, T2
    // lsl     t_a.lo, t_b.lo, t_c.lo
    // mov     a.lo, t_a.lo
    // mov     a.hi, t_a.hi
    //
    // GCC 4.8 does:
    // sub t_c1, c.lo, #32
    // lsl t_hi, b.hi, c.lo
    // orr t_hi, t_hi, b.lo, lsl t_c1
    // rsb t_c2, c.lo, #32
    // orr t_hi, t_hi, b.lo, lsr t_c2
    // lsl t_lo, b.lo, c.lo
    // a.lo = t_lo
    // a.hi = t_hi
    //
    // These are incompatible, therefore we mimic pnacl-llc.
    // Can be strength-reduced for constant-shifts, but we don't do that for
    // now.
    // Given the sub/rsb T_C, C.lo, #32, one of the T_C will be negative. On
    // ARM, shifts only take the lower 8 bits of the shift register, and
    // saturate to the range 0-32, so the negative value will saturate to 32.
    Operand *_32 = legalize(Ctx->getConstantInt32(32), Legal_Reg | Legal_Flex);
    Operand *_0 =
        legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
    Variable *T0 = makeReg(IceType_i32);
    Variable *T1 = makeReg(IceType_i32);
    Variable *T2 = makeReg(IceType_i32);
    Variable *TA_Hi = makeReg(IceType_i32);
    Variable *TA_Lo = makeReg(IceType_i32);
    Variable *Src0RLo = SrcsLo.unswappedSrc0R(this);
    Variable *Src0RHi = SrcsHi.unswappedSrc0R(this);
    Variable *Src1RLo = SrcsLo.unswappedSrc1R(this);
    _rsb(T0, Src1RLo, _32);
    _lsr(T1, Src0RLo, T0);
    _orr(TA_Hi, T1,
         OperandARM32FlexReg::create(Func, IceType_i32, Src0RHi,
                                     OperandARM32::LSL, Src1RLo));
    _sub(T2, Src1RLo, _32);
    _cmp(T2, _0);
    _lsl(TA_Hi, Src0RLo, T2, CondARM32::GE);
    _set_dest_redefined();
    _lsl(TA_Lo, Src0RLo, Src1RLo);
    _mov(DestLo, TA_Lo);
    _mov(DestHi, TA_Hi);
    return;
  }
  case InstArithmetic::Lshr:
  case InstArithmetic::Ashr: {
    const bool ASR = Op == InstArithmetic::Ashr;
    if (!SrcsLo.swappedOperands() && SrcsLo.hasConstOperand()) {
      Variable *Src0RHi = SrcsHi.src0R(this);
      // Truncating the ShAmt to [0, 63] because that's what ARM does anyway.
      const int32_t ShAmt = SrcsLo.getConstantValue() & 0x3F;
      if (ShAmt == 0) {
        _mov(DestHi, Src0RHi);
        _mov(DestLo, SrcsLo.src0R(this));
        return;
      }

      if (ShAmt >= 32) {
        if (ShAmt == 32) {
          _mov(DestLo, Src0RHi);
        } else {
          Operand *ShAmtImm = shAmtImm(ShAmt - 32);
          if (ASR) {
            _asr(T_Lo, Src0RHi, ShAmtImm);
          } else {
            _lsr(T_Lo, Src0RHi, ShAmtImm);
          }
          _mov(DestLo, T_Lo);
        }

        if (ASR) {
          Operand *_31 = shAmtImm(31);
          _asr(T_Hi, Src0RHi, _31);
        } else {
          Operand *_0 = legalize(Ctx->getConstantZero(IceType_i32),
                                 Legal_Reg | Legal_Flex);
          _mov(T_Hi, _0);
        }
        _mov(DestHi, T_Hi);
        return;
      }

      Variable *Src0RLo = SrcsLo.src0R(this);
      Operand *ShAmtImm = shAmtImm(ShAmt);
      Operand *ComplShAmtImm = shAmtImm(32 - ShAmt);
      _lsr(T_Lo, Src0RLo, ShAmtImm);
      _orr(T_Lo, T_Lo,
           OperandARM32FlexReg::create(Func, IceType_i32, Src0RHi,
                                       OperandARM32::LSL, ComplShAmtImm));
      _mov(DestLo, T_Lo);

      if (ASR) {
        _asr(T_Hi, Src0RHi, ShAmtImm);
      } else {
        _lsr(T_Hi, Src0RHi, ShAmtImm);
      }
      _mov(DestHi, T_Hi);
      return;
    }

    // a=b>>c
    // pnacl-llc does:
    // mov        t_b.lo, b.lo
    // mov        t_b.hi, b.hi
    // mov        t_c.lo, c.lo
    // lsr        T0, t_b.lo, t_c.lo
    // rsb        T1, t_c.lo, #32
    // orr        t_a.lo, T0, t_b.hi, lsl T1
    // sub        T2, t_c.lo, #32
    // cmp        T2, #0
    // [al]srge   t_a.lo, t_b.hi, T2
    // [al]sr     t_a.hi, t_b.hi, t_c.lo
    // mov        a.lo, t_a.lo
    // mov        a.hi, t_a.hi
    //
    // GCC 4.8 does (lsr):
    // rsb        t_c1, c.lo, #32
    // lsr        t_lo, b.lo, c.lo
    // orr        t_lo, t_lo, b.hi, lsl t_c1
    // sub        t_c2, c.lo, #32
    // orr        t_lo, t_lo, b.hi, lsr t_c2
    // lsr        t_hi, b.hi, c.lo
    // mov        a.lo, t_lo
    // mov        a.hi, t_hi
    //
    // These are incompatible, therefore we mimic pnacl-llc.
    Operand *_32 = legalize(Ctx->getConstantInt32(32), Legal_Reg | Legal_Flex);
    Operand *_0 =
        legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
    Variable *T0 = makeReg(IceType_i32);
    Variable *T1 = makeReg(IceType_i32);
    Variable *T2 = makeReg(IceType_i32);
    Variable *TA_Lo = makeReg(IceType_i32);
    Variable *TA_Hi = makeReg(IceType_i32);
    Variable *Src0RLo = SrcsLo.unswappedSrc0R(this);
    Variable *Src0RHi = SrcsHi.unswappedSrc0R(this);
    Variable *Src1RLo = SrcsLo.unswappedSrc1R(this);
    _lsr(T0, Src0RLo, Src1RLo);
    _rsb(T1, Src1RLo, _32);
    _orr(TA_Lo, T0,
         OperandARM32FlexReg::create(Func, IceType_i32, Src0RHi,
                                     OperandARM32::LSL, T1));
    _sub(T2, Src1RLo, _32);
    _cmp(T2, _0);
    if (ASR) {
      _asr(TA_Lo, Src0RHi, T2, CondARM32::GE);
      _set_dest_redefined();
      _asr(TA_Hi, Src0RHi, Src1RLo);
    } else {
      _lsr(TA_Lo, Src0RHi, T2, CondARM32::GE);
      _set_dest_redefined();
      _lsr(TA_Hi, Src0RHi, Src1RLo);
    }
    _mov(DestLo, TA_Lo);
    _mov(DestHi, TA_Hi);
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
    llvm::report_fatal_error("Call-helper-involved instruction for i64 type "
                             "should have already been handled before");
    return;
  }
}

namespace {
// StrengthReduction is a namespace with the strength reduction machinery. The
// entry point is the StrengthReduction::tryToOptimize method. It returns true
// if the optimization can be performed, and false otherwise.
//
// If the optimization can be performed, tryToOptimize sets its NumOperations
// parameter to the number of shifts that are needed to perform the
// multiplication; and it sets the Operations parameter with <ShAmt, AddOrSub>
// tuples that describe how to materialize the multiplication.
//
// The algorithm finds contiguous 1s in the Multiplication source, and uses one
// or two shifts to materialize it. A sequence of 1s, e.g.,
//
//                  M           N
//   ...00000000000011111...111110000000...
//
// is materializable with (1 << (M + 1)) - (1 << N):
//
//   ...00000000000100000...000000000000...      [1 << (M + 1)]
//   ...00000000000000000...000010000000... (-)  [1 << N]
//   --------------------------------------
//   ...00000000000011111...111110000000...
//
// And a single bit set, which is just a left shift.
namespace StrengthReduction {
enum AggregationOperation {
  AO_Invalid,
  AO_Add,
  AO_Sub,
};

// AggregateElement is a glorified <ShAmt, AddOrSub> tuple.
class AggregationElement {
  AggregationElement(const AggregationElement &) = delete;

public:
  AggregationElement() = default;
  AggregationElement &operator=(const AggregationElement &) = default;
  AggregationElement(AggregationOperation Op, uint32_t ShAmt)
      : Op(Op), ShAmt(ShAmt) {}

  Operand *createShiftedOperand(Cfg *Func, Variable *OpR) const {
    assert(OpR->mustHaveReg());
    if (ShAmt == 0) {
      return OpR;
    }
    return OperandARM32FlexReg::create(
        Func, IceType_i32, OpR, OperandARM32::LSL,
        OperandARM32ShAmtImm::create(
            Func, llvm::cast<ConstantInteger32>(
                      Func->getContext()->getConstantInt32(ShAmt))));
  }

  bool aggregateWithAdd() const {
    switch (Op) {
    case AO_Invalid:
      llvm::report_fatal_error("Invalid Strength Reduction Operations.");
    case AO_Add:
      return true;
    case AO_Sub:
      return false;
    }
    llvm_unreachable("(silence g++ warning)");
  }

  uint32_t shAmt() const { return ShAmt; }

private:
  AggregationOperation Op = AO_Invalid;
  uint32_t ShAmt;
};

// [RangeStart, RangeEnd] is a range of 1s in Src.
template <std::size_t N>
bool addOperations(uint32_t RangeStart, uint32_t RangeEnd, SizeT *NumOperations,
                   std::array<AggregationElement, N> *Operations) {
  assert(*NumOperations < N);
  if (RangeStart == RangeEnd) {
    // Single bit set:
    // Src           : 0...00010...
    // RangeStart    :        ^
    // RangeEnd      :        ^
    // NegSrc        : 0...00001...
    (*Operations)[*NumOperations] = AggregationElement(AO_Add, RangeStart);
    ++(*NumOperations);
    return true;
  }

  // Sequence of 1s: (two operations required.)
  // Src           : 0...00011...110...
  // RangeStart    :        ^
  // RangeEnd      :              ^
  // NegSrc        : 0...00000...001...
  if (*NumOperations + 1 >= N) {
    return false;
  }
  (*Operations)[*NumOperations] = AggregationElement(AO_Add, RangeStart + 1);
  ++(*NumOperations);
  (*Operations)[*NumOperations] = AggregationElement(AO_Sub, RangeEnd);
  ++(*NumOperations);
  return true;
}

// tryToOptmize scans Src looking for sequences of 1s (including the unitary bit
// 1 surrounded by zeroes.
template <std::size_t N>
bool tryToOptimize(uint32_t Src, SizeT *NumOperations,
                   std::array<AggregationElement, N> *Operations) {
  constexpr uint32_t SrcSizeBits = sizeof(Src) * CHAR_BIT;
  uint32_t NegSrc = ~Src;

  *NumOperations = 0;
  while (Src != 0 && *NumOperations < N) {
    // Each step of the algorithm:
    //   * finds L, the last bit set in Src;
    //   * clears all the upper bits in NegSrc up to bit L;
    //   * finds nL, the last bit set in NegSrc;
    //   * clears all the upper bits in Src up to bit nL;
    //
    // if L == nL + 1, then a unitary 1 was found in Src. Otherwise, a sequence
    // of 1s starting at L, and ending at nL + 1, was found.
    const uint32_t SrcLastBitSet = llvm::findLastSet(Src);
    const uint32_t NegSrcClearMask =
        (SrcLastBitSet == 0) ? 0
                             : (0xFFFFFFFFu) >> (SrcSizeBits - SrcLastBitSet);
    NegSrc &= NegSrcClearMask;
    if (NegSrc == 0) {
      if (addOperations(SrcLastBitSet, 0, NumOperations, Operations)) {
        return true;
      }
      return false;
    }
    const uint32_t NegSrcLastBitSet = llvm::findLastSet(NegSrc);
    assert(NegSrcLastBitSet < SrcLastBitSet);
    const uint32_t SrcClearMask =
        (NegSrcLastBitSet == 0)
            ? 0
            : (0xFFFFFFFFu) >> (SrcSizeBits - NegSrcLastBitSet);
    Src &= SrcClearMask;
    if (!addOperations(SrcLastBitSet, NegSrcLastBitSet + 1, NumOperations,
                       Operations)) {
      return false;
    }
  }

  return Src == 0;
}
} // end of namespace StrengthReduction
} // end of anonymous namespace

void TargetARM32::lowerArithmetic(const InstArithmetic *Instr) {
  Variable *Dest = Instr->getDest();

  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }

  Type DestTy = Dest->getType();
  if (DestTy == IceType_i1) {
    lowerInt1Arithmetic(Instr);
    return;
  }

  Operand *Src0 = legalizeUndef(Instr->getSrc(0));
  Operand *Src1 = legalizeUndef(Instr->getSrc(1));
  if (DestTy == IceType_i64) {
    lowerInt64Arithmetic(Instr->getOp(), Instr->getDest(), Src0, Src1);
    return;
  }

  if (isVectorType(DestTy)) {
    switch (Instr->getOp()) {
    default:
      UnimplementedLoweringError(this, Instr);
      return;
    // Explicitly allow vector instructions we have implemented/enabled.
    case InstArithmetic::Add:
    case InstArithmetic::And:
    case InstArithmetic::Ashr:
    case InstArithmetic::Fadd:
    case InstArithmetic::Fmul:
    case InstArithmetic::Fsub:
    case InstArithmetic::Lshr:
    case InstArithmetic::Mul:
    case InstArithmetic::Or:
    case InstArithmetic::Shl:
    case InstArithmetic::Sub:
    case InstArithmetic::Xor:
      break;
    }
  }

  Variable *T = makeReg(DestTy);

  // * Handle div/rem separately. They require a non-legalized Src1 to inspect
  // whether or not Src1 is a non-zero constant. Once legalized it is more
  // difficult to determine (constant may be moved to a register).
  // * Handle floating point arithmetic separately: they require Src1 to be
  // legalized to a register.
  switch (Instr->getOp()) {
  default:
    break;
  case InstArithmetic::Udiv: {
    constexpr bool NotRemainder = false;
    Variable *Src0R = legalizeToReg(Src0);
    lowerIDivRem(Dest, T, Src0R, Src1, &TargetARM32::_uxt, &TargetARM32::_udiv,
                 NotRemainder);
    return;
  }
  case InstArithmetic::Sdiv: {
    constexpr bool NotRemainder = false;
    Variable *Src0R = legalizeToReg(Src0);
    lowerIDivRem(Dest, T, Src0R, Src1, &TargetARM32::_sxt, &TargetARM32::_sdiv,
                 NotRemainder);
    return;
  }
  case InstArithmetic::Urem: {
    constexpr bool IsRemainder = true;
    Variable *Src0R = legalizeToReg(Src0);
    lowerIDivRem(Dest, T, Src0R, Src1, &TargetARM32::_uxt, &TargetARM32::_udiv,
                 IsRemainder);
    return;
  }
  case InstArithmetic::Srem: {
    constexpr bool IsRemainder = true;
    Variable *Src0R = legalizeToReg(Src0);
    lowerIDivRem(Dest, T, Src0R, Src1, &TargetARM32::_sxt, &TargetARM32::_sdiv,
                 IsRemainder);
    return;
  }
  case InstArithmetic::Frem: {
    if (!isScalarFloatingType(DestTy)) {
      llvm::report_fatal_error("Unexpected type when lowering frem.");
    }
    llvm::report_fatal_error("Frem should have already been lowered.");
  }
  case InstArithmetic::Fadd: {
    Variable *Src0R = legalizeToReg(Src0);
    if (const Inst *Src1Producer = Computations.getProducerOf(Src1)) {
      Variable *Src1R = legalizeToReg(Src1Producer->getSrc(0));
      Variable *Src2R = legalizeToReg(Src1Producer->getSrc(1));
      _vmla(Src0R, Src1R, Src2R);
      _mov(Dest, Src0R);
      return;
    }

    Variable *Src1R = legalizeToReg(Src1);
    _vadd(T, Src0R, Src1R);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Fsub: {
    Variable *Src0R = legalizeToReg(Src0);
    if (const Inst *Src1Producer = Computations.getProducerOf(Src1)) {
      Variable *Src1R = legalizeToReg(Src1Producer->getSrc(0));
      Variable *Src2R = legalizeToReg(Src1Producer->getSrc(1));
      _vmls(Src0R, Src1R, Src2R);
      _mov(Dest, Src0R);
      return;
    }
    Variable *Src1R = legalizeToReg(Src1);
    _vsub(T, Src0R, Src1R);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Fmul: {
    Variable *Src0R = legalizeToReg(Src0);
    Variable *Src1R = legalizeToReg(Src1);
    _vmul(T, Src0R, Src1R);
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Fdiv: {
    Variable *Src0R = legalizeToReg(Src0);
    Variable *Src1R = legalizeToReg(Src1);
    _vdiv(T, Src0R, Src1R);
    _mov(Dest, T);
    return;
  }
  }

  // Handle everything else here.
  Int32Operands Srcs(Src0, Src1);
  switch (Instr->getOp()) {
  case InstArithmetic::_num:
    llvm::report_fatal_error("Unknown arithmetic operator");
    return;
  case InstArithmetic::Add: {
    if (const Inst *Src1Producer = Computations.getProducerOf(Src1)) {
      assert(!isVectorType(DestTy));
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1Producer->getSrc(0));
      Variable *Src2R = legalizeToReg(Src1Producer->getSrc(1));
      _mla(T, Src1R, Src2R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Srcs.hasConstOperand()) {
      if (!Srcs.immediateIsFlexEncodable() &&
          Srcs.negatedImmediateIsFlexEncodable()) {
        assert(!isVectorType(DestTy));
        Variable *Src0R = Srcs.src0R(this);
        Operand *Src1F = Srcs.negatedSrc1F(this);
        if (!Srcs.swappedOperands()) {
          _sub(T, Src0R, Src1F);
        } else {
          _rsb(T, Src0R, Src1F);
        }
        _mov(Dest, T);
        return;
      }
    }
    Variable *Src0R = Srcs.src0R(this);
    if (isVectorType(DestTy)) {
      Variable *Src1R = legalizeToReg(Src1);
      _vadd(T, Src0R, Src1R);
    } else {
      Operand *Src1RF = Srcs.src1RF(this);
      _add(T, Src0R, Src1RF);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::And: {
    if (Srcs.hasConstOperand()) {
      if (!Srcs.immediateIsFlexEncodable() &&
          Srcs.invertedImmediateIsFlexEncodable()) {
        Variable *Src0R = Srcs.src0R(this);
        Operand *Src1F = Srcs.invertedSrc1F(this);
        _bic(T, Src0R, Src1F);
        _mov(Dest, T);
        return;
      }
    }
    assert(isIntegerType(DestTy));
    Variable *Src0R = Srcs.src0R(this);
    if (isVectorType(DestTy)) {
      Variable *Src1R = legalizeToReg(Src1);
      _vand(T, Src0R, Src1R);
    } else {
      Operand *Src1RF = Srcs.src1RF(this);
      _and(T, Src0R, Src1RF);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Or: {
    Variable *Src0R = Srcs.src0R(this);
    assert(isIntegerType(DestTy));
    if (isVectorType(DestTy)) {
      Variable *Src1R = legalizeToReg(Src1);
      _vorr(T, Src0R, Src1R);
    } else {
      Operand *Src1RF = Srcs.src1RF(this);
      _orr(T, Src0R, Src1RF);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Xor: {
    Variable *Src0R = Srcs.src0R(this);
    assert(isIntegerType(DestTy));
    if (isVectorType(DestTy)) {
      Variable *Src1R = legalizeToReg(Src1);
      _veor(T, Src0R, Src1R);
    } else {
      Operand *Src1RF = Srcs.src1RF(this);
      _eor(T, Src0R, Src1RF);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Sub: {
    if (const Inst *Src1Producer = Computations.getProducerOf(Src1)) {
      assert(!isVectorType(DestTy));
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1Producer->getSrc(0));
      Variable *Src2R = legalizeToReg(Src1Producer->getSrc(1));
      _mls(T, Src1R, Src2R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Srcs.hasConstOperand()) {
      assert(!isVectorType(DestTy));
      if (Srcs.immediateIsFlexEncodable()) {
        Variable *Src0R = Srcs.src0R(this);
        Operand *Src1RF = Srcs.src1RF(this);
        if (Srcs.swappedOperands()) {
          _rsb(T, Src0R, Src1RF);
        } else {
          _sub(T, Src0R, Src1RF);
        }
        _mov(Dest, T);
        return;
      }
      if (!Srcs.swappedOperands() && Srcs.negatedImmediateIsFlexEncodable()) {
        Variable *Src0R = Srcs.src0R(this);
        Operand *Src1F = Srcs.negatedSrc1F(this);
        _add(T, Src0R, Src1F);
        _mov(Dest, T);
        return;
      }
    }
    Variable *Src0R = Srcs.unswappedSrc0R(this);
    Variable *Src1R = Srcs.unswappedSrc1R(this);
    if (isVectorType(DestTy)) {
      _vsub(T, Src0R, Src1R);
    } else {
      _sub(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Mul: {
    const bool OptM1 = Func->getOptLevel() == Opt_m1;
    if (!OptM1 && Srcs.hasConstOperand()) {
      constexpr std::size_t MaxShifts = 4;
      std::array<StrengthReduction::AggregationElement, MaxShifts> Shifts;
      SizeT NumOperations;
      int32_t Const = Srcs.getConstantValue();
      const bool Invert = Const < 0;
      const bool MultiplyByZero = Const == 0;
      Operand *_0 =
          legalize(Ctx->getConstantZero(DestTy), Legal_Reg | Legal_Flex);

      if (MultiplyByZero) {
        _mov(T, _0);
        _mov(Dest, T);
        return;
      }

      if (Invert) {
        Const = -Const;
      }

      if (StrengthReduction::tryToOptimize(Const, &NumOperations, &Shifts)) {
        assert(NumOperations >= 1);
        Variable *Src0R = Srcs.src0R(this);
        int32_t Start;
        int32_t End;
        if (NumOperations == 1 || Shifts[NumOperations - 1].shAmt() != 0) {
          // Multiplication by a power of 2 (NumOperations == 1); or
          // Multiplication by a even number not a power of 2.
          Start = 1;
          End = NumOperations;
          assert(Shifts[0].aggregateWithAdd());
          _lsl(T, Src0R, shAmtImm(Shifts[0].shAmt()));
        } else {
          // Multiplication by an odd number. Put the free barrel shifter to a
          // good use.
          Start = 0;
          End = NumOperations - 2;
          const StrengthReduction::AggregationElement &Last =
              Shifts[NumOperations - 1];
          const StrengthReduction::AggregationElement &SecondToLast =
              Shifts[NumOperations - 2];
          if (!Last.aggregateWithAdd()) {
            assert(SecondToLast.aggregateWithAdd());
            _rsb(T, Src0R, SecondToLast.createShiftedOperand(Func, Src0R));
          } else if (!SecondToLast.aggregateWithAdd()) {
            assert(Last.aggregateWithAdd());
            _sub(T, Src0R, SecondToLast.createShiftedOperand(Func, Src0R));
          } else {
            _add(T, Src0R, SecondToLast.createShiftedOperand(Func, Src0R));
          }
        }

        // Odd numbers :   S                                 E   I   I
        //               +---+---+---+---+---+---+ ... +---+---+---+---+
        //     Shifts  = |   |   |   |   |   |   | ... |   |   |   |   |
        //               +---+---+---+---+---+---+ ... +---+---+---+---+
        // Even numbers:   I   S                                     E
        //
        // S: Start; E: End; I: Init
        for (int32_t I = Start; I < End; ++I) {
          const StrengthReduction::AggregationElement &Current = Shifts[I];
          Operand *SrcF = Current.createShiftedOperand(Func, Src0R);
          if (Current.aggregateWithAdd()) {
            _add(T, T, SrcF);
          } else {
            _sub(T, T, SrcF);
          }
        }

        if (Invert) {
          // T = 0 - T.
          _rsb(T, T, _0);
        }

        _mov(Dest, T);
        return;
      }
    }
    Variable *Src0R = Srcs.unswappedSrc0R(this);
    Variable *Src1R = Srcs.unswappedSrc1R(this);
    if (isVectorType(DestTy)) {
      _vmul(T, Src0R, Src1R);
    } else {
      _mul(T, Src0R, Src1R);
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Shl: {
    Variable *Src0R = Srcs.unswappedSrc0R(this);
    if (!isVectorType(T->getType())) {
      if (Srcs.isSrc1ImmediateZero()) {
        _mov(T, Src0R);
      } else {
        Operand *Src1R = Srcs.unswappedSrc1RShAmtImm(this);
        _lsl(T, Src0R, Src1R);
      }
    } else {
      if (Srcs.hasConstOperand()) {
        ConstantInteger32 *ShAmt = llvm::cast<ConstantInteger32>(Srcs.src1());
        _vshl(T, Src0R, ShAmt);
      } else {
        auto *Src1R = Srcs.unswappedSrc1R(this);
        _vshl(T, Src0R, Src1R)->setSignType(InstARM32::FS_Unsigned);
      }
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Lshr: {
    Variable *Src0R = Srcs.unswappedSrc0R(this);
    if (!isVectorType(T->getType())) {
      if (DestTy != IceType_i32) {
        _uxt(Src0R, Src0R);
      }
      if (Srcs.isSrc1ImmediateZero()) {
        _mov(T, Src0R);
      } else {
        Operand *Src1R = Srcs.unswappedSrc1RShAmtImm(this);
        _lsr(T, Src0R, Src1R);
      }
    } else {
      if (Srcs.hasConstOperand()) {
        ConstantInteger32 *ShAmt = llvm::cast<ConstantInteger32>(Srcs.src1());
        _vshr(T, Src0R, ShAmt)->setSignType(InstARM32::FS_Unsigned);
      } else {
        auto *Src1R = Srcs.unswappedSrc1R(this);
        auto *Src1RNeg = makeReg(Src1R->getType());
        _vneg(Src1RNeg, Src1R);
        _vshl(T, Src0R, Src1RNeg)->setSignType(InstARM32::FS_Unsigned);
      }
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Ashr: {
    Variable *Src0R = Srcs.unswappedSrc0R(this);
    if (!isVectorType(T->getType())) {
      if (DestTy != IceType_i32) {
        _sxt(Src0R, Src0R);
      }
      if (Srcs.isSrc1ImmediateZero()) {
        _mov(T, Src0R);
      } else {
        _asr(T, Src0R, Srcs.unswappedSrc1RShAmtImm(this));
      }
    } else {
      if (Srcs.hasConstOperand()) {
        ConstantInteger32 *ShAmt = llvm::cast<ConstantInteger32>(Srcs.src1());
        _vshr(T, Src0R, ShAmt)->setSignType(InstARM32::FS_Signed);
      } else {
        auto *Src1R = Srcs.unswappedSrc1R(this);
        auto *Src1RNeg = makeReg(Src1R->getType());
        _vneg(Src1RNeg, Src1R);
        _vshl(T, Src0R, Src1RNeg)->setSignType(InstARM32::FS_Signed);
      }
    }
    _mov(Dest, T);
    return;
  }
  case InstArithmetic::Udiv:
  case InstArithmetic::Sdiv:
  case InstArithmetic::Urem:
  case InstArithmetic::Srem:
    llvm::report_fatal_error(
        "Integer div/rem should have been handled earlier.");
    return;
  case InstArithmetic::Fadd:
  case InstArithmetic::Fsub:
  case InstArithmetic::Fmul:
  case InstArithmetic::Fdiv:
  case InstArithmetic::Frem:
    llvm::report_fatal_error(
        "Floating point arith should have been handled earlier.");
    return;
  }
}

void TargetARM32::lowerAssign(const InstAssign *Instr) {
  Variable *Dest = Instr->getDest();

  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }

  Operand *Src0 = Instr->getSrc(0);
  assert(Dest->getType() == Src0->getType());
  if (Dest->getType() == IceType_i64) {
    Src0 = legalizeUndef(Src0);

    Variable *T_Lo = makeReg(IceType_i32);
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    Operand *Src0Lo = legalize(loOperand(Src0), Legal_Reg | Legal_Flex);
    _mov(T_Lo, Src0Lo);
    _mov(DestLo, T_Lo);

    Variable *T_Hi = makeReg(IceType_i32);
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Operand *Src0Hi = legalize(hiOperand(Src0), Legal_Reg | Legal_Flex);
    _mov(T_Hi, Src0Hi);
    _mov(DestHi, T_Hi);

    return;
  }

  Operand *NewSrc;
  if (Dest->hasReg()) {
    // If Dest already has a physical register, then legalize the Src operand
    // into a Variable with the same register assignment. This especially
    // helps allow the use of Flex operands.
    NewSrc = legalize(Src0, Legal_Reg | Legal_Flex, Dest->getRegNum());
  } else {
    // Dest could be a stack operand. Since we could potentially need to do a
    // Store (and store can only have Register operands), legalize this to a
    // register.
    NewSrc = legalize(Src0, Legal_Reg);
  }

  if (isVectorType(Dest->getType()) || isScalarFloatingType(Dest->getType())) {
    NewSrc = legalize(NewSrc, Legal_Reg | Legal_Mem);
  }
  _mov(Dest, NewSrc);
}

TargetARM32::ShortCircuitCondAndLabel TargetARM32::lowerInt1ForBranch(
    Operand *Boolean, const LowerInt1BranchTarget &TargetTrue,
    const LowerInt1BranchTarget &TargetFalse, uint32_t ShortCircuitable) {
  InstARM32Label *NewShortCircuitLabel = nullptr;
  Operand *_1 = legalize(Ctx->getConstantInt1(1), Legal_Reg | Legal_Flex);

  const Inst *Producer = Computations.getProducerOf(Boolean);

  if (Producer == nullptr) {
    // No producer, no problem: just do emit code to perform (Boolean & 1) and
    // set the flags register. The branch should be taken if the resulting flags
    // indicate a non-zero result.
    _tst(legalizeToReg(Boolean), _1);
    return ShortCircuitCondAndLabel(CondWhenTrue(CondARM32::NE));
  }

  switch (Producer->getKind()) {
  default:
    llvm::report_fatal_error("Unexpected producer.");
  case Inst::Icmp: {
    return ShortCircuitCondAndLabel(
        lowerIcmpCond(llvm::cast<InstIcmp>(Producer)));
  } break;
  case Inst::Fcmp: {
    return ShortCircuitCondAndLabel(
        lowerFcmpCond(llvm::cast<InstFcmp>(Producer)));
  } break;
  case Inst::Cast: {
    const auto *CastProducer = llvm::cast<InstCast>(Producer);
    assert(CastProducer->getCastKind() == InstCast::Trunc);
    Operand *Src = CastProducer->getSrc(0);
    if (Src->getType() == IceType_i64)
      Src = loOperand(Src);
    _tst(legalizeToReg(Src), _1);
    return ShortCircuitCondAndLabel(CondWhenTrue(CondARM32::NE));
  } break;
  case Inst::Arithmetic: {
    const auto *ArithProducer = llvm::cast<InstArithmetic>(Producer);
    switch (ArithProducer->getOp()) {
    default:
      llvm::report_fatal_error("Unhandled Arithmetic Producer.");
    case InstArithmetic::And: {
      if (!(ShortCircuitable & SC_And)) {
        NewShortCircuitLabel = InstARM32Label::create(Func, this);
      }

      LowerInt1BranchTarget NewTarget =
          TargetFalse.createForLabelOrDuplicate(NewShortCircuitLabel);

      ShortCircuitCondAndLabel CondAndLabel = lowerInt1ForBranch(
          Producer->getSrc(0), TargetTrue, NewTarget, SC_And);
      const CondWhenTrue &Cond = CondAndLabel.Cond;

      _br_short_circuit(NewTarget, Cond.invert());

      InstARM32Label *const ShortCircuitLabel = CondAndLabel.ShortCircuitTarget;
      if (ShortCircuitLabel != nullptr)
        Context.insert(ShortCircuitLabel);

      return ShortCircuitCondAndLabel(
          lowerInt1ForBranch(Producer->getSrc(1), TargetTrue, NewTarget, SC_All)
              .assertNoLabelAndReturnCond(),
          NewShortCircuitLabel);
    } break;
    case InstArithmetic::Or: {
      if (!(ShortCircuitable & SC_Or)) {
        NewShortCircuitLabel = InstARM32Label::create(Func, this);
      }

      LowerInt1BranchTarget NewTarget =
          TargetTrue.createForLabelOrDuplicate(NewShortCircuitLabel);

      ShortCircuitCondAndLabel CondAndLabel = lowerInt1ForBranch(
          Producer->getSrc(0), NewTarget, TargetFalse, SC_Or);
      const CondWhenTrue &Cond = CondAndLabel.Cond;

      _br_short_circuit(NewTarget, Cond);

      InstARM32Label *const ShortCircuitLabel = CondAndLabel.ShortCircuitTarget;
      if (ShortCircuitLabel != nullptr)
        Context.insert(ShortCircuitLabel);

      return ShortCircuitCondAndLabel(lowerInt1ForBranch(Producer->getSrc(1),
                                                         NewTarget, TargetFalse,
                                                         SC_All)
                                          .assertNoLabelAndReturnCond(),
                                      NewShortCircuitLabel);
    } break;
    }
  }
  }
}

void TargetARM32::lowerBr(const InstBr *Instr) {
  if (Instr->isUnconditional()) {
    _br(Instr->getTargetUnconditional());
    return;
  }

  CfgNode *TargetTrue = Instr->getTargetTrue();
  CfgNode *TargetFalse = Instr->getTargetFalse();
  ShortCircuitCondAndLabel CondAndLabel = lowerInt1ForBranch(
      Instr->getCondition(), LowerInt1BranchTarget(TargetTrue),
      LowerInt1BranchTarget(TargetFalse), SC_All);
  assert(CondAndLabel.ShortCircuitTarget == nullptr);

  const CondWhenTrue &Cond = CondAndLabel.Cond;
  if (Cond.WhenTrue1 != CondARM32::kNone) {
    assert(Cond.WhenTrue0 != CondARM32::AL);
    _br(TargetTrue, Cond.WhenTrue1);
  }

  switch (Cond.WhenTrue0) {
  default:
    _br(TargetTrue, TargetFalse, Cond.WhenTrue0);
    break;
  case CondARM32::kNone:
    _br(TargetFalse);
    break;
  case CondARM32::AL:
    _br(TargetTrue);
    break;
  }
}

void TargetARM32::lowerCall(const InstCall *Instr) {
  Operand *CallTarget = Instr->getCallTarget();
  if (Instr->isTargetHelperCall()) {
    auto TargetHelperPreamble = ARM32HelpersPreamble.find(CallTarget);
    if (TargetHelperPreamble != ARM32HelpersPreamble.end()) {
      (this->*TargetHelperPreamble->second)(Instr);
    }
  }
  MaybeLeafFunc = false;
  NeedsStackAlignment = true;

  // Assign arguments to registers and stack. Also reserve stack.
  TargetARM32::CallingConv CC;
  // Pair of Arg Operand -> GPR number assignments.
  llvm::SmallVector<std::pair<Operand *, RegNumT>, NumGPRArgs> GPRArgs;
  llvm::SmallVector<std::pair<Operand *, RegNumT>, NumFP32Args> FPArgs;
  // Pair of Arg Operand -> stack offset.
  llvm::SmallVector<std::pair<Operand *, int32_t>, 8> StackArgs;
  size_t ParameterAreaSizeBytes = 0;

  // Classify each argument operand according to the location where the
  // argument is passed.
  for (SizeT i = 0, NumArgs = Instr->getNumArgs(); i < NumArgs; ++i) {
    Operand *Arg = legalizeUndef(Instr->getArg(i));
    const Type Ty = Arg->getType();
    bool InReg = false;
    RegNumT Reg;
    if (isScalarIntegerType(Ty)) {
      InReg = CC.argInGPR(Ty, &Reg);
    } else {
      InReg = CC.argInVFP(Ty, &Reg);
    }

    if (!InReg) {
      ParameterAreaSizeBytes =
          applyStackAlignmentTy(ParameterAreaSizeBytes, Ty);
      StackArgs.push_back(std::make_pair(Arg, ParameterAreaSizeBytes));
      ParameterAreaSizeBytes += typeWidthInBytesOnStack(Ty);
      continue;
    }

    if (Ty == IceType_i64) {
      Operand *Lo = loOperand(Arg);
      Operand *Hi = hiOperand(Arg);
      GPRArgs.push_back(std::make_pair(
          Lo, RegNumT::fixme(RegARM32::getI64PairFirstGPRNum(Reg))));
      GPRArgs.push_back(std::make_pair(
          Hi, RegNumT::fixme(RegARM32::getI64PairSecondGPRNum(Reg))));
    } else if (isScalarIntegerType(Ty)) {
      GPRArgs.push_back(std::make_pair(Arg, Reg));
    } else {
      FPArgs.push_back(std::make_pair(Arg, Reg));
    }
  }

  // Adjust the parameter area so that the stack is aligned. It is assumed that
  // the stack is already aligned at the start of the calling sequence.
  ParameterAreaSizeBytes = applyStackAlignment(ParameterAreaSizeBytes);

  if (ParameterAreaSizeBytes > MaxOutArgsSizeBytes) {
    llvm::report_fatal_error("MaxOutArgsSizeBytes is not really a max.");
  }

  // Copy arguments that are passed on the stack to the appropriate stack
  // locations.
  Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
  for (auto &StackArg : StackArgs) {
    ConstantInteger32 *Loc =
        llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(StackArg.second));
    Type Ty = StackArg.first->getType();
    OperandARM32Mem *Addr;
    constexpr bool SignExt = false;
    if (OperandARM32Mem::canHoldOffset(Ty, SignExt, StackArg.second)) {
      Addr = OperandARM32Mem::create(Func, Ty, SP, Loc);
    } else {
      Variable *NewBase = Func->makeVariable(SP->getType());
      lowerArithmetic(
          InstArithmetic::create(Func, InstArithmetic::Add, NewBase, SP, Loc));
      Addr = formMemoryOperand(NewBase, Ty);
    }
    lowerStore(InstStore::create(Func, StackArg.first, Addr));
  }

  // Generate the call instruction. Assign its result to a temporary with high
  // register allocation weight.
  Variable *Dest = Instr->getDest();
  // ReturnReg doubles as ReturnRegLo as necessary.
  Variable *ReturnReg = nullptr;
  Variable *ReturnRegHi = nullptr;
  if (Dest) {
    switch (Dest->getType()) {
    case IceType_NUM:
      llvm::report_fatal_error("Invalid Call dest type");
      break;
    case IceType_void:
      break;
    case IceType_i1:
      assert(Computations.getProducerOf(Dest) == nullptr);
    // Fall-through intended.
    case IceType_i8:
    case IceType_i16:
    case IceType_i32:
      ReturnReg = makeReg(Dest->getType(), RegARM32::Reg_r0);
      break;
    case IceType_i64:
      ReturnReg = makeReg(IceType_i32, RegARM32::Reg_r0);
      ReturnRegHi = makeReg(IceType_i32, RegARM32::Reg_r1);
      break;
    case IceType_f32:
      ReturnReg = makeReg(Dest->getType(), RegARM32::Reg_s0);
      break;
    case IceType_f64:
      ReturnReg = makeReg(Dest->getType(), RegARM32::Reg_d0);
      break;
    case IceType_v4i1:
    case IceType_v8i1:
    case IceType_v16i1:
    case IceType_v16i8:
    case IceType_v8i16:
    case IceType_v4i32:
    case IceType_v4f32:
      ReturnReg = makeReg(Dest->getType(), RegARM32::Reg_q0);
      break;
    }
  }

  // Allow ConstantRelocatable to be left alone as a direct call, but force
  // other constants like ConstantInteger32 to be in a register and make it an
  // indirect call.
  if (!llvm::isa<ConstantRelocatable>(CallTarget)) {
    CallTarget = legalize(CallTarget, Legal_Reg);
  }

  // Copy arguments to be passed in registers to the appropriate registers.
  CfgVector<Variable *> RegArgs;
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

  InstARM32Call *NewCall = Context.insert<InstARM32Call>(ReturnReg, CallTarget);

  if (ReturnRegHi)
    Context.insert<InstFakeDef>(ReturnRegHi);

  // Insert a register-kill pseudo instruction.
  Context.insert<InstFakeKill>(NewCall);

  // Generate a FakeUse to keep the call live if necessary.
  if (Instr->hasSideEffects() && ReturnReg) {
    Context.insert<InstFakeUse>(ReturnReg);
  }

  if (Dest != nullptr) {
    // Assign the result of the call to Dest.
    if (ReturnReg != nullptr) {
      if (ReturnRegHi) {
        auto *Dest64On32 = llvm::cast<Variable64On32>(Dest);
        Variable *DestLo = Dest64On32->getLo();
        Variable *DestHi = Dest64On32->getHi();
        _mov(DestLo, ReturnReg);
        _mov(DestHi, ReturnRegHi);
      } else {
        if (isFloatingType(Dest->getType()) || isVectorType(Dest->getType())) {
          _mov(Dest, ReturnReg);
        } else {
          assert(isIntegerType(Dest->getType()) &&
                 typeWidthInBytes(Dest->getType()) <= 4);
          _mov(Dest, ReturnReg);
        }
      }
    }
  }

  if (Instr->isTargetHelperCall()) {
    auto TargetHelpersPostamble = ARM32HelpersPostamble.find(CallTarget);
    if (TargetHelpersPostamble != ARM32HelpersPostamble.end()) {
      (this->*TargetHelpersPostamble->second)(Instr);
    }
  }
}

namespace {
void configureBitcastTemporary(Variable64On32 *Var) {
  Var->setMustNotHaveReg();
  Var->getHi()->setMustHaveReg();
  Var->getLo()->setMustHaveReg();
}
} // end of anonymous namespace

void TargetARM32::lowerCast(const InstCast *Instr) {
  InstCast::OpKind CastKind = Instr->getCastKind();
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();
  Operand *Src0 = legalizeUndef(Instr->getSrc(0));
  switch (CastKind) {
  default:
    Func->setError("Cast type not supported");
    return;
  case InstCast::Sext: {
    if (isVectorType(DestTy)) {
      Variable *T0 = makeReg(DestTy);
      Variable *T1 = makeReg(DestTy);
      ConstantInteger32 *ShAmt = nullptr;
      switch (DestTy) {
      default:
        llvm::report_fatal_error("Unexpected type in vector sext.");
      case IceType_v16i8:
        ShAmt = llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(7));
        break;
      case IceType_v8i16:
        ShAmt = llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(15));
        break;
      case IceType_v4i32:
        ShAmt = llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(31));
        break;
      }
      auto *Src0R = legalizeToReg(Src0);
      _vshl(T0, Src0R, ShAmt);
      _vshr(T1, T0, ShAmt)->setSignType(InstARM32::FS_Signed);
      _mov(Dest, T1);
    } else if (DestTy == IceType_i64) {
      // t1=sxtb src; t2= mov t1 asr #31; dst.lo=t1; dst.hi=t2
      Constant *ShiftAmt = Ctx->getConstantInt32(31);
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(DestLo->getType());
      if (Src0->getType() == IceType_i32) {
        Operand *Src0RF = legalize(Src0, Legal_Reg | Legal_Flex);
        _mov(T_Lo, Src0RF);
      } else if (Src0->getType() != IceType_i1) {
        Variable *Src0R = legalizeToReg(Src0);
        _sxt(T_Lo, Src0R);
      } else {
        Operand *_0 = Ctx->getConstantZero(IceType_i32);
        Operand *_m1 = Ctx->getConstantInt32(-1);
        lowerInt1ForSelect(T_Lo, Src0, _m1, _0);
      }
      _mov(DestLo, T_Lo);
      Variable *T_Hi = makeReg(DestHi->getType());
      if (Src0->getType() != IceType_i1) {
        _mov(T_Hi, OperandARM32FlexReg::create(Func, IceType_i32, T_Lo,
                                               OperandARM32::ASR, ShiftAmt));
      } else {
        // For i1, the asr instruction is already done above.
        _mov(T_Hi, T_Lo);
      }
      _mov(DestHi, T_Hi);
    } else if (Src0->getType() != IceType_i1) {
      // t1 = sxt src; dst = t1
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T = makeReg(DestTy);
      _sxt(T, Src0R);
      _mov(Dest, T);
    } else {
      Constant *_0 = Ctx->getConstantZero(IceType_i32);
      Operand *_m1 = Ctx->getConstantInt(DestTy, -1);
      Variable *T = makeReg(DestTy);
      lowerInt1ForSelect(T, Src0, _m1, _0);
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Zext: {
    if (isVectorType(DestTy)) {
      auto *Mask = makeReg(DestTy);
      auto *_1 = Ctx->getConstantInt32(1);
      auto *T = makeReg(DestTy);
      auto *Src0R = legalizeToReg(Src0);
      _mov(Mask, _1);
      _vand(T, Src0R, Mask);
      _mov(Dest, T);
    } else if (DestTy == IceType_i64) {
      // t1=uxtb src; dst.lo=t1; dst.hi=0
      Operand *_0 =
          legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(DestLo->getType());

      switch (Src0->getType()) {
      default: {
        assert(Src0->getType() != IceType_i64);
        _uxt(T_Lo, legalizeToReg(Src0));
      } break;
      case IceType_i32: {
        _mov(T_Lo, legalize(Src0, Legal_Reg | Legal_Flex));
      } break;
      case IceType_i1: {
        SafeBoolChain Safe = lowerInt1(T_Lo, Src0);
        if (Safe == SBC_No) {
          Operand *_1 =
              legalize(Ctx->getConstantInt1(1), Legal_Reg | Legal_Flex);
          _and(T_Lo, T_Lo, _1);
        }
      } break;
      }

      _mov(DestLo, T_Lo);

      Variable *T_Hi = makeReg(DestLo->getType());
      _mov(T_Hi, _0);
      _mov(DestHi, T_Hi);
    } else if (Src0->getType() == IceType_i1) {
      Variable *T = makeReg(DestTy);

      SafeBoolChain Safe = lowerInt1(T, Src0);
      if (Safe == SBC_No) {
        Operand *_1 = legalize(Ctx->getConstantInt1(1), Legal_Reg | Legal_Flex);
        _and(T, T, _1);
      }

      _mov(Dest, T);
    } else {
      // t1 = uxt src; dst = t1
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T = makeReg(DestTy);
      _uxt(T, Src0R);
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Trunc: {
    if (isVectorType(DestTy)) {
      auto *T = makeReg(DestTy);
      auto *Src0R = legalizeToReg(Src0);
      _mov(T, Src0R);
      _mov(Dest, T);
    } else {
      if (Src0->getType() == IceType_i64)
        Src0 = loOperand(Src0);
      Operand *Src0RF = legalize(Src0, Legal_Reg | Legal_Flex);
      // t1 = trunc Src0RF; Dest = t1
      Variable *T = makeReg(DestTy);
      _mov(T, Src0RF);
      if (DestTy == IceType_i1)
        _and(T, T, Ctx->getConstantInt1(1));
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Fptrunc:
  case InstCast::Fpext: {
    // fptrunc: dest.f32 = fptrunc src0.fp64
    // fpext: dest.f64 = fptrunc src0.fp32
    const bool IsTrunc = CastKind == InstCast::Fptrunc;
    assert(!isVectorType(DestTy));
    assert(DestTy == (IsTrunc ? IceType_f32 : IceType_f64));
    assert(Src0->getType() == (IsTrunc ? IceType_f64 : IceType_f32));
    Variable *Src0R = legalizeToReg(Src0);
    Variable *T = makeReg(DestTy);
    _vcvt(T, Src0R, IsTrunc ? InstARM32Vcvt::D2s : InstARM32Vcvt::S2d);
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptosi:
  case InstCast::Fptoui: {
    const bool DestIsSigned = CastKind == InstCast::Fptosi;
    Variable *Src0R = legalizeToReg(Src0);

    if (isVectorType(DestTy)) {
      assert(typeElementType(Src0->getType()) == IceType_f32);
      auto *T = makeReg(DestTy);
      _vcvt(T, Src0R,
            DestIsSigned ? InstARM32Vcvt::Vs2si : InstARM32Vcvt::Vs2ui);
      _mov(Dest, T);
      break;
    }

    const bool Src0IsF32 = isFloat32Asserting32Or64(Src0->getType());
    if (llvm::isa<Variable64On32>(Dest)) {
      llvm::report_fatal_error("fp-to-i64 should have been pre-lowered.");
    }
    // fptosi:
    //     t1.fp = vcvt src0.fp
    //     t2.i32 = vmov t1.fp
    //     dest.int = conv t2.i32     @ Truncates the result if needed.
    // fptoui:
    //     t1.fp = vcvt src0.fp
    //     t2.u32 = vmov t1.fp
    //     dest.uint = conv t2.u32    @ Truncates the result if needed.
    Variable *T_fp = makeReg(IceType_f32);
    const InstARM32Vcvt::VcvtVariant Conversion =
        Src0IsF32 ? (DestIsSigned ? InstARM32Vcvt::S2si : InstARM32Vcvt::S2ui)
                  : (DestIsSigned ? InstARM32Vcvt::D2si : InstARM32Vcvt::D2ui);
    _vcvt(T_fp, Src0R, Conversion);
    Variable *T = makeReg(IceType_i32);
    _mov(T, T_fp);
    if (DestTy != IceType_i32) {
      Variable *T_1 = makeReg(DestTy);
      lowerCast(InstCast::create(Func, InstCast::Trunc, T_1, T));
      T = T_1;
    }
    _mov(Dest, T);
    break;
  }
  case InstCast::Sitofp:
  case InstCast::Uitofp: {
    const bool SourceIsSigned = CastKind == InstCast::Sitofp;

    if (isVectorType(DestTy)) {
      assert(typeElementType(DestTy) == IceType_f32);
      auto *T = makeReg(DestTy);
      Variable *Src0R = legalizeToReg(Src0);
      _vcvt(T, Src0R,
            SourceIsSigned ? InstARM32Vcvt::Vsi2s : InstARM32Vcvt::Vui2s);
      _mov(Dest, T);
      break;
    }

    const bool DestIsF32 = isFloat32Asserting32Or64(DestTy);
    if (Src0->getType() == IceType_i64) {
      llvm::report_fatal_error("i64-to-fp should have been pre-lowered.");
    }
    // sitofp:
    //     t1.i32 = sext src.int    @ sign-extends src0 if needed.
    //     t2.fp32 = vmov t1.i32
    //     t3.fp = vcvt.{fp}.s32    @ fp is either f32 or f64
    // uitofp:
    //     t1.i32 = zext src.int    @ zero-extends src0 if needed.
    //     t2.fp32 = vmov t1.i32
    //     t3.fp = vcvt.{fp}.s32    @ fp is either f32 or f64
    if (Src0->getType() != IceType_i32) {
      Variable *Src0R_32 = makeReg(IceType_i32);
      lowerCast(InstCast::create(
          Func, SourceIsSigned ? InstCast::Sext : InstCast::Zext, Src0R_32,
          Src0));
      Src0 = Src0R_32;
    }
    Variable *Src0R = legalizeToReg(Src0);
    Variable *Src0R_f32 = makeReg(IceType_f32);
    _mov(Src0R_f32, Src0R);
    Src0R = Src0R_f32;
    Variable *T = makeReg(DestTy);
    const InstARM32Vcvt::VcvtVariant Conversion =
        DestIsF32
            ? (SourceIsSigned ? InstARM32Vcvt::Si2s : InstARM32Vcvt::Ui2s)
            : (SourceIsSigned ? InstARM32Vcvt::Si2d : InstARM32Vcvt::Ui2d);
    _vcvt(T, Src0R, Conversion);
    _mov(Dest, T);
    break;
  }
  case InstCast::Bitcast: {
    Operand *Src0 = Instr->getSrc(0);
    if (DestTy == Src0->getType()) {
      auto *Assign = InstAssign::create(Func, Dest, Src0);
      lowerAssign(Assign);
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
      Variable *T = makeReg(DestTy);
      _mov(T, Src0R);
      lowerAssign(InstAssign::create(Func, Dest, T));
      break;
    }
    case IceType_i64: {
      // t0, t1 <- src0
      // dest[31..0]  = t0
      // dest[63..32] = t1
      assert(Src0->getType() == IceType_f64);
      auto *T = llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
      T->initHiLo(Func);
      configureBitcastTemporary(T);
      Variable *Src0R = legalizeToReg(Src0);
      _mov(T, Src0R);
      Context.insert<InstFakeUse>(T->getHi());
      Context.insert<InstFakeUse>(T->getLo());
      lowerAssign(InstAssign::create(Func, Dest, T));
      break;
    }
    case IceType_f64: {
      // T0 <- lo(src)
      // T1 <- hi(src)
      // vmov T2, T0, T1
      // Dest <- T2
      assert(Src0->getType() == IceType_i64);
      Variable *T = makeReg(DestTy);
      auto *Src64 = llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
      Src64->initHiLo(Func);
      configureBitcastTemporary(Src64);
      lowerAssign(InstAssign::create(Func, Src64, Src0));
      _mov(T, Src64);
      lowerAssign(InstAssign::create(Func, Dest, T));
      break;
    }
    case IceType_v8i1:
      assert(Src0->getType() == IceType_i8);
      llvm::report_fatal_error(
          "v8i1 to i8 conversion should have been prelowered.");
      break;
    case IceType_v16i1:
      assert(Src0->getType() == IceType_i16);
      llvm::report_fatal_error(
          "v16i1 to i16 conversion should have been prelowered.");
      break;
    case IceType_v4i1:
    case IceType_v8i16:
    case IceType_v16i8:
    case IceType_v4f32:
    case IceType_v4i32: {
      assert(typeWidthInBytes(DestTy) == typeWidthInBytes(Src0->getType()));
      assert(isVectorType(DestTy) == isVectorType(Src0->getType()));
      Variable *T = makeReg(DestTy);
      _mov(T, Src0);
      _mov(Dest, T);
      break;
    }
    }
    break;
  }
  }
}

void TargetARM32::lowerExtractElement(const InstExtractElement *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = Dest->getType();

  Variable *Src0 = legalizeToReg(Instr->getSrc(0));
  Operand *Src1 = Instr->getSrc(1);

  if (const auto *Imm = llvm::dyn_cast<ConstantInteger32>(Src1)) {
    const uint32_t Index = Imm->getValue();
    Variable *T = makeReg(DestTy);
    Variable *TSrc0 = makeReg(Src0->getType());

    if (isFloatingType(DestTy)) {
      // We need to make sure the source is in a suitable register.
      TSrc0->setRegClass(RegARM32::RCARM32_QtoS);
    }

    _mov(TSrc0, Src0);
    _extractelement(T, TSrc0, Index);
    _mov(Dest, T);
    return;
  }
  assert(false && "extractelement requires a constant index");
}

namespace {
// Validates FCMPARM32_TABLE's declaration w.r.t. InstFcmp::FCondition ordering
// (and naming).
enum {
#define X(val, CC0, CC1, CC0_V, CC1_V, INV_V, NEG_V) _fcmp_ll_##val,
  FCMPARM32_TABLE
#undef X
      _fcmp_ll_NUM
};

enum {
#define X(tag, str) _fcmp_hl_##tag = InstFcmp::tag,
  ICEINSTFCMP_TABLE
#undef X
      _fcmp_hl_NUM
};

static_assert((uint32_t)_fcmp_hl_NUM == (uint32_t)_fcmp_ll_NUM,
              "Inconsistency between high-level and low-level fcmp tags.");
#define X(tag, str)                                                            \
  static_assert(                                                               \
      (uint32_t)_fcmp_hl_##tag == (uint32_t)_fcmp_ll_##tag,                    \
      "Inconsistency between high-level and low-level fcmp tag " #tag);
ICEINSTFCMP_TABLE
#undef X

struct {
  CondARM32::Cond CC0;
  CondARM32::Cond CC1;
} TableFcmp[] = {
#define X(val, CC0, CC1, CC0_V, CC1_V, INV_V, NEG_V)                           \
  {CondARM32::CC0, CondARM32::CC1},
    FCMPARM32_TABLE
#undef X
};

bool isFloatingPointZero(const Operand *Src) {
  if (const auto *F32 = llvm::dyn_cast<const ConstantFloat>(Src)) {
    return Utils::isPositiveZero(F32->getValue());
  }

  if (const auto *F64 = llvm::dyn_cast<const ConstantDouble>(Src)) {
    return Utils::isPositiveZero(F64->getValue());
  }

  return false;
}
} // end of anonymous namespace

TargetARM32::CondWhenTrue TargetARM32::lowerFcmpCond(const InstFcmp *Instr) {
  InstFcmp::FCond Condition = Instr->getCondition();
  switch (Condition) {
  case InstFcmp::False:
    return CondWhenTrue(CondARM32::kNone);
  case InstFcmp::True:
    return CondWhenTrue(CondARM32::AL);
    break;
  default: {
    Variable *Src0R = legalizeToReg(Instr->getSrc(0));
    Operand *Src1 = Instr->getSrc(1);
    if (isFloatingPointZero(Src1)) {
      _vcmp(Src0R, OperandARM32FlexFpZero::create(Func, Src0R->getType()));
    } else {
      _vcmp(Src0R, legalizeToReg(Src1));
    }
    _vmrs();
    assert(Condition < llvm::array_lengthof(TableFcmp));
    return CondWhenTrue(TableFcmp[Condition].CC0, TableFcmp[Condition].CC1);
  }
  }
}

void TargetARM32::lowerFcmp(const InstFcmp *Instr) {
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();

  if (isVectorType(DestTy)) {
    if (Instr->getCondition() == InstFcmp::False) {
      constexpr Type SafeTypeForMovingConstant = IceType_v4i32;
      auto *T = makeReg(SafeTypeForMovingConstant);
      _mov(T, llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(0)));
      _mov(Dest, T);
      return;
    }

    if (Instr->getCondition() == InstFcmp::True) {
      constexpr Type SafeTypeForMovingConstant = IceType_v4i32;
      auto *T = makeReg(SafeTypeForMovingConstant);
      _mov(T, llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(1)));
      _mov(Dest, T);
      return;
    }

    Variable *T0;
    Variable *T1;
    bool Negate = false;
    auto *Src0 = legalizeToReg(Instr->getSrc(0));
    auto *Src1 = legalizeToReg(Instr->getSrc(1));

    switch (Instr->getCondition()) {
    default:
      llvm::report_fatal_error("Unhandled fp comparison.");
#define _Vcnone(Tptr, S0, S1)                                                  \
  do {                                                                         \
    *(Tptr) = nullptr;                                                         \
  } while (0)
#define _Vceq(Tptr, S0, S1)                                                    \
  do {                                                                         \
    *(Tptr) = makeReg(DestTy);                                                 \
    _vceq(*(Tptr), S0, S1);                                                    \
  } while (0)
#define _Vcge(Tptr, S0, S1)                                                    \
  do {                                                                         \
    *(Tptr) = makeReg(DestTy);                                                 \
    _vcge(*(Tptr), S0, S1)->setSignType(InstARM32::FS_Signed);                 \
  } while (0)
#define _Vcgt(Tptr, S0, S1)                                                    \
  do {                                                                         \
    *(Tptr) = makeReg(DestTy);                                                 \
    _vcgt(*(Tptr), S0, S1)->setSignType(InstARM32::FS_Signed);                 \
  } while (0)
#define X(val, CC0, CC1, CC0_V, CC1_V, INV_V, NEG_V)                           \
  case InstFcmp::val: {                                                        \
    _Vc##CC0_V(&T0, (INV_V) ? Src1 : Src0, (INV_V) ? Src0 : Src1);             \
    _Vc##CC1_V(&T1, (INV_V) ? Src0 : Src1, (INV_V) ? Src1 : Src0);             \
    Negate = NEG_V;                                                            \
  } break;
      FCMPARM32_TABLE
#undef X
#undef _Vcgt
#undef _Vcge
#undef _Vceq
#undef _Vcnone
    }
    assert(T0 != nullptr);
    Variable *T = T0;
    if (T1 != nullptr) {
      T = makeReg(DestTy);
      _vorr(T, T0, T1);
    }

    if (Negate) {
      auto *TNeg = makeReg(DestTy);
      _vmvn(TNeg, T);
      T = TNeg;
    }

    _mov(Dest, T);
    return;
  }

  Variable *T = makeReg(IceType_i1);
  Operand *_1 = legalize(Ctx->getConstantInt32(1), Legal_Reg | Legal_Flex);
  Operand *_0 =
      legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);

  CondWhenTrue Cond = lowerFcmpCond(Instr);

  bool RedefineT = false;
  if (Cond.WhenTrue0 != CondARM32::AL) {
    _mov(T, _0);
    RedefineT = true;
  }

  if (Cond.WhenTrue0 == CondARM32::kNone) {
    _mov(Dest, T);
    return;
  }

  if (RedefineT) {
    _mov_redefined(T, _1, Cond.WhenTrue0);
  } else {
    _mov(T, _1, Cond.WhenTrue0);
  }

  if (Cond.WhenTrue1 != CondARM32::kNone) {
    _mov_redefined(T, _1, Cond.WhenTrue1);
  }

  _mov(Dest, T);
}

TargetARM32::CondWhenTrue
TargetARM32::lowerInt64IcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                                Operand *Src1) {
  assert(Condition < llvm::array_lengthof(TableIcmp64));

  Int32Operands SrcsLo(loOperand(Src0), loOperand(Src1));
  Int32Operands SrcsHi(hiOperand(Src0), hiOperand(Src1));
  assert(SrcsLo.hasConstOperand() == SrcsHi.hasConstOperand());
  assert(SrcsLo.swappedOperands() == SrcsHi.swappedOperands());

  if (SrcsLo.hasConstOperand()) {
    const uint32_t ValueLo = SrcsLo.getConstantValue();
    const uint32_t ValueHi = SrcsHi.getConstantValue();
    const uint64_t Value = (static_cast<uint64_t>(ValueHi) << 32) | ValueLo;
    if ((Condition == InstIcmp::Eq || Condition == InstIcmp::Ne) &&
        Value == 0) {
      Variable *T = makeReg(IceType_i32);
      Variable *Src0LoR = SrcsLo.src0R(this);
      Variable *Src0HiR = SrcsHi.src0R(this);
      _orrs(T, Src0LoR, Src0HiR);
      Context.insert<InstFakeUse>(T);
      return CondWhenTrue(TableIcmp64[Condition].C1);
    }

    Variable *Src0RLo = SrcsLo.src0R(this);
    Variable *Src0RHi = SrcsHi.src0R(this);
    Operand *Src1RFLo = SrcsLo.src1RF(this);
    Operand *Src1RFHi = ValueLo == ValueHi ? Src1RFLo : SrcsHi.src1RF(this);

    const bool UseRsb =
        TableIcmp64[Condition].Swapped != SrcsLo.swappedOperands();

    if (UseRsb) {
      if (TableIcmp64[Condition].IsSigned) {
        Variable *T = makeReg(IceType_i32);
        _rsbs(T, Src0RLo, Src1RFLo);
        Context.insert<InstFakeUse>(T);

        T = makeReg(IceType_i32);
        _rscs(T, Src0RHi, Src1RFHi);
        // We need to add a FakeUse here because liveness gets mad at us (Def
        // without Use.) Note that flag-setting instructions are considered to
        // have side effects and, therefore, are not DCE'ed.
        Context.insert<InstFakeUse>(T);
      } else {
        Variable *T = makeReg(IceType_i32);
        _rsbs(T, Src0RHi, Src1RFHi);
        Context.insert<InstFakeUse>(T);

        T = makeReg(IceType_i32);
        _rsbs(T, Src0RLo, Src1RFLo, CondARM32::EQ);
        Context.insert<InstFakeUse>(T);
      }
    } else {
      if (TableIcmp64[Condition].IsSigned) {
        _cmp(Src0RLo, Src1RFLo);
        Variable *T = makeReg(IceType_i32);
        _sbcs(T, Src0RHi, Src1RFHi);
        Context.insert<InstFakeUse>(T);
      } else {
        _cmp(Src0RHi, Src1RFHi);
        _cmp(Src0RLo, Src1RFLo, CondARM32::EQ);
      }
    }

    return CondWhenTrue(TableIcmp64[Condition].C1);
  }

  Variable *Src0RLo, *Src0RHi;
  Operand *Src1RFLo, *Src1RFHi;
  if (TableIcmp64[Condition].Swapped) {
    Src0RLo = legalizeToReg(loOperand(Src1));
    Src0RHi = legalizeToReg(hiOperand(Src1));
    Src1RFLo = legalizeToReg(loOperand(Src0));
    Src1RFHi = legalizeToReg(hiOperand(Src0));
  } else {
    Src0RLo = legalizeToReg(loOperand(Src0));
    Src0RHi = legalizeToReg(hiOperand(Src0));
    Src1RFLo = legalizeToReg(loOperand(Src1));
    Src1RFHi = legalizeToReg(hiOperand(Src1));
  }

  // a=icmp cond, b, c ==>
  // GCC does:
  //   cmp      b.hi, c.hi     or  cmp      b.lo, c.lo
  //   cmp.eq   b.lo, c.lo         sbcs t1, b.hi, c.hi
  //   mov.<C1> t, #1              mov.<C1> t, #1
  //   mov.<C2> t, #0              mov.<C2> t, #0
  //   mov      a, t               mov      a, t
  // where the "cmp.eq b.lo, c.lo" is used for unsigned and "sbcs t1, hi, hi"
  // is used for signed compares. In some cases, b and c need to be swapped as
  // well.
  //
  // LLVM does:
  // for EQ and NE:
  //   eor  t1, b.hi, c.hi
  //   eor  t2, b.lo, c.hi
  //   orrs t, t1, t2
  //   mov.<C> t, #1
  //   mov  a, t
  //
  // that's nice in that it's just as short but has fewer dependencies for
  // better ILP at the cost of more registers.
  //
  // Otherwise for signed/unsigned <, <=, etc. LLVM uses a sequence with two
  // unconditional mov #0, two cmps, two conditional mov #1, and one
  // conditional reg mov. That has few dependencies for good ILP, but is a
  // longer sequence.
  //
  // So, we are going with the GCC version since it's usually better (except
  // perhaps for eq/ne). We could revisit special-casing eq/ne later.
  if (TableIcmp64[Condition].IsSigned) {
    Variable *ScratchReg = makeReg(IceType_i32);
    _cmp(Src0RLo, Src1RFLo);
    _sbcs(ScratchReg, Src0RHi, Src1RFHi);
    // ScratchReg isn't going to be used, but we need the side-effect of
    // setting flags from this operation.
    Context.insert<InstFakeUse>(ScratchReg);
  } else {
    _cmp(Src0RHi, Src1RFHi);
    _cmp(Src0RLo, Src1RFLo, CondARM32::EQ);
  }
  return CondWhenTrue(TableIcmp64[Condition].C1);
}

TargetARM32::CondWhenTrue
TargetARM32::lowerInt32IcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                                Operand *Src1) {
  Int32Operands Srcs(Src0, Src1);
  if (!Srcs.hasConstOperand()) {

    Variable *Src0R = Srcs.src0R(this);
    Operand *Src1RF = Srcs.src1RF(this);
    _cmp(Src0R, Src1RF);
    return CondWhenTrue(getIcmp32Mapping(Condition));
  }

  Variable *Src0R = Srcs.src0R(this);
  const int32_t Value = Srcs.getConstantValue();
  if ((Condition == InstIcmp::Eq || Condition == InstIcmp::Ne) && Value == 0) {
    _tst(Src0R, Src0R);
    return CondWhenTrue(getIcmp32Mapping(Condition));
  }

  if (!Srcs.swappedOperands() && !Srcs.immediateIsFlexEncodable() &&
      Srcs.negatedImmediateIsFlexEncodable()) {
    Operand *Src1F = Srcs.negatedSrc1F(this);
    _cmn(Src0R, Src1F);
    return CondWhenTrue(getIcmp32Mapping(Condition));
  }

  Operand *Src1RF = Srcs.src1RF(this);
  if (!Srcs.swappedOperands()) {
    _cmp(Src0R, Src1RF);
  } else {
    Variable *T = makeReg(IceType_i32);
    _rsbs(T, Src0R, Src1RF);
    Context.insert<InstFakeUse>(T);
  }
  return CondWhenTrue(getIcmp32Mapping(Condition));
}

TargetARM32::CondWhenTrue
TargetARM32::lowerInt8AndInt16IcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                                       Operand *Src1) {
  Int32Operands Srcs(Src0, Src1);
  const int32_t ShAmt = 32 - getScalarIntBitWidth(Src0->getType());
  assert(ShAmt >= 0);

  if (!Srcs.hasConstOperand()) {
    Variable *Src0R = makeReg(IceType_i32);
    Operand *ShAmtImm = shAmtImm(ShAmt);
    _lsl(Src0R, legalizeToReg(Src0), ShAmtImm);

    Variable *Src1R = legalizeToReg(Src1);
    auto *Src1F = OperandARM32FlexReg::create(Func, IceType_i32, Src1R,
                                              OperandARM32::LSL, ShAmtImm);
    _cmp(Src0R, Src1F);
    return CondWhenTrue(getIcmp32Mapping(Condition));
  }

  const int32_t Value = Srcs.getConstantValue();
  if ((Condition == InstIcmp::Eq || Condition == InstIcmp::Ne) && Value == 0) {
    Operand *ShAmtImm = shAmtImm(ShAmt);
    Variable *T = makeReg(IceType_i32);
    _lsls(T, Srcs.src0R(this), ShAmtImm);
    Context.insert<InstFakeUse>(T);
    return CondWhenTrue(getIcmp32Mapping(Condition));
  }

  Variable *ConstR = makeReg(IceType_i32);
  _mov(ConstR,
       legalize(Ctx->getConstantInt32(Value << ShAmt), Legal_Reg | Legal_Flex));
  Operand *NonConstF = OperandARM32FlexReg::create(
      Func, IceType_i32, Srcs.src0R(this), OperandARM32::LSL,
      Ctx->getConstantInt32(ShAmt));

  if (Srcs.swappedOperands()) {
    _cmp(ConstR, NonConstF);
  } else {
    Variable *T = makeReg(IceType_i32);
    _rsbs(T, ConstR, NonConstF);
    Context.insert<InstFakeUse>(T);
  }
  return CondWhenTrue(getIcmp32Mapping(Condition));
}

TargetARM32::CondWhenTrue TargetARM32::lowerIcmpCond(const InstIcmp *Instr) {
  return lowerIcmpCond(Instr->getCondition(), Instr->getSrc(0),
                       Instr->getSrc(1));
}

TargetARM32::CondWhenTrue TargetARM32::lowerIcmpCond(InstIcmp::ICond Condition,
                                                     Operand *Src0,
                                                     Operand *Src1) {
  Src0 = legalizeUndef(Src0);
  Src1 = legalizeUndef(Src1);

  // a=icmp cond b, c ==>
  // GCC does:
  //   <u/s>xtb tb, b
  //   <u/s>xtb tc, c
  //   cmp      tb, tc
  //   mov.C1   t, #0
  //   mov.C2   t, #1
  //   mov      a, t
  // where the unsigned/sign extension is not needed for 32-bit. They also have
  // special cases for EQ and NE. E.g., for NE:
  //   <extend to tb, tc>
  //   subs     t, tb, tc
  //   movne    t, #1
  //   mov      a, t
  //
  // LLVM does:
  //   lsl     tb, b, #<N>
  //   mov     t, #0
  //   cmp     tb, c, lsl #<N>
  //   mov.<C> t, #1
  //   mov     a, t
  //
  // the left shift is by 0, 16, or 24, which allows the comparison to focus on
  // the digits that actually matter (for 16-bit or 8-bit signed/unsigned). For
  // the unsigned case, for some reason it does similar to GCC and does a uxtb
  // first. It's not clear to me why that special-casing is needed.
  //
  // We'll go with the LLVM way for now, since it's shorter and has just as few
  // dependencies.
  switch (Src0->getType()) {
  default:
    llvm::report_fatal_error("Unhandled type in lowerIcmpCond");
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
    return lowerInt8AndInt16IcmpCond(Condition, Src0, Src1);
  case IceType_i32:
    return lowerInt32IcmpCond(Condition, Src0, Src1);
  case IceType_i64:
    return lowerInt64IcmpCond(Condition, Src0, Src1);
  }
}

void TargetARM32::lowerIcmp(const InstIcmp *Instr) {
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();

  if (isVectorType(DestTy)) {
    auto *T = makeReg(DestTy);
    auto *Src0 = legalizeToReg(Instr->getSrc(0));
    auto *Src1 = legalizeToReg(Instr->getSrc(1));
    const Type SrcTy = Src0->getType();

    bool NeedsShl = false;
    Type NewTypeAfterShl;
    SizeT ShAmt;
    switch (SrcTy) {
    default:
      break;
    case IceType_v16i1:
      NeedsShl = true;
      NewTypeAfterShl = IceType_v16i8;
      ShAmt = 7;
      break;
    case IceType_v8i1:
      NeedsShl = true;
      NewTypeAfterShl = IceType_v8i16;
      ShAmt = 15;
      break;
    case IceType_v4i1:
      NeedsShl = true;
      NewTypeAfterShl = IceType_v4i32;
      ShAmt = 31;
      break;
    }

    if (NeedsShl) {
      auto *Imm = llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(ShAmt));
      auto *Src0T = makeReg(NewTypeAfterShl);
      auto *Src0Shl = makeReg(NewTypeAfterShl);
      _mov(Src0T, Src0);
      _vshl(Src0Shl, Src0T, Imm);
      Src0 = Src0Shl;

      auto *Src1T = makeReg(NewTypeAfterShl);
      auto *Src1Shl = makeReg(NewTypeAfterShl);
      _mov(Src1T, Src1);
      _vshl(Src1Shl, Src1T, Imm);
      Src1 = Src1Shl;
    }

    switch (Instr->getCondition()) {
    default:
      llvm::report_fatal_error("Unhandled integer comparison.");
#define _Vceq(T, S0, S1, Signed) _vceq(T, S0, S1)
#define _Vcge(T, S0, S1, Signed)                                               \
  _vcge(T, S0, S1)->setSignType(Signed ? InstARM32::FS_Signed                  \
                                       : InstARM32::FS_Unsigned)
#define _Vcgt(T, S0, S1, Signed)                                               \
  _vcgt(T, S0, S1)->setSignType(Signed ? InstARM32::FS_Signed                  \
                                       : InstARM32::FS_Unsigned)
#define X(val, is_signed, swapped64, C_32, C1_64, C2_64, C_V, INV_V, NEG_V)    \
  case InstIcmp::val: {                                                        \
    _Vc##C_V(T, (INV_V) ? Src1 : Src0, (INV_V) ? Src0 : Src1, is_signed);      \
    if (NEG_V) {                                                               \
      auto *TInv = makeReg(DestTy);                                            \
      _vmvn(TInv, T);                                                          \
      T = TInv;                                                                \
    }                                                                          \
  } break;
      ICMPARM32_TABLE
#undef X
#undef _Vcgt
#undef _Vcge
#undef _Vceq
    }
    _mov(Dest, T);
    return;
  }

  Operand *_0 =
      legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
  Operand *_1 = legalize(Ctx->getConstantInt32(1), Legal_Reg | Legal_Flex);
  Variable *T = makeReg(IceType_i1);

  _mov(T, _0);
  CondWhenTrue Cond = lowerIcmpCond(Instr);
  _mov_redefined(T, _1, Cond.WhenTrue0);
  _mov(Dest, T);

  assert(Cond.WhenTrue1 == CondARM32::kNone);

  return;
}

void TargetARM32::lowerInsertElement(const InstInsertElement *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = Dest->getType();

  Variable *Src0 = legalizeToReg(Instr->getSrc(0));
  Variable *Src1 = legalizeToReg(Instr->getSrc(1));
  Operand *Src2 = Instr->getSrc(2);

  if (const auto *Imm = llvm::dyn_cast<ConstantInteger32>(Src2)) {
    const uint32_t Index = Imm->getValue();
    Variable *T = makeReg(DestTy);

    if (isFloatingType(DestTy)) {
      T->setRegClass(RegARM32::RCARM32_QtoS);
    }

    _mov(T, Src0);
    _insertelement(T, Src1, Index);
    _set_dest_redefined();
    _mov(Dest, T);
    return;
  }
  assert(false && "insertelement requires a constant index");
}

namespace {
inline uint64_t getConstantMemoryOrder(Operand *Opnd) {
  if (auto *Integer = llvm::dyn_cast<ConstantInteger32>(Opnd))
    return Integer->getValue();
  return Intrinsics::MemoryOrderInvalid;
}
} // end of anonymous namespace

void TargetARM32::lowerLoadLinkedStoreExclusive(
    Type Ty, Operand *Addr, std::function<Variable *(Variable *)> Operation,
    CondARM32::Cond Cond) {

  auto *Retry = Context.insert<InstARM32Label>(this);

  { // scoping for loop highlighting.
    Variable *Success = makeReg(IceType_i32);
    Variable *Tmp = (Ty == IceType_i64) ? makeI64RegPair() : makeReg(Ty);
    auto *_0 = Ctx->getConstantZero(IceType_i32);

    Context.insert<InstFakeDef>(Tmp);
    Context.insert<InstFakeUse>(Tmp);
    Variable *AddrR = legalizeToReg(Addr);
    _ldrex(Tmp, formMemoryOperand(AddrR, Ty))->setDestRedefined();
    auto *StoreValue = Operation(Tmp);
    assert(StoreValue->mustHaveReg());
    // strex requires Dest to be a register other than Value or Addr. This
    // restriction is cleanly represented by adding an "early" definition of
    // Dest (or a latter use of all the sources.)
    Context.insert<InstFakeDef>(Success);
    if (Cond != CondARM32::AL) {
      _mov_redefined(Success, legalize(_0, Legal_Reg | Legal_Flex),
                     InstARM32::getOppositeCondition(Cond));
    }
    _strex(Success, StoreValue, formMemoryOperand(AddrR, Ty), Cond)
        ->setDestRedefined();
    _cmp(Success, _0);
  }

  _br(Retry, CondARM32::NE);
}

namespace {
InstArithmetic *createArithInst(Cfg *Func, uint32_t Operation, Variable *Dest,
                                Variable *Src0, Operand *Src1) {
  InstArithmetic::OpKind Oper;
  switch (Operation) {
  default:
    llvm::report_fatal_error("Unknown AtomicRMW operation");
  case Intrinsics::AtomicExchange:
    llvm::report_fatal_error("Can't handle Atomic xchg operation");
  case Intrinsics::AtomicAdd:
    Oper = InstArithmetic::Add;
    break;
  case Intrinsics::AtomicAnd:
    Oper = InstArithmetic::And;
    break;
  case Intrinsics::AtomicSub:
    Oper = InstArithmetic::Sub;
    break;
  case Intrinsics::AtomicOr:
    Oper = InstArithmetic::Or;
    break;
  case Intrinsics::AtomicXor:
    Oper = InstArithmetic::Xor;
    break;
  }
  return InstArithmetic::create(Func, Oper, Dest, Src0, Src1);
}
} // end of anonymous namespace

void TargetARM32::lowerAtomicRMW(Variable *Dest, uint32_t Operation,
                                 Operand *Addr, Operand *Val) {
  // retry:
  //     ldrex tmp, [addr]
  //     mov contents, tmp
  //     op result, contents, Val
  //     strex success, result, [addr]
  //     cmp success, 0
  //     jne retry
  //     fake-use(addr, operand)  @ prevents undesirable clobbering.
  //     mov dest, contents
  auto DestTy = Dest->getType();

  if (DestTy == IceType_i64) {
    lowerInt64AtomicRMW(Dest, Operation, Addr, Val);
    return;
  }

  Operand *ValRF = nullptr;
  if (llvm::isa<ConstantInteger32>(Val)) {
    ValRF = Val;
  } else {
    ValRF = legalizeToReg(Val);
  }
  auto *ContentsR = makeReg(DestTy);
  auto *ResultR = makeReg(DestTy);

  _dmb();
  lowerLoadLinkedStoreExclusive(
      DestTy, Addr,
      [this, Operation, ResultR, ContentsR, ValRF](Variable *Tmp) {
        lowerAssign(InstAssign::create(Func, ContentsR, Tmp));
        if (Operation == Intrinsics::AtomicExchange) {
          lowerAssign(InstAssign::create(Func, ResultR, ValRF));
        } else {
          lowerArithmetic(
              createArithInst(Func, Operation, ResultR, ContentsR, ValRF));
        }
        return ResultR;
      });
  _dmb();
  if (auto *ValR = llvm::dyn_cast<Variable>(ValRF)) {
    Context.insert<InstFakeUse>(ValR);
  }
  // Can't dce ContentsR.
  Context.insert<InstFakeUse>(ContentsR);
  lowerAssign(InstAssign::create(Func, Dest, ContentsR));
}

void TargetARM32::lowerInt64AtomicRMW(Variable *Dest, uint32_t Operation,
                                      Operand *Addr, Operand *Val) {
  assert(Dest->getType() == IceType_i64);

  auto *ResultR = makeI64RegPair();

  Context.insert<InstFakeDef>(ResultR);

  Operand *ValRF = nullptr;
  if (llvm::dyn_cast<ConstantInteger64>(Val)) {
    ValRF = Val;
  } else {
    auto *ValR64 = llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
    ValR64->initHiLo(Func);
    ValR64->setMustNotHaveReg();
    ValR64->getLo()->setMustHaveReg();
    ValR64->getHi()->setMustHaveReg();
    lowerAssign(InstAssign::create(Func, ValR64, Val));
    ValRF = ValR64;
  }

  auto *ContentsR = llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
  ContentsR->initHiLo(Func);
  ContentsR->setMustNotHaveReg();
  ContentsR->getLo()->setMustHaveReg();
  ContentsR->getHi()->setMustHaveReg();

  _dmb();
  lowerLoadLinkedStoreExclusive(
      IceType_i64, Addr,
      [this, Operation, ResultR, ContentsR, ValRF](Variable *Tmp) {
        lowerAssign(InstAssign::create(Func, ContentsR, Tmp));
        Context.insert<InstFakeUse>(Tmp);
        if (Operation == Intrinsics::AtomicExchange) {
          lowerAssign(InstAssign::create(Func, ResultR, ValRF));
        } else {
          lowerArithmetic(
              createArithInst(Func, Operation, ResultR, ContentsR, ValRF));
        }
        Context.insert<InstFakeUse>(ResultR->getHi());
        Context.insert<InstFakeDef>(ResultR, ResultR->getLo())
            ->setDestRedefined();
        return ResultR;
      });
  _dmb();
  if (auto *ValR64 = llvm::dyn_cast<Variable64On32>(ValRF)) {
    Context.insert<InstFakeUse>(ValR64->getLo());
    Context.insert<InstFakeUse>(ValR64->getHi());
  }
  lowerAssign(InstAssign::create(Func, Dest, ContentsR));
}

void TargetARM32::postambleCtpop64(const InstCall *Instr) {
  Operand *Arg0 = Instr->getArg(0);
  if (isInt32Asserting32Or64(Arg0->getType())) {
    return;
  }
  // The popcount helpers always return 32-bit values, while the intrinsic's
  // signature matches some 64-bit platform's native instructions and expect to
  // fill a 64-bit reg. Thus, clear the upper bits of the dest just in case the
  // user doesn't do that in the IR or doesn't toss the bits via truncate.
  auto *DestHi = llvm::cast<Variable>(hiOperand(Instr->getDest()));
  Variable *T = makeReg(IceType_i32);
  Operand *_0 =
      legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
  _mov(T, _0);
  _mov(DestHi, T);
}

void TargetARM32::lowerIntrinsic(const InstIntrinsic *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = (Dest != nullptr) ? Dest->getType() : IceType_void;
  Intrinsics::IntrinsicID ID = Instr->getIntrinsicID();
  switch (ID) {
  case Intrinsics::AtomicFence:
  case Intrinsics::AtomicFenceAll:
    assert(Dest == nullptr);
    _dmb();
    return;
  case Intrinsics::AtomicIsLockFree: {
    Operand *ByteSize = Instr->getArg(0);
    auto *CI = llvm::dyn_cast<ConstantInteger32>(ByteSize);
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
    case 8:
      Result = LockFree;
      break;
    }
    _mov(Dest, legalizeToReg(Ctx->getConstantInt32(Result)));
    return;
  }
  case Intrinsics::AtomicLoad: {
    assert(isScalarIntegerType(DestTy));
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(1)))) {
      Func->setError("Unexpected memory ordering for AtomicLoad");
      return;
    }
    Variable *T;

    if (DestTy == IceType_i64) {
      // ldrex is the only arm instruction that is guaranteed to load a 64-bit
      // integer atomically. Everything else works with a regular ldr.
      T = makeI64RegPair();
      _ldrex(T, formMemoryOperand(Instr->getArg(0), IceType_i64));
    } else {
      T = makeReg(DestTy);
      _ldr(T, formMemoryOperand(Instr->getArg(0), DestTy));
    }
    _dmb();
    lowerAssign(InstAssign::create(Func, Dest, T));
    // Adding a fake-use T to ensure the atomic load is not removed if Dest is
    // unused.
    Context.insert<InstFakeUse>(T);
    return;
  }
  case Intrinsics::AtomicStore: {
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(2)))) {
      Func->setError("Unexpected memory ordering for AtomicStore");
      return;
    }

    auto *Value = Instr->getArg(0);
    if (Value->getType() == IceType_i64) {
      auto *ValueR = makeI64RegPair();
      Context.insert<InstFakeDef>(ValueR);
      lowerAssign(InstAssign::create(Func, ValueR, Value));
      _dmb();
      lowerLoadLinkedStoreExclusive(
          IceType_i64, Instr->getArg(1), [this, ValueR](Variable *Tmp) {
            // The following fake-use prevents the ldrex instruction from being
            // dead code eliminated.
            Context.insert<InstFakeUse>(llvm::cast<Variable>(loOperand(Tmp)));
            Context.insert<InstFakeUse>(llvm::cast<Variable>(hiOperand(Tmp)));
            Context.insert<InstFakeUse>(Tmp);
            return ValueR;
          });
      Context.insert<InstFakeUse>(ValueR);
      _dmb();
      return;
    }

    auto *ValueR = legalizeToReg(Instr->getArg(0));
    const auto ValueTy = ValueR->getType();
    assert(isScalarIntegerType(ValueTy));
    auto *Addr = legalizeToReg(Instr->getArg(1));

    // non-64-bit stores are atomically as long as the address is aligned. This
    // is PNaCl, so addresses are aligned.
    _dmb();
    _str(ValueR, formMemoryOperand(Addr, ValueTy));
    _dmb();
    return;
  }
  case Intrinsics::AtomicCmpxchg: {
    // retry:
    //     ldrex tmp, [addr]
    //     cmp tmp, expected
    //     mov expected, tmp
    //     strexeq success, new, [addr]
    //     cmpeq success, #0
    //     bne retry
    //     mov dest, expected
    assert(isScalarIntegerType(DestTy));
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)),
            getConstantMemoryOrder(Instr->getArg(4)))) {
      Func->setError("Unexpected memory ordering for AtomicCmpxchg");
      return;
    }

    if (DestTy == IceType_i64) {
      Variable *LoadedValue = nullptr;

      auto *New = makeI64RegPair();
      Context.insert<InstFakeDef>(New);
      lowerAssign(InstAssign::create(Func, New, Instr->getArg(2)));

      auto *Expected = makeI64RegPair();
      Context.insert<InstFakeDef>(Expected);
      lowerAssign(InstAssign::create(Func, Expected, Instr->getArg(1)));

      _dmb();
      lowerLoadLinkedStoreExclusive(
          DestTy, Instr->getArg(0),
          [this, Expected, New, &LoadedValue](Variable *Tmp) {
            auto *ExpectedLoR = llvm::cast<Variable>(loOperand(Expected));
            auto *ExpectedHiR = llvm::cast<Variable>(hiOperand(Expected));
            auto *TmpLoR = llvm::cast<Variable>(loOperand(Tmp));
            auto *TmpHiR = llvm::cast<Variable>(hiOperand(Tmp));
            _cmp(TmpLoR, ExpectedLoR);
            _cmp(TmpHiR, ExpectedHiR, CondARM32::EQ);
            LoadedValue = Tmp;
            return New;
          },
          CondARM32::EQ);
      _dmb();

      Context.insert<InstFakeUse>(LoadedValue);
      lowerAssign(InstAssign::create(Func, Dest, LoadedValue));
      // The fake-use Expected prevents the assignments to Expected (above)
      // from being removed if Dest is not used.
      Context.insert<InstFakeUse>(Expected);
      // New needs to be alive here, or its live range will end in the
      // strex instruction.
      Context.insert<InstFakeUse>(New);
      return;
    }

    auto *New = legalizeToReg(Instr->getArg(2));
    auto *Expected = legalizeToReg(Instr->getArg(1));
    Variable *LoadedValue = nullptr;

    _dmb();
    lowerLoadLinkedStoreExclusive(
        DestTy, Instr->getArg(0),
        [this, Expected, New, &LoadedValue](Variable *Tmp) {
          lowerIcmpCond(InstIcmp::Eq, Tmp, Expected);
          LoadedValue = Tmp;
          return New;
        },
        CondARM32::EQ);
    _dmb();

    lowerAssign(InstAssign::create(Func, Dest, LoadedValue));
    Context.insert<InstFakeUse>(Expected);
    Context.insert<InstFakeUse>(New);
    return;
  }
  case Intrinsics::AtomicRMW: {
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)))) {
      Func->setError("Unexpected memory ordering for AtomicRMW");
      return;
    }
    lowerAtomicRMW(
        Dest,
        static_cast<uint32_t>(
            llvm::cast<ConstantInteger32>(Instr->getArg(0))->getValue()),
        Instr->getArg(1), Instr->getArg(2));
    return;
  }
  case Intrinsics::Bswap: {
    Operand *Val = Instr->getArg(0);
    Type Ty = Val->getType();
    if (Ty == IceType_i64) {
      Val = legalizeUndef(Val);
      Variable *Val_Lo = legalizeToReg(loOperand(Val));
      Variable *Val_Hi = legalizeToReg(hiOperand(Val));
      Variable *T_Lo = makeReg(IceType_i32);
      Variable *T_Hi = makeReg(IceType_i32);
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      _rev(T_Lo, Val_Lo);
      _rev(T_Hi, Val_Hi);
      _mov(DestLo, T_Hi);
      _mov(DestHi, T_Lo);
    } else {
      assert(Ty == IceType_i32 || Ty == IceType_i16);
      Variable *ValR = legalizeToReg(Val);
      Variable *T = makeReg(Ty);
      _rev(T, ValR);
      if (Val->getType() == IceType_i16) {
        Operand *_16 = shAmtImm(16);
        _lsr(T, T, _16);
      }
      _mov(Dest, T);
    }
    return;
  }
  case Intrinsics::Ctpop: {
    llvm::report_fatal_error("Ctpop should have been prelowered.");
  }
  case Intrinsics::Ctlz: {
    // The "is zero undef" parameter is ignored and we always return a
    // well-defined value.
    Operand *Val = Instr->getArg(0);
    Variable *ValLoR;
    Variable *ValHiR = nullptr;
    if (Val->getType() == IceType_i64) {
      Val = legalizeUndef(Val);
      ValLoR = legalizeToReg(loOperand(Val));
      ValHiR = legalizeToReg(hiOperand(Val));
    } else {
      ValLoR = legalizeToReg(Val);
    }
    lowerCLZ(Dest, ValLoR, ValHiR);
    return;
  }
  case Intrinsics::Cttz: {
    // Essentially like Clz, but reverse the bits first.
    Operand *Val = Instr->getArg(0);
    Variable *ValLoR;
    Variable *ValHiR = nullptr;
    if (Val->getType() == IceType_i64) {
      Val = legalizeUndef(Val);
      ValLoR = legalizeToReg(loOperand(Val));
      ValHiR = legalizeToReg(hiOperand(Val));
      Variable *TLo = makeReg(IceType_i32);
      Variable *THi = makeReg(IceType_i32);
      _rbit(TLo, ValLoR);
      _rbit(THi, ValHiR);
      ValLoR = THi;
      ValHiR = TLo;
    } else {
      ValLoR = legalizeToReg(Val);
      Variable *T = makeReg(IceType_i32);
      _rbit(T, ValLoR);
      ValLoR = T;
    }
    lowerCLZ(Dest, ValLoR, ValHiR);
    return;
  }
  case Intrinsics::Fabs: {
    Variable *T = makeReg(DestTy);
    _vabs(T, legalizeToReg(Instr->getArg(0)));
    _mov(Dest, T);
    return;
  }
  case Intrinsics::Longjmp: {
    llvm::report_fatal_error("longjmp should have been prelowered.");
  }
  case Intrinsics::Memcpy: {
    llvm::report_fatal_error("memcpy should have been prelowered.");
  }
  case Intrinsics::Memmove: {
    llvm::report_fatal_error("memmove should have been prelowered.");
  }
  case Intrinsics::Memset: {
    llvm::report_fatal_error("memmove should have been prelowered.");
  }
  case Intrinsics::Setjmp: {
    llvm::report_fatal_error("setjmp should have been prelowered.");
  }
  case Intrinsics::Sqrt: {
    Variable *Src = legalizeToReg(Instr->getArg(0));
    Variable *T = makeReg(DestTy);
    _vsqrt(T, Src);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::Stacksave: {
    Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
    _mov(Dest, SP);
    return;
  }
  case Intrinsics::Stackrestore: {
    Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
    Variable *Val = legalizeToReg(Instr->getArg(0));
    _mov_redefined(SP, Val);
    return;
  }
  case Intrinsics::Trap:
    _trap();
    return;
  case Intrinsics::AddSaturateSigned:
  case Intrinsics::AddSaturateUnsigned: {
    bool Unsigned = (ID == Intrinsics::AddSaturateUnsigned);
    Variable *Src0 = legalizeToReg(Instr->getArg(0));
    Variable *Src1 = legalizeToReg(Instr->getArg(1));
    Variable *T = makeReg(DestTy);
    _vqadd(T, Src0, Src1, Unsigned);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::LoadSubVector: {
    assert(llvm::isa<ConstantInteger32>(Instr->getArg(1)) &&
           "LoadSubVector second argument must be a constant");
    Variable *Dest = Instr->getDest();
    Type Ty = Dest->getType();
    auto *SubVectorSize = llvm::cast<ConstantInteger32>(Instr->getArg(1));
    Operand *Addr = Instr->getArg(0);
    OperandARM32Mem *Src = formMemoryOperand(Addr, Ty);
    doMockBoundsCheck(Src);

    if (Dest->isRematerializable()) {
      Context.insert<InstFakeDef>(Dest);
      return;
    }

    auto *T = makeReg(Ty);
    switch (SubVectorSize->getValue()) {
    case 4:
      _vldr1d(T, Src);
      break;
    case 8:
      _vldr1q(T, Src);
      break;
    default:
      Func->setError("Unexpected size for LoadSubVector");
      return;
    }
    _mov(Dest, T);
    return;
  }
  case Intrinsics::StoreSubVector: {
    assert(llvm::isa<ConstantInteger32>(Instr->getArg(2)) &&
           "StoreSubVector third argument must be a constant");
    auto *SubVectorSize = llvm::cast<ConstantInteger32>(Instr->getArg(2));
    Variable *Value = legalizeToReg(Instr->getArg(0));
    Operand *Addr = Instr->getArg(1);
    OperandARM32Mem *NewAddr = formMemoryOperand(Addr, Value->getType());
    doMockBoundsCheck(NewAddr);

    Value = legalizeToReg(Value);

    switch (SubVectorSize->getValue()) {
    case 4:
      _vstr1d(Value, NewAddr);
      break;
    case 8:
      _vstr1q(Value, NewAddr);
      break;
    default:
      Func->setError("Unexpected size for StoreSubVector");
      return;
    }
    return;
  }
  case Intrinsics::MultiplyAddPairs: {
    Variable *Src0 = legalizeToReg(Instr->getArg(0));
    Variable *Src1 = legalizeToReg(Instr->getArg(1));
    Variable *T = makeReg(DestTy);
    _vmlap(T, Src0, Src1);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::MultiplyHighSigned:
  case Intrinsics::MultiplyHighUnsigned: {
    bool Unsigned = (ID == Intrinsics::MultiplyHighUnsigned);
    Variable *Src0 = legalizeToReg(Instr->getArg(0));
    Variable *Src1 = legalizeToReg(Instr->getArg(1));
    Variable *T = makeReg(DestTy);
    _vmulh(T, Src0, Src1, Unsigned);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::Nearbyint: {
    UnimplementedLoweringError(this, Instr);
    return;
  }
  case Intrinsics::Round: {
    UnimplementedLoweringError(this, Instr);
    return;
  }
  case Intrinsics::SignMask: {
    UnimplementedLoweringError(this, Instr);
    return;
  }
  case Intrinsics::SubtractSaturateSigned:
  case Intrinsics::SubtractSaturateUnsigned: {
    bool Unsigned = (ID == Intrinsics::SubtractSaturateUnsigned);
    Variable *Src0 = legalizeToReg(Instr->getArg(0));
    Variable *Src1 = legalizeToReg(Instr->getArg(1));
    Variable *T = makeReg(DestTy);
    _vqsub(T, Src0, Src1, Unsigned);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::VectorPackSigned:
  case Intrinsics::VectorPackUnsigned: {
    bool Unsigned = (ID == Intrinsics::VectorPackUnsigned);
    bool Saturating = true;
    Variable *Src0 = legalizeToReg(Instr->getArg(0));
    Variable *Src1 = legalizeToReg(Instr->getArg(1));
    Variable *T = makeReg(DestTy);
    _vqmovn2(T, Src0, Src1, Unsigned, Saturating);
    _mov(Dest, T);
    return;
  }
  default: // UnknownIntrinsic
    Func->setError("Unexpected intrinsic");
    return;
  }
  return;
}

void TargetARM32::lowerCLZ(Variable *Dest, Variable *ValLoR, Variable *ValHiR) {
  Type Ty = Dest->getType();
  assert(Ty == IceType_i32 || Ty == IceType_i64);
  Variable *T = makeReg(IceType_i32);
  _clz(T, ValLoR);
  if (Ty == IceType_i64) {
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Operand *Zero =
        legalize(Ctx->getConstantZero(IceType_i32), Legal_Reg | Legal_Flex);
    Operand *ThirtyTwo =
        legalize(Ctx->getConstantInt32(32), Legal_Reg | Legal_Flex);
    _cmp(ValHiR, Zero);
    Variable *T2 = makeReg(IceType_i32);
    _add(T2, T, ThirtyTwo);
    _clz(T2, ValHiR, CondARM32::NE);
    // T2 is actually a source as well when the predicate is not AL (since it
    // may leave T2 alone). We use _set_dest_redefined to prolong the liveness
    // of T2 as if it was used as a source.
    _set_dest_redefined();
    _mov(DestLo, T2);
    Variable *T3 = makeReg(Zero->getType());
    _mov(T3, Zero);
    _mov(DestHi, T3);
    return;
  }
  _mov(Dest, T);
  return;
}

void TargetARM32::lowerLoad(const InstLoad *Load) {
  // A Load instruction can be treated the same as an Assign instruction, after
  // the source operand is transformed into an OperandARM32Mem operand.
  Type Ty = Load->getDest()->getType();
  Operand *Src0 = formMemoryOperand(Load->getLoadAddress(), Ty);
  Variable *DestLoad = Load->getDest();

  // TODO(jvoung): handled folding opportunities. Sign and zero extension can
  // be folded into a load.
  auto *Assign = InstAssign::create(Func, DestLoad, Src0);
  lowerAssign(Assign);
}

namespace {
void dumpAddressOpt(const Cfg *Func, const Variable *Base, int32_t Offset,
                    const Variable *OffsetReg, int16_t OffsetRegShAmt,
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
  Str << ", OffsetReg=";
  if (OffsetReg)
    OffsetReg->dump(Func);
  else
    Str << "<null>";
  Str << ", Shift=" << OffsetRegShAmt << ", Offset=" << Offset << "\n";
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

bool matchCombinedBaseIndex(const VariablesMetadata *VMetadata, Variable **Base,
                            Variable **OffsetReg, int32_t OffsetRegShamt,
                            const Inst **Reason) {
  // OffsetReg==nullptr && Base is Base=Var1+Var2 ==>
  //   set Base=Var1, OffsetReg=Var2, Shift=0
  if (*Base == nullptr)
    return false;
  if (*OffsetReg != nullptr)
    return false;
  (void)OffsetRegShamt;
  assert(OffsetRegShamt == 0);
  const Inst *BaseInst = VMetadata->getSingleDefinition(*Base);
  if (BaseInst == nullptr)
    return false;
  assert(!VMetadata->isMultiDef(*Base));
  if (BaseInst->getSrcSize() < 2)
    return false;
  auto *Var1 = llvm::dyn_cast<Variable>(BaseInst->getSrc(0));
  if (!Var1)
    return false;
  if (VMetadata->isMultiDef(Var1))
    return false;
  auto *Var2 = llvm::dyn_cast<Variable>(BaseInst->getSrc(1));
  if (!Var2)
    return false;
  if (VMetadata->isMultiDef(Var2))
    return false;
  InstArithmetic::OpKind _;
  if (!isAddOrSub(BaseInst, &_) ||
      // TODO: ensure Var1 and Var2 stay single-BB
      false)
    return false;
  *Base = Var1;
  *OffsetReg = Var2;
  // OffsetRegShamt is already 0.
  *Reason = BaseInst;
  return true;
}

bool matchShiftedOffsetReg(const VariablesMetadata *VMetadata,
                           Variable **OffsetReg, OperandARM32::ShiftKind *Kind,
                           int32_t *OffsetRegShamt, const Inst **Reason) {
  // OffsetReg is OffsetReg=Var*Const && log2(Const)+Shift<=32 ==>
  //   OffsetReg=Var, Shift+=log2(Const)
  // OffsetReg is OffsetReg=Var<<Const && Const+Shift<=32 ==>
  //   OffsetReg=Var, Shift+=Const
  // OffsetReg is OffsetReg=Var>>Const && Const-Shift>=-32 ==>
  //   OffsetReg=Var, Shift-=Const
  OperandARM32::ShiftKind NewShiftKind = OperandARM32::kNoShift;
  if (*OffsetReg == nullptr)
    return false;
  auto *IndexInst = VMetadata->getSingleDefinition(*OffsetReg);
  if (IndexInst == nullptr)
    return false;
  assert(!VMetadata->isMultiDef(*OffsetReg));
  if (IndexInst->getSrcSize() < 2)
    return false;
  auto *ArithInst = llvm::dyn_cast<InstArithmetic>(IndexInst);
  if (ArithInst == nullptr)
    return false;
  auto *Var = llvm::dyn_cast<Variable>(ArithInst->getSrc(0));
  if (Var == nullptr)
    return false;
  auto *Const = llvm::dyn_cast<ConstantInteger32>(ArithInst->getSrc(1));
  if (Const == nullptr) {
    assert(!llvm::isa<ConstantInteger32>(ArithInst->getSrc(0)));
    return false;
  }
  if (VMetadata->isMultiDef(Var) || Const->getType() != IceType_i32)
    return false;

  uint32_t NewShamt = -1;
  switch (ArithInst->getOp()) {
  default:
    return false;
  case InstArithmetic::Shl: {
    NewShiftKind = OperandARM32::LSL;
    NewShamt = Const->getValue();
    if (NewShamt > 31)
      return false;
  } break;
  case InstArithmetic::Lshr: {
    NewShiftKind = OperandARM32::LSR;
    NewShamt = Const->getValue();
    if (NewShamt > 31)
      return false;
  } break;
  case InstArithmetic::Ashr: {
    NewShiftKind = OperandARM32::ASR;
    NewShamt = Const->getValue();
    if (NewShamt > 31)
      return false;
  } break;
  case InstArithmetic::Udiv:
  case InstArithmetic::Mul: {
    const uint32_t UnsignedConst = Const->getValue();
    NewShamt = llvm::findFirstSet(UnsignedConst);
    if (NewShamt != llvm::findLastSet(UnsignedConst)) {
      // First bit set is not the same as the last bit set, so Const is not
      // a power of 2.
      return false;
    }
    NewShiftKind = ArithInst->getOp() == InstArithmetic::Udiv
                       ? OperandARM32::LSR
                       : OperandARM32::LSL;
  } break;
  }
  // Allowed "transitions":
  //   kNoShift -> * iff NewShamt < 31
  //   LSL -> LSL    iff NewShamt + OffsetRegShamt < 31
  //   LSR -> LSR    iff NewShamt + OffsetRegShamt < 31
  //   ASR -> ASR    iff NewShamt + OffsetRegShamt < 31
  if (*Kind != OperandARM32::kNoShift && *Kind != NewShiftKind) {
    return false;
  }
  const int32_t NewOffsetRegShamt = *OffsetRegShamt + NewShamt;
  if (NewOffsetRegShamt > 31)
    return false;
  *OffsetReg = Var;
  *OffsetRegShamt = NewOffsetRegShamt;
  *Kind = NewShiftKind;
  *Reason = IndexInst;
  return true;
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

OperandARM32Mem *TargetARM32::formAddressingMode(Type Ty, Cfg *Func,
                                                 const Inst *LdSt,
                                                 Operand *Base) {
  assert(Base != nullptr);
  int32_t OffsetImm = 0;
  Variable *OffsetReg = nullptr;
  int32_t OffsetRegShamt = 0;
  OperandARM32::ShiftKind ShiftKind = OperandARM32::kNoShift;

  Func->resetCurrentNode();
  if (Func->isVerbose(IceV_AddrOpt)) {
    OstreamLocker _(Func->getContext());
    Ostream &Str = Func->getContext()->getStrDump();
    Str << "\nAddress mode formation:\t";
    LdSt->dumpDecorated(Func);
  }

  if (isVectorType(Ty))
    // vector loads and stores do not allow offsets, and only support the
    // "[reg]" addressing mode (the other supported modes are write back.)
    return nullptr;

  auto *BaseVar = llvm::dyn_cast<Variable>(Base);
  if (BaseVar == nullptr)
    return nullptr;

  (void)MemTraitsSize;
  assert(Ty < MemTraitsSize);
  auto *TypeTraits = &MemTraits[Ty];
  const bool CanHaveIndex = TypeTraits->CanHaveIndex;
  const bool CanHaveShiftedIndex = TypeTraits->CanHaveShiftedIndex;
  const bool CanHaveImm = TypeTraits->CanHaveImm;
  const int32_t ValidImmMask = TypeTraits->ValidImmMask;
  (void)ValidImmMask;
  assert(!CanHaveImm || ValidImmMask >= 0);

  const VariablesMetadata *VMetadata = Func->getVMetadata();
  const Inst *Reason = nullptr;

  do {
    if (Reason != nullptr) {
      dumpAddressOpt(Func, BaseVar, OffsetImm, OffsetReg, OffsetRegShamt,
                     Reason);
      Reason = nullptr;
    }

    if (matchAssign(VMetadata, &BaseVar, &OffsetImm, &Reason)) {
      continue;
    }

    if (CanHaveIndex &&
        matchAssign(VMetadata, &OffsetReg, &OffsetImm, &Reason)) {
      continue;
    }

    if (CanHaveIndex && matchCombinedBaseIndex(VMetadata, &BaseVar, &OffsetReg,
                                               OffsetRegShamt, &Reason)) {
      continue;
    }

    if (CanHaveShiftedIndex) {
      if (matchShiftedOffsetReg(VMetadata, &OffsetReg, &ShiftKind,
                                &OffsetRegShamt, &Reason)) {
        continue;
      }

      if ((OffsetRegShamt == 0) &&
          matchShiftedOffsetReg(VMetadata, &BaseVar, &ShiftKind,
                                &OffsetRegShamt, &Reason)) {
        std::swap(BaseVar, OffsetReg);
        continue;
      }
    }

    if (matchOffsetBase(VMetadata, &BaseVar, &OffsetImm, &Reason)) {
      continue;
    }
  } while (Reason);

  if (BaseVar == nullptr) {
    // [OffsetReg{, LSL Shamt}{, #OffsetImm}] is not legal in ARM, so we have to
    // legalize the addressing mode to [BaseReg, OffsetReg{, LSL Shamt}].
    // Instead of a zeroed BaseReg, we initialize it with OffsetImm:
    //
    // [OffsetReg{, LSL Shamt}{, #OffsetImm}] ->
    //     mov BaseReg, #OffsetImm
    //     use of [BaseReg, OffsetReg{, LSL Shamt}]
    //
    const Type PointerType = getPointerType();
    BaseVar = makeReg(PointerType);
    Context.insert<InstAssign>(BaseVar, Ctx->getConstantInt32(OffsetImm));
    OffsetImm = 0;
  } else if (OffsetImm != 0) {
    // ARM Ldr/Str instructions have limited range immediates. The formation
    // loop above materialized an Immediate carelessly, so we ensure the
    // generated offset is sane.
    const int32_t PositiveOffset = OffsetImm > 0 ? OffsetImm : -OffsetImm;
    const InstArithmetic::OpKind Op =
        OffsetImm > 0 ? InstArithmetic::Add : InstArithmetic::Sub;

    if (!CanHaveImm || !isLegalMemOffset(Ty, OffsetImm) ||
        OffsetReg != nullptr) {
      if (OffsetReg == nullptr) {
        // We formed a [Base, #const] addressing mode which is not encodable in
        // ARM. There is little point in forming an address mode now if we don't
        // have an offset. Effectively, we would end up with something like
        //
        // [Base, #const] -> add T, Base, #const
        //                   use of [T]
        //
        // Which is exactly what we already have. So we just bite the bullet
        // here and don't form any address mode.
        return nullptr;
      }
      // We formed [Base, Offset {, LSL Amnt}, #const]. Oops. Legalize it to
      //
      // [Base, Offset, {LSL amount}, #const] ->
      //      add T, Base, #const
      //      use of [T, Offset {, LSL amount}]
      const Type PointerType = getPointerType();
      Variable *T = makeReg(PointerType);
      Context.insert<InstArithmetic>(Op, T, BaseVar,
                                     Ctx->getConstantInt32(PositiveOffset));
      BaseVar = T;
      OffsetImm = 0;
    }
  }

  assert(BaseVar != nullptr);
  assert(OffsetImm == 0 || OffsetReg == nullptr);
  assert(OffsetReg == nullptr || CanHaveIndex);
  assert(OffsetImm < 0 ? (ValidImmMask & -OffsetImm) == -OffsetImm
                       : (ValidImmMask & OffsetImm) == OffsetImm);

  if (OffsetReg != nullptr) {
    Variable *OffsetR = makeReg(getPointerType());
    Context.insert<InstAssign>(OffsetR, OffsetReg);
    return OperandARM32Mem::create(Func, Ty, BaseVar, OffsetR, ShiftKind,
                                   OffsetRegShamt);
  }

  return OperandARM32Mem::create(
      Func, Ty, BaseVar,
      llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(OffsetImm)));
}

void TargetARM32::doAddressOptLoad() {
  Inst *Instr = iteratorToInst(Context.getCur());
  assert(llvm::isa<InstLoad>(Instr));
  Variable *Dest = Instr->getDest();
  Operand *Addr = Instr->getSrc(0);
  if (OperandARM32Mem *Mem =
          formAddressingMode(Dest->getType(), Func, Instr, Addr)) {
    Instr->setDeleted();
    Context.insert<InstLoad>(Dest, Mem);
  }
}

void TargetARM32::lowerPhi(const InstPhi * /*Instr*/) {
  Func->setError("Phi found in regular instruction list");
}

void TargetARM32::lowerRet(const InstRet *Instr) {
  Variable *Reg = nullptr;
  if (Instr->hasRetValue()) {
    Operand *Src0 = Instr->getRetValue();
    Type Ty = Src0->getType();
    if (Ty == IceType_i64) {
      Src0 = legalizeUndef(Src0);
      Variable *R0 = legalizeToReg(loOperand(Src0), RegARM32::Reg_r0);
      Variable *R1 = legalizeToReg(hiOperand(Src0), RegARM32::Reg_r1);
      Reg = R0;
      Context.insert<InstFakeUse>(R1);
    } else if (Ty == IceType_f32) {
      Variable *S0 = legalizeToReg(Src0, RegARM32::Reg_s0);
      Reg = S0;
    } else if (Ty == IceType_f64) {
      Variable *D0 = legalizeToReg(Src0, RegARM32::Reg_d0);
      Reg = D0;
    } else if (isVectorType(Src0->getType())) {
      Variable *Q0 = legalizeToReg(Src0, RegARM32::Reg_q0);
      Reg = Q0;
    } else {
      Operand *Src0F = legalize(Src0, Legal_Reg | Legal_Flex);
      Reg = makeReg(Src0F->getType(), RegARM32::Reg_r0);
      _mov(Reg, Src0F, CondARM32::AL);
    }
  }
  // Add a ret instruction even if sandboxing is enabled, because addEpilog
  // explicitly looks for a ret instruction as a marker for where to insert the
  // frame removal instructions. addEpilog is responsible for restoring the
  // "lr" register as needed prior to this ret instruction.
  _ret(getPhysicalRegister(RegARM32::Reg_lr), Reg);

  // Add a fake use of sp to make sure sp stays alive for the entire function.
  // Otherwise post-call sp adjustments get dead-code eliminated.
  // TODO: Are there more places where the fake use should be inserted? E.g.
  // "void f(int n){while(1) g(n);}" may not have a ret instruction.
  Variable *SP = getPhysicalRegister(RegARM32::Reg_sp);
  Context.insert<InstFakeUse>(SP);
}

void TargetARM32::lowerShuffleVector(const InstShuffleVector *Instr) {
  auto *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();

  auto *T = makeReg(DestTy);
  auto *Src0 = Instr->getSrc(0);
  auto *Src1 = Instr->getSrc(1);
  const SizeT NumElements = typeNumElements(DestTy);
  const Type ElementType = typeElementType(DestTy);

  bool Replicate = true;
  for (SizeT I = 1; Replicate && I < Instr->getNumIndexes(); ++I) {
    if (Instr->getIndexValue(I) != Instr->getIndexValue(0)) {
      Replicate = false;
    }
  }

  if (Replicate) {
    Variable *Src0Var = legalizeToReg(Src0);
    _vdup(T, Src0Var, Instr->getIndexValue(0));
    _mov(Dest, T);
    return;
  }

  switch (DestTy) {
  case IceType_v8i1:
  case IceType_v8i16: {
    static constexpr SizeT ExpectedNumElements = 8;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    (void)ExpectedNumElements;

    if (Instr->indexesAre(0, 0, 1, 1, 2, 2, 3, 3)) {
      Variable *Src0R = legalizeToReg(Src0);
      _vzip(T, Src0R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 8, 1, 9, 2, 10, 3, 11)) {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1);
      _vzip(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 2, 4, 6, 0, 2, 4, 6)) {
      Variable *Src0R = legalizeToReg(Src0);
      _vqmovn2(T, Src0R, Src0R, false, false);
      _mov(Dest, T);
      return;
    }
  } break;
  case IceType_v16i1:
  case IceType_v16i8: {
    static constexpr SizeT ExpectedNumElements = 16;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    (void)ExpectedNumElements;

    if (Instr->indexesAre(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7)) {
      Variable *Src0R = legalizeToReg(Src0);
      _vzip(T, Src0R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7,
                          23)) {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1);
      _vzip(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }
  } break;
  case IceType_v4i1:
  case IceType_v4i32:
  case IceType_v4f32: {
    static constexpr SizeT ExpectedNumElements = 4;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    (void)ExpectedNumElements;

    if (Instr->indexesAre(0, 0, 1, 1)) {
      Variable *Src0R = legalizeToReg(Src0);
      _vzip(T, Src0R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 4, 1, 5)) {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1);
      _vzip(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 1, 4, 5)) {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1);
      _vmovlh(T, Src0R, Src1R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(2, 3, 2, 3)) {
      Variable *Src0R = legalizeToReg(Src0);
      _vmovhl(T, Src0R, Src0R);
      _mov(Dest, T);
      return;
    }

    if (Instr->indexesAre(2, 3, 6, 7)) {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *Src1R = legalizeToReg(Src1);
      _vmovhl(T, Src1R, Src0R);
      _mov(Dest, T);
      return;
    }
  } break;
  default:
    break;
    // TODO(jpp): figure out how to properly lower this without scalarization.
  }

  // Unoptimized shuffle. Perform a series of inserts and extracts.
  Context.insert<InstFakeDef>(T);
  for (SizeT I = 0; I < Instr->getNumIndexes(); ++I) {
    auto *Index = Instr->getIndex(I);
    const SizeT Elem = Index->getValue();
    auto *ExtElmt = makeReg(ElementType);
    if (Elem < NumElements) {
      lowerExtractElement(
          InstExtractElement::create(Func, ExtElmt, Src0, Index));
    } else {
      lowerExtractElement(InstExtractElement::create(
          Func, ExtElmt, Src1,
          Ctx->getConstantInt32(Index->getValue() - NumElements)));
    }
    auto *NewT = makeReg(DestTy);
    lowerInsertElement(InstInsertElement::create(Func, NewT, T, ExtElmt,
                                                 Ctx->getConstantInt32(I)));
    T = NewT;
  }
  _mov(Dest, T);
}

void TargetARM32::lowerSelect(const InstSelect *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = Dest->getType();
  Operand *SrcT = Instr->getTrueOperand();
  Operand *SrcF = Instr->getFalseOperand();
  Operand *Condition = Instr->getCondition();

  if (!isVectorType(DestTy)) {
    lowerInt1ForSelect(Dest, Condition, legalizeUndef(SrcT),
                       legalizeUndef(SrcF));
    return;
  }

  Type TType = DestTy;
  switch (DestTy) {
  default:
    llvm::report_fatal_error("Unexpected type for vector select.");
  case IceType_v4i1:
    TType = IceType_v4i32;
    break;
  case IceType_v8i1:
    TType = IceType_v8i16;
    break;
  case IceType_v16i1:
    TType = IceType_v16i8;
    break;
  case IceType_v4f32:
    TType = IceType_v4i32;
    break;
  case IceType_v4i32:
  case IceType_v8i16:
  case IceType_v16i8:
    break;
  }
  auto *T = makeReg(TType);
  lowerCast(InstCast::create(Func, InstCast::Sext, T, Condition));
  auto *SrcTR = legalizeToReg(SrcT);
  auto *SrcFR = legalizeToReg(SrcF);
  _vbsl(T, SrcTR, SrcFR)->setDestRedefined();
  _mov(Dest, T);
}

void TargetARM32::lowerStore(const InstStore *Instr) {
  Operand *Value = Instr->getData();
  Operand *Addr = Instr->getStoreAddress();
  OperandARM32Mem *NewAddr = formMemoryOperand(Addr, Value->getType());
  Type Ty = NewAddr->getType();

  if (Ty == IceType_i64) {
    Value = legalizeUndef(Value);
    Variable *ValueHi = legalizeToReg(hiOperand(Value));
    Variable *ValueLo = legalizeToReg(loOperand(Value));
    _str(ValueHi, llvm::cast<OperandARM32Mem>(hiOperand(NewAddr)));
    _str(ValueLo, llvm::cast<OperandARM32Mem>(loOperand(NewAddr)));
  } else {
    Variable *ValueR = legalizeToReg(Value);
    _str(ValueR, NewAddr);
  }
}

void TargetARM32::doAddressOptStore() {
  Inst *Instr = iteratorToInst(Context.getCur());
  assert(llvm::isa<InstStore>(Instr));
  Operand *Src = Instr->getSrc(0);
  Operand *Addr = Instr->getSrc(1);
  if (OperandARM32Mem *Mem =
          formAddressingMode(Src->getType(), Func, Instr, Addr)) {
    Instr->setDeleted();
    Context.insert<InstStore>(Src, Mem);
  }
}

void TargetARM32::lowerSwitch(const InstSwitch *Instr) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  Operand *Src0 = Instr->getComparison();
  SizeT NumCases = Instr->getNumCases();
  if (Src0->getType() == IceType_i64) {
    Src0 = legalizeUndef(Src0);
    Variable *Src0Lo = legalizeToReg(loOperand(Src0));
    Variable *Src0Hi = legalizeToReg(hiOperand(Src0));
    for (SizeT I = 0; I < NumCases; ++I) {
      Operand *ValueLo = Ctx->getConstantInt32(Instr->getValue(I));
      Operand *ValueHi = Ctx->getConstantInt32(Instr->getValue(I) >> 32);
      ValueLo = legalize(ValueLo, Legal_Reg | Legal_Flex);
      ValueHi = legalize(ValueHi, Legal_Reg | Legal_Flex);
      _cmp(Src0Lo, ValueLo);
      _cmp(Src0Hi, ValueHi, CondARM32::EQ);
      _br(Instr->getLabel(I), CondARM32::EQ);
    }
    _br(Instr->getLabelDefault());
    return;
  }

  Variable *Src0Var = legalizeToReg(Src0);
  // If Src0 is not an i32, we left shift it -- see the icmp lowering for the
  // reason.
  assert(Src0Var->mustHaveReg());
  const size_t ShiftAmt = 32 - getScalarIntBitWidth(Src0->getType());
  assert(ShiftAmt < 32);
  if (ShiftAmt > 0) {
    Operand *ShAmtImm = shAmtImm(ShiftAmt);
    Variable *T = makeReg(IceType_i32);
    _lsl(T, Src0Var, ShAmtImm);
    Src0Var = T;
  }

  for (SizeT I = 0; I < NumCases; ++I) {
    Operand *Value = Ctx->getConstantInt32(Instr->getValue(I) << ShiftAmt);
    Value = legalize(Value, Legal_Reg | Legal_Flex);
    _cmp(Src0Var, Value);
    _br(Instr->getLabel(I), CondARM32::EQ);
  }
  _br(Instr->getLabelDefault());
}

void TargetARM32::lowerBreakpoint(const InstBreakpoint *Instr) {
  UnimplementedLoweringError(this, Instr);
}

void TargetARM32::lowerUnreachable(const InstUnreachable * /*Instr*/) {
  _trap();
}

void TargetARM32::prelowerPhis() {
  CfgNode *Node = Context.getNode();
  PhiLowering::prelowerPhis32Bit(this, Node, Func);
}

Variable *TargetARM32::makeVectorOfZeros(Type Ty, RegNumT RegNum) {
  Variable *Reg = makeReg(Ty, RegNum);
  Context.insert<InstFakeDef>(Reg);
  assert(isVectorType(Ty));
  _veor(Reg, Reg, Reg);
  return Reg;
}

// Helper for legalize() to emit the right code to lower an operand to a
// register of the appropriate type.
Variable *TargetARM32::copyToReg(Operand *Src, RegNumT RegNum) {
  Type Ty = Src->getType();
  Variable *Reg = makeReg(Ty, RegNum);
  if (auto *Mem = llvm::dyn_cast<OperandARM32Mem>(Src)) {
    _ldr(Reg, Mem);
  } else {
    _mov(Reg, Src);
  }
  return Reg;
}

// TODO(jpp): remove unneeded else clauses in legalize.
Operand *TargetARM32::legalize(Operand *From, LegalMask Allowed,
                               RegNumT RegNum) {
  Type Ty = From->getType();
  // Assert that a physical register is allowed. To date, all calls to
  // legalize() allow a physical register. Legal_Flex converts registers to the
  // right type OperandARM32FlexReg as needed.
  assert(Allowed & Legal_Reg);

  // Copied ipsis literis from TargetX86Base<Machine>.
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

  // Go through the various types of operands: OperandARM32Mem,
  // OperandARM32Flex, Constant, and Variable. Given the above assertion, if
  // type of operand is not legal (e.g., OperandARM32Mem and !Legal_Mem), we
  // can always copy to a register.
  if (auto *Mem = llvm::dyn_cast<OperandARM32Mem>(From)) {
    // Before doing anything with a Mem operand, we need to ensure that the
    // Base and Index components are in physical registers.
    Variable *Base = Mem->getBase();
    Variable *Index = Mem->getIndex();
    ConstantInteger32 *Offset = Mem->getOffset();
    assert(Index == nullptr || Offset == nullptr);
    Variable *RegBase = nullptr;
    Variable *RegIndex = nullptr;
    assert(Base);
    RegBase = llvm::cast<Variable>(
        legalize(Base, Legal_Reg | Legal_Rematerializable));
    assert(Ty < MemTraitsSize);
    if (Index) {
      assert(Offset == nullptr);
      assert(MemTraits[Ty].CanHaveIndex);
      RegIndex = legalizeToReg(Index);
    }
    if (Offset && Offset->getValue() != 0) {
      assert(Index == nullptr);
      static constexpr bool ZeroExt = false;
      assert(MemTraits[Ty].CanHaveImm);
      if (!OperandARM32Mem::canHoldOffset(Ty, ZeroExt, Offset->getValue())) {
        llvm::report_fatal_error("Invalid memory offset.");
      }
    }

    // Create a new operand if there was a change.
    if (Base != RegBase || Index != RegIndex) {
      // There is only a reg +/- reg or reg + imm form.
      // Figure out which to re-create.
      if (RegIndex) {
        Mem = OperandARM32Mem::create(Func, Ty, RegBase, RegIndex,
                                      Mem->getShiftOp(), Mem->getShiftAmt(),
                                      Mem->getAddrMode());
      } else {
        Mem = OperandARM32Mem::create(Func, Ty, RegBase, Offset,
                                      Mem->getAddrMode());
      }
    }
    if (Allowed & Legal_Mem) {
      From = Mem;
    } else {
      Variable *Reg = makeReg(Ty, RegNum);
      _ldr(Reg, Mem);
      From = Reg;
    }
    return From;
  }

  if (auto *Flex = llvm::dyn_cast<OperandARM32Flex>(From)) {
    if (!(Allowed & Legal_Flex)) {
      if (auto *FlexReg = llvm::dyn_cast<OperandARM32FlexReg>(Flex)) {
        if (FlexReg->getShiftOp() == OperandARM32::kNoShift) {
          From = FlexReg->getReg();
          // Fall through and let From be checked as a Variable below, where it
          // may or may not need a register.
        } else {
          return copyToReg(Flex, RegNum);
        }
      } else {
        return copyToReg(Flex, RegNum);
      }
    } else {
      return From;
    }
  }

  if (llvm::isa<Constant>(From)) {
    if (llvm::isa<ConstantUndef>(From)) {
      From = legalizeUndef(From, RegNum);
      if (isVectorType(Ty))
        return From;
    }
    // There should be no constants of vector type (other than undef).
    assert(!isVectorType(Ty));
    if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(From)) {
      uint32_t RotateAmt;
      uint32_t Immed_8;
      uint32_t Value = static_cast<uint32_t>(C32->getValue());
      if (OperandARM32FlexImm::canHoldImm(Value, &RotateAmt, &Immed_8)) {
        // The immediate can be encoded as a Flex immediate. We may return the
        // Flex operand if the caller has Allow'ed it.
        auto *OpF = OperandARM32FlexImm::create(Func, Ty, Immed_8, RotateAmt);
        const bool CanBeFlex = Allowed & Legal_Flex;
        if (CanBeFlex)
          return OpF;
        return copyToReg(OpF, RegNum);
      } else if (OperandARM32FlexImm::canHoldImm(~Value, &RotateAmt,
                                                 &Immed_8)) {
        // Even though the immediate can't be encoded as a Flex operand, its
        // inverted bit pattern can, thus we use ARM's mvn to load the 32-bit
        // constant with a single instruction.
        auto *InvOpF =
            OperandARM32FlexImm::create(Func, Ty, Immed_8, RotateAmt);
        Variable *Reg = makeReg(Ty, RegNum);
        _mvn(Reg, InvOpF);
        return Reg;
      } else {
        // Do a movw/movt to a register.
        Variable *Reg = makeReg(Ty, RegNum);
        uint32_t UpperBits = (Value >> 16) & 0xFFFF;
        _movw(Reg,
              UpperBits != 0 ? Ctx->getConstantInt32(Value & 0xFFFF) : C32);
        if (UpperBits != 0) {
          _movt(Reg, Ctx->getConstantInt32(UpperBits));
        }
        return Reg;
      }
    } else if (auto *C = llvm::dyn_cast<ConstantRelocatable>(From)) {
      Variable *Reg = makeReg(Ty, RegNum);
      _movw(Reg, C);
      _movt(Reg, C);
      return Reg;
    } else {
      assert(isScalarFloatingType(Ty));
      uint32_t ModifiedImm;
      if (OperandARM32FlexFpImm::canHoldImm(From, &ModifiedImm)) {
        Variable *T = makeReg(Ty, RegNum);
        _mov(T,
             OperandARM32FlexFpImm::create(Func, From->getType(), ModifiedImm));
        return T;
      }

      if (Ty == IceType_f64 && isFloatingPointZero(From)) {
        // Use T = T ^ T to load a 64-bit fp zero. This does not work for f32
        // because ARM does not have a veor instruction with S registers.
        Variable *T = makeReg(IceType_f64, RegNum);
        Context.insert<InstFakeDef>(T);
        _veor(T, T, T);
        return T;
      }

      // Load floats/doubles from literal pool.
      auto *CFrom = llvm::cast<Constant>(From);
      assert(CFrom->getShouldBePooled());
      Constant *Offset = Ctx->getConstantSym(0, CFrom->getLabelName());
      Variable *BaseReg = makeReg(getPointerType());
      _movw(BaseReg, Offset);
      _movt(BaseReg, Offset);
      From = formMemoryOperand(BaseReg, Ty);
      return copyToReg(From, RegNum);
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
    // Check if the variable is guaranteed a physical register. This can happen
    // either when the variable is pre-colored or when it is assigned infinite
    // weight.
    bool MustHaveRegister = (Var->hasReg() || Var->mustHaveReg());
    // We need a new physical register for the operand if:
    //   Mem is not allowed and Var isn't guaranteed a physical
    //   register, or
    //   RegNum is required and Var->getRegNum() doesn't match.
    if ((!(Allowed & Legal_Mem) && !MustHaveRegister) ||
        (RegNum.hasValue() && (RegNum != Var->getRegNum()))) {
      From = copyToReg(From, RegNum);
    }
    return From;
  }
  llvm::report_fatal_error("Unhandled operand kind in legalize()");

  return From;
}

/// Provide a trivial wrapper to legalize() for this common usage.
Variable *TargetARM32::legalizeToReg(Operand *From, RegNumT RegNum) {
  return llvm::cast<Variable>(legalize(From, Legal_Reg, RegNum));
}

/// Legalize undef values to concrete values.
Operand *TargetARM32::legalizeUndef(Operand *From, RegNumT RegNum) {
  Type Ty = From->getType();
  if (llvm::isa<ConstantUndef>(From)) {
    // Lower undefs to zero. Another option is to lower undefs to an
    // uninitialized register; however, using an uninitialized register results
    // in less predictable code.
    //
    // If in the future the implementation is changed to lower undef values to
    // uninitialized registers, a FakeDef will be needed:
    // Context.insert(InstFakeDef::create(Func, Reg)); This is in order to
    // ensure that the live range of Reg is not overestimated. If the constant
    // being lowered is a 64 bit value, then the result should be split and the
    // lo and hi components will need to go in uninitialized registers.
    if (isVectorType(Ty))
      return makeVectorOfZeros(Ty, RegNum);
    return Ctx->getConstantZero(Ty);
  }
  return From;
}

OperandARM32Mem *TargetARM32::formMemoryOperand(Operand *Operand, Type Ty) {
  auto *Mem = llvm::dyn_cast<OperandARM32Mem>(Operand);
  // It may be the case that address mode optimization already creates an
  // OperandARM32Mem, so in that case it wouldn't need another level of
  // transformation.
  if (Mem) {
    return llvm::cast<OperandARM32Mem>(legalize(Mem));
  }
  // If we didn't do address mode optimization, then we only have a
  // base/offset to work with. ARM always requires a base register, so
  // just use that to hold the operand.
  auto *Base = llvm::cast<Variable>(
      legalize(Operand, Legal_Reg | Legal_Rematerializable));
  return OperandARM32Mem::create(
      Func, Ty, Base,
      llvm::cast<ConstantInteger32>(Ctx->getConstantZero(IceType_i32)));
}

Variable64On32 *TargetARM32::makeI64RegPair() {
  Variable64On32 *Reg =
      llvm::cast<Variable64On32>(Func->makeVariable(IceType_i64));
  Reg->setMustHaveReg();
  Reg->initHiLo(Func);
  Reg->getLo()->setMustNotHaveReg();
  Reg->getHi()->setMustNotHaveReg();
  return Reg;
}

Variable *TargetARM32::makeReg(Type Type, RegNumT RegNum) {
  // There aren't any 64-bit integer registers for ARM32.
  assert(Type != IceType_i64);
  assert(AllowTemporaryWithNoReg || RegNum.hasValue());
  Variable *Reg = Func->makeVariable(Type);
  if (RegNum.hasValue())
    Reg->setRegNum(RegNum);
  else
    Reg->setMustHaveReg();
  return Reg;
}

void TargetARM32::alignRegisterPow2(Variable *Reg, uint32_t Align,
                                    RegNumT TmpRegNum) {
  assert(llvm::isPowerOf2_32(Align));
  uint32_t RotateAmt;
  uint32_t Immed_8;
  Operand *Mask;
  // Use AND or BIC to mask off the bits, depending on which immediate fits (if
  // it fits at all). Assume Align is usually small, in which case BIC works
  // better. Thus, this rounds down to the alignment.
  if (OperandARM32FlexImm::canHoldImm(Align - 1, &RotateAmt, &Immed_8)) {
    Mask = legalize(Ctx->getConstantInt32(Align - 1), Legal_Reg | Legal_Flex,
                    TmpRegNum);
    _bic(Reg, Reg, Mask);
  } else {
    Mask = legalize(Ctx->getConstantInt32(-Align), Legal_Reg | Legal_Flex,
                    TmpRegNum);
    _and(Reg, Reg, Mask);
  }
}

void TargetARM32::postLower() {
  if (Func->getOptLevel() == Opt_m1)
    return;
  markRedefinitions();
  Context.availabilityUpdate();
}

void TargetARM32::emit(const ConstantInteger32 *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "#" << C->getValue();
}

void TargetARM32::emit(const ConstantInteger64 *) const {
  llvm::report_fatal_error("Not expecting to emit 64-bit integers");
}

void TargetARM32::emit(const ConstantFloat *C) const {
  (void)C;
  UnimplementedError(getFlags());
}

void TargetARM32::emit(const ConstantDouble *C) const {
  (void)C;
  UnimplementedError(getFlags());
}

void TargetARM32::emit(const ConstantUndef *) const {
  llvm::report_fatal_error("undef value encountered by emitter.");
}

void TargetARM32::emit(const ConstantRelocatable *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "#";
  emitWithoutPrefix(C);
}

void TargetARM32::lowerInt1ForSelect(Variable *Dest, Operand *Boolean,
                                     Operand *TrueValue, Operand *FalseValue) {
  Operand *_1 = legalize(Ctx->getConstantInt1(1), Legal_Reg | Legal_Flex);

  assert(Boolean->getType() == IceType_i1);

  bool NeedsAnd1 = false;
  if (TrueValue->getType() == IceType_i1) {
    assert(FalseValue->getType() == IceType_i1);

    Variable *TrueValueV = Func->makeVariable(IceType_i1);
    SafeBoolChain Src0Safe = lowerInt1(TrueValueV, TrueValue);
    TrueValue = TrueValueV;

    Variable *FalseValueV = Func->makeVariable(IceType_i1);
    SafeBoolChain Src1Safe = lowerInt1(FalseValueV, FalseValue);
    FalseValue = FalseValueV;

    NeedsAnd1 = Src0Safe == SBC_No || Src1Safe == SBC_No;
  }

  Variable *DestLo = (Dest->getType() == IceType_i64)
                         ? llvm::cast<Variable>(loOperand(Dest))
                         : Dest;
  Variable *DestHi = (Dest->getType() == IceType_i64)
                         ? llvm::cast<Variable>(hiOperand(Dest))
                         : nullptr;
  Operand *FalseValueLo = (FalseValue->getType() == IceType_i64)
                              ? loOperand(FalseValue)
                              : FalseValue;
  Operand *FalseValueHi =
      (FalseValue->getType() == IceType_i64) ? hiOperand(FalseValue) : nullptr;

  Operand *TrueValueLo =
      (TrueValue->getType() == IceType_i64) ? loOperand(TrueValue) : TrueValue;
  Operand *TrueValueHi =
      (TrueValue->getType() == IceType_i64) ? hiOperand(TrueValue) : nullptr;

  Variable *T_Lo = makeReg(DestLo->getType());
  Variable *T_Hi = (DestHi == nullptr) ? nullptr : makeReg(DestHi->getType());

  _mov(T_Lo, legalize(FalseValueLo, Legal_Reg | Legal_Flex));
  if (DestHi) {
    _mov(T_Hi, legalize(FalseValueHi, Legal_Reg | Legal_Flex));
  }

  CondWhenTrue Cond(CondARM32::kNone);
  // FlagsWereSet is used to determine wether Boolean was folded or not. If not,
  // add an explicit _tst instruction below.
  bool FlagsWereSet = false;
  if (const Inst *Producer = Computations.getProducerOf(Boolean)) {
    switch (Producer->getKind()) {
    default:
      llvm::report_fatal_error("Unexpected producer.");
    case Inst::Icmp: {
      Cond = lowerIcmpCond(llvm::cast<InstIcmp>(Producer));
      FlagsWereSet = true;
    } break;
    case Inst::Fcmp: {
      Cond = lowerFcmpCond(llvm::cast<InstFcmp>(Producer));
      FlagsWereSet = true;
    } break;
    case Inst::Cast: {
      const auto *CastProducer = llvm::cast<InstCast>(Producer);
      assert(CastProducer->getCastKind() == InstCast::Trunc);
      Boolean = CastProducer->getSrc(0);
      // No flags were set, so a _tst(Src, 1) will be emitted below. Don't
      // bother legalizing Src to a Reg because it will be legalized before
      // emitting the tst instruction.
      FlagsWereSet = false;
    } break;
    case Inst::Arithmetic: {
      // This is a special case: we eagerly assumed Producer could be folded,
      // but in reality, it can't. No reason to panic: we just lower it using
      // the regular lowerArithmetic helper.
      const auto *ArithProducer = llvm::cast<InstArithmetic>(Producer);
      lowerArithmetic(ArithProducer);
      Boolean = ArithProducer->getDest();
      // No flags were set, so a _tst(Dest, 1) will be emitted below. Don't
      // bother legalizing Dest to a Reg because it will be legalized before
      // emitting  the tst instruction.
      FlagsWereSet = false;
    } break;
    }
  }

  if (!FlagsWereSet) {
    // No flags have been set, so emit a tst Boolean, 1.
    Variable *Src = legalizeToReg(Boolean);
    _tst(Src, _1);
    Cond = CondWhenTrue(CondARM32::NE); // i.e., CondARM32::NotZero.
  }

  if (Cond.WhenTrue0 == CondARM32::kNone) {
    assert(Cond.WhenTrue1 == CondARM32::kNone);
  } else {
    _mov_redefined(T_Lo, legalize(TrueValueLo, Legal_Reg | Legal_Flex),
                   Cond.WhenTrue0);
    if (DestHi) {
      _mov_redefined(T_Hi, legalize(TrueValueHi, Legal_Reg | Legal_Flex),
                     Cond.WhenTrue0);
    }
  }

  if (Cond.WhenTrue1 != CondARM32::kNone) {
    _mov_redefined(T_Lo, legalize(TrueValueLo, Legal_Reg | Legal_Flex),
                   Cond.WhenTrue1);
    if (DestHi) {
      _mov_redefined(T_Hi, legalize(TrueValueHi, Legal_Reg | Legal_Flex),
                     Cond.WhenTrue1);
    }
  }

  if (NeedsAnd1) {
    // We lowered something that is unsafe (i.e., can't provably be zero or
    // one). Truncate the result.
    _and(T_Lo, T_Lo, _1);
  }

  _mov(DestLo, T_Lo);
  if (DestHi) {
    _mov(DestHi, T_Hi);
  }
}

TargetARM32::SafeBoolChain TargetARM32::lowerInt1(Variable *Dest,
                                                  Operand *Boolean) {
  assert(Boolean->getType() == IceType_i1);
  Variable *T = makeReg(IceType_i1);
  Operand *_0 =
      legalize(Ctx->getConstantZero(IceType_i1), Legal_Reg | Legal_Flex);
  Operand *_1 = legalize(Ctx->getConstantInt1(1), Legal_Reg | Legal_Flex);

  SafeBoolChain Safe = SBC_Yes;
  if (const Inst *Producer = Computations.getProducerOf(Boolean)) {
    switch (Producer->getKind()) {
    default:
      llvm::report_fatal_error("Unexpected producer.");
    case Inst::Icmp: {
      _mov(T, _0);
      CondWhenTrue Cond = lowerIcmpCond(llvm::cast<InstIcmp>(Producer));
      assert(Cond.WhenTrue0 != CondARM32::AL);
      assert(Cond.WhenTrue0 != CondARM32::kNone);
      assert(Cond.WhenTrue1 == CondARM32::kNone);
      _mov_redefined(T, _1, Cond.WhenTrue0);
    } break;
    case Inst::Fcmp: {
      _mov(T, _0);
      Inst *MovZero = Context.getLastInserted();
      CondWhenTrue Cond = lowerFcmpCond(llvm::cast<InstFcmp>(Producer));
      if (Cond.WhenTrue0 == CondARM32::AL) {
        assert(Cond.WhenTrue1 == CondARM32::kNone);
        MovZero->setDeleted();
        _mov(T, _1);
      } else if (Cond.WhenTrue0 != CondARM32::kNone) {
        _mov_redefined(T, _1, Cond.WhenTrue0);
      }
      if (Cond.WhenTrue1 != CondARM32::kNone) {
        assert(Cond.WhenTrue0 != CondARM32::kNone);
        assert(Cond.WhenTrue0 != CondARM32::AL);
        _mov_redefined(T, _1, Cond.WhenTrue1);
      }
    } break;
    case Inst::Cast: {
      const auto *CastProducer = llvm::cast<InstCast>(Producer);
      assert(CastProducer->getCastKind() == InstCast::Trunc);
      Operand *Src = CastProducer->getSrc(0);
      if (Src->getType() == IceType_i64)
        Src = loOperand(Src);
      _mov(T, legalize(Src, Legal_Reg | Legal_Flex));
      Safe = SBC_No;
    } break;
    case Inst::Arithmetic: {
      const auto *ArithProducer = llvm::cast<InstArithmetic>(Producer);
      Safe = lowerInt1Arithmetic(ArithProducer);
      _mov(T, ArithProducer->getDest());
    } break;
    }
  } else {
    _mov(T, legalize(Boolean, Legal_Reg | Legal_Flex));
  }

  _mov(Dest, T);
  return Safe;
}

namespace {
namespace BoolFolding {
bool shouldTrackProducer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Icmp:
  case Inst::Fcmp:
    return true;
  case Inst::Cast: {
    switch (llvm::cast<InstCast>(&Instr)->getCastKind()) {
    default:
      return false;
    case InstCast::Trunc:
      return true;
    }
  }
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::And:
    case InstArithmetic::Or:
      return true;
    }
  }
  }
}

bool isValidConsumer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Br:
    return true;
  case Inst::Select:
    return !isVectorType(Instr.getDest()->getType());
  case Inst::Cast: {
    switch (llvm::cast<InstCast>(&Instr)->getCastKind()) {
    default:
      return false;
    case InstCast::Sext:
      return !isVectorType(Instr.getDest()->getType());
    case InstCast::Zext:
      return !isVectorType(Instr.getDest()->getType());
    }
  }
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::And:
      return !isVectorType(Instr.getDest()->getType());
    case InstArithmetic::Or:
      return !isVectorType(Instr.getDest()->getType());
    }
  }
  }
}
} // end of namespace BoolFolding

namespace FpFolding {
bool shouldTrackProducer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::Fmul:
      return true;
    }
  }
  }
}

bool isValidConsumer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::Fadd:
    case InstArithmetic::Fsub:
      return true;
    }
  }
  }
}
} // end of namespace FpFolding

namespace IntFolding {
bool shouldTrackProducer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::Mul:
      return true;
    }
  }
  }
}

bool isValidConsumer(const Inst &Instr) {
  switch (Instr.getKind()) {
  default:
    return false;
  case Inst::Arithmetic: {
    switch (llvm::cast<InstArithmetic>(&Instr)->getOp()) {
    default:
      return false;
    case InstArithmetic::Add:
    case InstArithmetic::Sub:
      return true;
    }
  }
  }
}
} // namespace IntFolding
} // end of anonymous namespace

void TargetARM32::ComputationTracker::recordProducers(CfgNode *Node) {
  for (Inst &Instr : Node->getInsts()) {
    // Check whether Instr is a valid producer.
    Variable *Dest = Instr.getDest();
    if (!Instr.isDeleted() // only consider non-deleted instructions; and
        && Dest            // only instructions with an actual dest var; and
        && Dest->getType() == IceType_i1 // only bool-type dest vars; and
        && BoolFolding::shouldTrackProducer(Instr)) { // white-listed instr.
      KnownComputations.emplace(Dest->getIndex(),
                                ComputationEntry(&Instr, IceType_i1));
    }
    if (!Instr.isDeleted() // only consider non-deleted instructions; and
        && Dest            // only instructions with an actual dest var; and
        && isScalarFloatingType(Dest->getType()) // fp-type only dest vars; and
        && FpFolding::shouldTrackProducer(Instr)) { // white-listed instr.
      KnownComputations.emplace(Dest->getIndex(),
                                ComputationEntry(&Instr, Dest->getType()));
    }
    if (!Instr.isDeleted() // only consider non-deleted instructions; and
        && Dest            // only instructions with an actual dest var; and
        && Dest->getType() == IceType_i32            // i32 only dest vars; and
        && IntFolding::shouldTrackProducer(Instr)) { // white-listed instr.
      KnownComputations.emplace(Dest->getIndex(),
                                ComputationEntry(&Instr, IceType_i32));
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
      case IceType_i32:
        if (IndexOfVarInInst(Var) != 1 || !IntFolding::isValidConsumer(Instr)) {
          KnownComputations.erase(VarNum);
          continue;
        }
        break;
      case IceType_f32:
      case IceType_f64:
        if (IndexOfVarInInst(Var) != 1 || !FpFolding::isValidConsumer(Instr)) {
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

TargetDataARM32::TargetDataARM32(GlobalContext *Ctx)
    : TargetDataLowering(Ctx) {}

void TargetDataARM32::lowerGlobals(const VariableDeclarationList &Vars,
                                   const std::string &SectionSuffix) {
  const bool IsPIC = false;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();
    Writer->writeDataSection(Vars, llvm::ELF::R_ARM_ABS32, SectionSuffix,
                             IsPIC);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    OstreamLocker _(Ctx);
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

// TODO(jpp): implement the following when implementing constant randomization:
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
const char ConstantPoolEmitterTraits<float>::AsmTag[] = ".long";
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
  using Traits = ConstantPoolEmitterTraits<T>;
  Str << Const->getLabelName();
  Str << ":\n\t" << Traits::AsmTag << "\t0x";
  T Value = Const->getValue();
  Str.write_hex(Traits::bitcastToUint64(Value));
  Str << "\t/* " << Traits::TypeName << " " << Value << " */\n";
}

template <typename T> void emitConstantPool(GlobalContext *Ctx) {
  if (!BuildDefs::dump()) {
    return;
  }

  using Traits = ConstantPoolEmitterTraits<T>;
  static constexpr size_t MinimumAlignment = 4;
  SizeT Align = std::max(MinimumAlignment, typeAlignInBytes(Traits::IceType));
  assert((Align % 4) == 0 && "Constants should be aligned");
  Ostream &Str = Ctx->getStrEmit();
  ConstantList Pool = Ctx->getConstantPool(Traits::IceType);

  Str << "\t.section\t.rodata.cst" << Align << ",\"aM\",%progbits," << Align
      << "\n"
      << "\t.align\t" << Align << "\n";

  for (Constant *C : Pool) {
    if (!C->getShouldBePooled()) {
      continue;
    }

    emitConstant<T>(Str, llvm::dyn_cast<typename Traits::ConstantType>(C));
  }
}
} // end of anonymous namespace

void TargetDataARM32::lowerConstants() {
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

void TargetDataARM32::lowerJumpTables() {
  if (getFlags().getDisableTranslation())
    return;
  switch (getFlags().getOutFileType()) {
  case FT_Elf:
    if (!Ctx->getJumpTables().empty()) {
      llvm::report_fatal_error("ARM32 does not support jump tables yet.");
    }
    break;
  case FT_Asm:
    // Already emitted from Cfg
    break;
  case FT_Iasm: {
    // TODO(kschimpf): Fill this in when we get more information.
    break;
  }
  }
}

TargetHeaderARM32::TargetHeaderARM32(GlobalContext *Ctx)
    : TargetHeaderLowering(Ctx), CPUFeatures(getFlags()) {}

void TargetHeaderARM32::lower() {
  OstreamLocker _(Ctx);
  Ostream &Str = Ctx->getStrEmit();
  Str << ".syntax unified\n";
  // Emit build attributes in format: .eabi_attribute TAG, VALUE. See Sec. 2 of
  // "Addenda to, and Errata in the ABI for the ARM architecture"
  // http://infocenter.arm.com
  //                  /help/topic/com.arm.doc.ihi0045d/IHI0045D_ABI_addenda.pdf
  //
  // Tag_conformance should be be emitted first in a file-scope sub-subsection
  // of the first public subsection of the attributes.
  Str << ".eabi_attribute 67, \"2.09\"      @ Tag_conformance\n";
  // Chromebooks are at least A15, but do A9 for higher compat. For some
  // reason, the LLVM ARM asm parser has the .cpu directive override the mattr
  // specified on the commandline. So to test hwdiv, we need to set the .cpu
  // directive higher (can't just rely on --mattr=...).
  if (CPUFeatures.hasFeature(TargetARM32Features::HWDivArm)) {
    Str << ".cpu    cortex-a15\n";
  } else {
    Str << ".cpu    cortex-a9\n";
  }
  Str << ".eabi_attribute 6, 10   @ Tag_CPU_arch: ARMv7\n"
      << ".eabi_attribute 7, 65   @ Tag_CPU_arch_profile: App profile\n";
  Str << ".eabi_attribute 8, 1    @ Tag_ARM_ISA_use: Yes\n"
      << ".eabi_attribute 9, 2    @ Tag_THUMB_ISA_use: Thumb-2\n";
  Str << ".fpu    neon\n"
      << ".eabi_attribute 17, 1   @ Tag_ABI_PCS_GOT_use: permit directly\n"
      << ".eabi_attribute 20, 1   @ Tag_ABI_FP_denormal\n"
      << ".eabi_attribute 21, 1   @ Tag_ABI_FP_exceptions\n"
      << ".eabi_attribute 23, 3   @ Tag_ABI_FP_number_model: IEEE 754\n"
      << ".eabi_attribute 34, 1   @ Tag_CPU_unaligned_access\n"
      << ".eabi_attribute 24, 1   @ Tag_ABI_align_needed: 8-byte\n"
      << ".eabi_attribute 25, 1   @ Tag_ABI_align_preserved: 8-byte\n"
      << ".eabi_attribute 28, 1   @ Tag_ABI_VFP_args\n"
      << ".eabi_attribute 36, 1   @ Tag_FP_HP_extension\n"
      << ".eabi_attribute 38, 1   @ Tag_ABI_FP_16bit_format\n"
      << ".eabi_attribute 42, 1   @ Tag_MPextension_use\n"
      << ".eabi_attribute 68, 1   @ Tag_Virtualization_use\n";
  if (CPUFeatures.hasFeature(TargetARM32Features::HWDivArm)) {
    Str << ".eabi_attribute 44, 2   @ Tag_DIV_use\n";
  }
  // Technically R9 is used for TLS with Sandboxing, and we reserve it.
  // However, for compatibility with current NaCl LLVM, don't claim that.
  Str << ".eabi_attribute 14, 3   @ Tag_ABI_PCS_R9_use: Not used\n";
}

SmallBitVector TargetARM32::TypeToRegisterSet[RegARM32::RCARM32_NUM];
SmallBitVector TargetARM32::TypeToRegisterSetUnfiltered[RegARM32::RCARM32_NUM];
SmallBitVector TargetARM32::RegisterAliases[RegARM32::Reg_NUM];

} // end of namespace ARM32
} // end of namespace Ice
