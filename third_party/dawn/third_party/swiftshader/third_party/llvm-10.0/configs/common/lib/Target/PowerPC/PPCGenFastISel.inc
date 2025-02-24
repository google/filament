/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* "Fast" Instruction Selector for the PPC target                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


// FastEmit Immediate Predicate functions.
static bool Predicate_imm32SExt16(int64_t Imm) {

  // imm32SExt16 predicate - True if the i32 immediate fits in a 16-bit
  // sign extended field.  Used by instructions like 'addi'.
  return (int32_t)Imm == (short)Imm;

}
static bool Predicate_imm64SExt16(int64_t Imm) {

  // imm64SExt16 predicate - True if the i64 immediate fits in a 16-bit
  // sign extended field.  Used by instructions like 'addi'.
  return (int64_t)Imm == (short)Imm;

}
static bool Predicate_immSExt5NonZero(int64_t Imm) {
 return Imm && isInt<5>(Imm); 
}


// FastEmit functions for ISD::READCYCLECOUNTER.

unsigned fastEmit_ISD_READCYCLECOUNTER_MVT_i64_(MVT RetVT) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_(PPC::MFTB8, &PPC::G8RCRegClass);
}

unsigned fastEmit_ISD_READCYCLECOUNTER_(MVT VT, MVT RetVT) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_READCYCLECOUNTER_MVT_i64_(RetVT);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::MFFS.

unsigned fastEmit_PPCISD_MFFS_MVT_f64_(MVT RetVT) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_(PPC::MFFS, &PPC::F8RCRegClass);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MFFS_(MVT VT, MVT RetVT) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_MFFS_MVT_f64_(RetVT);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::PPC32_GOT.

unsigned fastEmit_PPCISD_PPC32_GOT_MVT_i32_(MVT RetVT) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_(PPC::PPC32GOT, &PPC::GPRCRegClass);
}

unsigned fastEmit_PPCISD_PPC32_GOT_(MVT VT, MVT RetVT) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_PPC32_GOT_MVT_i32_(RetVT);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_(MVT VT, MVT RetVT, unsigned Opcode) override {
  switch (Opcode) {
  case ISD::READCYCLECOUNTER: return fastEmit_ISD_READCYCLECOUNTER_(VT, RetVT);
  case PPCISD::MFFS: return fastEmit_PPCISD_MFFS_(VT, RetVT);
  case PPCISD::PPC32_GOT: return fastEmit_PPCISD_PPC32_GOT_(VT, RetVT);
  default: return 0;
  }
}

// FastEmit functions for ISD::BITCAST.

