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
namespace RISCV {
enum {
  Feature64Bit = 0,
  FeatureRV32E = 1,
  FeatureRVCHints = 2,
  FeatureRelax = 3,
  FeatureReserveX1 = 4,
  FeatureReserveX2 = 5,
  FeatureReserveX3 = 6,
  FeatureReserveX4 = 7,
  FeatureReserveX5 = 8,
  FeatureReserveX6 = 9,
  FeatureReserveX7 = 10,
  FeatureReserveX8 = 11,
  FeatureReserveX9 = 12,
  FeatureReserveX10 = 13,
  FeatureReserveX11 = 14,
  FeatureReserveX12 = 15,
  FeatureReserveX13 = 16,
  FeatureReserveX14 = 17,
  FeatureReserveX15 = 18,
  FeatureReserveX16 = 19,
  FeatureReserveX17 = 20,
  FeatureReserveX18 = 21,
  FeatureReserveX19 = 22,
  FeatureReserveX20 = 23,
  FeatureReserveX21 = 24,
  FeatureReserveX22 = 25,
  FeatureReserveX23 = 26,
  FeatureReserveX24 = 27,
  FeatureReserveX25 = 28,
  FeatureReserveX26 = 29,
  FeatureReserveX27 = 30,
  FeatureReserveX28 = 31,
  FeatureReserveX29 = 32,
  FeatureReserveX30 = 33,
  FeatureReserveX31 = 34,
  FeatureStdExtA = 35,
  FeatureStdExtC = 36,
  FeatureStdExtD = 37,
  FeatureStdExtF = 38,
  FeatureStdExtM = 39,
  NumSubtargetFeatures = 40
};
} // end namespace RISCV
} // end namespace llvm

#endif // GET_SUBTARGETINFO_ENUM


#ifdef GET_SUBTARGETINFO_MC_DESC
#undef GET_SUBTARGETINFO_MC_DESC

