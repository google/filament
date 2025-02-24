/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*  * Mips Disassembler                                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#include "llvm/MC/MCInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/LEB128.h"
#include "llvm/Support/raw_ostream.h"
#include <assert.h>

namespace llvm {

// Helper functions for extracting fields from encoded instructions.
// InsnType must either be integral or an APInt-like object that must:
// * Have a static const max_size_in_bits equal to the number of bits in the
//   encoding.
// * be default-constructible and copy-constructible
// * be constructible from a uint64_t
// * be constructible from an APInt (this can be private)
// * Support getBitsSet(loBit, hiBit)
// * be convertible to uint64_t
// * Support the ~, &, ==, !=, and |= operators with other objects of the same type
// * Support shift (<<, >>) with signed and unsigned integers on the RHS
// * Support put (<<) to raw_ostream&
template<typename InsnType>
#if defined(_MSC_VER) && !defined(__clang__)
__declspec(noinline)
#endif
static InsnType fieldFromInstruction(InsnType insn, unsigned startBit,
                                     unsigned numBits, std::true_type) {
  assert(startBit + numBits <= 64 && "Cannot support >64-bit extractions!");
  assert(startBit + numBits <= (sizeof(InsnType) * 8) &&
         "Instruction field out of bounds!");
  InsnType fieldMask;
  if (numBits == sizeof(InsnType) * 8)
    fieldMask = (InsnType)(-1LL);
  else
    fieldMask = (((InsnType)1 << numBits) - 1) << startBit;
  return (insn & fieldMask) >> startBit;
}

template<typename InsnType>
static InsnType fieldFromInstruction(InsnType insn, unsigned startBit,
                                     unsigned numBits, std::false_type) {
  assert(startBit + numBits <= InsnType::max_size_in_bits && "Instruction field out of bounds!");
  InsnType fieldMask = InsnType::getBitsSet(0, numBits);
  return (insn >> startBit) & fieldMask;
}

template<typename InsnType>
static InsnType fieldFromInstruction(InsnType insn, unsigned startBit,
                                     unsigned numBits) {
  return fieldFromInstruction(insn, startBit, numBits, std::is_integral<InsnType>());
}

static const uint8_t DecoderTable16[] = {
/* 0 */       MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3 */       MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 17
/* 8 */       MCD::OPC_CheckPredicate, 0, 71, 2, 0, // Skip to: 596
/* 13 */      MCD::OPC_Decode, 180, 7, 0, // Opcode: Bimm16
/* 17 */      MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 31
/* 22 */      MCD::OPC_CheckPredicate, 0, 57, 2, 0, // Skip to: 596
/* 27 */      MCD::OPC_Decode, 178, 7, 1, // Opcode: BeqzRxImm16
/* 31 */      MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 45
/* 36 */      MCD::OPC_CheckPredicate, 0, 43, 2, 0, // Skip to: 596
/* 41 */      MCD::OPC_Decode, 182, 7, 1, // Opcode: BnezRxImm16
/* 45 */      MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 59
/* 50 */      MCD::OPC_CheckPredicate, 0, 29, 2, 0, // Skip to: 596
/* 55 */      MCD::OPC_Decode, 241, 5, 2, // Opcode: AddiuRxRxImm16
/* 59 */      MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 73
/* 64 */      MCD::OPC_CheckPredicate, 0, 15, 2, 0, // Skip to: 596
/* 69 */      MCD::OPC_Decode, 216, 20, 1, // Opcode: SltiRxImm16
/* 73 */      MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 87
/* 78 */      MCD::OPC_CheckPredicate, 0, 1, 2, 0, // Skip to: 596
/* 83 */      MCD::OPC_Decode, 218, 20, 1, // Opcode: SltiuRxImm16
/* 87 */      MCD::OPC_FilterValue, 12, 73, 0, 0, // Skip to: 165
/* 92 */      MCD::OPC_ExtractField, 8, 3,  // Inst{10-8} ...
/* 95 */      MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 109
/* 100 */     MCD::OPC_CheckPredicate, 0, 235, 1, 0, // Skip to: 596
/* 105 */     MCD::OPC_Decode, 185, 7, 0, // Opcode: Bteqz16
/* 109 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 123
/* 114 */     MCD::OPC_CheckPredicate, 0, 221, 1, 0, // Skip to: 596
/* 119 */     MCD::OPC_Decode, 187, 7, 0, // Opcode: Btnez16
/* 123 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 137
/* 128 */     MCD::OPC_CheckPredicate, 0, 207, 1, 0, // Skip to: 596
/* 133 */     MCD::OPC_Decode, 244, 5, 0, // Opcode: AddiuSpImm16
/* 137 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 151
/* 142 */     MCD::OPC_CheckPredicate, 0, 193, 1, 0, // Skip to: 596
/* 147 */     MCD::OPC_Decode, 224, 16, 3, // Opcode: Move32R16
/* 151 */     MCD::OPC_FilterValue, 7, 184, 1, 0, // Skip to: 596
/* 156 */     MCD::OPC_CheckPredicate, 0, 179, 1, 0, // Skip to: 596
/* 161 */     MCD::OPC_Decode, 225, 16, 4, // Opcode: MoveR3216
/* 165 */     MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 179
/* 170 */     MCD::OPC_CheckPredicate, 0, 165, 1, 0, // Skip to: 596
/* 175 */     MCD::OPC_Decode, 179, 14, 1, // Opcode: LiRxImm16
/* 179 */     MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 193
/* 184 */     MCD::OPC_CheckPredicate, 0, 151, 1, 0, // Skip to: 596
/* 189 */     MCD::OPC_Decode, 249, 9, 1, // Opcode: CmpiRxImm16
/* 193 */     MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 207
/* 198 */     MCD::OPC_CheckPredicate, 0, 137, 1, 0, // Skip to: 596
/* 203 */     MCD::OPC_Decode, 182, 14, 1, // Opcode: LwRxPcTcp16
/* 207 */     MCD::OPC_FilterValue, 28, 31, 0, 0, // Skip to: 243
/* 212 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 215 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 229
/* 220 */     MCD::OPC_CheckPredicate, 0, 115, 1, 0, // Skip to: 596
/* 225 */     MCD::OPC_Decode, 246, 5, 5, // Opcode: AdduRxRyRz16
/* 229 */     MCD::OPC_FilterValue, 3, 106, 1, 0, // Skip to: 596
/* 234 */     MCD::OPC_CheckPredicate, 0, 101, 1, 0, // Skip to: 596
/* 239 */     MCD::OPC_Decode, 225, 20, 5, // Opcode: SubuRxRyRz16
/* 243 */     MCD::OPC_FilterValue, 29, 92, 1, 0, // Skip to: 596
/* 248 */     MCD::OPC_ExtractField, 0, 5,  // Inst{4-0} ...
/* 251 */     MCD::OPC_FilterValue, 0, 73, 0, 0, // Skip to: 329
/* 256 */     MCD::OPC_ExtractField, 5, 3,  // Inst{7-5} ...
/* 259 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 273
/* 264 */     MCD::OPC_CheckPredicate, 0, 71, 1, 0, // Skip to: 596
/* 269 */     MCD::OPC_Decode, 186, 13, 0, // Opcode: JumpLinkReg16
/* 273 */     MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 294
/* 278 */     MCD::OPC_CheckPredicate, 0, 57, 1, 0, // Skip to: 596
/* 283 */     MCD::OPC_CheckField, 8, 3, 0, 50, 1, 0, // Skip to: 596
/* 290 */     MCD::OPC_Decode, 183, 13, 0, // Opcode: JrRa16
/* 294 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 308
/* 299 */     MCD::OPC_CheckPredicate, 0, 36, 1, 0, // Skip to: 596
/* 304 */     MCD::OPC_Decode, 185, 13, 1, // Opcode: JrcRx16
/* 308 */     MCD::OPC_FilterValue, 7, 27, 1, 0, // Skip to: 596
/* 313 */     MCD::OPC_CheckPredicate, 0, 22, 1, 0, // Skip to: 596
/* 318 */     MCD::OPC_CheckField, 8, 3, 0, 15, 1, 0, // Skip to: 596
/* 325 */     MCD::OPC_Decode, 184, 13, 0, // Opcode: JrcRa16
/* 329 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 343
/* 334 */     MCD::OPC_CheckPredicate, 0, 1, 1, 0, // Skip to: 596
/* 339 */     MCD::OPC_Decode, 215, 20, 6, // Opcode: SltRxRy16
/* 343 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 357
/* 348 */     MCD::OPC_CheckPredicate, 0, 243, 0, 0, // Skip to: 596
/* 353 */     MCD::OPC_Decode, 220, 20, 6, // Opcode: SltuRxRy16
/* 357 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 371
/* 362 */     MCD::OPC_CheckPredicate, 0, 229, 0, 0, // Skip to: 596
/* 367 */     MCD::OPC_Decode, 214, 20, 7, // Opcode: SllvRxRy16
/* 371 */     MCD::OPC_FilterValue, 5, 16, 0, 0, // Skip to: 392
/* 376 */     MCD::OPC_CheckPredicate, 0, 215, 0, 0, // Skip to: 596
/* 381 */     MCD::OPC_CheckField, 5, 6, 0, 208, 0, 0, // Skip to: 596
/* 388 */     MCD::OPC_Decode, 184, 7, 0, // Opcode: Break16
/* 392 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 406
/* 397 */     MCD::OPC_CheckPredicate, 0, 194, 0, 0, // Skip to: 596
/* 402 */     MCD::OPC_Decode, 224, 20, 7, // Opcode: SrlvRxRy16
/* 406 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 420
/* 411 */     MCD::OPC_CheckPredicate, 0, 180, 0, 0, // Skip to: 596
/* 416 */     MCD::OPC_Decode, 222, 20, 7, // Opcode: SravRxRy16
/* 420 */     MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 434
/* 425 */     MCD::OPC_CheckPredicate, 0, 166, 0, 0, // Skip to: 596
/* 430 */     MCD::OPC_Decode, 248, 9, 6, // Opcode: CmpRxRy16
/* 434 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 448
/* 439 */     MCD::OPC_CheckPredicate, 0, 152, 0, 0, // Skip to: 596
/* 444 */     MCD::OPC_Decode, 247, 5, 7, // Opcode: AndRxRxRy16
/* 448 */     MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 462
/* 453 */     MCD::OPC_CheckPredicate, 0, 138, 0, 0, // Skip to: 596
/* 458 */     MCD::OPC_Decode, 138, 17, 7, // Opcode: OrRxRxRy16
/* 462 */     MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 476
/* 467 */     MCD::OPC_CheckPredicate, 0, 124, 0, 0, // Skip to: 596
/* 472 */     MCD::OPC_Decode, 189, 21, 7, // Opcode: XorRxRxRy16
/* 476 */     MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 490
/* 481 */     MCD::OPC_CheckPredicate, 0, 110, 0, 0, // Skip to: 596
/* 486 */     MCD::OPC_Decode, 253, 16, 6, // Opcode: NotRxRy16
/* 490 */     MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 504
/* 495 */     MCD::OPC_CheckPredicate, 0, 96, 0, 0, // Skip to: 596
/* 500 */     MCD::OPC_Decode, 222, 16, 1, // Opcode: Mfhi16
/* 504 */     MCD::OPC_FilterValue, 17, 31, 0, 0, // Skip to: 540
/* 509 */     MCD::OPC_ExtractField, 5, 3,  // Inst{7-5} ...
/* 512 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 526
/* 517 */     MCD::OPC_CheckPredicate, 0, 74, 0, 0, // Skip to: 596
/* 522 */     MCD::OPC_Decode, 210, 20, 2, // Opcode: SebRx16
/* 526 */     MCD::OPC_FilterValue, 5, 65, 0, 0, // Skip to: 596
/* 531 */     MCD::OPC_CheckPredicate, 0, 60, 0, 0, // Skip to: 596
/* 536 */     MCD::OPC_Decode, 211, 20, 2, // Opcode: SehRx16
/* 540 */     MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 554
/* 545 */     MCD::OPC_CheckPredicate, 0, 46, 0, 0, // Skip to: 596
/* 550 */     MCD::OPC_Decode, 223, 16, 1, // Opcode: Mflo16
/* 554 */     MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 568
/* 559 */     MCD::OPC_CheckPredicate, 0, 32, 0, 0, // Skip to: 596
/* 564 */     MCD::OPC_Decode, 130, 11, 6, // Opcode: DivRxRy16
/* 568 */     MCD::OPC_FilterValue, 27, 9, 0, 0, // Skip to: 582
/* 573 */     MCD::OPC_CheckPredicate, 0, 18, 0, 0, // Skip to: 596
/* 578 */     MCD::OPC_Decode, 131, 11, 6, // Opcode: DivuRxRy16
/* 582 */     MCD::OPC_FilterValue, 29, 9, 0, 0, // Skip to: 596
/* 587 */     MCD::OPC_CheckPredicate, 0, 4, 0, 0, // Skip to: 596
/* 592 */     MCD::OPC_Decode, 252, 16, 6, // Opcode: NegRxRy16
/* 596 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTable32[] = {
/* 0 */       MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3 */       MCD::OPC_FilterValue, 1, 23, 0, 0, // Skip to: 31
/* 8 */       MCD::OPC_CheckPredicate, 0, 2, 2, 0, // Skip to: 527
/* 13 */      MCD::OPC_CheckField, 27, 5, 30, 251, 1, 0, // Skip to: 527
/* 20 */      MCD::OPC_CheckField, 5, 3, 0, 244, 1, 0, // Skip to: 527
/* 27 */      MCD::OPC_Decode, 240, 5, 1, // Opcode: AddiuRxPcImmX16
/* 31 */      MCD::OPC_FilterValue, 2, 23, 0, 0, // Skip to: 59
/* 36 */      MCD::OPC_CheckPredicate, 0, 230, 1, 0, // Skip to: 527
/* 41 */      MCD::OPC_CheckField, 27, 5, 30, 223, 1, 0, // Skip to: 527
/* 48 */      MCD::OPC_CheckField, 5, 6, 0, 216, 1, 0, // Skip to: 527
/* 55 */      MCD::OPC_Decode, 181, 7, 8, // Opcode: BimmX16
/* 59 */      MCD::OPC_FilterValue, 4, 23, 0, 0, // Skip to: 87
/* 64 */      MCD::OPC_CheckPredicate, 0, 202, 1, 0, // Skip to: 527
/* 69 */      MCD::OPC_CheckField, 27, 5, 30, 195, 1, 0, // Skip to: 527
/* 76 */      MCD::OPC_CheckField, 5, 3, 0, 188, 1, 0, // Skip to: 527
/* 83 */      MCD::OPC_Decode, 179, 7, 1, // Opcode: BeqzRxImmX16
/* 87 */      MCD::OPC_FilterValue, 5, 23, 0, 0, // Skip to: 115
/* 92 */      MCD::OPC_CheckPredicate, 0, 174, 1, 0, // Skip to: 527
/* 97 */      MCD::OPC_CheckField, 27, 5, 30, 167, 1, 0, // Skip to: 527
/* 104 */     MCD::OPC_CheckField, 5, 3, 0, 160, 1, 0, // Skip to: 527
/* 111 */     MCD::OPC_Decode, 183, 7, 1, // Opcode: BnezRxImmX16
/* 115 */     MCD::OPC_FilterValue, 6, 106, 0, 0, // Skip to: 226
/* 120 */     MCD::OPC_ExtractField, 27, 5,  // Inst{31-27} ...
/* 123 */     MCD::OPC_FilterValue, 30, 143, 1, 0, // Skip to: 527
/* 128 */     MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 131 */     MCD::OPC_FilterValue, 0, 45, 0, 0, // Skip to: 181
/* 136 */     MCD::OPC_ExtractField, 0, 5,  // Inst{4-0} ...
/* 139 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 153
/* 144 */     MCD::OPC_CheckPredicate, 0, 32, 0, 0, // Skip to: 181
/* 149 */     MCD::OPC_Decode, 213, 20, 6, // Opcode: SllX16
/* 153 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 167
/* 158 */     MCD::OPC_CheckPredicate, 0, 18, 0, 0, // Skip to: 181
/* 163 */     MCD::OPC_Decode, 223, 20, 6, // Opcode: SrlX16
/* 167 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 181
/* 172 */     MCD::OPC_CheckPredicate, 0, 4, 0, 0, // Skip to: 181
/* 177 */     MCD::OPC_Decode, 221, 20, 6, // Opcode: SraX16
/* 181 */     MCD::OPC_ExtractField, 5, 6,  // Inst{10-5} ...
/* 184 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 198
/* 189 */     MCD::OPC_CheckPredicate, 0, 77, 1, 0, // Skip to: 527
/* 194 */     MCD::OPC_Decode, 186, 7, 0, // Opcode: BteqzX16
/* 198 */     MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 212
/* 203 */     MCD::OPC_CheckPredicate, 0, 63, 1, 0, // Skip to: 527
/* 208 */     MCD::OPC_Decode, 188, 7, 0, // Opcode: BtnezX16
/* 212 */     MCD::OPC_FilterValue, 24, 54, 1, 0, // Skip to: 527
/* 217 */     MCD::OPC_CheckPredicate, 0, 49, 1, 0, // Skip to: 527
/* 222 */     MCD::OPC_Decode, 245, 5, 0, // Opcode: AddiuSpImmX16
/* 226 */     MCD::OPC_FilterValue, 8, 23, 0, 0, // Skip to: 254
/* 231 */     MCD::OPC_CheckPredicate, 0, 35, 1, 0, // Skip to: 527
/* 236 */     MCD::OPC_CheckField, 27, 5, 30, 28, 1, 0, // Skip to: 527
/* 243 */     MCD::OPC_CheckField, 4, 1, 0, 21, 1, 0, // Skip to: 527
/* 250 */     MCD::OPC_Decode, 243, 5, 9, // Opcode: AddiuRxRyOffMemX16
/* 254 */     MCD::OPC_FilterValue, 9, 23, 0, 0, // Skip to: 282
/* 259 */     MCD::OPC_CheckPredicate, 0, 7, 1, 0, // Skip to: 527
/* 264 */     MCD::OPC_CheckField, 27, 5, 30, 0, 1, 0, // Skip to: 527
/* 271 */     MCD::OPC_CheckField, 5, 3, 0, 249, 0, 0, // Skip to: 527
/* 278 */     MCD::OPC_Decode, 239, 5, 1, // Opcode: AddiuRxImmX16
/* 282 */     MCD::OPC_FilterValue, 10, 23, 0, 0, // Skip to: 310
/* 287 */     MCD::OPC_CheckPredicate, 0, 235, 0, 0, // Skip to: 527
/* 292 */     MCD::OPC_CheckField, 27, 5, 30, 228, 0, 0, // Skip to: 527
/* 299 */     MCD::OPC_CheckField, 5, 3, 0, 221, 0, 0, // Skip to: 527
/* 306 */     MCD::OPC_Decode, 217, 20, 1, // Opcode: SltiRxImmX16
/* 310 */     MCD::OPC_FilterValue, 11, 23, 0, 0, // Skip to: 338
/* 315 */     MCD::OPC_CheckPredicate, 0, 207, 0, 0, // Skip to: 527
/* 320 */     MCD::OPC_CheckField, 27, 5, 30, 200, 0, 0, // Skip to: 527
/* 327 */     MCD::OPC_CheckField, 5, 3, 0, 193, 0, 0, // Skip to: 527
/* 334 */     MCD::OPC_Decode, 219, 20, 1, // Opcode: SltiuRxImmX16
/* 338 */     MCD::OPC_FilterValue, 13, 23, 0, 0, // Skip to: 366
/* 343 */     MCD::OPC_CheckPredicate, 0, 179, 0, 0, // Skip to: 527
/* 348 */     MCD::OPC_CheckField, 27, 5, 30, 172, 0, 0, // Skip to: 527
/* 355 */     MCD::OPC_CheckField, 5, 3, 0, 165, 0, 0, // Skip to: 527
/* 362 */     MCD::OPC_Decode, 181, 14, 1, // Opcode: LiRxImmX16
/* 366 */     MCD::OPC_FilterValue, 14, 23, 0, 0, // Skip to: 394
/* 371 */     MCD::OPC_CheckPredicate, 0, 151, 0, 0, // Skip to: 527
/* 376 */     MCD::OPC_CheckField, 27, 5, 30, 144, 0, 0, // Skip to: 527
/* 383 */     MCD::OPC_CheckField, 5, 3, 0, 137, 0, 0, // Skip to: 527
/* 390 */     MCD::OPC_Decode, 250, 9, 1, // Opcode: CmpiRxImmX16
/* 394 */     MCD::OPC_FilterValue, 18, 16, 0, 0, // Skip to: 415
/* 399 */     MCD::OPC_CheckPredicate, 0, 123, 0, 0, // Skip to: 527
/* 404 */     MCD::OPC_CheckField, 27, 5, 30, 116, 0, 0, // Skip to: 527
/* 411 */     MCD::OPC_Decode, 185, 14, 9, // Opcode: LwRxSpImmX16
/* 415 */     MCD::OPC_FilterValue, 22, 23, 0, 0, // Skip to: 443
/* 420 */     MCD::OPC_CheckPredicate, 0, 102, 0, 0, // Skip to: 527
/* 425 */     MCD::OPC_CheckField, 27, 5, 30, 95, 0, 0, // Skip to: 527
/* 432 */     MCD::OPC_CheckField, 5, 3, 0, 88, 0, 0, // Skip to: 527
/* 439 */     MCD::OPC_Decode, 183, 14, 1, // Opcode: LwRxPcTcpX16
/* 443 */     MCD::OPC_FilterValue, 24, 16, 0, 0, // Skip to: 464
/* 448 */     MCD::OPC_CheckPredicate, 0, 74, 0, 0, // Skip to: 527
/* 453 */     MCD::OPC_CheckField, 27, 5, 30, 67, 0, 0, // Skip to: 527
/* 460 */     MCD::OPC_Decode, 209, 20, 9, // Opcode: SbRxRyOffMemX16
/* 464 */     MCD::OPC_FilterValue, 25, 16, 0, 0, // Skip to: 485
/* 469 */     MCD::OPC_CheckPredicate, 0, 53, 0, 0, // Skip to: 527
/* 474 */     MCD::OPC_CheckField, 27, 5, 30, 46, 0, 0, // Skip to: 527
/* 481 */     MCD::OPC_Decode, 212, 20, 9, // Opcode: ShRxRyOffMemX16
/* 485 */     MCD::OPC_FilterValue, 26, 16, 0, 0, // Skip to: 506
/* 490 */     MCD::OPC_CheckPredicate, 0, 32, 0, 0, // Skip to: 527
/* 495 */     MCD::OPC_CheckField, 27, 5, 30, 25, 0, 0, // Skip to: 527
/* 502 */     MCD::OPC_Decode, 227, 20, 9, // Opcode: SwRxSpImmX16
/* 506 */     MCD::OPC_FilterValue, 27, 16, 0, 0, // Skip to: 527
/* 511 */     MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 527
/* 516 */     MCD::OPC_CheckField, 27, 5, 30, 4, 0, 0, // Skip to: 527
/* 523 */     MCD::OPC_Decode, 226, 20, 9, // Opcode: SwRxRyOffMemX16
/* 527 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableCOP3_32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 51, 9, 0, 0, // Skip to: 17
/* 8 */       MCD::OPC_CheckPredicate, 1, 46, 0, 0, // Skip to: 59
/* 13 */      MCD::OPC_Decode, 142, 14, 10, // Opcode: LWC3
/* 17 */      MCD::OPC_FilterValue, 55, 9, 0, 0, // Skip to: 31
/* 22 */      MCD::OPC_CheckPredicate, 2, 32, 0, 0, // Skip to: 59
/* 27 */      MCD::OPC_Decode, 210, 13, 10, // Opcode: LDC3
/* 31 */      MCD::OPC_FilterValue, 59, 9, 0, 0, // Skip to: 45
/* 36 */      MCD::OPC_CheckPredicate, 1, 18, 0, 0, // Skip to: 59
/* 41 */      MCD::OPC_Decode, 174, 20, 10, // Opcode: SWC3
/* 45 */      MCD::OPC_FilterValue, 63, 9, 0, 0, // Skip to: 59
/* 50 */      MCD::OPC_CheckPredicate, 2, 4, 0, 0, // Skip to: 59
/* 55 */      MCD::OPC_Decode, 170, 18, 10, // Opcode: SDC3
/* 59 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableCnMips32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 18, 31, 0, 0, // Skip to: 39
/* 8 */       MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 11 */      MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 25
/* 16 */      MCD::OPC_CheckPredicate, 3, 239, 1, 0, // Skip to: 516
/* 21 */      MCD::OPC_Decode, 168, 10, 11, // Opcode: DMFC2_OCTEON
/* 25 */      MCD::OPC_FilterValue, 5, 230, 1, 0, // Skip to: 516
/* 30 */      MCD::OPC_CheckPredicate, 3, 225, 1, 0, // Skip to: 516
/* 35 */      MCD::OPC_Decode, 176, 10, 11, // Opcode: DMTC2_OCTEON
/* 39 */      MCD::OPC_FilterValue, 28, 160, 1, 0, // Skip to: 460
/* 44 */      MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 47 */      MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 68
/* 52 */      MCD::OPC_CheckPredicate, 3, 203, 1, 0, // Skip to: 516
/* 57 */      MCD::OPC_CheckField, 6, 5, 0, 196, 1, 0, // Skip to: 516
/* 64 */      MCD::OPC_Decode, 180, 10, 12, // Opcode: DMUL
/* 68 */      MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 89
/* 73 */      MCD::OPC_CheckPredicate, 3, 182, 1, 0, // Skip to: 516
/* 78 */      MCD::OPC_CheckField, 6, 15, 0, 175, 1, 0, // Skip to: 516
/* 85 */      MCD::OPC_Decode, 165, 16, 13, // Opcode: MTM0
/* 89 */      MCD::OPC_FilterValue, 9, 16, 0, 0, // Skip to: 110
/* 94 */      MCD::OPC_CheckPredicate, 3, 161, 1, 0, // Skip to: 516
/* 99 */      MCD::OPC_CheckField, 6, 15, 0, 154, 1, 0, // Skip to: 516
/* 106 */     MCD::OPC_Decode, 168, 16, 13, // Opcode: MTP0
/* 110 */     MCD::OPC_FilterValue, 10, 16, 0, 0, // Skip to: 131
/* 115 */     MCD::OPC_CheckPredicate, 3, 140, 1, 0, // Skip to: 516
/* 120 */     MCD::OPC_CheckField, 6, 15, 0, 133, 1, 0, // Skip to: 516
/* 127 */     MCD::OPC_Decode, 169, 16, 13, // Opcode: MTP1
/* 131 */     MCD::OPC_FilterValue, 11, 16, 0, 0, // Skip to: 152
/* 136 */     MCD::OPC_CheckPredicate, 3, 119, 1, 0, // Skip to: 516
/* 141 */     MCD::OPC_CheckField, 6, 15, 0, 112, 1, 0, // Skip to: 516
/* 148 */     MCD::OPC_Decode, 170, 16, 13, // Opcode: MTP2
/* 152 */     MCD::OPC_FilterValue, 12, 16, 0, 0, // Skip to: 173
/* 157 */     MCD::OPC_CheckPredicate, 3, 98, 1, 0, // Skip to: 516
/* 162 */     MCD::OPC_CheckField, 6, 15, 0, 91, 1, 0, // Skip to: 516
/* 169 */     MCD::OPC_Decode, 166, 16, 13, // Opcode: MTM1
/* 173 */     MCD::OPC_FilterValue, 13, 16, 0, 0, // Skip to: 194
/* 178 */     MCD::OPC_CheckPredicate, 3, 77, 1, 0, // Skip to: 516
/* 183 */     MCD::OPC_CheckField, 6, 15, 0, 70, 1, 0, // Skip to: 516
/* 190 */     MCD::OPC_Decode, 167, 16, 13, // Opcode: MTM2
/* 194 */     MCD::OPC_FilterValue, 15, 16, 0, 0, // Skip to: 215
/* 199 */     MCD::OPC_CheckPredicate, 3, 56, 1, 0, // Skip to: 516
/* 204 */     MCD::OPC_CheckField, 6, 5, 0, 49, 1, 0, // Skip to: 516
/* 211 */     MCD::OPC_Decode, 163, 21, 12, // Opcode: VMULU
/* 215 */     MCD::OPC_FilterValue, 16, 16, 0, 0, // Skip to: 236
/* 220 */     MCD::OPC_CheckPredicate, 3, 35, 1, 0, // Skip to: 516
/* 225 */     MCD::OPC_CheckField, 6, 5, 0, 28, 1, 0, // Skip to: 516
/* 232 */     MCD::OPC_Decode, 162, 21, 12, // Opcode: VMM0
/* 236 */     MCD::OPC_FilterValue, 17, 16, 0, 0, // Skip to: 257
/* 241 */     MCD::OPC_CheckPredicate, 3, 14, 1, 0, // Skip to: 516
/* 246 */     MCD::OPC_CheckField, 6, 5, 0, 7, 1, 0, // Skip to: 516
/* 253 */     MCD::OPC_Decode, 161, 21, 12, // Opcode: V3MULU
/* 257 */     MCD::OPC_FilterValue, 40, 16, 0, 0, // Skip to: 278
/* 262 */     MCD::OPC_CheckPredicate, 3, 249, 0, 0, // Skip to: 516
/* 267 */     MCD::OPC_CheckField, 6, 5, 0, 242, 0, 0, // Skip to: 516
/* 274 */     MCD::OPC_Decode, 249, 5, 12, // Opcode: BADDu
/* 278 */     MCD::OPC_FilterValue, 42, 16, 0, 0, // Skip to: 299
/* 283 */     MCD::OPC_CheckPredicate, 3, 228, 0, 0, // Skip to: 516
/* 288 */     MCD::OPC_CheckField, 6, 5, 0, 221, 0, 0, // Skip to: 516
/* 295 */     MCD::OPC_Decode, 201, 18, 12, // Opcode: SEQ
/* 299 */     MCD::OPC_FilterValue, 43, 16, 0, 0, // Skip to: 320
/* 304 */     MCD::OPC_CheckPredicate, 3, 207, 0, 0, // Skip to: 516
/* 309 */     MCD::OPC_CheckField, 6, 5, 0, 200, 0, 0, // Skip to: 516
/* 316 */     MCD::OPC_Decode, 173, 19, 12, // Opcode: SNE
/* 320 */     MCD::OPC_FilterValue, 44, 23, 0, 0, // Skip to: 348
/* 325 */     MCD::OPC_CheckPredicate, 3, 186, 0, 0, // Skip to: 516
/* 330 */     MCD::OPC_CheckField, 16, 5, 0, 179, 0, 0, // Skip to: 516
/* 337 */     MCD::OPC_CheckField, 6, 5, 0, 172, 0, 0, // Skip to: 516
/* 344 */     MCD::OPC_Decode, 162, 17, 14, // Opcode: POP
/* 348 */     MCD::OPC_FilterValue, 45, 23, 0, 0, // Skip to: 376
/* 353 */     MCD::OPC_CheckPredicate, 3, 158, 0, 0, // Skip to: 516
/* 358 */     MCD::OPC_CheckField, 16, 5, 0, 151, 0, 0, // Skip to: 516
/* 365 */     MCD::OPC_CheckField, 6, 5, 0, 144, 0, 0, // Skip to: 516
/* 372 */     MCD::OPC_Decode, 213, 10, 15, // Opcode: DPOP
/* 376 */     MCD::OPC_FilterValue, 46, 9, 0, 0, // Skip to: 390
/* 381 */     MCD::OPC_CheckPredicate, 3, 130, 0, 0, // Skip to: 516
/* 386 */     MCD::OPC_Decode, 202, 18, 16, // Opcode: SEQi
/* 390 */     MCD::OPC_FilterValue, 47, 9, 0, 0, // Skip to: 404
/* 395 */     MCD::OPC_CheckPredicate, 3, 116, 0, 0, // Skip to: 516
/* 400 */     MCD::OPC_Decode, 174, 19, 16, // Opcode: SNEi
/* 404 */     MCD::OPC_FilterValue, 50, 9, 0, 0, // Skip to: 418
/* 409 */     MCD::OPC_CheckPredicate, 4, 102, 0, 0, // Skip to: 516
/* 414 */     MCD::OPC_Decode, 218, 7, 17, // Opcode: CINS
/* 418 */     MCD::OPC_FilterValue, 51, 9, 0, 0, // Skip to: 432
/* 423 */     MCD::OPC_CheckPredicate, 4, 88, 0, 0, // Skip to: 516
/* 428 */     MCD::OPC_Decode, 219, 7, 17, // Opcode: CINS32
/* 432 */     MCD::OPC_FilterValue, 58, 9, 0, 0, // Skip to: 446
/* 437 */     MCD::OPC_CheckPredicate, 4, 74, 0, 0, // Skip to: 516
/* 442 */     MCD::OPC_Decode, 172, 11, 17, // Opcode: EXTS
/* 446 */     MCD::OPC_FilterValue, 59, 65, 0, 0, // Skip to: 516
/* 451 */     MCD::OPC_CheckPredicate, 4, 60, 0, 0, // Skip to: 516
/* 456 */     MCD::OPC_Decode, 173, 11, 17, // Opcode: EXTS32
/* 460 */     MCD::OPC_FilterValue, 50, 9, 0, 0, // Skip to: 474
/* 465 */     MCD::OPC_CheckPredicate, 3, 46, 0, 0, // Skip to: 516
/* 470 */     MCD::OPC_Decode, 255, 5, 18, // Opcode: BBIT0
/* 474 */     MCD::OPC_FilterValue, 54, 9, 0, 0, // Skip to: 488
/* 479 */     MCD::OPC_CheckPredicate, 3, 32, 0, 0, // Skip to: 516
/* 484 */     MCD::OPC_Decode, 128, 6, 18, // Opcode: BBIT032
/* 488 */     MCD::OPC_FilterValue, 58, 9, 0, 0, // Skip to: 502
/* 493 */     MCD::OPC_CheckPredicate, 3, 18, 0, 0, // Skip to: 516
/* 498 */     MCD::OPC_Decode, 129, 6, 18, // Opcode: BBIT1
/* 502 */     MCD::OPC_FilterValue, 62, 9, 0, 0, // Skip to: 516
/* 507 */     MCD::OPC_CheckPredicate, 3, 4, 0, 0, // Skip to: 516
/* 512 */     MCD::OPC_Decode, 130, 6, 18, // Opcode: BBIT132
/* 516 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableCnMipsP32[] = {
/* 0 */       MCD::OPC_ExtractField, 0, 16,  // Inst{15-0} ...
/* 3 */       MCD::OPC_FilterValue, 24, 16, 0, 0, // Skip to: 24
/* 8 */       MCD::OPC_CheckPredicate, 5, 32, 0, 0, // Skip to: 45
/* 13 */      MCD::OPC_CheckField, 26, 6, 28, 25, 0, 0, // Skip to: 45
/* 20 */      MCD::OPC_Decode, 128, 18, 19, // Opcode: SAA
/* 24 */      MCD::OPC_FilterValue, 25, 16, 0, 0, // Skip to: 45
/* 29 */      MCD::OPC_CheckPredicate, 5, 11, 0, 0, // Skip to: 45
/* 34 */      MCD::OPC_CheckField, 26, 6, 28, 4, 0, 0, // Skip to: 45
/* 41 */      MCD::OPC_Decode, 129, 18, 19, // Opcode: SAAD
/* 45 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMips16[] = {
/* 0 */       MCD::OPC_ExtractField, 10, 6,  // Inst{15-10} ...
/* 3 */       MCD::OPC_FilterValue, 1, 31, 0, 0, // Skip to: 39
/* 8 */       MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 25
/* 16 */      MCD::OPC_CheckPredicate, 6, 114, 2, 0, // Skip to: 647
/* 21 */      MCD::OPC_Decode, 154, 5, 20, // Opcode: ADDU16_MM
/* 25 */      MCD::OPC_FilterValue, 1, 105, 2, 0, // Skip to: 647
/* 30 */      MCD::OPC_CheckPredicate, 6, 100, 2, 0, // Skip to: 647
/* 35 */      MCD::OPC_Decode, 135, 20, 20, // Opcode: SUBU16_MM
/* 39 */      MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 53
/* 44 */      MCD::OPC_CheckPredicate, 7, 86, 2, 0, // Skip to: 647
/* 49 */      MCD::OPC_Decode, 191, 13, 21, // Opcode: LBU16_MM
/* 53 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 67
/* 58 */      MCD::OPC_CheckPredicate, 6, 72, 2, 0, // Skip to: 647
/* 63 */      MCD::OPC_Decode, 192, 15, 22, // Opcode: MOVE16_MM
/* 67 */      MCD::OPC_FilterValue, 9, 31, 0, 0, // Skip to: 103
/* 72 */      MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 75 */      MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 89
/* 80 */      MCD::OPC_CheckPredicate, 6, 50, 2, 0, // Skip to: 647
/* 85 */      MCD::OPC_Decode, 145, 19, 23, // Opcode: SLL16_MM
/* 89 */      MCD::OPC_FilterValue, 1, 41, 2, 0, // Skip to: 647
/* 94 */      MCD::OPC_CheckPredicate, 6, 36, 2, 0, // Skip to: 647
/* 99 */      MCD::OPC_Decode, 204, 19, 23, // Opcode: SRL16_MM
/* 103 */     MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 117
/* 108 */     MCD::OPC_CheckPredicate, 7, 22, 2, 0, // Skip to: 647
/* 113 */     MCD::OPC_Decode, 231, 13, 21, // Opcode: LHU16_MM
/* 117 */     MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 131
/* 122 */     MCD::OPC_CheckPredicate, 6, 8, 2, 0, // Skip to: 647
/* 127 */     MCD::OPC_Decode, 199, 5, 24, // Opcode: ANDI16_MM
/* 131 */     MCD::OPC_FilterValue, 17, 8, 1, 0, // Skip to: 400
/* 136 */     MCD::OPC_ExtractField, 6, 4,  // Inst{9-6} ...
/* 139 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 153
/* 144 */     MCD::OPC_CheckPredicate, 6, 242, 1, 0, // Skip to: 647
/* 149 */     MCD::OPC_Decode, 250, 16, 25, // Opcode: NOT16_MM
/* 153 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 167
/* 158 */     MCD::OPC_CheckPredicate, 6, 228, 1, 0, // Skip to: 647
/* 163 */     MCD::OPC_Decode, 178, 21, 26, // Opcode: XOR16_MM
/* 167 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 181
/* 172 */     MCD::OPC_CheckPredicate, 6, 214, 1, 0, // Skip to: 647
/* 177 */     MCD::OPC_Decode, 196, 5, 26, // Opcode: AND16_MM
/* 181 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 195
/* 186 */     MCD::OPC_CheckPredicate, 6, 200, 1, 0, // Skip to: 647
/* 191 */     MCD::OPC_Decode, 255, 16, 26, // Opcode: OR16_MM
/* 195 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 209
/* 200 */     MCD::OPC_CheckPredicate, 6, 186, 1, 0, // Skip to: 647
/* 205 */     MCD::OPC_Decode, 153, 14, 27, // Opcode: LWM16_MM
/* 209 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 223
/* 214 */     MCD::OPC_CheckPredicate, 6, 172, 1, 0, // Skip to: 647
/* 219 */     MCD::OPC_Decode, 184, 20, 27, // Opcode: SWM16_MM
/* 223 */     MCD::OPC_FilterValue, 6, 31, 0, 0, // Skip to: 259
/* 228 */     MCD::OPC_ExtractField, 5, 1,  // Inst{5} ...
/* 231 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 245
/* 236 */     MCD::OPC_CheckPredicate, 6, 150, 1, 0, // Skip to: 647
/* 241 */     MCD::OPC_Decode, 169, 13, 28, // Opcode: JR16_MM
/* 245 */     MCD::OPC_FilterValue, 1, 141, 1, 0, // Skip to: 647
/* 250 */     MCD::OPC_CheckPredicate, 6, 136, 1, 0, // Skip to: 647
/* 255 */     MCD::OPC_Decode, 172, 13, 28, // Opcode: JRC16_MM
/* 259 */     MCD::OPC_FilterValue, 7, 31, 0, 0, // Skip to: 295
/* 264 */     MCD::OPC_ExtractField, 5, 1,  // Inst{5} ...
/* 267 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 281
/* 272 */     MCD::OPC_CheckPredicate, 6, 114, 1, 0, // Skip to: 647
/* 277 */     MCD::OPC_Decode, 148, 13, 28, // Opcode: JALR16_MM
/* 281 */     MCD::OPC_FilterValue, 1, 105, 1, 0, // Skip to: 647
/* 286 */     MCD::OPC_CheckPredicate, 6, 100, 1, 0, // Skip to: 647
/* 291 */     MCD::OPC_Decode, 153, 13, 28, // Opcode: JALRS16_MM
/* 295 */     MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 316
/* 300 */     MCD::OPC_CheckPredicate, 6, 86, 1, 0, // Skip to: 647
/* 305 */     MCD::OPC_CheckField, 5, 1, 0, 79, 1, 0, // Skip to: 647
/* 312 */     MCD::OPC_Decode, 138, 15, 28, // Opcode: MFHI16_MM
/* 316 */     MCD::OPC_FilterValue, 9, 16, 0, 0, // Skip to: 337
/* 321 */     MCD::OPC_CheckPredicate, 6, 65, 1, 0, // Skip to: 647
/* 326 */     MCD::OPC_CheckField, 5, 1, 0, 58, 1, 0, // Skip to: 647
/* 333 */     MCD::OPC_Decode, 144, 15, 28, // Opcode: MFLO16_MM
/* 337 */     MCD::OPC_FilterValue, 10, 16, 0, 0, // Skip to: 358
/* 342 */     MCD::OPC_CheckPredicate, 6, 44, 1, 0, // Skip to: 647
/* 347 */     MCD::OPC_CheckField, 4, 2, 0, 37, 1, 0, // Skip to: 647
/* 354 */     MCD::OPC_Decode, 159, 7, 29, // Opcode: BREAK16_MM
/* 358 */     MCD::OPC_FilterValue, 11, 16, 0, 0, // Skip to: 379
/* 363 */     MCD::OPC_CheckPredicate, 6, 23, 1, 0, // Skip to: 647
/* 368 */     MCD::OPC_CheckField, 4, 2, 0, 16, 1, 0, // Skip to: 647
/* 375 */     MCD::OPC_Decode, 158, 18, 29, // Opcode: SDBBP16_MM
/* 379 */     MCD::OPC_FilterValue, 12, 7, 1, 0, // Skip to: 647
/* 384 */     MCD::OPC_CheckPredicate, 6, 2, 1, 0, // Skip to: 647
/* 389 */     MCD::OPC_CheckField, 5, 1, 0, 251, 0, 0, // Skip to: 647
/* 396 */     MCD::OPC_Decode, 171, 13, 30, // Opcode: JRADDIUSP
/* 400 */     MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 414
/* 405 */     MCD::OPC_CheckPredicate, 7, 237, 0, 0, // Skip to: 647
/* 410 */     MCD::OPC_Decode, 164, 14, 31, // Opcode: LWSP_MM
/* 414 */     MCD::OPC_FilterValue, 19, 31, 0, 0, // Skip to: 450
/* 419 */     MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 422 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 436
/* 427 */     MCD::OPC_CheckPredicate, 7, 215, 0, 0, // Skip to: 647
/* 432 */     MCD::OPC_Decode, 251, 4, 32, // Opcode: ADDIUS5_MM
/* 436 */     MCD::OPC_FilterValue, 1, 206, 0, 0, // Skip to: 647
/* 441 */     MCD::OPC_CheckPredicate, 7, 201, 0, 0, // Skip to: 647
/* 446 */     MCD::OPC_Decode, 252, 4, 33, // Opcode: ADDIUSP_MM
/* 450 */     MCD::OPC_FilterValue, 25, 9, 0, 0, // Skip to: 464
/* 455 */     MCD::OPC_CheckPredicate, 7, 187, 0, 0, // Skip to: 647
/* 460 */     MCD::OPC_Decode, 147, 14, 34, // Opcode: LWGP_MM
/* 464 */     MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 478
/* 469 */     MCD::OPC_CheckPredicate, 7, 173, 0, 0, // Skip to: 647
/* 474 */     MCD::OPC_Decode, 135, 14, 21, // Opcode: LW16_MM
/* 478 */     MCD::OPC_FilterValue, 27, 31, 0, 0, // Skip to: 514
/* 483 */     MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 486 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 500
/* 491 */     MCD::OPC_CheckPredicate, 7, 151, 0, 0, // Skip to: 647
/* 496 */     MCD::OPC_Decode, 250, 4, 35, // Opcode: ADDIUR2_MM
/* 500 */     MCD::OPC_FilterValue, 1, 142, 0, 0, // Skip to: 647
/* 505 */     MCD::OPC_CheckPredicate, 7, 137, 0, 0, // Skip to: 647
/* 510 */     MCD::OPC_Decode, 249, 4, 36, // Opcode: ADDIUR1SP_MM
/* 514 */     MCD::OPC_FilterValue, 33, 16, 0, 0, // Skip to: 535
/* 519 */     MCD::OPC_CheckPredicate, 6, 123, 0, 0, // Skip to: 647
/* 524 */     MCD::OPC_CheckField, 0, 1, 0, 116, 0, 0, // Skip to: 647
/* 531 */     MCD::OPC_Decode, 194, 15, 37, // Opcode: MOVEP_MM
/* 535 */     MCD::OPC_FilterValue, 34, 9, 0, 0, // Skip to: 549
/* 540 */     MCD::OPC_CheckPredicate, 6, 102, 0, 0, // Skip to: 647
/* 545 */     MCD::OPC_Decode, 139, 18, 21, // Opcode: SB16_MM
/* 549 */     MCD::OPC_FilterValue, 35, 9, 0, 0, // Skip to: 563
/* 554 */     MCD::OPC_CheckPredicate, 6, 88, 0, 0, // Skip to: 647
/* 559 */     MCD::OPC_Decode, 162, 6, 38, // Opcode: BEQZ16_MM
/* 563 */     MCD::OPC_FilterValue, 42, 9, 0, 0, // Skip to: 577
/* 568 */     MCD::OPC_CheckPredicate, 6, 74, 0, 0, // Skip to: 647
/* 573 */     MCD::OPC_Decode, 204, 18, 21, // Opcode: SH16_MM
/* 577 */     MCD::OPC_FilterValue, 43, 9, 0, 0, // Skip to: 591
/* 582 */     MCD::OPC_CheckPredicate, 6, 60, 0, 0, // Skip to: 647
/* 587 */     MCD::OPC_Decode, 137, 7, 38, // Opcode: BNEZ16_MM
/* 591 */     MCD::OPC_FilterValue, 50, 9, 0, 0, // Skip to: 605
/* 596 */     MCD::OPC_CheckPredicate, 6, 46, 0, 0, // Skip to: 647
/* 601 */     MCD::OPC_Decode, 193, 20, 31, // Opcode: SWSP_MM
/* 605 */     MCD::OPC_FilterValue, 51, 9, 0, 0, // Skip to: 619
/* 610 */     MCD::OPC_CheckPredicate, 7, 32, 0, 0, // Skip to: 647
/* 615 */     MCD::OPC_Decode, 248, 5, 39, // Opcode: B16_MM
/* 619 */     MCD::OPC_FilterValue, 58, 9, 0, 0, // Skip to: 633
/* 624 */     MCD::OPC_CheckPredicate, 6, 18, 0, 0, // Skip to: 647
/* 629 */     MCD::OPC_Decode, 166, 20, 21, // Opcode: SW16_MM
/* 633 */     MCD::OPC_FilterValue, 59, 9, 0, 0, // Skip to: 647
/* 638 */     MCD::OPC_CheckPredicate, 6, 4, 0, 0, // Skip to: 647
/* 643 */     MCD::OPC_Decode, 240, 13, 40, // Opcode: LI16_MM
/* 647 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMips32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 238, 14, 0, // Skip to: 3830
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 104, 0, 0, // Skip to: 120
/* 16 */      MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 19 */      MCD::OPC_FilterValue, 0, 54, 0, 0, // Skip to: 78
/* 24 */      MCD::OPC_ExtractField, 11, 15,  // Inst{25-11} ...
/* 27 */      MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 41
/* 32 */      MCD::OPC_CheckPredicate, 7, 32, 0, 0, // Skip to: 69
/* 37 */      MCD::OPC_Decode, 226, 19, 0, // Opcode: SSNOP_MM
/* 41 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 55
/* 46 */      MCD::OPC_CheckPredicate, 7, 18, 0, 0, // Skip to: 69
/* 51 */      MCD::OPC_Decode, 133, 11, 0, // Opcode: EHB_MM
/* 55 */      MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 69
/* 60 */      MCD::OPC_CheckPredicate, 7, 4, 0, 0, // Skip to: 69
/* 65 */      MCD::OPC_Decode, 142, 17, 0, // Opcode: PAUSE_MM
/* 69 */      MCD::OPC_CheckPredicate, 7, 42, 25, 0, // Skip to: 6516
/* 74 */      MCD::OPC_Decode, 158, 19, 41, // Opcode: SLL_MM
/* 78 */      MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 92
/* 83 */      MCD::OPC_CheckPredicate, 7, 28, 25, 0, // Skip to: 6516
/* 88 */      MCD::OPC_Decode, 223, 19, 41, // Opcode: SRL_MM
/* 92 */      MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 106
/* 97 */      MCD::OPC_CheckPredicate, 7, 14, 25, 0, // Skip to: 6516
/* 102 */     MCD::OPC_Decode, 201, 19, 41, // Opcode: SRA_MM
/* 106 */     MCD::OPC_FilterValue, 3, 5, 25, 0, // Skip to: 6516
/* 111 */     MCD::OPC_CheckPredicate, 7, 0, 25, 0, // Skip to: 6516
/* 116 */     MCD::OPC_Decode, 236, 17, 41, // Opcode: ROTR_MM
/* 120 */     MCD::OPC_FilterValue, 5, 227, 0, 0, // Skip to: 352
/* 125 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 128 */     MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 149
/* 133 */     MCD::OPC_CheckPredicate, 8, 234, 24, 0, // Skip to: 6516
/* 138 */     MCD::OPC_CheckField, 11, 5, 0, 227, 24, 0, // Skip to: 6516
/* 145 */     MCD::OPC_Decode, 161, 8, 42, // Opcode: CMP_EQ_PH_MM
/* 149 */     MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 170
/* 154 */     MCD::OPC_CheckPredicate, 8, 213, 24, 0, // Skip to: 6516
/* 159 */     MCD::OPC_CheckField, 11, 5, 0, 206, 24, 0, // Skip to: 6516
/* 166 */     MCD::OPC_Decode, 175, 8, 42, // Opcode: CMP_LT_PH_MM
/* 170 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 191
/* 175 */     MCD::OPC_CheckPredicate, 8, 192, 24, 0, // Skip to: 6516
/* 180 */     MCD::OPC_CheckField, 11, 5, 0, 185, 24, 0, // Skip to: 6516
/* 187 */     MCD::OPC_Decode, 169, 8, 42, // Opcode: CMP_LE_PH_MM
/* 191 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 205
/* 196 */     MCD::OPC_CheckPredicate, 9, 171, 24, 0, // Skip to: 6516
/* 201 */     MCD::OPC_Decode, 139, 8, 43, // Opcode: CMPGDU_EQ_QB_MMR2
/* 205 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 219
/* 210 */     MCD::OPC_CheckPredicate, 9, 157, 24, 0, // Skip to: 6516
/* 215 */     MCD::OPC_Decode, 143, 8, 43, // Opcode: CMPGDU_LT_QB_MMR2
/* 219 */     MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 233
/* 224 */     MCD::OPC_CheckPredicate, 9, 143, 24, 0, // Skip to: 6516
/* 229 */     MCD::OPC_Decode, 141, 8, 43, // Opcode: CMPGDU_LE_QB_MMR2
/* 233 */     MCD::OPC_FilterValue, 9, 16, 0, 0, // Skip to: 254
/* 238 */     MCD::OPC_CheckPredicate, 8, 129, 24, 0, // Skip to: 6516
/* 243 */     MCD::OPC_CheckField, 11, 5, 0, 122, 24, 0, // Skip to: 6516
/* 250 */     MCD::OPC_Decode, 151, 8, 42, // Opcode: CMPU_EQ_QB_MM
/* 254 */     MCD::OPC_FilterValue, 10, 16, 0, 0, // Skip to: 275
/* 259 */     MCD::OPC_CheckPredicate, 8, 108, 24, 0, // Skip to: 6516
/* 264 */     MCD::OPC_CheckField, 11, 5, 0, 101, 24, 0, // Skip to: 6516
/* 271 */     MCD::OPC_Decode, 155, 8, 42, // Opcode: CMPU_LT_QB_MM
/* 275 */     MCD::OPC_FilterValue, 11, 16, 0, 0, // Skip to: 296
/* 280 */     MCD::OPC_CheckPredicate, 8, 87, 24, 0, // Skip to: 6516
/* 285 */     MCD::OPC_CheckField, 11, 5, 0, 80, 24, 0, // Skip to: 6516
/* 292 */     MCD::OPC_Decode, 153, 8, 42, // Opcode: CMPU_LE_QB_MM
/* 296 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 310
/* 301 */     MCD::OPC_CheckPredicate, 8, 66, 24, 0, // Skip to: 6516
/* 306 */     MCD::OPC_Decode, 139, 5, 44, // Opcode: ADDQ_S_W_MM
/* 310 */     MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 324
/* 315 */     MCD::OPC_CheckPredicate, 8, 52, 24, 0, // Skip to: 6516
/* 320 */     MCD::OPC_Decode, 246, 19, 44, // Opcode: SUBQ_S_W_MM
/* 324 */     MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 338
/* 329 */     MCD::OPC_CheckPredicate, 8, 38, 24, 0, // Skip to: 6516
/* 334 */     MCD::OPC_Decode, 141, 5, 44, // Opcode: ADDSC_MM
/* 338 */     MCD::OPC_FilterValue, 15, 29, 24, 0, // Skip to: 6516
/* 343 */     MCD::OPC_CheckPredicate, 8, 24, 24, 0, // Skip to: 6516
/* 348 */     MCD::OPC_Decode, 178, 5, 44, // Opcode: ADDWC_MM
/* 352 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 366
/* 357 */     MCD::OPC_CheckPredicate, 7, 10, 24, 0, // Skip to: 6516
/* 362 */     MCD::OPC_Decode, 161, 7, 45, // Opcode: BREAK_MM
/* 366 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 380
/* 371 */     MCD::OPC_CheckPredicate, 6, 252, 23, 0, // Skip to: 6516
/* 376 */     MCD::OPC_Decode, 143, 13, 46, // Opcode: INS_MM
/* 380 */     MCD::OPC_FilterValue, 13, 167, 1, 0, // Skip to: 808
/* 385 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 388 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 402
/* 393 */     MCD::OPC_CheckPredicate, 8, 230, 23, 0, // Skip to: 6516
/* 398 */     MCD::OPC_Decode, 135, 5, 47, // Opcode: ADDQ_PH_MM
/* 402 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 416
/* 407 */     MCD::OPC_CheckPredicate, 9, 216, 23, 0, // Skip to: 6516
/* 412 */     MCD::OPC_Decode, 255, 4, 47, // Opcode: ADDQH_PH_MMR2
/* 416 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 430
/* 421 */     MCD::OPC_CheckPredicate, 9, 202, 23, 0, // Skip to: 6516
/* 426 */     MCD::OPC_Decode, 133, 5, 44, // Opcode: ADDQH_W_MMR2
/* 430 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 444
/* 435 */     MCD::OPC_CheckPredicate, 8, 188, 23, 0, // Skip to: 6516
/* 440 */     MCD::OPC_Decode, 164, 5, 47, // Opcode: ADDU_QB_MM
/* 444 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 458
/* 449 */     MCD::OPC_CheckPredicate, 9, 174, 23, 0, // Skip to: 6516
/* 454 */     MCD::OPC_Decode, 162, 5, 47, // Opcode: ADDU_PH_MMR2
/* 458 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 472
/* 463 */     MCD::OPC_CheckPredicate, 9, 160, 23, 0, // Skip to: 6516
/* 468 */     MCD::OPC_Decode, 157, 5, 47, // Opcode: ADDUH_QB_MMR2
/* 472 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 486
/* 477 */     MCD::OPC_CheckPredicate, 8, 146, 23, 0, // Skip to: 6516
/* 482 */     MCD::OPC_Decode, 233, 18, 48, // Opcode: SHRAV_PH_MM
/* 486 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 500
/* 491 */     MCD::OPC_CheckPredicate, 9, 132, 23, 0, // Skip to: 6516
/* 496 */     MCD::OPC_Decode, 235, 18, 48, // Opcode: SHRAV_QB_MMR2
/* 500 */     MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 514
/* 505 */     MCD::OPC_CheckPredicate, 8, 118, 23, 0, // Skip to: 6516
/* 510 */     MCD::OPC_Decode, 242, 19, 47, // Opcode: SUBQ_PH_MM
/* 514 */     MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 528
/* 519 */     MCD::OPC_CheckPredicate, 9, 104, 23, 0, // Skip to: 6516
/* 524 */     MCD::OPC_Decode, 234, 19, 47, // Opcode: SUBQH_PH_MMR2
/* 528 */     MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 542
/* 533 */     MCD::OPC_CheckPredicate, 9, 90, 23, 0, // Skip to: 6516
/* 538 */     MCD::OPC_Decode, 240, 19, 44, // Opcode: SUBQH_W_MMR2
/* 542 */     MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 556
/* 547 */     MCD::OPC_CheckPredicate, 8, 76, 23, 0, // Skip to: 6516
/* 552 */     MCD::OPC_Decode, 145, 20, 47, // Opcode: SUBU_QB_MM
/* 556 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 570
/* 561 */     MCD::OPC_CheckPredicate, 9, 62, 23, 0, // Skip to: 6516
/* 566 */     MCD::OPC_Decode, 143, 20, 47, // Opcode: SUBU_PH_MMR2
/* 570 */     MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 584
/* 575 */     MCD::OPC_CheckPredicate, 9, 48, 23, 0, // Skip to: 6516
/* 580 */     MCD::OPC_Decode, 138, 20, 47, // Opcode: SUBUH_QB_MMR2
/* 584 */     MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 598
/* 589 */     MCD::OPC_CheckPredicate, 9, 34, 23, 0, // Skip to: 6516
/* 594 */     MCD::OPC_Decode, 194, 17, 49, // Opcode: PRECR_SRA_PH_W_MMR2
/* 598 */     MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 612
/* 603 */     MCD::OPC_CheckPredicate, 8, 20, 23, 0, // Skip to: 6516
/* 608 */     MCD::OPC_Decode, 137, 5, 47, // Opcode: ADDQ_S_PH_MM
/* 612 */     MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 626
/* 617 */     MCD::OPC_CheckPredicate, 9, 6, 23, 0, // Skip to: 6516
/* 622 */     MCD::OPC_Decode, 129, 5, 47, // Opcode: ADDQH_R_PH_MMR2
/* 626 */     MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 640
/* 631 */     MCD::OPC_CheckPredicate, 9, 248, 22, 0, // Skip to: 6516
/* 636 */     MCD::OPC_Decode, 131, 5, 44, // Opcode: ADDQH_R_W_MMR2
/* 640 */     MCD::OPC_FilterValue, 19, 9, 0, 0, // Skip to: 654
/* 645 */     MCD::OPC_CheckPredicate, 8, 234, 22, 0, // Skip to: 6516
/* 650 */     MCD::OPC_Decode, 168, 5, 47, // Opcode: ADDU_S_QB_MM
/* 654 */     MCD::OPC_FilterValue, 20, 9, 0, 0, // Skip to: 668
/* 659 */     MCD::OPC_CheckPredicate, 9, 220, 22, 0, // Skip to: 6516
/* 664 */     MCD::OPC_Decode, 166, 5, 47, // Opcode: ADDU_S_PH_MMR2
/* 668 */     MCD::OPC_FilterValue, 21, 9, 0, 0, // Skip to: 682
/* 673 */     MCD::OPC_CheckPredicate, 9, 206, 22, 0, // Skip to: 6516
/* 678 */     MCD::OPC_Decode, 159, 5, 47, // Opcode: ADDUH_R_QB_MMR2
/* 682 */     MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 696
/* 687 */     MCD::OPC_CheckPredicate, 8, 192, 22, 0, // Skip to: 6516
/* 692 */     MCD::OPC_Decode, 237, 18, 48, // Opcode: SHRAV_R_PH_MM
/* 696 */     MCD::OPC_FilterValue, 23, 9, 0, 0, // Skip to: 710
/* 701 */     MCD::OPC_CheckPredicate, 9, 178, 22, 0, // Skip to: 6516
/* 706 */     MCD::OPC_Decode, 239, 18, 48, // Opcode: SHRAV_R_QB_MMR2
/* 710 */     MCD::OPC_FilterValue, 24, 9, 0, 0, // Skip to: 724
/* 715 */     MCD::OPC_CheckPredicate, 8, 164, 22, 0, // Skip to: 6516
/* 720 */     MCD::OPC_Decode, 244, 19, 47, // Opcode: SUBQ_S_PH_MM
/* 724 */     MCD::OPC_FilterValue, 25, 9, 0, 0, // Skip to: 738
/* 729 */     MCD::OPC_CheckPredicate, 9, 150, 22, 0, // Skip to: 6516
/* 734 */     MCD::OPC_Decode, 236, 19, 47, // Opcode: SUBQH_R_PH_MMR2
/* 738 */     MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 752
/* 743 */     MCD::OPC_CheckPredicate, 9, 136, 22, 0, // Skip to: 6516
/* 748 */     MCD::OPC_Decode, 238, 19, 44, // Opcode: SUBQH_R_W_MMR2
/* 752 */     MCD::OPC_FilterValue, 27, 9, 0, 0, // Skip to: 766
/* 757 */     MCD::OPC_CheckPredicate, 8, 122, 22, 0, // Skip to: 6516
/* 762 */     MCD::OPC_Decode, 149, 20, 47, // Opcode: SUBU_S_QB_MM
/* 766 */     MCD::OPC_FilterValue, 28, 9, 0, 0, // Skip to: 780
/* 771 */     MCD::OPC_CheckPredicate, 9, 108, 22, 0, // Skip to: 6516
/* 776 */     MCD::OPC_Decode, 147, 20, 47, // Opcode: SUBU_S_PH_MMR2
/* 780 */     MCD::OPC_FilterValue, 29, 9, 0, 0, // Skip to: 794
/* 785 */     MCD::OPC_CheckPredicate, 9, 94, 22, 0, // Skip to: 6516
/* 790 */     MCD::OPC_Decode, 140, 20, 47, // Opcode: SUBUH_R_QB_MMR2
/* 794 */     MCD::OPC_FilterValue, 31, 85, 22, 0, // Skip to: 6516
/* 799 */     MCD::OPC_CheckPredicate, 9, 80, 22, 0, // Skip to: 6516
/* 804 */     MCD::OPC_Decode, 196, 17, 49, // Opcode: PRECR_SRA_R_PH_W_MMR2
/* 808 */     MCD::OPC_FilterValue, 14, 31, 0, 0, // Skip to: 844
/* 813 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 816 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 830
/* 821 */     MCD::OPC_CheckPredicate, 8, 58, 22, 0, // Skip to: 6516
/* 826 */     MCD::OPC_Decode, 217, 18, 48, // Opcode: SHLLV_PH_MM
/* 830 */     MCD::OPC_FilterValue, 16, 49, 22, 0, // Skip to: 6516
/* 835 */     MCD::OPC_CheckPredicate, 8, 44, 22, 0, // Skip to: 6516
/* 840 */     MCD::OPC_Decode, 221, 18, 48, // Opcode: SHLLV_S_PH_MM
/* 844 */     MCD::OPC_FilterValue, 16, 213, 0, 0, // Skip to: 1062
/* 849 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 852 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 866
/* 857 */     MCD::OPC_CheckPredicate, 7, 22, 22, 0, // Skip to: 6516
/* 862 */     MCD::OPC_Decode, 154, 19, 50, // Opcode: SLLV_MM
/* 866 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 880
/* 871 */     MCD::OPC_CheckPredicate, 7, 8, 22, 0, // Skip to: 6516
/* 876 */     MCD::OPC_Decode, 219, 19, 50, // Opcode: SRLV_MM
/* 880 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 894
/* 885 */     MCD::OPC_CheckPredicate, 7, 250, 21, 0, // Skip to: 6516
/* 890 */     MCD::OPC_Decode, 197, 19, 50, // Opcode: SRAV_MM
/* 894 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 908
/* 899 */     MCD::OPC_CheckPredicate, 7, 236, 21, 0, // Skip to: 6516
/* 904 */     MCD::OPC_Decode, 235, 17, 50, // Opcode: ROTRV_MM
/* 908 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 922
/* 913 */     MCD::OPC_CheckPredicate, 6, 222, 21, 0, // Skip to: 6516
/* 918 */     MCD::OPC_Decode, 183, 5, 44, // Opcode: ADD_MM
/* 922 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 936
/* 927 */     MCD::OPC_CheckPredicate, 6, 208, 21, 0, // Skip to: 6516
/* 932 */     MCD::OPC_Decode, 190, 5, 44, // Opcode: ADDu_MM
/* 936 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 950
/* 941 */     MCD::OPC_CheckPredicate, 6, 194, 21, 0, // Skip to: 6516
/* 946 */     MCD::OPC_Decode, 158, 20, 44, // Opcode: SUB_MM
/* 950 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 964
/* 955 */     MCD::OPC_CheckPredicate, 6, 180, 21, 0, // Skip to: 6516
/* 960 */     MCD::OPC_Decode, 161, 20, 44, // Opcode: SUBu_MM
/* 964 */     MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 978
/* 969 */     MCD::OPC_CheckPredicate, 6, 166, 21, 0, // Skip to: 6516
/* 974 */     MCD::OPC_Decode, 213, 16, 44, // Opcode: MUL_MM
/* 978 */     MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 992
/* 983 */     MCD::OPC_CheckPredicate, 6, 152, 21, 0, // Skip to: 6516
/* 988 */     MCD::OPC_Decode, 203, 5, 44, // Opcode: AND_MM
/* 992 */     MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 1006
/* 997 */     MCD::OPC_CheckPredicate, 6, 138, 21, 0, // Skip to: 6516
/* 1002 */    MCD::OPC_Decode, 132, 17, 44, // Opcode: OR_MM
/* 1006 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 1020
/* 1011 */    MCD::OPC_CheckPredicate, 6, 124, 21, 0, // Skip to: 6516
/* 1016 */    MCD::OPC_Decode, 247, 16, 44, // Opcode: NOR_MM
/* 1020 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 1034
/* 1025 */    MCD::OPC_CheckPredicate, 6, 110, 21, 0, // Skip to: 6516
/* 1030 */    MCD::OPC_Decode, 183, 21, 44, // Opcode: XOR_MM
/* 1034 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 1048
/* 1039 */    MCD::OPC_CheckPredicate, 7, 96, 21, 0, // Skip to: 6516
/* 1044 */    MCD::OPC_Decode, 163, 19, 44, // Opcode: SLT_MM
/* 1048 */    MCD::OPC_FilterValue, 14, 87, 21, 0, // Skip to: 6516
/* 1053 */    MCD::OPC_CheckPredicate, 7, 82, 21, 0, // Skip to: 6516
/* 1058 */    MCD::OPC_Decode, 172, 19, 44, // Opcode: SLTu_MM
/* 1062 */    MCD::OPC_FilterValue, 21, 199, 0, 0, // Skip to: 1266
/* 1067 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1070 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1084
/* 1075 */    MCD::OPC_CheckPredicate, 8, 60, 21, 0, // Skip to: 6516
/* 1080 */    MCD::OPC_Decode, 182, 16, 47, // Opcode: MULEU_S_PH_QBL_MM
/* 1084 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 1098
/* 1089 */    MCD::OPC_CheckPredicate, 8, 46, 21, 0, // Skip to: 6516
/* 1094 */    MCD::OPC_Decode, 184, 16, 47, // Opcode: MULEU_S_PH_QBR_MM
/* 1098 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 1112
/* 1103 */    MCD::OPC_CheckPredicate, 8, 32, 21, 0, // Skip to: 6516
/* 1108 */    MCD::OPC_Decode, 186, 16, 47, // Opcode: MULQ_RS_PH_MM
/* 1112 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 1126
/* 1117 */    MCD::OPC_CheckPredicate, 9, 18, 21, 0, // Skip to: 6516
/* 1122 */    MCD::OPC_Decode, 190, 16, 47, // Opcode: MULQ_S_PH_MMR2
/* 1126 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 1140
/* 1131 */    MCD::OPC_CheckPredicate, 9, 4, 21, 0, // Skip to: 6516
/* 1136 */    MCD::OPC_Decode, 188, 16, 44, // Opcode: MULQ_RS_W_MMR2
/* 1140 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 1154
/* 1145 */    MCD::OPC_CheckPredicate, 9, 246, 20, 0, // Skip to: 6516
/* 1150 */    MCD::OPC_Decode, 192, 16, 44, // Opcode: MULQ_S_W_MMR2
/* 1154 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 1168
/* 1159 */    MCD::OPC_CheckPredicate, 9, 232, 20, 0, // Skip to: 6516
/* 1164 */    MCD::OPC_Decode, 210, 5, 51, // Opcode: APPEND_MMR2
/* 1168 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 1182
/* 1173 */    MCD::OPC_CheckPredicate, 9, 218, 20, 0, // Skip to: 6516
/* 1178 */    MCD::OPC_Decode, 205, 17, 51, // Opcode: PREPEND_MMR2
/* 1182 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 1196
/* 1187 */    MCD::OPC_CheckPredicate, 8, 204, 20, 0, // Skip to: 6516
/* 1192 */    MCD::OPC_Decode, 180, 15, 44, // Opcode: MODSUB_MM
/* 1196 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 1210
/* 1201 */    MCD::OPC_CheckPredicate, 8, 190, 20, 0, // Skip to: 6516
/* 1206 */    MCD::OPC_Decode, 241, 18, 50, // Opcode: SHRAV_R_W_MM
/* 1210 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 1224
/* 1215 */    MCD::OPC_CheckPredicate, 9, 176, 20, 0, // Skip to: 6516
/* 1220 */    MCD::OPC_Decode, 253, 18, 48, // Opcode: SHRLV_PH_MMR2
/* 1224 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 1238
/* 1229 */    MCD::OPC_CheckPredicate, 8, 162, 20, 0, // Skip to: 6516
/* 1234 */    MCD::OPC_Decode, 255, 18, 48, // Opcode: SHRLV_QB_MM
/* 1238 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 1252
/* 1243 */    MCD::OPC_CheckPredicate, 8, 148, 20, 0, // Skip to: 6516
/* 1248 */    MCD::OPC_Decode, 219, 18, 48, // Opcode: SHLLV_QB_MM
/* 1252 */    MCD::OPC_FilterValue, 15, 139, 20, 0, // Skip to: 6516
/* 1257 */    MCD::OPC_CheckPredicate, 8, 134, 20, 0, // Skip to: 6516
/* 1262 */    MCD::OPC_Decode, 223, 18, 50, // Opcode: SHLLV_S_W_MM
/* 1266 */    MCD::OPC_FilterValue, 24, 45, 0, 0, // Skip to: 1316
/* 1271 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1274 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1288
/* 1279 */    MCD::OPC_CheckPredicate, 6, 112, 20, 0, // Skip to: 6516
/* 1284 */    MCD::OPC_Decode, 214, 15, 52, // Opcode: MOVN_I_MM
/* 1288 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1302
/* 1293 */    MCD::OPC_CheckPredicate, 6, 98, 20, 0, // Skip to: 6516
/* 1298 */    MCD::OPC_Decode, 234, 15, 52, // Opcode: MOVZ_I_MM
/* 1302 */    MCD::OPC_FilterValue, 4, 89, 20, 0, // Skip to: 6516
/* 1307 */    MCD::OPC_CheckPredicate, 7, 84, 20, 0, // Skip to: 6516
/* 1312 */    MCD::OPC_Decode, 170, 14, 53, // Opcode: LWXS_MM
/* 1316 */    MCD::OPC_FilterValue, 29, 23, 0, 0, // Skip to: 1344
/* 1321 */    MCD::OPC_CheckPredicate, 8, 70, 20, 0, // Skip to: 6516
/* 1326 */    MCD::OPC_CheckField, 22, 4, 0, 63, 20, 0, // Skip to: 6516
/* 1333 */    MCD::OPC_CheckField, 6, 8, 0, 56, 20, 0, // Skip to: 6516
/* 1340 */    MCD::OPC_Decode, 215, 18, 54, // Opcode: SHILO_MM
/* 1344 */    MCD::OPC_FilterValue, 37, 73, 0, 0, // Skip to: 1422
/* 1349 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1352 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1366
/* 1357 */    MCD::OPC_CheckPredicate, 8, 34, 20, 0, // Skip to: 6516
/* 1362 */    MCD::OPC_Decode, 178, 16, 43, // Opcode: MULEQ_S_W_PHL_MM
/* 1366 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1380
/* 1371 */    MCD::OPC_CheckPredicate, 8, 20, 20, 0, // Skip to: 6516
/* 1376 */    MCD::OPC_Decode, 180, 16, 43, // Opcode: MULEQ_S_W_PHR_MM
/* 1380 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 1394
/* 1385 */    MCD::OPC_CheckPredicate, 8, 6, 20, 0, // Skip to: 6516
/* 1390 */    MCD::OPC_Decode, 233, 13, 53, // Opcode: LHX_MM
/* 1394 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 1408
/* 1399 */    MCD::OPC_CheckPredicate, 8, 248, 19, 0, // Skip to: 6516
/* 1404 */    MCD::OPC_Decode, 171, 14, 53, // Opcode: LWX_MM
/* 1408 */    MCD::OPC_FilterValue, 8, 239, 19, 0, // Skip to: 6516
/* 1413 */    MCD::OPC_CheckPredicate, 8, 234, 19, 0, // Skip to: 6516
/* 1418 */    MCD::OPC_Decode, 193, 13, 53, // Opcode: LBUX_MM
/* 1422 */    MCD::OPC_FilterValue, 44, 9, 0, 0, // Skip to: 1436
/* 1427 */    MCD::OPC_CheckPredicate, 6, 220, 19, 0, // Skip to: 6516
/* 1432 */    MCD::OPC_Decode, 174, 11, 55, // Opcode: EXT_MM
/* 1436 */    MCD::OPC_FilterValue, 45, 143, 0, 0, // Skip to: 1584
/* 1441 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1444 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1458
/* 1449 */    MCD::OPC_CheckPredicate, 9, 198, 19, 0, // Skip to: 6516
/* 1454 */    MCD::OPC_Decode, 216, 16, 47, // Opcode: MUL_PH_MMR2
/* 1458 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1472
/* 1463 */    MCD::OPC_CheckPredicate, 9, 184, 19, 0, // Skip to: 6516
/* 1468 */    MCD::OPC_Decode, 192, 17, 47, // Opcode: PRECR_QB_PH_MMR2
/* 1472 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1486
/* 1477 */    MCD::OPC_CheckPredicate, 8, 170, 19, 0, // Skip to: 6516
/* 1482 */    MCD::OPC_Decode, 188, 17, 47, // Opcode: PRECRQ_QB_PH_MM
/* 1486 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 1500
/* 1491 */    MCD::OPC_CheckPredicate, 8, 156, 19, 0, // Skip to: 6516
/* 1496 */    MCD::OPC_Decode, 186, 17, 56, // Opcode: PRECRQ_PH_W_MM
/* 1500 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 1514
/* 1505 */    MCD::OPC_CheckPredicate, 8, 142, 19, 0, // Skip to: 6516
/* 1510 */    MCD::OPC_Decode, 190, 17, 56, // Opcode: PRECRQ_RS_PH_W_MM
/* 1514 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 1528
/* 1519 */    MCD::OPC_CheckPredicate, 8, 128, 19, 0, // Skip to: 6516
/* 1524 */    MCD::OPC_Decode, 184, 17, 47, // Opcode: PRECRQU_S_QB_PH_MM
/* 1528 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 1542
/* 1533 */    MCD::OPC_CheckPredicate, 8, 114, 19, 0, // Skip to: 6516
/* 1538 */    MCD::OPC_Decode, 140, 17, 47, // Opcode: PACKRL_PH_MM
/* 1542 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 1556
/* 1547 */    MCD::OPC_CheckPredicate, 8, 100, 19, 0, // Skip to: 6516
/* 1552 */    MCD::OPC_Decode, 159, 17, 47, // Opcode: PICK_QB_MM
/* 1556 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 1570
/* 1561 */    MCD::OPC_CheckPredicate, 8, 86, 19, 0, // Skip to: 6516
/* 1566 */    MCD::OPC_Decode, 157, 17, 47, // Opcode: PICK_PH_MM
/* 1570 */    MCD::OPC_FilterValue, 16, 77, 19, 0, // Skip to: 6516
/* 1575 */    MCD::OPC_CheckPredicate, 9, 72, 19, 0, // Skip to: 6516
/* 1580 */    MCD::OPC_Decode, 221, 16, 47, // Opcode: MUL_S_PH_MMR2
/* 1584 */    MCD::OPC_FilterValue, 52, 45, 0, 0, // Skip to: 1634
/* 1589 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1592 */    MCD::OPC_FilterValue, 19, 16, 0, 0, // Skip to: 1613
/* 1597 */    MCD::OPC_CheckPredicate, 10, 50, 19, 0, // Skip to: 6516
/* 1602 */    MCD::OPC_CheckField, 14, 2, 0, 43, 19, 0, // Skip to: 6516
/* 1609 */    MCD::OPC_Decode, 136, 15, 57, // Opcode: MFHGC0_MM
/* 1613 */    MCD::OPC_FilterValue, 27, 34, 19, 0, // Skip to: 6516
/* 1618 */    MCD::OPC_CheckPredicate, 10, 29, 19, 0, // Skip to: 6516
/* 1623 */    MCD::OPC_CheckField, 14, 2, 0, 22, 19, 0, // Skip to: 6516
/* 1630 */    MCD::OPC_Decode, 152, 16, 58, // Opcode: MTHGC0_MM
/* 1634 */    MCD::OPC_FilterValue, 53, 109, 0, 0, // Skip to: 1748
/* 1639 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1642 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 1656
/* 1647 */    MCD::OPC_CheckPredicate, 8, 0, 19, 0, // Skip to: 6516
/* 1652 */    MCD::OPC_Decode, 251, 18, 41, // Opcode: SHRA_R_W_MM
/* 1656 */    MCD::OPC_FilterValue, 12, 16, 0, 0, // Skip to: 1677
/* 1661 */    MCD::OPC_CheckPredicate, 8, 242, 18, 0, // Skip to: 6516
/* 1666 */    MCD::OPC_CheckField, 11, 1, 0, 235, 18, 0, // Skip to: 6516
/* 1673 */    MCD::OPC_Decode, 243, 18, 59, // Opcode: SHRA_PH_MM
/* 1677 */    MCD::OPC_FilterValue, 14, 31, 0, 0, // Skip to: 1713
/* 1682 */    MCD::OPC_ExtractField, 11, 1,  // Inst{11} ...
/* 1685 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1699
/* 1690 */    MCD::OPC_CheckPredicate, 8, 213, 18, 0, // Skip to: 6516
/* 1695 */    MCD::OPC_Decode, 225, 18, 59, // Opcode: SHLL_PH_MM
/* 1699 */    MCD::OPC_FilterValue, 1, 204, 18, 0, // Skip to: 6516
/* 1704 */    MCD::OPC_CheckPredicate, 8, 199, 18, 0, // Skip to: 6516
/* 1709 */    MCD::OPC_Decode, 229, 18, 59, // Opcode: SHLL_S_PH_MM
/* 1713 */    MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 1727
/* 1718 */    MCD::OPC_CheckPredicate, 8, 185, 18, 0, // Skip to: 6516
/* 1723 */    MCD::OPC_Decode, 231, 18, 41, // Opcode: SHLL_S_W_MM
/* 1727 */    MCD::OPC_FilterValue, 28, 176, 18, 0, // Skip to: 6516
/* 1732 */    MCD::OPC_CheckPredicate, 8, 171, 18, 0, // Skip to: 6516
/* 1737 */    MCD::OPC_CheckField, 11, 1, 0, 164, 18, 0, // Skip to: 6516
/* 1744 */    MCD::OPC_Decode, 247, 18, 59, // Opcode: SHRA_R_PH_MM
/* 1748 */    MCD::OPC_FilterValue, 60, 8, 8, 0, // Skip to: 3809
/* 1753 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1756 */    MCD::OPC_FilterValue, 0, 31, 0, 0, // Skip to: 1792
/* 1761 */    MCD::OPC_ExtractField, 11, 1,  // Inst{11} ...
/* 1764 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1778
/* 1769 */    MCD::OPC_CheckPredicate, 7, 134, 18, 0, // Skip to: 6516
/* 1774 */    MCD::OPC_Decode, 231, 20, 60, // Opcode: TEQ_MM
/* 1778 */    MCD::OPC_FilterValue, 1, 125, 18, 0, // Skip to: 6516
/* 1783 */    MCD::OPC_CheckPredicate, 7, 120, 18, 0, // Skip to: 6516
/* 1788 */    MCD::OPC_Decode, 142, 21, 60, // Opcode: TLT_MM
/* 1792 */    MCD::OPC_FilterValue, 1, 131, 0, 0, // Skip to: 1928
/* 1797 */    MCD::OPC_ExtractField, 11, 2,  // Inst{12-11} ...
/* 1800 */    MCD::OPC_FilterValue, 0, 45, 0, 0, // Skip to: 1850
/* 1805 */    MCD::OPC_ExtractField, 13, 1,  // Inst{13} ...
/* 1808 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 1829
/* 1813 */    MCD::OPC_CheckPredicate, 8, 90, 18, 0, // Skip to: 6516
/* 1818 */    MCD::OPC_CheckField, 21, 5, 0, 83, 18, 0, // Skip to: 6516
/* 1825 */    MCD::OPC_Decode, 141, 15, 61, // Opcode: MFHI_DSP_MM
/* 1829 */    MCD::OPC_FilterValue, 1, 74, 18, 0, // Skip to: 6516
/* 1834 */    MCD::OPC_CheckPredicate, 8, 69, 18, 0, // Skip to: 6516
/* 1839 */    MCD::OPC_CheckField, 21, 5, 0, 62, 18, 0, // Skip to: 6516
/* 1846 */    MCD::OPC_Decode, 156, 16, 62, // Opcode: MTHI_DSP_MM
/* 1850 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1864
/* 1855 */    MCD::OPC_CheckPredicate, 8, 48, 18, 0, // Skip to: 6516
/* 1860 */    MCD::OPC_Decode, 227, 18, 63, // Opcode: SHLL_QB_MM
/* 1864 */    MCD::OPC_FilterValue, 2, 45, 0, 0, // Skip to: 1914
/* 1869 */    MCD::OPC_ExtractField, 13, 1,  // Inst{13} ...
/* 1872 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 1893
/* 1877 */    MCD::OPC_CheckPredicate, 8, 26, 18, 0, // Skip to: 6516
/* 1882 */    MCD::OPC_CheckField, 21, 5, 0, 19, 18, 0, // Skip to: 6516
/* 1889 */    MCD::OPC_Decode, 147, 15, 61, // Opcode: MFLO_DSP_MM
/* 1893 */    MCD::OPC_FilterValue, 1, 10, 18, 0, // Skip to: 6516
/* 1898 */    MCD::OPC_CheckPredicate, 8, 5, 18, 0, // Skip to: 6516
/* 1903 */    MCD::OPC_CheckField, 21, 5, 0, 254, 17, 0, // Skip to: 6516
/* 1910 */    MCD::OPC_Decode, 163, 16, 64, // Opcode: MTLO_DSP_MM
/* 1914 */    MCD::OPC_FilterValue, 3, 245, 17, 0, // Skip to: 6516
/* 1919 */    MCD::OPC_CheckPredicate, 8, 240, 17, 0, // Skip to: 6516
/* 1924 */    MCD::OPC_Decode, 131, 19, 63, // Opcode: SHRL_QB_MM
/* 1928 */    MCD::OPC_FilterValue, 2, 101, 0, 0, // Skip to: 2034
/* 1933 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 1936 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1950
/* 1941 */    MCD::OPC_CheckPredicate, 9, 218, 17, 0, // Skip to: 6516
/* 1946 */    MCD::OPC_Decode, 212, 10, 65, // Opcode: DPA_W_PH_MMR2
/* 1950 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1964
/* 1955 */    MCD::OPC_CheckPredicate, 9, 204, 17, 0, // Skip to: 6516
/* 1960 */    MCD::OPC_Decode, 254, 5, 66, // Opcode: BALIGN_MMR2
/* 1964 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1978
/* 1969 */    MCD::OPC_CheckPredicate, 9, 190, 17, 0, // Skip to: 6516
/* 1974 */    MCD::OPC_Decode, 210, 10, 65, // Opcode: DPAX_W_PH_MMR2
/* 1978 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 1992
/* 1983 */    MCD::OPC_CheckPredicate, 8, 176, 17, 0, // Skip to: 6516
/* 1988 */    MCD::OPC_Decode, 206, 10, 65, // Opcode: DPAU_H_QBL_MM
/* 1992 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2006
/* 1997 */    MCD::OPC_CheckPredicate, 8, 162, 17, 0, // Skip to: 6516
/* 2002 */    MCD::OPC_Decode, 154, 11, 67, // Opcode: EXTPV_MM
/* 2006 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 2020
/* 2011 */    MCD::OPC_CheckPredicate, 8, 148, 17, 0, // Skip to: 6516
/* 2016 */    MCD::OPC_Decode, 208, 10, 65, // Opcode: DPAU_H_QBR_MM
/* 2020 */    MCD::OPC_FilterValue, 7, 139, 17, 0, // Skip to: 6516
/* 2025 */    MCD::OPC_CheckPredicate, 8, 134, 17, 0, // Skip to: 6516
/* 2030 */    MCD::OPC_Decode, 151, 11, 67, // Opcode: EXTPDPV_MM
/* 2034 */    MCD::OPC_FilterValue, 4, 171, 0, 0, // Skip to: 2210
/* 2039 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 2042 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2056
/* 2047 */    MCD::OPC_CheckPredicate, 9, 112, 17, 0, // Skip to: 6516
/* 2052 */    MCD::OPC_Decode, 242, 4, 68, // Opcode: ABSQ_S_QB_MMR2
/* 2056 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2070
/* 2061 */    MCD::OPC_CheckPredicate, 8, 98, 17, 0, // Skip to: 6516
/* 2066 */    MCD::OPC_Decode, 240, 4, 68, // Opcode: ABSQ_S_PH_MM
/* 2070 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 2084
/* 2075 */    MCD::OPC_CheckPredicate, 8, 84, 17, 0, // Skip to: 6516
/* 2080 */    MCD::OPC_Decode, 244, 4, 69, // Opcode: ABSQ_S_W_MM
/* 2084 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 2098
/* 2089 */    MCD::OPC_CheckPredicate, 8, 70, 17, 0, // Skip to: 6516
/* 2094 */    MCD::OPC_Decode, 216, 6, 69, // Opcode: BITREV_MM
/* 2098 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 2112
/* 2103 */    MCD::OPC_CheckPredicate, 8, 56, 17, 0, // Skip to: 6516
/* 2108 */    MCD::OPC_Decode, 142, 13, 70, // Opcode: INSV_MM
/* 2112 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 2126
/* 2117 */    MCD::OPC_CheckPredicate, 8, 42, 17, 0, // Skip to: 6516
/* 2122 */    MCD::OPC_Decode, 172, 17, 71, // Opcode: PRECEQ_W_PHL_MM
/* 2126 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 2140
/* 2131 */    MCD::OPC_CheckPredicate, 8, 28, 17, 0, // Skip to: 6516
/* 2136 */    MCD::OPC_Decode, 174, 17, 71, // Opcode: PRECEQ_W_PHR_MM
/* 2140 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 2154
/* 2145 */    MCD::OPC_CheckPredicate, 8, 14, 17, 0, // Skip to: 6516
/* 2150 */    MCD::OPC_Decode, 166, 17, 68, // Opcode: PRECEQU_PH_QBL_MM
/* 2154 */    MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 2168
/* 2159 */    MCD::OPC_CheckPredicate, 8, 0, 17, 0, // Skip to: 6516
/* 2164 */    MCD::OPC_Decode, 170, 17, 68, // Opcode: PRECEQU_PH_QBR_MM
/* 2168 */    MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 2182
/* 2173 */    MCD::OPC_CheckPredicate, 8, 242, 16, 0, // Skip to: 6516
/* 2178 */    MCD::OPC_Decode, 178, 17, 68, // Opcode: PRECEU_PH_QBL_MM
/* 2182 */    MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 2196
/* 2187 */    MCD::OPC_CheckPredicate, 8, 228, 16, 0, // Skip to: 6516
/* 2192 */    MCD::OPC_Decode, 182, 17, 68, // Opcode: PRECEU_PH_QBR_MM
/* 2196 */    MCD::OPC_FilterValue, 30, 219, 16, 0, // Skip to: 6516
/* 2201 */    MCD::OPC_CheckPredicate, 8, 214, 16, 0, // Skip to: 6516
/* 2206 */    MCD::OPC_Decode, 207, 17, 71, // Opcode: RADDU_W_QB_MM
/* 2210 */    MCD::OPC_FilterValue, 5, 87, 0, 0, // Skip to: 2302
/* 2215 */    MCD::OPC_ExtractField, 11, 15,  // Inst{25-11} ...
/* 2218 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2232
/* 2223 */    MCD::OPC_CheckPredicate, 10, 192, 16, 0, // Skip to: 6516
/* 2228 */    MCD::OPC_Decode, 245, 20, 0, // Opcode: TLBGP_MM
/* 2232 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2246
/* 2237 */    MCD::OPC_CheckPredicate, 10, 178, 16, 0, // Skip to: 6516
/* 2242 */    MCD::OPC_Decode, 247, 20, 0, // Opcode: TLBGR_MM
/* 2246 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 2260
/* 2251 */    MCD::OPC_CheckPredicate, 10, 164, 16, 0, // Skip to: 6516
/* 2256 */    MCD::OPC_Decode, 249, 20, 0, // Opcode: TLBGWI_MM
/* 2260 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 2274
/* 2265 */    MCD::OPC_CheckPredicate, 10, 150, 16, 0, // Skip to: 6516
/* 2270 */    MCD::OPC_Decode, 251, 20, 0, // Opcode: TLBGWR_MM
/* 2274 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 2288
/* 2279 */    MCD::OPC_CheckPredicate, 10, 136, 16, 0, // Skip to: 6516
/* 2284 */    MCD::OPC_Decode, 243, 20, 0, // Opcode: TLBGINV_MM
/* 2288 */    MCD::OPC_FilterValue, 10, 127, 16, 0, // Skip to: 6516
/* 2293 */    MCD::OPC_CheckPredicate, 10, 122, 16, 0, // Skip to: 6516
/* 2298 */    MCD::OPC_Decode, 242, 20, 0, // Opcode: TLBGINVF_MM
/* 2302 */    MCD::OPC_FilterValue, 7, 31, 0, 0, // Skip to: 2338
/* 2307 */    MCD::OPC_ExtractField, 11, 2,  // Inst{12-11} ...
/* 2310 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2324
/* 2315 */    MCD::OPC_CheckPredicate, 9, 100, 16, 0, // Skip to: 6516
/* 2320 */    MCD::OPC_Decode, 245, 18, 63, // Opcode: SHRA_QB_MMR2
/* 2324 */    MCD::OPC_FilterValue, 2, 91, 16, 0, // Skip to: 6516
/* 2329 */    MCD::OPC_CheckPredicate, 9, 86, 16, 0, // Skip to: 6516
/* 2334 */    MCD::OPC_Decode, 249, 18, 63, // Opcode: SHRA_R_QB_MMR2
/* 2338 */    MCD::OPC_FilterValue, 8, 31, 0, 0, // Skip to: 2374
/* 2343 */    MCD::OPC_ExtractField, 11, 1,  // Inst{11} ...
/* 2346 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2360
/* 2351 */    MCD::OPC_CheckPredicate, 7, 64, 16, 0, // Skip to: 6516
/* 2356 */    MCD::OPC_Decode, 239, 20, 60, // Opcode: TGE_MM
/* 2360 */    MCD::OPC_FilterValue, 1, 55, 16, 0, // Skip to: 6516
/* 2365 */    MCD::OPC_CheckPredicate, 7, 50, 16, 0, // Skip to: 6516
/* 2370 */    MCD::OPC_Decode, 141, 21, 60, // Opcode: TLTU_MM
/* 2374 */    MCD::OPC_FilterValue, 9, 101, 0, 0, // Skip to: 2480
/* 2379 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 2382 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 2403
/* 2387 */    MCD::OPC_CheckPredicate, 8, 28, 16, 0, // Skip to: 6516
/* 2392 */    MCD::OPC_CheckField, 21, 5, 0, 21, 16, 0, // Skip to: 6516
/* 2399 */    MCD::OPC_Decode, 159, 16, 72, // Opcode: MTHLIP_MM
/* 2403 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2417
/* 2408 */    MCD::OPC_CheckPredicate, 8, 7, 16, 0, // Skip to: 6516
/* 2413 */    MCD::OPC_Decode, 218, 14, 65, // Opcode: MAQ_S_W_PHR_MM
/* 2417 */    MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 2438
/* 2422 */    MCD::OPC_CheckPredicate, 8, 249, 15, 0, // Skip to: 6516
/* 2427 */    MCD::OPC_CheckField, 21, 5, 0, 242, 15, 0, // Skip to: 6516
/* 2434 */    MCD::OPC_Decode, 214, 18, 72, // Opcode: SHILOV_MM
/* 2438 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 2452
/* 2443 */    MCD::OPC_CheckPredicate, 8, 228, 15, 0, // Skip to: 6516
/* 2448 */    MCD::OPC_Decode, 216, 14, 65, // Opcode: MAQ_S_W_PHL_MM
/* 2452 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2466
/* 2457 */    MCD::OPC_CheckPredicate, 8, 214, 15, 0, // Skip to: 6516
/* 2462 */    MCD::OPC_Decode, 214, 14, 65, // Opcode: MAQ_SA_W_PHR_MM
/* 2466 */    MCD::OPC_FilterValue, 7, 205, 15, 0, // Skip to: 6516
/* 2471 */    MCD::OPC_CheckPredicate, 8, 200, 15, 0, // Skip to: 6516
/* 2476 */    MCD::OPC_Decode, 212, 14, 65, // Opcode: MAQ_SA_W_PHL_MM
/* 2480 */    MCD::OPC_FilterValue, 10, 115, 0, 0, // Skip to: 2600
/* 2485 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 2488 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2502
/* 2493 */    MCD::OPC_CheckPredicate, 8, 178, 15, 0, // Skip to: 6516
/* 2498 */    MCD::OPC_Decode, 204, 10, 65, // Opcode: DPAQ_S_W_PH_MM
/* 2502 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2516
/* 2507 */    MCD::OPC_CheckPredicate, 8, 164, 15, 0, // Skip to: 6516
/* 2512 */    MCD::OPC_Decode, 205, 14, 65, // Opcode: MADD_DSP_MM
/* 2516 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2530
/* 2521 */    MCD::OPC_CheckPredicate, 8, 150, 15, 0, // Skip to: 6516
/* 2526 */    MCD::OPC_Decode, 202, 10, 65, // Opcode: DPAQ_SA_L_W_MM
/* 2530 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 2544
/* 2535 */    MCD::OPC_CheckPredicate, 8, 136, 15, 0, // Skip to: 6516
/* 2540 */    MCD::OPC_Decode, 195, 14, 65, // Opcode: MADDU_DSP_MM
/* 2544 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 2558
/* 2549 */    MCD::OPC_CheckPredicate, 9, 122, 15, 0, // Skip to: 6516
/* 2554 */    MCD::OPC_Decode, 200, 10, 65, // Opcode: DPAQX_S_W_PH_MMR2
/* 2558 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2572
/* 2563 */    MCD::OPC_CheckPredicate, 8, 108, 15, 0, // Skip to: 6516
/* 2568 */    MCD::OPC_Decode, 128, 16, 65, // Opcode: MSUB_DSP_MM
/* 2572 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 2586
/* 2577 */    MCD::OPC_CheckPredicate, 9, 94, 15, 0, // Skip to: 6516
/* 2582 */    MCD::OPC_Decode, 198, 10, 65, // Opcode: DPAQX_SA_W_PH_MMR2
/* 2586 */    MCD::OPC_FilterValue, 7, 85, 15, 0, // Skip to: 6516
/* 2591 */    MCD::OPC_CheckPredicate, 8, 80, 15, 0, // Skip to: 6516
/* 2596 */    MCD::OPC_Decode, 246, 15, 65, // Opcode: MSUBU_DSP_MM
/* 2600 */    MCD::OPC_FilterValue, 12, 27, 1, 0, // Skip to: 2888
/* 2605 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 2608 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2622
/* 2613 */    MCD::OPC_CheckPredicate, 8, 58, 15, 0, // Skip to: 6516
/* 2618 */    MCD::OPC_Decode, 222, 17, 73, // Opcode: REPLV_PH_MM
/* 2622 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2636
/* 2627 */    MCD::OPC_CheckPredicate, 8, 44, 15, 0, // Skip to: 6516
/* 2632 */    MCD::OPC_Decode, 224, 17, 73, // Opcode: REPLV_QB_MM
/* 2636 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2650
/* 2641 */    MCD::OPC_CheckPredicate, 7, 30, 15, 0, // Skip to: 6516
/* 2646 */    MCD::OPC_Decode, 179, 18, 69, // Opcode: SEB_MM
/* 2650 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 2664
/* 2655 */    MCD::OPC_CheckPredicate, 7, 16, 15, 0, // Skip to: 6516
/* 2660 */    MCD::OPC_Decode, 182, 18, 69, // Opcode: SEH_MM
/* 2664 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 2678
/* 2669 */    MCD::OPC_CheckPredicate, 7, 2, 15, 0, // Skip to: 6516
/* 2674 */    MCD::OPC_Decode, 243, 7, 69, // Opcode: CLO_MM
/* 2678 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 2692
/* 2683 */    MCD::OPC_CheckPredicate, 7, 244, 14, 0, // Skip to: 6516
/* 2688 */    MCD::OPC_Decode, 135, 8, 69, // Opcode: CLZ_MM
/* 2692 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 2706
/* 2697 */    MCD::OPC_CheckPredicate, 6, 230, 14, 0, // Skip to: 6516
/* 2702 */    MCD::OPC_Decode, 212, 17, 74, // Opcode: RDHWR_MM
/* 2706 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 2720
/* 2711 */    MCD::OPC_CheckPredicate, 8, 216, 14, 0, // Skip to: 6516
/* 2716 */    MCD::OPC_Decode, 165, 17, 68, // Opcode: PRECEQU_PH_QBLA_MM
/* 2720 */    MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 2734
/* 2725 */    MCD::OPC_CheckPredicate, 7, 202, 14, 0, // Skip to: 6516
/* 2730 */    MCD::OPC_Decode, 175, 21, 69, // Opcode: WSBH_MM
/* 2734 */    MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 2748
/* 2739 */    MCD::OPC_CheckPredicate, 6, 188, 14, 0, // Skip to: 6516
/* 2744 */    MCD::OPC_Decode, 204, 16, 75, // Opcode: MULT_MM
/* 2748 */    MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 2762
/* 2753 */    MCD::OPC_CheckPredicate, 8, 174, 14, 0, // Skip to: 6516
/* 2758 */    MCD::OPC_Decode, 169, 17, 68, // Opcode: PRECEQU_PH_QBRA_MM
/* 2762 */    MCD::OPC_FilterValue, 19, 9, 0, 0, // Skip to: 2776
/* 2767 */    MCD::OPC_CheckPredicate, 6, 160, 14, 0, // Skip to: 6516
/* 2772 */    MCD::OPC_Decode, 206, 16, 75, // Opcode: MULTu_MM
/* 2776 */    MCD::OPC_FilterValue, 21, 9, 0, 0, // Skip to: 2790
/* 2781 */    MCD::OPC_CheckPredicate, 6, 146, 14, 0, // Skip to: 6516
/* 2786 */    MCD::OPC_Decode, 172, 18, 75, // Opcode: SDIV_MM
/* 2790 */    MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 2804
/* 2795 */    MCD::OPC_CheckPredicate, 8, 132, 14, 0, // Skip to: 6516
/* 2800 */    MCD::OPC_Decode, 177, 17, 68, // Opcode: PRECEU_PH_QBLA_MM
/* 2804 */    MCD::OPC_FilterValue, 23, 9, 0, 0, // Skip to: 2818
/* 2809 */    MCD::OPC_CheckPredicate, 6, 118, 14, 0, // Skip to: 6516
/* 2814 */    MCD::OPC_Decode, 160, 21, 75, // Opcode: UDIV_MM
/* 2818 */    MCD::OPC_FilterValue, 25, 9, 0, 0, // Skip to: 2832
/* 2823 */    MCD::OPC_CheckPredicate, 6, 104, 14, 0, // Skip to: 6516
/* 2828 */    MCD::OPC_Decode, 206, 14, 75, // Opcode: MADD_MM
/* 2832 */    MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 2846
/* 2837 */    MCD::OPC_CheckPredicate, 8, 90, 14, 0, // Skip to: 6516
/* 2842 */    MCD::OPC_Decode, 181, 17, 68, // Opcode: PRECEU_PH_QBRA_MM
/* 2846 */    MCD::OPC_FilterValue, 27, 9, 0, 0, // Skip to: 2860
/* 2851 */    MCD::OPC_CheckPredicate, 6, 76, 14, 0, // Skip to: 6516
/* 2856 */    MCD::OPC_Decode, 196, 14, 75, // Opcode: MADDU_MM
/* 2860 */    MCD::OPC_FilterValue, 29, 9, 0, 0, // Skip to: 2874
/* 2865 */    MCD::OPC_CheckPredicate, 6, 62, 14, 0, // Skip to: 6516
/* 2870 */    MCD::OPC_Decode, 129, 16, 75, // Opcode: MSUB_MM
/* 2874 */    MCD::OPC_FilterValue, 31, 53, 14, 0, // Skip to: 6516
/* 2879 */    MCD::OPC_CheckPredicate, 6, 48, 14, 0, // Skip to: 6516
/* 2884 */    MCD::OPC_Decode, 247, 15, 75, // Opcode: MSUBU_MM
/* 2888 */    MCD::OPC_FilterValue, 13, 206, 0, 0, // Skip to: 3099
/* 2893 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 2896 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 2917
/* 2901 */    MCD::OPC_CheckPredicate, 7, 26, 14, 0, // Skip to: 6516
/* 2906 */    MCD::OPC_CheckField, 16, 10, 0, 19, 14, 0, // Skip to: 6516
/* 2913 */    MCD::OPC_Decode, 129, 21, 0, // Opcode: TLBP_MM
/* 2917 */    MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 2938
/* 2922 */    MCD::OPC_CheckPredicate, 7, 5, 14, 0, // Skip to: 6516
/* 2927 */    MCD::OPC_CheckField, 16, 10, 0, 254, 13, 0, // Skip to: 6516
/* 2934 */    MCD::OPC_Decode, 131, 21, 0, // Opcode: TLBR_MM
/* 2938 */    MCD::OPC_FilterValue, 4, 16, 0, 0, // Skip to: 2959
/* 2943 */    MCD::OPC_CheckPredicate, 7, 240, 13, 0, // Skip to: 6516
/* 2948 */    MCD::OPC_CheckField, 16, 10, 0, 233, 13, 0, // Skip to: 6516
/* 2955 */    MCD::OPC_Decode, 133, 21, 0, // Opcode: TLBWI_MM
/* 2959 */    MCD::OPC_FilterValue, 6, 16, 0, 0, // Skip to: 2980
/* 2964 */    MCD::OPC_CheckPredicate, 7, 219, 13, 0, // Skip to: 6516
/* 2969 */    MCD::OPC_CheckField, 16, 10, 0, 212, 13, 0, // Skip to: 6516
/* 2976 */    MCD::OPC_Decode, 135, 21, 0, // Opcode: TLBWR_MM
/* 2980 */    MCD::OPC_FilterValue, 13, 16, 0, 0, // Skip to: 3001
/* 2985 */    MCD::OPC_CheckPredicate, 7, 198, 13, 0, // Skip to: 6516
/* 2990 */    MCD::OPC_CheckField, 21, 5, 0, 191, 13, 0, // Skip to: 6516
/* 2997 */    MCD::OPC_Decode, 203, 20, 76, // Opcode: SYNC_MM
/* 3001 */    MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 3015
/* 3006 */    MCD::OPC_CheckPredicate, 7, 177, 13, 0, // Skip to: 6516
/* 3011 */    MCD::OPC_Decode, 206, 20, 77, // Opcode: SYSCALL_MM
/* 3015 */    MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 3029
/* 3020 */    MCD::OPC_CheckPredicate, 7, 163, 13, 0, // Skip to: 6516
/* 3025 */    MCD::OPC_Decode, 169, 21, 77, // Opcode: WAIT_MM
/* 3029 */    MCD::OPC_FilterValue, 24, 9, 0, 0, // Skip to: 3043
/* 3034 */    MCD::OPC_CheckPredicate, 10, 149, 13, 0, // Skip to: 6516
/* 3039 */    MCD::OPC_Decode, 243, 12, 77, // Opcode: HYPCALL_MM
/* 3043 */    MCD::OPC_FilterValue, 27, 9, 0, 0, // Skip to: 3057
/* 3048 */    MCD::OPC_CheckPredicate, 7, 135, 13, 0, // Skip to: 6516
/* 3053 */    MCD::OPC_Decode, 160, 18, 77, // Opcode: SDBBP_MM
/* 3057 */    MCD::OPC_FilterValue, 28, 16, 0, 0, // Skip to: 3078
/* 3062 */    MCD::OPC_CheckPredicate, 7, 121, 13, 0, // Skip to: 6516
/* 3067 */    MCD::OPC_CheckField, 16, 10, 0, 114, 13, 0, // Skip to: 6516
/* 3074 */    MCD::OPC_Decode, 139, 10, 0, // Opcode: DERET_MM
/* 3078 */    MCD::OPC_FilterValue, 30, 105, 13, 0, // Skip to: 6516
/* 3083 */    MCD::OPC_CheckPredicate, 7, 100, 13, 0, // Skip to: 6516
/* 3088 */    MCD::OPC_CheckField, 16, 10, 0, 93, 13, 0, // Skip to: 6516
/* 3095 */    MCD::OPC_Decode, 142, 11, 0, // Opcode: ERET_MM
/* 3099 */    MCD::OPC_FilterValue, 15, 16, 0, 0, // Skip to: 3120
/* 3104 */    MCD::OPC_CheckPredicate, 9, 79, 13, 0, // Skip to: 6516
/* 3109 */    MCD::OPC_CheckField, 11, 1, 0, 72, 13, 0, // Skip to: 6516
/* 3116 */    MCD::OPC_Decode, 129, 19, 59, // Opcode: SHRL_PH_MMR2
/* 3120 */    MCD::OPC_FilterValue, 16, 31, 0, 0, // Skip to: 3156
/* 3125 */    MCD::OPC_ExtractField, 11, 1,  // Inst{11} ...
/* 3128 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3142
/* 3133 */    MCD::OPC_CheckPredicate, 7, 50, 13, 0, // Skip to: 6516
/* 3138 */    MCD::OPC_Decode, 238, 20, 60, // Opcode: TGEU_MM
/* 3142 */    MCD::OPC_FilterValue, 1, 41, 13, 0, // Skip to: 6516
/* 3147 */    MCD::OPC_CheckPredicate, 7, 36, 13, 0, // Skip to: 6516
/* 3152 */    MCD::OPC_Decode, 146, 21, 60, // Opcode: TNE_MM
/* 3156 */    MCD::OPC_FilterValue, 18, 115, 0, 0, // Skip to: 3276
/* 3161 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 3164 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3178
/* 3169 */    MCD::OPC_CheckPredicate, 9, 14, 13, 0, // Skip to: 6516
/* 3174 */    MCD::OPC_Decode, 235, 10, 65, // Opcode: DPS_W_PH_MMR2
/* 3178 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3192
/* 3183 */    MCD::OPC_CheckPredicate, 8, 0, 13, 0, // Skip to: 6516
/* 3188 */    MCD::OPC_Decode, 203, 16, 78, // Opcode: MULT_DSP_MM
/* 3192 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3206
/* 3197 */    MCD::OPC_CheckPredicate, 9, 242, 12, 0, // Skip to: 6516
/* 3202 */    MCD::OPC_Decode, 233, 10, 65, // Opcode: DPSX_W_PH_MMR2
/* 3206 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 3220
/* 3211 */    MCD::OPC_CheckPredicate, 8, 228, 12, 0, // Skip to: 6516
/* 3216 */    MCD::OPC_Decode, 201, 16, 78, // Opcode: MULTU_DSP_MM
/* 3220 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 3234
/* 3225 */    MCD::OPC_CheckPredicate, 8, 214, 12, 0, // Skip to: 6516
/* 3230 */    MCD::OPC_Decode, 229, 10, 65, // Opcode: DPSU_H_QBL_MM
/* 3234 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 3248
/* 3239 */    MCD::OPC_CheckPredicate, 9, 200, 12, 0, // Skip to: 6516
/* 3244 */    MCD::OPC_Decode, 198, 16, 65, // Opcode: MULSA_W_PH_MMR2
/* 3248 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 3262
/* 3253 */    MCD::OPC_CheckPredicate, 8, 186, 12, 0, // Skip to: 6516
/* 3258 */    MCD::OPC_Decode, 231, 10, 65, // Opcode: DPSU_H_QBR_MM
/* 3262 */    MCD::OPC_FilterValue, 7, 177, 12, 0, // Skip to: 6516
/* 3267 */    MCD::OPC_CheckPredicate, 8, 172, 12, 0, // Skip to: 6516
/* 3272 */    MCD::OPC_Decode, 196, 16, 65, // Opcode: MULSAQ_S_W_PH_MM
/* 3276 */    MCD::OPC_FilterValue, 19, 16, 0, 0, // Skip to: 3297
/* 3281 */    MCD::OPC_CheckPredicate, 10, 158, 12, 0, // Skip to: 6516
/* 3286 */    MCD::OPC_CheckField, 14, 2, 0, 151, 12, 0, // Skip to: 6516
/* 3293 */    MCD::OPC_Decode, 128, 15, 57, // Opcode: MFGC0_MM
/* 3297 */    MCD::OPC_FilterValue, 20, 31, 0, 0, // Skip to: 3333
/* 3302 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3305 */    MCD::OPC_FilterValue, 25, 9, 0, 0, // Skip to: 3319
/* 3310 */    MCD::OPC_CheckPredicate, 7, 129, 12, 0, // Skip to: 6516
/* 3315 */    MCD::OPC_Decode, 216, 7, 79, // Opcode: CFC2_MM
/* 3319 */    MCD::OPC_FilterValue, 27, 120, 12, 0, // Skip to: 6516
/* 3324 */    MCD::OPC_CheckPredicate, 7, 115, 12, 0, // Skip to: 6516
/* 3329 */    MCD::OPC_Decode, 243, 8, 80, // Opcode: CTC2_MM
/* 3333 */    MCD::OPC_FilterValue, 21, 87, 0, 0, // Skip to: 3425
/* 3338 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3341 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 3362
/* 3346 */    MCD::OPC_CheckPredicate, 6, 93, 12, 0, // Skip to: 6516
/* 3351 */    MCD::OPC_CheckField, 21, 5, 0, 86, 12, 0, // Skip to: 6516
/* 3358 */    MCD::OPC_Decode, 142, 15, 81, // Opcode: MFHI_MM
/* 3362 */    MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 3383
/* 3367 */    MCD::OPC_CheckPredicate, 6, 72, 12, 0, // Skip to: 6516
/* 3372 */    MCD::OPC_CheckField, 21, 5, 0, 65, 12, 0, // Skip to: 6516
/* 3379 */    MCD::OPC_Decode, 148, 15, 81, // Opcode: MFLO_MM
/* 3383 */    MCD::OPC_FilterValue, 5, 16, 0, 0, // Skip to: 3404
/* 3388 */    MCD::OPC_CheckPredicate, 6, 51, 12, 0, // Skip to: 6516
/* 3393 */    MCD::OPC_CheckField, 21, 5, 0, 44, 12, 0, // Skip to: 6516
/* 3400 */    MCD::OPC_Decode, 157, 16, 81, // Opcode: MTHI_MM
/* 3404 */    MCD::OPC_FilterValue, 7, 35, 12, 0, // Skip to: 6516
/* 3409 */    MCD::OPC_CheckPredicate, 6, 30, 12, 0, // Skip to: 6516
/* 3414 */    MCD::OPC_CheckField, 21, 5, 0, 23, 12, 0, // Skip to: 6516
/* 3421 */    MCD::OPC_Decode, 164, 16, 81, // Opcode: MTLO_MM
/* 3425 */    MCD::OPC_FilterValue, 23, 16, 0, 0, // Skip to: 3446
/* 3430 */    MCD::OPC_CheckPredicate, 8, 9, 12, 0, // Skip to: 6516
/* 3435 */    MCD::OPC_CheckField, 11, 2, 0, 2, 12, 0, // Skip to: 6516
/* 3442 */    MCD::OPC_Decode, 228, 17, 82, // Opcode: REPL_QB_MM
/* 3446 */    MCD::OPC_FilterValue, 25, 115, 0, 0, // Skip to: 3566
/* 3451 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 3454 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3468
/* 3459 */    MCD::OPC_CheckPredicate, 8, 236, 11, 0, // Skip to: 6516
/* 3464 */    MCD::OPC_Decode, 209, 17, 83, // Opcode: RDDSP_MM
/* 3468 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3482
/* 3473 */    MCD::OPC_CheckPredicate, 8, 222, 11, 0, // Skip to: 6516
/* 3478 */    MCD::OPC_Decode, 171, 11, 84, // Opcode: EXTR_W_MM
/* 3482 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3496
/* 3487 */    MCD::OPC_CheckPredicate, 8, 208, 11, 0, // Skip to: 6516
/* 3492 */    MCD::OPC_Decode, 172, 21, 83, // Opcode: WRDSP_MM
/* 3496 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 3510
/* 3501 */    MCD::OPC_CheckPredicate, 8, 194, 11, 0, // Skip to: 6516
/* 3506 */    MCD::OPC_Decode, 167, 11, 84, // Opcode: EXTR_R_W_MM
/* 3510 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 3524
/* 3515 */    MCD::OPC_CheckPredicate, 8, 180, 11, 0, // Skip to: 6516
/* 3520 */    MCD::OPC_Decode, 155, 11, 84, // Opcode: EXTP_MM
/* 3524 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 3538
/* 3529 */    MCD::OPC_CheckPredicate, 8, 166, 11, 0, // Skip to: 6516
/* 3534 */    MCD::OPC_Decode, 165, 11, 84, // Opcode: EXTR_RS_W_MM
/* 3538 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 3552
/* 3543 */    MCD::OPC_CheckPredicate, 8, 152, 11, 0, // Skip to: 6516
/* 3548 */    MCD::OPC_Decode, 152, 11, 84, // Opcode: EXTPDP_MM
/* 3552 */    MCD::OPC_FilterValue, 7, 143, 11, 0, // Skip to: 6516
/* 3557 */    MCD::OPC_CheckPredicate, 8, 138, 11, 0, // Skip to: 6516
/* 3562 */    MCD::OPC_Decode, 169, 11, 84, // Opcode: EXTR_S_H_MM
/* 3566 */    MCD::OPC_FilterValue, 26, 115, 0, 0, // Skip to: 3686
/* 3571 */    MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 3574 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3588
/* 3579 */    MCD::OPC_CheckPredicate, 8, 116, 11, 0, // Skip to: 6516
/* 3584 */    MCD::OPC_Decode, 221, 10, 65, // Opcode: DPSQ_S_W_PH_MM
/* 3588 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3602
/* 3593 */    MCD::OPC_CheckPredicate, 8, 102, 11, 0, // Skip to: 6516
/* 3598 */    MCD::OPC_Decode, 163, 11, 67, // Opcode: EXTRV_W_MM
/* 3602 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3616
/* 3607 */    MCD::OPC_CheckPredicate, 8, 88, 11, 0, // Skip to: 6516
/* 3612 */    MCD::OPC_Decode, 219, 10, 65, // Opcode: DPSQ_SA_L_W_MM
/* 3616 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 3630
/* 3621 */    MCD::OPC_CheckPredicate, 8, 74, 11, 0, // Skip to: 6516
/* 3626 */    MCD::OPC_Decode, 159, 11, 67, // Opcode: EXTRV_R_W_MM
/* 3630 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 3644
/* 3635 */    MCD::OPC_CheckPredicate, 9, 60, 11, 0, // Skip to: 6516
/* 3640 */    MCD::OPC_Decode, 217, 10, 65, // Opcode: DPSQX_S_W_PH_MMR2
/* 3644 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 3658
/* 3649 */    MCD::OPC_CheckPredicate, 8, 46, 11, 0, // Skip to: 6516
/* 3654 */    MCD::OPC_Decode, 157, 11, 67, // Opcode: EXTRV_RS_W_MM
/* 3658 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 3672
/* 3663 */    MCD::OPC_CheckPredicate, 9, 32, 11, 0, // Skip to: 6516
/* 3668 */    MCD::OPC_Decode, 215, 10, 65, // Opcode: DPSQX_SA_W_PH_MMR2
/* 3672 */    MCD::OPC_FilterValue, 7, 23, 11, 0, // Skip to: 6516
/* 3677 */    MCD::OPC_CheckPredicate, 8, 18, 11, 0, // Skip to: 6516
/* 3682 */    MCD::OPC_Decode, 161, 11, 67, // Opcode: EXTRV_S_H_MM
/* 3686 */    MCD::OPC_FilterValue, 27, 16, 0, 0, // Skip to: 3707
/* 3691 */    MCD::OPC_CheckPredicate, 10, 4, 11, 0, // Skip to: 6516
/* 3696 */    MCD::OPC_CheckField, 14, 2, 0, 253, 10, 0, // Skip to: 6516
/* 3703 */    MCD::OPC_Decode, 144, 16, 58, // Opcode: MTGC0_MM
/* 3707 */    MCD::OPC_FilterValue, 28, 47, 0, 0, // Skip to: 3759
/* 3712 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3715 */    MCD::OPC_FilterValue, 1, 25, 0, 0, // Skip to: 3745
/* 3720 */    MCD::OPC_CheckPredicate, 6, 11, 0, 0, // Skip to: 3736
/* 3725 */    MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 3736
/* 3732 */    MCD::OPC_Decode, 179, 13, 81, // Opcode: JR_MM
/* 3736 */    MCD::OPC_CheckPredicate, 6, 215, 10, 0, // Skip to: 6516
/* 3741 */    MCD::OPC_Decode, 157, 13, 69, // Opcode: JALR_MM
/* 3745 */    MCD::OPC_FilterValue, 9, 206, 10, 0, // Skip to: 6516
/* 3750 */    MCD::OPC_CheckPredicate, 6, 201, 10, 0, // Skip to: 6516
/* 3755 */    MCD::OPC_Decode, 154, 13, 69, // Opcode: JALRS_MM
/* 3759 */    MCD::OPC_FilterValue, 29, 192, 10, 0, // Skip to: 6516
/* 3764 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 3767 */    MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 3788
/* 3772 */    MCD::OPC_CheckPredicate, 7, 179, 10, 0, // Skip to: 6516
/* 3777 */    MCD::OPC_CheckField, 21, 5, 0, 172, 10, 0, // Skip to: 6516
/* 3784 */    MCD::OPC_Decode, 161, 10, 81, // Opcode: DI_MM
/* 3788 */    MCD::OPC_FilterValue, 10, 163, 10, 0, // Skip to: 6516
/* 3793 */    MCD::OPC_CheckPredicate, 7, 158, 10, 0, // Skip to: 6516
/* 3798 */    MCD::OPC_CheckField, 21, 5, 0, 151, 10, 0, // Skip to: 6516
/* 3805 */    MCD::OPC_Decode, 136, 11, 81, // Opcode: EI_MM
/* 3809 */    MCD::OPC_FilterValue, 61, 142, 10, 0, // Skip to: 6516
/* 3814 */    MCD::OPC_CheckPredicate, 8, 137, 10, 0, // Skip to: 6516
/* 3819 */    MCD::OPC_CheckField, 6, 5, 0, 130, 10, 0, // Skip to: 6516
/* 3826 */    MCD::OPC_Decode, 226, 17, 85, // Opcode: REPL_PH_MM
/* 3830 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 3844
/* 3835 */    MCD::OPC_CheckPredicate, 6, 116, 10, 0, // Skip to: 6516
/* 3840 */    MCD::OPC_Decode, 186, 5, 86, // Opcode: ADDi_MM
/* 3844 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 3858
/* 3849 */    MCD::OPC_CheckPredicate, 7, 102, 10, 0, // Skip to: 6516
/* 3854 */    MCD::OPC_Decode, 201, 13, 87, // Opcode: LBu_MM
/* 3858 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 3872
/* 3863 */    MCD::OPC_CheckPredicate, 7, 88, 10, 0, // Skip to: 6516
/* 3868 */    MCD::OPC_Decode, 144, 18, 87, // Opcode: SB_MM
/* 3872 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 3886
/* 3877 */    MCD::OPC_CheckPredicate, 7, 74, 10, 0, // Skip to: 6516
/* 3882 */    MCD::OPC_Decode, 195, 13, 87, // Opcode: LB_MM
/* 3886 */    MCD::OPC_FilterValue, 8, 73, 0, 0, // Skip to: 3964
/* 3891 */    MCD::OPC_ExtractField, 12, 4,  // Inst{15-12} ...
/* 3894 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3908
/* 3899 */    MCD::OPC_CheckPredicate, 7, 52, 10, 0, // Skip to: 6516
/* 3904 */    MCD::OPC_Decode, 158, 14, 88, // Opcode: LWP_MM
/* 3908 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 3922
/* 3913 */    MCD::OPC_CheckPredicate, 7, 38, 10, 0, // Skip to: 6516
/* 3918 */    MCD::OPC_Decode, 155, 14, 88, // Opcode: LWM32_MM
/* 3922 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 3936
/* 3927 */    MCD::OPC_CheckPredicate, 6, 24, 10, 0, // Skip to: 6516
/* 3932 */    MCD::OPC_Decode, 192, 7, 89, // Opcode: CACHE_MM
/* 3936 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 3950
/* 3941 */    MCD::OPC_CheckPredicate, 7, 10, 10, 0, // Skip to: 6516
/* 3946 */    MCD::OPC_Decode, 187, 20, 88, // Opcode: SWP_MM
/* 3950 */    MCD::OPC_FilterValue, 13, 1, 10, 0, // Skip to: 6516
/* 3955 */    MCD::OPC_CheckPredicate, 7, 252, 9, 0, // Skip to: 6516
/* 3960 */    MCD::OPC_Decode, 186, 20, 88, // Opcode: SWM32_MM
/* 3964 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 3978
/* 3969 */    MCD::OPC_CheckPredicate, 6, 238, 9, 0, // Skip to: 6516
/* 3974 */    MCD::OPC_Decode, 188, 5, 86, // Opcode: ADDiu_MM
/* 3978 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 3992
/* 3983 */    MCD::OPC_CheckPredicate, 7, 224, 9, 0, // Skip to: 6516
/* 3988 */    MCD::OPC_Decode, 239, 13, 87, // Opcode: LHu_MM
/* 3992 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 4006
/* 3997 */    MCD::OPC_CheckPredicate, 7, 210, 9, 0, // Skip to: 6516
/* 4002 */    MCD::OPC_Decode, 132, 19, 87, // Opcode: SH_MM
/* 4006 */    MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 4020
/* 4011 */    MCD::OPC_CheckPredicate, 7, 196, 9, 0, // Skip to: 6516
/* 4016 */    MCD::OPC_Decode, 234, 13, 87, // Opcode: LH_MM
/* 4020 */    MCD::OPC_FilterValue, 16, 83, 1, 0, // Skip to: 4364
/* 4025 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 4028 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 4042
/* 4033 */    MCD::OPC_CheckPredicate, 6, 174, 9, 0, // Skip to: 6516
/* 4038 */    MCD::OPC_Decode, 246, 6, 90, // Opcode: BLTZ_MM
/* 4042 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 4056
/* 4047 */    MCD::OPC_CheckPredicate, 6, 160, 9, 0, // Skip to: 6516
/* 4052 */    MCD::OPC_Decode, 241, 6, 90, // Opcode: BLTZAL_MM
/* 4056 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 4070
/* 4061 */    MCD::OPC_CheckPredicate, 6, 146, 9, 0, // Skip to: 6516
/* 4066 */    MCD::OPC_Decode, 189, 6, 90, // Opcode: BGEZ_MM
/* 4070 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 4084
/* 4075 */    MCD::OPC_CheckPredicate, 6, 132, 9, 0, // Skip to: 6516
/* 4080 */    MCD::OPC_Decode, 184, 6, 90, // Opcode: BGEZAL_MM
/* 4084 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 4098
/* 4089 */    MCD::OPC_CheckPredicate, 6, 118, 9, 0, // Skip to: 6516
/* 4094 */    MCD::OPC_Decode, 227, 6, 90, // Opcode: BLEZ_MM
/* 4098 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 4112
/* 4103 */    MCD::OPC_CheckPredicate, 6, 104, 9, 0, // Skip to: 6516
/* 4108 */    MCD::OPC_Decode, 143, 7, 90, // Opcode: BNEZC_MM
/* 4112 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 4126
/* 4117 */    MCD::OPC_CheckPredicate, 6, 90, 9, 0, // Skip to: 6516
/* 4122 */    MCD::OPC_Decode, 198, 6, 90, // Opcode: BGTZ_MM
/* 4126 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 4140
/* 4131 */    MCD::OPC_CheckPredicate, 6, 76, 9, 0, // Skip to: 6516
/* 4136 */    MCD::OPC_Decode, 168, 6, 90, // Opcode: BEQZC_MM
/* 4140 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 4154
/* 4145 */    MCD::OPC_CheckPredicate, 6, 62, 9, 0, // Skip to: 6516
/* 4150 */    MCD::OPC_Decode, 139, 21, 91, // Opcode: TLTI_MM
/* 4154 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 4168
/* 4159 */    MCD::OPC_CheckPredicate, 6, 48, 9, 0, // Skip to: 6516
/* 4164 */    MCD::OPC_Decode, 236, 20, 91, // Opcode: TGEI_MM
/* 4168 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 4182
/* 4173 */    MCD::OPC_CheckPredicate, 6, 34, 9, 0, // Skip to: 6516
/* 4178 */    MCD::OPC_Decode, 138, 21, 91, // Opcode: TLTIU_MM
/* 4182 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 4196
/* 4187 */    MCD::OPC_CheckPredicate, 6, 20, 9, 0, // Skip to: 6516
/* 4192 */    MCD::OPC_Decode, 235, 20, 91, // Opcode: TGEIU_MM
/* 4196 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 4210
/* 4201 */    MCD::OPC_CheckPredicate, 6, 6, 9, 0, // Skip to: 6516
/* 4206 */    MCD::OPC_Decode, 145, 21, 91, // Opcode: TNEI_MM
/* 4210 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 4224
/* 4215 */    MCD::OPC_CheckPredicate, 6, 248, 8, 0, // Skip to: 6516
/* 4220 */    MCD::OPC_Decode, 133, 14, 92, // Opcode: LUi_MM
/* 4224 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 4238
/* 4229 */    MCD::OPC_CheckPredicate, 6, 234, 8, 0, // Skip to: 6516
/* 4234 */    MCD::OPC_Decode, 230, 20, 91, // Opcode: TEQI_MM
/* 4238 */    MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 4252
/* 4243 */    MCD::OPC_CheckPredicate, 6, 220, 8, 0, // Skip to: 6516
/* 4248 */    MCD::OPC_Decode, 201, 20, 93, // Opcode: SYNCI_MM
/* 4252 */    MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 4266
/* 4257 */    MCD::OPC_CheckPredicate, 6, 206, 8, 0, // Skip to: 6516
/* 4262 */    MCD::OPC_Decode, 240, 6, 90, // Opcode: BLTZALS_MM
/* 4266 */    MCD::OPC_FilterValue, 19, 9, 0, 0, // Skip to: 4280
/* 4271 */    MCD::OPC_CheckPredicate, 6, 192, 8, 0, // Skip to: 6516
/* 4276 */    MCD::OPC_Decode, 183, 6, 90, // Opcode: BGEZALS_MM
/* 4280 */    MCD::OPC_FilterValue, 25, 16, 0, 0, // Skip to: 4301
/* 4285 */    MCD::OPC_CheckPredicate, 11, 178, 8, 0, // Skip to: 6516
/* 4290 */    MCD::OPC_CheckField, 16, 5, 0, 171, 8, 0, // Skip to: 6516
/* 4297 */    MCD::OPC_Decode, 156, 7, 94, // Opcode: BPOSGE32C_MMR3
/* 4301 */    MCD::OPC_FilterValue, 27, 16, 0, 0, // Skip to: 4322
/* 4306 */    MCD::OPC_CheckPredicate, 12, 157, 8, 0, // Skip to: 6516
/* 4311 */    MCD::OPC_CheckField, 16, 5, 0, 150, 8, 0, // Skip to: 6516
/* 4318 */    MCD::OPC_Decode, 157, 7, 95, // Opcode: BPOSGE32_MM
/* 4322 */    MCD::OPC_FilterValue, 28, 16, 0, 0, // Skip to: 4343
/* 4327 */    MCD::OPC_CheckPredicate, 13, 136, 8, 0, // Skip to: 6516
/* 4332 */    MCD::OPC_CheckField, 16, 2, 0, 129, 8, 0, // Skip to: 6516
/* 4339 */    MCD::OPC_Decode, 137, 6, 96, // Opcode: BC1F_MM
/* 4343 */    MCD::OPC_FilterValue, 29, 120, 8, 0, // Skip to: 6516
/* 4348 */    MCD::OPC_CheckPredicate, 13, 115, 8, 0, // Skip to: 6516
/* 4353 */    MCD::OPC_CheckField, 16, 2, 0, 108, 8, 0, // Skip to: 6516
/* 4360 */    MCD::OPC_Decode, 142, 6, 96, // Opcode: BC1T_MM
/* 4364 */    MCD::OPC_FilterValue, 20, 9, 0, 0, // Skip to: 4378
/* 4369 */    MCD::OPC_CheckPredicate, 6, 94, 8, 0, // Skip to: 6516
/* 4374 */    MCD::OPC_Decode, 137, 17, 97, // Opcode: ORi_MM
/* 4378 */    MCD::OPC_FilterValue, 21, 197, 5, 0, // Skip to: 5860
/* 4383 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 4386 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 4400
/* 4391 */    MCD::OPC_CheckPredicate, 14, 72, 8, 0, // Skip to: 6516
/* 4396 */    MCD::OPC_Decode, 210, 14, 98, // Opcode: MADD_S_MM
/* 4400 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 4414
/* 4405 */    MCD::OPC_CheckPredicate, 14, 58, 8, 0, // Skip to: 6516
/* 4410 */    MCD::OPC_Decode, 238, 16, 98, // Opcode: NMADD_S_MM
/* 4414 */    MCD::OPC_FilterValue, 8, 59, 0, 0, // Skip to: 4478
/* 4419 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4422 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 4436
/* 4427 */    MCD::OPC_CheckPredicate, 13, 36, 8, 0, // Skip to: 6516
/* 4432 */    MCD::OPC_Decode, 169, 14, 99, // Opcode: LWXC1_MM
/* 4436 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 4450
/* 4441 */    MCD::OPC_CheckPredicate, 13, 22, 8, 0, // Skip to: 6516
/* 4446 */    MCD::OPC_Decode, 196, 20, 99, // Opcode: SWXC1_MM
/* 4450 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 4464
/* 4455 */    MCD::OPC_CheckPredicate, 15, 8, 8, 0, // Skip to: 6516
/* 4460 */    MCD::OPC_Decode, 130, 14, 100, // Opcode: LUXC1_MM
/* 4464 */    MCD::OPC_FilterValue, 6, 255, 7, 0, // Skip to: 6516
/* 4469 */    MCD::OPC_CheckPredicate, 15, 250, 7, 0, // Skip to: 6516
/* 4474 */    MCD::OPC_Decode, 164, 20, 100, // Opcode: SUXC1_MM
/* 4478 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 4492
/* 4483 */    MCD::OPC_CheckPredicate, 16, 236, 7, 0, // Skip to: 6516
/* 4488 */    MCD::OPC_Decode, 202, 14, 101, // Opcode: MADD_D32_MM
/* 4492 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 4506
/* 4497 */    MCD::OPC_CheckPredicate, 16, 222, 7, 0, // Skip to: 6516
/* 4502 */    MCD::OPC_Decode, 235, 16, 101, // Opcode: NMADD_D32_MM
/* 4506 */    MCD::OPC_FilterValue, 32, 101, 0, 0, // Skip to: 4612
/* 4511 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4514 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 4535
/* 4519 */    MCD::OPC_CheckPredicate, 13, 200, 7, 0, // Skip to: 6516
/* 4524 */    MCD::OPC_CheckField, 11, 2, 0, 193, 7, 0, // Skip to: 6516
/* 4531 */    MCD::OPC_Decode, 204, 15, 102, // Opcode: MOVF_S_MM
/* 4535 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 4556
/* 4540 */    MCD::OPC_CheckPredicate, 13, 179, 7, 0, // Skip to: 6516
/* 4545 */    MCD::OPC_CheckField, 11, 2, 0, 172, 7, 0, // Skip to: 6516
/* 4552 */    MCD::OPC_Decode, 224, 15, 102, // Opcode: MOVT_S_MM
/* 4556 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 4570
/* 4561 */    MCD::OPC_CheckPredicate, 6, 158, 7, 0, // Skip to: 6516
/* 4566 */    MCD::OPC_Decode, 200, 17, 103, // Opcode: PREFX_MM
/* 4570 */    MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 4591
/* 4575 */    MCD::OPC_CheckPredicate, 17, 144, 7, 0, // Skip to: 6516
/* 4580 */    MCD::OPC_CheckField, 11, 2, 0, 137, 7, 0, // Skip to: 6516
/* 4587 */    MCD::OPC_Decode, 198, 15, 104, // Opcode: MOVF_D32_MM
/* 4591 */    MCD::OPC_FilterValue, 9, 128, 7, 0, // Skip to: 6516
/* 4596 */    MCD::OPC_CheckPredicate, 17, 123, 7, 0, // Skip to: 6516
/* 4601 */    MCD::OPC_CheckField, 11, 2, 0, 116, 7, 0, // Skip to: 6516
/* 4608 */    MCD::OPC_Decode, 218, 15, 104, // Opcode: MOVT_D32_MM
/* 4612 */    MCD::OPC_FilterValue, 33, 9, 0, 0, // Skip to: 4626
/* 4617 */    MCD::OPC_CheckPredicate, 14, 102, 7, 0, // Skip to: 6516
/* 4622 */    MCD::OPC_Decode, 133, 16, 98, // Opcode: MSUB_S_MM
/* 4626 */    MCD::OPC_FilterValue, 34, 9, 0, 0, // Skip to: 4640
/* 4631 */    MCD::OPC_CheckPredicate, 14, 88, 7, 0, // Skip to: 6516
/* 4636 */    MCD::OPC_Decode, 243, 16, 98, // Opcode: NMSUB_S_MM
/* 4640 */    MCD::OPC_FilterValue, 41, 9, 0, 0, // Skip to: 4654
/* 4645 */    MCD::OPC_CheckPredicate, 16, 74, 7, 0, // Skip to: 6516
/* 4650 */    MCD::OPC_Decode, 253, 15, 101, // Opcode: MSUB_D32_MM
/* 4654 */    MCD::OPC_FilterValue, 42, 9, 0, 0, // Skip to: 4668
/* 4659 */    MCD::OPC_CheckPredicate, 16, 60, 7, 0, // Skip to: 6516
/* 4664 */    MCD::OPC_Decode, 240, 16, 101, // Opcode: NMSUB_D32_MM
/* 4668 */    MCD::OPC_FilterValue, 48, 59, 0, 0, // Skip to: 4732
/* 4673 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4676 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 4690
/* 4681 */    MCD::OPC_CheckPredicate, 18, 38, 7, 0, // Skip to: 6516
/* 4686 */    MCD::OPC_Decode, 184, 11, 105, // Opcode: FADD_D32_MM
/* 4690 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 4704
/* 4695 */    MCD::OPC_CheckPredicate, 18, 24, 7, 0, // Skip to: 6516
/* 4700 */    MCD::OPC_Decode, 199, 12, 105, // Opcode: FSUB_D32_MM
/* 4704 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 4718
/* 4709 */    MCD::OPC_CheckPredicate, 18, 10, 7, 0, // Skip to: 6516
/* 4714 */    MCD::OPC_Decode, 156, 12, 105, // Opcode: FMUL_D32_MM
/* 4718 */    MCD::OPC_FilterValue, 7, 1, 7, 0, // Skip to: 6516
/* 4723 */    MCD::OPC_CheckPredicate, 18, 252, 6, 0, // Skip to: 6516
/* 4728 */    MCD::OPC_Decode, 222, 11, 105, // Opcode: FDIV_D32_MM
/* 4732 */    MCD::OPC_FilterValue, 56, 59, 0, 0, // Skip to: 4796
/* 4737 */    MCD::OPC_ExtractField, 6, 4,  // Inst{9-6} ...
/* 4740 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 4754
/* 4745 */    MCD::OPC_CheckPredicate, 13, 230, 6, 0, // Skip to: 6516
/* 4750 */    MCD::OPC_Decode, 216, 15, 106, // Opcode: MOVN_I_S_MM
/* 4754 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 4768
/* 4759 */    MCD::OPC_CheckPredicate, 13, 216, 6, 0, // Skip to: 6516
/* 4764 */    MCD::OPC_Decode, 236, 15, 106, // Opcode: MOVZ_I_S_MM
/* 4768 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 4782
/* 4773 */    MCD::OPC_CheckPredicate, 17, 202, 6, 0, // Skip to: 6516
/* 4778 */    MCD::OPC_Decode, 210, 15, 107, // Opcode: MOVN_I_D32_MM
/* 4782 */    MCD::OPC_FilterValue, 5, 193, 6, 0, // Skip to: 6516
/* 4787 */    MCD::OPC_CheckPredicate, 17, 188, 6, 0, // Skip to: 6516
/* 4792 */    MCD::OPC_Decode, 230, 15, 107, // Opcode: MOVZ_I_D32_MM
/* 4796 */    MCD::OPC_FilterValue, 59, 91, 2, 0, // Skip to: 5404
/* 4801 */    MCD::OPC_ExtractField, 6, 7,  // Inst{12-6} ...
/* 4804 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 4825
/* 4809 */    MCD::OPC_CheckPredicate, 19, 166, 6, 0, // Skip to: 6516
/* 4814 */    MCD::OPC_CheckField, 13, 3, 1, 159, 6, 0, // Skip to: 6516
/* 4821 */    MCD::OPC_Decode, 251, 14, 108, // Opcode: MFC1_MM
/* 4825 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 4846
/* 4830 */    MCD::OPC_CheckPredicate, 18, 145, 6, 0, // Skip to: 6516
/* 4835 */    MCD::OPC_CheckField, 13, 3, 1, 138, 6, 0, // Skip to: 6516
/* 4842 */    MCD::OPC_Decode, 145, 12, 109, // Opcode: FMOV_D32_MM
/* 4846 */    MCD::OPC_FilterValue, 4, 31, 0, 0, // Skip to: 4882
/* 4851 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4854 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 4868
/* 4859 */    MCD::OPC_CheckPredicate, 20, 116, 6, 0, // Skip to: 6516
/* 4864 */    MCD::OPC_Decode, 131, 9, 110, // Opcode: CVT_L_S_MM
/* 4868 */    MCD::OPC_FilterValue, 2, 107, 6, 0, // Skip to: 6516
/* 4873 */    MCD::OPC_CheckPredicate, 20, 102, 6, 0, // Skip to: 6516
/* 4878 */    MCD::OPC_Decode, 128, 9, 111, // Opcode: CVT_L_D64_MM
/* 4882 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 4896
/* 4887 */    MCD::OPC_CheckPredicate, 13, 88, 6, 0, // Skip to: 6516
/* 4892 */    MCD::OPC_Decode, 202, 15, 112, // Opcode: MOVF_I_MM
/* 4896 */    MCD::OPC_FilterValue, 8, 31, 0, 0, // Skip to: 4932
/* 4901 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4904 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 4918
/* 4909 */    MCD::OPC_CheckPredicate, 19, 66, 6, 0, // Skip to: 6516
/* 4914 */    MCD::OPC_Decode, 253, 17, 113, // Opcode: RSQRT_S_MM
/* 4918 */    MCD::OPC_FilterValue, 2, 57, 6, 0, // Skip to: 6516
/* 4923 */    MCD::OPC_CheckPredicate, 18, 52, 6, 0, // Skip to: 6516
/* 4928 */    MCD::OPC_Decode, 249, 17, 109, // Opcode: RSQRT_D32_MM
/* 4932 */    MCD::OPC_FilterValue, 13, 31, 0, 0, // Skip to: 4968
/* 4937 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4940 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 4954
/* 4945 */    MCD::OPC_CheckPredicate, 19, 30, 6, 0, // Skip to: 6516
/* 4950 */    MCD::OPC_Decode, 181, 11, 113, // Opcode: FABS_S_MM
/* 4954 */    MCD::OPC_FilterValue, 1, 21, 6, 0, // Skip to: 6516
/* 4959 */    MCD::OPC_CheckPredicate, 18, 16, 6, 0, // Skip to: 6516
/* 4964 */    MCD::OPC_Decode, 177, 11, 109, // Opcode: FABS_D32_MM
/* 4968 */    MCD::OPC_FilterValue, 32, 16, 0, 0, // Skip to: 4989
/* 4973 */    MCD::OPC_CheckPredicate, 19, 2, 6, 0, // Skip to: 6516
/* 4978 */    MCD::OPC_CheckField, 13, 3, 1, 251, 5, 0, // Skip to: 6516
/* 4985 */    MCD::OPC_Decode, 139, 16, 114, // Opcode: MTC1_MM
/* 4989 */    MCD::OPC_FilterValue, 36, 31, 0, 0, // Skip to: 5025
/* 4994 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4997 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5011
/* 5002 */    MCD::OPC_CheckPredicate, 19, 229, 5, 0, // Skip to: 6516
/* 5007 */    MCD::OPC_Decode, 150, 9, 113, // Opcode: CVT_W_S_MM
/* 5011 */    MCD::OPC_FilterValue, 2, 220, 5, 0, // Skip to: 6516
/* 5016 */    MCD::OPC_CheckPredicate, 18, 215, 5, 0, // Skip to: 6516
/* 5021 */    MCD::OPC_Decode, 146, 9, 115, // Opcode: CVT_W_D32_MM
/* 5025 */    MCD::OPC_FilterValue, 37, 9, 0, 0, // Skip to: 5039
/* 5030 */    MCD::OPC_CheckPredicate, 13, 201, 5, 0, // Skip to: 6516
/* 5035 */    MCD::OPC_Decode, 222, 15, 112, // Opcode: MOVT_I_MM
/* 5039 */    MCD::OPC_FilterValue, 40, 31, 0, 0, // Skip to: 5075
/* 5044 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5047 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5061
/* 5052 */    MCD::OPC_CheckPredicate, 19, 179, 5, 0, // Skip to: 6516
/* 5057 */    MCD::OPC_Decode, 195, 12, 113, // Opcode: FSQRT_S_MM
/* 5061 */    MCD::OPC_FilterValue, 2, 170, 5, 0, // Skip to: 6516
/* 5066 */    MCD::OPC_CheckPredicate, 18, 165, 5, 0, // Skip to: 6516
/* 5071 */    MCD::OPC_Decode, 191, 12, 109, // Opcode: FSQRT_D32_MM
/* 5075 */    MCD::OPC_FilterValue, 44, 59, 0, 0, // Skip to: 5139
/* 5080 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5083 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5097
/* 5088 */    MCD::OPC_CheckPredicate, 19, 143, 5, 0, // Skip to: 6516
/* 5093 */    MCD::OPC_Decode, 132, 12, 113, // Opcode: FLOOR_W_S_MM
/* 5097 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 5111
/* 5102 */    MCD::OPC_CheckPredicate, 19, 129, 5, 0, // Skip to: 6516
/* 5107 */    MCD::OPC_Decode, 156, 21, 113, // Opcode: TRUNC_W_S_MM
/* 5111 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 5125
/* 5116 */    MCD::OPC_CheckPredicate, 18, 115, 5, 0, // Skip to: 6516
/* 5121 */    MCD::OPC_Decode, 130, 12, 115, // Opcode: FLOOR_W_MM
/* 5125 */    MCD::OPC_FilterValue, 3, 106, 5, 0, // Skip to: 6516
/* 5130 */    MCD::OPC_CheckPredicate, 18, 101, 5, 0, // Skip to: 6516
/* 5135 */    MCD::OPC_Decode, 154, 21, 115, // Opcode: TRUNC_W_MM
/* 5139 */    MCD::OPC_FilterValue, 45, 16, 0, 0, // Skip to: 5160
/* 5144 */    MCD::OPC_CheckPredicate, 18, 87, 5, 0, // Skip to: 6516
/* 5149 */    MCD::OPC_CheckField, 13, 3, 1, 80, 5, 0, // Skip to: 6516
/* 5156 */    MCD::OPC_Decode, 164, 12, 109, // Opcode: FNEG_D32_MM
/* 5160 */    MCD::OPC_FilterValue, 64, 31, 0, 0, // Skip to: 5196
/* 5165 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5168 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5182
/* 5173 */    MCD::OPC_CheckPredicate, 19, 58, 5, 0, // Skip to: 6516
/* 5178 */    MCD::OPC_Decode, 215, 7, 116, // Opcode: CFC1_MM
/* 5182 */    MCD::OPC_FilterValue, 1, 49, 5, 0, // Skip to: 6516
/* 5187 */    MCD::OPC_CheckPredicate, 18, 44, 5, 0, // Skip to: 6516
/* 5192 */    MCD::OPC_Decode, 131, 15, 117, // Opcode: MFHC1_D32_MM
/* 5196 */    MCD::OPC_FilterValue, 72, 31, 0, 0, // Skip to: 5232
/* 5201 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5204 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5218
/* 5209 */    MCD::OPC_CheckPredicate, 19, 22, 5, 0, // Skip to: 6516
/* 5214 */    MCD::OPC_Decode, 220, 17, 113, // Opcode: RECIP_S_MM
/* 5218 */    MCD::OPC_FilterValue, 2, 13, 5, 0, // Skip to: 6516
/* 5223 */    MCD::OPC_CheckPredicate, 18, 8, 5, 0, // Skip to: 6516
/* 5228 */    MCD::OPC_Decode, 216, 17, 109, // Opcode: RECIP_D32_MM
/* 5232 */    MCD::OPC_FilterValue, 77, 31, 0, 0, // Skip to: 5268
/* 5237 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5240 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5254
/* 5245 */    MCD::OPC_CheckPredicate, 18, 242, 4, 0, // Skip to: 6516
/* 5250 */    MCD::OPC_Decode, 246, 8, 118, // Opcode: CVT_D32_S_MM
/* 5254 */    MCD::OPC_FilterValue, 1, 233, 4, 0, // Skip to: 6516
/* 5259 */    MCD::OPC_CheckPredicate, 18, 228, 4, 0, // Skip to: 6516
/* 5264 */    MCD::OPC_Decode, 248, 8, 118, // Opcode: CVT_D32_W_MM
/* 5268 */    MCD::OPC_FilterValue, 96, 31, 0, 0, // Skip to: 5304
/* 5273 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5276 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5290
/* 5281 */    MCD::OPC_CheckPredicate, 19, 206, 4, 0, // Skip to: 6516
/* 5286 */    MCD::OPC_Decode, 242, 8, 119, // Opcode: CTC1_MM
/* 5290 */    MCD::OPC_FilterValue, 1, 197, 4, 0, // Skip to: 6516
/* 5295 */    MCD::OPC_CheckPredicate, 18, 192, 4, 0, // Skip to: 6516
/* 5300 */    MCD::OPC_Decode, 147, 16, 120, // Opcode: MTHC1_D32_MM
/* 5304 */    MCD::OPC_FilterValue, 108, 59, 0, 0, // Skip to: 5368
/* 5309 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5312 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5326
/* 5317 */    MCD::OPC_CheckPredicate, 19, 170, 4, 0, // Skip to: 6516
/* 5322 */    MCD::OPC_Decode, 204, 7, 113, // Opcode: CEIL_W_S_MM
/* 5326 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 5340
/* 5331 */    MCD::OPC_CheckPredicate, 19, 156, 4, 0, // Skip to: 6516
/* 5336 */    MCD::OPC_Decode, 246, 17, 113, // Opcode: ROUND_W_S_MM
/* 5340 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 5354
/* 5345 */    MCD::OPC_CheckPredicate, 18, 142, 4, 0, // Skip to: 6516
/* 5350 */    MCD::OPC_Decode, 202, 7, 115, // Opcode: CEIL_W_MM
/* 5354 */    MCD::OPC_FilterValue, 3, 133, 4, 0, // Skip to: 6516
/* 5359 */    MCD::OPC_CheckPredicate, 18, 128, 4, 0, // Skip to: 6516
/* 5364 */    MCD::OPC_Decode, 244, 17, 115, // Opcode: ROUND_W_MM
/* 5368 */    MCD::OPC_FilterValue, 109, 119, 4, 0, // Skip to: 6516
/* 5373 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 5376 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5390
/* 5381 */    MCD::OPC_CheckPredicate, 18, 106, 4, 0, // Skip to: 6516
/* 5386 */    MCD::OPC_Decode, 135, 9, 115, // Opcode: CVT_S_D32_MM
/* 5390 */    MCD::OPC_FilterValue, 1, 97, 4, 0, // Skip to: 6516
/* 5395 */    MCD::OPC_CheckPredicate, 19, 92, 4, 0, // Skip to: 6516
/* 5400 */    MCD::OPC_Decode, 143, 9, 113, // Opcode: CVT_S_W_MM
/* 5404 */    MCD::OPC_FilterValue, 60, 83, 4, 0, // Skip to: 6516
/* 5409 */    MCD::OPC_ExtractField, 6, 7,  // Inst{12-6} ...
/* 5412 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5426
/* 5417 */    MCD::OPC_CheckPredicate, 13, 70, 4, 0, // Skip to: 6516
/* 5422 */    MCD::OPC_Decode, 163, 9, 121, // Opcode: C_F_S_MM
/* 5426 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 5440
/* 5431 */    MCD::OPC_CheckPredicate, 13, 56, 4, 0, // Skip to: 6516
/* 5436 */    MCD::OPC_Decode, 247, 9, 121, // Opcode: C_UN_S_MM
/* 5440 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 5454
/* 5445 */    MCD::OPC_CheckPredicate, 13, 42, 4, 0, // Skip to: 6516
/* 5450 */    MCD::OPC_Decode, 157, 9, 121, // Opcode: C_EQ_S_MM
/* 5454 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 5468
/* 5459 */    MCD::OPC_CheckPredicate, 13, 28, 4, 0, // Skip to: 6516
/* 5464 */    MCD::OPC_Decode, 229, 9, 121, // Opcode: C_UEQ_S_MM
/* 5468 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 5482
/* 5473 */    MCD::OPC_CheckPredicate, 13, 14, 4, 0, // Skip to: 6516
/* 5478 */    MCD::OPC_Decode, 211, 9, 121, // Opcode: C_OLT_S_MM
/* 5482 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 5496
/* 5487 */    MCD::OPC_CheckPredicate, 13, 0, 4, 0, // Skip to: 6516
/* 5492 */    MCD::OPC_Decode, 241, 9, 121, // Opcode: C_ULT_S_MM
/* 5496 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 5510
/* 5501 */    MCD::OPC_CheckPredicate, 13, 242, 3, 0, // Skip to: 6516
/* 5506 */    MCD::OPC_Decode, 205, 9, 121, // Opcode: C_OLE_S_MM
/* 5510 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 5524
/* 5515 */    MCD::OPC_CheckPredicate, 13, 228, 3, 0, // Skip to: 6516
/* 5520 */    MCD::OPC_Decode, 235, 9, 121, // Opcode: C_ULE_S_MM
/* 5524 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 5538
/* 5529 */    MCD::OPC_CheckPredicate, 13, 214, 3, 0, // Skip to: 6516
/* 5534 */    MCD::OPC_Decode, 223, 9, 121, // Opcode: C_SF_S_MM
/* 5538 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 5552
/* 5543 */    MCD::OPC_CheckPredicate, 13, 200, 3, 0, // Skip to: 6516
/* 5548 */    MCD::OPC_Decode, 187, 9, 121, // Opcode: C_NGLE_S_MM
/* 5552 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 5566
/* 5557 */    MCD::OPC_CheckPredicate, 13, 186, 3, 0, // Skip to: 6516
/* 5562 */    MCD::OPC_Decode, 217, 9, 121, // Opcode: C_SEQ_S_MM
/* 5566 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 5580
/* 5571 */    MCD::OPC_CheckPredicate, 13, 172, 3, 0, // Skip to: 6516
/* 5576 */    MCD::OPC_Decode, 193, 9, 121, // Opcode: C_NGL_S_MM
/* 5580 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 5594
/* 5585 */    MCD::OPC_CheckPredicate, 13, 158, 3, 0, // Skip to: 6516
/* 5590 */    MCD::OPC_Decode, 175, 9, 121, // Opcode: C_LT_S_MM
/* 5594 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 5608
/* 5599 */    MCD::OPC_CheckPredicate, 13, 144, 3, 0, // Skip to: 6516
/* 5604 */    MCD::OPC_Decode, 181, 9, 121, // Opcode: C_NGE_S_MM
/* 5608 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 5622
/* 5613 */    MCD::OPC_CheckPredicate, 13, 130, 3, 0, // Skip to: 6516
/* 5618 */    MCD::OPC_Decode, 169, 9, 121, // Opcode: C_LE_S_MM
/* 5622 */    MCD::OPC_FilterValue, 15, 9, 0, 0, // Skip to: 5636
/* 5627 */    MCD::OPC_CheckPredicate, 13, 116, 3, 0, // Skip to: 6516
/* 5632 */    MCD::OPC_Decode, 199, 9, 121, // Opcode: C_NGT_S_MM
/* 5636 */    MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 5650
/* 5641 */    MCD::OPC_CheckPredicate, 17, 102, 3, 0, // Skip to: 6516
/* 5646 */    MCD::OPC_Decode, 159, 9, 122, // Opcode: C_F_D32_MM
/* 5650 */    MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 5664
/* 5655 */    MCD::OPC_CheckPredicate, 17, 88, 3, 0, // Skip to: 6516
/* 5660 */    MCD::OPC_Decode, 243, 9, 122, // Opcode: C_UN_D32_MM
/* 5664 */    MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 5678
/* 5669 */    MCD::OPC_CheckPredicate, 17, 74, 3, 0, // Skip to: 6516
/* 5674 */    MCD::OPC_Decode, 153, 9, 122, // Opcode: C_EQ_D32_MM
/* 5678 */    MCD::OPC_FilterValue, 19, 9, 0, 0, // Skip to: 5692
/* 5683 */    MCD::OPC_CheckPredicate, 17, 60, 3, 0, // Skip to: 6516
/* 5688 */    MCD::OPC_Decode, 225, 9, 122, // Opcode: C_UEQ_D32_MM
/* 5692 */    MCD::OPC_FilterValue, 20, 9, 0, 0, // Skip to: 5706
/* 5697 */    MCD::OPC_CheckPredicate, 17, 46, 3, 0, // Skip to: 6516
/* 5702 */    MCD::OPC_Decode, 207, 9, 122, // Opcode: C_OLT_D32_MM
/* 5706 */    MCD::OPC_FilterValue, 21, 9, 0, 0, // Skip to: 5720
/* 5711 */    MCD::OPC_CheckPredicate, 17, 32, 3, 0, // Skip to: 6516
/* 5716 */    MCD::OPC_Decode, 237, 9, 122, // Opcode: C_ULT_D32_MM
/* 5720 */    MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 5734
/* 5725 */    MCD::OPC_CheckPredicate, 17, 18, 3, 0, // Skip to: 6516
/* 5730 */    MCD::OPC_Decode, 201, 9, 122, // Opcode: C_OLE_D32_MM
/* 5734 */    MCD::OPC_FilterValue, 23, 9, 0, 0, // Skip to: 5748
/* 5739 */    MCD::OPC_CheckPredicate, 17, 4, 3, 0, // Skip to: 6516
/* 5744 */    MCD::OPC_Decode, 231, 9, 122, // Opcode: C_ULE_D32_MM
/* 5748 */    MCD::OPC_FilterValue, 24, 9, 0, 0, // Skip to: 5762
/* 5753 */    MCD::OPC_CheckPredicate, 17, 246, 2, 0, // Skip to: 6516
/* 5758 */    MCD::OPC_Decode, 219, 9, 122, // Opcode: C_SF_D32_MM
/* 5762 */    MCD::OPC_FilterValue, 25, 9, 0, 0, // Skip to: 5776
/* 5767 */    MCD::OPC_CheckPredicate, 17, 232, 2, 0, // Skip to: 6516
/* 5772 */    MCD::OPC_Decode, 183, 9, 122, // Opcode: C_NGLE_D32_MM
/* 5776 */    MCD::OPC_FilterValue, 26, 9, 0, 0, // Skip to: 5790
/* 5781 */    MCD::OPC_CheckPredicate, 17, 218, 2, 0, // Skip to: 6516
/* 5786 */    MCD::OPC_Decode, 213, 9, 122, // Opcode: C_SEQ_D32_MM
/* 5790 */    MCD::OPC_FilterValue, 27, 9, 0, 0, // Skip to: 5804
/* 5795 */    MCD::OPC_CheckPredicate, 17, 204, 2, 0, // Skip to: 6516
/* 5800 */    MCD::OPC_Decode, 189, 9, 122, // Opcode: C_NGL_D32_MM
/* 5804 */    MCD::OPC_FilterValue, 28, 9, 0, 0, // Skip to: 5818
/* 5809 */    MCD::OPC_CheckPredicate, 17, 190, 2, 0, // Skip to: 6516
/* 5814 */    MCD::OPC_Decode, 171, 9, 122, // Opcode: C_LT_D32_MM
/* 5818 */    MCD::OPC_FilterValue, 29, 9, 0, 0, // Skip to: 5832
/* 5823 */    MCD::OPC_CheckPredicate, 17, 176, 2, 0, // Skip to: 6516
/* 5828 */    MCD::OPC_Decode, 177, 9, 122, // Opcode: C_NGE_D32_MM
/* 5832 */    MCD::OPC_FilterValue, 30, 9, 0, 0, // Skip to: 5846
/* 5837 */    MCD::OPC_CheckPredicate, 17, 162, 2, 0, // Skip to: 6516
/* 5842 */    MCD::OPC_Decode, 165, 9, 122, // Opcode: C_LE_D32_MM
/* 5846 */    MCD::OPC_FilterValue, 31, 153, 2, 0, // Skip to: 6516
/* 5851 */    MCD::OPC_CheckPredicate, 17, 148, 2, 0, // Skip to: 6516
/* 5856 */    MCD::OPC_Decode, 195, 9, 122, // Opcode: C_NGT_D32_MM
/* 5860 */    MCD::OPC_FilterValue, 22, 48, 0, 0, // Skip to: 5913
/* 5865 */    MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 5868 */    MCD::OPC_FilterValue, 197, 1, 9, 0, 0, // Skip to: 5883
/* 5874 */    MCD::OPC_CheckPredicate, 8, 125, 2, 0, // Skip to: 6516
/* 5879 */    MCD::OPC_Decode, 145, 8, 43, // Opcode: CMPGU_EQ_QB_MM
/* 5883 */    MCD::OPC_FilterValue, 133, 2, 9, 0, 0, // Skip to: 5898
/* 5889 */    MCD::OPC_CheckPredicate, 8, 110, 2, 0, // Skip to: 6516
/* 5894 */    MCD::OPC_Decode, 149, 8, 43, // Opcode: CMPGU_LT_QB_MM
/* 5898 */    MCD::OPC_FilterValue, 197, 2, 100, 2, 0, // Skip to: 6516
/* 5904 */    MCD::OPC_CheckPredicate, 8, 95, 2, 0, // Skip to: 6516
/* 5909 */    MCD::OPC_Decode, 147, 8, 43, // Opcode: CMPGU_LE_QB_MM
/* 5913 */    MCD::OPC_FilterValue, 24, 99, 1, 0, // Skip to: 6273
/* 5918 */    MCD::OPC_ExtractField, 12, 4,  // Inst{15-12} ...
/* 5921 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5935
/* 5926 */    MCD::OPC_CheckPredicate, 6, 73, 2, 0, // Skip to: 6516
/* 5931 */    MCD::OPC_Decode, 152, 14, 88, // Opcode: LWL_MM
/* 5935 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 5949
/* 5940 */    MCD::OPC_CheckPredicate, 6, 59, 2, 0, // Skip to: 6516
/* 5945 */    MCD::OPC_Decode, 163, 14, 88, // Opcode: LWR_MM
/* 5949 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 5963
/* 5954 */    MCD::OPC_CheckPredicate, 6, 45, 2, 0, // Skip to: 6516
/* 5959 */    MCD::OPC_Decode, 201, 17, 89, // Opcode: PREF_MM
/* 5963 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 5977
/* 5968 */    MCD::OPC_CheckPredicate, 6, 31, 2, 0, // Skip to: 6516
/* 5973 */    MCD::OPC_Decode, 249, 13, 88, // Opcode: LL_MM
/* 5977 */    MCD::OPC_FilterValue, 6, 115, 0, 0, // Skip to: 6097
/* 5982 */    MCD::OPC_ExtractField, 9, 3,  // Inst{11-9} ...
/* 5985 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 5999
/* 5990 */    MCD::OPC_CheckPredicate, 21, 9, 2, 0, // Skip to: 6516
/* 5995 */    MCD::OPC_Decode, 200, 13, 123, // Opcode: LBuE_MM
/* 5999 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 6013
/* 6004 */    MCD::OPC_CheckPredicate, 21, 251, 1, 0, // Skip to: 6516
/* 6009 */    MCD::OPC_Decode, 238, 13, 123, // Opcode: LHuE_MM
/* 6013 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 6027
/* 6018 */    MCD::OPC_CheckPredicate, 22, 237, 1, 0, // Skip to: 6516
/* 6023 */    MCD::OPC_Decode, 151, 14, 123, // Opcode: LWLE_MM
/* 6027 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 6041
/* 6032 */    MCD::OPC_CheckPredicate, 22, 223, 1, 0, // Skip to: 6516
/* 6037 */    MCD::OPC_Decode, 162, 14, 123, // Opcode: LWRE_MM
/* 6041 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 6055
/* 6046 */    MCD::OPC_CheckPredicate, 21, 209, 1, 0, // Skip to: 6516
/* 6051 */    MCD::OPC_Decode, 190, 13, 123, // Opcode: LBE_MM
/* 6055 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 6069
/* 6060 */    MCD::OPC_CheckPredicate, 21, 195, 1, 0, // Skip to: 6516
/* 6065 */    MCD::OPC_Decode, 230, 13, 123, // Opcode: LHE_MM
/* 6069 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 6083
/* 6074 */    MCD::OPC_CheckPredicate, 21, 181, 1, 0, // Skip to: 6516
/* 6079 */    MCD::OPC_Decode, 248, 13, 123, // Opcode: LLE_MM
/* 6083 */    MCD::OPC_FilterValue, 7, 172, 1, 0, // Skip to: 6516
/* 6088 */    MCD::OPC_CheckPredicate, 21, 167, 1, 0, // Skip to: 6516
/* 6093 */    MCD::OPC_Decode, 146, 14, 123, // Opcode: LWE_MM
/* 6097 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 6111
/* 6102 */    MCD::OPC_CheckPredicate, 6, 153, 1, 0, // Skip to: 6516
/* 6107 */    MCD::OPC_Decode, 183, 20, 88, // Opcode: SWL_MM
/* 6111 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 6125
/* 6116 */    MCD::OPC_CheckPredicate, 6, 139, 1, 0, // Skip to: 6516
/* 6121 */    MCD::OPC_Decode, 192, 20, 88, // Opcode: SWR_MM
/* 6125 */    MCD::OPC_FilterValue, 10, 115, 0, 0, // Skip to: 6245
/* 6130 */    MCD::OPC_ExtractField, 9, 3,  // Inst{11-9} ...
/* 6133 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 6147
/* 6138 */    MCD::OPC_CheckPredicate, 22, 117, 1, 0, // Skip to: 6516
/* 6143 */    MCD::OPC_Decode, 182, 20, 123, // Opcode: SWLE_MM
/* 6147 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 6161
/* 6152 */    MCD::OPC_CheckPredicate, 22, 103, 1, 0, // Skip to: 6516
/* 6157 */    MCD::OPC_Decode, 191, 20, 123, // Opcode: SWRE_MM
/* 6161 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 6175
/* 6166 */    MCD::OPC_CheckPredicate, 21, 89, 1, 0, // Skip to: 6516
/* 6171 */    MCD::OPC_Decode, 199, 17, 124, // Opcode: PREFE_MM
/* 6175 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 6189
/* 6180 */    MCD::OPC_CheckPredicate, 21, 75, 1, 0, // Skip to: 6516
/* 6185 */    MCD::OPC_Decode, 191, 7, 124, // Opcode: CACHEE_MM
/* 6189 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 6203
/* 6194 */    MCD::OPC_CheckPredicate, 21, 61, 1, 0, // Skip to: 6516
/* 6199 */    MCD::OPC_Decode, 143, 18, 123, // Opcode: SBE_MM
/* 6203 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 6217
/* 6208 */    MCD::OPC_CheckPredicate, 21, 47, 1, 0, // Skip to: 6516
/* 6213 */    MCD::OPC_Decode, 208, 18, 123, // Opcode: SHE_MM
/* 6217 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 6231
/* 6222 */    MCD::OPC_CheckPredicate, 21, 33, 1, 0, // Skip to: 6516
/* 6227 */    MCD::OPC_Decode, 152, 18, 123, // Opcode: SCE_MM
/* 6231 */    MCD::OPC_FilterValue, 7, 24, 1, 0, // Skip to: 6516
/* 6236 */    MCD::OPC_CheckPredicate, 21, 19, 1, 0, // Skip to: 6516
/* 6241 */    MCD::OPC_Decode, 178, 20, 123, // Opcode: SWE_MM
/* 6245 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 6259
/* 6250 */    MCD::OPC_CheckPredicate, 6, 5, 1, 0, // Skip to: 6516
/* 6255 */    MCD::OPC_Decode, 153, 18, 88, // Opcode: SC_MM
/* 6259 */    MCD::OPC_FilterValue, 14, 252, 0, 0, // Skip to: 6516
/* 6264 */    MCD::OPC_CheckPredicate, 6, 247, 0, 0, // Skip to: 6516
/* 6269 */    MCD::OPC_Decode, 166, 14, 88, // Opcode: LWU_MM
/* 6273 */    MCD::OPC_FilterValue, 28, 9, 0, 0, // Skip to: 6287
/* 6278 */    MCD::OPC_CheckPredicate, 6, 233, 0, 0, // Skip to: 6516
/* 6283 */    MCD::OPC_Decode, 188, 21, 97, // Opcode: XORi_MM
/* 6287 */    MCD::OPC_FilterValue, 29, 9, 0, 0, // Skip to: 6301
/* 6292 */    MCD::OPC_CheckPredicate, 6, 219, 0, 0, // Skip to: 6516
/* 6297 */    MCD::OPC_Decode, 158, 13, 125, // Opcode: JALS_MM
/* 6301 */    MCD::OPC_FilterValue, 30, 9, 0, 0, // Skip to: 6315
/* 6306 */    MCD::OPC_CheckPredicate, 6, 205, 0, 0, // Skip to: 6516
/* 6311 */    MCD::OPC_Decode, 247, 4, 126, // Opcode: ADDIUPC_MM
/* 6315 */    MCD::OPC_FilterValue, 36, 9, 0, 0, // Skip to: 6329
/* 6320 */    MCD::OPC_CheckPredicate, 7, 191, 0, 0, // Skip to: 6516
/* 6325 */    MCD::OPC_Decode, 166, 19, 86, // Opcode: SLTi_MM
/* 6329 */    MCD::OPC_FilterValue, 37, 9, 0, 0, // Skip to: 6343
/* 6334 */    MCD::OPC_CheckPredicate, 6, 177, 0, 0, // Skip to: 6516
/* 6339 */    MCD::OPC_Decode, 170, 6, 127, // Opcode: BEQ_MM
/* 6343 */    MCD::OPC_FilterValue, 38, 10, 0, 0, // Skip to: 6358
/* 6348 */    MCD::OPC_CheckPredicate, 19, 163, 0, 0, // Skip to: 6516
/* 6353 */    MCD::OPC_Decode, 170, 20, 128, 1, // Opcode: SWC1_MM
/* 6358 */    MCD::OPC_FilterValue, 39, 10, 0, 0, // Skip to: 6373
/* 6363 */    MCD::OPC_CheckPredicate, 19, 148, 0, 0, // Skip to: 6516
/* 6368 */    MCD::OPC_Decode, 138, 14, 128, 1, // Opcode: LWC1_MM
/* 6373 */    MCD::OPC_FilterValue, 44, 9, 0, 0, // Skip to: 6387
/* 6378 */    MCD::OPC_CheckPredicate, 7, 133, 0, 0, // Skip to: 6516
/* 6383 */    MCD::OPC_Decode, 169, 19, 86, // Opcode: SLTiu_MM
/* 6387 */    MCD::OPC_FilterValue, 45, 9, 0, 0, // Skip to: 6401
/* 6392 */    MCD::OPC_CheckPredicate, 6, 119, 0, 0, // Skip to: 6516
/* 6397 */    MCD::OPC_Decode, 145, 7, 127, // Opcode: BNE_MM
/* 6401 */    MCD::OPC_FilterValue, 46, 10, 0, 0, // Skip to: 6416
/* 6406 */    MCD::OPC_CheckPredicate, 18, 105, 0, 0, // Skip to: 6516
/* 6411 */    MCD::OPC_Decode, 166, 18, 128, 1, // Opcode: SDC1_MM
/* 6416 */    MCD::OPC_FilterValue, 47, 10, 0, 0, // Skip to: 6431
/* 6421 */    MCD::OPC_CheckPredicate, 18, 90, 0, 0, // Skip to: 6516
/* 6426 */    MCD::OPC_Decode, 206, 13, 128, 1, // Opcode: LDC1_MM
/* 6431 */    MCD::OPC_FilterValue, 52, 9, 0, 0, // Skip to: 6445
/* 6436 */    MCD::OPC_CheckPredicate, 6, 75, 0, 0, // Skip to: 6516
/* 6441 */    MCD::OPC_Decode, 208, 5, 97, // Opcode: ANDi_MM
/* 6445 */    MCD::OPC_FilterValue, 53, 9, 0, 0, // Skip to: 6459
/* 6450 */    MCD::OPC_CheckPredicate, 6, 61, 0, 0, // Skip to: 6516
/* 6455 */    MCD::OPC_Decode, 180, 13, 125, // Opcode: J_MM
/* 6459 */    MCD::OPC_FilterValue, 60, 10, 0, 0, // Skip to: 6474
/* 6464 */    MCD::OPC_CheckPredicate, 6, 47, 0, 0, // Skip to: 6516
/* 6469 */    MCD::OPC_Decode, 160, 13, 129, 1, // Opcode: JALX_MM
/* 6474 */    MCD::OPC_FilterValue, 61, 9, 0, 0, // Skip to: 6488
/* 6479 */    MCD::OPC_CheckPredicate, 6, 32, 0, 0, // Skip to: 6516
/* 6484 */    MCD::OPC_Decode, 161, 13, 125, // Opcode: JAL_MM
/* 6488 */    MCD::OPC_FilterValue, 62, 9, 0, 0, // Skip to: 6502
/* 6493 */    MCD::OPC_CheckPredicate, 7, 18, 0, 0, // Skip to: 6516
/* 6498 */    MCD::OPC_Decode, 197, 20, 87, // Opcode: SW_MM
/* 6502 */    MCD::OPC_FilterValue, 63, 9, 0, 0, // Skip to: 6516
/* 6507 */    MCD::OPC_CheckPredicate, 7, 4, 0, 0, // Skip to: 6516
/* 6512 */    MCD::OPC_Decode, 172, 14, 87, // Opcode: LW_MM
/* 6516 */    MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMipsDSP32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 62, 10, 0, 0, // Skip to: 18
/* 8 */       MCD::OPC_CheckPredicate, 8, 20, 0, 0, // Skip to: 33
/* 13 */      MCD::OPC_Decode, 176, 20, 130, 1, // Opcode: SWDSP_MM
/* 18 */      MCD::OPC_FilterValue, 63, 10, 0, 0, // Skip to: 33
/* 23 */      MCD::OPC_CheckPredicate, 8, 5, 0, 0, // Skip to: 33
/* 28 */      MCD::OPC_Decode, 144, 14, 130, 1, // Opcode: LWDSP_MM
/* 33 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMipsFP6432[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 21, 39, 1, 0, // Skip to: 303
/* 8 */       MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 11 */      MCD::OPC_FilterValue, 59, 48, 0, 0, // Skip to: 64
/* 16 */      MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 19 */      MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 34
/* 24 */      MCD::OPC_CheckPredicate, 20, 48, 1, 0, // Skip to: 333
/* 29 */      MCD::OPC_Decode, 138, 16, 131, 1, // Opcode: MTC1_D64_MM
/* 34 */      MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 49
/* 39 */      MCD::OPC_CheckPredicate, 20, 33, 1, 0, // Skip to: 333
/* 44 */      MCD::OPC_Decode, 133, 15, 132, 1, // Opcode: MFHC1_D64_MM
/* 49 */      MCD::OPC_FilterValue, 7, 23, 1, 0, // Skip to: 333
/* 54 */      MCD::OPC_CheckPredicate, 20, 18, 1, 0, // Skip to: 333
/* 59 */      MCD::OPC_Decode, 149, 16, 133, 1, // Opcode: MTHC1_D64_MM
/* 64 */      MCD::OPC_FilterValue, 123, 16, 0, 0, // Skip to: 85
/* 69 */      MCD::OPC_CheckPredicate, 20, 3, 1, 0, // Skip to: 333
/* 74 */      MCD::OPC_CheckField, 11, 5, 4, 252, 0, 0, // Skip to: 333
/* 81 */      MCD::OPC_Decode, 147, 12, 111, // Opcode: FMOV_D64_MM
/* 85 */      MCD::OPC_FilterValue, 176, 2, 10, 0, 0, // Skip to: 101
/* 91 */      MCD::OPC_CheckPredicate, 20, 237, 0, 0, // Skip to: 333
/* 96 */      MCD::OPC_Decode, 186, 11, 134, 1, // Opcode: FADD_D64_MM
/* 101 */     MCD::OPC_FilterValue, 187, 2, 17, 0, 0, // Skip to: 124
/* 107 */     MCD::OPC_CheckPredicate, 20, 221, 0, 0, // Skip to: 333
/* 112 */     MCD::OPC_CheckField, 11, 5, 9, 214, 0, 0, // Skip to: 333
/* 119 */     MCD::OPC_Decode, 148, 9, 135, 1, // Opcode: CVT_W_D64_MM
/* 124 */     MCD::OPC_FilterValue, 240, 2, 10, 0, 0, // Skip to: 140
/* 130 */     MCD::OPC_CheckPredicate, 20, 198, 0, 0, // Skip to: 333
/* 135 */     MCD::OPC_Decode, 201, 12, 134, 1, // Opcode: FSUB_D64_MM
/* 140 */     MCD::OPC_FilterValue, 176, 3, 10, 0, 0, // Skip to: 156
/* 146 */     MCD::OPC_CheckPredicate, 20, 182, 0, 0, // Skip to: 333
/* 151 */     MCD::OPC_Decode, 158, 12, 134, 1, // Opcode: FMUL_D64_MM
/* 156 */     MCD::OPC_FilterValue, 240, 3, 10, 0, 0, // Skip to: 172
/* 162 */     MCD::OPC_CheckPredicate, 20, 166, 0, 0, // Skip to: 333
/* 167 */     MCD::OPC_Decode, 224, 11, 134, 1, // Opcode: FDIV_D64_MM
/* 172 */     MCD::OPC_FilterValue, 187, 4, 45, 0, 0, // Skip to: 223
/* 178 */     MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 181 */     MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 195
/* 186 */     MCD::OPC_CheckPredicate, 20, 142, 0, 0, // Skip to: 333
/* 191 */     MCD::OPC_Decode, 251, 17, 111, // Opcode: RSQRT_D64_MM
/* 195 */     MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 209
/* 200 */     MCD::OPC_CheckPredicate, 20, 128, 0, 0, // Skip to: 333
/* 205 */     MCD::OPC_Decode, 193, 12, 111, // Opcode: FSQRT_D64_MM
/* 209 */     MCD::OPC_FilterValue, 10, 119, 0, 0, // Skip to: 333
/* 214 */     MCD::OPC_CheckPredicate, 20, 114, 0, 0, // Skip to: 333
/* 219 */     MCD::OPC_Decode, 218, 17, 111, // Opcode: RECIP_D64_MM
/* 223 */     MCD::OPC_FilterValue, 251, 6, 104, 0, 0, // Skip to: 333
/* 229 */     MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 232 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 246
/* 237 */     MCD::OPC_CheckPredicate, 20, 91, 0, 0, // Skip to: 333
/* 242 */     MCD::OPC_Decode, 251, 8, 110, // Opcode: CVT_D64_S_MM
/* 246 */     MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 261
/* 251 */     MCD::OPC_CheckPredicate, 20, 77, 0, 0, // Skip to: 333
/* 256 */     MCD::OPC_Decode, 137, 9, 135, 1, // Opcode: CVT_S_D64_MM
/* 261 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 275
/* 266 */     MCD::OPC_CheckPredicate, 20, 62, 0, 0, // Skip to: 333
/* 271 */     MCD::OPC_Decode, 179, 11, 111, // Opcode: FABS_D64_MM
/* 275 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 289
/* 280 */     MCD::OPC_CheckPredicate, 20, 48, 0, 0, // Skip to: 333
/* 285 */     MCD::OPC_Decode, 166, 12, 111, // Opcode: FNEG_D64_MM
/* 289 */     MCD::OPC_FilterValue, 6, 39, 0, 0, // Skip to: 333
/* 294 */     MCD::OPC_CheckPredicate, 20, 34, 0, 0, // Skip to: 333
/* 299 */     MCD::OPC_Decode, 253, 8, 110, // Opcode: CVT_D64_W_MM
/* 303 */     MCD::OPC_FilterValue, 46, 10, 0, 0, // Skip to: 318
/* 308 */     MCD::OPC_CheckPredicate, 23, 20, 0, 0, // Skip to: 333
/* 313 */     MCD::OPC_Decode, 165, 18, 128, 1, // Opcode: SDC1_D64_MMR6
/* 318 */     MCD::OPC_FilterValue, 47, 10, 0, 0, // Skip to: 333
/* 323 */     MCD::OPC_CheckPredicate, 23, 5, 0, 0, // Skip to: 333
/* 328 */     MCD::OPC_Decode, 205, 13, 128, 1, // Opcode: LDC1_D64_MMR6
/* 333 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMipsR616[] = {
/* 0 */       MCD::OPC_ExtractField, 10, 6,  // Inst{15-10} ...
/* 3 */       MCD::OPC_FilterValue, 1, 33, 0, 0, // Skip to: 41
/* 8 */       MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 26
/* 16 */      MCD::OPC_CheckPredicate, 24, 173, 1, 0, // Skip to: 450
/* 21 */      MCD::OPC_Decode, 155, 5, 136, 1, // Opcode: ADDU16_MMR6
/* 26 */      MCD::OPC_FilterValue, 1, 163, 1, 0, // Skip to: 450
/* 31 */      MCD::OPC_CheckPredicate, 24, 158, 1, 0, // Skip to: 450
/* 36 */      MCD::OPC_Decode, 136, 20, 136, 1, // Opcode: SUBU16_MMR6
/* 41 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 55
/* 46 */      MCD::OPC_CheckPredicate, 24, 143, 1, 0, // Skip to: 450
/* 51 */      MCD::OPC_Decode, 193, 15, 22, // Opcode: MOVE16_MMR6
/* 55 */      MCD::OPC_FilterValue, 9, 31, 0, 0, // Skip to: 91
/* 60 */      MCD::OPC_ExtractField, 0, 1,  // Inst{0} ...
/* 63 */      MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 77
/* 68 */      MCD::OPC_CheckPredicate, 24, 121, 1, 0, // Skip to: 450
/* 73 */      MCD::OPC_Decode, 146, 19, 23, // Opcode: SLL16_MMR6
/* 77 */      MCD::OPC_FilterValue, 1, 112, 1, 0, // Skip to: 450
/* 82 */      MCD::OPC_CheckPredicate, 24, 107, 1, 0, // Skip to: 450
/* 87 */      MCD::OPC_Decode, 205, 19, 23, // Opcode: SRL16_MMR6
/* 91 */      MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 105
/* 96 */      MCD::OPC_CheckPredicate, 24, 93, 1, 0, // Skip to: 450
/* 101 */     MCD::OPC_Decode, 200, 5, 24, // Opcode: ANDI16_MMR6
/* 105 */     MCD::OPC_FilterValue, 17, 228, 0, 0, // Skip to: 338
/* 110 */     MCD::OPC_ExtractField, 2, 1,  // Inst{2} ...
/* 113 */     MCD::OPC_FilterValue, 0, 206, 0, 0, // Skip to: 324
/* 118 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 121 */     MCD::OPC_FilterValue, 0, 33, 0, 0, // Skip to: 159
/* 126 */     MCD::OPC_ExtractField, 3, 1,  // Inst{3} ...
/* 129 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 144
/* 134 */     MCD::OPC_CheckPredicate, 24, 55, 1, 0, // Skip to: 450
/* 139 */     MCD::OPC_Decode, 251, 16, 137, 1, // Opcode: NOT16_MMR6
/* 144 */     MCD::OPC_FilterValue, 1, 45, 1, 0, // Skip to: 450
/* 149 */     MCD::OPC_CheckPredicate, 24, 40, 1, 0, // Skip to: 450
/* 154 */     MCD::OPC_Decode, 179, 21, 138, 1, // Opcode: XOR16_MMR6
/* 159 */     MCD::OPC_FilterValue, 1, 33, 0, 0, // Skip to: 197
/* 164 */     MCD::OPC_ExtractField, 3, 1,  // Inst{3} ...
/* 167 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 182
/* 172 */     MCD::OPC_CheckPredicate, 24, 17, 1, 0, // Skip to: 450
/* 177 */     MCD::OPC_Decode, 197, 5, 138, 1, // Opcode: AND16_MMR6
/* 182 */     MCD::OPC_FilterValue, 1, 7, 1, 0, // Skip to: 450
/* 187 */     MCD::OPC_CheckPredicate, 24, 2, 1, 0, // Skip to: 450
/* 192 */     MCD::OPC_Decode, 128, 17, 138, 1, // Opcode: OR16_MMR6
/* 197 */     MCD::OPC_FilterValue, 2, 31, 0, 0, // Skip to: 233
/* 202 */     MCD::OPC_ExtractField, 3, 1,  // Inst{3} ...
/* 205 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 219
/* 210 */     MCD::OPC_CheckPredicate, 24, 235, 0, 0, // Skip to: 450
/* 215 */     MCD::OPC_Decode, 154, 14, 27, // Opcode: LWM16_MMR6
/* 219 */     MCD::OPC_FilterValue, 1, 226, 0, 0, // Skip to: 450
/* 224 */     MCD::OPC_CheckPredicate, 24, 221, 0, 0, // Skip to: 450
/* 229 */     MCD::OPC_Decode, 185, 20, 27, // Opcode: SWM16_MMR6
/* 233 */     MCD::OPC_FilterValue, 3, 212, 0, 0, // Skip to: 450
/* 238 */     MCD::OPC_ExtractField, 3, 2,  // Inst{4-3} ...
/* 241 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 256
/* 246 */     MCD::OPC_CheckPredicate, 24, 199, 0, 0, // Skip to: 450
/* 251 */     MCD::OPC_Decode, 173, 13, 139, 1, // Opcode: JRC16_MMR6
/* 256 */     MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 271
/* 261 */     MCD::OPC_CheckPredicate, 24, 184, 0, 0, // Skip to: 450
/* 266 */     MCD::OPC_Decode, 150, 13, 139, 1, // Opcode: JALRC16_MMR6
/* 271 */     MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 286
/* 276 */     MCD::OPC_CheckPredicate, 24, 169, 0, 0, // Skip to: 450
/* 281 */     MCD::OPC_Decode, 174, 13, 140, 1, // Opcode: JRCADDIUSP_MMR6
/* 286 */     MCD::OPC_FilterValue, 3, 159, 0, 0, // Skip to: 450
/* 291 */     MCD::OPC_ExtractField, 5, 1,  // Inst{5} ...
/* 294 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 309
/* 299 */     MCD::OPC_CheckPredicate, 24, 146, 0, 0, // Skip to: 450
/* 304 */     MCD::OPC_Decode, 160, 7, 141, 1, // Opcode: BREAK16_MMR6
/* 309 */     MCD::OPC_FilterValue, 1, 136, 0, 0, // Skip to: 450
/* 314 */     MCD::OPC_CheckPredicate, 24, 131, 0, 0, // Skip to: 450
/* 319 */     MCD::OPC_Decode, 159, 18, 141, 1, // Opcode: SDBBP16_MMR6
/* 324 */     MCD::OPC_FilterValue, 1, 121, 0, 0, // Skip to: 450
/* 329 */     MCD::OPC_CheckPredicate, 24, 116, 0, 0, // Skip to: 450
/* 334 */     MCD::OPC_Decode, 195, 15, 37, // Opcode: MOVEP_MMR6
/* 338 */     MCD::OPC_FilterValue, 34, 9, 0, 0, // Skip to: 352
/* 343 */     MCD::OPC_CheckPredicate, 24, 102, 0, 0, // Skip to: 450
/* 348 */     MCD::OPC_Decode, 140, 18, 21, // Opcode: SB16_MMR6
/* 352 */     MCD::OPC_FilterValue, 35, 9, 0, 0, // Skip to: 366
/* 357 */     MCD::OPC_CheckPredicate, 24, 88, 0, 0, // Skip to: 450
/* 362 */     MCD::OPC_Decode, 166, 6, 38, // Opcode: BEQZC16_MMR6
/* 366 */     MCD::OPC_FilterValue, 42, 9, 0, 0, // Skip to: 380
/* 371 */     MCD::OPC_CheckPredicate, 24, 74, 0, 0, // Skip to: 450
/* 376 */     MCD::OPC_Decode, 205, 18, 21, // Opcode: SH16_MMR6
/* 380 */     MCD::OPC_FilterValue, 43, 9, 0, 0, // Skip to: 394
/* 385 */     MCD::OPC_CheckPredicate, 24, 60, 0, 0, // Skip to: 450
/* 390 */     MCD::OPC_Decode, 141, 7, 38, // Opcode: BNEZC16_MMR6
/* 394 */     MCD::OPC_FilterValue, 50, 9, 0, 0, // Skip to: 408
/* 399 */     MCD::OPC_CheckPredicate, 24, 46, 0, 0, // Skip to: 450
/* 404 */     MCD::OPC_Decode, 194, 20, 31, // Opcode: SWSP_MMR6
/* 408 */     MCD::OPC_FilterValue, 51, 9, 0, 0, // Skip to: 422
/* 413 */     MCD::OPC_CheckPredicate, 24, 32, 0, 0, // Skip to: 450
/* 418 */     MCD::OPC_Decode, 132, 6, 39, // Opcode: BC16_MMR6
/* 422 */     MCD::OPC_FilterValue, 58, 9, 0, 0, // Skip to: 436
/* 427 */     MCD::OPC_CheckPredicate, 24, 18, 0, 0, // Skip to: 450
/* 432 */     MCD::OPC_Decode, 167, 20, 21, // Opcode: SW16_MMR6
/* 436 */     MCD::OPC_FilterValue, 59, 9, 0, 0, // Skip to: 450
/* 441 */     MCD::OPC_CheckPredicate, 24, 4, 0, 0, // Skip to: 450
/* 446 */     MCD::OPC_Decode, 241, 13, 40, // Opcode: LI16_MMR6
/* 450 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMipsR632[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 98, 4, 0, // Skip to: 1130
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 112, 0, 0, // Skip to: 128
/* 16 */      MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 19 */      MCD::OPC_FilterValue, 0, 54, 0, 0, // Skip to: 78
/* 24 */      MCD::OPC_ExtractField, 11, 15,  // Inst{25-11} ...
/* 27 */      MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 41
/* 32 */      MCD::OPC_CheckPredicate, 24, 32, 0, 0, // Skip to: 69
/* 37 */      MCD::OPC_Decode, 227, 19, 0, // Opcode: SSNOP_MMR6
/* 41 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 55
/* 46 */      MCD::OPC_CheckPredicate, 24, 18, 0, 0, // Skip to: 69
/* 51 */      MCD::OPC_Decode, 134, 11, 0, // Opcode: EHB_MMR6
/* 55 */      MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 69
/* 60 */      MCD::OPC_CheckPredicate, 24, 4, 0, 0, // Skip to: 69
/* 65 */      MCD::OPC_Decode, 143, 17, 0, // Opcode: PAUSE_MMR6
/* 69 */      MCD::OPC_CheckPredicate, 24, 78, 12, 0, // Skip to: 3224
/* 74 */      MCD::OPC_Decode, 159, 19, 41, // Opcode: SLL_MMR6
/* 78 */      MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 92
/* 83 */      MCD::OPC_CheckPredicate, 24, 64, 12, 0, // Skip to: 3224
/* 88 */      MCD::OPC_Decode, 187, 18, 44, // Opcode: SELEQZ_MMR6
/* 92 */      MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 106
/* 97 */      MCD::OPC_CheckPredicate, 24, 50, 12, 0, // Skip to: 3224
/* 102 */     MCD::OPC_Decode, 194, 18, 44, // Opcode: SELNEZ_MMR6
/* 106 */     MCD::OPC_FilterValue, 7, 41, 12, 0, // Skip to: 3224
/* 111 */     MCD::OPC_CheckPredicate, 24, 36, 12, 0, // Skip to: 3224
/* 116 */     MCD::OPC_CheckField, 14, 2, 0, 29, 12, 0, // Skip to: 3224
/* 123 */     MCD::OPC_Decode, 213, 17, 142, 1, // Opcode: RDHWR_MMR6
/* 128 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 142
/* 133 */     MCD::OPC_CheckPredicate, 24, 14, 12, 0, // Skip to: 3224
/* 138 */     MCD::OPC_Decode, 162, 7, 45, // Opcode: BREAK_MMR6
/* 142 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 156
/* 147 */     MCD::OPC_CheckPredicate, 24, 0, 12, 0, // Skip to: 3224
/* 152 */     MCD::OPC_Decode, 144, 13, 46, // Opcode: INS_MMR6
/* 156 */     MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 178
/* 161 */     MCD::OPC_CheckPredicate, 24, 242, 11, 0, // Skip to: 3224
/* 166 */     MCD::OPC_CheckField, 6, 3, 0, 235, 11, 0, // Skip to: 3224
/* 173 */     MCD::OPC_Decode, 253, 13, 143, 1, // Opcode: LSA_MMR6
/* 178 */     MCD::OPC_FilterValue, 16, 136, 0, 0, // Skip to: 319
/* 183 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 186 */     MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 207
/* 191 */     MCD::OPC_CheckPredicate, 24, 212, 11, 0, // Skip to: 3224
/* 196 */     MCD::OPC_CheckField, 16, 5, 0, 205, 11, 0, // Skip to: 3224
/* 203 */     MCD::OPC_Decode, 136, 8, 14, // Opcode: CLZ_MMR6
/* 207 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 221
/* 212 */     MCD::OPC_CheckPredicate, 24, 191, 11, 0, // Skip to: 3224
/* 217 */     MCD::OPC_Decode, 184, 5, 44, // Opcode: ADD_MMR6
/* 221 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 235
/* 226 */     MCD::OPC_CheckPredicate, 24, 177, 11, 0, // Skip to: 3224
/* 231 */     MCD::OPC_Decode, 160, 5, 44, // Opcode: ADDU_MMR6
/* 235 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 249
/* 240 */     MCD::OPC_CheckPredicate, 24, 163, 11, 0, // Skip to: 3224
/* 245 */     MCD::OPC_Decode, 159, 20, 44, // Opcode: SUB_MMR6
/* 249 */     MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 263
/* 254 */     MCD::OPC_CheckPredicate, 24, 149, 11, 0, // Skip to: 3224
/* 259 */     MCD::OPC_Decode, 141, 20, 44, // Opcode: SUBU_MMR6
/* 263 */     MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 277
/* 268 */     MCD::OPC_CheckPredicate, 24, 135, 11, 0, // Skip to: 3224
/* 273 */     MCD::OPC_Decode, 204, 5, 44, // Opcode: AND_MMR6
/* 277 */     MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 291
/* 282 */     MCD::OPC_CheckPredicate, 24, 121, 11, 0, // Skip to: 3224
/* 287 */     MCD::OPC_Decode, 133, 17, 44, // Opcode: OR_MMR6
/* 291 */     MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 305
/* 296 */     MCD::OPC_CheckPredicate, 24, 107, 11, 0, // Skip to: 3224
/* 301 */     MCD::OPC_Decode, 248, 16, 44, // Opcode: NOR_MMR6
/* 305 */     MCD::OPC_FilterValue, 12, 98, 11, 0, // Skip to: 3224
/* 310 */     MCD::OPC_CheckPredicate, 24, 93, 11, 0, // Skip to: 3224
/* 315 */     MCD::OPC_Decode, 184, 21, 44, // Opcode: XOR_MMR6
/* 319 */     MCD::OPC_FilterValue, 24, 115, 0, 0, // Skip to: 439
/* 324 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 327 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 341
/* 332 */     MCD::OPC_CheckPredicate, 24, 71, 11, 0, // Skip to: 3224
/* 337 */     MCD::OPC_Decode, 214, 16, 44, // Opcode: MUL_MMR6
/* 341 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 355
/* 346 */     MCD::OPC_CheckPredicate, 24, 57, 11, 0, // Skip to: 3224
/* 351 */     MCD::OPC_Decode, 175, 16, 44, // Opcode: MUH_MMR6
/* 355 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 369
/* 360 */     MCD::OPC_CheckPredicate, 24, 43, 11, 0, // Skip to: 3224
/* 365 */     MCD::OPC_Decode, 208, 16, 44, // Opcode: MULU_MMR6
/* 369 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 383
/* 374 */     MCD::OPC_CheckPredicate, 24, 29, 11, 0, // Skip to: 3224
/* 379 */     MCD::OPC_Decode, 174, 16, 44, // Opcode: MUHU_MMR6
/* 383 */     MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 397
/* 388 */     MCD::OPC_CheckPredicate, 24, 15, 11, 0, // Skip to: 3224
/* 393 */     MCD::OPC_Decode, 152, 10, 44, // Opcode: DIV_MMR6
/* 397 */     MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 411
/* 402 */     MCD::OPC_CheckPredicate, 24, 1, 11, 0, // Skip to: 3224
/* 407 */     MCD::OPC_Decode, 183, 15, 44, // Opcode: MOD_MMR6
/* 411 */     MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 425
/* 416 */     MCD::OPC_CheckPredicate, 24, 243, 10, 0, // Skip to: 3224
/* 421 */     MCD::OPC_Decode, 151, 10, 44, // Opcode: DIVU_MMR6
/* 425 */     MCD::OPC_FilterValue, 7, 234, 10, 0, // Skip to: 3224
/* 430 */     MCD::OPC_CheckPredicate, 24, 229, 10, 0, // Skip to: 3224
/* 435 */     MCD::OPC_Decode, 182, 15, 44, // Opcode: MODU_MMR6
/* 439 */     MCD::OPC_FilterValue, 31, 17, 0, 0, // Skip to: 461
/* 444 */     MCD::OPC_CheckPredicate, 24, 215, 10, 0, // Skip to: 3224
/* 449 */     MCD::OPC_CheckField, 6, 3, 0, 208, 10, 0, // Skip to: 3224
/* 456 */     MCD::OPC_Decode, 192, 5, 144, 1, // Opcode: ALIGN_MMR6
/* 461 */     MCD::OPC_FilterValue, 44, 9, 0, 0, // Skip to: 475
/* 466 */     MCD::OPC_CheckPredicate, 24, 193, 10, 0, // Skip to: 3224
/* 471 */     MCD::OPC_Decode, 175, 11, 55, // Opcode: EXT_MMR6
/* 475 */     MCD::OPC_FilterValue, 52, 45, 0, 0, // Skip to: 525
/* 480 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 483 */     MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 504
/* 488 */     MCD::OPC_CheckPredicate, 24, 171, 10, 0, // Skip to: 3224
/* 493 */     MCD::OPC_CheckField, 14, 2, 0, 164, 10, 0, // Skip to: 3224
/* 500 */     MCD::OPC_Decode, 129, 15, 57, // Opcode: MFHC0_MMR6
/* 504 */     MCD::OPC_FilterValue, 11, 155, 10, 0, // Skip to: 3224
/* 509 */     MCD::OPC_CheckPredicate, 24, 150, 10, 0, // Skip to: 3224
/* 514 */     MCD::OPC_CheckField, 14, 2, 0, 143, 10, 0, // Skip to: 3224
/* 521 */     MCD::OPC_Decode, 145, 16, 58, // Opcode: MTHC0_MMR6
/* 525 */     MCD::OPC_FilterValue, 60, 66, 2, 0, // Skip to: 1108
/* 530 */     MCD::OPC_ExtractField, 14, 2,  // Inst{15-14} ...
/* 533 */     MCD::OPC_FilterValue, 0, 138, 0, 0, // Skip to: 676
/* 538 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 541 */     MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 555
/* 546 */     MCD::OPC_CheckPredicate, 24, 113, 10, 0, // Skip to: 3224
/* 551 */     MCD::OPC_Decode, 248, 14, 57, // Opcode: MFC0_MMR6
/* 555 */     MCD::OPC_FilterValue, 5, 45, 0, 0, // Skip to: 605
/* 560 */     MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 563 */     MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 584
/* 568 */     MCD::OPC_CheckPredicate, 24, 91, 10, 0, // Skip to: 3224
/* 573 */     MCD::OPC_CheckField, 21, 5, 0, 84, 10, 0, // Skip to: 3224
/* 580 */     MCD::OPC_Decode, 129, 11, 81, // Opcode: DVP_MMR6
/* 584 */     MCD::OPC_FilterValue, 7, 75, 10, 0, // Skip to: 3224
/* 589 */     MCD::OPC_CheckPredicate, 24, 70, 10, 0, // Skip to: 3224
/* 594 */     MCD::OPC_CheckField, 21, 5, 0, 63, 10, 0, // Skip to: 3224
/* 601 */     MCD::OPC_Decode, 146, 11, 81, // Opcode: EVP_MMR6
/* 605 */     MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 619
/* 610 */     MCD::OPC_CheckPredicate, 24, 49, 10, 0, // Skip to: 3224
/* 615 */     MCD::OPC_Decode, 135, 16, 58, // Opcode: MTC0_MMR6
/* 619 */     MCD::OPC_FilterValue, 12, 16, 0, 0, // Skip to: 640
/* 624 */     MCD::OPC_CheckPredicate, 24, 35, 10, 0, // Skip to: 3224
/* 629 */     MCD::OPC_CheckField, 11, 3, 1, 28, 10, 0, // Skip to: 3224
/* 636 */     MCD::OPC_Decode, 218, 6, 75, // Opcode: BITSWAP_MMR6
/* 640 */     MCD::OPC_FilterValue, 28, 19, 10, 0, // Skip to: 3224
/* 645 */     MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 648 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 662
/* 653 */     MCD::OPC_CheckPredicate, 24, 6, 10, 0, // Skip to: 3224
/* 658 */     MCD::OPC_Decode, 152, 13, 69, // Opcode: JALRC_MMR6
/* 662 */     MCD::OPC_FilterValue, 3, 253, 9, 0, // Skip to: 3224
/* 667 */     MCD::OPC_CheckPredicate, 24, 248, 9, 0, // Skip to: 3224
/* 672 */     MCD::OPC_Decode, 151, 13, 69, // Opcode: JALRC_HB_MMR6
/* 676 */     MCD::OPC_FilterValue, 1, 10, 1, 0, // Skip to: 947
/* 681 */     MCD::OPC_ExtractField, 11, 3,  // Inst{13-11} ...
/* 684 */     MCD::OPC_FilterValue, 0, 45, 0, 0, // Skip to: 734
/* 689 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 692 */     MCD::OPC_FilterValue, 13, 16, 0, 0, // Skip to: 713
/* 697 */     MCD::OPC_CheckPredicate, 24, 218, 9, 0, // Skip to: 3224
/* 702 */     MCD::OPC_CheckField, 16, 10, 0, 211, 9, 0, // Skip to: 3224
/* 709 */     MCD::OPC_Decode, 255, 20, 0, // Opcode: TLBINV_MMR6
/* 713 */     MCD::OPC_FilterValue, 29, 202, 9, 0, // Skip to: 3224
/* 718 */     MCD::OPC_CheckPredicate, 24, 197, 9, 0, // Skip to: 3224
/* 723 */     MCD::OPC_CheckField, 21, 5, 0, 190, 9, 0, // Skip to: 3224
/* 730 */     MCD::OPC_Decode, 162, 10, 81, // Opcode: DI_MMR6
/* 734 */     MCD::OPC_FilterValue, 1, 31, 0, 0, // Skip to: 770
/* 739 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 742 */     MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 756
/* 747 */     MCD::OPC_CheckPredicate, 24, 168, 9, 0, // Skip to: 3224
/* 752 */     MCD::OPC_Decode, 244, 7, 69, // Opcode: CLO_MMR6
/* 756 */     MCD::OPC_FilterValue, 20, 159, 9, 0, // Skip to: 3224
/* 761 */     MCD::OPC_CheckPredicate, 24, 154, 9, 0, // Skip to: 3224
/* 766 */     MCD::OPC_Decode, 254, 14, 79, // Opcode: MFC2_MMR6
/* 770 */     MCD::OPC_FilterValue, 2, 45, 0, 0, // Skip to: 820
/* 775 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 778 */     MCD::OPC_FilterValue, 13, 16, 0, 0, // Skip to: 799
/* 783 */     MCD::OPC_CheckPredicate, 24, 132, 9, 0, // Skip to: 3224
/* 788 */     MCD::OPC_CheckField, 16, 10, 0, 125, 9, 0, // Skip to: 3224
/* 795 */     MCD::OPC_Decode, 254, 20, 0, // Opcode: TLBINVF_MMR6
/* 799 */     MCD::OPC_FilterValue, 29, 116, 9, 0, // Skip to: 3224
/* 804 */     MCD::OPC_CheckPredicate, 24, 111, 9, 0, // Skip to: 3224
/* 809 */     MCD::OPC_CheckField, 21, 5, 0, 104, 9, 0, // Skip to: 3224
/* 816 */     MCD::OPC_Decode, 137, 11, 81, // Opcode: EI_MMR6
/* 820 */     MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 841
/* 825 */     MCD::OPC_CheckPredicate, 24, 90, 9, 0, // Skip to: 3224
/* 830 */     MCD::OPC_CheckField, 6, 5, 20, 83, 9, 0, // Skip to: 3224
/* 837 */     MCD::OPC_Decode, 142, 16, 80, // Opcode: MTC2_MMR6
/* 841 */     MCD::OPC_FilterValue, 4, 23, 0, 0, // Skip to: 869
/* 846 */     MCD::OPC_CheckPredicate, 25, 69, 9, 0, // Skip to: 3224
/* 851 */     MCD::OPC_CheckField, 21, 5, 0, 62, 9, 0, // Skip to: 3224
/* 858 */     MCD::OPC_CheckField, 6, 3, 5, 55, 9, 0, // Skip to: 3224
/* 865 */     MCD::OPC_Decode, 227, 12, 81, // Opcode: GINVI_MMR6
/* 869 */     MCD::OPC_FilterValue, 5, 23, 0, 0, // Skip to: 897
/* 874 */     MCD::OPC_CheckPredicate, 24, 41, 9, 0, // Skip to: 3224
/* 879 */     MCD::OPC_CheckField, 21, 5, 0, 34, 9, 0, // Skip to: 3224
/* 886 */     MCD::OPC_CheckField, 6, 5, 13, 27, 9, 0, // Skip to: 3224
/* 893 */     MCD::OPC_Decode, 204, 20, 76, // Opcode: SYNC_MMR6
/* 897 */     MCD::OPC_FilterValue, 6, 24, 0, 0, // Skip to: 926
/* 902 */     MCD::OPC_CheckPredicate, 25, 13, 9, 0, // Skip to: 3224
/* 907 */     MCD::OPC_CheckField, 21, 5, 0, 6, 9, 0, // Skip to: 3224
/* 914 */     MCD::OPC_CheckField, 6, 3, 5, 255, 8, 0, // Skip to: 3224
/* 921 */     MCD::OPC_Decode, 229, 12, 145, 1, // Opcode: GINVT_MMR6
/* 926 */     MCD::OPC_FilterValue, 7, 245, 8, 0, // Skip to: 3224
/* 931 */     MCD::OPC_CheckPredicate, 24, 240, 8, 0, // Skip to: 3224
/* 936 */     MCD::OPC_CheckField, 6, 5, 12, 233, 8, 0, // Skip to: 3224
/* 943 */     MCD::OPC_Decode, 176, 21, 69, // Opcode: WSBH_MMR6
/* 947 */     MCD::OPC_FilterValue, 2, 45, 0, 0, // Skip to: 997
/* 952 */     MCD::OPC_ExtractField, 6, 8,  // Inst{13-6} ...
/* 955 */     MCD::OPC_FilterValue, 52, 9, 0, 0, // Skip to: 969
/* 960 */     MCD::OPC_CheckPredicate, 24, 211, 8, 0, // Skip to: 3224
/* 965 */     MCD::OPC_Decode, 134, 15, 79, // Opcode: MFHC2_MMR6
/* 969 */     MCD::OPC_FilterValue, 77, 9, 0, 0, // Skip to: 983
/* 974 */     MCD::OPC_CheckPredicate, 24, 197, 8, 0, // Skip to: 3224
/* 979 */     MCD::OPC_Decode, 170, 21, 77, // Opcode: WAIT_MMR6
/* 983 */     MCD::OPC_FilterValue, 116, 188, 8, 0, // Skip to: 3224
/* 988 */     MCD::OPC_CheckPredicate, 24, 183, 8, 0, // Skip to: 3224
/* 993 */     MCD::OPC_Decode, 150, 16, 80, // Opcode: MTHC2_MMR6
/* 997 */     MCD::OPC_FilterValue, 3, 174, 8, 0, // Skip to: 3224
/* 1002 */    MCD::OPC_ExtractField, 6, 8,  // Inst{13-6} ...
/* 1005 */    MCD::OPC_FilterValue, 109, 9, 0, 0, // Skip to: 1019
/* 1010 */    MCD::OPC_CheckPredicate, 24, 161, 8, 0, // Skip to: 3224
/* 1015 */    MCD::OPC_Decode, 161, 18, 77, // Opcode: SDBBP_MMR6
/* 1019 */    MCD::OPC_FilterValue, 133, 1, 9, 0, 0, // Skip to: 1034
/* 1025 */    MCD::OPC_CheckPredicate, 24, 146, 8, 0, // Skip to: 3224
/* 1030 */    MCD::OPC_Decode, 214, 17, 69, // Opcode: RDPGPR_MMR6
/* 1034 */    MCD::OPC_FilterValue, 141, 1, 16, 0, 0, // Skip to: 1056
/* 1040 */    MCD::OPC_CheckPredicate, 24, 131, 8, 0, // Skip to: 3224
/* 1045 */    MCD::OPC_CheckField, 16, 10, 0, 124, 8, 0, // Skip to: 3224
/* 1052 */    MCD::OPC_Decode, 140, 10, 0, // Opcode: DERET_MMR6
/* 1056 */    MCD::OPC_FilterValue, 197, 1, 9, 0, 0, // Skip to: 1071
/* 1062 */    MCD::OPC_CheckPredicate, 24, 109, 8, 0, // Skip to: 3224
/* 1067 */    MCD::OPC_Decode, 173, 21, 69, // Opcode: WRPGPR_MMR6
/* 1071 */    MCD::OPC_FilterValue, 205, 1, 99, 8, 0, // Skip to: 3224
/* 1077 */    MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 1080 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1094
/* 1085 */    MCD::OPC_CheckPredicate, 24, 86, 8, 0, // Skip to: 3224
/* 1090 */    MCD::OPC_Decode, 143, 11, 0, // Opcode: ERET_MMR6
/* 1094 */    MCD::OPC_FilterValue, 1, 77, 8, 0, // Skip to: 3224
/* 1099 */    MCD::OPC_CheckPredicate, 24, 72, 8, 0, // Skip to: 3224
/* 1104 */    MCD::OPC_Decode, 141, 11, 0, // Opcode: ERETNC_MMR6
/* 1108 */    MCD::OPC_FilterValue, 63, 63, 8, 0, // Skip to: 3224
/* 1113 */    MCD::OPC_CheckPredicate, 24, 58, 8, 0, // Skip to: 3224
/* 1118 */    MCD::OPC_CheckField, 22, 4, 0, 51, 8, 0, // Skip to: 3224
/* 1125 */    MCD::OPC_Decode, 135, 19, 146, 1, // Opcode: SIGRIE_MMR6
/* 1130 */    MCD::OPC_FilterValue, 4, 26, 0, 0, // Skip to: 1161
/* 1135 */    MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 1152
/* 1140 */    MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 1152
/* 1147 */    MCD::OPC_Decode, 255, 13, 147, 1, // Opcode: LUI_MMR6
/* 1152 */    MCD::OPC_CheckPredicate, 24, 19, 8, 0, // Skip to: 3224
/* 1157 */    MCD::OPC_Decode, 222, 5, 97, // Opcode: AUI_MMR6
/* 1161 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1176
/* 1166 */    MCD::OPC_CheckPredicate, 24, 5, 8, 0, // Skip to: 3224
/* 1171 */    MCD::OPC_Decode, 194, 13, 148, 1, // Opcode: LBU_MMR6
/* 1176 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 1190
/* 1181 */    MCD::OPC_CheckPredicate, 24, 246, 7, 0, // Skip to: 3224
/* 1186 */    MCD::OPC_Decode, 145, 18, 87, // Opcode: SB_MMR6
/* 1190 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 1205
/* 1195 */    MCD::OPC_CheckPredicate, 24, 232, 7, 0, // Skip to: 3224
/* 1200 */    MCD::OPC_Decode, 196, 13, 148, 1, // Opcode: LB_MMR6
/* 1205 */    MCD::OPC_FilterValue, 8, 105, 0, 0, // Skip to: 1315
/* 1210 */    MCD::OPC_ExtractField, 12, 4,  // Inst{15-12} ...
/* 1213 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 1235
/* 1218 */    MCD::OPC_CheckPredicate, 24, 209, 7, 0, // Skip to: 3224
/* 1223 */    MCD::OPC_CheckField, 11, 1, 0, 202, 7, 0, // Skip to: 3224
/* 1230 */    MCD::OPC_Decode, 140, 14, 149, 1, // Opcode: LWC2_MMR6
/* 1235 */    MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 1257
/* 1240 */    MCD::OPC_CheckPredicate, 24, 187, 7, 0, // Skip to: 3224
/* 1245 */    MCD::OPC_CheckField, 11, 1, 0, 180, 7, 0, // Skip to: 3224
/* 1252 */    MCD::OPC_Decode, 208, 13, 149, 1, // Opcode: LDC2_MMR6
/* 1257 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 1271
/* 1262 */    MCD::OPC_CheckPredicate, 24, 165, 7, 0, // Skip to: 3224
/* 1267 */    MCD::OPC_Decode, 193, 7, 89, // Opcode: CACHE_MMR6
/* 1271 */    MCD::OPC_FilterValue, 8, 17, 0, 0, // Skip to: 1293
/* 1276 */    MCD::OPC_CheckPredicate, 24, 151, 7, 0, // Skip to: 3224
/* 1281 */    MCD::OPC_CheckField, 11, 1, 0, 144, 7, 0, // Skip to: 3224
/* 1288 */    MCD::OPC_Decode, 172, 20, 149, 1, // Opcode: SWC2_MMR6
/* 1293 */    MCD::OPC_FilterValue, 10, 134, 7, 0, // Skip to: 3224
/* 1298 */    MCD::OPC_CheckPredicate, 24, 129, 7, 0, // Skip to: 3224
/* 1303 */    MCD::OPC_CheckField, 11, 1, 0, 122, 7, 0, // Skip to: 3224
/* 1310 */    MCD::OPC_Decode, 168, 18, 149, 1, // Opcode: SDC2_MMR6
/* 1315 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 1329
/* 1320 */    MCD::OPC_CheckPredicate, 24, 107, 7, 0, // Skip to: 3224
/* 1325 */    MCD::OPC_Decode, 253, 4, 86, // Opcode: ADDIU_MMR6
/* 1329 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 1343
/* 1334 */    MCD::OPC_CheckPredicate, 24, 93, 7, 0, // Skip to: 3224
/* 1339 */    MCD::OPC_Decode, 133, 19, 87, // Opcode: SH_MMR6
/* 1343 */    MCD::OPC_FilterValue, 16, 78, 0, 0, // Skip to: 1426
/* 1348 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 1351 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 1366
/* 1356 */    MCD::OPC_CheckPredicate, 26, 71, 7, 0, // Skip to: 3224
/* 1361 */    MCD::OPC_Decode, 134, 6, 150, 1, // Opcode: BC1EQZC_MMR6
/* 1366 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1381
/* 1371 */    MCD::OPC_CheckPredicate, 26, 56, 7, 0, // Skip to: 3224
/* 1376 */    MCD::OPC_Decode, 139, 6, 150, 1, // Opcode: BC1NEZC_MMR6
/* 1381 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1396
/* 1386 */    MCD::OPC_CheckPredicate, 24, 41, 7, 0, // Skip to: 3224
/* 1391 */    MCD::OPC_Decode, 144, 6, 151, 1, // Opcode: BC2EQZC_MMR6
/* 1396 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1411
/* 1401 */    MCD::OPC_CheckPredicate, 24, 26, 7, 0, // Skip to: 3224
/* 1406 */    MCD::OPC_Decode, 146, 6, 151, 1, // Opcode: BC2NEZC_MMR6
/* 1411 */    MCD::OPC_FilterValue, 12, 16, 7, 0, // Skip to: 3224
/* 1416 */    MCD::OPC_CheckPredicate, 24, 11, 7, 0, // Skip to: 3224
/* 1421 */    MCD::OPC_Decode, 202, 20, 152, 1, // Opcode: SYNCI_MMR6
/* 1426 */    MCD::OPC_FilterValue, 20, 9, 0, 0, // Skip to: 1440
/* 1431 */    MCD::OPC_CheckPredicate, 24, 252, 6, 0, // Skip to: 3224
/* 1436 */    MCD::OPC_Decode, 131, 17, 97, // Opcode: ORI_MMR6
/* 1440 */    MCD::OPC_FilterValue, 21, 87, 5, 0, // Skip to: 2812
/* 1445 */    MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 1448 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1463
/* 1453 */    MCD::OPC_CheckPredicate, 26, 230, 6, 0, // Skip to: 3224
/* 1458 */    MCD::OPC_Decode, 172, 15, 153, 1, // Opcode: MIN_S_MMR6
/* 1463 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1478
/* 1468 */    MCD::OPC_CheckPredicate, 26, 215, 6, 0, // Skip to: 3224
/* 1473 */    MCD::OPC_Decode, 157, 8, 154, 1, // Opcode: CMP_AF_S_MMR6
/* 1478 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1493
/* 1483 */    MCD::OPC_CheckPredicate, 26, 200, 6, 0, // Skip to: 3224
/* 1488 */    MCD::OPC_Decode, 241, 14, 153, 1, // Opcode: MAX_S_MMR6
/* 1493 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 1508
/* 1498 */    MCD::OPC_CheckPredicate, 26, 185, 6, 0, // Skip to: 3224
/* 1503 */    MCD::OPC_Decode, 156, 8, 155, 1, // Opcode: CMP_AF_D_MMR6
/* 1508 */    MCD::OPC_FilterValue, 32, 17, 0, 0, // Skip to: 1530
/* 1513 */    MCD::OPC_CheckPredicate, 24, 170, 6, 0, // Skip to: 3224
/* 1518 */    MCD::OPC_CheckField, 11, 5, 0, 163, 6, 0, // Skip to: 3224
/* 1525 */    MCD::OPC_Decode, 232, 17, 156, 1, // Opcode: RINT_S_MMR6
/* 1530 */    MCD::OPC_FilterValue, 35, 10, 0, 0, // Skip to: 1545
/* 1535 */    MCD::OPC_CheckPredicate, 26, 148, 6, 0, // Skip to: 3224
/* 1540 */    MCD::OPC_Decode, 153, 15, 153, 1, // Opcode: MINA_S_MMR6
/* 1545 */    MCD::OPC_FilterValue, 43, 10, 0, 0, // Skip to: 1560
/* 1550 */    MCD::OPC_CheckPredicate, 26, 133, 6, 0, // Skip to: 3224
/* 1555 */    MCD::OPC_Decode, 222, 14, 153, 1, // Opcode: MAXA_S_MMR6
/* 1560 */    MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 1575
/* 1565 */    MCD::OPC_CheckPredicate, 26, 118, 6, 0, // Skip to: 3224
/* 1570 */    MCD::OPC_Decode, 189, 11, 157, 1, // Opcode: FADD_S_MMR6
/* 1575 */    MCD::OPC_FilterValue, 56, 10, 0, 0, // Skip to: 1590
/* 1580 */    MCD::OPC_CheckPredicate, 24, 103, 6, 0, // Skip to: 3224
/* 1585 */    MCD::OPC_Decode, 189, 18, 153, 1, // Opcode: SELEQZ_S_MMR6
/* 1590 */    MCD::OPC_FilterValue, 59, 31, 0, 0, // Skip to: 1626
/* 1595 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 1598 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 1612
/* 1603 */    MCD::OPC_CheckPredicate, 26, 80, 6, 0, // Skip to: 3224
/* 1608 */    MCD::OPC_Decode, 252, 14, 108, // Opcode: MFC1_MMR6
/* 1612 */    MCD::OPC_FilterValue, 5, 71, 6, 0, // Skip to: 3224
/* 1617 */    MCD::OPC_CheckPredicate, 26, 66, 6, 0, // Skip to: 3224
/* 1622 */    MCD::OPC_Decode, 140, 16, 114, // Opcode: MTC1_MMR6
/* 1626 */    MCD::OPC_FilterValue, 69, 10, 0, 0, // Skip to: 1641
/* 1631 */    MCD::OPC_CheckPredicate, 26, 52, 6, 0, // Skip to: 3224
/* 1636 */    MCD::OPC_Decode, 225, 8, 154, 1, // Opcode: CMP_UN_S_MMR6
/* 1641 */    MCD::OPC_FilterValue, 85, 10, 0, 0, // Skip to: 1656
/* 1646 */    MCD::OPC_CheckPredicate, 26, 37, 6, 0, // Skip to: 3224
/* 1651 */    MCD::OPC_Decode, 223, 8, 155, 1, // Opcode: CMP_UN_D_MMR6
/* 1656 */    MCD::OPC_FilterValue, 96, 17, 0, 0, // Skip to: 1678
/* 1661 */    MCD::OPC_CheckPredicate, 24, 22, 6, 0, // Skip to: 3224
/* 1666 */    MCD::OPC_CheckField, 11, 5, 0, 15, 6, 0, // Skip to: 3224
/* 1673 */    MCD::OPC_Decode, 225, 7, 156, 1, // Opcode: CLASS_S_MMR6
/* 1678 */    MCD::OPC_FilterValue, 112, 10, 0, 0, // Skip to: 1693
/* 1683 */    MCD::OPC_CheckPredicate, 26, 0, 6, 0, // Skip to: 3224
/* 1688 */    MCD::OPC_Decode, 204, 12, 157, 1, // Opcode: FSUB_S_MMR6
/* 1693 */    MCD::OPC_FilterValue, 120, 10, 0, 0, // Skip to: 1708
/* 1698 */    MCD::OPC_CheckPredicate, 24, 241, 5, 0, // Skip to: 3224
/* 1703 */    MCD::OPC_Decode, 196, 18, 153, 1, // Opcode: SELNEZ_S_MMR6
/* 1708 */    MCD::OPC_FilterValue, 123, 31, 0, 0, // Skip to: 1744
/* 1713 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 1716 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1730
/* 1721 */    MCD::OPC_CheckPredicate, 26, 218, 5, 0, // Skip to: 3224
/* 1726 */    MCD::OPC_Decode, 151, 12, 113, // Opcode: FMOV_S_MMR6
/* 1730 */    MCD::OPC_FilterValue, 4, 209, 5, 0, // Skip to: 3224
/* 1735 */    MCD::OPC_CheckPredicate, 26, 204, 5, 0, // Skip to: 3224
/* 1740 */    MCD::OPC_Decode, 148, 12, 111, // Opcode: FMOV_D_MMR6
/* 1744 */    MCD::OPC_FilterValue, 133, 1, 10, 0, 0, // Skip to: 1760
/* 1750 */    MCD::OPC_CheckPredicate, 26, 189, 5, 0, // Skip to: 3224
/* 1755 */    MCD::OPC_Decode, 163, 8, 154, 1, // Opcode: CMP_EQ_S_MMR6
/* 1760 */    MCD::OPC_FilterValue, 149, 1, 10, 0, 0, // Skip to: 1776
/* 1766 */    MCD::OPC_CheckPredicate, 26, 173, 5, 0, // Skip to: 3224
/* 1771 */    MCD::OPC_Decode, 159, 8, 155, 1, // Opcode: CMP_EQ_D_MMR6
/* 1776 */    MCD::OPC_FilterValue, 176, 1, 10, 0, 0, // Skip to: 1792
/* 1782 */    MCD::OPC_CheckPredicate, 26, 157, 5, 0, // Skip to: 3224
/* 1787 */    MCD::OPC_Decode, 161, 12, 157, 1, // Opcode: FMUL_S_MMR6
/* 1792 */    MCD::OPC_FilterValue, 184, 1, 10, 0, 0, // Skip to: 1808
/* 1798 */    MCD::OPC_CheckPredicate, 24, 141, 5, 0, // Skip to: 3224
/* 1803 */    MCD::OPC_Decode, 200, 18, 158, 1, // Opcode: SEL_S_MMR6
/* 1808 */    MCD::OPC_FilterValue, 197, 1, 10, 0, 0, // Skip to: 1824
/* 1814 */    MCD::OPC_CheckPredicate, 26, 125, 5, 0, // Skip to: 3224
/* 1819 */    MCD::OPC_Decode, 213, 8, 154, 1, // Opcode: CMP_UEQ_S_MMR6
/* 1824 */    MCD::OPC_FilterValue, 213, 1, 10, 0, 0, // Skip to: 1840
/* 1830 */    MCD::OPC_CheckPredicate, 26, 109, 5, 0, // Skip to: 3224
/* 1835 */    MCD::OPC_Decode, 211, 8, 155, 1, // Opcode: CMP_UEQ_D_MMR6
/* 1840 */    MCD::OPC_FilterValue, 240, 1, 10, 0, 0, // Skip to: 1856
/* 1846 */    MCD::OPC_CheckPredicate, 26, 93, 5, 0, // Skip to: 3224
/* 1851 */    MCD::OPC_Decode, 227, 11, 157, 1, // Opcode: FDIV_S_MMR6
/* 1856 */    MCD::OPC_FilterValue, 133, 2, 10, 0, 0, // Skip to: 1872
/* 1862 */    MCD::OPC_CheckPredicate, 26, 77, 5, 0, // Skip to: 3224
/* 1867 */    MCD::OPC_Decode, 177, 8, 154, 1, // Opcode: CMP_LT_S_MMR6
/* 1872 */    MCD::OPC_FilterValue, 149, 2, 10, 0, 0, // Skip to: 1888
/* 1878 */    MCD::OPC_CheckPredicate, 26, 61, 5, 0, // Skip to: 3224
/* 1883 */    MCD::OPC_Decode, 173, 8, 155, 1, // Opcode: CMP_LT_D_MMR6
/* 1888 */    MCD::OPC_FilterValue, 187, 2, 45, 0, 0, // Skip to: 1939
/* 1894 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 1897 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 1911
/* 1902 */    MCD::OPC_CheckPredicate, 26, 37, 5, 0, // Skip to: 3224
/* 1907 */    MCD::OPC_Decode, 132, 9, 110, // Opcode: CVT_L_S_MMR6
/* 1911 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1925
/* 1916 */    MCD::OPC_CheckPredicate, 26, 23, 5, 0, // Skip to: 3224
/* 1921 */    MCD::OPC_Decode, 151, 9, 113, // Opcode: CVT_W_S_MMR6
/* 1925 */    MCD::OPC_FilterValue, 8, 14, 5, 0, // Skip to: 3224
/* 1930 */    MCD::OPC_CheckPredicate, 26, 9, 5, 0, // Skip to: 3224
/* 1935 */    MCD::OPC_Decode, 129, 9, 111, // Opcode: CVT_L_D_MMR6
/* 1939 */    MCD::OPC_FilterValue, 197, 2, 10, 0, 0, // Skip to: 1955
/* 1945 */    MCD::OPC_CheckPredicate, 26, 250, 4, 0, // Skip to: 3224
/* 1950 */    MCD::OPC_Decode, 221, 8, 154, 1, // Opcode: CMP_ULT_S_MMR6
/* 1955 */    MCD::OPC_FilterValue, 213, 2, 10, 0, 0, // Skip to: 1971
/* 1961 */    MCD::OPC_CheckPredicate, 26, 234, 4, 0, // Skip to: 3224
/* 1966 */    MCD::OPC_Decode, 219, 8, 155, 1, // Opcode: CMP_ULT_D_MMR6
/* 1971 */    MCD::OPC_FilterValue, 133, 3, 10, 0, 0, // Skip to: 1987
/* 1977 */    MCD::OPC_CheckPredicate, 26, 218, 4, 0, // Skip to: 3224
/* 1982 */    MCD::OPC_Decode, 171, 8, 154, 1, // Opcode: CMP_LE_S_MMR6
/* 1987 */    MCD::OPC_FilterValue, 149, 3, 10, 0, 0, // Skip to: 2003
/* 1993 */    MCD::OPC_CheckPredicate, 26, 202, 4, 0, // Skip to: 3224
/* 1998 */    MCD::OPC_Decode, 167, 8, 155, 1, // Opcode: CMP_LE_D_MMR6
/* 2003 */    MCD::OPC_FilterValue, 184, 3, 10, 0, 0, // Skip to: 2019
/* 2009 */    MCD::OPC_CheckPredicate, 26, 186, 4, 0, // Skip to: 3224
/* 2014 */    MCD::OPC_Decode, 190, 14, 159, 1, // Opcode: MADDF_S_MMR6
/* 2019 */    MCD::OPC_FilterValue, 197, 3, 10, 0, 0, // Skip to: 2035
/* 2025 */    MCD::OPC_CheckPredicate, 26, 170, 4, 0, // Skip to: 3224
/* 2030 */    MCD::OPC_Decode, 217, 8, 154, 1, // Opcode: CMP_ULE_S_MMR6
/* 2035 */    MCD::OPC_FilterValue, 213, 3, 10, 0, 0, // Skip to: 2051
/* 2041 */    MCD::OPC_CheckPredicate, 26, 154, 4, 0, // Skip to: 3224
/* 2046 */    MCD::OPC_Decode, 215, 8, 155, 1, // Opcode: CMP_ULE_D_MMR6
/* 2051 */    MCD::OPC_FilterValue, 248, 3, 10, 0, 0, // Skip to: 2067
/* 2057 */    MCD::OPC_CheckPredicate, 26, 138, 4, 0, // Skip to: 3224
/* 2062 */    MCD::OPC_Decode, 241, 15, 159, 1, // Opcode: MSUBF_S_MMR6
/* 2067 */    MCD::OPC_FilterValue, 131, 4, 10, 0, 0, // Skip to: 2083
/* 2073 */    MCD::OPC_CheckPredicate, 26, 122, 4, 0, // Skip to: 3224
/* 2078 */    MCD::OPC_Decode, 167, 15, 134, 1, // Opcode: MIN_D_MMR6
/* 2083 */    MCD::OPC_FilterValue, 133, 4, 10, 0, 0, // Skip to: 2099
/* 2089 */    MCD::OPC_CheckPredicate, 26, 106, 4, 0, // Skip to: 3224
/* 2094 */    MCD::OPC_Decode, 181, 8, 154, 1, // Opcode: CMP_SAF_S_MMR6
/* 2099 */    MCD::OPC_FilterValue, 139, 4, 10, 0, 0, // Skip to: 2115
/* 2105 */    MCD::OPC_CheckPredicate, 26, 90, 4, 0, // Skip to: 3224
/* 2110 */    MCD::OPC_Decode, 236, 14, 134, 1, // Opcode: MAX_D_MMR6
/* 2115 */    MCD::OPC_FilterValue, 149, 4, 10, 0, 0, // Skip to: 2131
/* 2121 */    MCD::OPC_CheckPredicate, 26, 74, 4, 0, // Skip to: 3224
/* 2126 */    MCD::OPC_Decode, 179, 8, 155, 1, // Opcode: CMP_SAF_D_MMR6
/* 2131 */    MCD::OPC_FilterValue, 160, 4, 17, 0, 0, // Skip to: 2154
/* 2137 */    MCD::OPC_CheckPredicate, 24, 58, 4, 0, // Skip to: 3224
/* 2142 */    MCD::OPC_CheckField, 11, 5, 0, 51, 4, 0, // Skip to: 3224
/* 2149 */    MCD::OPC_Decode, 230, 17, 160, 1, // Opcode: RINT_D_MMR6
/* 2154 */    MCD::OPC_FilterValue, 163, 4, 10, 0, 0, // Skip to: 2170
/* 2160 */    MCD::OPC_CheckPredicate, 26, 35, 4, 0, // Skip to: 3224
/* 2165 */    MCD::OPC_Decode, 151, 15, 134, 1, // Opcode: MINA_D_MMR6
/* 2170 */    MCD::OPC_FilterValue, 171, 4, 10, 0, 0, // Skip to: 2186
/* 2176 */    MCD::OPC_CheckPredicate, 26, 19, 4, 0, // Skip to: 3224
/* 2181 */    MCD::OPC_Decode, 220, 14, 134, 1, // Opcode: MAXA_D_MMR6
/* 2186 */    MCD::OPC_FilterValue, 184, 4, 10, 0, 0, // Skip to: 2202
/* 2192 */    MCD::OPC_CheckPredicate, 24, 3, 4, 0, // Skip to: 3224
/* 2197 */    MCD::OPC_Decode, 186, 18, 134, 1, // Opcode: SELEQZ_D_MMR6
/* 2202 */    MCD::OPC_FilterValue, 197, 4, 10, 0, 0, // Skip to: 2218
/* 2208 */    MCD::OPC_CheckPredicate, 26, 243, 3, 0, // Skip to: 3224
/* 2213 */    MCD::OPC_Decode, 209, 8, 154, 1, // Opcode: CMP_SUN_S_MMR6
/* 2218 */    MCD::OPC_FilterValue, 213, 4, 10, 0, 0, // Skip to: 2234
/* 2224 */    MCD::OPC_CheckPredicate, 26, 227, 3, 0, // Skip to: 3224
/* 2229 */    MCD::OPC_Decode, 207, 8, 155, 1, // Opcode: CMP_SUN_D_MMR6
/* 2234 */    MCD::OPC_FilterValue, 224, 4, 17, 0, 0, // Skip to: 2257
/* 2240 */    MCD::OPC_CheckPredicate, 24, 211, 3, 0, // Skip to: 3224
/* 2245 */    MCD::OPC_CheckField, 11, 5, 0, 204, 3, 0, // Skip to: 3224
/* 2252 */    MCD::OPC_Decode, 223, 7, 160, 1, // Opcode: CLASS_D_MMR6
/* 2257 */    MCD::OPC_FilterValue, 248, 4, 10, 0, 0, // Skip to: 2273
/* 2263 */    MCD::OPC_CheckPredicate, 24, 188, 3, 0, // Skip to: 3224
/* 2268 */    MCD::OPC_Decode, 193, 18, 134, 1, // Opcode: SELNEZ_D_MMR6
/* 2273 */    MCD::OPC_FilterValue, 133, 5, 10, 0, 0, // Skip to: 2289
/* 2279 */    MCD::OPC_CheckPredicate, 26, 172, 3, 0, // Skip to: 3224
/* 2284 */    MCD::OPC_Decode, 185, 8, 154, 1, // Opcode: CMP_SEQ_S_MMR6
/* 2289 */    MCD::OPC_FilterValue, 149, 5, 10, 0, 0, // Skip to: 2305
/* 2295 */    MCD::OPC_CheckPredicate, 26, 156, 3, 0, // Skip to: 3224
/* 2300 */    MCD::OPC_Decode, 183, 8, 155, 1, // Opcode: CMP_SEQ_D_MMR6
/* 2305 */    MCD::OPC_FilterValue, 184, 5, 10, 0, 0, // Skip to: 2321
/* 2311 */    MCD::OPC_CheckPredicate, 24, 140, 3, 0, // Skip to: 3224
/* 2316 */    MCD::OPC_Decode, 198, 18, 161, 1, // Opcode: SEL_D_MMR6
/* 2321 */    MCD::OPC_FilterValue, 197, 5, 10, 0, 0, // Skip to: 2337
/* 2327 */    MCD::OPC_CheckPredicate, 26, 124, 3, 0, // Skip to: 3224
/* 2332 */    MCD::OPC_Decode, 197, 8, 154, 1, // Opcode: CMP_SUEQ_S_MMR6
/* 2337 */    MCD::OPC_FilterValue, 213, 5, 10, 0, 0, // Skip to: 2353
/* 2343 */    MCD::OPC_CheckPredicate, 26, 108, 3, 0, // Skip to: 3224
/* 2348 */    MCD::OPC_Decode, 195, 8, 155, 1, // Opcode: CMP_SUEQ_D_MMR6
/* 2353 */    MCD::OPC_FilterValue, 133, 6, 10, 0, 0, // Skip to: 2369
/* 2359 */    MCD::OPC_CheckPredicate, 26, 92, 3, 0, // Skip to: 3224
/* 2364 */    MCD::OPC_Decode, 193, 8, 154, 1, // Opcode: CMP_SLT_S_MMR6
/* 2369 */    MCD::OPC_FilterValue, 149, 6, 10, 0, 0, // Skip to: 2385
/* 2375 */    MCD::OPC_CheckPredicate, 26, 76, 3, 0, // Skip to: 3224
/* 2380 */    MCD::OPC_Decode, 191, 8, 155, 1, // Opcode: CMP_SLT_D_MMR6
/* 2385 */    MCD::OPC_FilterValue, 187, 6, 228, 0, 0, // Skip to: 2619
/* 2391 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 2394 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2408
/* 2399 */    MCD::OPC_CheckPredicate, 26, 52, 3, 0, // Skip to: 3224
/* 2404 */    MCD::OPC_Decode, 254, 11, 110, // Opcode: FLOOR_L_S_MMR6
/* 2408 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2422
/* 2413 */    MCD::OPC_CheckPredicate, 26, 38, 3, 0, // Skip to: 3224
/* 2418 */    MCD::OPC_Decode, 133, 12, 113, // Opcode: FLOOR_W_S_MMR6
/* 2422 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2436
/* 2427 */    MCD::OPC_CheckPredicate, 26, 24, 3, 0, // Skip to: 3224
/* 2432 */    MCD::OPC_Decode, 198, 7, 110, // Opcode: CEIL_L_S_MMR6
/* 2436 */    MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 2450
/* 2441 */    MCD::OPC_CheckPredicate, 26, 10, 3, 0, // Skip to: 3224
/* 2446 */    MCD::OPC_Decode, 205, 7, 113, // Opcode: CEIL_W_S_MMR6
/* 2450 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 2464
/* 2455 */    MCD::OPC_CheckPredicate, 26, 252, 2, 0, // Skip to: 3224
/* 2460 */    MCD::OPC_Decode, 150, 21, 110, // Opcode: TRUNC_L_S_MMR6
/* 2464 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2478
/* 2469 */    MCD::OPC_CheckPredicate, 26, 238, 2, 0, // Skip to: 3224
/* 2474 */    MCD::OPC_Decode, 157, 21, 113, // Opcode: TRUNC_W_S_MMR6
/* 2478 */    MCD::OPC_FilterValue, 6, 9, 0, 0, // Skip to: 2492
/* 2483 */    MCD::OPC_CheckPredicate, 26, 224, 2, 0, // Skip to: 3224
/* 2488 */    MCD::OPC_Decode, 240, 17, 110, // Opcode: ROUND_L_S_MMR6
/* 2492 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 2506
/* 2497 */    MCD::OPC_CheckPredicate, 26, 210, 2, 0, // Skip to: 3224
/* 2502 */    MCD::OPC_Decode, 247, 17, 113, // Opcode: ROUND_W_S_MMR6
/* 2506 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 2520
/* 2511 */    MCD::OPC_CheckPredicate, 26, 196, 2, 0, // Skip to: 3224
/* 2516 */    MCD::OPC_Decode, 252, 11, 111, // Opcode: FLOOR_L_D_MMR6
/* 2520 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 2534
/* 2525 */    MCD::OPC_CheckPredicate, 26, 182, 2, 0, // Skip to: 3224
/* 2530 */    MCD::OPC_Decode, 129, 12, 115, // Opcode: FLOOR_W_D_MMR6
/* 2534 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 2548
/* 2539 */    MCD::OPC_CheckPredicate, 26, 168, 2, 0, // Skip to: 3224
/* 2544 */    MCD::OPC_Decode, 196, 7, 111, // Opcode: CEIL_L_D_MMR6
/* 2548 */    MCD::OPC_FilterValue, 11, 9, 0, 0, // Skip to: 2562
/* 2553 */    MCD::OPC_CheckPredicate, 26, 154, 2, 0, // Skip to: 3224
/* 2558 */    MCD::OPC_Decode, 201, 7, 115, // Opcode: CEIL_W_D_MMR6
/* 2562 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 2576
/* 2567 */    MCD::OPC_CheckPredicate, 26, 140, 2, 0, // Skip to: 3224
/* 2572 */    MCD::OPC_Decode, 148, 21, 111, // Opcode: TRUNC_L_D_MMR6
/* 2576 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 2591
/* 2581 */    MCD::OPC_CheckPredicate, 26, 126, 2, 0, // Skip to: 3224
/* 2586 */    MCD::OPC_Decode, 153, 21, 135, 1, // Opcode: TRUNC_W_D_MMR6
/* 2591 */    MCD::OPC_FilterValue, 14, 9, 0, 0, // Skip to: 2605
/* 2596 */    MCD::OPC_CheckPredicate, 26, 111, 2, 0, // Skip to: 3224
/* 2601 */    MCD::OPC_Decode, 238, 17, 111, // Opcode: ROUND_L_D_MMR6
/* 2605 */    MCD::OPC_FilterValue, 15, 102, 2, 0, // Skip to: 3224
/* 2610 */    MCD::OPC_CheckPredicate, 26, 97, 2, 0, // Skip to: 3224
/* 2615 */    MCD::OPC_Decode, 243, 17, 111, // Opcode: ROUND_W_D_MMR6
/* 2619 */    MCD::OPC_FilterValue, 197, 6, 10, 0, 0, // Skip to: 2635
/* 2625 */    MCD::OPC_CheckPredicate, 26, 82, 2, 0, // Skip to: 3224
/* 2630 */    MCD::OPC_Decode, 205, 8, 154, 1, // Opcode: CMP_SULT_S_MMR6
/* 2635 */    MCD::OPC_FilterValue, 213, 6, 10, 0, 0, // Skip to: 2651
/* 2641 */    MCD::OPC_CheckPredicate, 26, 66, 2, 0, // Skip to: 3224
/* 2646 */    MCD::OPC_Decode, 203, 8, 155, 1, // Opcode: CMP_SULT_D_MMR6
/* 2651 */    MCD::OPC_FilterValue, 251, 6, 59, 0, 0, // Skip to: 2716
/* 2657 */    MCD::OPC_ExtractField, 11, 5,  // Inst{15-11} ...
/* 2660 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2674
/* 2665 */    MCD::OPC_CheckPredicate, 26, 42, 2, 0, // Skip to: 3224
/* 2670 */    MCD::OPC_Decode, 169, 12, 113, // Opcode: FNEG_S_MMR6
/* 2674 */    MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 2688
/* 2679 */    MCD::OPC_CheckPredicate, 26, 28, 2, 0, // Skip to: 3224
/* 2684 */    MCD::OPC_Decode, 144, 9, 113, // Opcode: CVT_S_W_MMR6
/* 2688 */    MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 2702
/* 2693 */    MCD::OPC_CheckPredicate, 23, 14, 2, 0, // Skip to: 3224
/* 2698 */    MCD::OPC_Decode, 254, 8, 111, // Opcode: CVT_D_L_MMR6
/* 2702 */    MCD::OPC_FilterValue, 11, 5, 2, 0, // Skip to: 3224
/* 2707 */    MCD::OPC_CheckPredicate, 23, 0, 2, 0, // Skip to: 3224
/* 2712 */    MCD::OPC_Decode, 139, 9, 110, // Opcode: CVT_S_L_MMR6
/* 2716 */    MCD::OPC_FilterValue, 133, 7, 10, 0, 0, // Skip to: 2732
/* 2722 */    MCD::OPC_CheckPredicate, 26, 241, 1, 0, // Skip to: 3224
/* 2727 */    MCD::OPC_Decode, 189, 8, 154, 1, // Opcode: CMP_SLE_S_MMR6
/* 2732 */    MCD::OPC_FilterValue, 149, 7, 10, 0, 0, // Skip to: 2748
/* 2738 */    MCD::OPC_CheckPredicate, 26, 225, 1, 0, // Skip to: 3224
/* 2743 */    MCD::OPC_Decode, 187, 8, 155, 1, // Opcode: CMP_SLE_D_MMR6
/* 2748 */    MCD::OPC_FilterValue, 184, 7, 10, 0, 0, // Skip to: 2764
/* 2754 */    MCD::OPC_CheckPredicate, 26, 209, 1, 0, // Skip to: 3224
/* 2759 */    MCD::OPC_Decode, 188, 14, 161, 1, // Opcode: MADDF_D_MMR6
/* 2764 */    MCD::OPC_FilterValue, 197, 7, 10, 0, 0, // Skip to: 2780
/* 2770 */    MCD::OPC_CheckPredicate, 26, 193, 1, 0, // Skip to: 3224
/* 2775 */    MCD::OPC_Decode, 201, 8, 154, 1, // Opcode: CMP_SULE_S_MMR6
/* 2780 */    MCD::OPC_FilterValue, 213, 7, 10, 0, 0, // Skip to: 2796
/* 2786 */    MCD::OPC_CheckPredicate, 26, 177, 1, 0, // Skip to: 3224
/* 2791 */    MCD::OPC_Decode, 199, 8, 155, 1, // Opcode: CMP_SULE_D_MMR6
/* 2796 */    MCD::OPC_FilterValue, 248, 7, 166, 1, 0, // Skip to: 3224
/* 2802 */    MCD::OPC_CheckPredicate, 26, 161, 1, 0, // Skip to: 3224
/* 2807 */    MCD::OPC_Decode, 239, 15, 161, 1, // Opcode: MSUBF_D_MMR6
/* 2812 */    MCD::OPC_FilterValue, 24, 59, 0, 0, // Skip to: 2876
/* 2817 */    MCD::OPC_ExtractField, 12, 4,  // Inst{15-12} ...
/* 2820 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2834
/* 2825 */    MCD::OPC_CheckPredicate, 24, 138, 1, 0, // Skip to: 3224
/* 2830 */    MCD::OPC_Decode, 202, 17, 89, // Opcode: PREF_MMR6
/* 2834 */    MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 2855
/* 2839 */    MCD::OPC_CheckPredicate, 24, 124, 1, 0, // Skip to: 3224
/* 2844 */    MCD::OPC_CheckField, 9, 3, 0, 117, 1, 0, // Skip to: 3224
/* 2851 */    MCD::OPC_Decode, 250, 13, 123, // Opcode: LL_MMR6
/* 2855 */    MCD::OPC_FilterValue, 11, 108, 1, 0, // Skip to: 3224
/* 2860 */    MCD::OPC_CheckPredicate, 24, 103, 1, 0, // Skip to: 3224
/* 2865 */    MCD::OPC_CheckField, 9, 3, 0, 96, 1, 0, // Skip to: 3224
/* 2872 */    MCD::OPC_Decode, 154, 18, 123, // Opcode: SC_MMR6
/* 2876 */    MCD::OPC_FilterValue, 28, 9, 0, 0, // Skip to: 2890
/* 2881 */    MCD::OPC_CheckPredicate, 24, 82, 1, 0, // Skip to: 3224
/* 2886 */    MCD::OPC_Decode, 182, 21, 97, // Opcode: XORI_MMR6
/* 2890 */    MCD::OPC_FilterValue, 29, 27, 0, 0, // Skip to: 2922
/* 2895 */    MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 2912
/* 2900 */    MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 2912
/* 2907 */    MCD::OPC_Decode, 164, 6, 162, 1, // Opcode: BEQZALC_MMR6
/* 2912 */    MCD::OPC_CheckPredicate, 24, 51, 1, 0, // Skip to: 3224
/* 2917 */    MCD::OPC_Decode, 160, 6, 162, 1, // Opcode: BEQC_MMR6
/* 2922 */    MCD::OPC_FilterValue, 30, 71, 0, 0, // Skip to: 2998
/* 2927 */    MCD::OPC_ExtractField, 19, 2,  // Inst{20-19} ...
/* 2930 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2945
/* 2935 */    MCD::OPC_CheckPredicate, 24, 28, 1, 0, // Skip to: 3224
/* 2940 */    MCD::OPC_Decode, 248, 4, 163, 1, // Opcode: ADDIUPC_MMR6
/* 2945 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 2960
/* 2950 */    MCD::OPC_CheckPredicate, 24, 13, 1, 0, // Skip to: 3224
/* 2955 */    MCD::OPC_Decode, 157, 14, 163, 1, // Opcode: LWPC_MMR6
/* 2960 */    MCD::OPC_FilterValue, 3, 3, 1, 0, // Skip to: 3224
/* 2965 */    MCD::OPC_ExtractField, 16, 3,  // Inst{18-16} ...
/* 2968 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 2983
/* 2973 */    MCD::OPC_CheckPredicate, 24, 246, 0, 0, // Skip to: 3224
/* 2978 */    MCD::OPC_Decode, 221, 5, 164, 1, // Opcode: AUIPC_MMR6
/* 2983 */    MCD::OPC_FilterValue, 7, 236, 0, 0, // Skip to: 3224
/* 2988 */    MCD::OPC_CheckPredicate, 24, 231, 0, 0, // Skip to: 3224
/* 2993 */    MCD::OPC_Decode, 194, 5, 164, 1, // Opcode: ALUIPC_MMR6
/* 2998 */    MCD::OPC_FilterValue, 31, 27, 0, 0, // Skip to: 3030
/* 3003 */    MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 3020
/* 3008 */    MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 3020
/* 3015 */    MCD::OPC_Decode, 139, 7, 165, 1, // Opcode: BNEZALC_MMR6
/* 3020 */    MCD::OPC_CheckPredicate, 24, 199, 0, 0, // Skip to: 3224
/* 3025 */    MCD::OPC_Decode, 255, 6, 165, 1, // Opcode: BNEC_MMR6
/* 3030 */    MCD::OPC_FilterValue, 32, 26, 0, 0, // Skip to: 3061
/* 3035 */    MCD::OPC_CheckPredicate, 24, 11, 0, 0, // Skip to: 3051
/* 3040 */    MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 3051
/* 3047 */    MCD::OPC_Decode, 164, 13, 92, // Opcode: JIALC_MMR6
/* 3051 */    MCD::OPC_CheckPredicate, 24, 168, 0, 0, // Skip to: 3224
/* 3056 */    MCD::OPC_Decode, 169, 6, 166, 1, // Opcode: BEQZC_MMR6
/* 3061 */    MCD::OPC_FilterValue, 37, 10, 0, 0, // Skip to: 3076
/* 3066 */    MCD::OPC_CheckPredicate, 24, 153, 0, 0, // Skip to: 3224
/* 3071 */    MCD::OPC_Decode, 155, 6, 167, 1, // Opcode: BC_MMR6
/* 3076 */    MCD::OPC_FilterValue, 40, 26, 0, 0, // Skip to: 3107
/* 3081 */    MCD::OPC_CheckPredicate, 24, 11, 0, 0, // Skip to: 3097
/* 3086 */    MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 3097
/* 3093 */    MCD::OPC_Decode, 167, 13, 92, // Opcode: JIC_MMR6
/* 3097 */    MCD::OPC_CheckPredicate, 24, 122, 0, 0, // Skip to: 3224
/* 3102 */    MCD::OPC_Decode, 144, 7, 166, 1, // Opcode: BNEZC_MMR6
/* 3107 */    MCD::OPC_FilterValue, 45, 10, 0, 0, // Skip to: 3122
/* 3112 */    MCD::OPC_CheckPredicate, 24, 107, 0, 0, // Skip to: 3224
/* 3117 */    MCD::OPC_Decode, 252, 5, 167, 1, // Opcode: BALC_MMR6
/* 3122 */    MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 3137
/* 3127 */    MCD::OPC_CheckPredicate, 24, 92, 0, 0, // Skip to: 3224
/* 3132 */    MCD::OPC_Decode, 176, 6, 168, 1, // Opcode: BGEUC_MMR6
/* 3137 */    MCD::OPC_FilterValue, 52, 9, 0, 0, // Skip to: 3151
/* 3142 */    MCD::OPC_CheckPredicate, 24, 77, 0, 0, // Skip to: 3224
/* 3147 */    MCD::OPC_Decode, 202, 5, 97, // Opcode: ANDI_MMR6
/* 3151 */    MCD::OPC_FilterValue, 53, 10, 0, 0, // Skip to: 3166
/* 3156 */    MCD::OPC_CheckPredicate, 24, 63, 0, 0, // Skip to: 3224
/* 3161 */    MCD::OPC_Decode, 230, 6, 169, 1, // Opcode: BLTC_MMR6
/* 3166 */    MCD::OPC_FilterValue, 56, 10, 0, 0, // Skip to: 3181
/* 3171 */    MCD::OPC_CheckPredicate, 24, 48, 0, 0, // Skip to: 3224
/* 3176 */    MCD::OPC_Decode, 233, 6, 170, 1, // Opcode: BLTUC_MMR6
/* 3181 */    MCD::OPC_FilterValue, 61, 10, 0, 0, // Skip to: 3196
/* 3186 */    MCD::OPC_CheckPredicate, 24, 33, 0, 0, // Skip to: 3224
/* 3191 */    MCD::OPC_Decode, 173, 6, 171, 1, // Opcode: BGEC_MMR6
/* 3196 */    MCD::OPC_FilterValue, 62, 9, 0, 0, // Skip to: 3210
/* 3201 */    MCD::OPC_CheckPredicate, 24, 18, 0, 0, // Skip to: 3224
/* 3206 */    MCD::OPC_Decode, 198, 20, 87, // Opcode: SW_MMR6
/* 3210 */    MCD::OPC_FilterValue, 63, 9, 0, 0, // Skip to: 3224
/* 3215 */    MCD::OPC_CheckPredicate, 24, 4, 0, 0, // Skip to: 3224
/* 3220 */    MCD::OPC_Decode, 173, 14, 87, // Opcode: LW_MMR6
/* 3224 */    MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMicroMipsR6_Ambiguous32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 18
/* 8 */       MCD::OPC_CheckPredicate, 24, 84, 0, 0, // Skip to: 97
/* 13 */      MCD::OPC_Decode, 154, 7, 162, 1, // Opcode: BOVC_MMR6
/* 18 */      MCD::OPC_FilterValue, 31, 10, 0, 0, // Skip to: 33
/* 23 */      MCD::OPC_CheckPredicate, 24, 69, 0, 0, // Skip to: 97
/* 28 */      MCD::OPC_Decode, 147, 7, 165, 1, // Opcode: BNVC_MMR6
/* 33 */      MCD::OPC_FilterValue, 48, 27, 0, 0, // Skip to: 65
/* 38 */      MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 55
/* 43 */      MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 55
/* 50 */      MCD::OPC_Decode, 222, 6, 168, 1, // Opcode: BLEZALC_MMR6
/* 55 */      MCD::OPC_CheckPredicate, 24, 37, 0, 0, // Skip to: 97
/* 60 */      MCD::OPC_Decode, 181, 6, 168, 1, // Opcode: BGEZALC_MMR6
/* 65 */      MCD::OPC_FilterValue, 56, 27, 0, 0, // Skip to: 97
/* 70 */      MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 87
/* 75 */      MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 87
/* 82 */      MCD::OPC_Decode, 193, 6, 170, 1, // Opcode: BGTZALC_MMR6
/* 87 */      MCD::OPC_CheckPredicate, 24, 5, 0, 0, // Skip to: 97
/* 92 */      MCD::OPC_Decode, 238, 6, 170, 1, // Opcode: BLTZALC_MMR6
/* 97 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 101, 4, 0, // Skip to: 1133
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 63, 0, 0, // Skip to: 79
/* 16 */      MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 19 */      MCD::OPC_FilterValue, 0, 178, 66, 0, // Skip to: 17098
/* 24 */      MCD::OPC_ExtractField, 6, 15,  // Inst{20-6} ...
/* 27 */      MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 41
/* 32 */      MCD::OPC_CheckPredicate, 27, 32, 0, 0, // Skip to: 69
/* 37 */      MCD::OPC_Decode, 225, 19, 0, // Opcode: SSNOP
/* 41 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 55
/* 46 */      MCD::OPC_CheckPredicate, 27, 18, 0, 0, // Skip to: 69
/* 51 */      MCD::OPC_Decode, 132, 11, 0, // Opcode: EHB
/* 55 */      MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 69
/* 60 */      MCD::OPC_CheckPredicate, 28, 4, 0, 0, // Skip to: 69
/* 65 */      MCD::OPC_Decode, 141, 17, 0, // Opcode: PAUSE
/* 69 */      MCD::OPC_CheckPredicate, 27, 128, 66, 0, // Skip to: 17098
/* 74 */      MCD::OPC_Decode, 144, 19, 172, 1, // Opcode: SLL
/* 79 */      MCD::OPC_FilterValue, 1, 47, 0, 0, // Skip to: 131
/* 84 */      MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 87 */      MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 109
/* 92 */      MCD::OPC_CheckPredicate, 29, 105, 66, 0, // Skip to: 17098
/* 97 */      MCD::OPC_CheckField, 6, 5, 0, 98, 66, 0, // Skip to: 17098
/* 104 */     MCD::OPC_Decode, 200, 15, 173, 1, // Opcode: MOVF_I
/* 109 */     MCD::OPC_FilterValue, 1, 88, 66, 0, // Skip to: 17098
/* 114 */     MCD::OPC_CheckPredicate, 29, 83, 66, 0, // Skip to: 17098
/* 119 */     MCD::OPC_CheckField, 6, 5, 0, 76, 66, 0, // Skip to: 17098
/* 126 */     MCD::OPC_Decode, 220, 15, 173, 1, // Opcode: MOVT_I
/* 131 */     MCD::OPC_FilterValue, 2, 33, 0, 0, // Skip to: 169
/* 136 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 139 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 154
/* 144 */     MCD::OPC_CheckPredicate, 27, 53, 66, 0, // Skip to: 17098
/* 149 */     MCD::OPC_Decode, 203, 19, 172, 1, // Opcode: SRL
/* 154 */     MCD::OPC_FilterValue, 1, 43, 66, 0, // Skip to: 17098
/* 159 */     MCD::OPC_CheckPredicate, 28, 38, 66, 0, // Skip to: 17098
/* 164 */     MCD::OPC_Decode, 233, 17, 172, 1, // Opcode: ROTR
/* 169 */     MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 191
/* 174 */     MCD::OPC_CheckPredicate, 27, 23, 66, 0, // Skip to: 17098
/* 179 */     MCD::OPC_CheckField, 21, 5, 0, 16, 66, 0, // Skip to: 17098
/* 186 */     MCD::OPC_Decode, 183, 19, 172, 1, // Opcode: SRA
/* 191 */     MCD::OPC_FilterValue, 4, 16, 0, 0, // Skip to: 212
/* 196 */     MCD::OPC_CheckPredicate, 27, 1, 66, 0, // Skip to: 17098
/* 201 */     MCD::OPC_CheckField, 6, 5, 0, 250, 65, 0, // Skip to: 17098
/* 208 */     MCD::OPC_Decode, 153, 19, 44, // Opcode: SLLV
/* 212 */     MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 234
/* 217 */     MCD::OPC_CheckPredicate, 30, 236, 65, 0, // Skip to: 17098
/* 222 */     MCD::OPC_CheckField, 8, 3, 0, 229, 65, 0, // Skip to: 17098
/* 229 */     MCD::OPC_Decode, 252, 13, 174, 1, // Opcode: LSA
/* 234 */     MCD::OPC_FilterValue, 6, 31, 0, 0, // Skip to: 270
/* 239 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 242 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 256
/* 247 */     MCD::OPC_CheckPredicate, 27, 206, 65, 0, // Skip to: 17098
/* 252 */     MCD::OPC_Decode, 218, 19, 44, // Opcode: SRLV
/* 256 */     MCD::OPC_FilterValue, 1, 197, 65, 0, // Skip to: 17098
/* 261 */     MCD::OPC_CheckPredicate, 28, 192, 65, 0, // Skip to: 17098
/* 266 */     MCD::OPC_Decode, 234, 17, 44, // Opcode: ROTRV
/* 270 */     MCD::OPC_FilterValue, 7, 16, 0, 0, // Skip to: 291
/* 275 */     MCD::OPC_CheckPredicate, 27, 178, 65, 0, // Skip to: 17098
/* 280 */     MCD::OPC_CheckField, 6, 5, 0, 171, 65, 0, // Skip to: 17098
/* 287 */     MCD::OPC_Decode, 196, 19, 44, // Opcode: SRAV
/* 291 */     MCD::OPC_FilterValue, 8, 33, 0, 0, // Skip to: 329
/* 296 */     MCD::OPC_ExtractField, 6, 15,  // Inst{20-6} ...
/* 299 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 314
/* 304 */     MCD::OPC_CheckPredicate, 31, 149, 65, 0, // Skip to: 17098
/* 309 */     MCD::OPC_Decode, 168, 13, 175, 1, // Opcode: JR
/* 314 */     MCD::OPC_FilterValue, 16, 139, 65, 0, // Skip to: 17098
/* 319 */     MCD::OPC_CheckPredicate, 32, 134, 65, 0, // Skip to: 17098
/* 324 */     MCD::OPC_Decode, 175, 13, 175, 1, // Opcode: JR_HB
/* 329 */     MCD::OPC_FilterValue, 9, 45, 0, 0, // Skip to: 379
/* 334 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 337 */     MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 358
/* 342 */     MCD::OPC_CheckPredicate, 33, 111, 65, 0, // Skip to: 17098
/* 347 */     MCD::OPC_CheckField, 16, 5, 0, 104, 65, 0, // Skip to: 17098
/* 354 */     MCD::OPC_Decode, 147, 13, 14, // Opcode: JALR
/* 358 */     MCD::OPC_FilterValue, 16, 95, 65, 0, // Skip to: 17098
/* 363 */     MCD::OPC_CheckPredicate, 34, 90, 65, 0, // Skip to: 17098
/* 368 */     MCD::OPC_CheckField, 16, 5, 0, 83, 65, 0, // Skip to: 17098
/* 375 */     MCD::OPC_Decode, 155, 13, 14, // Opcode: JALR_HB
/* 379 */     MCD::OPC_FilterValue, 10, 17, 0, 0, // Skip to: 401
/* 384 */     MCD::OPC_CheckPredicate, 35, 69, 65, 0, // Skip to: 17098
/* 389 */     MCD::OPC_CheckField, 6, 5, 0, 62, 65, 0, // Skip to: 17098
/* 396 */     MCD::OPC_Decode, 232, 15, 176, 1, // Opcode: MOVZ_I_I
/* 401 */     MCD::OPC_FilterValue, 11, 17, 0, 0, // Skip to: 423
/* 406 */     MCD::OPC_CheckPredicate, 35, 47, 65, 0, // Skip to: 17098
/* 411 */     MCD::OPC_CheckField, 6, 5, 0, 40, 65, 0, // Skip to: 17098
/* 418 */     MCD::OPC_Decode, 212, 15, 176, 1, // Opcode: MOVN_I_I
/* 423 */     MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 438
/* 428 */     MCD::OPC_CheckPredicate, 27, 25, 65, 0, // Skip to: 17098
/* 433 */     MCD::OPC_Decode, 205, 20, 177, 1, // Opcode: SYSCALL
/* 438 */     MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 452
/* 443 */     MCD::OPC_CheckPredicate, 27, 10, 65, 0, // Skip to: 17098
/* 448 */     MCD::OPC_Decode, 158, 7, 45, // Opcode: BREAK
/* 452 */     MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 467
/* 457 */     MCD::OPC_CheckPredicate, 36, 252, 64, 0, // Skip to: 17098
/* 462 */     MCD::OPC_Decode, 199, 20, 178, 1, // Opcode: SYNC
/* 467 */     MCD::OPC_FilterValue, 16, 51, 0, 0, // Skip to: 523
/* 472 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 475 */     MCD::OPC_FilterValue, 0, 234, 64, 0, // Skip to: 17098
/* 480 */     MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 483 */     MCD::OPC_FilterValue, 0, 226, 64, 0, // Skip to: 17098
/* 488 */     MCD::OPC_ExtractField, 23, 3,  // Inst{25-23} ...
/* 491 */     MCD::OPC_FilterValue, 0, 218, 64, 0, // Skip to: 17098
/* 496 */     MCD::OPC_CheckPredicate, 31, 12, 0, 0, // Skip to: 513
/* 501 */     MCD::OPC_CheckField, 21, 2, 0, 5, 0, 0, // Skip to: 513
/* 508 */     MCD::OPC_Decode, 137, 15, 179, 1, // Opcode: MFHI
/* 513 */     MCD::OPC_CheckPredicate, 37, 196, 64, 0, // Skip to: 17098
/* 518 */     MCD::OPC_Decode, 140, 15, 180, 1, // Opcode: MFHI_DSP
/* 523 */     MCD::OPC_FilterValue, 17, 43, 0, 0, // Skip to: 571
/* 528 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 531 */     MCD::OPC_FilterValue, 0, 178, 64, 0, // Skip to: 17098
/* 536 */     MCD::OPC_ExtractField, 13, 8,  // Inst{20-13} ...
/* 539 */     MCD::OPC_FilterValue, 0, 170, 64, 0, // Skip to: 17098
/* 544 */     MCD::OPC_CheckPredicate, 31, 12, 0, 0, // Skip to: 561
/* 549 */     MCD::OPC_CheckField, 11, 2, 0, 5, 0, 0, // Skip to: 561
/* 556 */     MCD::OPC_Decode, 153, 16, 175, 1, // Opcode: MTHI
/* 561 */     MCD::OPC_CheckPredicate, 37, 148, 64, 0, // Skip to: 17098
/* 566 */     MCD::OPC_Decode, 155, 16, 181, 1, // Opcode: MTHI_DSP
/* 571 */     MCD::OPC_FilterValue, 18, 51, 0, 0, // Skip to: 627
/* 576 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 579 */     MCD::OPC_FilterValue, 0, 130, 64, 0, // Skip to: 17098
/* 584 */     MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 587 */     MCD::OPC_FilterValue, 0, 122, 64, 0, // Skip to: 17098
/* 592 */     MCD::OPC_ExtractField, 23, 3,  // Inst{25-23} ...
/* 595 */     MCD::OPC_FilterValue, 0, 114, 64, 0, // Skip to: 17098
/* 600 */     MCD::OPC_CheckPredicate, 31, 12, 0, 0, // Skip to: 617
/* 605 */     MCD::OPC_CheckField, 21, 2, 0, 5, 0, 0, // Skip to: 617
/* 612 */     MCD::OPC_Decode, 143, 15, 179, 1, // Opcode: MFLO
/* 617 */     MCD::OPC_CheckPredicate, 37, 92, 64, 0, // Skip to: 17098
/* 622 */     MCD::OPC_Decode, 146, 15, 180, 1, // Opcode: MFLO_DSP
/* 627 */     MCD::OPC_FilterValue, 19, 43, 0, 0, // Skip to: 675
/* 632 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 635 */     MCD::OPC_FilterValue, 0, 74, 64, 0, // Skip to: 17098
/* 640 */     MCD::OPC_ExtractField, 13, 8,  // Inst{20-13} ...
/* 643 */     MCD::OPC_FilterValue, 0, 66, 64, 0, // Skip to: 17098
/* 648 */     MCD::OPC_CheckPredicate, 31, 12, 0, 0, // Skip to: 665
/* 653 */     MCD::OPC_CheckField, 11, 2, 0, 5, 0, 0, // Skip to: 665
/* 660 */     MCD::OPC_Decode, 160, 16, 175, 1, // Opcode: MTLO
/* 665 */     MCD::OPC_CheckPredicate, 37, 44, 64, 0, // Skip to: 17098
/* 670 */     MCD::OPC_Decode, 162, 16, 182, 1, // Opcode: MTLO_DSP
/* 675 */     MCD::OPC_FilterValue, 21, 17, 0, 0, // Skip to: 697
/* 680 */     MCD::OPC_CheckPredicate, 38, 29, 64, 0, // Skip to: 17098
/* 685 */     MCD::OPC_CheckField, 8, 3, 0, 22, 64, 0, // Skip to: 17098
/* 692 */     MCD::OPC_Decode, 163, 10, 183, 1, // Opcode: DLSA
/* 697 */     MCD::OPC_FilterValue, 24, 42, 0, 0, // Skip to: 744
/* 702 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 705 */     MCD::OPC_FilterValue, 0, 4, 64, 0, // Skip to: 17098
/* 710 */     MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 713 */     MCD::OPC_FilterValue, 0, 252, 63, 0, // Skip to: 17098
/* 718 */     MCD::OPC_CheckPredicate, 31, 11, 0, 0, // Skip to: 734
/* 723 */     MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 734
/* 730 */     MCD::OPC_Decode, 199, 16, 69, // Opcode: MULT
/* 734 */     MCD::OPC_CheckPredicate, 37, 231, 63, 0, // Skip to: 17098
/* 739 */     MCD::OPC_Decode, 202, 16, 184, 1, // Opcode: MULT_DSP
/* 744 */     MCD::OPC_FilterValue, 25, 42, 0, 0, // Skip to: 791
/* 749 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 752 */     MCD::OPC_FilterValue, 0, 213, 63, 0, // Skip to: 17098
/* 757 */     MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 760 */     MCD::OPC_FilterValue, 0, 205, 63, 0, // Skip to: 17098
/* 765 */     MCD::OPC_CheckPredicate, 31, 11, 0, 0, // Skip to: 781
/* 770 */     MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 781
/* 777 */     MCD::OPC_Decode, 205, 16, 69, // Opcode: MULTu
/* 781 */     MCD::OPC_CheckPredicate, 37, 184, 63, 0, // Skip to: 17098
/* 786 */     MCD::OPC_Decode, 200, 16, 184, 1, // Opcode: MULTU_DSP
/* 791 */     MCD::OPC_FilterValue, 26, 16, 0, 0, // Skip to: 812
/* 796 */     MCD::OPC_CheckPredicate, 31, 169, 63, 0, // Skip to: 17098
/* 801 */     MCD::OPC_CheckField, 6, 10, 0, 162, 63, 0, // Skip to: 17098
/* 808 */     MCD::OPC_Decode, 171, 18, 69, // Opcode: SDIV
/* 812 */     MCD::OPC_FilterValue, 27, 16, 0, 0, // Skip to: 833
/* 817 */     MCD::OPC_CheckPredicate, 31, 148, 63, 0, // Skip to: 17098
/* 822 */     MCD::OPC_CheckField, 6, 10, 0, 141, 63, 0, // Skip to: 17098
/* 829 */     MCD::OPC_Decode, 159, 21, 69, // Opcode: UDIV
/* 833 */     MCD::OPC_FilterValue, 32, 16, 0, 0, // Skip to: 854
/* 838 */     MCD::OPC_CheckPredicate, 27, 127, 63, 0, // Skip to: 17098
/* 843 */     MCD::OPC_CheckField, 6, 5, 0, 120, 63, 0, // Skip to: 17098
/* 850 */     MCD::OPC_Decode, 245, 4, 50, // Opcode: ADD
/* 854 */     MCD::OPC_FilterValue, 33, 16, 0, 0, // Skip to: 875
/* 859 */     MCD::OPC_CheckPredicate, 27, 106, 63, 0, // Skip to: 17098
/* 864 */     MCD::OPC_CheckField, 6, 5, 0, 99, 63, 0, // Skip to: 17098
/* 871 */     MCD::OPC_Decode, 189, 5, 50, // Opcode: ADDu
/* 875 */     MCD::OPC_FilterValue, 34, 16, 0, 0, // Skip to: 896
/* 880 */     MCD::OPC_CheckPredicate, 27, 85, 63, 0, // Skip to: 17098
/* 885 */     MCD::OPC_CheckField, 6, 5, 0, 78, 63, 0, // Skip to: 17098
/* 892 */     MCD::OPC_Decode, 232, 19, 50, // Opcode: SUB
/* 896 */     MCD::OPC_FilterValue, 35, 16, 0, 0, // Skip to: 917
/* 901 */     MCD::OPC_CheckPredicate, 27, 64, 63, 0, // Skip to: 17098
/* 906 */     MCD::OPC_CheckField, 6, 5, 0, 57, 63, 0, // Skip to: 17098
/* 913 */     MCD::OPC_Decode, 160, 20, 50, // Opcode: SUBu
/* 917 */     MCD::OPC_FilterValue, 36, 16, 0, 0, // Skip to: 938
/* 922 */     MCD::OPC_CheckPredicate, 27, 43, 63, 0, // Skip to: 17098
/* 927 */     MCD::OPC_CheckField, 6, 5, 0, 36, 63, 0, // Skip to: 17098
/* 934 */     MCD::OPC_Decode, 195, 5, 50, // Opcode: AND
/* 938 */     MCD::OPC_FilterValue, 37, 16, 0, 0, // Skip to: 959
/* 943 */     MCD::OPC_CheckPredicate, 27, 22, 63, 0, // Skip to: 17098
/* 948 */     MCD::OPC_CheckField, 6, 5, 0, 15, 63, 0, // Skip to: 17098
/* 955 */     MCD::OPC_Decode, 254, 16, 50, // Opcode: OR
/* 959 */     MCD::OPC_FilterValue, 38, 16, 0, 0, // Skip to: 980
/* 964 */     MCD::OPC_CheckPredicate, 27, 1, 63, 0, // Skip to: 17098
/* 969 */     MCD::OPC_CheckField, 6, 5, 0, 250, 62, 0, // Skip to: 17098
/* 976 */     MCD::OPC_Decode, 177, 21, 50, // Opcode: XOR
/* 980 */     MCD::OPC_FilterValue, 39, 16, 0, 0, // Skip to: 1001
/* 985 */     MCD::OPC_CheckPredicate, 27, 236, 62, 0, // Skip to: 17098
/* 990 */     MCD::OPC_CheckField, 6, 5, 0, 229, 62, 0, // Skip to: 17098
/* 997 */     MCD::OPC_Decode, 244, 16, 50, // Opcode: NOR
/* 1001 */    MCD::OPC_FilterValue, 42, 16, 0, 0, // Skip to: 1022
/* 1006 */    MCD::OPC_CheckPredicate, 27, 215, 62, 0, // Skip to: 17098
/* 1011 */    MCD::OPC_CheckField, 6, 5, 0, 208, 62, 0, // Skip to: 17098
/* 1018 */    MCD::OPC_Decode, 161, 19, 50, // Opcode: SLT
/* 1022 */    MCD::OPC_FilterValue, 43, 16, 0, 0, // Skip to: 1043
/* 1027 */    MCD::OPC_CheckPredicate, 27, 194, 62, 0, // Skip to: 17098
/* 1032 */    MCD::OPC_CheckField, 6, 5, 0, 187, 62, 0, // Skip to: 17098
/* 1039 */    MCD::OPC_Decode, 170, 19, 50, // Opcode: SLTu
/* 1043 */    MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 1058
/* 1048 */    MCD::OPC_CheckPredicate, 36, 173, 62, 0, // Skip to: 17098
/* 1053 */    MCD::OPC_Decode, 232, 20, 185, 1, // Opcode: TGE
/* 1058 */    MCD::OPC_FilterValue, 49, 10, 0, 0, // Skip to: 1073
/* 1063 */    MCD::OPC_CheckPredicate, 36, 158, 62, 0, // Skip to: 17098
/* 1068 */    MCD::OPC_Decode, 237, 20, 185, 1, // Opcode: TGEU
/* 1073 */    MCD::OPC_FilterValue, 50, 10, 0, 0, // Skip to: 1088
/* 1078 */    MCD::OPC_CheckPredicate, 36, 143, 62, 0, // Skip to: 17098
/* 1083 */    MCD::OPC_Decode, 136, 21, 185, 1, // Opcode: TLT
/* 1088 */    MCD::OPC_FilterValue, 51, 10, 0, 0, // Skip to: 1103
/* 1093 */    MCD::OPC_CheckPredicate, 36, 128, 62, 0, // Skip to: 17098
/* 1098 */    MCD::OPC_Decode, 140, 21, 185, 1, // Opcode: TLTU
/* 1103 */    MCD::OPC_FilterValue, 52, 10, 0, 0, // Skip to: 1118
/* 1108 */    MCD::OPC_CheckPredicate, 36, 113, 62, 0, // Skip to: 17098
/* 1113 */    MCD::OPC_Decode, 228, 20, 185, 1, // Opcode: TEQ
/* 1118 */    MCD::OPC_FilterValue, 54, 103, 62, 0, // Skip to: 17098
/* 1123 */    MCD::OPC_CheckPredicate, 36, 98, 62, 0, // Skip to: 17098
/* 1128 */    MCD::OPC_Decode, 143, 21, 185, 1, // Opcode: TNE
/* 1133 */    MCD::OPC_FilterValue, 1, 250, 0, 0, // Skip to: 1388
/* 1138 */    MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 1141 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 1156
/* 1146 */    MCD::OPC_CheckPredicate, 27, 75, 62, 0, // Skip to: 17098
/* 1151 */    MCD::OPC_Decode, 234, 6, 186, 1, // Opcode: BLTZ
/* 1156 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 1171
/* 1161 */    MCD::OPC_CheckPredicate, 27, 60, 62, 0, // Skip to: 17098
/* 1166 */    MCD::OPC_Decode, 177, 6, 186, 1, // Opcode: BGEZ
/* 1171 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1186
/* 1176 */    MCD::OPC_CheckPredicate, 39, 45, 62, 0, // Skip to: 17098
/* 1181 */    MCD::OPC_Decode, 245, 6, 186, 1, // Opcode: BLTZL
/* 1186 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1201
/* 1191 */    MCD::OPC_CheckPredicate, 39, 30, 62, 0, // Skip to: 17098
/* 1196 */    MCD::OPC_Decode, 188, 6, 186, 1, // Opcode: BGEZL
/* 1201 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 1216
/* 1206 */    MCD::OPC_CheckPredicate, 39, 15, 62, 0, // Skip to: 17098
/* 1211 */    MCD::OPC_Decode, 233, 20, 164, 1, // Opcode: TGEI
/* 1216 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1231
/* 1221 */    MCD::OPC_CheckPredicate, 39, 0, 62, 0, // Skip to: 17098
/* 1226 */    MCD::OPC_Decode, 234, 20, 164, 1, // Opcode: TGEIU
/* 1231 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1246
/* 1236 */    MCD::OPC_CheckPredicate, 39, 241, 61, 0, // Skip to: 17098
/* 1241 */    MCD::OPC_Decode, 137, 21, 164, 1, // Opcode: TLTI
/* 1246 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1261
/* 1251 */    MCD::OPC_CheckPredicate, 39, 226, 61, 0, // Skip to: 17098
/* 1256 */    MCD::OPC_Decode, 158, 21, 164, 1, // Opcode: TTLTIU
/* 1261 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 1276
/* 1266 */    MCD::OPC_CheckPredicate, 39, 211, 61, 0, // Skip to: 17098
/* 1271 */    MCD::OPC_Decode, 229, 20, 164, 1, // Opcode: TEQI
/* 1276 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 1291
/* 1281 */    MCD::OPC_CheckPredicate, 39, 196, 61, 0, // Skip to: 17098
/* 1286 */    MCD::OPC_Decode, 144, 21, 164, 1, // Opcode: TNEI
/* 1291 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 1306
/* 1296 */    MCD::OPC_CheckPredicate, 31, 181, 61, 0, // Skip to: 17098
/* 1301 */    MCD::OPC_Decode, 236, 6, 186, 1, // Opcode: BLTZAL
/* 1306 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 1321
/* 1311 */    MCD::OPC_CheckPredicate, 31, 166, 61, 0, // Skip to: 17098
/* 1316 */    MCD::OPC_Decode, 179, 6, 186, 1, // Opcode: BGEZAL
/* 1321 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 1336
/* 1326 */    MCD::OPC_CheckPredicate, 39, 151, 61, 0, // Skip to: 17098
/* 1331 */    MCD::OPC_Decode, 239, 6, 186, 1, // Opcode: BLTZALL
/* 1336 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 1351
/* 1341 */    MCD::OPC_CheckPredicate, 39, 136, 61, 0, // Skip to: 17098
/* 1346 */    MCD::OPC_Decode, 182, 6, 186, 1, // Opcode: BGEZALL
/* 1351 */    MCD::OPC_FilterValue, 28, 17, 0, 0, // Skip to: 1373
/* 1356 */    MCD::OPC_CheckPredicate, 40, 121, 61, 0, // Skip to: 17098
/* 1361 */    MCD::OPC_CheckField, 21, 5, 0, 114, 61, 0, // Skip to: 17098
/* 1368 */    MCD::OPC_Decode, 155, 7, 187, 1, // Opcode: BPOSGE32
/* 1373 */    MCD::OPC_FilterValue, 31, 104, 61, 0, // Skip to: 17098
/* 1378 */    MCD::OPC_CheckPredicate, 28, 99, 61, 0, // Skip to: 17098
/* 1383 */    MCD::OPC_Decode, 200, 20, 188, 1, // Opcode: SYNCI
/* 1388 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1403
/* 1393 */    MCD::OPC_CheckPredicate, 27, 84, 61, 0, // Skip to: 17098
/* 1398 */    MCD::OPC_Decode, 145, 13, 189, 1, // Opcode: J
/* 1403 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1418
/* 1408 */    MCD::OPC_CheckPredicate, 27, 69, 61, 0, // Skip to: 17098
/* 1413 */    MCD::OPC_Decode, 146, 13, 189, 1, // Opcode: JAL
/* 1418 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 1433
/* 1423 */    MCD::OPC_CheckPredicate, 27, 54, 61, 0, // Skip to: 17098
/* 1428 */    MCD::OPC_Decode, 156, 6, 190, 1, // Opcode: BEQ
/* 1433 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1448
/* 1438 */    MCD::OPC_CheckPredicate, 27, 39, 61, 0, // Skip to: 17098
/* 1443 */    MCD::OPC_Decode, 251, 6, 190, 1, // Opcode: BNE
/* 1448 */    MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 1470
/* 1453 */    MCD::OPC_CheckPredicate, 27, 24, 61, 0, // Skip to: 17098
/* 1458 */    MCD::OPC_CheckField, 16, 5, 0, 17, 61, 0, // Skip to: 17098
/* 1465 */    MCD::OPC_Decode, 219, 6, 186, 1, // Opcode: BLEZ
/* 1470 */    MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 1492
/* 1475 */    MCD::OPC_CheckPredicate, 27, 2, 61, 0, // Skip to: 17098
/* 1480 */    MCD::OPC_CheckField, 16, 5, 0, 251, 60, 0, // Skip to: 17098
/* 1487 */    MCD::OPC_Decode, 190, 6, 186, 1, // Opcode: BGTZ
/* 1492 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 1507
/* 1497 */    MCD::OPC_CheckPredicate, 31, 236, 60, 0, // Skip to: 17098
/* 1502 */    MCD::OPC_Decode, 185, 5, 191, 1, // Opcode: ADDi
/* 1507 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1522
/* 1512 */    MCD::OPC_CheckPredicate, 27, 221, 60, 0, // Skip to: 17098
/* 1517 */    MCD::OPC_Decode, 187, 5, 191, 1, // Opcode: ADDiu
/* 1522 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1537
/* 1527 */    MCD::OPC_CheckPredicate, 27, 206, 60, 0, // Skip to: 17098
/* 1532 */    MCD::OPC_Decode, 164, 19, 191, 1, // Opcode: SLTi
/* 1537 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1552
/* 1542 */    MCD::OPC_CheckPredicate, 27, 191, 60, 0, // Skip to: 17098
/* 1547 */    MCD::OPC_Decode, 167, 19, 191, 1, // Opcode: SLTiu
/* 1552 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 1567
/* 1557 */    MCD::OPC_CheckPredicate, 27, 176, 60, 0, // Skip to: 17098
/* 1562 */    MCD::OPC_Decode, 206, 5, 192, 1, // Opcode: ANDi
/* 1567 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 1582
/* 1572 */    MCD::OPC_CheckPredicate, 27, 161, 60, 0, // Skip to: 17098
/* 1577 */    MCD::OPC_Decode, 135, 17, 192, 1, // Opcode: ORi
/* 1582 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 1597
/* 1587 */    MCD::OPC_CheckPredicate, 27, 146, 60, 0, // Skip to: 17098
/* 1592 */    MCD::OPC_Decode, 186, 21, 192, 1, // Opcode: XORi
/* 1597 */    MCD::OPC_FilterValue, 15, 16, 0, 0, // Skip to: 1618
/* 1602 */    MCD::OPC_CheckPredicate, 27, 131, 60, 0, // Skip to: 17098
/* 1607 */    MCD::OPC_CheckField, 21, 5, 0, 124, 60, 0, // Skip to: 17098
/* 1614 */    MCD::OPC_Decode, 131, 14, 92, // Opcode: LUi
/* 1618 */    MCD::OPC_FilterValue, 16, 187, 2, 0, // Skip to: 2322
/* 1623 */    MCD::OPC_ExtractField, 3, 1,  // Inst{3} ...
/* 1626 */    MCD::OPC_FilterValue, 0, 190, 1, 0, // Skip to: 2077
/* 1631 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 1634 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 1656
/* 1639 */    MCD::OPC_CheckPredicate, 27, 94, 60, 0, // Skip to: 17098
/* 1644 */    MCD::OPC_CheckField, 4, 7, 0, 87, 60, 0, // Skip to: 17098
/* 1651 */    MCD::OPC_Decode, 247, 14, 193, 1, // Opcode: MFC0
/* 1656 */    MCD::OPC_FilterValue, 3, 63, 0, 0, // Skip to: 1724
/* 1661 */    MCD::OPC_ExtractField, 4, 7,  // Inst{10-4} ...
/* 1664 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 1679
/* 1669 */    MCD::OPC_CheckPredicate, 41, 64, 60, 0, // Skip to: 17098
/* 1674 */    MCD::OPC_Decode, 255, 14, 193, 1, // Opcode: MFGC0
/* 1679 */    MCD::OPC_FilterValue, 32, 10, 0, 0, // Skip to: 1694
/* 1684 */    MCD::OPC_CheckPredicate, 41, 49, 60, 0, // Skip to: 17098
/* 1689 */    MCD::OPC_Decode, 143, 16, 194, 1, // Opcode: MTGC0
/* 1694 */    MCD::OPC_FilterValue, 64, 10, 0, 0, // Skip to: 1709
/* 1699 */    MCD::OPC_CheckPredicate, 41, 34, 60, 0, // Skip to: 17098
/* 1704 */    MCD::OPC_Decode, 135, 15, 193, 1, // Opcode: MFHGC0
/* 1709 */    MCD::OPC_FilterValue, 96, 24, 60, 0, // Skip to: 17098
/* 1714 */    MCD::OPC_CheckPredicate, 41, 19, 60, 0, // Skip to: 17098
/* 1719 */    MCD::OPC_Decode, 151, 16, 194, 1, // Opcode: MTHGC0
/* 1724 */    MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 1746
/* 1729 */    MCD::OPC_CheckPredicate, 27, 4, 60, 0, // Skip to: 17098
/* 1734 */    MCD::OPC_CheckField, 4, 7, 0, 253, 59, 0, // Skip to: 17098
/* 1741 */    MCD::OPC_Decode, 134, 16, 194, 1, // Opcode: MTC0
/* 1746 */    MCD::OPC_FilterValue, 8, 17, 0, 0, // Skip to: 1768
/* 1751 */    MCD::OPC_CheckPredicate, 42, 238, 59, 0, // Skip to: 17098
/* 1756 */    MCD::OPC_CheckField, 6, 5, 0, 231, 59, 0, // Skip to: 17098
/* 1763 */    MCD::OPC_Decode, 149, 15, 195, 1, // Opcode: MFTR
/* 1768 */    MCD::OPC_FilterValue, 11, 133, 0, 0, // Skip to: 1906
/* 1773 */    MCD::OPC_ExtractField, 4, 12,  // Inst{15-4} ...
/* 1776 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 1797
/* 1781 */    MCD::OPC_CheckPredicate, 42, 208, 59, 0, // Skip to: 17098
/* 1786 */    MCD::OPC_CheckField, 0, 3, 1, 201, 59, 0, // Skip to: 17098
/* 1793 */    MCD::OPC_Decode, 128, 11, 81, // Opcode: DVPE
/* 1797 */    MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 1818
/* 1802 */    MCD::OPC_CheckPredicate, 42, 187, 59, 0, // Skip to: 17098
/* 1807 */    MCD::OPC_CheckField, 0, 3, 1, 180, 59, 0, // Skip to: 17098
/* 1814 */    MCD::OPC_Decode, 145, 11, 81, // Opcode: EVPE
/* 1818 */    MCD::OPC_FilterValue, 188, 1, 16, 0, 0, // Skip to: 1840
/* 1824 */    MCD::OPC_CheckPredicate, 42, 165, 59, 0, // Skip to: 17098
/* 1829 */    MCD::OPC_CheckField, 0, 3, 1, 158, 59, 0, // Skip to: 17098
/* 1836 */    MCD::OPC_Decode, 172, 10, 81, // Opcode: DMT
/* 1840 */    MCD::OPC_FilterValue, 190, 1, 16, 0, 0, // Skip to: 1862
/* 1846 */    MCD::OPC_CheckPredicate, 42, 143, 59, 0, // Skip to: 17098
/* 1851 */    MCD::OPC_CheckField, 0, 3, 1, 136, 59, 0, // Skip to: 17098
/* 1858 */    MCD::OPC_Decode, 138, 11, 81, // Opcode: EMT
/* 1862 */    MCD::OPC_FilterValue, 128, 12, 16, 0, 0, // Skip to: 1884
/* 1868 */    MCD::OPC_CheckPredicate, 28, 121, 59, 0, // Skip to: 17098
/* 1873 */    MCD::OPC_CheckField, 0, 3, 0, 114, 59, 0, // Skip to: 17098
/* 1880 */    MCD::OPC_Decode, 145, 10, 81, // Opcode: DI
/* 1884 */    MCD::OPC_FilterValue, 130, 12, 104, 59, 0, // Skip to: 17098
/* 1890 */    MCD::OPC_CheckPredicate, 28, 99, 59, 0, // Skip to: 17098
/* 1895 */    MCD::OPC_CheckField, 0, 3, 0, 92, 59, 0, // Skip to: 17098
/* 1902 */    MCD::OPC_Decode, 135, 11, 81, // Opcode: EI
/* 1906 */    MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 1928
/* 1911 */    MCD::OPC_CheckPredicate, 42, 78, 59, 0, // Skip to: 17098
/* 1916 */    MCD::OPC_CheckField, 6, 5, 0, 71, 59, 0, // Skip to: 17098
/* 1923 */    MCD::OPC_Decode, 171, 16, 195, 1, // Opcode: MTTR
/* 1928 */    MCD::OPC_FilterValue, 16, 61, 59, 0, // Skip to: 17098
/* 1933 */    MCD::OPC_ExtractField, 0, 3,  // Inst{2-0} ...
/* 1936 */    MCD::OPC_FilterValue, 0, 31, 0, 0, // Skip to: 1972
/* 1941 */    MCD::OPC_ExtractField, 4, 17,  // Inst{20-4} ...
/* 1944 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 1958
/* 1949 */    MCD::OPC_CheckPredicate, 41, 40, 59, 0, // Skip to: 17098
/* 1954 */    MCD::OPC_Decode, 244, 20, 0, // Opcode: TLBGP
/* 1958 */    MCD::OPC_FilterValue, 2, 31, 59, 0, // Skip to: 17098
/* 1963 */    MCD::OPC_CheckPredicate, 43, 26, 59, 0, // Skip to: 17098
/* 1968 */    MCD::OPC_Decode, 168, 21, 0, // Opcode: WAIT
/* 1972 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 1993
/* 1977 */    MCD::OPC_CheckPredicate, 27, 12, 59, 0, // Skip to: 17098
/* 1982 */    MCD::OPC_CheckField, 4, 17, 0, 5, 59, 0, // Skip to: 17098
/* 1989 */    MCD::OPC_Decode, 130, 21, 0, // Opcode: TLBR
/* 1993 */    MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 2014
/* 1998 */    MCD::OPC_CheckPredicate, 27, 247, 58, 0, // Skip to: 17098
/* 2003 */    MCD::OPC_CheckField, 4, 17, 0, 240, 58, 0, // Skip to: 17098
/* 2010 */    MCD::OPC_Decode, 132, 21, 0, // Opcode: TLBWI
/* 2014 */    MCD::OPC_FilterValue, 3, 16, 0, 0, // Skip to: 2035
/* 2019 */    MCD::OPC_CheckPredicate, 44, 226, 58, 0, // Skip to: 17098
/* 2024 */    MCD::OPC_CheckField, 4, 17, 0, 219, 58, 0, // Skip to: 17098
/* 2031 */    MCD::OPC_Decode, 252, 20, 0, // Opcode: TLBINV
/* 2035 */    MCD::OPC_FilterValue, 4, 16, 0, 0, // Skip to: 2056
/* 2040 */    MCD::OPC_CheckPredicate, 44, 205, 58, 0, // Skip to: 17098
/* 2045 */    MCD::OPC_CheckField, 4, 17, 0, 198, 58, 0, // Skip to: 17098
/* 2052 */    MCD::OPC_Decode, 253, 20, 0, // Opcode: TLBINVF
/* 2056 */    MCD::OPC_FilterValue, 6, 189, 58, 0, // Skip to: 17098
/* 2061 */    MCD::OPC_CheckPredicate, 27, 184, 58, 0, // Skip to: 17098
/* 2066 */    MCD::OPC_CheckField, 4, 17, 0, 177, 58, 0, // Skip to: 17098
/* 2073 */    MCD::OPC_Decode, 134, 21, 0, // Opcode: TLBWR
/* 2077 */    MCD::OPC_FilterValue, 1, 168, 58, 0, // Skip to: 17098
/* 2082 */    MCD::OPC_ExtractField, 0, 3,  // Inst{2-0} ...
/* 2085 */    MCD::OPC_FilterValue, 0, 88, 0, 0, // Skip to: 2178
/* 2090 */    MCD::OPC_ExtractField, 4, 2,  // Inst{5-4} ...
/* 2093 */    MCD::OPC_FilterValue, 0, 18, 0, 0, // Skip to: 2116
/* 2098 */    MCD::OPC_CheckPredicate, 27, 147, 58, 0, // Skip to: 17098
/* 2103 */    MCD::OPC_CheckField, 6, 20, 128, 128, 32, 138, 58, 0, // Skip to: 17098
/* 2112 */    MCD::OPC_Decode, 128, 21, 0, // Opcode: TLBP
/* 2116 */    MCD::OPC_FilterValue, 1, 35, 0, 0, // Skip to: 2156
/* 2121 */    MCD::OPC_ExtractField, 6, 20,  // Inst{25-6} ...
/* 2124 */    MCD::OPC_FilterValue, 128, 128, 32, 9, 0, 0, // Skip to: 2140
/* 2131 */    MCD::OPC_CheckPredicate, 43, 114, 58, 0, // Skip to: 17098
/* 2136 */    MCD::OPC_Decode, 139, 11, 0, // Opcode: ERET
/* 2140 */    MCD::OPC_FilterValue, 129, 128, 32, 103, 58, 0, // Skip to: 17098
/* 2147 */    MCD::OPC_CheckPredicate, 45, 98, 58, 0, // Skip to: 17098
/* 2152 */    MCD::OPC_Decode, 140, 11, 0, // Opcode: ERETNC
/* 2156 */    MCD::OPC_FilterValue, 2, 89, 58, 0, // Skip to: 17098
/* 2161 */    MCD::OPC_CheckPredicate, 41, 84, 58, 0, // Skip to: 17098
/* 2166 */    MCD::OPC_CheckField, 25, 1, 1, 77, 58, 0, // Skip to: 17098
/* 2173 */    MCD::OPC_Decode, 242, 12, 196, 1, // Opcode: HYPCALL
/* 2178 */    MCD::OPC_FilterValue, 1, 19, 0, 0, // Skip to: 2202
/* 2183 */    MCD::OPC_CheckPredicate, 41, 62, 58, 0, // Skip to: 17098
/* 2188 */    MCD::OPC_CheckField, 4, 22, 128, 128, 128, 1, 52, 58, 0, // Skip to: 17098
/* 2198 */    MCD::OPC_Decode, 246, 20, 0, // Opcode: TLBGR
/* 2202 */    MCD::OPC_FilterValue, 2, 19, 0, 0, // Skip to: 2226
/* 2207 */    MCD::OPC_CheckPredicate, 41, 38, 58, 0, // Skip to: 17098
/* 2212 */    MCD::OPC_CheckField, 4, 22, 128, 128, 128, 1, 28, 58, 0, // Skip to: 17098
/* 2222 */    MCD::OPC_Decode, 248, 20, 0, // Opcode: TLBGWI
/* 2226 */    MCD::OPC_FilterValue, 3, 19, 0, 0, // Skip to: 2250
/* 2231 */    MCD::OPC_CheckPredicate, 41, 14, 58, 0, // Skip to: 17098
/* 2236 */    MCD::OPC_CheckField, 4, 22, 128, 128, 128, 1, 4, 58, 0, // Skip to: 17098
/* 2246 */    MCD::OPC_Decode, 240, 20, 0, // Opcode: TLBGINV
/* 2250 */    MCD::OPC_FilterValue, 4, 19, 0, 0, // Skip to: 2274
/* 2255 */    MCD::OPC_CheckPredicate, 41, 246, 57, 0, // Skip to: 17098
/* 2260 */    MCD::OPC_CheckField, 4, 22, 128, 128, 128, 1, 236, 57, 0, // Skip to: 17098
/* 2270 */    MCD::OPC_Decode, 241, 20, 0, // Opcode: TLBGINVF
/* 2274 */    MCD::OPC_FilterValue, 6, 19, 0, 0, // Skip to: 2298
/* 2279 */    MCD::OPC_CheckPredicate, 41, 222, 57, 0, // Skip to: 17098
/* 2284 */    MCD::OPC_CheckField, 4, 22, 128, 128, 128, 1, 212, 57, 0, // Skip to: 17098
/* 2294 */    MCD::OPC_Decode, 250, 20, 0, // Opcode: TLBGWR
/* 2298 */    MCD::OPC_FilterValue, 7, 203, 57, 0, // Skip to: 17098
/* 2303 */    MCD::OPC_CheckPredicate, 46, 198, 57, 0, // Skip to: 17098
/* 2308 */    MCD::OPC_CheckField, 4, 22, 129, 128, 128, 1, 188, 57, 0, // Skip to: 17098
/* 2318 */    MCD::OPC_Decode, 138, 10, 0, // Opcode: DERET
/* 2322 */    MCD::OPC_FilterValue, 17, 205, 7, 0, // Skip to: 4324
/* 2327 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 2330 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 2352
/* 2335 */    MCD::OPC_CheckPredicate, 47, 166, 57, 0, // Skip to: 17098
/* 2340 */    MCD::OPC_CheckField, 0, 11, 0, 159, 57, 0, // Skip to: 17098
/* 2347 */    MCD::OPC_Decode, 249, 14, 197, 1, // Opcode: MFC1
/* 2352 */    MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 2374
/* 2357 */    MCD::OPC_CheckPredicate, 48, 144, 57, 0, // Skip to: 17098
/* 2362 */    MCD::OPC_CheckField, 0, 11, 0, 137, 57, 0, // Skip to: 17098
/* 2369 */    MCD::OPC_Decode, 166, 10, 198, 1, // Opcode: DMFC1
/* 2374 */    MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 2396
/* 2379 */    MCD::OPC_CheckPredicate, 47, 122, 57, 0, // Skip to: 17098
/* 2384 */    MCD::OPC_CheckField, 0, 11, 0, 115, 57, 0, // Skip to: 17098
/* 2391 */    MCD::OPC_Decode, 214, 7, 199, 1, // Opcode: CFC1
/* 2396 */    MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 2418
/* 2401 */    MCD::OPC_CheckPredicate, 49, 100, 57, 0, // Skip to: 17098
/* 2406 */    MCD::OPC_CheckField, 0, 11, 0, 93, 57, 0, // Skip to: 17098
/* 2413 */    MCD::OPC_Decode, 130, 15, 200, 1, // Opcode: MFHC1_D32
/* 2418 */    MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 2440
/* 2423 */    MCD::OPC_CheckPredicate, 47, 78, 57, 0, // Skip to: 17098
/* 2428 */    MCD::OPC_CheckField, 0, 11, 0, 71, 57, 0, // Skip to: 17098
/* 2435 */    MCD::OPC_Decode, 136, 16, 201, 1, // Opcode: MTC1
/* 2440 */    MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 2462
/* 2445 */    MCD::OPC_CheckPredicate, 48, 56, 57, 0, // Skip to: 17098
/* 2450 */    MCD::OPC_CheckField, 0, 11, 0, 49, 57, 0, // Skip to: 17098
/* 2457 */    MCD::OPC_Decode, 174, 10, 202, 1, // Opcode: DMTC1
/* 2462 */    MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 2484
/* 2467 */    MCD::OPC_CheckPredicate, 47, 34, 57, 0, // Skip to: 17098
/* 2472 */    MCD::OPC_CheckField, 0, 11, 0, 27, 57, 0, // Skip to: 17098
/* 2479 */    MCD::OPC_Decode, 241, 8, 203, 1, // Opcode: CTC1
/* 2484 */    MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 2506
/* 2489 */    MCD::OPC_CheckPredicate, 49, 12, 57, 0, // Skip to: 17098
/* 2494 */    MCD::OPC_CheckField, 0, 11, 0, 5, 57, 0, // Skip to: 17098
/* 2501 */    MCD::OPC_Decode, 146, 16, 204, 1, // Opcode: MTHC1_D32
/* 2506 */    MCD::OPC_FilterValue, 8, 63, 0, 0, // Skip to: 2574
/* 2511 */    MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 2514 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2529
/* 2519 */    MCD::OPC_CheckPredicate, 50, 238, 56, 0, // Skip to: 17098
/* 2524 */    MCD::OPC_Decode, 135, 6, 205, 1, // Opcode: BC1F
/* 2529 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 2544
/* 2534 */    MCD::OPC_CheckPredicate, 50, 223, 56, 0, // Skip to: 17098
/* 2539 */    MCD::OPC_Decode, 140, 6, 205, 1, // Opcode: BC1T
/* 2544 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 2559
/* 2549 */    MCD::OPC_CheckPredicate, 51, 208, 56, 0, // Skip to: 17098
/* 2554 */    MCD::OPC_Decode, 136, 6, 205, 1, // Opcode: BC1FL
/* 2559 */    MCD::OPC_FilterValue, 3, 198, 56, 0, // Skip to: 17098
/* 2564 */    MCD::OPC_CheckPredicate, 51, 193, 56, 0, // Skip to: 17098
/* 2569 */    MCD::OPC_Decode, 141, 6, 205, 1, // Opcode: BC1TL
/* 2574 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 2589
/* 2579 */    MCD::OPC_CheckPredicate, 30, 178, 56, 0, // Skip to: 17098
/* 2584 */    MCD::OPC_Decode, 176, 7, 206, 1, // Opcode: BZ_V
/* 2589 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 2604
/* 2594 */    MCD::OPC_CheckPredicate, 30, 163, 56, 0, // Skip to: 17098
/* 2599 */    MCD::OPC_Decode, 151, 7, 206, 1, // Opcode: BNZ_V
/* 2604 */    MCD::OPC_FilterValue, 16, 1, 3, 0, // Skip to: 3378
/* 2609 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 2612 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2627
/* 2617 */    MCD::OPC_CheckPredicate, 47, 140, 56, 0, // Skip to: 17098
/* 2622 */    MCD::OPC_Decode, 187, 11, 207, 1, // Opcode: FADD_S
/* 2627 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 2642
/* 2632 */    MCD::OPC_CheckPredicate, 47, 125, 56, 0, // Skip to: 17098
/* 2637 */    MCD::OPC_Decode, 202, 12, 207, 1, // Opcode: FSUB_S
/* 2642 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 2657
/* 2647 */    MCD::OPC_CheckPredicate, 47, 110, 56, 0, // Skip to: 17098
/* 2652 */    MCD::OPC_Decode, 159, 12, 207, 1, // Opcode: FMUL_S
/* 2657 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 2672
/* 2662 */    MCD::OPC_CheckPredicate, 47, 95, 56, 0, // Skip to: 17098
/* 2667 */    MCD::OPC_Decode, 225, 11, 207, 1, // Opcode: FDIV_S
/* 2672 */    MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 2694
/* 2677 */    MCD::OPC_CheckPredicate, 52, 80, 56, 0, // Skip to: 17098
/* 2682 */    MCD::OPC_CheckField, 16, 5, 0, 73, 56, 0, // Skip to: 17098
/* 2689 */    MCD::OPC_Decode, 194, 12, 208, 1, // Opcode: FSQRT_S
/* 2694 */    MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 2716
/* 2699 */    MCD::OPC_CheckPredicate, 47, 58, 56, 0, // Skip to: 17098
/* 2704 */    MCD::OPC_CheckField, 16, 5, 0, 51, 56, 0, // Skip to: 17098
/* 2711 */    MCD::OPC_Decode, 180, 11, 208, 1, // Opcode: FABS_S
/* 2716 */    MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 2738
/* 2721 */    MCD::OPC_CheckPredicate, 47, 36, 56, 0, // Skip to: 17098
/* 2726 */    MCD::OPC_CheckField, 16, 5, 0, 29, 56, 0, // Skip to: 17098
/* 2733 */    MCD::OPC_Decode, 149, 12, 208, 1, // Opcode: FMOV_S
/* 2738 */    MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 2760
/* 2743 */    MCD::OPC_CheckPredicate, 53, 14, 56, 0, // Skip to: 17098
/* 2748 */    MCD::OPC_CheckField, 16, 5, 0, 7, 56, 0, // Skip to: 17098
/* 2755 */    MCD::OPC_Decode, 167, 12, 208, 1, // Opcode: FNEG_S
/* 2760 */    MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 2782
/* 2765 */    MCD::OPC_CheckPredicate, 52, 248, 55, 0, // Skip to: 17098
/* 2770 */    MCD::OPC_CheckField, 16, 5, 0, 241, 55, 0, // Skip to: 17098
/* 2777 */    MCD::OPC_Decode, 245, 17, 208, 1, // Opcode: ROUND_W_S
/* 2782 */    MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 2804
/* 2787 */    MCD::OPC_CheckPredicate, 52, 226, 55, 0, // Skip to: 17098
/* 2792 */    MCD::OPC_CheckField, 16, 5, 0, 219, 55, 0, // Skip to: 17098
/* 2799 */    MCD::OPC_Decode, 155, 21, 208, 1, // Opcode: TRUNC_W_S
/* 2804 */    MCD::OPC_FilterValue, 14, 17, 0, 0, // Skip to: 2826
/* 2809 */    MCD::OPC_CheckPredicate, 52, 204, 55, 0, // Skip to: 17098
/* 2814 */    MCD::OPC_CheckField, 16, 5, 0, 197, 55, 0, // Skip to: 17098
/* 2821 */    MCD::OPC_Decode, 203, 7, 208, 1, // Opcode: CEIL_W_S
/* 2826 */    MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 2848
/* 2831 */    MCD::OPC_CheckPredicate, 52, 182, 55, 0, // Skip to: 17098
/* 2836 */    MCD::OPC_CheckField, 16, 5, 0, 175, 55, 0, // Skip to: 17098
/* 2843 */    MCD::OPC_Decode, 131, 12, 208, 1, // Opcode: FLOOR_W_S
/* 2848 */    MCD::OPC_FilterValue, 17, 33, 0, 0, // Skip to: 2886
/* 2853 */    MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 2856 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2871
/* 2861 */    MCD::OPC_CheckPredicate, 29, 152, 55, 0, // Skip to: 17098
/* 2866 */    MCD::OPC_Decode, 203, 15, 209, 1, // Opcode: MOVF_S
/* 2871 */    MCD::OPC_FilterValue, 1, 142, 55, 0, // Skip to: 17098
/* 2876 */    MCD::OPC_CheckPredicate, 29, 137, 55, 0, // Skip to: 17098
/* 2881 */    MCD::OPC_Decode, 223, 15, 209, 1, // Opcode: MOVT_S
/* 2886 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 2901
/* 2891 */    MCD::OPC_CheckPredicate, 29, 122, 55, 0, // Skip to: 17098
/* 2896 */    MCD::OPC_Decode, 235, 15, 210, 1, // Opcode: MOVZ_I_S
/* 2901 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 2916
/* 2906 */    MCD::OPC_CheckPredicate, 29, 107, 55, 0, // Skip to: 17098
/* 2911 */    MCD::OPC_Decode, 215, 15, 210, 1, // Opcode: MOVN_I_S
/* 2916 */    MCD::OPC_FilterValue, 21, 17, 0, 0, // Skip to: 2938
/* 2921 */    MCD::OPC_CheckPredicate, 54, 92, 55, 0, // Skip to: 17098
/* 2926 */    MCD::OPC_CheckField, 16, 5, 0, 85, 55, 0, // Skip to: 17098
/* 2933 */    MCD::OPC_Decode, 219, 17, 208, 1, // Opcode: RECIP_S
/* 2938 */    MCD::OPC_FilterValue, 22, 17, 0, 0, // Skip to: 2960
/* 2943 */    MCD::OPC_CheckPredicate, 54, 70, 55, 0, // Skip to: 17098
/* 2948 */    MCD::OPC_CheckField, 16, 5, 0, 63, 55, 0, // Skip to: 17098
/* 2955 */    MCD::OPC_Decode, 252, 17, 208, 1, // Opcode: RSQRT_S
/* 2960 */    MCD::OPC_FilterValue, 33, 17, 0, 0, // Skip to: 2982
/* 2965 */    MCD::OPC_CheckPredicate, 55, 48, 55, 0, // Skip to: 17098
/* 2970 */    MCD::OPC_CheckField, 16, 5, 0, 41, 55, 0, // Skip to: 17098
/* 2977 */    MCD::OPC_Decode, 245, 8, 211, 1, // Opcode: CVT_D32_S
/* 2982 */    MCD::OPC_FilterValue, 36, 17, 0, 0, // Skip to: 3004
/* 2987 */    MCD::OPC_CheckPredicate, 47, 26, 55, 0, // Skip to: 17098
/* 2992 */    MCD::OPC_CheckField, 16, 5, 0, 19, 55, 0, // Skip to: 17098
/* 2999 */    MCD::OPC_Decode, 149, 9, 208, 1, // Opcode: CVT_W_S
/* 3004 */    MCD::OPC_FilterValue, 37, 17, 0, 0, // Skip to: 3026
/* 3009 */    MCD::OPC_CheckPredicate, 56, 4, 55, 0, // Skip to: 17098
/* 3014 */    MCD::OPC_CheckField, 16, 5, 0, 253, 54, 0, // Skip to: 17098
/* 3021 */    MCD::OPC_Decode, 130, 9, 212, 1, // Opcode: CVT_L_S
/* 3026 */    MCD::OPC_FilterValue, 48, 17, 0, 0, // Skip to: 3048
/* 3031 */    MCD::OPC_CheckPredicate, 50, 238, 54, 0, // Skip to: 17098
/* 3036 */    MCD::OPC_CheckField, 6, 2, 0, 231, 54, 0, // Skip to: 17098
/* 3043 */    MCD::OPC_Decode, 162, 9, 213, 1, // Opcode: C_F_S
/* 3048 */    MCD::OPC_FilterValue, 49, 17, 0, 0, // Skip to: 3070
/* 3053 */    MCD::OPC_CheckPredicate, 50, 216, 54, 0, // Skip to: 17098
/* 3058 */    MCD::OPC_CheckField, 6, 2, 0, 209, 54, 0, // Skip to: 17098
/* 3065 */    MCD::OPC_Decode, 246, 9, 213, 1, // Opcode: C_UN_S
/* 3070 */    MCD::OPC_FilterValue, 50, 17, 0, 0, // Skip to: 3092
/* 3075 */    MCD::OPC_CheckPredicate, 50, 194, 54, 0, // Skip to: 17098
/* 3080 */    MCD::OPC_CheckField, 6, 2, 0, 187, 54, 0, // Skip to: 17098
/* 3087 */    MCD::OPC_Decode, 156, 9, 213, 1, // Opcode: C_EQ_S
/* 3092 */    MCD::OPC_FilterValue, 51, 17, 0, 0, // Skip to: 3114
/* 3097 */    MCD::OPC_CheckPredicate, 50, 172, 54, 0, // Skip to: 17098
/* 3102 */    MCD::OPC_CheckField, 6, 2, 0, 165, 54, 0, // Skip to: 17098
/* 3109 */    MCD::OPC_Decode, 228, 9, 213, 1, // Opcode: C_UEQ_S
/* 3114 */    MCD::OPC_FilterValue, 52, 17, 0, 0, // Skip to: 3136
/* 3119 */    MCD::OPC_CheckPredicate, 50, 150, 54, 0, // Skip to: 17098
/* 3124 */    MCD::OPC_CheckField, 6, 2, 0, 143, 54, 0, // Skip to: 17098
/* 3131 */    MCD::OPC_Decode, 210, 9, 213, 1, // Opcode: C_OLT_S
/* 3136 */    MCD::OPC_FilterValue, 53, 17, 0, 0, // Skip to: 3158
/* 3141 */    MCD::OPC_CheckPredicate, 50, 128, 54, 0, // Skip to: 17098
/* 3146 */    MCD::OPC_CheckField, 6, 2, 0, 121, 54, 0, // Skip to: 17098
/* 3153 */    MCD::OPC_Decode, 240, 9, 213, 1, // Opcode: C_ULT_S
/* 3158 */    MCD::OPC_FilterValue, 54, 17, 0, 0, // Skip to: 3180
/* 3163 */    MCD::OPC_CheckPredicate, 50, 106, 54, 0, // Skip to: 17098
/* 3168 */    MCD::OPC_CheckField, 6, 2, 0, 99, 54, 0, // Skip to: 17098
/* 3175 */    MCD::OPC_Decode, 204, 9, 213, 1, // Opcode: C_OLE_S
/* 3180 */    MCD::OPC_FilterValue, 55, 17, 0, 0, // Skip to: 3202
/* 3185 */    MCD::OPC_CheckPredicate, 50, 84, 54, 0, // Skip to: 17098
/* 3190 */    MCD::OPC_CheckField, 6, 2, 0, 77, 54, 0, // Skip to: 17098
/* 3197 */    MCD::OPC_Decode, 234, 9, 213, 1, // Opcode: C_ULE_S
/* 3202 */    MCD::OPC_FilterValue, 56, 17, 0, 0, // Skip to: 3224
/* 3207 */    MCD::OPC_CheckPredicate, 50, 62, 54, 0, // Skip to: 17098
/* 3212 */    MCD::OPC_CheckField, 6, 2, 0, 55, 54, 0, // Skip to: 17098
/* 3219 */    MCD::OPC_Decode, 222, 9, 213, 1, // Opcode: C_SF_S
/* 3224 */    MCD::OPC_FilterValue, 57, 17, 0, 0, // Skip to: 3246
/* 3229 */    MCD::OPC_CheckPredicate, 50, 40, 54, 0, // Skip to: 17098
/* 3234 */    MCD::OPC_CheckField, 6, 2, 0, 33, 54, 0, // Skip to: 17098
/* 3241 */    MCD::OPC_Decode, 186, 9, 213, 1, // Opcode: C_NGLE_S
/* 3246 */    MCD::OPC_FilterValue, 58, 17, 0, 0, // Skip to: 3268
/* 3251 */    MCD::OPC_CheckPredicate, 50, 18, 54, 0, // Skip to: 17098
/* 3256 */    MCD::OPC_CheckField, 6, 2, 0, 11, 54, 0, // Skip to: 17098
/* 3263 */    MCD::OPC_Decode, 216, 9, 213, 1, // Opcode: C_SEQ_S
/* 3268 */    MCD::OPC_FilterValue, 59, 17, 0, 0, // Skip to: 3290
/* 3273 */    MCD::OPC_CheckPredicate, 50, 252, 53, 0, // Skip to: 17098
/* 3278 */    MCD::OPC_CheckField, 6, 2, 0, 245, 53, 0, // Skip to: 17098
/* 3285 */    MCD::OPC_Decode, 192, 9, 213, 1, // Opcode: C_NGL_S
/* 3290 */    MCD::OPC_FilterValue, 60, 17, 0, 0, // Skip to: 3312
/* 3295 */    MCD::OPC_CheckPredicate, 50, 230, 53, 0, // Skip to: 17098
/* 3300 */    MCD::OPC_CheckField, 6, 2, 0, 223, 53, 0, // Skip to: 17098
/* 3307 */    MCD::OPC_Decode, 174, 9, 213, 1, // Opcode: C_LT_S
/* 3312 */    MCD::OPC_FilterValue, 61, 17, 0, 0, // Skip to: 3334
/* 3317 */    MCD::OPC_CheckPredicate, 50, 208, 53, 0, // Skip to: 17098
/* 3322 */    MCD::OPC_CheckField, 6, 2, 0, 201, 53, 0, // Skip to: 17098
/* 3329 */    MCD::OPC_Decode, 180, 9, 213, 1, // Opcode: C_NGE_S
/* 3334 */    MCD::OPC_FilterValue, 62, 17, 0, 0, // Skip to: 3356
/* 3339 */    MCD::OPC_CheckPredicate, 50, 186, 53, 0, // Skip to: 17098
/* 3344 */    MCD::OPC_CheckField, 6, 2, 0, 179, 53, 0, // Skip to: 17098
/* 3351 */    MCD::OPC_Decode, 168, 9, 213, 1, // Opcode: C_LE_S
/* 3356 */    MCD::OPC_FilterValue, 63, 169, 53, 0, // Skip to: 17098
/* 3361 */    MCD::OPC_CheckPredicate, 50, 164, 53, 0, // Skip to: 17098
/* 3366 */    MCD::OPC_CheckField, 6, 2, 0, 157, 53, 0, // Skip to: 17098
/* 3373 */    MCD::OPC_Decode, 198, 9, 213, 1, // Opcode: C_NGT_S
/* 3378 */    MCD::OPC_FilterValue, 17, 1, 3, 0, // Skip to: 4152
/* 3383 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 3386 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 3401
/* 3391 */    MCD::OPC_CheckPredicate, 55, 134, 53, 0, // Skip to: 17098
/* 3396 */    MCD::OPC_Decode, 183, 11, 214, 1, // Opcode: FADD_D32
/* 3401 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 3416
/* 3406 */    MCD::OPC_CheckPredicate, 55, 119, 53, 0, // Skip to: 17098
/* 3411 */    MCD::OPC_Decode, 198, 12, 214, 1, // Opcode: FSUB_D32
/* 3416 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 3431
/* 3421 */    MCD::OPC_CheckPredicate, 55, 104, 53, 0, // Skip to: 17098
/* 3426 */    MCD::OPC_Decode, 155, 12, 214, 1, // Opcode: FMUL_D32
/* 3431 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 3446
/* 3436 */    MCD::OPC_CheckPredicate, 55, 89, 53, 0, // Skip to: 17098
/* 3441 */    MCD::OPC_Decode, 221, 11, 214, 1, // Opcode: FDIV_D32
/* 3446 */    MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 3468
/* 3451 */    MCD::OPC_CheckPredicate, 57, 74, 53, 0, // Skip to: 17098
/* 3456 */    MCD::OPC_CheckField, 16, 5, 0, 67, 53, 0, // Skip to: 17098
/* 3463 */    MCD::OPC_Decode, 190, 12, 215, 1, // Opcode: FSQRT_D32
/* 3468 */    MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 3490
/* 3473 */    MCD::OPC_CheckPredicate, 55, 52, 53, 0, // Skip to: 17098
/* 3478 */    MCD::OPC_CheckField, 16, 5, 0, 45, 53, 0, // Skip to: 17098
/* 3485 */    MCD::OPC_Decode, 176, 11, 215, 1, // Opcode: FABS_D32
/* 3490 */    MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 3512
/* 3495 */    MCD::OPC_CheckPredicate, 55, 30, 53, 0, // Skip to: 17098
/* 3500 */    MCD::OPC_CheckField, 16, 5, 0, 23, 53, 0, // Skip to: 17098
/* 3507 */    MCD::OPC_Decode, 144, 12, 215, 1, // Opcode: FMOV_D32
/* 3512 */    MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 3534
/* 3517 */    MCD::OPC_CheckPredicate, 55, 8, 53, 0, // Skip to: 17098
/* 3522 */    MCD::OPC_CheckField, 16, 5, 0, 1, 53, 0, // Skip to: 17098
/* 3529 */    MCD::OPC_Decode, 163, 12, 215, 1, // Opcode: FNEG_D32
/* 3534 */    MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 3556
/* 3539 */    MCD::OPC_CheckPredicate, 57, 242, 52, 0, // Skip to: 17098
/* 3544 */    MCD::OPC_CheckField, 16, 5, 0, 235, 52, 0, // Skip to: 17098
/* 3551 */    MCD::OPC_Decode, 241, 17, 216, 1, // Opcode: ROUND_W_D32
/* 3556 */    MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 3578
/* 3561 */    MCD::OPC_CheckPredicate, 57, 220, 52, 0, // Skip to: 17098
/* 3566 */    MCD::OPC_CheckField, 16, 5, 0, 213, 52, 0, // Skip to: 17098
/* 3573 */    MCD::OPC_Decode, 151, 21, 216, 1, // Opcode: TRUNC_W_D32
/* 3578 */    MCD::OPC_FilterValue, 14, 17, 0, 0, // Skip to: 3600
/* 3583 */    MCD::OPC_CheckPredicate, 57, 198, 52, 0, // Skip to: 17098
/* 3588 */    MCD::OPC_CheckField, 16, 5, 0, 191, 52, 0, // Skip to: 17098
/* 3595 */    MCD::OPC_Decode, 199, 7, 216, 1, // Opcode: CEIL_W_D32
/* 3600 */    MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 3622
/* 3605 */    MCD::OPC_CheckPredicate, 57, 176, 52, 0, // Skip to: 17098
/* 3610 */    MCD::OPC_CheckField, 16, 5, 0, 169, 52, 0, // Skip to: 17098
/* 3617 */    MCD::OPC_Decode, 255, 11, 216, 1, // Opcode: FLOOR_W_D32
/* 3622 */    MCD::OPC_FilterValue, 17, 33, 0, 0, // Skip to: 3660
/* 3627 */    MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 3630 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 3645
/* 3635 */    MCD::OPC_CheckPredicate, 58, 146, 52, 0, // Skip to: 17098
/* 3640 */    MCD::OPC_Decode, 197, 15, 217, 1, // Opcode: MOVF_D32
/* 3645 */    MCD::OPC_FilterValue, 1, 136, 52, 0, // Skip to: 17098
/* 3650 */    MCD::OPC_CheckPredicate, 58, 131, 52, 0, // Skip to: 17098
/* 3655 */    MCD::OPC_Decode, 217, 15, 217, 1, // Opcode: MOVT_D32
/* 3660 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 3675
/* 3665 */    MCD::OPC_CheckPredicate, 58, 116, 52, 0, // Skip to: 17098
/* 3670 */    MCD::OPC_Decode, 229, 15, 218, 1, // Opcode: MOVZ_I_D32
/* 3675 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 3690
/* 3680 */    MCD::OPC_CheckPredicate, 58, 101, 52, 0, // Skip to: 17098
/* 3685 */    MCD::OPC_Decode, 209, 15, 218, 1, // Opcode: MOVN_I_D32
/* 3690 */    MCD::OPC_FilterValue, 21, 17, 0, 0, // Skip to: 3712
/* 3695 */    MCD::OPC_CheckPredicate, 59, 86, 52, 0, // Skip to: 17098
/* 3700 */    MCD::OPC_CheckField, 16, 5, 0, 79, 52, 0, // Skip to: 17098
/* 3707 */    MCD::OPC_Decode, 215, 17, 215, 1, // Opcode: RECIP_D32
/* 3712 */    MCD::OPC_FilterValue, 22, 17, 0, 0, // Skip to: 3734
/* 3717 */    MCD::OPC_CheckPredicate, 59, 64, 52, 0, // Skip to: 17098
/* 3722 */    MCD::OPC_CheckField, 16, 5, 0, 57, 52, 0, // Skip to: 17098
/* 3729 */    MCD::OPC_Decode, 248, 17, 215, 1, // Opcode: RSQRT_D32
/* 3734 */    MCD::OPC_FilterValue, 32, 17, 0, 0, // Skip to: 3756
/* 3739 */    MCD::OPC_CheckPredicate, 55, 42, 52, 0, // Skip to: 17098
/* 3744 */    MCD::OPC_CheckField, 16, 5, 0, 35, 52, 0, // Skip to: 17098
/* 3751 */    MCD::OPC_Decode, 134, 9, 216, 1, // Opcode: CVT_S_D32
/* 3756 */    MCD::OPC_FilterValue, 36, 17, 0, 0, // Skip to: 3778
/* 3761 */    MCD::OPC_CheckPredicate, 55, 20, 52, 0, // Skip to: 17098
/* 3766 */    MCD::OPC_CheckField, 16, 5, 0, 13, 52, 0, // Skip to: 17098
/* 3773 */    MCD::OPC_Decode, 145, 9, 216, 1, // Opcode: CVT_W_D32
/* 3778 */    MCD::OPC_FilterValue, 37, 17, 0, 0, // Skip to: 3800
/* 3783 */    MCD::OPC_CheckPredicate, 56, 254, 51, 0, // Skip to: 17098
/* 3788 */    MCD::OPC_CheckField, 16, 5, 0, 247, 51, 0, // Skip to: 17098
/* 3795 */    MCD::OPC_Decode, 255, 8, 219, 1, // Opcode: CVT_L_D64
/* 3800 */    MCD::OPC_FilterValue, 48, 17, 0, 0, // Skip to: 3822
/* 3805 */    MCD::OPC_CheckPredicate, 60, 232, 51, 0, // Skip to: 17098
/* 3810 */    MCD::OPC_CheckField, 6, 2, 0, 225, 51, 0, // Skip to: 17098
/* 3817 */    MCD::OPC_Decode, 158, 9, 220, 1, // Opcode: C_F_D32
/* 3822 */    MCD::OPC_FilterValue, 49, 17, 0, 0, // Skip to: 3844
/* 3827 */    MCD::OPC_CheckPredicate, 60, 210, 51, 0, // Skip to: 17098
/* 3832 */    MCD::OPC_CheckField, 6, 2, 0, 203, 51, 0, // Skip to: 17098
/* 3839 */    MCD::OPC_Decode, 242, 9, 220, 1, // Opcode: C_UN_D32
/* 3844 */    MCD::OPC_FilterValue, 50, 17, 0, 0, // Skip to: 3866
/* 3849 */    MCD::OPC_CheckPredicate, 60, 188, 51, 0, // Skip to: 17098
/* 3854 */    MCD::OPC_CheckField, 6, 2, 0, 181, 51, 0, // Skip to: 17098
/* 3861 */    MCD::OPC_Decode, 152, 9, 220, 1, // Opcode: C_EQ_D32
/* 3866 */    MCD::OPC_FilterValue, 51, 17, 0, 0, // Skip to: 3888
/* 3871 */    MCD::OPC_CheckPredicate, 60, 166, 51, 0, // Skip to: 17098
/* 3876 */    MCD::OPC_CheckField, 6, 2, 0, 159, 51, 0, // Skip to: 17098
/* 3883 */    MCD::OPC_Decode, 224, 9, 220, 1, // Opcode: C_UEQ_D32
/* 3888 */    MCD::OPC_FilterValue, 52, 17, 0, 0, // Skip to: 3910
/* 3893 */    MCD::OPC_CheckPredicate, 60, 144, 51, 0, // Skip to: 17098
/* 3898 */    MCD::OPC_CheckField, 6, 2, 0, 137, 51, 0, // Skip to: 17098
/* 3905 */    MCD::OPC_Decode, 206, 9, 220, 1, // Opcode: C_OLT_D32
/* 3910 */    MCD::OPC_FilterValue, 53, 17, 0, 0, // Skip to: 3932
/* 3915 */    MCD::OPC_CheckPredicate, 60, 122, 51, 0, // Skip to: 17098
/* 3920 */    MCD::OPC_CheckField, 6, 2, 0, 115, 51, 0, // Skip to: 17098
/* 3927 */    MCD::OPC_Decode, 236, 9, 220, 1, // Opcode: C_ULT_D32
/* 3932 */    MCD::OPC_FilterValue, 54, 17, 0, 0, // Skip to: 3954
/* 3937 */    MCD::OPC_CheckPredicate, 60, 100, 51, 0, // Skip to: 17098
/* 3942 */    MCD::OPC_CheckField, 6, 2, 0, 93, 51, 0, // Skip to: 17098
/* 3949 */    MCD::OPC_Decode, 200, 9, 220, 1, // Opcode: C_OLE_D32
/* 3954 */    MCD::OPC_FilterValue, 55, 17, 0, 0, // Skip to: 3976
/* 3959 */    MCD::OPC_CheckPredicate, 60, 78, 51, 0, // Skip to: 17098
/* 3964 */    MCD::OPC_CheckField, 6, 2, 0, 71, 51, 0, // Skip to: 17098
/* 3971 */    MCD::OPC_Decode, 230, 9, 220, 1, // Opcode: C_ULE_D32
/* 3976 */    MCD::OPC_FilterValue, 56, 17, 0, 0, // Skip to: 3998
/* 3981 */    MCD::OPC_CheckPredicate, 60, 56, 51, 0, // Skip to: 17098
/* 3986 */    MCD::OPC_CheckField, 6, 2, 0, 49, 51, 0, // Skip to: 17098
/* 3993 */    MCD::OPC_Decode, 218, 9, 220, 1, // Opcode: C_SF_D32
/* 3998 */    MCD::OPC_FilterValue, 57, 17, 0, 0, // Skip to: 4020
/* 4003 */    MCD::OPC_CheckPredicate, 60, 34, 51, 0, // Skip to: 17098
/* 4008 */    MCD::OPC_CheckField, 6, 2, 0, 27, 51, 0, // Skip to: 17098
/* 4015 */    MCD::OPC_Decode, 182, 9, 220, 1, // Opcode: C_NGLE_D32
/* 4020 */    MCD::OPC_FilterValue, 58, 17, 0, 0, // Skip to: 4042
/* 4025 */    MCD::OPC_CheckPredicate, 60, 12, 51, 0, // Skip to: 17098
/* 4030 */    MCD::OPC_CheckField, 6, 2, 0, 5, 51, 0, // Skip to: 17098
/* 4037 */    MCD::OPC_Decode, 212, 9, 220, 1, // Opcode: C_SEQ_D32
/* 4042 */    MCD::OPC_FilterValue, 59, 17, 0, 0, // Skip to: 4064
/* 4047 */    MCD::OPC_CheckPredicate, 60, 246, 50, 0, // Skip to: 17098
/* 4052 */    MCD::OPC_CheckField, 6, 2, 0, 239, 50, 0, // Skip to: 17098
/* 4059 */    MCD::OPC_Decode, 188, 9, 220, 1, // Opcode: C_NGL_D32
/* 4064 */    MCD::OPC_FilterValue, 60, 17, 0, 0, // Skip to: 4086
/* 4069 */    MCD::OPC_CheckPredicate, 60, 224, 50, 0, // Skip to: 17098
/* 4074 */    MCD::OPC_CheckField, 6, 2, 0, 217, 50, 0, // Skip to: 17098
/* 4081 */    MCD::OPC_Decode, 170, 9, 220, 1, // Opcode: C_LT_D32
/* 4086 */    MCD::OPC_FilterValue, 61, 17, 0, 0, // Skip to: 4108
/* 4091 */    MCD::OPC_CheckPredicate, 60, 202, 50, 0, // Skip to: 17098
/* 4096 */    MCD::OPC_CheckField, 6, 2, 0, 195, 50, 0, // Skip to: 17098
/* 4103 */    MCD::OPC_Decode, 176, 9, 220, 1, // Opcode: C_NGE_D32
/* 4108 */    MCD::OPC_FilterValue, 62, 17, 0, 0, // Skip to: 4130
/* 4113 */    MCD::OPC_CheckPredicate, 60, 180, 50, 0, // Skip to: 17098
/* 4118 */    MCD::OPC_CheckField, 6, 2, 0, 173, 50, 0, // Skip to: 17098
/* 4125 */    MCD::OPC_Decode, 164, 9, 220, 1, // Opcode: C_LE_D32
/* 4130 */    MCD::OPC_FilterValue, 63, 163, 50, 0, // Skip to: 17098
/* 4135 */    MCD::OPC_CheckPredicate, 60, 158, 50, 0, // Skip to: 17098
/* 4140 */    MCD::OPC_CheckField, 6, 2, 0, 151, 50, 0, // Skip to: 17098
/* 4147 */    MCD::OPC_Decode, 194, 9, 220, 1, // Opcode: C_NGT_D32
/* 4152 */    MCD::OPC_FilterValue, 20, 47, 0, 0, // Skip to: 4204
/* 4157 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 4160 */    MCD::OPC_FilterValue, 32, 17, 0, 0, // Skip to: 4182
/* 4165 */    MCD::OPC_CheckPredicate, 47, 128, 50, 0, // Skip to: 17098
/* 4170 */    MCD::OPC_CheckField, 16, 5, 0, 121, 50, 0, // Skip to: 17098
/* 4177 */    MCD::OPC_Decode, 142, 9, 208, 1, // Opcode: CVT_S_W
/* 4182 */    MCD::OPC_FilterValue, 33, 111, 50, 0, // Skip to: 17098
/* 4187 */    MCD::OPC_CheckPredicate, 55, 106, 50, 0, // Skip to: 17098
/* 4192 */    MCD::OPC_CheckField, 16, 5, 0, 99, 50, 0, // Skip to: 17098
/* 4199 */    MCD::OPC_Decode, 247, 8, 211, 1, // Opcode: CVT_D32_W
/* 4204 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 4219
/* 4209 */    MCD::OPC_CheckPredicate, 30, 84, 50, 0, // Skip to: 17098
/* 4214 */    MCD::OPC_Decode, 173, 7, 206, 1, // Opcode: BZ_B
/* 4219 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 4234
/* 4224 */    MCD::OPC_CheckPredicate, 30, 69, 50, 0, // Skip to: 17098
/* 4229 */    MCD::OPC_Decode, 175, 7, 221, 1, // Opcode: BZ_H
/* 4234 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 4249
/* 4239 */    MCD::OPC_CheckPredicate, 30, 54, 50, 0, // Skip to: 17098
/* 4244 */    MCD::OPC_Decode, 177, 7, 222, 1, // Opcode: BZ_W
/* 4249 */    MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 4264
/* 4254 */    MCD::OPC_CheckPredicate, 30, 39, 50, 0, // Skip to: 17098
/* 4259 */    MCD::OPC_Decode, 174, 7, 223, 1, // Opcode: BZ_D
/* 4264 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 4279
/* 4269 */    MCD::OPC_CheckPredicate, 30, 24, 50, 0, // Skip to: 17098
/* 4274 */    MCD::OPC_Decode, 148, 7, 206, 1, // Opcode: BNZ_B
/* 4279 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 4294
/* 4284 */    MCD::OPC_CheckPredicate, 30, 9, 50, 0, // Skip to: 17098
/* 4289 */    MCD::OPC_Decode, 150, 7, 221, 1, // Opcode: BNZ_H
/* 4294 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 4309
/* 4299 */    MCD::OPC_CheckPredicate, 30, 250, 49, 0, // Skip to: 17098
/* 4304 */    MCD::OPC_Decode, 152, 7, 222, 1, // Opcode: BNZ_W
/* 4309 */    MCD::OPC_FilterValue, 31, 240, 49, 0, // Skip to: 17098
/* 4314 */    MCD::OPC_CheckPredicate, 30, 235, 49, 0, // Skip to: 17098
/* 4319 */    MCD::OPC_Decode, 149, 7, 223, 1, // Opcode: BNZ_D
/* 4324 */    MCD::OPC_FilterValue, 18, 47, 0, 0, // Skip to: 4376
/* 4329 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 4332 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 4354
/* 4337 */    MCD::OPC_CheckPredicate, 27, 212, 49, 0, // Skip to: 17098
/* 4342 */    MCD::OPC_CheckField, 3, 8, 0, 205, 49, 0, // Skip to: 17098
/* 4349 */    MCD::OPC_Decode, 253, 14, 224, 1, // Opcode: MFC2
/* 4354 */    MCD::OPC_FilterValue, 4, 195, 49, 0, // Skip to: 17098
/* 4359 */    MCD::OPC_CheckPredicate, 27, 190, 49, 0, // Skip to: 17098
/* 4364 */    MCD::OPC_CheckField, 3, 8, 0, 183, 49, 0, // Skip to: 17098
/* 4371 */    MCD::OPC_Decode, 141, 16, 225, 1, // Opcode: MTC2
/* 4376 */    MCD::OPC_FilterValue, 19, 255, 0, 0, // Skip to: 4636
/* 4381 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 4384 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 4406
/* 4389 */    MCD::OPC_CheckPredicate, 61, 160, 49, 0, // Skip to: 17098
/* 4394 */    MCD::OPC_CheckField, 11, 5, 0, 153, 49, 0, // Skip to: 17098
/* 4401 */    MCD::OPC_Decode, 168, 14, 226, 1, // Opcode: LWXC1
/* 4406 */    MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 4428
/* 4411 */    MCD::OPC_CheckPredicate, 62, 138, 49, 0, // Skip to: 17098
/* 4416 */    MCD::OPC_CheckField, 11, 5, 0, 131, 49, 0, // Skip to: 17098
/* 4423 */    MCD::OPC_Decode, 218, 13, 227, 1, // Opcode: LDXC1
/* 4428 */    MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 4450
/* 4433 */    MCD::OPC_CheckPredicate, 63, 116, 49, 0, // Skip to: 17098
/* 4438 */    MCD::OPC_CheckField, 11, 5, 0, 109, 49, 0, // Skip to: 17098
/* 4445 */    MCD::OPC_Decode, 128, 14, 227, 1, // Opcode: LUXC1
/* 4450 */    MCD::OPC_FilterValue, 8, 17, 0, 0, // Skip to: 4472
/* 4455 */    MCD::OPC_CheckPredicate, 61, 94, 49, 0, // Skip to: 17098
/* 4460 */    MCD::OPC_CheckField, 6, 5, 0, 87, 49, 0, // Skip to: 17098
/* 4467 */    MCD::OPC_Decode, 195, 20, 228, 1, // Opcode: SWXC1
/* 4472 */    MCD::OPC_FilterValue, 9, 17, 0, 0, // Skip to: 4494
/* 4477 */    MCD::OPC_CheckPredicate, 62, 72, 49, 0, // Skip to: 17098
/* 4482 */    MCD::OPC_CheckField, 6, 5, 0, 65, 49, 0, // Skip to: 17098
/* 4489 */    MCD::OPC_Decode, 175, 18, 229, 1, // Opcode: SDXC1
/* 4494 */    MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 4516
/* 4499 */    MCD::OPC_CheckPredicate, 63, 50, 49, 0, // Skip to: 17098
/* 4504 */    MCD::OPC_CheckField, 6, 5, 0, 43, 49, 0, // Skip to: 17098
/* 4511 */    MCD::OPC_Decode, 162, 20, 229, 1, // Opcode: SUXC1
/* 4516 */    MCD::OPC_FilterValue, 32, 10, 0, 0, // Skip to: 4531
/* 4521 */    MCD::OPC_CheckPredicate, 64, 28, 49, 0, // Skip to: 17098
/* 4526 */    MCD::OPC_Decode, 209, 14, 230, 1, // Opcode: MADD_S
/* 4531 */    MCD::OPC_FilterValue, 33, 10, 0, 0, // Skip to: 4546
/* 4536 */    MCD::OPC_CheckPredicate, 65, 13, 49, 0, // Skip to: 17098
/* 4541 */    MCD::OPC_Decode, 201, 14, 231, 1, // Opcode: MADD_D32
/* 4546 */    MCD::OPC_FilterValue, 40, 10, 0, 0, // Skip to: 4561
/* 4551 */    MCD::OPC_CheckPredicate, 64, 254, 48, 0, // Skip to: 17098
/* 4556 */    MCD::OPC_Decode, 132, 16, 230, 1, // Opcode: MSUB_S
/* 4561 */    MCD::OPC_FilterValue, 41, 10, 0, 0, // Skip to: 4576
/* 4566 */    MCD::OPC_CheckPredicate, 65, 239, 48, 0, // Skip to: 17098
/* 4571 */    MCD::OPC_Decode, 252, 15, 231, 1, // Opcode: MSUB_D32
/* 4576 */    MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 4591
/* 4581 */    MCD::OPC_CheckPredicate, 66, 224, 48, 0, // Skip to: 17098
/* 4586 */    MCD::OPC_Decode, 237, 16, 230, 1, // Opcode: NMADD_S
/* 4591 */    MCD::OPC_FilterValue, 49, 10, 0, 0, // Skip to: 4606
/* 4596 */    MCD::OPC_CheckPredicate, 67, 209, 48, 0, // Skip to: 17098
/* 4601 */    MCD::OPC_Decode, 234, 16, 231, 1, // Opcode: NMADD_D32
/* 4606 */    MCD::OPC_FilterValue, 56, 10, 0, 0, // Skip to: 4621
/* 4611 */    MCD::OPC_CheckPredicate, 66, 194, 48, 0, // Skip to: 17098
/* 4616 */    MCD::OPC_Decode, 242, 16, 230, 1, // Opcode: NMSUB_S
/* 4621 */    MCD::OPC_FilterValue, 57, 184, 48, 0, // Skip to: 17098
/* 4626 */    MCD::OPC_CheckPredicate, 67, 179, 48, 0, // Skip to: 17098
/* 4631 */    MCD::OPC_Decode, 239, 16, 231, 1, // Opcode: NMSUB_D32
/* 4636 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 4651
/* 4641 */    MCD::OPC_CheckPredicate, 39, 164, 48, 0, // Skip to: 17098
/* 4646 */    MCD::OPC_Decode, 161, 6, 190, 1, // Opcode: BEQL
/* 4651 */    MCD::OPC_FilterValue, 21, 107, 0, 0, // Skip to: 4763
/* 4656 */    MCD::OPC_ExtractField, 0, 16,  // Inst{15-0} ...
/* 4659 */    MCD::OPC_FilterValue, 123, 9, 0, 0, // Skip to: 4673
/* 4664 */    MCD::OPC_CheckPredicate, 19, 19, 0, 0, // Skip to: 4688
/* 4669 */    MCD::OPC_Decode, 150, 12, 113, // Opcode: FMOV_S_MM
/* 4673 */    MCD::OPC_FilterValue, 251, 22, 9, 0, 0, // Skip to: 4688
/* 4679 */    MCD::OPC_CheckPredicate, 19, 4, 0, 0, // Skip to: 4688
/* 4684 */    MCD::OPC_Decode, 168, 12, 113, // Opcode: FNEG_S_MM
/* 4688 */    MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 4691 */    MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 4706
/* 4696 */    MCD::OPC_CheckPredicate, 19, 52, 0, 0, // Skip to: 4753
/* 4701 */    MCD::OPC_Decode, 188, 11, 153, 1, // Opcode: FADD_S_MM
/* 4706 */    MCD::OPC_FilterValue, 112, 10, 0, 0, // Skip to: 4721
/* 4711 */    MCD::OPC_CheckPredicate, 19, 37, 0, 0, // Skip to: 4753
/* 4716 */    MCD::OPC_Decode, 203, 12, 153, 1, // Opcode: FSUB_S_MM
/* 4721 */    MCD::OPC_FilterValue, 176, 1, 10, 0, 0, // Skip to: 4737
/* 4727 */    MCD::OPC_CheckPredicate, 19, 21, 0, 0, // Skip to: 4753
/* 4732 */    MCD::OPC_Decode, 160, 12, 153, 1, // Opcode: FMUL_S_MM
/* 4737 */    MCD::OPC_FilterValue, 240, 1, 10, 0, 0, // Skip to: 4753
/* 4743 */    MCD::OPC_CheckPredicate, 19, 5, 0, 0, // Skip to: 4753
/* 4748 */    MCD::OPC_Decode, 226, 11, 153, 1, // Opcode: FDIV_S_MM
/* 4753 */    MCD::OPC_CheckPredicate, 39, 52, 48, 0, // Skip to: 17098
/* 4758 */    MCD::OPC_Decode, 136, 7, 190, 1, // Opcode: BNEL
/* 4763 */    MCD::OPC_FilterValue, 22, 17, 0, 0, // Skip to: 4785
/* 4768 */    MCD::OPC_CheckPredicate, 39, 37, 48, 0, // Skip to: 17098
/* 4773 */    MCD::OPC_CheckField, 16, 5, 0, 30, 48, 0, // Skip to: 17098
/* 4780 */    MCD::OPC_Decode, 226, 6, 186, 1, // Opcode: BLEZL
/* 4785 */    MCD::OPC_FilterValue, 23, 17, 0, 0, // Skip to: 4807
/* 4790 */    MCD::OPC_CheckPredicate, 39, 15, 48, 0, // Skip to: 17098
/* 4795 */    MCD::OPC_CheckField, 16, 5, 0, 8, 48, 0, // Skip to: 17098
/* 4802 */    MCD::OPC_Decode, 197, 6, 186, 1, // Opcode: BGTZL
/* 4807 */    MCD::OPC_FilterValue, 28, 15, 1, 0, // Skip to: 5083
/* 4812 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 4815 */    MCD::OPC_FilterValue, 0, 42, 0, 0, // Skip to: 4862
/* 4820 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4823 */    MCD::OPC_FilterValue, 0, 238, 47, 0, // Skip to: 17098
/* 4828 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4831 */    MCD::OPC_FilterValue, 0, 230, 47, 0, // Skip to: 17098
/* 4836 */    MCD::OPC_CheckPredicate, 68, 11, 0, 0, // Skip to: 4852
/* 4841 */    MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 4852
/* 4848 */    MCD::OPC_Decode, 186, 14, 69, // Opcode: MADD
/* 4852 */    MCD::OPC_CheckPredicate, 37, 209, 47, 0, // Skip to: 17098
/* 4857 */    MCD::OPC_Decode, 204, 14, 232, 1, // Opcode: MADD_DSP
/* 4862 */    MCD::OPC_FilterValue, 1, 42, 0, 0, // Skip to: 4909
/* 4867 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4870 */    MCD::OPC_FilterValue, 0, 191, 47, 0, // Skip to: 17098
/* 4875 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4878 */    MCD::OPC_FilterValue, 0, 183, 47, 0, // Skip to: 17098
/* 4883 */    MCD::OPC_CheckPredicate, 68, 11, 0, 0, // Skip to: 4899
/* 4888 */    MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 4899
/* 4895 */    MCD::OPC_Decode, 193, 14, 69, // Opcode: MADDU
/* 4899 */    MCD::OPC_CheckPredicate, 37, 162, 47, 0, // Skip to: 17098
/* 4904 */    MCD::OPC_Decode, 194, 14, 232, 1, // Opcode: MADDU_DSP
/* 4909 */    MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 4930
/* 4914 */    MCD::OPC_CheckPredicate, 68, 147, 47, 0, // Skip to: 17098
/* 4919 */    MCD::OPC_CheckField, 6, 5, 0, 140, 47, 0, // Skip to: 17098
/* 4926 */    MCD::OPC_Decode, 176, 16, 50, // Opcode: MUL
/* 4930 */    MCD::OPC_FilterValue, 4, 42, 0, 0, // Skip to: 4977
/* 4935 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4938 */    MCD::OPC_FilterValue, 0, 123, 47, 0, // Skip to: 17098
/* 4943 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4946 */    MCD::OPC_FilterValue, 0, 115, 47, 0, // Skip to: 17098
/* 4951 */    MCD::OPC_CheckPredicate, 68, 11, 0, 0, // Skip to: 4967
/* 4956 */    MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 4967
/* 4963 */    MCD::OPC_Decode, 237, 15, 69, // Opcode: MSUB
/* 4967 */    MCD::OPC_CheckPredicate, 37, 94, 47, 0, // Skip to: 17098
/* 4972 */    MCD::OPC_Decode, 255, 15, 232, 1, // Opcode: MSUB_DSP
/* 4977 */    MCD::OPC_FilterValue, 5, 42, 0, 0, // Skip to: 5024
/* 4982 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 4985 */    MCD::OPC_FilterValue, 0, 76, 47, 0, // Skip to: 17098
/* 4990 */    MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 4993 */    MCD::OPC_FilterValue, 0, 68, 47, 0, // Skip to: 17098
/* 4998 */    MCD::OPC_CheckPredicate, 68, 11, 0, 0, // Skip to: 5014
/* 5003 */    MCD::OPC_CheckField, 11, 2, 0, 4, 0, 0, // Skip to: 5014
/* 5010 */    MCD::OPC_Decode, 244, 15, 69, // Opcode: MSUBU
/* 5014 */    MCD::OPC_CheckPredicate, 37, 47, 47, 0, // Skip to: 17098
/* 5019 */    MCD::OPC_Decode, 245, 15, 232, 1, // Opcode: MSUBU_DSP
/* 5024 */    MCD::OPC_FilterValue, 32, 17, 0, 0, // Skip to: 5046
/* 5029 */    MCD::OPC_CheckPredicate, 68, 32, 47, 0, // Skip to: 17098
/* 5034 */    MCD::OPC_CheckField, 6, 5, 0, 25, 47, 0, // Skip to: 17098
/* 5041 */    MCD::OPC_Decode, 134, 8, 233, 1, // Opcode: CLZ
/* 5046 */    MCD::OPC_FilterValue, 33, 17, 0, 0, // Skip to: 5068
/* 5051 */    MCD::OPC_CheckPredicate, 68, 10, 47, 0, // Skip to: 17098
/* 5056 */    MCD::OPC_CheckField, 6, 5, 0, 3, 47, 0, // Skip to: 17098
/* 5063 */    MCD::OPC_Decode, 242, 7, 233, 1, // Opcode: CLO
/* 5068 */    MCD::OPC_FilterValue, 63, 249, 46, 0, // Skip to: 17098
/* 5073 */    MCD::OPC_CheckPredicate, 68, 244, 46, 0, // Skip to: 17098
/* 5078 */    MCD::OPC_Decode, 157, 18, 177, 1, // Opcode: SDBBP
/* 5083 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 5098
/* 5088 */    MCD::OPC_CheckPredicate, 68, 229, 46, 0, // Skip to: 17098
/* 5093 */    MCD::OPC_Decode, 159, 13, 189, 1, // Opcode: JALX
/* 5098 */    MCD::OPC_FilterValue, 30, 201, 32, 0, // Skip to: 13496
/* 5103 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 5106 */    MCD::OPC_FilterValue, 0, 63, 0, 0, // Skip to: 5174
/* 5111 */    MCD::OPC_ExtractField, 24, 2,  // Inst{25-24} ...
/* 5114 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 5129
/* 5119 */    MCD::OPC_CheckPredicate, 30, 198, 46, 0, // Skip to: 17098
/* 5124 */    MCD::OPC_Decode, 201, 5, 234, 1, // Opcode: ANDI_B
/* 5129 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 5144
/* 5134 */    MCD::OPC_CheckPredicate, 30, 183, 46, 0, // Skip to: 17098
/* 5139 */    MCD::OPC_Decode, 130, 17, 234, 1, // Opcode: ORI_B
/* 5144 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 5159
/* 5149 */    MCD::OPC_CheckPredicate, 30, 168, 46, 0, // Skip to: 17098
/* 5154 */    MCD::OPC_Decode, 246, 16, 234, 1, // Opcode: NORI_B
/* 5159 */    MCD::OPC_FilterValue, 3, 158, 46, 0, // Skip to: 17098
/* 5164 */    MCD::OPC_CheckPredicate, 30, 153, 46, 0, // Skip to: 17098
/* 5169 */    MCD::OPC_Decode, 181, 21, 234, 1, // Opcode: XORI_B
/* 5174 */    MCD::OPC_FilterValue, 1, 48, 0, 0, // Skip to: 5227
/* 5179 */    MCD::OPC_ExtractField, 24, 2,  // Inst{25-24} ...
/* 5182 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 5197
/* 5187 */    MCD::OPC_CheckPredicate, 30, 130, 46, 0, // Skip to: 17098
/* 5192 */    MCD::OPC_Decode, 247, 6, 235, 1, // Opcode: BMNZI_B
/* 5197 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 5212
/* 5202 */    MCD::OPC_CheckPredicate, 30, 115, 46, 0, // Skip to: 17098
/* 5207 */    MCD::OPC_Decode, 249, 6, 235, 1, // Opcode: BMZI_B
/* 5212 */    MCD::OPC_FilterValue, 2, 105, 46, 0, // Skip to: 17098
/* 5217 */    MCD::OPC_CheckPredicate, 30, 100, 46, 0, // Skip to: 17098
/* 5222 */    MCD::OPC_Decode, 163, 7, 235, 1, // Opcode: BSELI_B
/* 5227 */    MCD::OPC_FilterValue, 2, 48, 0, 0, // Skip to: 5280
/* 5232 */    MCD::OPC_ExtractField, 24, 2,  // Inst{25-24} ...
/* 5235 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 5250
/* 5240 */    MCD::OPC_CheckPredicate, 30, 77, 46, 0, // Skip to: 17098
/* 5245 */    MCD::OPC_Decode, 209, 18, 234, 1, // Opcode: SHF_B
/* 5250 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 5265
/* 5255 */    MCD::OPC_CheckPredicate, 30, 62, 46, 0, // Skip to: 17098
/* 5260 */    MCD::OPC_Decode, 210, 18, 236, 1, // Opcode: SHF_H
/* 5265 */    MCD::OPC_FilterValue, 2, 52, 46, 0, // Skip to: 17098
/* 5270 */    MCD::OPC_CheckPredicate, 30, 47, 46, 0, // Skip to: 17098
/* 5275 */    MCD::OPC_Decode, 211, 18, 237, 1, // Opcode: SHF_W
/* 5280 */    MCD::OPC_FilterValue, 6, 107, 1, 0, // Skip to: 5648
/* 5285 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 5288 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 5303
/* 5293 */    MCD::OPC_CheckPredicate, 30, 24, 46, 0, // Skip to: 17098
/* 5298 */    MCD::OPC_Decode, 169, 5, 238, 1, // Opcode: ADDVI_B
/* 5303 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 5318
/* 5308 */    MCD::OPC_CheckPredicate, 30, 9, 46, 0, // Skip to: 17098
/* 5313 */    MCD::OPC_Decode, 171, 5, 239, 1, // Opcode: ADDVI_H
/* 5318 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 5333
/* 5323 */    MCD::OPC_CheckPredicate, 30, 250, 45, 0, // Skip to: 17098
/* 5328 */    MCD::OPC_Decode, 172, 5, 240, 1, // Opcode: ADDVI_W
/* 5333 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 5348
/* 5338 */    MCD::OPC_CheckPredicate, 30, 235, 45, 0, // Skip to: 17098
/* 5343 */    MCD::OPC_Decode, 170, 5, 241, 1, // Opcode: ADDVI_D
/* 5348 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 5363
/* 5353 */    MCD::OPC_CheckPredicate, 30, 220, 45, 0, // Skip to: 17098
/* 5358 */    MCD::OPC_Decode, 150, 20, 238, 1, // Opcode: SUBVI_B
/* 5363 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 5378
/* 5368 */    MCD::OPC_CheckPredicate, 30, 205, 45, 0, // Skip to: 17098
/* 5373 */    MCD::OPC_Decode, 152, 20, 239, 1, // Opcode: SUBVI_H
/* 5378 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 5393
/* 5383 */    MCD::OPC_CheckPredicate, 30, 190, 45, 0, // Skip to: 17098
/* 5388 */    MCD::OPC_Decode, 153, 20, 240, 1, // Opcode: SUBVI_W
/* 5393 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 5408
/* 5398 */    MCD::OPC_CheckPredicate, 30, 175, 45, 0, // Skip to: 17098
/* 5403 */    MCD::OPC_Decode, 151, 20, 241, 1, // Opcode: SUBVI_D
/* 5408 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 5423
/* 5413 */    MCD::OPC_CheckPredicate, 30, 160, 45, 0, // Skip to: 17098
/* 5418 */    MCD::OPC_Decode, 223, 14, 238, 1, // Opcode: MAXI_S_B
/* 5423 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 5438
/* 5428 */    MCD::OPC_CheckPredicate, 30, 145, 45, 0, // Skip to: 17098
/* 5433 */    MCD::OPC_Decode, 225, 14, 239, 1, // Opcode: MAXI_S_H
/* 5438 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 5453
/* 5443 */    MCD::OPC_CheckPredicate, 30, 130, 45, 0, // Skip to: 17098
/* 5448 */    MCD::OPC_Decode, 226, 14, 240, 1, // Opcode: MAXI_S_W
/* 5453 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 5468
/* 5458 */    MCD::OPC_CheckPredicate, 30, 115, 45, 0, // Skip to: 17098
/* 5463 */    MCD::OPC_Decode, 224, 14, 241, 1, // Opcode: MAXI_S_D
/* 5468 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 5483
/* 5473 */    MCD::OPC_CheckPredicate, 30, 100, 45, 0, // Skip to: 17098
/* 5478 */    MCD::OPC_Decode, 227, 14, 238, 1, // Opcode: MAXI_U_B
/* 5483 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 5498
/* 5488 */    MCD::OPC_CheckPredicate, 30, 85, 45, 0, // Skip to: 17098
/* 5493 */    MCD::OPC_Decode, 229, 14, 239, 1, // Opcode: MAXI_U_H
/* 5498 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 5513
/* 5503 */    MCD::OPC_CheckPredicate, 30, 70, 45, 0, // Skip to: 17098
/* 5508 */    MCD::OPC_Decode, 230, 14, 240, 1, // Opcode: MAXI_U_W
/* 5513 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 5528
/* 5518 */    MCD::OPC_CheckPredicate, 30, 55, 45, 0, // Skip to: 17098
/* 5523 */    MCD::OPC_Decode, 228, 14, 241, 1, // Opcode: MAXI_U_D
/* 5528 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 5543
/* 5533 */    MCD::OPC_CheckPredicate, 30, 40, 45, 0, // Skip to: 17098
/* 5538 */    MCD::OPC_Decode, 154, 15, 238, 1, // Opcode: MINI_S_B
/* 5543 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 5558
/* 5548 */    MCD::OPC_CheckPredicate, 30, 25, 45, 0, // Skip to: 17098
/* 5553 */    MCD::OPC_Decode, 156, 15, 239, 1, // Opcode: MINI_S_H
/* 5558 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 5573
/* 5563 */    MCD::OPC_CheckPredicate, 30, 10, 45, 0, // Skip to: 17098
/* 5568 */    MCD::OPC_Decode, 157, 15, 240, 1, // Opcode: MINI_S_W
/* 5573 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 5588
/* 5578 */    MCD::OPC_CheckPredicate, 30, 251, 44, 0, // Skip to: 17098
/* 5583 */    MCD::OPC_Decode, 155, 15, 241, 1, // Opcode: MINI_S_D
/* 5588 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 5603
/* 5593 */    MCD::OPC_CheckPredicate, 30, 236, 44, 0, // Skip to: 17098
/* 5598 */    MCD::OPC_Decode, 158, 15, 238, 1, // Opcode: MINI_U_B
/* 5603 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 5618
/* 5608 */    MCD::OPC_CheckPredicate, 30, 221, 44, 0, // Skip to: 17098
/* 5613 */    MCD::OPC_Decode, 160, 15, 239, 1, // Opcode: MINI_U_H
/* 5618 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 5633
/* 5623 */    MCD::OPC_CheckPredicate, 30, 206, 44, 0, // Skip to: 17098
/* 5628 */    MCD::OPC_Decode, 161, 15, 240, 1, // Opcode: MINI_U_W
/* 5633 */    MCD::OPC_FilterValue, 23, 196, 44, 0, // Skip to: 17098
/* 5638 */    MCD::OPC_CheckPredicate, 30, 191, 44, 0, // Skip to: 17098
/* 5643 */    MCD::OPC_Decode, 159, 15, 241, 1, // Opcode: MINI_U_D
/* 5648 */    MCD::OPC_FilterValue, 7, 107, 1, 0, // Skip to: 6016
/* 5653 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 5656 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 5671
/* 5661 */    MCD::OPC_CheckPredicate, 30, 168, 44, 0, // Skip to: 17098
/* 5666 */    MCD::OPC_Decode, 206, 7, 238, 1, // Opcode: CEQI_B
/* 5671 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 5686
/* 5676 */    MCD::OPC_CheckPredicate, 30, 153, 44, 0, // Skip to: 17098
/* 5681 */    MCD::OPC_Decode, 208, 7, 239, 1, // Opcode: CEQI_H
/* 5686 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 5701
/* 5691 */    MCD::OPC_CheckPredicate, 30, 138, 44, 0, // Skip to: 17098
/* 5696 */    MCD::OPC_Decode, 209, 7, 240, 1, // Opcode: CEQI_W
/* 5701 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 5716
/* 5706 */    MCD::OPC_CheckPredicate, 30, 123, 44, 0, // Skip to: 17098
/* 5711 */    MCD::OPC_Decode, 207, 7, 241, 1, // Opcode: CEQI_D
/* 5716 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 5731
/* 5721 */    MCD::OPC_CheckPredicate, 30, 108, 44, 0, // Skip to: 17098
/* 5726 */    MCD::OPC_Decode, 246, 7, 238, 1, // Opcode: CLTI_S_B
/* 5731 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 5746
/* 5736 */    MCD::OPC_CheckPredicate, 30, 93, 44, 0, // Skip to: 17098
/* 5741 */    MCD::OPC_Decode, 248, 7, 239, 1, // Opcode: CLTI_S_H
/* 5746 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 5761
/* 5751 */    MCD::OPC_CheckPredicate, 30, 78, 44, 0, // Skip to: 17098
/* 5756 */    MCD::OPC_Decode, 249, 7, 240, 1, // Opcode: CLTI_S_W
/* 5761 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 5776
/* 5766 */    MCD::OPC_CheckPredicate, 30, 63, 44, 0, // Skip to: 17098
/* 5771 */    MCD::OPC_Decode, 247, 7, 241, 1, // Opcode: CLTI_S_D
/* 5776 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 5791
/* 5781 */    MCD::OPC_CheckPredicate, 30, 48, 44, 0, // Skip to: 17098
/* 5786 */    MCD::OPC_Decode, 250, 7, 238, 1, // Opcode: CLTI_U_B
/* 5791 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 5806
/* 5796 */    MCD::OPC_CheckPredicate, 30, 33, 44, 0, // Skip to: 17098
/* 5801 */    MCD::OPC_Decode, 252, 7, 239, 1, // Opcode: CLTI_U_H
/* 5806 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 5821
/* 5811 */    MCD::OPC_CheckPredicate, 30, 18, 44, 0, // Skip to: 17098
/* 5816 */    MCD::OPC_Decode, 253, 7, 240, 1, // Opcode: CLTI_U_W
/* 5821 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 5836
/* 5826 */    MCD::OPC_CheckPredicate, 30, 3, 44, 0, // Skip to: 17098
/* 5831 */    MCD::OPC_Decode, 251, 7, 241, 1, // Opcode: CLTI_U_D
/* 5836 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 5851
/* 5841 */    MCD::OPC_CheckPredicate, 30, 244, 43, 0, // Skip to: 17098
/* 5846 */    MCD::OPC_Decode, 226, 7, 238, 1, // Opcode: CLEI_S_B
/* 5851 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 5866
/* 5856 */    MCD::OPC_CheckPredicate, 30, 229, 43, 0, // Skip to: 17098
/* 5861 */    MCD::OPC_Decode, 228, 7, 239, 1, // Opcode: CLEI_S_H
/* 5866 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 5881
/* 5871 */    MCD::OPC_CheckPredicate, 30, 214, 43, 0, // Skip to: 17098
/* 5876 */    MCD::OPC_Decode, 229, 7, 240, 1, // Opcode: CLEI_S_W
/* 5881 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 5896
/* 5886 */    MCD::OPC_CheckPredicate, 30, 199, 43, 0, // Skip to: 17098
/* 5891 */    MCD::OPC_Decode, 227, 7, 241, 1, // Opcode: CLEI_S_D
/* 5896 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 5911
/* 5901 */    MCD::OPC_CheckPredicate, 30, 184, 43, 0, // Skip to: 17098
/* 5906 */    MCD::OPC_Decode, 230, 7, 238, 1, // Opcode: CLEI_U_B
/* 5911 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 5926
/* 5916 */    MCD::OPC_CheckPredicate, 30, 169, 43, 0, // Skip to: 17098
/* 5921 */    MCD::OPC_Decode, 232, 7, 239, 1, // Opcode: CLEI_U_H
/* 5926 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 5941
/* 5931 */    MCD::OPC_CheckPredicate, 30, 154, 43, 0, // Skip to: 17098
/* 5936 */    MCD::OPC_Decode, 233, 7, 240, 1, // Opcode: CLEI_U_W
/* 5941 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 5956
/* 5946 */    MCD::OPC_CheckPredicate, 30, 139, 43, 0, // Skip to: 17098
/* 5951 */    MCD::OPC_Decode, 231, 7, 241, 1, // Opcode: CLEI_U_D
/* 5956 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 5971
/* 5961 */    MCD::OPC_CheckPredicate, 30, 124, 43, 0, // Skip to: 17098
/* 5966 */    MCD::OPC_Decode, 211, 13, 242, 1, // Opcode: LDI_B
/* 5971 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 5986
/* 5976 */    MCD::OPC_CheckPredicate, 30, 109, 43, 0, // Skip to: 17098
/* 5981 */    MCD::OPC_Decode, 213, 13, 243, 1, // Opcode: LDI_H
/* 5986 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 6001
/* 5991 */    MCD::OPC_CheckPredicate, 30, 94, 43, 0, // Skip to: 17098
/* 5996 */    MCD::OPC_Decode, 214, 13, 244, 1, // Opcode: LDI_W
/* 6001 */    MCD::OPC_FilterValue, 27, 84, 43, 0, // Skip to: 17098
/* 6006 */    MCD::OPC_CheckPredicate, 30, 79, 43, 0, // Skip to: 17098
/* 6011 */    MCD::OPC_Decode, 212, 13, 245, 1, // Opcode: LDI_D
/* 6016 */    MCD::OPC_FilterValue, 9, 155, 2, 0, // Skip to: 6688
/* 6021 */    MCD::OPC_ExtractField, 22, 4,  // Inst{25-22} ...
/* 6024 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6039
/* 6029 */    MCD::OPC_CheckPredicate, 30, 56, 43, 0, // Skip to: 17098
/* 6034 */    MCD::OPC_Decode, 150, 19, 246, 1, // Opcode: SLLI_D
/* 6039 */    MCD::OPC_FilterValue, 1, 63, 0, 0, // Skip to: 6107
/* 6044 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6047 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6062
/* 6052 */    MCD::OPC_CheckPredicate, 30, 33, 43, 0, // Skip to: 17098
/* 6057 */    MCD::OPC_Decode, 152, 19, 240, 1, // Opcode: SLLI_W
/* 6062 */    MCD::OPC_FilterValue, 1, 23, 43, 0, // Skip to: 17098
/* 6067 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6070 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6085
/* 6075 */    MCD::OPC_CheckPredicate, 30, 10, 43, 0, // Skip to: 17098
/* 6080 */    MCD::OPC_Decode, 151, 19, 247, 1, // Opcode: SLLI_H
/* 6085 */    MCD::OPC_FilterValue, 1, 0, 43, 0, // Skip to: 17098
/* 6090 */    MCD::OPC_CheckPredicate, 30, 251, 42, 0, // Skip to: 17098
/* 6095 */    MCD::OPC_CheckField, 19, 1, 0, 244, 42, 0, // Skip to: 17098
/* 6102 */    MCD::OPC_Decode, 149, 19, 248, 1, // Opcode: SLLI_B
/* 6107 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 6122
/* 6112 */    MCD::OPC_CheckPredicate, 30, 229, 42, 0, // Skip to: 17098
/* 6117 */    MCD::OPC_Decode, 185, 19, 246, 1, // Opcode: SRAI_D
/* 6122 */    MCD::OPC_FilterValue, 3, 63, 0, 0, // Skip to: 6190
/* 6127 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6130 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6145
/* 6135 */    MCD::OPC_CheckPredicate, 30, 206, 42, 0, // Skip to: 17098
/* 6140 */    MCD::OPC_Decode, 187, 19, 240, 1, // Opcode: SRAI_W
/* 6145 */    MCD::OPC_FilterValue, 1, 196, 42, 0, // Skip to: 17098
/* 6150 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6153 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6168
/* 6158 */    MCD::OPC_CheckPredicate, 30, 183, 42, 0, // Skip to: 17098
/* 6163 */    MCD::OPC_Decode, 186, 19, 247, 1, // Opcode: SRAI_H
/* 6168 */    MCD::OPC_FilterValue, 1, 173, 42, 0, // Skip to: 17098
/* 6173 */    MCD::OPC_CheckPredicate, 30, 168, 42, 0, // Skip to: 17098
/* 6178 */    MCD::OPC_CheckField, 19, 1, 0, 161, 42, 0, // Skip to: 17098
/* 6185 */    MCD::OPC_Decode, 184, 19, 248, 1, // Opcode: SRAI_B
/* 6190 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 6205
/* 6195 */    MCD::OPC_CheckPredicate, 30, 146, 42, 0, // Skip to: 17098
/* 6200 */    MCD::OPC_Decode, 207, 19, 246, 1, // Opcode: SRLI_D
/* 6205 */    MCD::OPC_FilterValue, 5, 63, 0, 0, // Skip to: 6273
/* 6210 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6213 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6228
/* 6218 */    MCD::OPC_CheckPredicate, 30, 123, 42, 0, // Skip to: 17098
/* 6223 */    MCD::OPC_Decode, 209, 19, 240, 1, // Opcode: SRLI_W
/* 6228 */    MCD::OPC_FilterValue, 1, 113, 42, 0, // Skip to: 17098
/* 6233 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6236 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6251
/* 6241 */    MCD::OPC_CheckPredicate, 30, 100, 42, 0, // Skip to: 17098
/* 6246 */    MCD::OPC_Decode, 208, 19, 247, 1, // Opcode: SRLI_H
/* 6251 */    MCD::OPC_FilterValue, 1, 90, 42, 0, // Skip to: 17098
/* 6256 */    MCD::OPC_CheckPredicate, 30, 85, 42, 0, // Skip to: 17098
/* 6261 */    MCD::OPC_CheckField, 19, 1, 0, 78, 42, 0, // Skip to: 17098
/* 6268 */    MCD::OPC_Decode, 206, 19, 248, 1, // Opcode: SRLI_B
/* 6273 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 6288
/* 6278 */    MCD::OPC_CheckPredicate, 30, 63, 42, 0, // Skip to: 17098
/* 6283 */    MCD::OPC_Decode, 148, 6, 246, 1, // Opcode: BCLRI_D
/* 6288 */    MCD::OPC_FilterValue, 7, 63, 0, 0, // Skip to: 6356
/* 6293 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6296 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6311
/* 6301 */    MCD::OPC_CheckPredicate, 30, 40, 42, 0, // Skip to: 17098
/* 6306 */    MCD::OPC_Decode, 150, 6, 240, 1, // Opcode: BCLRI_W
/* 6311 */    MCD::OPC_FilterValue, 1, 30, 42, 0, // Skip to: 17098
/* 6316 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6319 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6334
/* 6324 */    MCD::OPC_CheckPredicate, 30, 17, 42, 0, // Skip to: 17098
/* 6329 */    MCD::OPC_Decode, 149, 6, 247, 1, // Opcode: BCLRI_H
/* 6334 */    MCD::OPC_FilterValue, 1, 7, 42, 0, // Skip to: 17098
/* 6339 */    MCD::OPC_CheckPredicate, 30, 2, 42, 0, // Skip to: 17098
/* 6344 */    MCD::OPC_CheckField, 19, 1, 0, 251, 41, 0, // Skip to: 17098
/* 6351 */    MCD::OPC_Decode, 147, 6, 248, 1, // Opcode: BCLRI_B
/* 6356 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 6371
/* 6361 */    MCD::OPC_CheckPredicate, 30, 236, 41, 0, // Skip to: 17098
/* 6366 */    MCD::OPC_Decode, 166, 7, 246, 1, // Opcode: BSETI_D
/* 6371 */    MCD::OPC_FilterValue, 9, 63, 0, 0, // Skip to: 6439
/* 6376 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6379 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6394
/* 6384 */    MCD::OPC_CheckPredicate, 30, 213, 41, 0, // Skip to: 17098
/* 6389 */    MCD::OPC_Decode, 168, 7, 240, 1, // Opcode: BSETI_W
/* 6394 */    MCD::OPC_FilterValue, 1, 203, 41, 0, // Skip to: 17098
/* 6399 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6402 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6417
/* 6407 */    MCD::OPC_CheckPredicate, 30, 190, 41, 0, // Skip to: 17098
/* 6412 */    MCD::OPC_Decode, 167, 7, 247, 1, // Opcode: BSETI_H
/* 6417 */    MCD::OPC_FilterValue, 1, 180, 41, 0, // Skip to: 17098
/* 6422 */    MCD::OPC_CheckPredicate, 30, 175, 41, 0, // Skip to: 17098
/* 6427 */    MCD::OPC_CheckField, 19, 1, 0, 168, 41, 0, // Skip to: 17098
/* 6434 */    MCD::OPC_Decode, 165, 7, 248, 1, // Opcode: BSETI_B
/* 6439 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 6454
/* 6444 */    MCD::OPC_CheckPredicate, 30, 153, 41, 0, // Skip to: 17098
/* 6449 */    MCD::OPC_Decode, 129, 7, 246, 1, // Opcode: BNEGI_D
/* 6454 */    MCD::OPC_FilterValue, 11, 63, 0, 0, // Skip to: 6522
/* 6459 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6462 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6477
/* 6467 */    MCD::OPC_CheckPredicate, 30, 130, 41, 0, // Skip to: 17098
/* 6472 */    MCD::OPC_Decode, 131, 7, 240, 1, // Opcode: BNEGI_W
/* 6477 */    MCD::OPC_FilterValue, 1, 120, 41, 0, // Skip to: 17098
/* 6482 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6485 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6500
/* 6490 */    MCD::OPC_CheckPredicate, 30, 107, 41, 0, // Skip to: 17098
/* 6495 */    MCD::OPC_Decode, 130, 7, 247, 1, // Opcode: BNEGI_H
/* 6500 */    MCD::OPC_FilterValue, 1, 97, 41, 0, // Skip to: 17098
/* 6505 */    MCD::OPC_CheckPredicate, 30, 92, 41, 0, // Skip to: 17098
/* 6510 */    MCD::OPC_CheckField, 19, 1, 0, 85, 41, 0, // Skip to: 17098
/* 6517 */    MCD::OPC_Decode, 128, 7, 248, 1, // Opcode: BNEGI_B
/* 6522 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 6537
/* 6527 */    MCD::OPC_CheckPredicate, 30, 70, 41, 0, // Skip to: 17098
/* 6532 */    MCD::OPC_Decode, 200, 6, 249, 1, // Opcode: BINSLI_D
/* 6537 */    MCD::OPC_FilterValue, 13, 63, 0, 0, // Skip to: 6605
/* 6542 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6545 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6560
/* 6550 */    MCD::OPC_CheckPredicate, 30, 47, 41, 0, // Skip to: 17098
/* 6555 */    MCD::OPC_Decode, 202, 6, 250, 1, // Opcode: BINSLI_W
/* 6560 */    MCD::OPC_FilterValue, 1, 37, 41, 0, // Skip to: 17098
/* 6565 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6568 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6583
/* 6573 */    MCD::OPC_CheckPredicate, 30, 24, 41, 0, // Skip to: 17098
/* 6578 */    MCD::OPC_Decode, 201, 6, 251, 1, // Opcode: BINSLI_H
/* 6583 */    MCD::OPC_FilterValue, 1, 14, 41, 0, // Skip to: 17098
/* 6588 */    MCD::OPC_CheckPredicate, 30, 9, 41, 0, // Skip to: 17098
/* 6593 */    MCD::OPC_CheckField, 19, 1, 0, 2, 41, 0, // Skip to: 17098
/* 6600 */    MCD::OPC_Decode, 199, 6, 252, 1, // Opcode: BINSLI_B
/* 6605 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 6620
/* 6610 */    MCD::OPC_CheckPredicate, 30, 243, 40, 0, // Skip to: 17098
/* 6615 */    MCD::OPC_Decode, 208, 6, 249, 1, // Opcode: BINSRI_D
/* 6620 */    MCD::OPC_FilterValue, 15, 233, 40, 0, // Skip to: 17098
/* 6625 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6628 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6643
/* 6633 */    MCD::OPC_CheckPredicate, 30, 220, 40, 0, // Skip to: 17098
/* 6638 */    MCD::OPC_Decode, 210, 6, 250, 1, // Opcode: BINSRI_W
/* 6643 */    MCD::OPC_FilterValue, 1, 210, 40, 0, // Skip to: 17098
/* 6648 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6651 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6666
/* 6656 */    MCD::OPC_CheckPredicate, 30, 197, 40, 0, // Skip to: 17098
/* 6661 */    MCD::OPC_Decode, 209, 6, 251, 1, // Opcode: BINSRI_H
/* 6666 */    MCD::OPC_FilterValue, 1, 187, 40, 0, // Skip to: 17098
/* 6671 */    MCD::OPC_CheckPredicate, 30, 182, 40, 0, // Skip to: 17098
/* 6676 */    MCD::OPC_CheckField, 19, 1, 0, 175, 40, 0, // Skip to: 17098
/* 6683 */    MCD::OPC_Decode, 207, 6, 252, 1, // Opcode: BINSRI_B
/* 6688 */    MCD::OPC_FilterValue, 10, 79, 1, 0, // Skip to: 7028
/* 6693 */    MCD::OPC_ExtractField, 22, 4,  // Inst{25-22} ...
/* 6696 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6711
/* 6701 */    MCD::OPC_CheckPredicate, 30, 152, 40, 0, // Skip to: 17098
/* 6706 */    MCD::OPC_Decode, 131, 18, 246, 1, // Opcode: SAT_S_D
/* 6711 */    MCD::OPC_FilterValue, 1, 63, 0, 0, // Skip to: 6779
/* 6716 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6719 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6734
/* 6724 */    MCD::OPC_CheckPredicate, 30, 129, 40, 0, // Skip to: 17098
/* 6729 */    MCD::OPC_Decode, 133, 18, 240, 1, // Opcode: SAT_S_W
/* 6734 */    MCD::OPC_FilterValue, 1, 119, 40, 0, // Skip to: 17098
/* 6739 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6742 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6757
/* 6747 */    MCD::OPC_CheckPredicate, 30, 106, 40, 0, // Skip to: 17098
/* 6752 */    MCD::OPC_Decode, 132, 18, 247, 1, // Opcode: SAT_S_H
/* 6757 */    MCD::OPC_FilterValue, 1, 96, 40, 0, // Skip to: 17098
/* 6762 */    MCD::OPC_CheckPredicate, 30, 91, 40, 0, // Skip to: 17098
/* 6767 */    MCD::OPC_CheckField, 19, 1, 0, 84, 40, 0, // Skip to: 17098
/* 6774 */    MCD::OPC_Decode, 130, 18, 248, 1, // Opcode: SAT_S_B
/* 6779 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 6794
/* 6784 */    MCD::OPC_CheckPredicate, 30, 69, 40, 0, // Skip to: 17098
/* 6789 */    MCD::OPC_Decode, 135, 18, 246, 1, // Opcode: SAT_U_D
/* 6794 */    MCD::OPC_FilterValue, 3, 63, 0, 0, // Skip to: 6862
/* 6799 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6802 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6817
/* 6807 */    MCD::OPC_CheckPredicate, 30, 46, 40, 0, // Skip to: 17098
/* 6812 */    MCD::OPC_Decode, 137, 18, 240, 1, // Opcode: SAT_U_W
/* 6817 */    MCD::OPC_FilterValue, 1, 36, 40, 0, // Skip to: 17098
/* 6822 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6825 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6840
/* 6830 */    MCD::OPC_CheckPredicate, 30, 23, 40, 0, // Skip to: 17098
/* 6835 */    MCD::OPC_Decode, 136, 18, 247, 1, // Opcode: SAT_U_H
/* 6840 */    MCD::OPC_FilterValue, 1, 13, 40, 0, // Skip to: 17098
/* 6845 */    MCD::OPC_CheckPredicate, 30, 8, 40, 0, // Skip to: 17098
/* 6850 */    MCD::OPC_CheckField, 19, 1, 0, 1, 40, 0, // Skip to: 17098
/* 6857 */    MCD::OPC_Decode, 134, 18, 248, 1, // Opcode: SAT_U_B
/* 6862 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 6877
/* 6867 */    MCD::OPC_CheckPredicate, 30, 242, 39, 0, // Skip to: 17098
/* 6872 */    MCD::OPC_Decode, 189, 19, 246, 1, // Opcode: SRARI_D
/* 6877 */    MCD::OPC_FilterValue, 5, 63, 0, 0, // Skip to: 6945
/* 6882 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6885 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6900
/* 6890 */    MCD::OPC_CheckPredicate, 30, 219, 39, 0, // Skip to: 17098
/* 6895 */    MCD::OPC_Decode, 191, 19, 240, 1, // Opcode: SRARI_W
/* 6900 */    MCD::OPC_FilterValue, 1, 209, 39, 0, // Skip to: 17098
/* 6905 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6908 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6923
/* 6913 */    MCD::OPC_CheckPredicate, 30, 196, 39, 0, // Skip to: 17098
/* 6918 */    MCD::OPC_Decode, 190, 19, 247, 1, // Opcode: SRARI_H
/* 6923 */    MCD::OPC_FilterValue, 1, 186, 39, 0, // Skip to: 17098
/* 6928 */    MCD::OPC_CheckPredicate, 30, 181, 39, 0, // Skip to: 17098
/* 6933 */    MCD::OPC_CheckField, 19, 1, 0, 174, 39, 0, // Skip to: 17098
/* 6940 */    MCD::OPC_Decode, 188, 19, 248, 1, // Opcode: SRARI_B
/* 6945 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 6960
/* 6950 */    MCD::OPC_CheckPredicate, 30, 159, 39, 0, // Skip to: 17098
/* 6955 */    MCD::OPC_Decode, 211, 19, 246, 1, // Opcode: SRLRI_D
/* 6960 */    MCD::OPC_FilterValue, 7, 149, 39, 0, // Skip to: 17098
/* 6965 */    MCD::OPC_ExtractField, 21, 1,  // Inst{21} ...
/* 6968 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 6983
/* 6973 */    MCD::OPC_CheckPredicate, 30, 136, 39, 0, // Skip to: 17098
/* 6978 */    MCD::OPC_Decode, 213, 19, 240, 1, // Opcode: SRLRI_W
/* 6983 */    MCD::OPC_FilterValue, 1, 126, 39, 0, // Skip to: 17098
/* 6988 */    MCD::OPC_ExtractField, 20, 1,  // Inst{20} ...
/* 6991 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 7006
/* 6996 */    MCD::OPC_CheckPredicate, 30, 113, 39, 0, // Skip to: 17098
/* 7001 */    MCD::OPC_Decode, 212, 19, 247, 1, // Opcode: SRLRI_H
/* 7006 */    MCD::OPC_FilterValue, 1, 103, 39, 0, // Skip to: 17098
/* 7011 */    MCD::OPC_CheckPredicate, 30, 98, 39, 0, // Skip to: 17098
/* 7016 */    MCD::OPC_CheckField, 19, 1, 0, 91, 39, 0, // Skip to: 17098
/* 7023 */    MCD::OPC_Decode, 210, 19, 248, 1, // Opcode: SRLRI_B
/* 7028 */    MCD::OPC_FilterValue, 13, 227, 1, 0, // Skip to: 7516
/* 7033 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 7036 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 7051
/* 7041 */    MCD::OPC_CheckPredicate, 30, 68, 39, 0, // Skip to: 17098
/* 7046 */    MCD::OPC_Decode, 155, 19, 253, 1, // Opcode: SLL_B
/* 7051 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 7066
/* 7056 */    MCD::OPC_CheckPredicate, 30, 53, 39, 0, // Skip to: 17098
/* 7061 */    MCD::OPC_Decode, 157, 19, 254, 1, // Opcode: SLL_H
/* 7066 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 7081
/* 7071 */    MCD::OPC_CheckPredicate, 30, 38, 39, 0, // Skip to: 17098
/* 7076 */    MCD::OPC_Decode, 160, 19, 255, 1, // Opcode: SLL_W
/* 7081 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 7096
/* 7086 */    MCD::OPC_CheckPredicate, 30, 23, 39, 0, // Skip to: 17098
/* 7091 */    MCD::OPC_Decode, 156, 19, 128, 2, // Opcode: SLL_D
/* 7096 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 7111
/* 7101 */    MCD::OPC_CheckPredicate, 30, 8, 39, 0, // Skip to: 17098
/* 7106 */    MCD::OPC_Decode, 198, 19, 253, 1, // Opcode: SRA_B
/* 7111 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 7126
/* 7116 */    MCD::OPC_CheckPredicate, 30, 249, 38, 0, // Skip to: 17098
/* 7121 */    MCD::OPC_Decode, 200, 19, 254, 1, // Opcode: SRA_H
/* 7126 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 7141
/* 7131 */    MCD::OPC_CheckPredicate, 30, 234, 38, 0, // Skip to: 17098
/* 7136 */    MCD::OPC_Decode, 202, 19, 255, 1, // Opcode: SRA_W
/* 7141 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 7156
/* 7146 */    MCD::OPC_CheckPredicate, 30, 219, 38, 0, // Skip to: 17098
/* 7151 */    MCD::OPC_Decode, 199, 19, 128, 2, // Opcode: SRA_D
/* 7156 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 7171
/* 7161 */    MCD::OPC_CheckPredicate, 30, 204, 38, 0, // Skip to: 17098
/* 7166 */    MCD::OPC_Decode, 220, 19, 253, 1, // Opcode: SRL_B
/* 7171 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 7186
/* 7176 */    MCD::OPC_CheckPredicate, 30, 189, 38, 0, // Skip to: 17098
/* 7181 */    MCD::OPC_Decode, 222, 19, 254, 1, // Opcode: SRL_H
/* 7186 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 7201
/* 7191 */    MCD::OPC_CheckPredicate, 30, 174, 38, 0, // Skip to: 17098
/* 7196 */    MCD::OPC_Decode, 224, 19, 255, 1, // Opcode: SRL_W
/* 7201 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 7216
/* 7206 */    MCD::OPC_CheckPredicate, 30, 159, 38, 0, // Skip to: 17098
/* 7211 */    MCD::OPC_Decode, 221, 19, 128, 2, // Opcode: SRL_D
/* 7216 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 7231
/* 7221 */    MCD::OPC_CheckPredicate, 30, 144, 38, 0, // Skip to: 17098
/* 7226 */    MCD::OPC_Decode, 151, 6, 253, 1, // Opcode: BCLR_B
/* 7231 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 7246
/* 7236 */    MCD::OPC_CheckPredicate, 30, 129, 38, 0, // Skip to: 17098
/* 7241 */    MCD::OPC_Decode, 153, 6, 254, 1, // Opcode: BCLR_H
/* 7246 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 7261
/* 7251 */    MCD::OPC_CheckPredicate, 30, 114, 38, 0, // Skip to: 17098
/* 7256 */    MCD::OPC_Decode, 154, 6, 255, 1, // Opcode: BCLR_W
/* 7261 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 7276
/* 7266 */    MCD::OPC_CheckPredicate, 30, 99, 38, 0, // Skip to: 17098
/* 7271 */    MCD::OPC_Decode, 152, 6, 128, 2, // Opcode: BCLR_D
/* 7276 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 7291
/* 7281 */    MCD::OPC_CheckPredicate, 30, 84, 38, 0, // Skip to: 17098
/* 7286 */    MCD::OPC_Decode, 169, 7, 253, 1, // Opcode: BSET_B
/* 7291 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 7306
/* 7296 */    MCD::OPC_CheckPredicate, 30, 69, 38, 0, // Skip to: 17098
/* 7301 */    MCD::OPC_Decode, 171, 7, 254, 1, // Opcode: BSET_H
/* 7306 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 7321
/* 7311 */    MCD::OPC_CheckPredicate, 30, 54, 38, 0, // Skip to: 17098
/* 7316 */    MCD::OPC_Decode, 172, 7, 255, 1, // Opcode: BSET_W
/* 7321 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 7336
/* 7326 */    MCD::OPC_CheckPredicate, 30, 39, 38, 0, // Skip to: 17098
/* 7331 */    MCD::OPC_Decode, 170, 7, 128, 2, // Opcode: BSET_D
/* 7336 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 7351
/* 7341 */    MCD::OPC_CheckPredicate, 30, 24, 38, 0, // Skip to: 17098
/* 7346 */    MCD::OPC_Decode, 132, 7, 253, 1, // Opcode: BNEG_B
/* 7351 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 7366
/* 7356 */    MCD::OPC_CheckPredicate, 30, 9, 38, 0, // Skip to: 17098
/* 7361 */    MCD::OPC_Decode, 134, 7, 254, 1, // Opcode: BNEG_H
/* 7366 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 7381
/* 7371 */    MCD::OPC_CheckPredicate, 30, 250, 37, 0, // Skip to: 17098
/* 7376 */    MCD::OPC_Decode, 135, 7, 255, 1, // Opcode: BNEG_W
/* 7381 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 7396
/* 7386 */    MCD::OPC_CheckPredicate, 30, 235, 37, 0, // Skip to: 17098
/* 7391 */    MCD::OPC_Decode, 133, 7, 128, 2, // Opcode: BNEG_D
/* 7396 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 7411
/* 7401 */    MCD::OPC_CheckPredicate, 30, 220, 37, 0, // Skip to: 17098
/* 7406 */    MCD::OPC_Decode, 203, 6, 129, 2, // Opcode: BINSL_B
/* 7411 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 7426
/* 7416 */    MCD::OPC_CheckPredicate, 30, 205, 37, 0, // Skip to: 17098
/* 7421 */    MCD::OPC_Decode, 205, 6, 130, 2, // Opcode: BINSL_H
/* 7426 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 7441
/* 7431 */    MCD::OPC_CheckPredicate, 30, 190, 37, 0, // Skip to: 17098
/* 7436 */    MCD::OPC_Decode, 206, 6, 131, 2, // Opcode: BINSL_W
/* 7441 */    MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 7456
/* 7446 */    MCD::OPC_CheckPredicate, 30, 175, 37, 0, // Skip to: 17098
/* 7451 */    MCD::OPC_Decode, 204, 6, 132, 2, // Opcode: BINSL_D
/* 7456 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 7471
/* 7461 */    MCD::OPC_CheckPredicate, 30, 160, 37, 0, // Skip to: 17098
/* 7466 */    MCD::OPC_Decode, 211, 6, 129, 2, // Opcode: BINSR_B
/* 7471 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 7486
/* 7476 */    MCD::OPC_CheckPredicate, 30, 145, 37, 0, // Skip to: 17098
/* 7481 */    MCD::OPC_Decode, 213, 6, 130, 2, // Opcode: BINSR_H
/* 7486 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 7501
/* 7491 */    MCD::OPC_CheckPredicate, 30, 130, 37, 0, // Skip to: 17098
/* 7496 */    MCD::OPC_Decode, 214, 6, 131, 2, // Opcode: BINSR_W
/* 7501 */    MCD::OPC_FilterValue, 31, 120, 37, 0, // Skip to: 17098
/* 7506 */    MCD::OPC_CheckPredicate, 30, 115, 37, 0, // Skip to: 17098
/* 7511 */    MCD::OPC_Decode, 212, 6, 132, 2, // Opcode: BINSR_D
/* 7516 */    MCD::OPC_FilterValue, 14, 227, 1, 0, // Skip to: 8004
/* 7521 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 7524 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 7539
/* 7529 */    MCD::OPC_CheckPredicate, 30, 92, 37, 0, // Skip to: 17098
/* 7534 */    MCD::OPC_Decode, 173, 5, 253, 1, // Opcode: ADDV_B
/* 7539 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 7554
/* 7544 */    MCD::OPC_CheckPredicate, 30, 77, 37, 0, // Skip to: 17098
/* 7549 */    MCD::OPC_Decode, 175, 5, 254, 1, // Opcode: ADDV_H
/* 7554 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 7569
/* 7559 */    MCD::OPC_CheckPredicate, 30, 62, 37, 0, // Skip to: 17098
/* 7564 */    MCD::OPC_Decode, 176, 5, 255, 1, // Opcode: ADDV_W
/* 7569 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 7584
/* 7574 */    MCD::OPC_CheckPredicate, 30, 47, 37, 0, // Skip to: 17098
/* 7579 */    MCD::OPC_Decode, 174, 5, 128, 2, // Opcode: ADDV_D
/* 7584 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 7599
/* 7589 */    MCD::OPC_CheckPredicate, 30, 32, 37, 0, // Skip to: 17098
/* 7594 */    MCD::OPC_Decode, 154, 20, 253, 1, // Opcode: SUBV_B
/* 7599 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 7614
/* 7604 */    MCD::OPC_CheckPredicate, 30, 17, 37, 0, // Skip to: 17098
/* 7609 */    MCD::OPC_Decode, 156, 20, 254, 1, // Opcode: SUBV_H
/* 7614 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 7629
/* 7619 */    MCD::OPC_CheckPredicate, 30, 2, 37, 0, // Skip to: 17098
/* 7624 */    MCD::OPC_Decode, 157, 20, 255, 1, // Opcode: SUBV_W
/* 7629 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 7644
/* 7634 */    MCD::OPC_CheckPredicate, 30, 243, 36, 0, // Skip to: 17098
/* 7639 */    MCD::OPC_Decode, 155, 20, 128, 2, // Opcode: SUBV_D
/* 7644 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 7659
/* 7649 */    MCD::OPC_CheckPredicate, 30, 228, 36, 0, // Skip to: 17098
/* 7654 */    MCD::OPC_Decode, 238, 14, 253, 1, // Opcode: MAX_S_B
/* 7659 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 7674
/* 7664 */    MCD::OPC_CheckPredicate, 30, 213, 36, 0, // Skip to: 17098
/* 7669 */    MCD::OPC_Decode, 240, 14, 254, 1, // Opcode: MAX_S_H
/* 7674 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 7689
/* 7679 */    MCD::OPC_CheckPredicate, 30, 198, 36, 0, // Skip to: 17098
/* 7684 */    MCD::OPC_Decode, 242, 14, 255, 1, // Opcode: MAX_S_W
/* 7689 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 7704
/* 7694 */    MCD::OPC_CheckPredicate, 30, 183, 36, 0, // Skip to: 17098
/* 7699 */    MCD::OPC_Decode, 239, 14, 128, 2, // Opcode: MAX_S_D
/* 7704 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 7719
/* 7709 */    MCD::OPC_CheckPredicate, 30, 168, 36, 0, // Skip to: 17098
/* 7714 */    MCD::OPC_Decode, 243, 14, 253, 1, // Opcode: MAX_U_B
/* 7719 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 7734
/* 7724 */    MCD::OPC_CheckPredicate, 30, 153, 36, 0, // Skip to: 17098
/* 7729 */    MCD::OPC_Decode, 245, 14, 254, 1, // Opcode: MAX_U_H
/* 7734 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 7749
/* 7739 */    MCD::OPC_CheckPredicate, 30, 138, 36, 0, // Skip to: 17098
/* 7744 */    MCD::OPC_Decode, 246, 14, 255, 1, // Opcode: MAX_U_W
/* 7749 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 7764
/* 7754 */    MCD::OPC_CheckPredicate, 30, 123, 36, 0, // Skip to: 17098
/* 7759 */    MCD::OPC_Decode, 244, 14, 128, 2, // Opcode: MAX_U_D
/* 7764 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 7779
/* 7769 */    MCD::OPC_CheckPredicate, 30, 108, 36, 0, // Skip to: 17098
/* 7774 */    MCD::OPC_Decode, 169, 15, 253, 1, // Opcode: MIN_S_B
/* 7779 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 7794
/* 7784 */    MCD::OPC_CheckPredicate, 30, 93, 36, 0, // Skip to: 17098
/* 7789 */    MCD::OPC_Decode, 171, 15, 254, 1, // Opcode: MIN_S_H
/* 7794 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 7809
/* 7799 */    MCD::OPC_CheckPredicate, 30, 78, 36, 0, // Skip to: 17098
/* 7804 */    MCD::OPC_Decode, 173, 15, 255, 1, // Opcode: MIN_S_W
/* 7809 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 7824
/* 7814 */    MCD::OPC_CheckPredicate, 30, 63, 36, 0, // Skip to: 17098
/* 7819 */    MCD::OPC_Decode, 170, 15, 128, 2, // Opcode: MIN_S_D
/* 7824 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 7839
/* 7829 */    MCD::OPC_CheckPredicate, 30, 48, 36, 0, // Skip to: 17098
/* 7834 */    MCD::OPC_Decode, 174, 15, 253, 1, // Opcode: MIN_U_B
/* 7839 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 7854
/* 7844 */    MCD::OPC_CheckPredicate, 30, 33, 36, 0, // Skip to: 17098
/* 7849 */    MCD::OPC_Decode, 176, 15, 254, 1, // Opcode: MIN_U_H
/* 7854 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 7869
/* 7859 */    MCD::OPC_CheckPredicate, 30, 18, 36, 0, // Skip to: 17098
/* 7864 */    MCD::OPC_Decode, 177, 15, 255, 1, // Opcode: MIN_U_W
/* 7869 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 7884
/* 7874 */    MCD::OPC_CheckPredicate, 30, 3, 36, 0, // Skip to: 17098
/* 7879 */    MCD::OPC_Decode, 175, 15, 128, 2, // Opcode: MIN_U_D
/* 7884 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 7899
/* 7889 */    MCD::OPC_CheckPredicate, 30, 244, 35, 0, // Skip to: 17098
/* 7894 */    MCD::OPC_Decode, 231, 14, 253, 1, // Opcode: MAX_A_B
/* 7899 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 7914
/* 7904 */    MCD::OPC_CheckPredicate, 30, 229, 35, 0, // Skip to: 17098
/* 7909 */    MCD::OPC_Decode, 233, 14, 254, 1, // Opcode: MAX_A_H
/* 7914 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 7929
/* 7919 */    MCD::OPC_CheckPredicate, 30, 214, 35, 0, // Skip to: 17098
/* 7924 */    MCD::OPC_Decode, 234, 14, 255, 1, // Opcode: MAX_A_W
/* 7929 */    MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 7944
/* 7934 */    MCD::OPC_CheckPredicate, 30, 199, 35, 0, // Skip to: 17098
/* 7939 */    MCD::OPC_Decode, 232, 14, 128, 2, // Opcode: MAX_A_D
/* 7944 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 7959
/* 7949 */    MCD::OPC_CheckPredicate, 30, 184, 35, 0, // Skip to: 17098
/* 7954 */    MCD::OPC_Decode, 162, 15, 253, 1, // Opcode: MIN_A_B
/* 7959 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 7974
/* 7964 */    MCD::OPC_CheckPredicate, 30, 169, 35, 0, // Skip to: 17098
/* 7969 */    MCD::OPC_Decode, 164, 15, 254, 1, // Opcode: MIN_A_H
/* 7974 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 7989
/* 7979 */    MCD::OPC_CheckPredicate, 30, 154, 35, 0, // Skip to: 17098
/* 7984 */    MCD::OPC_Decode, 165, 15, 255, 1, // Opcode: MIN_A_W
/* 7989 */    MCD::OPC_FilterValue, 31, 144, 35, 0, // Skip to: 17098
/* 7994 */    MCD::OPC_CheckPredicate, 30, 139, 35, 0, // Skip to: 17098
/* 7999 */    MCD::OPC_Decode, 163, 15, 128, 2, // Opcode: MIN_A_D
/* 8004 */    MCD::OPC_FilterValue, 15, 47, 1, 0, // Skip to: 8312
/* 8009 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 8012 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 8027
/* 8017 */    MCD::OPC_CheckPredicate, 30, 116, 35, 0, // Skip to: 17098
/* 8022 */    MCD::OPC_Decode, 210, 7, 253, 1, // Opcode: CEQ_B
/* 8027 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 8042
/* 8032 */    MCD::OPC_CheckPredicate, 30, 101, 35, 0, // Skip to: 17098
/* 8037 */    MCD::OPC_Decode, 212, 7, 254, 1, // Opcode: CEQ_H
/* 8042 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 8057
/* 8047 */    MCD::OPC_CheckPredicate, 30, 86, 35, 0, // Skip to: 17098
/* 8052 */    MCD::OPC_Decode, 213, 7, 255, 1, // Opcode: CEQ_W
/* 8057 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 8072
/* 8062 */    MCD::OPC_CheckPredicate, 30, 71, 35, 0, // Skip to: 17098
/* 8067 */    MCD::OPC_Decode, 211, 7, 128, 2, // Opcode: CEQ_D
/* 8072 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 8087
/* 8077 */    MCD::OPC_CheckPredicate, 30, 56, 35, 0, // Skip to: 17098
/* 8082 */    MCD::OPC_Decode, 254, 7, 253, 1, // Opcode: CLT_S_B
/* 8087 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 8102
/* 8092 */    MCD::OPC_CheckPredicate, 30, 41, 35, 0, // Skip to: 17098
/* 8097 */    MCD::OPC_Decode, 128, 8, 254, 1, // Opcode: CLT_S_H
/* 8102 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 8117
/* 8107 */    MCD::OPC_CheckPredicate, 30, 26, 35, 0, // Skip to: 17098
/* 8112 */    MCD::OPC_Decode, 129, 8, 255, 1, // Opcode: CLT_S_W
/* 8117 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 8132
/* 8122 */    MCD::OPC_CheckPredicate, 30, 11, 35, 0, // Skip to: 17098
/* 8127 */    MCD::OPC_Decode, 255, 7, 128, 2, // Opcode: CLT_S_D
/* 8132 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 8147
/* 8137 */    MCD::OPC_CheckPredicate, 30, 252, 34, 0, // Skip to: 17098
/* 8142 */    MCD::OPC_Decode, 130, 8, 253, 1, // Opcode: CLT_U_B
/* 8147 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 8162
/* 8152 */    MCD::OPC_CheckPredicate, 30, 237, 34, 0, // Skip to: 17098
/* 8157 */    MCD::OPC_Decode, 132, 8, 254, 1, // Opcode: CLT_U_H
/* 8162 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 8177
/* 8167 */    MCD::OPC_CheckPredicate, 30, 222, 34, 0, // Skip to: 17098
/* 8172 */    MCD::OPC_Decode, 133, 8, 255, 1, // Opcode: CLT_U_W
/* 8177 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 8192
/* 8182 */    MCD::OPC_CheckPredicate, 30, 207, 34, 0, // Skip to: 17098
/* 8187 */    MCD::OPC_Decode, 131, 8, 128, 2, // Opcode: CLT_U_D
/* 8192 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 8207
/* 8197 */    MCD::OPC_CheckPredicate, 30, 192, 34, 0, // Skip to: 17098
/* 8202 */    MCD::OPC_Decode, 234, 7, 253, 1, // Opcode: CLE_S_B
/* 8207 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 8222
/* 8212 */    MCD::OPC_CheckPredicate, 30, 177, 34, 0, // Skip to: 17098
/* 8217 */    MCD::OPC_Decode, 236, 7, 254, 1, // Opcode: CLE_S_H
/* 8222 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 8237
/* 8227 */    MCD::OPC_CheckPredicate, 30, 162, 34, 0, // Skip to: 17098
/* 8232 */    MCD::OPC_Decode, 237, 7, 255, 1, // Opcode: CLE_S_W
/* 8237 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 8252
/* 8242 */    MCD::OPC_CheckPredicate, 30, 147, 34, 0, // Skip to: 17098
/* 8247 */    MCD::OPC_Decode, 235, 7, 128, 2, // Opcode: CLE_S_D
/* 8252 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 8267
/* 8257 */    MCD::OPC_CheckPredicate, 30, 132, 34, 0, // Skip to: 17098
/* 8262 */    MCD::OPC_Decode, 238, 7, 253, 1, // Opcode: CLE_U_B
/* 8267 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 8282
/* 8272 */    MCD::OPC_CheckPredicate, 30, 117, 34, 0, // Skip to: 17098
/* 8277 */    MCD::OPC_Decode, 240, 7, 254, 1, // Opcode: CLE_U_H
/* 8282 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 8297
/* 8287 */    MCD::OPC_CheckPredicate, 30, 102, 34, 0, // Skip to: 17098
/* 8292 */    MCD::OPC_Decode, 241, 7, 255, 1, // Opcode: CLE_U_W
/* 8297 */    MCD::OPC_FilterValue, 23, 92, 34, 0, // Skip to: 17098
/* 8302 */    MCD::OPC_CheckPredicate, 30, 87, 34, 0, // Skip to: 17098
/* 8307 */    MCD::OPC_Decode, 239, 7, 128, 2, // Opcode: CLE_U_D
/* 8312 */    MCD::OPC_FilterValue, 16, 227, 1, 0, // Skip to: 8800
/* 8317 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 8320 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 8335
/* 8325 */    MCD::OPC_CheckPredicate, 30, 64, 34, 0, // Skip to: 17098
/* 8330 */    MCD::OPC_Decode, 179, 5, 253, 1, // Opcode: ADD_A_B
/* 8335 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 8350
/* 8340 */    MCD::OPC_CheckPredicate, 30, 49, 34, 0, // Skip to: 17098
/* 8345 */    MCD::OPC_Decode, 181, 5, 254, 1, // Opcode: ADD_A_H
/* 8350 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 8365
/* 8355 */    MCD::OPC_CheckPredicate, 30, 34, 34, 0, // Skip to: 17098
/* 8360 */    MCD::OPC_Decode, 182, 5, 255, 1, // Opcode: ADD_A_W
/* 8365 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 8380
/* 8370 */    MCD::OPC_CheckPredicate, 30, 19, 34, 0, // Skip to: 17098
/* 8375 */    MCD::OPC_Decode, 180, 5, 128, 2, // Opcode: ADD_A_D
/* 8380 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 8395
/* 8385 */    MCD::OPC_CheckPredicate, 30, 4, 34, 0, // Skip to: 17098
/* 8390 */    MCD::OPC_Decode, 142, 5, 253, 1, // Opcode: ADDS_A_B
/* 8395 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 8410
/* 8400 */    MCD::OPC_CheckPredicate, 30, 245, 33, 0, // Skip to: 17098
/* 8405 */    MCD::OPC_Decode, 144, 5, 254, 1, // Opcode: ADDS_A_H
/* 8410 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 8425
/* 8415 */    MCD::OPC_CheckPredicate, 30, 230, 33, 0, // Skip to: 17098
/* 8420 */    MCD::OPC_Decode, 145, 5, 255, 1, // Opcode: ADDS_A_W
/* 8425 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 8440
/* 8430 */    MCD::OPC_CheckPredicate, 30, 215, 33, 0, // Skip to: 17098
/* 8435 */    MCD::OPC_Decode, 143, 5, 128, 2, // Opcode: ADDS_A_D
/* 8440 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 8455
/* 8445 */    MCD::OPC_CheckPredicate, 30, 200, 33, 0, // Skip to: 17098
/* 8450 */    MCD::OPC_Decode, 146, 5, 253, 1, // Opcode: ADDS_S_B
/* 8455 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 8470
/* 8460 */    MCD::OPC_CheckPredicate, 30, 185, 33, 0, // Skip to: 17098
/* 8465 */    MCD::OPC_Decode, 148, 5, 254, 1, // Opcode: ADDS_S_H
/* 8470 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 8485
/* 8475 */    MCD::OPC_CheckPredicate, 30, 170, 33, 0, // Skip to: 17098
/* 8480 */    MCD::OPC_Decode, 149, 5, 255, 1, // Opcode: ADDS_S_W
/* 8485 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 8500
/* 8490 */    MCD::OPC_CheckPredicate, 30, 155, 33, 0, // Skip to: 17098
/* 8495 */    MCD::OPC_Decode, 147, 5, 128, 2, // Opcode: ADDS_S_D
/* 8500 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 8515
/* 8505 */    MCD::OPC_CheckPredicate, 30, 140, 33, 0, // Skip to: 17098
/* 8510 */    MCD::OPC_Decode, 150, 5, 253, 1, // Opcode: ADDS_U_B
/* 8515 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 8530
/* 8520 */    MCD::OPC_CheckPredicate, 30, 125, 33, 0, // Skip to: 17098
/* 8525 */    MCD::OPC_Decode, 152, 5, 254, 1, // Opcode: ADDS_U_H
/* 8530 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 8545
/* 8535 */    MCD::OPC_CheckPredicate, 30, 110, 33, 0, // Skip to: 17098
/* 8540 */    MCD::OPC_Decode, 153, 5, 255, 1, // Opcode: ADDS_U_W
/* 8545 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 8560
/* 8550 */    MCD::OPC_CheckPredicate, 30, 95, 33, 0, // Skip to: 17098
/* 8555 */    MCD::OPC_Decode, 151, 5, 128, 2, // Opcode: ADDS_U_D
/* 8560 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 8575
/* 8565 */    MCD::OPC_CheckPredicate, 30, 80, 33, 0, // Skip to: 17098
/* 8570 */    MCD::OPC_Decode, 231, 5, 253, 1, // Opcode: AVE_S_B
/* 8575 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 8590
/* 8580 */    MCD::OPC_CheckPredicate, 30, 65, 33, 0, // Skip to: 17098
/* 8585 */    MCD::OPC_Decode, 233, 5, 254, 1, // Opcode: AVE_S_H
/* 8590 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 8605
/* 8595 */    MCD::OPC_CheckPredicate, 30, 50, 33, 0, // Skip to: 17098
/* 8600 */    MCD::OPC_Decode, 234, 5, 255, 1, // Opcode: AVE_S_W
/* 8605 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 8620
/* 8610 */    MCD::OPC_CheckPredicate, 30, 35, 33, 0, // Skip to: 17098
/* 8615 */    MCD::OPC_Decode, 232, 5, 128, 2, // Opcode: AVE_S_D
/* 8620 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 8635
/* 8625 */    MCD::OPC_CheckPredicate, 30, 20, 33, 0, // Skip to: 17098
/* 8630 */    MCD::OPC_Decode, 235, 5, 253, 1, // Opcode: AVE_U_B
/* 8635 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 8650
/* 8640 */    MCD::OPC_CheckPredicate, 30, 5, 33, 0, // Skip to: 17098
/* 8645 */    MCD::OPC_Decode, 237, 5, 254, 1, // Opcode: AVE_U_H
/* 8650 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 8665
/* 8655 */    MCD::OPC_CheckPredicate, 30, 246, 32, 0, // Skip to: 17098
/* 8660 */    MCD::OPC_Decode, 238, 5, 255, 1, // Opcode: AVE_U_W
/* 8665 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 8680
/* 8670 */    MCD::OPC_CheckPredicate, 30, 231, 32, 0, // Skip to: 17098
/* 8675 */    MCD::OPC_Decode, 236, 5, 128, 2, // Opcode: AVE_U_D
/* 8680 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 8695
/* 8685 */    MCD::OPC_CheckPredicate, 30, 216, 32, 0, // Skip to: 17098
/* 8690 */    MCD::OPC_Decode, 223, 5, 253, 1, // Opcode: AVER_S_B
/* 8695 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 8710
/* 8700 */    MCD::OPC_CheckPredicate, 30, 201, 32, 0, // Skip to: 17098
/* 8705 */    MCD::OPC_Decode, 225, 5, 254, 1, // Opcode: AVER_S_H
/* 8710 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 8725
/* 8715 */    MCD::OPC_CheckPredicate, 30, 186, 32, 0, // Skip to: 17098
/* 8720 */    MCD::OPC_Decode, 226, 5, 255, 1, // Opcode: AVER_S_W
/* 8725 */    MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 8740
/* 8730 */    MCD::OPC_CheckPredicate, 30, 171, 32, 0, // Skip to: 17098
/* 8735 */    MCD::OPC_Decode, 224, 5, 128, 2, // Opcode: AVER_S_D
/* 8740 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 8755
/* 8745 */    MCD::OPC_CheckPredicate, 30, 156, 32, 0, // Skip to: 17098
/* 8750 */    MCD::OPC_Decode, 227, 5, 253, 1, // Opcode: AVER_U_B
/* 8755 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 8770
/* 8760 */    MCD::OPC_CheckPredicate, 30, 141, 32, 0, // Skip to: 17098
/* 8765 */    MCD::OPC_Decode, 229, 5, 254, 1, // Opcode: AVER_U_H
/* 8770 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 8785
/* 8775 */    MCD::OPC_CheckPredicate, 30, 126, 32, 0, // Skip to: 17098
/* 8780 */    MCD::OPC_Decode, 230, 5, 255, 1, // Opcode: AVER_U_W
/* 8785 */    MCD::OPC_FilterValue, 31, 116, 32, 0, // Skip to: 17098
/* 8790 */    MCD::OPC_CheckPredicate, 30, 111, 32, 0, // Skip to: 17098
/* 8795 */    MCD::OPC_Decode, 228, 5, 128, 2, // Opcode: AVER_U_D
/* 8800 */    MCD::OPC_FilterValue, 17, 107, 1, 0, // Skip to: 9168
/* 8805 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 8808 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 8823
/* 8813 */    MCD::OPC_CheckPredicate, 30, 88, 32, 0, // Skip to: 17098
/* 8818 */    MCD::OPC_Decode, 255, 19, 253, 1, // Opcode: SUBS_S_B
/* 8823 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 8838
/* 8828 */    MCD::OPC_CheckPredicate, 30, 73, 32, 0, // Skip to: 17098
/* 8833 */    MCD::OPC_Decode, 129, 20, 254, 1, // Opcode: SUBS_S_H
/* 8838 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 8853
/* 8843 */    MCD::OPC_CheckPredicate, 30, 58, 32, 0, // Skip to: 17098
/* 8848 */    MCD::OPC_Decode, 130, 20, 255, 1, // Opcode: SUBS_S_W
/* 8853 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 8868
/* 8858 */    MCD::OPC_CheckPredicate, 30, 43, 32, 0, // Skip to: 17098
/* 8863 */    MCD::OPC_Decode, 128, 20, 128, 2, // Opcode: SUBS_S_D
/* 8868 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 8883
/* 8873 */    MCD::OPC_CheckPredicate, 30, 28, 32, 0, // Skip to: 17098
/* 8878 */    MCD::OPC_Decode, 131, 20, 253, 1, // Opcode: SUBS_U_B
/* 8883 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 8898
/* 8888 */    MCD::OPC_CheckPredicate, 30, 13, 32, 0, // Skip to: 17098
/* 8893 */    MCD::OPC_Decode, 133, 20, 254, 1, // Opcode: SUBS_U_H
/* 8898 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 8913
/* 8903 */    MCD::OPC_CheckPredicate, 30, 254, 31, 0, // Skip to: 17098
/* 8908 */    MCD::OPC_Decode, 134, 20, 255, 1, // Opcode: SUBS_U_W
/* 8913 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 8928
/* 8918 */    MCD::OPC_CheckPredicate, 30, 239, 31, 0, // Skip to: 17098
/* 8923 */    MCD::OPC_Decode, 132, 20, 128, 2, // Opcode: SUBS_U_D
/* 8928 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 8943
/* 8933 */    MCD::OPC_CheckPredicate, 30, 224, 31, 0, // Skip to: 17098
/* 8938 */    MCD::OPC_Decode, 247, 19, 253, 1, // Opcode: SUBSUS_U_B
/* 8943 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 8958
/* 8948 */    MCD::OPC_CheckPredicate, 30, 209, 31, 0, // Skip to: 17098
/* 8953 */    MCD::OPC_Decode, 249, 19, 254, 1, // Opcode: SUBSUS_U_H
/* 8958 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 8973
/* 8963 */    MCD::OPC_CheckPredicate, 30, 194, 31, 0, // Skip to: 17098
/* 8968 */    MCD::OPC_Decode, 250, 19, 255, 1, // Opcode: SUBSUS_U_W
/* 8973 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 8988
/* 8978 */    MCD::OPC_CheckPredicate, 30, 179, 31, 0, // Skip to: 17098
/* 8983 */    MCD::OPC_Decode, 248, 19, 128, 2, // Opcode: SUBSUS_U_D
/* 8988 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 9003
/* 8993 */    MCD::OPC_CheckPredicate, 30, 164, 31, 0, // Skip to: 17098
/* 8998 */    MCD::OPC_Decode, 251, 19, 253, 1, // Opcode: SUBSUU_S_B
/* 9003 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 9018
/* 9008 */    MCD::OPC_CheckPredicate, 30, 149, 31, 0, // Skip to: 17098
/* 9013 */    MCD::OPC_Decode, 253, 19, 254, 1, // Opcode: SUBSUU_S_H
/* 9018 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 9033
/* 9023 */    MCD::OPC_CheckPredicate, 30, 134, 31, 0, // Skip to: 17098
/* 9028 */    MCD::OPC_Decode, 254, 19, 255, 1, // Opcode: SUBSUU_S_W
/* 9033 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 9048
/* 9038 */    MCD::OPC_CheckPredicate, 30, 119, 31, 0, // Skip to: 17098
/* 9043 */    MCD::OPC_Decode, 252, 19, 128, 2, // Opcode: SUBSUU_S_D
/* 9048 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 9063
/* 9053 */    MCD::OPC_CheckPredicate, 30, 104, 31, 0, // Skip to: 17098
/* 9058 */    MCD::OPC_Decode, 211, 5, 253, 1, // Opcode: ASUB_S_B
/* 9063 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 9078
/* 9068 */    MCD::OPC_CheckPredicate, 30, 89, 31, 0, // Skip to: 17098
/* 9073 */    MCD::OPC_Decode, 213, 5, 254, 1, // Opcode: ASUB_S_H
/* 9078 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 9093
/* 9083 */    MCD::OPC_CheckPredicate, 30, 74, 31, 0, // Skip to: 17098
/* 9088 */    MCD::OPC_Decode, 214, 5, 255, 1, // Opcode: ASUB_S_W
/* 9093 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 9108
/* 9098 */    MCD::OPC_CheckPredicate, 30, 59, 31, 0, // Skip to: 17098
/* 9103 */    MCD::OPC_Decode, 212, 5, 128, 2, // Opcode: ASUB_S_D
/* 9108 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 9123
/* 9113 */    MCD::OPC_CheckPredicate, 30, 44, 31, 0, // Skip to: 17098
/* 9118 */    MCD::OPC_Decode, 215, 5, 253, 1, // Opcode: ASUB_U_B
/* 9123 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 9138
/* 9128 */    MCD::OPC_CheckPredicate, 30, 29, 31, 0, // Skip to: 17098
/* 9133 */    MCD::OPC_Decode, 217, 5, 254, 1, // Opcode: ASUB_U_H
/* 9138 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 9153
/* 9143 */    MCD::OPC_CheckPredicate, 30, 14, 31, 0, // Skip to: 17098
/* 9148 */    MCD::OPC_Decode, 218, 5, 255, 1, // Opcode: ASUB_U_W
/* 9153 */    MCD::OPC_FilterValue, 23, 4, 31, 0, // Skip to: 17098
/* 9158 */    MCD::OPC_CheckPredicate, 30, 255, 30, 0, // Skip to: 17098
/* 9163 */    MCD::OPC_Decode, 216, 5, 128, 2, // Opcode: ASUB_U_D
/* 9168 */    MCD::OPC_FilterValue, 18, 167, 1, 0, // Skip to: 9596
/* 9173 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 9176 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 9191
/* 9181 */    MCD::OPC_CheckPredicate, 30, 232, 30, 0, // Skip to: 17098
/* 9186 */    MCD::OPC_Decode, 209, 16, 253, 1, // Opcode: MULV_B
/* 9191 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 9206
/* 9196 */    MCD::OPC_CheckPredicate, 30, 217, 30, 0, // Skip to: 17098
/* 9201 */    MCD::OPC_Decode, 211, 16, 254, 1, // Opcode: MULV_H
/* 9206 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 9221
/* 9211 */    MCD::OPC_CheckPredicate, 30, 202, 30, 0, // Skip to: 17098
/* 9216 */    MCD::OPC_Decode, 212, 16, 255, 1, // Opcode: MULV_W
/* 9221 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 9236
/* 9226 */    MCD::OPC_CheckPredicate, 30, 187, 30, 0, // Skip to: 17098
/* 9231 */    MCD::OPC_Decode, 210, 16, 128, 2, // Opcode: MULV_D
/* 9236 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 9251
/* 9241 */    MCD::OPC_CheckPredicate, 30, 172, 30, 0, // Skip to: 17098
/* 9246 */    MCD::OPC_Decode, 197, 14, 129, 2, // Opcode: MADDV_B
/* 9251 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 9266
/* 9256 */    MCD::OPC_CheckPredicate, 30, 157, 30, 0, // Skip to: 17098
/* 9261 */    MCD::OPC_Decode, 199, 14, 130, 2, // Opcode: MADDV_H
/* 9266 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 9281
/* 9271 */    MCD::OPC_CheckPredicate, 30, 142, 30, 0, // Skip to: 17098
/* 9276 */    MCD::OPC_Decode, 200, 14, 131, 2, // Opcode: MADDV_W
/* 9281 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 9296
/* 9286 */    MCD::OPC_CheckPredicate, 30, 127, 30, 0, // Skip to: 17098
/* 9291 */    MCD::OPC_Decode, 198, 14, 132, 2, // Opcode: MADDV_D
/* 9296 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 9311
/* 9301 */    MCD::OPC_CheckPredicate, 30, 112, 30, 0, // Skip to: 17098
/* 9306 */    MCD::OPC_Decode, 248, 15, 129, 2, // Opcode: MSUBV_B
/* 9311 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 9326
/* 9316 */    MCD::OPC_CheckPredicate, 30, 97, 30, 0, // Skip to: 17098
/* 9321 */    MCD::OPC_Decode, 250, 15, 130, 2, // Opcode: MSUBV_H
/* 9326 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 9341
/* 9331 */    MCD::OPC_CheckPredicate, 30, 82, 30, 0, // Skip to: 17098
/* 9336 */    MCD::OPC_Decode, 251, 15, 131, 2, // Opcode: MSUBV_W
/* 9341 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 9356
/* 9346 */    MCD::OPC_CheckPredicate, 30, 67, 30, 0, // Skip to: 17098
/* 9351 */    MCD::OPC_Decode, 249, 15, 132, 2, // Opcode: MSUBV_D
/* 9356 */    MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 9371
/* 9361 */    MCD::OPC_CheckPredicate, 30, 52, 30, 0, // Skip to: 17098
/* 9366 */    MCD::OPC_Decode, 153, 10, 253, 1, // Opcode: DIV_S_B
/* 9371 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 9386
/* 9376 */    MCD::OPC_CheckPredicate, 30, 37, 30, 0, // Skip to: 17098
/* 9381 */    MCD::OPC_Decode, 155, 10, 254, 1, // Opcode: DIV_S_H
/* 9386 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 9401
/* 9391 */    MCD::OPC_CheckPredicate, 30, 22, 30, 0, // Skip to: 17098
/* 9396 */    MCD::OPC_Decode, 156, 10, 255, 1, // Opcode: DIV_S_W
/* 9401 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 9416
/* 9406 */    MCD::OPC_CheckPredicate, 30, 7, 30, 0, // Skip to: 17098
/* 9411 */    MCD::OPC_Decode, 154, 10, 128, 2, // Opcode: DIV_S_D
/* 9416 */    MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 9431
/* 9421 */    MCD::OPC_CheckPredicate, 30, 248, 29, 0, // Skip to: 17098
/* 9426 */    MCD::OPC_Decode, 157, 10, 253, 1, // Opcode: DIV_U_B
/* 9431 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 9446
/* 9436 */    MCD::OPC_CheckPredicate, 30, 233, 29, 0, // Skip to: 17098
/* 9441 */    MCD::OPC_Decode, 159, 10, 254, 1, // Opcode: DIV_U_H
/* 9446 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 9461
/* 9451 */    MCD::OPC_CheckPredicate, 30, 218, 29, 0, // Skip to: 17098
/* 9456 */    MCD::OPC_Decode, 160, 10, 255, 1, // Opcode: DIV_U_W
/* 9461 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 9476
/* 9466 */    MCD::OPC_CheckPredicate, 30, 203, 29, 0, // Skip to: 17098
/* 9471 */    MCD::OPC_Decode, 158, 10, 128, 2, // Opcode: DIV_U_D
/* 9476 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 9491
/* 9481 */    MCD::OPC_CheckPredicate, 30, 188, 29, 0, // Skip to: 17098
/* 9486 */    MCD::OPC_Decode, 184, 15, 253, 1, // Opcode: MOD_S_B
/* 9491 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 9506
/* 9496 */    MCD::OPC_CheckPredicate, 30, 173, 29, 0, // Skip to: 17098
/* 9501 */    MCD::OPC_Decode, 186, 15, 254, 1, // Opcode: MOD_S_H
/* 9506 */    MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 9521
/* 9511 */    MCD::OPC_CheckPredicate, 30, 158, 29, 0, // Skip to: 17098
/* 9516 */    MCD::OPC_Decode, 187, 15, 255, 1, // Opcode: MOD_S_W
/* 9521 */    MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 9536
/* 9526 */    MCD::OPC_CheckPredicate, 30, 143, 29, 0, // Skip to: 17098
/* 9531 */    MCD::OPC_Decode, 185, 15, 128, 2, // Opcode: MOD_S_D
/* 9536 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 9551
/* 9541 */    MCD::OPC_CheckPredicate, 30, 128, 29, 0, // Skip to: 17098
/* 9546 */    MCD::OPC_Decode, 188, 15, 253, 1, // Opcode: MOD_U_B
/* 9551 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 9566
/* 9556 */    MCD::OPC_CheckPredicate, 30, 113, 29, 0, // Skip to: 17098
/* 9561 */    MCD::OPC_Decode, 190, 15, 254, 1, // Opcode: MOD_U_H
/* 9566 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 9581
/* 9571 */    MCD::OPC_CheckPredicate, 30, 98, 29, 0, // Skip to: 17098
/* 9576 */    MCD::OPC_Decode, 191, 15, 255, 1, // Opcode: MOD_U_W
/* 9581 */    MCD::OPC_FilterValue, 31, 88, 29, 0, // Skip to: 17098
/* 9586 */    MCD::OPC_CheckPredicate, 30, 83, 29, 0, // Skip to: 17098
/* 9591 */    MCD::OPC_Decode, 189, 15, 128, 2, // Opcode: MOD_U_D
/* 9596 */    MCD::OPC_FilterValue, 19, 17, 1, 0, // Skip to: 9874
/* 9601 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 9604 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 9619
/* 9609 */    MCD::OPC_CheckPredicate, 30, 60, 29, 0, // Skip to: 17098
/* 9614 */    MCD::OPC_Decode, 186, 10, 133, 2, // Opcode: DOTP_S_H
/* 9619 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 9634
/* 9624 */    MCD::OPC_CheckPredicate, 30, 45, 29, 0, // Skip to: 17098
/* 9629 */    MCD::OPC_Decode, 187, 10, 134, 2, // Opcode: DOTP_S_W
/* 9634 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 9649
/* 9639 */    MCD::OPC_CheckPredicate, 30, 30, 29, 0, // Skip to: 17098
/* 9644 */    MCD::OPC_Decode, 185, 10, 135, 2, // Opcode: DOTP_S_D
/* 9649 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 9664
/* 9654 */    MCD::OPC_CheckPredicate, 30, 15, 29, 0, // Skip to: 17098
/* 9659 */    MCD::OPC_Decode, 189, 10, 133, 2, // Opcode: DOTP_U_H
/* 9664 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 9679
/* 9669 */    MCD::OPC_CheckPredicate, 30, 0, 29, 0, // Skip to: 17098
/* 9674 */    MCD::OPC_Decode, 190, 10, 134, 2, // Opcode: DOTP_U_W
/* 9679 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 9694
/* 9684 */    MCD::OPC_CheckPredicate, 30, 241, 28, 0, // Skip to: 17098
/* 9689 */    MCD::OPC_Decode, 188, 10, 135, 2, // Opcode: DOTP_U_D
/* 9694 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 9709
/* 9699 */    MCD::OPC_CheckPredicate, 30, 226, 28, 0, // Skip to: 17098
/* 9704 */    MCD::OPC_Decode, 192, 10, 136, 2, // Opcode: DPADD_S_H
/* 9709 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 9724
/* 9714 */    MCD::OPC_CheckPredicate, 30, 211, 28, 0, // Skip to: 17098
/* 9719 */    MCD::OPC_Decode, 193, 10, 137, 2, // Opcode: DPADD_S_W
/* 9724 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 9739
/* 9729 */    MCD::OPC_CheckPredicate, 30, 196, 28, 0, // Skip to: 17098
/* 9734 */    MCD::OPC_Decode, 191, 10, 138, 2, // Opcode: DPADD_S_D
/* 9739 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 9754
/* 9744 */    MCD::OPC_CheckPredicate, 30, 181, 28, 0, // Skip to: 17098
/* 9749 */    MCD::OPC_Decode, 195, 10, 136, 2, // Opcode: DPADD_U_H
/* 9754 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 9769
/* 9759 */    MCD::OPC_CheckPredicate, 30, 166, 28, 0, // Skip to: 17098
/* 9764 */    MCD::OPC_Decode, 196, 10, 137, 2, // Opcode: DPADD_U_W
/* 9769 */    MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 9784
/* 9774 */    MCD::OPC_CheckPredicate, 30, 151, 28, 0, // Skip to: 17098
/* 9779 */    MCD::OPC_Decode, 194, 10, 138, 2, // Opcode: DPADD_U_D
/* 9784 */    MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 9799
/* 9789 */    MCD::OPC_CheckPredicate, 30, 136, 28, 0, // Skip to: 17098
/* 9794 */    MCD::OPC_Decode, 223, 10, 136, 2, // Opcode: DPSUB_S_H
/* 9799 */    MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 9814
/* 9804 */    MCD::OPC_CheckPredicate, 30, 121, 28, 0, // Skip to: 17098
/* 9809 */    MCD::OPC_Decode, 224, 10, 137, 2, // Opcode: DPSUB_S_W
/* 9814 */    MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 9829
/* 9819 */    MCD::OPC_CheckPredicate, 30, 106, 28, 0, // Skip to: 17098
/* 9824 */    MCD::OPC_Decode, 222, 10, 138, 2, // Opcode: DPSUB_S_D
/* 9829 */    MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 9844
/* 9834 */    MCD::OPC_CheckPredicate, 30, 91, 28, 0, // Skip to: 17098
/* 9839 */    MCD::OPC_Decode, 226, 10, 136, 2, // Opcode: DPSUB_U_H
/* 9844 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 9859
/* 9849 */    MCD::OPC_CheckPredicate, 30, 76, 28, 0, // Skip to: 17098
/* 9854 */    MCD::OPC_Decode, 227, 10, 137, 2, // Opcode: DPSUB_U_W
/* 9859 */    MCD::OPC_FilterValue, 23, 66, 28, 0, // Skip to: 17098
/* 9864 */    MCD::OPC_CheckPredicate, 30, 61, 28, 0, // Skip to: 17098
/* 9869 */    MCD::OPC_Decode, 225, 10, 138, 2, // Opcode: DPSUB_U_D
/* 9874 */    MCD::OPC_FilterValue, 20, 227, 1, 0, // Skip to: 10362
/* 9879 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 9882 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 9897
/* 9887 */    MCD::OPC_CheckPredicate, 30, 38, 28, 0, // Skip to: 17098
/* 9892 */    MCD::OPC_Decode, 140, 19, 139, 2, // Opcode: SLD_B
/* 9897 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 9912
/* 9902 */    MCD::OPC_CheckPredicate, 30, 23, 28, 0, // Skip to: 17098
/* 9907 */    MCD::OPC_Decode, 142, 19, 140, 2, // Opcode: SLD_H
/* 9912 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 9927
/* 9917 */    MCD::OPC_CheckPredicate, 30, 8, 28, 0, // Skip to: 17098
/* 9922 */    MCD::OPC_Decode, 143, 19, 141, 2, // Opcode: SLD_W
/* 9927 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 9942
/* 9932 */    MCD::OPC_CheckPredicate, 30, 249, 27, 0, // Skip to: 17098
/* 9937 */    MCD::OPC_Decode, 141, 19, 142, 2, // Opcode: SLD_D
/* 9942 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 9957
/* 9947 */    MCD::OPC_CheckPredicate, 30, 234, 27, 0, // Skip to: 17098
/* 9952 */    MCD::OPC_Decode, 179, 19, 143, 2, // Opcode: SPLAT_B
/* 9957 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 9972
/* 9962 */    MCD::OPC_CheckPredicate, 30, 219, 27, 0, // Skip to: 17098
/* 9967 */    MCD::OPC_Decode, 181, 19, 144, 2, // Opcode: SPLAT_H
/* 9972 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 9987
/* 9977 */    MCD::OPC_CheckPredicate, 30, 204, 27, 0, // Skip to: 17098
/* 9982 */    MCD::OPC_Decode, 182, 19, 145, 2, // Opcode: SPLAT_W
/* 9987 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 10002
/* 9992 */    MCD::OPC_CheckPredicate, 30, 189, 27, 0, // Skip to: 17098
/* 9997 */    MCD::OPC_Decode, 180, 19, 146, 2, // Opcode: SPLAT_D
/* 10002 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 10017
/* 10007 */   MCD::OPC_CheckPredicate, 30, 174, 27, 0, // Skip to: 17098
/* 10012 */   MCD::OPC_Decode, 144, 17, 253, 1, // Opcode: PCKEV_B
/* 10017 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 10032
/* 10022 */   MCD::OPC_CheckPredicate, 30, 159, 27, 0, // Skip to: 17098
/* 10027 */   MCD::OPC_Decode, 146, 17, 254, 1, // Opcode: PCKEV_H
/* 10032 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 10047
/* 10037 */   MCD::OPC_CheckPredicate, 30, 144, 27, 0, // Skip to: 17098
/* 10042 */   MCD::OPC_Decode, 147, 17, 255, 1, // Opcode: PCKEV_W
/* 10047 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 10062
/* 10052 */   MCD::OPC_CheckPredicate, 30, 129, 27, 0, // Skip to: 17098
/* 10057 */   MCD::OPC_Decode, 145, 17, 128, 2, // Opcode: PCKEV_D
/* 10062 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 10077
/* 10067 */   MCD::OPC_CheckPredicate, 30, 114, 27, 0, // Skip to: 17098
/* 10072 */   MCD::OPC_Decode, 148, 17, 253, 1, // Opcode: PCKOD_B
/* 10077 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 10092
/* 10082 */   MCD::OPC_CheckPredicate, 30, 99, 27, 0, // Skip to: 17098
/* 10087 */   MCD::OPC_Decode, 150, 17, 254, 1, // Opcode: PCKOD_H
/* 10092 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 10107
/* 10097 */   MCD::OPC_CheckPredicate, 30, 84, 27, 0, // Skip to: 17098
/* 10102 */   MCD::OPC_Decode, 151, 17, 255, 1, // Opcode: PCKOD_W
/* 10107 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 10122
/* 10112 */   MCD::OPC_CheckPredicate, 30, 69, 27, 0, // Skip to: 17098
/* 10117 */   MCD::OPC_Decode, 149, 17, 128, 2, // Opcode: PCKOD_D
/* 10122 */   MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 10137
/* 10127 */   MCD::OPC_CheckPredicate, 30, 54, 27, 0, // Skip to: 17098
/* 10132 */   MCD::OPC_Decode, 248, 12, 253, 1, // Opcode: ILVL_B
/* 10137 */   MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 10152
/* 10142 */   MCD::OPC_CheckPredicate, 30, 39, 27, 0, // Skip to: 17098
/* 10147 */   MCD::OPC_Decode, 250, 12, 254, 1, // Opcode: ILVL_H
/* 10152 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 10167
/* 10157 */   MCD::OPC_CheckPredicate, 30, 24, 27, 0, // Skip to: 17098
/* 10162 */   MCD::OPC_Decode, 251, 12, 255, 1, // Opcode: ILVL_W
/* 10167 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 10182
/* 10172 */   MCD::OPC_CheckPredicate, 30, 9, 27, 0, // Skip to: 17098
/* 10177 */   MCD::OPC_Decode, 249, 12, 128, 2, // Opcode: ILVL_D
/* 10182 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 10197
/* 10187 */   MCD::OPC_CheckPredicate, 30, 250, 26, 0, // Skip to: 17098
/* 10192 */   MCD::OPC_Decode, 128, 13, 253, 1, // Opcode: ILVR_B
/* 10197 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 10212
/* 10202 */   MCD::OPC_CheckPredicate, 30, 235, 26, 0, // Skip to: 17098
/* 10207 */   MCD::OPC_Decode, 130, 13, 254, 1, // Opcode: ILVR_H
/* 10212 */   MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 10227
/* 10217 */   MCD::OPC_CheckPredicate, 30, 220, 26, 0, // Skip to: 17098
/* 10222 */   MCD::OPC_Decode, 131, 13, 255, 1, // Opcode: ILVR_W
/* 10227 */   MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 10242
/* 10232 */   MCD::OPC_CheckPredicate, 30, 205, 26, 0, // Skip to: 17098
/* 10237 */   MCD::OPC_Decode, 129, 13, 128, 2, // Opcode: ILVR_D
/* 10242 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 10257
/* 10247 */   MCD::OPC_CheckPredicate, 30, 190, 26, 0, // Skip to: 17098
/* 10252 */   MCD::OPC_Decode, 244, 12, 253, 1, // Opcode: ILVEV_B
/* 10257 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 10272
/* 10262 */   MCD::OPC_CheckPredicate, 30, 175, 26, 0, // Skip to: 17098
/* 10267 */   MCD::OPC_Decode, 246, 12, 254, 1, // Opcode: ILVEV_H
/* 10272 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 10287
/* 10277 */   MCD::OPC_CheckPredicate, 30, 160, 26, 0, // Skip to: 17098
/* 10282 */   MCD::OPC_Decode, 247, 12, 255, 1, // Opcode: ILVEV_W
/* 10287 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 10302
/* 10292 */   MCD::OPC_CheckPredicate, 30, 145, 26, 0, // Skip to: 17098
/* 10297 */   MCD::OPC_Decode, 245, 12, 128, 2, // Opcode: ILVEV_D
/* 10302 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 10317
/* 10307 */   MCD::OPC_CheckPredicate, 30, 130, 26, 0, // Skip to: 17098
/* 10312 */   MCD::OPC_Decode, 252, 12, 253, 1, // Opcode: ILVOD_B
/* 10317 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 10332
/* 10322 */   MCD::OPC_CheckPredicate, 30, 115, 26, 0, // Skip to: 17098
/* 10327 */   MCD::OPC_Decode, 254, 12, 254, 1, // Opcode: ILVOD_H
/* 10332 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 10347
/* 10337 */   MCD::OPC_CheckPredicate, 30, 100, 26, 0, // Skip to: 17098
/* 10342 */   MCD::OPC_Decode, 255, 12, 255, 1, // Opcode: ILVOD_W
/* 10347 */   MCD::OPC_FilterValue, 31, 90, 26, 0, // Skip to: 17098
/* 10352 */   MCD::OPC_CheckPredicate, 30, 85, 26, 0, // Skip to: 17098
/* 10357 */   MCD::OPC_Decode, 253, 12, 128, 2, // Opcode: ILVOD_D
/* 10362 */   MCD::OPC_FilterValue, 21, 107, 1, 0, // Skip to: 10730
/* 10367 */   MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 10370 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 10385
/* 10375 */   MCD::OPC_CheckPredicate, 30, 62, 26, 0, // Skip to: 17098
/* 10380 */   MCD::OPC_Decode, 164, 21, 129, 2, // Opcode: VSHF_B
/* 10385 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 10400
/* 10390 */   MCD::OPC_CheckPredicate, 30, 47, 26, 0, // Skip to: 17098
/* 10395 */   MCD::OPC_Decode, 166, 21, 130, 2, // Opcode: VSHF_H
/* 10400 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 10415
/* 10405 */   MCD::OPC_CheckPredicate, 30, 32, 26, 0, // Skip to: 17098
/* 10410 */   MCD::OPC_Decode, 167, 21, 131, 2, // Opcode: VSHF_W
/* 10415 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 10430
/* 10420 */   MCD::OPC_CheckPredicate, 30, 17, 26, 0, // Skip to: 17098
/* 10425 */   MCD::OPC_Decode, 165, 21, 132, 2, // Opcode: VSHF_D
/* 10430 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 10445
/* 10435 */   MCD::OPC_CheckPredicate, 30, 2, 26, 0, // Skip to: 17098
/* 10440 */   MCD::OPC_Decode, 192, 19, 253, 1, // Opcode: SRAR_B
/* 10445 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 10460
/* 10450 */   MCD::OPC_CheckPredicate, 30, 243, 25, 0, // Skip to: 17098
/* 10455 */   MCD::OPC_Decode, 194, 19, 254, 1, // Opcode: SRAR_H
/* 10460 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 10475
/* 10465 */   MCD::OPC_CheckPredicate, 30, 228, 25, 0, // Skip to: 17098
/* 10470 */   MCD::OPC_Decode, 195, 19, 255, 1, // Opcode: SRAR_W
/* 10475 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 10490
/* 10480 */   MCD::OPC_CheckPredicate, 30, 213, 25, 0, // Skip to: 17098
/* 10485 */   MCD::OPC_Decode, 193, 19, 128, 2, // Opcode: SRAR_D
/* 10490 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 10505
/* 10495 */   MCD::OPC_CheckPredicate, 30, 198, 25, 0, // Skip to: 17098
/* 10500 */   MCD::OPC_Decode, 214, 19, 253, 1, // Opcode: SRLR_B
/* 10505 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 10520
/* 10510 */   MCD::OPC_CheckPredicate, 30, 183, 25, 0, // Skip to: 17098
/* 10515 */   MCD::OPC_Decode, 216, 19, 254, 1, // Opcode: SRLR_H
/* 10520 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 10535
/* 10525 */   MCD::OPC_CheckPredicate, 30, 168, 25, 0, // Skip to: 17098
/* 10530 */   MCD::OPC_Decode, 217, 19, 255, 1, // Opcode: SRLR_W
/* 10535 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 10550
/* 10540 */   MCD::OPC_CheckPredicate, 30, 153, 25, 0, // Skip to: 17098
/* 10545 */   MCD::OPC_Decode, 215, 19, 128, 2, // Opcode: SRLR_D
/* 10550 */   MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 10565
/* 10555 */   MCD::OPC_CheckPredicate, 30, 138, 25, 0, // Skip to: 17098
/* 10560 */   MCD::OPC_Decode, 231, 12, 133, 2, // Opcode: HADD_S_H
/* 10565 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 10580
/* 10570 */   MCD::OPC_CheckPredicate, 30, 123, 25, 0, // Skip to: 17098
/* 10575 */   MCD::OPC_Decode, 232, 12, 134, 2, // Opcode: HADD_S_W
/* 10580 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 10595
/* 10585 */   MCD::OPC_CheckPredicate, 30, 108, 25, 0, // Skip to: 17098
/* 10590 */   MCD::OPC_Decode, 230, 12, 135, 2, // Opcode: HADD_S_D
/* 10595 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 10610
/* 10600 */   MCD::OPC_CheckPredicate, 30, 93, 25, 0, // Skip to: 17098
/* 10605 */   MCD::OPC_Decode, 234, 12, 133, 2, // Opcode: HADD_U_H
/* 10610 */   MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 10625
/* 10615 */   MCD::OPC_CheckPredicate, 30, 78, 25, 0, // Skip to: 17098
/* 10620 */   MCD::OPC_Decode, 235, 12, 134, 2, // Opcode: HADD_U_W
/* 10625 */   MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 10640
/* 10630 */   MCD::OPC_CheckPredicate, 30, 63, 25, 0, // Skip to: 17098
/* 10635 */   MCD::OPC_Decode, 233, 12, 135, 2, // Opcode: HADD_U_D
/* 10640 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 10655
/* 10645 */   MCD::OPC_CheckPredicate, 30, 48, 25, 0, // Skip to: 17098
/* 10650 */   MCD::OPC_Decode, 237, 12, 133, 2, // Opcode: HSUB_S_H
/* 10655 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 10670
/* 10660 */   MCD::OPC_CheckPredicate, 30, 33, 25, 0, // Skip to: 17098
/* 10665 */   MCD::OPC_Decode, 238, 12, 134, 2, // Opcode: HSUB_S_W
/* 10670 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 10685
/* 10675 */   MCD::OPC_CheckPredicate, 30, 18, 25, 0, // Skip to: 17098
/* 10680 */   MCD::OPC_Decode, 236, 12, 135, 2, // Opcode: HSUB_S_D
/* 10685 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 10700
/* 10690 */   MCD::OPC_CheckPredicate, 30, 3, 25, 0, // Skip to: 17098
/* 10695 */   MCD::OPC_Decode, 240, 12, 133, 2, // Opcode: HSUB_U_H
/* 10700 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 10715
/* 10705 */   MCD::OPC_CheckPredicate, 30, 244, 24, 0, // Skip to: 17098
/* 10710 */   MCD::OPC_Decode, 241, 12, 134, 2, // Opcode: HSUB_U_W
/* 10715 */   MCD::OPC_FilterValue, 31, 234, 24, 0, // Skip to: 17098
/* 10720 */   MCD::OPC_CheckPredicate, 30, 229, 24, 0, // Skip to: 17098
/* 10725 */   MCD::OPC_Decode, 239, 12, 135, 2, // Opcode: HSUB_U_D
/* 10730 */   MCD::OPC_FilterValue, 25, 26, 2, 0, // Skip to: 11273
/* 10735 */   MCD::OPC_ExtractField, 20, 6,  // Inst{25-20} ...
/* 10738 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 10753
/* 10743 */   MCD::OPC_CheckPredicate, 30, 206, 24, 0, // Skip to: 17098
/* 10748 */   MCD::OPC_Decode, 136, 19, 147, 2, // Opcode: SLDI_B
/* 10753 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 10775
/* 10758 */   MCD::OPC_CheckPredicate, 30, 191, 24, 0, // Skip to: 17098
/* 10763 */   MCD::OPC_CheckField, 19, 1, 0, 184, 24, 0, // Skip to: 17098
/* 10770 */   MCD::OPC_Decode, 138, 19, 148, 2, // Opcode: SLDI_H
/* 10775 */   MCD::OPC_FilterValue, 3, 62, 0, 0, // Skip to: 10842
/* 10780 */   MCD::OPC_ExtractField, 18, 2,  // Inst{19-18} ...
/* 10783 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 10798
/* 10788 */   MCD::OPC_CheckPredicate, 30, 161, 24, 0, // Skip to: 17098
/* 10793 */   MCD::OPC_Decode, 139, 19, 149, 2, // Opcode: SLDI_W
/* 10798 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 10820
/* 10803 */   MCD::OPC_CheckPredicate, 30, 146, 24, 0, // Skip to: 17098
/* 10808 */   MCD::OPC_CheckField, 17, 1, 0, 139, 24, 0, // Skip to: 17098
/* 10815 */   MCD::OPC_Decode, 137, 19, 150, 2, // Opcode: SLDI_D
/* 10820 */   MCD::OPC_FilterValue, 3, 129, 24, 0, // Skip to: 17098
/* 10825 */   MCD::OPC_CheckPredicate, 30, 124, 24, 0, // Skip to: 17098
/* 10830 */   MCD::OPC_CheckField, 16, 2, 2, 117, 24, 0, // Skip to: 17098
/* 10837 */   MCD::OPC_Decode, 244, 8, 151, 2, // Opcode: CTCMSA
/* 10842 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 10857
/* 10847 */   MCD::OPC_CheckPredicate, 30, 102, 24, 0, // Skip to: 17098
/* 10852 */   MCD::OPC_Decode, 175, 19, 152, 2, // Opcode: SPLATI_B
/* 10857 */   MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 10879
/* 10862 */   MCD::OPC_CheckPredicate, 30, 87, 24, 0, // Skip to: 17098
/* 10867 */   MCD::OPC_CheckField, 19, 1, 0, 80, 24, 0, // Skip to: 17098
/* 10874 */   MCD::OPC_Decode, 177, 19, 153, 2, // Opcode: SPLATI_H
/* 10879 */   MCD::OPC_FilterValue, 7, 62, 0, 0, // Skip to: 10946
/* 10884 */   MCD::OPC_ExtractField, 18, 2,  // Inst{19-18} ...
/* 10887 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 10902
/* 10892 */   MCD::OPC_CheckPredicate, 30, 57, 24, 0, // Skip to: 17098
/* 10897 */   MCD::OPC_Decode, 178, 19, 154, 2, // Opcode: SPLATI_W
/* 10902 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 10924
/* 10907 */   MCD::OPC_CheckPredicate, 30, 42, 24, 0, // Skip to: 17098
/* 10912 */   MCD::OPC_CheckField, 17, 1, 0, 35, 24, 0, // Skip to: 17098
/* 10919 */   MCD::OPC_Decode, 176, 19, 155, 2, // Opcode: SPLATI_D
/* 10924 */   MCD::OPC_FilterValue, 3, 25, 24, 0, // Skip to: 17098
/* 10929 */   MCD::OPC_CheckPredicate, 30, 20, 24, 0, // Skip to: 17098
/* 10934 */   MCD::OPC_CheckField, 16, 2, 2, 13, 24, 0, // Skip to: 17098
/* 10941 */   MCD::OPC_Decode, 217, 7, 156, 2, // Opcode: CFCMSA
/* 10946 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 10961
/* 10951 */   MCD::OPC_CheckPredicate, 30, 254, 23, 0, // Skip to: 17098
/* 10956 */   MCD::OPC_Decode, 226, 8, 157, 2, // Opcode: COPY_S_B
/* 10961 */   MCD::OPC_FilterValue, 10, 17, 0, 0, // Skip to: 10983
/* 10966 */   MCD::OPC_CheckPredicate, 30, 239, 23, 0, // Skip to: 17098
/* 10971 */   MCD::OPC_CheckField, 19, 1, 0, 232, 23, 0, // Skip to: 17098
/* 10978 */   MCD::OPC_Decode, 228, 8, 158, 2, // Opcode: COPY_S_H
/* 10983 */   MCD::OPC_FilterValue, 11, 62, 0, 0, // Skip to: 11050
/* 10988 */   MCD::OPC_ExtractField, 18, 2,  // Inst{19-18} ...
/* 10991 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 11006
/* 10996 */   MCD::OPC_CheckPredicate, 30, 209, 23, 0, // Skip to: 17098
/* 11001 */   MCD::OPC_Decode, 229, 8, 159, 2, // Opcode: COPY_S_W
/* 11006 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 11028
/* 11011 */   MCD::OPC_CheckPredicate, 38, 194, 23, 0, // Skip to: 17098
/* 11016 */   MCD::OPC_CheckField, 17, 1, 0, 187, 23, 0, // Skip to: 17098
/* 11023 */   MCD::OPC_Decode, 227, 8, 160, 2, // Opcode: COPY_S_D
/* 11028 */   MCD::OPC_FilterValue, 3, 177, 23, 0, // Skip to: 17098
/* 11033 */   MCD::OPC_CheckPredicate, 30, 172, 23, 0, // Skip to: 17098
/* 11038 */   MCD::OPC_CheckField, 16, 2, 2, 165, 23, 0, // Skip to: 17098
/* 11045 */   MCD::OPC_Decode, 196, 15, 161, 2, // Opcode: MOVE_V
/* 11050 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 11065
/* 11055 */   MCD::OPC_CheckPredicate, 30, 150, 23, 0, // Skip to: 17098
/* 11060 */   MCD::OPC_Decode, 230, 8, 157, 2, // Opcode: COPY_U_B
/* 11065 */   MCD::OPC_FilterValue, 14, 17, 0, 0, // Skip to: 11087
/* 11070 */   MCD::OPC_CheckPredicate, 30, 135, 23, 0, // Skip to: 17098
/* 11075 */   MCD::OPC_CheckField, 19, 1, 0, 128, 23, 0, // Skip to: 17098
/* 11082 */   MCD::OPC_Decode, 231, 8, 158, 2, // Opcode: COPY_U_H
/* 11087 */   MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 11109
/* 11092 */   MCD::OPC_CheckPredicate, 38, 113, 23, 0, // Skip to: 17098
/* 11097 */   MCD::OPC_CheckField, 18, 2, 0, 106, 23, 0, // Skip to: 17098
/* 11104 */   MCD::OPC_Decode, 232, 8, 159, 2, // Opcode: COPY_U_W
/* 11109 */   MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 11124
/* 11114 */   MCD::OPC_CheckPredicate, 30, 91, 23, 0, // Skip to: 17098
/* 11119 */   MCD::OPC_Decode, 133, 13, 162, 2, // Opcode: INSERT_B
/* 11124 */   MCD::OPC_FilterValue, 18, 17, 0, 0, // Skip to: 11146
/* 11129 */   MCD::OPC_CheckPredicate, 30, 76, 23, 0, // Skip to: 17098
/* 11134 */   MCD::OPC_CheckField, 19, 1, 0, 69, 23, 0, // Skip to: 17098
/* 11141 */   MCD::OPC_Decode, 135, 13, 163, 2, // Opcode: INSERT_H
/* 11146 */   MCD::OPC_FilterValue, 19, 40, 0, 0, // Skip to: 11191
/* 11151 */   MCD::OPC_ExtractField, 18, 2,  // Inst{19-18} ...
/* 11154 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 11169
/* 11159 */   MCD::OPC_CheckPredicate, 30, 46, 23, 0, // Skip to: 17098
/* 11164 */   MCD::OPC_Decode, 136, 13, 164, 2, // Opcode: INSERT_W
/* 11169 */   MCD::OPC_FilterValue, 2, 36, 23, 0, // Skip to: 17098
/* 11174 */   MCD::OPC_CheckPredicate, 38, 31, 23, 0, // Skip to: 17098
/* 11179 */   MCD::OPC_CheckField, 17, 1, 0, 24, 23, 0, // Skip to: 17098
/* 11186 */   MCD::OPC_Decode, 134, 13, 165, 2, // Opcode: INSERT_D
/* 11191 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 11206
/* 11196 */   MCD::OPC_CheckPredicate, 30, 9, 23, 0, // Skip to: 17098
/* 11201 */   MCD::OPC_Decode, 138, 13, 166, 2, // Opcode: INSVE_B
/* 11206 */   MCD::OPC_FilterValue, 22, 17, 0, 0, // Skip to: 11228
/* 11211 */   MCD::OPC_CheckPredicate, 30, 250, 22, 0, // Skip to: 17098
/* 11216 */   MCD::OPC_CheckField, 19, 1, 0, 243, 22, 0, // Skip to: 17098
/* 11223 */   MCD::OPC_Decode, 140, 13, 166, 2, // Opcode: INSVE_H
/* 11228 */   MCD::OPC_FilterValue, 23, 233, 22, 0, // Skip to: 17098
/* 11233 */   MCD::OPC_ExtractField, 18, 2,  // Inst{19-18} ...
/* 11236 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 11251
/* 11241 */   MCD::OPC_CheckPredicate, 30, 220, 22, 0, // Skip to: 17098
/* 11246 */   MCD::OPC_Decode, 141, 13, 166, 2, // Opcode: INSVE_W
/* 11251 */   MCD::OPC_FilterValue, 2, 210, 22, 0, // Skip to: 17098
/* 11256 */   MCD::OPC_CheckPredicate, 30, 205, 22, 0, // Skip to: 17098
/* 11261 */   MCD::OPC_CheckField, 17, 1, 0, 198, 22, 0, // Skip to: 17098
/* 11268 */   MCD::OPC_Decode, 139, 13, 166, 2, // Opcode: INSVE_D
/* 11273 */   MCD::OPC_FilterValue, 26, 227, 1, 0, // Skip to: 11761
/* 11278 */   MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 11281 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 11296
/* 11286 */   MCD::OPC_CheckPredicate, 30, 175, 22, 0, // Skip to: 17098
/* 11291 */   MCD::OPC_Decode, 192, 11, 255, 1, // Opcode: FCAF_W
/* 11296 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 11311
/* 11301 */   MCD::OPC_CheckPredicate, 30, 160, 22, 0, // Skip to: 17098
/* 11306 */   MCD::OPC_Decode, 191, 11, 128, 2, // Opcode: FCAF_D
/* 11311 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 11326
/* 11316 */   MCD::OPC_CheckPredicate, 30, 145, 22, 0, // Skip to: 17098
/* 11321 */   MCD::OPC_Decode, 219, 11, 255, 1, // Opcode: FCUN_W
/* 11326 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 11341
/* 11331 */   MCD::OPC_CheckPredicate, 30, 130, 22, 0, // Skip to: 17098
/* 11336 */   MCD::OPC_Decode, 218, 11, 128, 2, // Opcode: FCUN_D
/* 11341 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 11356
/* 11346 */   MCD::OPC_CheckPredicate, 30, 115, 22, 0, // Skip to: 17098
/* 11351 */   MCD::OPC_Decode, 194, 11, 255, 1, // Opcode: FCEQ_W
/* 11356 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 11371
/* 11361 */   MCD::OPC_CheckPredicate, 30, 100, 22, 0, // Skip to: 17098
/* 11366 */   MCD::OPC_Decode, 193, 11, 128, 2, // Opcode: FCEQ_D
/* 11371 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 11386
/* 11376 */   MCD::OPC_CheckPredicate, 30, 85, 22, 0, // Skip to: 17098
/* 11381 */   MCD::OPC_Decode, 211, 11, 255, 1, // Opcode: FCUEQ_W
/* 11386 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 11401
/* 11391 */   MCD::OPC_CheckPredicate, 30, 70, 22, 0, // Skip to: 17098
/* 11396 */   MCD::OPC_Decode, 210, 11, 128, 2, // Opcode: FCUEQ_D
/* 11401 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 11416
/* 11406 */   MCD::OPC_CheckPredicate, 30, 55, 22, 0, // Skip to: 17098
/* 11411 */   MCD::OPC_Decode, 200, 11, 255, 1, // Opcode: FCLT_W
/* 11416 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 11431
/* 11421 */   MCD::OPC_CheckPredicate, 30, 40, 22, 0, // Skip to: 17098
/* 11426 */   MCD::OPC_Decode, 199, 11, 128, 2, // Opcode: FCLT_D
/* 11431 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 11446
/* 11436 */   MCD::OPC_CheckPredicate, 30, 25, 22, 0, // Skip to: 17098
/* 11441 */   MCD::OPC_Decode, 215, 11, 255, 1, // Opcode: FCULT_W
/* 11446 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 11461
/* 11451 */   MCD::OPC_CheckPredicate, 30, 10, 22, 0, // Skip to: 17098
/* 11456 */   MCD::OPC_Decode, 214, 11, 128, 2, // Opcode: FCULT_D
/* 11461 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 11476
/* 11466 */   MCD::OPC_CheckPredicate, 30, 251, 21, 0, // Skip to: 17098
/* 11471 */   MCD::OPC_Decode, 198, 11, 255, 1, // Opcode: FCLE_W
/* 11476 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 11491
/* 11481 */   MCD::OPC_CheckPredicate, 30, 236, 21, 0, // Skip to: 17098
/* 11486 */   MCD::OPC_Decode, 197, 11, 128, 2, // Opcode: FCLE_D
/* 11491 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 11506
/* 11496 */   MCD::OPC_CheckPredicate, 30, 221, 21, 0, // Skip to: 17098
/* 11501 */   MCD::OPC_Decode, 213, 11, 255, 1, // Opcode: FCULE_W
/* 11506 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 11521
/* 11511 */   MCD::OPC_CheckPredicate, 30, 206, 21, 0, // Skip to: 17098
/* 11516 */   MCD::OPC_Decode, 212, 11, 128, 2, // Opcode: FCULE_D
/* 11521 */   MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 11536
/* 11526 */   MCD::OPC_CheckPredicate, 30, 191, 21, 0, // Skip to: 17098
/* 11531 */   MCD::OPC_Decode, 178, 12, 255, 1, // Opcode: FSAF_W
/* 11536 */   MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 11551
/* 11541 */   MCD::OPC_CheckPredicate, 30, 176, 21, 0, // Skip to: 17098
/* 11546 */   MCD::OPC_Decode, 177, 12, 128, 2, // Opcode: FSAF_D
/* 11551 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 11566
/* 11556 */   MCD::OPC_CheckPredicate, 30, 161, 21, 0, // Skip to: 17098
/* 11561 */   MCD::OPC_Decode, 215, 12, 255, 1, // Opcode: FSUN_W
/* 11566 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 11581
/* 11571 */   MCD::OPC_CheckPredicate, 30, 146, 21, 0, // Skip to: 17098
/* 11576 */   MCD::OPC_Decode, 214, 12, 128, 2, // Opcode: FSUN_D
/* 11581 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 11596
/* 11586 */   MCD::OPC_CheckPredicate, 30, 131, 21, 0, // Skip to: 17098
/* 11591 */   MCD::OPC_Decode, 180, 12, 255, 1, // Opcode: FSEQ_W
/* 11596 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 11611
/* 11601 */   MCD::OPC_CheckPredicate, 30, 116, 21, 0, // Skip to: 17098
/* 11606 */   MCD::OPC_Decode, 179, 12, 128, 2, // Opcode: FSEQ_D
/* 11611 */   MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 11626
/* 11616 */   MCD::OPC_CheckPredicate, 30, 101, 21, 0, // Skip to: 17098
/* 11621 */   MCD::OPC_Decode, 207, 12, 255, 1, // Opcode: FSUEQ_W
/* 11626 */   MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 11641
/* 11631 */   MCD::OPC_CheckPredicate, 30, 86, 21, 0, // Skip to: 17098
/* 11636 */   MCD::OPC_Decode, 206, 12, 128, 2, // Opcode: FSUEQ_D
/* 11641 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 11656
/* 11646 */   MCD::OPC_CheckPredicate, 30, 71, 21, 0, // Skip to: 17098
/* 11651 */   MCD::OPC_Decode, 184, 12, 255, 1, // Opcode: FSLT_W
/* 11656 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 11671
/* 11661 */   MCD::OPC_CheckPredicate, 30, 56, 21, 0, // Skip to: 17098
/* 11666 */   MCD::OPC_Decode, 183, 12, 128, 2, // Opcode: FSLT_D
/* 11671 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 11686
/* 11676 */   MCD::OPC_CheckPredicate, 30, 41, 21, 0, // Skip to: 17098
/* 11681 */   MCD::OPC_Decode, 211, 12, 255, 1, // Opcode: FSULT_W
/* 11686 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 11701
/* 11691 */   MCD::OPC_CheckPredicate, 30, 26, 21, 0, // Skip to: 17098
/* 11696 */   MCD::OPC_Decode, 210, 12, 128, 2, // Opcode: FSULT_D
/* 11701 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 11716
/* 11706 */   MCD::OPC_CheckPredicate, 30, 11, 21, 0, // Skip to: 17098
/* 11711 */   MCD::OPC_Decode, 182, 12, 255, 1, // Opcode: FSLE_W
/* 11716 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 11731
/* 11721 */   MCD::OPC_CheckPredicate, 30, 252, 20, 0, // Skip to: 17098
/* 11726 */   MCD::OPC_Decode, 181, 12, 128, 2, // Opcode: FSLE_D
/* 11731 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 11746
/* 11736 */   MCD::OPC_CheckPredicate, 30, 237, 20, 0, // Skip to: 17098
/* 11741 */   MCD::OPC_Decode, 209, 12, 255, 1, // Opcode: FSULE_W
/* 11746 */   MCD::OPC_FilterValue, 31, 227, 20, 0, // Skip to: 17098
/* 11751 */   MCD::OPC_CheckPredicate, 30, 222, 20, 0, // Skip to: 17098
/* 11756 */   MCD::OPC_Decode, 208, 12, 128, 2, // Opcode: FSULE_D
/* 11761 */   MCD::OPC_FilterValue, 27, 137, 1, 0, // Skip to: 12159
/* 11766 */   MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 11769 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 11784
/* 11774 */   MCD::OPC_CheckPredicate, 30, 199, 20, 0, // Skip to: 17098
/* 11779 */   MCD::OPC_Decode, 190, 11, 255, 1, // Opcode: FADD_W
/* 11784 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 11799
/* 11789 */   MCD::OPC_CheckPredicate, 30, 184, 20, 0, // Skip to: 17098
/* 11794 */   MCD::OPC_Decode, 182, 11, 128, 2, // Opcode: FADD_D
/* 11799 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 11814
/* 11804 */   MCD::OPC_CheckPredicate, 30, 169, 20, 0, // Skip to: 17098
/* 11809 */   MCD::OPC_Decode, 205, 12, 255, 1, // Opcode: FSUB_W
/* 11814 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 11829
/* 11819 */   MCD::OPC_CheckPredicate, 30, 154, 20, 0, // Skip to: 17098
/* 11824 */   MCD::OPC_Decode, 197, 12, 128, 2, // Opcode: FSUB_D
/* 11829 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 11844
/* 11834 */   MCD::OPC_CheckPredicate, 30, 139, 20, 0, // Skip to: 17098
/* 11839 */   MCD::OPC_Decode, 162, 12, 255, 1, // Opcode: FMUL_W
/* 11844 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 11859
/* 11849 */   MCD::OPC_CheckPredicate, 30, 124, 20, 0, // Skip to: 17098
/* 11854 */   MCD::OPC_Decode, 154, 12, 128, 2, // Opcode: FMUL_D
/* 11859 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 11874
/* 11864 */   MCD::OPC_CheckPredicate, 30, 109, 20, 0, // Skip to: 17098
/* 11869 */   MCD::OPC_Decode, 228, 11, 255, 1, // Opcode: FDIV_W
/* 11874 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 11889
/* 11879 */   MCD::OPC_CheckPredicate, 30, 94, 20, 0, // Skip to: 17098
/* 11884 */   MCD::OPC_Decode, 220, 11, 128, 2, // Opcode: FDIV_D
/* 11889 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 11904
/* 11894 */   MCD::OPC_CheckPredicate, 30, 79, 20, 0, // Skip to: 17098
/* 11899 */   MCD::OPC_Decode, 135, 12, 131, 2, // Opcode: FMADD_W
/* 11904 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 11919
/* 11909 */   MCD::OPC_CheckPredicate, 30, 64, 20, 0, // Skip to: 17098
/* 11914 */   MCD::OPC_Decode, 134, 12, 132, 2, // Opcode: FMADD_D
/* 11919 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 11934
/* 11924 */   MCD::OPC_CheckPredicate, 30, 49, 20, 0, // Skip to: 17098
/* 11929 */   MCD::OPC_Decode, 153, 12, 131, 2, // Opcode: FMSUB_W
/* 11934 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 11949
/* 11939 */   MCD::OPC_CheckPredicate, 30, 34, 20, 0, // Skip to: 17098
/* 11944 */   MCD::OPC_Decode, 152, 12, 132, 2, // Opcode: FMSUB_D
/* 11949 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 11964
/* 11954 */   MCD::OPC_CheckPredicate, 30, 19, 20, 0, // Skip to: 17098
/* 11959 */   MCD::OPC_Decode, 232, 11, 255, 1, // Opcode: FEXP2_W
/* 11964 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 11979
/* 11969 */   MCD::OPC_CheckPredicate, 30, 4, 20, 0, // Skip to: 17098
/* 11974 */   MCD::OPC_Decode, 231, 11, 128, 2, // Opcode: FEXP2_D
/* 11979 */   MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 11994
/* 11984 */   MCD::OPC_CheckPredicate, 30, 245, 19, 0, // Skip to: 17098
/* 11989 */   MCD::OPC_Decode, 229, 11, 167, 2, // Opcode: FEXDO_H
/* 11994 */   MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 12009
/* 11999 */   MCD::OPC_CheckPredicate, 30, 230, 19, 0, // Skip to: 17098
/* 12004 */   MCD::OPC_Decode, 230, 11, 168, 2, // Opcode: FEXDO_W
/* 12009 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 12024
/* 12014 */   MCD::OPC_CheckPredicate, 30, 215, 19, 0, // Skip to: 17098
/* 12019 */   MCD::OPC_Decode, 220, 12, 167, 2, // Opcode: FTQ_H
/* 12024 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 12039
/* 12029 */   MCD::OPC_CheckPredicate, 30, 200, 19, 0, // Skip to: 17098
/* 12034 */   MCD::OPC_Decode, 221, 12, 168, 2, // Opcode: FTQ_W
/* 12039 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 12054
/* 12044 */   MCD::OPC_CheckPredicate, 30, 185, 19, 0, // Skip to: 17098
/* 12049 */   MCD::OPC_Decode, 143, 12, 255, 1, // Opcode: FMIN_W
/* 12054 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 12069
/* 12059 */   MCD::OPC_CheckPredicate, 30, 170, 19, 0, // Skip to: 17098
/* 12064 */   MCD::OPC_Decode, 142, 12, 128, 2, // Opcode: FMIN_D
/* 12069 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 12084
/* 12074 */   MCD::OPC_CheckPredicate, 30, 155, 19, 0, // Skip to: 17098
/* 12079 */   MCD::OPC_Decode, 141, 12, 255, 1, // Opcode: FMIN_A_W
/* 12084 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 12099
/* 12089 */   MCD::OPC_CheckPredicate, 30, 140, 19, 0, // Skip to: 17098
/* 12094 */   MCD::OPC_Decode, 140, 12, 128, 2, // Opcode: FMIN_A_D
/* 12099 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 12114
/* 12104 */   MCD::OPC_CheckPredicate, 30, 125, 19, 0, // Skip to: 17098
/* 12109 */   MCD::OPC_Decode, 139, 12, 255, 1, // Opcode: FMAX_W
/* 12114 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 12129
/* 12119 */   MCD::OPC_CheckPredicate, 30, 110, 19, 0, // Skip to: 17098
/* 12124 */   MCD::OPC_Decode, 138, 12, 128, 2, // Opcode: FMAX_D
/* 12129 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 12144
/* 12134 */   MCD::OPC_CheckPredicate, 30, 95, 19, 0, // Skip to: 17098
/* 12139 */   MCD::OPC_Decode, 137, 12, 255, 1, // Opcode: FMAX_A_W
/* 12144 */   MCD::OPC_FilterValue, 31, 85, 19, 0, // Skip to: 17098
/* 12149 */   MCD::OPC_CheckPredicate, 30, 80, 19, 0, // Skip to: 17098
/* 12154 */   MCD::OPC_Decode, 136, 12, 128, 2, // Opcode: FMAX_A_D
/* 12159 */   MCD::OPC_FilterValue, 28, 107, 1, 0, // Skip to: 12527
/* 12164 */   MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 12167 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 12182
/* 12172 */   MCD::OPC_CheckPredicate, 30, 57, 19, 0, // Skip to: 17098
/* 12177 */   MCD::OPC_Decode, 209, 11, 255, 1, // Opcode: FCOR_W
/* 12182 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 12197
/* 12187 */   MCD::OPC_CheckPredicate, 30, 42, 19, 0, // Skip to: 17098
/* 12192 */   MCD::OPC_Decode, 208, 11, 128, 2, // Opcode: FCOR_D
/* 12197 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 12212
/* 12202 */   MCD::OPC_CheckPredicate, 30, 27, 19, 0, // Skip to: 17098
/* 12207 */   MCD::OPC_Decode, 217, 11, 255, 1, // Opcode: FCUNE_W
/* 12212 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 12227
/* 12217 */   MCD::OPC_CheckPredicate, 30, 12, 19, 0, // Skip to: 17098
/* 12222 */   MCD::OPC_Decode, 216, 11, 128, 2, // Opcode: FCUNE_D
/* 12227 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 12242
/* 12232 */   MCD::OPC_CheckPredicate, 30, 253, 18, 0, // Skip to: 17098
/* 12237 */   MCD::OPC_Decode, 207, 11, 255, 1, // Opcode: FCNE_W
/* 12242 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 12257
/* 12247 */   MCD::OPC_CheckPredicate, 30, 238, 18, 0, // Skip to: 17098
/* 12252 */   MCD::OPC_Decode, 206, 11, 128, 2, // Opcode: FCNE_D
/* 12257 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 12272
/* 12262 */   MCD::OPC_CheckPredicate, 30, 223, 18, 0, // Skip to: 17098
/* 12267 */   MCD::OPC_Decode, 217, 16, 254, 1, // Opcode: MUL_Q_H
/* 12272 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 12287
/* 12277 */   MCD::OPC_CheckPredicate, 30, 208, 18, 0, // Skip to: 17098
/* 12282 */   MCD::OPC_Decode, 218, 16, 255, 1, // Opcode: MUL_Q_W
/* 12287 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 12302
/* 12292 */   MCD::OPC_CheckPredicate, 30, 193, 18, 0, // Skip to: 17098
/* 12297 */   MCD::OPC_Decode, 207, 14, 130, 2, // Opcode: MADD_Q_H
/* 12302 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 12317
/* 12307 */   MCD::OPC_CheckPredicate, 30, 178, 18, 0, // Skip to: 17098
/* 12312 */   MCD::OPC_Decode, 208, 14, 131, 2, // Opcode: MADD_Q_W
/* 12317 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 12332
/* 12322 */   MCD::OPC_CheckPredicate, 30, 163, 18, 0, // Skip to: 17098
/* 12327 */   MCD::OPC_Decode, 130, 16, 130, 2, // Opcode: MSUB_Q_H
/* 12332 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 12347
/* 12337 */   MCD::OPC_CheckPredicate, 30, 148, 18, 0, // Skip to: 17098
/* 12342 */   MCD::OPC_Decode, 131, 16, 131, 2, // Opcode: MSUB_Q_W
/* 12347 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 12362
/* 12352 */   MCD::OPC_CheckPredicate, 30, 133, 18, 0, // Skip to: 17098
/* 12357 */   MCD::OPC_Decode, 188, 12, 255, 1, // Opcode: FSOR_W
/* 12362 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 12377
/* 12367 */   MCD::OPC_CheckPredicate, 30, 118, 18, 0, // Skip to: 17098
/* 12372 */   MCD::OPC_Decode, 187, 12, 128, 2, // Opcode: FSOR_D
/* 12377 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 12392
/* 12382 */   MCD::OPC_CheckPredicate, 30, 103, 18, 0, // Skip to: 17098
/* 12387 */   MCD::OPC_Decode, 213, 12, 255, 1, // Opcode: FSUNE_W
/* 12392 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 12407
/* 12397 */   MCD::OPC_CheckPredicate, 30, 88, 18, 0, // Skip to: 17098
/* 12402 */   MCD::OPC_Decode, 212, 12, 128, 2, // Opcode: FSUNE_D
/* 12407 */   MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 12422
/* 12412 */   MCD::OPC_CheckPredicate, 30, 73, 18, 0, // Skip to: 17098
/* 12417 */   MCD::OPC_Decode, 186, 12, 255, 1, // Opcode: FSNE_W
/* 12422 */   MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 12437
/* 12427 */   MCD::OPC_CheckPredicate, 30, 58, 18, 0, // Skip to: 17098
/* 12432 */   MCD::OPC_Decode, 185, 12, 128, 2, // Opcode: FSNE_D
/* 12437 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 12452
/* 12442 */   MCD::OPC_CheckPredicate, 30, 43, 18, 0, // Skip to: 17098
/* 12447 */   MCD::OPC_Decode, 193, 16, 254, 1, // Opcode: MULR_Q_H
/* 12452 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 12467
/* 12457 */   MCD::OPC_CheckPredicate, 30, 28, 18, 0, // Skip to: 17098
/* 12462 */   MCD::OPC_Decode, 194, 16, 255, 1, // Opcode: MULR_Q_W
/* 12467 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 12482
/* 12472 */   MCD::OPC_CheckPredicate, 30, 13, 18, 0, // Skip to: 17098
/* 12477 */   MCD::OPC_Decode, 191, 14, 130, 2, // Opcode: MADDR_Q_H
/* 12482 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 12497
/* 12487 */   MCD::OPC_CheckPredicate, 30, 254, 17, 0, // Skip to: 17098
/* 12492 */   MCD::OPC_Decode, 192, 14, 131, 2, // Opcode: MADDR_Q_W
/* 12497 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 12512
/* 12502 */   MCD::OPC_CheckPredicate, 30, 239, 17, 0, // Skip to: 17098
/* 12507 */   MCD::OPC_Decode, 242, 15, 130, 2, // Opcode: MSUBR_Q_H
/* 12512 */   MCD::OPC_FilterValue, 29, 229, 17, 0, // Skip to: 17098
/* 12517 */   MCD::OPC_CheckPredicate, 30, 224, 17, 0, // Skip to: 17098
/* 12522 */   MCD::OPC_Decode, 243, 15, 131, 2, // Opcode: MSUBR_Q_W
/* 12527 */   MCD::OPC_FilterValue, 30, 76, 3, 0, // Skip to: 13376
/* 12532 */   MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 12535 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 12550
/* 12540 */   MCD::OPC_CheckPredicate, 30, 201, 17, 0, // Skip to: 17098
/* 12545 */   MCD::OPC_Decode, 205, 5, 253, 1, // Opcode: AND_V
/* 12550 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 12565
/* 12555 */   MCD::OPC_CheckPredicate, 30, 186, 17, 0, // Skip to: 17098
/* 12560 */   MCD::OPC_Decode, 134, 17, 253, 1, // Opcode: OR_V
/* 12565 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 12580
/* 12570 */   MCD::OPC_CheckPredicate, 30, 171, 17, 0, // Skip to: 17098
/* 12575 */   MCD::OPC_Decode, 249, 16, 253, 1, // Opcode: NOR_V
/* 12580 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 12595
/* 12585 */   MCD::OPC_CheckPredicate, 30, 156, 17, 0, // Skip to: 17098
/* 12590 */   MCD::OPC_Decode, 185, 21, 253, 1, // Opcode: XOR_V
/* 12595 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 12610
/* 12600 */   MCD::OPC_CheckPredicate, 30, 141, 17, 0, // Skip to: 17098
/* 12605 */   MCD::OPC_Decode, 248, 6, 129, 2, // Opcode: BMNZ_V
/* 12610 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 12625
/* 12615 */   MCD::OPC_CheckPredicate, 30, 126, 17, 0, // Skip to: 17098
/* 12620 */   MCD::OPC_Decode, 250, 6, 129, 2, // Opcode: BMZ_V
/* 12625 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 12640
/* 12630 */   MCD::OPC_CheckPredicate, 30, 111, 17, 0, // Skip to: 17098
/* 12635 */   MCD::OPC_Decode, 164, 7, 129, 2, // Opcode: BSEL_V
/* 12640 */   MCD::OPC_FilterValue, 24, 243, 0, 0, // Skip to: 12888
/* 12645 */   MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 12648 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 12663
/* 12653 */   MCD::OPC_CheckPredicate, 30, 88, 17, 0, // Skip to: 17098
/* 12658 */   MCD::OPC_Decode, 245, 11, 169, 2, // Opcode: FILL_B
/* 12663 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 12678
/* 12668 */   MCD::OPC_CheckPredicate, 30, 73, 17, 0, // Skip to: 17098
/* 12673 */   MCD::OPC_Decode, 247, 11, 170, 2, // Opcode: FILL_H
/* 12678 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 12693
/* 12683 */   MCD::OPC_CheckPredicate, 30, 58, 17, 0, // Skip to: 17098
/* 12688 */   MCD::OPC_Decode, 248, 11, 171, 2, // Opcode: FILL_W
/* 12693 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 12708
/* 12698 */   MCD::OPC_CheckPredicate, 38, 43, 17, 0, // Skip to: 17098
/* 12703 */   MCD::OPC_Decode, 246, 11, 172, 2, // Opcode: FILL_D
/* 12708 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 12723
/* 12713 */   MCD::OPC_CheckPredicate, 30, 28, 17, 0, // Skip to: 17098
/* 12718 */   MCD::OPC_Decode, 152, 17, 161, 2, // Opcode: PCNT_B
/* 12723 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 12738
/* 12728 */   MCD::OPC_CheckPredicate, 30, 13, 17, 0, // Skip to: 17098
/* 12733 */   MCD::OPC_Decode, 154, 17, 173, 2, // Opcode: PCNT_H
/* 12738 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 12753
/* 12743 */   MCD::OPC_CheckPredicate, 30, 254, 16, 0, // Skip to: 17098
/* 12748 */   MCD::OPC_Decode, 155, 17, 174, 2, // Opcode: PCNT_W
/* 12753 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 12768
/* 12758 */   MCD::OPC_CheckPredicate, 30, 239, 16, 0, // Skip to: 17098
/* 12763 */   MCD::OPC_Decode, 153, 17, 175, 2, // Opcode: PCNT_D
/* 12768 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 12783
/* 12773 */   MCD::OPC_CheckPredicate, 30, 224, 16, 0, // Skip to: 17098
/* 12778 */   MCD::OPC_Decode, 226, 16, 161, 2, // Opcode: NLOC_B
/* 12783 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 12798
/* 12788 */   MCD::OPC_CheckPredicate, 30, 209, 16, 0, // Skip to: 17098
/* 12793 */   MCD::OPC_Decode, 228, 16, 173, 2, // Opcode: NLOC_H
/* 12798 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 12813
/* 12803 */   MCD::OPC_CheckPredicate, 30, 194, 16, 0, // Skip to: 17098
/* 12808 */   MCD::OPC_Decode, 229, 16, 174, 2, // Opcode: NLOC_W
/* 12813 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 12828
/* 12818 */   MCD::OPC_CheckPredicate, 30, 179, 16, 0, // Skip to: 17098
/* 12823 */   MCD::OPC_Decode, 227, 16, 175, 2, // Opcode: NLOC_D
/* 12828 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 12843
/* 12833 */   MCD::OPC_CheckPredicate, 30, 164, 16, 0, // Skip to: 17098
/* 12838 */   MCD::OPC_Decode, 230, 16, 161, 2, // Opcode: NLZC_B
/* 12843 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 12858
/* 12848 */   MCD::OPC_CheckPredicate, 30, 149, 16, 0, // Skip to: 17098
/* 12853 */   MCD::OPC_Decode, 232, 16, 173, 2, // Opcode: NLZC_H
/* 12858 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 12873
/* 12863 */   MCD::OPC_CheckPredicate, 30, 134, 16, 0, // Skip to: 17098
/* 12868 */   MCD::OPC_Decode, 233, 16, 174, 2, // Opcode: NLZC_W
/* 12873 */   MCD::OPC_FilterValue, 15, 124, 16, 0, // Skip to: 17098
/* 12878 */   MCD::OPC_CheckPredicate, 30, 119, 16, 0, // Skip to: 17098
/* 12883 */   MCD::OPC_Decode, 231, 16, 175, 2, // Opcode: NLZC_D
/* 12888 */   MCD::OPC_FilterValue, 25, 109, 16, 0, // Skip to: 17098
/* 12893 */   MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 12896 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 12911
/* 12901 */   MCD::OPC_CheckPredicate, 30, 96, 16, 0, // Skip to: 17098
/* 12906 */   MCD::OPC_Decode, 196, 11, 174, 2, // Opcode: FCLASS_W
/* 12911 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 12926
/* 12916 */   MCD::OPC_CheckPredicate, 30, 81, 16, 0, // Skip to: 17098
/* 12921 */   MCD::OPC_Decode, 195, 11, 175, 2, // Opcode: FCLASS_D
/* 12926 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 12941
/* 12931 */   MCD::OPC_CheckPredicate, 30, 66, 16, 0, // Skip to: 17098
/* 12936 */   MCD::OPC_Decode, 223, 12, 174, 2, // Opcode: FTRUNC_S_W
/* 12941 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 12956
/* 12946 */   MCD::OPC_CheckPredicate, 30, 51, 16, 0, // Skip to: 17098
/* 12951 */   MCD::OPC_Decode, 222, 12, 175, 2, // Opcode: FTRUNC_S_D
/* 12956 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 12971
/* 12961 */   MCD::OPC_CheckPredicate, 30, 36, 16, 0, // Skip to: 17098
/* 12966 */   MCD::OPC_Decode, 225, 12, 174, 2, // Opcode: FTRUNC_U_W
/* 12971 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 12986
/* 12976 */   MCD::OPC_CheckPredicate, 30, 21, 16, 0, // Skip to: 17098
/* 12981 */   MCD::OPC_Decode, 224, 12, 175, 2, // Opcode: FTRUNC_U_D
/* 12986 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 13001
/* 12991 */   MCD::OPC_CheckPredicate, 30, 6, 16, 0, // Skip to: 17098
/* 12996 */   MCD::OPC_Decode, 196, 12, 174, 2, // Opcode: FSQRT_W
/* 13001 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 13016
/* 13006 */   MCD::OPC_CheckPredicate, 30, 247, 15, 0, // Skip to: 17098
/* 13011 */   MCD::OPC_Decode, 189, 12, 175, 2, // Opcode: FSQRT_D
/* 13016 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 13031
/* 13021 */   MCD::OPC_CheckPredicate, 30, 232, 15, 0, // Skip to: 17098
/* 13026 */   MCD::OPC_Decode, 176, 12, 174, 2, // Opcode: FRSQRT_W
/* 13031 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 13046
/* 13036 */   MCD::OPC_CheckPredicate, 30, 217, 15, 0, // Skip to: 17098
/* 13041 */   MCD::OPC_Decode, 175, 12, 175, 2, // Opcode: FRSQRT_D
/* 13046 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 13061
/* 13051 */   MCD::OPC_CheckPredicate, 30, 202, 15, 0, // Skip to: 17098
/* 13056 */   MCD::OPC_Decode, 172, 12, 174, 2, // Opcode: FRCP_W
/* 13061 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 13076
/* 13066 */   MCD::OPC_CheckPredicate, 30, 187, 15, 0, // Skip to: 17098
/* 13071 */   MCD::OPC_Decode, 171, 12, 175, 2, // Opcode: FRCP_D
/* 13076 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 13091
/* 13081 */   MCD::OPC_CheckPredicate, 30, 172, 15, 0, // Skip to: 17098
/* 13086 */   MCD::OPC_Decode, 174, 12, 174, 2, // Opcode: FRINT_W
/* 13091 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 13106
/* 13096 */   MCD::OPC_CheckPredicate, 30, 157, 15, 0, // Skip to: 17098
/* 13101 */   MCD::OPC_Decode, 173, 12, 175, 2, // Opcode: FRINT_D
/* 13106 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 13121
/* 13111 */   MCD::OPC_CheckPredicate, 30, 142, 15, 0, // Skip to: 17098
/* 13116 */   MCD::OPC_Decode, 250, 11, 174, 2, // Opcode: FLOG2_W
/* 13121 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 13136
/* 13126 */   MCD::OPC_CheckPredicate, 30, 127, 15, 0, // Skip to: 17098
/* 13131 */   MCD::OPC_Decode, 249, 11, 175, 2, // Opcode: FLOG2_D
/* 13136 */   MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 13151
/* 13141 */   MCD::OPC_CheckPredicate, 30, 112, 15, 0, // Skip to: 17098
/* 13146 */   MCD::OPC_Decode, 234, 11, 176, 2, // Opcode: FEXUPL_W
/* 13151 */   MCD::OPC_FilterValue, 17, 10, 0, 0, // Skip to: 13166
/* 13156 */   MCD::OPC_CheckPredicate, 30, 97, 15, 0, // Skip to: 17098
/* 13161 */   MCD::OPC_Decode, 233, 11, 177, 2, // Opcode: FEXUPL_D
/* 13166 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 13181
/* 13171 */   MCD::OPC_CheckPredicate, 30, 82, 15, 0, // Skip to: 17098
/* 13176 */   MCD::OPC_Decode, 236, 11, 176, 2, // Opcode: FEXUPR_W
/* 13181 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 13196
/* 13186 */   MCD::OPC_CheckPredicate, 30, 67, 15, 0, // Skip to: 17098
/* 13191 */   MCD::OPC_Decode, 235, 11, 177, 2, // Opcode: FEXUPR_D
/* 13196 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 13211
/* 13201 */   MCD::OPC_CheckPredicate, 30, 52, 15, 0, // Skip to: 17098
/* 13206 */   MCD::OPC_Decode, 242, 11, 176, 2, // Opcode: FFQL_W
/* 13211 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 13226
/* 13216 */   MCD::OPC_CheckPredicate, 30, 37, 15, 0, // Skip to: 17098
/* 13221 */   MCD::OPC_Decode, 241, 11, 177, 2, // Opcode: FFQL_D
/* 13226 */   MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 13241
/* 13231 */   MCD::OPC_CheckPredicate, 30, 22, 15, 0, // Skip to: 17098
/* 13236 */   MCD::OPC_Decode, 244, 11, 176, 2, // Opcode: FFQR_W
/* 13241 */   MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 13256
/* 13246 */   MCD::OPC_CheckPredicate, 30, 7, 15, 0, // Skip to: 17098
/* 13251 */   MCD::OPC_Decode, 243, 11, 177, 2, // Opcode: FFQR_D
/* 13256 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 13271
/* 13261 */   MCD::OPC_CheckPredicate, 30, 248, 14, 0, // Skip to: 17098
/* 13266 */   MCD::OPC_Decode, 217, 12, 174, 2, // Opcode: FTINT_S_W
/* 13271 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 13286
/* 13276 */   MCD::OPC_CheckPredicate, 30, 233, 14, 0, // Skip to: 17098
/* 13281 */   MCD::OPC_Decode, 216, 12, 175, 2, // Opcode: FTINT_S_D
/* 13286 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 13301
/* 13291 */   MCD::OPC_CheckPredicate, 30, 218, 14, 0, // Skip to: 17098
/* 13296 */   MCD::OPC_Decode, 219, 12, 174, 2, // Opcode: FTINT_U_W
/* 13301 */   MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 13316
/* 13306 */   MCD::OPC_CheckPredicate, 30, 203, 14, 0, // Skip to: 17098
/* 13311 */   MCD::OPC_Decode, 218, 12, 175, 2, // Opcode: FTINT_U_D
/* 13316 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 13331
/* 13321 */   MCD::OPC_CheckPredicate, 30, 188, 14, 0, // Skip to: 17098
/* 13326 */   MCD::OPC_Decode, 238, 11, 174, 2, // Opcode: FFINT_S_W
/* 13331 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 13346
/* 13336 */   MCD::OPC_CheckPredicate, 30, 173, 14, 0, // Skip to: 17098
/* 13341 */   MCD::OPC_Decode, 237, 11, 175, 2, // Opcode: FFINT_S_D
/* 13346 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 13361
/* 13351 */   MCD::OPC_CheckPredicate, 30, 158, 14, 0, // Skip to: 17098
/* 13356 */   MCD::OPC_Decode, 240, 11, 174, 2, // Opcode: FFINT_U_W
/* 13361 */   MCD::OPC_FilterValue, 31, 148, 14, 0, // Skip to: 17098
/* 13366 */   MCD::OPC_CheckPredicate, 30, 143, 14, 0, // Skip to: 17098
/* 13371 */   MCD::OPC_Decode, 239, 11, 175, 2, // Opcode: FFINT_U_D
/* 13376 */   MCD::OPC_FilterValue, 32, 10, 0, 0, // Skip to: 13391
/* 13381 */   MCD::OPC_CheckPredicate, 30, 128, 14, 0, // Skip to: 17098
/* 13386 */   MCD::OPC_Decode, 220, 13, 178, 2, // Opcode: LD_B
/* 13391 */   MCD::OPC_FilterValue, 33, 10, 0, 0, // Skip to: 13406
/* 13396 */   MCD::OPC_CheckPredicate, 30, 113, 14, 0, // Skip to: 17098
/* 13401 */   MCD::OPC_Decode, 222, 13, 178, 2, // Opcode: LD_H
/* 13406 */   MCD::OPC_FilterValue, 34, 10, 0, 0, // Skip to: 13421
/* 13411 */   MCD::OPC_CheckPredicate, 30, 98, 14, 0, // Skip to: 17098
/* 13416 */   MCD::OPC_Decode, 223, 13, 178, 2, // Opcode: LD_W
/* 13421 */   MCD::OPC_FilterValue, 35, 10, 0, 0, // Skip to: 13436
/* 13426 */   MCD::OPC_CheckPredicate, 30, 83, 14, 0, // Skip to: 17098
/* 13431 */   MCD::OPC_Decode, 221, 13, 178, 2, // Opcode: LD_D
/* 13436 */   MCD::OPC_FilterValue, 36, 10, 0, 0, // Skip to: 13451
/* 13441 */   MCD::OPC_CheckPredicate, 30, 68, 14, 0, // Skip to: 17098
/* 13446 */   MCD::OPC_Decode, 228, 19, 178, 2, // Opcode: ST_B
/* 13451 */   MCD::OPC_FilterValue, 37, 10, 0, 0, // Skip to: 13466
/* 13456 */   MCD::OPC_CheckPredicate, 30, 53, 14, 0, // Skip to: 17098
/* 13461 */   MCD::OPC_Decode, 230, 19, 178, 2, // Opcode: ST_H
/* 13466 */   MCD::OPC_FilterValue, 38, 10, 0, 0, // Skip to: 13481
/* 13471 */   MCD::OPC_CheckPredicate, 30, 38, 14, 0, // Skip to: 17098
/* 13476 */   MCD::OPC_Decode, 231, 19, 178, 2, // Opcode: ST_W
/* 13481 */   MCD::OPC_FilterValue, 39, 28, 14, 0, // Skip to: 17098
/* 13486 */   MCD::OPC_CheckPredicate, 30, 23, 14, 0, // Skip to: 17098
/* 13491 */   MCD::OPC_Decode, 229, 19, 178, 2, // Opcode: ST_D
/* 13496 */   MCD::OPC_FilterValue, 31, 165, 12, 0, // Skip to: 16738
/* 13501 */   MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 13504 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 13519
/* 13509 */   MCD::OPC_CheckPredicate, 28, 0, 14, 0, // Skip to: 17098
/* 13514 */   MCD::OPC_Decode, 147, 11, 179, 2, // Opcode: EXT
/* 13519 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 13534
/* 13524 */   MCD::OPC_CheckPredicate, 28, 241, 13, 0, // Skip to: 17098
/* 13529 */   MCD::OPC_Decode, 132, 13, 180, 2, // Opcode: INS
/* 13534 */   MCD::OPC_FilterValue, 8, 17, 0, 0, // Skip to: 13556
/* 13539 */   MCD::OPC_CheckPredicate, 42, 226, 13, 0, // Skip to: 17098
/* 13544 */   MCD::OPC_CheckField, 6, 5, 0, 219, 13, 0, // Skip to: 17098
/* 13551 */   MCD::OPC_Decode, 170, 12, 181, 2, // Opcode: FORK
/* 13556 */   MCD::OPC_FilterValue, 9, 23, 0, 0, // Skip to: 13584
/* 13561 */   MCD::OPC_CheckPredicate, 42, 204, 13, 0, // Skip to: 17098
/* 13566 */   MCD::OPC_CheckField, 16, 5, 0, 197, 13, 0, // Skip to: 17098
/* 13573 */   MCD::OPC_CheckField, 6, 5, 0, 190, 13, 0, // Skip to: 17098
/* 13580 */   MCD::OPC_Decode, 190, 21, 14, // Opcode: YIELD
/* 13584 */   MCD::OPC_FilterValue, 10, 48, 0, 0, // Skip to: 13637
/* 13589 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 13592 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 13607
/* 13597 */   MCD::OPC_CheckPredicate, 37, 168, 13, 0, // Skip to: 17098
/* 13602 */   MCD::OPC_Decode, 167, 14, 182, 2, // Opcode: LWX
/* 13607 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 13622
/* 13612 */   MCD::OPC_CheckPredicate, 37, 153, 13, 0, // Skip to: 17098
/* 13617 */   MCD::OPC_Decode, 232, 13, 182, 2, // Opcode: LHX
/* 13622 */   MCD::OPC_FilterValue, 6, 143, 13, 0, // Skip to: 17098
/* 13627 */   MCD::OPC_CheckPredicate, 37, 138, 13, 0, // Skip to: 17098
/* 13632 */   MCD::OPC_Decode, 192, 13, 182, 2, // Opcode: LBUX
/* 13637 */   MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 13659
/* 13642 */   MCD::OPC_CheckPredicate, 37, 123, 13, 0, // Skip to: 17098
/* 13647 */   MCD::OPC_CheckField, 6, 10, 0, 116, 13, 0, // Skip to: 17098
/* 13654 */   MCD::OPC_Decode, 137, 13, 183, 2, // Opcode: INSV
/* 13659 */   MCD::OPC_FilterValue, 16, 109, 1, 0, // Skip to: 14029
/* 13664 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 13667 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 13682
/* 13672 */   MCD::OPC_CheckPredicate, 37, 93, 13, 0, // Skip to: 17098
/* 13677 */   MCD::OPC_Decode, 163, 5, 184, 2, // Opcode: ADDU_QB
/* 13682 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 13697
/* 13687 */   MCD::OPC_CheckPredicate, 37, 78, 13, 0, // Skip to: 17098
/* 13692 */   MCD::OPC_Decode, 144, 20, 184, 2, // Opcode: SUBU_QB
/* 13697 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 13712
/* 13702 */   MCD::OPC_CheckPredicate, 37, 63, 13, 0, // Skip to: 17098
/* 13707 */   MCD::OPC_Decode, 167, 5, 184, 2, // Opcode: ADDU_S_QB
/* 13712 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 13727
/* 13717 */   MCD::OPC_CheckPredicate, 37, 48, 13, 0, // Skip to: 17098
/* 13722 */   MCD::OPC_Decode, 148, 20, 184, 2, // Opcode: SUBU_S_QB
/* 13727 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 13742
/* 13732 */   MCD::OPC_CheckPredicate, 37, 33, 13, 0, // Skip to: 17098
/* 13737 */   MCD::OPC_Decode, 181, 16, 184, 2, // Opcode: MULEU_S_PH_QBL
/* 13742 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 13757
/* 13747 */   MCD::OPC_CheckPredicate, 37, 18, 13, 0, // Skip to: 17098
/* 13752 */   MCD::OPC_Decode, 183, 16, 184, 2, // Opcode: MULEU_S_PH_QBR
/* 13757 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 13772
/* 13762 */   MCD::OPC_CheckPredicate, 69, 3, 13, 0, // Skip to: 17098
/* 13767 */   MCD::OPC_Decode, 161, 5, 184, 2, // Opcode: ADDU_PH
/* 13772 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 13787
/* 13777 */   MCD::OPC_CheckPredicate, 69, 244, 12, 0, // Skip to: 17098
/* 13782 */   MCD::OPC_Decode, 142, 20, 184, 2, // Opcode: SUBU_PH
/* 13787 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 13802
/* 13792 */   MCD::OPC_CheckPredicate, 37, 229, 12, 0, // Skip to: 17098
/* 13797 */   MCD::OPC_Decode, 134, 5, 184, 2, // Opcode: ADDQ_PH
/* 13802 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 13817
/* 13807 */   MCD::OPC_CheckPredicate, 37, 214, 12, 0, // Skip to: 17098
/* 13812 */   MCD::OPC_Decode, 241, 19, 184, 2, // Opcode: SUBQ_PH
/* 13817 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 13832
/* 13822 */   MCD::OPC_CheckPredicate, 69, 199, 12, 0, // Skip to: 17098
/* 13827 */   MCD::OPC_Decode, 165, 5, 184, 2, // Opcode: ADDU_S_PH
/* 13832 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 13847
/* 13837 */   MCD::OPC_CheckPredicate, 69, 184, 12, 0, // Skip to: 17098
/* 13842 */   MCD::OPC_Decode, 146, 20, 184, 2, // Opcode: SUBU_S_PH
/* 13847 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 13862
/* 13852 */   MCD::OPC_CheckPredicate, 37, 169, 12, 0, // Skip to: 17098
/* 13857 */   MCD::OPC_Decode, 136, 5, 184, 2, // Opcode: ADDQ_S_PH
/* 13862 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 13877
/* 13867 */   MCD::OPC_CheckPredicate, 37, 154, 12, 0, // Skip to: 17098
/* 13872 */   MCD::OPC_Decode, 243, 19, 184, 2, // Opcode: SUBQ_S_PH
/* 13877 */   MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 13891
/* 13882 */   MCD::OPC_CheckPredicate, 37, 139, 12, 0, // Skip to: 17098
/* 13887 */   MCD::OPC_Decode, 140, 5, 50, // Opcode: ADDSC
/* 13891 */   MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 13905
/* 13896 */   MCD::OPC_CheckPredicate, 37, 125, 12, 0, // Skip to: 17098
/* 13901 */   MCD::OPC_Decode, 177, 5, 50, // Opcode: ADDWC
/* 13905 */   MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 13919
/* 13910 */   MCD::OPC_CheckPredicate, 37, 111, 12, 0, // Skip to: 17098
/* 13915 */   MCD::OPC_Decode, 179, 15, 50, // Opcode: MODSUB
/* 13919 */   MCD::OPC_FilterValue, 20, 17, 0, 0, // Skip to: 13941
/* 13924 */   MCD::OPC_CheckPredicate, 37, 97, 12, 0, // Skip to: 17098
/* 13929 */   MCD::OPC_CheckField, 16, 5, 0, 90, 12, 0, // Skip to: 17098
/* 13936 */   MCD::OPC_Decode, 206, 17, 185, 2, // Opcode: RADDU_W_QB
/* 13941 */   MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 13955
/* 13946 */   MCD::OPC_CheckPredicate, 37, 75, 12, 0, // Skip to: 17098
/* 13951 */   MCD::OPC_Decode, 138, 5, 50, // Opcode: ADDQ_S_W
/* 13955 */   MCD::OPC_FilterValue, 23, 9, 0, 0, // Skip to: 13969
/* 13960 */   MCD::OPC_CheckPredicate, 37, 61, 12, 0, // Skip to: 17098
/* 13965 */   MCD::OPC_Decode, 245, 19, 50, // Opcode: SUBQ_S_W
/* 13969 */   MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 13984
/* 13974 */   MCD::OPC_CheckPredicate, 37, 47, 12, 0, // Skip to: 17098
/* 13979 */   MCD::OPC_Decode, 177, 16, 186, 2, // Opcode: MULEQ_S_W_PHL
/* 13984 */   MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 13999
/* 13989 */   MCD::OPC_CheckPredicate, 37, 32, 12, 0, // Skip to: 17098
/* 13994 */   MCD::OPC_Decode, 179, 16, 186, 2, // Opcode: MULEQ_S_W_PHR
/* 13999 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 14014
/* 14004 */   MCD::OPC_CheckPredicate, 69, 17, 12, 0, // Skip to: 17098
/* 14009 */   MCD::OPC_Decode, 189, 16, 184, 2, // Opcode: MULQ_S_PH
/* 14014 */   MCD::OPC_FilterValue, 31, 7, 12, 0, // Skip to: 17098
/* 14019 */   MCD::OPC_CheckPredicate, 37, 2, 12, 0, // Skip to: 17098
/* 14024 */   MCD::OPC_Decode, 185, 16, 184, 2, // Opcode: MULQ_RS_PH
/* 14029 */   MCD::OPC_FilterValue, 17, 113, 1, 0, // Skip to: 14403
/* 14034 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 14037 */   MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 14058
/* 14042 */   MCD::OPC_CheckPredicate, 37, 235, 11, 0, // Skip to: 17098
/* 14047 */   MCD::OPC_CheckField, 11, 5, 0, 228, 11, 0, // Skip to: 17098
/* 14054 */   MCD::OPC_Decode, 150, 8, 68, // Opcode: CMPU_EQ_QB
/* 14058 */   MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 14079
/* 14063 */   MCD::OPC_CheckPredicate, 37, 214, 11, 0, // Skip to: 17098
/* 14068 */   MCD::OPC_CheckField, 11, 5, 0, 207, 11, 0, // Skip to: 17098
/* 14075 */   MCD::OPC_Decode, 154, 8, 68, // Opcode: CMPU_LT_QB
/* 14079 */   MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 14100
/* 14084 */   MCD::OPC_CheckPredicate, 37, 193, 11, 0, // Skip to: 17098
/* 14089 */   MCD::OPC_CheckField, 11, 5, 0, 186, 11, 0, // Skip to: 17098
/* 14096 */   MCD::OPC_Decode, 152, 8, 68, // Opcode: CMPU_LE_QB
/* 14100 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 14115
/* 14105 */   MCD::OPC_CheckPredicate, 37, 172, 11, 0, // Skip to: 17098
/* 14110 */   MCD::OPC_Decode, 158, 17, 184, 2, // Opcode: PICK_QB
/* 14115 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 14130
/* 14120 */   MCD::OPC_CheckPredicate, 37, 157, 11, 0, // Skip to: 17098
/* 14125 */   MCD::OPC_Decode, 144, 8, 186, 2, // Opcode: CMPGU_EQ_QB
/* 14130 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 14145
/* 14135 */   MCD::OPC_CheckPredicate, 37, 142, 11, 0, // Skip to: 17098
/* 14140 */   MCD::OPC_Decode, 148, 8, 186, 2, // Opcode: CMPGU_LT_QB
/* 14145 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 14160
/* 14150 */   MCD::OPC_CheckPredicate, 37, 127, 11, 0, // Skip to: 17098
/* 14155 */   MCD::OPC_Decode, 146, 8, 186, 2, // Opcode: CMPGU_LE_QB
/* 14160 */   MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 14181
/* 14165 */   MCD::OPC_CheckPredicate, 37, 112, 11, 0, // Skip to: 17098
/* 14170 */   MCD::OPC_CheckField, 11, 5, 0, 105, 11, 0, // Skip to: 17098
/* 14177 */   MCD::OPC_Decode, 160, 8, 68, // Opcode: CMP_EQ_PH
/* 14181 */   MCD::OPC_FilterValue, 9, 16, 0, 0, // Skip to: 14202
/* 14186 */   MCD::OPC_CheckPredicate, 37, 91, 11, 0, // Skip to: 17098
/* 14191 */   MCD::OPC_CheckField, 11, 5, 0, 84, 11, 0, // Skip to: 17098
/* 14198 */   MCD::OPC_Decode, 174, 8, 68, // Opcode: CMP_LT_PH
/* 14202 */   MCD::OPC_FilterValue, 10, 16, 0, 0, // Skip to: 14223
/* 14207 */   MCD::OPC_CheckPredicate, 37, 70, 11, 0, // Skip to: 17098
/* 14212 */   MCD::OPC_CheckField, 11, 5, 0, 63, 11, 0, // Skip to: 17098
/* 14219 */   MCD::OPC_Decode, 168, 8, 68, // Opcode: CMP_LE_PH
/* 14223 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 14238
/* 14228 */   MCD::OPC_CheckPredicate, 37, 49, 11, 0, // Skip to: 17098
/* 14233 */   MCD::OPC_Decode, 156, 17, 184, 2, // Opcode: PICK_PH
/* 14238 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 14253
/* 14243 */   MCD::OPC_CheckPredicate, 37, 34, 11, 0, // Skip to: 17098
/* 14248 */   MCD::OPC_Decode, 187, 17, 184, 2, // Opcode: PRECRQ_QB_PH
/* 14253 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 14268
/* 14258 */   MCD::OPC_CheckPredicate, 69, 19, 11, 0, // Skip to: 17098
/* 14263 */   MCD::OPC_Decode, 191, 17, 184, 2, // Opcode: PRECR_QB_PH
/* 14268 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 14283
/* 14273 */   MCD::OPC_CheckPredicate, 37, 4, 11, 0, // Skip to: 17098
/* 14278 */   MCD::OPC_Decode, 139, 17, 184, 2, // Opcode: PACKRL_PH
/* 14283 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 14298
/* 14288 */   MCD::OPC_CheckPredicate, 37, 245, 10, 0, // Skip to: 17098
/* 14293 */   MCD::OPC_Decode, 183, 17, 184, 2, // Opcode: PRECRQU_S_QB_PH
/* 14298 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 14313
/* 14303 */   MCD::OPC_CheckPredicate, 37, 230, 10, 0, // Skip to: 17098
/* 14308 */   MCD::OPC_Decode, 185, 17, 187, 2, // Opcode: PRECRQ_PH_W
/* 14313 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 14328
/* 14318 */   MCD::OPC_CheckPredicate, 37, 215, 10, 0, // Skip to: 17098
/* 14323 */   MCD::OPC_Decode, 189, 17, 187, 2, // Opcode: PRECRQ_RS_PH_W
/* 14328 */   MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 14343
/* 14333 */   MCD::OPC_CheckPredicate, 69, 200, 10, 0, // Skip to: 17098
/* 14338 */   MCD::OPC_Decode, 138, 8, 186, 2, // Opcode: CMPGDU_EQ_QB
/* 14343 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 14358
/* 14348 */   MCD::OPC_CheckPredicate, 69, 185, 10, 0, // Skip to: 17098
/* 14353 */   MCD::OPC_Decode, 142, 8, 186, 2, // Opcode: CMPGDU_LT_QB
/* 14358 */   MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 14373
/* 14363 */   MCD::OPC_CheckPredicate, 69, 170, 10, 0, // Skip to: 17098
/* 14368 */   MCD::OPC_Decode, 140, 8, 186, 2, // Opcode: CMPGDU_LE_QB
/* 14373 */   MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 14388
/* 14378 */   MCD::OPC_CheckPredicate, 69, 155, 10, 0, // Skip to: 17098
/* 14383 */   MCD::OPC_Decode, 193, 17, 188, 2, // Opcode: PRECR_SRA_PH_W
/* 14388 */   MCD::OPC_FilterValue, 31, 145, 10, 0, // Skip to: 17098
/* 14393 */   MCD::OPC_CheckPredicate, 69, 140, 10, 0, // Skip to: 17098
/* 14398 */   MCD::OPC_Decode, 195, 17, 188, 2, // Opcode: PRECR_SRA_R_PH_W
/* 14403 */   MCD::OPC_FilterValue, 18, 128, 1, 0, // Skip to: 14792
/* 14408 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 14411 */   MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 14433
/* 14416 */   MCD::OPC_CheckPredicate, 69, 117, 10, 0, // Skip to: 17098
/* 14421 */   MCD::OPC_CheckField, 21, 5, 0, 110, 10, 0, // Skip to: 17098
/* 14428 */   MCD::OPC_Decode, 241, 4, 189, 2, // Opcode: ABSQ_S_QB
/* 14433 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 14448
/* 14438 */   MCD::OPC_CheckPredicate, 37, 95, 10, 0, // Skip to: 17098
/* 14443 */   MCD::OPC_Decode, 227, 17, 190, 2, // Opcode: REPL_QB
/* 14448 */   MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 14470
/* 14453 */   MCD::OPC_CheckPredicate, 37, 80, 10, 0, // Skip to: 17098
/* 14458 */   MCD::OPC_CheckField, 21, 5, 0, 73, 10, 0, // Skip to: 17098
/* 14465 */   MCD::OPC_Decode, 223, 17, 191, 2, // Opcode: REPLV_QB
/* 14470 */   MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 14492
/* 14475 */   MCD::OPC_CheckPredicate, 37, 58, 10, 0, // Skip to: 17098
/* 14480 */   MCD::OPC_CheckField, 21, 5, 0, 51, 10, 0, // Skip to: 17098
/* 14487 */   MCD::OPC_Decode, 163, 17, 189, 2, // Opcode: PRECEQU_PH_QBL
/* 14492 */   MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 14514
/* 14497 */   MCD::OPC_CheckPredicate, 37, 36, 10, 0, // Skip to: 17098
/* 14502 */   MCD::OPC_CheckField, 21, 5, 0, 29, 10, 0, // Skip to: 17098
/* 14509 */   MCD::OPC_Decode, 167, 17, 189, 2, // Opcode: PRECEQU_PH_QBR
/* 14514 */   MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 14536
/* 14519 */   MCD::OPC_CheckPredicate, 37, 14, 10, 0, // Skip to: 17098
/* 14524 */   MCD::OPC_CheckField, 21, 5, 0, 7, 10, 0, // Skip to: 17098
/* 14531 */   MCD::OPC_Decode, 164, 17, 189, 2, // Opcode: PRECEQU_PH_QBLA
/* 14536 */   MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 14558
/* 14541 */   MCD::OPC_CheckPredicate, 37, 248, 9, 0, // Skip to: 17098
/* 14546 */   MCD::OPC_CheckField, 21, 5, 0, 241, 9, 0, // Skip to: 17098
/* 14553 */   MCD::OPC_Decode, 168, 17, 189, 2, // Opcode: PRECEQU_PH_QBRA
/* 14558 */   MCD::OPC_FilterValue, 9, 17, 0, 0, // Skip to: 14580
/* 14563 */   MCD::OPC_CheckPredicate, 37, 226, 9, 0, // Skip to: 17098
/* 14568 */   MCD::OPC_CheckField, 21, 5, 0, 219, 9, 0, // Skip to: 17098
/* 14575 */   MCD::OPC_Decode, 239, 4, 189, 2, // Opcode: ABSQ_S_PH
/* 14580 */   MCD::OPC_FilterValue, 10, 9, 0, 0, // Skip to: 14594
/* 14585 */   MCD::OPC_CheckPredicate, 37, 204, 9, 0, // Skip to: 17098
/* 14590 */   MCD::OPC_Decode, 225, 17, 85, // Opcode: REPL_PH
/* 14594 */   MCD::OPC_FilterValue, 11, 17, 0, 0, // Skip to: 14616
/* 14599 */   MCD::OPC_CheckPredicate, 37, 190, 9, 0, // Skip to: 17098
/* 14604 */   MCD::OPC_CheckField, 21, 5, 0, 183, 9, 0, // Skip to: 17098
/* 14611 */   MCD::OPC_Decode, 221, 17, 191, 2, // Opcode: REPLV_PH
/* 14616 */   MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 14638
/* 14621 */   MCD::OPC_CheckPredicate, 37, 168, 9, 0, // Skip to: 17098
/* 14626 */   MCD::OPC_CheckField, 21, 5, 0, 161, 9, 0, // Skip to: 17098
/* 14633 */   MCD::OPC_Decode, 171, 17, 192, 2, // Opcode: PRECEQ_W_PHL
/* 14638 */   MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 14660
/* 14643 */   MCD::OPC_CheckPredicate, 37, 146, 9, 0, // Skip to: 17098
/* 14648 */   MCD::OPC_CheckField, 21, 5, 0, 139, 9, 0, // Skip to: 17098
/* 14655 */   MCD::OPC_Decode, 173, 17, 192, 2, // Opcode: PRECEQ_W_PHR
/* 14660 */   MCD::OPC_FilterValue, 17, 17, 0, 0, // Skip to: 14682
/* 14665 */   MCD::OPC_CheckPredicate, 37, 124, 9, 0, // Skip to: 17098
/* 14670 */   MCD::OPC_CheckField, 21, 5, 0, 117, 9, 0, // Skip to: 17098
/* 14677 */   MCD::OPC_Decode, 243, 4, 193, 2, // Opcode: ABSQ_S_W
/* 14682 */   MCD::OPC_FilterValue, 27, 17, 0, 0, // Skip to: 14704
/* 14687 */   MCD::OPC_CheckPredicate, 37, 102, 9, 0, // Skip to: 17098
/* 14692 */   MCD::OPC_CheckField, 21, 5, 0, 95, 9, 0, // Skip to: 17098
/* 14699 */   MCD::OPC_Decode, 215, 6, 193, 2, // Opcode: BITREV
/* 14704 */   MCD::OPC_FilterValue, 28, 17, 0, 0, // Skip to: 14726
/* 14709 */   MCD::OPC_CheckPredicate, 37, 80, 9, 0, // Skip to: 17098
/* 14714 */   MCD::OPC_CheckField, 21, 5, 0, 73, 9, 0, // Skip to: 17098
/* 14721 */   MCD::OPC_Decode, 175, 17, 189, 2, // Opcode: PRECEU_PH_QBL
/* 14726 */   MCD::OPC_FilterValue, 29, 17, 0, 0, // Skip to: 14748
/* 14731 */   MCD::OPC_CheckPredicate, 37, 58, 9, 0, // Skip to: 17098
/* 14736 */   MCD::OPC_CheckField, 21, 5, 0, 51, 9, 0, // Skip to: 17098
/* 14743 */   MCD::OPC_Decode, 179, 17, 189, 2, // Opcode: PRECEU_PH_QBR
/* 14748 */   MCD::OPC_FilterValue, 30, 17, 0, 0, // Skip to: 14770
/* 14753 */   MCD::OPC_CheckPredicate, 37, 36, 9, 0, // Skip to: 17098
/* 14758 */   MCD::OPC_CheckField, 21, 5, 0, 29, 9, 0, // Skip to: 17098
/* 14765 */   MCD::OPC_Decode, 176, 17, 189, 2, // Opcode: PRECEU_PH_QBLA
/* 14770 */   MCD::OPC_FilterValue, 31, 19, 9, 0, // Skip to: 17098
/* 14775 */   MCD::OPC_CheckPredicate, 37, 14, 9, 0, // Skip to: 17098
/* 14780 */   MCD::OPC_CheckField, 21, 5, 0, 7, 9, 0, // Skip to: 17098
/* 14787 */   MCD::OPC_Decode, 180, 17, 189, 2, // Opcode: PRECEU_PH_QBRA
/* 14792 */   MCD::OPC_FilterValue, 19, 75, 1, 0, // Skip to: 15128
/* 14797 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 14800 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 14815
/* 14805 */   MCD::OPC_CheckPredicate, 37, 240, 8, 0, // Skip to: 17098
/* 14810 */   MCD::OPC_Decode, 226, 18, 194, 2, // Opcode: SHLL_QB
/* 14815 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 14830
/* 14820 */   MCD::OPC_CheckPredicate, 37, 225, 8, 0, // Skip to: 17098
/* 14825 */   MCD::OPC_Decode, 130, 19, 194, 2, // Opcode: SHRL_QB
/* 14830 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 14845
/* 14835 */   MCD::OPC_CheckPredicate, 37, 210, 8, 0, // Skip to: 17098
/* 14840 */   MCD::OPC_Decode, 218, 18, 195, 2, // Opcode: SHLLV_QB
/* 14845 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 14860
/* 14850 */   MCD::OPC_CheckPredicate, 37, 195, 8, 0, // Skip to: 17098
/* 14855 */   MCD::OPC_Decode, 254, 18, 195, 2, // Opcode: SHRLV_QB
/* 14860 */   MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 14875
/* 14865 */   MCD::OPC_CheckPredicate, 69, 180, 8, 0, // Skip to: 17098
/* 14870 */   MCD::OPC_Decode, 244, 18, 194, 2, // Opcode: SHRA_QB
/* 14875 */   MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 14890
/* 14880 */   MCD::OPC_CheckPredicate, 69, 165, 8, 0, // Skip to: 17098
/* 14885 */   MCD::OPC_Decode, 248, 18, 194, 2, // Opcode: SHRA_R_QB
/* 14890 */   MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 14905
/* 14895 */   MCD::OPC_CheckPredicate, 69, 150, 8, 0, // Skip to: 17098
/* 14900 */   MCD::OPC_Decode, 234, 18, 195, 2, // Opcode: SHRAV_QB
/* 14905 */   MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 14920
/* 14910 */   MCD::OPC_CheckPredicate, 69, 135, 8, 0, // Skip to: 17098
/* 14915 */   MCD::OPC_Decode, 238, 18, 195, 2, // Opcode: SHRAV_R_QB
/* 14920 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 14935
/* 14925 */   MCD::OPC_CheckPredicate, 37, 120, 8, 0, // Skip to: 17098
/* 14930 */   MCD::OPC_Decode, 224, 18, 194, 2, // Opcode: SHLL_PH
/* 14935 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 14950
/* 14940 */   MCD::OPC_CheckPredicate, 37, 105, 8, 0, // Skip to: 17098
/* 14945 */   MCD::OPC_Decode, 242, 18, 194, 2, // Opcode: SHRA_PH
/* 14950 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 14965
/* 14955 */   MCD::OPC_CheckPredicate, 37, 90, 8, 0, // Skip to: 17098
/* 14960 */   MCD::OPC_Decode, 216, 18, 195, 2, // Opcode: SHLLV_PH
/* 14965 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 14980
/* 14970 */   MCD::OPC_CheckPredicate, 37, 75, 8, 0, // Skip to: 17098
/* 14975 */   MCD::OPC_Decode, 232, 18, 195, 2, // Opcode: SHRAV_PH
/* 14980 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 14995
/* 14985 */   MCD::OPC_CheckPredicate, 37, 60, 8, 0, // Skip to: 17098
/* 14990 */   MCD::OPC_Decode, 228, 18, 194, 2, // Opcode: SHLL_S_PH
/* 14995 */   MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 15010
/* 15000 */   MCD::OPC_CheckPredicate, 37, 45, 8, 0, // Skip to: 17098
/* 15005 */   MCD::OPC_Decode, 246, 18, 194, 2, // Opcode: SHRA_R_PH
/* 15010 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 15025
/* 15015 */   MCD::OPC_CheckPredicate, 37, 30, 8, 0, // Skip to: 17098
/* 15020 */   MCD::OPC_Decode, 220, 18, 195, 2, // Opcode: SHLLV_S_PH
/* 15025 */   MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 15040
/* 15030 */   MCD::OPC_CheckPredicate, 37, 15, 8, 0, // Skip to: 17098
/* 15035 */   MCD::OPC_Decode, 236, 18, 195, 2, // Opcode: SHRAV_R_PH
/* 15040 */   MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 15055
/* 15045 */   MCD::OPC_CheckPredicate, 37, 0, 8, 0, // Skip to: 17098
/* 15050 */   MCD::OPC_Decode, 230, 18, 196, 2, // Opcode: SHLL_S_W
/* 15055 */   MCD::OPC_FilterValue, 21, 10, 0, 0, // Skip to: 15070
/* 15060 */   MCD::OPC_CheckPredicate, 37, 241, 7, 0, // Skip to: 17098
/* 15065 */   MCD::OPC_Decode, 250, 18, 196, 2, // Opcode: SHRA_R_W
/* 15070 */   MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 15084
/* 15075 */   MCD::OPC_CheckPredicate, 37, 226, 7, 0, // Skip to: 17098
/* 15080 */   MCD::OPC_Decode, 222, 18, 44, // Opcode: SHLLV_S_W
/* 15084 */   MCD::OPC_FilterValue, 23, 9, 0, 0, // Skip to: 15098
/* 15089 */   MCD::OPC_CheckPredicate, 37, 212, 7, 0, // Skip to: 17098
/* 15094 */   MCD::OPC_Decode, 240, 18, 44, // Opcode: SHRAV_R_W
/* 15098 */   MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 15113
/* 15103 */   MCD::OPC_CheckPredicate, 69, 198, 7, 0, // Skip to: 17098
/* 15108 */   MCD::OPC_Decode, 128, 19, 194, 2, // Opcode: SHRL_PH
/* 15113 */   MCD::OPC_FilterValue, 27, 188, 7, 0, // Skip to: 17098
/* 15118 */   MCD::OPC_CheckPredicate, 69, 183, 7, 0, // Skip to: 17098
/* 15123 */   MCD::OPC_Decode, 252, 18, 195, 2, // Opcode: SHRLV_PH
/* 15128 */   MCD::OPC_FilterValue, 24, 237, 0, 0, // Skip to: 15370
/* 15133 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 15136 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 15151
/* 15141 */   MCD::OPC_CheckPredicate, 69, 160, 7, 0, // Skip to: 17098
/* 15146 */   MCD::OPC_Decode, 156, 5, 184, 2, // Opcode: ADDUH_QB
/* 15151 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 15166
/* 15156 */   MCD::OPC_CheckPredicate, 69, 145, 7, 0, // Skip to: 17098
/* 15161 */   MCD::OPC_Decode, 137, 20, 184, 2, // Opcode: SUBUH_QB
/* 15166 */   MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 15181
/* 15171 */   MCD::OPC_CheckPredicate, 69, 130, 7, 0, // Skip to: 17098
/* 15176 */   MCD::OPC_Decode, 158, 5, 184, 2, // Opcode: ADDUH_R_QB
/* 15181 */   MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 15196
/* 15186 */   MCD::OPC_CheckPredicate, 69, 115, 7, 0, // Skip to: 17098
/* 15191 */   MCD::OPC_Decode, 139, 20, 184, 2, // Opcode: SUBUH_R_QB
/* 15196 */   MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 15211
/* 15201 */   MCD::OPC_CheckPredicate, 69, 100, 7, 0, // Skip to: 17098
/* 15206 */   MCD::OPC_Decode, 254, 4, 184, 2, // Opcode: ADDQH_PH
/* 15211 */   MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 15226
/* 15216 */   MCD::OPC_CheckPredicate, 69, 85, 7, 0, // Skip to: 17098
/* 15221 */   MCD::OPC_Decode, 233, 19, 184, 2, // Opcode: SUBQH_PH
/* 15226 */   MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 15241
/* 15231 */   MCD::OPC_CheckPredicate, 69, 70, 7, 0, // Skip to: 17098
/* 15236 */   MCD::OPC_Decode, 128, 5, 184, 2, // Opcode: ADDQH_R_PH
/* 15241 */   MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 15256
/* 15246 */   MCD::OPC_CheckPredicate, 69, 55, 7, 0, // Skip to: 17098
/* 15251 */   MCD::OPC_Decode, 235, 19, 184, 2, // Opcode: SUBQH_R_PH
/* 15256 */   MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 15271
/* 15261 */   MCD::OPC_CheckPredicate, 69, 40, 7, 0, // Skip to: 17098
/* 15266 */   MCD::OPC_Decode, 215, 16, 184, 2, // Opcode: MUL_PH
/* 15271 */   MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 15286
/* 15276 */   MCD::OPC_CheckPredicate, 69, 25, 7, 0, // Skip to: 17098
/* 15281 */   MCD::OPC_Decode, 220, 16, 184, 2, // Opcode: MUL_S_PH
/* 15286 */   MCD::OPC_FilterValue, 16, 9, 0, 0, // Skip to: 15300
/* 15291 */   MCD::OPC_CheckPredicate, 69, 10, 7, 0, // Skip to: 17098
/* 15296 */   MCD::OPC_Decode, 132, 5, 50, // Opcode: ADDQH_W
/* 15300 */   MCD::OPC_FilterValue, 17, 9, 0, 0, // Skip to: 15314
/* 15305 */   MCD::OPC_CheckPredicate, 69, 252, 6, 0, // Skip to: 17098
/* 15310 */   MCD::OPC_Decode, 239, 19, 50, // Opcode: SUBQH_W
/* 15314 */   MCD::OPC_FilterValue, 18, 9, 0, 0, // Skip to: 15328
/* 15319 */   MCD::OPC_CheckPredicate, 69, 238, 6, 0, // Skip to: 17098
/* 15324 */   MCD::OPC_Decode, 130, 5, 50, // Opcode: ADDQH_R_W
/* 15328 */   MCD::OPC_FilterValue, 19, 9, 0, 0, // Skip to: 15342
/* 15333 */   MCD::OPC_CheckPredicate, 69, 224, 6, 0, // Skip to: 17098
/* 15338 */   MCD::OPC_Decode, 237, 19, 50, // Opcode: SUBQH_R_W
/* 15342 */   MCD::OPC_FilterValue, 22, 9, 0, 0, // Skip to: 15356
/* 15347 */   MCD::OPC_CheckPredicate, 69, 210, 6, 0, // Skip to: 17098
/* 15352 */   MCD::OPC_Decode, 191, 16, 50, // Opcode: MULQ_S_W
/* 15356 */   MCD::OPC_FilterValue, 23, 201, 6, 0, // Skip to: 17098
/* 15361 */   MCD::OPC_CheckPredicate, 69, 196, 6, 0, // Skip to: 17098
/* 15366 */   MCD::OPC_Decode, 187, 16, 50, // Opcode: MULQ_RS_W
/* 15370 */   MCD::OPC_FilterValue, 25, 17, 0, 0, // Skip to: 15392
/* 15375 */   MCD::OPC_CheckPredicate, 70, 182, 6, 0, // Skip to: 17098
/* 15380 */   MCD::OPC_CheckField, 6, 1, 0, 175, 6, 0, // Skip to: 17098
/* 15387 */   MCD::OPC_Decode, 150, 14, 197, 2, // Opcode: LWLE
/* 15392 */   MCD::OPC_FilterValue, 26, 17, 0, 0, // Skip to: 15414
/* 15397 */   MCD::OPC_CheckPredicate, 70, 160, 6, 0, // Skip to: 17098
/* 15402 */   MCD::OPC_CheckField, 6, 1, 0, 153, 6, 0, // Skip to: 17098
/* 15409 */   MCD::OPC_Decode, 161, 14, 197, 2, // Opcode: LWRE
/* 15414 */   MCD::OPC_FilterValue, 27, 17, 0, 0, // Skip to: 15436
/* 15419 */   MCD::OPC_CheckPredicate, 44, 138, 6, 0, // Skip to: 17098
/* 15424 */   MCD::OPC_CheckField, 6, 1, 0, 131, 6, 0, // Skip to: 17098
/* 15431 */   MCD::OPC_Decode, 190, 7, 198, 2, // Opcode: CACHEE
/* 15436 */   MCD::OPC_FilterValue, 28, 17, 0, 0, // Skip to: 15458
/* 15441 */   MCD::OPC_CheckPredicate, 44, 116, 6, 0, // Skip to: 17098
/* 15446 */   MCD::OPC_CheckField, 6, 1, 0, 109, 6, 0, // Skip to: 17098
/* 15453 */   MCD::OPC_Decode, 142, 18, 197, 2, // Opcode: SBE
/* 15458 */   MCD::OPC_FilterValue, 29, 17, 0, 0, // Skip to: 15480
/* 15463 */   MCD::OPC_CheckPredicate, 44, 94, 6, 0, // Skip to: 17098
/* 15468 */   MCD::OPC_CheckField, 6, 1, 0, 87, 6, 0, // Skip to: 17098
/* 15475 */   MCD::OPC_Decode, 207, 18, 197, 2, // Opcode: SHE
/* 15480 */   MCD::OPC_FilterValue, 30, 17, 0, 0, // Skip to: 15502
/* 15485 */   MCD::OPC_CheckPredicate, 44, 72, 6, 0, // Skip to: 17098
/* 15490 */   MCD::OPC_CheckField, 6, 1, 0, 65, 6, 0, // Skip to: 17098
/* 15497 */   MCD::OPC_Decode, 151, 18, 197, 2, // Opcode: SCE
/* 15502 */   MCD::OPC_FilterValue, 31, 17, 0, 0, // Skip to: 15524
/* 15507 */   MCD::OPC_CheckPredicate, 44, 50, 6, 0, // Skip to: 17098
/* 15512 */   MCD::OPC_CheckField, 6, 1, 0, 43, 6, 0, // Skip to: 17098
/* 15519 */   MCD::OPC_Decode, 177, 20, 197, 2, // Opcode: SWE
/* 15524 */   MCD::OPC_FilterValue, 32, 69, 0, 0, // Skip to: 15598
/* 15529 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 15532 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 15554
/* 15537 */   MCD::OPC_CheckPredicate, 28, 20, 6, 0, // Skip to: 17098
/* 15542 */   MCD::OPC_CheckField, 21, 5, 0, 13, 6, 0, // Skip to: 17098
/* 15549 */   MCD::OPC_Decode, 174, 21, 193, 2, // Opcode: WSBH
/* 15554 */   MCD::OPC_FilterValue, 16, 17, 0, 0, // Skip to: 15576
/* 15559 */   MCD::OPC_CheckPredicate, 28, 254, 5, 0, // Skip to: 17098
/* 15564 */   MCD::OPC_CheckField, 21, 5, 0, 247, 5, 0, // Skip to: 17098
/* 15571 */   MCD::OPC_Decode, 177, 18, 193, 2, // Opcode: SEB
/* 15576 */   MCD::OPC_FilterValue, 24, 237, 5, 0, // Skip to: 17098
/* 15581 */   MCD::OPC_CheckPredicate, 28, 232, 5, 0, // Skip to: 17098
/* 15586 */   MCD::OPC_CheckField, 21, 5, 0, 225, 5, 0, // Skip to: 17098
/* 15593 */   MCD::OPC_Decode, 180, 18, 193, 2, // Opcode: SEH
/* 15598 */   MCD::OPC_FilterValue, 33, 17, 0, 0, // Skip to: 15620
/* 15603 */   MCD::OPC_CheckPredicate, 70, 210, 5, 0, // Skip to: 17098
/* 15608 */   MCD::OPC_CheckField, 6, 1, 0, 203, 5, 0, // Skip to: 17098
/* 15615 */   MCD::OPC_Decode, 181, 20, 197, 2, // Opcode: SWLE
/* 15620 */   MCD::OPC_FilterValue, 34, 17, 0, 0, // Skip to: 15642
/* 15625 */   MCD::OPC_CheckPredicate, 70, 188, 5, 0, // Skip to: 17098
/* 15630 */   MCD::OPC_CheckField, 6, 1, 0, 181, 5, 0, // Skip to: 17098
/* 15637 */   MCD::OPC_Decode, 190, 20, 197, 2, // Opcode: SWRE
/* 15642 */   MCD::OPC_FilterValue, 35, 17, 0, 0, // Skip to: 15664
/* 15647 */   MCD::OPC_CheckPredicate, 44, 166, 5, 0, // Skip to: 17098
/* 15652 */   MCD::OPC_CheckField, 6, 1, 0, 159, 5, 0, // Skip to: 17098
/* 15659 */   MCD::OPC_Decode, 198, 17, 198, 2, // Opcode: PREFE
/* 15664 */   MCD::OPC_FilterValue, 40, 17, 0, 0, // Skip to: 15686
/* 15669 */   MCD::OPC_CheckPredicate, 44, 144, 5, 0, // Skip to: 17098
/* 15674 */   MCD::OPC_CheckField, 6, 1, 0, 137, 5, 0, // Skip to: 17098
/* 15681 */   MCD::OPC_Decode, 199, 13, 197, 2, // Opcode: LBuE
/* 15686 */   MCD::OPC_FilterValue, 41, 17, 0, 0, // Skip to: 15708
/* 15691 */   MCD::OPC_CheckPredicate, 44, 122, 5, 0, // Skip to: 17098
/* 15696 */   MCD::OPC_CheckField, 6, 1, 0, 115, 5, 0, // Skip to: 17098
/* 15703 */   MCD::OPC_Decode, 237, 13, 197, 2, // Opcode: LHuE
/* 15708 */   MCD::OPC_FilterValue, 44, 17, 0, 0, // Skip to: 15730
/* 15713 */   MCD::OPC_CheckPredicate, 44, 100, 5, 0, // Skip to: 17098
/* 15718 */   MCD::OPC_CheckField, 6, 1, 0, 93, 5, 0, // Skip to: 17098
/* 15725 */   MCD::OPC_Decode, 189, 13, 197, 2, // Opcode: LBE
/* 15730 */   MCD::OPC_FilterValue, 45, 17, 0, 0, // Skip to: 15752
/* 15735 */   MCD::OPC_CheckPredicate, 44, 78, 5, 0, // Skip to: 17098
/* 15740 */   MCD::OPC_CheckField, 6, 1, 0, 71, 5, 0, // Skip to: 17098
/* 15747 */   MCD::OPC_Decode, 229, 13, 197, 2, // Opcode: LHE
/* 15752 */   MCD::OPC_FilterValue, 46, 17, 0, 0, // Skip to: 15774
/* 15757 */   MCD::OPC_CheckPredicate, 44, 56, 5, 0, // Skip to: 17098
/* 15762 */   MCD::OPC_CheckField, 6, 1, 0, 49, 5, 0, // Skip to: 17098
/* 15769 */   MCD::OPC_Decode, 247, 13, 197, 2, // Opcode: LLE
/* 15774 */   MCD::OPC_FilterValue, 47, 17, 0, 0, // Skip to: 15796
/* 15779 */   MCD::OPC_CheckPredicate, 44, 34, 5, 0, // Skip to: 17098
/* 15784 */   MCD::OPC_CheckField, 6, 1, 0, 27, 5, 0, // Skip to: 17098
/* 15791 */   MCD::OPC_Decode, 145, 14, 197, 2, // Opcode: LWE
/* 15796 */   MCD::OPC_FilterValue, 48, 231, 1, 0, // Skip to: 16288
/* 15801 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 15804 */   MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 15826
/* 15809 */   MCD::OPC_CheckPredicate, 69, 4, 5, 0, // Skip to: 17098
/* 15814 */   MCD::OPC_CheckField, 13, 3, 0, 253, 4, 0, // Skip to: 17098
/* 15821 */   MCD::OPC_Decode, 211, 10, 232, 1, // Opcode: DPA_W_PH
/* 15826 */   MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 15848
/* 15831 */   MCD::OPC_CheckPredicate, 69, 238, 4, 0, // Skip to: 17098
/* 15836 */   MCD::OPC_CheckField, 13, 3, 0, 231, 4, 0, // Skip to: 17098
/* 15843 */   MCD::OPC_Decode, 234, 10, 232, 1, // Opcode: DPS_W_PH
/* 15848 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 15870
/* 15853 */   MCD::OPC_CheckPredicate, 69, 216, 4, 0, // Skip to: 17098
/* 15858 */   MCD::OPC_CheckField, 13, 3, 0, 209, 4, 0, // Skip to: 17098
/* 15865 */   MCD::OPC_Decode, 197, 16, 232, 1, // Opcode: MULSA_W_PH
/* 15870 */   MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 15892
/* 15875 */   MCD::OPC_CheckPredicate, 37, 194, 4, 0, // Skip to: 17098
/* 15880 */   MCD::OPC_CheckField, 13, 3, 0, 187, 4, 0, // Skip to: 17098
/* 15887 */   MCD::OPC_Decode, 205, 10, 232, 1, // Opcode: DPAU_H_QBL
/* 15892 */   MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 15914
/* 15897 */   MCD::OPC_CheckPredicate, 37, 172, 4, 0, // Skip to: 17098
/* 15902 */   MCD::OPC_CheckField, 13, 3, 0, 165, 4, 0, // Skip to: 17098
/* 15909 */   MCD::OPC_Decode, 203, 10, 232, 1, // Opcode: DPAQ_S_W_PH
/* 15914 */   MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 15936
/* 15919 */   MCD::OPC_CheckPredicate, 37, 150, 4, 0, // Skip to: 17098
/* 15924 */   MCD::OPC_CheckField, 13, 3, 0, 143, 4, 0, // Skip to: 17098
/* 15931 */   MCD::OPC_Decode, 220, 10, 232, 1, // Opcode: DPSQ_S_W_PH
/* 15936 */   MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 15958
/* 15941 */   MCD::OPC_CheckPredicate, 37, 128, 4, 0, // Skip to: 17098
/* 15946 */   MCD::OPC_CheckField, 13, 3, 0, 121, 4, 0, // Skip to: 17098
/* 15953 */   MCD::OPC_Decode, 195, 16, 232, 1, // Opcode: MULSAQ_S_W_PH
/* 15958 */   MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 15980
/* 15963 */   MCD::OPC_CheckPredicate, 37, 106, 4, 0, // Skip to: 17098
/* 15968 */   MCD::OPC_CheckField, 13, 3, 0, 99, 4, 0, // Skip to: 17098
/* 15975 */   MCD::OPC_Decode, 207, 10, 232, 1, // Opcode: DPAU_H_QBR
/* 15980 */   MCD::OPC_FilterValue, 8, 17, 0, 0, // Skip to: 16002
/* 15985 */   MCD::OPC_CheckPredicate, 69, 84, 4, 0, // Skip to: 17098
/* 15990 */   MCD::OPC_CheckField, 13, 3, 0, 77, 4, 0, // Skip to: 17098
/* 15997 */   MCD::OPC_Decode, 209, 10, 232, 1, // Opcode: DPAX_W_PH
/* 16002 */   MCD::OPC_FilterValue, 9, 17, 0, 0, // Skip to: 16024
/* 16007 */   MCD::OPC_CheckPredicate, 69, 62, 4, 0, // Skip to: 17098
/* 16012 */   MCD::OPC_CheckField, 13, 3, 0, 55, 4, 0, // Skip to: 17098
/* 16019 */   MCD::OPC_Decode, 232, 10, 232, 1, // Opcode: DPSX_W_PH
/* 16024 */   MCD::OPC_FilterValue, 11, 17, 0, 0, // Skip to: 16046
/* 16029 */   MCD::OPC_CheckPredicate, 37, 40, 4, 0, // Skip to: 17098
/* 16034 */   MCD::OPC_CheckField, 13, 3, 0, 33, 4, 0, // Skip to: 17098
/* 16041 */   MCD::OPC_Decode, 228, 10, 232, 1, // Opcode: DPSU_H_QBL
/* 16046 */   MCD::OPC_FilterValue, 12, 17, 0, 0, // Skip to: 16068
/* 16051 */   MCD::OPC_CheckPredicate, 37, 18, 4, 0, // Skip to: 17098
/* 16056 */   MCD::OPC_CheckField, 13, 3, 0, 11, 4, 0, // Skip to: 17098
/* 16063 */   MCD::OPC_Decode, 201, 10, 232, 1, // Opcode: DPAQ_SA_L_W
/* 16068 */   MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 16090
/* 16073 */   MCD::OPC_CheckPredicate, 37, 252, 3, 0, // Skip to: 17098
/* 16078 */   MCD::OPC_CheckField, 13, 3, 0, 245, 3, 0, // Skip to: 17098
/* 16085 */   MCD::OPC_Decode, 218, 10, 232, 1, // Opcode: DPSQ_SA_L_W
/* 16090 */   MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 16112
/* 16095 */   MCD::OPC_CheckPredicate, 37, 230, 3, 0, // Skip to: 17098
/* 16100 */   MCD::OPC_CheckField, 13, 3, 0, 223, 3, 0, // Skip to: 17098
/* 16107 */   MCD::OPC_Decode, 230, 10, 232, 1, // Opcode: DPSU_H_QBR
/* 16112 */   MCD::OPC_FilterValue, 16, 17, 0, 0, // Skip to: 16134
/* 16117 */   MCD::OPC_CheckPredicate, 37, 208, 3, 0, // Skip to: 17098
/* 16122 */   MCD::OPC_CheckField, 13, 3, 0, 201, 3, 0, // Skip to: 17098
/* 16129 */   MCD::OPC_Decode, 211, 14, 232, 1, // Opcode: MAQ_SA_W_PHL
/* 16134 */   MCD::OPC_FilterValue, 18, 17, 0, 0, // Skip to: 16156
/* 16139 */   MCD::OPC_CheckPredicate, 37, 186, 3, 0, // Skip to: 17098
/* 16144 */   MCD::OPC_CheckField, 13, 3, 0, 179, 3, 0, // Skip to: 17098
/* 16151 */   MCD::OPC_Decode, 213, 14, 232, 1, // Opcode: MAQ_SA_W_PHR
/* 16156 */   MCD::OPC_FilterValue, 20, 17, 0, 0, // Skip to: 16178
/* 16161 */   MCD::OPC_CheckPredicate, 37, 164, 3, 0, // Skip to: 17098
/* 16166 */   MCD::OPC_CheckField, 13, 3, 0, 157, 3, 0, // Skip to: 17098
/* 16173 */   MCD::OPC_Decode, 215, 14, 232, 1, // Opcode: MAQ_S_W_PHL
/* 16178 */   MCD::OPC_FilterValue, 22, 17, 0, 0, // Skip to: 16200
/* 16183 */   MCD::OPC_CheckPredicate, 37, 142, 3, 0, // Skip to: 17098
/* 16188 */   MCD::OPC_CheckField, 13, 3, 0, 135, 3, 0, // Skip to: 17098
/* 16195 */   MCD::OPC_Decode, 217, 14, 232, 1, // Opcode: MAQ_S_W_PHR
/* 16200 */   MCD::OPC_FilterValue, 24, 17, 0, 0, // Skip to: 16222
/* 16205 */   MCD::OPC_CheckPredicate, 69, 120, 3, 0, // Skip to: 17098
/* 16210 */   MCD::OPC_CheckField, 13, 3, 0, 113, 3, 0, // Skip to: 17098
/* 16217 */   MCD::OPC_Decode, 199, 10, 232, 1, // Opcode: DPAQX_S_W_PH
/* 16222 */   MCD::OPC_FilterValue, 25, 17, 0, 0, // Skip to: 16244
/* 16227 */   MCD::OPC_CheckPredicate, 69, 98, 3, 0, // Skip to: 17098
/* 16232 */   MCD::OPC_CheckField, 13, 3, 0, 91, 3, 0, // Skip to: 17098
/* 16239 */   MCD::OPC_Decode, 216, 10, 232, 1, // Opcode: DPSQX_S_W_PH
/* 16244 */   MCD::OPC_FilterValue, 26, 17, 0, 0, // Skip to: 16266
/* 16249 */   MCD::OPC_CheckPredicate, 69, 76, 3, 0, // Skip to: 17098
/* 16254 */   MCD::OPC_CheckField, 13, 3, 0, 69, 3, 0, // Skip to: 17098
/* 16261 */   MCD::OPC_Decode, 197, 10, 232, 1, // Opcode: DPAQX_SA_W_PH
/* 16266 */   MCD::OPC_FilterValue, 27, 59, 3, 0, // Skip to: 17098
/* 16271 */   MCD::OPC_CheckPredicate, 69, 54, 3, 0, // Skip to: 17098
/* 16276 */   MCD::OPC_CheckField, 13, 3, 0, 47, 3, 0, // Skip to: 17098
/* 16283 */   MCD::OPC_Decode, 214, 10, 232, 1, // Opcode: DPSQX_SA_W_PH
/* 16288 */   MCD::OPC_FilterValue, 49, 48, 0, 0, // Skip to: 16341
/* 16293 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 16296 */   MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 16311
/* 16301 */   MCD::OPC_CheckPredicate, 69, 24, 3, 0, // Skip to: 17098
/* 16306 */   MCD::OPC_Decode, 209, 5, 199, 2, // Opcode: APPEND
/* 16311 */   MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 16326
/* 16316 */   MCD::OPC_CheckPredicate, 69, 9, 3, 0, // Skip to: 17098
/* 16321 */   MCD::OPC_Decode, 204, 17, 199, 2, // Opcode: PREPEND
/* 16326 */   MCD::OPC_FilterValue, 16, 255, 2, 0, // Skip to: 17098
/* 16331 */   MCD::OPC_CheckPredicate, 69, 250, 2, 0, // Skip to: 17098
/* 16336 */   MCD::OPC_Decode, 253, 5, 199, 2, // Opcode: BALIGN
/* 16341 */   MCD::OPC_FilterValue, 56, 107, 1, 0, // Skip to: 16709
/* 16346 */   MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 16349 */   MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 16371
/* 16354 */   MCD::OPC_CheckPredicate, 37, 227, 2, 0, // Skip to: 17098
/* 16359 */   MCD::OPC_CheckField, 13, 3, 0, 220, 2, 0, // Skip to: 17098
/* 16366 */   MCD::OPC_Decode, 170, 11, 200, 2, // Opcode: EXTR_W
/* 16371 */   MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 16393
/* 16376 */   MCD::OPC_CheckPredicate, 37, 205, 2, 0, // Skip to: 17098
/* 16381 */   MCD::OPC_CheckField, 13, 3, 0, 198, 2, 0, // Skip to: 17098
/* 16388 */   MCD::OPC_Decode, 162, 11, 201, 2, // Opcode: EXTRV_W
/* 16393 */   MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 16415
/* 16398 */   MCD::OPC_CheckPredicate, 37, 183, 2, 0, // Skip to: 17098
/* 16403 */   MCD::OPC_CheckField, 13, 3, 0, 176, 2, 0, // Skip to: 17098
/* 16410 */   MCD::OPC_Decode, 148, 11, 200, 2, // Opcode: EXTP
/* 16415 */   MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 16437
/* 16420 */   MCD::OPC_CheckPredicate, 37, 161, 2, 0, // Skip to: 17098
/* 16425 */   MCD::OPC_CheckField, 13, 3, 0, 154, 2, 0, // Skip to: 17098
/* 16432 */   MCD::OPC_Decode, 153, 11, 201, 2, // Opcode: EXTPV
/* 16437 */   MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 16459
/* 16442 */   MCD::OPC_CheckPredicate, 37, 139, 2, 0, // Skip to: 17098
/* 16447 */   MCD::OPC_CheckField, 13, 3, 0, 132, 2, 0, // Skip to: 17098
/* 16454 */   MCD::OPC_Decode, 166, 11, 200, 2, // Opcode: EXTR_R_W
/* 16459 */   MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 16481
/* 16464 */   MCD::OPC_CheckPredicate, 37, 117, 2, 0, // Skip to: 17098
/* 16469 */   MCD::OPC_CheckField, 13, 3, 0, 110, 2, 0, // Skip to: 17098
/* 16476 */   MCD::OPC_Decode, 158, 11, 201, 2, // Opcode: EXTRV_R_W
/* 16481 */   MCD::OPC_FilterValue, 6, 17, 0, 0, // Skip to: 16503
/* 16486 */   MCD::OPC_CheckPredicate, 37, 95, 2, 0, // Skip to: 17098
/* 16491 */   MCD::OPC_CheckField, 13, 3, 0, 88, 2, 0, // Skip to: 17098
/* 16498 */   MCD::OPC_Decode, 164, 11, 200, 2, // Opcode: EXTR_RS_W
/* 16503 */   MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 16525
/* 16508 */   MCD::OPC_CheckPredicate, 37, 73, 2, 0, // Skip to: 17098
/* 16513 */   MCD::OPC_CheckField, 13, 3, 0, 66, 2, 0, // Skip to: 17098
/* 16520 */   MCD::OPC_Decode, 156, 11, 201, 2, // Opcode: EXTRV_RS_W
/* 16525 */   MCD::OPC_FilterValue, 10, 17, 0, 0, // Skip to: 16547
/* 16530 */   MCD::OPC_CheckPredicate, 37, 51, 2, 0, // Skip to: 17098
/* 16535 */   MCD::OPC_CheckField, 13, 3, 0, 44, 2, 0, // Skip to: 17098
/* 16542 */   MCD::OPC_Decode, 149, 11, 200, 2, // Opcode: EXTPDP
/* 16547 */   MCD::OPC_FilterValue, 11, 17, 0, 0, // Skip to: 16569
/* 16552 */   MCD::OPC_CheckPredicate, 37, 29, 2, 0, // Skip to: 17098
/* 16557 */   MCD::OPC_CheckField, 13, 3, 0, 22, 2, 0, // Skip to: 17098
/* 16564 */   MCD::OPC_Decode, 150, 11, 201, 2, // Opcode: EXTPDPV
/* 16569 */   MCD::OPC_FilterValue, 14, 17, 0, 0, // Skip to: 16591
/* 16574 */   MCD::OPC_CheckPredicate, 37, 7, 2, 0, // Skip to: 17098
/* 16579 */   MCD::OPC_CheckField, 13, 3, 0, 0, 2, 0, // Skip to: 17098
/* 16586 */   MCD::OPC_Decode, 168, 11, 200, 2, // Opcode: EXTR_S_H
/* 16591 */   MCD::OPC_FilterValue, 15, 17, 0, 0, // Skip to: 16613
/* 16596 */   MCD::OPC_CheckPredicate, 37, 241, 1, 0, // Skip to: 17098
/* 16601 */   MCD::OPC_CheckField, 13, 3, 0, 234, 1, 0, // Skip to: 17098
/* 16608 */   MCD::OPC_Decode, 160, 11, 201, 2, // Opcode: EXTRV_S_H
/* 16613 */   MCD::OPC_FilterValue, 18, 10, 0, 0, // Skip to: 16628
/* 16618 */   MCD::OPC_CheckPredicate, 37, 219, 1, 0, // Skip to: 17098
/* 16623 */   MCD::OPC_Decode, 208, 17, 202, 2, // Opcode: RDDSP
/* 16628 */   MCD::OPC_FilterValue, 19, 10, 0, 0, // Skip to: 16643
/* 16633 */   MCD::OPC_CheckPredicate, 40, 204, 1, 0, // Skip to: 17098
/* 16638 */   MCD::OPC_Decode, 171, 21, 203, 2, // Opcode: WRDSP
/* 16643 */   MCD::OPC_FilterValue, 26, 17, 0, 0, // Skip to: 16665
/* 16648 */   MCD::OPC_CheckPredicate, 37, 189, 1, 0, // Skip to: 17098
/* 16653 */   MCD::OPC_CheckField, 13, 7, 0, 182, 1, 0, // Skip to: 17098
/* 16660 */   MCD::OPC_Decode, 212, 18, 204, 2, // Opcode: SHILO
/* 16665 */   MCD::OPC_FilterValue, 27, 17, 0, 0, // Skip to: 16687
/* 16670 */   MCD::OPC_CheckPredicate, 37, 167, 1, 0, // Skip to: 17098
/* 16675 */   MCD::OPC_CheckField, 13, 8, 0, 160, 1, 0, // Skip to: 17098
/* 16682 */   MCD::OPC_Decode, 213, 18, 205, 2, // Opcode: SHILOV
/* 16687 */   MCD::OPC_FilterValue, 31, 150, 1, 0, // Skip to: 17098
/* 16692 */   MCD::OPC_CheckPredicate, 37, 145, 1, 0, // Skip to: 17098
/* 16697 */   MCD::OPC_CheckField, 13, 8, 0, 138, 1, 0, // Skip to: 17098
/* 16704 */   MCD::OPC_Decode, 158, 16, 205, 2, // Opcode: MTHLIP
/* 16709 */   MCD::OPC_FilterValue, 59, 128, 1, 0, // Skip to: 17098
/* 16714 */   MCD::OPC_CheckPredicate, 27, 123, 1, 0, // Skip to: 17098
/* 16719 */   MCD::OPC_CheckField, 21, 5, 0, 116, 1, 0, // Skip to: 17098
/* 16726 */   MCD::OPC_CheckField, 9, 2, 0, 109, 1, 0, // Skip to: 17098
/* 16733 */   MCD::OPC_Decode, 210, 17, 206, 2, // Opcode: RDHWR
/* 16738 */   MCD::OPC_FilterValue, 32, 10, 0, 0, // Skip to: 16753
/* 16743 */   MCD::OPC_CheckPredicate, 27, 94, 1, 0, // Skip to: 17098
/* 16748 */   MCD::OPC_Decode, 187, 13, 130, 1, // Opcode: LB
/* 16753 */   MCD::OPC_FilterValue, 33, 10, 0, 0, // Skip to: 16768
/* 16758 */   MCD::OPC_CheckPredicate, 27, 79, 1, 0, // Skip to: 17098
/* 16763 */   MCD::OPC_Decode, 227, 13, 130, 1, // Opcode: LH
/* 16768 */   MCD::OPC_FilterValue, 34, 10, 0, 0, // Skip to: 16783
/* 16773 */   MCD::OPC_CheckPredicate, 31, 64, 1, 0, // Skip to: 17098
/* 16778 */   MCD::OPC_Decode, 148, 14, 130, 1, // Opcode: LWL
/* 16783 */   MCD::OPC_FilterValue, 35, 10, 0, 0, // Skip to: 16798
/* 16788 */   MCD::OPC_CheckPredicate, 27, 49, 1, 0, // Skip to: 17098
/* 16793 */   MCD::OPC_Decode, 134, 14, 130, 1, // Opcode: LW
/* 16798 */   MCD::OPC_FilterValue, 36, 10, 0, 0, // Skip to: 16813
/* 16803 */   MCD::OPC_CheckPredicate, 27, 34, 1, 0, // Skip to: 17098
/* 16808 */   MCD::OPC_Decode, 197, 13, 130, 1, // Opcode: LBu
/* 16813 */   MCD::OPC_FilterValue, 37, 10, 0, 0, // Skip to: 16828
/* 16818 */   MCD::OPC_CheckPredicate, 27, 19, 1, 0, // Skip to: 17098
/* 16823 */   MCD::OPC_Decode, 235, 13, 130, 1, // Opcode: LHu
/* 16828 */   MCD::OPC_FilterValue, 38, 10, 0, 0, // Skip to: 16843
/* 16833 */   MCD::OPC_CheckPredicate, 31, 4, 1, 0, // Skip to: 17098
/* 16838 */   MCD::OPC_Decode, 159, 14, 130, 1, // Opcode: LWR
/* 16843 */   MCD::OPC_FilterValue, 40, 10, 0, 0, // Skip to: 16858
/* 16848 */   MCD::OPC_CheckPredicate, 27, 245, 0, 0, // Skip to: 17098
/* 16853 */   MCD::OPC_Decode, 138, 18, 130, 1, // Opcode: SB
/* 16858 */   MCD::OPC_FilterValue, 41, 10, 0, 0, // Skip to: 16873
/* 16863 */   MCD::OPC_CheckPredicate, 27, 230, 0, 0, // Skip to: 17098
/* 16868 */   MCD::OPC_Decode, 203, 18, 130, 1, // Opcode: SH
/* 16873 */   MCD::OPC_FilterValue, 42, 10, 0, 0, // Skip to: 16888
/* 16878 */   MCD::OPC_CheckPredicate, 31, 215, 0, 0, // Skip to: 17098
/* 16883 */   MCD::OPC_Decode, 179, 20, 130, 1, // Opcode: SWL
/* 16888 */   MCD::OPC_FilterValue, 43, 10, 0, 0, // Skip to: 16903
/* 16893 */   MCD::OPC_CheckPredicate, 27, 200, 0, 0, // Skip to: 17098
/* 16898 */   MCD::OPC_Decode, 165, 20, 130, 1, // Opcode: SW
/* 16903 */   MCD::OPC_FilterValue, 46, 10, 0, 0, // Skip to: 16918
/* 16908 */   MCD::OPC_CheckPredicate, 31, 185, 0, 0, // Skip to: 17098
/* 16913 */   MCD::OPC_Decode, 188, 20, 130, 1, // Opcode: SWR
/* 16918 */   MCD::OPC_FilterValue, 47, 10, 0, 0, // Skip to: 16933
/* 16923 */   MCD::OPC_CheckPredicate, 71, 170, 0, 0, // Skip to: 17098
/* 16928 */   MCD::OPC_Decode, 189, 7, 207, 2, // Opcode: CACHE
/* 16933 */   MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 16948
/* 16938 */   MCD::OPC_CheckPredicate, 72, 155, 0, 0, // Skip to: 17098
/* 16943 */   MCD::OPC_Decode, 242, 13, 130, 1, // Opcode: LL
/* 16948 */   MCD::OPC_FilterValue, 49, 10, 0, 0, // Skip to: 16963
/* 16953 */   MCD::OPC_CheckPredicate, 47, 140, 0, 0, // Skip to: 17098
/* 16958 */   MCD::OPC_Decode, 137, 14, 208, 2, // Opcode: LWC1
/* 16963 */   MCD::OPC_FilterValue, 50, 10, 0, 0, // Skip to: 16978
/* 16968 */   MCD::OPC_CheckPredicate, 31, 125, 0, 0, // Skip to: 17098
/* 16973 */   MCD::OPC_Decode, 139, 14, 209, 2, // Opcode: LWC2
/* 16978 */   MCD::OPC_FilterValue, 51, 10, 0, 0, // Skip to: 16993
/* 16983 */   MCD::OPC_CheckPredicate, 71, 110, 0, 0, // Skip to: 17098
/* 16988 */   MCD::OPC_Decode, 197, 17, 207, 2, // Opcode: PREF
/* 16993 */   MCD::OPC_FilterValue, 53, 10, 0, 0, // Skip to: 17008
/* 16998 */   MCD::OPC_CheckPredicate, 57, 95, 0, 0, // Skip to: 17098
/* 17003 */   MCD::OPC_Decode, 203, 13, 208, 2, // Opcode: LDC1
/* 17008 */   MCD::OPC_FilterValue, 54, 10, 0, 0, // Skip to: 17023
/* 17013 */   MCD::OPC_CheckPredicate, 39, 80, 0, 0, // Skip to: 17098
/* 17018 */   MCD::OPC_Decode, 207, 13, 209, 2, // Opcode: LDC2
/* 17023 */   MCD::OPC_FilterValue, 56, 10, 0, 0, // Skip to: 17038
/* 17028 */   MCD::OPC_CheckPredicate, 72, 65, 0, 0, // Skip to: 17098
/* 17033 */   MCD::OPC_Decode, 146, 18, 130, 1, // Opcode: SC
/* 17038 */   MCD::OPC_FilterValue, 57, 10, 0, 0, // Skip to: 17053
/* 17043 */   MCD::OPC_CheckPredicate, 47, 50, 0, 0, // Skip to: 17098
/* 17048 */   MCD::OPC_Decode, 169, 20, 208, 2, // Opcode: SWC1
/* 17053 */   MCD::OPC_FilterValue, 58, 10, 0, 0, // Skip to: 17068
/* 17058 */   MCD::OPC_CheckPredicate, 31, 35, 0, 0, // Skip to: 17098
/* 17063 */   MCD::OPC_Decode, 171, 20, 209, 2, // Opcode: SWC2
/* 17068 */   MCD::OPC_FilterValue, 61, 10, 0, 0, // Skip to: 17083
/* 17073 */   MCD::OPC_CheckPredicate, 57, 20, 0, 0, // Skip to: 17098
/* 17078 */   MCD::OPC_Decode, 163, 18, 208, 2, // Opcode: SDC1
/* 17083 */   MCD::OPC_FilterValue, 62, 10, 0, 0, // Skip to: 17098
/* 17088 */   MCD::OPC_CheckPredicate, 39, 5, 0, 0, // Skip to: 17098
/* 17093 */   MCD::OPC_Decode, 167, 18, 209, 2, // Opcode: SDC2
/* 17098 */   MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32_64_PTR6432[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 24
/* 8 */       MCD::OPC_CheckPredicate, 73, 41, 0, 0, // Skip to: 54
/* 13 */      MCD::OPC_CheckField, 0, 21, 8, 34, 0, 0, // Skip to: 54
/* 20 */      MCD::OPC_Decode, 170, 13, 13, // Opcode: JR64
/* 24 */      MCD::OPC_FilterValue, 48, 10, 0, 0, // Skip to: 39
/* 29 */      MCD::OPC_CheckPredicate, 74, 20, 0, 0, // Skip to: 54
/* 34 */      MCD::OPC_Decode, 243, 13, 130, 1, // Opcode: LL64
/* 39 */      MCD::OPC_FilterValue, 56, 10, 0, 0, // Skip to: 54
/* 44 */      MCD::OPC_CheckPredicate, 74, 5, 0, 0, // Skip to: 54
/* 49 */      MCD::OPC_Decode, 147, 18, 130, 1, // Opcode: SC64
/* 54 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32r6_64r632[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 14, 2, 0, // Skip to: 534
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 33
/* 16 */      MCD::OPC_CheckPredicate, 75, 133, 9, 0, // Skip to: 2458
/* 21 */      MCD::OPC_CheckField, 8, 3, 0, 126, 9, 0, // Skip to: 2458
/* 28 */      MCD::OPC_Decode, 254, 13, 174, 1, // Opcode: LSA_R6
/* 33 */      MCD::OPC_FilterValue, 9, 17, 0, 0, // Skip to: 55
/* 38 */      MCD::OPC_CheckPredicate, 75, 111, 9, 0, // Skip to: 2458
/* 43 */      MCD::OPC_CheckField, 6, 15, 16, 104, 9, 0, // Skip to: 2458
/* 50 */      MCD::OPC_Decode, 178, 13, 175, 1, // Opcode: JR_HB_R6
/* 55 */      MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 70
/* 60 */      MCD::OPC_CheckPredicate, 76, 89, 9, 0, // Skip to: 2458
/* 65 */      MCD::OPC_Decode, 162, 18, 177, 1, // Opcode: SDBBP_R6
/* 70 */      MCD::OPC_FilterValue, 16, 23, 0, 0, // Skip to: 98
/* 75 */      MCD::OPC_CheckPredicate, 75, 74, 9, 0, // Skip to: 2458
/* 80 */      MCD::OPC_CheckField, 16, 5, 0, 67, 9, 0, // Skip to: 2458
/* 87 */      MCD::OPC_CheckField, 6, 5, 1, 60, 9, 0, // Skip to: 2458
/* 94 */      MCD::OPC_Decode, 137, 8, 14, // Opcode: CLZ_R6
/* 98 */      MCD::OPC_FilterValue, 17, 23, 0, 0, // Skip to: 126
/* 103 */     MCD::OPC_CheckPredicate, 75, 46, 9, 0, // Skip to: 2458
/* 108 */     MCD::OPC_CheckField, 16, 5, 0, 39, 9, 0, // Skip to: 2458
/* 115 */     MCD::OPC_CheckField, 6, 5, 1, 32, 9, 0, // Skip to: 2458
/* 122 */     MCD::OPC_Decode, 245, 7, 14, // Opcode: CLO_R6
/* 126 */     MCD::OPC_FilterValue, 18, 23, 0, 0, // Skip to: 154
/* 131 */     MCD::OPC_CheckPredicate, 77, 18, 9, 0, // Skip to: 2458
/* 136 */     MCD::OPC_CheckField, 16, 5, 0, 11, 9, 0, // Skip to: 2458
/* 143 */     MCD::OPC_CheckField, 6, 5, 1, 4, 9, 0, // Skip to: 2458
/* 150 */     MCD::OPC_Decode, 135, 10, 15, // Opcode: DCLZ_R6
/* 154 */     MCD::OPC_FilterValue, 19, 23, 0, 0, // Skip to: 182
/* 159 */     MCD::OPC_CheckPredicate, 77, 246, 8, 0, // Skip to: 2458
/* 164 */     MCD::OPC_CheckField, 16, 5, 0, 239, 8, 0, // Skip to: 2458
/* 171 */     MCD::OPC_CheckField, 6, 5, 1, 232, 8, 0, // Skip to: 2458
/* 178 */     MCD::OPC_Decode, 133, 10, 15, // Opcode: DCLO_R6
/* 182 */     MCD::OPC_FilterValue, 21, 17, 0, 0, // Skip to: 204
/* 187 */     MCD::OPC_CheckPredicate, 77, 218, 8, 0, // Skip to: 2458
/* 192 */     MCD::OPC_CheckField, 8, 3, 0, 211, 8, 0, // Skip to: 2458
/* 199 */     MCD::OPC_Decode, 164, 10, 183, 1, // Opcode: DLSA_R6
/* 204 */     MCD::OPC_FilterValue, 24, 31, 0, 0, // Skip to: 240
/* 209 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 212 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 226
/* 217 */     MCD::OPC_CheckPredicate, 76, 188, 8, 0, // Skip to: 2458
/* 222 */     MCD::OPC_Decode, 219, 16, 50, // Opcode: MUL_R6
/* 226 */     MCD::OPC_FilterValue, 3, 179, 8, 0, // Skip to: 2458
/* 231 */     MCD::OPC_CheckPredicate, 76, 174, 8, 0, // Skip to: 2458
/* 236 */     MCD::OPC_Decode, 172, 16, 50, // Opcode: MUH
/* 240 */     MCD::OPC_FilterValue, 25, 31, 0, 0, // Skip to: 276
/* 245 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 248 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 262
/* 253 */     MCD::OPC_CheckPredicate, 76, 152, 8, 0, // Skip to: 2458
/* 258 */     MCD::OPC_Decode, 207, 16, 50, // Opcode: MULU
/* 262 */     MCD::OPC_FilterValue, 3, 143, 8, 0, // Skip to: 2458
/* 267 */     MCD::OPC_CheckPredicate, 76, 138, 8, 0, // Skip to: 2458
/* 272 */     MCD::OPC_Decode, 173, 16, 50, // Opcode: MUHU
/* 276 */     MCD::OPC_FilterValue, 26, 31, 0, 0, // Skip to: 312
/* 281 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 284 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 298
/* 289 */     MCD::OPC_CheckPredicate, 76, 116, 8, 0, // Skip to: 2458
/* 294 */     MCD::OPC_Decode, 149, 10, 50, // Opcode: DIV
/* 298 */     MCD::OPC_FilterValue, 3, 107, 8, 0, // Skip to: 2458
/* 303 */     MCD::OPC_CheckPredicate, 76, 102, 8, 0, // Skip to: 2458
/* 308 */     MCD::OPC_Decode, 178, 15, 50, // Opcode: MOD
/* 312 */     MCD::OPC_FilterValue, 27, 31, 0, 0, // Skip to: 348
/* 317 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 320 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 334
/* 325 */     MCD::OPC_CheckPredicate, 76, 80, 8, 0, // Skip to: 2458
/* 330 */     MCD::OPC_Decode, 150, 10, 50, // Opcode: DIVU
/* 334 */     MCD::OPC_FilterValue, 3, 71, 8, 0, // Skip to: 2458
/* 339 */     MCD::OPC_CheckPredicate, 76, 66, 8, 0, // Skip to: 2458
/* 344 */     MCD::OPC_Decode, 181, 15, 50, // Opcode: MODU
/* 348 */     MCD::OPC_FilterValue, 28, 31, 0, 0, // Skip to: 384
/* 353 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 356 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 370
/* 361 */     MCD::OPC_CheckPredicate, 77, 44, 8, 0, // Skip to: 2458
/* 366 */     MCD::OPC_Decode, 184, 10, 12, // Opcode: DMUL_R6
/* 370 */     MCD::OPC_FilterValue, 3, 35, 8, 0, // Skip to: 2458
/* 375 */     MCD::OPC_CheckPredicate, 77, 30, 8, 0, // Skip to: 2458
/* 380 */     MCD::OPC_Decode, 178, 10, 12, // Opcode: DMUH
/* 384 */     MCD::OPC_FilterValue, 29, 31, 0, 0, // Skip to: 420
/* 389 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 392 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 406
/* 397 */     MCD::OPC_CheckPredicate, 77, 8, 8, 0, // Skip to: 2458
/* 402 */     MCD::OPC_Decode, 183, 10, 12, // Opcode: DMULU
/* 406 */     MCD::OPC_FilterValue, 3, 255, 7, 0, // Skip to: 2458
/* 411 */     MCD::OPC_CheckPredicate, 77, 250, 7, 0, // Skip to: 2458
/* 416 */     MCD::OPC_Decode, 179, 10, 12, // Opcode: DMUHU
/* 420 */     MCD::OPC_FilterValue, 30, 31, 0, 0, // Skip to: 456
/* 425 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 428 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 442
/* 433 */     MCD::OPC_CheckPredicate, 77, 228, 7, 0, // Skip to: 2458
/* 438 */     MCD::OPC_Decode, 136, 10, 12, // Opcode: DDIV
/* 442 */     MCD::OPC_FilterValue, 3, 219, 7, 0, // Skip to: 2458
/* 447 */     MCD::OPC_CheckPredicate, 77, 214, 7, 0, // Skip to: 2458
/* 452 */     MCD::OPC_Decode, 170, 10, 12, // Opcode: DMOD
/* 456 */     MCD::OPC_FilterValue, 31, 31, 0, 0, // Skip to: 492
/* 461 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 464 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 478
/* 469 */     MCD::OPC_CheckPredicate, 77, 192, 7, 0, // Skip to: 2458
/* 474 */     MCD::OPC_Decode, 137, 10, 12, // Opcode: DDIVU
/* 478 */     MCD::OPC_FilterValue, 3, 183, 7, 0, // Skip to: 2458
/* 483 */     MCD::OPC_CheckPredicate, 77, 178, 7, 0, // Skip to: 2458
/* 488 */     MCD::OPC_Decode, 171, 10, 12, // Opcode: DMODU
/* 492 */     MCD::OPC_FilterValue, 53, 16, 0, 0, // Skip to: 513
/* 497 */     MCD::OPC_CheckPredicate, 78, 164, 7, 0, // Skip to: 2458
/* 502 */     MCD::OPC_CheckField, 6, 5, 0, 157, 7, 0, // Skip to: 2458
/* 509 */     MCD::OPC_Decode, 183, 18, 50, // Opcode: SELEQZ
/* 513 */     MCD::OPC_FilterValue, 55, 148, 7, 0, // Skip to: 2458
/* 518 */     MCD::OPC_CheckPredicate, 78, 143, 7, 0, // Skip to: 2458
/* 523 */     MCD::OPC_CheckField, 6, 5, 0, 136, 7, 0, // Skip to: 2458
/* 530 */     MCD::OPC_Decode, 190, 18, 50, // Opcode: SELNEZ
/* 534 */     MCD::OPC_FilterValue, 1, 77, 0, 0, // Skip to: 616
/* 539 */     MCD::OPC_ExtractField, 16, 5,  // Inst{20-16} ...
/* 542 */     MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 557
/* 547 */     MCD::OPC_CheckPredicate, 77, 114, 7, 0, // Skip to: 2458
/* 552 */     MCD::OPC_Decode, 255, 9, 210, 2, // Opcode: DAHI
/* 557 */     MCD::OPC_FilterValue, 17, 17, 0, 0, // Skip to: 579
/* 562 */     MCD::OPC_CheckPredicate, 75, 99, 7, 0, // Skip to: 2458
/* 567 */     MCD::OPC_CheckField, 21, 5, 0, 92, 7, 0, // Skip to: 2458
/* 574 */     MCD::OPC_Decode, 250, 5, 187, 1, // Opcode: BAL
/* 579 */     MCD::OPC_FilterValue, 23, 17, 0, 0, // Skip to: 601
/* 584 */     MCD::OPC_CheckPredicate, 76, 77, 7, 0, // Skip to: 2458
/* 589 */     MCD::OPC_CheckField, 21, 5, 0, 70, 7, 0, // Skip to: 2458
/* 596 */     MCD::OPC_Decode, 134, 19, 211, 2, // Opcode: SIGRIE
/* 601 */     MCD::OPC_FilterValue, 30, 60, 7, 0, // Skip to: 2458
/* 606 */     MCD::OPC_CheckPredicate, 77, 55, 7, 0, // Skip to: 2458
/* 611 */     MCD::OPC_Decode, 129, 10, 210, 2, // Opcode: DATI
/* 616 */     MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 631
/* 621 */     MCD::OPC_CheckPredicate, 76, 40, 7, 0, // Skip to: 2458
/* 626 */     MCD::OPC_Decode, 180, 6, 212, 2, // Opcode: BGEZALC
/* 631 */     MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 646
/* 636 */     MCD::OPC_CheckPredicate, 76, 25, 7, 0, // Skip to: 2458
/* 641 */     MCD::OPC_Decode, 237, 6, 213, 2, // Opcode: BLTZALC
/* 646 */     MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 661
/* 651 */     MCD::OPC_CheckPredicate, 76, 10, 7, 0, // Skip to: 2458
/* 656 */     MCD::OPC_Decode, 158, 6, 214, 2, // Opcode: BEQC
/* 661 */     MCD::OPC_FilterValue, 15, 10, 0, 0, // Skip to: 676
/* 666 */     MCD::OPC_CheckPredicate, 75, 251, 6, 0, // Skip to: 2458
/* 671 */     MCD::OPC_Decode, 219, 5, 192, 1, // Opcode: AUI
/* 676 */     MCD::OPC_FilterValue, 16, 45, 0, 0, // Skip to: 726
/* 681 */     MCD::OPC_ExtractField, 0, 16,  // Inst{15-0} ...
/* 684 */     MCD::OPC_FilterValue, 4, 16, 0, 0, // Skip to: 705
/* 689 */     MCD::OPC_CheckPredicate, 75, 228, 6, 0, // Skip to: 2458
/* 694 */     MCD::OPC_CheckField, 21, 5, 11, 221, 6, 0, // Skip to: 2458
/* 701 */     MCD::OPC_Decode, 144, 11, 81, // Opcode: EVP
/* 705 */     MCD::OPC_FilterValue, 36, 212, 6, 0, // Skip to: 2458
/* 710 */     MCD::OPC_CheckPredicate, 75, 207, 6, 0, // Skip to: 2458
/* 715 */     MCD::OPC_CheckField, 21, 5, 11, 200, 6, 0, // Skip to: 2458
/* 722 */     MCD::OPC_Decode, 255, 10, 81, // Opcode: DVP
/* 726 */     MCD::OPC_FilterValue, 17, 135, 3, 0, // Skip to: 1634
/* 731 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 734 */     MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 749
/* 739 */     MCD::OPC_CheckPredicate, 79, 178, 6, 0, // Skip to: 2458
/* 744 */     MCD::OPC_Decode, 133, 6, 215, 2, // Opcode: BC1EQZ
/* 749 */     MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 764
/* 754 */     MCD::OPC_CheckPredicate, 79, 163, 6, 0, // Skip to: 2458
/* 759 */     MCD::OPC_Decode, 138, 6, 215, 2, // Opcode: BC1NEZ
/* 764 */     MCD::OPC_FilterValue, 16, 182, 0, 0, // Skip to: 951
/* 769 */     MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 772 */     MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 787
/* 777 */     MCD::OPC_CheckPredicate, 79, 140, 6, 0, // Skip to: 2458
/* 782 */     MCD::OPC_Decode, 199, 18, 216, 2, // Opcode: SEL_S
/* 787 */     MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 802
/* 792 */     MCD::OPC_CheckPredicate, 79, 125, 6, 0, // Skip to: 2458
/* 797 */     MCD::OPC_Decode, 188, 18, 207, 1, // Opcode: SELEQZ_S
/* 802 */     MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 817
/* 807 */     MCD::OPC_CheckPredicate, 79, 110, 6, 0, // Skip to: 2458
/* 812 */     MCD::OPC_Decode, 195, 18, 207, 1, // Opcode: SELNEZ_S
/* 817 */     MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 832
/* 822 */     MCD::OPC_CheckPredicate, 79, 95, 6, 0, // Skip to: 2458
/* 827 */     MCD::OPC_Decode, 189, 14, 217, 2, // Opcode: MADDF_S
/* 832 */     MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 847
/* 837 */     MCD::OPC_CheckPredicate, 79, 80, 6, 0, // Skip to: 2458
/* 842 */     MCD::OPC_Decode, 240, 15, 217, 2, // Opcode: MSUBF_S
/* 847 */     MCD::OPC_FilterValue, 26, 17, 0, 0, // Skip to: 869
/* 852 */     MCD::OPC_CheckPredicate, 79, 65, 6, 0, // Skip to: 2458
/* 857 */     MCD::OPC_CheckField, 16, 5, 0, 58, 6, 0, // Skip to: 2458
/* 864 */     MCD::OPC_Decode, 231, 17, 208, 1, // Opcode: RINT_S
/* 869 */     MCD::OPC_FilterValue, 27, 17, 0, 0, // Skip to: 891
/* 874 */     MCD::OPC_CheckPredicate, 79, 43, 6, 0, // Skip to: 2458
/* 879 */     MCD::OPC_CheckField, 16, 5, 0, 36, 6, 0, // Skip to: 2458
/* 886 */     MCD::OPC_Decode, 224, 7, 208, 1, // Opcode: CLASS_S
/* 891 */     MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 906
/* 896 */     MCD::OPC_CheckPredicate, 79, 21, 6, 0, // Skip to: 2458
/* 901 */     MCD::OPC_Decode, 168, 15, 207, 1, // Opcode: MIN_S
/* 906 */     MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 921
/* 911 */     MCD::OPC_CheckPredicate, 79, 6, 6, 0, // Skip to: 2458
/* 916 */     MCD::OPC_Decode, 237, 14, 207, 1, // Opcode: MAX_S
/* 921 */     MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 936
/* 926 */     MCD::OPC_CheckPredicate, 79, 247, 5, 0, // Skip to: 2458
/* 931 */     MCD::OPC_Decode, 152, 15, 207, 1, // Opcode: MINA_S
/* 936 */     MCD::OPC_FilterValue, 31, 237, 5, 0, // Skip to: 2458
/* 941 */     MCD::OPC_CheckPredicate, 79, 232, 5, 0, // Skip to: 2458
/* 946 */     MCD::OPC_Decode, 221, 14, 207, 1, // Opcode: MAXA_S
/* 951 */     MCD::OPC_FilterValue, 17, 182, 0, 0, // Skip to: 1138
/* 956 */     MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 959 */     MCD::OPC_FilterValue, 16, 10, 0, 0, // Skip to: 974
/* 964 */     MCD::OPC_CheckPredicate, 79, 209, 5, 0, // Skip to: 2458
/* 969 */     MCD::OPC_Decode, 197, 18, 218, 2, // Opcode: SEL_D
/* 974 */     MCD::OPC_FilterValue, 20, 10, 0, 0, // Skip to: 989
/* 979 */     MCD::OPC_CheckPredicate, 79, 194, 5, 0, // Skip to: 2458
/* 984 */     MCD::OPC_Decode, 185, 18, 219, 2, // Opcode: SELEQZ_D
/* 989 */     MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 1004
/* 994 */     MCD::OPC_CheckPredicate, 79, 179, 5, 0, // Skip to: 2458
/* 999 */     MCD::OPC_Decode, 192, 18, 219, 2, // Opcode: SELNEZ_D
/* 1004 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 1019
/* 1009 */    MCD::OPC_CheckPredicate, 79, 164, 5, 0, // Skip to: 2458
/* 1014 */    MCD::OPC_Decode, 187, 14, 218, 2, // Opcode: MADDF_D
/* 1019 */    MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 1034
/* 1024 */    MCD::OPC_CheckPredicate, 79, 149, 5, 0, // Skip to: 2458
/* 1029 */    MCD::OPC_Decode, 238, 15, 218, 2, // Opcode: MSUBF_D
/* 1034 */    MCD::OPC_FilterValue, 26, 17, 0, 0, // Skip to: 1056
/* 1039 */    MCD::OPC_CheckPredicate, 79, 134, 5, 0, // Skip to: 2458
/* 1044 */    MCD::OPC_CheckField, 16, 5, 0, 127, 5, 0, // Skip to: 2458
/* 1051 */    MCD::OPC_Decode, 229, 17, 219, 1, // Opcode: RINT_D
/* 1056 */    MCD::OPC_FilterValue, 27, 17, 0, 0, // Skip to: 1078
/* 1061 */    MCD::OPC_CheckPredicate, 79, 112, 5, 0, // Skip to: 2458
/* 1066 */    MCD::OPC_CheckField, 16, 5, 0, 105, 5, 0, // Skip to: 2458
/* 1073 */    MCD::OPC_Decode, 222, 7, 219, 1, // Opcode: CLASS_D
/* 1078 */    MCD::OPC_FilterValue, 28, 10, 0, 0, // Skip to: 1093
/* 1083 */    MCD::OPC_CheckPredicate, 79, 90, 5, 0, // Skip to: 2458
/* 1088 */    MCD::OPC_Decode, 166, 15, 219, 2, // Opcode: MIN_D
/* 1093 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 1108
/* 1098 */    MCD::OPC_CheckPredicate, 79, 75, 5, 0, // Skip to: 2458
/* 1103 */    MCD::OPC_Decode, 235, 14, 219, 2, // Opcode: MAX_D
/* 1108 */    MCD::OPC_FilterValue, 30, 10, 0, 0, // Skip to: 1123
/* 1113 */    MCD::OPC_CheckPredicate, 79, 60, 5, 0, // Skip to: 2458
/* 1118 */    MCD::OPC_Decode, 150, 15, 219, 2, // Opcode: MINA_D
/* 1123 */    MCD::OPC_FilterValue, 31, 50, 5, 0, // Skip to: 2458
/* 1128 */    MCD::OPC_CheckPredicate, 79, 45, 5, 0, // Skip to: 2458
/* 1133 */    MCD::OPC_Decode, 219, 14, 219, 2, // Opcode: MAXA_D
/* 1138 */    MCD::OPC_FilterValue, 20, 243, 0, 0, // Skip to: 1386
/* 1143 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 1146 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 1161
/* 1151 */    MCD::OPC_CheckPredicate, 79, 22, 5, 0, // Skip to: 2458
/* 1156 */    MCD::OPC_Decode, 165, 8, 220, 2, // Opcode: CMP_F_S
/* 1161 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 1176
/* 1166 */    MCD::OPC_CheckPredicate, 79, 7, 5, 0, // Skip to: 2458
/* 1171 */    MCD::OPC_Decode, 224, 8, 220, 2, // Opcode: CMP_UN_S
/* 1176 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1191
/* 1181 */    MCD::OPC_CheckPredicate, 79, 248, 4, 0, // Skip to: 2458
/* 1186 */    MCD::OPC_Decode, 162, 8, 220, 2, // Opcode: CMP_EQ_S
/* 1191 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1206
/* 1196 */    MCD::OPC_CheckPredicate, 79, 233, 4, 0, // Skip to: 2458
/* 1201 */    MCD::OPC_Decode, 212, 8, 220, 2, // Opcode: CMP_UEQ_S
/* 1206 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 1221
/* 1211 */    MCD::OPC_CheckPredicate, 79, 218, 4, 0, // Skip to: 2458
/* 1216 */    MCD::OPC_Decode, 176, 8, 220, 2, // Opcode: CMP_LT_S
/* 1221 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1236
/* 1226 */    MCD::OPC_CheckPredicate, 79, 203, 4, 0, // Skip to: 2458
/* 1231 */    MCD::OPC_Decode, 220, 8, 220, 2, // Opcode: CMP_ULT_S
/* 1236 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 1251
/* 1241 */    MCD::OPC_CheckPredicate, 79, 188, 4, 0, // Skip to: 2458
/* 1246 */    MCD::OPC_Decode, 170, 8, 220, 2, // Opcode: CMP_LE_S
/* 1251 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 1266
/* 1256 */    MCD::OPC_CheckPredicate, 79, 173, 4, 0, // Skip to: 2458
/* 1261 */    MCD::OPC_Decode, 216, 8, 220, 2, // Opcode: CMP_ULE_S
/* 1266 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 1281
/* 1271 */    MCD::OPC_CheckPredicate, 79, 158, 4, 0, // Skip to: 2458
/* 1276 */    MCD::OPC_Decode, 180, 8, 220, 2, // Opcode: CMP_SAF_S
/* 1281 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1296
/* 1286 */    MCD::OPC_CheckPredicate, 79, 143, 4, 0, // Skip to: 2458
/* 1291 */    MCD::OPC_Decode, 208, 8, 220, 2, // Opcode: CMP_SUN_S
/* 1296 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1311
/* 1301 */    MCD::OPC_CheckPredicate, 79, 128, 4, 0, // Skip to: 2458
/* 1306 */    MCD::OPC_Decode, 184, 8, 220, 2, // Opcode: CMP_SEQ_S
/* 1311 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1326
/* 1316 */    MCD::OPC_CheckPredicate, 79, 113, 4, 0, // Skip to: 2458
/* 1321 */    MCD::OPC_Decode, 196, 8, 220, 2, // Opcode: CMP_SUEQ_S
/* 1326 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 1341
/* 1331 */    MCD::OPC_CheckPredicate, 79, 98, 4, 0, // Skip to: 2458
/* 1336 */    MCD::OPC_Decode, 192, 8, 220, 2, // Opcode: CMP_SLT_S
/* 1341 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 1356
/* 1346 */    MCD::OPC_CheckPredicate, 79, 83, 4, 0, // Skip to: 2458
/* 1351 */    MCD::OPC_Decode, 204, 8, 220, 2, // Opcode: CMP_SULT_S
/* 1356 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 1371
/* 1361 */    MCD::OPC_CheckPredicate, 79, 68, 4, 0, // Skip to: 2458
/* 1366 */    MCD::OPC_Decode, 188, 8, 220, 2, // Opcode: CMP_SLE_S
/* 1371 */    MCD::OPC_FilterValue, 15, 58, 4, 0, // Skip to: 2458
/* 1376 */    MCD::OPC_CheckPredicate, 79, 53, 4, 0, // Skip to: 2458
/* 1381 */    MCD::OPC_Decode, 200, 8, 220, 2, // Opcode: CMP_SULE_S
/* 1386 */    MCD::OPC_FilterValue, 21, 43, 4, 0, // Skip to: 2458
/* 1391 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 1394 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 1409
/* 1399 */    MCD::OPC_CheckPredicate, 79, 30, 4, 0, // Skip to: 2458
/* 1404 */    MCD::OPC_Decode, 164, 8, 221, 2, // Opcode: CMP_F_D
/* 1409 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 1424
/* 1414 */    MCD::OPC_CheckPredicate, 79, 15, 4, 0, // Skip to: 2458
/* 1419 */    MCD::OPC_Decode, 222, 8, 221, 2, // Opcode: CMP_UN_D
/* 1424 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1439
/* 1429 */    MCD::OPC_CheckPredicate, 79, 0, 4, 0, // Skip to: 2458
/* 1434 */    MCD::OPC_Decode, 158, 8, 221, 2, // Opcode: CMP_EQ_D
/* 1439 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1454
/* 1444 */    MCD::OPC_CheckPredicate, 79, 241, 3, 0, // Skip to: 2458
/* 1449 */    MCD::OPC_Decode, 210, 8, 221, 2, // Opcode: CMP_UEQ_D
/* 1454 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 1469
/* 1459 */    MCD::OPC_CheckPredicate, 79, 226, 3, 0, // Skip to: 2458
/* 1464 */    MCD::OPC_Decode, 172, 8, 221, 2, // Opcode: CMP_LT_D
/* 1469 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1484
/* 1474 */    MCD::OPC_CheckPredicate, 79, 211, 3, 0, // Skip to: 2458
/* 1479 */    MCD::OPC_Decode, 218, 8, 221, 2, // Opcode: CMP_ULT_D
/* 1484 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 1499
/* 1489 */    MCD::OPC_CheckPredicate, 79, 196, 3, 0, // Skip to: 2458
/* 1494 */    MCD::OPC_Decode, 166, 8, 221, 2, // Opcode: CMP_LE_D
/* 1499 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 1514
/* 1504 */    MCD::OPC_CheckPredicate, 79, 181, 3, 0, // Skip to: 2458
/* 1509 */    MCD::OPC_Decode, 214, 8, 221, 2, // Opcode: CMP_ULE_D
/* 1514 */    MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 1529
/* 1519 */    MCD::OPC_CheckPredicate, 79, 166, 3, 0, // Skip to: 2458
/* 1524 */    MCD::OPC_Decode, 178, 8, 221, 2, // Opcode: CMP_SAF_D
/* 1529 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1544
/* 1534 */    MCD::OPC_CheckPredicate, 79, 151, 3, 0, // Skip to: 2458
/* 1539 */    MCD::OPC_Decode, 206, 8, 221, 2, // Opcode: CMP_SUN_D
/* 1544 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1559
/* 1549 */    MCD::OPC_CheckPredicate, 79, 136, 3, 0, // Skip to: 2458
/* 1554 */    MCD::OPC_Decode, 182, 8, 221, 2, // Opcode: CMP_SEQ_D
/* 1559 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1574
/* 1564 */    MCD::OPC_CheckPredicate, 79, 121, 3, 0, // Skip to: 2458
/* 1569 */    MCD::OPC_Decode, 194, 8, 221, 2, // Opcode: CMP_SUEQ_D
/* 1574 */    MCD::OPC_FilterValue, 12, 10, 0, 0, // Skip to: 1589
/* 1579 */    MCD::OPC_CheckPredicate, 79, 106, 3, 0, // Skip to: 2458
/* 1584 */    MCD::OPC_Decode, 190, 8, 221, 2, // Opcode: CMP_SLT_D
/* 1589 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 1604
/* 1594 */    MCD::OPC_CheckPredicate, 79, 91, 3, 0, // Skip to: 2458
/* 1599 */    MCD::OPC_Decode, 202, 8, 221, 2, // Opcode: CMP_SULT_D
/* 1604 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 1619
/* 1609 */    MCD::OPC_CheckPredicate, 79, 76, 3, 0, // Skip to: 2458
/* 1614 */    MCD::OPC_Decode, 186, 8, 221, 2, // Opcode: CMP_SLE_D
/* 1619 */    MCD::OPC_FilterValue, 15, 66, 3, 0, // Skip to: 2458
/* 1624 */    MCD::OPC_CheckPredicate, 79, 61, 3, 0, // Skip to: 2458
/* 1629 */    MCD::OPC_Decode, 198, 8, 221, 2, // Opcode: CMP_SULE_D
/* 1634 */    MCD::OPC_FilterValue, 18, 93, 0, 0, // Skip to: 1732
/* 1639 */    MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 1642 */    MCD::OPC_FilterValue, 9, 10, 0, 0, // Skip to: 1657
/* 1647 */    MCD::OPC_CheckPredicate, 76, 38, 3, 0, // Skip to: 2458
/* 1652 */    MCD::OPC_Decode, 143, 6, 222, 2, // Opcode: BC2EQZ
/* 1657 */    MCD::OPC_FilterValue, 10, 10, 0, 0, // Skip to: 1672
/* 1662 */    MCD::OPC_CheckPredicate, 76, 23, 3, 0, // Skip to: 2458
/* 1667 */    MCD::OPC_Decode, 141, 14, 223, 2, // Opcode: LWC2_R6
/* 1672 */    MCD::OPC_FilterValue, 11, 10, 0, 0, // Skip to: 1687
/* 1677 */    MCD::OPC_CheckPredicate, 76, 8, 3, 0, // Skip to: 2458
/* 1682 */    MCD::OPC_Decode, 173, 20, 223, 2, // Opcode: SWC2_R6
/* 1687 */    MCD::OPC_FilterValue, 13, 10, 0, 0, // Skip to: 1702
/* 1692 */    MCD::OPC_CheckPredicate, 76, 249, 2, 0, // Skip to: 2458
/* 1697 */    MCD::OPC_Decode, 145, 6, 222, 2, // Opcode: BC2NEZ
/* 1702 */    MCD::OPC_FilterValue, 14, 10, 0, 0, // Skip to: 1717
/* 1707 */    MCD::OPC_CheckPredicate, 76, 234, 2, 0, // Skip to: 2458
/* 1712 */    MCD::OPC_Decode, 209, 13, 223, 2, // Opcode: LDC2_R6
/* 1717 */    MCD::OPC_FilterValue, 15, 224, 2, 0, // Skip to: 2458
/* 1722 */    MCD::OPC_CheckPredicate, 76, 219, 2, 0, // Skip to: 2458
/* 1727 */    MCD::OPC_Decode, 169, 18, 223, 2, // Opcode: SDC2_R6
/* 1732 */    MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 1747
/* 1737 */    MCD::OPC_CheckPredicate, 76, 204, 2, 0, // Skip to: 2458
/* 1742 */    MCD::OPC_Decode, 185, 6, 224, 2, // Opcode: BGEZC
/* 1747 */    MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 1762
/* 1752 */    MCD::OPC_CheckPredicate, 76, 189, 2, 0, // Skip to: 2458
/* 1757 */    MCD::OPC_Decode, 242, 6, 225, 2, // Opcode: BLTZC
/* 1762 */    MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 1777
/* 1767 */    MCD::OPC_CheckPredicate, 76, 174, 2, 0, // Skip to: 2458
/* 1772 */    MCD::OPC_Decode, 253, 6, 226, 2, // Opcode: BNEC
/* 1777 */    MCD::OPC_FilterValue, 29, 10, 0, 0, // Skip to: 1792
/* 1782 */    MCD::OPC_CheckPredicate, 77, 159, 2, 0, // Skip to: 2458
/* 1787 */    MCD::OPC_Decode, 130, 10, 227, 2, // Opcode: DAUI
/* 1792 */    MCD::OPC_FilterValue, 31, 135, 1, 0, // Skip to: 2188
/* 1797 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 1800 */    MCD::OPC_FilterValue, 15, 123, 0, 0, // Skip to: 1928
/* 1805 */    MCD::OPC_ExtractField, 6, 10,  // Inst{15-6} ...
/* 1808 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 1823
/* 1813 */    MCD::OPC_CheckPredicate, 80, 128, 2, 0, // Skip to: 2458
/* 1818 */    MCD::OPC_Decode, 233, 8, 228, 2, // Opcode: CRC32B
/* 1823 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 1838
/* 1828 */    MCD::OPC_CheckPredicate, 80, 113, 2, 0, // Skip to: 2458
/* 1833 */    MCD::OPC_Decode, 239, 8, 228, 2, // Opcode: CRC32H
/* 1838 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1853
/* 1843 */    MCD::OPC_CheckPredicate, 80, 98, 2, 0, // Skip to: 2458
/* 1848 */    MCD::OPC_Decode, 240, 8, 228, 2, // Opcode: CRC32W
/* 1853 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1868
/* 1858 */    MCD::OPC_CheckPredicate, 81, 83, 2, 0, // Skip to: 2458
/* 1863 */    MCD::OPC_Decode, 238, 8, 228, 2, // Opcode: CRC32D
/* 1868 */    MCD::OPC_FilterValue, 4, 10, 0, 0, // Skip to: 1883
/* 1873 */    MCD::OPC_CheckPredicate, 80, 68, 2, 0, // Skip to: 2458
/* 1878 */    MCD::OPC_Decode, 234, 8, 228, 2, // Opcode: CRC32CB
/* 1883 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1898
/* 1888 */    MCD::OPC_CheckPredicate, 80, 53, 2, 0, // Skip to: 2458
/* 1893 */    MCD::OPC_Decode, 236, 8, 228, 2, // Opcode: CRC32CH
/* 1898 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 1913
/* 1903 */    MCD::OPC_CheckPredicate, 80, 38, 2, 0, // Skip to: 2458
/* 1908 */    MCD::OPC_Decode, 237, 8, 228, 2, // Opcode: CRC32CW
/* 1913 */    MCD::OPC_FilterValue, 7, 28, 2, 0, // Skip to: 2458
/* 1918 */    MCD::OPC_CheckPredicate, 81, 23, 2, 0, // Skip to: 2458
/* 1923 */    MCD::OPC_Decode, 235, 8, 228, 2, // Opcode: CRC32CD
/* 1928 */    MCD::OPC_FilterValue, 32, 47, 0, 0, // Skip to: 1980
/* 1933 */    MCD::OPC_ExtractField, 8, 3,  // Inst{10-8} ...
/* 1936 */    MCD::OPC_FilterValue, 0, 24, 0, 0, // Skip to: 1965
/* 1941 */    MCD::OPC_CheckPredicate, 75, 0, 2, 0, // Skip to: 2458
/* 1946 */    MCD::OPC_CheckField, 21, 5, 0, 249, 1, 0, // Skip to: 2458
/* 1953 */    MCD::OPC_CheckField, 6, 2, 0, 242, 1, 0, // Skip to: 2458
/* 1960 */    MCD::OPC_Decode, 217, 6, 193, 2, // Opcode: BITSWAP
/* 1965 */    MCD::OPC_FilterValue, 2, 232, 1, 0, // Skip to: 2458
/* 1970 */    MCD::OPC_CheckPredicate, 75, 227, 1, 0, // Skip to: 2458
/* 1975 */    MCD::OPC_Decode, 191, 5, 229, 2, // Opcode: ALIGN
/* 1980 */    MCD::OPC_FilterValue, 36, 47, 0, 0, // Skip to: 2032
/* 1985 */    MCD::OPC_ExtractField, 9, 2,  // Inst{10-9} ...
/* 1988 */    MCD::OPC_FilterValue, 0, 24, 0, 0, // Skip to: 2017
/* 1993 */    MCD::OPC_CheckPredicate, 77, 204, 1, 0, // Skip to: 2458
/* 1998 */    MCD::OPC_CheckField, 21, 5, 0, 197, 1, 0, // Skip to: 2458
/* 2005 */    MCD::OPC_CheckField, 6, 3, 0, 190, 1, 0, // Skip to: 2458
/* 2012 */    MCD::OPC_Decode, 131, 10, 230, 2, // Opcode: DBITSWAP
/* 2017 */    MCD::OPC_FilterValue, 1, 180, 1, 0, // Skip to: 2458
/* 2022 */    MCD::OPC_CheckPredicate, 77, 175, 1, 0, // Skip to: 2458
/* 2027 */    MCD::OPC_Decode, 128, 10, 231, 2, // Opcode: DALIGN
/* 2032 */    MCD::OPC_FilterValue, 37, 17, 0, 0, // Skip to: 2054
/* 2037 */    MCD::OPC_CheckPredicate, 76, 160, 1, 0, // Skip to: 2458
/* 2042 */    MCD::OPC_CheckField, 6, 1, 0, 153, 1, 0, // Skip to: 2458
/* 2049 */    MCD::OPC_Decode, 194, 7, 198, 2, // Opcode: CACHE_R6
/* 2054 */    MCD::OPC_FilterValue, 38, 10, 0, 0, // Skip to: 2069
/* 2059 */    MCD::OPC_CheckPredicate, 82, 138, 1, 0, // Skip to: 2458
/* 2064 */    MCD::OPC_Decode, 155, 18, 232, 2, // Opcode: SC_R6
/* 2069 */    MCD::OPC_FilterValue, 39, 10, 0, 0, // Skip to: 2084
/* 2074 */    MCD::OPC_CheckPredicate, 75, 123, 1, 0, // Skip to: 2458
/* 2079 */    MCD::OPC_Decode, 150, 18, 232, 2, // Opcode: SCD_R6
/* 2084 */    MCD::OPC_FilterValue, 53, 17, 0, 0, // Skip to: 2106
/* 2089 */    MCD::OPC_CheckPredicate, 76, 108, 1, 0, // Skip to: 2458
/* 2094 */    MCD::OPC_CheckField, 6, 1, 0, 101, 1, 0, // Skip to: 2458
/* 2101 */    MCD::OPC_Decode, 203, 17, 198, 2, // Opcode: PREF_R6
/* 2106 */    MCD::OPC_FilterValue, 54, 10, 0, 0, // Skip to: 2121
/* 2111 */    MCD::OPC_CheckPredicate, 82, 86, 1, 0, // Skip to: 2458
/* 2116 */    MCD::OPC_Decode, 251, 13, 232, 2, // Opcode: LL_R6
/* 2121 */    MCD::OPC_FilterValue, 55, 10, 0, 0, // Skip to: 2136
/* 2126 */    MCD::OPC_CheckPredicate, 77, 71, 1, 0, // Skip to: 2458
/* 2131 */    MCD::OPC_Decode, 246, 13, 232, 2, // Opcode: LLD_R6
/* 2136 */    MCD::OPC_FilterValue, 61, 61, 1, 0, // Skip to: 2458
/* 2141 */    MCD::OPC_ExtractField, 6, 2,  // Inst{7-6} ...
/* 2144 */    MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 2166
/* 2149 */    MCD::OPC_CheckPredicate, 83, 48, 1, 0, // Skip to: 2458
/* 2154 */    MCD::OPC_CheckField, 10, 11, 0, 41, 1, 0, // Skip to: 2458
/* 2161 */    MCD::OPC_Decode, 226, 12, 175, 1, // Opcode: GINVI
/* 2166 */    MCD::OPC_FilterValue, 2, 31, 1, 0, // Skip to: 2458
/* 2171 */    MCD::OPC_CheckPredicate, 83, 26, 1, 0, // Skip to: 2458
/* 2176 */    MCD::OPC_CheckField, 10, 11, 0, 19, 1, 0, // Skip to: 2458
/* 2183 */    MCD::OPC_Decode, 228, 12, 233, 2, // Opcode: GINVT
/* 2188 */    MCD::OPC_FilterValue, 50, 10, 0, 0, // Skip to: 2203
/* 2193 */    MCD::OPC_CheckPredicate, 76, 4, 1, 0, // Skip to: 2458
/* 2198 */    MCD::OPC_Decode, 131, 6, 234, 2, // Opcode: BC
/* 2203 */    MCD::OPC_FilterValue, 53, 27, 0, 0, // Skip to: 2235
/* 2208 */    MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 2225
/* 2213 */    MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 2225
/* 2220 */    MCD::OPC_Decode, 196, 6, 169, 1, // Opcode: BGTZC_MMR6
/* 2225 */    MCD::OPC_CheckPredicate, 24, 228, 0, 0, // Skip to: 2458
/* 2230 */    MCD::OPC_Decode, 244, 6, 169, 1, // Opcode: BLTZC_MMR6
/* 2235 */    MCD::OPC_FilterValue, 54, 26, 0, 0, // Skip to: 2266
/* 2240 */    MCD::OPC_CheckPredicate, 75, 11, 0, 0, // Skip to: 2256
/* 2245 */    MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 2256
/* 2252 */    MCD::OPC_Decode, 165, 13, 92, // Opcode: JIC
/* 2256 */    MCD::OPC_CheckPredicate, 76, 197, 0, 0, // Skip to: 2458
/* 2261 */    MCD::OPC_Decode, 165, 6, 235, 2, // Opcode: BEQZC
/* 2266 */    MCD::OPC_FilterValue, 58, 10, 0, 0, // Skip to: 2281
/* 2271 */    MCD::OPC_CheckPredicate, 75, 182, 0, 0, // Skip to: 2458
/* 2276 */    MCD::OPC_Decode, 251, 5, 234, 2, // Opcode: BALC
/* 2281 */    MCD::OPC_FilterValue, 59, 109, 0, 0, // Skip to: 2395
/* 2286 */    MCD::OPC_ExtractField, 19, 2,  // Inst{20-19} ...
/* 2289 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2304
/* 2294 */    MCD::OPC_CheckPredicate, 75, 159, 0, 0, // Skip to: 2458
/* 2299 */    MCD::OPC_Decode, 246, 4, 163, 1, // Opcode: ADDIUPC
/* 2304 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 2319
/* 2309 */    MCD::OPC_CheckPredicate, 75, 144, 0, 0, // Skip to: 2458
/* 2314 */    MCD::OPC_Decode, 156, 14, 163, 1, // Opcode: LWPC
/* 2319 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 2334
/* 2324 */    MCD::OPC_CheckPredicate, 84, 129, 0, 0, // Skip to: 2458
/* 2329 */    MCD::OPC_Decode, 165, 14, 163, 1, // Opcode: LWUPC
/* 2334 */    MCD::OPC_FilterValue, 3, 119, 0, 0, // Skip to: 2458
/* 2339 */    MCD::OPC_ExtractField, 18, 1,  // Inst{18} ...
/* 2342 */    MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 2357
/* 2347 */    MCD::OPC_CheckPredicate, 84, 106, 0, 0, // Skip to: 2458
/* 2352 */    MCD::OPC_Decode, 216, 13, 236, 2, // Opcode: LDPC
/* 2357 */    MCD::OPC_FilterValue, 1, 96, 0, 0, // Skip to: 2458
/* 2362 */    MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 2365 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 2380
/* 2370 */    MCD::OPC_CheckPredicate, 75, 83, 0, 0, // Skip to: 2458
/* 2375 */    MCD::OPC_Decode, 220, 5, 164, 1, // Opcode: AUIPC
/* 2380 */    MCD::OPC_FilterValue, 3, 73, 0, 0, // Skip to: 2458
/* 2385 */    MCD::OPC_CheckPredicate, 75, 68, 0, 0, // Skip to: 2458
/* 2390 */    MCD::OPC_Decode, 193, 5, 164, 1, // Opcode: ALUIPC
/* 2395 */    MCD::OPC_FilterValue, 61, 27, 0, 0, // Skip to: 2427
/* 2400 */    MCD::OPC_CheckPredicate, 24, 12, 0, 0, // Skip to: 2417
/* 2405 */    MCD::OPC_CheckField, 16, 5, 0, 5, 0, 0, // Skip to: 2417
/* 2412 */    MCD::OPC_Decode, 225, 6, 171, 1, // Opcode: BLEZC_MMR6
/* 2417 */    MCD::OPC_CheckPredicate, 24, 36, 0, 0, // Skip to: 2458
/* 2422 */    MCD::OPC_Decode, 187, 6, 171, 1, // Opcode: BGEZC_MMR6
/* 2427 */    MCD::OPC_FilterValue, 62, 26, 0, 0, // Skip to: 2458
/* 2432 */    MCD::OPC_CheckPredicate, 75, 11, 0, 0, // Skip to: 2448
/* 2437 */    MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 2448
/* 2444 */    MCD::OPC_Decode, 162, 13, 92, // Opcode: JIALC
/* 2448 */    MCD::OPC_CheckPredicate, 76, 5, 0, 0, // Skip to: 2458
/* 2453 */    MCD::OPC_Decode, 140, 7, 235, 2, // Opcode: BNEZC
/* 2458 */    MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32r6_64r6_Ambiguous32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 6, 27, 0, 0, // Skip to: 35
/* 8 */       MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 25
/* 13 */      MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 25
/* 20 */      MCD::OPC_Decode, 221, 6, 212, 2, // Opcode: BLEZALC
/* 25 */      MCD::OPC_CheckPredicate, 76, 165, 0, 0, // Skip to: 195
/* 30 */      MCD::OPC_Decode, 174, 6, 212, 2, // Opcode: BGEUC
/* 35 */      MCD::OPC_FilterValue, 7, 27, 0, 0, // Skip to: 67
/* 40 */      MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 57
/* 45 */      MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 57
/* 52 */      MCD::OPC_Decode, 192, 6, 213, 2, // Opcode: BGTZALC
/* 57 */      MCD::OPC_CheckPredicate, 76, 133, 0, 0, // Skip to: 195
/* 62 */      MCD::OPC_Decode, 231, 6, 213, 2, // Opcode: BLTUC
/* 67 */      MCD::OPC_FilterValue, 8, 27, 0, 0, // Skip to: 99
/* 72 */      MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 89
/* 77 */      MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 89
/* 84 */      MCD::OPC_Decode, 163, 6, 226, 2, // Opcode: BEQZALC
/* 89 */      MCD::OPC_CheckPredicate, 76, 101, 0, 0, // Skip to: 195
/* 94 */      MCD::OPC_Decode, 153, 7, 214, 2, // Opcode: BOVC
/* 99 */      MCD::OPC_FilterValue, 22, 27, 0, 0, // Skip to: 131
/* 104 */     MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 121
/* 109 */     MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 121
/* 116 */     MCD::OPC_Decode, 223, 6, 224, 2, // Opcode: BLEZC
/* 121 */     MCD::OPC_CheckPredicate, 76, 69, 0, 0, // Skip to: 195
/* 126 */     MCD::OPC_Decode, 171, 6, 224, 2, // Opcode: BGEC
/* 131 */     MCD::OPC_FilterValue, 23, 27, 0, 0, // Skip to: 163
/* 136 */     MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 153
/* 141 */     MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 153
/* 148 */     MCD::OPC_Decode, 194, 6, 225, 2, // Opcode: BGTZC
/* 153 */     MCD::OPC_CheckPredicate, 76, 37, 0, 0, // Skip to: 195
/* 158 */     MCD::OPC_Decode, 228, 6, 225, 2, // Opcode: BLTC
/* 163 */     MCD::OPC_FilterValue, 24, 27, 0, 0, // Skip to: 195
/* 168 */     MCD::OPC_CheckPredicate, 76, 12, 0, 0, // Skip to: 185
/* 173 */     MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 185
/* 180 */     MCD::OPC_Decode, 138, 7, 226, 2, // Opcode: BNEZALC
/* 185 */     MCD::OPC_CheckPredicate, 76, 5, 0, 0, // Skip to: 195
/* 190 */     MCD::OPC_Decode, 146, 7, 226, 2, // Opcode: BNVC
/* 195 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32r6_64r6_BranchZero32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 22, 10, 0, 0, // Skip to: 18
/* 8 */       MCD::OPC_CheckPredicate, 85, 20, 0, 0, // Skip to: 33
/* 13 */      MCD::OPC_Decode, 186, 6, 224, 2, // Opcode: BGEZC64
/* 18 */      MCD::OPC_FilterValue, 23, 10, 0, 0, // Skip to: 33
/* 23 */      MCD::OPC_CheckPredicate, 85, 5, 0, 0, // Skip to: 33
/* 28 */      MCD::OPC_Decode, 243, 6, 225, 2, // Opcode: BLTZC64
/* 33 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32r6_64r6_GP6432[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 53, 0, 0, // Skip to: 61
/* 8 */       MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 11 */      MCD::OPC_FilterValue, 53, 9, 0, 0, // Skip to: 25
/* 16 */      MCD::OPC_CheckPredicate, 86, 226, 0, 0, // Skip to: 247
/* 21 */      MCD::OPC_Decode, 184, 18, 12, // Opcode: SELEQZ64
/* 25 */      MCD::OPC_FilterValue, 55, 9, 0, 0, // Skip to: 39
/* 30 */      MCD::OPC_CheckPredicate, 86, 212, 0, 0, // Skip to: 247
/* 35 */      MCD::OPC_Decode, 191, 18, 12, // Opcode: SELNEZ64
/* 39 */      MCD::OPC_FilterValue, 137, 8, 202, 0, 0, // Skip to: 247
/* 45 */      MCD::OPC_CheckPredicate, 75, 197, 0, 0, // Skip to: 247
/* 50 */      MCD::OPC_CheckField, 11, 10, 0, 190, 0, 0, // Skip to: 247
/* 57 */      MCD::OPC_Decode, 177, 13, 13, // Opcode: JR_HB64_R6
/* 61 */      MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 76
/* 66 */      MCD::OPC_CheckPredicate, 85, 176, 0, 0, // Skip to: 247
/* 71 */      MCD::OPC_Decode, 175, 6, 212, 2, // Opcode: BGEUC64
/* 76 */      MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 91
/* 81 */      MCD::OPC_CheckPredicate, 85, 161, 0, 0, // Skip to: 247
/* 86 */      MCD::OPC_Decode, 232, 6, 213, 2, // Opcode: BLTUC64
/* 91 */      MCD::OPC_FilterValue, 8, 10, 0, 0, // Skip to: 106
/* 96 */      MCD::OPC_CheckPredicate, 85, 146, 0, 0, // Skip to: 247
/* 101 */     MCD::OPC_Decode, 159, 6, 214, 2, // Opcode: BEQC64
/* 106 */     MCD::OPC_FilterValue, 22, 27, 0, 0, // Skip to: 138
/* 111 */     MCD::OPC_CheckPredicate, 85, 12, 0, 0, // Skip to: 128
/* 116 */     MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 128
/* 123 */     MCD::OPC_Decode, 224, 6, 224, 2, // Opcode: BLEZC64
/* 128 */     MCD::OPC_CheckPredicate, 85, 114, 0, 0, // Skip to: 247
/* 133 */     MCD::OPC_Decode, 172, 6, 224, 2, // Opcode: BGEC64
/* 138 */     MCD::OPC_FilterValue, 23, 27, 0, 0, // Skip to: 170
/* 143 */     MCD::OPC_CheckPredicate, 85, 12, 0, 0, // Skip to: 160
/* 148 */     MCD::OPC_CheckField, 21, 5, 0, 5, 0, 0, // Skip to: 160
/* 155 */     MCD::OPC_Decode, 195, 6, 225, 2, // Opcode: BGTZC64
/* 160 */     MCD::OPC_CheckPredicate, 85, 82, 0, 0, // Skip to: 247
/* 165 */     MCD::OPC_Decode, 229, 6, 225, 2, // Opcode: BLTC64
/* 170 */     MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 185
/* 175 */     MCD::OPC_CheckPredicate, 85, 67, 0, 0, // Skip to: 247
/* 180 */     MCD::OPC_Decode, 254, 6, 226, 2, // Opcode: BNEC64
/* 185 */     MCD::OPC_FilterValue, 54, 26, 0, 0, // Skip to: 216
/* 190 */     MCD::OPC_CheckPredicate, 85, 11, 0, 0, // Skip to: 206
/* 195 */     MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 206
/* 202 */     MCD::OPC_Decode, 166, 13, 11, // Opcode: JIC64
/* 206 */     MCD::OPC_CheckPredicate, 85, 36, 0, 0, // Skip to: 247
/* 211 */     MCD::OPC_Decode, 167, 6, 237, 2, // Opcode: BEQZC64
/* 216 */     MCD::OPC_FilterValue, 62, 26, 0, 0, // Skip to: 247
/* 221 */     MCD::OPC_CheckPredicate, 85, 11, 0, 0, // Skip to: 237
/* 226 */     MCD::OPC_CheckField, 21, 5, 0, 4, 0, 0, // Skip to: 237
/* 233 */     MCD::OPC_Decode, 163, 13, 11, // Opcode: JIALC64
/* 237 */     MCD::OPC_CheckPredicate, 85, 5, 0, 0, // Skip to: 247
/* 242 */     MCD::OPC_Decode, 142, 7, 237, 2, // Opcode: BNEZC64
/* 247 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips32r6_64r6_PTR6432[] = {
/* 0 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 3 */       MCD::OPC_FilterValue, 38, 17, 0, 0, // Skip to: 25
/* 8 */       MCD::OPC_CheckPredicate, 87, 34, 0, 0, // Skip to: 47
/* 13 */      MCD::OPC_CheckField, 26, 6, 31, 27, 0, 0, // Skip to: 47
/* 20 */      MCD::OPC_Decode, 148, 18, 232, 2, // Opcode: SC64_R6
/* 25 */      MCD::OPC_FilterValue, 54, 17, 0, 0, // Skip to: 47
/* 30 */      MCD::OPC_CheckPredicate, 87, 12, 0, 0, // Skip to: 47
/* 35 */      MCD::OPC_CheckField, 26, 6, 31, 5, 0, 0, // Skip to: 47
/* 42 */      MCD::OPC_Decode, 244, 13, 232, 2, // Opcode: LL64_R6
/* 47 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMips6432[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 0, 236, 1, 0, // Skip to: 500
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 8, 16, 0, 0, // Skip to: 32
/* 16 */      MCD::OPC_CheckPredicate, 88, 214, 4, 0, // Skip to: 1259
/* 21 */      MCD::OPC_CheckField, 6, 15, 16, 207, 4, 0, // Skip to: 1259
/* 28 */      MCD::OPC_Decode, 176, 13, 13, // Opcode: JR_HB64
/* 32 */      MCD::OPC_FilterValue, 9, 45, 0, 0, // Skip to: 82
/* 37 */      MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 40 */      MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 61
/* 45 */      MCD::OPC_CheckPredicate, 89, 185, 4, 0, // Skip to: 1259
/* 50 */      MCD::OPC_CheckField, 16, 5, 0, 178, 4, 0, // Skip to: 1259
/* 57 */      MCD::OPC_Decode, 149, 13, 15, // Opcode: JALR64
/* 61 */      MCD::OPC_FilterValue, 16, 169, 4, 0, // Skip to: 1259
/* 66 */      MCD::OPC_CheckPredicate, 90, 164, 4, 0, // Skip to: 1259
/* 71 */      MCD::OPC_CheckField, 16, 5, 0, 157, 4, 0, // Skip to: 1259
/* 78 */      MCD::OPC_Decode, 156, 13, 15, // Opcode: JALR_HB64
/* 82 */      MCD::OPC_FilterValue, 20, 17, 0, 0, // Skip to: 104
/* 87 */      MCD::OPC_CheckPredicate, 91, 143, 4, 0, // Skip to: 1259
/* 92 */      MCD::OPC_CheckField, 6, 5, 0, 136, 4, 0, // Skip to: 1259
/* 99 */      MCD::OPC_Decode, 245, 10, 238, 2, // Opcode: DSLLV
/* 104 */     MCD::OPC_FilterValue, 22, 33, 0, 0, // Skip to: 142
/* 109 */     MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 112 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 127
/* 117 */     MCD::OPC_CheckPredicate, 91, 113, 4, 0, // Skip to: 1259
/* 122 */     MCD::OPC_Decode, 251, 10, 238, 2, // Opcode: DSRLV
/* 127 */     MCD::OPC_FilterValue, 1, 103, 4, 0, // Skip to: 1259
/* 132 */     MCD::OPC_CheckPredicate, 90, 98, 4, 0, // Skip to: 1259
/* 137 */     MCD::OPC_Decode, 238, 10, 238, 2, // Opcode: DROTRV
/* 142 */     MCD::OPC_FilterValue, 23, 17, 0, 0, // Skip to: 164
/* 147 */     MCD::OPC_CheckPredicate, 91, 83, 4, 0, // Skip to: 1259
/* 152 */     MCD::OPC_CheckField, 6, 5, 0, 76, 4, 0, // Skip to: 1259
/* 159 */     MCD::OPC_Decode, 248, 10, 238, 2, // Opcode: DSRAV
/* 164 */     MCD::OPC_FilterValue, 28, 17, 0, 0, // Skip to: 186
/* 169 */     MCD::OPC_CheckPredicate, 92, 61, 4, 0, // Skip to: 1259
/* 174 */     MCD::OPC_CheckField, 6, 10, 0, 54, 4, 0, // Skip to: 1259
/* 181 */     MCD::OPC_Decode, 181, 10, 239, 2, // Opcode: DMULT
/* 186 */     MCD::OPC_FilterValue, 29, 17, 0, 0, // Skip to: 208
/* 191 */     MCD::OPC_CheckPredicate, 92, 39, 4, 0, // Skip to: 1259
/* 196 */     MCD::OPC_CheckField, 6, 10, 0, 32, 4, 0, // Skip to: 1259
/* 203 */     MCD::OPC_Decode, 182, 10, 239, 2, // Opcode: DMULTu
/* 208 */     MCD::OPC_FilterValue, 30, 17, 0, 0, // Skip to: 230
/* 213 */     MCD::OPC_CheckPredicate, 92, 17, 4, 0, // Skip to: 1259
/* 218 */     MCD::OPC_CheckField, 6, 10, 0, 10, 4, 0, // Skip to: 1259
/* 225 */     MCD::OPC_Decode, 240, 10, 239, 2, // Opcode: DSDIV
/* 230 */     MCD::OPC_FilterValue, 31, 17, 0, 0, // Skip to: 252
/* 235 */     MCD::OPC_CheckPredicate, 92, 251, 3, 0, // Skip to: 1259
/* 240 */     MCD::OPC_CheckField, 6, 10, 0, 244, 3, 0, // Skip to: 1259
/* 247 */     MCD::OPC_Decode, 254, 10, 239, 2, // Opcode: DUDIV
/* 252 */     MCD::OPC_FilterValue, 44, 16, 0, 0, // Skip to: 273
/* 257 */     MCD::OPC_CheckPredicate, 91, 229, 3, 0, // Skip to: 1259
/* 262 */     MCD::OPC_CheckField, 6, 5, 0, 222, 3, 0, // Skip to: 1259
/* 269 */     MCD::OPC_Decode, 251, 9, 12, // Opcode: DADD
/* 273 */     MCD::OPC_FilterValue, 45, 16, 0, 0, // Skip to: 294
/* 278 */     MCD::OPC_CheckPredicate, 91, 208, 3, 0, // Skip to: 1259
/* 283 */     MCD::OPC_CheckField, 6, 5, 0, 201, 3, 0, // Skip to: 1259
/* 290 */     MCD::OPC_Decode, 254, 9, 12, // Opcode: DADDu
/* 294 */     MCD::OPC_FilterValue, 46, 16, 0, 0, // Skip to: 315
/* 299 */     MCD::OPC_CheckPredicate, 91, 187, 3, 0, // Skip to: 1259
/* 304 */     MCD::OPC_CheckField, 6, 5, 0, 180, 3, 0, // Skip to: 1259
/* 311 */     MCD::OPC_Decode, 252, 10, 12, // Opcode: DSUB
/* 315 */     MCD::OPC_FilterValue, 47, 16, 0, 0, // Skip to: 336
/* 320 */     MCD::OPC_CheckPredicate, 91, 166, 3, 0, // Skip to: 1259
/* 325 */     MCD::OPC_CheckField, 6, 5, 0, 159, 3, 0, // Skip to: 1259
/* 332 */     MCD::OPC_Decode, 253, 10, 12, // Opcode: DSUBu
/* 336 */     MCD::OPC_FilterValue, 56, 17, 0, 0, // Skip to: 358
/* 341 */     MCD::OPC_CheckPredicate, 91, 145, 3, 0, // Skip to: 1259
/* 346 */     MCD::OPC_CheckField, 21, 5, 0, 138, 3, 0, // Skip to: 1259
/* 353 */     MCD::OPC_Decode, 242, 10, 240, 2, // Opcode: DSLL
/* 358 */     MCD::OPC_FilterValue, 58, 33, 0, 0, // Skip to: 396
/* 363 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 366 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 381
/* 371 */     MCD::OPC_CheckPredicate, 91, 115, 3, 0, // Skip to: 1259
/* 376 */     MCD::OPC_Decode, 249, 10, 240, 2, // Opcode: DSRL
/* 381 */     MCD::OPC_FilterValue, 1, 105, 3, 0, // Skip to: 1259
/* 386 */     MCD::OPC_CheckPredicate, 90, 100, 3, 0, // Skip to: 1259
/* 391 */     MCD::OPC_Decode, 236, 10, 240, 2, // Opcode: DROTR
/* 396 */     MCD::OPC_FilterValue, 59, 17, 0, 0, // Skip to: 418
/* 401 */     MCD::OPC_CheckPredicate, 91, 85, 3, 0, // Skip to: 1259
/* 406 */     MCD::OPC_CheckField, 21, 5, 0, 78, 3, 0, // Skip to: 1259
/* 413 */     MCD::OPC_Decode, 246, 10, 240, 2, // Opcode: DSRA
/* 418 */     MCD::OPC_FilterValue, 60, 17, 0, 0, // Skip to: 440
/* 423 */     MCD::OPC_CheckPredicate, 91, 63, 3, 0, // Skip to: 1259
/* 428 */     MCD::OPC_CheckField, 21, 5, 0, 56, 3, 0, // Skip to: 1259
/* 435 */     MCD::OPC_Decode, 243, 10, 240, 2, // Opcode: DSLL32
/* 440 */     MCD::OPC_FilterValue, 62, 33, 0, 0, // Skip to: 478
/* 445 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 448 */     MCD::OPC_FilterValue, 0, 10, 0, 0, // Skip to: 463
/* 453 */     MCD::OPC_CheckPredicate, 91, 33, 3, 0, // Skip to: 1259
/* 458 */     MCD::OPC_Decode, 250, 10, 240, 2, // Opcode: DSRL32
/* 463 */     MCD::OPC_FilterValue, 1, 23, 3, 0, // Skip to: 1259
/* 468 */     MCD::OPC_CheckPredicate, 90, 18, 3, 0, // Skip to: 1259
/* 473 */     MCD::OPC_Decode, 237, 10, 240, 2, // Opcode: DROTR32
/* 478 */     MCD::OPC_FilterValue, 63, 8, 3, 0, // Skip to: 1259
/* 483 */     MCD::OPC_CheckPredicate, 91, 3, 3, 0, // Skip to: 1259
/* 488 */     MCD::OPC_CheckField, 21, 5, 0, 252, 2, 0, // Skip to: 1259
/* 495 */     MCD::OPC_Decode, 247, 10, 240, 2, // Opcode: DSRA32
/* 500 */     MCD::OPC_FilterValue, 16, 85, 0, 0, // Skip to: 590
/* 505 */     MCD::OPC_ExtractField, 3, 8,  // Inst{10-3} ...
/* 508 */     MCD::OPC_FilterValue, 0, 33, 0, 0, // Skip to: 546
/* 513 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 516 */     MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 531
/* 521 */     MCD::OPC_CheckPredicate, 93, 221, 2, 0, // Skip to: 1259
/* 526 */     MCD::OPC_Decode, 165, 10, 241, 2, // Opcode: DMFC0
/* 531 */     MCD::OPC_FilterValue, 5, 211, 2, 0, // Skip to: 1259
/* 536 */     MCD::OPC_CheckPredicate, 93, 206, 2, 0, // Skip to: 1259
/* 541 */     MCD::OPC_Decode, 173, 10, 242, 2, // Opcode: DMTC0
/* 546 */     MCD::OPC_FilterValue, 32, 17, 0, 0, // Skip to: 568
/* 551 */     MCD::OPC_CheckPredicate, 94, 191, 2, 0, // Skip to: 1259
/* 556 */     MCD::OPC_CheckField, 21, 5, 3, 184, 2, 0, // Skip to: 1259
/* 563 */     MCD::OPC_Decode, 169, 10, 241, 2, // Opcode: DMFGC0
/* 568 */     MCD::OPC_FilterValue, 96, 174, 2, 0, // Skip to: 1259
/* 573 */     MCD::OPC_CheckPredicate, 94, 169, 2, 0, // Skip to: 1259
/* 578 */     MCD::OPC_CheckField, 21, 5, 3, 162, 2, 0, // Skip to: 1259
/* 585 */     MCD::OPC_Decode, 177, 10, 242, 2, // Opcode: DMTGC0
/* 590 */     MCD::OPC_FilterValue, 18, 47, 0, 0, // Skip to: 642
/* 595 */     MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 598 */     MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 620
/* 603 */     MCD::OPC_CheckPredicate, 93, 139, 2, 0, // Skip to: 1259
/* 608 */     MCD::OPC_CheckField, 3, 8, 0, 132, 2, 0, // Skip to: 1259
/* 615 */     MCD::OPC_Decode, 167, 10, 243, 2, // Opcode: DMFC2
/* 620 */     MCD::OPC_FilterValue, 5, 122, 2, 0, // Skip to: 1259
/* 625 */     MCD::OPC_CheckPredicate, 93, 117, 2, 0, // Skip to: 1259
/* 630 */     MCD::OPC_CheckField, 3, 8, 0, 110, 2, 0, // Skip to: 1259
/* 637 */     MCD::OPC_Decode, 175, 10, 244, 2, // Opcode: DMTC2
/* 642 */     MCD::OPC_FilterValue, 21, 3, 1, 0, // Skip to: 906
/* 647 */     MCD::OPC_ExtractField, 0, 13,  // Inst{12-0} ...
/* 650 */     MCD::OPC_FilterValue, 188, 8, 10, 0, 0, // Skip to: 666
/* 656 */     MCD::OPC_CheckPredicate, 15, 86, 2, 0, // Skip to: 1259
/* 661 */     MCD::OPC_Decode, 161, 9, 245, 2, // Opcode: C_F_D64_MM
/* 666 */     MCD::OPC_FilterValue, 252, 8, 10, 0, 0, // Skip to: 682
/* 672 */     MCD::OPC_CheckPredicate, 15, 70, 2, 0, // Skip to: 1259
/* 677 */     MCD::OPC_Decode, 245, 9, 245, 2, // Opcode: C_UN_D64_MM
/* 682 */     MCD::OPC_FilterValue, 188, 9, 10, 0, 0, // Skip to: 698
/* 688 */     MCD::OPC_CheckPredicate, 15, 54, 2, 0, // Skip to: 1259
/* 693 */     MCD::OPC_Decode, 155, 9, 245, 2, // Opcode: C_EQ_D64_MM
/* 698 */     MCD::OPC_FilterValue, 252, 9, 10, 0, 0, // Skip to: 714
/* 704 */     MCD::OPC_CheckPredicate, 15, 38, 2, 0, // Skip to: 1259
/* 709 */     MCD::OPC_Decode, 227, 9, 245, 2, // Opcode: C_UEQ_D64_MM
/* 714 */     MCD::OPC_FilterValue, 188, 10, 10, 0, 0, // Skip to: 730
/* 720 */     MCD::OPC_CheckPredicate, 15, 22, 2, 0, // Skip to: 1259
/* 725 */     MCD::OPC_Decode, 209, 9, 245, 2, // Opcode: C_OLT_D64_MM
/* 730 */     MCD::OPC_FilterValue, 252, 10, 10, 0, 0, // Skip to: 746
/* 736 */     MCD::OPC_CheckPredicate, 15, 6, 2, 0, // Skip to: 1259
/* 741 */     MCD::OPC_Decode, 239, 9, 245, 2, // Opcode: C_ULT_D64_MM
/* 746 */     MCD::OPC_FilterValue, 188, 11, 10, 0, 0, // Skip to: 762
/* 752 */     MCD::OPC_CheckPredicate, 15, 246, 1, 0, // Skip to: 1259
/* 757 */     MCD::OPC_Decode, 203, 9, 245, 2, // Opcode: C_OLE_D64_MM
/* 762 */     MCD::OPC_FilterValue, 252, 11, 10, 0, 0, // Skip to: 778
/* 768 */     MCD::OPC_CheckPredicate, 15, 230, 1, 0, // Skip to: 1259
/* 773 */     MCD::OPC_Decode, 233, 9, 245, 2, // Opcode: C_ULE_D64_MM
/* 778 */     MCD::OPC_FilterValue, 188, 12, 10, 0, 0, // Skip to: 794
/* 784 */     MCD::OPC_CheckPredicate, 15, 214, 1, 0, // Skip to: 1259
/* 789 */     MCD::OPC_Decode, 221, 9, 245, 2, // Opcode: C_SF_D64_MM
/* 794 */     MCD::OPC_FilterValue, 252, 12, 10, 0, 0, // Skip to: 810
/* 800 */     MCD::OPC_CheckPredicate, 15, 198, 1, 0, // Skip to: 1259
/* 805 */     MCD::OPC_Decode, 185, 9, 245, 2, // Opcode: C_NGLE_D64_MM
/* 810 */     MCD::OPC_FilterValue, 188, 13, 10, 0, 0, // Skip to: 826
/* 816 */     MCD::OPC_CheckPredicate, 15, 182, 1, 0, // Skip to: 1259
/* 821 */     MCD::OPC_Decode, 215, 9, 245, 2, // Opcode: C_SEQ_D64_MM
/* 826 */     MCD::OPC_FilterValue, 252, 13, 10, 0, 0, // Skip to: 842
/* 832 */     MCD::OPC_CheckPredicate, 15, 166, 1, 0, // Skip to: 1259
/* 837 */     MCD::OPC_Decode, 191, 9, 245, 2, // Opcode: C_NGL_D64_MM
/* 842 */     MCD::OPC_FilterValue, 188, 14, 10, 0, 0, // Skip to: 858
/* 848 */     MCD::OPC_CheckPredicate, 15, 150, 1, 0, // Skip to: 1259
/* 853 */     MCD::OPC_Decode, 173, 9, 245, 2, // Opcode: C_LT_D64_MM
/* 858 */     MCD::OPC_FilterValue, 252, 14, 10, 0, 0, // Skip to: 874
/* 864 */     MCD::OPC_CheckPredicate, 15, 134, 1, 0, // Skip to: 1259
/* 869 */     MCD::OPC_Decode, 179, 9, 245, 2, // Opcode: C_NGE_D64_MM
/* 874 */     MCD::OPC_FilterValue, 188, 15, 10, 0, 0, // Skip to: 890
/* 880 */     MCD::OPC_CheckPredicate, 15, 118, 1, 0, // Skip to: 1259
/* 885 */     MCD::OPC_Decode, 167, 9, 245, 2, // Opcode: C_LE_D64_MM
/* 890 */     MCD::OPC_FilterValue, 252, 15, 107, 1, 0, // Skip to: 1259
/* 896 */     MCD::OPC_CheckPredicate, 15, 102, 1, 0, // Skip to: 1259
/* 901 */     MCD::OPC_Decode, 197, 9, 245, 2, // Opcode: C_NGT_D64_MM
/* 906 */     MCD::OPC_FilterValue, 24, 10, 0, 0, // Skip to: 921
/* 911 */     MCD::OPC_CheckPredicate, 95, 87, 1, 0, // Skip to: 1259
/* 916 */     MCD::OPC_Decode, 252, 9, 246, 2, // Opcode: DADDi
/* 921 */     MCD::OPC_FilterValue, 25, 10, 0, 0, // Skip to: 936
/* 926 */     MCD::OPC_CheckPredicate, 91, 72, 1, 0, // Skip to: 1259
/* 931 */     MCD::OPC_Decode, 253, 9, 246, 2, // Opcode: DADDiu
/* 936 */     MCD::OPC_FilterValue, 26, 10, 0, 0, // Skip to: 951
/* 941 */     MCD::OPC_CheckPredicate, 95, 57, 1, 0, // Skip to: 1259
/* 946 */     MCD::OPC_Decode, 215, 13, 130, 1, // Opcode: LDL
/* 951 */     MCD::OPC_FilterValue, 27, 10, 0, 0, // Skip to: 966
/* 956 */     MCD::OPC_CheckPredicate, 95, 42, 1, 0, // Skip to: 1259
/* 961 */     MCD::OPC_Decode, 217, 13, 130, 1, // Opcode: LDR
/* 966 */     MCD::OPC_FilterValue, 28, 33, 0, 0, // Skip to: 1004
/* 971 */     MCD::OPC_ExtractField, 0, 11,  // Inst{10-0} ...
/* 974 */     MCD::OPC_FilterValue, 36, 10, 0, 0, // Skip to: 989
/* 979 */     MCD::OPC_CheckPredicate, 96, 19, 1, 0, // Skip to: 1259
/* 984 */     MCD::OPC_Decode, 134, 10, 247, 2, // Opcode: DCLZ
/* 989 */     MCD::OPC_FilterValue, 37, 9, 1, 0, // Skip to: 1259
/* 994 */     MCD::OPC_CheckPredicate, 96, 4, 1, 0, // Skip to: 1259
/* 999 */     MCD::OPC_Decode, 132, 10, 247, 2, // Opcode: DCLO
/* 1004 */    MCD::OPC_FilterValue, 31, 145, 0, 0, // Skip to: 1154
/* 1009 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 1012 */    MCD::OPC_FilterValue, 1, 10, 0, 0, // Skip to: 1027
/* 1017 */    MCD::OPC_CheckPredicate, 90, 237, 0, 0, // Skip to: 1259
/* 1022 */    MCD::OPC_Decode, 143, 10, 248, 2, // Opcode: DEXTM
/* 1027 */    MCD::OPC_FilterValue, 2, 10, 0, 0, // Skip to: 1042
/* 1032 */    MCD::OPC_CheckPredicate, 90, 222, 0, 0, // Skip to: 1259
/* 1037 */    MCD::OPC_Decode, 144, 10, 248, 2, // Opcode: DEXTU
/* 1042 */    MCD::OPC_FilterValue, 3, 10, 0, 0, // Skip to: 1057
/* 1047 */    MCD::OPC_CheckPredicate, 90, 207, 0, 0, // Skip to: 1259
/* 1052 */    MCD::OPC_Decode, 141, 10, 248, 2, // Opcode: DEXT
/* 1057 */    MCD::OPC_FilterValue, 5, 10, 0, 0, // Skip to: 1072
/* 1062 */    MCD::OPC_CheckPredicate, 90, 192, 0, 0, // Skip to: 1259
/* 1067 */    MCD::OPC_Decode, 147, 10, 249, 2, // Opcode: DINSM
/* 1072 */    MCD::OPC_FilterValue, 6, 10, 0, 0, // Skip to: 1087
/* 1077 */    MCD::OPC_CheckPredicate, 90, 177, 0, 0, // Skip to: 1259
/* 1082 */    MCD::OPC_Decode, 148, 10, 249, 2, // Opcode: DINSU
/* 1087 */    MCD::OPC_FilterValue, 7, 10, 0, 0, // Skip to: 1102
/* 1092 */    MCD::OPC_CheckPredicate, 90, 162, 0, 0, // Skip to: 1259
/* 1097 */    MCD::OPC_Decode, 146, 10, 249, 2, // Opcode: DINS
/* 1102 */    MCD::OPC_FilterValue, 36, 152, 0, 0, // Skip to: 1259
/* 1107 */    MCD::OPC_ExtractField, 6, 5,  // Inst{10-6} ...
/* 1110 */    MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 1132
/* 1115 */    MCD::OPC_CheckPredicate, 90, 139, 0, 0, // Skip to: 1259
/* 1120 */    MCD::OPC_CheckField, 21, 5, 0, 132, 0, 0, // Skip to: 1259
/* 1127 */    MCD::OPC_Decode, 239, 10, 230, 2, // Opcode: DSBH
/* 1132 */    MCD::OPC_FilterValue, 5, 122, 0, 0, // Skip to: 1259
/* 1137 */    MCD::OPC_CheckPredicate, 90, 117, 0, 0, // Skip to: 1259
/* 1142 */    MCD::OPC_CheckField, 21, 5, 0, 110, 0, 0, // Skip to: 1259
/* 1149 */    MCD::OPC_Decode, 241, 10, 230, 2, // Opcode: DSHD
/* 1154 */    MCD::OPC_FilterValue, 39, 10, 0, 0, // Skip to: 1169
/* 1159 */    MCD::OPC_CheckPredicate, 91, 95, 0, 0, // Skip to: 1259
/* 1164 */    MCD::OPC_Decode, 174, 14, 130, 1, // Opcode: LWu
/* 1169 */    MCD::OPC_FilterValue, 44, 10, 0, 0, // Skip to: 1184
/* 1174 */    MCD::OPC_CheckPredicate, 95, 80, 0, 0, // Skip to: 1259
/* 1179 */    MCD::OPC_Decode, 173, 18, 130, 1, // Opcode: SDL
/* 1184 */    MCD::OPC_FilterValue, 45, 10, 0, 0, // Skip to: 1199
/* 1189 */    MCD::OPC_CheckPredicate, 95, 65, 0, 0, // Skip to: 1259
/* 1194 */    MCD::OPC_Decode, 174, 18, 130, 1, // Opcode: SDR
/* 1199 */    MCD::OPC_FilterValue, 52, 10, 0, 0, // Skip to: 1214
/* 1204 */    MCD::OPC_CheckPredicate, 92, 50, 0, 0, // Skip to: 1259
/* 1209 */    MCD::OPC_Decode, 245, 13, 130, 1, // Opcode: LLD
/* 1214 */    MCD::OPC_FilterValue, 55, 10, 0, 0, // Skip to: 1229
/* 1219 */    MCD::OPC_CheckPredicate, 91, 35, 0, 0, // Skip to: 1259
/* 1224 */    MCD::OPC_Decode, 202, 13, 130, 1, // Opcode: LD
/* 1229 */    MCD::OPC_FilterValue, 60, 10, 0, 0, // Skip to: 1244
/* 1234 */    MCD::OPC_CheckPredicate, 95, 20, 0, 0, // Skip to: 1259
/* 1239 */    MCD::OPC_Decode, 149, 18, 130, 1, // Opcode: SCD
/* 1244 */    MCD::OPC_FilterValue, 63, 10, 0, 0, // Skip to: 1259
/* 1249 */    MCD::OPC_CheckPredicate, 91, 5, 0, 0, // Skip to: 1259
/* 1254 */    MCD::OPC_Decode, 156, 18, 130, 1, // Opcode: SD
/* 1259 */    MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMipsDSP32[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 35, 10, 0, 0, // Skip to: 18
/* 8 */       MCD::OPC_CheckPredicate, 97, 20, 0, 0, // Skip to: 33
/* 13 */      MCD::OPC_Decode, 143, 14, 130, 1, // Opcode: LWDSP
/* 18 */      MCD::OPC_FilterValue, 43, 10, 0, 0, // Skip to: 33
/* 23 */      MCD::OPC_CheckPredicate, 97, 5, 0, 0, // Skip to: 33
/* 28 */      MCD::OPC_Decode, 175, 20, 130, 1, // Opcode: SWDSP
/* 33 */      MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableMipsFP6432[] = {
/* 0 */       MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 3 */       MCD::OPC_FilterValue, 17, 74, 5, 0, // Skip to: 1362
/* 8 */       MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 106, 0, 0, // Skip to: 122
/* 16 */      MCD::OPC_ExtractField, 21, 5,  // Inst{25-21} ...
/* 19 */      MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 41
/* 24 */      MCD::OPC_CheckPredicate, 98, 239, 5, 0, // Skip to: 1548
/* 29 */      MCD::OPC_CheckField, 6, 5, 0, 232, 5, 0, // Skip to: 1548
/* 36 */      MCD::OPC_Decode, 250, 14, 250, 2, // Opcode: MFC1_D64
/* 41 */      MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 63
/* 46 */      MCD::OPC_CheckPredicate, 99, 217, 5, 0, // Skip to: 1548
/* 51 */      MCD::OPC_CheckField, 6, 5, 0, 210, 5, 0, // Skip to: 1548
/* 58 */      MCD::OPC_Decode, 132, 15, 250, 2, // Opcode: MFHC1_D64
/* 63 */      MCD::OPC_FilterValue, 4, 17, 0, 0, // Skip to: 85
/* 68 */      MCD::OPC_CheckPredicate, 98, 195, 5, 0, // Skip to: 1548
/* 73 */      MCD::OPC_CheckField, 6, 5, 0, 188, 5, 0, // Skip to: 1548
/* 80 */      MCD::OPC_Decode, 137, 16, 251, 2, // Opcode: MTC1_D64
/* 85 */      MCD::OPC_FilterValue, 7, 17, 0, 0, // Skip to: 107
/* 90 */      MCD::OPC_CheckPredicate, 99, 173, 5, 0, // Skip to: 1548
/* 95 */      MCD::OPC_CheckField, 6, 5, 0, 166, 5, 0, // Skip to: 1548
/* 102 */     MCD::OPC_Decode, 148, 16, 252, 2, // Opcode: MTHC1_D64
/* 107 */     MCD::OPC_FilterValue, 17, 156, 5, 0, // Skip to: 1548
/* 112 */     MCD::OPC_CheckPredicate, 98, 151, 5, 0, // Skip to: 1548
/* 117 */     MCD::OPC_Decode, 185, 11, 219, 2, // Opcode: FADD_D64
/* 122 */     MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 144
/* 127 */     MCD::OPC_CheckPredicate, 98, 136, 5, 0, // Skip to: 1548
/* 132 */     MCD::OPC_CheckField, 21, 5, 17, 129, 5, 0, // Skip to: 1548
/* 139 */     MCD::OPC_Decode, 200, 12, 219, 2, // Opcode: FSUB_D64
/* 144 */     MCD::OPC_FilterValue, 2, 17, 0, 0, // Skip to: 166
/* 149 */     MCD::OPC_CheckPredicate, 98, 114, 5, 0, // Skip to: 1548
/* 154 */     MCD::OPC_CheckField, 21, 5, 17, 107, 5, 0, // Skip to: 1548
/* 161 */     MCD::OPC_Decode, 157, 12, 219, 2, // Opcode: FMUL_D64
/* 166 */     MCD::OPC_FilterValue, 3, 17, 0, 0, // Skip to: 188
/* 171 */     MCD::OPC_CheckPredicate, 98, 92, 5, 0, // Skip to: 1548
/* 176 */     MCD::OPC_CheckField, 21, 5, 17, 85, 5, 0, // Skip to: 1548
/* 183 */     MCD::OPC_Decode, 223, 11, 219, 2, // Opcode: FDIV_D64
/* 188 */     MCD::OPC_FilterValue, 4, 18, 0, 0, // Skip to: 211
/* 193 */     MCD::OPC_CheckPredicate, 100, 70, 5, 0, // Skip to: 1548
/* 198 */     MCD::OPC_CheckField, 16, 10, 160, 4, 62, 5, 0, // Skip to: 1548
/* 206 */     MCD::OPC_Decode, 192, 12, 219, 1, // Opcode: FSQRT_D64
/* 211 */     MCD::OPC_FilterValue, 5, 18, 0, 0, // Skip to: 234
/* 216 */     MCD::OPC_CheckPredicate, 98, 47, 5, 0, // Skip to: 1548
/* 221 */     MCD::OPC_CheckField, 16, 10, 160, 4, 39, 5, 0, // Skip to: 1548
/* 229 */     MCD::OPC_Decode, 178, 11, 219, 1, // Opcode: FABS_D64
/* 234 */     MCD::OPC_FilterValue, 6, 18, 0, 0, // Skip to: 257
/* 239 */     MCD::OPC_CheckPredicate, 98, 24, 5, 0, // Skip to: 1548
/* 244 */     MCD::OPC_CheckField, 16, 10, 160, 4, 16, 5, 0, // Skip to: 1548
/* 252 */     MCD::OPC_Decode, 146, 12, 219, 1, // Opcode: FMOV_D64
/* 257 */     MCD::OPC_FilterValue, 7, 18, 0, 0, // Skip to: 280
/* 262 */     MCD::OPC_CheckPredicate, 98, 1, 5, 0, // Skip to: 1548
/* 267 */     MCD::OPC_CheckField, 16, 10, 160, 4, 249, 4, 0, // Skip to: 1548
/* 275 */     MCD::OPC_Decode, 165, 12, 219, 1, // Opcode: FNEG_D64
/* 280 */     MCD::OPC_FilterValue, 8, 35, 0, 0, // Skip to: 320
/* 285 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 288 */     MCD::OPC_FilterValue, 128, 4, 10, 0, 0, // Skip to: 304
/* 294 */     MCD::OPC_CheckPredicate, 100, 225, 4, 0, // Skip to: 1548
/* 299 */     MCD::OPC_Decode, 239, 17, 212, 1, // Opcode: ROUND_L_S
/* 304 */     MCD::OPC_FilterValue, 160, 4, 214, 4, 0, // Skip to: 1548
/* 310 */     MCD::OPC_CheckPredicate, 101, 209, 4, 0, // Skip to: 1548
/* 315 */     MCD::OPC_Decode, 237, 17, 219, 1, // Opcode: ROUND_L_D64
/* 320 */     MCD::OPC_FilterValue, 9, 35, 0, 0, // Skip to: 360
/* 325 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 328 */     MCD::OPC_FilterValue, 128, 4, 10, 0, 0, // Skip to: 344
/* 334 */     MCD::OPC_CheckPredicate, 100, 185, 4, 0, // Skip to: 1548
/* 339 */     MCD::OPC_Decode, 149, 21, 212, 1, // Opcode: TRUNC_L_S
/* 344 */     MCD::OPC_FilterValue, 160, 4, 174, 4, 0, // Skip to: 1548
/* 350 */     MCD::OPC_CheckPredicate, 101, 169, 4, 0, // Skip to: 1548
/* 355 */     MCD::OPC_Decode, 147, 21, 219, 1, // Opcode: TRUNC_L_D64
/* 360 */     MCD::OPC_FilterValue, 10, 35, 0, 0, // Skip to: 400
/* 365 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 368 */     MCD::OPC_FilterValue, 128, 4, 10, 0, 0, // Skip to: 384
/* 374 */     MCD::OPC_CheckPredicate, 100, 145, 4, 0, // Skip to: 1548
/* 379 */     MCD::OPC_Decode, 197, 7, 212, 1, // Opcode: CEIL_L_S
/* 384 */     MCD::OPC_FilterValue, 160, 4, 134, 4, 0, // Skip to: 1548
/* 390 */     MCD::OPC_CheckPredicate, 101, 129, 4, 0, // Skip to: 1548
/* 395 */     MCD::OPC_Decode, 195, 7, 219, 1, // Opcode: CEIL_L_D64
/* 400 */     MCD::OPC_FilterValue, 11, 35, 0, 0, // Skip to: 440
/* 405 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 408 */     MCD::OPC_FilterValue, 128, 4, 10, 0, 0, // Skip to: 424
/* 414 */     MCD::OPC_CheckPredicate, 100, 105, 4, 0, // Skip to: 1548
/* 419 */     MCD::OPC_Decode, 253, 11, 212, 1, // Opcode: FLOOR_L_S
/* 424 */     MCD::OPC_FilterValue, 160, 4, 94, 4, 0, // Skip to: 1548
/* 430 */     MCD::OPC_CheckPredicate, 101, 89, 4, 0, // Skip to: 1548
/* 435 */     MCD::OPC_Decode, 251, 11, 219, 1, // Opcode: FLOOR_L_D64
/* 440 */     MCD::OPC_FilterValue, 12, 18, 0, 0, // Skip to: 463
/* 445 */     MCD::OPC_CheckPredicate, 100, 74, 4, 0, // Skip to: 1548
/* 450 */     MCD::OPC_CheckField, 16, 10, 160, 4, 66, 4, 0, // Skip to: 1548
/* 458 */     MCD::OPC_Decode, 242, 17, 253, 2, // Opcode: ROUND_W_D64
/* 463 */     MCD::OPC_FilterValue, 13, 18, 0, 0, // Skip to: 486
/* 468 */     MCD::OPC_CheckPredicate, 100, 51, 4, 0, // Skip to: 1548
/* 473 */     MCD::OPC_CheckField, 16, 10, 160, 4, 43, 4, 0, // Skip to: 1548
/* 481 */     MCD::OPC_Decode, 152, 21, 253, 2, // Opcode: TRUNC_W_D64
/* 486 */     MCD::OPC_FilterValue, 14, 18, 0, 0, // Skip to: 509
/* 491 */     MCD::OPC_CheckPredicate, 100, 28, 4, 0, // Skip to: 1548
/* 496 */     MCD::OPC_CheckField, 16, 10, 160, 4, 20, 4, 0, // Skip to: 1548
/* 504 */     MCD::OPC_Decode, 200, 7, 253, 2, // Opcode: CEIL_W_D64
/* 509 */     MCD::OPC_FilterValue, 15, 18, 0, 0, // Skip to: 532
/* 514 */     MCD::OPC_CheckPredicate, 100, 5, 4, 0, // Skip to: 1548
/* 519 */     MCD::OPC_CheckField, 16, 10, 160, 4, 253, 3, 0, // Skip to: 1548
/* 527 */     MCD::OPC_Decode, 128, 12, 253, 2, // Opcode: FLOOR_W_D64
/* 532 */     MCD::OPC_FilterValue, 17, 47, 0, 0, // Skip to: 584
/* 537 */     MCD::OPC_ExtractField, 16, 2,  // Inst{17-16} ...
/* 540 */     MCD::OPC_FilterValue, 0, 17, 0, 0, // Skip to: 562
/* 545 */     MCD::OPC_CheckPredicate, 102, 230, 3, 0, // Skip to: 1548
/* 550 */     MCD::OPC_CheckField, 21, 5, 17, 223, 3, 0, // Skip to: 1548
/* 557 */     MCD::OPC_Decode, 199, 15, 254, 2, // Opcode: MOVF_D64
/* 562 */     MCD::OPC_FilterValue, 1, 213, 3, 0, // Skip to: 1548
/* 567 */     MCD::OPC_CheckPredicate, 102, 208, 3, 0, // Skip to: 1548
/* 572 */     MCD::OPC_CheckField, 21, 5, 17, 201, 3, 0, // Skip to: 1548
/* 579 */     MCD::OPC_Decode, 219, 15, 254, 2, // Opcode: MOVT_D64
/* 584 */     MCD::OPC_FilterValue, 18, 17, 0, 0, // Skip to: 606
/* 589 */     MCD::OPC_CheckPredicate, 102, 186, 3, 0, // Skip to: 1548
/* 594 */     MCD::OPC_CheckField, 21, 5, 17, 179, 3, 0, // Skip to: 1548
/* 601 */     MCD::OPC_Decode, 231, 15, 255, 2, // Opcode: MOVZ_I_D64
/* 606 */     MCD::OPC_FilterValue, 19, 17, 0, 0, // Skip to: 628
/* 611 */     MCD::OPC_CheckPredicate, 102, 164, 3, 0, // Skip to: 1548
/* 616 */     MCD::OPC_CheckField, 21, 5, 17, 157, 3, 0, // Skip to: 1548
/* 623 */     MCD::OPC_Decode, 211, 15, 255, 2, // Opcode: MOVN_I_D64
/* 628 */     MCD::OPC_FilterValue, 21, 18, 0, 0, // Skip to: 651
/* 633 */     MCD::OPC_CheckPredicate, 103, 142, 3, 0, // Skip to: 1548
/* 638 */     MCD::OPC_CheckField, 16, 10, 160, 4, 134, 3, 0, // Skip to: 1548
/* 646 */     MCD::OPC_Decode, 217, 17, 219, 1, // Opcode: RECIP_D64
/* 651 */     MCD::OPC_FilterValue, 22, 18, 0, 0, // Skip to: 674
/* 656 */     MCD::OPC_CheckPredicate, 103, 119, 3, 0, // Skip to: 1548
/* 661 */     MCD::OPC_CheckField, 16, 10, 160, 4, 111, 3, 0, // Skip to: 1548
/* 669 */     MCD::OPC_Decode, 250, 17, 219, 1, // Opcode: RSQRT_D64
/* 674 */     MCD::OPC_FilterValue, 32, 51, 0, 0, // Skip to: 730
/* 679 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 682 */     MCD::OPC_FilterValue, 160, 4, 10, 0, 0, // Skip to: 698
/* 688 */     MCD::OPC_CheckPredicate, 98, 87, 3, 0, // Skip to: 1548
/* 693 */     MCD::OPC_Decode, 136, 9, 253, 2, // Opcode: CVT_S_D64
/* 698 */     MCD::OPC_FilterValue, 160, 5, 10, 0, 0, // Skip to: 714
/* 704 */     MCD::OPC_CheckPredicate, 104, 71, 3, 0, // Skip to: 1548
/* 709 */     MCD::OPC_Decode, 138, 9, 253, 2, // Opcode: CVT_S_L
/* 714 */     MCD::OPC_FilterValue, 192, 5, 60, 3, 0, // Skip to: 1548
/* 720 */     MCD::OPC_CheckPredicate, 105, 55, 3, 0, // Skip to: 1548
/* 725 */     MCD::OPC_Decode, 141, 9, 253, 2, // Opcode: CVT_S_PU64
/* 730 */     MCD::OPC_FilterValue, 33, 51, 0, 0, // Skip to: 786
/* 735 */     MCD::OPC_ExtractField, 16, 10,  // Inst{25-16} ...
/* 738 */     MCD::OPC_FilterValue, 128, 4, 10, 0, 0, // Skip to: 754
/* 744 */     MCD::OPC_CheckPredicate, 98, 31, 3, 0, // Skip to: 1548
/* 749 */     MCD::OPC_Decode, 250, 8, 212, 1, // Opcode: CVT_D64_S
/* 754 */     MCD::OPC_FilterValue, 128, 5, 10, 0, 0, // Skip to: 770
/* 760 */     MCD::OPC_CheckPredicate, 98, 15, 3, 0, // Skip to: 1548
/* 765 */     MCD::OPC_Decode, 252, 8, 212, 1, // Opcode: CVT_D64_W
/* 770 */     MCD::OPC_FilterValue, 160, 5, 4, 3, 0, // Skip to: 1548
/* 776 */     MCD::OPC_CheckPredicate, 104, 255, 2, 0, // Skip to: 1548
/* 781 */     MCD::OPC_Decode, 249, 8, 219, 1, // Opcode: CVT_D64_L
/* 786 */     MCD::OPC_FilterValue, 36, 18, 0, 0, // Skip to: 809
/* 791 */     MCD::OPC_CheckPredicate, 98, 240, 2, 0, // Skip to: 1548
/* 796 */     MCD::OPC_CheckField, 16, 10, 160, 4, 232, 2, 0, // Skip to: 1548
/* 804 */     MCD::OPC_Decode, 147, 9, 253, 2, // Opcode: CVT_W_D64
/* 809 */     MCD::OPC_FilterValue, 38, 17, 0, 0, // Skip to: 831
/* 814 */     MCD::OPC_CheckPredicate, 105, 217, 2, 0, // Skip to: 1548
/* 819 */     MCD::OPC_CheckField, 21, 5, 16, 210, 2, 0, // Skip to: 1548
/* 826 */     MCD::OPC_Decode, 133, 9, 128, 3, // Opcode: CVT_PS_S64
/* 831 */     MCD::OPC_FilterValue, 40, 18, 0, 0, // Skip to: 854
/* 836 */     MCD::OPC_CheckPredicate, 105, 195, 2, 0, // Skip to: 1548
/* 841 */     MCD::OPC_CheckField, 16, 10, 192, 5, 187, 2, 0, // Skip to: 1548
/* 849 */     MCD::OPC_Decode, 140, 9, 253, 2, // Opcode: CVT_S_PL64
/* 854 */     MCD::OPC_FilterValue, 44, 17, 0, 0, // Skip to: 876
/* 859 */     MCD::OPC_CheckPredicate, 105, 172, 2, 0, // Skip to: 1548
/* 864 */     MCD::OPC_CheckField, 21, 5, 22, 165, 2, 0, // Skip to: 1548
/* 871 */     MCD::OPC_Decode, 160, 17, 219, 2, // Opcode: PLL_PS64
/* 876 */     MCD::OPC_FilterValue, 45, 17, 0, 0, // Skip to: 898
/* 881 */     MCD::OPC_CheckPredicate, 105, 150, 2, 0, // Skip to: 1548
/* 886 */     MCD::OPC_CheckField, 21, 5, 22, 143, 2, 0, // Skip to: 1548
/* 893 */     MCD::OPC_Decode, 161, 17, 219, 2, // Opcode: PLU_PS64
/* 898 */     MCD::OPC_FilterValue, 48, 24, 0, 0, // Skip to: 927
/* 903 */     MCD::OPC_CheckPredicate, 106, 128, 2, 0, // Skip to: 1548
/* 908 */     MCD::OPC_CheckField, 21, 5, 17, 121, 2, 0, // Skip to: 1548
/* 915 */     MCD::OPC_CheckField, 6, 2, 0, 114, 2, 0, // Skip to: 1548
/* 922 */     MCD::OPC_Decode, 160, 9, 129, 3, // Opcode: C_F_D64
/* 927 */     MCD::OPC_FilterValue, 49, 24, 0, 0, // Skip to: 956
/* 932 */     MCD::OPC_CheckPredicate, 106, 99, 2, 0, // Skip to: 1548
/* 937 */     MCD::OPC_CheckField, 21, 5, 17, 92, 2, 0, // Skip to: 1548
/* 944 */     MCD::OPC_CheckField, 6, 2, 0, 85, 2, 0, // Skip to: 1548
/* 951 */     MCD::OPC_Decode, 244, 9, 129, 3, // Opcode: C_UN_D64
/* 956 */     MCD::OPC_FilterValue, 50, 24, 0, 0, // Skip to: 985
/* 961 */     MCD::OPC_CheckPredicate, 106, 70, 2, 0, // Skip to: 1548
/* 966 */     MCD::OPC_CheckField, 21, 5, 17, 63, 2, 0, // Skip to: 1548
/* 973 */     MCD::OPC_CheckField, 6, 2, 0, 56, 2, 0, // Skip to: 1548
/* 980 */     MCD::OPC_Decode, 154, 9, 129, 3, // Opcode: C_EQ_D64
/* 985 */     MCD::OPC_FilterValue, 51, 24, 0, 0, // Skip to: 1014
/* 990 */     MCD::OPC_CheckPredicate, 106, 41, 2, 0, // Skip to: 1548
/* 995 */     MCD::OPC_CheckField, 21, 5, 17, 34, 2, 0, // Skip to: 1548
/* 1002 */    MCD::OPC_CheckField, 6, 2, 0, 27, 2, 0, // Skip to: 1548
/* 1009 */    MCD::OPC_Decode, 226, 9, 129, 3, // Opcode: C_UEQ_D64
/* 1014 */    MCD::OPC_FilterValue, 52, 24, 0, 0, // Skip to: 1043
/* 1019 */    MCD::OPC_CheckPredicate, 106, 12, 2, 0, // Skip to: 1548
/* 1024 */    MCD::OPC_CheckField, 21, 5, 17, 5, 2, 0, // Skip to: 1548
/* 1031 */    MCD::OPC_CheckField, 6, 2, 0, 254, 1, 0, // Skip to: 1548
/* 1038 */    MCD::OPC_Decode, 208, 9, 129, 3, // Opcode: C_OLT_D64
/* 1043 */    MCD::OPC_FilterValue, 53, 24, 0, 0, // Skip to: 1072
/* 1048 */    MCD::OPC_CheckPredicate, 106, 239, 1, 0, // Skip to: 1548
/* 1053 */    MCD::OPC_CheckField, 21, 5, 17, 232, 1, 0, // Skip to: 1548
/* 1060 */    MCD::OPC_CheckField, 6, 2, 0, 225, 1, 0, // Skip to: 1548
/* 1067 */    MCD::OPC_Decode, 238, 9, 129, 3, // Opcode: C_ULT_D64
/* 1072 */    MCD::OPC_FilterValue, 54, 24, 0, 0, // Skip to: 1101
/* 1077 */    MCD::OPC_CheckPredicate, 106, 210, 1, 0, // Skip to: 1548
/* 1082 */    MCD::OPC_CheckField, 21, 5, 17, 203, 1, 0, // Skip to: 1548
/* 1089 */    MCD::OPC_CheckField, 6, 2, 0, 196, 1, 0, // Skip to: 1548
/* 1096 */    MCD::OPC_Decode, 202, 9, 129, 3, // Opcode: C_OLE_D64
/* 1101 */    MCD::OPC_FilterValue, 55, 24, 0, 0, // Skip to: 1130
/* 1106 */    MCD::OPC_CheckPredicate, 106, 181, 1, 0, // Skip to: 1548
/* 1111 */    MCD::OPC_CheckField, 21, 5, 17, 174, 1, 0, // Skip to: 1548
/* 1118 */    MCD::OPC_CheckField, 6, 2, 0, 167, 1, 0, // Skip to: 1548
/* 1125 */    MCD::OPC_Decode, 232, 9, 129, 3, // Opcode: C_ULE_D64
/* 1130 */    MCD::OPC_FilterValue, 56, 24, 0, 0, // Skip to: 1159
/* 1135 */    MCD::OPC_CheckPredicate, 106, 152, 1, 0, // Skip to: 1548
/* 1140 */    MCD::OPC_CheckField, 21, 5, 17, 145, 1, 0, // Skip to: 1548
/* 1147 */    MCD::OPC_CheckField, 6, 2, 0, 138, 1, 0, // Skip to: 1548
/* 1154 */    MCD::OPC_Decode, 220, 9, 129, 3, // Opcode: C_SF_D64
/* 1159 */    MCD::OPC_FilterValue, 57, 24, 0, 0, // Skip to: 1188
/* 1164 */    MCD::OPC_CheckPredicate, 106, 123, 1, 0, // Skip to: 1548
/* 1169 */    MCD::OPC_CheckField, 21, 5, 17, 116, 1, 0, // Skip to: 1548
/* 1176 */    MCD::OPC_CheckField, 6, 2, 0, 109, 1, 0, // Skip to: 1548
/* 1183 */    MCD::OPC_Decode, 184, 9, 129, 3, // Opcode: C_NGLE_D64
/* 1188 */    MCD::OPC_FilterValue, 58, 24, 0, 0, // Skip to: 1217
/* 1193 */    MCD::OPC_CheckPredicate, 106, 94, 1, 0, // Skip to: 1548
/* 1198 */    MCD::OPC_CheckField, 21, 5, 17, 87, 1, 0, // Skip to: 1548
/* 1205 */    MCD::OPC_CheckField, 6, 2, 0, 80, 1, 0, // Skip to: 1548
/* 1212 */    MCD::OPC_Decode, 214, 9, 129, 3, // Opcode: C_SEQ_D64
/* 1217 */    MCD::OPC_FilterValue, 59, 24, 0, 0, // Skip to: 1246
/* 1222 */    MCD::OPC_CheckPredicate, 106, 65, 1, 0, // Skip to: 1548
/* 1227 */    MCD::OPC_CheckField, 21, 5, 17, 58, 1, 0, // Skip to: 1548
/* 1234 */    MCD::OPC_CheckField, 6, 2, 0, 51, 1, 0, // Skip to: 1548
/* 1241 */    MCD::OPC_Decode, 190, 9, 129, 3, // Opcode: C_NGL_D64
/* 1246 */    MCD::OPC_FilterValue, 60, 24, 0, 0, // Skip to: 1275
/* 1251 */    MCD::OPC_CheckPredicate, 106, 36, 1, 0, // Skip to: 1548
/* 1256 */    MCD::OPC_CheckField, 21, 5, 17, 29, 1, 0, // Skip to: 1548
/* 1263 */    MCD::OPC_CheckField, 6, 2, 0, 22, 1, 0, // Skip to: 1548
/* 1270 */    MCD::OPC_Decode, 172, 9, 129, 3, // Opcode: C_LT_D64
/* 1275 */    MCD::OPC_FilterValue, 61, 24, 0, 0, // Skip to: 1304
/* 1280 */    MCD::OPC_CheckPredicate, 106, 7, 1, 0, // Skip to: 1548
/* 1285 */    MCD::OPC_CheckField, 21, 5, 17, 0, 1, 0, // Skip to: 1548
/* 1292 */    MCD::OPC_CheckField, 6, 2, 0, 249, 0, 0, // Skip to: 1548
/* 1299 */    MCD::OPC_Decode, 178, 9, 129, 3, // Opcode: C_NGE_D64
/* 1304 */    MCD::OPC_FilterValue, 62, 24, 0, 0, // Skip to: 1333
/* 1309 */    MCD::OPC_CheckPredicate, 106, 234, 0, 0, // Skip to: 1548
/* 1314 */    MCD::OPC_CheckField, 21, 5, 17, 227, 0, 0, // Skip to: 1548
/* 1321 */    MCD::OPC_CheckField, 6, 2, 0, 220, 0, 0, // Skip to: 1548
/* 1328 */    MCD::OPC_Decode, 166, 9, 129, 3, // Opcode: C_LE_D64
/* 1333 */    MCD::OPC_FilterValue, 63, 210, 0, 0, // Skip to: 1548
/* 1338 */    MCD::OPC_CheckPredicate, 106, 205, 0, 0, // Skip to: 1548
/* 1343 */    MCD::OPC_CheckField, 21, 5, 17, 198, 0, 0, // Skip to: 1548
/* 1350 */    MCD::OPC_CheckField, 6, 2, 0, 191, 0, 0, // Skip to: 1548
/* 1357 */    MCD::OPC_Decode, 196, 9, 129, 3, // Opcode: C_NGT_D64
/* 1362 */    MCD::OPC_FilterValue, 19, 151, 0, 0, // Skip to: 1518
/* 1367 */    MCD::OPC_ExtractField, 0, 6,  // Inst{5-0} ...
/* 1370 */    MCD::OPC_FilterValue, 1, 17, 0, 0, // Skip to: 1392
/* 1375 */    MCD::OPC_CheckPredicate, 107, 168, 0, 0, // Skip to: 1548
/* 1380 */    MCD::OPC_CheckField, 11, 5, 0, 161, 0, 0, // Skip to: 1548
/* 1387 */    MCD::OPC_Decode, 219, 13, 130, 3, // Opcode: LDXC164
/* 1392 */    MCD::OPC_FilterValue, 5, 17, 0, 0, // Skip to: 1414
/* 1397 */    MCD::OPC_CheckPredicate, 108, 146, 0, 0, // Skip to: 1548
/* 1402 */    MCD::OPC_CheckField, 11, 5, 0, 139, 0, 0, // Skip to: 1548
/* 1409 */    MCD::OPC_Decode, 129, 14, 130, 3, // Opcode: LUXC164
/* 1414 */    MCD::OPC_FilterValue, 9, 17, 0, 0, // Skip to: 1436
/* 1419 */    MCD::OPC_CheckPredicate, 107, 124, 0, 0, // Skip to: 1548
/* 1424 */    MCD::OPC_CheckField, 6, 5, 0, 117, 0, 0, // Skip to: 1548
/* 1431 */    MCD::OPC_Decode, 176, 18, 131, 3, // Opcode: SDXC164
/* 1436 */    MCD::OPC_FilterValue, 13, 17, 0, 0, // Skip to: 1458
/* 1441 */    MCD::OPC_CheckPredicate, 108, 102, 0, 0, // Skip to: 1548
/* 1446 */    MCD::OPC_CheckField, 6, 5, 0, 95, 0, 0, // Skip to: 1548
/* 1453 */    MCD::OPC_Decode, 163, 20, 131, 3, // Opcode: SUXC164
/* 1458 */    MCD::OPC_FilterValue, 33, 10, 0, 0, // Skip to: 1473
/* 1463 */    MCD::OPC_CheckPredicate, 109, 80, 0, 0, // Skip to: 1548
/* 1468 */    MCD::OPC_Decode, 203, 14, 132, 3, // Opcode: MADD_D64
/* 1473 */    MCD::OPC_FilterValue, 41, 10, 0, 0, // Skip to: 1488
/* 1478 */    MCD::OPC_CheckPredicate, 109, 65, 0, 0, // Skip to: 1548
/* 1483 */    MCD::OPC_Decode, 254, 15, 132, 3, // Opcode: MSUB_D64
/* 1488 */    MCD::OPC_FilterValue, 49, 10, 0, 0, // Skip to: 1503
/* 1493 */    MCD::OPC_CheckPredicate, 110, 50, 0, 0, // Skip to: 1548
/* 1498 */    MCD::OPC_Decode, 236, 16, 132, 3, // Opcode: NMADD_D64
/* 1503 */    MCD::OPC_FilterValue, 57, 40, 0, 0, // Skip to: 1548
/* 1508 */    MCD::OPC_CheckPredicate, 110, 35, 0, 0, // Skip to: 1548
/* 1513 */    MCD::OPC_Decode, 241, 16, 132, 3, // Opcode: NMSUB_D64
/* 1518 */    MCD::OPC_FilterValue, 53, 10, 0, 0, // Skip to: 1533
/* 1523 */    MCD::OPC_CheckPredicate, 100, 20, 0, 0, // Skip to: 1548
/* 1528 */    MCD::OPC_Decode, 204, 13, 208, 2, // Opcode: LDC164
/* 1533 */    MCD::OPC_FilterValue, 61, 10, 0, 0, // Skip to: 1548
/* 1538 */    MCD::OPC_CheckPredicate, 100, 5, 0, 0, // Skip to: 1548
/* 1543 */    MCD::OPC_Decode, 164, 18, 208, 2, // Opcode: SDC164
/* 1548 */    MCD::OPC_Fail,
  0
};

static bool checkDecoderPredicate(unsigned Idx, const FeatureBitset& Bits) {
  switch (Idx) {
  default: llvm_unreachable("Invalid index!");
  case 0:
    return (Bits[Mips::FeatureMips16]);
  case 1:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureCnMips] && !Bits[Mips::FeatureMicroMips]);
  case 2:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureCnMips] && !Bits[Mips::FeatureMicroMips]);
  case 3:
    return (Bits[Mips::FeatureCnMips]);
  case 4:
    return (Bits[Mips::FeatureMips64] && Bits[Mips::FeatureCnMips] && !Bits[Mips::FeatureMicroMips]);
  case 5:
    return (Bits[Mips::FeatureCnMipsP]);
  case 6:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMips32r6]);
  case 7:
    return (Bits[Mips::FeatureMicroMips]);
  case 8:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureDSP]);
  case 9:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureDSPR2]);
  case 10:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureMips32r5] && Bits[Mips::FeatureVirt]);
  case 11:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureDSPR3]);
  case 12:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMips32r6] && Bits[Mips::FeatureDSP]);
  case 13:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat]);
  case 14:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMadd4]);
  case 15:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat]);
  case 16:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMadd4]);
  case 17:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat]);
  case 18:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureSoftFloat]);
  case 19:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureSoftFloat]);
  case 20:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureSoftFloat]);
  case 21:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureEVA]);
  case 22:
    return (Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMips32r6] && Bits[Mips::FeatureEVA]);
  case 23:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat]);
  case 24:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureMips32r6]);
  case 25:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureMips32r6] && Bits[Mips::FeatureGINV]);
  case 26:
    return (Bits[Mips::FeatureMicroMips] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat]);
  case 27:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureMicroMips]);
  case 28:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureMicroMips]);
  case 29:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 30:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMSA]);
  case 31:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 32:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6]);
  case 33:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureUseIndirectJumpsHazard]);
  case 34:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32]);
  case 35:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 36:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureMicroMips]);
  case 37:
    return (Bits[Mips::FeatureDSP]);
  case 38:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMSA] && Bits[Mips::FeatureMips64]);
  case 39:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 40:
    return (Bits[Mips::FeatureDSP] && !Bits[Mips::FeatureMicroMips]);
  case 41:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r5] && Bits[Mips::FeatureVirt] && !Bits[Mips::FeatureMicroMips]);
  case 42:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMT] && !Bits[Mips::FeatureMicroMips]);
  case 43:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3_32] && !Bits[Mips::FeatureMicroMips]);
  case 44:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r2] && Bits[Mips::FeatureEVA] && !Bits[Mips::FeatureMicroMips]);
  case 45:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r5] && !Bits[Mips::FeatureMicroMips]);
  case 46:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32] && !Bits[Mips::FeatureMicroMips]);
  case 47:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 48:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 49:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 50:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 51:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 52:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 53:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureSoftFloat]);
  case 54:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 55:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 56:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3_32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 57:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 58:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 59:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 60:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 61:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat]);
  case 62:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 63:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips5_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 64:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMadd4]);
  case 65:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMadd4]);
  case 66:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMadd4] && !Bits[Mips::FeatureMicroMips]);
  case 67:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMadd4] && !Bits[Mips::FeatureMicroMips]);
  case 68:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 69:
    return (Bits[Mips::FeatureDSPR2]);
  case 70:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && Bits[Mips::FeatureEVA] && !Bits[Mips::FeatureMicroMips]);
  case 71:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3_32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 72:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeaturePTR64Bit] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 73:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeaturePTR64Bit] && !Bits[Mips::FeatureMicroMips]);
  case 74:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeaturePTR64Bit] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 75:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r6]);
  case 76:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMicroMips]);
  case 77:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 78:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeatureGP64Bit] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMicroMips]);
  case 79:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 80:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r6] && Bits[Mips::FeatureCRC] && !Bits[Mips::FeatureMicroMips]);
  case 81:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64r6] && Bits[Mips::FeatureCRC] && !Bits[Mips::FeatureMicroMips]);
  case 82:
    return (!Bits[Mips::FeatureMips16] && !Bits[Mips::FeaturePTR64Bit] && Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMicroMips]);
  case 83:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips32r6] && Bits[Mips::FeatureGINV] && !Bits[Mips::FeatureMicroMips]);
  case 84:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64r6]);
  case 85:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureGP64Bit] && Bits[Mips::FeatureMips64r6]);
  case 86:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureGP64Bit] && Bits[Mips::FeatureMips32r6]);
  case 87:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeaturePTR64Bit] && Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 88:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 89:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeaturePTR64Bit]);
  case 90:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64r2] && !Bits[Mips::FeatureMicroMips]);
  case 91:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3] && !Bits[Mips::FeatureMicroMips]);
  case 92:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 93:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureGP64Bit] && Bits[Mips::FeatureMips3]);
  case 94:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips64r5] && Bits[Mips::FeatureVirt]);
  case 95:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureMips3] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6]);
  case 96:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureGP64Bit] && Bits[Mips::FeatureMips64] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureMicroMips]);
  case 97:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureDSP]);
  case 98:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 99:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 100:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 101:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips3_32] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 102:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 103:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 104:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips3_32r2] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 105:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 106:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 107:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat]);
  case 108:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips5_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips]);
  case 109:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMicroMips] && !Bits[Mips::FeatureMadd4]);
  case 110:
    return (!Bits[Mips::FeatureMips16] && Bits[Mips::FeatureFP64Bit] && Bits[Mips::FeatureMips4_32r2] && !Bits[Mips::FeatureMips32r6] && !Bits[Mips::FeatureMips64r6] && !Bits[Mips::FeatureSoftFloat] && !Bits[Mips::FeatureMadd4] && !Bits[Mips::FeatureMicroMips]);
  }
}

template<typename InsnType>
static DecodeStatus decodeToMCInst(DecodeStatus S, unsigned Idx, InsnType insn, MCInst &MI,
                                   uint64_t Address, const void *Decoder, bool &DecodeComplete) {
  DecodeComplete = true;
  InsnType tmp;
  switch (Idx) {
  default: llvm_unreachable("Invalid index!");
  case 0:
    return S;
  case 1:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 2:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 3:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 3, 2) << 3;
    tmp |= fieldFromInstruction(insn, 5, 3) << 0;
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 4:
    tmp = fieldFromInstruction(insn, 0, 4);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 5:
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 5, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 6:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 5, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 7:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 5, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 8:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 0, 5) << 0;
    tmp |= fieldFromInstruction(insn, 16, 5) << 11;
    tmp |= fieldFromInstruction(insn, 21, 6) << 5;
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 9:
    tmp = fieldFromInstruction(insn, 5, 3);
    if (DecodeCPU16RegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 10:
    if (DecodeFMem3(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 11:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 12:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 13:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 14:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 15:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 16:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 10);
    if (DecodeSImmWithOffsetAndScale<10>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 17:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 18:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 19:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 20:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 1, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 21:
    if (DecodeMemMMImm4(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 22:
    tmp = fieldFromInstruction(insn, 5, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 23:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 1, 3);
    if (DecodePOOL16BEncodedField(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 24:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 4);
    if (DecodeANDI16Imm(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 25:
    tmp = fieldFromInstruction(insn, 3, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 26:
    tmp = fieldFromInstruction(insn, 3, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 3, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 27:
    if (DecodeMemMMReglistImm4Lsl2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 28:
    tmp = fieldFromInstruction(insn, 0, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 29:
    tmp = fieldFromInstruction(insn, 0, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 30:
    tmp = fieldFromInstruction(insn, 0, 5);
    if (DecodeUImmWithOffsetAndScale<5, 0, 4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 31:
    if (DecodeMemMMSPImm5Lsl2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 32:
    tmp = fieldFromInstruction(insn, 5, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 5, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 1, 4);
    if (DecodeSImmWithOffsetAndScale<4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 33:
    tmp = fieldFromInstruction(insn, 1, 9);
    if (DecodeSimm9SP(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 34:
    if (DecodeMemMMGPImm7Lsl2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 35:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 1, 3);
    if (DecodeAddiur2Simm7(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 36:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 1, 6);
    if (DecodeUImmWithOffsetAndScale<6, 0, 4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 37:
    if (DecodeMovePOperands(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 38:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 7);
    if (DecodeBranchTarget7MM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 39:
    tmp = fieldFromInstruction(insn, 0, 10);
    if (DecodeBranchTarget10MM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 40:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 7);
    if (DecodeLi16Imm(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 41:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 42:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 43:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 44:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 45:
    tmp = fieldFromInstruction(insn, 16, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 6, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 46:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeInsSize(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 47:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 48:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 49:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 50:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 51:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 52:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 53:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 54:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 6);
    if (DecodeSImmWithOffsetAndScale<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 55:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeUImmWithOffset<5, 1>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 56:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 57:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 58:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 59:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 60:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 61:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 62:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeHI32DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 63:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 13, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 64:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeLO32DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 65:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 66:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 67:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 68:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 69:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 70:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 71:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 72:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 73:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 74:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeHWRegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 75:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 76:
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 77:
    tmp = fieldFromInstruction(insn, 16, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 78:
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 79:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 80:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 81:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 82:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 13, 8);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 83:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 7);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 84:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 14, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 85:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 10);
    if (DecodeSImmWithOffsetAndScale<10>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 86:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeSImmWithOffsetAndScale<16>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 87:
    if (DecodeMemMMImm16(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 88:
    if (DecodeMemMMImm12(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 89:
    if (DecodeCacheOpMM(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 90:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 91:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeSImmWithOffsetAndScale<16>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 92:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 93:
    if (DecodeSyncI_MM(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 94:
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget1SImm16(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 95:
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 96:
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 97:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 98:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 99:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 100:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 101:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 102:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 103:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 104:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 105:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 106:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 107:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 108:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 109:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 110:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 111:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 112:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 113:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 114:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 115:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 116:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCCRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 117:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 118:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 119:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCCRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 120:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 121:
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 122:
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 123:
    if (DecodeMemMMImm9(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 124:
    if (DecodePrefeOpMM(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 125:
    if (DecodeJumpTargetMM(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 126:
    tmp = fieldFromInstruction(insn, 23, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 23);
    if (DecodeSimm23Lsl2(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 127:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 128:
    if (DecodeFMemMMR2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 129:
    if (DecodeJumpTargetXMM(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 130:
    if (DecodeMem(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 131:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 132:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 133:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 134:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 135:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 136:
    tmp = fieldFromInstruction(insn, 1, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 137:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 138:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 4, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRMM16RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 139:
    tmp = fieldFromInstruction(insn, 5, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 140:
    tmp = fieldFromInstruction(insn, 5, 5);
    if (DecodeUImmWithOffsetAndScale<5, 0, 4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 141:
    tmp = fieldFromInstruction(insn, 6, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 142:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeHWRegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 143:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 9, 2);
    if (DecodeUImmWithOffset<2, 1>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 144:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 9, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 145:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 9, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 146:
    tmp = fieldFromInstruction(insn, 6, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 147:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 148:
    if (DecodeLoadByte15(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 149:
    if (DecodeFMemCop2MMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 150:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 151:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTargetMM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 152:
    if (DecodeSynciR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 153:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 154:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 155:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 156:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 157:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 158:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 159:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 160:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 161:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 162:
    if (DecodePOP35GroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 163:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 19);
    if (DecodeSimm19Lsl2(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 164:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeSImmWithOffsetAndScale<16>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 165:
    if (DecodePOP37GroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 166:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 21);
    if (DecodeBranchTarget21MM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 167:
    tmp = fieldFromInstruction(insn, 0, 26);
    if (DecodeBranchTarget26MM(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 168:
    if (DecodeBlezGroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 169:
    if (DecodePOP65GroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 170:
    if (DecodeBgtzGroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 171:
    if (DecodePOP75GroupBranchMMR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 172:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 173:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 174:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 2);
    if (DecodeUImmWithOffset<2, 1>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 175:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 176:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 177:
    tmp = fieldFromInstruction(insn, 6, 20);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 178:
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 179:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 180:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 181:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeHI32DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 182:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeLO32DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 183:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 2);
    if (DecodeUImmWithOffset<2, 1>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 184:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 185:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 186:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 187:
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 188:
    if (DecodeSyncI(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 189:
    if (DecodeJumpTarget(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 190:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 191:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeSImmWithOffsetAndScale<16>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 192:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 193:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 194:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 195:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 5, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 4, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 196:
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 197:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 198:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 199:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCCRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 200:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 201:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 202:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 203:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCCRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 204:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 205:
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 206:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 207:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 208:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 209:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 210:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 211:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 212:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 213:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 214:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 215:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 216:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 217:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 218:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 219:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 220:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 221:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 222:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 223:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 224:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 225:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 226:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 227:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 228:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 229:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 230:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 231:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeAFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 232:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 233:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 11, 5) << 0;
    tmp |= fieldFromInstruction(insn, 16, 5) << 0;
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 234:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 8);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 235:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 8);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 236:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 8);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 237:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 8);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 238:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 239:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 240:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 241:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 242:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 243:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 244:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 245:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 246:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 6);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 247:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 248:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 249:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 6);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 250:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 251:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 252:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 253:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 254:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 255:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 256:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 257:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 258:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 259:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 260:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 261:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 262:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 263:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 264:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 265:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 266:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 267:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 268:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 269:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 270:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 271:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 272:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 273:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 274:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 275:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 276:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 277:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 278:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 279:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSACtrlRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 280:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 281:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 282:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 283:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 284:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSACtrlRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 285:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 286:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 287:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 288:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 289:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 290:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 4);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 291:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 292:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 293:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 1);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 294:
    if (DecodeINSVE_DF(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 295:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 296:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 297:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128BRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 298:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 299:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 300:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 301:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 302:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 303:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 304:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128HRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 305:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeMSA128DRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeMSA128WRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 306:
    if (DecodeMSA128Mem(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 307:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeUImmWithOffset<5, 1>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 308:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeInsSize(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 309:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 310:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 311:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 312:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 313:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 314:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 315:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 316:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 317:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 318:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 319:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 320:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 321:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 322:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 323:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeDSPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 324:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 325:
    if (DecodeMemEVA(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 326:
    if (DecodeCacheeOp_CacheOpR6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 327:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 328:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 329:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 330:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 331:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 10);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 332:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 6);
    if (DecodeSImmWithOffsetAndScale<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 333:
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 2);
    if (DecodeACC64DSPRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 334:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeHWRegsRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 335:
    if (DecodeCacheOp(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 336:
    if (DecodeFMem(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 337:
    if (DecodeFMem2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 338:
    if (DecodeDAHIDATI(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 339:
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 340:
    if (DecodeBlezGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 341:
    if (DecodeBgtzGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 342:
    if (DecodeAddiGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 343:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 344:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 345:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 346:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 347:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 348:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 349:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGRCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 350:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeBranchTarget(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 351:
    if (DecodeFMemCop2R6(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 352:
    if (DecodeBlezlGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 353:
    if (DecodeBgtzlGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 354:
    if (DecodeDaddiGroupBranch(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 355:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 356:
    if (DecodeCRC(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 357:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 358:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 359:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 360:
    if (DecodeSpecial3LlSc(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 361:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 8, 2);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 362:
    tmp = fieldFromInstruction(insn, 0, 26);
    if (DecodeBranchTarget26(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 363:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 21);
    if (DecodeBranchTarget21(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 364:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 18);
    if (DecodeSimm18Lsl3(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 365:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 21);
    if (DecodeBranchTarget21(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 366:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 367:
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 368:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 369:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 370:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 371:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 372:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeCOP2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 3);
    MI.addOperand(MCOperand::createImm(tmp));
    return S;
  case 373:
    tmp = fieldFromInstruction(insn, 13, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 374:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 0, 16);
    if (DecodeSImmWithOffsetAndScale<16>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 375:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 11, 5) << 0;
    tmp |= fieldFromInstruction(insn, 16, 5) << 0;
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeGPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 376:
    if (DecodeDEXT(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 377:
    if (DecodeDINS(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 378:
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 379:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 380:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 381:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 382:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 18, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 383:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeGPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 384:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 385:
    tmp = fieldFromInstruction(insn, 8, 3);
    if (DecodeFCCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 386:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 387:
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodePtrRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 388:
    tmp = fieldFromInstruction(insn, 6, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 21, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 11, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 16, 5);
    if (DecodeFGR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  }
}

template<typename InsnType>
static DecodeStatus decodeInstruction(const uint8_t DecodeTable[], MCInst &MI,
                                      InsnType insn, uint64_t Address,
                                      const void *DisAsm,
                                      const MCSubtargetInfo &STI) {
  const FeatureBitset& Bits = STI.getFeatureBits();

  const uint8_t *Ptr = DecodeTable;
  InsnType CurFieldValue = 0;
  DecodeStatus S = MCDisassembler::Success;
  while (true) {
    ptrdiff_t Loc = Ptr - DecodeTable;
    switch (*Ptr) {
    default:
      errs() << Loc << ": Unexpected decode table opcode!\n";
      return MCDisassembler::Fail;
    case MCD::OPC_ExtractField: {
      unsigned Start = *++Ptr;
      unsigned Len = *++Ptr;
      ++Ptr;
      CurFieldValue = fieldFromInstruction(insn, Start, Len);
      LLVM_DEBUG(dbgs() << Loc << ": OPC_ExtractField(" << Start << ", "
                   << Len << "): " << CurFieldValue << "\n");
      break;
    }
    case MCD::OPC_FilterValue: {
      // Decode the field value.
      unsigned Len;
      InsnType Val = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      // NumToSkip is a plain 24-bit integer.
      unsigned NumToSkip = *Ptr++;
      NumToSkip |= (*Ptr++) << 8;
      NumToSkip |= (*Ptr++) << 16;

      // Perform the filter operation.
      if (Val != CurFieldValue)
        Ptr += NumToSkip;
      LLVM_DEBUG(dbgs() << Loc << ": OPC_FilterValue(" << Val << ", " << NumToSkip
                   << "): " << ((Val != CurFieldValue) ? "FAIL:" : "PASS:")
                   << " continuing at " << (Ptr - DecodeTable) << "\n");

      break;
    }
    case MCD::OPC_CheckField: {
      unsigned Start = *++Ptr;
      unsigned Len = *++Ptr;
      InsnType FieldValue = fieldFromInstruction(insn, Start, Len);
      // Decode the field value.
      InsnType ExpectedValue = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      // NumToSkip is a plain 24-bit integer.
      unsigned NumToSkip = *Ptr++;
      NumToSkip |= (*Ptr++) << 8;
      NumToSkip |= (*Ptr++) << 16;

      // If the actual and expected values don't match, skip.
      if (ExpectedValue != FieldValue)
        Ptr += NumToSkip;
      LLVM_DEBUG(dbgs() << Loc << ": OPC_CheckField(" << Start << ", "
                   << Len << ", " << ExpectedValue << ", " << NumToSkip
                   << "): FieldValue = " << FieldValue << ", ExpectedValue = "
                   << ExpectedValue << ": "
                   << ((ExpectedValue == FieldValue) ? "PASS\n" : "FAIL\n"));
      break;
    }
    case MCD::OPC_CheckPredicate: {
      unsigned Len;
      // Decode the Predicate Index value.
      unsigned PIdx = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      // NumToSkip is a plain 24-bit integer.
      unsigned NumToSkip = *Ptr++;
      NumToSkip |= (*Ptr++) << 8;
      NumToSkip |= (*Ptr++) << 16;
      // Check the predicate.
      bool Pred;
      if (!(Pred = checkDecoderPredicate(PIdx, Bits)))
        Ptr += NumToSkip;
      (void)Pred;
      LLVM_DEBUG(dbgs() << Loc << ": OPC_CheckPredicate(" << PIdx << "): "
            << (Pred ? "PASS\n" : "FAIL\n"));

      break;
    }
    case MCD::OPC_Decode: {
      unsigned Len;
      // Decode the Opcode value.
      unsigned Opc = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      unsigned DecodeIdx = decodeULEB128(Ptr, &Len);
      Ptr += Len;

      MI.clear();
      MI.setOpcode(Opc);
      bool DecodeComplete;
      S = decodeToMCInst(S, DecodeIdx, insn, MI, Address, DisAsm, DecodeComplete);
      assert(DecodeComplete);

      LLVM_DEBUG(dbgs() << Loc << ": OPC_Decode: opcode " << Opc
                   << ", using decoder " << DecodeIdx << ": "
                   << (S != MCDisassembler::Fail ? "PASS" : "FAIL") << "\n");
      return S;
    }
    case MCD::OPC_TryDecode: {
      unsigned Len;
      // Decode the Opcode value.
      unsigned Opc = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      unsigned DecodeIdx = decodeULEB128(Ptr, &Len);
      Ptr += Len;
      // NumToSkip is a plain 24-bit integer.
      unsigned NumToSkip = *Ptr++;
      NumToSkip |= (*Ptr++) << 8;
      NumToSkip |= (*Ptr++) << 16;

      // Perform the decode operation.
      MCInst TmpMI;
      TmpMI.setOpcode(Opc);
      bool DecodeComplete;
      S = decodeToMCInst(S, DecodeIdx, insn, TmpMI, Address, DisAsm, DecodeComplete);
      LLVM_DEBUG(dbgs() << Loc << ": OPC_TryDecode: opcode " << Opc
                   << ", using decoder " << DecodeIdx << ": ");

      if (DecodeComplete) {
        // Decoding complete.
        LLVM_DEBUG(dbgs() << (S != MCDisassembler::Fail ? "PASS" : "FAIL") << "\n");
        MI = TmpMI;
        return S;
      } else {
        assert(S == MCDisassembler::Fail);
        // If the decoding was incomplete, skip.
        Ptr += NumToSkip;
        LLVM_DEBUG(dbgs() << "FAIL: continuing at " << (Ptr - DecodeTable) << "\n");
        // Reset decode status. This also drops a SoftFail status that could be
        // set before the decode attempt.
        S = MCDisassembler::Success;
      }
      break;
    }
    case MCD::OPC_SoftFail: {
      // Decode the mask values.
      unsigned Len;
      InsnType PositiveMask = decodeULEB128(++Ptr, &Len);
      Ptr += Len;
      InsnType NegativeMask = decodeULEB128(Ptr, &Len);
      Ptr += Len;
      bool Fail = (insn & PositiveMask) || (~insn & NegativeMask);
      if (Fail)
        S = MCDisassembler::SoftFail;
      LLVM_DEBUG(dbgs() << Loc << ": OPC_SoftFail: " << (Fail ? "FAIL\n":"PASS\n"));
      break;
    }
    case MCD::OPC_Fail: {
      LLVM_DEBUG(dbgs() << Loc << ": OPC_Fail\n");
      return MCDisassembler::Fail;
    }
    }
  }
  llvm_unreachable("bogosity detected in disassembler state machine!");
}


} // end namespace llvm
