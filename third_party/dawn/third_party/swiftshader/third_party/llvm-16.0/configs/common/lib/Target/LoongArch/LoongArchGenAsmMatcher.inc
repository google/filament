/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Assembly Matcher Source Fragment                                           *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GET_ASSEMBLER_HEADER
#undef GET_ASSEMBLER_HEADER
  // This should be included into the middle of the declaration of
  // your subclasses implementation of MCTargetAsmParser.
  FeatureBitset ComputeAvailableFeatures(const FeatureBitset &FB) const;
  void convertToMCInst(unsigned Kind, MCInst &Inst, unsigned Opcode,
                       const OperandVector &Operands);
  void convertToMapAndConstraints(unsigned Kind,
                           const OperandVector &Operands) override;
  unsigned MatchInstructionImpl(const OperandVector &Operands,
                                MCInst &Inst,
                                uint64_t &ErrorInfo,
                                FeatureBitset &MissingFeatures,
                                bool matchingInlineAsm,
                                unsigned VariantID = 0);
  unsigned MatchInstructionImpl(const OperandVector &Operands,
                                MCInst &Inst,
                                uint64_t &ErrorInfo,
                                bool matchingInlineAsm,
                                unsigned VariantID = 0) {
    FeatureBitset MissingFeatures;
    return MatchInstructionImpl(Operands, Inst, ErrorInfo, MissingFeatures,
                                matchingInlineAsm, VariantID);
  }

  OperandMatchResultTy MatchOperandParserImpl(
    OperandVector &Operands,
    StringRef Mnemonic,
    bool ParseForAllFeatures = false);
  OperandMatchResultTy tryCustomParseOperand(
    OperandVector &Operands,
    unsigned MCK);

#endif // GET_ASSEMBLER_HEADER_INFO


#ifdef GET_OPERAND_DIAGNOSTIC_TYPES
#undef GET_OPERAND_DIAGNOSTIC_TYPES

  Match_InvalidBareSymbol,
  Match_InvalidImm32,
  Match_InvalidSImm12,
  Match_InvalidSImm12addlike,
  Match_InvalidSImm12lu52id,
  Match_InvalidSImm14lsl2,
  Match_InvalidSImm16,
  Match_InvalidSImm16lsl2,
  Match_InvalidSImm20,
  Match_InvalidSImm20lu12iw,
  Match_InvalidSImm20lu32id,
  Match_InvalidSImm20pcalau12i,
  Match_InvalidSImm21lsl2,
  Match_InvalidSImm26Operand,
  Match_InvalidUImm12,
  Match_InvalidUImm12ori,
  Match_InvalidUImm14,
  Match_InvalidUImm15,
  Match_InvalidUImm2,
  Match_InvalidUImm2plus1,
  Match_InvalidUImm3,
  Match_InvalidUImm5,
  Match_InvalidUImm6,
  Match_InvalidUImm8,
  END_OPERAND_DIAGNOSTIC_TYPES
#endif // GET_OPERAND_DIAGNOSTIC_TYPES


#ifdef GET_REGISTER_MATCHER
#undef GET_REGISTER_MATCHER

// Bits for subtarget features that participate in instruction matching.
enum SubtargetFeatureBits : uint8_t {
  Feature_IsLA64Bit = 10,
  Feature_IsLA32Bit = 9,
  Feature_HasBasicFBit = 1,
  Feature_HasBasicDBit = 0,
  Feature_HasExtLSXBit = 4,
  Feature_HasExtLASXBit = 2,
  Feature_HasExtLVZBit = 5,
  Feature_HasExtLBTBit = 3,
  Feature_HasLaGlobalWithPcrelBit = 7,
  Feature_HasLaGlobalWithAbsBit = 6,
  Feature_HasLaLocalWithAbsBit = 8,
};

static unsigned MatchRegisterName(StringRef Name) {
  switch (Name.size()) {
  default: break;
  case 2:	 // 30 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 20 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 1;	 // "f0"
      case '1':	 // 2 strings to match.
        return 2;	 // "f1"
      case '2':	 // 2 strings to match.
        return 3;	 // "f2"
      case '3':	 // 2 strings to match.
        return 4;	 // "f3"
      case '4':	 // 2 strings to match.
        return 5;	 // "f4"
      case '5':	 // 2 strings to match.
        return 6;	 // "f5"
      case '6':	 // 2 strings to match.
        return 7;	 // "f6"
      case '7':	 // 2 strings to match.
        return 8;	 // "f7"
      case '8':	 // 2 strings to match.
        return 9;	 // "f8"
      case '9':	 // 2 strings to match.
        return 10;	 // "f9"
      }
      break;
    case 'r':	 // 10 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 1 string to match.
        return 45;	 // "r0"
      case '1':	 // 1 string to match.
        return 46;	 // "r1"
      case '2':	 // 1 string to match.
        return 47;	 // "r2"
      case '3':	 // 1 string to match.
        return 48;	 // "r3"
      case '4':	 // 1 string to match.
        return 49;	 // "r4"
      case '5':	 // 1 string to match.
        return 50;	 // "r5"
      case '6':	 // 1 string to match.
        return 51;	 // "r6"
      case '7':	 // 1 string to match.
        return 52;	 // "r7"
      case '8':	 // 1 string to match.
        return 53;	 // "r8"
      case '9':	 // 1 string to match.
        return 54;	 // "r9"
      }
      break;
    }
    break;
  case 3:	 // 66 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 44 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 20 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 11;	 // "f10"
        case '1':	 // 2 strings to match.
          return 12;	 // "f11"
        case '2':	 // 2 strings to match.
          return 13;	 // "f12"
        case '3':	 // 2 strings to match.
          return 14;	 // "f13"
        case '4':	 // 2 strings to match.
          return 15;	 // "f14"
        case '5':	 // 2 strings to match.
          return 16;	 // "f15"
        case '6':	 // 2 strings to match.
          return 17;	 // "f16"
        case '7':	 // 2 strings to match.
          return 18;	 // "f17"
        case '8':	 // 2 strings to match.
          return 19;	 // "f18"
        case '9':	 // 2 strings to match.
          return 20;	 // "f19"
        }
        break;
      case '2':	 // 20 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 21;	 // "f20"
        case '1':	 // 2 strings to match.
          return 22;	 // "f21"
        case '2':	 // 2 strings to match.
          return 23;	 // "f22"
        case '3':	 // 2 strings to match.
          return 24;	 // "f23"
        case '4':	 // 2 strings to match.
          return 25;	 // "f24"
        case '5':	 // 2 strings to match.
          return 26;	 // "f25"
        case '6':	 // 2 strings to match.
          return 27;	 // "f26"
        case '7':	 // 2 strings to match.
          return 28;	 // "f27"
        case '8':	 // 2 strings to match.
          return 29;	 // "f28"
        case '9':	 // 2 strings to match.
          return 30;	 // "f29"
        }
        break;
      case '3':	 // 4 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 31;	 // "f30"
        case '1':	 // 2 strings to match.
          return 32;	 // "f31"
        }
        break;
      }
      break;
    case 'r':	 // 22 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 10 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 1 string to match.
          return 55;	 // "r10"
        case '1':	 // 1 string to match.
          return 56;	 // "r11"
        case '2':	 // 1 string to match.
          return 57;	 // "r12"
        case '3':	 // 1 string to match.
          return 58;	 // "r13"
        case '4':	 // 1 string to match.
          return 59;	 // "r14"
        case '5':	 // 1 string to match.
          return 60;	 // "r15"
        case '6':	 // 1 string to match.
          return 61;	 // "r16"
        case '7':	 // 1 string to match.
          return 62;	 // "r17"
        case '8':	 // 1 string to match.
          return 63;	 // "r18"
        case '9':	 // 1 string to match.
          return 64;	 // "r19"
        }
        break;
      case '2':	 // 10 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 1 string to match.
          return 65;	 // "r20"
        case '1':	 // 1 string to match.
          return 66;	 // "r21"
        case '2':	 // 1 string to match.
          return 67;	 // "r22"
        case '3':	 // 1 string to match.
          return 68;	 // "r23"
        case '4':	 // 1 string to match.
          return 69;	 // "r24"
        case '5':	 // 1 string to match.
          return 70;	 // "r25"
        case '6':	 // 1 string to match.
          return 71;	 // "r26"
        case '7':	 // 1 string to match.
          return 72;	 // "r27"
        case '8':	 // 1 string to match.
          return 73;	 // "r28"
        case '9':	 // 1 string to match.
          return 74;	 // "r29"
        }
        break;
      case '3':	 // 2 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 1 string to match.
          return 75;	 // "r30"
        case '1':	 // 1 string to match.
          return 76;	 // "r31"
        }
        break;
      }
      break;
    }
    break;
  case 4:	 // 8 strings to match.
    if (memcmp(Name.data()+0, "fcc", 3) != 0)
      break;
    switch (Name[3]) {
    default: break;
    case '0':	 // 1 string to match.
      return 33;	 // "fcc0"
    case '1':	 // 1 string to match.
      return 34;	 // "fcc1"
    case '2':	 // 1 string to match.
      return 35;	 // "fcc2"
    case '3':	 // 1 string to match.
      return 36;	 // "fcc3"
    case '4':	 // 1 string to match.
      return 37;	 // "fcc4"
    case '5':	 // 1 string to match.
      return 38;	 // "fcc5"
    case '6':	 // 1 string to match.
      return 39;	 // "fcc6"
    case '7':	 // 1 string to match.
      return 40;	 // "fcc7"
    }
    break;
  case 5:	 // 4 strings to match.
    if (memcmp(Name.data()+0, "fcsr", 4) != 0)
      break;
    switch (Name[4]) {
    default: break;
    case '0':	 // 1 string to match.
      return 41;	 // "fcsr0"
    case '1':	 // 1 string to match.
      return 42;	 // "fcsr1"
    case '2':	 // 1 string to match.
      return 43;	 // "fcsr2"
    case '3':	 // 1 string to match.
      return 44;	 // "fcsr3"
    }
    break;
  }
  return 0;
}

static unsigned MatchRegisterAltName(StringRef Name) {
  switch (Name.size()) {
  default: break;
  case 2:	 // 31 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 8 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 1 string to match.
        return 49;	 // "a0"
      case '1':	 // 1 string to match.
        return 50;	 // "a1"
      case '2':	 // 1 string to match.
        return 51;	 // "a2"
      case '3':	 // 1 string to match.
        return 52;	 // "a3"
      case '4':	 // 1 string to match.
        return 53;	 // "a4"
      case '5':	 // 1 string to match.
        return 54;	 // "a5"
      case '6':	 // 1 string to match.
        return 55;	 // "a6"
      case '7':	 // 1 string to match.
        return 56;	 // "a7"
      }
      break;
    case 'f':	 // 1 string to match.
      if (Name[1] != 'p')
        break;
      return 67;	 // "fp"
    case 'r':	 // 1 string to match.
      if (Name[1] != 'a')
        break;
      return 46;	 // "ra"
    case 's':	 // 11 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 1 string to match.
        return 68;	 // "s0"
      case '1':	 // 1 string to match.
        return 69;	 // "s1"
      case '2':	 // 1 string to match.
        return 70;	 // "s2"
      case '3':	 // 1 string to match.
        return 71;	 // "s3"
      case '4':	 // 1 string to match.
        return 72;	 // "s4"
      case '5':	 // 1 string to match.
        return 73;	 // "s5"
      case '6':	 // 1 string to match.
        return 74;	 // "s6"
      case '7':	 // 1 string to match.
        return 75;	 // "s7"
      case '8':	 // 1 string to match.
        return 76;	 // "s8"
      case '9':	 // 1 string to match.
        return 67;	 // "s9"
      case 'p':	 // 1 string to match.
        return 48;	 // "sp"
      }
      break;
    case 't':	 // 10 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 1 string to match.
        return 57;	 // "t0"
      case '1':	 // 1 string to match.
        return 58;	 // "t1"
      case '2':	 // 1 string to match.
        return 59;	 // "t2"
      case '3':	 // 1 string to match.
        return 60;	 // "t3"
      case '4':	 // 1 string to match.
        return 61;	 // "t4"
      case '5':	 // 1 string to match.
        return 62;	 // "t5"
      case '6':	 // 1 string to match.
        return 63;	 // "t6"
      case '7':	 // 1 string to match.
        return 64;	 // "t7"
      case '8':	 // 1 string to match.
        return 65;	 // "t8"
      case 'p':	 // 1 string to match.
        return 47;	 // "tp"
      }
      break;
    }
    break;
  case 3:	 // 52 strings to match.
    if (Name[0] != 'f')
      break;
    switch (Name[1]) {
    default: break;
    case 'a':	 // 16 strings to match.
      switch (Name[2]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 1;	 // "fa0"
      case '1':	 // 2 strings to match.
        return 2;	 // "fa1"
      case '2':	 // 2 strings to match.
        return 3;	 // "fa2"
      case '3':	 // 2 strings to match.
        return 4;	 // "fa3"
      case '4':	 // 2 strings to match.
        return 5;	 // "fa4"
      case '5':	 // 2 strings to match.
        return 6;	 // "fa5"
      case '6':	 // 2 strings to match.
        return 7;	 // "fa6"
      case '7':	 // 2 strings to match.
        return 8;	 // "fa7"
      }
      break;
    case 's':	 // 16 strings to match.
      switch (Name[2]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 25;	 // "fs0"
      case '1':	 // 2 strings to match.
        return 26;	 // "fs1"
      case '2':	 // 2 strings to match.
        return 27;	 // "fs2"
      case '3':	 // 2 strings to match.
        return 28;	 // "fs3"
      case '4':	 // 2 strings to match.
        return 29;	 // "fs4"
      case '5':	 // 2 strings to match.
        return 30;	 // "fs5"
      case '6':	 // 2 strings to match.
        return 31;	 // "fs6"
      case '7':	 // 2 strings to match.
        return 32;	 // "fs7"
      }
      break;
    case 't':	 // 20 strings to match.
      switch (Name[2]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 9;	 // "ft0"
      case '1':	 // 2 strings to match.
        return 10;	 // "ft1"
      case '2':	 // 2 strings to match.
        return 11;	 // "ft2"
      case '3':	 // 2 strings to match.
        return 12;	 // "ft3"
      case '4':	 // 2 strings to match.
        return 13;	 // "ft4"
      case '5':	 // 2 strings to match.
        return 14;	 // "ft5"
      case '6':	 // 2 strings to match.
        return 15;	 // "ft6"
      case '7':	 // 2 strings to match.
        return 16;	 // "ft7"
      case '8':	 // 2 strings to match.
        return 17;	 // "ft8"
      case '9':	 // 2 strings to match.
        return 18;	 // "ft9"
      }
      break;
    }
    break;
  case 4:	 // 13 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 12 strings to match.
      if (memcmp(Name.data()+1, "t1", 2) != 0)
        break;
      switch (Name[3]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 19;	 // "ft10"
      case '1':	 // 2 strings to match.
        return 20;	 // "ft11"
      case '2':	 // 2 strings to match.
        return 21;	 // "ft12"
      case '3':	 // 2 strings to match.
        return 22;	 // "ft13"
      case '4':	 // 2 strings to match.
        return 23;	 // "ft14"
      case '5':	 // 2 strings to match.
        return 24;	 // "ft15"
      }
      break;
    case 'z':	 // 1 string to match.
      if (memcmp(Name.data()+1, "ero", 3) != 0)
        break;
      return 45;	 // "zero"
    }
    break;
  }
  return 0;
}

