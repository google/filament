/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Subtarget Enumeration Source Fragment                                      *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_SUBTARGETINFO_ENUM
#undef GET_SUBTARGETINFO_ENUM

namespace llvm {
namespace Mips {
enum {
  FeatureAbs2008 = 0,
  FeatureCRC = 1,
  FeatureCnMips = 2,
  FeatureCnMipsP = 3,
  FeatureDSP = 4,
  FeatureDSPR2 = 5,
  FeatureDSPR3 = 6,
  FeatureEVA = 7,
  FeatureFP64Bit = 8,
  FeatureFPXX = 9,
  FeatureGINV = 10,
  FeatureGP64Bit = 11,
  FeatureLongCalls = 12,
  FeatureMSA = 13,
  FeatureMT = 14,
  FeatureMicroMips = 15,
  FeatureMips1 = 16,
  FeatureMips2 = 17,
  FeatureMips3 = 18,
  FeatureMips3D = 19,
  FeatureMips3_32 = 20,
  FeatureMips3_32r2 = 21,
  FeatureMips4 = 22,
  FeatureMips4_32 = 23,
  FeatureMips4_32r2 = 24,
  FeatureMips5 = 25,
  FeatureMips5_32r2 = 26,
  FeatureMips16 = 27,
  FeatureMips32 = 28,
  FeatureMips32r2 = 29,
  FeatureMips32r3 = 30,
  FeatureMips32r5 = 31,
  FeatureMips32r6 = 32,
  FeatureMips64 = 33,
  FeatureMips64r2 = 34,
  FeatureMips64r3 = 35,
  FeatureMips64r5 = 36,
  FeatureMips64r6 = 37,
  FeatureNaN2008 = 38,
  FeatureNoABICalls = 39,
  FeatureNoMadd4 = 40,
  FeatureNoOddSPReg = 41,
  FeaturePTR64Bit = 42,
  FeatureSingleFloat = 43,
  FeatureSoftFloat = 44,
  FeatureSym32 = 45,
  FeatureUseIndirectJumpsHazard = 46,
  FeatureUseTCCInDIV = 47,
  FeatureVFPU = 48,
  FeatureVirt = 49,
  FeatureXGOT = 50,
  ImplP5600 = 51,
  NumSubtargetFeatures = 52
};
} // end namespace Mips
} // end namespace llvm

#endif // GET_SUBTARGETINFO_ENUM


#ifdef GET_SUBTARGETINFO_MACRO
GET_SUBTARGETINFO_MACRO(Abs2008, false, abs2008)
GET_SUBTARGETINFO_MACRO(HasCnMips, false, hasCnMips)
GET_SUBTARGETINFO_MACRO(HasCnMipsP, false, hasCnMipsP)
GET_SUBTARGETINFO_MACRO(HasCRC, false, hasCRC)
GET_SUBTARGETINFO_MACRO(HasDSP, false, hasDSP)
GET_SUBTARGETINFO_MACRO(HasDSPR2, false, hasDSPR2)
GET_SUBTARGETINFO_MACRO(HasDSPR3, false, hasDSPR3)
GET_SUBTARGETINFO_MACRO(HasEVA, false, hasEVA)
GET_SUBTARGETINFO_MACRO(IsFP64bit, false, isFP64bit)
GET_SUBTARGETINFO_MACRO(IsFPXX, false, isFPXX)
GET_SUBTARGETINFO_MACRO(HasGINV, false, hasGINV)
GET_SUBTARGETINFO_MACRO(IsGP64bit, false, isGP64bit)
GET_SUBTARGETINFO_MACRO(UseLongCalls, false, useLongCalls)
GET_SUBTARGETINFO_MACRO(InMicroMipsMode, false, inMicroMipsMode)
GET_SUBTARGETINFO_MACRO(InMips16Mode, false, inMips16Mode)
GET_SUBTARGETINFO_MACRO(HasMips3_32, false, hasMips3_32)
GET_SUBTARGETINFO_MACRO(HasMips3_32r2, false, hasMips3_32r2)
GET_SUBTARGETINFO_MACRO(Has3D, false, has3D)
GET_SUBTARGETINFO_MACRO(HasMips4_32, false, hasMips4_32)
GET_SUBTARGETINFO_MACRO(HasMips4_32r2, false, hasMips4_32r2)
GET_SUBTARGETINFO_MACRO(HasMips5_32r2, false, hasMips5_32r2)
GET_SUBTARGETINFO_MACRO(HasMSA, false, hasMSA)
GET_SUBTARGETINFO_MACRO(HasMT, false, hasMT)
GET_SUBTARGETINFO_MACRO(IsNaN2008bit, false, isNaN2008bit)
GET_SUBTARGETINFO_MACRO(NoABICalls, false, noABICalls)
GET_SUBTARGETINFO_MACRO(DisableMadd4, false, disableMadd4)
GET_SUBTARGETINFO_MACRO(UseOddSPReg, true, useOddSPReg)
GET_SUBTARGETINFO_MACRO(IsPTR64bit, false, isPTR64bit)
GET_SUBTARGETINFO_MACRO(IsSingleFloat, false, isSingleFloat)
GET_SUBTARGETINFO_MACRO(IsSoftFloat, false, isSoftFloat)
GET_SUBTARGETINFO_MACRO(HasSym32, false, hasSym32)
GET_SUBTARGETINFO_MACRO(UseIndirectJumpsHazard, false, useIndirectJumpsHazard)
GET_SUBTARGETINFO_MACRO(UseTCCInDIV, true, useTCCInDIV)
GET_SUBTARGETINFO_MACRO(HasVFPU, false, hasVFPU)
GET_SUBTARGETINFO_MACRO(HasVirt, false, hasVirt)
GET_SUBTARGETINFO_MACRO(UseXGOT, false, useXGOT)
#undef GET_SUBTARGETINFO_MACRO
#endif // GET_SUBTARGETINFO_MACRO


#ifdef GET_SUBTARGETINFO_MC_DESC
#undef GET_SUBTARGETINFO_MC_DESC

