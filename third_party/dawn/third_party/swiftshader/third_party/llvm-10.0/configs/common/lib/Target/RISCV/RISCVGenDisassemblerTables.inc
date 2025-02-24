/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|*  * RISCV Disassembler                                                      *|
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
/* 0 */       MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 3 */       MCD::OPC_FilterValue, 0, 132, 0, 0, // Skip to: 140
/* 8 */       MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 11 */      MCD::OPC_FilterValue, 0, 25, 0, 0, // Skip to: 41
/* 16 */      MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 32
/* 21 */      MCD::OPC_CheckField, 2, 11, 0, 4, 0, 0, // Skip to: 32
/* 28 */      MCD::OPC_Decode, 243, 2, 0, // Opcode: C_UNIMP
/* 32 */      MCD::OPC_CheckPredicate, 0, 24, 3, 0, // Skip to: 829
/* 37 */      MCD::OPC_Decode, 193, 2, 1, // Opcode: C_ADDI4SPN
/* 41 */      MCD::OPC_FilterValue, 1, 41, 0, 0, // Skip to: 87
/* 46 */      MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 62
/* 51 */      MCD::OPC_CheckField, 2, 11, 0, 4, 0, 0, // Skip to: 62
/* 58 */      MCD::OPC_Decode, 227, 2, 0, // Opcode: C_NOP
/* 62 */      MCD::OPC_CheckPredicate, 1, 11, 0, 0, // Skip to: 78
/* 67 */      MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 78
/* 74 */      MCD::OPC_Decode, 228, 2, 2, // Opcode: C_NOP_HINT
/* 78 */      MCD::OPC_CheckPredicate, 0, 234, 2, 0, // Skip to: 829
/* 83 */      MCD::OPC_Decode, 191, 2, 3, // Opcode: C_ADDI
/* 87 */      MCD::OPC_FilterValue, 2, 225, 2, 0, // Skip to: 829
/* 92 */      MCD::OPC_CheckPredicate, 1, 18, 0, 0, // Skip to: 115
/* 97 */      MCD::OPC_CheckField, 12, 1, 0, 11, 0, 0, // Skip to: 115
/* 104 */     MCD::OPC_CheckField, 2, 5, 0, 4, 0, 0, // Skip to: 115
/* 111 */     MCD::OPC_Decode, 233, 2, 4, // Opcode: C_SLLI64_HINT
/* 115 */     MCD::OPC_CheckPredicate, 1, 11, 0, 0, // Skip to: 131
/* 120 */     MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 131
/* 127 */     MCD::OPC_Decode, 234, 2, 5, // Opcode: C_SLLI_HINT
/* 131 */     MCD::OPC_CheckPredicate, 0, 181, 2, 0, // Skip to: 829
/* 136 */     MCD::OPC_Decode, 232, 2, 6, // Opcode: C_SLLI
/* 140 */     MCD::OPC_FilterValue, 1, 45, 0, 0, // Skip to: 190
/* 145 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 148 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 162
/* 153 */     MCD::OPC_CheckPredicate, 2, 159, 2, 0, // Skip to: 829
/* 158 */     MCD::OPC_Decode, 205, 2, 7, // Opcode: C_FLD
/* 162 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 176
/* 167 */     MCD::OPC_CheckPredicate, 3, 145, 2, 0, // Skip to: 829
/* 172 */     MCD::OPC_Decode, 194, 2, 3, // Opcode: C_ADDIW
/* 176 */     MCD::OPC_FilterValue, 2, 136, 2, 0, // Skip to: 829
/* 181 */     MCD::OPC_CheckPredicate, 2, 131, 2, 0, // Skip to: 829
/* 186 */     MCD::OPC_Decode, 206, 2, 8, // Opcode: C_FLDSP
/* 190 */     MCD::OPC_FilterValue, 2, 61, 0, 0, // Skip to: 256
/* 195 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 198 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 212
/* 203 */     MCD::OPC_CheckPredicate, 0, 109, 2, 0, // Skip to: 829
/* 208 */     MCD::OPC_Decode, 223, 2, 9, // Opcode: C_LW
/* 212 */     MCD::OPC_FilterValue, 1, 25, 0, 0, // Skip to: 242
/* 217 */     MCD::OPC_CheckPredicate, 1, 11, 0, 0, // Skip to: 233
/* 222 */     MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 233
/* 229 */     MCD::OPC_Decode, 220, 2, 10, // Opcode: C_LI_HINT
/* 233 */     MCD::OPC_CheckPredicate, 0, 79, 2, 0, // Skip to: 829
/* 238 */     MCD::OPC_Decode, 219, 2, 11, // Opcode: C_LI
/* 242 */     MCD::OPC_FilterValue, 2, 70, 2, 0, // Skip to: 829
/* 247 */     MCD::OPC_CheckPredicate, 0, 65, 2, 0, // Skip to: 829
/* 252 */     MCD::OPC_Decode, 224, 2, 12, // Opcode: C_LWSP
/* 256 */     MCD::OPC_FilterValue, 3, 76, 0, 0, // Skip to: 337
/* 261 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 264 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 278
/* 269 */     MCD::OPC_CheckPredicate, 3, 43, 2, 0, // Skip to: 829
/* 274 */     MCD::OPC_Decode, 217, 2, 13, // Opcode: C_LD
/* 278 */     MCD::OPC_FilterValue, 1, 40, 0, 0, // Skip to: 323
/* 283 */     MCD::OPC_ExtractField, 7, 5,  // Inst{11-7} ...
/* 286 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 300
/* 291 */     MCD::OPC_CheckPredicate, 1, 18, 0, 0, // Skip to: 314
/* 296 */     MCD::OPC_Decode, 222, 2, 10, // Opcode: C_LUI_HINT
/* 300 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 314
/* 305 */     MCD::OPC_CheckPredicate, 0, 4, 0, 0, // Skip to: 314
/* 310 */     MCD::OPC_Decode, 192, 2, 14, // Opcode: C_ADDI16SP
/* 314 */     MCD::OPC_CheckPredicate, 0, 254, 1, 0, // Skip to: 829
/* 319 */     MCD::OPC_Decode, 221, 2, 15, // Opcode: C_LUI
/* 323 */     MCD::OPC_FilterValue, 2, 245, 1, 0, // Skip to: 829
/* 328 */     MCD::OPC_CheckPredicate, 3, 240, 1, 0, // Skip to: 829
/* 333 */     MCD::OPC_Decode, 218, 2, 16, // Opcode: C_LDSP
/* 337 */     MCD::OPC_FilterValue, 4, 81, 1, 0, // Skip to: 679
/* 342 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 345 */     MCD::OPC_FilterValue, 1, 213, 0, 0, // Skip to: 563
/* 350 */     MCD::OPC_ExtractField, 10, 2,  // Inst{11-10} ...
/* 353 */     MCD::OPC_FilterValue, 0, 32, 0, 0, // Skip to: 390
/* 358 */     MCD::OPC_CheckPredicate, 1, 18, 0, 0, // Skip to: 381
/* 363 */     MCD::OPC_CheckField, 12, 1, 0, 11, 0, 0, // Skip to: 381
/* 370 */     MCD::OPC_CheckField, 2, 5, 0, 4, 0, 0, // Skip to: 381
/* 377 */     MCD::OPC_Decode, 238, 2, 17, // Opcode: C_SRLI64_HINT
/* 381 */     MCD::OPC_CheckPredicate, 0, 187, 1, 0, // Skip to: 829
/* 386 */     MCD::OPC_Decode, 237, 2, 18, // Opcode: C_SRLI
/* 390 */     MCD::OPC_FilterValue, 1, 32, 0, 0, // Skip to: 427
/* 395 */     MCD::OPC_CheckPredicate, 1, 18, 0, 0, // Skip to: 418
/* 400 */     MCD::OPC_CheckField, 12, 1, 0, 11, 0, 0, // Skip to: 418
/* 407 */     MCD::OPC_CheckField, 2, 5, 0, 4, 0, 0, // Skip to: 418
/* 414 */     MCD::OPC_Decode, 236, 2, 17, // Opcode: C_SRAI64_HINT
/* 418 */     MCD::OPC_CheckPredicate, 0, 150, 1, 0, // Skip to: 829
/* 423 */     MCD::OPC_Decode, 235, 2, 18, // Opcode: C_SRAI
/* 427 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 441
/* 432 */     MCD::OPC_CheckPredicate, 0, 136, 1, 0, // Skip to: 829
/* 437 */     MCD::OPC_Decode, 201, 2, 19, // Opcode: C_ANDI
/* 441 */     MCD::OPC_FilterValue, 3, 127, 1, 0, // Skip to: 829
/* 446 */     MCD::OPC_ExtractField, 5, 2,  // Inst{6-5} ...
/* 449 */     MCD::OPC_FilterValue, 0, 31, 0, 0, // Skip to: 485
/* 454 */     MCD::OPC_ExtractField, 12, 1,  // Inst{12} ...
/* 457 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 471
/* 462 */     MCD::OPC_CheckPredicate, 0, 106, 1, 0, // Skip to: 829
/* 467 */     MCD::OPC_Decode, 239, 2, 20, // Opcode: C_SUB
/* 471 */     MCD::OPC_FilterValue, 1, 97, 1, 0, // Skip to: 829
/* 476 */     MCD::OPC_CheckPredicate, 3, 92, 1, 0, // Skip to: 829
/* 481 */     MCD::OPC_Decode, 240, 2, 20, // Opcode: C_SUBW
/* 485 */     MCD::OPC_FilterValue, 1, 31, 0, 0, // Skip to: 521
/* 490 */     MCD::OPC_ExtractField, 12, 1,  // Inst{12} ...
/* 493 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 507
/* 498 */     MCD::OPC_CheckPredicate, 0, 70, 1, 0, // Skip to: 829
/* 503 */     MCD::OPC_Decode, 244, 2, 20, // Opcode: C_XOR
/* 507 */     MCD::OPC_FilterValue, 1, 61, 1, 0, // Skip to: 829
/* 512 */     MCD::OPC_CheckPredicate, 3, 56, 1, 0, // Skip to: 829
/* 517 */     MCD::OPC_Decode, 198, 2, 20, // Opcode: C_ADDW
/* 521 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 542
/* 526 */     MCD::OPC_CheckPredicate, 0, 42, 1, 0, // Skip to: 829
/* 531 */     MCD::OPC_CheckField, 12, 1, 0, 35, 1, 0, // Skip to: 829
/* 538 */     MCD::OPC_Decode, 229, 2, 20, // Opcode: C_OR
/* 542 */     MCD::OPC_FilterValue, 3, 26, 1, 0, // Skip to: 829
/* 547 */     MCD::OPC_CheckPredicate, 0, 21, 1, 0, // Skip to: 829
/* 552 */     MCD::OPC_CheckField, 12, 1, 0, 14, 1, 0, // Skip to: 829
/* 559 */     MCD::OPC_Decode, 200, 2, 20, // Opcode: C_AND
/* 563 */     MCD::OPC_FilterValue, 2, 5, 1, 0, // Skip to: 829
/* 568 */     MCD::OPC_ExtractField, 12, 1,  // Inst{12} ...
/* 571 */     MCD::OPC_FilterValue, 0, 41, 0, 0, // Skip to: 617
/* 576 */     MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 592
/* 581 */     MCD::OPC_CheckField, 2, 5, 0, 4, 0, 0, // Skip to: 592
/* 588 */     MCD::OPC_Decode, 216, 2, 21, // Opcode: C_JR
/* 592 */     MCD::OPC_CheckPredicate, 1, 11, 0, 0, // Skip to: 608
/* 597 */     MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 608
/* 604 */     MCD::OPC_Decode, 226, 2, 22, // Opcode: C_MV_HINT
/* 608 */     MCD::OPC_CheckPredicate, 0, 216, 0, 0, // Skip to: 829
/* 613 */     MCD::OPC_Decode, 225, 2, 23, // Opcode: C_MV
/* 617 */     MCD::OPC_FilterValue, 1, 207, 0, 0, // Skip to: 829
/* 622 */     MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 638
/* 627 */     MCD::OPC_CheckField, 2, 10, 0, 4, 0, 0, // Skip to: 638
/* 634 */     MCD::OPC_Decode, 204, 2, 0, // Opcode: C_EBREAK
/* 638 */     MCD::OPC_CheckPredicate, 1, 11, 0, 0, // Skip to: 654
/* 643 */     MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 654
/* 650 */     MCD::OPC_Decode, 199, 2, 24, // Opcode: C_ADD_HINT
/* 654 */     MCD::OPC_CheckPredicate, 0, 11, 0, 0, // Skip to: 670
/* 659 */     MCD::OPC_CheckField, 2, 5, 0, 4, 0, 0, // Skip to: 670
/* 666 */     MCD::OPC_Decode, 215, 2, 21, // Opcode: C_JALR
/* 670 */     MCD::OPC_CheckPredicate, 0, 154, 0, 0, // Skip to: 829
/* 675 */     MCD::OPC_Decode, 190, 2, 25, // Opcode: C_ADD
/* 679 */     MCD::OPC_FilterValue, 5, 45, 0, 0, // Skip to: 729
/* 684 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 687 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 701
/* 692 */     MCD::OPC_CheckPredicate, 2, 132, 0, 0, // Skip to: 829
/* 697 */     MCD::OPC_Decode, 209, 2, 7, // Opcode: C_FSD
/* 701 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 715
/* 706 */     MCD::OPC_CheckPredicate, 0, 118, 0, 0, // Skip to: 829
/* 711 */     MCD::OPC_Decode, 213, 2, 26, // Opcode: C_J
/* 715 */     MCD::OPC_FilterValue, 2, 109, 0, 0, // Skip to: 829
/* 720 */     MCD::OPC_CheckPredicate, 2, 104, 0, 0, // Skip to: 829
/* 725 */     MCD::OPC_Decode, 210, 2, 27, // Opcode: C_FSDSP
/* 729 */     MCD::OPC_FilterValue, 6, 45, 0, 0, // Skip to: 779
/* 734 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 737 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 751
/* 742 */     MCD::OPC_CheckPredicate, 0, 82, 0, 0, // Skip to: 829
/* 747 */     MCD::OPC_Decode, 241, 2, 9, // Opcode: C_SW
/* 751 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 765
/* 756 */     MCD::OPC_CheckPredicate, 0, 68, 0, 0, // Skip to: 829
/* 761 */     MCD::OPC_Decode, 202, 2, 28, // Opcode: C_BEQZ
/* 765 */     MCD::OPC_FilterValue, 2, 59, 0, 0, // Skip to: 829
/* 770 */     MCD::OPC_CheckPredicate, 0, 54, 0, 0, // Skip to: 829
/* 775 */     MCD::OPC_Decode, 242, 2, 29, // Opcode: C_SWSP
/* 779 */     MCD::OPC_FilterValue, 7, 45, 0, 0, // Skip to: 829
/* 784 */     MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 787 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 801
/* 792 */     MCD::OPC_CheckPredicate, 3, 32, 0, 0, // Skip to: 829
/* 797 */     MCD::OPC_Decode, 230, 2, 13, // Opcode: C_SD
/* 801 */     MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 815
/* 806 */     MCD::OPC_CheckPredicate, 0, 18, 0, 0, // Skip to: 829
/* 811 */     MCD::OPC_Decode, 203, 2, 28, // Opcode: C_BNEZ
/* 815 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 829
/* 820 */     MCD::OPC_CheckPredicate, 3, 4, 0, 0, // Skip to: 829
/* 825 */     MCD::OPC_Decode, 231, 2, 30, // Opcode: C_SDSP
/* 829 */     MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTable32[] = {
/* 0 */       MCD::OPC_ExtractField, 0, 7,  // Inst{6-0} ...
/* 3 */       MCD::OPC_FilterValue, 3, 76, 0, 0, // Skip to: 84
/* 8 */       MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 11 */      MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 20
/* 16 */      MCD::OPC_Decode, 190, 3, 31, // Opcode: LB
/* 20 */      MCD::OPC_FilterValue, 1, 4, 0, 0, // Skip to: 29
/* 25 */      MCD::OPC_Decode, 193, 3, 31, // Opcode: LH
/* 29 */      MCD::OPC_FilterValue, 2, 4, 0, 0, // Skip to: 38
/* 34 */      MCD::OPC_Decode, 204, 3, 31, // Opcode: LW
/* 38 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 52
/* 43 */      MCD::OPC_CheckPredicate, 4, 55, 15, 0, // Skip to: 3943
/* 48 */      MCD::OPC_Decode, 192, 3, 31, // Opcode: LD
/* 52 */      MCD::OPC_FilterValue, 4, 4, 0, 0, // Skip to: 61
/* 57 */      MCD::OPC_Decode, 191, 3, 31, // Opcode: LBU
/* 61 */      MCD::OPC_FilterValue, 5, 4, 0, 0, // Skip to: 70
/* 66 */      MCD::OPC_Decode, 194, 3, 31, // Opcode: LHU
/* 70 */      MCD::OPC_FilterValue, 6, 28, 15, 0, // Skip to: 3943
/* 75 */      MCD::OPC_CheckPredicate, 4, 23, 15, 0, // Skip to: 3943
/* 80 */      MCD::OPC_Decode, 205, 3, 31, // Opcode: LWU
/* 84 */      MCD::OPC_FilterValue, 7, 31, 0, 0, // Skip to: 120
/* 89 */      MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 92 */      MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 106
/* 97 */      MCD::OPC_CheckPredicate, 5, 1, 15, 0, // Skip to: 3943
/* 102 */     MCD::OPC_Decode, 157, 3, 32, // Opcode: FLW
/* 106 */     MCD::OPC_FilterValue, 3, 248, 14, 0, // Skip to: 3943
/* 111 */     MCD::OPC_CheckPredicate, 6, 243, 14, 0, // Skip to: 3943
/* 116 */     MCD::OPC_Decode, 152, 3, 33, // Opcode: FLD
/* 120 */     MCD::OPC_FilterValue, 15, 52, 0, 0, // Skip to: 177
/* 125 */     MCD::OPC_ExtractField, 7, 13,  // Inst{19-7} ...
/* 128 */     MCD::OPC_FilterValue, 0, 28, 0, 0, // Skip to: 161
/* 133 */     MCD::OPC_ExtractField, 28, 4,  // Inst{31-28} ...
/* 136 */     MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 145
/* 141 */     MCD::OPC_Decode, 147, 3, 34, // Opcode: FENCE
/* 145 */     MCD::OPC_FilterValue, 8, 209, 14, 0, // Skip to: 3943
/* 150 */     MCD::OPC_CheckField, 20, 8, 51, 202, 14, 0, // Skip to: 3943
/* 157 */     MCD::OPC_Decode, 149, 3, 0, // Opcode: FENCE_TSO
/* 161 */     MCD::OPC_FilterValue, 32, 193, 14, 0, // Skip to: 3943
/* 166 */     MCD::OPC_CheckField, 20, 12, 0, 186, 14, 0, // Skip to: 3943
/* 173 */     MCD::OPC_Decode, 148, 3, 0, // Opcode: FENCE_I
/* 177 */     MCD::OPC_FilterValue, 19, 99, 0, 0, // Skip to: 281
/* 182 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 185 */     MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 194
/* 190 */     MCD::OPC_Decode, 228, 1, 31, // Opcode: ADDI
/* 194 */     MCD::OPC_FilterValue, 1, 11, 0, 0, // Skip to: 210
/* 199 */     MCD::OPC_CheckField, 26, 6, 0, 153, 14, 0, // Skip to: 3943
/* 206 */     MCD::OPC_Decode, 231, 3, 35, // Opcode: SLLI
/* 210 */     MCD::OPC_FilterValue, 2, 4, 0, 0, // Skip to: 219
/* 215 */     MCD::OPC_Decode, 235, 3, 31, // Opcode: SLTI
/* 219 */     MCD::OPC_FilterValue, 3, 4, 0, 0, // Skip to: 228
/* 224 */     MCD::OPC_Decode, 236, 3, 31, // Opcode: SLTIU
/* 228 */     MCD::OPC_FilterValue, 4, 4, 0, 0, // Skip to: 237
/* 233 */     MCD::OPC_Decode, 254, 3, 31, // Opcode: XORI
/* 237 */     MCD::OPC_FilterValue, 5, 21, 0, 0, // Skip to: 263
/* 242 */     MCD::OPC_ExtractField, 26, 6,  // Inst{31-26} ...
/* 245 */     MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 254
/* 250 */     MCD::OPC_Decode, 244, 3, 35, // Opcode: SRLI
/* 254 */     MCD::OPC_FilterValue, 16, 100, 14, 0, // Skip to: 3943
/* 259 */     MCD::OPC_Decode, 239, 3, 35, // Opcode: SRAI
/* 263 */     MCD::OPC_FilterValue, 6, 4, 0, 0, // Skip to: 272
/* 268 */     MCD::OPC_Decode, 213, 3, 31, // Opcode: ORI
/* 272 */     MCD::OPC_FilterValue, 7, 82, 14, 0, // Skip to: 3943
/* 277 */     MCD::OPC_Decode, 176, 2, 31, // Opcode: ANDI
/* 281 */     MCD::OPC_FilterValue, 23, 4, 0, 0, // Skip to: 290
/* 286 */     MCD::OPC_Decode, 177, 2, 36, // Opcode: AUIPC
/* 290 */     MCD::OPC_FilterValue, 27, 74, 0, 0, // Skip to: 369
/* 295 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 298 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 312
/* 303 */     MCD::OPC_CheckPredicate, 4, 51, 14, 0, // Skip to: 3943
/* 308 */     MCD::OPC_Decode, 229, 1, 31, // Opcode: ADDIW
/* 312 */     MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 333
/* 317 */     MCD::OPC_CheckPredicate, 4, 37, 14, 0, // Skip to: 3943
/* 322 */     MCD::OPC_CheckField, 25, 7, 0, 30, 14, 0, // Skip to: 3943
/* 329 */     MCD::OPC_Decode, 232, 3, 37, // Opcode: SLLIW
/* 333 */     MCD::OPC_FilterValue, 5, 21, 14, 0, // Skip to: 3943
/* 338 */     MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 341 */     MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 355
/* 346 */     MCD::OPC_CheckPredicate, 4, 8, 14, 0, // Skip to: 3943
/* 351 */     MCD::OPC_Decode, 245, 3, 37, // Opcode: SRLIW
/* 355 */     MCD::OPC_FilterValue, 32, 255, 13, 0, // Skip to: 3943
/* 360 */     MCD::OPC_CheckPredicate, 4, 250, 13, 0, // Skip to: 3943
/* 365 */     MCD::OPC_Decode, 240, 3, 37, // Opcode: SRAIW
/* 369 */     MCD::OPC_FilterValue, 35, 44, 0, 0, // Skip to: 418
/* 374 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 377 */     MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 386
/* 382 */     MCD::OPC_Decode, 218, 3, 38, // Opcode: SB
/* 386 */     MCD::OPC_FilterValue, 1, 4, 0, 0, // Skip to: 395
/* 391 */     MCD::OPC_Decode, 229, 3, 38, // Opcode: SH
/* 395 */     MCD::OPC_FilterValue, 2, 4, 0, 0, // Skip to: 404
/* 400 */     MCD::OPC_Decode, 249, 3, 38, // Opcode: SW
/* 404 */     MCD::OPC_FilterValue, 3, 206, 13, 0, // Skip to: 3943
/* 409 */     MCD::OPC_CheckPredicate, 4, 201, 13, 0, // Skip to: 3943
/* 414 */     MCD::OPC_Decode, 227, 3, 38, // Opcode: SD
/* 418 */     MCD::OPC_FilterValue, 39, 31, 0, 0, // Skip to: 454
/* 423 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 426 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 440
/* 431 */     MCD::OPC_CheckPredicate, 5, 179, 13, 0, // Skip to: 3943
/* 436 */     MCD::OPC_Decode, 187, 3, 39, // Opcode: FSW
/* 440 */     MCD::OPC_FilterValue, 3, 170, 13, 0, // Skip to: 3943
/* 445 */     MCD::OPC_CheckPredicate, 6, 165, 13, 0, // Skip to: 3943
/* 450 */     MCD::OPC_Decode, 176, 3, 40, // Opcode: FSD
/* 454 */     MCD::OPC_FilterValue, 47, 107, 6, 0, // Skip to: 2102
/* 459 */     MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 462 */     MCD::OPC_FilterValue, 0, 31, 0, 0, // Skip to: 498
/* 467 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 470 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 484
/* 475 */     MCD::OPC_CheckPredicate, 7, 135, 13, 0, // Skip to: 3943
/* 480 */     MCD::OPC_Decode, 235, 1, 41, // Opcode: AMOADD_W
/* 484 */     MCD::OPC_FilterValue, 3, 126, 13, 0, // Skip to: 3943
/* 489 */     MCD::OPC_CheckPredicate, 8, 121, 13, 0, // Skip to: 3943
/* 494 */     MCD::OPC_Decode, 231, 1, 41, // Opcode: AMOADD_D
/* 498 */     MCD::OPC_FilterValue, 1, 31, 0, 0, // Skip to: 534
/* 503 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 506 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 520
/* 511 */     MCD::OPC_CheckPredicate, 7, 99, 13, 0, // Skip to: 3943
/* 516 */     MCD::OPC_Decode, 238, 1, 41, // Opcode: AMOADD_W_RL
/* 520 */     MCD::OPC_FilterValue, 3, 90, 13, 0, // Skip to: 3943
/* 525 */     MCD::OPC_CheckPredicate, 8, 85, 13, 0, // Skip to: 3943
/* 530 */     MCD::OPC_Decode, 234, 1, 41, // Opcode: AMOADD_D_RL
/* 534 */     MCD::OPC_FilterValue, 2, 31, 0, 0, // Skip to: 570
/* 539 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 542 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 556
/* 547 */     MCD::OPC_CheckPredicate, 7, 63, 13, 0, // Skip to: 3943
/* 552 */     MCD::OPC_Decode, 236, 1, 41, // Opcode: AMOADD_W_AQ
/* 556 */     MCD::OPC_FilterValue, 3, 54, 13, 0, // Skip to: 3943
/* 561 */     MCD::OPC_CheckPredicate, 8, 49, 13, 0, // Skip to: 3943
/* 566 */     MCD::OPC_Decode, 232, 1, 41, // Opcode: AMOADD_D_AQ
/* 570 */     MCD::OPC_FilterValue, 3, 31, 0, 0, // Skip to: 606
/* 575 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 578 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 592
/* 583 */     MCD::OPC_CheckPredicate, 7, 27, 13, 0, // Skip to: 3943
/* 588 */     MCD::OPC_Decode, 237, 1, 41, // Opcode: AMOADD_W_AQ_RL
/* 592 */     MCD::OPC_FilterValue, 3, 18, 13, 0, // Skip to: 3943
/* 597 */     MCD::OPC_CheckPredicate, 8, 13, 13, 0, // Skip to: 3943
/* 602 */     MCD::OPC_Decode, 233, 1, 41, // Opcode: AMOADD_D_AQ_RL
/* 606 */     MCD::OPC_FilterValue, 4, 31, 0, 0, // Skip to: 642
/* 611 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 614 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 628
/* 619 */     MCD::OPC_CheckPredicate, 7, 247, 12, 0, // Skip to: 3943
/* 624 */     MCD::OPC_Decode, 163, 2, 41, // Opcode: AMOSWAP_W
/* 628 */     MCD::OPC_FilterValue, 3, 238, 12, 0, // Skip to: 3943
/* 633 */     MCD::OPC_CheckPredicate, 8, 233, 12, 0, // Skip to: 3943
/* 638 */     MCD::OPC_Decode, 159, 2, 41, // Opcode: AMOSWAP_D
/* 642 */     MCD::OPC_FilterValue, 5, 31, 0, 0, // Skip to: 678
/* 647 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 650 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 664
/* 655 */     MCD::OPC_CheckPredicate, 7, 211, 12, 0, // Skip to: 3943
/* 660 */     MCD::OPC_Decode, 166, 2, 41, // Opcode: AMOSWAP_W_RL
/* 664 */     MCD::OPC_FilterValue, 3, 202, 12, 0, // Skip to: 3943
/* 669 */     MCD::OPC_CheckPredicate, 8, 197, 12, 0, // Skip to: 3943
/* 674 */     MCD::OPC_Decode, 162, 2, 41, // Opcode: AMOSWAP_D_RL
/* 678 */     MCD::OPC_FilterValue, 6, 31, 0, 0, // Skip to: 714
/* 683 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 686 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 700
/* 691 */     MCD::OPC_CheckPredicate, 7, 175, 12, 0, // Skip to: 3943
/* 696 */     MCD::OPC_Decode, 164, 2, 41, // Opcode: AMOSWAP_W_AQ
/* 700 */     MCD::OPC_FilterValue, 3, 166, 12, 0, // Skip to: 3943
/* 705 */     MCD::OPC_CheckPredicate, 8, 161, 12, 0, // Skip to: 3943
/* 710 */     MCD::OPC_Decode, 160, 2, 41, // Opcode: AMOSWAP_D_AQ
/* 714 */     MCD::OPC_FilterValue, 7, 31, 0, 0, // Skip to: 750
/* 719 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 722 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 736
/* 727 */     MCD::OPC_CheckPredicate, 7, 139, 12, 0, // Skip to: 3943
/* 732 */     MCD::OPC_Decode, 165, 2, 41, // Opcode: AMOSWAP_W_AQ_RL
/* 736 */     MCD::OPC_FilterValue, 3, 130, 12, 0, // Skip to: 3943
/* 741 */     MCD::OPC_CheckPredicate, 8, 125, 12, 0, // Skip to: 3943
/* 746 */     MCD::OPC_Decode, 161, 2, 41, // Opcode: AMOSWAP_D_AQ_RL
/* 750 */     MCD::OPC_FilterValue, 8, 45, 0, 0, // Skip to: 800
/* 755 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 758 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 779
/* 763 */     MCD::OPC_CheckPredicate, 7, 103, 12, 0, // Skip to: 3943
/* 768 */     MCD::OPC_CheckField, 20, 5, 0, 96, 12, 0, // Skip to: 3943
/* 775 */     MCD::OPC_Decode, 199, 3, 42, // Opcode: LR_W
/* 779 */     MCD::OPC_FilterValue, 3, 87, 12, 0, // Skip to: 3943
/* 784 */     MCD::OPC_CheckPredicate, 8, 82, 12, 0, // Skip to: 3943
/* 789 */     MCD::OPC_CheckField, 20, 5, 0, 75, 12, 0, // Skip to: 3943
/* 796 */     MCD::OPC_Decode, 195, 3, 42, // Opcode: LR_D
/* 800 */     MCD::OPC_FilterValue, 9, 45, 0, 0, // Skip to: 850
/* 805 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 808 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 829
/* 813 */     MCD::OPC_CheckPredicate, 7, 53, 12, 0, // Skip to: 3943
/* 818 */     MCD::OPC_CheckField, 20, 5, 0, 46, 12, 0, // Skip to: 3943
/* 825 */     MCD::OPC_Decode, 202, 3, 42, // Opcode: LR_W_RL
/* 829 */     MCD::OPC_FilterValue, 3, 37, 12, 0, // Skip to: 3943
/* 834 */     MCD::OPC_CheckPredicate, 8, 32, 12, 0, // Skip to: 3943
/* 839 */     MCD::OPC_CheckField, 20, 5, 0, 25, 12, 0, // Skip to: 3943
/* 846 */     MCD::OPC_Decode, 198, 3, 42, // Opcode: LR_D_RL
/* 850 */     MCD::OPC_FilterValue, 10, 45, 0, 0, // Skip to: 900
/* 855 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 858 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 879
/* 863 */     MCD::OPC_CheckPredicate, 7, 3, 12, 0, // Skip to: 3943
/* 868 */     MCD::OPC_CheckField, 20, 5, 0, 252, 11, 0, // Skip to: 3943
/* 875 */     MCD::OPC_Decode, 200, 3, 42, // Opcode: LR_W_AQ
/* 879 */     MCD::OPC_FilterValue, 3, 243, 11, 0, // Skip to: 3943
/* 884 */     MCD::OPC_CheckPredicate, 8, 238, 11, 0, // Skip to: 3943
/* 889 */     MCD::OPC_CheckField, 20, 5, 0, 231, 11, 0, // Skip to: 3943
/* 896 */     MCD::OPC_Decode, 196, 3, 42, // Opcode: LR_D_AQ
/* 900 */     MCD::OPC_FilterValue, 11, 45, 0, 0, // Skip to: 950
/* 905 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 908 */     MCD::OPC_FilterValue, 2, 16, 0, 0, // Skip to: 929
/* 913 */     MCD::OPC_CheckPredicate, 7, 209, 11, 0, // Skip to: 3943
/* 918 */     MCD::OPC_CheckField, 20, 5, 0, 202, 11, 0, // Skip to: 3943
/* 925 */     MCD::OPC_Decode, 201, 3, 42, // Opcode: LR_W_AQ_RL
/* 929 */     MCD::OPC_FilterValue, 3, 193, 11, 0, // Skip to: 3943
/* 934 */     MCD::OPC_CheckPredicate, 8, 188, 11, 0, // Skip to: 3943
/* 939 */     MCD::OPC_CheckField, 20, 5, 0, 181, 11, 0, // Skip to: 3943
/* 946 */     MCD::OPC_Decode, 197, 3, 42, // Opcode: LR_D_AQ_RL
/* 950 */     MCD::OPC_FilterValue, 12, 31, 0, 0, // Skip to: 986
/* 955 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 958 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 972
/* 963 */     MCD::OPC_CheckPredicate, 7, 159, 11, 0, // Skip to: 3943
/* 968 */     MCD::OPC_Decode, 223, 3, 41, // Opcode: SC_W
/* 972 */     MCD::OPC_FilterValue, 3, 150, 11, 0, // Skip to: 3943
/* 977 */     MCD::OPC_CheckPredicate, 8, 145, 11, 0, // Skip to: 3943
/* 982 */     MCD::OPC_Decode, 219, 3, 41, // Opcode: SC_D
/* 986 */     MCD::OPC_FilterValue, 13, 31, 0, 0, // Skip to: 1022
/* 991 */     MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 994 */     MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1008
/* 999 */     MCD::OPC_CheckPredicate, 7, 123, 11, 0, // Skip to: 3943
/* 1004 */    MCD::OPC_Decode, 226, 3, 41, // Opcode: SC_W_RL
/* 1008 */    MCD::OPC_FilterValue, 3, 114, 11, 0, // Skip to: 3943
/* 1013 */    MCD::OPC_CheckPredicate, 8, 109, 11, 0, // Skip to: 3943
/* 1018 */    MCD::OPC_Decode, 222, 3, 41, // Opcode: SC_D_RL
/* 1022 */    MCD::OPC_FilterValue, 14, 31, 0, 0, // Skip to: 1058
/* 1027 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1030 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1044
/* 1035 */    MCD::OPC_CheckPredicate, 7, 87, 11, 0, // Skip to: 3943
/* 1040 */    MCD::OPC_Decode, 224, 3, 41, // Opcode: SC_W_AQ
/* 1044 */    MCD::OPC_FilterValue, 3, 78, 11, 0, // Skip to: 3943
/* 1049 */    MCD::OPC_CheckPredicate, 8, 73, 11, 0, // Skip to: 3943
/* 1054 */    MCD::OPC_Decode, 220, 3, 41, // Opcode: SC_D_AQ
/* 1058 */    MCD::OPC_FilterValue, 15, 31, 0, 0, // Skip to: 1094
/* 1063 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1066 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1080
/* 1071 */    MCD::OPC_CheckPredicate, 7, 51, 11, 0, // Skip to: 3943
/* 1076 */    MCD::OPC_Decode, 225, 3, 41, // Opcode: SC_W_AQ_RL
/* 1080 */    MCD::OPC_FilterValue, 3, 42, 11, 0, // Skip to: 3943
/* 1085 */    MCD::OPC_CheckPredicate, 8, 37, 11, 0, // Skip to: 3943
/* 1090 */    MCD::OPC_Decode, 221, 3, 41, // Opcode: SC_D_AQ_RL
/* 1094 */    MCD::OPC_FilterValue, 16, 31, 0, 0, // Skip to: 1130
/* 1099 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1102 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1116
/* 1107 */    MCD::OPC_CheckPredicate, 7, 15, 11, 0, // Skip to: 3943
/* 1112 */    MCD::OPC_Decode, 171, 2, 41, // Opcode: AMOXOR_W
/* 1116 */    MCD::OPC_FilterValue, 3, 6, 11, 0, // Skip to: 3943
/* 1121 */    MCD::OPC_CheckPredicate, 8, 1, 11, 0, // Skip to: 3943
/* 1126 */    MCD::OPC_Decode, 167, 2, 41, // Opcode: AMOXOR_D
/* 1130 */    MCD::OPC_FilterValue, 17, 31, 0, 0, // Skip to: 1166
/* 1135 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1138 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1152
/* 1143 */    MCD::OPC_CheckPredicate, 7, 235, 10, 0, // Skip to: 3943
/* 1148 */    MCD::OPC_Decode, 174, 2, 41, // Opcode: AMOXOR_W_RL
/* 1152 */    MCD::OPC_FilterValue, 3, 226, 10, 0, // Skip to: 3943
/* 1157 */    MCD::OPC_CheckPredicate, 8, 221, 10, 0, // Skip to: 3943
/* 1162 */    MCD::OPC_Decode, 170, 2, 41, // Opcode: AMOXOR_D_RL
/* 1166 */    MCD::OPC_FilterValue, 18, 31, 0, 0, // Skip to: 1202
/* 1171 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1174 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1188
/* 1179 */    MCD::OPC_CheckPredicate, 7, 199, 10, 0, // Skip to: 3943
/* 1184 */    MCD::OPC_Decode, 172, 2, 41, // Opcode: AMOXOR_W_AQ
/* 1188 */    MCD::OPC_FilterValue, 3, 190, 10, 0, // Skip to: 3943
/* 1193 */    MCD::OPC_CheckPredicate, 8, 185, 10, 0, // Skip to: 3943
/* 1198 */    MCD::OPC_Decode, 168, 2, 41, // Opcode: AMOXOR_D_AQ
/* 1202 */    MCD::OPC_FilterValue, 19, 31, 0, 0, // Skip to: 1238
/* 1207 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1210 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1224
/* 1215 */    MCD::OPC_CheckPredicate, 7, 163, 10, 0, // Skip to: 3943
/* 1220 */    MCD::OPC_Decode, 173, 2, 41, // Opcode: AMOXOR_W_AQ_RL
/* 1224 */    MCD::OPC_FilterValue, 3, 154, 10, 0, // Skip to: 3943
/* 1229 */    MCD::OPC_CheckPredicate, 8, 149, 10, 0, // Skip to: 3943
/* 1234 */    MCD::OPC_Decode, 169, 2, 41, // Opcode: AMOXOR_D_AQ_RL
/* 1238 */    MCD::OPC_FilterValue, 32, 31, 0, 0, // Skip to: 1274
/* 1243 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1246 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1260
/* 1251 */    MCD::OPC_CheckPredicate, 7, 127, 10, 0, // Skip to: 3943
/* 1256 */    MCD::OPC_Decode, 155, 2, 41, // Opcode: AMOOR_W
/* 1260 */    MCD::OPC_FilterValue, 3, 118, 10, 0, // Skip to: 3943
/* 1265 */    MCD::OPC_CheckPredicate, 8, 113, 10, 0, // Skip to: 3943
/* 1270 */    MCD::OPC_Decode, 151, 2, 41, // Opcode: AMOOR_D
/* 1274 */    MCD::OPC_FilterValue, 33, 31, 0, 0, // Skip to: 1310
/* 1279 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1282 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1296
/* 1287 */    MCD::OPC_CheckPredicate, 7, 91, 10, 0, // Skip to: 3943
/* 1292 */    MCD::OPC_Decode, 158, 2, 41, // Opcode: AMOOR_W_RL
/* 1296 */    MCD::OPC_FilterValue, 3, 82, 10, 0, // Skip to: 3943
/* 1301 */    MCD::OPC_CheckPredicate, 8, 77, 10, 0, // Skip to: 3943
/* 1306 */    MCD::OPC_Decode, 154, 2, 41, // Opcode: AMOOR_D_RL
/* 1310 */    MCD::OPC_FilterValue, 34, 31, 0, 0, // Skip to: 1346
/* 1315 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1318 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1332
/* 1323 */    MCD::OPC_CheckPredicate, 7, 55, 10, 0, // Skip to: 3943
/* 1328 */    MCD::OPC_Decode, 156, 2, 41, // Opcode: AMOOR_W_AQ
/* 1332 */    MCD::OPC_FilterValue, 3, 46, 10, 0, // Skip to: 3943
/* 1337 */    MCD::OPC_CheckPredicate, 8, 41, 10, 0, // Skip to: 3943
/* 1342 */    MCD::OPC_Decode, 152, 2, 41, // Opcode: AMOOR_D_AQ
/* 1346 */    MCD::OPC_FilterValue, 35, 31, 0, 0, // Skip to: 1382
/* 1351 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1354 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1368
/* 1359 */    MCD::OPC_CheckPredicate, 7, 19, 10, 0, // Skip to: 3943
/* 1364 */    MCD::OPC_Decode, 157, 2, 41, // Opcode: AMOOR_W_AQ_RL
/* 1368 */    MCD::OPC_FilterValue, 3, 10, 10, 0, // Skip to: 3943
/* 1373 */    MCD::OPC_CheckPredicate, 8, 5, 10, 0, // Skip to: 3943
/* 1378 */    MCD::OPC_Decode, 153, 2, 41, // Opcode: AMOOR_D_AQ_RL
/* 1382 */    MCD::OPC_FilterValue, 48, 31, 0, 0, // Skip to: 1418
/* 1387 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1390 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1404
/* 1395 */    MCD::OPC_CheckPredicate, 7, 239, 9, 0, // Skip to: 3943
/* 1400 */    MCD::OPC_Decode, 243, 1, 41, // Opcode: AMOAND_W
/* 1404 */    MCD::OPC_FilterValue, 3, 230, 9, 0, // Skip to: 3943
/* 1409 */    MCD::OPC_CheckPredicate, 8, 225, 9, 0, // Skip to: 3943
/* 1414 */    MCD::OPC_Decode, 239, 1, 41, // Opcode: AMOAND_D
/* 1418 */    MCD::OPC_FilterValue, 49, 31, 0, 0, // Skip to: 1454
/* 1423 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1426 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1440
/* 1431 */    MCD::OPC_CheckPredicate, 7, 203, 9, 0, // Skip to: 3943
/* 1436 */    MCD::OPC_Decode, 246, 1, 41, // Opcode: AMOAND_W_RL
/* 1440 */    MCD::OPC_FilterValue, 3, 194, 9, 0, // Skip to: 3943
/* 1445 */    MCD::OPC_CheckPredicate, 8, 189, 9, 0, // Skip to: 3943
/* 1450 */    MCD::OPC_Decode, 242, 1, 41, // Opcode: AMOAND_D_RL
/* 1454 */    MCD::OPC_FilterValue, 50, 31, 0, 0, // Skip to: 1490
/* 1459 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1462 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1476
/* 1467 */    MCD::OPC_CheckPredicate, 7, 167, 9, 0, // Skip to: 3943
/* 1472 */    MCD::OPC_Decode, 244, 1, 41, // Opcode: AMOAND_W_AQ
/* 1476 */    MCD::OPC_FilterValue, 3, 158, 9, 0, // Skip to: 3943
/* 1481 */    MCD::OPC_CheckPredicate, 8, 153, 9, 0, // Skip to: 3943
/* 1486 */    MCD::OPC_Decode, 240, 1, 41, // Opcode: AMOAND_D_AQ
/* 1490 */    MCD::OPC_FilterValue, 51, 31, 0, 0, // Skip to: 1526
/* 1495 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1498 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1512
/* 1503 */    MCD::OPC_CheckPredicate, 7, 131, 9, 0, // Skip to: 3943
/* 1508 */    MCD::OPC_Decode, 245, 1, 41, // Opcode: AMOAND_W_AQ_RL
/* 1512 */    MCD::OPC_FilterValue, 3, 122, 9, 0, // Skip to: 3943
/* 1517 */    MCD::OPC_CheckPredicate, 8, 117, 9, 0, // Skip to: 3943
/* 1522 */    MCD::OPC_Decode, 241, 1, 41, // Opcode: AMOAND_D_AQ_RL
/* 1526 */    MCD::OPC_FilterValue, 64, 31, 0, 0, // Skip to: 1562
/* 1531 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1534 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1548
/* 1539 */    MCD::OPC_CheckPredicate, 7, 95, 9, 0, // Skip to: 3943
/* 1544 */    MCD::OPC_Decode, 147, 2, 41, // Opcode: AMOMIN_W
/* 1548 */    MCD::OPC_FilterValue, 3, 86, 9, 0, // Skip to: 3943
/* 1553 */    MCD::OPC_CheckPredicate, 8, 81, 9, 0, // Skip to: 3943
/* 1558 */    MCD::OPC_Decode, 143, 2, 41, // Opcode: AMOMIN_D
/* 1562 */    MCD::OPC_FilterValue, 65, 31, 0, 0, // Skip to: 1598
/* 1567 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1570 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1584
/* 1575 */    MCD::OPC_CheckPredicate, 7, 59, 9, 0, // Skip to: 3943
/* 1580 */    MCD::OPC_Decode, 150, 2, 41, // Opcode: AMOMIN_W_RL
/* 1584 */    MCD::OPC_FilterValue, 3, 50, 9, 0, // Skip to: 3943
/* 1589 */    MCD::OPC_CheckPredicate, 8, 45, 9, 0, // Skip to: 3943
/* 1594 */    MCD::OPC_Decode, 146, 2, 41, // Opcode: AMOMIN_D_RL
/* 1598 */    MCD::OPC_FilterValue, 66, 31, 0, 0, // Skip to: 1634
/* 1603 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1606 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1620
/* 1611 */    MCD::OPC_CheckPredicate, 7, 23, 9, 0, // Skip to: 3943
/* 1616 */    MCD::OPC_Decode, 148, 2, 41, // Opcode: AMOMIN_W_AQ
/* 1620 */    MCD::OPC_FilterValue, 3, 14, 9, 0, // Skip to: 3943
/* 1625 */    MCD::OPC_CheckPredicate, 8, 9, 9, 0, // Skip to: 3943
/* 1630 */    MCD::OPC_Decode, 144, 2, 41, // Opcode: AMOMIN_D_AQ
/* 1634 */    MCD::OPC_FilterValue, 67, 31, 0, 0, // Skip to: 1670
/* 1639 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1642 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1656
/* 1647 */    MCD::OPC_CheckPredicate, 7, 243, 8, 0, // Skip to: 3943
/* 1652 */    MCD::OPC_Decode, 149, 2, 41, // Opcode: AMOMIN_W_AQ_RL
/* 1656 */    MCD::OPC_FilterValue, 3, 234, 8, 0, // Skip to: 3943
/* 1661 */    MCD::OPC_CheckPredicate, 8, 229, 8, 0, // Skip to: 3943
/* 1666 */    MCD::OPC_Decode, 145, 2, 41, // Opcode: AMOMIN_D_AQ_RL
/* 1670 */    MCD::OPC_FilterValue, 80, 31, 0, 0, // Skip to: 1706
/* 1675 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1678 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1692
/* 1683 */    MCD::OPC_CheckPredicate, 7, 207, 8, 0, // Skip to: 3943
/* 1688 */    MCD::OPC_Decode, 131, 2, 41, // Opcode: AMOMAX_W
/* 1692 */    MCD::OPC_FilterValue, 3, 198, 8, 0, // Skip to: 3943
/* 1697 */    MCD::OPC_CheckPredicate, 8, 193, 8, 0, // Skip to: 3943
/* 1702 */    MCD::OPC_Decode, 255, 1, 41, // Opcode: AMOMAX_D
/* 1706 */    MCD::OPC_FilterValue, 81, 31, 0, 0, // Skip to: 1742
/* 1711 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1714 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1728
/* 1719 */    MCD::OPC_CheckPredicate, 7, 171, 8, 0, // Skip to: 3943
/* 1724 */    MCD::OPC_Decode, 134, 2, 41, // Opcode: AMOMAX_W_RL
/* 1728 */    MCD::OPC_FilterValue, 3, 162, 8, 0, // Skip to: 3943
/* 1733 */    MCD::OPC_CheckPredicate, 8, 157, 8, 0, // Skip to: 3943
/* 1738 */    MCD::OPC_Decode, 130, 2, 41, // Opcode: AMOMAX_D_RL
/* 1742 */    MCD::OPC_FilterValue, 82, 31, 0, 0, // Skip to: 1778
/* 1747 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1750 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1764
/* 1755 */    MCD::OPC_CheckPredicate, 7, 135, 8, 0, // Skip to: 3943
/* 1760 */    MCD::OPC_Decode, 132, 2, 41, // Opcode: AMOMAX_W_AQ
/* 1764 */    MCD::OPC_FilterValue, 3, 126, 8, 0, // Skip to: 3943
/* 1769 */    MCD::OPC_CheckPredicate, 8, 121, 8, 0, // Skip to: 3943
/* 1774 */    MCD::OPC_Decode, 128, 2, 41, // Opcode: AMOMAX_D_AQ
/* 1778 */    MCD::OPC_FilterValue, 83, 31, 0, 0, // Skip to: 1814
/* 1783 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1786 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1800
/* 1791 */    MCD::OPC_CheckPredicate, 7, 99, 8, 0, // Skip to: 3943
/* 1796 */    MCD::OPC_Decode, 133, 2, 41, // Opcode: AMOMAX_W_AQ_RL
/* 1800 */    MCD::OPC_FilterValue, 3, 90, 8, 0, // Skip to: 3943
/* 1805 */    MCD::OPC_CheckPredicate, 8, 85, 8, 0, // Skip to: 3943
/* 1810 */    MCD::OPC_Decode, 129, 2, 41, // Opcode: AMOMAX_D_AQ_RL
/* 1814 */    MCD::OPC_FilterValue, 96, 31, 0, 0, // Skip to: 1850
/* 1819 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1822 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1836
/* 1827 */    MCD::OPC_CheckPredicate, 7, 63, 8, 0, // Skip to: 3943
/* 1832 */    MCD::OPC_Decode, 139, 2, 41, // Opcode: AMOMINU_W
/* 1836 */    MCD::OPC_FilterValue, 3, 54, 8, 0, // Skip to: 3943
/* 1841 */    MCD::OPC_CheckPredicate, 8, 49, 8, 0, // Skip to: 3943
/* 1846 */    MCD::OPC_Decode, 135, 2, 41, // Opcode: AMOMINU_D
/* 1850 */    MCD::OPC_FilterValue, 97, 31, 0, 0, // Skip to: 1886
/* 1855 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1858 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1872
/* 1863 */    MCD::OPC_CheckPredicate, 7, 27, 8, 0, // Skip to: 3943
/* 1868 */    MCD::OPC_Decode, 142, 2, 41, // Opcode: AMOMINU_W_RL
/* 1872 */    MCD::OPC_FilterValue, 3, 18, 8, 0, // Skip to: 3943
/* 1877 */    MCD::OPC_CheckPredicate, 8, 13, 8, 0, // Skip to: 3943
/* 1882 */    MCD::OPC_Decode, 138, 2, 41, // Opcode: AMOMINU_D_RL
/* 1886 */    MCD::OPC_FilterValue, 98, 31, 0, 0, // Skip to: 1922
/* 1891 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1894 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1908
/* 1899 */    MCD::OPC_CheckPredicate, 7, 247, 7, 0, // Skip to: 3943
/* 1904 */    MCD::OPC_Decode, 140, 2, 41, // Opcode: AMOMINU_W_AQ
/* 1908 */    MCD::OPC_FilterValue, 3, 238, 7, 0, // Skip to: 3943
/* 1913 */    MCD::OPC_CheckPredicate, 8, 233, 7, 0, // Skip to: 3943
/* 1918 */    MCD::OPC_Decode, 136, 2, 41, // Opcode: AMOMINU_D_AQ
/* 1922 */    MCD::OPC_FilterValue, 99, 31, 0, 0, // Skip to: 1958
/* 1927 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1930 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1944
/* 1935 */    MCD::OPC_CheckPredicate, 7, 211, 7, 0, // Skip to: 3943
/* 1940 */    MCD::OPC_Decode, 141, 2, 41, // Opcode: AMOMINU_W_AQ_RL
/* 1944 */    MCD::OPC_FilterValue, 3, 202, 7, 0, // Skip to: 3943
/* 1949 */    MCD::OPC_CheckPredicate, 8, 197, 7, 0, // Skip to: 3943
/* 1954 */    MCD::OPC_Decode, 137, 2, 41, // Opcode: AMOMINU_D_AQ_RL
/* 1958 */    MCD::OPC_FilterValue, 112, 31, 0, 0, // Skip to: 1994
/* 1963 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 1966 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 1980
/* 1971 */    MCD::OPC_CheckPredicate, 7, 175, 7, 0, // Skip to: 3943
/* 1976 */    MCD::OPC_Decode, 251, 1, 41, // Opcode: AMOMAXU_W
/* 1980 */    MCD::OPC_FilterValue, 3, 166, 7, 0, // Skip to: 3943
/* 1985 */    MCD::OPC_CheckPredicate, 8, 161, 7, 0, // Skip to: 3943
/* 1990 */    MCD::OPC_Decode, 247, 1, 41, // Opcode: AMOMAXU_D
/* 1994 */    MCD::OPC_FilterValue, 113, 31, 0, 0, // Skip to: 2030
/* 1999 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2002 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2016
/* 2007 */    MCD::OPC_CheckPredicate, 7, 139, 7, 0, // Skip to: 3943
/* 2012 */    MCD::OPC_Decode, 254, 1, 41, // Opcode: AMOMAXU_W_RL
/* 2016 */    MCD::OPC_FilterValue, 3, 130, 7, 0, // Skip to: 3943
/* 2021 */    MCD::OPC_CheckPredicate, 8, 125, 7, 0, // Skip to: 3943
/* 2026 */    MCD::OPC_Decode, 250, 1, 41, // Opcode: AMOMAXU_D_RL
/* 2030 */    MCD::OPC_FilterValue, 114, 31, 0, 0, // Skip to: 2066
/* 2035 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2038 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2052
/* 2043 */    MCD::OPC_CheckPredicate, 7, 103, 7, 0, // Skip to: 3943
/* 2048 */    MCD::OPC_Decode, 252, 1, 41, // Opcode: AMOMAXU_W_AQ
/* 2052 */    MCD::OPC_FilterValue, 3, 94, 7, 0, // Skip to: 3943
/* 2057 */    MCD::OPC_CheckPredicate, 8, 89, 7, 0, // Skip to: 3943
/* 2062 */    MCD::OPC_Decode, 248, 1, 41, // Opcode: AMOMAXU_D_AQ
/* 2066 */    MCD::OPC_FilterValue, 115, 80, 7, 0, // Skip to: 3943
/* 2071 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2074 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 2088
/* 2079 */    MCD::OPC_CheckPredicate, 7, 67, 7, 0, // Skip to: 3943
/* 2084 */    MCD::OPC_Decode, 253, 1, 41, // Opcode: AMOMAXU_W_AQ_RL
/* 2088 */    MCD::OPC_FilterValue, 3, 58, 7, 0, // Skip to: 3943
/* 2093 */    MCD::OPC_CheckPredicate, 8, 53, 7, 0, // Skip to: 3943
/* 2098 */    MCD::OPC_Decode, 249, 1, 41, // Opcode: AMOMAXU_D_AQ_RL
/* 2102 */    MCD::OPC_FilterValue, 51, 13, 1, 0, // Skip to: 2376
/* 2107 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2110 */    MCD::OPC_FilterValue, 0, 35, 0, 0, // Skip to: 2150
/* 2115 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2118 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2127
/* 2123 */    MCD::OPC_Decode, 227, 1, 41, // Opcode: ADD
/* 2127 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2141
/* 2132 */    MCD::OPC_CheckPredicate, 9, 14, 7, 0, // Skip to: 3943
/* 2137 */    MCD::OPC_Decode, 207, 3, 41, // Opcode: MUL
/* 2141 */    MCD::OPC_FilterValue, 32, 5, 7, 0, // Skip to: 3943
/* 2146 */    MCD::OPC_Decode, 247, 3, 41, // Opcode: SUB
/* 2150 */    MCD::OPC_FilterValue, 1, 26, 0, 0, // Skip to: 2181
/* 2155 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2158 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2167
/* 2163 */    MCD::OPC_Decode, 230, 3, 41, // Opcode: SLL
/* 2167 */    MCD::OPC_FilterValue, 1, 235, 6, 0, // Skip to: 3943
/* 2172 */    MCD::OPC_CheckPredicate, 9, 230, 6, 0, // Skip to: 3943
/* 2177 */    MCD::OPC_Decode, 208, 3, 41, // Opcode: MULH
/* 2181 */    MCD::OPC_FilterValue, 2, 26, 0, 0, // Skip to: 2212
/* 2186 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2189 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2198
/* 2194 */    MCD::OPC_Decode, 234, 3, 41, // Opcode: SLT
/* 2198 */    MCD::OPC_FilterValue, 1, 204, 6, 0, // Skip to: 3943
/* 2203 */    MCD::OPC_CheckPredicate, 9, 199, 6, 0, // Skip to: 3943
/* 2208 */    MCD::OPC_Decode, 209, 3, 41, // Opcode: MULHSU
/* 2212 */    MCD::OPC_FilterValue, 3, 26, 0, 0, // Skip to: 2243
/* 2217 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2220 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2229
/* 2225 */    MCD::OPC_Decode, 237, 3, 41, // Opcode: SLTU
/* 2229 */    MCD::OPC_FilterValue, 1, 173, 6, 0, // Skip to: 3943
/* 2234 */    MCD::OPC_CheckPredicate, 9, 168, 6, 0, // Skip to: 3943
/* 2239 */    MCD::OPC_Decode, 210, 3, 41, // Opcode: MULHU
/* 2243 */    MCD::OPC_FilterValue, 4, 26, 0, 0, // Skip to: 2274
/* 2248 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2251 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2260
/* 2256 */    MCD::OPC_Decode, 253, 3, 41, // Opcode: XOR
/* 2260 */    MCD::OPC_FilterValue, 1, 142, 6, 0, // Skip to: 3943
/* 2265 */    MCD::OPC_CheckPredicate, 9, 137, 6, 0, // Skip to: 3943
/* 2270 */    MCD::OPC_Decode, 245, 2, 41, // Opcode: DIV
/* 2274 */    MCD::OPC_FilterValue, 5, 35, 0, 0, // Skip to: 2314
/* 2279 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2282 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2291
/* 2287 */    MCD::OPC_Decode, 243, 3, 41, // Opcode: SRL
/* 2291 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2305
/* 2296 */    MCD::OPC_CheckPredicate, 9, 106, 6, 0, // Skip to: 3943
/* 2301 */    MCD::OPC_Decode, 246, 2, 41, // Opcode: DIVU
/* 2305 */    MCD::OPC_FilterValue, 32, 97, 6, 0, // Skip to: 3943
/* 2310 */    MCD::OPC_Decode, 238, 3, 41, // Opcode: SRA
/* 2314 */    MCD::OPC_FilterValue, 6, 26, 0, 0, // Skip to: 2345
/* 2319 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2322 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2331
/* 2327 */    MCD::OPC_Decode, 212, 3, 41, // Opcode: OR
/* 2331 */    MCD::OPC_FilterValue, 1, 71, 6, 0, // Skip to: 3943
/* 2336 */    MCD::OPC_CheckPredicate, 9, 66, 6, 0, // Skip to: 3943
/* 2341 */    MCD::OPC_Decode, 214, 3, 41, // Opcode: REM
/* 2345 */    MCD::OPC_FilterValue, 7, 57, 6, 0, // Skip to: 3943
/* 2350 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2353 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 2362
/* 2358 */    MCD::OPC_Decode, 175, 2, 41, // Opcode: AND
/* 2362 */    MCD::OPC_FilterValue, 1, 40, 6, 0, // Skip to: 3943
/* 2367 */    MCD::OPC_CheckPredicate, 9, 35, 6, 0, // Skip to: 3943
/* 2372 */    MCD::OPC_Decode, 215, 3, 41, // Opcode: REMU
/* 2376 */    MCD::OPC_FilterValue, 55, 4, 0, 0, // Skip to: 2385
/* 2381 */    MCD::OPC_Decode, 203, 3, 36, // Opcode: LUI
/* 2385 */    MCD::OPC_FilterValue, 59, 187, 0, 0, // Skip to: 2577
/* 2390 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2393 */    MCD::OPC_FilterValue, 0, 45, 0, 0, // Skip to: 2443
/* 2398 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2401 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2415
/* 2406 */    MCD::OPC_CheckPredicate, 4, 252, 5, 0, // Skip to: 3943
/* 2411 */    MCD::OPC_Decode, 230, 1, 41, // Opcode: ADDW
/* 2415 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2429
/* 2420 */    MCD::OPC_CheckPredicate, 10, 238, 5, 0, // Skip to: 3943
/* 2425 */    MCD::OPC_Decode, 211, 3, 41, // Opcode: MULW
/* 2429 */    MCD::OPC_FilterValue, 32, 229, 5, 0, // Skip to: 3943
/* 2434 */    MCD::OPC_CheckPredicate, 4, 224, 5, 0, // Skip to: 3943
/* 2439 */    MCD::OPC_Decode, 248, 3, 41, // Opcode: SUBW
/* 2443 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 2464
/* 2448 */    MCD::OPC_CheckPredicate, 4, 210, 5, 0, // Skip to: 3943
/* 2453 */    MCD::OPC_CheckField, 25, 7, 0, 203, 5, 0, // Skip to: 3943
/* 2460 */    MCD::OPC_Decode, 233, 3, 41, // Opcode: SLLW
/* 2464 */    MCD::OPC_FilterValue, 4, 16, 0, 0, // Skip to: 2485
/* 2469 */    MCD::OPC_CheckPredicate, 10, 189, 5, 0, // Skip to: 3943
/* 2474 */    MCD::OPC_CheckField, 25, 7, 1, 182, 5, 0, // Skip to: 3943
/* 2481 */    MCD::OPC_Decode, 248, 2, 41, // Opcode: DIVW
/* 2485 */    MCD::OPC_FilterValue, 5, 45, 0, 0, // Skip to: 2535
/* 2490 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2493 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2507
/* 2498 */    MCD::OPC_CheckPredicate, 4, 160, 5, 0, // Skip to: 3943
/* 2503 */    MCD::OPC_Decode, 246, 3, 41, // Opcode: SRLW
/* 2507 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2521
/* 2512 */    MCD::OPC_CheckPredicate, 10, 146, 5, 0, // Skip to: 3943
/* 2517 */    MCD::OPC_Decode, 247, 2, 41, // Opcode: DIVUW
/* 2521 */    MCD::OPC_FilterValue, 32, 137, 5, 0, // Skip to: 3943
/* 2526 */    MCD::OPC_CheckPredicate, 4, 132, 5, 0, // Skip to: 3943
/* 2531 */    MCD::OPC_Decode, 241, 3, 41, // Opcode: SRAW
/* 2535 */    MCD::OPC_FilterValue, 6, 16, 0, 0, // Skip to: 2556
/* 2540 */    MCD::OPC_CheckPredicate, 10, 118, 5, 0, // Skip to: 3943
/* 2545 */    MCD::OPC_CheckField, 25, 7, 1, 111, 5, 0, // Skip to: 3943
/* 2552 */    MCD::OPC_Decode, 217, 3, 41, // Opcode: REMW
/* 2556 */    MCD::OPC_FilterValue, 7, 102, 5, 0, // Skip to: 3943
/* 2561 */    MCD::OPC_CheckPredicate, 10, 97, 5, 0, // Skip to: 3943
/* 2566 */    MCD::OPC_CheckField, 25, 7, 1, 90, 5, 0, // Skip to: 3943
/* 2573 */    MCD::OPC_Decode, 216, 3, 41, // Opcode: REMUW
/* 2577 */    MCD::OPC_FilterValue, 67, 31, 0, 0, // Skip to: 2613
/* 2582 */    MCD::OPC_ExtractField, 25, 2,  // Inst{26-25} ...
/* 2585 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2599
/* 2590 */    MCD::OPC_CheckPredicate, 5, 68, 5, 0, // Skip to: 3943
/* 2595 */    MCD::OPC_Decode, 159, 3, 43, // Opcode: FMADD_S
/* 2599 */    MCD::OPC_FilterValue, 1, 59, 5, 0, // Skip to: 3943
/* 2604 */    MCD::OPC_CheckPredicate, 6, 54, 5, 0, // Skip to: 3943
/* 2609 */    MCD::OPC_Decode, 158, 3, 44, // Opcode: FMADD_D
/* 2613 */    MCD::OPC_FilterValue, 71, 31, 0, 0, // Skip to: 2649
/* 2618 */    MCD::OPC_ExtractField, 25, 2,  // Inst{26-25} ...
/* 2621 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2635
/* 2626 */    MCD::OPC_CheckPredicate, 5, 32, 5, 0, // Skip to: 3943
/* 2631 */    MCD::OPC_Decode, 165, 3, 43, // Opcode: FMSUB_S
/* 2635 */    MCD::OPC_FilterValue, 1, 23, 5, 0, // Skip to: 3943
/* 2640 */    MCD::OPC_CheckPredicate, 6, 18, 5, 0, // Skip to: 3943
/* 2645 */    MCD::OPC_Decode, 164, 3, 44, // Opcode: FMSUB_D
/* 2649 */    MCD::OPC_FilterValue, 75, 31, 0, 0, // Skip to: 2685
/* 2654 */    MCD::OPC_ExtractField, 25, 2,  // Inst{26-25} ...
/* 2657 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2671
/* 2662 */    MCD::OPC_CheckPredicate, 5, 252, 4, 0, // Skip to: 3943
/* 2667 */    MCD::OPC_Decode, 175, 3, 43, // Opcode: FNMSUB_S
/* 2671 */    MCD::OPC_FilterValue, 1, 243, 4, 0, // Skip to: 3943
/* 2676 */    MCD::OPC_CheckPredicate, 6, 238, 4, 0, // Skip to: 3943
/* 2681 */    MCD::OPC_Decode, 174, 3, 44, // Opcode: FNMSUB_D
/* 2685 */    MCD::OPC_FilterValue, 79, 31, 0, 0, // Skip to: 2721
/* 2690 */    MCD::OPC_ExtractField, 25, 2,  // Inst{26-25} ...
/* 2693 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2707
/* 2698 */    MCD::OPC_CheckPredicate, 5, 216, 4, 0, // Skip to: 3943
/* 2703 */    MCD::OPC_Decode, 173, 3, 43, // Opcode: FNMADD_S
/* 2707 */    MCD::OPC_FilterValue, 1, 207, 4, 0, // Skip to: 3943
/* 2712 */    MCD::OPC_CheckPredicate, 6, 202, 4, 0, // Skip to: 3943
/* 2717 */    MCD::OPC_Decode, 172, 3, 44, // Opcode: FNMADD_D
/* 2721 */    MCD::OPC_FilterValue, 83, 136, 3, 0, // Skip to: 3630
/* 2726 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 2729 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2743
/* 2734 */    MCD::OPC_CheckPredicate, 5, 180, 4, 0, // Skip to: 3943
/* 2739 */    MCD::OPC_Decode, 252, 2, 45, // Opcode: FADD_S
/* 2743 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2757
/* 2748 */    MCD::OPC_CheckPredicate, 6, 166, 4, 0, // Skip to: 3943
/* 2753 */    MCD::OPC_Decode, 251, 2, 46, // Opcode: FADD_D
/* 2757 */    MCD::OPC_FilterValue, 4, 9, 0, 0, // Skip to: 2771
/* 2762 */    MCD::OPC_CheckPredicate, 5, 152, 4, 0, // Skip to: 3943
/* 2767 */    MCD::OPC_Decode, 186, 3, 45, // Opcode: FSUB_S
/* 2771 */    MCD::OPC_FilterValue, 5, 9, 0, 0, // Skip to: 2785
/* 2776 */    MCD::OPC_CheckPredicate, 6, 138, 4, 0, // Skip to: 3943
/* 2781 */    MCD::OPC_Decode, 185, 3, 46, // Opcode: FSUB_D
/* 2785 */    MCD::OPC_FilterValue, 8, 9, 0, 0, // Skip to: 2799
/* 2790 */    MCD::OPC_CheckPredicate, 5, 124, 4, 0, // Skip to: 3943
/* 2795 */    MCD::OPC_Decode, 167, 3, 45, // Opcode: FMUL_S
/* 2799 */    MCD::OPC_FilterValue, 9, 9, 0, 0, // Skip to: 2813
/* 2804 */    MCD::OPC_CheckPredicate, 6, 110, 4, 0, // Skip to: 3943
/* 2809 */    MCD::OPC_Decode, 166, 3, 46, // Opcode: FMUL_D
/* 2813 */    MCD::OPC_FilterValue, 12, 9, 0, 0, // Skip to: 2827
/* 2818 */    MCD::OPC_CheckPredicate, 5, 96, 4, 0, // Skip to: 3943
/* 2823 */    MCD::OPC_Decode, 146, 3, 45, // Opcode: FDIV_S
/* 2827 */    MCD::OPC_FilterValue, 13, 9, 0, 0, // Skip to: 2841
/* 2832 */    MCD::OPC_CheckPredicate, 6, 82, 4, 0, // Skip to: 3943
/* 2837 */    MCD::OPC_Decode, 145, 3, 46, // Opcode: FDIV_D
/* 2841 */    MCD::OPC_FilterValue, 16, 45, 0, 0, // Skip to: 2891
/* 2846 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2849 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2863
/* 2854 */    MCD::OPC_CheckPredicate, 5, 60, 4, 0, // Skip to: 3943
/* 2859 */    MCD::OPC_Decode, 182, 3, 47, // Opcode: FSGNJ_S
/* 2863 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2877
/* 2868 */    MCD::OPC_CheckPredicate, 5, 46, 4, 0, // Skip to: 3943
/* 2873 */    MCD::OPC_Decode, 178, 3, 47, // Opcode: FSGNJN_S
/* 2877 */    MCD::OPC_FilterValue, 2, 37, 4, 0, // Skip to: 3943
/* 2882 */    MCD::OPC_CheckPredicate, 5, 32, 4, 0, // Skip to: 3943
/* 2887 */    MCD::OPC_Decode, 180, 3, 47, // Opcode: FSGNJX_S
/* 2891 */    MCD::OPC_FilterValue, 17, 45, 0, 0, // Skip to: 2941
/* 2896 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2899 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2913
/* 2904 */    MCD::OPC_CheckPredicate, 6, 10, 4, 0, // Skip to: 3943
/* 2909 */    MCD::OPC_Decode, 181, 3, 48, // Opcode: FSGNJ_D
/* 2913 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 2927
/* 2918 */    MCD::OPC_CheckPredicate, 6, 252, 3, 0, // Skip to: 3943
/* 2923 */    MCD::OPC_Decode, 177, 3, 48, // Opcode: FSGNJN_D
/* 2927 */    MCD::OPC_FilterValue, 2, 243, 3, 0, // Skip to: 3943
/* 2932 */    MCD::OPC_CheckPredicate, 6, 238, 3, 0, // Skip to: 3943
/* 2937 */    MCD::OPC_Decode, 179, 3, 48, // Opcode: FSGNJX_D
/* 2941 */    MCD::OPC_FilterValue, 20, 31, 0, 0, // Skip to: 2977
/* 2946 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2949 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2963
/* 2954 */    MCD::OPC_CheckPredicate, 5, 216, 3, 0, // Skip to: 3943
/* 2959 */    MCD::OPC_Decode, 163, 3, 47, // Opcode: FMIN_S
/* 2963 */    MCD::OPC_FilterValue, 1, 207, 3, 0, // Skip to: 3943
/* 2968 */    MCD::OPC_CheckPredicate, 5, 202, 3, 0, // Skip to: 3943
/* 2973 */    MCD::OPC_Decode, 161, 3, 47, // Opcode: FMAX_S
/* 2977 */    MCD::OPC_FilterValue, 21, 31, 0, 0, // Skip to: 3013
/* 2982 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 2985 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 2999
/* 2990 */    MCD::OPC_CheckPredicate, 6, 180, 3, 0, // Skip to: 3943
/* 2995 */    MCD::OPC_Decode, 162, 3, 48, // Opcode: FMIN_D
/* 2999 */    MCD::OPC_FilterValue, 1, 171, 3, 0, // Skip to: 3943
/* 3004 */    MCD::OPC_CheckPredicate, 6, 166, 3, 0, // Skip to: 3943
/* 3009 */    MCD::OPC_Decode, 160, 3, 48, // Opcode: FMAX_D
/* 3013 */    MCD::OPC_FilterValue, 32, 16, 0, 0, // Skip to: 3034
/* 3018 */    MCD::OPC_CheckPredicate, 6, 152, 3, 0, // Skip to: 3943
/* 3023 */    MCD::OPC_CheckField, 20, 5, 1, 145, 3, 0, // Skip to: 3943
/* 3030 */    MCD::OPC_Decode, 136, 3, 49, // Opcode: FCVT_S_D
/* 3034 */    MCD::OPC_FilterValue, 33, 23, 0, 0, // Skip to: 3062
/* 3039 */    MCD::OPC_CheckPredicate, 6, 131, 3, 0, // Skip to: 3943
/* 3044 */    MCD::OPC_CheckField, 20, 5, 0, 124, 3, 0, // Skip to: 3943
/* 3051 */    MCD::OPC_CheckField, 12, 3, 0, 117, 3, 0, // Skip to: 3943
/* 3058 */    MCD::OPC_Decode, 129, 3, 50, // Opcode: FCVT_D_S
/* 3062 */    MCD::OPC_FilterValue, 44, 16, 0, 0, // Skip to: 3083
/* 3067 */    MCD::OPC_CheckPredicate, 5, 103, 3, 0, // Skip to: 3943
/* 3072 */    MCD::OPC_CheckField, 20, 5, 0, 96, 3, 0, // Skip to: 3943
/* 3079 */    MCD::OPC_Decode, 184, 3, 51, // Opcode: FSQRT_S
/* 3083 */    MCD::OPC_FilterValue, 45, 16, 0, 0, // Skip to: 3104
/* 3088 */    MCD::OPC_CheckPredicate, 6, 82, 3, 0, // Skip to: 3943
/* 3093 */    MCD::OPC_CheckField, 20, 5, 0, 75, 3, 0, // Skip to: 3943
/* 3100 */    MCD::OPC_Decode, 183, 3, 52, // Opcode: FSQRT_D
/* 3104 */    MCD::OPC_FilterValue, 80, 45, 0, 0, // Skip to: 3154
/* 3109 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3112 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3126
/* 3117 */    MCD::OPC_CheckPredicate, 5, 53, 3, 0, // Skip to: 3943
/* 3122 */    MCD::OPC_Decode, 154, 3, 53, // Opcode: FLE_S
/* 3126 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3140
/* 3131 */    MCD::OPC_CheckPredicate, 5, 39, 3, 0, // Skip to: 3943
/* 3136 */    MCD::OPC_Decode, 156, 3, 53, // Opcode: FLT_S
/* 3140 */    MCD::OPC_FilterValue, 2, 30, 3, 0, // Skip to: 3943
/* 3145 */    MCD::OPC_CheckPredicate, 5, 25, 3, 0, // Skip to: 3943
/* 3150 */    MCD::OPC_Decode, 151, 3, 53, // Opcode: FEQ_S
/* 3154 */    MCD::OPC_FilterValue, 81, 45, 0, 0, // Skip to: 3204
/* 3159 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3162 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3176
/* 3167 */    MCD::OPC_CheckPredicate, 6, 3, 3, 0, // Skip to: 3943
/* 3172 */    MCD::OPC_Decode, 153, 3, 54, // Opcode: FLE_D
/* 3176 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3190
/* 3181 */    MCD::OPC_CheckPredicate, 6, 245, 2, 0, // Skip to: 3943
/* 3186 */    MCD::OPC_Decode, 155, 3, 54, // Opcode: FLT_D
/* 3190 */    MCD::OPC_FilterValue, 2, 236, 2, 0, // Skip to: 3943
/* 3195 */    MCD::OPC_CheckPredicate, 6, 231, 2, 0, // Skip to: 3943
/* 3200 */    MCD::OPC_Decode, 150, 3, 54, // Opcode: FEQ_D
/* 3204 */    MCD::OPC_FilterValue, 96, 59, 0, 0, // Skip to: 3268
/* 3209 */    MCD::OPC_ExtractField, 20, 5,  // Inst{24-20} ...
/* 3212 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3226
/* 3217 */    MCD::OPC_CheckPredicate, 5, 209, 2, 0, // Skip to: 3943
/* 3222 */    MCD::OPC_Decode, 144, 3, 55, // Opcode: FCVT_W_S
/* 3226 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3240
/* 3231 */    MCD::OPC_CheckPredicate, 5, 195, 2, 0, // Skip to: 3943
/* 3236 */    MCD::OPC_Decode, 142, 3, 55, // Opcode: FCVT_WU_S
/* 3240 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3254
/* 3245 */    MCD::OPC_CheckPredicate, 11, 181, 2, 0, // Skip to: 3943
/* 3250 */    MCD::OPC_Decode, 135, 3, 55, // Opcode: FCVT_L_S
/* 3254 */    MCD::OPC_FilterValue, 3, 172, 2, 0, // Skip to: 3943
/* 3259 */    MCD::OPC_CheckPredicate, 11, 167, 2, 0, // Skip to: 3943
/* 3264 */    MCD::OPC_Decode, 133, 3, 55, // Opcode: FCVT_LU_S
/* 3268 */    MCD::OPC_FilterValue, 97, 59, 0, 0, // Skip to: 3332
/* 3273 */    MCD::OPC_ExtractField, 20, 5,  // Inst{24-20} ...
/* 3276 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3290
/* 3281 */    MCD::OPC_CheckPredicate, 6, 145, 2, 0, // Skip to: 3943
/* 3286 */    MCD::OPC_Decode, 143, 3, 56, // Opcode: FCVT_W_D
/* 3290 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3304
/* 3295 */    MCD::OPC_CheckPredicate, 6, 131, 2, 0, // Skip to: 3943
/* 3300 */    MCD::OPC_Decode, 141, 3, 56, // Opcode: FCVT_WU_D
/* 3304 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3318
/* 3309 */    MCD::OPC_CheckPredicate, 12, 117, 2, 0, // Skip to: 3943
/* 3314 */    MCD::OPC_Decode, 134, 3, 56, // Opcode: FCVT_L_D
/* 3318 */    MCD::OPC_FilterValue, 3, 108, 2, 0, // Skip to: 3943
/* 3323 */    MCD::OPC_CheckPredicate, 12, 103, 2, 0, // Skip to: 3943
/* 3328 */    MCD::OPC_Decode, 132, 3, 56, // Opcode: FCVT_LU_D
/* 3332 */    MCD::OPC_FilterValue, 104, 59, 0, 0, // Skip to: 3396
/* 3337 */    MCD::OPC_ExtractField, 20, 5,  // Inst{24-20} ...
/* 3340 */    MCD::OPC_FilterValue, 0, 9, 0, 0, // Skip to: 3354
/* 3345 */    MCD::OPC_CheckPredicate, 5, 81, 2, 0, // Skip to: 3943
/* 3350 */    MCD::OPC_Decode, 139, 3, 57, // Opcode: FCVT_S_W
/* 3354 */    MCD::OPC_FilterValue, 1, 9, 0, 0, // Skip to: 3368
/* 3359 */    MCD::OPC_CheckPredicate, 5, 67, 2, 0, // Skip to: 3943
/* 3364 */    MCD::OPC_Decode, 140, 3, 57, // Opcode: FCVT_S_WU
/* 3368 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3382
/* 3373 */    MCD::OPC_CheckPredicate, 11, 53, 2, 0, // Skip to: 3943
/* 3378 */    MCD::OPC_Decode, 137, 3, 57, // Opcode: FCVT_S_L
/* 3382 */    MCD::OPC_FilterValue, 3, 44, 2, 0, // Skip to: 3943
/* 3387 */    MCD::OPC_CheckPredicate, 11, 39, 2, 0, // Skip to: 3943
/* 3392 */    MCD::OPC_Decode, 138, 3, 57, // Opcode: FCVT_S_LU
/* 3396 */    MCD::OPC_FilterValue, 105, 73, 0, 0, // Skip to: 3474
/* 3401 */    MCD::OPC_ExtractField, 20, 5,  // Inst{24-20} ...
/* 3404 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 3425
/* 3409 */    MCD::OPC_CheckPredicate, 6, 17, 2, 0, // Skip to: 3943
/* 3414 */    MCD::OPC_CheckField, 12, 3, 0, 10, 2, 0, // Skip to: 3943
/* 3421 */    MCD::OPC_Decode, 130, 3, 58, // Opcode: FCVT_D_W
/* 3425 */    MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 3446
/* 3430 */    MCD::OPC_CheckPredicate, 6, 252, 1, 0, // Skip to: 3943
/* 3435 */    MCD::OPC_CheckField, 12, 3, 0, 245, 1, 0, // Skip to: 3943
/* 3442 */    MCD::OPC_Decode, 131, 3, 58, // Opcode: FCVT_D_WU
/* 3446 */    MCD::OPC_FilterValue, 2, 9, 0, 0, // Skip to: 3460
/* 3451 */    MCD::OPC_CheckPredicate, 12, 231, 1, 0, // Skip to: 3943
/* 3456 */    MCD::OPC_Decode, 255, 2, 59, // Opcode: FCVT_D_L
/* 3460 */    MCD::OPC_FilterValue, 3, 222, 1, 0, // Skip to: 3943
/* 3465 */    MCD::OPC_CheckPredicate, 12, 217, 1, 0, // Skip to: 3943
/* 3470 */    MCD::OPC_Decode, 128, 3, 59, // Opcode: FCVT_D_LU
/* 3474 */    MCD::OPC_FilterValue, 112, 45, 0, 0, // Skip to: 3524
/* 3479 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3482 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 3503
/* 3487 */    MCD::OPC_CheckPredicate, 5, 195, 1, 0, // Skip to: 3943
/* 3492 */    MCD::OPC_CheckField, 20, 5, 0, 188, 1, 0, // Skip to: 3943
/* 3499 */    MCD::OPC_Decode, 171, 3, 60, // Opcode: FMV_X_W
/* 3503 */    MCD::OPC_FilterValue, 1, 179, 1, 0, // Skip to: 3943
/* 3508 */    MCD::OPC_CheckPredicate, 5, 174, 1, 0, // Skip to: 3943
/* 3513 */    MCD::OPC_CheckField, 20, 5, 0, 167, 1, 0, // Skip to: 3943
/* 3520 */    MCD::OPC_Decode, 254, 2, 60, // Opcode: FCLASS_S
/* 3524 */    MCD::OPC_FilterValue, 113, 45, 0, 0, // Skip to: 3574
/* 3529 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3532 */    MCD::OPC_FilterValue, 0, 16, 0, 0, // Skip to: 3553
/* 3537 */    MCD::OPC_CheckPredicate, 12, 145, 1, 0, // Skip to: 3943
/* 3542 */    MCD::OPC_CheckField, 20, 5, 0, 138, 1, 0, // Skip to: 3943
/* 3549 */    MCD::OPC_Decode, 170, 3, 61, // Opcode: FMV_X_D
/* 3553 */    MCD::OPC_FilterValue, 1, 129, 1, 0, // Skip to: 3943
/* 3558 */    MCD::OPC_CheckPredicate, 6, 124, 1, 0, // Skip to: 3943
/* 3563 */    MCD::OPC_CheckField, 20, 5, 0, 117, 1, 0, // Skip to: 3943
/* 3570 */    MCD::OPC_Decode, 253, 2, 61, // Opcode: FCLASS_D
/* 3574 */    MCD::OPC_FilterValue, 120, 23, 0, 0, // Skip to: 3602
/* 3579 */    MCD::OPC_CheckPredicate, 5, 103, 1, 0, // Skip to: 3943
/* 3584 */    MCD::OPC_CheckField, 20, 5, 0, 96, 1, 0, // Skip to: 3943
/* 3591 */    MCD::OPC_CheckField, 12, 3, 0, 89, 1, 0, // Skip to: 3943
/* 3598 */    MCD::OPC_Decode, 169, 3, 62, // Opcode: FMV_W_X
/* 3602 */    MCD::OPC_FilterValue, 121, 80, 1, 0, // Skip to: 3943
/* 3607 */    MCD::OPC_CheckPredicate, 12, 75, 1, 0, // Skip to: 3943
/* 3612 */    MCD::OPC_CheckField, 20, 5, 0, 68, 1, 0, // Skip to: 3943
/* 3619 */    MCD::OPC_CheckField, 12, 3, 0, 61, 1, 0, // Skip to: 3943
/* 3626 */    MCD::OPC_Decode, 168, 3, 58, // Opcode: FMV_D_X
/* 3630 */    MCD::OPC_FilterValue, 99, 57, 0, 0, // Skip to: 3692
/* 3635 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3638 */    MCD::OPC_FilterValue, 0, 4, 0, 0, // Skip to: 3647
/* 3643 */    MCD::OPC_Decode, 178, 2, 63, // Opcode: BEQ
/* 3647 */    MCD::OPC_FilterValue, 1, 4, 0, 0, // Skip to: 3656
/* 3652 */    MCD::OPC_Decode, 183, 2, 63, // Opcode: BNE
/* 3656 */    MCD::OPC_FilterValue, 4, 4, 0, 0, // Skip to: 3665
/* 3661 */    MCD::OPC_Decode, 181, 2, 63, // Opcode: BLT
/* 3665 */    MCD::OPC_FilterValue, 5, 4, 0, 0, // Skip to: 3674
/* 3670 */    MCD::OPC_Decode, 179, 2, 63, // Opcode: BGE
/* 3674 */    MCD::OPC_FilterValue, 6, 4, 0, 0, // Skip to: 3683
/* 3679 */    MCD::OPC_Decode, 182, 2, 63, // Opcode: BLTU
/* 3683 */    MCD::OPC_FilterValue, 7, 255, 0, 0, // Skip to: 3943
/* 3688 */    MCD::OPC_Decode, 180, 2, 63, // Opcode: BGEU
/* 3692 */    MCD::OPC_FilterValue, 103, 11, 0, 0, // Skip to: 3708
/* 3697 */    MCD::OPC_CheckField, 12, 3, 0, 239, 0, 0, // Skip to: 3943
/* 3704 */    MCD::OPC_Decode, 189, 3, 31, // Opcode: JALR
/* 3708 */    MCD::OPC_FilterValue, 111, 4, 0, 0, // Skip to: 3717
/* 3713 */    MCD::OPC_Decode, 188, 3, 64, // Opcode: JAL
/* 3717 */    MCD::OPC_FilterValue, 115, 221, 0, 0, // Skip to: 3943
/* 3722 */    MCD::OPC_ExtractField, 12, 3,  // Inst{14-12} ...
/* 3725 */    MCD::OPC_FilterValue, 0, 139, 0, 0, // Skip to: 3869
/* 3730 */    MCD::OPC_ExtractField, 25, 7,  // Inst{31-25} ...
/* 3733 */    MCD::OPC_FilterValue, 0, 51, 0, 0, // Skip to: 3789
/* 3738 */    MCD::OPC_ExtractField, 15, 10,  // Inst{24-15} ...
/* 3741 */    MCD::OPC_FilterValue, 0, 11, 0, 0, // Skip to: 3757
/* 3746 */    MCD::OPC_CheckField, 7, 5, 0, 190, 0, 0, // Skip to: 3943
/* 3753 */    MCD::OPC_Decode, 250, 2, 0, // Opcode: ECALL
/* 3757 */    MCD::OPC_FilterValue, 32, 11, 0, 0, // Skip to: 3773
/* 3762 */    MCD::OPC_CheckField, 7, 5, 0, 174, 0, 0, // Skip to: 3943
/* 3769 */    MCD::OPC_Decode, 249, 2, 0, // Opcode: EBREAK
/* 3773 */    MCD::OPC_FilterValue, 64, 165, 0, 0, // Skip to: 3943
/* 3778 */    MCD::OPC_CheckField, 7, 5, 0, 158, 0, 0, // Skip to: 3943
/* 3785 */    MCD::OPC_Decode, 251, 3, 0, // Opcode: URET
/* 3789 */    MCD::OPC_FilterValue, 8, 36, 0, 0, // Skip to: 3830
/* 3794 */    MCD::OPC_ExtractField, 15, 10,  // Inst{24-15} ...
/* 3797 */    MCD::OPC_FilterValue, 64, 11, 0, 0, // Skip to: 3813
/* 3802 */    MCD::OPC_CheckField, 7, 5, 0, 134, 0, 0, // Skip to: 3943
/* 3809 */    MCD::OPC_Decode, 242, 3, 0, // Opcode: SRET
/* 3813 */    MCD::OPC_FilterValue, 160, 1, 124, 0, 0, // Skip to: 3943
/* 3819 */    MCD::OPC_CheckField, 7, 5, 0, 117, 0, 0, // Skip to: 3943
/* 3826 */    MCD::OPC_Decode, 252, 3, 0, // Opcode: WFI
/* 3830 */    MCD::OPC_FilterValue, 9, 11, 0, 0, // Skip to: 3846
/* 3835 */    MCD::OPC_CheckField, 7, 5, 0, 101, 0, 0, // Skip to: 3943
/* 3842 */    MCD::OPC_Decode, 228, 3, 65, // Opcode: SFENCE_VMA
/* 3846 */    MCD::OPC_FilterValue, 24, 92, 0, 0, // Skip to: 3943
/* 3851 */    MCD::OPC_CheckField, 15, 10, 64, 85, 0, 0, // Skip to: 3943
/* 3858 */    MCD::OPC_CheckField, 7, 5, 0, 78, 0, 0, // Skip to: 3943
/* 3865 */    MCD::OPC_Decode, 206, 3, 0, // Opcode: MRET
/* 3869 */    MCD::OPC_FilterValue, 1, 24, 0, 0, // Skip to: 3898
/* 3874 */    MCD::OPC_CheckField, 15, 17, 128, 128, 6, 11, 0, 0, // Skip to: 3894
/* 3883 */    MCD::OPC_CheckField, 7, 5, 0, 4, 0, 0, // Skip to: 3894
/* 3890 */    MCD::OPC_Decode, 250, 3, 0, // Opcode: UNIMP
/* 3894 */    MCD::OPC_Decode, 188, 2, 66, // Opcode: CSRRW
/* 3898 */    MCD::OPC_FilterValue, 2, 4, 0, 0, // Skip to: 3907
/* 3903 */    MCD::OPC_Decode, 186, 2, 66, // Opcode: CSRRS
/* 3907 */    MCD::OPC_FilterValue, 3, 4, 0, 0, // Skip to: 3916
/* 3912 */    MCD::OPC_Decode, 184, 2, 66, // Opcode: CSRRC
/* 3916 */    MCD::OPC_FilterValue, 5, 4, 0, 0, // Skip to: 3925
/* 3921 */    MCD::OPC_Decode, 189, 2, 67, // Opcode: CSRRWI
/* 3925 */    MCD::OPC_FilterValue, 6, 4, 0, 0, // Skip to: 3934
/* 3930 */    MCD::OPC_Decode, 187, 2, 67, // Opcode: CSRRSI
/* 3934 */    MCD::OPC_FilterValue, 7, 4, 0, 0, // Skip to: 3943
/* 3939 */    MCD::OPC_Decode, 185, 2, 67, // Opcode: CSRRCI
/* 3943 */    MCD::OPC_Fail,
  0
};