namespace llvm {
// Sorted (by key) array of values for CPU features.
extern const llvm::SubtargetFeatureKV RISCVFeatureKV[] = {
  { "64bit", "Implements RV64", RISCV::Feature64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "a", "'A' (Atomic Instructions)", RISCV::FeatureStdExtA, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "c", "'C' (Compressed Instructions)", RISCV::FeatureStdExtC, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "d", "'D' (Double-Precision Floating-Point)", RISCV::FeatureStdExtD, { { { 0x4000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "e", "Implements RV32E (provides 16 rather than 32 GPRs)", RISCV::FeatureRV32E, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "f", "'F' (Single-Precision Floating-Point)", RISCV::FeatureStdExtF, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "m", "'M' (Integer Multiplication and Division)", RISCV::FeatureStdExtM, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "relax", "Enable Linker relaxation.", RISCV::FeatureRelax, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x1", "Reserve X1", RISCV::FeatureReserveX1, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x10", "Reserve X10", RISCV::FeatureReserveX10, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x11", "Reserve X11", RISCV::FeatureReserveX11, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x12", "Reserve X12", RISCV::FeatureReserveX12, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x13", "Reserve X13", RISCV::FeatureReserveX13, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x14", "Reserve X14", RISCV::FeatureReserveX14, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x15", "Reserve X15", RISCV::FeatureReserveX15, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x16", "Reserve X16", RISCV::FeatureReserveX16, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x17", "Reserve X17", RISCV::FeatureReserveX17, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x18", "Reserve X18", RISCV::FeatureReserveX18, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x19", "Reserve X19", RISCV::FeatureReserveX19, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x2", "Reserve X2", RISCV::FeatureReserveX2, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x20", "Reserve X20", RISCV::FeatureReserveX20, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x21", "Reserve X21", RISCV::FeatureReserveX21, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x22", "Reserve X22", RISCV::FeatureReserveX22, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x23", "Reserve X23", RISCV::FeatureReserveX23, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x24", "Reserve X24", RISCV::FeatureReserveX24, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x25", "Reserve X25", RISCV::FeatureReserveX25, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x26", "Reserve X26", RISCV::FeatureReserveX26, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x27", "Reserve X27", RISCV::FeatureReserveX27, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x28", "Reserve X28", RISCV::FeatureReserveX28, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x29", "Reserve X29", RISCV::FeatureReserveX29, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x3", "Reserve X3", RISCV::FeatureReserveX3, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x30", "Reserve X30", RISCV::FeatureReserveX30, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x31", "Reserve X31", RISCV::FeatureReserveX31, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x4", "Reserve X4", RISCV::FeatureReserveX4, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x5", "Reserve X5", RISCV::FeatureReserveX5, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x6", "Reserve X6", RISCV::FeatureReserveX6, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x7", "Reserve X7", RISCV::FeatureReserveX7, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x8", "Reserve X8", RISCV::FeatureReserveX8, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "reserve-x9", "Reserve X9", RISCV::FeatureReserveX9, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "rvc-hints", "Enable RVC Hint Instructions.", RISCV::FeatureRVCHints, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
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
extern const llvm::MCWriteProcResEntry RISCVWriteProcResTable[] = {
  { 0,  0}, // Invalid
  { 1,  1}, // #1
  { 7,  1}, // #2
  { 2,  1}, // #3
  { 5, 34}, // #4
  { 3,  1}, // #5
  { 4, 20}, // #6
  { 6,  1}, // #7
  { 8,  1}, // #8
  { 6, 33}, // #9
  { 6, 34}, // #10
  { 4,  1}, // #11
  { 5, 20} // #12
}; // RISCVWriteProcResTable

// {Cycles, WriteResourceID}
extern const llvm::MCWriteLatencyEntry RISCVWriteLatencyTable[] = {
  { 0,  0}, // Invalid
  { 1,  0}, // #1 WriteIALU_WriteJmp_WriteFST64_WriteFST32_WriteJal_WriteJalr_WriteJmpReg_WriteNop_WriteShift_WriteSTW_WriteSTB_WriteAtomicSTW_WriteSTH_WriteIALU32_WriteSTD_WriteAtomicSTD_WriteShift32
  { 2,  0}, // #2 WriteAtomicW_WriteFLD64_WriteFLD32_WriteLDW_WriteFClass64_WriteFClass32_WriteFCvtF32ToF64_WriteFCvtI32ToF64_WriteFCvtF64ToF32_WriteFCvtI32ToF32_WriteFCvtF64ToI32_WriteFCvtF32ToI32_WriteFCmp64_WriteFCmp32_WriteFMovI32ToF32_WriteFMovF32ToI32_WriteAtomicLDW_WriteAtomicD_WriteLDD_WriteFCvtI64ToF64_WriteFCvtF64ToI64_WriteFCvtF32ToI64_WriteFCvtI64ToF32_WriteFMovI64ToF64_WriteFMovF64ToI64_WriteAtomicLDD_WriteLDWU
  { 3,  0}, // #3 WriteCSR_WriteLDB_WriteLDH
  {34,  0}, // #4 WriteIDiv_WriteIDiv32
  { 6,  0}, // #5 WriteFALU64
  { 4,  0}, // #6 WriteFALU32_WriteIMul_WriteIMul32
  {20,  0}, // #7 WriteFDiv32_WriteFSqrt32
  { 7,  0}, // #8 WriteFMulAdd64_WriteFMulSub64
  { 5,  0}, // #9 WriteFMulAdd32_WriteFMulSub32_WriteFMul32
  {33,  0} // #10 WriteIDiv
}; // RISCVWriteLatencyTable

// {UseIdx, WriteResourceID, Cycles}
extern const llvm::MCReadAdvanceEntry RISCVReadAdvanceTable[] = {
  {0,  0,  0}, // Invalid
  {0,  0,  0}, // #1
  {1,  0,  0}, // #2
  {0,  0,  0}, // #3
  {1,  0,  0}, // #4
  {2,  0,  0} // #5
}; // RISCVReadAdvanceTable

// {Name, NumMicroOps, BeginGroup, EndGroup, WriteProcResIdx,#, WriteLatencyIdx,#, ReadAdvanceIdx,#}
static const llvm::MCSchedClassDesc Rocket32ModelSchedClasses[] = {
  {DBGFIELD("InvalidSchedClass")  16383, false, false,  0, 0,  0, 0,  0, 0},
  {DBGFIELD("WriteIALU_ReadIALU_ReadIALU") 1, false, false,  1, 1,  1, 1,  1, 2}, // #1
  {DBGFIELD("WriteIALU_ReadIALU") 1, false, false,  1, 1,  1, 1,  0, 1}, // #2
  {DBGFIELD("WriteIALU32_ReadIALU32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #3
  {DBGFIELD("WriteIALU32_ReadIALU32_ReadIALU32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #4
  {DBGFIELD("WriteAtomicD_ReadAtomicDA_ReadAtomicDD") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #5
  {DBGFIELD("WriteAtomicW_ReadAtomicWA_ReadAtomicWD") 1, false, false,  2, 1,  2, 1,  1, 2}, // #6
  {DBGFIELD("WriteIALU")          1, false, false,  1, 1,  1, 1,  0, 0}, // #7
  {DBGFIELD("WriteJmp")           1, false, false,  3, 1,  1, 1,  0, 0}, // #8
  {DBGFIELD("WriteCSR_ReadCSR")   1, false, false,  1, 1,  3, 1,  0, 1}, // #9
  {DBGFIELD("WriteCSR")           1, false, false,  1, 1,  3, 1,  0, 0}, // #10
  {DBGFIELD("WriteFLD64_ReadMemBase") 1, false, false,  2, 1,  2, 1,  0, 1}, // #11
  {DBGFIELD("WriteFLD32_ReadMemBase") 1, false, false,  2, 1,  2, 1,  0, 1}, // #12
  {DBGFIELD("WriteFST64_ReadStoreData_ReadMemBase") 1, false, false,  2, 1,  1, 1,  1, 2}, // #13
  {DBGFIELD("WriteFST32_ReadStoreData_ReadMemBase") 1, false, false,  2, 1,  1, 1,  1, 2}, // #14
  {DBGFIELD("WriteJal")           1, false, false,  3, 1,  1, 1,  0, 0}, // #15
  {DBGFIELD("WriteJalr_ReadJalr") 1, false, false,  3, 1,  1, 1,  0, 1}, // #16
  {DBGFIELD("WriteJmpReg")        1, false, false,  3, 1,  1, 1,  0, 0}, // #17
  {DBGFIELD("WriteLDD_ReadMemBase") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #18
  {DBGFIELD("WriteLDW_ReadMemBase") 1, false, false,  2, 1,  2, 1,  0, 1}, // #19
  {DBGFIELD("WriteNop")           1, false, false,  0, 0,  1, 1,  0, 0}, // #20
  {DBGFIELD("WriteSTD_ReadStoreData_ReadMemBase") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #21
  {DBGFIELD("WriteShift_ReadShift") 1, false, false,  1, 1,  1, 1,  0, 1}, // #22
  {DBGFIELD("WriteSTW_ReadStoreData_ReadMemBase") 1, false, false,  2, 1,  1, 1,  1, 2}, // #23
  {DBGFIELD("WriteIDiv_ReadIDiv_ReadIDiv") 1, false, false,  4, 1,  4, 1,  1, 2}, // #24
  {DBGFIELD("WriteIDiv32_ReadIDiv32_ReadIDiv32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #25
  {DBGFIELD("WriteFALU64_ReadFALU64_ReadFALU64") 1, false, false,  5, 1,  5, 1,  1, 2}, // #26
  {DBGFIELD("WriteFALU32_ReadFALU32_ReadFALU32") 1, false, false,  5, 1,  6, 1,  1, 2}, // #27
  {DBGFIELD("WriteFClass64_ReadFClass64") 1, false, false,  5, 1,  2, 1,  0, 1}, // #28
  {DBGFIELD("WriteFClass32_ReadFClass32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #29
  {DBGFIELD("WriteFCvtI64ToF64_ReadFCvtI64ToF64") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #30
  {DBGFIELD("WriteFCvtF32ToF64_ReadFCvtF32ToF64") 1, false, false,  5, 1,  2, 1,  0, 1}, // #31
  {DBGFIELD("WriteFCvtI32ToF64_ReadFCvtI32ToF64") 1, false, false,  5, 1,  2, 1,  0, 1}, // #32
  {DBGFIELD("WriteFCvtF64ToI64_ReadFCvtF64ToI64") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #33
  {DBGFIELD("WriteFCvtF32ToI64_ReadFCvtF32ToI64") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #34
  {DBGFIELD("WriteFCvtF64ToF32_ReadFCvtF64ToF32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #35
  {DBGFIELD("WriteFCvtI64ToF32_ReadFCvtI64ToF32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #36
  {DBGFIELD("WriteFCvtI32ToF32_ReadFCvtI32ToF32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #37
  {DBGFIELD("WriteFCvtF64ToI32_ReadFCvtF64ToI32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #38
  {DBGFIELD("WriteFCvtF32ToI32_ReadFCvtF32ToI32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #39
  {DBGFIELD("WriteFDiv32_ReadFDiv32_ReadFDiv32") 1, false, false,  6, 1,  7, 1,  1, 2}, // #40
  {DBGFIELD("WriteFCmp64_ReadFCmp64_ReadFCmp64") 1, false, false,  5, 1,  2, 1,  1, 2}, // #41
  {DBGFIELD("WriteFCmp32_ReadFCmp32_ReadFCmp32") 1, false, false,  5, 1,  2, 1,  1, 2}, // #42
  {DBGFIELD("WriteFMulAdd64_ReadFMulAdd64_ReadFMulAdd64_ReadFMulAdd64") 1, false, false,  5, 1,  8, 1,  3, 3}, // #43
  {DBGFIELD("WriteFMulAdd32_ReadFMulAdd32_ReadFMulAdd32_ReadFMulAdd32") 1, false, false,  5, 1,  9, 1,  3, 3}, // #44
  {DBGFIELD("WriteFMulSub64_ReadFMulSub64_ReadFMulSub64_ReadFMulSub64") 1, false, false,  5, 1,  8, 1,  3, 3}, // #45
  {DBGFIELD("WriteFMulSub32_ReadFMulSub32_ReadFMulSub32_ReadFMulSub32") 1, false, false,  5, 1,  9, 1,  3, 3}, // #46
  {DBGFIELD("WriteFMul32_ReadFMul32_ReadFMul32") 1, false, false,  5, 1,  9, 1,  1, 2}, // #47
  {DBGFIELD("WriteFMovI64ToF64_ReadFMovI64ToF64") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #48
  {DBGFIELD("WriteFMovI32ToF32_ReadFMovI32ToF32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #49
  {DBGFIELD("WriteFMovF64ToI64_ReadFMovF64ToI64") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #50
  {DBGFIELD("WriteFMovF32ToI32_ReadFMovF32ToI32") 1, false, false,  5, 1,  2, 1,  0, 1}, // #51
  {DBGFIELD("WriteFSqrt32_ReadFSqrt32") 1, false, false,  6, 1,  7, 1,  0, 1}, // #52
  {DBGFIELD("WriteLDB_ReadMemBase") 1, false, false,  2, 1,  3, 1,  0, 1}, // #53
  {DBGFIELD("WriteLDH_ReadMemBase") 1, false, false,  2, 1,  3, 1,  0, 1}, // #54
  {DBGFIELD("WriteAtomicLDD_ReadAtomicLDD") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #55
  {DBGFIELD("WriteAtomicLDW_ReadAtomicLDW") 1, false, false,  2, 1,  2, 1,  0, 1}, // #56
  {DBGFIELD("WriteLDWU_ReadMemBase") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #57
  {DBGFIELD("WriteIMul_ReadIMul_ReadIMul") 1, false, false,  7, 1,  6, 1,  1, 2}, // #58
  {DBGFIELD("WriteIMul32_ReadIMul32_ReadIMul32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #59
  {DBGFIELD("WriteSTB_ReadStoreData_ReadMemBase") 1, false, false,  2, 1,  1, 1,  1, 2}, // #60
  {DBGFIELD("WriteAtomicSTD_ReadAtomicSTD_ReadAtomicSTD") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #61
  {DBGFIELD("WriteAtomicSTW_ReadAtomicSTW_ReadAtomicSTW") 1, false, false,  2, 1,  1, 1,  1, 2}, // #62
  {DBGFIELD("WriteSTH_ReadStoreData_ReadMemBase") 1, false, false,  2, 1,  1, 1,  1, 2}, // #63
  {DBGFIELD("WriteShift32_ReadShift32") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #64
  {DBGFIELD("COPY")               1, false, false,  1, 1,  1, 1,  0, 0}, // #65
}; // Rocket32ModelSchedClasses

// {Name, NumMicroOps, BeginGroup, EndGroup, WriteProcResIdx,#, WriteLatencyIdx,#, ReadAdvanceIdx,#}
static const llvm::MCSchedClassDesc Rocket64ModelSchedClasses[] = {
  {DBGFIELD("InvalidSchedClass")  16383, false, false,  0, 0,  0, 0,  0, 0},
  {DBGFIELD("WriteIALU_ReadIALU_ReadIALU") 1, false, false,  3, 1,  1, 1,  1, 2}, // #1
  {DBGFIELD("WriteIALU_ReadIALU") 1, false, false,  3, 1,  1, 1,  0, 1}, // #2
  {DBGFIELD("WriteIALU32_ReadIALU32") 1, false, false,  3, 1,  1, 1,  0, 1}, // #3
  {DBGFIELD("WriteIALU32_ReadIALU32_ReadIALU32") 1, false, false,  3, 1,  1, 1,  1, 2}, // #4
  {DBGFIELD("WriteAtomicD_ReadAtomicDA_ReadAtomicDD") 1, false, false,  8, 1,  2, 1,  1, 2}, // #5
  {DBGFIELD("WriteAtomicW_ReadAtomicWA_ReadAtomicWD") 1, false, false,  8, 1,  2, 1,  1, 2}, // #6
  {DBGFIELD("WriteIALU")          1, false, false,  3, 1,  1, 1,  0, 0}, // #7
  {DBGFIELD("WriteJmp")           1, false, false,  5, 1,  1, 1,  0, 0}, // #8
  {DBGFIELD("WriteCSR_ReadCSR")   1, false, false,  3, 1,  3, 1,  0, 1}, // #9
  {DBGFIELD("WriteCSR")           1, false, false,  3, 1,  3, 1,  0, 0}, // #10
  {DBGFIELD("WriteFLD64_ReadMemBase") 1, false, false,  8, 1,  2, 1,  0, 1}, // #11
  {DBGFIELD("WriteFLD32_ReadMemBase") 1, false, false,  8, 1,  2, 1,  0, 1}, // #12
  {DBGFIELD("WriteFST64_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #13
  {DBGFIELD("WriteFST32_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #14
  {DBGFIELD("WriteJal")           1, false, false,  5, 1,  1, 1,  0, 0}, // #15
  {DBGFIELD("WriteJalr_ReadJalr") 1, false, false,  5, 1,  1, 1,  0, 1}, // #16
  {DBGFIELD("WriteJmpReg")        1, false, false,  5, 1,  1, 1,  0, 0}, // #17
  {DBGFIELD("WriteLDD_ReadMemBase") 1, false, false,  8, 1,  2, 1,  0, 1}, // #18
  {DBGFIELD("WriteLDW_ReadMemBase") 1, false, false,  8, 1,  2, 1,  0, 1}, // #19
  {DBGFIELD("WriteNop")           1, false, false,  0, 0,  1, 1,  0, 0}, // #20
  {DBGFIELD("WriteSTD_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #21
  {DBGFIELD("WriteShift_ReadShift") 1, false, false,  3, 1,  1, 1,  0, 1}, // #22
  {DBGFIELD("WriteSTW_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #23
  {DBGFIELD("WriteIDiv_ReadIDiv_ReadIDiv") 1, false, false,  9, 1, 10, 1,  1, 2}, // #24
  {DBGFIELD("WriteIDiv32_ReadIDiv32_ReadIDiv32") 1, false, false, 10, 1,  4, 1,  1, 2}, // #25
  {DBGFIELD("WriteFALU64_ReadFALU64_ReadFALU64") 1, false, false, 11, 1,  5, 1,  1, 2}, // #26
  {DBGFIELD("WriteFALU32_ReadFALU32_ReadFALU32") 1, false, false, 11, 1,  6, 1,  1, 2}, // #27
  {DBGFIELD("WriteFClass64_ReadFClass64") 1, false, false, 11, 1,  2, 1,  0, 1}, // #28
  {DBGFIELD("WriteFClass32_ReadFClass32") 1, false, false, 11, 1,  2, 1,  0, 1}, // #29
  {DBGFIELD("WriteFCvtI64ToF64_ReadFCvtI64ToF64") 1, false, false,  1, 1,  2, 1,  0, 1}, // #30
  {DBGFIELD("WriteFCvtF32ToF64_ReadFCvtF32ToF64") 1, false, false,  1, 1,  2, 1,  0, 1}, // #31
  {DBGFIELD("WriteFCvtI32ToF64_ReadFCvtI32ToF64") 1, false, false,  1, 1,  2, 1,  0, 1}, // #32
  {DBGFIELD("WriteFCvtF64ToI64_ReadFCvtF64ToI64") 1, false, false,  1, 1,  2, 1,  0, 1}, // #33
  {DBGFIELD("WriteFCvtF32ToI64_ReadFCvtF32ToI64") 1, false, false,  1, 1,  2, 1,  0, 1}, // #34
  {DBGFIELD("WriteFCvtF64ToF32_ReadFCvtF64ToF32") 1, false, false,  1, 1,  2, 1,  0, 1}, // #35
  {DBGFIELD("WriteFCvtI64ToF32_ReadFCvtI64ToF32") 1, false, false,  1, 1,  2, 1,  0, 1}, // #36
  {DBGFIELD("WriteFCvtI32ToF32_ReadFCvtI32ToF32") 1, false, false,  1, 1,  2, 1,  0, 1}, // #37
  {DBGFIELD("WriteFCvtF64ToI32_ReadFCvtF64ToI32") 1, false, false,  1, 1,  2, 1,  0, 1}, // #38
  {DBGFIELD("WriteFCvtF32ToI32_ReadFCvtF32ToI32") 1, false, false,  1, 1,  2, 1,  0, 1}, // #39
  {DBGFIELD("WriteFDiv32_ReadFDiv32_ReadFDiv32") 1, false, false, 12, 1,  7, 1,  1, 2}, // #40
  {DBGFIELD("WriteFCmp64_ReadFCmp64_ReadFCmp64") 1, false, false, 11, 1,  2, 1,  1, 2}, // #41
  {DBGFIELD("WriteFCmp32_ReadFCmp32_ReadFCmp32") 1, false, false, 11, 1,  2, 1,  1, 2}, // #42
  {DBGFIELD("WriteFMulAdd64_ReadFMulAdd64_ReadFMulAdd64_ReadFMulAdd64") 1, false, false, 11, 1,  8, 1,  3, 3}, // #43
  {DBGFIELD("WriteFMulAdd32_ReadFMulAdd32_ReadFMulAdd32_ReadFMulAdd32") 1, false, false, 11, 1,  9, 1,  3, 3}, // #44
  {DBGFIELD("WriteFMulSub64_ReadFMulSub64_ReadFMulSub64_ReadFMulSub64") 1, false, false, 11, 1,  8, 1,  3, 3}, // #45
  {DBGFIELD("WriteFMulSub32_ReadFMulSub32_ReadFMulSub32_ReadFMulSub32") 1, false, false, 11, 1,  9, 1,  3, 3}, // #46
  {DBGFIELD("WriteFMul32_ReadFMul32_ReadFMul32") 1, false, false, 11, 1,  9, 1,  1, 2}, // #47
  {DBGFIELD("WriteFMovI64ToF64_ReadFMovI64ToF64") 1, false, false, 11, 1,  2, 1,  0, 1}, // #48
  {DBGFIELD("WriteFMovI32ToF32_ReadFMovI32ToF32") 1, false, false, 11, 1,  2, 1,  0, 1}, // #49
  {DBGFIELD("WriteFMovF64ToI64_ReadFMovF64ToI64") 1, false, false, 11, 1,  2, 1,  0, 1}, // #50
  {DBGFIELD("WriteFMovF32ToI32_ReadFMovF32ToI32") 1, false, false, 11, 1,  2, 1,  0, 1}, // #51
  {DBGFIELD("WriteFSqrt32_ReadFSqrt32") 1, false, false, 12, 1,  7, 1,  0, 1}, // #52
  {DBGFIELD("WriteLDB_ReadMemBase") 1, false, false,  8, 1,  3, 1,  0, 1}, // #53
  {DBGFIELD("WriteLDH_ReadMemBase") 1, false, false,  8, 1,  3, 1,  0, 1}, // #54
  {DBGFIELD("WriteAtomicLDD_ReadAtomicLDD") 1, false, false,  8, 1,  2, 1,  0, 1}, // #55
  {DBGFIELD("WriteAtomicLDW_ReadAtomicLDW") 1, false, false,  8, 1,  2, 1,  0, 1}, // #56
  {DBGFIELD("WriteLDWU_ReadMemBase") 1, false, false,  8, 1,  2, 1,  0, 1}, // #57
  {DBGFIELD("WriteIMul_ReadIMul_ReadIMul") 1, false, false,  2, 1,  6, 1,  1, 2}, // #58
  {DBGFIELD("WriteIMul32_ReadIMul32_ReadIMul32") 1, false, false,  2, 1,  6, 1,  1, 2}, // #59
  {DBGFIELD("WriteSTB_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #60
  {DBGFIELD("WriteAtomicSTD_ReadAtomicSTD_ReadAtomicSTD") 1, false, false,  8, 1,  1, 1,  1, 2}, // #61
  {DBGFIELD("WriteAtomicSTW_ReadAtomicSTW_ReadAtomicSTW") 1, false, false,  8, 1,  1, 1,  1, 2}, // #62
  {DBGFIELD("WriteSTH_ReadStoreData_ReadMemBase") 1, false, false,  8, 1,  1, 1,  1, 2}, // #63
  {DBGFIELD("WriteShift32_ReadShift32") 1, false, false,  3, 1,  1, 1,  0, 1}, // #64
  {DBGFIELD("COPY")               1, false, false,  3, 1,  1, 1,  0, 0}, // #65
}; // Rocket64ModelSchedClasses

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

static const unsigned Rocket32ModelProcResourceSubUnits[] = {
  0,  // Invalid
};

// {Name, NumUnits, SuperIdx, BufferSize, SubUnitsIdxBegin}
static const llvm::MCProcResourceDesc Rocket32ModelProcResources[] = {
  {"InvalidUnit", 0, 0, 0, 0},
  {"Rocket32UnitALU", 1, 0, 0, nullptr}, // #1
  {"Rocket32UnitB",   1, 0, 0, nullptr}, // #2
  {"Rocket32UnitFPALU", 1, 0, 0, nullptr}, // #3
  {"Rocket32UnitFPDivSqrt", 1, 0, 1, nullptr}, // #4
  {"Rocket32UnitIDiv", 1, 0, 1, nullptr}, // #5
  {"Rocket32UnitIMul", 1, 0, 0, nullptr}, // #6
  {"Rocket32UnitMem", 1, 0, 0, nullptr}, // #7
};

static const llvm::MCSchedModel Rocket32Model = {
  1, // IssueWidth
  0, // MicroOpBufferSize
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  3, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  3, // MispredictPenalty
  false, // PostRAScheduler
  true, // CompleteModel
  1, // Processor ID
  Rocket32ModelProcResources,
  Rocket32ModelSchedClasses,
  8,
  66,
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

static const unsigned Rocket64ModelProcResourceSubUnits[] = {
  0,  // Invalid
};

// {Name, NumUnits, SuperIdx, BufferSize, SubUnitsIdxBegin}
static const llvm::MCProcResourceDesc Rocket64ModelProcResources[] = {
  {"InvalidUnit", 0, 0, 0, 0},
  {"Rocket32UnitFPALU", 1, 0, 0, nullptr}, // #1
  {"Rocket64UnitALU", 1, 0, 0, nullptr}, // #2
  {"Rocket64UnitB",   1, 0, 0, nullptr}, // #3
  {"Rocket64UnitFPALU", 1, 0, 0, nullptr}, // #4
  {"Rocket64UnitFPDivSqrt", 1, 0, 1, nullptr}, // #5
  {"Rocket64UnitIDiv", 1, 0, 1, nullptr}, // #6
  {"Rocket64UnitIMul", 1, 0, 0, nullptr}, // #7
  {"Rocket64UnitMem", 1, 0, 0, nullptr}, // #8
};

static const llvm::MCSchedModel Rocket64Model = {
  1, // IssueWidth
  0, // MicroOpBufferSize
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  3, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  3, // MispredictPenalty
  false, // PostRAScheduler
  true, // CompleteModel
  2, // Processor ID
  Rocket64ModelProcResources,
  Rocket64ModelSchedClasses,
  9,
  66,
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

// Sorted (by key) array of values for CPU subtype.
extern const llvm::SubtargetSubTypeKV RISCVSubTypeKV[] = {
 { "generic-rv32", { { { 0x4ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
 { "generic-rv64", { { { 0x5ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
 { "rocket-rv32", { { { 0x4ULL, 0x0ULL, 0x0ULL, } } }, &Rocket32Model },
 { "rocket-rv64", { { { 0x5ULL, 0x0ULL, 0x0ULL, } } }, &Rocket64Model },
};

namespace RISCV_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass,
    const MCInst *MI, unsigned CPUID) {
  // Don't know how to resolve this scheduling class.
  return 0;
}
} // end namespace RISCV_MC

struct RISCVGenMCSubtargetInfo : public MCSubtargetInfo {
  RISCVGenMCSubtargetInfo(const Triple &TT,
    StringRef CPU, StringRef FS, ArrayRef<SubtargetFeatureKV> PF,
    ArrayRef<SubtargetSubTypeKV> PD,
    const MCWriteProcResEntry *WPR,
    const MCWriteLatencyEntry *WL,
    const MCReadAdvanceEntry *RA, const InstrStage *IS,
    const unsigned *OC, const unsigned *FP) :
      MCSubtargetInfo(TT, CPU, FS, PF, PD,
                      WPR, WL, RA, IS, OC, FP) { }

  unsigned resolveVariantSchedClass(unsigned SchedClass,
      const MCInst *MI, unsigned CPUID) const override {
    return RISCV_MC::resolveVariantSchedClassImpl(SchedClass, MI, CPUID);
  }
  unsigned getHwMode() const override;
};
unsigned RISCVGenMCSubtargetInfo::getHwMode() const {
  if (checkFeatures("-64bit")) return 1;
  if (checkFeatures("+64bit")) return 2;
  return 0;
}

static inline MCSubtargetInfo *createRISCVMCSubtargetInfoImpl(const Triple &TT, StringRef CPU, StringRef FS) {
  return new RISCVGenMCSubtargetInfo(TT, CPU, FS, RISCVFeatureKV, RISCVSubTypeKV,
                      RISCVWriteProcResTable, RISCVWriteLatencyTable, RISCVReadAdvanceTable,
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
void llvm::RISCVSubtarget::ParseSubtargetFeatures(StringRef CPU, StringRef FS) {
  LLVM_DEBUG(dbgs() << "\nFeatures:" << FS);
  LLVM_DEBUG(dbgs() << "\nCPU:" << CPU << "\n\n");
  InitMCProcessorInfo(CPU, FS);
  const FeatureBitset& Bits = getFeatureBits();
  if (Bits[RISCV::Feature64Bit]) HasRV64 = true;
  if (Bits[RISCV::FeatureRV32E]) IsRV32E = true;
  if (Bits[RISCV::FeatureRVCHints]) EnableRVCHintInstrs = true;
  if (Bits[RISCV::FeatureRelax]) EnableLinkerRelax = true;
  if (Bits[RISCV::FeatureReserveX1]) UserReservedRegister[RISCV::X1] = true;
  if (Bits[RISCV::FeatureReserveX2]) UserReservedRegister[RISCV::X2] = true;
  if (Bits[RISCV::FeatureReserveX3]) UserReservedRegister[RISCV::X3] = true;
  if (Bits[RISCV::FeatureReserveX4]) UserReservedRegister[RISCV::X4] = true;
  if (Bits[RISCV::FeatureReserveX5]) UserReservedRegister[RISCV::X5] = true;
  if (Bits[RISCV::FeatureReserveX6]) UserReservedRegister[RISCV::X6] = true;
  if (Bits[RISCV::FeatureReserveX7]) UserReservedRegister[RISCV::X7] = true;
  if (Bits[RISCV::FeatureReserveX8]) UserReservedRegister[RISCV::X8] = true;
  if (Bits[RISCV::FeatureReserveX9]) UserReservedRegister[RISCV::X9] = true;
  if (Bits[RISCV::FeatureReserveX10]) UserReservedRegister[RISCV::X10] = true;
  if (Bits[RISCV::FeatureReserveX11]) UserReservedRegister[RISCV::X11] = true;
  if (Bits[RISCV::FeatureReserveX12]) UserReservedRegister[RISCV::X12] = true;
  if (Bits[RISCV::FeatureReserveX13]) UserReservedRegister[RISCV::X13] = true;
  if (Bits[RISCV::FeatureReserveX14]) UserReservedRegister[RISCV::X14] = true;
  if (Bits[RISCV::FeatureReserveX15]) UserReservedRegister[RISCV::X15] = true;
  if (Bits[RISCV::FeatureReserveX16]) UserReservedRegister[RISCV::X16] = true;
  if (Bits[RISCV::FeatureReserveX17]) UserReservedRegister[RISCV::X17] = true;
  if (Bits[RISCV::FeatureReserveX18]) UserReservedRegister[RISCV::X18] = true;
  if (Bits[RISCV::FeatureReserveX19]) UserReservedRegister[RISCV::X19] = true;
  if (Bits[RISCV::FeatureReserveX20]) UserReservedRegister[RISCV::X20] = true;
  if (Bits[RISCV::FeatureReserveX21]) UserReservedRegister[RISCV::X21] = true;
  if (Bits[RISCV::FeatureReserveX22]) UserReservedRegister[RISCV::X22] = true;
  if (Bits[RISCV::FeatureReserveX23]) UserReservedRegister[RISCV::X23] = true;
  if (Bits[RISCV::FeatureReserveX24]) UserReservedRegister[RISCV::X24] = true;
  if (Bits[RISCV::FeatureReserveX25]) UserReservedRegister[RISCV::X25] = true;
  if (Bits[RISCV::FeatureReserveX26]) UserReservedRegister[RISCV::X26] = true;
  if (Bits[RISCV::FeatureReserveX27]) UserReservedRegister[RISCV::X27] = true;
  if (Bits[RISCV::FeatureReserveX28]) UserReservedRegister[RISCV::X28] = true;
  if (Bits[RISCV::FeatureReserveX29]) UserReservedRegister[RISCV::X29] = true;
  if (Bits[RISCV::FeatureReserveX30]) UserReservedRegister[RISCV::X30] = true;
  if (Bits[RISCV::FeatureReserveX31]) UserReservedRegister[RISCV::X31] = true;
  if (Bits[RISCV::FeatureStdExtA]) HasStdExtA = true;
  if (Bits[RISCV::FeatureStdExtC]) HasStdExtC = true;
  if (Bits[RISCV::FeatureStdExtD]) HasStdExtD = true;
  if (Bits[RISCV::FeatureStdExtF]) HasStdExtF = true;
  if (Bits[RISCV::FeatureStdExtM]) HasStdExtM = true;
}
#endif // GET_SUBTARGETINFO_TARGET_DESC


#ifdef GET_SUBTARGETINFO_HEADER
#undef GET_SUBTARGETINFO_HEADER

namespace llvm {
class DFAPacketizer;
namespace RISCV_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass, const MCInst *MI, unsigned CPUID);
} // end namespace RISCV_MC

struct RISCVGenSubtargetInfo : public TargetSubtargetInfo {
  explicit RISCVGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS);
public:
  unsigned resolveSchedClass(unsigned SchedClass,  const MachineInstr *DefMI, const TargetSchedModel *SchedModel) const override;
  unsigned resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, unsigned CPUID) const override;
  DFAPacketizer *createDFAPacketizer(const InstrItineraryData *IID) const;
  unsigned getHwMode() const override;
};
} // end namespace llvm

#endif // GET_SUBTARGETINFO_HEADER


#ifdef GET_SUBTARGETINFO_CTOR
#undef GET_SUBTARGETINFO_CTOR

#include "llvm/CodeGen/TargetSchedule.h"

namespace llvm {
extern const llvm::SubtargetFeatureKV RISCVFeatureKV[];
extern const llvm::SubtargetSubTypeKV RISCVSubTypeKV[];
extern const llvm::MCWriteProcResEntry RISCVWriteProcResTable[];
extern const llvm::MCWriteLatencyEntry RISCVWriteLatencyTable[];
extern const llvm::MCReadAdvanceEntry RISCVReadAdvanceTable[];
RISCVGenSubtargetInfo::RISCVGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS)
  : TargetSubtargetInfo(TT, CPU, FS, makeArrayRef(RISCVFeatureKV, 40), makeArrayRef(RISCVSubTypeKV, 4),
                        RISCVWriteProcResTable, RISCVWriteLatencyTable, RISCVReadAdvanceTable,
                        nullptr, nullptr, nullptr) {}

unsigned RISCVGenSubtargetInfo
::resolveSchedClass(unsigned SchedClass, const MachineInstr *MI, const TargetSchedModel *SchedModel) const {
  report_fatal_error("Expected a variant SchedClass");
} // RISCVGenSubtargetInfo::resolveSchedClass

unsigned RISCVGenSubtargetInfo
::resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, unsigned CPUID) const {
  return RISCV_MC::resolveVariantSchedClassImpl(SchedClass, MI, CPUID);
} // RISCVGenSubtargetInfo::resolveVariantSchedClass

unsigned RISCVGenSubtargetInfo::getHwMode() const {
  if (checkFeatures("-64bit")) return 1;
  if (checkFeatures("+64bit")) return 2;
  return 0;
}
} // end namespace llvm

#endif // GET_SUBTARGETINFO_CTOR


#ifdef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS


#ifdef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

