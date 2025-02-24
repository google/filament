/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Calling Convention Implementation Fragment                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef GET_CC_REGISTER_LISTS

bool llvm::CC_PPC32_SVR4(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_PPC32_SVR4_ByVal(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_PPC32_SVR4_Common(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_PPC32_SVR4_VarArg(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_PPC64_AnyReg(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_PPC(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_PPC64_AnyReg(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_PPC_Cold(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);


bool llvm::CC_PPC32_SVR4(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v1i128 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList1[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9, PPC::V10, PPC::V11, PPC::V12, PPC::V13
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f128) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList2[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9, PPC::V10, PPC::V11, PPC::V12, PPC::V13
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!CC_PPC32_SVR4_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::CC_PPC32_SVR4_ByVal(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (CC_PPC32_SVR4_Custom_Dummy(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_PPC32_SVR4_Common(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.isSplit()) {
      if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
        if (!static_cast<PPCCCState *>(&State)->WasOriginalArgPPCF128(ValNo)) {
          if (CC_PPC32_SVR4_Custom_AlignArgRegs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
            return false;
        }
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.isSplit()) {
      if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
        if (CC_PPC32_SVR4_Custom_AlignArgRegs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
          return false;
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
      if (CC_PPC32_SVR4_Custom_AlignArgRegs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (ArgFlags.isSplit()) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).useSoftFloat()) {
      if (static_cast<PPCCCState *>(&State)->WasOriginalArgPPCF128(ValNo)) {
        if (CC_PPC32_SVR4_Custom_SkipLastArgRegsPPCF128(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
          return false;
      }
    }
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(PPC::R11)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9, PPC::R10
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    if (ArgFlags.isSplit()) {
      if (CC_PPC32_SVR4_Custom_AlignFPArgRegs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
      static const MCPhysReg RegList2[] = {
        PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
      if (CC_PPC32_SPE_CustomSplitFP64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f32) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
      static const MCPhysReg RegList3[] = {
        PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9, PPC::R10
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (ArgFlags.isSplit()) {
      unsigned Offset4 = State.AllocateStack(4, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    unsigned Offset5 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f32) {
    unsigned Offset6 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset6, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    unsigned Offset7 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64 ||
      LocVT == MVT::v2i64) {
    unsigned Offset8 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f128) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      unsigned Offset9 = State.AllocateStack(16, Align(16));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::CC_PPC32_SVR4_VarArg(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (!CC_PPC32_SVR4_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_PPC64_AnyReg(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_PPC_AnyReg_Error(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::CC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::AnyReg) {
    if (!CC_PPC64_AnyReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i16) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      PPC::X3, PPC::X4, PPC::X5, PPC::X6, PPC::X7, PPC::X8, PPC::X9, PPC::X10
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    static const MCPhysReg RegList2[] = {
      PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


bool llvm::RetCC_PPC(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::AnyReg) {
    if (!RetCC_PPC64_AnyReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::i1) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).isPPC64()) {
      LocVT = MVT::i64;
      if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExt;
      else
        LocInfo = CCValAssign::AExt;
    }
  }

  if (LocVT == MVT::i1) {
    if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).isPPC64()) {
      LocVT = MVT::i32;
      if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExt;
      else
        LocInfo = CCValAssign::AExt;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9, PPC::R10
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      PPC::X3, PPC::X4, PPC::X5, PPC::X6
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i128) {
    static const MCPhysReg RegList3[] = {
      PPC::X3, PPC::X4, PPC::X5, PPC::X6
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
    if (LocVT == MVT::f32) {
      static const MCPhysReg RegList4[] = {
        PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
    if (LocVT == MVT::f64) {
      static const MCPhysReg RegList5[] = {
        PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
    if (LocVT == MVT::f32) {
      static const MCPhysReg RegList6[] = {
        PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9, PPC::R10
      };
      if (unsigned Reg = State.AllocateReg(RegList6)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasSPE()) {
    if (LocVT == MVT::f64) {
      if (CC_PPC32_SPE_RetF64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f128) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList7[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9
      };
      if (unsigned Reg = State.AllocateReg(RegList7)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v1i128 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList8[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9
      };
      if (unsigned Reg = State.AllocateReg(RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_PPC64_AnyReg(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_PPC_AnyReg_Error(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::RetCC_PPC64_ELF_FIS(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::AnyReg) {
    if (!RetCC_PPC64_AnyReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i16) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i32) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      PPC::X3, PPC::X4, PPC::X5, PPC::X6
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i128) {
    static const MCPhysReg RegList2[] = {
      PPC::X3, PPC::X4, PPC::X5, PPC::X6
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList3[] = {
      PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList4[] = {
      PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList5[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v1i128 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      static const MCPhysReg RegList6[] = {
        PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9
      };
      if (unsigned Reg = State.AllocateReg(RegList6)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


bool llvm::RetCC_PPC_Cold(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32 ||
      LocVT == MVT::i1) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).isPPC64()) {
      LocVT = MVT::i64;
      if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExt;
      else
        LocInfo = CCValAssign::AExt;
    }
  }

  if (LocVT == MVT::i1) {
    if (!static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).isPPC64()) {
      LocVT = MVT::i32;
      if (ArgFlags.isSExt())
        LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
        LocInfo = CCValAssign::ZExt;
      else
        LocInfo = CCValAssign::AExt;
    }
  }

  if (LocVT == MVT::i32) {
    if (unsigned Reg = State.AllocateReg(PPC::R3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(PPC::X3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i128) {
    if (unsigned Reg = State.AllocateReg(PPC::X3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    if (unsigned Reg = State.AllocateReg(PPC::F1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    if (unsigned Reg = State.AllocateReg(PPC::F1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      if (unsigned Reg = State.AllocateReg(PPC::V2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v1i128 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const PPCSubtarget&>(State.getMachineFunction().getSubtarget()).hasAltivec()) {
      if (unsigned Reg = State.AllocateReg(PPC::V2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}

#else

const MCRegister CC_PPC32_SVR4_ArgRegs[] = { PPC::V10, PPC::V11, PPC::V12, PPC::V13, PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9 };
const MCRegister CC_PPC32_SVR4_ByVal_ArgRegs[] = { 0 };
const MCRegister CC_PPC32_SVR4_Common_ArgRegs[] = { PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8, PPC::R10, PPC::R11, PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9 };
const MCRegister CC_PPC32_SVR4_VarArg_ArgRegs[] = { 0 };
const MCRegister CC_PPC64_AnyReg_ArgRegs[] = { 0 };
const MCRegister CC_PPC64_ELF_FIS_ArgRegs[] = { PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8, PPC::X10, PPC::X3, PPC::X4, PPC::X5, PPC::X6, PPC::X7, PPC::X8, PPC::X9 };
const MCRegister RetCC_PPC_ArgRegs[] = { PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8, PPC::R10, PPC::R3, PPC::R4, PPC::R5, PPC::R6, PPC::R7, PPC::R8, PPC::R9, PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9, PPC::X3, PPC::X4, PPC::X5, PPC::X6 };
const MCRegister RetCC_PPC64_AnyReg_ArgRegs[] = { 0 };
const MCRegister RetCC_PPC64_ELF_FIS_ArgRegs[] = { PPC::F1, PPC::F2, PPC::F3, PPC::F4, PPC::F5, PPC::F6, PPC::F7, PPC::F8, PPC::V2, PPC::V3, PPC::V4, PPC::V5, PPC::V6, PPC::V7, PPC::V8, PPC::V9, PPC::X3, PPC::X4, PPC::X5, PPC::X6 };
const MCRegister RetCC_PPC_Cold_ArgRegs[] = { PPC::F1, PPC::R3, PPC::V2, PPC::X3 };

#endif // CC_REGISTER_LIST
