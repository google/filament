/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Calling Convention Implementation Fragment                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

bool llvm::CC_AArch64_AAPCS(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_DarwinPCS(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_DarwinPCS_ILP32_VarArg(unsigned ValNo, MVT ValVT,
                                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                                             ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_DarwinPCS_VarArg(unsigned ValNo, MVT ValVT,
                                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                                       ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_Win64_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                                          ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_AArch64_Win64_VarArg(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_AArch64_AAPCS(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_AArch64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State);


bool llvm::CC_AArch64_AAPCS(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::iPTR) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f32) {
    LocVT = MVT::v2i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (State.getMachineFunction().getDataLayout().isBigEndian()) {
    if (LocVT == MVT::v2i32 ||
        LocVT == MVT::v2f32 ||
        LocVT == MVT::v4i16 ||
        LocVT == MVT::v4f16 ||
        LocVT == MVT::v8i8) {
      LocVT = MVT::f64;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (State.getMachineFunction().getDataLayout().isBigEndian()) {
    if (LocVT == MVT::v2i64 ||
        LocVT == MVT::v2f64 ||
        LocVT == MVT::v4i32 ||
        LocVT == MVT::v4f32 ||
        LocVT == MVT::v8i16 ||
        LocVT == MVT::v8f16 ||
        LocVT == MVT::v16i8) {
      LocVT = MVT::f128;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (ArgFlags.isInReg()) {
    if (LocVT == MVT::i64) {
      if (ArgFlags.isSRet()) {
        if (LocVT == MVT::i64) {
          static const MCPhysReg RegList1[] = {
            AArch64::X0, AArch64::X1
          };
          static const MCPhysReg RegList2[] = {
            AArch64::W0, AArch64::W1
          };
          if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
            State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
            return false;
          }
        }
      }
    }
  }

  if (ArgFlags.isSRet()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X8, AArch64::W8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, 8, ArgFlags);
    return false;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(AArch64::X18)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X20, AArch64::W20)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X21, AArch64::W21)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isInConsecutiveRegs()) {
    if (CC_AArch64_Custom_Block(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
  }

  if (LocVT == MVT::nxv16i8 ||
      LocVT == MVT::nxv8i16 ||
      LocVT == MVT::nxv4i32 ||
      LocVT == MVT::nxv2i64 ||
      LocVT == MVT::nxv2f16 ||
      LocVT == MVT::nxv4f16 ||
      LocVT == MVT::nxv8f16 ||
      LocVT == MVT::nxv2f32 ||
      LocVT == MVT::nxv4f32 ||
      LocVT == MVT::nxv2f64) {
    static const MCPhysReg RegList3[] = {
      AArch64::Z0, AArch64::Z1, AArch64::Z2, AArch64::Z3, AArch64::Z4, AArch64::Z5, AArch64::Z6, AArch64::Z7
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::nxv16i8 ||
      LocVT == MVT::nxv8i16 ||
      LocVT == MVT::nxv4i32 ||
      LocVT == MVT::nxv2i64 ||
      LocVT == MVT::nxv2f16 ||
      LocVT == MVT::nxv4f16 ||
      LocVT == MVT::nxv8f16 ||
      LocVT == MVT::nxv2f32 ||
      LocVT == MVT::nxv4f32 ||
      LocVT == MVT::nxv2f64) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::nxv2i1 ||
      LocVT == MVT::nxv4i1 ||
      LocVT == MVT::nxv8i1 ||
      LocVT == MVT::nxv16i1) {
    static const MCPhysReg RegList4[] = {
      AArch64::P0, AArch64::P1, AArch64::P2, AArch64::P3
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::nxv2i1 ||
      LocVT == MVT::nxv4i1 ||
      LocVT == MVT::nxv8i1 ||
      LocVT == MVT::nxv16i1) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
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

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList5[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    static const MCPhysReg RegList6[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    if (unsigned Reg = State.AllocateReg(RegList5, RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      static const MCPhysReg RegList7[] = {
        AArch64::X0, AArch64::X2, AArch64::X4, AArch64::X6
      };
      static const MCPhysReg RegList8[] = {
        AArch64::X0, AArch64::X1, AArch64::X3, AArch64::X5
      };
      if (unsigned Reg = State.AllocateReg(RegList7, RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      static const MCPhysReg ShadowRegList9[] = {
        AArch64::X7
      };
      unsigned Offset10 = State.AllocateStack(8, 16, ShadowRegList9);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset10, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList11[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    static const MCPhysReg RegList12[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    if (unsigned Reg = State.AllocateReg(RegList11, RegList12)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f16) {
    static const MCPhysReg RegList13[] = {
      AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7
    };
    static const MCPhysReg RegList14[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList13, RegList14)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList15[] = {
      AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7
    };
    static const MCPhysReg RegList16[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList15, RegList16)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList17[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList18[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList17, RegList18)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v4f16) {
    static const MCPhysReg RegList19[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList20[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList19, RegList20)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    static const MCPhysReg RegList21[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList21)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::f16) {
    unsigned Offset22 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset22, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset23 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset23, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v4f16) {
    unsigned Offset24 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset24, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f128 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    unsigned Offset25 = State.AllocateStack(16, 16);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset25, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_DarwinPCS(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::iPTR) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f32) {
    LocVT = MVT::v2i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::f128) {
    LocVT = MVT::v2i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isSRet()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X8, AArch64::W8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, 8, ArgFlags);
    return false;
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X20, AArch64::W20)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X21, AArch64::W21)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isInConsecutiveRegs()) {
    if (CC_AArch64_Custom_Block(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
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

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    static const MCPhysReg RegList2[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      static const MCPhysReg RegList3[] = {
        AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6
      };
      static const MCPhysReg RegList4[] = {
        AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6
      };
      if (unsigned Reg = State.AllocateReg(RegList3, RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      static const MCPhysReg ShadowRegList5[] = {
        AArch64::X7
      };
      unsigned Offset6 = State.AllocateStack(8, 16, ShadowRegList5);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset6, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList7[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    static const MCPhysReg RegList8[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    if (unsigned Reg = State.AllocateReg(RegList7, RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f16) {
    static const MCPhysReg RegList9[] = {
      AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7
    };
    static const MCPhysReg RegList10[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList9, RegList10)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList11[] = {
      AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7
    };
    static const MCPhysReg RegList12[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList11, RegList12)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList13[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList14[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList13, RegList14)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v4f16) {
    static const MCPhysReg RegList15[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList16[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList15, RegList16)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    static const MCPhysReg RegList17[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList17)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ValVT == MVT::i1 || ValVT == MVT::i8) {
    unsigned Offset18 = State.AllocateStack(1, 1);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset18, LocVT, LocInfo));
    return false;
  }

  if (ValVT == MVT::i16 || ValVT == MVT::f16) {
    unsigned Offset19 = State.AllocateStack(2, 2);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset19, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset20 = State.AllocateStack(4, 4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset20, LocVT, LocInfo));
    return false;
  }

  if (ArgFlags.isPointer()) {
    if (State.getMachineFunction().getDataLayout().getPointerSize() == 4) {
      LocVT = MVT::i32;
      LocInfo = CCValAssign::Trunc;
    }
  }

  if (ArgFlags.isPointer()) {
    if (State.getMachineFunction().getDataLayout().getPointerSize() == 4) {
      unsigned Offset21 = State.AllocateStack(4, 4);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset21, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v4f16) {
    unsigned Offset22 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset22, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    unsigned Offset23 = State.AllocateStack(16, 16);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset23, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_DarwinPCS_ILP32_VarArg(unsigned ValNo, MVT ValVT,
                                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v2f32) {
    LocVT = MVT::v2i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::f128) {
    LocVT = MVT::v2i64;
    LocInfo = CCValAssign::BCvt;
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

  if (LocVT == MVT::f16) {
    LocVT = MVT::f32;
    LocInfo = CCValAssign::FPExt;
  }

  if (ArgFlags.isPointer()) {
    if (State.getMachineFunction().getDataLayout().getPointerSize() == 4) {
      LocVT = MVT::i32;
      LocInfo = CCValAssign::Trunc;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset1 = State.AllocateStack(4, 4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      unsigned Offset2 = State.AllocateStack(8, 16);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v4f16) {
    unsigned Offset3 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    unsigned Offset4 = State.AllocateStack(16, 16);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_DarwinPCS_VarArg(unsigned ValNo, MVT ValVT,
                                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::iPTR) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f32) {
    LocVT = MVT::v2i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::f128) {
    LocVT = MVT::v2i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isInConsecutiveRegs()) {
    if (CC_AArch64_Custom_Stack_Block(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
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

  if (LocVT == MVT::f16 ||
      LocVT == MVT::f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::FPExt;
  }

  if (LocVT == MVT::i64) {
    if (ArgFlags.isSplit()) {
      unsigned Offset1 = State.AllocateStack(8, 16);
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v4f16) {
    unsigned Offset2 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    unsigned Offset3 = State.AllocateStack(16, 16);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::iPTR) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v2f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::f128) {
    LocVT = MVT::v2f64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      AArch64::Q4, AArch64::Q5
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList2[] = {
      AArch64::S8, AArch64::S9, AArch64::S10, AArch64::S11
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList3[] = {
      AArch64::D12, AArch64::D13, AArch64::D14, AArch64::D15
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
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

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList4[] = {
      AArch64::X19, AArch64::X20, AArch64::X21, AArch64::X22, AArch64::X23, AArch64::X24, AArch64::X25, AArch64::X26, AArch64::X27, AArch64::X28
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_WebKit_JS(unsigned ValNo, MVT ValVT,
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
    if (unsigned Reg = State.AllocateReg(AArch64::W0, AArch64::X0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(AArch64::X0, AArch64::W0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset1 = State.AllocateStack(4, 4);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset2 = State.AllocateStack(8, 8);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_Win64_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(AArch64::X15)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}


bool llvm::CC_AArch64_Win64_VarArg(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f16 ||
      LocVT == MVT::f32) {
    LocVT = MVT::f64;
    LocInfo = CCValAssign::FPExt;
  }

  if (LocVT == MVT::f64) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (!CC_AArch64_AAPCS(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true;  // CC didn't match.
}


bool llvm::RetCC_AArch64_AAPCS(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::iPTR) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f32) {
    LocVT = MVT::v2i32;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::v2f64 ||
      LocVT == MVT::v4f32) {
    LocVT = MVT::v2i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (ArgFlags.isInConsecutiveRegs()) {
    if (CC_AArch64_Custom_Block(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(AArch64::X21, AArch64::W21)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (State.getMachineFunction().getDataLayout().isBigEndian()) {
    if (LocVT == MVT::v2i32 ||
        LocVT == MVT::v2f32 ||
        LocVT == MVT::v4i16 ||
        LocVT == MVT::v4f16 ||
        LocVT == MVT::v8i8) {
      LocVT = MVT::f64;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (State.getMachineFunction().getDataLayout().isBigEndian()) {
    if (LocVT == MVT::v2i64 ||
        LocVT == MVT::v2f64 ||
        LocVT == MVT::v4i32 ||
        LocVT == MVT::v4f32 ||
        LocVT == MVT::v8i16 ||
        LocVT == MVT::v8f16 ||
        LocVT == MVT::v16i8) {
      LocVT = MVT::f128;
      LocInfo = CCValAssign::BCvt;
    }
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

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    static const MCPhysReg RegList2[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList3[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    static const MCPhysReg RegList4[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    if (unsigned Reg = State.AllocateReg(RegList3, RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f16) {
    static const MCPhysReg RegList5[] = {
      AArch64::H0, AArch64::H1, AArch64::H2, AArch64::H3, AArch64::H4, AArch64::H5, AArch64::H6, AArch64::H7
    };
    static const MCPhysReg RegList6[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList5, RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList7[] = {
      AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7
    };
    static const MCPhysReg RegList8[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList7, RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList9[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList10[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList9, RegList10)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v1i64 ||
      LocVT == MVT::v2i32 ||
      LocVT == MVT::v4i16 ||
      LocVT == MVT::v8i8 ||
      LocVT == MVT::v1f64 ||
      LocVT == MVT::v2f32 ||
      LocVT == MVT::v4f16) {
    static const MCPhysReg RegList11[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList12[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList11, RegList12)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v8f16) {
    static const MCPhysReg RegList13[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList13)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::nxv16i8 ||
      LocVT == MVT::nxv8i16 ||
      LocVT == MVT::nxv4i32 ||
      LocVT == MVT::nxv2i64 ||
      LocVT == MVT::nxv2f16 ||
      LocVT == MVT::nxv4f16 ||
      LocVT == MVT::nxv8f16 ||
      LocVT == MVT::nxv2f32 ||
      LocVT == MVT::nxv4f32 ||
      LocVT == MVT::nxv2f64) {
    static const MCPhysReg RegList14[] = {
      AArch64::Z0, AArch64::Z1, AArch64::Z2, AArch64::Z3, AArch64::Z4, AArch64::Z5, AArch64::Z6, AArch64::Z7
    };
    if (unsigned Reg = State.AllocateReg(RegList14)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::nxv2i1 ||
      LocVT == MVT::nxv4i1 ||
      LocVT == MVT::nxv8i1 ||
      LocVT == MVT::nxv16i1) {
    static const MCPhysReg RegList15[] = {
      AArch64::P0, AArch64::P1, AArch64::P2, AArch64::P3
    };
    if (unsigned Reg = State.AllocateReg(RegList15)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}


bool llvm::RetCC_AArch64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    static const MCPhysReg RegList2[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList3[] = {
      AArch64::X0, AArch64::X1, AArch64::X2, AArch64::X3, AArch64::X4, AArch64::X5, AArch64::X6, AArch64::X7
    };
    static const MCPhysReg RegList4[] = {
      AArch64::W0, AArch64::W1, AArch64::W2, AArch64::W3, AArch64::W4, AArch64::W5, AArch64::W6, AArch64::W7
    };
    if (unsigned Reg = State.AllocateReg(RegList3, RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList5[] = {
      AArch64::S0, AArch64::S1, AArch64::S2, AArch64::S3, AArch64::S4, AArch64::S5, AArch64::S6, AArch64::S7
    };
    static const MCPhysReg RegList6[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList5, RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList7[] = {
      AArch64::D0, AArch64::D1, AArch64::D2, AArch64::D3, AArch64::D4, AArch64::D5, AArch64::D6, AArch64::D7
    };
    static const MCPhysReg RegList8[] = {
      AArch64::Q0, AArch64::Q1, AArch64::Q2, AArch64::Q3, AArch64::Q4, AArch64::Q5, AArch64::Q6, AArch64::Q7
    };
    if (unsigned Reg = State.AllocateReg(RegList7, RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true;  // CC didn't match.
}