#endif // GET_REGISTER_MATCHER


#ifdef GET_SUBTARGET_FEATURE_NAME
#undef GET_SUBTARGET_FEATURE_NAME

// User-level names for subtarget features that participate in
// instruction matching.
static const char *getSubtargetFeatureName(uint64_t Val) {
  switch(Val) {
  case Feature_IsLA64Bit: return "LA64 Basic Integer and Privilege Instruction Set";
  case Feature_IsLA32Bit: return "LA32 Basic Integer and Privilege Instruction Set";
  case Feature_HasBasicFBit: return "'F' (Single-Precision Floating-Point)";
  case Feature_HasBasicDBit: return "'D' (Double-Precision Floating-Point)";
  case Feature_HasExtLSXBit: return "'LSX' (Loongson SIMD Extension)";
  case Feature_HasExtLASXBit: return "'LASX' (Loongson Advanced SIMD Extension)";
  case Feature_HasExtLVZBit: return "'LVZ' (Loongson Virtualization Extension)";
  case Feature_HasExtLBTBit: return "'LBT' (Loongson Binary Translation Extension)";
  case Feature_HasLaGlobalWithPcrelBit: return "Expand la.global as la.pcrel";
  case Feature_HasLaGlobalWithAbsBit: return "Expand la.global as la.abs";
  case Feature_HasLaLocalWithAbsBit: return "Expand la.local as la.abs";
  default: return "(unknown)";
  }
}

#endif // GET_SUBTARGET_FEATURE_NAME


#ifdef GET_MATCHER_IMPLEMENTATION
#undef GET_MATCHER_IMPLEMENTATION

enum {
  Tie0_1_1,
};

static const uint8_t TiedAsmOperandTable[][3] = {
  /* Tie0_1_1 */ { 0, 1, 1 },
};

namespace {
enum OperatorConversionKind {
  CVT_Done,
  CVT_Reg,
  CVT_Tied,
  CVT_95_Reg,
  CVT_95_addImmOperands,
  CVT_95_addRegOperands,
  CVT_regR0,
  CVT_imm_95_0,
  CVT_regR1,
  CVT_NUM_CONVERTERS
};

enum InstructionConversionKind {
  Convert__Reg1_0__Reg1_1__Reg1_2,
  Convert__Reg1_0__Reg1_1__SImm12addlike1_2,
  Convert__Reg1_0__Reg1_1__SImm161_2,
  Convert__Reg1_0__Reg1_1__Reg1_2__UImm2plus11_3,
  Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2,
  Convert__Reg1_0__Reg1_1__UImm121_2,
  Convert__Reg1_0__Reg1_1,
  Convert__SImm26OperandB1_0,
  Convert__Reg1_0__SImm21lsl21_1,
  Convert__Reg1_0__Reg1_1__SImm16lsl21_2,
  Convert__Reg1_0__regR0__SImm16lsl21_1,
  Convert__Reg1_1__Reg1_0__SImm16lsl21_2,
  Convert__regR0__Reg1_0__SImm16lsl21_1,
  Convert__SImm26OperandBL1_0,
  Convert__UImm151_0,
  Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm61_2__UImm61_3,
  Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm51_2__UImm51_3,
  Convert__Reg1_0__Reg1_1__UImm61_2__UImm61_3,
  Convert__Reg1_0__Reg1_1__UImm51_2__UImm51_3,
  Convert__Reg1_0__Reg1_1__Reg1_2__UImm31_3,
  Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3,
  Convert__UImm51_0__Reg1_1__SImm121_2,
  Convert__Reg1_0__UImm141_1,
  Convert__Reg1_0__Tie0_1_1__UImm141_1,
  Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm141_2,
  Convert_NoOperands,
  Convert__Reg1_0__Reg1_1__SImm121_2,
  Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3,
  Convert__Reg1_2__Reg1_1__UImm51_0,
  Convert__regR0__Reg1_0__imm_95_0,
  Convert__Reg1_0__BareSymbol1_1,
  Convert__Reg1_0__imm_95_0__BareSymbol1_1,
  Convert__Reg1_0__Reg1_1__BareSymbol1_2,
  Convert__Reg1_0__Reg1_1__UImm81_2,
  Convert__Reg1_0__UImm81_1,
  Convert__Reg1_0__Reg1_1__SImm14lsl21_2,
  Convert__Reg1_0__Imm1_1,
  Convert__Reg1_0__Imm321_1,
  Convert__Reg1_0__SImm20lu12iw1_1,
  Convert__Reg1_0__Tie0_1_1__SImm20lu32id1_1,
  Convert__Reg1_0__Reg1_1__SImm12lu52id1_2,
  Convert__Reg1_0__Reg1_1__regR0,
  Convert__Reg1_0__Tie0_1_1__Reg1_1,
  Convert__regR0__regR0__imm_95_0,
  Convert__Reg1_0__Reg1_1__UImm12ori1_2,
  Convert__Reg1_0__SImm201_1,
  Convert__Reg1_0__SImm20pcalau12i1_1,
  Convert__UImm51_0__Reg1_1__Reg1_2,
  Convert__regR0__regR1__imm_95_0,
  Convert__Reg1_0__Reg1_1__UImm61_2,
  Convert__Reg1_0__Reg1_1__UImm51_2,
  Convert__Reg1_0__Tie0_1_1__Reg1_1__SImm14lsl21_2,
  CVT_NUM_SIGNATURES
};

} // end anonymous namespace

