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
                       const OperandVector &Operands,
                       const SmallBitVector &OptionalOperandsMask);
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
  Match_InvalidCLUIImm,
  Match_InvalidCSRSystemRegister,
  Match_InvalidCallSymbol,
  Match_InvalidFRMArg,
  Match_InvalidFenceArg,
  Match_InvalidImmXLenLI,
  Match_InvalidImmZero,
  Match_InvalidPseudoJumpSymbol,
  Match_InvalidRnumArg,
  Match_InvalidSImm10Lsb0000NonZero,
  Match_InvalidSImm12,
  Match_InvalidSImm12Lsb0,
  Match_InvalidSImm12Lsb00000,
  Match_InvalidSImm13Lsb0,
  Match_InvalidSImm21Lsb0JAL,
  Match_InvalidSImm5,
  Match_InvalidSImm5Plus1,
  Match_InvalidSImm6,
  Match_InvalidSImm6NonZero,
  Match_InvalidSImm9Lsb0,
  Match_InvalidTPRelAddSymbol,
  Match_InvalidUImm10Lsb00NonZero,
  Match_InvalidUImm2,
  Match_InvalidUImm20AUIPC,
  Match_InvalidUImm20LUI,
  Match_InvalidUImm3,
  Match_InvalidUImm5,
  Match_InvalidUImm7,
  Match_InvalidUImm7Lsb00,
  Match_InvalidUImm8Lsb00,
  Match_InvalidUImm8Lsb000,
  Match_InvalidUImm9Lsb000,
  Match_InvalidUImmLog2XLen,
  Match_InvalidUImmLog2XLenHalf,
  Match_InvalidUImmLog2XLenNonZero,
  Match_InvalidVMaskRegister,
  Match_InvalidVTypeI,
  END_OPERAND_DIAGNOSTIC_TYPES
#endif // GET_OPERAND_DIAGNOSTIC_TYPES


#ifdef GET_REGISTER_MATCHER
#undef GET_REGISTER_MATCHER

// Bits for subtarget features that participate in instruction matching.
enum SubtargetFeatureBits : uint8_t {
  Feature_HasStdExtMBit = 9,
  Feature_HasStdExtMOrZmmulBit = 10,
  Feature_HasStdExtABit = 1,
  Feature_HasStdExtFBit = 7,
  Feature_HasStdExtDBit = 6,
  Feature_HasStdExtHBit = 8,
  Feature_HasStdExtZihintpauseBit = 34,
  Feature_HasStdExtZihintntlBit = 33,
  Feature_HasStdExtZfhminBit = 25,
  Feature_HasStdExtZfhBit = 23,
  Feature_HasStdExtZfhOrZfhminBit = 24,
  Feature_HasStdExtZfinxBit = 26,
  Feature_HasStdExtZdinxBit = 22,
  Feature_HasStdExtZhinxminBit = 29,
  Feature_HasStdExtZhinxBit = 27,
  Feature_HasStdExtZhinxOrZhinxminBit = 28,
  Feature_HasStdExtCBit = 2,
  Feature_HasStdExtZbaBit = 13,
  Feature_HasStdExtZbbBit = 14,
  Feature_HasStdExtZbcBit = 16,
  Feature_HasStdExtZbsBit = 21,
  Feature_HasStdExtZbkbBit = 18,
  Feature_HasStdExtZbkxBit = 20,
  Feature_HasStdExtZbbOrZbkbBit = 15,
  Feature_HasStdExtZbkcBit = 19,
  Feature_HasStdExtZbcOrZbkcBit = 17,
  Feature_HasStdExtZkndBit = 35,
  Feature_HasStdExtZkneBit = 37,
  Feature_HasStdExtZkndOrZkneBit = 36,
  Feature_HasStdExtZknhBit = 38,
  Feature_HasStdExtZksedBit = 40,
  Feature_HasStdExtZkshBit = 41,
  Feature_HasStdExtZkrBit = 39,
  Feature_HasStdExtCOrZcaBit = 3,
  Feature_HasStdExtCOrZcdBit = 4,
  Feature_HasStdExtCOrZcfBit = 5,
  Feature_HasRVCHintsBit = 0,
  Feature_HasVInstructionsBit = 43,
  Feature_HasVInstructionsI64Bit = 45,
  Feature_HasVInstructionsAnyFBit = 44,
  Feature_HasStdExtZicbomBit = 30,
  Feature_HasStdExtZicbozBit = 32,
  Feature_HasStdExtZicbopBit = 31,
  Feature_HasStdExtSvinvalBit = 11,
  Feature_HasStdExtZtsoBit = 42,
  Feature_HasStdExtZawrsBit = 12,
  Feature_HasVendorXVentanaCondOpsBit = 47,
  Feature_HasVendorXTHeadVdotBit = 46,
  Feature_IsRV64Bit = 50,
  Feature_IsRV32Bit = 48,
  Feature_IsRV32EBit = 49,
};

static unsigned MatchRegisterName(StringRef Name) {
  switch (Name.size()) {
  default: break;
  case 2:	 // 66 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 30 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 3 strings to match.
        return 72;	 // "f0"
      case '1':	 // 3 strings to match.
        return 73;	 // "f1"
      case '2':	 // 3 strings to match.
        return 74;	 // "f2"
      case '3':	 // 3 strings to match.
        return 75;	 // "f3"
      case '4':	 // 3 strings to match.
        return 76;	 // "f4"
      case '5':	 // 3 strings to match.
        return 77;	 // "f5"
      case '6':	 // 3 strings to match.
        return 78;	 // "f6"
      case '7':	 // 3 strings to match.
        return 79;	 // "f7"
      case '8':	 // 3 strings to match.
        return 80;	 // "f8"
      case '9':	 // 3 strings to match.
        return 81;	 // "f9"
      }
      break;
    case 'v':	 // 21 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 4 strings to match.
        return 8;	 // "v0"
      case '1':	 // 1 string to match.
        return 9;	 // "v1"
      case '2':	 // 2 strings to match.
        return 10;	 // "v2"
      case '3':	 // 1 string to match.
        return 11;	 // "v3"
      case '4':	 // 3 strings to match.
        return 12;	 // "v4"
      case '5':	 // 1 string to match.
        return 13;	 // "v5"
      case '6':	 // 2 strings to match.
        return 14;	 // "v6"
      case '7':	 // 1 string to match.
        return 15;	 // "v7"
      case '8':	 // 4 strings to match.
        return 16;	 // "v8"
      case '9':	 // 1 string to match.
        return 17;	 // "v9"
      case 'l':	 // 1 string to match.
        return 3;	 // "vl"
      }
      break;
    case 'x':	 // 15 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 40;	 // "x0"
      case '1':	 // 1 string to match.
        return 41;	 // "x1"
      case '2':	 // 2 strings to match.
        return 42;	 // "x2"
      case '3':	 // 1 string to match.
        return 43;	 // "x3"
      case '4':	 // 2 strings to match.
        return 44;	 // "x4"
      case '5':	 // 1 string to match.
        return 45;	 // "x5"
      case '6':	 // 2 strings to match.
        return 46;	 // "x6"
      case '7':	 // 1 string to match.
        return 47;	 // "x7"
      case '8':	 // 2 strings to match.
        return 48;	 // "x8"
      case '9':	 // 1 string to match.
        return 49;	 // "x9"
      }
      break;
    }
    break;
  case 3:	 // 140 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 67 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 30 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 82;	 // "f10"
        case '1':	 // 3 strings to match.
          return 83;	 // "f11"
        case '2':	 // 3 strings to match.
          return 84;	 // "f12"
        case '3':	 // 3 strings to match.
          return 85;	 // "f13"
        case '4':	 // 3 strings to match.
          return 86;	 // "f14"
        case '5':	 // 3 strings to match.
          return 87;	 // "f15"
        case '6':	 // 3 strings to match.
          return 88;	 // "f16"
        case '7':	 // 3 strings to match.
          return 89;	 // "f17"
        case '8':	 // 3 strings to match.
          return 90;	 // "f18"
        case '9':	 // 3 strings to match.
          return 91;	 // "f19"
        }
        break;
      case '2':	 // 30 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 92;	 // "f20"
        case '1':	 // 3 strings to match.
          return 93;	 // "f21"
        case '2':	 // 3 strings to match.
          return 94;	 // "f22"
        case '3':	 // 3 strings to match.
          return 95;	 // "f23"
        case '4':	 // 3 strings to match.
          return 96;	 // "f24"
        case '5':	 // 3 strings to match.
          return 97;	 // "f25"
        case '6':	 // 3 strings to match.
          return 98;	 // "f26"
        case '7':	 // 3 strings to match.
          return 99;	 // "f27"
        case '8':	 // 3 strings to match.
          return 100;	 // "f28"
        case '9':	 // 3 strings to match.
          return 101;	 // "f29"
        }
        break;
      case '3':	 // 6 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 102;	 // "f30"
        case '1':	 // 3 strings to match.
          return 103;	 // "f31"
        }
        break;
      case 'r':	 // 1 string to match.
        if (Name[2] != 'm')
          break;
        return 2;	 // "frm"
      }
      break;
    case 'v':	 // 40 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 18 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 18;	 // "v10"
        case '1':	 // 1 string to match.
          return 19;	 // "v11"
        case '2':	 // 3 strings to match.
          return 20;	 // "v12"
        case '3':	 // 1 string to match.
          return 21;	 // "v13"
        case '4':	 // 2 strings to match.
          return 22;	 // "v14"
        case '5':	 // 1 string to match.
          return 23;	 // "v15"
        case '6':	 // 4 strings to match.
          return 24;	 // "v16"
        case '7':	 // 1 string to match.
          return 25;	 // "v17"
        case '8':	 // 2 strings to match.
          return 26;	 // "v18"
        case '9':	 // 1 string to match.
          return 27;	 // "v19"
        }
        break;
      case '2':	 // 19 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 28;	 // "v20"
        case '1':	 // 1 string to match.
          return 29;	 // "v21"
        case '2':	 // 2 strings to match.
          return 30;	 // "v22"
        case '3':	 // 1 string to match.
          return 31;	 // "v23"
        case '4':	 // 4 strings to match.
          return 32;	 // "v24"
        case '5':	 // 1 string to match.
          return 33;	 // "v25"
        case '6':	 // 2 strings to match.
          return 34;	 // "v26"
        case '7':	 // 1 string to match.
          return 35;	 // "v27"
        case '8':	 // 3 strings to match.
          return 36;	 // "v28"
        case '9':	 // 1 string to match.
          return 37;	 // "v29"
        }
        break;
      case '3':	 // 3 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 38;	 // "v30"
        case '1':	 // 1 string to match.
          return 39;	 // "v31"
        }
        break;
      }
      break;
    case 'x':	 // 33 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 15 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 50;	 // "x10"
        case '1':	 // 1 string to match.
          return 51;	 // "x11"
        case '2':	 // 2 strings to match.
          return 52;	 // "x12"
        case '3':	 // 1 string to match.
          return 53;	 // "x13"
        case '4':	 // 2 strings to match.
          return 54;	 // "x14"
        case '5':	 // 1 string to match.
          return 55;	 // "x15"
        case '6':	 // 2 strings to match.
          return 56;	 // "x16"
        case '7':	 // 1 string to match.
          return 57;	 // "x17"
        case '8':	 // 2 strings to match.
          return 58;	 // "x18"
        case '9':	 // 1 string to match.
          return 59;	 // "x19"
        }
        break;
      case '2':	 // 15 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 60;	 // "x20"
        case '1':	 // 1 string to match.
          return 61;	 // "x21"
        case '2':	 // 2 strings to match.
          return 62;	 // "x22"
        case '3':	 // 1 string to match.
          return 63;	 // "x23"
        case '4':	 // 2 strings to match.
          return 64;	 // "x24"
        case '5':	 // 1 string to match.
          return 65;	 // "x25"
        case '6':	 // 2 strings to match.
          return 66;	 // "x26"
        case '7':	 // 1 string to match.
          return 67;	 // "x27"
        case '8':	 // 2 strings to match.
          return 68;	 // "x28"
        case '9':	 // 1 string to match.
          return 69;	 // "x29"
        }
        break;
      case '3':	 // 3 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 2 strings to match.
          return 70;	 // "x30"
        case '1':	 // 1 string to match.
          return 71;	 // "x31"
        }
        break;
      }
      break;
    }
    break;
  case 4:	 // 1 string to match.
    if (memcmp(Name.data()+0, "vxrm", 4) != 0)
      break;
    return 6;	 // "vxrm"
  case 5:	 // 3 strings to match.
    if (Name[0] != 'v')
      break;
    switch (Name[1]) {
    default: break;
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+2, "enb", 3) != 0)
        break;
      return 4;	 // "vlenb"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+2, "ype", 3) != 0)
        break;
      return 5;	 // "vtype"
    case 'x':	 // 1 string to match.
      if (memcmp(Name.data()+2, "sat", 3) != 0)
        break;
      return 7;	 // "vxsat"
    }
    break;
  case 6:	 // 1 string to match.
    if (memcmp(Name.data()+0, "fflags", 6) != 0)
      break;
    return 1;	 // "fflags"
  }
  return 0;
}

static unsigned MatchRegisterAltName(StringRef Name) {
  switch (Name.size()) {
  default: break;
  case 2:	 // 154 strings to match.
    switch (Name[0]) {
    default: break;
    case 'a':	 // 12 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 50;	 // "a0"
      case '1':	 // 1 string to match.
        return 51;	 // "a1"
      case '2':	 // 2 strings to match.
        return 52;	 // "a2"
      case '3':	 // 1 string to match.
        return 53;	 // "a3"
      case '4':	 // 2 strings to match.
        return 54;	 // "a4"
      case '5':	 // 1 string to match.
        return 55;	 // "a5"
      case '6':	 // 2 strings to match.
        return 56;	 // "a6"
      case '7':	 // 1 string to match.
        return 57;	 // "a7"
      }
      break;
    case 'f':	 // 2 strings to match.
      if (Name[1] != 'p')
        break;
      return 48;	 // "fp"
    case 'g':	 // 1 string to match.
      if (Name[1] != 'p')
        break;
      return 43;	 // "gp"
    case 'r':	 // 1 string to match.
      if (Name[1] != 'a')
        break;
      return 41;	 // "ra"
    case 's':	 // 17 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 48;	 // "s0"
      case '1':	 // 1 string to match.
        return 49;	 // "s1"
      case '2':	 // 2 strings to match.
        return 58;	 // "s2"
      case '3':	 // 1 string to match.
        return 59;	 // "s3"
      case '4':	 // 2 strings to match.
        return 60;	 // "s4"
      case '5':	 // 1 string to match.
        return 61;	 // "s5"
      case '6':	 // 2 strings to match.
        return 62;	 // "s6"
      case '7':	 // 1 string to match.
        return 63;	 // "s7"
      case '8':	 // 2 strings to match.
        return 64;	 // "s8"
      case '9':	 // 1 string to match.
        return 65;	 // "s9"
      case 'p':	 // 2 strings to match.
        return 42;	 // "sp"
      }
      break;
    case 't':	 // 12 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 1 string to match.
        return 45;	 // "t0"
      case '1':	 // 2 strings to match.
        return 46;	 // "t1"
      case '2':	 // 1 string to match.
        return 47;	 // "t2"
      case '3':	 // 2 strings to match.
        return 68;	 // "t3"
      case '4':	 // 1 string to match.
        return 69;	 // "t4"
      case '5':	 // 2 strings to match.
        return 70;	 // "t5"
      case '6':	 // 1 string to match.
        return 71;	 // "t6"
      case 'p':	 // 2 strings to match.
        return 44;	 // "tp"
      }
      break;
    case 'v':	 // 109 strings to match.
      switch (Name[1]) {
      default: break;
      case '0':	 // 15 strings to match.
        return 8;	 // "v0"
      case '1':	 // 8 strings to match.
        return 9;	 // "v1"
      case '2':	 // 12 strings to match.
        return 10;	 // "v2"
      case '3':	 // 8 strings to match.
        return 11;	 // "v3"
      case '4':	 // 14 strings to match.
        return 12;	 // "v4"
      case '5':	 // 8 strings to match.
        return 13;	 // "v5"
      case '6':	 // 12 strings to match.
        return 14;	 // "v6"
      case '7':	 // 8 strings to match.
        return 15;	 // "v7"
      case '8':	 // 15 strings to match.
        return 16;	 // "v8"
      case '9':	 // 8 strings to match.
        return 17;	 // "v9"
      case 'l':	 // 1 string to match.
        return 3;	 // "vl"
      }
      break;
    }
    break;
  case 3:	 // 284 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 84 strings to match.
      switch (Name[1]) {
      default: break;
      case 'a':	 // 24 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 82;	 // "fa0"
        case '1':	 // 3 strings to match.
          return 83;	 // "fa1"
        case '2':	 // 3 strings to match.
          return 84;	 // "fa2"
        case '3':	 // 3 strings to match.
          return 85;	 // "fa3"
        case '4':	 // 3 strings to match.
          return 86;	 // "fa4"
        case '5':	 // 3 strings to match.
          return 87;	 // "fa5"
        case '6':	 // 3 strings to match.
          return 88;	 // "fa6"
        case '7':	 // 3 strings to match.
          return 89;	 // "fa7"
        }
        break;
      case 's':	 // 30 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 80;	 // "fs0"
        case '1':	 // 3 strings to match.
          return 81;	 // "fs1"
        case '2':	 // 3 strings to match.
          return 90;	 // "fs2"
        case '3':	 // 3 strings to match.
          return 91;	 // "fs3"
        case '4':	 // 3 strings to match.
          return 92;	 // "fs4"
        case '5':	 // 3 strings to match.
          return 93;	 // "fs5"
        case '6':	 // 3 strings to match.
          return 94;	 // "fs6"
        case '7':	 // 3 strings to match.
          return 95;	 // "fs7"
        case '8':	 // 3 strings to match.
          return 96;	 // "fs8"
        case '9':	 // 3 strings to match.
          return 97;	 // "fs9"
        }
        break;
      case 't':	 // 30 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 72;	 // "ft0"
        case '1':	 // 3 strings to match.
          return 73;	 // "ft1"
        case '2':	 // 3 strings to match.
          return 74;	 // "ft2"
        case '3':	 // 3 strings to match.
          return 75;	 // "ft3"
        case '4':	 // 3 strings to match.
          return 76;	 // "ft4"
        case '5':	 // 3 strings to match.
          return 77;	 // "ft5"
        case '6':	 // 3 strings to match.
          return 78;	 // "ft6"
        case '7':	 // 3 strings to match.
          return 79;	 // "ft7"
        case '8':	 // 3 strings to match.
          return 100;	 // "ft8"
        case '9':	 // 3 strings to match.
          return 101;	 // "ft9"
        }
        break;
      }
      break;
    case 's':	 // 3 strings to match.
      if (Name[1] != '1')
        break;
      switch (Name[2]) {
      default: break;
      case '0':	 // 2 strings to match.
        return 66;	 // "s10"
      case '1':	 // 1 string to match.
        return 67;	 // "s11"
      }
      break;
    case 'v':	 // 197 strings to match.
      switch (Name[1]) {
      default: break;
      case '1':	 // 105 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 12 strings to match.
          return 18;	 // "v10"
        case '1':	 // 8 strings to match.
          return 19;	 // "v11"
        case '2':	 // 14 strings to match.
          return 20;	 // "v12"
        case '3':	 // 8 strings to match.
          return 21;	 // "v13"
        case '4':	 // 12 strings to match.
          return 22;	 // "v14"
        case '5':	 // 8 strings to match.
          return 23;	 // "v15"
        case '6':	 // 15 strings to match.
          return 24;	 // "v16"
        case '7':	 // 8 strings to match.
          return 25;	 // "v17"
        case '8':	 // 12 strings to match.
          return 26;	 // "v18"
        case '9':	 // 8 strings to match.
          return 27;	 // "v19"
        }
        break;
      case '2':	 // 88 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 14 strings to match.
          return 28;	 // "v20"
        case '1':	 // 8 strings to match.
          return 29;	 // "v21"
        case '2':	 // 12 strings to match.
          return 30;	 // "v22"
        case '3':	 // 8 strings to match.
          return 31;	 // "v23"
        case '4':	 // 15 strings to match.
          return 32;	 // "v24"
        case '5':	 // 7 strings to match.
          return 33;	 // "v25"
        case '6':	 // 9 strings to match.
          return 34;	 // "v26"
        case '7':	 // 5 strings to match.
          return 35;	 // "v27"
        case '8':	 // 7 strings to match.
          return 36;	 // "v28"
        case '9':	 // 3 strings to match.
          return 37;	 // "v29"
        }
        break;
      case '3':	 // 4 strings to match.
        switch (Name[2]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 38;	 // "v30"
        case '1':	 // 1 string to match.
          return 39;	 // "v31"
        }
        break;
      }
      break;
    }
    break;
  case 4:	 // 15 strings to match.
    switch (Name[0]) {
    default: break;
    case 'f':	 // 12 strings to match.
      switch (Name[1]) {
      default: break;
      case 's':	 // 6 strings to match.
        if (Name[2] != '1')
          break;
        switch (Name[3]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 98;	 // "fs10"
        case '1':	 // 3 strings to match.
          return 99;	 // "fs11"
        }
        break;
      case 't':	 // 6 strings to match.
        if (Name[2] != '1')
          break;
        switch (Name[3]) {
        default: break;
        case '0':	 // 3 strings to match.
          return 102;	 // "ft10"
        case '1':	 // 3 strings to match.
          return 103;	 // "ft11"
        }
        break;
      }
      break;
    case 'v':	 // 1 string to match.
      if (memcmp(Name.data()+1, "xrm", 3) != 0)
        break;
      return 6;	 // "vxrm"
    case 'z':	 // 2 strings to match.
      if (memcmp(Name.data()+1, "ero", 3) != 0)
        break;
      return 40;	 // "zero"
    }
    break;
  case 5:	 // 3 strings to match.
    if (Name[0] != 'v')
      break;
    switch (Name[1]) {
    default: break;
    case 'l':	 // 1 string to match.
      if (memcmp(Name.data()+2, "enb", 3) != 0)
        break;
      return 4;	 // "vlenb"
    case 't':	 // 1 string to match.
      if (memcmp(Name.data()+2, "ype", 3) != 0)
        break;
      return 5;	 // "vtype"
    case 'x':	 // 1 string to match.
      if (memcmp(Name.data()+2, "sat", 3) != 0)
        break;
      return 7;	 // "vxsat"
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
  case Feature_HasStdExtMBit: return "'M' (Integer Multiplication and Division)";
  case Feature_HasStdExtMOrZmmulBit: return "'M' (Integer Multiplication and Division) or 'Zmmul' (Integer Multiplication)";
  case Feature_HasStdExtABit: return "'A' (Atomic Instructions)";
  case Feature_HasStdExtFBit: return "'F' (Single-Precision Floating-Point)";
  case Feature_HasStdExtDBit: return "'D' (Double-Precision Floating-Point)";
  case Feature_HasStdExtHBit: return "'H' (Hypervisor)";
  case Feature_HasStdExtZihintpauseBit: return "'Zihintpause' (Pause Hint)";
  case Feature_HasStdExtZihintntlBit: return "'Zihintntl' (Non-Temporal Locality Hints)";
  case Feature_HasStdExtZfhminBit: return "'Zfhmin' (Half-Precision Floating-Point Minimal)";
  case Feature_HasStdExtZfhBit: return "'Zfh' (Half-Precision Floating-Point)";
  case Feature_HasStdExtZfhOrZfhminBit: return "'Zfh' (Half-Precision Floating-Point) or 'Zfhmin' (Half-Precision Floating-Point Minimal)";
  case Feature_HasStdExtZfinxBit: return "'Zfinx' (Float in Integer)";
  case Feature_HasStdExtZdinxBit: return "'Zdinx' (Double in Integer)";
  case Feature_HasStdExtZhinxminBit: return "'Zhinxmin' (Half Float in Integer Minimal)";
  case Feature_HasStdExtZhinxBit: return "'Zhinx' (Half Float in Integer)";
  case Feature_HasStdExtZhinxOrZhinxminBit: return "'Zhinx' (Half Float in Integer) or 'Zhinxmin' (Half Float in Integer Minimal)";
  case Feature_HasStdExtCBit: return "'C' (Compressed Instructions)";
  case Feature_HasStdExtZbaBit: return "'Zba' (Address Generation Instructions)";
  case Feature_HasStdExtZbbBit: return "'Zbb' (Basic Bit-Manipulation)";
  case Feature_HasStdExtZbcBit: return "'Zbc' (Carry-Less Multiplication)";
  case Feature_HasStdExtZbsBit: return "'Zbs' (Single-Bit Instructions)";
  case Feature_HasStdExtZbkbBit: return "'Zbkb' (Bitmanip instructions for Cryptography)";
  case Feature_HasStdExtZbkxBit: return "'Zbkx' (Crossbar permutation instructions)";
  case Feature_HasStdExtZbbOrZbkbBit: return "'Zbb' (Basic Bit-Manipulation) or 'Zbkb' (Bitmanip instructions for Cryptography)";
  case Feature_HasStdExtZbkcBit: return "'Zbkc' (Carry-less multiply instructions for Cryptography)";
  case Feature_HasStdExtZbcOrZbkcBit: return "'Zbc' (Carry-Less Multiplication) or 'Zbkc' (Carry-less multiply instructions for Cryptography)";
  case Feature_HasStdExtZkndBit: return "'Zknd' (NIST Suite: AES Decryption)";
  case Feature_HasStdExtZkneBit: return "'Zkne' (NIST Suite: AES Encryption)";
  case Feature_HasStdExtZkndOrZkneBit: return "'Zknd' (NIST Suite: AES Decryption) or 'Zkne' (NIST Suite: AES Encryption)";
  case Feature_HasStdExtZknhBit: return "'Zknh' (NIST Suite: Hash Function Instructions)";
  case Feature_HasStdExtZksedBit: return "'Zksed' (ShangMi Suite: SM4 Block Cipher Instructions)";
  case Feature_HasStdExtZkshBit: return "'Zksh' (ShangMi Suite: SM3 Hash Function Instructions)";
  case Feature_HasStdExtZkrBit: return "'Zkr' (Entropy Source Extension)";
  case Feature_HasStdExtCOrZcaBit: return "'C' (Compressed Instructions) or 'Zca' (part of the C extension, excluding compressed floating point loads/stores)";
  case Feature_HasStdExtCOrZcdBit: return "'C' (Compressed Instructions) or 'Zcd' (Compressed Double-Precision Floating-Point Instructions)";
  case Feature_HasStdExtCOrZcfBit: return "'C' (Compressed Instructions) or 'Zcf' (Compressed Single-Precision Floating-Point Instructions)";
  case Feature_HasRVCHintsBit: return "RVC Hint Instructions";
  case Feature_HasVInstructionsBit: return "'V' (Vector Extension for Application Processors), 'Zve32x' or 'Zve64x' (Vector Extensions for Embedded Processors)";
  case Feature_HasVInstructionsI64Bit: return "'V' (Vector Extension for Application Processors) or 'Zve64x' (Vector Extensions for Embedded Processors)";
  case Feature_HasVInstructionsAnyFBit: return "'V' (Vector Extension for Application Processors), 'Zve32f', 'Zve64f' or 'Zve64d' (Vector Extensions for Embedded Processors)";
  case Feature_HasStdExtZicbomBit: return "'Zicbom' (Cache-Block Management Instructions)";
  case Feature_HasStdExtZicbozBit: return "'Zicboz' (Cache-Block Zero Instructions)";
  case Feature_HasStdExtZicbopBit: return "'Zicbop' (Cache-Block Prefetch Instructions)";
  case Feature_HasStdExtSvinvalBit: return "'Svinval' (Fine-Grained Address-Translation Cache Invalidation)";
  case Feature_HasStdExtZtsoBit: return "'Ztso' (Memory Model - Total Store Order)";
  case Feature_HasStdExtZawrsBit: return "'Zawrs' (Wait on Reservation Set)";
  case Feature_HasVendorXVentanaCondOpsBit: return "'XVentanaCondOps' (Ventana Conditional Ops)";
  case Feature_HasVendorXTHeadVdotBit: return "'xtheadvdot' (T-Head Vector Extensions for Dot)";
  case Feature_IsRV64Bit: return "RV64I Base Instruction Set";
  case Feature_IsRV32Bit: return "RV32I Base Instruction Set";
  case Feature_IsRV32EBit: return "";
  default: return "(unknown)";
  }
}

#endif // GET_SUBTARGET_FEATURE_NAME


#ifdef GET_MATCHER_IMPLEMENTATION
#undef GET_MATCHER_IMPLEMENTATION

static void applyMnemonicAliases(StringRef &Mnemonic, const FeatureBitset &Features, unsigned VariantID) {
  switch (VariantID) {
  case 0:
      switch (Mnemonic.size()) {
      default: break;
      case 4:	 // 1 string to match.
        if (memcmp(Mnemonic.data()+0, "move", 4) != 0)
          break;
        Mnemonic = "mv";	 // "move"
        return;
      case 5:	 // 1 string to match.
        if (memcmp(Mnemonic.data()+0, "scall", 5) != 0)
          break;
        Mnemonic = "ecall";	 // "scall"
        return;
      case 6:	 // 1 string to match.
        if (memcmp(Mnemonic.data()+0, "sbreak", 6) != 0)
          break;
        Mnemonic = "ebreak";	 // "sbreak"
        return;
      case 7:	 // 2 strings to match.
        if (memcmp(Mnemonic.data()+0, "fmv.", 4) != 0)
          break;
        switch (Mnemonic[4]) {
        default: break;
        case 's':	 // 1 string to match.
          if (memcmp(Mnemonic.data()+5, ".x", 2) != 0)
            break;
          if (Features.test(Feature_HasStdExtFBit))	 // "fmv.s.x"
            Mnemonic = "fmv.w.x";
          return;
        case 'x':	 // 1 string to match.
          if (memcmp(Mnemonic.data()+5, ".s", 2) != 0)
            break;
          if (Features.test(Feature_HasStdExtFBit))	 // "fmv.x.s"
            Mnemonic = "fmv.x.w";
          return;
        }
        break;
      }
    break;
  }
  switch (Mnemonic.size()) {
  default: break;
  case 4:	 // 1 string to match.
    if (memcmp(Mnemonic.data()+0, "move", 4) != 0)
      break;
    Mnemonic = "mv";	 // "move"
    return;
  case 5:	 // 1 string to match.
    if (memcmp(Mnemonic.data()+0, "scall", 5) != 0)
      break;
    Mnemonic = "ecall";	 // "scall"
    return;
  case 6:	 // 1 string to match.
    if (memcmp(Mnemonic.data()+0, "sbreak", 6) != 0)
      break;
    Mnemonic = "ebreak";	 // "sbreak"
    return;
  case 7:	 // 2 strings to match.
    if (memcmp(Mnemonic.data()+0, "fmv.", 4) != 0)
      break;
    switch (Mnemonic[4]) {
    default: break;
    case 's':	 // 1 string to match.
      if (memcmp(Mnemonic.data()+5, ".x", 2) != 0)
        break;
      if (Features.test(Feature_HasStdExtFBit))	 // "fmv.s.x"
        Mnemonic = "fmv.w.x";
      return;
    case 'x':	 // 1 string to match.
      if (memcmp(Mnemonic.data()+5, ".s", 2) != 0)
        break;
      if (Features.test(Feature_HasStdExtFBit))	 // "fmv.x.s"
        Mnemonic = "fmv.x.w";
      return;
    }
    break;
  }
}

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
  CVT_95_addImmOperands,
  CVT_95_Reg,
  CVT_95_addRegOperands,
  CVT_regX0,
  CVT_imm_95_0,
  CVT_regX5,
  CVT_regX2,
  CVT_regX3,
  CVT_regX4,
  CVT_95_addCSRSystemRegisterOperands,
  CVT_imm_95_7,
  CVT_95_addFRMArgOperands,
  CVT_imm_95_15,
  CVT_95_addFenceArgOperands,
  CVT_imm_95_3,
  CVT_imm_95_1,
  CVT_imm_95_2,
  CVT_regX1,
  CVT_imm_95__MINUS_1,
  CVT_imm_95_3072,
  CVT_imm_95_3200,
  CVT_imm_95_3074,
  CVT_imm_95_3202,
  CVT_imm_95_3073,
  CVT_imm_95_3201,
  CVT_95_addRegOperands_95_defaultMaskRegOp,
  CVT_reg0,
  CVT_95_addVTypeIOperands,
  CVT_imm_95_255,
  CVT_NUM_CONVERTERS
};

enum InstructionConversionKind {
  Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_3__SImm13Lsb01_4,
  Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_3__SImm121_4,
  Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_5__SImm121_3,
  Convert__Reg1_1__InsnDirectiveOpcode1_0__SImm21Lsb0JAL1_2,
  Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm71_2__Reg1_4__Reg1_5,
  Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm21_2__Reg1_4__Reg1_5__Reg1_6,
  Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_5__SImm121_3,
  Convert__Reg1_1__InsnDirectiveOpcode1_0__UImm20LUI1_2,
  Convert__Reg1_0__Reg1_1__Reg1_2,
  Convert__Reg1_0__Reg1_1__SImm121_2,
  Convert__Reg1_0__Reg1_1__Reg1_2__TPRelAddSymbol1_3,
  Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3,
  Convert__Reg1_0__Reg1_1,
  Convert__Reg1_0__Reg1_1__RnumArg1_2,
  Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1,
  Convert__Reg1_0__UImm20AUIPC1_1,
  Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2,
  Convert__Reg1_0__Reg1_1__SImm13Lsb01_2,
  Convert__Reg1_0__regX0__SImm13Lsb01_1,
  Convert__Reg1_1__Reg1_0__SImm13Lsb01_2,
  Convert__regX0__Reg1_0__SImm13Lsb01_1,
  Convert__Reg1_0__Tie0_1_1__Reg1_1,
  Convert__Reg1_0__Tie0_1_1__ImmZero1_1,
  Convert__Reg1_0__Tie0_1_1__SImm6NonZero1_1,
  Convert__Reg1_0__Tie0_1_1__SImm10Lsb0000NonZero1_1,
  Convert__Reg1_0__Reg1_1__UImm10Lsb00NonZero1_2,
  Convert__Reg1_0__Tie0_1_1__SImm61_1,
  Convert__Reg1_0__SImm9Lsb01_1,
  Convert_NoOperands,
  Convert__Reg1_0__Reg1_2__imm_95_0,
  Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1,
  Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1,
  Convert__Reg1_0__Reg1_3__UImm7Lsb001_1,
  Convert__Reg1_0__Reg1_3__UImm8Lsb001_1,
  Convert__SImm12Lsb01_0,
  Convert__Reg1_0,
  Convert__Reg1_0__SImm61_1,
  Convert__Reg1_0__CLUIImm1_1,
  Convert__SImm6NonZero1_0,
  Convert__regX0__Tie0_1_1__regX5,
  Convert__regX0__Tie0_1_1__regX2,
  Convert__regX0__Tie0_1_1__regX3,
  Convert__regX0__Tie0_1_1__regX4,
  Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1,
  Convert__Reg1_0__Tie0_1_1,
  Convert__CallSymbol1_0,
  Convert__Reg1_0__CallSymbol1_1,
  Convert__ZeroOffsetMemOpOperand1_0,
  Convert__regX0__CSRSystemRegister1_0__Reg1_1,
  Convert__regX0__CSRSystemRegister1_0__UImm51_1,
  Convert__Reg1_0__CSRSystemRegister1_1__regX0,
  Convert__Reg1_0__CSRSystemRegister1_1__Reg1_2,
  Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2,
  Convert__imm_95_0__imm_95_0,
  Convert__Reg1_0__Reg1_1__Reg1_1,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_1,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_1,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1,
  Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7,
  Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3,
  Convert__Reg1_0__GPRF64AsFPR1_1,
  Convert__Reg1_0__GPRPF64AsFPR1_1,
  Convert__Reg1_0__GPRAsFPR1_1,
  Convert__GPRF64AsFPR1_0__GPRAsFPR1_1,
  Convert__Reg1_0__Reg1_1__imm_95_7,
  Convert__GPRF64AsFPR1_0__Reg1_1__imm_95_7,
  Convert__Reg1_0__Reg1_1__FRMArg1_2,
  Convert__GPRF64AsFPR1_0__Reg1_1__FRMArg1_2,
  Convert__GPRPF64AsFPR1_0__GPRAsFPR1_1,
  Convert__GPRF64AsFPR1_0__Reg1_1,
  Convert__GPRPF64AsFPR1_0__Reg1_1,
  Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__imm_95_7,
  Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2,
  Convert__GPRAsFPR1_0__Reg1_1__imm_95_7,
  Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__imm_95_7,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__FRMArg1_2,
  Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7,
  Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2,
  Convert__Reg1_0__GPRAsFPR1_1__imm_95_7,
  Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2,
  Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__imm_95_7,
  Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1,
  Convert__Reg1_0__GPRPF64AsFPR1_1__imm_95_7,
  Convert__Reg1_0__GPRPF64AsFPR1_1__FRMArg1_2,
  Convert__imm_95_15__imm_95_15,
  Convert__FenceArg1_0__FenceArg1_1,
  Convert__Reg1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2,
  Convert__Reg1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2,
  Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2,
  Convert__Reg1_0__Reg1_2__Reg1_1,
  Convert__Reg1_0__GPRF64AsFPR1_2__GPRF64AsFPR1_1,
  Convert__Reg1_0__GPRPF64AsFPR1_2__GPRPF64AsFPR1_1,
  Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1,
  Convert__Reg1_2__Reg1_0__BareSymbol1_1,
  Convert__Reg1_0__Reg1_3__SImm121_1,
  Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7,
  Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2,
  Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2,
  Convert__Reg1_0__imm_95_3__regX0,
  Convert__Reg1_0__imm_95_1__regX0,
  Convert__Reg1_0__imm_95_2__regX0,
  Convert__regX0__imm_95_3__Reg1_0,
  Convert__Reg1_0__imm_95_3__Reg1_1,
  Convert__regX0__imm_95_1__Reg1_0,
  Convert__Reg1_0__imm_95_1__Reg1_1,
  Convert__regX0__imm_95_1__UImm51_0,
  Convert__Reg1_0__imm_95_1__UImm51_1,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__imm_95_7,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__imm_95_7,
  Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2,
  Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2,
  Convert__regX0__imm_95_2__Reg1_0,
  Convert__Reg1_0__imm_95_2__Reg1_1,
  Convert__regX0__imm_95_2__UImm51_0,
  Convert__Reg1_0__imm_95_2__UImm51_1,
  Convert__regX0__regX0,
  Convert__Reg1_0__regX0,
  Convert__Reg1_0__ZeroOffsetMemOpOperand1_1,
  Convert__regX0__SImm21Lsb0JAL1_0,
  Convert__regX1__SImm21Lsb0JAL1_0,
  Convert__Reg1_0__SImm21Lsb0JAL1_1,
  Convert__regX1__Reg1_0__imm_95_0,
  Convert__Reg1_0__Reg1_1__imm_95_0,
  Convert__regX1__Reg1_0__SImm121_1,
  Convert__regX1__Reg1_2__SImm121_0,
  Convert__regX0__Reg1_0__imm_95_0,
  Convert__regX0__Reg1_0__SImm121_1,
  Convert__regX0__Reg1_2__SImm121_0,
  Convert__Reg1_1__PseudoJumpSymbol1_0,
  Convert__Reg1_0__BareSymbol1_1,
  Convert__Reg1_0__regX0__SImm121_1,
  Convert__Reg1_0__ImmXLenLI1_1,
  Convert__Reg1_0__UImm20LUI1_1,
  Convert__Reg1_0__regX0__Reg1_1,
  Convert__regX0__regX0__imm_95_0,
  Convert__Reg1_0__Reg1_1__imm_95__MINUS_1,
  Convert__regX0__regX0__regX5,
  Convert__regX0__regX0__regX2,
  Convert__regX0__regX0__regX3,
  Convert__regX0__regX0__regX4,
  Convert__imm_95_1__imm_95_0,
  Convert__Reg1_2__SImm12Lsb000001_0,
  Convert__Reg1_0__imm_95_3072__regX0,
  Convert__Reg1_0__imm_95_3200__regX0,
  Convert__Reg1_0__imm_95_3074__regX0,
  Convert__Reg1_0__imm_95_3202__regX0,
  Convert__Reg1_0__imm_95_3073__regX0,
  Convert__Reg1_0__imm_95_3201__regX0,
  Convert__regX0__regX1__imm_95_0,
  Convert__Reg1_0__Reg1_1__UImm51_2,
  Convert__Reg1_0__Reg1_1__imm_95_1,
  Convert__Reg1_0__Reg1_1__regX0,
  Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__Reg1_1__SImm51_2__imm_95_0,
  Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0,
  Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2,
  Convert__Reg1_0__Reg1_1__Reg1_1__reg0,
  Convert__Reg1_0__Reg1_1__Reg1_1__RVVMaskRegOpOperand1_2,
  Convert__Reg1_0__RVVMaskRegOpOperand1_1,
  Convert__Reg1_0__Reg1_2,
  Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4,
  Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5,
  Convert__Reg1_0__Reg1_1__SImm51_2,
  Convert__Reg1_0__Reg1_0__Reg1_0,
  Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__Reg1_4__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__SImm51_1,
  Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3,
  Convert__Reg1_0__Reg1_1__regX0__reg0,
  Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2,
  Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__reg0,
  Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__RVVMaskRegOpOperand1_2,
  Convert__Reg1_0__UImm51_1__VTypeI101_2,
  Convert__Reg1_0__Reg1_1__VTypeI111_2,
  Convert__Reg1_0__Reg1_1__imm_95_255,
  CVT_NUM_SIGNATURES
};

} // end anonymous namespace

static const uint8_t ConversionTable[CVT_NUM_SIGNATURES][15] = {
  // Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_3__SImm13Lsb01_4
  { CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_Reg, 3, CVT_95_Reg, 4, CVT_95_addImmOperands, 5, CVT_Done },
  // Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_3__SImm121_4
  { CVT_95_Reg, 3, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_Reg, 4, CVT_95_addImmOperands, 5, CVT_Done },
  // Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_5__SImm121_3
  { CVT_95_Reg, 3, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_Reg, 6, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_1__InsnDirectiveOpcode1_0__SImm21Lsb0JAL1_2
  { CVT_95_Reg, 2, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm71_2__Reg1_4__Reg1_5
  { CVT_95_Reg, 4, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_addImmOperands, 3, CVT_95_Reg, 5, CVT_95_Reg, 6, CVT_Done },
  // Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm21_2__Reg1_4__Reg1_5__Reg1_6
  { CVT_95_Reg, 4, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_addImmOperands, 3, CVT_95_Reg, 5, CVT_95_Reg, 6, CVT_95_Reg, 7, CVT_Done },
  // Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_5__SImm121_3
  { CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 2, CVT_95_Reg, 3, CVT_95_Reg, 6, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_1__InsnDirectiveOpcode1_0__UImm20LUI1_2
  { CVT_95_Reg, 2, CVT_95_addImmOperands, 1, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm121_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__TPRelAddSymbol1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addImmOperands, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__RnumArg1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 3, CVT_95_Reg, 2, CVT_Done },
  // Convert__Reg1_0__UImm20AUIPC1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm13Lsb01_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__regX0__SImm13Lsb01_1
  { CVT_95_Reg, 1, CVT_regX0, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_1__Reg1_0__SImm13Lsb01_2
  { CVT_95_Reg, 2, CVT_95_Reg, 1, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__regX0__Reg1_0__SImm13Lsb01_1
  { CVT_regX0, 0, CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__Reg1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_Reg, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__ImmZero1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__SImm6NonZero1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__SImm10Lsb0000NonZero1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm10Lsb00NonZero1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__SImm61_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__SImm9Lsb01_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert_NoOperands
  { CVT_Done },
  // Convert__Reg1_0__Reg1_2__imm_95_0
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1
  { CVT_95_Reg, 1, CVT_95_Reg, 4, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1
  { CVT_95_Reg, 1, CVT_95_Reg, 4, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_3__UImm7Lsb001_1
  { CVT_95_Reg, 1, CVT_95_Reg, 4, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_3__UImm8Lsb001_1
  { CVT_95_Reg, 1, CVT_95_Reg, 4, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__SImm12Lsb01_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0
  { CVT_95_Reg, 1, CVT_Done },
  // Convert__Reg1_0__SImm61_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__CLUIImm1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__SImm6NonZero1_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__regX0__Tie0_1_1__regX5
  { CVT_regX0, 0, CVT_Tied, Tie0_1_1, CVT_regX5, 0, CVT_Done },
  // Convert__regX0__Tie0_1_1__regX2
  { CVT_regX0, 0, CVT_Tied, Tie0_1_1, CVT_regX2, 0, CVT_Done },
  // Convert__regX0__Tie0_1_1__regX3
  { CVT_regX0, 0, CVT_Tied, Tie0_1_1, CVT_regX3, 0, CVT_Done },
  // Convert__regX0__Tie0_1_1__regX4
  { CVT_regX0, 0, CVT_Tied, Tie0_1_1, CVT_regX4, 0, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Tie0_1_1
  { CVT_95_Reg, 1, CVT_Tied, Tie0_1_1, CVT_Done },
  // Convert__CallSymbol1_0
  { CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__CallSymbol1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__ZeroOffsetMemOpOperand1_0
  { CVT_95_addRegOperands, 1, CVT_Done },
  // Convert__regX0__CSRSystemRegister1_0__Reg1_1
  { CVT_regX0, 0, CVT_95_addCSRSystemRegisterOperands, 1, CVT_95_Reg, 2, CVT_Done },
  // Convert__regX0__CSRSystemRegister1_0__UImm51_1
  { CVT_regX0, 0, CVT_95_addCSRSystemRegisterOperands, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__CSRSystemRegister1_1__regX0
  { CVT_95_Reg, 1, CVT_95_addCSRSystemRegisterOperands, 2, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__CSRSystemRegister1_1__Reg1_2
  { CVT_95_Reg, 1, CVT_95_addCSRSystemRegisterOperands, 2, CVT_95_Reg, 3, CVT_Done },
  // Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2
  { CVT_95_Reg, 1, CVT_95_addCSRSystemRegisterOperands, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__imm_95_0__imm_95_0
  { CVT_imm_95_0, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_1
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 2, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addFRMArgOperands, 4, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addFRMArgOperands, 4, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addFRMArgOperands, 4, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addFRMArgOperands, 4, CVT_Done },
  // Convert__Reg1_0__GPRF64AsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRPF64AsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRAsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRAsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95_7
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRF64AsFPR1_0__Reg1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__FRMArg1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRF64AsFPR1_0__Reg1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRAsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__GPRF64AsFPR1_0__Reg1_1
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__Reg1_1
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRAsFPR1_0__Reg1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_Reg, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__Reg1_0__GPRAsFPR1_1__imm_95_7
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRPF64AsFPR1_1__imm_95_7
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__GPRPF64AsFPR1_1__FRMArg1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__imm_95_15__imm_95_15
  { CVT_imm_95_15, 0, CVT_imm_95_15, 0, CVT_Done },
  // Convert__FenceArg1_0__FenceArg1_1
  { CVT_95_addFenceArgOperands, 1, CVT_95_addFenceArgOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__Reg1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_2__Reg1_1
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_95_Reg, 2, CVT_Done },
  // Convert__Reg1_0__GPRF64AsFPR1_2__GPRF64AsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRPF64AsFPR1_2__GPRPF64AsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__Reg1_2__Reg1_0__BareSymbol1_1
  { CVT_95_Reg, 3, CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_3__SImm121_1
  { CVT_95_Reg, 1, CVT_95_Reg, 4, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_Reg, 4, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_imm_95_7, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_Reg, 4, CVT_95_addFRMArgOperands, 5, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_95_addFRMArgOperands, 5, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_95_addFRMArgOperands, 5, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_95_addRegOperands, 4, CVT_95_addFRMArgOperands, 5, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addRegOperands, 3, CVT_Done },
  // Convert__Reg1_0__imm_95_3__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_1__regX0
  { CVT_95_Reg, 1, CVT_imm_95_1, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_2__regX0
  { CVT_95_Reg, 1, CVT_imm_95_2, 0, CVT_regX0, 0, CVT_Done },
  // Convert__regX0__imm_95_3__Reg1_0
  { CVT_regX0, 0, CVT_imm_95_3, 0, CVT_95_Reg, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_3__Reg1_1
  { CVT_95_Reg, 1, CVT_imm_95_3, 0, CVT_95_Reg, 2, CVT_Done },
  // Convert__regX0__imm_95_1__Reg1_0
  { CVT_regX0, 0, CVT_imm_95_1, 0, CVT_95_Reg, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_1__Reg1_1
  { CVT_95_Reg, 1, CVT_imm_95_1, 0, CVT_95_Reg, 2, CVT_Done },
  // Convert__regX0__imm_95_1__UImm51_0
  { CVT_regX0, 0, CVT_imm_95_1, 0, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_1__UImm51_1
  { CVT_95_Reg, 1, CVT_imm_95_1, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__imm_95_7
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_imm_95_7, 0, CVT_Done },
  // Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2
  { CVT_95_addRegOperands, 1, CVT_95_addRegOperands, 2, CVT_95_addFRMArgOperands, 3, CVT_Done },
  // Convert__regX0__imm_95_2__Reg1_0
  { CVT_regX0, 0, CVT_imm_95_2, 0, CVT_95_Reg, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_2__Reg1_1
  { CVT_95_Reg, 1, CVT_imm_95_2, 0, CVT_95_Reg, 2, CVT_Done },
  // Convert__regX0__imm_95_2__UImm51_0
  { CVT_regX0, 0, CVT_imm_95_2, 0, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_2__UImm51_1
  { CVT_95_Reg, 1, CVT_imm_95_2, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__regX0__regX0
  { CVT_regX0, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__regX0
  { CVT_95_Reg, 1, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__ZeroOffsetMemOpOperand1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands, 2, CVT_Done },
  // Convert__regX0__SImm21Lsb0JAL1_0
  { CVT_regX0, 0, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__regX1__SImm21Lsb0JAL1_0
  { CVT_regX1, 0, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__SImm21Lsb0JAL1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__regX1__Reg1_0__imm_95_0
  { CVT_regX1, 0, CVT_95_Reg, 1, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95_0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95_0, 0, CVT_Done },
  // Convert__regX1__Reg1_0__SImm121_1
  { CVT_regX1, 0, CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__regX1__Reg1_2__SImm121_0
  { CVT_regX1, 0, CVT_95_Reg, 3, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__regX0__Reg1_0__imm_95_0
  { CVT_regX0, 0, CVT_95_Reg, 1, CVT_imm_95_0, 0, CVT_Done },
  // Convert__regX0__Reg1_0__SImm121_1
  { CVT_regX0, 0, CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__regX0__Reg1_2__SImm121_0
  { CVT_regX0, 0, CVT_95_Reg, 3, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_1__PseudoJumpSymbol1_0
  { CVT_95_Reg, 2, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__BareSymbol1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__regX0__SImm121_1
  { CVT_95_Reg, 1, CVT_regX0, 0, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__ImmXLenLI1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__UImm20LUI1_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__regX0__Reg1_1
  { CVT_95_Reg, 1, CVT_regX0, 0, CVT_95_Reg, 2, CVT_Done },
  // Convert__regX0__regX0__imm_95_0
  { CVT_regX0, 0, CVT_regX0, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95__MINUS_1
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95__MINUS_1, 0, CVT_Done },
  // Convert__regX0__regX0__regX5
  { CVT_regX0, 0, CVT_regX0, 0, CVT_regX5, 0, CVT_Done },
  // Convert__regX0__regX0__regX2
  { CVT_regX0, 0, CVT_regX0, 0, CVT_regX2, 0, CVT_Done },
  // Convert__regX0__regX0__regX3
  { CVT_regX0, 0, CVT_regX0, 0, CVT_regX3, 0, CVT_Done },
  // Convert__regX0__regX0__regX4
  { CVT_regX0, 0, CVT_regX0, 0, CVT_regX4, 0, CVT_Done },
  // Convert__imm_95_1__imm_95_0
  { CVT_imm_95_1, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_2__SImm12Lsb000001_0
  { CVT_95_Reg, 3, CVT_95_addImmOperands, 1, CVT_Done },
  // Convert__Reg1_0__imm_95_3072__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3072, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_3200__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3200, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_3074__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3074, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_3202__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3202, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_3073__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3073, 0, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__imm_95_3201__regX0
  { CVT_95_Reg, 1, CVT_imm_95_3201, 0, CVT_regX0, 0, CVT_Done },
  // Convert__regX0__regX1__imm_95_0
  { CVT_regX0, 0, CVT_regX1, 0, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm51_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95_1
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95_1, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__regX0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_regX0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm51_2__imm_95_0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_imm_95_0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addRegOperands_95_defaultMaskRegOp, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_1__reg0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 2, CVT_reg0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__Reg1_1__RVVMaskRegOpOperand1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_Reg, 2, CVT_95_addRegOperands_95_defaultMaskRegOp, 3, CVT_Done },
  // Convert__Reg1_0__RVVMaskRegOpOperand1_1
  { CVT_95_Reg, 1, CVT_95_addRegOperands_95_defaultMaskRegOp, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 5, CVT_Done },
  // Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_95_Reg, 5, CVT_95_addRegOperands_95_defaultMaskRegOp, 6, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm51_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_0__Reg1_0
  { CVT_95_Reg, 1, CVT_95_Reg, 1, CVT_95_Reg, 1, CVT_Done },
  // Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 3, CVT_95_Reg, 2, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_4__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 5, CVT_95_Reg, 2, CVT_95_Reg, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__SImm51_1
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_Done },
  // Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addImmOperands, 3, CVT_95_addRegOperands_95_defaultMaskRegOp, 4, CVT_Done },
  // Convert__Reg1_0__Reg1_1__regX0__reg0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_regX0, 0, CVT_reg0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_regX0, 0, CVT_95_addRegOperands_95_defaultMaskRegOp, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__reg0
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95__MINUS_1, 0, CVT_reg0, 0, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__RVVMaskRegOpOperand1_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95__MINUS_1, 0, CVT_95_addRegOperands_95_defaultMaskRegOp, 3, CVT_Done },
  // Convert__Reg1_0__UImm51_1__VTypeI101_2
  { CVT_95_Reg, 1, CVT_95_addImmOperands, 2, CVT_95_addVTypeIOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__VTypeI111_2
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_95_addVTypeIOperands, 3, CVT_Done },
  // Convert__Reg1_0__Reg1_1__imm_95_255
  { CVT_95_Reg, 1, CVT_95_Reg, 2, CVT_imm_95_255, 0, CVT_Done },
};

void RISCVAsmParser::
convertToMCInst(unsigned Kind, MCInst &Inst, unsigned Opcode,
                const OperandVector &Operands,
                const SmallBitVector &OptionalOperandsMask) {
  assert(Kind < CVT_NUM_SIGNATURES && "Invalid signature!");
  const uint8_t *Converter = ConversionTable[Kind];
  unsigned DefaultsOffset[8] = { 0 };
  assert(OptionalOperandsMask.size() == 7);
  for (unsigned i = 0, NumDefaults = 0; i < 7; ++i) {
    DefaultsOffset[i + 1] = NumDefaults;
    NumDefaults += (OptionalOperandsMask[i] ? 1 : 0);
  }
  unsigned OpIdx;
  Inst.setOpcode(Opcode);
  for (const uint8_t *p = Converter; *p; p += 2) {
    OpIdx = *(p + 1) - DefaultsOffset[*(p + 1)];
    switch (*p) {
    default: llvm_unreachable("invalid conversion entry!");
    case CVT_Reg:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
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
    case CVT_95_addImmOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addImmOperands(Inst, 1);
      break;
    case CVT_95_Reg:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      break;
    case CVT_95_addRegOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      break;
    case CVT_regX0:
      Inst.addOperand(MCOperand::createReg(RISCV::X0));
      break;
    case CVT_imm_95_0:
      Inst.addOperand(MCOperand::createImm(0));
      break;
    case CVT_regX5:
      Inst.addOperand(MCOperand::createReg(RISCV::X5));
      break;
    case CVT_regX2:
      Inst.addOperand(MCOperand::createReg(RISCV::X2));
      break;
    case CVT_regX3:
      Inst.addOperand(MCOperand::createReg(RISCV::X3));
      break;
    case CVT_regX4:
      Inst.addOperand(MCOperand::createReg(RISCV::X4));
      break;
    case CVT_95_addCSRSystemRegisterOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addCSRSystemRegisterOperands(Inst, 1);
      break;
    case CVT_imm_95_7:
      Inst.addOperand(MCOperand::createImm(7));
      break;
    case CVT_95_addFRMArgOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addFRMArgOperands(Inst, 1);
      break;
    case CVT_imm_95_15:
      Inst.addOperand(MCOperand::createImm(15));
      break;
    case CVT_95_addFenceArgOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addFenceArgOperands(Inst, 1);
      break;
    case CVT_imm_95_3:
      Inst.addOperand(MCOperand::createImm(3));
      break;
    case CVT_imm_95_1:
      Inst.addOperand(MCOperand::createImm(1));
      break;
    case CVT_imm_95_2:
      Inst.addOperand(MCOperand::createImm(2));
      break;
    case CVT_regX1:
      Inst.addOperand(MCOperand::createReg(RISCV::X1));
      break;
    case CVT_imm_95__MINUS_1:
      Inst.addOperand(MCOperand::createImm(-1));
      break;
    case CVT_imm_95_3072:
      Inst.addOperand(MCOperand::createImm(3072));
      break;
    case CVT_imm_95_3200:
      Inst.addOperand(MCOperand::createImm(3200));
      break;
    case CVT_imm_95_3074:
      Inst.addOperand(MCOperand::createImm(3074));
      break;
    case CVT_imm_95_3202:
      Inst.addOperand(MCOperand::createImm(3202));
      break;
    case CVT_imm_95_3073:
      Inst.addOperand(MCOperand::createImm(3073));
      break;
    case CVT_imm_95_3201:
      Inst.addOperand(MCOperand::createImm(3201));
      break;
    case CVT_95_addRegOperands_95_defaultMaskRegOp:
      if (OptionalOperandsMask[*(p + 1) - 1]) {
        defaultMaskRegOp()->addRegOperands(Inst, 1);
      } else {
        static_cast<RISCVOperand &>(*Operands[OpIdx]).addRegOperands(Inst, 1);
      }
      break;
    case CVT_reg0:
      Inst.addOperand(MCOperand::createReg(0));
      break;
    case CVT_95_addVTypeIOperands:
      static_cast<RISCVOperand &>(*Operands[OpIdx]).addVTypeIOperands(Inst, 1);
      break;
    case CVT_imm_95_255:
      Inst.addOperand(MCOperand::createImm(255));
      break;
    }
  }
}

void RISCVAsmParser::
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
    case CVT_95_addImmOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_95_Reg:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("r");
      NumMCOperands += 1;
      break;
    case CVT_95_addRegOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_regX0:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_imm_95_0:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_regX5:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_regX2:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_regX3:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_regX4:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_95_addCSRSystemRegisterOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_imm_95_7:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_95_addFRMArgOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_imm_95_15:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_95_addFenceArgOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_imm_95_3:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_1:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_2:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_regX1:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_imm_95__MINUS_1:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3072:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3200:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3074:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3202:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3073:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_imm_95_3201:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
      ++NumMCOperands;
      break;
    case CVT_95_addRegOperands_95_defaultMaskRegOp:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_reg0:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      ++NumMCOperands;
      break;
    case CVT_95_addVTypeIOperands:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("m");
      NumMCOperands += 1;
      break;
    case CVT_imm_95_255:
      Operands[*(p + 1)]->setMCOperandNum(NumMCOperands);
      Operands[*(p + 1)]->setConstraint("");
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
  MCK__40_, // '('
  MCK__41_, // ')'
  MCK_LAST_TOKEN = MCK__41_,
  MCK_Reg69, // derived register class
  MCK_Reg66, // derived register class
  MCK_Reg63, // derived register class
  MCK_Reg60, // derived register class
  MCK_Reg57, // derived register class
  MCK_Reg54, // derived register class
  MCK_Reg51, // derived register class
  MCK_Reg48, // derived register class
  MCK_Reg45, // derived register class
  MCK_Reg42, // derived register class
  MCK_Reg39, // derived register class
  MCK_Reg30, // derived register class
  MCK_Reg28, // derived register class
  MCK_Reg24, // derived register class
  MCK_Reg21, // derived register class
  MCK_Reg18, // derived register class
  MCK_GPRX0, // register class 'GPRX0'
  MCK_SP, // register class 'SP'
  MCK_VMV0, // register class 'VMV0,V0'
  MCK_Reg36, // derived register class
  MCK_VCSR, // register class 'VCSR'
  MCK_VRM8NoV0, // register class 'VRM8NoV0'
  MCK_Reg35, // derived register class
  MCK_VRM8, // register class 'VRM8'
  MCK_Reg9, // derived register class
  MCK_VRN2M4NoV0, // register class 'VRN2M4NoV0'
  MCK_Reg34, // derived register class
  MCK_VRM4NoV0, // register class 'VRM4NoV0'
  MCK_VRN2M4, // register class 'VRN2M4'
  MCK_FPR32C, // register class 'FPR32C'
  MCK_FPR64C, // register class 'FPR64C'
  MCK_GPRC, // register class 'GPRC'
  MCK_VRM4, // register class 'VRM4'
  MCK_VRN4M2NoV0, // register class 'VRN4M2NoV0'
  MCK_Reg33, // derived register class
  MCK_VRN3M2NoV0, // register class 'VRN3M2NoV0'
  MCK_VRN4M2, // register class 'VRN4M2'
  MCK_Reg32, // derived register class
  MCK_GPRTC, // register class 'GPRTC'
  MCK_VRN2M2NoV0, // register class 'VRN2M2NoV0'
  MCK_VRN3M2, // register class 'VRN3M2'
  MCK_Reg31, // derived register class
  MCK_VRM2NoV0, // register class 'VRM2NoV0'
  MCK_VRN2M2, // register class 'VRN2M2'
  MCK_GPRPF64, // register class 'GPRPF64'
  MCK_VRM2, // register class 'VRM2'
  MCK_VRN8M1NoV0, // register class 'VRN8M1NoV0'
  MCK_VRN7M1NoV0, // register class 'VRN7M1NoV0'
  MCK_VRN8M1, // register class 'VRN8M1'
  MCK_GPRJALR, // register class 'GPRJALR'
  MCK_VRN6M1NoV0, // register class 'VRN6M1NoV0'
  MCK_VRN7M1, // register class 'VRN7M1'
  MCK_VRN5M1NoV0, // register class 'VRN5M1NoV0'
  MCK_VRN6M1, // register class 'VRN6M1'
  MCK_VRN4M1NoV0, // register class 'VRN4M1NoV0'
  MCK_VRN5M1, // register class 'VRN5M1'
  MCK_VRN3M1NoV0, // register class 'VRN3M1NoV0'
  MCK_VRN4M1, // register class 'VRN4M1'
  MCK_GPRNoX0X2, // register class 'GPRNoX0X2'
  MCK_VRN2M1NoV0, // register class 'VRN2M1NoV0'
  MCK_VRN3M1, // register class 'VRN3M1'
  MCK_GPRNoX0, // register class 'GPRNoX0'
  MCK_VRN2M1, // register class 'VRN2M1'
  MCK_VRNoV0, // register class 'VRNoV0'
  MCK_FPR16, // register class 'FPR16'
  MCK_FPR32, // register class 'FPR32'
  MCK_FPR64, // register class 'FPR64'
  MCK_GPR, // register class 'GPR,GPRF16,GPRF32,GPRF64'
  MCK_VM, // register class 'VM,VR'
  MCK_AnyReg, // register class 'AnyReg'
  MCK_LAST_REGISTER = MCK_AnyReg,
  MCK_BareSymbol, // user defined class 'BareSymbol'
  MCK_CLUIImm, // user defined class 'CLUIImmAsmOperand'
  MCK_CSRSystemRegister, // user defined class 'CSRSystemRegister'
  MCK_CallSymbol, // user defined class 'CallSymbol'
  MCK_FRMArg, // user defined class 'FRMArg'
  MCK_FenceArg, // user defined class 'FenceArg'
  MCK_GPRAsFPR, // user defined class 'GPRAsFPR'
  MCK_GPRF64AsFPR, // user defined class 'GPRF64AsFPR'
  MCK_GPRPF64AsFPR, // user defined class 'GPRPF64AsFPR'
  MCK_Imm, // user defined class 'ImmAsmOperand'
  MCK_ImmZero, // user defined class 'ImmZeroAsmOperand'
  MCK_InsnDirectiveOpcode, // user defined class 'InsnDirectiveOpcode'
  MCK_PseudoJumpSymbol, // user defined class 'PseudoJumpSymbol'
  MCK_RnumArg, // user defined class 'RnumArg'
  MCK_SImm5Plus1, // user defined class 'SImm5Plus1AsmOperand'
  MCK_SImm21Lsb0JAL, // user defined class 'Simm21Lsb0JALAsmOperand'
  MCK_TPRelAddSymbol, // user defined class 'TPRelAddSymbol'
  MCK_UImmLog2XLen, // user defined class 'UImmLog2XLenAsmOperand'
  MCK_UImmLog2XLenHalf, // user defined class 'UImmLog2XLenHalfAsmOperand'
  MCK_UImmLog2XLenNonZero, // user defined class 'UImmLog2XLenNonZeroAsmOperand'
  MCK_RVVMaskRegOpOperand, // user defined class 'VMaskAsmOperand'
  MCK_ZeroOffsetMemOpOperand, // user defined class 'ZeroOffsetMemOpOperand'
  MCK_UImm2, // user defined class 'anonymous_4648'
  MCK_UImm3, // user defined class 'anonymous_4649'
  MCK_UImm5, // user defined class 'anonymous_4650'
  MCK_UImm7, // user defined class 'anonymous_4651'
  MCK_SImm12, // user defined class 'anonymous_4652'
  MCK_SImm13Lsb0, // user defined class 'anonymous_4653'
  MCK_UImm20LUI, // user defined class 'anonymous_4654'
  MCK_UImm20AUIPC, // user defined class 'anonymous_4655'
  MCK_ImmXLenLI, // user defined class 'anonymous_4656'
  MCK_SImm12Lsb00000, // user defined class 'anonymous_49973'
  MCK_SImm6, // user defined class 'anonymous_5403'
  MCK_SImm6NonZero, // user defined class 'anonymous_5404'
  MCK_UImm7Lsb00, // user defined class 'anonymous_5405'
  MCK_UImm8Lsb00, // user defined class 'anonymous_5406'
  MCK_UImm8Lsb000, // user defined class 'anonymous_5407'
  MCK_SImm9Lsb0, // user defined class 'anonymous_5408'
  MCK_UImm9Lsb000, // user defined class 'anonymous_5409'
  MCK_UImm10Lsb00NonZero, // user defined class 'anonymous_5410'
  MCK_SImm10Lsb0000NonZero, // user defined class 'anonymous_5411'
  MCK_SImm12Lsb0, // user defined class 'anonymous_5412'
  MCK_VTypeI10, // user defined class 'anonymous_5639'
  MCK_VTypeI11, // user defined class 'anonymous_5640'
  MCK_SImm5, // user defined class 'anonymous_5641'
  NumMatchClassKinds
};

} // end anonymous namespace

static unsigned getDiagKindFromRegisterClass(MatchClassKind RegisterClass) {
  return MCTargetAsmParser::Match_InvalidOperand;
}

static MatchClassKind matchTokenString(StringRef Name) {
  switch (Name.size()) {
  default: break;
  case 1:	 // 2 strings to match.
    switch (Name[0]) {
    default: break;
    case '(':	 // 1 string to match.
      return MCK__40_;	 // "("
    case ')':	 // 1 string to match.
      return MCK__41_;	 // ")"
    }
    break;
  }
  return InvalidMatchClass;
}

/// isSubclass - Compute whether \p A is a subclass of \p B.
static bool isSubclass(MatchClassKind A, MatchClassKind B) {
  if (A == B)
    return true;

  switch (A) {
  default:
    return false;

  case MCK_Reg69:
    return B == MCK_VRN8M1;

  case MCK_Reg66:
    return B == MCK_VRN7M1;

  case MCK_Reg63:
    return B == MCK_VRN6M1;

  case MCK_Reg60:
    return B == MCK_VRN5M1;

  case MCK_Reg57:
    return B == MCK_VRN4M2;

  case MCK_Reg54:
    return B == MCK_VRN4M1;

  case MCK_Reg51:
    return B == MCK_VRN3M2;

  case MCK_Reg48:
    return B == MCK_VRN3M1;

  case MCK_Reg45:
    return B == MCK_VRN2M4;

  case MCK_Reg42:
    return B == MCK_VRN2M2;

  case MCK_Reg39:
    return B == MCK_VRN2M1;

  case MCK_Reg30:
    switch (B) {
    default: return false;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_Reg28:
    return B == MCK_GPRPF64;

  case MCK_Reg24:
    return B == MCK_VRM8;

  case MCK_Reg21:
    return B == MCK_VRM4;

  case MCK_Reg18:
    return B == MCK_VRM2;

  case MCK_GPRX0:
    switch (B) {
    default: return false;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_SP:
    switch (B) {
    default: return false;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VMV0:
    switch (B) {
    default: return false;
    case MCK_VM: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_Reg36:
    switch (B) {
    default: return false;
    case MCK_Reg35: return true;
    case MCK_Reg34: return true;
    case MCK_Reg33: return true;
    case MCK_Reg32: return true;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_VRM8NoV0:
    return B == MCK_VRM8;

  case MCK_Reg35:
    switch (B) {
    default: return false;
    case MCK_Reg33: return true;
    case MCK_Reg32: return true;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_Reg9:
    switch (B) {
    default: return false;
    case MCK_GPRC: return true;
    case MCK_GPRTC: return true;
    case MCK_GPRJALR: return true;
    case MCK_GPRNoX0X2: return true;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRN2M4NoV0:
    return B == MCK_VRN2M4;

  case MCK_Reg34:
    switch (B) {
    default: return false;
    case MCK_Reg33: return true;
    case MCK_Reg32: return true;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_VRM4NoV0:
    return B == MCK_VRM4;

  case MCK_FPR32C:
    return B == MCK_FPR32;

  case MCK_FPR64C:
    switch (B) {
    default: return false;
    case MCK_FPR64: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_GPRC:
    switch (B) {
    default: return false;
    case MCK_GPRJALR: return true;
    case MCK_GPRNoX0X2: return true;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRN4M2NoV0:
    return B == MCK_VRN4M2;

  case MCK_Reg33:
    switch (B) {
    default: return false;
    case MCK_Reg32: return true;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_VRN3M2NoV0:
    return B == MCK_VRN3M2;

  case MCK_Reg32:
    switch (B) {
    default: return false;
    case MCK_Reg31: return true;
    case MCK_GPRPF64: return true;
    }

  case MCK_GPRTC:
    switch (B) {
    default: return false;
    case MCK_GPRJALR: return true;
    case MCK_GPRNoX0X2: return true;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRN2M2NoV0:
    return B == MCK_VRN2M2;

  case MCK_Reg31:
    return B == MCK_GPRPF64;

  case MCK_VRM2NoV0:
    return B == MCK_VRM2;

  case MCK_VRN8M1NoV0:
    return B == MCK_VRN8M1;

  case MCK_VRN7M1NoV0:
    return B == MCK_VRN7M1;

  case MCK_GPRJALR:
    switch (B) {
    default: return false;
    case MCK_GPRNoX0X2: return true;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRN6M1NoV0:
    return B == MCK_VRN6M1;

  case MCK_VRN5M1NoV0:
    return B == MCK_VRN5M1;

  case MCK_VRN4M1NoV0:
    return B == MCK_VRN4M1;

  case MCK_VRN3M1NoV0:
    return B == MCK_VRN3M1;

  case MCK_GPRNoX0X2:
    switch (B) {
    default: return false;
    case MCK_GPRNoX0: return true;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRN2M1NoV0:
    return B == MCK_VRN2M1;

  case MCK_GPRNoX0:
    switch (B) {
    default: return false;
    case MCK_GPR: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_VRNoV0:
    switch (B) {
    default: return false;
    case MCK_VM: return true;
    case MCK_AnyReg: return true;
    }

  case MCK_FPR64:
    return B == MCK_AnyReg;

  case MCK_GPR:
    return B == MCK_AnyReg;

  case MCK_VM:
    return B == MCK_AnyReg;

  case MCK_RVVMaskRegOpOperand:
    return B == OptionalMatchClass;
  }
}

static unsigned validateOperandClass(MCParsedAsmOperand &GOp, MatchClassKind Kind) {
  RISCVOperand &Operand = (RISCVOperand &)GOp;
  if (Kind == InvalidMatchClass)
    return MCTargetAsmParser::Match_InvalidOperand;

  if (Operand.isToken() && Kind <= MCK_LAST_TOKEN)
    return isSubclass(matchTokenString(Operand.getToken()), Kind) ?
             MCTargetAsmParser::Match_Success :
             MCTargetAsmParser::Match_InvalidOperand;

  switch (Kind) {
  default: break;
  // 'BareSymbol' class
  case MCK_BareSymbol: {
    DiagnosticPredicate DP(Operand.isBareSymbol());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidBareSymbol;
    break;
    }
  // 'CLUIImm' class
  case MCK_CLUIImm: {
    DiagnosticPredicate DP(Operand.isCLUIImm());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidCLUIImm;
    break;
    }
  // 'CSRSystemRegister' class
  case MCK_CSRSystemRegister: {
    DiagnosticPredicate DP(Operand.isCSRSystemRegister());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidCSRSystemRegister;
    break;
    }
  // 'CallSymbol' class
  case MCK_CallSymbol: {
    DiagnosticPredicate DP(Operand.isCallSymbol());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidCallSymbol;
    break;
    }
  // 'FRMArg' class
  case MCK_FRMArg: {
    DiagnosticPredicate DP(Operand.isFRMArg());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidFRMArg;
    break;
    }
  // 'FenceArg' class
  case MCK_FenceArg: {
    DiagnosticPredicate DP(Operand.isFenceArg());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidFenceArg;
    break;
    }
  // 'GPRAsFPR' class
  case MCK_GPRAsFPR: {
    DiagnosticPredicate DP(Operand.isGPRAsFPR());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'GPRF64AsFPR' class
  case MCK_GPRF64AsFPR: {
    DiagnosticPredicate DP(Operand.isGPRF64AsFPR());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'GPRPF64AsFPR' class
  case MCK_GPRPF64AsFPR: {
    DiagnosticPredicate DP(Operand.isGPRPF64AsFPR());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'Imm' class
  case MCK_Imm: {
    DiagnosticPredicate DP(Operand.isImm());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'ImmZero' class
  case MCK_ImmZero: {
    DiagnosticPredicate DP(Operand.isImmZero());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidImmZero;
    break;
    }
  // 'InsnDirectiveOpcode' class
  case MCK_InsnDirectiveOpcode: {
    DiagnosticPredicate DP(Operand.isImm());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'PseudoJumpSymbol' class
  case MCK_PseudoJumpSymbol: {
    DiagnosticPredicate DP(Operand.isPseudoJumpSymbol());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidPseudoJumpSymbol;
    break;
    }
  // 'RnumArg' class
  case MCK_RnumArg: {
    DiagnosticPredicate DP(Operand.isRnumArg());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidRnumArg;
    break;
    }
  // 'SImm5Plus1' class
  case MCK_SImm5Plus1: {
    DiagnosticPredicate DP(Operand.isSImm5Plus1());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm5Plus1;
    break;
    }
  // 'SImm21Lsb0JAL' class
  case MCK_SImm21Lsb0JAL: {
    DiagnosticPredicate DP(Operand.isSImm21Lsb0JAL());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm21Lsb0JAL;
    break;
    }
  // 'TPRelAddSymbol' class
  case MCK_TPRelAddSymbol: {
    DiagnosticPredicate DP(Operand.isTPRelAddSymbol());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidTPRelAddSymbol;
    break;
    }
  // 'UImmLog2XLen' class
  case MCK_UImmLog2XLen: {
    DiagnosticPredicate DP(Operand.isUImmLog2XLen());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImmLog2XLen;
    break;
    }
  // 'UImmLog2XLenHalf' class
  case MCK_UImmLog2XLenHalf: {
    DiagnosticPredicate DP(Operand.isUImmLog2XLenHalf());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImmLog2XLenHalf;
    break;
    }
  // 'UImmLog2XLenNonZero' class
  case MCK_UImmLog2XLenNonZero: {
    DiagnosticPredicate DP(Operand.isUImmLog2XLenNonZero());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImmLog2XLenNonZero;
    break;
    }
  // 'RVVMaskRegOpOperand' class
  case MCK_RVVMaskRegOpOperand: {
    DiagnosticPredicate DP(Operand.isV0Reg());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidVMaskRegister;
    break;
    }
  // 'ZeroOffsetMemOpOperand' class
  case MCK_ZeroOffsetMemOpOperand: {
    DiagnosticPredicate DP(Operand.isGPR());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    break;
    }
  // 'UImm2' class
  case MCK_UImm2: {
    DiagnosticPredicate DP(Operand.isUImm2());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm2;
    break;
    }
  // 'UImm3' class
  case MCK_UImm3: {
    DiagnosticPredicate DP(Operand.isUImm3());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm3;
    break;
    }
  // 'UImm5' class
  case MCK_UImm5: {
    DiagnosticPredicate DP(Operand.isUImm5());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm5;
    break;
    }
  // 'UImm7' class
  case MCK_UImm7: {
    DiagnosticPredicate DP(Operand.isUImm7());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm7;
    break;
    }
  // 'SImm12' class
  case MCK_SImm12: {
    DiagnosticPredicate DP(Operand.isSImm12());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm12;
    break;
    }
  // 'SImm13Lsb0' class
  case MCK_SImm13Lsb0: {
    DiagnosticPredicate DP(Operand.isSImm13Lsb0());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm13Lsb0;
    break;
    }
  // 'UImm20LUI' class
  case MCK_UImm20LUI: {
    DiagnosticPredicate DP(Operand.isUImm20LUI());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm20LUI;
    break;
    }
  // 'UImm20AUIPC' class
  case MCK_UImm20AUIPC: {
    DiagnosticPredicate DP(Operand.isUImm20AUIPC());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm20AUIPC;
    break;
    }
  // 'ImmXLenLI' class
  case MCK_ImmXLenLI: {
    DiagnosticPredicate DP(Operand.isImmXLenLI());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidImmXLenLI;
    break;
    }
  // 'SImm12Lsb00000' class
  case MCK_SImm12Lsb00000: {
    DiagnosticPredicate DP(Operand.isSImm12Lsb00000());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm12Lsb00000;
    break;
    }
  // 'SImm6' class
  case MCK_SImm6: {
    DiagnosticPredicate DP(Operand.isSImm6());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm6;
    break;
    }
  // 'SImm6NonZero' class
  case MCK_SImm6NonZero: {
    DiagnosticPredicate DP(Operand.isSImm6NonZero());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm6NonZero;
    break;
    }
  // 'UImm7Lsb00' class
  case MCK_UImm7Lsb00: {
    DiagnosticPredicate DP(Operand.isUImm7Lsb00());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm7Lsb00;
    break;
    }
  // 'UImm8Lsb00' class
  case MCK_UImm8Lsb00: {
    DiagnosticPredicate DP(Operand.isUImm8Lsb00());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm8Lsb00;
    break;
    }
  // 'UImm8Lsb000' class
  case MCK_UImm8Lsb000: {
    DiagnosticPredicate DP(Operand.isUImm8Lsb000());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm8Lsb000;
    break;
    }
  // 'SImm9Lsb0' class
  case MCK_SImm9Lsb0: {
    DiagnosticPredicate DP(Operand.isSImm9Lsb0());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm9Lsb0;
    break;
    }
  // 'UImm9Lsb000' class
  case MCK_UImm9Lsb000: {
    DiagnosticPredicate DP(Operand.isUImm9Lsb000());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm9Lsb000;
    break;
    }
  // 'UImm10Lsb00NonZero' class
  case MCK_UImm10Lsb00NonZero: {
    DiagnosticPredicate DP(Operand.isUImm10Lsb00NonZero());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidUImm10Lsb00NonZero;
    break;
    }
  // 'SImm10Lsb0000NonZero' class
  case MCK_SImm10Lsb0000NonZero: {
    DiagnosticPredicate DP(Operand.isSImm10Lsb0000NonZero());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm10Lsb0000NonZero;
    break;
    }
  // 'SImm12Lsb0' class
  case MCK_SImm12Lsb0: {
    DiagnosticPredicate DP(Operand.isSImm12Lsb0());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm12Lsb0;
    break;
    }
  // 'VTypeI10' class
  case MCK_VTypeI10: {
    DiagnosticPredicate DP(Operand.isVTypeI10());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidVTypeI;
    break;
    }
  // 'VTypeI11' class
  case MCK_VTypeI11: {
    DiagnosticPredicate DP(Operand.isVTypeI11());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidVTypeI;
    break;
    }
  // 'SImm5' class
  case MCK_SImm5: {
    DiagnosticPredicate DP(Operand.isSImm5());
    if (DP.isMatch())
      return MCTargetAsmParser::Match_Success;
    if (DP.isNearMatch())
      return RISCVAsmParser::Match_InvalidSImm5;
    break;
    }
  } // end switch (Kind)

  if (Operand.isReg()) {
    MatchClassKind OpKind;
    switch (Operand.getReg()) {
    default: OpKind = InvalidMatchClass; break;
    case RISCV::X0: OpKind = MCK_GPRX0; break;
    case RISCV::X1: OpKind = MCK_GPRNoX0X2; break;
    case RISCV::X2: OpKind = MCK_SP; break;
    case RISCV::X3: OpKind = MCK_GPRNoX0X2; break;
    case RISCV::X4: OpKind = MCK_GPRNoX0X2; break;
    case RISCV::X5: OpKind = MCK_GPRNoX0X2; break;
    case RISCV::X6: OpKind = MCK_GPRTC; break;
    case RISCV::X7: OpKind = MCK_GPRTC; break;
    case RISCV::X8: OpKind = MCK_GPRC; break;
    case RISCV::X9: OpKind = MCK_GPRC; break;
    case RISCV::X10: OpKind = MCK_Reg9; break;
    case RISCV::X11: OpKind = MCK_Reg9; break;
    case RISCV::X12: OpKind = MCK_Reg9; break;
    case RISCV::X13: OpKind = MCK_Reg9; break;
    case RISCV::X14: OpKind = MCK_Reg9; break;
    case RISCV::X15: OpKind = MCK_Reg9; break;
    case RISCV::X16: OpKind = MCK_GPRTC; break;
    case RISCV::X17: OpKind = MCK_GPRTC; break;
    case RISCV::X18: OpKind = MCK_GPRJALR; break;
    case RISCV::X19: OpKind = MCK_GPRJALR; break;
    case RISCV::X20: OpKind = MCK_GPRJALR; break;
    case RISCV::X21: OpKind = MCK_GPRJALR; break;
    case RISCV::X22: OpKind = MCK_GPRJALR; break;
    case RISCV::X23: OpKind = MCK_GPRJALR; break;
    case RISCV::X24: OpKind = MCK_GPRJALR; break;
    case RISCV::X25: OpKind = MCK_GPRJALR; break;
    case RISCV::X26: OpKind = MCK_GPRJALR; break;
    case RISCV::X27: OpKind = MCK_GPRJALR; break;
    case RISCV::X28: OpKind = MCK_GPRTC; break;
    case RISCV::X29: OpKind = MCK_GPRTC; break;
    case RISCV::X30: OpKind = MCK_GPRTC; break;
    case RISCV::X31: OpKind = MCK_GPRTC; break;
    case RISCV::F0_H: OpKind = MCK_FPR16; break;
    case RISCV::F1_H: OpKind = MCK_FPR16; break;
    case RISCV::F2_H: OpKind = MCK_FPR16; break;
    case RISCV::F3_H: OpKind = MCK_FPR16; break;
    case RISCV::F4_H: OpKind = MCK_FPR16; break;
    case RISCV::F5_H: OpKind = MCK_FPR16; break;
    case RISCV::F6_H: OpKind = MCK_FPR16; break;
    case RISCV::F7_H: OpKind = MCK_FPR16; break;
    case RISCV::F8_H: OpKind = MCK_FPR16; break;
    case RISCV::F9_H: OpKind = MCK_FPR16; break;
    case RISCV::F10_H: OpKind = MCK_FPR16; break;
    case RISCV::F11_H: OpKind = MCK_FPR16; break;
    case RISCV::F12_H: OpKind = MCK_FPR16; break;
    case RISCV::F13_H: OpKind = MCK_FPR16; break;
    case RISCV::F14_H: OpKind = MCK_FPR16; break;
    case RISCV::F15_H: OpKind = MCK_FPR16; break;
    case RISCV::F16_H: OpKind = MCK_FPR16; break;
    case RISCV::F17_H: OpKind = MCK_FPR16; break;
    case RISCV::F18_H: OpKind = MCK_FPR16; break;
    case RISCV::F19_H: OpKind = MCK_FPR16; break;
    case RISCV::F20_H: OpKind = MCK_FPR16; break;
    case RISCV::F21_H: OpKind = MCK_FPR16; break;
    case RISCV::F22_H: OpKind = MCK_FPR16; break;
    case RISCV::F23_H: OpKind = MCK_FPR16; break;
    case RISCV::F24_H: OpKind = MCK_FPR16; break;
    case RISCV::F25_H: OpKind = MCK_FPR16; break;
    case RISCV::F26_H: OpKind = MCK_FPR16; break;
    case RISCV::F27_H: OpKind = MCK_FPR16; break;
    case RISCV::F28_H: OpKind = MCK_FPR16; break;
    case RISCV::F29_H: OpKind = MCK_FPR16; break;
    case RISCV::F30_H: OpKind = MCK_FPR16; break;
    case RISCV::F31_H: OpKind = MCK_FPR16; break;
    case RISCV::F0_F: OpKind = MCK_FPR32; break;
    case RISCV::F1_F: OpKind = MCK_FPR32; break;
    case RISCV::F2_F: OpKind = MCK_FPR32; break;
    case RISCV::F3_F: OpKind = MCK_FPR32; break;
    case RISCV::F4_F: OpKind = MCK_FPR32; break;
    case RISCV::F5_F: OpKind = MCK_FPR32; break;
    case RISCV::F6_F: OpKind = MCK_FPR32; break;
    case RISCV::F7_F: OpKind = MCK_FPR32; break;
    case RISCV::F8_F: OpKind = MCK_FPR32C; break;
    case RISCV::F9_F: OpKind = MCK_FPR32C; break;
    case RISCV::F10_F: OpKind = MCK_FPR32C; break;
    case RISCV::F11_F: OpKind = MCK_FPR32C; break;
    case RISCV::F12_F: OpKind = MCK_FPR32C; break;
    case RISCV::F13_F: OpKind = MCK_FPR32C; break;
    case RISCV::F14_F: OpKind = MCK_FPR32C; break;
    case RISCV::F15_F: OpKind = MCK_FPR32C; break;
    case RISCV::F16_F: OpKind = MCK_FPR32; break;
    case RISCV::F17_F: OpKind = MCK_FPR32; break;
    case RISCV::F18_F: OpKind = MCK_FPR32; break;
    case RISCV::F19_F: OpKind = MCK_FPR32; break;
    case RISCV::F20_F: OpKind = MCK_FPR32; break;
    case RISCV::F21_F: OpKind = MCK_FPR32; break;
    case RISCV::F22_F: OpKind = MCK_FPR32; break;
    case RISCV::F23_F: OpKind = MCK_FPR32; break;
    case RISCV::F24_F: OpKind = MCK_FPR32; break;
    case RISCV::F25_F: OpKind = MCK_FPR32; break;
    case RISCV::F26_F: OpKind = MCK_FPR32; break;
    case RISCV::F27_F: OpKind = MCK_FPR32; break;
    case RISCV::F28_F: OpKind = MCK_FPR32; break;
    case RISCV::F29_F: OpKind = MCK_FPR32; break;
    case RISCV::F30_F: OpKind = MCK_FPR32; break;
    case RISCV::F31_F: OpKind = MCK_FPR32; break;
    case RISCV::F0_D: OpKind = MCK_FPR64; break;
    case RISCV::F1_D: OpKind = MCK_FPR64; break;
    case RISCV::F2_D: OpKind = MCK_FPR64; break;
    case RISCV::F3_D: OpKind = MCK_FPR64; break;
    case RISCV::F4_D: OpKind = MCK_FPR64; break;
    case RISCV::F5_D: OpKind = MCK_FPR64; break;
    case RISCV::F6_D: OpKind = MCK_FPR64; break;
    case RISCV::F7_D: OpKind = MCK_FPR64; break;
    case RISCV::F8_D: OpKind = MCK_FPR64C; break;
    case RISCV::F9_D: OpKind = MCK_FPR64C; break;
    case RISCV::F10_D: OpKind = MCK_FPR64C; break;
    case RISCV::F11_D: OpKind = MCK_FPR64C; break;
    case RISCV::F12_D: OpKind = MCK_FPR64C; break;
    case RISCV::F13_D: OpKind = MCK_FPR64C; break;
    case RISCV::F14_D: OpKind = MCK_FPR64C; break;
    case RISCV::F15_D: OpKind = MCK_FPR64C; break;
    case RISCV::F16_D: OpKind = MCK_FPR64; break;
    case RISCV::F17_D: OpKind = MCK_FPR64; break;
    case RISCV::F18_D: OpKind = MCK_FPR64; break;
    case RISCV::F19_D: OpKind = MCK_FPR64; break;
    case RISCV::F20_D: OpKind = MCK_FPR64; break;
    case RISCV::F21_D: OpKind = MCK_FPR64; break;
    case RISCV::F22_D: OpKind = MCK_FPR64; break;
    case RISCV::F23_D: OpKind = MCK_FPR64; break;
    case RISCV::F24_D: OpKind = MCK_FPR64; break;
    case RISCV::F25_D: OpKind = MCK_FPR64; break;
    case RISCV::F26_D: OpKind = MCK_FPR64; break;
    case RISCV::F27_D: OpKind = MCK_FPR64; break;
    case RISCV::F28_D: OpKind = MCK_FPR64; break;
    case RISCV::F29_D: OpKind = MCK_FPR64; break;
    case RISCV::F30_D: OpKind = MCK_FPR64; break;
    case RISCV::F31_D: OpKind = MCK_FPR64; break;
    case RISCV::V0: OpKind = MCK_VMV0; break;
    case RISCV::V1: OpKind = MCK_VRNoV0; break;
    case RISCV::V2: OpKind = MCK_VRNoV0; break;
    case RISCV::V3: OpKind = MCK_VRNoV0; break;
    case RISCV::V4: OpKind = MCK_VRNoV0; break;
    case RISCV::V5: OpKind = MCK_VRNoV0; break;
    case RISCV::V6: OpKind = MCK_VRNoV0; break;
    case RISCV::V7: OpKind = MCK_VRNoV0; break;
    case RISCV::V8: OpKind = MCK_VRNoV0; break;
    case RISCV::V9: OpKind = MCK_VRNoV0; break;
    case RISCV::V10: OpKind = MCK_VRNoV0; break;
    case RISCV::V11: OpKind = MCK_VRNoV0; break;
    case RISCV::V12: OpKind = MCK_VRNoV0; break;
    case RISCV::V13: OpKind = MCK_VRNoV0; break;
    case RISCV::V14: OpKind = MCK_VRNoV0; break;
    case RISCV::V15: OpKind = MCK_VRNoV0; break;
    case RISCV::V16: OpKind = MCK_VRNoV0; break;
    case RISCV::V17: OpKind = MCK_VRNoV0; break;
    case RISCV::V18: OpKind = MCK_VRNoV0; break;
    case RISCV::V19: OpKind = MCK_VRNoV0; break;
    case RISCV::V20: OpKind = MCK_VRNoV0; break;
    case RISCV::V21: OpKind = MCK_VRNoV0; break;
    case RISCV::V22: OpKind = MCK_VRNoV0; break;
    case RISCV::V23: OpKind = MCK_VRNoV0; break;
    case RISCV::V24: OpKind = MCK_VRNoV0; break;
    case RISCV::V25: OpKind = MCK_VRNoV0; break;
    case RISCV::V26: OpKind = MCK_VRNoV0; break;
    case RISCV::V27: OpKind = MCK_VRNoV0; break;
    case RISCV::V28: OpKind = MCK_VRNoV0; break;
    case RISCV::V29: OpKind = MCK_VRNoV0; break;
    case RISCV::V30: OpKind = MCK_VRNoV0; break;
    case RISCV::V31: OpKind = MCK_VRNoV0; break;
    case RISCV::V0M2: OpKind = MCK_Reg18; break;
    case RISCV::V2M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V4M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V6M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V8M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V10M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V12M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V14M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V16M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V18M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V20M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V22M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V24M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V26M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V28M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V30M2: OpKind = MCK_VRM2NoV0; break;
    case RISCV::V0M4: OpKind = MCK_Reg21; break;
    case RISCV::V4M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V8M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V12M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V16M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V20M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V24M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V28M4: OpKind = MCK_VRM4NoV0; break;
    case RISCV::V0M8: OpKind = MCK_Reg24; break;
    case RISCV::V8M8: OpKind = MCK_VRM8NoV0; break;
    case RISCV::V16M8: OpKind = MCK_VRM8NoV0; break;
    case RISCV::V24M8: OpKind = MCK_VRM8NoV0; break;
    case RISCV::VTYPE: OpKind = MCK_VCSR; break;
    case RISCV::VL: OpKind = MCK_VCSR; break;
    case RISCV::VLENB: OpKind = MCK_VCSR; break;
    case RISCV::X0_PD: OpKind = MCK_Reg28; break;
    case RISCV::X2_PD: OpKind = MCK_Reg30; break;
    case RISCV::X4_PD: OpKind = MCK_Reg32; break;
    case RISCV::X6_PD: OpKind = MCK_Reg34; break;
    case RISCV::X8_PD: OpKind = MCK_Reg35; break;
    case RISCV::X10_PD: OpKind = MCK_Reg36; break;
    case RISCV::X12_PD: OpKind = MCK_Reg36; break;
    case RISCV::X14_PD: OpKind = MCK_Reg36; break;
    case RISCV::X16_PD: OpKind = MCK_Reg34; break;
    case RISCV::X18_PD: OpKind = MCK_Reg33; break;
    case RISCV::X20_PD: OpKind = MCK_Reg33; break;
    case RISCV::X22_PD: OpKind = MCK_Reg33; break;
    case RISCV::X24_PD: OpKind = MCK_Reg33; break;
    case RISCV::X26_PD: OpKind = MCK_Reg33; break;
    case RISCV::X28_PD: OpKind = MCK_Reg34; break;
    case RISCV::X30_PD: OpKind = MCK_Reg34; break;
    case RISCV::V8_V9: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V9_V10: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V10_V11: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V11_V12: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V12_V13: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V13_V14: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V14_V15: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V15_V16: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V16_V17: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V17_V18: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V18_V19: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V19_V20: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V20_V21: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V21_V22: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V22_V23: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V23_V24: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V24_V25: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V25_V26: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V26_V27: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V27_V28: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V28_V29: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V29_V30: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V30_V31: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V1_V2: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V2_V3: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V3_V4: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V4_V5: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V5_V6: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V6_V7: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V7_V8: OpKind = MCK_VRN2M1NoV0; break;
    case RISCV::V0_V1: OpKind = MCK_Reg39; break;
    case RISCV::V8M2_V10M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V10M2_V12M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V12M2_V14M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V14M2_V16M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V16M2_V18M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V18M2_V20M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V20M2_V22M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V22M2_V24M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V24M2_V26M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V26M2_V28M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V28M2_V30M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V2M2_V4M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V4M2_V6M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V6M2_V8M2: OpKind = MCK_VRN2M2NoV0; break;
    case RISCV::V0M2_V2M2: OpKind = MCK_Reg42; break;
    case RISCV::V8M4_V12M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V12M4_V16M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V16M4_V20M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V20M4_V24M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V24M4_V28M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V4M4_V8M4: OpKind = MCK_VRN2M4NoV0; break;
    case RISCV::V0M4_V4M4: OpKind = MCK_Reg45; break;
    case RISCV::V8_V9_V10: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V9_V10_V11: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V10_V11_V12: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V11_V12_V13: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V12_V13_V14: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V13_V14_V15: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V14_V15_V16: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V15_V16_V17: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V16_V17_V18: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V17_V18_V19: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V18_V19_V20: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V19_V20_V21: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V20_V21_V22: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V21_V22_V23: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V22_V23_V24: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V23_V24_V25: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V24_V25_V26: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V25_V26_V27: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V26_V27_V28: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V27_V28_V29: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V28_V29_V30: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V29_V30_V31: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V1_V2_V3: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V2_V3_V4: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V3_V4_V5: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V4_V5_V6: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V5_V6_V7: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V6_V7_V8: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V7_V8_V9: OpKind = MCK_VRN3M1NoV0; break;
    case RISCV::V0_V1_V2: OpKind = MCK_Reg48; break;
    case RISCV::V8M2_V10M2_V12M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V10M2_V12M2_V14M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V12M2_V14M2_V16M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V14M2_V16M2_V18M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V16M2_V18M2_V20M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V18M2_V20M2_V22M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V20M2_V22M2_V24M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V22M2_V24M2_V26M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V24M2_V26M2_V28M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V26M2_V28M2_V30M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V2M2_V4M2_V6M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V4M2_V6M2_V8M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V6M2_V8M2_V10M2: OpKind = MCK_VRN3M2NoV0; break;
    case RISCV::V0M2_V2M2_V4M2: OpKind = MCK_Reg51; break;
    case RISCV::V8_V9_V10_V11: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V9_V10_V11_V12: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V10_V11_V12_V13: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V11_V12_V13_V14: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V12_V13_V14_V15: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V13_V14_V15_V16: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V14_V15_V16_V17: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V15_V16_V17_V18: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V16_V17_V18_V19: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V17_V18_V19_V20: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V18_V19_V20_V21: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V19_V20_V21_V22: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V20_V21_V22_V23: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V21_V22_V23_V24: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V22_V23_V24_V25: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V23_V24_V25_V26: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V24_V25_V26_V27: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V25_V26_V27_V28: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V26_V27_V28_V29: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V27_V28_V29_V30: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V28_V29_V30_V31: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V1_V2_V3_V4: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V2_V3_V4_V5: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V3_V4_V5_V6: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V4_V5_V6_V7: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V5_V6_V7_V8: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V6_V7_V8_V9: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V7_V8_V9_V10: OpKind = MCK_VRN4M1NoV0; break;
    case RISCV::V0_V1_V2_V3: OpKind = MCK_Reg54; break;
    case RISCV::V8M2_V10M2_V12M2_V14M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V10M2_V12M2_V14M2_V16M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V12M2_V14M2_V16M2_V18M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V14M2_V16M2_V18M2_V20M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V16M2_V18M2_V20M2_V22M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V18M2_V20M2_V22M2_V24M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V20M2_V22M2_V24M2_V26M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V22M2_V24M2_V26M2_V28M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V24M2_V26M2_V28M2_V30M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V2M2_V4M2_V6M2_V8M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V4M2_V6M2_V8M2_V10M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V6M2_V8M2_V10M2_V12M2: OpKind = MCK_VRN4M2NoV0; break;
    case RISCV::V0M2_V2M2_V4M2_V6M2: OpKind = MCK_Reg57; break;
    case RISCV::V8_V9_V10_V11_V12: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V9_V10_V11_V12_V13: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V10_V11_V12_V13_V14: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V11_V12_V13_V14_V15: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V12_V13_V14_V15_V16: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V13_V14_V15_V16_V17: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V14_V15_V16_V17_V18: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V15_V16_V17_V18_V19: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V16_V17_V18_V19_V20: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V17_V18_V19_V20_V21: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V18_V19_V20_V21_V22: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V19_V20_V21_V22_V23: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V20_V21_V22_V23_V24: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V21_V22_V23_V24_V25: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V22_V23_V24_V25_V26: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V23_V24_V25_V26_V27: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V24_V25_V26_V27_V28: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V25_V26_V27_V28_V29: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V26_V27_V28_V29_V30: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V27_V28_V29_V30_V31: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V1_V2_V3_V4_V5: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V2_V3_V4_V5_V6: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V3_V4_V5_V6_V7: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V4_V5_V6_V7_V8: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V5_V6_V7_V8_V9: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V6_V7_V8_V9_V10: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V7_V8_V9_V10_V11: OpKind = MCK_VRN5M1NoV0; break;
    case RISCV::V0_V1_V2_V3_V4: OpKind = MCK_Reg60; break;
    case RISCV::V8_V9_V10_V11_V12_V13: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V9_V10_V11_V12_V13_V14: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V10_V11_V12_V13_V14_V15: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V11_V12_V13_V14_V15_V16: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V12_V13_V14_V15_V16_V17: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V13_V14_V15_V16_V17_V18: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V14_V15_V16_V17_V18_V19: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V15_V16_V17_V18_V19_V20: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V16_V17_V18_V19_V20_V21: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V17_V18_V19_V20_V21_V22: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V18_V19_V20_V21_V22_V23: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V19_V20_V21_V22_V23_V24: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V20_V21_V22_V23_V24_V25: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V21_V22_V23_V24_V25_V26: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V22_V23_V24_V25_V26_V27: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V23_V24_V25_V26_V27_V28: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V24_V25_V26_V27_V28_V29: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V25_V26_V27_V28_V29_V30: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V26_V27_V28_V29_V30_V31: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V1_V2_V3_V4_V5_V6: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V2_V3_V4_V5_V6_V7: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V3_V4_V5_V6_V7_V8: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V4_V5_V6_V7_V8_V9: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V5_V6_V7_V8_V9_V10: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V6_V7_V8_V9_V10_V11: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V7_V8_V9_V10_V11_V12: OpKind = MCK_VRN6M1NoV0; break;
    case RISCV::V0_V1_V2_V3_V4_V5: OpKind = MCK_Reg63; break;
    case RISCV::V8_V9_V10_V11_V12_V13_V14: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V9_V10_V11_V12_V13_V14_V15: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V10_V11_V12_V13_V14_V15_V16: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V11_V12_V13_V14_V15_V16_V17: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V12_V13_V14_V15_V16_V17_V18: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V13_V14_V15_V16_V17_V18_V19: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V14_V15_V16_V17_V18_V19_V20: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V15_V16_V17_V18_V19_V20_V21: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V16_V17_V18_V19_V20_V21_V22: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V17_V18_V19_V20_V21_V22_V23: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V18_V19_V20_V21_V22_V23_V24: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V19_V20_V21_V22_V23_V24_V25: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V20_V21_V22_V23_V24_V25_V26: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V21_V22_V23_V24_V25_V26_V27: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V22_V23_V24_V25_V26_V27_V28: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V23_V24_V25_V26_V27_V28_V29: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V24_V25_V26_V27_V28_V29_V30: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V25_V26_V27_V28_V29_V30_V31: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V1_V2_V3_V4_V5_V6_V7: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V2_V3_V4_V5_V6_V7_V8: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V3_V4_V5_V6_V7_V8_V9: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V4_V5_V6_V7_V8_V9_V10: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V5_V6_V7_V8_V9_V10_V11: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V6_V7_V8_V9_V10_V11_V12: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V7_V8_V9_V10_V11_V12_V13: OpKind = MCK_VRN7M1NoV0; break;
    case RISCV::V0_V1_V2_V3_V4_V5_V6: OpKind = MCK_Reg66; break;
    case RISCV::V8_V9_V10_V11_V12_V13_V14_V15: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V9_V10_V11_V12_V13_V14_V15_V16: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V10_V11_V12_V13_V14_V15_V16_V17: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V11_V12_V13_V14_V15_V16_V17_V18: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V12_V13_V14_V15_V16_V17_V18_V19: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V13_V14_V15_V16_V17_V18_V19_V20: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V14_V15_V16_V17_V18_V19_V20_V21: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V15_V16_V17_V18_V19_V20_V21_V22: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V16_V17_V18_V19_V20_V21_V22_V23: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V17_V18_V19_V20_V21_V22_V23_V24: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V18_V19_V20_V21_V22_V23_V24_V25: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V19_V20_V21_V22_V23_V24_V25_V26: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V20_V21_V22_V23_V24_V25_V26_V27: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V21_V22_V23_V24_V25_V26_V27_V28: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V22_V23_V24_V25_V26_V27_V28_V29: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V23_V24_V25_V26_V27_V28_V29_V30: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V24_V25_V26_V27_V28_V29_V30_V31: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V1_V2_V3_V4_V5_V6_V7_V8: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V2_V3_V4_V5_V6_V7_V8_V9: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V3_V4_V5_V6_V7_V8_V9_V10: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V4_V5_V6_V7_V8_V9_V10_V11: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V5_V6_V7_V8_V9_V10_V11_V12: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V6_V7_V8_V9_V10_V11_V12_V13: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V7_V8_V9_V10_V11_V12_V13_V14: OpKind = MCK_VRN8M1NoV0; break;
    case RISCV::V0_V1_V2_V3_V4_V5_V6_V7: OpKind = MCK_Reg69; break;
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
  case MCK__40_: return "MCK__40_";
  case MCK__41_: return "MCK__41_";
  case MCK_Reg69: return "MCK_Reg69";
  case MCK_Reg66: return "MCK_Reg66";
  case MCK_Reg63: return "MCK_Reg63";
  case MCK_Reg60: return "MCK_Reg60";
  case MCK_Reg57: return "MCK_Reg57";
  case MCK_Reg54: return "MCK_Reg54";
  case MCK_Reg51: return "MCK_Reg51";
  case MCK_Reg48: return "MCK_Reg48";
  case MCK_Reg45: return "MCK_Reg45";
  case MCK_Reg42: return "MCK_Reg42";
  case MCK_Reg39: return "MCK_Reg39";
  case MCK_Reg30: return "MCK_Reg30";
  case MCK_Reg28: return "MCK_Reg28";
  case MCK_Reg24: return "MCK_Reg24";
  case MCK_Reg21: return "MCK_Reg21";
  case MCK_Reg18: return "MCK_Reg18";
  case MCK_GPRX0: return "MCK_GPRX0";
  case MCK_SP: return "MCK_SP";
  case MCK_VMV0: return "MCK_VMV0";
  case MCK_Reg36: return "MCK_Reg36";
  case MCK_VCSR: return "MCK_VCSR";
  case MCK_VRM8NoV0: return "MCK_VRM8NoV0";
  case MCK_Reg35: return "MCK_Reg35";
  case MCK_VRM8: return "MCK_VRM8";
  case MCK_Reg9: return "MCK_Reg9";
  case MCK_VRN2M4NoV0: return "MCK_VRN2M4NoV0";
  case MCK_Reg34: return "MCK_Reg34";
  case MCK_VRM4NoV0: return "MCK_VRM4NoV0";
  case MCK_VRN2M4: return "MCK_VRN2M4";
  case MCK_FPR32C: return "MCK_FPR32C";
  case MCK_FPR64C: return "MCK_FPR64C";
  case MCK_GPRC: return "MCK_GPRC";
  case MCK_VRM4: return "MCK_VRM4";
  case MCK_VRN4M2NoV0: return "MCK_VRN4M2NoV0";
  case MCK_Reg33: return "MCK_Reg33";
  case MCK_VRN3M2NoV0: return "MCK_VRN3M2NoV0";
  case MCK_VRN4M2: return "MCK_VRN4M2";
  case MCK_Reg32: return "MCK_Reg32";
  case MCK_GPRTC: return "MCK_GPRTC";
  case MCK_VRN2M2NoV0: return "MCK_VRN2M2NoV0";
  case MCK_VRN3M2: return "MCK_VRN3M2";
  case MCK_Reg31: return "MCK_Reg31";
  case MCK_VRM2NoV0: return "MCK_VRM2NoV0";
  case MCK_VRN2M2: return "MCK_VRN2M2";
  case MCK_GPRPF64: return "MCK_GPRPF64";
  case MCK_VRM2: return "MCK_VRM2";
  case MCK_VRN8M1NoV0: return "MCK_VRN8M1NoV0";
  case MCK_VRN7M1NoV0: return "MCK_VRN7M1NoV0";
  case MCK_VRN8M1: return "MCK_VRN8M1";
  case MCK_GPRJALR: return "MCK_GPRJALR";
  case MCK_VRN6M1NoV0: return "MCK_VRN6M1NoV0";
  case MCK_VRN7M1: return "MCK_VRN7M1";
  case MCK_VRN5M1NoV0: return "MCK_VRN5M1NoV0";
  case MCK_VRN6M1: return "MCK_VRN6M1";
  case MCK_VRN4M1NoV0: return "MCK_VRN4M1NoV0";
  case MCK_VRN5M1: return "MCK_VRN5M1";
  case MCK_VRN3M1NoV0: return "MCK_VRN3M1NoV0";
  case MCK_VRN4M1: return "MCK_VRN4M1";
  case MCK_GPRNoX0X2: return "MCK_GPRNoX0X2";
  case MCK_VRN2M1NoV0: return "MCK_VRN2M1NoV0";
  case MCK_VRN3M1: return "MCK_VRN3M1";
  case MCK_GPRNoX0: return "MCK_GPRNoX0";
  case MCK_VRN2M1: return "MCK_VRN2M1";
  case MCK_VRNoV0: return "MCK_VRNoV0";
  case MCK_FPR16: return "MCK_FPR16";
  case MCK_FPR32: return "MCK_FPR32";
  case MCK_FPR64: return "MCK_FPR64";
  case MCK_GPR: return "MCK_GPR";
  case MCK_VM: return "MCK_VM";
  case MCK_AnyReg: return "MCK_AnyReg";
  case MCK_BareSymbol: return "MCK_BareSymbol";
  case MCK_CLUIImm: return "MCK_CLUIImm";
  case MCK_CSRSystemRegister: return "MCK_CSRSystemRegister";
  case MCK_CallSymbol: return "MCK_CallSymbol";
  case MCK_FRMArg: return "MCK_FRMArg";
  case MCK_FenceArg: return "MCK_FenceArg";
  case MCK_GPRAsFPR: return "MCK_GPRAsFPR";
  case MCK_GPRF64AsFPR: return "MCK_GPRF64AsFPR";
  case MCK_GPRPF64AsFPR: return "MCK_GPRPF64AsFPR";
  case MCK_Imm: return "MCK_Imm";
  case MCK_ImmZero: return "MCK_ImmZero";
  case MCK_InsnDirectiveOpcode: return "MCK_InsnDirectiveOpcode";
  case MCK_PseudoJumpSymbol: return "MCK_PseudoJumpSymbol";
  case MCK_RnumArg: return "MCK_RnumArg";
  case MCK_SImm5Plus1: return "MCK_SImm5Plus1";
  case MCK_SImm21Lsb0JAL: return "MCK_SImm21Lsb0JAL";
  case MCK_TPRelAddSymbol: return "MCK_TPRelAddSymbol";
  case MCK_UImmLog2XLen: return "MCK_UImmLog2XLen";
  case MCK_UImmLog2XLenHalf: return "MCK_UImmLog2XLenHalf";
  case MCK_UImmLog2XLenNonZero: return "MCK_UImmLog2XLenNonZero";
  case MCK_RVVMaskRegOpOperand: return "MCK_RVVMaskRegOpOperand";
  case MCK_ZeroOffsetMemOpOperand: return "MCK_ZeroOffsetMemOpOperand";
  case MCK_UImm2: return "MCK_UImm2";
  case MCK_UImm3: return "MCK_UImm3";
  case MCK_UImm5: return "MCK_UImm5";
  case MCK_UImm7: return "MCK_UImm7";
  case MCK_SImm12: return "MCK_SImm12";
  case MCK_SImm13Lsb0: return "MCK_SImm13Lsb0";
  case MCK_UImm20LUI: return "MCK_UImm20LUI";
  case MCK_UImm20AUIPC: return "MCK_UImm20AUIPC";
  case MCK_ImmXLenLI: return "MCK_ImmXLenLI";
  case MCK_SImm12Lsb00000: return "MCK_SImm12Lsb00000";
  case MCK_SImm6: return "MCK_SImm6";
  case MCK_SImm6NonZero: return "MCK_SImm6NonZero";
  case MCK_UImm7Lsb00: return "MCK_UImm7Lsb00";
  case MCK_UImm8Lsb00: return "MCK_UImm8Lsb00";
  case MCK_UImm8Lsb000: return "MCK_UImm8Lsb000";
  case MCK_SImm9Lsb0: return "MCK_SImm9Lsb0";
  case MCK_UImm9Lsb000: return "MCK_UImm9Lsb000";
  case MCK_UImm10Lsb00NonZero: return "MCK_UImm10Lsb00NonZero";
  case MCK_SImm10Lsb0000NonZero: return "MCK_SImm10Lsb0000NonZero";
  case MCK_SImm12Lsb0: return "MCK_SImm12Lsb0";
  case MCK_VTypeI10: return "MCK_VTypeI10";
  case MCK_VTypeI11: return "MCK_VTypeI11";
  case MCK_SImm5: return "MCK_SImm5";
  case NumMatchClassKinds: return "NumMatchClassKinds";
  }
  llvm_unreachable("unhandled MatchClassKind!");
}

#endif // NDEBUG
FeatureBitset RISCVAsmParser::
ComputeAvailableFeatures(const FeatureBitset &FB) const {
  FeatureBitset Features;
  if (FB[RISCV::FeatureStdExtM])
    Features.set(Feature_HasStdExtMBit);
  if (FB[RISCV::FeatureStdExtM] || FB[RISCV::FeatureStdExtZmmul])
    Features.set(Feature_HasStdExtMOrZmmulBit);
  if (FB[RISCV::FeatureStdExtA])
    Features.set(Feature_HasStdExtABit);
  if (FB[RISCV::FeatureStdExtF])
    Features.set(Feature_HasStdExtFBit);
  if (FB[RISCV::FeatureStdExtD])
    Features.set(Feature_HasStdExtDBit);
  if (FB[RISCV::FeatureStdExtH])
    Features.set(Feature_HasStdExtHBit);
  if (FB[RISCV::FeatureStdExtZihintpause])
    Features.set(Feature_HasStdExtZihintpauseBit);
  if (FB[RISCV::FeatureStdExtZihintntl])
    Features.set(Feature_HasStdExtZihintntlBit);
  if (FB[RISCV::FeatureStdExtZfhmin])
    Features.set(Feature_HasStdExtZfhminBit);
  if (FB[RISCV::FeatureStdExtZfh])
    Features.set(Feature_HasStdExtZfhBit);
  if (FB[RISCV::FeatureStdExtZfh] || FB[RISCV::FeatureStdExtZfhmin])
    Features.set(Feature_HasStdExtZfhOrZfhminBit);
  if (FB[RISCV::FeatureStdExtZfinx])
    Features.set(Feature_HasStdExtZfinxBit);
  if (FB[RISCV::FeatureStdExtZdinx])
    Features.set(Feature_HasStdExtZdinxBit);
  if (FB[RISCV::FeatureStdExtZhinxmin])
    Features.set(Feature_HasStdExtZhinxminBit);
  if (FB[RISCV::FeatureStdExtZhinx])
    Features.set(Feature_HasStdExtZhinxBit);
  if (FB[RISCV::FeatureStdExtZhinx] || FB[RISCV::FeatureStdExtZhinxmin])
    Features.set(Feature_HasStdExtZhinxOrZhinxminBit);
  if (FB[RISCV::FeatureStdExtC])
    Features.set(Feature_HasStdExtCBit);
  if (FB[RISCV::FeatureStdExtZba])
    Features.set(Feature_HasStdExtZbaBit);
  if (FB[RISCV::FeatureStdExtZbb])
    Features.set(Feature_HasStdExtZbbBit);
  if (FB[RISCV::FeatureStdExtZbc])
    Features.set(Feature_HasStdExtZbcBit);
  if (FB[RISCV::FeatureStdExtZbs])
    Features.set(Feature_HasStdExtZbsBit);
  if (FB[RISCV::FeatureStdExtZbkb])
    Features.set(Feature_HasStdExtZbkbBit);
  if (FB[RISCV::FeatureStdExtZbkx])
    Features.set(Feature_HasStdExtZbkxBit);
  if (FB[RISCV::FeatureStdExtZbb] || FB[RISCV::FeatureStdExtZbkb])
    Features.set(Feature_HasStdExtZbbOrZbkbBit);
  if (FB[RISCV::FeatureStdExtZbkc])
    Features.set(Feature_HasStdExtZbkcBit);
  if (FB[RISCV::FeatureStdExtZbc] || FB[RISCV::FeatureStdExtZbkc])
    Features.set(Feature_HasStdExtZbcOrZbkcBit);
  if (FB[RISCV::FeatureStdExtZknd])
    Features.set(Feature_HasStdExtZkndBit);
  if (FB[RISCV::FeatureStdExtZkne])
    Features.set(Feature_HasStdExtZkneBit);
  if (FB[RISCV::FeatureStdExtZknd] || FB[RISCV::FeatureStdExtZkne])
    Features.set(Feature_HasStdExtZkndOrZkneBit);
  if (FB[RISCV::FeatureStdExtZknh])
    Features.set(Feature_HasStdExtZknhBit);
  if (FB[RISCV::FeatureStdExtZksed])
    Features.set(Feature_HasStdExtZksedBit);
  if (FB[RISCV::FeatureStdExtZksh])
    Features.set(Feature_HasStdExtZkshBit);
  if (FB[RISCV::FeatureStdExtZkr])
    Features.set(Feature_HasStdExtZkrBit);
  if (FB[RISCV::FeatureStdExtC] || FB[RISCV::FeatureExtZca])
    Features.set(Feature_HasStdExtCOrZcaBit);
  if (FB[RISCV::FeatureStdExtC] || FB[RISCV::FeatureExtZcd])
    Features.set(Feature_HasStdExtCOrZcdBit);
  if (FB[RISCV::FeatureStdExtC] || FB[RISCV::FeatureExtZcf])
    Features.set(Feature_HasStdExtCOrZcfBit);
  if (!FB[RISCV::FeatureNoRVCHints])
    Features.set(Feature_HasRVCHintsBit);
  if (FB[RISCV::FeatureStdExtZve32x])
    Features.set(Feature_HasVInstructionsBit);
  if (FB[RISCV::FeatureStdExtZve64x])
    Features.set(Feature_HasVInstructionsI64Bit);
  if (FB[RISCV::FeatureStdExtZve32f])
    Features.set(Feature_HasVInstructionsAnyFBit);
  if (FB[RISCV::FeatureStdExtZicbom])
    Features.set(Feature_HasStdExtZicbomBit);
  if (FB[RISCV::FeatureStdExtZicboz])
    Features.set(Feature_HasStdExtZicbozBit);
  if (FB[RISCV::FeatureStdExtZicbop])
    Features.set(Feature_HasStdExtZicbopBit);
  if (FB[RISCV::FeatureStdExtSvinval])
    Features.set(Feature_HasStdExtSvinvalBit);
  if (FB[RISCV::FeatureStdExtZtso])
    Features.set(Feature_HasStdExtZtsoBit);
  if (FB[RISCV::FeatureStdExtZawrs])
    Features.set(Feature_HasStdExtZawrsBit);
  if (FB[RISCV::FeatureVendorXVentanaCondOps])
    Features.set(Feature_HasVendorXVentanaCondOpsBit);
  if (FB[RISCV::FeatureVendorXTHeadVdot])
    Features.set(Feature_HasVendorXTHeadVdotBit);
  if (FB[RISCV::Feature64Bit])
    Features.set(Feature_IsRV64Bit);
  if (!FB[RISCV::Feature64Bit])
    Features.set(Feature_IsRV32Bit);
  if (FB[RISCV::FeatureRV32E])
    Features.set(Feature_IsRV32EBit);
  return Features;
}

static bool checkAsmTiedOperandConstraints(const RISCVAsmParser&AsmParser,
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
    "\007.insn_b\007.insn_i\007.insn_j\007.insn_r\010.insn_r4\007.insn_s\010"
    ".insn_sb\007.insn_u\010.insn_uj\003add\006add.uw\004addi\005addiw\004ad"
    "dw\010aes32dsi\taes32dsmi\010aes32esi\taes32esmi\007aes64ds\010aes64dsm"
    "\007aes64es\010aes64esm\007aes64im\taes64ks1i\010aes64ks2\010amoadd.d\013"
    "amoadd.d.aq\015amoadd.d.aqrl\013amoadd.d.rl\010amoadd.w\013amoadd.w.aq\015"
    "amoadd.w.aqrl\013amoadd.w.rl\010amoand.d\013amoand.d.aq\015amoand.d.aqr"
    "l\013amoand.d.rl\010amoand.w\013amoand.w.aq\015amoand.w.aqrl\013amoand."
    "w.rl\010amomax.d\013amomax.d.aq\015amomax.d.aqrl\013amomax.d.rl\010amom"
    "ax.w\013amomax.w.aq\015amomax.w.aqrl\013amomax.w.rl\tamomaxu.d\014amoma"
    "xu.d.aq\016amomaxu.d.aqrl\014amomaxu.d.rl\tamomaxu.w\014amomaxu.w.aq\016"
    "amomaxu.w.aqrl\014amomaxu.w.rl\010amomin.d\013amomin.d.aq\015amomin.d.a"
    "qrl\013amomin.d.rl\010amomin.w\013amomin.w.aq\015amomin.w.aqrl\013amomi"
    "n.w.rl\tamominu.d\014amominu.d.aq\016amominu.d.aqrl\014amominu.d.rl\tam"
    "ominu.w\014amominu.w.aq\016amominu.w.aqrl\014amominu.w.rl\007amoor.d\na"
    "moor.d.aq\014amoor.d.aqrl\namoor.d.rl\007amoor.w\namoor.w.aq\014amoor.w"
    ".aqrl\namoor.w.rl\tamoswap.d\014amoswap.d.aq\016amoswap.d.aqrl\014amosw"
    "ap.d.rl\tamoswap.w\014amoswap.w.aq\016amoswap.w.aqrl\014amoswap.w.rl\010"
    "amoxor.d\013amoxor.d.aq\015amoxor.d.aqrl\013amoxor.d.rl\010amoxor.w\013"
    "amoxor.w.aq\015amoxor.w.aqrl\013amoxor.w.rl\003and\004andi\004andn\005a"
    "uipc\004bclr\005bclri\003beq\004beqz\004bext\005bexti\003bge\004bgeu\004"
    "bgez\003bgt\004bgtu\004bgtz\004binv\005binvi\003ble\004bleu\004blez\003"
    "blt\004bltu\004bltz\003bne\004bnez\005brev8\004bset\005bseti\005c.add\006"
    "c.addi\nc.addi16sp\nc.addi4spn\007c.addiw\006c.addw\005c.and\006c.andi\006"
    "c.beqz\006c.bnez\010c.ebreak\005c.fld\007c.fldsp\005c.flw\007c.flwsp\005"
    "c.fsd\007c.fsdsp\005c.fsw\007c.fswsp\003c.j\005c.jal\006c.jalr\004c.jr\004"
    "c.ld\006c.ldsp\004c.li\005c.lui\004c.lw\006c.lwsp\004c.mv\005c.nop\tc.n"
    "tl.all\010c.ntl.p1\nc.ntl.pall\010c.ntl.s1\004c.or\004c.sd\006c.sdsp\006"
    "c.slli\010c.slli64\006c.srai\010c.srai64\006c.srli\010c.srli64\005c.sub"
    "\006c.subw\004c.sw\006c.swsp\007c.unimp\005c.xor\004call\tcbo.clean\tcb"
    "o.flush\tcbo.inval\010cbo.zero\005clmul\006clmulh\006clmulr\003clz\004c"
    "lzw\004cpop\005cpopw\004csrc\005csrci\004csrr\005csrrc\006csrrci\005csr"
    "rs\006csrrsi\005csrrw\006csrrwi\004csrs\005csrsi\004csrw\005csrwi\003ct"
    "z\004ctzw\003div\004divu\005divuw\004divw\004dret\006ebreak\005ecall\006"
    "fabs.d\006fabs.h\006fabs.s\006fadd.d\006fadd.h\006fadd.s\010fclass.d\010"
    "fclass.h\010fclass.s\010fcvt.d.h\010fcvt.d.l\tfcvt.d.lu\010fcvt.d.s\010"
    "fcvt.d.w\tfcvt.d.wu\010fcvt.h.d\010fcvt.h.l\tfcvt.h.lu\010fcvt.h.s\010f"
    "cvt.h.w\tfcvt.h.wu\010fcvt.l.d\010fcvt.l.h\010fcvt.l.s\tfcvt.lu.d\tfcvt"
    ".lu.h\tfcvt.lu.s\010fcvt.s.d\010fcvt.s.h\010fcvt.s.l\tfcvt.s.lu\010fcvt"
    ".s.w\tfcvt.s.wu\010fcvt.w.d\010fcvt.w.h\010fcvt.w.s\tfcvt.wu.d\tfcvt.wu"
    ".h\tfcvt.wu.s\006fdiv.d\006fdiv.h\006fdiv.s\005fence\007fence.i\tfence."
    "tso\005feq.d\005feq.h\005feq.s\005fge.d\005fge.h\005fge.s\005fgt.d\005f"
    "gt.h\005fgt.s\003fld\005fle.d\005fle.h\005fle.s\003flh\005flt.d\005flt."
    "h\005flt.s\003flw\007fmadd.d\007fmadd.h\007fmadd.s\006fmax.d\006fmax.h\006"
    "fmax.s\006fmin.d\006fmin.h\006fmin.s\007fmsub.d\007fmsub.h\007fmsub.s\006"
    "fmul.d\006fmul.h\006fmul.s\005fmv.d\007fmv.d.x\005fmv.h\007fmv.h.x\005f"
    "mv.s\007fmv.w.x\007fmv.x.d\007fmv.x.h\007fmv.x.w\006fneg.d\006fneg.h\006"
    "fneg.s\010fnmadd.d\010fnmadd.h\010fnmadd.s\010fnmsub.d\010fnmsub.h\010f"
    "nmsub.s\005frcsr\007frflags\004frrm\004frsr\005fscsr\003fsd\007fsflags\010"
    "fsflagsi\007fsgnj.d\007fsgnj.h\007fsgnj.s\010fsgnjn.d\010fsgnjn.h\010fs"
    "gnjn.s\010fsgnjx.d\010fsgnjx.h\010fsgnjx.s\003fsh\007fsqrt.d\007fsqrt.h"
    "\007fsqrt.s\004fsrm\005fsrmi\004fssr\006fsub.d\006fsub.h\006fsub.s\003f"
    "sw\013hfence.gvma\013hfence.vvma\013hinval.gvma\013hinval.vvma\005hlv.b"
    "\006hlv.bu\005hlv.d\005hlv.h\006hlv.hu\005hlv.w\006hlv.wu\007hlvx.hu\007"
    "hlvx.wu\005hsv.b\005hsv.d\005hsv.h\005hsv.w\001j\003jal\004jalr\002jr\004"
    "jump\002la\tla.tls.gd\tla.tls.ie\002lb\003lbu\002ld\002lh\003lhu\002li\003"
    "lla\004lr.d\007lr.d.aq\tlr.d.aqrl\007lr.d.rl\004lr.w\007lr.w.aq\tlr.w.a"
    "qrl\007lr.w.rl\003lui\002lw\003lwu\003max\004maxu\003min\004minu\004mre"
    "t\003mul\004mulh\006mulhsu\005mulhu\004mulw\002mv\003neg\004negw\003nop"
    "\003not\007ntl.all\006ntl.p1\010ntl.pall\006ntl.s1\002or\005orc.b\003or"
    "i\003orn\004pack\005packh\005packw\005pause\nprefetch.i\nprefetch.r\npr"
    "efetch.w\007rdcycle\010rdcycleh\trdinstret\nrdinstreth\006rdtime\007rdt"
    "imeh\003rem\004remu\005remuw\004remw\003ret\004rev8\003rol\004rolw\003r"
    "or\004rori\005roriw\004rorw\002sb\004sc.d\007sc.d.aq\tsc.d.aqrl\007sc.d"
    ".rl\004sc.w\007sc.w.aq\tsc.w.aqrl\007sc.w.rl\002sd\004seqz\006sext.b\006"
    "sext.h\006sext.w\017sfence.inval.ir\nsfence.vma\016sfence.w.inval\003sg"
    "t\004sgtu\004sgtz\002sh\006sh1add\tsh1add.uw\006sh2add\tsh2add.uw\006sh"
    "3add\tsh3add.uw\nsha256sig0\nsha256sig1\nsha256sum0\nsha256sum1\nsha512"
    "sig0\013sha512sig0h\013sha512sig0l\nsha512sig1\013sha512sig1h\013sha512"
    "sig1l\nsha512sum0\013sha512sum0r\nsha512sum1\013sha512sum1r\nsinval.vma"
    "\003sll\004slli\007slli.uw\005slliw\004sllw\003slt\004slti\005sltiu\004"
    "sltu\004sltz\005sm3p0\005sm3p1\005sm4ed\005sm4ks\004snez\003sra\004srai"
    "\005sraiw\004sraw\004sret\003srl\004srli\005srliw\004srlw\003sub\004sub"
    "w\002sw\004tail\013th.vmaqa.vv\013th.vmaqa.vx\015th.vmaqasu.vv\015th.vm"
    "aqasu.vx\014th.vmaqau.vv\014th.vmaqau.vx\015th.vmaqaus.vx\005unimp\005u"
    "nzip\004uret\010vaadd.vv\010vaadd.vx\tvaaddu.vv\tvaaddu.vx\010vadc.vim\010"
    "vadc.vvm\010vadc.vxm\007vadd.vi\007vadd.vv\007vadd.vx\007vand.vi\007van"
    "d.vv\007vand.vx\010vasub.vv\010vasub.vx\tvasubu.vv\tvasubu.vx\014vcompr"
    "ess.vm\007vcpop.m\007vdiv.vv\007vdiv.vx\010vdivu.vv\010vdivu.vx\007vfab"
    "s.v\010vfadd.vf\010vfadd.vv\tvfclass.v\013vfcvt.f.x.v\014vfcvt.f.xu.v\017"
    "vfcvt.rtz.x.f.v\020vfcvt.rtz.xu.f.v\013vfcvt.x.f.v\014vfcvt.xu.f.v\010v"
    "fdiv.vf\010vfdiv.vv\010vfirst.m\tvfmacc.vf\tvfmacc.vv\tvfmadd.vf\tvfmad"
    "d.vv\010vfmax.vf\010vfmax.vv\013vfmerge.vfm\010vfmin.vf\010vfmin.vv\tvf"
    "msac.vf\tvfmsac.vv\tvfmsub.vf\tvfmsub.vv\010vfmul.vf\010vfmul.vv\010vfm"
    "v.f.s\010vfmv.s.f\010vfmv.v.f\014vfncvt.f.f.w\014vfncvt.f.x.w\015vfncvt"
    ".f.xu.w\020vfncvt.rod.f.f.w\020vfncvt.rtz.x.f.w\021vfncvt.rtz.xu.f.w\014"
    "vfncvt.x.f.w\015vfncvt.xu.f.w\007vfneg.v\nvfnmacc.vf\nvfnmacc.vv\nvfnma"
    "dd.vf\nvfnmadd.vv\nvfnmsac.vf\nvfnmsac.vv\nvfnmsub.vf\nvfnmsub.vv\tvfrd"
    "iv.vf\010vfrec7.v\013vfredmax.vs\013vfredmin.vs\014vfredosum.vs\013vfre"
    "dsum.vs\014vfredusum.vs\nvfrsqrt7.v\tvfrsub.vf\tvfsgnj.vf\tvfsgnj.vv\nv"
    "fsgnjn.vf\nvfsgnjn.vv\nvfsgnjx.vf\nvfsgnjx.vv\017vfslide1down.vf\015vfs"
    "lide1up.vf\010vfsqrt.v\010vfsub.vf\010vfsub.vv\tvfwadd.vf\tvfwadd.vv\tv"
    "fwadd.wf\tvfwadd.wv\014vfwcvt.f.f.v\014vfwcvt.f.x.v\015vfwcvt.f.xu.v\020"
    "vfwcvt.rtz.x.f.v\021vfwcvt.rtz.xu.f.v\014vfwcvt.x.f.v\015vfwcvt.xu.f.v\n"
    "vfwmacc.vf\nvfwmacc.vv\nvfwmsac.vf\nvfwmsac.vv\tvfwmul.vf\tvfwmul.vv\013"
    "vfwnmacc.vf\013vfwnmacc.vv\013vfwnmsac.vf\013vfwnmsac.vv\015vfwredosum."
    "vs\014vfwredsum.vs\015vfwredusum.vs\tvfwsub.vf\tvfwsub.vv\tvfwsub.wf\tv"
    "fwsub.wv\005vid.v\007viota.m\006vl1r.v\tvl1re16.v\tvl1re32.v\tvl1re64.v"
    "\010vl1re8.v\006vl2r.v\tvl2re16.v\tvl2re32.v\tvl2re64.v\010vl2re8.v\006"
    "vl4r.v\tvl4re16.v\tvl4re32.v\tvl4re64.v\010vl4re8.v\006vl8r.v\tvl8re16."
    "v\tvl8re32.v\tvl8re64.v\010vl8re8.v\006vle1.v\007vle16.v\tvle16ff.v\007"
    "vle32.v\tvle32ff.v\007vle64.v\tvle64ff.v\006vle8.v\010vle8ff.v\005vlm.v"
    "\nvloxei16.v\nvloxei32.v\nvloxei64.v\tvloxei8.v\016vloxseg2ei16.v\016vl"
    "oxseg2ei32.v\016vloxseg2ei64.v\015vloxseg2ei8.v\016vloxseg3ei16.v\016vl"
    "oxseg3ei32.v\016vloxseg3ei64.v\015vloxseg3ei8.v\016vloxseg4ei16.v\016vl"
    "oxseg4ei32.v\016vloxseg4ei64.v\015vloxseg4ei8.v\016vloxseg5ei16.v\016vl"
    "oxseg5ei32.v\016vloxseg5ei64.v\015vloxseg5ei8.v\016vloxseg6ei16.v\016vl"
    "oxseg6ei32.v\016vloxseg6ei64.v\015vloxseg6ei8.v\016vloxseg7ei16.v\016vl"
    "oxseg7ei32.v\016vloxseg7ei64.v\015vloxseg7ei8.v\016vloxseg8ei16.v\016vl"
    "oxseg8ei32.v\016vloxseg8ei64.v\015vloxseg8ei8.v\010vlse16.v\010vlse32.v"
    "\010vlse64.v\007vlse8.v\013vlseg2e16.v\015vlseg2e16ff.v\013vlseg2e32.v\015"
    "vlseg2e32ff.v\013vlseg2e64.v\015vlseg2e64ff.v\nvlseg2e8.v\014vlseg2e8ff"
    ".v\013vlseg3e16.v\015vlseg3e16ff.v\013vlseg3e32.v\015vlseg3e32ff.v\013v"
    "lseg3e64.v\015vlseg3e64ff.v\nvlseg3e8.v\014vlseg3e8ff.v\013vlseg4e16.v\015"
    "vlseg4e16ff.v\013vlseg4e32.v\015vlseg4e32ff.v\013vlseg4e64.v\015vlseg4e"
    "64ff.v\nvlseg4e8.v\014vlseg4e8ff.v\013vlseg5e16.v\015vlseg5e16ff.v\013v"
    "lseg5e32.v\015vlseg5e32ff.v\013vlseg5e64.v\015vlseg5e64ff.v\nvlseg5e8.v"
    "\014vlseg5e8ff.v\013vlseg6e16.v\015vlseg6e16ff.v\013vlseg6e32.v\015vlse"
    "g6e32ff.v\013vlseg6e64.v\015vlseg6e64ff.v\nvlseg6e8.v\014vlseg6e8ff.v\013"
    "vlseg7e16.v\015vlseg7e16ff.v\013vlseg7e32.v\015vlseg7e32ff.v\013vlseg7e"
    "64.v\015vlseg7e64ff.v\nvlseg7e8.v\014vlseg7e8ff.v\013vlseg8e16.v\015vls"
    "eg8e16ff.v\013vlseg8e32.v\015vlseg8e32ff.v\013vlseg8e64.v\015vlseg8e64f"
    "f.v\nvlseg8e8.v\014vlseg8e8ff.v\014vlsseg2e16.v\014vlsseg2e32.v\014vlss"
    "eg2e64.v\013vlsseg2e8.v\014vlsseg3e16.v\014vlsseg3e32.v\014vlsseg3e64.v"
    "\013vlsseg3e8.v\014vlsseg4e16.v\014vlsseg4e32.v\014vlsseg4e64.v\013vlss"
    "eg4e8.v\014vlsseg5e16.v\014vlsseg5e32.v\014vlsseg5e64.v\013vlsseg5e8.v\014"
    "vlsseg6e16.v\014vlsseg6e32.v\014vlsseg6e64.v\013vlsseg6e8.v\014vlsseg7e"
    "16.v\014vlsseg7e32.v\014vlsseg7e64.v\013vlsseg7e8.v\014vlsseg8e16.v\014"
    "vlsseg8e32.v\014vlsseg8e64.v\013vlsseg8e8.v\nvluxei16.v\nvluxei32.v\nvl"
    "uxei64.v\tvluxei8.v\016vluxseg2ei16.v\016vluxseg2ei32.v\016vluxseg2ei64"
    ".v\015vluxseg2ei8.v\016vluxseg3ei16.v\016vluxseg3ei32.v\016vluxseg3ei64"
    ".v\015vluxseg3ei8.v\016vluxseg4ei16.v\016vluxseg4ei32.v\016vluxseg4ei64"
    ".v\015vluxseg4ei8.v\016vluxseg5ei16.v\016vluxseg5ei32.v\016vluxseg5ei64"
    ".v\015vluxseg5ei8.v\016vluxseg6ei16.v\016vluxseg6ei32.v\016vluxseg6ei64"
    ".v\015vluxseg6ei8.v\016vluxseg7ei16.v\016vluxseg7ei32.v\016vluxseg7ei64"
    ".v\015vluxseg7ei8.v\016vluxseg8ei16.v\016vluxseg8ei32.v\016vluxseg8ei64"
    ".v\015vluxseg8ei8.v\010vmacc.vv\010vmacc.vx\010vmadc.vi\tvmadc.vim\010v"
    "madc.vv\tvmadc.vvm\010vmadc.vx\tvmadc.vxm\010vmadd.vv\010vmadd.vx\010vm"
    "and.mm\tvmandn.mm\013vmandnot.mm\007vmax.vv\007vmax.vx\010vmaxu.vv\010v"
    "maxu.vx\007vmclr.m\nvmerge.vim\nvmerge.vvm\nvmerge.vxm\010vmfeq.vf\010v"
    "mfeq.vv\010vmfge.vf\010vmfge.vv\010vmfgt.vf\010vmfgt.vv\010vmfle.vf\010"
    "vmfle.vv\010vmflt.vf\010vmflt.vv\010vmfne.vf\010vmfne.vv\007vmin.vv\007"
    "vmin.vx\010vminu.vv\010vminu.vx\006vmmv.m\tvmnand.mm\010vmnor.mm\007vmn"
    "ot.m\007vmor.mm\010vmorn.mm\nvmornot.mm\010vmsbc.vv\tvmsbc.vvm\010vmsbc"
    ".vx\tvmsbc.vxm\007vmsbf.m\010vmseq.vi\010vmseq.vv\010vmseq.vx\007vmset."
    "m\010vmsge.vi\010vmsge.vv\010vmsge.vx\tvmsgeu.vi\tvmsgeu.vv\tvmsgeu.vx\010"
    "vmsgt.vi\010vmsgt.vv\010vmsgt.vx\tvmsgtu.vi\tvmsgtu.vv\tvmsgtu.vx\007vm"
    "sif.m\010vmsle.vi\010vmsle.vv\010vmsle.vx\tvmsleu.vi\tvmsleu.vv\tvmsleu"
    ".vx\010vmslt.vi\010vmslt.vv\010vmslt.vx\tvmsltu.vi\tvmsltu.vv\tvmsltu.v"
    "x\010vmsne.vi\010vmsne.vv\010vmsne.vx\007vmsof.m\007vmul.vv\007vmul.vx\010"
    "vmulh.vv\010vmulh.vx\nvmulhsu.vv\nvmulhsu.vx\tvmulhu.vv\tvmulhu.vx\007v"
    "mv.s.x\007vmv.v.i\007vmv.v.v\007vmv.v.x\007vmv.x.s\007vmv1r.v\007vmv2r."
    "v\007vmv4r.v\007vmv8r.v\tvmxnor.mm\010vmxor.mm\tvnclip.wi\tvnclip.wv\tv"
    "nclip.wx\nvnclipu.wi\nvnclipu.wv\nvnclipu.wx\013vncvt.x.x.w\006vneg.v\t"
    "vnmsac.vv\tvnmsac.vx\tvnmsub.vv\tvnmsub.vx\006vnot.v\010vnsra.wi\010vns"
    "ra.wv\010vnsra.wx\010vnsrl.wi\010vnsrl.wv\010vnsrl.wx\006vor.vi\006vor."
    "vv\006vor.vx\007vpopc.m\nvredand.vs\nvredmax.vs\013vredmaxu.vs\nvredmin"
    ".vs\013vredminu.vs\tvredor.vs\nvredsum.vs\nvredxor.vs\007vrem.vv\007vre"
    "m.vx\010vremu.vv\010vremu.vx\013vrgather.vi\013vrgather.vv\013vrgather."
    "vx\017vrgatherei16.vv\010vrsub.vi\010vrsub.vx\006vs1r.v\006vs2r.v\006vs"
    "4r.v\006vs8r.v\010vsadd.vi\010vsadd.vv\010vsadd.vx\tvsaddu.vi\tvsaddu.v"
    "v\tvsaddu.vx\010vsbc.vvm\010vsbc.vxm\006vse1.v\007vse16.v\007vse32.v\007"
    "vse64.v\006vse8.v\010vsetivli\006vsetvl\007vsetvli\tvsext.vf2\tvsext.vf"
    "4\tvsext.vf8\016vslide1down.vx\014vslide1up.vx\015vslidedown.vi\015vsli"
    "dedown.vx\013vslideup.vi\013vslideup.vx\007vsll.vi\007vsll.vv\007vsll.v"
    "x\005vsm.v\010vsmul.vv\010vsmul.vx\nvsoxei16.v\nvsoxei32.v\nvsoxei64.v\t"
    "vsoxei8.v\016vsoxseg2ei16.v\016vsoxseg2ei32.v\016vsoxseg2ei64.v\015vsox"
    "seg2ei8.v\016vsoxseg3ei16.v\016vsoxseg3ei32.v\016vsoxseg3ei64.v\015vsox"
    "seg3ei8.v\016vsoxseg4ei16.v\016vsoxseg4ei32.v\016vsoxseg4ei64.v\015vsox"
    "seg4ei8.v\016vsoxseg5ei16.v\016vsoxseg5ei32.v\016vsoxseg5ei64.v\015vsox"
    "seg5ei8.v\016vsoxseg6ei16.v\016vsoxseg6ei32.v\016vsoxseg6ei64.v\015vsox"
    "seg6ei8.v\016vsoxseg7ei16.v\016vsoxseg7ei32.v\016vsoxseg7ei64.v\015vsox"
    "seg7ei8.v\016vsoxseg8ei16.v\016vsoxseg8ei32.v\016vsoxseg8ei64.v\015vsox"
    "seg8ei8.v\007vsra.vi\007vsra.vv\007vsra.vx\007vsrl.vi\007vsrl.vv\007vsr"
    "l.vx\010vsse16.v\010vsse32.v\010vsse64.v\007vsse8.v\013vsseg2e16.v\013v"
    "sseg2e32.v\013vsseg2e64.v\nvsseg2e8.v\013vsseg3e16.v\013vsseg3e32.v\013"
    "vsseg3e64.v\nvsseg3e8.v\013vsseg4e16.v\013vsseg4e32.v\013vsseg4e64.v\nv"
    "sseg4e8.v\013vsseg5e16.v\013vsseg5e32.v\013vsseg5e64.v\nvsseg5e8.v\013v"
    "sseg6e16.v\013vsseg6e32.v\013vsseg6e64.v\nvsseg6e8.v\013vsseg7e16.v\013"
    "vsseg7e32.v\013vsseg7e64.v\nvsseg7e8.v\013vsseg8e16.v\013vsseg8e32.v\013"
    "vsseg8e64.v\nvsseg8e8.v\010vssra.vi\010vssra.vv\010vssra.vx\010vssrl.vi"
    "\010vssrl.vv\010vssrl.vx\014vssseg2e16.v\014vssseg2e32.v\014vssseg2e64."
    "v\013vssseg2e8.v\014vssseg3e16.v\014vssseg3e32.v\014vssseg3e64.v\013vss"
    "seg3e8.v\014vssseg4e16.v\014vssseg4e32.v\014vssseg4e64.v\013vssseg4e8.v"
    "\014vssseg5e16.v\014vssseg5e32.v\014vssseg5e64.v\013vssseg5e8.v\014vsss"
    "eg6e16.v\014vssseg6e32.v\014vssseg6e64.v\013vssseg6e8.v\014vssseg7e16.v"
    "\014vssseg7e32.v\014vssseg7e64.v\013vssseg7e8.v\014vssseg8e16.v\014vsss"
    "eg8e32.v\014vssseg8e64.v\013vssseg8e8.v\010vssub.vv\010vssub.vx\tvssubu"
    ".vv\tvssubu.vx\007vsub.vv\007vsub.vx\nvsuxei16.v\nvsuxei32.v\nvsuxei64."
    "v\tvsuxei8.v\016vsuxseg2ei16.v\016vsuxseg2ei32.v\016vsuxseg2ei64.v\015v"
    "suxseg2ei8.v\016vsuxseg3ei16.v\016vsuxseg3ei32.v\016vsuxseg3ei64.v\015v"
    "suxseg3ei8.v\016vsuxseg4ei16.v\016vsuxseg4ei32.v\016vsuxseg4ei64.v\015v"
    "suxseg4ei8.v\016vsuxseg5ei16.v\016vsuxseg5ei32.v\016vsuxseg5ei64.v\015v"
    "suxseg5ei8.v\016vsuxseg6ei16.v\016vsuxseg6ei32.v\016vsuxseg6ei64.v\015v"
    "suxseg6ei8.v\016vsuxseg7ei16.v\016vsuxseg7ei32.v\016vsuxseg7ei64.v\015v"
    "suxseg7ei8.v\016vsuxseg8ei16.v\016vsuxseg8ei32.v\016vsuxseg8ei64.v\015v"
    "suxseg8ei8.v\010vt.maskc\tvt.maskcn\010vwadd.vv\010vwadd.vx\010vwadd.wv"
    "\010vwadd.wx\tvwaddu.vv\tvwaddu.vx\tvwaddu.wv\tvwaddu.wx\013vwcvt.x.x.v"
    "\014vwcvtu.x.x.v\tvwmacc.vv\tvwmacc.vx\013vwmaccsu.vv\013vwmaccsu.vx\nv"
    "wmaccu.vv\nvwmaccu.vx\013vwmaccus.vx\010vwmul.vv\010vwmul.vx\nvwmulsu.v"
    "v\nvwmulsu.vx\tvwmulu.vv\tvwmulu.vx\013vwredsum.vs\014vwredsumu.vs\010v"
    "wsub.vv\010vwsub.vx\010vwsub.wv\010vwsub.wx\tvwsubu.vv\tvwsubu.vx\tvwsu"
    "bu.wv\tvwsubu.wx\007vxor.vi\007vxor.vv\007vxor.vx\tvzext.vf2\tvzext.vf4"
    "\tvzext.vf8\003wfi\007wrs.nto\007wrs.sto\004xnor\003xor\004xori\006xper"
    "m4\006xperm8\006zext.b\006zext.h\006zext.w\003zip";

// Feature bitsets.
enum : uint8_t {
  AMFBS_None,
  AMFBS_HasStdExtA,
  AMFBS_HasStdExtCOrZca,
  AMFBS_HasStdExtD,
  AMFBS_HasStdExtF,
  AMFBS_HasStdExtH,
  AMFBS_HasStdExtM,
  AMFBS_HasStdExtMOrZmmul,
  AMFBS_HasStdExtSvinval,
  AMFBS_HasStdExtZawrs,
  AMFBS_HasStdExtZba,
  AMFBS_HasStdExtZbb,
  AMFBS_HasStdExtZbbOrZbkb,
  AMFBS_HasStdExtZbc,
  AMFBS_HasStdExtZbcOrZbkc,
  AMFBS_HasStdExtZbkb,
  AMFBS_HasStdExtZbkx,
  AMFBS_HasStdExtZbs,
  AMFBS_HasStdExtZfh,
  AMFBS_HasStdExtZfhOrZfhmin,
  AMFBS_HasStdExtZfinx,
  AMFBS_HasStdExtZhinx,
  AMFBS_HasStdExtZhinxOrZhinxmin,
  AMFBS_HasStdExtZicbom,
  AMFBS_HasStdExtZicbop,
  AMFBS_HasStdExtZicboz,
  AMFBS_HasStdExtZihintntl,
  AMFBS_HasStdExtZihintpause,
  AMFBS_HasStdExtZknh,
  AMFBS_HasStdExtZksed,
  AMFBS_HasStdExtZksh,
  AMFBS_HasVInstructions,
  AMFBS_HasVInstructionsAnyF,
  AMFBS_HasVInstructionsI64,
  AMFBS_HasVendorXTHeadVdot,
  AMFBS_IsRV32,
  AMFBS_IsRV64,
  AMFBS_HasStdExtA_IsRV64,
  AMFBS_HasStdExtCOrZca_HasRVCHints,
  AMFBS_HasStdExtCOrZca_IsRV32,
  AMFBS_HasStdExtCOrZca_IsRV64,
  AMFBS_HasStdExtCOrZcd_HasStdExtD,
  AMFBS_HasStdExtD_IsRV64,
  AMFBS_HasStdExtF_IsRV64,
  AMFBS_HasStdExtM_IsRV64,
  AMFBS_HasStdExtMOrZmmul_IsRV64,
  AMFBS_HasStdExtZba_IsRV64,
  AMFBS_HasStdExtZbb_IsRV32,
  AMFBS_HasStdExtZbb_IsRV64,
  AMFBS_HasStdExtZbbOrZbkb_IsRV32,
  AMFBS_HasStdExtZbbOrZbkb_IsRV64,
  AMFBS_HasStdExtZbkb_IsRV32,
  AMFBS_HasStdExtZbkb_IsRV64,
  AMFBS_HasStdExtZdinx_IsRV32,
  AMFBS_HasStdExtZdinx_IsRV64,
  AMFBS_HasStdExtZfh_IsRV64,
  AMFBS_HasStdExtZfhOrZfhmin_HasStdExtD,
  AMFBS_HasStdExtZfinx_IsRV64,
  AMFBS_HasStdExtZhinx_IsRV64,
  AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx,
  AMFBS_HasStdExtZknd_IsRV32,
  AMFBS_HasStdExtZknd_IsRV64,
  AMFBS_HasStdExtZkndOrZkne_IsRV64,
  AMFBS_HasStdExtZkne_IsRV32,
  AMFBS_HasStdExtZkne_IsRV64,
  AMFBS_HasStdExtZknh_IsRV32,
  AMFBS_HasStdExtZknh_IsRV64,
  AMFBS_HasVInstructionsI64_IsRV64,
  AMFBS_IsRV64_HasStdExtH,
  AMFBS_IsRV64_HasVInstructionsI64,
  AMFBS_IsRV64_HasVendorXVentanaCondOps,
  AMFBS_HasStdExtC_HasRVCHints_HasStdExtZihintntl,
  AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32,
};

static constexpr FeatureBitset FeatureBitsets[] = {
  {}, // AMFBS_None
  {Feature_HasStdExtABit, },
  {Feature_HasStdExtCOrZcaBit, },
  {Feature_HasStdExtDBit, },
  {Feature_HasStdExtFBit, },
  {Feature_HasStdExtHBit, },
  {Feature_HasStdExtMBit, },
  {Feature_HasStdExtMOrZmmulBit, },
  {Feature_HasStdExtSvinvalBit, },
  {Feature_HasStdExtZawrsBit, },
  {Feature_HasStdExtZbaBit, },
  {Feature_HasStdExtZbbBit, },
  {Feature_HasStdExtZbbOrZbkbBit, },
  {Feature_HasStdExtZbcBit, },
  {Feature_HasStdExtZbcOrZbkcBit, },
  {Feature_HasStdExtZbkbBit, },
  {Feature_HasStdExtZbkxBit, },
  {Feature_HasStdExtZbsBit, },
  {Feature_HasStdExtZfhBit, },
  {Feature_HasStdExtZfhOrZfhminBit, },
  {Feature_HasStdExtZfinxBit, },
  {Feature_HasStdExtZhinxBit, },
  {Feature_HasStdExtZhinxOrZhinxminBit, },
  {Feature_HasStdExtZicbomBit, },
  {Feature_HasStdExtZicbopBit, },
  {Feature_HasStdExtZicbozBit, },
  {Feature_HasStdExtZihintntlBit, },
  {Feature_HasStdExtZihintpauseBit, },
  {Feature_HasStdExtZknhBit, },
  {Feature_HasStdExtZksedBit, },
  {Feature_HasStdExtZkshBit, },
  {Feature_HasVInstructionsBit, },
  {Feature_HasVInstructionsAnyFBit, },
  {Feature_HasVInstructionsI64Bit, },
  {Feature_HasVendorXTHeadVdotBit, },
  {Feature_IsRV32Bit, },
  {Feature_IsRV64Bit, },
  {Feature_HasStdExtABit, Feature_IsRV64Bit, },
  {Feature_HasStdExtCOrZcaBit, Feature_HasRVCHintsBit, },
  {Feature_HasStdExtCOrZcaBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtCOrZcaBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtCOrZcdBit, Feature_HasStdExtDBit, },
  {Feature_HasStdExtDBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtFBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtMBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtMOrZmmulBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZbaBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZbbBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZbbBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZbbOrZbkbBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZbbOrZbkbBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZbkbBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZbkbBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZdinxBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZdinxBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZfhBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZfhOrZfhminBit, Feature_HasStdExtDBit, },
  {Feature_HasStdExtZfinxBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZhinxBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZhinxOrZhinxminBit, Feature_HasStdExtZdinxBit, },
  {Feature_HasStdExtZkndBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZkndBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZkndOrZkneBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZkneBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZkneBit, Feature_IsRV64Bit, },
  {Feature_HasStdExtZknhBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtZknhBit, Feature_IsRV64Bit, },
  {Feature_HasVInstructionsI64Bit, Feature_IsRV64Bit, },
  {Feature_IsRV64Bit, Feature_HasStdExtHBit, },
  {Feature_IsRV64Bit, Feature_HasVInstructionsI64Bit, },
  {Feature_IsRV64Bit, Feature_HasVendorXVentanaCondOpsBit, },
  {Feature_HasStdExtCBit, Feature_HasRVCHintsBit, Feature_HasStdExtZihintntlBit, },
  {Feature_HasStdExtCOrZcfBit, Feature_HasStdExtFBit, Feature_IsRV32Bit, },
};

namespace {
  struct MatchEntry {
    uint16_t Mnemonic;
    uint16_t Opcode;
    uint8_t ConvertFn;
    uint8_t RequiredFeaturesIdx;
    uint8_t Classes[7];
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
  { 0 /* .insn_b */, RISCV::InsnB, Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_3__SImm13Lsb01_4, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_AnyReg, MCK_AnyReg, MCK_SImm13Lsb0 }, },
  { 8 /* .insn_i */, RISCV::InsnI, Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_3__SImm121_4, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_AnyReg, MCK_AnyReg, MCK_SImm12 }, },
  { 8 /* .insn_i */, RISCV::InsnI_Mem, Convert__Reg1_2__InsnDirectiveOpcode1_0__UImm31_1__Reg1_5__SImm121_3, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_AnyReg, MCK_SImm12, MCK__40_, MCK_AnyReg, MCK__41_ }, },
  { 16 /* .insn_j */, RISCV::InsnJ, Convert__Reg1_1__InsnDirectiveOpcode1_0__SImm21Lsb0JAL1_2, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_AnyReg, MCK_SImm21Lsb0JAL }, },
  { 24 /* .insn_r */, RISCV::InsnR, Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm71_2__Reg1_4__Reg1_5, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_UImm7, MCK_AnyReg, MCK_AnyReg, MCK_AnyReg }, },
  { 24 /* .insn_r */, RISCV::InsnR4, Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm21_2__Reg1_4__Reg1_5__Reg1_6, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_UImm2, MCK_AnyReg, MCK_AnyReg, MCK_AnyReg, MCK_AnyReg }, },
  { 32 /* .insn_r4 */, RISCV::InsnR4, Convert__Reg1_3__InsnDirectiveOpcode1_0__UImm31_1__UImm21_2__Reg1_4__Reg1_5__Reg1_6, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_UImm2, MCK_AnyReg, MCK_AnyReg, MCK_AnyReg, MCK_AnyReg }, },
  { 41 /* .insn_s */, RISCV::InsnS, Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_5__SImm121_3, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_AnyReg, MCK_SImm12, MCK__40_, MCK_AnyReg, MCK__41_ }, },
  { 49 /* .insn_sb */, RISCV::InsnB, Convert__InsnDirectiveOpcode1_0__UImm31_1__Reg1_2__Reg1_3__SImm13Lsb01_4, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_UImm3, MCK_AnyReg, MCK_AnyReg, MCK_SImm13Lsb0 }, },
  { 58 /* .insn_u */, RISCV::InsnU, Convert__Reg1_1__InsnDirectiveOpcode1_0__UImm20LUI1_2, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_AnyReg, MCK_UImm20LUI }, },
  { 66 /* .insn_uj */, RISCV::InsnJ, Convert__Reg1_1__InsnDirectiveOpcode1_0__SImm21Lsb0JAL1_2, AMFBS_None, { MCK_InsnDirectiveOpcode, MCK_AnyReg, MCK_SImm21Lsb0JAL }, },
  { 75 /* add */, RISCV::ADD, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 75 /* add */, RISCV::ADDI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 75 /* add */, RISCV::PseudoAddTPRel, Convert__Reg1_0__Reg1_1__Reg1_2__TPRelAddSymbol1_3, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_TPRelAddSymbol }, },
  { 79 /* add.uw */, RISCV::ADD_UW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 86 /* addi */, RISCV::ADDI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 91 /* addiw */, RISCV::ADDIW, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 97 /* addw */, RISCV::ADDW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 97 /* addw */, RISCV::ADDIW, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 102 /* aes32dsi */, RISCV::AES32DSI, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZknd_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 111 /* aes32dsmi */, RISCV::AES32DSMI, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZknd_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 121 /* aes32esi */, RISCV::AES32ESI, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZkne_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 130 /* aes32esmi */, RISCV::AES32ESMI, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZkne_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 140 /* aes64ds */, RISCV::AES64DS, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknd_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 148 /* aes64dsm */, RISCV::AES64DSM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknd_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 157 /* aes64es */, RISCV::AES64ES, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZkne_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 165 /* aes64esm */, RISCV::AES64ESM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZkne_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 174 /* aes64im */, RISCV::AES64IM, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknd_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 182 /* aes64ks1i */, RISCV::AES64KS1I, Convert__Reg1_0__Reg1_1__RnumArg1_2, AMFBS_HasStdExtZkndOrZkne_IsRV64, { MCK_GPR, MCK_GPR, MCK_RnumArg }, },
  { 192 /* aes64ks2 */, RISCV::AES64KS2, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZkndOrZkne_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 201 /* amoadd.d */, RISCV::AMOADD_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 210 /* amoadd.d.aq */, RISCV::AMOADD_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 222 /* amoadd.d.aqrl */, RISCV::AMOADD_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 236 /* amoadd.d.rl */, RISCV::AMOADD_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 248 /* amoadd.w */, RISCV::AMOADD_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 257 /* amoadd.w.aq */, RISCV::AMOADD_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 269 /* amoadd.w.aqrl */, RISCV::AMOADD_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 283 /* amoadd.w.rl */, RISCV::AMOADD_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 295 /* amoand.d */, RISCV::AMOAND_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 304 /* amoand.d.aq */, RISCV::AMOAND_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 316 /* amoand.d.aqrl */, RISCV::AMOAND_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 330 /* amoand.d.rl */, RISCV::AMOAND_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 342 /* amoand.w */, RISCV::AMOAND_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 351 /* amoand.w.aq */, RISCV::AMOAND_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 363 /* amoand.w.aqrl */, RISCV::AMOAND_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 377 /* amoand.w.rl */, RISCV::AMOAND_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 389 /* amomax.d */, RISCV::AMOMAX_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 398 /* amomax.d.aq */, RISCV::AMOMAX_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 410 /* amomax.d.aqrl */, RISCV::AMOMAX_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 424 /* amomax.d.rl */, RISCV::AMOMAX_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 436 /* amomax.w */, RISCV::AMOMAX_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 445 /* amomax.w.aq */, RISCV::AMOMAX_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 457 /* amomax.w.aqrl */, RISCV::AMOMAX_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 471 /* amomax.w.rl */, RISCV::AMOMAX_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 483 /* amomaxu.d */, RISCV::AMOMAXU_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 493 /* amomaxu.d.aq */, RISCV::AMOMAXU_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 506 /* amomaxu.d.aqrl */, RISCV::AMOMAXU_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 521 /* amomaxu.d.rl */, RISCV::AMOMAXU_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 534 /* amomaxu.w */, RISCV::AMOMAXU_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 544 /* amomaxu.w.aq */, RISCV::AMOMAXU_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 557 /* amomaxu.w.aqrl */, RISCV::AMOMAXU_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 572 /* amomaxu.w.rl */, RISCV::AMOMAXU_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 585 /* amomin.d */, RISCV::AMOMIN_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 594 /* amomin.d.aq */, RISCV::AMOMIN_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 606 /* amomin.d.aqrl */, RISCV::AMOMIN_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 620 /* amomin.d.rl */, RISCV::AMOMIN_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 632 /* amomin.w */, RISCV::AMOMIN_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 641 /* amomin.w.aq */, RISCV::AMOMIN_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 653 /* amomin.w.aqrl */, RISCV::AMOMIN_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 667 /* amomin.w.rl */, RISCV::AMOMIN_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 679 /* amominu.d */, RISCV::AMOMINU_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 689 /* amominu.d.aq */, RISCV::AMOMINU_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 702 /* amominu.d.aqrl */, RISCV::AMOMINU_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 717 /* amominu.d.rl */, RISCV::AMOMINU_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 730 /* amominu.w */, RISCV::AMOMINU_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 740 /* amominu.w.aq */, RISCV::AMOMINU_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 753 /* amominu.w.aqrl */, RISCV::AMOMINU_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 768 /* amominu.w.rl */, RISCV::AMOMINU_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 781 /* amoor.d */, RISCV::AMOOR_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 789 /* amoor.d.aq */, RISCV::AMOOR_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 800 /* amoor.d.aqrl */, RISCV::AMOOR_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 813 /* amoor.d.rl */, RISCV::AMOOR_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 824 /* amoor.w */, RISCV::AMOOR_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 832 /* amoor.w.aq */, RISCV::AMOOR_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 843 /* amoor.w.aqrl */, RISCV::AMOOR_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 856 /* amoor.w.rl */, RISCV::AMOOR_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 867 /* amoswap.d */, RISCV::AMOSWAP_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 877 /* amoswap.d.aq */, RISCV::AMOSWAP_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 890 /* amoswap.d.aqrl */, RISCV::AMOSWAP_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 905 /* amoswap.d.rl */, RISCV::AMOSWAP_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 918 /* amoswap.w */, RISCV::AMOSWAP_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 928 /* amoswap.w.aq */, RISCV::AMOSWAP_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 941 /* amoswap.w.aqrl */, RISCV::AMOSWAP_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 956 /* amoswap.w.rl */, RISCV::AMOSWAP_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 969 /* amoxor.d */, RISCV::AMOXOR_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 978 /* amoxor.d.aq */, RISCV::AMOXOR_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 990 /* amoxor.d.aqrl */, RISCV::AMOXOR_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1004 /* amoxor.d.rl */, RISCV::AMOXOR_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1016 /* amoxor.w */, RISCV::AMOXOR_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1025 /* amoxor.w.aq */, RISCV::AMOXOR_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1037 /* amoxor.w.aqrl */, RISCV::AMOXOR_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1051 /* amoxor.w.rl */, RISCV::AMOXOR_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 1063 /* and */, RISCV::AND, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1063 /* and */, RISCV::ANDI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 1067 /* andi */, RISCV::ANDI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 1072 /* andn */, RISCV::ANDN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1077 /* auipc */, RISCV::AUIPC, Convert__Reg1_0__UImm20AUIPC1_1, AMFBS_None, { MCK_GPR, MCK_UImm20AUIPC }, },
  { 1083 /* bclr */, RISCV::BCLR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1083 /* bclr */, RISCV::BCLRI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1088 /* bclri */, RISCV::BCLRI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1094 /* beq */, RISCV::BEQ, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1098 /* beqz */, RISCV::BEQ, Convert__Reg1_0__regX0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1103 /* bext */, RISCV::BEXT, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1103 /* bext */, RISCV::BEXTI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1108 /* bexti */, RISCV::BEXTI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1114 /* bge */, RISCV::BGE, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1118 /* bgeu */, RISCV::BGEU, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1123 /* bgez */, RISCV::BGE, Convert__Reg1_0__regX0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1128 /* bgt */, RISCV::BLT, Convert__Reg1_1__Reg1_0__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1132 /* bgtu */, RISCV::BLTU, Convert__Reg1_1__Reg1_0__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1137 /* bgtz */, RISCV::BLT, Convert__regX0__Reg1_0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1142 /* binv */, RISCV::BINV, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1142 /* binv */, RISCV::BINVI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1147 /* binvi */, RISCV::BINVI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1153 /* ble */, RISCV::BGE, Convert__Reg1_1__Reg1_0__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1157 /* bleu */, RISCV::BGEU, Convert__Reg1_1__Reg1_0__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1162 /* blez */, RISCV::BGE, Convert__regX0__Reg1_0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1167 /* blt */, RISCV::BLT, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1171 /* bltu */, RISCV::BLTU, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1176 /* bltz */, RISCV::BLT, Convert__Reg1_0__regX0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1181 /* bne */, RISCV::BNE, Convert__Reg1_0__Reg1_1__SImm13Lsb01_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1185 /* bnez */, RISCV::BNE, Convert__Reg1_0__regX0__SImm13Lsb01_1, AMFBS_None, { MCK_GPR, MCK_SImm13Lsb0 }, },
  { 1190 /* brev8 */, RISCV::BREV8, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbkb, { MCK_GPR, MCK_GPR }, },
  { 1196 /* bset */, RISCV::BSET, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1196 /* bset */, RISCV::BSETI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1201 /* bseti */, RISCV::BSETI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbs, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 1207 /* c.add */, RISCV::C_ADD_HINT, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_GPRNoX0 }, },
  { 1207 /* c.add */, RISCV::C_ADD, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_GPRNoX0 }, },
  { 1213 /* c.addi */, RISCV::C_ADDI_NOP, Convert__Reg1_0__Tie0_1_1__ImmZero1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRX0, MCK_ImmZero }, },
  { 1213 /* c.addi */, RISCV::C_ADDI_HINT_X0, Convert__Reg1_0__Tie0_1_1__SImm6NonZero1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_SImm6NonZero }, },
  { 1213 /* c.addi */, RISCV::C_ADDI_HINT_IMM_ZERO, Convert__Reg1_0__Tie0_1_1__ImmZero1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRNoX0, MCK_ImmZero }, },
  { 1213 /* c.addi */, RISCV::C_ADDI, Convert__Reg1_0__Tie0_1_1__SImm6NonZero1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_SImm6NonZero }, },
  { 1220 /* c.addi16sp */, RISCV::C_ADDI16SP, Convert__Reg1_0__Tie0_1_1__SImm10Lsb0000NonZero1_1, AMFBS_HasStdExtCOrZca, { MCK_SP, MCK_SImm10Lsb0000NonZero }, },
  { 1231 /* c.addi4spn */, RISCV::C_ADDI4SPN, Convert__Reg1_0__Reg1_1__UImm10Lsb00NonZero1_2, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_SP, MCK_UImm10Lsb00NonZero }, },
  { 1242 /* c.addiw */, RISCV::C_ADDIW, Convert__Reg1_0__Tie0_1_1__SImm61_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRNoX0, MCK_SImm6 }, },
  { 1250 /* c.addw */, RISCV::C_ADDW, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK_GPRC }, },
  { 1257 /* c.and */, RISCV::C_AND, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_GPRC }, },
  { 1263 /* c.andi */, RISCV::C_ANDI, Convert__Reg1_0__Tie0_1_1__SImm61_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_SImm6 }, },
  { 1270 /* c.beqz */, RISCV::C_BEQZ, Convert__Reg1_0__SImm9Lsb01_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_SImm9Lsb0 }, },
  { 1277 /* c.bnez */, RISCV::C_BNEZ, Convert__Reg1_0__SImm9Lsb01_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_SImm9Lsb0 }, },
  { 1284 /* c.ebreak */, RISCV::C_EBREAK, Convert_NoOperands, AMFBS_HasStdExtCOrZca, {  }, },
  { 1293 /* c.fld */, RISCV::C_FLD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1293 /* c.fld */, RISCV::C_FLD, Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK_UImm8Lsb000, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1299 /* c.fldsp */, RISCV::C_FLDSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1299 /* c.fldsp */, RISCV::C_FLDSP, Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64, MCK_UImm9Lsb000, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1307 /* c.flw */, RISCV::C_FLW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1307 /* c.flw */, RISCV::C_FLW, Convert__Reg1_0__Reg1_3__UImm7Lsb001_1, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK_UImm7Lsb00, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1313 /* c.flwsp */, RISCV::C_FLWSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1313 /* c.flwsp */, RISCV::C_FLWSP, Convert__Reg1_0__Reg1_3__UImm8Lsb001_1, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32, MCK_UImm8Lsb00, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1321 /* c.fsd */, RISCV::C_FSD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1321 /* c.fsd */, RISCV::C_FSD, Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK_UImm8Lsb000, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1327 /* c.fsdsp */, RISCV::C_FSDSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64C, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1327 /* c.fsdsp */, RISCV::C_FSDSP, Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1, AMFBS_HasStdExtCOrZcd_HasStdExtD, { MCK_FPR64, MCK_UImm9Lsb000, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1335 /* c.fsw */, RISCV::C_FSW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1335 /* c.fsw */, RISCV::C_FSW, Convert__Reg1_0__Reg1_3__UImm7Lsb001_1, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK_UImm7Lsb00, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1341 /* c.fswsp */, RISCV::C_FSWSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32C, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1341 /* c.fswsp */, RISCV::C_FSWSP, Convert__Reg1_0__Reg1_3__UImm8Lsb001_1, AMFBS_HasStdExtCOrZcf_HasStdExtF_IsRV32, { MCK_FPR32, MCK_UImm8Lsb00, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1349 /* c.j */, RISCV::C_J, Convert__SImm12Lsb01_0, AMFBS_HasStdExtCOrZca, { MCK_SImm12Lsb0 }, },
  { 1353 /* c.jal */, RISCV::C_JAL, Convert__SImm12Lsb01_0, AMFBS_HasStdExtCOrZca_IsRV32, { MCK_SImm12Lsb0 }, },
  { 1359 /* c.jalr */, RISCV::C_JALR, Convert__Reg1_0, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0 }, },
  { 1366 /* c.jr */, RISCV::C_JR, Convert__Reg1_0, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0 }, },
  { 1371 /* c.ld */, RISCV::C_LD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1371 /* c.ld */, RISCV::C_LD, Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK_UImm8Lsb000, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1376 /* c.ldsp */, RISCV::C_LDSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1376 /* c.ldsp */, RISCV::C_LDSP, Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRNoX0, MCK_UImm9Lsb000, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1383 /* c.li */, RISCV::C_LI_HINT, Convert__Reg1_0__SImm61_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_SImm6 }, },
  { 1383 /* c.li */, RISCV::C_LI, Convert__Reg1_0__SImm61_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_SImm6 }, },
  { 1388 /* c.lui */, RISCV::C_LUI_HINT, Convert__Reg1_0__CLUIImm1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_CLUIImm }, },
  { 1388 /* c.lui */, RISCV::C_LUI, Convert__Reg1_0__CLUIImm1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0X2, MCK_CLUIImm }, },
  { 1394 /* c.lw */, RISCV::C_LW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1394 /* c.lw */, RISCV::C_LW, Convert__Reg1_0__Reg1_3__UImm7Lsb001_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_UImm7Lsb00, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1399 /* c.lwsp */, RISCV::C_LWSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1399 /* c.lwsp */, RISCV::C_LWSP, Convert__Reg1_0__Reg1_3__UImm8Lsb001_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_UImm8Lsb00, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1406 /* c.mv */, RISCV::C_MV_HINT, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_GPRNoX0 }, },
  { 1406 /* c.mv */, RISCV::C_MV, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_GPRNoX0 }, },
  { 1411 /* c.nop */, RISCV::C_NOP, Convert_NoOperands, AMFBS_HasStdExtCOrZca, {  }, },
  { 1411 /* c.nop */, RISCV::C_NOP_HINT, Convert__SImm6NonZero1_0, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_SImm6NonZero }, },
  { 1417 /* c.ntl.all */, RISCV::C_ADD_HINT, Convert__regX0__Tie0_1_1__regX5, AMFBS_HasStdExtC_HasRVCHints_HasStdExtZihintntl, {  }, },
  { 1427 /* c.ntl.p1 */, RISCV::C_ADD_HINT, Convert__regX0__Tie0_1_1__regX2, AMFBS_HasStdExtC_HasRVCHints_HasStdExtZihintntl, {  }, },
  { 1436 /* c.ntl.pall */, RISCV::C_ADD_HINT, Convert__regX0__Tie0_1_1__regX3, AMFBS_HasStdExtC_HasRVCHints_HasStdExtZihintntl, {  }, },
  { 1447 /* c.ntl.s1 */, RISCV::C_ADD_HINT, Convert__regX0__Tie0_1_1__regX4, AMFBS_HasStdExtC_HasRVCHints_HasStdExtZihintntl, {  }, },
  { 1456 /* c.or */, RISCV::C_OR, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_GPRC }, },
  { 1461 /* c.sd */, RISCV::C_SD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1461 /* c.sd */, RISCV::C_SD, Convert__Reg1_0__Reg1_3__UImm8Lsb0001_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK_UImm8Lsb000, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1466 /* c.sdsp */, RISCV::C_SDSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1466 /* c.sdsp */, RISCV::C_SDSP, Convert__Reg1_0__Reg1_3__UImm9Lsb0001_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPR, MCK_UImm9Lsb000, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1473 /* c.slli */, RISCV::C_SLLI_HINT, Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRX0, MCK_UImmLog2XLenNonZero }, },
  { 1473 /* c.slli */, RISCV::C_SLLI, Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRNoX0, MCK_UImmLog2XLenNonZero }, },
  { 1480 /* c.slli64 */, RISCV::C_SLLI64_HINT, Convert__Reg1_0__Tie0_1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPR }, },
  { 1489 /* c.srai */, RISCV::C_SRAI, Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_UImmLog2XLenNonZero }, },
  { 1496 /* c.srai64 */, RISCV::C_SRAI64_HINT, Convert__Reg1_0__Tie0_1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRC }, },
  { 1505 /* c.srli */, RISCV::C_SRLI, Convert__Reg1_0__Tie0_1_1__UImmLog2XLenNonZero1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_UImmLog2XLenNonZero }, },
  { 1512 /* c.srli64 */, RISCV::C_SRLI64_HINT, Convert__Reg1_0__Tie0_1_1, AMFBS_HasStdExtCOrZca_HasRVCHints, { MCK_GPRC }, },
  { 1521 /* c.sub */, RISCV::C_SUB, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_GPRC }, },
  { 1527 /* c.subw */, RISCV::C_SUBW, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca_IsRV64, { MCK_GPRC, MCK_GPRC }, },
  { 1534 /* c.sw */, RISCV::C_SW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1534 /* c.sw */, RISCV::C_SW, Convert__Reg1_0__Reg1_3__UImm7Lsb001_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_UImm7Lsb00, MCK__40_, MCK_GPRC, MCK__41_ }, },
  { 1539 /* c.swsp */, RISCV::C_SWSP, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1539 /* c.swsp */, RISCV::C_SWSP, Convert__Reg1_0__Reg1_3__UImm8Lsb001_1, AMFBS_HasStdExtCOrZca, { MCK_GPR, MCK_UImm8Lsb00, MCK__40_, MCK_SP, MCK__41_ }, },
  { 1546 /* c.unimp */, RISCV::C_UNIMP, Convert_NoOperands, AMFBS_HasStdExtCOrZca, {  }, },
  { 1554 /* c.xor */, RISCV::C_XOR, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasStdExtCOrZca, { MCK_GPRC, MCK_GPRC }, },
  { 1560 /* call */, RISCV::PseudoCALL, Convert__CallSymbol1_0, AMFBS_None, { MCK_CallSymbol }, },
  { 1560 /* call */, RISCV::PseudoCALLReg, Convert__Reg1_0__CallSymbol1_1, AMFBS_None, { MCK_GPR, MCK_CallSymbol }, },
  { 1565 /* cbo.clean */, RISCV::CBO_CLEAN, Convert__ZeroOffsetMemOpOperand1_0, AMFBS_HasStdExtZicbom, { MCK_ZeroOffsetMemOpOperand }, },
  { 1575 /* cbo.flush */, RISCV::CBO_FLUSH, Convert__ZeroOffsetMemOpOperand1_0, AMFBS_HasStdExtZicbom, { MCK_ZeroOffsetMemOpOperand }, },
  { 1585 /* cbo.inval */, RISCV::CBO_INVAL, Convert__ZeroOffsetMemOpOperand1_0, AMFBS_HasStdExtZicbom, { MCK_ZeroOffsetMemOpOperand }, },
  { 1595 /* cbo.zero */, RISCV::CBO_ZERO, Convert__ZeroOffsetMemOpOperand1_0, AMFBS_HasStdExtZicboz, { MCK_ZeroOffsetMemOpOperand }, },
  { 1604 /* clmul */, RISCV::CLMUL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbcOrZbkc, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1610 /* clmulh */, RISCV::CLMULH, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbcOrZbkc, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1617 /* clmulr */, RISCV::CLMULR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbc, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1624 /* clz */, RISCV::CLZ, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 1628 /* clzw */, RISCV::CLZW, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 1633 /* cpop */, RISCV::CPOP, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 1638 /* cpopw */, RISCV::CPOPW, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 1644 /* csrc */, RISCV::CSRRC, Convert__regX0__CSRSystemRegister1_0__Reg1_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_GPR }, },
  { 1644 /* csrc */, RISCV::CSRRCI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1649 /* csrci */, RISCV::CSRRCI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1655 /* csrr */, RISCV::CSRRS, Convert__Reg1_0__CSRSystemRegister1_1__regX0, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister }, },
  { 1660 /* csrrc */, RISCV::CSRRC, Convert__Reg1_0__CSRSystemRegister1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_GPR }, },
  { 1660 /* csrrc */, RISCV::CSRRCI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1666 /* csrrci */, RISCV::CSRRCI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1673 /* csrrs */, RISCV::CSRRS, Convert__Reg1_0__CSRSystemRegister1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_GPR }, },
  { 1673 /* csrrs */, RISCV::CSRRSI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1679 /* csrrsi */, RISCV::CSRRSI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1686 /* csrrw */, RISCV::CSRRW, Convert__Reg1_0__CSRSystemRegister1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_GPR }, },
  { 1686 /* csrrw */, RISCV::CSRRWI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1692 /* csrrwi */, RISCV::CSRRWI, Convert__Reg1_0__CSRSystemRegister1_1__UImm51_2, AMFBS_None, { MCK_GPR, MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1699 /* csrs */, RISCV::CSRRS, Convert__regX0__CSRSystemRegister1_0__Reg1_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_GPR }, },
  { 1699 /* csrs */, RISCV::CSRRSI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1704 /* csrsi */, RISCV::CSRRSI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1710 /* csrw */, RISCV::CSRRW, Convert__regX0__CSRSystemRegister1_0__Reg1_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_GPR }, },
  { 1710 /* csrw */, RISCV::CSRRWI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1715 /* csrwi */, RISCV::CSRRWI, Convert__regX0__CSRSystemRegister1_0__UImm51_1, AMFBS_None, { MCK_CSRSystemRegister, MCK_UImm5 }, },
  { 1721 /* ctz */, RISCV::CTZ, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 1725 /* ctzw */, RISCV::CTZW, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 1730 /* div */, RISCV::DIV, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1734 /* divu */, RISCV::DIVU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1739 /* divuw */, RISCV::DIVUW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1745 /* divw */, RISCV::DIVW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 1750 /* dret */, RISCV::DRET, Convert__imm_95_0__imm_95_0, AMFBS_None, {  }, },
  { 1755 /* ebreak */, RISCV::EBREAK, Convert_NoOperands, AMFBS_None, {  }, },
  { 1762 /* ecall */, RISCV::ECALL, Convert_NoOperands, AMFBS_None, {  }, },
  { 1768 /* fabs.d */, RISCV::FSGNJX_D, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64 }, },
  { 1768 /* fabs.d */, RISCV::FSGNJX_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 1768 /* fabs.d */, RISCV::FSGNJX_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 1775 /* fabs.h */, RISCV::FSGNJX_H, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16 }, },
  { 1775 /* fabs.h */, RISCV::FSGNJX_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 1782 /* fabs.s */, RISCV::FSGNJX_S, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32 }, },
  { 1782 /* fabs.s */, RISCV::FSGNJX_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 1789 /* fadd.d */, RISCV::FADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 1789 /* fadd.d */, RISCV::FADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 1789 /* fadd.d */, RISCV::FADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 1789 /* fadd.d */, RISCV::FADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 1789 /* fadd.d */, RISCV::FADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 1789 /* fadd.d */, RISCV::FADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 1796 /* fadd.h */, RISCV::FADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 1796 /* fadd.h */, RISCV::FADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 1796 /* fadd.h */, RISCV::FADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 1796 /* fadd.h */, RISCV::FADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1803 /* fadd.s */, RISCV::FADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 1803 /* fadd.s */, RISCV::FADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 1803 /* fadd.s */, RISCV::FADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 1803 /* fadd.s */, RISCV::FADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1810 /* fclass.d */, RISCV::FCLASS_D, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64 }, },
  { 1810 /* fclass.d */, RISCV::FCLASS_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR }, },
  { 1810 /* fclass.d */, RISCV::FCLASS_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR }, },
  { 1819 /* fclass.h */, RISCV::FCLASS_H, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16 }, },
  { 1819 /* fclass.h */, RISCV::FCLASS_H_INX, Convert__Reg1_0__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1828 /* fclass.s */, RISCV::FCLASS_S, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32 }, },
  { 1828 /* fclass.s */, RISCV::FCLASS_S_INX, Convert__Reg1_0__GPRAsFPR1_1, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1837 /* fcvt.d.h */, RISCV::FCVT_D_H, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZfhOrZfhmin_HasStdExtD, { MCK_FPR64, MCK_FPR16 }, },
  { 1837 /* fcvt.d.h */, RISCV::FCVT_D_H_INX, Convert__GPRF64AsFPR1_0__GPRAsFPR1_1, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx, { MCK_GPRF64AsFPR, MCK_GPRAsFPR }, },
  { 1846 /* fcvt.d.l */, RISCV::FCVT_D_L, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD_IsRV64, { MCK_FPR64, MCK_GPR }, },
  { 1846 /* fcvt.d.l */, RISCV::FCVT_D_L_INX, Convert__GPRF64AsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR }, },
  { 1846 /* fcvt.d.l */, RISCV::FCVT_D_L, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD_IsRV64, { MCK_FPR64, MCK_GPR, MCK_FRMArg }, },
  { 1846 /* fcvt.d.l */, RISCV::FCVT_D_L_INX, Convert__GPRF64AsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1855 /* fcvt.d.lu */, RISCV::FCVT_D_LU, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD_IsRV64, { MCK_FPR64, MCK_GPR }, },
  { 1855 /* fcvt.d.lu */, RISCV::FCVT_D_LU_INX, Convert__GPRF64AsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR }, },
  { 1855 /* fcvt.d.lu */, RISCV::FCVT_D_LU, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD_IsRV64, { MCK_FPR64, MCK_GPR, MCK_FRMArg }, },
  { 1855 /* fcvt.d.lu */, RISCV::FCVT_D_LU_INX, Convert__GPRF64AsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1865 /* fcvt.d.s */, RISCV::FCVT_D_S, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR32 }, },
  { 1865 /* fcvt.d.s */, RISCV::FCVT_D_S_INX, Convert__GPRF64AsFPR1_0__GPRAsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRAsFPR }, },
  { 1865 /* fcvt.d.s */, RISCV::FCVT_D_S_IN32X, Convert__GPRPF64AsFPR1_0__GPRAsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRAsFPR }, },
  { 1874 /* fcvt.d.w */, RISCV::FCVT_D_W, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_GPR }, },
  { 1874 /* fcvt.d.w */, RISCV::FCVT_D_W_INX, Convert__GPRF64AsFPR1_0__Reg1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR }, },
  { 1874 /* fcvt.d.w */, RISCV::FCVT_D_W_IN32X, Convert__GPRPF64AsFPR1_0__Reg1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPR }, },
  { 1883 /* fcvt.d.wu */, RISCV::FCVT_D_WU, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_GPR }, },
  { 1883 /* fcvt.d.wu */, RISCV::FCVT_D_WU_INX, Convert__GPRF64AsFPR1_0__Reg1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPR }, },
  { 1883 /* fcvt.d.wu */, RISCV::FCVT_D_WU_IN32X, Convert__GPRPF64AsFPR1_0__Reg1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPR }, },
  { 1893 /* fcvt.h.d */, RISCV::FCVT_H_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfhOrZfhmin_HasStdExtD, { MCK_FPR16, MCK_FPR64 }, },
  { 1893 /* fcvt.h.d */, RISCV::FCVT_H_D_INX, Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx, { MCK_GPRAsFPR, MCK_GPRF64AsFPR }, },
  { 1893 /* fcvt.h.d */, RISCV::FCVT_H_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfhOrZfhmin_HasStdExtD, { MCK_FPR16, MCK_FPR64, MCK_FRMArg }, },
  { 1893 /* fcvt.h.d */, RISCV::FCVT_H_D_INX, Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx, { MCK_GPRAsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 1902 /* fcvt.h.l */, RISCV::FCVT_H_L, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh_IsRV64, { MCK_FPR16, MCK_GPR }, },
  { 1902 /* fcvt.h.l */, RISCV::FCVT_H_L_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR }, },
  { 1902 /* fcvt.h.l */, RISCV::FCVT_H_L, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh_IsRV64, { MCK_FPR16, MCK_GPR, MCK_FRMArg }, },
  { 1902 /* fcvt.h.l */, RISCV::FCVT_H_L_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1911 /* fcvt.h.lu */, RISCV::FCVT_H_LU, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh_IsRV64, { MCK_FPR16, MCK_GPR }, },
  { 1911 /* fcvt.h.lu */, RISCV::FCVT_H_LU_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR }, },
  { 1911 /* fcvt.h.lu */, RISCV::FCVT_H_LU, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh_IsRV64, { MCK_FPR16, MCK_GPR, MCK_FRMArg }, },
  { 1911 /* fcvt.h.lu */, RISCV::FCVT_H_LU_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1921 /* fcvt.h.s */, RISCV::FCVT_H_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_FPR32 }, },
  { 1921 /* fcvt.h.s */, RISCV::FCVT_H_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinxOrZhinxmin, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 1921 /* fcvt.h.s */, RISCV::FCVT_H_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_FPR32, MCK_FRMArg }, },
  { 1921 /* fcvt.h.s */, RISCV::FCVT_H_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinxOrZhinxmin, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1930 /* fcvt.h.w */, RISCV::FCVT_H_W, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_GPR }, },
  { 1930 /* fcvt.h.w */, RISCV::FCVT_H_W_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPR }, },
  { 1930 /* fcvt.h.w */, RISCV::FCVT_H_W, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_GPR, MCK_FRMArg }, },
  { 1930 /* fcvt.h.w */, RISCV::FCVT_H_W_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1939 /* fcvt.h.wu */, RISCV::FCVT_H_WU, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_GPR }, },
  { 1939 /* fcvt.h.wu */, RISCV::FCVT_H_WU_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPR }, },
  { 1939 /* fcvt.h.wu */, RISCV::FCVT_H_WU, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_GPR, MCK_FRMArg }, },
  { 1939 /* fcvt.h.wu */, RISCV::FCVT_H_WU_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 1949 /* fcvt.l.d */, RISCV::FCVT_L_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD_IsRV64, { MCK_GPR, MCK_FPR64 }, },
  { 1949 /* fcvt.l.d */, RISCV::FCVT_L_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR }, },
  { 1949 /* fcvt.l.d */, RISCV::FCVT_L_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD_IsRV64, { MCK_GPR, MCK_FPR64, MCK_FRMArg }, },
  { 1949 /* fcvt.l.d */, RISCV::FCVT_L_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 1958 /* fcvt.l.h */, RISCV::FCVT_L_H, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh_IsRV64, { MCK_GPR, MCK_FPR16 }, },
  { 1958 /* fcvt.l.h */, RISCV::FCVT_L_H_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1958 /* fcvt.l.h */, RISCV::FCVT_L_H, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh_IsRV64, { MCK_GPR, MCK_FPR16, MCK_FRMArg }, },
  { 1958 /* fcvt.l.h */, RISCV::FCVT_L_H_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1967 /* fcvt.l.s */, RISCV::FCVT_L_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF_IsRV64, { MCK_GPR, MCK_FPR32 }, },
  { 1967 /* fcvt.l.s */, RISCV::FCVT_L_S_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1967 /* fcvt.l.s */, RISCV::FCVT_L_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF_IsRV64, { MCK_GPR, MCK_FPR32, MCK_FRMArg }, },
  { 1967 /* fcvt.l.s */, RISCV::FCVT_L_S_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1976 /* fcvt.lu.d */, RISCV::FCVT_LU_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD_IsRV64, { MCK_GPR, MCK_FPR64 }, },
  { 1976 /* fcvt.lu.d */, RISCV::FCVT_LU_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR }, },
  { 1976 /* fcvt.lu.d */, RISCV::FCVT_LU_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD_IsRV64, { MCK_GPR, MCK_FPR64, MCK_FRMArg }, },
  { 1976 /* fcvt.lu.d */, RISCV::FCVT_LU_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 1986 /* fcvt.lu.h */, RISCV::FCVT_LU_H, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh_IsRV64, { MCK_GPR, MCK_FPR16 }, },
  { 1986 /* fcvt.lu.h */, RISCV::FCVT_LU_H_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1986 /* fcvt.lu.h */, RISCV::FCVT_LU_H, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh_IsRV64, { MCK_GPR, MCK_FPR16, MCK_FRMArg }, },
  { 1986 /* fcvt.lu.h */, RISCV::FCVT_LU_H_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 1996 /* fcvt.lu.s */, RISCV::FCVT_LU_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF_IsRV64, { MCK_GPR, MCK_FPR32 }, },
  { 1996 /* fcvt.lu.s */, RISCV::FCVT_LU_S_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR }, },
  { 1996 /* fcvt.lu.s */, RISCV::FCVT_LU_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF_IsRV64, { MCK_GPR, MCK_FPR32, MCK_FRMArg }, },
  { 1996 /* fcvt.lu.s */, RISCV::FCVT_LU_S_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR32, MCK_FPR64 }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D_INX, Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRAsFPR, MCK_GPRF64AsFPR }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D_IN32X, Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRAsFPR, MCK_GPRPF64AsFPR }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD, { MCK_FPR32, MCK_FPR64, MCK_FRMArg }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D_INX, Convert__GPRAsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRAsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2006 /* fcvt.s.d */, RISCV::FCVT_S_D_IN32X, Convert__GPRAsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRAsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2015 /* fcvt.s.h */, RISCV::FCVT_S_H, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR32, MCK_FPR16 }, },
  { 2015 /* fcvt.s.h */, RISCV::FCVT_S_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1, AMFBS_HasStdExtZhinxOrZhinxmin, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2024 /* fcvt.s.l */, RISCV::FCVT_S_L, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF_IsRV64, { MCK_FPR32, MCK_GPR }, },
  { 2024 /* fcvt.s.l */, RISCV::FCVT_S_L_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR }, },
  { 2024 /* fcvt.s.l */, RISCV::FCVT_S_L, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF_IsRV64, { MCK_FPR32, MCK_GPR, MCK_FRMArg }, },
  { 2024 /* fcvt.s.l */, RISCV::FCVT_S_L_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 2033 /* fcvt.s.lu */, RISCV::FCVT_S_LU, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF_IsRV64, { MCK_FPR32, MCK_GPR }, },
  { 2033 /* fcvt.s.lu */, RISCV::FCVT_S_LU_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR }, },
  { 2033 /* fcvt.s.lu */, RISCV::FCVT_S_LU, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF_IsRV64, { MCK_FPR32, MCK_GPR, MCK_FRMArg }, },
  { 2033 /* fcvt.s.lu */, RISCV::FCVT_S_LU_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfinx_IsRV64, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 2043 /* fcvt.s.w */, RISCV::FCVT_S_W, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_GPR }, },
  { 2043 /* fcvt.s.w */, RISCV::FCVT_S_W_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPR }, },
  { 2043 /* fcvt.s.w */, RISCV::FCVT_S_W, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_GPR, MCK_FRMArg }, },
  { 2043 /* fcvt.s.w */, RISCV::FCVT_S_W_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 2052 /* fcvt.s.wu */, RISCV::FCVT_S_WU, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_GPR }, },
  { 2052 /* fcvt.s.wu */, RISCV::FCVT_S_WU_INX, Convert__GPRAsFPR1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPR }, },
  { 2052 /* fcvt.s.wu */, RISCV::FCVT_S_WU, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_GPR, MCK_FRMArg }, },
  { 2052 /* fcvt.s.wu */, RISCV::FCVT_S_WU_INX, Convert__GPRAsFPR1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPR, MCK_FRMArg }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64 }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FRMArg }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2062 /* fcvt.w.d */, RISCV::FCVT_W_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2071 /* fcvt.w.h */, RISCV::FCVT_W_H, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16 }, },
  { 2071 /* fcvt.w.h */, RISCV::FCVT_W_H_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 2071 /* fcvt.w.h */, RISCV::FCVT_W_H, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FRMArg }, },
  { 2071 /* fcvt.w.h */, RISCV::FCVT_W_H_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2080 /* fcvt.w.s */, RISCV::FCVT_W_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32 }, },
  { 2080 /* fcvt.w.s */, RISCV::FCVT_W_S_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 2080 /* fcvt.w.s */, RISCV::FCVT_W_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FRMArg }, },
  { 2080 /* fcvt.w.s */, RISCV::FCVT_W_S_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64 }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FRMArg }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2089 /* fcvt.wu.d */, RISCV::FCVT_WU_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2099 /* fcvt.wu.h */, RISCV::FCVT_WU_H, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16 }, },
  { 2099 /* fcvt.wu.h */, RISCV::FCVT_WU_H_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 2099 /* fcvt.wu.h */, RISCV::FCVT_WU_H, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FRMArg }, },
  { 2099 /* fcvt.wu.h */, RISCV::FCVT_WU_H_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2109 /* fcvt.wu.s */, RISCV::FCVT_WU_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32 }, },
  { 2109 /* fcvt.wu.s */, RISCV::FCVT_WU_S_INX, Convert__Reg1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR }, },
  { 2109 /* fcvt.wu.s */, RISCV::FCVT_WU_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FRMArg }, },
  { 2109 /* fcvt.wu.s */, RISCV::FCVT_WU_S_INX, Convert__Reg1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2119 /* fdiv.d */, RISCV::FDIV_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2126 /* fdiv.h */, RISCV::FDIV_H, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2126 /* fdiv.h */, RISCV::FDIV_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2126 /* fdiv.h */, RISCV::FDIV_H, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2126 /* fdiv.h */, RISCV::FDIV_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2133 /* fdiv.s */, RISCV::FDIV_S, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2133 /* fdiv.s */, RISCV::FDIV_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2133 /* fdiv.s */, RISCV::FDIV_S, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2133 /* fdiv.s */, RISCV::FDIV_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2140 /* fence */, RISCV::FENCE, Convert__imm_95_15__imm_95_15, AMFBS_None, {  }, },
  { 2140 /* fence */, RISCV::FENCE, Convert__FenceArg1_0__FenceArg1_1, AMFBS_None, { MCK_FenceArg, MCK_FenceArg }, },
  { 2146 /* fence.i */, RISCV::FENCE_I, Convert_NoOperands, AMFBS_None, {  }, },
  { 2154 /* fence.tso */, RISCV::FENCE_TSO, Convert_NoOperands, AMFBS_None, {  }, },
  { 2164 /* feq.d */, RISCV::FEQ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FPR64 }, },
  { 2164 /* feq.d */, RISCV::FEQ_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2164 /* feq.d */, RISCV::FEQ_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2170 /* feq.h */, RISCV::FEQ_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FPR16 }, },
  { 2170 /* feq.h */, RISCV::FEQ_H_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2176 /* feq.s */, RISCV::FEQ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FPR32 }, },
  { 2176 /* feq.s */, RISCV::FEQ_S_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2182 /* fge.d */, RISCV::FLE_D, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FPR64 }, },
  { 2182 /* fge.d */, RISCV::FLE_D_INX, Convert__Reg1_0__GPRF64AsFPR1_2__GPRF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2182 /* fge.d */, RISCV::FLE_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_2__GPRPF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2188 /* fge.h */, RISCV::FLE_H, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FPR16 }, },
  { 2188 /* fge.h */, RISCV::FLE_H_INX, Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2194 /* fge.s */, RISCV::FLE_S, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FPR32 }, },
  { 2194 /* fge.s */, RISCV::FLE_S_INX, Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2200 /* fgt.d */, RISCV::FLT_D, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FPR64 }, },
  { 2200 /* fgt.d */, RISCV::FLT_D_INX, Convert__Reg1_0__GPRF64AsFPR1_2__GPRF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2200 /* fgt.d */, RISCV::FLT_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_2__GPRPF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2206 /* fgt.h */, RISCV::FLT_H, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FPR16 }, },
  { 2206 /* fgt.h */, RISCV::FLT_H_INX, Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2212 /* fgt.s */, RISCV::FLT_S, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FPR32 }, },
  { 2212 /* fgt.s */, RISCV::FLT_S_INX, Convert__Reg1_0__GPRAsFPR1_2__GPRAsFPR1_1, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2218 /* fld */, RISCV::PseudoFLD, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_BareSymbol, MCK_GPR }, },
  { 2218 /* fld */, RISCV::FLD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtD, { MCK_FPR64, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2218 /* fld */, RISCV::FLD, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2222 /* fle.d */, RISCV::FLE_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FPR64 }, },
  { 2222 /* fle.d */, RISCV::FLE_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2222 /* fle.d */, RISCV::FLE_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2228 /* fle.h */, RISCV::FLE_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FPR16 }, },
  { 2228 /* fle.h */, RISCV::FLE_H_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2234 /* fle.s */, RISCV::FLE_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FPR32 }, },
  { 2234 /* fle.s */, RISCV::FLE_S_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2240 /* flh */, RISCV::PseudoFLH, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_BareSymbol, MCK_GPR }, },
  { 2240 /* flh */, RISCV::FLH, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2240 /* flh */, RISCV::FLH, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2244 /* flt.d */, RISCV::FLT_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_GPR, MCK_FPR64, MCK_FPR64 }, },
  { 2244 /* flt.d */, RISCV::FLT_D_INX, Convert__Reg1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2244 /* flt.d */, RISCV::FLT_D_IN32X, Convert__Reg1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2250 /* flt.h */, RISCV::FLT_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_GPR, MCK_FPR16, MCK_FPR16 }, },
  { 2250 /* flt.h */, RISCV::FLT_H_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2256 /* flt.s */, RISCV::FLT_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32, MCK_FPR32 }, },
  { 2256 /* flt.s */, RISCV::FLT_S_INX, Convert__Reg1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2262 /* flw */, RISCV::PseudoFLW, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_BareSymbol, MCK_GPR }, },
  { 2262 /* flw */, RISCV::FLW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtF, { MCK_FPR32, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2262 /* flw */, RISCV::FLW, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2266 /* fmadd.d */, RISCV::FMADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2274 /* fmadd.h */, RISCV::FMADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2274 /* fmadd.h */, RISCV::FMADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2274 /* fmadd.h */, RISCV::FMADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2274 /* fmadd.h */, RISCV::FMADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2282 /* fmadd.s */, RISCV::FMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2282 /* fmadd.s */, RISCV::FMADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2282 /* fmadd.s */, RISCV::FMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2282 /* fmadd.s */, RISCV::FMADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2290 /* fmax.d */, RISCV::FMAX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2290 /* fmax.d */, RISCV::FMAX_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2290 /* fmax.d */, RISCV::FMAX_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2297 /* fmax.h */, RISCV::FMAX_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2297 /* fmax.h */, RISCV::FMAX_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2304 /* fmax.s */, RISCV::FMAX_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2304 /* fmax.s */, RISCV::FMAX_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2311 /* fmin.d */, RISCV::FMIN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2311 /* fmin.d */, RISCV::FMIN_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2311 /* fmin.d */, RISCV::FMIN_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2318 /* fmin.h */, RISCV::FMIN_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2318 /* fmin.h */, RISCV::FMIN_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2325 /* fmin.s */, RISCV::FMIN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2325 /* fmin.s */, RISCV::FMIN_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2332 /* fmsub.d */, RISCV::FMSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2340 /* fmsub.h */, RISCV::FMSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2340 /* fmsub.h */, RISCV::FMSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2340 /* fmsub.h */, RISCV::FMSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2340 /* fmsub.h */, RISCV::FMSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2348 /* fmsub.s */, RISCV::FMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2348 /* fmsub.s */, RISCV::FMSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2348 /* fmsub.s */, RISCV::FMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2348 /* fmsub.s */, RISCV::FMSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2356 /* fmul.d */, RISCV::FMUL_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2363 /* fmul.h */, RISCV::FMUL_H, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2363 /* fmul.h */, RISCV::FMUL_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2363 /* fmul.h */, RISCV::FMUL_H, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2363 /* fmul.h */, RISCV::FMUL_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2370 /* fmul.s */, RISCV::FMUL_S, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2370 /* fmul.s */, RISCV::FMUL_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2370 /* fmul.s */, RISCV::FMUL_S, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2370 /* fmul.s */, RISCV::FMUL_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2377 /* fmv.d */, RISCV::FSGNJ_D, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64 }, },
  { 2383 /* fmv.d.x */, RISCV::FMV_D_X, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD_IsRV64, { MCK_FPR64, MCK_GPR }, },
  { 2391 /* fmv.h */, RISCV::FSGNJ_H, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16 }, },
  { 2391 /* fmv.h */, RISCV::FSGNJ_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2397 /* fmv.h.x */, RISCV::FMV_H_X, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_GPR }, },
  { 2405 /* fmv.s */, RISCV::FSGNJ_S, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32 }, },
  { 2411 /* fmv.w.x */, RISCV::FMV_W_X, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_GPR }, },
  { 2419 /* fmv.x.d */, RISCV::FMV_X_D, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtD_IsRV64, { MCK_GPR, MCK_FPR64 }, },
  { 2427 /* fmv.x.h */, RISCV::FMV_X_H, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_GPR, MCK_FPR16 }, },
  { 2435 /* fmv.x.w */, RISCV::FMV_X_W, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_FPR32 }, },
  { 2443 /* fneg.d */, RISCV::FSGNJN_D, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64 }, },
  { 2443 /* fneg.d */, RISCV::FSGNJN_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2443 /* fneg.d */, RISCV::FSGNJN_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_1, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2450 /* fneg.h */, RISCV::FSGNJN_H, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16 }, },
  { 2450 /* fneg.h */, RISCV::FSGNJN_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2457 /* fneg.s */, RISCV::FSGNJN_S, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32 }, },
  { 2457 /* fneg.s */, RISCV::FSGNJN_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_1, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2464 /* fnmadd.d */, RISCV::FNMADD_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2473 /* fnmadd.h */, RISCV::FNMADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2473 /* fnmadd.h */, RISCV::FNMADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2473 /* fnmadd.h */, RISCV::FNMADD_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2473 /* fnmadd.h */, RISCV::FNMADD_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2482 /* fnmadd.s */, RISCV::FNMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2482 /* fnmadd.s */, RISCV::FNMADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2482 /* fnmadd.s */, RISCV::FNMADD_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2482 /* fnmadd.s */, RISCV::FNMADD_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__GPRF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2491 /* fnmsub.d */, RISCV::FNMSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__GPRPF64AsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2500 /* fnmsub.h */, RISCV::FNMSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2500 /* fnmsub.h */, RISCV::FNMSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2500 /* fnmsub.h */, RISCV::FNMSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2500 /* fnmsub.h */, RISCV::FNMSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2509 /* fnmsub.s */, RISCV::FNMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2509 /* fnmsub.s */, RISCV::FNMSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2509 /* fnmsub.s */, RISCV::FNMSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__Reg1_3__FRMArg1_4, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2509 /* fnmsub.s */, RISCV::FNMSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__GPRAsFPR1_3__FRMArg1_4, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2518 /* frcsr */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3__regX0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2524 /* frflags */, RISCV::CSRRS, Convert__Reg1_0__imm_95_1__regX0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2532 /* frrm */, RISCV::CSRRS, Convert__Reg1_0__imm_95_2__regX0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2537 /* frsr */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3__regX0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2542 /* fscsr */, RISCV::CSRRW, Convert__regX0__imm_95_3__Reg1_0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2542 /* fscsr */, RISCV::CSRRW, Convert__Reg1_0__imm_95_3__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_GPR }, },
  { 2548 /* fsd */, RISCV::PseudoFSD, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_BareSymbol, MCK_GPR }, },
  { 2548 /* fsd */, RISCV::FSD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtD, { MCK_FPR64, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2548 /* fsd */, RISCV::FSD, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtD, { MCK_FPR64, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2552 /* fsflags */, RISCV::CSRRW, Convert__regX0__imm_95_1__Reg1_0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2552 /* fsflags */, RISCV::CSRRW, Convert__Reg1_0__imm_95_1__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_GPR }, },
  { 2560 /* fsflagsi */, RISCV::CSRRWI, Convert__regX0__imm_95_1__UImm51_0, AMFBS_HasStdExtF, { MCK_UImm5 }, },
  { 2560 /* fsflagsi */, RISCV::CSRRWI, Convert__Reg1_0__imm_95_1__UImm51_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_UImm5 }, },
  { 2569 /* fsgnj.d */, RISCV::FSGNJ_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2569 /* fsgnj.d */, RISCV::FSGNJ_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2569 /* fsgnj.d */, RISCV::FSGNJ_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2577 /* fsgnj.h */, RISCV::FSGNJ_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2577 /* fsgnj.h */, RISCV::FSGNJ_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2585 /* fsgnj.s */, RISCV::FSGNJ_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2585 /* fsgnj.s */, RISCV::FSGNJ_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2593 /* fsgnjn.d */, RISCV::FSGNJN_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2593 /* fsgnjn.d */, RISCV::FSGNJN_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2593 /* fsgnjn.d */, RISCV::FSGNJN_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2602 /* fsgnjn.h */, RISCV::FSGNJN_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2602 /* fsgnjn.h */, RISCV::FSGNJN_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2611 /* fsgnjn.s */, RISCV::FSGNJN_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2611 /* fsgnjn.s */, RISCV::FSGNJN_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2620 /* fsgnjx.d */, RISCV::FSGNJX_D, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2620 /* fsgnjx.d */, RISCV::FSGNJX_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2620 /* fsgnjx.d */, RISCV::FSGNJX_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2629 /* fsgnjx.h */, RISCV::FSGNJX_H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2629 /* fsgnjx.h */, RISCV::FSGNJX_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2638 /* fsgnjx.s */, RISCV::FSGNJX_S, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2638 /* fsgnjx.s */, RISCV::FSGNJX_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2647 /* fsh */, RISCV::PseudoFSH, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_BareSymbol, MCK_GPR }, },
  { 2647 /* fsh */, RISCV::FSH, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2647 /* fsh */, RISCV::FSH, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtZfhOrZfhmin, { MCK_FPR16, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64 }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2651 /* fsqrt.d */, RISCV::FSQRT_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2659 /* fsqrt.h */, RISCV::FSQRT_H, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16 }, },
  { 2659 /* fsqrt.h */, RISCV::FSQRT_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2659 /* fsqrt.h */, RISCV::FSQRT_H, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2659 /* fsqrt.h */, RISCV::FSQRT_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2667 /* fsqrt.s */, RISCV::FSQRT_S, Convert__Reg1_0__Reg1_1__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32 }, },
  { 2667 /* fsqrt.s */, RISCV::FSQRT_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2667 /* fsqrt.s */, RISCV::FSQRT_S, Convert__Reg1_0__Reg1_1__FRMArg1_2, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2667 /* fsqrt.s */, RISCV::FSQRT_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__FRMArg1_2, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2675 /* fsrm */, RISCV::CSRRW, Convert__regX0__imm_95_2__Reg1_0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2675 /* fsrm */, RISCV::CSRRW, Convert__Reg1_0__imm_95_2__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_GPR }, },
  { 2680 /* fsrmi */, RISCV::CSRRWI, Convert__regX0__imm_95_2__UImm51_0, AMFBS_HasStdExtF, { MCK_UImm5 }, },
  { 2680 /* fsrmi */, RISCV::CSRRWI, Convert__Reg1_0__imm_95_2__UImm51_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_UImm5 }, },
  { 2686 /* fssr */, RISCV::CSRRW, Convert__regX0__imm_95_3__Reg1_0, AMFBS_HasStdExtF, { MCK_GPR }, },
  { 2686 /* fssr */, RISCV::CSRRW, Convert__Reg1_0__imm_95_3__Reg1_1, AMFBS_HasStdExtF, { MCK_GPR, MCK_GPR }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64 }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__imm_95_7, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtD, { MCK_FPR64, MCK_FPR64, MCK_FPR64, MCK_FRMArg }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D_INX, Convert__GPRF64AsFPR1_0__GPRF64AsFPR1_1__GPRF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV64, { MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_GPRF64AsFPR, MCK_FRMArg }, },
  { 2691 /* fsub.d */, RISCV::FSUB_D_IN32X, Convert__GPRPF64AsFPR1_0__GPRPF64AsFPR1_1__GPRPF64AsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZdinx_IsRV32, { MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_GPRPF64AsFPR, MCK_FRMArg }, },
  { 2698 /* fsub.h */, RISCV::FSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16 }, },
  { 2698 /* fsub.h */, RISCV::FSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2698 /* fsub.h */, RISCV::FSUB_H, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtZfh, { MCK_FPR16, MCK_FPR16, MCK_FPR16, MCK_FRMArg }, },
  { 2698 /* fsub.h */, RISCV::FSUB_H_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZhinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2705 /* fsub.s */, RISCV::FSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_7, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32 }, },
  { 2705 /* fsub.s */, RISCV::FSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__imm_95_7, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR }, },
  { 2705 /* fsub.s */, RISCV::FSUB_S, Convert__Reg1_0__Reg1_1__Reg1_2__FRMArg1_3, AMFBS_HasStdExtF, { MCK_FPR32, MCK_FPR32, MCK_FPR32, MCK_FRMArg }, },
  { 2705 /* fsub.s */, RISCV::FSUB_S_INX, Convert__GPRAsFPR1_0__GPRAsFPR1_1__GPRAsFPR1_2__FRMArg1_3, AMFBS_HasStdExtZfinx, { MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_GPRAsFPR, MCK_FRMArg }, },
  { 2712 /* fsw */, RISCV::PseudoFSW, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_BareSymbol, MCK_GPR }, },
  { 2712 /* fsw */, RISCV::FSW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_HasStdExtF, { MCK_FPR32, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2712 /* fsw */, RISCV::FSW, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_HasStdExtF, { MCK_FPR32, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2716 /* hfence.gvma */, RISCV::HFENCE_GVMA, Convert__regX0__regX0, AMFBS_None, {  }, },
  { 2716 /* hfence.gvma */, RISCV::HFENCE_GVMA, Convert__Reg1_0__regX0, AMFBS_None, { MCK_GPR }, },
  { 2716 /* hfence.gvma */, RISCV::HFENCE_GVMA, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_GPR }, },
  { 2728 /* hfence.vvma */, RISCV::HFENCE_VVMA, Convert__regX0__regX0, AMFBS_None, {  }, },
  { 2728 /* hfence.vvma */, RISCV::HFENCE_VVMA, Convert__Reg1_0__regX0, AMFBS_None, { MCK_GPR }, },
  { 2728 /* hfence.vvma */, RISCV::HFENCE_VVMA, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_GPR }, },
  { 2740 /* hinval.gvma */, RISCV::HINVAL_GVMA, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtSvinval, { MCK_GPR, MCK_GPR }, },
  { 2752 /* hinval.vvma */, RISCV::HINVAL_VVMA, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtSvinval, { MCK_GPR, MCK_GPR }, },
  { 2764 /* hlv.b */, RISCV::HLV_B, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2770 /* hlv.bu */, RISCV::HLV_BU, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2777 /* hlv.d */, RISCV::HLV_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_IsRV64_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2783 /* hlv.h */, RISCV::HLV_H, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2789 /* hlv.hu */, RISCV::HLV_HU, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2796 /* hlv.w */, RISCV::HLV_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2802 /* hlv.wu */, RISCV::HLV_WU, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_IsRV64_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2809 /* hlvx.hu */, RISCV::HLVX_HU, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2817 /* hlvx.wu */, RISCV::HLVX_WU, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2825 /* hsv.b */, RISCV::HSV_B, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2831 /* hsv.d */, RISCV::HSV_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_IsRV64_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2837 /* hsv.h */, RISCV::HSV_H, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2843 /* hsv.w */, RISCV::HSV_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtH, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2849 /* j */, RISCV::JAL, Convert__regX0__SImm21Lsb0JAL1_0, AMFBS_None, { MCK_SImm21Lsb0JAL }, },
  { 2851 /* jal */, RISCV::JAL, Convert__regX1__SImm21Lsb0JAL1_0, AMFBS_None, { MCK_SImm21Lsb0JAL }, },
  { 2851 /* jal */, RISCV::JAL, Convert__Reg1_0__SImm21Lsb0JAL1_1, AMFBS_None, { MCK_GPR, MCK_SImm21Lsb0JAL }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__regX1__Reg1_0__imm_95_0, AMFBS_None, { MCK_GPR }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__Reg1_0__Reg1_1__imm_95_0, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__regX1__Reg1_0__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12 }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__regX1__Reg1_2__SImm121_0, AMFBS_None, { MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2855 /* jalr */, RISCV::JALR, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2860 /* jr */, RISCV::JALR, Convert__regX0__Reg1_0__imm_95_0, AMFBS_None, { MCK_GPR }, },
  { 2860 /* jr */, RISCV::JALR, Convert__regX0__Reg1_0__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12 }, },
  { 2860 /* jr */, RISCV::JALR, Convert__regX0__Reg1_2__SImm121_0, AMFBS_None, { MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2863 /* jump */, RISCV::PseudoJump, Convert__Reg1_1__PseudoJumpSymbol1_0, AMFBS_None, { MCK_PseudoJumpSymbol, MCK_GPR }, },
  { 2868 /* la */, RISCV::PseudoLA, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2871 /* la.tls.gd */, RISCV::PseudoLA_TLS_GD, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2881 /* la.tls.ie */, RISCV::PseudoLA_TLS_IE, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2891 /* lb */, RISCV::PseudoLB, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2891 /* lb */, RISCV::LB, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2891 /* lb */, RISCV::LB, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2894 /* lbu */, RISCV::PseudoLBU, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2894 /* lbu */, RISCV::LBU, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2894 /* lbu */, RISCV::LBU, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2898 /* ld */, RISCV::PseudoLD, Convert__Reg1_0__BareSymbol1_1, AMFBS_IsRV64, { MCK_GPR, MCK_BareSymbol }, },
  { 2898 /* ld */, RISCV::LD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_IsRV64, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2898 /* ld */, RISCV::LD, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_IsRV64, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2901 /* lh */, RISCV::PseudoLH, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2901 /* lh */, RISCV::LH, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2901 /* lh */, RISCV::LH, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2904 /* lhu */, RISCV::PseudoLHU, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2904 /* lhu */, RISCV::LHU, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2904 /* lhu */, RISCV::LHU, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2908 /* li */, RISCV::ADDI, Convert__Reg1_0__regX0__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12 }, },
  { 2908 /* li */, RISCV::PseudoLI, Convert__Reg1_0__ImmXLenLI1_1, AMFBS_None, { MCK_GPR, MCK_ImmXLenLI }, },
  { 2911 /* lla */, RISCV::PseudoLLA, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2915 /* lr.d */, RISCV::LR_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2920 /* lr.d.aq */, RISCV::LR_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2928 /* lr.d.aqrl */, RISCV::LR_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2938 /* lr.d.rl */, RISCV::LR_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2946 /* lr.w */, RISCV::LR_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2951 /* lr.w.aq */, RISCV::LR_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2959 /* lr.w.aqrl */, RISCV::LR_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2969 /* lr.w.rl */, RISCV::LR_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 2977 /* lui */, RISCV::LUI, Convert__Reg1_0__UImm20LUI1_1, AMFBS_None, { MCK_GPR, MCK_UImm20LUI }, },
  { 2981 /* lw */, RISCV::PseudoLW, Convert__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol }, },
  { 2981 /* lw */, RISCV::LW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2981 /* lw */, RISCV::LW, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2984 /* lwu */, RISCV::PseudoLWU, Convert__Reg1_0__BareSymbol1_1, AMFBS_IsRV64, { MCK_GPR, MCK_BareSymbol }, },
  { 2984 /* lwu */, RISCV::LWU, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_IsRV64, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2984 /* lwu */, RISCV::LWU, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_IsRV64, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 2988 /* max */, RISCV::MAX, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2992 /* maxu */, RISCV::MAXU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 2997 /* min */, RISCV::MIN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3001 /* minu */, RISCV::MINU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3006 /* mret */, RISCV::MRET, Convert__imm_95_0__imm_95_0, AMFBS_None, {  }, },
  { 3011 /* mul */, RISCV::MUL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtMOrZmmul, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3015 /* mulh */, RISCV::MULH, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtMOrZmmul, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3020 /* mulhsu */, RISCV::MULHSU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtMOrZmmul, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3027 /* mulhu */, RISCV::MULHU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtMOrZmmul, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3033 /* mulw */, RISCV::MULW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtMOrZmmul_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3038 /* mv */, RISCV::ADDI, Convert__Reg1_0__Reg1_1__imm_95_0, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3041 /* neg */, RISCV::SUB, Convert__Reg1_0__regX0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3045 /* negw */, RISCV::SUBW, Convert__Reg1_0__regX0__Reg1_1, AMFBS_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3050 /* nop */, RISCV::ADDI, Convert__regX0__regX0__imm_95_0, AMFBS_None, {  }, },
  { 3054 /* not */, RISCV::XORI, Convert__Reg1_0__Reg1_1__imm_95__MINUS_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3058 /* ntl.all */, RISCV::ADD, Convert__regX0__regX0__regX5, AMFBS_HasStdExtZihintntl, {  }, },
  { 3066 /* ntl.p1 */, RISCV::ADD, Convert__regX0__regX0__regX2, AMFBS_HasStdExtZihintntl, {  }, },
  { 3073 /* ntl.pall */, RISCV::ADD, Convert__regX0__regX0__regX3, AMFBS_HasStdExtZihintntl, {  }, },
  { 3082 /* ntl.s1 */, RISCV::ADD, Convert__regX0__regX0__regX4, AMFBS_HasStdExtZihintntl, {  }, },
  { 3089 /* or */, RISCV::OR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3089 /* or */, RISCV::ORI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3092 /* orc.b */, RISCV::ORC_B, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 3098 /* ori */, RISCV::ORI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3102 /* orn */, RISCV::ORN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3106 /* pack */, RISCV::PACK, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3111 /* packh */, RISCV::PACKH, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3117 /* packw */, RISCV::PACKW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbkb_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3123 /* pause */, RISCV::FENCE, Convert__imm_95_1__imm_95_0, AMFBS_HasStdExtZihintpause, {  }, },
  { 3129 /* prefetch.i */, RISCV::PREFETCH_I, Convert__Reg1_2__SImm12Lsb000001_0, AMFBS_HasStdExtZicbop, { MCK_SImm12Lsb00000, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3140 /* prefetch.r */, RISCV::PREFETCH_R, Convert__Reg1_2__SImm12Lsb000001_0, AMFBS_HasStdExtZicbop, { MCK_SImm12Lsb00000, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3151 /* prefetch.w */, RISCV::PREFETCH_W, Convert__Reg1_2__SImm12Lsb000001_0, AMFBS_HasStdExtZicbop, { MCK_SImm12Lsb00000, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3162 /* rdcycle */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3072__regX0, AMFBS_None, { MCK_GPR }, },
  { 3170 /* rdcycleh */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3200__regX0, AMFBS_IsRV32, { MCK_GPR }, },
  { 3179 /* rdinstret */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3074__regX0, AMFBS_None, { MCK_GPR }, },
  { 3189 /* rdinstreth */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3202__regX0, AMFBS_IsRV32, { MCK_GPR }, },
  { 3200 /* rdtime */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3073__regX0, AMFBS_None, { MCK_GPR }, },
  { 3207 /* rdtimeh */, RISCV::CSRRS, Convert__Reg1_0__imm_95_3201__regX0, AMFBS_IsRV32, { MCK_GPR }, },
  { 3215 /* rem */, RISCV::REM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3219 /* remu */, RISCV::REMU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3224 /* remuw */, RISCV::REMUW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3230 /* remw */, RISCV::REMW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtM_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3235 /* ret */, RISCV::JALR, Convert__regX0__regX1__imm_95_0, AMFBS_None, {  }, },
  { 3239 /* rev8 */, RISCV::REV8_RV32, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbbOrZbkb_IsRV32, { MCK_GPR, MCK_GPR }, },
  { 3239 /* rev8 */, RISCV::REV8_RV64, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbbOrZbkb_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3244 /* rol */, RISCV::ROL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3248 /* rolw */, RISCV::ROLW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3253 /* ror */, RISCV::ROR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3253 /* ror */, RISCV::RORI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3257 /* rori */, RISCV::RORI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3262 /* roriw */, RISCV::RORIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_HasStdExtZbbOrZbkb_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3268 /* rorw */, RISCV::RORW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3268 /* rorw */, RISCV::RORIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_HasStdExtZbb_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3273 /* sb */, RISCV::PseudoSB, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol, MCK_GPR }, },
  { 3273 /* sb */, RISCV::SB, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3273 /* sb */, RISCV::SB, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3276 /* sc.d */, RISCV::SC_D, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3281 /* sc.d.aq */, RISCV::SC_D_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3289 /* sc.d.aqrl */, RISCV::SC_D_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3299 /* sc.d.rl */, RISCV::SC_D_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA_IsRV64, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3307 /* sc.w */, RISCV::SC_W, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3312 /* sc.w.aq */, RISCV::SC_W_AQ, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3320 /* sc.w.aqrl */, RISCV::SC_W_AQ_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3330 /* sc.w.rl */, RISCV::SC_W_RL, Convert__Reg1_0__ZeroOffsetMemOpOperand1_2__Reg1_1, AMFBS_HasStdExtA, { MCK_GPR, MCK_GPR, MCK_ZeroOffsetMemOpOperand }, },
  { 3338 /* sd */, RISCV::PseudoSD, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_IsRV64, { MCK_GPR, MCK_BareSymbol, MCK_GPR }, },
  { 3338 /* sd */, RISCV::SD, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_IsRV64, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3338 /* sd */, RISCV::SD, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_IsRV64, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3341 /* seqz */, RISCV::SLTIU, Convert__Reg1_0__Reg1_1__imm_95_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3346 /* sext.b */, RISCV::SEXT_B, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 3346 /* sext.b */, RISCV::PseudoSEXT_B, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3353 /* sext.h */, RISCV::SEXT_H, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb, { MCK_GPR, MCK_GPR }, },
  { 3353 /* sext.h */, RISCV::PseudoSEXT_H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3360 /* sext.w */, RISCV::ADDIW, Convert__Reg1_0__Reg1_1__imm_95_0, AMFBS_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3367 /* sfence.inval.ir */, RISCV::SFENCE_INVAL_IR, Convert__imm_95_0__imm_95_0, AMFBS_HasStdExtSvinval, {  }, },
  { 3383 /* sfence.vma */, RISCV::SFENCE_VMA, Convert__regX0__regX0, AMFBS_None, {  }, },
  { 3383 /* sfence.vma */, RISCV::SFENCE_VMA, Convert__Reg1_0__regX0, AMFBS_None, { MCK_GPR }, },
  { 3383 /* sfence.vma */, RISCV::SFENCE_VMA, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3394 /* sfence.w.inval */, RISCV::SFENCE_W_INVAL, Convert__imm_95_0__imm_95_0, AMFBS_HasStdExtSvinval, {  }, },
  { 3409 /* sgt */, RISCV::SLT, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3413 /* sgtu */, RISCV::SLTU, Convert__Reg1_0__Reg1_2__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3418 /* sgtz */, RISCV::SLT, Convert__Reg1_0__regX0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3423 /* sh */, RISCV::PseudoSH, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol, MCK_GPR }, },
  { 3423 /* sh */, RISCV::SH, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3423 /* sh */, RISCV::SH, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3426 /* sh1add */, RISCV::SH1ADD, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3433 /* sh1add.uw */, RISCV::SH1ADD_UW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3443 /* sh2add */, RISCV::SH2ADD, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3450 /* sh2add.uw */, RISCV::SH2ADD_UW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3460 /* sh3add */, RISCV::SH3ADD, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3467 /* sh3add.uw */, RISCV::SH3ADD_UW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3477 /* sha256sig0 */, RISCV::SHA256SIG0, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh, { MCK_GPR, MCK_GPR }, },
  { 3488 /* sha256sig1 */, RISCV::SHA256SIG1, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh, { MCK_GPR, MCK_GPR }, },
  { 3499 /* sha256sum0 */, RISCV::SHA256SUM0, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh, { MCK_GPR, MCK_GPR }, },
  { 3510 /* sha256sum1 */, RISCV::SHA256SUM1, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh, { MCK_GPR, MCK_GPR }, },
  { 3521 /* sha512sig0 */, RISCV::SHA512SIG0, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3532 /* sha512sig0h */, RISCV::SHA512SIG0H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3544 /* sha512sig0l */, RISCV::SHA512SIG0L, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3556 /* sha512sig1 */, RISCV::SHA512SIG1, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3567 /* sha512sig1h */, RISCV::SHA512SIG1H, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3579 /* sha512sig1l */, RISCV::SHA512SIG1L, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3591 /* sha512sum0 */, RISCV::SHA512SUM0, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3602 /* sha512sum0r */, RISCV::SHA512SUM0R, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3614 /* sha512sum1 */, RISCV::SHA512SUM1, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZknh_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 3625 /* sha512sum1r */, RISCV::SHA512SUM1R, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZknh_IsRV32, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3637 /* sinval.vma */, RISCV::SINVAL_VMA, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtSvinval, { MCK_GPR, MCK_GPR }, },
  { 3648 /* sll */, RISCV::SLL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3648 /* sll */, RISCV::SLLI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3652 /* slli */, RISCV::SLLI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3657 /* slli.uw */, RISCV::SLLI_UW, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3665 /* slliw */, RISCV::SLLIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3671 /* sllw */, RISCV::SLLW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3671 /* sllw */, RISCV::SLLIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3676 /* slt */, RISCV::SLT, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3676 /* slt */, RISCV::SLTI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3680 /* slti */, RISCV::SLTI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3685 /* sltiu */, RISCV::SLTIU, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3691 /* sltu */, RISCV::SLTU, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3691 /* sltu */, RISCV::SLTIU, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 3696 /* sltz */, RISCV::SLT, Convert__Reg1_0__Reg1_1__regX0, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3701 /* sm3p0 */, RISCV::SM3P0, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZksh, { MCK_GPR, MCK_GPR }, },
  { 3707 /* sm3p1 */, RISCV::SM3P1, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZksh, { MCK_GPR, MCK_GPR }, },
  { 3713 /* sm4ed */, RISCV::SM4ED, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZksed, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 3719 /* sm4ks */, RISCV::SM4KS, Convert__Reg1_0__Reg1_1__Reg1_2__UImm21_3, AMFBS_HasStdExtZksed, { MCK_GPR, MCK_GPR, MCK_GPR, MCK_UImm2 }, },
  { 3725 /* snez */, RISCV::SLTU, Convert__Reg1_0__regX0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 3730 /* sra */, RISCV::SRA, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3730 /* sra */, RISCV::SRAI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3734 /* srai */, RISCV::SRAI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3739 /* sraiw */, RISCV::SRAIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3745 /* sraw */, RISCV::SRAW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3745 /* sraw */, RISCV::SRAIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3750 /* sret */, RISCV::SRET, Convert__imm_95_0__imm_95_0, AMFBS_None, {  }, },
  { 3755 /* srl */, RISCV::SRL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3755 /* srl */, RISCV::SRLI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3759 /* srli */, RISCV::SRLI, Convert__Reg1_0__Reg1_1__UImmLog2XLen1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_UImmLog2XLen }, },
  { 3764 /* srliw */, RISCV::SRLIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3770 /* srlw */, RISCV::SRLW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3770 /* srlw */, RISCV::SRLIW, Convert__Reg1_0__Reg1_1__UImm51_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_UImm5 }, },
  { 3775 /* sub */, RISCV::SUB, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3779 /* subw */, RISCV::SUBW, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 3784 /* sw */, RISCV::PseudoSW, Convert__Reg1_2__Reg1_0__BareSymbol1_1, AMFBS_None, { MCK_GPR, MCK_BareSymbol, MCK_GPR }, },
  { 3784 /* sw */, RISCV::SW, Convert__Reg1_0__Reg1_2__imm_95_0, AMFBS_None, { MCK_GPR, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3784 /* sw */, RISCV::SW, Convert__Reg1_0__Reg1_3__SImm121_1, AMFBS_None, { MCK_GPR, MCK_SImm12, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 3787 /* tail */, RISCV::PseudoTAIL, Convert__CallSymbol1_0, AMFBS_None, { MCK_CallSymbol }, },
  { 3792 /* th.vmaqa.vv */, RISCV::THVdotVMAQA_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3804 /* th.vmaqa.vx */, RISCV::THVdotVMAQA_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3816 /* th.vmaqasu.vv */, RISCV::THVdotVMAQASU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3830 /* th.vmaqasu.vx */, RISCV::THVdotVMAQASU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3844 /* th.vmaqau.vv */, RISCV::THVdotVMAQAU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3857 /* th.vmaqau.vx */, RISCV::THVdotVMAQAU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3870 /* th.vmaqaus.vx */, RISCV::THVdotVMAQAUS_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVendorXTHeadVdot, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3884 /* unimp */, RISCV::UNIMP, Convert_NoOperands, AMFBS_None, {  }, },
  { 3890 /* unzip */, RISCV::UNZIP_RV32, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbkb_IsRV32, { MCK_GPR, MCK_GPR }, },
  { 3896 /* uret */, RISCV::URET, Convert__imm_95_0__imm_95_0, AMFBS_None, {  }, },
  { 3901 /* vaadd.vv */, RISCV::VAADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3910 /* vaadd.vx */, RISCV::VAADD_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 3919 /* vaaddu.vv */, RISCV::VAADDU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3929 /* vaaddu.vx */, RISCV::VAADDU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 3939 /* vadc.vim */, RISCV::VADC_VIM, Convert__Reg1_0__Reg1_1__SImm51_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_VMV0 }, },
  { 3948 /* vadc.vvm */, RISCV::VADC_VVM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_VMV0 }, },
  { 3957 /* vadc.vxm */, RISCV::VADC_VXM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_VMV0 }, },
  { 3966 /* vadd.vi */, RISCV::VADD_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 3974 /* vadd.vv */, RISCV::VADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 3982 /* vadd.vx */, RISCV::VADD_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 3990 /* vand.vi */, RISCV::VAND_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 3998 /* vand.vv */, RISCV::VAND_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4006 /* vand.vx */, RISCV::VAND_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 4014 /* vasub.vv */, RISCV::VASUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4023 /* vasub.vx */, RISCV::VASUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 4032 /* vasubu.vv */, RISCV::VASUBU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4042 /* vasubu.vx */, RISCV::VASUBU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 4052 /* vcompress.vm */, RISCV::VCOMPRESS_VM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 4065 /* vcpop.m */, RISCV::VCPOP_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4073 /* vdiv.vv */, RISCV::VDIV_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4081 /* vdiv.vx */, RISCV::VDIV_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 4089 /* vdivu.vv */, RISCV::VDIVU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4098 /* vdivu.vx */, RISCV::VDIVU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 4107 /* vfabs.v */, RISCV::VFSGNJX_VV, Convert__Reg1_0__Reg1_1__Reg1_1__reg0, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM }, },
  { 4107 /* vfabs.v */, RISCV::VFSGNJX_VV, Convert__Reg1_0__Reg1_1__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4115 /* vfadd.vf */, RISCV::VFADD_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4124 /* vfadd.vv */, RISCV::VFADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4133 /* vfclass.v */, RISCV::VFCLASS_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4143 /* vfcvt.f.x.v */, RISCV::VFCVT_F_X_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4155 /* vfcvt.f.xu.v */, RISCV::VFCVT_F_XU_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4168 /* vfcvt.rtz.x.f.v */, RISCV::VFCVT_RTZ_X_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4184 /* vfcvt.rtz.xu.f.v */, RISCV::VFCVT_RTZ_XU_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4201 /* vfcvt.x.f.v */, RISCV::VFCVT_X_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4213 /* vfcvt.xu.f.v */, RISCV::VFCVT_XU_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4226 /* vfdiv.vf */, RISCV::VFDIV_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4235 /* vfdiv.vv */, RISCV::VFDIV_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4244 /* vfirst.m */, RISCV::VFIRST_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4253 /* vfmacc.vf */, RISCV::VFMACC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4263 /* vfmacc.vv */, RISCV::VFMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4273 /* vfmadd.vf */, RISCV::VFMADD_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4283 /* vfmadd.vv */, RISCV::VFMADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4293 /* vfmax.vf */, RISCV::VFMAX_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4302 /* vfmax.vv */, RISCV::VFMAX_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4311 /* vfmerge.vfm */, RISCV::VFMERGE_VFM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_VMV0 }, },
  { 4323 /* vfmin.vf */, RISCV::VFMIN_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4332 /* vfmin.vv */, RISCV::VFMIN_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4341 /* vfmsac.vf */, RISCV::VFMSAC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4351 /* vfmsac.vv */, RISCV::VFMSAC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4361 /* vfmsub.vf */, RISCV::VFMSUB_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4371 /* vfmsub.vv */, RISCV::VFMSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4381 /* vfmul.vf */, RISCV::VFMUL_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4390 /* vfmul.vv */, RISCV::VFMUL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4399 /* vfmv.f.s */, RISCV::VFMV_F_S, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructionsAnyF, { MCK_FPR32, MCK_VM }, },
  { 4408 /* vfmv.s.f */, RISCV::VFMV_S_F, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32 }, },
  { 4417 /* vfmv.v.f */, RISCV::VFMV_V_F, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32 }, },
  { 4426 /* vfncvt.f.f.w */, RISCV::VFNCVT_F_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4439 /* vfncvt.f.x.w */, RISCV::VFNCVT_F_X_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4452 /* vfncvt.f.xu.w */, RISCV::VFNCVT_F_XU_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4466 /* vfncvt.rod.f.f.w */, RISCV::VFNCVT_ROD_F_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4483 /* vfncvt.rtz.x.f.w */, RISCV::VFNCVT_RTZ_X_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4500 /* vfncvt.rtz.xu.f.w */, RISCV::VFNCVT_RTZ_XU_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4518 /* vfncvt.x.f.w */, RISCV::VFNCVT_X_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4531 /* vfncvt.xu.f.w */, RISCV::VFNCVT_XU_F_W, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4545 /* vfneg.v */, RISCV::VFSGNJN_VV, Convert__Reg1_0__Reg1_1__Reg1_1__reg0, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM }, },
  { 4545 /* vfneg.v */, RISCV::VFSGNJN_VV, Convert__Reg1_0__Reg1_1__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4553 /* vfnmacc.vf */, RISCV::VFNMACC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4564 /* vfnmacc.vv */, RISCV::VFNMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4575 /* vfnmadd.vf */, RISCV::VFNMADD_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4586 /* vfnmadd.vv */, RISCV::VFNMADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4597 /* vfnmsac.vf */, RISCV::VFNMSAC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4608 /* vfnmsac.vv */, RISCV::VFNMSAC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4619 /* vfnmsub.vf */, RISCV::VFNMSUB_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4630 /* vfnmsub.vv */, RISCV::VFNMSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4641 /* vfrdiv.vf */, RISCV::VFRDIV_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4651 /* vfrec7.v */, RISCV::VFREC7_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4660 /* vfredmax.vs */, RISCV::VFREDMAX_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4672 /* vfredmin.vs */, RISCV::VFREDMIN_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4684 /* vfredosum.vs */, RISCV::VFREDOSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4697 /* vfredsum.vs */, RISCV::VFREDUSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4709 /* vfredusum.vs */, RISCV::VFREDUSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4722 /* vfrsqrt7.v */, RISCV::VFRSQRT7_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4733 /* vfrsub.vf */, RISCV::VFRSUB_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4743 /* vfsgnj.vf */, RISCV::VFSGNJ_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4753 /* vfsgnj.vv */, RISCV::VFSGNJ_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4763 /* vfsgnjn.vf */, RISCV::VFSGNJN_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4774 /* vfsgnjn.vv */, RISCV::VFSGNJN_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4785 /* vfsgnjx.vf */, RISCV::VFSGNJX_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4796 /* vfsgnjx.vv */, RISCV::VFSGNJX_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4807 /* vfslide1down.vf */, RISCV::VFSLIDE1DOWN_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4823 /* vfslide1up.vf */, RISCV::VFSLIDE1UP_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4837 /* vfsqrt.v */, RISCV::VFSQRT_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4846 /* vfsub.vf */, RISCV::VFSUB_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4855 /* vfsub.vv */, RISCV::VFSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4864 /* vfwadd.vf */, RISCV::VFWADD_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4874 /* vfwadd.vv */, RISCV::VFWADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4884 /* vfwadd.wf */, RISCV::VFWADD_WF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 4894 /* vfwadd.wv */, RISCV::VFWADD_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4904 /* vfwcvt.f.f.v */, RISCV::VFWCVT_F_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4917 /* vfwcvt.f.x.v */, RISCV::VFWCVT_F_X_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4930 /* vfwcvt.f.xu.v */, RISCV::VFWCVT_F_XU_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4944 /* vfwcvt.rtz.x.f.v */, RISCV::VFWCVT_RTZ_X_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4961 /* vfwcvt.rtz.xu.f.v */, RISCV::VFWCVT_RTZ_XU_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4979 /* vfwcvt.x.f.v */, RISCV::VFWCVT_X_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 4992 /* vfwcvt.xu.f.v */, RISCV::VFWCVT_XU_F_V, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5006 /* vfwmacc.vf */, RISCV::VFWMACC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5017 /* vfwmacc.vv */, RISCV::VFWMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5028 /* vfwmsac.vf */, RISCV::VFWMSAC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5039 /* vfwmsac.vv */, RISCV::VFWMSAC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5050 /* vfwmul.vf */, RISCV::VFWMUL_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 5060 /* vfwmul.vv */, RISCV::VFWMUL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5070 /* vfwnmacc.vf */, RISCV::VFWNMACC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5082 /* vfwnmacc.vv */, RISCV::VFWNMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5094 /* vfwnmsac.vf */, RISCV::VFWNMSAC_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_FPR32, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5106 /* vfwnmsac.vv */, RISCV::VFWNMSAC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5118 /* vfwredosum.vs */, RISCV::VFWREDOSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5132 /* vfwredsum.vs */, RISCV::VFWREDUSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5145 /* vfwredusum.vs */, RISCV::VFWREDUSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5159 /* vfwsub.vf */, RISCV::VFWSUB_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 5169 /* vfwsub.vv */, RISCV::VFWSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5179 /* vfwsub.wf */, RISCV::VFWSUB_WF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 5189 /* vfwsub.wv */, RISCV::VFWSUB_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5199 /* vid.v */, RISCV::VID_V, Convert__Reg1_0__RVVMaskRegOpOperand1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5205 /* viota.m */, RISCV::VIOTA_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5213 /* vl1r.v */, RISCV::VL1RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5220 /* vl1re16.v */, RISCV::VL1RE16_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5230 /* vl1re32.v */, RISCV::VL1RE32_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5240 /* vl1re64.v */, RISCV::VL1RE64_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5250 /* vl1re8.v */, RISCV::VL1RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5259 /* vl2r.v */, RISCV::VL2RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5266 /* vl2re16.v */, RISCV::VL2RE16_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5276 /* vl2re32.v */, RISCV::VL2RE32_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5286 /* vl2re64.v */, RISCV::VL2RE64_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructionsI64, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5296 /* vl2re8.v */, RISCV::VL2RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5305 /* vl4r.v */, RISCV::VL4RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5312 /* vl4re16.v */, RISCV::VL4RE16_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5322 /* vl4re32.v */, RISCV::VL4RE32_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5332 /* vl4re64.v */, RISCV::VL4RE64_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructionsI64, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5342 /* vl4re8.v */, RISCV::VL4RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5351 /* vl8r.v */, RISCV::VL8RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5358 /* vl8re16.v */, RISCV::VL8RE16_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5368 /* vl8re32.v */, RISCV::VL8RE32_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5378 /* vl8re64.v */, RISCV::VL8RE64_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructionsI64, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5388 /* vl8re8.v */, RISCV::VL8RE8_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5397 /* vle1.v */, RISCV::VLM_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5404 /* vle16.v */, RISCV::VLE16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5412 /* vle16ff.v */, RISCV::VLE16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5422 /* vle32.v */, RISCV::VLE32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5430 /* vle32ff.v */, RISCV::VLE32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5440 /* vle64.v */, RISCV::VLE64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5448 /* vle64ff.v */, RISCV::VLE64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5458 /* vle8.v */, RISCV::VLE8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5465 /* vle8ff.v */, RISCV::VLE8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5474 /* vlm.v */, RISCV::VLM_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 5480 /* vloxei16.v */, RISCV::VLOXEI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5491 /* vloxei32.v */, RISCV::VLOXEI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5502 /* vloxei64.v */, RISCV::VLOXEI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_IsRV64_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5513 /* vloxei8.v */, RISCV::VLOXEI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5523 /* vloxseg2ei16.v */, RISCV::VLOXSEG2EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5538 /* vloxseg2ei32.v */, RISCV::VLOXSEG2EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5553 /* vloxseg2ei64.v */, RISCV::VLOXSEG2EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5568 /* vloxseg2ei8.v */, RISCV::VLOXSEG2EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5582 /* vloxseg3ei16.v */, RISCV::VLOXSEG3EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5597 /* vloxseg3ei32.v */, RISCV::VLOXSEG3EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5612 /* vloxseg3ei64.v */, RISCV::VLOXSEG3EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5627 /* vloxseg3ei8.v */, RISCV::VLOXSEG3EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5641 /* vloxseg4ei16.v */, RISCV::VLOXSEG4EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5656 /* vloxseg4ei32.v */, RISCV::VLOXSEG4EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5671 /* vloxseg4ei64.v */, RISCV::VLOXSEG4EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5686 /* vloxseg4ei8.v */, RISCV::VLOXSEG4EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5700 /* vloxseg5ei16.v */, RISCV::VLOXSEG5EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5715 /* vloxseg5ei32.v */, RISCV::VLOXSEG5EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5730 /* vloxseg5ei64.v */, RISCV::VLOXSEG5EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5745 /* vloxseg5ei8.v */, RISCV::VLOXSEG5EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5759 /* vloxseg6ei16.v */, RISCV::VLOXSEG6EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5774 /* vloxseg6ei32.v */, RISCV::VLOXSEG6EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5789 /* vloxseg6ei64.v */, RISCV::VLOXSEG6EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5804 /* vloxseg6ei8.v */, RISCV::VLOXSEG6EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5818 /* vloxseg7ei16.v */, RISCV::VLOXSEG7EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5833 /* vloxseg7ei32.v */, RISCV::VLOXSEG7EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5848 /* vloxseg7ei64.v */, RISCV::VLOXSEG7EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5863 /* vloxseg7ei8.v */, RISCV::VLOXSEG7EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5877 /* vloxseg8ei16.v */, RISCV::VLOXSEG8EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5892 /* vloxseg8ei32.v */, RISCV::VLOXSEG8EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5907 /* vloxseg8ei64.v */, RISCV::VLOXSEG8EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5922 /* vloxseg8ei8.v */, RISCV::VLOXSEG8EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 5936 /* vlse16.v */, RISCV::VLSE16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 5945 /* vlse32.v */, RISCV::VLSE32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 5954 /* vlse64.v */, RISCV::VLSE64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 5963 /* vlse8.v */, RISCV::VLSE8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 5971 /* vlseg2e16.v */, RISCV::VLSEG2E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5983 /* vlseg2e16ff.v */, RISCV::VLSEG2E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 5997 /* vlseg2e32.v */, RISCV::VLSEG2E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6009 /* vlseg2e32ff.v */, RISCV::VLSEG2E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6023 /* vlseg2e64.v */, RISCV::VLSEG2E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6035 /* vlseg2e64ff.v */, RISCV::VLSEG2E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6049 /* vlseg2e8.v */, RISCV::VLSEG2E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6060 /* vlseg2e8ff.v */, RISCV::VLSEG2E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6073 /* vlseg3e16.v */, RISCV::VLSEG3E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6085 /* vlseg3e16ff.v */, RISCV::VLSEG3E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6099 /* vlseg3e32.v */, RISCV::VLSEG3E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6111 /* vlseg3e32ff.v */, RISCV::VLSEG3E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6125 /* vlseg3e64.v */, RISCV::VLSEG3E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6137 /* vlseg3e64ff.v */, RISCV::VLSEG3E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6151 /* vlseg3e8.v */, RISCV::VLSEG3E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6162 /* vlseg3e8ff.v */, RISCV::VLSEG3E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6175 /* vlseg4e16.v */, RISCV::VLSEG4E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6187 /* vlseg4e16ff.v */, RISCV::VLSEG4E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6201 /* vlseg4e32.v */, RISCV::VLSEG4E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6213 /* vlseg4e32ff.v */, RISCV::VLSEG4E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6227 /* vlseg4e64.v */, RISCV::VLSEG4E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6239 /* vlseg4e64ff.v */, RISCV::VLSEG4E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6253 /* vlseg4e8.v */, RISCV::VLSEG4E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6264 /* vlseg4e8ff.v */, RISCV::VLSEG4E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6277 /* vlseg5e16.v */, RISCV::VLSEG5E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6289 /* vlseg5e16ff.v */, RISCV::VLSEG5E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6303 /* vlseg5e32.v */, RISCV::VLSEG5E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6315 /* vlseg5e32ff.v */, RISCV::VLSEG5E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6329 /* vlseg5e64.v */, RISCV::VLSEG5E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6341 /* vlseg5e64ff.v */, RISCV::VLSEG5E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6355 /* vlseg5e8.v */, RISCV::VLSEG5E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6366 /* vlseg5e8ff.v */, RISCV::VLSEG5E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6379 /* vlseg6e16.v */, RISCV::VLSEG6E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6391 /* vlseg6e16ff.v */, RISCV::VLSEG6E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6405 /* vlseg6e32.v */, RISCV::VLSEG6E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6417 /* vlseg6e32ff.v */, RISCV::VLSEG6E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6431 /* vlseg6e64.v */, RISCV::VLSEG6E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6443 /* vlseg6e64ff.v */, RISCV::VLSEG6E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6457 /* vlseg6e8.v */, RISCV::VLSEG6E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6468 /* vlseg6e8ff.v */, RISCV::VLSEG6E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6481 /* vlseg7e16.v */, RISCV::VLSEG7E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6493 /* vlseg7e16ff.v */, RISCV::VLSEG7E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6507 /* vlseg7e32.v */, RISCV::VLSEG7E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6519 /* vlseg7e32ff.v */, RISCV::VLSEG7E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6533 /* vlseg7e64.v */, RISCV::VLSEG7E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6545 /* vlseg7e64ff.v */, RISCV::VLSEG7E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6559 /* vlseg7e8.v */, RISCV::VLSEG7E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6570 /* vlseg7e8ff.v */, RISCV::VLSEG7E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6583 /* vlseg8e16.v */, RISCV::VLSEG8E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6595 /* vlseg8e16ff.v */, RISCV::VLSEG8E16FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6609 /* vlseg8e32.v */, RISCV::VLSEG8E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6621 /* vlseg8e32ff.v */, RISCV::VLSEG8E32FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6635 /* vlseg8e64.v */, RISCV::VLSEG8E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6647 /* vlseg8e64ff.v */, RISCV::VLSEG8E64FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6661 /* vlseg8e8.v */, RISCV::VLSEG8E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6672 /* vlseg8e8ff.v */, RISCV::VLSEG8E8FF_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 6685 /* vlsseg2e16.v */, RISCV::VLSSEG2E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6698 /* vlsseg2e32.v */, RISCV::VLSSEG2E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6711 /* vlsseg2e64.v */, RISCV::VLSSEG2E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6724 /* vlsseg2e8.v */, RISCV::VLSSEG2E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6736 /* vlsseg3e16.v */, RISCV::VLSSEG3E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6749 /* vlsseg3e32.v */, RISCV::VLSSEG3E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6762 /* vlsseg3e64.v */, RISCV::VLSSEG3E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6775 /* vlsseg3e8.v */, RISCV::VLSSEG3E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6787 /* vlsseg4e16.v */, RISCV::VLSSEG4E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6800 /* vlsseg4e32.v */, RISCV::VLSSEG4E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6813 /* vlsseg4e64.v */, RISCV::VLSSEG4E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6826 /* vlsseg4e8.v */, RISCV::VLSSEG4E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6838 /* vlsseg5e16.v */, RISCV::VLSSEG5E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6851 /* vlsseg5e32.v */, RISCV::VLSSEG5E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6864 /* vlsseg5e64.v */, RISCV::VLSSEG5E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6877 /* vlsseg5e8.v */, RISCV::VLSSEG5E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6889 /* vlsseg6e16.v */, RISCV::VLSSEG6E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6902 /* vlsseg6e32.v */, RISCV::VLSSEG6E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6915 /* vlsseg6e64.v */, RISCV::VLSSEG6E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6928 /* vlsseg6e8.v */, RISCV::VLSSEG6E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6940 /* vlsseg7e16.v */, RISCV::VLSSEG7E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6953 /* vlsseg7e32.v */, RISCV::VLSSEG7E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6966 /* vlsseg7e64.v */, RISCV::VLSSEG7E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6979 /* vlsseg7e8.v */, RISCV::VLSSEG7E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 6991 /* vlsseg8e16.v */, RISCV::VLSSEG8E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7004 /* vlsseg8e32.v */, RISCV::VLSSEG8E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7017 /* vlsseg8e64.v */, RISCV::VLSSEG8E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7030 /* vlsseg8e8.v */, RISCV::VLSSEG8E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7042 /* vluxei16.v */, RISCV::VLUXEI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7053 /* vluxei32.v */, RISCV::VLUXEI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7064 /* vluxei64.v */, RISCV::VLUXEI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_IsRV64_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7075 /* vluxei8.v */, RISCV::VLUXEI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7085 /* vluxseg2ei16.v */, RISCV::VLUXSEG2EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7100 /* vluxseg2ei32.v */, RISCV::VLUXSEG2EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7115 /* vluxseg2ei64.v */, RISCV::VLUXSEG2EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7130 /* vluxseg2ei8.v */, RISCV::VLUXSEG2EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7144 /* vluxseg3ei16.v */, RISCV::VLUXSEG3EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7159 /* vluxseg3ei32.v */, RISCV::VLUXSEG3EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7174 /* vluxseg3ei64.v */, RISCV::VLUXSEG3EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7189 /* vluxseg3ei8.v */, RISCV::VLUXSEG3EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7203 /* vluxseg4ei16.v */, RISCV::VLUXSEG4EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7218 /* vluxseg4ei32.v */, RISCV::VLUXSEG4EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7233 /* vluxseg4ei64.v */, RISCV::VLUXSEG4EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7248 /* vluxseg4ei8.v */, RISCV::VLUXSEG4EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7262 /* vluxseg5ei16.v */, RISCV::VLUXSEG5EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7277 /* vluxseg5ei32.v */, RISCV::VLUXSEG5EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7292 /* vluxseg5ei64.v */, RISCV::VLUXSEG5EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7307 /* vluxseg5ei8.v */, RISCV::VLUXSEG5EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7321 /* vluxseg6ei16.v */, RISCV::VLUXSEG6EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7336 /* vluxseg6ei32.v */, RISCV::VLUXSEG6EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7351 /* vluxseg6ei64.v */, RISCV::VLUXSEG6EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7366 /* vluxseg6ei8.v */, RISCV::VLUXSEG6EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7380 /* vluxseg7ei16.v */, RISCV::VLUXSEG7EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7395 /* vluxseg7ei32.v */, RISCV::VLUXSEG7EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7410 /* vluxseg7ei64.v */, RISCV::VLUXSEG7EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7425 /* vluxseg7ei8.v */, RISCV::VLUXSEG7EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7439 /* vluxseg8ei16.v */, RISCV::VLUXSEG8EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7454 /* vluxseg8ei32.v */, RISCV::VLUXSEG8EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7469 /* vluxseg8ei64.v */, RISCV::VLUXSEG8EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7484 /* vluxseg8ei8.v */, RISCV::VLUXSEG8EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7498 /* vmacc.vv */, RISCV::VMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7507 /* vmacc.vx */, RISCV::VMACC_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7516 /* vmadc.vi */, RISCV::VMADC_VI, Convert__Reg1_0__Reg1_1__SImm51_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5 }, },
  { 7525 /* vmadc.vim */, RISCV::VMADC_VIM, Convert__Reg1_0__Reg1_1__SImm51_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_VMV0 }, },
  { 7535 /* vmadc.vv */, RISCV::VMADC_VV, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7544 /* vmadc.vvm */, RISCV::VMADC_VVM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_VMV0 }, },
  { 7554 /* vmadc.vx */, RISCV::VMADC_VX, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR }, },
  { 7563 /* vmadc.vxm */, RISCV::VMADC_VXM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_VMV0 }, },
  { 7573 /* vmadd.vv */, RISCV::VMADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7582 /* vmadd.vx */, RISCV::VMADD_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7591 /* vmand.mm */, RISCV::VMAND_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7600 /* vmandn.mm */, RISCV::VMANDN_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7610 /* vmandnot.mm */, RISCV::VMANDN_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7622 /* vmax.vv */, RISCV::VMAX_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7630 /* vmax.vx */, RISCV::VMAX_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7638 /* vmaxu.vv */, RISCV::VMAXU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7647 /* vmaxu.vx */, RISCV::VMAXU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7656 /* vmclr.m */, RISCV::VMXOR_MM, Convert__Reg1_0__Reg1_0__Reg1_0, AMFBS_HasVInstructions, { MCK_VM }, },
  { 7664 /* vmerge.vim */, RISCV::VMERGE_VIM, Convert__Reg1_0__Reg1_1__SImm51_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_VMV0 }, },
  { 7675 /* vmerge.vvm */, RISCV::VMERGE_VVM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_VMV0 }, },
  { 7686 /* vmerge.vxm */, RISCV::VMERGE_VXM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_VMV0 }, },
  { 7697 /* vmfeq.vf */, RISCV::VMFEQ_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7706 /* vmfeq.vv */, RISCV::VMFEQ_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7715 /* vmfge.vf */, RISCV::VMFGE_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7724 /* vmfge.vv */, RISCV::VMFLE_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7733 /* vmfgt.vf */, RISCV::VMFGT_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7742 /* vmfgt.vv */, RISCV::VMFLT_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7751 /* vmfle.vf */, RISCV::VMFLE_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7760 /* vmfle.vv */, RISCV::VMFLE_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7769 /* vmflt.vf */, RISCV::VMFLT_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7778 /* vmflt.vv */, RISCV::VMFLT_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7787 /* vmfne.vf */, RISCV::VMFNE_VF, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_FPR32, MCK_RVVMaskRegOpOperand }, },
  { 7796 /* vmfne.vv */, RISCV::VMFNE_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructionsAnyF, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7805 /* vmin.vv */, RISCV::VMIN_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7813 /* vmin.vx */, RISCV::VMIN_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7821 /* vminu.vv */, RISCV::VMINU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7830 /* vminu.vx */, RISCV::VMINU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7839 /* vmmv.m */, RISCV::VMAND_MM, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 7846 /* vmnand.mm */, RISCV::VMNAND_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7856 /* vmnor.mm */, RISCV::VMNOR_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7865 /* vmnot.m */, RISCV::VMNAND_MM, Convert__Reg1_0__Reg1_1__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 7873 /* vmor.mm */, RISCV::VMOR_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7881 /* vmorn.mm */, RISCV::VMORN_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7890 /* vmornot.mm */, RISCV::VMORN_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7901 /* vmsbc.vv */, RISCV::VMSBC_VV, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 7910 /* vmsbc.vvm */, RISCV::VMSBC_VVM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_VMV0 }, },
  { 7920 /* vmsbc.vx */, RISCV::VMSBC_VX, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR }, },
  { 7929 /* vmsbc.vxm */, RISCV::VMSBC_VXM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_VMV0 }, },
  { 7939 /* vmsbf.m */, RISCV::VMSBF_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7947 /* vmseq.vi */, RISCV::VMSEQ_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 7956 /* vmseq.vv */, RISCV::VMSEQ_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 7965 /* vmseq.vx */, RISCV::VMSEQ_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 7974 /* vmset.m */, RISCV::VMXNOR_MM, Convert__Reg1_0__Reg1_0__Reg1_0, AMFBS_HasVInstructions, { MCK_VM }, },
  { 7982 /* vmsge.vi */, RISCV::PseudoVMSGE_VI, Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5Plus1, MCK_RVVMaskRegOpOperand }, },
  { 7991 /* vmsge.vv */, RISCV::VMSLE_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8000 /* vmsge.vx */, RISCV::PseudoVMSGE_VX, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR }, },
  { 8000 /* vmsge.vx */, RISCV::PseudoVMSGE_VX_M, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VRNoV0, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8000 /* vmsge.vx */, RISCV::PseudoVMSGE_VX_M_T, Convert__Reg1_0__Reg1_4__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand, MCK_VRNoV0 }, },
  { 8009 /* vmsgeu.vi */, RISCV::PseudoVMSGEU_VI, Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5Plus1, MCK_RVVMaskRegOpOperand }, },
  { 8019 /* vmsgeu.vv */, RISCV::VMSLEU_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8029 /* vmsgeu.vx */, RISCV::PseudoVMSGEU_VX, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR }, },
  { 8029 /* vmsgeu.vx */, RISCV::PseudoVMSGEU_VX_M, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VRNoV0, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8029 /* vmsgeu.vx */, RISCV::PseudoVMSGEU_VX_M_T, Convert__Reg1_0__Reg1_4__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand, MCK_VRNoV0 }, },
  { 8039 /* vmsgt.vi */, RISCV::VMSGT_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8048 /* vmsgt.vv */, RISCV::VMSLT_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8057 /* vmsgt.vx */, RISCV::VMSGT_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8066 /* vmsgtu.vi */, RISCV::VMSGTU_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8076 /* vmsgtu.vv */, RISCV::VMSLTU_VV, Convert__Reg1_0__Reg1_2__Reg1_1__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8086 /* vmsgtu.vx */, RISCV::VMSGTU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8096 /* vmsif.m */, RISCV::VMSIF_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8104 /* vmsle.vi */, RISCV::VMSLE_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8113 /* vmsle.vv */, RISCV::VMSLE_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8122 /* vmsle.vx */, RISCV::VMSLE_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8131 /* vmsleu.vi */, RISCV::VMSLEU_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8141 /* vmsleu.vv */, RISCV::VMSLEU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8151 /* vmsleu.vx */, RISCV::VMSLEU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8161 /* vmslt.vi */, RISCV::PseudoVMSLT_VI, Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5Plus1, MCK_RVVMaskRegOpOperand }, },
  { 8170 /* vmslt.vv */, RISCV::VMSLT_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8179 /* vmslt.vx */, RISCV::VMSLT_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8188 /* vmsltu.vi */, RISCV::PseudoVMSLTU_VI, Convert__Reg1_0__Reg1_1__SImm5Plus11_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5Plus1, MCK_RVVMaskRegOpOperand }, },
  { 8198 /* vmsltu.vv */, RISCV::VMSLTU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8208 /* vmsltu.vx */, RISCV::VMSLTU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8218 /* vmsne.vi */, RISCV::VMSNE_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8227 /* vmsne.vv */, RISCV::VMSNE_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8236 /* vmsne.vx */, RISCV::VMSNE_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8245 /* vmsof.m */, RISCV::VMSOF_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8253 /* vmul.vv */, RISCV::VMUL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8261 /* vmul.vx */, RISCV::VMUL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8269 /* vmulh.vv */, RISCV::VMULH_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8278 /* vmulh.vx */, RISCV::VMULH_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8287 /* vmulhsu.vv */, RISCV::VMULHSU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8298 /* vmulhsu.vx */, RISCV::VMULHSU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8309 /* vmulhu.vv */, RISCV::VMULHU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8319 /* vmulhu.vx */, RISCV::VMULHU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8329 /* vmv.s.x */, RISCV::VMV_S_X, Convert__Reg1_0__Tie0_1_1__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR }, },
  { 8337 /* vmv.v.i */, RISCV::VMV_V_I, Convert__Reg1_0__SImm51_1, AMFBS_HasVInstructions, { MCK_VM, MCK_SImm5 }, },
  { 8345 /* vmv.v.v */, RISCV::VMV_V_V, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 8353 /* vmv.v.x */, RISCV::VMV_V_X, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR }, },
  { 8361 /* vmv.x.s */, RISCV::VMV_X_S, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_GPR, MCK_VM }, },
  { 8369 /* vmv1r.v */, RISCV::VMV1R_V, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 8377 /* vmv2r.v */, RISCV::VMV2R_V, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VRM2, MCK_VRM2 }, },
  { 8385 /* vmv4r.v */, RISCV::VMV4R_V, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VRM4, MCK_VRM4 }, },
  { 8393 /* vmv8r.v */, RISCV::VMV8R_V, Convert__Reg1_0__Reg1_1, AMFBS_HasVInstructions, { MCK_VRM8, MCK_VRM8 }, },
  { 8401 /* vmxnor.mm */, RISCV::VMXNOR_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 8411 /* vmxor.mm */, RISCV::VMXOR_MM, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM }, },
  { 8420 /* vnclip.wi */, RISCV::VNCLIP_WI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 8430 /* vnclip.wv */, RISCV::VNCLIP_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8440 /* vnclip.wx */, RISCV::VNCLIP_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8450 /* vnclipu.wi */, RISCV::VNCLIPU_WI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 8461 /* vnclipu.wv */, RISCV::VNCLIPU_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8472 /* vnclipu.wx */, RISCV::VNCLIPU_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8483 /* vncvt.x.x.w */, RISCV::VNSRL_WX, Convert__Reg1_0__Reg1_1__regX0__reg0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 8483 /* vncvt.x.x.w */, RISCV::VNSRL_WX, Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8495 /* vneg.v */, RISCV::VRSUB_VX, Convert__Reg1_0__Reg1_1__regX0__reg0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 8495 /* vneg.v */, RISCV::VRSUB_VX, Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8502 /* vnmsac.vv */, RISCV::VNMSAC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8512 /* vnmsac.vx */, RISCV::VNMSAC_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8522 /* vnmsub.vv */, RISCV::VNMSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8532 /* vnmsub.vx */, RISCV::VNMSUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8542 /* vnot.v */, RISCV::VXOR_VI, Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__reg0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 8542 /* vnot.v */, RISCV::VXOR_VI, Convert__Reg1_0__Reg1_1__imm_95__MINUS_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8549 /* vnsra.wi */, RISCV::VNSRA_WI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 8558 /* vnsra.wv */, RISCV::VNSRA_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8567 /* vnsra.wx */, RISCV::VNSRA_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8576 /* vnsrl.wi */, RISCV::VNSRL_WI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 8585 /* vnsrl.wv */, RISCV::VNSRL_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8594 /* vnsrl.wx */, RISCV::VNSRL_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8603 /* vor.vi */, RISCV::VOR_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8610 /* vor.vv */, RISCV::VOR_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8617 /* vor.vx */, RISCV::VOR_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8624 /* vpopc.m */, RISCV::VCPOP_M, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8632 /* vredand.vs */, RISCV::VREDAND_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8643 /* vredmax.vs */, RISCV::VREDMAX_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8654 /* vredmaxu.vs */, RISCV::VREDMAXU_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8666 /* vredmin.vs */, RISCV::VREDMIN_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8677 /* vredminu.vs */, RISCV::VREDMINU_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8689 /* vredor.vs */, RISCV::VREDOR_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8699 /* vredsum.vs */, RISCV::VREDSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8710 /* vredxor.vs */, RISCV::VREDXOR_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8721 /* vrem.vv */, RISCV::VREM_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8729 /* vrem.vx */, RISCV::VREM_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8737 /* vremu.vv */, RISCV::VREMU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8746 /* vremu.vx */, RISCV::VREMU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8755 /* vrgather.vi */, RISCV::VRGATHER_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 8767 /* vrgather.vv */, RISCV::VRGATHER_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8779 /* vrgather.vx */, RISCV::VRGATHER_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8791 /* vrgatherei16.vv */, RISCV::VRGATHEREI16_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8807 /* vrsub.vi */, RISCV::VRSUB_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8816 /* vrsub.vx */, RISCV::VRSUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8825 /* vs1r.v */, RISCV::VS1R_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 8832 /* vs2r.v */, RISCV::VS2R_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM2, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 8839 /* vs4r.v */, RISCV::VS4R_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM4, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 8846 /* vs8r.v */, RISCV::VS8R_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VRM8, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 8853 /* vsadd.vi */, RISCV::VSADD_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8862 /* vsadd.vv */, RISCV::VSADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8871 /* vsadd.vx */, RISCV::VSADD_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8880 /* vsaddu.vi */, RISCV::VSADDU_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 8890 /* vsaddu.vv */, RISCV::VSADDU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 8900 /* vsaddu.vx */, RISCV::VSADDU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 8910 /* vsbc.vvm */, RISCV::VSBC_VVM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_VMV0 }, },
  { 8919 /* vsbc.vxm */, RISCV::VSBC_VXM, Convert__Reg1_0__Reg1_1__Reg1_2__imm_95_0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_VMV0 }, },
  { 8928 /* vse1.v */, RISCV::VSM_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 8935 /* vse16.v */, RISCV::VSE16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 8943 /* vse32.v */, RISCV::VSE32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 8951 /* vse64.v */, RISCV::VSE64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 8959 /* vse8.v */, RISCV::VSE8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 8966 /* vsetivli */, RISCV::VSETIVLI, Convert__Reg1_0__UImm51_1__VTypeI101_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_UImm5, MCK_VTypeI10 }, },
  { 8975 /* vsetvl */, RISCV::VSETVL, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 8982 /* vsetvli */, RISCV::VSETVLI, Convert__Reg1_0__Reg1_1__VTypeI111_2, AMFBS_HasVInstructions, { MCK_GPR, MCK_GPR, MCK_VTypeI11 }, },
  { 8990 /* vsext.vf2 */, RISCV::VSEXT_VF2, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9000 /* vsext.vf4 */, RISCV::VSEXT_VF4, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9010 /* vsext.vf8 */, RISCV::VSEXT_VF8, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9020 /* vslide1down.vx */, RISCV::VSLIDE1DOWN_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9035 /* vslide1up.vx */, RISCV::VSLIDE1UP_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9048 /* vslidedown.vi */, RISCV::VSLIDEDOWN_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 9062 /* vslidedown.vx */, RISCV::VSLIDEDOWN_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9076 /* vslideup.vi */, RISCV::VSLIDEUP_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 9088 /* vslideup.vx */, RISCV::VSLIDEUP_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9100 /* vsll.vi */, RISCV::VSLL_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 9108 /* vsll.vv */, RISCV::VSLL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9116 /* vsll.vx */, RISCV::VSLL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9124 /* vsm.v */, RISCV::VSM_V, Convert__Reg1_0__Reg1_2, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_ }, },
  { 9130 /* vsmul.vv */, RISCV::VSMUL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9139 /* vsmul.vx */, RISCV::VSMUL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9148 /* vsoxei16.v */, RISCV::VSOXEI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9159 /* vsoxei32.v */, RISCV::VSOXEI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9170 /* vsoxei64.v */, RISCV::VSOXEI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_IsRV64_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9181 /* vsoxei8.v */, RISCV::VSOXEI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9191 /* vsoxseg2ei16.v */, RISCV::VSOXSEG2EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9206 /* vsoxseg2ei32.v */, RISCV::VSOXSEG2EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9221 /* vsoxseg2ei64.v */, RISCV::VSOXSEG2EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9236 /* vsoxseg2ei8.v */, RISCV::VSOXSEG2EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9250 /* vsoxseg3ei16.v */, RISCV::VSOXSEG3EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9265 /* vsoxseg3ei32.v */, RISCV::VSOXSEG3EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9280 /* vsoxseg3ei64.v */, RISCV::VSOXSEG3EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9295 /* vsoxseg3ei8.v */, RISCV::VSOXSEG3EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9309 /* vsoxseg4ei16.v */, RISCV::VSOXSEG4EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9324 /* vsoxseg4ei32.v */, RISCV::VSOXSEG4EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9339 /* vsoxseg4ei64.v */, RISCV::VSOXSEG4EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9354 /* vsoxseg4ei8.v */, RISCV::VSOXSEG4EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9368 /* vsoxseg5ei16.v */, RISCV::VSOXSEG5EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9383 /* vsoxseg5ei32.v */, RISCV::VSOXSEG5EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9398 /* vsoxseg5ei64.v */, RISCV::VSOXSEG5EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9413 /* vsoxseg5ei8.v */, RISCV::VSOXSEG5EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9427 /* vsoxseg6ei16.v */, RISCV::VSOXSEG6EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9442 /* vsoxseg6ei32.v */, RISCV::VSOXSEG6EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9457 /* vsoxseg6ei64.v */, RISCV::VSOXSEG6EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9472 /* vsoxseg6ei8.v */, RISCV::VSOXSEG6EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9486 /* vsoxseg7ei16.v */, RISCV::VSOXSEG7EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9501 /* vsoxseg7ei32.v */, RISCV::VSOXSEG7EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9516 /* vsoxseg7ei64.v */, RISCV::VSOXSEG7EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9531 /* vsoxseg7ei8.v */, RISCV::VSOXSEG7EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9545 /* vsoxseg8ei16.v */, RISCV::VSOXSEG8EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9560 /* vsoxseg8ei32.v */, RISCV::VSOXSEG8EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9575 /* vsoxseg8ei64.v */, RISCV::VSOXSEG8EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9590 /* vsoxseg8ei8.v */, RISCV::VSOXSEG8EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9604 /* vsra.vi */, RISCV::VSRA_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 9612 /* vsra.vv */, RISCV::VSRA_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9620 /* vsra.vx */, RISCV::VSRA_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9628 /* vsrl.vi */, RISCV::VSRL_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 9636 /* vsrl.vv */, RISCV::VSRL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 9644 /* vsrl.vx */, RISCV::VSRL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9652 /* vsse16.v */, RISCV::VSSE16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9661 /* vsse32.v */, RISCV::VSSE32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9670 /* vsse64.v */, RISCV::VSSE64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9679 /* vsse8.v */, RISCV::VSSE8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 9687 /* vsseg2e16.v */, RISCV::VSSEG2E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9699 /* vsseg2e32.v */, RISCV::VSSEG2E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9711 /* vsseg2e64.v */, RISCV::VSSEG2E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9723 /* vsseg2e8.v */, RISCV::VSSEG2E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9734 /* vsseg3e16.v */, RISCV::VSSEG3E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9746 /* vsseg3e32.v */, RISCV::VSSEG3E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9758 /* vsseg3e64.v */, RISCV::VSSEG3E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9770 /* vsseg3e8.v */, RISCV::VSSEG3E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9781 /* vsseg4e16.v */, RISCV::VSSEG4E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9793 /* vsseg4e32.v */, RISCV::VSSEG4E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9805 /* vsseg4e64.v */, RISCV::VSSEG4E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9817 /* vsseg4e8.v */, RISCV::VSSEG4E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9828 /* vsseg5e16.v */, RISCV::VSSEG5E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9840 /* vsseg5e32.v */, RISCV::VSSEG5E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9852 /* vsseg5e64.v */, RISCV::VSSEG5E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9864 /* vsseg5e8.v */, RISCV::VSSEG5E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9875 /* vsseg6e16.v */, RISCV::VSSEG6E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9887 /* vsseg6e32.v */, RISCV::VSSEG6E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9899 /* vsseg6e64.v */, RISCV::VSSEG6E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9911 /* vsseg6e8.v */, RISCV::VSSEG6E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9922 /* vsseg7e16.v */, RISCV::VSSEG7E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9934 /* vsseg7e32.v */, RISCV::VSSEG7E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9946 /* vsseg7e64.v */, RISCV::VSSEG7E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9958 /* vsseg7e8.v */, RISCV::VSSEG7E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9969 /* vsseg8e16.v */, RISCV::VSSEG8E16_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9981 /* vsseg8e32.v */, RISCV::VSSEG8E32_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 9993 /* vsseg8e64.v */, RISCV::VSSEG8E64_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 10005 /* vsseg8e8.v */, RISCV::VSSEG8E8_V, Convert__Reg1_0__Reg1_2__RVVMaskRegOpOperand1_4, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_RVVMaskRegOpOperand }, },
  { 10016 /* vssra.vi */, RISCV::VSSRA_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 10025 /* vssra.vv */, RISCV::VSSRA_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10034 /* vssra.vx */, RISCV::VSSRA_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10043 /* vssrl.vi */, RISCV::VSSRL_VI, Convert__Reg1_0__Reg1_1__UImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_UImm5, MCK_RVVMaskRegOpOperand }, },
  { 10052 /* vssrl.vv */, RISCV::VSSRL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10061 /* vssrl.vx */, RISCV::VSSRL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10070 /* vssseg2e16.v */, RISCV::VSSSEG2E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10083 /* vssseg2e32.v */, RISCV::VSSSEG2E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10096 /* vssseg2e64.v */, RISCV::VSSSEG2E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10109 /* vssseg2e8.v */, RISCV::VSSSEG2E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10121 /* vssseg3e16.v */, RISCV::VSSSEG3E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10134 /* vssseg3e32.v */, RISCV::VSSSEG3E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10147 /* vssseg3e64.v */, RISCV::VSSSEG3E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10160 /* vssseg3e8.v */, RISCV::VSSSEG3E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10172 /* vssseg4e16.v */, RISCV::VSSSEG4E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10185 /* vssseg4e32.v */, RISCV::VSSSEG4E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10198 /* vssseg4e64.v */, RISCV::VSSSEG4E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10211 /* vssseg4e8.v */, RISCV::VSSSEG4E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10223 /* vssseg5e16.v */, RISCV::VSSSEG5E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10236 /* vssseg5e32.v */, RISCV::VSSSEG5E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10249 /* vssseg5e64.v */, RISCV::VSSSEG5E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10262 /* vssseg5e8.v */, RISCV::VSSSEG5E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10274 /* vssseg6e16.v */, RISCV::VSSSEG6E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10287 /* vssseg6e32.v */, RISCV::VSSSEG6E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10300 /* vssseg6e64.v */, RISCV::VSSSEG6E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10313 /* vssseg6e8.v */, RISCV::VSSSEG6E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10325 /* vssseg7e16.v */, RISCV::VSSSEG7E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10338 /* vssseg7e32.v */, RISCV::VSSSEG7E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10351 /* vssseg7e64.v */, RISCV::VSSSEG7E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10364 /* vssseg7e8.v */, RISCV::VSSSEG7E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10376 /* vssseg8e16.v */, RISCV::VSSSEG8E16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10389 /* vssseg8e32.v */, RISCV::VSSSEG8E32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10402 /* vssseg8e64.v */, RISCV::VSSSEG8E64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10415 /* vssseg8e8.v */, RISCV::VSSSEG8E8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10427 /* vssub.vv */, RISCV::VSSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10436 /* vssub.vx */, RISCV::VSSUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10445 /* vssubu.vv */, RISCV::VSSUBU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10455 /* vssubu.vx */, RISCV::VSSUBU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10465 /* vsub.vv */, RISCV::VSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10473 /* vsub.vx */, RISCV::VSUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10481 /* vsuxei16.v */, RISCV::VSUXEI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10492 /* vsuxei32.v */, RISCV::VSUXEI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10503 /* vsuxei64.v */, RISCV::VSUXEI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_IsRV64_HasVInstructionsI64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10514 /* vsuxei8.v */, RISCV::VSUXEI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10524 /* vsuxseg2ei16.v */, RISCV::VSUXSEG2EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10539 /* vsuxseg2ei32.v */, RISCV::VSUXSEG2EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10554 /* vsuxseg2ei64.v */, RISCV::VSUXSEG2EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10569 /* vsuxseg2ei8.v */, RISCV::VSUXSEG2EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10583 /* vsuxseg3ei16.v */, RISCV::VSUXSEG3EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10598 /* vsuxseg3ei32.v */, RISCV::VSUXSEG3EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10613 /* vsuxseg3ei64.v */, RISCV::VSUXSEG3EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10628 /* vsuxseg3ei8.v */, RISCV::VSUXSEG3EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10642 /* vsuxseg4ei16.v */, RISCV::VSUXSEG4EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10657 /* vsuxseg4ei32.v */, RISCV::VSUXSEG4EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10672 /* vsuxseg4ei64.v */, RISCV::VSUXSEG4EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10687 /* vsuxseg4ei8.v */, RISCV::VSUXSEG4EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10701 /* vsuxseg5ei16.v */, RISCV::VSUXSEG5EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10716 /* vsuxseg5ei32.v */, RISCV::VSUXSEG5EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10731 /* vsuxseg5ei64.v */, RISCV::VSUXSEG5EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10746 /* vsuxseg5ei8.v */, RISCV::VSUXSEG5EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10760 /* vsuxseg6ei16.v */, RISCV::VSUXSEG6EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10775 /* vsuxseg6ei32.v */, RISCV::VSUXSEG6EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10790 /* vsuxseg6ei64.v */, RISCV::VSUXSEG6EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10805 /* vsuxseg6ei8.v */, RISCV::VSUXSEG6EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10819 /* vsuxseg7ei16.v */, RISCV::VSUXSEG7EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10834 /* vsuxseg7ei32.v */, RISCV::VSUXSEG7EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10849 /* vsuxseg7ei64.v */, RISCV::VSUXSEG7EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10864 /* vsuxseg7ei8.v */, RISCV::VSUXSEG7EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10878 /* vsuxseg8ei16.v */, RISCV::VSUXSEG8EI16_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10893 /* vsuxseg8ei32.v */, RISCV::VSUXSEG8EI32_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10908 /* vsuxseg8ei64.v */, RISCV::VSUXSEG8EI64_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructionsI64_IsRV64, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10923 /* vsuxseg8ei8.v */, RISCV::VSUXSEG8EI8_V, Convert__Reg1_0__Reg1_2__Reg1_4__RVVMaskRegOpOperand1_5, AMFBS_HasVInstructions, { MCK_VM, MCK__40_, MCK_GPR, MCK__41_, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10937 /* vt.maskc */, RISCV::VT_MASKC, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64_HasVendorXVentanaCondOps, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 10946 /* vt.maskcn */, RISCV::VT_MASKCN, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_IsRV64_HasVendorXVentanaCondOps, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 10956 /* vwadd.vv */, RISCV::VWADD_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10965 /* vwadd.vx */, RISCV::VWADD_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10974 /* vwadd.wv */, RISCV::VWADD_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 10983 /* vwadd.wx */, RISCV::VWADD_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 10992 /* vwaddu.vv */, RISCV::VWADDU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11002 /* vwaddu.vx */, RISCV::VWADDU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11012 /* vwaddu.wv */, RISCV::VWADDU_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11022 /* vwaddu.wx */, RISCV::VWADDU_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11032 /* vwcvt.x.x.v */, RISCV::VWADD_VX, Convert__Reg1_0__Reg1_1__regX0__reg0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 11032 /* vwcvt.x.x.v */, RISCV::VWADD_VX, Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11044 /* vwcvtu.x.x.v */, RISCV::VWADDU_VX, Convert__Reg1_0__Reg1_1__regX0__reg0, AMFBS_HasVInstructions, { MCK_VM, MCK_VM }, },
  { 11044 /* vwcvtu.x.x.v */, RISCV::VWADDU_VX, Convert__Reg1_0__Reg1_1__regX0__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11057 /* vwmacc.vv */, RISCV::VWMACC_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11067 /* vwmacc.vx */, RISCV::VWMACC_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11077 /* vwmaccsu.vv */, RISCV::VWMACCSU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11089 /* vwmaccsu.vx */, RISCV::VWMACCSU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11101 /* vwmaccu.vv */, RISCV::VWMACCU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11112 /* vwmaccu.vx */, RISCV::VWMACCU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11123 /* vwmaccus.vx */, RISCV::VWMACCUS_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_GPR, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11135 /* vwmul.vv */, RISCV::VWMUL_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11144 /* vwmul.vx */, RISCV::VWMUL_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11153 /* vwmulsu.vv */, RISCV::VWMULSU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11164 /* vwmulsu.vx */, RISCV::VWMULSU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11175 /* vwmulu.vv */, RISCV::VWMULU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11185 /* vwmulu.vx */, RISCV::VWMULU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11195 /* vwredsum.vs */, RISCV::VWREDSUM_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11207 /* vwredsumu.vs */, RISCV::VWREDSUMU_VS, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11220 /* vwsub.vv */, RISCV::VWSUB_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11229 /* vwsub.vx */, RISCV::VWSUB_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11238 /* vwsub.wv */, RISCV::VWSUB_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11247 /* vwsub.wx */, RISCV::VWSUB_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11256 /* vwsubu.vv */, RISCV::VWSUBU_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11266 /* vwsubu.vx */, RISCV::VWSUBU_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11276 /* vwsubu.wv */, RISCV::VWSUBU_WV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11286 /* vwsubu.wx */, RISCV::VWSUBU_WX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11296 /* vxor.vi */, RISCV::VXOR_VI, Convert__Reg1_0__Reg1_1__SImm51_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_SImm5, MCK_RVVMaskRegOpOperand }, },
  { 11304 /* vxor.vv */, RISCV::VXOR_VV, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11312 /* vxor.vx */, RISCV::VXOR_VX, Convert__Reg1_0__Reg1_1__Reg1_2__RVVMaskRegOpOperand1_3, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_GPR, MCK_RVVMaskRegOpOperand }, },
  { 11320 /* vzext.vf2 */, RISCV::VZEXT_VF2, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11330 /* vzext.vf4 */, RISCV::VZEXT_VF4, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11340 /* vzext.vf8 */, RISCV::VZEXT_VF8, Convert__Reg1_0__Reg1_1__RVVMaskRegOpOperand1_2, AMFBS_HasVInstructions, { MCK_VM, MCK_VM, MCK_RVVMaskRegOpOperand }, },
  { 11350 /* wfi */, RISCV::WFI, Convert__imm_95_0__imm_95_0, AMFBS_None, {  }, },
  { 11354 /* wrs.nto */, RISCV::WRS_NTO, Convert_NoOperands, AMFBS_HasStdExtZawrs, {  }, },
  { 11362 /* wrs.sto */, RISCV::WRS_STO, Convert_NoOperands, AMFBS_HasStdExtZawrs, {  }, },
  { 11370 /* xnor */, RISCV::XNOR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbbOrZbkb, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 11375 /* xor */, RISCV::XOR, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 11375 /* xor */, RISCV::XORI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 11379 /* xori */, RISCV::XORI, Convert__Reg1_0__Reg1_1__SImm121_2, AMFBS_None, { MCK_GPR, MCK_GPR, MCK_SImm12 }, },
  { 11384 /* xperm4 */, RISCV::XPERM4, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbkx, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 11391 /* xperm8 */, RISCV::XPERM8, Convert__Reg1_0__Reg1_1__Reg1_2, AMFBS_HasStdExtZbkx, { MCK_GPR, MCK_GPR, MCK_GPR }, },
  { 11398 /* zext.b */, RISCV::ANDI, Convert__Reg1_0__Reg1_1__imm_95_255, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 11405 /* zext.h */, RISCV::ZEXT_H_RV32, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb_IsRV32, { MCK_GPR, MCK_GPR }, },
  { 11405 /* zext.h */, RISCV::ZEXT_H_RV64, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbb_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 11405 /* zext.h */, RISCV::PseudoZEXT_H, Convert__Reg1_0__Reg1_1, AMFBS_None, { MCK_GPR, MCK_GPR }, },
  { 11412 /* zext.w */, RISCV::ADD_UW, Convert__Reg1_0__Reg1_1__regX0, AMFBS_HasStdExtZba_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 11412 /* zext.w */, RISCV::PseudoZEXT_W, Convert__Reg1_0__Reg1_1, AMFBS_IsRV64, { MCK_GPR, MCK_GPR }, },
  { 11419 /* zip */, RISCV::ZIP_RV32, Convert__Reg1_0__Reg1_1, AMFBS_HasStdExtZbkb_IsRV32, { MCK_GPR, MCK_GPR }, },
};

#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

unsigned RISCVAsmParser::
MatchInstructionImpl(const OperandVector &Operands,
                     MCInst &Inst,
                     uint64_t &ErrorInfo,
                     FeatureBitset &MissingFeatures,
                     bool matchingInlineAsm, unsigned VariantID) {
  // Eliminate obvious mismatches.
  if (Operands.size() > 8) {
    ErrorInfo = 8;
    return Match_InvalidOperand;
  }

  // Get the current feature set.
  const FeatureBitset &AvailableFeatures = getAvailableFeatures();

  // Get the instruction mnemonic, which is the first token.
  StringRef Mnemonic = ((RISCVOperand &)*Operands[0]).getToken();

  // Process all MnemonicAliases to remap the mnemonic.
  applyMnemonicAliases(Mnemonic, AvailableFeatures, VariantID);

  // Some state to try to produce better error messages.
  bool HadMatchOtherThanFeatures = false;
  bool HadMatchOtherThanPredicate = false;
  unsigned RetCode = Match_InvalidOperand;
  MissingFeatures.set();
  // Set ErrorInfo to the operand that mismatches if it is
  // wrong for all instances of the instruction.
  ErrorInfo = ~0ULL;
  SmallBitVector OptionalOperandsMask(7);
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
    OptionalOperandsMask.reset(0, 7);
    for (unsigned FormalIdx = 0, ActualIdx = 1; FormalIdx != 7; ++FormalIdx) {
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
          OptionalOperandsMask.set(FormalIdx, 7);
          break;
        }
        if (isSubclass(Formal, OptionalMatchClass)) {
          OptionalOperandsMask.set(FormalIdx);
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
        OptionalOperandsMask.set(FormalIdx);
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
    convertToMCInst(it->ConvertFn, Inst, it->Opcode, Operands,
                    OptionalOperandsMask);

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

static const OperandMatchEntry OperandMatchTable[959] = {
  /* Operand List Mnemonic, Mask, Operand Class, Features */
  { 0 /* .insn_b */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 8 /* .insn_i */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 8 /* .insn_i */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 16 /* .insn_j */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 16 /* .insn_j */, 4 /* 2 */, MCK_SImm21Lsb0JAL, AMFBS_None },
  { 24 /* .insn_r */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 24 /* .insn_r */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 32 /* .insn_r4 */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 41 /* .insn_s */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 49 /* .insn_sb */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 58 /* .insn_u */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 66 /* .insn_uj */, 1 /* 0 */, MCK_InsnDirectiveOpcode, AMFBS_None },
  { 66 /* .insn_uj */, 4 /* 2 */, MCK_SImm21Lsb0JAL, AMFBS_None },
  { 75 /* add */, 8 /* 3 */, MCK_TPRelAddSymbol, AMFBS_None },
  { 201 /* amoadd.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 210 /* amoadd.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 222 /* amoadd.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 236 /* amoadd.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 248 /* amoadd.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 257 /* amoadd.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 269 /* amoadd.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 283 /* amoadd.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 295 /* amoand.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 304 /* amoand.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 316 /* amoand.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 330 /* amoand.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 342 /* amoand.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 351 /* amoand.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 363 /* amoand.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 377 /* amoand.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 389 /* amomax.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 398 /* amomax.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 410 /* amomax.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 424 /* amomax.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 436 /* amomax.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 445 /* amomax.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 457 /* amomax.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 471 /* amomax.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 483 /* amomaxu.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 493 /* amomaxu.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 506 /* amomaxu.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 521 /* amomaxu.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 534 /* amomaxu.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 544 /* amomaxu.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 557 /* amomaxu.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 572 /* amomaxu.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 585 /* amomin.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 594 /* amomin.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 606 /* amomin.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 620 /* amomin.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 632 /* amomin.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 641 /* amomin.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 653 /* amomin.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 667 /* amomin.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 679 /* amominu.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 689 /* amominu.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 702 /* amominu.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 717 /* amominu.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 730 /* amominu.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 740 /* amominu.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 753 /* amominu.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 768 /* amominu.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 781 /* amoor.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 789 /* amoor.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 800 /* amoor.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 813 /* amoor.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 824 /* amoor.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 832 /* amoor.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 843 /* amoor.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 856 /* amoor.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 867 /* amoswap.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 877 /* amoswap.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 890 /* amoswap.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 905 /* amoswap.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 918 /* amoswap.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 928 /* amoswap.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 941 /* amoswap.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 956 /* amoswap.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 969 /* amoxor.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 978 /* amoxor.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 990 /* amoxor.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 1004 /* amoxor.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 1016 /* amoxor.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 1025 /* amoxor.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 1037 /* amoxor.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 1051 /* amoxor.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 1560 /* call */, 1 /* 0 */, MCK_CallSymbol, AMFBS_None },
  { 1560 /* call */, 2 /* 1 */, MCK_CallSymbol, AMFBS_None },
  { 1565 /* cbo.clean */, 1 /* 0 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtZicbom },
  { 1575 /* cbo.flush */, 1 /* 0 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtZicbom },
  { 1585 /* cbo.inval */, 1 /* 0 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtZicbom },
  { 1595 /* cbo.zero */, 1 /* 0 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtZicboz },
  { 1644 /* csrc */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1644 /* csrc */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1649 /* csrci */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1655 /* csrr */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1660 /* csrrc */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1660 /* csrrc */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1666 /* csrrci */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1673 /* csrrs */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1673 /* csrrs */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1679 /* csrrsi */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1686 /* csrrw */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1686 /* csrrw */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1692 /* csrrwi */, 2 /* 1 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1699 /* csrs */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1699 /* csrs */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1704 /* csrsi */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1710 /* csrw */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1710 /* csrw */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1715 /* csrwi */, 1 /* 0 */, MCK_CSRSystemRegister, AMFBS_None },
  { 1768 /* fabs.d */, 3 /* 0, 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1768 /* fabs.d */, 3 /* 0, 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1775 /* fabs.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1782 /* fabs.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 1789 /* fadd.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1789 /* fadd.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1789 /* fadd.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1789 /* fadd.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1796 /* fadd.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1796 /* fadd.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1803 /* fadd.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 1803 /* fadd.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 1810 /* fclass.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1810 /* fclass.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1819 /* fclass.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1828 /* fclass.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 1837 /* fcvt.d.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1837 /* fcvt.d.h */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1846 /* fcvt.d.l */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1846 /* fcvt.d.l */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1855 /* fcvt.d.lu */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1855 /* fcvt.d.lu */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1865 /* fcvt.d.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1865 /* fcvt.d.s */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1865 /* fcvt.d.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1865 /* fcvt.d.s */, 1 /* 0 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1874 /* fcvt.d.w */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1874 /* fcvt.d.w */, 1 /* 0 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1883 /* fcvt.d.wu */, 1 /* 0 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1883 /* fcvt.d.wu */, 1 /* 0 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 1893 /* fcvt.h.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1893 /* fcvt.h.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1893 /* fcvt.h.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1893 /* fcvt.h.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZhinxOrZhinxmin_HasStdExtZdinx },
  { 1902 /* fcvt.h.l */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1902 /* fcvt.h.l */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1911 /* fcvt.h.lu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1911 /* fcvt.h.lu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1921 /* fcvt.h.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin },
  { 1921 /* fcvt.h.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin },
  { 1930 /* fcvt.h.w */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1930 /* fcvt.h.w */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1939 /* fcvt.h.wu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1939 /* fcvt.h.wu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 1949 /* fcvt.l.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1949 /* fcvt.l.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1958 /* fcvt.l.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1958 /* fcvt.l.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1967 /* fcvt.l.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 1967 /* fcvt.l.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 1976 /* fcvt.lu.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1976 /* fcvt.lu.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 1986 /* fcvt.lu.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1986 /* fcvt.lu.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx_IsRV64 },
  { 1996 /* fcvt.lu.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 1996 /* fcvt.lu.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 2006 /* fcvt.s.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2006 /* fcvt.s.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2006 /* fcvt.s.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2006 /* fcvt.s.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2006 /* fcvt.s.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2006 /* fcvt.s.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2006 /* fcvt.s.d */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2006 /* fcvt.s.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2015 /* fcvt.s.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinxOrZhinxmin },
  { 2024 /* fcvt.s.l */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 2024 /* fcvt.s.l */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 2033 /* fcvt.s.lu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 2033 /* fcvt.s.lu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx_IsRV64 },
  { 2043 /* fcvt.s.w */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2043 /* fcvt.s.w */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2052 /* fcvt.s.wu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2052 /* fcvt.s.wu */, 1 /* 0 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2062 /* fcvt.w.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2062 /* fcvt.w.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2062 /* fcvt.w.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2062 /* fcvt.w.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2071 /* fcvt.w.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2071 /* fcvt.w.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2080 /* fcvt.w.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2080 /* fcvt.w.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2089 /* fcvt.wu.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2089 /* fcvt.wu.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2089 /* fcvt.wu.d */, 2 /* 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2089 /* fcvt.wu.d */, 2 /* 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2099 /* fcvt.wu.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2099 /* fcvt.wu.h */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2109 /* fcvt.wu.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2109 /* fcvt.wu.s */, 2 /* 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2119 /* fdiv.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2119 /* fdiv.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2119 /* fdiv.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2119 /* fdiv.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2126 /* fdiv.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2126 /* fdiv.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2133 /* fdiv.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2133 /* fdiv.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2164 /* feq.d */, 6 /* 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2164 /* feq.d */, 6 /* 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2170 /* feq.h */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2176 /* feq.s */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2182 /* fge.d */, 6 /* 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2182 /* fge.d */, 6 /* 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2188 /* fge.h */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2194 /* fge.s */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2200 /* fgt.d */, 6 /* 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2200 /* fgt.d */, 6 /* 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2206 /* fgt.h */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2212 /* fgt.s */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2218 /* fld */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtD },
  { 2222 /* fle.d */, 6 /* 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2222 /* fle.d */, 6 /* 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2228 /* fle.h */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2234 /* fle.s */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2240 /* flh */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtZfhOrZfhmin },
  { 2244 /* flt.d */, 6 /* 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2244 /* flt.d */, 6 /* 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2250 /* flt.h */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2256 /* flt.s */, 6 /* 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2262 /* flw */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtF },
  { 2266 /* fmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2266 /* fmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2266 /* fmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2266 /* fmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2274 /* fmadd.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2274 /* fmadd.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2282 /* fmadd.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2282 /* fmadd.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2290 /* fmax.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2290 /* fmax.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2297 /* fmax.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2304 /* fmax.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2311 /* fmin.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2311 /* fmin.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2318 /* fmin.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2325 /* fmin.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2332 /* fmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2332 /* fmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2332 /* fmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2332 /* fmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2340 /* fmsub.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2340 /* fmsub.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2348 /* fmsub.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2348 /* fmsub.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2356 /* fmul.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2356 /* fmul.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2356 /* fmul.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2356 /* fmul.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2363 /* fmul.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2363 /* fmul.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2370 /* fmul.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2370 /* fmul.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2391 /* fmv.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2443 /* fneg.d */, 3 /* 0, 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2443 /* fneg.d */, 3 /* 0, 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2450 /* fneg.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2457 /* fneg.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2464 /* fnmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2464 /* fnmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2464 /* fnmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2464 /* fnmadd.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2473 /* fnmadd.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2473 /* fnmadd.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2482 /* fnmadd.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2482 /* fnmadd.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2491 /* fnmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2491 /* fnmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2491 /* fnmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2491 /* fnmsub.d */, 15 /* 0, 1, 2, 3 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2500 /* fnmsub.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2500 /* fnmsub.h */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2509 /* fnmsub.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2509 /* fnmsub.s */, 15 /* 0, 1, 2, 3 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2548 /* fsd */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtD },
  { 2569 /* fsgnj.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2569 /* fsgnj.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2577 /* fsgnj.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2585 /* fsgnj.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2593 /* fsgnjn.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2593 /* fsgnjn.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2602 /* fsgnjn.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2611 /* fsgnjn.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2620 /* fsgnjx.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2620 /* fsgnjx.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2629 /* fsgnjx.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2638 /* fsgnjx.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2647 /* fsh */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtZfhOrZfhmin },
  { 2651 /* fsqrt.d */, 3 /* 0, 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2651 /* fsqrt.d */, 3 /* 0, 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2651 /* fsqrt.d */, 3 /* 0, 1 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2651 /* fsqrt.d */, 3 /* 0, 1 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2659 /* fsqrt.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2659 /* fsqrt.h */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2667 /* fsqrt.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2667 /* fsqrt.s */, 3 /* 0, 1 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2691 /* fsub.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2691 /* fsub.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2691 /* fsub.d */, 7 /* 0, 1, 2 */, MCK_GPRF64AsFPR, AMFBS_HasStdExtZdinx_IsRV64 },
  { 2691 /* fsub.d */, 7 /* 0, 1, 2 */, MCK_GPRPF64AsFPR, AMFBS_HasStdExtZdinx_IsRV32 },
  { 2698 /* fsub.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2698 /* fsub.h */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZhinx },
  { 2705 /* fsub.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2705 /* fsub.s */, 7 /* 0, 1, 2 */, MCK_GPRAsFPR, AMFBS_HasStdExtZfinx },
  { 2712 /* fsw */, 2 /* 1 */, MCK_BareSymbol, AMFBS_HasStdExtF },
  { 2764 /* hlv.b */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2770 /* hlv.bu */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2777 /* hlv.d */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_IsRV64_HasStdExtH },
  { 2783 /* hlv.h */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2789 /* hlv.hu */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2796 /* hlv.w */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2802 /* hlv.wu */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_IsRV64_HasStdExtH },
  { 2809 /* hlvx.hu */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2817 /* hlvx.wu */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2825 /* hsv.b */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2831 /* hsv.d */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_IsRV64_HasStdExtH },
  { 2837 /* hsv.h */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2843 /* hsv.w */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtH },
  { 2849 /* j */, 1 /* 0 */, MCK_SImm21Lsb0JAL, AMFBS_None },
  { 2851 /* jal */, 1 /* 0 */, MCK_SImm21Lsb0JAL, AMFBS_None },
  { 2851 /* jal */, 2 /* 1 */, MCK_SImm21Lsb0JAL, AMFBS_None },
  { 2863 /* jump */, 1 /* 0 */, MCK_PseudoJumpSymbol, AMFBS_None },
  { 2868 /* la */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2871 /* la.tls.gd */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2881 /* la.tls.ie */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2891 /* lb */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2894 /* lbu */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2898 /* ld */, 2 /* 1 */, MCK_BareSymbol, AMFBS_IsRV64 },
  { 2901 /* lh */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2904 /* lhu */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2911 /* lla */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2915 /* lr.d */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 2920 /* lr.d.aq */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 2928 /* lr.d.aqrl */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 2938 /* lr.d.rl */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 2946 /* lr.w */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 2951 /* lr.w.aq */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 2959 /* lr.w.aqrl */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 2969 /* lr.w.rl */, 2 /* 1 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 2981 /* lw */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 2984 /* lwu */, 2 /* 1 */, MCK_BareSymbol, AMFBS_IsRV64 },
  { 3273 /* sb */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 3276 /* sc.d */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 3281 /* sc.d.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 3289 /* sc.d.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 3299 /* sc.d.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA_IsRV64 },
  { 3307 /* sc.w */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 3312 /* sc.w.aq */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 3320 /* sc.w.aqrl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 3330 /* sc.w.rl */, 4 /* 2 */, MCK_ZeroOffsetMemOpOperand, AMFBS_HasStdExtA },
  { 3338 /* sd */, 2 /* 1 */, MCK_BareSymbol, AMFBS_IsRV64 },
  { 3423 /* sh */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 3784 /* sw */, 2 /* 1 */, MCK_BareSymbol, AMFBS_None },
  { 3787 /* tail */, 1 /* 0 */, MCK_CallSymbol, AMFBS_None },
  { 3792 /* th.vmaqa.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3804 /* th.vmaqa.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3816 /* th.vmaqasu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3830 /* th.vmaqasu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3844 /* th.vmaqau.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3857 /* th.vmaqau.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3870 /* th.vmaqaus.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVendorXTHeadVdot },
  { 3901 /* vaadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3910 /* vaadd.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3919 /* vaaddu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3929 /* vaaddu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3966 /* vadd.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3974 /* vadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3982 /* vadd.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3990 /* vand.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 3998 /* vand.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4006 /* vand.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4014 /* vasub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4023 /* vasub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4032 /* vasubu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4042 /* vasubu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4065 /* vcpop.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4073 /* vdiv.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4081 /* vdiv.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4089 /* vdivu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4098 /* vdivu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4107 /* vfabs.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4115 /* vfadd.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4124 /* vfadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4133 /* vfclass.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4143 /* vfcvt.f.x.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4155 /* vfcvt.f.xu.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4168 /* vfcvt.rtz.x.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4184 /* vfcvt.rtz.xu.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4201 /* vfcvt.x.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4213 /* vfcvt.xu.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4226 /* vfdiv.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4235 /* vfdiv.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4244 /* vfirst.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 4253 /* vfmacc.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4263 /* vfmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4273 /* vfmadd.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4283 /* vfmadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4293 /* vfmax.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4302 /* vfmax.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4323 /* vfmin.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4332 /* vfmin.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4341 /* vfmsac.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4351 /* vfmsac.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4361 /* vfmsub.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4371 /* vfmsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4381 /* vfmul.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4390 /* vfmul.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4426 /* vfncvt.f.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4439 /* vfncvt.f.x.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4452 /* vfncvt.f.xu.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4466 /* vfncvt.rod.f.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4483 /* vfncvt.rtz.x.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4500 /* vfncvt.rtz.xu.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4518 /* vfncvt.x.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4531 /* vfncvt.xu.f.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4545 /* vfneg.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4553 /* vfnmacc.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4564 /* vfnmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4575 /* vfnmadd.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4586 /* vfnmadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4597 /* vfnmsac.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4608 /* vfnmsac.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4619 /* vfnmsub.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4630 /* vfnmsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4641 /* vfrdiv.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4651 /* vfrec7.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4660 /* vfredmax.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4672 /* vfredmin.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4684 /* vfredosum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4697 /* vfredsum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4709 /* vfredusum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4722 /* vfrsqrt7.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4733 /* vfrsub.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4743 /* vfsgnj.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4753 /* vfsgnj.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4763 /* vfsgnjn.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4774 /* vfsgnjn.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4785 /* vfsgnjx.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4796 /* vfsgnjx.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4807 /* vfslide1down.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4823 /* vfslide1up.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4837 /* vfsqrt.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4846 /* vfsub.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4855 /* vfsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4864 /* vfwadd.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4874 /* vfwadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4884 /* vfwadd.wf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4894 /* vfwadd.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4904 /* vfwcvt.f.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4917 /* vfwcvt.f.x.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4930 /* vfwcvt.f.xu.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4944 /* vfwcvt.rtz.x.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4961 /* vfwcvt.rtz.xu.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4979 /* vfwcvt.x.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 4992 /* vfwcvt.xu.f.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5006 /* vfwmacc.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5017 /* vfwmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5028 /* vfwmsac.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5039 /* vfwmsac.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5050 /* vfwmul.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5060 /* vfwmul.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5070 /* vfwnmacc.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5082 /* vfwnmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5094 /* vfwnmsac.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5106 /* vfwnmsac.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5118 /* vfwredosum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5132 /* vfwredsum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5145 /* vfwredusum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5159 /* vfwsub.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5169 /* vfwsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5179 /* vfwsub.wf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5189 /* vfwsub.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 5199 /* vid.v */, 2 /* 1 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5205 /* viota.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5404 /* vle16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5412 /* vle16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5422 /* vle32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5430 /* vle32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5440 /* vle64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 5448 /* vle64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 5458 /* vle8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5465 /* vle8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5480 /* vloxei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5491 /* vloxei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5502 /* vloxei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_IsRV64_HasVInstructionsI64 },
  { 5513 /* vloxei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5523 /* vloxseg2ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5538 /* vloxseg2ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5553 /* vloxseg2ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5568 /* vloxseg2ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5582 /* vloxseg3ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5597 /* vloxseg3ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5612 /* vloxseg3ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5627 /* vloxseg3ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5641 /* vloxseg4ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5656 /* vloxseg4ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5671 /* vloxseg4ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5686 /* vloxseg4ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5700 /* vloxseg5ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5715 /* vloxseg5ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5730 /* vloxseg5ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5745 /* vloxseg5ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5759 /* vloxseg6ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5774 /* vloxseg6ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5789 /* vloxseg6ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5804 /* vloxseg6ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5818 /* vloxseg7ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5833 /* vloxseg7ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5848 /* vloxseg7ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5863 /* vloxseg7ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5877 /* vloxseg8ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5892 /* vloxseg8ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5907 /* vloxseg8ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 5922 /* vloxseg8ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5936 /* vlse16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5945 /* vlse32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5954 /* vlse64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 5963 /* vlse8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5971 /* vlseg2e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5983 /* vlseg2e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 5997 /* vlseg2e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6009 /* vlseg2e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6023 /* vlseg2e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6035 /* vlseg2e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6049 /* vlseg2e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6060 /* vlseg2e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6073 /* vlseg3e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6085 /* vlseg3e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6099 /* vlseg3e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6111 /* vlseg3e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6125 /* vlseg3e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6137 /* vlseg3e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6151 /* vlseg3e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6162 /* vlseg3e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6175 /* vlseg4e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6187 /* vlseg4e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6201 /* vlseg4e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6213 /* vlseg4e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6227 /* vlseg4e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6239 /* vlseg4e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6253 /* vlseg4e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6264 /* vlseg4e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6277 /* vlseg5e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6289 /* vlseg5e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6303 /* vlseg5e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6315 /* vlseg5e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6329 /* vlseg5e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6341 /* vlseg5e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6355 /* vlseg5e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6366 /* vlseg5e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6379 /* vlseg6e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6391 /* vlseg6e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6405 /* vlseg6e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6417 /* vlseg6e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6431 /* vlseg6e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6443 /* vlseg6e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6457 /* vlseg6e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6468 /* vlseg6e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6481 /* vlseg7e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6493 /* vlseg7e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6507 /* vlseg7e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6519 /* vlseg7e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6533 /* vlseg7e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6545 /* vlseg7e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6559 /* vlseg7e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6570 /* vlseg7e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6583 /* vlseg8e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6595 /* vlseg8e16ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6609 /* vlseg8e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6621 /* vlseg8e32ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6635 /* vlseg8e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6647 /* vlseg8e64ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6661 /* vlseg8e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6672 /* vlseg8e8ff.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6685 /* vlsseg2e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6698 /* vlsseg2e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6711 /* vlsseg2e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6724 /* vlsseg2e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6736 /* vlsseg3e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6749 /* vlsseg3e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6762 /* vlsseg3e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6775 /* vlsseg3e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6787 /* vlsseg4e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6800 /* vlsseg4e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6813 /* vlsseg4e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6826 /* vlsseg4e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6838 /* vlsseg5e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6851 /* vlsseg5e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6864 /* vlsseg5e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6877 /* vlsseg5e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6889 /* vlsseg6e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6902 /* vlsseg6e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6915 /* vlsseg6e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6928 /* vlsseg6e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6940 /* vlsseg7e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6953 /* vlsseg7e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6966 /* vlsseg7e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 6979 /* vlsseg7e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 6991 /* vlsseg8e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7004 /* vlsseg8e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7017 /* vlsseg8e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 7030 /* vlsseg8e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7042 /* vluxei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7053 /* vluxei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7064 /* vluxei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_IsRV64_HasVInstructionsI64 },
  { 7075 /* vluxei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7085 /* vluxseg2ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7100 /* vluxseg2ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7115 /* vluxseg2ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7130 /* vluxseg2ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7144 /* vluxseg3ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7159 /* vluxseg3ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7174 /* vluxseg3ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7189 /* vluxseg3ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7203 /* vluxseg4ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7218 /* vluxseg4ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7233 /* vluxseg4ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7248 /* vluxseg4ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7262 /* vluxseg5ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7277 /* vluxseg5ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7292 /* vluxseg5ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7307 /* vluxseg5ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7321 /* vluxseg6ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7336 /* vluxseg6ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7351 /* vluxseg6ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7366 /* vluxseg6ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7380 /* vluxseg7ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7395 /* vluxseg7ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7410 /* vluxseg7ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7425 /* vluxseg7ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7439 /* vluxseg8ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7454 /* vluxseg8ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7469 /* vluxseg8ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 7484 /* vluxseg8ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7498 /* vmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7507 /* vmacc.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7573 /* vmadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7582 /* vmadd.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7622 /* vmax.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7630 /* vmax.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7638 /* vmaxu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7647 /* vmaxu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7697 /* vmfeq.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7706 /* vmfeq.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7715 /* vmfge.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7724 /* vmfge.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7733 /* vmfgt.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7742 /* vmfgt.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7751 /* vmfle.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7760 /* vmfle.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7769 /* vmflt.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7778 /* vmflt.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7787 /* vmfne.vf */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7796 /* vmfne.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsAnyF },
  { 7805 /* vmin.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7813 /* vmin.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7821 /* vminu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7830 /* vminu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7939 /* vmsbf.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7947 /* vmseq.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7956 /* vmseq.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7965 /* vmseq.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7982 /* vmsge.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 7991 /* vmsge.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8000 /* vmsge.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8000 /* vmsge.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8009 /* vmsgeu.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8019 /* vmsgeu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8029 /* vmsgeu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8029 /* vmsgeu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8039 /* vmsgt.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8048 /* vmsgt.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8057 /* vmsgt.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8066 /* vmsgtu.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8076 /* vmsgtu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8086 /* vmsgtu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8096 /* vmsif.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8104 /* vmsle.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8113 /* vmsle.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8122 /* vmsle.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8131 /* vmsleu.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8141 /* vmsleu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8151 /* vmsleu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8161 /* vmslt.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8170 /* vmslt.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8179 /* vmslt.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8188 /* vmsltu.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8198 /* vmsltu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8208 /* vmsltu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8218 /* vmsne.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8227 /* vmsne.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8236 /* vmsne.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8245 /* vmsof.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8253 /* vmul.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8261 /* vmul.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8269 /* vmulh.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8278 /* vmulh.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8287 /* vmulhsu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8298 /* vmulhsu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8309 /* vmulhu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8319 /* vmulhu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8420 /* vnclip.wi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8430 /* vnclip.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8440 /* vnclip.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8450 /* vnclipu.wi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8461 /* vnclipu.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8472 /* vnclipu.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8483 /* vncvt.x.x.w */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8495 /* vneg.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8502 /* vnmsac.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8512 /* vnmsac.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8522 /* vnmsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8532 /* vnmsub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8542 /* vnot.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8549 /* vnsra.wi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8558 /* vnsra.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8567 /* vnsra.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8576 /* vnsrl.wi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8585 /* vnsrl.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8594 /* vnsrl.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8603 /* vor.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8610 /* vor.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8617 /* vor.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8624 /* vpopc.m */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8632 /* vredand.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8643 /* vredmax.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8654 /* vredmaxu.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8666 /* vredmin.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8677 /* vredminu.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8689 /* vredor.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8699 /* vredsum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8710 /* vredxor.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8721 /* vrem.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8729 /* vrem.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8737 /* vremu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8746 /* vremu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8755 /* vrgather.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8767 /* vrgather.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8779 /* vrgather.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8791 /* vrgatherei16.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8807 /* vrsub.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8816 /* vrsub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8853 /* vsadd.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8862 /* vsadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8871 /* vsadd.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8880 /* vsaddu.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8890 /* vsaddu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8900 /* vsaddu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8935 /* vse16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8943 /* vse32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8951 /* vse64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 8959 /* vse8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 8966 /* vsetivli */, 4 /* 2 */, MCK_VTypeI10, AMFBS_HasVInstructions },
  { 8982 /* vsetvli */, 4 /* 2 */, MCK_VTypeI11, AMFBS_HasVInstructions },
  { 8990 /* vsext.vf2 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9000 /* vsext.vf4 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9010 /* vsext.vf8 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9020 /* vslide1down.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9035 /* vslide1up.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9048 /* vslidedown.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9062 /* vslidedown.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9076 /* vslideup.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9088 /* vslideup.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9100 /* vsll.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9108 /* vsll.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9116 /* vsll.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9130 /* vsmul.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9139 /* vsmul.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9148 /* vsoxei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9159 /* vsoxei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9170 /* vsoxei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_IsRV64_HasVInstructionsI64 },
  { 9181 /* vsoxei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9191 /* vsoxseg2ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9206 /* vsoxseg2ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9221 /* vsoxseg2ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9236 /* vsoxseg2ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9250 /* vsoxseg3ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9265 /* vsoxseg3ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9280 /* vsoxseg3ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9295 /* vsoxseg3ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9309 /* vsoxseg4ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9324 /* vsoxseg4ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9339 /* vsoxseg4ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9354 /* vsoxseg4ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9368 /* vsoxseg5ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9383 /* vsoxseg5ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9398 /* vsoxseg5ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9413 /* vsoxseg5ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9427 /* vsoxseg6ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9442 /* vsoxseg6ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9457 /* vsoxseg6ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9472 /* vsoxseg6ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9486 /* vsoxseg7ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9501 /* vsoxseg7ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9516 /* vsoxseg7ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9531 /* vsoxseg7ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9545 /* vsoxseg8ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9560 /* vsoxseg8ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9575 /* vsoxseg8ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 9590 /* vsoxseg8ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9604 /* vsra.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9612 /* vsra.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9620 /* vsra.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9628 /* vsrl.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9636 /* vsrl.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9644 /* vsrl.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9652 /* vsse16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9661 /* vsse32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9670 /* vsse64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9679 /* vsse8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9687 /* vsseg2e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9699 /* vsseg2e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9711 /* vsseg2e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9723 /* vsseg2e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9734 /* vsseg3e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9746 /* vsseg3e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9758 /* vsseg3e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9770 /* vsseg3e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9781 /* vsseg4e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9793 /* vsseg4e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9805 /* vsseg4e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9817 /* vsseg4e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9828 /* vsseg5e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9840 /* vsseg5e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9852 /* vsseg5e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9864 /* vsseg5e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9875 /* vsseg6e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9887 /* vsseg6e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9899 /* vsseg6e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9911 /* vsseg6e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9922 /* vsseg7e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9934 /* vsseg7e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9946 /* vsseg7e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 9958 /* vsseg7e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9969 /* vsseg8e16.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9981 /* vsseg8e32.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 9993 /* vsseg8e64.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10005 /* vsseg8e8.v */, 16 /* 4 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10016 /* vssra.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10025 /* vssra.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10034 /* vssra.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10043 /* vssrl.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10052 /* vssrl.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10061 /* vssrl.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10070 /* vssseg2e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10083 /* vssseg2e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10096 /* vssseg2e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10109 /* vssseg2e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10121 /* vssseg3e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10134 /* vssseg3e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10147 /* vssseg3e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10160 /* vssseg3e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10172 /* vssseg4e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10185 /* vssseg4e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10198 /* vssseg4e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10211 /* vssseg4e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10223 /* vssseg5e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10236 /* vssseg5e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10249 /* vssseg5e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10262 /* vssseg5e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10274 /* vssseg6e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10287 /* vssseg6e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10300 /* vssseg6e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10313 /* vssseg6e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10325 /* vssseg7e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10338 /* vssseg7e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10351 /* vssseg7e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10364 /* vssseg7e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10376 /* vssseg8e16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10389 /* vssseg8e32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10402 /* vssseg8e64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64 },
  { 10415 /* vssseg8e8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10427 /* vssub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10436 /* vssub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10445 /* vssubu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10455 /* vssubu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10465 /* vsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10473 /* vsub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10481 /* vsuxei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10492 /* vsuxei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10503 /* vsuxei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_IsRV64_HasVInstructionsI64 },
  { 10514 /* vsuxei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10524 /* vsuxseg2ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10539 /* vsuxseg2ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10554 /* vsuxseg2ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10569 /* vsuxseg2ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10583 /* vsuxseg3ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10598 /* vsuxseg3ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10613 /* vsuxseg3ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10628 /* vsuxseg3ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10642 /* vsuxseg4ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10657 /* vsuxseg4ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10672 /* vsuxseg4ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10687 /* vsuxseg4ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10701 /* vsuxseg5ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10716 /* vsuxseg5ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10731 /* vsuxseg5ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10746 /* vsuxseg5ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10760 /* vsuxseg6ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10775 /* vsuxseg6ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10790 /* vsuxseg6ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10805 /* vsuxseg6ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10819 /* vsuxseg7ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10834 /* vsuxseg7ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10849 /* vsuxseg7ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10864 /* vsuxseg7ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10878 /* vsuxseg8ei16.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10893 /* vsuxseg8ei32.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10908 /* vsuxseg8ei64.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructionsI64_IsRV64 },
  { 10923 /* vsuxseg8ei8.v */, 32 /* 5 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10956 /* vwadd.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10965 /* vwadd.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10974 /* vwadd.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10983 /* vwadd.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 10992 /* vwaddu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11002 /* vwaddu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11012 /* vwaddu.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11022 /* vwaddu.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11032 /* vwcvt.x.x.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11044 /* vwcvtu.x.x.v */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11057 /* vwmacc.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11067 /* vwmacc.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11077 /* vwmaccsu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11089 /* vwmaccsu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11101 /* vwmaccu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11112 /* vwmaccu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11123 /* vwmaccus.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11135 /* vwmul.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11144 /* vwmul.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11153 /* vwmulsu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11164 /* vwmulsu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11175 /* vwmulu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11185 /* vwmulu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11195 /* vwredsum.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11207 /* vwredsumu.vs */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11220 /* vwsub.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11229 /* vwsub.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11238 /* vwsub.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11247 /* vwsub.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11256 /* vwsubu.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11266 /* vwsubu.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11276 /* vwsubu.wv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11286 /* vwsubu.wx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11296 /* vxor.vi */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11304 /* vxor.vv */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11312 /* vxor.vx */, 8 /* 3 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11320 /* vzext.vf2 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11330 /* vzext.vf4 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
  { 11340 /* vzext.vf8 */, 4 /* 2 */, MCK_RVVMaskRegOpOperand, AMFBS_HasVInstructions },
};

OperandMatchResultTy RISCVAsmParser::
tryCustomParseOperand(OperandVector &Operands,
                      unsigned MCK) {

  switch(MCK) {
  case MCK_BareSymbol:
    return parseBareSymbol(Operands);
  case MCK_CSRSystemRegister:
    return parseCSRSystemRegister(Operands);
  case MCK_CallSymbol:
    return parseCallSymbol(Operands);
  case MCK_GPRAsFPR:
    return parseGPRAsFPR(Operands);
  case MCK_GPRF64AsFPR:
    return parseGPRAsFPR(Operands);
  case MCK_GPRPF64AsFPR:
    return parseGPRAsFPR(Operands);
  case MCK_InsnDirectiveOpcode:
    return parseInsnDirectiveOpcode(Operands);
  case MCK_PseudoJumpSymbol:
    return parsePseudoJumpSymbol(Operands);
  case MCK_SImm21Lsb0JAL:
    return parseJALOffset(Operands);
  case MCK_TPRelAddSymbol:
    return parseOperandWithModifier(Operands);
  case MCK_RVVMaskRegOpOperand:
    return parseMaskReg(Operands);
  case MCK_ZeroOffsetMemOpOperand:
    return parseZeroOffsetMemOp(Operands);
  case MCK_VTypeI10:
    return parseVTypeI(Operands);
  case MCK_VTypeI11:
    return parseVTypeI(Operands);
  default:
    return MatchOperand_NoMatch;
  }
  return MatchOperand_NoMatch;
}

OperandMatchResultTy RISCVAsmParser::
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

static std::string RISCVMnemonicSpellCheck(StringRef S, const FeatureBitset &FBS, unsigned VariantID) {
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

static bool RISCVCheckMnemonic(StringRef Mnemonic,
                                const FeatureBitset &AvailableFeatures,
                                unsigned VariantID) {
  // Process all MnemonicAliases to remap the mnemonic.
  applyMnemonicAliases(Mnemonic, AvailableFeatures, VariantID);

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

