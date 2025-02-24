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
extern const MCRegisterClass RISCVMCRegisterClasses[];

namespace RISCV {
enum {
  NoRegister,
  X0 = 1,
  X1 = 2,
  X2 = 3,
  X3 = 4,
  X4 = 5,
  X5 = 6,
  X6 = 7,
  X7 = 8,
  X8 = 9,
  X9 = 10,
  X10 = 11,
  X11 = 12,
  X12 = 13,
  X13 = 14,
  X14 = 15,
  X15 = 16,
  X16 = 17,
  X17 = 18,
  X18 = 19,
  X19 = 20,
  X20 = 21,
  X21 = 22,
  X22 = 23,
  X23 = 24,
  X24 = 25,
  X25 = 26,
  X26 = 27,
  X27 = 28,
  X28 = 29,
  X29 = 30,
  X30 = 31,
  X31 = 32,
  F0_D = 33,
  F1_D = 34,
  F2_D = 35,
  F3_D = 36,
  F4_D = 37,
  F5_D = 38,
  F6_D = 39,
  F7_D = 40,
  F8_D = 41,
  F9_D = 42,
  F10_D = 43,
  F11_D = 44,
  F12_D = 45,
  F13_D = 46,
  F14_D = 47,
  F15_D = 48,
  F16_D = 49,
  F17_D = 50,
  F18_D = 51,
  F19_D = 52,
  F20_D = 53,
  F21_D = 54,
  F22_D = 55,
  F23_D = 56,
  F24_D = 57,
  F25_D = 58,
  F26_D = 59,
  F27_D = 60,
  F28_D = 61,
  F29_D = 62,
  F30_D = 63,
  F31_D = 64,
  F0_F = 65,
  F1_F = 66,
  F2_F = 67,
  F3_F = 68,
  F4_F = 69,
  F5_F = 70,
  F6_F = 71,
  F7_F = 72,
  F8_F = 73,
  F9_F = 74,
  F10_F = 75,
  F11_F = 76,
  F12_F = 77,
  F13_F = 78,
  F14_F = 79,
  F15_F = 80,
  F16_F = 81,
  F17_F = 82,
  F18_F = 83,
  F19_F = 84,
  F20_F = 85,
  F21_F = 86,
  F22_F = 87,
  F23_F = 88,
  F24_F = 89,
  F25_F = 90,
  F26_F = 91,
  F27_F = 92,
  F28_F = 93,
  F29_F = 94,
  F30_F = 95,
  F31_F = 96,
  NUM_TARGET_REGS 	// 97
};
} // end namespace RISCV

// Register classes

namespace RISCV {
enum {
  FPR32RegClassID = 0,
  GPRRegClassID = 1,
  GPRNoX0RegClassID = 2,
  GPRNoX0X2RegClassID = 3,
  GPRTCRegClassID = 4,
  FPR32CRegClassID = 5,
  GPRCRegClassID = 6,
  GPRC_and_GPRTCRegClassID = 7,
  GPRX0RegClassID = 8,
  SPRegClassID = 9,
  FPR64RegClassID = 10,
  FPR64CRegClassID = 11,

  };
} // end namespace RISCV


// Register alternate name indices

namespace RISCV {
enum {
  ABIRegAltName,	// 0
  NoRegAltName,	// 1
  NUM_TARGET_REG_ALT_NAMES = 2
};
} // end namespace RISCV


// Subregister indices

namespace RISCV {
enum {
  NoSubRegister,
  sub_32,	// 1
  NUM_TARGET_SUBREGS
};
} // end namespace RISCV

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

extern const MCPhysReg RISCVRegDiffLists[] = {
  /* 0 */ 32, 0,
  /* 2 */ 65503, 0,
  /* 4 */ 65504, 0,
  /* 6 */ 65535, 0,
};

extern const LaneBitmask RISCVLaneMaskLists[] = {
  /* 0 */ LaneBitmask(0x00000000), LaneBitmask::getAll(),
  /* 2 */ LaneBitmask(0x00000001), LaneBitmask::getAll(),
};

extern const uint16_t RISCVSubRegIdxLists[] = {
  /* 0 */ 1, 0,
};

extern const MCRegisterInfo::SubRegCoveredBits RISCVSubRegIdxRanges[] = {
  { 65535, 65535 },
  { 0, 32 },	// sub_32
};

extern const char RISCVRegStrings[] = {
  /* 0 */ 'X', '1', '0', 0,
  /* 4 */ 'X', '2', '0', 0,
  /* 8 */ 'X', '3', '0', 0,
  /* 12 */ 'X', '0', 0,
  /* 15 */ 'X', '1', '1', 0,
  /* 19 */ 'X', '2', '1', 0,
  /* 23 */ 'X', '3', '1', 0,
  /* 27 */ 'X', '1', 0,
  /* 30 */ 'X', '1', '2', 0,
  /* 34 */ 'X', '2', '2', 0,
  /* 38 */ 'X', '2', 0,
  /* 41 */ 'X', '1', '3', 0,
  /* 45 */ 'X', '2', '3', 0,
  /* 49 */ 'X', '3', 0,
  /* 52 */ 'X', '1', '4', 0,
  /* 56 */ 'X', '2', '4', 0,
  /* 60 */ 'X', '4', 0,
  /* 63 */ 'X', '1', '5', 0,
  /* 67 */ 'X', '2', '5', 0,
  /* 71 */ 'X', '5', 0,
  /* 74 */ 'X', '1', '6', 0,
  /* 78 */ 'X', '2', '6', 0,
  /* 82 */ 'X', '6', 0,
  /* 85 */ 'X', '1', '7', 0,
  /* 89 */ 'X', '2', '7', 0,
  /* 93 */ 'X', '7', 0,
  /* 96 */ 'X', '1', '8', 0,
  /* 100 */ 'X', '2', '8', 0,
  /* 104 */ 'X', '8', 0,
  /* 107 */ 'X', '1', '9', 0,
  /* 111 */ 'X', '2', '9', 0,
  /* 115 */ 'X', '9', 0,
  /* 118 */ 'F', '1', '0', '_', 'D', 0,
  /* 124 */ 'F', '2', '0', '_', 'D', 0,
  /* 130 */ 'F', '3', '0', '_', 'D', 0,
  /* 136 */ 'F', '0', '_', 'D', 0,
  /* 141 */ 'F', '1', '1', '_', 'D', 0,
  /* 147 */ 'F', '2', '1', '_', 'D', 0,
  /* 153 */ 'F', '3', '1', '_', 'D', 0,
  /* 159 */ 'F', '1', '_', 'D', 0,
  /* 164 */ 'F', '1', '2', '_', 'D', 0,
  /* 170 */ 'F', '2', '2', '_', 'D', 0,
  /* 176 */ 'F', '2', '_', 'D', 0,
  /* 181 */ 'F', '1', '3', '_', 'D', 0,
  /* 187 */ 'F', '2', '3', '_', 'D', 0,
  /* 193 */ 'F', '3', '_', 'D', 0,
  /* 198 */ 'F', '1', '4', '_', 'D', 0,
  /* 204 */ 'F', '2', '4', '_', 'D', 0,
  /* 210 */ 'F', '4', '_', 'D', 0,
  /* 215 */ 'F', '1', '5', '_', 'D', 0,
  /* 221 */ 'F', '2', '5', '_', 'D', 0,
  /* 227 */ 'F', '5', '_', 'D', 0,
  /* 232 */ 'F', '1', '6', '_', 'D', 0,
  /* 238 */ 'F', '2', '6', '_', 'D', 0,
  /* 244 */ 'F', '6', '_', 'D', 0,
  /* 249 */ 'F', '1', '7', '_', 'D', 0,
  /* 255 */ 'F', '2', '7', '_', 'D', 0,
  /* 261 */ 'F', '7', '_', 'D', 0,
  /* 266 */ 'F', '1', '8', '_', 'D', 0,
  /* 272 */ 'F', '2', '8', '_', 'D', 0,
  /* 278 */ 'F', '8', '_', 'D', 0,
  /* 283 */ 'F', '1', '9', '_', 'D', 0,
  /* 289 */ 'F', '2', '9', '_', 'D', 0,
  /* 295 */ 'F', '9', '_', 'D', 0,
  /* 300 */ 'F', '1', '0', '_', 'F', 0,
  /* 306 */ 'F', '2', '0', '_', 'F', 0,
  /* 312 */ 'F', '3', '0', '_', 'F', 0,
  /* 318 */ 'F', '0', '_', 'F', 0,
  /* 323 */ 'F', '1', '1', '_', 'F', 0,
  /* 329 */ 'F', '2', '1', '_', 'F', 0,
  /* 335 */ 'F', '3', '1', '_', 'F', 0,
  /* 341 */ 'F', '1', '_', 'F', 0,
  /* 346 */ 'F', '1', '2', '_', 'F', 0,
  /* 352 */ 'F', '2', '2', '_', 'F', 0,
  /* 358 */ 'F', '2', '_', 'F', 0,
  /* 363 */ 'F', '1', '3', '_', 'F', 0,
  /* 369 */ 'F', '2', '3', '_', 'F', 0,
  /* 375 */ 'F', '3', '_', 'F', 0,
  /* 380 */ 'F', '1', '4', '_', 'F', 0,
  /* 386 */ 'F', '2', '4', '_', 'F', 0,
  /* 392 */ 'F', '4', '_', 'F', 0,
  /* 397 */ 'F', '1', '5', '_', 'F', 0,
  /* 403 */ 'F', '2', '5', '_', 'F', 0,
  /* 409 */ 'F', '5', '_', 'F', 0,
  /* 414 */ 'F', '1', '6', '_', 'F', 0,
  /* 420 */ 'F', '2', '6', '_', 'F', 0,
  /* 426 */ 'F', '6', '_', 'F', 0,
  /* 431 */ 'F', '1', '7', '_', 'F', 0,
  /* 437 */ 'F', '2', '7', '_', 'F', 0,
  /* 443 */ 'F', '7', '_', 'F', 0,
  /* 448 */ 'F', '1', '8', '_', 'F', 0,
  /* 454 */ 'F', '2', '8', '_', 'F', 0,
  /* 460 */ 'F', '8', '_', 'F', 0,
  /* 465 */ 'F', '1', '9', '_', 'F', 0,
  /* 471 */ 'F', '2', '9', '_', 'F', 0,
  /* 477 */ 'F', '9', '_', 'F', 0,
};