static const uint8_t ConversionTable[CVT_NUM_SIGNATURES][11] = {
  // Convert__Reg1_0__Reg1_1__Reg1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm12addlike1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm161_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__UImm2plus11_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm121_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_Done },
  // Convert__SImm26OperandB1_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__SImm21lsl21_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm16lsl21_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__regR0__SImm16lsl21_1
  { CVT_95_Reg, 1, CVT_regR0, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_1__Reg1_0__SImm16lsl21_2
  { CVT_95_Reg, 2, CVT_95_Reg, 1, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__regR0__Reg1_0__SImm16lsl21_1
  { CVT_regR0, 0, CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__SImm26OperandBL1_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__UImm151_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm61_2__UImm61_3
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm51_2__UImm51_3
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm61_2__UImm61_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm51_2__UImm51_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__UImm31_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__UImm51_0__Reg1_1__SImm121_2
  { CVT_95_addImmOperands, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__UImm141_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__UImm141_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm141_2
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert_NoOperands
  { CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm121_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_Reg, 4, CVT_Done },
  // Convert__Reg1_2__Reg1_1__UImm51_0
  { CVT_95_Reg, 3, CVT_95_Reg, 2, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__regR0__Reg1_0__imm_95_0
  { CVT_regR0, 0, CVT_95_Reg, 1, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__BareSymbol1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__imm_95_0__BareSymbol1_1
  { CVT_95_Reg, 1, CVT_imm_95_0, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__BareSymbol1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm81_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__UImm81_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm14lsl21_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Imm1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Imm321_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__SImm20lu12iw1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__SImm20lu32id1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm12lu52id1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__regR0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_regR0, 0, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_Done },
  // Convert__regR0__regR0__imm_95_0
  { CVT_regR0, 0, CVT_regR0, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm12ori1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__SImm201_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__SImm20pcalau12i1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__UImm51_0__Reg1_1__Reg1_2
  { CVT_95_addImmOperands, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_Done },
  // Convert__regR0__regR1__imm_95_0
  { CVT_regR0, 0, CVT_regR1, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm61_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm51_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1__SImm14lsl21_2
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
};

void LoongArchAsmParser::
convertToMCInst(unsigned Kind, MCInst &Inst, unsigned Opcode,
                const OperandVector &Operands) {
  assert(Kind < CVT_NUM_SIGNATURES && "Invalid signature!");
  const uint8_t *Converter = ConversionTable[Kind];
  unsigned OpIdx;
  Inst.setOpcode(Opcode);
  for (const uint8_t *p = Converter; *p; p += 2) {
    OpIdx = *(p + 1);
    switch (*p) {
    default: llvm_unreachable("invalid conversion entry!");
    case CVT_Reg:
      static_cast<LoongArchOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      break;
    case CVT_Tied: {
      assert(OpIdx < (size_t)(std::end(TiedAsmOperandTable) -
                              std::begin(TiedAsmOperandTable)) &&
             "Tied operand not found");
      unsigned TiedResOpnd = TiedAsmOperandTable[OpIdx][0];
      if (TiedResOpnd != (uint8_t)-1)
        Inst.addOperand(Inst.getOperand(TiedResOpnd));
      break;
    }
    case CVT_95_Reg:
      static_cast<LoongArchOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      break;
    case CVT_95_addImmOperands:
      static_cast<LoongArchOperand &>(*Operands[OpIdx]).addImmOperands(Inst, 1);
      break;
    case CVT_95_addRegOperands:
      static_cast<LoongArchOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      break;
    case CVT_regR0:
      Inst.addOperand(MCOperand::createReg(LoongArch::R0));
      break;
    case CVT_imm_95_0:
      Inst.addOperand(MCOperand::createImm(0));
      break;
    case CVT_regR1:
      Inst.addOperand(MCOperand::createReg(LoongArch::R1));
      break;
    }
  }
}

void LoongArchAsmParser::
convertToMapAndConstraints(unsigned Kind,
                           const OperandVector &Operands) {
  assert(Kind < CVT_NUM_SIGNATURES && "Invalid signature!");
  unsigned NumMCOperands = 0;
  const uint8_t *Converter = ConversionTable[Kind];
  for (const uint8_t *p = Converter; *p; p += 2) {
    switch (*p) {
    default: llvm_unreachable("invalid conversion entry!");
    case CVT_Reg:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("r");
      ++NumMCOperands;
      break;
    case CVT_Tied:
      ++NumMCOperands;
      break;
    case CVT_95_Reg:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("r");
      NumMCOperands += 1;
      break;
    case CVT_95_addImmOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_95_addRegOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_regR0:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_imm_95_0:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_regR1:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    }
  }
}

namespace {

/// MatchClassKind - The kinds of classes which participate in
/// instruction matching.
enum MatchClassKind {
  InvalidMatchClass = 0,
  OptionalMatchClass = 1,
  MCK_LAST_TOKEN = OptionalMatchClass,
  MCK_FCSR, // register class 'FCSR'
  MCK_CFR, // register class 'CFR'
  MCK_GPRT, // register class 'GPRT'
  MCK_FPR32, // register class 'FPR32'
  MCK_FPR64, // register class 'FPR64'
  MCK_GPR, // register class 'GPR'
  MCK_LAST_REGISTER = MCK_GPR,
  MCK_AtomicMemAsmOperand, // user defined class 'AtomicMemAsmOperand'
  MCK_BareSymbol, // user defined class 'BareSymbol'
  MCK_Imm, // user defined class 'ImmAsmOperand'
  MCK_SImm26OperandB, // user defined class 'SImm26OperandB'
  MCK_SImm26OperandBL, // user defined class 'SImm26OperandBL'
  MCK_Imm32, // user defined class 'anonymous_3918'
  MCK_UImm2, // user defined class 'anonymous_3919'
  MCK_UImm2plus1, // user defined class 'anonymous_3920'
  MCK_UImm3, // user defined class 'anonymous_3921'
  MCK_UImm5, // user defined class 'anonymous_3922'
  MCK_UImm6, // user defined class 'anonymous_3923'
  MCK_UImm8, // user defined class 'anonymous_3924'
  MCK_UImm12, // user defined class 'anonymous_3925'
  MCK_UImm12ori, // user defined class 'anonymous_3926'
  MCK_UImm14, // user defined class 'anonymous_3927'
  MCK_UImm15, // user defined class 'anonymous_3928'
  MCK_SImm12, // user defined class 'anonymous_3929'
  MCK_SImm12addlike, // user defined class 'anonymous_3930'
  MCK_SImm12lu52id, // user defined class 'anonymous_3931'
  MCK_SImm14lsl2, // user defined class 'anonymous_3932'
  MCK_SImm16, // user defined class 'anonymous_3933'
  MCK_SImm16lsl2, // user defined class 'anonymous_3934'
  MCK_SImm20, // user defined class 'anonymous_3935'
  MCK_SImm20pcalau12i, // user defined class 'anonymous_3936'
  MCK_SImm20lu12iw, // user defined class 'anonymous_3937'
  MCK_SImm20lu32id, // user defined class 'anonymous_3938'
  MCK_SImm21lsl2, // user defined class 'anonymous_3939'
  NumMatchClassKinds
};

} // end anonymous namespace

static unsigned getDiagKindFromRegisterClass(MatchClassKind RegisterClass) {
  return MCTargetAsmParser::Match_InvalidOperand;
}

static MatchClassKind matchTokenString(StringRef Name) {
  return InvalidMatchClass;
}

/// isSubclass - Compute whether \p A is a subclass of \p B.
static bool isSubclass(MatchClassKind A, MatchClassKind B) {
  if (A == B)
    return true;

  switch (A) {
  default:
    return false;

  case MCK_GPRT:
    return B == MCK_GPR;
  }
}

static unsigned validateOperandClass(MCParsedAsmOperand &GOp, MatchClassKind Kind) {
  LoongArchOperand &Operand = (LoongArchOperand &)GOp;
  if (Kind == InvalidMatchClass)
    return MCTargetAsmParser::Match_InvalidOperand;

  if (Operand.isToken() && Kind <= MCK_LAST_TOKEN)
    return isSubclass(matchTokenString(Operand.getToken()), Kind) ?
             MCTargetAsmParser::Match_Success :
             MCTargetAsmParser::Match_InvalidOperand;

  switch (Kind) {
  default: break;
  // 'AtomicMemAsmOperand' class
  case MCK_AtomicMemAsmOperand: {
    DiagnosticPredicate DP(Operand.isGPR());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'BareSymbol' class
  case MCK_BareSymbol: {
    DiagnosticPredicate DP(Operand.isBareSymbol());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidBareSymbol;
    break;
    }
  // 'Imm' class
  case MCK_Imm: {
    DiagnosticPredicate DP(Operand.isImm());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'SImm26OperandB' class
  case MCK_SImm26OperandB: {
    DiagnosticPredicate DP(Operand.isSImm26Operand());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm26Operand;
    break;
    }
  // 'SImm26OperandBL' class
  case MCK_SImm26OperandBL: {
    DiagnosticPredicate DP(Operand.isSImm26Operand());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm26Operand;
    break;
    }
  // 'Imm32' class
  case MCK_Imm32: {
    DiagnosticPredicate DP(Operand.isImm32());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidImm32;
    break;
    }
  // 'UImm2' class
  case MCK_UImm2: {
    DiagnosticPredicate DP(Operand.isUImm2());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm2;
    break;
    }
  // 'UImm2plus1' class
  case MCK_UImm2plus1: {
    DiagnosticPredicate DP(Operand.isUImm2plus1());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm2plus1;
    break;
    }
  // 'UImm3' class
  case MCK_UImm3: {
    DiagnosticPredicate DP(Operand.isUImm3());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm3;
    break;
    }
  // 'UImm5' class
  case MCK_UImm5: {
    DiagnosticPredicate DP(Operand.isUImm5());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm5;
    break;
    }
  // 'UImm6' class
  case MCK_UImm6: {
    DiagnosticPredicate DP(Operand.isUImm6());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm6;
    break;
    }
  // 'UImm8' class
  case MCK_UImm8: {
    DiagnosticPredicate DP(Operand.isUImm8());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm8;
    break;
    }
  // 'UImm12' class
  case MCK_UImm12: {
    DiagnosticPredicate DP(Operand.isUImm12());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm12;
    break;
    }
  // 'UImm12ori' class
  case MCK_UImm12ori: {
    DiagnosticPredicate DP(Operand.isUImm12ori());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm12ori;
    break;
    }
  // 'UImm14' class
  case MCK_UImm14: {
    DiagnosticPredicate DP(Operand.isUImm14());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm14;
    break;
    }
  // 'UImm15' class
  case MCK_UImm15: {
    DiagnosticPredicate DP(Operand.isUImm15());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidUImm15;
    break;
    }
  // 'SImm12' class
  case MCK_SImm12: {
    DiagnosticPredicate DP(Operand.isSImm12());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm12;
    break;
    }
  // 'SImm12addlike' class
  case MCK_SImm12addlike: {
    DiagnosticPredicate DP(Operand.isSImm12addlike());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm12addlike;
    break;
    }
  // 'SImm12lu52id' class
  case MCK_SImm12lu52id: {
    DiagnosticPredicate DP(Operand.isSImm12lu52id());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm12lu52id;
    break;
    }
  // 'SImm14lsl2' class
  case MCK_SImm14lsl2: {
    DiagnosticPredicate DP(Operand.isSImm14lsl2());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm14lsl2;
    break;
    }
  // 'SImm16' class
  case MCK_SImm16: {
    DiagnosticPredicate DP(Operand.isSImm16());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm16;
    break;
    }
  // 'SImm16lsl2' class
  case MCK_SImm16lsl2: {
    DiagnosticPredicate DP(Operand.isSImm16lsl2());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm16lsl2;
    break;
    }
  // 'SImm20' class
  case MCK_SImm20: {
    DiagnosticPredicate DP(Operand.isSImm20());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm20;
    break;
    }
  // 'SImm20pcalau12i' class
  case MCK_SImm20pcalau12i: {
    DiagnosticPredicate DP(Operand.isSImm20pcalau12i());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm20pcalau12i;
    break;
    }
  // 'SImm20lu12iw' class
  case MCK_SImm20lu12iw: {
    DiagnosticPredicate DP(Operand.isSImm20lu12iw());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm20lu12iw;
    break;
    }
  // 'SImm20lu32id' class
  case MCK_SImm20lu32id: {
    DiagnosticPredicate DP(Operand.isSImm20lu32id());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm20lu32id;
    break;
    }
  // 'SImm21lsl2' class
  case MCK_SImm21lsl2: {
    DiagnosticPredicate DP(Operand.isSImm21lsl2());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return LoongArchAsmParser::Match_InvalidSImm21lsl2;
    break;
    }
  } // end switch (Kind)

  if (Operand.isReg()) {
    MatchClassKind OpKind;
    switch (Operand.getReg()) {
    default: OpKind = InvalidMatchClass; break;
    case LoongArch::R0: OpKind = MCK_GPR; break;
    case LoongArch::R1: OpKind = MCK_GPR; break;
    case LoongArch::R2: OpKind = MCK_GPR; break;
    case LoongArch::R3: OpKind = MCK_GPR; break;
    case LoongArch::R4: OpKind = MCK_GPRT; break;
    case LoongArch::R5: OpKind = MCK_GPRT; break;
    case LoongArch::R6: OpKind = MCK_GPRT; break;
    case LoongArch::R7: OpKind = MCK_GPRT; break;
    case LoongArch::R8: OpKind = MCK_GPRT; break;
    case LoongArch::R9: OpKind = MCK_GPRT; break;
    case LoongArch::R10: OpKind = MCK_GPRT; break;
    case LoongArch::R11: OpKind = MCK_GPRT; break;
    case LoongArch::R12: OpKind = MCK_GPRT; break;
    case LoongArch::R13: OpKind = MCK_GPRT; break;
    case LoongArch::R14: OpKind = MCK_GPRT; break;
    case LoongArch::R15: OpKind = MCK_GPRT; break;
    case LoongArch::R16: OpKind = MCK_GPRT; break;
    case LoongArch::R17: OpKind = MCK_GPRT; break;
    case LoongArch::R18: OpKind = MCK_GPRT; break;
    case LoongArch::R19: OpKind = MCK_GPRT; break;
    case LoongArch::R20: OpKind = MCK_GPRT; break;
    case LoongArch::R21: OpKind = MCK_GPR; break;
    case LoongArch::R22: OpKind = MCK_GPR; break;
    case LoongArch::R23: OpKind = MCK_GPR; break;
    case LoongArch::R24: OpKind = MCK_GPR; break;
    case LoongArch::R25: OpKind = MCK_GPR; break;
    case LoongArch::R26: OpKind = MCK_GPR; break;
    case LoongArch::R27: OpKind = MCK_GPR; break;
    case LoongArch::R28: OpKind = MCK_GPR; break;
    case LoongArch::R29: OpKind = MCK_GPR; break;
    case LoongArch::R30: OpKind = MCK_GPR; break;
    case LoongArch::R31: OpKind = MCK_GPR; break;
    case LoongArch::F0: OpKind = MCK_FPR32; break;
    case LoongArch::F1: OpKind = MCK_FPR32; break;
    case LoongArch::F2: OpKind = MCK_FPR32; break;
    case LoongArch::F3: OpKind = MCK_FPR32; break;
    case LoongArch::F4: OpKind = MCK_FPR32; break;
    case LoongArch::F5: OpKind = MCK_FPR32; break;
    case LoongArch::F6: OpKind = MCK_FPR32; break;
    case LoongArch::F7: OpKind = MCK_FPR32; break;
    case LoongArch::F8: OpKind = MCK_FPR32; break;
    case LoongArch::F9: OpKind = MCK_FPR32; break;
    case LoongArch::F10: OpKind = MCK_FPR32; break;
    case LoongArch::F11: OpKind = MCK_FPR32; break;
    case LoongArch::F12: OpKind = MCK_FPR32; break;
    case LoongArch::F13: OpKind = MCK_FPR32; break;
    case LoongArch::F14: OpKind = MCK_FPR32; break;
    case LoongArch::F15: OpKind = MCK_FPR32; break;
    case LoongArch::F16: OpKind = MCK_FPR32; break;
    case LoongArch::F17: OpKind = MCK_FPR32; break;
    case LoongArch::F18: OpKind = MCK_FPR32; break;
    case LoongArch::F19: OpKind = MCK_FPR32; break;
    case LoongArch::F20: OpKind = MCK_FPR32; break;
    case LoongArch::F21: OpKind = MCK_FPR32; break;
    case LoongArch::F22: OpKind = MCK_FPR32; break;
    case LoongArch::F23: OpKind = MCK_FPR32; break;
    case LoongArch::F24: OpKind = MCK_FPR32; break;
    case LoongArch::F25: OpKind = MCK_FPR32; break;
    case LoongArch::F26: OpKind = MCK_FPR32; break;
    case LoongArch::F27: OpKind = MCK_FPR32; break;
    case LoongArch::F28: OpKind = MCK_FPR32; break;
    case LoongArch::F29: OpKind = MCK_FPR32; break;
    case LoongArch::F30: OpKind = MCK_FPR32; break;
    case LoongArch::F31: OpKind = MCK_FPR32; break;
    case LoongArch::F0_64: OpKind = MCK_FPR64; break;
    case LoongArch::F1_64: OpKind = MCK_FPR64; break;
    case LoongArch::F2_64: OpKind = MCK_FPR64; break;
    case LoongArch::F3_64: OpKind = MCK_FPR64; break;
    case LoongArch::F4_64: OpKind = MCK_FPR64; break;
    case LoongArch::F5_64: OpKind = MCK_FPR64; break;
    case LoongArch::F6_64: OpKind = MCK_FPR64; break;
    case LoongArch::F7_64: OpKind = MCK_FPR64; break;
    case LoongArch::F8_64: OpKind = MCK_FPR64; break;
    case LoongArch::F9_64: OpKind = MCK_FPR64; break;
    case LoongArch::F10_64: OpKind = MCK_FPR64; break;
    case LoongArch::F11_64: OpKind = MCK_FPR64; break;
    case LoongArch::F12_64: OpKind = MCK_FPR64; break;
    case LoongArch::F13_64: OpKind = MCK_FPR64; break;
    case LoongArch::F14_64: OpKind = MCK_FPR64; break;
    case LoongArch::F15_64: OpKind = MCK_FPR64; break;
    case LoongArch::F16_64: OpKind = MCK_FPR64; break;
    case LoongArch::F17_64: OpKind = MCK_FPR64; break;
    case LoongArch::F18_64: OpKind = MCK_FPR64; break;
    case LoongArch::F19_64: OpKind = MCK_FPR64; break;
    case LoongArch::F20_64: OpKind = MCK_FPR64; break;
    case LoongArch::F21_64: OpKind = MCK_FPR64; break;
    case LoongArch::F22_64: OpKind = MCK_FPR64; break;
    case LoongArch::F23_64: OpKind = MCK_FPR64; break;
    case LoongArch::F24_64: OpKind = MCK_FPR64; break;
    case LoongArch::F25_64: OpKind = MCK_FPR64; break;
    case LoongArch::F26_64: OpKind = MCK_FPR64; break;
    case LoongArch::F27_64: OpKind = MCK_FPR64; break;
    case LoongArch::F28_64: OpKind = MCK_FPR64; break;
    case LoongArch::F29_64: OpKind = MCK_FPR64; break;
    case LoongArch::F30_64: OpKind = MCK_FPR64; break;
    case LoongArch::F31_64: OpKind = MCK_FPR64; break;
    case LoongArch::FCC0: OpKind = MCK_CFR; break;
    case LoongArch::FCC1: OpKind = MCK_CFR; break;
    case LoongArch::FCC2: OpKind = MCK_CFR; break;
    case LoongArch::FCC3: OpKind = MCK_CFR; break;
    case LoongArch::FCC4: OpKind = MCK_CFR; break;
    case LoongArch::FCC5: OpKind = MCK_CFR; break;
    case LoongArch::FCC6: OpKind = MCK_CFR; break;
    case LoongArch::FCC7: OpKind = MCK_CFR; break;
    case LoongArch::FCSR0: OpKind = MCK_FCSR; break;
    case LoongArch::FCSR1: OpKind = MCK_FCSR; break;
    case LoongArch::FCSR2: OpKind = MCK_FCSR; break;
    case LoongArch::FCSR3: OpKind = MCK_FCSR; break;
    }
    return isSubclass(OpKind, Kind) ? (unsigned)MCTargetAsmParser::Match_Success :
                                      getDiagKindFromRegisterClass(Kind);
  }

  if (Kind > MCK_LAST_TOKEN && Kind <= MCK_LAST_REGISTER)
    return getDiagKindFromRegisterClass(Kind);

  return MCTargetAsmParser::Match_InvalidOperand;
}

#ifndef NDEBUG
const char *getMatchClassName(MatchClassKind Kind) {
  switch (Kind) {
  case InvalidMatchClass: return "InvalidMatchClass";
  case OptionalMatchClass: return "OptionalMatchClass";
  case MCK_FCSR: return "MCK_FCSR";
  case MCK_CFR: return "MCK_CFR";
  case MCK_GPRT: return "MCK_GPRT";
  case MCK_FPR32: return "MCK_FPR32";
  case MCK_FPR64: return "MCK_FPR64";
  case MCK_GPR: return "MCK_GPR";
  case MCK_AtomicMemAsmOperand: return "MCK_AtomicMemAsmOperand";
  case MCK_BareSymbol: return "MCK_BareSymbol";
  case MCK_Imm: return "MCK_Imm";
  case MCK_SImm26OperandB: return "MCK_SImm26OperandB";
  case MCK_SImm26OperandBL: return "MCK_SImm26OperandBL";
  case MCK_Imm32: return "MCK_Imm32";
  case MCK_UImm2: return "MCK_UImm2";
  case MCK_UImm2plus1: return "MCK_UImm2plus1";
  case MCK_UImm3: return "MCK_UImm3";
  case MCK_UImm5: return "MCK_UImm5";
  case MCK_UImm6: return "MCK_UImm6";
  case MCK_UImm8: return "MCK_UImm8";
  case MCK_UImm12: return "MCK_UImm12";
  case MCK_UImm12ori: return "MCK_UImm12ori";
  case MCK_UImm14: return "MCK_UImm14";
  case MCK_UImm15: return "MCK_UImm15";
  case MCK_SImm12: return "MCK_SImm12";
  case MCK_SImm12addlike: return "MCK_SImm12addlike";
  case MCK_SImm12lu52id: return "MCK_SImm12lu52id";
  case MCK_SImm14lsl2: return "MCK_SImm14lsl2";
  case MCK_SImm16: return "MCK_SImm16";
  case MCK_SImm16lsl2: return "MCK_SImm16lsl2";
  case MCK_SImm20: return "MCK_SImm20";
  case MCK_SImm20pcalau12i: return "MCK_SImm20pcalau12i";
  case MCK_SImm20lu12iw: return "MCK_SImm20lu12iw";
  case MCK_SImm20lu32id: return "MCK_SImm20lu32id";
  case MCK_SImm21lsl2: return "MCK_SImm21lsl2";
  case NumMatchClassKinds: return "NumMatchClassKinds";
  }
  llvm_unreachable("unhandled MatchClassKind!");
}

#endif // NDEBUG
FeatureBitset LoongArchAsmParser::
ComputeAvailableFeatures(const FeatureBitset &FB) const {
  FeatureBitset Features;
  if (FB[LoongArch::Feature64Bit])
    Features.set(Feature_IsLA64Bit);
  if (!FB[LoongArch::Feature64Bit])
    Features.set(Feature_IsLA32Bit);
  if (FB[LoongArch::FeatureBasicF])
    Features.set(Feature_HasBasicFBit);
  if (FB[LoongArch::FeatureBasicD])
    Features.set(Feature_HasBasicDBit);
  if (FB[LoongArch::FeatureExtLSX])
    Features.set(Feature_HasExtLSXBit);
  if (FB[LoongArch::FeatureExtLASX])
    Features.set(Feature_HasExtLASXBit);
  if (FB[LoongArch::FeatureExtLVZ])
    Features.set(Feature_HasExtLVZBit);
  if (FB[LoongArch::FeatureExtLBT])
    Features.set(Feature_HasExtLBTBit);
  if (FB[LoongArch::LaGlobalWithPcrel])
    Features.set(Feature_HasLaGlobalWithPcrelBit);
  if (FB[LoongArch::LaGlobalWithAbs])
    Features.set(Feature_HasLaGlobalWithAbsBit);
  if (FB[LoongArch::LaLocalWithAbs])
    Features.set(Feature_HasLaLocalWithAbsBit);
  return Features;
}

static bool checkAsmTiedOperandConstraints(const LoongArchAsmParser&AsmParser,
                               unsigned Kind,
                               const OperandVector &Operands,
                               uint64_t &ErrorInfo) {
  assert(Kind < CVT_NUM_SIGNATURES && "Invalid signature!");
  const uint8_t *Converter = ConversionTable[Kind];
  for (const uint8_t *p = Converter; *p; p += 2) {
    switch (*p) {
    case CVT_Tied: {
      unsigned OpIdx = *(p + 1);
      assert(OpIdx < (size_t)(std::end(TiedAsmOperandTable) -
                              std::begin(TiedAsmOperandTable)) &&
             "Tied operand not found");
      unsigned OpndNum1 = TiedAsmOperandTable[OpIdx][1];
      unsigned OpndNum2 = TiedAsmOperandTable[OpIdx][2];
      if (OpndNum1 != OpndNum2) {
        auto &SrcOp1 = Operands[OpndNum1];
        auto &SrcOp2 = Operands[OpndNum2];
        if (!AsmParser.areEqualRegs(*SrcOp1, *SrcOp2)) {
          ErrorInfo = OpndNum2;
          return false;
        }
      }
      break;
    }
    default:
      break;
    }
  }
  return true;
}

static const char MnemonicTable[] =
    "\005add.d\005add.w\006addi.d\006addi.w\taddu16i.d\006alsl.d\006alsl.w\007"
    "alsl.wu\007amadd.d\007amadd.w\namadd_db.d\namadd_db.w\007amand.d\007ama"
    "nd.w\namand_db.d\namand_db.w\007ammax.d\010ammax.du\007ammax.w\010ammax"
    ".wu\nammax_db.d\013ammax_db.du\nammax_db.w\013ammax_db.wu\007ammin.d\010"
    "ammin.du\007ammin.w\010ammin.wu\nammin_db.d\013ammin_db.du\nammin_db.w\013"
    "ammin_db.wu\006amor.d\006amor.w\tamor_db.d\tamor_db.w\010amswap.d\010am"
    "swap.w\013amswap_db.d\013amswap_db.w\007amxor.d\007amxor.w\namxor_db.d\n"
    "amxor_db.w\003and\004andi\004andn\010asrtgt.d\010asrtle.d\001b\005bceqz"
    "\005bcnez\003beq\004beqz\003bge\004bgeu\004bgez\003bgt\004bgtu\004bgtz\t"
    "bitrev.4b\tbitrev.8b\010bitrev.d\010bitrev.w\002bl\003ble\004bleu\004bl"
    "ez\003blt\004bltu\004bltz\003bne\004bnez\005break\tbstrins.d\tbstrins.w"
    "\nbstrpick.d\nbstrpick.w\nbytepick.d\nbytepick.w\005cacop\005clo.d\005c"
    "lo.w\005clz.d\005clz.w\006cpucfg\tcrc.w.b.w\tcrc.w.d.w\tcrc.w.h.w\tcrc."
    "w.w.w\ncrcc.w.b.w\ncrcc.w.d.w\ncrcc.w.h.w\ncrcc.w.w.w\005csrrd\005csrwr"
    "\007csrxchg\005cto.d\005cto.w\005ctz.d\005ctz.w\004dbar\004dbcl\005div."
    "d\006div.du\005div.w\006div.wu\004ertn\007ext.w.b\007ext.w.h\006fabs.d\006"
    "fabs.s\006fadd.d\006fadd.s\010fclass.d\010fclass.s\nfcmp.caf.d\nfcmp.ca"
    "f.s\nfcmp.ceq.d\nfcmp.ceq.s\nfcmp.cle.d\nfcmp.cle.s\nfcmp.clt.d\nfcmp.c"
    "lt.s\nfcmp.cne.d\nfcmp.cne.s\nfcmp.cor.d\nfcmp.cor.s\013fcmp.cueq.d\013"
    "fcmp.cueq.s\013fcmp.cule.d\013fcmp.cule.s\013fcmp.cult.d\013fcmp.cult.s"
    "\nfcmp.cun.d\nfcmp.cun.s\013fcmp.cune.d\013fcmp.cune.s\nfcmp.saf.d\nfcm"
    "p.saf.s\nfcmp.seq.d\nfcmp.seq.s\nfcmp.sle.d\nfcmp.sle.s\nfcmp.slt.d\nfc"
    "mp.slt.s\nfcmp.sne.d\nfcmp.sne.s\nfcmp.sor.d\nfcmp.sor.s\013fcmp.sueq.d"
    "\013fcmp.sueq.s\013fcmp.sule.d\013fcmp.sule.s\013fcmp.sult.d\013fcmp.su"
    "lt.s\nfcmp.sun.d\nfcmp.sun.s\013fcmp.sune.d\013fcmp.sune.s\013fcopysign"
    ".d\013fcopysign.s\010fcvt.d.s\010fcvt.s.d\006fdiv.d\006fdiv.s\tffint.d."
    "l\tffint.d.w\tffint.s.l\tffint.s.w\005fld.d\005fld.s\007fldgt.d\007fldg"
    "t.s\007fldle.d\007fldle.s\006fldx.d\006fldx.s\007flogb.d\007flogb.s\007"
    "fmadd.d\007fmadd.s\006fmax.d\006fmax.s\007fmaxa.d\007fmaxa.s\006fmin.d\006"
    "fmin.s\007fmina.d\007fmina.s\006fmov.d\006fmov.s\007fmsub.d\007fmsub.s\006"
    "fmul.d\006fmul.s\006fneg.d\006fneg.s\010fnmadd.d\010fnmadd.s\010fnmsub."
    "d\010fnmsub.s\010frecip.d\010frecip.s\007frint.d\007frint.s\010frsqrt.d"
    "\010frsqrt.s\tfscaleb.d\tfscaleb.s\004fsel\007fsqrt.d\007fsqrt.s\005fst"
    ".d\005fst.s\007fstgt.d\007fstgt.s\007fstle.d\007fstle.s\006fstx.d\006fs"
    "tx.s\006fsub.d\006fsub.s\tftint.l.d\tftint.l.s\tftint.w.d\tftint.w.s\013"
    "ftintrm.l.d\013ftintrm.l.s\013ftintrm.w.d\013ftintrm.w.s\014ftintrne.l."
    "d\014ftintrne.l.s\014ftintrne.w.d\014ftintrne.w.s\013ftintrp.l.d\013fti"
    "ntrp.l.s\013ftintrp.w.d\013ftintrp.w.s\013ftintrz.l.d\013ftintrz.l.s\013"
    "ftintrz.w.d\013ftintrz.w.s\004ibar\004idle\006invtlb\tiocsrrd.b\tiocsrr"
    "d.d\tiocsrrd.h\tiocsrrd.w\tiocsrwr.b\tiocsrwr.d\tiocsrwr.h\tiocsrwr.w\004"
    "jirl\002jr\002la\006la.abs\tla.global\006la.got\010la.local\010la.pcrel"
    "\tla.tls.gd\tla.tls.ie\tla.tls.ld\tla.tls.le\004ld.b\005ld.bu\004ld.d\004"
    "ld.h\005ld.hu\004ld.w\005ld.wu\005lddir\006ldgt.b\006ldgt.d\006ldgt.h\006"
    "ldgt.w\006ldle.b\006ldle.d\006ldle.h\006ldle.w\005ldpte\007ldptr.d\007l"
    "dptr.w\005ldx.b\006ldx.bu\005ldx.d\005ldx.h\006ldx.hu\005ldx.w\006ldx.w"
    "u\004li.d\004li.w\004ll.d\004ll.w\007lu12i.w\007lu32i.d\007lu52i.d\007m"
    "askeqz\007masknez\005mod.d\006mod.du\005mod.w\006mod.wu\010movcf2fr\010"
    "movcf2gr\004move\nmovfcsr2gr\010movfr2cf\nmovfr2gr.d\nmovfr2gr.s\013mov"
    "frh2gr.s\010movgr2cf\nmovgr2fcsr\nmovgr2fr.d\nmovgr2fr.w\013movgr2frh.w"
    "\005mul.d\005mul.w\006mulh.d\007mulh.du\006mulh.w\007mulh.wu\010mulw.d."
    "w\tmulw.d.wu\003nop\003nor\002or\003ori\003orn\006pcaddi\tpcaddu12i\tpc"
    "addu18i\tpcalau12i\005preld\006preldx\010rdtime.d\trdtimeh.w\trdtimel.w"
    "\003ret\007revb.2h\007revb.2w\007revb.4h\006revb.d\007revh.2w\006revh.d"
    "\006rotr.d\006rotr.w\007rotri.d\007rotri.w\004sc.d\004sc.w\005sll.d\005"
    "sll.w\006slli.d\006slli.w\003slt\004slti\004sltu\005sltui\005sra.d\005s"
    "ra.w\006srai.d\006srai.w\005srl.d\005srl.w\006srli.d\006srli.w\004st.b\004"
    "st.d\004st.h\004st.w\006stgt.b\006stgt.d\006stgt.h\006stgt.w\006stle.b\006"
    "stle.d\006stle.h\006stle.w\007stptr.d\007stptr.w\005stx.b\005stx.d\005s"
    "tx.h\005stx.w\005sub.d\005sub.w\007syscall\006tlbclr\007tlbfill\010tlbf"
    "lush\005tlbrd\007tlbsrch\005tlbwr\003xor\004xori";

// Feature bitsets.
enum : uint8_t {
  AMFBS_None,
  AMFBS_HasBasicD,
  AMFBS_HasBasicF,
  AMFBS_HasLaGlobalWithAbs,
  AMFBS_HasLaGlobalWithPcrel,
  AMFBS_HasLaLocalWithAbs,
  AMFBS_IsLA64,
  AMFBS_HasBasicD_IsLA64,
};

static constexpr FeatureBitset FeatureBitsets[] = {
  {}, // AMFBS_None
  {Feature_HasBasicDBit, },
  {Feature_HasBasicFBit, },
  {Feature_HasLaGlobalWithAbsBit, },
  {Feature_HasLaGlobalWithPcrelBit, },
  {Feature_HasLaLocalWithAbsBit, },
  {Feature_IsLA64Bit, },
  {Feature_HasBasicDBit, Feature_IsLA64Bit, },
};

namespace {
  struct MatchEntry {
    uint16_t Mnemonic;
    uint16_t Opcode;
    uint8_t ConvertFn;
    uint8_t RequiredFeaturesIdx;
    uint8_t Classes[4];
    StringRef getMnemonic() const {
      return StringRef(MnemonicTable + Mnemonic + 1,
                       MnemonicTable[Mnemonic]);
    }
  };

  // Predicate for searching for an opcode.
  struct LessOpcode {
    bool operator()(const MatchEntry &LHS, StringRef RHS) {
      return LHS.getMnemonic() < RHS;
    }
    bool operator()(StringRef LHS, const MatchEntry &RHS) {
      return LHS < RHS.getMnemonic();
    }
    bool operator()(const MatchEntry &LHS, const MatchEntry &RHS) {
      return LHS.getMnemonic() < RHS.getMnemonic();
    }
  };
} // end anonymous namespace

static const MatchEntry MatchTable0[] = {
  { 0 /* add.d */, LoongArch::ADD_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 6 /* add.w */, LoongArch::ADD_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 12 /* addi.d */, LoongArch::ADDI_D, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 19 /* addi.w */, LoongArch::ADDI_W, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 26 /* addu16i.d */, LoongArch::ADDU16I_D, Convert__Reg1_0__Reg1_1__SImm161_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm16 }, },
  { 36 /* alsl.d */, LoongArch::ALSL_D, Convert__Reg1_0__Reg1_1__Reg1_2__UImm2plus11_3, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2plus1 }, },
  { 43 /* alsl.w */, LoongArch::ALSL_W, Convert__Reg1_0__Reg1_1__Reg1_2__UImm2plus11_3, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2plus1 }, },
  { 50 /* alsl.wu */, LoongArch::ALSL_WU, Convert__Reg1_0__Reg1_1__Reg1_2__UImm2plus11_3, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2plus1 }, },
  { 58 /* amadd.d */, LoongArch::AMADD_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 66 /* amadd.w */, LoongArch::AMADD_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 74 /* amadd_db.d */, LoongArch::AMADD_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 85 /* amadd_db.w */, LoongArch::AMADD_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 96 /* amand.d */, LoongArch::AMAND_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 104 /* amand.w */, LoongArch::AMAND_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 112 /* amand_db.d */, LoongArch::AMAND_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 123 /* amand_db.w */, LoongArch::AMAND_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 134 /* ammax.d */, LoongArch::AMMAX_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 142 /* ammax.du */, LoongArch::AMMAX_DU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 151 /* ammax.w */, LoongArch::AMMAX_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 159 /* ammax.wu */, LoongArch::AMMAX_WU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 168 /* ammax_db.d */, LoongArch::AMMAX_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 179 /* ammax_db.du */, LoongArch::AMMAX_DB_DU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 191 /* ammax_db.w */, LoongArch::AMMAX_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 202 /* ammax_db.wu */, LoongArch::AMMAX_DB_WU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 214 /* ammin.d */, LoongArch::AMMIN_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 222 /* ammin.du */, LoongArch::AMMIN_DU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 231 /* ammin.w */, LoongArch::AMMIN_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 239 /* ammin.wu */, LoongArch::AMMIN_WU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 248 /* ammin_db.d */, LoongArch::AMMIN_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 259 /* ammin_db.du */, LoongArch::AMMIN_DB_DU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 271 /* ammin_db.w */, LoongArch::AMMIN_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 282 /* ammin_db.wu */, LoongArch::AMMIN_DB_WU, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 294 /* amor.d */, LoongArch::AMOR_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 301 /* amor.w */, LoongArch::AMOR_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 308 /* amor_db.d */, LoongArch::AMOR_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 318 /* amor_db.w */, LoongArch::AMOR_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 328 /* amswap.d */, LoongArch::AMSWAP_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 337 /* amswap.w */, LoongArch::AMSWAP_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 346 /* amswap_db.d */, LoongArch::AMSWAP_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 358 /* amswap_db.w */, LoongArch::AMSWAP_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 370 /* amxor.d */, LoongArch::AMXOR_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 378 /* amxor.w */, LoongArch::AMXOR_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 386 /* amxor_db.d */, LoongArch::AMXOR_DB_D, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 397 /* amxor_db.w */, LoongArch::AMXOR_DB_W, Convert__Reg1_0__Reg1_1__AtomicMemAsmOperand1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_AtomicMemAsmOperand }, },
  { 408 /* and */, LoongArch::AND, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 412 /* andi */, LoongArch::ANDI, Convert__Reg1_0__Reg1_1__UImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm12 }, },
  { 417 /* andn */, LoongArch::ANDN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 422 /* asrtgt.d */, LoongArch::ASRTGT_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 431 /* asrtle.d */, LoongArch::ASRTLE_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 440 /* b */, LoongArch::B, Convert__SImm26OperandB1_0, AMFBS_None, { MCK_SImm26OperandB }, },
  { 442 /* bceqz */, LoongArch::BCEQZ, Convert__Reg1_0__SImm21lsl21_1, AMFBS_HasBasicF, { MCK_CFR, MCK_SImm21lsl2 }, },
  { 448 /* bcnez */, LoongArch::BCNEZ, Convert__Reg1_0__SImm21lsl21_1, AMFBS_HasBasicF, { MCK_CFR, MCK_SImm21lsl2 }, },
  { 454 /* beq */, LoongArch::BEQ, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 458 /* beqz */, LoongArch::BEQZ, Convert__Reg1_0__SImm21lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm21lsl2 }, },
  { 463 /* bge */, LoongArch::BGE, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 467 /* bgeu */, LoongArch::BGEU, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 472 /* bgez */, LoongArch::BGE, Convert__Reg1_0__regR0__SImm16lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm16lsl2 }, },
  { 477 /* bgt */, LoongArch::BLT, Convert__Reg1_1__Reg1_0__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 481 /* bgtu */, LoongArch::BLTU, Convert__Reg1_1__Reg1_0__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 486 /* bgtz */, LoongArch::BLT, Convert__regR0__Reg1_0__SImm16lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm16lsl2 }, },
  { 491 /* bitrev.4b */, LoongArch::BITREV_4B, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 501 /* bitrev.8b */, LoongArch::BITREV_8B, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 511 /* bitrev.d */, LoongArch::BITREV_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 520 /* bitrev.w */, LoongArch::BITREV_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 529 /* bl */, LoongArch::BL, Convert__SImm26OperandBL1_0, AMFBS_None, { MCK_SImm26OperandBL }, },
  { 532 /* ble */, LoongArch::BGE, Convert__Reg1_1__Reg1_0__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 536 /* bleu */, LoongArch::BGEU, Convert__Reg1_1__Reg1_0__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 541 /* blez */, LoongArch::BGE, Convert__regR0__Reg1_0__SImm16lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm16lsl2 }, },
  { 546 /* blt */, LoongArch::BLT, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 550 /* bltu */, LoongArch::BLTU, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 555 /* bltz */, LoongArch::BLT, Convert__Reg1_0__regR0__SImm16lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm16lsl2 }, },
  { 560 /* bne */, LoongArch::BNE, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 564 /* bnez */, LoongArch::BNEZ, Convert__Reg1_0__SImm21lsl21_1, AMFBS_None, { MCK_GPR, MCK_SImm21lsl2 }, },
  { 569 /* break */, LoongArch::BREAK, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 575 /* bstrins.d */, LoongArch::BSTRINS_D, Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm61_2__UImm61_3, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6, MCK_UImm6 }, },
  { 585 /* bstrins.w */, LoongArch::BSTRINS_W, Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm51_2__UImm51_3, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5, MCK_UImm5 }, },
  { 595 /* bstrpick.d */, LoongArch::BSTRPICK_D, Convert__Reg1_0__Reg1_1__UImm61_2__UImm61_3, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6, MCK_UImm6 }, },
  { 606 /* bstrpick.w */, LoongArch::BSTRPICK_W, Convert__Reg1_0__Reg1_1__UImm51_2__UImm51_3, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5, MCK_UImm5 }, },
  { 617 /* bytepick.d */, LoongArch::BYTEPICK_D, Convert__Reg1_0__Reg1_1__Reg1_2__UImm31_3, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm3 }, },
  { 628 /* bytepick.w */, LoongArch::BYTEPICK_W, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 639 /* cacop */, LoongArch::CACOP, Convert__UImm51_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_UImm5, MCK_GPR, MCK_SImm12 }, },
  { 645 /* clo.d */, LoongArch::CLO_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 651 /* clo.w */, LoongArch::CLO_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 657 /* clz.d */, LoongArch::CLZ_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 663 /* clz.w */, LoongArch::CLZ_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 669 /* cpucfg */, LoongArch::CPUCFG, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 676 /* crc.w.b.w */, LoongArch::CRC_W_B_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 686 /* crc.w.d.w */, LoongArch::CRC_W_D_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 696 /* crc.w.h.w */, LoongArch::CRC_W_H_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 706 /* crc.w.w.w */, LoongArch::CRC_W_W_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 716 /* crcc.w.b.w */, LoongArch::CRCC_W_B_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 727 /* crcc.w.d.w */, LoongArch::CRCC_W_D_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 738 /* crcc.w.h.w */, LoongArch::CRCC_W_H_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 749 /* crcc.w.w.w */, LoongArch::CRCC_W_W_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 760 /* csrrd */, LoongArch::CSRRD, Convert__Reg1_0__UImm141_1, AMFBS_None, { MCK_GPR, MCK_UImm14 }, },
  { 766 /* csrwr */, LoongArch::CSRWR, Convert__Reg1_0__Tie0_1_1__UImm141_1, AMFBS_None, { MCK_GPR, MCK_UImm14 }, },
  { 772 /* csrxchg */, LoongArch::CSRXCHG, Convert__Reg1_0__Tie0_1_1__Reg1_1__UImm141_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm14 }, },
  { 780 /* cto.d */, LoongArch::CTO_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 786 /* cto.w */, LoongArch::CTO_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 792 /* ctz.d */, LoongArch::CTZ_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 798 /* ctz.w */, LoongArch::CTZ_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 804 /* dbar */, LoongArch::DBAR, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 809 /* dbcl */, LoongArch::DBCL, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 814 /* div.d */, LoongArch::DIV_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 820 /* div.du */, LoongArch::DIV_DU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 827 /* div.w */, LoongArch::DIV_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 833 /* div.wu */, LoongArch::DIV_WU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 840 /* ertn */, LoongArch::ERTN, Convert_NoOperands, AMFBS_None, {  }, },
  { 845 /* ext.w.b */, LoongArch::EXT_W_B, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 853 /* ext.w.h */, LoongArch::EXT_W_H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 861 /* fabs.d */, LoongArch::FABS_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 868 /* fabs.s */, LoongArch::FABS_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 875 /* fadd.d */, LoongArch::FADD_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 882 /* fadd.s */, LoongArch::FADD_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 889 /* fclass.d */, LoongArch::FCLASS_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 898 /* fclass.s */, LoongArch::FCLASS_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 907 /* fcmp.caf.d */, LoongArch::FCMP_CAF_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 918 /* fcmp.caf.s */, LoongArch::FCMP_CAF_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 929 /* fcmp.ceq.d */, LoongArch::FCMP_CEQ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 940 /* fcmp.ceq.s */, LoongArch::FCMP_CEQ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 951 /* fcmp.cle.d */, LoongArch::FCMP_CLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 962 /* fcmp.cle.s */, LoongArch::FCMP_CLE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 973 /* fcmp.clt.d */, LoongArch::FCMP_CLT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 984 /* fcmp.clt.s */, LoongArch::FCMP_CLT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 995 /* fcmp.cne.d */, LoongArch::FCMP_CNE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1006 /* fcmp.cne.s */, LoongArch::FCMP_CNE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1017 /* fcmp.cor.d */, LoongArch::FCMP_COR_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1028 /* fcmp.cor.s */, LoongArch::FCMP_COR_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1039 /* fcmp.cueq.d */, LoongArch::FCMP_CUEQ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1051 /* fcmp.cueq.s */, LoongArch::FCMP_CUEQ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1063 /* fcmp.cule.d */, LoongArch::FCMP_CULE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1075 /* fcmp.cule.s */, LoongArch::FCMP_CULE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1087 /* fcmp.cult.d */, LoongArch::FCMP_CULT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1099 /* fcmp.cult.s */, LoongArch::FCMP_CULT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1111 /* fcmp.cun.d */, LoongArch::FCMP_CUN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1122 /* fcmp.cun.s */, LoongArch::FCMP_CUN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1133 /* fcmp.cune.d */, LoongArch::FCMP_CUNE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1145 /* fcmp.cune.s */, LoongArch::FCMP_CUNE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1157 /* fcmp.saf.d */, LoongArch::FCMP_SAF_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1168 /* fcmp.saf.s */, LoongArch::FCMP_SAF_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1179 /* fcmp.seq.d */, LoongArch::FCMP_SEQ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1190 /* fcmp.seq.s */, LoongArch::FCMP_SEQ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1201 /* fcmp.sle.d */, LoongArch::FCMP_SLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1212 /* fcmp.sle.s */, LoongArch::FCMP_SLE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1223 /* fcmp.slt.d */, LoongArch::FCMP_SLT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1234 /* fcmp.slt.s */, LoongArch::FCMP_SLT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1245 /* fcmp.sne.d */, LoongArch::FCMP_SNE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1256 /* fcmp.sne.s */, LoongArch::FCMP_SNE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1267 /* fcmp.sor.d */, LoongArch::FCMP_SOR_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1278 /* fcmp.sor.s */, LoongArch::FCMP_SOR_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1289 /* fcmp.sueq.d */, LoongArch::FCMP_SUEQ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1301 /* fcmp.sueq.s */, LoongArch::FCMP_SUEQ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1313 /* fcmp.sule.d */, LoongArch::FCMP_SULE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1325 /* fcmp.sule.s */, LoongArch::FCMP_SULE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1337 /* fcmp.sult.d */, LoongArch::FCMP_SULT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1349 /* fcmp.sult.s */, LoongArch::FCMP_SULT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1361 /* fcmp.sun.d */, LoongArch::FCMP_SUN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1372 /* fcmp.sun.s */, LoongArch::FCMP_SUN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1383 /* fcmp.sune.d */, LoongArch::FCMP_SUNE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_CFR, MCK_FPR64, MCK_FPR64 }, },
  { 1395 /* fcmp.sune.s */, LoongArch::FCMP_SUNE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32, MCK_FPR32 }, },
  { 1407 /* fcopysign.d */, LoongArch::FCOPYSIGN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1419 /* fcopysign.s */, LoongArch::FCOPYSIGN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1431 /* fcvt.d.s */, LoongArch::FCVT_D_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 1440 /* fcvt.s.d */, LoongArch::FCVT_S_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 1449 /* fdiv.d */, LoongArch::FDIV_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1456 /* fdiv.s */, LoongArch::FDIV_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1463 /* ffint.d.l */, LoongArch::FFINT_D_L, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1473 /* ffint.d.w */, LoongArch::FFINT_D_W, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 1483 /* ffint.s.l */, LoongArch::FFINT_S_L, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 1493 /* ffint.s.w */, LoongArch::FFINT_S_W, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1503 /* fld.d */, LoongArch::FLD_D, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_SImm12 }, },
  { 1509 /* fld.s */, LoongArch::FLD_S, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_SImm12 }, },
  { 1515 /* fldgt.d */, LoongArch::FLDGT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1523 /* fldgt.s */, LoongArch::FLDGT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1531 /* fldle.d */, LoongArch::FLDLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1539 /* fldle.s */, LoongArch::FLDLE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1547 /* fldx.d */, LoongArch::FLDX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1554 /* fldx.s */, LoongArch::FLDX_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1561 /* flogb.d */, LoongArch::FLOGB_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1569 /* flogb.s */, LoongArch::FLOGB_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1577 /* fmadd.d */, LoongArch::FMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1585 /* fmadd.s */, LoongArch::FMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1593 /* fmax.d */, LoongArch::FMAX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1600 /* fmax.s */, LoongArch::FMAX_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1607 /* fmaxa.d */, LoongArch::FMAXA_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1615 /* fmaxa.s */, LoongArch::FMAXA_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1623 /* fmin.d */, LoongArch::FMIN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1630 /* fmin.s */, LoongArch::FMIN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1637 /* fmina.d */, LoongArch::FMINA_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1645 /* fmina.s */, LoongArch::FMINA_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1653 /* fmov.d */, LoongArch::FMOV_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1660 /* fmov.s */, LoongArch::FMOV_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1667 /* fmsub.d */, LoongArch::FMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1675 /* fmsub.s */, LoongArch::FMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1683 /* fmul.d */, LoongArch::FMUL_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1690 /* fmul.s */, LoongArch::FMUL_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1697 /* fneg.d */, LoongArch::FNEG_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1704 /* fneg.s */, LoongArch::FNEG_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1711 /* fnmadd.d */, LoongArch::FNMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1720 /* fnmadd.s */, LoongArch::FNMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1729 /* fnmsub.d */, LoongArch::FNMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1738 /* fnmsub.s */, LoongArch::FNMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1747 /* frecip.d */, LoongArch::FRECIP_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1756 /* frecip.s */, LoongArch::FRECIP_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1765 /* frint.d */, LoongArch::FRINT_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1773 /* frint.s */, LoongArch::FRINT_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1781 /* frsqrt.d */, LoongArch::FRSQRT_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1790 /* frsqrt.s */, LoongArch::FRSQRT_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1799 /* fscaleb.d */, LoongArch::FSCALEB_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1809 /* fscaleb.s */, LoongArch::FSCALEB_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1819 /* fsel */, LoongArch::FSEL_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_CFR }, },
  { 1824 /* fsqrt.d */, LoongArch::FSQRT_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1832 /* fsqrt.s */, LoongArch::FSQRT_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1840 /* fst.d */, LoongArch::FST_D, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_SImm12 }, },
  { 1846 /* fst.s */, LoongArch::FST_S, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_SImm12 }, },
  { 1852 /* fstgt.d */, LoongArch::FSTGT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1860 /* fstgt.s */, LoongArch::FSTGT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1868 /* fstle.d */, LoongArch::FSTLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1876 /* fstle.s */, LoongArch::FSTLE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1884 /* fstx.d */, LoongArch::FSTX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR, MCK_GPR }, },
  { 1891 /* fstx.s */, LoongArch::FSTX_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR, MCK_GPR }, },
  { 1898 /* fsub.d */, LoongArch::FSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1905 /* fsub.s */, LoongArch::FSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1912 /* ftint.l.d */, LoongArch::FTINT_L_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1922 /* ftint.l.s */, LoongArch::FTINT_L_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 1932 /* ftint.w.d */, LoongArch::FTINT_W_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 1942 /* ftint.w.s */, LoongArch::FTINT_W_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 1952 /* ftintrm.l.d */, LoongArch::FTINTRM_L_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 1964 /* ftintrm.l.s */, LoongArch::FTINTRM_L_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 1976 /* ftintrm.w.d */, LoongArch::FTINTRM_W_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 1988 /* ftintrm.w.s */, LoongArch::FTINTRM_W_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 2000 /* ftintrne.l.d */, LoongArch::FTINTRNE_L_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 2013 /* ftintrne.l.s */, LoongArch::FTINTRNE_L_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 2026 /* ftintrne.w.d */, LoongArch::FTINTRNE_W_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 2039 /* ftintrne.w.s */, LoongArch::FTINTRNE_W_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 2052 /* ftintrp.l.d */, LoongArch::FTINTRP_L_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 2064 /* ftintrp.l.s */, LoongArch::FTINTRP_L_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 2076 /* ftintrp.w.d */, LoongArch::FTINTRP_W_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 2088 /* ftintrp.w.s */, LoongArch::FTINTRP_W_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 2100 /* ftintrz.l.d */, LoongArch::FTINTRZ_L_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR64 }, },
  { 2112 /* ftintrz.l.s */, LoongArch::FTINTRZ_L_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_FPR32 }, },
  { 2124 /* ftintrz.w.d */, LoongArch::FTINTRZ_W_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_FPR32, MCK_FPR64 }, },
  { 2136 /* ftintrz.w.s */, LoongArch::FTINTRZ_W_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_FPR32 }, },
  { 2148 /* ibar */, LoongArch::IBAR, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 2153 /* idle */, LoongArch::IDLE, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 2158 /* invtlb */, LoongArch::INVTLB, Convert__Reg1_2__Reg1_1__UImm51_0, AMFBS_None, { MCK_UImm5, MCK_GPR, MCK_GPR }, },
  { 2165 /* iocsrrd.b */, LoongArch::IOCSRRD_B, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2175 /* iocsrrd.d */, LoongArch::IOCSRRD_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2185 /* iocsrrd.h */, LoongArch::IOCSRRD_H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2195 /* iocsrrd.w */, LoongArch::IOCSRRD_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2205 /* iocsrwr.b */, LoongArch::IOCSRWR_B, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2215 /* iocsrwr.d */, LoongArch::IOCSRWR_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2225 /* iocsrwr.h */, LoongArch::IOCSRWR_H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2235 /* iocsrwr.w */, LoongArch::IOCSRWR_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2245 /* jirl */, LoongArch::JIRL, Convert__Reg1_0__Reg1_1__SImm16lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm16lsl2 }, },
  { 2250 /* jr */, LoongArch::JIRL, Convert__regR0__Reg1_0__imm_95_0, AMFBS_None, { MCK_GPR }, },
  { 2253 /* la */, LoongArch::PseudoLA_PCREL, Convert__Reg1_0__BareSymbol1_1, AMFBS_HasLaGlobalWithPcrel, { MCK_GPR, MCK_BareSymbol }, },
  { 2253 /* la */, LoongArch::PseudoLA_ABS, Convert__Reg1_0__BareSymbol1_1, AMFBS_HasLaGlobalWithAbs, { MCK_GPR, MCK_BareSymbol }, },
  { 2253 /* la */, LoongArch::PseudoLA_GOT, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2256 /* la.abs */, LoongArch::PseudoLA_ABS, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2256 /* la.abs */, LoongArch::PseudoLA_ABS_LARGE, Convert__Reg1_0__imm_95_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_PCREL, Convert__Reg1_0__BareSymbol1_1, AMFBS_HasLaGlobalWithPcrel, { MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_ABS, Convert__Reg1_0__BareSymbol1_1, AMFBS_HasLaGlobalWithAbs, { MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_GOT, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_PCREL_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_HasLaGlobalWithPcrel, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_ABS_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_HasLaGlobalWithAbs, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2263 /* la.global */, LoongArch::PseudoLA_GOT_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2273 /* la.got */, LoongArch::PseudoLA_GOT, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2273 /* la.got */, LoongArch::PseudoLA_GOT_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2280 /* la.local */, LoongArch::PseudoLA_ABS, Convert__Reg1_0__BareSymbol1_1, AMFBS_HasLaLocalWithAbs, { MCK_GPR, MCK_BareSymbol }, },
  { 2280 /* la.local */, LoongArch::PseudoLA_PCREL, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2280 /* la.local */, LoongArch::PseudoLA_ABS_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_HasLaLocalWithAbs, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2280 /* la.local */, LoongArch::PseudoLA_PCREL_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2289 /* la.pcrel */, LoongArch::PseudoLA_PCREL, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2289 /* la.pcrel */, LoongArch::PseudoLA_PCREL_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2298 /* la.tls.gd */, LoongArch::PseudoLA_TLS_GD, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2298 /* la.tls.gd */, LoongArch::PseudoLA_TLS_GD_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2308 /* la.tls.ie */, LoongArch::PseudoLA_TLS_IE, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2308 /* la.tls.ie */, LoongArch::PseudoLA_TLS_IE_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2318 /* la.tls.ld */, LoongArch::PseudoLA_TLS_LD, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2318 /* la.tls.ld */, LoongArch::PseudoLA_TLS_LD_LARGE, Convert__Reg1_0__Reg1_1__BareSymbol1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_BareSymbol }, },
  { 2328 /* la.tls.le */, LoongArch::PseudoLA_TLS_LE, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2338 /* ld.b */, LoongArch::LD_B, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2343 /* ld.bu */, LoongArch::LD_BU, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2349 /* ld.d */, LoongArch::LD_D, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2354 /* ld.h */, LoongArch::LD_H, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2359 /* ld.hu */, LoongArch::LD_HU, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2365 /* ld.w */, LoongArch::LD_W, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2370 /* ld.wu */, LoongArch::LD_WU, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 2376 /* lddir */, LoongArch::LDDIR, Convert__Reg1_0__Reg1_1__UImm81_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm8 }, },
  { 2382 /* ldgt.b */, LoongArch::LDGT_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2389 /* ldgt.d */, LoongArch::LDGT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2396 /* ldgt.h */, LoongArch::LDGT_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2403 /* ldgt.w */, LoongArch::LDGT_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2410 /* ldle.b */, LoongArch::LDLE_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2417 /* ldle.d */, LoongArch::LDLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2424 /* ldle.h */, LoongArch::LDLE_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2431 /* ldle.w */, LoongArch::LDLE_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2438 /* ldpte */, LoongArch::LDPTE, Convert__Reg1_0__UImm81_1, AMFBS_None, { MCK_GPR, MCK_UImm8 }, },
  { 2444 /* ldptr.d */, LoongArch::LDPTR_D, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2452 /* ldptr.w */, LoongArch::LDPTR_W, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2460 /* ldx.b */, LoongArch::LDX_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2466 /* ldx.bu */, LoongArch::LDX_BU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2473 /* ldx.d */, LoongArch::LDX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2479 /* ldx.h */, LoongArch::LDX_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2485 /* ldx.hu */, LoongArch::LDX_HU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2492 /* ldx.w */, LoongArch::LDX_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2498 /* ldx.wu */, LoongArch::LDX_WU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2505 /* li.d */, LoongArch::PseudoLI_D, Convert__Reg1_0__Imm1_1, AMFBS_IsLA64, { MCK_GPR, MCK_Imm }, },
  { 2510 /* li.w */, LoongArch::PseudoLI_W, Convert__Reg1_0__Imm321_1, AMFBS_None, { MCK_GPR, MCK_Imm32 }, },
  { 2515 /* ll.d */, LoongArch::LL_D, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2520 /* ll.w */, LoongArch::LL_W, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2525 /* lu12i.w */, LoongArch::LU12I_W, Convert__Reg1_0__SImm20lu12iw1_1, AMFBS_None, { MCK_GPR, MCK_SImm20lu12iw }, },
  { 2533 /* lu32i.d */, LoongArch::LU32I_D, Convert__Reg1_0__Tie0_1_1__SImm20lu32id1_1, AMFBS_IsLA64, { MCK_GPR, MCK_SImm20lu32id }, },
  { 2541 /* lu52i.d */, LoongArch::LU52I_D, Convert__Reg1_0__Reg1_1__SImm12lu52id1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm12lu52id }, },
  { 2549 /* maskeqz */, LoongArch::MASKEQZ, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2557 /* masknez */, LoongArch::MASKNEZ, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2565 /* mod.d */, LoongArch::MOD_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2571 /* mod.du */, LoongArch::MOD_DU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2578 /* mod.w */, LoongArch::MOD_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2584 /* mod.wu */, LoongArch::MOD_WU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2591 /* movcf2fr */, LoongArch::MOVCF2FR_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_CFR }, },
  { 2600 /* movcf2gr */, LoongArch::MOVCF2GR, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_GPR, MCK_CFR }, },
  { 2609 /* move */, LoongArch::OR, Convert__Reg1_0__Reg1_1__regR0, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2614 /* movfcsr2gr */, LoongArch::MOVFCSR2GR, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_GPR, MCK_FCSR }, },
  { 2625 /* movfr2cf */, LoongArch::MOVFR2CF_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_CFR, MCK_FPR32 }, },
  { 2634 /* movfr2gr.d */, LoongArch::MOVFR2GR_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD_IsLA64, { MCK_GPR, MCK_FPR64 }, },
  { 2645 /* movfr2gr.s */, LoongArch::MOVFR2GR_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_GPR, MCK_FPR32 }, },
  { 2656 /* movfrh2gr.s */, LoongArch::MOVFRH2GR_S, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD, { MCK_GPR, MCK_FPR64 }, },
  { 2668 /* movgr2cf */, LoongArch::MOVGR2CF, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_CFR, MCK_GPR }, },
  { 2677 /* movgr2fcsr */, LoongArch::MOVGR2FCSR, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FCSR, MCK_GPR }, },
  { 2688 /* movgr2fr.d */, LoongArch::MOVGR2FR_D, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicD_IsLA64, { MCK_FPR64, MCK_GPR }, },
  { 2699 /* movgr2fr.w */, LoongArch::MOVGR2FR_W, Convert__Reg1_0__Reg1_1, AMFBS_HasBasicF, { MCK_FPR32, MCK_GPR }, },
  { 2710 /* movgr2frh.w */, LoongArch::MOVGR2FRH_W, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasBasicD, { MCK_FPR64, MCK_GPR }, },
  { 2722 /* mul.d */, LoongArch::MUL_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2728 /* mul.w */, LoongArch::MUL_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2734 /* mulh.d */, LoongArch::MULH_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2741 /* mulh.du */, LoongArch::MULH_DU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2749 /* mulh.w */, LoongArch::MULH_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2756 /* mulh.wu */, LoongArch::MULH_WU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2764 /* mulw.d.w */, LoongArch::MULW_D_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2773 /* mulw.d.wu */, LoongArch::MULW_D_WU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2783 /* nop */, LoongArch::ANDI, Convert__regR0__regR0__imm_95_0, AMFBS_None, {  }, },
  { 2787 /* nor */, LoongArch::NOR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2791 /* or */, LoongArch::OR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2794 /* ori */, LoongArch::ORI, Convert__Reg1_0__Reg1_1__UImm12ori1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm12ori }, },
  { 2798 /* orn */, LoongArch::ORN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2802 /* pcaddi */, LoongArch::PCADDI, Convert__Reg1_0__SImm201_1, AMFBS_None, { MCK_GPR, MCK_SImm20 }, },
  { 2809 /* pcaddu12i */, LoongArch::PCADDU12I, Convert__Reg1_0__SImm201_1, AMFBS_None, { MCK_GPR, MCK_SImm20 }, },
  { 2819 /* pcaddu18i */, LoongArch::PCADDU18I, Convert__Reg1_0__SImm201_1, AMFBS_IsLA64, { MCK_GPR, MCK_SImm20 }, },
  { 2829 /* pcalau12i */, LoongArch::PCALAU12I, Convert__Reg1_0__SImm20pcalau12i1_1, AMFBS_None, { MCK_GPR, MCK_SImm20pcalau12i }, },
  { 2839 /* preld */, LoongArch::PRELD, Convert__UImm51_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_UImm5, MCK_GPR, MCK_SImm12 }, },
  { 2845 /* preldx */, LoongArch::PRELDX, Convert__UImm51_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_UImm5, MCK_GPR, MCK_GPR }, },
  { 2852 /* rdtime.d */, LoongArch::RDTIME_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2861 /* rdtimeh.w */, LoongArch::RDTIMEH_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2871 /* rdtimel.w */, LoongArch::RDTIMEL_W, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2881 /* ret */, LoongArch::JIRL, Convert__regR0__regR1__imm_95_0, AMFBS_None, {  }, },
  { 2885 /* revb.2h */, LoongArch::REVB_2H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2893 /* revb.2w */, LoongArch::REVB_2W, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2901 /* revb.4h */, LoongArch::REVB_4H, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2909 /* revb.d */, LoongArch::REVB_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2916 /* revh.2w */, LoongArch::REVH_2W, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2924 /* revh.d */, LoongArch::REVH_D, Convert__Reg1_0__Reg1_1, AMFBS_IsLA64, { MCK_GPR, MCK_GPR }, },
  { 2931 /* rotr.d */, LoongArch::ROTR_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2938 /* rotr.w */, LoongArch::ROTR_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2945 /* rotri.d */, LoongArch::ROTRI_D, Convert__Reg1_0__Reg1_1__UImm61_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6 }, },
  { 2953 /* rotri.w */, LoongArch::ROTRI_W, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 2961 /* sc.d */, LoongArch::SC_D, Convert__Reg1_0__Tie0_1_1__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2966 /* sc.w */, LoongArch::SC_W, Convert__Reg1_0__Tie0_1_1__Reg1_1__SImm14lsl21_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 2971 /* sll.d */, LoongArch::SLL_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2977 /* sll.w */, LoongArch::SLL_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2983 /* slli.d */, LoongArch::SLLI_D, Convert__Reg1_0__Reg1_1__UImm61_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6 }, },
  { 2990 /* slli.w */, LoongArch::SLLI_W, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 2997 /* slt */, LoongArch::SLT, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3001 /* slti */, LoongArch::SLTI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3006 /* sltu */, LoongArch::SLTU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3011 /* sltui */, LoongArch::SLTUI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3017 /* sra.d */, LoongArch::SRA_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3023 /* sra.w */, LoongArch::SRA_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3029 /* srai.d */, LoongArch::SRAI_D, Convert__Reg1_0__Reg1_1__UImm61_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6 }, },
  { 3036 /* srai.w */, LoongArch::SRAI_W, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3043 /* srl.d */, LoongArch::SRL_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3049 /* srl.w */, LoongArch::SRL_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3055 /* srli.d */, LoongArch::SRLI_D, Convert__Reg1_0__Reg1_1__UImm61_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_UImm6 }, },
  { 3062 /* srli.w */, LoongArch::SRLI_W, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3069 /* st.b */, LoongArch::ST_B, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 3074 /* st.d */, LoongArch::ST_D, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 3079 /* st.h */, LoongArch::ST_H, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 3084 /* st.w */, LoongArch::ST_W, Convert__Reg1_0__Reg1_1__SImm12addlike1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12addlike }, },
  { 3089 /* stgt.b */, LoongArch::STGT_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3096 /* stgt.d */, LoongArch::STGT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3103 /* stgt.h */, LoongArch::STGT_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3110 /* stgt.w */, LoongArch::STGT_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3117 /* stle.b */, LoongArch::STLE_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3124 /* stle.d */, LoongArch::STLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3131 /* stle.h */, LoongArch::STLE_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3138 /* stle.w */, LoongArch::STLE_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3145 /* stptr.d */, LoongArch::STPTR_D, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 3153 /* stptr.w */, LoongArch::STPTR_W, Convert__Reg1_0__Reg1_1__SImm14lsl21_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_SImm14lsl2 }, },
  { 3161 /* stx.b */, LoongArch::STX_B, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3167 /* stx.d */, LoongArch::STX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3173 /* stx.h */, LoongArch::STX_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3179 /* stx.w */, LoongArch::STX_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3185 /* sub.d */, LoongArch::SUB_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsLA64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3191 /* sub.w */, LoongArch::SUB_W, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3197 /* syscall */, LoongArch::SYSCALL, Convert__UImm151_0, AMFBS_None, { MCK_UImm15 }, },
  { 3205 /* tlbclr */, LoongArch::TLBCLR, Convert_NoOperands, AMFBS_None, {  }, },
  { 3212 /* tlbfill */, LoongArch::TLBFILL, Convert_NoOperands, AMFBS_None, {  }, },
  { 3220 /* tlbflush */, LoongArch::TLBFLUSH, Convert_NoOperands, AMFBS_None, {  }, },
  { 3229 /* tlbrd */, LoongArch::TLBRD, Convert_NoOperands, AMFBS_None, {  }, },
  { 3235 /* tlbsrch */, LoongArch::TLBSRCH, Convert_NoOperands, AMFBS_None, {  }, },
  { 3243 /* tlbwr */, LoongArch::TLBWR, Convert_NoOperands, AMFBS_None, {  }, },
  { 3249 /* xor */, LoongArch::XOR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3253 /* xori */, LoongArch::XORI, Convert__Reg1_0__Reg1_1__UImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImm12 }, },
};

