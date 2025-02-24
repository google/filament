/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* "Fast" Instruction Selector for the X86 target                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


// FastEmit Immediate Predicate functions.
static bool Predicate_i64immSExt32(int64_t Imm) {
 return isInt<32>(Imm); 
}
static bool Predicate_AndMask64(int64_t Imm) {

  return isMask_64(Imm) && !isUInt<32>(Imm);

}
static bool Predicate_BTRMask64(int64_t Imm) {

  return !isUInt<32>(Imm) && !isInt<32>(Imm) && isPowerOf2_64(~Imm);

}
static bool Predicate_BTCBTSMask64(int64_t Imm) {

  return !isInt<32>(Imm) && isPowerOf2_64(Imm);

}
static bool Predicate_i16immSExt8(int64_t Imm) {
 return isInt<8>(Imm); 
}
static bool Predicate_i32immSExt8(int64_t Imm) {
 return isInt<8>(Imm); 
}
static bool Predicate_i64immSExt8(int64_t Imm) {
 return isInt<8>(Imm); 
}


// FastEmit functions for ISD::ABS.

unsigned fastEmit_ISD_ABS_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PABSBrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSBrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v32i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSBYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v64i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSBZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PABSWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSWYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v32i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPABSWZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PABSDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPABSDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPABSQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPABSQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ABS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ABS_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_ISD_ABS_MVT_v32i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v64i8: return fastEmit_ISD_ABS_MVT_v64i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_ABS_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_ABS_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_ISD_ABS_MVT_v32i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_ABS_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_ABS_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_ABS_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_ABS_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_ABS_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_ABS_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ANY_EXTEND.