extern const MCRegisterDesc RISCVRegDesc[] = { // Descriptors
  { 3, 0, 0, 0, 0, 0 },
  { 12, 1, 1, 1, 97, 0 },
  { 27, 1, 1, 1, 97, 0 },
  { 38, 1, 1, 1, 97, 0 },
  { 49, 1, 1, 1, 97, 0 },
  { 60, 1, 1, 1, 97, 0 },
  { 71, 1, 1, 1, 97, 0 },
  { 82, 1, 1, 1, 97, 0 },
  { 93, 1, 1, 1, 97, 0 },
  { 104, 1, 1, 1, 97, 0 },
  { 115, 1, 1, 1, 97, 0 },
  { 0, 1, 1, 1, 97, 0 },
  { 15, 1, 1, 1, 97, 0 },
  { 30, 1, 1, 1, 97, 0 },
  { 41, 1, 1, 1, 97, 0 },
  { 52, 1, 1, 1, 97, 0 },
  { 63, 1, 1, 1, 97, 0 },
  { 74, 1, 1, 1, 97, 0 },
  { 85, 1, 1, 1, 97, 0 },
  { 96, 1, 1, 1, 97, 0 },
  { 107, 1, 1, 1, 97, 0 },
  { 4, 1, 1, 1, 97, 0 },
  { 19, 1, 1, 1, 97, 0 },
  { 34, 1, 1, 1, 97, 0 },
  { 45, 1, 1, 1, 97, 0 },
  { 56, 1, 1, 1, 97, 0 },
  { 67, 1, 1, 1, 97, 0 },
  { 78, 1, 1, 1, 97, 0 },
  { 89, 1, 1, 1, 97, 0 },
  { 100, 1, 1, 1, 97, 0 },
  { 111, 1, 1, 1, 97, 0 },
  { 8, 1, 1, 1, 97, 0 },
  { 23, 1, 1, 1, 97, 0 },
  { 136, 0, 1, 0, 97, 2 },
  { 159, 0, 1, 0, 97, 2 },
  { 176, 0, 1, 0, 97, 2 },
  { 193, 0, 1, 0, 97, 2 },
  { 210, 0, 1, 0, 97, 2 },
  { 227, 0, 1, 0, 97, 2 },
  { 244, 0, 1, 0, 97, 2 },
  { 261, 0, 1, 0, 97, 2 },
  { 278, 0, 1, 0, 97, 2 },
  { 295, 0, 1, 0, 97, 2 },
  { 118, 0, 1, 0, 97, 2 },
  { 141, 0, 1, 0, 97, 2 },
  { 164, 0, 1, 0, 97, 2 },
  { 181, 0, 1, 0, 97, 2 },
  { 198, 0, 1, 0, 97, 2 },
  { 215, 0, 1, 0, 97, 2 },
  { 232, 0, 1, 0, 97, 2 },
  { 249, 0, 1, 0, 97, 2 },
  { 266, 0, 1, 0, 97, 2 },
  { 283, 0, 1, 0, 97, 2 },
  { 124, 0, 1, 0, 97, 2 },
  { 147, 0, 1, 0, 97, 2 },
  { 170, 0, 1, 0, 97, 2 },
  { 187, 0, 1, 0, 97, 2 },
  { 204, 0, 1, 0, 97, 2 },
  { 221, 0, 1, 0, 97, 2 },
  { 238, 0, 1, 0, 97, 2 },
  { 255, 0, 1, 0, 97, 2 },
  { 272, 0, 1, 0, 97, 2 },
  { 289, 0, 1, 0, 97, 2 },
  { 130, 0, 1, 0, 97, 2 },
  { 153, 0, 1, 0, 97, 2 },
  { 318, 1, 4, 1, 33, 0 },
  { 341, 1, 4, 1, 33, 0 },
  { 358, 1, 4, 1, 33, 0 },
  { 375, 1, 4, 1, 33, 0 },
  { 392, 1, 4, 1, 33, 0 },
  { 409, 1, 4, 1, 33, 0 },
  { 426, 1, 4, 1, 33, 0 },
  { 443, 1, 4, 1, 33, 0 },
  { 460, 1, 4, 1, 33, 0 },
  { 477, 1, 4, 1, 33, 0 },
  { 300, 1, 4, 1, 33, 0 },
  { 323, 1, 4, 1, 33, 0 },
  { 346, 1, 4, 1, 33, 0 },
  { 363, 1, 4, 1, 33, 0 },
  { 380, 1, 4, 1, 33, 0 },
  { 397, 1, 4, 1, 33, 0 },
  { 414, 1, 4, 1, 33, 0 },
  { 431, 1, 4, 1, 33, 0 },
  { 448, 1, 4, 1, 33, 0 },
  { 465, 1, 4, 1, 33, 0 },
  { 306, 1, 4, 1, 33, 0 },
  { 329, 1, 4, 1, 33, 0 },
  { 352, 1, 4, 1, 33, 0 },
  { 369, 1, 4, 1, 33, 0 },
  { 386, 1, 4, 1, 33, 0 },
  { 403, 1, 4, 1, 33, 0 },
  { 420, 1, 4, 1, 33, 0 },
  { 437, 1, 4, 1, 33, 0 },
  { 454, 1, 4, 1, 33, 0 },
  { 471, 1, 4, 1, 33, 0 },
  { 312, 1, 4, 1, 33, 0 },
  { 335, 1, 4, 1, 33, 0 },
};