namespace llvm {
// Sorted (by key) array of values for CPU features.
extern const llvm::SubtargetFeatureKV MipsFeatureKV[] = {
  { "abs2008", "Disable IEEE 754-2008 abs.fmt mode", Mips::FeatureAbs2008, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "cnmips", "Octeon cnMIPS Support", Mips::FeatureCnMips, { { { 0x400000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "cnmipsp", "Octeon+ cnMIPS Support", Mips::FeatureCnMipsP, { { { 0x4ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "crc", "Mips R6 CRC ASE", Mips::FeatureCRC, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "dsp", "Mips DSP ASE", Mips::FeatureDSP, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "dspr2", "Mips DSP-R2 ASE", Mips::FeatureDSPR2, { { { 0x10ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "dspr3", "Mips DSP-R3 ASE", Mips::FeatureDSPR3, { { { 0x30ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "eva", "Mips EVA ASE", Mips::FeatureEVA, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "fp64", "Support 64-bit FP registers", Mips::FeatureFP64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "fpxx", "Support for FPXX", Mips::FeatureFPXX, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ginv", "Mips Global Invalidate ASE", Mips::FeatureGINV, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "gp64", "General Purpose Registers are 64-bit wide", Mips::FeatureGP64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "long-calls", "Disable use of the jal instruction", Mips::FeatureLongCalls, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "micromips", "microMips mode", Mips::FeatureMicroMips, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips1", "Mips I ISA Support [highly experimental]", Mips::FeatureMips1, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips16", "Mips16 mode", Mips::FeatureMips16, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips2", "Mips II ISA Support [highly experimental]", Mips::FeatureMips2, { { { 0x10000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips3", "MIPS III ISA Support [highly experimental]", Mips::FeatureMips3, { { { 0x320900ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips32", "Mips32 ISA Support", Mips::FeatureMips32, { { { 0x920000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips32r2", "Mips32r2 ISA Support", Mips::FeatureMips32r2, { { { 0x15200000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips32r3", "Mips32r3 ISA Support", Mips::FeatureMips32r3, { { { 0x20000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips32r5", "Mips32r5 ISA Support", Mips::FeatureMips32r5, { { { 0x40000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips32r6", "Mips32r6 ISA Support [experimental]", Mips::FeatureMips32r6, { { { 0x4080000101ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips3_32", "Subset of MIPS-III that is also in MIPS32 [highly experimental]", Mips::FeatureMips3_32, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips3_32r2", "Subset of MIPS-III that is also in MIPS32r2 [highly experimental]", Mips::FeatureMips3_32r2, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips3d", "Mips 3D ASE", Mips::FeatureMips3D, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips4", "MIPS IV ISA Support", Mips::FeatureMips4, { { { 0x1840000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips4_32", "Subset of MIPS-IV that is also in MIPS32 [highly experimental]", Mips::FeatureMips4_32, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips4_32r2", "Subset of MIPS-IV that is also in MIPS32r2 [highly experimental]", Mips::FeatureMips4_32r2, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips5", "MIPS V ISA Support [highly experimental]", Mips::FeatureMips5, { { { 0x4400000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips5_32r2", "Subset of MIPS-V that is also in MIPS32r2 [highly experimental]", Mips::FeatureMips5_32r2, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips64", "Mips64 ISA Support", Mips::FeatureMips64, { { { 0x12000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips64r2", "Mips64r2 ISA Support", Mips::FeatureMips64r2, { { { 0x220000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips64r3", "Mips64r3 ISA Support", Mips::FeatureMips64r3, { { { 0x440000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips64r5", "Mips64r5 ISA Support", Mips::FeatureMips64r5, { { { 0x880000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mips64r6", "Mips64r6 ISA Support [experimental]", Mips::FeatureMips64r6, { { { 0x5100000001ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "msa", "Mips MSA ASE", Mips::FeatureMSA, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mt", "Mips MT ASE", Mips::FeatureMT, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "nan2008", "IEEE 754-2008 NaN encoding", Mips::FeatureNaN2008, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "noabicalls", "Disable SVR4-style position-independent code", Mips::FeatureNoABICalls, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "nomadd4", "Disable 4-operand madd.fmt and related instructions", Mips::FeatureNoMadd4, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "nooddspreg", "Disable odd numbered single-precision registers", Mips::FeatureNoOddSPReg, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "p5600", "The P5600 Processor", Mips::ImplP5600, { { { 0x80000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ptr64", "Pointers are 64-bit wide", Mips::FeaturePTR64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "single-float", "Only supports single precision float", Mips::FeatureSingleFloat, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "soft-float", "Does not support floating point instructions", Mips::FeatureSoftFloat, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "sym32", "Symbols are 32 bit on Mips64", Mips::FeatureSym32, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "use-indirect-jump-hazard", "Use indirect jump guards to prevent certain speculation based attacks", Mips::FeatureUseIndirectJumpsHazard, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "use-tcc-in-div", "Force the assembler to use trapping", Mips::FeatureUseTCCInDIV, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "vfpu", "Enable vector FPU instructions", Mips::FeatureVFPU, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "virt", "Mips Virtualization ASE", Mips::FeatureVirt, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "xgot", "Assume 32-bit GOT", Mips::FeatureXGOT, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
};

#ifdef DBGFIELD
#error "<target>GenSubtargetInfo.inc requires a DBGFIELD macro"
#endif
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#define DBGFIELD(x) x,
#else
#define DBGFIELD(x)
#endif

// ===============================================================
// Data tables for the new per-operand machine model.

// {ProcResourceIdx, Cycles}
extern const llvm::MCWriteProcResEntry MipsWriteProcResTable[] = {
  { 0,  0}, // Invalid
  { 1,  1}, // #1
  { 8,  1}, // #2
  { 1,  1}, // #3
  {10,  1}, // #4
  { 3,  1}, // #5
  { 9,  1}, // #6
  { 4,  1}, // #7
  {17,  1}, // #8
  {19,  1}, // #9
  {11, 33}, // #10
  {20, 33}, // #11
  { 1,  1}, // #12
  {18,  1}, // #13
  { 6,  1}, // #14
  {15,  1}, // #15
  { 6,  1}, // #16
  {14,  1}, // #17
  { 6, 33}, // #18
  { 6,  1}, // #19
  {12,  1}, // #20
  { 6, 14}, // #21
  { 7, 14}, // #22
  { 6, 29}, // #23
  { 7, 29}, // #24
  { 6, 21}, // #25
  { 7, 21}, // #26
  { 6, 10}, // #27
  { 7, 10}, // #28
  { 6,  1}, // #29
  {16,  1}, // #30
  { 6,  1}, // #31
  {13,  1}, // #32
  { 2,  1}, // #33
  {11, 31}, // #34
  {20, 31}, // #35
  { 5,  1}, // #36
  { 1,  1}, // #37
  { 5,  1}, // #38
  {10,  1}, // #39
  { 1,  1}, // #40
  {14,  1}, // #41
  { 1,  1}, // #42
  { 5,  1}, // #43
  {14,  1}, // #44
  { 1,  1}, // #45
  { 5,  3}, // #46
  {14,  1}, // #47
  { 1,  1}, // #48
  { 5,  1}, // #49
  { 8,  1}, // #50
  { 1,  1}, // #51
  { 2, 34}, // #52
  { 8,  1}, // #53
  { 6,  1}, // #54
  { 7, 23}, // #55
  {11,  1}, // #56
  { 6,  1}, // #57
  {11,  1}, // #58
  { 6,  1}, // #59
  { 7, 31}, // #60
  {11,  1}, // #61
  { 6,  1}, // #62
  { 7, 27}, // #63
  {11,  1}, // #64
  { 6,  1}, // #65
  { 7, 35}, // #66
  {11,  1}, // #67
  { 1,  1}, // #68
  { 6,  1}, // #69
  {13,  1}, // #70
  {14,  1}, // #71
  { 1,  1}, // #72
  { 6,  1}, // #73
  {11,  1}, // #74
  {14,  1}, // #75
  { 1,  1}, // #76
  { 6,  1}, // #77
  {12,  1}, // #78
  {14,  1} // #79
}; // MipsWriteProcResTable

// {Cycles, WriteResourceID}
extern const llvm::MCWriteLatencyEntry MipsWriteLatencyTable[] = {
  { 0,  0}, // Invalid
  { 1,  0}, // #1 GenericWriteALU_GenericWriteJump_GenericWriteTrap_GenericWriteCOPOther_GenericWriteCOP0Short_GenericWriteStore_GenericWritePref_GenericWriteSync_GenericWriteHILO_GenericWriteMSAShortLogic_GenericWriteFPUStore_P5600WriteALU_P5600WriteJump_P5600COP0_P5600COP2_P5600WriteStore_P5600WriteCache_P5600WriteAL2_P5600WriteMSAShortLogic_P5600WriteFPUS_P5600WriteMoveOtherUnitsToFPU_P5600WriteStoreFromOtherUnits
  { 2,  0}, // #2 GenericWriteJumpAndLink_GenericReadCOP0_GenericWriteLoad_GenericWriteStoreSC_GenericWriteMove_GenericWriteMSAShortInt_GenericWriteFPUCmp_GenericWriteFPUMoveGPRFPU_GenericWriteFPULoad_GenericWriteAtomic_GenericDSPShort_P5600WriteJumpAndLink_P5600WriteAL2ShadowMov_P5600WriteAL2CondMov_P5600WriteAL2BitExt_P5600WriteMSAShortInt_P5600WriteGPRFromBypass_P5600WriteStoreFromOtherUnits_P5600WriteAtomic
  { 3,  0}, // #3 GenericWriteCOP0_GenericWriteMSAOther3_P5600WriteAL2Mul_P5600WriteMSAOther3
  { 4,  0}, // #4 GenericWriteCOP0TLB_GenericWriteMul_GenericWriteFPUS_GenericWriteFPUMoveFP_P5600WriteLoad_P5600WriteLoadShifted_P5600WriteFPUL_P5600WriteLoadOtherUnitsToFPU
  { 5,  0}, // #5 GenericWriteCache_GenericWriteALULong_GenericWriteMDUtoGPR_GenericWriteFPUL_GenericWriteMSALongInt_GenericReadWriteCOP0Long_GenericDSPMTHLIP_GenericDSPMTHILO_P5600WriteAL2Mult_P5600WriteAL2MAdd_P5600WriteMSALongInt
  {33,  0}, // #6 GenericWriteDIV_GenericWriteFPUDivI
  {17,  0}, // #7 GenericWriteFPUDivS_GenericWriteFPUSqrtS
  {32,  0}, // #8 GenericWriteFPUDivD
  {29,  0}, // #9 GenericWriteFPUSqrtD
  {25,  0}, // #10 GenericWriteFPURcpD
  {13,  0}, // #11 GenericWriteFPURcpS
  {31,  0}, // #12 GenericWriteDIVU_P5600WriteFPUDivD
  { 6,  0}, // #13 GenericDSPLong_P5600WriteFPUL_MADDSUB
  {34,  0}, // #14 P5600WriteAL2Div_P5600WriteAL2DivU
  {23,  0}, // #15 P5600WriteFPUDivI_P5600WriteFPUDivS
  {27,  0}, // #16 P5600WriteFPUSqrtS_P5600WriteFPURsqrtD_P5600WriteFPURsqrtS
  {35,  0} // #17 P5600WriteFPUSqrtD
}; // MipsWriteLatencyTable

// {UseIdx, WriteResourceID, Cycles}
extern const llvm::MCReadAdvanceEntry MipsReadAdvanceTable[] = {
  {0,  0,  0}, // Invalid
  {0,  0,  5} // #1
}; // MipsReadAdvanceTable

// {Name, NumMicroOps, BeginGroup, EndGroup, RetireOOO, WriteProcResIdx,#, WriteLatencyIdx,#, ReadAdvanceIdx,#}
static const llvm::MCSchedClassDesc MipsGenericModelSchedClasses[] = {
  {DBGFIELD("InvalidSchedClass")  8191, false, false, false, 0, 0,  0, 0,  0, 0},
  {DBGFIELD("IIPseudo")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1
  {DBGFIELD("II_B")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #2
  {DBGFIELD("II_BCCZAL")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #3
  {DBGFIELD("II_MTC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #4
  {DBGFIELD("II_MFC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #5
  {DBGFIELD("II_JALR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #6
  {DBGFIELD("II_JAL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #7
  {DBGFIELD("II_CVT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #8
  {DBGFIELD("II_DMULT")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #9
  {DBGFIELD("II_DMULTU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #10
  {DBGFIELD("II_DDIV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #11
  {DBGFIELD("II_DDIVU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #12
  {DBGFIELD("II_IndirectBranchPseudo") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #13
  {DBGFIELD("II_MADD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #14
  {DBGFIELD("II_MADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #15
  {DBGFIELD("II_MFHI_MFLO")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #16
  {DBGFIELD("II_MSUB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #17
  {DBGFIELD("II_MSUBU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #18
  {DBGFIELD("II_MTHI_MTLO")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #19
  {DBGFIELD("II_MULT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #20
  {DBGFIELD("II_MULTU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #21
  {DBGFIELD("II_ReturnPseudo")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #22
  {DBGFIELD("II_DIV")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #23
  {DBGFIELD("II_DIVU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #24
  {DBGFIELD("II_J")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #25
  {DBGFIELD("II_JR")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #26
  {DBGFIELD("II_TRAP")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #27
  {DBGFIELD("II_ADD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #28
  {DBGFIELD("II_ADDIUPC")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #29
  {DBGFIELD("II_ADDIU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #30
  {DBGFIELD("II_ADDR_PS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #31
  {DBGFIELD("II_ADDU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #32
  {DBGFIELD("II_ADDI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #33
  {DBGFIELD("II_ALIGN")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #34
  {DBGFIELD("II_ALUIPC")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #35
  {DBGFIELD("II_AND")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #36
  {DBGFIELD("II_ANDI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #37
  {DBGFIELD("II_AUI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #38
  {DBGFIELD("II_AUIPC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #39
  {DBGFIELD("IIM16Alu")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #40
  {DBGFIELD("II_BADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #41
  {DBGFIELD("II_BC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #42
  {DBGFIELD("II_BALC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #43
  {DBGFIELD("II_BBIT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #44
  {DBGFIELD("II_BC1CCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #45
  {DBGFIELD("II_BC1F")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #46
  {DBGFIELD("II_BC1FL")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #47
  {DBGFIELD("II_BC1T")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #48
  {DBGFIELD("II_BC1TL")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #49
  {DBGFIELD("II_BC2CCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #50
  {DBGFIELD("II_BCC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #51
  {DBGFIELD("II_BCCC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #52
  {DBGFIELD("II_BCCZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #53
  {DBGFIELD("II_BCCZC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #54
  {DBGFIELD("II_BCCZALS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #55
  {DBGFIELD("II_BITSWAP")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #56
  {DBGFIELD("II_BREAK")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #57
  {DBGFIELD("II_CACHE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #58
  {DBGFIELD("II_CACHEE")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #59
  {DBGFIELD("II_CEIL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #60
  {DBGFIELD("II_CFC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #61
  {DBGFIELD("II_CFC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #62
  {DBGFIELD("II_INS")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #63
  {DBGFIELD("II_CLASS_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #64
  {DBGFIELD("II_CLASS_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #65
  {DBGFIELD("II_CLO")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #66
  {DBGFIELD("II_CLZ")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #67
  {DBGFIELD("II_CMP_CC_D")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #68
  {DBGFIELD("II_CMP_CC_S")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #69
  {DBGFIELD("II_CRC32B")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #70
  {DBGFIELD("II_CRC32CB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #71
  {DBGFIELD("II_CRC32CD")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #72
  {DBGFIELD("II_CRC32CH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #73
  {DBGFIELD("II_CRC32CW")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #74
  {DBGFIELD("II_CRC32D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #75
  {DBGFIELD("II_CRC32H")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #76
  {DBGFIELD("II_CRC32W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #77
  {DBGFIELD("II_CTC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #78
  {DBGFIELD("II_CTC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #79
  {DBGFIELD("II_C_CC_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #80
  {DBGFIELD("II_C_CC_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #81
  {DBGFIELD("II_DADD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #82
  {DBGFIELD("II_DADDI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #83
  {DBGFIELD("II_DADDIU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #84
  {DBGFIELD("II_DADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #85
  {DBGFIELD("II_DAHI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #86
  {DBGFIELD("II_DALIGN")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #87
  {DBGFIELD("II_DATI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #88
  {DBGFIELD("II_DAUI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #89
  {DBGFIELD("II_DBITSWAP")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #90
  {DBGFIELD("II_DCLO")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #91
  {DBGFIELD("II_DCLZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #92
  {DBGFIELD("II_DERET")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #93
  {DBGFIELD("II_EXT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #94
  {DBGFIELD("II_DI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #95
  {DBGFIELD("II_DLSA")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #96
  {DBGFIELD("II_DMFC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #97
  {DBGFIELD("II_DMFC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #98
  {DBGFIELD("II_DMFC2")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #99
  {DBGFIELD("II_DMFGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #100
  {DBGFIELD("II_DMOD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #101
  {DBGFIELD("II_DMODU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #102
  {DBGFIELD("II_DMT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #103
  {DBGFIELD("II_DMTC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #104
  {DBGFIELD("II_DMTC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #105
  {DBGFIELD("II_DMTC2")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #106
  {DBGFIELD("II_DMTGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #107
  {DBGFIELD("II_DMUH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #108
  {DBGFIELD("II_DMUHU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #109
  {DBGFIELD("II_DMUL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #110
  {DBGFIELD("II_POP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #111
  {DBGFIELD("II_DROTR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #112
  {DBGFIELD("II_DROTR32")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #113
  {DBGFIELD("II_DROTRV")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #114
  {DBGFIELD("II_DSBH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #115
  {DBGFIELD("II_DSHD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #116
  {DBGFIELD("II_DSLL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #117
  {DBGFIELD("II_DSLL32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #118
  {DBGFIELD("II_DSLLV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #119
  {DBGFIELD("II_DSRA")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #120
  {DBGFIELD("II_DSRA32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #121
  {DBGFIELD("II_DSRAV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #122
  {DBGFIELD("II_DSRL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #123
  {DBGFIELD("II_DSRL32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #124
  {DBGFIELD("II_DSRLV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #125
  {DBGFIELD("II_DSUB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #126
  {DBGFIELD("II_DSUBU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #127
  {DBGFIELD("II_DVP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #128
  {DBGFIELD("II_DVPE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #129
  {DBGFIELD("II_EHB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #130
  {DBGFIELD("II_EI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #131
  {DBGFIELD("II_EMT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #132
  {DBGFIELD("II_ERET")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #133
  {DBGFIELD("II_ERETNC")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #134
  {DBGFIELD("II_EVP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #135
  {DBGFIELD("II_EVPE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #136
  {DBGFIELD("II_ABS")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #137
  {DBGFIELD("II_SQRT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #138
  {DBGFIELD("II_ADD_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #139
  {DBGFIELD("II_ADD_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #140
  {DBGFIELD("II_ADD_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #141
  {DBGFIELD("II_DIV_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #142
  {DBGFIELD("II_DIV_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #143
  {DBGFIELD("II_FLOOR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #144
  {DBGFIELD("II_MOV_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #145
  {DBGFIELD("II_MOV_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #146
  {DBGFIELD("II_MUL_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #147
  {DBGFIELD("II_MUL_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #148
  {DBGFIELD("II_MUL_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #149
  {DBGFIELD("II_NEG")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #150
  {DBGFIELD("II_FORK")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #151
  {DBGFIELD("II_SQRT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #152
  {DBGFIELD("II_SUB_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #153
  {DBGFIELD("II_SUB_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #154
  {DBGFIELD("II_SUB_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #155
  {DBGFIELD("II_GINVI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #156
  {DBGFIELD("II_GINVT")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #157
  {DBGFIELD("II_HYPCALL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #158
  {DBGFIELD("II_JALR_HB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #159
  {DBGFIELD("II_JALRC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #160
  {DBGFIELD("II_JALRS")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #161
  {DBGFIELD("II_JALS")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #162
  {DBGFIELD("II_JIALC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #163
  {DBGFIELD("II_JIC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #164
  {DBGFIELD("II_JRADDIUSP")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #165
  {DBGFIELD("II_JRC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #166
  {DBGFIELD("II_JR_HB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #167
  {DBGFIELD("II_LB")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #168
  {DBGFIELD("II_LBE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #169
  {DBGFIELD("II_LBU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #170
  {DBGFIELD("II_LBUE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #171
  {DBGFIELD("II_LD")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #172
  {DBGFIELD("II_LDC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #173
  {DBGFIELD("II_LDC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #174
  {DBGFIELD("II_LDC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #175
  {DBGFIELD("II_LDL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #176
  {DBGFIELD("II_LDPC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #177
  {DBGFIELD("II_LDR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #178
  {DBGFIELD("II_LDXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #179
  {DBGFIELD("II_LH")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #180
  {DBGFIELD("II_LHE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #181
  {DBGFIELD("II_LHU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #182
  {DBGFIELD("II_LHUE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #183
  {DBGFIELD("II_LI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #184
  {DBGFIELD("II_LL")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #185
  {DBGFIELD("II_LLD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #186
  {DBGFIELD("II_LLE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #187
  {DBGFIELD("II_LSA")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #188
  {DBGFIELD("II_LUI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #189
  {DBGFIELD("II_LUXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #190
  {DBGFIELD("II_LW")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #191
  {DBGFIELD("II_LWC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #192
  {DBGFIELD("II_LWC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #193
  {DBGFIELD("II_LWC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #194
  {DBGFIELD("II_LWE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #195
  {DBGFIELD("II_LWL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #196
  {DBGFIELD("II_LWLE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #197
  {DBGFIELD("II_LWM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #198
  {DBGFIELD("II_LWPC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #199
  {DBGFIELD("II_LWP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #200
  {DBGFIELD("II_LWR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #201
  {DBGFIELD("II_LWRE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #202
  {DBGFIELD("II_LWUPC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #203
  {DBGFIELD("II_LWU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #204
  {DBGFIELD("II_LWXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #205
  {DBGFIELD("II_LWXS")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #206
  {DBGFIELD("II_MADDF_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #207
  {DBGFIELD("II_MADDF_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #208
  {DBGFIELD("II_MADD_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #209
  {DBGFIELD("II_MADD_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #210
  {DBGFIELD("II_MAX_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #211
  {DBGFIELD("II_MAXA_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #212
  {DBGFIELD("II_MAX_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #213
  {DBGFIELD("II_MAXA_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #214
  {DBGFIELD("II_MFC0")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #215
  {DBGFIELD("II_MFC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #216
  {DBGFIELD("II_MFGC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #217
  {DBGFIELD("II_MFHC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #218
  {DBGFIELD("II_MFHC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #219
  {DBGFIELD("II_MFHGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #220
  {DBGFIELD("II_MFTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #221
  {DBGFIELD("II_MIN_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #222
  {DBGFIELD("II_MINA_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #223
  {DBGFIELD("II_MIN_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #224
  {DBGFIELD("II_MINA_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #225
  {DBGFIELD("II_MOD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #226
  {DBGFIELD("II_MODU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #227
  {DBGFIELD("II_MOVE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #228
  {DBGFIELD("II_MOVF_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #229
  {DBGFIELD("II_MOVF")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #230
  {DBGFIELD("II_MOVF_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #231
  {DBGFIELD("II_MOVN_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #232
  {DBGFIELD("II_MOVN")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #233
  {DBGFIELD("II_MOVN_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #234
  {DBGFIELD("II_MOVT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #235
  {DBGFIELD("II_MOVT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #236
  {DBGFIELD("II_MOVT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #237
  {DBGFIELD("II_MOVZ_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #238
  {DBGFIELD("II_MOVZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #239
  {DBGFIELD("II_MOVZ_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #240
  {DBGFIELD("II_MSUBF_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #241
  {DBGFIELD("II_MSUBF_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #242
  {DBGFIELD("II_MSUB_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #243
  {DBGFIELD("II_MSUB_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #244
  {DBGFIELD("II_MTC0")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #245
  {DBGFIELD("II_MTC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #246
  {DBGFIELD("II_MTGC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #247
  {DBGFIELD("II_MTHC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #248
  {DBGFIELD("II_MTHC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #249
  {DBGFIELD("II_MTHGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #250
  {DBGFIELD("II_MTTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #251
  {DBGFIELD("II_MUH")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #252
  {DBGFIELD("II_MUHU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #253
  {DBGFIELD("II_MUL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #254
  {DBGFIELD("II_MULR_PS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #255
  {DBGFIELD("II_MULU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #256
  {DBGFIELD("II_NMADD_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #257
  {DBGFIELD("II_NMADD_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #258
  {DBGFIELD("II_NMSUB_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #259
  {DBGFIELD("II_NMSUB_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #260
  {DBGFIELD("II_NOR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #261
  {DBGFIELD("II_NOT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #262
  {DBGFIELD("II_OR")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #263
  {DBGFIELD("II_ORI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #264
  {DBGFIELD("II_PAUSE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #265
  {DBGFIELD("II_PREF")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #266
  {DBGFIELD("II_PREFE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #267
  {DBGFIELD("II_RDHWR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #268
  {DBGFIELD("II_RDPGPR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #269
  {DBGFIELD("II_RECIP_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #270
  {DBGFIELD("II_RECIP_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #271
  {DBGFIELD("II_RINT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #272
  {DBGFIELD("II_RINT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #273
  {DBGFIELD("II_ROTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #274
  {DBGFIELD("II_ROTRV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #275
  {DBGFIELD("II_ROUND")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #276
  {DBGFIELD("II_RSQRT_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #277
  {DBGFIELD("II_RSQRT_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #278
  {DBGFIELD("II_RESTORE")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #279
  {DBGFIELD("II_SB")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #280
  {DBGFIELD("II_SBE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #281
  {DBGFIELD("II_SC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #282
  {DBGFIELD("II_SCD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #283
  {DBGFIELD("II_SCE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #284
  {DBGFIELD("II_SD")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #285
  {DBGFIELD("II_SDBBP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #286
  {DBGFIELD("II_SDC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #287
  {DBGFIELD("II_SDC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #288
  {DBGFIELD("II_SDC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #289
  {DBGFIELD("II_SDL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #290
  {DBGFIELD("II_SDR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #291
  {DBGFIELD("II_SDXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #292
  {DBGFIELD("II_SEB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #293
  {DBGFIELD("II_SEH")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #294
  {DBGFIELD("II_SELCCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #295
  {DBGFIELD("II_SELCCZ_D")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #296
  {DBGFIELD("II_SELCCZ_S")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #297
  {DBGFIELD("II_SEL_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #298
  {DBGFIELD("II_SEL_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #299
  {DBGFIELD("II_SEQ_SNE")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #300
  {DBGFIELD("II_SEQI_SNEI")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #301
  {DBGFIELD("II_SH")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #302
  {DBGFIELD("II_SHE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #303
  {DBGFIELD("II_SIGRIE")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #304
  {DBGFIELD("II_SLL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #305
  {DBGFIELD("II_SLLV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #306
  {DBGFIELD("II_SLT_SLTU")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #307
  {DBGFIELD("II_SLTI_SLTIU")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #308
  {DBGFIELD("II_SRA")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #309
  {DBGFIELD("II_SRAV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #310
  {DBGFIELD("II_SRL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #311
  {DBGFIELD("II_SRLV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #312
  {DBGFIELD("II_SSNOP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #313
  {DBGFIELD("II_SUB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #314
  {DBGFIELD("II_SUBU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #315
  {DBGFIELD("II_SUXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #316
  {DBGFIELD("II_SW")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #317
  {DBGFIELD("II_SWC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #318
  {DBGFIELD("II_SWC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #319
  {DBGFIELD("II_SWC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #320
  {DBGFIELD("II_SWE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #321
  {DBGFIELD("II_SWL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #322
  {DBGFIELD("II_SWLE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #323
  {DBGFIELD("II_SWM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #324
  {DBGFIELD("II_SWP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #325
  {DBGFIELD("II_SWR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #326
  {DBGFIELD("II_SWRE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #327
  {DBGFIELD("II_SWXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #328
  {DBGFIELD("II_SYNC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #329
  {DBGFIELD("II_SYNCI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #330
  {DBGFIELD("II_SYSCALL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #331
  {DBGFIELD("II_SAVE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #332
  {DBGFIELD("II_TEQ")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #333
  {DBGFIELD("II_TEQI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #334
  {DBGFIELD("II_TGE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #335
  {DBGFIELD("II_TGEI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #336
  {DBGFIELD("II_TGEIU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #337
  {DBGFIELD("II_TGEU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #338
  {DBGFIELD("II_TLBGINV")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #339
  {DBGFIELD("II_TLBGINVF")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #340
  {DBGFIELD("II_TLBGP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #341
  {DBGFIELD("II_TLBGR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #342
  {DBGFIELD("II_TLBGWI")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #343
  {DBGFIELD("II_TLBGWR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #344
  {DBGFIELD("II_TLBINV")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #345
  {DBGFIELD("II_TLBINVF")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #346
  {DBGFIELD("II_TLBP")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #347
  {DBGFIELD("II_TLBR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #348
  {DBGFIELD("II_TLBWI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #349
  {DBGFIELD("II_TLBWR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #350
  {DBGFIELD("II_TLT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #351
  {DBGFIELD("II_TLTI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #352
  {DBGFIELD("II_TTLTIU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #353
  {DBGFIELD("II_TLTU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #354
  {DBGFIELD("II_TNE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #355
  {DBGFIELD("II_TNEI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #356
  {DBGFIELD("II_TRUNC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #357
  {DBGFIELD("II_WAIT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #358
  {DBGFIELD("II_WRPGPR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #359
  {DBGFIELD("II_WSBH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #360
  {DBGFIELD("II_XOR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #361
  {DBGFIELD("II_XORI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #362
  {DBGFIELD("II_YIELD")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #363
  {DBGFIELD("AND")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #364
  {DBGFIELD("LUi")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #365
  {DBGFIELD("NOR")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #366
  {DBGFIELD("OR")                 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #367
  {DBGFIELD("SLTi_SLTiu")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #368
  {DBGFIELD("SUB")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #369
  {DBGFIELD("SUBu")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #370
  {DBGFIELD("XOR")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #371
  {DBGFIELD("SSNOP")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #372
  {DBGFIELD("NOP")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #373
  {DBGFIELD("B")                  1, false, false, false,  3, 2,  1, 1,  0, 0}, // #374
  {DBGFIELD("BAL")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #375
  {DBGFIELD("BAL_BR_BGEZAL_BGEZALL_BLTZAL_BLTZALL") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #376
  {DBGFIELD("BEQ_BEQL_BNE_BNEL")  1, false, false, false,  3, 2,  1, 1,  0, 0}, // #377
  {DBGFIELD("BGEZ_BGEZL_BGTZ_BGTZL_BLEZ_BLEZL_BLTZ_BLTZL") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #378
  {DBGFIELD("BREAK")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #379
  {DBGFIELD("DERET")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #380
  {DBGFIELD("ERET")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #381
  {DBGFIELD("ERet_RetRA")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #382
  {DBGFIELD("ERETNC")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #383
  {DBGFIELD("J_TAILCALL")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #384
  {DBGFIELD("JR_TAILCALLREG_TAILCALLREGHB") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #385
  {DBGFIELD("JR_HB")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #386
  {DBGFIELD("PseudoIndirectBranch_PseudoIndirectHazardBranch") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #387
  {DBGFIELD("PseudoReturn")       1, false, false, false,  3, 2,  1, 1,  0, 0}, // #388
  {DBGFIELD("SDBBP")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #389
  {DBGFIELD("SYSCALL")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #390
  {DBGFIELD("TEQ")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #391
  {DBGFIELD("TEQI")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #392
  {DBGFIELD("TGE")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #393
  {DBGFIELD("TGEI")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #394
  {DBGFIELD("TGEIU")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #395
  {DBGFIELD("TGEU")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #396
  {DBGFIELD("TLT")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #397
  {DBGFIELD("TLTI")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #398
  {DBGFIELD("TLTU")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #399
  {DBGFIELD("TNE")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #400
  {DBGFIELD("TNEI")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #401
  {DBGFIELD("TRAP")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #402
  {DBGFIELD("TTLTIU")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #403
  {DBGFIELD("WAIT")               1, false, false, false,  5, 2,  3, 1,  0, 0}, // #404
  {DBGFIELD("PAUSE")              1, false, false, false,  5, 2,  3, 1,  0, 0}, // #405
  {DBGFIELD("JAL")                1, false, false, false,  3, 2,  2, 1,  0, 0}, // #406
  {DBGFIELD("JALR_JALRHBPseudo_JALRPseudo") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #407
  {DBGFIELD("JALR_HB")            1, false, false, false,  3, 2,  2, 1,  0, 0}, // #408
  {DBGFIELD("JALX")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #409
  {DBGFIELD("TLBINV")             1, false, false, false,  5, 2,  4, 1,  0, 0}, // #410
  {DBGFIELD("TLBINVF")            1, false, false, false,  5, 2,  4, 1,  0, 0}, // #411
  {DBGFIELD("TLBP")               1, false, false, false,  5, 2,  4, 1,  0, 0}, // #412
  {DBGFIELD("TLBR")               1, false, false, false,  5, 2,  4, 1,  0, 0}, // #413
  {DBGFIELD("TLBWI")              1, false, false, false,  5, 2,  4, 1,  0, 0}, // #414
  {DBGFIELD("TLBWR")              1, false, false, false,  5, 2,  4, 1,  0, 0}, // #415
  {DBGFIELD("MFC0")               1, false, false, false,  5, 2,  2, 1,  0, 0}, // #416
  {DBGFIELD("MTC0")               1, false, false, false,  5, 2,  3, 1,  0, 0}, // #417
  {DBGFIELD("MFC2")               1, false, false, false,  7, 1,  1, 1,  0, 0}, // #418
  {DBGFIELD("MTC2")               1, false, false, false,  7, 1,  1, 1,  0, 0}, // #419
  {DBGFIELD("HYPCALL")            1, false, false, false,  5, 2,  1, 1,  0, 0}, // #420
  {DBGFIELD("MFGC0")              1, false, false, false,  5, 2,  1, 1,  0, 0}, // #421
  {DBGFIELD("MFHGC0")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #422
  {DBGFIELD("MTGC0")              1, false, false, false,  5, 2,  1, 1,  0, 0}, // #423
  {DBGFIELD("MTHGC0")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #424
  {DBGFIELD("TLBGINV")            1, false, false, false,  5, 2,  1, 1,  0, 0}, // #425
  {DBGFIELD("TLBGINVF")           1, false, false, false,  5, 2,  1, 1,  0, 0}, // #426
  {DBGFIELD("TLBGP")              1, false, false, false,  5, 2,  1, 1,  0, 0}, // #427
  {DBGFIELD("TLBGR")              1, false, false, false,  5, 2,  1, 1,  0, 0}, // #428
  {DBGFIELD("TLBGWI")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #429
  {DBGFIELD("TLBGWR")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #430
  {DBGFIELD("LB")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #431
  {DBGFIELD("LBu")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #432
  {DBGFIELD("LH")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #433
  {DBGFIELD("LHu")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #434
  {DBGFIELD("LW")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #435
  {DBGFIELD("LL")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #436
  {DBGFIELD("LWC2")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #437
  {DBGFIELD("LWC3")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #438
  {DBGFIELD("LDC2")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #439
  {DBGFIELD("LDC3")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #440
  {DBGFIELD("LBE")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #441
  {DBGFIELD("LBuE")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #442
  {DBGFIELD("LHE")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #443
  {DBGFIELD("LHuE")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #444
  {DBGFIELD("LWE")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #445
  {DBGFIELD("LLE")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #446
  {DBGFIELD("LWPC")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #447
  {DBGFIELD("LWL")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #448
  {DBGFIELD("LWR")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #449
  {DBGFIELD("LWLE")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #450
  {DBGFIELD("LWRE")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #451
  {DBGFIELD("SB")                 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #452
  {DBGFIELD("SH")                 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #453
  {DBGFIELD("SW")                 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #454
  {DBGFIELD("SWC2")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #455
  {DBGFIELD("SWC3")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #456
  {DBGFIELD("SDC2")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #457
  {DBGFIELD("SDC3")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #458
  {DBGFIELD("SC")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #459
  {DBGFIELD("SBE")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #460
  {DBGFIELD("SHE")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #461
  {DBGFIELD("SWE")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #462
  {DBGFIELD("SCE")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #463
  {DBGFIELD("SWL")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #464
  {DBGFIELD("SWR")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #465
  {DBGFIELD("SWLE")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #466
  {DBGFIELD("SWRE")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #467
  {DBGFIELD("PREF")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #468
  {DBGFIELD("PREFE")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #469
  {DBGFIELD("CACHE")              1, false, false, false,  8, 2,  5, 1,  0, 0}, // #470
  {DBGFIELD("CACHEE")             1, false, false, false,  8, 2,  5, 1,  0, 0}, // #471
  {DBGFIELD("SYNC")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #472
  {DBGFIELD("SYNCI")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #473
  {DBGFIELD("CLO")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #474
  {DBGFIELD("CLZ")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #475
  {DBGFIELD("DI")                 1, false, false, false,  5, 2,  3, 1,  0, 0}, // #476
  {DBGFIELD("EI")                 1, false, false, false,  5, 2,  3, 1,  0, 0}, // #477
  {DBGFIELD("MFHI_MFLO_PseudoMFHI_PseudoMFLO") 1, false, false, false,  1, 2,  5, 1,  0, 0}, // #478
  {DBGFIELD("EHB")                1, false, false, false,  5, 2,  3, 1,  0, 0}, // #479
  {DBGFIELD("RDHWR")              1, false, false, false,  1, 2,  2, 1,  0, 0}, // #480
  {DBGFIELD("WSBH")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #481
  {DBGFIELD("MOVN_I_I")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #482
  {DBGFIELD("MOVZ_I_I")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #483
  {DBGFIELD("DIV_PseudoSDIV_SDIV") 1, false, false, false, 10, 2,  6, 1,  0, 0}, // #484
  {DBGFIELD("DIVU_PseudoUDIV_UDIV") 1, false, false, false, 10, 2,  6, 1,  0, 0}, // #485
  {DBGFIELD("MUL")                1, false, false, false, 12, 2,  5, 1,  0, 0}, // #486
  {DBGFIELD("MULT_PseudoMULT")    1, false, false, false, 12, 2,  4, 1,  0, 0}, // #487
  {DBGFIELD("MULTu_PseudoMULTu")  1, false, false, false, 12, 2,  4, 1,  0, 0}, // #488
  {DBGFIELD("MADD_PseudoMADD")    1, false, false, false, 12, 2,  1, 1,  0, 0}, // #489
  {DBGFIELD("MADDU_PseudoMADDU")  1, false, false, false, 12, 2,  1, 1,  0, 0}, // #490
  {DBGFIELD("MSUB_PseudoMSUB")    1, false, false, false, 12, 2,  1, 1,  0, 0}, // #491
  {DBGFIELD("MSUBU_PseudoMSUBU")  1, false, false, false, 12, 2,  1, 1,  0, 0}, // #492
  {DBGFIELD("MTHI_MTLO_PseudoMTLOHI") 1, false, false, false,  1, 2,  2, 1,  0, 0}, // #493
  {DBGFIELD("EXT")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #494
  {DBGFIELD("INS")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #495
  {DBGFIELD("ADD")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #496
  {DBGFIELD("ADDi")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #497
  {DBGFIELD("ADDiu")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #498
  {DBGFIELD("ANDi")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #499
  {DBGFIELD("ORi")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #500
  {DBGFIELD("ROTR")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #501
  {DBGFIELD("SEB")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #502
  {DBGFIELD("SEH")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #503
  {DBGFIELD("SLT_SLTu")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #504
  {DBGFIELD("SLL")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #505
  {DBGFIELD("SRA")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #506
  {DBGFIELD("SRL")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #507
  {DBGFIELD("XORi")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #508
  {DBGFIELD("ADDu")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #509
  {DBGFIELD("SLLV")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #510
  {DBGFIELD("SRAV")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #511
  {DBGFIELD("SRLV")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #512
  {DBGFIELD("LSA")                1, false, false, false, 14, 2,  1, 1,  0, 0}, // #513
  {DBGFIELD("COPY")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #514
  {DBGFIELD("VSHF_B_VSHF_D_VSHF_H_VSHF_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #515
  {DBGFIELD("BINSLI_B_BINSLI_D_BINSLI_H_BINSLI_W_BINSL_B_BINSL_D_BINSL_H_BINSL_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #516
  {DBGFIELD("BINSRI_B_BINSRI_D_BINSRI_H_BINSRI_W_BINSR_B_BINSR_D_BINSR_H_BINSR_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #517
  {DBGFIELD("INSERT_B_INSERT_D_INSERT_H_INSERT_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #518
  {DBGFIELD("SLDI_B_SLDI_D_SLDI_H_SLDI_W_SLD_B_SLD_D_SLD_H_SLD_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #519
  {DBGFIELD("BSETI_B_BSETI_D_BSETI_H_BSETI_W_BSET_B_BSET_D_BSET_H_BSET_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #520
  {DBGFIELD("BCLRI_B_BCLRI_D_BCLRI_H_BCLRI_W_BCLR_B_BCLR_D_BCLR_H_BCLR_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #521
  {DBGFIELD("BNEGI_B_BNEGI_D_BNEGI_H_BNEGI_W_BNEG_B_BNEG_D_BNEG_H_BNEG_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #522
  {DBGFIELD("BSELI_B_BSEL_V")     1, false, false, false, 14, 2,  2, 1,  0, 0}, // #523
  {DBGFIELD("BMNZI_B_BMNZ_V_BMZI_B_BMZ_V") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #524
  {DBGFIELD("BSEL_D_PSEUDO_BSEL_FD_PSEUDO_BSEL_FW_PSEUDO_BSEL_H_PSEUDO_BSEL_W_PSEUDO") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #525
  {DBGFIELD("PCNT_B_PCNT_D_PCNT_H_PCNT_W") 1, false, false, false, 14, 2,  3, 1,  0, 0}, // #526
  {DBGFIELD("SAT_S_B_SAT_S_D_SAT_S_H_SAT_S_W_SAT_U_B_SAT_U_D_SAT_U_H_SAT_U_W") 1, false, false, false, 14, 2,  3, 1,  0, 0}, // #527
  {DBGFIELD("BNZ_B_BNZ_D_BNZ_H_BNZ_V_BNZ_W_BZ_B_BZ_D_BZ_H_BZ_V_BZ_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #528
  {DBGFIELD("CFCMSA_CTCMSA")      1, false, false, false, 14, 2,  1, 1,  0, 0}, // #529
  {DBGFIELD("FABS_S_FABS_D32_FABS_D64") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #530
  {DBGFIELD("MOVF_D32_MOVF_D64")  1, false, false, false, 16, 2,  4, 1,  0, 0}, // #531
  {DBGFIELD("MOVF_S")             1, false, false, false, 16, 2,  4, 1,  0, 0}, // #532
  {DBGFIELD("MOVT_D32_MOVT_D64")  1, false, false, false, 16, 2,  4, 1,  0, 0}, // #533
  {DBGFIELD("MOVT_S")             1, false, false, false, 16, 2,  4, 1,  0, 0}, // #534
  {DBGFIELD("FMOV_D32_FMOV_D64")  1, false, false, false, 16, 2,  4, 1,  0, 0}, // #535
  {DBGFIELD("FMOV_S")             1, false, false, false, 16, 2,  4, 1,  0, 0}, // #536
  {DBGFIELD("FNEG_S_FNEG_D32_FNEG_D64") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #537
  {DBGFIELD("ADD_A_B_ADD_A_D_ADD_A_H_ADD_A_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #538
  {DBGFIELD("ADDS_A_B_ADDS_A_D_ADDS_A_H_ADDS_A_W_ADDS_S_B_ADDS_S_D_ADDS_S_H_ADDS_S_W_ADDS_U_B_ADDS_U_D_ADDS_U_H_ADDS_U_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #539
  {DBGFIELD("ADDVI_B_ADDVI_D_ADDVI_H_ADDVI_W_ADDV_B_ADDV_D_ADDV_H_ADDV_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #540
  {DBGFIELD("ASUB_S_B_ASUB_S_D_ASUB_S_H_ASUB_S_W_ASUB_U_B_ASUB_U_D_ASUB_U_H_ASUB_U_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #541
  {DBGFIELD("AVER_S_B_AVER_S_D_AVER_S_H_AVER_S_W_AVER_U_B_AVER_U_D_AVER_U_H_AVER_U_W_AVE_S_B_AVE_S_D_AVE_S_H_AVE_S_W_AVE_U_B_AVE_U_D_AVE_U_H_AVE_U_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #542
  {DBGFIELD("SHF_B_SHF_H_SHF_W")  1, false, false, false, 14, 2,  2, 1,  0, 0}, // #543
  {DBGFIELD("FILL_B_FILL_D_FILL_H_FILL_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #544
  {DBGFIELD("SPLATI_B_SPLATI_D_SPLATI_H_SPLATI_W_SPLAT_B_SPLAT_D_SPLAT_H_SPLAT_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #545
  {DBGFIELD("MOVE_V")             1, false, false, false, 14, 2,  1, 1,  0, 0}, // #546
  {DBGFIELD("LDI_B_LDI_D_LDI_H_LDI_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #547
  {DBGFIELD("AND_V_NOR_V_OR_V_XOR_V") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #548
  {DBGFIELD("ANDI_B_NORI_B_ORI_B_XORI_B") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #549
  {DBGFIELD("AND_V_D_PSEUDO_AND_V_H_PSEUDO_AND_V_W_PSEUDO_NOR_V_D_PSEUDO_NOR_V_H_PSEUDO_NOR_V_W_PSEUDO_OR_V_D_PSEUDO_OR_V_H_PSEUDO_OR_V_W_PSEUDO_XOR_V_D_PSEUDO_XOR_V_H_PSEUDO_XOR_V_W_PSEUDO") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #550
  {DBGFIELD("FILL_FD_PSEUDO_FILL_FW_PSEUDO") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #551
  {DBGFIELD("INSERT_FD_PSEUDO_INSERT_FW_PSEUDO") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #552
  {DBGFIELD("FEXP2_D_FEXP2_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #553
  {DBGFIELD("CLTI_S_B_CLTI_S_D_CLTI_S_H_CLTI_S_W_CLTI_U_B_CLTI_U_D_CLTI_U_H_CLTI_U_W_CLT_S_B_CLT_S_D_CLT_S_H_CLT_S_W_CLT_U_B_CLT_U_D_CLT_U_H_CLT_U_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #554
  {DBGFIELD("CLEI_S_B_CLEI_S_D_CLEI_S_H_CLEI_S_W_CLEI_U_B_CLEI_U_D_CLEI_U_H_CLEI_U_W_CLE_S_B_CLE_S_D_CLE_S_H_CLE_S_W_CLE_U_B_CLE_U_D_CLE_U_H_CLE_U_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #555
  {DBGFIELD("CEQI_B_CEQI_D_CEQI_H_CEQI_W_CEQ_B_CEQ_D_CEQ_H_CEQ_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #556
  {DBGFIELD("CMP_UN_D")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #557
  {DBGFIELD("CMP_UN_S")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #558
  {DBGFIELD("CMP_UEQ_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #559
  {DBGFIELD("CMP_UEQ_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #560
  {DBGFIELD("CMP_EQ_D")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #561
  {DBGFIELD("CMP_EQ_S")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #562
  {DBGFIELD("CMP_LT_D")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #563
  {DBGFIELD("CMP_LT_S")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #564
  {DBGFIELD("CMP_ULT_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #565
  {DBGFIELD("CMP_ULT_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #566
  {DBGFIELD("CMP_LE_D")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #567
  {DBGFIELD("CMP_LE_S")           1, false, false, false, 14, 2,  4, 1,  0, 0}, // #568
  {DBGFIELD("CMP_ULE_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #569
  {DBGFIELD("CMP_ULE_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #570
  {DBGFIELD("FSAF_D_FSAF_W_FSEQ_D_FSEQ_W_FSLE_D_FSLE_W_FSLT_D_FSLT_W_FSNE_D_FSNE_W_FSOR_D_FSOR_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #571
  {DBGFIELD("FSUEQ_D_FSUEQ_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #572
  {DBGFIELD("FSULE_D_FSULE_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #573
  {DBGFIELD("FSULT_D_FSULT_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #574
  {DBGFIELD("FSUNE_D_FSUNE_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #575
  {DBGFIELD("FSUN_D_FSUN_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #576
  {DBGFIELD("FCAF_D_FCAF_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #577
  {DBGFIELD("FCEQ_D_FCEQ_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #578
  {DBGFIELD("FCLE_D_FCLE_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #579
  {DBGFIELD("FCLT_D_FCLT_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #580
  {DBGFIELD("FCNE_D_FCNE_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #581
  {DBGFIELD("FCOR_D_FCOR_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #582
  {DBGFIELD("FCUEQ_D_FCUEQ_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #583
  {DBGFIELD("FCULE_D_FCULE_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #584
  {DBGFIELD("FCULT_D_FCULT_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #585
  {DBGFIELD("FCUNE_D_FCUNE_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #586
  {DBGFIELD("FCUN_D_FCUN_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #587
  {DBGFIELD("FABS_D_FABS_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #588
  {DBGFIELD("FFINT_S_D_FFINT_S_W_FFINT_U_D_FFINT_U_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #589
  {DBGFIELD("FFQL_D_FFQL_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #590
  {DBGFIELD("FFQR_D_FFQR_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #591
  {DBGFIELD("FTINT_S_D_FTINT_S_W_FTINT_U_D_FTINT_U_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #592
  {DBGFIELD("FRINT_D_FRINT_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #593
  {DBGFIELD("FTQ_H_FTQ_W")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #594
  {DBGFIELD("FTRUNC_S_D_FTRUNC_S_W_FTRUNC_U_D_FTRUNC_U_W") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #595
  {DBGFIELD("FEXDO_H_FEXDO_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #596
  {DBGFIELD("FEXUPL_D_FEXUPL_W")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #597
  {DBGFIELD("FEXUPR_D_FEXUPR_W")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #598
  {DBGFIELD("FCLASS_D_FCLASS_W")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #599
  {DBGFIELD("FMAX_A_D_FMAX_A_W")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #600
  {DBGFIELD("FMAX_D_FMAX_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #601
  {DBGFIELD("FMIN_A_D_FMIN_A_W")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #602
  {DBGFIELD("FMIN_D_FMIN_W")      1, false, false, false, 14, 2,  4, 1,  0, 0}, // #603
  {DBGFIELD("FLOG2_D_FLOG2_W")    1, false, false, false, 14, 2,  4, 1,  0, 0}, // #604
  {DBGFIELD("ILVL_B_ILVL_D_ILVL_H_ILVL_W_ILVR_B_ILVR_D_ILVR_H_ILVR_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #605
  {DBGFIELD("ILVEV_B_ILVEV_D_ILVEV_H_ILVEV_W_ILVOD_B_ILVOD_D_ILVOD_H_ILVOD_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #606
  {DBGFIELD("INSVE_B_INSVE_D_INSVE_H_INSVE_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #607
  {DBGFIELD("SUBS_S_B_SUBS_S_D_SUBS_S_H_SUBS_S_W_SUBS_U_B_SUBS_U_D_SUBS_U_H_SUBS_U_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #608
  {DBGFIELD("SUBSUS_U_B_SUBSUS_U_D_SUBSUS_U_H_SUBSUS_U_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #609
  {DBGFIELD("SUBSUU_S_B_SUBSUU_S_D_SUBSUU_S_H_SUBSUU_S_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #610
  {DBGFIELD("SUBVI_B_SUBVI_D_SUBVI_H_SUBVI_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #611
  {DBGFIELD("SUBV_B_SUBV_D_SUBV_H_SUBV_W") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #612
  {DBGFIELD("MOD_S_B_MOD_S_D_MOD_S_H_MOD_S_W_MOD_U_B_MOD_U_D_MOD_U_H_MOD_U_W") 1, false, false, false, 18, 1,  6, 1,  0, 0}, // #613
  {DBGFIELD("DIV_S_B_DIV_S_D_DIV_S_H_DIV_S_W_DIV_U_B_DIV_U_D_DIV_U_H_DIV_U_W") 1, false, false, false, 18, 1,  6, 1,  0, 0}, // #614
  {DBGFIELD("HADD_S_D_HADD_S_H_HADD_S_W_HADD_U_D_HADD_U_H_HADD_U_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #615
  {DBGFIELD("HSUB_S_D_HSUB_S_H_HSUB_S_W_HSUB_U_D_HSUB_U_H_HSUB_U_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #616
  {DBGFIELD("MAX_S_B_MAX_S_D_MAX_S_H_MAX_S_W_MIN_S_B_MIN_S_D_MIN_S_H_MIN_S_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #617
  {DBGFIELD("MAX_U_B_MAX_U_D_MAX_U_H_MAX_U_W_MIN_U_B_MIN_U_D_MIN_U_H_MIN_U_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #618
  {DBGFIELD("MAX_A_B_MAX_A_D_MAX_A_H_MAX_A_W_MIN_A_B_MIN_A_D_MIN_A_H_MIN_A_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #619
  {DBGFIELD("MAXI_S_B_MAXI_S_D_MAXI_S_H_MAXI_S_W_MAXI_U_B_MAXI_U_D_MAXI_U_H_MAXI_U_W_MINI_S_B_MINI_S_D_MINI_S_H_MINI_S_W_MINI_U_B_MINI_U_D_MINI_U_H_MINI_U_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #620
  {DBGFIELD("SRAI_B_SRAI_D_SRAI_H_SRAI_W_SRA_B_SRA_D_SRA_H_SRA_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #621
  {DBGFIELD("SRLI_B_SRLI_D_SRLI_H_SRLI_W_SRL_B_SRL_D_SRL_H_SRL_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #622
  {DBGFIELD("SRARI_B_SRARI_D_SRARI_H_SRARI_W_SRAR_B_SRAR_D_SRAR_H_SRAR_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #623
  {DBGFIELD("SRLRI_B_SRLRI_D_SRLRI_H_SRLRI_W_SRLR_B_SRLR_D_SRLR_H_SRLR_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #624
  {DBGFIELD("SLLI_B_SLLI_D_SLLI_H_SLLI_W_SLL_B_SLL_D_SLL_H_SLL_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #625
  {DBGFIELD("PCKEV_B_PCKEV_D_PCKEV_H_PCKEV_W_PCKOD_B_PCKOD_D_PCKOD_H_PCKOD_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #626
  {DBGFIELD("NLOC_B_NLOC_D_NLOC_H_NLOC_W_NLZC_B_NLZC_D_NLZC_H_NLZC_W") 1, false, false, false, 14, 2,  1, 1,  0, 0}, // #627
  {DBGFIELD("FADD_D32_FADD_D64")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #628
  {DBGFIELD("FADD_PS64")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #629
  {DBGFIELD("FADD_S")             1, false, false, false, 14, 2,  4, 1,  0, 0}, // #630
  {DBGFIELD("FMUL_D32_FMUL_D64")  1, false, false, false, 19, 2,  5, 1,  0, 0}, // #631
  {DBGFIELD("FMUL_PS64")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #632
  {DBGFIELD("FMUL_S")             1, false, false, false, 14, 2,  4, 1,  0, 0}, // #633
  {DBGFIELD("FSUB_D32_FSUB_D64")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #634
  {DBGFIELD("FSUB_PS64")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #635
  {DBGFIELD("FSUB_S")             1, false, false, false, 14, 2,  4, 1,  0, 0}, // #636
  {DBGFIELD("TRUNC_L_D64_TRUNC_L_S_TRUNC_W_D32_TRUNC_W_D64_TRUNC_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #637
  {DBGFIELD("CVT_D32_S_CVT_D32_W_CVT_D64_L_CVT_D64_S_CVT_D64_W_CVT_L_D64_CVT_L_S_CVT_S_D32_CVT_S_D64_CVT_S_L_CVT_S_W_CVT_W_D32_CVT_W_D64_CVT_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #638
  {DBGFIELD("CVT_PS_S64_CVT_S_PL64_CVT_S_PU64") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #639
  {DBGFIELD("C_EQ_D32_C_EQ_D64_C_F_D32_C_F_D64_C_LE_D32_C_LE_D64_C_LT_D32_C_LT_D64_C_NGE_D32_C_NGE_D64_C_NGLE_D32_C_NGLE_D64_C_NGL_D32_C_NGL_D64_C_NGT_D32_C_NGT_D64_C_OLE_D32_C_OLE_D64_C_OLT_D32_C_OLT_D64_C_SEQ_D32_C_SEQ_D64_C_SF_D32_C_SF_D64_C_UEQ_D32_C_UEQ_D64_C_ULE_D32_C_ULE_D64_C_ULT_D32_C_ULT_D64_C_UN_D32_C_UN_D64") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #640
  {DBGFIELD("C_EQ_S_C_F_S_C_LE_S_C_LT_S_C_NGE_S_C_NGLE_S_C_NGL_S_C_NGT_S_C_OLE_S_C_OLT_S_C_SEQ_S_C_SF_S_C_UEQ_S_C_ULE_S_C_ULT_S_C_UN_S") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #641
  {DBGFIELD("FCMP_D32_FCMP_D64")  1, false, false, false, 14, 2,  2, 1,  0, 0}, // #642
  {DBGFIELD("FCMP_S32")           1, false, false, false, 14, 2,  2, 1,  0, 0}, // #643
  {DBGFIELD("PseudoCVT_D32_W_PseudoCVT_D64_L_PseudoCVT_D64_W_PseudoCVT_S_L_PseudoCVT_S_W") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #644
  {DBGFIELD("PLL_PS64_PLU_PS64_PUL_PS64_PUU_PS64") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #645
  {DBGFIELD("FDIV_S")             1, false, false, false, 21, 2,  7, 1,  0, 0}, // #646
  {DBGFIELD("FDIV_D32_FDIV_D64")  1, false, false, false, 23, 2,  8, 1,  0, 0}, // #647
  {DBGFIELD("FSQRT_S")            1, false, false, false, 21, 2,  7, 1,  0, 0}, // #648
  {DBGFIELD("FSQRT_D32_FSQRT_D64") 1, false, false, false, 23, 2,  9, 1,  0, 0}, // #649
  {DBGFIELD("FRCP_D_FRCP_W")      1, false, false, false, 19, 2,  5, 1,  0, 0}, // #650
  {DBGFIELD("FRSQRT_D_FRSQRT_W")  1, false, false, false, 19, 2,  5, 1,  0, 0}, // #651
  {DBGFIELD("RECIP_D32_RECIP_D64") 1, false, false, false, 25, 2, 10, 1,  0, 0}, // #652
  {DBGFIELD("RSQRT_D32_RSQRT_D64") 1, false, false, false, 25, 2, 10, 1,  0, 0}, // #653
  {DBGFIELD("RECIP_S")            1, false, false, false, 27, 2, 11, 1,  0, 0}, // #654
  {DBGFIELD("RSQRT_S")            1, false, false, false, 27, 2, 11, 1,  0, 0}, // #655
  {DBGFIELD("FMADD_D_FMADD_W")    1, false, false, false, 19, 2,  5, 1,  0, 0}, // #656
  {DBGFIELD("FMSUB_D_FMSUB_W")    1, false, false, false, 19, 2,  5, 1,  0, 0}, // #657
  {DBGFIELD("FDIV_W")             1, false, false, false, 19, 2,  5, 1,  0, 0}, // #658
  {DBGFIELD("FDIV_D")             1, false, false, false, 19, 2,  5, 1,  0, 0}, // #659
  {DBGFIELD("FSQRT_W")            1, false, false, false, 19, 2,  5, 1,  0, 0}, // #660
  {DBGFIELD("FSQRT_D")            1, false, false, false, 19, 2,  5, 1,  0, 0}, // #661
  {DBGFIELD("FMUL_D_FMUL_W")      1, false, false, false, 19, 2,  5, 1,  0, 0}, // #662
  {DBGFIELD("FADD_D_FADD_W")      1, false, false, false, 19, 2,  5, 1,  0, 0}, // #663
  {DBGFIELD("FSUB_D_FSUB_W")      1, false, false, false, 19, 2,  5, 1,  0, 0}, // #664
  {DBGFIELD("DPADD_S_D_DPADD_S_H_DPADD_S_W_DPADD_U_D_DPADD_U_H_DPADD_U_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #665
  {DBGFIELD("DPSUB_S_D_DPSUB_S_H_DPSUB_S_W_DPSUB_U_D_DPSUB_U_H_DPSUB_U_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #666
  {DBGFIELD("DOTP_S_D_DOTP_S_H_DOTP_S_W_DOTP_U_D_DOTP_U_H_DOTP_U_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #667
  {DBGFIELD("MSUBV_B_MSUBV_D_MSUBV_H_MSUBV_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #668
  {DBGFIELD("MADDV_B_MADDV_D_MADDV_H_MADDV_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #669
  {DBGFIELD("MULV_B_MULV_D_MULV_H_MULV_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #670
  {DBGFIELD("MADDR_Q_H_MADDR_Q_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #671
  {DBGFIELD("MADD_Q_H_MADD_Q_W")  1, false, false, false, 14, 2,  5, 1,  0, 0}, // #672
  {DBGFIELD("MSUBR_Q_H_MSUBR_Q_W") 1, false, false, false, 14, 2,  5, 1,  0, 0}, // #673
  {DBGFIELD("MSUB_Q_H_MSUB_Q_W")  1, false, false, false, 14, 2,  5, 1,  0, 0}, // #674
  {DBGFIELD("MULR_Q_H_MULR_Q_W")  1, false, false, false, 14, 2,  5, 1,  0, 0}, // #675
  {DBGFIELD("MUL_Q_H_MUL_Q_W")    1, false, false, false, 14, 2,  5, 1,  0, 0}, // #676
  {DBGFIELD("MADD_D32_MADD_D64")  1, false, false, false, 19, 2,  5, 1,  0, 0}, // #677
  {DBGFIELD("MADD_S")             1, false, false, false, 14, 2,  4, 1,  0, 0}, // #678
  {DBGFIELD("MSUB_D32_MSUB_D64")  1, false, false, false, 19, 2,  5, 1,  0, 0}, // #679
  {DBGFIELD("MSUB_S")             1, false, false, false, 14, 2,  4, 1,  0, 0}, // #680
  {DBGFIELD("NMADD_D32_NMADD_D64") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #681
  {DBGFIELD("NMADD_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #682
  {DBGFIELD("NMSUB_D32_NMSUB_D64") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #683
  {DBGFIELD("NMSUB_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #684
  {DBGFIELD("CTC1")               1, false, false, false, 16, 2,  2, 1,  0, 0}, // #685
  {DBGFIELD("MTC1_MTC1_D64_BuildPairF64_BuildPairF64_64") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #686
  {DBGFIELD("MTHC1_D32_MTHC1_D64") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #687
  {DBGFIELD("COPY_U_B_COPY_U_H_COPY_U_W") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #688
  {DBGFIELD("COPY_S_B_COPY_S_D_COPY_S_H_COPY_S_W") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #689
  {DBGFIELD("BC1F")               1, false, false, false, 14, 2,  2, 1,  0, 0}, // #690
  {DBGFIELD("BC1FL")              1, false, false, false, 14, 2,  2, 1,  0, 0}, // #691
  {DBGFIELD("BC1T")               1, false, false, false, 14, 2,  2, 1,  0, 0}, // #692
  {DBGFIELD("BC1TL")              1, false, false, false, 14, 2,  2, 1,  0, 0}, // #693
  {DBGFIELD("CFC1")               1, false, false, false, 16, 2,  2, 1,  0, 0}, // #694
  {DBGFIELD("MFC1_MFC1_D64_ExtractElementF64_ExtractElementF64_64") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #695
  {DBGFIELD("MFHC1_D32_MFHC1_D64") 1, false, false, false, 16, 2,  2, 1,  0, 0}, // #696
  {DBGFIELD("MOVF_I")             1, false, false, false, 16, 2,  4, 1,  0, 0}, // #697
  {DBGFIELD("MOVT_I")             1, false, false, false, 16, 2,  4, 1,  0, 0}, // #698
  {DBGFIELD("SDC1_SDC164")        1, false, false, false, 29, 2,  1, 1,  0, 0}, // #699
  {DBGFIELD("SDXC1_SDXC164")      1, false, false, false, 29, 2,  1, 1,  0, 0}, // #700
  {DBGFIELD("SWC1")               1, false, false, false, 29, 2,  1, 1,  0, 0}, // #701
  {DBGFIELD("SWXC1")              1, false, false, false, 29, 2,  1, 1,  0, 0}, // #702
  {DBGFIELD("SUXC1_SUXC164")      1, false, false, false, 29, 2,  1, 1,  0, 0}, // #703
  {DBGFIELD("ST_B_ST_D_ST_H_ST_W") 1, false, false, false, 29, 2,  1, 1,  0, 0}, // #704
  {DBGFIELD("ST_F16")             1, false, false, false, 29, 2,  1, 1,  0, 0}, // #705
  {DBGFIELD("MOVN_I_D32_MOVN_I_D64") 1, false, false, false, 16, 2,  4, 1,  0, 0}, // #706
  {DBGFIELD("MOVN_I_S")           1, false, false, false, 16, 2,  4, 1,  0, 0}, // #707
  {DBGFIELD("MOVZ_I_D32_MOVZ_I_D64") 1, false, false, false, 16, 2,  4, 1,  0, 0}, // #708
  {DBGFIELD("MOVZ_I_S")           1, false, false, false, 16, 2,  4, 1,  0, 0}, // #709
  {DBGFIELD("LDC1_LDC164")        1, false, false, false, 31, 2,  2, 1,  0, 0}, // #710
  {DBGFIELD("LDXC1_LDXC164")      1, false, false, false, 31, 2,  2, 1,  0, 0}, // #711
  {DBGFIELD("LWC1")               1, false, false, false, 31, 2,  2, 1,  0, 0}, // #712
  {DBGFIELD("LWXC1")              1, false, false, false, 31, 2,  2, 1,  0, 0}, // #713
  {DBGFIELD("LUXC1_LUXC164")      1, false, false, false, 31, 2,  2, 1,  0, 0}, // #714
  {DBGFIELD("LD_B_LD_D_LD_H_LD_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #715
  {DBGFIELD("LD_F16")             1, false, false, false, 31, 2,  2, 1,  0, 0}, // #716
  {DBGFIELD("CEIL_L_D64_CEIL_L_S_CEIL_W_D32_CEIL_W_D64_CEIL_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #717
  {DBGFIELD("FLOOR_L_D64_FLOOR_L_S_FLOOR_W_D32_FLOOR_W_D64_FLOOR_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #718
  {DBGFIELD("ROUND_L_D64_ROUND_L_S_ROUND_W_D32_ROUND_W_D64_ROUND_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #719
  {DBGFIELD("ROTRV")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #720
  {DBGFIELD("ATOMIC_SWAP_I16_POSTRA_ATOMIC_SWAP_I32_POSTRA_ATOMIC_SWAP_I64_POSTRA_ATOMIC_SWAP_I8_POSTRA") 1, false, false, false, 33, 1,  2, 1,  0, 0}, // #721
  {DBGFIELD("ATOMIC_CMP_SWAP_I16_POSTRA_ATOMIC_CMP_SWAP_I32_POSTRA_ATOMIC_CMP_SWAP_I64_POSTRA_ATOMIC_CMP_SWAP_I8_POSTRA") 1, false, false, false, 33, 1,  2, 1,  0, 0}, // #722
  {DBGFIELD("ATOMIC_LOAD_ADD_I16_POSTRA_ATOMIC_LOAD_ADD_I32_POSTRA_ATOMIC_LOAD_ADD_I64_POSTRA_ATOMIC_LOAD_ADD_I8_POSTRA_ATOMIC_LOAD_AND_I16_POSTRA_ATOMIC_LOAD_AND_I32_POSTRA_ATOMIC_LOAD_AND_I64_POSTRA_ATOMIC_LOAD_AND_I8_POSTRA_ATOMIC_LOAD_MAX_I16_POSTRA_ATOMIC_LOAD_MAX_I32_POSTRA_ATOMIC_LOAD_MAX_I64_POSTRA_ATOMIC_LOAD_MAX_I8_POSTRA_ATOMIC_LOAD_MIN_I16_POSTRA_ATOMIC_LOAD_MIN_I32_POSTRA_ATOMIC_LOAD_MIN_I64_POSTRA_ATOMIC_LOAD_MIN_I8_POSTRA_ATOMIC_LOAD_NAND_I16_POSTRA_ATOMIC_LOAD_NAND_I32_POSTRA_ATOMIC_LOAD_NAND_I64_POSTRA_ATOMIC_LOAD_NAND_I8_POSTRA_ATOMIC_LOAD_OR_I16_POSTRA_ATOMIC_LOAD_OR_I32_POSTRA_ATOMIC_LOAD_OR_I64_POSTRA_ATOMIC_LOAD_OR_I8_POSTRA_ATOMIC_LOAD_SUB_I16_POSTRA_ATOMIC_LOAD_SUB_I32_POSTRA_ATOMIC_LOAD_SUB_I64_POSTRA_ATOMIC_LOAD_SUB_I8_POSTRA_ATOMIC_LOAD_UMAX_I16_POSTRA_ATOMIC_LOAD_UMAX_I32_POSTRA_ATOMIC_LOAD_UMAX_I64_POSTRA_ATOMIC_LOAD_UMAX_I8_POSTRA_ATOMIC_LOAD_UMIN_I16_POSTRA_ATOMIC_LOAD_UMIN_I32_POSTRA_ATOMIC_LOAD_UMIN_I64_POSTRA_ATOMIC_LOAD_UMIN_I8_POSTRA_ATOMIC_LOAD_XOR_I16_POSTRA_ATOMIC_LOAD_XOR_I32_POSTRA_ATOMIC_LOAD_XOR_I64_POSTRA_ATOMIC_LOAD_XOR_I8_POSTRA") 1, false, false, false, 33, 1,  2, 1,  0, 0}, // #723
  {DBGFIELD("LEA_ADDiu")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #724
  {DBGFIELD("ADDIUPC")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #725
  {DBGFIELD("ALIGN")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #726
  {DBGFIELD("ALUIPC")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #727
  {DBGFIELD("AUI")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #728
  {DBGFIELD("AUIPC")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #729
  {DBGFIELD("BITSWAP")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #730
  {DBGFIELD("CLO_R6")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #731
  {DBGFIELD("CLZ_R6")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #732
  {DBGFIELD("LSA_R6")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #733
  {DBGFIELD("SELEQZ_SELNEZ")      1, false, false, false,  1, 2,  1, 1,  0, 0}, // #734
  {DBGFIELD("AddiuRxImmX16_AddiuRxRxImm16_AddiuRxRxImmX16_AddiuRxRyOffMemX16_AddiuRxPcImmX16_AddiuSpImm16_AddiuSpImmX16_AdduRxRyRz16_AndRxRxRy16_CmpRxRy16_CmpiRxImm16_CmpiRxImmX16_LiRxImm16_LiRxImmX16_LiRxImmAlignX16_Move32R16_MoveR3216_Mfhi16_Mflo16_NegRxRy16_NotRxRy16_OrRxRxRy16_SebRx16_SehRx16_SllX16_SllvRxRy16_SltiRxImm16_SltiRxImmX16_SltiuRxImm16_SltiuRxImmX16_SltRxRy16_SltuRxRy16_SravRxRy16_SraX16_SrlvRxRy16_SrlX16_SubuRxRyRz16_XorRxRxRy16") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #735
  {DBGFIELD("SltiCCRxImmX16_SltiuCCRxImmX16_SltCCRxRy16_SltuRxRyRz16_SltuCCRxRy16") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #736
  {DBGFIELD("Constant32_LwConstant32_GotPrologue16_CONSTPOOL_ENTRY") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #737
  {DBGFIELD("ADDIUPC_MM_ADDIUR1SP_MM_ADDIUR2_MM_ADDIUS5_MM_ADDIUSP_MM_ADDiu_MM_LEA_ADDiu_MM") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #738
  {DBGFIELD("ADDU16_MM_ADDu_MM")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #739
  {DBGFIELD("ADD_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #740
  {DBGFIELD("ADDi_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #741
  {DBGFIELD("AND16_MM_ANDI16_MM_AND_MM") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #742
  {DBGFIELD("ANDi_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #743
  {DBGFIELD("CLO_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #744
  {DBGFIELD("CLZ_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #745
  {DBGFIELD("EXT_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #746
  {DBGFIELD("INS_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #747
  {DBGFIELD("LI16_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #748
  {DBGFIELD("LUi_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #749
  {DBGFIELD("MOVE16_MM")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #750
  {DBGFIELD("MOVEP_MM")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #751
  {DBGFIELD("NOR_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #752
  {DBGFIELD("NOT16_MM")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #753
  {DBGFIELD("OR16_MM_OR_MM")      1, false, false, false,  1, 2,  1, 1,  0, 0}, // #754
  {DBGFIELD("ORi_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #755
  {DBGFIELD("ROTRV_MM")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #756
  {DBGFIELD("ROTR_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #757
  {DBGFIELD("SEB_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #758
  {DBGFIELD("SEH_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #759
  {DBGFIELD("SLL16_MM_SLL_MM")    1, false, false, false,  1, 2,  1, 1,  0, 0}, // #760
  {DBGFIELD("SLLV_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #761
  {DBGFIELD("SLT_MM_SLTu_MM")     1, false, false, false,  1, 2,  1, 1,  0, 0}, // #762
  {DBGFIELD("SLTi_MM_SLTiu_MM")   1, false, false, false,  1, 2,  1, 1,  0, 0}, // #763
  {DBGFIELD("SRAV_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #764
  {DBGFIELD("SRA_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #765
  {DBGFIELD("SRL16_MM_SRL_MM")    1, false, false, false,  1, 2,  1, 1,  0, 0}, // #766
  {DBGFIELD("SRLV_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #767
  {DBGFIELD("SSNOP_MM")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #768
  {DBGFIELD("SUBU16_MM_SUBu_MM")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #769
  {DBGFIELD("SUB_MM")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #770
  {DBGFIELD("WSBH_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #771
  {DBGFIELD("XOR16_MM_XOR_MM")    1, false, false, false,  1, 2,  1, 1,  0, 0}, // #772
  {DBGFIELD("XORi_MM")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #773
  {DBGFIELD("ADDIUPC_MMR6")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #774
  {DBGFIELD("ADDIU_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #775
  {DBGFIELD("ADDU16_MMR6_ADDU_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #776
  {DBGFIELD("ADD_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #777
  {DBGFIELD("ALIGN_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #778
  {DBGFIELD("ALUIPC_MMR6")        1, false, false, false,  1, 2,  1, 1,  0, 0}, // #779
  {DBGFIELD("AND16_MMR6_ANDI16_MMR6_AND_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #780
  {DBGFIELD("ANDI_MMR6")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #781
  {DBGFIELD("AUIPC_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #782
  {DBGFIELD("AUI_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #783
  {DBGFIELD("BITSWAP_MMR6")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #784
  {DBGFIELD("CLO_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #785
  {DBGFIELD("CLZ_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #786
  {DBGFIELD("EXT_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #787
  {DBGFIELD("INS_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #788
  {DBGFIELD("LI16_MMR6")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #789
  {DBGFIELD("LSA_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #790
  {DBGFIELD("LUI_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #791
  {DBGFIELD("MOVE16_MMR6")        1, false, false, false,  1, 2,  1, 1,  0, 0}, // #792
  {DBGFIELD("NOR_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #793
  {DBGFIELD("NOT16_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #794
  {DBGFIELD("OR16_MMR6_OR_MMR6")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #795
  {DBGFIELD("ORI_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #796
  {DBGFIELD("SELEQZ_MMR6_SELNEZ_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #797
  {DBGFIELD("SLL16_MMR6_SLL_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #798
  {DBGFIELD("SRL16_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #799
  {DBGFIELD("SSNOP_MMR6")         1, false, false, false,  1, 2,  1, 1,  0, 0}, // #800
  {DBGFIELD("SUBU16_MMR6_SUBU_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #801
  {DBGFIELD("SUB_MMR6")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #802
  {DBGFIELD("WSBH_MMR6")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #803
  {DBGFIELD("XOR16_MMR6_XOR_MMR6") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #804
  {DBGFIELD("XORI_MMR6")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #805
  {DBGFIELD("AND64_ANDi64")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #806
  {DBGFIELD("DEXT64_32")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #807
  {DBGFIELD("DSLL64_32")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #808
  {DBGFIELD("ORi64")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #809
  {DBGFIELD("SEB64")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #810
  {DBGFIELD("SEH64")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #811
  {DBGFIELD("SLL64_32_SLL64_64")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #812
  {DBGFIELD("SLT64_SLTu64")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #813
  {DBGFIELD("SLTi64_SLTiu64")     1, false, false, false,  1, 2,  1, 1,  0, 0}, // #814
  {DBGFIELD("XOR64_XORi64")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #815
  {DBGFIELD("DADD")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #816
  {DBGFIELD("DADDi")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #817
  {DBGFIELD("DADDiu")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #818
  {DBGFIELD("DADDu")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #819
  {DBGFIELD("DCLO")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #820
  {DBGFIELD("DCLZ")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #821
  {DBGFIELD("DEXT_DEXTM_DEXTU")   1, false, false, false,  1, 2,  1, 1,  0, 0}, // #822
  {DBGFIELD("DINS_DINSM_DINSU")   1, false, false, false,  1, 2,  1, 1,  0, 0}, // #823
  {DBGFIELD("DROTR")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #824
  {DBGFIELD("DROTR32")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #825
  {DBGFIELD("DROTRV")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #826
  {DBGFIELD("DSBH")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #827
  {DBGFIELD("DSHD")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #828
  {DBGFIELD("DSLL")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #829
  {DBGFIELD("DSLL32")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #830
  {DBGFIELD("DSLLV")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #831
  {DBGFIELD("DSRA")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #832
  {DBGFIELD("DSRA32")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #833
  {DBGFIELD("DSRAV")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #834
  {DBGFIELD("DSRL")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #835
  {DBGFIELD("DSRL32")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #836
  {DBGFIELD("DSRLV")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #837
  {DBGFIELD("DSUB")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #838
  {DBGFIELD("DSUBu")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #839
  {DBGFIELD("LEA_ADDiu64")        1, false, false, false,  1, 2,  1, 1,  0, 0}, // #840
  {DBGFIELD("LUi64")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #841
  {DBGFIELD("NOR64")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #842
  {DBGFIELD("OR64")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #843
  {DBGFIELD("DALIGN")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #844
  {DBGFIELD("DAHI")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #845
  {DBGFIELD("DATI")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #846
  {DBGFIELD("DAUI")               1, false, false, false,  1, 2,  1, 1,  0, 0}, // #847
  {DBGFIELD("DCLO_R6")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #848
  {DBGFIELD("DCLZ_R6")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #849
  {DBGFIELD("DBITSWAP")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #850
  {DBGFIELD("DLSA_DLSA_R6")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #851
  {DBGFIELD("SELEQZ64_SELNEZ64")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #852
  {DBGFIELD("MADD")               1, false, false, false, 12, 2,  1, 1,  0, 0}, // #853
  {DBGFIELD("MADDU")              1, false, false, false, 12, 2,  1, 1,  0, 0}, // #854
  {DBGFIELD("MSUB")               1, false, false, false, 12, 2,  1, 1,  0, 0}, // #855
  {DBGFIELD("MSUBU")              1, false, false, false, 12, 2,  1, 1,  0, 0}, // #856
  {DBGFIELD("PseudoMADD_MM")      1, false, false, false, 12, 2,  1, 1,  0, 0}, // #857
  {DBGFIELD("PseudoMADDU_MM")     1, false, false, false, 12, 2,  1, 1,  0, 0}, // #858
  {DBGFIELD("PseudoMSUB_MM")      1, false, false, false, 12, 2,  1, 1,  0, 0}, // #859
  {DBGFIELD("PseudoMSUBU_MM")     1, false, false, false, 12, 2,  1, 1,  0, 0}, // #860
  {DBGFIELD("PseudoMULT_MM")      1, false, false, false, 12, 2,  1, 1,  0, 0}, // #861
  {DBGFIELD("PseudoMULTu_MM")     1, false, false, false, 12, 2,  1, 1,  0, 0}, // #862
  {DBGFIELD("PseudoMULT")         1, false, false, false, 12, 2,  1, 1,  0, 0}, // #863
  {DBGFIELD("PseudoMULTu")        1, false, false, false, 12, 2,  1, 1,  0, 0}, // #864
  {DBGFIELD("PseudoSDIV_SDIV")    1, false, false, false, 10, 2,  6, 1,  0, 0}, // #865
  {DBGFIELD("PseudoUDIV_UDIV")    1, false, false, false, 34, 2, 12, 1,  0, 0}, // #866
  {DBGFIELD("PseudoMFHI_MM_PseudoMFLO_MM") 1, false, false, false,  1, 2,  5, 1,  0, 0}, // #867
  {DBGFIELD("PseudoMTLOHI_MM")    1, false, false, false,  1, 2,  2, 1,  0, 0}, // #868
  {DBGFIELD("MUH")                1, false, false, false, 12, 2,  4, 1,  0, 0}, // #869
  {DBGFIELD("MUHU")               1, false, false, false, 12, 2,  4, 1,  0, 0}, // #870
  {DBGFIELD("MULU")               1, false, false, false, 12, 2,  4, 1,  0, 0}, // #871
  {DBGFIELD("MUL_R6")             1, false, false, false, 12, 2,  4, 1,  0, 0}, // #872
  {DBGFIELD("MOD")                1, false, false, false, 10, 2,  6, 1,  0, 0}, // #873
  {DBGFIELD("MODU")               1, false, false, false, 10, 2,  6, 1,  0, 0}, // #874
  {DBGFIELD("MultRxRy16_MultuRxRy16_MultRxRyRz16_MultuRxRyRz16") 1, false, false, false, 12, 2,  1, 1,  0, 0}, // #875
  {DBGFIELD("DivRxRy16")          1, false, false, false, 10, 2,  6, 1,  0, 0}, // #876
  {DBGFIELD("DivuRxRy16")         1, false, false, false, 34, 2, 12, 1,  0, 0}, // #877
  {DBGFIELD("MULT_MM")            1, false, false, false, 12, 2,  4, 1,  0, 0}, // #878
  {DBGFIELD("MULTu_MM")           1, false, false, false, 12, 2,  4, 1,  0, 0}, // #879
  {DBGFIELD("MADD_MM")            1, false, false, false, 12, 2,  4, 1,  0, 0}, // #880
  {DBGFIELD("MADDU_MM")           1, false, false, false, 12, 2,  4, 1,  0, 0}, // #881
  {DBGFIELD("MSUB_MM")            1, false, false, false, 12, 2,  4, 1,  0, 0}, // #882
  {DBGFIELD("MSUBU_MM")           1, false, false, false, 12, 2,  4, 1,  0, 0}, // #883
  {DBGFIELD("MUL_MM")             1, false, false, false,  1, 2,  5, 1,  0, 0}, // #884
  {DBGFIELD("SDIV_MM_SDIV_MM_Pseudo") 1, false, false, false, 10, 2,  6, 1,  0, 0}, // #885
  {DBGFIELD("UDIV_MM_UDIV_MM_Pseudo") 1, false, false, false, 34, 2, 12, 1,  0, 0}, // #886
  {DBGFIELD("MFHI16_MM_MFLO16_MM_MFHI_MM_MFLO_MM") 1, false, false, false,  1, 2,  2, 1,  0, 0}, // #887
  {DBGFIELD("MOVF_I_MM")          1, false, false, false,  1, 2,  2, 1,  0, 0}, // #888
  {DBGFIELD("MOVT_I_MM")          1, false, false, false,  1, 2,  2, 1,  0, 0}, // #889
  {DBGFIELD("MTHI_MM_MTLO_MM")    1, false, false, false,  1, 2,  2, 1,  0, 0}, // #890
  {DBGFIELD("RDHWR_MM")           1, false, false, false,  1, 2,  2, 1,  0, 0}, // #891
  {DBGFIELD("MUHU_MMR6")          1, false, false, false, 12, 2,  4, 1,  0, 0}, // #892
  {DBGFIELD("MUH_MMR6")           1, false, false, false, 12, 2,  4, 1,  0, 0}, // #893
  {DBGFIELD("MULU_MMR6")          1, false, false, false, 12, 2,  4, 1,  0, 0}, // #894
  {DBGFIELD("MUL_MMR6")           1, false, false, false, 12, 2,  4, 1,  0, 0}, // #895
  {DBGFIELD("MODU_MMR6")          1, false, false, false, 10, 2,  6, 1,  0, 0}, // #896
  {DBGFIELD("MOD_MMR6")           1, false, false, false, 10, 2,  6, 1,  0, 0}, // #897
  {DBGFIELD("DIVU_MMR6")          1, false, false, false, 10, 2,  6, 1,  0, 0}, // #898
  {DBGFIELD("DIV_MMR6")           1, false, false, false, 10, 2,  6, 1,  0, 0}, // #899
  {DBGFIELD("RDHWR_MMR6")         1, false, false, false,  1, 2,  2, 1,  0, 0}, // #900
  {DBGFIELD("DMULU")              1, false, false, false, 12, 2,  1, 1,  0, 0}, // #901
  {DBGFIELD("DMULT_PseudoDMULT")  1, false, false, false, 12, 2,  1, 1,  0, 0}, // #902
  {DBGFIELD("DMULTu_PseudoDMULTu") 1, false, false, false, 12, 2,  1, 1,  0, 0}, // #903
  {DBGFIELD("DSDIV_PseudoDSDIV")  1, false, false, false, 10, 2,  6, 1,  0, 0}, // #904
  {DBGFIELD("DUDIV_PseudoDUDIV")  1, false, false, false, 34, 2, 12, 1,  0, 0}, // #905
  {DBGFIELD("MFHI64_MFLO64_PseudoMFHI64_PseudoMFLO64") 1, false, false, false,  1, 2,  5, 1,  0, 0}, // #906
  {DBGFIELD("PseudoMTLOHI64")     1, false, false, false,  1, 2,  5, 1,  0, 0}, // #907
  {DBGFIELD("MTHI64_MTLO64")      1, false, false, false,  1, 2,  2, 1,  0, 0}, // #908
  {DBGFIELD("RDHWR64")            1, false, false, false,  1, 2,  2, 1,  0, 0}, // #909
  {DBGFIELD("MOVN_I_I64_MOVN_I64_I_MOVN_I64_I64") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #910
  {DBGFIELD("MOVZ_I_I64_MOVZ_I64_I_MOVZ_I64_I64") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #911
  {DBGFIELD("DMUH")               1, false, false, false, 12, 2,  5, 1,  0, 0}, // #912
  {DBGFIELD("DMUHU")              1, false, false, false, 12, 2,  5, 1,  0, 0}, // #913
  {DBGFIELD("DMUL_R6")            1, false, false, false, 12, 2,  5, 1,  0, 0}, // #914
  {DBGFIELD("DDIV")               1, false, false, false, 10, 2,  6, 1,  0, 0}, // #915
  {DBGFIELD("DMOD")               1, false, false, false, 10, 2,  6, 1,  0, 0}, // #916
  {DBGFIELD("DDIVU")              1, false, false, false, 34, 2, 12, 1,  0, 0}, // #917
  {DBGFIELD("DMODU")              1, false, false, false, 34, 2, 12, 1,  0, 0}, // #918
  {DBGFIELD("BAL_BR_BLTZAL")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #919
  {DBGFIELD("BEQ_BNE")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #920
  {DBGFIELD("BGTZ_BGEZ_BLEZ_BLTZ") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #921
  {DBGFIELD("J")                  1, false, false, false,  3, 2,  1, 1,  0, 0}, // #922
  {DBGFIELD("JR")                 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #923
  {DBGFIELD("ERet")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #924
  {DBGFIELD("BGEZAL")             1, false, false, false,  3, 2,  2, 1,  0, 0}, // #925
  {DBGFIELD("BALC")               1, false, false, false,  3, 2,  2, 1,  0, 0}, // #926
  {DBGFIELD("BEQZALC_BGEZALC_BGTZALC_BLEZALC_BLTZALC_BNEZALC") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #927
  {DBGFIELD("JIALC")              1, false, false, false,  3, 2,  2, 1,  0, 0}, // #928
  {DBGFIELD("BC")                 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #929
  {DBGFIELD("BC2EQZ_BC2NEZ")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #930
  {DBGFIELD("BEQC_BGEC_BGEUC_BLTC_BLTUC_BNEC_BNVC_BOVC") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #931
  {DBGFIELD("BEQZC_BGEZC_BGTZC_BLEZC_BLTZC_BNEZC") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #932
  {DBGFIELD("JIC")                1, false, false, false,  3, 2,  1, 1,  0, 0}, // #933
  {DBGFIELD("JR_HB_R6")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #934
  {DBGFIELD("SIGRIE")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #935
  {DBGFIELD("PseudoIndirectBranchR6_PseudoIndrectHazardBranchR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #936
  {DBGFIELD("TAILCALLR6REG_TAILCALLHBR6REG") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #937
  {DBGFIELD("SDBBP_R6")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #938
  {DBGFIELD("Bimm16_BimmX16_BeqzRxImm16_BeqzRxImmX16_BnezRxImm16_BnezRxImmX16_Bteqz16_BteqzX16_Btnez16_BtnezX16_JrRa16_JrcRa16_JrcRx16") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #939
  {DBGFIELD("BteqzT8CmpX16_BteqzT8CmpiX16_BteqzT8SltX16_BteqzT8SltuX16_BteqzT8SltiX16_BteqzT8SltiuX16_BtnezT8CmpX16_BtnezT8CmpiX16_BtnezT8SltX16_BtnezT8SltuX16_BtnezT8SltiX16_BtnezT8SltiuX16_RetRA16") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #940
  {DBGFIELD("Jal16_JalB16")       1, false, false, false,  3, 2,  2, 1,  0, 0}, // #941
  {DBGFIELD("JumpLinkReg16")      1, false, false, false,  3, 2,  2, 1,  0, 0}, // #942
  {DBGFIELD("Break16")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #943
  {DBGFIELD("SelBeqZ_SelTBteqZCmp_SelTBteqZCmpi_SelTBteqZSlt_SelTBteqZSlti_SelTBteqZSltu_SelTBteqZSltiu_SelBneZ_SelTBtneZCmp_SelTBtneZCmpi_SelTBtneZSlt_SelTBtneZSlti_SelTBtneZSltu_SelTBtneZSltiu") 1, false, false, false,  1, 2,  5, 1,  0, 0}, // #944
  {DBGFIELD("B16_MM_B_MM")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #945
  {DBGFIELD("BAL_BR_MM")          1, false, false, false,  3, 2,  1, 1,  0, 0}, // #946
  {DBGFIELD("BC1F_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #947
  {DBGFIELD("BC1T_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #948
  {DBGFIELD("BEQZ16_MM_BGEZ_MM_BGTZ_MM_BLEZ_MM_BLTZ_MM_BNEZ16_MM") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #949
  {DBGFIELD("BEQZC_MM_BNEZC_MM")  1, false, false, false,  3, 2,  1, 1,  0, 0}, // #950
  {DBGFIELD("BEQ_MM_BNE_MM")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #951
  {DBGFIELD("DERET_MM")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #952
  {DBGFIELD("ERET_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #953
  {DBGFIELD("JR16_MM_JR_MM")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #954
  {DBGFIELD("J_MM")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #955
  {DBGFIELD("B_MM_Pseudo")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #956
  {DBGFIELD("BGEZALS_MM_BLTZALS_MM") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #957
  {DBGFIELD("BGEZAL_MM_BLTZAL_MM") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #958
  {DBGFIELD("JALR16_MM_JALR_MM")  1, false, false, false,  3, 2,  2, 1,  0, 0}, // #959
  {DBGFIELD("JALRS16_MM_JALRS_MM") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #960
  {DBGFIELD("JALS_MM")            1, false, false, false,  3, 2,  2, 1,  0, 0}, // #961
  {DBGFIELD("JALX_MM_JAL_MM")     1, false, false, false,  3, 2,  2, 1,  0, 0}, // #962
  {DBGFIELD("TAILCALLREG_MM")     1, false, false, false,  3, 2,  1, 1,  0, 0}, // #963
  {DBGFIELD("TAILCALL_MM")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #964
  {DBGFIELD("PseudoIndirectBranch_MM") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #965
  {DBGFIELD("BREAK16_MM_BREAK_MM") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #966
  {DBGFIELD("SDBBP16_MM_SDBBP_MM") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #967
  {DBGFIELD("SYSCALL_MM")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #968
  {DBGFIELD("TEQI_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #969
  {DBGFIELD("TEQ_MM")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #970
  {DBGFIELD("TGEIU_MM")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #971
  {DBGFIELD("TGEI_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #972
  {DBGFIELD("TGEU_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #973
  {DBGFIELD("TGE_MM")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #974
  {DBGFIELD("TLTIU_MM")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #975
  {DBGFIELD("TLTI_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #976
  {DBGFIELD("TLTU_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #977
  {DBGFIELD("TLT_MM")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #978
  {DBGFIELD("TNEI_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #979
  {DBGFIELD("TNE_MM")             1, false, false, false,  3, 2,  1, 1,  0, 0}, // #980
  {DBGFIELD("TRAP_MM")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #981
  {DBGFIELD("BC16_MMR6_BC_MMR6")  1, false, false, false,  3, 2,  1, 1,  0, 0}, // #982
  {DBGFIELD("BC1EQZC_MMR6_BC1NEZC_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #983
  {DBGFIELD("BC2EQZC_MMR6_BC2NEZC_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #984
  {DBGFIELD("BEQC_MMR6_BGEC_MMR6_BGEUC_MMR6_BLTC_MMR6_BLTUC_MMR6_BNEC_MMR6_BNVC_MMR6_BOVC_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #985
  {DBGFIELD("BEQZC16_MMR6_BNEZC16_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #986
  {DBGFIELD("BEQZC_MMR6_BGEZC_MMR6_BGTZC_MMR6_BLEZC_MMR6_BLTZC_MMR6_BNEZC_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #987
  {DBGFIELD("DERET_MMR6")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #988
  {DBGFIELD("ERETNC_MMR6")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #989
  {DBGFIELD("JAL_MMR6")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #990
  {DBGFIELD("ERET_MMR6")          1, false, false, false,  3, 2,  1, 1,  0, 0}, // #991
  {DBGFIELD("JIC_MMR6")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #992
  {DBGFIELD("JRADDIUSP_JRCADDIUSP_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #993
  {DBGFIELD("JRC16_MM")           1, false, false, false,  3, 2,  1, 1,  0, 0}, // #994
  {DBGFIELD("JRC16_MMR6")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #995
  {DBGFIELD("SIGRIE_MMR6")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #996
  {DBGFIELD("B_MMR6_Pseudo")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #997
  {DBGFIELD("PseudoIndirectBranch_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #998
  {DBGFIELD("BALC_MMR6")          1, false, false, false,  3, 2,  2, 1,  0, 0}, // #999
  {DBGFIELD("BEQZALC_MMR6_BGEZALC_MMR6_BGTZALC_MMR6_BLEZALC_MMR6_BLTZALC_MMR6_BNEZALC_MMR6") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1000
  {DBGFIELD("JALRC16_MMR6")       1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1001
  {DBGFIELD("JALRC_HB_MMR6")      1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1002
  {DBGFIELD("JALRC_MMR6")         1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1003
  {DBGFIELD("JIALC_MMR6")         1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1004
  {DBGFIELD("TAILCALLREG_MMR6")   1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1005
  {DBGFIELD("TAILCALL_MMR6")      1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1006
  {DBGFIELD("BREAK16_MMR6_BREAK_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1007
  {DBGFIELD("SDBBP_MMR6_SDBBP16_MMR6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1008
  {DBGFIELD("BEQ64_BNE64")        1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1009
  {DBGFIELD("BGEZ64_BGTZ64_BLEZ64_BLTZ64") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1010
  {DBGFIELD("JR64")               1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1011
  {DBGFIELD("JALR64_JALR64Pseudo_JALRHB64Pseudo") 1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1012
  {DBGFIELD("JALR_HB64")          1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1013
  {DBGFIELD("JR_HB64")            1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1014
  {DBGFIELD("TAILCALLREG64_TAILCALLREGHB64") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1015
  {DBGFIELD("PseudoReturn64")     1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1016
  {DBGFIELD("BEQC64_BGEC64_BGEUC64_BLTC64_BLTUC64_BNEC64") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1017
  {DBGFIELD("BEQZC64_BGEZC64_BGTZC64_BLEZC64_BLTZC64_BNEZC64") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1018
  {DBGFIELD("JIC64")              1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1019
  {DBGFIELD("PseudoIndirectBranch64_PseudoIndirectHazardBranch64") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1020
  {DBGFIELD("JIALC64")            1, false, false, false,  3, 2,  2, 1,  0, 0}, // #1021
  {DBGFIELD("JR_HB64_R6")         1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1022
  {DBGFIELD("TAILCALL64R6REG_TAILCALLHB64R6REG") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1023
  {DBGFIELD("PseudoIndirectBranch64R6_PseudoIndrectHazardBranch64R6") 1, false, false, false,  3, 2,  1, 1,  0, 0}, // #1024
  {DBGFIELD("EVP")                1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1025
  {DBGFIELD("DVP")                1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1026
  {DBGFIELD("TLBP_MM")            1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1027
  {DBGFIELD("TLBR_MM")            1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1028
  {DBGFIELD("TLBWI_MM")           1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1029
  {DBGFIELD("TLBWR_MM")           1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1030
  {DBGFIELD("DI_MM")              1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1031
  {DBGFIELD("EI_MM")              1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1032
  {DBGFIELD("EHB_MM")             1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1033
  {DBGFIELD("PAUSE_MM")           1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1034
  {DBGFIELD("WAIT_MM")            1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1035
  {DBGFIELD("RDPGPR_MMR6")        1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1036
  {DBGFIELD("WRPGPR_MMR6")        1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1037
  {DBGFIELD("TLBINV_MMR6")        1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1038
  {DBGFIELD("TLBINVF_MMR6")       1, false, false, false,  5, 2,  4, 1,  0, 0}, // #1039
  {DBGFIELD("MFHC0_MMR6")         1, false, false, false,  5, 2,  2, 1,  0, 0}, // #1040
  {DBGFIELD("MFC0_MMR6")          1, false, false, false,  5, 2,  2, 1,  0, 0}, // #1041
  {DBGFIELD("MFHC2_MMR6_MFC2_MMR6") 1, false, false, false,  5, 2,  2, 1,  0, 0}, // #1042
  {DBGFIELD("MTHC0_MMR6")         1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1043
  {DBGFIELD("MTC0_MMR6")          1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1044
  {DBGFIELD("MTHC2_MMR6_MTC2_MMR6") 1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1045
  {DBGFIELD("EVP_MMR6")           1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1046
  {DBGFIELD("DVP_MMR6")           1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1047
  {DBGFIELD("DI_MMR6")            1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1048
  {DBGFIELD("EI_MMR6")            1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1049
  {DBGFIELD("EHB_MMR6")           1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1050
  {DBGFIELD("PAUSE_MMR6")         1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1051
  {DBGFIELD("WAIT_MMR6")          1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1052
  {DBGFIELD("DMFC0")              1, false, false, false,  5, 2,  2, 1,  0, 0}, // #1053
  {DBGFIELD("DMTC0")              1, false, false, false,  5, 2,  3, 1,  0, 0}, // #1054
  {DBGFIELD("DMFC2")              1, false, false, false,  7, 1,  1, 1,  0, 0}, // #1055
  {DBGFIELD("DMTC2")              1, false, false, false,  7, 1,  1, 1,  0, 0}, // #1056
  {DBGFIELD("CFC2_MM")            1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1057
  {DBGFIELD("CTC2_MM")            1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1058
  {DBGFIELD("DMT")                1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1059
  {DBGFIELD("DVPE")               1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1060
  {DBGFIELD("EMT")                1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1061
  {DBGFIELD("EVPE")               1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1062
  {DBGFIELD("MFTR")               1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1063
  {DBGFIELD("MTTR")               1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1064
  {DBGFIELD("YIELD")              1, false, false, false,  5, 2,  5, 1,  0, 0}, // #1065
  {DBGFIELD("FORK")               1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1066
  {DBGFIELD("DMFGC0")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1067
  {DBGFIELD("DMTGC0")             1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1068
  {DBGFIELD("HYPCALL_MM")         1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1069
  {DBGFIELD("TLBGINVF_MM")        1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1070
  {DBGFIELD("TLBGINV_MM")         1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1071
  {DBGFIELD("TLBGP_MM")           1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1072
  {DBGFIELD("TLBGR_MM")           1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1073
  {DBGFIELD("TLBGWI_MM")          1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1074
  {DBGFIELD("TLBGWR_MM")          1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1075
  {DBGFIELD("MFGC0_MM")           1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1076
  {DBGFIELD("MFHGC0_MM")          1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1077
  {DBGFIELD("MTGC0_MM")           1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1078
  {DBGFIELD("MTHGC0_MM")          1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1079
  {DBGFIELD("SC_MMR6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1080
  {DBGFIELD("LDC2_R6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1081
  {DBGFIELD("LL_R6")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1082
  {DBGFIELD("LWC2_R6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1083
  {DBGFIELD("SWC2_R6")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1084
  {DBGFIELD("SDC2_R6")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1085
  {DBGFIELD("SC_R6")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1086
  {DBGFIELD("PREF_R6")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1087
  {DBGFIELD("CACHE_R6")           1, false, false, false,  8, 2,  5, 1,  0, 0}, // #1088
  {DBGFIELD("GINVI")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1089
  {DBGFIELD("GINVT")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1090
  {DBGFIELD("LBE_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1091
  {DBGFIELD("LBuE_MM")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1092
  {DBGFIELD("LHE_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1093
  {DBGFIELD("LHuE_MM")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1094
  {DBGFIELD("LWE_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1095
  {DBGFIELD("LWLE_MM")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1096
  {DBGFIELD("LWRE_MM")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1097
  {DBGFIELD("LLE_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1098
  {DBGFIELD("SBE_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1099
  {DBGFIELD("SB_MM")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1100
  {DBGFIELD("SHE_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1101
  {DBGFIELD("SWE_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1102
  {DBGFIELD("SWLE_MM")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1103
  {DBGFIELD("SWRE_MM")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1104
  {DBGFIELD("SCE_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1105
  {DBGFIELD("PREFE_MM")           1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1106
  {DBGFIELD("CACHEE_MM")          1, false, false, false,  8, 2,  5, 1,  0, 0}, // #1107
  {DBGFIELD("Restore16_RestoreX16") 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1108
  {DBGFIELD("LbRxRyOffMemX16")    1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1109
  {DBGFIELD("LbuRxRyOffMemX16")   1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1110
  {DBGFIELD("LhRxRyOffMemX16")    1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1111
  {DBGFIELD("LhuRxRyOffMemX16")   1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1112
  {DBGFIELD("LwRxRyOffMemX16_LwRxSpImmX16_LwRxPcTcp16_LwRxPcTcpX16") 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1113
  {DBGFIELD("Save16_SaveX16")     1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1114
  {DBGFIELD("SbRxRyOffMemX16")    1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1115
  {DBGFIELD("ShRxRyOffMemX16")    1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1116
  {DBGFIELD("SwRxRyOffMemX16_SwRxSpImmX16") 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1117
  {DBGFIELD("LBU16_MM_LBu_MM")    1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1118
  {DBGFIELD("LB_MM")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1119
  {DBGFIELD("LHU16_MM_LHu_MM")    1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1120
  {DBGFIELD("LH_MM")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1121
  {DBGFIELD("LL_MM")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1122
  {DBGFIELD("LW16_MM_LWGP_MM_LWSP_MM_LW_MM") 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1123
  {DBGFIELD("LWL_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1124
  {DBGFIELD("LWM16_MM_LWM32_MM")  1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1125
  {DBGFIELD("LWP_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1126
  {DBGFIELD("LWR_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1127
  {DBGFIELD("LWU_MM")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1128
  {DBGFIELD("LWXS_MM")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1129
  {DBGFIELD("SB16_MM")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1130
  {DBGFIELD("SC_MM")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1131
  {DBGFIELD("SH16_MM_SH_MM")      1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1132
  {DBGFIELD("SW16_MM_SWSP_MM_SW_MM") 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1133
  {DBGFIELD("SWL_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1134
  {DBGFIELD("SWM16_MM_SWM32_MM")  1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1135
  {DBGFIELD("SWM_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1136
  {DBGFIELD("SWP_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1137
  {DBGFIELD("SWR_MM")             1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1138
  {DBGFIELD("PREF_MM_PREFX_MM")   1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1139
  {DBGFIELD("CACHE_MM")           1, false, false, false,  8, 2,  5, 1,  0, 0}, // #1140
  {DBGFIELD("SYNC_MM")            1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1141
  {DBGFIELD("SYNCI_MM")           1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1142
  {DBGFIELD("GINVI_MMR6")         1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1143
  {DBGFIELD("GINVT_MMR6")         1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1144
  {DBGFIELD("LBU_MMR6")           1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1145
  {DBGFIELD("LB_MMR6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1146
  {DBGFIELD("LDC2_MMR6")          1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1147
  {DBGFIELD("LL_MMR6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1148
  {DBGFIELD("LWM16_MMR6")         1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1149
  {DBGFIELD("LWC2_MMR6")          1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1150
  {DBGFIELD("LWPC_MMR6")          1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1151
  {DBGFIELD("LW_MMR6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1152
  {DBGFIELD("SB16_MMR6_SB_MMR6")  1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1153
  {DBGFIELD("SDC2_MMR6")          1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1154
  {DBGFIELD("SH16_MMR6_SH_MMR6")  1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1155
  {DBGFIELD("SW16_MMR6_SWSP_MMR6_SW_MMR6") 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1156
  {DBGFIELD("SWC2_MMR6")          1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1157
  {DBGFIELD("SWM16_MMR6")         1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1158
  {DBGFIELD("SYNC_MMR6")          1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1159
  {DBGFIELD("SYNCI_MMR6")         1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1160
  {DBGFIELD("PREF_MMR6")          1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1161
  {DBGFIELD("CACHE_MMR6")         1, false, false, false,  8, 2,  5, 1,  0, 0}, // #1162
  {DBGFIELD("LD")                 1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1163
  {DBGFIELD("LL64_LLD")           1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1164
  {DBGFIELD("LWu")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1165
  {DBGFIELD("LB64")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1166
  {DBGFIELD("LBu64")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1167
  {DBGFIELD("LH64")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1168
  {DBGFIELD("LHu64")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1169
  {DBGFIELD("LW64")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1170
  {DBGFIELD("LWL64")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1171
  {DBGFIELD("LWR64")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1172
  {DBGFIELD("LDL")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1173
  {DBGFIELD("LDR")                1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1174
  {DBGFIELD("SD")                 1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1175
  {DBGFIELD("SC64_SCD")           1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1176
  {DBGFIELD("SB64")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1177
  {DBGFIELD("SH64")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1178
  {DBGFIELD("SW64")               1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1179
  {DBGFIELD("SWL64")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1180
  {DBGFIELD("SWR64")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1181
  {DBGFIELD("SDL")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1182
  {DBGFIELD("SDR")                1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1183
  {DBGFIELD("LWUPC")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1184
  {DBGFIELD("LDPC")               1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1185
  {DBGFIELD("LLD_R6")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1186
  {DBGFIELD("LL64_R6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1187
  {DBGFIELD("SC64_R6")            1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1188
  {DBGFIELD("SCD_R6")             1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1189
  {DBGFIELD("CRC32B")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1190
  {DBGFIELD("CRC32H")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1191
  {DBGFIELD("CRC32W")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1192
  {DBGFIELD("CRC32CB")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1193
  {DBGFIELD("CRC32CH")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1194
  {DBGFIELD("CRC32CW")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1195
  {DBGFIELD("CRC32D")             1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1196
  {DBGFIELD("CRC32CD")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1197
  {DBGFIELD("BADDu")              1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1198
  {DBGFIELD("BBIT0_BBIT032_BBIT1_BBIT132") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1199
  {DBGFIELD("CINS_CINS32_CINS64_32_CINS_i32") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1200
  {DBGFIELD("DMFC2_OCTEON")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1201
  {DBGFIELD("DMTC2_OCTEON")       1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1202
  {DBGFIELD("DPOP_POP")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1203
  {DBGFIELD("EXTS_EXTS32")        1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1204
  {DBGFIELD("MTM0_MTM1_MTM2_MTP0_MTP1_MTP2") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1205
  {DBGFIELD("SEQ_SNE")            1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1206
  {DBGFIELD("SEQi_SNEi")          1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1207
  {DBGFIELD("V3MULU_VMM0_VMULU")  1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1208
  {DBGFIELD("DMUL")               1, false, false, false, 12, 2,  5, 1,  0, 0}, // #1209
  {DBGFIELD("SAA_SAAD")           1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1210
  {DBGFIELD("ADDR_PS64")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1211
  {DBGFIELD("CVT_PS_PW64_CVT_PW_PS64") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1212
  {DBGFIELD("MULR_PS64")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1213
  {DBGFIELD("PseudoTRUNC_W_D_PseudoTRUNC_W_D32_PseudoTRUNC_W_S") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1214
  {DBGFIELD("MOVT_I64")           1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1215
  {DBGFIELD("MOVF_I64")           1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1216
  {DBGFIELD("MOVZ_I64_S")         1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1217
  {DBGFIELD("MOVN_I64_D64")       1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1218
  {DBGFIELD("MOVN_I64_S")         1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1219
  {DBGFIELD("MOVZ_I64_D64")       1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1220
  {DBGFIELD("SELEQZ_S_SELNEZ_S")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1221
  {DBGFIELD("SELEQZ_D_SELNEZ_D")  1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1222
  {DBGFIELD("MAX_S_MAXA_S")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1223
  {DBGFIELD("MAX_D_MAXA_D")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1224
  {DBGFIELD("MIN_S_MINA_D")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1225
  {DBGFIELD("MIN_D_MINA_S")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1226
  {DBGFIELD("CLASS_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1227
  {DBGFIELD("CLASS_D")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1228
  {DBGFIELD("RINT_S")             1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1229
  {DBGFIELD("RINT_D")             1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1230
  {DBGFIELD("BC1EQZ_BC1NEZ")      1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1231
  {DBGFIELD("SEL_D")              1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1232
  {DBGFIELD("SEL_S")              1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1233
  {DBGFIELD("MADDF_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1234
  {DBGFIELD("MSUBF_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1235
  {DBGFIELD("MADDF_D")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1236
  {DBGFIELD("MSUBF_D")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1237
  {DBGFIELD("MOVF_D32_MM")        1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1238
  {DBGFIELD("MOVF_S_MM")          1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1239
  {DBGFIELD("MOVN_I_D32_MM")      1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1240
  {DBGFIELD("MOVN_I_S_MM")        1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1241
  {DBGFIELD("MOVT_D32_MM")        1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1242
  {DBGFIELD("MOVT_S_MM")          1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1243
  {DBGFIELD("MOVZ_I_D32_MM")      1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1244
  {DBGFIELD("MOVZ_I_S_MM")        1, false, false, false, 16, 2,  4, 1,  0, 0}, // #1245
  {DBGFIELD("CVT_D32_S_MM_CVT_D32_W_MM_CVT_D64_S_MM_CVT_D64_W_MM_CVT_L_D64_MM_CVT_L_S_MM_CVT_S_D32_MM_CVT_S_D64_MM_CVT_S_W_MM_CVT_W_D32_MM_CVT_W_D64_MM_CVT_W_S_MM") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1246
  {DBGFIELD("CEIL_W_MM_CEIL_W_S_MM") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1247
  {DBGFIELD("FLOOR_W_MM_FLOOR_W_S_MM") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1248
  {DBGFIELD("NMADD_S_MM")         1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1249
  {DBGFIELD("NMADD_D32_MM")       1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1250
  {DBGFIELD("NMSUB_S_MM")         1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1251
  {DBGFIELD("NMSUB_D32_MM")       1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1252
  {DBGFIELD("MADD_S_MM")          1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1253
  {DBGFIELD("MADD_D32_MM")        1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1254
  {DBGFIELD("ROUND_W_MM_ROUND_W_S_MM") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1255
  {DBGFIELD("TRUNC_W_MM_TRUNC_W_S_MM") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1256
  {DBGFIELD("C_F_D32_MM_C_F_D64_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1257
  {DBGFIELD("C_F_S_MM")           1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1258
  {DBGFIELD("C_EQ_D32_MM_C_EQ_D64_MM_C_LE_D32_MM_C_LE_D64_MM_C_LT_D32_MM_C_LT_D64_MM_C_SF_D32_MM_C_SF_D64_MM_C_UN_D32_MM_C_UN_D64_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1259
  {DBGFIELD("C_EQ_S_MM_C_LE_S_MM_C_LT_S_MM_C_SF_S_MM_C_UN_S_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1260
  {DBGFIELD("C_NGE_D32_MM_C_NGE_D64_MM_C_NGL_D32_MM_C_NGL_D64_MM_C_NGT_D32_MM_C_NGT_D64_MM_C_OLE_D32_MM_C_OLE_D64_MM_C_OLT_D32_MM_C_OLT_D64_MM_C_SEQ_D32_MM_C_SEQ_D64_MM_C_UEQ_D32_MM_C_UEQ_D64_MM_C_ULE_D32_MM_C_ULE_D64_MM_C_ULT_D32_MM_C_ULT_D64_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1261
  {DBGFIELD("C_NGE_S_MM_C_NGL_S_MM_C_NGT_S_MM_C_OLE_S_MM_C_OLT_S_MM_C_SEQ_S_MM_C_UEQ_S_MM_C_ULE_S_MM_C_ULT_S_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1262
  {DBGFIELD("C_NGLE_D32_MM_C_NGLE_D64_MM") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1263
  {DBGFIELD("C_NGLE_S_MM")        1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1264
  {DBGFIELD("FCMP_S32_MM")        1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1265
  {DBGFIELD("FCMP_D32_MM")        1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1266
  {DBGFIELD("MFC1_MM")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1267
  {DBGFIELD("MFHC1_D32_MM_MFHC1_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1268
  {DBGFIELD("MTC1_MM_MTC1_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1269
  {DBGFIELD("MTHC1_D32_MM_MTHC1_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1270
  {DBGFIELD("FABS_D32_MM_FABS_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1271
  {DBGFIELD("FABS_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1272
  {DBGFIELD("FNEG_D32_MM_FNEG_D64_MM_FNEG_S_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1273
  {DBGFIELD("FADD_D32_MM_FADD_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1274
  {DBGFIELD("FADD_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1275
  {DBGFIELD("FMOV_D32_MM_FMOV_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1276
  {DBGFIELD("FMOV_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1277
  {DBGFIELD("FMUL_D32_MM_FMUL_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1278
  {DBGFIELD("FMUL_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1279
  {DBGFIELD("FSUB_D32_MM_FSUB_D64_MM") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1280
  {DBGFIELD("FSUB_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1281
  {DBGFIELD("MSUB_S_MM")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1282
  {DBGFIELD("MSUB_D32_MM")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1283
  {DBGFIELD("FDIV_S_MM")          1, false, false, false, 21, 2,  7, 1,  0, 0}, // #1284
  {DBGFIELD("FDIV_D32_MM_FDIV_D64_MM") 1, false, false, false, 23, 2,  8, 1,  0, 0}, // #1285
  {DBGFIELD("FSQRT_S_MM")         1, false, false, false, 21, 2,  7, 1,  0, 0}, // #1286
  {DBGFIELD("FSQRT_D32_MM_FSQRT_D64_MM") 1, false, false, false, 23, 2,  9, 1,  0, 0}, // #1287
  {DBGFIELD("RECIP_S_MM_RSQRT_S_MM") 1, false, false, false, 27, 2, 11, 1,  0, 0}, // #1288
  {DBGFIELD("RECIP_D32_MM_RECIP_D64_MM_RSQRT_D32_MM_RSQRT_D64_MM") 1, false, false, false, 25, 2, 10, 1,  0, 0}, // #1289
  {DBGFIELD("SDC1_MM_D32_SDC1_MM_D64") 1, false, false, false, 29, 2,  1, 1,  0, 0}, // #1290
  {DBGFIELD("SWC1_MM")            1, false, false, false, 29, 2,  1, 1,  0, 0}, // #1291
  {DBGFIELD("SUXC1_MM")           1, false, false, false, 29, 2,  1, 1,  0, 0}, // #1292
  {DBGFIELD("SWXC1_MM")           1, false, false, false, 29, 2,  1, 1,  0, 0}, // #1293
  {DBGFIELD("CFC1_MM")            1, false, false, false, 16, 2,  2, 1,  0, 0}, // #1294
  {DBGFIELD("CTC1_MM")            1, false, false, false, 16, 2,  2, 1,  0, 0}, // #1295
  {DBGFIELD("LDC1_MM_D32_LDC1_MM_D64") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #1296
  {DBGFIELD("LUXC1_MM")           1, false, false, false, 31, 2,  2, 1,  0, 0}, // #1297
  {DBGFIELD("LWC1_MM")            1, false, false, false, 31, 2,  2, 1,  0, 0}, // #1298
  {DBGFIELD("LWXC1_MM")           1, false, false, false, 31, 2,  2, 1,  0, 0}, // #1299
  {DBGFIELD("FNEG_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1300
  {DBGFIELD("CMP_AF_D_MMR6_CMP_EQ_D_MMR6_CMP_LE_D_MMR6_CMP_LT_D_MMR6_CMP_UN_D_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1301
  {DBGFIELD("CMP_AF_S_MMR6_CMP_EQ_S_MMR6_CMP_LE_S_MMR6_CMP_LT_S_MMR6_CMP_UN_S_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1302
  {DBGFIELD("CMP_SAF_D_MMR6_CMP_SEQ_D_MMR6_CMP_SLE_D_MMR6_CMP_SLT_D_MMR6_CMP_SUN_D_MMR6_CMP_UEQ_D_MMR6_CMP_ULE_D_MMR6_CMP_ULT_D_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1303
  {DBGFIELD("CMP_SAF_S_MMR6_CMP_SEQ_S_MMR6_CMP_SLE_S_MMR6_CMP_SLT_S_MMR6_CMP_SUN_S_MMR6_CMP_UEQ_S_MMR6_CMP_ULE_S_MMR6_CMP_ULT_S_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1304
  {DBGFIELD("CMP_SUEQ_D_MMR6_CMP_SULE_D_MMR6_CMP_SULT_D_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1305
  {DBGFIELD("CMP_SUEQ_S_MMR6_CMP_SULE_S_MMR6_CMP_SULT_S_MMR6") 1, false, false, false, 14, 2,  2, 1,  0, 0}, // #1306
  {DBGFIELD("CVT_D_L_MMR6_CVT_L_D_MMR6_CVT_L_S_MMR6_CVT_S_L_MMR6_CVT_S_W_MMR6_CVT_W_S_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1307
  {DBGFIELD("TRUNC_L_D_MMR6_TRUNC_L_S_MMR6_TRUNC_W_D_MMR6_TRUNC_W_S_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1308
  {DBGFIELD("ROUND_L_D_MMR6_ROUND_L_S_MMR6_ROUND_W_D_MMR6_ROUND_W_S_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1309
  {DBGFIELD("FLOOR_L_D_MMR6_FLOOR_L_S_MMR6_FLOOR_W_D_MMR6_FLOOR_W_S_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1310
  {DBGFIELD("CEIL_L_D_MMR6_CEIL_L_S_MMR6_CEIL_W_D_MMR6_CEIL_W_S_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1311
  {DBGFIELD("MFC1_MMR6")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1312
  {DBGFIELD("MTC1_MMR6")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1313
  {DBGFIELD("CLASS_S_MMR6_CLASS_D_MMR6") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1314
  {DBGFIELD("FADD_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1315
  {DBGFIELD("MAX_D_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1316
  {DBGFIELD("MAX_S_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1317
  {DBGFIELD("MIN_D_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1318
  {DBGFIELD("MIN_S_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1319
  {DBGFIELD("MAXA_D_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1320
  {DBGFIELD("MAXA_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1321
  {DBGFIELD("MINA_D_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1322
  {DBGFIELD("MINA_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1323
  {DBGFIELD("SELEQZ_D_MMR6_SELNEZ_D_MMR6") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1324
  {DBGFIELD("SELEQZ_S_MMR6_SELNEZ_S_MMR6") 1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1325
  {DBGFIELD("SEL_D_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1326
  {DBGFIELD("SEL_S_MMR6")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1327
  {DBGFIELD("RINT_S_MMR6_RINT_D_MMR6") 1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1328
  {DBGFIELD("MADDF_D_MMR6")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1329
  {DBGFIELD("MADDF_S_MMR6")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1330
  {DBGFIELD("MSUBF_D_MMR6")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1331
  {DBGFIELD("MSUBF_S_MMR6")       1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1332
  {DBGFIELD("FMOV_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1333
  {DBGFIELD("FMUL_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1334
  {DBGFIELD("FSUB_S_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1335
  {DBGFIELD("FMOV_D_MMR6")        1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1336
  {DBGFIELD("FDIV_S_MMR6")        1, false, false, false, 19, 2,  5, 1,  0, 0}, // #1337
  {DBGFIELD("SDC1_D64_MMR6")      1, false, false, false, 29, 2,  1, 1,  0, 0}, // #1338
  {DBGFIELD("LDC1_D64_MMR6")      1, false, false, false, 31, 2,  2, 1,  0, 0}, // #1339
  {DBGFIELD("DMFC1")              1, false, false, false, 16, 2,  2, 1,  0, 0}, // #1340
  {DBGFIELD("DMTC1")              1, false, false, false, 16, 2,  2, 1,  0, 0}, // #1341
  {DBGFIELD("SWDSP")              1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1342
  {DBGFIELD("LWDSP")              1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1343
  {DBGFIELD("PseudoMTLOHI_DSP")   1, false, false, false,  1, 2,  2, 1,  0, 0}, // #1344
  {DBGFIELD("EXTRV_RS_W")         1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1345
  {DBGFIELD("EXTRV_R_W")          1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1346
  {DBGFIELD("EXTRV_S_H")          1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1347
  {DBGFIELD("EXTRV_W")            1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1348
  {DBGFIELD("EXTR_RS_W")          1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1349
  {DBGFIELD("EXTR_R_W")           1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1350
  {DBGFIELD("EXTR_S_H")           1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1351
  {DBGFIELD("EXTR_W")             1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1352
  {DBGFIELD("INSV")               1, false, false, false, 36, 1, 13, 1,  0, 0}, // #1353
  {DBGFIELD("MTHLIP")             1, false, false, false, 36, 1,  5, 1,  0, 0}, // #1354
  {DBGFIELD("MTHI_DSP")           1, false, false, false, 36, 1,  5, 1,  0, 0}, // #1355
  {DBGFIELD("MTLO_DSP")           1, false, false, false, 36, 1,  5, 1,  0, 0}, // #1356
  {DBGFIELD("ABSQ_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1357
  {DBGFIELD("ABSQ_S_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1358
  {DBGFIELD("ADDQ_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1359
  {DBGFIELD("ADDQ_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1360
  {DBGFIELD("ADDQ_S_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1361
  {DBGFIELD("ADDSC")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1362
  {DBGFIELD("ADDU_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1363
  {DBGFIELD("ADDU_S_QB")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1364
  {DBGFIELD("ADDWC")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1365
  {DBGFIELD("BITREV")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1366
  {DBGFIELD("BPOSGE32")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1367
  {DBGFIELD("CMPGU_EQ_QB")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1368
  {DBGFIELD("CMPGU_LE_QB")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1369
  {DBGFIELD("CMPGU_LT_QB")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1370
  {DBGFIELD("CMPU_EQ_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1371
  {DBGFIELD("CMPU_LE_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1372
  {DBGFIELD("CMPU_LT_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1373
  {DBGFIELD("CMP_EQ_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1374
  {DBGFIELD("CMP_LE_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1375
  {DBGFIELD("CMP_LT_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1376
  {DBGFIELD("DPAQ_SA_L_W")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1377
  {DBGFIELD("DPAQ_S_W_PH")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1378
  {DBGFIELD("DPAU_H_QBL")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1379
  {DBGFIELD("DPAU_H_QBR")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1380
  {DBGFIELD("DPSQ_SA_L_W")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1381
  {DBGFIELD("DPSQ_S_W_PH")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1382
  {DBGFIELD("DPSU_H_QBL")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1383
  {DBGFIELD("DPSU_H_QBR")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1384
  {DBGFIELD("EXTPDPV")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1385
  {DBGFIELD("EXTPDP")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1386
  {DBGFIELD("EXTPV")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1387
  {DBGFIELD("EXTP")               1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1388
  {DBGFIELD("LBUX")               1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1389
  {DBGFIELD("LHX")                1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1390
  {DBGFIELD("LWX")                1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1391
  {DBGFIELD("MADDU_DSP")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1392
  {DBGFIELD("MADD_DSP")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1393
  {DBGFIELD("MAQ_SA_W_PHL")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1394
  {DBGFIELD("MAQ_SA_W_PHR")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1395
  {DBGFIELD("MAQ_S_W_PHL")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1396
  {DBGFIELD("MAQ_S_W_PHR")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1397
  {DBGFIELD("MFHI_DSP")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1398
  {DBGFIELD("MFLO_DSP")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1399
  {DBGFIELD("MODSUB")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1400
  {DBGFIELD("MSUBU_DSP")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1401
  {DBGFIELD("MSUB_DSP")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1402
  {DBGFIELD("MULEQ_S_W_PHL")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1403
  {DBGFIELD("MULEQ_S_W_PHR")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1404
  {DBGFIELD("MULEU_S_PH_QBL")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1405
  {DBGFIELD("MULEU_S_PH_QBR")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1406
  {DBGFIELD("MULQ_RS_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1407
  {DBGFIELD("MULSAQ_S_W_PH")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1408
  {DBGFIELD("MULTU_DSP")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1409
  {DBGFIELD("MULT_DSP")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1410
  {DBGFIELD("PACKRL_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1411
  {DBGFIELD("PICK_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1412
  {DBGFIELD("PICK_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1413
  {DBGFIELD("PRECEQU_PH_QBLA")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1414
  {DBGFIELD("PRECEQU_PH_QBL")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1415
  {DBGFIELD("PRECEQU_PH_QBRA")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1416
  {DBGFIELD("PRECEQU_PH_QBR")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1417
  {DBGFIELD("PRECEQ_W_PHL")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1418
  {DBGFIELD("PRECEQ_W_PHR")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1419
  {DBGFIELD("PRECEU_PH_QBLA")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1420
  {DBGFIELD("PRECEU_PH_QBL")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1421
  {DBGFIELD("PRECEU_PH_QBRA")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1422
  {DBGFIELD("PRECEU_PH_QBR")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1423
  {DBGFIELD("PRECRQU_S_QB_PH")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1424
  {DBGFIELD("PRECRQ_PH_W")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1425
  {DBGFIELD("PRECRQ_QB_PH")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1426
  {DBGFIELD("PRECRQ_RS_PH_W")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1427
  {DBGFIELD("RADDU_W_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1428
  {DBGFIELD("RDDSP")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1429
  {DBGFIELD("REPLV_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1430
  {DBGFIELD("REPLV_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1431
  {DBGFIELD("REPL_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1432
  {DBGFIELD("REPL_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1433
  {DBGFIELD("SHILOV")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1434
  {DBGFIELD("SHILO")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1435
  {DBGFIELD("SHLLV_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1436
  {DBGFIELD("SHLLV_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1437
  {DBGFIELD("SHLLV_S_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1438
  {DBGFIELD("SHLLV_S_W")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1439
  {DBGFIELD("SHLL_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1440
  {DBGFIELD("SHLL_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1441
  {DBGFIELD("SHLL_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1442
  {DBGFIELD("SHLL_S_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1443
  {DBGFIELD("SHRAV_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1444
  {DBGFIELD("SHRAV_R_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1445
  {DBGFIELD("SHRAV_R_W")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1446
  {DBGFIELD("SHRA_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1447
  {DBGFIELD("SHRA_R_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1448
  {DBGFIELD("SHRA_R_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1449
  {DBGFIELD("SHRLV_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1450
  {DBGFIELD("SHRL_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1451
  {DBGFIELD("SUBQ_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1452
  {DBGFIELD("SUBQ_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1453
  {DBGFIELD("SUBQ_S_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1454
  {DBGFIELD("SUBU_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1455
  {DBGFIELD("SUBU_S_QB")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1456
  {DBGFIELD("WRDSP")              1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1457
  {DBGFIELD("PseudoCMPU_EQ_QB_PseudoCMPU_LE_QB_PseudoCMPU_LT_QB_PseudoCMP_EQ_PH_PseudoCMP_LE_PH_PseudoCMP_LT_PH") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1458
  {DBGFIELD("PseudoPICK_PH_PseudoPICK_QB") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1459
  {DBGFIELD("ABSQ_S_QB")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1460
  {DBGFIELD("ADDQH_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1461
  {DBGFIELD("ADDQH_R_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1462
  {DBGFIELD("ADDQH_R_W")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1463
  {DBGFIELD("ADDQH_W")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1464
  {DBGFIELD("ADDUH_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1465
  {DBGFIELD("ADDUH_R_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1466
  {DBGFIELD("ADDU_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1467
  {DBGFIELD("ADDU_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1468
  {DBGFIELD("APPEND")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1469
  {DBGFIELD("BALIGN")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1470
  {DBGFIELD("CMPGDU_EQ_QB")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1471
  {DBGFIELD("CMPGDU_LE_QB")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1472
  {DBGFIELD("CMPGDU_LT_QB")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1473
  {DBGFIELD("DPA_W_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1474
  {DBGFIELD("DPAQX_SA_W_PH")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1475
  {DBGFIELD("DPAQX_S_W_PH")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1476
  {DBGFIELD("DPAX_W_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1477
  {DBGFIELD("DPS_W_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1478
  {DBGFIELD("DPSQX_S_W_PH")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1479
  {DBGFIELD("DPSQX_SA_W_PH")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1480
  {DBGFIELD("DPSX_W_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1481
  {DBGFIELD("MUL_PH")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1482
  {DBGFIELD("MUL_S_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1483
  {DBGFIELD("MULQ_RS_W")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1484
  {DBGFIELD("MULQ_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1485
  {DBGFIELD("MULQ_S_W")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1486
  {DBGFIELD("MULSA_W_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1487
  {DBGFIELD("PRECR_QB_PH")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1488
  {DBGFIELD("PRECR_SRA_PH_W")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1489
  {DBGFIELD("PRECR_SRA_R_PH_W")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1490
  {DBGFIELD("PREPEND")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1491
  {DBGFIELD("SHRA_QB")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1492
  {DBGFIELD("SHRA_R_QB")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1493
  {DBGFIELD("SHRAV_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1494
  {DBGFIELD("SHRAV_R_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1495
  {DBGFIELD("SHRL_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1496
  {DBGFIELD("SHRLV_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1497
  {DBGFIELD("SUBQH_PH")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1498
  {DBGFIELD("SUBQH_R_PH")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1499
  {DBGFIELD("SUBQH_W")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1500
  {DBGFIELD("SUBQH_R_W")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1501
  {DBGFIELD("SUBU_PH")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1502
  {DBGFIELD("SUBU_S_PH")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1503
  {DBGFIELD("SUBUH_QB")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1504
  {DBGFIELD("SUBUH_R_QB")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1505
  {DBGFIELD("LWDSP_MM")           1, false, false, false,  8, 2,  2, 1,  0, 0}, // #1506
  {DBGFIELD("SWDSP_MM")           1, false, false, false,  8, 2,  1, 1,  0, 0}, // #1507
  {DBGFIELD("ABSQ_S_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1508
  {DBGFIELD("ABSQ_S_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1509
  {DBGFIELD("ADDQ_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1510
  {DBGFIELD("ADDQ_S_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1511
  {DBGFIELD("ADDQ_S_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1512
  {DBGFIELD("ADDSC_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1513
  {DBGFIELD("ADDU_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1514
  {DBGFIELD("ADDU_S_QB_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1515
  {DBGFIELD("ADDWC_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1516
  {DBGFIELD("BITREV_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1517
  {DBGFIELD("BPOSGE32_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1518
  {DBGFIELD("CMPGU_EQ_QB_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1519
  {DBGFIELD("CMPGU_LE_QB_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1520
  {DBGFIELD("CMPGU_LT_QB_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1521
  {DBGFIELD("CMPU_EQ_QB_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1522
  {DBGFIELD("CMPU_LE_QB_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1523
  {DBGFIELD("CMPU_LT_QB_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1524
  {DBGFIELD("CMP_EQ_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1525
  {DBGFIELD("CMP_LE_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1526
  {DBGFIELD("CMP_LT_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1527
  {DBGFIELD("DPAQ_SA_L_W_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1528
  {DBGFIELD("DPAQ_S_W_PH_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1529
  {DBGFIELD("DPAU_H_QBL_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1530
  {DBGFIELD("DPAU_H_QBR_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1531
  {DBGFIELD("DPSQ_SA_L_W_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1532
  {DBGFIELD("DPSQ_S_W_PH_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1533
  {DBGFIELD("DPSU_H_QBL_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1534
  {DBGFIELD("DPSU_H_QBR_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1535
  {DBGFIELD("EXTPDPV_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1536
  {DBGFIELD("EXTPDP_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1537
  {DBGFIELD("EXTPV_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1538
  {DBGFIELD("EXTP_MM")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1539
  {DBGFIELD("EXTRV_RS_W_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1540
  {DBGFIELD("EXTRV_R_W_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1541
  {DBGFIELD("EXTRV_S_H_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1542
  {DBGFIELD("EXTRV_W_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1543
  {DBGFIELD("EXTR_RS_W_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1544
  {DBGFIELD("EXTR_R_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1545
  {DBGFIELD("EXTR_S_H_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1546
  {DBGFIELD("EXTR_W_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1547
  {DBGFIELD("INSV_MM")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1548
  {DBGFIELD("LBUX_MM")            1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1549
  {DBGFIELD("LHX_MM")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1550
  {DBGFIELD("LWX_MM")             1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1551
  {DBGFIELD("MADDU_DSP_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1552
  {DBGFIELD("MADD_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1553
  {DBGFIELD("MAQ_SA_W_PHL_MM")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1554
  {DBGFIELD("MAQ_SA_W_PHR_MM")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1555
  {DBGFIELD("MAQ_S_W_PHL_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1556
  {DBGFIELD("MAQ_S_W_PHR_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1557
  {DBGFIELD("MFHI_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1558
  {DBGFIELD("MFLO_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1559
  {DBGFIELD("MODSUB_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1560
  {DBGFIELD("MOVEP_MMR6")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1561
  {DBGFIELD("MOVN_I_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1562
  {DBGFIELD("MOVZ_I_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1563
  {DBGFIELD("MSUBU_DSP_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1564
  {DBGFIELD("MSUB_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1565
  {DBGFIELD("MTHI_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1566
  {DBGFIELD("MTHLIP_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1567
  {DBGFIELD("MTLO_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1568
  {DBGFIELD("MULEQ_S_W_PHL_MM")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1569
  {DBGFIELD("MULEQ_S_W_PHR_MM")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1570
  {DBGFIELD("MULEU_S_PH_QBL_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1571
  {DBGFIELD("MULEU_S_PH_QBR_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1572
  {DBGFIELD("MULQ_RS_PH_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1573
  {DBGFIELD("MULSAQ_S_W_PH_MM")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1574
  {DBGFIELD("MULTU_DSP_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1575
  {DBGFIELD("MULT_DSP_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1576
  {DBGFIELD("PACKRL_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1577
  {DBGFIELD("PICK_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1578
  {DBGFIELD("PICK_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1579
  {DBGFIELD("PRECEQU_PH_QBLA_MM") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1580
  {DBGFIELD("PRECEQU_PH_QBL_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1581
  {DBGFIELD("PRECEQU_PH_QBRA_MM") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1582
  {DBGFIELD("PRECEQU_PH_QBR_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1583
  {DBGFIELD("PRECEQ_W_PHL_MM")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1584
  {DBGFIELD("PRECEQ_W_PHR_MM")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1585
  {DBGFIELD("PRECEU_PH_QBLA_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1586
  {DBGFIELD("PRECEU_PH_QBL_MM")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1587
  {DBGFIELD("PRECEU_PH_QBRA_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1588
  {DBGFIELD("PRECEU_PH_QBR_MM")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1589
  {DBGFIELD("PRECRQU_S_QB_PH_MM") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1590
  {DBGFIELD("PRECRQ_PH_W_MM")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1591
  {DBGFIELD("PRECRQ_QB_PH_MM")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1592
  {DBGFIELD("PRECRQ_RS_PH_W_MM")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1593
  {DBGFIELD("RADDU_W_QB_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1594
  {DBGFIELD("RDDSP_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1595
  {DBGFIELD("REPLV_PH_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1596
  {DBGFIELD("REPLV_QB_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1597
  {DBGFIELD("REPL_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1598
  {DBGFIELD("REPL_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1599
  {DBGFIELD("SHILOV_MM")          1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1600
  {DBGFIELD("SHILO_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1601
  {DBGFIELD("SHLLV_PH_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1602
  {DBGFIELD("SHLLV_QB_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1603
  {DBGFIELD("SHLLV_S_PH_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1604
  {DBGFIELD("SHLLV_S_W_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1605
  {DBGFIELD("SHLL_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1606
  {DBGFIELD("SHLL_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1607
  {DBGFIELD("SHLL_S_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1608
  {DBGFIELD("SHLL_S_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1609
  {DBGFIELD("SHRAV_PH_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1610
  {DBGFIELD("SHRAV_R_PH_MM")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1611
  {DBGFIELD("SHRAV_R_W_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1612
  {DBGFIELD("SHRA_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1613
  {DBGFIELD("SHRA_R_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1614
  {DBGFIELD("SHRA_R_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1615
  {DBGFIELD("SHRLV_QB_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1616
  {DBGFIELD("SHRL_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1617
  {DBGFIELD("SUBQ_PH_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1618
  {DBGFIELD("SUBQ_S_PH_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1619
  {DBGFIELD("SUBQ_S_W_MM")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1620
  {DBGFIELD("SUBU_QB_MM")         1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1621
  {DBGFIELD("SUBU_S_QB_MM")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1622
  {DBGFIELD("WRDSP_MM")           1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1623
  {DBGFIELD("ABSQ_S_QB_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1624
  {DBGFIELD("ADDQH_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1625
  {DBGFIELD("ADDQH_R_PH_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1626
  {DBGFIELD("ADDQH_R_W_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1627
  {DBGFIELD("ADDQH_W_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1628
  {DBGFIELD("ADDUH_QB_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1629
  {DBGFIELD("ADDUH_R_QB_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1630
  {DBGFIELD("ADDU_PH_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1631
  {DBGFIELD("ADDU_S_PH_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1632
  {DBGFIELD("APPEND_MMR2")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1633
  {DBGFIELD("BALIGN_MMR2")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1634
  {DBGFIELD("CMPGDU_EQ_QB_MMR2")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1635
  {DBGFIELD("CMPGDU_LE_QB_MMR2")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1636
  {DBGFIELD("CMPGDU_LT_QB_MMR2")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1637
  {DBGFIELD("DPA_W_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1638
  {DBGFIELD("DPAQX_SA_W_PH_MMR2") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1639
  {DBGFIELD("DPAQX_S_W_PH_MMR2")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1640
  {DBGFIELD("DPAX_W_PH_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1641
  {DBGFIELD("DPS_W_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1642
  {DBGFIELD("DPSQX_S_W_PH_MMR2")  1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1643
  {DBGFIELD("DPSQX_SA_W_PH_MMR2") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1644
  {DBGFIELD("DPSX_W_PH_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1645
  {DBGFIELD("MUL_PH_MMR2")        1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1646
  {DBGFIELD("MUL_S_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1647
  {DBGFIELD("MULQ_RS_W_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1648
  {DBGFIELD("MULQ_S_PH_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1649
  {DBGFIELD("MULQ_S_W_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1650
  {DBGFIELD("MULSA_W_PH_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1651
  {DBGFIELD("PRECR_QB_PH_MMR2")   1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1652
  {DBGFIELD("PRECR_SRA_PH_W_MMR2") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1653
  {DBGFIELD("PRECR_SRA_R_PH_W_MMR2") 1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1654
  {DBGFIELD("PREPEND_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1655
  {DBGFIELD("SHRA_QB_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1656
  {DBGFIELD("SHRA_R_QB_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1657
  {DBGFIELD("SHRAV_QB_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1658
  {DBGFIELD("SHRAV_R_QB_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1659
  {DBGFIELD("SHRL_PH_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1660
  {DBGFIELD("SHRLV_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1661
  {DBGFIELD("SUBQH_PH_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1662
  {DBGFIELD("SUBQH_R_PH_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1663
  {DBGFIELD("SUBQH_W_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1664
  {DBGFIELD("SUBQH_R_W_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1665
  {DBGFIELD("SUBU_PH_MMR2")       1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1666
  {DBGFIELD("SUBU_S_PH_MMR2")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1667
  {DBGFIELD("SUBUH_QB_MMR2")      1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1668
  {DBGFIELD("SUBUH_R_QB_MMR2")    1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1669
  {DBGFIELD("BPOSGE32C_MMR3")     1, false, false, false, 36, 1,  2, 1,  0, 0}, // #1670
  {DBGFIELD("CMP_F_D")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1671
  {DBGFIELD("CMP_F_S")            1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1672
  {DBGFIELD("CMP_SAF_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1673
  {DBGFIELD("CMP_SAF_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1674
  {DBGFIELD("CMP_SEQ_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1675
  {DBGFIELD("CMP_SEQ_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1676
  {DBGFIELD("CMP_SLE_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1677
  {DBGFIELD("CMP_SLE_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1678
  {DBGFIELD("CMP_SLT_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1679
  {DBGFIELD("CMP_SLT_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1680
  {DBGFIELD("CMP_SUEQ_D")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1681
  {DBGFIELD("CMP_SUEQ_S")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1682
  {DBGFIELD("CMP_SULE_D")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1683
  {DBGFIELD("CMP_SULE_S")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1684
  {DBGFIELD("CMP_SULT_D")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1685
  {DBGFIELD("CMP_SULT_S")         1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1686
  {DBGFIELD("CMP_SUN_D")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1687
  {DBGFIELD("CMP_SUN_S")          1, false, false, false, 14, 2,  4, 1,  0, 0}, // #1688
  {DBGFIELD("P5600WriteALU")      0, false, false, false,  0, 0,  0, 0,  0, 0}, // #1689
  {DBGFIELD("P5600WriteAL2")      0, false, false, false,  0, 0,  0, 0,  0, 0}, // #1690
}; // MipsGenericModelSchedClasses

// {Name, NumMicroOps, BeginGroup, EndGroup, RetireOOO, WriteProcResIdx,#, WriteLatencyIdx,#, ReadAdvanceIdx,#}
static const llvm::MCSchedClassDesc MipsP5600ModelSchedClasses[] = {
  {DBGFIELD("InvalidSchedClass")  8191, false, false, false, 0, 0,  0, 0,  0, 0},
  {DBGFIELD("IIPseudo")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1
  {DBGFIELD("II_B")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #2
  {DBGFIELD("II_BCCZAL")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #3
  {DBGFIELD("II_MTC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #4
  {DBGFIELD("II_MFC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #5
  {DBGFIELD("II_JALR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #6
  {DBGFIELD("II_JAL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #7
  {DBGFIELD("II_CVT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #8
  {DBGFIELD("II_DMULT")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #9
  {DBGFIELD("II_DMULTU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #10
  {DBGFIELD("II_DDIV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #11
  {DBGFIELD("II_DDIVU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #12
  {DBGFIELD("II_IndirectBranchPseudo") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #13
  {DBGFIELD("II_MADD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #14
  {DBGFIELD("II_MADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #15
  {DBGFIELD("II_MFHI_MFLO")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #16
  {DBGFIELD("II_MSUB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #17
  {DBGFIELD("II_MSUBU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #18
  {DBGFIELD("II_MTHI_MTLO")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #19
  {DBGFIELD("II_MULT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #20
  {DBGFIELD("II_MULTU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #21
  {DBGFIELD("II_ReturnPseudo")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #22
  {DBGFIELD("II_DIV")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #23
  {DBGFIELD("II_DIVU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #24
  {DBGFIELD("II_J")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #25
  {DBGFIELD("II_JR")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #26
  {DBGFIELD("II_TRAP")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #27
  {DBGFIELD("II_ADD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #28
  {DBGFIELD("II_ADDIUPC")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #29
  {DBGFIELD("II_ADDIU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #30
  {DBGFIELD("II_ADDR_PS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #31
  {DBGFIELD("II_ADDU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #32
  {DBGFIELD("II_ADDI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #33
  {DBGFIELD("II_ALIGN")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #34
  {DBGFIELD("II_ALUIPC")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #35
  {DBGFIELD("II_AND")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #36
  {DBGFIELD("II_ANDI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #37
  {DBGFIELD("II_AUI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #38
  {DBGFIELD("II_AUIPC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #39
  {DBGFIELD("IIM16Alu")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #40
  {DBGFIELD("II_BADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #41
  {DBGFIELD("II_BC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #42
  {DBGFIELD("II_BALC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #43
  {DBGFIELD("II_BBIT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #44
  {DBGFIELD("II_BC1CCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #45
  {DBGFIELD("II_BC1F")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #46
  {DBGFIELD("II_BC1FL")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #47
  {DBGFIELD("II_BC1T")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #48
  {DBGFIELD("II_BC1TL")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #49
  {DBGFIELD("II_BC2CCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #50
  {DBGFIELD("II_BCC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #51
  {DBGFIELD("II_BCCC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #52
  {DBGFIELD("II_BCCZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #53
  {DBGFIELD("II_BCCZC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #54
  {DBGFIELD("II_BCCZALS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #55
  {DBGFIELD("II_BITSWAP")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #56
  {DBGFIELD("II_BREAK")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #57
  {DBGFIELD("II_CACHE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #58
  {DBGFIELD("II_CACHEE")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #59
  {DBGFIELD("II_CEIL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #60
  {DBGFIELD("II_CFC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #61
  {DBGFIELD("II_CFC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #62
  {DBGFIELD("II_INS")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #63
  {DBGFIELD("II_CLASS_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #64
  {DBGFIELD("II_CLASS_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #65
  {DBGFIELD("II_CLO")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #66
  {DBGFIELD("II_CLZ")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #67
  {DBGFIELD("II_CMP_CC_D")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #68
  {DBGFIELD("II_CMP_CC_S")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #69
  {DBGFIELD("II_CRC32B")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #70
  {DBGFIELD("II_CRC32CB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #71
  {DBGFIELD("II_CRC32CD")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #72
  {DBGFIELD("II_CRC32CH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #73
  {DBGFIELD("II_CRC32CW")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #74
  {DBGFIELD("II_CRC32D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #75
  {DBGFIELD("II_CRC32H")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #76
  {DBGFIELD("II_CRC32W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #77
  {DBGFIELD("II_CTC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #78
  {DBGFIELD("II_CTC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #79
  {DBGFIELD("II_C_CC_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #80
  {DBGFIELD("II_C_CC_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #81
  {DBGFIELD("II_DADD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #82
  {DBGFIELD("II_DADDI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #83
  {DBGFIELD("II_DADDIU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #84
  {DBGFIELD("II_DADDU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #85
  {DBGFIELD("II_DAHI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #86
  {DBGFIELD("II_DALIGN")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #87
  {DBGFIELD("II_DATI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #88
  {DBGFIELD("II_DAUI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #89
  {DBGFIELD("II_DBITSWAP")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #90
  {DBGFIELD("II_DCLO")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #91
  {DBGFIELD("II_DCLZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #92
  {DBGFIELD("II_DERET")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #93
  {DBGFIELD("II_EXT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #94
  {DBGFIELD("II_DI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #95
  {DBGFIELD("II_DLSA")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #96
  {DBGFIELD("II_DMFC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #97
  {DBGFIELD("II_DMFC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #98
  {DBGFIELD("II_DMFC2")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #99
  {DBGFIELD("II_DMFGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #100
  {DBGFIELD("II_DMOD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #101
  {DBGFIELD("II_DMODU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #102
  {DBGFIELD("II_DMT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #103
  {DBGFIELD("II_DMTC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #104
  {DBGFIELD("II_DMTC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #105
  {DBGFIELD("II_DMTC2")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #106
  {DBGFIELD("II_DMTGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #107
  {DBGFIELD("II_DMUH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #108
  {DBGFIELD("II_DMUHU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #109
  {DBGFIELD("II_DMUL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #110
  {DBGFIELD("II_POP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #111
  {DBGFIELD("II_DROTR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #112
  {DBGFIELD("II_DROTR32")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #113
  {DBGFIELD("II_DROTRV")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #114
  {DBGFIELD("II_DSBH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #115
  {DBGFIELD("II_DSHD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #116
  {DBGFIELD("II_DSLL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #117
  {DBGFIELD("II_DSLL32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #118
  {DBGFIELD("II_DSLLV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #119
  {DBGFIELD("II_DSRA")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #120
  {DBGFIELD("II_DSRA32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #121
  {DBGFIELD("II_DSRAV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #122
  {DBGFIELD("II_DSRL")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #123
  {DBGFIELD("II_DSRL32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #124
  {DBGFIELD("II_DSRLV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #125
  {DBGFIELD("II_DSUB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #126
  {DBGFIELD("II_DSUBU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #127
  {DBGFIELD("II_DVP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #128
  {DBGFIELD("II_DVPE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #129
  {DBGFIELD("II_EHB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #130
  {DBGFIELD("II_EI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #131
  {DBGFIELD("II_EMT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #132
  {DBGFIELD("II_ERET")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #133
  {DBGFIELD("II_ERETNC")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #134
  {DBGFIELD("II_EVP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #135
  {DBGFIELD("II_EVPE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #136
  {DBGFIELD("II_ABS")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #137
  {DBGFIELD("II_SQRT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #138
  {DBGFIELD("II_ADD_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #139
  {DBGFIELD("II_ADD_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #140
  {DBGFIELD("II_ADD_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #141
  {DBGFIELD("II_DIV_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #142
  {DBGFIELD("II_DIV_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #143
  {DBGFIELD("II_FLOOR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #144
  {DBGFIELD("II_MOV_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #145
  {DBGFIELD("II_MOV_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #146
  {DBGFIELD("II_MUL_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #147
  {DBGFIELD("II_MUL_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #148
  {DBGFIELD("II_MUL_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #149
  {DBGFIELD("II_NEG")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #150
  {DBGFIELD("II_FORK")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #151
  {DBGFIELD("II_SQRT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #152
  {DBGFIELD("II_SUB_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #153
  {DBGFIELD("II_SUB_PS")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #154
  {DBGFIELD("II_SUB_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #155
  {DBGFIELD("II_GINVI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #156
  {DBGFIELD("II_GINVT")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #157
  {DBGFIELD("II_HYPCALL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #158
  {DBGFIELD("II_JALR_HB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #159
  {DBGFIELD("II_JALRC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #160
  {DBGFIELD("II_JALRS")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #161
  {DBGFIELD("II_JALS")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #162
  {DBGFIELD("II_JIALC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #163
  {DBGFIELD("II_JIC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #164
  {DBGFIELD("II_JRADDIUSP")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #165
  {DBGFIELD("II_JRC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #166
  {DBGFIELD("II_JR_HB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #167
  {DBGFIELD("II_LB")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #168
  {DBGFIELD("II_LBE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #169
  {DBGFIELD("II_LBU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #170
  {DBGFIELD("II_LBUE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #171
  {DBGFIELD("II_LD")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #172
  {DBGFIELD("II_LDC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #173
  {DBGFIELD("II_LDC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #174
  {DBGFIELD("II_LDC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #175
  {DBGFIELD("II_LDL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #176
  {DBGFIELD("II_LDPC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #177
  {DBGFIELD("II_LDR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #178
  {DBGFIELD("II_LDXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #179
  {DBGFIELD("II_LH")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #180
  {DBGFIELD("II_LHE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #181
  {DBGFIELD("II_LHU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #182
  {DBGFIELD("II_LHUE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #183
  {DBGFIELD("II_LI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #184
  {DBGFIELD("II_LL")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #185
  {DBGFIELD("II_LLD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #186
  {DBGFIELD("II_LLE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #187
  {DBGFIELD("II_LSA")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #188
  {DBGFIELD("II_LUI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #189
  {DBGFIELD("II_LUXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #190
  {DBGFIELD("II_LW")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #191
  {DBGFIELD("II_LWC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #192
  {DBGFIELD("II_LWC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #193
  {DBGFIELD("II_LWC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #194
  {DBGFIELD("II_LWE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #195
  {DBGFIELD("II_LWL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #196
  {DBGFIELD("II_LWLE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #197
  {DBGFIELD("II_LWM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #198
  {DBGFIELD("II_LWPC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #199
  {DBGFIELD("II_LWP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #200
  {DBGFIELD("II_LWR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #201
  {DBGFIELD("II_LWRE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #202
  {DBGFIELD("II_LWUPC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #203
  {DBGFIELD("II_LWU")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #204
  {DBGFIELD("II_LWXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #205
  {DBGFIELD("II_LWXS")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #206
  {DBGFIELD("II_MADDF_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #207
  {DBGFIELD("II_MADDF_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #208
  {DBGFIELD("II_MADD_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #209
  {DBGFIELD("II_MADD_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #210
  {DBGFIELD("II_MAX_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #211
  {DBGFIELD("II_MAXA_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #212
  {DBGFIELD("II_MAX_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #213
  {DBGFIELD("II_MAXA_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #214
  {DBGFIELD("II_MFC0")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #215
  {DBGFIELD("II_MFC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #216
  {DBGFIELD("II_MFGC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #217
  {DBGFIELD("II_MFHC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #218
  {DBGFIELD("II_MFHC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #219
  {DBGFIELD("II_MFHGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #220
  {DBGFIELD("II_MFTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #221
  {DBGFIELD("II_MIN_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #222
  {DBGFIELD("II_MINA_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #223
  {DBGFIELD("II_MIN_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #224
  {DBGFIELD("II_MINA_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #225
  {DBGFIELD("II_MOD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #226
  {DBGFIELD("II_MODU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #227
  {DBGFIELD("II_MOVE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #228
  {DBGFIELD("II_MOVF_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #229
  {DBGFIELD("II_MOVF")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #230
  {DBGFIELD("II_MOVF_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #231
  {DBGFIELD("II_MOVN_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #232
  {DBGFIELD("II_MOVN")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #233
  {DBGFIELD("II_MOVN_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #234
  {DBGFIELD("II_MOVT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #235
  {DBGFIELD("II_MOVT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #236
  {DBGFIELD("II_MOVT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #237
  {DBGFIELD("II_MOVZ_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #238
  {DBGFIELD("II_MOVZ")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #239
  {DBGFIELD("II_MOVZ_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #240
  {DBGFIELD("II_MSUBF_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #241
  {DBGFIELD("II_MSUBF_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #242
  {DBGFIELD("II_MSUB_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #243
  {DBGFIELD("II_MSUB_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #244
  {DBGFIELD("II_MTC0")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #245
  {DBGFIELD("II_MTC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #246
  {DBGFIELD("II_MTGC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #247
  {DBGFIELD("II_MTHC0")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #248
  {DBGFIELD("II_MTHC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #249
  {DBGFIELD("II_MTHGC0")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #250
  {DBGFIELD("II_MTTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #251
  {DBGFIELD("II_MUH")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #252
  {DBGFIELD("II_MUHU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #253
  {DBGFIELD("II_MUL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #254
  {DBGFIELD("II_MULR_PS")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #255
  {DBGFIELD("II_MULU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #256
  {DBGFIELD("II_NMADD_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #257
  {DBGFIELD("II_NMADD_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #258
  {DBGFIELD("II_NMSUB_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #259
  {DBGFIELD("II_NMSUB_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #260
  {DBGFIELD("II_NOR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #261
  {DBGFIELD("II_NOT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #262
  {DBGFIELD("II_OR")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #263
  {DBGFIELD("II_ORI")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #264
  {DBGFIELD("II_PAUSE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #265
  {DBGFIELD("II_PREF")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #266
  {DBGFIELD("II_PREFE")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #267
  {DBGFIELD("II_RDHWR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #268
  {DBGFIELD("II_RDPGPR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #269
  {DBGFIELD("II_RECIP_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #270
  {DBGFIELD("II_RECIP_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #271
  {DBGFIELD("II_RINT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #272
  {DBGFIELD("II_RINT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #273
  {DBGFIELD("II_ROTR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #274
  {DBGFIELD("II_ROTRV")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #275
  {DBGFIELD("II_ROUND")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #276
  {DBGFIELD("II_RSQRT_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #277
  {DBGFIELD("II_RSQRT_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #278
  {DBGFIELD("II_RESTORE")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #279
  {DBGFIELD("II_SB")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #280
  {DBGFIELD("II_SBE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #281
  {DBGFIELD("II_SC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #282
  {DBGFIELD("II_SCD")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #283
  {DBGFIELD("II_SCE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #284
  {DBGFIELD("II_SD")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #285
  {DBGFIELD("II_SDBBP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #286
  {DBGFIELD("II_SDC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #287
  {DBGFIELD("II_SDC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #288
  {DBGFIELD("II_SDC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #289
  {DBGFIELD("II_SDL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #290
  {DBGFIELD("II_SDR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #291
  {DBGFIELD("II_SDXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #292
  {DBGFIELD("II_SEB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #293
  {DBGFIELD("II_SEH")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #294
  {DBGFIELD("II_SELCCZ")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #295
  {DBGFIELD("II_SELCCZ_D")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #296
  {DBGFIELD("II_SELCCZ_S")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #297
  {DBGFIELD("II_SEL_D")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #298
  {DBGFIELD("II_SEL_S")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #299
  {DBGFIELD("II_SEQ_SNE")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #300
  {DBGFIELD("II_SEQI_SNEI")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #301
  {DBGFIELD("II_SH")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #302
  {DBGFIELD("II_SHE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #303
  {DBGFIELD("II_SIGRIE")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #304
  {DBGFIELD("II_SLL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #305
  {DBGFIELD("II_SLLV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #306
  {DBGFIELD("II_SLT_SLTU")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #307
  {DBGFIELD("II_SLTI_SLTIU")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #308
  {DBGFIELD("II_SRA")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #309
  {DBGFIELD("II_SRAV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #310
  {DBGFIELD("II_SRL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #311
  {DBGFIELD("II_SRLV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #312
  {DBGFIELD("II_SSNOP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #313
  {DBGFIELD("II_SUB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #314
  {DBGFIELD("II_SUBU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #315
  {DBGFIELD("II_SUXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #316
  {DBGFIELD("II_SW")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #317
  {DBGFIELD("II_SWC1")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #318
  {DBGFIELD("II_SWC2")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #319
  {DBGFIELD("II_SWC3")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #320
  {DBGFIELD("II_SWE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #321
  {DBGFIELD("II_SWL")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #322
  {DBGFIELD("II_SWLE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #323
  {DBGFIELD("II_SWM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #324
  {DBGFIELD("II_SWP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #325
  {DBGFIELD("II_SWR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #326
  {DBGFIELD("II_SWRE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #327
  {DBGFIELD("II_SWXC1")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #328
  {DBGFIELD("II_SYNC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #329
  {DBGFIELD("II_SYNCI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #330
  {DBGFIELD("II_SYSCALL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #331
  {DBGFIELD("II_SAVE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #332
  {DBGFIELD("II_TEQ")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #333
  {DBGFIELD("II_TEQI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #334
  {DBGFIELD("II_TGE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #335
  {DBGFIELD("II_TGEI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #336
  {DBGFIELD("II_TGEIU")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #337
  {DBGFIELD("II_TGEU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #338
  {DBGFIELD("II_TLBGINV")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #339
  {DBGFIELD("II_TLBGINVF")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #340
  {DBGFIELD("II_TLBGP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #341
  {DBGFIELD("II_TLBGR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #342
  {DBGFIELD("II_TLBGWI")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #343
  {DBGFIELD("II_TLBGWR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #344
  {DBGFIELD("II_TLBINV")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #345
  {DBGFIELD("II_TLBINVF")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #346
  {DBGFIELD("II_TLBP")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #347
  {DBGFIELD("II_TLBR")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #348
  {DBGFIELD("II_TLBWI")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #349
  {DBGFIELD("II_TLBWR")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #350
  {DBGFIELD("II_TLT")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #351
  {DBGFIELD("II_TLTI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #352
  {DBGFIELD("II_TTLTIU")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #353
  {DBGFIELD("II_TLTU")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #354
  {DBGFIELD("II_TNE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #355
  {DBGFIELD("II_TNEI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #356
  {DBGFIELD("II_TRUNC")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #357
  {DBGFIELD("II_WAIT")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #358
  {DBGFIELD("II_WRPGPR")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #359
  {DBGFIELD("II_WSBH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #360
  {DBGFIELD("II_XOR")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #361
  {DBGFIELD("II_XORI")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #362
  {DBGFIELD("II_YIELD")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #363
  {DBGFIELD("AND")                1, false, false, false,  5, 2,  1, 1,  0, 0}, // #364
  {DBGFIELD("LUi")                1, false, false, false,  5, 2,  1, 1,  0, 0}, // #365
  {DBGFIELD("NOR")                1, false, false, false,  5, 2,  1, 1,  0, 0}, // #366
  {DBGFIELD("OR")                 1, false, false, false,  5, 2,  1, 1,  0, 0}, // #367
  {DBGFIELD("SLTi_SLTiu")         1, false, false, false,  5, 2,  1, 1,  0, 0}, // #368
  {DBGFIELD("SUB")                1, false, false, false,  5, 2,  1, 1,  0, 0}, // #369
  {DBGFIELD("SUBu")               1, false, false, false,  5, 2,  1, 1,  0, 0}, // #370
  {DBGFIELD("XOR")                1, false, false, false,  5, 2,  1, 1,  0, 0}, // #371
  {DBGFIELD("SSNOP")              1, false, false, false,  3, 2,  0, 1,  0, 0}, // #372
  {DBGFIELD("NOP")                1, false, false, false,  3, 2,  0, 1,  0, 0}, // #373
  {DBGFIELD("B")                  1, false, false, false, 37, 3,  1, 1,  0, 0}, // #374
  {DBGFIELD("BAL")                1, false, false, false, 37, 3,  1, 1,  0, 0}, // #375
  {DBGFIELD("BAL_BR_BGEZAL_BGEZALL_BLTZAL_BLTZALL") 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #376
  {DBGFIELD("BEQ_BEQL_BNE_BNEL")  1, false, false, false, 37, 3,  1, 1,  0, 0}, // #377
  {DBGFIELD("BGEZ_BGEZL_BGTZ_BGTZL_BLEZ_BLEZL_BLTZ_BLTZL") 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #378
  {DBGFIELD("BREAK")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #379
  {DBGFIELD("DERET")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #380
  {DBGFIELD("ERET")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #381
  {DBGFIELD("ERet_RetRA")         1, false, false, false, 37, 3,  1, 1,  0, 0}, // #382
  {DBGFIELD("ERETNC")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #383
  {DBGFIELD("J_TAILCALL")         1, false, false, false, 37, 3,  1, 1,  0, 0}, // #384
  {DBGFIELD("JR_TAILCALLREG_TAILCALLREGHB") 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #385
  {DBGFIELD("JR_HB")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #386
  {DBGFIELD("PseudoIndirectBranch_PseudoIndirectHazardBranch") 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #387
  {DBGFIELD("PseudoReturn")       1, false, false, false, 37, 3,  1, 1,  0, 0}, // #388
  {DBGFIELD("SDBBP")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #389
  {DBGFIELD("SYSCALL")            1, false, false, false, 37, 3,  1, 1,  0, 0}, // #390
  {DBGFIELD("TEQ")                1, false, false, false, 37, 3,  1, 1,  0, 0}, // #391
  {DBGFIELD("TEQI")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #392
  {DBGFIELD("TGE")                1, false, false, false, 37, 3,  1, 1,  0, 0}, // #393
  {DBGFIELD("TGEI")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #394
  {DBGFIELD("TGEIU")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #395
  {DBGFIELD("TGEU")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #396
  {DBGFIELD("TLT")                1, false, false, false, 37, 3,  1, 1,  0, 0}, // #397
  {DBGFIELD("TLTI")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #398
  {DBGFIELD("TLTU")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #399
  {DBGFIELD("TNE")                1, false, false, false, 37, 3,  1, 1,  0, 0}, // #400
  {DBGFIELD("TNEI")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #401
  {DBGFIELD("TRAP")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #402
  {DBGFIELD("TTLTIU")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #403
  {DBGFIELD("WAIT")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #404
  {DBGFIELD("PAUSE")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #405
  {DBGFIELD("JAL")                1, false, false, false, 37, 3,  2, 1,  0, 0}, // #406
  {DBGFIELD("JALR_JALRHBPseudo_JALRPseudo") 1, false, false, false, 37, 3,  2, 1,  0, 0}, // #407
  {DBGFIELD("JALR_HB")            1, false, false, false, 37, 3,  2, 1,  0, 0}, // #408
  {DBGFIELD("JALX")               1, false, false, false, 37, 3,  2, 1,  0, 0}, // #409
  {DBGFIELD("TLBINV")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #410
  {DBGFIELD("TLBINVF")            1, false, false, false, 37, 3,  1, 1,  0, 0}, // #411
  {DBGFIELD("TLBP")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #412
  {DBGFIELD("TLBR")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #413
  {DBGFIELD("TLBWI")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #414
  {DBGFIELD("TLBWR")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #415
  {DBGFIELD("MFC0")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #416
  {DBGFIELD("MTC0")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #417
  {DBGFIELD("MFC2")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #418
  {DBGFIELD("MTC2")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #419
  {DBGFIELD("HYPCALL")            1, false, false, false, 37, 3,  1, 1,  0, 0}, // #420
  {DBGFIELD("MFGC0")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #421
  {DBGFIELD("MFHGC0")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #422
  {DBGFIELD("MTGC0")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #423
  {DBGFIELD("MTHGC0")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #424
  {DBGFIELD("TLBGINV")            1, false, false, false, 37, 3,  1, 1,  0, 0}, // #425
  {DBGFIELD("TLBGINVF")           1, false, false, false, 37, 3,  1, 1,  0, 0}, // #426
  {DBGFIELD("TLBGP")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #427
  {DBGFIELD("TLBGR")              1, false, false, false, 37, 3,  1, 1,  0, 0}, // #428
  {DBGFIELD("TLBGWI")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #429
  {DBGFIELD("TLBGWR")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #430
  {DBGFIELD("LB")                 1, false, false, false, 40, 2,  4, 1,  0, 0}, // #431
  {DBGFIELD("LBu")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #432
  {DBGFIELD("LH")                 1, false, false, false, 40, 2,  4, 1,  0, 0}, // #433
  {DBGFIELD("LHu")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #434
  {DBGFIELD("LW")                 1, false, false, false, 40, 2,  4, 1,  0, 0}, // #435
  {DBGFIELD("LL")                 1, false, false, false, 40, 2,  4, 1,  0, 0}, // #436
  {DBGFIELD("LWC2")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #437
  {DBGFIELD("LWC3")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #438
  {DBGFIELD("LDC2")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #439
  {DBGFIELD("LDC3")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #440
  {DBGFIELD("LBE")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #441
  {DBGFIELD("LBuE")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #442
  {DBGFIELD("LHE")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #443
  {DBGFIELD("LHuE")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #444
  {DBGFIELD("LWE")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #445
  {DBGFIELD("LLE")                1, false, false, false, 40, 2,  4, 1,  0, 0}, // #446
  {DBGFIELD("LWPC")               1, false, false, false, 40, 2,  4, 1,  0, 0}, // #447
  {DBGFIELD("LWL")                1, false, false, false, 42, 3,  4, 1,  0, 0}, // #448
  {DBGFIELD("LWR")                1, false, false, false, 42, 3,  4, 1,  0, 0}, // #449
  {DBGFIELD("LWLE")               1, false, false, false, 42, 3,  4, 1,  0, 0}, // #450
  {DBGFIELD("LWRE")               1, false, false, false, 42, 3,  4, 1,  0, 0}, // #451
  {DBGFIELD("SB")                 1, false, false, false, 45, 3,  1, 1,  0, 0}, // #452
  {DBGFIELD("SH")                 1, false, false, false, 45, 3,  1, 1,  0, 0}, // #453
  {DBGFIELD("SW")                 1, false, false, false, 45, 3,  1, 1,  0, 0}, // #454
  {DBGFIELD("SWC2")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #455
  {DBGFIELD("SWC3")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #456
  {DBGFIELD("SDC2")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #457
  {DBGFIELD("SDC3")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #458
  {DBGFIELD("SC")                 1, false, false, false, 45, 3,  1, 1,  0, 0}, // #459
  {DBGFIELD("SBE")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #460
  {DBGFIELD("SHE")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #461
  {DBGFIELD("SWE")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #462
  {DBGFIELD("SCE")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #463
  {DBGFIELD("SWL")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #464
  {DBGFIELD("SWR")                1, false, false, false, 45, 3,  1, 1,  0, 0}, // #465
  {DBGFIELD("SWLE")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #466
  {DBGFIELD("SWRE")               1, false, false, false, 45, 3,  1, 1,  0, 0}, // #467
  {DBGFIELD("PREF")               1, false, false, false, 40, 2,  1, 1,  0, 0}, // #468
  {DBGFIELD("PREFE")              1, false, false, false, 40, 2,  1, 1,  0, 0}, // #469
  {DBGFIELD("CACHE")              1, false, false, false, 40, 2,  1, 1,  0, 0}, // #470
  {DBGFIELD("CACHEE")             1, false, false, false, 40, 2,  1, 1,  0, 0}, // #471
  {DBGFIELD("SYNC")               1, false, false, false, 40, 2,  1, 1,  0, 0}, // #472
  {DBGFIELD("SYNCI")              1, false, false, false, 40, 2,  1, 1,  0, 0}, // #473
  {DBGFIELD("CLO")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #474
  {DBGFIELD("CLZ")                1, false, false, false,  1, 2,  1, 1,  0, 0}, // #475
  {DBGFIELD("DI")                 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #476
  {DBGFIELD("EI")                 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #477
  {DBGFIELD("MFHI_MFLO_PseudoMFHI_PseudoMFLO") 1, false, false, false,  1, 2,  1, 1,  0, 0}, // #478
  {DBGFIELD("EHB")                1, false, false, false,  1, 2,  2, 1,  0, 0}, // #479
  {DBGFIELD("RDHWR")              1, false, false, false,  1, 2,  2, 1,  0, 0}, // #480
  {DBGFIELD("WSBH")               1, false, false, false,  1, 2,  2, 1,  0, 0}, // #481
  {DBGFIELD("MOVN_I_I")           1, false, false, false, 48, 3,  2, 1,  0, 0}, // #482
  {DBGFIELD("MOVZ_I_I")           1, false, false, false, 48, 3,  2, 1,  0, 0}, // #483
  {DBGFIELD("DIV_PseudoSDIV_SDIV") 1, false, false, false, 51, 3, 14, 1,  0, 0}, // #484
  {DBGFIELD("DIVU_PseudoUDIV_UDIV") 1, false, false, false, 51, 3, 14, 1,  0, 0}, // #485
  {DBGFIELD("MUL")                1, false, false, false,  1, 2,  3, 1,  0, 0}, // #486
  {DBGFIELD("MULT_PseudoMULT")    1, false, false, false,  1, 2,  5, 1,  0, 0}, // #487
  {DBGFIELD("MULTu_PseudoMULTu")  1, false, false, false,  1, 2,  5, 1,  0, 0}, // #488
  {DBGFIELD("MADD_PseudoMADD")    1, false, false, false, 48, 3,  5, 1,  0, 0}, // #489
  {DBGFIELD("MADDU_PseudoMADDU")  1, false, false, false, 48, 3,  5, 1,  0, 0}, // #490
  {DBGFIELD("MSUB_PseudoMSUB")    1, false, false, false, 48, 3,  5, 1,  0, 0}, // #491
  {DBGFIELD("MSUBU_PseudoMSUBU")  1, false, false, false, 48, 3,  5, 1,  0, 0}, // #492
  {DBGFIELD("MTHI_MTLO_PseudoMTLOHI") 1, false, false, false, 48, 3,  5, 1,  0, 0}, // #493
  {DBGFIELD("EXT")                1, false, false, false,  1, 2,  2, 1,  0, 0}, // #494
  {DBGFIELD("INS")                1, false, false, false,  1, 2,  2, 1,  0, 0}, // #495
  {DBGFIELD("ADD")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #496
  {DBGFIELD("ADDi")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #497
  {DBGFIELD("ADDiu")              8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #498
  {DBGFIELD("ANDi")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #499
  {DBGFIELD("ORi")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #500
  {DBGFIELD("ROTR")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #501
  {DBGFIELD("SEB")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #502
  {DBGFIELD("SEH")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #503
  {DBGFIELD("SLT_SLTu")           8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #504
  {DBGFIELD("SLL")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #505
  {DBGFIELD("SRA")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #506
  {DBGFIELD("SRL")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #507
  {DBGFIELD("XORi")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #508
  {DBGFIELD("ADDu")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #509
  {DBGFIELD("SLLV")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #510
  {DBGFIELD("SRAV")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #511
  {DBGFIELD("SRLV")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #512
  {DBGFIELD("LSA")                8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #513
  {DBGFIELD("COPY")               8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #514
  {DBGFIELD("VSHF_B_VSHF_D_VSHF_H_VSHF_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #515
  {DBGFIELD("BINSLI_B_BINSLI_D_BINSLI_H_BINSLI_W_BINSL_B_BINSL_D_BINSL_H_BINSL_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #516
  {DBGFIELD("BINSRI_B_BINSRI_D_BINSRI_H_BINSRI_W_BINSR_B_BINSR_D_BINSR_H_BINSR_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #517
  {DBGFIELD("INSERT_B_INSERT_D_INSERT_H_INSERT_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #518
  {DBGFIELD("SLDI_B_SLDI_D_SLDI_H_SLDI_W_SLD_B_SLD_D_SLD_H_SLD_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #519
  {DBGFIELD("BSETI_B_BSETI_D_BSETI_H_BSETI_W_BSET_B_BSET_D_BSET_H_BSET_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #520
  {DBGFIELD("BCLRI_B_BCLRI_D_BCLRI_H_BCLRI_W_BCLR_B_BCLR_D_BCLR_H_BCLR_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #521
  {DBGFIELD("BNEGI_B_BNEGI_D_BNEGI_H_BNEGI_W_BNEG_B_BNEG_D_BNEG_H_BNEG_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #522
  {DBGFIELD("BSELI_B_BSEL_V")     1, false, false, false, 31, 2,  2, 1,  0, 0}, // #523
  {DBGFIELD("BMNZI_B_BMNZ_V_BMZI_B_BMZ_V") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #524
  {DBGFIELD("BSEL_D_PSEUDO_BSEL_FD_PSEUDO_BSEL_FW_PSEUDO_BSEL_H_PSEUDO_BSEL_W_PSEUDO") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #525
  {DBGFIELD("PCNT_B_PCNT_D_PCNT_H_PCNT_W") 1, false, false, false, 31, 2,  3, 1,  0, 0}, // #526
  {DBGFIELD("SAT_S_B_SAT_S_D_SAT_S_H_SAT_S_W_SAT_U_B_SAT_U_D_SAT_U_H_SAT_U_W") 1, false, false, false, 31, 2,  3, 1,  0, 0}, // #527
  {DBGFIELD("BNZ_B_BNZ_D_BNZ_H_BNZ_V_BNZ_W_BZ_B_BZ_D_BZ_H_BZ_V_BZ_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #528
  {DBGFIELD("CFCMSA_CTCMSA")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #529
  {DBGFIELD("FABS_S_FABS_D32_FABS_D64") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #530
  {DBGFIELD("MOVF_D32_MOVF_D64")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #531
  {DBGFIELD("MOVF_S")             1, false, false, false, 31, 2,  1, 1,  0, 0}, // #532
  {DBGFIELD("MOVT_D32_MOVT_D64")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #533
  {DBGFIELD("MOVT_S")             1, false, false, false, 31, 2,  1, 1,  0, 0}, // #534
  {DBGFIELD("FMOV_D32_FMOV_D64")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #535
  {DBGFIELD("FMOV_S")             1, false, false, false, 31, 2,  1, 1,  0, 0}, // #536
  {DBGFIELD("FNEG_S_FNEG_D32_FNEG_D64") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #537
  {DBGFIELD("ADD_A_B_ADD_A_D_ADD_A_H_ADD_A_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #538
  {DBGFIELD("ADDS_A_B_ADDS_A_D_ADDS_A_H_ADDS_A_W_ADDS_S_B_ADDS_S_D_ADDS_S_H_ADDS_S_W_ADDS_U_B_ADDS_U_D_ADDS_U_H_ADDS_U_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #539
  {DBGFIELD("ADDVI_B_ADDVI_D_ADDVI_H_ADDVI_W_ADDV_B_ADDV_D_ADDV_H_ADDV_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #540
  {DBGFIELD("ASUB_S_B_ASUB_S_D_ASUB_S_H_ASUB_S_W_ASUB_U_B_ASUB_U_D_ASUB_U_H_ASUB_U_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #541
  {DBGFIELD("AVER_S_B_AVER_S_D_AVER_S_H_AVER_S_W_AVER_U_B_AVER_U_D_AVER_U_H_AVER_U_W_AVE_S_B_AVE_S_D_AVE_S_H_AVE_S_W_AVE_U_B_AVE_U_D_AVE_U_H_AVE_U_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #542
  {DBGFIELD("SHF_B_SHF_H_SHF_W")  1, false, false, false, 31, 2,  2, 1,  0, 0}, // #543
  {DBGFIELD("FILL_B_FILL_D_FILL_H_FILL_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #544
  {DBGFIELD("SPLATI_B_SPLATI_D_SPLATI_H_SPLATI_W_SPLAT_B_SPLAT_D_SPLAT_H_SPLAT_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #545
  {DBGFIELD("MOVE_V")             1, false, false, false, 31, 2,  1, 1,  0, 0}, // #546
  {DBGFIELD("LDI_B_LDI_D_LDI_H_LDI_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #547
  {DBGFIELD("AND_V_NOR_V_OR_V_XOR_V") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #548
  {DBGFIELD("ANDI_B_NORI_B_ORI_B_XORI_B") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #549
  {DBGFIELD("AND_V_D_PSEUDO_AND_V_H_PSEUDO_AND_V_W_PSEUDO_NOR_V_D_PSEUDO_NOR_V_H_PSEUDO_NOR_V_W_PSEUDO_OR_V_D_PSEUDO_OR_V_H_PSEUDO_OR_V_W_PSEUDO_XOR_V_D_PSEUDO_XOR_V_H_PSEUDO_XOR_V_W_PSEUDO") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #550
  {DBGFIELD("FILL_FD_PSEUDO_FILL_FW_PSEUDO") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #551
  {DBGFIELD("INSERT_FD_PSEUDO_INSERT_FW_PSEUDO") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #552
  {DBGFIELD("FEXP2_D_FEXP2_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #553
  {DBGFIELD("CLTI_S_B_CLTI_S_D_CLTI_S_H_CLTI_S_W_CLTI_U_B_CLTI_U_D_CLTI_U_H_CLTI_U_W_CLT_S_B_CLT_S_D_CLT_S_H_CLT_S_W_CLT_U_B_CLT_U_D_CLT_U_H_CLT_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #554
  {DBGFIELD("CLEI_S_B_CLEI_S_D_CLEI_S_H_CLEI_S_W_CLEI_U_B_CLEI_U_D_CLEI_U_H_CLEI_U_W_CLE_S_B_CLE_S_D_CLE_S_H_CLE_S_W_CLE_U_B_CLE_U_D_CLE_U_H_CLE_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #555
  {DBGFIELD("CEQI_B_CEQI_D_CEQI_H_CEQI_W_CEQ_B_CEQ_D_CEQ_H_CEQ_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #556
  {DBGFIELD("CMP_UN_D")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #557
  {DBGFIELD("CMP_UN_S")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #558
  {DBGFIELD("CMP_UEQ_D")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #559
  {DBGFIELD("CMP_UEQ_S")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #560
  {DBGFIELD("CMP_EQ_D")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #561
  {DBGFIELD("CMP_EQ_S")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #562
  {DBGFIELD("CMP_LT_D")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #563
  {DBGFIELD("CMP_LT_S")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #564
  {DBGFIELD("CMP_ULT_D")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #565
  {DBGFIELD("CMP_ULT_S")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #566
  {DBGFIELD("CMP_LE_D")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #567
  {DBGFIELD("CMP_LE_S")           1, false, false, false, 31, 2,  1, 1,  0, 0}, // #568
  {DBGFIELD("CMP_ULE_D")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #569
  {DBGFIELD("CMP_ULE_S")          1, false, false, false, 31, 2,  1, 1,  0, 0}, // #570
  {DBGFIELD("FSAF_D_FSAF_W_FSEQ_D_FSEQ_W_FSLE_D_FSLE_W_FSLT_D_FSLT_W_FSNE_D_FSNE_W_FSOR_D_FSOR_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #571
  {DBGFIELD("FSUEQ_D_FSUEQ_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #572
  {DBGFIELD("FSULE_D_FSULE_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #573
  {DBGFIELD("FSULT_D_FSULT_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #574
  {DBGFIELD("FSUNE_D_FSUNE_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #575
  {DBGFIELD("FSUN_D_FSUN_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #576
  {DBGFIELD("FCAF_D_FCAF_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #577
  {DBGFIELD("FCEQ_D_FCEQ_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #578
  {DBGFIELD("FCLE_D_FCLE_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #579
  {DBGFIELD("FCLT_D_FCLT_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #580
  {DBGFIELD("FCNE_D_FCNE_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #581
  {DBGFIELD("FCOR_D_FCOR_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #582
  {DBGFIELD("FCUEQ_D_FCUEQ_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #583
  {DBGFIELD("FCULE_D_FCULE_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #584
  {DBGFIELD("FCULT_D_FCULT_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #585
  {DBGFIELD("FCUNE_D_FCUNE_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #586
  {DBGFIELD("FCUN_D_FCUN_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #587
  {DBGFIELD("FABS_D_FABS_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #588
  {DBGFIELD("FFINT_S_D_FFINT_S_W_FFINT_U_D_FFINT_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #589
  {DBGFIELD("FFQL_D_FFQL_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #590
  {DBGFIELD("FFQR_D_FFQR_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #591
  {DBGFIELD("FTINT_S_D_FTINT_S_W_FTINT_U_D_FTINT_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #592
  {DBGFIELD("FRINT_D_FRINT_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #593
  {DBGFIELD("FTQ_H_FTQ_W")        1, false, false, false, 31, 2,  1, 1,  0, 0}, // #594
  {DBGFIELD("FTRUNC_S_D_FTRUNC_S_W_FTRUNC_U_D_FTRUNC_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #595
  {DBGFIELD("FEXDO_H_FEXDO_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #596
  {DBGFIELD("FEXUPL_D_FEXUPL_W")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #597
  {DBGFIELD("FEXUPR_D_FEXUPR_W")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #598
  {DBGFIELD("FCLASS_D_FCLASS_W")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #599
  {DBGFIELD("FMAX_A_D_FMAX_A_W")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #600
  {DBGFIELD("FMAX_D_FMAX_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #601
  {DBGFIELD("FMIN_A_D_FMIN_A_W")  1, false, false, false, 31, 2,  1, 1,  0, 0}, // #602
  {DBGFIELD("FMIN_D_FMIN_W")      1, false, false, false, 31, 2,  1, 1,  0, 0}, // #603
  {DBGFIELD("FLOG2_D_FLOG2_W")    1, false, false, false, 31, 2,  1, 1,  0, 0}, // #604
  {DBGFIELD("ILVL_B_ILVL_D_ILVL_H_ILVL_W_ILVR_B_ILVR_D_ILVR_H_ILVR_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #605
  {DBGFIELD("ILVEV_B_ILVEV_D_ILVEV_H_ILVEV_W_ILVOD_B_ILVOD_D_ILVOD_H_ILVOD_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #606
  {DBGFIELD("INSVE_B_INSVE_D_INSVE_H_INSVE_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #607
  {DBGFIELD("SUBS_S_B_SUBS_S_D_SUBS_S_H_SUBS_S_W_SUBS_U_B_SUBS_U_D_SUBS_U_H_SUBS_U_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #608
  {DBGFIELD("SUBSUS_U_B_SUBSUS_U_D_SUBSUS_U_H_SUBSUS_U_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #609
  {DBGFIELD("SUBSUU_S_B_SUBSUU_S_D_SUBSUU_S_H_SUBSUU_S_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #610
  {DBGFIELD("SUBVI_B_SUBVI_D_SUBVI_H_SUBVI_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #611
  {DBGFIELD("SUBV_B_SUBV_D_SUBV_H_SUBV_W") 1, false, false, false, 31, 2,  2, 1,  0, 0}, // #612
  {DBGFIELD("MOD_S_B_MOD_S_D_MOD_S_H_MOD_S_W_MOD_U_B_MOD_U_D_MOD_U_H_MOD_U_W") 1, false, false, false, 54, 3, 15, 1,  0, 0}, // #613
  {DBGFIELD("DIV_S_B_DIV_S_D_DIV_S_H_DIV_S_W_DIV_U_B_DIV_U_D_DIV_U_H_DIV_U_W") 1, false, false, false, 54, 3, 15, 1,  0, 0}, // #614
  {DBGFIELD("HADD_S_D_HADD_S_H_HADD_S_W_HADD_U_D_HADD_U_H_HADD_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #615
  {DBGFIELD("HSUB_S_D_HSUB_S_H_HSUB_S_W_HSUB_U_D_HSUB_U_H_HSUB_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #616
  {DBGFIELD("MAX_S_B_MAX_S_D_MAX_S_H_MAX_S_W_MIN_S_B_MIN_S_D_MIN_S_H_MIN_S_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #617
  {DBGFIELD("MAX_U_B_MAX_U_D_MAX_U_H_MAX_U_W_MIN_U_B_MIN_U_D_MIN_U_H_MIN_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #618
  {DBGFIELD("MAX_A_B_MAX_A_D_MAX_A_H_MAX_A_W_MIN_A_B_MIN_A_D_MIN_A_H_MIN_A_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #619
  {DBGFIELD("MAXI_S_B_MAXI_S_D_MAXI_S_H_MAXI_S_W_MAXI_U_B_MAXI_U_D_MAXI_U_H_MAXI_U_W_MINI_S_B_MINI_S_D_MINI_S_H_MINI_S_W_MINI_U_B_MINI_U_D_MINI_U_H_MINI_U_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #620
  {DBGFIELD("SRAI_B_SRAI_D_SRAI_H_SRAI_W_SRA_B_SRA_D_SRA_H_SRA_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #621
  {DBGFIELD("SRLI_B_SRLI_D_SRLI_H_SRLI_W_SRL_B_SRL_D_SRL_H_SRL_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #622
  {DBGFIELD("SRARI_B_SRARI_D_SRARI_H_SRARI_W_SRAR_B_SRAR_D_SRAR_H_SRAR_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #623
  {DBGFIELD("SRLRI_B_SRLRI_D_SRLRI_H_SRLRI_W_SRLR_B_SRLR_D_SRLR_H_SRLR_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #624
  {DBGFIELD("SLLI_B_SLLI_D_SLLI_H_SLLI_W_SLL_B_SLL_D_SLL_H_SLL_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #625
  {DBGFIELD("PCKEV_B_PCKEV_D_PCKEV_H_PCKEV_W_PCKOD_B_PCKOD_D_PCKOD_H_PCKOD_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #626
  {DBGFIELD("NLOC_B_NLOC_D_NLOC_H_NLOC_W_NLZC_B_NLZC_D_NLZC_H_NLZC_W") 1, false, false, false, 31, 2,  1, 1,  0, 0}, // #627
  {DBGFIELD("FADD_D32_FADD_D64")  1, false, false, false, 57, 2,  4, 1,  0, 0}, // #628
  {DBGFIELD("FADD_PS64")          1, false, false, false, 57, 2,  4, 1,  0, 0}, // #629
  {DBGFIELD("FADD_S")             1, false, false, false, 57, 2,  4, 1,  0, 0}, // #630
  {DBGFIELD("FMUL_D32_FMUL_D64")  1, false, false, false, 57, 2,  4, 1,  0, 0}, // #631
  {DBGFIELD("FMUL_PS64")          1, false, false, false, 57, 2,  4, 1,  0, 0}, // #632
  {DBGFIELD("FMUL_S")             1, false, false, false, 57, 2,  4, 1,  0, 0}, // #633
  {DBGFIELD("FSUB_D32_FSUB_D64")  1, false, false, false, 57, 2,  4, 1,  0, 0}, // #634
  {DBGFIELD("FSUB_PS64")          1, false, false, false, 57, 2,  4, 1,  0, 0}, // #635
  {DBGFIELD("FSUB_S")             1, false, false, false, 57, 2,  4, 1,  0, 0}, // #636
  {DBGFIELD("TRUNC_L_D64_TRUNC_L_S_TRUNC_W_D32_TRUNC_W_D64_TRUNC_W_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #637
  {DBGFIELD("CVT_D32_S_CVT_D32_W_CVT_D64_L_CVT_D64_S_CVT_D64_W_CVT_L_D64_CVT_L_S_CVT_S_D32_CVT_S_D64_CVT_S_L_CVT_S_W_CVT_W_D32_CVT_W_D64_CVT_W_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #638
  {DBGFIELD("CVT_PS_S64_CVT_S_PL64_CVT_S_PU64") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #639
  {DBGFIELD("C_EQ_D32_C_EQ_D64_C_F_D32_C_F_D64_C_LE_D32_C_LE_D64_C_LT_D32_C_LT_D64_C_NGE_D32_C_NGE_D64_C_NGLE_D32_C_NGLE_D64_C_NGL_D32_C_NGL_D64_C_NGT_D32_C_NGT_D64_C_OLE_D32_C_OLE_D64_C_OLT_D32_C_OLT_D64_C_SEQ_D32_C_SEQ_D64_C_SF_D32_C_SF_D64_C_UEQ_D32_C_UEQ_D64_C_ULE_D32_C_ULE_D64_C_ULT_D32_C_ULT_D64_C_UN_D32_C_UN_D64") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #640
  {DBGFIELD("C_EQ_S_C_F_S_C_LE_S_C_LT_S_C_NGE_S_C_NGLE_S_C_NGL_S_C_NGT_S_C_OLE_S_C_OLT_S_C_SEQ_S_C_SF_S_C_UEQ_S_C_ULE_S_C_ULT_S_C_UN_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #641
  {DBGFIELD("FCMP_D32_FCMP_D64")  1, false, false, false, 57, 2,  4, 1,  0, 0}, // #642
  {DBGFIELD("FCMP_S32")           1, false, false, false, 57, 2,  4, 1,  0, 0}, // #643
  {DBGFIELD("PseudoCVT_D32_W_PseudoCVT_D64_L_PseudoCVT_D64_W_PseudoCVT_S_L_PseudoCVT_S_W") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #644
  {DBGFIELD("PLL_PS64_PLU_PS64_PUL_PS64_PUU_PS64") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #645
  {DBGFIELD("FDIV_S")             1, false, false, false, 54, 3, 15, 1,  0, 0}, // #646
  {DBGFIELD("FDIV_D32_FDIV_D64")  1, false, false, false, 59, 3, 12, 1,  0, 0}, // #647
  {DBGFIELD("FSQRT_S")            1, false, false, false, 62, 3, 16, 1,  0, 0}, // #648
  {DBGFIELD("FSQRT_D32_FSQRT_D64") 1, false, false, false, 65, 3, 17, 1,  0, 0}, // #649
  {DBGFIELD("FRCP_D_FRCP_W")      1, false, false, false, 62, 3, 16, 1,  0, 0}, // #650
  {DBGFIELD("FRSQRT_D_FRSQRT_W")  1, false, false, false, 62, 3, 16, 1,  0, 0}, // #651
  {DBGFIELD("RECIP_D32_RECIP_D64") 1, false, false, false, 62, 3, 16, 1,  0, 0}, // #652
  {DBGFIELD("RSQRT_D32_RSQRT_D64") 1, false, false, false, 62, 3, 16, 1,  0, 0}, // #653
  {DBGFIELD("RECIP_S")            1, false, false, false, 62, 3, 16, 1,  0, 0}, // #654
  {DBGFIELD("RSQRT_S")            1, false, false, false, 62, 3, 16, 1,  0, 0}, // #655
  {DBGFIELD("FMADD_D_FMADD_W")    1, false, false, false, 57, 2, 13, 1,  0, 0}, // #656
  {DBGFIELD("FMSUB_D_FMSUB_W")    1, false, false, false, 57, 2, 13, 1,  0, 0}, // #657
  {DBGFIELD("FDIV_W")             1, false, false, false, 54, 3, 15, 1,  0, 0}, // #658
  {DBGFIELD("FDIV_D")             1, false, false, false, 59, 3, 12, 1,  0, 0}, // #659
  {DBGFIELD("FSQRT_W")            1, false, false, false, 62, 3, 16, 1,  0, 0}, // #660
  {DBGFIELD("FSQRT_D")            1, false, false, false, 65, 3, 17, 1,  0, 0}, // #661
  {DBGFIELD("FMUL_D_FMUL_W")      1, false, false, false, 57, 2,  4, 1,  0, 0}, // #662
  {DBGFIELD("FADD_D_FADD_W")      1, false, false, false, 57, 2,  4, 1,  0, 0}, // #663
  {DBGFIELD("FSUB_D_FSUB_W")      1, false, false, false, 57, 2,  4, 1,  0, 0}, // #664
  {DBGFIELD("DPADD_S_D_DPADD_S_H_DPADD_S_W_DPADD_U_D_DPADD_U_H_DPADD_U_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #665
  {DBGFIELD("DPSUB_S_D_DPSUB_S_H_DPSUB_S_W_DPSUB_U_D_DPSUB_U_H_DPSUB_U_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #666
  {DBGFIELD("DOTP_S_D_DOTP_S_H_DOTP_S_W_DOTP_U_D_DOTP_U_H_DOTP_U_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #667
  {DBGFIELD("MSUBV_B_MSUBV_D_MSUBV_H_MSUBV_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #668
  {DBGFIELD("MADDV_B_MADDV_D_MADDV_H_MADDV_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #669
  {DBGFIELD("MULV_B_MULV_D_MULV_H_MULV_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #670
  {DBGFIELD("MADDR_Q_H_MADDR_Q_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #671
  {DBGFIELD("MADD_Q_H_MADD_Q_W")  1, false, false, false, 31, 2,  5, 1,  0, 0}, // #672
  {DBGFIELD("MSUBR_Q_H_MSUBR_Q_W") 1, false, false, false, 31, 2,  5, 1,  0, 0}, // #673
  {DBGFIELD("MSUB_Q_H_MSUB_Q_W")  1, false, false, false, 31, 2,  5, 1,  0, 0}, // #674
  {DBGFIELD("MULR_Q_H_MULR_Q_W")  1, false, false, false, 31, 2,  5, 1,  0, 0}, // #675
  {DBGFIELD("MUL_Q_H_MUL_Q_W")    1, false, false, false, 31, 2,  5, 1,  0, 0}, // #676
  {DBGFIELD("MADD_D32_MADD_D64")  1, false, false, false, 57, 2, 13, 1,  1, 1}, // #677
  {DBGFIELD("MADD_S")             1, false, false, false, 57, 2, 13, 1,  1, 1}, // #678
  {DBGFIELD("MSUB_D32_MSUB_D64")  1, false, false, false, 57, 2, 13, 1,  1, 1}, // #679
  {DBGFIELD("MSUB_S")             1, false, false, false, 57, 2, 13, 1,  1, 1}, // #680
  {DBGFIELD("NMADD_D32_NMADD_D64") 1, false, false, false, 57, 2, 13, 1,  1, 1}, // #681
  {DBGFIELD("NMADD_S")            1, false, false, false, 57, 2, 13, 1,  1, 1}, // #682
  {DBGFIELD("NMSUB_D32_NMSUB_D64") 1, false, false, false, 57, 2, 13, 1,  1, 1}, // #683
  {DBGFIELD("NMSUB_S")            1, false, false, false, 57, 2, 13, 1,  1, 1}, // #684
  {DBGFIELD("CTC1")               2, false, false, false, 68, 4,  1, 1,  0, 0}, // #685
  {DBGFIELD("MTC1_MTC1_D64_BuildPairF64_BuildPairF64_64") 2, false, false, false, 68, 4,  1, 1,  0, 0}, // #686
  {DBGFIELD("MTHC1_D32_MTHC1_D64") 2, false, false, false, 68, 4,  1, 1,  0, 0}, // #687
  {DBGFIELD("COPY_U_B_COPY_U_H_COPY_U_W") 2, false, false, false, 68, 4,  2, 1,  0, 0}, // #688
  {DBGFIELD("COPY_S_B_COPY_S_D_COPY_S_H_COPY_S_W") 2, false, false, false, 68, 4,  2, 1,  0, 0}, // #689
  {DBGFIELD("BC1F")               2, false, false, false, 68, 4,  2, 1,  0, 0}, // #690
  {DBGFIELD("BC1FL")              2, false, false, false, 68, 4,  2, 1,  0, 0}, // #691
  {DBGFIELD("BC1T")               2, false, false, false, 68, 4,  2, 1,  0, 0}, // #692
  {DBGFIELD("BC1TL")              2, false, false, false, 68, 4,  2, 1,  0, 0}, // #693
  {DBGFIELD("CFC1")               2, false, false, false, 68, 4,  2, 1,  0, 0}, // #694
  {DBGFIELD("MFC1_MFC1_D64_ExtractElementF64_ExtractElementF64_64") 2, false, false, false, 68, 4,  2, 1,  0, 0}, // #695
  {DBGFIELD("MFHC1_D32_MFHC1_D64") 2, false, false, false, 68, 4,  2, 1,  0, 0}, // #696
  {DBGFIELD("MOVF_I")             2, false, false, false, 68, 4,  2, 1,  0, 0}, // #697
  {DBGFIELD("MOVT_I")             2, false, false, false, 68, 4,  2, 1,  0, 0}, // #698
  {DBGFIELD("SDC1_SDC164")        2, false, false, false, 68, 4,  1, 1,  0, 0}, // #699
  {DBGFIELD("SDXC1_SDXC164")      2, false, false, false, 68, 4,  1, 1,  0, 0}, // #700
  {DBGFIELD("SWC1")               2, false, false, false, 68, 4,  1, 1,  0, 0}, // #701
  {DBGFIELD("SWXC1")              2, false, false, false, 68, 4,  1, 1,  0, 0}, // #702
  {DBGFIELD("SUXC1_SUXC164")      2, false, false, false, 68, 4,  1, 1,  0, 0}, // #703
  {DBGFIELD("ST_B_ST_D_ST_H_ST_W") 2, false, false, false, 68, 4,  1, 1,  0, 0}, // #704
  {DBGFIELD("ST_F16")             2, false, false, false, 68, 4,  1, 1,  0, 0}, // #705
  {DBGFIELD("MOVN_I_D32_MOVN_I_D64") 2, false, false, false, 72, 4,  2, 1,  0, 0}, // #706
  {DBGFIELD("MOVN_I_S")           2, false, false, false, 72, 4,  2, 1,  0, 0}, // #707
  {DBGFIELD("MOVZ_I_D32_MOVZ_I_D64") 2, false, false, false, 72, 4,  2, 1,  0, 0}, // #708
  {DBGFIELD("MOVZ_I_S")           2, false, false, false, 72, 4,  2, 1,  0, 0}, // #709
  {DBGFIELD("LDC1_LDC164")        2, false, false, false, 76, 4,  4, 1,  0, 0}, // #710
  {DBGFIELD("LDXC1_LDXC164")      2, false, false, false, 76, 4,  4, 1,  0, 0}, // #711
  {DBGFIELD("LWC1")               2, false, false, false, 76, 4,  4, 1,  0, 0}, // #712
  {DBGFIELD("LWXC1")              2, false, false, false, 76, 4,  4, 1,  0, 0}, // #713
  {DBGFIELD("LUXC1_LUXC164")      2, false, false, false, 76, 4,  4, 1,  0, 0}, // #714
  {DBGFIELD("LD_B_LD_D_LD_H_LD_W") 2, false, false, false, 76, 4,  4, 1,  0, 0}, // #715
  {DBGFIELD("LD_F16")             2, false, false, false, 76, 4,  4, 1,  0, 0}, // #716
  {DBGFIELD("CEIL_L_D64_CEIL_L_S_CEIL_W_D32_CEIL_W_D64_CEIL_W_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #717
  {DBGFIELD("FLOOR_L_D64_FLOOR_L_S_FLOOR_W_D32_FLOOR_W_D64_FLOOR_W_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #718
  {DBGFIELD("ROUND_L_D64_ROUND_L_S_ROUND_W_D32_ROUND_W_D64_ROUND_W_S") 1, false, false, false, 57, 2,  4, 1,  0, 0}, // #719
  {DBGFIELD("ROTRV")              8190, false, false, false,  0, 0,  0, 0,  0, 0}, // #720
  {DBGFIELD("ATOMIC_SWAP_I16_POSTRA_ATOMIC_SWAP_I32_POSTRA_ATOMIC_SWAP_I64_POSTRA_ATOMIC_SWAP_I8_POSTRA") 1, false, false, false,  7, 1,  2, 1,  0, 0}, // #721
  {DBGFIELD("ATOMIC_CMP_SWAP_I16_POSTRA_ATOMIC_CMP_SWAP_I32_POSTRA_ATOMIC_CMP_SWAP_I64_POSTRA_ATOMIC_CMP_SWAP_I8_POSTRA") 1, false, false, false,  7, 1,  2, 1,  0, 0}, // #722
  {DBGFIELD("ATOMIC_LOAD_ADD_I16_POSTRA_ATOMIC_LOAD_ADD_I32_POSTRA_ATOMIC_LOAD_ADD_I64_POSTRA_ATOMIC_LOAD_ADD_I8_POSTRA_ATOMIC_LOAD_AND_I16_POSTRA_ATOMIC_LOAD_AND_I32_POSTRA_ATOMIC_LOAD_AND_I64_POSTRA_ATOMIC_LOAD_AND_I8_POSTRA_ATOMIC_LOAD_MAX_I16_POSTRA_ATOMIC_LOAD_MAX_I32_POSTRA_ATOMIC_LOAD_MAX_I64_POSTRA_ATOMIC_LOAD_MAX_I8_POSTRA_ATOMIC_LOAD_MIN_I16_POSTRA_ATOMIC_LOAD_MIN_I32_POSTRA_ATOMIC_LOAD_MIN_I64_POSTRA_ATOMIC_LOAD_MIN_I8_POSTRA_ATOMIC_LOAD_NAND_I16_POSTRA_ATOMIC_LOAD_NAND_I32_POSTRA_ATOMIC_LOAD_NAND_I64_POSTRA_ATOMIC_LOAD_NAND_I8_POSTRA_ATOMIC_LOAD_OR_I16_POSTRA_ATOMIC_LOAD_OR_I32_POSTRA_ATOMIC_LOAD_OR_I64_POSTRA_ATOMIC_LOAD_OR_I8_POSTRA_ATOMIC_LOAD_SUB_I16_POSTRA_ATOMIC_LOAD_SUB_I32_POSTRA_ATOMIC_LOAD_SUB_I64_POSTRA_ATOMIC_LOAD_SUB_I8_POSTRA_ATOMIC_LOAD_UMAX_I16_POSTRA_ATOMIC_LOAD_UMAX_I32_POSTRA_ATOMIC_LOAD_UMAX_I64_POSTRA_ATOMIC_LOAD_UMAX_I8_POSTRA_ATOMIC_LOAD_UMIN_I16_POSTRA_ATOMIC_LOAD_UMIN_I32_POSTRA_ATOMIC_LOAD_UMIN_I64_POSTRA_ATOMIC_LOAD_UMIN_I8_POSTRA_ATOMIC_LOAD_XOR_I16_POSTRA_ATOMIC_LOAD_XOR_I32_POSTRA_ATOMIC_LOAD_XOR_I64_POSTRA_ATOMIC_LOAD_XOR_I8_POSTRA") 1, false, false, false,  7, 1,  2, 1,  0, 0}, // #723
  {DBGFIELD("LEA_ADDiu")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #724
  {DBGFIELD("ADDIUPC")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #725
  {DBGFIELD("ALIGN")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #726
  {DBGFIELD("ALUIPC")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #727
  {DBGFIELD("AUI")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #728
  {DBGFIELD("AUIPC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #729
  {DBGFIELD("BITSWAP")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #730
  {DBGFIELD("CLO_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #731
  {DBGFIELD("CLZ_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #732
  {DBGFIELD("LSA_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #733
  {DBGFIELD("SELEQZ_SELNEZ")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #734
  {DBGFIELD("AddiuRxImmX16_AddiuRxRxImm16_AddiuRxRxImmX16_AddiuRxRyOffMemX16_AddiuRxPcImmX16_AddiuSpImm16_AddiuSpImmX16_AdduRxRyRz16_AndRxRxRy16_CmpRxRy16_CmpiRxImm16_CmpiRxImmX16_LiRxImm16_LiRxImmX16_LiRxImmAlignX16_Move32R16_MoveR3216_Mfhi16_Mflo16_NegRxRy16_NotRxRy16_OrRxRxRy16_SebRx16_SehRx16_SllX16_SllvRxRy16_SltiRxImm16_SltiRxImmX16_SltiuRxImm16_SltiuRxImmX16_SltRxRy16_SltuRxRy16_SravRxRy16_SraX16_SrlvRxRy16_SrlX16_SubuRxRyRz16_XorRxRxRy16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #735
  {DBGFIELD("SltiCCRxImmX16_SltiuCCRxImmX16_SltCCRxRy16_SltuRxRyRz16_SltuCCRxRy16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #736
  {DBGFIELD("Constant32_LwConstant32_GotPrologue16_CONSTPOOL_ENTRY") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #737
  {DBGFIELD("ADDIUPC_MM_ADDIUR1SP_MM_ADDIUR2_MM_ADDIUS5_MM_ADDIUSP_MM_ADDiu_MM_LEA_ADDiu_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #738
  {DBGFIELD("ADDU16_MM_ADDu_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #739
  {DBGFIELD("ADD_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #740
  {DBGFIELD("ADDi_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #741
  {DBGFIELD("AND16_MM_ANDI16_MM_AND_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #742
  {DBGFIELD("ANDi_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #743
  {DBGFIELD("CLO_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #744
  {DBGFIELD("CLZ_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #745
  {DBGFIELD("EXT_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #746
  {DBGFIELD("INS_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #747
  {DBGFIELD("LI16_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #748
  {DBGFIELD("LUi_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #749
  {DBGFIELD("MOVE16_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #750
  {DBGFIELD("MOVEP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #751
  {DBGFIELD("NOR_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #752
  {DBGFIELD("NOT16_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #753
  {DBGFIELD("OR16_MM_OR_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #754
  {DBGFIELD("ORi_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #755
  {DBGFIELD("ROTRV_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #756
  {DBGFIELD("ROTR_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #757
  {DBGFIELD("SEB_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #758
  {DBGFIELD("SEH_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #759
  {DBGFIELD("SLL16_MM_SLL_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #760
  {DBGFIELD("SLLV_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #761
  {DBGFIELD("SLT_MM_SLTu_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #762
  {DBGFIELD("SLTi_MM_SLTiu_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #763
  {DBGFIELD("SRAV_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #764
  {DBGFIELD("SRA_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #765
  {DBGFIELD("SRL16_MM_SRL_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #766
  {DBGFIELD("SRLV_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #767
  {DBGFIELD("SSNOP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #768
  {DBGFIELD("SUBU16_MM_SUBu_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #769
  {DBGFIELD("SUB_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #770
  {DBGFIELD("WSBH_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #771
  {DBGFIELD("XOR16_MM_XOR_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #772
  {DBGFIELD("XORi_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #773
  {DBGFIELD("ADDIUPC_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #774
  {DBGFIELD("ADDIU_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #775
  {DBGFIELD("ADDU16_MMR6_ADDU_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #776
  {DBGFIELD("ADD_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #777
  {DBGFIELD("ALIGN_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #778
  {DBGFIELD("ALUIPC_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #779
  {DBGFIELD("AND16_MMR6_ANDI16_MMR6_AND_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #780
  {DBGFIELD("ANDI_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #781
  {DBGFIELD("AUIPC_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #782
  {DBGFIELD("AUI_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #783
  {DBGFIELD("BITSWAP_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #784
  {DBGFIELD("CLO_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #785
  {DBGFIELD("CLZ_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #786
  {DBGFIELD("EXT_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #787
  {DBGFIELD("INS_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #788
  {DBGFIELD("LI16_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #789
  {DBGFIELD("LSA_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #790
  {DBGFIELD("LUI_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #791
  {DBGFIELD("MOVE16_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #792
  {DBGFIELD("NOR_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #793
  {DBGFIELD("NOT16_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #794
  {DBGFIELD("OR16_MMR6_OR_MMR6")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #795
  {DBGFIELD("ORI_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #796
  {DBGFIELD("SELEQZ_MMR6_SELNEZ_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #797
  {DBGFIELD("SLL16_MMR6_SLL_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #798
  {DBGFIELD("SRL16_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #799
  {DBGFIELD("SSNOP_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #800
  {DBGFIELD("SUBU16_MMR6_SUBU_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #801
  {DBGFIELD("SUB_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #802
  {DBGFIELD("WSBH_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #803
  {DBGFIELD("XOR16_MMR6_XOR_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #804
  {DBGFIELD("XORI_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #805
  {DBGFIELD("AND64_ANDi64")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #806
  {DBGFIELD("DEXT64_32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #807
  {DBGFIELD("DSLL64_32")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #808
  {DBGFIELD("ORi64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #809
  {DBGFIELD("SEB64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #810
  {DBGFIELD("SEH64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #811
  {DBGFIELD("SLL64_32_SLL64_64")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #812
  {DBGFIELD("SLT64_SLTu64")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #813
  {DBGFIELD("SLTi64_SLTiu64")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #814
  {DBGFIELD("XOR64_XORi64")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #815
  {DBGFIELD("DADD")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #816
  {DBGFIELD("DADDi")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #817
  {DBGFIELD("DADDiu")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #818
  {DBGFIELD("DADDu")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #819
  {DBGFIELD("DCLO")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #820
  {DBGFIELD("DCLZ")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #821
  {DBGFIELD("DEXT_DEXTM_DEXTU")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #822
  {DBGFIELD("DINS_DINSM_DINSU")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #823
  {DBGFIELD("DROTR")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #824
  {DBGFIELD("DROTR32")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #825
  {DBGFIELD("DROTRV")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #826
  {DBGFIELD("DSBH")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #827
  {DBGFIELD("DSHD")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #828
  {DBGFIELD("DSLL")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #829
  {DBGFIELD("DSLL32")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #830
  {DBGFIELD("DSLLV")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #831
  {DBGFIELD("DSRA")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #832
  {DBGFIELD("DSRA32")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #833
  {DBGFIELD("DSRAV")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #834
  {DBGFIELD("DSRL")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #835
  {DBGFIELD("DSRL32")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #836
  {DBGFIELD("DSRLV")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #837
  {DBGFIELD("DSUB")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #838
  {DBGFIELD("DSUBu")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #839
  {DBGFIELD("LEA_ADDiu64")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #840
  {DBGFIELD("LUi64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #841
  {DBGFIELD("NOR64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #842
  {DBGFIELD("OR64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #843
  {DBGFIELD("DALIGN")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #844
  {DBGFIELD("DAHI")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #845
  {DBGFIELD("DATI")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #846
  {DBGFIELD("DAUI")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #847
  {DBGFIELD("DCLO_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #848
  {DBGFIELD("DCLZ_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #849
  {DBGFIELD("DBITSWAP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #850
  {DBGFIELD("DLSA_DLSA_R6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #851
  {DBGFIELD("SELEQZ64_SELNEZ64")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #852
  {DBGFIELD("MADD")               1, false, false, false, 48, 3,  5, 1,  0, 0}, // #853
  {DBGFIELD("MADDU")              1, false, false, false, 48, 3,  5, 1,  0, 0}, // #854
  {DBGFIELD("MSUB")               1, false, false, false, 48, 3,  5, 1,  0, 0}, // #855
  {DBGFIELD("MSUBU")              1, false, false, false, 48, 3,  5, 1,  0, 0}, // #856
  {DBGFIELD("PseudoMADD_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #857
  {DBGFIELD("PseudoMADDU_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #858
  {DBGFIELD("PseudoMSUB_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #859
  {DBGFIELD("PseudoMSUBU_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #860
  {DBGFIELD("PseudoMULT_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #861
  {DBGFIELD("PseudoMULTu_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #862
  {DBGFIELD("PseudoMULT")         1, false, false, false,  1, 2,  5, 1,  0, 0}, // #863
  {DBGFIELD("PseudoMULTu")        1, false, false, false,  1, 2,  5, 1,  0, 0}, // #864
  {DBGFIELD("PseudoSDIV_SDIV")    1, false, false, false, 51, 3, 14, 1,  0, 0}, // #865
  {DBGFIELD("PseudoUDIV_UDIV")    1, false, false, false, 51, 3, 14, 1,  0, 0}, // #866
  {DBGFIELD("PseudoMFHI_MM_PseudoMFLO_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #867
  {DBGFIELD("PseudoMTLOHI_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #868
  {DBGFIELD("MUH")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #869
  {DBGFIELD("MUHU")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #870
  {DBGFIELD("MULU")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #871
  {DBGFIELD("MUL_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #872
  {DBGFIELD("MOD")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #873
  {DBGFIELD("MODU")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #874
  {DBGFIELD("MultRxRy16_MultuRxRy16_MultRxRyRz16_MultuRxRyRz16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #875
  {DBGFIELD("DivRxRy16")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #876
  {DBGFIELD("DivuRxRy16")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #877
  {DBGFIELD("MULT_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #878
  {DBGFIELD("MULTu_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #879
  {DBGFIELD("MADD_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #880
  {DBGFIELD("MADDU_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #881
  {DBGFIELD("MSUB_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #882
  {DBGFIELD("MSUBU_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #883
  {DBGFIELD("MUL_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #884
  {DBGFIELD("SDIV_MM_SDIV_MM_Pseudo") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #885
  {DBGFIELD("UDIV_MM_UDIV_MM_Pseudo") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #886
  {DBGFIELD("MFHI16_MM_MFLO16_MM_MFHI_MM_MFLO_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #887
  {DBGFIELD("MOVF_I_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #888
  {DBGFIELD("MOVT_I_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #889
  {DBGFIELD("MTHI_MM_MTLO_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #890
  {DBGFIELD("RDHWR_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #891
  {DBGFIELD("MUHU_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #892
  {DBGFIELD("MUH_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #893
  {DBGFIELD("MULU_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #894
  {DBGFIELD("MUL_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #895
  {DBGFIELD("MODU_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #896
  {DBGFIELD("MOD_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #897
  {DBGFIELD("DIVU_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #898
  {DBGFIELD("DIV_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #899
  {DBGFIELD("RDHWR_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #900
  {DBGFIELD("DMULU")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #901
  {DBGFIELD("DMULT_PseudoDMULT")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #902
  {DBGFIELD("DMULTu_PseudoDMULTu") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #903
  {DBGFIELD("DSDIV_PseudoDSDIV")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #904
  {DBGFIELD("DUDIV_PseudoDUDIV")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #905
  {DBGFIELD("MFHI64_MFLO64_PseudoMFHI64_PseudoMFLO64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #906
  {DBGFIELD("PseudoMTLOHI64")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #907
  {DBGFIELD("MTHI64_MTLO64")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #908
  {DBGFIELD("RDHWR64")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #909
  {DBGFIELD("MOVN_I_I64_MOVN_I64_I_MOVN_I64_I64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #910
  {DBGFIELD("MOVZ_I_I64_MOVZ_I64_I_MOVZ_I64_I64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #911
  {DBGFIELD("DMUH")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #912
  {DBGFIELD("DMUHU")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #913
  {DBGFIELD("DMUL_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #914
  {DBGFIELD("DDIV")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #915
  {DBGFIELD("DMOD")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #916
  {DBGFIELD("DDIVU")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #917
  {DBGFIELD("DMODU")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #918
  {DBGFIELD("BAL_BR_BLTZAL")      1, false, false, false, 37, 3,  1, 1,  0, 0}, // #919
  {DBGFIELD("BEQ_BNE")            1, false, false, false, 37, 3,  1, 1,  0, 0}, // #920
  {DBGFIELD("BGTZ_BGEZ_BLEZ_BLTZ") 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #921
  {DBGFIELD("J")                  1, false, false, false, 37, 3,  1, 1,  0, 0}, // #922
  {DBGFIELD("JR")                 1, false, false, false, 37, 3,  1, 1,  0, 0}, // #923
  {DBGFIELD("ERet")               1, false, false, false, 37, 3,  1, 1,  0, 0}, // #924
  {DBGFIELD("BGEZAL")             1, false, false, false, 37, 3,  1, 1,  0, 0}, // #925
  {DBGFIELD("BALC")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #926
  {DBGFIELD("BEQZALC_BGEZALC_BGTZALC_BLEZALC_BLTZALC_BNEZALC") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #927
  {DBGFIELD("JIALC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #928
  {DBGFIELD("BC")                 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #929
  {DBGFIELD("BC2EQZ_BC2NEZ")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #930
  {DBGFIELD("BEQC_BGEC_BGEUC_BLTC_BLTUC_BNEC_BNVC_BOVC") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #931
  {DBGFIELD("BEQZC_BGEZC_BGTZC_BLEZC_BLTZC_BNEZC") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #932
  {DBGFIELD("JIC")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #933
  {DBGFIELD("JR_HB_R6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #934
  {DBGFIELD("SIGRIE")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #935
  {DBGFIELD("PseudoIndirectBranchR6_PseudoIndrectHazardBranchR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #936
  {DBGFIELD("TAILCALLR6REG_TAILCALLHBR6REG") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #937
  {DBGFIELD("SDBBP_R6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #938
  {DBGFIELD("Bimm16_BimmX16_BeqzRxImm16_BeqzRxImmX16_BnezRxImm16_BnezRxImmX16_Bteqz16_BteqzX16_Btnez16_BtnezX16_JrRa16_JrcRa16_JrcRx16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #939
  {DBGFIELD("BteqzT8CmpX16_BteqzT8CmpiX16_BteqzT8SltX16_BteqzT8SltuX16_BteqzT8SltiX16_BteqzT8SltiuX16_BtnezT8CmpX16_BtnezT8CmpiX16_BtnezT8SltX16_BtnezT8SltuX16_BtnezT8SltiX16_BtnezT8SltiuX16_RetRA16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #940
  {DBGFIELD("Jal16_JalB16")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #941
  {DBGFIELD("JumpLinkReg16")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #942
  {DBGFIELD("Break16")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #943
  {DBGFIELD("SelBeqZ_SelTBteqZCmp_SelTBteqZCmpi_SelTBteqZSlt_SelTBteqZSlti_SelTBteqZSltu_SelTBteqZSltiu_SelBneZ_SelTBtneZCmp_SelTBtneZCmpi_SelTBtneZSlt_SelTBtneZSlti_SelTBtneZSltu_SelTBtneZSltiu") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #944
  {DBGFIELD("B16_MM_B_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #945
  {DBGFIELD("BAL_BR_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #946
  {DBGFIELD("BC1F_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #947
  {DBGFIELD("BC1T_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #948
  {DBGFIELD("BEQZ16_MM_BGEZ_MM_BGTZ_MM_BLEZ_MM_BLTZ_MM_BNEZ16_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #949
  {DBGFIELD("BEQZC_MM_BNEZC_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #950
  {DBGFIELD("BEQ_MM_BNE_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #951
  {DBGFIELD("DERET_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #952
  {DBGFIELD("ERET_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #953
  {DBGFIELD("JR16_MM_JR_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #954
  {DBGFIELD("J_MM")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #955
  {DBGFIELD("B_MM_Pseudo")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #956
  {DBGFIELD("BGEZALS_MM_BLTZALS_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #957
  {DBGFIELD("BGEZAL_MM_BLTZAL_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #958
  {DBGFIELD("JALR16_MM_JALR_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #959
  {DBGFIELD("JALRS16_MM_JALRS_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #960
  {DBGFIELD("JALS_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #961
  {DBGFIELD("JALX_MM_JAL_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #962
  {DBGFIELD("TAILCALLREG_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #963
  {DBGFIELD("TAILCALL_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #964
  {DBGFIELD("PseudoIndirectBranch_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #965
  {DBGFIELD("BREAK16_MM_BREAK_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #966
  {DBGFIELD("SDBBP16_MM_SDBBP_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #967
  {DBGFIELD("SYSCALL_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #968
  {DBGFIELD("TEQI_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #969
  {DBGFIELD("TEQ_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #970
  {DBGFIELD("TGEIU_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #971
  {DBGFIELD("TGEI_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #972
  {DBGFIELD("TGEU_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #973
  {DBGFIELD("TGE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #974
  {DBGFIELD("TLTIU_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #975
  {DBGFIELD("TLTI_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #976
  {DBGFIELD("TLTU_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #977
  {DBGFIELD("TLT_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #978
  {DBGFIELD("TNEI_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #979
  {DBGFIELD("TNE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #980
  {DBGFIELD("TRAP_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #981
  {DBGFIELD("BC16_MMR6_BC_MMR6")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #982
  {DBGFIELD("BC1EQZC_MMR6_BC1NEZC_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #983
  {DBGFIELD("BC2EQZC_MMR6_BC2NEZC_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #984
  {DBGFIELD("BEQC_MMR6_BGEC_MMR6_BGEUC_MMR6_BLTC_MMR6_BLTUC_MMR6_BNEC_MMR6_BNVC_MMR6_BOVC_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #985
  {DBGFIELD("BEQZC16_MMR6_BNEZC16_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #986
  {DBGFIELD("BEQZC_MMR6_BGEZC_MMR6_BGTZC_MMR6_BLEZC_MMR6_BLTZC_MMR6_BNEZC_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #987
  {DBGFIELD("DERET_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #988
  {DBGFIELD("ERETNC_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #989
  {DBGFIELD("JAL_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #990
  {DBGFIELD("ERET_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #991
  {DBGFIELD("JIC_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #992
  {DBGFIELD("JRADDIUSP_JRCADDIUSP_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #993
  {DBGFIELD("JRC16_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #994
  {DBGFIELD("JRC16_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #995
  {DBGFIELD("SIGRIE_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #996
  {DBGFIELD("B_MMR6_Pseudo")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #997
  {DBGFIELD("PseudoIndirectBranch_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #998
  {DBGFIELD("BALC_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #999
  {DBGFIELD("BEQZALC_MMR6_BGEZALC_MMR6_BGTZALC_MMR6_BLEZALC_MMR6_BLTZALC_MMR6_BNEZALC_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1000
  {DBGFIELD("JALRC16_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1001
  {DBGFIELD("JALRC_HB_MMR6")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1002
  {DBGFIELD("JALRC_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1003
  {DBGFIELD("JIALC_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1004
  {DBGFIELD("TAILCALLREG_MMR6")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1005
  {DBGFIELD("TAILCALL_MMR6")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1006
  {DBGFIELD("BREAK16_MMR6_BREAK_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1007
  {DBGFIELD("SDBBP_MMR6_SDBBP16_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1008
  {DBGFIELD("BEQ64_BNE64")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1009
  {DBGFIELD("BGEZ64_BGTZ64_BLEZ64_BLTZ64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1010
  {DBGFIELD("JR64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1011
  {DBGFIELD("JALR64_JALR64Pseudo_JALRHB64Pseudo") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1012
  {DBGFIELD("JALR_HB64")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1013
  {DBGFIELD("JR_HB64")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1014
  {DBGFIELD("TAILCALLREG64_TAILCALLREGHB64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1015
  {DBGFIELD("PseudoReturn64")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1016
  {DBGFIELD("BEQC64_BGEC64_BGEUC64_BLTC64_BLTUC64_BNEC64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1017
  {DBGFIELD("BEQZC64_BGEZC64_BGTZC64_BLEZC64_BLTZC64_BNEZC64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1018
  {DBGFIELD("JIC64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1019
  {DBGFIELD("PseudoIndirectBranch64_PseudoIndirectHazardBranch64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1020
  {DBGFIELD("JIALC64")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1021
  {DBGFIELD("JR_HB64_R6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1022
  {DBGFIELD("TAILCALL64R6REG_TAILCALLHB64R6REG") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1023
  {DBGFIELD("PseudoIndirectBranch64R6_PseudoIndrectHazardBranch64R6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1024
  {DBGFIELD("EVP")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1025
  {DBGFIELD("DVP")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1026
  {DBGFIELD("TLBP_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1027
  {DBGFIELD("TLBR_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1028
  {DBGFIELD("TLBWI_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1029
  {DBGFIELD("TLBWR_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1030
  {DBGFIELD("DI_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1031
  {DBGFIELD("EI_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1032
  {DBGFIELD("EHB_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1033
  {DBGFIELD("PAUSE_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1034
  {DBGFIELD("WAIT_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1035
  {DBGFIELD("RDPGPR_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1036
  {DBGFIELD("WRPGPR_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1037
  {DBGFIELD("TLBINV_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1038
  {DBGFIELD("TLBINVF_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1039
  {DBGFIELD("MFHC0_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1040
  {DBGFIELD("MFC0_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1041
  {DBGFIELD("MFHC2_MMR6_MFC2_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1042
  {DBGFIELD("MTHC0_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1043
  {DBGFIELD("MTC0_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1044
  {DBGFIELD("MTHC2_MMR6_MTC2_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1045
  {DBGFIELD("EVP_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1046
  {DBGFIELD("DVP_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1047
  {DBGFIELD("DI_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1048
  {DBGFIELD("EI_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1049
  {DBGFIELD("EHB_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1050
  {DBGFIELD("PAUSE_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1051
  {DBGFIELD("WAIT_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1052
  {DBGFIELD("DMFC0")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1053
  {DBGFIELD("DMTC0")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1054
  {DBGFIELD("DMFC2")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1055
  {DBGFIELD("DMTC2")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1056
  {DBGFIELD("CFC2_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1057
  {DBGFIELD("CTC2_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1058
  {DBGFIELD("DMT")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1059
  {DBGFIELD("DVPE")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1060
  {DBGFIELD("EMT")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1061
  {DBGFIELD("EVPE")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1062
  {DBGFIELD("MFTR")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1063
  {DBGFIELD("MTTR")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1064
  {DBGFIELD("YIELD")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1065
  {DBGFIELD("FORK")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1066
  {DBGFIELD("DMFGC0")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1067
  {DBGFIELD("DMTGC0")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1068
  {DBGFIELD("HYPCALL_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1069
  {DBGFIELD("TLBGINVF_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1070
  {DBGFIELD("TLBGINV_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1071
  {DBGFIELD("TLBGP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1072
  {DBGFIELD("TLBGR_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1073
  {DBGFIELD("TLBGWI_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1074
  {DBGFIELD("TLBGWR_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1075
  {DBGFIELD("MFGC0_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1076
  {DBGFIELD("MFHGC0_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1077
  {DBGFIELD("MTGC0_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1078
  {DBGFIELD("MTHGC0_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1079
  {DBGFIELD("SC_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1080
  {DBGFIELD("LDC2_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1081
  {DBGFIELD("LL_R6")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1082
  {DBGFIELD("LWC2_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1083
  {DBGFIELD("SWC2_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1084
  {DBGFIELD("SDC2_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1085
  {DBGFIELD("SC_R6")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1086
  {DBGFIELD("PREF_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1087
  {DBGFIELD("CACHE_R6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1088
  {DBGFIELD("GINVI")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1089
  {DBGFIELD("GINVT")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1090
  {DBGFIELD("LBE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1091
  {DBGFIELD("LBuE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1092
  {DBGFIELD("LHE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1093
  {DBGFIELD("LHuE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1094
  {DBGFIELD("LWE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1095
  {DBGFIELD("LWLE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1096
  {DBGFIELD("LWRE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1097
  {DBGFIELD("LLE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1098
  {DBGFIELD("SBE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1099
  {DBGFIELD("SB_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1100
  {DBGFIELD("SHE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1101
  {DBGFIELD("SWE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1102
  {DBGFIELD("SWLE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1103
  {DBGFIELD("SWRE_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1104
  {DBGFIELD("SCE_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1105
  {DBGFIELD("PREFE_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1106
  {DBGFIELD("CACHEE_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1107
  {DBGFIELD("Restore16_RestoreX16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1108
  {DBGFIELD("LbRxRyOffMemX16")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1109
  {DBGFIELD("LbuRxRyOffMemX16")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1110
  {DBGFIELD("LhRxRyOffMemX16")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1111
  {DBGFIELD("LhuRxRyOffMemX16")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1112
  {DBGFIELD("LwRxRyOffMemX16_LwRxSpImmX16_LwRxPcTcp16_LwRxPcTcpX16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1113
  {DBGFIELD("Save16_SaveX16")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1114
  {DBGFIELD("SbRxRyOffMemX16")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1115
  {DBGFIELD("ShRxRyOffMemX16")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1116
  {DBGFIELD("SwRxRyOffMemX16_SwRxSpImmX16") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1117
  {DBGFIELD("LBU16_MM_LBu_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1118
  {DBGFIELD("LB_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1119
  {DBGFIELD("LHU16_MM_LHu_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1120
  {DBGFIELD("LH_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1121
  {DBGFIELD("LL_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1122
  {DBGFIELD("LW16_MM_LWGP_MM_LWSP_MM_LW_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1123
  {DBGFIELD("LWL_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1124
  {DBGFIELD("LWM16_MM_LWM32_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1125
  {DBGFIELD("LWP_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1126
  {DBGFIELD("LWR_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1127
  {DBGFIELD("LWU_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1128
  {DBGFIELD("LWXS_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1129
  {DBGFIELD("SB16_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1130
  {DBGFIELD("SC_MM")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1131
  {DBGFIELD("SH16_MM_SH_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1132
  {DBGFIELD("SW16_MM_SWSP_MM_SW_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1133
  {DBGFIELD("SWL_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1134
  {DBGFIELD("SWM16_MM_SWM32_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1135
  {DBGFIELD("SWM_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1136
  {DBGFIELD("SWP_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1137
  {DBGFIELD("SWR_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1138
  {DBGFIELD("PREF_MM_PREFX_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1139
  {DBGFIELD("CACHE_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1140
  {DBGFIELD("SYNC_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1141
  {DBGFIELD("SYNCI_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1142
  {DBGFIELD("GINVI_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1143
  {DBGFIELD("GINVT_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1144
  {DBGFIELD("LBU_MMR6")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1145
  {DBGFIELD("LB_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1146
  {DBGFIELD("LDC2_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1147
  {DBGFIELD("LL_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1148
  {DBGFIELD("LWM16_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1149
  {DBGFIELD("LWC2_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1150
  {DBGFIELD("LWPC_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1151
  {DBGFIELD("LW_MMR6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1152
  {DBGFIELD("SB16_MMR6_SB_MMR6")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1153
  {DBGFIELD("SDC2_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1154
  {DBGFIELD("SH16_MMR6_SH_MMR6")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1155
  {DBGFIELD("SW16_MMR6_SWSP_MMR6_SW_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1156
  {DBGFIELD("SWC2_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1157
  {DBGFIELD("SWM16_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1158
  {DBGFIELD("SYNC_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1159
  {DBGFIELD("SYNCI_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1160
  {DBGFIELD("PREF_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1161
  {DBGFIELD("CACHE_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1162
  {DBGFIELD("LD")                 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1163
  {DBGFIELD("LL64_LLD")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1164
  {DBGFIELD("LWu")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1165
  {DBGFIELD("LB64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1166
  {DBGFIELD("LBu64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1167
  {DBGFIELD("LH64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1168
  {DBGFIELD("LHu64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1169
  {DBGFIELD("LW64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1170
  {DBGFIELD("LWL64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1171
  {DBGFIELD("LWR64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1172
  {DBGFIELD("LDL")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1173
  {DBGFIELD("LDR")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1174
  {DBGFIELD("SD")                 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1175
  {DBGFIELD("SC64_SCD")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1176
  {DBGFIELD("SB64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1177
  {DBGFIELD("SH64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1178
  {DBGFIELD("SW64")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1179
  {DBGFIELD("SWL64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1180
  {DBGFIELD("SWR64")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1181
  {DBGFIELD("SDL")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1182
  {DBGFIELD("SDR")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1183
  {DBGFIELD("LWUPC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1184
  {DBGFIELD("LDPC")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1185
  {DBGFIELD("LLD_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1186
  {DBGFIELD("LL64_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1187
  {DBGFIELD("SC64_R6")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1188
  {DBGFIELD("SCD_R6")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1189
  {DBGFIELD("CRC32B")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1190
  {DBGFIELD("CRC32H")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1191
  {DBGFIELD("CRC32W")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1192
  {DBGFIELD("CRC32CB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1193
  {DBGFIELD("CRC32CH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1194
  {DBGFIELD("CRC32CW")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1195
  {DBGFIELD("CRC32D")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1196
  {DBGFIELD("CRC32CD")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1197
  {DBGFIELD("BADDu")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1198
  {DBGFIELD("BBIT0_BBIT032_BBIT1_BBIT132") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1199
  {DBGFIELD("CINS_CINS32_CINS64_32_CINS_i32") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1200
  {DBGFIELD("DMFC2_OCTEON")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1201
  {DBGFIELD("DMTC2_OCTEON")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1202
  {DBGFIELD("DPOP_POP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1203
  {DBGFIELD("EXTS_EXTS32")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1204
  {DBGFIELD("MTM0_MTM1_MTM2_MTP0_MTP1_MTP2") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1205
  {DBGFIELD("SEQ_SNE")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1206
  {DBGFIELD("SEQi_SNEi")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1207
  {DBGFIELD("V3MULU_VMM0_VMULU")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1208
  {DBGFIELD("DMUL")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1209
  {DBGFIELD("SAA_SAAD")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1210
  {DBGFIELD("ADDR_PS64")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1211
  {DBGFIELD("CVT_PS_PW64_CVT_PW_PS64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1212
  {DBGFIELD("MULR_PS64")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1213
  {DBGFIELD("PseudoTRUNC_W_D_PseudoTRUNC_W_D32_PseudoTRUNC_W_S") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1214
  {DBGFIELD("MOVT_I64")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1215
  {DBGFIELD("MOVF_I64")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1216
  {DBGFIELD("MOVZ_I64_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1217
  {DBGFIELD("MOVN_I64_D64")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1218
  {DBGFIELD("MOVN_I64_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1219
  {DBGFIELD("MOVZ_I64_D64")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1220
  {DBGFIELD("SELEQZ_S_SELNEZ_S")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1221
  {DBGFIELD("SELEQZ_D_SELNEZ_D")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1222
  {DBGFIELD("MAX_S_MAXA_S")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1223
  {DBGFIELD("MAX_D_MAXA_D")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1224
  {DBGFIELD("MIN_S_MINA_D")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1225
  {DBGFIELD("MIN_D_MINA_S")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1226
  {DBGFIELD("CLASS_S")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1227
  {DBGFIELD("CLASS_D")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1228
  {DBGFIELD("RINT_S")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1229
  {DBGFIELD("RINT_D")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1230
  {DBGFIELD("BC1EQZ_BC1NEZ")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1231
  {DBGFIELD("SEL_D")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1232
  {DBGFIELD("SEL_S")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1233
  {DBGFIELD("MADDF_S")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1234
  {DBGFIELD("MSUBF_S")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1235
  {DBGFIELD("MADDF_D")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1236
  {DBGFIELD("MSUBF_D")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1237
  {DBGFIELD("MOVF_D32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1238
  {DBGFIELD("MOVF_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1239
  {DBGFIELD("MOVN_I_D32_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1240
  {DBGFIELD("MOVN_I_S_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1241
  {DBGFIELD("MOVT_D32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1242
  {DBGFIELD("MOVT_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1243
  {DBGFIELD("MOVZ_I_D32_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1244
  {DBGFIELD("MOVZ_I_S_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1245
  {DBGFIELD("CVT_D32_S_MM_CVT_D32_W_MM_CVT_D64_S_MM_CVT_D64_W_MM_CVT_L_D64_MM_CVT_L_S_MM_CVT_S_D32_MM_CVT_S_D64_MM_CVT_S_W_MM_CVT_W_D32_MM_CVT_W_D64_MM_CVT_W_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1246
  {DBGFIELD("CEIL_W_MM_CEIL_W_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1247
  {DBGFIELD("FLOOR_W_MM_FLOOR_W_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1248
  {DBGFIELD("NMADD_S_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1249
  {DBGFIELD("NMADD_D32_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1250
  {DBGFIELD("NMSUB_S_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1251
  {DBGFIELD("NMSUB_D32_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1252
  {DBGFIELD("MADD_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1253
  {DBGFIELD("MADD_D32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1254
  {DBGFIELD("ROUND_W_MM_ROUND_W_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1255
  {DBGFIELD("TRUNC_W_MM_TRUNC_W_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1256
  {DBGFIELD("C_F_D32_MM_C_F_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1257
  {DBGFIELD("C_F_S_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1258
  {DBGFIELD("C_EQ_D32_MM_C_EQ_D64_MM_C_LE_D32_MM_C_LE_D64_MM_C_LT_D32_MM_C_LT_D64_MM_C_SF_D32_MM_C_SF_D64_MM_C_UN_D32_MM_C_UN_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1259
  {DBGFIELD("C_EQ_S_MM_C_LE_S_MM_C_LT_S_MM_C_SF_S_MM_C_UN_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1260
  {DBGFIELD("C_NGE_D32_MM_C_NGE_D64_MM_C_NGL_D32_MM_C_NGL_D64_MM_C_NGT_D32_MM_C_NGT_D64_MM_C_OLE_D32_MM_C_OLE_D64_MM_C_OLT_D32_MM_C_OLT_D64_MM_C_SEQ_D32_MM_C_SEQ_D64_MM_C_UEQ_D32_MM_C_UEQ_D64_MM_C_ULE_D32_MM_C_ULE_D64_MM_C_ULT_D32_MM_C_ULT_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1261
  {DBGFIELD("C_NGE_S_MM_C_NGL_S_MM_C_NGT_S_MM_C_OLE_S_MM_C_OLT_S_MM_C_SEQ_S_MM_C_UEQ_S_MM_C_ULE_S_MM_C_ULT_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1262
  {DBGFIELD("C_NGLE_D32_MM_C_NGLE_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1263
  {DBGFIELD("C_NGLE_S_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1264
  {DBGFIELD("FCMP_S32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1265
  {DBGFIELD("FCMP_D32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1266
  {DBGFIELD("MFC1_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1267
  {DBGFIELD("MFHC1_D32_MM_MFHC1_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1268
  {DBGFIELD("MTC1_MM_MTC1_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1269
  {DBGFIELD("MTHC1_D32_MM_MTHC1_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1270
  {DBGFIELD("FABS_D32_MM_FABS_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1271
  {DBGFIELD("FABS_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1272
  {DBGFIELD("FNEG_D32_MM_FNEG_D64_MM_FNEG_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1273
  {DBGFIELD("FADD_D32_MM_FADD_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1274
  {DBGFIELD("FADD_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1275
  {DBGFIELD("FMOV_D32_MM_FMOV_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1276
  {DBGFIELD("FMOV_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1277
  {DBGFIELD("FMUL_D32_MM_FMUL_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1278
  {DBGFIELD("FMUL_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1279
  {DBGFIELD("FSUB_D32_MM_FSUB_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1280
  {DBGFIELD("FSUB_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1281
  {DBGFIELD("MSUB_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1282
  {DBGFIELD("MSUB_D32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1283
  {DBGFIELD("FDIV_S_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1284
  {DBGFIELD("FDIV_D32_MM_FDIV_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1285
  {DBGFIELD("FSQRT_S_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1286
  {DBGFIELD("FSQRT_D32_MM_FSQRT_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1287
  {DBGFIELD("RECIP_S_MM_RSQRT_S_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1288
  {DBGFIELD("RECIP_D32_MM_RECIP_D64_MM_RSQRT_D32_MM_RSQRT_D64_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1289
  {DBGFIELD("SDC1_MM_D32_SDC1_MM_D64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1290
  {DBGFIELD("SWC1_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1291
  {DBGFIELD("SUXC1_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1292
  {DBGFIELD("SWXC1_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1293
  {DBGFIELD("CFC1_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1294
  {DBGFIELD("CTC1_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1295
  {DBGFIELD("LDC1_MM_D32_LDC1_MM_D64") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1296
  {DBGFIELD("LUXC1_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1297
  {DBGFIELD("LWC1_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1298
  {DBGFIELD("LWXC1_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1299
  {DBGFIELD("FNEG_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1300
  {DBGFIELD("CMP_AF_D_MMR6_CMP_EQ_D_MMR6_CMP_LE_D_MMR6_CMP_LT_D_MMR6_CMP_UN_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1301
  {DBGFIELD("CMP_AF_S_MMR6_CMP_EQ_S_MMR6_CMP_LE_S_MMR6_CMP_LT_S_MMR6_CMP_UN_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1302
  {DBGFIELD("CMP_SAF_D_MMR6_CMP_SEQ_D_MMR6_CMP_SLE_D_MMR6_CMP_SLT_D_MMR6_CMP_SUN_D_MMR6_CMP_UEQ_D_MMR6_CMP_ULE_D_MMR6_CMP_ULT_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1303
  {DBGFIELD("CMP_SAF_S_MMR6_CMP_SEQ_S_MMR6_CMP_SLE_S_MMR6_CMP_SLT_S_MMR6_CMP_SUN_S_MMR6_CMP_UEQ_S_MMR6_CMP_ULE_S_MMR6_CMP_ULT_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1304
  {DBGFIELD("CMP_SUEQ_D_MMR6_CMP_SULE_D_MMR6_CMP_SULT_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1305
  {DBGFIELD("CMP_SUEQ_S_MMR6_CMP_SULE_S_MMR6_CMP_SULT_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1306
  {DBGFIELD("CVT_D_L_MMR6_CVT_L_D_MMR6_CVT_L_S_MMR6_CVT_S_L_MMR6_CVT_S_W_MMR6_CVT_W_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1307
  {DBGFIELD("TRUNC_L_D_MMR6_TRUNC_L_S_MMR6_TRUNC_W_D_MMR6_TRUNC_W_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1308
  {DBGFIELD("ROUND_L_D_MMR6_ROUND_L_S_MMR6_ROUND_W_D_MMR6_ROUND_W_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1309
  {DBGFIELD("FLOOR_L_D_MMR6_FLOOR_L_S_MMR6_FLOOR_W_D_MMR6_FLOOR_W_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1310
  {DBGFIELD("CEIL_L_D_MMR6_CEIL_L_S_MMR6_CEIL_W_D_MMR6_CEIL_W_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1311
  {DBGFIELD("MFC1_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1312
  {DBGFIELD("MTC1_MMR6")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1313
  {DBGFIELD("CLASS_S_MMR6_CLASS_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1314
  {DBGFIELD("FADD_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1315
  {DBGFIELD("MAX_D_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1316
  {DBGFIELD("MAX_S_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1317
  {DBGFIELD("MIN_D_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1318
  {DBGFIELD("MIN_S_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1319
  {DBGFIELD("MAXA_D_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1320
  {DBGFIELD("MAXA_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1321
  {DBGFIELD("MINA_D_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1322
  {DBGFIELD("MINA_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1323
  {DBGFIELD("SELEQZ_D_MMR6_SELNEZ_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1324
  {DBGFIELD("SELEQZ_S_MMR6_SELNEZ_S_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1325
  {DBGFIELD("SEL_D_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1326
  {DBGFIELD("SEL_S_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1327
  {DBGFIELD("RINT_S_MMR6_RINT_D_MMR6") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1328
  {DBGFIELD("MADDF_D_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1329
  {DBGFIELD("MADDF_S_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1330
  {DBGFIELD("MSUBF_D_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1331
  {DBGFIELD("MSUBF_S_MMR6")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1332
  {DBGFIELD("FMOV_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1333
  {DBGFIELD("FMUL_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1334
  {DBGFIELD("FSUB_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1335
  {DBGFIELD("FMOV_D_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1336
  {DBGFIELD("FDIV_S_MMR6")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1337
  {DBGFIELD("SDC1_D64_MMR6")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1338
  {DBGFIELD("LDC1_D64_MMR6")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1339
  {DBGFIELD("DMFC1")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1340
  {DBGFIELD("DMTC1")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1341
  {DBGFIELD("SWDSP")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1342
  {DBGFIELD("LWDSP")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1343
  {DBGFIELD("PseudoMTLOHI_DSP")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1344
  {DBGFIELD("EXTRV_RS_W")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1345
  {DBGFIELD("EXTRV_R_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1346
  {DBGFIELD("EXTRV_S_H")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1347
  {DBGFIELD("EXTRV_W")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1348
  {DBGFIELD("EXTR_RS_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1349
  {DBGFIELD("EXTR_R_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1350
  {DBGFIELD("EXTR_S_H")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1351
  {DBGFIELD("EXTR_W")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1352
  {DBGFIELD("INSV")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1353
  {DBGFIELD("MTHLIP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1354
  {DBGFIELD("MTHI_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1355
  {DBGFIELD("MTLO_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1356
  {DBGFIELD("ABSQ_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1357
  {DBGFIELD("ABSQ_S_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1358
  {DBGFIELD("ADDQ_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1359
  {DBGFIELD("ADDQ_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1360
  {DBGFIELD("ADDQ_S_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1361
  {DBGFIELD("ADDSC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1362
  {DBGFIELD("ADDU_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1363
  {DBGFIELD("ADDU_S_QB")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1364
  {DBGFIELD("ADDWC")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1365
  {DBGFIELD("BITREV")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1366
  {DBGFIELD("BPOSGE32")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1367
  {DBGFIELD("CMPGU_EQ_QB")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1368
  {DBGFIELD("CMPGU_LE_QB")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1369
  {DBGFIELD("CMPGU_LT_QB")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1370
  {DBGFIELD("CMPU_EQ_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1371
  {DBGFIELD("CMPU_LE_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1372
  {DBGFIELD("CMPU_LT_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1373
  {DBGFIELD("CMP_EQ_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1374
  {DBGFIELD("CMP_LE_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1375
  {DBGFIELD("CMP_LT_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1376
  {DBGFIELD("DPAQ_SA_L_W")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1377
  {DBGFIELD("DPAQ_S_W_PH")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1378
  {DBGFIELD("DPAU_H_QBL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1379
  {DBGFIELD("DPAU_H_QBR")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1380
  {DBGFIELD("DPSQ_SA_L_W")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1381
  {DBGFIELD("DPSQ_S_W_PH")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1382
  {DBGFIELD("DPSU_H_QBL")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1383
  {DBGFIELD("DPSU_H_QBR")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1384
  {DBGFIELD("EXTPDPV")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1385
  {DBGFIELD("EXTPDP")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1386
  {DBGFIELD("EXTPV")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1387
  {DBGFIELD("EXTP")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1388
  {DBGFIELD("LBUX")               8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1389
  {DBGFIELD("LHX")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1390
  {DBGFIELD("LWX")                8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1391
  {DBGFIELD("MADDU_DSP")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1392
  {DBGFIELD("MADD_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1393
  {DBGFIELD("MAQ_SA_W_PHL")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1394
  {DBGFIELD("MAQ_SA_W_PHR")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1395
  {DBGFIELD("MAQ_S_W_PHL")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1396
  {DBGFIELD("MAQ_S_W_PHR")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1397
  {DBGFIELD("MFHI_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1398
  {DBGFIELD("MFLO_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1399
  {DBGFIELD("MODSUB")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1400
  {DBGFIELD("MSUBU_DSP")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1401
  {DBGFIELD("MSUB_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1402
  {DBGFIELD("MULEQ_S_W_PHL")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1403
  {DBGFIELD("MULEQ_S_W_PHR")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1404
  {DBGFIELD("MULEU_S_PH_QBL")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1405
  {DBGFIELD("MULEU_S_PH_QBR")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1406
  {DBGFIELD("MULQ_RS_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1407
  {DBGFIELD("MULSAQ_S_W_PH")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1408
  {DBGFIELD("MULTU_DSP")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1409
  {DBGFIELD("MULT_DSP")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1410
  {DBGFIELD("PACKRL_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1411
  {DBGFIELD("PICK_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1412
  {DBGFIELD("PICK_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1413
  {DBGFIELD("PRECEQU_PH_QBLA")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1414
  {DBGFIELD("PRECEQU_PH_QBL")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1415
  {DBGFIELD("PRECEQU_PH_QBRA")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1416
  {DBGFIELD("PRECEQU_PH_QBR")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1417
  {DBGFIELD("PRECEQ_W_PHL")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1418
  {DBGFIELD("PRECEQ_W_PHR")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1419
  {DBGFIELD("PRECEU_PH_QBLA")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1420
  {DBGFIELD("PRECEU_PH_QBL")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1421
  {DBGFIELD("PRECEU_PH_QBRA")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1422
  {DBGFIELD("PRECEU_PH_QBR")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1423
  {DBGFIELD("PRECRQU_S_QB_PH")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1424
  {DBGFIELD("PRECRQ_PH_W")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1425
  {DBGFIELD("PRECRQ_QB_PH")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1426
  {DBGFIELD("PRECRQ_RS_PH_W")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1427
  {DBGFIELD("RADDU_W_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1428
  {DBGFIELD("RDDSP")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1429
  {DBGFIELD("REPLV_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1430
  {DBGFIELD("REPLV_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1431
  {DBGFIELD("REPL_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1432
  {DBGFIELD("REPL_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1433
  {DBGFIELD("SHILOV")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1434
  {DBGFIELD("SHILO")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1435
  {DBGFIELD("SHLLV_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1436
  {DBGFIELD("SHLLV_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1437
  {DBGFIELD("SHLLV_S_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1438
  {DBGFIELD("SHLLV_S_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1439
  {DBGFIELD("SHLL_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1440
  {DBGFIELD("SHLL_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1441
  {DBGFIELD("SHLL_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1442
  {DBGFIELD("SHLL_S_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1443
  {DBGFIELD("SHRAV_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1444
  {DBGFIELD("SHRAV_R_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1445
  {DBGFIELD("SHRAV_R_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1446
  {DBGFIELD("SHRA_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1447
  {DBGFIELD("SHRA_R_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1448
  {DBGFIELD("SHRA_R_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1449
  {DBGFIELD("SHRLV_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1450
  {DBGFIELD("SHRL_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1451
  {DBGFIELD("SUBQ_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1452
  {DBGFIELD("SUBQ_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1453
  {DBGFIELD("SUBQ_S_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1454
  {DBGFIELD("SUBU_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1455
  {DBGFIELD("SUBU_S_QB")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1456
  {DBGFIELD("WRDSP")              8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1457
  {DBGFIELD("PseudoCMPU_EQ_QB_PseudoCMPU_LE_QB_PseudoCMPU_LT_QB_PseudoCMP_EQ_PH_PseudoCMP_LE_PH_PseudoCMP_LT_PH") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1458
  {DBGFIELD("PseudoPICK_PH_PseudoPICK_QB") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1459
  {DBGFIELD("ABSQ_S_QB")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1460
  {DBGFIELD("ADDQH_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1461
  {DBGFIELD("ADDQH_R_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1462
  {DBGFIELD("ADDQH_R_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1463
  {DBGFIELD("ADDQH_W")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1464
  {DBGFIELD("ADDUH_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1465
  {DBGFIELD("ADDUH_R_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1466
  {DBGFIELD("ADDU_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1467
  {DBGFIELD("ADDU_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1468
  {DBGFIELD("APPEND")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1469
  {DBGFIELD("BALIGN")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1470
  {DBGFIELD("CMPGDU_EQ_QB")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1471
  {DBGFIELD("CMPGDU_LE_QB")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1472
  {DBGFIELD("CMPGDU_LT_QB")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1473
  {DBGFIELD("DPA_W_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1474
  {DBGFIELD("DPAQX_SA_W_PH")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1475
  {DBGFIELD("DPAQX_S_W_PH")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1476
  {DBGFIELD("DPAX_W_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1477
  {DBGFIELD("DPS_W_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1478
  {DBGFIELD("DPSQX_S_W_PH")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1479
  {DBGFIELD("DPSQX_SA_W_PH")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1480
  {DBGFIELD("DPSX_W_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1481
  {DBGFIELD("MUL_PH")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1482
  {DBGFIELD("MUL_S_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1483
  {DBGFIELD("MULQ_RS_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1484
  {DBGFIELD("MULQ_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1485
  {DBGFIELD("MULQ_S_W")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1486
  {DBGFIELD("MULSA_W_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1487
  {DBGFIELD("PRECR_QB_PH")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1488
  {DBGFIELD("PRECR_SRA_PH_W")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1489
  {DBGFIELD("PRECR_SRA_R_PH_W")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1490
  {DBGFIELD("PREPEND")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1491
  {DBGFIELD("SHRA_QB")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1492
  {DBGFIELD("SHRA_R_QB")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1493
  {DBGFIELD("SHRAV_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1494
  {DBGFIELD("SHRAV_R_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1495
  {DBGFIELD("SHRL_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1496
  {DBGFIELD("SHRLV_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1497
  {DBGFIELD("SUBQH_PH")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1498
  {DBGFIELD("SUBQH_R_PH")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1499
  {DBGFIELD("SUBQH_W")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1500
  {DBGFIELD("SUBQH_R_W")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1501
  {DBGFIELD("SUBU_PH")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1502
  {DBGFIELD("SUBU_S_PH")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1503
  {DBGFIELD("SUBUH_QB")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1504
  {DBGFIELD("SUBUH_R_QB")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1505
  {DBGFIELD("LWDSP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1506
  {DBGFIELD("SWDSP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1507
  {DBGFIELD("ABSQ_S_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1508
  {DBGFIELD("ABSQ_S_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1509
  {DBGFIELD("ADDQ_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1510
  {DBGFIELD("ADDQ_S_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1511
  {DBGFIELD("ADDQ_S_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1512
  {DBGFIELD("ADDSC_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1513
  {DBGFIELD("ADDU_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1514
  {DBGFIELD("ADDU_S_QB_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1515
  {DBGFIELD("ADDWC_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1516
  {DBGFIELD("BITREV_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1517
  {DBGFIELD("BPOSGE32_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1518
  {DBGFIELD("CMPGU_EQ_QB_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1519
  {DBGFIELD("CMPGU_LE_QB_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1520
  {DBGFIELD("CMPGU_LT_QB_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1521
  {DBGFIELD("CMPU_EQ_QB_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1522
  {DBGFIELD("CMPU_LE_QB_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1523
  {DBGFIELD("CMPU_LT_QB_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1524
  {DBGFIELD("CMP_EQ_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1525
  {DBGFIELD("CMP_LE_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1526
  {DBGFIELD("CMP_LT_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1527
  {DBGFIELD("DPAQ_SA_L_W_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1528
  {DBGFIELD("DPAQ_S_W_PH_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1529
  {DBGFIELD("DPAU_H_QBL_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1530
  {DBGFIELD("DPAU_H_QBR_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1531
  {DBGFIELD("DPSQ_SA_L_W_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1532
  {DBGFIELD("DPSQ_S_W_PH_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1533
  {DBGFIELD("DPSU_H_QBL_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1534
  {DBGFIELD("DPSU_H_QBR_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1535
  {DBGFIELD("EXTPDPV_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1536
  {DBGFIELD("EXTPDP_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1537
  {DBGFIELD("EXTPV_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1538
  {DBGFIELD("EXTP_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1539
  {DBGFIELD("EXTRV_RS_W_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1540
  {DBGFIELD("EXTRV_R_W_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1541
  {DBGFIELD("EXTRV_S_H_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1542
  {DBGFIELD("EXTRV_W_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1543
  {DBGFIELD("EXTR_RS_W_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1544
  {DBGFIELD("EXTR_R_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1545
  {DBGFIELD("EXTR_S_H_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1546
  {DBGFIELD("EXTR_W_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1547
  {DBGFIELD("INSV_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1548
  {DBGFIELD("LBUX_MM")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1549
  {DBGFIELD("LHX_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1550
  {DBGFIELD("LWX_MM")             8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1551
  {DBGFIELD("MADDU_DSP_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1552
  {DBGFIELD("MADD_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1553
  {DBGFIELD("MAQ_SA_W_PHL_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1554
  {DBGFIELD("MAQ_SA_W_PHR_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1555
  {DBGFIELD("MAQ_S_W_PHL_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1556
  {DBGFIELD("MAQ_S_W_PHR_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1557
  {DBGFIELD("MFHI_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1558
  {DBGFIELD("MFLO_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1559
  {DBGFIELD("MODSUB_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1560
  {DBGFIELD("MOVEP_MMR6")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1561
  {DBGFIELD("MOVN_I_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1562
  {DBGFIELD("MOVZ_I_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1563
  {DBGFIELD("MSUBU_DSP_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1564
  {DBGFIELD("MSUB_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1565
  {DBGFIELD("MTHI_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1566
  {DBGFIELD("MTHLIP_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1567
  {DBGFIELD("MTLO_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1568
  {DBGFIELD("MULEQ_S_W_PHL_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1569
  {DBGFIELD("MULEQ_S_W_PHR_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1570
  {DBGFIELD("MULEU_S_PH_QBL_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1571
  {DBGFIELD("MULEU_S_PH_QBR_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1572
  {DBGFIELD("MULQ_RS_PH_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1573
  {DBGFIELD("MULSAQ_S_W_PH_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1574
  {DBGFIELD("MULTU_DSP_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1575
  {DBGFIELD("MULT_DSP_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1576
  {DBGFIELD("PACKRL_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1577
  {DBGFIELD("PICK_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1578
  {DBGFIELD("PICK_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1579
  {DBGFIELD("PRECEQU_PH_QBLA_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1580
  {DBGFIELD("PRECEQU_PH_QBL_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1581
  {DBGFIELD("PRECEQU_PH_QBRA_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1582
  {DBGFIELD("PRECEQU_PH_QBR_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1583
  {DBGFIELD("PRECEQ_W_PHL_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1584
  {DBGFIELD("PRECEQ_W_PHR_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1585
  {DBGFIELD("PRECEU_PH_QBLA_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1586
  {DBGFIELD("PRECEU_PH_QBL_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1587
  {DBGFIELD("PRECEU_PH_QBRA_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1588
  {DBGFIELD("PRECEU_PH_QBR_MM")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1589
  {DBGFIELD("PRECRQU_S_QB_PH_MM") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1590
  {DBGFIELD("PRECRQ_PH_W_MM")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1591
  {DBGFIELD("PRECRQ_QB_PH_MM")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1592
  {DBGFIELD("PRECRQ_RS_PH_W_MM")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1593
  {DBGFIELD("RADDU_W_QB_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1594
  {DBGFIELD("RDDSP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1595
  {DBGFIELD("REPLV_PH_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1596
  {DBGFIELD("REPLV_QB_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1597
  {DBGFIELD("REPL_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1598
  {DBGFIELD("REPL_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1599
  {DBGFIELD("SHILOV_MM")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1600
  {DBGFIELD("SHILO_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1601
  {DBGFIELD("SHLLV_PH_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1602
  {DBGFIELD("SHLLV_QB_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1603
  {DBGFIELD("SHLLV_S_PH_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1604
  {DBGFIELD("SHLLV_S_W_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1605
  {DBGFIELD("SHLL_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1606
  {DBGFIELD("SHLL_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1607
  {DBGFIELD("SHLL_S_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1608
  {DBGFIELD("SHLL_S_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1609
  {DBGFIELD("SHRAV_PH_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1610
  {DBGFIELD("SHRAV_R_PH_MM")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1611
  {DBGFIELD("SHRAV_R_W_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1612
  {DBGFIELD("SHRA_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1613
  {DBGFIELD("SHRA_R_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1614
  {DBGFIELD("SHRA_R_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1615
  {DBGFIELD("SHRLV_QB_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1616
  {DBGFIELD("SHRL_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1617
  {DBGFIELD("SUBQ_PH_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1618
  {DBGFIELD("SUBQ_S_PH_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1619
  {DBGFIELD("SUBQ_S_W_MM")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1620
  {DBGFIELD("SUBU_QB_MM")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1621
  {DBGFIELD("SUBU_S_QB_MM")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1622
  {DBGFIELD("WRDSP_MM")           8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1623
  {DBGFIELD("ABSQ_S_QB_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1624
  {DBGFIELD("ADDQH_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1625
  {DBGFIELD("ADDQH_R_PH_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1626
  {DBGFIELD("ADDQH_R_W_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1627
  {DBGFIELD("ADDQH_W_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1628
  {DBGFIELD("ADDUH_QB_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1629
  {DBGFIELD("ADDUH_R_QB_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1630
  {DBGFIELD("ADDU_PH_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1631
  {DBGFIELD("ADDU_S_PH_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1632
  {DBGFIELD("APPEND_MMR2")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1633
  {DBGFIELD("BALIGN_MMR2")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1634
  {DBGFIELD("CMPGDU_EQ_QB_MMR2")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1635
  {DBGFIELD("CMPGDU_LE_QB_MMR2")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1636
  {DBGFIELD("CMPGDU_LT_QB_MMR2")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1637
  {DBGFIELD("DPA_W_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1638
  {DBGFIELD("DPAQX_SA_W_PH_MMR2") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1639
  {DBGFIELD("DPAQX_S_W_PH_MMR2")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1640
  {DBGFIELD("DPAX_W_PH_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1641
  {DBGFIELD("DPS_W_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1642
  {DBGFIELD("DPSQX_S_W_PH_MMR2")  8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1643
  {DBGFIELD("DPSQX_SA_W_PH_MMR2") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1644
  {DBGFIELD("DPSX_W_PH_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1645
  {DBGFIELD("MUL_PH_MMR2")        8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1646
  {DBGFIELD("MUL_S_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1647
  {DBGFIELD("MULQ_RS_W_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1648
  {DBGFIELD("MULQ_S_PH_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1649
  {DBGFIELD("MULQ_S_W_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1650
  {DBGFIELD("MULSA_W_PH_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1651
  {DBGFIELD("PRECR_QB_PH_MMR2")   8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1652
  {DBGFIELD("PRECR_SRA_PH_W_MMR2") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1653
  {DBGFIELD("PRECR_SRA_R_PH_W_MMR2") 8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1654
  {DBGFIELD("PREPEND_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1655
  {DBGFIELD("SHRA_QB_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1656
  {DBGFIELD("SHRA_R_QB_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1657
  {DBGFIELD("SHRAV_QB_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1658
  {DBGFIELD("SHRAV_R_QB_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1659
  {DBGFIELD("SHRL_PH_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1660
  {DBGFIELD("SHRLV_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1661
  {DBGFIELD("SUBQH_PH_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1662
  {DBGFIELD("SUBQH_R_PH_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1663
  {DBGFIELD("SUBQH_W_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1664
  {DBGFIELD("SUBQH_R_W_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1665
  {DBGFIELD("SUBU_PH_MMR2")       8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1666
  {DBGFIELD("SUBU_S_PH_MMR2")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1667
  {DBGFIELD("SUBUH_QB_MMR2")      8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1668
  {DBGFIELD("SUBUH_R_QB_MMR2")    8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1669
  {DBGFIELD("BPOSGE32C_MMR3")     8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1670
  {DBGFIELD("CMP_F_D")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1671
  {DBGFIELD("CMP_F_S")            8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1672
  {DBGFIELD("CMP_SAF_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1673
  {DBGFIELD("CMP_SAF_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1674
  {DBGFIELD("CMP_SEQ_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1675
  {DBGFIELD("CMP_SEQ_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1676
  {DBGFIELD("CMP_SLE_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1677
  {DBGFIELD("CMP_SLE_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1678
  {DBGFIELD("CMP_SLT_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1679
  {DBGFIELD("CMP_SLT_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1680
  {DBGFIELD("CMP_SUEQ_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1681
  {DBGFIELD("CMP_SUEQ_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1682
  {DBGFIELD("CMP_SULE_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1683
  {DBGFIELD("CMP_SULE_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1684
  {DBGFIELD("CMP_SULT_D")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1685
  {DBGFIELD("CMP_SULT_S")         8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1686
  {DBGFIELD("CMP_SUN_D")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1687
  {DBGFIELD("CMP_SUN_S")          8191, false, false, false,  0, 0,  0, 0,  0, 0}, // #1688
  {DBGFIELD("P5600WriteALU")      1, false, false, false,  5, 2,  1, 1,  0, 0}, // #1689
  {DBGFIELD("P5600WriteAL2")      1, false, false, false,  1, 2,  1, 1,  0, 0}, // #1690
}; // MipsP5600ModelSchedClasses

#undef DBGFIELD

static const llvm::MCSchedModel NoSchedModel = {
  MCSchedModel::DefaultIssueWidth,
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  MCSchedModel::DefaultLoadLatency,
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  0, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

static const unsigned MipsGenericModelProcResourceSubUnits[] = {
  0,  // Invalid
};

// {Name, NumUnits, SuperIdx, BufferSize, SubUnitsIdxBegin}
static const llvm::MCProcResourceDesc MipsGenericModelProcResources[] = {
  {"InvalidUnit", 0, 0, 0, 0},
  {"GenericALU",      1, 0, 1, nullptr}, // #1
  {"GenericAtomic",   1, 0, 1, nullptr}, // #2
  {"GenericCOP0",     1, 0, 1, nullptr}, // #3
  {"GenericCOP2",     1, 0, 1, nullptr}, // #4
  {"GenericDSP",      1, 0, 1, nullptr}, // #5
  {"GenericFPQ",      1, 0, 1, nullptr}, // #6
  {"GenericFPUDivSqrt", 1, 6, -1, nullptr}, // #7, Super=GenericFPQ
  {"GenericIssueALU", 1, 1, -1, nullptr}, // #8, Super=GenericALU
  {"GenericIssueCOP0", 1, 3, -1, nullptr}, // #9, Super=GenericCOP0
  {"GenericIssueCTISTD", 1, 1, -1, nullptr}, // #10, Super=GenericALU
  {"GenericIssueDIV", 1, 20, -1, nullptr}, // #11, Super=GenericMDU
  {"GenericIssueFPUL", 1, 6, -1, nullptr}, // #12, Super=GenericFPQ
  {"GenericIssueFPULoad", 1, 6, -1, nullptr}, // #13, Super=GenericFPQ
  {"GenericIssueFPUMove", 1, 6, -1, nullptr}, // #14, Super=GenericFPQ
  {"GenericIssueFPUS", 1, 6, -1, nullptr}, // #15, Super=GenericFPQ
  {"GenericIssueFPUStore", 1, 6, -1, nullptr}, // #16, Super=GenericFPQ
  {"GenericIssueLDST", 1, 19, -1, nullptr}, // #17, Super=GenericLDST
  {"GenericIssueMDU", 1, 1, -1, nullptr}, // #18, Super=GenericALU
  {"GenericLDST",     1, 0, 1, nullptr}, // #19
  {"GenericMDU",      1, 0, 1, nullptr}, // #20
};

static const llvm::MCSchedModel MipsGenericModel = {
  1, // IssueWidth
  0, // MicroOpBufferSize
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  2, // LoadLatency
  37, // HighLatency
  4, // MispredictPenalty
  true, // PostRAScheduler
  true, // CompleteModel
  1, // Processor ID
  MipsGenericModelProcResources,
  MipsGenericModelSchedClasses,
  21,
  1691,
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

static const unsigned MipsP5600ModelProcResourceSubUnits[] = {
  0,  // Invalid
};

// {Name, NumUnits, SuperIdx, BufferSize, SubUnitsIdxBegin}
static const llvm::MCProcResourceDesc MipsP5600ModelProcResources[] = {
  {"InvalidUnit", 0, 0, 0, 0},
  {"P5600AGQ",        3, 0, 16, nullptr}, // #1
  {"P5600AL2Div",     1, 0, -1, nullptr}, // #2
  {"P5600ALQ",        1, 0, 16, nullptr}, // #3
  {"P5600Atomic",     1, 0, 1, nullptr}, // #4
  {"P5600CTISTD",     1, 0, -1, nullptr}, // #5
  {"P5600FPQ",        3, 0, 16, nullptr}, // #6
  {"P5600FPUDivSqrt", 2, 0, -1, nullptr}, // #7
  {"P5600IssueAL2",   1, 1, -1, nullptr}, // #8, Super=P5600AGQ
  {"P5600IssueALU",   1, 3, -1, nullptr}, // #9, Super=P5600ALQ
  {"P5600IssueCTISTD", 1, 1, -1, nullptr}, // #10, Super=P5600AGQ
  {"P5600IssueFPUL",  1, 6, -1, nullptr}, // #11, Super=P5600FPQ
  {"P5600IssueFPULoad", 1, 6, -1, nullptr}, // #12, Super=P5600FPQ
  {"P5600IssueFPUS",  1, 6, -1, nullptr}, // #13, Super=P5600FPQ
  {"P5600IssueLDST",  1, 1, -1, nullptr}, // #14, Super=P5600AGQ
};

static const llvm::MCSchedModel MipsP5600Model = {
  2, // IssueWidth
  48, // MicroOpBufferSize
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  4, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  8, // MispredictPenalty
  false, // PostRAScheduler
  true, // CompleteModel
  2, // Processor ID
  MipsP5600ModelProcResources,
  MipsP5600ModelSchedClasses,
  15,
  1691,
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

// Sorted (by key) array of values for CPU subtype.
extern const llvm::SubtargetSubTypeKV MipsSubTypeKV[] = {
 { "generic", { { { 0x10000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips1", { { { 0x10000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips2", { { { 0x20000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips3", { { { 0x40000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips32", { { { 0x10000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips32r2", { { { 0x20000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips32r3", { { { 0x40000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips32r5", { { { 0x80000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips32r6", { { { 0x100000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips4", { { { 0x400000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips5", { { { 0x2000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips64", { { { 0x200000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips64r2", { { { 0x400000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips64r3", { { { 0x800000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips64r5", { { { 0x1000000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "mips64r6", { { { 0x2000000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "octeon", { { { 0x400000004ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "octeon+", { { { 0x40000000cULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsGenericModel },
 { "p5600", { { { 0x8000000000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &MipsP5600Model },
};

namespace Mips_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass,
    const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) {
  // Don't know how to resolve this scheduling class.
  return 0;
}
} // end namespace Mips_MC

struct MipsGenMCSubtargetInfo : public MCSubtargetInfo {
  MipsGenMCSubtargetInfo(const Triple &TT,
    StringRef CPU, StringRef TuneCPU, StringRef FS,
    ArrayRef<SubtargetFeatureKV> PF,
    ArrayRef<SubtargetSubTypeKV> PD,
    const MCWriteProcResEntry *WPR,
    const MCWriteLatencyEntry *WL,
    const MCReadAdvanceEntry *RA, const InstrStage *IS,
    const unsigned *OC, const unsigned *FP) :
      MCSubtargetInfo(TT, CPU, TuneCPU, FS, PF, PD,
                      WPR, WL, RA, IS, OC, FP) { }

  unsigned resolveVariantSchedClass(unsigned SchedClass,
      const MCInst *MI, const MCInstrInfo *MCII,
      unsigned CPUID) const override {
    return Mips_MC::resolveVariantSchedClassImpl(SchedClass, MI, MCII, CPUID);
  }
};

static inline MCSubtargetInfo *createMipsMCSubtargetInfoImpl(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS) {
  return new MipsGenMCSubtargetInfo(TT, CPU, TuneCPU, FS, MipsFeatureKV, MipsSubTypeKV, 
                      MipsWriteProcResTable, MipsWriteLatencyTable, MipsReadAdvanceTable, 
                      nullptr, nullptr, nullptr);
}

} // end namespace llvm

#endif // GET_SUBTARGETINFO_MC_DESC


#ifdef GET_SUBTARGETINFO_TARGET_DESC
#undef GET_SUBTARGETINFO_TARGET_DESC

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

// ParseSubtargetFeatures - Parses features string setting specified
// subtarget options.
void llvm::MipsSubtarget::ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS) {
  LLVM_DEBUG(dbgs() << "\nFeatures:" << FS);
  LLVM_DEBUG(dbgs() << "\nCPU:" << CPU);
  LLVM_DEBUG(dbgs() << "\nTuneCPU:" << TuneCPU << "\n\n");
  InitMCProcessorInfo(CPU, TuneCPU, FS);
  const FeatureBitset &Bits = getFeatureBits();
  if (Bits[Mips::FeatureAbs2008]) Abs2008 = true;
  if (Bits[Mips::FeatureCRC]) HasCRC = true;
  if (Bits[Mips::FeatureCnMips]) HasCnMips = true;
  if (Bits[Mips::FeatureCnMipsP]) HasCnMipsP = true;
  if (Bits[Mips::FeatureDSP]) HasDSP = true;
  if (Bits[Mips::FeatureDSPR2]) HasDSPR2 = true;
  if (Bits[Mips::FeatureDSPR3]) HasDSPR3 = true;
  if (Bits[Mips::FeatureEVA]) HasEVA = true;
  if (Bits[Mips::FeatureFP64Bit]) IsFP64bit = true;
  if (Bits[Mips::FeatureFPXX]) IsFPXX = true;
  if (Bits[Mips::FeatureGINV]) HasGINV = true;
  if (Bits[Mips::FeatureGP64Bit]) IsGP64bit = true;
  if (Bits[Mips::FeatureLongCalls]) UseLongCalls = true;
  if (Bits[Mips::FeatureMSA]) HasMSA = true;
  if (Bits[Mips::FeatureMT]) HasMT = true;
  if (Bits[Mips::FeatureMicroMips]) InMicroMipsMode = true;
  if (Bits[Mips::FeatureMips1] && MipsArchVersion < Mips1) MipsArchVersion = Mips1;
  if (Bits[Mips::FeatureMips2] && MipsArchVersion < Mips2) MipsArchVersion = Mips2;
  if (Bits[Mips::FeatureMips3] && MipsArchVersion < Mips3) MipsArchVersion = Mips3;
  if (Bits[Mips::FeatureMips3D]) Has3D = true;
  if (Bits[Mips::FeatureMips3_32]) HasMips3_32 = true;
  if (Bits[Mips::FeatureMips3_32r2]) HasMips3_32r2 = true;
  if (Bits[Mips::FeatureMips4] && MipsArchVersion < Mips4) MipsArchVersion = Mips4;
  if (Bits[Mips::FeatureMips4_32]) HasMips4_32 = true;
  if (Bits[Mips::FeatureMips4_32r2]) HasMips4_32r2 = true;
  if (Bits[Mips::FeatureMips5] && MipsArchVersion < Mips5) MipsArchVersion = Mips5;
  if (Bits[Mips::FeatureMips5_32r2]) HasMips5_32r2 = true;
  if (Bits[Mips::FeatureMips16]) InMips16Mode = true;
  if (Bits[Mips::FeatureMips32] && MipsArchVersion < Mips32) MipsArchVersion = Mips32;
  if (Bits[Mips::FeatureMips32r2] && MipsArchVersion < Mips32r2) MipsArchVersion = Mips32r2;
  if (Bits[Mips::FeatureMips32r3] && MipsArchVersion < Mips32r3) MipsArchVersion = Mips32r3;
  if (Bits[Mips::FeatureMips32r5] && MipsArchVersion < Mips32r5) MipsArchVersion = Mips32r5;
  if (Bits[Mips::FeatureMips32r6] && MipsArchVersion < Mips32r6) MipsArchVersion = Mips32r6;
  if (Bits[Mips::FeatureMips64] && MipsArchVersion < Mips64) MipsArchVersion = Mips64;
  if (Bits[Mips::FeatureMips64r2] && MipsArchVersion < Mips64r2) MipsArchVersion = Mips64r2;
  if (Bits[Mips::FeatureMips64r3] && MipsArchVersion < Mips64r3) MipsArchVersion = Mips64r3;
  if (Bits[Mips::FeatureMips64r5] && MipsArchVersion < Mips64r5) MipsArchVersion = Mips64r5;
  if (Bits[Mips::FeatureMips64r6] && MipsArchVersion < Mips64r6) MipsArchVersion = Mips64r6;
  if (Bits[Mips::FeatureNaN2008]) IsNaN2008bit = true;
  if (Bits[Mips::FeatureNoABICalls]) NoABICalls = true;
  if (Bits[Mips::FeatureNoMadd4]) DisableMadd4 = true;
  if (Bits[Mips::FeatureNoOddSPReg]) UseOddSPReg = false;
  if (Bits[Mips::FeaturePTR64Bit]) IsPTR64bit = true;
  if (Bits[Mips::FeatureSingleFloat]) IsSingleFloat = true;
  if (Bits[Mips::FeatureSoftFloat]) IsSoftFloat = true;
  if (Bits[Mips::FeatureSym32]) HasSym32 = true;
  if (Bits[Mips::FeatureUseIndirectJumpsHazard]) UseIndirectJumpsHazard = true;
  if (Bits[Mips::FeatureUseTCCInDIV]) UseTCCInDIV = false;
  if (Bits[Mips::FeatureVFPU]) HasVFPU = true;
  if (Bits[Mips::FeatureVirt]) HasVirt = true;
  if (Bits[Mips::FeatureXGOT]) UseXGOT = true;
  if (Bits[Mips::ImplP5600] && ProcImpl < MipsSubtarget::CPU::P5600) ProcImpl = MipsSubtarget::CPU::P5600;
}
#endif // GET_SUBTARGETINFO_TARGET_DESC


#ifdef GET_SUBTARGETINFO_HEADER
#undef GET_SUBTARGETINFO_HEADER

namespace llvm {
class DFAPacketizer;
namespace Mips_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID);
} // end namespace Mips_MC

struct MipsGenSubtargetInfo : public TargetSubtargetInfo {
  explicit MipsGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS);
public:
  unsigned resolveSchedClass(unsigned SchedClass,  const MachineInstr *DefMI, const TargetSchedModel *SchedModel) const override;
  unsigned resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) const override;
  DFAPacketizer *createDFAPacketizer(const InstrItineraryData *IID) const;
};
} // end namespace llvm

#endif // GET_SUBTARGETINFO_HEADER


#ifdef GET_SUBTARGETINFO_CTOR
#undef GET_SUBTARGETINFO_CTOR

#include "llvm/CodeGen/TargetSchedule.h"

namespace llvm {
extern const llvm::SubtargetFeatureKV MipsFeatureKV[];
extern const llvm::SubtargetSubTypeKV MipsSubTypeKV[];
extern const llvm::MCWriteProcResEntry MipsWriteProcResTable[];
extern const llvm::MCWriteLatencyEntry MipsWriteLatencyTable[];
extern const llvm::MCReadAdvanceEntry MipsReadAdvanceTable[];
MipsGenSubtargetInfo::MipsGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS)
  : TargetSubtargetInfo(TT, CPU, TuneCPU, FS, ArrayRef(MipsFeatureKV, 52), ArrayRef(MipsSubTypeKV, 19), 
                        MipsWriteProcResTable, MipsWriteLatencyTable, MipsReadAdvanceTable, 
                        nullptr, nullptr, nullptr) {}

unsigned MipsGenSubtargetInfo
::resolveSchedClass(unsigned SchedClass, const MachineInstr *MI, const TargetSchedModel *SchedModel) const {
  switch (SchedClass) {
  case 496: // ADD
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 497: // ADDi
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 498: // ADDiu
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 499: // ANDi
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 500: // ORi
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 501: // ROTR
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 502: // SEB
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 503: // SEH
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 504: // SLT_SLTu
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 505: // SLL
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 506: // SRA
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 507: // SRL
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 508: // XORi
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 509: // ADDu
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 510: // SLLV
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 511: // SRAV
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 512: // SRLV
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 513: // LSA
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 514: // COPY
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  case 720: // ROTRV
    if (SchedModel->getProcessorID() == 2) { // MipsP5600Model
      if (1)
        return 1689; // P5600WriteALU
      if (0)
        return 1690; // P5600WriteAL2
    }
    break;
  };
  report_fatal_error("Expected a variant SchedClass");
} // MipsGenSubtargetInfo::resolveSchedClass

unsigned MipsGenSubtargetInfo
::resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) const {
  return Mips_MC::resolveVariantSchedClassImpl(SchedClass, MI, MCII, CPUID);
} // MipsGenSubtargetInfo::resolveVariantSchedClass

} // end namespace llvm

#endif // GET_SUBTARGETINFO_CTOR


#ifdef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS


#ifdef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

