/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Target Register Enum Values                                                *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_ENUM
#undef GET_REGINFO_ENUM

namespace llvm {

class MCRegisterClass;
extern const MCRegisterClass LoongArchMCRegisterClasses[];

namespace LoongArch {
enum {
  NoRegister,
  F0 = 1,
  F1 = 2,
  F2 = 3,
  F3 = 4,
  F4 = 5,
  F5 = 6,
  F6 = 7,
  F7 = 8,
  F8 = 9,
  F9 = 10,
  F10 = 11,
  F11 = 12,
  F12 = 13,
  F13 = 14,
  F14 = 15,
  F15 = 16,
  F16 = 17,
  F17 = 18,
  F18 = 19,
  F19 = 20,
  F20 = 21,
  F21 = 22,
  F22 = 23,
  F23 = 24,
  F24 = 25,
  F25 = 26,
  F26 = 27,
  F27 = 28,
  F28 = 29,
  F29 = 30,
  F30 = 31,
  F31 = 32,
  FCC0 = 33,
  FCC1 = 34,
  FCC2 = 35,
  FCC3 = 36,
  FCC4 = 37,
  FCC5 = 38,
  FCC6 = 39,
  FCC7 = 40,
  FCSR0 = 41,
  FCSR1 = 42,
  FCSR2 = 43,
  FCSR3 = 44,
  R0 = 45,
  R1 = 46,
  R2 = 47,
  R3 = 48,
  R4 = 49,
  R5 = 50,
  R6 = 51,
  R7 = 52,
  R8 = 53,
  R9 = 54,
  R10 = 55,
  R11 = 56,
  R12 = 57,
  R13 = 58,
  R14 = 59,
  R15 = 60,
  R16 = 61,
  R17 = 62,
  R18 = 63,
  R19 = 64,
  R20 = 65,
  R21 = 66,
  R22 = 67,
  R23 = 68,
  R24 = 69,
  R25 = 70,
  R26 = 71,
  R27 = 72,
  R28 = 73,
  R29 = 74,
  R30 = 75,
  R31 = 76,
  F0_64 = 77,
  F1_64 = 78,
  F2_64 = 79,
  F3_64 = 80,
  F4_64 = 81,
  F5_64 = 82,
  F6_64 = 83,
  F7_64 = 84,
  F8_64 = 85,
  F9_64 = 86,
  F10_64 = 87,
  F11_64 = 88,
  F12_64 = 89,
  F13_64 = 90,
  F14_64 = 91,
  F15_64 = 92,
  F16_64 = 93,
  F17_64 = 94,
  F18_64 = 95,
  F19_64 = 96,
  F20_64 = 97,
  F21_64 = 98,
  F22_64 = 99,
  F23_64 = 100,
  F24_64 = 101,
  F25_64 = 102,
  F26_64 = 103,
  F27_64 = 104,
  F28_64 = 105,
  F29_64 = 106,
  F30_64 = 107,
  F31_64 = 108,
  NUM_TARGET_REGS // 109
};
} // end namespace LoongArch

// Register classes

namespace LoongArch {
enum {
  FPR32RegClassID = 0,
  GPRRegClassID = 1,
  GPRTRegClassID = 2,
  CFRRegClassID = 3,
  FCSRRegClassID = 4,
  FPR64RegClassID = 5,

};
} // end namespace LoongArch


// Register alternate name indices

namespace LoongArch {
enum {
  NoRegAltName,	// 0
  RegAliasName,	// 1
  NUM_TARGET_REG_ALT_NAMES = 2
};
} // end namespace LoongArch


// Subregister indices

namespace LoongArch {
enum : uint16_t {
  NoSubRegister,
  sub_32,	// 1
  NUM_TARGET_SUBREGS
};
} // end namespace LoongArch

// Register pressure sets enum.
namespace LoongArch {
enum RegisterPressureSets {
  CFR = 0,
  GPRT = 1,
  FPR32 = 2,
  GPR = 3,
};
} // end namespace LoongArch

} // end namespace llvm

#endif // GET_REGINFO_ENUM

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* MC Register Information                                                    *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_MC_DESC
#undef GET_REGINFO_MC_DESC