#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

unsigned LoongArchAsmParser::
MatchInstructionImpl(const OperandVector &Operands,
                     MCInst &Inst,
                     uint64_t &ErrorInfo,
                     FeatureBitset &MissingFeatures,
                     bool matchingInlineAsm, unsigned VariantID) {
  // Eliminate obvious mismatches.
  if (Operands.size() > 5) {
    ErrorInfo = 5;
    return Match_InvalidOperand;
  }

  // Get the current feature set.
  const FeatureBitset &AvailableFeatures = getAvailableFeatures();

  // Get the instruction mnemonic, which is the first token.
  StringRef Mnemonic = ((LoongArchOperand &)*Operands[0]).getToken();

  // Some state to try to produce better error messages.
  bool HadMatchOtherThanFeatures = false;
  bool HadMatchOtherThanPredicate = false;
  unsigned RetCode = Match_InvalidOperand;
  MissingFeatures.set();
  // Set ErrorInfo to the operand that mismatches if it is
  // wrong for all instances of the instruction.
  ErrorInfo = ~0ULL;
  // Find the appropriate table for this asm variant.
  const MatchEntry *Start, *End;
  switch (VariantID) {
  default: llvm_unreachable("invalid variant!");
  case 0: Start = std::begin(MatchTable0); End = std::end(MatchTable0); break;
  }
  // Search the table.
  auto MnemonicRange = std::equal_range(Start, End, Mnemonic, LessOpcode());

  DEBUG_WITH_TYPE("asm-matcher", dbgs() << "AsmMatcher: found " <<
  std::distance(MnemonicRange.first, MnemonicRange.second) <<
  " encodings with mnemonic '" << Mnemonic << "'\n");

  // Return a more specific error code if no mnemonics match.
  if (MnemonicRange.first == MnemonicRange.second)
    return Match_MnemonicFail;

  for (const MatchEntry *it = MnemonicRange.first, *ie = MnemonicRange.second;
       it != ie; ++it) {
    const FeatureBitset &RequiredFeatures = FeatureBitsets[it->RequiredFeaturesIdx];
    bool HasRequiredFeatures =
      (AvailableFeatures & RequiredFeatures) == RequiredFeatures;
    DEBUG_WITH_TYPE("asm-matcher", dbgs() << "Trying to match opcode "
                                          << MII.getName(it->Opcode) << "\n");
    // equal_range guarantees that instruction mnemonic matches.
    assert(Mnemonic == it->getMnemonic());
    bool OperandsValid = true;
    for (unsigned FormalIdx = 0, ActualIdx = 1; FormalIdx != 4; ++FormalIdx) {
      auto Formal = static_cast<MatchClassKind>(it->Classes[FormalIdx]);
      DEBUG_WITH_TYPE("asm-matcher",
                      dbgs() << "  Matching formal operand class " << getMatchClassName(Formal)
                             << " against actual operand at index " << ActualIdx);
      if (ActualIdx < Operands.size())
        DEBUG_WITH_TYPE("asm-matcher", dbgs() << " (";
                        Operands[ActualIdx]->print(dbgs()); dbgs() << "): ");
      else
        DEBUG_WITH_TYPE("asm-matcher", dbgs() << ": ");
      if (ActualIdx >= Operands.size()) {
        DEBUG_WITH_TYPE("asm-matcher", dbgs() << "actual operand index out of range\n");
        if (Formal == InvalidMatchClass) {
          break;
        }
        if (isSubclass(Formal, OptionalMatchClass)) {
          continue;
        }
        OperandsValid = false;
        ErrorInfo = ActualIdx;
        break;
      }
      MCParsedAsmOperand &Actual = *Operands[ActualIdx];
      unsigned Diag = validateOperandClass(Actual, Formal);
      if (Diag == Match_Success) {
        DEBUG_WITH_TYPE("asm-matcher",
                        dbgs() << "match success using generic matcher\n");
        ++ActualIdx;
        continue;
      }
      // If the generic handler indicates an invalid operand
      // failure, check for a special case.
      if (Diag != Match_Success) {
        unsigned TargetDiag = validateTargetOperandClass(Actual, Formal);
        if (TargetDiag == Match_Success) {
          DEBUG_WITH_TYPE("asm-matcher",
                          dbgs() << "match success using target matcher\n");
          ++ActualIdx;
          continue;
        }
        // If the target matcher returned a specific error code use
        // that, else use the one from the generic matcher.
        if (TargetDiag != Match_InvalidOperand && HasRequiredFeatures)
          Diag = TargetDiag;
      }
      // If current formal operand wasn't matched and it is optional
      // then try to match next formal operand
      if (Diag == Match_InvalidOperand && isSubclass(Formal, OptionalMatchClass)) {
        DEBUG_WITH_TYPE("asm-matcher", dbgs() << "ignoring optional operand\n");
        continue;
      }
      // If this operand is broken for all of the instances of this
      // mnemonic, keep track of it so we can report loc info.
      // If we already had a match that only failed due to a
      // target predicate, that diagnostic is preferred.
      if (!HadMatchOtherThanPredicate &&
          (it == MnemonicRange.first || ErrorInfo <= ActualIdx)) {
        if (HasRequiredFeatures && (ErrorInfo != ActualIdx || Diag != Match_InvalidOperand))
          RetCode = Diag;
        ErrorInfo = ActualIdx;
      }
      // Otherwise, just reject this instance of the mnemonic.
      OperandsValid = false;
      break;
    }

    if (!OperandsValid) {
      DEBUG_WITH_TYPE("asm-matcher", dbgs() << "Opcode result: multiple "
                                               "operand mismatches, ignoring "
                                               "this opcode\n");
      continue;
    }
    if (!HasRequiredFeatures) {
      HadMatchOtherThanFeatures = true;
      FeatureBitset NewMissingFeatures = RequiredFeatures & ~AvailableFeatures;
      DEBUG_WITH_TYPE("asm-matcher", dbgs() << "Missing target features:";
                      for (unsigned I = 0, E = NewMissingFeatures.size(); I != E; ++I)
                        if (NewMissingFeatures[I])
                          dbgs() << ' ' << I;
                      dbgs() << "\n");
      if (NewMissingFeatures.count() <=
          MissingFeatures.count())
        MissingFeatures = NewMissingFeatures;
      continue;
    }

    Inst.clear();

    Inst.setOpcode(it->Opcode);
    // We have a potential match but have not rendered the operands.
    // Check the target predicate to handle any context sensitive
    // constraints.
    // For example, Ties that are referenced multiple times must be
    // checked here to ensure the input is the same for each match
    // constraints. If we leave it any later the ties will have been
    // canonicalized
    unsigned MatchResult;
    if ((MatchResult = checkEarlyTargetMatchPredicate(Inst, Operands)) != Match_Success) {
      Inst.clear();
      DEBUG_WITH_TYPE(
          "asm-matcher",
          dbgs() << "Early target match predicate failed with diag code "
                 << MatchResult << "\n");
      RetCode = MatchResult;
      HadMatchOtherThanPredicate = true;
      continue;
    }

    if (matchingInlineAsm) {
      convertToMapAndConstraints(it->ConvertFn, Operands);
      if (!checkAsmTiedOperandConstraints(*this, it->ConvertFn, Operands, ErrorInfo))
        return Match_InvalidTiedOperand;

      return Match_Success;
    }

    // We have selected a definite instruction, convert the parsed
    // operands into the appropriate MCInst.
    convertToMCInst(it->ConvertFn, Inst, it->Opcode, Operands);

    // We have a potential match. Check the target predicate to
    // handle any context sensitive constraints.
    if ((MatchResult = checkTargetMatchPredicate(Inst)) != Match_Success) {
      DEBUG_WITH_TYPE("asm-matcher",
                      dbgs() << "Target match predicate failed with diag code "
                             << MatchResult << "\n");
      Inst.clear();
      RetCode = MatchResult;
      HadMatchOtherThanPredicate = true;
      continue;
    }

    if (!checkAsmTiedOperandConstraints(*this, it->ConvertFn, Operands, ErrorInfo))
      return Match_InvalidTiedOperand;

    DEBUG_WITH_TYPE(
        "asm-matcher",
        dbgs() << "Opcode result: complete match, selecting this opcode\n");
    return Match_Success;
  }

  // Okay, we had no match.  Try to return a useful error code.
  if (HadMatchOtherThanPredicate || !HadMatchOtherThanFeatures)
    return RetCode;

  ErrorInfo = 0;
  return Match_MissingFeature;
}

