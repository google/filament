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
namespace PPC {
enum {
  DeprecatedDST = 0,
  Directive32 = 1,
  Directive64 = 2,
  Directive440 = 3,
  Directive601 = 4,
  Directive602 = 5,
  Directive603 = 6,
  Directive604 = 7,
  Directive620 = 8,
  Directive750 = 9,
  Directive970 = 10,
  Directive7400 = 11,
  DirectiveA2 = 12,
  DirectiveE500 = 13,
  DirectiveE500mc = 14,
  DirectiveE5500 = 15,
  DirectivePwr3 = 16,
  DirectivePwr4 = 17,
  DirectivePwr5 = 18,
  DirectivePwr5x = 19,
  DirectivePwr6 = 20,
  DirectivePwr6x = 21,
  DirectivePwr7 = 22,
  DirectivePwr8 = 23,
  DirectivePwr9 = 24,
  DirectivePwrFuture = 25,
  Feature64Bit = 26,
  Feature64BitRegs = 27,
  FeatureAltivec = 28,
  FeatureBPERMD = 29,
  FeatureBookE = 30,
  FeatureCMPB = 31,
  FeatureCRBits = 32,
  FeatureDirectMove = 33,
  FeatureE500 = 34,
  FeatureExtDiv = 35,
  FeatureFCPSGN = 36,
  FeatureFPCVT = 37,
  FeatureFPRND = 38,
  FeatureFPU = 39,
  FeatureFRE = 40,
  FeatureFRES = 41,
  FeatureFRSQRTE = 42,
  FeatureFRSQRTES = 43,
  FeatureFSqrt = 44,
  FeatureFloat128 = 45,
  FeatureHTM = 46,
  FeatureHardFloat = 47,
  FeatureICBT = 48,
  FeatureISA3_0 = 49,
  FeatureISEL = 50,
  FeatureInvariantFunctionDescriptors = 51,
  FeatureLDBRX = 52,
  FeatureLFIWAX = 53,
  FeatureLongCall = 54,
  FeatureMFOCRF = 55,
  FeatureMFTB = 56,
  FeatureMSYNC = 57,
  FeatureP8Altivec = 58,
  FeatureP8Crypto = 59,
  FeatureP8Vector = 60,
  FeatureP9Altivec = 61,
  FeatureP9Vector = 62,
  FeaturePOPCNTD = 63,
  FeaturePPC4xx = 64,
  FeaturePPC6xx = 65,
  FeaturePPCPostRASched = 66,
  FeaturePPCPreRASched = 67,
  FeaturePartwordAtomic = 68,
  FeatureQPX = 69,
  FeatureRecipPrec = 70,
  FeatureSPE = 71,
  FeatureSTFIWX = 72,
  FeatureSecurePlt = 73,
  FeatureSlowPOPCNTD = 74,
  FeatureTwoConstNR = 75,
  FeatureUnalignedFloats = 76,
  FeatureVSX = 77,
  FeatureVectorsUseTwoUnits = 78,
  NumSubtargetFeatures = 79
};
} // end namespace PPC
} // end namespace llvm

#endif // GET_SUBTARGETINFO_ENUM


#ifdef GET_SUBTARGETINFO_MC_DESC
#undef GET_SUBTARGETINFO_MC_DESC

namespace llvm {
// Sorted (by key) array of values for CPU features.
extern const llvm::SubtargetFeatureKV PPCFeatureKV[] = {
  { "64bit", "Enable 64-bit instructions", PPC::Feature64Bit, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "64bitregs", "Enable 64-bit registers usage for ppc32 [beta]", PPC::Feature64BitRegs, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "allow-unaligned-fp-access", "CPU does not trap on unaligned FP access", PPC::FeatureUnalignedFloats, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "altivec", "Enable Altivec instructions", PPC::FeatureAltivec, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "booke", "Enable Book E instructions", PPC::FeatureBookE, { { { 0x1000000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "bpermd", "Enable the bpermd instruction", PPC::FeatureBPERMD, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "cmpb", "Enable the cmpb instruction", PPC::FeatureCMPB, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "crbits", "Use condition-register bits individually", PPC::FeatureCRBits, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "crypto", "Enable POWER8 Crypto instructions", PPC::FeatureP8Crypto, { { { 0x400000000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "direct-move", "Enable Power8 direct move instructions", PPC::FeatureDirectMove, { { { 0x0ULL, 0x2000ULL, 0x0ULL, } } } },
  { "e500", "Enable E500/E500mc instructions", PPC::FeatureE500, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "extdiv", "Enable extended divide instructions", PPC::FeatureExtDiv, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "fcpsgn", "Enable the fcpsgn instruction", PPC::FeatureFCPSGN, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "float128", "Enable the __float128 data type for IEEE-754R Binary128.", PPC::FeatureFloat128, { { { 0x0ULL, 0x2000ULL, 0x0ULL, } } } },
  { "fpcvt", "Enable fc[ft]* (unsigned and single-precision) and lfiwzx instructions", PPC::FeatureFPCVT, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "fprnd", "Enable the fri[mnpz] instructions", PPC::FeatureFPRND, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "fpu", "Enable classic FPU instructions", PPC::FeatureFPU, { { { 0x800000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "fre", "Enable the fre instruction", PPC::FeatureFRE, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "fres", "Enable the fres instruction", PPC::FeatureFRES, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "frsqrte", "Enable the frsqrte instruction", PPC::FeatureFRSQRTE, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "frsqrtes", "Enable the frsqrtes instruction", PPC::FeatureFRSQRTES, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "fsqrt", "Enable the fsqrt instruction", PPC::FeatureFSqrt, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "hard-float", "Enable floating-point instructions", PPC::FeatureHardFloat, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "htm", "Enable Hardware Transactional Memory instructions", PPC::FeatureHTM, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "icbt", "Enable icbt instruction", PPC::FeatureICBT, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "invariant-function-descriptors", "Assume function descriptors are invariant", PPC::FeatureInvariantFunctionDescriptors, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "isa-v30-instructions", "Enable instructions added in ISA 3.0.", PPC::FeatureISA3_0, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "isel", "Enable the isel instruction", PPC::FeatureISEL, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ldbrx", "Enable the ldbrx instruction", PPC::FeatureLDBRX, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "lfiwax", "Enable the lfiwax instruction", PPC::FeatureLFIWAX, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "longcall", "Always use indirect calls", PPC::FeatureLongCall, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "mfocrf", "Enable the MFOCRF instruction", PPC::FeatureMFOCRF, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "msync", "Has only the msync instruction instead of sync", PPC::FeatureMSYNC, { { { 0x40000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "partword-atomics", "Enable l[bh]arx and st[bh]cx.", PPC::FeaturePartwordAtomic, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "popcntd", "Enable the popcnt[dw] instructions", PPC::FeaturePOPCNTD, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "power8-altivec", "Enable POWER8 Altivec instructions", PPC::FeatureP8Altivec, { { { 0x10000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "power8-vector", "Enable POWER8 vector instructions", PPC::FeatureP8Vector, { { { 0x400000000000000ULL, 0x2000ULL, 0x0ULL, } } } },
  { "power9-altivec", "Enable POWER9 Altivec instructions", PPC::FeatureP9Altivec, { { { 0x402000000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "power9-vector", "Enable POWER9 vector instructions", PPC::FeatureP9Vector, { { { 0x3002000000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "ppc-postra-sched", "Use PowerPC post-RA scheduling strategy", PPC::FeaturePPCPostRASched, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ppc-prera-sched", "Use PowerPC pre-RA scheduling strategy", PPC::FeaturePPCPreRASched, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ppc4xx", "Enable PPC 4xx instructions", PPC::FeaturePPC4xx, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "ppc6xx", "Enable PPC 6xx instructions", PPC::FeaturePPC6xx, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "qpx", "Enable QPX instructions", PPC::FeatureQPX, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "recipprec", "Assume higher precision reciprocal estimates", PPC::FeatureRecipPrec, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "secure-plt", "Enable secure plt mode", PPC::FeatureSecurePlt, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "slow-popcntd", "Has slow popcnt[dw] instructions", PPC::FeatureSlowPOPCNTD, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "spe", "Enable SPE instructions", PPC::FeatureSPE, { { { 0x800000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "stfiwx", "Enable the stfiwx instruction", PPC::FeatureSTFIWX, { { { 0x8000000000ULL, 0x0ULL, 0x0ULL, } } } },
  { "two-const-nr", "Requires two constant Newton-Raphson computation", PPC::FeatureTwoConstNR, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "vectors-use-two-units", "Vectors use two units", PPC::FeatureVectorsUseTwoUnits, { { { 0x0ULL, 0x0ULL, 0x0ULL, } } } },
  { "vsx", "Enable VSX instructions", PPC::FeatureVSX, { { { 0x10000000ULL, 0x0ULL, 0x0ULL, } } } },
};

#ifdef DBGFIELD
#error "<target>GenSubtargetInfo.inc requires a DBGFIELD macro"
#endif
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#define DBGFIELD(x) x,
#else
#define DBGFIELD(x)
#endif

// Functional units for "PPC440Itineraries"
namespace PPC440ItinerariesFU {
  const unsigned P440_DISS1 = 1 << 0;
  const unsigned P440_DISS2 = 1 << 1;
  const unsigned P440_FRACC = 1 << 2;
  const unsigned P440_IRACC = 1 << 3;
  const unsigned P440_IEXE1 = 1 << 4;
  const unsigned P440_IEXE2 = 1 << 5;
  const unsigned P440_IWB = 1 << 6;
  const unsigned P440_LRACC = 1 << 7;
  const unsigned P440_JEXE1 = 1 << 8;
  const unsigned P440_JEXE2 = 1 << 9;
  const unsigned P440_JWB = 1 << 10;
  const unsigned P440_AGEN = 1 << 11;
  const unsigned P440_CRD = 1 << 12;
  const unsigned P440_LWB = 1 << 13;
  const unsigned P440_FEXE1 = 1 << 14;
  const unsigned P440_FEXE2 = 1 << 15;
  const unsigned P440_FEXE3 = 1 << 16;
  const unsigned P440_FEXE4 = 1 << 17;
  const unsigned P440_FEXE5 = 1 << 18;
  const unsigned P440_FEXE6 = 1 << 19;
  const unsigned P440_FWB = 1 << 20;
  const unsigned P440_LWARX_Hold = 1 << 21;
} // end namespace PPC440ItinerariesFU

// Pipeline forwarding paths for itineraries "PPC440Itineraries"
namespace PPC440ItinerariesBypass {
  const unsigned NoBypass = 0;
  const unsigned P440_GPR_Bypass = 1 << 0;
  const unsigned P440_FPR_Bypass = 1 << 1;
} // end namespace PPC440ItinerariesBypass

// Functional units for "G3Itineraries"
namespace G3ItinerariesFU {
  const unsigned G3_IU1 = 1 << 0;
  const unsigned G3_IU2 = 1 << 1;
  const unsigned G3_FPU1 = 1 << 2;
  const unsigned G3_BPU = 1 << 3;
  const unsigned G3_SRU = 1 << 4;
  const unsigned G3_SLU = 1 << 5;
} // end namespace G3ItinerariesFU

// Functional units for "G4Itineraries"
namespace G4ItinerariesFU {
  const unsigned G4_IU1 = 1 << 0;
  const unsigned G4_IU2 = 1 << 1;
  const unsigned G4_SLU = 1 << 2;
  const unsigned G4_SRU = 1 << 3;
  const unsigned G4_BPU = 1 << 4;
  const unsigned G4_FPU1 = 1 << 5;
  const unsigned G4_VIU1 = 1 << 6;
  const unsigned G4_VIU2 = 1 << 7;
  const unsigned G4_VPU = 1 << 8;
  const unsigned G4_VFPU = 1 << 9;
} // end namespace G4ItinerariesFU

// Functional units for "G4PlusItineraries"
namespace G4PlusItinerariesFU {
  const unsigned G4P_IU1 = 1 << 0;
  const unsigned G4P_IU2 = 1 << 1;
  const unsigned G4P_IU3 = 1 << 2;
  const unsigned G4P_IU4 = 1 << 3;
  const unsigned G4P_BPU = 1 << 4;
  const unsigned G4P_SLU = 1 << 5;
  const unsigned G4P_FPU1 = 1 << 6;
  const unsigned G4P_VFPU = 1 << 7;
  const unsigned G4P_VIU1 = 1 << 8;
  const unsigned G4P_VIU2 = 1 << 9;
  const unsigned G4P_VPU = 1 << 10;
} // end namespace G4PlusItinerariesFU

// Functional units for "G5Itineraries"
namespace G5ItinerariesFU {
  const unsigned G5_IU1 = 1 << 0;
  const unsigned G5_IU2 = 1 << 1;
  const unsigned G5_SLU = 1 << 2;
  const unsigned G5_BPU = 1 << 3;
  const unsigned G5_FPU1 = 1 << 4;
  const unsigned G5_FPU2 = 1 << 5;
  const unsigned G5_VFPU = 1 << 6;
  const unsigned G5_VIU1 = 1 << 7;
  const unsigned G5_VIU2 = 1 << 8;
  const unsigned G5_VPU = 1 << 9;
} // end namespace G5ItinerariesFU

// Functional units for "PPCA2Itineraries"
namespace PPCA2ItinerariesFU {
  const unsigned A2_XU = 1 << 0;
  const unsigned A2_FU = 1 << 1;
} // end namespace PPCA2ItinerariesFU

// Functional units for "PPCE500Itineraries"
namespace PPCE500ItinerariesFU {
  const unsigned E500_DIS0 = 1 << 0;
  const unsigned E500_DIS1 = 1 << 1;
  const unsigned E500_SU0 = 1 << 2;
  const unsigned E500_SU1 = 1 << 3;
  const unsigned E500_BU = 1 << 4;
  const unsigned E500_MU = 1 << 5;
  const unsigned E500_LSU_0 = 1 << 6;
} // end namespace PPCE500ItinerariesFU

// Pipeline forwarding paths for itineraries "PPCE500Itineraries"
namespace PPCE500ItinerariesBypass {
  const unsigned NoBypass = 0;
  const unsigned E500_CR_Bypass = 1 << 0;
  const unsigned E500_GPR_Bypass = 1 << 1;
  const unsigned E500_DivBypass = 1 << 2;
} // end namespace PPCE500ItinerariesBypass

// Functional units for "PPCE500mcItineraries"
namespace PPCE500mcItinerariesFU {
  const unsigned E500mc_DIS0 = 1 << 0;
  const unsigned E500mc_DIS1 = 1 << 1;
  const unsigned E500mc_SFX0 = 1 << 2;
  const unsigned E500mc_SFX1 = 1 << 3;
  const unsigned E500mc_BU = 1 << 4;
  const unsigned E500mc_CFX_DivBypass = 1 << 5;
  const unsigned E500mc_CFX_0 = 1 << 6;
  const unsigned E500mc_LSU_0 = 1 << 7;
  const unsigned E500mc_FPU_0 = 1 << 8;
} // end namespace PPCE500mcItinerariesFU

// Pipeline forwarding paths for itineraries "PPCE500mcItineraries"
namespace PPCE500mcItinerariesBypass {
  const unsigned NoBypass = 0;
  const unsigned E500mc_CR_Bypass = 1 << 0;
  const unsigned E500mc_GPR_Bypass = 1 << 1;
  const unsigned E500mc_FPR_Bypass = 1 << 2;
} // end namespace PPCE500mcItinerariesBypass

// Functional units for "PPCE5500Itineraries"
namespace PPCE5500ItinerariesFU {
  const unsigned E5500_DIS0 = 1 << 0;
  const unsigned E5500_DIS1 = 1 << 1;
  const unsigned E5500_SFX0 = 1 << 2;
  const unsigned E5500_SFX1 = 1 << 3;
  const unsigned E5500_BU = 1 << 4;
  const unsigned E5500_CFX_DivBypass = 1 << 5;
  const unsigned E5500_CFX_0 = 1 << 6;
  const unsigned E5500_CFX_1 = 1 << 7;
  const unsigned E5500_LSU_0 = 1 << 8;
  const unsigned E5500_FPU_0 = 1 << 9;
} // end namespace PPCE5500ItinerariesFU

// Pipeline forwarding paths for itineraries "PPCE5500Itineraries"
namespace PPCE5500ItinerariesBypass {
  const unsigned NoBypass = 0;
  const unsigned E5500_CR_Bypass = 1 << 0;
  const unsigned E5500_GPR_Bypass = 1 << 1;
  const unsigned E5500_FPR_Bypass = 1 << 2;
} // end namespace PPCE5500ItinerariesBypass

// Functional units for "P8Itineraries"
namespace P8ItinerariesFU {
  const unsigned P8_DU1 = 1 << 0;
  const unsigned P8_DU2 = 1 << 1;
  const unsigned P8_DU3 = 1 << 2;
  const unsigned P8_DU4 = 1 << 3;
  const unsigned P8_DU5 = 1 << 4;
  const unsigned P8_DU6 = 1 << 5;
  const unsigned P8_DU7 = 1 << 6;
  const unsigned P8_DU8 = 1 << 7;
  const unsigned P8_LU1 = 1 << 8;
  const unsigned P8_LU2 = 1 << 9;
  const unsigned P8_LSU1 = 1 << 10;
  const unsigned P8_LSU2 = 1 << 11;
  const unsigned P8_FXU1 = 1 << 12;
  const unsigned P8_FXU2 = 1 << 13;
  const unsigned P8_FPU1 = 1 << 14;
  const unsigned P8_FPU2 = 1 << 15;
  const unsigned P8_CRU = 1 << 16;
  const unsigned P8_BRU = 1 << 17;
} // end namespace P8ItinerariesFU

// Functional units for "P7Itineraries"
namespace P7ItinerariesFU {
  const unsigned P7_DU1 = 1 << 0;
  const unsigned P7_DU2 = 1 << 1;
  const unsigned P7_DU3 = 1 << 2;
  const unsigned P7_DU4 = 1 << 3;
  const unsigned P7_DU5 = 1 << 4;
  const unsigned P7_DU6 = 1 << 5;
  const unsigned P7_LS1 = 1 << 6;
  const unsigned P7_LS2 = 1 << 7;
  const unsigned P7_FX1 = 1 << 8;
  const unsigned P7_FX2 = 1 << 9;
  const unsigned P7_VS1 = 1 << 10;
  const unsigned P7_VS2 = 1 << 11;
  const unsigned P7_CRU = 1 << 12;
  const unsigned P7_BRU = 1 << 13;
} // end namespace P7ItinerariesFU

extern const llvm::InstrStage PPCStages[] = {
  { 0, 0, 0, llvm::InstrStage::Required }, // No itinerary
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 3, PPC440ItinerariesFU::P440_AGEN, 1, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPC440ItinerariesFU::P440_CRD, 1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 1-5
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC | PPC440ItinerariesFU::P440_LRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE1 | PPC440ItinerariesFU::P440_JEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE2 | PPC440ItinerariesFU::P440_JEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IWB | PPC440ItinerariesFU::P440_JWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 6-10
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 11-15
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_AGEN, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_CRD, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 16-20
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_AGEN, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_CRD, -1, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPC440ItinerariesFU::P440_LWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 21-25
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 33, PPC440ItinerariesFU::P440_IWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 26-30
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE3, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE4, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE5, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE6, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 31-39
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE3, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE4, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE5, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE6, -1, (llvm::InstrStage::ReservationKinds)0 },   { 25, PPC440ItinerariesFU::P440_FWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 40-48
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE3, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE4, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE5, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE6, -1, (llvm::InstrStage::ReservationKinds)0 },   { 13, PPC440ItinerariesFU::P440_FWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 49-57
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FRACC, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LRACC, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_AGEN, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_JEXE1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_FEXE2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_CRD, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_JEXE2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 6, PPC440ItinerariesFU::P440_FEXE3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 6, PPC440ItinerariesFU::P440_LWB, 0, (llvm::InstrStage::ReservationKinds)0 },   { 6, PPC440ItinerariesFU::P440_JWB, 0, (llvm::InstrStage::ReservationKinds)0 },   { 6, PPC440ItinerariesFU::P440_IWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 58-73
  { 1, PPC440ItinerariesFU::P440_DISS1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPC440ItinerariesFU::P440_LWARX_Hold, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_AGEN, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_CRD, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_LWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 74-80
  { 1, PPC440ItinerariesFU::P440_DISS1 | PPC440ItinerariesFU::P440_DISS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IRACC, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE1, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPC440ItinerariesFU::P440_IEXE2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 3, PPC440ItinerariesFU::P440_IWB, -1, (llvm::InstrStage::ReservationKinds)0 }, // 81-85
  { 3, G3ItinerariesFU::G3_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 86
  { 1, G3ItinerariesFU::G3_IU1 | G3ItinerariesFU::G3_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 87
  { 1, G3ItinerariesFU::G3_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 88
  { 1, G3ItinerariesFU::G3_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 89
  { 2, G3ItinerariesFU::G3_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 90
  { 19, G3ItinerariesFU::G3_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 91
  { 1, G3ItinerariesFU::G3_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 92
  { 31, G3ItinerariesFU::G3_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 93
  { 17, G3ItinerariesFU::G3_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 94
  { 2, G3ItinerariesFU::G3_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 95
  { 2, G3ItinerariesFU::G3_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 96
  { 34, G3ItinerariesFU::G3_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 97
  { 3, G3ItinerariesFU::G3_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 98
  { 3, G3ItinerariesFU::G3_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 99
  { 5, G3ItinerariesFU::G3_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 100
  { 6, G3ItinerariesFU::G3_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 101
  { 3, G3ItinerariesFU::G3_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 102
  { 8, G3ItinerariesFU::G3_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 103
  { 2, G3ItinerariesFU::G3_IU1 | G3ItinerariesFU::G3_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 104
  { 8, G4ItinerariesFU::G4_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 105
  { 1, G4ItinerariesFU::G4_IU1 | G4ItinerariesFU::G4_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 106
  { 1, G4ItinerariesFU::G4_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 107
  { 4, G4ItinerariesFU::G4_VFPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 108
  { 1, G4ItinerariesFU::G4_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 109
  { 2, G4ItinerariesFU::G4_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 110
  { 19, G4ItinerariesFU::G4_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 111
  { 1, G4ItinerariesFU::G4_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 112
  { 31, G4ItinerariesFU::G4_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 113
  { 1, G4ItinerariesFU::G4_VIU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 114
  { 3, G4ItinerariesFU::G4_VIU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 115
  { 17, G4ItinerariesFU::G4_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 116
  { 2, G4ItinerariesFU::G4_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 117
  { 3, G4ItinerariesFU::G4_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 118
  { 34, G4ItinerariesFU::G4_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 119
  { 3, G4ItinerariesFU::G4_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 120
  { 3, G4ItinerariesFU::G4_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 121
  { 5, G4ItinerariesFU::G4_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 122
  { 6, G4ItinerariesFU::G4_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 123
  { 3, G4ItinerariesFU::G4_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 124
  { 1, G4ItinerariesFU::G4_VPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 125
  { 5, G4ItinerariesFU::G4_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 126
  { 8, G4ItinerariesFU::G4_SRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 127
  { 2, G4ItinerariesFU::G4_IU1 | G4ItinerariesFU::G4_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 128
  { 35, G4PlusItinerariesFU::G4P_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 129
  { 1, G4PlusItinerariesFU::G4P_IU1 | G4PlusItinerariesFU::G4P_IU2 | G4PlusItinerariesFU::G4P_IU3 | G4PlusItinerariesFU::G4P_IU4, -1, (llvm::InstrStage::ReservationKinds)0 }, // 130
  { 1, G4PlusItinerariesFU::G4P_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 131
  { 4, G4PlusItinerariesFU::G4P_VFPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 132
  { 2, G4PlusItinerariesFU::G4P_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 133
  { 3, G4PlusItinerariesFU::G4P_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 134
  { 23, G4PlusItinerariesFU::G4P_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 135
  { 5, G4PlusItinerariesFU::G4P_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 136
  { 35, G4PlusItinerariesFU::G4P_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 137
  { 1, G4PlusItinerariesFU::G4P_VIU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 138
  { 4, G4PlusItinerariesFU::G4P_VIU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 139
  { 21, G4PlusItinerariesFU::G4P_FPU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 140
  { 3, G4PlusItinerariesFU::G4P_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 141
  { 0, G4PlusItinerariesFU::G4P_IU1 | G4PlusItinerariesFU::G4P_IU2 | G4PlusItinerariesFU::G4P_IU3 | G4PlusItinerariesFU::G4P_IU4, -1, (llvm::InstrStage::ReservationKinds)0 }, // 142
  { 4, G4PlusItinerariesFU::G4P_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 143
  { 37, G4PlusItinerariesFU::G4P_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 144
  { 4, G4PlusItinerariesFU::G4P_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 145
  { 5, G4PlusItinerariesFU::G4P_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 146
  { 2, G4PlusItinerariesFU::G4P_VPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 147
  { 2, G4PlusItinerariesFU::G4P_IU1 | G4PlusItinerariesFU::G4P_IU2 | G4PlusItinerariesFU::G4P_IU3 | G4PlusItinerariesFU::G4P_IU4, -1, (llvm::InstrStage::ReservationKinds)0 }, // 148
  { 2, G4PlusItinerariesFU::G4P_VFPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 149
  { 35, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 150
  { 2, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 151
  { 1, G5ItinerariesFU::G5_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 152
  { 8, G5ItinerariesFU::G5_VFPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 153
  { 3, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 154
  { 4, G5ItinerariesFU::G5_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 155
  { 3, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 156
  { 68, G5ItinerariesFU::G5_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 157
  { 36, G5ItinerariesFU::G5_IU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 158
  { 6, G5ItinerariesFU::G5_FPU1 | G5ItinerariesFU::G5_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 159
  { 33, G5ItinerariesFU::G5_FPU1 | G5ItinerariesFU::G5_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 160
  { 8, G5ItinerariesFU::G5_FPU1 | G5ItinerariesFU::G5_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 161
  { 2, G5ItinerariesFU::G5_VIU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 162
  { 5, G5ItinerariesFU::G5_VIU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 163
  { 40, G5ItinerariesFU::G5_FPU1 | G5ItinerariesFU::G5_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 164
  { 40, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 165
  { 11, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 166
  { 5, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 167
  { 64, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 168
  { 7, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 169
  { 2, G5ItinerariesFU::G5_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 170
  { 3, G5ItinerariesFU::G5_BPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 171
  { 2, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 172
  { 3, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 173
  { 6, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 174
  { 10, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 175
  { 8, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 176
  { 5, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 177
  { 4, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 178
  { 3, G5ItinerariesFU::G5_VPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 179
  { 4, G5ItinerariesFU::G5_SLU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 180
  { 1, G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 181
  { 1, G5ItinerariesFU::G5_IU1 | G5ItinerariesFU::G5_IU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 182
  { 2, G5ItinerariesFU::G5_VFPU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 183
  { 1, PPCA2ItinerariesFU::A2_XU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 184
  { 1, PPCA2ItinerariesFU::A2_FU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 185
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 186-187
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_SU0 | PPCE500ItinerariesFU::E500_SU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 188-189
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_BU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 190-191
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_MU, 0, (llvm::InstrStage::ReservationKinds)0 },   { 14, PPCE500ItinerariesFU::E500_MU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 192-194
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 6, PPCE500ItinerariesFU::E500_MU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 195-196
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 32, PPCE500ItinerariesFU::E500_MU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 197-198
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE500ItinerariesFU::E500_MU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 199-200
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_SU0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 201-202
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 29, PPCE500ItinerariesFU::E500_MU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 203-204
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 3, PPCE500ItinerariesFU::E500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 205-206
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_SU0 | PPCE500ItinerariesFU::E500_SU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 207-209
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 5, PPCE500ItinerariesFU::E500_SU0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 210-211
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE500ItinerariesFU::E500_SU0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 212-213
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE500ItinerariesFU::E500_SU0 | PPCE500ItinerariesFU::E500_SU1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 214-215
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500ItinerariesFU::E500_LSU_0, 0, (llvm::InstrStage::ReservationKinds)0 }, // 216-217
  { 1, PPCE500ItinerariesFU::E500_DIS0 | PPCE500ItinerariesFU::E500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE500ItinerariesFU::E500_SU0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 218-219
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 220-221
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_SFX0 | PPCE500mcItinerariesFU::E500mc_SFX1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 222-223
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_BU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 224-225
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_CFX_0, 0, (llvm::InstrStage::ReservationKinds)0 },   { 14, PPCE500mcItinerariesFU::E500mc_CFX_DivBypass, -1, (llvm::InstrStage::ReservationKinds)0 }, // 226-228
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE500mcItinerariesFU::E500mc_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 229-230
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 68, PPCE500mcItinerariesFU::E500mc_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 231-232
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE500mcItinerariesFU::E500mc_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 233-234
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 38, PPCE500mcItinerariesFU::E500mc_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 235-236
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 3, PPCE500mcItinerariesFU::E500mc_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 237-238
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_SFX0 | PPCE500mcItinerariesFU::E500mc_SFX1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 239-241
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 5, PPCE500mcItinerariesFU::E500mc_SFX0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 242-243
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 8, PPCE500mcItinerariesFU::E500mc_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 244-245
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE500mcItinerariesFU::E500mc_SFX0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 246-247
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE500mcItinerariesFU::E500mc_SFX0 | PPCE500mcItinerariesFU::E500mc_SFX1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 248-249
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_SFX0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 250-251
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_CFX_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 252-253
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE500mcItinerariesFU::E500mc_LSU_0, 0, (llvm::InstrStage::ReservationKinds)0 }, // 254-255
  { 1, PPCE500mcItinerariesFU::E500mc_DIS0 | PPCE500mcItinerariesFU::E500mc_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE500mcItinerariesFU::E500mc_SFX0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 256-257
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 258-259
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_SFX0 | PPCE5500ItinerariesFU::E5500_SFX1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 260-261
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_BU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 262-263
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_0, 0, (llvm::InstrStage::ReservationKinds)0 },   { 26, PPCE5500ItinerariesFU::E5500_CFX_DivBypass, -1, (llvm::InstrStage::ReservationKinds)0 }, // 264-266
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_0, 0, (llvm::InstrStage::ReservationKinds)0 },   { 16, PPCE5500ItinerariesFU::E5500_CFX_DivBypass, -1, (llvm::InstrStage::ReservationKinds)0 }, // 267-269
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 270-271
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 31, PPCE5500ItinerariesFU::E5500_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 272-273
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 16, PPCE5500ItinerariesFU::E5500_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 274-275
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 3, PPCE5500ItinerariesFU::E5500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 276-277
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_SFX0 | PPCE5500ItinerariesFU::E5500_SFX1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 278-280
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE5500ItinerariesFU::E5500_LSU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 281-282
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_0, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE5500ItinerariesFU::E5500_CFX_1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 283-285
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 286-287
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 5, PPCE5500ItinerariesFU::E5500_CFX_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 288-289
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 4, PPCE5500ItinerariesFU::E5500_CFX_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 290-291
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 7, PPCE5500ItinerariesFU::E5500_FPU_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 292-293
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE5500ItinerariesFU::E5500_CFX_0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 294-295
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_0, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_CFX_1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 296-298
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE5500ItinerariesFU::E5500_SFX0 | PPCE5500ItinerariesFU::E5500_SFX1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 299-300
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, PPCE5500ItinerariesFU::E5500_LSU_0, 0, (llvm::InstrStage::ReservationKinds)0 }, // 301-302
  { 1, PPCE5500ItinerariesFU::E5500_DIS0 | PPCE5500ItinerariesFU::E5500_DIS1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 2, PPCE5500ItinerariesFU::E5500_SFX0, -1, (llvm::InstrStage::ReservationKinds)0 }, // 303-304
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2 | P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 305-306
  { 1, P8ItinerariesFU::P8_DU7 | P8ItinerariesFU::P8_DU8, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_BRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 307-308
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FPU1 | P8ItinerariesFU::P8_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 309-310
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 311-312
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_CRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 313-314
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 315-316
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 23, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 317-318
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 15, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 319-320
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FPU1 | P8ItinerariesFU::P8_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 321-322
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 323-325
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_BRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 326-328
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 329-333
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 334-337
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU5, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 338-344
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 345-346
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 347-350
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 351-354
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 355-359
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU5, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 360-368
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2 | P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 369-372
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_CRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 373-374
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 375-376
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FPU1 | P8ItinerariesFU::P8_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 377-378
  { 1, P8ItinerariesFU::P8_DU1 | P8ItinerariesFU::P8_DU2 | P8ItinerariesFU::P8_DU3 | P8ItinerariesFU::P8_DU4 | P8ItinerariesFU::P8_DU5 | P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FPU1 | P8ItinerariesFU::P8_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 379-381
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FPU1 | P8ItinerariesFU::P8_FPU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 382-386
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU5, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 387-394
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2 | P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 395-398
  { 1, P8ItinerariesFU::P8_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU5, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_FXU1 | P8ItinerariesFU::P8_FXU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 399-407
  { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2 | P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2 | P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P8ItinerariesFU::P8_LU1 | P8ItinerariesFU::P8_LU2 | P8ItinerariesFU::P8_LSU1 | P8ItinerariesFU::P8_LSU2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 408-410
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2 | P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 411-412
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 413-414
  { 1, P7ItinerariesFU::P7_DU5 | P7ItinerariesFU::P7_DU6, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_BRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 415-416
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS1 | P7ItinerariesFU::P7_VS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 417-418
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_CRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 419-420
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 421-422
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 68, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 423-425
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 36, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 426-428
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS1 | P7ItinerariesFU::P7_VS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 429-430
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 431-432
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 433-435
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_BRU, -1, (llvm::InstrStage::ReservationKinds)0 }, // 436-438
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 439-443
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 444-447
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 448-454
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 455-458
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 459-463
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 464-471
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_CRU, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 472-477
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1, -1, (llvm::InstrStage::ReservationKinds)0 }, // 478-479
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 480-481
  { 1, P7ItinerariesFU::P7_DU1 | P7ItinerariesFU::P7_DU2 | P7ItinerariesFU::P7_DU3 | P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS1 | P7ItinerariesFU::P7_VS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 482-484
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_VS1 | P7ItinerariesFU::P7_VS2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 485-489
  { 1, P7ItinerariesFU::P7_DU1, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU3, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_DU4, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_LS1 | P7ItinerariesFU::P7_LS2, 0, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 },   { 1, P7ItinerariesFU::P7_FX1 | P7ItinerariesFU::P7_FX2, -1, (llvm::InstrStage::ReservationKinds)0 }, // 490-496
  { 0, 0, 0, llvm::InstrStage::Required } // End stages
};
extern const unsigned PPCOperandCycles[] = {
  0, // No itinerary
  2,   0,   0, // 1-3
  2,   0,   0, // 4-6
  4,   0, // 7-8
  2,   0,   0, // 9-11
  4,   0,   0, // 12-14
  1,   1, // 15-16
  5,   1,   1, // 17-19
  36,   0,   0, // 20-22
  6,   0,   0, // 23-25
  31,   0,   0, // 26-28
  6,   0,   0, // 29-31
  6,   0,   0, // 32-34
  1,   1,   1, // 35-37
  19,   0,   0, // 38-40
  6,   0,   0,   0, // 41-44
  4,   1,   1, // 45-47
  2,   0,   0,   0, // 48-51
  4,   1,   1, // 52-54
  5,   2,   1,   1, // 55-58
  5,   2,   1,   1, // 59-62
  5,   1,   1, // 63-65
  5,   2,   1,   1, // 66-69
  5,   2,   1,   1, // 70-73
  4,   1,   1, // 74-76
  4,   1,   1, // 77-79
  4,   1,   1, // 80-82
  4,   1,   1, // 83-85
  4,   0,   0, // 86-88
  4,   0,   0, // 89-91
  4,   0, // 92-93
  6,   0, // 94-95
  3,   0,   0, // 96-98
  3,   0, // 99-100
  2,   0, // 101-102
  6,   0, // 103-104
  6,   0, // 105-106
  3,   0,   0, // 107-109
  2,   0, // 110-111
  5,   0, // 112-113
  4,   0,   0, // 114-116
  4,   0,   0, // 117-119
  4,   0,   0, // 120-122
  1,   1,   1, // 123-125
  2,   1,   1,   1, // 126-129
  4,   0, // 130-131
  2,   0,   0, // 132-134
  2,   0,   0, // 135-137
  4,   1,   1, // 138-140
  2,   1,   1,   1, // 141-144
  2,   1,   1,   1, // 145-148
  4,   1,   1, // 149-151
  4,   1,   1, // 152-154
  2,   0, // 155-156
  6,   0, // 157-158
  3,   0,   0, // 159-161
  3,   0,   0, // 162-164
  2,   0,   0, // 165-167
  2,   0, // 168-169
  6,   0,   0, // 170-172
  6,   0,   0, // 173-175
  2,   0,   0, // 176-178
  2,   0,   0, // 179-181
  2,   0,   0, // 182-184
  2,   0,   0, // 185-187
  4,   0,   0, // 188-190
  4,   0,   0, // 191-193
  4,   0, // 194-195
  6,   0, // 196-197
  5,   1,   1, // 198-200
  2,   0,   0, // 201-203
  4,   0,   0, // 204-206
  4,   0,   0, // 207-209
  2,   0,   0, // 210-212
  6,   0,   0, // 213-215
  2,   0,   0, // 216-218
  2,   0,   0,   0, // 219-222
  6,   0, // 223-224
  1,   1,   1, // 225-227
  5,   1,   1, // 228-230
  6,   0,   0, // 231-233
  4,   0,   0, // 234-236
  4,   0,   0, // 237-239
  4,   0,   0, // 240-242
  6,   0,   0, // 243-245
  6,   0,   0, // 246-248
  6,   0,   0,   0, // 249-252
  6,   0,   0, // 253-255
  4,   0,   0, // 256-258
  4,   0,   0, // 259-261
  6,   0,   0, // 262-264
  6,   0,   0, // 265-267
  6,   0,   0,   0, // 268-271
  5,   1,   1, // 272-274
  6,   0,   0, // 275-277
  5,   1,   1, // 278-280
  5,   1,   1, // 281-283
  5,   1,   1, // 284-286
  1,   1, // 287-288
  4,   1,   1, // 289-291
  5,   1,   1, // 292-294
  4,   1,   1, // 295-297
  4,   1,   1, // 298-300
  5,   1,   1, // 301-303
  5,   2,   1,   1, // 304-307
  5,   2,   1,   1, // 308-311
  6,   0, // 312-313
  4,   1,   1, // 314-316
  4,   1,   1, // 317-319
  4,   1,   1, // 320-322
  6,   0, // 323-324
  5,   1,   1, // 325-327
  5,   1,   1, // 328-330
  5,   1,   1, // 331-333
  5,   1,   1, // 334-336
  4,   1,   1, // 337-339
  4,   1,   1, // 340-342
  5,   1,   1, // 343-345
  1,   1,   1, // 346-348
  1,   1,   1, // 349-351
  4,   1,   1, // 352-354
  5,   1,   1, // 355-357
  4,   1,   1, // 358-360
  1,   1,   1, // 361-363
  1,   1,   1, // 364-366
  5,   1,   1, // 367-369
  6,   0, // 370-371
  2,   0,   0, // 372-374
  6,   0, // 375-376
  6,   0, // 377-378
  3,   0, // 379-380
  2,   0, // 381-382
  6,   0, // 383-384
  36,   0,   0, // 385-387
  36,   0,   0, // 388-390
  36,   0,   0, // 391-393
  36,   0,   0, // 394-396
  4,   0,   0, // 397-399
  2,   0,   0, // 400-402
  6,   0,   0, // 403-405
  4,   0,   0, // 406-408
  3,   0,   0, // 409-411
  3,   0,   0, // 412-414
  2,   0,   0, // 415-417
  2,   0,   0, // 418-420
  2,   0,   0, // 421-423
  3,   0,   0, // 424-426
  4,   0, // 427-428
  31,   0,   0, // 429-431
  31,   0,   0, // 432-434
  31,   0,   0, // 435-437
  19,   0,   0, // 438-440
  19,   0,   0, // 441-443
  19,   0,   0, // 444-446
  19,   0,   0, // 447-449
  31,   0,   0, // 450-452
  5,   2,   1,   1, // 453-456
  5,   2,   1,   1, // 457-460
  2,   1,   1,   1, // 461-464
  2,   1,   1,   1, // 465-468
  2,   1,   1,   1, // 469-472
  5,   2,   1,   1, // 473-476
  5,   2,   1,   1, // 477-480
  5,   2,   1,   1, // 481-484
  5,   2,   1,   1, // 485-488
  4,   0, // 489-490
  4,   0, // 491-492
  5,   1,   1, // 493-495
  1,   1,   1, // 496-498
  2,   0,   0, // 499-501
  5,   1,   1, // 502-504
  1,   1,   1, // 505-507
  4,   0, // 508-509
  4,   0, // 510-511
  5,   1,   1, // 512-514
  1,   1,   1, // 515-517
  5,   1,   1, // 518-520
  1,   1,   1, // 521-523
  2,   0, // 524-525
  5,   0, // 526-527
  6,   0, // 528-529
  6,   0, // 530-531
  2,   0,   0, // 532-534
  2,   0, // 535-536
  1,   1, // 537-538
  6, // 539
  1,   0,   0, // 540-542
  2,   0,   0, // 543-545
  6,   0,   0, // 546-548
  2,   0,   0, // 549-551
  1,   0,   0, // 552-554
  1,   0,   0, // 555-557
  6,   0,   0, // 558-560
  71,   0,   0, // 561-563
  39,   0,   0, // 564-566
  6,   0,   0, // 567-569
  72,   0,   0, // 570-572
  5,   0,   0, // 573-575
  6,   0,   0, // 576-578
  0,   0,   0, // 579-581
  2,   0,   0, // 582-584
  59,   0,   0, // 585-587
  6,   0,   0,   0, // 588-591
  69,   0,   0, // 592-594
  65,   0,   0, // 595-597
  16,   0,   0, // 598-600
  2,   0,   0,   0, // 601-604
  16, // 605
  82,   0,   0, // 606-608
  6,   8,   0,   0, // 609-612
  6,   8,   0,   0, // 613-616
  6,   0,   0, // 617-619
  6,   0,   0, // 620-622
  7,   0,   0, // 623-625
  7,   9,   0,   0, // 626-629
  7,   9,   0,   0, // 630-633
  6,   0,   0, // 634-636
  6,   8,   0,   0, // 637-640
  6,   8,   0,   0, // 641-644
  5,   0,   0, // 645-647
  1,   0,   0, // 648-650
  6,   0, // 651-652
  6,   0, // 653-654
  4,   0, // 655-656
  1,   0, // 657-658
  4,   0, // 659-660
  6,   0, // 661-662
  16,   0, // 663-664
  5,   0,   0, // 665-667
  5,   0,   0, // 668-670
  6,   0,   0, // 671-673
  0,   0,   0, // 674-676
  2,   0,   0,   0, // 677-680
  16, // 681
  2,   0,   0, // 682-684
  2,   0,   0, // 685-687
  2,   0,   0, // 688-690
  82,   0,   0, // 691-693
  2,   0,   0,   0, // 694-697
  2,   0,   0,   0, // 698-701
  0,   0,   0, // 702-704
  82,   0,   0, // 705-707
  2,   0, // 708-709
  2,   0, // 710-711
  6,   0, // 712-713
  2,   0,   0, // 714-716
  2,   0, // 717-718
  2,   0, // 719-720
  5,   0,   0, // 721-723
  5,   0,   0, // 724-726
  2,   0,   0, // 727-729
  2,   0,   0, // 730-732
  2,   0,   0, // 733-735
  2,   0,   0, // 736-738
  2,   0,   0, // 739-741
  1,   0,   0, // 742-744
  5,   0,   0, // 745-747
  1,   0,   0, // 748-750
  6,   0,   0, // 751-753
  6,   0, // 754-755
  6,   0,   0, // 756-758
  2,   0,   0, // 759-761
  2,   0,   0, // 762-764
  2,   0,   0, // 765-767
  1,   0, // 768-769
  1,   0,   0, // 770-772
  1,   0,   0, // 773-775
  2,   0,   0, // 776-778
  6,   0,   0, // 779-781
  2,   0,   0, // 782-784
  2,   0,   0,   0, // 785-788
  6,   0, // 789-790
  0,   0,   0, // 791-793
  6,   0,   0, // 794-796
  5,   0,   0, // 797-799
  5,   0,   0, // 800-802
  5,   0,   0, // 803-805
  6,   0,   0, // 806-808
  6,   0,   0, // 809-811
  6,   0,   0, // 812-814
  6,   0,   0,   0, // 815-818
  6,   0,   0, // 819-821
  5,   0,   0, // 822-824
  5,   0,   0, // 825-827
  6,   0,   0, // 828-830
  6,   0,   0, // 831-833
  6,   0,   0,   0, // 834-837
  6,   0,   0, // 838-840
  5,   0,   0, // 841-843
  6,   0,   0, // 844-846
  6,   0,   0, // 847-849
  7,   0,   0, // 850-852
  1,   0,   0, // 853-855
  16,   0,   0, // 856-858
  6,   0,   0, // 859-861
  82,   0,   0, // 862-864
  16, // 865
  6, // 866
  7,   0,   0, // 867-869
  6,   8,   0,   0, // 870-873
  6,   8,   0,   0, // 874-877
  6,   0, // 878-879
  82,   0,   0, // 880-882
  82,   0,   0, // 883-885
  6,   0,   0, // 886-888
  6,   0, // 889-890
  7,   0,   0, // 891-893
  7,   0,   0, // 894-896
  7,   0,   0, // 897-899
  7,   0,   0, // 900-902
  6,   8,   0,   0, // 903-906
  6,   8,   0,   0, // 907-910
  7,   0,   0, // 911-913
  0,   0,   0, // 914-916
  0,   0,   0, // 917-919
  0,   0,   0, // 920-922
  6,   0,   0, // 923-925
  0,   0,   0, // 926-928
  0,   0,   0, // 929-931
  6,   0,   0, // 932-934
  6,   0, // 935-936
  2,   0,   0, // 937-939
  4,   0, // 940-941
  6,   0, // 942-943
  4,   0, // 944-945
  16,   0, // 946-947
  6,   0, // 948-949
  39,   0,   0, // 950-952
  39,   0,   0, // 953-955
  71,   0,   0, // 956-958
  71,   0,   0, // 959-961
  39,   0,   0, // 962-964
  71,   0,   0, // 965-967
  39,   0,   0, // 968-970
  71,   0,   0, // 971-973
  1,   0,   0, // 974-976
  2,   0,   0, // 977-979
  6,   0,   0, // 980-982
  5,   0,   0, // 983-985
  2,   0,   0, // 986-988
  2,   0,   0, // 989-991
  2,   0,   0, // 992-994
  2,   0,   0, // 995-997
  2,   0,   0, // 998-1000
  6,   0, // 1001-1002
  2,   0,   0, // 1003-1005
  2,   0,   0, // 1006-1008
  72,   0,   0, // 1009-1011
  72,   0,   0, // 1012-1014
  69,   0,   0, // 1015-1017
  69,   0,   0, // 1018-1020
  69,   0,   0, // 1021-1023
  65,   0,   0, // 1024-1026
  69,   0,   0, // 1027-1029
  65,   0,   0, // 1030-1032
  65,   0,   0, // 1033-1035
  65,   0,   0, // 1036-1038
  72,   0,   0, // 1039-1041
  59,   0,   0, // 1042-1044
  59,   0,   0, // 1045-1047
  59,   0,   0, // 1048-1050
  59,   0,   0, // 1051-1053
  72,   0,   0, // 1054-1056
  7,   9,   0,   0, // 1057-1060
  7,   9,   0,   0, // 1061-1064
  2,   0,   0,   0, // 1065-1068
  2,   0,   0,   0, // 1069-1072
  2,   0,   0,   0, // 1073-1076
  6,   8,   0,   0, // 1077-1080
  6,   8,   0,   0, // 1081-1084
  6,   0,   0, // 1085-1087
  6,   0,   0, // 1088-1090
  7,   9,   0,   0, // 1091-1094
  7,   9,   0,   0, // 1095-1098
  6,   0,   0, // 1099-1101
  6,   0,   0, // 1102-1104
  6,   0,   0, // 1105-1107
  0,   0,   0, // 1108-1110
  1,   0,   0, // 1111-1113
  6,   0,   0, // 1114-1116
  0,   0,   0, // 1117-1119
  6,   0,   0, // 1120-1122
  16, // 1123
  6,   0,   0, // 1124-1126
  0,   0,   0, // 1127-1129
  7,   0,   0, // 1130-1132
  0,   0,   0, // 1133-1135
  6,   0, // 1136-1137
  6,   0, // 1138-1139
  1,   0,   0, // 1140-1142
  16,   0, // 1143-1144
  6, // 1145
  1,   0,   0, // 1146-1148
  4,   1,   1, // 1149-1151
  4,   1,   1, // 1152-1154
  4,   1, // 1155-1156
  5,   1,   1, // 1157-1159
  4,   1,   1, // 1160-1162
  6,   1, // 1163-1164
  6,   1, // 1165-1166
  17,   1,   1, // 1167-1169
  9,   1,   1, // 1170-1172
  35,   1,   1, // 1173-1175
  7,   1,   1, // 1176-1178
  4,   1,   1, // 1179-1181
  7,   1,   1, // 1182-1184
  6,   1, // 1185-1186
  32,   1,   1, // 1187-1189
  6,   1, // 1190-1191
  4,   1,   1,   1, // 1192-1195
  6,   1,   1, // 1196-1198
  6,   1, // 1199-1200
  6,   1, // 1201-1202
  6,   1, // 1203-1204
  6,   1, // 1205-1206
  6,   1, // 1207-1208
  7,   1, // 1209-1210
  4,   1, // 1211-1212
  4,   1,   1, // 1213-1215
  8,   1, // 1216-1217
  4,   1, // 1218-1219
  7,   1, // 1220-1221
  8,   1, // 1222-1223
  7,   1, // 1224-1225
  7,   1, // 1226-1227
  7,   1, // 1228-1229
  4,   1, // 1230-1231
  5,   1, // 1232-1233
  4,   1, // 1234-1235
  5,   1, // 1236-1237
  7,   1,   1, // 1238-1240
  7,   1,   1, // 1241-1243
  7,   1,   1, // 1244-1246
  4,   1,   1, // 1247-1249
  4,   1,   1, // 1250-1252
  6,   1, // 1253-1254
  6,   1, // 1255-1256
  6,   1, // 1257-1258
  5,   1, // 1259-1260
  4,   1,   1, // 1261-1263
  4,   1, // 1264-1265
  5,   1,   1, // 1266-1268
  5,   1, // 1269-1270
  4,   1,   1, // 1271-1273
  4,   1,   1, // 1274-1276
  5,   1,   1, // 1277-1279
  4,   1,   1, // 1280-1282
  4,   1,   1, // 1283-1285
  4,   1, // 1286-1287
  4,   1,   1, // 1288-1290
  4,   1, // 1291-1292
  4,   1, // 1293-1294
  6,   1, // 1295-1296
  4,   1,   1, // 1297-1299
  8,   1, // 1300-1301
  4,   1,   1, // 1302-1304
  4,   1,   1, // 1305-1307
  4,   1,   1, // 1308-1310
  4,   1,   1, // 1311-1313
  4,   1,   1,   1, // 1314-1317
  4,   1, // 1318-1319
  6,   1, // 1320-1321
  6,   1, // 1322-1323
  4,   1,   1, // 1324-1326
  4,   1,   1, // 1327-1329
  7,   1,   1, // 1330-1332
  7,   1,   1, // 1333-1335
  7,   1,   1, // 1336-1338
  7,   1,   1, // 1339-1341
  7,   1,   1, // 1342-1344
  6,   1, // 1345-1346
  4,   1,   1, // 1347-1349
  6,   1, // 1350-1351
  6,   1, // 1352-1353
  6,   1, // 1354-1355
  6,   1, // 1356-1357
  6,   1, // 1358-1359
  6,   1,   1, // 1360-1362
  7,   1, // 1363-1364
  6,   1, // 1365-1366
  6,   1, // 1367-1368
  4,   1, // 1369-1370
  6,   1, // 1371-1372
  6,   1, // 1373-1374
  4,   1, // 1375-1376
  6,   1, // 1377-1378
  6,   1, // 1379-1380
  6,   1, // 1381-1382
  6,   1, // 1383-1384
  7,   1, // 1385-1386
  6,   1, // 1387-1388
  6,   1, // 1389-1390
  4,   1, // 1391-1392
  4,   1,   1, // 1393-1395
  7,   1, // 1396-1397
  4,   1, // 1398-1399
  7,   1, // 1400-1401
  4,   1, // 1402-1403
  7,   1, // 1404-1405
  5,   1, // 1406-1407
  4,   1, // 1408-1409
  17,   1,   1, // 1410-1412
  17,   1,   1, // 1413-1415
  17,   1,   1, // 1416-1418
  17,   1,   1, // 1419-1421
  4,   1,   1, // 1422-1424
  4,   1,   1, // 1425-1427
  4,   1, // 1428-1429
  4,   1,   1, // 1430-1432
  4,   1,   1, // 1433-1435
  4,   1,   1, // 1436-1438
  8,   1, // 1439-1440
  35,   1,   1, // 1441-1443
  35,   1,   1, // 1444-1446
  35,   1,   1, // 1447-1449
  32,   1,   1, // 1450-1452
  32,   1,   1, // 1453-1455
  32,   1,   1, // 1456-1458
  32,   1,   1, // 1459-1461
  35,   1,   1, // 1462-1464
  6,   1, // 1465-1466
  6,   1, // 1467-1468
  6,   1, // 1469-1470
  6,   1, // 1471-1472
  4,   1, // 1473-1474
  4,   1, // 1475-1476
  6,   1, // 1477-1478
  6,   1, // 1479-1480
  4,   1,   1, // 1481-1483
  4,   1,   1, // 1484-1486
  7,   1,   1, // 1487-1489
  6,   1, // 1490-1491
  6,   1, // 1492-1493
  4,   1, // 1494-1495
  6,   1, // 1496-1497
  6,   1, // 1498-1499
  7,   1, // 1500-1501
  5,   1, // 1502-1503
  4,   1, // 1504-1505
  4,   1, // 1506-1507
  4,   1,   1, // 1508-1510
  5,   1, // 1511-1512
  6,   1, // 1513-1514
  4,   1,   1, // 1515-1517
  4,   1,   1, // 1518-1520
  4,   1, // 1521-1522
  5,   1,   1, // 1523-1525
  4,   1,   1, // 1526-1528
  6,   1, // 1529-1530
  6,   1, // 1531-1532
  17,   1,   1, // 1533-1535
  13,   1,   1, // 1536-1538
  71,   1,   1, // 1539-1541
  11,   1,   1, // 1542-1544
  11,   1,   1, // 1545-1547
  6,   1, // 1548-1549
  41,   1,   1, // 1550-1552
  13,   1,   1,   1, // 1553-1556
  6,   1, // 1557-1558
  4,   1,   1,   1, // 1559-1562
  6,   1,   1, // 1563-1565
  6,   1, // 1566-1567
  6,   1, // 1568-1569
  7,   1,   1, // 1570-1572
  7,   1,   1, // 1573-1575
  7,   1,   1, // 1576-1578
  6,   1, // 1579-1580
  6,   1, // 1581-1582
  6,   1, // 1583-1584
  7,   1, // 1585-1586
  4,   1, // 1587-1588
  4,   1,   1, // 1589-1591
  8,   1, // 1592-1593
  4,   1, // 1594-1595
  11, // 1596
  7,   1, // 1597-1598
  8,   1, // 1599-1600
  7,   1, // 1601-1602
  7,   1, // 1603-1604
  7,   1, // 1605-1606
  4,   1, // 1607-1608
  11,   1,   1, // 1609-1611
  5,   1, // 1612-1613
  4,   1, // 1614-1615
  5,   1, // 1616-1617
  7,   1,   1, // 1618-1620
  7,   1,   1, // 1621-1623
  7,   1,   1, // 1624-1626
  6,   1,   1, // 1627-1629
  6,   1,   1, // 1630-1632
  4,   1,   1, // 1633-1635
  4,   1,   1, // 1636-1638
  6,   1, // 1639-1640
  6,   1, // 1641-1642
  6,   1, // 1643-1644
  5,   1, // 1645-1646
  4,   1, // 1647-1648
  11,   1,   1, // 1649-1651
  11, // 1652
  5,   1,   1, // 1653-1655
  5,   1, // 1656-1657
  11,   1,   1, // 1658-1660
  11,   1,   1, // 1661-1663
  4,   1,   1, // 1664-1666
  5,   1,   1, // 1667-1669
  4,   1,   1, // 1670-1672
  4,   1,   1, // 1673-1675
  4,   1, // 1676-1677
  4,   1,   1, // 1678-1680
  4,   1, // 1681-1682
  4,   1, // 1683-1684
  6,   1, // 1685-1686
  4,   1,   1, // 1687-1689
  8,   1, // 1690-1691
  4,   1,   1, // 1692-1694
  4,   1,   1, // 1695-1697
  4,   1,   1, // 1698-1700
  11,   1,   1, // 1701-1703
  4,   1,   1, // 1704-1706
  4,   1,   1,   1, // 1707-1710
  4,   1, // 1711-1712
  6,   1, // 1713-1714
  6,   1, // 1715-1716
  11,   1,   1, // 1717-1719
  7,   1,   1, // 1720-1722
  7,   1,   1, // 1723-1725
  7,   1,   1, // 1726-1728
  11,   1,   1, // 1729-1731
  13,   1,   1, // 1732-1734
  13,   1,   1,   1, // 1735-1738
  11,   1,   1, // 1739-1741
  7,   1,   1, // 1742-1744
  7,   1,   1, // 1745-1747
  11,   1,   1, // 1748-1750
  13,   1,   1, // 1751-1753
  13,   1,   1,   1, // 1754-1757
  6,   1, // 1758-1759
  11,   1,   1, // 1760-1762
  6,   1, // 1763-1764
  6,   1, // 1765-1766
  7,   1,   1, // 1767-1769
  6,   1, // 1770-1771
  6,   1, // 1772-1773
  6,   1, // 1774-1775
  6,   1,   1, // 1776-1778
  7,   1, // 1779-1780
  7,   1,   1, // 1781-1783
  6,   1, // 1784-1785
  6,   1, // 1786-1787
  4,   1, // 1788-1789
  6,   1, // 1790-1791
  6,   1, // 1792-1793
  4,   1, // 1794-1795
  7,   1,   1, // 1796-1798
  7,   1,   1, // 1799-1801
  7,   1,   1, // 1802-1804
  7,   1,   1, // 1805-1807
  6,   1, // 1808-1809
  6,   1, // 1810-1811
  7,   1,   1, // 1812-1814
  6,   1,   1, // 1815-1817
  6,   1, // 1818-1819
  6,   1, // 1820-1821
  7,   1, // 1822-1823
  6,   1, // 1824-1825
  6,   1,   1, // 1826-1828
  6,   1, // 1829-1830
  4,   1, // 1831-1832
  4,   1,   1, // 1833-1835
  7,   1, // 1836-1837
  4,   1, // 1838-1839
  7,   1, // 1840-1841
  4,   1, // 1842-1843
  7,   1, // 1844-1845
  5,   1, // 1846-1847
  4,   1, // 1848-1849
  17,   1,   1, // 1850-1852
  17,   1,   1, // 1853-1855
  17,   1,   1, // 1856-1858
  17,   1,   1, // 1859-1861
  4,   1,   1, // 1862-1864
  4,   1,   1, // 1865-1867
  11,   1,   1, // 1868-1870
  4,   1, // 1871-1872
  11, // 1873
  11,   1,   1, // 1874-1876
  4,   1,   1, // 1877-1879
  4,   1,   1, // 1880-1882
  4,   1,   1, // 1883-1885
  11, // 1886
  8,   1, // 1887-1888
  71,   1,   1, // 1889-1891
  71,   1,   1, // 1892-1894
  71,   1,   1, // 1895-1897
  41,   1,   1, // 1898-1900
  41,   1,   1, // 1901-1903
  41,   1,   1, // 1904-1906
  41,   1,   1, // 1907-1909
  71,   1,   1, // 1910-1912
  7,   1,   1, // 1913-1915
  7,   1,   1, // 1916-1918
  6,   1,   1, // 1919-1921
  6,   1, // 1922-1923
  6,   1, // 1924-1925
  6,   1, // 1926-1927
  6,   1, // 1928-1929
  7,   1,   1, // 1930-1932
  7,   1,   1, // 1933-1935
  4,   1, // 1936-1937
  4,   1, // 1938-1939
  6,   1, // 1940-1941
  6,   1, // 1942-1943
  4,   1,   1, // 1944-1946
  6,   1, // 1947-1948
  6,   1, // 1949-1950
  4,   1, // 1951-1952
  6,   1, // 1953-1954
  6,   1, // 1955-1956
  7,   1,   1, // 1957-1959
  6,   1,   1, // 1960-1962
  7,   1, // 1963-1964
  5,   1, // 1965-1966
  4,   1, // 1967-1968
  4,   1, // 1969-1970
  4,   1,   1, // 1971-1973
  5,   1, // 1974-1975
  6,   1, // 1976-1977
  5,   2,   2, // 1978-1980
  5,   2,   2, // 1981-1983
  5,   2, // 1984-1985
  6,   2,   2, // 1986-1988
  5,   2,   2, // 1989-1991
  7,   2, // 1992-1993
  7,   2, // 1994-1995
  7,   2, // 1996-1997
  30,   2,   2, // 1998-2000
  20,   2,   2, // 2001-2003
  11,   2,   2, // 2004-2006
  39,   2,   2, // 2007-2009
  11,   2,   2, // 2010-2012
  11,   2,   2, // 2013-2015
  7,   2, // 2016-2017
  5,   2,   2, // 2018-2020
  24,   2,   2, // 2021-2023
  11,   2,   2,   2, // 2024-2027
  7,   2, // 2028-2029
  5,   2,   2,   2, // 2030-2033
  7,   2,   2, // 2034-2036
  7,   2, // 2037-2038
  7,   2, // 2039-2040
  7,   2, // 2041-2042
  7,   2, // 2043-2044
  7,   2, // 2045-2046
  8,   2,   2, // 2047-2049
  8,   2,   2, // 2050-2052
  8,   2,   2, // 2053-2055
  7,   2, // 2056-2057
  7,   2, // 2058-2059
  7,   2, // 2060-2061
  8,   2, // 2062-2063
  9,   2,   2, // 2064-2066
  5,   2, // 2067-2068
  5,   2,   2, // 2069-2071
  9,   2, // 2072-2073
  5, // 2074
  11, // 2075
  9,   2, // 2076-2077
  8,   2, // 2078-2079
  8,   2, // 2080-2081
  5, // 2082
  11,   2,   2, // 2083-2085
  6,   2, // 2086-2087
  5, // 2088
  8,   2,   2, // 2089-2091
  8,   2,   2, // 2092-2094
  8,   2,   2, // 2095-2097
  7,   2,   2, // 2098-2100
  7,   2,   2, // 2101-2103
  6,   2,   2, // 2104-2106
  5,   2,   2, // 2107-2109
  6,   2,   2, // 2110-2112
  7,   2, // 2113-2114
  7,   2, // 2115-2116
  7,   2, // 2117-2118
  7,   2, // 2119-2120
  7,   2, // 2121-2122
  6,   2, // 2123-2124
  5, // 2125
  11,   2,   2, // 2126-2128
  11, // 2129
  6,   2,   2, // 2130-2132
  6,   2, // 2133-2134
  11,   2,   2, // 2135-2137
  11,   2,   2, // 2138-2140
  5,   2,   2, // 2141-2143
  6,   2,   2, // 2144-2146
  5,   2,   2, // 2147-2149
  6,   2,   2, // 2150-2152
  5,   2,   2, // 2153-2155
  5,   2,   2, // 2156-2158
  5,   2, // 2159-2160
  5,   2,   2, // 2161-2163
  5,   2, // 2164-2165
  5, // 2166
  7,   2, // 2167-2168
  6,   2,   2, // 2169-2171
  5,   2,   2, // 2172-2174
  5,   2,   2, // 2175-2177
  9,   2, // 2178-2179
  5,   2,   2, // 2180-2182
  5,   2,   2, // 2183-2185
  5,   2,   2, // 2186-2188
  11,   2,   2, // 2189-2191
  6,   2,   2, // 2192-2194
  5,   2,   2,   2, // 2195-2198
  5, // 2199
  7,   2, // 2200-2201
  7,   2, // 2202-2203
  11,   2,   2, // 2204-2206
  9,   2,   2, // 2207-2209
  8,   2,   2, // 2210-2212
  8,   2,   2, // 2213-2215
  8,   2,   2, // 2216-2218
  11,   2,   2, // 2219-2221
  11,   2,   2, // 2222-2224
  11,   2,   2,   2, // 2225-2228
  11,   2,   2, // 2229-2231
  8,   2,   2, // 2232-2234
  8,   2,   2, // 2235-2237
  9,   2,   2, // 2238-2240
  11,   2,   2, // 2241-2243
  11,   2,   2, // 2244-2246
  11,   2,   2,   2, // 2247-2250
  7,   2, // 2251-2252
  11,   2,   2, // 2253-2255
  7,   2, // 2256-2257
  7,   2, // 2258-2259
  8,   2,   2, // 2260-2262
  7,   2, // 2263-2264
  7,   2, // 2265-2266
  7,   2, // 2267-2268
  7,   2,   2, // 2269-2271
  7,   2, // 2272-2273
  7,   2, // 2274-2275
  8,   2, // 2276-2277
  8,   2,   2, // 2278-2280
  7,   2, // 2281-2282
  7,   2, // 2283-2284
  5, // 2285
  7,   2, // 2286-2287
  7,   2, // 2288-2289
  7,   2, // 2290-2291
  7,   2, // 2292-2293
  5, // 2294
  8,   2,   2, // 2295-2297
  8,   2,   2, // 2298-2300
  8,   2,   2, // 2301-2303
  8,   2,   2, // 2304-2306
  7,   2, // 2307-2308
  7,   2, // 2309-2310
  8,   2,   2, // 2311-2313
  7,   2,   2, // 2314-2316
  7,   2, // 2317-2318
  7,   2, // 2319-2320
  7,   2, // 2321-2322
  8,   2, // 2323-2324
  7,   2, // 2325-2326
  7,   2,   2, // 2327-2329
  7,   2, // 2330-2331
  5, // 2332
  5,   2,   2, // 2333-2335
  8,   2, // 2336-2337
  5, // 2338
  8,   2, // 2339-2340
  5, // 2341
  6,   2, // 2342-2343
  5, // 2344
  20,   2,   2, // 2345-2347
  20,   2,   2, // 2348-2350
  30,   2,   2, // 2351-2353
  30,   2,   2, // 2354-2356
  20,   2,   2, // 2357-2359
  30,   2,   2, // 2360-2362
  20,   2,   2, // 2363-2365
  30,   2,   2, // 2366-2368
  5,   2,   2, // 2369-2371
  5,   2,   2, // 2372-2374
  11,   2,   2, // 2375-2377
  5,   2, // 2378-2379
  11, // 2380
  11,   2,   2, // 2381-2383
  6,   2,   2, // 2384-2386
  5,   2,   2, // 2387-2389
  5,   2,   2, // 2390-2392
  5,   2,   2, // 2393-2395
  6,   2,   2, // 2396-2398
  11, // 2399
  9,   2, // 2400-2401
  5,   2,   2, // 2402-2404
  6,   2,   2, // 2405-2407
  39,   2,   2, // 2408-2410
  39,   2,   2, // 2411-2413
  39,   2,   2, // 2414-2416
  24,   2,   2, // 2417-2419
  24,   2,   2, // 2420-2422
  24,   2,   2, // 2423-2425
  24,   2,   2, // 2426-2428
  39,   2,   2, // 2429-2431
  8,   2,   2, // 2432-2434
  8,   2,   2, // 2435-2437
  7,   2,   2, // 2438-2440
  7,   2, // 2441-2442
  7,   2, // 2443-2444
  7,   2, // 2445-2446
  7,   2, // 2447-2448
  7,   2, // 2449-2450
  7,   2, // 2451-2452
  8,   2,   2, // 2453-2455
  8,   2,   2, // 2456-2458
  5,   2, // 2459-2460
  5,   2, // 2461-2462
  7,   2, // 2463-2464
  7,   2, // 2465-2466
  5,   2,   2, // 2467-2469
  7,   2, // 2470-2471
  7,   2, // 2472-2473
  5,   2, // 2474-2475
  7,   2, // 2476-2477
  7,   2, // 2478-2479
  8,   2,   2, // 2480-2482
  7,   2,   2, // 2483-2485
  5, // 2486
  5, // 2487
  5,   2,   2, // 2488-2490
  6,   2, // 2491-2492
  7,   2, // 2493-2494
  1,   1,   1, // 2495-2497
  1,   1,   1, // 2498-2500
  3,   1,   1, // 2501-2503
  6,   1,   1, // 2504-2506
  1,   1,   1, // 2507-2509
  3,   1,   1, // 2510-2512
  2,   1,   1, // 2513-2515
  2,   1,   1, // 2516-2518
  23,   1,   1, // 2519-2521
  15,   1,   1, // 2522-2524
  5,   1,   1, // 2525-2527
  33,   1,   1, // 2528-2530
  8,   1,   1, // 2531-2533
  5,   1,   1, // 2534-2536
  2,   1,   1, // 2537-2539
  7,   1,   1, // 2540-2542
  1,   1,   1, // 2543-2545
  1,   1,   1, // 2546-2548
  27,   1,   1, // 2549-2551
  5,   1,   1,   1, // 2552-2555
  44,   1,   1, // 2556-2558
  32,   1,   1, // 2559-2561
  1,   1,   1,   1, // 2562-2565
  3,   1,   1, // 2566-2568
  2,   2,   1,   1, // 2569-2572
  3,   3,   1,   1, // 2573-2576
  3,   1,   1, // 2577-2579
  2,   2,   1,   1, // 2580-2583
  3,   3,   1,   1, // 2584-2587
  3,   1,   1, // 2588-2590
  3,   3,   1,   1, // 2591-2594
  3,   3,   1,   1, // 2595-2598
  3,   1,   1, // 2599-2601
  4,   4,   1,   1, // 2602-2605
  4,   4,   1,   1, // 2606-2609
  2,   1,   1, // 2610-2612
  3,   1,   1, // 2613-2615
  4,   1,   1, // 2616-2618
  3,   1,   1, // 2619-2621
  3,   1, // 2622-2623
  6,   1, // 2624-2625
  3,   1, // 2626-2627
  4,   1, // 2628-2629
  4,   1,   1, // 2630-2632
  4,   1,   1, // 2633-2635
  4,   1,   1, // 2636-2638
  3,   1,   1, // 2639-2641
  1,   1,   1, // 2642-2644
  2,   1,   1,   1, // 2645-2648
  1,   1,   1, // 2649-2651
  1,   1,   1, // 2652-2654
  1,   1,   1, // 2655-2657
  1,   1,   1, // 2658-2660
  2,   1,   1,   1, // 2661-2664
  2,   1,   1,   1, // 2665-2668
  1,   1,   1, // 2669-2671
  1,   1, // 2672-2673
  1,   1, // 2674-2675
  6,   1,   1, // 2676-2678
  2,   1,   1, // 2679-2681
  6,   1,   1, // 2682-2684
  3,   1,   1, // 2685-2687
  4,   1, // 2688-2689
  1,   1,   1, // 2690-2692
  1,   1, // 2693-2694
  1,   1, // 2695-2696
  8,   1,   1, // 2697-2699
  6,   1,   1, // 2700-2702
  6,   1,   1, // 2703-2705
  8,   1,   1, // 2706-2708
  1,   1,   1, // 2709-2711
  1,   1,   1, // 2712-2714
  1,   1,   1, // 2715-2717
  2,   1,   1, // 2718-2720
  1,   1,   1, // 2721-2723
  1,   1,   1, // 2724-2726
  1,   1,   1, // 2727-2729
  3,   1,   1, // 2730-2732
  3,   1, // 2733-2734
  6,   1,   1, // 2735-2737
  3,   1,   1, // 2738-2740
  4,   1, // 2741-2742
  2,   1,   1, // 2743-2745
  1,   1,   1, // 2746-2748
  1,   1,   1, // 2749-2751
  1,   1,   1, // 2752-2754
  3,   1, // 2755-2756
  3,   1, // 2757-2758
  3,   1,   1, // 2759-2761
  1,   1,   1, // 2762-2764
  5,   1,   1, // 2765-2767
  1,   1,   1, // 2768-2770
  1,   1,   1,   1, // 2771-2774
  6,   1,   1, // 2775-2777
  4,   1, // 2778-2779
  1,   1,   1, // 2780-2782
  2,   1,   1, // 2783-2785
  6,   1,   1, // 2786-2788
  2,   1,   1, // 2789-2791
  6,   1,   1, // 2792-2794
  8,   1,   1, // 2795-2797
  6,   1,   1, // 2798-2800
  2,   1,   1, // 2801-2803
  4,   1,   1, // 2804-2806
  4,   1,   1, // 2807-2809
  4,   1,   1, // 2810-2812
  4,   1,   1, // 2813-2815
  5,   1,   1, // 2816-2818
  5,   1,   1, // 2819-2821
  5,   1,   1,   1, // 2822-2825
  6,   1,   1, // 2826-2828
  5,   1,   1, // 2829-2831
  4,   1,   1, // 2832-2834
  4,   1,   1, // 2835-2837
  4,   1,   1, // 2838-2840
  5,   1,   1, // 2841-2843
  5,   1,   1, // 2844-2846
  5,   1,   1,   1, // 2847-2850
  6,   1,   1, // 2851-2853
  2,   1,   1, // 2854-2856
  3,   1,   1, // 2857-2859
  6,   1,   1, // 2860-2862
  2,   1,   1, // 2863-2865
  8,   1,   1, // 2866-2868
  6,   1,   1, // 2869-2871
  6,   1,   1, // 2872-2874
  6,   1,   1, // 2875-2877
  6,   1,   1, // 2878-2880
  6,   1,   1, // 2881-2883
  6,   1,   1, // 2884-2886
  2,   1,   1, // 2887-2889
  2,   1,   1, // 2890-2892
  3,   1,   1, // 2893-2895
  2,   1,   1, // 2896-2898
  3,   1,   1, // 2899-2901
  2,   1,   1, // 2902-2904
  3,   1,   1, // 2905-2907
  2,   1,   1, // 2908-2910
  3,   1,   1, // 2911-2913
  2,   2,   1,   1, // 2914-2917
  3,   3,   1,   1, // 2918-2921
  4,   1, // 2922-2923
  1,   1,   1, // 2924-2926
  1,   1,   1, // 2927-2929
  2,   1,   1, // 2930-2932
  3,   1,   1, // 2933-2935
  3,   1,   1, // 2936-2938
  4,   1, // 2939-2940
  3,   1,   1, // 2941-2943
  3,   1,   1, // 2944-2946
  3,   1,   1, // 2947-2949
  3,   1,   1, // 2950-2952
  4,   4,   1,   1, // 2953-2956
  4,   4,   1,   1, // 2957-2960
  3,   1,   1, // 2961-2963
  1,   1,   1, // 2964-2966
  1,   1,   1, // 2967-2969
  2,   1,   1, // 2970-2972
  2,   1,   1, // 2973-2975
  1,   1,   1, // 2976-2978
  1,   1,   1, // 2979-2981
  2,   1,   1, // 2982-2984
  4,   1, // 2985-2986
  1,   1,   1, // 2987-2989
  4,   1, // 2990-2991
  15,   1,   1, // 2992-2994
  15,   1,   1, // 2995-2997
  23,   1,   1, // 2998-3000
  23,   1,   1, // 3001-3003
  15,   1,   1, // 3004-3006
  23,   1,   1, // 3007-3009
  15,   1,   1, // 3010-3012
  23,   1,   1, // 3013-3015
  3,   1, // 3016-3017
  1,   1,   1, // 3018-3020
  5,   1,   1, // 3021-3023
  3,   1,   1, // 3024-3026
  1,   1,   1, // 3027-3029
  1,   1,   1, // 3030-3032
  1,   1,   1, // 3033-3035
  1,   1,   1, // 3036-3038
  1,   1,   1, // 3039-3041
  6,   1, // 3042-3043
  1,   1,   1, // 3044-3046
  1,   1,   1, // 3047-3049
  33,   1,   1, // 3050-3052
  33,   1,   1, // 3053-3055
  44,   1,   1, // 3056-3058
  44,   1,   1, // 3059-3061
  44,   1,   1, // 3062-3064
  32,   1,   1, // 3065-3067
  44,   1,   1, // 3068-3070
  32,   1,   1, // 3071-3073
  32,   1,   1, // 3074-3076
  32,   1,   1, // 3077-3079
  33,   1,   1, // 3080-3082
  27,   1,   1, // 3083-3085
  27,   1,   1, // 3086-3088
  27,   1,   1, // 3089-3091
  27,   1,   1, // 3092-3094
  33,   1,   1, // 3095-3097
  3,   3,   1,   1, // 3098-3101
  3,   3,   1,   1, // 3102-3105
  2,   1,   1,   1, // 3106-3109
  2,   1,   1,   1, // 3110-3113
  2,   1,   1,   1, // 3114-3117
  2,   2,   1,   1, // 3118-3121
  3,   3,   1,   1, // 3122-3125
  2,   2,   1,   1, // 3126-3129
  3,   3,   1,   1, // 3130-3133
  3,   3,   1,   1, // 3134-3137
  3,   3,   1,   1, // 3138-3141
  6,   1,   1, // 3142-3144
  3,   1,   1, // 3145-3147
  3,   1,   1, // 3148-3150
  2,   1,   1, // 3151-3153
  1,   1,   1, // 3154-3156
  1,   1,   1, // 3157-3159
  2,   1,   1, // 3160-3162
  7,   1,   1, // 3163-3165
  2,   1,   1, // 3166-3168
  1,   1,   1, // 3169-3171
  3,   1,   1, // 3172-3174
  2,   1,   1, // 3175-3177
  1,   1,   1, // 3178-3180
  3,   1,   1, // 3181-3183
  1,   1,   1, // 3184-3186
  4,   1, // 3187-3188
  1,   1,   1, // 3189-3191
  1,   1,   1, // 3192-3194
  1,   1,   1, // 3195-3197
  3,   1,   1, // 3198-3200
  6,   1,   1, // 3201-3203
  1,   1,   1, // 3204-3206
  3,   1,   1, // 3207-3209
  2,   1,   1, // 3210-3212
  2,   1,   1, // 3213-3215
  68,   1,   1, // 3216-3218
  36,   1,   1, // 3219-3221
  5,   1,   1, // 3222-3224
  33,   1,   1, // 3225-3227
  8,   1,   1, // 3228-3230
  5,   1,   1, // 3231-3233
  2,   1,   1, // 3234-3236
  7,   1,   1, // 3237-3239
  1,   1,   1, // 3240-3242
  1,   1,   1, // 3243-3245
  27,   1,   1, // 3246-3248
  5,   1,   1,   1, // 3249-3252
  44,   1,   1, // 3253-3255
  32,   1,   1, // 3256-3258
  1,   1,   1,   1, // 3259-3262
  3,   1,   1, // 3263-3265
  2,   2,   1,   1, // 3266-3269
  3,   3,   1,   1, // 3270-3273
  3,   1,   1, // 3274-3276
  2,   2,   1,   1, // 3277-3280
  3,   3,   1,   1, // 3281-3284
  3,   1,   1, // 3285-3287
  3,   3,   1,   1, // 3288-3291
  3,   3,   1,   1, // 3292-3295
  3,   1,   1, // 3296-3298
  4,   4,   1,   1, // 3299-3302
  4,   4,   1,   1, // 3303-3306
  2,   1,   1, // 3307-3309
  3,   1,   1, // 3310-3312
  4,   1,   1, // 3313-3315
  3,   1,   1, // 3316-3318
  3,   1, // 3319-3320
  6,   1, // 3321-3322
  3,   1, // 3323-3324
  4,   1, // 3325-3326
  4,   1,   1, // 3327-3329
  4,   1,   1, // 3330-3332
  4,   1,   1, // 3333-3335
  3,   1,   1, // 3336-3338
  1,   1,   1, // 3339-3341
  2,   1,   1,   1, // 3342-3345
  1,   1,   1, // 3346-3348
  1,   1,   1, // 3349-3351
  1,   1,   1, // 3352-3354
  1,   1,   1, // 3355-3357
  2,   1,   1,   1, // 3358-3361
  2,   1,   1,   1, // 3362-3365
  1,   1,   1, // 3366-3368
  1,   1,   1, // 3369-3371
  1,   1, // 3372-3373
  1,   1, // 3374-3375
  6,   1,   1, // 3376-3378
  2,   1,   1, // 3379-3381
  6,   1,   1, // 3382-3384
  3,   1,   1, // 3385-3387
  4,   1, // 3388-3389
  1,   1,   1, // 3390-3392
  1,   1, // 3393-3394
  1,   1, // 3395-3396
  8,   1,   1, // 3397-3399
  6,   1,   1, // 3400-3402
  6,   1,   1, // 3403-3405
  8,   1,   1, // 3406-3408
  1,   1,   1, // 3409-3411
  1,   1,   1, // 3412-3414
  1,   1,   1, // 3415-3417
  2,   1,   1, // 3418-3420
  1,   1,   1, // 3421-3423
  1,   1,   1, // 3424-3426
  1,   1,   1, // 3427-3429
  3,   1,   1, // 3430-3432
  3,   1, // 3433-3434
  6,   1,   1, // 3435-3437
  3,   1,   1, // 3438-3440
  4,   1, // 3441-3442
  2,   1,   1, // 3443-3445
  1,   1,   1, // 3446-3448
  1,   1,   1, // 3449-3451
  1,   1,   1, // 3452-3454
  3,   1, // 3455-3456
  3,   1, // 3457-3458
  3,   1,   1, // 3459-3461
  1,   1,   1, // 3462-3464
  5,   1,   1, // 3465-3467
  1,   1,   1, // 3468-3470
  1,   1,   1,   1, // 3471-3474
  6,   1,   1, // 3475-3477
  4,   1, // 3478-3479
  1,   1,   1, // 3480-3482
  2,   1,   1, // 3483-3485
  6,   1,   1, // 3486-3488
  2,   1,   1, // 3489-3491
  6,   1,   1, // 3492-3494
  8,   1,   1, // 3495-3497
  6,   1,   1, // 3498-3500
  2,   1,   1, // 3501-3503
  4,   1,   1, // 3504-3506
  4,   1,   1, // 3507-3509
  4,   1,   1, // 3510-3512
  4,   1,   1, // 3513-3515
  5,   1,   1, // 3516-3518
  5,   1,   1, // 3519-3521
  5,   1,   1,   1, // 3522-3525
  6,   1,   1, // 3526-3528
  5,   1,   1, // 3529-3531
  4,   1,   1, // 3532-3534
  4,   1,   1, // 3535-3537
  4,   1,   1, // 3538-3540
  5,   1,   1, // 3541-3543
  5,   1,   1, // 3544-3546
  5,   1,   1,   1, // 3547-3550
  6,   1,   1, // 3551-3553
  2,   1,   1, // 3554-3556
  3,   1,   1, // 3557-3559
  6,   1,   1, // 3560-3562
  2,   1,   1, // 3563-3565
  8,   1,   1, // 3566-3568
  6,   1,   1, // 3569-3571
  6,   1,   1, // 3572-3574
  6,   1,   1, // 3575-3577
  6,   1,   1, // 3578-3580
  6,   1,   1, // 3581-3583
  6,   1,   1, // 3584-3586
  2,   1,   1, // 3587-3589
  2,   1,   1, // 3590-3592
  3,   1,   1, // 3593-3595
  2,   1,   1, // 3596-3598
  3,   1,   1, // 3599-3601
  2,   1,   1, // 3602-3604
  3,   1,   1, // 3605-3607
  2,   1,   1, // 3608-3610
  3,   1,   1, // 3611-3613
  2,   2,   1,   1, // 3614-3617
  3,   3,   1,   1, // 3618-3621
  4,   1, // 3622-3623
  1,   1,   1, // 3624-3626
  1,   1,   1, // 3627-3629
  2,   1,   1, // 3630-3632
  3,   1,   1, // 3633-3635
  3,   1,   1, // 3636-3638
  4,   1, // 3639-3640
  3,   1,   1, // 3641-3643
  3,   1,   1, // 3644-3646
  3,   1,   1, // 3647-3649
  3,   1,   1, // 3650-3652
  4,   4,   1,   1, // 3653-3656
  4,   4,   1,   1, // 3657-3660
  3,   1,   1, // 3661-3663
  1,   1,   1, // 3664-3666
  1,   1,   1, // 3667-3669
  1,   1,   1, // 3670-3672
  2,   1,   1, // 3673-3675
  2,   1,   1, // 3676-3678
  1,   1,   1, // 3679-3681
  1,   1,   1, // 3682-3684
  2,   1,   1, // 3685-3687
  4,   1, // 3688-3689
  1,   1,   1, // 3690-3692
  4,   1, // 3693-3694
  36,   1,   1, // 3695-3697
  36,   1,   1, // 3698-3700
  68,   1,   1, // 3701-3703
  68,   1,   1, // 3704-3706
  36,   1,   1, // 3707-3709
  68,   1,   1, // 3710-3712
  36,   1,   1, // 3713-3715
  68,   1,   1, // 3716-3718
  3,   1, // 3719-3720
  1,   1,   1, // 3721-3723
  5,   1,   1, // 3724-3726
  3,   1,   1, // 3727-3729
  1,   1,   1, // 3730-3732
  1,   1,   1, // 3733-3735
  1,   1,   1, // 3736-3738
  1,   1,   1, // 3739-3741
  1,   1,   1, // 3742-3744
  6,   1, // 3745-3746
  1,   1,   1, // 3747-3749
  1,   1,   1, // 3750-3752
  33,   1,   1, // 3753-3755
  33,   1,   1, // 3756-3758
  44,   1,   1, // 3759-3761
  44,   1,   1, // 3762-3764
  44,   1,   1, // 3765-3767
  32,   1,   1, // 3768-3770
  44,   1,   1, // 3771-3773
  32,   1,   1, // 3774-3776
  32,   1,   1, // 3777-3779
  32,   1,   1, // 3780-3782
  33,   1,   1, // 3783-3785
  27,   1,   1, // 3786-3788
  27,   1,   1, // 3789-3791
  27,   1,   1, // 3792-3794
  27,   1,   1, // 3795-3797
  33,   1,   1, // 3798-3800
  3,   3,   1,   1, // 3801-3804
  3,   3,   1,   1, // 3805-3808
  2,   1,   1,   1, // 3809-3812
  2,   1,   1,   1, // 3813-3816
  2,   1,   1,   1, // 3817-3820
  2,   2,   1,   1, // 3821-3824
  3,   3,   1,   1, // 3825-3828
  2,   2,   1,   1, // 3829-3832
  3,   3,   1,   1, // 3833-3836
  3,   3,   1,   1, // 3837-3840
  3,   3,   1,   1, // 3841-3844
  6,   1,   1, // 3845-3847
  3,   1,   1, // 3848-3850
  3,   1,   1, // 3851-3853
  2,   1,   1, // 3854-3856
  1,   1,   1, // 3857-3859
  1,   1,   1, // 3860-3862
  2,   1,   1, // 3863-3865
  7,   1,   1, // 3866-3868
  2,   1,   1, // 3869-3871
  1,   1,   1, // 3872-3874
  3,   1,   1, // 3875-3877
  2,   1,   1, // 3878-3880
  1,   1,   1, // 3881-3883
  3,   1,   1, // 3884-3886
  1,   1,   1, // 3887-3889
  4,   1, // 3890-3891
  1,   1,   1, // 3892-3894
  0 // End operand cycles
};
extern const unsigned PPCForwardingPaths[] = {
 0, // No itinerary
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 1-3
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 4-6
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 7-8
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 9-11
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 12-14
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 15-16
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 17-19
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 20-22
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 23-25
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 26-28
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 29-31
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 32-34
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 35-37
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 38-40
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 41-44
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 45-47
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::NoBypass, // 48-51
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 52-54
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 55-58
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 59-62
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 63-65
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 66-69
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 70-73
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 74-76
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 77-79
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 80-82
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 83-85
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 86-88
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 89-91
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 92-93
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 94-95
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 96-98
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 99-100
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 101-102
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 103-104
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 105-106
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 107-109
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 110-111
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 112-113
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 114-116
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 117-119
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 120-122
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 123-125
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 126-129
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 130-131
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 132-134
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 135-137
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 138-140
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 141-144
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 145-148
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 149-151
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 152-154
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 155-156
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 157-158
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 159-161
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 162-164
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 165-167
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 168-169
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 170-172
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 173-175
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 176-178
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 179-181
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 182-184
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 185-187
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 188-190
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 191-193
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 194-195
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 196-197
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 198-200
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 201-203
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 204-206
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 207-209
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 210-212
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 213-215
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 216-218
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::NoBypass, // 219-222
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 223-224
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 225-227
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 228-230
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 231-233
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 234-236
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 237-239
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 240-242
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 243-245
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 246-248
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 249-252
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 253-255
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 256-258
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 259-261
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 262-264
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 265-267
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 268-271
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 272-274
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 275-277
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 278-280
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 281-283
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 284-286
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 287-288
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 289-291
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 292-294
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 295-297
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 298-300
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 301-303
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 304-307
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 308-311
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 312-313
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 314-316
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 317-319
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 320-322
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 323-324
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 325-327
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 328-330
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 331-333
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 334-336
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 337-339
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 340-342
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 343-345
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 346-348
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 349-351
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 352-354
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 355-357
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 358-360
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 361-363
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 364-366
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 367-369
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 370-371
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 372-374
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 375-376
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 377-378
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 379-380
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 381-382
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 383-384
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 385-387
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 388-390
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 391-393
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 394-396
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 397-399
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 400-402
PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 403-405
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 406-408
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 409-411
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 412-414
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 415-417
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 418-420
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 421-423
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 424-426
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 427-428
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 429-431
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 432-434
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 435-437
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 438-440
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 441-443
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 444-446
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 447-449
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_FPR_Bypass, PPC440ItinerariesBypass::P440_FPR_Bypass, // 450-452
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 453-456
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 457-460
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 461-464
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 465-468
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 469-472
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 473-476
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0,  0, // 477-480
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 481-484
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 485-488
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 489-490
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 491-492
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 493-495
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 496-498
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 499-501
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 502-504
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 505-507
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 508-509
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 510-511
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 512-514
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass,  0, // 515-517
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 518-520
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 521-523
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 524-525
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 526-527
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 528-529
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 530-531
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 532-534
PPC440ItinerariesBypass::P440_GPR_Bypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 535-536
PPC440ItinerariesBypass::NoBypass, PPC440ItinerariesBypass::P440_GPR_Bypass, // 537-538
 0, // 539
 0,  0,  0, // 540-542
 0,  0,  0, // 543-545
 0,  0,  0, // 546-548
 0,  0,  0, // 549-551
 0,  0,  0, // 552-554
 0,  0,  0, // 555-557
 0,  0,  0, // 558-560
 0,  0,  0, // 561-563
 0,  0,  0, // 564-566
 0,  0,  0, // 567-569
 0,  0,  0, // 570-572
 0,  0,  0, // 573-575
 0,  0,  0, // 576-578
 0,  0,  0, // 579-581
 0,  0,  0, // 582-584
 0,  0,  0, // 585-587
 0,  0,  0,  0, // 588-591
 0,  0,  0, // 592-594
 0,  0,  0, // 595-597
 0,  0,  0, // 598-600
 0,  0,  0,  0, // 601-604
 0, // 605
 0,  0,  0, // 606-608
 0,  0,  0,  0, // 609-612
 0,  0,  0,  0, // 613-616
 0,  0,  0, // 617-619
 0,  0,  0, // 620-622
 0,  0,  0, // 623-625
 0,  0,  0,  0, // 626-629
 0,  0,  0,  0, // 630-633
 0,  0,  0, // 634-636
 0,  0,  0,  0, // 637-640
 0,  0,  0,  0, // 641-644
 0,  0,  0, // 645-647
 0,  0,  0, // 648-650
 0,  0, // 651-652
 0,  0, // 653-654
 0,  0, // 655-656
 0,  0, // 657-658
 0,  0, // 659-660
 0,  0, // 661-662
 0,  0, // 663-664
 0,  0,  0, // 665-667
 0,  0,  0, // 668-670
 0,  0,  0, // 671-673
 0,  0,  0, // 674-676
 0,  0,  0,  0, // 677-680
 0, // 681
 0,  0,  0, // 682-684
 0,  0,  0, // 685-687
 0,  0,  0, // 688-690
 0,  0,  0, // 691-693
 0,  0,  0,  0, // 694-697
 0,  0,  0,  0, // 698-701
 0,  0,  0, // 702-704
 0,  0,  0, // 705-707
 0,  0, // 708-709
 0,  0, // 710-711
 0,  0, // 712-713
 0,  0,  0, // 714-716
 0,  0, // 717-718
 0,  0, // 719-720
 0,  0,  0, // 721-723
 0,  0,  0, // 724-726
 0,  0,  0, // 727-729
 0,  0,  0, // 730-732
 0,  0,  0, // 733-735
 0,  0,  0, // 736-738
 0,  0,  0, // 739-741
 0,  0,  0, // 742-744
 0,  0,  0, // 745-747
 0,  0,  0, // 748-750
 0,  0,  0, // 751-753
 0,  0, // 754-755
 0,  0,  0, // 756-758
 0,  0,  0, // 759-761
 0,  0,  0, // 762-764
 0,  0,  0, // 765-767
 0,  0, // 768-769
 0,  0,  0, // 770-772
 0,  0,  0, // 773-775
 0,  0,  0, // 776-778
 0,  0,  0, // 779-781
 0,  0,  0, // 782-784
 0,  0,  0,  0, // 785-788
 0,  0, // 789-790
 0,  0,  0, // 791-793
 0,  0,  0, // 794-796
 0,  0,  0, // 797-799
 0,  0,  0, // 800-802
 0,  0,  0, // 803-805
 0,  0,  0, // 806-808
 0,  0,  0, // 809-811
 0,  0,  0, // 812-814
 0,  0,  0,  0, // 815-818
 0,  0,  0, // 819-821
 0,  0,  0, // 822-824
 0,  0,  0, // 825-827
 0,  0,  0, // 828-830
 0,  0,  0, // 831-833
 0,  0,  0,  0, // 834-837
 0,  0,  0, // 838-840
 0,  0,  0, // 841-843
 0,  0,  0, // 844-846
 0,  0,  0, // 847-849
 0,  0,  0, // 850-852
 0,  0,  0, // 853-855
 0,  0,  0, // 856-858
 0,  0,  0, // 859-861
 0,  0,  0, // 862-864
 0, // 865
 0, // 866
 0,  0,  0, // 867-869
 0,  0,  0,  0, // 870-873
 0,  0,  0,  0, // 874-877
 0,  0, // 878-879
 0,  0,  0, // 880-882
 0,  0,  0, // 883-885
 0,  0,  0, // 886-888
 0,  0, // 889-890
 0,  0,  0, // 891-893
 0,  0,  0, // 894-896
 0,  0,  0, // 897-899
 0,  0,  0, // 900-902
 0,  0,  0,  0, // 903-906
 0,  0,  0,  0, // 907-910
 0,  0,  0, // 911-913
 0,  0,  0, // 914-916
 0,  0,  0, // 917-919
 0,  0,  0, // 920-922
 0,  0,  0, // 923-925
 0,  0,  0, // 926-928
 0,  0,  0, // 929-931
 0,  0,  0, // 932-934
 0,  0, // 935-936
 0,  0,  0, // 937-939
 0,  0, // 940-941
 0,  0, // 942-943
 0,  0, // 944-945
 0,  0, // 946-947
 0,  0, // 948-949
 0,  0,  0, // 950-952
 0,  0,  0, // 953-955
 0,  0,  0, // 956-958
 0,  0,  0, // 959-961
 0,  0,  0, // 962-964
 0,  0,  0, // 965-967
 0,  0,  0, // 968-970
 0,  0,  0, // 971-973
 0,  0,  0, // 974-976
 0,  0,  0, // 977-979
 0,  0,  0, // 980-982
 0,  0,  0, // 983-985
 0,  0,  0, // 986-988
 0,  0,  0, // 989-991
 0,  0,  0, // 992-994
 0,  0,  0, // 995-997
 0,  0,  0, // 998-1000
 0,  0, // 1001-1002
 0,  0,  0, // 1003-1005
 0,  0,  0, // 1006-1008
 0,  0,  0, // 1009-1011
 0,  0,  0, // 1012-1014
 0,  0,  0, // 1015-1017
 0,  0,  0, // 1018-1020
 0,  0,  0, // 1021-1023
 0,  0,  0, // 1024-1026
 0,  0,  0, // 1027-1029
 0,  0,  0, // 1030-1032
 0,  0,  0, // 1033-1035
 0,  0,  0, // 1036-1038
 0,  0,  0, // 1039-1041
 0,  0,  0, // 1042-1044
 0,  0,  0, // 1045-1047
 0,  0,  0, // 1048-1050
 0,  0,  0, // 1051-1053
 0,  0,  0, // 1054-1056
 0,  0,  0,  0, // 1057-1060
 0,  0,  0,  0, // 1061-1064
 0,  0,  0,  0, // 1065-1068
 0,  0,  0,  0, // 1069-1072
 0,  0,  0,  0, // 1073-1076
 0,  0,  0,  0, // 1077-1080
 0,  0,  0,  0, // 1081-1084
 0,  0,  0, // 1085-1087
 0,  0,  0, // 1088-1090
 0,  0,  0,  0, // 1091-1094
 0,  0,  0,  0, // 1095-1098
 0,  0,  0, // 1099-1101
 0,  0,  0, // 1102-1104
 0,  0,  0, // 1105-1107
 0,  0,  0, // 1108-1110
 0,  0,  0, // 1111-1113
 0,  0,  0, // 1114-1116
 0,  0,  0, // 1117-1119
 0,  0,  0, // 1120-1122
 0, // 1123
 0,  0,  0, // 1124-1126
 0,  0,  0, // 1127-1129
 0,  0,  0, // 1130-1132
 0,  0,  0, // 1133-1135
 0,  0, // 1136-1137
 0,  0, // 1138-1139
 0,  0,  0, // 1140-1142
 0,  0, // 1143-1144
 0, // 1145
 0,  0,  0, // 1146-1148
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1149-1151
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1152-1154
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1155-1156
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1157-1159
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1160-1162
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1163-1164
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1165-1166
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1167-1169
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1170-1172
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1173-1175
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1176-1178
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1179-1181
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1182-1184
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1185-1186
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1187-1189
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1190-1191
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1192-1195
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1196-1198
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1199-1200
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1201-1202
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1203-1204
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1205-1206
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1207-1208
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1209-1210
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1211-1212
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass,  0, // 1213-1215
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1216-1217
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1218-1219
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1220-1221
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1222-1223
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1224-1225
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1226-1227
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1228-1229
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1230-1231
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1232-1233
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1234-1235
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1236-1237
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1238-1240
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1241-1243
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1244-1246
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1247-1249
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1250-1252
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1253-1254
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1255-1256
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1257-1258
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1259-1260
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1261-1263
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1264-1265
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1266-1268
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1269-1270
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1271-1273
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1274-1276
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1277-1279
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1280-1282
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1283-1285
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1286-1287
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass,  0, // 1288-1290
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1291-1292
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1293-1294
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1295-1296
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1297-1299
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1300-1301
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass,  0, // 1302-1304
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1305-1307
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1308-1310
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1311-1313
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1314-1317
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1318-1319
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1320-1321
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1322-1323
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1324-1326
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1327-1329
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1330-1332
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1333-1335
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1336-1338
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1339-1341
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1342-1344
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1345-1346
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1347-1349
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1350-1351
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1352-1353
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1354-1355
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1356-1357
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1358-1359
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1360-1362
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1363-1364
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1365-1366
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1367-1368
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1369-1370
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1371-1372
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1373-1374
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1375-1376
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1377-1378
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1379-1380
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1381-1382
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1383-1384
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1385-1386
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1387-1388
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1389-1390
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1391-1392
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1393-1395
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1396-1397
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1398-1399
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1400-1401
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1402-1403
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1404-1405
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1406-1407
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1408-1409
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1410-1412
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1413-1415
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1416-1418
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1419-1421
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass,  0, // 1422-1424
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1425-1427
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1428-1429
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1430-1432
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1433-1435
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1436-1438
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1439-1440
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1441-1443
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1444-1446
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1447-1449
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1450-1452
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1453-1455
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1456-1458
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1459-1461
PPCE500ItinerariesBypass::E500_DivBypass,  0,  0, // 1462-1464
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1465-1466
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1467-1468
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1469-1470
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1471-1472
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1473-1474
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1475-1476
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1477-1478
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1479-1480
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1481-1483
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1484-1486
PPCE500ItinerariesBypass::NoBypass,  0,  0, // 1487-1489
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1490-1491
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1492-1493
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1494-1495
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1496-1497
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1498-1499
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1500-1501
PPCE500ItinerariesBypass::NoBypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1502-1503
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_CR_Bypass, // 1504-1505
PPCE500ItinerariesBypass::E500_CR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1506-1507
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1508-1510
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1511-1512
PPCE500ItinerariesBypass::E500_GPR_Bypass, PPCE500ItinerariesBypass::E500_GPR_Bypass, // 1513-1514
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1515-1517
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1518-1520
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1521-1522
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1523-1525
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1526-1528
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1529-1530
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1531-1532
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1533-1535
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1536-1538
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1539-1541
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1542-1544
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1545-1547
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1548-1549
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1550-1552
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1553-1556
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1557-1558
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1559-1562
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1563-1565
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1566-1567
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1568-1569
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1570-1572
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1573-1575
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1576-1578
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1579-1580
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1581-1582
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1583-1584
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1585-1586
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1587-1588
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass,  0, // 1589-1591
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1592-1593
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1594-1595
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1596
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1597-1598
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1599-1600
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1601-1602
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1603-1604
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1605-1606
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1607-1608
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, // 1609-1611
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1612-1613
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1614-1615
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1616-1617
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1618-1620
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1621-1623
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1624-1626
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1627-1629
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1630-1632
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1633-1635
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1636-1638
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1639-1640
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1641-1642
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1643-1644
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1645-1646
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1647-1648
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, // 1649-1651
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1652
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1653-1655
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1656-1657
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1658-1660
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1661-1663
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1664-1666
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1667-1669
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1670-1672
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1673-1675
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1676-1677
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass,  0, // 1678-1680
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1681-1682
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1683-1684
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1685-1686
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1687-1689
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1690-1691
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass,  0, // 1692-1694
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1695-1697
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1698-1700
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1701-1703
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1704-1706
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1707-1710
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1711-1712
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1713-1714
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1715-1716
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1717-1719
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1720-1722
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1723-1725
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1726-1728
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1729-1731
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1732-1734
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1735-1738
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1739-1741
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1742-1744
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1745-1747
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1748-1750
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1751-1753
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1754-1757
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1758-1759
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1760-1762
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1763-1764
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1765-1766
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1767-1769
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1770-1771
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1772-1773
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1774-1775
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1776-1778
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1779-1780
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1781-1783
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1784-1785
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1786-1787
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1788-1789
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1790-1791
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1792-1793
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1794-1795
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1796-1798
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1799-1801
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1802-1804
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1805-1807
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1808-1809
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1810-1811
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1812-1814
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1815-1817
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1818-1819
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1820-1821
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1822-1823
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1824-1825
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1826-1828
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1829-1830
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1831-1832
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1833-1835
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1836-1837
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1838-1839
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1840-1841
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1842-1843
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1844-1845
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1846-1847
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1848-1849
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1850-1852
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1853-1855
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1856-1858
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1859-1861
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass,  0, // 1862-1864
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1865-1867
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1868-1870
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1871-1872
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1873
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::NoBypass, // 1874-1876
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1877-1879
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1880-1882
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1883-1885
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1886
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1887-1888
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1889-1891
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1892-1894
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1895-1897
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1898-1900
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1901-1903
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1904-1906
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1907-1909
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, // 1910-1912
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1913-1915
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1916-1918
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1919-1921
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1922-1923
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1924-1925
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1926-1927
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1928-1929
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1930-1932
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1933-1935
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1936-1937
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1938-1939
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1940-1941
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1942-1943
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1944-1946
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1947-1948
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1949-1950
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1951-1952
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1953-1954
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1955-1956
PPCE500mcItinerariesBypass::E500mc_FPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1957-1959
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1960-1962
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1963-1964
PPCE500mcItinerariesBypass::NoBypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1965-1966
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_CR_Bypass, // 1967-1968
PPCE500mcItinerariesBypass::E500mc_CR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1969-1970
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1971-1973
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1974-1975
PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, PPCE500mcItinerariesBypass::E500mc_GPR_Bypass, // 1976-1977
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1978-1980
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1981-1983
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1984-1985
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1986-1988
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 1989-1991
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1992-1993
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1994-1995
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1996-1997
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 1998-2000
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2001-2003
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2004-2006
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2007-2009
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2010-2012
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2013-2015
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2016-2017
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2018-2020
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2021-2023
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2024-2027
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2028-2029
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2030-2033
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2034-2036
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2037-2038
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2039-2040
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2041-2042
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2043-2044
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2045-2046
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2047-2049
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2050-2052
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2053-2055
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2056-2057
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2058-2059
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2060-2061
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2062-2063
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2064-2066
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2067-2068
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass,  0, // 2069-2071
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2072-2073
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2074
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2075
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2076-2077
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2078-2079
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2080-2081
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2082
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, // 2083-2085
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2086-2087
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2088
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2089-2091
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2092-2094
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2095-2097
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2098-2100
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2101-2103
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2104-2106
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2107-2109
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2110-2112
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2113-2114
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2115-2116
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2117-2118
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2119-2120
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2121-2122
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2123-2124
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2125
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, // 2126-2128
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2129
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2130-2132
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2133-2134
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2135-2137
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2138-2140
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2141-2143
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2144-2146
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2147-2149
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2150-2152
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2153-2155
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2156-2158
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2159-2160
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass,  0, // 2161-2163
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2164-2165
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2166
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2167-2168
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2169-2171
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2172-2174
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2175-2177
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2178-2179
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass,  0, // 2180-2182
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2183-2185
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2186-2188
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2189-2191
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2192-2194
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2195-2198
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2199
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2200-2201
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2202-2203
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2204-2206
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2207-2209
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2210-2212
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2213-2215
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2216-2218
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2219-2221
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2222-2224
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2225-2228
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2229-2231
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2232-2234
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2235-2237
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2238-2240
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2241-2243
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2244-2246
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2247-2250
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2251-2252
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2253-2255
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2256-2257
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2258-2259
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2260-2262
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2263-2264
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2265-2266
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2267-2268
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2269-2271
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2272-2273
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2274-2275
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2276-2277
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2278-2280
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2281-2282
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2283-2284
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2285
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2286-2287
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2288-2289
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2290-2291
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2292-2293
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2294
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2295-2297
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2298-2300
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2301-2303
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2304-2306
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2307-2308
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2309-2310
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2311-2313
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2314-2316
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2317-2318
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2319-2320
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2321-2322
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2323-2324
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2325-2326
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2327-2329
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2330-2331
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2332
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2333-2335
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2336-2337
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2338
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2339-2340
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2341
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2342-2343
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2344
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2345-2347
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2348-2350
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2351-2353
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2354-2356
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2357-2359
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2360-2362
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2363-2365
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2366-2368
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass,  0, // 2369-2371
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2372-2374
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2375-2377
PPCE5500ItinerariesBypass::E5500_CR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2378-2379
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2380
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::NoBypass, // 2381-2383
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2384-2386
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2387-2389
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2390-2392
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2393-2395
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2396-2398
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2399
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_CR_Bypass, // 2400-2401
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2402-2404
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2405-2407
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2408-2410
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2411-2413
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2414-2416
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2417-2419
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2420-2422
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2423-2425
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2426-2428
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_FPR_Bypass, // 2429-2431
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2432-2434
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2435-2437
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2438-2440
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2441-2442
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2443-2444
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2445-2446
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2447-2448
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2449-2450
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2451-2452
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2453-2455
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2456-2458
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2459-2460
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2461-2462
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2463-2464
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2465-2466
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2467-2469
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2470-2471
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2472-2473
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2474-2475
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2476-2477
PPCE5500ItinerariesBypass::NoBypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2478-2479
PPCE5500ItinerariesBypass::E5500_FPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2480-2482
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2483-2485
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2486
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2487
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2488-2490
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2491-2492
PPCE5500ItinerariesBypass::E5500_GPR_Bypass, PPCE5500ItinerariesBypass::E5500_GPR_Bypass, // 2493-2494
 0,  0,  0, // 2495-2497
 0,  0,  0, // 2498-2500
 0,  0,  0, // 2501-2503
 0,  0,  0, // 2504-2506
 0,  0,  0, // 2507-2509
 0,  0,  0, // 2510-2512
 0,  0,  0, // 2513-2515
 0,  0,  0, // 2516-2518
 0,  0,  0, // 2519-2521
 0,  0,  0, // 2522-2524
 0,  0,  0, // 2525-2527
 0,  0,  0, // 2528-2530
 0,  0,  0, // 2531-2533
 0,  0,  0, // 2534-2536
 0,  0,  0, // 2537-2539
 0,  0,  0, // 2540-2542
 0,  0,  0, // 2543-2545
 0,  0,  0, // 2546-2548
 0,  0,  0, // 2549-2551
 0,  0,  0,  0, // 2552-2555
 0,  0,  0, // 2556-2558
 0,  0,  0, // 2559-2561
 0,  0,  0,  0, // 2562-2565
 0,  0,  0, // 2566-2568
 0,  0,  0,  0, // 2569-2572
 0,  0,  0,  0, // 2573-2576
 0,  0,  0, // 2577-2579
 0,  0,  0,  0, // 2580-2583
 0,  0,  0,  0, // 2584-2587
 0,  0,  0, // 2588-2590
 0,  0,  0,  0, // 2591-2594
 0,  0,  0,  0, // 2595-2598
 0,  0,  0, // 2599-2601
 0,  0,  0,  0, // 2602-2605
 0,  0,  0,  0, // 2606-2609
 0,  0,  0, // 2610-2612
 0,  0,  0, // 2613-2615
 0,  0,  0, // 2616-2618
 0,  0,  0, // 2619-2621
 0,  0, // 2622-2623
 0,  0, // 2624-2625
 0,  0, // 2626-2627
 0,  0, // 2628-2629
 0,  0,  0, // 2630-2632
 0,  0,  0, // 2633-2635
 0,  0,  0, // 2636-2638
 0,  0,  0, // 2639-2641
 0,  0,  0, // 2642-2644
 0,  0,  0,  0, // 2645-2648
 0,  0,  0, // 2649-2651
 0,  0,  0, // 2652-2654
 0,  0,  0, // 2655-2657
 0,  0,  0, // 2658-2660
 0,  0,  0,  0, // 2661-2664
 0,  0,  0,  0, // 2665-2668
 0,  0,  0, // 2669-2671
 0,  0, // 2672-2673
 0,  0, // 2674-2675
 0,  0,  0, // 2676-2678
 0,  0,  0, // 2679-2681
 0,  0,  0, // 2682-2684
 0,  0,  0, // 2685-2687
 0,  0, // 2688-2689
 0,  0,  0, // 2690-2692
 0,  0, // 2693-2694
 0,  0, // 2695-2696
 0,  0,  0, // 2697-2699
 0,  0,  0, // 2700-2702
 0,  0,  0, // 2703-2705
 0,  0,  0, // 2706-2708
 0,  0,  0, // 2709-2711
 0,  0,  0, // 2712-2714
 0,  0,  0, // 2715-2717
 0,  0,  0, // 2718-2720
 0,  0,  0, // 2721-2723
 0,  0,  0, // 2724-2726
 0,  0,  0, // 2727-2729
 0,  0,  0, // 2730-2732
 0,  0, // 2733-2734
 0,  0,  0, // 2735-2737
 0,  0,  0, // 2738-2740
 0,  0, // 2741-2742
 0,  0,  0, // 2743-2745
 0,  0,  0, // 2746-2748
 0,  0,  0, // 2749-2751
 0,  0,  0, // 2752-2754
 0,  0, // 2755-2756
 0,  0, // 2757-2758
 0,  0,  0, // 2759-2761
 0,  0,  0, // 2762-2764
 0,  0,  0, // 2765-2767
 0,  0,  0, // 2768-2770
 0,  0,  0,  0, // 2771-2774
 0,  0,  0, // 2775-2777
 0,  0, // 2778-2779
 0,  0,  0, // 2780-2782
 0,  0,  0, // 2783-2785
 0,  0,  0, // 2786-2788
 0,  0,  0, // 2789-2791
 0,  0,  0, // 2792-2794
 0,  0,  0, // 2795-2797
 0,  0,  0, // 2798-2800
 0,  0,  0, // 2801-2803
 0,  0,  0, // 2804-2806
 0,  0,  0, // 2807-2809
 0,  0,  0, // 2810-2812
 0,  0,  0, // 2813-2815
 0,  0,  0, // 2816-2818
 0,  0,  0, // 2819-2821
 0,  0,  0,  0, // 2822-2825
 0,  0,  0, // 2826-2828
 0,  0,  0, // 2829-2831
 0,  0,  0, // 2832-2834
 0,  0,  0, // 2835-2837
 0,  0,  0, // 2838-2840
 0,  0,  0, // 2841-2843
 0,  0,  0, // 2844-2846
 0,  0,  0,  0, // 2847-2850
 0,  0,  0, // 2851-2853
 0,  0,  0, // 2854-2856
 0,  0,  0, // 2857-2859
 0,  0,  0, // 2860-2862
 0,  0,  0, // 2863-2865
 0,  0,  0, // 2866-2868
 0,  0,  0, // 2869-2871
 0,  0,  0, // 2872-2874
 0,  0,  0, // 2875-2877
 0,  0,  0, // 2878-2880
 0,  0,  0, // 2881-2883
 0,  0,  0, // 2884-2886
 0,  0,  0, // 2887-2889
 0,  0,  0, // 2890-2892
 0,  0,  0, // 2893-2895
 0,  0,  0, // 2896-2898
 0,  0,  0, // 2899-2901
 0,  0,  0, // 2902-2904
 0,  0,  0, // 2905-2907
 0,  0,  0, // 2908-2910
 0,  0,  0, // 2911-2913
 0,  0,  0,  0, // 2914-2917
 0,  0,  0,  0, // 2918-2921
 0,  0, // 2922-2923
 0,  0,  0, // 2924-2926
 0,  0,  0, // 2927-2929
 0,  0,  0, // 2930-2932
 0,  0,  0, // 2933-2935
 0,  0,  0, // 2936-2938
 0,  0, // 2939-2940
 0,  0,  0, // 2941-2943
 0,  0,  0, // 2944-2946
 0,  0,  0, // 2947-2949
 0,  0,  0, // 2950-2952
 0,  0,  0,  0, // 2953-2956
 0,  0,  0,  0, // 2957-2960
 0,  0,  0, // 2961-2963
 0,  0,  0, // 2964-2966
 0,  0,  0, // 2967-2969
 0,  0,  0, // 2970-2972
 0,  0,  0, // 2973-2975
 0,  0,  0, // 2976-2978
 0,  0,  0, // 2979-2981
 0,  0,  0, // 2982-2984
 0,  0, // 2985-2986
 0,  0,  0, // 2987-2989
 0,  0, // 2990-2991
 0,  0,  0, // 2992-2994
 0,  0,  0, // 2995-2997
 0,  0,  0, // 2998-3000
 0,  0,  0, // 3001-3003
 0,  0,  0, // 3004-3006
 0,  0,  0, // 3007-3009
 0,  0,  0, // 3010-3012
 0,  0,  0, // 3013-3015
 0,  0, // 3016-3017
 0,  0,  0, // 3018-3020
 0,  0,  0, // 3021-3023
 0,  0,  0, // 3024-3026
 0,  0,  0, // 3027-3029
 0,  0,  0, // 3030-3032
 0,  0,  0, // 3033-3035
 0,  0,  0, // 3036-3038
 0,  0,  0, // 3039-3041
 0,  0, // 3042-3043
 0,  0,  0, // 3044-3046
 0,  0,  0, // 3047-3049
 0,  0,  0, // 3050-3052
 0,  0,  0, // 3053-3055
 0,  0,  0, // 3056-3058
 0,  0,  0, // 3059-3061
 0,  0,  0, // 3062-3064
 0,  0,  0, // 3065-3067
 0,  0,  0, // 3068-3070
 0,  0,  0, // 3071-3073
 0,  0,  0, // 3074-3076
 0,  0,  0, // 3077-3079
 0,  0,  0, // 3080-3082
 0,  0,  0, // 3083-3085
 0,  0,  0, // 3086-3088
 0,  0,  0, // 3089-3091
 0,  0,  0, // 3092-3094
 0,  0,  0, // 3095-3097
 0,  0,  0,  0, // 3098-3101
 0,  0,  0,  0, // 3102-3105
 0,  0,  0,  0, // 3106-3109
 0,  0,  0,  0, // 3110-3113
 0,  0,  0,  0, // 3114-3117
 0,  0,  0,  0, // 3118-3121
 0,  0,  0,  0, // 3122-3125
 0,  0,  0,  0, // 3126-3129
 0,  0,  0,  0, // 3130-3133
 0,  0,  0,  0, // 3134-3137
 0,  0,  0,  0, // 3138-3141
 0,  0,  0, // 3142-3144
 0,  0,  0, // 3145-3147
 0,  0,  0, // 3148-3150
 0,  0,  0, // 3151-3153
 0,  0,  0, // 3154-3156
 0,  0,  0, // 3157-3159
 0,  0,  0, // 3160-3162
 0,  0,  0, // 3163-3165
 0,  0,  0, // 3166-3168
 0,  0,  0, // 3169-3171
 0,  0,  0, // 3172-3174
 0,  0,  0, // 3175-3177
 0,  0,  0, // 3178-3180
 0,  0,  0, // 3181-3183
 0,  0,  0, // 3184-3186
 0,  0, // 3187-3188
 0,  0,  0, // 3189-3191
 0,  0,  0, // 3192-3194
 0,  0,  0, // 3195-3197
 0,  0,  0, // 3198-3200
 0,  0,  0, // 3201-3203
 0,  0,  0, // 3204-3206
 0,  0,  0, // 3207-3209
 0,  0,  0, // 3210-3212
 0,  0,  0, // 3213-3215
 0,  0,  0, // 3216-3218
 0,  0,  0, // 3219-3221
 0,  0,  0, // 3222-3224
 0,  0,  0, // 3225-3227
 0,  0,  0, // 3228-3230
 0,  0,  0, // 3231-3233
 0,  0,  0, // 3234-3236
 0,  0,  0, // 3237-3239
 0,  0,  0, // 3240-3242
 0,  0,  0, // 3243-3245
 0,  0,  0, // 3246-3248
 0,  0,  0,  0, // 3249-3252
 0,  0,  0, // 3253-3255
 0,  0,  0, // 3256-3258
 0,  0,  0,  0, // 3259-3262
 0,  0,  0, // 3263-3265
 0,  0,  0,  0, // 3266-3269
 0,  0,  0,  0, // 3270-3273
 0,  0,  0, // 3274-3276
 0,  0,  0,  0, // 3277-3280
 0,  0,  0,  0, // 3281-3284
 0,  0,  0, // 3285-3287
 0,  0,  0,  0, // 3288-3291
 0,  0,  0,  0, // 3292-3295
 0,  0,  0, // 3296-3298
 0,  0,  0,  0, // 3299-3302
 0,  0,  0,  0, // 3303-3306
 0,  0,  0, // 3307-3309
 0,  0,  0, // 3310-3312
 0,  0,  0, // 3313-3315
 0,  0,  0, // 3316-3318
 0,  0, // 3319-3320
 0,  0, // 3321-3322
 0,  0, // 3323-3324
 0,  0, // 3325-3326
 0,  0,  0, // 3327-3329
 0,  0,  0, // 3330-3332
 0,  0,  0, // 3333-3335
 0,  0,  0, // 3336-3338
 0,  0,  0, // 3339-3341
 0,  0,  0,  0, // 3342-3345
 0,  0,  0, // 3346-3348
 0,  0,  0, // 3349-3351
 0,  0,  0, // 3352-3354
 0,  0,  0, // 3355-3357
 0,  0,  0,  0, // 3358-3361
 0,  0,  0,  0, // 3362-3365
 0,  0,  0, // 3366-3368
 0,  0,  0, // 3369-3371
 0,  0, // 3372-3373
 0,  0, // 3374-3375
 0,  0,  0, // 3376-3378
 0,  0,  0, // 3379-3381
 0,  0,  0, // 3382-3384
 0,  0,  0, // 3385-3387
 0,  0, // 3388-3389
 0,  0,  0, // 3390-3392
 0,  0, // 3393-3394
 0,  0, // 3395-3396
 0,  0,  0, // 3397-3399
 0,  0,  0, // 3400-3402
 0,  0,  0, // 3403-3405
 0,  0,  0, // 3406-3408
 0,  0,  0, // 3409-3411
 0,  0,  0, // 3412-3414
 0,  0,  0, // 3415-3417
 0,  0,  0, // 3418-3420
 0,  0,  0, // 3421-3423
 0,  0,  0, // 3424-3426
 0,  0,  0, // 3427-3429
 0,  0,  0, // 3430-3432
 0,  0, // 3433-3434
 0,  0,  0, // 3435-3437
 0,  0,  0, // 3438-3440
 0,  0, // 3441-3442
 0,  0,  0, // 3443-3445
 0,  0,  0, // 3446-3448
 0,  0,  0, // 3449-3451
 0,  0,  0, // 3452-3454
 0,  0, // 3455-3456
 0,  0, // 3457-3458
 0,  0,  0, // 3459-3461
 0,  0,  0, // 3462-3464
 0,  0,  0, // 3465-3467
 0,  0,  0, // 3468-3470
 0,  0,  0,  0, // 3471-3474
 0,  0,  0, // 3475-3477
 0,  0, // 3478-3479
 0,  0,  0, // 3480-3482
 0,  0,  0, // 3483-3485
 0,  0,  0, // 3486-3488
 0,  0,  0, // 3489-3491
 0,  0,  0, // 3492-3494
 0,  0,  0, // 3495-3497
 0,  0,  0, // 3498-3500
 0,  0,  0, // 3501-3503
 0,  0,  0, // 3504-3506
 0,  0,  0, // 3507-3509
 0,  0,  0, // 3510-3512
 0,  0,  0, // 3513-3515
 0,  0,  0, // 3516-3518
 0,  0,  0, // 3519-3521
 0,  0,  0,  0, // 3522-3525
 0,  0,  0, // 3526-3528
 0,  0,  0, // 3529-3531
 0,  0,  0, // 3532-3534
 0,  0,  0, // 3535-3537
 0,  0,  0, // 3538-3540
 0,  0,  0, // 3541-3543
 0,  0,  0, // 3544-3546
 0,  0,  0,  0, // 3547-3550
 0,  0,  0, // 3551-3553
 0,  0,  0, // 3554-3556
 0,  0,  0, // 3557-3559
 0,  0,  0, // 3560-3562
 0,  0,  0, // 3563-3565
 0,  0,  0, // 3566-3568
 0,  0,  0, // 3569-3571
 0,  0,  0, // 3572-3574
 0,  0,  0, // 3575-3577
 0,  0,  0, // 3578-3580
 0,  0,  0, // 3581-3583
 0,  0,  0, // 3584-3586
 0,  0,  0, // 3587-3589
 0,  0,  0, // 3590-3592
 0,  0,  0, // 3593-3595
 0,  0,  0, // 3596-3598
 0,  0,  0, // 3599-3601
 0,  0,  0, // 3602-3604
 0,  0,  0, // 3605-3607
 0,  0,  0, // 3608-3610
 0,  0,  0, // 3611-3613
 0,  0,  0,  0, // 3614-3617
 0,  0,  0,  0, // 3618-3621
 0,  0, // 3622-3623
 0,  0,  0, // 3624-3626
 0,  0,  0, // 3627-3629
 0,  0,  0, // 3630-3632
 0,  0,  0, // 3633-3635
 0,  0,  0, // 3636-3638
 0,  0, // 3639-3640
 0,  0,  0, // 3641-3643
 0,  0,  0, // 3644-3646
 0,  0,  0, // 3647-3649
 0,  0,  0, // 3650-3652
 0,  0,  0,  0, // 3653-3656
 0,  0,  0,  0, // 3657-3660
 0,  0,  0, // 3661-3663
 0,  0,  0, // 3664-3666
 0,  0,  0, // 3667-3669
 0,  0,  0, // 3670-3672
 0,  0,  0, // 3673-3675
 0,  0,  0, // 3676-3678
 0,  0,  0, // 3679-3681
 0,  0,  0, // 3682-3684
 0,  0,  0, // 3685-3687
 0,  0, // 3688-3689
 0,  0,  0, // 3690-3692
 0,  0, // 3693-3694
 0,  0,  0, // 3695-3697
 0,  0,  0, // 3698-3700
 0,  0,  0, // 3701-3703
 0,  0,  0, // 3704-3706
 0,  0,  0, // 3707-3709
 0,  0,  0, // 3710-3712
 0,  0,  0, // 3713-3715
 0,  0,  0, // 3716-3718
 0,  0, // 3719-3720
 0,  0,  0, // 3721-3723
 0,  0,  0, // 3724-3726
 0,  0,  0, // 3727-3729
 0,  0,  0, // 3730-3732
 0,  0,  0, // 3733-3735
 0,  0,  0, // 3736-3738
 0,  0,  0, // 3739-3741
 0,  0,  0, // 3742-3744
 0,  0, // 3745-3746
 0,  0,  0, // 3747-3749
 0,  0,  0, // 3750-3752
 0,  0,  0, // 3753-3755
 0,  0,  0, // 3756-3758
 0,  0,  0, // 3759-3761
 0,  0,  0, // 3762-3764
 0,  0,  0, // 3765-3767
 0,  0,  0, // 3768-3770
 0,  0,  0, // 3771-3773
 0,  0,  0, // 3774-3776
 0,  0,  0, // 3777-3779
 0,  0,  0, // 3780-3782
 0,  0,  0, // 3783-3785
 0,  0,  0, // 3786-3788
 0,  0,  0, // 3789-3791
 0,  0,  0, // 3792-3794
 0,  0,  0, // 3795-3797
 0,  0,  0, // 3798-3800
 0,  0,  0,  0, // 3801-3804
 0,  0,  0,  0, // 3805-3808
 0,  0,  0,  0, // 3809-3812
 0,  0,  0,  0, // 3813-3816
 0,  0,  0,  0, // 3817-3820
 0,  0,  0,  0, // 3821-3824
 0,  0,  0,  0, // 3825-3828
 0,  0,  0,  0, // 3829-3832
 0,  0,  0,  0, // 3833-3836
 0,  0,  0,  0, // 3837-3840
 0,  0,  0,  0, // 3841-3844
 0,  0,  0, // 3845-3847
 0,  0,  0, // 3848-3850
 0,  0,  0, // 3851-3853
 0,  0,  0, // 3854-3856
 0,  0,  0, // 3857-3859
 0,  0,  0, // 3860-3862
 0,  0,  0, // 3863-3865
 0,  0,  0, // 3866-3868
 0,  0,  0, // 3869-3871
 0,  0,  0, // 3872-3874
 0,  0,  0, // 3875-3877
 0,  0,  0, // 3878-3880
 0,  0,  0, // 3881-3883
 0,  0,  0, // 3884-3886
 0,  0,  0, // 3887-3889
 0,  0, // 3890-3891
 0,  0,  0, // 3892-3894
 0 // End bypass tables
};

static const llvm::InstrItinerary PPC440Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 1, 6, 0, 0 }, // 1 IIC_LdStSync
  { 1, 6, 11, 1, 4 }, // 2 IIC_IntSimple
  { 1, 6, 11, 4, 7 }, // 3 IIC_IntGeneral
  { 1, 11, 16, 7, 9 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 6, 11, 9, 12 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 11, 16, 12, 15 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 16, 21, 15, 17 }, // 12 IIC_LdStDCBF
  { 1, 21, 26, 17, 20 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 26, 31, 20, 23 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 31, 40, 23, 26 }, // 17 IIC_FPAddSub
  { 1, 40, 49, 26, 29 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 31, 40, 29, 32 }, // 20 IIC_FPCompare
  { 1, 31, 40, 32, 35 }, // 21 IIC_FPGeneral
  { 0, 0, 0, 0, 0 }, // 22 IIC_VecGeneral
  { 0, 0, 0, 0, 0 }, // 23 IIC_VecComplex
  { 1, 21, 26, 35, 38 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 49, 58, 38, 41 }, // 26 IIC_FPDivS
  { 1, 31, 40, 41, 45 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 16, 21, 45, 48 }, // 30 IIC_LdStICBI
  { 1, 6, 11, 48, 52 }, // 31 IIC_IntISEL
  { 1, 58, 74, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 74, 81, 52, 55 }, // 33 IIC_LdStLWARX
  { 1, 21, 26, 55, 59 }, // 34 IIC_LdStLoadUpd
  { 1, 21, 26, 59, 63 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 21, 26, 63, 66 }, // 39 IIC_LdStLFD
  { 1, 16, 21, 66, 70 }, // 40 IIC_LdStLFDU
  { 1, 16, 21, 70, 74 }, // 41 IIC_LdStLFDUX
  { 1, 16, 21, 74, 77 }, // 42 IIC_LdStLHA
  { 1, 16, 21, 77, 80 }, // 43 IIC_LdStLHAU
  { 1, 16, 21, 80, 83 }, // 44 IIC_LdStLHAUX
  { 1, 16, 21, 83, 86 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 11, 16, 86, 89 }, // 48 IIC_BrMCR
  { 1, 11, 16, 89, 92 }, // 49 IIC_BrMCRX
  { 1, 11, 16, 92, 94 }, // 50 IIC_SprMFCR
  { 1, 81, 86, 94, 96 }, // 51 IIC_SprMFSPR
  { 1, 11, 16, 96, 99 }, // 52 IIC_IntMFFS
  { 1, 11, 16, 99, 101 }, // 53 IIC_SprMFMSR
  { 0, 0, 0, 0, 0 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 1, 11, 16, 101, 103 }, // 56 IIC_SprMFSR
  { 1, 81, 86, 103, 105 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 81, 86, 105, 107 }, // 59 IIC_SprMTSPR
  { 1, 11, 16, 107, 110 }, // 60 IIC_IntMTFSB0
  { 1, 11, 16, 110, 112 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 1, 81, 86, 112, 114 }, // 64 IIC_SprMTSR
  { 1, 11, 16, 114, 117 }, // 65 IIC_IntMulHW
  { 1, 11, 16, 117, 120 }, // 66 IIC_IntMulHWU
  { 1, 11, 16, 120, 123 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 1, 16, 21, 123, 126 }, // 69 IIC_LdStSTFD
  { 1, 16, 21, 126, 130 }, // 70 IIC_LdStSTFDU
  { 1, 11, 16, 130, 132 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 6, 11, 132, 135 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 6, 11, 135, 138 }, // 83 IIC_IntShift
  { 1, 74, 81, 138, 141 }, // 84 IIC_LdStSTWCX
  { 1, 21, 26, 141, 145 }, // 85 IIC_LdStSTU
  { 1, 21, 26, 145, 149 }, // 86 IIC_LdStSTUX
  { 1, 21, 26, 149, 152 }, // 87 IIC_LdStSTD
  { 1, 74, 81, 152, 155 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 11, 16, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 11, 16, 155, 157 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 0, 0, 0, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 81, 86, 157, 159 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 11, 16, 159, 162 }, // 101 MTFSB0_MTFSB1
  { 1, 11, 16, 162, 165 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 6, 11, 165, 168 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 11, 16, 168, 170 }, // 105 TW_TWI
  { 1, 31, 40, 170, 173 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 31, 40, 173, 176 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 6, 11, 176, 179 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 0, 0, 0, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 6, 11, 179, 182 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 6, 11, 182, 185 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 6, 11, 185, 188 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 11, 16, 188, 191 }, // 118 MCRF
  { 1, 11, 16, 191, 194 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 11, 16, 194, 196 }, // 121 RFEBB
  { 1, 81, 86, 196, 198 }, // 122 TBEGIN_TRECHKPT
  { 1, 21, 26, 198, 201 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 6, 11, 201, 204 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 0, 0, 0, 0, 0 }, // 127 MFOCRF_MFOCRF8
  { 1, 11, 16, 204, 207 }, // 128 MTOCRF_MTOCRF8
  { 1, 11, 16, 207, 210 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 6, 11, 210, 213 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 31, 40, 213, 216 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 6, 11, 216, 219 }, // 132 SRAW_SRAWI
  { 1, 6, 11, 219, 223 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 81, 86, 223, 225 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 21, 26, 225, 228 }, // 136 MFVSCR
  { 1, 21, 26, 228, 231 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 0, 0, 0, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 31, 40, 231, 234 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 0, 0, 0, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 11, 16, 234, 237 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 11, 16, 237, 240 }, // 146 MULHDU_MULHWU
  { 1, 11, 16, 240, 243 }, // 147 MULLI_MULLI8
  { 1, 31, 40, 243, 246 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 31, 40, 246, 249 }, // 149 FADD_FSUB
  { 1, 31, 40, 249, 253 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 31, 40, 253, 256 }, // 152 FSELD_rec_FSELS_rec
  { 1, 11, 16, 256, 259 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 11, 16, 259, 262 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 31, 40, 262, 265 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 31, 40, 265, 268 }, // 157 FADD_rec_FSUB_rec
  { 1, 31, 40, 268, 272 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 21, 26, 272, 275 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 0, 0, 0, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 31, 40, 275, 278 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 21, 26, 278, 281 }, // 171 LXVL_LXVLL
  { 1, 21, 26, 281, 284 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 21, 26, 284, 287 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 16, 21, 287, 289 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 16, 21, 289, 292 }, // 178 ICBI_ICBIEP
  { 1, 21, 26, 292, 295 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 74, 81, 295, 298 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 58, 74, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 11, 16, 0, 0 }, // 186 TLBSYNC
  { 1, 1, 6, 0, 0 }, // 187 SYNC
  { 1, 16, 21, 298, 301 }, // 188 LMW
  { 1, 21, 26, 301, 304 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 21, 26, 304, 308 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 21, 26, 308, 312 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 81, 86, 312, 314 }, // 198 TEND
  { 1, 74, 81, 314, 317 }, // 199 STBCX_STHCX_STWCX
  { 1, 74, 81, 317, 320 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 16, 21, 320, 323 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 81, 86, 323, 325 }, // 205 TCHECK
  { 1, 21, 26, 325, 328 }, // 206 LFIWAX
  { 1, 21, 26, 328, 331 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 21, 26, 331, 334 }, // 209 LFSX_LFS
  { 1, 21, 26, 334, 337 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 16, 21, 337, 340 }, // 212 LHAU_LHAU8
  { 1, 16, 21, 340, 343 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 21, 26, 343, 346 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 16, 21, 346, 349 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 21, 26, 349, 352 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 21, 26, 352, 355 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 21, 26, 355, 358 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 16, 21, 358, 361 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 21, 26, 361, 364 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 16, 21, 364, 367 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 21, 26, 367, 370 }, // 225 STXVL_STXVLL
  { 1, 81, 86, 370, 372 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 6, 11, 372, 375 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 81, 86, 375, 377 }, // 230 MFTB_MFTB8
  { 1, 81, 86, 377, 379 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 11, 16, 379, 381 }, // 232 MFMSR
  { 1, 11, 16, 381, 383 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 81, 86, 383, 385 }, // 235 MTSPR_MTSPR8
  { 1, 26, 31, 385, 388 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 26, 31, 388, 391 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 26, 31, 391, 394 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 26, 31, 394, 397 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 11, 16, 397, 400 }, // 244 MTCRF_MTCRF8
  { 1, 6, 11, 400, 403 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 31, 40, 403, 406 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 11, 16, 406, 409 }, // 247 MCRFS
  { 1, 11, 16, 409, 412 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 11, 16, 412, 415 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 6, 11, 415, 418 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 6, 11, 418, 421 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 6, 11, 421, 424 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 11, 16, 424, 427 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 11, 16, 427, 429 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 40, 49, 429, 432 }, // 259 FDIV
  { 1, 40, 49, 432, 435 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 40, 49, 435, 438 }, // 269 XSDIVDP
  { 1, 49, 58, 438, 441 }, // 270 FDIVS
  { 1, 49, 58, 441, 444 }, // 271 FDIVS_rec
  { 1, 49, 58, 444, 447 }, // 272 XSDIVSP
  { 1, 49, 58, 447, 450 }, // 273 XVDIVSP
  { 1, 40, 49, 450, 453 }, // 274 XVDIVDP
  { 1, 16, 21, 453, 457 }, // 275 LFSU
  { 1, 16, 21, 457, 461 }, // 276 LFSUX
  { 1, 16, 21, 461, 465 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 21, 26, 465, 469 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 21, 26, 469, 473 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 21, 26, 473, 477 }, // 280 LBZU_LBZU8
  { 1, 21, 26, 477, 481 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 1, 16, 21, 481, 485 }, // 284 LFDU
  { 1, 16, 21, 485, 489 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 11, 16, 489, 491 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 11, 16, 491, 493 }, // 288 ADDPCIS
  { 1, 21, 26, 493, 496 }, // 289 LDAT_LWAT
  { 1, 21, 26, 496, 499 }, // 290 STDAT_STWAT
  { 1, 6, 11, 499, 502 }, // 291 BRINC
  { 0, 0, 0, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 0, 0, 0, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 21, 26, 502, 505 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 21, 26, 505, 508 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 11, 16, 508, 510 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 1, 11, 16, 510, 512 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 21, 26, 512, 515 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 21, 26, 515, 518 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 21, 26, 518, 521 }, // 301 LFDEPX
  { 1, 16, 21, 521, 524 }, // 302 STFDEPX
  { 1, 11, 16, 524, 526 }, // 303 MFSR_MFSRIN
  { 1, 81, 86, 526, 528 }, // 304 MTSR_MTSRIN
  { 1, 81, 86, 528, 530 }, // 305 MFDCR
  { 1, 81, 86, 530, 532 }, // 306 MTDCR
  { 1, 6, 11, 532, 535 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 11, 16, 535, 537 }, // 309 WRTEE_WRTEEI
  { 1, 1, 6, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 16, 21, 537, 539 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary G3Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 86, 87, 0, 0 }, // 1 IIC_LdStSync
  { 1, 87, 88, 0, 0 }, // 2 IIC_IntSimple
  { 1, 87, 88, 0, 0 }, // 3 IIC_IntGeneral
  { 1, 88, 89, 0, 0 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 87, 88, 0, 0 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 89, 90, 0, 0 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 86, 87, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 90, 91, 0, 0 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 91, 92, 0, 0 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 92, 93, 0, 0 }, // 17 IIC_FPAddSub
  { 1, 93, 94, 0, 0 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 92, 93, 0, 0 }, // 20 IIC_FPCompare
  { 1, 92, 93, 0, 0 }, // 21 IIC_FPGeneral
  { 0, 0, 0, 0, 0 }, // 22 IIC_VecGeneral
  { 0, 0, 0, 0, 0 }, // 23 IIC_VecComplex
  { 1, 90, 91, 0, 0 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 94, 95, 0, 0 }, // 26 IIC_FPDivS
  { 1, 95, 96, 0, 0 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 86, 87, 0, 0 }, // 30 IIC_LdStICBI
  { 0, 0, 0, 0, 0 }, // 31 IIC_IntISEL
  { 1, 96, 97, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 86, 87, 0, 0 }, // 33 IIC_LdStLWARX
  { 1, 90, 91, 0, 0 }, // 34 IIC_LdStLoadUpd
  { 1, 90, 91, 0, 0 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 90, 91, 0, 0 }, // 39 IIC_LdStLFD
  { 1, 90, 91, 0, 0 }, // 40 IIC_LdStLFDU
  { 1, 90, 91, 0, 0 }, // 41 IIC_LdStLFDUX
  { 1, 90, 91, 0, 0 }, // 42 IIC_LdStLHA
  { 1, 90, 91, 0, 0 }, // 43 IIC_LdStLHAU
  { 1, 90, 91, 0, 0 }, // 44 IIC_LdStLHAUX
  { 1, 97, 98, 0, 0 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 89, 90, 0, 0 }, // 48 IIC_BrMCR
  { 1, 89, 90, 0, 0 }, // 49 IIC_BrMCRX
  { 1, 89, 90, 0, 0 }, // 50 IIC_SprMFCR
  { 1, 98, 99, 0, 0 }, // 51 IIC_SprMFSPR
  { 1, 92, 93, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 89, 90, 0, 0 }, // 53 IIC_SprMFMSR
  { 0, 0, 0, 0, 0 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 1, 98, 99, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 98, 99, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 96, 97, 0, 0 }, // 59 IIC_SprMTSPR
  { 1, 99, 100, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 89, 90, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 1, 96, 97, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 100, 101, 0, 0 }, // 65 IIC_IntMulHW
  { 1, 101, 102, 0, 0 }, // 66 IIC_IntMulHWU
  { 1, 102, 103, 0, 0 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 1, 90, 91, 0, 0 }, // 69 IIC_LdStSTFD
  { 1, 90, 91, 0, 0 }, // 70 IIC_LdStSTFDU
  { 1, 96, 97, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 87, 88, 0, 0 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 87, 88, 0, 0 }, // 83 IIC_IntShift
  { 1, 103, 104, 0, 0 }, // 84 IIC_LdStSTWCX
  { 1, 90, 91, 0, 0 }, // 85 IIC_LdStSTU
  { 1, 90, 91, 0, 0 }, // 86 IIC_LdStSTUX
  { 0, 0, 0, 0, 0 }, // 87 IIC_LdStSTD
  { 0, 0, 0, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 98, 99, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 104, 105, 0, 0 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 0, 0, 0, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 96, 97, 0, 0 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 99, 100, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 1, 92, 93, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 87, 88, 0, 0 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 104, 105, 0, 0 }, // 105 TW_TWI
  { 1, 92, 93, 0, 0 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 92, 93, 0, 0 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 87, 88, 0, 0 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 0, 0, 0, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 87, 88, 0, 0 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 87, 88, 0, 0 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 87, 88, 0, 0 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 89, 90, 0, 0 }, // 118 MCRF
  { 1, 89, 90, 0, 0 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 88, 89, 0, 0 }, // 121 RFEBB
  { 1, 96, 97, 0, 0 }, // 122 TBEGIN_TRECHKPT
  { 1, 90, 91, 0, 0 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 87, 88, 0, 0 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 0, 0, 0, 0, 0 }, // 127 MFOCRF_MFOCRF8
  { 1, 89, 90, 0, 0 }, // 128 MTOCRF_MTOCRF8
  { 1, 89, 90, 0, 0 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 87, 88, 0, 0 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 92, 93, 0, 0 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 87, 88, 0, 0 }, // 132 SRAW_SRAWI
  { 0, 0, 0, 0, 0 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 96, 97, 0, 0 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 90, 91, 0, 0 }, // 136 MFVSCR
  { 1, 90, 91, 0, 0 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 0, 0, 0, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 92, 93, 0, 0 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 0, 0, 0, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 100, 101, 0, 0 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 101, 102, 0, 0 }, // 146 MULHDU_MULHWU
  { 1, 102, 103, 0, 0 }, // 147 MULLI_MULLI8
  { 1, 92, 93, 0, 0 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 92, 93, 0, 0 }, // 149 FADD_FSUB
  { 1, 95, 96, 0, 0 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 92, 93, 0, 0 }, // 152 FSELD_rec_FSELS_rec
  { 1, 101, 102, 0, 0 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 100, 101, 0, 0 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 92, 93, 0, 0 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 92, 93, 0, 0 }, // 157 FADD_rec_FSUB_rec
  { 1, 95, 96, 0, 0 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 90, 91, 0, 0 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 0, 0, 0, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 92, 93, 0, 0 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 90, 91, 0, 0 }, // 171 LXVL_LXVLL
  { 1, 90, 91, 0, 0 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 90, 91, 0, 0 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 86, 87, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 86, 87, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 90, 91, 0, 0 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 86, 87, 0, 0 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 96, 97, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 98, 99, 0, 0 }, // 186 TLBSYNC
  { 1, 86, 87, 0, 0 }, // 187 SYNC
  { 1, 97, 98, 0, 0 }, // 188 LMW
  { 1, 90, 91, 0, 0 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 90, 91, 0, 0 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 90, 91, 0, 0 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 96, 97, 0, 0 }, // 198 TEND
  { 1, 103, 104, 0, 0 }, // 199 STBCX_STHCX_STWCX
  { 0, 0, 0, 0, 0 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 90, 91, 0, 0 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 96, 97, 0, 0 }, // 205 TCHECK
  { 1, 90, 91, 0, 0 }, // 206 LFIWAX
  { 1, 90, 91, 0, 0 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 90, 91, 0, 0 }, // 209 LFSX_LFS
  { 1, 90, 91, 0, 0 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 90, 91, 0, 0 }, // 212 LHAU_LHAU8
  { 1, 90, 91, 0, 0 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 90, 91, 0, 0 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 90, 91, 0, 0 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 90, 91, 0, 0 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 0, 0, 0, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 90, 91, 0, 0 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 97, 98, 0, 0 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 90, 91, 0, 0 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 90, 91, 0, 0 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 90, 91, 0, 0 }, // 225 STXVL_STXVLL
  { 1, 96, 97, 0, 0 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 87, 88, 0, 0 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 98, 99, 0, 0 }, // 230 MFTB_MFTB8
  { 1, 98, 99, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 89, 90, 0, 0 }, // 232 MFMSR
  { 1, 89, 90, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 96, 97, 0, 0 }, // 235 MTSPR_MTSPR8
  { 1, 91, 92, 0, 0 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 91, 92, 0, 0 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 91, 92, 0, 0 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 91, 92, 0, 0 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 89, 90, 0, 0 }, // 244 MTCRF_MTCRF8
  { 1, 87, 88, 0, 0 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 92, 93, 0, 0 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 89, 90, 0, 0 }, // 247 MCRFS
  { 1, 92, 93, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 99, 100, 0, 0 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 87, 88, 0, 0 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 87, 88, 0, 0 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 87, 88, 0, 0 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 92, 93, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 89, 90, 0, 0 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 93, 94, 0, 0 }, // 259 FDIV
  { 1, 93, 94, 0, 0 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 93, 94, 0, 0 }, // 269 XSDIVDP
  { 1, 94, 95, 0, 0 }, // 270 FDIVS
  { 1, 94, 95, 0, 0 }, // 271 FDIVS_rec
  { 1, 94, 95, 0, 0 }, // 272 XSDIVSP
  { 1, 94, 95, 0, 0 }, // 273 XVDIVSP
  { 1, 93, 94, 0, 0 }, // 274 XVDIVDP
  { 1, 90, 91, 0, 0 }, // 275 LFSU
  { 1, 90, 91, 0, 0 }, // 276 LFSUX
  { 1, 90, 91, 0, 0 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 90, 91, 0, 0 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 90, 91, 0, 0 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 90, 91, 0, 0 }, // 280 LBZU_LBZU8
  { 1, 90, 91, 0, 0 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 1, 90, 91, 0, 0 }, // 284 LFDU
  { 1, 90, 91, 0, 0 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 88, 89, 0, 0 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 88, 89, 0, 0 }, // 288 ADDPCIS
  { 1, 90, 91, 0, 0 }, // 289 LDAT_LWAT
  { 1, 90, 91, 0, 0 }, // 290 STDAT_STWAT
  { 1, 87, 88, 0, 0 }, // 291 BRINC
  { 0, 0, 0, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 0, 0, 0, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 90, 91, 0, 0 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 90, 91, 0, 0 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 88, 89, 0, 0 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 1, 96, 97, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 90, 91, 0, 0 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 90, 91, 0, 0 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 90, 91, 0, 0 }, // 301 LFDEPX
  { 1, 90, 91, 0, 0 }, // 302 STFDEPX
  { 1, 98, 99, 0, 0 }, // 303 MFSR_MFSRIN
  { 1, 96, 97, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 98, 99, 0, 0 }, // 305 MFDCR
  { 1, 96, 97, 0, 0 }, // 306 MTDCR
  { 1, 87, 88, 0, 0 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 89, 90, 0, 0 }, // 309 WRTEE_WRTEEI
  { 1, 86, 87, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 86, 87, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary G4Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 105, 106, 0, 0 }, // 1 IIC_LdStSync
  { 1, 106, 107, 0, 0 }, // 2 IIC_IntSimple
  { 1, 106, 107, 0, 0 }, // 3 IIC_IntGeneral
  { 1, 107, 108, 0, 0 }, // 4 IIC_BrB
  { 1, 108, 109, 0, 0 }, // 5 IIC_VecFP
  { 1, 106, 107, 0, 0 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 109, 110, 0, 0 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 110, 111, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 110, 111, 0, 0 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 111, 112, 0, 0 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 112, 113, 0, 0 }, // 17 IIC_FPAddSub
  { 1, 113, 114, 0, 0 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 112, 113, 0, 0 }, // 20 IIC_FPCompare
  { 1, 112, 113, 0, 0 }, // 21 IIC_FPGeneral
  { 1, 114, 115, 0, 0 }, // 22 IIC_VecGeneral
  { 1, 115, 116, 0, 0 }, // 23 IIC_VecComplex
  { 1, 110, 111, 0, 0 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 116, 117, 0, 0 }, // 26 IIC_FPDivS
  { 1, 112, 113, 0, 0 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 110, 111, 0, 0 }, // 30 IIC_LdStICBI
  { 0, 0, 0, 0, 0 }, // 31 IIC_IntISEL
  { 1, 117, 118, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 118, 119, 0, 0 }, // 33 IIC_LdStLWARX
  { 1, 110, 111, 0, 0 }, // 34 IIC_LdStLoadUpd
  { 1, 110, 111, 0, 0 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 110, 111, 0, 0 }, // 39 IIC_LdStLFD
  { 1, 110, 111, 0, 0 }, // 40 IIC_LdStLFDU
  { 1, 110, 111, 0, 0 }, // 41 IIC_LdStLFDUX
  { 1, 110, 111, 0, 0 }, // 42 IIC_LdStLHA
  { 1, 110, 111, 0, 0 }, // 43 IIC_LdStLHAU
  { 1, 110, 111, 0, 0 }, // 44 IIC_LdStLHAUX
  { 1, 119, 120, 0, 0 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 109, 110, 0, 0 }, // 48 IIC_BrMCR
  { 1, 109, 110, 0, 0 }, // 49 IIC_BrMCRX
  { 1, 109, 110, 0, 0 }, // 50 IIC_SprMFCR
  { 1, 120, 121, 0, 0 }, // 51 IIC_SprMFSPR
  { 1, 121, 122, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 109, 110, 0, 0 }, // 53 IIC_SprMFMSR
  { 0, 0, 0, 0, 0 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 1, 120, 121, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 109, 110, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 117, 118, 0, 0 }, // 59 IIC_SprMTSPR
  { 1, 121, 122, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 109, 110, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 1, 117, 118, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 122, 123, 0, 0 }, // 65 IIC_IntMulHW
  { 1, 123, 124, 0, 0 }, // 66 IIC_IntMulHWU
  { 1, 124, 125, 0, 0 }, // 67 IIC_IntMulLI
  { 1, 125, 126, 0, 0 }, // 68 IIC_VecPerm
  { 1, 110, 111, 0, 0 }, // 69 IIC_LdStSTFD
  { 1, 110, 111, 0, 0 }, // 70 IIC_LdStSTFDU
  { 1, 117, 118, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 106, 107, 0, 0 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 106, 107, 0, 0 }, // 83 IIC_IntShift
  { 1, 126, 127, 0, 0 }, // 84 IIC_LdStSTWCX
  { 1, 110, 111, 0, 0 }, // 85 IIC_LdStSTU
  { 1, 110, 111, 0, 0 }, // 86 IIC_LdStSTUX
  { 0, 0, 0, 0, 0 }, // 87 IIC_LdStSTD
  { 0, 0, 0, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 127, 128, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 128, 129, 0, 0 }, // 95 IIC_IntTrapW
  { 1, 114, 115, 0, 0 }, // 96 IIC_VecFPCompare
  { 1, 114, 115, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 1, 108, 109, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 1, 125, 126, 0, 0 }, // 99 XXSEL
  { 1, 117, 118, 0, 0 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 121, 122, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 1, 121, 122, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 106, 107, 0, 0 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 128, 129, 0, 0 }, // 105 TW_TWI
  { 1, 112, 113, 0, 0 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 1, 108, 109, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 1, 108, 109, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 112, 113, 0, 0 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 106, 107, 0, 0 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 114, 115, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 106, 107, 0, 0 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 106, 107, 0, 0 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 106, 107, 0, 0 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 109, 110, 0, 0 }, // 118 MCRF
  { 1, 109, 110, 0, 0 }, // 119 MCRXRX
  { 1, 108, 109, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 107, 108, 0, 0 }, // 121 RFEBB
  { 1, 117, 118, 0, 0 }, // 122 TBEGIN_TRECHKPT
  { 1, 110, 111, 0, 0 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 106, 107, 0, 0 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 0, 0, 0, 0, 0 }, // 127 MFOCRF_MFOCRF8
  { 1, 109, 110, 0, 0 }, // 128 MTOCRF_MTOCRF8
  { 1, 109, 110, 0, 0 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 106, 107, 0, 0 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 112, 113, 0, 0 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 106, 107, 0, 0 }, // 132 SRAW_SRAWI
  { 0, 0, 0, 0, 0 }, // 133 ISEL_ISEL8
  { 1, 108, 109, 0, 0 }, // 134 XSIEXPDP
  { 1, 117, 118, 0, 0 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 110, 111, 0, 0 }, // 136 MFVSCR
  { 1, 110, 111, 0, 0 }, // 137 MTVSCR
  { 1, 114, 115, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 114, 115, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 1, 108, 109, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 112, 113, 0, 0 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 1, 108, 109, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 114, 115, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 122, 123, 0, 0 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 123, 124, 0, 0 }, // 146 MULHDU_MULHWU
  { 1, 124, 125, 0, 0 }, // 147 MULLI_MULLI8
  { 1, 112, 113, 0, 0 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 112, 113, 0, 0 }, // 149 FADD_FSUB
  { 1, 112, 113, 0, 0 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 1, 108, 109, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 112, 113, 0, 0 }, // 152 FSELD_rec_FSELS_rec
  { 1, 123, 124, 0, 0 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 122, 123, 0, 0 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 112, 113, 0, 0 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 112, 113, 0, 0 }, // 157 FADD_rec_FSUB_rec
  { 1, 112, 113, 0, 0 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 1, 108, 109, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 110, 111, 0, 0 }, // 160 LVSL_LVSR
  { 1, 125, 126, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 1, 108, 109, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 114, 115, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 112, 113, 0, 0 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 1, 108, 109, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 1, 108, 109, 0, 0 }, // 166 BCDCTSQ_rec
  { 1, 108, 109, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 1, 108, 109, 0, 0 }, // 168 BCDCFSQ_rec
  { 1, 108, 109, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 1, 108, 109, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 110, 111, 0, 0 }, // 171 LXVL_LXVLL
  { 1, 110, 111, 0, 0 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 110, 111, 0, 0 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 110, 111, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 110, 111, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 110, 111, 0, 0 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 118, 119, 0, 0 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 117, 118, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 127, 128, 0, 0 }, // 186 TLBSYNC
  { 1, 105, 106, 0, 0 }, // 187 SYNC
  { 1, 119, 120, 0, 0 }, // 188 LMW
  { 1, 110, 111, 0, 0 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 110, 111, 0, 0 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 110, 111, 0, 0 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 117, 118, 0, 0 }, // 198 TEND
  { 1, 126, 127, 0, 0 }, // 199 STBCX_STHCX_STWCX
  { 0, 0, 0, 0, 0 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 110, 111, 0, 0 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 117, 118, 0, 0 }, // 205 TCHECK
  { 1, 110, 111, 0, 0 }, // 206 LFIWAX
  { 1, 110, 111, 0, 0 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 110, 111, 0, 0 }, // 209 LFSX_LFS
  { 1, 110, 111, 0, 0 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 110, 111, 0, 0 }, // 212 LHAU_LHAU8
  { 1, 110, 111, 0, 0 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 110, 111, 0, 0 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 110, 111, 0, 0 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 110, 111, 0, 0 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 0, 0, 0, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 110, 111, 0, 0 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 119, 120, 0, 0 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 110, 111, 0, 0 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 110, 111, 0, 0 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 110, 111, 0, 0 }, // 225 STXVL_STXVLL
  { 1, 117, 118, 0, 0 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 106, 107, 0, 0 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 109, 110, 0, 0 }, // 230 MFTB_MFTB8
  { 1, 120, 121, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 109, 110, 0, 0 }, // 232 MFMSR
  { 1, 109, 110, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 117, 118, 0, 0 }, // 235 MTSPR_MTSPR8
  { 1, 111, 112, 0, 0 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 111, 112, 0, 0 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 111, 112, 0, 0 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 111, 112, 0, 0 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 109, 110, 0, 0 }, // 244 MTCRF_MTCRF8
  { 1, 106, 107, 0, 0 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 112, 113, 0, 0 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 109, 110, 0, 0 }, // 247 MCRFS
  { 1, 121, 122, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 121, 122, 0, 0 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 106, 107, 0, 0 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 106, 107, 0, 0 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 106, 107, 0, 0 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 121, 122, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 109, 110, 0, 0 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 113, 114, 0, 0 }, // 259 FDIV
  { 1, 113, 114, 0, 0 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 113, 114, 0, 0 }, // 269 XSDIVDP
  { 1, 116, 117, 0, 0 }, // 270 FDIVS
  { 1, 116, 117, 0, 0 }, // 271 FDIVS_rec
  { 1, 116, 117, 0, 0 }, // 272 XSDIVSP
  { 1, 116, 117, 0, 0 }, // 273 XVDIVSP
  { 1, 113, 114, 0, 0 }, // 274 XVDIVDP
  { 1, 110, 111, 0, 0 }, // 275 LFSU
  { 1, 110, 111, 0, 0 }, // 276 LFSUX
  { 1, 110, 111, 0, 0 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 110, 111, 0, 0 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 110, 111, 0, 0 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 110, 111, 0, 0 }, // 280 LBZU_LBZU8
  { 1, 110, 111, 0, 0 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 1, 110, 111, 0, 0 }, // 284 LFDU
  { 1, 110, 111, 0, 0 }, // 285 LFDUX
  { 1, 108, 109, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 107, 108, 0, 0 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 107, 108, 0, 0 }, // 288 ADDPCIS
  { 1, 110, 111, 0, 0 }, // 289 LDAT_LWAT
  { 1, 110, 111, 0, 0 }, // 290 STDAT_STWAT
  { 1, 106, 107, 0, 0 }, // 291 BRINC
  { 1, 114, 115, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 115, 116, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 110, 111, 0, 0 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 110, 111, 0, 0 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 107, 108, 0, 0 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 1, 117, 118, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 110, 111, 0, 0 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 110, 111, 0, 0 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 110, 111, 0, 0 }, // 301 LFDEPX
  { 1, 110, 111, 0, 0 }, // 302 STFDEPX
  { 1, 120, 121, 0, 0 }, // 303 MFSR_MFSRIN
  { 1, 117, 118, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 120, 121, 0, 0 }, // 305 MFDCR
  { 1, 117, 118, 0, 0 }, // 306 MTDCR
  { 1, 106, 107, 0, 0 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 109, 110, 0, 0 }, // 309 WRTEE_WRTEEI
  { 1, 105, 106, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 110, 111, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary G4PlusItineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 129, 130, 0, 0 }, // 1 IIC_LdStSync
  { 1, 130, 131, 0, 0 }, // 2 IIC_IntSimple
  { 1, 130, 131, 0, 0 }, // 3 IIC_IntGeneral
  { 1, 131, 132, 0, 0 }, // 4 IIC_BrB
  { 1, 132, 133, 0, 0 }, // 5 IIC_VecFP
  { 1, 130, 131, 0, 0 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 133, 134, 0, 0 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 134, 135, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 134, 135, 0, 0 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 135, 136, 0, 0 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 136, 137, 0, 0 }, // 17 IIC_FPAddSub
  { 1, 137, 138, 0, 0 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 136, 137, 0, 0 }, // 20 IIC_FPCompare
  { 1, 136, 137, 0, 0 }, // 21 IIC_FPGeneral
  { 1, 138, 139, 0, 0 }, // 22 IIC_VecGeneral
  { 1, 139, 140, 0, 0 }, // 23 IIC_VecComplex
  { 1, 134, 135, 0, 0 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 140, 141, 0, 0 }, // 26 IIC_FPDivS
  { 1, 136, 137, 0, 0 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 141, 142, 0, 0 }, // 30 IIC_LdStICBI
  { 0, 0, 0, 0, 0 }, // 31 IIC_IntISEL
  { 1, 142, 143, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 134, 135, 0, 0 }, // 33 IIC_LdStLWARX
  { 1, 134, 135, 0, 0 }, // 34 IIC_LdStLoadUpd
  { 1, 134, 135, 0, 0 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 143, 144, 0, 0 }, // 39 IIC_LdStLFD
  { 1, 143, 144, 0, 0 }, // 40 IIC_LdStLFDU
  { 1, 143, 144, 0, 0 }, // 41 IIC_LdStLFDUX
  { 1, 134, 135, 0, 0 }, // 42 IIC_LdStLHA
  { 1, 134, 135, 0, 0 }, // 43 IIC_LdStLHAU
  { 1, 134, 135, 0, 0 }, // 44 IIC_LdStLHAUX
  { 1, 144, 145, 0, 0 }, // 45 IIC_LdStLMW
  { 1, 134, 135, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 133, 134, 0, 0 }, // 48 IIC_BrMCR
  { 1, 133, 134, 0, 0 }, // 49 IIC_BrMCRX
  { 1, 133, 134, 0, 0 }, // 50 IIC_SprMFCR
  { 1, 145, 146, 0, 0 }, // 51 IIC_SprMFSPR
  { 1, 136, 137, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 141, 142, 0, 0 }, // 53 IIC_SprMFMSR
  { 0, 0, 0, 0, 0 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 1, 145, 146, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 146, 147, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 133, 134, 0, 0 }, // 59 IIC_SprMTSPR
  { 1, 136, 137, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 133, 134, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 1, 133, 134, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 145, 146, 0, 0 }, // 65 IIC_IntMulHW
  { 1, 145, 146, 0, 0 }, // 66 IIC_IntMulHWU
  { 1, 141, 142, 0, 0 }, // 67 IIC_IntMulLI
  { 1, 147, 148, 0, 0 }, // 68 IIC_VecPerm
  { 1, 134, 135, 0, 0 }, // 69 IIC_LdStSTFD
  { 1, 134, 135, 0, 0 }, // 70 IIC_LdStSTFDU
  { 1, 130, 131, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 130, 131, 0, 0 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 148, 149, 0, 0 }, // 83 IIC_IntShift
  { 1, 134, 135, 0, 0 }, // 84 IIC_LdStSTWCX
  { 1, 134, 135, 0, 0 }, // 85 IIC_LdStSTU
  { 1, 134, 135, 0, 0 }, // 86 IIC_LdStSTUX
  { 1, 134, 135, 0, 0 }, // 87 IIC_LdStSTD
  { 1, 134, 135, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 134, 135, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 148, 149, 0, 0 }, // 95 IIC_IntTrapW
  { 1, 149, 150, 0, 0 }, // 96 IIC_VecFPCompare
  { 1, 138, 139, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 1, 132, 133, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 1, 147, 148, 0, 0 }, // 99 XXSEL
  { 1, 133, 134, 0, 0 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 136, 137, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 1, 136, 137, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 130, 131, 0, 0 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 148, 149, 0, 0 }, // 105 TW_TWI
  { 1, 136, 137, 0, 0 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 1, 132, 133, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 1, 132, 133, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 136, 137, 0, 0 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 130, 131, 0, 0 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 138, 139, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 130, 131, 0, 0 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 130, 131, 0, 0 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 130, 131, 0, 0 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 133, 134, 0, 0 }, // 118 MCRF
  { 1, 133, 134, 0, 0 }, // 119 MCRXRX
  { 1, 132, 133, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 131, 132, 0, 0 }, // 121 RFEBB
  { 1, 133, 134, 0, 0 }, // 122 TBEGIN_TRECHKPT
  { 1, 134, 135, 0, 0 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 130, 131, 0, 0 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 0, 0, 0, 0, 0 }, // 127 MFOCRF_MFOCRF8
  { 1, 133, 134, 0, 0 }, // 128 MTOCRF_MTOCRF8
  { 1, 133, 134, 0, 0 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 130, 131, 0, 0 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 136, 137, 0, 0 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 148, 149, 0, 0 }, // 132 SRAW_SRAWI
  { 0, 0, 0, 0, 0 }, // 133 ISEL_ISEL8
  { 1, 132, 133, 0, 0 }, // 134 XSIEXPDP
  { 1, 133, 134, 0, 0 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 134, 135, 0, 0 }, // 136 MFVSCR
  { 1, 134, 135, 0, 0 }, // 137 MTVSCR
  { 1, 149, 150, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 138, 139, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 1, 132, 133, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 136, 137, 0, 0 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 1, 132, 133, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 138, 139, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 145, 146, 0, 0 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 145, 146, 0, 0 }, // 146 MULHDU_MULHWU
  { 1, 141, 142, 0, 0 }, // 147 MULLI_MULLI8
  { 1, 136, 137, 0, 0 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 136, 137, 0, 0 }, // 149 FADD_FSUB
  { 1, 136, 137, 0, 0 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 1, 132, 133, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 136, 137, 0, 0 }, // 152 FSELD_rec_FSELS_rec
  { 1, 145, 146, 0, 0 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 145, 146, 0, 0 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 136, 137, 0, 0 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 136, 137, 0, 0 }, // 157 FADD_rec_FSUB_rec
  { 1, 136, 137, 0, 0 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 1, 132, 133, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 134, 135, 0, 0 }, // 160 LVSL_LVSR
  { 1, 147, 148, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 1, 132, 133, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 138, 139, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 136, 137, 0, 0 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 1, 132, 133, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 1, 132, 133, 0, 0 }, // 166 BCDCTSQ_rec
  { 1, 132, 133, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 1, 132, 133, 0, 0 }, // 168 BCDCFSQ_rec
  { 1, 132, 133, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 1, 132, 133, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 134, 135, 0, 0 }, // 171 LXVL_LXVLL
  { 1, 134, 135, 0, 0 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 143, 144, 0, 0 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 134, 135, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 141, 142, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 134, 135, 0, 0 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 134, 135, 0, 0 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 142, 143, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 134, 135, 0, 0 }, // 186 TLBSYNC
  { 1, 129, 130, 0, 0 }, // 187 SYNC
  { 1, 144, 145, 0, 0 }, // 188 LMW
  { 1, 143, 144, 0, 0 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 134, 135, 0, 0 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 134, 135, 0, 0 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 133, 134, 0, 0 }, // 198 TEND
  { 1, 134, 135, 0, 0 }, // 199 STBCX_STHCX_STWCX
  { 1, 134, 135, 0, 0 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 134, 135, 0, 0 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 1, 134, 135, 0, 0 }, // 204 LWA_LWA_32
  { 1, 133, 134, 0, 0 }, // 205 TCHECK
  { 1, 143, 144, 0, 0 }, // 206 LFIWAX
  { 1, 143, 144, 0, 0 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 143, 144, 0, 0 }, // 209 LFSX_LFS
  { 1, 143, 144, 0, 0 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 134, 135, 0, 0 }, // 212 LHAU_LHAU8
  { 1, 134, 135, 0, 0 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 143, 144, 0, 0 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 134, 135, 0, 0 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 134, 135, 0, 0 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 134, 135, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 134, 135, 0, 0 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 144, 145, 0, 0 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 134, 135, 0, 0 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 134, 135, 0, 0 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 134, 135, 0, 0 }, // 225 STXVL_STXVLL
  { 1, 133, 134, 0, 0 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 130, 131, 0, 0 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 146, 147, 0, 0 }, // 230 MFTB_MFTB8
  { 1, 145, 146, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 141, 142, 0, 0 }, // 232 MFMSR
  { 1, 133, 134, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 133, 134, 0, 0 }, // 235 MTSPR_MTSPR8
  { 1, 135, 136, 0, 0 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 135, 136, 0, 0 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 135, 136, 0, 0 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 135, 136, 0, 0 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 133, 134, 0, 0 }, // 244 MTCRF_MTCRF8
  { 1, 130, 131, 0, 0 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 136, 137, 0, 0 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 133, 134, 0, 0 }, // 247 MCRFS
  { 1, 136, 137, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 136, 137, 0, 0 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 130, 131, 0, 0 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 130, 131, 0, 0 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 148, 149, 0, 0 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 136, 137, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 133, 134, 0, 0 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 137, 138, 0, 0 }, // 259 FDIV
  { 1, 137, 138, 0, 0 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 137, 138, 0, 0 }, // 269 XSDIVDP
  { 1, 140, 141, 0, 0 }, // 270 FDIVS
  { 1, 140, 141, 0, 0 }, // 271 FDIVS_rec
  { 1, 140, 141, 0, 0 }, // 272 XSDIVSP
  { 1, 140, 141, 0, 0 }, // 273 XVDIVSP
  { 1, 137, 138, 0, 0 }, // 274 XVDIVDP
  { 1, 143, 144, 0, 0 }, // 275 LFSU
  { 1, 143, 144, 0, 0 }, // 276 LFSUX
  { 1, 134, 135, 0, 0 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 134, 135, 0, 0 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 134, 135, 0, 0 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 134, 135, 0, 0 }, // 280 LBZU_LBZU8
  { 1, 134, 135, 0, 0 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 1, 143, 144, 0, 0 }, // 284 LFDU
  { 1, 143, 144, 0, 0 }, // 285 LFDUX
  { 1, 132, 133, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 131, 132, 0, 0 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 131, 132, 0, 0 }, // 288 ADDPCIS
  { 1, 134, 135, 0, 0 }, // 289 LDAT_LWAT
  { 1, 134, 135, 0, 0 }, // 290 STDAT_STWAT
  { 1, 130, 131, 0, 0 }, // 291 BRINC
  { 1, 138, 139, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 139, 140, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 134, 135, 0, 0 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 134, 135, 0, 0 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 131, 132, 0, 0 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 1, 130, 131, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 134, 135, 0, 0 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 134, 135, 0, 0 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 143, 144, 0, 0 }, // 301 LFDEPX
  { 1, 134, 135, 0, 0 }, // 302 STFDEPX
  { 1, 145, 146, 0, 0 }, // 303 MFSR_MFSRIN
  { 1, 133, 134, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 145, 146, 0, 0 }, // 305 MFDCR
  { 1, 133, 134, 0, 0 }, // 306 MTDCR
  { 1, 130, 131, 0, 0 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 133, 134, 0, 0 }, // 309 WRTEE_WRTEEI
  { 1, 129, 130, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 134, 135, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary G5Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 150, 151, 0, 0 }, // 1 IIC_LdStSync
  { 1, 151, 152, 0, 0 }, // 2 IIC_IntSimple
  { 1, 151, 152, 0, 0 }, // 3 IIC_IntGeneral
  { 1, 152, 153, 0, 0 }, // 4 IIC_BrB
  { 1, 153, 154, 0, 0 }, // 5 IIC_VecFP
  { 1, 154, 155, 0, 0 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 155, 156, 0, 0 }, // 10 IIC_BrCR
  { 1, 156, 157, 0, 0 }, // 11 IIC_LdStLD
  { 1, 156, 157, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 156, 157, 0, 0 }, // 13 IIC_LdStLoad
  { 1, 157, 158, 0, 0 }, // 14 IIC_IntDivD
  { 1, 158, 159, 0, 0 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 159, 160, 0, 0 }, // 17 IIC_FPAddSub
  { 1, 160, 161, 0, 0 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 161, 162, 0, 0 }, // 20 IIC_FPCompare
  { 1, 159, 160, 0, 0 }, // 21 IIC_FPGeneral
  { 1, 162, 163, 0, 0 }, // 22 IIC_VecGeneral
  { 1, 163, 164, 0, 0 }, // 23 IIC_VecComplex
  { 1, 156, 157, 0, 0 }, // 24 IIC_LdStStore
  { 1, 151, 152, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 160, 161, 0, 0 }, // 26 IIC_FPDivS
  { 1, 159, 160, 0, 0 }, // 27 IIC_FPFused
  { 1, 164, 165, 0, 0 }, // 28 IIC_FPSqrtD
  { 1, 164, 165, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 165, 166, 0, 0 }, // 30 IIC_LdStICBI
  { 0, 0, 0, 0, 0 }, // 31 IIC_IntISEL
  { 1, 165, 166, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 166, 167, 0, 0 }, // 33 IIC_LdStLWARX
  { 1, 156, 157, 0, 0 }, // 34 IIC_LdStLoadUpd
  { 1, 156, 157, 0, 0 }, // 35 IIC_LdStLoadUpdX
  { 1, 166, 167, 0, 0 }, // 36 IIC_LdStLDARX
  { 1, 156, 157, 0, 0 }, // 37 IIC_LdStLDU
  { 1, 156, 157, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 156, 157, 0, 0 }, // 39 IIC_LdStLFD
  { 1, 167, 168, 0, 0 }, // 40 IIC_LdStLFDU
  { 1, 167, 168, 0, 0 }, // 41 IIC_LdStLFDUX
  { 1, 167, 168, 0, 0 }, // 42 IIC_LdStLHA
  { 1, 167, 168, 0, 0 }, // 43 IIC_LdStLHAU
  { 1, 167, 168, 0, 0 }, // 44 IIC_LdStLHAUX
  { 1, 168, 169, 0, 0 }, // 45 IIC_LdStLMW
  { 1, 167, 168, 0, 0 }, // 46 IIC_LdStLWA
  { 1, 169, 170, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 170, 171, 0, 0 }, // 48 IIC_BrMCR
  { 1, 171, 172, 0, 0 }, // 49 IIC_BrMCRX
  { 1, 172, 173, 0, 0 }, // 50 IIC_SprMFCR
  { 1, 173, 174, 0, 0 }, // 51 IIC_SprMFSPR
  { 1, 174, 175, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 173, 174, 0, 0 }, // 53 IIC_SprMFMSR
  { 1, 172, 173, 0, 0 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 1, 156, 157, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 175, 176, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 176, 177, 0, 0 }, // 59 IIC_SprMTSPR
  { 1, 159, 160, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 156, 157, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 1, 156, 157, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 177, 178, 0, 0 }, // 65 IIC_IntMulHW
  { 1, 177, 178, 0, 0 }, // 66 IIC_IntMulHWU
  { 1, 178, 179, 0, 0 }, // 67 IIC_IntMulLI
  { 1, 179, 180, 0, 0 }, // 68 IIC_VecPerm
  { 1, 180, 181, 0, 0 }, // 69 IIC_LdStSTFD
  { 1, 180, 181, 0, 0 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 1, 181, 182, 0, 0 }, // 72 IIC_IntRFID
  { 1, 151, 152, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 178, 179, 0, 0 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 151, 152, 0, 0 }, // 83 IIC_IntShift
  { 1, 166, 167, 0, 0 }, // 84 IIC_LdStSTWCX
  { 1, 156, 157, 0, 0 }, // 85 IIC_LdStSTU
  { 1, 156, 157, 0, 0 }, // 86 IIC_LdStSTUX
  { 1, 156, 157, 0, 0 }, // 87 IIC_LdStSTD
  { 1, 166, 167, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 1, 182, 183, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 156, 157, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 182, 183, 0, 0 }, // 95 IIC_IntTrapW
  { 1, 183, 184, 0, 0 }, // 96 IIC_VecFPCompare
  { 1, 162, 163, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 1, 153, 154, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 1, 179, 180, 0, 0 }, // 99 XXSEL
  { 1, 176, 177, 0, 0 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 159, 160, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 1, 174, 175, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 154, 155, 0, 0 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 1, 182, 183, 0, 0 }, // 104 TD_TDI
  { 1, 182, 183, 0, 0 }, // 105 TW_TWI
  { 1, 161, 162, 0, 0 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 1, 153, 154, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 1, 153, 154, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 161, 162, 0, 0 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 151, 152, 0, 0 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 1, 151, 152, 0, 0 }, // 111 SLD_SRD_SRAD
  { 1, 151, 152, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 162, 163, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 154, 155, 0, 0 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 151, 152, 0, 0 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 151, 152, 0, 0 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 170, 171, 0, 0 }, // 118 MCRF
  { 1, 171, 172, 0, 0 }, // 119 MCRXRX
  { 1, 153, 154, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 152, 153, 0, 0 }, // 121 RFEBB
  { 1, 176, 177, 0, 0 }, // 122 TBEGIN_TRECHKPT
  { 1, 156, 157, 0, 0 }, // 123 WAIT
  { 1, 151, 152, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 178, 179, 0, 0 }, // 125 RLWIMI_RLWIMI8
  { 1, 151, 152, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 172, 173, 0, 0 }, // 127 MFOCRF_MFOCRF8
  { 1, 171, 172, 0, 0 }, // 128 MTOCRF_MTOCRF8
  { 1, 155, 156, 0, 0 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 151, 152, 0, 0 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 159, 160, 0, 0 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 151, 152, 0, 0 }, // 132 SRAW_SRAWI
  { 0, 0, 0, 0, 0 }, // 133 ISEL_ISEL8
  { 1, 153, 154, 0, 0 }, // 134 XSIEXPDP
  { 1, 176, 177, 0, 0 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 156, 157, 0, 0 }, // 136 MFVSCR
  { 1, 156, 157, 0, 0 }, // 137 MTVSCR
  { 1, 183, 184, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 162, 163, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 1, 153, 154, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 161, 162, 0, 0 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 1, 153, 154, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 162, 163, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 1, 169, 170, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 177, 178, 0, 0 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 177, 178, 0, 0 }, // 146 MULHDU_MULHWU
  { 1, 178, 179, 0, 0 }, // 147 MULLI_MULLI8
  { 1, 159, 160, 0, 0 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 159, 160, 0, 0 }, // 149 FADD_FSUB
  { 1, 159, 160, 0, 0 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 1, 153, 154, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 159, 160, 0, 0 }, // 152 FSELD_rec_FSELS_rec
  { 1, 177, 178, 0, 0 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 177, 178, 0, 0 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 1, 169, 170, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 159, 160, 0, 0 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 159, 160, 0, 0 }, // 157 FADD_rec_FSUB_rec
  { 1, 159, 160, 0, 0 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 1, 153, 154, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 156, 157, 0, 0 }, // 160 LVSL_LVSR
  { 1, 179, 180, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 1, 153, 154, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 162, 163, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 161, 162, 0, 0 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 1, 153, 154, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 1, 153, 154, 0, 0 }, // 166 BCDCTSQ_rec
  { 1, 153, 154, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 1, 153, 154, 0, 0 }, // 168 BCDCFSQ_rec
  { 1, 153, 154, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 1, 153, 154, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 156, 157, 0, 0 }, // 171 LXVL_LXVLL
  { 1, 156, 157, 0, 0 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 156, 157, 0, 0 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 156, 157, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 165, 166, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 156, 157, 0, 0 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 166, 167, 0, 0 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 1, 156, 157, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 1, 166, 167, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 165, 166, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 156, 157, 0, 0 }, // 186 TLBSYNC
  { 1, 150, 151, 0, 0 }, // 187 SYNC
  { 1, 168, 169, 0, 0 }, // 188 LMW
  { 1, 156, 157, 0, 0 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 156, 157, 0, 0 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 156, 157, 0, 0 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 176, 177, 0, 0 }, // 198 TEND
  { 1, 166, 167, 0, 0 }, // 199 STBCX_STHCX_STWCX
  { 1, 166, 167, 0, 0 }, // 200 STDCX
  { 1, 156, 157, 0, 0 }, // 201 LDMX
  { 1, 167, 168, 0, 0 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 1, 167, 168, 0, 0 }, // 204 LWA_LWA_32
  { 1, 176, 177, 0, 0 }, // 205 TCHECK
  { 1, 156, 157, 0, 0 }, // 206 LFIWAX
  { 1, 156, 157, 0, 0 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 156, 157, 0, 0 }, // 209 LFSX_LFS
  { 1, 156, 157, 0, 0 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 167, 168, 0, 0 }, // 212 LHAU_LHAU8
  { 1, 167, 168, 0, 0 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 156, 157, 0, 0 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 180, 181, 0, 0 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 156, 157, 0, 0 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 156, 157, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 156, 157, 0, 0 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 168, 169, 0, 0 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 156, 157, 0, 0 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 180, 181, 0, 0 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 156, 157, 0, 0 }, // 225 STXVL_STXVLL
  { 1, 176, 177, 0, 0 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 151, 152, 0, 0 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 175, 176, 0, 0 }, // 230 MFTB_MFTB8
  { 1, 173, 174, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 173, 174, 0, 0 }, // 232 MFMSR
  { 1, 156, 157, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 176, 177, 0, 0 }, // 235 MTSPR_MTSPR8
  { 1, 158, 159, 0, 0 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 158, 159, 0, 0 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 1, 157, 158, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 1, 157, 158, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 158, 159, 0, 0 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 1, 157, 158, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 158, 159, 0, 0 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 1, 157, 158, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 171, 172, 0, 0 }, // 244 MTCRF_MTCRF8
  { 1, 151, 152, 0, 0 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 159, 160, 0, 0 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 170, 171, 0, 0 }, // 247 MCRFS
  { 1, 174, 175, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 159, 160, 0, 0 }, // 249 MTFSFb
  { 1, 151, 152, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 1, 151, 152, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 178, 179, 0, 0 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 151, 152, 0, 0 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 151, 152, 0, 0 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 174, 175, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 172, 173, 0, 0 }, // 256 MFCR_MFCR8
  { 1, 151, 152, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 1, 151, 152, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 160, 161, 0, 0 }, // 259 FDIV
  { 1, 160, 161, 0, 0 }, // 260 FDIV_rec
  { 1, 164, 165, 0, 0 }, // 261 XSSQRTDP
  { 1, 164, 165, 0, 0 }, // 262 FSQRT
  { 1, 164, 165, 0, 0 }, // 263 XVSQRTDP
  { 1, 164, 165, 0, 0 }, // 264 XVSQRTSP
  { 1, 164, 165, 0, 0 }, // 265 FSQRT_rec
  { 1, 164, 165, 0, 0 }, // 266 XSSQRTSP
  { 1, 164, 165, 0, 0 }, // 267 FSQRTS
  { 1, 164, 165, 0, 0 }, // 268 FSQRTS_rec
  { 1, 160, 161, 0, 0 }, // 269 XSDIVDP
  { 1, 160, 161, 0, 0 }, // 270 FDIVS
  { 1, 160, 161, 0, 0 }, // 271 FDIVS_rec
  { 1, 160, 161, 0, 0 }, // 272 XSDIVSP
  { 1, 160, 161, 0, 0 }, // 273 XVDIVSP
  { 1, 160, 161, 0, 0 }, // 274 XVDIVDP
  { 1, 167, 168, 0, 0 }, // 275 LFSU
  { 1, 167, 168, 0, 0 }, // 276 LFSUX
  { 1, 180, 181, 0, 0 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 156, 157, 0, 0 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 156, 157, 0, 0 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 156, 157, 0, 0 }, // 280 LBZU_LBZU8
  { 1, 156, 157, 0, 0 }, // 281 LBZUX_LBZUX8
  { 1, 156, 157, 0, 0 }, // 282 LDU
  { 1, 156, 157, 0, 0 }, // 283 LDUX
  { 1, 167, 168, 0, 0 }, // 284 LFDU
  { 1, 167, 168, 0, 0 }, // 285 LFDUX
  { 1, 153, 154, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 152, 153, 0, 0 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 152, 153, 0, 0 }, // 288 ADDPCIS
  { 1, 156, 157, 0, 0 }, // 289 LDAT_LWAT
  { 1, 156, 157, 0, 0 }, // 290 STDAT_STWAT
  { 1, 151, 152, 0, 0 }, // 291 BRINC
  { 1, 162, 163, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 163, 164, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 156, 157, 0, 0 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 156, 157, 0, 0 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 152, 153, 0, 0 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 1, 181, 182, 0, 0 }, // 298 RFID
  { 1, 156, 157, 0, 0 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 156, 157, 0, 0 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 156, 157, 0, 0 }, // 301 LFDEPX
  { 1, 180, 181, 0, 0 }, // 302 STFDEPX
  { 1, 156, 157, 0, 0 }, // 303 MFSR_MFSRIN
  { 1, 156, 157, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 173, 174, 0, 0 }, // 305 MFDCR
  { 1, 176, 177, 0, 0 }, // 306 MTDCR
  { 1, 151, 152, 0, 0 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 156, 157, 0, 0 }, // 309 WRTEE_WRTEEI
  { 1, 150, 151, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 156, 157, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary PPCA2Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 184, 185, 539, 540 }, // 1 IIC_LdStSync
  { 1, 184, 185, 540, 543 }, // 2 IIC_IntSimple
  { 1, 184, 185, 543, 546 }, // 3 IIC_IntGeneral
  { 1, 184, 185, 546, 549 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 184, 185, 549, 552 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 184, 185, 552, 555 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 184, 185, 555, 558 }, // 12 IIC_LdStDCBF
  { 1, 184, 185, 558, 561 }, // 13 IIC_LdStLoad
  { 1, 184, 185, 561, 564 }, // 14 IIC_IntDivD
  { 1, 184, 185, 564, 567 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 185, 186, 567, 570 }, // 17 IIC_FPAddSub
  { 1, 185, 186, 570, 573 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 185, 186, 573, 576 }, // 20 IIC_FPCompare
  { 1, 185, 186, 576, 579 }, // 21 IIC_FPGeneral
  { 0, 0, 0, 0, 0 }, // 22 IIC_VecGeneral
  { 0, 0, 0, 0, 0 }, // 23 IIC_VecComplex
  { 1, 184, 185, 579, 582 }, // 24 IIC_LdStStore
  { 1, 184, 185, 582, 585 }, // 25 IIC_IntRotateDI
  { 1, 185, 186, 585, 588 }, // 26 IIC_FPDivS
  { 1, 185, 186, 588, 592 }, // 27 IIC_FPFused
  { 1, 185, 186, 592, 595 }, // 28 IIC_FPSqrtD
  { 1, 185, 186, 595, 598 }, // 29 IIC_FPSqrtS
  { 1, 184, 185, 598, 601 }, // 30 IIC_LdStICBI
  { 1, 184, 185, 601, 605 }, // 31 IIC_IntISEL
  { 1, 184, 185, 605, 606 }, // 32 IIC_SprISYNC
  { 1, 184, 185, 606, 609 }, // 33 IIC_LdStLWARX
  { 1, 184, 185, 609, 613 }, // 34 IIC_LdStLoadUpd
  { 1, 184, 185, 613, 617 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 1, 184, 185, 617, 620 }, // 37 IIC_LdStLDU
  { 1, 184, 185, 620, 623 }, // 38 IIC_LdStLDUX
  { 1, 184, 185, 623, 626 }, // 39 IIC_LdStLFD
  { 1, 184, 185, 626, 630 }, // 40 IIC_LdStLFDU
  { 1, 184, 185, 630, 634 }, // 41 IIC_LdStLFDUX
  { 1, 184, 185, 634, 637 }, // 42 IIC_LdStLHA
  { 1, 184, 185, 637, 641 }, // 43 IIC_LdStLHAU
  { 1, 184, 185, 641, 645 }, // 44 IIC_LdStLHAUX
  { 0, 0, 0, 0, 0 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 184, 185, 645, 648 }, // 48 IIC_BrMCR
  { 1, 184, 185, 648, 651 }, // 49 IIC_BrMCRX
  { 1, 184, 185, 651, 653 }, // 50 IIC_SprMFCR
  { 1, 184, 185, 653, 655 }, // 51 IIC_SprMFSPR
  { 0, 0, 0, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 184, 185, 655, 657 }, // 53 IIC_SprMFMSR
  { 1, 184, 185, 657, 659 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 0, 0, 0, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 184, 185, 659, 661 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 184, 185, 661, 663 }, // 59 IIC_SprMTSPR
  { 0, 0, 0, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 184, 185, 663, 665 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 0, 0, 0, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 184, 185, 665, 668 }, // 65 IIC_IntMulHW
  { 1, 184, 185, 668, 671 }, // 66 IIC_IntMulHWU
  { 1, 184, 185, 671, 674 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 1, 184, 185, 674, 677 }, // 69 IIC_LdStSTFD
  { 1, 184, 185, 677, 681 }, // 70 IIC_LdStSTFDU
  { 1, 184, 185, 681, 682 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 1, 184, 185, 682, 685 }, // 73 IIC_IntRotateD
  { 1, 184, 185, 685, 688 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 184, 185, 688, 691 }, // 83 IIC_IntShift
  { 1, 184, 185, 691, 694 }, // 84 IIC_LdStSTWCX
  { 1, 184, 185, 694, 698 }, // 85 IIC_LdStSTU
  { 1, 184, 185, 698, 702 }, // 86 IIC_LdStSTUX
  { 1, 184, 185, 702, 705 }, // 87 IIC_LdStSTD
  { 1, 184, 185, 705, 708 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 1, 184, 185, 708, 710 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 0, 0, 0, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 184, 185, 710, 712 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 0, 0, 0, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 184, 185, 712, 714 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 0, 0, 0, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 0, 0, 0, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 184, 185, 714, 717 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 1, 184, 185, 717, 719 }, // 104 TD_TDI
  { 1, 184, 185, 719, 721 }, // 105 TW_TWI
  { 1, 185, 186, 721, 724 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 185, 186, 724, 727 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 184, 185, 727, 730 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 1, 184, 185, 730, 733 }, // 111 SLD_SRD_SRAD
  { 1, 184, 185, 733, 736 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 0, 0, 0, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 184, 185, 736, 739 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 184, 185, 739, 742 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 184, 185, 742, 745 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 184, 185, 745, 748 }, // 118 MCRF
  { 1, 184, 185, 748, 751 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 184, 185, 751, 754 }, // 121 RFEBB
  { 1, 184, 185, 754, 756 }, // 122 TBEGIN_TRECHKPT
  { 1, 184, 185, 756, 759 }, // 123 WAIT
  { 1, 184, 185, 759, 762 }, // 124 RLDCL_RLDCR
  { 1, 184, 185, 762, 765 }, // 125 RLWIMI_RLWIMI8
  { 1, 184, 185, 765, 768 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 184, 185, 768, 770 }, // 127 MFOCRF_MFOCRF8
  { 1, 184, 185, 770, 773 }, // 128 MTOCRF_MTOCRF8
  { 1, 184, 185, 773, 776 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 184, 185, 776, 779 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 185, 186, 779, 782 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 184, 185, 782, 785 }, // 132 SRAW_SRAWI
  { 1, 184, 185, 785, 789 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 184, 185, 789, 791 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 184, 185, 791, 794 }, // 136 MFVSCR
  { 1, 184, 185, 794, 797 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 0, 0, 0, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 185, 186, 797, 800 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 0, 0, 0, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 184, 185, 800, 803 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 184, 185, 803, 806 }, // 146 MULHDU_MULHWU
  { 1, 184, 185, 806, 809 }, // 147 MULLI_MULLI8
  { 1, 185, 186, 809, 812 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 185, 186, 812, 815 }, // 149 FADD_FSUB
  { 1, 185, 186, 815, 819 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 185, 186, 819, 822 }, // 152 FSELD_rec_FSELS_rec
  { 1, 184, 185, 822, 825 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 184, 185, 825, 828 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 185, 186, 828, 831 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 185, 186, 831, 834 }, // 157 FADD_rec_FSUB_rec
  { 1, 185, 186, 834, 838 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 184, 185, 838, 841 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 0, 0, 0, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 185, 186, 841, 844 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 184, 185, 844, 847 }, // 171 LXVL_LXVLL
  { 1, 184, 185, 847, 850 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 184, 185, 850, 853 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 184, 185, 853, 856 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 184, 185, 856, 859 }, // 178 ICBI_ICBIEP
  { 1, 184, 185, 859, 862 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 184, 185, 862, 865 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 1, 184, 185, 865, 866 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 0, 0, 0, 0, 0 }, // 186 TLBSYNC
  { 1, 184, 185, 866, 867 }, // 187 SYNC
  { 0, 0, 0, 0, 0 }, // 188 LMW
  { 1, 184, 185, 867, 870 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 184, 185, 870, 874 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 184, 185, 874, 878 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 184, 185, 878, 880 }, // 198 TEND
  { 1, 184, 185, 880, 883 }, // 199 STBCX_STHCX_STWCX
  { 1, 184, 185, 883, 886 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 184, 185, 886, 889 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 184, 185, 889, 891 }, // 205 TCHECK
  { 1, 184, 185, 891, 894 }, // 206 LFIWAX
  { 1, 184, 185, 894, 897 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 184, 185, 897, 900 }, // 209 LFSX_LFS
  { 1, 184, 185, 900, 903 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 184, 185, 903, 907 }, // 212 LHAU_LHAU8
  { 1, 184, 185, 907, 911 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 184, 185, 911, 914 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 184, 185, 914, 917 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 184, 185, 917, 920 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 184, 185, 920, 923 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 184, 185, 923, 926 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 0, 0, 0, 0, 0 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 184, 185, 926, 929 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 184, 185, 929, 932 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 184, 185, 932, 935 }, // 225 STXVL_STXVLL
  { 1, 184, 185, 935, 937 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 184, 185, 937, 940 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 1, 184, 185, 940, 942 }, // 230 MFTB_MFTB8
  { 1, 184, 185, 942, 944 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 184, 185, 944, 946 }, // 232 MFMSR
  { 1, 184, 185, 946, 948 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 184, 185, 948, 950 }, // 235 MTSPR_MTSPR8
  { 1, 184, 185, 950, 953 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 184, 185, 953, 956 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 1, 184, 185, 956, 959 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 1, 184, 185, 959, 962 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 184, 185, 962, 965 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 1, 184, 185, 965, 968 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 184, 185, 968, 971 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 1, 184, 185, 971, 974 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 184, 185, 974, 977 }, // 244 MTCRF_MTCRF8
  { 1, 184, 185, 977, 980 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 185, 186, 980, 983 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 184, 185, 983, 986 }, // 247 MCRFS
  { 0, 0, 0, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 0, 0, 0, 0, 0 }, // 249 MTFSFb
  { 1, 184, 185, 986, 989 }, // 250 RLDCL_rec_RLDCR_rec
  { 1, 184, 185, 989, 992 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 184, 185, 992, 995 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 184, 185, 995, 998 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 184, 185, 998, 1001 }, // 254 SRAWI_rec_SRAW_rec
  { 0, 0, 0, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 184, 185, 1001, 1003 }, // 256 MFCR_MFCR8
  { 1, 184, 185, 1003, 1006 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 1, 184, 185, 1006, 1009 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 185, 186, 1009, 1012 }, // 259 FDIV
  { 1, 185, 186, 1012, 1015 }, // 260 FDIV_rec
  { 1, 185, 186, 1015, 1018 }, // 261 XSSQRTDP
  { 1, 185, 186, 1018, 1021 }, // 262 FSQRT
  { 1, 185, 186, 1021, 1024 }, // 263 XVSQRTDP
  { 1, 185, 186, 1024, 1027 }, // 264 XVSQRTSP
  { 1, 185, 186, 1027, 1030 }, // 265 FSQRT_rec
  { 1, 185, 186, 1030, 1033 }, // 266 XSSQRTSP
  { 1, 185, 186, 1033, 1036 }, // 267 FSQRTS
  { 1, 185, 186, 1036, 1039 }, // 268 FSQRTS_rec
  { 1, 185, 186, 1039, 1042 }, // 269 XSDIVDP
  { 1, 185, 186, 1042, 1045 }, // 270 FDIVS
  { 1, 185, 186, 1045, 1048 }, // 271 FDIVS_rec
  { 1, 185, 186, 1048, 1051 }, // 272 XSDIVSP
  { 1, 185, 186, 1051, 1054 }, // 273 XVDIVSP
  { 1, 185, 186, 1054, 1057 }, // 274 XVDIVDP
  { 1, 184, 185, 1057, 1061 }, // 275 LFSU
  { 1, 184, 185, 1061, 1065 }, // 276 LFSUX
  { 1, 184, 185, 1065, 1069 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 184, 185, 1069, 1073 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 184, 185, 1073, 1077 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 184, 185, 1077, 1081 }, // 280 LBZU_LBZU8
  { 1, 184, 185, 1081, 1085 }, // 281 LBZUX_LBZUX8
  { 1, 184, 185, 1085, 1088 }, // 282 LDU
  { 1, 184, 185, 1088, 1091 }, // 283 LDUX
  { 1, 184, 185, 1091, 1095 }, // 284 LFDU
  { 1, 184, 185, 1095, 1099 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 184, 185, 1099, 1102 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 184, 185, 1102, 1105 }, // 288 ADDPCIS
  { 1, 184, 185, 1105, 1108 }, // 289 LDAT_LWAT
  { 1, 184, 185, 1108, 1111 }, // 290 STDAT_STWAT
  { 1, 184, 185, 1111, 1114 }, // 291 BRINC
  { 0, 0, 0, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 0, 0, 0, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 184, 185, 1114, 1117 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 184, 185, 1117, 1120 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 184, 185, 1120, 1123 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 1, 184, 185, 1123, 1124 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 184, 185, 1124, 1127 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 184, 185, 1127, 1130 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 184, 185, 1130, 1133 }, // 301 LFDEPX
  { 1, 184, 185, 1133, 1136 }, // 302 STFDEPX
  { 0, 0, 0, 0, 0 }, // 303 MFSR_MFSRIN
  { 0, 0, 0, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 184, 185, 1136, 1138 }, // 305 MFDCR
  { 1, 184, 185, 1138, 1140 }, // 306 MTDCR
  { 1, 184, 185, 1140, 1143 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 184, 185, 1143, 1145 }, // 309 WRTEE_WRTEEI
  { 1, 184, 185, 1145, 1146 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 184, 185, 1146, 1149 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary PPCE500Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 186, 188, 0, 0 }, // 1 IIC_LdStSync
  { 1, 188, 190, 1149, 1152 }, // 2 IIC_IntSimple
  { 1, 188, 190, 1152, 1155 }, // 3 IIC_IntGeneral
  { 1, 190, 192, 1155, 1157 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 188, 190, 1157, 1160 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 190, 192, 1160, 1163 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 186, 188, 1163, 1165 }, // 12 IIC_LdStDCBF
  { 1, 186, 188, 1165, 1167 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 192, 195, 1167, 1170 }, // 15 IIC_IntDivW
  { 1, 195, 197, 1170, 1173 }, // 16 IIC_FPDGeneral
  { 0, 0, 0, 0, 0 }, // 17 IIC_FPAddSub
  { 1, 197, 199, 1173, 1176 }, // 18 IIC_FPDivD
  { 1, 199, 201, 1176, 1179 }, // 19 IIC_FPSGeneral
  { 0, 0, 0, 0, 0 }, // 20 IIC_FPCompare
  { 0, 0, 0, 0, 0 }, // 21 IIC_FPGeneral
  { 1, 201, 203, 1179, 1182 }, // 22 IIC_VecGeneral
  { 1, 199, 201, 1182, 1185 }, // 23 IIC_VecComplex
  { 1, 186, 188, 1185, 1187 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 203, 205, 1187, 1190 }, // 26 IIC_FPDivS
  { 0, 0, 0, 0, 0 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 186, 188, 1190, 1192 }, // 30 IIC_LdStICBI
  { 1, 188, 190, 1192, 1196 }, // 31 IIC_IntISEL
  { 0, 0, 0, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 205, 207, 1196, 1199 }, // 33 IIC_LdStLWARX
  { 2, 207, 210, 1199, 1201 }, // 34 IIC_LdStLoadUpd
  { 2, 207, 210, 1201, 1203 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 0, 0, 0, 0, 0 }, // 39 IIC_LdStLFD
  { 0, 0, 0, 0, 0 }, // 40 IIC_LdStLFDU
  { 0, 0, 0, 0, 0 }, // 41 IIC_LdStLFDUX
  { 1, 186, 188, 1203, 1205 }, // 42 IIC_LdStLHA
  { 1, 207, 210, 1205, 1207 }, // 43 IIC_LdStLHAU
  { 1, 207, 210, 1207, 1209 }, // 44 IIC_LdStLHAUX
  { 1, 186, 188, 1209, 1211 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 190, 192, 1211, 1213 }, // 48 IIC_BrMCR
  { 1, 188, 190, 1213, 1216 }, // 49 IIC_BrMCRX
  { 1, 210, 212, 1216, 1218 }, // 50 IIC_SprMFCR
  { 1, 188, 190, 1218, 1220 }, // 51 IIC_SprMFSPR
  { 0, 0, 0, 0, 0 }, // 52 IIC_IntMFFS
  { 1, 212, 214, 1220, 1222 }, // 53 IIC_SprMFMSR
  { 1, 210, 212, 1222, 1224 }, // 54 IIC_SprMFCRF
  { 1, 212, 214, 1224, 1226 }, // 55 IIC_SprMFPMR
  { 1, 212, 214, 1226, 1228 }, // 56 IIC_SprMFSR
  { 1, 212, 214, 1228, 1230 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 188, 190, 1230, 1232 }, // 59 IIC_SprMTSPR
  { 0, 0, 0, 0, 0 }, // 60 IIC_IntMTFSB0
  { 1, 214, 216, 1232, 1234 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 1, 201, 203, 1234, 1236 }, // 63 IIC_SprMTPMR
  { 1, 201, 203, 1236, 1238 }, // 64 IIC_SprMTSR
  { 1, 199, 201, 1238, 1241 }, // 65 IIC_IntMulHW
  { 1, 199, 201, 1241, 1244 }, // 66 IIC_IntMulHWU
  { 1, 199, 201, 1244, 1247 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 0, 0, 0, 0, 0 }, // 69 IIC_LdStSTFD
  { 0, 0, 0, 0, 0 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 188, 190, 1247, 1250 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 188, 190, 1250, 1253 }, // 83 IIC_IntShift
  { 1, 186, 188, 1253, 1255 }, // 84 IIC_LdStSTWCX
  { 2, 207, 210, 1255, 1257 }, // 85 IIC_LdStSTU
  { 2, 207, 210, 1257, 1259 }, // 86 IIC_LdStSTUX
  { 0, 0, 0, 0, 0 }, // 87 IIC_LdStSTD
  { 0, 0, 0, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 216, 218, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 218, 220, 1259, 1261 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 1, 201, 203, 1261, 1264 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 188, 190, 1264, 1266 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 0, 0, 0, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 0, 0, 0, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 188, 190, 1266, 1269 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 218, 220, 1269, 1271 }, // 105 TW_TWI
  { 0, 0, 0, 0, 0 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 0, 0, 0, 0, 0 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 188, 190, 1271, 1274 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 201, 203, 1274, 1277 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 188, 190, 1277, 1280 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 188, 190, 1280, 1283 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 188, 190, 1283, 1286 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 190, 192, 1286, 1288 }, // 118 MCRF
  { 1, 188, 190, 1288, 1291 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 190, 192, 1291, 1293 }, // 121 RFEBB
  { 1, 188, 190, 1293, 1295 }, // 122 TBEGIN_TRECHKPT
  { 1, 186, 188, 1295, 1297 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 188, 190, 1297, 1300 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 210, 212, 1300, 1302 }, // 127 MFOCRF_MFOCRF8
  { 1, 188, 190, 1302, 1305 }, // 128 MTOCRF_MTOCRF8
  { 1, 190, 192, 1305, 1308 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 188, 190, 1308, 1311 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 0, 0, 0, 0, 0 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 188, 190, 1311, 1314 }, // 132 SRAW_SRAWI
  { 1, 188, 190, 1314, 1318 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 188, 190, 1318, 1320 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 186, 188, 1320, 1322 }, // 136 MFVSCR
  { 1, 186, 188, 1322, 1324 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 201, 203, 1324, 1327 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 0, 0, 0, 0, 0 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 201, 203, 1327, 1330 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 199, 201, 1330, 1333 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 199, 201, 1333, 1336 }, // 146 MULHDU_MULHWU
  { 1, 199, 201, 1336, 1339 }, // 147 MULLI_MULLI8
  { 0, 0, 0, 0, 0 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 0, 0, 0, 0, 0 }, // 149 FADD_FSUB
  { 0, 0, 0, 0, 0 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 0, 0, 0, 0, 0 }, // 152 FSELD_rec_FSELS_rec
  { 1, 199, 201, 1339, 1342 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 199, 201, 1342, 1345 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 0, 0, 0, 0, 0 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 0, 0, 0, 0, 0 }, // 157 FADD_rec_FSUB_rec
  { 0, 0, 0, 0, 0 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 186, 188, 1345, 1347 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 201, 203, 1347, 1350 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 0, 0, 0, 0, 0 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 186, 188, 1350, 1352 }, // 171 LXVL_LXVLL
  { 1, 186, 188, 1352, 1354 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 0, 0, 0, 0, 0 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 186, 188, 1354, 1356 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 186, 188, 1356, 1358 }, // 178 ICBI_ICBIEP
  { 1, 186, 188, 1358, 1360 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 205, 207, 1360, 1363 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 0, 0, 0, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 216, 218, 0, 0 }, // 186 TLBSYNC
  { 1, 186, 188, 0, 0 }, // 187 SYNC
  { 1, 186, 188, 1363, 1365 }, // 188 LMW
  { 0, 0, 0, 0, 0 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 2, 207, 210, 1365, 1367 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 2, 207, 210, 1367, 1369 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 188, 190, 1369, 1371 }, // 198 TEND
  { 1, 186, 188, 1371, 1373 }, // 199 STBCX_STHCX_STWCX
  { 0, 0, 0, 0, 0 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 186, 188, 1373, 1375 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 188, 190, 1375, 1377 }, // 205 TCHECK
  { 0, 0, 0, 0, 0 }, // 206 LFIWAX
  { 0, 0, 0, 0, 0 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 0, 0, 0, 0, 0 }, // 209 LFSX_LFS
  { 0, 0, 0, 0, 0 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 207, 210, 1377, 1379 }, // 212 LHAU_LHAU8
  { 1, 207, 210, 1379, 1381 }, // 213 LHAUX_LHAUX8_LWAUX
  { 0, 0, 0, 0, 0 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 0, 0, 0, 0, 0 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 186, 188, 1381, 1383 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 0, 0, 0, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 186, 188, 1383, 1385 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 186, 188, 1385, 1387 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 186, 188, 1387, 1389 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 0, 0, 0, 0, 0 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 186, 188, 1389, 1391 }, // 225 STXVL_STXVLL
  { 1, 188, 190, 1391, 1393 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 188, 190, 1393, 1396 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 1, 212, 214, 1396, 1398 }, // 228 MFPMR
  { 1, 201, 203, 1398, 1400 }, // 229 MTPMR
  { 1, 212, 214, 1400, 1402 }, // 230 MFTB_MFTB8
  { 1, 188, 190, 1402, 1404 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 212, 214, 1404, 1406 }, // 232 MFMSR
  { 1, 214, 216, 1406, 1408 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 188, 190, 1408, 1410 }, // 235 MTSPR_MTSPR8
  { 1, 192, 195, 1410, 1413 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 192, 195, 1413, 1416 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 192, 195, 1416, 1419 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 192, 195, 1419, 1422 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 188, 190, 1422, 1425 }, // 244 MTCRF_MTCRF8
  { 1, 188, 190, 1425, 1428 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 0, 0, 0, 0, 0 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 190, 192, 1428, 1430 }, // 247 MCRFS
  { 0, 0, 0, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 0, 0, 0, 0, 0 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 188, 190, 1430, 1433 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 188, 190, 1433, 1436 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 188, 190, 1436, 1439 }, // 254 SRAWI_rec_SRAW_rec
  { 0, 0, 0, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 210, 212, 1439, 1441 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 197, 199, 1441, 1444 }, // 259 FDIV
  { 1, 197, 199, 1444, 1447 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 197, 199, 1447, 1450 }, // 269 XSDIVDP
  { 1, 203, 205, 1450, 1453 }, // 270 FDIVS
  { 1, 203, 205, 1453, 1456 }, // 271 FDIVS_rec
  { 1, 203, 205, 1456, 1459 }, // 272 XSDIVSP
  { 1, 203, 205, 1459, 1462 }, // 273 XVDIVSP
  { 1, 197, 199, 1462, 1465 }, // 274 XVDIVDP
  { 0, 0, 0, 0, 0 }, // 275 LFSU
  { 0, 0, 0, 0, 0 }, // 276 LFSUX
  { 0, 0, 0, 0, 0 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 2, 207, 210, 1465, 1467 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 2, 207, 210, 1467, 1469 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 2, 207, 210, 1469, 1471 }, // 280 LBZU_LBZU8
  { 2, 207, 210, 1471, 1473 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 0, 0, 0, 0, 0 }, // 284 LFDU
  { 0, 0, 0, 0, 0 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 190, 192, 1473, 1475 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 190, 192, 1475, 1477 }, // 288 ADDPCIS
  { 1, 186, 188, 1477, 1479 }, // 289 LDAT_LWAT
  { 1, 186, 188, 1479, 1481 }, // 290 STDAT_STWAT
  { 1, 188, 190, 1481, 1484 }, // 291 BRINC
  { 1, 201, 203, 1484, 1487 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 199, 201, 1487, 1490 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 186, 188, 1490, 1492 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 186, 188, 1492, 1494 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 190, 192, 1494, 1496 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 186, 188, 1496, 1498 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 186, 188, 1498, 1500 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 0, 0, 0, 0, 0 }, // 301 LFDEPX
  { 0, 0, 0, 0, 0 }, // 302 STFDEPX
  { 1, 212, 214, 1500, 1502 }, // 303 MFSR_MFSRIN
  { 1, 201, 203, 1502, 1504 }, // 304 MTSR_MTSRIN
  { 1, 188, 190, 1504, 1506 }, // 305 MFDCR
  { 1, 188, 190, 1506, 1508 }, // 306 MTDCR
  { 1, 188, 190, 1508, 1511 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 214, 216, 1511, 1513 }, // 309 WRTEE_WRTEEI
  { 1, 186, 188, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 186, 188, 1513, 1515 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary PPCE500mcItineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 220, 222, 0, 0 }, // 1 IIC_LdStSync
  { 1, 222, 224, 1515, 1518 }, // 2 IIC_IntSimple
  { 1, 222, 224, 1518, 1521 }, // 3 IIC_IntGeneral
  { 1, 224, 226, 1521, 1523 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 222, 224, 1523, 1526 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 224, 226, 1526, 1529 }, // 10 IIC_BrCR
  { 0, 0, 0, 0, 0 }, // 11 IIC_LdStLD
  { 1, 220, 222, 1529, 1531 }, // 12 IIC_LdStDCBF
  { 1, 220, 222, 1531, 1533 }, // 13 IIC_LdStLoad
  { 0, 0, 0, 0, 0 }, // 14 IIC_IntDivD
  { 1, 226, 229, 1533, 1536 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 229, 231, 1536, 1539 }, // 17 IIC_FPAddSub
  { 1, 231, 233, 1539, 1542 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 233, 235, 1542, 1545 }, // 20 IIC_FPCompare
  { 1, 233, 235, 1545, 1548 }, // 21 IIC_FPGeneral
  { 0, 0, 0, 0, 0 }, // 22 IIC_VecGeneral
  { 0, 0, 0, 0, 0 }, // 23 IIC_VecComplex
  { 1, 220, 222, 1548, 1550 }, // 24 IIC_LdStStore
  { 0, 0, 0, 0, 0 }, // 25 IIC_IntRotateDI
  { 1, 235, 237, 1550, 1553 }, // 26 IIC_FPDivS
  { 1, 229, 231, 1553, 1557 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 220, 222, 1557, 1559 }, // 30 IIC_LdStICBI
  { 1, 222, 224, 1559, 1563 }, // 31 IIC_IntISEL
  { 0, 0, 0, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 237, 239, 1563, 1566 }, // 33 IIC_LdStLWARX
  { 2, 239, 242, 1566, 1568 }, // 34 IIC_LdStLoadUpd
  { 2, 239, 242, 1568, 1570 }, // 35 IIC_LdStLoadUpdX
  { 0, 0, 0, 0, 0 }, // 36 IIC_LdStLDARX
  { 0, 0, 0, 0, 0 }, // 37 IIC_LdStLDU
  { 0, 0, 0, 0, 0 }, // 38 IIC_LdStLDUX
  { 1, 220, 222, 1570, 1573 }, // 39 IIC_LdStLFD
  { 2, 239, 242, 1573, 1576 }, // 40 IIC_LdStLFDU
  { 2, 239, 242, 1576, 1579 }, // 41 IIC_LdStLFDUX
  { 1, 220, 222, 1579, 1581 }, // 42 IIC_LdStLHA
  { 1, 239, 242, 1581, 1583 }, // 43 IIC_LdStLHAU
  { 1, 239, 242, 1583, 1585 }, // 44 IIC_LdStLHAUX
  { 1, 220, 222, 1585, 1587 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 0, 0, 0, 0, 0 }, // 47 IIC_IntMulHD
  { 1, 224, 226, 1587, 1589 }, // 48 IIC_BrMCR
  { 1, 222, 224, 1589, 1592 }, // 49 IIC_BrMCRX
  { 1, 242, 244, 1592, 1594 }, // 50 IIC_SprMFCR
  { 1, 222, 224, 1594, 1596 }, // 51 IIC_SprMFSPR
  { 1, 244, 246, 1596, 1597 }, // 52 IIC_IntMFFS
  { 1, 246, 248, 1597, 1599 }, // 53 IIC_SprMFMSR
  { 1, 242, 244, 1599, 1601 }, // 54 IIC_SprMFCRF
  { 1, 246, 248, 1601, 1603 }, // 55 IIC_SprMFPMR
  { 1, 246, 248, 1603, 1605 }, // 56 IIC_SprMFSR
  { 1, 246, 248, 1605, 1607 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 222, 224, 1607, 1609 }, // 59 IIC_SprMTSPR
  { 1, 244, 246, 1609, 1612 }, // 60 IIC_IntMTFSB0
  { 1, 248, 250, 1612, 1614 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 1, 250, 252, 1614, 1616 }, // 63 IIC_SprMTPMR
  { 1, 250, 252, 1616, 1618 }, // 64 IIC_SprMTSR
  { 1, 252, 254, 1618, 1621 }, // 65 IIC_IntMulHW
  { 1, 252, 254, 1621, 1624 }, // 66 IIC_IntMulHWU
  { 1, 252, 254, 1624, 1627 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 1, 220, 222, 1627, 1630 }, // 69 IIC_LdStSTFD
  { 2, 239, 242, 1630, 1633 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 0, 0, 0, 0, 0 }, // 73 IIC_IntRotateD
  { 1, 222, 224, 1633, 1636 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 222, 224, 1636, 1639 }, // 83 IIC_IntShift
  { 1, 220, 222, 1639, 1641 }, // 84 IIC_LdStSTWCX
  { 2, 239, 242, 1641, 1643 }, // 85 IIC_LdStSTU
  { 2, 239, 242, 1643, 1645 }, // 86 IIC_LdStSTUX
  { 0, 0, 0, 0, 0 }, // 87 IIC_LdStSTD
  { 0, 0, 0, 0, 0 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 254, 256, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 256, 258, 1645, 1647 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 0, 0, 0, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 222, 224, 1647, 1649 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 244, 246, 1649, 1652 }, // 101 MTFSB0_MTFSB1
  { 1, 244, 246, 1652, 1653 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 222, 224, 1653, 1656 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 256, 258, 1656, 1658 }, // 105 TW_TWI
  { 1, 233, 235, 1658, 1661 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 233, 235, 1661, 1664 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 222, 224, 1664, 1667 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 0, 0, 0, 0, 0 }, // 111 SLD_SRD_SRAD
  { 0, 0, 0, 0, 0 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 0, 0, 0, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 222, 224, 1667, 1670 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 222, 224, 1670, 1673 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 222, 224, 1673, 1676 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 224, 226, 1676, 1678 }, // 118 MCRF
  { 1, 222, 224, 1678, 1681 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 224, 226, 1681, 1683 }, // 121 RFEBB
  { 1, 222, 224, 1683, 1685 }, // 122 TBEGIN_TRECHKPT
  { 1, 220, 222, 1685, 1687 }, // 123 WAIT
  { 0, 0, 0, 0, 0 }, // 124 RLDCL_RLDCR
  { 1, 222, 224, 1687, 1690 }, // 125 RLWIMI_RLWIMI8
  { 0, 0, 0, 0, 0 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 242, 244, 1690, 1692 }, // 127 MFOCRF_MFOCRF8
  { 1, 222, 224, 1692, 1695 }, // 128 MTOCRF_MTOCRF8
  { 1, 224, 226, 1695, 1698 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 222, 224, 1698, 1701 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 233, 235, 1701, 1704 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 222, 224, 1704, 1707 }, // 132 SRAW_SRAWI
  { 1, 222, 224, 1707, 1711 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 222, 224, 1711, 1713 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 220, 222, 1713, 1715 }, // 136 MFVSCR
  { 1, 220, 222, 1715, 1717 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 0, 0, 0, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 233, 235, 1717, 1720 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 0, 0, 0, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 0, 0, 0, 0, 0 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 252, 254, 1720, 1723 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 252, 254, 1723, 1726 }, // 146 MULHDU_MULHWU
  { 1, 252, 254, 1726, 1729 }, // 147 MULLI_MULLI8
  { 1, 233, 235, 1729, 1732 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 229, 231, 1732, 1735 }, // 149 FADD_FSUB
  { 1, 229, 231, 1735, 1739 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 233, 235, 1739, 1742 }, // 152 FSELD_rec_FSELS_rec
  { 1, 252, 254, 1742, 1745 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 252, 254, 1745, 1748 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 0, 0, 0, 0, 0 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 233, 235, 1748, 1751 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 229, 231, 1751, 1754 }, // 157 FADD_rec_FSUB_rec
  { 1, 229, 231, 1754, 1758 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 220, 222, 1758, 1760 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 0, 0, 0, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 233, 235, 1760, 1763 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 220, 222, 1763, 1765 }, // 171 LXVL_LXVLL
  { 1, 220, 222, 1765, 1767 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 220, 222, 1767, 1770 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 220, 222, 1770, 1772 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 220, 222, 1772, 1774 }, // 178 ICBI_ICBIEP
  { 1, 220, 222, 1774, 1776 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 237, 239, 1776, 1779 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 0, 0, 0, 0, 0 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 0, 0, 0, 0, 0 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 0, 0, 0, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 254, 256, 0, 0 }, // 186 TLBSYNC
  { 1, 220, 222, 0, 0 }, // 187 SYNC
  { 1, 220, 222, 1779, 1781 }, // 188 LMW
  { 1, 220, 222, 1781, 1784 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 2, 239, 242, 1784, 1786 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 2, 239, 242, 1786, 1788 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 222, 224, 1788, 1790 }, // 198 TEND
  { 1, 220, 222, 1790, 1792 }, // 199 STBCX_STHCX_STWCX
  { 0, 0, 0, 0, 0 }, // 200 STDCX
  { 0, 0, 0, 0, 0 }, // 201 LDMX
  { 1, 220, 222, 1792, 1794 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 222, 224, 1794, 1796 }, // 205 TCHECK
  { 1, 220, 222, 1796, 1799 }, // 206 LFIWAX
  { 1, 220, 222, 1799, 1802 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 220, 222, 1802, 1805 }, // 209 LFSX_LFS
  { 1, 220, 222, 1805, 1808 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 239, 242, 1808, 1810 }, // 212 LHAU_LHAU8
  { 1, 239, 242, 1810, 1812 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 220, 222, 1812, 1815 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 220, 222, 1815, 1818 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 220, 222, 1818, 1820 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 0, 0, 0, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 220, 222, 1820, 1822 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 220, 222, 1822, 1824 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 220, 222, 1824, 1826 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 220, 222, 1826, 1829 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 220, 222, 1829, 1831 }, // 225 STXVL_STXVLL
  { 1, 222, 224, 1831, 1833 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 222, 224, 1833, 1836 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 1, 246, 248, 1836, 1838 }, // 228 MFPMR
  { 1, 250, 252, 1838, 1840 }, // 229 MTPMR
  { 1, 246, 248, 1840, 1842 }, // 230 MFTB_MFTB8
  { 1, 222, 224, 1842, 1844 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 1, 246, 248, 1844, 1846 }, // 232 MFMSR
  { 1, 248, 250, 1846, 1848 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 222, 224, 1848, 1850 }, // 235 MTSPR_MTSPR8
  { 1, 226, 229, 1850, 1853 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 226, 229, 1853, 1856 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 0, 0, 0, 0, 0 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 0, 0, 0, 0, 0 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 226, 229, 1856, 1859 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 0, 0, 0, 0, 0 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 226, 229, 1859, 1862 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 0, 0, 0, 0, 0 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 222, 224, 1862, 1865 }, // 244 MTCRF_MTCRF8
  { 1, 222, 224, 1865, 1868 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 233, 235, 1868, 1871 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 224, 226, 1871, 1873 }, // 247 MCRFS
  { 1, 244, 246, 1873, 1874 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 244, 246, 1874, 1877 }, // 249 MTFSFb
  { 0, 0, 0, 0, 0 }, // 250 RLDCL_rec_RLDCR_rec
  { 0, 0, 0, 0, 0 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 222, 224, 1877, 1880 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 222, 224, 1880, 1883 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 222, 224, 1883, 1886 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 244, 246, 1886, 1887 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 242, 244, 1887, 1889 }, // 256 MFCR_MFCR8
  { 0, 0, 0, 0, 0 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 0, 0, 0, 0, 0 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 231, 233, 1889, 1892 }, // 259 FDIV
  { 1, 231, 233, 1892, 1895 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 231, 233, 1895, 1898 }, // 269 XSDIVDP
  { 1, 235, 237, 1898, 1901 }, // 270 FDIVS
  { 1, 235, 237, 1901, 1904 }, // 271 FDIVS_rec
  { 1, 235, 237, 1904, 1907 }, // 272 XSDIVSP
  { 1, 235, 237, 1907, 1910 }, // 273 XVDIVSP
  { 1, 231, 233, 1910, 1913 }, // 274 XVDIVDP
  { 2, 239, 242, 1913, 1916 }, // 275 LFSU
  { 2, 239, 242, 1916, 1919 }, // 276 LFSUX
  { 2, 239, 242, 1919, 1922 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 2, 239, 242, 1922, 1924 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 2, 239, 242, 1924, 1926 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 2, 239, 242, 1926, 1928 }, // 280 LBZU_LBZU8
  { 2, 239, 242, 1928, 1930 }, // 281 LBZUX_LBZUX8
  { 0, 0, 0, 0, 0 }, // 282 LDU
  { 0, 0, 0, 0, 0 }, // 283 LDUX
  { 2, 239, 242, 1930, 1933 }, // 284 LFDU
  { 2, 239, 242, 1933, 1936 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 224, 226, 1936, 1938 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 224, 226, 1938, 1940 }, // 288 ADDPCIS
  { 1, 220, 222, 1940, 1942 }, // 289 LDAT_LWAT
  { 1, 220, 222, 1942, 1944 }, // 290 STDAT_STWAT
  { 1, 222, 224, 1944, 1947 }, // 291 BRINC
  { 0, 0, 0, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 0, 0, 0, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 220, 222, 1947, 1949 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 220, 222, 1949, 1951 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 224, 226, 1951, 1953 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 220, 222, 1953, 1955 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 220, 222, 1955, 1957 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 220, 222, 1957, 1960 }, // 301 LFDEPX
  { 1, 220, 222, 1960, 1963 }, // 302 STFDEPX
  { 1, 246, 248, 1963, 1965 }, // 303 MFSR_MFSRIN
  { 1, 250, 252, 1965, 1967 }, // 304 MTSR_MTSRIN
  { 1, 222, 224, 1967, 1969 }, // 305 MFDCR
  { 1, 222, 224, 1969, 1971 }, // 306 MTDCR
  { 1, 222, 224, 1971, 1974 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 248, 250, 1974, 1976 }, // 309 WRTEE_WRTEEI
  { 1, 220, 222, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 220, 222, 1976, 1978 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary PPCE5500Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 1, 258, 260, 0, 0 }, // 1 IIC_LdStSync
  { 1, 260, 262, 1978, 1981 }, // 2 IIC_IntSimple
  { 1, 260, 262, 1981, 1984 }, // 3 IIC_IntGeneral
  { 1, 262, 264, 1984, 1986 }, // 4 IIC_BrB
  { 0, 0, 0, 0, 0 }, // 5 IIC_VecFP
  { 1, 260, 262, 1986, 1989 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 262, 264, 1989, 1992 }, // 10 IIC_BrCR
  { 1, 258, 260, 1992, 1994 }, // 11 IIC_LdStLD
  { 1, 258, 260, 1994, 1996 }, // 12 IIC_LdStDCBF
  { 1, 258, 260, 1996, 1998 }, // 13 IIC_LdStLoad
  { 1, 264, 267, 1998, 2001 }, // 14 IIC_IntDivD
  { 1, 267, 270, 2001, 2004 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 270, 272, 2004, 2007 }, // 17 IIC_FPAddSub
  { 1, 272, 274, 2007, 2010 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 270, 272, 2010, 2013 }, // 20 IIC_FPCompare
  { 1, 270, 272, 2013, 2016 }, // 21 IIC_FPGeneral
  { 0, 0, 0, 0, 0 }, // 22 IIC_VecGeneral
  { 0, 0, 0, 0, 0 }, // 23 IIC_VecComplex
  { 1, 258, 260, 2016, 2018 }, // 24 IIC_LdStStore
  { 1, 260, 262, 2018, 2021 }, // 25 IIC_IntRotateDI
  { 1, 274, 276, 2021, 2024 }, // 26 IIC_FPDivS
  { 1, 270, 272, 2024, 2028 }, // 27 IIC_FPFused
  { 0, 0, 0, 0, 0 }, // 28 IIC_FPSqrtD
  { 0, 0, 0, 0, 0 }, // 29 IIC_FPSqrtS
  { 1, 258, 260, 2028, 2030 }, // 30 IIC_LdStICBI
  { 1, 260, 262, 2030, 2034 }, // 31 IIC_IntISEL
  { 0, 0, 0, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 276, 278, 2034, 2037 }, // 33 IIC_LdStLWARX
  { 2, 278, 281, 2037, 2039 }, // 34 IIC_LdStLoadUpd
  { 2, 278, 281, 2039, 2041 }, // 35 IIC_LdStLoadUpdX
  { 1, 276, 278, 2041, 2043 }, // 36 IIC_LdStLDARX
  { 2, 278, 281, 2043, 2045 }, // 37 IIC_LdStLDU
  { 2, 278, 281, 2045, 2047 }, // 38 IIC_LdStLDUX
  { 2, 258, 260, 2047, 2050 }, // 39 IIC_LdStLFD
  { 2, 278, 281, 2050, 2053 }, // 40 IIC_LdStLFDU
  { 2, 278, 281, 2053, 2056 }, // 41 IIC_LdStLFDUX
  { 1, 258, 260, 2056, 2058 }, // 42 IIC_LdStLHA
  { 2, 278, 281, 2058, 2060 }, // 43 IIC_LdStLHAU
  { 2, 278, 281, 2060, 2062 }, // 44 IIC_LdStLHAUX
  { 1, 281, 283, 2062, 2064 }, // 45 IIC_LdStLMW
  { 0, 0, 0, 0, 0 }, // 46 IIC_LdStLWA
  { 1, 283, 286, 2064, 2067 }, // 47 IIC_IntMulHD
  { 1, 262, 264, 2067, 2069 }, // 48 IIC_BrMCR
  { 1, 286, 288, 2069, 2072 }, // 49 IIC_BrMCRX
  { 1, 288, 290, 2072, 2074 }, // 50 IIC_SprMFCR
  { 1, 286, 288, 2074, 2075 }, // 51 IIC_SprMFSPR
  { 1, 270, 272, 2075, 2076 }, // 52 IIC_IntMFFS
  { 0, 0, 0, 0, 0 }, // 53 IIC_SprMFMSR
  { 1, 288, 290, 2076, 2078 }, // 54 IIC_SprMFCRF
  { 1, 290, 292, 2078, 2080 }, // 55 IIC_SprMFPMR
  { 0, 0, 0, 0, 0 }, // 56 IIC_SprMFSR
  { 1, 290, 292, 2080, 2082 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 286, 288, 2082, 2083 }, // 59 IIC_SprMTSPR
  { 1, 292, 294, 2083, 2086 }, // 60 IIC_IntMTFSB0
  { 1, 294, 296, 2086, 2088 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 1, 286, 288, 2088, 2089 }, // 63 IIC_SprMTPMR
  { 0, 0, 0, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 296, 299, 2089, 2092 }, // 65 IIC_IntMulHW
  { 1, 296, 299, 2092, 2095 }, // 66 IIC_IntMulHWU
  { 1, 283, 286, 2095, 2098 }, // 67 IIC_IntMulLI
  { 0, 0, 0, 0, 0 }, // 68 IIC_VecPerm
  { 1, 258, 260, 2098, 2101 }, // 69 IIC_LdStSTFD
  { 2, 278, 281, 2101, 2104 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 1, 299, 301, 2104, 2107 }, // 73 IIC_IntRotateD
  { 1, 260, 262, 2107, 2110 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 299, 301, 2110, 2113 }, // 83 IIC_IntShift
  { 1, 258, 260, 2113, 2115 }, // 84 IIC_LdStSTWCX
  { 2, 278, 281, 2115, 2117 }, // 85 IIC_LdStSTU
  { 2, 278, 281, 2117, 2119 }, // 86 IIC_LdStSTUX
  { 1, 258, 260, 2119, 2121 }, // 87 IIC_LdStSTD
  { 1, 258, 260, 2121, 2123 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 0, 0, 0, 0, 0 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 1, 301, 303, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 303, 305, 2123, 2125 }, // 95 IIC_IntTrapW
  { 0, 0, 0, 0, 0 }, // 96 IIC_VecFPCompare
  { 0, 0, 0, 0, 0 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 0, 0, 0, 0, 0 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 0, 0, 0, 0, 0 }, // 99 XXSEL
  { 1, 286, 288, 2125, 2126 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 1, 292, 294, 2126, 2129 }, // 101 MTFSB0_MTFSB1
  { 1, 270, 272, 2129, 2130 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 260, 262, 2130, 2133 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 0, 0, 0, 0, 0 }, // 104 TD_TDI
  { 1, 303, 305, 2133, 2135 }, // 105 TW_TWI
  { 1, 270, 272, 2135, 2138 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 0, 0, 0, 0, 0 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 0, 0, 0, 0, 0 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 270, 272, 2138, 2141 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 260, 262, 2141, 2144 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 1, 299, 301, 2144, 2147 }, // 111 SLD_SRD_SRAD
  { 1, 260, 262, 2147, 2150 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 0, 0, 0, 0, 0 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 260, 262, 2150, 2153 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 260, 262, 2153, 2156 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 260, 262, 2156, 2159 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 262, 264, 2159, 2161 }, // 118 MCRF
  { 1, 286, 288, 2161, 2164 }, // 119 MCRXRX
  { 0, 0, 0, 0, 0 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 262, 264, 2164, 2166 }, // 121 RFEBB
  { 1, 286, 288, 2166, 2167 }, // 122 TBEGIN_TRECHKPT
  { 1, 258, 260, 2167, 2169 }, // 123 WAIT
  { 1, 299, 301, 2169, 2172 }, // 124 RLDCL_RLDCR
  { 1, 260, 262, 2172, 2175 }, // 125 RLWIMI_RLWIMI8
  { 1, 260, 262, 2175, 2178 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 288, 290, 2178, 2180 }, // 127 MFOCRF_MFOCRF8
  { 1, 286, 288, 2180, 2183 }, // 128 MTOCRF_MTOCRF8
  { 1, 262, 264, 2183, 2186 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 260, 262, 2186, 2189 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 270, 272, 2189, 2192 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 299, 301, 2192, 2195 }, // 132 SRAW_SRAWI
  { 1, 260, 262, 2195, 2199 }, // 133 ISEL_ISEL8
  { 0, 0, 0, 0, 0 }, // 134 XSIEXPDP
  { 1, 286, 288, 2199, 2200 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 258, 260, 2200, 2202 }, // 136 MFVSCR
  { 1, 258, 260, 2202, 2204 }, // 137 MTVSCR
  { 0, 0, 0, 0, 0 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 0, 0, 0, 0, 0 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 0, 0, 0, 0, 0 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 270, 272, 2204, 2207 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 0, 0, 0, 0, 0 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 0, 0, 0, 0, 0 }, // 143 VSUBFP_VMULUWM
  { 1, 283, 286, 2207, 2210 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 296, 299, 2210, 2213 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 296, 299, 2213, 2216 }, // 146 MULHDU_MULHWU
  { 1, 283, 286, 2216, 2219 }, // 147 MULLI_MULLI8
  { 1, 270, 272, 2219, 2222 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 270, 272, 2222, 2225 }, // 149 FADD_FSUB
  { 1, 270, 272, 2225, 2229 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 0, 0, 0, 0, 0 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 270, 272, 2229, 2232 }, // 152 FSELD_rec_FSELS_rec
  { 1, 296, 299, 2232, 2235 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 296, 299, 2235, 2238 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 1, 283, 286, 2238, 2241 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 270, 272, 2241, 2244 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 270, 272, 2244, 2247 }, // 157 FADD_rec_FSUB_rec
  { 1, 270, 272, 2247, 2251 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 0, 0, 0, 0, 0 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 258, 260, 2251, 2253 }, // 160 LVSL_LVSR
  { 0, 0, 0, 0, 0 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 0, 0, 0, 0, 0 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 0, 0, 0, 0, 0 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 270, 272, 2253, 2256 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 0, 0, 0, 0, 0 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 0, 0, 0, 0, 0 }, // 166 BCDCTSQ_rec
  { 0, 0, 0, 0, 0 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 0, 0, 0, 0, 0 }, // 168 BCDCFSQ_rec
  { 0, 0, 0, 0, 0 }, // 169 XSDIVQP_XSDIVQPO
  { 0, 0, 0, 0, 0 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 258, 260, 2256, 2258 }, // 171 LXVL_LXVLL
  { 1, 258, 260, 2258, 2260 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 2, 258, 260, 2260, 2263 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 1, 258, 260, 2263, 2265 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 1, 258, 260, 2265, 2267 }, // 178 ICBI_ICBIEP
  { 1, 258, 260, 2267, 2269 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 276, 278, 2269, 2272 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 1, 258, 260, 2272, 2274 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 1, 276, 278, 2274, 2276 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 0, 0, 0, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 1, 301, 303, 0, 0 }, // 186 TLBSYNC
  { 1, 258, 260, 0, 0 }, // 187 SYNC
  { 1, 281, 283, 2276, 2278 }, // 188 LMW
  { 2, 258, 260, 2278, 2281 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 2, 278, 281, 2281, 2283 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 2, 278, 281, 2283, 2285 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 286, 288, 2285, 2286 }, // 198 TEND
  { 1, 258, 260, 2286, 2288 }, // 199 STBCX_STHCX_STWCX
  { 1, 258, 260, 2288, 2290 }, // 200 STDCX
  { 1, 258, 260, 2290, 2292 }, // 201 LDMX
  { 1, 258, 260, 2292, 2294 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 0, 0, 0, 0, 0 }, // 204 LWA_LWA_32
  { 1, 286, 288, 2294, 2295 }, // 205 TCHECK
  { 2, 258, 260, 2295, 2298 }, // 206 LFIWAX
  { 2, 258, 260, 2298, 2301 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 2, 258, 260, 2301, 2304 }, // 209 LFSX_LFS
  { 2, 258, 260, 2304, 2307 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 2, 278, 281, 2307, 2309 }, // 212 LHAU_LHAU8
  { 2, 278, 281, 2309, 2311 }, // 213 LHAUX_LHAUX8_LWAUX
  { 2, 258, 260, 2311, 2314 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 258, 260, 2314, 2317 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 258, 260, 2317, 2319 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 258, 260, 2319, 2321 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 258, 260, 2321, 2323 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 281, 283, 2323, 2325 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 258, 260, 2325, 2327 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 258, 260, 2327, 2330 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 258, 260, 2330, 2332 }, // 225 STXVL_STXVLL
  { 1, 286, 288, 2332, 2333 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 260, 262, 2333, 2336 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 1, 290, 292, 2336, 2338 }, // 228 MFPMR
  { 1, 286, 288, 2338, 2339 }, // 229 MTPMR
  { 1, 290, 292, 2339, 2341 }, // 230 MFTB_MFTB8
  { 1, 286, 288, 2341, 2342 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 0, 0, 0, 0, 0 }, // 232 MFMSR
  { 1, 294, 296, 2342, 2344 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 286, 288, 2344, 2345 }, // 235 MTSPR_MTSPR8
  { 1, 267, 270, 2345, 2348 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 267, 270, 2348, 2351 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 1, 264, 267, 2351, 2354 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 1, 264, 267, 2354, 2357 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 267, 270, 2357, 2360 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 1, 264, 267, 2360, 2363 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 267, 270, 2363, 2366 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 1, 264, 267, 2366, 2369 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 286, 288, 2369, 2372 }, // 244 MTCRF_MTCRF8
  { 1, 260, 262, 2372, 2375 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 270, 272, 2375, 2378 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 262, 264, 2378, 2380 }, // 247 MCRFS
  { 1, 270, 272, 2380, 2381 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 1, 292, 294, 2381, 2384 }, // 249 MTFSFb
  { 1, 299, 301, 2384, 2387 }, // 250 RLDCL_rec_RLDCR_rec
  { 1, 260, 262, 2387, 2390 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 260, 262, 2390, 2393 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 260, 262, 2393, 2396 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 299, 301, 2396, 2399 }, // 254 SRAWI_rec_SRAW_rec
  { 1, 270, 272, 2399, 2400 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 288, 290, 2400, 2402 }, // 256 MFCR_MFCR8
  { 1, 260, 262, 2402, 2405 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 1, 299, 301, 2405, 2408 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 272, 274, 2408, 2411 }, // 259 FDIV
  { 1, 272, 274, 2411, 2414 }, // 260 FDIV_rec
  { 0, 0, 0, 0, 0 }, // 261 XSSQRTDP
  { 0, 0, 0, 0, 0 }, // 262 FSQRT
  { 0, 0, 0, 0, 0 }, // 263 XVSQRTDP
  { 0, 0, 0, 0, 0 }, // 264 XVSQRTSP
  { 0, 0, 0, 0, 0 }, // 265 FSQRT_rec
  { 0, 0, 0, 0, 0 }, // 266 XSSQRTSP
  { 0, 0, 0, 0, 0 }, // 267 FSQRTS
  { 0, 0, 0, 0, 0 }, // 268 FSQRTS_rec
  { 1, 272, 274, 2414, 2417 }, // 269 XSDIVDP
  { 1, 274, 276, 2417, 2420 }, // 270 FDIVS
  { 1, 274, 276, 2420, 2423 }, // 271 FDIVS_rec
  { 1, 274, 276, 2423, 2426 }, // 272 XSDIVSP
  { 1, 274, 276, 2426, 2429 }, // 273 XVDIVSP
  { 1, 272, 274, 2429, 2432 }, // 274 XVDIVDP
  { 2, 278, 281, 2432, 2435 }, // 275 LFSU
  { 2, 278, 281, 2435, 2438 }, // 276 LFSUX
  { 2, 278, 281, 2438, 2441 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 2, 278, 281, 2441, 2443 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 2, 278, 281, 2443, 2445 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 2, 278, 281, 2445, 2447 }, // 280 LBZU_LBZU8
  { 2, 278, 281, 2447, 2449 }, // 281 LBZUX_LBZUX8
  { 2, 278, 281, 2449, 2451 }, // 282 LDU
  { 2, 278, 281, 2451, 2453 }, // 283 LDUX
  { 2, 278, 281, 2453, 2456 }, // 284 LFDU
  { 2, 278, 281, 2456, 2459 }, // 285 LFDUX
  { 0, 0, 0, 0, 0 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 262, 264, 2459, 2461 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 262, 264, 2461, 2463 }, // 288 ADDPCIS
  { 1, 258, 260, 2463, 2465 }, // 289 LDAT_LWAT
  { 1, 258, 260, 2465, 2467 }, // 290 STDAT_STWAT
  { 1, 260, 262, 2467, 2470 }, // 291 BRINC
  { 0, 0, 0, 0, 0 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 0, 0, 0, 0, 0 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 258, 260, 2470, 2472 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 258, 260, 2472, 2474 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 262, 264, 2474, 2476 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 258, 260, 2476, 2478 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 258, 260, 2478, 2480 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 2, 258, 260, 2480, 2483 }, // 301 LFDEPX
  { 1, 258, 260, 2483, 2486 }, // 302 STFDEPX
  { 0, 0, 0, 0, 0 }, // 303 MFSR_MFSRIN
  { 0, 0, 0, 0, 0 }, // 304 MTSR_MTSRIN
  { 1, 286, 288, 2486, 2487 }, // 305 MFDCR
  { 1, 286, 288, 2487, 2488 }, // 306 MTDCR
  { 1, 260, 262, 2488, 2491 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 1, 294, 296, 2491, 2493 }, // 309 WRTEE_WRTEEI
  { 1, 258, 260, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 1, 258, 260, 2493, 2495 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary P8Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 0, 0, 0, 0, 0 }, // 1 IIC_LdStSync
  { 1, 305, 307, 2495, 2498 }, // 2 IIC_IntSimple
  { 1, 305, 307, 2498, 2501 }, // 3 IIC_IntGeneral
  { 1, 307, 309, 2501, 2504 }, // 4 IIC_BrB
  { 1, 309, 311, 2504, 2507 }, // 5 IIC_VecFP
  { 1, 311, 313, 2507, 2510 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 313, 315, 2510, 2513 }, // 10 IIC_BrCR
  { 1, 315, 317, 2513, 2516 }, // 11 IIC_LdStLD
  { 0, 0, 0, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 315, 317, 2516, 2519 }, // 13 IIC_LdStLoad
  { 1, 317, 319, 2519, 2522 }, // 14 IIC_IntDivD
  { 1, 319, 321, 2522, 2525 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 321, 323, 2525, 2528 }, // 17 IIC_FPAddSub
  { 1, 321, 323, 2528, 2531 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 321, 323, 2531, 2534 }, // 20 IIC_FPCompare
  { 1, 321, 323, 2534, 2537 }, // 21 IIC_FPGeneral
  { 1, 309, 311, 2537, 2540 }, // 22 IIC_VecGeneral
  { 1, 309, 311, 2540, 2543 }, // 23 IIC_VecComplex
  { 1, 323, 326, 2543, 2546 }, // 24 IIC_LdStStore
  { 1, 311, 313, 2546, 2549 }, // 25 IIC_IntRotateDI
  { 1, 321, 323, 2549, 2552 }, // 26 IIC_FPDivS
  { 1, 321, 323, 2552, 2556 }, // 27 IIC_FPFused
  { 1, 321, 323, 2556, 2559 }, // 28 IIC_FPSqrtD
  { 1, 321, 323, 2559, 2562 }, // 29 IIC_FPSqrtS
  { 0, 0, 0, 0, 0 }, // 30 IIC_LdStICBI
  { 1, 326, 329, 2562, 2566 }, // 31 IIC_IntISEL
  { 0, 0, 0, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 329, 334, 2566, 2569 }, // 33 IIC_LdStLWARX
  { 1, 334, 338, 2569, 2573 }, // 34 IIC_LdStLoadUpd
  { 1, 334, 338, 2573, 2577 }, // 35 IIC_LdStLoadUpdX
  { 1, 338, 345, 2577, 2580 }, // 36 IIC_LdStLDARX
  { 1, 334, 338, 2580, 2584 }, // 37 IIC_LdStLDU
  { 1, 334, 338, 2584, 2588 }, // 38 IIC_LdStLDUX
  { 1, 345, 347, 2588, 2591 }, // 39 IIC_LdStLFD
  { 1, 347, 351, 2591, 2595 }, // 40 IIC_LdStLFDU
  { 1, 347, 351, 2595, 2599 }, // 41 IIC_LdStLFDUX
  { 1, 351, 355, 2599, 2602 }, // 42 IIC_LdStLHA
  { 1, 355, 360, 2602, 2606 }, // 43 IIC_LdStLHAU
  { 1, 360, 369, 2606, 2610 }, // 44 IIC_LdStLHAUX
  { 1, 315, 317, 2610, 2613 }, // 45 IIC_LdStLMW
  { 1, 369, 373, 2613, 2616 }, // 46 IIC_LdStLWA
  { 1, 311, 313, 2616, 2619 }, // 47 IIC_IntMulHD
  { 1, 373, 375, 2619, 2622 }, // 48 IIC_BrMCR
  { 1, 375, 377, 2622, 2624 }, // 49 IIC_BrMCRX
  { 1, 313, 315, 2624, 2626 }, // 50 IIC_SprMFCR
  { 0, 0, 0, 0, 0 }, // 51 IIC_SprMFSPR
  { 0, 0, 0, 0, 0 }, // 52 IIC_IntMFFS
  { 0, 0, 0, 0, 0 }, // 53 IIC_SprMFMSR
  { 1, 313, 315, 2626, 2628 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 0, 0, 0, 0, 0 }, // 56 IIC_SprMFSR
  { 0, 0, 0, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 375, 377, 2628, 2630 }, // 59 IIC_SprMTSPR
  { 0, 0, 0, 0, 0 }, // 60 IIC_IntMTFSB0
  { 0, 0, 0, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 0, 0, 0, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 311, 313, 2630, 2633 }, // 65 IIC_IntMulHW
  { 1, 311, 313, 2633, 2636 }, // 66 IIC_IntMulHWU
  { 1, 311, 313, 2636, 2639 }, // 67 IIC_IntMulLI
  { 1, 377, 379, 2639, 2642 }, // 68 IIC_VecPerm
  { 1, 379, 382, 2642, 2645 }, // 69 IIC_LdStSTFD
  { 1, 382, 387, 2645, 2649 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 1, 311, 313, 2649, 2652 }, // 73 IIC_IntRotateD
  { 1, 311, 313, 2652, 2655 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 311, 313, 2655, 2658 }, // 83 IIC_IntShift
  { 1, 387, 395, 2658, 2661 }, // 84 IIC_LdStSTWCX
  { 1, 395, 399, 2661, 2665 }, // 85 IIC_LdStSTU
  { 1, 399, 408, 2665, 2669 }, // 86 IIC_LdStSTUX
  { 1, 408, 411, 0, 0 }, // 87 IIC_LdStSTD
  { 1, 387, 395, 2669, 2672 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 1, 311, 313, 2672, 2674 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 0, 0, 0, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 311, 313, 2674, 2676 }, // 95 IIC_IntTrapW
  { 1, 309, 311, 2676, 2679 }, // 96 IIC_VecFPCompare
  { 1, 309, 311, 2679, 2682 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 1, 309, 311, 2682, 2685 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 1, 377, 379, 2685, 2688 }, // 99 XXSEL
  { 1, 375, 377, 2688, 2690 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 0, 0, 0, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 0, 0, 0, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 311, 313, 2690, 2693 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 1, 311, 313, 2693, 2695 }, // 104 TD_TDI
  { 1, 311, 313, 2695, 2697 }, // 105 TW_TWI
  { 1, 321, 323, 2697, 2700 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 1, 309, 311, 2700, 2703 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 1, 309, 311, 2703, 2706 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 321, 323, 2706, 2709 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 305, 307, 2709, 2712 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 1, 311, 313, 2712, 2715 }, // 111 SLD_SRD_SRAD
  { 1, 311, 313, 2715, 2718 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 309, 311, 2718, 2721 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 311, 313, 2721, 2724 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 305, 307, 2724, 2727 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 305, 307, 2727, 2730 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 373, 375, 2730, 2733 }, // 118 MCRF
  { 1, 375, 377, 2733, 2735 }, // 119 MCRXRX
  { 1, 309, 311, 2735, 2738 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 307, 309, 2738, 2741 }, // 121 RFEBB
  { 1, 375, 377, 2741, 2743 }, // 122 TBEGIN_TRECHKPT
  { 1, 315, 317, 2743, 2746 }, // 123 WAIT
  { 1, 311, 313, 2746, 2749 }, // 124 RLDCL_RLDCR
  { 1, 311, 313, 2749, 2752 }, // 125 RLWIMI_RLWIMI8
  { 1, 311, 313, 2752, 2755 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 313, 315, 2755, 2757 }, // 127 MFOCRF_MFOCRF8
  { 1, 375, 377, 2757, 2759 }, // 128 MTOCRF_MTOCRF8
  { 1, 313, 315, 2759, 2762 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 305, 307, 2762, 2765 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 321, 323, 2765, 2768 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 311, 313, 2768, 2771 }, // 132 SRAW_SRAWI
  { 1, 326, 329, 2771, 2775 }, // 133 ISEL_ISEL8
  { 1, 309, 311, 2775, 2778 }, // 134 XSIEXPDP
  { 1, 375, 377, 2778, 2780 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 323, 326, 2780, 2783 }, // 136 MFVSCR
  { 1, 315, 317, 2783, 2786 }, // 137 MTVSCR
  { 1, 309, 311, 2786, 2789 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 309, 311, 2789, 2792 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 1, 309, 311, 2792, 2795 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 321, 323, 2795, 2798 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 1, 309, 311, 2798, 2801 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 309, 311, 2801, 2804 }, // 143 VSUBFP_VMULUWM
  { 1, 311, 313, 2804, 2807 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 311, 313, 2807, 2810 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 311, 313, 2810, 2813 }, // 146 MULHDU_MULHWU
  { 1, 311, 313, 2813, 2816 }, // 147 MULLI_MULLI8
  { 1, 321, 323, 2816, 2819 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 321, 323, 2819, 2822 }, // 149 FADD_FSUB
  { 1, 321, 323, 2822, 2826 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 1, 309, 311, 2826, 2829 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 321, 323, 2829, 2832 }, // 152 FSELD_rec_FSELS_rec
  { 1, 311, 313, 2832, 2835 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 311, 313, 2835, 2838 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 1, 311, 313, 2838, 2841 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 321, 323, 2841, 2844 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 321, 323, 2844, 2847 }, // 157 FADD_rec_FSUB_rec
  { 1, 321, 323, 2847, 2851 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 1, 309, 311, 2851, 2854 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 315, 317, 2854, 2857 }, // 160 LVSL_LVSR
  { 1, 377, 379, 2857, 2860 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 1, 309, 311, 2860, 2863 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 309, 311, 2863, 2866 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 321, 323, 2866, 2869 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 1, 309, 311, 2869, 2872 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 1, 309, 311, 2872, 2875 }, // 166 BCDCTSQ_rec
  { 1, 309, 311, 2875, 2878 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 1, 309, 311, 2878, 2881 }, // 168 BCDCFSQ_rec
  { 1, 309, 311, 2881, 2884 }, // 169 XSDIVQP_XSDIVQPO
  { 1, 309, 311, 2884, 2887 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 315, 317, 2887, 2890 }, // 171 LXVL_LXVLL
  { 1, 315, 317, 2890, 2893 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 345, 347, 2893, 2896 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 0, 0, 0, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 0, 0, 0, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 315, 317, 2896, 2899 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 329, 334, 2899, 2902 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 1, 315, 317, 2902, 2905 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 1, 338, 345, 2905, 2908 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 0, 0, 0, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 0, 0, 0, 0, 0 }, // 186 TLBSYNC
  { 0, 0, 0, 0, 0 }, // 187 SYNC
  { 1, 315, 317, 2908, 2911 }, // 188 LMW
  { 1, 345, 347, 2911, 2914 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 334, 338, 2914, 2918 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 334, 338, 2918, 2922 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 375, 377, 2922, 2924 }, // 198 TEND
  { 1, 387, 395, 2924, 2927 }, // 199 STBCX_STHCX_STWCX
  { 1, 387, 395, 2927, 2930 }, // 200 STDCX
  { 1, 315, 317, 2930, 2933 }, // 201 LDMX
  { 1, 351, 355, 2933, 2936 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 1, 369, 373, 2936, 2939 }, // 204 LWA_LWA_32
  { 1, 375, 377, 2939, 2941 }, // 205 TCHECK
  { 1, 345, 347, 2941, 2944 }, // 206 LFIWAX
  { 1, 345, 347, 2944, 2947 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 345, 347, 2947, 2950 }, // 209 LFSX_LFS
  { 1, 345, 347, 2950, 2953 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 355, 360, 2953, 2957 }, // 212 LHAU_LHAU8
  { 1, 360, 369, 2957, 2961 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 345, 347, 2961, 2964 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 379, 382, 2964, 2967 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 323, 326, 2967, 2970 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 408, 411, 0, 0 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 315, 317, 2970, 2973 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 315, 317, 2973, 2976 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 323, 326, 2976, 2979 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 379, 382, 2979, 2982 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 315, 317, 2982, 2985 }, // 225 STXVL_STXVLL
  { 1, 375, 377, 2985, 2987 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 305, 307, 2987, 2990 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 0, 0, 0, 0, 0 }, // 230 MFTB_MFTB8
  { 0, 0, 0, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 0, 0, 0, 0, 0 }, // 232 MFMSR
  { 0, 0, 0, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 375, 377, 2990, 2992 }, // 235 MTSPR_MTSPR8
  { 1, 319, 321, 2992, 2995 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 319, 321, 2995, 2998 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 1, 317, 319, 2998, 3001 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 1, 317, 319, 3001, 3004 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 319, 321, 3004, 3007 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 1, 317, 319, 3007, 3010 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 319, 321, 3010, 3013 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 1, 317, 319, 3013, 3016 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 375, 377, 3016, 3018 }, // 244 MTCRF_MTCRF8
  { 1, 305, 307, 3018, 3021 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 321, 323, 3021, 3024 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 373, 375, 3024, 3027 }, // 247 MCRFS
  { 0, 0, 0, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 0, 0, 0, 0, 0 }, // 249 MTFSFb
  { 1, 311, 313, 3027, 3030 }, // 250 RLDCL_rec_RLDCR_rec
  { 1, 311, 313, 3030, 3033 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 311, 313, 3033, 3036 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 305, 307, 3036, 3039 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 311, 313, 3039, 3042 }, // 254 SRAWI_rec_SRAW_rec
  { 0, 0, 0, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 313, 315, 3042, 3044 }, // 256 MFCR_MFCR8
  { 1, 311, 313, 3044, 3047 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 1, 311, 313, 3047, 3050 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 321, 323, 3050, 3053 }, // 259 FDIV
  { 1, 321, 323, 3053, 3056 }, // 260 FDIV_rec
  { 1, 321, 323, 3056, 3059 }, // 261 XSSQRTDP
  { 1, 321, 323, 3059, 3062 }, // 262 FSQRT
  { 1, 321, 323, 3062, 3065 }, // 263 XVSQRTDP
  { 1, 321, 323, 3065, 3068 }, // 264 XVSQRTSP
  { 1, 321, 323, 3068, 3071 }, // 265 FSQRT_rec
  { 1, 321, 323, 3071, 3074 }, // 266 XSSQRTSP
  { 1, 321, 323, 3074, 3077 }, // 267 FSQRTS
  { 1, 321, 323, 3077, 3080 }, // 268 FSQRTS_rec
  { 1, 321, 323, 3080, 3083 }, // 269 XSDIVDP
  { 1, 321, 323, 3083, 3086 }, // 270 FDIVS
  { 1, 321, 323, 3086, 3089 }, // 271 FDIVS_rec
  { 1, 321, 323, 3089, 3092 }, // 272 XSDIVSP
  { 1, 321, 323, 3092, 3095 }, // 273 XVDIVSP
  { 1, 321, 323, 3095, 3098 }, // 274 XVDIVDP
  { 1, 347, 351, 3098, 3102 }, // 275 LFSU
  { 1, 347, 351, 3102, 3106 }, // 276 LFSUX
  { 1, 382, 387, 3106, 3110 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 395, 399, 3110, 3114 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 399, 408, 3114, 3118 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 334, 338, 3118, 3122 }, // 280 LBZU_LBZU8
  { 1, 334, 338, 3122, 3126 }, // 281 LBZUX_LBZUX8
  { 1, 334, 338, 3126, 3130 }, // 282 LDU
  { 1, 334, 338, 3130, 3134 }, // 283 LDUX
  { 1, 347, 351, 3134, 3138 }, // 284 LFDU
  { 1, 347, 351, 3138, 3142 }, // 285 LFDUX
  { 1, 309, 311, 3142, 3145 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 307, 309, 3145, 3148 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 307, 309, 3148, 3151 }, // 288 ADDPCIS
  { 1, 315, 317, 3151, 3154 }, // 289 LDAT_LWAT
  { 1, 323, 326, 3154, 3157 }, // 290 STDAT_STWAT
  { 1, 305, 307, 3157, 3160 }, // 291 BRINC
  { 1, 309, 311, 3160, 3163 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 309, 311, 3163, 3166 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 315, 317, 3166, 3169 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 323, 326, 3169, 3172 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 307, 309, 3172, 3175 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 315, 317, 3175, 3178 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 323, 326, 3178, 3181 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 345, 347, 3181, 3184 }, // 301 LFDEPX
  { 1, 379, 382, 3184, 3187 }, // 302 STFDEPX
  { 0, 0, 0, 0, 0 }, // 303 MFSR_MFSRIN
  { 0, 0, 0, 0, 0 }, // 304 MTSR_MTSRIN
  { 0, 0, 0, 0, 0 }, // 305 MFDCR
  { 1, 375, 377, 3187, 3189 }, // 306 MTDCR
  { 1, 305, 307, 3189, 3192 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 0, 0, 0, 0, 0 }, // 309 WRTEE_WRTEEI
  { 0, 0, 0, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 0, 0, 0, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

static const llvm::InstrItinerary P7Itineraries[] = {
  { 0, 0, 0, 0, 0 }, // 0 NoInstrModel
  { 0, 0, 0, 0, 0 }, // 1 IIC_LdStSync
  { 1, 411, 413, 3192, 3195 }, // 2 IIC_IntSimple
  { 1, 413, 415, 3195, 3198 }, // 3 IIC_IntGeneral
  { 1, 415, 417, 3198, 3201 }, // 4 IIC_BrB
  { 1, 417, 419, 3201, 3204 }, // 5 IIC_VecFP
  { 1, 413, 415, 3204, 3207 }, // 6 IIC_IntCompare
  { 0, 0, 0, 0, 0 }, // 7 IIC_SprABORT
  { 0, 0, 0, 0, 0 }, // 8 IIC_LdStCOPY
  { 0, 0, 0, 0, 0 }, // 9 IIC_LdStPASTE
  { 1, 419, 421, 3207, 3210 }, // 10 IIC_BrCR
  { 1, 421, 423, 3210, 3213 }, // 11 IIC_LdStLD
  { 0, 0, 0, 0, 0 }, // 12 IIC_LdStDCBF
  { 1, 421, 423, 3213, 3216 }, // 13 IIC_LdStLoad
  { 1, 423, 426, 3216, 3219 }, // 14 IIC_IntDivD
  { 1, 426, 429, 3219, 3222 }, // 15 IIC_IntDivW
  { 0, 0, 0, 0, 0 }, // 16 IIC_FPDGeneral
  { 1, 429, 431, 3222, 3225 }, // 17 IIC_FPAddSub
  { 1, 429, 431, 3225, 3228 }, // 18 IIC_FPDivD
  { 0, 0, 0, 0, 0 }, // 19 IIC_FPSGeneral
  { 1, 429, 431, 3228, 3231 }, // 20 IIC_FPCompare
  { 1, 429, 431, 3231, 3234 }, // 21 IIC_FPGeneral
  { 1, 431, 433, 3234, 3237 }, // 22 IIC_VecGeneral
  { 1, 431, 433, 3237, 3240 }, // 23 IIC_VecComplex
  { 1, 433, 436, 3240, 3243 }, // 24 IIC_LdStStore
  { 1, 413, 415, 3243, 3246 }, // 25 IIC_IntRotateDI
  { 1, 429, 431, 3246, 3249 }, // 26 IIC_FPDivS
  { 1, 429, 431, 3249, 3253 }, // 27 IIC_FPFused
  { 1, 429, 431, 3253, 3256 }, // 28 IIC_FPSqrtD
  { 1, 429, 431, 3256, 3259 }, // 29 IIC_FPSqrtS
  { 0, 0, 0, 0, 0 }, // 30 IIC_LdStICBI
  { 1, 436, 439, 3259, 3263 }, // 31 IIC_IntISEL
  { 0, 0, 0, 0, 0 }, // 32 IIC_SprISYNC
  { 1, 439, 444, 3263, 3266 }, // 33 IIC_LdStLWARX
  { 1, 444, 448, 3266, 3270 }, // 34 IIC_LdStLoadUpd
  { 1, 448, 455, 3270, 3274 }, // 35 IIC_LdStLoadUpdX
  { 1, 439, 444, 3274, 3277 }, // 36 IIC_LdStLDARX
  { 1, 444, 448, 3277, 3281 }, // 37 IIC_LdStLDU
  { 1, 448, 455, 3281, 3285 }, // 38 IIC_LdStLDUX
  { 1, 421, 423, 3285, 3288 }, // 39 IIC_LdStLFD
  { 1, 444, 448, 3288, 3292 }, // 40 IIC_LdStLFDU
  { 1, 444, 448, 3292, 3296 }, // 41 IIC_LdStLFDUX
  { 1, 455, 459, 3296, 3299 }, // 42 IIC_LdStLHA
  { 1, 459, 464, 3299, 3303 }, // 43 IIC_LdStLHAU
  { 1, 464, 472, 3303, 3307 }, // 44 IIC_LdStLHAUX
  { 1, 421, 423, 3307, 3310 }, // 45 IIC_LdStLMW
  { 1, 455, 459, 3310, 3313 }, // 46 IIC_LdStLWA
  { 1, 413, 415, 3313, 3316 }, // 47 IIC_IntMulHD
  { 1, 415, 417, 3316, 3319 }, // 48 IIC_BrMCR
  { 1, 472, 478, 3319, 3321 }, // 49 IIC_BrMCRX
  { 1, 419, 421, 3321, 3323 }, // 50 IIC_SprMFCR
  { 0, 0, 0, 0, 0 }, // 51 IIC_SprMFSPR
  { 0, 0, 0, 0, 0 }, // 52 IIC_IntMFFS
  { 0, 0, 0, 0, 0 }, // 53 IIC_SprMFMSR
  { 1, 419, 421, 3323, 3325 }, // 54 IIC_SprMFCRF
  { 0, 0, 0, 0, 0 }, // 55 IIC_SprMFPMR
  { 0, 0, 0, 0, 0 }, // 56 IIC_SprMFSR
  { 0, 0, 0, 0, 0 }, // 57 IIC_SprMFTB
  { 0, 0, 0, 0, 0 }, // 58 IIC_SprMSGSYNC
  { 1, 478, 480, 3325, 3327 }, // 59 IIC_SprMTSPR
  { 0, 0, 0, 0, 0 }, // 60 IIC_IntMTFSB0
  { 0, 0, 0, 0, 0 }, // 61 IIC_SprMTMSR
  { 0, 0, 0, 0, 0 }, // 62 IIC_SprMTMSRD
  { 0, 0, 0, 0, 0 }, // 63 IIC_SprMTPMR
  { 0, 0, 0, 0, 0 }, // 64 IIC_SprMTSR
  { 1, 413, 415, 3327, 3330 }, // 65 IIC_IntMulHW
  { 1, 413, 415, 3330, 3333 }, // 66 IIC_IntMulHWU
  { 1, 413, 415, 3333, 3336 }, // 67 IIC_IntMulLI
  { 1, 480, 482, 3336, 3339 }, // 68 IIC_VecPerm
  { 1, 482, 485, 3339, 3342 }, // 69 IIC_LdStSTFD
  { 1, 485, 490, 3342, 3346 }, // 70 IIC_LdStSTFDU
  { 0, 0, 0, 0, 0 }, // 71 IIC_SprRFI
  { 0, 0, 0, 0, 0 }, // 72 IIC_IntRFID
  { 1, 413, 415, 3346, 3349 }, // 73 IIC_IntRotateD
  { 1, 413, 415, 3349, 3352 }, // 74 IIC_IntRotate
  { 0, 0, 0, 0, 0 }, // 75 IIC_SprSLBFEE
  { 0, 0, 0, 0, 0 }, // 76 IIC_SprSLBIA
  { 0, 0, 0, 0, 0 }, // 77 IIC_SprSLBIE
  { 0, 0, 0, 0, 0 }, // 78 IIC_SprSLBIEG
  { 0, 0, 0, 0, 0 }, // 79 IIC_SprSLBMFEE
  { 0, 0, 0, 0, 0 }, // 80 IIC_SprSLBMFEV
  { 0, 0, 0, 0, 0 }, // 81 IIC_SprSLBMTE
  { 0, 0, 0, 0, 0 }, // 82 IIC_SprSLBSYNC
  { 1, 413, 415, 3352, 3355 }, // 83 IIC_IntShift
  { 1, 439, 444, 3355, 3358 }, // 84 IIC_LdStSTWCX
  { 1, 459, 464, 3358, 3362 }, // 85 IIC_LdStSTU
  { 1, 490, 497, 3362, 3366 }, // 86 IIC_LdStSTUX
  { 1, 433, 436, 3366, 3369 }, // 87 IIC_LdStSTD
  { 1, 439, 444, 3369, 3372 }, // 88 IIC_LdStSTDCX
  { 0, 0, 0, 0, 0 }, // 89 IIC_SprSTOP
  { 1, 413, 415, 3372, 3374 }, // 90 IIC_IntTrapD
  { 0, 0, 0, 0, 0 }, // 91 IIC_SprTLBIA
  { 0, 0, 0, 0, 0 }, // 92 IIC_SprTLBIE
  { 0, 0, 0, 0, 0 }, // 93 IIC_SprTLBIEL
  { 0, 0, 0, 0, 0 }, // 94 IIC_SprTLBSYNC
  { 1, 413, 415, 3374, 3376 }, // 95 IIC_IntTrapW
  { 1, 417, 419, 3376, 3379 }, // 96 IIC_VecFPCompare
  { 1, 431, 433, 3379, 3382 }, // 97 VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz
  { 1, 417, 419, 3382, 3385 }, // 98 VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP
  { 1, 480, 482, 3385, 3388 }, // 99 XXSEL
  { 1, 478, 480, 3388, 3390 }, // 100 TABORTDC_TABORTDCI_TABORTWC_TABORTWCI
  { 0, 0, 0, 0, 0 }, // 101 MTFSB0_MTFSB1
  { 0, 0, 0, 0, 0 }, // 102 MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI
  { 1, 413, 415, 3390, 3393 }, // 103 CMPRB_CMPRB8_CMPEQB
  { 1, 413, 415, 3393, 3395 }, // 104 TD_TDI
  { 1, 413, 415, 3395, 3397 }, // 105 TW_TWI
  { 1, 429, 431, 3397, 3400 }, // 106 FCMPUD_FCMPUS_FTDIV_FTSQRT
  { 1, 417, 419, 3400, 3403 }, // 107 XSTSTDCDP_XSTSTDCSP
  { 1, 417, 419, 3403, 3406 }, // 108 XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN
  { 1, 429, 431, 3406, 3409 }, // 109 XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP
  { 1, 413, 415, 3409, 3412 }, // 110 CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD
  { 1, 413, 415, 3412, 3415 }, // 111 SLD_SRD_SRAD
  { 1, 413, 415, 3415, 3418 }, // 112 SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC
  { 1, 431, 433, 3418, 3421 }, // 113 MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ
  { 1, 413, 415, 3421, 3424 }, // 114 CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI
  { 1, 413, 415, 3424, 3427 }, // 115 SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA
  { 1, 411, 413, 3427, 3430 }, // 116 ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP
  { 0, 0, 0, 0, 0 }, // 117 ADDIStocHA_ADDIStocHA8_ADDItocL_COPY
  { 1, 415, 417, 3430, 3433 }, // 118 MCRF
  { 1, 472, 478, 3433, 3435 }, // 119 MCRXRX
  { 1, 417, 419, 3435, 3438 }, // 120 XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP
  { 1, 415, 417, 3438, 3441 }, // 121 RFEBB
  { 1, 478, 480, 3441, 3443 }, // 122 TBEGIN_TRECHKPT
  { 1, 421, 423, 3443, 3446 }, // 123 WAIT
  { 1, 413, 415, 3446, 3449 }, // 124 RLDCL_RLDCR
  { 1, 413, 415, 3449, 3452 }, // 125 RLWIMI_RLWIMI8
  { 1, 413, 415, 3452, 3455 }, // 126 RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI
  { 1, 419, 421, 3455, 3457 }, // 127 MFOCRF_MFOCRF8
  { 1, 472, 478, 3457, 3459 }, // 128 MTOCRF_MTOCRF8
  { 1, 419, 421, 3459, 3462 }, // 129 CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR
  { 1, 413, 415, 3462, 3465 }, // 130 SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8
  { 1, 429, 431, 3465, 3468 }, // 131 FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR
  { 1, 413, 415, 3468, 3471 }, // 132 SRAW_SRAWI
  { 1, 436, 439, 3471, 3475 }, // 133 ISEL_ISEL8
  { 1, 417, 419, 3475, 3478 }, // 134 XSIEXPDP
  { 1, 478, 480, 3478, 3480 }, // 135 TRECLAIM_TSR_TABORT
  { 1, 433, 436, 3480, 3483 }, // 136 MFVSCR
  { 1, 421, 423, 3483, 3486 }, // 137 MTVSCR
  { 1, 417, 419, 3486, 3489 }, // 138 VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec
  { 1, 431, 433, 3489, 3492 }, // 139 VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW
  { 1, 417, 419, 3492, 3495 }, // 140 VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP
  { 1, 429, 431, 3495, 3498 }, // 141 XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP
  { 1, 417, 419, 3498, 3501 }, // 142 VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS
  { 1, 431, 433, 3501, 3504 }, // 143 VSUBFP_VMULUWM
  { 1, 413, 415, 3504, 3507 }, // 144 MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO
  { 1, 413, 415, 3507, 3510 }, // 145 MULHD_MULHW_MULLW_MULLWO
  { 1, 413, 415, 3510, 3513 }, // 146 MULHDU_MULHWU
  { 1, 413, 415, 3513, 3516 }, // 147 MULLI_MULLI8
  { 1, 429, 431, 3516, 3519 }, // 148 FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS
  { 1, 429, 431, 3519, 3522 }, // 149 FADD_FSUB
  { 1, 429, 431, 3522, 3526 }, // 150 FMSUB_FMADD_FNMADD_FNMSUB_FMUL
  { 1, 417, 419, 3526, 3529 }, // 151 XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP
  { 1, 429, 431, 3529, 3532 }, // 152 FSELD_rec_FSELS_rec
  { 1, 413, 415, 3532, 3535 }, // 153 MULHDU_rec_MULHWU_rec
  { 1, 413, 415, 3535, 3538 }, // 154 MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec
  { 1, 413, 415, 3538, 3541 }, // 155 MULLDO_rec_MULLD_rec
  { 1, 429, 431, 3541, 3544 }, // 156 FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec
  { 1, 429, 431, 3544, 3547 }, // 157 FADD_rec_FSUB_rec
  { 1, 429, 431, 3547, 3551 }, // 158 FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec
  { 1, 417, 419, 3551, 3554 }, // 159 XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP
  { 1, 421, 423, 3554, 3557 }, // 160 LVSL_LVSR
  { 1, 480, 482, 3557, 3560 }, // 161 VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs
  { 1, 417, 419, 3560, 3563 }, // 162 V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec
  { 1, 431, 433, 3563, 3566 }, // 163 VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM
  { 1, 429, 431, 3566, 3569 }, // 164 XSCMPEXPQP_XSCMPOQP_XSCMPUQP
  { 1, 417, 419, 3569, 3572 }, // 165 BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO
  { 1, 417, 419, 3572, 3575 }, // 166 BCDCTSQ_rec
  { 1, 417, 419, 3575, 3578 }, // 167 XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO
  { 1, 417, 419, 3578, 3581 }, // 168 BCDCFSQ_rec
  { 1, 417, 419, 3581, 3584 }, // 169 XSDIVQP_XSDIVQPO
  { 1, 417, 419, 3584, 3587 }, // 170 XSSQRTQP_XSSQRTQPO
  { 1, 421, 423, 3587, 3590 }, // 171 LXVL_LXVLL
  { 1, 421, 423, 3590, 3593 }, // 172 LVEBX_LVEHX_LVEWX_LVX_LVXL
  { 1, 421, 423, 3593, 3596 }, // 173 LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD
  { 0, 0, 0, 0, 0 }, // 174 DFLOADf64_XFLOADf64_LIWZX
  { 0, 0, 0, 0, 0 }, // 175 DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP
  { 0, 0, 0, 0, 0 }, // 176 CP_COPY_CP_COPY8
  { 0, 0, 0, 0, 0 }, // 177 CP_PASTE_CP_PASTE8
  { 0, 0, 0, 0, 0 }, // 178 ICBI_ICBIEP
  { 1, 421, 423, 3596, 3599 }, // 179 ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI
  { 1, 439, 444, 3599, 3602 }, // 180 LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL
  { 1, 421, 423, 3602, 3605 }, // 181 LD_LDX_LDXTLS_LDXTLS__DARN
  { 1, 439, 444, 3605, 3608 }, // 182 LDARX_LDARXL
  { 0, 0, 0, 0, 0 }, // 183 CP_ABORT
  { 0, 0, 0, 0, 0 }, // 184 ISYNC
  { 0, 0, 0, 0, 0 }, // 185 MSGSYNC
  { 0, 0, 0, 0, 0 }, // 186 TLBSYNC
  { 0, 0, 0, 0, 0 }, // 187 SYNC
  { 1, 421, 423, 3608, 3611 }, // 188 LMW
  { 1, 421, 423, 3611, 3614 }, // 189 LFIWZX_LFDX_LFD
  { 0, 0, 0, 0, 0 }, // 190 SLBIA
  { 0, 0, 0, 0, 0 }, // 191 SLBIE
  { 0, 0, 0, 0, 0 }, // 192 SLBMFEE
  { 0, 0, 0, 0, 0 }, // 193 SLBMFEV
  { 0, 0, 0, 0, 0 }, // 194 SLBMTE
  { 0, 0, 0, 0, 0 }, // 195 TLBIEL
  { 1, 444, 448, 3614, 3618 }, // 196 LHZU_LHZU8_LWZU_LWZU8
  { 1, 448, 455, 3618, 3622 }, // 197 LHZUX_LHZUX8_LWZUX_LWZUX8
  { 1, 478, 480, 3622, 3624 }, // 198 TEND
  { 1, 439, 444, 3624, 3627 }, // 199 STBCX_STHCX_STWCX
  { 1, 439, 444, 3627, 3630 }, // 200 STDCX
  { 1, 421, 423, 3630, 3633 }, // 201 LDMX
  { 1, 455, 459, 3633, 3636 }, // 202 LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32
  { 0, 0, 0, 0, 0 }, // 203 CP_PASTE8_rec_CP_PASTE_rec
  { 1, 455, 459, 3636, 3639 }, // 204 LWA_LWA_32
  { 1, 478, 480, 3639, 3641 }, // 205 TCHECK
  { 1, 421, 423, 3641, 3644 }, // 206 LFIWAX
  { 1, 421, 423, 3644, 3647 }, // 207 LXSIWAX
  { 0, 0, 0, 0, 0 }, // 208 LIWAX
  { 1, 421, 423, 3647, 3650 }, // 209 LFSX_LFS
  { 1, 421, 423, 3650, 3653 }, // 210 LXSSP_LXSSPX
  { 0, 0, 0, 0, 0 }, // 211 XFLOADf32_DFLOADf32
  { 1, 459, 464, 3653, 3657 }, // 212 LHAU_LHAU8
  { 1, 464, 472, 3657, 3661 }, // 213 LHAUX_LHAUX8_LWAUX
  { 1, 421, 423, 3661, 3664 }, // 214 LXVH8X_LXVDSX_LXVW4X
  { 1, 482, 485, 3664, 3667 }, // 215 STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX
  { 1, 433, 436, 3667, 3670 }, // 216 STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32
  { 0, 0, 0, 0, 0 }, // 217 DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX
  { 1, 433, 436, 3670, 3673 }, // 218 STD_STDX_STDXTLS_STDXTLS_
  { 1, 421, 423, 3673, 3676 }, // 219 STBCIX_STDCIX_STHCIX_STWCIX_STSWI
  { 0, 0, 0, 0, 0 }, // 220 SLBIEG
  { 1, 421, 423, 3676, 3679 }, // 221 STMW
  { 0, 0, 0, 0, 0 }, // 222 TLBIE
  { 1, 433, 436, 3679, 3682 }, // 223 STVEBX_STVEHX_STVEWX_STVX_STVXL
  { 1, 482, 485, 3682, 3685 }, // 224 STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX
  { 1, 421, 423, 3685, 3688 }, // 225 STXVL_STXVLL
  { 1, 478, 480, 3688, 3690 }, // 226 MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8
  { 1, 413, 415, 3690, 3693 }, // 227 MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv
  { 0, 0, 0, 0, 0 }, // 228 MFPMR
  { 0, 0, 0, 0, 0 }, // 229 MTPMR
  { 0, 0, 0, 0, 0 }, // 230 MFTB_MFTB8
  { 0, 0, 0, 0, 0 }, // 231 MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8
  { 0, 0, 0, 0, 0 }, // 232 MFMSR
  { 0, 0, 0, 0, 0 }, // 233 MTMSR
  { 0, 0, 0, 0, 0 }, // 234 MTMSRD
  { 1, 478, 480, 3693, 3695 }, // 235 MTSPR_MTSPR8
  { 1, 426, 429, 3695, 3698 }, // 236 DIVW_DIVWO_DIVWU_DIVWUO_MODSW
  { 1, 426, 429, 3698, 3701 }, // 237 DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW
  { 1, 423, 426, 3701, 3704 }, // 238 DIVD_DIVDO_DIVDU_DIVDUO
  { 1, 423, 426, 3704, 3707 }, // 239 DIVDE_DIVDEO_DIVDEU_DIVDEUO
  { 1, 426, 429, 3707, 3710 }, // 240 DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec
  { 1, 423, 426, 3710, 3713 }, // 241 DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec
  { 1, 426, 429, 3713, 3716 }, // 242 DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec
  { 1, 423, 426, 3716, 3719 }, // 243 DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec
  { 1, 472, 478, 3719, 3721 }, // 244 MTCRF_MTCRF8
  { 1, 413, 415, 3721, 3724 }, // 245 ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec
  { 1, 429, 431, 3724, 3727 }, // 246 FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec
  { 1, 415, 417, 3727, 3730 }, // 247 MCRFS
  { 0, 0, 0, 0, 0 }, // 248 MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec
  { 0, 0, 0, 0, 0 }, // 249 MTFSFb
  { 1, 413, 415, 3730, 3733 }, // 250 RLDCL_rec_RLDCR_rec
  { 1, 413, 415, 3733, 3736 }, // 251 RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec
  { 1, 413, 415, 3736, 3739 }, // 252 RLWIMI8_rec_RLWIMI_rec
  { 1, 413, 415, 3739, 3742 }, // 253 RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec
  { 1, 413, 415, 3742, 3745 }, // 254 SRAWI_rec_SRAW_rec
  { 0, 0, 0, 0, 0 }, // 255 MFFS_MFFSCE_MFFSL_MFFS_rec
  { 1, 419, 421, 3745, 3747 }, // 256 MFCR_MFCR8
  { 1, 413, 415, 3747, 3750 }, // 257 EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec
  { 1, 413, 415, 3750, 3753 }, // 258 SRAD_rec_SLD_rec_SRD_rec
  { 1, 429, 431, 3753, 3756 }, // 259 FDIV
  { 1, 429, 431, 3756, 3759 }, // 260 FDIV_rec
  { 1, 429, 431, 3759, 3762 }, // 261 XSSQRTDP
  { 1, 429, 431, 3762, 3765 }, // 262 FSQRT
  { 1, 429, 431, 3765, 3768 }, // 263 XVSQRTDP
  { 1, 429, 431, 3768, 3771 }, // 264 XVSQRTSP
  { 1, 429, 431, 3771, 3774 }, // 265 FSQRT_rec
  { 1, 429, 431, 3774, 3777 }, // 266 XSSQRTSP
  { 1, 429, 431, 3777, 3780 }, // 267 FSQRTS
  { 1, 429, 431, 3780, 3783 }, // 268 FSQRTS_rec
  { 1, 429, 431, 3783, 3786 }, // 269 XSDIVDP
  { 1, 429, 431, 3786, 3789 }, // 270 FDIVS
  { 1, 429, 431, 3789, 3792 }, // 271 FDIVS_rec
  { 1, 429, 431, 3792, 3795 }, // 272 XSDIVSP
  { 1, 429, 431, 3795, 3798 }, // 273 XVDIVSP
  { 1, 429, 431, 3798, 3801 }, // 274 XVDIVDP
  { 1, 444, 448, 3801, 3805 }, // 275 LFSU
  { 1, 444, 448, 3805, 3809 }, // 276 LFSUX
  { 1, 485, 490, 3809, 3813 }, // 277 STFDU_STFDUX_STFSU_STFSUX
  { 1, 459, 464, 3813, 3817 }, // 278 STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8
  { 1, 490, 497, 3817, 3821 }, // 279 STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8
  { 1, 444, 448, 3821, 3825 }, // 280 LBZU_LBZU8
  { 1, 448, 455, 3825, 3829 }, // 281 LBZUX_LBZUX8
  { 1, 444, 448, 3829, 3833 }, // 282 LDU
  { 1, 448, 455, 3833, 3837 }, // 283 LDUX
  { 1, 444, 448, 3837, 3841 }, // 284 LFDU
  { 1, 444, 448, 3841, 3845 }, // 285 LFDUX
  { 1, 417, 419, 3845, 3848 }, // 286 VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX
  { 1, 415, 417, 3848, 3851 }, // 287 BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP
  { 1, 415, 417, 3851, 3854 }, // 288 ADDPCIS
  { 1, 421, 423, 3854, 3857 }, // 289 LDAT_LWAT
  { 1, 433, 436, 3857, 3860 }, // 290 STDAT_STWAT
  { 1, 411, 413, 3860, 3863 }, // 291 BRINC
  { 1, 431, 433, 3863, 3866 }, // 292 EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW
  { 1, 431, 433, 3866, 3869 }, // 293 EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW
  { 1, 421, 423, 3869, 3872 }, // 294 EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX
  { 1, 433, 436, 3872, 3875 }, // 295 EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX
  { 1, 415, 417, 3875, 3878 }, // 296 HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC
  { 0, 0, 0, 0, 0 }, // 297 RFI
  { 0, 0, 0, 0, 0 }, // 298 RFID
  { 1, 421, 423, 3878, 3881 }, // 299 DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI
  { 1, 433, 436, 3881, 3884 }, // 300 ICBLC_STBEPX_STHEPX_STWEPX
  { 1, 421, 423, 3884, 3887 }, // 301 LFDEPX
  { 1, 482, 485, 3887, 3890 }, // 302 STFDEPX
  { 0, 0, 0, 0, 0 }, // 303 MFSR_MFSRIN
  { 0, 0, 0, 0, 0 }, // 304 MTSR_MTSRIN
  { 0, 0, 0, 0, 0 }, // 305 MFDCR
  { 1, 478, 480, 3890, 3892 }, // 306 MTDCR
  { 1, 411, 413, 3892, 3895 }, // 307 NOP_GT_PWR6_NOP_GT_PWR7
  { 0, 0, 0, 0, 0 }, // 308 TLBIA
  { 0, 0, 0, 0, 0 }, // 309 WRTEE_WRTEEI
  { 0, 0, 0, 0, 0 }, // 310 MSYNC
  { 0, 0, 0, 0, 0 }, // 311 SLBSYNC
  { 0, 0, 0, 0, 0 }, // 312 SLBFEE_rec
  { 0, 0, 0, 0, 0 }, // 313 STOP
  { 0, 0, 0, 0, 0 }, // 314 DCBA_DCBI
  { 0, uint16_t(~0U), uint16_t(~0U), uint16_t(~0U), uint16_t(~0U) }// end marker
};

// ===============================================================
// Data tables for the new per-operand machine model.

// {ProcResourceIdx, Cycles}
extern const llvm::MCWriteProcResEntry PPCWriteProcResTable[] = {
  { 0,  0}, // Invalid
  { 1,  2}, // #1
  { 2,  1}, // #2
  { 3,  1}, // #3
  { 7,  1}, // #4
  {18,  2}, // #5
  {19,  1}, // #6
  {20,  1}, // #7
  { 1,  1}, // #8
  { 7,  3}, // #9
  { 8,  2}, // #10
  {10,  1}, // #11
  {11,  1}, // #12
  {12,  1}, // #13
  {18,  1}, // #14
  { 1,  1}, // #15
  { 7,  1}, // #16
  {18,  1}, // #17
  { 7,  1}, // #18
  {14,  2}, // #19
  {15,  1}, // #20
  {16,  1}, // #21
  {18,  2}, // #22
  {19,  1}, // #23
  {20,  1}, // #24
  { 7,  3}, // #25
  { 8,  2}, // #26
  {10,  1}, // #27
  {11,  1}, // #28
  {12,  1}, // #29
  {14,  1}, // #30
  {18,  1}, // #31
  { 1,  1}, // #32
  { 7,  4}, // #33
  { 8,  2}, // #34
  {10,  1}, // #35
  {11,  1}, // #36
  {12,  1}, // #37
  {14,  1}, // #38
  {18,  2}, // #39
  { 7,  1}, // #40
  {14,  1}, // #41
  {18,  1}, // #42
  { 7,  1}, // #43
  {18,  2}, // #44
  {19,  1}, // #45
  {20,  1}, // #46
  {22,  1}, // #47
  { 6,  1}, // #48
  { 7,  1}, // #49
  {18,  2}, // #50
  {19,  1}, // #51
  {20,  1}, // #52
  { 6, 11}, // #53
  { 7,  1}, // #54
  {18,  2}, // #55
  {19,  1}, // #56
  {20,  1}, // #57
  { 6, 12}, // #58
  { 7,  1}, // #59
  {18,  2}, // #60
  {19,  1}, // #61
  {20,  1}, // #62
  { 6, 25}, // #63
  { 7,  1}, // #64
  {18,  2}, // #65
  {19,  1}, // #66
  {20,  1}, // #67
  { 6, 44}, // #68
  { 7,  1}, // #69
  {18,  2}, // #70
  {19,  1}, // #71
  {20,  1}, // #72
  { 6, 62}, // #73
  { 7,  1}, // #74
  {18,  2}, // #75
  {19,  1}, // #76
  {20,  1}, // #77
  { 7,  1}, // #78
  {17,  1}, // #79
  {21,  1}, // #80
  { 7,  3}, // #81
  { 8,  2}, // #82
  {10,  1}, // #83
  {11,  1}, // #84
  {12,  1}, // #85
  {17,  1}, // #86
  {21,  1}, // #87
  { 7,  2}, // #88
  { 8,  2}, // #89
  {17,  2}, // #90
  {21,  2}, // #91
  { 1,  1}, // #92
  { 7,  4}, // #93
  { 8,  4}, // #94
  {17,  1}, // #95
  {18,  1}, // #96
  {21,  1}, // #97
  { 1,  1}, // #98
  { 7,  2}, // #99
  {17,  1}, // #100
  {18,  1}, // #101
  {21,  1}, // #102
  { 1,  1}, // #103
  { 7,  4}, // #104
  { 8,  2}, // #105
  {10,  1}, // #106
  {11,  1}, // #107
  {12,  1}, // #108
  {17,  1}, // #109
  {18,  2}, // #110
  {21,  1}, // #111
  { 7,  2}, // #112
  {17,  2}, // #113
  {21,  2}, // #114
  { 1,  1}, // #115
  { 7,  6}, // #116
  { 8,  4}, // #117
  {10,  2}, // #118
  {11,  2}, // #119
  {12,  2}, // #120
  {17,  1}, // #121
  {18,  1}, // #122
  {21,  1}, // #123
  { 1,  2}, // #124
  { 7,  5}, // #125
  { 8,  4}, // #126
  {17,  1}, // #127
  {18,  2}, // #128
  {21,  1}, // #129
  { 7,  2}, // #130
  {17,  1}, // #131
  {18,  2}, // #132
  {19,  1}, // #133
  {20,  1}, // #134
  {21,  1}, // #135
  {22,  1}, // #136
  { 7,  3}, // #137
  { 8,  2}, // #138
  {10,  1}, // #139
  {11,  1}, // #140
  {12,  1}, // #141
  {17,  1}, // #142
  {18,  1}, // #143
  {21,  1}, // #144
  { 7,  1}, // #145
  {17,  1}, // #146
  {18,  2}, // #147
  {19,  1}, // #148
  {20,  1}, // #149
  {21,  1}, // #150
  { 7,  2}, // #151
  { 8,  2}, // #152
  {10,  1}, // #153
  {11,  1}, // #154
  {13,  1}, // #155
  {18,  2}, // #156
  {19,  1}, // #157
  {20,  1}, // #158
  { 7,  2}, // #159
  { 8,  2}, // #160
  {10,  1}, // #161
  {11,  1}, // #162
  {13,  8}, // #163
  {18,  2}, // #164
  {19,  1}, // #165
  {20,  1}, // #166
  { 1,  1}, // #167
  { 7,  3}, // #168
  { 8,  2}, // #169
  {10,  1}, // #170
  {11,  1}, // #171
  {13,  8}, // #172
  {18,  3}, // #173
  {19,  1}, // #174
  {20,  1}, // #175
  { 1,  2}, // #176
  { 7,  6}, // #177
  { 8,  4}, // #178
  {10,  2}, // #179
  {11,  2}, // #180
  {12,  2}, // #181
  {18,  2}, // #182
  { 1,  2}, // #183
  { 7,  2}, // #184
  {18,  2}, // #185
  { 1,  2}, // #186
  { 7,  4}, // #187
  { 8,  2}, // #188
  {10,  1}, // #189
  {11,  1}, // #190
  {12,  1}, // #191
  {18,  2}, // #192
  { 1,  3}, // #193
  { 7,  9}, // #194
  { 8,  6}, // #195
  {10,  3}, // #196
  {11,  3}, // #197
  {12,  3}, // #198
  {18,  3}, // #199
  { 7,  3}, // #200
  { 8,  2}, // #201
  {10,  1}, // #202
  {11,  1}, // #203
  {12,  1}, // #204
  {14,  8}, // #205
  {18,  1}, // #206
  { 1,  1}, // #207
  { 7,  4}, // #208
  { 8,  2}, // #209
  {10,  1}, // #210
  {11,  1}, // #211
  {12,  1}, // #212
  {14,  8}, // #213
  {18,  2}, // #214
  { 7,  1}, // #215
  {14, 10}, // #216
  {18,  1}, // #217
  { 7,  3}, // #218
  { 8,  2}, // #219
  {10,  1}, // #220
  {11,  1}, // #221
  {12,  1}, // #222
  {14, 10}, // #223
  {18,  1}, // #224
  { 7,  1}, // #225
  {14, 20}, // #226
  {18,  2}, // #227
  {19,  1}, // #228
  {20,  1}, // #229
  { 1,  1}, // #230
  { 7,  4}, // #231
  { 8,  2}, // #232
  {10,  1}, // #233
  {11,  1}, // #234
  {12,  1}, // #235
  {14, 10}, // #236
  {18,  2}, // #237
  { 7,  1}, // #238
  {14,  5}, // #239
  {18,  1}, // #240
  { 7,  3}, // #241
  { 8,  2}, // #242
  {10,  1}, // #243
  {11,  1}, // #244
  {12,  1}, // #245
  {14,  5}, // #246
  {18,  1}, // #247
  { 1,  1}, // #248
  { 7,  4}, // #249
  { 8,  2}, // #250
  {10,  1}, // #251
  {11,  1}, // #252
  {12,  1}, // #253
  {14,  5}, // #254
  {18,  2}, // #255
  { 7,  1}, // #256
  {14,  8}, // #257
  {18,  1}, // #258
  { 7,  1}, // #259
  {14, 16}, // #260
  {15,  8}, // #261
  {16,  8}, // #262
  {18,  2}, // #263
  {19,  1}, // #264
  {20,  1}, // #265
  { 1,  2}, // #266
  { 7,  7}, // #267
  { 8,  4}, // #268
  {10,  2}, // #269
  {11,  2}, // #270
  {12,  2}, // #271
  {17,  1}, // #272
  {18,  2}, // #273
  {21,  1}, // #274
  { 1,  1}, // #275
  { 7,  4}, // #276
  { 8,  2}, // #277
  {10,  1}, // #278
  {11,  1}, // #279
  {12,  1}, // #280
  {17,  1}, // #281
  {18,  1}, // #282
  {21,  1}, // #283
  { 5,  1}, // #284
  { 7,  1}, // #285
  {18,  2}, // #286
  {19,  1}, // #287
  {20,  1}, // #288
  { 4,  1}, // #289
  { 7,  1}, // #290
  {12,  1}, // #291
  { 1,  1}, // #292
  { 4,  1}, // #293
  { 7,  2}, // #294
  {12,  1}, // #295
  {18,  1}, // #296
  { 7, 10}, // #297
  { 8,  4}, // #298
  {10,  2}, // #299
  {11,  2}, // #300
  {12,  2}, // #301
  {17,  5}, // #302
  {18,  2}, // #303
  {21,  5}, // #304
  { 7,  5}, // #305
  { 8,  2}, // #306
  {10,  1}, // #307
  {11,  1}, // #308
  {12,  1}, // #309
  {17,  3}, // #310
  {18,  1}, // #311
  {21,  3} // #312
}; // PPCWriteProcResTable

// {Cycles, WriteResourceID}
extern const llvm::MCWriteLatencyEntry PPCWriteLatencyTable[] = {
  { 0,  0}, // Invalid
  { 2,  0}, // #1 P9_ALUE_2C
  { 2,  0}, // #2 P9_ALUO_2C_P9_ALU_2C_P9_BR_2C
  { 1,  0}, // #3 IP_EXECE_1C_IP_EXEC_1C_DISP_BR_1C
  { 1,  0}, // #4 IP_EXECO_1C_DISP_1C_DISP_3SLOTS_1C
  { 1,  0}, // #5 DISP_1C
  { 3,  0}, // #6 P9_ALU_3C
  { 1,  0}, // #7 IP_EXEC_1C
  { 1,  0}, // #8 DISP_3SLOTS_1C_DISP_1C
  { 3,  0}, // #9 P9_ALUE_3C
  { 3,  0}, // #10 P9_ALUO_3C_P9_PM_3C
  { 1,  0}, // #11 IP_EXECE_1C_IP_EXECO_1C
  { 1,  0}, // #12 IP_EXECO_1C_IP_EXECE_1C
  { 1,  0}, // #13 DISP_1C
  { 7,  0}, // #14 P9_DPE_7C
  { 7,  0}, // #15 P9_DPO_7C_P9_DP_7C_P9_ALU_2C
  { 1,  0}, // #16 IP_EXECE_1C_IP_EXEC_1C
  { 1,  0}, // #17 IP_EXECO_1C_DISP_3SLOTS_1C_DISP_1C_DISP_BR_1C
  { 1,  0}, // #18 DISP_1C
  { 5,  0}, // #19 P9_DP_5C_P9_LS_5C
  { 1,  0}, // #20 IP_EXEC_1C_IP_AGEN_1C
  { 1,  0}, // #21 DISP_3SLOTS_1C_DISP_1C
  { 7,  0}, // #22 P9_DP_7C
  { 3,  0}, // #23 P9_ALU_3C
  { 1,  0}, // #24 IP_EXEC_1C_P9_LS_1C
  { 1,  0}, // #25 IP_EXEC_1C
  { 1,  0}, // #26 DISP_3SLOTS_1C_IP_AGEN_1C
  { 1,  0}, // #27 DISP_1C_DISP_3SLOTS_1C
  { 7,  0}, // #28 P9_ALU_2C_P9_ALU_3C
  { 1,  0}, // #29 IP_EXEC_1C_IP_AGEN_1C
  { 1,  0}, // #30 IP_EXEC_1C
  { 1,  0}, // #31 DISP_3SLOTS_1C_DISP_1C
  { 1,  0}, // #32 DISP_1C_DISP_3SLOTS_1C
  {10,  0}, // #33 P9_ALU_3C
  { 1,  0}, // #34 IP_EXEC_1C
  { 1,  0}, // #35 IP_EXEC_1C
  { 1,  0}, // #36 DISP_3SLOTS_1C
  { 1,  0}, // #37 DISP_1C
  {12,  0}, // #38 P9_DFU_12C_P9_DIV_12C
  { 1,  0}, // #39 IP_EXECE_1C
  { 1,  0}, // #40 IP_EXECO_1C
  { 1,  0}, // #41 DISP_1C_DISP_EVEN_1C
  {23,  0}, // #42 P9_DFU_23C
  { 1,  0}, // #43 IP_EXECE_1C
  { 1,  0}, // #44 IP_EXECO_1C
  { 1,  0}, // #45 DISP_1C
  {24,  0}, // #46 P9_DFU_24C_P9_DIV_24C_8
  { 1,  0}, // #47 IP_EXECE_1C_IP_EXECO_1C
  { 1,  0}, // #48 IP_EXECO_1C_IP_EXECE_1C
  { 1,  0}, // #49 DISP_1C_DISP_EVEN_1C
  {37,  0}, // #50 P9_DFU_37C
  { 1,  0}, // #51 IP_EXECE_1C
  { 1,  0}, // #52 IP_EXECO_1C
  { 1,  0}, // #53 DISP_1C
  {58,  0}, // #54 P9_DFU_58C
  { 1,  0}, // #55 IP_EXECE_1C
  { 1,  0}, // #56 IP_EXECO_1C
  { 1,  0}, // #57 DISP_1C
  {76,  0}, // #58 P9_DFU_76C
  { 1,  0}, // #59 IP_EXECE_1C
  { 1,  0}, // #60 IP_EXECO_1C
  { 1,  0}, // #61 DISP_1C
  { 6,  0}, // #62 P9_LS_6C
  { 1,  0}, // #63 IP_AGEN_1C
  { 1,  0}, // #64 DISP_1C
  { 4,  0}, // #65 P9_LS_4C
  { 1,  0}, // #66 IP_AGEN_1C
  { 1,  0}, // #67 DISP_1C_DISP_3SLOTS_1C
  { 4,  0}, // #68 P9_LS_4C
  { 4,  0}, // #69 P9_LS_4C
  { 1,  0}, // #70 IP_AGEN_1C
  { 1,  0}, // #71 IP_AGEN_1C
  { 1,  0}, // #72 DISP_PAIR_1C
  { 4,  0}, // #73 P9_LS_4C
  { 2,  0}, // #74 P9_ALU_2C
  { 1,  0}, // #75 IP_EXEC_1C_IP_AGEN_1C
  { 1,  0}, // #76 IP_AGEN_1C_IP_EXEC_1C
  { 1,  0}, // #77 DISP_PAIR_1C_DISP_1C_DISP_3SLOTS_1C
  { 1,  0}, // #78 DISP_PAIR_1C_DISP_1C
  { 3,  0}, // #79 P9_ALU_2C
  { 1,  0}, // #80 IP_EXEC_1C_P9_LS_1C
  { 1,  0}, // #81 IP_EXEC_1C_IP_EXECE_1C
  { 1,  0}, // #82 IP_AGEN_1C_IP_EXECO_1C
  { 1,  0}, // #83 DISP_3SLOTS_1C_IP_AGEN_1C
  { 1,  0}, // #84 DISP_1C
  { 8,  0}, // #85 P9_LS_4C_P9_ALU_3C
  { 1,  0}, // #86 IP_AGEN_1C
  { 1,  0}, // #87 IP_AGEN_1C_IP_EXEC_1C
  { 1,  0}, // #88 DISP_1C
  { 1,  0}, // #89 DISP_1C
  { 6,  0}, // #90 P9_ALU_2C_P9_ALU_3C_P9_CY_6C
  { 1,  0}, // #91 IP_EXEC_1C_IP_EXECO_1C
  { 1,  0}, // #92 IP_AGEN_1C_IP_EXEC_1C_IP_EXECE_1C
  { 1,  0}, // #93 DISP_1C_DISP_3SLOTS_1C
  { 1,  0}, // #94 DISP_1C_DISP_3SLOTS_1C
  { 6,  0}, // #95 P9_ALU_2C
  { 2,  0}, // #96 P9_ALU_2C
  { 1,  0}, // #97 IP_AGEN_1C
  { 1,  0}, // #98 IP_EXEC_1C
  { 1,  0}, // #99 IP_EXEC_1C
  { 1,  0}, // #100 DISP_PAIR_1C
  { 1,  0}, // #101 DISP_PAIR_1C
  { 1,  0}, // #102 DISP_1C
  { 8,  0}, // #103 P9_PM_3C
  { 1,  0}, // #104 IP_AGEN_1C
  { 1,  0}, // #105 IP_EXECE_1C
  { 1,  0}, // #106 IP_EXECO_1C
  { 1,  0}, // #107 DISP_1C
  { 1,  0}, // #108 DISP_1C
  { 5,  0}, // #109 P9_DIV_5C
  { 1,  0}, // #110 IP_EXECE_1C
  { 1,  0}, // #111 IP_EXECO_1C
  { 1,  0}, // #112 DISP_EVEN_1C
  {16,  0}, // #113 P9_DIV_16C_8
  { 1,  0}, // #114 IP_EXECO_1C
  { 1,  0}, // #115 IP_EXECE_1C
  { 1,  0}, // #116 DISP_EVEN_1C
  {40,  0}, // #117 P9_DIV_40C_8
  { 1,  0}, // #118 IP_EXECO_1C
  { 1,  0}, // #119 IP_EXECE_1C
  { 1,  0}, // #120 DISP_EVEN_1C
  {18,  0}, // #121 P9_ALU_2C
  { 1,  0}, // #122 IP_EXECE_1C
  { 1,  0}, // #123 IP_EXECO_1C
  { 1,  0}, // #124 IP_EXEC_1C
  { 1,  0}, // #125 DISP_EVEN_1C
  { 1,  0}, // #126 DISP_1C
  {26,  0}, // #127 P9_ALU_2C
  { 1,  0}, // #128 IP_EXECE_1C
  { 1,  0}, // #129 IP_EXECO_1C
  { 1,  0}, // #130 IP_EXEC_1C
  { 1,  0}, // #131 DISP_EVEN_1C
  { 1,  0}, // #132 DISP_1C
  {42,  0}, // #133 P9_ALU_2C
  { 1,  0}, // #134 IP_EXECE_1C
  { 1,  0}, // #135 IP_EXECO_1C
  { 1,  0}, // #136 IP_EXEC_1C
  { 1,  0}, // #137 DISP_EVEN_1C
  { 1,  0}, // #138 DISP_1C
  { 2,  0}, // #139 P9_ALU_2C
  { 2,  0}, // #140 P9_ALU_2C
  { 1,  0}, // #141 IP_EXEC_1C
  { 1,  0}, // #142 IP_EXEC_1C
  { 1,  0}, // #143 DISP_3SLOTS_1C_DISP_1C
  { 1,  0}, // #144 DISP_3SLOTS_1C_DISP_1C
  { 2,  0}, // #145 P9_ALU_2C
  { 3,  0}, // #146 P9_ALU_3C
  { 1,  0}, // #147 IP_EXEC_1C
  { 1,  0}, // #148 IP_EXEC_1C
  { 1,  0}, // #149 DISP_3SLOTS_1C
  { 1,  0}, // #150 DISP_1C
  { 3,  0}, // #151 P9_ALU_3C
  { 3,  0}, // #152 P9_ALU_3C
  { 1,  0}, // #153 IP_EXEC_1C
  { 1,  0}, // #154 IP_EXEC_1C
  { 1,  0}, // #155 DISP_1C_DISP_3SLOTS_1C
  { 1,  0}, // #156 DISP_1C_DISP_3SLOTS_1C
  { 4,  0}, // #157 P9_ALU_2C
  { 1,  0}, // #158 IP_EXEC_1C
  { 1,  0}, // #159 IP_EXEC_1C
  { 1,  0}, // #160 DISP_3SLOTS_1C_DISP_1C
  { 1,  0}, // #161 DISP_1C
  { 6,  0}, // #162 P9_ALU_2C
  { 1,  0}, // #163 IP_EXEC_1C
  { 1,  0}, // #164 IP_EXEC_1C
  { 1,  0}, // #165 IP_EXEC_1C
  { 1,  0}, // #166 DISP_3SLOTS_1C
  { 1,  0}, // #167 DISP_3SLOTS_1C
  { 1,  0}, // #168 DISP_3SLOTS_1C
  {33,  0}, // #169 P9_DP_33C_8
  { 1,  0}, // #170 IP_EXEC_1C
  { 1,  0}, // #171 DISP_3SLOTS_1C_DISP_1C
  {36,  0}, // #172 P9_ALU_3C_P9_DP_36C_10
  { 1,  0}, // #173 IP_EXEC_1C
  { 1,  0}, // #174 IP_EXEC_1C_DISP_1C_DISP_3SLOTS_1C
  { 1,  0}, // #175 DISP_3SLOTS_1C
  { 1,  0}, // #176 DISP_1C
  {36,  0}, // #177 P9_DPE_36C_10
  {36,  0}, // #178 P9_DPO_36C_10
  { 1,  0}, // #179 IP_EXECE_1C
  { 1,  0}, // #180 IP_EXECO_1C
  { 1,  0}, // #181 DISP_1C
  {27,  0}, // #182 P9_DPE_27C_10
  {27,  0}, // #183 P9_DPO_27C_10
  { 1,  0}, // #184 IP_EXECE_1C
  { 1,  0}, // #185 IP_EXECO_1C
  { 1,  0}, // #186 DISP_1C
  {39,  0}, // #187 P9_ALU_3C
  { 1,  0}, // #188 IP_EXEC_1C
  { 1,  0}, // #189 IP_EXEC_1C
  { 1,  0}, // #190 DISP_3SLOTS_1C
  { 1,  0}, // #191 DISP_1C
  {22,  0}, // #192 P9_DP_26C_5_P9_DP_22C_5
  { 1,  0}, // #193 IP_EXEC_1C
  { 1,  0}, // #194 DISP_1C_DISP_3SLOTS_1C
  {25,  0}, // #195 P9_ALU_3C
  { 1,  0}, // #196 IP_EXEC_1C
  { 1,  0}, // #197 IP_EXEC_1C
  { 1,  0}, // #198 DISP_3SLOTS_1C
  { 1,  0}, // #199 DISP_1C
  {24,  0}, // #200 P9_DPE_24C_8
  {24,  0}, // #201 P9_DPO_24C_8
  { 1,  0}, // #202 IP_EXECE_1C
  { 1,  0}, // #203 IP_EXECO_1C
  { 1,  0}, // #204 DISP_1C
  {33,  0}, // #205 P9_DPE_33C_8
  {33,  0}, // #206 P9_DPO_33C_8
  { 1,  0}, // #207 IP_EXECE_1C
  { 1,  0}, // #208 IP_EXECO_1C
  { 1,  0}, // #209 DISP_1C
  { 7,  0}, // #210 P9_ALU_3C
  { 2,  0}, // #211 P9_ALU_2C
  { 1,  0}, // #212 IP_AGEN_1C
  { 1,  0}, // #213 IP_EXEC_1C
  { 1,  0}, // #214 IP_EXEC_1C
  { 1,  0}, // #215 DISP_3SLOTS_1C
  { 1,  0}, // #216 DISP_3SLOTS_1C
  { 1,  0}, // #217 DISP_1C
  { 1,  0}, // #218 P9_LS_1C
  { 2,  0}, // #219 P9_ALU_2C
  { 1,  0}, // #220 IP_AGEN_1C
  { 1,  0}, // #221 IP_EXEC_1C
  { 1,  0}, // #222 IP_EXEC_1C
  { 1,  0}, // #223 DISP_3SLOTS_1C
  { 1,  0}, // #224 DISP_1C
  { 1,  0}, // #225 P9_LS_1C
  { 1,  0}, // #226 P9_LS_1C
  { 4,  0}, // #227 P9_LS_4C
  { 4,  0}, // #228 P9_LS_4C
  { 4,  0}, // #229 P9_LS_4C
  { 1,  0}, // #230 IP_EXEC_1C
  { 1,  0}, // #231 IP_EXEC_1C
  { 1,  0}, // #232 IP_AGEN_1C
  { 1,  0}, // #233 IP_AGEN_1C
  { 1,  0}, // #234 IP_AGEN_1C
  { 1,  0}, // #235 IP_AGEN_1C
  { 1,  0}, // #236 IP_AGEN_1C
  { 1,  0}, // #237 DISP_1C
  { 1,  0}, // #238 DISP_3SLOTS_1C
  { 1,  0}, // #239 DISP_3SLOTS_1C
  { 1,  0}, // #240 DISP_1C
  { 1,  0}, // #241 DISP_1C
  { 1,  0}, // #242 DISP_1C
  { 1,  0}, // #243 P9_LS_1C
  { 4,  0}, // #244 P9_LS_4C
  { 4,  0}, // #245 P9_LS_4C
  { 1,  0}, // #246 IP_EXEC_1C
  { 1,  0}, // #247 IP_AGEN_1C
  { 1,  0}, // #248 IP_AGEN_1C
  { 1,  0}, // #249 IP_AGEN_1C
  { 1,  0}, // #250 DISP_1C
  { 1,  0}, // #251 DISP_3SLOTS_1C
  { 1,  0} // #252 DISP_1C
}; // PPCWriteLatencyTable

// {UseIdx, WriteResourceID, Cycles}
extern const llvm::MCReadAdvanceEntry PPCReadAdvanceTable[] = {
  {0,  0,  0}, // Invalid
}; // PPCReadAdvanceTable

// {Name, NumMicroOps, BeginGroup, EndGroup, WriteProcResIdx,#, WriteLatencyIdx,#, ReadAdvanceIdx,#}
static const llvm::MCSchedClassDesc P9ModelSchedClasses[] = {
  {DBGFIELD("InvalidSchedClass")  16383, false, false,  0, 0,  0, 0,  0, 0},
  {DBGFIELD("IIC_LdStSync")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #1
  {DBGFIELD("IIC_IntSimple")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #2
  {DBGFIELD("IIC_IntGeneral")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #3
  {DBGFIELD("IIC_BrB")            16383, false, false,  0, 0,  0, 0,  0, 0}, // #4
  {DBGFIELD("IIC_VecFP")          16383, false, false,  0, 0,  0, 0,  0, 0}, // #5
  {DBGFIELD("IIC_IntCompare")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #6
  {DBGFIELD("IIC_SprABORT")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #7
  {DBGFIELD("IIC_LdStCOPY")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #8
  {DBGFIELD("IIC_LdStPASTE")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #9
  {DBGFIELD("IIC_BrCR")           16383, false, false,  0, 0,  0, 0,  0, 0}, // #10
  {DBGFIELD("IIC_LdStLD")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #11
  {DBGFIELD("IIC_LdStDCBF")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #12
  {DBGFIELD("IIC_LdStLoad")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #13
  {DBGFIELD("IIC_IntDivD")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #14
  {DBGFIELD("IIC_IntDivW")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #15
  {DBGFIELD("IIC_FPDGeneral")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #16
  {DBGFIELD("IIC_FPAddSub")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #17
  {DBGFIELD("IIC_FPDivD")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #18
  {DBGFIELD("IIC_FPSGeneral")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #19
  {DBGFIELD("IIC_FPCompare")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #20
  {DBGFIELD("IIC_FPGeneral")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #21
  {DBGFIELD("IIC_VecGeneral")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #22
  {DBGFIELD("IIC_VecComplex")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #23
  {DBGFIELD("IIC_LdStStore")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #24
  {DBGFIELD("IIC_IntRotateDI")    16383, false, false,  0, 0,  0, 0,  0, 0}, // #25
  {DBGFIELD("IIC_FPDivS")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #26
  {DBGFIELD("IIC_FPFused")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #27
  {DBGFIELD("IIC_FPSqrtD")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #28
  {DBGFIELD("IIC_FPSqrtS")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #29
  {DBGFIELD("IIC_LdStICBI")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #30
  {DBGFIELD("IIC_IntISEL")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #31
  {DBGFIELD("IIC_SprISYNC")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #32
  {DBGFIELD("IIC_LdStLWARX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #33
  {DBGFIELD("IIC_LdStLoadUpd")    16383, false, false,  0, 0,  0, 0,  0, 0}, // #34
  {DBGFIELD("IIC_LdStLoadUpdX")   16383, false, false,  0, 0,  0, 0,  0, 0}, // #35
  {DBGFIELD("IIC_LdStLDARX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #36
  {DBGFIELD("IIC_LdStLDU")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #37
  {DBGFIELD("IIC_LdStLDUX")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #38
  {DBGFIELD("IIC_LdStLFD")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #39
  {DBGFIELD("IIC_LdStLFDU")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #40
  {DBGFIELD("IIC_LdStLFDUX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #41
  {DBGFIELD("IIC_LdStLHA")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #42
  {DBGFIELD("IIC_LdStLHAU")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #43
  {DBGFIELD("IIC_LdStLHAUX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #44
  {DBGFIELD("IIC_LdStLMW")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #45
  {DBGFIELD("IIC_LdStLWA")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #46
  {DBGFIELD("IIC_IntMulHD")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #47
  {DBGFIELD("IIC_BrMCR")          16383, false, false,  0, 0,  0, 0,  0, 0}, // #48
  {DBGFIELD("IIC_BrMCRX")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #49
  {DBGFIELD("IIC_SprMFCR")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #50
  {DBGFIELD("IIC_SprMFSPR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #51
  {DBGFIELD("IIC_IntMFFS")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #52
  {DBGFIELD("IIC_SprMFMSR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #53
  {DBGFIELD("IIC_SprMFCRF")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #54
  {DBGFIELD("IIC_SprMFPMR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #55
  {DBGFIELD("IIC_SprMFSR")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #56
  {DBGFIELD("IIC_SprMFTB")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #57
  {DBGFIELD("IIC_SprMSGSYNC")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #58
  {DBGFIELD("IIC_SprMTSPR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #59
  {DBGFIELD("IIC_IntMTFSB0")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #60
  {DBGFIELD("IIC_SprMTMSR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #61
  {DBGFIELD("IIC_SprMTMSRD")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #62
  {DBGFIELD("IIC_SprMTPMR")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #63
  {DBGFIELD("IIC_SprMTSR")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #64
  {DBGFIELD("IIC_IntMulHW")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #65
  {DBGFIELD("IIC_IntMulHWU")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #66
  {DBGFIELD("IIC_IntMulLI")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #67
  {DBGFIELD("IIC_VecPerm")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #68
  {DBGFIELD("IIC_LdStSTFD")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #69
  {DBGFIELD("IIC_LdStSTFDU")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #70
  {DBGFIELD("IIC_SprRFI")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #71
  {DBGFIELD("IIC_IntRFID")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #72
  {DBGFIELD("IIC_IntRotateD")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #73
  {DBGFIELD("IIC_IntRotate")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #74
  {DBGFIELD("IIC_SprSLBFEE")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #75
  {DBGFIELD("IIC_SprSLBIA")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #76
  {DBGFIELD("IIC_SprSLBIE")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #77
  {DBGFIELD("IIC_SprSLBIEG")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #78
  {DBGFIELD("IIC_SprSLBMFEE")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #79
  {DBGFIELD("IIC_SprSLBMFEV")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #80
  {DBGFIELD("IIC_SprSLBMTE")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #81
  {DBGFIELD("IIC_SprSLBSYNC")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #82
  {DBGFIELD("IIC_IntShift")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #83
  {DBGFIELD("IIC_LdStSTWCX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #84
  {DBGFIELD("IIC_LdStSTU")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #85
  {DBGFIELD("IIC_LdStSTUX")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #86
  {DBGFIELD("IIC_LdStSTD")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #87
  {DBGFIELD("IIC_LdStSTDCX")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #88
  {DBGFIELD("IIC_SprSTOP")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #89
  {DBGFIELD("IIC_IntTrapD")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #90
  {DBGFIELD("IIC_SprTLBIA")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #91
  {DBGFIELD("IIC_SprTLBIE")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #92
  {DBGFIELD("IIC_SprTLBIEL")      16383, false, false,  0, 0,  0, 0,  0, 0}, // #93
  {DBGFIELD("IIC_SprTLBSYNC")     16383, false, false,  0, 0,  0, 0,  0, 0}, // #94
  {DBGFIELD("IIC_IntTrapW")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #95
  {DBGFIELD("IIC_VecFPCompare")   16383, false, false,  0, 0,  0, 0,  0, 0}, // #96
  {DBGFIELD("VADDUBM_VADDUDM_VADDUHM_VADDUWM_VEXTSB2D_VEXTSB2Ds_VEXTSB2W_VEXTSB2Ws_VEXTSH2D_VEXTSH2Ds_VEXTSH2W_VEXTSH2Ws_VEXTSW2D_VEXTSW2Ds_VSLD_VSRD_VSUBUBM_VSUBUDM_VSUBUHM_VSUBUWM_VPOPCNTB_VPOPCNTH_VSRAD_MTVSRDD_VEQV_VNAND_VNEGD_VNEGW_VORC_XXLAND_XXLANDC_XXLEQV_XXLEQVOnes_XXLNAND_XXLNOR_XXLOR_XXLORf_XXLORC_XXLXOR_XXLXORdpz_XXLXORspz_XXLXORz") 2, false, false,  1, 7,  1, 5,  0, 0}, // #97
  {DBGFIELD("VAND_VANDC_V_SET0_V_SET0B_V_SET0H_VSLB_VSLH_VSLW_VSRB_VSRH_VSRW_VRLB_VRLD_VRLH_VRLW_VSRAB_VSRAH_VSRAW_XVABSDP_XVABSSP_XVNABSDP_XVNABSSP_XVCPSGNDP_XVCPSGNSP_XVIEXPDP_XVIEXPSP_XVXEXPDP_XVXEXPSP_VRLDMI_VRLDNM_VRLWMI_VRLWNM_VMRGEW_VMRGOW_VNOR_VOR_VSEL_VXOR_XVNEGDP_XVNEGSP_XSABSQP_XSCPSGNQP_XSIEXPQP_XSNABSQP_XSNEGQP_XSXEXPQP") 2, false, false,  1, 7,  1, 5,  0, 0}, // #98
  {DBGFIELD("XXSEL")              2, false, false,  1, 7,  1, 5,  0, 0}, // #99
  {DBGFIELD("TABORTDC_TABORTDCI_TABORTWC_TABORTWCI") 1, false, false,  8, 7,  6, 3,  0, 0}, // #100
  {DBGFIELD("MTFSB0_MTFSB1")      1, false, false,  8, 7,  6, 3,  0, 0}, // #101
  {DBGFIELD("MFFSCDRN_MFFSCDRNI_MFFSCRN_MFFSCRNI") 1, false, false,  8, 7,  6, 3,  0, 0}, // #102
  {DBGFIELD("CMPRB_CMPRB8_CMPEQB") 1, false, false,  8, 7,  6, 3,  0, 0}, // #103
  {DBGFIELD("TD_TDI")             1, false, false,  8, 7,  6, 3,  0, 0}, // #104
  {DBGFIELD("TW_TWI")             1, false, false,  8, 7,  6, 3,  0, 0}, // #105
  {DBGFIELD("FCMPUD_FCMPUS_FTDIV_FTSQRT") 1, false, false,  8, 7,  6, 3,  0, 0}, // #106
  {DBGFIELD("XSTSTDCDP_XSTSTDCSP") 1, false, false,  8, 7,  6, 3,  0, 0}, // #107
  {DBGFIELD("XSMAXCDP_XSMAXDP_XSMAXJDP_XSMINCDP_XSMINDP_XSMINJDP_XSXSIGDP_XSCVSPDPN") 1, false, false, 15, 3,  6, 3,  0, 0}, // #108
  {DBGFIELD("XSCMPEQDP_XSCMPEXPDP_XSCMPGEDP_XSCMPGTDP_XSCMPODP_XSCMPUDP_XSTDIVDP_XSTSQRTDP") 1, false, false, 15, 3,  6, 3,  0, 0}, // #109
  {DBGFIELD("CNTLZD_CNTLZD_rec_CNTLZW_CNTLZW8_CNTLZW8_rec_CNTLZW_rec_CNTTZD_CNTTZD_rec_CNTTZW_CNTTZW8_CNTTZW8_rec_CNTTZW_rec_POPCNTD_POPCNTW_CMPB_CMPB8_SETB_SETB8_BPERMD") 1, false, false, 15, 3,  6, 3,  0, 0}, // #110
  {DBGFIELD("SLD_SRD_SRAD")       1, false, false, 15, 3,  2, 3,  0, 0}, // #111
  {DBGFIELD("SRADI_EXTSWSLI_32_64_EXTSWSLI_SRADI_32_RLDIC") 1, false, false, 15, 3,  2, 3,  0, 0}, // #112
  {DBGFIELD("MFVRD_MFVSRD_MTVRD_MTVSRD_MTVRWA_MTVRWZ_MTVSRWA_MTVSRWZ_MFVSRWZ_MFVRWZ") 1, false, false, 15, 3,  2, 3,  0, 0}, // #113
  {DBGFIELD("CMPLW_CMPLWI_CMPW_CMPWI_CMPD_CMPDI_CMPLD_CMPLDI") 1, false, false, 15, 3,  2, 3,  0, 0}, // #114
  {DBGFIELD("SUBFC_SUBFC8_SUBFC8O_SUBFCO_SUBFIC_SUBFIC8_ANDI8_rec_ANDIS8_rec_ANDIS_rec_ANDI_rec_ADDC_ADDC8_ADDC8O_ADDCO_ADDIC_ADDIC8_ADDIC_rec_ADDE_ADDE8_ADDE8O_ADDE8O_rec_ADDE8_rec_ADDEO_ADDEO_rec_ADDE_rec_ADDME_ADDME8_ADDME8O_ADDME8O_rec_ADDME8_rec_ADDMEO_ADDMEO_rec_ADDME_rec_ADDZE_ADDZE8_ADDZE8O_ADDZE8O_rec_ADDZE8_rec_ADDZEO_ADDZEO_rec_ADDZE_rec_SUBF_SUBF8_SUBF8O_SUBF8O_rec_SUBF8_rec_SUBFE_SUBFE8_SUBFE8O_SUBFE8O_rec_SUBFE8_rec_SUBFEO_SUBFEO_rec_SUBFE_rec_SUBFME_SUBFME8_SUBFME8O_SUBFME8O_rec_SUBFME8_rec_SUBFMEO_SUBFMEO_rec_SUBFME_rec_SUBFO_SUBFO_rec_SUBFZE_SUBFZE8_SUBFZE8O_SUBFZE8O_rec_SUBFZE8_rec_SUBFZEO_SUBFZEO_rec_SUBFZE_rec_SUBF_rec_POPCNTB_LA") 1, false, false, 15, 3,  2, 3,  0, 0}, // #115
  {DBGFIELD("ADD4_ADD4O_ADD4O_rec_ADD4_rec_ADD8_ADD8O_ADD8O_rec_ADD8_rec_NEG_NEG8_NEG8O_NEG8O_rec_NEG8_rec_NEGO_NEGO_rec_NEG_rec_ADDI_ADDI8_ADDIS_ADDIS8_LI_LI8_LIS_LIS8_OR_OR8_OR8_rec_ORI_ORI8_ORIS_ORIS8_OR_rec_XOR_XOR8_XOR8_rec_XORI_XORI8_XORIS_XORIS8_XOR_rec_NAND_NAND8_NAND8_rec_NAND_rec_AND_AND8_AND8_rec_ANDC_ANDC8_ANDC8_rec_ANDC_rec_AND_rec_NOR_NOR8_NOR8_rec_NOR_rec_ORC_ORC8_ORC8_rec_ORC_rec_EQV_EQV8_EQV8_rec_EQV_rec_EXTSB_EXTSB8_EXTSB8_32_64_EXTSB8_rec_EXTSB_rec_EXTSH_EXTSH8_EXTSH8_32_64_EXTSH8_rec_EXTSH_rec_EXTSW_EXTSW_32_EXTSW_32_64_EXTSW_32_64_rec_EXTSW_rec_ADD4TLS_ADD8TLS_ADD8TLS__NOP") 1, false, false, 15, 3,  2, 3,  0, 0}, // #116
  {DBGFIELD("ADDIStocHA_ADDIStocHA8_ADDItocL_COPY") 1, false, false, 15, 3,  2, 3,  0, 0}, // #117
  {DBGFIELD("MCRF")               1, false, false, 15, 3,  2, 3,  0, 0}, // #118
  {DBGFIELD("MCRXRX")             1, false, false, 15, 3,  2, 3,  0, 0}, // #119
  {DBGFIELD("XSNABSDP_XSXEXPDP_XSABSDP_XSNEGDP_XSCPSGNDP") 1, false, false, 15, 3,  2, 3,  0, 0}, // #120
  {DBGFIELD("RFEBB")              1, false, false, 15, 3,  2, 3,  0, 0}, // #121
  {DBGFIELD("TBEGIN_TRECHKPT")    1, false, false, 15, 3,  2, 3,  0, 0}, // #122
  {DBGFIELD("WAIT")               1, false, false, 15, 3,  2, 3,  0, 0}, // #123
  {DBGFIELD("RLDCL_RLDCR")        1, false, false,  8, 7,  2, 3,  0, 0}, // #124
  {DBGFIELD("RLWIMI_RLWIMI8")     1, false, false,  8, 7,  2, 3,  0, 0}, // #125
  {DBGFIELD("RLDICL_RLDICL_32_RLDICL_32_64_RLDICR_RLDICR_32_RLDIMI") 1, false, false,  8, 7,  2, 3,  0, 0}, // #126
  {DBGFIELD("MFOCRF_MFOCRF8")     1, false, false,  8, 7,  2, 3,  0, 0}, // #127
  {DBGFIELD("MTOCRF_MTOCRF8")     1, false, false,  8, 7,  2, 3,  0, 0}, // #128
  {DBGFIELD("CR6SET_CR6UNSET_CRSET_CRUNSET_CRAND_CRANDC_CRNAND_CRNOR_CROR_CRORC_CREQV_CRXOR") 1, false, false,  8, 7,  2, 3,  0, 0}, // #129
  {DBGFIELD("SLW_SLW8_SRW_SRW8_RLWINM_RLWINM8_RLWNM_RLWNM8") 1, false, false,  8, 7,  2, 3,  0, 0}, // #130
  {DBGFIELD("FABSD_FABSS_FNABSD_FNABSS_FNEGD_FNEGS_FCPSGND_FCPSGNS_FMR") 1, false, false,  8, 7,  2, 3,  0, 0}, // #131
  {DBGFIELD("SRAW_SRAWI")         1, false, false,  8, 7,  2, 3,  0, 0}, // #132
  {DBGFIELD("ISEL_ISEL8")         1, false, false,  8, 7,  2, 3,  0, 0}, // #133
  {DBGFIELD("XSIEXPDP")           1, false, false,  8, 7,  2, 3,  0, 0}, // #134
  {DBGFIELD("TRECLAIM_TSR_TABORT") 1, false, false,  8, 7,  2, 3,  0, 0}, // #135
  {DBGFIELD("MFVSCR")             2, false, false,  1, 7,  9, 5,  0, 0}, // #136
  {DBGFIELD("MTVSCR")             2, false, false,  1, 7,  9, 5,  0, 0}, // #137
  {DBGFIELD("VCMPNEZB_VCMPNEZH_VCMPNEZW_VCMPEQUB_VCMPEQUD_VCMPEQUH_VCMPEQUW_VCMPNEB_VCMPNEH_VCMPNEW_VCMPEQFP_VCMPEQFP_rec_VCMPGEFP_VCMPGEFP_rec_VCMPGTFP_VCMPGTFP_rec_VCMPBFP_VCMPBFP_rec_VCMPGTSB_VCMPGTSB_rec_VCMPGTSD_VCMPGTSD_rec_VCMPGTSH_VCMPGTSH_rec_VCMPGTSW_VCMPGTSW_rec_VCMPGTUB_VCMPGTUB_rec_VCMPGTUD_VCMPGTUD_rec_VCMPGTUH_VCMPGTUH_rec_VCMPGTUW_VCMPGTUW_rec_VCMPNEB_rec_VCMPNEH_rec_VCMPNEW_rec_VCMPNEZB_rec_VCMPNEZH_rec_VCMPNEZW_rec_VCMPEQUB_rec_VCMPEQUD_rec_VCMPEQUH_rec_VCMPEQUW_rec_XVCMPEQDP_XVCMPEQDP_rec_XVCMPEQSP_XVCMPEQSP_rec_XVCMPGEDP_XVCMPGEDP_rec_XVCMPGESP_XVCMPGESP_rec_XVCMPGTDP_XVCMPGTDP_rec_XVCMPGTSP_XVCMPGTSP_rec") 2, false, false,  1, 7,  9, 5,  0, 0}, // #138
  {DBGFIELD("VABSDUB_VABSDUH_VABSDUW_VCLZB_VCLZD_VCLZH_VCLZW_VCTZB_VCTZD_VCTZH_VCTZW_VPOPCNTW_VPOPCNTD_VPRTYBD_VPRTYBW") 2, false, false,  1, 7,  9, 5,  0, 0}, // #139
  {DBGFIELD("VADDUBS_VADDUHS_VADDUWS_VAVGSB_VAVGSH_VAVGSW_VAVGUB_VAVGUH_VAVGUW_VADDSBS_VADDSHS_VADDSWS_VMAXFP_VMINFP_VMAXSB_VMAXSD_VMAXSH_VMAXSW_VMAXUB_VMAXUD_VMAXUH_VMAXUW_VMINSB_VMINSD_VMINSH_VMINSW_VMINUB_VMINUD_VMINUH_VMINUW_VBPERMD_VADDCUW_VSHASIGMAD_VSHASIGMAW_VSUBSBS_VSUBSHS_VSUBSWS_VSUBUBS_VSUBUHS_VSUBUWS_VSUBCUW_XVMAXDP_XVMAXSP_XVMINDP_XVMINSP_XVTSTDCDP_XVTSTDCSP_XVXSIGDP_XVXSIGSP") 2, false, false,  1, 7,  9, 5,  0, 0}, // #140
  {DBGFIELD("XVTDIVDP_XVTDIVSP_XVTSQRTDP_XVTSQRTSP") 2, false, false,  1, 7,  9, 5,  0, 0}, // #141
  {DBGFIELD("VADDFP_VCTSXS_VCTSXS_0_VCTUXS_VCTUXS_0_VEXPTEFP_VLOGEFP_VMADDFP_VMHADDSHS_VNMSUBFP_VREFP_VRFIM_VRFIN_VRFIP_VRFIZ_VRSQRTEFP_XVADDDP_XVADDSP_XVCVDPSP_XVCVDPSXDS_XVCVDPSXWS_XVCVDPUXDS_XVCVDPUXWS_XVCVHPSP_XVCVSPDP_XVCVSPHP_XVCVSPSXDS_XVCVSPSXWS_XVCVSPUXDS_XVCVSPUXWS_XVCVSXDDP_XVCVSXDSP_XVCVSXWDP_XVCVSXWSP_XVCVUXDDP_XVCVUXDSP_XVCVUXWDP_XVCVUXWSP_XVMADDADP_XVMADDASP_XVMADDMDP_XVMADDMSP_XVMSUBADP_XVMSUBASP_XVMSUBMDP_XVMSUBMSP_XVMULDP_XVMULSP_XVNMADDADP_XVNMADDASP_XVNMADDMDP_XVNMADDMSP_XVNMSUBADP_XVNMSUBASP_XVNMSUBMDP_XVNMSUBMSP_XVRDPI_XVRDPIC_XVRDPIM_XVRDPIP_XVRDPIZ_XVREDP_XVRESP_XVRSPI_XVRSPIC_XVRSPIM_XVRSPIP_XVRSPIZ_XVRSQRTEDP_XVRSQRTESP_XVSUBDP_XVSUBSP_VCFSX_VCFSX_0_VCFUX_VCFUX_0_VMHRADDSHS_VMLADDUHM_VMSUMMBM_VMSUMSHM_VMSUMSHS_VMSUMUBM_VMSUMUHM_VMSUMUHS_VMULESB_VMULESH_VMULESW_VMULEUB_VMULEUH_VMULEUW_VMULOSB_VMULOSH_VMULOSW_VMULOUB_VMULOUH_VMULOUW_VSUM2SWS_VSUM4SBS_VSUM4SHS_VSUM4UBS_VSUMSWS") 2, false, false, 18, 7, 14, 5,  0, 0}, // #142
  {DBGFIELD("VSUBFP_VMULUWM")     2, false, false, 18, 7, 14, 5,  0, 0}, // #143
  {DBGFIELD("MADDHD_MADDHDU_MADDLD_MADDLD8_MULLD_MULLDO") 1, false, false, 25, 7, 19, 3,  0, 0}, // #144
  {DBGFIELD("MULHD_MULHW_MULLW_MULLWO") 1, false, false, 25, 7, 19, 3,  0, 0}, // #145
  {DBGFIELD("MULHDU_MULHWU")      1, false, false, 25, 7, 19, 3,  0, 0}, // #146
  {DBGFIELD("MULLI_MULLI8")       1, false, false, 25, 7, 19, 3,  0, 0}, // #147
  {DBGFIELD("FRSP_FRIMD_FRIMS_FRIND_FRINS_FRIPD_FRIPS_FRIZD_FRIZS_FRE_FRES_FADDS_FMSUBS_FMADDS_FSUBS_FCFID_FCFIDS_FCFIDU_FCFIDUS_FCTID_FCTIDU_FCTIDUZ_FCTIDZ_FCTIW_FCTIWU_FCTIWUZ_FCTIWZ_FRSQRTE_FRSQRTES_FNMADDS_FNMSUBS_FSELD_FSELS_FMULS") 1, false, false, 25, 7, 15, 3,  0, 0}, // #148
  {DBGFIELD("FADD_FSUB")          1, false, false, 25, 7, 15, 3,  0, 0}, // #149
  {DBGFIELD("FMSUB_FMADD_FNMADD_FNMSUB_FMUL") 1, false, false, 25, 7, 15, 3,  0, 0}, // #150
  {DBGFIELD("XSMADDADP_XSMADDASP_XSMADDMDP_XSMADDMSP_XSMSUBADP_XSMSUBASP_XSMSUBMDP_XSMSUBMSP_XSMULDP_XSMULSP_XSNMADDADP_XSNMADDASP_XSNMADDMDP_XSNMADDMSP_XSNMSUBADP_XSNMSUBASP_XSNMSUBMDP_XSNMSUBMSP") 1, false, false, 25, 7, 15, 3,  0, 0}, // #151
  {DBGFIELD("FSELD_rec_FSELS_rec") 2, false, false, 32, 8, 22, 6,  0, 0}, // #152
  {DBGFIELD("MULHDU_rec_MULHWU_rec") 2, false, false, 32, 8, 28, 5,  0, 0}, // #153
  {DBGFIELD("MULHD_rec_MULHW_rec_MULLWO_rec_MULLW_rec") 2, false, false, 32, 8, 28, 5,  0, 0}, // #154
  {DBGFIELD("MULLDO_rec_MULLD_rec") 2, false, false, 32, 8, 28, 5,  0, 0}, // #155
  {DBGFIELD("FRIMD_rec_FRIMS_rec_FRIND_rec_FRINS_rec_FRIPD_rec_FRIPS_rec_FRIZD_rec_FRIZS_rec_FRES_rec_FRE_rec_FADDS_rec_FSUBS_rec_FMSUBS_rec_FNMSUBS_rec_FMADDS_rec_FNMADDS_rec_FCFIDS_rec_FCFIDUS_rec_FCFIDU_rec_FCFID_rec_FCTIDUZ_rec_FCTIDU_rec_FCTIDZ_rec_FCTID_rec_FCTIWUZ_rec_FCTIWU_rec_FCTIWZ_rec_FCTIW_rec_FMULS_rec_FRSQRTES_rec_FRSQRTE_rec_FRSP_rec") 2, false, false, 32, 8, 33, 5,  0, 0}, // #156
  {DBGFIELD("FADD_rec_FSUB_rec")  2, false, false, 32, 8, 33, 5,  0, 0}, // #157
  {DBGFIELD("FMSUB_rec_FNMSUB_rec_FMADD_rec_FNMADD_rec_FMUL_rec") 2, false, false, 32, 8, 33, 5,  0, 0}, // #158
  {DBGFIELD("XSADDDP_XSADDSP_XSCVDPHP_XSCVDPSP_XSCVDPSXDS_XSCVDPSXDSs_XSCVDPSXWS_XSCVDPUXDS_XSCVDPUXDSs_XSCVDPUXWS_XSCVDPSXWSs_XSCVDPUXWSs_XSCVHPDP_XSCVSPDP_XSCVSXDDP_XSCVSXDSP_XSCVUXDDP_XSCVUXDSP_XSRDPI_XSRDPIC_XSRDPIM_XSRDPIP_XSRDPIZ_XSREDP_XSRESP_XSRSQRTEDP_XSRSQRTESP_XSSUBDP_XSSUBSP_XSCVDPSPN_XSRSP") 1, false, false, 40, 3, 15, 3,  0, 0}, // #159
  {DBGFIELD("LVSL_LVSR")          1, false, false, 43, 5, 10, 4,  0, 0}, // #160
  {DBGFIELD("VSPLTISB_VSPLTISH_VSPLTISW_VSPLTB_VSPLTBs_VSPLTH_VSPLTHs_VSPLTW_XXMRGHW_XXMRGLW_XXPERM_XXPERMR_XXSLDWI_XXSLDWIs_XXSPLTIB_XXSPLTW_XXSPLTWs_XXPERMDI_XXPERMDIs") 1, false, false, 43, 5, 10, 4,  0, 0}, // #161
  {DBGFIELD("V_SETALLONES_V_SETALLONESB_V_SETALLONESH_VBPERMQ_VGBBD_VMRGHB_VMRGHH_VMRGHW_VMRGLB_VMRGLH_VMRGLW_VPERM_VPERMR_VPERMXOR_VPKPX_VPKSDSS_VPKSDUS_VPKSHSS_VPKSHUS_VPKSWSS_VPKSWUS_VPKUDUM_VPKUDUS_VPKUHUM_VPKUHUS_VPKUWUM_VPKUWUS_VSL_VSLDOI_VSLO_VSLV_VSR_VSRO_VSRV_VUPKHPX_VUPKHSB_VUPKHSH_VUPKHSW_VUPKLPX_VUPKLSB_VUPKLSH_VUPKLSW_XXBRD_XXBRH_XXBRQ_XXBRW_XXEXTRACTUW_XXINSERTW_VADDCUQ_VADDECUQ_VADDEUQM_VMUL10CUQ_VMUL10ECUQ_VMUL10EUQ_VMUL10UQ_VSUBCUQ_VSUBECUQ_VSUBEUQM_XSTSTDCQP_XSXSIGQP_BCDCFN_rec_BCDCFZ_rec_BCDCPSGN_rec_BCDCTN_rec_BCDCTZ_rec_BCDSETSGN_rec_BCDS_rec_BCDTRUNC_rec_BCDUS_rec_BCDUTRUNC_rec") 1, false, false, 43, 5, 10, 4,  0, 0}, // #162
  {DBGFIELD("VEXTRACTUB_VEXTRACTUH_VEXTRACTUW_VINSERTB_VINSERTD_VINSERTH_VINSERTW_MFVSRLD_MTVSRWS_VCLZLSBB_VCTZLSBB_VEXTRACTD_VEXTUBLX_VEXTUBRX_VEXTUHLX_VEXTUHRX_VEXTUWLX_VEXTUWRX_VPRTYBQ_VADDUQM_VSUBUQM") 1, false, false, 43, 5, 10, 4,  0, 0}, // #163
  {DBGFIELD("XSCMPEXPQP_XSCMPOQP_XSCMPUQP") 1, false, false, 43, 5, 10, 4,  0, 0}, // #164
  {DBGFIELD("BCDSR_rec_XSADDQP_XSADDQPO_XSCVDPQP_XSCVQPDP_XSCVQPDPO_XSCVQPSDZ_XSCVQPSWZ_XSCVQPUDZ_XSCVQPUWZ_XSCVSDQP_XSCVUDQP_XSRQPI_XSRQPIX_XSRQPXP_XSSUBQP_XSSUBQPO") 1, false, false, 48, 5, 38, 4,  0, 0}, // #165
  {DBGFIELD("BCDCTSQ_rec")        1, false, false, 53, 5, 42, 4,  0, 0}, // #166
  {DBGFIELD("XSMADDQP_XSMADDQPO_XSMSUBQP_XSMSUBQPO_XSMULQP_XSMULQPO_XSNMADDQP_XSNMADDQPO_XSNMSUBQP_XSNMSUBQPO") 1, false, false, 58, 5, 46, 4,  0, 0}, // #167
  {DBGFIELD("BCDCFSQ_rec")        1, false, false, 63, 5, 50, 4,  0, 0}, // #168
  {DBGFIELD("XSDIVQP_XSDIVQPO")   1, false, false, 68, 5, 54, 4,  0, 0}, // #169
  {DBGFIELD("XSSQRTQP_XSSQRTQPO") 1, false, false, 73, 5, 58, 4,  0, 0}, // #170
  {DBGFIELD("LXVL_LXVLL")         1, false, false, 78, 3, 62, 3,  0, 0}, // #171
  {DBGFIELD("LVEBX_LVEHX_LVEWX_LVX_LVXL") 1, false, false, 78, 3, 19, 3,  0, 0}, // #172
  {DBGFIELD("LXSIBZX_LXSIHZX_LXSDX_LXVB16X_LXVD2X_LXVWSX_LXSIWZX_LXV_LXVX_LXSD") 1, false, false, 78, 3, 19, 3,  0, 0}, // #173
  {DBGFIELD("DFLOADf64_XFLOADf64_LIWZX") 1, false, false, 78, 3, 19, 3,  0, 0}, // #174
  {DBGFIELD("DCBF_DCBFEP_DCBST_DCBSTEP_DCBT_DCBTEP_DCBZ_DCBZEP_DCBZL_DCBZLEP_DCBTST_DCBTSTEP") 1, false, false, 78, 3, 65, 3,  0, 0}, // #175
  {DBGFIELD("CP_COPY_CP_COPY8")   1, false, false, 78, 3, 65, 3,  0, 0}, // #176
  {DBGFIELD("CP_PASTE_CP_PASTE8") 1, false, false, 78, 3, 65, 3,  0, 0}, // #177
  {DBGFIELD("ICBI_ICBIEP")        1, false, false, 78, 3, 65, 3,  0, 0}, // #178
  {DBGFIELD("ICBT_ICBTLS_LBZ_LBZ8_LBZCIX_LBZX_LBZX8_LBZXTLS_LBZXTLS__LBZXTLS_32_LDBRX_LDCIX_LHBRX_LHBRX8_LHZ_LHZ8_LHZCIX_LHZX_LHZX8_LHZXTLS_LHZXTLS__LHZXTLS_32_LWBRX_LWBRX8_LWZ_LWZ8_LWZCIX_LWZX_LWZX8_LWZXTLS_LWZXTLS__LWZXTLS_32_EnforceIEIO_LSWI") 1, false, false, 78, 3, 65, 3,  0, 0}, // #179
  {DBGFIELD("LBARX_LBARXL_LHARX_LHARXL_LWARX_LWARXL") 1, false, false, 78, 3, 65, 3,  0, 0}, // #180
  {DBGFIELD("LD_LDX_LDXTLS_LDXTLS__DARN") 1, false, false, 78, 3, 65, 3,  0, 0}, // #181
  {DBGFIELD("LDARX_LDARXL")       1, false, false, 78, 3, 65, 3,  0, 0}, // #182
  {DBGFIELD("CP_ABORT")           1, false, false, 78, 3, 65, 3,  0, 0}, // #183
  {DBGFIELD("ISYNC")              1, false, false, 78, 3, 65, 3,  0, 0}, // #184
  {DBGFIELD("MSGSYNC")            1, false, false, 78, 3, 65, 3,  0, 0}, // #185
  {DBGFIELD("TLBSYNC")            1, false, false, 78, 3, 65, 3,  0, 0}, // #186
  {DBGFIELD("SYNC")               1, false, false, 78, 3, 65, 3,  0, 0}, // #187
  {DBGFIELD("LMW")                1, false, false, 78, 3, 65, 3,  0, 0}, // #188
  {DBGFIELD("LFIWZX_LFDX_LFD")    1, false, false, 81, 7, 65, 3,  0, 0}, // #189
  {DBGFIELD("SLBIA")              2, false, false, 88, 4, 68, 5,  0, 0}, // #190
  {DBGFIELD("SLBIE")              2, false, false, 88, 4, 68, 5,  0, 0}, // #191
  {DBGFIELD("SLBMFEE")            2, false, false, 88, 4, 68, 5,  0, 0}, // #192
  {DBGFIELD("SLBMFEV")            2, false, false, 88, 4, 68, 5,  0, 0}, // #193
  {DBGFIELD("SLBMTE")             2, false, false, 88, 4, 68, 5,  0, 0}, // #194
  {DBGFIELD("TLBIEL")             2, false, false, 88, 4, 68, 5,  0, 0}, // #195
  {DBGFIELD("LHZU_LHZU8_LWZU_LWZU8") 2, false, false, 92, 6, 73, 6,  0, 0}, // #196
  {DBGFIELD("LHZUX_LHZUX8_LWZUX_LWZUX8") 2, false, false, 92, 6, 73, 6,  0, 0}, // #197
  {DBGFIELD("TEND")               2, false, false, 98, 5, 73, 6,  0, 0}, // #198
  {DBGFIELD("STBCX_STHCX_STWCX")  2, false, false, 103, 9, 79, 6,  0, 0}, // #199
  {DBGFIELD("STDCX")              2, false, false, 103, 9, 79, 6,  0, 0}, // #200
  {DBGFIELD("LDMX")               2, false, false, 112, 3, 85, 5,  0, 0}, // #201
  {DBGFIELD("LHA_LHA8_LHAX_LHAX8_LWAX_LWAX_32") 2, false, false, 98, 5, 90, 5,  0, 0}, // #202
  {DBGFIELD("CP_PASTE8_rec_CP_PASTE_rec") 2, false, false, 98, 5, 90, 5,  0, 0}, // #203
  {DBGFIELD("LWA_LWA_32")         2, false, false, 98, 5, 90, 5,  0, 0}, // #204
  {DBGFIELD("TCHECK")             2, false, false, 98, 5, 90, 5,  0, 0}, // #205
  {DBGFIELD("LFIWAX")             2, false, false, 115, 9, 90, 5,  0, 0}, // #206
  {DBGFIELD("LXSIWAX")            2, false, false, 98, 5, 28, 5,  0, 0}, // #207
  {DBGFIELD("LIWAX")              2, false, false, 98, 5, 28, 5,  0, 0}, // #208
  {DBGFIELD("LFSX_LFS")           2, false, false, 115, 9, 28, 5,  0, 0}, // #209
  {DBGFIELD("LXSSP_LXSSPX")       2, false, false, 98, 5, 85, 5,  0, 0}, // #210
  {DBGFIELD("XFLOADf32_DFLOADf32") 2, false, false, 98, 5, 85, 5,  0, 0}, // #211
  {DBGFIELD("LHAU_LHAU8")         3, false, false, 124, 6, 95, 8,  0, 0}, // #212
  {DBGFIELD("LHAUX_LHAUX8_LWAUX") 3, false, false, 124, 6, 95, 8,  0, 0}, // #213
  {DBGFIELD("LXVH8X_LXVDSX_LXVW4X") 2, false, false, 130, 7, 103, 6,  0, 0}, // #214
  {DBGFIELD("STFD_STFDX_STFIWX_STFS_STFSX_STXSD_STXSDX_STXSIBX_STXSIBXv_STXSIHX_STXSIHXv_STXSIWX_STXSSP_STXSSPX") 1, false, false, 137, 8, 24, 4,  0, 0}, // #215
  {DBGFIELD("STW_STW8_STDBRX_STHBRX_STWBRX_STB_STB8_STH_STH8_STBX_STBX8_STBXTLS_STBXTLS__STBXTLS_32_STHX_STHX8_STHXTLS_STHXTLS__STHXTLS_32_STWX_STWX8_STWXTLS_STWXTLS__STWXTLS_32") 1, false, false, 137, 8, 24, 4,  0, 0}, // #216
  {DBGFIELD("DFSTOREf32_DFSTOREf64_XFSTOREf32_XFSTOREf64_STIWX") 1, false, false, 137, 8, 24, 4,  0, 0}, // #217
  {DBGFIELD("STD_STDX_STDXTLS_STDXTLS_") 1, false, false, 137, 8, 24, 4,  0, 0}, // #218
  {DBGFIELD("STBCIX_STDCIX_STHCIX_STWCIX_STSWI") 1, false, false, 137, 8, 24, 4,  0, 0}, // #219
  {DBGFIELD("SLBIEG")             1, false, false, 137, 8, 24, 4,  0, 0}, // #220
  {DBGFIELD("STMW")               1, false, false, 137, 8, 24, 4,  0, 0}, // #221
  {DBGFIELD("TLBIE")              1, false, false, 137, 8, 24, 4,  0, 0}, // #222
  {DBGFIELD("STVEBX_STVEHX_STVEWX_STVX_STVXL") 1, false, false, 145, 6, 80, 5,  0, 0}, // #223
  {DBGFIELD("STXV_STXVB16X_STXVD2X_STXVH8X_STXVW4X_STXVX") 1, false, false, 145, 6, 80, 5,  0, 0}, // #224
  {DBGFIELD("STXVL_STXVLL")       1, false, false, 145, 6, 80, 5,  0, 0}, // #225
  {DBGFIELD("MTCTR_MTCTR8_MTCTR8loop_MTCTRloop_MTLR_MTLR8") 1, false, false, 151, 8, 109, 4,  0, 0}, // #226
  {DBGFIELD("MFVRSAVE_MFVRSAVEv_MTVRSAVE_MTVRSAVEv") 1, false, false, 151, 8, 38, 4,  0, 0}, // #227
  {DBGFIELD("MFPMR")              1, false, false, 151, 8, 38, 4,  0, 0}, // #228
  {DBGFIELD("MTPMR")              1, false, false, 151, 8, 38, 4,  0, 0}, // #229
  {DBGFIELD("MFTB_MFTB8")         1, false, false, 151, 8, 38, 4,  0, 0}, // #230
  {DBGFIELD("MFCTR_MFCTR8_MFLR_MFLR8_MFSPR_MFSPR8") 1, false, false, 151, 8, 38, 4,  0, 0}, // #231
  {DBGFIELD("MFMSR")              1, false, false, 151, 8, 38, 4,  0, 0}, // #232
  {DBGFIELD("MTMSR")              1, false, false, 151, 8, 38, 4,  0, 0}, // #233
  {DBGFIELD("MTMSRD")             1, false, false, 151, 8, 38, 4,  0, 0}, // #234
  {DBGFIELD("MTSPR_MTSPR8")       1, false, false, 151, 8, 38, 4,  0, 0}, // #235
  {DBGFIELD("DIVW_DIVWO_DIVWU_DIVWUO_MODSW") 1, false, false, 159, 8, 113, 4,  0, 0}, // #236
  {DBGFIELD("DIVWE_DIVWEO_DIVWEU_DIVWEUO_MODSD_MODUD_MODUW") 1, false, false, 159, 8, 46, 4,  0, 0}, // #237
  {DBGFIELD("DIVD_DIVDO_DIVDU_DIVDUO") 1, false, false, 159, 8, 46, 4,  0, 0}, // #238
  {DBGFIELD("DIVDE_DIVDEO_DIVDEU_DIVDEUO") 1, false, false, 159, 8, 117, 4,  0, 0}, // #239
  {DBGFIELD("DIVWO_rec_DIVWUO_rec_DIVWU_rec_DIVW_rec") 2, false, false, 167, 9, 121, 6,  0, 0}, // #240
  {DBGFIELD("DIVD_rec_DIVDO_rec_DIVDU_rec_DIVDUO_rec") 2, false, false, 167, 9, 127, 6,  0, 0}, // #241
  {DBGFIELD("DIVWE_rec_DIVWEO_rec_DIVWEU_rec_DIVWEUO_rec") 2, false, false, 167, 9, 127, 6,  0, 0}, // #242
  {DBGFIELD("DIVDE_rec_DIVDEO_rec_DIVDEU_rec_DIVDEUO_rec") 2, false, false, 167, 9, 133, 6,  0, 0}, // #243
  {DBGFIELD("MTCRF_MTCRF8")       2, false, false, 176, 7, 139, 6,  0, 0}, // #244
  {DBGFIELD("ADDC8O_rec_ADDC8_rec_ADDCO_rec_ADDC_rec_SUBFC8O_rec_SUBFC8_rec_SUBFCO_rec_SUBFC_rec") 2, false, false, 183, 3, 139, 6,  0, 0}, // #245
  {DBGFIELD("FABSD_rec_FABSS_rec_FNABSD_rec_FNABSS_rec_FCPSGND_rec_FCPSGNS_rec_FNEGD_rec_FNEGS_rec_FMR_rec") 2, false, false, 186, 7, 145, 6,  0, 0}, // #246
  {DBGFIELD("MCRFS")              2, false, false, 183, 3, 151, 6,  0, 0}, // #247
  {DBGFIELD("MTFSF_MTFSF_rec_MTFSFI_MTFSFI_rec") 2, false, false, 176, 7, 151, 6,  0, 0}, // #248
  {DBGFIELD("MTFSFb")             2, false, false, 176, 7, 151, 6,  0, 0}, // #249
  {DBGFIELD("RLDCL_rec_RLDCR_rec") 2, false, false, 186, 7, 157, 5,  0, 0}, // #250
  {DBGFIELD("RLDICL_rec_RLDICR_rec_RLDICL_32_rec_RLDIMI_rec") 2, false, false, 186, 7, 157, 5,  0, 0}, // #251
  {DBGFIELD("RLWIMI8_rec_RLWIMI_rec") 2, false, false, 186, 7, 157, 5,  0, 0}, // #252
  {DBGFIELD("RLWINM8_rec_RLWINM_rec_RLWNM8_rec_RLWNM_rec_SLW8_rec_SLW_rec_SRW8_rec_SRW_rec") 2, false, false, 186, 7, 157, 5,  0, 0}, // #253
  {DBGFIELD("SRAWI_rec_SRAW_rec") 2, false, false, 186, 7, 157, 5,  0, 0}, // #254
  {DBGFIELD("MFFS_MFFSCE_MFFSL_MFFS_rec") 2, false, false, 176, 7, 90, 5,  0, 0}, // #255
  {DBGFIELD("MFCR_MFCR8")         3, false, false, 193, 7, 162, 7,  0, 0}, // #256
  {DBGFIELD("EXTSWSLI_32_64_rec_SRADI_rec_EXTSWSLI_rec_RLDIC_rec") 2, false, false, 183, 3, 157, 5,  0, 0}, // #257
  {DBGFIELD("SRAD_rec_SLD_rec_SRD_rec") 2, false, false, 183, 3, 157, 5,  0, 0}, // #258
  {DBGFIELD("FDIV")               1, false, false, 200, 7, 169, 3,  0, 0}, // #259
  {DBGFIELD("FDIV_rec")           2, false, false, 207, 8, 172, 5,  0, 0}, // #260
  {DBGFIELD("XSSQRTDP")           1, false, false, 215, 3, 172, 3,  0, 0}, // #261
  {DBGFIELD("FSQRT")              1, false, false, 218, 7, 172, 3,  0, 0}, // #262
  {DBGFIELD("XVSQRTDP")           2, false, false, 225, 5, 177, 5,  0, 0}, // #263
  {DBGFIELD("XVSQRTSP")           2, false, false, 225, 5, 182, 5,  0, 0}, // #264
  {DBGFIELD("FSQRT_rec")          2, false, false, 230, 8, 187, 5,  0, 0}, // #265
  {DBGFIELD("XSSQRTSP")           1, false, false, 238, 3, 192, 3,  0, 0}, // #266
  {DBGFIELD("FSQRTS")             1, false, false, 241, 7, 192, 3,  0, 0}, // #267
  {DBGFIELD("FSQRTS_rec")         2, false, false, 248, 8, 195, 5,  0, 0}, // #268
  {DBGFIELD("XSDIVDP")            1, false, false, 256, 3, 169, 3,  0, 0}, // #269
  {DBGFIELD("FDIVS")              1, false, false, 241, 7, 192, 3,  0, 0}, // #270
  {DBGFIELD("FDIVS_rec")          2, false, false, 248, 8, 195, 5,  0, 0}, // #271
  {DBGFIELD("XSDIVSP")            1, false, false, 238, 3, 192, 3,  0, 0}, // #272
  {DBGFIELD("XVDIVSP")            2, false, false, 259, 7, 200, 5,  0, 0}, // #273
  {DBGFIELD("XVDIVDP")            2, false, false, 259, 7, 205, 5,  0, 0}, // #274
  {DBGFIELD("LFSU")               3, false, false, 266, 9, 210, 8,  0, 0}, // #275
  {DBGFIELD("LFSUX")              3, false, false, 266, 9, 210, 8,  0, 0}, // #276
  {DBGFIELD("STFDU_STFDUX_STFSU_STFSUX") 2, false, false, 103, 9, 218, 7,  0, 0}, // #277
  {DBGFIELD("STBU_STBU8_STDU_STHU_STHU8_STWU_STWU8") 2, false, false, 103, 9, 218, 7,  0, 0}, // #278
  {DBGFIELD("STBUX_STBUX8_STDUX_STHUX_STHUX8_STWUX_STWUX8") 2, false, false, 103, 9, 218, 7,  0, 0}, // #279
  {DBGFIELD("LBZU_LBZU8")         2, false, false, 92, 6, 73, 6,  0, 0}, // #280
  {DBGFIELD("LBZUX_LBZUX8")       2, false, false, 92, 6, 73, 6,  0, 0}, // #281
  {DBGFIELD("LDU")                2, false, false, 92, 6, 73, 6,  0, 0}, // #282
  {DBGFIELD("LDUX")               2, false, false, 92, 6, 73, 6,  0, 0}, // #283
  {DBGFIELD("LFDU")               2, false, false, 275, 9, 73, 6,  0, 0}, // #284
  {DBGFIELD("LFDUX")              2, false, false, 275, 9, 73, 6,  0, 0}, // #285
  {DBGFIELD("VPMSUMB_VPMSUMD_VPMSUMH_VPMSUMW_VCIPHER_VCIPHERLAST_VNCIPHER_VNCIPHERLAST_VSBOX") 1, false, false, 284, 5, 90, 4,  0, 0}, // #286
  {DBGFIELD("BCCCTR_BCCCTR8_BCCCTRL_BCCCTRL8_BCCL_BCCLA_BCCLR_BCCLRL_BCCTR_BCCTR8_BCCTR8n_BCCTRL_BCCTRL8_BCCTRL8n_BCCTRLn_BCCTRn_BDNZ_BDNZ8_BDNZA_BDNZAm_BDNZAp_BDNZm_BDNZp_BDZ_BDZ8_BDZA_BDZAm_BDZAp_BDZm_BDZp_BDNZL_BDNZLA_BDNZLAm_BDNZLAp_BDNZLR_BDNZLR8_BDNZLRL_BDNZLRLm_BDNZLRLp_BDNZLRm_BDNZLRp_BDNZLm_BDNZLp_BDZL_BDZLA_BDZLAm_BDZLAp_BDZLR_BDZLR8_BDZLRL_BDZLRLm_BDZLRLp_BDZLRm_BDZLRp_BDZLm_BDZLp_BL_BL_NOP_BL_TLS_BL8_BL8_NOP_BL8_NOP_TLS_BL8_TLS_BL8_TLS__BLA_BLA8_BLA8_NOP_BLR_BLR8_BLRL_TAILB_TAILB8_TAILBA_TAILBA8_TAILBCTR_TAILBCTR8_gBC_gBCA_gBCAat_gBCCTR_gBCCTRL_gBCL_gBCLA_gBCLAat_gBCLR_gBCLRL_gBCLat_gBCat_BCLR_BCLRL_BCLRLn_BCLRn_BCTR_BCTR8_BCTRL_BCTRL8_B_BA_BC_BCC_BCCA_BCL_BCLalways_BCLn_BCTRL8_LDinto_toc_BCTRL_LWZinto_toc_BCn_CTRL_DEP") 1, false, false, 289, 3,  2, 2,  0, 0}, // #287
  {DBGFIELD("ADDPCIS")            2, false, false, 292, 5, 15, 4,  0, 0}, // #288
  {DBGFIELD("LDAT_LWAT")          5, false, false, 297, 8, 225, 18,  0, 0}, // #289
  {DBGFIELD("STDAT_STWAT")        3, false, false, 305, 8, 243, 10,  0, 0}, // #290
  {DBGFIELD("BRINC")              16383, false, false,  0, 0,  0, 0,  0, 0}, // #291
  {DBGFIELD("EVABS_EVEQV_EVNAND_EVNEG_EVADDIW_EVADDW_EVAND_EVANDC_EVCMPEQ_EVCMPGTS_EVCMPGTU_EVCMPLTS_EVCMPLTU_EVCNTLSW_EVCNTLZW_EVEXTSB_EVEXTSH_EVMERGEHI_EVMERGEHILO_EVMERGELO_EVMERGELOHI_EVNOR_EVOR_EVORC_EVXOR_EVRLW_EVRLWI_EVRNDW_EVSLW_EVSLWI_EVSPLATFI_EVSPLATI_EVSRWIS_EVSRWIU_EVSRWS_EVSRWU_EVSUBFW_EVSUBIFW") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #292
  {DBGFIELD("EVMRA_EVADDSMIAAW_EVADDSSIAAW_EVADDUMIAAW_EVADDUSIAAW_EVDIVWS_EVDIVWU_EVMHEGSMFAA_EVMHEGSMFAN_EVMHEGSMIAA_EVMHEGSMIAN_EVMHEGUMIAA_EVMHEGUMIAN_EVMHESMF_EVMHESMFA_EVMHESMFAAW_EVMHESMFANW_EVMHESMI_EVMHESMIA_EVMHESMIAAW_EVMHESMIANW_EVMHESSF_EVMHESSFA_EVMHESSFAAW_EVMHESSFANW_EVMHESSIAAW_EVMHESSIANW_EVMHEUMI_EVMHEUMIA_EVMHEUMIAAW_EVMHEUMIANW_EVMHEUSIAAW_EVMHEUSIANW_EVMHOGSMFAA_EVMHOGSMFAN_EVMHOGSMIAA_EVMHOGSMIAN_EVMHOGUMIAA_EVMHOGUMIAN_EVMHOSMF_EVMHOSMFA_EVMHOSMFAAW_EVMHOSMFANW_EVMHOSMI_EVMHOSMIA_EVMHOSMIAAW_EVMHOSMIANW_EVMHOSSF_EVMHOSSFA_EVMHOSSFAAW_EVMHOSSFANW_EVMHOSSIAAW_EVMHOSSIANW_EVMHOUMI_EVMHOUMIA_EVMHOUMIAAW_EVMHOUMIANW_EVMHOUSIAAW_EVMHOUSIANW_EVMWHSMF_EVMWHSMFA_EVMWHSMI_EVMWHSMIA_EVMWHSSF_EVMWHSSFA_EVMWHUMI_EVMWHUMIA_EVMWLSMIAAW_EVMWLSMIANW_EVMWLSSIAAW_EVMWLSSIANW_EVMWLUMI_EVMWLUMIA_EVMWLUMIAAW_EVMWLUMIANW_EVMWLUSIAAW_EVMWLUSIANW_EVMWSMF_EVMWSMFA_EVMWSMFAA_EVMWSMFAN_EVMWSMI_EVMWSMIA_EVMWSMIAA_EVMWSMIAN_EVMWSSF_EVMWSSFA_EVMWSSFAA_EVMWSSFAN_EVMWUMI_EVMWUMIA_EVMWUMIAA_EVMWUMIAN_EVSUBFSMIAAW_EVSUBFSSIAAW_EVSUBFUMIAAW_EVSUBFUSIAAW") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #293
  {DBGFIELD("EVLDD_EVLDDX_EVLDH_EVLDHX_EVLDW_EVLDWX_EVLHHESPLAT_EVLHHESPLATX_EVLHHOSSPLAT_EVLHHOSSPLATX_EVLHHOUSPLAT_EVLHHOUSPLATX_EVLWHE_EVLWHEX_EVLWHOS_EVLWHOSX_EVLWHOU_EVLWHOUX_EVLWHSPLAT_EVLWHSPLATX_EVLWWSPLAT_EVLWWSPLATX") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #294
  {DBGFIELD("EVSTDD_EVSTDDX_EVSTDH_EVSTDHX_EVSTDW_EVSTDWX_EVSTWHE_EVSTWHEX_EVSTWHO_EVSTWHOX_EVSTWWE_EVSTWWEX_EVSTWWO_EVSTWWOX") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #295
  {DBGFIELD("HRFID_ATTN_CLRBHRB_MFBHRBE_NAP_RFCI_RFDI_RFMCI_SC") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #296
  {DBGFIELD("RFI")                16383, false, false,  0, 0,  0, 0,  0, 0}, // #297
  {DBGFIELD("RFID")               16383, false, false,  0, 0,  0, 0,  0, 0}, // #298
  {DBGFIELD("DSS_DSSALL_DST_DST64_DSTST_DSTST64_DSTSTT_DSTSTT64_DSTT_DSTT64_ICBLQ_LBEPX_LHEPX_LWEPX_TLBIVAX_TLBLD_TLBLI_TLBRE_TLBRE2_TLBSX_TLBSX2_TLBSX2D_TLBWE_TLBWE2_MBAR_TRAP_DCCCI_ICCCI") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #299
  {DBGFIELD("ICBLC_STBEPX_STHEPX_STWEPX") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #300
  {DBGFIELD("LFDEPX")             16383, false, false,  0, 0,  0, 0,  0, 0}, // #301
  {DBGFIELD("STFDEPX")            16383, false, false,  0, 0,  0, 0,  0, 0}, // #302
  {DBGFIELD("MFSR_MFSRIN")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #303
  {DBGFIELD("MTSR_MTSRIN")        16383, false, false,  0, 0,  0, 0,  0, 0}, // #304
  {DBGFIELD("MFDCR")              16383, false, false,  0, 0,  0, 0,  0, 0}, // #305
  {DBGFIELD("MTDCR")              16383, false, false,  0, 0,  0, 0,  0, 0}, // #306
  {DBGFIELD("NOP_GT_PWR6_NOP_GT_PWR7") 16383, false, false,  0, 0,  0, 0,  0, 0}, // #307
  {DBGFIELD("TLBIA")              16383, false, false,  0, 0,  0, 0,  0, 0}, // #308
  {DBGFIELD("WRTEE_WRTEEI")       16383, false, false,  0, 0,  0, 0,  0, 0}, // #309
  {DBGFIELD("MSYNC")              16383, false, false,  0, 0,  0, 0,  0, 0}, // #310
  {DBGFIELD("SLBSYNC")            16383, false, false,  0, 0,  0, 0,  0, 0}, // #311
  {DBGFIELD("SLBFEE_rec")         16383, false, false,  0, 0,  0, 0,  0, 0}, // #312
  {DBGFIELD("STOP")               16383, false, false,  0, 0,  0, 0,  0, 0}, // #313
  {DBGFIELD("DCBA_DCBI")          16383, false, false,  0, 0,  0, 0,  0, 0}, // #314
}; // P9ModelSchedClasses

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

static const llvm::MCSchedModel PPC440Model = {
  2, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  5, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  1, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  PPC440Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel G3ItinerariesModel = {
  MCSchedModel::DefaultIssueWidth,
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  MCSchedModel::DefaultLoadLatency,
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  2, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  G3Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel G4ItinerariesModel = {
  MCSchedModel::DefaultIssueWidth,
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  MCSchedModel::DefaultLoadLatency,
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  3, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  G4Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel G4PlusItinerariesModel = {
  MCSchedModel::DefaultIssueWidth,
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  MCSchedModel::DefaultLoadLatency,
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  4, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  G4PlusItineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel G5Model = {
  4, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  3, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  16, // MispredictPenalty
  false, // PostRAScheduler
  false, // CompleteModel
  5, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  G5Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel PPCA2Model = {
  1, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  6, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  13, // MispredictPenalty
  false, // PostRAScheduler
  false, // CompleteModel
  6, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  PPCA2Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel PPCE500Model = {
  2, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  5, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  7, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  PPCE500Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel PPCE500mcModel = {
  2, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  5, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  8, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  PPCE500mcItineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel PPCE5500Model = {
  2, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  MCSchedModel::DefaultLoopMicroOpBufferSize,
  6, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  MCSchedModel::DefaultMispredictPenalty,
  false, // PostRAScheduler
  false, // CompleteModel
  9, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  PPCE5500Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel P8Model = {
  8, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  60, // LoopMicroOpBufferSize
  3, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  16, // MispredictPenalty
  false, // PostRAScheduler
  false, // CompleteModel
  10, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  P8Itineraries,
  nullptr // No extra processor descriptor
};

static const llvm::MCSchedModel P7Model = {
  6, // IssueWidth
  MCSchedModel::DefaultMicroOpBufferSize,
  40, // LoopMicroOpBufferSize
  3, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  16, // MispredictPenalty
  false, // PostRAScheduler
  false, // CompleteModel
  11, // Processor ID
  nullptr, nullptr, 0, 0, // No instruction-level machine model.
  P7Itineraries,
  nullptr // No extra processor descriptor
};

static const unsigned P9ModelProcResourceSubUnits[] = {
  0,  // Invalid
  10,   10,   11,   11,   12,   12,   // DISP_NBR
  10,   10,   11,   11,   // DISP_SS
};

// {Name, NumUnits, SuperIdx, BufferSize, SubUnitsIdxBegin}
static const llvm::MCProcResourceDesc P9ModelProcResources[] = {
  {"InvalidUnit", 0, 0, 0, 0},
  {"ALU",             4, 0, -1, nullptr}, // #1
  {"ALUE",            2, 1, -1, nullptr}, // #2, Super=ALU
  {"ALUO",            2, 1, -1, nullptr}, // #3, Super=ALU
  {"BR",              1, 0, 16, nullptr}, // #4
  {"CY",              1, 0, -1, nullptr}, // #5
  {"DFU",             1, 0, -1, nullptr}, // #6
  {"DISP_NBR",        6, 0, -1, P9ModelProcResourceSubUnits + 1}, // #7
  {"DISP_SS",         4, 0, -1, P9ModelProcResourceSubUnits + 7}, // #8
  {"DISPb01",         2, 0, -1, nullptr}, // #9
  {"DISPx02",         2, 0, -1, nullptr}, // #10
  {"DISPx13",         2, 0, -1, nullptr}, // #11
  {"DISPxab",         2, 0, -1, nullptr}, // #12
  {"DIV",             2, 0, -1, nullptr}, // #13
  {"DP",              4, 0, -1, nullptr}, // #14
  {"DPE",             2, 14, -1, nullptr}, // #15, Super=DP
  {"DPO",             2, 14, -1, nullptr}, // #16, Super=DP
  {"IP_AGEN",         4, 0, -1, nullptr}, // #17
  {"IP_EXEC",         4, 0, -1, nullptr}, // #18
  {"IP_EXECE",        2, 18, -1, nullptr}, // #19, Super=IP_EXEC
  {"IP_EXECO",        2, 18, -1, nullptr}, // #20, Super=IP_EXEC
  {"LS",              4, 0, -1, nullptr}, // #21
  {"PM",              2, 0, -1, nullptr}, // #22
};

static const llvm::MCSchedModel P9Model = {
  8, // IssueWidth
  44, // MicroOpBufferSize
  60, // LoopMicroOpBufferSize
  5, // LoadLatency
  MCSchedModel::DefaultHighLatency,
  16, // MispredictPenalty
  false, // PostRAScheduler
  true, // CompleteModel
  12, // Processor ID
  P9ModelProcResources,
  P9ModelSchedClasses,
  23,
  315,
  nullptr, // No Itinerary
  nullptr // No extra processor descriptor
};

// Sorted (by key) array of values for CPU subtype.
extern const llvm::SubtargetSubTypeKV PPCSubTypeKV[] = {
 { "440", { { { 0x305060040000008ULL, 0x0ULL, 0x0ULL, } } }, &PPC440Model },
 { "450", { { { 0x305060040000008ULL, 0x0ULL, 0x0ULL, } } }, &PPC440Model },
 { "601", { { { 0x8000000010ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "602", { { { 0x100008000000020ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "603", { { { 0x100060000000040ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "603e", { { { 0x100060000000040ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "603ev", { { { 0x100060000000040ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "604", { { { 0x100060000000080ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "604e", { { { 0x100060000000080ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "620", { { { 0x100060000000100ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "7400", { { { 0x100060010000800ULL, 0x0ULL, 0x0ULL, } } }, &G4ItinerariesModel },
 { "7450", { { { 0x100060010000800ULL, 0x0ULL, 0x0ULL, } } }, &G4PlusItinerariesModel },
 { "750", { { { 0x100060000000200ULL, 0x0ULL, 0x0ULL, } } }, &G4ItinerariesModel },
 { "970", { { { 0x180160014000400ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "a2", { { { 0x1b51f70c4001000ULL, 0x540ULL, 0x0ULL, } } }, &PPCA2Model },
 { "a2q", { { { 0x1b51f70c4001000ULL, 0x560ULL, 0x0ULL, } } }, &PPCA2Model },
 { "e500", { { { 0x105000040002000ULL, 0x80ULL, 0x0ULL, } } }, &PPCE500Model },
 { "e500mc", { { { 0x105000040004000ULL, 0x100ULL, 0x0ULL, } } }, &PPCE500mcModel },
 { "e5500", { { { 0x185000044008000ULL, 0x100ULL, 0x0ULL, } } }, &PPCE5500Model },
 { "future", { { { 0xfdb75f7ab5c00001ULL, 0x3950ULL, 0x0ULL, } } }, &NoSchedModel },
 { "g3", { { { 0x100060000000200ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "g4", { { { 0x100060010000800ULL, 0x0ULL, 0x0ULL, } } }, &G4ItinerariesModel },
 { "g4+", { { { 0x100060010000800ULL, 0x0ULL, 0x0ULL, } } }, &G4PlusItinerariesModel },
 { "g5", { { { 0x180160014000401ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "generic", { { { 0x100800000000002ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "ppc", { { { 0x100800000000002ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "ppc32", { { { 0x100800000000002ULL, 0x0ULL, 0x0ULL, } } }, &G3ItinerariesModel },
 { "ppc64", { { { 0x180160014000004ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "ppc64le", { { { 0x9db55f7ab4c00001ULL, 0x3950ULL, 0x0ULL, } } }, &P8Model },
 { "pwr3", { { { 0x80060014010000ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "pwr4", { { { 0x180160014020000ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "pwr5", { { { 0x1801f0014040001ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "pwr5x", { { { 0x1801f4014080001ULL, 0x100ULL, 0x0ULL, } } }, &G5Model },
 { "pwr6", { { { 0x1a01f5094100001ULL, 0x140ULL, 0x0ULL, } } }, &G5Model },
 { "pwr6x", { { { 0x1a01f5094080001ULL, 0x140ULL, 0x0ULL, } } }, &G5Model },
 { "pwr7", { { { 0x81b41f78b4400001ULL, 0x3940ULL, 0x0ULL, } } }, &P7Model },
 { "pwr8", { { { 0x9db55f7ab4c00001ULL, 0x3950ULL, 0x0ULL, } } }, &P8Model },
 { "pwr9", { { { 0xfdb75f7ab5c00001ULL, 0x795cULL, 0x0ULL, } } }, &P9Model },
};

namespace PPC_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass,
    const MCInst *MI, unsigned CPUID) {
  // Don't know how to resolve this scheduling class.
  return 0;
}
} // end namespace PPC_MC

struct PPCGenMCSubtargetInfo : public MCSubtargetInfo {
  PPCGenMCSubtargetInfo(const Triple &TT, 
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
    return PPC_MC::resolveVariantSchedClassImpl(SchedClass, MI, CPUID); 
  }
};

static inline MCSubtargetInfo *createPPCMCSubtargetInfoImpl(const Triple &TT, StringRef CPU, StringRef FS) {
  return new PPCGenMCSubtargetInfo(TT, CPU, FS, PPCFeatureKV, PPCSubTypeKV, 
                      PPCWriteProcResTable, PPCWriteLatencyTable, PPCReadAdvanceTable, 
                      PPCStages, PPCOperandCycles, PPCForwardingPaths);
}

} // end namespace llvm

#endif // GET_SUBTARGETINFO_MC_DESC


#ifdef GET_SUBTARGETINFO_TARGET_DESC
#undef GET_SUBTARGETINFO_TARGET_DESC

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

// ParseSubtargetFeatures - Parses features string setting specified
// subtarget options.
void llvm::PPCSubtarget::ParseSubtargetFeatures(StringRef CPU, StringRef FS) {
  LLVM_DEBUG(dbgs() << "\nFeatures:" << FS);
  LLVM_DEBUG(dbgs() << "\nCPU:" << CPU << "\n\n");
  InitMCProcessorInfo(CPU, FS);
  const FeatureBitset& Bits = getFeatureBits();
  if (Bits[PPC::DeprecatedDST]) DeprecatedDST = true;
  if (Bits[PPC::Directive32] && CPUDirective < PPC::DIR_32) CPUDirective = PPC::DIR_32;
  if (Bits[PPC::Directive64] && CPUDirective < PPC::DIR_64) CPUDirective = PPC::DIR_64;
  if (Bits[PPC::Directive440] && CPUDirective < PPC::DIR_440) CPUDirective = PPC::DIR_440;
  if (Bits[PPC::Directive601] && CPUDirective < PPC::DIR_601) CPUDirective = PPC::DIR_601;
  if (Bits[PPC::Directive602] && CPUDirective < PPC::DIR_602) CPUDirective = PPC::DIR_602;
  if (Bits[PPC::Directive603] && CPUDirective < PPC::DIR_603) CPUDirective = PPC::DIR_603;
  if (Bits[PPC::Directive604] && CPUDirective < PPC::DIR_603) CPUDirective = PPC::DIR_603;
  if (Bits[PPC::Directive620] && CPUDirective < PPC::DIR_603) CPUDirective = PPC::DIR_603;
  if (Bits[PPC::Directive750] && CPUDirective < PPC::DIR_750) CPUDirective = PPC::DIR_750;
  if (Bits[PPC::Directive970] && CPUDirective < PPC::DIR_970) CPUDirective = PPC::DIR_970;
  if (Bits[PPC::Directive7400] && CPUDirective < PPC::DIR_7400) CPUDirective = PPC::DIR_7400;
  if (Bits[PPC::DirectiveA2] && CPUDirective < PPC::DIR_A2) CPUDirective = PPC::DIR_A2;
  if (Bits[PPC::DirectiveE500] && CPUDirective < PPC::DIR_E500) CPUDirective = PPC::DIR_E500;
  if (Bits[PPC::DirectiveE500mc] && CPUDirective < PPC::DIR_E500mc) CPUDirective = PPC::DIR_E500mc;
  if (Bits[PPC::DirectiveE5500] && CPUDirective < PPC::DIR_E5500) CPUDirective = PPC::DIR_E5500;
  if (Bits[PPC::DirectivePwr3] && CPUDirective < PPC::DIR_PWR3) CPUDirective = PPC::DIR_PWR3;
  if (Bits[PPC::DirectivePwr4] && CPUDirective < PPC::DIR_PWR4) CPUDirective = PPC::DIR_PWR4;
  if (Bits[PPC::DirectivePwr5] && CPUDirective < PPC::DIR_PWR5) CPUDirective = PPC::DIR_PWR5;
  if (Bits[PPC::DirectivePwr5x] && CPUDirective < PPC::DIR_PWR5X) CPUDirective = PPC::DIR_PWR5X;
  if (Bits[PPC::DirectivePwr6] && CPUDirective < PPC::DIR_PWR6) CPUDirective = PPC::DIR_PWR6;
  if (Bits[PPC::DirectivePwr6x] && CPUDirective < PPC::DIR_PWR6X) CPUDirective = PPC::DIR_PWR6X;
  if (Bits[PPC::DirectivePwr7] && CPUDirective < PPC::DIR_PWR7) CPUDirective = PPC::DIR_PWR7;
  if (Bits[PPC::DirectivePwr8] && CPUDirective < PPC::DIR_PWR8) CPUDirective = PPC::DIR_PWR8;
  if (Bits[PPC::DirectivePwr9] && CPUDirective < PPC::DIR_PWR9) CPUDirective = PPC::DIR_PWR9;
  if (Bits[PPC::DirectivePwrFuture] && CPUDirective < PPC::DIR_PWR_FUTURE) CPUDirective = PPC::DIR_PWR_FUTURE;
  if (Bits[PPC::Feature64Bit]) Has64BitSupport = true;
  if (Bits[PPC::Feature64BitRegs]) Use64BitRegs = true;
  if (Bits[PPC::FeatureAltivec]) HasAltivec = true;
  if (Bits[PPC::FeatureBPERMD]) HasBPERMD = true;
  if (Bits[PPC::FeatureBookE]) IsBookE = true;
  if (Bits[PPC::FeatureCMPB]) HasCMPB = true;
  if (Bits[PPC::FeatureCRBits]) UseCRBits = true;
  if (Bits[PPC::FeatureDirectMove]) HasDirectMove = true;
  if (Bits[PPC::FeatureE500]) IsE500 = true;
  if (Bits[PPC::FeatureExtDiv]) HasExtDiv = true;
  if (Bits[PPC::FeatureFCPSGN]) HasFCPSGN = true;
  if (Bits[PPC::FeatureFPCVT]) HasFPCVT = true;
  if (Bits[PPC::FeatureFPRND]) HasFPRND = true;
  if (Bits[PPC::FeatureFPU]) HasFPU = true;
  if (Bits[PPC::FeatureFRE]) HasFRE = true;
  if (Bits[PPC::FeatureFRES]) HasFRES = true;
  if (Bits[PPC::FeatureFRSQRTE]) HasFRSQRTE = true;
  if (Bits[PPC::FeatureFRSQRTES]) HasFRSQRTES = true;
  if (Bits[PPC::FeatureFSqrt]) HasFSQRT = true;
  if (Bits[PPC::FeatureFloat128]) HasFloat128 = true;
  if (Bits[PPC::FeatureHTM]) HasHTM = true;
  if (Bits[PPC::FeatureHardFloat]) HasHardFloat = true;
  if (Bits[PPC::FeatureICBT]) HasICBT = true;
  if (Bits[PPC::FeatureISA3_0]) IsISA3_0 = true;
  if (Bits[PPC::FeatureISEL]) HasISEL = true;
  if (Bits[PPC::FeatureInvariantFunctionDescriptors]) HasInvariantFunctionDescriptors = true;
  if (Bits[PPC::FeatureLDBRX]) HasLDBRX = true;
  if (Bits[PPC::FeatureLFIWAX]) HasLFIWAX = true;
  if (Bits[PPC::FeatureLongCall]) UseLongCalls = true;
  if (Bits[PPC::FeatureMFOCRF]) HasMFOCRF = true;
  if (Bits[PPC::FeatureMFTB]) FeatureMFTB = true;
  if (Bits[PPC::FeatureMSYNC]) HasOnlyMSYNC = true;
  if (Bits[PPC::FeatureP8Altivec]) HasP8Altivec = true;
  if (Bits[PPC::FeatureP8Crypto]) HasP8Crypto = true;
  if (Bits[PPC::FeatureP8Vector]) HasP8Vector = true;
  if (Bits[PPC::FeatureP9Altivec]) HasP9Altivec = true;
  if (Bits[PPC::FeatureP9Vector]) HasP9Vector = true;
  if (Bits[PPC::FeaturePOPCNTD] && HasPOPCNTD < POPCNTD_Fast) HasPOPCNTD = POPCNTD_Fast;
  if (Bits[PPC::FeaturePPC4xx]) IsPPC4xx = true;
  if (Bits[PPC::FeaturePPC6xx]) IsPPC6xx = true;
  if (Bits[PPC::FeaturePPCPostRASched]) UsePPCPostRASchedStrategy = true;
  if (Bits[PPC::FeaturePPCPreRASched]) UsePPCPreRASchedStrategy = true;
  if (Bits[PPC::FeaturePartwordAtomic]) HasPartwordAtomics = true;
  if (Bits[PPC::FeatureQPX]) HasQPX = true;
  if (Bits[PPC::FeatureRecipPrec]) HasRecipPrec = true;
  if (Bits[PPC::FeatureSPE]) HasSPE = true;
  if (Bits[PPC::FeatureSTFIWX]) HasSTFIWX = true;
  if (Bits[PPC::FeatureSecurePlt]) SecurePlt = true;
  if (Bits[PPC::FeatureSlowPOPCNTD] && HasPOPCNTD < POPCNTD_Slow) HasPOPCNTD = POPCNTD_Slow;
  if (Bits[PPC::FeatureTwoConstNR]) NeedsTwoConstNR = true;
  if (Bits[PPC::FeatureUnalignedFloats]) AllowsUnalignedFPAccess = true;
  if (Bits[PPC::FeatureVSX]) HasVSX = true;
  if (Bits[PPC::FeatureVectorsUseTwoUnits]) VectorsUseTwoUnits = true;
}
#endif // GET_SUBTARGETINFO_TARGET_DESC


#ifdef GET_SUBTARGETINFO_HEADER
#undef GET_SUBTARGETINFO_HEADER

namespace llvm {
class DFAPacketizer;
namespace PPC_MC {
unsigned resolveVariantSchedClassImpl(unsigned SchedClass, const MCInst *MI, unsigned CPUID);
} // end namespace PPC_MC

struct PPCGenSubtargetInfo : public TargetSubtargetInfo {
  explicit PPCGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS);
public:
  unsigned resolveSchedClass(unsigned SchedClass,  const MachineInstr *DefMI, const TargetSchedModel *SchedModel) const override;
  unsigned resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, unsigned CPUID) const override;
  DFAPacketizer *createDFAPacketizer(const InstrItineraryData *IID) const;
};
} // end namespace llvm

#endif // GET_SUBTARGETINFO_HEADER


#ifdef GET_SUBTARGETINFO_CTOR
#undef GET_SUBTARGETINFO_CTOR

#include "llvm/CodeGen/TargetSchedule.h"

namespace llvm {
extern const llvm::SubtargetFeatureKV PPCFeatureKV[];
extern const llvm::SubtargetSubTypeKV PPCSubTypeKV[];
extern const llvm::MCWriteProcResEntry PPCWriteProcResTable[];
extern const llvm::MCWriteLatencyEntry PPCWriteLatencyTable[];
extern const llvm::MCReadAdvanceEntry PPCReadAdvanceTable[];
extern const llvm::InstrStage PPCStages[];
extern const unsigned PPCOperandCycles[];
extern const unsigned PPCForwardingPaths[];
PPCGenSubtargetInfo::PPCGenSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS)
  : TargetSubtargetInfo(TT, CPU, FS, makeArrayRef(PPCFeatureKV, 52), makeArrayRef(PPCSubTypeKV, 38), 
                        PPCWriteProcResTable, PPCWriteLatencyTable, PPCReadAdvanceTable, 
                        PPCStages, PPCOperandCycles, PPCForwardingPaths) {}

unsigned PPCGenSubtargetInfo
::resolveSchedClass(unsigned SchedClass, const MachineInstr *MI, const TargetSchedModel *SchedModel) const {
  report_fatal_error("Expected a variant SchedClass");
} // PPCGenSubtargetInfo::resolveSchedClass

unsigned PPCGenSubtargetInfo
::resolveVariantSchedClass(unsigned SchedClass, const MCInst *MI, unsigned CPUID) const {
  return PPC_MC::resolveVariantSchedClassImpl(SchedClass, MI, CPUID);
} // PPCGenSubtargetInfo::resolveVariantSchedClass

} // end namespace llvm

#endif // GET_SUBTARGETINFO_CTOR


#ifdef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DECLS_FOR_MC_ANALYSIS


#ifdef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS
#undef GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

#endif // GET_STIPREDICATE_DEFS_FOR_MC_ANALYSIS

