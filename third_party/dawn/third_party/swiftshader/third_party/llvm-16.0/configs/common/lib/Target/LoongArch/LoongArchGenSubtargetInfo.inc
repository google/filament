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
namespace LoongArch {
enum {
  Feature32Bit = 0,
  Feature64Bit = 1,
  FeatureBasicD = 2,
  FeatureBasicF = 3,
  FeatureExtLASX = 4,
  FeatureExtLBT = 5,
  FeatureExtLSX = 6,
  FeatureExtLVZ = 7,
  LaGlobalWithAbs = 8,
  LaGlobalWithPcrel = 9,
  LaLocalWithAbs = 10,
  NumSubtargetFeatures = 11
};
} // end namespace LoongArch
} // end namespace llvm

#endif // GET_SUBTARGETINFO_ENUM


#ifdef GET_SUBTARGETINFO_MACRO
GET_SUBTARGETINFO_MACRO(HasLA32, false, hasLA32)
GET_SUBTARGETINFO_MACRO(HasLA64, false, hasLA64)
GET_SUBTARGETINFO_MACRO(HasBasicD, false, hasBasicD)
GET_SUBTARGETINFO_MACRO(HasBasicF, false, hasBasicF)
GET_SUBTARGETINFO_MACRO(HasLaGlobalWithAbs, false, hasLaGlobalWithAbs)
GET_SUBTARGETINFO_MACRO(HasLaGlobalWithPcrel, false, hasLaGlobalWithPcrel)
GET_SUBTARGETINFO_MACRO(HasLaLocalWithAbs, false, hasLaLocalWithAbs)
GET_SUBTARGETINFO_MACRO(HasExtLASX, false, hasExtLASX)
GET_SUBTARGETINFO_MACRO(HasExtLBT, false, hasExtLBT)
GET_SUBTARGETINFO_MACRO(HasExtLSX, false, hasExtLSX)
GET_SUBTARGETINFO_MACRO(HasExtLVZ, false, hasExtLVZ)
#undef GET_SUBTARGETINFO_MACRO
#endif // GET_SUBTARGETINFO_MACRO


#ifdef GET_SUBTARGETINFO_MC_DESC
#undef GET_SUBTARGETINFO_MC_DESC

