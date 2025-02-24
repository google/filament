/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Calling Convention Implementation Fragment                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

static bool CC_Mips(unsigned ValNo, MVT ValVT,
                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_Mips16RetHelper(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsN(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsN_FastCC(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsN_SoftFloat(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsN_VarArg(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsO32(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsO32_FP(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_MipsO32_FastCC(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_Mips_ByVal(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_Mips_FastCC(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_Mips_FixedArg(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_Mips_VarArg(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_F128(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_F128HardFloat(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_F128SoftFloat(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_Mips(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_MipsN(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_MipsO32(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);


static bool CC_Mips(unsigned ValNo, MVT ValVT,
                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.isVarArg()) {
    if (!static_cast<MipsCCState *>(&State)->IsCallOperandFixed(ValNo)) {
      if (!CC_Mips_VarArg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (!CC_Mips_FixedArg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


static bool CC_Mips16RetHelper(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    if (!CC_Mips_ByVal(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      Mips::V0, Mips::V1, Mips::A0, Mips::A1
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}


static bool CC_MipsN(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::i32 ||
      LocVT == MVT::i64) {
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isLittle()) {
      if (ArgFlags.isInReg()) {
        LocVT = MVT::i64;
        if (ArgFlags.isSExt())
                LocInfo = CCValAssign::SExtUpper;
        else if (ArgFlags.isZExt())
                LocInfo = CCValAssign::ZExtUpper;
        else
                LocInfo = CCValAssign::AExtUpper;
      }
    }
  }

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::i32) {
    if (!static_cast<MipsCCState *>(&State)->WasOriginalArgFloat(ValNo)) {
      LocVT = MVT::i64;
      if (ArgFlags.isSExt())
            LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
            LocInfo = CCValAssign::ZExt;
      else
            LocInfo = CCValAssign::AExt;
    }
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
    if (LocVT == MVT::i32) {
      if (!CC_MipsN_SoftFloat(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64
    };
    static const MCPhysReg RegList2[] = {
      Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64, Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
    };
    if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      Mips::F12, Mips::F13, Mips::F14, Mips::F15, Mips::F16, Mips::F17, Mips::F18, Mips::F19
    };
    static const MCPhysReg RegList4[] = {
      Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64
    };
    if (unsigned Reg = State.AllocateReg(RegList3, RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList5[] = {
      Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64, Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
    };
    static const MCPhysReg RegList6[] = {
      Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64
    };
    if (unsigned Reg = State.AllocateReg(RegList5, RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    unsigned Offset7 = State.AllocateStack(4, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset8 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_MipsN_FastCC(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64, Mips::T4_64, Mips::T5_64, Mips::T6_64, Mips::T7_64, Mips::T8_64, Mips::V1_64
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      Mips::D0_64, Mips::D1_64, Mips::D2_64, Mips::D3_64, Mips::D4_64, Mips::D5_64, Mips::D6_64, Mips::D7_64, Mips::D8_64, Mips::D9_64, Mips::D10_64, Mips::D11_64, Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64, Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset3 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_MipsN_SoftFloat(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  static const MCPhysReg RegList1[] = {
    Mips::A0, Mips::A1, Mips::A2, Mips::A3, Mips::T0, Mips::T1, Mips::T2, Mips::T3
  };
  static const MCPhysReg RegList2[] = {
    Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64, Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
  };
  if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return false;
  }

  unsigned Offset3 = State.AllocateStack(4, 8);
  State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
  return false;

  return true;  // CC didn't match.
}


static bool CC_MipsN_VarArg(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::i32 ||
      LocVT == MVT::i64) {
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isLittle()) {
      if (ArgFlags.isInReg()) {
        LocVT = MVT::i64;
        if (ArgFlags.isSExt())
                LocInfo = CCValAssign::SExtUpper;
        else if (ArgFlags.isZExt())
                LocInfo = CCValAssign::ZExtUpper;
        else
                LocInfo = CCValAssign::AExtUpper;
      }
    }
  }

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::i32) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExt;
    else
        LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList1[] = {
      Mips::A0, Mips::A1, Mips::A2, Mips::A3, Mips::T0, Mips::T1, Mips::T2, Mips::T3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      Mips::A0_64, Mips::A1_64, Mips::A2_64, Mips::A3_64, Mips::T0_64, Mips::T1_64, Mips::T2_64, Mips::T3_64
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    unsigned Offset3 = State.AllocateStack(4, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset4 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_MipsO32(unsigned ValNo, MVT ValVT,
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

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset1 = State.AllocateStack(4, 4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    unsigned Offset2 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_MipsO32_FP(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
    if (!CC_MipsO32_FP32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
    if (!CC_MipsO32_FP64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  return true;  // CC didn't match.
}


static bool CC_MipsO32_FastCC(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f64) {
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
      static const MCPhysReg RegList1[] = {
        Mips::D0, Mips::D1, Mips::D2, Mips::D3, Mips::D4, Mips::D5, Mips::D6, Mips::D7, Mips::D8, Mips::D9
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
      if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useOddSPReg()) {
        static const MCPhysReg RegList2[] = {
          Mips::D0_64, Mips::D1_64, Mips::D2_64, Mips::D3_64, Mips::D4_64, Mips::D5_64, Mips::D6_64, Mips::D7_64, Mips::D8_64, Mips::D9_64, Mips::D10_64, Mips::D11_64, Mips::D12_64, Mips::D13_64, Mips::D14_64, Mips::D15_64, Mips::D16_64, Mips::D17_64, Mips::D18_64, Mips::D19_64
        };
        if (unsigned Reg = State.AllocateReg(RegList2)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
      if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).noOddSPReg()) {
        static const MCPhysReg RegList3[] = {
          Mips::D0_64, Mips::D2_64, Mips::D4_64, Mips::D6_64, Mips::D8_64, Mips::D10_64, Mips::D12_64, Mips::D14_64, Mips::D16_64, Mips::D18_64
        };
        if (unsigned Reg = State.AllocateReg(RegList3)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (LocVT == MVT::f64) {
    unsigned Offset4 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_Mips_ByVal(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_O32()) {
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, 4, ArgFlags);
      return false;
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, 8, ArgFlags);
    return false;
  }

  return true;  // CC didn't match.
}


static bool CC_Mips_FastCC(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, 4, ArgFlags);
    return false;
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
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isTargetNaCl()) {
      static const MCPhysReg RegList1[] = {
        Mips::A0, Mips::A1, Mips::A2, Mips::A3, Mips::T0, Mips::T1, Mips::T2, Mips::T3, Mips::T4, Mips::T5, Mips::T6, Mips::T7, Mips::T8, Mips::V1
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isTargetNaCl()) {
      static const MCPhysReg RegList2[] = {
        Mips::A0, Mips::A1, Mips::A2, Mips::A3, Mips::T0, Mips::T1, Mips::T2, Mips::T3, Mips::T4, Mips::T5, Mips::V1
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f32) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useOddSPReg()) {
      static const MCPhysReg RegList3[] = {
        Mips::F0, Mips::F1, Mips::F2, Mips::F3, Mips::F4, Mips::F5, Mips::F6, Mips::F7, Mips::F8, Mips::F9, Mips::F10, Mips::F11, Mips::F12, Mips::F13, Mips::F14, Mips::F15, Mips::F16, Mips::F17, Mips::F18, Mips::F19
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f32) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).noOddSPReg()) {
      static const MCPhysReg RegList4[] = {
        Mips::F0, Mips::F2, Mips::F4, Mips::F6, Mips::F8, Mips::F10, Mips::F12, Mips::F14, Mips::F16, Mips::F18
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset5 = State.AllocateStack(4, 4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
    return false;
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_O32()) {
    if (!CC_MipsO32_FastCC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_MipsN_FastCC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


static bool CC_Mips_FixedArg(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() != CallingConv::Fast) {
    if (static_cast<MipsCCState *>(&State)->getSpecialCallingConv() == MipsCCState::Mips16RetHelperConv) {
      if (!CC_Mips16RetHelper(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (ArgFlags.isByVal()) {
    if (!CC_Mips_ByVal(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i64) {
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
      if (static_cast<MipsCCState *>(&State)->WasOriginalArgF128(ValNo)) {
        LocVT = MVT::f64;
        LocInfo = CCValAssign::BCvt;
      }
    }
  }

  if (State.getCallingConv() == CallingConv::Fast) {
    if (!CC_Mips_FastCC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_O32()) {
    if (!CC_MipsO32_FP(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_MipsN(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


static bool CC_Mips_VarArg(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    if (!CC_Mips_ByVal(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_O32()) {
    if (!CC_MipsO32_FP(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_MipsN_VarArg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


static bool RetCC_F128(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
    if (LocVT == MVT::i64) {
      if (!RetCC_F128SoftFloat(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
    if (LocVT == MVT::i64) {
      if (!RetCC_F128HardFloat(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  return true;  // CC didn't match.
}


static bool RetCC_F128HardFloat(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  LocVT = MVT::f64;
  LocInfo = CCValAssign::BCvt;

  if (ArgFlags.isInReg()) {
    static const MCPhysReg RegList1[] = {
      Mips::D0_64, Mips::D1_64
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  static const MCPhysReg RegList2[] = {
    Mips::D0_64, Mips::D2_64
  };
  if (unsigned Reg = State.AllocateReg(RegList2)) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool RetCC_F128SoftFloat(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  static const MCPhysReg RegList1[] = {
    Mips::V0_64, Mips::A0_64
  };
  if (unsigned Reg = State.AllocateReg(RegList1)) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


static bool RetCC_Mips(unsigned ValNo, MVT ValVT,
                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_N32()) {
    if (!RetCC_MipsN(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isABI_N64()) {
    if (!RetCC_MipsN(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_MipsO32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


static bool RetCC_MipsN(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i64) {
    if (static_cast<MipsCCState *>(&State)->WasOriginalArgF128(ValNo)) {
      if (!RetCC_F128(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isLittle()) {
    if (LocVT == MVT::i8 ||
        LocVT == MVT::i16 ||
        LocVT == MVT::i32 ||
        LocVT == MVT::i64) {
      if (ArgFlags.isInReg()) {
        LocVT = MVT::i64;
        if (ArgFlags.isSExt())
                LocInfo = CCValAssign::SExt;
        else if (ArgFlags.isZExt())
                LocInfo = CCValAssign::ZExt;
        else
                LocInfo = CCValAssign::AExt;
      }
    }
  }

  if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isLittle()) {
    if (LocVT == MVT::i8 ||
        LocVT == MVT::i16 ||
        LocVT == MVT::i32 ||
        LocVT == MVT::i64) {
      if (ArgFlags.isInReg()) {
        LocVT = MVT::i64;
        if (ArgFlags.isSExt())
                LocInfo = CCValAssign::SExtUpper;
        else if (ArgFlags.isZExt())
                LocInfo = CCValAssign::ZExtUpper;
        else
                LocInfo = CCValAssign::AExtUpper;
      }
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      Mips::V0_64, Mips::V1_64
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList2[] = {
      Mips::F0, Mips::F2
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList3[] = {
      Mips::D0_64, Mips::D2_64
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}


static bool RetCC_MipsO32(unsigned ValNo, MVT ValVT,
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

  if (!static_cast<MipsCCState *>(&State)->WasOriginalRetVectorFloat(ValNo)) {
    if (LocVT == MVT::i32) {
      static const MCPhysReg RegList1[] = {
        Mips::V0, Mips::V1, Mips::A0, Mips::A1
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList2[] = {
      Mips::F0, Mips::F2
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
      static const MCPhysReg RegList3[] = {
        Mips::D0_64, Mips::D2_64
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (!static_cast<const MipsSubtarget&>(State.getMachineFunction().getSubtarget()).isFP64bit()) {
      static const MCPhysReg RegList4[] = {
        Mips::D0, Mips::D1
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true;  // CC didn't match.
}