namespace llvm {

extern const MCPhysReg LoongArchRegDiffLists[] = {
  /* 0 */ 76, 0,
  /* 2 */ 65459, 0,
  /* 4 */ 65460, 0,
  /* 6 */ 65535, 0,
};

extern const LaneBitmask LoongArchLaneMaskLists[] = {
  /* 0 */ LaneBitmask(0x0000000000000000), LaneBitmask::getAll(),
  /* 2 */ LaneBitmask(0x0000000000000001), LaneBitmask::getAll(),
};

extern const uint16_t LoongArchSubRegIdxLists[] = {
  /* 0 */ 1, 0,
};

extern const MCRegisterInfo::SubRegCoveredBits LoongArchSubRegIdxRanges[] = {
  { 65535, 65535 },
  { 0, 32 },	// sub_32
};


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif
extern const char LoongArchRegStrings[] = {
  /* 0 */ "F10\0"
  /* 4 */ "R10\0"
  /* 8 */ "F20\0"
  /* 12 */ "R20\0"
  /* 16 */ "F30\0"
  /* 20 */ "R30\0"
  /* 24 */ "FCC0\0"
  /* 29 */ "F0\0"
  /* 32 */ "FCSR0\0"
  /* 38 */ "F11\0"
  /* 42 */ "R11\0"
  /* 46 */ "F21\0"
  /* 50 */ "R21\0"
  /* 54 */ "F31\0"
  /* 58 */ "R31\0"
  /* 62 */ "FCC1\0"
  /* 67 */ "F1\0"
  /* 70 */ "FCSR1\0"
  /* 76 */ "F12\0"
  /* 80 */ "R12\0"
  /* 84 */ "F22\0"
  /* 88 */ "R22\0"
  /* 92 */ "FCC2\0"
  /* 97 */ "F2\0"
  /* 100 */ "FCSR2\0"
  /* 106 */ "F13\0"
  /* 110 */ "R13\0"
  /* 114 */ "F23\0"
  /* 118 */ "R23\0"
  /* 122 */ "FCC3\0"
  /* 127 */ "F3\0"
  /* 130 */ "FCSR3\0"
  /* 136 */ "F14\0"
  /* 140 */ "R14\0"
  /* 144 */ "F24\0"
  /* 148 */ "R24\0"
  /* 152 */ "F10_64\0"
  /* 159 */ "F20_64\0"
  /* 166 */ "F30_64\0"
  /* 173 */ "F0_64\0"
  /* 179 */ "F11_64\0"
  /* 186 */ "F21_64\0"
  /* 193 */ "F31_64\0"
  /* 200 */ "F1_64\0"
  /* 206 */ "F12_64\0"
  /* 213 */ "F22_64\0"
  /* 220 */ "F2_64\0"
  /* 226 */ "F13_64\0"
  /* 233 */ "F23_64\0"
  /* 240 */ "F3_64\0"
  /* 246 */ "F14_64\0"
  /* 253 */ "F24_64\0"
  /* 260 */ "F4_64\0"
  /* 266 */ "F15_64\0"
  /* 273 */ "F25_64\0"
  /* 280 */ "F5_64\0"
  /* 286 */ "F16_64\0"
  /* 293 */ "F26_64\0"
  /* 300 */ "F6_64\0"
  /* 306 */ "F17_64\0"
  /* 313 */ "F27_64\0"
  /* 320 */ "F7_64\0"
  /* 326 */ "F18_64\0"
  /* 333 */ "F28_64\0"
  /* 340 */ "F8_64\0"
  /* 346 */ "F19_64\0"
  /* 353 */ "F29_64\0"
  /* 360 */ "F9_64\0"
  /* 366 */ "FCC4\0"
  /* 371 */ "F4\0"
  /* 374 */ "R4\0"
  /* 377 */ "F15\0"
  /* 381 */ "R15\0"
  /* 385 */ "F25\0"
  /* 389 */ "R25\0"
  /* 393 */ "FCC5\0"
  /* 398 */ "F5\0"
  /* 401 */ "R5\0"
  /* 404 */ "F16\0"
  /* 408 */ "R16\0"
  /* 412 */ "F26\0"
  /* 416 */ "R26\0"
  /* 420 */ "FCC6\0"
  /* 425 */ "F6\0"
  /* 428 */ "R6\0"
  /* 431 */ "F17\0"
  /* 435 */ "R17\0"
  /* 439 */ "F27\0"
  /* 443 */ "R27\0"
  /* 447 */ "FCC7\0"
  /* 452 */ "F7\0"
  /* 455 */ "R7\0"
  /* 458 */ "F18\0"
  /* 462 */ "R18\0"
  /* 466 */ "F28\0"
  /* 470 */ "R28\0"
  /* 474 */ "F8\0"
  /* 477 */ "R8\0"
  /* 480 */ "F19\0"
  /* 484 */ "R19\0"
  /* 488 */ "F29\0"
  /* 492 */ "R29\0"
  /* 496 */ "F9\0"
  /* 499 */ "R9\0"
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern const MCRegisterDesc LoongArchRegDesc[] = { // Descriptors
  { 3, 0, 0, 0, 0, 0 },
  { 29, 1, 0, 1, 97, 0 },
  { 67, 1, 0, 1, 97, 0 },
  { 97, 1, 0, 1, 97, 0 },
  { 127, 1, 0, 1, 97, 0 },
  { 371, 1, 0, 1, 97, 0 },
  { 398, 1, 0, 1, 97, 0 },
  { 425, 1, 0, 1, 97, 0 },
  { 452, 1, 0, 1, 97, 0 },
  { 474, 1, 0, 1, 97, 0 },
  { 496, 1, 0, 1, 97, 0 },
  { 0, 1, 0, 1, 97, 0 },
  { 38, 1, 0, 1, 97, 0 },
  { 76, 1, 0, 1, 97, 0 },
  { 106, 1, 0, 1, 97, 0 },
  { 136, 1, 0, 1, 97, 0 },
  { 377, 1, 0, 1, 97, 0 },
  { 404, 1, 0, 1, 97, 0 },
  { 431, 1, 0, 1, 97, 0 },
  { 458, 1, 0, 1, 97, 0 },
  { 480, 1, 0, 1, 97, 0 },
  { 8, 1, 0, 1, 97, 0 },
  { 46, 1, 0, 1, 97, 0 },
  { 84, 1, 0, 1, 97, 0 },
  { 114, 1, 0, 1, 97, 0 },
  { 144, 1, 0, 1, 97, 0 },
  { 385, 1, 0, 1, 97, 0 },
  { 412, 1, 0, 1, 97, 0 },
  { 439, 1, 0, 1, 97, 0 },
  { 466, 1, 0, 1, 97, 0 },
  { 488, 1, 0, 1, 97, 0 },
  { 16, 1, 0, 1, 97, 0 },
  { 54, 1, 0, 1, 97, 0 },
  { 24, 1, 1, 1, 97, 0 },
  { 62, 1, 1, 1, 97, 0 },
  { 92, 1, 1, 1, 97, 0 },
  { 122, 1, 1, 1, 97, 0 },
  { 366, 1, 1, 1, 97, 0 },
  { 393, 1, 1, 1, 97, 0 },
  { 420, 1, 1, 1, 97, 0 },
  { 447, 1, 1, 1, 97, 0 },
  { 32, 1, 1, 1, 97, 0 },
  { 70, 1, 1, 1, 97, 0 },
  { 100, 1, 1, 1, 97, 0 },
  { 130, 1, 1, 1, 97, 0 },
  { 35, 1, 1, 1, 97, 0 },
  { 73, 1, 1, 1, 97, 0 },
  { 103, 1, 1, 1, 97, 0 },
  { 133, 1, 1, 1, 97, 0 },
  { 374, 1, 1, 1, 97, 0 },
  { 401, 1, 1, 1, 97, 0 },
  { 428, 1, 1, 1, 97, 0 },
  { 455, 1, 1, 1, 97, 0 },
  { 477, 1, 1, 1, 97, 0 },
  { 499, 1, 1, 1, 97, 0 },
  { 4, 1, 1, 1, 97, 0 },
  { 42, 1, 1, 1, 97, 0 },
  { 80, 1, 1, 1, 97, 0 },
  { 110, 1, 1, 1, 97, 0 },
  { 140, 1, 1, 1, 97, 0 },
  { 381, 1, 1, 1, 97, 0 },
  { 408, 1, 1, 1, 97, 0 },
  { 435, 1, 1, 1, 97, 0 },
  { 462, 1, 1, 1, 97, 0 },
  { 484, 1, 1, 1, 97, 0 },
  { 12, 1, 1, 1, 97, 0 },
  { 50, 1, 1, 1, 97, 0 },
  { 88, 1, 1, 1, 97, 0 },
  { 118, 1, 1, 1, 97, 0 },
  { 148, 1, 1, 1, 97, 0 },
  { 389, 1, 1, 1, 97, 0 },
  { 416, 1, 1, 1, 97, 0 },
  { 443, 1, 1, 1, 97, 0 },
  { 470, 1, 1, 1, 97, 0 },
  { 492, 1, 1, 1, 97, 0 },
  { 20, 1, 1, 1, 97, 0 },
  { 58, 1, 1, 1, 97, 0 },
  { 173, 4, 1, 0, 33, 2 },
  { 200, 4, 1, 0, 33, 2 },
  { 220, 4, 1, 0, 33, 2 },
  { 240, 4, 1, 0, 33, 2 },
  { 260, 4, 1, 0, 33, 2 },
  { 280, 4, 1, 0, 33, 2 },
  { 300, 4, 1, 0, 33, 2 },
  { 320, 4, 1, 0, 33, 2 },
  { 340, 4, 1, 0, 33, 2 },
  { 360, 4, 1, 0, 33, 2 },
  { 152, 4, 1, 0, 33, 2 },
  { 179, 4, 1, 0, 33, 2 },
  { 206, 4, 1, 0, 33, 2 },
  { 226, 4, 1, 0, 33, 2 },
  { 246, 4, 1, 0, 33, 2 },
  { 266, 4, 1, 0, 33, 2 },
  { 286, 4, 1, 0, 33, 2 },
  { 306, 4, 1, 0, 33, 2 },
  { 326, 4, 1, 0, 33, 2 },
  { 346, 4, 1, 0, 33, 2 },
  { 159, 4, 1, 0, 33, 2 },
  { 186, 4, 1, 0, 33, 2 },
  { 213, 4, 1, 0, 33, 2 },
  { 233, 4, 1, 0, 33, 2 },
  { 253, 4, 1, 0, 33, 2 },
  { 273, 4, 1, 0, 33, 2 },
  { 293, 4, 1, 0, 33, 2 },
  { 313, 4, 1, 0, 33, 2 },
  { 333, 4, 1, 0, 33, 2 },
  { 353, 4, 1, 0, 33, 2 },
  { 166, 4, 1, 0, 33, 2 },
  { 193, 4, 1, 0, 33, 2 },
};

extern const MCPhysReg LoongArchRegUnitRoots[][2] = {
  { LoongArch::F0 },
  { LoongArch::F1 },
  { LoongArch::F2 },
  { LoongArch::F3 },
  { LoongArch::F4 },
  { LoongArch::F5 },
  { LoongArch::F6 },
  { LoongArch::F7 },
  { LoongArch::F8 },
  { LoongArch::F9 },
  { LoongArch::F10 },
  { LoongArch::F11 },
  { LoongArch::F12 },
  { LoongArch::F13 },
  { LoongArch::F14 },
  { LoongArch::F15 },
  { LoongArch::F16 },
  { LoongArch::F17 },
  { LoongArch::F18 },
  { LoongArch::F19 },
  { LoongArch::F20 },
  { LoongArch::F21 },
  { LoongArch::F22 },
  { LoongArch::F23 },
  { LoongArch::F24 },
  { LoongArch::F25 },
  { LoongArch::F26 },
  { LoongArch::F27 },
  { LoongArch::F28 },
  { LoongArch::F29 },
  { LoongArch::F30 },
  { LoongArch::F31 },
  { LoongArch::FCC0 },
  { LoongArch::FCC1 },
  { LoongArch::FCC2 },
  { LoongArch::FCC3 },
  { LoongArch::FCC4 },
  { LoongArch::FCC5 },
  { LoongArch::FCC6 },
  { LoongArch::FCC7 },
  { LoongArch::FCSR0 },
  { LoongArch::FCSR1 },
  { LoongArch::FCSR2 },
  { LoongArch::FCSR3 },
  { LoongArch::R0 },
  { LoongArch::R1 },
  { LoongArch::R2 },
  { LoongArch::R3 },
  { LoongArch::R4 },
  { LoongArch::R5 },
  { LoongArch::R6 },
  { LoongArch::R7 },
  { LoongArch::R8 },
  { LoongArch::R9 },
  { LoongArch::R10 },
  { LoongArch::R11 },
  { LoongArch::R12 },
  { LoongArch::R13 },
  { LoongArch::R14 },
  { LoongArch::R15 },
  { LoongArch::R16 },
  { LoongArch::R17 },
  { LoongArch::R18 },
  { LoongArch::R19 },
  { LoongArch::R20 },
  { LoongArch::R21 },
  { LoongArch::R22 },
  { LoongArch::R23 },
  { LoongArch::R24 },
  { LoongArch::R25 },
  { LoongArch::R26 },
  { LoongArch::R27 },
  { LoongArch::R28 },
  { LoongArch::R29 },
  { LoongArch::R30 },
  { LoongArch::R31 },
};

namespace {     // Register classes...
  // FPR32 Register Class...
  const MCPhysReg FPR32[] = {
    LoongArch::F0, LoongArch::F1, LoongArch::F2, LoongArch::F3, LoongArch::F4, LoongArch::F5, LoongArch::F6, LoongArch::F7, LoongArch::F8, LoongArch::F9, LoongArch::F10, LoongArch::F11, LoongArch::F12, LoongArch::F13, LoongArch::F14, LoongArch::F15, LoongArch::F16, LoongArch::F17, LoongArch::F18, LoongArch::F19, LoongArch::F20, LoongArch::F21, LoongArch::F22, LoongArch::F23, LoongArch::F24, LoongArch::F25, LoongArch::F26, LoongArch::F27, LoongArch::F28, LoongArch::F29, LoongArch::F30, LoongArch::F31, 
  };

  // FPR32 Bit set.
  const uint8_t FPR32Bits[] = {
    0xfe, 0xff, 0xff, 0xff, 0x01, 
  };

  // GPR Register Class...
  const MCPhysReg GPR[] = {
    LoongArch::R4, LoongArch::R5, LoongArch::R6, LoongArch::R7, LoongArch::R8, LoongArch::R9, LoongArch::R10, LoongArch::R11, LoongArch::R12, LoongArch::R13, LoongArch::R14, LoongArch::R15, LoongArch::R16, LoongArch::R17, LoongArch::R18, LoongArch::R19, LoongArch::R20, LoongArch::R22, LoongArch::R23, LoongArch::R24, LoongArch::R25, LoongArch::R26, LoongArch::R27, LoongArch::R28, LoongArch::R29, LoongArch::R30, LoongArch::R31, LoongArch::R0, LoongArch::R1, LoongArch::R2, LoongArch::R3, LoongArch::R21, 
  };

  // GPR Bit set.
  const uint8_t GPRBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

  // GPRT Register Class...
  const MCPhysReg GPRT[] = {
    LoongArch::R4, LoongArch::R5, LoongArch::R6, LoongArch::R7, LoongArch::R8, LoongArch::R9, LoongArch::R10, LoongArch::R11, LoongArch::R12, LoongArch::R13, LoongArch::R14, LoongArch::R15, LoongArch::R16, LoongArch::R17, LoongArch::R18, LoongArch::R19, LoongArch::R20, 
  };

  // GPRT Bit set.
  const uint8_t GPRTBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 
  };

  // CFR Register Class...
  const MCPhysReg CFR[] = {
    LoongArch::FCC0, LoongArch::FCC1, LoongArch::FCC2, LoongArch::FCC3, LoongArch::FCC4, LoongArch::FCC5, LoongArch::FCC6, LoongArch::FCC7, 
  };

  // CFR Bit set.
  const uint8_t CFRBits[] = {
    0x00, 0x00, 0x00, 0x00, 0xfe, 0x01, 
  };

  // FCSR Register Class...
  const MCPhysReg FCSR[] = {
    LoongArch::FCSR0, LoongArch::FCSR1, LoongArch::FCSR2, LoongArch::FCSR3, 
  };

  // FCSR Bit set.
  const uint8_t FCSRBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 
  };