static const uint8_t DecoderTableRISCV32Only_16[] = {
/* 0 */       MCD::OPC_ExtractField, 0, 2,  // Inst{1-0} ...
/* 3 */       MCD::OPC_FilterValue, 0, 31, 0, 0, // Skip to: 39
/* 8 */       MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 11 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 25
/* 16 */      MCD::OPC_CheckPredicate, 13, 75, 0, 0, // Skip to: 96
/* 21 */      MCD::OPC_Decode, 207, 2, 68, // Opcode: C_FLW
/* 25 */      MCD::OPC_FilterValue, 7, 66, 0, 0, // Skip to: 96
/* 30 */      MCD::OPC_CheckPredicate, 13, 61, 0, 0, // Skip to: 96
/* 35 */      MCD::OPC_Decode, 211, 2, 68, // Opcode: C_FSW
/* 39 */      MCD::OPC_FilterValue, 1, 16, 0, 0, // Skip to: 60
/* 44 */      MCD::OPC_CheckPredicate, 14, 47, 0, 0, // Skip to: 96
/* 49 */      MCD::OPC_CheckField, 13, 3, 1, 40, 0, 0, // Skip to: 96
/* 56 */      MCD::OPC_Decode, 214, 2, 26, // Opcode: C_JAL
/* 60 */      MCD::OPC_FilterValue, 2, 31, 0, 0, // Skip to: 96
/* 65 */      MCD::OPC_ExtractField, 13, 3,  // Inst{15-13} ...
/* 68 */      MCD::OPC_FilterValue, 3, 9, 0, 0, // Skip to: 82
/* 73 */      MCD::OPC_CheckPredicate, 13, 18, 0, 0, // Skip to: 96
/* 78 */      MCD::OPC_Decode, 208, 2, 69, // Opcode: C_FLWSP
/* 82 */      MCD::OPC_FilterValue, 7, 9, 0, 0, // Skip to: 96
/* 87 */      MCD::OPC_CheckPredicate, 13, 4, 0, 0, // Skip to: 96
/* 92 */      MCD::OPC_Decode, 212, 2, 70, // Opcode: C_FSWSP
/* 96 */      MCD::OPC_Fail,
  0
};

