/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* "Fast" Instruction Selector for the Mips target                            *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


// FastEmit Immediate Predicate functions.
static bool Predicate_immZExt5(int64_t Imm) {
return Imm == (Imm & 0x1f);
}
static bool Predicate_immZExt6(int64_t Imm) {
return Imm == (Imm & 0x3f);
}
static bool Predicate_immSExt6(int64_t Imm) {
return isInt<6>(Imm);
}
static bool Predicate_immZExt4Ptr(int64_t Imm) {
return isUInt<4>(Imm);
}
static bool Predicate_immZExt3Ptr(int64_t Imm) {
return isUInt<3>(Imm);
}
static bool Predicate_immZExt2Ptr(int64_t Imm) {
return isUInt<2>(Imm);
}
static bool Predicate_immZExt1Ptr(int64_t Imm) {
return isUInt<1>(Imm);
}
static bool Predicate_immZExt4(int64_t Imm) {
return isUInt<4>(Imm);
}
static bool Predicate_immSExtAddiur2(int64_t Imm) {
return Imm == 1 || Imm == -1 ||
                                           ((Imm % 4 == 0) &&
                                            Imm < 28 && Imm > 0);
}
static bool Predicate_immSExtAddius5(int64_t Imm) {
return Imm >= -8 && Imm <= 7;
}
static bool Predicate_immZExtAndi16(int64_t Imm) {
return (Imm == 128 || (Imm >= 1 && Imm <= 4) || Imm == 7 || Imm == 8 ||
            Imm == 15 || Imm == 16 || Imm == 31 || Imm == 32 || Imm == 63 ||
            Imm == 64 || Imm == 255 || Imm == 32768 || Imm == 65535 );
}
static bool Predicate_immZExt2Shift(int64_t Imm) {
return Imm >= 1 && Imm <= 8;
}


// FastEmit functions for ISD::BITCAST.

unsigned fastEmit_ISD_BITCAST_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::MTC1_MMR6, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::MTC1_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::MTC1, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::DMTC1, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::MFC1_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::MFC1_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::MFC1, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::DMFC1, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BITCAST_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_BITCAST_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::f32: return fastEmit_ISD_BITCAST_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BRIND.

unsigned fastEmit_ISD_BRIND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::JrcRx16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranch_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranch_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::PseudoIndrectHazardBranchR6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranchR6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::PseudoIndirectHazardBranch, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranch, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::PseudoIndrectHazardBranch64R6, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranch64R6, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isABI_N64()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::PseudoIndirectHazardBranch64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoIndirectBranch64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BRIND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_BRIND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_BRIND_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTLZ.

unsigned fastEmit_ISD_CTLZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::CLZ_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::CLZ_R6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::CLZ, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::DCLZ_R6, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips64()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isGP64bit()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::DCLZ, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::NLZC_B, &Mips::MSA128BRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::NLZC_H, &Mips::MSA128HRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::NLZC_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::NLZC_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTLZ_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTLZ_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_CTLZ_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_CTLZ_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_CTLZ_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_CTLZ_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTPOP.