  // FPR64 Register Class...
  const MCPhysReg FPR64[] = {
    LoongArch::F0_64, LoongArch::F1_64, LoongArch::F2_64, LoongArch::F3_64, LoongArch::F4_64, LoongArch::F5_64, LoongArch::F6_64, LoongArch::F7_64, LoongArch::F8_64, LoongArch::F9_64, LoongArch::F10_64, LoongArch::F11_64, LoongArch::F12_64, LoongArch::F13_64, LoongArch::F14_64, LoongArch::F15_64, LoongArch::F16_64, LoongArch::F17_64, LoongArch::F18_64, LoongArch::F19_64, LoongArch::F20_64, LoongArch::F21_64, LoongArch::F22_64, LoongArch::F23_64, LoongArch::F24_64, LoongArch::F25_64, LoongArch::F26_64, LoongArch::F27_64, LoongArch::F28_64, LoongArch::F29_64, LoongArch::F30_64, LoongArch::F31_64, 
  };

  // FPR64 Bit set.
  const uint8_t FPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x1f, 
  };

} // end anonymous namespace


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif
extern const char LoongArchRegClassStrings[] = {
  /* 0 */ "FPR32\0"
  /* 6 */ "FPR64\0"
  /* 12 */ "CFR\0"
  /* 16 */ "GPR\0"
  /* 20 */ "FCSR\0"
  /* 25 */ "GPRT\0"
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern const MCRegisterClass LoongArchMCRegisterClasses[] = {
  { FPR32, FPR32Bits, 0, 32, sizeof(FPR32Bits), LoongArch::FPR32RegClassID, 32, 1, true },
  { GPR, GPRBits, 16, 32, sizeof(GPRBits), LoongArch::GPRRegClassID, 0, 1, true },
  { GPRT, GPRTBits, 25, 17, sizeof(GPRTBits), LoongArch::GPRTRegClassID, 0, 1, true },
  { CFR, CFRBits, 12, 8, sizeof(CFRBits), LoongArch::CFRRegClassID, 0, 1, true },
  { FCSR, FCSRBits, 20, 4, sizeof(FCSRBits), LoongArch::FCSRRegClassID, 32, 1, false },
  { FPR64, FPR64Bits, 6, 32, sizeof(FPR64Bits), LoongArch::FPR64RegClassID, 64, 1, true },
};

// LoongArch Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchDwarfFlavour0Dwarf2L[] = {
  { 0U, LoongArch::R0 },
  { 1U, LoongArch::R1 },
  { 2U, LoongArch::R2 },
  { 3U, LoongArch::R3 },
  { 4U, LoongArch::R4 },
  { 5U, LoongArch::R5 },
  { 6U, LoongArch::R6 },
  { 7U, LoongArch::R7 },
  { 8U, LoongArch::R8 },
  { 9U, LoongArch::R9 },
  { 10U, LoongArch::R10 },
  { 11U, LoongArch::R11 },
  { 12U, LoongArch::R12 },
  { 13U, LoongArch::R13 },
  { 14U, LoongArch::R14 },
  { 15U, LoongArch::R15 },
  { 16U, LoongArch::R16 },
  { 17U, LoongArch::R17 },
  { 18U, LoongArch::R18 },
  { 19U, LoongArch::R19 },
  { 20U, LoongArch::R20 },
  { 21U, LoongArch::R21 },
  { 22U, LoongArch::R22 },
  { 23U, LoongArch::R23 },
  { 24U, LoongArch::R24 },
  { 25U, LoongArch::R25 },
  { 26U, LoongArch::R26 },
  { 27U, LoongArch::R27 },
  { 28U, LoongArch::R28 },
  { 29U, LoongArch::R29 },
  { 30U, LoongArch::R30 },
  { 31U, LoongArch::R31 },
  { 32U, LoongArch::F0_64 },
  { 33U, LoongArch::F1_64 },
  { 34U, LoongArch::F2_64 },
  { 35U, LoongArch::F3_64 },
  { 36U, LoongArch::F4_64 },
  { 37U, LoongArch::F5_64 },
  { 38U, LoongArch::F6_64 },
  { 39U, LoongArch::F7_64 },
  { 40U, LoongArch::F8_64 },
  { 41U, LoongArch::F9_64 },
  { 42U, LoongArch::F10_64 },
  { 43U, LoongArch::F11_64 },
  { 44U, LoongArch::F12_64 },
  { 45U, LoongArch::F13_64 },
  { 46U, LoongArch::F14_64 },
  { 47U, LoongArch::F15_64 },
  { 48U, LoongArch::F16_64 },
  { 49U, LoongArch::F17_64 },
  { 50U, LoongArch::F18_64 },
  { 51U, LoongArch::F19_64 },
  { 52U, LoongArch::F20_64 },
  { 53U, LoongArch::F21_64 },
  { 54U, LoongArch::F22_64 },
  { 55U, LoongArch::F23_64 },
  { 56U, LoongArch::F24_64 },
  { 57U, LoongArch::F25_64 },
  { 58U, LoongArch::F26_64 },
  { 59U, LoongArch::F27_64 },
  { 60U, LoongArch::F28_64 },
  { 61U, LoongArch::F29_64 },
  { 62U, LoongArch::F30_64 },
  { 63U, LoongArch::F31_64 },
};
extern const unsigned LoongArchDwarfFlavour0Dwarf2LSize = std::size(LoongArchDwarfFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchEHFlavour0Dwarf2L[] = {
  { 0U, LoongArch::R0 },
  { 1U, LoongArch::R1 },
  { 2U, LoongArch::R2 },
  { 3U, LoongArch::R3 },
  { 4U, LoongArch::R4 },
  { 5U, LoongArch::R5 },
  { 6U, LoongArch::R6 },
  { 7U, LoongArch::R7 },
  { 8U, LoongArch::R8 },
  { 9U, LoongArch::R9 },
  { 10U, LoongArch::R10 },
  { 11U, LoongArch::R11 },
  { 12U, LoongArch::R12 },
  { 13U, LoongArch::R13 },
  { 14U, LoongArch::R14 },
  { 15U, LoongArch::R15 },
  { 16U, LoongArch::R16 },
  { 17U, LoongArch::R17 },
  { 18U, LoongArch::R18 },
  { 19U, LoongArch::R19 },
  { 20U, LoongArch::R20 },
  { 21U, LoongArch::R21 },
  { 22U, LoongArch::R22 },
  { 23U, LoongArch::R23 },
  { 24U, LoongArch::R24 },
  { 25U, LoongArch::R25 },
  { 26U, LoongArch::R26 },
  { 27U, LoongArch::R27 },
  { 28U, LoongArch::R28 },
  { 29U, LoongArch::R29 },
  { 30U, LoongArch::R30 },
  { 31U, LoongArch::R31 },
  { 32U, LoongArch::F0_64 },
  { 33U, LoongArch::F1_64 },
  { 34U, LoongArch::F2_64 },
  { 35U, LoongArch::F3_64 },
  { 36U, LoongArch::F4_64 },
  { 37U, LoongArch::F5_64 },
  { 38U, LoongArch::F6_64 },
  { 39U, LoongArch::F7_64 },
  { 40U, LoongArch::F8_64 },
  { 41U, LoongArch::F9_64 },
  { 42U, LoongArch::F10_64 },
  { 43U, LoongArch::F11_64 },
  { 44U, LoongArch::F12_64 },
  { 45U, LoongArch::F13_64 },
  { 46U, LoongArch::F14_64 },
  { 47U, LoongArch::F15_64 },
  { 48U, LoongArch::F16_64 },
  { 49U, LoongArch::F17_64 },
  { 50U, LoongArch::F18_64 },
  { 51U, LoongArch::F19_64 },
  { 52U, LoongArch::F20_64 },
  { 53U, LoongArch::F21_64 },
  { 54U, LoongArch::F22_64 },
  { 55U, LoongArch::F23_64 },
  { 56U, LoongArch::F24_64 },
  { 57U, LoongArch::F25_64 },
  { 58U, LoongArch::F26_64 },
  { 59U, LoongArch::F27_64 },
  { 60U, LoongArch::F28_64 },
  { 61U, LoongArch::F29_64 },
  { 62U, LoongArch::F30_64 },
  { 63U, LoongArch::F31_64 },
};
extern const unsigned LoongArchEHFlavour0Dwarf2LSize = std::size(LoongArchEHFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchDwarfFlavour0L2Dwarf[] = {
  { LoongArch::F0, 32U },
  { LoongArch::F1, 33U },
  { LoongArch::F2, 34U },
  { LoongArch::F3, 35U },
  { LoongArch::F4, 36U },
  { LoongArch::F5, 37U },
  { LoongArch::F6, 38U },
  { LoongArch::F7, 39U },
  { LoongArch::F8, 40U },
  { LoongArch::F9, 41U },
  { LoongArch::F10, 42U },
  { LoongArch::F11, 43U },
  { LoongArch::F12, 44U },
  { LoongArch::F13, 45U },
  { LoongArch::F14, 46U },
  { LoongArch::F15, 47U },
  { LoongArch::F16, 48U },
  { LoongArch::F17, 49U },
  { LoongArch::F18, 50U },
  { LoongArch::F19, 51U },
  { LoongArch::F20, 52U },
  { LoongArch::F21, 53U },
  { LoongArch::F22, 54U },
  { LoongArch::F23, 55U },
  { LoongArch::F24, 56U },
  { LoongArch::F25, 57U },
  { LoongArch::F26, 58U },
  { LoongArch::F27, 59U },
  { LoongArch::F28, 60U },
  { LoongArch::F29, 61U },
  { LoongArch::F30, 62U },
  { LoongArch::F31, 63U },
  { LoongArch::R0, 0U },
  { LoongArch::R1, 1U },
  { LoongArch::R2, 2U },
  { LoongArch::R3, 3U },
  { LoongArch::R4, 4U },
  { LoongArch::R5, 5U },
  { LoongArch::R6, 6U },
  { LoongArch::R7, 7U },
  { LoongArch::R8, 8U },
  { LoongArch::R9, 9U },
  { LoongArch::R10, 10U },
  { LoongArch::R11, 11U },
  { LoongArch::R12, 12U },
  { LoongArch::R13, 13U },
  { LoongArch::R14, 14U },
  { LoongArch::R15, 15U },
  { LoongArch::R16, 16U },
  { LoongArch::R17, 17U },
  { LoongArch::R18, 18U },
  { LoongArch::R19, 19U },
  { LoongArch::R20, 20U },
  { LoongArch::R21, 21U },
  { LoongArch::R22, 22U },
  { LoongArch::R23, 23U },
  { LoongArch::R24, 24U },
  { LoongArch::R25, 25U },
  { LoongArch::R26, 26U },
  { LoongArch::R27, 27U },
  { LoongArch::R28, 28U },
  { LoongArch::R29, 29U },
  { LoongArch::R30, 30U },
  { LoongArch::R31, 31U },
  { LoongArch::F0_64, 32U },
  { LoongArch::F1_64, 33U },
  { LoongArch::F2_64, 34U },
  { LoongArch::F3_64, 35U },
  { LoongArch::F4_64, 36U },
  { LoongArch::F5_64, 37U },
  { LoongArch::F6_64, 38U },
  { LoongArch::F7_64, 39U },
  { LoongArch::F8_64, 40U },
  { LoongArch::F9_64, 41U },
  { LoongArch::F10_64, 42U },
  { LoongArch::F11_64, 43U },
  { LoongArch::F12_64, 44U },
  { LoongArch::F13_64, 45U },
  { LoongArch::F14_64, 46U },
  { LoongArch::F15_64, 47U },
  { LoongArch::F16_64, 48U },
  { LoongArch::F17_64, 49U },
  { LoongArch::F18_64, 50U },
  { LoongArch::F19_64, 51U },
  { LoongArch::F20_64, 52U },
  { LoongArch::F21_64, 53U },
  { LoongArch::F22_64, 54U },
  { LoongArch::F23_64, 55U },
  { LoongArch::F24_64, 56U },
  { LoongArch::F25_64, 57U },
  { LoongArch::F26_64, 58U },
  { LoongArch::F27_64, 59U },
  { LoongArch::F28_64, 60U },
  { LoongArch::F29_64, 61U },
  { LoongArch::F30_64, 62U },
  { LoongArch::F31_64, 63U },
};
extern const unsigned LoongArchDwarfFlavour0L2DwarfSize = std::size(LoongArchDwarfFlavour0L2Dwarf);

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchEHFlavour0L2Dwarf[] = {
  { LoongArch::F0, 32U },
  { LoongArch::F1, 33U },
  { LoongArch::F2, 34U },
  { LoongArch::F3, 35U },
  { LoongArch::F4, 36U },
  { LoongArch::F5, 37U },
  { LoongArch::F6, 38U },
  { LoongArch::F7, 39U },
  { LoongArch::F8, 40U },
  { LoongArch::F9, 41U },
  { LoongArch::F10, 42U },
  { LoongArch::F11, 43U },
  { LoongArch::F12, 44U },
  { LoongArch::F13, 45U },
  { LoongArch::F14, 46U },
  { LoongArch::F15, 47U },
  { LoongArch::F16, 48U },
  { LoongArch::F17, 49U },
  { LoongArch::F18, 50U },
  { LoongArch::F19, 51U },
  { LoongArch::F20, 52U },
  { LoongArch::F21, 53U },
  { LoongArch::F22, 54U },
  { LoongArch::F23, 55U },
  { LoongArch::F24, 56U },
  { LoongArch::F25, 57U },
  { LoongArch::F26, 58U },
  { LoongArch::F27, 59U },
  { LoongArch::F28, 60U },
  { LoongArch::F29, 61U },
  { LoongArch::F30, 62U },
  { LoongArch::F31, 63U },
  { LoongArch::R0, 0U },
  { LoongArch::R1, 1U },
  { LoongArch::R2, 2U },
  { LoongArch::R3, 3U },
  { LoongArch::R4, 4U },
  { LoongArch::R5, 5U },
  { LoongArch::R6, 6U },
  { LoongArch::R7, 7U },
  { LoongArch::R8, 8U },
  { LoongArch::R9, 9U },
  { LoongArch::R10, 10U },
  { LoongArch::R11, 11U },
  { LoongArch::R12, 12U },
  { LoongArch::R13, 13U },
  { LoongArch::R14, 14U },
  { LoongArch::R15, 15U },
  { LoongArch::R16, 16U },
  { LoongArch::R17, 17U },
  { LoongArch::R18, 18U },
  { LoongArch::R19, 19U },
  { LoongArch::R20, 20U },
  { LoongArch::R21, 21U },
  { LoongArch::R22, 22U },
  { LoongArch::R23, 23U },
  { LoongArch::R24, 24U },
  { LoongArch::R25, 25U },
  { LoongArch::R26, 26U },
  { LoongArch::R27, 27U },
  { LoongArch::R28, 28U },
  { LoongArch::R29, 29U },
  { LoongArch::R30, 30U },
  { LoongArch::R31, 31U },
  { LoongArch::F0_64, 32U },
  { LoongArch::F1_64, 33U },
  { LoongArch::F2_64, 34U },
  { LoongArch::F3_64, 35U },
  { LoongArch::F4_64, 36U },
  { LoongArch::F5_64, 37U },
  { LoongArch::F6_64, 38U },
  { LoongArch::F7_64, 39U },
  { LoongArch::F8_64, 40U },
  { LoongArch::F9_64, 41U },
  { LoongArch::F10_64, 42U },
  { LoongArch::F11_64, 43U },
  { LoongArch::F12_64, 44U },
  { LoongArch::F13_64, 45U },
  { LoongArch::F14_64, 46U },
  { LoongArch::F15_64, 47U },
  { LoongArch::F16_64, 48U },
  { LoongArch::F17_64, 49U },
  { LoongArch::F18_64, 50U },
  { LoongArch::F19_64, 51U },
  { LoongArch::F20_64, 52U },
  { LoongArch::F21_64, 53U },
  { LoongArch::F22_64, 54U },
  { LoongArch::F23_64, 55U },
  { LoongArch::F24_64, 56U },
  { LoongArch::F25_64, 57U },
  { LoongArch::F26_64, 58U },
  { LoongArch::F27_64, 59U },
  { LoongArch::F28_64, 60U },
  { LoongArch::F29_64, 61U },
  { LoongArch::F30_64, 62U },
  { LoongArch::F31_64, 63U },
};
extern const unsigned LoongArchEHFlavour0L2DwarfSize = std::size(LoongArchEHFlavour0L2Dwarf);

extern const uint16_t LoongArchRegEncodingTable[] = {
  0,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  0,
  1,
  2,
  3,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
  0,
  1,
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
  12,
  13,
  14,
  15,
  16,
  17,
  18,
  19,
  20,
  21,
  22,
  23,
  24,
  25,
  26,
  27,
  28,
  29,
  30,
  31,
};
static inline void InitLoongArchMCRegisterInfo(MCRegisterInfo *RI, unsigned RA, unsigned DwarfFlavour = 0, unsigned EHFlavour = 0, unsigned PC = 0) {
  RI->InitMCRegisterInfo(LoongArchRegDesc, 109, RA, PC, LoongArchMCRegisterClasses, 6, LoongArchRegUnitRoots, 76, LoongArchRegDiffLists, LoongArchLaneMaskLists, LoongArchRegStrings, LoongArchRegClassStrings, LoongArchSubRegIdxLists, 2,
LoongArchSubRegIdxRanges, LoongArchRegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(LoongArchDwarfFlavour0Dwarf2L, LoongArchDwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(LoongArchEHFlavour0Dwarf2L, LoongArchEHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(LoongArchDwarfFlavour0L2Dwarf, LoongArchDwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(LoongArchEHFlavour0L2Dwarf, LoongArchEHFlavour0L2DwarfSize, true);
    break;
  }
}

} // end namespace llvm

#endif // GET_REGINFO_MC_DESC

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Register Information Header Fragment                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_HEADER
#undef GET_REGINFO_HEADER

#include "llvm/CodeGen/TargetRegisterInfo.h"

namespace llvm {

class LoongArchFrameLowering;

struct LoongArchGenRegisterInfo : public TargetRegisterInfo {
  explicit LoongArchGenRegisterInfo(unsigned RA, unsigned D = 0, unsigned E = 0,
      unsigned PC = 0, unsigned HwMode = 0);
  unsigned composeSubRegIndicesImpl(unsigned, unsigned) const override;
  LaneBitmask composeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  LaneBitmask reverseComposeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  const TargetRegisterClass *getSubClassWithSubReg(const TargetRegisterClass *, unsigned) const override;
  const TargetRegisterClass *getSubRegisterClass(const TargetRegisterClass *, unsigned) const override;
  const RegClassWeight &getRegClassWeight(const TargetRegisterClass *RC) const override;
  unsigned getRegUnitWeight(unsigned RegUnit) const override;
  unsigned getNumRegPressureSets() const override;
  const char *getRegPressureSetName(unsigned Idx) const override;
  unsigned getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const override;
  const int *getRegClassPressureSets(const TargetRegisterClass *RC) const override;
  const int *getRegUnitPressureSets(unsigned RegUnit) const override;
  ArrayRef<const char *> getRegMaskNames() const override;
  ArrayRef<const uint32_t *> getRegMasks() const override;
  bool isGeneralPurposeRegister(const MachineFunction &, MCRegister) const override;
  bool isFixedRegister(const MachineFunction &, MCRegister) const override;
  bool isArgumentRegister(const MachineFunction &, MCRegister) const override;
  bool isConstantPhysReg(MCRegister PhysReg) const override final;
  /// Devirtualized TargetFrameLowering.
  static const LoongArchFrameLowering *getFrameLowering(
      const MachineFunction &MF);
};

namespace LoongArch { // Register classes
  extern const TargetRegisterClass FPR32RegClass;
  extern const TargetRegisterClass GPRRegClass;
  extern const TargetRegisterClass GPRTRegClass;
  extern const TargetRegisterClass CFRRegClass;
  extern const TargetRegisterClass FCSRRegClass;
  extern const TargetRegisterClass FPR64RegClass;
} // end namespace LoongArch

} // end namespace llvm

#endif // GET_REGINFO_HEADER

/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Target Register and Register Classes Information                           *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_REGINFO_TARGET_DESC
#undef GET_REGINFO_TARGET_DESC

namespace llvm {

extern const MCRegisterClass LoongArchMCRegisterClasses[];

static const MVT::SimpleValueType VTLists[] = {
  /* 0 */ MVT::i32, MVT::Other,
  /* 2 */ MVT::i64, MVT::Other,
  /* 4 */ MVT::f32, MVT::Other,
  /* 6 */ MVT::f64, MVT::Other,
};

static const char *SubRegIndexNameTable[] = { "sub_32", "" };


static const LaneBitmask SubRegIndexLaneMaskTable[] = {
  LaneBitmask::getAll(),
  LaneBitmask(0x0000000000000001), // sub_32
 };



static const TargetRegisterInfo::RegClassInfo RegClassInfos[] = {
  // Mode = 0 (Default)
  { 32, 32, 32, VTLists+4 },    // FPR32
  { 32, 32, 32, VTLists+0 },    // GPR
  { 32, 32, 32, VTLists+0 },    // GPRT
  { 32, 32, 32, VTLists+0 },    // CFR
  { 32, 32, 32, VTLists+0 },    // FCSR
  { 64, 64, 64, VTLists+6 },    // FPR64
  // Mode = 1 (LA64)
  { 32, 32, 32, VTLists+4 },    // FPR32
  { 64, 64, 64, VTLists+2 },    // GPR
  { 64, 64, 64, VTLists+2 },    // GPRT
  { 64, 64, 64, VTLists+2 },    // CFR
  { 32, 32, 32, VTLists+0 },    // FCSR
  { 64, 64, 64, VTLists+6 },    // FPR64
};

static const TargetRegisterClass *const NullRegClasses[] = { nullptr };

static const uint32_t FPR32SubClassMask[] = {
  0x00000001, 
  0x00000020, // sub_32
};

static const uint32_t GPRSubClassMask[] = {
  0x00000006, 
};

static const uint32_t GPRTSubClassMask[] = {
  0x00000004, 
};

static const uint32_t CFRSubClassMask[] = {
  0x00000008, 
};

static const uint32_t FCSRSubClassMask[] = {
  0x00000010, 
};

static const uint32_t FPR64SubClassMask[] = {
  0x00000020, 
};

static const uint16_t SuperRegIdxSeqs[] = {
  /* 0 */ 1, 0,
};

static const TargetRegisterClass *const GPRTSuperclasses[] = {
  &LoongArch::GPRRegClass,
  nullptr
};


namespace LoongArch {   // Register class instances
  extern const TargetRegisterClass FPR32RegClass = {
    &LoongArchMCRegisterClasses[FPR32RegClassID],
    FPR32SubClassMask,
    SuperRegIdxSeqs + 0,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPRRegClass = {
    &LoongArchMCRegisterClasses[GPRRegClassID],
    GPRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPRTRegClass = {
    &LoongArchMCRegisterClasses[GPRTRegClassID],
    GPRTSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRTSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass CFRRegClass = {
    &LoongArchMCRegisterClasses[CFRRegClassID],
    CFRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FCSRRegClass = {
    &LoongArchMCRegisterClasses[FCSRRegClassID],
    FCSRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR64RegClass = {
    &LoongArchMCRegisterClasses[FPR64RegClassID],
    FPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x0000000000000001),
    0,
    false,
    0x00, /* TSFlags */
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

} // end namespace LoongArch

namespace {
  const TargetRegisterClass *const RegisterClasses[] = {
    &LoongArch::FPR32RegClass,
    &LoongArch::GPRRegClass,
    &LoongArch::GPRTRegClass,
    &LoongArch::CFRRegClass,
    &LoongArch::FCSRRegClass,
    &LoongArch::FPR64RegClass,
  };
} // end anonymous namespace

static const uint8_t CostPerUseTable[] = { 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };


static const bool InAllocatableClassTable[] = { 
false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, };


static const TargetRegisterInfoDesc LoongArchRegInfoDesc = { // Extra Descriptors
CostPerUseTable, 1, InAllocatableClassTable};

unsigned LoongArchGenRegisterInfo::composeSubRegIndicesImpl(unsigned IdxA, unsigned IdxB) const {
  static const uint8_t Rows[1][1] = {
    { 0, },
  };

  --IdxA; assert(IdxA < 1); (void) IdxA;
  --IdxB; assert(IdxB < 1);
  return Rows[0][IdxB];
}

  struct MaskRolOp {
    LaneBitmask Mask;
    uint8_t  RotateLeft;
  };
  static const MaskRolOp LaneMaskComposeSequences[] = {
    { LaneBitmask(0xFFFFFFFFFFFFFFFF),  0 }, { LaneBitmask::getNone(), 0 }  // Sequence 0
  };
  static const uint8_t CompositeSequences[] = {
    0 // to sub_32
  };

LaneBitmask LoongArchGenRegisterInfo::composeSubRegIndexLaneMaskImpl(unsigned IdxA, LaneBitmask LaneMask) const {
  --IdxA; assert(IdxA < 1 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops =
       &LaneMaskComposeSequences[CompositeSequences[IdxA]];
       Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger() & Ops->Mask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M << S) | (M >> (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

LaneBitmask LoongArchGenRegisterInfo::reverseComposeSubRegIndexLaneMaskImpl(unsigned IdxA,  LaneBitmask LaneMask) const {
  LaneMask &= getSubRegIndexLaneMask(IdxA);
  --IdxA; assert(IdxA < 1 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops =
       &LaneMaskComposeSequences[CompositeSequences[IdxA]];
       Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M >> S) | (M << (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

const TargetRegisterClass *LoongArchGenRegisterInfo::getSubClassWithSubReg(const TargetRegisterClass *RC, unsigned Idx) const {
  static const uint8_t Table[6][1] = {
    {	// FPR32
      0,	// sub_32
    },
    {	// GPR
      0,	// sub_32
    },
    {	// GPRT
      0,	// sub_32
    },
    {	// CFR
      0,	// sub_32
    },
    {	// FCSR
      0,	// sub_32
    },
    {	// FPR64
      6,	// sub_32 -> FPR64
    },
  };
  assert(RC && "Missing regclass");
  if (!Idx) return RC;
  --Idx;
  assert(Idx < 1 && "Bad subreg");
  unsigned TV = Table[RC->getID()][Idx];
  return TV ? getRegClass(TV - 1) : nullptr;
}

const TargetRegisterClass *LoongArchGenRegisterInfo::getSubRegisterClass(const TargetRegisterClass *RC, unsigned Idx) const {
  static const uint8_t Table[6][1] = {
    {	// FPR32
      0,	// FPR32:sub_32
    },
    {	// GPR
      0,	// GPR:sub_32
    },
    {	// GPRT
      0,	// GPRT:sub_32
    },
    {	// CFR
      0,	// CFR:sub_32
    },
    {	// FCSR
      0,	// FCSR:sub_32
    },
    {	// FPR64
      1,	// FPR64:sub_32 -> FPR32
    },
  };
  assert(RC && "Missing regclass");
  if (!Idx) return RC;
  --Idx;
  assert(Idx < 1 && "Bad subreg");
  unsigned TV = Table[RC->getID()][Idx];
  return TV ? getRegClass(TV - 1) : nullptr;
}

/// Get the weight in units of pressure for this register class.
const RegClassWeight &LoongArchGenRegisterInfo::
getRegClassWeight(const TargetRegisterClass *RC) const {
  static const RegClassWeight RCWeightTable[] = {
    {1, 32},  	// FPR32
    {1, 32},  	// GPR
    {1, 17},  	// GPRT
    {1, 8},  	// CFR
    {0, 0},  	// FCSR
    {1, 32},  	// FPR64
  };
  return RCWeightTable[RC->getID()];
}

/// Get the weight in units of pressure for this register unit.
unsigned LoongArchGenRegisterInfo::
getRegUnitWeight(unsigned RegUnit) const {
  assert(RegUnit < 76 && "invalid register unit");
  // All register units have unit weight.
  return 1;
}


// Get the number of dimensions of register pressure.
unsigned LoongArchGenRegisterInfo::getNumRegPressureSets() const {
  return 4;
}

// Get the name of this register unit pressure set.
const char *LoongArchGenRegisterInfo::
getRegPressureSetName(unsigned Idx) const {
  static const char *PressureNameTable[] = {
    "CFR",
    "GPRT",
    "FPR32",
    "GPR",
  };
  return PressureNameTable[Idx];
}

// Get the register unit pressure limit for this dimension.
// This limit must be adjusted dynamically for reserved registers.
unsigned LoongArchGenRegisterInfo::
getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const {
  static const uint8_t PressureLimitTable[] = {
    8,  	// 0: CFR
    17,  	// 1: GPRT
    32,  	// 2: FPR32
    32,  	// 3: GPR
  };
  return PressureLimitTable[Idx];
}

/// Table of pressure sets per register class or unit.
static const int RCSetsTable[] = {
  /* 0 */ 0, -1,
  /* 2 */ 2, -1,
  /* 4 */ 1, 3, -1,
};

/// Get the dimensions of register pressure impacted by this register class.
/// Returns a -1 terminated array of pressure set IDs
const int *LoongArchGenRegisterInfo::
getRegClassPressureSets(const TargetRegisterClass *RC) const {
  static const uint8_t RCSetStartTable[] = {
    2,5,4,0,1,2,};
  return &RCSetsTable[RCSetStartTable[RC->getID()]];
}

/// Get the dimensions of register pressure impacted by this register unit.
/// Returns a -1 terminated array of pressure set IDs
const int *LoongArchGenRegisterInfo::
getRegUnitPressureSets(unsigned RegUnit) const {
  assert(RegUnit < 76 && "invalid register unit");
  static const uint8_t RUSetStartTable[] = {
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,1,1,1,1,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,};
  return &RCSetsTable[RUSetStartTable[RegUnit]];
}

extern const MCRegisterDesc LoongArchRegDesc[];
extern const MCPhysReg LoongArchRegDiffLists[];
extern const LaneBitmask LoongArchLaneMaskLists[];
extern const char LoongArchRegStrings[];
extern const char LoongArchRegClassStrings[];
extern const MCPhysReg LoongArchRegUnitRoots[][2];
extern const uint16_t LoongArchSubRegIdxLists[];
extern const MCRegisterInfo::SubRegCoveredBits LoongArchSubRegIdxRanges[];
extern const uint16_t LoongArchRegEncodingTable[];
// LoongArch Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchDwarfFlavour0Dwarf2L[];
extern const unsigned LoongArchDwarfFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchEHFlavour0Dwarf2L[];
extern const unsigned LoongArchEHFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchDwarfFlavour0L2Dwarf[];
extern const unsigned LoongArchDwarfFlavour0L2DwarfSize;

extern const MCRegisterInfo::DwarfLLVMRegPair LoongArchEHFlavour0L2Dwarf[];
extern const unsigned LoongArchEHFlavour0L2DwarfSize;

LoongArchGenRegisterInfo::
LoongArchGenRegisterInfo(unsigned RA, unsigned DwarfFlavour, unsigned EHFlavour,
      unsigned PC, unsigned HwMode)
  : TargetRegisterInfo(&LoongArchRegInfoDesc, RegisterClasses, RegisterClasses+6,
             SubRegIndexNameTable, SubRegIndexLaneMaskTable,
             LaneBitmask(0xFFFFFFFFFFFFFFFE), RegClassInfos, HwMode) {
  InitMCRegisterInfo(LoongArchRegDesc, 109, RA, PC,
                     LoongArchMCRegisterClasses, 6,
                     LoongArchRegUnitRoots,
                     76,
                     LoongArchRegDiffLists,
                     LoongArchLaneMaskLists,
                     LoongArchRegStrings,
                     LoongArchRegClassStrings,
                     LoongArchSubRegIdxLists,
                     2,
                     LoongArchSubRegIdxRanges,
                     LoongArchRegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(LoongArchDwarfFlavour0Dwarf2L, LoongArchDwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(LoongArchEHFlavour0Dwarf2L, LoongArchEHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(LoongArchDwarfFlavour0L2Dwarf, LoongArchDwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(LoongArchEHFlavour0L2Dwarf, LoongArchEHFlavour0L2DwarfSize, true);
    break;
  }
}

static const MCPhysReg CSR_ILP32D_LP64D_SaveList[] = { LoongArch::R1, LoongArch::R22, LoongArch::R23, LoongArch::R24, LoongArch::R25, LoongArch::R26, LoongArch::R27, LoongArch::R28, LoongArch::R29, LoongArch::R30, LoongArch::R31, LoongArch::F24_64, LoongArch::F25_64, LoongArch::F26_64, LoongArch::F27_64, LoongArch::F28_64, LoongArch::F29_64, LoongArch::F30_64, LoongArch::F31_64, 0 };
static const uint32_t CSR_ILP32D_LP64D_RegMask[] = { 0xfe000000, 0x00006001, 0x00001ff8, 0x00001fe0, };
static const MCPhysReg CSR_ILP32F_LP64F_SaveList[] = { LoongArch::R1, LoongArch::R22, LoongArch::R23, LoongArch::R24, LoongArch::R25, LoongArch::R26, LoongArch::R27, LoongArch::R28, LoongArch::R29, LoongArch::R30, LoongArch::R31, LoongArch::F24, LoongArch::F25, LoongArch::F26, LoongArch::F27, LoongArch::F28, LoongArch::F29, LoongArch::F30, LoongArch::F31, 0 };
static const uint32_t CSR_ILP32F_LP64F_RegMask[] = { 0xfe000000, 0x00006001, 0x00001ff8, 0x00000000, };
static const MCPhysReg CSR_ILP32S_LP64S_SaveList[] = { LoongArch::R1, LoongArch::R22, LoongArch::R23, LoongArch::R24, LoongArch::R25, LoongArch::R26, LoongArch::R27, LoongArch::R28, LoongArch::R29, LoongArch::R30, LoongArch::R31, 0 };
static const uint32_t CSR_ILP32S_LP64S_RegMask[] = { 0x00000000, 0x00006000, 0x00001ff8, 0x00000000, };
static const MCPhysReg CSR_NoRegs_SaveList[] = { 0 };
static const uint32_t CSR_NoRegs_RegMask[] = { 0x00000000, 0x00002000, 0x00000000, 0x00000000, };


ArrayRef<const uint32_t *> LoongArchGenRegisterInfo::getRegMasks() const {
  static const uint32_t *const Masks[] = {
    CSR_ILP32D_LP64D_RegMask,
    CSR_ILP32F_LP64F_RegMask,
    CSR_ILP32S_LP64S_RegMask,
    CSR_NoRegs_RegMask,
  };
  return ArrayRef(Masks);
}

bool LoongArchGenRegisterInfo::
isGeneralPurposeRegister(const MachineFunction &MF, MCRegister PhysReg) const {
  return
      false;
}

bool LoongArchGenRegisterInfo::
isFixedRegister(const MachineFunction &MF, MCRegister PhysReg) const {
  return
      false;
}

bool LoongArchGenRegisterInfo::
isArgumentRegister(const MachineFunction &MF, MCRegister PhysReg) const {
  return
      false;
}

bool LoongArchGenRegisterInfo::
isConstantPhysReg(MCRegister PhysReg) const {
  return
      PhysReg == LoongArch::R0 ||
      false;
}

ArrayRef<const char *> LoongArchGenRegisterInfo::getRegMaskNames() const {
  static const char *Names[] = {
    "CSR_ILP32D_LP64D",
    "CSR_ILP32F_LP64F",
    "CSR_ILP32S_LP64S",
    "CSR_NoRegs",
  };
  return ArrayRef(Names);
}

const LoongArchFrameLowering *
LoongArchGenRegisterInfo::getFrameLowering(const MachineFunction &MF) {
  return static_cast<const LoongArchFrameLowering *>(
      MF.getSubtarget().getFrameLowering());
}

} // end namespace llvm

#endif // GET_REGINFO_TARGET_DESC

