/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Calling Convention Implementation Fragment                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef GET_CC_REGISTER_LISTS

bool llvm::CC_ARM_AAPCS(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_ARM_AAPCS_Common(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_ARM_APCS(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_ARM_APCS_GHC(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_ARM_Win32_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::FastCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_ARM_AAPCS(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_ARM_AAPCS_Common(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetFastCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);


bool llvm::CC_ARM_AAPCS(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(ARM::R12)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64 ||
      LocVT == MVT::v2f64) {
    if (CC_ARM_AAPCS_Custom_f64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::f32) {
    LocVT = MVT::i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::bf16) {
    if (CC_ARM_AAPCS_Custom_f16(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_ARM_AAPCS_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_ARM_AAPCS_Common(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.getNonZeroOrigAlign() == 8) {
      static const MCPhysReg RegList1[] = {
        ARM::R0, ARM::R2
      };
      static const MCPhysReg RegList2[] = {
        ARM::R0, ARM::R1
      };
      if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.getNonZeroOrigAlign() != Align(8)) {
      static const MCPhysReg RegList3[] = {
        ARM::R0, ARM::R1, ARM::R2, ARM::R3
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.getNonZeroOrigAlign() == 8) {
      static const MCPhysReg ShadowRegList4[] = {
        ARM::R0, ARM::R1, ARM::R2, ARM::R3
      };
      unsigned Offset5 = State.AllocateStack(4, Align(8), ShadowRegList4);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg ShadowRegList6[] = {
      ARM::R0, ARM::R1, ARM::R2, ARM::R3
    };
    unsigned Offset7 = State.AllocateStack(4, Align(4), ShadowRegList6);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::bf16 ||
      LocVT == MVT::f32) {
    static const MCPhysReg ShadowRegList8[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset9 = State.AllocateStack(4, Align(4), ShadowRegList8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg ShadowRegList10[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset11 = State.AllocateStack(8, Align(8), ShadowRegList10);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset11, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2f64) {
    if (ArgFlags.getNonZeroOrigAlign() == 16) {
      static const MCPhysReg ShadowRegList12[] = {
        ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
      };
      unsigned Offset13 = State.AllocateStack(16, Align(16), ShadowRegList12);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset13, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg ShadowRegList14[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset15 = State.AllocateStack(16, Align(8), ShadowRegList14);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset15, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


bool llvm::CC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isInConsecutiveRegs()) {
    if (CC_ARM_AAPCS_Custom_Aggregate(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      ARM::S0, ARM::S1, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::bf16) {
    if (CC_ARM_AAPCS_VFP_Custom_f16(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_ARM_AAPCS_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::CC_ARM_APCS(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f64 ||
      LocVT == MVT::v2f64) {
    if (CC_ARM_APCS_Custom_f64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::f32) {
    LocVT = MVT::i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      ARM::R0, ARM::R1, ARM::R2, ARM::R3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    unsigned Offset2 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    unsigned Offset3 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2f64) {
    unsigned Offset4 = State.AllocateStack(16, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


bool llvm::CC_ARM_APCS_GHC(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      ARM::Q4, ARM::Q5
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      ARM::D8, ARM::D9, ARM::D10, ARM::D11
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      ARM::S16, ARM::S17, ARM::S18, ARM::S19, ARM::S20, ARM::S21, ARM::S22, ARM::S23
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList4[] = {
      ARM::R4, ARM::R5, ARM::R6, ARM::R7, ARM::R8, ARM::R9, ARM::R10, ARM::R11
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::CC_ARM_Win32_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32) {
    if (unsigned Reg = State.AllocateReg(ARM::R0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::FastCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      ARM::S0, ARM::S1, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg ShadowRegList4[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset5 = State.AllocateStack(4, Align(4), ShadowRegList4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg ShadowRegList6[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset7 = State.AllocateStack(8, Align(4), ShadowRegList6);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg ShadowRegList8[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    unsigned Offset9 = State.AllocateStack(16, Align(4), ShadowRegList8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
    return false;
  }

  if (!CC_ARM_APCS(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::RetCC_ARM_AAPCS(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64 ||
      LocVT == MVT::v2f64) {
    if (RetCC_ARM_AAPCS_Custom_f64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::f32) {
    LocVT = MVT::i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::bf16) {
    if (CC_ARM_AAPCS_Custom_f16(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_ARM_AAPCS_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_ARM_AAPCS_Common(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      ARM::R0, ARM::R1, ARM::R2, ARM::R3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      ARM::R0, ARM::R2
    };
    static const MCPhysReg RegList3[] = {
      ARM::R1, ARM::R3
    };
    if (unsigned Reg = State.AllocateReg(RegList2, RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::RetCC_ARM_AAPCS_VFP(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      ARM::S0, ARM::S1, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::bf16) {
    if (CC_ARM_AAPCS_VFP_Custom_f16(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_ARM_AAPCS_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::RetCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::f32) {
    LocVT = MVT::i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i32) {
      if (unsigned Reg = State.AllocateReg(ARM::R8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f64 ||
      LocVT == MVT::v2f64) {
    if (RetCC_ARM_APCS_Custom_f64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      ARM::R0, ARM::R1, ARM::R2, ARM::R3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      ARM::R0, ARM::R2
    };
    static const MCPhysReg RegList3[] = {
      ARM::R1, ARM::R3
    };
    if (unsigned Reg = State.AllocateReg(RegList2, RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::RetFastCC_ARM_APCS(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v4f16 ||
      LocVT == MVT::v4bf16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v8bf16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      ARM::S0, ARM::S1, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_ARM_APCS(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}

#else

const MCRegister CC_ARM_AAPCS_ArgRegs[] = { ARM::R12 };
const MCRegister CC_ARM_AAPCS_Common_ArgRegs[] = { ARM::R0, ARM::R1, ARM::R2, ARM::R3 };
const MCRegister CC_ARM_AAPCS_VFP_ArgRegs[] = { ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7, ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3, ARM::S0, ARM::S1, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9 };
const MCRegister CC_ARM_APCS_ArgRegs[] = { ARM::R0, ARM::R1, ARM::R2, ARM::R3 };
const MCRegister CC_ARM_APCS_GHC_ArgRegs[] = { ARM::D10, ARM::D11, ARM::D8, ARM::D9, ARM::Q4, ARM::Q5, ARM::R10, ARM::R11, ARM::R4, ARM::R5, ARM::R6, ARM::R7, ARM::R8, ARM::R9, ARM::S16, ARM::S17, ARM::S18, ARM::S19, ARM::S20, ARM::S21, ARM::S22, ARM::S23 };
const MCRegister CC_ARM_Win32_CFGuard_Check_ArgRegs[] = { ARM::R0 };
const MCRegister FastCC_ARM_APCS_ArgRegs[] = { ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7, ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3, ARM::S0, ARM::S1, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9 };
const MCRegister RetCC_ARM_AAPCS_ArgRegs[] = { 0 };
const MCRegister RetCC_ARM_AAPCS_Common_ArgRegs[] = { ARM::R0, ARM::R1, ARM::R2, ARM::R3 };
const MCRegister RetCC_ARM_AAPCS_VFP_ArgRegs[] = { ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7, ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3, ARM::S0, ARM::S1, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9 };
const MCRegister RetCC_ARM_APCS_ArgRegs[] = { ARM::R0, ARM::R1, ARM::R2, ARM::R3 };
const MCRegister RetFastCC_ARM_APCS_ArgRegs[] = { ARM::D0, ARM::D1, ARM::D2, ARM::D3, ARM::D4, ARM::D5, ARM::D6, ARM::D7, ARM::Q0, ARM::Q1, ARM::Q2, ARM::Q3, ARM::S0, ARM::S1, ARM::S10, ARM::S11, ARM::S12, ARM::S13, ARM::S14, ARM::S15, ARM::S2, ARM::S3, ARM::S4, ARM::S5, ARM::S6, ARM::S7, ARM::S8, ARM::S9 };

// Registers used by Swift.
const MCRegister CC_ARM_AAPCS_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };
const MCRegister CC_ARM_AAPCS_VFP_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };
const MCRegister CC_ARM_APCS_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };
const MCRegister RetCC_ARM_AAPCS_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };
const MCRegister RetCC_ARM_AAPCS_VFP_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };
const MCRegister RetCC_ARM_APCS_Swift_ArgRegs[] = { ARM::R10, ARM::R8 };

#endif // CC_REGISTER_LIST