extern const MCPhysReg RISCVRegUnitRoots[][2] = {
  { RISCV::X0 },
  { RISCV::X1 },
  { RISCV::X2 },
  { RISCV::X3 },
  { RISCV::X4 },
  { RISCV::X5 },
  { RISCV::X6 },
  { RISCV::X7 },
  { RISCV::X8 },
  { RISCV::X9 },
  { RISCV::X10 },
  { RISCV::X11 },
  { RISCV::X12 },
  { RISCV::X13 },
  { RISCV::X14 },
  { RISCV::X15 },
  { RISCV::X16 },
  { RISCV::X17 },
  { RISCV::X18 },
  { RISCV::X19 },
  { RISCV::X20 },
  { RISCV::X21 },
  { RISCV::X22 },
  { RISCV::X23 },
  { RISCV::X24 },
  { RISCV::X25 },
  { RISCV::X26 },
  { RISCV::X27 },
  { RISCV::X28 },
  { RISCV::X29 },
  { RISCV::X30 },
  { RISCV::X31 },
  { RISCV::F0_F },
  { RISCV::F1_F },
  { RISCV::F2_F },
  { RISCV::F3_F },
  { RISCV::F4_F },
  { RISCV::F5_F },
  { RISCV::F6_F },
  { RISCV::F7_F },
  { RISCV::F8_F },
  { RISCV::F9_F },
  { RISCV::F10_F },
  { RISCV::F11_F },
  { RISCV::F12_F },
  { RISCV::F13_F },
  { RISCV::F14_F },
  { RISCV::F15_F },
  { RISCV::F16_F },
  { RISCV::F17_F },
  { RISCV::F18_F },
  { RISCV::F19_F },
  { RISCV::F20_F },
  { RISCV::F21_F },
  { RISCV::F22_F },
  { RISCV::F23_F },
  { RISCV::F24_F },
  { RISCV::F25_F },
  { RISCV::F26_F },
  { RISCV::F27_F },
  { RISCV::F28_F },
  { RISCV::F29_F },
  { RISCV::F30_F },
  { RISCV::F31_F },
};

namespace {     // Register classes...
  // FPR32 Register Class...
  const MCPhysReg FPR32[] = {
    RISCV::F0_F, RISCV::F1_F, RISCV::F2_F, RISCV::F3_F, RISCV::F4_F, RISCV::F5_F, RISCV::F6_F, RISCV::F7_F, RISCV::F10_F, RISCV::F11_F, RISCV::F12_F, RISCV::F13_F, RISCV::F14_F, RISCV::F15_F, RISCV::F16_F, RISCV::F17_F, RISCV::F28_F, RISCV::F29_F, RISCV::F30_F, RISCV::F31_F, RISCV::F8_F, RISCV::F9_F, RISCV::F18_F, RISCV::F19_F, RISCV::F20_F, RISCV::F21_F, RISCV::F22_F, RISCV::F23_F, RISCV::F24_F, RISCV::F25_F, RISCV::F26_F, RISCV::F27_F,
  };

  // FPR32 Bit set.
  const uint8_t FPR32Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01,
  };

  // GPR Register Class...
  const MCPhysReg GPR[] = {
    RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X0, RISCV::X1, RISCV::X2, RISCV::X3, RISCV::X4,
  };

  // GPR Bit set.
  const uint8_t GPRBits[] = {
    0xfe, 0xff, 0xff, 0xff, 0x01,
  };

  // GPRNoX0 Register Class...
  const MCPhysReg GPRNoX0[] = {
    RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X1, RISCV::X2, RISCV::X3, RISCV::X4,
  };

  // GPRNoX0 Bit set.
  const uint8_t GPRNoX0Bits[] = {
    0xfc, 0xff, 0xff, 0xff, 0x01,
  };

  // GPRNoX0X2 Register Class...
  const MCPhysReg GPRNoX0X2[] = {
    RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X1, RISCV::X3, RISCV::X4,
  };

  // GPRNoX0X2 Bit set.
  const uint8_t GPRNoX0X2Bits[] = {
    0xf4, 0xff, 0xff, 0xff, 0x01,
  };

  // GPRTC Register Class...
  const MCPhysReg GPRTC[] = {
    RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31,
  };

  // GPRTC Bit set.
  const uint8_t GPRTCBits[] = {
    0xc0, 0xf9, 0x07, 0xe0, 0x01,
  };

  // FPR32C Register Class...
  const MCPhysReg FPR32C[] = {
    RISCV::F10_F, RISCV::F11_F, RISCV::F12_F, RISCV::F13_F, RISCV::F14_F, RISCV::F15_F, RISCV::F8_F, RISCV::F9_F,
  };

  // FPR32C Bit set.
  const uint8_t FPR32CBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01,
  };

  // GPRC Register Class...
  const MCPhysReg GPRC[] = {
    RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X8, RISCV::X9,
  };

  // GPRC Bit set.
  const uint8_t GPRCBits[] = {
    0x00, 0xfe, 0x01,
  };

  // GPRC_and_GPRTC Register Class...
  const MCPhysReg GPRC_and_GPRTC[] = {
    RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15,
  };

  // GPRC_and_GPRTC Bit set.
  const uint8_t GPRC_and_GPRTCBits[] = {
    0x00, 0xf8, 0x01,
  };

  // GPRX0 Register Class...
  const MCPhysReg GPRX0[] = {
    RISCV::X0,
  };

  // GPRX0 Bit set.
  const uint8_t GPRX0Bits[] = {
    0x02,
  };

  // SP Register Class...
  const MCPhysReg SP[] = {
    RISCV::X2,
  };

  // SP Bit set.
  const uint8_t SPBits[] = {
    0x08,
  };

  // FPR64 Register Class...
  const MCPhysReg FPR64[] = {
    RISCV::F0_D, RISCV::F1_D, RISCV::F2_D, RISCV::F3_D, RISCV::F4_D, RISCV::F5_D, RISCV::F6_D, RISCV::F7_D, RISCV::F10_D, RISCV::F11_D, RISCV::F12_D, RISCV::F13_D, RISCV::F14_D, RISCV::F15_D, RISCV::F16_D, RISCV::F17_D, RISCV::F28_D, RISCV::F29_D, RISCV::F30_D, RISCV::F31_D, RISCV::F8_D, RISCV::F9_D, RISCV::F18_D, RISCV::F19_D, RISCV::F20_D, RISCV::F21_D, RISCV::F22_D, RISCV::F23_D, RISCV::F24_D, RISCV::F25_D, RISCV::F26_D, RISCV::F27_D,
  };

  // FPR64 Bit set.
  const uint8_t FPR64Bits[] = {
    0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0x01,
  };

  // FPR64C Register Class...
  const MCPhysReg FPR64C[] = {
    RISCV::F10_D, RISCV::F11_D, RISCV::F12_D, RISCV::F13_D, RISCV::F14_D, RISCV::F15_D, RISCV::F8_D, RISCV::F9_D,
  };

  // FPR64C Bit set.
  const uint8_t FPR64CBits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x01,
  };

} // end anonymous namespace

extern const char RISCVRegClassStrings[] = {
  /* 0 */ 'G', 'P', 'R', 'X', '0', 0,
  /* 6 */ 'G', 'P', 'R', 'N', 'o', 'X', '0', 0,
  /* 14 */ 'F', 'P', 'R', '3', '2', 0,
  /* 20 */ 'G', 'P', 'R', 'N', 'o', 'X', '0', 'X', '2', 0,
  /* 30 */ 'F', 'P', 'R', '6', '4', 0,
  /* 36 */ 'F', 'P', 'R', '3', '2', 'C', 0,
  /* 43 */ 'F', 'P', 'R', '6', '4', 'C', 0,
  /* 50 */ 'G', 'P', 'R', 'C', 0,
  /* 55 */ 'G', 'P', 'R', 'C', '_', 'a', 'n', 'd', '_', 'G', 'P', 'R', 'T', 'C', 0,
  /* 70 */ 'S', 'P', 0,
  /* 73 */ 'G', 'P', 'R', 0,
};

extern const MCRegisterClass RISCVMCRegisterClasses[] = {
  { FPR32, FPR32Bits, 14, 32, sizeof(FPR32Bits), RISCV::FPR32RegClassID, 1, true },
  { GPR, GPRBits, 73, 32, sizeof(GPRBits), RISCV::GPRRegClassID, 1, true },
  { GPRNoX0, GPRNoX0Bits, 6, 31, sizeof(GPRNoX0Bits), RISCV::GPRNoX0RegClassID, 1, true },
  { GPRNoX0X2, GPRNoX0X2Bits, 20, 30, sizeof(GPRNoX0X2Bits), RISCV::GPRNoX0X2RegClassID, 1, true },
  { GPRTC, GPRTCBits, 64, 15, sizeof(GPRTCBits), RISCV::GPRTCRegClassID, 1, true },
  { FPR32C, FPR32CBits, 36, 8, sizeof(FPR32CBits), RISCV::FPR32CRegClassID, 1, true },
  { GPRC, GPRCBits, 50, 8, sizeof(GPRCBits), RISCV::GPRCRegClassID, 1, true },
  { GPRC_and_GPRTC, GPRC_and_GPRTCBits, 55, 6, sizeof(GPRC_and_GPRTCBits), RISCV::GPRC_and_GPRTCRegClassID, 1, true },
  { GPRX0, GPRX0Bits, 0, 1, sizeof(GPRX0Bits), RISCV::GPRX0RegClassID, 1, true },
  { SP, SPBits, 70, 1, sizeof(SPBits), RISCV::SPRegClassID, 1, true },
  { FPR64, FPR64Bits, 30, 32, sizeof(FPR64Bits), RISCV::FPR64RegClassID, 1, true },
  { FPR64C, FPR64CBits, 43, 8, sizeof(FPR64CBits), RISCV::FPR64CRegClassID, 1, true },
};