unsigned fastEmit_ISD_BITCAST_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasDirectMove()) && (PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::MTVSRD, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((PPCSubTarget->hasDirectMove()) && (PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::MFVSRD, &PPC::G8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BITCAST_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_BITCAST_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_BITCAST_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BSWAP.

unsigned fastEmit_ISD_BSWAP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XXBRW, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BSWAP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XXBRD, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BSWAP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_ISD_BSWAP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_BSWAP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::CTLZ.

unsigned fastEmit_ISD_CTLZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_r(PPC::CNTLZW, &PPC::GPRCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTLZ_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(PPC::CNTLZD, &PPC::G8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTLZ_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VCLZB, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VCLZH, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VCLZW, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTLZ_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VCLZD, &PPC::VRRCRegClass, Op0, Op0IsKill);
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
  return fastEmitInst_r(PPC::POPCNTW, &PPC::GPRCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTPOP_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(PPC::POPCNTD, &PPC::G8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_CTPOP_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VPOPCNTB, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VPOPCNTH, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VPOPCNTW, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTPOP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_r(PPC::VPOPCNTD, &PPC::VRRCRegClass, Op0, Op0IsKill);
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

// FastEmit functions for ISD::CTTZ.

unsigned fastEmit_ISD_CTTZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_r(PPC::CNTTZW, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_r(PPC::CNTTZD, &PPC::G8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VCTZB, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VCTZH, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VCTZW, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VCTZD, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_CTTZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_CTTZ_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_CTTZ_MVT_i64_r(RetVT, Op0, Op0IsKill);
  case MVT::v16i8: return fastEmit_ISD_CTTZ_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_ISD_CTTZ_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_CTTZ_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_CTTZ_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FABS.

unsigned fastEmit_ISD_FABS_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSABS, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FABSS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSABSDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDABS, &PPC::SPERCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FABSD, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_f128_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XSABSQP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVABSSP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFABSs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVABSDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFABS, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FABS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FABS_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FABS_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f128: return fastEmit_ISD_FABS_MVT_f128_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FABS_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FABS_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FABS_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FCEIL.

unsigned fastEmit_ISD_FCEIL_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIPS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRDPIP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIPD, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSPIP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VRFIP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIPs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRDPIP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIP, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FCEIL_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FCEIL_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FCEIL_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FCEIL_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FCEIL_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FCEIL_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FFLOOR.

unsigned fastEmit_ISD_FFLOOR_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIMS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRDPIM, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIMD, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSPIM, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VRFIM, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIMs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRDPIM, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIM, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FFLOOR_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FFLOOR_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FFLOOR_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FFLOOR_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FFLOOR_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FFLOOR_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEARBYINT.

unsigned fastEmit_ISD_FNEARBYINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRDPIC, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSPIC, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VRFIN, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRDPIC, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEARBYINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FNEARBYINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FNEARBYINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FNEARBYINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FNEG.

unsigned fastEmit_ISD_FNEG_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSNEG, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FNEGS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSNEGDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDNEG, &PPC::SPERCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FNEGD, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_f128_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XSNEGQP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVNEGSP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFNEGs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVNEGDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFNEG, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FNEG_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FNEG_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FNEG_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f128: return fastEmit_ISD_FNEG_MVT_f128_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FNEG_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FNEG_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FNEG_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_EXTEND.

unsigned fastEmit_ISD_FP_EXTEND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDCFS, &PPC::SPERCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XSCVDPQP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_EXTEND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_EXTEND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_ROUND.

unsigned fastEmit_ISD_FP_ROUND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSCFD, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRSP, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_MVT_f128_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XSCVQPDP, &PPC::VFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_ROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FP_ROUND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f128: return fastEmit_ISD_FP_ROUND_MVT_f128_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_SINT.

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSCTSIZ, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDCTSIZ, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVSPSXWS, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VCTSXS_0, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVDPSXDS, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_SINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_TO_SINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_TO_SINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_SINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_SINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FP_TO_UINT.

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSCTUIZ, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDCTUIZ, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVSPUXWS, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VCTUXS_0, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVDPUXDS, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FP_TO_UINT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FP_TO_UINT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FP_TO_UINT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FP_TO_UINT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FP_TO_UINT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FROUND.

unsigned fastEmit_ISD_FROUND_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRINS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRDPI, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIND, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSPI, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRINs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRDPI, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIN, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FROUND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FROUND_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FROUND_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FROUND_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FROUND_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FROUND_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSQRT.

unsigned fastEmit_ISD_FSQRT_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_r(PPC::XSSQRTSP, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FSQRTS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSSQRTDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FSQRT, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_f128_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::XSSQRTQP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVSQRTSP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVSQRTDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSQRT_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSQRT_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FSQRT_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::f128: return fastEmit_ISD_FSQRT_MVT_f128_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSQRT_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSQRT_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FTRUNC.

unsigned fastEmit_ISD_FTRUNC_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIZS, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRDPIZ, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRIZD, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSPIZ, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VRFIZ, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIZs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRDPIZ, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRIZ, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FTRUNC_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FTRUNC_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_FTRUNC_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_ISD_FTRUNC_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_ISD_FTRUNC_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_ISD_FTRUNC_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SCALAR_TO_VECTOR.

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_r(PPC::MTVSRWS, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector()) && (!PPCSubTarget->isLittleEndian())) {
    return fastEmitInst_r(PPC::XSCVDPSPN, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SCALAR_TO_VECTOR_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::f32: return fastEmit_ISD_SCALAR_TO_VECTOR_MVT_f32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SIGN_EXTEND.

unsigned fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_r(PPC::EXTSW_32_64, &PPC::G8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_SIGN_EXTEND_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SIGN_EXTEND_MVT_i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SINT_TO_FP.

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSCFSI, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDCFSI, &PPC::SPERCRegClass, Op0, Op0IsKill);
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

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVSXWSP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VCFSX_0, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVSXDDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SINT_TO_FP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_SINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_SINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::TRUNCATE.

unsigned fastEmit_ISD_TRUNCATE_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_r(PPC::ANDI_rec_1_GT_BIT, &PPC::CRBITRCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_TRUNCATE_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_r(PPC::ANDI_rec_1_GT_BIT8, &PPC::CRBITRCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_ISD_TRUNCATE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_TRUNCATE_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_ISD_TRUNCATE_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UINT_TO_FP.

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f32_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFSCFUI, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f64_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_r(PPC::EFDCFUI, &PPC::SPERCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f32_r(Op0, Op0IsKill);
  case MVT::f64: return fastEmit_ISD_UINT_TO_FP_MVT_i32_MVT_f64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVUXWSP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VCFUX_0, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVCVUXDDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UINT_TO_FP_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UINT_TO_FP_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_ISD_UINT_TO_FP_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_ISD_UINT_TO_FP_MVT_v2i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCFID.

unsigned fastEmit_PPCISD_FCFID_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVSXDDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCFID, &PPC::F8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCFID_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_FCFID_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCFIDS.

unsigned fastEmit_PPCISD_FCFIDS_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_r(PPC::XSCVSXDSP, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCFIDS, &PPC::F4RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCFIDS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_FCFIDS_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCFIDU.

unsigned fastEmit_PPCISD_FCFIDU_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVUXDDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCFIDU, &PPC::F8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCFIDU_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_FCFIDU_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCFIDUS.

unsigned fastEmit_PPCISD_FCFIDUS_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_r(PPC::XSCVUXDSP, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCFIDUS, &PPC::F4RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCFIDUS_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_FCFIDUS_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCTIDUZ.

unsigned fastEmit_PPCISD_FCTIDUZ_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPUXDSs, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FCTIDUZ_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPUXDS, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCTIDUZ, &PPC::F8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCTIDUZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FCTIDUZ_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FCTIDUZ_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCTIDZ.

unsigned fastEmit_PPCISD_FCTIDZ_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPSXDSs, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FCTIDZ_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPSXDS, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCTIDZ, &PPC::F8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCTIDZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FCTIDZ_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FCTIDZ_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCTIWUZ.

unsigned fastEmit_PPCISD_FCTIWUZ_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPUXWSs, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FCTIWUZ_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPUXWS, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return fastEmitInst_r(PPC::FCTIWUZ, &PPC::F8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_FCTIWUZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FCTIWUZ_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FCTIWUZ_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FCTIWZ.

unsigned fastEmit_PPCISD_FCTIWZ_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPSXWSs, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FCTIWZ_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSCVDPSXWS, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FCTIWZ, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FCTIWZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FCTIWZ_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FCTIWZ_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FRE.

unsigned fastEmit_PPCISD_FRE_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_r(PPC::XSRESP, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRES, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRE_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSREDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRE, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRE_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRESP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VREFP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRESs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRE_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVREDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRE_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRE, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FRE_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FRE_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_PPCISD_FRE_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_PPCISD_FRE_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_PPCISD_FRE_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FRSQRTE.

unsigned fastEmit_PPCISD_FRSQRTE_MVT_f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_r(PPC::XSRSQRTESP, &PPC::VSSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRSQRTES, &PPC::F4RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRSQRTE_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XSRSQRTEDP, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_r(PPC::FRSQRTE, &PPC::F8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRSQRTE_MVT_v4f32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSQRTESP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_r(PPC::VRSQRTEFP, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRSQRTESs, &PPC::QSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRSQRTE_MVT_v2f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_r(PPC::XVRSQRTEDP, &PPC::VSRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRSQRTE_MVT_v4f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_r(PPC::QVFRSQRTE, &PPC::QFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FRSQRTE_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_PPCISD_FRSQRTE_MVT_f32_r(RetVT, Op0, Op0IsKill);
  case MVT::f64: return fastEmit_PPCISD_FRSQRTE_MVT_f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f32: return fastEmit_PPCISD_FRSQRTE_MVT_v4f32_r(RetVT, Op0, Op0IsKill);
  case MVT::v2f64: return fastEmit_PPCISD_FRSQRTE_MVT_v2f64_r(RetVT, Op0, Op0IsKill);
  case MVT::v4f64: return fastEmit_PPCISD_FRSQRTE_MVT_v4f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::MFVSR.

unsigned fastEmit_PPCISD_MFVSR_MVT_f64_MVT_i32_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasDirectMove())) {
    return fastEmitInst_r(PPC::MFVSRWZ, &PPC::GPRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MFVSR_MVT_f64_MVT_i64_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasDirectMove())) {
    return fastEmitInst_r(PPC::MFVSRD, &PPC::G8RCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MFVSR_MVT_f64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_MFVSR_MVT_f64_MVT_i32_r(Op0, Op0IsKill);
  case MVT::i64: return fastEmit_PPCISD_MFVSR_MVT_f64_MVT_i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_PPCISD_MFVSR_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_MFVSR_MVT_f64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::MTCTR.

unsigned fastEmit_PPCISD_MTCTR_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(PPC::MTCTR, &PPC::GPRCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_MTCTR_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_r(PPC::MTCTR8, &PPC::G8RCRegClass, Op0, Op0IsKill);
}

unsigned fastEmit_PPCISD_MTCTR_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_MTCTR_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_PPCISD_MTCTR_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::MTVSRA.

unsigned fastEmit_PPCISD_MTVSRA_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasDirectMove())) {
    return fastEmitInst_r(PPC::MTVSRWA, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MTVSRA_MVT_i64_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasDirectMove())) {
    return fastEmitInst_r(PPC::MTVSRD, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MTVSRA_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_MTVSRA_MVT_i32_r(RetVT, Op0, Op0IsKill);
  case MVT::i64: return fastEmit_PPCISD_MTVSRA_MVT_i64_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::MTVSRZ.

unsigned fastEmit_PPCISD_MTVSRZ_MVT_i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasDirectMove())) {
    return fastEmitInst_r(PPC::MTVSRWZ, &PPC::VSFRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_MTVSRZ_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_MTVSRZ_MVT_i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::SExtVElems.

unsigned fastEmit_PPCISD_SExtVElems_MVT_v16i8_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VEXTSB2W, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v16i8_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VEXTSB2D, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v16i8_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_PPCISD_SExtVElems_MVT_v16i8_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_PPCISD_SExtVElems_MVT_v16i8_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v8i16_MVT_v4i32_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VEXTSH2W, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v8i16_MVT_v2i64_r(unsigned Op0, bool Op0IsKill) {
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VEXTSH2D, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v8i16_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
switch (RetVT.SimpleTy) {
  case MVT::v4i32: return fastEmit_PPCISD_SExtVElems_MVT_v8i16_MVT_v4i32_r(Op0, Op0IsKill);
  case MVT::v2i64: return fastEmit_PPCISD_SExtVElems_MVT_v8i16_MVT_v2i64_r(Op0, Op0IsKill);
  default: return 0;
}
}

unsigned fastEmit_PPCISD_SExtVElems_MVT_v4i32_r(MVT RetVT, unsigned Op0, bool Op0IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP9Altivec())) {
    return fastEmitInst_r(PPC::VEXTSW2D, &PPC::VRRCRegClass, Op0, Op0IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SExtVElems_r(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_PPCISD_SExtVElems_MVT_v16i8_r(RetVT, Op0, Op0IsKill);
  case MVT::v8i16: return fastEmit_PPCISD_SExtVElems_MVT_v8i16_r(RetVT, Op0, Op0IsKill);
  case MVT::v4i32: return fastEmit_PPCISD_SExtVElems_MVT_v4i32_r(RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_r(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill) override {
  switch (Opcode) {
  case ISD::BITCAST: return fastEmit_ISD_BITCAST_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::BSWAP: return fastEmit_ISD_BSWAP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTLZ: return fastEmit_ISD_CTLZ_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTPOP: return fastEmit_ISD_CTPOP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::CTTZ: return fastEmit_ISD_CTTZ_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FABS: return fastEmit_ISD_FABS_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FCEIL: return fastEmit_ISD_FCEIL_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FFLOOR: return fastEmit_ISD_FFLOOR_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FNEARBYINT: return fastEmit_ISD_FNEARBYINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FNEG: return fastEmit_ISD_FNEG_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_EXTEND: return fastEmit_ISD_FP_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_ROUND: return fastEmit_ISD_FP_ROUND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_SINT: return fastEmit_ISD_FP_TO_SINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FP_TO_UINT: return fastEmit_ISD_FP_TO_UINT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FROUND: return fastEmit_ISD_FROUND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FSQRT: return fastEmit_ISD_FSQRT_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::FTRUNC: return fastEmit_ISD_FTRUNC_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SCALAR_TO_VECTOR: return fastEmit_ISD_SCALAR_TO_VECTOR_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SIGN_EXTEND: return fastEmit_ISD_SIGN_EXTEND_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::SINT_TO_FP: return fastEmit_ISD_SINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::TRUNCATE: return fastEmit_ISD_TRUNCATE_r(VT, RetVT, Op0, Op0IsKill);
  case ISD::UINT_TO_FP: return fastEmit_ISD_UINT_TO_FP_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCFID: return fastEmit_PPCISD_FCFID_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCFIDS: return fastEmit_PPCISD_FCFIDS_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCFIDU: return fastEmit_PPCISD_FCFIDU_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCFIDUS: return fastEmit_PPCISD_FCFIDUS_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCTIDUZ: return fastEmit_PPCISD_FCTIDUZ_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCTIDZ: return fastEmit_PPCISD_FCTIDZ_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCTIWUZ: return fastEmit_PPCISD_FCTIWUZ_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FCTIWZ: return fastEmit_PPCISD_FCTIWZ_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FRE: return fastEmit_PPCISD_FRE_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::FRSQRTE: return fastEmit_PPCISD_FRSQRTE_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::MFVSR: return fastEmit_PPCISD_MFVSR_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::MTCTR: return fastEmit_PPCISD_MTCTR_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::MTVSRA: return fastEmit_PPCISD_MTVSRA_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::MTVSRZ: return fastEmit_PPCISD_MTVSRZ_r(VT, RetVT, Op0, Op0IsKill);
  case PPCISD::SExtVElems: return fastEmit_PPCISD_SExtVElems_r(VT, RetVT, Op0, Op0IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CRXOR, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::ADD4, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::ADD8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADD_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VADDUBM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VADDUHM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VADDUWM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VADDUDM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_MVT_v1i128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v1i128)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VADDUQM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_ADD_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_ADD_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_ADD_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ADD_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ADD_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v1i128: return fastEmit_ISD_ADD_MVT_v1i128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADDC.

unsigned fastEmit_ISD_ADDC_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::ADDC, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADDC_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::ADDC8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
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
  return fastEmitInst_rr(PPC::ADDE, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADDE_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::ADDE8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_ADDE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADDE_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_ADDE_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::AND.

unsigned fastEmit_ISD_AND_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CRAND, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::AND, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::AND8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_AND_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XXLAND, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VAND, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_AND_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_AND_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_AND_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_AND_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_AND_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::BUILD_VECTOR.

unsigned fastEmit_ISD_BUILD_VECTOR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasDirectMove()) && (!PPCSubTarget->isLittleEndian()) && (PPCSubTarget->isISA3_0())) {
    return fastEmitInst_rr(PPC::MTVSRDD, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_BUILD_VECTOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_BUILD_VECTOR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FADD.

unsigned fastEmit_ISD_FADD_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_rr(PPC::XSADDSP, &PPC::VSSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFSADD, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FADDS, &PPC::F4RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSADDDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFDADD, &PPC::SPERCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FADD, &PPC::F8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSADDQP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVADDSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFADDSs, &PPC::QSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VADDFP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVADDDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFADD, &PPC::QFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FADD_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FADD_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FADD_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f128: return fastEmit_ISD_FADD_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FADD_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FADD_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FADD_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FDIV.

unsigned fastEmit_ISD_FDIV_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_rr(PPC::XSDIVSP, &PPC::VSSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFSDIV, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FDIVS, &PPC::F4RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSDIVDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFDDIV, &PPC::SPERCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FDIV, &PPC::F8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSDIVQP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVDIVSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVDIVDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FDIV_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FDIV_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f128: return fastEmit_ISD_FDIV_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FDIV_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FDIV_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXNUM.

unsigned fastEmit_ISD_FMAXNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMAXSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMAXDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FMAXNUM_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FMAXNUM_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMAXNUM_IEEE.

unsigned fastEmit_ISD_FMAXNUM_IEEE_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSMAXDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMAXNUM_IEEE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FMAXNUM_IEEE_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINNUM.

unsigned fastEmit_ISD_FMINNUM_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMINSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMINDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v4f32: return fastEmit_ISD_FMINNUM_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FMINNUM_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMINNUM_IEEE.

unsigned fastEmit_ISD_FMINNUM_IEEE_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSMINDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMINNUM_IEEE_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_ISD_FMINNUM_IEEE_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FMUL.

unsigned fastEmit_ISD_FMUL_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_rr(PPC::XSMULSP, &PPC::VSSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFSMUL, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FMULS, &PPC::F4RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSMULDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFDMUL, &PPC::SPERCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FMUL, &PPC::F8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSMULQP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMULSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFMULSs, &PPC::QSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVMULDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFMUL, &PPC::QFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FMUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FMUL_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FMUL_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f128: return fastEmit_ISD_FMUL_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FMUL_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FMUL_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FMUL_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::FSUB.

unsigned fastEmit_ISD_FSUB_MVT_f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f32)
    return 0;
  if ((PPCSubTarget->hasP8Vector())) {
    return fastEmitInst_rr(PPC::XSSUBSP, &PPC::VSSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFSSUB, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FSUBS, &PPC::F4RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XSSUBDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasSPE())) {
    return fastEmitInst_rr(PPC::EFDSUB, &PPC::SPERCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FSUB, &PPC::F8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_f128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f128)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSSUBQP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVSUBSP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFSUBSs, &PPC::QSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSUBFP, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v2f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2f64)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XVSUBDP, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_MVT_v4f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_rr(PPC::QVFSUB, &PPC::QFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_FSUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f32: return fastEmit_ISD_FSUB_MVT_f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f64: return fastEmit_ISD_FSUB_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::f128: return fastEmit_ISD_FSUB_MVT_f128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f32: return fastEmit_ISD_FSUB_MVT_v4f32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2f64: return fastEmit_ISD_FSUB_MVT_v2f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4f64: return fastEmit_ISD_FSUB_MVT_v4f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CRAND, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::MULLW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::MULLD, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MUL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VMULUWM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_MUL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_MUL_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_MUL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::MULHS.

unsigned fastEmit_ISD_MULHS_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::MULHW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MULHS_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::MULHD, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
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
  return fastEmitInst_rr(PPC::MULHWU, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MULHU_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::MULHDU, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_MULHU_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MULHU_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_MULHU_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::OR.

unsigned fastEmit_ISD_OR_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CROR, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::OR, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::OR8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_OR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XXLOR, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VOR, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_OR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_OR_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_OR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_OR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_OR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::ROTL.

unsigned fastEmit_ISD_ROTL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VRLB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VRLH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VRLW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VRLD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_ROTL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_ROTL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_ROTL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_ROTL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_ROTL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SDIV.

unsigned fastEmit_ISD_SDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::DIVW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::DIVD, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SDIV_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SDIV_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SHL.

unsigned fastEmit_ISD_SHL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SLW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SHL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SHL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSLD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
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
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXSB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXSH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXSW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SMIN.

unsigned fastEmit_ISD_SMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINSB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINSH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINSW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_SMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SRAW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SRA_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRA_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSRAD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
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
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_rr(PPC::MODSW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_rr(PPC::MODSD, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SREM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SREM_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_SREM_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRL.

unsigned fastEmit_ISD_SRL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SRW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SRL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SRL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSRD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
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

unsigned fastEmit_ISD_SUB_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CRXOR, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_SUB_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSUBUBM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSUBUHM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSUBUWM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSUBUDM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_MVT_v1i128_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v1i128)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSUBUQM, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_SUB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_SUB_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_ISD_SUB_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_SUB_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_SUB_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_ISD_SUB_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v1i128: return fastEmit_ISD_SUB_MVT_v1i128_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UDIV.

unsigned fastEmit_ISD_UDIV_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::DIVWU, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_UDIV_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::DIVDU, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_UDIV_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UDIV_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_UDIV_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMAX.

unsigned fastEmit_ISD_UMAX_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXUB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXUH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMAXUW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMAX_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMAX_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMAX_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMAX_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UMIN.

unsigned fastEmit_ISD_UMIN_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINUB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINUH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VMINUW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UMIN_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::v16i8: return fastEmit_ISD_UMIN_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_ISD_UMIN_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_UMIN_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::UREM.

unsigned fastEmit_ISD_UREM_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_rr(PPC::MODUW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_rr(PPC::MODUD, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_UREM_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_UREM_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_UREM_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::XOR.

unsigned fastEmit_ISD_XOR_MVT_i1_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i1)
    return 0;
  return fastEmitInst_rr(PPC::CRXOR, &PPC::CRBITRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::XOR, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::XOR8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_ISD_XOR_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_rr(PPC::XXLXOR, &PPC::VSRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VXOR, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_ISD_XOR_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i1: return fastEmit_ISD_XOR_MVT_i1_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i32: return fastEmit_ISD_XOR_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_ISD_XOR_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_ISD_XOR_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::CMPB.

unsigned fastEmit_PPCISD_CMPB_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::CMPB, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_PPCISD_CMPB_MVT_i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_rr(PPC::CMPB8, &PPC::G8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_PPCISD_CMPB_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_CMPB_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::i64: return fastEmit_PPCISD_CMPB_MVT_i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::FADDRTZ.

unsigned fastEmit_PPCISD_FADDRTZ_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasFPU())) {
    return fastEmitInst_rr(PPC::FADDrtz, &PPC::F8RCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_FADDRTZ_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_FADDRTZ_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::SHL.

unsigned fastEmit_PPCISD_SHL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SLW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_PPCISD_SHL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SHL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SHL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSLW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SHL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSLD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SHL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_SHL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_PPCISD_SHL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_PPCISD_SHL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_PPCISD_SHL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_PPCISD_SHL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::SRA.

unsigned fastEmit_PPCISD_SRA_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SRAW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_PPCISD_SRA_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRA_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRA_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRAW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRA_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSRAD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRA_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_SRA_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_PPCISD_SRA_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_PPCISD_SRA_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_PPCISD_SRA_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_PPCISD_SRA_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::SRL.

unsigned fastEmit_PPCISD_SRL_MVT_i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_rr(PPC::SRW, &PPC::GPRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
}

unsigned fastEmit_PPCISD_SRL_MVT_v16i8_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v16i8)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRB, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRL_MVT_v8i16_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v8i16)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRH, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRL_MVT_v4i32_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasAltivec())) {
    return fastEmitInst_rr(PPC::VSRW, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRL_MVT_v2i64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::v2i64)
    return 0;
  if ((PPCSubTarget->hasP8Altivec())) {
    return fastEmitInst_rr(PPC::VSRD, &PPC::VRRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_SRL_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_SRL_MVT_i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v16i8: return fastEmit_PPCISD_SRL_MVT_v16i8_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v8i16: return fastEmit_PPCISD_SRL_MVT_v8i16_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v4i32: return fastEmit_PPCISD_SRL_MVT_v4i32_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case MVT::v2i64: return fastEmit_PPCISD_SRL_MVT_v2i64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::XSMAXCDP.

unsigned fastEmit_PPCISD_XSMAXCDP_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSMAXCDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_XSMAXCDP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_XSMAXCDP_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::XSMINCDP.

unsigned fastEmit_PPCISD_XSMINCDP_MVT_f64_rr(MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  if (RetVT.SimpleTy != MVT::f64)
    return 0;
  if ((PPCSubTarget->hasP9Vector())) {
    return fastEmitInst_rr(PPC::XSMINCDP, &PPC::VSFRCRegClass, Op0, Op0IsKill, Op1, Op1IsKill);
  }
  return 0;
}

unsigned fastEmit_PPCISD_XSMINCDP_rr(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, unsigned Op1, bool Op1IsKill) {
  switch (VT.SimpleTy) {
  case MVT::f64: return fastEmit_PPCISD_XSMINCDP_MVT_f64_rr(RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
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
  case ISD::BUILD_VECTOR: return fastEmit_ISD_BUILD_VECTOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FADD: return fastEmit_ISD_FADD_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FDIV: return fastEmit_ISD_FDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMAXNUM: return fastEmit_ISD_FMAXNUM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMAXNUM_IEEE: return fastEmit_ISD_FMAXNUM_IEEE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMINNUM: return fastEmit_ISD_FMINNUM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMINNUM_IEEE: return fastEmit_ISD_FMINNUM_IEEE_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FMUL: return fastEmit_ISD_FMUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::FSUB: return fastEmit_ISD_FSUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MUL: return fastEmit_ISD_MUL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHS: return fastEmit_ISD_MULHS_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::MULHU: return fastEmit_ISD_MULHU_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::OR: return fastEmit_ISD_OR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::ROTL: return fastEmit_ISD_ROTL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SDIV: return fastEmit_ISD_SDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SHL: return fastEmit_ISD_SHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMAX: return fastEmit_ISD_SMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SMIN: return fastEmit_ISD_SMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRA: return fastEmit_ISD_SRA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SREM: return fastEmit_ISD_SREM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SRL: return fastEmit_ISD_SRL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::SUB: return fastEmit_ISD_SUB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UDIV: return fastEmit_ISD_UDIV_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMAX: return fastEmit_ISD_UMAX_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UMIN: return fastEmit_ISD_UMIN_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::UREM: return fastEmit_ISD_UREM_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case ISD::XOR: return fastEmit_ISD_XOR_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::CMPB: return fastEmit_PPCISD_CMPB_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::FADDRTZ: return fastEmit_PPCISD_FADDRTZ_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::SHL: return fastEmit_PPCISD_SHL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::SRA: return fastEmit_PPCISD_SRA_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::SRL: return fastEmit_PPCISD_SRL_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::XSMAXCDP: return fastEmit_PPCISD_XSMAXCDP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  case PPCISD::XSMINCDP: return fastEmit_PPCISD_XSMINCDP_rr(VT, RetVT, Op0, Op0IsKill, Op1, Op1IsKill);
  default: return 0;
  }
}

// FastEmit functions for ISD::SRA.

unsigned fastEmit_ISD_SRA_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(PPC::SRAWI, &PPC::GPRCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(PPC::SRADI, &PPC::G8RCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_SRA_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_SRA_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_ISD_SRA_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::EXTSWSLI.

unsigned fastEmit_PPCISD_EXTSWSLI_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  if ((PPCSubTarget->isISA3_0())) {
    return fastEmitInst_ri(PPC::EXTSWSLI_32_64, &PPC::G8RCRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_PPCISD_EXTSWSLI_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_EXTSWSLI_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::QVESPLATI.

unsigned fastEmit_PPCISD_QVESPLATI_MVT_v4i1_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i1)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_ri(PPC::QVESPLATIb, &PPC::QBRCRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_PPCISD_QVESPLATI_MVT_v4f32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4f32)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_ri(PPC::QVESPLATIs, &PPC::QSRCRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_PPCISD_QVESPLATI_MVT_v4f64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4f64)
    return 0;
  if ((PPCSubTarget->hasQPX())) {
    return fastEmitInst_ri(PPC::QVESPLATI, &PPC::QFRCRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_PPCISD_QVESPLATI_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i1: return fastEmit_PPCISD_QVESPLATI_MVT_v4i1_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v4f32: return fastEmit_PPCISD_QVESPLATI_MVT_v4f32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::v4f64: return fastEmit_PPCISD_QVESPLATI_MVT_v4f64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::TC_RETURN.

unsigned fastEmit_PPCISD_TC_RETURN_MVT_i32_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_ri(PPC::TCRETURNri, &PPC::CTRRCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_PPCISD_TC_RETURN_MVT_i64_ri(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::isVoid)
    return 0;
  return fastEmitInst_ri(PPC::TCRETURNri8, &PPC::CTRRC8RegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_PPCISD_TC_RETURN_ri(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_PPCISD_TC_RETURN_MVT_i32_ri(RetVT, Op0, Op0IsKill, imm1);
  case MVT::i64: return fastEmit_PPCISD_TC_RETURN_MVT_i64_ri(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) override {
  if (VT == MVT::i32 && Predicate_imm32SExt16(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm32SExt16(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  if (VT == MVT::i64 && Predicate_imm64SExt16(imm1))
    if (unsigned Reg = fastEmit_ri_Predicate_imm64SExt16(VT, RetVT, Opcode, Op0, Op0IsKill, imm1))
      return Reg;

  switch (Opcode) {
  case ISD::SRA: return fastEmit_ISD_SRA_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case PPCISD::EXTSWSLI: return fastEmit_PPCISD_EXTSWSLI_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case PPCISD::QVESPLATI: return fastEmit_PPCISD_QVESPLATI_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  case PPCISD::TC_RETURN: return fastEmit_PPCISD_TC_RETURN_ri(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm32SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(PPC::ADDI, &PPC::GPRCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_imm32SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADD_MVT_i32_ri_Predicate_imm32SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADDC.

unsigned fastEmit_ISD_ADDC_MVT_i32_ri_Predicate_imm32SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(PPC::ADDIC, &PPC::GPRCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADDC_ri_Predicate_imm32SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_ADDC_MVT_i32_ri_Predicate_imm32SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i32_ri_Predicate_imm32SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i32)
    return 0;
  return fastEmitInst_ri(PPC::MULLI, &PPC::GPRCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_imm32SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i32: return fastEmit_ISD_MUL_MVT_i32_ri_Predicate_imm32SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for PPCISD::XXSPLT.

unsigned fastEmit_PPCISD_XXSPLT_MVT_v4i32_ri_Predicate_imm32SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::v4i32)
    return 0;
  if ((PPCSubTarget->hasVSX())) {
    return fastEmitInst_ri(PPC::XXSPLTW, &PPC::VSRCRegClass, Op0, Op0IsKill, imm1);
  }
  return 0;
}

unsigned fastEmit_PPCISD_XXSPLT_ri_Predicate_imm32SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::v4i32: return fastEmit_PPCISD_XXSPLT_MVT_v4i32_ri_Predicate_imm32SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm32SExt16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_imm32SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::ADDC: return fastEmit_ISD_ADDC_ri_Predicate_imm32SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_imm32SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  case PPCISD::XXSPLT: return fastEmit_PPCISD_XXSPLT_ri_Predicate_imm32SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADD.

unsigned fastEmit_ISD_ADD_MVT_i64_ri_Predicate_imm64SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(PPC::ADDI8, &PPC::G8RCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADD_ri_Predicate_imm64SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ADD_MVT_i64_ri_Predicate_imm64SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::ADDC.

unsigned fastEmit_ISD_ADDC_MVT_i64_ri_Predicate_imm64SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(PPC::ADDIC8, &PPC::G8RCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_ADDC_ri_Predicate_imm64SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_ADDC_MVT_i64_ri_Predicate_imm64SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// FastEmit functions for ISD::MUL.

unsigned fastEmit_ISD_MUL_MVT_i64_ri_Predicate_imm64SExt16(MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  if (RetVT.SimpleTy != MVT::i64)
    return 0;
  return fastEmitInst_ri(PPC::MULLI8, &PPC::G8RCRegClass, Op0, Op0IsKill, imm1);
}

unsigned fastEmit_ISD_MUL_ri_Predicate_imm64SExt16(MVT VT, MVT RetVT, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (VT.SimpleTy) {
  case MVT::i64: return fastEmit_ISD_MUL_MVT_i64_ri_Predicate_imm64SExt16(RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

// Top-level FastEmit function.

unsigned fastEmit_ri_Predicate_imm64SExt16(MVT VT, MVT RetVT, unsigned Opcode, unsigned Op0, bool Op0IsKill, uint64_t imm1) {
  switch (Opcode) {
  case ISD::ADD: return fastEmit_ISD_ADD_ri_Predicate_imm64SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::ADDC: return fastEmit_ISD_ADDC_ri_Predicate_imm64SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  case ISD::MUL: return fastEmit_ISD_MUL_ri_Predicate_imm64SExt16(VT, RetVT, Op0, Op0IsKill, imm1);
  default: return 0;
  }
}

