/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Calling Convention Implementation Fragment                                 *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef GET_CC_REGISTER_LISTS

static bool CC_Intel_OCL_BI(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::CC_X86(unsigned ValNo, MVT ValVT,
                  MVT LocVT, CCValAssign::LocInfo LocInfo,
                  ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32(unsigned ValNo, MVT ValVT,
                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                      ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_C(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_Common(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_FastCC(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_FastCall(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_HiPE(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_MCU(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_RegCall(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_ThisCall(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_ThisCall_Common(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_ThisCall_Mingw(unsigned ValNo, MVT ValVT,
                                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                                     ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_ThisCall_Win(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_Vector_Common(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_Vector_Darwin(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_32_Vector_Standard(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64(unsigned ValNo, MVT ValVT,
                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                      ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_AnyReg(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_C(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_HHVM(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_HHVM_C(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_HiPE(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_SysV64_RegCall(unsigned ValNo, MVT ValVT,
                                  MVT LocVT, CCValAssign::LocInfo LocInfo,
                                  ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win32_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                                       ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win32_Vector(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win32_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win64_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win64_RegCall(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool CC_X86_Win64_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_Intel_OCL_BI(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
bool llvm::RetCC_X86(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86Common(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32_Fast(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32_HiPE(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32_RegCall(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_32_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_AnyReg(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_HHVM(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_HiPE(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_Swift(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_Vectorcall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_SysV64_RegCall(unsigned ValNo, MVT ValVT,
                                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                                     ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_Win64_C(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State);
static bool RetCC_X86_Win64_RegCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State);


static bool CC_Intel_OCL_BI(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
      static const MCPhysReg RegList1[] = {
        X86::ECX, X86::EDX, X86::R8D, X86::R9D
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
      static const MCPhysReg RegList2[] = {
        X86::RCX, X86::RDX, X86::R8, X86::R9
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
      static const MCPhysReg RegList3[] = {
        X86::EDI, X86::ESI, X86::EDX, X86::ECX
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
      static const MCPhysReg RegList4[] = {
        X86::RDI, X86::RSI, X86::RDX, X86::RCX
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i32) {
    unsigned Offset5 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    static const MCPhysReg RegList6[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64) {
    static const MCPhysReg RegList7[] = {
      X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList7)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64) {
    static const MCPhysReg RegList8[] = {
      X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16i1 ||
      LocVT == MVT::v8i1) {
    if (unsigned Reg = State.AllocateReg(X86::K1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
    if (!CC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (!CC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::CC_X86(unsigned ValNo, MVT ValVT,
                  MVT LocVT, CCValAssign::LocInfo LocInfo,
                  ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::Intel_OCL_BI) {
    if (!CC_Intel_OCL_BI(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (!CC_X86_64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32(unsigned ValNo, MVT ValVT,
                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                      ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::X86_INTR) {
    if (CC_X86_Intr(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetMCU()) {
    if (!CC_X86_32_MCU(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_FastCall) {
    if (!CC_X86_32_FastCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_VectorCall) {
    if (!CC_X86_Win32_VectorCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_ThisCall) {
    if (!CC_X86_32_ThisCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::CFGuard_Check) {
    if (!CC_X86_Win32_CFGuard_Check(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Fast) {
    if (!CC_X86_32_FastCC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Tail) {
    if (!CC_X86_32_FastCC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::GHC) {
    if (!CC_X86_32_GHC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HiPE) {
    if (!CC_X86_32_HiPE(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (!CC_X86_32_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_C(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(X86::ECX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (State.getCallingConv() == CallingConv::SwiftTail) {
    if (ArgFlags.isSwiftSelf()) {
      if (LocVT == MVT::i32) {
        if (unsigned Reg = State.AllocateReg(X86::ECX)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (ArgFlags.isInReg()) {
      if (LocVT == MVT::i32) {
        static const MCPhysReg RegList1[] = {
          X86::EAX, X86::EDX, X86::ECX
        };
        if (unsigned Reg = State.AllocateReg(RegList1)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!CC_X86_32_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_Common(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (ArgFlags.isPreallocated()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (!State.isVarArg()) {
    if (ArgFlags.isInReg()) {
      if (LocVT == MVT::f32 ||
          LocVT == MVT::f64) {
        if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
          static const MCPhysReg RegList1[] = {
            X86::XMM0, X86::XMM1, X86::XMM2
          };
          if (unsigned Reg = State.AllocateReg(RegList1)) {
            State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
            return false;
          }
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (ArgFlags.isInReg()) {
      if (LocVT == MVT::f16) {
        static const MCPhysReg RegList2[] = {
          X86::XMM0, X86::XMM1, X86::XMM2
        };
        if (unsigned Reg = State.AllocateReg(RegList2)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::x86mmx) {
      static const MCPhysReg RegList3[] = {
        X86::MM0, X86::MM1, X86::MM2
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f16) {
    unsigned Offset4 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset4, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset5 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset5, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f64) {
    unsigned Offset6 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset6, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f80) {
    unsigned Offset7 = State.AllocateStack(
      State.getMachineFunction().getDataLayout().getTypeAllocSize(EVT(LocVT).getTypeForEVT(State.getContext())), 
      State.getMachineFunction().getDataLayout().getABITypeAlign(EVT(LocVT).getTypeForEVT(State.getContext())));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v2i1) {
    LocVT = MVT::v2i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v4i1) {
    LocVT = MVT::v4i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1) {
    LocVT = MVT::v8i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::v16i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::v32i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::v64i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::x86mmx) {
    unsigned Offset8 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
    return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetDarwin()) {
    if (!CC_X86_32_Vector_Darwin(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32_Vector_Standard(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_FastCC(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(X86::EAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      X86::ECX, X86::EDX
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::f32 ||
        LocVT == MVT::f64) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
        static const MCPhysReg RegList2[] = {
          X86::XMM0, X86::XMM1, X86::XMM2
        };
        if (unsigned Reg = State.AllocateReg(RegList2)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (LocVT == MVT::f64) {
    unsigned Offset3 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  if (!CC_X86_32_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_FastCall(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(X86::EAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ArgFlags.isInReg()) {
    if (LocVT == MVT::i8) {
      static const MCPhysReg RegList1[] = {
        X86::CL, X86::DL
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isInReg()) {
    if (LocVT == MVT::i16) {
      static const MCPhysReg RegList2[] = {
        X86::CX, X86::DX
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isInReg()) {
    if (LocVT == MVT::i32) {
      static const MCPhysReg RegList3[] = {
        X86::ECX, X86::EDX
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!CC_X86_32_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::EBX, X86::EBP, X86::EDI, X86::ESI
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool CC_X86_32_HiPE(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::ESI, X86::EBP, X86::EAX, X86::EDX, X86::ECX
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset2 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_32_MCU(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::i32) {
      if (CC_X86_32_MCUInReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (!CC_X86_32_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_RegCall(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, Align(8), ArgFlags);
      return false;
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1 ||
      LocVT == MVT::v16i1 ||
      LocVT == MVT::v32i1) {
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
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(X86::RAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::RAX)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList2[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f80) {
    if (unsigned Reg = State.AllocateReg(X86::FP0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList3[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList4[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList5[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i32 ||
        LocVT == MVT::i64 ||
        LocVT == MVT::f32 ||
        LocVT == MVT::f64) {
      unsigned Offset6 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset6, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset7 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset8 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
    return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::x86mmx) {
      unsigned Offset9 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    unsigned Offset10 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset10, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f80 ||
      LocVT == MVT::f128) {
    unsigned Offset11 = State.AllocateStack(
      State.getMachineFunction().getDataLayout().getTypeAllocSize(EVT(LocVT).getTypeForEVT(State.getContext())), 
      State.getMachineFunction().getDataLayout().getABITypeAlign(EVT(LocVT).getTypeForEVT(State.getContext())));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset11, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset12 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset12, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset13 = State.AllocateStack(32, Align(32));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset13, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset14 = State.AllocateStack(64, Align(64));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset14, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_32_ThisCall(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetCygMing()) {
    if (!CC_X86_32_ThisCall_Mingw(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32_ThisCall_Win(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_ThisCall_Common(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32) {
    if (unsigned Reg = State.AllocateReg(X86::ECX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!CC_X86_32_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_ThisCall_Mingw(unsigned ValNo, MVT ValVT,
                                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                                     ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (!CC_X86_32_ThisCall_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_ThisCall_Win(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isSRet()) {
    unsigned Offset1 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (!CC_X86_32_ThisCall_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_Vector_Common(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset1 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v16f16 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset2 = State.AllocateStack(32, Align(32));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v32f16 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset3 = State.AllocateStack(64, Align(64));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_32_Vector_Darwin(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (!State.isVarArg()) {
    if (LocVT == MVT::v16i8 ||
        LocVT == MVT::v8i16 ||
        LocVT == MVT::v4i32 ||
        LocVT == MVT::v2i64 ||
        LocVT == MVT::v8f16 ||
        LocVT == MVT::v4f32 ||
        LocVT == MVT::v2f64) {
      static const MCPhysReg RegList1[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v32i8 ||
        LocVT == MVT::v16i16 ||
        LocVT == MVT::v8i32 ||
        LocVT == MVT::v4i64 ||
        LocVT == MVT::v16f16 ||
        LocVT == MVT::v8f32 ||
        LocVT == MVT::v4f64) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
        static const MCPhysReg RegList2[] = {
          X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3
        };
        if (unsigned Reg = State.AllocateReg(RegList2)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v64i8 ||
        LocVT == MVT::v32i16 ||
        LocVT == MVT::v16i32 ||
        LocVT == MVT::v8i64 ||
        LocVT == MVT::v32f16 ||
        LocVT == MVT::v16f32 ||
        LocVT == MVT::v8f64) {
      static const MCPhysReg RegList3[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!CC_X86_32_Vector_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_32_Vector_Standard(unsigned ValNo, MVT ValVT,
                                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                                      ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (!State.isVarArg()) {
    if (LocVT == MVT::v16i8 ||
        LocVT == MVT::v8i16 ||
        LocVT == MVT::v4i32 ||
        LocVT == MVT::v2i64 ||
        LocVT == MVT::v8f16 ||
        LocVT == MVT::v4f32 ||
        LocVT == MVT::v2f64) {
      static const MCPhysReg RegList1[] = {
        X86::XMM0, X86::XMM1, X86::XMM2
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v32i8 ||
        LocVT == MVT::v16i16 ||
        LocVT == MVT::v8i32 ||
        LocVT == MVT::v4i64 ||
        LocVT == MVT::v16f16 ||
        LocVT == MVT::v8f32 ||
        LocVT == MVT::v4f64) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
        static const MCPhysReg RegList2[] = {
          X86::YMM0, X86::YMM1, X86::YMM2
        };
        if (unsigned Reg = State.AllocateReg(RegList2)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v64i8 ||
        LocVT == MVT::v32i16 ||
        LocVT == MVT::v16i32 ||
        LocVT == MVT::v8i64 ||
        LocVT == MVT::v32f16 ||
        LocVT == MVT::v16f32 ||
        LocVT == MVT::v8f64) {
      static const MCPhysReg RegList3[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (State.isVarArg() && State.getMachineFunction().getSubtarget().getTargetTriple().isWindowsMSVCEnvironment()) {
    if (!CC_X86_Win32_Vector(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_32_Vector_Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_64(unsigned ValNo, MVT ValVT,
                      MVT LocVT, CCValAssign::LocInfo LocInfo,
                      ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::GHC) {
    if (!CC_X86_64_GHC(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HiPE) {
    if (!CC_X86_64_HiPE(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::WebKit_JS) {
    if (!CC_X86_64_WebKit_JS(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::AnyReg) {
    if (!CC_X86_64_AnyReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Win64) {
    if (!CC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_64_SysV) {
    if (!CC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_VectorCall) {
    if (!CC_X86_Win64_VectorCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HHVM) {
    if (!CC_X86_64_HHVM(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HHVM_C) {
    if (!CC_X86_64_HHVM_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
      if (!CC_X86_Win64_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (!CC_X86_SysV64_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_INTR) {
    if (CC_X86_Intr(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
    if (!CC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!CC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_64_AnyReg(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_X86_AnyReg_Error(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_64_C(unsigned ValNo, MVT ValVT,
                        MVT LocVT, CCValAssign::LocInfo LocInfo,
                        ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, Align(8), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isNest()) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTarget64BitILP32()) {
      if (unsigned Reg = State.AllocateReg(X86::R10D)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(X86::R10)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R13)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R12)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftAsync()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R14)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (State.getCallingConv() == CallingConv::Swift) {
    if (ArgFlags.isSRet()) {
      if (LocVT == MVT::i64) {
        if (unsigned Reg = State.AllocateReg(X86::RAX)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (State.getCallingConv() == CallingConv::SwiftTail) {
    if (ArgFlags.isSRet()) {
      if (LocVT == MVT::i64) {
        if (unsigned Reg = State.AllocateReg(X86::RAX)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (ArgFlags.isPointer()) {
    if (CC_X86_64_Pointer(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList1[] = {
      X86::EDI, X86::ESI, X86::EDX, X86::ECX, X86::R8D, X86::R9D
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      X86::RDI, X86::RSI, X86::RDX, X86::RCX, X86::R8, X86::R9
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetDarwin()) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
        LocVT = MVT::v2i64;
        if (ArgFlags.isSExt())
          LocInfo = CCValAssign::SExt;
        else if (ArgFlags.isZExt())
          LocInfo = CCValAssign::ZExt;
        else
          LocInfo = CCValAssign::AExt;
      }
    }
  }

  if (LocVT == MVT::v2i1) {
    LocVT = MVT::v2i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v4i1) {
    LocVT = MVT::v4i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1) {
    LocVT = MVT::v8i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::v16i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::v32i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::v64i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList3[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v32i8 ||
        LocVT == MVT::v16i16 ||
        LocVT == MVT::v8i32 ||
        LocVT == MVT::v4i64 ||
        LocVT == MVT::v16f16 ||
        LocVT == MVT::v8f32 ||
        LocVT == MVT::v4f64) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
        static const MCPhysReg RegList4[] = {
          X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7
        };
        if (unsigned Reg = State.AllocateReg(RegList4)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (!State.isVarArg()) {
    if (LocVT == MVT::v64i8 ||
        LocVT == MVT::v32i16 ||
        LocVT == MVT::v16i32 ||
        LocVT == MVT::v8i64 ||
        LocVT == MVT::v32f16 ||
        LocVT == MVT::v16f32 ||
        LocVT == MVT::v8f64) {
      if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
        static const MCPhysReg RegList5[] = {
          X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7
        };
        if (unsigned Reg = State.AllocateReg(RegList5)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::i64 ||
      LocVT == MVT::f16 ||
      LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    unsigned Offset6 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset6, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f80 ||
      LocVT == MVT::f128) {
    unsigned Offset7 = State.AllocateStack(
      State.getMachineFunction().getDataLayout().getTypeAllocSize(EVT(LocVT).getTypeForEVT(State.getContext())), 
      State.getMachineFunction().getDataLayout().getABITypeAlign(EVT(LocVT).getTypeForEVT(State.getContext())));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset7, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset8 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v16f16 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset9 = State.AllocateStack(32, Align(32));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v32f16 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset10 = State.AllocateStack(64, Align(64));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset10, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_64_GHC(unsigned ValNo, MVT ValVT,
                          MVT LocVT, CCValAssign::LocInfo LocInfo,
                          ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::R13, X86::RBP, X86::R12, X86::RBX, X86::R14, X86::RSI, X86::RDI, X86::R8, X86::R9, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList2[] = {
        X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList3[] = {
        X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList4[] = {
        X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


static bool CC_X86_64_HHVM(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList1[] = {
      X86::RBX, X86::R12, X86::RBP, X86::R15, X86::RDI, X86::RSI, X86::RDX, X86::RCX, X86::R8, X86::R9, X86::RAX, X86::R10, X86::R11, X86::R13, X86::R14
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool CC_X86_64_HHVM_C(unsigned ValNo, MVT ValVT,
                             MVT LocVT, CCValAssign::LocInfo LocInfo,
                             ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(X86::RBP)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!CC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_64_HiPE(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::R15, X86::RBP, X86::RSI, X86::RDX, X86::RCX, X86::R8
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::i64 ||
      LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    unsigned Offset2 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    if (unsigned Reg = State.AllocateReg(X86::EAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(X86::RAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset1 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset2 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_SysV64_RegCall(unsigned ValNo, MVT ValVT,
                                  MVT LocVT, CCValAssign::LocInfo LocInfo,
                                  ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, Align(8), ArgFlags);
      return false;
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1 ||
      LocVT == MVT::v16i1 ||
      LocVT == MVT::v32i1) {
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
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI, X86::R8D, X86::R9D, X86::R12D, X86::R13D, X86::R14D, X86::R15D
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R12, X86::R13, X86::R14, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      static const MCPhysReg RegList3[] = {
        X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R12, X86::R13, X86::R14, X86::R15
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList4[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f80) {
    if (unsigned Reg = State.AllocateReg(X86::FP0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList5[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList6[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList6)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList7[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList7)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i32 ||
        LocVT == MVT::i64 ||
        LocVT == MVT::f32 ||
        LocVT == MVT::f64) {
      unsigned Offset8 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset9 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset10 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset10, LocVT, LocInfo));
    return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::x86mmx) {
      unsigned Offset11 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset11, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    unsigned Offset12 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset12, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f80 ||
      LocVT == MVT::f128) {
    unsigned Offset13 = State.AllocateStack(
      State.getMachineFunction().getDataLayout().getTypeAllocSize(EVT(LocVT).getTypeForEVT(State.getContext())), 
      State.getMachineFunction().getDataLayout().getABITypeAlign(EVT(LocVT).getTypeForEVT(State.getContext())));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset13, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset14 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset14, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset15 = State.AllocateStack(32, Align(32));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset15, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset16 = State.AllocateStack(64, Align(64));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset16, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_Win32_CFGuard_Check(unsigned ValNo, MVT ValVT,
                                       MVT LocVT, CCValAssign::LocInfo LocInfo,
                                       ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i32) {
    if (unsigned Reg = State.AllocateReg(X86::ECX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool CC_X86_Win32_Vector(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset1 = State.AllocateStack(16, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset1, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v16f16 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset2 = State.AllocateStack(32, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset2, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v32f16 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset3 = State.AllocateStack(64, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset3, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_Win32_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_X86_32_VectorCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  if (!CC_X86_32_FastCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool CC_X86_Win64_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isByVal()) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (ArgFlags.isNest()) {
    if (unsigned Reg = State.AllocateReg(X86::R10)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R12)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftSelf()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R13)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isSwiftAsync()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R14)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (ArgFlags.isCFGuardTarget()) {
    if (unsigned Reg = State.AllocateReg(X86::RAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v16f16 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v32f16 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64 ||
      LocVT == MVT::v8i64) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::f80) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::Indirect;
  }

  if (LocVT == MVT::x86mmx) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f32) {
    if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      LocVT = MVT::i32;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (LocVT == MVT::f64) {
    if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      LocVT = MVT::i64;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (LocVT == MVT::f16 ||
      LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    static const MCPhysReg RegList1[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    static const MCPhysReg RegList2[] = {
      X86::RCX, X86::RDX, X86::R8, X86::R9
    };
    if (unsigned Reg = State.AllocateReg(RegList1, RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList3[] = {
      X86::CL, X86::DL, X86::R8B, X86::R9B
    };
    static const MCPhysReg RegList4[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList3, RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList5[] = {
      X86::CX, X86::DX, X86::R8W, X86::R9W
    };
    static const MCPhysReg RegList6[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList5, RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList7[] = {
      X86::ECX, X86::EDX, X86::R8D, X86::R9D
    };
    static const MCPhysReg RegList8[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList7, RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (State.getCallingConv() == CallingConv::X86_ThisCall) {
    if (ArgFlags.isSRet()) {
      if (LocVT == MVT::i64) {
        static const MCPhysReg RegList9[] = {
          X86::RDX, X86::R8, X86::R9
        };
        static const MCPhysReg RegList10[] = {
          X86::XMM1, X86::XMM2, X86::XMM3
        };
        if (unsigned Reg = State.AllocateReg(RegList9, RegList10)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList11[] = {
      X86::RCX, X86::RDX, X86::R8, X86::R9
    };
    static const MCPhysReg RegList12[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList11, RegList12)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::i32 ||
      LocVT == MVT::i64 ||
      LocVT == MVT::f16 ||
      LocVT == MVT::f32 ||
      LocVT == MVT::f64) {
    unsigned Offset13 = State.AllocateStack(8, Align(8));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset13, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_Win64_RegCall(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 8, Align(8), ArgFlags);
      return false;
    }
  }

  if (ArgFlags.isByVal()) {
    State.HandleByVal(ValNo, ValVT, LocVT, LocInfo, 4, Align(4), ArgFlags);
    return false;
  }

  if (LocVT == MVT::i1 ||
      LocVT == MVT::i8 ||
      LocVT == MVT::i16 ||
      LocVT == MVT::v1i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1 ||
      LocVT == MVT::v16i1 ||
      LocVT == MVT::v32i1) {
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
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI, X86::R8D, X86::R9D, X86::R10D, X86::R11D, X86::R12D, X86::R14D, X86::R15D
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList2[] = {
      X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R10, X86::R11, X86::R12, X86::R14, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      static const MCPhysReg RegList3[] = {
        X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R10, X86::R11, X86::R12, X86::R14, X86::R15
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList4[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList4)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f80) {
    if (unsigned Reg = State.AllocateReg(X86::FP0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList5[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList6[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList6)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList7[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList7)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i32 ||
        LocVT == MVT::i64 ||
        LocVT == MVT::f32 ||
        LocVT == MVT::f64) {
      unsigned Offset8 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset8, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32 ||
      LocVT == MVT::f32) {
    unsigned Offset9 = State.AllocateStack(4, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset9, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::i64 ||
      LocVT == MVT::f64) {
    unsigned Offset10 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset10, LocVT, LocInfo));
    return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::x86mmx) {
      unsigned Offset11 = State.AllocateStack(8, Align(8));
      State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset11, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    unsigned Offset12 = State.AllocateStack(8, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset12, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::f80 ||
      LocVT == MVT::f128) {
    unsigned Offset13 = State.AllocateStack(
      State.getMachineFunction().getDataLayout().getTypeAllocSize(EVT(LocVT).getTypeForEVT(State.getContext())), 
      State.getMachineFunction().getDataLayout().getABITypeAlign(EVT(LocVT).getTypeForEVT(State.getContext())));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset13, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    unsigned Offset14 = State.AllocateStack(16, Align(16));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset14, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    unsigned Offset15 = State.AllocateStack(32, Align(32));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset15, LocVT, LocInfo));
    return false;
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    unsigned Offset16 = State.AllocateStack(64, Align(64));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset16, LocVT, LocInfo));
    return false;
  }

  return true; // CC didn't match.
}


static bool CC_X86_Win64_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_X86_64_VectorCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  if (!CC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_Intel_OCL_BI(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    static const MCPhysReg RegList1[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64) {
    static const MCPhysReg RegList2[] = {
      X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64) {
    static const MCPhysReg RegList3[] = {
      X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


bool llvm::RetCC_X86(unsigned ValNo, MVT ValVT,
                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                     ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::Intel_OCL_BI) {
    if (!RetCC_Intel_OCL_BI(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (!RetCC_X86_64(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_X86_32(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86Common(unsigned ValNo, MVT ValVT,
                            MVT LocVT, CCValAssign::LocInfo LocInfo,
                            ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::v1i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList1[] = {
      X86::AL, X86::DL, X86::CL
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList2[] = {
      X86::AX, X86::DX, X86::CX
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList3[] = {
      X86::EAX, X86::EDX, X86::ECX
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList4[] = {
      X86::RAX, X86::RDX, X86::RCX
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v2i1) {
    LocVT = MVT::v2i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v4i1) {
    LocVT = MVT::v4i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v8i1) {
    LocVT = MVT::v8i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::v16i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::v32i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::v64i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i8 ||
      LocVT == MVT::v8i16 ||
      LocVT == MVT::v4i32 ||
      LocVT == MVT::v2i64 ||
      LocVT == MVT::v8f16 ||
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    static const MCPhysReg RegList5[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList5)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v16f16 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    static const MCPhysReg RegList6[] = {
      X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v32f16 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    static const MCPhysReg RegList7[] = {
      X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList7)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    if (unsigned Reg = State.AllocateReg(X86::MM0)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
    if (LocVT == MVT::f80) {
      static const MCPhysReg RegList8[] = {
        X86::FP0, X86::FP1
      };
      if (unsigned Reg = State.AllocateReg(RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_32(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::Fast) {
    if (!RetCC_X86_32_Fast(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Tail) {
    if (!RetCC_X86_32_Fast(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HiPE) {
    if (!RetCC_X86_32_HiPE(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_VectorCall) {
    if (!RetCC_X86_32_VectorCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (!RetCC_X86_32_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_X86_32_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_32_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isInReg()) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
      if (LocVT == MVT::f32 ||
          LocVT == MVT::f64) {
        static const MCPhysReg RegList1[] = {
          X86::XMM0, X86::XMM1, X86::XMM2
        };
        if (unsigned Reg = State.AllocateReg(RegList1)) {
          State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
          return false;
        }
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasX87()) {
    if (LocVT == MVT::f32 ||
        LocVT == MVT::f64) {
      static const MCPhysReg RegList2[] = {
        X86::FP0, X86::FP1
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasX87()) {
    if (LocVT == MVT::f32) {
      static const MCPhysReg RegList3[] = {
        X86::EAX, X86::EDX, X86::ECX
      };
      if (unsigned Reg = State.AllocateReg(RegList3)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f16) {
    static const MCPhysReg RegList4[] = {
      X86::XMM0, X86::XMM1, X86::XMM2
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_32_Fast(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f32) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
      static const MCPhysReg RegList1[] = {
        X86::XMM0, X86::XMM1, X86::XMM2
      };
      if (unsigned Reg = State.AllocateReg(RegList1)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE2()) {
      static const MCPhysReg RegList2[] = {
        X86::XMM0, X86::XMM1, X86::XMM2
      };
      if (unsigned Reg = State.AllocateReg(RegList2)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList3[] = {
      X86::AL, X86::DL, X86::CL
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList4[] = {
      X86::AX, X86::DX, X86::CX
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList5[] = {
      X86::EAX, X86::EDX, X86::ECX
    };
    if (unsigned Reg = State.AllocateReg(RegList5)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_32_HiPE(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::ESI, X86::EBP, X86::EAX, X86::EDX
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_32_RegCall(unsigned ValNo, MVT ValVT,
                                 MVT LocVT, CCValAssign::LocInfo LocInfo,
                                 ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::v1i1 ||
      LocVT == MVT::v8i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList1[] = {
      X86::AL, X86::CL, X86::DL, X86::DIL, X86::SIL
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList2[] = {
      X86::AX, X86::CX, X86::DX, X86::DI, X86::SI
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList3[] = {
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    if (unsigned Reg = State.AllocateReg(X86::RAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::RAX)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f80) {
    static const MCPhysReg RegList4[] = {
      X86::FP0, X86::FP1
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList5[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
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
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList6[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList6)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList7[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList7)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList8[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7
      };
      if (unsigned Reg = State.AllocateReg(RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_32_VectorCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    static const MCPhysReg RegList1[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64(unsigned ValNo, MVT ValVT,
                         MVT LocVT, CCValAssign::LocInfo LocInfo,
                         ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (State.getCallingConv() == CallingConv::HiPE) {
    if (!RetCC_X86_64_HiPE(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::WebKit_JS) {
    if (!RetCC_X86_64_WebKit_JS(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::AnyReg) {
    if (!RetCC_X86_64_AnyReg(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Swift) {
    if (!RetCC_X86_64_Swift(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::SwiftTail) {
    if (!RetCC_X86_64_Swift(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::Win64) {
    if (!RetCC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_64_SysV) {
    if (!RetCC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_VectorCall) {
    if (!RetCC_X86_64_Vectorcall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::HHVM) {
    if (!RetCC_X86_64_HHVM(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
      if (!RetCC_X86_Win64_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (State.getCallingConv() == CallingConv::X86_RegCall) {
    if (!RetCC_X86_SysV64_RegCall(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).isTargetWin64()) {
    if (!RetCC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (!RetCC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64_AnyReg(unsigned ValNo, MVT ValVT,
                                MVT LocVT, CCValAssign::LocInfo LocInfo,
                                ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (CC_X86_AnyReg_Error(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64_C(unsigned ValNo, MVT ValVT,
                           MVT LocVT, CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f16) {
    static const MCPhysReg RegList1[] = {
      X86::XMM0, X86::XMM1
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList2[] = {
      X86::XMM0, X86::XMM1
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList3[] = {
      X86::XMM0, X86::XMM1
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128) {
    static const MCPhysReg RegList4[] = {
      X86::XMM0, X86::XMM1
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    static const MCPhysReg RegList5[] = {
      X86::XMM0, X86::XMM1
    };
    if (unsigned Reg = State.AllocateReg(RegList5)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (ArgFlags.isPointer()) {
    if (CC_X86_64_Pointer(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
      return false;
  }

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R12)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64_HHVM(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::RBX, X86::RBP, X86::RDI, X86::RSI, X86::RDX, X86::RCX, X86::R8, X86::R9, X86::RAX, X86::R10, X86::R11, X86::R13, X86::R14, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_64_HiPE(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    static const MCPhysReg RegList1[] = {
      X86::R15, X86::RBP, X86::RAX, X86::RDX
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_64_Swift(unsigned ValNo, MVT ValVT,
                               MVT LocVT, CCValAssign::LocInfo LocInfo,
                               ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (ArgFlags.isSwiftError()) {
    if (LocVT == MVT::i64) {
      if (unsigned Reg = State.AllocateReg(X86::R12)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v1i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList1[] = {
      X86::AL, X86::DL, X86::CL, X86::R8B
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList2[] = {
      X86::AX, X86::DX, X86::CX, X86::R8W
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList3[] = {
      X86::EAX, X86::EDX, X86::ECX, X86::R8D
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList4[] = {
      X86::RAX, X86::RDX, X86::RCX, X86::R8
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32) {
    static const MCPhysReg RegList5[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList5)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f64) {
    static const MCPhysReg RegList6[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f128) {
    static const MCPhysReg RegList7[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList7)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::x86mmx) {
    static const MCPhysReg RegList8[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList8)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86Common(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64_Vectorcall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    static const MCPhysReg RegList1[] = {
      X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (!RetCC_X86_Win64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_64_WebKit_JS(unsigned ValNo, MVT ValVT,
                                   MVT LocVT, CCValAssign::LocInfo LocInfo,
                                   ISD::ArgFlagsTy ArgFlags, CCState &State) {

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
    if (unsigned Reg = State.AllocateReg(X86::RAX)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_SysV64_RegCall(unsigned ValNo, MVT ValVT,
                                     MVT LocVT, CCValAssign::LocInfo LocInfo,
                                     ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::v1i1 ||
      LocVT == MVT::v8i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList1[] = {
      X86::AL, X86::CL, X86::DL, X86::DIL, X86::SIL, X86::R8B, X86::R9B, X86::R12B, X86::R13B, X86::R14B, X86::R15B
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList2[] = {
      X86::AX, X86::CX, X86::DX, X86::DI, X86::SI, X86::R8W, X86::R9W, X86::R12W, X86::R13W, X86::R14W, X86::R15W
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList3[] = {
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI, X86::R8D, X86::R9D, X86::R12D, X86::R13D, X86::R14D, X86::R15D
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList4[] = {
      X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R12, X86::R13, X86::R14, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      static const MCPhysReg RegList5[] = {
        X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R12, X86::R13, X86::R14, X86::R15
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f80) {
    static const MCPhysReg RegList6[] = {
      X86::FP0, X86::FP1
    };
    if (unsigned Reg = State.AllocateReg(RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList7[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
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
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList8[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList9[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList9)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList10[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}


static bool RetCC_X86_Win64_C(unsigned ValNo, MVT ValVT,
                              MVT LocVT, CCValAssign::LocInfo LocInfo,
                              ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::x86mmx) {
    LocVT = MVT::i64;
    LocInfo = CCValAssign::BCvt;
  }

  if (LocVT == MVT::f32) {
    if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      LocVT = MVT::i32;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (LocVT == MVT::f64) {
    if (!static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      LocVT = MVT::i64;
      LocInfo = CCValAssign::BCvt;
    }
  }

  if (!RetCC_X86_64_C(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
    return false;

  return true; // CC didn't match.
}


static bool RetCC_X86_Win64_RegCall(unsigned ValNo, MVT ValVT,
                                    MVT LocVT, CCValAssign::LocInfo LocInfo,
                                    ISD::ArgFlagsTy ArgFlags, CCState &State) {

  if (LocVT == MVT::i1 ||
      LocVT == MVT::v1i1 ||
      LocVT == MVT::v8i1) {
    LocVT = MVT::i8;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v16i1) {
    LocVT = MVT::i16;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::v32i1) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (LocVT == MVT::i8) {
    static const MCPhysReg RegList1[] = {
      X86::AL, X86::CL, X86::DL, X86::DIL, X86::SIL, X86::R8B, X86::R9B, X86::R10B, X86::R11B, X86::R12B, X86::R14B, X86::R15B
    };
    if (unsigned Reg = State.AllocateReg(RegList1)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i16) {
    static const MCPhysReg RegList2[] = {
      X86::AX, X86::CX, X86::DX, X86::DI, X86::SI, X86::R8W, X86::R9W, X86::R10W, X86::R11W, X86::R12W, X86::R14W, X86::R15W
    };
    if (unsigned Reg = State.AllocateReg(RegList2)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i32) {
    static const MCPhysReg RegList3[] = {
      X86::EAX, X86::ECX, X86::EDX, X86::EDI, X86::ESI, X86::R8D, X86::R9D, X86::R10D, X86::R11D, X86::R12D, X86::R14D, X86::R15D
    };
    if (unsigned Reg = State.AllocateReg(RegList3)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::i64) {
    static const MCPhysReg RegList4[] = {
      X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R10, X86::R11, X86::R12, X86::R14, X86::R15
    };
    if (unsigned Reg = State.AllocateReg(RegList4)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::v64i1) {
    LocVT = MVT::i64;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is64Bit()) {
    if (LocVT == MVT::i64) {
      static const MCPhysReg RegList5[] = {
        X86::RAX, X86::RCX, X86::RDX, X86::RDI, X86::RSI, X86::R8, X86::R9, X86::R10, X86::R11, X86::R12, X86::R14, X86::R15
      };
      if (unsigned Reg = State.AllocateReg(RegList5)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).is32Bit()) {
    if (LocVT == MVT::i64) {
      if (CC_X86_32_RegCall_Assign2Regs(ValNo, ValVT, LocVT, LocInfo, ArgFlags, State))
        return false;
    }
  }

  if (LocVT == MVT::f80) {
    static const MCPhysReg RegList6[] = {
      X86::FP0, X86::FP1
    };
    if (unsigned Reg = State.AllocateReg(RegList6)) {
      State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
      return false;
    }
  }

  if (LocVT == MVT::f32 ||
      LocVT == MVT::f64 ||
      LocVT == MVT::f128) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList7[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
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
      LocVT == MVT::v4f32 ||
      LocVT == MVT::v2f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasSSE1()) {
      static const MCPhysReg RegList8[] = {
        X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList8)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v32i8 ||
      LocVT == MVT::v16i16 ||
      LocVT == MVT::v8i32 ||
      LocVT == MVT::v4i64 ||
      LocVT == MVT::v8f32 ||
      LocVT == MVT::v4f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX()) {
      static const MCPhysReg RegList9[] = {
        X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList9)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  if (LocVT == MVT::v64i8 ||
      LocVT == MVT::v32i16 ||
      LocVT == MVT::v16i32 ||
      LocVT == MVT::v8i64 ||
      LocVT == MVT::v16f32 ||
      LocVT == MVT::v8f64) {
    if (static_cast<const X86Subtarget&>(State.getMachineFunction().getSubtarget()).hasAVX512()) {
      static const MCPhysReg RegList10[] = {
        X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15
      };
      if (unsigned Reg = State.AllocateReg(RegList10)) {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
        return false;
      }
    }
  }

  return true; // CC didn't match.
}

#else

const MCRegister CC_Intel_OCL_BI_ArgRegs[] = { X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::K1, X86::R8, X86::R8D, X86::R9, X86::R9D, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3 };
const MCRegister CC_X86_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_C_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDX };
const MCRegister CC_X86_32_Common_ArgRegs[] = { X86::MM0, X86::MM1, X86::MM2, X86::XMM0, X86::XMM1, X86::XMM2 };
const MCRegister CC_X86_32_FastCC_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDX, X86::XMM0, X86::XMM1, X86::XMM2 };
const MCRegister CC_X86_32_FastCall_ArgRegs[] = { X86::CL, X86::CX, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDX };
const MCRegister CC_X86_32_GHC_ArgRegs[] = { X86::EBP, X86::EBX, X86::EDI, X86::ESI };
const MCRegister CC_X86_32_HiPE_ArgRegs[] = { X86::EAX, X86::EBP, X86::ECX, X86::EDX, X86::ESI };
const MCRegister CC_X86_32_MCU_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_RegCall_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::RAX, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7 };
const MCRegister CC_X86_32_ThisCall_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_ThisCall_Common_ArgRegs[] = { X86::ECX };
const MCRegister CC_X86_32_ThisCall_Mingw_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_ThisCall_Win_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_Vector_Common_ArgRegs[] = { 0 };
const MCRegister CC_X86_32_Vector_Darwin_ArgRegs[] = { X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3 };
const MCRegister CC_X86_32_Vector_Standard_ArgRegs[] = { X86::XMM0, X86::XMM1, X86::XMM2, X86::YMM0, X86::YMM1, X86::YMM2, X86::ZMM0, X86::ZMM1, X86::ZMM2 };
const MCRegister CC_X86_64_ArgRegs[] = { 0 };
const MCRegister CC_X86_64_AnyReg_ArgRegs[] = { 0 };
const MCRegister CC_X86_64_C_ArgRegs[] = { X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::R10, X86::R10D, X86::R8, X86::R8D, X86::R9, X86::R9D, X86::RAX, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7 };
const MCRegister CC_X86_64_GHC_ArgRegs[] = { X86::R12, X86::R13, X86::R14, X86::R15, X86::R8, X86::R9, X86::RBP, X86::RBX, X86::RDI, X86::RSI, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6 };
const MCRegister CC_X86_64_HHVM_ArgRegs[] = { X86::R10, X86::R11, X86::R12, X86::R13, X86::R14, X86::R15, X86::R8, X86::R9, X86::RAX, X86::RBP, X86::RBX, X86::RCX, X86::RDI, X86::RDX, X86::RSI };
const MCRegister CC_X86_64_HHVM_C_ArgRegs[] = { X86::RBP };
const MCRegister CC_X86_64_HiPE_ArgRegs[] = { X86::R15, X86::R8, X86::RBP, X86::RCX, X86::RDX, X86::RSI };
const MCRegister CC_X86_64_WebKit_JS_ArgRegs[] = { X86::EAX, X86::RAX };
const MCRegister CC_X86_SysV64_RegCall_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::R12, X86::R12D, X86::R13, X86::R13D, X86::R14, X86::R14D, X86::R15, X86::R15D, X86::R8, X86::R8D, X86::R9, X86::R9D, X86::RAX, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::XMM0, X86::XMM1, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::YMM0, X86::YMM1, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::ZMM0, X86::ZMM1, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9 };
const MCRegister CC_X86_Win32_CFGuard_Check_ArgRegs[] = { X86::ECX };
const MCRegister CC_X86_Win32_Vector_ArgRegs[] = { 0 };
const MCRegister CC_X86_Win32_VectorCall_ArgRegs[] = { 0 };
const MCRegister CC_X86_Win64_C_ArgRegs[] = { X86::R10, X86::RAX };
const MCRegister CC_X86_Win64_RegCall_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::R10, X86::R10D, X86::R11, X86::R11D, X86::R12, X86::R12D, X86::R14, X86::R14D, X86::R15, X86::R15D, X86::R8, X86::R8D, X86::R9, X86::R9D, X86::RAX, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::XMM0, X86::XMM1, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::YMM0, X86::YMM1, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::ZMM0, X86::ZMM1, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9 };
const MCRegister CC_X86_Win64_VectorCall_ArgRegs[] = { 0 };
const MCRegister RetCC_Intel_OCL_BI_ArgRegs[] = { X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3 };
const MCRegister RetCC_X86_ArgRegs[] = { 0 };
const MCRegister RetCC_X86Common_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDX, X86::FP0, X86::FP1, X86::MM0, X86::RAX, X86::RCX, X86::RDX, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3 };
const MCRegister RetCC_X86_32_ArgRegs[] = { 0 };
const MCRegister RetCC_X86_32_C_ArgRegs[] = { X86::EAX, X86::ECX, X86::EDX, X86::FP0, X86::FP1, X86::XMM0, X86::XMM1, X86::XMM2 };
const MCRegister RetCC_X86_32_Fast_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDX, X86::XMM0, X86::XMM1, X86::XMM2 };
const MCRegister RetCC_X86_32_HiPE_ArgRegs[] = { X86::EAX, X86::EBP, X86::EDX, X86::ESI };
const MCRegister RetCC_X86_32_RegCall_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DI, X86::DIL, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::FP1, X86::RAX, X86::SI, X86::SIL, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::YMM0, X86::YMM1, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::ZMM0, X86::ZMM1, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7 };
const MCRegister RetCC_X86_32_VectorCall_ArgRegs[] = { X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3 };
const MCRegister RetCC_X86_64_ArgRegs[] = { 0 };
const MCRegister RetCC_X86_64_AnyReg_ArgRegs[] = { 0 };
const MCRegister RetCC_X86_64_C_ArgRegs[] = { X86::XMM0, X86::XMM1 };
const MCRegister RetCC_X86_64_HHVM_ArgRegs[] = { X86::R10, X86::R11, X86::R13, X86::R14, X86::R15, X86::R8, X86::R9, X86::RAX, X86::RBP, X86::RBX, X86::RCX, X86::RDI, X86::RDX, X86::RSI };
const MCRegister RetCC_X86_64_HiPE_ArgRegs[] = { X86::R15, X86::RAX, X86::RBP, X86::RDX };
const MCRegister RetCC_X86_64_Swift_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDX, X86::R8, X86::R8B, X86::R8D, X86::R8W, X86::RAX, X86::RCX, X86::RDX, X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3 };
const MCRegister RetCC_X86_64_Vectorcall_ArgRegs[] = { X86::XMM0, X86::XMM1, X86::XMM2, X86::XMM3 };
const MCRegister RetCC_X86_64_WebKit_JS_ArgRegs[] = { X86::RAX };
const MCRegister RetCC_X86_SysV64_RegCall_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DI, X86::DIL, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::FP1, X86::R12, X86::R12B, X86::R12D, X86::R12W, X86::R13, X86::R13B, X86::R13D, X86::R13W, X86::R14, X86::R14B, X86::R14D, X86::R14W, X86::R15, X86::R15B, X86::R15D, X86::R15W, X86::R8, X86::R8B, X86::R8D, X86::R8W, X86::R9, X86::R9B, X86::R9D, X86::R9W, X86::RAX, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::SI, X86::SIL, X86::XMM0, X86::XMM1, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::YMM0, X86::YMM1, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::ZMM0, X86::ZMM1, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9 };
const MCRegister RetCC_X86_Win64_C_ArgRegs[] = { 0 };
const MCRegister RetCC_X86_Win64_RegCall_ArgRegs[] = { X86::AL, X86::AX, X86::CL, X86::CX, X86::DI, X86::DIL, X86::DL, X86::DX, X86::EAX, X86::ECX, X86::EDI, X86::EDX, X86::ESI, X86::FP0, X86::FP1, X86::R10, X86::R10B, X86::R10D, X86::R10W, X86::R11, X86::R11B, X86::R11D, X86::R11W, X86::R12, X86::R12B, X86::R12D, X86::R12W, X86::R14, X86::R14B, X86::R14D, X86::R14W, X86::R15, X86::R15B, X86::R15D, X86::R15W, X86::R8, X86::R8B, X86::R8D, X86::R8W, X86::R9, X86::R9B, X86::R9D, X86::R9W, X86::RAX, X86::RCX, X86::RDI, X86::RDX, X86::RSI, X86::SI, X86::SIL, X86::XMM0, X86::XMM1, X86::XMM10, X86::XMM11, X86::XMM12, X86::XMM13, X86::XMM14, X86::XMM15, X86::XMM2, X86::XMM3, X86::XMM4, X86::XMM5, X86::XMM6, X86::XMM7, X86::XMM8, X86::XMM9, X86::YMM0, X86::YMM1, X86::YMM10, X86::YMM11, X86::YMM12, X86::YMM13, X86::YMM14, X86::YMM15, X86::YMM2, X86::YMM3, X86::YMM4, X86::YMM5, X86::YMM6, X86::YMM7, X86::YMM8, X86::YMM9, X86::ZMM0, X86::ZMM1, X86::ZMM10, X86::ZMM11, X86::ZMM12, X86::ZMM13, X86::ZMM14, X86::ZMM15, X86::ZMM2, X86::ZMM3, X86::ZMM4, X86::ZMM5, X86::ZMM6, X86::ZMM7, X86::ZMM8, X86::ZMM9 };

// Registers used by Swift.
const MCRegister CC_X86_64_C_Swift_ArgRegs[] = { X86::R12, X86::R13, X86::R14 };
const MCRegister CC_X86_Win64_C_Swift_ArgRegs[] = { X86::R12, X86::R13, X86::R14 };
const MCRegister RetCC_X86_64_C_Swift_ArgRegs[] = { X86::R12 };
const MCRegister RetCC_X86_64_Swift_Swift_ArgRegs[] = { X86::R12 };

#endif // CC_REGISTER_LIST