unsigned fastEmit_ISD_CTPOP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasCnMips())) {
    return fastEmitInst_r(Mips::POP, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasCnMips())) {
    return fastEmitInst_r(Mips::DPOP, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::PCNT_B, &Mips::MSA128BRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::PCNT_H, &Mips::MSA128HRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::PCNT_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::PCNT_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTPOP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTPOP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_CTPOP_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_CTPOP_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_CTPOP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_CTPOP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FABS.

unsigned fastEmit_ISD_FABS_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (Subtarget->inAbs2008Mode() ||TM.Options.NoNaNsFPMath)) {
    return fastEmitInst_r(Mips::FABS_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode()) && (Subtarget->inAbs2008Mode() ||TM.Options.NoNaNsFPMath)) {
    return fastEmitInst_r(Mips::FABS_S, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FABS_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::FABS_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode()) && (Subtarget->inAbs2008Mode() ||TM.Options.NoNaNsFPMath)) {
    return fastEmitInst_r(Mips::FABS_D64, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode()) && (Subtarget->inAbs2008Mode() ||TM.Options.NoNaNsFPMath)) {
    return fastEmitInst_r(Mips::FABS_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FABS_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FABS_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FABS_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FABS_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FABS_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FABS_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FEXP2.

unsigned fastEmit_ISD_FEXP2_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FEXP2_W_1_PSEUDO, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FEXP2_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FEXP2_D_1_PSEUDO, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FEXP2_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FEXP2_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FEXP2_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FLOG2.

unsigned fastEmit_ISD_FLOG2_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FLOG2_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FLOG2_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FLOG2_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FLOG2_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FLOG2_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FLOG2_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEG.

unsigned fastEmit_ISD_FNEG_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FNEG_S_MMR6, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FNEG_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FNEG_S, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FNEG_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::FNEG_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::FNEG_D64, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::FNEG_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FNEG_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FNEG_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_EXTEND.

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMSA())) {
    return fastEmitInst_r(Mips::MSA_FP_EXTEND_W_PSEUDO, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMSA())) {
    return fastEmitInst_r(Mips::MSA_FP_EXTEND_D_PSEUDO, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_EXTEND_MVT_f16_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::CVT_D32_S_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::CVT_D64_S_MM, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::CVT_D64_S, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::CVT_D32_S, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_EXTEND_MVT_f16_r(RetVT, Op0, Op0IsKill);
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_ROUND.

unsigned fastEmit_ISD_FP_ROUND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f16)
    return 0;
  if ((Subtarget->hasMSA())) {
    return fastEmitInst_r(Mips::MSA_FP_ROUND_W_PSEUDO, &Mips::MSA128F16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f16_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMSA())) {
    return fastEmitInst_r(Mips::MSA_FP_ROUND_D_PSEUDO, &Mips::MSA128F16RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::CVT_S_D32_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::CVT_S_D64_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::CVT_S_D64, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::CVT_S_D32, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f16: return fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f16_r(Op0, Op0IsKill);
  case MVT::f32: return fastEmit_ISD_FP_ROUND_MVT_f64_MVT_f32_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_ROUND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_ROUND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_SINT.

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FTRUNC_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FTRUNC_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_UINT.

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FTRUNC_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FTRUNC_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FRINT.

unsigned fastEmit_ISD_FRINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FRINT_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FRINT_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FRINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FRINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FRINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSQRT.

unsigned fastEmit_ISD_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FSQRT_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::FSQRT_S, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_r(Mips::FSQRT_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::FSQRT_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::FSQRT_D64, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::FSQRT_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FSQRT_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FSQRT_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSQRT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FSQRT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSQRT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSQRT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SIGN_EXTEND.

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isGP64bit())) {
    return fastEmitInst_r(Mips::SLL64_32, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SIGN_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SINT_TO_FP.

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  return fastEmitInst_r(Mips::PseudoCVT_S_W, &Mips::FGR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::PseudoCVT_D64_W, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::PseudoCVT_D32_W, &Mips::AFGR64RegClass, Op0, Op0IsKill);
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

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::PseudoCVT_D64_L, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FFINT_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FFINT_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_SINT_TO_FP_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UINT_TO_FP.

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FFINT_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::FFINT_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::JmpLink.

unsigned fastEmit_MipsISD_JmpLink_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::JALR16_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::JumpLinkReg16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill);
  }
  if ((!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::JALRHBPseudo, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::JALRPseudo, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_JmpLink_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->isABI_N64()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::JALRHB64Pseudo, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->isABI_N64()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::JALR64Pseudo, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_JmpLink_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_JmpLink_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_MipsISD_JmpLink_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::MFHI.

unsigned fastEmit_MipsISD_MFHI_MVT_Untyped_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDSP()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::MFHI_DSP_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::PseudoMFHI_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_r(Mips::MFHI_DSP, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoMFHI, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MFHI_MVT_Untyped_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoMFHI64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MFHI_MVT_Untyped_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_MFHI_MVT_Untyped_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_MipsISD_MFHI_MVT_Untyped_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_MipsISD_MFHI_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::Untyped: return fastEmit_MipsISD_MFHI_MVT_Untyped_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::MFLO.

unsigned fastEmit_MipsISD_MFLO_MVT_Untyped_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasDSP()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::MFLO_DSP_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::PseudoMFLO_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_r(Mips::MFLO_DSP, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoMFLO, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MFLO_MVT_Untyped_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::PseudoMFLO64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MFLO_MVT_Untyped_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_MFLO_MVT_Untyped_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_MipsISD_MFLO_MVT_Untyped_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_MipsISD_MFLO_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::Untyped: return fastEmit_MipsISD_MFLO_MVT_Untyped_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::MTC1_D64.

unsigned fastEmit_MipsISD_MTC1_D64_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::MTC1_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::MTC1_D64, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MTC1_D64_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_MTC1_D64_MVT_i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::TailCall.

unsigned fastEmit_MipsISD_TailCall_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::TAILCALLREG_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::TAILCALLREG_MM, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::TAILCALLHBR6REG, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::TAILCALLR6REG, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::TAILCALLREGHB, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::TAILCALLREG, &Mips::GPR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TailCall_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::TAILCALLHB64R6REG, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_r(Mips::TAILCALL64R6REG, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips32r2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isABI_N64()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6()) && (Subtarget->useIndirectJumpsHazard())) {
    return fastEmitInst_r(Mips::TAILCALLREGHB64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isABI_N64()) && (!Subtarget->useIndirectJumpsHazard()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->inMips16Mode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_r(Mips::TAILCALLREG64, &Mips::GPR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TailCall_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_TailCall_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_MipsISD_TailCall_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::TruncIntFP.

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::TRUNC_W_S_MMR6, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::TRUNC_W_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasStandardEncoding())) {
    return fastEmitInst_r(Mips::TRUNC_W_S, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::TRUNC_L_S, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_MipsISD_TruncIntFP_MVT_f32_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_MipsISD_TruncIntFP_MVT_f32_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f64_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::TRUNC_W_D_MMR6, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::CVT_W_D64_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->isFP64bit()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_r(Mips::TRUNC_W_MM, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::TRUNC_W_D64, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_r(Mips::TRUNC_W_D32, &Mips::FGR32RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f64_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((Subtarget->hasMips2()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit())) {
    return fastEmitInst_r(Mips::TRUNC_L_D64, &Mips::FGR64RegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_TruncIntFP_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_MipsISD_TruncIntFP_MVT_f64_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_MipsISD_TruncIntFP_MVT_f64_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_MipsISD_TruncIntFP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_MipsISD_TruncIntFP_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_MipsISD_TruncIntFP_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::VALL_NONZERO.

unsigned fastEmit_MipsISD_VALL_NONZERO_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SNZ_B_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_NONZERO_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SNZ_H_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_NONZERO_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SNZ_W_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_NONZERO_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SNZ_D_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_NONZERO_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_VALL_NONZERO_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_VALL_NONZERO_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_VALL_NONZERO_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_VALL_NONZERO_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::VALL_ZERO.

unsigned fastEmit_MipsISD_VALL_ZERO_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SZ_B_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_ZERO_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SZ_H_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_ZERO_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SZ_W_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_ZERO_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SZ_D_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VALL_ZERO_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_VALL_ZERO_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_VALL_ZERO_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_VALL_ZERO_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_VALL_ZERO_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::VANY_NONZERO.

unsigned fastEmit_MipsISD_VANY_NONZERO_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SNZ_V_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VANY_NONZERO_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_VANY_NONZERO_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::VANY_ZERO.

unsigned fastEmit_MipsISD_VANY_ZERO_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(Mips::SZ_V_PSEUDO, &Mips::GPR32RegClass, Op0, Op0IsKill);
}

unsigned fastEmit_MipsISD_VANY_ZERO_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_VANY_ZERO_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_r(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill) override {
  switch (Opcode) {
  case ISD::BITCAST: return fastEmit_ISD_BITCAST_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::BRIND: return fastEmit_ISD_BRIND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTLZ: return fastEmit_ISD_CTLZ_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTPOP: return fastEmit_ISD_CTPOP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FABS: return fastEmit_ISD_FABS_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FEXP2: return fastEmit_ISD_FEXP2_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FLOG2: return fastEmit_ISD_FLOG2_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FNEG: return fastEmit_ISD_FNEG_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_EXTEND: return fastEmit_ISD_FP_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_ROUND: return fastEmit_ISD_FP_ROUND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_SINT: return fastEmit_ISD_FP_TO_SINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_UINT: return fastEmit_ISD_FP_TO_UINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FRINT: return fastEmit_ISD_FRINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FSQRT: return fastEmit_ISD_FSQRT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SIGN_EXTEND: return fastEmit_ISD_SIGN_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SINT_TO_FP: return fastEmit_ISD_SINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::UINT_TO_FP: return fastEmit_ISD_UINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::JmpLink: return fastEmit_MipsISD_JmpLink_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::MFHI: return fastEmit_MipsISD_MFHI_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::MFLO: return fastEmit_MipsISD_MFLO_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::MTC1_D64: return fastEmit_MipsISD_MTC1_D64_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::TailCall: return fastEmit_MipsISD_TailCall_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::TruncIntFP: return fastEmit_MipsISD_TruncIntFP_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::VALL_NONZERO: return fastEmit_MipsISD_VALL_NONZERO_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::VALL_ZERO: return fastEmit_MipsISD_VALL_ZERO_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::VANY_NONZERO: return fastEmit_MipsISD_VANY_NONZERO_r(VT, RetVT, Op0, Op0IsKill);
  case MipsISD::VANY_ZERO: return fastEmit_MipsISD_VANY_ZERO_r(VT, RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::ADDU16_MMR6, &Mips::GPRMM16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::AdduRxRyRz16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::ADDu_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::ADDu, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DADDu, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i8)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::ADDU_QB, &Mips::DSPRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ADDV_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::ADDQ_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ADDV_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ADDV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ADDV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i8: return fastEmit_ISD_ADD_MVT_v4i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_ADD_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i16: return fastEmit_ISD_ADD_MVT_v2i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_ADD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ADD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ADD_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADDC.

unsigned fastEmit_ISD_ADDC_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::ADDSC, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::ADDu, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADDC_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isGP64bit()) && (!Subtarget->hasDSP()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DADDu, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADDC_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADDC_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_ADDC_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADDE.

unsigned fastEmit_ISD_ADDE_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::ADDWC, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADDE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADDE_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::AndRxRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::AND_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::AND_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::AND, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->isGP64bit()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::AND64, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::AND_V, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::AND_V_H_PSEUDO, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::AND_V_W_PSEUDO, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::AND_V_D_PSEUDO, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_AND_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_AND_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_AND_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_AND_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FADD.

unsigned fastEmit_ISD_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FADD_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FADD_S, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FADD_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_rr(Mips::FADD_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FADD_D64, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FADD_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FADD_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FADD_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FADD_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FADD_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FADD_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FADD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FDIV.

unsigned fastEmit_ISD_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FDIV_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FDIV_S, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FDIV_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_rr(Mips::FDIV_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FDIV_D64, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FDIV_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FDIV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FDIV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FDIV_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FDIV_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FDIV_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FDIV_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMUL.

unsigned fastEmit_ISD_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FMUL_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FMUL_S, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FMUL_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_rr(Mips::FMUL_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FMUL_D64, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FMUL_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FMUL_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FMUL_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FMUL_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FMUL_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FMUL_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FMUL_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSUB.

unsigned fastEmit_ISD_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FSUB_S_MM, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FSUB_S, &Mips::FGR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat())) {
    return fastEmitInst_rr(Mips::FSUB_D64_MM, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit())) {
    return fastEmitInst_rr(Mips::FSUB_D32_MM, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FSUB_D64, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::FSUB_D32, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FSUB_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::FSUB_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSUB_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FSUB_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::MultRxRyRz16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUL_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::MUL_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUL_R6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::MUL, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DMUL_R6, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasCnMips())) {
    return fastEmitInst_rr(Mips::DMUL, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MULV_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v2i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSPR2())) {
    return fastEmitInst_rr(Mips::MUL_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MULV_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MULV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MULV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_MUL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i16: return fastEmit_ISD_MUL_MVT_v2i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_MUL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_MUL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_MUL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHS.

unsigned fastEmit_ISD_MULHS_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUH_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUH, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DMUH, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHS_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MULHS_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MULHS_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHU.

unsigned fastEmit_ISD_MULHU_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUHU_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MUHU, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DMUHU, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MULHU_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MULHU_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MULHU_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::OrRxRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::OR_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::OR_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::OR, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->isGP64bit()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::OR64, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::OR_V, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::OR_V_H_PSEUDO, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::OR_V_W_PSEUDO, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::OR_V_D_PSEUDO, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_OR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_OR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_OR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_OR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::ROTRV_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::ROTRV, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SDIV.

unsigned fastEmit_ISD_SDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DIV_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DIV, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DDIV, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_S_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_S_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SDIV_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SDIV_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SDIV_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SDIV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SDIV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SDIV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SLLV_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::SllvRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SLLV, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SLL_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SLL_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SLL_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SLL_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SHL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SHL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SHL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SHL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMAX.

unsigned fastEmit_ISD_SMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_S_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_S_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SMAX_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_S_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_S_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SMIN_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SRAV_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::SravRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SRAV, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRA_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRA_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRA_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRA_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRA_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SRA_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SRA_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SRA_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SRA_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SREM.

unsigned fastEmit_ISD_SREM_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MOD_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MOD, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DMOD, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_S_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_S_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_S_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_S_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SREM_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SREM_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SREM_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SREM_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SREM_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SREM_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SRLV_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::SrlvRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SRLV, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRL_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRL_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRL_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SRL_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SRL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SRL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SRL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SRL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUB.

unsigned fastEmit_ISD_SUB_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SUBU16_MMR6, &Mips::GPRMM16RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::SubuRxRyRz16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::SUBu_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SUBu, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DSUBu, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i8)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::SUBU_QB, &Mips::DSPRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SUBV_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::SUBQ_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SUBV_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SUBV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::SUBV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUB_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SUB_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i8: return fastEmit_ISD_SUB_MVT_v4i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SUB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i16: return fastEmit_ISD_SUB_MVT_v2i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SUB_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SUB_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SUB_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SUBC.

unsigned fastEmit_ISD_SUBC_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SUBU_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SUBu_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::SUBu, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUBC_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (Subtarget->isGP64bit()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DSUBu, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUBC_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SUBC_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SUBC_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UDIV.

unsigned fastEmit_ISD_UDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DIVU_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DIVU, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DDIVU, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_U_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_U_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::DIV_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UDIV_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_UDIV_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_UDIV_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UDIV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UDIV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UDIV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_U_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_U_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MAX_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UMAX_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_U_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_U_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MIN_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UMIN_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UREM.

unsigned fastEmit_ISD_UREM_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MODU_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MODU, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r6()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::DMODU, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_U_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_U_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_U_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::MOD_U_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UREM_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_UREM_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_UREM_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UREM_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UREM_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_UREM_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::XorRxRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::XOR_MMR6, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::XOR_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::XOR, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->isGP64bit()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::XOR64, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::XOR_V, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::XOR_V_H_PSEUDO, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::XOR_V_W_PSEUDO, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::XOR_V_D_PSEUDO, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_XOR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_XOR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_XOR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_XOR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::BuildPairF64.

unsigned fastEmit_MipsISD_BuildPairF64_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::BuildPairF64_64, &Mips::FGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::BuildPairF64, &Mips::AFGR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_BuildPairF64_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_BuildPairF64_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::DivRem.

unsigned fastEmit_MipsISD_DivRem_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::SDIV_MM_Pseudo, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoSDIV, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRem_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoDSDIV, &Mips::ACC128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRem_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_DivRem_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_DivRem_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::DivRem16.

unsigned fastEmit_MipsISD_DivRem16_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::DivRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRem16_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_DivRem16_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::DivRemU.

unsigned fastEmit_MipsISD_DivRemU_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::UDIV_MM_Pseudo, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoUDIV, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRemU_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoDUDIV, &Mips::ACC128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRemU_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_DivRemU_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_DivRemU_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::DivRemU16.

unsigned fastEmit_MipsISD_DivRemU16_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::DivuRxRy16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_DivRemU16_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_DivRemU16_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::EH_RETURN.

unsigned fastEmit_MipsISD_EH_RETURN_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_rr(Mips::MIPSeh_return32, &Mips::GPR32RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_MipsISD_EH_RETURN_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_rr(Mips::MIPSeh_return64, &Mips::GPR64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_MipsISD_EH_RETURN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_EH_RETURN_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_EH_RETURN_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::ILVEV.

unsigned fastEmit_MipsISD_ILVEV_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVEV_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVEV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVEV_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVEV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVEV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVEV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVEV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVEV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_ILVEV_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_ILVEV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_ILVEV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_ILVEV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::ILVL.

unsigned fastEmit_MipsISD_ILVL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVL_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVL_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVL_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVL_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_ILVL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_ILVL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_ILVL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_ILVL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::ILVOD.

unsigned fastEmit_MipsISD_ILVOD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVOD_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVOD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVOD_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVOD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVOD_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVOD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVOD_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVOD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_ILVOD_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_ILVOD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_ILVOD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_ILVOD_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::ILVR.

unsigned fastEmit_MipsISD_ILVR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVR_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVR_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVR_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::ILVR_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ILVR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_ILVR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_ILVR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_ILVR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_ILVR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::MTLOHI.

unsigned fastEmit_MipsISD_MTLOHI_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::PseudoMTLOHI_MM, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasDSP()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_rr(Mips::PseudoMTLOHI_DSP, &Mips::ACC64DSPRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoMTLOHI, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MTLOHI_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoMTLOHI64, &Mips::ACC128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_MTLOHI_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_MTLOHI_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_MTLOHI_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::Mult.

unsigned fastEmit_MipsISD_Mult_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MULT_DSP_MM, &Mips::ACC64DSPRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::PseudoMULT_MM, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::MULT_DSP, &Mips::ACC64DSPRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoMULT, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_Mult_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoDMULT, &Mips::ACC128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_Mult_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_Mult_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_Mult_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::Multu.

unsigned fastEmit_MipsISD_Multu_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasDSP()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_rr(Mips::MULTU_DSP_MM, &Mips::ACC64DSPRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->inMicroMipsMode()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6())) {
    return fastEmitInst_rr(Mips::PseudoMULTu_MM, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_rr(Mips::MULTU_DSP, &Mips::ACC64DSPRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->hasDSP()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoMULTu, &Mips::ACC64RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_Multu_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::Untyped)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->hasMips32r6()) && (!Subtarget->hasMips64r6())) {
    return fastEmitInst_rr(Mips::PseudoDMULTu, &Mips::ACC128RegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_Multu_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_MipsISD_Multu_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_MipsISD_Multu_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::PCKEV.

unsigned fastEmit_MipsISD_PCKEV_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKEV_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKEV_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKEV_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKEV_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKEV_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKEV_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKEV_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKEV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_PCKEV_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_PCKEV_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_PCKEV_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_PCKEV_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::PCKOD.

unsigned fastEmit_MipsISD_PCKOD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKOD_B, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKOD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKOD_H, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKOD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKOD_W, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKOD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::PCKOD_D, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_PCKOD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_PCKOD_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_PCKOD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_PCKOD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_PCKOD_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::VNOR.

unsigned fastEmit_MipsISD_VNOR_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::NOR_V, &Mips::MSA128BRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_VNOR_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::NOR_V_H_PSEUDO, &Mips::MSA128HRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_VNOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::NOR_V_W_PSEUDO, &Mips::MSA128WRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_VNOR_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_rr(Mips::NOR_V_D_PSEUDO, &Mips::MSA128DRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_MipsISD_VNOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_MipsISD_VNOR_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_MipsISD_VNOR_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_MipsISD_VNOR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_MipsISD_VNOR_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_rr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) override {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ADDC: return fastEmit_ISD_ADDC_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ADDE: return fastEmit_ISD_ADDE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::AND: return fastEmit_ISD_AND_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FADD: return fastEmit_ISD_FADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FDIV: return fastEmit_ISD_FDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMUL: return fastEmit_ISD_FMUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FSUB: return fastEmit_ISD_FSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MUL: return fastEmit_ISD_MUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHS: return fastEmit_ISD_MULHS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHU: return fastEmit_ISD_MULHU_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::OR: return fastEmit_ISD_OR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ROTR: return fastEmit_ISD_ROTR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SDIV: return fastEmit_ISD_SDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SHL: return fastEmit_ISD_SHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMAX: return fastEmit_ISD_SMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMIN: return fastEmit_ISD_SMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRA: return fastEmit_ISD_SRA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SREM: return fastEmit_ISD_SREM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRL: return fastEmit_ISD_SRL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SUB: return fastEmit_ISD_SUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SUBC: return fastEmit_ISD_SUBC_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UDIV: return fastEmit_ISD_UDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMAX: return fastEmit_ISD_UMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMIN: return fastEmit_ISD_UMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UREM: return fastEmit_ISD_UREM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::XOR: return fastEmit_ISD_XOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::BuildPairF64: return fastEmit_MipsISD_BuildPairF64_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::DivRem: return fastEmit_MipsISD_DivRem_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::DivRem16: return fastEmit_MipsISD_DivRem16_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::DivRemU: return fastEmit_MipsISD_DivRemU_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::DivRemU16: return fastEmit_MipsISD_DivRemU16_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::EH_RETURN: return fastEmit_MipsISD_EH_RETURN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::ILVEV: return fastEmit_MipsISD_ILVEV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::ILVL: return fastEmit_MipsISD_ILVL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::ILVOD: return fastEmit_MipsISD_ILVOD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::ILVR: return fastEmit_MipsISD_ILVR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::MTLOHI: return fastEmit_MipsISD_MTLOHI_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::Mult: return fastEmit_MipsISD_Mult_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::Multu: return fastEmit_MipsISD_Multu_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::PCKEV: return fastEmit_MipsISD_PCKEV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::PCKOD: return fastEmit_MipsISD_PCKOD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MipsISD::VNOR: return fastEmit_MipsISD_VNOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::ExtractElementF64.

unsigned fastEmit_MipsISD_ExtractElementF64_MVT_f64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->isFP64bit()) && (!Subtarget->useSoftFloat()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_ri(Mips::ExtractElementF64_64, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((!Subtarget->useSoftFloat()) && (!Subtarget->isFP64bit()) && (!Subtarget->inMips16Mode())) {
    return fastEmitInst_ri(Mips::ExtractElementF64, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_ExtractElementF64_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_MipsISD_ExtractElementF64_MVT_f64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::SHLL_DSP.

unsigned fastEmit_MipsISD_SHLL_DSP_MVT_v4i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i8)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_ri(Mips::SHLL_QB, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHLL_DSP_MVT_v2i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_ri(Mips::SHLL_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHLL_DSP_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i8: return fastEmit_MipsISD_SHLL_DSP_MVT_v4i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v2i16: return fastEmit_MipsISD_SHLL_DSP_MVT_v2i16_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::SHRA_DSP.

unsigned fastEmit_MipsISD_SHRA_DSP_MVT_v4i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i8)
    return 0;
  if ((Subtarget->hasDSPR2())) {
    return fastEmitInst_ri(Mips::SHRA_QB, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHRA_DSP_MVT_v2i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_ri(Mips::SHRA_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHRA_DSP_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i8: return fastEmit_MipsISD_SHRA_DSP_MVT_v4i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v2i16: return fastEmit_MipsISD_SHRA_DSP_MVT_v2i16_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for MipsISD::SHRL_DSP.

unsigned fastEmit_MipsISD_SHRL_DSP_MVT_v4i8_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i8)
    return 0;
  if ((Subtarget->hasDSP())) {
    return fastEmitInst_ri(Mips::SHRL_QB, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHRL_DSP_MVT_v2i16_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v2i16)
    return 0;
  if ((Subtarget->hasDSPR2())) {
    return fastEmitInst_ri(Mips::SHRL_PH, &Mips::DSPRRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_MipsISD_SHRL_DSP_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i8: return fastEmit_MipsISD_SHRL_DSP_MVT_v4i8_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v2i16: return fastEmit_MipsISD_SHRL_DSP_MVT_v2i16_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) override {
  if (VT == MVT::i32 && Predicate_immZExt5(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt5(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immZExt6(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt6(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::iPTR && Predicate_immZExt2Ptr(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt2Ptr(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::iPTR && Predicate_immZExt1Ptr(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt1Ptr(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immZExt4(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt4(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immSExtAddiur2(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immSExtAddiur2(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immSExtAddius5(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immSExtAddius5(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immZExtAndi16(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExtAndi16(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i32 && Predicate_immZExt2Shift(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_immZExt2Shift(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  switch (Opcode) {
  case MipsISD::ExtractElementF64: return fastEmit_MipsISD_ExtractElementF64_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case MipsISD::SHLL_DSP: return fastEmit_MipsISD_SHLL_DSP_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case MipsISD::SHRA_DSP: return fastEmit_MipsISD_SHRA_DSP_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case MipsISD::SHRL_DSP: return fastEmit_MipsISD_SHRL_DSP_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i32_ri_Predicate_immZExt5(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ROTR_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasMips32r2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ROTR, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_ri_Predicate_immZExt5(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ROTR_MVT_i32_ri_Predicate_immZExt5(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_ri_Predicate_immZExt5(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SLL_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_ri(Mips::SllX16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SLL, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_ri_Predicate_immZExt5(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_ri_Predicate_immZExt5(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i32_ri_Predicate_immZExt5(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SRA_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_ri(Mips::SraX16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SRA, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_ri_Predicate_immZExt5(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRA_MVT_i32_ri_Predicate_immZExt5(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i32_ri_Predicate_immZExt5(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SRL_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_ri(Mips::SrlX16, &Mips::CPU16RegsRegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SRL, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_ri_Predicate_immZExt5(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRL_MVT_i32_ri_Predicate_immZExt5(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt5(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ROTR: return fastEmit_ISD_ROTR_ri_Predicate_immZExt5(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SHL: return fastEmit_ISD_SHL_ri_Predicate_immZExt5(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRA: return fastEmit_ISD_SRA_ri_Predicate_immZExt5(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRL: return fastEmit_ISD_SRL_ri_Predicate_immZExt5(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTR.

unsigned fastEmit_ISD_ROTR_MVT_i64_ri_Predicate_immZExt6(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips64r2()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::DROTR, &Mips::GPR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTR_ri_Predicate_immZExt6(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ROTR_MVT_i64_ri_Predicate_immZExt6(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i64_ri_Predicate_immZExt6(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::DSLL, &Mips::GPR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_ri_Predicate_immZExt6(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SHL_MVT_i64_ri_Predicate_immZExt6(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i64_ri_Predicate_immZExt6(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::DSRA, &Mips::GPR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_ri_Predicate_immZExt6(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SRA_MVT_i64_ri_Predicate_immZExt6(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i64_ri_Predicate_immZExt6(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((Subtarget->hasMips3()) && (Subtarget->hasStandardEncoding()) && (!Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::DSRL, &Mips::GPR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_ri_Predicate_immZExt6(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_SRL_MVT_i64_ri_Predicate_immZExt6(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt6(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ROTR: return fastEmit_ISD_ROTR_ri_Predicate_immZExt6(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SHL: return fastEmit_ISD_SHL_ri_Predicate_immZExt6(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRA: return fastEmit_ISD_SRA_ri_Predicate_immZExt6(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRL: return fastEmit_ISD_SRL_ri_Predicate_immZExt6(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4f32_ri_Predicate_immZExt2Ptr(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_ri(Mips::COPY_FW_PSEUDO, &Mips::FGR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt2Ptr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4f32_ri_Predicate_immZExt2Ptr(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt2Ptr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt2Ptr(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2f64_ri_Predicate_immZExt1Ptr(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((Subtarget->hasMSA()) && (Subtarget->hasStandardEncoding())) {
    return fastEmitInst_ri(Mips::COPY_FD_PSEUDO, &Mips::FGR64RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt1Ptr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v2f64: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v2f64_ri_Predicate_immZExt1Ptr(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt1Ptr(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt1Ptr(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::EXTRACT_VECTOR_ELT.

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_immZExt4(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMSA())) {
    return fastEmitInst_ri(Mips::COPY_S_W, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt4(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_EXTRACT_VECTOR_ELT_MVT_v4i32_ri_Predicate_immZExt4(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt4(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::EXTRACT_VECTOR_ELT: return fastEmit_ISD_EXTRACT_VECTOR_ELT_ri_Predicate_immZExt4(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_immSExtAddiur2(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ADDIUR2_MM, &Mips::GPRMM16RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_immSExtAddiur2(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_immSExtAddiur2(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immSExtAddiur2(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_immSExtAddiur2(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_immSExtAddius5(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ADDIUS5_MM, &Mips::GPR32RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_ri_Predicate_immSExtAddius5(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_immSExtAddius5(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immSExtAddius5(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_immSExtAddius5(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i32_ri_Predicate_immZExtAndi16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->hasMips32r6()) && (Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ANDI16_MMR6, &Mips::GPRMM16RegClass, Op0, Op0IsKill, imm1);
  }
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::ANDI16_MM, &Mips::GPRMM16RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_ri_Predicate_immZExtAndi16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_ri_Predicate_immZExtAndi16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExtAndi16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::AND: return fastEmit_ISD_AND_ri_Predicate_immZExtAndi16(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_ri_Predicate_immZExt2Shift(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SLL16_MM, &Mips::GPRMM16RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_ri_Predicate_immZExt2Shift(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SHL_MVT_i32_ri_Predicate_immZExt2Shift(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i32_ri_Predicate_immZExt2Shift(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMicroMipsMode())) {
    return fastEmitInst_ri(Mips::SRL16_MM, &Mips::GPRMM16RegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_ri_Predicate_immZExt2Shift(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRL_MVT_i32_ri_Predicate_immZExt2Shift(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_immZExt2Shift(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::SHL: return fastEmit_ISD_SHL_ri_Predicate_immZExt2Shift(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::SRL: return fastEmit_ISD_SRL_ri_Predicate_immZExt2Shift(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::Constant.

unsigned fastEmit_ISD_Constant_MVT_i32_i(MVT RetVT, uint64_t imm0) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((Subtarget->inMips16Mode())) {
    return fastEmitInst_i(Mips::LwConstant32, &Mips::CPU16RegsRegClass, imm0);
  }
  return 0;
}

unsigned fastEmit_ISD_Constant_i(MVT VT, MVT RetVT, uint64_t imm0) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_Constant_MVT_i32_i(RetVT, imm0);
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