// RISCV Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair RISCVDwarfFlavour0Dwarf2L[] = {
  { 0U, RISCV::X0 },
  { 1U, RISCV::X1 },
  { 2U, RISCV::X2 },
  { 3U, RISCV::X3 },
  { 4U, RISCV::X4 },
  { 5U, RISCV::X5 },
  { 6U, RISCV::X6 },
  { 7U, RISCV::X7 },
  { 8U, RISCV::X8 },
  { 9U, RISCV::X9 },
  { 10U, RISCV::X10 },
  { 11U, RISCV::X11 },
  { 12U, RISCV::X12 },
  { 13U, RISCV::X13 },
  { 14U, RISCV::X14 },
  { 15U, RISCV::X15 },
  { 16U, RISCV::X16 },
  { 17U, RISCV::X17 },
  { 18U, RISCV::X18 },
  { 19U, RISCV::X19 },
  { 20U, RISCV::X20 },
  { 21U, RISCV::X21 },
  { 22U, RISCV::X22 },
  { 23U, RISCV::X23 },
  { 24U, RISCV::X24 },
  { 25U, RISCV::X25 },
  { 26U, RISCV::X26 },
  { 27U, RISCV::X27 },
  { 28U, RISCV::X28 },
  { 29U, RISCV::X29 },
  { 30U, RISCV::X30 },
  { 31U, RISCV::X31 },
  { 32U, RISCV::F0_F },
  { 33U, RISCV::F1_F },
  { 34U, RISCV::F2_F },
  { 35U, RISCV::F3_F },
  { 36U, RISCV::F4_F },
  { 37U, RISCV::F5_F },
  { 38U, RISCV::F6_F },
  { 39U, RISCV::F7_F },
  { 40U, RISCV::F8_F },
  { 41U, RISCV::F9_F },
  { 42U, RISCV::F10_F },
  { 43U, RISCV::F11_F },
  { 44U, RISCV::F12_F },
  { 45U, RISCV::F13_F },
  { 46U, RISCV::F14_F },
  { 47U, RISCV::F15_F },
  { 48U, RISCV::F16_F },
  { 49U, RISCV::F17_F },
  { 50U, RISCV::F18_F },
  { 51U, RISCV::F19_F },
  { 52U, RISCV::F20_F },
  { 53U, RISCV::F21_F },
  { 54U, RISCV::F22_F },
  { 55U, RISCV::F23_F },
  { 56U, RISCV::F24_F },
  { 57U, RISCV::F25_F },
  { 58U, RISCV::F26_F },
  { 59U, RISCV::F27_F },
  { 60U, RISCV::F28_F },
  { 61U, RISCV::F29_F },
  { 62U, RISCV::F30_F },
  { 63U, RISCV::F31_F },
};
extern const unsigned RISCVDwarfFlavour0Dwarf2LSize = array_lengthof(RISCVDwarfFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVEHFlavour0Dwarf2L[] = {
  { 0U, RISCV::X0 },
  { 1U, RISCV::X1 },
  { 2U, RISCV::X2 },
  { 3U, RISCV::X3 },
  { 4U, RISCV::X4 },
  { 5U, RISCV::X5 },
  { 6U, RISCV::X6 },
  { 7U, RISCV::X7 },
  { 8U, RISCV::X8 },
  { 9U, RISCV::X9 },
  { 10U, RISCV::X10 },
  { 11U, RISCV::X11 },
  { 12U, RISCV::X12 },
  { 13U, RISCV::X13 },
  { 14U, RISCV::X14 },
  { 15U, RISCV::X15 },
  { 16U, RISCV::X16 },
  { 17U, RISCV::X17 },
  { 18U, RISCV::X18 },
  { 19U, RISCV::X19 },
  { 20U, RISCV::X20 },
  { 21U, RISCV::X21 },
  { 22U, RISCV::X22 },
  { 23U, RISCV::X23 },
  { 24U, RISCV::X24 },
  { 25U, RISCV::X25 },
  { 26U, RISCV::X26 },
  { 27U, RISCV::X27 },
  { 28U, RISCV::X28 },
  { 29U, RISCV::X29 },
  { 30U, RISCV::X30 },
  { 31U, RISCV::X31 },
  { 32U, RISCV::F0_F },
  { 33U, RISCV::F1_F },
  { 34U, RISCV::F2_F },
  { 35U, RISCV::F3_F },
  { 36U, RISCV::F4_F },
  { 37U, RISCV::F5_F },
  { 38U, RISCV::F6_F },
  { 39U, RISCV::F7_F },
  { 40U, RISCV::F8_F },
  { 41U, RISCV::F9_F },
  { 42U, RISCV::F10_F },
  { 43U, RISCV::F11_F },
  { 44U, RISCV::F12_F },
  { 45U, RISCV::F13_F },
  { 46U, RISCV::F14_F },
  { 47U, RISCV::F15_F },
  { 48U, RISCV::F16_F },
  { 49U, RISCV::F17_F },
  { 50U, RISCV::F18_F },
  { 51U, RISCV::F19_F },
  { 52U, RISCV::F20_F },
  { 53U, RISCV::F21_F },
  { 54U, RISCV::F22_F },
  { 55U, RISCV::F23_F },
  { 56U, RISCV::F24_F },
  { 57U, RISCV::F25_F },
  { 58U, RISCV::F26_F },
  { 59U, RISCV::F27_F },
  { 60U, RISCV::F28_F },
  { 61U, RISCV::F29_F },
  { 62U, RISCV::F30_F },
  { 63U, RISCV::F31_F },
};
extern const unsigned RISCVEHFlavour0Dwarf2LSize = array_lengthof(RISCVEHFlavour0Dwarf2L);

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVDwarfFlavour0L2Dwarf[] = {
  { RISCV::X0, 0U },
  { RISCV::X1, 1U },
  { RISCV::X2, 2U },
  { RISCV::X3, 3U },
  { RISCV::X4, 4U },
  { RISCV::X5, 5U },
  { RISCV::X6, 6U },
  { RISCV::X7, 7U },
  { RISCV::X8, 8U },
  { RISCV::X9, 9U },
  { RISCV::X10, 10U },
  { RISCV::X11, 11U },
  { RISCV::X12, 12U },
  { RISCV::X13, 13U },
  { RISCV::X14, 14U },
  { RISCV::X15, 15U },
  { RISCV::X16, 16U },
  { RISCV::X17, 17U },
  { RISCV::X18, 18U },
  { RISCV::X19, 19U },
  { RISCV::X20, 20U },
  { RISCV::X21, 21U },
  { RISCV::X22, 22U },
  { RISCV::X23, 23U },
  { RISCV::X24, 24U },
  { RISCV::X25, 25U },
  { RISCV::X26, 26U },
  { RISCV::X27, 27U },
  { RISCV::X28, 28U },
  { RISCV::X29, 29U },
  { RISCV::X30, 30U },
  { RISCV::X31, 31U },
  { RISCV::F0_D, 32U },
  { RISCV::F1_D, 33U },
  { RISCV::F2_D, 34U },
  { RISCV::F3_D, 35U },
  { RISCV::F4_D, 36U },
  { RISCV::F5_D, 37U },
  { RISCV::F6_D, 38U },
  { RISCV::F7_D, 39U },
  { RISCV::F8_D, 40U },
  { RISCV::F9_D, 41U },
  { RISCV::F10_D, 42U },
  { RISCV::F11_D, 43U },
  { RISCV::F12_D, 44U },
  { RISCV::F13_D, 45U },
  { RISCV::F14_D, 46U },
  { RISCV::F15_D, 47U },
  { RISCV::F16_D, 48U },
  { RISCV::F17_D, 49U },
  { RISCV::F18_D, 50U },
  { RISCV::F19_D, 51U },
  { RISCV::F20_D, 52U },
  { RISCV::F21_D, 53U },
  { RISCV::F22_D, 54U },
  { RISCV::F23_D, 55U },
  { RISCV::F24_D, 56U },
  { RISCV::F25_D, 57U },
  { RISCV::F26_D, 58U },
  { RISCV::F27_D, 59U },
  { RISCV::F28_D, 60U },
  { RISCV::F29_D, 61U },
  { RISCV::F30_D, 62U },
  { RISCV::F31_D, 63U },
  { RISCV::F0_F, 32U },
  { RISCV::F1_F, 33U },
  { RISCV::F2_F, 34U },
  { RISCV::F3_F, 35U },
  { RISCV::F4_F, 36U },
  { RISCV::F5_F, 37U },
  { RISCV::F6_F, 38U },
  { RISCV::F7_F, 39U },
  { RISCV::F8_F, 40U },
  { RISCV::F9_F, 41U },
  { RISCV::F10_F, 42U },
  { RISCV::F11_F, 43U },
  { RISCV::F12_F, 44U },
  { RISCV::F13_F, 45U },
  { RISCV::F14_F, 46U },
  { RISCV::F15_F, 47U },
  { RISCV::F16_F, 48U },
  { RISCV::F17_F, 49U },
  { RISCV::F18_F, 50U },
  { RISCV::F19_F, 51U },
  { RISCV::F20_F, 52U },
  { RISCV::F21_F, 53U },
  { RISCV::F22_F, 54U },
  { RISCV::F23_F, 55U },
  { RISCV::F24_F, 56U },
  { RISCV::F25_F, 57U },
  { RISCV::F26_F, 58U },
  { RISCV::F27_F, 59U },
  { RISCV::F28_F, 60U },
  { RISCV::F29_F, 61U },
  { RISCV::F30_F, 62U },
  { RISCV::F31_F, 63U },
};
extern const unsigned RISCVDwarfFlavour0L2DwarfSize = array_lengthof(RISCVDwarfFlavour0L2Dwarf);

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVEHFlavour0L2Dwarf[] = {
  { RISCV::X0, 0U },
  { RISCV::X1, 1U },
  { RISCV::X2, 2U },
  { RISCV::X3, 3U },
  { RISCV::X4, 4U },
  { RISCV::X5, 5U },
  { RISCV::X6, 6U },
  { RISCV::X7, 7U },
  { RISCV::X8, 8U },
  { RISCV::X9, 9U },
  { RISCV::X10, 10U },
  { RISCV::X11, 11U },
  { RISCV::X12, 12U },
  { RISCV::X13, 13U },
  { RISCV::X14, 14U },
  { RISCV::X15, 15U },
  { RISCV::X16, 16U },
  { RISCV::X17, 17U },
  { RISCV::X18, 18U },
  { RISCV::X19, 19U },
  { RISCV::X20, 20U },
  { RISCV::X21, 21U },
  { RISCV::X22, 22U },
  { RISCV::X23, 23U },
  { RISCV::X24, 24U },
  { RISCV::X25, 25U },
  { RISCV::X26, 26U },
  { RISCV::X27, 27U },
  { RISCV::X28, 28U },
  { RISCV::X29, 29U },
  { RISCV::X30, 30U },
  { RISCV::X31, 31U },
  { RISCV::F0_D, 32U },
  { RISCV::F1_D, 33U },
  { RISCV::F2_D, 34U },
  { RISCV::F3_D, 35U },
  { RISCV::F4_D, 36U },
  { RISCV::F5_D, 37U },
  { RISCV::F6_D, 38U },
  { RISCV::F7_D, 39U },
  { RISCV::F8_D, 40U },
  { RISCV::F9_D, 41U },
  { RISCV::F10_D, 42U },
  { RISCV::F11_D, 43U },
  { RISCV::F12_D, 44U },
  { RISCV::F13_D, 45U },
  { RISCV::F14_D, 46U },
  { RISCV::F15_D, 47U },
  { RISCV::F16_D, 48U },
  { RISCV::F17_D, 49U },
  { RISCV::F18_D, 50U },
  { RISCV::F19_D, 51U },
  { RISCV::F20_D, 52U },
  { RISCV::F21_D, 53U },
  { RISCV::F22_D, 54U },
  { RISCV::F23_D, 55U },
  { RISCV::F24_D, 56U },
  { RISCV::F25_D, 57U },
  { RISCV::F26_D, 58U },
  { RISCV::F27_D, 59U },
  { RISCV::F28_D, 60U },
  { RISCV::F29_D, 61U },
  { RISCV::F30_D, 62U },
  { RISCV::F31_D, 63U },
  { RISCV::F0_F, 32U },
  { RISCV::F1_F, 33U },
  { RISCV::F2_F, 34U },
  { RISCV::F3_F, 35U },
  { RISCV::F4_F, 36U },
  { RISCV::F5_F, 37U },
  { RISCV::F6_F, 38U },
  { RISCV::F7_F, 39U },
  { RISCV::F8_F, 40U },
  { RISCV::F9_F, 41U },
  { RISCV::F10_F, 42U },
  { RISCV::F11_F, 43U },
  { RISCV::F12_F, 44U },
  { RISCV::F13_F, 45U },
  { RISCV::F14_F, 46U },
  { RISCV::F15_F, 47U },
  { RISCV::F16_F, 48U },
  { RISCV::F17_F, 49U },
  { RISCV::F18_F, 50U },
  { RISCV::F19_F, 51U },
  { RISCV::F20_F, 52U },
  { RISCV::F21_F, 53U },
  { RISCV::F22_F, 54U },
  { RISCV::F23_F, 55U },
  { RISCV::F24_F, 56U },
  { RISCV::F25_F, 57U },
  { RISCV::F26_F, 58U },
  { RISCV::F27_F, 59U },
  { RISCV::F28_F, 60U },
  { RISCV::F29_F, 61U },
  { RISCV::F30_F, 62U },
  { RISCV::F31_F, 63U },
};
extern const unsigned RISCVEHFlavour0L2DwarfSize = array_lengthof(RISCVEHFlavour0L2Dwarf);

extern const uint16_t RISCVRegEncodingTable[] = {
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
static inline void InitRISCVMCRegisterInfo(MCRegisterInfo *RI, unsigned RA, unsigned DwarfFlavour = 0, unsigned EHFlavour = 0, unsigned PC = 0) {
  RI->InitMCRegisterInfo(RISCVRegDesc, 97, RA, PC, RISCVMCRegisterClasses, 12, RISCVRegUnitRoots, 64, RISCVRegDiffLists, RISCVLaneMaskLists, RISCVRegStrings, RISCVRegClassStrings, RISCVSubRegIdxLists, 2,
RISCVSubRegIdxRanges, RISCVRegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(RISCVDwarfFlavour0Dwarf2L, RISCVDwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapDwarfRegsToLLVMRegs(RISCVEHFlavour0Dwarf2L, RISCVEHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(RISCVDwarfFlavour0L2Dwarf, RISCVDwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    RI->mapLLVMRegsToDwarfRegs(RISCVEHFlavour0L2Dwarf, RISCVEHFlavour0L2DwarfSize, true);
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

class RISCVFrameLowering;

struct RISCVGenRegisterInfo : public TargetRegisterInfo {
  explicit RISCVGenRegisterInfo(unsigned RA, unsigned D = 0, unsigned E = 0,
      unsigned PC = 0, unsigned HwMode = 0);
  unsigned composeSubRegIndicesImpl(unsigned, unsigned) const override;
  LaneBitmask composeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  LaneBitmask reverseComposeSubRegIndexLaneMaskImpl(unsigned, LaneBitmask) const override;
  const TargetRegisterClass *getSubClassWithSubReg(const TargetRegisterClass*, unsigned) const override;
  const RegClassWeight &getRegClassWeight(const TargetRegisterClass *RC) const override;
  unsigned getRegUnitWeight(unsigned RegUnit) const override;
  unsigned getNumRegPressureSets() const override;
  const char *getRegPressureSetName(unsigned Idx) const override;
  unsigned getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const override;
  const int *getRegClassPressureSets(const TargetRegisterClass *RC) const override;
  const int *getRegUnitPressureSets(unsigned RegUnit) const override;
  ArrayRef<const char *> getRegMaskNames() const override;
  ArrayRef<const uint32_t *> getRegMasks() const override;
  /// Devirtualized TargetFrameLowering.
  static const RISCVFrameLowering *getFrameLowering(
      const MachineFunction &MF);
};

namespace RISCV { // Register classes
  extern const TargetRegisterClass FPR32RegClass;
  extern const TargetRegisterClass GPRRegClass;
  extern const TargetRegisterClass GPRNoX0RegClass;
  extern const TargetRegisterClass GPRNoX0X2RegClass;
  extern const TargetRegisterClass GPRTCRegClass;
  extern const TargetRegisterClass FPR32CRegClass;
  extern const TargetRegisterClass GPRCRegClass;
  extern const TargetRegisterClass GPRC_and_GPRTCRegClass;
  extern const TargetRegisterClass GPRX0RegClass;
  extern const TargetRegisterClass SPRegClass;
  extern const TargetRegisterClass FPR64RegClass;
  extern const TargetRegisterClass FPR64CRegClass;
} // end namespace RISCV

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

extern const MCRegisterClass RISCVMCRegisterClasses[];

static const MVT::SimpleValueType VTLists[] = {
  /* 0 */ MVT::i32, MVT::Other,
  /* 2 */ MVT::i64, MVT::Other,
  /* 4 */ MVT::f32, MVT::Other,
  /* 6 */ MVT::f64, MVT::Other,
};

static const char *const SubRegIndexNameTable[] = { "sub_32", "" };


static const LaneBitmask SubRegIndexLaneMaskTable[] = {
  LaneBitmask::getAll(),
  LaneBitmask(0x00000001), // sub_32
 };



static const TargetRegisterInfo::RegClassInfo RegClassInfos[] = {
  // Mode = 0 (Default)
  { 32, 32, 32, VTLists+4 },    // FPR32
  { 32, 32, 32, VTLists+0 },    // GPR
  { 32, 32, 32, VTLists+0 },    // GPRNoX0
  { 32, 32, 32, VTLists+0 },    // GPRNoX0X2
  { 32, 32, 32, VTLists+0 },    // GPRTC
  { 32, 32, 32, VTLists+4 },    // FPR32C
  { 32, 32, 32, VTLists+0 },    // GPRC
  { 32, 32, 32, VTLists+0 },    // GPRC_and_GPRTC
  { 32, 32, 32, VTLists+0 },    // GPRX0
  { 32, 32, 32, VTLists+0 },    // SP
  { 64, 64, 64, VTLists+6 },    // FPR64
  { 64, 64, 64, VTLists+6 },    // FPR64C
  // Mode = 1 (RV32)
  { 32, 32, 32, VTLists+4 },    // FPR32
  { 32, 32, 32, VTLists+0 },    // GPR
  { 32, 32, 32, VTLists+0 },    // GPRNoX0
  { 32, 32, 32, VTLists+0 },    // GPRNoX0X2
  { 32, 32, 32, VTLists+0 },    // GPRTC
  { 32, 32, 32, VTLists+4 },    // FPR32C
  { 32, 32, 32, VTLists+0 },    // GPRC
  { 32, 32, 32, VTLists+0 },    // GPRC_and_GPRTC
  { 32, 32, 32, VTLists+0 },    // GPRX0
  { 32, 32, 32, VTLists+0 },    // SP
  { 64, 64, 64, VTLists+6 },    // FPR64
  { 64, 64, 64, VTLists+6 },    // FPR64C
  // Mode = 2 (RV64)
  { 32, 32, 32, VTLists+4 },    // FPR32
  { 64, 64, 64, VTLists+2 },    // GPR
  { 64, 64, 64, VTLists+2 },    // GPRNoX0
  { 64, 64, 64, VTLists+2 },    // GPRNoX0X2
  { 64, 64, 64, VTLists+2 },    // GPRTC
  { 32, 32, 32, VTLists+4 },    // FPR32C
  { 64, 64, 64, VTLists+2 },    // GPRC
  { 64, 64, 64, VTLists+2 },    // GPRC_and_GPRTC
  { 64, 64, 64, VTLists+2 },    // GPRX0
  { 64, 64, 64, VTLists+2 },    // SP
  { 64, 64, 64, VTLists+6 },    // FPR64
  { 64, 64, 64, VTLists+6 },    // FPR64C
};

static const TargetRegisterClass *const NullRegClasses[] = { nullptr };

static const uint32_t FPR32SubClassMask[] = {
  0x00000021,
  0x00000c00, // sub_32
};

static const uint32_t GPRSubClassMask[] = {
  0x000003de,
};

static const uint32_t GPRNoX0SubClassMask[] = {
  0x000002dc,
};

static const uint32_t GPRNoX0X2SubClassMask[] = {
  0x000000d8,
};

static const uint32_t GPRTCSubClassMask[] = {
  0x00000090,
};

static const uint32_t FPR32CSubClassMask[] = {
  0x00000020,
  0x00000800, // sub_32
};

static const uint32_t GPRCSubClassMask[] = {
  0x000000c0,
};

static const uint32_t GPRC_and_GPRTCSubClassMask[] = {
  0x00000080,
};

static const uint32_t GPRX0SubClassMask[] = {
  0x00000100,
};

static const uint32_t SPSubClassMask[] = {
  0x00000200,
};

static const uint32_t FPR64SubClassMask[] = {
  0x00000c00,
};

static const uint32_t FPR64CSubClassMask[] = {
  0x00000800,
};

static const uint16_t SuperRegIdxSeqs[] = {
  /* 0 */ 1, 0,
};

static const TargetRegisterClass *const GPRNoX0Superclasses[] = {
  &RISCV::GPRRegClass,
  nullptr
};

static const TargetRegisterClass *const GPRNoX0X2Superclasses[] = {
  &RISCV::GPRRegClass,
  &RISCV::GPRNoX0RegClass,
  nullptr
};

static const TargetRegisterClass *const GPRTCSuperclasses[] = {
  &RISCV::GPRRegClass,
  &RISCV::GPRNoX0RegClass,
  &RISCV::GPRNoX0X2RegClass,
  nullptr
};

static const TargetRegisterClass *const FPR32CSuperclasses[] = {
  &RISCV::FPR32RegClass,
  nullptr
};

static const TargetRegisterClass *const GPRCSuperclasses[] = {
  &RISCV::GPRRegClass,
  &RISCV::GPRNoX0RegClass,
  &RISCV::GPRNoX0X2RegClass,
  nullptr
};

static const TargetRegisterClass *const GPRC_and_GPRTCSuperclasses[] = {
  &RISCV::GPRRegClass,
  &RISCV::GPRNoX0RegClass,
  &RISCV::GPRNoX0X2RegClass,
  &RISCV::GPRTCRegClass,
  &RISCV::GPRCRegClass,
  nullptr
};

static const TargetRegisterClass *const GPRX0Superclasses[] = {
  &RISCV::GPRRegClass,
  nullptr
};

static const TargetRegisterClass *const SPSuperclasses[] = {
  &RISCV::GPRRegClass,
  &RISCV::GPRNoX0RegClass,
  nullptr
};

static const TargetRegisterClass *const FPR64CSuperclasses[] = {
  &RISCV::FPR64RegClass,
  nullptr
};


namespace RISCV {   // Register class instances
  extern const TargetRegisterClass FPR32RegClass = {
    &RISCVMCRegisterClasses[FPR32RegClassID],
    FPR32SubClassMask,
    SuperRegIdxSeqs + 0,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPRRegClass = {
    &RISCVMCRegisterClasses[GPRRegClassID],
    GPRSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass GPRNoX0RegClass = {
    &RISCVMCRegisterClasses[GPRNoX0RegClassID],
    GPRNoX0SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRNoX0Superclasses,
    nullptr
  };

  extern const TargetRegisterClass GPRNoX0X2RegClass = {
    &RISCVMCRegisterClasses[GPRNoX0X2RegClassID],
    GPRNoX0X2SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRNoX0X2Superclasses,
    nullptr
  };

  extern const TargetRegisterClass GPRTCRegClass = {
    &RISCVMCRegisterClasses[GPRTCRegClassID],
    GPRTCSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRTCSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass FPR32CRegClass = {
    &RISCVMCRegisterClasses[FPR32CRegClassID],
    FPR32CSubClassMask,
    SuperRegIdxSeqs + 0,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    FPR32CSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPRCRegClass = {
    &RISCVMCRegisterClasses[GPRCRegClassID],
    GPRCSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRCSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPRC_and_GPRTCRegClass = {
    &RISCVMCRegisterClasses[GPRC_and_GPRTCRegClassID],
    GPRC_and_GPRTCSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRC_and_GPRTCSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass GPRX0RegClass = {
    &RISCVMCRegisterClasses[GPRX0RegClassID],
    GPRX0SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    GPRX0Superclasses,
    nullptr
  };

  extern const TargetRegisterClass SPRegClass = {
    &RISCVMCRegisterClasses[SPRegClassID],
    SPSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    SPSuperclasses,
    nullptr
  };

  extern const TargetRegisterClass FPR64RegClass = {
    &RISCVMCRegisterClasses[FPR64RegClassID],
    FPR64SubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    NullRegClasses,
    nullptr
  };

  extern const TargetRegisterClass FPR64CRegClass = {
    &RISCVMCRegisterClasses[FPR64CRegClassID],
    FPR64CSubClassMask,
    SuperRegIdxSeqs + 1,
    LaneBitmask(0x00000001),
    0,
    false, /* HasDisjunctSubRegs */
    false, /* CoveredBySubRegs */
    FPR64CSuperclasses,
    nullptr
  };

} // end namespace RISCV

namespace {
  const TargetRegisterClass* const RegisterClasses[] = {
    &RISCV::FPR32RegClass,
    &RISCV::GPRRegClass,
    &RISCV::GPRNoX0RegClass,
    &RISCV::GPRNoX0X2RegClass,
    &RISCV::GPRTCRegClass,
    &RISCV::FPR32CRegClass,
    &RISCV::GPRCRegClass,
    &RISCV::GPRC_and_GPRTCRegClass,
    &RISCV::GPRX0RegClass,
    &RISCV::SPRegClass,
    &RISCV::FPR64RegClass,
    &RISCV::FPR64CRegClass,
  };
} // end anonymous namespace

static const TargetRegisterInfoDesc RISCVRegInfoDesc[] = { // Extra Descriptors
  { 0, false },
  { 0, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 1, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
  { 0, true },
};
unsigned RISCVGenRegisterInfo::composeSubRegIndicesImpl(unsigned IdxA, unsigned IdxB) const {
  static const uint8_t Rows[1][1] = {
    { 0, },
  };

  --IdxA; assert(IdxA < 1);
  --IdxB; assert(IdxB < 1);
  return Rows[0][IdxB];
}

  struct MaskRolOp {
    LaneBitmask Mask;
    uint8_t  RotateLeft;
  };
  static const MaskRolOp LaneMaskComposeSequences[] = {
    { LaneBitmask(0xFFFFFFFF),  0 }, { LaneBitmask::getNone(), 0 }  // Sequence 0
  };
  static const MaskRolOp *const CompositeSequences[] = {
    &LaneMaskComposeSequences[0] // to sub_32
  };

LaneBitmask RISCVGenRegisterInfo::composeSubRegIndexLaneMaskImpl(unsigned IdxA, LaneBitmask LaneMask) const {
  --IdxA; assert(IdxA < 1 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops = CompositeSequences[IdxA]; Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger() & Ops->Mask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M << S) | (M >> (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

LaneBitmask RISCVGenRegisterInfo::reverseComposeSubRegIndexLaneMaskImpl(unsigned IdxA,  LaneBitmask LaneMask) const {
  LaneMask &= getSubRegIndexLaneMask(IdxA);
  --IdxA; assert(IdxA < 1 && "Subregister index out of bounds");
  LaneBitmask Result;
  for (const MaskRolOp *Ops = CompositeSequences[IdxA]; Ops->Mask.any(); ++Ops) {
    LaneBitmask::Type M = LaneMask.getAsInteger();
    if (unsigned S = Ops->RotateLeft)
      Result |= LaneBitmask((M >> S) | (M << (LaneBitmask::BitWidth - S)));
    else
      Result |= LaneBitmask(M);
  }
  return Result;
}

const TargetRegisterClass *RISCVGenRegisterInfo::getSubClassWithSubReg(const TargetRegisterClass *RC, unsigned Idx) const {
  static const uint8_t Table[12][1] = {
    {	// FPR32
      0,	// sub_32
    },
    {	// GPR
      0,	// sub_32
    },
    {	// GPRNoX0
      0,	// sub_32
    },
    {	// GPRNoX0X2
      0,	// sub_32
    },
    {	// GPRTC
      0,	// sub_32
    },
    {	// FPR32C
      0,	// sub_32
    },
    {	// GPRC
      0,	// sub_32
    },
    {	// GPRC_and_GPRTC
      0,	// sub_32
    },
    {	// GPRX0
      0,	// sub_32
    },
    {	// SP
      0,	// sub_32
    },
    {	// FPR64
      11,	// sub_32 -> FPR64
    },
    {	// FPR64C
      12,	// sub_32 -> FPR64C
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
const RegClassWeight &RISCVGenRegisterInfo::
getRegClassWeight(const TargetRegisterClass *RC) const {
  static const RegClassWeight RCWeightTable[] = {
    {1, 32},  	// FPR32
    {1, 32},  	// GPR
    {1, 31},  	// GPRNoX0
    {1, 30},  	// GPRNoX0X2
    {1, 15},  	// GPRTC
    {1, 8},  	// FPR32C
    {1, 8},  	// GPRC
    {1, 6},  	// GPRC_and_GPRTC
    {1, 1},  	// GPRX0
    {1, 1},  	// SP
    {1, 32},  	// FPR64
    {1, 8},  	// FPR64C
  };
  return RCWeightTable[RC->getID()];
}

/// Get the weight in units of pressure for this register unit.
unsigned RISCVGenRegisterInfo::
getRegUnitWeight(unsigned RegUnit) const {
  assert(RegUnit < 64 && "invalid register unit");
  // All register units have unit weight.
  return 1;
}


// Get the number of dimensions of register pressure.
unsigned RISCVGenRegisterInfo::getNumRegPressureSets() const {
  return 7;
}

// Get the name of this register unit pressure set.
const char *RISCVGenRegisterInfo::
getRegPressureSetName(unsigned Idx) const {
  static const char *const PressureNameTable[] = {
    "GPRX0",
    "SP",
    "FPR32C",
    "GPRC",
    "GPRTC",
    "FPR32",
    "GPR",
  };
  return PressureNameTable[Idx];
}

// Get the register unit pressure limit for this dimension.
// This limit must be adjusted dynamically for reserved registers.
unsigned RISCVGenRegisterInfo::
getRegPressureSetLimit(const MachineFunction &MF, unsigned Idx) const {
  static const uint8_t PressureLimitTable[] = {
    1,  	// 0: GPRX0
    1,  	// 1: SP
    8,  	// 2: FPR32C
    8,  	// 3: GPRC
    17,  	// 4: GPRTC
    32,  	// 5: FPR32
    32,  	// 6: GPR
  };
  return PressureLimitTable[Idx];
}

/// Table of pressure sets per register class or unit.
static const int RCSetsTable[] = {
  /* 0 */ 2, 5, -1,
  /* 3 */ 0, 6, -1,
  /* 6 */ 1, 6, -1,
  /* 9 */ 3, 4, 6, -1,
};

/// Get the dimensions of register pressure impacted by this register class.
/// Returns a -1 terminated array of pressure set IDs
const int* RISCVGenRegisterInfo::
getRegClassPressureSets(const TargetRegisterClass *RC) const {
  static const uint8_t RCSetStartTable[] = {
    1,4,4,4,10,0,9,9,3,6,1,0,};
  return &RCSetsTable[RCSetStartTable[RC->getID()]];
}

/// Get the dimensions of register pressure impacted by this register unit.
/// Returns a -1 terminated array of pressure set IDs
const int* RISCVGenRegisterInfo::
getRegUnitPressureSets(unsigned RegUnit) const {
  assert(RegUnit < 64 && "invalid register unit");
  static const uint8_t RUSetStartTable[] = {
    3,4,6,4,4,10,10,10,9,9,9,9,9,9,9,9,10,10,4,4,4,4,4,4,4,4,4,4,10,10,10,10,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,};
  return &RCSetsTable[RUSetStartTable[RegUnit]];
}

extern const MCRegisterDesc RISCVRegDesc[];
extern const MCPhysReg RISCVRegDiffLists[];
extern const LaneBitmask RISCVLaneMaskLists[];
extern const char RISCVRegStrings[];
extern const char RISCVRegClassStrings[];
extern const MCPhysReg RISCVRegUnitRoots[][2];
extern const uint16_t RISCVSubRegIdxLists[];
extern const MCRegisterInfo::SubRegCoveredBits RISCVSubRegIdxRanges[];
extern const uint16_t RISCVRegEncodingTable[];
// RISCV Dwarf<->LLVM register mappings.
extern const MCRegisterInfo::DwarfLLVMRegPair RISCVDwarfFlavour0Dwarf2L[];
extern const unsigned RISCVDwarfFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVEHFlavour0Dwarf2L[];
extern const unsigned RISCVEHFlavour0Dwarf2LSize;

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVDwarfFlavour0L2Dwarf[];
extern const unsigned RISCVDwarfFlavour0L2DwarfSize;

extern const MCRegisterInfo::DwarfLLVMRegPair RISCVEHFlavour0L2Dwarf[];
extern const unsigned RISCVEHFlavour0L2DwarfSize;

RISCVGenRegisterInfo::
RISCVGenRegisterInfo(unsigned RA, unsigned DwarfFlavour, unsigned EHFlavour,
      unsigned PC, unsigned HwMode)
  : TargetRegisterInfo(RISCVRegInfoDesc, RegisterClasses, RegisterClasses+12,
             SubRegIndexNameTable, SubRegIndexLaneMaskTable,
             LaneBitmask(0xFFFFFFFE), RegClassInfos, HwMode) {
  InitMCRegisterInfo(RISCVRegDesc, 97, RA, PC,
                     RISCVMCRegisterClasses, 12,
                     RISCVRegUnitRoots,
                     64,
                     RISCVRegDiffLists,
                     RISCVLaneMaskLists,
                     RISCVRegStrings,
                     RISCVRegClassStrings,
                     RISCVSubRegIdxLists,
                     2,
                     RISCVSubRegIdxRanges,
                     RISCVRegEncodingTable);

  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(RISCVDwarfFlavour0Dwarf2L, RISCVDwarfFlavour0Dwarf2LSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapDwarfRegsToLLVMRegs(RISCVEHFlavour0Dwarf2L, RISCVEHFlavour0Dwarf2LSize, true);
    break;
  }
  switch (DwarfFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(RISCVDwarfFlavour0L2Dwarf, RISCVDwarfFlavour0L2DwarfSize, false);
    break;
  }
  switch (EHFlavour) {
  default:
    llvm_unreachable("Unknown DWARF flavour");
  case 0:
    mapLLVMRegsToDwarfRegs(RISCVEHFlavour0L2Dwarf, RISCVEHFlavour0L2DwarfSize, true);
    break;
  }
}

static const MCPhysReg CSR_ILP32D_LP64D_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::F8_D, RISCV::F9_D, RISCV::F18_D, RISCV::F19_D, RISCV::F20_D, RISCV::F21_D, RISCV::F22_D, RISCV::F23_D, RISCV::F24_D, RISCV::F25_D, RISCV::F26_D, RISCV::F27_D, 0 };
static const uint32_t CSR_ILP32D_LP64D_RegMask[] = { 0x1ff80634, 0x1ff80600, 0x1ff80600, 0x00000000, };
static const MCPhysReg CSR_ILP32F_LP64F_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::F8_F, RISCV::F9_F, RISCV::F18_F, RISCV::F19_F, RISCV::F20_F, RISCV::F21_F, RISCV::F22_F, RISCV::F23_F, RISCV::F24_F, RISCV::F25_F, RISCV::F26_F, RISCV::F27_F, 0 };
static const uint32_t CSR_ILP32F_LP64F_RegMask[] = { 0x1ff80634, 0x00000000, 0x1ff80600, 0x00000000, };
static const MCPhysReg CSR_ILP32_LP64_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X8, RISCV::X9, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, 0 };
static const uint32_t CSR_ILP32_LP64_RegMask[] = { 0x1ff80634, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_Interrupt_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X8, RISCV::X9, RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, 0 };
static const uint32_t CSR_Interrupt_RegMask[] = { 0xfffffff4, 0x00000001, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_NoRegs_SaveList[] = { 0 };
static const uint32_t CSR_NoRegs_RegMask[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, };
static const MCPhysReg CSR_XLEN_F32_Interrupt_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X8, RISCV::X9, RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::F0_F, RISCV::F1_F, RISCV::F2_F, RISCV::F3_F, RISCV::F4_F, RISCV::F5_F, RISCV::F6_F, RISCV::F7_F, RISCV::F10_F, RISCV::F11_F, RISCV::F12_F, RISCV::F13_F, RISCV::F14_F, RISCV::F15_F, RISCV::F16_F, RISCV::F17_F, RISCV::F28_F, RISCV::F29_F, RISCV::F30_F, RISCV::F31_F, RISCV::F8_F, RISCV::F9_F, RISCV::F18_F, RISCV::F19_F, RISCV::F20_F, RISCV::F21_F, RISCV::F22_F, RISCV::F23_F, RISCV::F24_F, RISCV::F25_F, RISCV::F26_F, RISCV::F27_F, 0 };
static const uint32_t CSR_XLEN_F32_Interrupt_RegMask[] = { 0xfffffff4, 0x00000001, 0xfffffffe, 0x00000001, };
static const MCPhysReg CSR_XLEN_F64_Interrupt_SaveList[] = { RISCV::X1, RISCV::X3, RISCV::X4, RISCV::X5, RISCV::X6, RISCV::X7, RISCV::X8, RISCV::X9, RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16, RISCV::X17, RISCV::X18, RISCV::X19, RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::F0_D, RISCV::F1_D, RISCV::F2_D, RISCV::F3_D, RISCV::F4_D, RISCV::F5_D, RISCV::F6_D, RISCV::F7_D, RISCV::F10_D, RISCV::F11_D, RISCV::F12_D, RISCV::F13_D, RISCV::F14_D, RISCV::F15_D, RISCV::F16_D, RISCV::F17_D, RISCV::F28_D, RISCV::F29_D, RISCV::F30_D, RISCV::F31_D, RISCV::F8_D, RISCV::F9_D, RISCV::F18_D, RISCV::F19_D, RISCV::F20_D, RISCV::F21_D, RISCV::F22_D, RISCV::F23_D, RISCV::F24_D, RISCV::F25_D, RISCV::F26_D, RISCV::F27_D, 0 };
static const uint32_t CSR_XLEN_F64_Interrupt_RegMask[] = { 0xfffffff4, 0xffffffff, 0xffffffff, 0x00000001, };


ArrayRef<const uint32_t *> RISCVGenRegisterInfo::getRegMasks() const {
  static const uint32_t *const Masks[] = {
    CSR_ILP32D_LP64D_RegMask,
    CSR_ILP32F_LP64F_RegMask,
    CSR_ILP32_LP64_RegMask,
    CSR_Interrupt_RegMask,
    CSR_NoRegs_RegMask,
    CSR_XLEN_F32_Interrupt_RegMask,
    CSR_XLEN_F64_Interrupt_RegMask,
  };
  return makeArrayRef(Masks);
}

ArrayRef<const char *> RISCVGenRegisterInfo::getRegMaskNames() const {
  static const char *const Names[] = {
    "CSR_ILP32D_LP64D",
    "CSR_ILP32F_LP64F",
    "CSR_ILP32_LP64",
    "CSR_Interrupt",
    "CSR_NoRegs",
    "CSR_XLEN_F32_Interrupt",
    "CSR_XLEN_F64_Interrupt",
  };
  return makeArrayRef(Names);
}

const RISCVFrameLowering *
RISCVGenRegisterInfo::getFrameLowering(const MachineFunction &MF) {
  return static_cast<const RISCVFrameLowering *>(
      MF.getSubtarget().getFrameLowering());
}

} // end namespace llvm

#endif // GET_REGINFO_TARGET_DESC