static bool checkDecoderPredicate(unsigned Idx, const FeatureBitset& Bits) {
  switch (Idx) {
  default: llvm_unreachable("Invalid index!");
  case 0:
    return (Bits[RISCV::FeatureStdExtC]);
  case 1:
    return (Bits[RISCV::FeatureStdExtC] && Bits[RISCV::FeatureRVCHints]);
  case 2:
    return (Bits[RISCV::FeatureStdExtC] && Bits[RISCV::FeatureStdExtD]);
  case 3:
    return (Bits[RISCV::FeatureStdExtC] && Bits[RISCV::Feature64Bit]);
  case 4:
    return (Bits[RISCV::Feature64Bit]);
  case 5:
    return (Bits[RISCV::FeatureStdExtF]);
  case 6:
    return (Bits[RISCV::FeatureStdExtD]);
  case 7:
    return (Bits[RISCV::FeatureStdExtA]);
  case 8:
    return (Bits[RISCV::FeatureStdExtA] && Bits[RISCV::Feature64Bit]);
  case 9:
    return (Bits[RISCV::FeatureStdExtM]);
  case 10:
    return (Bits[RISCV::FeatureStdExtM] && Bits[RISCV::Feature64Bit]);
  case 11:
    return (Bits[RISCV::FeatureStdExtF] && Bits[RISCV::Feature64Bit]);
  case 12:
    return (Bits[RISCV::FeatureStdExtD] && Bits[RISCV::Feature64Bit]);
  case 13:
    return (Bits[RISCV::FeatureStdExtC] && Bits[RISCV::FeatureStdExtF] && !Bits[RISCV::Feature64Bit]);
  case 14:
    return (Bits[RISCV::FeatureStdExtC] && !Bits[RISCV::Feature64Bit]);
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
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 5, 1) << 3;
    tmp |= fieldFromInstruction(insn, 6, 1) << 2;
    tmp |= fieldFromInstruction(insn, 7, 4) << 6;
    tmp |= fieldFromInstruction(insn, 11, 2) << 4;
    if (decodeUImmNonZeroOperand<10>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 2:
    if (decodeRVCInstrSImm(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 3:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeSImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 4:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 5:
    if (decodeRVCInstrRdRs1UImm(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 6:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 7:
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeFPR64CRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 5, 2) << 6;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 8:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 3) << 6;
    tmp |= fieldFromInstruction(insn, 5, 2) << 3;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<9>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 9:
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 5, 1) << 6;
    tmp |= fieldFromInstruction(insn, 6, 1) << 2;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<7>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 10:
    if (decodeRVCInstrRdSImm(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 11:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeSImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 12:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 2) << 6;
    tmp |= fieldFromInstruction(insn, 4, 3) << 2;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 13:
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 5, 2) << 6;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 14:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 1) << 5;
    tmp |= fieldFromInstruction(insn, 3, 2) << 7;
    tmp |= fieldFromInstruction(insn, 5, 1) << 6;
    tmp |= fieldFromInstruction(insn, 6, 1) << 4;
    tmp |= fieldFromInstruction(insn, 12, 1) << 9;
    if (decodeSImmNonZeroOperand<10>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 15:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0X2RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeCLUIImmOperand(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 16:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 3) << 6;
    tmp |= fieldFromInstruction(insn, 5, 2) << 3;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<9>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 17:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 18:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 19:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 5) << 0;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeSImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 20:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 21:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 22:
    if (decodeRVCInstrRdRs2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 23:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 24:
    if (decodeRVCInstrRdRs1Rs2(MI, insn, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 25:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeGPRNoX0RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 26:
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 1) << 4;
    tmp |= fieldFromInstruction(insn, 3, 3) << 0;
    tmp |= fieldFromInstruction(insn, 6, 1) << 6;
    tmp |= fieldFromInstruction(insn, 7, 1) << 5;
    tmp |= fieldFromInstruction(insn, 8, 1) << 9;
    tmp |= fieldFromInstruction(insn, 9, 2) << 7;
    tmp |= fieldFromInstruction(insn, 11, 1) << 3;
    tmp |= fieldFromInstruction(insn, 12, 1) << 10;
    if (decodeSImmOperandAndLsl1<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 27:
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 3) << 6;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<9>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 28:
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 1) << 4;
    tmp |= fieldFromInstruction(insn, 3, 2) << 0;
    tmp |= fieldFromInstruction(insn, 5, 2) << 5;
    tmp |= fieldFromInstruction(insn, 10, 2) << 2;
    tmp |= fieldFromInstruction(insn, 12, 1) << 7;
    if (decodeSImmOperandAndLsl1<9>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 29:
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 2) << 6;
    tmp |= fieldFromInstruction(insn, 9, 4) << 2;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 30:
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 3) << 6;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<9>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 31:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 12);
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 32:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 12);
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 33:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 12);
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 34:
    tmp = fieldFromInstruction(insn, 24, 4);
    if (decodeUImmOperand<4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 4);
    if (decodeUImmOperand<4>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 35:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 6);
    if (decodeUImmOperand<6>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 36:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 20);
    if (decodeUImmOperand<20>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 37:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (decodeUImmOperand<5>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 38:
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 5) << 0;
    tmp |= fieldFromInstruction(insn, 25, 7) << 5;
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 39:
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 5) << 0;
    tmp |= fieldFromInstruction(insn, 25, 7) << 5;
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 40:
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 5) << 0;
    tmp |= fieldFromInstruction(insn, 25, 7) << 5;
    if (decodeSImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 41:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 42:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 43:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 27, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 44:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 27, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 45:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 46:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 47:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 48:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 49:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 50:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 51:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 52:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 53:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 54:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 55:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 56:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 57:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 58:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 59:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 12, 3);
    if (decodeFRMArg(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 60:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 61:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeFPR64RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 62:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 63:
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 1) << 10;
    tmp |= fieldFromInstruction(insn, 8, 4) << 0;
    tmp |= fieldFromInstruction(insn, 25, 6) << 4;
    tmp |= fieldFromInstruction(insn, 31, 1) << 11;
    if (decodeSImmOperandAndLsl1<13>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 64:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 12, 8) << 11;
    tmp |= fieldFromInstruction(insn, 20, 1) << 10;
    tmp |= fieldFromInstruction(insn, 21, 10) << 0;
    tmp |= fieldFromInstruction(insn, 31, 1) << 19;
    if (decodeSImmOperandAndLsl1<21>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 65:
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 66:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 12);
    if (decodeUImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 67:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeGPRRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 20, 12);
    if (decodeUImmOperand<12>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 15, 5);
    if (decodeUImmOperand<5>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 68:
    tmp = fieldFromInstruction(insn, 2, 3);
    if (DecodeFPR32CRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = fieldFromInstruction(insn, 7, 3);
    if (DecodeGPRCRegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 5, 1) << 6;
    tmp |= fieldFromInstruction(insn, 6, 1) << 2;
    tmp |= fieldFromInstruction(insn, 10, 3) << 3;
    if (decodeUImmOperand<7>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 69:
    tmp = fieldFromInstruction(insn, 7, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 2, 2) << 6;
    tmp |= fieldFromInstruction(insn, 4, 3) << 2;
    tmp |= fieldFromInstruction(insn, 12, 1) << 5;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    return S;
  case 70:
    tmp = fieldFromInstruction(insn, 2, 5);
    if (DecodeFPR32RegisterClass(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
    tmp = 0x0;
    tmp |= fieldFromInstruction(insn, 7, 2) << 6;
    tmp |= fieldFromInstruction(insn, 9, 4) << 2;
    if (decodeUImmOperand<8>(MI, tmp, Address, Decoder) == MCDisassembler::Fail) { return MCDisassembler::Fail; }
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