unsigned fastEmit_ISD_ANY_EXTEND_MVT_i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(X86::MOVZX32rr8, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v2i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2QZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v4i1_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2DZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v4i1_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2QZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v4i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_ANY_EXTEND_MVT_v4i1_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_ANY_EXTEND_MVT_v4i1_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2WZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2DZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VPMOVM2QZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v8i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_ANY_EXTEND_MVT_v8i1_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2BZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2WZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VPMOVM2DZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v16i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i16_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_ANY_EXTEND_MVT_v16i1_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v32i1_MVT_v32i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2BZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v32i1_MVT_v32i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVM2WZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v32i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v32i8: return fastEmit_ISD_ANY_EXTEND_MVT_v32i1_MVT_v32i8_r(Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_ISD_ANY_EXTEND_MVT_v32i1_MVT_v32i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ANY_EXTEND_MVT_v64i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVM2BZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ANY_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ANY_EXTEND_MVT_i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i1: return fastEmit_ISD_ANY_EXTEND_MVT_v2i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i1: return fastEmit_ISD_ANY_EXTEND_MVT_v4i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i1: return fastEmit_ISD_ANY_EXTEND_MVT_v8i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i1: return fastEmit_ISD_ANY_EXTEND_MVT_v16i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i1: return fastEmit_ISD_ANY_EXTEND_MVT_v32i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v64i1: return fastEmit_ISD_ANY_EXTEND_MVT_v64i1_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITCAST.

unsigned fastEmit_ISD_BITCAST_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVDI2SSZrr, &X86::FR32XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVDI2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVDI2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_i64_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOV64toSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOV64toSDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOV64toSDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_i64_MVT_x86mmx_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMMX())) {
    return fastEmitInst_r(X86::MMX_MOVD64to64rr, &X86::VR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_i64_MVT_f64_r(Op0, Op0IsKill);
  case MVT::x86mmx: return fastEmit_ISD_BITCAST_MVT_i64_MVT_x86mmx_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSS2DIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSS2DIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSS2DIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSDto64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSDto64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSDto64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_MVT_x86mmx_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMMX()) && (Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::MMX_MOVFR642Qrr, &X86::VR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_BITCAST_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  case MVT::x86mmx: return fastEmit_ISD_BITCAST_MVT_f64_MVT_x86mmx_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_MVT_x86mmx_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMMX())) {
    return fastEmitInst_r(X86::MMX_MOVD64from64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_x86mmx_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMMX()) && (Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::MMX_MOVQ2FR64rr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_x86mmx_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_BITCAST_MVT_x86mmx_MVT_i64_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_x86mmx_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_BITCAST_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BITCAST_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_BITCAST_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::f32: return fastEmit_ISD_BITCAST_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::x86mmx: return fastEmit_ISD_BITCAST_MVT_x86mmx_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BRIND.

unsigned fastEmit_ISD_BRIND_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP16r, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP32r, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP64r, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_BRIND_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_BRIND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_BRIND_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BSWAP.

unsigned fastEmit_ISD_BSWAP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(X86::BSWAP32r, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_BSWAP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(X86::BSWAP64r, &X86::GR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_BSWAP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BSWAP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_BSWAP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTLZ.

unsigned fastEmit_ISD_CTLZ_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  if ((Subtarget->hasLZCNT())) {
    return fastEmitInst_r(X86::LZCNT16rr, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasLZCNT())) {
    return fastEmitInst_r(X86::LZCNT32rr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasLZCNT())) {
    return fastEmitInst_r(X86::LZCNT64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPLZCNTDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPLZCNTDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPLZCNTDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPLZCNTQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPLZCNTQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPLZCNTQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_CTLZ_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_CTLZ_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTLZ_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_CTLZ_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_CTLZ_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_CTLZ_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_CTLZ_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_CTLZ_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_CTLZ_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTPOP.

unsigned fastEmit_ISD_CTPOP_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  if ((Subtarget->hasPOPCNT())) {
    return fastEmitInst_r(X86::POPCNT16rr, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasPOPCNT())) {
    return fastEmitInst_r(X86::POPCNT32rr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasPOPCNT())) {
    return fastEmitInst_r(X86::POPCNT64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPOPCNTBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v32i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPOPCNTBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v64i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBITALG())) {
    return fastEmitInst_r(X86::VPOPCNTBZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPOPCNTWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPOPCNTWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v32i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBITALG())) {
    return fastEmitInst_r(X86::VPOPCNTWZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasVLX()) && (Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasVLX()) && (Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasVLX()) && (Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasVLX()) && (Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasVPOPCNTDQ())) {
    return fastEmitInst_r(X86::VPOPCNTQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_CTPOP_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_CTPOP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTPOP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_CTPOP_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_ISD_CTPOP_MVT_v32i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v64i8: return fastEmit_ISD_CTPOP_MVT_v64i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_CTPOP_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_CTPOP_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_ISD_CTPOP_MVT_v32i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_CTPOP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_CTPOP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_CTPOP_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_CTPOP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_CTPOP_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_CTPOP_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTTZ.

unsigned fastEmit_ISD_CTTZ_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  if ((Subtarget->hasBMI())) {
    return fastEmitInst_r(X86::TZCNT16rr, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBMI())) {
    return fastEmitInst_r(X86::TZCNT32rr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasBMI())) {
    return fastEmitInst_r(X86::TZCNT64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_CTTZ_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_CTTZ_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTTZ_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTTZ_ZERO_UNDEF.

unsigned fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_r(X86::BSF16rr, &X86::GR16RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(X86::BSF32rr, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(X86::BSF64rr, &X86::GR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTTZ_ZERO_UNDEF_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTTZ_ZERO_UNDEF_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FABS.

unsigned fastEmit_ISD_FABS_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_r(X86::ABS_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::ABS_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f80_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_r(X86::ABS_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_FABS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FABS_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FABS_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f80: return fastEmit_ISD_FABS_MVT_f80_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEG.

unsigned fastEmit_ISD_FNEG_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_r(X86::CHS_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::CHS_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f80_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_r(X86::CHS_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_FNEG_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FNEG_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FNEG_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f80: return fastEmit_ISD_FNEG_MVT_f80_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_EXTEND.

unsigned fastEmit_ISD_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSS2SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPS2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FP_EXTEND_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_ISD_FP_EXTEND_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_ROUND.

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSD2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FP_ROUND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_SINT.

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_FP_TO_SINT_MVT_f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_FP_TO_SINT_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_TO_SINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_TO_SINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FP_TO_SINT_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_UINT.

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_FP_TO_UINT_MVT_f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_FP_TO_UINT_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_TO_UINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_TO_UINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSQRT.

unsigned fastEmit_ISD_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTSSr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_r(X86::SQRT_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTSDr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::SQRT_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f80_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_r(X86::SQRT_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VSQRTPSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTPDr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VSQRTPDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSQRT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FSQRT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f80: return fastEmit_ISD_FSQRT_MVT_f80_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSQRT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_ISD_FSQRT_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_ISD_FSQRT_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSQRT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FSQRT_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_FSQRT_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SCALAR_TO_VECTOR.

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVDI2PDIZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVDI2PDIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVDI2PDIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_MVT_x86mmx_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMMX())) {
    return fastEmitInst_r(X86::MMX_MOVD64rr, &X86::VR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::x86mmx: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_MVT_x86mmx_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOV64toPQIZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOV64toPQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOV64toPQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SIGN_EXTEND.

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i8_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_r(X86::MOVSX32rr8, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i8_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_r(X86::MOVSX64rr8, &X86::GR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SIGN_EXTEND_MVT_i8_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_SIGN_EXTEND_MVT_i8_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i16_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_r(X86::MOVSX32rr16, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i16_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_r(X86::MOVSX64rr16, &X86::GR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SIGN_EXTEND_MVT_i16_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_SIGN_EXTEND_MVT_i16_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::MOVSX64rr32, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v2i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2QZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2DZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2QZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2WZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2DZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VPMOVM2QZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2BZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2WZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VPMOVM2DZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i16_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_MVT_v32i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVM2BZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_MVT_v32i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVM2WZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v32i8: return fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_MVT_v32i8_r(Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_MVT_v32i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v64i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVM2BZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVSXBWYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSXBDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_MVT_v16i16_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v32i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVSXBWZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSXWQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSXWDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXDQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSXDQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SIGN_EXTEND_MVT_i8_r(RetVT, Op0, Op0IsKill);
  case MVT::i16: return fastEmit_ISD_SIGN_EXTEND_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v2i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v4i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v32i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v64i1: return fastEmit_ISD_SIGN_EXTEND_MVT_v64i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_ISD_SIGN_EXTEND_MVT_v32i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_SIGN_EXTEND_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_SIGN_EXTEND_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SIGN_EXTEND_VECTOR_INREG.

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXBWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVSXBWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXBDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXBQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXBQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSXBQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i64_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXWDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXWQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXWQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVSXDQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXDQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSXDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SINT_TO_FP.

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI2SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI642SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI642SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_SINT_TO_FP_MVT_i64_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTDQ2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i32_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i32_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_SINT_TO_FP_MVT_v8i32_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_SINT_TO_FP_MVT_v8i32_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i64_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i64_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i64_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_SINT_TO_FP_MVT_v4i64_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i64_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i64_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_SINT_TO_FP_MVT_v8i64_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_SINT_TO_FP_MVT_v8i64_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_SINT_TO_FP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_SINT_TO_FP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_SINT_TO_FP_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_SINT_TO_FP_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_SINT_TO_FP_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_EXTEND.

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSS2SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPS2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FP_EXTEND_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_ROUND.

unsigned fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSD2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_STRICT_FP_ROUND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_TO_SINT.

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64rr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FP_TO_SINT_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FP_TO_UINT.

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USIZrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USI64Zrr, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FP_TO_UINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FSQRT.

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTSSr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_r(X86::SQRT_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTSDr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_r(X86::SQRT_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_f80_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_r(X86::SQRT_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPSYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VSQRTPSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::SQRTPDr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VSQRTPDYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VSQRTPDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FSQRT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FSQRT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f80: return fastEmit_ISD_STRICT_FSQRT_MVT_f80_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FSQRT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FSQRT_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_ISD_STRICT_FSQRT_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FSQRT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FSQRT_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_FSQRT_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_SINT_TO_FP.

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI2SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI2SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI642SSrr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSI642SDrr, &X86::FR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTDQ2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PSYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTDQ2PSZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTQQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_STRICT_SINT_TO_FP_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_UINT_TO_FP.

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_STRICT_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_STRICT_UINT_TO_FP_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::TRUNCATE.

unsigned fastEmit_ISD_TRUNCATE_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_extractsubreg(RetVT, Op0, Op0IsKill, X86::sub_8bit);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i32_MVT_i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_extractsubreg(MVT::i8, Op0, Op0IsKill, X86::sub_8bit);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i32_MVT_i16_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_extractsubreg(MVT::i16, Op0, Op0IsKill, X86::sub_16bit);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_TRUNCATE_MVT_i32_MVT_i8_r(Op0, Op0IsKill);
  case MVT::i16: return fastEmit_ISD_TRUNCATE_MVT_i32_MVT_i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i8_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_extractsubreg(MVT::i8, Op0, Op0IsKill, X86::sub_8bit);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i16_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_extractsubreg(MVT::i16, Op0, Op0IsKill, X86::sub_16bit);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_extractsubreg(MVT::i32, Op0, Op0IsKill, X86::sub_32bit);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i8_r(Op0, Op0IsKill);
  case MVT::i16: return fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i16_r(Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_TRUNCATE_MVT_i64_MVT_i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVWBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v32i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVWBZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVDWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v16i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVDBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v16i32_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVDWZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_TRUNCATE_MVT_v16i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_TRUNCATE_MVT_v16i32_MVT_v16i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQDZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVQWZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVQDZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_TRUNCATE_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_TRUNCATE_MVT_v8i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_TRUNCATE_MVT_v8i64_MVT_v8i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_TRUNCATE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_TRUNCATE_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_ISD_TRUNCATE_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_TRUNCATE_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_TRUNCATE_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_ISD_TRUNCATE_MVT_v32i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_TRUNCATE_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_TRUNCATE_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_TRUNCATE_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_TRUNCATE_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UINT_TO_FP.

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i32_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i32_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_UINT_TO_FP_MVT_v8i32_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_UINT_TO_FP_MVT_v8i32_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTUDQ2PSZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i64_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i64_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i64_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_UINT_TO_FP_MVT_v4i64_MVT_v4f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i64_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i64_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTUQQ2PDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8f32: return fastEmit_ISD_UINT_TO_FP_MVT_v8i64_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_ISD_UINT_TO_FP_MVT_v8i64_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_UINT_TO_FP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_UINT_TO_FP_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_UINT_TO_FP_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_UINT_TO_FP_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ZERO_EXTEND.

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(X86::MOVZX32rr8, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(X86::MOVZX32rr16, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVZXBWYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVZXBDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i16: return fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_MVT_v16i16_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v32i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVZXBWZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVZXWQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVZXWDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXDQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVZXDQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ZERO_EXTEND_MVT_i8_r(RetVT, Op0, Op0IsKill);
  case MVT::i16: return fastEmit_ISD_ZERO_EXTEND_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_ZERO_EXTEND_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_ISD_ZERO_EXTEND_MVT_v32i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_ZERO_EXTEND_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_ISD_ZERO_EXTEND_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_ZERO_EXTEND_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_ZERO_EXTEND_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ZERO_EXTEND_VECTOR_INREG.

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXBWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVZXBWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXBDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXBQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXBQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVZXBQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v4i64_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXWDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXWQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXWQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVZXDQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXDQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVZXDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CALL.

unsigned fastEmit_X86ISD_CALL_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::CALL16r, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CALL_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit()) && (Subtarget->useRetpolineIndirectCalls())) {
    return fastEmitInst_r(X86::RETPOLINE_CALL32, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->is64Bit()) && (!Subtarget->useRetpolineIndirectCalls())) {
    return fastEmitInst_r(X86::CALL32r, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CALL_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->is64Bit()) && (Subtarget->useRetpolineIndirectCalls())) {
    return fastEmitInst_r(X86::RETPOLINE_CALL64, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->is64Bit()) && (!Subtarget->useRetpolineIndirectCalls())) {
    return fastEmitInst_r(X86::CALL64r, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CALL_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_X86ISD_CALL_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_X86ISD_CALL_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CALL_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CONFLICT.

unsigned fastEmit_X86ISD_CONFLICT_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPCONFLICTDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPCONFLICTDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPCONFLICTDZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPCONFLICTQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPCONFLICTQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPCONFLICTQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CONFLICT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CONFLICT_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_CONFLICT_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_CONFLICT_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CONFLICT_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CONFLICT_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CONFLICT_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTNEPS2BF16.

unsigned fastEmit_X86ISD_CVTNEPS2BF16_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBF16()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTNEPS2BF16Z128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNEPS2BF16_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBF16()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTNEPS2BF16Z256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNEPS2BF16_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBF16())) {
    return fastEmitInst_r(X86::VCVTNEPS2BF16Zrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNEPS2BF16_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTNEPS2BF16_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTNEPS2BF16_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTNEPS2BF16_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTP2SI.

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTP2SI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2DQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2DQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTPS2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTP2SI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTP2SI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPS2DQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2SI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTP2SI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2DQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2DQYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2SI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTP2SI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPD2DQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTPD2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2SI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTP2SI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTP2SI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2SI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTP2SI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTP2SI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTP2SI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTP2SI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_CVTP2SI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTP2SI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTP2UI.

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTP2UI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2UDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTPS2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTP2UI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTP2UI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPS2UDQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2UI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTP2UI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2UDQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTP2UI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTP2UI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPD2UDQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTPD2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTP2UI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTP2UI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTP2UI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTP2UI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTP2UI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTP2UI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTP2UI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTP2UI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_CVTP2UI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTP2UI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTPH2PS.

unsigned fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPH2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasF16C()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPH2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPH2PSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasF16C()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPH2PSYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_MVT_v8f32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTPH2PS_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPH2PSZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTPH2PS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_CVTPH2PS_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_CVTPH2PS_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTPH2PS_SAE.

unsigned fastEmit_X86ISD_CVTPH2PS_SAE_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPH2PSZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTPH2PS_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i16: return fastEmit_X86ISD_CVTPH2PS_SAE_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTS2SI.

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2SIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSS2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2SI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSS2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTS2SI_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTS2SI_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2SIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSD2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2SI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTSD2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2SI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTS2SI_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTS2SI_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTS2SI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTS2SI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTS2SI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTS2UI.

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2USIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSS2USI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTS2UI_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTS2UI_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2USIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTSD2USI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTS2UI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTS2UI_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTS2UI_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTS2UI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTS2UI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTS2UI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTSI2P.

unsigned fastEmit_X86ISD_CVTSI2P_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTDQ2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTSI2P_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTSI2P_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTSI2P_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTSI2P_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTP2SI.

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTTP2SI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTP2SI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_CVTTP2SI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTTP2SI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTP2SI_SAE.

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZrrb, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2SI_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8f32: return fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTTP2SI_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTP2UI.

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTTP2UI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTP2UI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_CVTTP2UI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTTP2UI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTP2UI_SAE.

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZrrb, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTP2UI_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8f32: return fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_CVTTP2UI_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTS2SI.

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSS2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTSD2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64rr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2SI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTS2SI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTS2SI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTS2SI_SAE.

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SIZrrb_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2SI64Zrrb_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SIZrrb_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2SI64Zrrb_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2SI_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTS2SI_SAE_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTS2UI.

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USIZrr_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USI64Zrr_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2UI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTS2UI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTS2UI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTTS2UI_SAE.

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USIZrrb_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSS2USI64Zrrb_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USIZrrb_Int, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTSD2USI64Zrrb_Int, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_CVTTS2UI_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_CVTTS2UI_SAE_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTUI2P.

unsigned fastEmit_X86ISD_CVTUI2P_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTUI2P_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTUI2P_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_CVTUI2P_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_CVTUI2P_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::EH_RETURN.

unsigned fastEmit_X86ISD_EH_RETURN_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(X86::EH_RETURN, &X86::GR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_X86ISD_EH_RETURN_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(X86::EH_RETURN64, &X86::GR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_X86ISD_EH_RETURN_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_EH_RETURN_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_EH_RETURN_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::EXP2.

unsigned fastEmit_X86ISD_EXP2_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VEXP2PSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_EXP2_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VEXP2PDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_EXP2_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_EXP2_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_EXP2_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::EXP2_SAE.

unsigned fastEmit_X86ISD_EXP2_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VEXP2PSZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_EXP2_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VEXP2PDZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_EXP2_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_EXP2_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_EXP2_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FGETEXP.

unsigned fastEmit_X86ISD_FGETEXP_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VGETEXPPSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VGETEXPPSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VGETEXPPSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VGETEXPPDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VGETEXPPDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VGETEXPPDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FGETEXP_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FGETEXP_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_FGETEXP_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FGETEXP_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FGETEXP_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FGETEXP_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FGETEXP_SAE.

unsigned fastEmit_X86ISD_FGETEXP_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VGETEXPPSZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VGETEXPPDZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXP_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_FGETEXP_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FGETEXP_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FRCP.

unsigned fastEmit_X86ISD_FRCP_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::RCPSSr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRCP_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::RCPPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VRCPPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRCP_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VRCPPSYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRCP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FRCP_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FRCP_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FRCP_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FRSQRT.

unsigned fastEmit_X86ISD_FRSQRT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::RSQRTSSr, &X86::FR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::RSQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VRSQRTPSr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRSQRT_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VRSQRTPSYr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FRSQRT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FRSQRT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FRSQRT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FRSQRT_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MMX_MOVD2W.

unsigned fastEmit_X86ISD_MMX_MOVD2W_MVT_x86mmx_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMMX())) {
    return fastEmitInst_r(X86::MMX_MOVD64grr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MMX_MOVD2W_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::x86mmx: return fastEmit_X86ISD_MMX_MOVD2W_MVT_x86mmx_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MMX_MOVW2D.

unsigned fastEmit_X86ISD_MMX_MOVW2D_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::x86mmx)
    return 0;
  if ((Subtarget->hasMMX())) {
    return fastEmitInst_r(X86::MMX_MOVD64rr, &X86::VR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MMX_MOVW2D_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_MMX_MOVW2D_MVT_i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVDDUP.

unsigned fastEmit_X86ISD_MOVDDUP_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVDDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVDDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVDDUP_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVDDUPZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVDDUPYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVDDUP_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVDDUPZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVDDUP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2f64: return fastEmit_X86ISD_MOVDDUP_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_MOVDDUP_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_MOVDDUP_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVDQ2Q.

unsigned fastEmit_X86ISD_MOVDQ2Q_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::x86mmx)
    return 0;
  return fastEmitInst_r(X86::MMX_MOVDQ2Qrr, &X86::VR64RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_X86ISD_MOVDQ2Q_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_MOVDQ2Q_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVMSK.

unsigned fastEmit_X86ISD_MOVMSK_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PMOVMSKBrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VPMOVMSKBrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v32i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_r(X86::VPMOVMSKBYrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVMSKPSrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPSrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPSYrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVMSKPDrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPDrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPDYrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVMSKPSrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPSrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPSYrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVMSKPDrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPDrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VMOVMSKPDYrr, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVMSK_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_MOVMSK_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_MOVMSK_MVT_v32i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_MOVMSK_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_MOVMSK_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_MOVMSK_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_MOVMSK_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_MOVMSK_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_MOVMSK_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_MOVMSK_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_MOVMSK_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVQ2DQ.

unsigned fastEmit_X86ISD_MOVQ2DQ_MVT_x86mmx_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  return fastEmitInst_r(X86::MMX_MOVQ2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_X86ISD_MOVQ2DQ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::x86mmx: return fastEmit_X86ISD_MOVQ2DQ_MVT_x86mmx_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVSHDUP.

unsigned fastEmit_X86ISD_MOVSHDUP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSHDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSHDUP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSHDUP_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSHDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSHDUP_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSHDUPYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSHDUP_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSHDUPZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSHDUP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_MOVSHDUP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_MOVSHDUP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_MOVSHDUP_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_MOVSHDUP_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_MOVSHDUP_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVSLDUP.

unsigned fastEmit_X86ISD_MOVSLDUP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSLDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSLDUP_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSLDUP_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVSLDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSLDUP_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVSLDUPYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSLDUP_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVSLDUPZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSLDUP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_MOVSLDUP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_MOVSLDUP_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_MOVSLDUP_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_MOVSLDUP_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_MOVSLDUP_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::NT_BRIND.

unsigned fastEmit_X86ISD_NT_BRIND_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP16r_NT, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_BRIND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP32r_NT, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_BRIND_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::JMP64r_NT, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_BRIND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_X86ISD_NT_BRIND_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_X86ISD_NT_BRIND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_NT_BRIND_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::NT_CALL.

unsigned fastEmit_X86ISD_NT_CALL_MVT_i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::CALL16r_NT, &X86::GR16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_CALL_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::CALL32r_NT, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_CALL_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::CALL64r_NT, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_NT_CALL_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_X86ISD_NT_CALL_MVT_i16_r(RetVT, Op0, Op0IsKill);
  case MVT::i32: return fastEmit_X86ISD_NT_CALL_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_NT_CALL_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PHMINPOS.

unsigned fastEmit_X86ISD_PHMINPOS_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::PHMINPOSUWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::VPHMINPOSUWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PHMINPOS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_PHMINPOS_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP14.

unsigned fastEmit_X86ISD_RCP14_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRCP14PSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRCP14PSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VRCP14PSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRCP14PDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRCP14PDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VRCP14PDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RCP14_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_RCP14_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_RCP14_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RCP14_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_RCP14_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RCP14_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP28.

unsigned fastEmit_X86ISD_RCP28_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRCP28PSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRCP28PDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_RCP28_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RCP28_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP28_SAE.

unsigned fastEmit_X86ISD_RCP28_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRCP28PSZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRCP28PDZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_RCP28_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RCP28_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT14.

unsigned fastEmit_X86ISD_RSQRT14_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRSQRT14PSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRSQRT14PSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VRSQRT14PSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRSQRT14PDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VRSQRT14PDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VRSQRT14PDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RSQRT14_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_RSQRT14_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_RSQRT14_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RSQRT14_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_RSQRT14_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RSQRT14_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT28.

unsigned fastEmit_X86ISD_RSQRT28_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRSQRT28PSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRSQRT28PDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_RSQRT28_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RSQRT28_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT28_SAE.

unsigned fastEmit_X86ISD_RSQRT28_SAE_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRSQRT28PSZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28_SAE_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_r(X86::VRSQRT28PDZrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_RSQRT28_SAE_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_RSQRT28_SAE_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::SEG_ALLOCA.

unsigned fastEmit_X86ISD_SEG_ALLOCA_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((!Subtarget->isTarget64BitLP64())) {
    return fastEmitInst_r(X86::SEG_ALLOCA_32, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SEG_ALLOCA_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::SEG_ALLOCA_64, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SEG_ALLOCA_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_SEG_ALLOCA_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_SEG_ALLOCA_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_CVTSI2P.

unsigned fastEmit_X86ISD_STRICT_CVTSI2P_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTDQ2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTDQ2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTSI2P_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTQQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTSI2P_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTSI2P_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTSI2P_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_CVTTP2SI.

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2DQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2DQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2DQYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2DQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2QQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2SI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_STRICT_CVTTP2SI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_CVTTP2UI.

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPS2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v16f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPS2UDQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_MVT_v4i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTTPD2UDQZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_r(X86::VCVTTPD2UQQZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_STRICT_CVTTP2UI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v16f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_STRICT_CVTTP2UI_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_CVTUI2P.

unsigned fastEmit_X86ISD_STRICT_CVTUI2P_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUDQ2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTUI2P_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTUQQ2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_CVTUI2P_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_STRICT_CVTUI2P_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_STRICT_CVTUI2P_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_VFPEXT.

unsigned fastEmit_X86ISD_STRICT_VFPEXT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPS2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_VFPEXT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_STRICT_VFPEXT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_VFPROUND.

unsigned fastEmit_X86ISD_STRICT_VFPROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPD2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_VFPROUND_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_VFPROUND_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPD2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_VFPROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2f64: return fastEmit_X86ISD_STRICT_VFPROUND_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_STRICT_VFPROUND_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_STRICT_VFPROUND_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VBROADCAST.

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDrZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDrZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPBROADCASTDrZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VBROADCAST_MVT_i32_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQrZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQrZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPBROADCASTQrZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v4i64_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VBROADCAST_MVT_i64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTBZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTBrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v32i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTBZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTBYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v64i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTBZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v32i8_r(Op0, Op0IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_VBROADCAST_MVT_v16i8_MVT_v64i8_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTWZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTWrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTWZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTWYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v32i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPBROADCASTWZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v16i16_r(Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VBROADCAST_MVT_v8i16_MVT_v32i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPBROADCASTDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VBROADCAST_MVT_v4i32_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTQYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPBROADCASTQZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v4i64_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VBROADCAST_MVT_v2i64_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v4f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSSZ128r, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v8f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSSZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSSYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v16f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VBROADCASTSSZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v4f32_r(Op0, Op0IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v8f32_r(Op0, Op0IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_VBROADCAST_MVT_v4f32_MVT_v16f32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v2f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVDDUPrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VMOVDDUPZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v4f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSDZ256r, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VBROADCASTSDYrr, &X86::VR256RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v8f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VBROADCASTSDZr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCAST_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v2f64: return fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v2f64_r(Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v4f64_r(Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_VBROADCAST_MVT_v2f64_MVT_v8f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCAST_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_VBROADCAST_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_VBROADCAST_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_X86ISD_VBROADCAST_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VBROADCAST_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VBROADCAST_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VBROADCAST_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_VBROADCAST_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_VBROADCAST_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VBROADCASTM.

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTMB2QZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v4i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTMB2QZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v8i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPBROADCASTMB2QZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v2i64_r(Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v4i64_r(Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_MVT_v8i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTMW2DZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPBROADCASTMW2DZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v16i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasCDI())) {
    return fastEmitInst_r(X86::VPBROADCASTMW2DZrr, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v8i32_r(Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_MVT_v16i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VBROADCASTM_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i1: return fastEmit_X86ISD_VBROADCASTM_MVT_v8i1_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i1: return fastEmit_X86ISD_VBROADCASTM_MVT_v16i1_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VFPEXT.

unsigned fastEmit_X86ISD_VFPEXT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPS2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPS2PDrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VFPEXT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_VFPEXT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VFPEXT_SAE.

unsigned fastEmit_X86ISD_VFPEXT_SAE_MVT_v8f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPS2PDZrrb, &X86::VR512RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VFPEXT_SAE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8f32: return fastEmit_X86ISD_VFPEXT_SAE_MVT_v8f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VFPROUND.

unsigned fastEmit_X86ISD_VFPROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::CVTPD2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VFPROUND_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VCVTPD2PSYrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VFPROUND_MVT_v8f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VCVTPD2PSZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VFPROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2f64: return fastEmit_X86ISD_VFPROUND_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_VFPROUND_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_VFPROUND_MVT_v8f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VTRUNC.

unsigned fastEmit_X86ISD_VTRUNC_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVWBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVDBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i32_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNC_MVT_v4i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNC_MVT_v4i32_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVDBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNC_MVT_v2i64_MVT_v4i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVQWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNC_MVT_v4i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNC_MVT_v4i64_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNC_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVQBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNC_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNC_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNC_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VTRUNC_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VTRUNC_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VTRUNC_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VTRUNC_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VTRUNCS.

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSWBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSWBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v32i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVSWBZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSDBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i32_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v4i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v4i32_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSDBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i32_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSDWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v8i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v8i32_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v16i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSDBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v16i32_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSDWZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v16i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VTRUNCS_MVT_v16i32_MVT_v16i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCS_MVT_v2i64_MVT_v4i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVSQDZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCS_MVT_v4i64_MVT_v4i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSQBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSQWZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVSQDZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCS_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VTRUNCS_MVT_v8i64_MVT_v8i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCS_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VTRUNCS_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VTRUNCS_MVT_v32i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCS_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VTRUNCS_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VTRUNCS_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VTRUNCS_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VTRUNCS_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VTRUNCS_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VTRUNCUS.

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSWBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v16i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSWBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v32i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_r(X86::VPMOVUSWBZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSDBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSDBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSDWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_MVT_v8i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVUSDBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_MVT_v16i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVUSDWZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_MVT_v16i16_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_MVT_v4i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQBZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQWZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_r(X86::VPMOVUSQDZ256rr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_MVT_v4i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v16i8_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVUSQBZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v8i16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVUSQWZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v8i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VPMOVUSQDZrr, &X86::VR256XRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v16i8_r(Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v8i16_r(Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_MVT_v8i32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_X86ISD_VTRUNCUS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v16i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VTRUNCUS_MVT_v32i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VTRUNCUS_MVT_v16i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VTRUNCUS_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VTRUNCUS_MVT_v4i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VTRUNCUS_MVT_v8i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VZEXT_MOVL.

unsigned fastEmit_X86ISD_VZEXT_MOVL_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVZPQILo2PQIZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVZPQILo2PQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVZPQILo2PQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VZEXT_MOVL_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVZPQILo2PQIZrr, &X86::VR128XRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_r(X86::MOVZPQILo2PQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_r(X86::VMOVZPQILo2PQIrr, &X86::VR128RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VZEXT_MOVL_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_VZEXT_MOVL_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_VZEXT_MOVL_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::WIN_ALLOCA.

unsigned fastEmit_X86ISD_WIN_ALLOCA_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((!Subtarget->isTarget64BitLP64())) {
    return fastEmitInst_r(X86::WIN_ALLOCA_32, &X86::GR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_WIN_ALLOCA_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->is64Bit())) {
    return fastEmitInst_r(X86::WIN_ALLOCA_64, &X86::GR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_WIN_ALLOCA_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_WIN_ALLOCA_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_X86ISD_WIN_ALLOCA_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_r(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill) override {
  switch (Opcode) {
  case ISD::ABS: return fastEmit_ISD_ABS_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::ANY_EXTEND: return fastEmit_ISD_ANY_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::BITCAST: return fastEmit_ISD_BITCAST_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::BRIND: return fastEmit_ISD_BRIND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::BSWAP: return fastEmit_ISD_BSWAP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTLZ: return fastEmit_ISD_CTLZ_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTPOP: return fastEmit_ISD_CTPOP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTTZ: return fastEmit_ISD_CTTZ_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTTZ_ZERO_UNDEF: return fastEmit_ISD_CTTZ_ZERO_UNDEF_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FABS: return fastEmit_ISD_FABS_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FNEG: return fastEmit_ISD_FNEG_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_EXTEND: return fastEmit_ISD_FP_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_ROUND: return fastEmit_ISD_FP_ROUND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_SINT: return fastEmit_ISD_FP_TO_SINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_UINT: return fastEmit_ISD_FP_TO_UINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FSQRT: return fastEmit_ISD_FSQRT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SCALAR_TO_VECTOR: return fastEmit_ISD_SCALAR_TO_VECTOR_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SIGN_EXTEND: return fastEmit_ISD_SIGN_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SIGN_EXTEND_VECTOR_INREG: return fastEmit_ISD_SIGN_EXTEND_VECTOR_INREG_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SINT_TO_FP: return fastEmit_ISD_SINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_FP_EXTEND: return fastEmit_ISD_STRICT_FP_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_FP_ROUND: return fastEmit_ISD_STRICT_FP_ROUND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_FP_TO_SINT: return fastEmit_ISD_STRICT_FP_TO_SINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_FP_TO_UINT: return fastEmit_ISD_STRICT_FP_TO_UINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_FSQRT: return fastEmit_ISD_STRICT_FSQRT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_SINT_TO_FP: return fastEmit_ISD_STRICT_SINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::STRICT_UINT_TO_FP: return fastEmit_ISD_STRICT_UINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::TRUNCATE: return fastEmit_ISD_TRUNCATE_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::UINT_TO_FP: return fastEmit_ISD_UINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::ZERO_EXTEND: return fastEmit_ISD_ZERO_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::ZERO_EXTEND_VECTOR_INREG: return fastEmit_ISD_ZERO_EXTEND_VECTOR_INREG_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CALL: return fastEmit_X86ISD_CALL_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CONFLICT: return fastEmit_X86ISD_CONFLICT_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTNEPS2BF16: return fastEmit_X86ISD_CVTNEPS2BF16_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTP2SI: return fastEmit_X86ISD_CVTP2SI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTP2UI: return fastEmit_X86ISD_CVTP2UI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTPH2PS: return fastEmit_X86ISD_CVTPH2PS_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTPH2PS_SAE: return fastEmit_X86ISD_CVTPH2PS_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTS2SI: return fastEmit_X86ISD_CVTS2SI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTS2UI: return fastEmit_X86ISD_CVTS2UI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTSI2P: return fastEmit_X86ISD_CVTSI2P_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTP2SI: return fastEmit_X86ISD_CVTTP2SI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTP2SI_SAE: return fastEmit_X86ISD_CVTTP2SI_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTP2UI: return fastEmit_X86ISD_CVTTP2UI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTP2UI_SAE: return fastEmit_X86ISD_CVTTP2UI_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTS2SI: return fastEmit_X86ISD_CVTTS2SI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTS2SI_SAE: return fastEmit_X86ISD_CVTTS2SI_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTS2UI: return fastEmit_X86ISD_CVTTS2UI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTTS2UI_SAE: return fastEmit_X86ISD_CVTTS2UI_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::CVTUI2P: return fastEmit_X86ISD_CVTUI2P_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::EH_RETURN: return fastEmit_X86ISD_EH_RETURN_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::EXP2: return fastEmit_X86ISD_EXP2_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::EXP2_SAE: return fastEmit_X86ISD_EXP2_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::FGETEXP: return fastEmit_X86ISD_FGETEXP_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::FGETEXP_SAE: return fastEmit_X86ISD_FGETEXP_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::FRCP: return fastEmit_X86ISD_FRCP_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::FRSQRT: return fastEmit_X86ISD_FRSQRT_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MMX_MOVD2W: return fastEmit_X86ISD_MMX_MOVD2W_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MMX_MOVW2D: return fastEmit_X86ISD_MMX_MOVW2D_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVDDUP: return fastEmit_X86ISD_MOVDDUP_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVDQ2Q: return fastEmit_X86ISD_MOVDQ2Q_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVMSK: return fastEmit_X86ISD_MOVMSK_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVQ2DQ: return fastEmit_X86ISD_MOVQ2DQ_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVSHDUP: return fastEmit_X86ISD_MOVSHDUP_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::MOVSLDUP: return fastEmit_X86ISD_MOVSLDUP_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::NT_BRIND: return fastEmit_X86ISD_NT_BRIND_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::NT_CALL: return fastEmit_X86ISD_NT_CALL_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::PHMINPOS: return fastEmit_X86ISD_PHMINPOS_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RCP14: return fastEmit_X86ISD_RCP14_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RCP28: return fastEmit_X86ISD_RCP28_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RCP28_SAE: return fastEmit_X86ISD_RCP28_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RSQRT14: return fastEmit_X86ISD_RSQRT14_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RSQRT28: return fastEmit_X86ISD_RSQRT28_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::RSQRT28_SAE: return fastEmit_X86ISD_RSQRT28_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::SEG_ALLOCA: return fastEmit_X86ISD_SEG_ALLOCA_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_CVTSI2P: return fastEmit_X86ISD_STRICT_CVTSI2P_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_CVTTP2SI: return fastEmit_X86ISD_STRICT_CVTTP2SI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_CVTTP2UI: return fastEmit_X86ISD_STRICT_CVTTP2UI_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_CVTUI2P: return fastEmit_X86ISD_STRICT_CVTUI2P_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_VFPEXT: return fastEmit_X86ISD_STRICT_VFPEXT_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::STRICT_VFPROUND: return fastEmit_X86ISD_STRICT_VFPROUND_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VBROADCAST: return fastEmit_X86ISD_VBROADCAST_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VBROADCASTM: return fastEmit_X86ISD_VBROADCASTM_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VFPEXT: return fastEmit_X86ISD_VFPEXT_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VFPEXT_SAE: return fastEmit_X86ISD_VFPEXT_SAE_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VFPROUND: return fastEmit_X86ISD_VFPROUND_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VTRUNC: return fastEmit_X86ISD_VTRUNC_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VTRUNCS: return fastEmit_X86ISD_VTRUNCS_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VTRUNCUS: return fastEmit_X86ISD_VTRUNCUS_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::VZEXT_MOVL: return fastEmit_X86ISD_VZEXT_MOVL_r(VT, RetVT, Op0, Op0IsKill);
  case X86ISD::WIN_ALLOCA: return fastEmit_X86ISD_WIN_ALLOCA_r(VT, RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_rr(X86::ADD8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::ADD16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::ADD32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::ADD64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPADDDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPADDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ADD_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_ADD_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_ADD_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_ADD_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_ADD_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_ADD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_ADD_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_ADD_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ADD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_ADD_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_ADD_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ADD_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_ADD_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_ADD_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_rr(X86::AND8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::AND16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::AND32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::AND64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i1)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KANDBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i1)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::KANDWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KANDDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KANDQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_AND_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_AND_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i1: return fastEmit_ISD_AND_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_ISD_AND_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_ISD_AND_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_ISD_AND_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_AND_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_AND_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_AND_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_AND_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_AND_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_AND_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_AND_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_AND_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_AND_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_AND_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_AND_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_AND_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FADD.

unsigned fastEmit_ISD_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::ADD_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::ADD_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::ADD_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FADD_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FADD_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_FADD_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FADD_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_FADD_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_FADD_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FADD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FADD_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_FADD_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FDIV.

unsigned fastEmit_ISD_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::DIV_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::DIV_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::DIV_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FDIV_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FDIV_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_FDIV_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FDIV_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_FDIV_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_FDIV_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FDIV_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FDIV_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_FDIV_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMUL.

unsigned fastEmit_ISD_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::MUL_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::MUL_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::MUL_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FMUL_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FMUL_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_FMUL_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FMUL_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_FMUL_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_FMUL_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FMUL_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FMUL_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_FMUL_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSUB.

unsigned fastEmit_ISD_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::SUB_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::SUB_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::SUB_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSUB_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FSUB_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_FSUB_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_FSUB_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_FSUB_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FSUB_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_FSUB_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::AL).addReg(Op0);
  return fastEmitInst_r(X86::MUL8r, &X86::GR8RegClass, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::IMUL16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::IMUL32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::IMUL64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULLWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULLWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMULLDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasDQI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULLQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::VPMULLQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_MUL_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_MUL_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_MUL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_MUL_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_MUL_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_MUL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_MUL_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_MUL_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_MUL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_MUL_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_MUL_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHS.

unsigned fastEmit_ISD_MULHS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULHWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_MULHS_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_MULHS_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_MULHS_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHU.

unsigned fastEmit_ISD_MULHU_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHUWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULHUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHUWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHUWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHUWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_ISD_MULHU_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_MULHU_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_MULHU_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_rr(X86::OR8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::OR16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::OR32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::OR64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i1)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KORBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i1)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::KORWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KORDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KORQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPORDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_OR_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_OR_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i1: return fastEmit_ISD_OR_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_ISD_OR_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_ISD_OR_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_ISD_OR_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_OR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_OR_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_OR_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_OR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_OR_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_OR_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_OR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_OR_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_OR_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_OR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_OR_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_OR_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTL.

unsigned fastEmit_ISD_ROTL_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::CL).addReg(Op1);
  return fastEmitInst_r(X86::ROL8rCL, &X86::GR8RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_ROTL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPROTBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPROTWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPROLVDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPROTDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPROLVDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPROLVDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPROLVQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPROTQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPROLVQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPROLVQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ROTL_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_ROTL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_ROTL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ROTL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_ROTL_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_ROTL_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ROTL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_ROTL_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_ROTL_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::CL).addReg(Op1);
  return fastEmitInst_r(X86::ROR8rCL, &X86::GR8RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_ROTR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPRORVDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPRORVDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPRORVDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPRORVQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPRORVQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPRORVQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ROTR_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ROTR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_ROTR_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_ROTR_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ROTR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_ROTR_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_ROTR_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SADDSAT.

unsigned fastEmit_ISD_SADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SADDSAT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SADDSAT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_SADDSAT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_SADDSAT_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SADDSAT_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_SADDSAT_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_SADDSAT_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::CL).addReg(Op1);
  return fastEmitInst_r(X86::SHL8rCL, &X86::GR8RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SHL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SHL_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMAXSDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXSQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMAXSQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_SMAX_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_SMAX_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_SMAX_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_SMAX_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_SMAX_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_SMAX_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SMAX_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_SMAX_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_SMAX_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMINSDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINSQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMINSQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_SMIN_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_SMIN_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_SMIN_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_SMIN_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_SMIN_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_SMIN_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SMIN_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_SMIN_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_SMIN_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::CL).addReg(Op1);
  return fastEmitInst_r(X86::SAR8rCL, &X86::GR8RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SRA_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SRA_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  BuildMI(*FuncInfo.MBB, FuncInfo.InsertPt, DbgLoc, TII.get(TargetOpcode::COPY), X86::CL).addReg(Op1);
  return fastEmitInst_r(X86::SHR8rCL, &X86::GR8RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SRL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SRL_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SSUBSAT.

unsigned fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SSUBSAT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SSUBSAT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_SSUBSAT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_SSUBSAT_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SSUBSAT_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_SSUBSAT_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_SSUBSAT_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FADD.

unsigned fastEmit_ISD_STRICT_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::ADD_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::ADD_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::ADD_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VADDPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FADD_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FADD_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_STRICT_FADD_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FADD_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FADD_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_STRICT_FADD_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FADD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FADD_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_FADD_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FDIV.

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::DIV_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::DIV_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::DIV_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::DIVPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VDIVPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FDIV_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FDIV_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_STRICT_FDIV_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FDIV_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FDIV_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_STRICT_FDIV_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FDIV_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FDIV_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_FDIV_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FMUL.

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::MUL_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::MUL_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::MUL_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MULPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMULPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FMUL_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FMUL_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_STRICT_FMUL_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FMUL_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FMUL_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_STRICT_FMUL_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FMUL_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FMUL_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_FMUL_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::STRICT_FSUB.

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1())) {
    return fastEmitInst_rr(X86::SUB_Fp32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2())) {
    return fastEmitInst_rr(X86::SUB_Fp64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f80)
    return 0;
  return fastEmitInst_rr(X86::SUB_Fp80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::SUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSUBPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_STRICT_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_STRICT_FSUB_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_STRICT_FSUB_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_ISD_STRICT_FSUB_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_STRICT_FSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_ISD_STRICT_FSUB_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_ISD_STRICT_FSUB_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_STRICT_FSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_STRICT_FSUB_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_ISD_STRICT_FSUB_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_rr(X86::SUB8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SUB_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::SUB16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SUB_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::SUB32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SUB_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::SUB64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SUB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSUBDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSUBQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SUB_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_SUB_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SUB_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SUB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_SUB_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_SUB_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SUB_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_SUB_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_SUB_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SUB_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_SUB_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_SUB_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SUB_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_SUB_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_SUB_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UADDSAT.

unsigned fastEmit_ISD_UADDSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDUSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDUSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDUSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDUSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PADDUSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPADDUSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPADDUSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UADDSAT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UADDSAT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_UADDSAT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_UADDSAT_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UADDSAT_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_UADDSAT_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_UADDSAT_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXUBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMAXUWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMAXUDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMAXUDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMAXUQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMAXUQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_UMAX_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_UMAX_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_UMAX_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_UMAX_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_UMAX_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_UMAX_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UMAX_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_UMAX_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_UMAX_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINUBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMINUWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMINUDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMINUDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMINUQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMINUQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_UMIN_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_UMIN_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_UMIN_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_UMIN_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_UMIN_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_UMIN_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UMIN_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_UMIN_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_UMIN_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::USUBSAT.

unsigned fastEmit_ISD_USUBSAT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBUSBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBUSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBUSBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBUSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSUBUSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSUBUSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSUBUSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_USUBSAT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_USUBSAT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_USUBSAT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_USUBSAT_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_USUBSAT_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_USUBSAT_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_USUBSAT_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_rr(X86::XOR8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_rr(X86::XOR16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::XOR32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(X86::XOR64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i1)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KXORBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i1)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::KXORWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KXORDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KXORQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VXORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPXORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VXORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPXORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VXORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPXORDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VXORPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPXORYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPXORQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_XOR_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_ISD_XOR_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i1: return fastEmit_ISD_XOR_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_ISD_XOR_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_ISD_XOR_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_ISD_XOR_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_XOR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_ISD_XOR_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_ISD_XOR_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_XOR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_ISD_XOR_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_ISD_XOR_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_XOR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_ISD_XOR_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_ISD_XOR_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_XOR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_ISD_XOR_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_ISD_XOR_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::ADDSUB.

unsigned fastEmit_X86ISD_ADDSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VADDSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ADDSUB_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VADDSUBPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ADDSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ADDSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VADDSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ADDSUB_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VADDSUBPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ADDSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_ADDSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_ADDSUB_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_ADDSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_ADDSUB_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::ANDNP.

unsigned fastEmit_X86ISD_ANDNP_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDNPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDNQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDNPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDNQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDNPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDNDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VANDNPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPANDNYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPANDNQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_ANDNP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_ANDNP_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_ANDNP_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_ANDNP_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_ANDNP_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_ANDNP_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_ANDNP_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_ANDNP_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_ANDNP_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_ANDNP_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_ANDNP_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_ANDNP_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_ANDNP_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::AVG.

unsigned fastEmit_X86ISD_AVG_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPAVGBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PAVGBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPAVGBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPAVGWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PAVGWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPAVGWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPAVGWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_AVG_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_AVG_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_AVG_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_AVG_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_AVG_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_AVG_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_AVG_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BEXTR.

unsigned fastEmit_X86ISD_BEXTR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBMI())) {
    return fastEmitInst_rr(X86::BEXTR32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BEXTR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasBMI())) {
    return fastEmitInst_rr(X86::BEXTR64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BEXTR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_BEXTR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_X86ISD_BEXTR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BT.

unsigned fastEmit_X86ISD_BT_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::BT16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_BT_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::BT32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_BT_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::BT64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_BT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_X86ISD_BT_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_X86ISD_BT_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_X86ISD_BT_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BZHI.

unsigned fastEmit_X86ISD_BZHI_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBMI2())) {
    return fastEmitInst_rr(X86::BZHI32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BZHI_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasBMI2())) {
    return fastEmitInst_rr(X86::BZHI64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BZHI_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_BZHI_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_X86ISD_BZHI_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CMP.

unsigned fastEmit_X86ISD_CMP_MVT_i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::CMP8rr, &X86::GR8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_CMP_MVT_i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::CMP16rr, &X86::GR16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_CMP_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::CMP32rr, &X86::GR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_CMP_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(X86::CMP64rr, &X86::GR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_CMP_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CMP_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CMP_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CMP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_X86ISD_CMP_MVT_i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i16: return fastEmit_X86ISD_CMP_MVT_i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_X86ISD_CMP_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_X86ISD_CMP_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f32: return fastEmit_X86ISD_CMP_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_CMP_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_X86ISD_CMP_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::COMI.

unsigned fastEmit_X86ISD_COMI_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::COMISSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_COMI_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::COMISDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_COMI_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_COMI_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_COMI_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::CVTNE2PS2BF16.

unsigned fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBF16()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VCVTNE2PS2BF16Z128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBF16()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VCVTNE2PS2BF16Z256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBF16())) {
    return fastEmitInst_rr(X86::VCVTNE2PS2BF16Zrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_CVTNE2PS2BF16_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_CVTNE2PS2BF16_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FADDS.

unsigned fastEmit_X86ISD_FADDS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FADDS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VADDSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FADDS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FADDS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FADDS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FAND.

unsigned fastEmit_X86ISD_FAND_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VANDPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VANDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ANDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FAND_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(X86::ANDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_FAND_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f128: return fastEmit_X86ISD_FAND_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FAND_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FANDN.

unsigned fastEmit_X86ISD_FANDN_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(X86::ANDNPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_FANDN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FANDN_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FDIVS.

unsigned fastEmit_X86ISD_FDIVS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FDIVS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VDIVSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FDIVS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FDIVS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FDIVS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FGETEXPS.

unsigned fastEmit_X86ISD_FGETEXPS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VGETEXPSSZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXPS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VGETEXPSDZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXPS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FGETEXPS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FGETEXPS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FGETEXPS_SAE.

unsigned fastEmit_X86ISD_FGETEXPS_SAE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VGETEXPSSZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXPS_SAE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VGETEXPSDZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FGETEXPS_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FGETEXPS_SAE_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FGETEXPS_SAE_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FHADD.

unsigned fastEmit_X86ISD_FHADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::HADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHADDPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHADD_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHADDPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::HADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHADDPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHADD_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHADDPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FHADD_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FHADD_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FHADD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FHADD_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FHSUB.

unsigned fastEmit_X86ISD_FHSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::HSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHSUBPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHSUB_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHSUBPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::HSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHSUBPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHSUB_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VHSUBPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FHSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FHSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FHSUB_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FHSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FHSUB_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMAX.

unsigned fastEmit_X86ISD_FMAX_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FMAX_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_FMAX_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FMAX_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FMAX_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_FMAX_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMAX_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FMAX_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMAX_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMAXC.

unsigned fastEmit_X86ISD_FMAXC_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXCSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXCSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXCPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXCPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMAXCPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXCPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXC_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FMAXC_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_FMAXC_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FMAXC_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FMAXC_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_FMAXC_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMAXC_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FMAXC_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMAXC_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMAXS.

unsigned fastEmit_X86ISD_FMAXS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXSSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MAXSDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FMAXS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMAXS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMAXS_SAE.

unsigned fastEmit_X86ISD_FMAXS_SAE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSSZrrb_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXS_SAE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXSDZrrb_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAXS_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FMAXS_SAE_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMAXS_SAE_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMAX_SAE.

unsigned fastEmit_X86ISD_FMAX_SAE_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXPSZrrb, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_SAE_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMAXPDZrrb, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMAX_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_FMAX_SAE_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMAX_SAE_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMIN.

unsigned fastEmit_X86ISD_FMIN_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FMIN_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_FMIN_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FMIN_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FMIN_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_FMIN_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMIN_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FMIN_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMIN_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMINC.

unsigned fastEmit_X86ISD_FMINC_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCSSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINCSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCSSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCSDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINCSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCSDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINCPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINCPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VMINCPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINCPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINC_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_FMINC_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_FMINC_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FMINC_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_FMINC_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_FMINC_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMINC_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_FMINC_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMINC_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMINS.

unsigned fastEmit_X86ISD_FMINS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINSSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MINSDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FMINS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMINS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMINS_SAE.

unsigned fastEmit_X86ISD_FMINS_SAE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSSZrrb_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINS_SAE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINSDZrrb_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMINS_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FMINS_SAE_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMINS_SAE_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMIN_SAE.

unsigned fastEmit_X86ISD_FMIN_SAE_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINPSZrrb, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_SAE_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMINPDZrrb, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMIN_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16f32: return fastEmit_X86ISD_FMIN_SAE_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_FMIN_SAE_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FMULS.

unsigned fastEmit_X86ISD_FMULS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMULS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMULSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FMULS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FMULS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FMULS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FOR.

unsigned fastEmit_X86ISD_FOR_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VORPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::ORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FOR_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(X86::ORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_FOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f128: return fastEmit_X86ISD_FOR_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FOR_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FSQRTS.

unsigned fastEmit_X86ISD_FSQRTS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSQRTSSZr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FSQRTS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSQRTSDZr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FSQRTS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FSQRTS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FSQRTS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FSUBS.

unsigned fastEmit_X86ISD_FSUBS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FSUBS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSUBSDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FSUBS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_FSUBS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_FSUBS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::FXOR.

unsigned fastEmit_X86ISD_FXOR_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VXORPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VXORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::XORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_FXOR_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  return fastEmitInst_rr(X86::XORPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_X86ISD_FXOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f128: return fastEmit_X86ISD_FXOR_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_FXOR_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::GF2P8MULB.

unsigned fastEmit_X86ISD_GF2P8MULB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasGFNI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VGF2P8MULBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (Subtarget->hasGFNI()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VGF2P8MULBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasGFNI()) && (Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::GF2P8MULBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_GF2P8MULB_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasGFNI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VGF2P8MULBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (Subtarget->hasGFNI()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VGF2P8MULBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_GF2P8MULB_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasBWI()) && (Subtarget->hasGFNI())) {
    return fastEmitInst_rr(X86::VGF2P8MULBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_GF2P8MULB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_GF2P8MULB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_GF2P8MULB_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_GF2P8MULB_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::HADD.

unsigned fastEmit_X86ISD_HADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PHADDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPHADDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HADD_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPHADDWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PHADDDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPHADDDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HADD_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPHADDDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_HADD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_HADD_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_HADD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_HADD_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::HSUB.

unsigned fastEmit_X86ISD_HSUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PHSUBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPHSUBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HSUB_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPHSUBWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HSUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PHSUBDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPHSUBDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HSUB_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPHSUBDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_HSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_HSUB_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_HSUB_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_HSUB_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_HSUB_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::KADD.

unsigned fastEmit_X86ISD_KADD_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i1)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KADDBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KADD_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i1)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KADDWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KADD_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KADDDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KADD_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i1)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KADDQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i1: return fastEmit_X86ISD_KADD_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_X86ISD_KADD_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_X86ISD_KADD_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_X86ISD_KADD_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::KORTEST.

unsigned fastEmit_X86ISD_KORTEST_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KORTESTBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KORTEST_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::KORTESTWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KORTEST_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KORTESTDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KORTEST_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KORTESTQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KORTEST_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i1: return fastEmit_X86ISD_KORTEST_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_X86ISD_KORTEST_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_X86ISD_KORTEST_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_X86ISD_KORTEST_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::KTEST.

unsigned fastEmit_X86ISD_KTEST_MVT_v8i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KTESTBrr, &X86::VK8RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KTEST_MVT_v16i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_rr(X86::KTESTWrr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KTEST_MVT_v32i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KTESTDrr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KTEST_MVT_v64i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::KTESTQrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_KTEST_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i1: return fastEmit_X86ISD_KTEST_MVT_v8i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i1: return fastEmit_X86ISD_KTEST_MVT_v16i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i1: return fastEmit_X86ISD_KTEST_MVT_v32i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i1: return fastEmit_X86ISD_KTEST_MVT_v64i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVHLPS.

unsigned fastEmit_X86ISD_MOVHLPS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVHLPSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MOVHLPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVHLPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVHLPS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_MOVHLPS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVLHPS.

unsigned fastEmit_X86ISD_MOVLHPS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVLHPSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MOVLHPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVLHPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVLHPS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_MOVLHPS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVSD.

unsigned fastEmit_X86ISD_MOVSD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (shouldOptForSize(MF))) {
    return fastEmitInst_rr(X86::VMOVSDZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((shouldOptForSize(MF) || !Subtarget->hasSSE41()) && (Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MOVSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((shouldOptForSize(MF)) && (Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVSDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2f64: return fastEmit_X86ISD_MOVSD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MOVSS.

unsigned fastEmit_X86ISD_MOVSS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (shouldOptForSize(MF))) {
    return fastEmitInst_rr(X86::VMOVSSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((shouldOptForSize(MF) || !Subtarget->hasSSE41()) && (Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::MOVSSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((shouldOptForSize(MF)) && (Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VMOVSSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MOVSS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_MOVSS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MULHRS.

unsigned fastEmit_X86ISD_MULHRS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHRSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULHRSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHRSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULHRS_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULHRSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHRSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULHRS_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMULHRSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULHRS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_MULHRS_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_MULHRS_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_MULHRS_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::MULTISHIFT.

unsigned fastEmit_X86ISD_MULTISHIFT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVBMI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULTISHIFTQBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULTISHIFT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVBMI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULTISHIFTQBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULTISHIFT_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasVBMI())) {
    return fastEmitInst_rr(X86::VPMULTISHIFTQBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_MULTISHIFT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_MULTISHIFT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_MULTISHIFT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_MULTISHIFT_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PACKSS.

unsigned fastEmit_X86ISD_PACKSS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKSSWBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PACKSSWBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSWBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKSSWBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSWBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSWBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKSSDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PACKSSDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKSSDWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSDWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKSSDWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKSS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_PACKSS_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_PACKSS_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_PACKSS_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_PACKSS_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_PACKSS_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_PACKSS_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PACKUS.

unsigned fastEmit_X86ISD_PACKUS_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKUSWBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PACKUSWBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSWBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKUSWBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSWBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSWBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKUSDWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PACKUSDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSDWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPACKUSDWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSDWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPACKUSDWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PACKUS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_PACKUS_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_PACKUS_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_PACKUS_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_PACKUS_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_PACKUS_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_PACKUS_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PCMPEQ.

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPEQBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPEQWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPEQDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPEQDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPEQQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPCMPEQQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPCMPEQQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPEQ_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_PCMPEQ_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_PCMPEQ_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_PCMPEQ_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_PCMPEQ_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_PCMPEQ_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_PCMPEQ_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_PCMPEQ_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_PCMPEQ_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PCMPGT.

unsigned fastEmit_X86ISD_PCMPGT_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPGTBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPGTWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPGTDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX2()) && (true)) {
    return fastEmitInst_rr(X86::VPCMPGTDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasSSE42() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PCMPGTQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPCMPGTQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VPCMPGTQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PCMPGT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_PCMPGT_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_PCMPGT_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_PCMPGT_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_PCMPGT_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_PCMPGT_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_PCMPGT_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_PCMPGT_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_PCMPGT_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PMULDQ.

unsigned fastEmit_X86ISD_PMULDQ_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULDQ_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULDQ_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMULDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULDQ_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_PMULDQ_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_PMULDQ_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_PMULDQ_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PMULUDQ.

unsigned fastEmit_X86ISD_PMULUDQ_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULUDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMULUDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULUDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULUDQ_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULUDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMULUDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULUDQ_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPMULUDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PMULUDQ_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_PMULUDQ_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_PMULUDQ_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_PMULUDQ_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PSADBW.

unsigned fastEmit_X86ISD_PSADBW_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSADBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSADBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSADBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSADBW_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSADBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSADBWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSADBW_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSADBWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSADBW_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_PSADBW_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_PSADBW_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_PSADBW_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PSHUFB.

unsigned fastEmit_X86ISD_PSHUFB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSHUFBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSHUFBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSHUFBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSHUFB_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSHUFBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSHUFBYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSHUFB_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSHUFBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PSHUFB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_PSHUFB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_PSHUFB_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_PSHUFB_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PTEST.

unsigned fastEmit_X86ISD_PTEST_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PTESTrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPTESTrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PTEST_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VPTESTYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PTEST_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v2i64: return fastEmit_X86ISD_PTEST_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_PTEST_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP14S.

unsigned fastEmit_X86ISD_RCP14S_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VRCP14SSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14S_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VRCP14SDZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP14S_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RCP14S_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RCP14S_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP28S.

unsigned fastEmit_X86ISD_RCP28S_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRCP28SSZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28S_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRCP28SDZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28S_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RCP28S_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RCP28S_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RCP28S_SAE.

unsigned fastEmit_X86ISD_RCP28S_SAE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRCP28SSZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28S_SAE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRCP28SDZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RCP28S_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RCP28S_SAE_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RCP28S_SAE_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT14S.

unsigned fastEmit_X86ISD_RSQRT14S_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VRSQRT14SSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14S_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VRSQRT14SDZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT14S_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RSQRT14S_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RSQRT14S_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT28S.

unsigned fastEmit_X86ISD_RSQRT28S_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRSQRT28SSZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28S_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRSQRT28SDZr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28S_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RSQRT28S_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RSQRT28S_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::RSQRT28S_SAE.

unsigned fastEmit_X86ISD_RSQRT28S_SAE_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRSQRT28SSZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28S_SAE_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasERI())) {
    return fastEmitInst_rr(X86::VRSQRT28SDZrb, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_RSQRT28S_SAE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_RSQRT28S_SAE_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_RSQRT28S_SAE_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::SCALEF.

unsigned fastEmit_X86ISD_SCALEF_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSCALEFPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSCALEFPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSCALEFPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSCALEFPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VSCALEFPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSCALEFPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEF_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_SCALEF_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_SCALEF_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_SCALEF_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_SCALEF_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_SCALEF_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_SCALEF_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::SCALEFS.

unsigned fastEmit_X86ISD_SCALEFS_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSCALEFSSZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEFS_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VSCALEFSDZrr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_SCALEFS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_SCALEFS_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_SCALEFS_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_FCMP.

unsigned fastEmit_X86ISD_STRICT_FCMP_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMP_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMP_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::UCOM_FpIr80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_STRICT_FCMP_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_STRICT_FCMP_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_X86ISD_STRICT_FCMP_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::STRICT_FCMPS.

unsigned fastEmit_X86ISD_STRICT_FCMPS_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISSZrr, &X86::FR32XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::COMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISSrr, &X86::FR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE1()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::COM_FpIr32, &X86::RFP32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMPS_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISDZrr, &X86::FR64XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::COMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VCOMISDrr, &X86::FR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->hasSSE2()) && (Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::COM_FpIr64, &X86::RFP64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMPS_MVT_f80_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCMov())) {
    return fastEmitInst_rr(X86::COM_FpIr80, &X86::RFP80RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_STRICT_FCMPS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_X86ISD_STRICT_FCMPS_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_X86ISD_STRICT_FCMPS_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f80: return fastEmit_X86ISD_STRICT_FCMPS_MVT_f80_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::TESTP.

unsigned fastEmit_X86ISD_TESTP_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VTESTPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_TESTP_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VTESTPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_TESTP_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VTESTPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_TESTP_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::VTESTPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_TESTP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_TESTP_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_TESTP_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_TESTP_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_TESTP_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::UCOMI.

unsigned fastEmit_X86ISD_UCOMI_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISSrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UCOMI_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDZrr_Int, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UCOMISDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUCOMISDrr_Int, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UCOMI_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_X86ISD_UCOMI_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_UCOMI_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::UNPCKH.

unsigned fastEmit_X86ISD_UNPCKH_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKHBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHBWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHBWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHWDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKHWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHWDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHWDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKHWDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKHDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VUNPCKHPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPUNPCKHDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHQDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKHQDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHQDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VUNPCKHPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHQDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKHQDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPUNPCKHQDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UNPCKHPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUNPCKHPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UNPCKHPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKHPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUNPCKHPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKH_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_UNPCKH_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_UNPCKH_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_UNPCKH_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_UNPCKH_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_UNPCKH_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_UNPCKH_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_UNPCKH_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_UNPCKH_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_UNPCKH_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_UNPCKH_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_UNPCKH_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_UNPCKH_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_UNPCKH_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_UNPCKH_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_UNPCKH_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_UNPCKH_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_UNPCKH_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_UNPCKH_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::UNPCKL.

unsigned fastEmit_X86ISD_UNPCKL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLBWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKLBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLBWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLBWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLBWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLBWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLWDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKLWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLWDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLWDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPUNPCKLWDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKLDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VUNPCKLPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPUNPCKLDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLQDQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PUNPCKLQDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLQDQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX() && !Subtarget->hasAVX2())) {
    return fastEmitInst_rr(X86::VUNPCKLPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLQDQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPUNPCKLQDQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPUNPCKLQDQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPSZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE1() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UNPCKLPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPSrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v8f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPSZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPSYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v16f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16f32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUNPCKLPSZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::UNPCKLPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VUNPCKLPDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_MVT_v8f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8f64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VUNPCKLPDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_UNPCKL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_UNPCKL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_UNPCKL_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_UNPCKL_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_UNPCKL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_UNPCKL_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_UNPCKL_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_UNPCKL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_UNPCKL_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_UNPCKL_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_UNPCKL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_UNPCKL_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_UNPCKL_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_X86ISD_UNPCKL_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f32: return fastEmit_X86ISD_UNPCKL_MVT_v8f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16f32: return fastEmit_X86ISD_UNPCKL_MVT_v16f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_X86ISD_UNPCKL_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_X86ISD_UNPCKL_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8f64: return fastEmit_X86ISD_UNPCKL_MVT_v8f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VP2INTERSECT.

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTDZ128rr, &X86::VK4PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTDZ256rr, &X86::VK8PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTDZrr, &X86::VK16PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTQZ128rr, &X86::VK2PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTQZ256rr, &X86::VK4PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVP2INTERSECT())) {
    return fastEmitInst_rr(X86::VP2INTERSECTQZrr, &X86::VK8PAIRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VP2INTERSECT_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_X86ISD_VP2INTERSECT_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VP2INTERSECT_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VP2INTERSECT_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VP2INTERSECT_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VP2INTERSECT_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VP2INTERSECT_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPERMV.

unsigned fastEmit_X86ISD_VPERMV_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasVBMI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMBZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i8)
    return 0;
  if ((Subtarget->hasVBMI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMBZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i8)
    return 0;
  if ((Subtarget->hasVBMI())) {
    return fastEmitInst_rr(X86::VPERMBZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPERMWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPERMDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPERMQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPERMQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPERMV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VPERMV_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_VPERMV_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_VPERMV_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VPERMV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VPERMV_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VPERMV_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VPERMV_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VPERMV_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VPERMV_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VPERMV_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPMADDUBSW.

unsigned fastEmit_X86ISD_VPMADDUBSW_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMADDUBSWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSSE3() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMADDUBSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDUBSWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDUBSW_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMADDUBSWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDUBSWYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDUBSW_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDUBSWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDUBSW_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VPMADDUBSW_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_VPMADDUBSW_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_VPMADDUBSW_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPMADDWD.

unsigned fastEmit_X86ISD_VPMADDWD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMADDWDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PMADDWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDWDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDWD_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPMADDWDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDWDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDWD_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPMADDWDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPMADDWD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VPMADDWD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VPMADDWD_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VPMADDWD_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPSHA.

unsigned fastEmit_X86ISD_VPSHA_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHABrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHA_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHAWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHA_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHADrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHA_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHAQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHA_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VPSHA_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VPSHA_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VPSHA_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VPSHA_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPSHL.

unsigned fastEmit_X86ISD_VPSHL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHLBrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasXOP())) {
    return fastEmitInst_rr(X86::VPSHLQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VPSHL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_X86ISD_VPSHL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VPSHL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VPSHL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VPSHUFBITQMB.

unsigned fastEmit_X86ISD_VPSHUFBITQMB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i1)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSHUFBITQMBZ128rr, &X86::VK16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHUFBITQMB_MVT_v32i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i1)
    return 0;
  if ((Subtarget->hasBITALG()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSHUFBITQMBZ256rr, &X86::VK32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHUFBITQMB_MVT_v64i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v64i1)
    return 0;
  if ((Subtarget->hasBITALG())) {
    return fastEmitInst_rr(X86::VPSHUFBITQMBZrr, &X86::VK64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VPSHUFBITQMB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_VPSHUFBITQMB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i8: return fastEmit_X86ISD_VPSHUFBITQMB_MVT_v32i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v64i8: return fastEmit_X86ISD_VPSHUFBITQMB_MVT_v64i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSHL.

unsigned fastEmit_X86ISD_VSHL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSLLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSLLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSLLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSLLQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSHL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSHL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSHL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSHLV.

unsigned fastEmit_X86ISD_VSHLV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSLLVWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSLLVDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSLLVQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSLLVQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSHLV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSHLV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VSHLV_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VSHLV_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSHLV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VSHLV_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VSHLV_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSHLV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VSHLV_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VSHLV_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSRA.

unsigned fastEmit_X86ISD_VSRA_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSRAWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSRAWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRA_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRADZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSRADrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRADrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRA_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRA_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSRA_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSRA_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSRA_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSRAV.

unsigned fastEmit_X86ISD_VSRAV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSRAVWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSRAVDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRAVQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSRAVQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRAV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSRAV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VSRAV_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VSRAV_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSRAV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VSRAV_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VSRAV_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSRAV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VSRAV_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VSRAV_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSRL.

unsigned fastEmit_X86ISD_VSRL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSRLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX() || !Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSRLWrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSRLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_rr(X86::PSRLQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSRL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSRL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSRL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::VSRLV.

unsigned fastEmit_X86ISD_VSRLV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVWZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v16i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i16)
    return 0;
  if ((Subtarget->hasBWI()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVWZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v32i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v32i16)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_rr(X86::VPSRLVWZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVDZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVDrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v8i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i32)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVDZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVDYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v16i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i32)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSRLVDZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVQZ128rr, &X86::VR128XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVQrr, &X86::VR128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v4i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i64)
    return 0;
  if ((Subtarget->hasAVX512()) && (Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVQZ256rr, &X86::VR256XRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasAVX2()) && (!Subtarget->hasVLX())) {
    return fastEmitInst_rr(X86::VPSRLVQYrr, &X86::VR256RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_MVT_v8i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i64)
    return 0;
  if ((Subtarget->hasAVX512())) {
    return fastEmitInst_rr(X86::VPSRLVQZrr, &X86::VR512RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_X86ISD_VSRLV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_VSRLV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i16: return fastEmit_X86ISD_VSRLV_MVT_v16i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v32i16: return fastEmit_X86ISD_VSRLV_MVT_v32i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_X86ISD_VSRLV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i32: return fastEmit_X86ISD_VSRLV_MVT_v8i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i32: return fastEmit_X86ISD_VSRLV_MVT_v16i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_X86ISD_VSRLV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i64: return fastEmit_X86ISD_VSRLV_MVT_v4i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i64: return fastEmit_X86ISD_VSRLV_MVT_v8i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_rr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) override {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::AND: return fastEmit_ISD_AND_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FADD: return fastEmit_ISD_FADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FDIV: return fastEmit_ISD_FDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMUL: return fastEmit_ISD_FMUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FSUB: return fastEmit_ISD_FSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MUL: return fastEmit_ISD_MUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHS: return fastEmit_ISD_MULHS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHU: return fastEmit_ISD_MULHU_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::OR: return fastEmit_ISD_OR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ROTL: return fastEmit_ISD_ROTL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ROTR: return fastEmit_ISD_ROTR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SADDSAT: return fastEmit_ISD_SADDSAT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SHL: return fastEmit_ISD_SHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMAX: return fastEmit_ISD_SMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMIN: return fastEmit_ISD_SMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRA: return fastEmit_ISD_SRA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRL: return fastEmit_ISD_SRL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SSUBSAT: return fastEmit_ISD_SSUBSAT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::STRICT_FADD: return fastEmit_ISD_STRICT_FADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::STRICT_FDIV: return fastEmit_ISD_STRICT_FDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::STRICT_FMUL: return fastEmit_ISD_STRICT_FMUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::STRICT_FSUB: return fastEmit_ISD_STRICT_FSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SUB: return fastEmit_ISD_SUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UADDSAT: return fastEmit_ISD_UADDSAT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMAX: return fastEmit_ISD_UMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMIN: return fastEmit_ISD_UMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::USUBSAT: return fastEmit_ISD_USUBSAT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::XOR: return fastEmit_ISD_XOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::ADDSUB: return fastEmit_X86ISD_ADDSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::ANDNP: return fastEmit_X86ISD_ANDNP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::AVG: return fastEmit_X86ISD_AVG_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::BEXTR: return fastEmit_X86ISD_BEXTR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::BT: return fastEmit_X86ISD_BT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::BZHI: return fastEmit_X86ISD_BZHI_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::CMP: return fastEmit_X86ISD_CMP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::COMI: return fastEmit_X86ISD_COMI_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::CVTNE2PS2BF16: return fastEmit_X86ISD_CVTNE2PS2BF16_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FADDS: return fastEmit_X86ISD_FADDS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FAND: return fastEmit_X86ISD_FAND_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FANDN: return fastEmit_X86ISD_FANDN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FDIVS: return fastEmit_X86ISD_FDIVS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FGETEXPS: return fastEmit_X86ISD_FGETEXPS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FGETEXPS_SAE: return fastEmit_X86ISD_FGETEXPS_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FHADD: return fastEmit_X86ISD_FHADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FHSUB: return fastEmit_X86ISD_FHSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMAX: return fastEmit_X86ISD_FMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMAXC: return fastEmit_X86ISD_FMAXC_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMAXS: return fastEmit_X86ISD_FMAXS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMAXS_SAE: return fastEmit_X86ISD_FMAXS_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMAX_SAE: return fastEmit_X86ISD_FMAX_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMIN: return fastEmit_X86ISD_FMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMINC: return fastEmit_X86ISD_FMINC_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMINS: return fastEmit_X86ISD_FMINS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMINS_SAE: return fastEmit_X86ISD_FMINS_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMIN_SAE: return fastEmit_X86ISD_FMIN_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FMULS: return fastEmit_X86ISD_FMULS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FOR: return fastEmit_X86ISD_FOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FSQRTS: return fastEmit_X86ISD_FSQRTS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FSUBS: return fastEmit_X86ISD_FSUBS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::FXOR: return fastEmit_X86ISD_FXOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::GF2P8MULB: return fastEmit_X86ISD_GF2P8MULB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::HADD: return fastEmit_X86ISD_HADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::HSUB: return fastEmit_X86ISD_HSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::KADD: return fastEmit_X86ISD_KADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::KORTEST: return fastEmit_X86ISD_KORTEST_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::KTEST: return fastEmit_X86ISD_KTEST_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MOVHLPS: return fastEmit_X86ISD_MOVHLPS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MOVLHPS: return fastEmit_X86ISD_MOVLHPS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MOVSD: return fastEmit_X86ISD_MOVSD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MOVSS: return fastEmit_X86ISD_MOVSS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MULHRS: return fastEmit_X86ISD_MULHRS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::MULTISHIFT: return fastEmit_X86ISD_MULTISHIFT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PACKSS: return fastEmit_X86ISD_PACKSS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PACKUS: return fastEmit_X86ISD_PACKUS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PCMPEQ: return fastEmit_X86ISD_PCMPEQ_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PCMPGT: return fastEmit_X86ISD_PCMPGT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PMULDQ: return fastEmit_X86ISD_PMULDQ_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PMULUDQ: return fastEmit_X86ISD_PMULUDQ_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PSADBW: return fastEmit_X86ISD_PSADBW_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PSHUFB: return fastEmit_X86ISD_PSHUFB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::PTEST: return fastEmit_X86ISD_PTEST_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RCP14S: return fastEmit_X86ISD_RCP14S_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RCP28S: return fastEmit_X86ISD_RCP28S_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RCP28S_SAE: return fastEmit_X86ISD_RCP28S_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RSQRT14S: return fastEmit_X86ISD_RSQRT14S_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RSQRT28S: return fastEmit_X86ISD_RSQRT28S_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::RSQRT28S_SAE: return fastEmit_X86ISD_RSQRT28S_SAE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::SCALEF: return fastEmit_X86ISD_SCALEF_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::SCALEFS: return fastEmit_X86ISD_SCALEFS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::STRICT_FCMP: return fastEmit_X86ISD_STRICT_FCMP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::STRICT_FCMPS: return fastEmit_X86ISD_STRICT_FCMPS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::TESTP: return fastEmit_X86ISD_TESTP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::UCOMI: return fastEmit_X86ISD_UCOMI_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::UNPCKH: return fastEmit_X86ISD_UNPCKH_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::UNPCKL: return fastEmit_X86ISD_UNPCKL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VP2INTERSECT: return fastEmit_X86ISD_VP2INTERSECT_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPERMV: return fastEmit_X86ISD_VPERMV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPMADDUBSW: return fastEmit_X86ISD_VPMADDUBSW_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPMADDWD: return fastEmit_X86ISD_VPMADDWD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPSHA: return fastEmit_X86ISD_VPSHA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPSHL: return fastEmit_X86ISD_VPSHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VPSHUFBITQMB: return fastEmit_X86ISD_VPSHUFBITQMB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSHL: return fastEmit_X86ISD_VSHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSHLV: return fastEmit_X86ISD_VSHLV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSRA: return fastEmit_X86ISD_VSRA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSRAV: return fastEmit_X86ISD_VSRAV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSRL: return fastEmit_X86ISD_VSRL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case X86ISD::VSRLV: return fastEmit_X86ISD_VSRLV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::ADD8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::ADD16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::ADD32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_ADD_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_ADD_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::AND8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::AND16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::AND32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_AND_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_AND_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_ri(X86::VPEXTRDZrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_ri(X86::PEXTRDrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasDQI())) {
    return fastEmitInst_ri(X86::VPEXTRDrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasDQI())) {
    return fastEmitInst_ri(X86::VPEXTRQZrr, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_ri(X86::PEXTRQrr, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasDQI())) {
    return fastEmitInst_ri(X86::VPEXTRQrr, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v2i64: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::IMUL16rri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::IMUL32rri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_MUL_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::OR8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::OR16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::OR32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_OR_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_OR_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTL.

unsigned fastEmit_ISD_ROTL_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasFastSHLDRotate())) {
    return fastEmitInst_ri(X86::SHLDROT32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasFastSHLDRotate())) {
    return fastEmitInst_ri(X86::SHLDROT64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTL_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_ROTL_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBMI2())) {
    return fastEmitInst_ri(X86::RORX32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasFastSHLDRotate())) {
    return fastEmitInst_ri(X86::SHRDROT32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasBMI2())) {
    return fastEmitInst_ri(X86::RORX64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasFastSHLDRotate())) {
    return fastEmitInst_ri(X86::SHRDROT64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTR_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_ROTR_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::SHL8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SHL_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::SHL16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SHL_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::SHL32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SHL_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::SHL64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SHL_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SHL_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_SHL_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_SHL_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::SAR8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::SAR16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::SAR32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::SAR64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SRA_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_SRA_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_SRA_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_SRA_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::SHR8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRL_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::SHR16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRL_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::SHR32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRL_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::SHR64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRL_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SRL_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_SRL_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_SRL_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_SRL_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::SUB8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::SUB16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::SUB32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_SUB_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_SUB_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_ri(X86::XOR8ri, &X86::GR8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::XOR16ri, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::XOR32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_XOR_MVT_i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i16: return fastEmit_ISD_XOR_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BEXTR.

unsigned fastEmit_X86ISD_BEXTR_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasTBM())) {
    return fastEmitInst_ri(X86::BEXTRI32ri, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BEXTR_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_X86ISD_BEXTR_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BT.

unsigned fastEmit_X86ISD_BT_MVT_i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::BT16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_X86ISD_BT_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::BT32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_X86ISD_BT_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::BT64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_X86ISD_BT_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_X86ISD_BT_MVT_i16_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i32: return fastEmit_X86ISD_BT_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_X86ISD_BT_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PEXTRB.

unsigned fastEmit_X86ISD_PEXTRB_MVT_v16i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_ri(X86::VPEXTRBZrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasSSE41() && !Subtarget->hasAVX())) {
    return fastEmitInst_ri(X86::PEXTRBrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasBWI())) {
    return fastEmitInst_ri(X86::VPEXTRBrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PEXTRB_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_X86ISD_PEXTRB_MVT_v16i8_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::PEXTRW.

unsigned fastEmit_X86ISD_PEXTRW_MVT_v8i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasBWI())) {
    return fastEmitInst_ri(X86::VPEXTRWZrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasSSE2() && !Subtarget->hasAVX())) {
    return fastEmitInst_ri(X86::PEXTRWrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasAVX()) && (!Subtarget->hasBWI())) {
    return fastEmitInst_ri(X86::VPEXTRWrr, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_X86ISD_PEXTRW_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v8i16: return fastEmit_X86ISD_PEXTRW_MVT_v8i16_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) override {
  if (VT == MVT::i64 && Predicate_i64immSExt32(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_i64immSExt32(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i16 && Predicate_i16immSExt8(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_i16immSExt8(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_i32immSExt8(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_i32immSExt8(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i64 && Predicate_i64immSExt8(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_i64immSExt8(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::ROTL: return fastEmit_ISD_ROTL_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::ROTR: return fastEmit_ISD_ROTR_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SHL: return fastEmit_ISD_SHL_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRA: return fastEmit_ISD_SRA_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRL: return fastEmit_ISD_SRL_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case X86ISD::BEXTR: return fastEmit_X86ISD_BEXTR_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case X86ISD::BT: return fastEmit_X86ISD_BT_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case X86ISD::PEXTRB: return fastEmit_X86ISD_PEXTRB_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case X86ISD::PEXTRW: return fastEmit_X86ISD_PEXTRW_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::ADD64ri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::AND64ri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::IMUL64rri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::OR64ri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::SUB64ri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SUB_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::XOR64ri32, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for X86ISD::BEXTR.

unsigned fastEmit_X86ISD_BEXTR_MVT_i64_ri_Predicate_i64immSExt32(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasTBM())) {
    return fastEmitInst_ri(X86::BEXTRI64ri, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_X86ISD_BEXTR_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_X86ISD_BEXTR_MVT_i64_ri_Predicate_i64immSExt32(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_i64immSExt32(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  case X86ISD::BEXTR: return fastEmit_X86ISD_BEXTR_ri_Predicate_i64immSExt32(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::ADD16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_ADD_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::AND16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_AND_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::IMUL16rri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_MUL_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::OR16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_OR_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::SUB16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_SUB_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i16_ri_Predicate_i16immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_ri(X86::XOR16ri8, &X86::GR16RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i16: return fastEmit_ISD_XOR_MVT_i16_ri_Predicate_i16immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_i16immSExt8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_i16immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::ADD32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::AND32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::IMUL32rri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::OR32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::SUB32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_ri_Predicate_i32immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(X86::XOR32ri8, &X86::GR32RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_ri_Predicate_i32immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_i32immSExt8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_i32immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::ADD64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::AND64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_AND_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::IMUL64rri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::OR64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_OR_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::SUB64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SUB_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SUB_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i64_ri_Predicate_i64immSExt8(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(X86::XOR64ri8, &X86::GR64RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_XOR_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_ri_Predicate_i64immSExt8(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_i64immSExt8(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::OR: return fastEmit_ISD_OR_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SUB: return fastEmit_ISD_SUB_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::XOR: return fastEmit_ISD_XOR_ri_Predicate_i64immSExt8(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::Constant.

unsigned fastEmit_ISD_Constant_MVT_i8_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i8)
    return 0;
  return fastEmitInst_i(X86::MOV8ri, &X86::GR8RegClass, imm0);
}

unsigned fastEmit_ISD_Constant_MVT_i16_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i16)
    return 0;
  return fastEmitInst_i(X86::MOV16ri, &X86::GR16RegClass, imm0);
}

unsigned fastEmit_ISD_Constant_i(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i8: return fastEmit_ISD_Constant_MVT_i8_i(RetVT, imm0);
  case MVT::i16: return fastEmit_ISD_Constant_MVT_i16_i(RetVT, imm0);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_i(MVT VT, MVT RetVT, unsigned Opcode, uint64_t imm0) override {
  switch (Opcode) {
  case ISD::Constant: return fastEmit_ISD_Constant_i(VT, RetVT, imm0);
  default: return 0;
  }
}