namespace llvm {
// Sorted (by key) array of values for CPU features.
extern const llvm::SubtargetFeatureKV LoongArchFeatureKV[] = {
  { "32bit", "LA32 Basic Integer and Privilege Instruction Set", LoongArch::Feature32Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "64bit", "LA64 Basic Integer and Privilege Instruction Set", LoongArch::Feature64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "d", "'D' (Double-Precision Floating-Point)", LoongArch::FeatureBasicD, { { { 0x8ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "f", "'F' (Single-Precision Floating-Point)", LoongArch::FeatureBasicF, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "la-global-with-abs", "Expand la.global as la.abs", LoongArch::LaGlobalWithAbs, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "la-global-with-pcrel", "Expand la.global as la.pcrel", LoongArch::LaGlobalWithPcrel, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "la-local-with-abs", "Expand la.local as la.abs", LoongArch::LaLocalWithAbs, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "lasx", "'LASX' (Loongson Advanced SIMD Extension)", LoongArch::FeatureExtLASX, { { { 0x40ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "lbt", "'LBT' (Loongson Binary Translation Extension)", LoongArch::FeatureExtLBT, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "lsx", "'LSX' (Loongson SIMD Extension)", LoongArch::FeatureExtLSX, { { { 0x4ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "lvz", "'LVZ' (Loongson Virtualization Extension)", LoongArch::FeatureExtLVZ, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } } },
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
extern const llvm::MCWriteProcResEntry LoongArchWriteProcResTable[] = {
  { 0,  0}, // Invalid
}; // LoongArchWriteProcResTable

// {Cycles, WriteResourceID}
extern const llvm::MCWriteLatencyEntry LoongArchWriteLatencyTable[] = {
  { 0,  0}, // Invalid
}; // LoongArchWriteLatencyTable

// {UseIdx, WriteResourceID, Cycles}
extern const llvm::MCReadAdvanceEntry LoongArchReadAdvanceTable[] = {
  {0,  0,  0}, // Invalid
}; // LoongArchReadAdvanceTable

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

// Sorted (by key) array of values for CPU subtype.
extern const llvm::SubtargetSubTypeKV LoongArchSubTypeKV[] = {
 { "generic", { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
 { "generic-la32", { { { 0x1ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
 { "generic-la64", { { { 0x2ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
 { "la464", { { { 0xb2ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, { { { 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, } } }, &NoSchedModel },
};

namespace LoongArch_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass,
    const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) {
  // Don't know how to resolve this scheduling class.
  return 0;
}
} // end namespace LoongArch_MC

struct LoongArchGenMCSubtargetInfo : public MCSubtargetInfo {
  LoongArchGenMCSubtargetInfo(const Triple &TT,
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
    return LoongArch_MC::resolveVariantSchedClassImpl(SchedClass, MI, MCII, CPUID);
  }
  unsigned getHwMode() const override;
};
unsigned LoongArchGenMCSubtargetInfo::getHwMode() const {
  if (checkFeatures("+64bit")) return 1;
  return 0;
}

static inline MCSubtargetInfo *createLoongArchMCSubtargetInfoImpl(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS) {
  return new LoongArchGenMCSubtargetInfo(TT, CPU, TuneCPU, FS, LoongArchFeatureKV, LoongArchSubTypeKV, 
                      LoongArchWriteProcResTable, LoongArchWriteLatencyTable, LoongArchReadAdvanceTable, 
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
void llvm::LoongArchSubtarget::ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS) {
  LLVM_DEBUG(dbgs() << "\nFeatures:" << FS);
  LLVM_DEBUG(dbgs() << "\nCPU:" << CPU);
  LLVM_DEBUG(dbgs() << "\nTuneCPU:" << TuneCPU << "\n\n");
  InitMCProcessorInfo(CPU, TuneCPU, FS);
  const FeatureBitset &Bits = getFeatureBits();
  if (Bits[LoongArch::Feature32Bit]) HasLA32 = true;
  if (Bits[LoongArch::Feature64Bit]) HasLA64 = true;
  if (Bits[LoongArch::FeatureBasicD]) HasBasicD = true;
  if (Bits[LoongArch::FeatureBasicF]) HasBasicF = true;
  if (Bits[LoongArch::FeatureExtLASX]) HasExtLASX = true;
  if (Bits[LoongArch::FeatureExtLBT]) HasExtLBT = true;
  if (Bits[LoongArch::FeatureExtLSX]) HasExtLSX = true;
  if (Bits[LoongArch::FeatureExtLVZ]) HasExtLVZ = true;
  if (Bits[LoongArch::LaGlobalWithAbs]) HasLaGlobalWithAbs = true;
  if (Bits[LoongArch::LaGlobalWithPcrel]) HasLaGlobalWithPcrel = true;
  if (Bits[LoongArch::LaLocalWithAbs]) HasLaLocalWithAbs = true;
}
#endif // GET_SUBTARGETINFO_TARGET_DESC


#ifdef GET_SUBTARGETINFO_HEADER
#undef GET_SUBTARGETINFO_HEADER

namespace llvm {
class DFAPacketizer;
namespace LoongArch_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID);
} // end namespace LoongArch_MC

struct LoongArchGenSubtargetInfo : public TargetSubtargetInfo {
  explicit LoongArchGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS);
public:
  unsigned resolveSchedClass(unsigned SchedClass,  const MachineInstr *DefMI, const TargetSchedModel *SchedModel) const override;
  unsigned resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) const override;
  DFAPacketizer *createDFAPacketizer(const InstrItineraryData *IID) const;
  unsigned getHwMode() const override;
};
} // end namespace llvm

#endif // GET_SUBTARGETINFO_HEADER


#ifdef GET_SUBTARGETINFO_CTOR
#undef GET_SUBTARGETINFO_CTOR

#include "llvm/CodeGen/TargetSchedule.h"

namespace llvm {
extern const llvm::SubtargetFeatureKV LoongArchFeatureKV[];
extern const llvm::SubtargetSubTypeKV LoongArchSubTypeKV[];
extern const llvm::MCWriteProcResEntry LoongArchWriteProcResTable[];
extern const llvm::MCWriteLatencyEntry LoongArchWriteLatencyTable[];
extern const llvm::MCReadAdvanceEntry LoongArchReadAdvanceTable[];
LoongArchGenSubtargetInfo::LoongArchGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS)
  : TargetSubtargetInfo(TT, CPU, TuneCPU, FS, ArrayRef(LoongArchFeatureKV, 11), ArrayRef(LoongArchSubTypeKV, 4), 
                        LoongArchWriteProcResTable, LoongArchWriteLatencyTable, LoongArchReadAdvanceTable, 
                        nullptr, nullptr, nullptr) {}

unsigned LoongArchGenSubtargetInfo
::resolveSchedClass(unsigned SchedClass, const MachineInstr *MI, const TargetSchedModel *SchedModel) const {
  report_fatal_error("Expected a variant SchedClass");
} // LoongArchGenSubtargetInfo::resolveSchedClass

unsigned LoongArchGenSubtargetInfo
::resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, const MCInstrInfo *MCII, unsigned CPUID) const {
  return LoongArch_MC::resolveVariantSchedClassImpl(SchedClass, MI, MCII, CPUID);
} // LoongArchGenSubtargetInfo::resolveVariantSchedClass

unsigned LoongArchGenSubtargetInfo::getHwMode() const {
  if (checkFeatures("+64bit")) return 1;
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