namespace {
  struct OperandMatchEntry {
    uint16_t Mnemonic;
    uint8_t OperandMask;
    uint8_t Class;
    uint8_t RequiredFeaturesIdx;

    StringRef getMnemonic() const {
      return StringRef(MnemonicTable + Mnemonic + 1,
                       MnemonicTable[Mnemonic]);
    }
  };

  // Predicate for searching for an opcode.
  struct LessOpcodeOperand {
    bool operator()(const OperandMatchEntry &LHS, StringRef RHS) {
      return LHS.getMnemonic()  < RHS;
    }
    bool operator()(StringRef LHS, const OperandMatchEntry &RHS) {
      return LHS < RHS.getMnemonic();
    }
    bool operator()(const OperandMatchEntry &LHS, const OperandMatchEntry &RHS) {
      return LHS.getMnemonic() < RHS.getMnemonic();
    }
  };
} // end anonymous namespace

static const OperandMatchEntry OperandMatchTable[64] = {
  /* Operand List Mnemonic, Mask, Operand Class, Features */
  { 58 /* amadd.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 66 /* amadd.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 74 /* amadd_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 85 /* amadd_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 96 /* amand.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 104 /* amand.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 112 /* amand_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 123 /* amand_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 134 /* ammax.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 142 /* ammax.du */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 151 /* ammax.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 159 /* ammax.wu */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 168 /* ammax_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 179 /* ammax_db.du */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 191 /* ammax_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 202 /* ammax_db.wu */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 214 /* ammin.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 222 /* ammin.du */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 231 /* ammin.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 239 /* ammin.wu */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 248 /* ammin_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 259 /* ammin_db.du */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 271 /* ammin_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 282 /* ammin_db.wu */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 294 /* amor.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 301 /* amor.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 308 /* amor_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 318 /* amor_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 328 /* amswap.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 337 /* amswap.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 346 /* amswap_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 358 /* amswap_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 370 /* amxor.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 378 /* amxor.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 386 /* amxor_db.d */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 397 /* amxor_db.w */, 4 /* 2 */, MCK_AtomicMemAsmOperand, AMFBS_IsLA64 },
  { 440 /* b */, 1 /* 0 */, MCK_SImm26OperandB, AMFBS_None },
  { 529 /* bl */, 1 /* 0 */, MCK_SImm26OperandBL, AMFBS_None },
  { 2253 /* la */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithPcrel },
  { 2253 /* la */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithAbs },
  { 2253 /* la */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2256 /* la.abs */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2256 /* la.abs */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2263 /* la.global */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithPcrel },
  { 2263 /* la.global */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithAbs },
  { 2263 /* la.global */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2263 /* la.global */, 4 /* 2 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithPcrel },
  { 2263 /* la.global */, 4 /* 2 */, MCK_BareSymbol, AMFBS_HasLaGlobalWithAbs },
  { 2263 /* la.global */, 4 /* 2 */, MCK_BareSymbol, AMFBS_None },
  { 2273 /* la.got */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2273 /* la.got */, 4 /* 2 */, MCK_BareSymbol, AMFBS_IsLA64 },
  { 2280 /* la.local */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasLaLocalWithAbs },
  { 2280 /* la.local */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2280 /* la.local */, 4 /* 2 */, MCK_BareSymbol, AMFBS_HasLaLocalWithAbs },
  { 2280 /* la.local */, 4 /* 2 */, MCK_BareSymbol, AMFBS_None },
  { 2289 /* la.pcrel */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2289 /* la.pcrel */, 4 /* 2 */, MCK_BareSymbol, AMFBS_IsLA64 },
  { 2298 /* la.tls.gd */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2298 /* la.tls.gd */, 4 /* 2 */, MCK_BareSymbol, AMFBS_IsLA64 },
  { 2308 /* la.tls.ie */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2308 /* la.tls.ie */, 4 /* 2 */, MCK_BareSymbol, AMFBS_IsLA64 },
  { 2318 /* la.tls.ld */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2318 /* la.tls.ld */, 4 /* 2 */, MCK_BareSymbol, AMFBS_IsLA64 },
  { 2328 /* la.tls.le */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
};

OperandMatchResultTy LoongArchAsmParser::
tryCustomParseOperand(OperandVector &Operands,
                      unsigned MCK) {

  switch(MCK) {
  case MCK_AtomicMemAsmOperand:
    return parseAtomicMemOp(Operands);
  case MCK_BareSymbol:
    return parseImmediate(Operands);
  case MCK_SImm26OperandB:
    return parseImmediate(Operands);
  case MCK_SImm26OperandBL:
    return parseSImm26Operand(Operands);
  default:
    return MatchOperand_NoMatch;
  }
  return MatchOperand_NoMatch;
}

OperandMatchResultTy LoongArchAsmParser::
MatchOperandParserImpl(OperandVector &Operands,
                       StringRef Mnemonic,
                       bool ParseForAllFeatures) {
  // Get the current feature set.
  const FeatureBitset &AvailableFeatures = getAvailableFeatures();

  // Get the next operand index.
  unsigned NextOpNum = Operands.size() - 1;
  // Search the table.
  auto MnemonicRange =
    std::equal_range(std::begin(OperandMatchTable), std::end(OperandMatchTable),
                     Mnemonic, LessOpcodeOperand());

  if (MnemonicRange.first == MnemonicRange.second)
    return MatchOperand_NoMatch;

  for (const OperandMatchEntry *it = MnemonicRange.first,
       *ie = MnemonicRange.second; it != ie; ++it) {
    // equal_range guarantees that instruction mnemonic matches.
    assert(Mnemonic == it->getMnemonic());

    // check if the available features match
    const FeatureBitset &RequiredFeatures = FeatureBitsets[it->RequiredFeaturesIdx];
    if (!ParseForAllFeatures && (AvailableFeatures & RequiredFeatures) != RequiredFeatures)
      continue;

    // check if the operand in question has a custom parser.
    if (!(it->OperandMask & (1 << NextOpNum)))
      continue;

    // call custom parse method to handle the operand
    OperandMatchResultTy Result = tryCustomParseOperand(Operands, it->Class);
    if (Result != MatchOperand_NoMatch)
      return Result;
  }

  // Okay, we had no match.
  return MatchOperand_NoMatch;
}

#endif // GET_MATCHER_IMPLEMENTATION


#ifdef GET_MNEMONIC_SPELL_CHECKER
#undef GET_MNEMONIC_SPELL_CHECKER

static std::string LoongArchMnemonicSpellCheck(StringRef S, const FeatureBitset &FBS, unsigned VariantID) {
  const unsigned MaxEditDist = 2;
  std::vector<StringRef> Candidates;
  StringRef Prev = "";

  // Find the appropriate table for this asm variant.
  const MatchEntry *Start, *End;
  switch (VariantID) {
  default: llvm_unreachable("invalid variant!");
  case 0: Start = std::begin(MatchTable0); End = std::end(MatchTable0); break;
  }

  for (auto I = Start; I < End; I++) {
    // Ignore unsupported instructions.
    const FeatureBitset &RequiredFeatures = FeatureBitsets[I->RequiredFeaturesIdx];
    if ((FBS & RequiredFeatures) != RequiredFeatures)
      continue;

    StringRef T = I->getMnemonic();
    // Avoid recomputing the edit distance for the same string.
    if (T.equals(Prev))
      continue;

    Prev = T;
    unsigned Dist = S.edit_distance(T, false, MaxEditDist);
    if (Dist <= MaxEditDist)
      Candidates.push_back(T);
  }

  if (Candidates.empty())
    return "";

  std::string Res = ", did you mean: ";
  unsigned i = 0;
  for (; i < Candidates.size() - 1; i++)
    Res += Candidates[i].str() + ", ";
  return Res + Candidates[i].str() + "?";
}

#endif // GET_MNEMONIC_SPELL_CHECKER


#ifdef GET_MNEMONIC_CHECKER
#undef GET_MNEMONIC_CHECKER

static bool LoongArchCheckMnemonic(StringRef Mnemonic,
                                const FeatureBitset &AvailableFeatures,
                                unsigned VariantID) {
  // Find the appropriate table for this asm variant.
  const MatchEntry *Start, *End;
  switch (VariantID) {
  default: llvm_unreachable("invalid variant!");
  case 0: Start = std::begin(MatchTable0); End = std::end(MatchTable0); break;
  }

  // Search the table.
  auto MnemonicRange = std::equal_range(Start, End, Mnemonic, LessOpcode());

  if (MnemonicRange.first == MnemonicRange.second)
    return false;

  for (const MatchEntry *it = MnemonicRange.first, *ie = MnemonicRange.second;
       it != ie; ++it) {
    const FeatureBitset &RequiredFeatures =
      FeatureBitsets[it->RequiredFeaturesIdx];
    if ((AvailableFeatures & RequiredFeatures) == RequiredFeatures)
      return true;
  }
  return false;
}

#endif // GET_MNEMONIC_CHECKER

